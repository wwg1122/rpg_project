#include "monster.h"
#include <stdio.h>

Enemy g_monsters[MAX_MONSTERS];
int g_monster_count = 0;

void spawn_monsters_in_rooms(Room* rooms, int room_count, int difficulty, int is_boss_floor)
{
    g_monster_count = 0;

    for (int i = 1; i < room_count; i++)
    {
        /* 마지막 방에 보스 배치 */
        if (is_boss_floor && i == room_count - 1)
        {
            Enemy* boss = &g_monsters[g_monster_count++];
            sprintf(boss->name, "Demon Boss");
            boss->hp = 150; boss->max_hp = 150; boss->atk = 15;
            boss->x = rooms[i].x + rooms[i].w / 2;
            boss->y = rooms[i].y + rooms[i].h / 2;
            
            /* 보스 구역 설정 */
            boss->min_x = rooms[i].x; boss->max_x = rooms[i].x + rooms[i].w - 1;
            boss->min_y = rooms[i].y; boss->max_y = rooms[i].y + rooms[i].h - 1;
            
            boss->is_alive = 1;
            boss->ignore_turns = 0;
            g_dungeon_map[boss->y][boss->x] = TILE_BOSS;
            continue;
        }

        /* 일반 몬스터 배치 */
        if (g_monster_count < MAX_MONSTERS)
        {
            Enemy* e = &g_monsters[g_monster_count++];
            
            /* 난이도별 이름 설정 */
            if (difficulty == 0) sprintf(e->name, "Slime");
            else if (difficulty == 1) sprintf(e->name, "Bat");
            else sprintf(e->name, "Ghost");

            e->hp = 30; e->max_hp = 30; e->atk = 5;
            e->x = GET_RAND(rooms[i].x, rooms[i].x + rooms[i].w - 1);
            e->y = GET_RAND(rooms[i].y, rooms[i].y + rooms[i].h - 1);
            
            /* [수정됨] 일반 몬스터도 움직일 수 있게 활동 구역 제한 설정! */
            e->min_x = rooms[i].x; e->max_x = rooms[i].x + rooms[i].w - 1;
            e->min_y = rooms[i].y; e->max_y = rooms[i].y + rooms[i].h - 1;

            e->is_alive = 1;
            e->ignore_turns = 0;
            g_dungeon_map[e->y][e->x] = TILE_MONSTER;
        }
    }
}

// 모든 몬스터의 AI를 한 번에 업데이트하는 함수
void update_all_monsters_ai(int player_x, int player_y)
{
    for (int i = 0; i < g_monster_count; i++)
    {
        Enemy* e = &g_monsters[i];
        if (!e->is_alive || g_dungeon_map[e->y][e->x] == TILE_BOSS) continue; 
    
        if (e->ignore_turns > 0)
        {
            e->ignore_turns--;
            continue;
        }

        int dx = abs(e->x - player_x);
        int dy = abs(e->y - player_y);
        int nx = e->x, ny = e->y;

        // 추격 (거리 3 이하)
        if (player_x >= e->min_x && player_x <= e->max_x && player_y >= e->min_y && player_y <= e->max_y && (dx + dy <= 3))
        {
            if (e->x < player_x) nx++; else if (e->x > player_x) nx--;
            else if (e->y < player_y) ny++; else if (e->y > player_y) ny--;
        }
        else // 배회
        {
            int dir = GET_RAND(0, 4);
            if (dir == 0) ny--; else if (dir == 1) ny++;
            else if (dir == 2) nx--; else if (dir == 3) nx++;
        }

        // 이동 가능성 검증 및 맵 반영
        if (nx >= e->min_x && nx <= e->max_x && ny >= e->min_y && ny <= e->max_y)
        {
            if (g_dungeon_map[ny][nx] == TILE_PATH)
            {
                g_dungeon_map[e->y][e->x] = TILE_PATH; // 예전 자리 지우기
                e->x = nx; e->y = ny;
                g_dungeon_map[e->y][e->x] = TILE_MONSTER; // 새 자리 그리기
            }
        }
    }
}

void free_enemy(Enemy *target)
{
    if (target != NULL)
    {
        printf("[시스템] 몬스터 '%s' 메모리 해제\n", target->name);
        free(target);
    }
}
