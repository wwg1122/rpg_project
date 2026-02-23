#include "monster.h"
#include <stdio.h>

Enemy g_monsters[MAX_MONSTERS];
int g_monster_count = 0;

/* [1단계] 몬스터 데이터 밸런싱 적용 */
void spawn_monsters_in_rooms(Room* rooms, int room_count, int difficulty, int is_boss_floor, int current_floor)
{
    g_monster_count = 0;

    for (int i = 1; i < room_count; i++)
    {
        /* 마지막 방에 보스 배치 */
        if (is_boss_floor && i == room_count - 1)
        {
            Enemy* boss = &g_monsters[g_monster_count++];
            sprintf(boss->name, "Demon Boss");
            
            /* 난이도 및 층수에 비례한 보스 체력/공격력 증가 */
            boss->max_hp = 150 + (difficulty * 50) + (current_floor * 20);
            boss->hp = boss->max_hp; 
            boss->atk = 15 + (difficulty * 5) + (current_floor * 2);
            
            boss->x = rooms[i].x + rooms[i].w / 2;
            boss->y = rooms[i].y + rooms[i].h / 2;
            
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
            
            /* 난이도별 명칭 수정 및 스탯 밸런싱 */
            if (difficulty == 0) 
            {
                sprintf(e->name, "Slime");
                e->max_hp = 30 + (current_floor * 5);
                e->atk = 5 + (current_floor * 1);
            }
            else if (difficulty == 1) 
            {
                sprintf(e->name, "Bat"); /* 2단계 박쥐로 명칭 통일 */
                e->max_hp = 50 + (current_floor * 10);
                e->atk = 10 + (current_floor * 2);
            }
            else 
            {
                sprintf(e->name, "Skeleton"); /* 3단계 해골로 명칭 통일 */
                e->max_hp = 80 + (current_floor * 15);
                e->atk = 15 + (current_floor * 3);
            }
            e->hp = e->max_hp;

            e->x = GET_RAND(rooms[i].x, rooms[i].x + rooms[i].w - 1);
            e->y = GET_RAND(rooms[i].y, rooms[i].y + rooms[i].h - 1);
            
            e->min_x = rooms[i].x; e->max_x = rooms[i].x + rooms[i].w - 1;
            e->min_y = rooms[i].y; e->max_y = rooms[i].y + rooms[i].h - 1;

            e->is_alive = 1;
            e->ignore_turns = 0;
            g_dungeon_map[e->y][e->x] = TILE_MONSTER;
        }
    }
}

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

        /* 추격 (거리 3 이하) */
        if (player_x >= e->min_x && player_x <= e->max_x && player_y >= e->min_y && player_y <= e->max_y && (dx + dy <= 3))
        {
            if (e->x < player_x) nx++; else if (e->x > player_x) nx--;
            else if (e->y < player_y) ny++; else if (e->y > player_y) ny--;
        }
        else /* 배회 */
        {
            int dir = GET_RAND(0, 4);
            if (dir == 0) ny--; else if (dir == 1) ny++;
            else if (dir == 2) nx--; else if (dir == 3) nx++;
        }

        /* 이동 가능성 검증 및 맵 반영 */
        if (nx >= e->min_x && nx <= e->max_x && ny >= e->min_y && ny <= e->max_y)
        {
            if (g_dungeon_map[ny][nx] == TILE_PATH)
            {
                g_dungeon_map[e->y][e->x] = TILE_PATH; 
                e->x = nx; e->y = ny;
                g_dungeon_map[e->y][e->x] = TILE_MONSTER; 
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
