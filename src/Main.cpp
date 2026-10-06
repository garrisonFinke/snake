#include <windows.h>
#include <iostream>
#include <deque>
#include <algorithm>
#include <string>

HANDLE hConsole = CreateConsoleScreenBuffer(
	GENERIC_READ | GENERIC_WRITE,
	0,
	nullptr,
	CONSOLE_TEXTMODE_BUFFER,
	nullptr
);

HANDLE hInput = GetStdHandle(STD_INPUT_HANDLE);

const int screenWidth = 120;
const int screenHeight = 30;

CHAR_INFO cells[screenWidth * screenHeight]{};
SMALL_RECT writeRegion{ 0, 0, screenWidth - 1, screenHeight - 1 };

const std::string title = "SNAKE";
const std::string pause = "PAUSE";

void DrawBorders() {

	for (int y = 1; y < screenHeight - 1; y++) {
		for (int x = 1; x < screenWidth - 1; x++) {
			CHAR_INFO& c = cells[y * screenWidth + x];
			if (x == 1 || x == screenWidth - 2 ||
				y == 1 || y == screenHeight - 2 || y == 5) {
				c.Char.AsciiChar = '#';
				c.Attributes = FOREGROUND_INTENSITY;
			}
			else {
				c.Char.AsciiChar = ' ';
				c.Attributes = 0;
			}
		}
	}

	for (int i = 0; i < title.length(); i++) {
		int index = 3 * screenWidth + screenWidth / 2 - static_cast<int>(title.length()) / 2 + i;
		cells[index].Char.AsciiChar = title[i];
		cells[index].Attributes = FOREGROUND_INTENSITY;
	}
}

bool areValidIndices(int indices[], int size) {
	for (int i = 0; i < size; i ++) {
		if (cells[indices[i]].Char.AsciiChar == '#' || cells[indices[i]].Attributes == FOREGROUND_GREEN) {
			return false;
		}
	}
	return true;
}

void AddObstacle() {
	int indices[4];
	do {
		int index = (rand() % (screenHeight - 9) + 6) * screenWidth + (rand() % (screenWidth - 5) + 2);
		indices[0] = index;
		indices[1] = index + 1;
		indices[2] = index + screenWidth;
		indices[3] = index + screenWidth + 1;;
	} while (!areValidIndices(indices, 4));
	for (int i : indices) {
		cells[i].Char.AsciiChar = '#';
		cells[i].Attributes = FOREGROUND_INTENSITY;
	}
}

enum Direction { NONE, LEFT, RIGHT, UP, DOWN };

struct GameState {
	bool paused = false;
	bool quit = false;
	int score = 0;
	std::deque<int> snake;
	int head = 0;
	int apple = 0;
	Direction direction = NONE;
	Direction next = NONE;
};

GameState gamestate;

void DrawScore() {
	int i = 0;
	std::string scoreDisplay = "SCORE " + std::to_string(gamestate.score);
	for (i; i < scoreDisplay.length(); i++) {
		int index = 3 * screenWidth + 4 + i;
		cells[index].Char.AsciiChar = scoreDisplay[i];
		cells[index].Attributes = FOREGROUND_INTENSITY;
	}
}

void ProcessInput() {

	DWORD count;
	GetNumberOfConsoleInputEvents(hInput, &count);
	INPUT_RECORD record;
	DWORD eventsRead;

	while (PeekConsoleInput(hInput, &record, 1, &eventsRead) && eventsRead) {
		ReadConsoleInput(hInput, &record, 1, &eventsRead);
		if (record.EventType == KEY_EVENT) {
			auto& key = record.Event.KeyEvent;
			if (key.bKeyDown) {
				switch (key.wVirtualKeyCode) {
				case VK_ESCAPE:
					gamestate.quit = true; break;
				case VK_SHIFT:
					gamestate.paused = !gamestate.paused; break;
				case VK_LEFT:
					if (!gamestate.paused && gamestate.direction != RIGHT) gamestate.next = LEFT; break;
				case VK_RIGHT:
					if (!gamestate.paused && gamestate.direction != LEFT) gamestate.next = RIGHT; break;
				case VK_UP:
					if (!gamestate.paused && gamestate.direction != DOWN) gamestate.next = UP; break;
				case VK_DOWN:
					if (!gamestate.paused && gamestate.direction != UP) gamestate.next = DOWN; break;
				}
			}
		}
	}
}

void SetApple() {
	int y;
	int x;
	do {
		y = rand() % (screenHeight - 8) + 6;
		x = rand() % (screenWidth - 4) + 2;
		gamestate.apple = y * screenWidth + x;
	} while (cells[gamestate.apple].Char.AsciiChar == '#' ||
		cells[gamestate.apple].Attributes == FOREGROUND_GREEN);
	cells[gamestate.apple].Char.AsciiChar = '0';
	cells[gamestate.apple].Attributes = FOREGROUND_RED;
}

void RunGame() {

	FlushConsoleInputBuffer(hInput);

	DrawBorders();

	gamestate.score = 0;

	gamestate.snake.clear();
	gamestate.head = ((screenHeight - 6) / 2 + 5) * screenWidth + ((screenWidth - 2) / 2 + 1);
	gamestate.snake.push_front(gamestate.head);

	gamestate.direction = NONE;
	gamestate.next = NONE;

	cells[gamestate.apple].Char.AsciiChar = ' ';
	cells[gamestate.apple].Attributes = 0;
	SetApple();

	bool gameOver = false;

	while (!gameOver && !gamestate.quit) {

		Sleep(50);

		ProcessInput();

		if (!gamestate.paused) {

			for (int i = 0; i < pause.length(); i++) {
				int index = 3 * screenWidth + (screenWidth - static_cast<int>(pause.length()) - 4 + i);
				cells[index].Char.AsciiChar = ' ';
			}

			if (gamestate.next != NONE) {
				gamestate.direction = gamestate.next;
			}

			switch (gamestate.direction) {
			case LEFT: gamestate.head = gamestate.head - 1; break;
			case RIGHT: gamestate.head = gamestate.head + 1; break;
			case UP: gamestate.head = gamestate.head - screenWidth; break;
			case DOWN: gamestate.head = gamestate.head + screenWidth; break;
			}

			gameOver = gamestate.direction != NONE && std::find(gamestate.snake.begin(), gamestate.snake.end(), gamestate.head) != gamestate.snake.end() ||
				cells[gamestate.head].Char.AsciiChar == '#';

			gamestate.snake.push_front(gamestate.head);

			if (gamestate.head == gamestate.apple) {
				gamestate.score = gamestate.score + 100;
				if (gamestate.score % 200 == 0) {
					AddObstacle();
				}

				SetApple();
			}
			else {
				int tail = gamestate.snake.back();
				gamestate.snake.pop_back();
				cells[tail].Char.AsciiChar = ' ';
			}

			if (gameOver) {
				for (int i : gamestate.snake) {
					cells[i].Char.AsciiChar = 'X';
					cells[i].Attributes = FOREGROUND_INTENSITY;
				}
			}
			else {

				cells[gamestate.head].Char.AsciiChar = '0';
				cells[gamestate.head].Attributes = FOREGROUND_GREEN;

				DrawScore();
			}
		}
		else {
			for (int i = 0; i < pause.length(); i++) {
				int index = 3 * screenWidth + (screenWidth - static_cast<int>(pause.length()) - 4 + i);
				cells[index].Char.AsciiChar = pause[i];
				cells[index].Attributes = FOREGROUND_INTENSITY;
			}
		}

		WriteConsoleOutput(
			hConsole,
			cells,
			{ screenWidth, screenHeight },
			{ 0, 0 },
			&writeRegion
		);
	}

	for (int i : gamestate.snake) {
		cells[i].Char.AsciiChar = ' ';
	}

	for (int i = 0; i < screenWidth; i++) {
		cells[3 * screenWidth + i].Char.AsciiChar = ' ';
	}
	
	Sleep(1000);
}

int main() {

	srand(static_cast<unsigned int>(time(nullptr)));

	CONSOLE_CURSOR_INFO cursorInfo;
	GetConsoleCursorInfo(hConsole, &cursorInfo);
	cursorInfo.bVisible = FALSE;
	SetConsoleCursorInfo(hConsole, &cursorInfo);

	SetConsoleActiveScreenBuffer(hConsole);

	while (!gamestate.quit) {
		RunGame();
	}

	cursorInfo.bVisible = TRUE;
	SetConsoleCursorInfo(hConsole, &cursorInfo);

	CloseHandle(hConsole);
	CloseHandle(hInput);

	return 0;
}