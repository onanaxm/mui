#include "mui.h"


int running = 1;

void handle_events(struct mui_win *mw)
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


int main(void)
{
    struct mui_win *mw = mui_create_window("Hello win", 320, 240);

    while (running) {
        handle_events(mw);
        mui_update();
    }

    mui_delete_window(mw);
    return 0;
}
