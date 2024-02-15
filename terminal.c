#ifdef __unix__
//C standard libraries
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

//Linux time libraries for usleep
#include <unistd.h>

//NCurses
#include <ncurses.h>

#include "snake.h"

#define SCREEN_WIDTH 50
#define SCREEN_HEIGHT 15
char screen[SCREEN_HEIGHT][SCREEN_WIDTH];

void init_screen() {
	initscr();
	cbreak();
	noecho();
}

void draw_screen() {
  for (int y = 0; y < SCREEN_HEIGHT; y++) {
    for (int x = 0; x < SCREEN_WIDTH; x++) {
      mvaddch(y, x, screen[SCREEN_HEIGHT - y - 1][x]);
    }
  }
  refresh();
}

void deinit_screen() {
	endwin();
}

void render_state(struct game_state* state) {
  for(int y = 0; y < SCREEN_HEIGHT; y++) {
    for(int x = 0; x < SCREEN_WIDTH; x++) {
      screen[y][x] = state->occupied_squares[y * SCREEN_WIDTH + x] ?
        '#' : '.';
    }
  }

  screen[state->food_y][state->food_x] = '+';
}

int main() {
	init_screen();
	refresh();

	srand(time(NULL));
	rand();
	rand();
	rand();

	struct game_state state;

	init_snake_game(
		&state,
		SCREEN_WIDTH / 2,
		SCREEN_HEIGHT / 2,
		SCREEN_WIDTH,
		SCREEN_HEIGHT
	);

	state.growth_backlog = 5;

	for(int y = 0; y < SCREEN_HEIGHT; y++) {
		for(int x = 0; x < SCREEN_WIDTH; x++) {
			screen[y][x] = '.';
		}
	}

	draw_screen();

	while(!snake_dies(&state))
	{
		tick_snake(&state);

		timeout(0);
		char c = getch();

		render_state(&state);
		
		if(c != ERR) {
			switch(c) {
			case 'a':
				set_snake_direction(&state, -1, 0);
				break;
			case 'd':
				set_snake_direction(&state, 1, 0);
				break;
			case 'w':
				set_snake_direction(&state, 0, 1);
				break;
			case 's':
				set_snake_direction(&state, 0, -1);
				break;
			}
		}

		draw_screen();
		usleep(100 * 1000);
	}

	deinit_screen();

	return 0;
}

#endif
