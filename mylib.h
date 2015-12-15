// Name: Nathan Dass

typedef unsigned short u16;

extern u16* videoBuffer;

#define INITIAL_LIVES		3
#define INITIAL_PLAYER_X	100
#define INITIAL_PLAYER_Y	100
#define ENEMY_LIMIT			35

void setPixel(int x, int y, u16 color);
void drawRect(int x, int y, int width, int height, u16 color);
void drawHollowRect(int x, int y, int width, int height, u16 color);
void drawImage3(int x, int y, int width, int height, const u16* image);
void clearScreen();

void drawChar(int row, int col, char ch, unsigned short color);
void drawString(int row, int col, char *str, unsigned short color);

#define BUTTONS 			*(volatile unsigned int*) 0x4000130
#define BUTTON_A			(1 << 0)	// Z
#define BUTTON_B			(1 << 1)	// X
#define BUTTON_SELECT		(1 << 2)	// Backspace
#define BUTTON_START		(1 << 3)	// Enter
#define BUTTON_RIGHT		(1 << 4)	// Right Arrow
#define BUTTON_LEFT			(1 << 5)	// Left Arrow
#define BUTTON_UP			(1 << 6)	// Up Arrow
#define BUTTON_DOWN			(1 << 7)	// Down Arrow
#define BUTTON_R			(1 << 8)	// S
#define BUTTON_L			(1 << 9)	// A
#define KEY_PRESSED(key)	(~(BUTTONS) & key)

#define SCANLINECOUNTER 	*(volatile unsigned short*) 0x4000006
void waitForVBlank();