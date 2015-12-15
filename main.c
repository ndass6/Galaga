// Name: Nathan Dass

#include "spaceship.h"
#include "enemy1.h"
#include "enemy2.h"
#include "start.h"
#include "gameover.h"
#include "mylib.h"
#include "levels.h"
#include "text.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define REG_DISPCNT *(u16*) 0x4000000
#define MODE_3 3
#define BG2_EN (1 << 10)
#define RGB(red, green, blue) blue << 10 | green << 5 | red;
#define INT_ARRAY_LENGTH(arr) ((int) sizeof(arr) / sizeof(int));

enum {
	START_GAME, PLAY_GAME, LOSE_LIFE, GAME_OVER, WON_GAME
};

typedef struct Spaceship {
	int x;
	int y;
	int oldX;
	int oldY;
	int width;
	int height;
} Spaceship;

typedef struct Enemy {
	int x;
	int y;
	int oldX;
	int oldY;
	int originalX;
	int originalY;
	int direction;
	int attack;
	int type;
	int width;
	int height;
} Enemy;

typedef struct Laser {
	int x;
	int y;
	int oldX;
	int oldY;
	int width;
	int height;
} Laser;

Spaceship player;

Enemy enemies[20];
int enemyPos;

Laser lasers[10];
int laserPos;

int buttonA;
int lives;
int state;
int level;
char filename[50];

void startScreen();
void playGame();
void checkKeys();
void updatePlayer();
void updateEnemies();
void updateLasers();
void loseLife();
void lostGame();
void wonGame();
void getEnemies();
void newLevel();
void resetPlayer();
void resetEnemies();

int main(void) {
	REG_DISPCNT = MODE_3 | BG2_EN;
	srand(100);

	enemyPos = 0;
	laserPos = 0;

	buttonA = 0;
	lives = INITIAL_LIVES;
	level = 1;

	state = START_GAME;
	while(1) {
		while (KEY_PRESSED(BUTTON_SELECT)) {
			state = START_GAME;
		} switch (state) {
			case START_GAME:
				startScreen();
				clearScreen();
				break;
			case PLAY_GAME:
				playGame();
				break;
			case LOSE_LIFE:
				loseLife();
				break;
			case GAME_OVER:
				lostGame();
				break;
			case WON_GAME:
				wonGame();
				break;
		}
	}
	return 0;
}

void startScreen() {
	drawImage3(0, 0, 240, 160, start);
	while (!KEY_PRESSED(BUTTON_START));
	state = PLAY_GAME;

	player.x = INITIAL_PLAYER_X;
	player.y = INITIAL_PLAYER_Y;
	player.oldX = player.x;
	player.oldY = player.y;
	player.width = SPACESHIP_WIDTH;
	player.height = SPACESHIP_HEIGHT;
	lives = INITIAL_LIVES;
	level = 1;
	enemyPos = 0;
	laserPos = 0;
	getEnemies();
}

void playGame() {
	checkKeys();
	updatePlayer();
	updateEnemies();
	updateLasers();
	
	char buffer[50];
	if (enemyPos == 1)
		sprintf(buffer, "%d enemy left.", enemyPos);
	else
		sprintf(buffer, "%d enemies left.", enemyPos);
	drawString(150, 35, buffer, 0x0107d);
	if (lives == 1)
		sprintf(buffer, "%d life left.", lives);
	else
		sprintf(buffer, "%d lives left.", lives);
	drawString(150, 135, buffer, 0x0107d);
}

void checkKeys() {
	player.oldX = player.x;
	player.oldY = player.y;
	if (KEY_PRESSED(BUTTON_LEFT))
		player.x = player.x - 1;
	if (KEY_PRESSED(BUTTON_RIGHT))
		player.x = player.x + 1;
	if (KEY_PRESSED(BUTTON_UP))
		player.y = player.y - 1;
	if (KEY_PRESSED(BUTTON_DOWN))
		player.y = player.y + 1;
	if (buttonA == 0 && KEY_PRESSED(BUTTON_A) && laserPos < 10) {
		Laser newLaser;
		newLaser.x = player.x + player.width / 2;
		newLaser.y = player.y;
		newLaser.oldX = newLaser.x;
		newLaser.oldY = newLaser.y;
		newLaser.width = 2;
		newLaser.height = 4;
		lasers[laserPos++] = newLaser;
		buttonA = 1;
	} else if (buttonA == 1 && !KEY_PRESSED(BUTTON_A))
		buttonA = 0;
}

void updatePlayer() {
	if (player.y < 0)
		player.y = 0;
	if (player.y + player.width > 148)
		player.y = 148 - player.width;
	if (player.x < 0)
		player.x = 0;
	if (player.x + player.height > 240)
		player.x = 240 - player.height;

	waitForVBlank();
	drawRect(player.oldY, player.oldX, player.width, player.height, 0x00000);
	drawImage3(player.y, player.x, player.width, player.height, spaceship);
}

void updateEnemies() {
	int collision = 0;
	for (int i = 0; i < enemyPos; i++) {
		if (i % 4 == 0)
			waitForVBlank();
		if (enemies[i].attack == 0)
			if (rand() % 1000 < 2)
				enemies[i].attack = 5;
		if (enemies[i].attack != 0)
			enemies[i].y += enemies[i].attack;
		if (enemies[i].y + enemies[i].width >= 148) {
			enemies[i].attack *= -1;
			enemies[i].y = 148 - enemies[i].width;
		}
		if (enemies[i].y <= enemies[i].originalY) {
			enemies[i].attack = 0;
			enemies[i].y = enemies[i].originalY;
		}
		drawRect(enemies[i].oldY, enemies[i].oldX, enemies[i].width, enemies[i].height, 0x00000);
		if (enemies[i].type == 1)
			drawImage3(enemies[i].y, enemies[i].x, enemies[i].width, enemies[i].height, enemy1);
		else if (enemies[i].type == 2)
			drawImage3(enemies[i].y, enemies[i].x, enemies[i].width, enemies[i].height, enemy2);
		
		enemies[i].oldX = enemies[i].x;
		enemies[i].oldY = enemies[i].y;
		if (enemies[i].x - enemies[i].originalX > 35 || enemies[i].x <= 0)
			enemies[i].direction *= -1;
		else if (enemies[i].x - enemies[i].originalX < -35 || enemies[i].x >= 240)
			enemies[i].direction *= -1;

		enemies[i].x += enemies[i].direction;

		if (enemies[i].y < 0)
			enemies[i].y = 0;
		if (enemies[i].y + enemies[i].width > 148)
			enemies[i].y = 148 - enemies[i].width;
		if (enemies[i].x < 0)
			enemies[i].x = 0;
		if (enemies[i].x + enemies[i].height > 240)
			enemies[i].x = 240 - enemies[i].height;

		if ((player.x <= enemies[i].x + enemies[i].height &&
				player.x >= enemies[i].x &&
				player.y <= enemies[i].y + enemies[i].width &&
				player.y >= enemies[i].y) ||
				(enemies[i].x <= player.x + player.height &&
				enemies[i].x >= player.x &&
				enemies[i].y <= player.y + player.width &&
				enemies[i].y >= player.y)) {
			drawRect(enemies[i].y, enemies[i].x, enemies[i].width, enemies[i].height, 0x00000);
			drawRect(player.y, player.x, player.width, player.height, 0x00000);
			for (int k = i; k < enemyPos - 1; k++)
				enemies[k] = enemies[k + 1];
			enemyPos--;
			drawRect(150, 0, 240, 100, 0x00000);
			lives--;
			if (lives > 0)
				state = LOSE_LIFE;
			else
				state = GAME_OVER;
			i--;
			collision = 1;
		}
		if (enemyPos == 0 && lives > 0) {
			clearScreen();
			state = WON_GAME;
		}
	}
}

void updateLasers() {
	for (int i = 0; i < laserPos; i++) {
		int collision = 0;
		drawRect(lasers[i].oldY, lasers[i].oldX, lasers[i].width, lasers[i].height, 0x00000);
		drawRect(lasers[i].y, lasers[i].x, lasers[i].width, lasers[i].height, 0x0107d);
		lasers[i].oldX = lasers[i].x;
		lasers[i].oldY = lasers[i].y;
		lasers[i].y -= 3;
		if (lasers[i].y < 0) {
			drawRect(lasers[i].oldY, lasers[i].oldX, lasers[i].width, lasers[i].height, 0x00000);
			for (int j = i; j < laserPos - 1; j++)
				lasers[j] = lasers[j + 1];
			laserPos--;
			i--;
		}

		for (int j = 0; j < enemyPos; j++) {
			if ((lasers[i].x <= enemies[j].x + enemies[j].height &&
					lasers[i].x >= enemies[j].x &&
					lasers[i].y <= enemies[j].y + enemies[j].width &&
					lasers[i].y >= enemies[j].y) ||
					(enemies[j].x <= lasers[i].x + lasers[i].height &&
					enemies[j].x >= lasers[i].x &&
					enemies[j].y <= lasers[i].y + lasers[i].width &&
					enemies[j].y >= lasers[i].y)) {
				drawRect(enemies[j].oldY, enemies[j].oldX, enemies[j].width, enemies[j].height, 0x00000);
				drawRect(lasers[i].oldY, lasers[i].oldX, lasers[i].width, lasers[i].height, 0x00000);
				for (int k = i; k < laserPos - 1; k++)
					lasers[k] = lasers[k + 1];
				for (int k = j; k < enemyPos - 1; k++)
					enemies[k] = enemies[k + 1];
				enemyPos--;
				drawRect(150, 0, 240, 100, 0x00000);
				laserPos--;
				i--;
				collision = 1;
			}
		}
		if (enemyPos == 0) {
			clearScreen();
			laserPos = 0;
			enemyPos = 0;
			state = WON_GAME;
		}
	}
}

void loseLife() {
	char buffer[50];
	if (lives > 1)
		sprintf(buffer, "%d lives left.", lives);
	else
		sprintf(buffer, "%d life left.", lives);
	drawString(50, 75, buffer, 0x0107d);
	if (KEY_PRESSED(BUTTON_START)) {
		clearScreen();
		laserPos = 0;
		state = PLAY_GAME;
		resetPlayer();
		resetEnemies();
	}
}

void lostGame() {
	drawImage3(0, 0, 240, 160, gameover);
	if (KEY_PRESSED(BUTTON_START)) {
		state = START_GAME;
		level = 1;
		laserPos = 0;
		enemyPos = 0;
		lives = INITIAL_LIVES;
		clearScreen();
		newLevel();
		getEnemies();
	}
}

void wonGame() {
	drawString(50, 85, "YOU WON!!!", 0x0107d);
	if (level + 1 <= HIGHEST_LEVEL) {
		drawString(75, 30, "PRESS START TO GO TO NEXT LEVEL", 0x003df);
	} else {
		drawString(75, 45, "PRESS START TO PLAY AGAIN", 0x003df);
	}
	if (KEY_PRESSED(BUTTON_START)) {
		level++;
		if (level <= HIGHEST_LEVEL)
			state = PLAY_GAME;
		else {
			state = START_GAME;
			level = 1;
		}
		clearScreen();
		newLevel();
		getEnemies();
	}
}

void getEnemies() {
	if (level == 1) {
		for (int i = 0; i < ((int) (sizeof(level1) / sizeof(int))); i += 4) {
			Enemy newEnemy;
			newEnemy.x = level1[i + 0];
			newEnemy.y = level1[i + 1];
			newEnemy.oldX = newEnemy.x;
			newEnemy.oldY = newEnemy.y;
			newEnemy.originalX = newEnemy.x;
			newEnemy.originalY = newEnemy.y;
			newEnemy.attack = 0;
			newEnemy.direction = level1[i + 2];
			newEnemy.type = level1[i + 3];

			if (newEnemy.type == 1) {
				newEnemy.height = ENEMY1_HEIGHT;
				newEnemy.width = ENEMY1_WIDTH;
			} else if (newEnemy.type == 2) {
				newEnemy.height = ENEMY2_HEIGHT;
				newEnemy.width = ENEMY2_WIDTH;
			}
			enemies[enemyPos++] = newEnemy;
		}
	} else if (level == 2) {
		for (int i = 0; i < ((int) (sizeof(level2) / sizeof(int))); i += 4) {
			Enemy newEnemy;
			newEnemy.x = level2[i + 0];
			newEnemy.y = level2[i + 1];
			newEnemy.oldX = newEnemy.x;
			newEnemy.oldY = newEnemy.y;
			newEnemy.originalX = newEnemy.x;
			newEnemy.originalY = newEnemy.y;
			newEnemy.attack = 0;
			newEnemy.direction = level2[i + 2];
			newEnemy.type = level2[i + 3];
			if (newEnemy.type == 1) {
				newEnemy.height = ENEMY1_HEIGHT;
				newEnemy.width = ENEMY1_WIDTH;
			} else if (newEnemy.type == 2) {
				newEnemy.height = ENEMY2_HEIGHT;
				newEnemy.width = ENEMY2_WIDTH;
			}
			enemies[enemyPos++] = newEnemy;
		}
	} else if (level == 3) {
		for (int i = 0; i < ((int) (sizeof(level3) / sizeof(int))); i += 4) {
			Enemy newEnemy;
			newEnemy.x = level3[i + 0];
			newEnemy.y = level3[i + 1];
			newEnemy.oldX = newEnemy.x;
			newEnemy.oldY = newEnemy.y;
			newEnemy.originalX = newEnemy.x;
			newEnemy.originalY = newEnemy.y;
			newEnemy.attack = 0;
			newEnemy.direction = level3[i + 2];
			newEnemy.type = level3[i + 3];
			if (newEnemy.type == 1) {
				newEnemy.height = ENEMY1_HEIGHT;
				newEnemy.width = ENEMY1_WIDTH;
			} else if (newEnemy.type == 2) {
				newEnemy.height = ENEMY2_HEIGHT;
				newEnemy.width = ENEMY2_WIDTH;
			}
			enemies[enemyPos++] = newEnemy;
		}
	}
}

void newLevel() {
	resetPlayer();
	enemyPos = 0;
	laserPos = 0;
}

void resetPlayer() {
	player.x = INITIAL_PLAYER_X;
	player.y = INITIAL_PLAYER_Y;
}

void resetEnemies() {
	for (int i = 0; i < enemyPos; i++) {
		enemies[i].y = enemies[i].originalY;
		enemies[i].attack = 0;
	}
}