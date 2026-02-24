#ifndef MONSTER_H
#define MONSTER_H

#include "common.h"
#include "map.h"

typedef struct
{
    char name[20];
    int hp, max_hp, atk;
    int mp;
    int x, y;
    int min_x, max_x; 
    int min_y, max_y;
    int is_alive;
    int ignore_turns;
    int exp_reward, gold_reward;
} Enemy;

#define MAX_MONSTERS 30
extern Enemy g_monsters[MAX_MONSTERS];
extern int g_monster_count;

void spawn_monsters_in_rooms(Room* rooms, int room_count, int difficulty, int is_boss_floor, int current_floor, int shop_idx);
void update_all_monsters_ai(int player_x, int player_y);
void free_enemy(Enemy *target);

#endif
