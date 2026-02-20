#ifndef UI_H
#define UI_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

/* 이미지 파일을 텍스처로 로드하는 함수 */
SDL_Texture* load_ui_texture(SDL_Renderer* renderer, const char* file_path);

/* 드래곤 워리어 스타일 윈도우 및 텍스트 함수 선언 */
void draw_dw_window(SDL_Renderer* renderer, int x, int y, int w, int h);
void render_text(SDL_Renderer* renderer, const char* text, int x, int y, int r, int g, int b);
void draw_tile(SDL_Renderer* renderer, SDL_Texture* tileset, int tile_x, int tile_y, int screen_x, int screen_y, int size);
void set_color(SDL_Renderer* renderer, int r, int g, int b);
void fill_rect(SDL_Renderer* renderer, SDL_Rect rect);
void play_encounter_transition(SDL_Renderer* renderer);

void draw_scanlines(SDL_Renderer* renderer, int screen_width, int screen_height);


#endif
