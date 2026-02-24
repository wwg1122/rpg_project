#ifndef MAP_H
#define MAP_H

#include "common.h"

#define MAP_WIDTH  40
#define MAP_HEIGHT 40

// 몬스터 이동범위 제한을 위한 맵 구조체
typedef struct
{
    int x, y, w, h;
} Room;

extern int g_dungeon_map[MAP_HEIGHT][MAP_WIDTH];

void init_map(int difficulty, int current_floor, int max_floor, Character *player);

#endif
