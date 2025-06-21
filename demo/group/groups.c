#include "mui.h"


int running = 1;

void
handle_events(struct mui_win *mw)
{
    struct mui_ev ev;

    while (mui_event_pending(mw)) {
        ev = mui_pop_event(mw);

        switch (ev.type) {
            case MUI_EV_QUIT:
                running = 0;
                break;
        }
    }
}



int
main(void)
{
    struct mui_win *mw;
    struct mui_text *text;
    struct mui_group *group;

    char msg[] =
    "Hello World,\n\n"
    "Bye World";


    mw = mui_create_window("Groups", 320, 240);
    text = mui_create_text(msg, sizeof(msg));
    group = mui_create_group();

    mui_group_add(group, MUI_ELEMENT_TEXT, text);
    mui_win_add(mw, MUI_ELEMENT_GROUP, group);

    while (running) {
        handle_events(mw);
        mui_update();
    }

    mui_delete_window(mw);

    return 0;
}
