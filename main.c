#include <SDL2/SDL.h>		//SDL2 헤더 파일
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include "monster.h"
#include "map.h"
#include "ui.h"
#include "common.h"

TTF_Font* g_font = NULL;
SDL_Texture* map_tex = NULL;
SDL_Texture* char_tex = NULL;
SDL_Texture* tileset_tex = NULL;

#define KEY e.key.keysym.sym	//75줄
#define SET_COLOR(r,g,b) SDL_SetRenderDrawColor(renderer, r, g, b, 255)
#define FILL_RECT(rect) SDL_RenderFillRect(renderer, &rect)
#define GET_RAND(min, max) (rand() % (max - min + 1 ) + min) 	//랜덤 난수 범위 구현

//난이도 설정 전역 변수
char *diff_names[] = {"Slime Forest", "Bat Cave", "Demon Castle"};
//전투 시 사용 가능한 메뉴 나열
char *battle_menu[] = {"Attack", "Skill", "Item", "Defense", "Run"};
//직업별 스킬 설정
char *warrior_skills[] = {"Power Strike", "Whirlwind"};		//휠이 아닌 훨윈드이다.
char *mage_skills[] = 	{"Magic Arrow", "Meteor"};		
char *item_menu[] = {"HP Potion", "MP Potion"};

char **current_skills; 					 	//더블 포인터로 직업 스킬 연결

int saved_map_x = 0;						//전투 후 복귀를 위한 좌표
int saved_map_y = 0;

// 전투 메시지 출력을 위한 전역 버퍼 선언
char g_msg1[128] = "";
char g_msg2[128] = "";

WeaponItem g_current_shop_weapon; // [ADD] 상점에 입장할 때 생성되는 무기 데이터 저장용

SaveSlot g_save_slots[5];
void init_save_data()
{
    FILE* fp = fopen("save.dat", "rb");
    if (fp) { fread(g_save_slots, sizeof(SaveSlot), 5, fp); fclose(fp); } 
    else { for (int i = 0; i < 5; i++) g_save_slots[i].is_empty = 1; }
}
void write_save_data()
{
    FILE* fp = fopen("save.dat", "wb");
    if (fp) { fwrite(g_save_slots, sizeof(SaveSlot), 5, fp); fclose(fp); }
}

// 전투씬 배경 유지 렌더러
void render_battle_scene(SDL_Renderer* renderer, Character* player, Enemy* current_monster, int difficulty, int is_boss, int menu_index, int hide_monster)
{
	SET_COLOR(0, 0, 0);
	SDL_Rect b_bg = {0, 0, 800, 600};
	FILL_RECT(b_bg);

	// 몬스터 숨김 상태가 아닐 때만 렌더링
	if (current_monster != NULL && !hide_monster)
	{
		int msx = 0; int msy = 0;
		if (is_boss) { msx = 96; msy = 64; }
		else if (difficulty == 0) { msx = 0; msy = 64; }
		else if (difficulty == 1) { msx = 48; msy = 64; }
		else { msx = 144; msy = 0; } 

		SDL_Rect m_src = { msx, msy, 16, 16 };
		SDL_Rect m_dst;
		if (is_boss) { m_dst.x = 225; m_dst.y = 70; m_dst.w = 350; m_dst.h = 350; }
        else { m_dst.x = 275; m_dst.y = 120; m_dst.w = 250; m_dst.h = 250; }

		SDL_RenderCopy(renderer, char_tex, &m_src, &m_dst);

		char m_hp_str[32];
		sprintf(m_hp_str, "HP: %d/%d", current_monster->hp, current_monster->max_hp);
        // [UI 수정 위치: 몬스터 체력 텍스트]
		render_text(renderer, m_hp_str, 340, 380, 255, 100, 100);
	}

    // [UI 수정 위치: 전투 커맨드 윈도우 배경]
	draw_dw_window(renderer, 20, 20, 230, 260);
	render_text(renderer, "COMMAND", 50, 40, 255, 255, 255);
	for (int i = 0; i < 5; i++)
	{
		if (menu_index == i) render_text(renderer, ">", 35, 90 + (i * 35), 255, 255, 0);
        // [UI 수정 위치: 전투 메뉴 항목]
		render_text(renderer, battle_menu[i], 60, 90 + (i * 35), 255, 255, (menu_index == i ? 0 : 255));
	}

    // [UI 수정 위치: 전투 우측 스탯 윈도우 배경]
	draw_dw_window(renderer, 550, 20, 220, 200);
	render_text(renderer, "STATUS", 600, 40, 255, 255, 255);

	char stat_p[32];
    sprintf(stat_p, "LV:%d G:%d", player->level, player->gold);
    render_text(renderer, stat_p, 580, 70, 255, 255, 0);
	sprintf(stat_p, "HP: %d", player->hp); 
	render_text(renderer, stat_p, 580, 105, 255, 255, 255);
	sprintf(stat_p, "MP: %d", player->mp); 
	render_text(renderer, stat_p, 580, 140, 255, 255, 255);
    
    // [ADD] 최종 공격력 깔끔하게 통합 표시
    int total_atk = player->atk + (player->equipped_weapon_idx >= 0 ? player->weapons[player->equipped_weapon_idx].atk_bonus : 0);
	sprintf(stat_p, "ATK: %d", total_atk); 
	render_text(renderer, stat_p, 580, 175, 255, 255, 255);
}

// 드퀘 감성 타자 치는 효과 텍스트 박스
void render_typing_message(SDL_Renderer* renderer, const char* text1, const char* text2, Character* player, Enemy* current_monster, int difficulty, int is_boss, int menu_index)
{
	char buf1[128] = {0}; char buf2[128] = {0}; int i = 0, j = 0;

	if (text1 != NULL) 
	{
		while (text1[i] != '\0') 
		{
			if ((text1[i] & 0x80) == 0) { buf1[j++] = text1[i++]; }
			else { buf1[j++] = text1[i++]; buf1[j++] = text1[i++]; buf1[j++] = text1[i++]; }
			buf1[j] = '\0';
			render_battle_scene(renderer, player, current_monster, difficulty, is_boss, menu_index, 0);
            // [UI 수정 위치: 전투 하단 메시지 윈도우 (타이핑 중)]
			draw_dw_window(renderer, 100, 420, 600, 140);
			render_text(renderer, buf1, 130, 460, 255, 255, 255);
			SDL_RenderPresent(renderer); SDL_Delay(35); 
		}
	}
	
	i = 0; j = 0;
	if (text2 != NULL) 
	{
		while (text2[i] != '\0') 
		{
			if ((text2[i] & 0x80) == 0) { buf2[j++] = text2[i++]; }
			else { buf2[j++] = text2[i++]; buf2[j++] = text2[i++]; buf2[j++] = text2[i++]; }
			buf2[j] = '\0';
			render_battle_scene(renderer, player, current_monster, difficulty, is_boss, menu_index, 0);
            // [UI 수정 위치: 전투 하단 메시지 윈도우 (타이핑 중 2번째 줄)]
			draw_dw_window(renderer, 100, 420, 600, 140);
			if (text1 != NULL) render_text(renderer, buf1, 130, 460, 255, 255, 255);
			render_text(renderer, buf2, 130, 500, 255, 255, 255);
			SDL_RenderPresent(renderer); SDL_Delay(35);
		}
	}
	SDL_Delay(800); 
	if (text1 != NULL) { sprintf(g_msg1, "%s", text1); } else { g_msg1[0] = '\0'; }
	if (text2 != NULL) { sprintf(g_msg2, "%s", text2); } else { g_msg2[0] = '\0'; }
}

void play_hit_effect(SDL_Renderer* renderer, Character* player, Enemy* current_monster, int difficulty, int is_boss, int menu_index, int is_player_hit)
{
	for (int i = 0; i < 2; i++) 
	{
		render_battle_scene(renderer, player, current_monster, difficulty, is_boss, menu_index, 0);
		SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
		if (is_player_hit) SDL_SetRenderDrawColor(renderer, 255, 0, 0, 120); else SDL_SetRenderDrawColor(renderer, 255, 255, 255, 120); 
		SDL_Rect bg = {0, 0, 800, 600}; SDL_RenderFillRect(renderer, &bg); SDL_RenderPresent(renderer); SDL_Delay(50);
		render_battle_scene(renderer, player, current_monster, difficulty, is_boss, menu_index, 0); SDL_RenderPresent(renderer); SDL_Delay(50);
	}
}

void play_monster_death_effect(SDL_Renderer* renderer, Character* player, Enemy* current_monster, int difficulty, int is_boss, int menu_index)
{
	int msx = 0; int msy = 0;
	if (is_boss) { msx = 96; msy = 64; }
	else if (difficulty == 0) { msx = 0; msy = 64; }
	else if (difficulty == 1) { msx = 48; msy = 64; }
	else { msx = 144; msy = 0; }

	SDL_Rect m_src = { msx, msy, 16, 16 };
	SDL_Rect m_dst = { 275, 120, 250, 250 };
	
	SDL_SetTextureBlendMode(char_tex, SDL_BLENDMODE_BLEND);
	for (int alpha = 255; alpha >= 0; alpha -= 15) 
	{
		render_battle_scene(renderer, player, current_monster, difficulty, is_boss, menu_index, 1);
		m_dst.y += 2; SDL_SetTextureAlphaMod(char_tex, alpha); SDL_RenderCopy(renderer, char_tex, &m_src, &m_dst);
		SDL_RenderPresent(renderer); SDL_Delay(30);
	}
	SDL_SetTextureAlphaMod(char_tex, 255); 
}

void play_stair_effect(SDL_Renderer* renderer)
{
	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
	for (int a = 0; a <= 255; a += 15) 
	{
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, a);
		SDL_Rect bg = {0, 0, 800, 600}; SDL_RenderFillRect(renderer, &bg); SDL_RenderPresent(renderer); SDL_Delay(20);
	}
	SDL_Delay(200); 
}

int main(int argc, char *argv[])
{	
	int diff_count = sizeof(diff_names) / sizeof(diff_names[0]);
	int current_skill_count = 0;
	int battle_menu_count = sizeof(battle_menu) / sizeof(battle_menu[0]);
	int item_menu_count = sizeof(item_menu) / sizeof(item_menu[0]);
	srand(time(NULL));

	int difficulty = 0;
	int hp_potions = 2; int mp_potions = 2;
	int current_floor = 1; int max_floor = 3;	

	if (SDL_Init(SDL_INIT_VIDEO) < 0) return 1;
	if (TTF_Init() == -1) return 1;

	g_font = TTF_OpenFont("dq_font.ttf", 24);
	if (g_font == NULL) { printf("폰트 로드 실패\n"); }

	SDL_Window *window = SDL_CreateWindow("RPG Project", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_SHOWN);
	if (window == NULL) { SDL_Quit(); return 1; }

	SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	map_tex = IMG_LoadTexture(renderer, "map_tiles.png"); char_tex = IMG_LoadTexture(renderer, "master_tiles.png");
	if (map_tex == NULL || char_tex == NULL) return 1;

	SDL_SetTextureBlendMode(map_tex, SDL_BLENDMODE_BLEND); SDL_SetTextureBlendMode(char_tex, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    init_save_data();

	int menu_index = 0;	int game_state = 0; int is_boss = 0; int quit = 0;
	SDL_Event e;

	Character warrior = {150, 150, 30, 30, 30, 0, 0, 0};	
	Character mage = {80, 80, 120, 120, 10, 0, 0, 0};
	Character player;				
	Enemy *current_monster = NULL; 			

	while(!quit)
	{
		if (game_state == 99)
		{
			player.anim_timer++;
			if (player.anim_timer > 12) { player.anim_frame = !player.anim_frame; player.anim_timer = 0; }

			if (player.is_moving)
			{
				int target_px = player.x * 48; int target_py = player.y * 48; int move_speed = 4;
				if (player.pixel_x < target_px) player.pixel_x += move_speed; else if (player.pixel_x > target_px) player.pixel_x -= move_speed;
				if (player.pixel_y < target_py) player.pixel_y += move_speed; else if (player.pixel_y > target_py) player.pixel_y -= move_speed;

				if (player.pixel_x == target_px && player.pixel_y == target_py)
				{
					player.is_moving = 0;
					update_all_monsters_ai(player.x, player.y);
					
					for (int i = 0; i < g_monster_count; i++)
					{
						if (g_monsters[i].is_alive && g_monsters[i].x == player.x && g_monsters[i].y == player.y)
						{
							if (g_monsters[i].ignore_turns <= 0)
							{
								current_monster = &g_monsters[i];
								saved_map_x = current_monster->x; saved_map_y = current_monster->y;
								is_boss = (g_dungeon_map[current_monster->y][current_monster->x] == TILE_BOSS);
								play_encounter_transition(renderer);
								sprintf(g_msg1, "AMBUSHED BY %s!", current_monster->name); g_msg2[0] = '\0';
								game_state = 3; menu_index = 0; break;
							}
						}
					}

					if (game_state != 3 && g_dungeon_map[player.y][player.x] == TILE_STAIRS)
					{
						play_stair_effect(renderer);
						current_floor++;
						init_map(difficulty, current_floor, max_floor, &player);
						player.pixel_x = player.x * 48; player.pixel_y = player.y * 48;
					}
                    else if (game_state != 3 && g_dungeon_map[player.y][player.x] == TILE_PORTAL)
                    {
                        play_stair_effect(renderer);
                        game_state = 7;
                    }
				}
			}
		}

		while (SDL_PollEvent(&e) != 0)
		{
			if (e.type == SDL_QUIT) quit = 1;
			else if (e.type == SDL_KEYDOWN)
			{
				if (KEY == SDLK_ESCAPE)
				{
					if (game_state == 4 || game_state == 5 || game_state == 6)
					{
						if (game_state == 4) menu_index = 1;      
						else if (game_state == 5) menu_index = 2; 
						else if (game_state == 6) menu_index = 4; 
						game_state = 3;
					}
                    else if (game_state == 99 && !player.is_moving) { game_state = STATE_ESC_MENU; menu_index = 0; }
                    else if (game_state == STATE_ESC_MENU) { game_state = 99; }
                    else if (game_state == STATE_SAVE_MENU) { game_state = STATE_ESC_MENU; menu_index = 3; }
                    else if (game_state == STATE_LOAD_MENU) { game_state = 0; menu_index = 1; }
                    else if (game_state >= STATE_STATUS_MENU && game_state <= STATE_INVEN_MENU) { game_state = STATE_ESC_MENU; menu_index = 1; } // 장비창 껐을 때 커서 기억
                    else if (game_state == STATE_SHOP_UI) { game_state = 99; }
					else { game_state = 0; menu_index = 0; }
					continue; 
				}	

				if (game_state == 0)
				{
					if (KEY == SDLK_UP) { if (menu_index > 0) menu_index--; }
					else if (KEY == SDLK_DOWN) { if (menu_index < 2) menu_index++; }
					else if (KEY == SDLK_RETURN)
					{
						if (menu_index == 0) { game_state = 1; menu_index = 0; }
                        else if (menu_index == 1) { game_state = STATE_LOAD_MENU; menu_index = 0; }
						else { quit = 1; }
					}
				}
				else if (game_state == 1)
				{
					if (KEY == SDLK_LEFT) { menu_index = 0; }
					else if (KEY == SDLK_RIGHT) { menu_index = 1; }
					else if (KEY == SDLK_RETURN)
					{
						if (menu_index == 0) { player = warrior; current_skills = warrior_skills; player.class_type = 0; }
						else { player = mage; current_skills = mage_skills; player.class_type = 1; }
                        current_skill_count = 2;
                        player.level = 1; player.exp = 0; player.gold = 100;
                        
                        // [ADD] 초기 무기 지급
                        player.weapon_count = 1;
                        player.equipped_weapon_idx = 0;
                        WeaponItem start_wpn;
                        sprintf(start_wpn.name, "RUSTY SWORD"); start_wpn.atk_bonus = 5; start_wpn.rarity = RARITY_COMMON;
                        player.weapons[0] = start_wpn;

						game_state = 2; menu_index = 0;
					}	
				}
				else if (game_state == 2)
				{
					if (KEY == SDLK_UP) { if(menu_index > 0) menu_index--; }
					else if (KEY == SDLK_DOWN) { if(menu_index < diff_count -1) menu_index++; }
					else if (KEY == SDLK_RETURN)
					{
						difficulty = menu_index;
						player.hp = player.max_hp; player.mp = player.max_mp;
						hp_potions = 2; mp_potions = 2;
						current_floor = 1;
						init_map(difficulty, current_floor, max_floor, &player);
						player.pixel_x = player.x * 48; player.pixel_y = player.y * 48;
						player.is_moving = 0; game_state = 99; menu_index = 0;
					}
				}
                else if (game_state == STATE_SAVE_MENU || game_state == STATE_LOAD_MENU)
                {
                    if (KEY == SDLK_UP) { if(menu_index > 0) menu_index--; }
                    else if (KEY == SDLK_DOWN) { if(menu_index < 4) menu_index++; }
                    else if (KEY == SDLK_RETURN)
                    {
                        if (game_state == STATE_SAVE_MENU)
                        {
                            g_save_slots[menu_index].is_empty = 0; g_save_slots[menu_index].player_data = player;
                            g_save_slots[menu_index].hp_potions = hp_potions; g_save_slots[menu_index].mp_potions = mp_potions;
                            g_save_slots[menu_index].difficulty = difficulty; g_save_slots[menu_index].current_floor = current_floor;
                            write_save_data(); game_state = STATE_ESC_MENU; menu_index = 0;
                        }
                        else if (game_state == STATE_LOAD_MENU)
                        {
                            if (!g_save_slots[menu_index].is_empty)
                            {
                                player = g_save_slots[menu_index].player_data;
                                hp_potions = g_save_slots[menu_index].hp_potions; mp_potions = g_save_slots[menu_index].mp_potions;
                                difficulty = g_save_slots[menu_index].difficulty;
                                current_skills = (player.class_type == 0) ? warrior_skills : mage_skills; current_skill_count = 2;
                                game_state = 2; menu_index = 0;
                            }
                        }
                    }
                }
                else if (game_state == STATE_ESC_MENU)
                {
                    if (KEY == SDLK_UP) { if (menu_index > 0) menu_index--; }
                    else if (KEY == SDLK_DOWN) { if (menu_index < 5) menu_index++; }
                    else if (KEY == SDLK_RETURN)
                    {
                        if (menu_index == 0) { game_state = STATE_STATUS_MENU; } 
                        else if (menu_index == 1) { game_state = STATE_EQUIP_MENU; menu_index = 0; } // 장비창 이동
                        else if (menu_index == 2) { game_state = STATE_INVEN_MENU; } 
                        else if (menu_index == 3) { game_state = STATE_SAVE_MENU; menu_index = 0; } 
                        else if (menu_index == 4) { game_state = 0; menu_index = 0; } 
                        else if (menu_index == 5) { game_state = 99; } 
                    }
                }
                else if (game_state == STATE_EQUIP_MENU) // [ADD] 무기 장착 로직
                {
                    if (KEY == SDLK_UP) { if (menu_index > 0) menu_index--; }
                    else if (KEY == SDLK_DOWN) { if (menu_index < player.weapon_count - 1) menu_index++; }
                    else if (KEY == SDLK_RETURN)
                    {
                        if (player.weapon_count > 0) { player.equipped_weapon_idx = menu_index; } // 엔터 시 무기 스왑
                    }
                }
                else if (game_state == STATE_SHOP_UI)
                {
                    int price_hp = 50; int price_mp = 100; int price_wpn = 300;
                    if (KEY == SDLK_UP) { if (menu_index > 0) menu_index--; }
                    else if (KEY == SDLK_DOWN) { if (menu_index < 3) menu_index++; }
                    else if (KEY == SDLK_RETURN)
                    {
                        if (menu_index == 0) { if (player.gold >= price_hp) { player.gold -= price_hp; hp_potions++; } }
                        else if (menu_index == 1) { if (player.gold >= price_mp) { player.gold -= price_mp; mp_potions++; } }
                        else if (menu_index == 2) // [ADD] 등급별 무기 구매 및 인벤토리 추가
                        { 
                            if (player.gold >= price_wpn && player.weapon_count < 20) 
                            { 
                                player.gold -= price_wpn; 
                                player.weapons[player.weapon_count] = g_current_shop_weapon;
                                player.weapon_count++;
                            } 
                        }
                        else if (menu_index == 3) { game_state = 99; }
                    }
                }
				else if (game_state == 99 && !player.is_moving)
				{
					int nx = player.x; int ny = player.y; int moved = 0;

					if (KEY == SDLK_UP) { ny--; player.dir = 3; moved = 1; }
					else if (KEY == SDLK_DOWN) { ny++; player.dir = 0; moved = 1; }
					else if (KEY == SDLK_LEFT) { nx--; player.dir = 1; moved = 1; }
					else if (KEY == SDLK_RIGHT) { nx++; player.dir = 2; moved = 1; }
					
					else if (KEY == SDLK_z || KEY == SDLK_RETURN) 
					{
						int chk_x = player.x; int chk_y = player.y;
						if (player.dir == 0) chk_y++; else if (player.dir == 1) chk_x--; else if (player.dir == 2) chk_x++; else if (player.dir == 3) chk_y--;

						if (chk_x >= 0 && chk_x < MAP_WIDTH && chk_y >= 0 && chk_y < MAP_HEIGHT)
						{
                            int target_tile = g_dungeon_map[chk_y][chk_x];
							if (target_tile == TILE_CHEST || target_tile == TILE_BOSS_CHEST)
							{
								g_dungeon_map[chk_y][chk_x] = TILE_PATH; 
                                char msg[64];

                                // [ADD] 보스 장비 상자 무기 획득
                                if (target_tile == TILE_BOSS_CHEST && player.weapon_count < 20)
                                {
                                    WeaponItem b_wpn;
                                    b_wpn.rarity = RARITY_EPIC;
                                    b_wpn.atk_bonus = 30 + (difficulty * 10);
                                    if (player.class_type == 0) { sprintf(b_wpn.name, "HERO SWORD"); sprintf(msg, "GOT %s!", b_wpn.name); } 
                                    else { sprintf(b_wpn.name, "SAGE WAND"); sprintf(msg, "GOT %s!", b_wpn.name); }
                                    
                                    player.weapons[player.weapon_count] = b_wpn;
                                    player.weapon_count++;
                                }
                                else 
                                {
                                    int r = GET_RAND(0, 1);
                                    if (r == 0) { hp_potions++; sprintf(msg, "FOUND HP POTION!"); } 
                                    else { mp_potions++; sprintf(msg, "FOUND MP POTION!"); }
                                }
                                
                                // [UI 수정 위치: 필드 보물상자 획득 메시지]
								draw_dw_window(renderer, 205, 250, 405, 100);
								render_text(renderer, msg, 220, 290, 255, 255, 0);
								SDL_RenderPresent(renderer); SDL_Delay(1000);
							}
                            else if (g_dungeon_map[chk_y][chk_x] == TILE_NPC)
                            {
                                // [ADD] 상점 진입 시 랜덤 무기 생성 (재질 및 등급 확률)
                                int rnd_rarity = GET_RAND(1, 100);
                                g_current_shop_weapon.rarity = (rnd_rarity <= 10) ? RARITY_EPIC : ((rnd_rarity <= 40) ? RARITY_RARE : RARITY_COMMON);
                                
                                int base_atk = (difficulty == 0) ? 10 : ((difficulty == 1) ? 20 : 30);
                                int extra_atk = (g_current_shop_weapon.rarity == RARITY_EPIC) ? 15 : ((g_current_shop_weapon.rarity == RARITY_RARE) ? 5 : 0);
                                g_current_shop_weapon.atk_bonus = base_atk + extra_atk;
                                
                                const char* mat = (difficulty == 0) ? "IRON" : ((difficulty == 1) ? "GOLD" : "DIAMOND");
                                const char* type = (player.class_type == 0) ? "SWORD" : "WAND";
                                sprintf(g_current_shop_weapon.name, "%s %s", mat, type);

                                game_state = STATE_SHOP_UI; menu_index = 0;
                            }
						}
					}

					if (moved && nx >= 0 && nx < MAP_WIDTH && ny >= 0 && ny < MAP_HEIGHT)
					{
						Enemy* target_mon = NULL;
						for (int i = 0; i < g_monster_count; i++)
						{
							if (g_monsters[i].is_alive && g_monsters[i].x == nx && g_monsters[i].y == ny)
							{ target_mon = &g_monsters[i]; break; }
						}

						if (target_mon != NULL)
						{
							if (target_mon->ignore_turns <= 0)
							{
								current_monster = target_mon;
								saved_map_x = nx; saved_map_y = ny;
								is_boss = (g_dungeon_map[ny][nx] == TILE_BOSS);
								
								play_encounter_transition(renderer);
								sprintf(g_msg1, "%s APPEARED!", current_monster->name); g_msg2[0] = '\0';
								game_state = 3; menu_index = 0;
							}
						}
						else
						{
							int tile = g_dungeon_map[ny][nx];
							if (tile == TILE_PATH || tile == TILE_STAIRS || tile == TILE_PORTAL || tile == TILE_NPC || tile == TILE_TORCH) 
							{
                                if (tile != TILE_NPC && tile != TILE_TORCH)
								    { player.x = nx; player.y = ny; player.is_moving = 1; }
							}
						}
					}
				}	
				else if (game_state == 3) // 전투 로직
				{
					if (KEY == SDLK_UP) { if(menu_index > 0) menu_index--; }
					else if (KEY == SDLK_DOWN) { if(menu_index < battle_menu_count - 1) menu_index++; }
					else if (KEY == SDLK_RETURN)
					{
						switch (menu_index)
						{
							case 0:
							{
                                // [ADD] 평타 무기 공격력 합산
                                int total_atk = player.atk + (player.equipped_weapon_idx >= 0 ? player.weapons[player.equipped_weapon_idx].atk_bonus : 0);
								int final_dmg = GET_RAND(total_atk - 5, total_atk + 5);
								current_monster->hp -= final_dmg;
								char m1[64], m2[64];
								sprintf(m1, "ATTACKED %s!", current_monster->name); sprintf(m2, "DEALT %d DAMAGE!", final_dmg);
								
								play_hit_effect(renderer, &player, current_monster, difficulty, is_boss, menu_index, 0);
								render_typing_message(renderer, m1, m2, &player, current_monster, difficulty, is_boss, menu_index);
								
								if (current_monster->hp <= 0)
								{
									current_monster->hp = 0;
									sprintf(m1, "DEFEATED %s!", current_monster->name);
									
                                    int gained_exp = current_monster->exp_reward; int gained_gold = current_monster->gold_reward;
                                    player.exp += gained_exp; player.gold += gained_gold;

                                    char reward_msg[64]; sprintf(reward_msg, "GOT %d EXP & %d G!", gained_exp, gained_gold);
                                    render_typing_message(renderer, m1, reward_msg, &player, current_monster, difficulty, is_boss, menu_index);
									play_monster_death_effect(renderer, &player, current_monster, difficulty, is_boss, menu_index);

                                    int exp_needed = player.level * 100;
                                    if (player.exp >= exp_needed)
                                    {
                                        player.level++; player.exp -= exp_needed;
                                        player.max_hp += 20; player.hp = player.max_hp;
                                        player.max_mp += 10; player.mp = player.max_mp; player.atk += 5;
                                        char lvl_msg[64]; sprintf(lvl_msg, "LEVEL UP! NOW LV %d", player.level);
                                        render_typing_message(renderer, lvl_msg, "STATS INCREASED!", &player, current_monster, difficulty, is_boss, menu_index);
                                    }

									current_monster->is_alive = 0; player.is_defending = 0;
									
									if (is_boss) 
									{
										g_dungeon_map[saved_map_y][saved_map_x] = TILE_BOSS_CHEST;
										if (saved_map_y - 1 >= 0 && g_dungeon_map[saved_map_y - 1][saved_map_x] != TILE_WALL) g_dungeon_map[saved_map_y - 1][saved_map_x] = TILE_PORTAL;
										else g_dungeon_map[saved_map_y][saved_map_x + 1] = TILE_PORTAL;
										game_state = 99;
									}
									else { g_dungeon_map[saved_map_y][saved_map_x] = TILE_PATH; game_state = 99; }
									current_monster = NULL;
								}

								if (current_monster != NULL && current_monster->hp > 0)
								{
									SDL_Delay(500); char m_dmg_msg[64];
									if (is_boss)
									{
										if (current_monster->mp == 1)
										{
											current_monster->mp = 0;
											int boss_dmg = GET_RAND(current_monster->atk * 2, current_monster->atk * 3);
											if (player.is_defending) { boss_dmg /= 4; sprintf(m1, "BLOCKED DEADLY BLOW!"); } else { sprintf(m1, "DEVASTATING STRIKE!"); }
											player.hp -= boss_dmg; sprintf(m_dmg_msg, "TOOK %d DAMAGE!", boss_dmg);
											play_hit_effect(renderer, &player, current_monster, difficulty, is_boss, menu_index, 1);
											render_typing_message(renderer, m1, m_dmg_msg, &player, current_monster, difficulty, is_boss, menu_index);
										}
										else if (GET_RAND(1, 100) <= 30) { current_monster->mp = 1; sprintf(m1, "THE BOSS IS GATHERING"); render_typing_message(renderer, m1, "DARK ENERGY...", &player, current_monster, difficulty, is_boss, menu_index); }
										else
										{
											int m_dmg = GET_RAND(current_monster->atk, current_monster->atk + 5);
											if (player.is_defending) m_dmg /= 2;
											player.hp -= m_dmg; sprintf(m1, "%s ATTACKED!", current_monster->name); sprintf(m_dmg_msg, "TOOK %d DAMAGE!", m_dmg);
											play_hit_effect(renderer, &player, current_monster, difficulty, is_boss, menu_index, 1);
											render_typing_message(renderer, m1, m_dmg_msg, &player, current_monster, difficulty, is_boss, menu_index);
										}
									}
									else
									{
										int m_dmg = GET_RAND(current_monster->atk - 2, current_monster->atk + 2);
										if (player.is_defending) m_dmg /= 2;
										player.hp -= m_dmg; sprintf(m1, "%s ATTACKED!", current_monster->name); sprintf(m_dmg_msg, "TOOK %d DAMAGE!", m_dmg);
										play_hit_effect(renderer, &player, current_monster, difficulty, is_boss, menu_index, 1);
										render_typing_message(renderer, m1, m_dmg_msg, &player, current_monster, difficulty, is_boss, menu_index);
									}
									player.is_defending = 0;
									if (player.hp <= 0) { player.hp = 0; render_typing_message(renderer, "YOU DIED...", "", &player, current_monster, difficulty, is_boss, menu_index); game_state = 7; }
								}
								break;
							}
							case 1: game_state = 4; menu_index = 0; break;
							case 2: game_state = 5; menu_index = 0; break;
							case 3: 
								{
									player.is_defending = 1; render_typing_message(renderer, "DEFENSE STANCE!", "READY FOR ATTACK.", &player, current_monster, difficulty, is_boss, menu_index);
									int m_dmg = GET_RAND(current_monster->atk - 2, current_monster->atk + 2); m_dmg /= 2; 
									char m1[64], m2[64]; sprintf(m1, "%s ATTACKS!", current_monster->name); sprintf(m2, "BLOCKED! TOOK %d DAMAGE.", m_dmg);
									play_hit_effect(renderer, &player, current_monster, difficulty, is_boss, menu_index, 1);
									player.hp -= m_dmg; player.is_defending = 0; 
									render_typing_message(renderer, m1, m2, &player, current_monster, difficulty, is_boss, menu_index);
									if (player.hp <= 0) { player.hp = 0; render_typing_message(renderer, "YOU DIED...", NULL, &player, current_monster, difficulty, is_boss, menu_index); game_state = 7; }
									break;
								}
							case 4: game_state = 6; menu_index = 0; break;
						}
					}
				}
				else if (game_state == 4) // 스킬 창 로직
				{
					if (KEY == SDLK_UP) { if(menu_index > 0) menu_index--; }
					else if (KEY == SDLK_DOWN) { if(menu_index < current_skill_count - 1) menu_index++; }
					else if (KEY == SDLK_RETURN)
					{
						int skill_dmg = 0; int mp_cost = 0;
                        int total_atk = player.atk + (player.equipped_weapon_idx >= 0 ? player.weapons[player.equipped_weapon_idx].atk_bonus : 0);

						if (current_skills == warrior_skills) { mp_cost = 15; skill_dmg = total_atk * 2; }
						else { mp_cost = 25; skill_dmg = total_atk * 3; }

						if (player.mp >= mp_cost)
						{
							player.mp -= mp_cost; current_monster->hp -= skill_dmg;
							char m1[64], m2[64]; sprintf(m1, "USED %s!", current_skills[menu_index]); sprintf(m2, "DEALT %d DAMAGE!", skill_dmg);
							play_hit_effect(renderer, &player, current_monster, difficulty, is_boss, 1, 0);
							render_typing_message(renderer, m1, m2, &player, current_monster, difficulty, is_boss, 1);

							if (current_monster->hp <= 0)
							{
								current_monster->hp = 0; sprintf(m1, "DEFEATED %s!", current_monster->name);
                                int gained_exp = current_monster->exp_reward; int gained_gold = current_monster->gold_reward;
                                player.exp += gained_exp; player.gold += gained_gold;

                                char reward_msg[64]; sprintf(reward_msg, "GOT %d EXP & %d G!", gained_exp, gained_gold);
                                render_typing_message(renderer, m1, reward_msg, &player, current_monster, difficulty, is_boss, 1);
								play_monster_death_effect(renderer, &player, current_monster, difficulty, is_boss, 1);
								
                                int exp_needed = player.level * 100;
                                if (player.exp >= exp_needed)
                                {
                                    player.level++; player.exp -= exp_needed; player.max_hp += 20; player.hp = player.max_hp;
                                    player.max_mp += 10; player.mp = player.max_mp; player.atk += 5;
                                    char lvl_msg[64]; sprintf(lvl_msg, "LEVEL UP! NOW LV %d", player.level);
                                    render_typing_message(renderer, lvl_msg, "STATS INCREASED!", &player, current_monster, difficulty, is_boss, 1);
                                }

								g_dungeon_map[saved_map_y][saved_map_x] = TILE_PATH;
								current_monster->is_alive = 0; current_monster = NULL; player.is_defending = 0;
                                if (is_boss) { if (saved_map_y - 1 >= 0) g_dungeon_map[saved_map_y - 1][saved_map_x] = TILE_PORTAL; game_state = 99; }
                                else game_state = 99;
							}
							else
							{
								int m_dmg = GET_RAND(current_monster->atk - 2, current_monster->atk + 2);
								sprintf(m1, "%s ATTACKS!", current_monster->name); sprintf(m2, "TOOK %d DAMAGE.", m_dmg);
								play_hit_effect(renderer, &player, current_monster, difficulty, is_boss, 1, 1);
								player.hp -= m_dmg; render_typing_message(renderer, m1, m2, &player, current_monster, difficulty, is_boss, 1);
								if (player.hp <= 0) { player.hp = 0; render_typing_message(renderer, "YOU DIED...", NULL, &player, current_monster, difficulty, is_boss, 1); game_state = 7; }
								else { game_state = 3; }
							}
							menu_index = 0;
						}						
						else { render_typing_message(renderer, "NOT ENOUGH MP!", NULL, &player, current_monster, difficulty, is_boss, 1); }
					}					
					else if (KEY == SDLK_ESCAPE) { game_state = 3; menu_index = 1; }
				}
				else if (game_state == 5) // 아이템 창
				{
					if (KEY == SDLK_UP) { if(menu_index > 0) menu_index--; }
					else if (KEY == SDLK_DOWN) { if(menu_index < item_menu_count - 1) menu_index++; }
					else if (KEY == SDLK_RETURN)
					{
						int item_used = 0; char m1[64], m2[64];
						if (menu_index == 0) 
						{
							if (hp_potions > 0) { hp_potions--; player.hp += 50; if (player.hp > player.max_hp) player.hp = player.max_hp; sprintf(m1, "USED HP POTION!"); sprintf(m2, "RECOVERED HP! LEFT: %d", hp_potions); item_used = 1; }
							else { render_typing_message(renderer, "NO HP POTIONS!", NULL, &player, current_monster, difficulty, is_boss, 2); }
						}
						else if (menu_index == 1) 
						{
							if(mp_potions > 0) { mp_potions--; player.mp += 30; if (player.mp > player.max_mp) player.mp = player.max_mp; sprintf(m1, "USED MP POTION!"); sprintf(m2, "RECOVERED MP! LEFT: %d", mp_potions); item_used = 1; }
							else { render_typing_message(renderer, "NO MP POTIONS!", NULL, &player, current_monster, difficulty, is_boss, 2); }
						}

						if (item_used)
						{
							render_typing_message(renderer, m1, m2, &player, current_monster, difficulty, is_boss, 2);
							int m_dmg = GET_RAND(current_monster->atk - 2, current_monster->atk + 2);
							sprintf(m1, "%s ATTACKS!", current_monster->name); sprintf(m2, "TOOK %d DAMAGE.", m_dmg);
							play_hit_effect(renderer, &player, current_monster, difficulty, is_boss, 2, 1);
							player.hp -= m_dmg; render_typing_message(renderer, m1, m2, &player, current_monster, difficulty, is_boss, 2);
							if (player.hp <= 0) { player.hp = 0; render_typing_message(renderer, "YOU DIED...", NULL, &player, current_monster, difficulty, is_boss, 2); game_state = 7; }
							else { game_state = 3; } 
						}
						menu_index = 0;
					}
					else if (KEY == SDLK_ESCAPE) { game_state = 3; menu_index = 2; }
				}
				else if (game_state == 6)
				{
					if (KEY == SDLK_UP) { if(menu_index > 0) menu_index--; }
					else if (KEY == SDLK_DOWN) { if(menu_index < 1) menu_index++; }
					else if (KEY == SDLK_RETURN)
					{
						if(menu_index == 0) { render_typing_message(renderer, "RAN AWAY SAFELY!", NULL, &player, current_monster, difficulty, is_boss, 4); if (current_monster != NULL) { if (!is_boss) current_monster->ignore_turns = 5; else current_monster->ignore_turns = 0; } current_monster = NULL; game_state = 99; } 
						else { game_state = 3; }
						menu_index = 0;
					}		
				}
				else if (game_state == 7)
				{
					if (KEY == SDLK_RETURN)
					{
                        if (player.hp > 0) {
                            g_save_slots[0].is_empty = 0; g_save_slots[0].player_data = player; 
                            g_save_slots[0].hp_potions = hp_potions; g_save_slots[0].mp_potions = mp_potions;
                            write_save_data();
                        }
						game_state = 0; menu_index = 0;
					}
				}
			}
		}

		/* 화면 초기화 */
		SET_COLOR(0,0,0);
		SDL_RenderClear(renderer);          

		/* 메인 메뉴 */
		if (game_state == 0)
		{
            // [UI 수정 위치: 타이틀 윈도우 배경]
			draw_dw_window(renderer, 150, 80, 500, 120);
			render_text(renderer, "RPG PROJECT", 285, 125, 255, 255, 255);

            // [UI 수정 위치: 타이틀 메뉴 배경]
			draw_dw_window(renderer, 300, 280, 200, 160); 

			if (menu_index == 0) render_text(renderer, "START", 360, 310, 255, 255, 0); else render_text(renderer, "START", 360, 310, 255, 255, 255);
			if (menu_index == 1) render_text(renderer, "LOAD", 360, 355, 255, 255, 0); else render_text(renderer, "LOAD", 360, 355, 255, 255, 255);
			if (menu_index == 2) render_text(renderer, "EXIT", 360, 400, 255, 255, 0); else render_text(renderer, "EXIT", 360, 400, 255, 255, 255);

			int cursor_y = (menu_index == 0) ? 310 : ((menu_index == 1) ? 355 : 400);
			render_text(renderer, ">", 335, cursor_y, 255, 255, 0);
		}
		
		/* 캐릭터 선택 */
		else if (game_state == 1)
		{
            // [UI 수정 위치: 캐릭터 선택 상단 제목]
			draw_dw_window(renderer, 30, 40, 740, 80);
			render_text(renderer, "CHOOSE THY PROFESSION", 170, 65, 255, 255, 255);

            // [UI 수정 위치: 전사 정보 박스]
			draw_dw_window(renderer, 30, 140, 360, 280);
			if (menu_index == 0) { render_text(renderer, ">", 50, 160, 255, 255, 0); render_text(renderer, "WARRIOR", 80, 160, 255, 255, 0); }
			else render_text(renderer, "WARRIOR", 80, 160, 255, 255, 255);
			render_text(renderer, "HP : 150", 80, 230, 200, 200, 200); render_text(renderer, "MP : 30", 80, 280, 200, 200, 200); render_text(renderer, "ATK: 30", 80, 330, 200, 200, 200);

            // [UI 수정 위치: 마법사 정보 박스]
			draw_dw_window(renderer, 410, 140, 360, 280);
			if (menu_index == 1) { render_text(renderer, ">", 430, 160, 255, 255, 0); render_text(renderer, "MAGE", 460, 160, 255, 255, 0); }
			else render_text(renderer, "MAGE", 460, 160, 255, 255, 255);
			render_text(renderer, "HP : 80", 460, 230, 200, 200, 200); render_text(renderer, "MP : 120", 460, 280, 200, 200, 200); render_text(renderer, "ATK: 10", 460, 330, 200, 200, 200);

            // [UI 수정 위치: 직업 설명 박스]
			draw_dw_window(renderer, 30, 440, 740, 130);
			if (menu_index == 0) { render_text(renderer, "STURDY OF BODY,SWIFT OF BLADE.", 50, 470, 255, 255, 255); render_text(renderer, "A TRUE CHAMPION FOR THEE.", 50, 515, 255, 255, 255); }
			else { render_text(renderer, "MASTER OF ANCIENT SPELLS.", 50, 470, 255, 255, 255); render_text(renderer, "FRAGILE, YET WISE IN MIND.", 50, 515, 255, 255, 255); }
		}
		else if (game_state == 2)
		{
            // [UI 수정 위치: 난이도 선택 제목]
			draw_dw_window(renderer, 100, 40, 600, 80); render_text(renderer, "WHITHER ART THOU BOUND?", 130, 65, 255, 255, 255);

            // [UI 수정 위치: 난이도 선택 리스트]
			draw_dw_window(renderer, 180, 160, 440, 360);
			for (int i = 0; i < diff_count; i++)
			{
				if (menu_index == i) { render_text(renderer, ">", 210, 210 + (i * 100), 255, 255, 0); render_text(renderer, diff_names[i], 250, 210 + (i * 100), 255, 255, 0); }
				else render_text(renderer, diff_names[i], 250, 210 + (i * 100), 255, 255, 255);
			}
		}
		else if (game_state == 99)
		{
			/* 타일 원본 크기(ts) 및 화면 출력 크기(ss) 설정 */
			int ts = 16; int ss = 48;
			int ox = (800 - (17 * ss)) / 2; int oy = (600 - (13 * ss)) / 2;

			for (int cy = 0; cy < 13; cy++)
			{
				for (int cx = 0; cx < 17; cx++)
				{
					int mx = player.x - 8 + cx; int my = player.y - 6 + cy;

					if (mx >= 0 && mx < MAP_WIDTH && my >= 0 && my < MAP_HEIGHT)
					{
						int t = g_dungeon_map[my][mx];
						SDL_Rect d_r = { ox + cx * ss, oy + cy * ss, ss, ss };

						/* 1. 맵 타일 렌더링 */
						if (t == TILE_WALL) { SDL_Rect s_r = { (difficulty == 0 ? 64 : (difficulty == 1 ? 112 : 0)), (difficulty == 0 ? 176 : (difficulty == 1 ? 16 : 0)), 16, 16 }; SDL_RenderCopy(renderer, map_tex, &s_r, &d_r); }
						else
						{
							SDL_Rect s_r = { 16, (difficulty == 0 ? 128 : (difficulty == 1 ? 144 : 0)), 16, 16 }; SDL_RenderCopy(renderer, map_tex, &s_r, &d_r);

							if (t == TILE_STAIRS) { SDL_Rect st = {0, 112, 16, 16}; SDL_RenderCopy(renderer, map_tex, &st, &d_r); }
                            else if (t == TILE_PORTAL) { SDL_Rect po = {0, 64, 16, 16}; SDL_RenderCopy(renderer, map_tex, &po, &d_r); }
                            else if (t == TILE_CHEST) { SDL_Rect ch = {64, 64, 16, 16}; SDL_RenderCopy(renderer, map_tex, &ch, &d_r); }
                            else if (t == TILE_BOSS_CHEST) { SDL_Rect bch = {64, 64, 16, 16}; SDL_RenderCopy(renderer, map_tex, &bch, &d_r); }
                            else if (t == TILE_NPC) { SDL_Rect npc = {112, 0, 16, 16}; SDL_RenderCopy(renderer, char_tex, &npc, &d_r); }
                            else if (t == TILE_TORCH) { SDL_Rect to = {0, 144, 16, 16}; SDL_RenderCopy(renderer, map_tex, &to, &d_r); }
							else if (t == TILE_MONSTER || t == TILE_BOSS)
							{
								int msx = 0; if (t == TILE_BOSS) msx = 96; else msx = (difficulty == 0 ? 0 : (difficulty == 1 ? 48 : 144));
								SDL_Rect m_s = { msx + (player.anim_frame * 16), (t == TILE_BOSS ? 64 : (difficulty == 2 ? 0 : 64)), 16, 16 };
								SDL_RenderCopy(renderer, char_tex, &m_s, &d_r);
							}
						}
					}
				}
			}
			SDL_Rect cp_s = { (player.anim_frame * 16), (player.dir * 16), 16, 16 };
			SDL_Rect cp_d = { ox + 8 * ss, oy + 6 * ss, ss, ss };
			SDL_RenderCopy(renderer, char_tex, &cp_s, &cp_d);
		}
		else if (game_state == 3)
		{
			render_battle_scene(renderer, &player, current_monster, difficulty, is_boss, menu_index, 0);
            // [UI 수정 위치: 전투 화면 - 기본 메시지 윈도우 배경]
			draw_dw_window(renderer, 100, 420, 600, 140);
			if (g_msg1[0] != '\0') render_text(renderer, g_msg1, 130, 460, 255, 255, 255);
			if (g_msg2[0] != '\0') render_text(renderer, g_msg2, 130, 500, 255, 255, 255);
		}
		else if (game_state >= 4 && game_state <= 6)
		{
			SET_COLOR(30, 10, 10); SDL_Rect bg = {0, 0, 800, 600}; FILL_RECT(bg);

			if (game_state == 4) SET_COLOR(20, 40, 80); else if (game_state == 5) SET_COLOR(20, 80, 40); else SET_COLOR(60, 40, 20);

            // [UI 수정 위치: 전투 서브메뉴(스킬/아이템) 팝업창]
			SDL_Rect popup = {230, 200, 350, 200}; FILL_RECT(popup);

			if (game_state == 4)
			{
				render_text(renderer, "--- SKILL ---", 255, 210, 255, 255, 255);
				for (int i = 0; i < 2; i++) { SDL_Rect opt = {260, 260 + (i * 60), 300, 40}; if (menu_index == i) SET_COLOR(255, 255, 0); else SET_COLOR(150, 150, 150); FILL_RECT(opt); render_text(renderer, current_skills[i], 265, 270 + (i * 60), 0, 0, 0); }
			}
			else if (game_state == 5)
			{
				render_text(renderer, "--- ITEM ---", 255, 210, 255, 255, 255);
				for (int i = 0; i < 2; i++) { SDL_Rect opt = {260, 260 + (i * 60), 280, 40}; if (menu_index == i) SET_COLOR(255, 255, 0); else SET_COLOR(150, 150, 150); FILL_RECT(opt); render_text(renderer, item_menu[i], 265, 270 + (i * 60), 0, 0, 0); }
			}
			else if (game_state == 6)
			{
				render_text(renderer, "RUN AWAY?", 280, 230, 255, 255, 255); char* r_list[] = {"1. YES", "2. NO"};
				for (int i = 0; i < 2; i++) { SDL_Rect opt = {300, 280 + (i * 50), 200, 40}; if (menu_index == i) SET_COLOR(255, 255, 0); else SET_COLOR(150, 150, 150); FILL_RECT(opt); render_text(renderer, r_list[i], 350, 290 + (i * 50), 0, 0, 0); }
			}
		}
		else if (game_state == 7)
		{
			SDL_Rect full_bg = {0, 0, 800, 600};
			if (player.hp <= 0) SET_COLOR(100, 0, 0); else SET_COLOR(0, 50, 100); FILL_RECT(full_bg);

            // [UI 수정 위치: 게임 오버/클리어 중앙 메시지 박스]
			draw_dw_window(renderer, 155, 200, 505, 200);

			if (player.hp <= 0) { render_text(renderer, "YOU DIED...", 330, 240, 255, 50, 50); }
			else { render_text(renderer, "YOU WIN!", 300, 230, 50, 255, 50); render_text(renderer, diff_names[difficulty], 255, 280, 255, 255, 255); render_text(renderer, "STAGE CLEARED!", 245, 320, 255, 255, 0); }
			render_text(renderer, "PRESS ENTER TO RETURN", 170, 465, 255, 255, 255);
		}
        else if (game_state == STATE_SAVE_MENU || game_state == STATE_LOAD_MENU)
        {
            // [UI 수정 위치: 세이브/로드 메뉴]
            draw_dw_window(renderer, 100, 50, 600, 500);
            const char* title = (game_state == STATE_SAVE_MENU) ? "--- SAVE DATA ---" : "--- LOAD DATA ---";
            render_text(renderer, title, 280, 80, 255, 255, 0);

            for (int i = 0; i < 5; i++)
            {
                int slot_y = 140 + (i * 60);
                if (menu_index == i) render_text(renderer, ">", 120, slot_y, 255, 255, 0);

                char slot_info[128];
                if (g_save_slots[i].is_empty) { sprintf(slot_info, "[SLOT %d] - EMPTY -", i + 1); render_text(renderer, slot_info, 150, slot_y, 150, 150, 150); }
                else
                {
                    const char* c_name = (g_save_slots[i].player_data.class_type == 0) ? "WARRIOR" : "MAGE";
                    sprintf(slot_info, "[SLOT %d] LV:%d %s F:%d", i + 1, g_save_slots[i].player_data.level, c_name, g_save_slots[i].current_floor);
                    render_text(renderer, slot_info, 150, slot_y, 255, 255, (menu_index == i ? 0 : 255));
                }
            }
            render_text(renderer, "PRESS ESC TO CANCEL", 260, 500, 200, 200, 200);
        }
        else if (game_state == STATE_ESC_MENU) 
        {
            // [UI 수정 위치: 던전 탐험 중 ESC 메뉴 박스]
            draw_dw_window(renderer, 250, 150, 300, 350);
            render_text(renderer, "--- MENU ---", 320, 180, 255, 255, 255);

            render_text(renderer, "STATUS", 310, 230, 255, 255, (menu_index == 0 ? 0 : 255));
            render_text(renderer, "EQUIPMENT", 310, 270, 255, 255, (menu_index == 1 ? 0 : 255));
            render_text(renderer, "INVENTORY", 310, 310, 255, 255, (menu_index == 2 ? 0 : 255));
            render_text(renderer, "SAVE DATA", 310, 350, 255, 255, (menu_index == 3 ? 0 : 255));
            render_text(renderer, "QUIT TO TITLE", 310, 390, 255, 255, (menu_index == 4 ? 0 : 255));
            render_text(renderer, "RETURN TO GAME", 310, 430, 255, 255, (menu_index == 5 ? 0 : 255));

            int cursor_y = 230 + (menu_index * 40);
            render_text(renderer, ">", 280, cursor_y, 255, 255, 0);
        }
        else if (game_state == STATE_STATUS_MENU) 
        {
            // [UI 수정 위치: ESC 하위 - 상태창]
            draw_dw_window(renderer, 200, 100, 400, 400);
            render_text(renderer, "-- STATUS --", 330, 130, 255, 255, 0);
            
            char buf[64];
            sprintf(buf, "CLASS: %s", (player.class_type == 0 ? "WARRIOR" : "MAGE")); render_text(renderer, buf, 240, 180, 255, 255, 255);
            sprintf(buf, "LEVEL: %d", player.level); render_text(renderer, buf, 240, 220, 255, 255, 255);
            sprintf(buf, "EXP  : %d / %d", player.exp, player.level * 100); render_text(renderer, buf, 240, 260, 255, 255, 255);
            sprintf(buf, "GOLD : %d G", player.gold); render_text(renderer, buf, 240, 300, 255, 255, 255);
            sprintf(buf, "HP   : %d / %d", player.hp, player.max_hp); render_text(renderer, buf, 240, 340, 255, 255, 255);
            sprintf(buf, "MP   : %d / %d", player.mp, player.max_mp); render_text(renderer, buf, 240, 380, 255, 255, 255);
            
            int total_atk = player.atk + (player.equipped_weapon_idx >= 0 ? player.weapons[player.equipped_weapon_idx].atk_bonus : 0);
            sprintf(buf, "ATK  : %d", total_atk); render_text(renderer, buf, 240, 420, 255, 255, 255);
            
            render_text(renderer, "PRESS ESC TO RETURN", 280, 470, 200, 200, 200);
        }
        else if (game_state == STATE_EQUIP_MENU) 
        {
            // [UI 수정 위치: 장비창 왼쪽 - 보유 장비 리스트]
            draw_dw_window(renderer, 50, 50, 350, 500);
            render_text(renderer, "-- WEAPONS --", 120, 80, 255, 255, 0);
            
            for (int i = 0; i < player.weapon_count; i++)
            {
                int item_y = 130 + (i * 40);
                if (menu_index == i) render_text(renderer, ">", 70, item_y, 255, 255, 0);
                
                int r = 255, g = 255, b = 255;
                if (player.weapons[i].rarity == RARITY_RARE) { r = 100; g = 150; b = 255; }
                else if (player.weapons[i].rarity == RARITY_EPIC) { r = 200; g = 50; b = 200; }
                
                char w_name[64];
                if (player.equipped_weapon_idx == i) sprintf(w_name, "[E] %s", player.weapons[i].name);
                else sprintf(w_name, "    %s", player.weapons[i].name);
                
                render_text(renderer, w_name, 100, item_y, r, g, b);
            }

            // [UI 수정 위치: 장비창 오른쪽 - 장착 미리보기 스탯창]
            draw_dw_window(renderer, 420, 50, 330, 500);
            render_text(renderer, "-- PREVIEW --", 500, 80, 255, 255, 0);

            if (player.weapon_count > 0)
            {
                int current_bonus = (player.equipped_weapon_idx >= 0) ? player.weapons[player.equipped_weapon_idx].atk_bonus : 0;
                int hover_bonus = player.weapons[menu_index].atk_bonus;
                int diff = hover_bonus - current_bonus;
                int new_total = player.atk + hover_bonus;

                char p_buf[64];
                sprintf(p_buf, "CURRENT ATK: %d", player.atk + current_bonus);
                render_text(renderer, p_buf, 450, 180, 200, 200, 200);
                
                if (diff >= 0) sprintf(p_buf, "AFTER EQUIP: %d (+%d)", new_total, diff);
                else sprintf(p_buf, "AFTER EQUIP: %d (%d)", new_total, diff);
                
                int p_r = (diff > 0) ? 50 : (diff < 0 ? 255 : 255);
                int p_g = (diff > 0) ? 255 : (diff < 0 ? 50 : 255);
                int p_b = (diff > 0) ? 50 : (diff < 0 ? 50 : 255);
                render_text(renderer, p_buf, 450, 240, p_r, p_g, p_b);
            }

            render_text(renderer, "ENTER : EQUIP", 500, 440, 255, 255, 0);
            render_text(renderer, "ESC : RETURN", 500, 480, 150, 150, 150);
        }
        else if (game_state == STATE_INVEN_MENU) 
        {
            // [UI 수정 위치: ESC 하위 - 인벤토리]
            draw_dw_window(renderer, 200, 150, 400, 300);
            render_text(renderer, "-- INVENTORY --", 310, 180, 255, 255, 0);
            char buf[64];
            sprintf(buf, "HP POTIONS: %d", hp_potions); render_text(renderer, buf, 280, 250, 255, 255, 255);
            sprintf(buf, "MP POTIONS: %d", mp_potions); render_text(renderer, buf, 280, 300, 255, 255, 255);
            render_text(renderer, "PRESS ESC TO RETURN", 280, 410, 200, 200, 200);
        }
	else if (game_state == STATE_SHOP_UI)
        {
            // [UI 수정 위치: 상점창]
            draw_dw_window(renderer, 150, 150, 500, 300);
            render_text(renderer, "--- MERCHANT ---", 330, 180, 255, 255, 255);
            
            char price_buf[64];
            sprintf(price_buf, "BUY HP POTION (50G)"); render_text(renderer, price_buf, 210, 240, 255, 255, (menu_index == 0 ? 0 : 255));
            sprintf(price_buf, "BUY MP POTION (100G)"); render_text(renderer, price_buf, 210, 290, 255, 255, (menu_index == 1 ? 0 : 255));
            
            // 등급별 무기 이름 및 색상 설정
            sprintf(price_buf, "BUY %s (300G)", g_current_shop_weapon.name);
            int w_r = 255, w_g = 255, w_b = 255;
            if (g_current_shop_weapon.rarity == RARITY_RARE) { w_r = 100; w_g = 150; w_b = 255; }
            else if (g_current_shop_weapon.rarity == RARITY_EPIC) { w_r = 200; w_g = 50; w_b = 200; }
            if (menu_index == 2) { w_r = 255; w_g = 255; w_b = 0; } 
            
            render_text(renderer, price_buf, 210, 340, w_r, w_g, w_b);
            render_text(renderer, "LEAVE", 210, 390, 255, 255, (menu_index == 3 ? 0 : 255));
            
            int cursor_y = 240 + (menu_index * 50);
            render_text(renderer, ">", 180, cursor_y, 255, 255, 0);

            // [UI 수정 위치: 골드 표시 박스 - 여기가 수정된 부분이야!]
            draw_dw_window(renderer, 670, 150, 110, 100);
            render_text(renderer, "GOLD", 690, 170, 255, 255, 255);
            sprintf(price_buf, "%d G", player.gold); 
            render_text(renderer, price_buf, 690, 210, 255, 255, 0); // price_buf를 인자로 꼭 넣어줘써어!
        }
		draw_scanlines(renderer, 800, 600);
		SDL_RenderPresent(renderer);
	}

	SDL_DestroyRenderer(renderer); SDL_DestroyWindow(window);
	TTF_Quit(); IMG_Quit(); SDL_Quit();
	return 0;
}
