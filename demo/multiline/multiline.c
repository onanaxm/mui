#include <unistd.h>

#include "mui.h"

int run;

void
handle_events(struct mui_win *mw)
{
    struct mui_ev event;
    while (mui_event_pending(mw)) {
        event = mui_pop_event(mw);

        switch (event.type) {
            case MUI_EV_QUIT:
                run = 0;
                break;

            case MUI_EV_RESIZE:
                break;
        }
    }
}



uint8_t msg[] = 
"Hello,\n\n"
"This is the mui multiline program\n"
"Created by yours truly\n\n"
"Goodbye";


int main(void)
{
    struct mui_win *win = mui_create_window("window", 320, 240);

    struct mui_text *t1 = mui_create_text(msg, sizeof(msg));

    mui_win_add(win, MUI_ELEMENT_TEXT, t1);

    run = 1;

    while (run) {
        handle_events(win);
        mui_update();

        usleep(10000);
    }

    mui_delete_window(win);

    return 0;
}
