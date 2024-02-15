#pragma once

struct snake_segment {
    int x;
    int y;
    struct snake_segment *next;
};

struct game_state {
    struct snake_segment *head;
    int growth_backlog;

	int dx;
	int dy;

    int food_x;
    int food_y;

    int field_size_x;
    int field_size_y;
    char* occupied_squares;
    int occupied_square_count;

    int dead;
};

void init_snake_game(struct game_state *state, int starting_x, int starting_y, int field_size_x, int field_size_y);
void deinit_snake_game(struct game_state* state);
void grow_snake(struct game_state *state);
int set_snake_direction(struct game_state *state, int dx, int dy);
void tick_snake(struct game_state* state);
int snake_dies(const struct game_state* state);
