#pragma once

#define MAX_PLAYERS (2)

struct snake_segment {
    int x;
    int y;
    struct snake_segment *next;
};

struct player_state {
	int head_x;
	int head_y;
	int growth_backlog;

	int dx;
	int dy;

	int dead;
};

struct game_config {
	int wrap_snakes;
	int player_count;
	int field_size_x;
	int field_size_y;
};

struct game_state {
	const struct game_config* config;
	struct player_state* players;
	struct snake_segment** snakes;
	int player_count;

    int food_x;
    int food_y;

    char* occupied_squares;
    int occupied_square_count;
};

void init_snake_game(struct game_state *state, struct player_state player_states[], int player_count, const struct game_config* config);
void deinit_snake_game(struct game_state* state);
int set_snake_direction(struct game_state *state, int player_num, int dx, int dy);
int snake_dies(const struct game_state* state, int player_num);
void tick_snake_game(struct game_state* state);
