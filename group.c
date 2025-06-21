#include "mui.h"

#include <stdlib.h>


/***************************************/
/* UTILITY FUNCTIONS */
/***************************************/
static int
match_elm(struct mui_element *elm, void *match)
{
    switch (elm->type) {
        case MUI_ELEMENT_TEXT:
            return elm->data.text == (struct mui_text *)match;
        case MUI_ELEMENT_IMAGE:
            return elm->data.image == (struct mui_image *)match;
        case MUI_ELEMENT_GROUP:
            return elm->data.group == (struct mui_group *)match;
    }
    return 0;
}




/***************************************/
/* INTERNAL FUNCTIONS */
/***************************************/

/*
 * TODO:
 *   Complete this function
 */
void
group_attach_window(struct mui_group *group, struct mui_win *mw)
{
    struct xorg_info xi = xorg_get_info(mw);
    group->mw = mw;

    TAILQ_FOREACH(group->mem_np, &group->mem_head, entries) {
        struct mui_element *e = group->mem_np;

        switch(e->type) {
            case MUI_ELEMENT_TEXT:
                text_attach_window(e->data.text, xi);
                break;
            case MUI_ELEMENT_IMAGE:
                image_attach_window(e->data.image, xi);
                break;

            case MUI_ELEMENT_GROUP:
                group_attach_window(e->data.group, mw);
                break;
        }
    }

}

void
group_update(struct mui_group *group)
{
    struct xorg_info xi = xorg_get_info(group->mw);

    TAILQ_FOREACH(group->mem_np, &group->mem_head, entries) {
        struct mui_element *e = group->mem_np;

        switch (e->type) {
            case MUI_ELEMENT_TEXT: {
                struct mui_text *mtext = e->data.text;
                text_draw(mtext, xi);
                break;
            }

            case MUI_ELEMENT_IMAGE: {
                struct mui_image *mimg = e->data.image;
                image_draw(mimg, xi);
                break;
            }

            case MUI_ELEMENT_GROUP:
                group_update(e->data.group);
                break;

        }
    }
}


/***************************************/
/* EXTERNAL FUNCTIONS */
/***************************************/
struct mui_group*
mui_create_group()
{
    struct mui_group *group;

    group = malloc(sizeof(*group));
    group->mw = NULL;

    TAILQ_INIT(&group->mem_head);


    return group;
}


void
mui_delete_group(struct mui_group *group)
{

}


void
mui_group_add(struct mui_group *group, unsigned int id, void *data)
{
    /*
     * Check if the element is in group
     */

    TAILQ_FOREACH(group->mem_np, &group->mem_head, entries) {
        if (match_elm(group->mem_np, data))  {
            DEBUG_PRINT(DBUG_WAR, "The element is already a group memeber. Skipping...");
            return;
        }
    }

    struct mui_element *elm = malloc(sizeof(struct mui_element));
    struct xorg_info xi;
    if (group->mw)
        xi =  xorg_get_info(group->mw);
    elm->type = id;

    switch (id) {
        case MUI_ELEMENT_TEXT:
            if (group->mw)
                text_attach_window(data, xi);
            elm->data.text = data;
            break;

        case MUI_ELEMENT_IMAGE:
            if (group->mw)
                image_attach_window(data, xi);
            elm->data.image = data;
            break;

        case MUI_ELEMENT_GROUP:
            if (group->mw)
                group_attach_window(data, group->mw);
            elm->data.group = data;
            break;
    }

    TAILQ_INSERT_TAIL(&group->mem_head, elm, entries);
}


void
mui_group_remove(struct mui_group *group, unsigned int id, void *element)
{


}


