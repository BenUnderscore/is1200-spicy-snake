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

#define SCREEN_WIDTH 64
#define SCREEN_HEIGHT 16
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
      screen[y][x] = state->segments[y * state->config->field_size_x + x] == SNAKE_SEGMENT_NONE ?
        '.' : '#';
    }
  }

  screen[state->food_y][state->food_x] = '+';
}

struct game_state state;
struct game_pf pf;

int main() {
	init_screen();
	refresh();

	srand(time(NULL));
	rand();
	rand();
	rand();

	struct player_state player;
	player.head_x = SCREEN_WIDTH / 2;
	player.head_y = SCREEN_HEIGHT / 2;
	player.tail_x = player.head_x;
	player.tail_y = player.head_y;
	player.growth_backlog = 2;
	player.dx = 1;
	player.dy = 0;
	player.dead = 0;

	struct game_config config;
	config.wrap_snakes = 0;
	config.field_size_x = SCREEN_WIDTH;
	config.field_size_y = SCREEN_HEIGHT;

	init_snake_game(
		&state,
		&player,
		1,
		&config,
		18386
	);

	for(int y = 0; y < SCREEN_HEIGHT; y++) {
		for(int x = 0; x < SCREEN_WIDTH; x++) {
			screen[y][x] = '.';
		}
	}

	draw_screen();

	while(!snake_dies(&state, 0))
	{
		timeout(0);
		// char c = getch();
		
		// if(c != ERR) {
		// 	switch(c) {
		// 	case 'a':
		// 		set_snake_direction(&state, 0, -1, 0);
		// 		break;
		// 	case 'd':
		// 		set_snake_direction(&state, 0, 1, 0);
		// 		break;
		// 	case 'w':
		// 		set_snake_direction(&state, 0, 0, 1);
		// 		break;
		// 	case 's':
		// 		set_snake_direction(&state, 0, 0, -1);
		// 		break;
		// 	}
		// }

		game_pf_calc(&state, &pf);
		move_npc(&state, &player, &pf);

		tick_snake_game(&state);
		render_state(&state);
		draw_screen();
		usleep(100 * 1000);
	}

	deinit_screen();
	deinit_snake_game(&state);

	return 0;
}

#endif
