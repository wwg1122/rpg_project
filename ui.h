#ifndef UI_H
#define UI_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

// 게임 내부 ui 관련 시스템들 정리

void draw_dw_window(SDL_Renderer* renderer, int x, int y, int w, int h);
void render_text(SDL_Renderer* renderer, const char* text, int x, int y, int r, int g, int b);
void play_encounter_transition(SDL_Renderer* renderer);

void draw_scanlines(SDL_Renderer* renderer, int screen_width, int screen_height);


#endif
