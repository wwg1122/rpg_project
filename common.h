#ifndef COMMON_H
#define COMMON_H

#include <SDL2/SDL.h>
#include <stdlib.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
//키 매크로 정리
#define KEY e.key.keysym.sym
#define SET_COLOR(r,g,b) SDL_SetRenderDrawColor(renderer, r, g, b, 255)
#define FILL_RECT(rect) SDL_RenderFillRect(renderer, &rect)
#define GET_RAND(min, max) (rand() % (max - min + 1 ) + min)

//맵 타일 종류 정의
#define TILE_WALL 0
#define TILE_PATH 1
#define TILE_MONSTER 2
#define TILE_BOSS 3

// 게임 상태 정의
#define STATE_MAIN_MENU 0
#define STATE_SELECT_JOB 1
#define STATE_SELECT_DIFF 2
#define STATE_MAP 99      // 던전 탐험
#define STATE_BATTLE 3    // 전투
#define STATE_GAME_OVER 7 // 결과


typedef struct
{
    int hp;
    int max_hp;
    int mp;
    int max_mp;
    int atk;
    int is_defending;

    int x;
    int y;

    int pixel_x;
    int pixel_y;
    int is_moving;

    /* 애니메이션 및 방향 */
    int dir;
    int anim_frame;
    int anim_timer;
} Character;

extern TTF_Font* g_font;

#endif

