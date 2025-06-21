#include "mui.h"

#include <xcb/xcb.h>
#include <xcb/render.h>

#include <ft2build.h>
#include <freetype/freetype.h>


#define TEXT_MAX_SIZE       255


struct {
    FT_Library          library;
    char               *path;
    int                 init;
    int                 size;

    xcb_connection_t   *conn;
} xfont;


struct glyphset {
    xcb_render_glyphset_t id;
    unsigned int height;
};


/*
 * glyph header that is the first part of the glyph_cmds argument in 
 * `xcb_render_composite_glyphs_8/16/32`. Succeeding that are the
 * glyph ids that can be either 8/16/32 bits
 */
struct gheader {
    uint8_t count;
    uint8_t pad[3];
    uint16_t dx;
    uint16_t dy;
};


struct tstream {
    uint8_t             count;
    uint8_t            *glyphs;
    uint32_t           *data;

    TAILQ_ENTRY(tstream) entries;
};



struct glyphset **loaded_gs;


/***************************************************/
/* UTILITY FUNCTIONS */
/***************************************************/
static void
load_glyph(struct glyphset *gs, FT_Face face, int charcode)
{
    int gindx;
    uint32_t gid;
    xcb_render_glyphinfo_t ginfo;

    gindx = FT_Get_Char_Index(face, charcode);
    FT_Load_Glyph(face, gindx, FT_LOAD_RENDER | FT_LOAD_FORCE_AUTOHINT);

    FT_Bitmap *bitmap = &face->glyph->bitmap;

    ginfo.x = -face->glyph->bitmap_left;
    ginfo.y =  face->glyph->bitmap_top;
    ginfo.width = bitmap->width;
    ginfo.height = bitmap->rows;
    ginfo.x_off = face->glyph->advance.x / 64;
    ginfo.y_off = face->glyph->advance.y / 64;

    gid = charcode;

    int stride = (ginfo.width+3)&~3;
    uint8_t tmp_bitmap[stride * ginfo.height];
    for (int y = 0; y < ginfo.height; y++)
        memcpy(tmp_bitmap+y*stride, bitmap->buffer+y * ginfo.width, ginfo.width);

    int height = face->glyph->metrics.height / 64;
    gs->height = (gs->height < height) ? height : gs->height;
    xcb_render_add_glyphs(xfont.conn, gs->id, 1, &gid, &ginfo, stride * ginfo.height, tmp_bitmap);
    xcb_flush(xfont.conn);

}


void
load_glyphset(int id)
{
    FT_Face face;
    if (FT_New_Face(xfont.library, xfont.path, 0, &face)) {
        DEBUG_PRINT(DBUG_ERR, "failed to load FreeType face");
        exit(1);
    }

    loaded_gs[id]->id = xcb_generate_id(xfont.conn);
    xcb_render_create_glyph_set(xfont.conn, loaded_gs[id]->id, xfmt.alpha8);

    FT_Set_Char_Size(face, 0, xfont.size * 64, 90, 90);

    for (int n = 32; n < 128; n++)
        load_glyph(loaded_gs[id], face, n);

    FT_Done_Face(face);
}


int
add_text(struct mui_text *mt, uint8_t *glyphs, unsigned int count, int dx, int dy)
{
    uint32_t glyphid;
    int write = count;
    struct tstream *ts;
    struct gheader header = { count, {0, 0, 0}, 0, dy };

    ts = calloc(1, sizeof(*ts));

    int offx = 2;
    uint32_t *data = calloc(count + offx, sizeof(uint32_t));

    memcpy(data, &header, sizeof(header));

    for (int i = 0; i < count; i++) {
        memcpy(&glyphid, glyphs+i, sizeof(*glyphs));
        memcpy(data + offx, &glyphid, sizeof(glyphid));
        offx = offx + 1;

        /*
         * If a newline character is the glyph add text again
         */
        if (glyphs[i] == '\n') {
            header.count = i;
            data = reallocarray(data, offx, sizeof(uint32_t));

            ts->glyphs = malloc(i + 1);
            memcpy(ts->glyphs, glyphs, i);
            ts->glyphs[i] = '\0';

            memcpy(data, &header, sizeof(header));
            write = ++i;
            break;
        }
    }

    ts->data = data;
    ts->count = header.count;

    TAILQ_INSERT_TAIL(&mt->ts_head, ts, entries);
    return write;
}


/***************************************************/
/* INTERNAL FUNCTIONS */
/***************************************************/

int
text_init(void)
{
    if (xfont.init) {
        DEBUG_PRINT(DBUG_WAR, "xfont already initialized!");
        return 1;
    }

    if (FT_Init_FreeType(&xfont.library)) {
        DEBUG_PRINT(DBUG_ERR, "failed to initialized FreeType");
        exit(1);
    }

    char fpath[] = "/home/manu/dev/c99/mui/fonts/spleen-12x24.otf";
    xfont.path   = malloc(sizeof(fpath) + 1);
    memcpy(xfont.path, fpath, sizeof(fpath));
    xfont.path[sizeof(fpath)] = '\0';

    xfont.conn = xorg_get_connection();
    xfont.size = 12;

    loaded_gs = calloc(1, sizeof(*loaded_gs));
    loaded_gs[0] = calloc(1, sizeof(**loaded_gs));

    load_glyphset(0);

    return 0;
}


void
text_attach_window(struct mui_text *mt, struct xorg_info xi)
{
    xcb_pixmap_t    pixmap;
    uint32_t        vl[] = { XCB_RENDER_REPEAT_NORMAL };
    xcb_rectangle_t rect = { 0, 0, 1, 1 };

    mt->pen = xcb_generate_id(xi.conn);
    pixmap  = xcb_generate_id(xi.conn);

    xcb_create_pixmap(xi.conn,
                      32,
                      pixmap,
                      xi.win_id,
                      1,
                      1
    );

    xcb_render_create_picture(
                      xi.conn,
                      mt->pen,
                      pixmap,
                      xfmt.argb32,
                      XCB_RENDER_CP_REPEAT,
                      vl
    );

    xcb_render_fill_rectangles(
                      xi.conn,
                      XCB_RENDER_PICT_OP_SRC,
                      mt->pen,
                      (xcb_render_color_t) { 0, .alpha = 0xffff },
                      1,
                      &rect
    );

    xcb_free_pixmap(xi.conn, pixmap);
    DEBUG_PRINT(DBUG_MSG, "text attached to window");
}

void
text_detach_window(struct mui_text *mt)
{

}

void
text_draw(struct mui_text *mt, struct xorg_info xi)
{
    TAILQ_FOREACH(mt->ts_np, &mt->ts_head, entries) {
        struct tstream *ts = mt->ts_np;

        xcb_render_composite_glyphs_32(
            xi.conn, XCB_RENDER_PICT_OP_OVER, mt->pen, xi.pic_id, 0,
            loaded_gs[0]->id, 0, 0, sizeof(struct gheader) + ts->count * sizeof(uint32_t),
            (uint8_t *) ts->data
        );
    }
}


/***************************************************/
/* EXTERNAL FUNCTIONS */
/***************************************************/
struct mui_text*
mui_create_text(uint8_t *glyphs, unsigned int count)
{
    struct mui_text *mt;
    mt = calloc(1, sizeof(*mt));

    TAILQ_INIT(&mt->ts_head);

    if (count > 0) {
        mui_text_add(mt, glyphs, count, 0);
    }
    return mt;
}

void
mui_delete_text(struct mui_text *mt)
{

}


void
mui_text_add(struct mui_text *mt, uint8_t *glyphs, unsigned int count, unsigned int start)
{
    int iter = 0;
    int write = 0;
    do {
        iter += 1;
        count  -= write;
        glyphs += write;
        write = add_text(mt, glyphs, count, 0, iter * loaded_gs[0]->height);
    } while (count > 0);
}
