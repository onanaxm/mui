#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "mui.h"


#define WIDTH   200
#define HEIGHT  200

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


char *
load_fish(int id)
{
    FILE *fp;
    long  fsize;
    char *buffer = malloc(WIDTH * HEIGHT * 4);

    if (id == 0) {
        if ((fp = fopen("fish.raw", "rb")) == NULL) {
            printf("Failed to open fish.raw\n");
            exit(1);
        }
    } else {
        if ((fp = fopen("flag.raw", "rb")) == NULL) {
            printf("Failed to open fish.raw\n");
            exit(1);
        }
    }
    fseek(fp, 0, SEEK_END);
    fsize = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    int c = 0;
    for (int i = 0; i < (WIDTH * HEIGHT * 4) && c != EOF; i++) {
        c = fgetc(fp);
        buffer[i] = c;
    }

    fgets(buffer, fsize, fp);

    return buffer;
}

int main(int argc, char *argv[])
{
    if (argc != 2) {
        printf("Run: `./bin 0` or `./bin 1`...\n");
        return 1;
    }

    uint8_t *buffer;
    if (strcmp(argv[1], "0") == 0) {
        buffer = load_fish(0);
    } else if (strcmp(argv[1], "1") == 0) {
        buffer = load_fish(1);
    } else {
        printf("Run: `./bin 0` or `./bin 1`...\n");
        return 1;
    }


    struct mui_win *mw = mui_create_window("image", WIDTH, HEIGHT);

    struct mui_image *img = mui_create_image(buffer, WIDTH, HEIGHT);

    mui_win_add(mw, MUI_ELEMENT_IMAGE, img);

    while (running) {
        handle_events(mw);
        mui_update();

        usleep(10000);
    }

}
