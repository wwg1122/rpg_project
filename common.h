#ifndef COMMON_H		//중복 정의 방지
#define COMMON_H

#include <SDL2/SDL.h>
#include <stdlib.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>

//키 매크로 정리
#define KEY e.key.keysym.sym
#define SET_COLOR(r,g,b) SDL_SetRenderDrawColor(renderer, r, g, b, 255)		//랜더링 함수 간소화
#define FILL_RECT(rect) SDL_RenderFillRect(renderer, &rect)			
#define GET_RAND(min, max) (rand() % (max - min + 1 ) + min)			//난수 생성 관련 코드

//맵 타일 종류 정의 g_dungeon_map에 들어갈 고유 정수값 모음
#define TILE_WALL 0
#define TILE_PATH 1
#define TILE_MONSTER 2
#define TILE_BOSS 3
#define TILE_CHEST 4
#define TILE_STAIRS 5
#define TILE_NPC 6
#define TILE_TORCH 7
#define TILE_PORTAL 8
#define TILE_BOSS_CHEST 9
#define TILE_DECOR1 10
#define TILE_DECOR2 11
#define TILE_VOID 99

// 게임 상태 정의
#define STATE_MAIN_MENU 0
#define STATE_SELECT_JOB 1
#define STATE_SELECT_DIFF 2
#define STATE_MAP 99      
#define STATE_BATTLE 3    
#define STATE_GAME_OVER 7 
#define STATE_LOAD_MENU 10
#define STATE_SAVE_MENU 11
#define STATE_ESC_MENU 12
#define STATE_STATUS_MENU 13
#define STATE_EQUIP_MENU 14
#define STATE_INVEN_MENU 15
#define STATE_SHOP_UI 16

// 무기 등급 관련 시스템
#define RARITY_COMMON 0 // 60% (흰색)
#define RARITY_RARE 1   // 30% (파랑)
#define RARITY_EPIC 2   // 10% (보라)

// 무기 구조체
typedef struct {
    char name[32];
    int atk_bonus; 	// 추가 공격력 정의
    int rarity;		// 등급 관련 정의
} WeaponItem;

// 캐릭터 구조체
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
    int pixel_x;	// 이동 연출 관련 
    int pixel_y;
    int is_moving;

    int dir;
    int anim_frame;
    int anim_timer;

    int class_type; 
    int level;
    int exp;
    int gold;
    
    // 무기 장착 및 인벤토리 관련 부분
    WeaponItem weapons[20];
    int weapon_count;
    int equipped_weapon_idx; // -1이면 미장착
    
} Character;

// 세이브 구조
typedef struct
{
    int is_empty;
    Character player_data;
    int hp_potions;
    int mp_potions;
    int difficulty;
    int current_floor;
} SaveSlot;

// 전역 변수 공유 (전투 화면 메시지 관련)
extern TTF_Font* g_font;
extern char g_msg1[128];
extern char g_msg2[128];

#endif
