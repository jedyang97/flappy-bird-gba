#include "mylib.h"
#include <stdlib.h>
#include "text.h"
#include "bird.h"
#include "startScreen.h"

#define NUMOBJS 7

#define MAXHEIGHT 149

typedef struct {
    int row;
    int col;
    int rdel;
    int cdel;
    int size;
    u16 color;
} MOVOBJ;

typedef struct {
    int row;
    int col;
    int dy;
} BIRD;

// State enum definition
enum GBAState {
    START,
    START_NODRAW,
    PLAY
//    S1,
//    S1_NODRAW,
//    S2,
//    S2_NODRAW,
//    S3,
//    S3_NODRAW
};

int main() {

    REG_DISPCTL = MODE3 | BG2_ENABLE;

    enum GBAState state = START;
    char keyDownLastFrame = 0;

    while (1) {
        waitForVblank();
        switch (state) {
            case START:
                drawImage3(0, 0, STARTSCREEN_WIDTH, STARTSCREEN_HEIGHT, startScreen);
                drawImage3(SCREEN_HEIGHT / 2, SCREEN_WIDTH / 4, BIRD_WIDTH, BIRD_HEIGHT, bird);
                drawString(30, (SCREEN_WIDTH - calcStringWidth("Flappy Bird")) / 2, "Flappy Bird", MAGENTA);
                drawString(50, (SCREEN_WIDTH - calcStringWidth("Press SELECT to start")) / 2, "Press SELECT to start",
                           WHITE);
                state = START_NODRAW;
                break;
            case START_NODRAW:
                if (KEY_DOWN_NOW(BUTTON_SELECT) && !keyDownLastFrame) {
                    state = PLAY;
                }
                break;
            case PLAY:
                fillScreen(CYAN);
                drawImage3(SCREEN_HEIGHT / 2, SCREEN_WIDTH / 4, BIRD_WIDTH, BIRD_HEIGHT, bird);
                break;

        }

        if (KEY_DOWN_NOW(BUTTON_SELECT)) {
            keyDownLastFrame = 1;
        } else {
            keyDownLastFrame = 0;
        }
    }


}