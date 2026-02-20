#include "ui.h"
#include <SDL2/SDL_ttf.h>
#include <stdio.h>

#define DW1_COLOR_BLACK 0, 0, 0, 255
#define DW1_COLOR_WHITE 255, 255, 255, 255
#define VISION_RADIUS_TORCH 2

/* 전역 폰트 포인터 (main.c에서 초기화됨) */
extern TTF_Font* g_font;

/* 이미지 파일을 SDL_Texture로 변환하여 로드하는 함수 */
SDL_Texture* load_ui_texture(SDL_Renderer* renderer, const char* file_path)
{
    SDL_Texture* texture = IMG_LoadTexture(renderer, file_path);
    if (texture == NULL)
    {
        printf("이미지 로드 실패: %s, 에러: %s\n", file_path, IMG_GetError());
    }
    return texture;
}

/* 색상 설정을 위한 헬퍼 함수 */
void set_color(SDL_Renderer* renderer, int r, int g, int b)
{
    SDL_SetRenderDrawColor(renderer, r, g, b, 255);
}

/* 사각형을 채우는 헬퍼 함수 */
void fill_rect(SDL_Renderer* renderer, SDL_Rect rect)
{
    SDL_RenderFillRect(renderer, &rect);
}

/* 텍스트 렌더링 함수: SDL_ttf를 사용하여 지정된 좌표에 출력 */
void render_text(SDL_Renderer* renderer, const char* text, int x, int y, int r, int g, int b)
{
    if (g_font == NULL || text == NULL)
    {
        return;
    }

    SDL_Color color = { (Uint8)r, (Uint8)g, (Uint8)b, 255 };
    
    /* 레트로 감성을 위해 Solid 모드로 렌더링 */
    SDL_Surface* surface = TTF_RenderText_Solid(g_font, text, color);
    if (surface == NULL)
    {
        return;
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_Rect dst = { x, y, surface->w, surface->h };

    SDL_RenderCopy(renderer, texture, NULL, &dst);

    /* 리소스 해제 */
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}

void draw_dw_window(SDL_Renderer* renderer, int x, int y, int w, int h)
{
    SDL_Rect bg_rect;
    bg_rect.x = x; bg_rect.y = y; bg_rect.w = w; bg_rect.h = h;
    SDL_SetRenderDrawColor(renderer, DW1_COLOR_BLACK);
    SDL_RenderFillRect(renderer, &bg_rect);

    SDL_Rect outer_rect;
    outer_rect.x = x; outer_rect.y = y; outer_rect.w = w; outer_rect.h = h;
    SDL_SetRenderDrawColor(renderer, DW1_COLOR_WHITE);
    SDL_RenderDrawRect(renderer, &outer_rect);

    SDL_Rect inner_rect;
    inner_rect.x = x + 3; inner_rect.y = y + 3;
    inner_rect.w = w - 6; inner_rect.h = h - 6;
    SDL_RenderDrawRect(renderer, &inner_rect);
}

void render_dungeon_state(SDL_Renderer* renderer, int player_x, int player_y)
{
    /* 화면 전체를 드퀘 던전 특유의 칠흑 같은 어둠으로 초기화 */
    SDL_SetRenderDrawColor(renderer, DW1_COLOR_BLACK);
    SDL_RenderClear(renderer);


    /* 1. 커맨드 윈도우 (왼쪽 상단 찰칵: COMMAND) */
    draw_dw_window(renderer, 30, 30, 200, 260);

    /* 2. 스테이터스 윈도우 (오른쪽 상단 찰칵: NAME, HP, MP, LV, G, E) */
    draw_dw_window(renderer, 550, 30, 220, 200);

    /* 3. 메시지 윈도우 (하단 찰칵: 대화 및 전투 로그) */
    draw_dw_window(renderer, 100, 420, 600, 150);

    SDL_RenderPresent(renderer);
}

/* 나중에 Tiny 16 Basic 같은 타일셋 입힐 때 쓸 타일 렌더러 */
void draw_tile(SDL_Renderer* renderer, SDL_Texture* tileset, int tile_x, int tile_y, int screen_x, int screen_y, int size)
{
    if (tileset == NULL)
    {
        return;
    }
    SDL_Rect src;
    src.x = tile_x; src.y = tile_y; src.w = 16; src.h = 16;
    SDL_Rect dst;
    dst.x = screen_x; dst.y = screen_y; dst.w = size; dst.h = size;
    SDL_RenderCopy(renderer, tileset, &src, &dst);
}

void draw_scanlines(SDL_Renderer* renderer, int screen_width, int screen_height)
{
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 80);
    for (int y = 0; y < screen_height; y += 2)
    {
        SDL_RenderDrawLine(renderer, 0, y, screen_width - 1, y);
    }
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
}


void play_encounter_transition(SDL_Renderer* renderer)
{
    int w = 800;
    int h = 600;
    int step = 40; /* 8비트 특유의 투박한 타일 느낌을 내기 위한 블록 두께 */

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

    /* 화면 바깥쪽 테두리부터 중심부까지 사각형을 그리며 좁혀 들어감 */
    for (int i = 0; i < w / 2; i += step)
    {
        /* 1. 상단 가로줄 덮기 */
        SDL_Rect top = { i, i, w - 2 * i, step };
        SDL_RenderFillRect(renderer, &top);
        SDL_RenderPresent(renderer);
        SDL_Delay(15); /* 회오리치는 속도 조절 (너무 빠르면 숫자를 늘릴 것) */

        /* 2. 우측 세로줄 덮기 */
        SDL_Rect right = { w - i - step, i + step, step, h - 2 * i - 2 * step };
        SDL_RenderFillRect(renderer, &right);
        SDL_RenderPresent(renderer);
        SDL_Delay(15);

        /* 3. 하단 가로줄 덮기 */
        SDL_Rect bottom = { i, h - i - step, w - 2 * i, step };
        SDL_RenderFillRect(renderer, &bottom);
        SDL_RenderPresent(renderer);
        SDL_Delay(15);

        /* 4. 좌측 세로줄 덮기 */
        SDL_Rect left = { i, i + step, step, h - 2 * i - 2 * step };
        SDL_RenderFillRect(renderer, &left);
        SDL_RenderPresent(renderer);
        SDL_Delay(15);
    }
}
