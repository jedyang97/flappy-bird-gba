#include "mylib.h"
#include <stdlib.h>
#include "text.h"
#include "bird.h"

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
    // TODO add more states here!
            S1,
    S1_NODRAW,
    S2,
    S2_NODRAW,
    S3,
    S3_NODRAW
};

int main() {

    REG_DISPCTL = MODE3 | BG2_ENABLE;

    drawImage3(5, 10, BIRD_WIDTH, BIRD_HEIGHT, bird);
}