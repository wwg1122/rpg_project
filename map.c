#include "map.h"
#include "monster.h"
#include <stdio.h>

#define MIN_ROOM_SIZE  6
#define MAX_ROOM_SIZE  9
#define BOSS_ROOM_W    12
#define BOSS_ROOM_H    9
#define ROOM_MARGIN    5

int g_dungeon_map[MAP_HEIGHT][MAP_WIDTH];

// 길 생성 함수: 벽만 골라서 뚫어 불필요한 관통 최소화
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

void init_map(int difficulty, int current_floor, int max_floor, Character *player)
{
    for (int y = 0; y < MAP_HEIGHT; y++)
        for (int x = 0; x < MAP_WIDTH; x++)
            g_dungeon_map[y][x] = TILE_WALL;

    int room_limit = 5 + difficulty; 
    Room rooms[12];
    int actual_rooms = 0;

    for (int i = 0; i < room_limit; i++)
    {
        Room r;
        if (current_floor == max_floor && i == room_limit - 1)
        { r.w = BOSS_ROOM_W; r.h = BOSS_ROOM_H; }
        else
        { r.w = GET_RAND(MIN_ROOM_SIZE, MAX_ROOM_SIZE); r.h = GET_RAND(MIN_ROOM_SIZE, MAX_ROOM_SIZE); }

        int is_overlap; int tries = 0;
        
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

        if (actual_rooms > 0)
        {
            connect_points(rooms[actual_rooms - 1].x + rooms[actual_rooms - 1].w / 2,
                           rooms[actual_rooms - 1].y + rooms[actual_rooms - 1].h / 2,
                           r.x + r.w / 2, r.y + r.h / 2);
        }
        actual_rooms++;
    }

    if (actual_rooms > 0)
    {
        player->x = rooms[0].x + rooms[0].w / 2;
        player->y = rooms[0].y + rooms[0].h / 2;

        int shop_idx = -1;
        if (actual_rooms > 2 && (current_floor == 1 || GET_RAND(1, 100) <= 30))
        {
            shop_idx = 1;
            int sx = rooms[shop_idx].x + rooms[shop_idx].w / 2;
            int sy = rooms[shop_idx].y + rooms[shop_idx].h / 2;
            g_dungeon_map[sy][sx] = TILE_NPC;
            g_dungeon_map[sy][sx - 1] = TILE_TORCH;
            g_dungeon_map[sy][sx + 1] = TILE_TORCH;
        }

        Room last = rooms[actual_rooms - 1];
        int tx = last.x + last.w / 2;
        int ty = last.y + last.h / 2;
        g_dungeon_map[ty][tx] = (current_floor == max_floor) ? TILE_BOSS : TILE_STAIRS;

        for (int i = 1; i < actual_rooms - 1; i++)
        {
            if (i == shop_idx) continue;
            if (GET_RAND(1, 100) <= 40)
            {
                int cx = GET_RAND(rooms[i].x + 1, rooms[i].x + rooms[i].w - 2);
                int cy = GET_RAND(rooms[i].y + 1, rooms[i].y + rooms[i].h - 2);
                if (g_dungeon_map[cy][cx] == TILE_PATH) g_dungeon_map[cy][cx] = TILE_CHEST;
            }
        }
        spawn_monsters_in_rooms(rooms, actual_rooms, difficulty, (current_floor == max_floor), current_floor, shop_idx);
    }
}
