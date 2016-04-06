#include "mylib.h"
#include <stdlib.h>
#include "text.h"
#include "bird.h"
#include "startScreen.h"
#include "pipeNeckBottom.h"
#include "pipeNeckTop.h"
#include "pipeBody.h"

typedef struct {
    int current;
    int showing;
    int col;
    int topHeight;
    int gapHeight;
} PIPE;

typedef struct {
    int row;
    int col;
} BIRD;

// State enum definition
enum GBAState {
    START,
    START_NO_DRAW,
    PRE_PLAY,
    PRE_PLAY_NO_DRAW,
    PLAY,
    GAME_OVER
};

const int birdWidth = BIRD_WIDTH;
const int birdHeight = BIRD_HEIGHT;
const int pipeBodyWidth = PIPEBODY_WIDTH;
const int pipeBodyHeight = PIPEBODY_HEIGHT;
const int pipeNeckWidth = PIPENECKBOTTOM_WIDTH;
const int pipeNeckHeight = PIPENECKBOTTOM_HEIGHT;
const int pipeMargin = 1;

const int delayTime = 1;
const int flyHeight = 5;
const int gravity = 1;
const int pipeSpeed = 1;
const int pipeDistance = 200;

const int numPipes = 10;

void reset(BIRD *bird1, PIPE pipes[]);

void enablePipe(PIPE *pipe);

int detectCollision(BIRD *bird1, PIPE *pipe);

int checkAlive(BIRD *bird, PIPE pipes[]);

void generatePipeHeight(PIPE *pipe);

void drawBird(BIRD *bird1);

void drawPipes(PIPE pipes[]);

void drawPipe(PIPE *pipe);

void applyGravity(BIRD *bird1);

void fly(BIRD *bird1);

void flyLess(BIRD *bird1);

void movePipes(PIPE pipes[]);

int score = 0;

BIRD ourBird;
PIPE pipes[numPipes];
PIPE *currentPipe = pipes;

int main() {

    REG_DISPCTL = MODE3 | BG2_ENABLE;



    enum GBAState state = START;
    int startDownLastFrame = 0;
    int upDownLastFrame = 0;
    int bDownLastFrame = 0;

    while (1) {
        waitForVBlank();
        switch (state) {
            case START:
                reset(&ourBird, pipes);
                state = START_NO_DRAW;
                drawImage3(0, 0, STARTSCREEN_WIDTH, STARTSCREEN_HEIGHT, startScreen);
                drawBird(&ourBird);
                drawPipes(pipes);
                drawString(30, (SCREEN_WIDTH - calcStringWidth("Flappy Bird")) / 2, "Flappy Bird", MAGENTA);
                drawString(50, (SCREEN_WIDTH - calcStringWidth("Press START to start")) / 2, "Press START to start",
                           WHITE);
                break;
            case START_NO_DRAW:
                if (KEY_DOWN_NOW(BUTTON_START) && !startDownLastFrame) {
                    state = PRE_PLAY;
                }
                break;
            case PRE_PLAY:
                state = PRE_PLAY_NO_DRAW;
                if (KEY_DOWN_NOW(BUTTON_B) && !bDownLastFrame) {
                    state = PLAY;
                }
                fillScreen(CYAN);
                drawBird(&ourBird);
                drawPipes(pipes);
                drawString(SCREEN_WIDTH / 2 - 70, SCREEN_HEIGHT / 2, "Press B to release ball", RED);
                break;
            case PRE_PLAY_NO_DRAW:
                if (KEY_DOWN_NOW(BUTTON_B) && !bDownLastFrame) {
                    state = PLAY;
                }
                break;
            case PLAY:
                applyGravity(&ourBird);
                if (KEY_DOWN_NOW(BUTTON_UP)) {
                    if (upDownLastFrame) {
                        ourBird.row -= flyHeight - 2;
                    } else {
                        flyLess(&ourBird);
                    }
                }
                movePipes(pipes);
                fillScreen(CYAN);
                drawBird(&ourBird);
                drawPipes(pipes);
                break;
            case GAME_OVER:
                fillScreen(GREY);
                break;

        }

        if (KEY_DOWN_NOW(BUTTON_START)) {
            startDownLastFrame = 1;
        } else {
            startDownLastFrame = 0;
        }
        if (KEY_DOWN_NOW(BUTTON_UP)) {
            upDownLastFrame = 1;
        } else {
            upDownLastFrame = 0;
        }
        if (KEY_DOWN_NOW(BUTTON_B)) {
            bDownLastFrame = 1;
        } else {
            bDownLastFrame = 0;
        }
    }


}

void reset(BIRD *bird1, PIPE pipes[]) {
    bird1->col = SCREEN_WIDTH / 6;
    bird1->row = SCREEN_HEIGHT / 2 - birdHeight / 2;

    enablePipe(pipes);
    pipes[0].current = 1;
    for (int i = 1; i < numPipes; ++i) {
        pipes[i].showing = 0;
        pipes[i].current = 0;
        generatePipeHeight(pipes + i);
        pipes[i].col = pipes[0].col + i * pipeDistance;
    }
    score = 0;
}

void enablePipe(PIPE *pipe) {
    pipe->showing = 1;
    pipe->col = SCREEN_WIDTH - pipeNeckWidth - 1;
    generatePipeHeight(pipe);
}

int detectCollision(BIRD *bird1, PIPE *pipe) {
    if (bird1->row > (pipe->topHeight - 1) && (bird1->row + birdHeight) < pipe->topHeight + pipe->gapHeight) {
        return 0;
    } else {
        if ((bird1->col + birdWidth) < pipe->col || bird1->col > pipe->col + pipeNeckWidth) {
            return 0;
        } else {
            return 1;
        }
    }
}
//
//int checkAlive(BIRD *bird, PIPE pipes[]) {
//
//}

void generatePipeHeight(PIPE *pipe) {
    int baseHeight = pipeNeckHeight + 10;
    pipe->gapHeight = (birdHeight * 3) + rand() % (birdHeight * 4);
    pipe->topHeight = rand() % (SCREEN_HEIGHT - pipe->gapHeight - baseHeight) + (baseHeight / 2);
}

void drawBird(BIRD *bird1) {
    drawImage3(bird1->row, bird1->col, birdWidth, birdHeight, bird);
}

void drawPipes(PIPE pipes[]) {
    for (int i = 0; i < numPipes; ++i) {
        drawPipe(pipes + i);
    }
}

void drawPipe(PIPE *pipe) {
    if (!pipe->showing) {
        return;
    }
    for (int i = 0; i < pipe->topHeight - pipeNeckHeight; ++i) {
        drawImage3(i, pipe->col + pipeMargin, pipeBodyWidth, pipeBodyHeight, pipeBody);
    }
    drawImage3(pipe->topHeight - pipeNeckHeight, pipe->col, pipeNeckWidth, pipeNeckHeight, pipeNeckTop);
    drawImage3(pipe->topHeight + pipe->gapHeight, pipe->col, pipeNeckWidth, pipeNeckHeight, pipeNeckBottom);
    for (int i = 0; i < SCREEN_HEIGHT - (pipe->topHeight + pipe->gapHeight + pipeNeckHeight); ++i) {
        drawImage3(i + pipe->topHeight + pipe->gapHeight + pipeNeckHeight, pipe->col + pipeMargin, pipeBodyWidth,
                   pipeBodyHeight, pipeBody);
    }
}

void applyGravity(BIRD *bird1) {
    bird1->row += gravity;
}

void fly(BIRD *bird1) {
    bird1->row -= flyHeight;
}

void flyLess(BIRD *bird1) {
    bird1->row -= flyHeight - 3;
}

void movePipes(PIPE pipes[]) {
    for (int i = 0; i < numPipes; ++i) {
        pipes[i].col -= pipeSpeed;
        if (pipes[i].col < SCREEN_WIDTH - pipeNeckWidth && pipes[i].col > 0) {
            pipes[i].showing = 1;
        } else {
            pipes[i].showing = 0;
        }
    }
}