#include <stdlib.h>

#include "mui.h"

int match_elm(struct mui_element *elm, void *match) 
{
    switch (elm->type) {
        case MUI_ELEMENT_TEXT:
            return elm->data.text == (struct mui_text*)match;
    }
    return 0;
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
    }

    TAILQ_INSERT_TAIL(&win->elm_head, elm, entries);
}


void
mui_win_remove(struct mui_win *win, unsigned int id, void *data)
{

}
