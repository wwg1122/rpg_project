#include "map.h"
#include "monster.h"
#include <stdio.h>

#define MIN_ROOM_SIZE  6
#define MAX_ROOM_SIZE  9
#define BOSS_ROOM_W    12
#define BOSS_ROOM_H    9
#define ROOM_MARGIN    5

int g_dungeon_map[MAP_HEIGHT][MAP_WIDTH];

// 방과 방 사이의 통로를 뚫어주는 로직
// TILE_WALL 에서만 작동하게 설계
void connect_points(int x1, int y1, int x2, int y2)
{
    int cx = x1;
    int cy = y1;

    while (cx != x2)
    {
        if (g_dungeon_map[cy][cx] == TILE_WALL) g_dungeon_map[cy][cx] = TILE_PATH;
        cx += (x1 < x2) ? 1 : -1;
    }
    while (cy != y2)
    {
        if (g_dungeon_map[cy][cx] == TILE_WALL) g_dungeon_map[cy][cx] = TILE_PATH;
        cy += (y1 < y2) ? 1 : -1;
    }
}

// 맵 생성 메인 함수
void init_map(int difficulty, int current_floor, int max_floor, Character *player)
{	

    // 맵 생성 자체를 모든 공간을 벽으로 초기화 후 -> 방 생성 순서로 진행함	
	for (int y = 0; y < MAP_HEIGHT; y++)
	{
		for (int x = 0; x < MAP_WIDTH; x++)
		{
			g_dungeon_map[y][x] = TILE_WALL; 
		}
	}	

    int room_limit = 5 + difficulty;		// 난이도 관련 방의 개수
    Room rooms[12];
    int actual_rooms = 0;
	

    // 방 생성 랜덤 로직
    for (int i = 0; i < room_limit; i++)
    {
        Room r;
        if (current_floor == max_floor && i == room_limit - 1)
        { r.w = BOSS_ROOM_W; r.h = BOSS_ROOM_H; }
        else
        { r.w = GET_RAND(MIN_ROOM_SIZE, MAX_ROOM_SIZE); r.h = GET_RAND(MIN_ROOM_SIZE, MAX_ROOM_SIZE); }
	
        int is_overlap; int tries = 0;
        
	// 방 겹침 관련 방지 로직
        do {
            is_overlap = 0;
            r.x = GET_RAND(2, MAP_WIDTH - r.w - 2);
            r.y = GET_RAND(2, MAP_HEIGHT - r.h - 2);

            for (int j = 0; j < actual_rooms; j++)
            {
                if (!(r.x + r.w + ROOM_MARGIN < rooms[j].x || r.x > rooms[j].x + rooms[j].w + ROOM_MARGIN ||
                      r.y + r.h + ROOM_MARGIN < rooms[j].y || r.y > rooms[j].y + rooms[j].h + ROOM_MARGIN))
                { is_overlap = 1; break; }
            }
            tries++;
        } while (is_overlap && tries < 150);

        if (is_overlap) continue;
	

        for (int py = r.y; py < r.y + r.h; py++)
            for (int px = r.x; px < r.x + r.w; px++)
                g_dungeon_map[py][px] = TILE_PATH;

        rooms[actual_rooms] = r;
	
	// 길 뚫기 
        if (actual_rooms > 0)
        {
            connect_points(rooms[actual_rooms - 1].x + rooms[actual_rooms - 1].w / 2,
                           rooms[actual_rooms - 1].y + rooms[actual_rooms - 1].h / 2,
                           r.x + r.w / 2, r.y + r.h / 2);
        }
        actual_rooms++;
    }
    

    // 오브젝트 배치 (상자, 계단, 보스, 플레이어 등)
    if (actual_rooms > 0)
    {	
        player->x = rooms[0].x + rooms[0].w / 2;					// 플레이어 시작 위치 무조건 방 중앙 고정
        player->y = rooms[0].y + rooms[0].h / 2;

        int shop_idx = -1;
        if (actual_rooms > 2 && (current_floor == 1 || GET_RAND(1, 100) <= 30))		// 상점 생성 관련 확률
        {
            shop_idx = 1;
            int sx = rooms[shop_idx].x + rooms[shop_idx].w / 2;
            int sy = rooms[shop_idx].y + rooms[shop_idx].h / 2;
            g_dungeon_map[sy][sx] = TILE_NPC;
            g_dungeon_map[sy][sx - 1] = TILE_TORCH;
            g_dungeon_map[sy][sx + 1] = TILE_TORCH;
        }

        Room last = rooms[actual_rooms - 1];						// 마지막 방 보스 or 계단
        int tx = last.x + last.w / 2;
        int ty = last.y + last.h / 2;
        g_dungeon_map[ty][tx] = (current_floor == max_floor) ? TILE_BOSS : TILE_STAIRS;

        for (int i = 1; i < actual_rooms - 1; i++)					// 상자 생성 (상점 방 생성 x)
        {
            if (i == shop_idx) continue;
            if (GET_RAND(1, 100) <= 40)
            {
                int cx = GET_RAND(rooms[i].x + 1, rooms[i].x + rooms[i].w - 2);
                int cy = GET_RAND(rooms[i].y + 1, rooms[i].y + rooms[i].h - 2);
                if (g_dungeon_map[cy][cx] == TILE_PATH) g_dungeon_map[cy][cx] = TILE_CHEST;
            }
        }
	
	for (int y = 1; y < MAP_HEIGHT - 1; y++)
	{
		for (int x = 1; x < MAP_WIDTH - 1; x++)
		{
			if (g_dungeon_map[y][x] == TILE_PATH)
			{
				int path_count = 0;
				for (int dy = -1; dy <= 1; dy++)
				{
					for (int dx = -1; dx <= 1; dx++)
					{
						if (g_dungeon_map[y + dy][x + dx] == TILE_PATH)
						{
							path_count++;
						}
					}
				}

				if (path_count == 4 || path_count == 6)
				{
					if (GET_RAND(1, 100) <= 10) 
					{
						g_dungeon_map[y][x] = GET_RAND(0, 1) ? TILE_DECOR1 : TILE_DECOR2;
					}
				}
			}
		}
	}	
		


	if (difficulty >= 1)
	{
		for (int y = 0; y < MAP_HEIGHT; y++)
		{
			for (int x = 0; x < MAP_WIDTH; x++)
			{
				if (g_dungeon_map[y][x] == TILE_WALL)
				{
					int is_near_path = 0;
					for (int dy = -1; dy <= 1; dy++)
					{
						for (int dx = -1; dx <= 1; dx++)
						{
							int ny = y + dy;
							int nx = x + dx;
							if (ny >= 0 && ny < MAP_HEIGHT && nx >= 0 && nx < MAP_WIDTH)
							{
								int neighbor = g_dungeon_map[ny][nx];
								if (neighbor != TILE_WALL && neighbor != TILE_VOID)
								{
									is_near_path = 1;
								}
							}
						}
					}
					if (!is_near_path) g_dungeon_map[y][x] = TILE_VOID;
				}
			}
		}
	}

	// 맵 생성 완료시 몬스터 스폰 함수 호출
        spawn_monsters_in_rooms(rooms, actual_rooms, difficulty, (current_floor == max_floor), current_floor, shop_idx);
    }
}
