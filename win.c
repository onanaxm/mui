#include <stdlib.h>
#include <string.h>

#include "mui.h"


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


    /*
     * for version reason, the current internal type is only X11. Other
     * internal types might be availabe in the future
     */
    xorg_attach_internal(win);

    win->group = mui_create_group();
    group_attach_window(win->group, win);

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
    mui_group_add(win->group, id, data);
}


void
mui_win_remove(struct mui_win *win, unsigned int id, void *data)
{

}
