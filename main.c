#include "mylib.h"
#include <stdlib.h>
#include <stdio.h>
#include "text.h"
#include "bird.h"
#include "startScreen.h"
#include "pipeNeckBottom.h"
#include "pipeNeckTop.h"
#include "pipeBody.h"
#include "gameoverScreen.h"
#include "background.h"

typedef struct {
    volatile int showing;
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
    PLAY,
    GAME_OVER,
    GAME_OVER_NO_DRAW
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
const int pipeDistance = 100;

const int numPipes = 5;

void reset();

void reGeneratePipes();

void enablePipe(PIPE *pipe);

void generatePipeHeight(PIPE *pipe);

void drawBird();

void undrawBird(const u16 *image);

void drawPipe(PIPE *pipe);

void undrawPipeRear(PIPE *pipe, const u16 *image);

void drawPipes();

void undrawPipes();

void applyGravity();


void movePipes();

void fly();

void flyLess();

int detectCollision(BIRD *bird1, PIPE *pipe);

int checkAlive();

void drawBackground(const u16 *image);


volatile int score = 0;

BIRD ourBird;
PIPE *pipes;
PIPE *currentPipe;
PIPE *nextPipe;

// main function, containing the main loop
int main() {

    REG_DISPCTL = MODE3 | BG2_ENABLE;

    enum GBAState state = START;

    // tokens to keep track of keys
    int startDownLastFrame = 0;
    int upDownLastFrame = 0;
    int selectDownLastFrame = 0;

    // main loop
    while (1) {
        waitForVBlank();
        switch (state) {
            case START:
                reset();
                state = START_NO_DRAW;
                drawBackground(startScreen);
                drawBird();
                drawPipes();
                drawString(50, (SCREEN_WIDTH - calcStringWidth("Press UP to start")) / 2, "Press UP to start",
                           WHITE);
                break;
            case START_NO_DRAW:
                if (KEY_DOWN_NOW(BUTTON_UP) && !upDownLastFrame) {
                    drawBackground(background);
                    state = PLAY;
                }
                break;
            case PLAY:
                undrawBird(background);
                undrawPipes();
                movePipes();
                applyGravity();
                if (KEY_DOWN_NOW(BUTTON_UP)) {
                    if (upDownLastFrame) {
                        ourBird.row -= flyHeight - 2;
                    } else {
                        flyLess();
                    }
                }

                if (!checkAlive() && !KEY_DOWN_NOW(BUTTON_A)) {
                    state = GAME_OVER;
                    break;
                }

                drawBird();
                drawPipes();
                break;
            case GAME_OVER:
                drawBackground(gameoverScreen);
                char string[5];
                sprintf(string, "%d", score);
                drawString(68, 150, string, WHITE);
                drawString(150, (SCREEN_WIDTH - calcStringWidth("Press START to restart")) / 2,
                           "Press START to restart",
                           WHITE);
                state = GAME_OVER_NO_DRAW;
                break;
            case GAME_OVER_NO_DRAW:
                if (KEY_DOWN_NOW(BUTTON_START) && !startDownLastFrame) {
                    state = START;
                }
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
        if (KEY_DOWN_NOW(BUTTON_SELECT)) {
            selectDownLastFrame = 1;
        } else {
            selectDownLastFrame = 0;
        }
    }


}

// reset the status of the game
void reset() {
    pipes = malloc(sizeof(PIPE) * numPipes);
    currentPipe = pipes;

    reGeneratePipes();

    ourBird.col = SCREEN_WIDTH / 6;
    ourBird.row = SCREEN_HEIGHT / 2 - birdHeight / 2;

    score = 0;
}

// initiate pipe data
void reGeneratePipes() {
    enablePipe(pipes);
    for (int i = 1; i < numPipes; ++i) {
        pipes[i].showing = 0;
        generatePipeHeight(pipes + i);
        pipes[i].col = pipes[0].col + i * pipeDistance;
    }
}

// make a pipe visible
void enablePipe(PIPE *pipe) {
    pipe->showing = 1;
    pipe->col = SCREEN_WIDTH - pipeNeckWidth - 1;
    generatePipeHeight(pipe);
}

// detect collision between bird and pipe
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

// check if we survived
int checkAlive() {
    return !detectCollision(&ourBird, currentPipe) && ourBird.row < SCREEN_HEIGHT;
}

// randomize a pipe
void generatePipeHeight(PIPE *pipe) {
    int baseHeight = pipeNeckHeight + 10;
    pipe->gapHeight = (birdHeight * 3) + rand() % (birdHeight * 4);
    pipe->topHeight = rand() % (SCREEN_HEIGHT - pipe->gapHeight - baseHeight) + (baseHeight / 2);
}

// draw the bird
void drawBird() {
    drawImage3(ourBird.row, ourBird.col, birdWidth, birdHeight, bird);
}

// undraw the bird
void undrawBird(const u16 *image) {
    undrawImage3(ourBird.row, ourBird.col, birdWidth, birdHeight, image);
}

void drawPipes() {
    for (int i = 0; i < numPipes; ++i) {
        drawPipe(pipes + i);
    }
}

// draw one pipe, behaves differently when near edges
void drawPipe(PIPE *pipe) {
    if (!pipe->showing) {
        return;
    }
    if (pipe->col < 0) {
        for (int i = 0; i < pipe->topHeight - pipeNeckHeight; ++i) {
            drawImage3FromCol(i, -1, -pipe->col - pipeMargin, pipeBodyWidth + 1, pipeBodyHeight,
                              pipeBody);
        }
        drawImage3FromCol(pipe->topHeight - pipeNeckHeight, 0, -pipe->col, pipeNeckWidth, pipeNeckHeight,
                          pipeNeckTop);
        drawImage3FromCol(pipe->topHeight + pipe->gapHeight, 0, -pipe->col, pipeNeckWidth, pipeNeckHeight,
                          pipeNeckBottom);
        for (int i = 0; i < SCREEN_HEIGHT - (pipe->topHeight + pipe->gapHeight + pipeNeckHeight); ++i) {
            drawImage3FromCol(i + pipe->topHeight + pipe->gapHeight + pipeNeckHeight, -1, -pipe->col - pipeMargin,
                              pipeBodyWidth + 1, pipeBodyHeight, pipeBody);
        }
    } else {
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

}

// undraw all pipes
void undrawPipes() {
    for (int i = 0; i < numPipes; ++i) {
        undrawPipeRear(pipes + i, background);
    }
}

// undraw the rear part of a pipe
void undrawPipeRear(PIPE *pipe, const u16 *image) {
    if (!pipe->showing) {
        return;
    }
    for (int i = 0; i < pipe->topHeight - pipeNeckHeight; ++i) {
        undrawImage3(i, pipe->col + pipeMargin + pipeBodyWidth - 1, pipeSpeed + 1, pipeBodyHeight, image);
    }
    undrawImage3(pipe->topHeight - pipeNeckHeight, pipe->col + pipeNeckWidth - 1, pipeSpeed, pipeNeckHeight, image);
    undrawImage3(pipe->topHeight + pipe->gapHeight, pipe->col + pipeNeckWidth - 1, pipeSpeed, pipeNeckHeight, image);
    for (int i = 0; i < SCREEN_HEIGHT - (pipe->topHeight + pipe->gapHeight + pipeNeckHeight); ++i) {
        undrawImage3(i + pipe->topHeight + pipe->gapHeight + pipeNeckHeight, pipe->col + pipeMargin + pipeBodyWidth - 1,
                     pipeSpeed + 1, pipeBodyHeight, image);
    }
}

void applyGravity() {
    ourBird.row += gravity;
}

void fly() {
    ourBird.row -= flyHeight;
}

void flyLess() {
    ourBird.row -= flyHeight - 3;
}

// moves the pipes as game proceeds
void movePipes() {
    if (nextPipe != NULL && currentPipe->col + pipeNeckWidth < 0) {
        free(pipes);
        pipes = nextPipe;
        currentPipe = nextPipe;
        reGeneratePipes();
        nextPipe = NULL;
    }
    for (int i = 0; i < numPipes; ++i) {
        pipes[i].col -= pipeSpeed;
        if (pipes[i].col < SCREEN_WIDTH - pipeNeckWidth && pipes[i].col + pipeNeckWidth > 0) {
            pipes[i].showing = 1;
        } else {
            pipes[i].showing = 0;
        }
        if (pipes[i].col <= ourBird.col + birdWidth) {
            if (i == numPipes - 1) {
                nextPipe = malloc(sizeof(PIPE) * numPipes);
            } else {
                nextPipe = NULL;
            }
            if (currentPipe != pipes + i) {
                score++;
            }
            currentPipe = pipes + i;
        }
    }

}

