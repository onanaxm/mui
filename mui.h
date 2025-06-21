#ifndef _MUI_H_
#define _MUI_H_

#include <stdio.h>
#include <stdint.h>
#include <sys/queue.h>

enum {
    MUI_ELEMENT_TEXT,
    MUI_ELEMENT_IMAGE,
    MUI_ELEMENT_GROUP
};


/*
 * this is an internal struct that should not be used by developers.
 * use at your own risk
 */
struct mui_element {
    unsigned int type;
    union {
        struct mui_text  *text;
        struct mui_image *image;
        struct mui_group *group;
    } data;

    TAILQ_ENTRY(mui_element) entries;
};


enum {
    MUI_EV_NONE,
    MUI_EV_QUIT,
    MUI_EV_RESIZE,
    MUI_EV_KREYPRESS,
    MUI_EV_KEYRELEASE,
    MUI_EV_MOUSEPRESS,
    MUI_EV_MOUSE_RELEASE
};

struct ev_resize { 
    unsigned int pre_width, pre_height;
    unsigned int new_width, new_height;
};

struct ev_keypress { };

struct ev_keyrelease { };

struct ev_mousepress { };

struct ev_mouserelease { };


struct mui_ev {

    unsigned int    type;

    struct          ev_resize resize;
    struct          ev_keypress keypress;
    struct          ev_keyrelease keyrelease;
    struct          ev_mousepress mousepress;
    struct          ev_mouserelease mouserelease;


    TAILQ_ENTRY(mui_ev) entries;
};


/******************************************************************************/
/* LIST OF MUI WIDGETS */
/******************************************************************************/

struct mui_text {
    uint32_t  pen;
    struct    tstream *ts_np;
    TAILQ_HEAD(tts_head, tstream) ts_head;
};

struct mui_image { 
    void       *ximg;
    uint32_t    pix;
    uint32_t    pic;
    uint16_t    width;
    uint16_t    height;
    uint8_t    *data;
};


struct mui_group { 
    struct mui_win     *mw;
    struct mui_element *members;
    struct mui_element *mem_np;
    TAILQ_HEAD(mem_head, mui_element) mem_head;
};


/*
 * Used for region highlight and shape rendering
 */

#define MUI_WIN_EV_SIZE     32

struct mui_win {
    char           *name;
    unsigned int    width;
    unsigned int    height;

    void           *xorg_win;
    struct          mui_group *group;

    /* window events */
    struct               mui_ev *evnp;
    unsigned int         ev_size;
    TAILQ_HEAD(elisthead, mui_ev) ev_head;
};


struct mui_win     *mui_create_window(const char*, unsigned int, unsigned int);
void                mui_delete_window(struct mui_win*);
void                mui_update(void);


/*
 * This methods adds an element into a container. The container can
 * either be a window or a widget
 */
void                mui_add(unsigned int, void*, unsigned int, void*);
void                mui_remove(unsigned int, void*, unsigned int, void*);


void                mui_win_add(struct mui_win*, unsigned int, void*);
void                mui_win_remove(struct mui_win*, unsigned int, void*);


struct mui_text    *mui_create_text(uint8_t*, unsigned int len);
void                mui_delete_text(struct mui_text*);
void                mui_text_add(struct mui_text*, uint8_t*, unsigned int, unsigned int);


struct mui_image   *mui_create_image(uint8_t*, uint16_t, uint16_t);


struct mui_group   *mui_create_group(void);
void                mui_group_add(struct mui_group*, unsigned int, void*);



int                 mui_event_pending(struct mui_win*);
void                mui_push_event(struct mui_win*, struct mui_ev*);
struct mui_ev       mui_pop_event(struct mui_win*);



/******************************************************************************/
/* INTERNAL FUNCTIONS */
/******************************************************************************/

#define DBUG_MSG    0
#define DBUG_ERR    1
#define DBUG_WAR    2

#define DBUG_NRM    "\x1B[0m"
#define DBUG_RED    "\x1B[31m"
#define DBUG_GRN    "\x1B[32m"
#define DBUG_YLW    "\x1B[33m"


#ifdef MUI_DEBUG
#define DEBUG_PRINT(prefix, fmt, ...) \
( prefix == DBUG_MSG ? fprintf(stderr, DBUG_GRN "[debug_message]: " fmt "\n" DBUG_NRM, ##__VA_ARGS__) \
: prefix == DBUG_ERR ? fprintf(stderr, DBUG_RED "[debug_error]: " fmt "\n" DBUG_NRM, ##__VA_ARGS__) \
: prefix == DBUG_WAR ? fprintf(stderr, DBUG_YLW "[debug_warning]: " fmt "\n" DBUG_NRM, ##__VA_ARGS__) \
: 0 )

#else
#define DEBUG_PRINT(prefix, fmt, ...)

#endif



struct xorg_info {
    void           *conn;
    uint32_t        win_id;
    uint32_t        pic_id;
    uint32_t        bg_pix;
    uint32_t        fg_pic;
    uint32_t        gc;
};

struct _xfmt { uint32_t normal, alpha8, argb32; } extern xfmt;


void                   *xorg_attach_internal(struct mui_win*);
void                    xorg_detach_internal(struct mui_win*);
void                    xorg_handle_events(void);

void                   *xorg_get_connection(void);
uint32_t                xorg_get_window(struct mui_win*);

struct xorg_info        xorg_get_info(struct mui_win*);

int                     text_init(void);
void                    text_attach_window(struct mui_text*, struct xorg_info);
void                    text_detach_window(struct mui_text*);
void                    text_draw(struct mui_text*, struct xorg_info);


void                    image_attach_window(struct mui_image*, struct xorg_info);
void                    image_draw(struct mui_image*, struct xorg_info);

void                    group_update(struct mui_group*);
void                    group_attach_window(struct mui_group*, struct mui_win*);


#endif
