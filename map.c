#include "map.h"
#include <stdio.h>

// 아까 선언한 실제 전역변수 생성
int g_dungeon_map[MAX_MAP_Y][MAX_MAP_X];
int g_fog_map[MAX_MAP_Y][MAX_MAP_X];
int g_map_width = 7;
int g_map_height = 15;

//전장의 안개 걷는 함수
void reveal_fog(int px, int py)
{
	for(int y = py -2; y <= py +2; y++)
	{
		for(int x = px -2; x <= px + 2; x++)
		{
			//맵 이탈 확인 로직
			if(y >= 0 && y < g_map_height && x >= 0 && x < g_map_width)
			{
				g_fog_map[y][x] = 1;
			}
		}
	}
}

//맵 초기화 함수
void init_map(int difficulty, Character *player)
{
	for(int y = 0; y < MAX_MAP_Y; y++)
		{
			for(int x = 0; x < MAX_MAP_X; x++)
			{
				g_dungeon_map[y][x] = TILE_WALL;
				g_fog_map[y][x] = 0;
			}
		}
	if(difficulty == 0)			//easy 난이도
	{
		g_map_width = 7;
		g_map_height = 15;

		player->x = 3;
		player->y = 13;

		g_dungeon_map[1][3] = TILE_BOSS;

		for(int y = 2; y < 14; y++)
		{
			g_dungeon_map[y][3] = TILE_PATH;
		}
		
		//던전  갈림길
		g_dungeon_map[10][2] = TILE_PATH; g_dungeon_map[10][4] = TILE_PATH;
		g_dungeon_map[9][2] = TILE_PATH; g_dungeon_map[9][4] = TILE_PATH;

		g_dungeon_map[7][3] = TILE_MONSTER;
		g_dungeon_map[4][3] = TILE_MONSTER;
	}
	else 					//normal & hard 난이도
	{
		g_map_width = 15;
		g_map_height = 25;

        	player->x = 7;
        	player->y = 23;

        	g_dungeon_map[1][7] = TILE_BOSS;

        	// 메인 도로
       		for(int y = 2; y < 24; y++) g_dungeon_map[y][7] = TILE_PATH;

        	// 미로형 우회로
        	for(int y = 5; y < 20; y++)
        	{
            		if(y % 2 == 0) { g_dungeon_map[y][5] = TILE_PATH; g_dungeon_map[y][9] = TILE_PATH; }
            		else { g_dungeon_map[y][4] = TILE_PATH; g_dungeon_map[y][10] = TILE_PATH; }
        	}	

        	// 가로 연결 통로
        	g_dungeon_map[10][6] = TILE_PATH; g_dungeon_map[10][8] = TILE_PATH;
        	g_dungeon_map[18][6] = TILE_PATH; g_dungeon_map[18][8] = TILE_PATH;

        	// 몬스터 대량 배치
        	g_dungeon_map[15][7] = TILE_MONSTER;
        	g_dungeon_map[12][5] = TILE_MONSTER;
        	g_dungeon_map[12][9] = TILE_MONSTER;
        	g_dungeon_map[3][7] = TILE_MONSTER; // 중간 보스
    	}	
	
	reveal_fog(player->x, player->y);

	printf("[시스템] 맵 생성 완료 (크기: %dx%d)\n", g_map_width, g_map_height);
}

	
