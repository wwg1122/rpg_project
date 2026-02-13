#ifndef COMMON_H
#define COMMON_H

#include <SDL2/SDL.>
#include <stdlib.h>

#define KEY e.key.keysym.sym
#define SET_COLOR(r,g,b) SDL_SetRenderDrawColor(renderer, r, g, b, 255)
#define FILL_RECT(rect) SDL_RenderFillRect(renderer, &rect)
#define GET_RAND(min, max) (rand() % (max - min + 1 ) + min)



//구조체 모음
typedef struct
{
	int hp;
	int max_hp;
	int mp;
	int max_mp;
	int atk;
} Character;

typedef struct Enemy
{
    int hp;
    int max_hp;
    int atk;
    char *name;
    struct Enemy *next; 
} Enemy;

#endif

