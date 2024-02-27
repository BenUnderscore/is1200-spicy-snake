#ifdef __unix__
#include <stdlib.h>
#endif

#include "snake.h"

#ifndef __unix__
static int abs(int n) {
    return n < 0 ? -n : n;
}
#endif

static uint16_t get_random(struct game_state* state, int max_plus_one) {
    //Seed has range of 0-65535

    int r = (int) state->random_seed;
    r *= 253;
    r += 41168;
    state->random_seed = r % 65536;
    return r % max_plus_one;
}

static void move_player(struct game_state* state, int playernum) {
    struct player_state* player = &state->players[playernum];

    int new_x = player->head_x + player->dx;
    int new_y = player->head_y + player->dy;

    char new_state = SNAKE_SEGMENT_NONE;
    if(player->dx == 1) {
        new_state = SNAKE_SEGMENT_RIGHT;
    } else if(player->dx == -1) {
        new_state = SNAKE_SEGMENT_LEFT;
    } else if(player->dy == 1) {
        new_state = SNAKE_SEGMENT_UP;
    } else if(player->dy == -1) {
        new_state = SNAKE_SEGMENT_DOWN;
    }

    state->segments[new_y * state->config->field_size_x + new_x] = SNAKE_SEGMENT_HEAD;
    state->segments[player->head_y * state->config->field_size_x + player->head_x] = new_state;
    state->segment_count++;

    if(player->growth_backlog <= 0) {
        char* old_segment = &state->segments[player->tail_y * state->config->field_size_x + player->tail_x];
        switch(*old_segment) {
        case SNAKE_SEGMENT_RIGHT:
            player->tail_x++;
            break;
        case SNAKE_SEGMENT_LEFT:
            player->tail_x--;
            break;
        case SNAKE_SEGMENT_UP:
            player->tail_y++;
            break;
        case SNAKE_SEGMENT_DOWN:
            player->tail_y--;
            break;
        }
        *old_segment = SNAKE_SEGMENT_NONE;
        state->segment_count--;
    }

    player->head_x = new_x;
    player->head_y = new_y;
}

static void regenerate_food(struct game_state* state) {
    int location_count = state->config->field_size_x * state->config->field_size_y - state->segment_count;

    if(location_count <= 0) {
        state->food_x = -1;
        state->food_y = -1;
        return;
    }

    int location = get_random(state, location_count);

    int x = 0;
    int y = 0;
    int i = 0;
    while(i < location) {
        x++;
        if(x >= state->config->field_size_x) {
            x = 0;
            y++;
        }

        if(state->segments[y * state->config->field_size_x + x] == SNAKE_SEGMENT_NONE) {
            i++;
        }
    }

    state->food_x = x;
    state->food_y = y;
}

#define OBSTACLE_WIDTH 3
#define OBSTACLE_HEIGHT 3

static void generate_obstacles(struct game_state *state) {
    int success = 0;

    while(success < state->config->obstacle_count) {
        int obst_x = get_random(state, state->config->field_size_x - OBSTACLE_WIDTH);
        int obst_y = get_random(state, state->config->field_size_y - OBSTACLE_HEIGHT);

        int colliding = 0;
        int x, y;
        for(y = obst_y; y < obst_y + OBSTACLE_HEIGHT; y++) {
            for(x = obst_x; x < obst_x + OBSTACLE_WIDTH; x++) {
                if(state->segments[y * state->config->field_size_x + x] != SNAKE_SEGMENT_NONE) {
                    colliding = 1;
                    break;
                }
            }
            
            if(colliding) {
                break;
            }
        }

        if(!colliding) {
            int x, y;
            for(y = obst_y; y < obst_y + OBSTACLE_HEIGHT; y++) {
                state->segments[y * state->config->field_size_x + (obst_x + (OBSTACLE_WIDTH / 2))] = SNAKE_SEGMENT_OBSTACLE;
            }

            for(x = obst_x; x < obst_x + OBSTACLE_WIDTH; x++) {
                state->segments[(obst_y + (OBSTACLE_HEIGHT / 2)) * state->config->field_size_x + x] = SNAKE_SEGMENT_OBSTACLE;
            }

            success++;
        }
    }
}

void init_snake_game(struct game_state *state, struct player_state player_states[], int player_count, const struct game_config* config, uint16_t random_seed) {
    if(config->field_size_x * config->field_size_y > GAME_MAX_SIZE) {
        #ifdef __unix__
        abort();
        #endif
    }

	state->config = config;
	state->players = player_states;
	state->player_count = player_count;
    state->random_seed = random_seed;
    get_random(state, 1);

    int i;
    for(i = 0; i < (state->config->field_size_x * state->config->field_size_y); i++) {
        state->segments[i] = SNAKE_SEGMENT_NONE;
    }
    for(i = 0; i < player_count; i++) {
        state->segments[player_states[i].head_y * state->config->field_size_x + player_states[i].head_x] = SNAKE_SEGMENT_HEAD;
    }
    state->segment_count = player_count;

    generate_obstacles(state);

    regenerate_food(state);
}

void deinit_snake_game(struct game_state* state) {
}

int set_snake_direction(struct game_state *state, int player_num, int dx, int dy) {
	if(abs(dx) + abs(dy) > 1) {
        #ifdef __unix__
		abort();
        #endif
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

static int max(int a, int b) {
    return a > b ? a : b;
}

void move_npc(struct game_state *state, struct player_state *npc, struct game_pf *pf, int difficulty){
    int dx_cw,dy_cw;
    int dx_ccw, dy_ccw;
    if(npc->dx < 0){
        dx_ccw = 0;
        dx_cw = 0;
        dy_ccw = -1;
        dy_cw = 1;
    }
    else if(npc->dx > 0){
        dx_ccw = 0;
        dx_cw = 0;
        dy_ccw = 1;
        dy_cw = -1;
    }
    else if(npc->dy < 0){
        dx_ccw = 1;
        dx_cw = -1;
        dy_ccw = 0;
        dy_cw =0;
    }
    else{
        dx_ccw = -1;
        dx_cw = 1;
        dy_ccw = 0;
        dy_cw = 0;
    }

    int forward_x = npc->head_x + npc->dx;
    int forward_y = npc->head_y + npc->dy;

    int cw_x = npc->head_x + dx_cw;
    int cw_y = npc->head_y + dy_cw;

    int ccw_x = npc->head_x + dx_ccw;
    int ccw_y = npc->head_y + dy_ccw;

    int forward_oob =
        forward_x < 0 ||
        forward_x >= state->config->field_size_x ||
        forward_y < 0 ||
        forward_y >= state->config->field_size_y;
    
    int cw_oob =
        cw_x < 0 ||
        cw_x >= state->config->field_size_x ||
        cw_y < 0 ||
        cw_y >= state->config->field_size_y;
    
    int ccw_oob =
        ccw_x < 0 ||
        ccw_x >= state->config->field_size_x ||
        ccw_y < 0 ||
        ccw_y >= state->config->field_size_y;

    int forward = -1;
    int cw = -1;
    int ccw = -1;

    if(!forward_oob)
    {
        forward = pf->distances[forward_y * state->config->field_size_x + forward_x];
        if(forward != -1) {
            int rand = get_random(state, 256);
            int bias = max(rand - difficulty, 0);
            bias = (bias + 99) / 10;
            forward += bias;
        }
    }

    if(!cw_oob)
    {
        cw = pf->distances[cw_y * state->config->field_size_x + cw_x];
        if(cw != -1) {
            int rand = get_random(state, 256);
            int bias = max(rand - difficulty, 0);
            bias = (bias + 99) / 10;
            cw += bias;
        }
    }

    if(!ccw_oob)
    {
        ccw = pf->distances[ccw_y * state->config->field_size_x + ccw_x];
        if(ccw != -1) {
            int rand = get_random(state, 256);
            int bias = max(rand - difficulty, 0);
            bias = (bias + 99) / 10;
            ccw += bias;
        }
    }

    if(cw != -1 && (ccw == -1 || cw < ccw) && (forward == -1 || cw < forward)){
        npc->dx = dx_cw;
        npc->dy = dy_cw;
    }
    else if(ccw != -1 && (forward == -1 || ccw < forward)){
        npc->dx = dx_ccw;
        npc->dy = dy_ccw;
    } else if(forward == -1) {
        if(!cw_oob && state->segments[cw_y * state->config->field_size_x + cw_x] == SNAKE_SEGMENT_NONE) {
            npc->dx = dx_cw;
            npc->dy = dy_cw;
        } else if(!ccw_oob && state->segments[ccw_y * state->config->field_size_x + ccw_x] == SNAKE_SEGMENT_NONE) {
            npc->dx = dx_ccw;
            npc->dy = dy_ccw;
        }
    }
}

void tick_snake_game(struct game_state* state) {
    get_random(state, 2);
    int player_num;
	for(player_num = 0; player_num < state->player_count; player_num++) {
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

		if(!player->dead && state->segments[next_y * state->config->field_size_x + next_x] != SNAKE_SEGMENT_NONE) {
			player->dead = 1;
		}

		if(!player->dead) {
			int eaten = 0;
			if(next_x == state->food_x && next_y == state->food_y) {
				eaten = 1;
			}

			move_player(state, player_num);

			if(player->growth_backlog > 0) {
				player->growth_backlog--;
			}

			if(eaten) {
                player->score += FOOD_REWARD;
				player->growth_backlog++;
				regenerate_food(state);
			}

            player->subscore += TIME_SUBSCORE_REWARD;
            if(player->subscore >= SUBSCORE_MAX) {
                player->subscore -= SUBSCORE_MAX;
                player->score++;
            }
		}
	}
}

int snake_dies(const struct game_state* state, int player_num) {
	struct player_state* player = &state->players[player_num];
    return player->dead;
}

void game_pf_calc(struct game_state *state, struct game_pf *pf){
    int x, y;
    int sizex = state->config->field_size_x;
    int sizey = state->config->field_size_y;
    int stack_length = 0;
    int back_stack_length = 0;
    struct game_pf_point* stack = pf->stack1;
    struct game_pf_point* back_stack = pf->stack2;
    for(y = 0; y<sizey; y++){
        for(x=0; x<sizex; x++){
            pf->distances[y*sizex + x] = -1;
        }
    }
    pf->distances[state->food_y*sizex + state->food_x] = 0;
    stack[stack_length].x = state->food_x;
    stack[stack_length].y = state->food_y;
    stack_length = 1;

    while(stack_length > 0) {
        int i;
        for(i = 0; i < stack_length; i++) {
            int x = stack[i].x;
            int y = stack[i].y;
            
            int value = pf->distances[y * sizex + x];

            //Left cell
            if(x - 1 >= 0 &&
                state->segments[y * sizex + (x - 1)] == SNAKE_SEGMENT_NONE &&
                pf->distances[y * sizex + (x - 1)] == -1)
            {
                pf->distances[y * sizex + (x - 1)] = value + 1;
                back_stack[back_stack_length].x = x - 1;
                back_stack[back_stack_length].y = y;
                back_stack_length++;
            }

            //Right cell
            if(x + 1 < sizex &&
                state->segments[y * sizex + (x + 1)] == SNAKE_SEGMENT_NONE &&
                pf->distances[y * sizex + (x + 1)] == -1)
            {
                pf->distances[y * sizex + (x + 1)] = value + 1;
                back_stack[back_stack_length].x = x + 1;
                back_stack[back_stack_length].y = y;
                back_stack_length++;
            }

            //Top cell
            if(y - 1 >= 0 &&
                state->segments[(y - 1) * sizex + x] == SNAKE_SEGMENT_NONE &&
                pf->distances[(y - 1) * sizex + x] == -1)
            {
                pf->distances[(y - 1) * sizex + x] = value + 1;
                back_stack[back_stack_length].x = x;
                back_stack[back_stack_length].y = y - 1;
                back_stack_length++;
            }

            //Bottom cell
            if(y + 1 < sizey &&
                state->segments[(y + 1) * sizex + x] == SNAKE_SEGMENT_NONE &&
                pf->distances[(y + 1) * sizex + x] == -1)
            {
                pf->distances[(y + 1) * sizex + x] = value + 1;
                back_stack[back_stack_length].x = x;
                back_stack[back_stack_length].y = y + 1;
                back_stack_length++;
            }
        }

        void* temp = stack;

        stack_length = back_stack_length;
        stack = back_stack;

        back_stack_length = 0;
        back_stack = temp;
    }
}

void get_tail_dir(struct game_state* state, int player_num, int* dx, int* dy)
{
    struct player_state* player = &state->players[player_num];
    char segment = state->segments[player->tail_y * state->config->field_size_x + player->tail_x];

    switch(segment) {
    case SNAKE_SEGMENT_HEAD:
        *dx = player->dx;
        *dy = player->dy;
        break;
    case SNAKE_SEGMENT_LEFT:
        *dx = -1;
        *dy = 0;
        break;
    case SNAKE_SEGMENT_RIGHT:
        *dx = 1;
        *dy = 0;
        break;
    case SNAKE_SEGMENT_DOWN:
        *dx = 0;
        *dy = -1;
        break;
    case SNAKE_SEGMENT_UP:
        *dx = 0;
        *dy = 1;
        break;
    }
}