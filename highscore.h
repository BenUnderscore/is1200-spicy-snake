#pragma once

#define INITIALS_COUNT (3)
#define MAX_HIGHSCORES (16)

struct highscore_entry {
    char initials[INITIALS_COUNT];
    int score;
};

struct highscore_list {
    struct highscore_entry entries[MAX_HIGHSCORES];
    int valid_entries;
};

void clear_highscore_list(struct highscore_list* l);
void add_highscore(struct highscore_entry* entry, struct highscore_list* list);
int get_highscore_count(struct highscore_list* entry);