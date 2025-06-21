/*
 * This section covers the xcb section for the GUI library. This rely on xcb
 * and Xrender for most of the work.
 *
 *
 * From what I understand with xcb-render you need to query a request and
 * fetch the reply to receive the information you want in the returned struct.
 * You can view the content of the struct in the header file of the
 * extension you want to use.
 */



#include <stdlib.h>
#include <string.h>
#include <sys/queue.h>

#include <xcb/xcb.h>
#include <xcb/xcb_renderutil.h>


#include "mui.h"


struct _xfmt xfmt;

struct {
    xcb_connection_t           *conn;
    xcb_screen_t               *screen;
} xorg;



LIST_HEAD(, xwin) xwhead;
struct xwin {
    xcb_window_t                window;
    xcb_gcontext_t              gc;
    struct                      mui_win *mw;
    xcb_pixmap_t                bg_pixmap;
    xcb_render_picture_t        bg_picture;
    xcb_render_color_t          bg_color;
    xcb_pixmap_t                fg_pixmap;
    xcb_render_picture_t        fg_picture;


    xcb_intern_atom_reply_t    *wm_delete_window;
    LIST_ENTRY(xwin)            entries;
} *xwnp;



/*
 * Font information
 *
 * To render fonts FreeType library will be used as there is nothing
 */
xcb_pixmap_t            ft_pixmap;
xcb_render_picture_t    ft_picture;
xcb_render_glyphset_t   gs;
xcb_render_util_composite_text_stream_t *ts;

void render_window(struct xwin*);


void *xorg_get_connection() { return xorg.conn; }

uint32_t 
xorg_get_window(struct mui_win *mw)
{
    struct xwin *xw = (struct xwin*)mw->xorg_win;
    return xw->window;
}


struct xorg_info
xorg_get_info(struct mui_win *mw)
{
    struct xwin *xw = (struct xwin*)mw->xorg_win;
    struct xorg_info xinfo = {
        .conn       = xorg.conn,
        .win_id     = xw->window,
        .fg_pic     = xw->fg_picture,
        .pic_id     = xw->bg_picture,
        .bg_pix     = xw->bg_pixmap,
        .gc         = xw->gc
    };

    return xinfo;
}


/*
 * This function is to fetch the picture formats that will be used between different
 * c source files. The values are stored in the `xfmt` struct that is used globaly
 */

static xcb_render_pictdepth_t*
find_screen_pictdepth(xcb_render_pictscreen_t *ps)
{
    if (ps == NULL) return NULL;

    xcb_render_pictdepth_iterator_t iter =
        xcb_render_pictscreen_depths_iterator(ps);

    for (int i = 0; i < ps->num_depths; i++) {
        xcb_render_pictdepth_t *pd = iter.data;
        if (pd->depth == xorg.screen->root_depth)
            return pd;
        xcb_render_pictdepth_next(&iter);
    }
    return NULL;
}

static xcb_render_pictvisual_t*
find_screen_pictvisual(xcb_render_pictdepth_t *pd)
{
    if (pd == NULL) return NULL;

    xcb_render_pictvisual_iterator_t iter =
        xcb_render_pictdepth_visuals_iterator(pd);

    for (int i = 0; i < pd->num_visuals; i++) {
        xcb_render_pictvisual_t *pv = iter.data;

        if (pv->visual == xorg.screen->root_visual)
            return pv;

        xcb_render_pictvisual_next(&iter);
    }

    return NULL;
}

static xcb_render_pictformat_t
find_screen_format(xcb_render_query_pict_formats_reply_t *reply)
{
    xcb_render_pictscreen_iterator_t iter =
        xcb_render_query_pict_formats_screens_iterator(reply);

    for (int n = 0; n < reply->num_screens; n++) {

        xcb_render_pictscreen_t *ps = iter.data;

        xcb_render_pictdepth_t  *pd = find_screen_pictdepth(ps);
        xcb_render_pictvisual_t *pv = find_screen_pictvisual(pd);

        /* window format found */
        if (pv != NULL) return pv->format;

        xcb_render_pictscreen_next(&iter);
    }

    return 0;
}


static void
load_xrender_formats()
{
    xcb_render_query_version_cookie_t vrequest = 
        xcb_render_query_version_unchecked(xorg.conn, 1, 1);

    xcb_render_query_version_reply_t *vreply = 
        xcb_render_query_version_reply(xorg.conn, vrequest, NULL);

    xcb_render_query_pict_formats_cookie_t fmt_request = 
        xcb_render_query_pict_formats_unchecked(xorg.conn);

    xcb_render_query_pict_formats_reply_t *fmt_reply = 
        xcb_render_query_pict_formats_reply(xorg.conn, fmt_request, NULL);

    xcb_render_pictforminfo_t *alpha8 = 
        xcb_render_util_find_standard_format(fmt_reply, XCB_PICT_STANDARD_A_8);

    xcb_render_pictforminfo_t *argb32 = 
        xcb_render_util_find_standard_format(fmt_reply, XCB_PICT_STANDARD_ARGB_32);

    xfmt.normal = find_screen_format(fmt_reply);
    xfmt.alpha8 = alpha8->id;
    xfmt.argb32 = argb32->id;

    free(vreply);
    free(fmt_reply);

    DEBUG_PRINT(DBUG_MSG, "xrender format created sucessfuly");
}



void *
xorg_attach_internal(struct mui_win *mw)
{
    /*
     * connect to X server if its the first time creating an
     * xcb window
     */

    if (xorg.conn == NULL) {
        DEBUG_PRINT(DBUG_MSG, "connecting to X server...");
        xorg.conn = xcb_connect(NULL, NULL);

        int cerror = xcb_connection_has_error(xorg.conn);

        if (cerror) {
            switch (cerror) {
                case XCB_CONN_ERROR:
                    DEBUG_PRINT(DBUG_ERR, "failed to connect to X server (socket, pipe, stream)");
                    break;

                case XCB_CONN_CLOSED_EXT_NOTSUPPORTED:
                    DEBUG_PRINT(DBUG_ERR, "failed to connect to X server (extension unsupported");
                    break;

                case XCB_CONN_CLOSED_MEM_INSUFFICIENT:
                    DEBUG_PRINT(DBUG_ERR, "failed to connect to X server (not enough memory");
                    break;

                case XCB_CONN_CLOSED_REQ_LEN_EXCEED:
                    DEBUG_PRINT(DBUG_ERR, "failed to connect to X server (timeout)");
                    break;

                case XCB_CONN_CLOSED_PARSE_ERR:
                    DEBUG_PRINT(DBUG_ERR, "failed to connect to X server (display parse error)");
                    break;

                case XCB_CONN_CLOSED_INVALID_SCREEN:
                    DEBUG_PRINT(DBUG_ERR, "failed to connect to X server (invalid screen)");
                    break;
            }
            exit(1);
        } 

        DEBUG_PRINT(DBUG_MSG, "successfuly connected to X server");

        const xcb_setup_t      *setup = xcb_get_setup(xorg.conn);
        xcb_screen_iterator_t   iter = xcb_setup_roots_iterator(setup);
        xorg.screen = iter.data;

        load_xrender_formats();
        text_init();
        LIST_INIT(&xwhead);
    }

    struct xwin    *xw;
    xw = malloc(sizeof(struct xwin));


    uint32_t        mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
    uint32_t        values[2] = { 0xcbcbcb, XCB_EVENT_MASK_EXPOSURE };

    xw->gc = xcb_generate_id(xorg.conn);
    xw->window = xcb_generate_id(xorg.conn);




    xcb_create_window(xorg.conn,
                      XCB_COPY_FROM_PARENT,
                      xw->window,
                      xorg.screen->root,
                      0, 0,
                      mw->width,
                      mw->height,
                      0,
                      XCB_WINDOW_CLASS_INPUT_OUTPUT,
                      xorg.screen->root_visual,
                      mask,
                      values
    );

    mask = XCB_GC_FOREGROUND | XCB_GC_GRAPHICS_EXPOSURES;
    values[0] = 0xcbcbcb;
    values[1] = 0;


    /*
     * Removing the default background pixmap to avoid issues when using
     * `xcb_copy_area` while rendering
     */
    const xcb_change_window_attributes_value_list_t cw = { 0 };
    xcb_change_window_attributes(xorg.conn, xw->window, XCB_CW_BACK_PIXMAP, &cw);


    /*
     * Creating the pixmap and picture that will be assigned to the
     * xcb window
     */
    xcb_create_gc(xorg.conn, xw->gc, xw->window, mask, values);

    xw->bg_pixmap = xcb_generate_id(xorg.conn);
    xw->bg_picture = xcb_generate_id(xorg.conn);
    xcb_create_pixmap(xorg.conn,
                      xorg.screen->root_depth,
                      xw->bg_pixmap,
                      xw->window,
                      mw->width,
                      mw->height
    );
    xcb_render_create_picture(xorg.conn,
                              xw->bg_picture,
                              xw->bg_pixmap,
                              xfmt.normal,
                              0,
                              NULL
    );

    xw->bg_color.red = 0xcbcb;
    xw->bg_color.green = 0xcbcb;
    xw->bg_color.blue = 0xcbcb;
    xw->bg_color.alpha = 0xffff;

    xw->fg_pixmap = xcb_generate_id(xorg.conn);
    xw->fg_picture = xcb_generate_id(xorg.conn);
    xcb_create_pixmap(xorg.conn,
                      32,
                      xw->fg_pixmap,
                      xw->window,
                      mw->width,
                      mw->height
    );
    xcb_render_create_picture(xorg.conn,
                              xw->fg_picture,
                              xw->fg_pixmap,
                              xfmt.argb32,
                              0,
                              NULL
    );


    xcb_intern_atom_cookie_t cookie1 = xcb_intern_atom(xorg.conn, 1, 12, "WM_PROTOCOLS");
    xcb_intern_atom_cookie_t cookie2 = xcb_intern_atom(xorg.conn, 1, 16, "WM_DELETE_WINDOW");

    xcb_intern_atom_reply_t *reply1 = xcb_intern_atom_reply(xorg.conn, cookie1, 0);
    xw->wm_delete_window = xcb_intern_atom_reply(xorg.conn, cookie2, 0);


    xcb_change_property(xorg.conn,
                        XCB_PROP_MODE_REPLACE,
                        xw->window,
                        reply1->atom,
                        4,
                        32,
                        1,
                        &(xw->wm_delete_window->atom)
    );


    xcb_change_property(xorg.conn,
                        XCB_PROP_MODE_REPLACE,
                        xw->window,
                        XCB_ATOM_WM_NAME,
                        XCB_ATOM_STRING,
                        8,
                        strlen(mw->name),
                        mw->name
    );


    mw->xorg_win = xw;
    xw->mw = mw;

    xcb_map_window(xorg.conn, xw->window);

    LIST_INSERT_HEAD(&xwhead, xw, entries);
    xcb_flush(xorg.conn);
    return xw;
}



void
xorg_detach_internal(struct mui_win *mw)
{
    struct xwin *xw = (struct xwin*)mw->xorg_win;
    LIST_REMOVE(xw, entries);

    xcb_free_pixmap(xorg.conn, xw->bg_pixmap);
    xcb_free_gc(xorg.conn, xw->gc);
    xcb_destroy_window(xorg.conn, xw->window);
    xcb_flush(xorg.conn);
}



void
render_window(struct xwin *xw)
{
    struct mui_win *mw = xw->mw;
    //struct xorg_info xi = xorg_get_info(mw);

    xcb_render_fill_rectangles(xorg.conn,
                               XCB_RENDER_PICT_OP_SRC,
                               xw->bg_picture,
                               xw->bg_color,
                               1, &(xcb_rectangle_t) { 0, 0, mw->width, mw->height }
    );

    xcb_render_fill_rectangles(xorg.conn,
                               XCB_RENDER_PICT_OP_SRC,
                               xw->fg_picture,
                               xw->bg_color,
                               1, &(xcb_rectangle_t) { 0, 0, mw->width, mw->height }
    );


    xcb_render_composite(xorg.conn,
                         XCB_RENDER_PICT_OP_SRC,
                         xw->fg_picture,
                         0,
                         xw->bg_picture,
                         0, 0,
                         0, 0,
                         0, 0,
                         mw->width,
                         mw->height
    );


    /*
    TAILQ_FOREACH(mw->elm_np, &mw->elm_head, entries) {
        struct mui_element *e = mw->elm_np;

        switch (e->type) {
            case MUI_ELEMENT_TEXT: {
                struct mui_text *mtext = e->data.text;
                text_draw(mtext, xi);
                break;
            }

            case MUI_ELEMENT_IMAGE: {
                struct mui_image *mimg = e->data.image;
                image_draw(mimg, xi);
                break;
            }
        }
    }
    */
    group_update(mw->group);

    xcb_copy_area(xorg.conn,
                  xw->bg_pixmap,
                  xw->window,
                  xw->gc,
                  0, 0,
                  0,0,
                  mw->width,
                  mw->height
    );
    xcb_flush(xorg.conn);
}

void
resize_window(struct xwin *xw)
{
    struct mui_win *mw = xw->mw;
    xcb_render_free_picture(xorg.conn, xw->bg_picture);
    xcb_render_free_picture(xorg.conn, xw->fg_picture);
    xcb_free_pixmap(xorg.conn, xw->bg_pixmap);
    xcb_free_pixmap(xorg.conn, xw->fg_pixmap);
    xcb_configure_window(xorg.conn,
                         xw->window,
                         XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT, 
                         &(xcb_configure_window_value_list_t) { .width = mw->width, .height = mw->height }
    );


    xcb_create_pixmap(xorg.conn,
                      xorg.screen->root_depth,
                      xw->bg_pixmap,
                      xw->window,
                      mw->width,
                      mw->height
    );
    xcb_create_pixmap(xorg.conn,
                      32,
                      xw->fg_pixmap,
                      xw->window,
                      mw->width,
                      mw->height
    );
    xcb_render_create_picture(xorg.conn,
                              xw->bg_picture,
                              xw->bg_pixmap,
                              xfmt.normal,
                              0,
                              NULL
    );
    xcb_render_create_picture(xorg.conn,
                              xw->fg_picture,
                              xw->fg_pixmap,
                              xfmt.argb32,
                              0,
                              NULL
    );
}


struct xwin *
match_window(uint32_t id) {
    LIST_FOREACH(xwnp, &xwhead, entries) {
        if (id == xwnp->window)
            return xwnp;
    }
    return NULL;
}

void
xorg_handle_events()
{
    xcb_generic_event_t    *event;

    while ((event = xcb_poll_for_event(xorg.conn))) {
        switch(event->response_type & ~0x80) {
            case XCB_EXPOSE: {
                struct xcb_expose_event_t *expose = (xcb_expose_event_t *)event;
                struct xwin *xw = match_window(expose->window);
                struct mui_win *mw = xw->mw;

                if (mw->width != expose->width || mw->height != expose->height) {
                    mui_push_event(mw, &(struct mui_ev) { .type = MUI_EV_RESIZE,
                        .resize = {
                            .pre_width  = mw->width,
                            .pre_height = mw->height,
                            .new_width  = (mw->width = expose->width),
                            .new_height = (mw->height = expose->height)
                        }});
                    resize_window(xw);
                }
                render_window(xw);
                break;
            }
            case XCB_CLIENT_MESSAGE: {
                struct xcb_client_message_event_t *client_message = (xcb_client_message_event_t *) event;
                struct xwin *xw = match_window(client_message->window);
                struct mui_win *mw = xw->mw;

                if (client_message->data.data32[0] == xw->wm_delete_window->atom) {
                    mui_push_event(mw, &(struct mui_ev) { .type = MUI_EV_QUIT });
                }
                xcb_flush(xorg.conn);
                break;
            }
            default:
                break;

        }
        free(event);
        continue;
    }
}
