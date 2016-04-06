#include "mylib.h"

unsigned short *videoBuffer = (unsigned short *) 0x6000000;

void setPixel(int r, int c, unsigned short color) {
    videoBuffer[OFFSET(r, c, 240)] = color;
}

void drawRect(int row, int col, int height, int width, unsigned short color) {
    for (int r = 0; r < height; r++) {

        DMA[3].src = &color;
        DMA[3].dst = &videoBuffer[OFFSET(row + r, col, 240)];
        DMA[3].cnt = width | DMA_ON | DMA_SOURCE_FIXED;
    }
}

void delay(int n) {
    volatile int x = 0;
    for (int i = 0; i < n * 10000; i++) {
        x++;
    }
}

void waitForVblank() {
    while (SCANLINECOUNTER > 160);
    while (SCANLINECOUNTER < 160);
}

void fillScreen(volatile u16 color) {
    DMA[3].src = &color;
    DMA[3].dst = videoBuffer;
    DMA[3].cnt = 38400 | DMA_ON | DMA_SOURCE_FIXED;
}