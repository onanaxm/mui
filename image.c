#include <stdlib.h>
#include <string.h>

#include <xcb/render.h>
#include <xcb/xcb_image.h>

#include "mui.h"



/************************************************************/
/* UTILITY FUNCTIONS */
/************************************************************/
uint32_t
u8_to_u32(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
    uint8_t ra = (float) r / 255 * a;
    uint8_t ga = (float) g / 255 * a;
    uint8_t ba = (float) b / 255 * a;
    uint32_t result = (ra << 24) + (ga << 16) + (ba << 8) + a;

    return result;
}


/************************************************************/
/* INTERNAL FUNCTIONS */
/************************************************************/
void
image_attach_window(struct mui_image *mimg, struct xorg_info xi)
{
    mimg->pix = xcb_generate_id(xi.conn);
    mimg->pic = xcb_generate_id(xi.conn);
    xcb_create_pixmap(xi.conn, 32, mimg->pix, xi.win_id, mimg->width, mimg->height);

    uint8_t *data = malloc(mimg->width * mimg->height * 4);


    for (int i = 0; i < mimg->width * mimg->height; i++) {
        int start = i * 4;
        uint32_t rgba = u8_to_u32(mimg->data[start + 0],
                                  mimg->data[start + 1],
                                  mimg->data[start + 2],
                                  mimg->data[start + 3]
        );
        uint8_t  r = (rgba & 0xff000000) >> 24;
        uint8_t  g = (rgba & 0x00ff0000) >> 16;
        uint8_t  b = (rgba & 0x0000ff00) >> 8;
        data[start + 0] = b;
        data[start + 1] = g;
        data[start + 2] = r;
        data[start + 3] = mimg->data[start + 3];
    }

    mimg->ximg = xcb_image_create_native(xi.conn,
                     mimg->width,
                     mimg->height,
                     XCB_IMAGE_FORMAT_Z_PIXMAP,
                     32,
                     NULL,
                     0,
                     data
    );

    xcb_render_create_picture(xi.conn,
                              mimg->pic,
                              mimg->pix,
                              xfmt.argb32,
                              0,
                              NULL
    );

    xcb_gcontext_t gc = xcb_generate_id(xi.conn);
    xcb_create_gc(xi.conn, gc, mimg->pix, 0, NULL);
    xcb_image_put(xi.conn, mimg->pix, gc, mimg->ximg, 0, 0, 0);
    xcb_free_gc(xi.conn, gc);

    DEBUG_PRINT(DBUG_MSG, "image attached to window");
}


void
image_draw(struct mui_image *img, struct xorg_info xi)
{
    xcb_render_composite(xi.conn,
                         XCB_RENDER_PICT_OP_OVER,
                         img->pic,
                         0,
                         xi.pic_id,
                         0, 0,
                         0, 0,
                         0, 0,
                         img->width,
                         img->height
    );

    xcb_flush(xi.conn);
}


/************************************************************/
/* EXTERNAL FUNCTIONS */
/************************************************************/
struct mui_image*
mui_create_image(uint8_t *source, uint16_t width, uint16_t height)
{
    struct mui_image *img;
    img = malloc(sizeof(*img));
    img->width = width;
    img->height = height;

    uint8_t *dest = malloc(width * height * 4);
    memcpy(dest, source, width * height * 4); 
    img->data = dest;


    return img;
}
