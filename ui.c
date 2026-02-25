#include "ui.h"
#include <SDL2/SDL_ttf.h>
#include <stdio.h>

#define DW1_COLOR_BLACK 0, 0, 0, 255
#define DW1_COLOR_WHITE 255, 255, 255, 255
#define VISION_RADIUS_TORCH 2
extern TTF_Font* g_font;

// 텍스트 랜더링 
//  8비트 감성의 픽셀 텍스트 구현
void render_text(SDL_Renderer* renderer, const char* text, int x, int y, int r, int g, int b)
{
    if (g_font == NULL || text == NULL) return;
    
    SDL_Color color = { (Uint8)r, (Uint8)g, (Uint8)b, 255 };
    
    // SDL_ttf Soild 모드 사용
    
    SDL_Surface* surface = TTF_RenderText_Solid(g_font, text, color);
    if (surface == NULL) return;

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_Rect dst = { x, y, surface->w, surface->h };

    SDL_RenderCopy(renderer, texture, NULL, &dst);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}

// 8비트 게임 느낌의 UI 랜더링
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

// 가로줄 생성 (레트로 느낌)
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

// 몬스터 충돌 시 화면 전환 연출
void play_encounter_transition(SDL_Renderer* renderer)
{
    int w = 800;
    int h = 600;
    int step = 40;

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

    for (int i = 0; i < w / 2; i += step)
    {
        // 상단 가로줄 덮기 
        SDL_Rect top = { i, i, w - 2 * i, step };
        SDL_RenderFillRect(renderer, &top);
        SDL_RenderPresent(renderer);
        SDL_Delay(15); 

        // 우측 세로줄 덮기 
        SDL_Rect right = { w - i - step, i + step, step, h - 2 * i - 2 * step };
        SDL_RenderFillRect(renderer, &right);
        SDL_RenderPresent(renderer);
        SDL_Delay(15);

        // 하단 가로줄 덮기 
        SDL_Rect bottom = { i, h - i - step, w - 2 * i, step };
        SDL_RenderFillRect(renderer, &bottom);
        SDL_RenderPresent(renderer);
        SDL_Delay(15);

        // 좌측 세로줄 덮기 
        SDL_Rect left = { i, i + step, step, h - 2 * i - 2 * step };
        SDL_RenderFillRect(renderer, &left);
        SDL_RenderPresent(renderer);
        SDL_Delay(15);
    }
}
