#ifndef MAP_H
#define MAP_H
#include "common.h"

#define MAP_WIDTH  40
#define MAP_HEIGHT 40

// 방 구초제
typedef struct
{
    int x, y, w, h;
} Room;

extern int g_dungeon_map[MAP_HEIGHT][MAP_WIDTH];
void init_map(int difficulty, int current_floor, int max_floor, Character *player);

#endif
