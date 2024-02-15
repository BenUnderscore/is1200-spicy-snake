#ifdef __unix__
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <poll.h>
#include <time.h>
#include <bits/types/struct_timeval.h>
#include <sys/poll.h>

#include "snake.h"

#define SCREEN_WIDTH 50
#define SCREEN_HEIGHT 15
char screen[SCREEN_HEIGHT][SCREEN_WIDTH];

void draw_screen() {
  static const char reset_cursor[] = "\e[H";
  puts(reset_cursor);

  for (int y = 0; y < SCREEN_HEIGHT; y++) {
    for (int x = 0; x < SCREEN_WIDTH; x++) {
      putchar(screen[SCREEN_HEIGHT - y - 1][x]);
    }
    putchar('\n');
  }
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

  int dx = 1;
  int dy = 0;

  while(1) {
    tick_snake(&state, dx, dy);

    if(snake_dies(&state)) {
      break;
    }

    render_state(&state);
    draw_screen();

    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(fileno(stdin), &fds);

    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 0;
    while(select(1, &fds, NULL, NULL, &tv) > 0) {
      char c = getchar();
      switch(c) {
        case 'a':
          dx = -1;
          dy = 0;
          break;
        case 'd':
          dx = 1;
          dy = 0;
          break;
        case 'w':
          dx = 0;
          dy = 1;
          break;
        case 's':
          dx = 0;
          dy = -1;
          break;
      }
    }

    usleep(200000);
  }

  deinit_snake_game(&state);

  return 0;
}
#endif