#include "highscore.h"

void clear_highscore_list(struct highscore_list* l) {
    l->valid_entries = 0;
}

void add_highscore(struct highscore_entry* entry, struct highscore_list* list) {
    int next_lowest_index = MAX_HIGHSCORES;
    int i;
    for(i = 0; i < list->valid_entries; i++) {
        if(list->entries[i].score < entry->score) {
            next_lowest_index = i;
            break;
        }
    }

    if(next_lowest_index >= list->valid_entries) {
        list->entries[list->valid_entries++] = *entry;
    } else {
        for(i = list->valid_entries - 1; i >= next_lowest_index; i--) {
            list->entries[i + 1] = list->entries[i];
        }

        list->valid_entries++;

        list->entries[next_lowest_index] = *entry;
    }
}

int get_highscore_count(struct highscore_list* list) {
    return list->valid_entries;
}

#ifdef __unix__

#include <stdio.h>

void print_high_scores(struct highscore_list* list) {
    int count = get_highscore_count(list);
    for(int i = 0; i < count; i++) {
        struct highscore_entry* entry = &list->entries[i];
        char initials[4];
        initials[0] = entry->initials[0];
        initials[1] = entry->initials[1];
        initials[2] = entry->initials[2];
        initials[3] = '\0';
        printf("%s %i\n", initials, entry->score);
    }
}

void add_new(struct highscore_list* l, const char* str, int score) {
    struct highscore_entry e;
    e.score = score;
    e.initials[0] = str[0];
    e.initials[1] = str[1];
    e.initials[2] = str[2];

    add_highscore(&e, l);
}

int main() {
    struct highscore_list l;
    clear_highscore_list(&l);

    add_new(&l, "AAA", 1234);

    print_high_scores(&l);
    printf("\n");

    add_new(&l, "DAW", 1231);

    print_high_scores(&l);
    printf("\n");
    
    add_new(&l, "JFK", 9999);

    print_high_scores(&l);
    printf("\n");

    add_new(&l, "BKK", 9998);

    print_high_scores(&l);
}
#endif