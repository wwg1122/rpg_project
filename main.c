#include <SDL2/SDL.h>		//SDL2 헤더 파일
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
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


// 전투씬 배경 유지 렌더러 (애니메이션 재생 시 배경 깜빡임 방지용)
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
		// 박쥐(2단계)와 해골(3단계) 이미지 좌표 스왑 반영
		else if (difficulty == 1) { msx = 48; msy = 64; }
		else { msx = 144; msy = 0; } 

		SDL_Rect m_src = { msx, msy, 16, 16 };
		SDL_Rect m_dst;
		if (is_boss) 
        	{
            		m_dst.x = 225; m_dst.y = 70; m_dst.w = 350; m_dst.h = 350;
        	}
        	else 
        	{
            		m_dst.x = 275; m_dst.y = 120; m_dst.w = 250; m_dst.h = 250;
        	}

		SDL_RenderCopy(renderer, char_tex, &m_src, &m_dst);

		char m_hp_str[32];
		sprintf(m_hp_str, "HP: %d/%d", current_monster->hp, current_monster->max_hp);
		render_text(renderer, m_hp_str, 340, 380, 255, 100, 100);
	}

	draw_dw_window(renderer, 20, 20, 230, 260);
	render_text(renderer, "COMMAND", 50, 40, 255, 255, 255);
	for (int i = 0; i < 5; i++)
	{
		if (menu_index == i) render_text(renderer, ">", 35, 90 + (i * 35), 255, 255, 0);
		render_text(renderer, battle_menu[i], 60, 90 + (i * 35), (menu_index == i ? 255 : 255), (menu_index == i ? 255 : 255), (menu_index == i ? 0 : 255));
	}

	draw_dw_window(renderer, 550, 20, 220, 200);
	render_text(renderer, "STATUS", 600, 40, 255, 255, 255);

	char stat_p[32];
	sprintf(stat_p, "HP: %d", player->hp); 
	render_text(renderer, stat_p, 580, 90, 255, 255, 255);
	sprintf(stat_p, "MP: %d", player->mp); 
	render_text(renderer, stat_p, 580, 130, 255, 255, 255);
	sprintf(stat_p, "ATK: %d", player->atk); 
	render_text(renderer, stat_p, 580, 170, 255, 255, 255);
}

// 드퀘 감성 타자 치는 효과 텍스트 박스
// printf 대신 화면 하단 메시지 창에 한 글자씩 출력하며 딜레이를 부여
void render_typing_message(SDL_Renderer* renderer, const char* text1, const char* text2, Character* player, Enemy* current_monster, int difficulty, int is_boss, int menu_index)
{
	char buf1[128] = {0};
	char buf2[128] = {0};
	int i = 0, j = 0;

	if (text1 != NULL) 
	{
		while (text1[i] != '\0') 
		{
			if ((text1[i] & 0x80) == 0) { buf1[j++] = text1[i++]; }
			else { buf1[j++] = text1[i++]; buf1[j++] = text1[i++]; buf1[j++] = text1[i++]; }
			buf1[j] = '\0';
			
			render_battle_scene(renderer, player, current_monster, difficulty, is_boss, menu_index, 0);
			draw_dw_window(renderer, 100, 420, 600, 140);
			render_text(renderer, buf1, 130, 460, 255, 255, 255);
			SDL_RenderPresent(renderer);
			SDL_Delay(35); // 한 글자당 출력 속도
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
			draw_dw_window(renderer, 100, 420, 600, 140);
			if (text1 != NULL) render_text(renderer, buf1, 130, 460, 255, 255, 255);
			render_text(renderer, buf2, 130, 500, 255, 255, 255);
			SDL_RenderPresent(renderer);
			SDL_Delay(35);
		}
	}
	SDL_Delay(800); // 텍스트 출력이 완료된 후 사용자가 읽을 수 있도록 대기 시간 부여

	if (text1 != NULL) { sprintf(g_msg1, "%s", text1); } else { g_msg1[0] = '\0'; }
	if (text2 != NULL) { sprintf(g_msg2, "%s", text2); } else { g_msg2[0] = '\0'; }
}

// 피격 시 화면 깜빡임 연출
// 플레이어 피격 시 붉은 화면, 몬스터 피격 시 하얀 화면으로 깜빡임 효과 제공
void play_hit_effect(SDL_Renderer* renderer, Character* player, Enemy* current_monster, int difficulty, int is_boss, int menu_index, int is_player_hit)
{
	for (int i = 0; i < 2; i++) 
	{
		render_battle_scene(renderer, player, current_monster, difficulty, is_boss, menu_index, 0);
		
		SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
		if (is_player_hit) SDL_SetRenderDrawColor(renderer, 255, 0, 0, 120); 
		else SDL_SetRenderDrawColor(renderer, 255, 255, 255, 120); 
		
		SDL_Rect bg = {0, 0, 800, 600};
		SDL_RenderFillRect(renderer, &bg);
		SDL_RenderPresent(renderer);
		SDL_Delay(50);
		
		render_battle_scene(renderer, player, current_monster, difficulty, is_boss, menu_index, 0);
		SDL_RenderPresent(renderer);
		SDL_Delay(50);
	}
}

// 몬스터 사망 시 투명도 조절하며 소멸 (파스스 효과)
// 알파값을 점진적으로 감소시키고 y좌표를 하강시켜 무너져 내리는 듯한 연출
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
		
		m_dst.y += 2; 
		SDL_SetTextureAlphaMod(char_tex, alpha);
		SDL_RenderCopy(renderer, char_tex, &m_src, &m_dst);
		
		SDL_RenderPresent(renderer);
		SDL_Delay(30);
	}
	SDL_SetTextureAlphaMod(char_tex, 255); 
}

// 계단 이용 시 화면 페이드아웃 연출
void play_stair_effect(SDL_Renderer* renderer)
{
	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
	for (int a = 0; a <= 255; a += 15) 
	{
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, a);
		SDL_Rect bg = {0, 0, 800, 600};
		SDL_RenderFillRect(renderer, &bg);
		SDL_RenderPresent(renderer);
		SDL_Delay(20);
	}
	SDL_Delay(200); 
}


int main(int argc, char *argv[])
{	
	//배열 크기 변수 설정
	int diff_count = sizeof(diff_names) / sizeof(diff_names[0]);
	int current_skill_count = 0;
	int battle_menu_count = sizeof(battle_menu) / sizeof(battle_menu[0]);
	int item_menu_count = sizeof(item_menu) / sizeof(item_menu[0]);
	srand(time(NULL));

	int difficulty = 0;
	int hp_potions = 2;
	int mp_potions = 2;
	int current_floor = 1;
	int max_floor = 3;	

	//시스템 초기화 후 부팅 확인 여부 판단
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{				//시스템 에러 확인
		printf("초기화 실패 에러내용: %s\n", SDL_GetError());	//에러 이유 출력
		return 1;
	}

	if (TTF_Init() == -1) 
	{
		printf("폰트 초기화 실패: %s\n", TTF_GetError());
		return 1;
	}

	g_font = TTF_OpenFont("dq_font.ttf", 24);
	if (g_font == NULL)
	{
		/* 파일 로드 실패 시 에러 메시지 출력 후 종료 방지 또는 처리 */
		printf("폰트 로드 실패 (dq_font.ttf): %s\n", TTF_GetError());
	}

	//윈도우 창 생성 설정	
	SDL_Window *window = SDL_CreateWindow(
			"RPG Project",			//창 제목
			SDL_WINDOWPOS_CENTERED,		//창 가로 크기
			SDL_WINDOWPOS_CENTERED,		//창 세로 크기
			800, 600,			
			SDL_WINDOW_SHOWN 		
			);

	//윈도우 창 생성 성공 여부 관련 코드
	if (window == NULL)
	{
		printf("윈도우 생성 실패 에러내용: %s\n",SDL_GetError());
		SDL_Quit();
		return 1;
	}

	//시스템 렌더 생성
	SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

	map_tex = IMG_LoadTexture(renderer, "map_tiles.png");
	char_tex = IMG_LoadTexture(renderer, "master_tiles.png");

	if (map_tex == NULL || char_tex == NULL)
	{
		printf("이미지 로드 실패\n");
		return 1;
	}

	SDL_SetTextureBlendMode(map_tex, SDL_BLENDMODE_BLEND);
	SDL_SetTextureBlendMode(char_tex, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

	//사용자 정의 변수
	int menu_index = 0;	//**아마 0,1 로 게임 시작과 종료를 해둘 예정
	int game_state = 0;
	int is_boss = 0;

	//시스템 이벤트 루프 명령어
	int quit = 0;
	SDL_Event e;

	//사용자 정의 데이터(캐릭터)
	Character warrior = {150, 150, 30, 30, 300, 0, 0, 0};	//체력,최대체력,마나,최대마나,공격력
	Character mage = {80, 80, 120, 120, 10, 0, 0, 0};
	Character player;				//실제 플레이어 데이터 설정

	Enemy *current_monster = NULL; 			//동적할당으로 변경(2026-02-13)

	while(!quit)
	{
		if (game_state == 99)
		{
			player.anim_timer++;
			if (player.anim_timer > 12)
			{
				player.anim_frame = !player.anim_frame;
				player.anim_timer = 0;
			}

			if (player.is_moving)
			{
				int target_px = player.x * 48;
				int target_py = player.y * 48;
				int move_speed = 4;

				if (player.pixel_x < target_px) player.pixel_x += move_speed;
				else if (player.pixel_x > target_px) player.pixel_x -= move_speed;

				if (player.pixel_y < target_py) player.pixel_y += move_speed;
				else if (player.pixel_y > target_py) player.pixel_y -= move_speed;

				if (player.pixel_x == target_px && player.pixel_y == target_py)
				{
					player.is_moving = 0;
					update_all_monsters_ai(player.x, player.y);

					if (g_dungeon_map[player.y][player.x] == TILE_STAIRS)
					{
						// 계단 암전 연출 적용
						play_stair_effect(renderer);
						current_floor++;
						init_map(difficulty, current_floor, max_floor, &player);
						player.pixel_x = player.x * 48;
						player.pixel_y = player.y * 48;
					}
				}
			}
		}

		while (SDL_PollEvent(&e) != 0)
		{
			if (e.type == SDL_QUIT)
			{
				quit = 1;
			}
			else if (e.type == SDL_KEYDOWN)
			{
				if (KEY == SDLK_ESCAPE)
				{
					// 스킬(4), 아이템(5), 도망(6) 메뉴일 때는 전투 메인(3)으로 복귀
					if (game_state == 4 || game_state == 5 || game_state == 6)
					{
						if (game_state == 4) menu_index = 1;      // 스킬 메뉴였으면 스킬 칸으로
						else if (game_state == 5) menu_index = 2; // 아이템 메뉴였으면 아이템 칸으로
						else if (game_state == 6) menu_index = 4; // 도망 메뉴였으면 도망 칸으로
						game_state = 3;
					}
					// 그 외 상태(맵 탐험, 선택창 등)일 때만 메인화면(0)으로 탈출
					else 
					{
						game_state = 0;
						menu_index = 0;
					}
					continue; // ESC 처리가 끝났으므로 아래쪽 state별 입력 로직 중복 실행 방지
				}	

				if (game_state == 0)
				{
					if (KEY == SDLK_UP) { menu_index = 0; }
					else if (KEY == SDLK_DOWN) { menu_index = 1; }
					else if (KEY == SDLK_RETURN)
					{
						if (menu_index == 0) { game_state = 1; menu_index = 0; }
						else { quit = 1; }
					}
				}
				else if (game_state == 1)
				{
					if (KEY == SDLK_LEFT) { menu_index = 0; }
					else if (KEY == SDLK_RIGHT) { menu_index = 1; }
					else if (KEY == SDLK_RETURN)
					{
						if (menu_index == 0)
						{
							player = warrior;
							current_skills = warrior_skills;
							current_skill_count = 2;
						}
						else
						{
							player = mage;
							current_skills = mage_skills;
							current_skill_count = 2;
						}
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
						//나가거나 다시 입장 할 때 체력 초기화 및 포션 보충 로직
						player.hp = player.max_hp;
						player.mp = player.max_mp;
						hp_potions = 2;
						mp_potions = 2;
						current_floor = 1;
						init_map(difficulty, current_floor, max_floor, &player);
						player.pixel_x = player.x * 48;
						player.pixel_y = player.y * 48;
						player.is_moving = 0;
						game_state = 99;
						menu_index = 0;
					}
				}
				else if (game_state == 99 && !player.is_moving)
				{
					int nx = player.x;
					int ny = player.y;
					int moved = 0;

					if (KEY == SDLK_UP) { ny--; player.dir = 3; moved = 1; }
					else if (KEY == SDLK_DOWN) { ny++; player.dir = 0; moved = 1; }
					else if (KEY == SDLK_LEFT) { nx--; player.dir = 1; moved = 1; }
					else if (KEY == SDLK_RIGHT) { nx++; player.dir = 2; moved = 1; }
					
					// 보물상자 상호작용 - Z키 또는 엔터키
					else if (KEY == SDLK_z || KEY == SDLK_RETURN) 
					{
						int chk_x = player.x; int chk_y = player.y;
						// 플레이어가 바라보는 방향의 타일 확인
						if (player.dir == 0) chk_y++;
						else if (player.dir == 1) chk_x--;
						else if (player.dir == 2) chk_x++;
						else if (player.dir == 3) chk_y--;

						if (chk_x >= 0 && chk_x < MAP_WIDTH && chk_y >= 0 && chk_y < MAP_HEIGHT)
						{
							if (g_dungeon_map[chk_y][chk_x] == TILE_CHEST)
							{
								g_dungeon_map[chk_y][chk_x] = TILE_PATH; // 상자를 열면 빈 길로 변경
								int r = GET_RAND(0, 1);
								char msg[64];
								if (r == 0) {
									hp_potions++;
									sprintf(msg, "FOUND HP POTION!");
								} else {
									mp_potions++;
									sprintf(msg, "FOUND MP POTION!");
								}
								draw_dw_window(renderer, 205, 250, 405, 100);
								render_text(renderer, msg, 220, 290, 255, 255, 0);
								SDL_RenderPresent(renderer);
								SDL_Delay(1000);
							}
						}
					}

					if (moved && nx >= 0 && nx < MAP_WIDTH && ny >= 0 && ny < MAP_HEIGHT)
					{
						Enemy* target_mon = NULL;
						for (int i = 0; i < g_monster_count; i++)
						{
							if (g_monsters[i].is_alive && g_monsters[i].x == nx && g_monsters[i].y == ny)
							{
								target_mon = &g_monsters[i];
								break;
							}
						}

						if (target_mon != NULL)
						{
							if (target_mon->ignore_turns <= 0)
							{
								current_monster = target_mon;
								// 플레이어 좌표가 아닌 실제 '몬스터의 타일 좌표(nx, ny)'를 저장!
								saved_map_x = nx;  
								saved_map_y = ny;
								is_boss = (g_dungeon_map[ny][nx] == TILE_BOSS);
								
								// 전투 진입 전 화면 전환 효과
								play_encounter_transition(renderer);
								
								// 조우 메시지 설정
								sprintf(g_msg1, "%s APPEARED!", current_monster->name);
								g_msg2[0] = '\0';

								game_state = 3; 
								menu_index = 0;
							}
						}
						else
						{
							int tile = g_dungeon_map[ny][nx];
							if (tile == TILE_PATH || tile == TILE_STAIRS || tile == TILE_CHEST) 
							{
								if (tile != TILE_CHEST) // 보물상자는 통과 불가 지형 처리
								{
									player.x = nx; player.y = ny; player.is_moving = 1;
								}
							}
						}
					}
				}	
				else if (game_state == 3)
				{
					if (KEY == SDLK_UP) { if(menu_index > 0) menu_index--; }
					else if (KEY == SDLK_DOWN) { if(menu_index < battle_menu_count - 1) menu_index++; }
					else if (KEY == SDLK_RETURN)
					{
						switch (menu_index)
						{
							case 0: // 일반 공격 로직 전환 (효과 적용)
								{
									int final_dmg = GET_RAND(player.atk - 5, player.atk + 5);
									current_monster->hp -= final_dmg;
									
									char m1[64], m2[64];
									sprintf(m1, "ATTACKED %s!", current_monster->name);
									sprintf(m2, "DEALT %d DAMAGE!", final_dmg);
									
									play_hit_effect(renderer, &player, current_monster, difficulty, is_boss, menu_index, 0);
									render_typing_message(renderer, m1, m2, &player, current_monster, difficulty, is_boss, menu_index);

									if (current_monster->hp <= 0)
									{
										// 1. 몬스터가 죽었을 때 (맵 복귀)
										current_monster->hp = 0;
										sprintf(m1, "DEFEATED %s!", current_monster->name);
										render_typing_message(renderer, m1, "YOU WIN!", &player, current_monster, difficulty, is_boss, menu_index);
										
										play_monster_death_effect(renderer, &player, current_monster, difficulty, is_boss, menu_index);
										
										g_dungeon_map[saved_map_y][saved_map_x] = TILE_PATH;
										current_monster->is_alive = 0;
										current_monster = NULL;
										player.is_defending = 0;
										if (is_boss) game_state = 7;
										else game_state = 99;
									}
									else
									{	
										// 몬스터 반격
										int m_dmg = GET_RAND(current_monster->atk - 2, current_monster->atk + 2);
										if (player.is_defending)
										{
											// 방어 중이므로 무조건 데미지 반감
											m_dmg /= 2;
											sprintf(m1, "DEFENDED!");
											sprintf(m2, "TOOK %d DAMAGE.", m_dmg);
											player.is_defending = 0; // 방어 해제
										}
										else
										{
											sprintf(m1, "%s ATTACKS!", current_monster->name);
											sprintf(m2, "TOOK %d DAMAGE.", m_dmg);
										}
										
										play_hit_effect(renderer, &player, current_monster, difficulty, is_boss, menu_index, 1);
										player.hp -= m_dmg;
										render_typing_message(renderer, m1, m2, &player, current_monster, difficulty, is_boss, menu_index);

										if (player.hp <= 0)
										{
											// 맞고 나서 방어 해제 및 게임 오버 판정
											player.hp = 0;
											render_typing_message(renderer, "YOU DIED...", NULL, &player, current_monster, difficulty, is_boss, menu_index);
											game_state = 7;
										}
									}
									break;
								}
							case 1: game_state = 4; menu_index = 0; break;
							case 2: game_state = 5; menu_index = 0; break;
							case 3: 
								{
									// 방어 즉시 몬스터가 때림 (턴 소모)
									player.is_defending = 1;
									render_typing_message(renderer, "DEFENSE STANCE!", "READY FOR ATTACK.", &player, current_monster, difficulty, is_boss, menu_index);

									int m_dmg = GET_RAND(current_monster->atk - 2, current_monster->atk + 2);
									m_dmg /= 2; 
									
									char m1[64], m2[64];
									sprintf(m1, "%s ATTACKS!", current_monster->name);
									sprintf(m2, "BLOCKED! TOOK %d DAMAGE.", m_dmg);
									
									play_hit_effect(renderer, &player, current_monster, difficulty, is_boss, menu_index, 1);
									player.hp -= m_dmg;
									player.is_defending = 0; // 맞고 나서 방어 해제
									
									render_typing_message(renderer, m1, m2, &player, current_monster, difficulty, is_boss, menu_index);

									if (player.hp <= 0)
									{
										player.hp = 0;
										render_typing_message(renderer, "YOU DIED...", NULL, &player, current_monster, difficulty, is_boss, menu_index);
										game_state = 7;
									}
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
						int skill_dmg = 0;
						int mp_cost = 0;

						// 전사 스킬 판정
						if (current_skills == warrior_skills) 
						{
							mp_cost = 15;
							skill_dmg = player.atk * 2; // 평타 2배
						}
						else // 마법사 스킬 판정
						{
							mp_cost = 25;
							skill_dmg = player.atk * 3; // 평타 3배
						}

						if (player.mp >= mp_cost)
						{
							player.mp -= mp_cost;
							current_monster->hp -= skill_dmg;
							
							char m1[64], m2[64];
							sprintf(m1, "USED %s!", current_skills[menu_index]);
							sprintf(m2, "DEALT %d DAMAGE!", skill_dmg);
							play_hit_effect(renderer, &player, current_monster, difficulty, is_boss, 1, 0);
							render_typing_message(renderer, m1, m2, &player, current_monster, difficulty, is_boss, 1);

							if (current_monster->hp <= 0)
							{
								// 1. 몬스터가 죽었을 때 (맵 복귀)
								current_monster->hp = 0;
								sprintf(m1, "DEFEATED %s!", current_monster->name);
								render_typing_message(renderer, m1, "YOU WIN!", &player, current_monster, difficulty, is_boss, 1);
								play_monster_death_effect(renderer, &player, current_monster, difficulty, is_boss, 1);
								
								g_dungeon_map[saved_map_y][saved_map_x] = TILE_PATH;
								current_monster->is_alive = 0;
								current_monster = NULL;
								player.is_defending = 0;
								if (is_boss) game_state = 7;
								else game_state = 99;
							}
							else
							{
								// 몬스터가 살아서 반격
								int m_dmg = GET_RAND(current_monster->atk - 2, current_monster->atk + 2);
								sprintf(m1, "%s ATTACKS!", current_monster->name);
								sprintf(m2, "TOOK %d DAMAGE.", m_dmg);
								play_hit_effect(renderer, &player, current_monster, difficulty, is_boss, 1, 1);
								player.hp -= m_dmg;
								render_typing_message(renderer, m1, m2, &player, current_monster, difficulty, is_boss, 1);

								if (player.hp <= 0)
								{
									// 2. 플레이어가 죽었을 때 (게임 오버)
									player.hp = 0;
									render_typing_message(renderer, "YOU DIED...", NULL, &player, current_monster, difficulty, is_boss, 1);
									game_state = 7;
								}
								else
								{
									// 3. 둘 다 살았을 때만 전투 메인으로 복귀
									game_state = 3;
								}
							}
							menu_index = 0;
						}						
						else
						{
							render_typing_message(renderer, "NOT ENOUGH MP!", NULL, &player, current_monster, difficulty, is_boss, 1);
						}
					}					
					else if (KEY == SDLK_ESCAPE)
					{
						game_state = 3;
						menu_index = 1; // ESC 누르면 스킬 커서로 돌아가기
					}
				}
				else if (game_state == 5) // 아이템 창 로직
				{
					if (KEY == SDLK_UP) { if(menu_index > 0) menu_index--; }
					else if (KEY == SDLK_DOWN) { if(menu_index < item_menu_count - 1) menu_index++; }
					else if (KEY == SDLK_RETURN)
					{
						int item_used = 0; // 아이템 사용 여부 플래그
						char m1[64], m2[64];
						
						// HP 포션
						if (menu_index == 0) 
						{
							if (hp_potions > 0)
							{
								hp_potions--;
								player.hp += 50;
								if (player.hp > player.max_hp) player.hp = player.max_hp;
								sprintf(m1, "USED HP POTION!");
								sprintf(m2, "RECOVERED HP! LEFT: %d", hp_potions);
								item_used = 1;
							}
							else { render_typing_message(renderer, "NO HP POTIONS!", NULL, &player, current_monster, difficulty, is_boss, 2); }
						}
						// MP 포션
						else if (menu_index == 1) 
						{
							if(mp_potions > 0)
							{
								mp_potions--;
								player.mp += 30;
								if (player.mp > player.max_mp) player.mp = player.max_mp;
								sprintf(m1, "USED MP POTION!");
								sprintf(m2, "RECOVERED MP! LEFT: %d", mp_potions);
								item_used = 1;
							}
							else { render_typing_message(renderer, "NO MP POTIONS!", NULL, &player, current_monster, difficulty, is_boss, 2); }
						}

						// 아이템을 썼을 때만 몬스터가 때림 (턴 소모)
						if (item_used)
						{
							render_typing_message(renderer, m1, m2, &player, current_monster, difficulty, is_boss, 2);

							int m_dmg = GET_RAND(current_monster->atk - 2, current_monster->atk + 2);
							sprintf(m1, "%s ATTACKS!", current_monster->name);
							sprintf(m2, "TOOK %d DAMAGE.", m_dmg);
							play_hit_effect(renderer, &player, current_monster, difficulty, is_boss, 2, 1);
							player.hp -= m_dmg;
							render_typing_message(renderer, m1, m2, &player, current_monster, difficulty, is_boss, 2);

							if (player.hp <= 0)
							{
								player.hp = 0;
								render_typing_message(renderer, "YOU DIED...", NULL, &player, current_monster, difficulty, is_boss, 2);
								game_state = 7;
							}
							else { game_state = 3; } // 전투 메인으로 복귀
						}
						menu_index = 0;
					}
					else if (KEY == SDLK_ESCAPE)
					{
						game_state = 3;
						menu_index = 2; // ESC 누르면 아이템 커서로 돌아가기
					}
				}
				else if (game_state == 6)
				{
					if (KEY == SDLK_UP) { if(menu_index > 0) menu_index--; }
					else if (KEY == SDLK_DOWN) { if(menu_index < 1) menu_index++; }
					else if (KEY == SDLK_RETURN)
					{
						if(menu_index == 0) // "예" 선택 시 도망
						{
							render_typing_message(renderer, "RAN AWAY SAFELY!", NULL, &player, current_monster, difficulty, is_boss, 4);
							// 보스가 아닐 때만 무적 시간 부여 (보스는 즉시 재조우 가능하게)
							if (current_monster != NULL)
							{
								if (!is_boss) current_monster->ignore_turns = 5; 
								else current_monster->ignore_turns = 0; // 보스는 무적 없음
							}
							current_monster = NULL; // 조우 중인 몬스터 정보 초기화
							game_state = 99;        // 던전으로 복귀
						} 
						else 
						{ 
							game_state = 3;         // "아니오" 선택 시 다시 전투 메뉴로
						}
						menu_index = 0;
					}		
				}
				else if (game_state == 7)
				{
					if (KEY == SDLK_RETURN)
					{
						game_state = 0;
						menu_index = 0;
					}
				}
			}
		}

		/* 화면 초기화 */
		SET_COLOR(0,0,0);
		SDL_RenderClear(renderer);          

		/* 메인 메뉴 (game_state == 0) */
		if (game_state == 0)
		{
			draw_dw_window(renderer, 150, 80, 500, 120);
			render_text(renderer, "RPG PROJECT", 285, 125, 255, 255, 255);

			draw_dw_window(renderer, 300, 300, 200, 120);

			if (menu_index == 0) render_text(renderer, "START", 360, 330, 255, 255, 0);
			else render_text(renderer, "START", 360, 330, 255, 255, 255);

			if (menu_index == 1) render_text(renderer, "EXIT", 360, 375, 255, 255, 0);
			else render_text(renderer, "EXIT", 360, 375, 255, 255, 255);

			int cursor_y = (menu_index == 0) ? 330 : 375;
			render_text(renderer, ">", 335, cursor_y, 255, 255, 0);
		}
		
		/* 캐릭터 선택 (game_state == 1) */
		else if (game_state == 1)
		{
			draw_dw_window(renderer, 30, 40, 740, 80);
			render_text(renderer, "CHOOSE THY PROFESSION", 170, 65, 255, 255, 255);

			draw_dw_window(renderer, 30, 140, 360, 280);
			if (menu_index == 0) 
			{
				render_text(renderer, ">", 50, 160, 255, 255, 0);
				render_text(renderer, "WARRIOR", 80, 160, 255, 255, 0);
			}
			else render_text(renderer, "WARRIOR", 80, 160, 255, 255, 255);

			render_text(renderer, "HP : 150", 80, 230, 200, 200, 200);
			render_text(renderer, "MP : 30", 80, 280, 200, 200, 200);
			render_text(renderer, "ATK: 20", 80, 330, 200, 200, 200);

			draw_dw_window(renderer, 410, 140, 360, 280);
			if (menu_index == 1) 
			{
				render_text(renderer, ">", 430, 160, 255, 255, 0);
				render_text(renderer, "MAGE", 460, 160, 255, 255, 0);
			}
			else render_text(renderer, "MAGE", 460, 160, 255, 255, 255);

			render_text(renderer, "HP : 80", 460, 230, 200, 200, 200);
			render_text(renderer, "MP : 120", 460, 280, 200, 200, 200);
			render_text(renderer, "ATK: 10", 460, 330, 200, 200, 200);

			draw_dw_window(renderer, 30, 440, 740, 130);
			if (menu_index == 0)
			{
				render_text(renderer, "STURDY OF BODY,SWIFT OF BLADE.", 50, 470, 255, 255, 255);
				render_text(renderer, "A TRUE CHAMPION FOR THEE.", 50, 515, 255, 255, 255);
			}
			else
			{
				render_text(renderer, "MASTER OF ANCIENT SPELLS.", 50, 470, 255, 255, 255);
				render_text(renderer, "FRAGILE, YET WISE IN MIND.", 50, 515, 255, 255, 255);
			}
		}
		else if (game_state == 2)
		{
			draw_dw_window(renderer, 100, 40, 600, 80);
			render_text(renderer, "WHITHER ART THOU BOUND?", 130, 65, 255, 255, 255);

			draw_dw_window(renderer, 180, 160, 440, 360);
			for (int i = 0; i < diff_count; i++)
			{
				if (menu_index == i) 
				{
					render_text(renderer, ">", 210, 210 + (i * 100), 255, 255, 0);
					render_text(renderer, diff_names[i], 250, 210 + (i * 100), 255, 255, 0);
				}
				else
				{
					render_text(renderer, diff_names[i], 250, 210 + (i * 100), 255, 255, 255);
				}
			}
		}
		else if (game_state == 99)
		{
			/* 타일 원본 크기(ts) 및 화면 출력 크기(ss) 설정 */
			int ts = 16;
			int ss = 48;
			int ox = (800 - (17 * ss)) / 2;
			int oy = (600 - (13 * ss)) / 2;

			for (int cy = 0; cy < 13; cy++)
			{
				for (int cx = 0; cx < 17; cx++)
				{
					int mx = player.x - 8 + cx;
					int my = player.y - 6 + cy;

					if (mx >= 0 && mx < MAP_WIDTH && my >= 0 && my < MAP_HEIGHT)
					{
						int t = g_dungeon_map[my][mx];
						SDL_Rect d_r = { ox + cx * ss, oy + cy * ss, ss, ss };

						/* 1. 맵 타일 렌더링 (map_tiles.png 사용) */
						if (t == TILE_WALL)
						{
							int sx = (difficulty == 0) ? 64 : ((difficulty == 1) ? 112 : 0);
							int sy = (difficulty == 0) ? 176 : ((difficulty == 1) ? 16 : 0);
							SDL_Rect s_r = { sx, sy, ts, ts };
							SDL_RenderCopy(renderer, map_tex, &s_r, &d_r);
						}
						else
						{
							int psx = (difficulty == 0) ? 16 : ((difficulty == 1) ? 32 : 16);
							int psy = (difficulty == 0) ? 128 : ((difficulty == 1) ? 144 : 0);
							SDL_Rect s_r = { psx, psy, ts, ts };
							SDL_RenderCopy(renderer, map_tex, &s_r, &d_r);

							if (t == TILE_STAIRS)
							{
								SDL_Rect stair_s = { 0, 112, ts, ts };
								SDL_RenderCopy(renderer, map_tex, &stair_s, &d_r);
							}
							else if (t == TILE_CHEST)
							{
								// 보물상자 타일 렌더링 적용 (계단 타일 인접 좌표 사용)
								SDL_Rect chest_s = { 64, 64, ts, ts };
								SDL_RenderCopy(renderer, map_tex, &chest_s, &d_r);
							}
							else if (t == TILE_MONSTER || t == TILE_BOSS)
							{
								/* [수정 포인트] 몬스터 및 보스 좌표 */
								int msx = 0; int msy = 0;
								if (t == TILE_BOSS) { msx = 96; msy = 64; }       
								else if (difficulty == 0) { msx = 0; msy = 64; }  
								else if (difficulty == 1) { msx = 48; msy = 64; } // 박쥐 이미지 
								else { msx = 144; msy = 0; }                      // 해골 이미지

								SDL_Rect m_s = { msx + (player.anim_frame * 16), msy, ts, ts };
								SDL_RenderCopy(renderer, char_tex, &m_s, &d_r);
							}
						}
					}
				}
			}

			/* 3. 플레이어 타일 렌더링 (전사/마법사 통합 0,0 고정) */
			int player_start_x = 0;
			int player_start_y = 0;

			SDL_Rect cp_s = { player_start_x + (player.anim_frame * 16), player_start_y + (player.dir * 16), ts, ts };
			SDL_Rect cp_d = { ox + 8 * ss, oy + 6 * ss, ss, ss };

			/* 반드시 char_tex를 사용해서 그려야 주인공이 보임 */
			SDL_RenderCopy(renderer, char_tex, &cp_s, &cp_d);
		}
		
		/* [5. 전투 화면 렌더링] */
		else if (game_state == 3)
		{
			render_battle_scene(renderer, &player, current_monster, difficulty, is_boss, menu_index, 0);
			
			draw_dw_window(renderer, 100, 420, 600, 140);
			if (g_msg1[0] != '\0') render_text(renderer, g_msg1, 130, 460, 255, 255, 255);
			if (g_msg2[0] != '\0') render_text(renderer, g_msg2, 130, 500, 255, 255, 255);
		}
		
		/* [6. 기타 메뉴 및 게임오버] */
		else if (game_state >= 4 && game_state <= 6)
		{
			SET_COLOR(30, 10, 10);
			SDL_Rect bg = {0, 0, 800, 600};
			FILL_RECT(bg);

			if (game_state == 4) SET_COLOR(20, 40, 80);
			else if (game_state == 5) SET_COLOR(20, 80, 40);
			else SET_COLOR(60, 40, 20);

			SDL_Rect popup = {230, 200, 350, 200};
			FILL_RECT(popup);

			if (game_state == 4)
			{
				render_text(renderer, "--- SKILL ---", 255, 210, 255, 255, 255);
				for (int i = 0; i < 2; i++)
				{
					SDL_Rect opt = {260, 260 + (i * 60), 300, 40};
					if (menu_index == i) SET_COLOR(255, 255, 0);
					else SET_COLOR(150, 150, 150);
					FILL_RECT(opt);
					render_text(renderer, current_skills[i], 265, 270 + (i * 60), 0, 0, 0);
				}
			}
			else if (game_state == 5)
			{
				render_text(renderer, "--- ITEM ---", 255, 210, 255, 255, 255);
				for (int i = 0; i < 2; i++)
				{
					SDL_Rect opt = {260, 260 + (i * 60), 280, 40};
					if (menu_index == i) SET_COLOR(255, 255, 0);
					else SET_COLOR(150, 150, 150);
					FILL_RECT(opt);
					render_text(renderer, item_menu[i], 265, 270 + (i * 60), 0, 0, 0);
				}
			}
			else if (game_state == 6)
			{
				render_text(renderer, "RUN AWAY?", 280, 230, 255, 255, 255);
				char* r_list[] = {"1. YES", "2. NO"};
				for (int i = 0; i < 2; i++)
				{
					SDL_Rect opt = {300, 280 + (i * 50), 200, 40};
					if (menu_index == i) SET_COLOR(255, 255, 0);
					else SET_COLOR(150, 150, 150);
					FILL_RECT(opt);
					render_text(renderer, r_list[i], 350, 290 + (i * 50), 0, 0, 0);
				}
			}
		}
		else if (game_state == 7)
		{
			SDL_Rect full_bg = {0, 0, 800, 600};
			if (player.hp <= 0) SET_COLOR(100, 0, 0);
			else SET_COLOR(0, 50, 100);
			FILL_RECT(full_bg);

			draw_dw_window(renderer, 155, 200, 505, 200);

			/* 중앙 대칭 맞춤형 좌표 설정 완료 */
			if (player.hp <= 0)
			{
				render_text(renderer, "YOU DIED...", 330, 240, 255, 50, 50);
			}
			else
			{
				render_text(renderer, "YOU WIN!", 300, 230, 50, 255, 50);
				render_text(renderer, diff_names[difficulty], 255, 280, 255, 255, 255);
				render_text(renderer, "STAGE CLEARED!", 245, 320, 255, 255, 0);
			}
			render_text(renderer, "PRESS ENTER TO RETURN", 170, 465, 255, 255, 255);
		}

		draw_scanlines(renderer, 800, 600);
		SDL_RenderPresent(renderer);
	}

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	TTF_Quit();
	IMG_Quit();
	SDL_Quit();

	return 0;
}
