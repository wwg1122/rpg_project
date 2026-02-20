#ifndef MONSTER_H
#define MONSTER_H

#include "common.h"
#include "map.h"

typedef struct
{
    char name[20];
    int hp, max_hp, atk;
    int x, y;
    int min_x, max_x; // 몬스터 활동 구역 (방 경계)
    int min_y, max_y;
    int is_alive;
    int ignore_turns;
} Enemy;

#define MAX_MONSTERS 30
extern Enemy g_monsters[MAX_MONSTERS];
extern int g_monster_count;

void spawn_monsters_in_rooms(Room* rooms, int room_count, int difficulty, int is_boss_floor);
void update_all_monsters_ai(int player_x, int player_y);
void free_enemy(Enemy *target);

//Enemy *create_enemy(char *name, int hp, int atk);
//void free_enemy(Enemy *target);


#endif
