#pragma once

#include <stdint.h>

#define MAX_PLAYERS (2)
#define GAME_MAX_SIZE (1024)

#define SNAKE_SEGMENT_NONE (0)
#define SNAKE_SEGMENT_UP (1)
#define SNAKE_SEGMENT_RIGHT (2)
#define SNAKE_SEGMENT_DOWN (3)
#define SNAKE_SEGMENT_LEFT (4)
#define SNAKE_SEGMENT_HEAD (5)

struct player_state {
	int head_x;
	int head_y;
	int tail_x;
	int tail_y;
	int growth_backlog;

	int dx;
	int dy;

	int dead;
};

struct game_pf_point {
	char x;
	char y;
};

struct game_pf{
	short distances[GAME_MAX_SIZE];
	struct game_pf_point stack1[GAME_MAX_SIZE];
	struct game_pf_point stack2[GAME_MAX_SIZE];
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
	int player_count;

	uint16_t random_seed;
    int food_x;
    int food_y;

	char segments[GAME_MAX_SIZE];
    int segment_count;
};

void init_snake_game(struct game_state *state, struct player_state player_states[], int player_count, const struct game_config* config, uint16_t random_seed);
void deinit_snake_game(struct game_state* state);
int set_snake_direction(struct game_state *state, int player_num, int dx, int dy);
int snake_dies(const struct game_state* state, int player_num);
void tick_snake_game(struct game_state* state);
void game_pf_calc(struct game_state *state, struct game_pf *pf);
void move_npc(struct game_state *state, struct player_state *npc, struct game_pf *pf);