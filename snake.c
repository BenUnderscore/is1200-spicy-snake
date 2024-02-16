#include <stdlib.h>
#include <string.h>

#include "snake.h"

static void set_square_occupied(struct game_state* state, int x, int y, char occupied) {
    char* prev_state = &(state->occupied_squares[y * state->config->field_size_x + x]);
    if(!*prev_state && occupied) {
        state->occupied_square_count++;
        *prev_state = 1;
    } else if(*prev_state && !occupied) {
        state->occupied_square_count--;
        *prev_state = 0;
    }
}

static struct snake_segment* create_segment(int x, int y) {
    struct snake_segment* s = (struct snake_segment*) malloc(sizeof(struct snake_segment));
    s->next = NULL;
    s->x = x;
    s->y = y;

    return s;
}

static void destroy_segment(struct snake_segment* segment) {
    if(segment->next) {
        destroy_segment(segment->next);
    }

    free(segment);
}

static void move_segment(struct game_state* state, struct snake_segment* seg, int dx, int dy, int grow) {
    int old_x = seg->x;
    int old_y = seg->y;

    seg->x += dx;
    seg->y += dy;

    set_square_occupied(state, seg->x, seg->y, 1);

    if(seg->next) {
        move_segment(state, seg->next, old_x - seg->next->x, old_y - seg->next->y, grow);
    } else if(grow) {
        seg->next = create_segment(old_x, old_y);
    } else {
        set_square_occupied(state, old_x, old_y, 0);
    }
}

static void regenerate_food(struct game_state* state) {
    int location_count = state->config->field_size_x * state->config->field_size_y - state->occupied_square_count;

    if(location_count <= 0) {
        state->food_x = -1;
        state->food_y = -1;
        return;
    }

    int location = rand() % location_count;

    if(location_count > RAND_MAX) {
        abort();
    }

    int x = 0;
    int y = 0;
    int i = 0;
    while(i < location) {
        x++;
        if(x >= state->config->field_size_x) {
            x = 0;
            y++;
        }

        if(!state->occupied_squares[y * state->config->field_size_x + x]) {
            i++;
        }
    }

    state->food_x = x;
    state->food_y = y;
}

void init_snake_game(struct game_state *state, struct player_state player_states[], int player_count, const struct game_config* config) {
	state->config = config;
	state->players = player_states;
	state->player_count = player_count;

    state->occupied_squares = (char*) malloc(sizeof(char) * state->config->field_size_x * state->config->field_size_y);
    memset(state->occupied_squares, 0, state->config->field_size_x * state->config->field_size_y);
    state->occupied_square_count = 0;

	state->snakes = (struct snake_segment**) malloc(sizeof(struct snake_segment*) * state->player_count);
	for(int i = 0; i < state->player_count; i++) {
		set_square_occupied(state, state->players[i].head_x, state->players[i].head_y, 1);
		state->snakes[i] = create_segment(state->players[i].head_x, state->players[i].head_y);
	}
    regenerate_food(state);
}

void deinit_snake_game(struct game_state* state) {
	for(int i = 0; i < state->player_count; i++) {
		if(state->snakes[i] != NULL) {
			destroy_segment(state->snakes[i]);
		}
	}
	free(state->snakes);
    free(state->occupied_squares);
}

int set_snake_direction(struct game_state *state, int player_num, int dx, int dy) {
	if(abs(dx) + abs(dy) > 1) {
		abort();
	}

	struct player_state* player = &state->players[player_num];

	if(dx != 0 && dx == -player->dx) {
		return 0;
	}

	if(dy != 0 && dy == -player->dy) {
		return 0;
	}

	player->dx = dx;
	player->dy = dy;

	return 1;
}

void tick_snake_game(struct game_state* state) {
	for(int player_num = 0; player_num < state->player_count; player_num++) {
		struct player_state* player = &state->players[player_num];

		int next_x = player->head_x + player->dx;
		int next_y = player->head_y + player->dy;
		int already_dead = player->dead;

		if(next_x < 0 || next_x >= state->config->field_size_x) {
			player->dead = 1;
		}

		if(next_y < 0 || next_y >= state->config->field_size_y) {
			player->dead = 1;
		}

		if(!player->dead && state->occupied_squares[next_y * state->config->field_size_x + next_x]) {
			player->dead = 1;
		}

		if(!player->dead) {
			int eaten = 0;
			if(next_x == state->food_x && next_y == state->food_y) {
				eaten = 1;
			}

			move_segment(state, state->snakes[player_num], player->dx, player->dy, player->growth_backlog > 0);
			state->players[player_num].head_x = state->snakes[player_num]->x;
			state->players[player_num].head_y = state->snakes[player_num]->y;

			if(player->growth_backlog > 0) {
				player->growth_backlog--;
			}

			if(eaten) {
				player->growth_backlog++;
				regenerate_food(state);
			}
		}
		
	}
}

int snake_dies(const struct game_state* state, int player_num) {
	struct player_state* player = &state->players[player_num];
    return player->dead;
}
