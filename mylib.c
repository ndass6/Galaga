// Name: Nathan Dass

#include "mylib.h"
#include "text.h"

u16* videoBuffer = (u16*) 0x6000000;

void setPixel(int x, int y, u16 color) {
	videoBuffer[x * 240 + y] = color;
}

void drawRect(int x, int y, int width, int height, u16 color) {
	for (int h = 0; h <= height; h++)
		for (int w = 0; w <= width; w++)
			setPixel(x + h, y + w, color);
}

//TODO: IMPLEMENT IN O(W+H)
void drawHollowRect(int x, int y, int width, int height, u16 color) {
	for (int h = 0; h <= height; h++) {
		setPixel(x + h, y, color);
		setPixel(x + h, y + width, color);
	}
	for (int w = 0; w <= width; w++) {
		setPixel(x, y + w, color);
		setPixel(x + height, y + w, color);
	}
}

void drawImage3(int x, int y, int width, int height, const u16* image) {
	int size = 0;
	for (int h = 0; h < height; h++)
		for (int w = 0; w < width; w++)
			setPixel(x + h, y + w, image[size++]);
}

void clearScreen() {
	drawRect(0, 0, 240, 160, 0x00000);
}

void drawChar(int x, int y, char ch, u16 color) {
	for (int h = 0; h < 8; h++)
		for (int w = 0; w < 6; w++)
			if (fontdata_6x8[h * 6 + w + 48 * ch])
				setPixel(x + h, y + w, color);
}


void drawString(int x, int y, char *str, u16 color) {
	while (*str) {
        drawChar(x, y, *str++, color);
        y += 6;
    }
}

void waitForVBlank() {
	while(SCANLINECOUNTER > 160);
    while(SCANLINECOUNTER < 160);
}