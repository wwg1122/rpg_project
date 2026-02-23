#include "map.h"
#include "monster.h"
#include <stdio.h>

#define MIN_ROOM_SIZE  6
#define MAX_ROOM_SIZE  9
#define BOSS_ROOM_W    12  // 맵 크기(40x40) 대비 너무 크지 않게 조정
#define BOSS_ROOM_H    9
#define ROOM_MARGIN    5   // 방 사이의 최소 간격

int g_dungeon_map[MAP_HEIGHT][MAP_WIDTH];

// 길 생성 함수: 벽만 골라서 뚫어 불필요한 관통 최소화
void connect_points(int x1, int y1, int x2, int y2)
{
    int cx = x1;
    int cy = y1;

    // 가로 이동
    while (cx != x2)
    {
        // 벽일 때만 길(TILE_PATH)로 변경하여 기존 방/보스방 타일 보호
        if (g_dungeon_map[cy][cx] == TILE_WALL) 
        {
            g_dungeon_map[cy][cx] = TILE_PATH;
        }
        cx += (x1 < x2) ? 1 : -1;
    }
    // 세로 이동
    while (cy != y2)
    {
        if (g_dungeon_map[cy][cx] == TILE_WALL) 
        {
            g_dungeon_map[cy][cx] = TILE_PATH;
        }
        cy += (y1 < y2) ? 1 : -1;
    }
}

void init_map(int difficulty, int current_floor, int max_floor, Character *player)
{
    // 1. 맵 전체 초기화
    for (int y = 0; y < MAP_HEIGHT; y++)
        for (int x = 0; x < MAP_WIDTH; x++)
            g_dungeon_map[y][x] = TILE_WALL;

    int room_limit = 4 + difficulty; 
    Room rooms[10];
    int actual_rooms = 0;

    // 2. 방 생성 루프
    for (int i = 0; i < room_limit; i++)
    {
        Room r;
        // 마지막 방은 보스방으로 설정
        if (current_floor == max_floor && i == room_limit - 1)
        {
            r.w = BOSS_ROOM_W; r.h = BOSS_ROOM_H;
        }
        else
        {
            r.w = GET_RAND(MIN_ROOM_SIZE, MAX_ROOM_SIZE);
            r.h = GET_RAND(MIN_ROOM_SIZE, MAX_ROOM_SIZE);
        }

        int is_overlap;
        int tries = 0;
        
        do {
            is_overlap = 0;
            r.x = GET_RAND(2, MAP_WIDTH - r.w - 2);
            r.y = GET_RAND(2, MAP_HEIGHT - r.h - 2);

            // ROOM_MARGIN을 적용해 방 사이 거리를 강제함
            for (int j = 0; j < actual_rooms; j++)
            {
                if (!(r.x + r.w + ROOM_MARGIN < rooms[j].x || r.x > rooms[j].x + rooms[j].w + ROOM_MARGIN ||
                      r.y + r.h + ROOM_MARGIN < rooms[j].y || r.y > rooms[j].y + rooms[j].h + ROOM_MARGIN))
                {
                    is_overlap = 1;
                    break;
                }
            }
            tries++;
        } while (is_overlap && tries < 150);

        if (is_overlap) continue;

        // 방 바닥 생성
        for (int py = r.y; py < r.y + r.h; py++)
            for (int px = r.x; px < r.x + r.w; px++)
                g_dungeon_map[py][px] = TILE_PATH;

        rooms[actual_rooms] = r;

        // [핵심] 이전 방과 현재 방을 일대일로만 연결 (방-길-방 구조)
        if (actual_rooms > 0)
        {
            connect_points(rooms[actual_rooms - 1].x + rooms[actual_rooms - 1].w / 2,
                           rooms[actual_rooms - 1].y + rooms[actual_rooms - 1].h / 2,
                           r.x + r.w / 2, r.y + r.h / 2);
        }
        actual_rooms++;
    }

    // 3. 플레이어 및 보스/계단 배치
    if (actual_rooms > 0)
    {
        // 시작 위치: 첫 번째 방
        player->x = rooms[0].x + rooms[0].w / 2;
        player->y = rooms[0].y + rooms[0].h / 2;

        // 탈출구/보스 위치: 마지막 방
        Room last = rooms[actual_rooms - 1];
        int tx = last.x + last.w / 2;
        int ty = last.y + last.h / 2;
        g_dungeon_map[ty][tx] = (current_floor == max_floor) ? TILE_BOSS : TILE_STAIRS;

	//상자 생성 함수 추가
	for (int i = 1; i < actual_rooms - 1; i++)
        {
            if (GET_RAND(1, 100) <= 40)
            {
                int cx = GET_RAND(rooms[i].x + 1, rooms[i].x + rooms[i].w - 2);
                int cy = GET_RAND(rooms[i].y + 1, rooms[i].y + rooms[i].h - 2);
                if (g_dungeon_map[cy][cx] == TILE_PATH)
                {
                    g_dungeon_map[cy][cx] = TILE_CHEST;
                }
            }
        }

        // 몬스터 생성
        spawn_monsters_in_rooms(rooms, actual_rooms, difficulty, (current_floor == max_floor), current_floor);
    }

    printf("[시스템] %d층 던전 생성 완료 (방: %d개)\n", current_floor, actual_rooms);
}
