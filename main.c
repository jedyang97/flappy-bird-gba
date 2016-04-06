#include "mylib.h"
#include <stdlib.h>
#include "text.h"
#include "bird.h"
#include "startScreen.h"
#include "pipeNeckBottom.h"
#include "pipeNeckTop.h"
#include "pipeBody.h"

typedef struct {
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
const int pipeDistance = 200;

const int numPipes = 2;

void reset();

void reGeneratePipes();

void enablePipe(PIPE *pipe);

int detectCollision(BIRD *bird1, PIPE *pipe);

int checkAlive();

void generatePipeHeight(PIPE *pipe);

void drawBird();

void drawPipes();

void drawPipe(PIPE *pipe);

void applyGravity();

void fly();

void flyLess();

void movePipes();

int score = 0;

BIRD ourBird;
PIPE *pipes;
PIPE *currentPipe;
PIPE *nextPipe;

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
                drawBird();
                drawPipes();
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
                if (KEY_DOWN_NOW(BUTTON_UP) && !upDownLastFrame) {
                    state = PLAY;
                }
                fillScreen(CYAN);
                drawBird();
                drawPipes();
                drawString(SCREEN_WIDTH / 2 - 70, SCREEN_HEIGHT / 2, "Press UP to begin", RED);
                break;
            case PRE_PLAY_NO_DRAW:
                if (KEY_DOWN_NOW(BUTTON_UP) && !upDownLastFrame) {
                    state = PLAY;
                }
                break;
            case PLAY:
                movePipes();
                applyGravity(&ourBird);
                if (KEY_DOWN_NOW(BUTTON_UP)) {
                    if (upDownLastFrame) {
                        ourBird.row -= flyHeight - 2;
                    } else {
                        flyLess(&ourBird);
                    }
                }

                if (!checkAlive()) {
                    state = GAME_OVER;
                    break;
                }
                fillScreen(CYAN);
                drawBird();
                drawPipes();
                break;
            case GAME_OVER:
                fillScreen(GREY);
                state = GAME_OVER_NO_DRAW;
                break;
            case GAME_OVER_NO_DRAW:
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

void reset() {
    pipes = malloc(sizeof(PIPE) * numPipes);
    currentPipe = pipes;

    reGeneratePipes();

    ourBird.col = SCREEN_WIDTH / 6;
    ourBird.row = SCREEN_HEIGHT / 2 - birdHeight / 2;


    score = 0;
}

void reGeneratePipes() {
    enablePipe(pipes);
    for (int i = 1; i < numPipes; ++i) {
        pipes[i].showing = 0;
        generatePipeHeight(pipes + i);
        pipes[i].col = pipes[0].col + i * pipeDistance;
    }
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

int checkAlive() {
    return !detectCollision(&ourBird, currentPipe) && ourBird.row < SCREEN_HEIGHT;
}

void generatePipeHeight(PIPE *pipe) {
    int baseHeight = pipeNeckHeight + 10;
    pipe->gapHeight = (birdHeight * 3) + rand() % (birdHeight * 4);
    pipe->topHeight = rand() % (SCREEN_HEIGHT - pipe->gapHeight - baseHeight) + (baseHeight / 2);
}

void drawBird() {
    drawImage3(ourBird.row, ourBird.col, birdWidth, birdHeight, bird);
}

void drawPipes() {
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

void applyGravity() {
    ourBird.row += gravity;
}

void fly() {
    ourBird.row -= flyHeight;
}

void flyLess() {
    ourBird.row -= flyHeight - 3;
}

void movePipes() {
    if (nextPipe != NULL) {
        free(pipes);
        pipes = nextPipe;
        currentPipe = nextPipe;
        reGeneratePipes();
    }
    for (int i = 0; i < numPipes; ++i) {
        pipes[i].col -= pipeSpeed;
        if (pipes[i].col < SCREEN_WIDTH - pipeNeckWidth && pipes[i].col > 0) {
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
            currentPipe = pipes + i;
            score++;

        }
    }

}