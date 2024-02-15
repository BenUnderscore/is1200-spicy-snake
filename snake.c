#include <stdlib.h>
#include <string.h>

#include "snake.h"

static void set_square_occupied(struct game_state* state, int x, int y, char occupied) {
    char* prev_state = &(state->occupied_squares[y * state->field_size_x + x]);
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
    int location_count = state->field_size_x * state->field_size_y - state->occupied_square_count;

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
        if(x >= state->field_size_x) {
            x = 0;
            y++;
        }

        if(!state->occupied_squares[y * state->field_size_x + x]) {
            i++;
        }
    }

    state->food_x = x;
    state->food_y = y;
}

void init_snake_game(struct game_state *state, int starting_x, int starting_y, int field_size_x, int field_size_y) {
    state->dead = 0;
    state->field_size_x = field_size_x;
    state->field_size_y = field_size_y;
    state->head = create_segment(starting_x, starting_y);
    state->growth_backlog = 0;

    state->occupied_squares = (char*) malloc(sizeof(char) * state->field_size_x * state->field_size_y);
    memset(state->occupied_squares, 0, state->field_size_x * state->field_size_y);
    state->occupied_square_count = 0;

    set_square_occupied(state, starting_x, starting_y, 1);
    regenerate_food(state);
}

void deinit_snake_game(struct game_state* state) {
    destroy_segment(state->head);
    free(state->occupied_squares);
}

void grow_snake(struct game_state *state) {
    state->growth_backlog++;
}

void tick_snake(struct game_state* state, int dx, int dy) {
    
    if(state->head->x + dx < 0 ||
        state->head->x + dx >= state->field_size_x || state->head->y + dy < 0 || state->head->y + dy >= state->field_size_y) {
        state->dead = 1;
    }

    if(state->dead) {
        return;
    }

    int eaten = state->head->x + dx == state->food_x && state->head->y + dy == state->food_y;
    if(eaten) {
        state->growth_backlog++;
    }

    move_segment(state, state->head, dx, dy, state->growth_backlog > 0);
    if(state->growth_backlog > 0) {
        state->growth_backlog--;
    }

    if(eaten) {
        regenerate_food(state);
    }
}

int snake_dies(const struct game_state* state) {
    if(state->dead) {
        return 1;
    }

    struct snake_segment* current_seg = state->head->next;
    while(current_seg != NULL) {
        if(current_seg->x == state->head->x && current_seg->y == state->head->y) {
            return 1;
        }

        current_seg = current_seg->next;
    }

    return 0;
}