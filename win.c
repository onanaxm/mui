#include <stdlib.h>
#include <string.h>

#include "mui.h"

int match_elm(struct mui_element *elm, void *match) 
{
    switch (elm->type) {
        case MUI_ELEMENT_TEXT:
            return elm->data.text == (struct mui_text*)match;
        case MUI_ELEMENT_IMAGE:
            return elm->data.image == (struct mui_image*)match;
    }
    return 0;
}



struct mui_win *
mui_create_window(const char *name, unsigned int width, unsigned int height)
{
    struct mui_win *win = malloc(sizeof(struct mui_win));

    int nsz = strlen(name);
    win->name = malloc(nsz + 1);
    memcpy(win->name, name, nsz);
    win->name[nsz] = '\0';

    win->width = width;
    win->height = height;


    /*
     * initializing events
     */
    win->ev_size = 0;
    TAILQ_INIT(&win->ev_head);
    TAILQ_INIT(&win->elm_head);


    /*
     * for version reason, the current internal type is only X11. Other
     * internal types might be availabe in the future
     */
    win->internal.type = INTERNAL_TYPE_X11;
    xorg_attach_internal(win);

    return win;
}


void
mui_delete_window(struct mui_win *win)
{
    xorg_detach_internal(win);
    free(win->name);
    free(win);
}



void
mui_update()
{
    xorg_handle_events();
}


void
mui_win_add(struct mui_win *win, unsigned int id, void *data)
{
    /*
     * Check if the element we are trying to add is a
     * member of the window
     */

    TAILQ_FOREACH(win->elm_np, &win->elm_head, entries) {
        if (match_elm(win->elm_np, data)) {
            DEBUG_PRINT(DBUG_WAR, "The element you are trying to add to the window is already a member");
            return;
        }
    }

    struct mui_element *elm = malloc(sizeof(struct mui_element));
    struct xorg_info xi = xorg_get_info(win);
    elm->type = id;

    switch (id) {
        case MUI_ELEMENT_TEXT:
            text_attach_window(data, xi);
            elm->data.text = data;
            break;

        case MUI_ELEMENT_IMAGE:
            image_attach_window(data, xi);
            elm->data.image = data;
            break;
    }

    TAILQ_INSERT_TAIL(&win->elm_head, elm, entries);
}


void
mui_win_remove(struct mui_win *win, unsigned int id, void *data)
{

}
