#ifndef MAP_H
#define MAP_H

#include "common.h"

#define MAX_MAP_Y 30
#define MAX_MAP_X 20

//맵 전역변수
extern int g_dungeon_map[MAX_MAP_Y][MAX_MAP_X];		//맵 데이터
extern int g_fog_map[MAX_MAP_Y][MAX_MAP_X];		//안개 데이터
extern int g_map_width;
extern int g_map_height;

void init_map(int difficulty, Character *player);
void reveal_fog(int px, int py);

#endif
