#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mui.h"


int
mui_event_pending(struct mui_win *mw)
{
    return mw->ev_size > 0;
}


void
mui_push_event(struct mui_win *mw, struct mui_ev *ev)
{
    if (mw == NULL)
        return;

    if (mw->ev_size == MUI_WIN_EV_SIZE) {
        struct mui_ev *last = TAILQ_LAST(&mw->ev_head, elisthead);
        TAILQ_REMOVE(&mw->ev_head, last, entries);
        mw->ev_size--;
        free(last);
    }

    struct mui_ev *e = malloc(sizeof(struct mui_ev));
    memcpy(e, ev, sizeof(struct mui_ev));

    TAILQ_INSERT_HEAD(&mw->ev_head, e, entries);
    mw->ev_size++;
}


struct mui_ev
mui_pop_event(struct mui_win *mw)
{
    struct mui_ev ev = { .type = MUI_EV_NONE };
    if (mw->ev_size > 0) {
        struct mui_ev *e = TAILQ_FIRST(&mw->ev_head);

        memcpy(&ev, e, sizeof(struct mui_ev));
        TAILQ_REMOVE(&mw->ev_head, e, entries);
        free(e);

        mw->ev_size--;
    }
    return ev;
}
