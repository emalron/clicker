#include <Windows.h>
#include <stdbool.h>
#include <stdio.h>
#include <conio.h>

#define SCR_WIDTH 40
#define SCR_HEIGHT 30
#define MAX_BULLETS 30

typedef struct {
	int state_;
	int width_;
	int height_;
	int pos_x_;
	int pos_y_;
	char* image_;
} Object, *pObject;

Object player;
Object enemy;
// Object* p_bullet_array[MAX_BULLETS];
pObject p_bullet_array[MAX_BULLETS];
// Object* p_bullet = NULL;
int gold = 0;
int ammo = 1;
int cost = 10;

char front_buffer[SCR_HEIGHT][SCR_WIDTH];
char back_buffer[SCR_HEIGHT][SCR_WIDTH];

void drawToBackBuffer(const int i, const int j, const char* image) {
	int ix = 0;
	while (1) {
		if (image[ix] == '\0') break;
		back_buffer[j][i + ix] = image[ix];
		ix++;
	}
}

void shootBullet() {
	int idx = -1;
	for (int i = 0; i < MAX_BULLETS; i++) {
		if (p_bullet_array[i] == NULL) {
			idx = i;
			break;
		}
	}
	if (idx == -1) return;
	if (p_bullet_array[idx] == NULL) {
		p_bullet_array[idx] = (Object*)malloc(sizeof(Object));
		p_bullet_array[idx]->pos_x_ = player.pos_x_ + 2;
		p_bullet_array[idx]->pos_y_ = player.pos_y_;
		p_bullet_array[idx]->height_ = 1;
		p_bullet_array[idx]->width_ = 1;
		p_bullet_array[idx]->image_ = "!\0@\0#\0$\0^\0*\0+";
	}
}

void moveCursorTo(const short x, const short y) {
	const COORD pos = { x, SCR_HEIGHT - 1 - y };
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
}

void drawBoundary() {
	for (int j = 2; j < SCR_HEIGHT; j++) {
		drawToBackBuffer(0, j, "|");
		drawToBackBuffer(SCR_WIDTH - 1, j, "|");
	}
	char upgradeText[36];
	sprintf_s(upgradeText, "[1] UPGRADE AMMO %d GOLD", cost);
	drawToBackBuffer(0, 1, upgradeText);
	drawToBackBuffer(0, 0, "[");
	for (int i = 1; i < ammo+1; i++) {
		drawToBackBuffer(i, 0, "*");
	}
	for (int i = ammo+1; i < 9+1; i++) {
		drawToBackBuffer(i, 0, "-");
	}
	drawToBackBuffer(10, 0, "]");
}

void init() {
	CONSOLE_CURSOR_INFO cur_info;
	cur_info.dwSize = 1;
	cur_info.bVisible = false;
	SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cur_info);

	// initialize bullet array
	for (int i = 0; i < MAX_BULLETS; i++) {
		p_bullet_array[i] = NULL;
	}

	player.pos_x_ = 15;
	player.pos_y_ = 3;
	player.image_ = ">-0-<";
	player.width_ = 5;
	player.height_ = 1;

	enemy.state_ = 0;
	enemy.pos_x_ = 18;
	enemy.pos_y_ = SCR_HEIGHT - 4;
	enemy.image_ = "(0_0)\0[X_X]\0[X_X]\0[X_X]\0[X_X]\0[X_X]\0[X_X]\0[X_X]\0[X_X]\0[X_X]";
	enemy.width_ = 5;
	enemy.height_ = 1;

	// initialize display buffer
	for (int j = 0; j < SCR_HEIGHT; j++)
		for (int i = 0; i < SCR_WIDTH; i++) {
			front_buffer[j][i] = '\0';
			back_buffer[j][i] = '\0';
		}
}

void getInput() {
	if (GetAsyncKeyState(VK_LEFT) & 0x8000) {
		if (player.pos_x_ > 1)
			player.pos_x_--;
	}
	if (GetAsyncKeyState(VK_RIGHT) & 0x8000) {
		if (player.pos_x_ < SCR_WIDTH - player.width_ - 1)
			player.pos_x_++;
	}
	if (GetAsyncKeyState(VK_UP) & 0x8000) {
		if (player.pos_y_ < SCR_HEIGHT - player.height_ - 1)
			player.pos_y_++;
	}
	if (GetAsyncKeyState(VK_DOWN) & 0x8000) {
		if (player.pos_y_ > 2)
			player.pos_y_--;
	}
	static bool reload = true;
	if (GetAsyncKeyState(VK_SPACE) & 0x8000) {
		if (reload == true) {
			shootBullet();
			reload = false;
		}
	}
	else {
		reload = true;
	}
	if (GetAsyncKeyState(0x31) & 0x8000) {
		if (gold >= cost) {
			gold -= cost;
			ammo++;
			cost = 10 * ammo * ammo;
		}
	}
	
}

void simulate() {
	for (int i = 0; i < MAX_BULLETS; i++) {
		if (p_bullet_array[i] != NULL) {
			if (p_bullet_array[i]->pos_y_ >= SCR_HEIGHT - 1) {
				free(p_bullet_array[i]);
				p_bullet_array[i] = NULL;
			}
			else
				p_bullet_array[i]->pos_y_++;
		}
	}

	// check bullet-enemy collison
	if (enemy.state_ > 0)
		enemy.state_--;
	for (int i = 0; i < MAX_BULLETS; i++) {
		if (p_bullet_array[i] != NULL) {
			if (p_bullet_array[i]->pos_y_ == enemy.pos_y_ &&
				p_bullet_array[i]->pos_x_ >= enemy.pos_x_ && p_bullet_array[i]->pos_x_ <= enemy.pos_x_ + enemy.width_ - 1) {
				gold += ammo;
				enemy.state_ = 9;
				free(p_bullet_array[i]);
				p_bullet_array[i] = NULL;
			}
		}
	}
	const int dice = rand() % 3; // 0: stay, 1: left, 2: right
	switch (dice) {
	case 0:
		break;
	case 1:
		if (enemy.pos_x_ > 1)
			enemy.pos_x_--;
		break;
	case 2:
		if (enemy.pos_x_ < SCR_HEIGHT - 2)
			enemy.pos_x_++;
		break;
	}
}

void drawAll() {
	// draw boundary
	drawBoundary();
	// draw player
	drawToBackBuffer(player.pos_x_, player.pos_y_, player.image_);
	// draw enemy
	char* current_state_image = enemy.image_ + (enemy.width_ + 1) * enemy.state_;
	drawToBackBuffer(enemy.pos_x_, enemy.pos_y_, current_state_image);
	// draw a bullet
	for (int i = 0; i < MAX_BULLETS; i++) {
		if (p_bullet_array[i] != NULL) {
			char* curret_ammo_image = p_bullet_array[i]->image_ + (p_bullet_array[i]->width_ + 1) * (ammo-1);
			drawToBackBuffer(p_bullet_array[i]->pos_x_, p_bullet_array[i]->pos_y_, curret_ammo_image);
		}
	}
	// draw gold
	char goldText[15];
	sprintf_s(goldText, "GOLD %d", gold);
	drawToBackBuffer(15, SCR_HEIGHT - 1, goldText);
	char ammoText[8];
	sprintf_s(ammoText, "AMMO %d", ammo);
	drawToBackBuffer(SCR_WIDTH-9, SCR_HEIGHT - 1, ammoText);
}

void close() {
	for (int i = 0; i < MAX_BULLETS; i++) {
		if (p_bullet_array[i] != NULL)
			free(p_bullet_array[i]);
	}
}

void render() {
	//re-draw changed part
	for (int j = 0; j < SCR_HEIGHT; j++)
		for (int i = 0; i < SCR_WIDTH; i++) {
			if (back_buffer[j][i] != front_buffer[j][i]) {
				moveCursorTo(i, j);
				if (back_buffer[j][i] == '\0') {
					printf("%c", ' ');
				}
				else {
					printf("%c", back_buffer[j][i]);
				}
			}
		}
	// update frame buffer
	for (int j = 0; j < SCR_HEIGHT; j++)
		for (int i = 0; i < SCR_WIDTH; i++) {
			front_buffer[j][i] = back_buffer[j][i];
			back_buffer[j][i] = '\0';
		}
}

int main()
{
	init();
	while (true) {
		getInput();
		simulate();
		drawAll();
		render();
		Sleep(20);
	}
	close();
}
