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
SDL_Texture* g_p_img = NULL;
SDL_Texture* g_e_img = NULL;
SDL_Texture* main_bg = NULL;
SDL_Texture* tileset_tex = NULL;
SDL_Texture* char_tex = NULL;
SDL_Texture* map_tex = NULL;

#define KEY e.key.keysym.sym	//75줄
#define SET_COLOR(r,g,b) SDL_SetRenderDrawColor(renderer, r, g, b, 255)
#define FILL_RECT(rect) SDL_RenderFillRect(renderer, &rect)
#define GET_RAND(min, max) (rand() % (max - min + 1 ) + min) 	//랜덤 난수 범위 구현

//난이도 설정 전역 변수
char *diff_names[] = {"Slime Forest", "Goblin Cave", "Demon Castle"};
//전투 시 사용 가능한 메뉴 나열
char *battle_menu[] = {"Attack", "Skill", "Item", "Defense", "Run"};
//
char *warrior_skills[] = {"Power Strike", "Whirlwind"};		//휠이 아닌 훨윈드이다.
char *mage_skills[] = 	{"Magic Arrow", "Meteor"};		
char *item_menu[] = {"HP Potion", "MP Potion"};

char **current_skills; 					 	//더블 포인터로 직업 스킬 연결

int saved_map_x = 0;						//전투 후 복귀를 위한 좌표
int saved_map_y = 0;


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
	if (SDL_Init(SDL_INIT_VIDEO) < 0){				//시스템 에러 확인
		printf("초기화 실패 에러내용: %s\n", SDL_GetError());	//에러 이유 출력
		return 1;
	}

	if (TTF_Init() == -1) {
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


	SDL_Surface* temp_surf = IMG_Load("tileset.png");
	if (temp_surf != NULL)
	{
		SDL_SetColorKey(temp_surf, SDL_TRUE, SDL_MapRGB(temp_surf->format, 255, 255, 255));
		tileset_tex = SDL_CreateTextureFromSurface(renderer, temp_surf);
		SDL_SetTextureBlendMode(tileset_tex, SDL_BLENDMODE_BLEND);
		SDL_FreeSurface(temp_surf);
	}

	else
	{
		printf("타일셋 로드 실패... 파일 경로\n");
	}	
	//투명도 사용을 위한 코드 설정
	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

	//사용자 정의 변수
	int menu_index = 0;	//**아마 0,1 로 게임 시작과 종료를 해둘 예정
	int game_state = 0;
	int is_boss = 0;

	//시스템 이벤트 루프 명령어
	int quit = 0;
	SDL_Event e;

	//사용자 정의 데이터(캐릭터)
	Character warrior = {150, 150, 30, 30, 20, 0, 0, 0};	//체력,최대체력,마나,최대마나,공격력
								//
	Character mage = {80, 80, 120, 120, 10, 0, 0, 0};
	Character player;				//실제 플레이어 데이터 설정

	Enemy *current_monster = NULL; 			//동적할당으로 변경(2026-02-13)

	while(!quit)
	{
		if (game_state == 99)
		{
			/* 1. 애니메이션 타이머 업데이트 */
			player.anim_timer++;
			if (player.anim_timer > 12)
			{
				player.anim_frame = !player.anim_frame;
				player.anim_timer = 0;
			}

			/* 2. 스무스 이동 물리 업데이트 */
			if (player.is_moving)
			{
				int target_px = player.x * 48;
				int target_py = player.y * 48;
				int move_speed = 4;

				if (player.pixel_x < target_px) player.pixel_x += move_speed;
				else if (player.pixel_x > target_px) player.pixel_x -= move_speed;

				if (player.pixel_y < target_py) player.pixel_y += move_speed;
				else if (player.pixel_y > target_py) player.pixel_y -= move_speed;

				/* 목표 지점 도착 판정 */
				if (player.pixel_x == target_px && player.pixel_y == target_py)
				{
					player.is_moving = 0;
					update_all_monsters_ai(player.x, player.y);

					/* 계단 타일 도착 시 다음 층 이동 */
					if (g_dungeon_map[player.y][player.x] == TILE_STAIRS)
					{
						current_floor++;
						init_map(difficulty, current_floor, max_floor, &player);
						player.pixel_x = player.x * 48;
						player.pixel_y = player.y * 48;
					}
				}
			}
			/* 3. 키 입력 및 몬스터/계단 충돌 판정 */
			else if (e.type == SDL_KEYDOWN)
			{
				int nx = player.x;
				int ny = player.y;
				int moved = 0;

				switch (e.key.keysym.sym)
				{
					case SDLK_UP:    ny--; player.dir = 3; moved = 1; break;
					case SDLK_DOWN:  ny++; player.dir = 0; moved = 1; break;
					case SDLK_LEFT:  nx--; player.dir = 1; moved = 1; break;
					case SDLK_RIGHT: nx++; player.dir = 2; moved = 1; break;
				}

				if (moved && nx >= 0 && nx < MAP_WIDTH && ny >= 0 && ny < MAP_HEIGHT)
				{
					int tile = g_dungeon_map[ny][nx];
					/* 길이나 계단이면 이동 시작 */
					if (tile == TILE_PATH || tile == TILE_STAIRS)
					{
						player.x = nx;
						player.y = ny;
						player.is_moving = 1;
					}
					/* 몬스터 조우 시 5초 무적(ignore_turns) 체크 */
					else if (tile == TILE_MONSTER || tile == TILE_BOSS)
					{
						Enemy* target_mon = NULL;
						for (int i = 0; i < g_monster_count; i++)
						{
							if (g_monsters[i].x == nx && g_monsters[i].y == ny && g_monsters[i].is_alive)
							{
								target_mon = &g_monsters[i];
								break;
							}
						}

						if (target_mon && target_mon->ignore_turns <= 0)
						{
							current_monster = target_mon;
							saved_map_x = nx;
							saved_map_y = ny;
							is_boss = (tile == TILE_BOSS);
							play_encounter_transition(renderer);
							game_state = 3;
							menu_index = 0;
						}
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


				else if (game_state == 99)
				{
					/* [1] 애니메이션 및 스무스 이동 로직 업데이트 */
					player.anim_timer++;
					if (player.anim_timer > 15)
					{
						player.anim_frame = !player.anim_frame;
						player.anim_timer = 0;
					}

					if (player.is_moving)
					{
						int target_px = player.x * 48;
						int target_py = player.y * 48;
						int move_speed = 4;

						if (player.pixel_x < target_px)
						{
							player.pixel_x += move_speed;
						}
						else if (player.pixel_x > target_px)
						{
							player.pixel_x -= move_speed;
						}

						if (player.pixel_y < target_py)
						{
							player.pixel_y += move_speed;
						}
						else if (player.pixel_y > target_py)
						{
							player.pixel_y -= move_speed;
						}

						if (player.pixel_x == target_px && player.pixel_y == target_py)
						{
							player.is_moving = 0;
						}
					}

					if (!player.is_moving && e.type == SDL_KEYDOWN)
					{
						int nx = player.x;
						int ny = player.y;
						int moved = 0;

						switch (e.key.keysym.sym)
						{
							case SDLK_UP:
								ny--; player.dir = 3; moved = 1;
								break;
							case SDLK_DOWN:
								ny++; player.dir = 0; moved = 1;
								break;
							case SDLK_LEFT:
								nx--; player.dir = 1; moved = 1;
								break;
							case SDLK_RIGHT:
								nx++; player.dir = 2; moved = 1;
								break;
						}

						if (moved && nx >= 0 && nx < MAP_WIDTH && ny >= 0 && ny < MAP_HEIGHT)
						{
							int target_tile = g_dungeon_map[ny][nx];
							if (target_tile == TILE_PATH || target_tile == TILE_STAIRS)
							{
								player.x = nx;
								player.y = ny;
								player.is_moving = 1;
							}
							else if (target_tile == TILE_MONSTER || target_tile == TILE_BOSS)
							{
								play_encounter_transition(renderer);
								game_state = 3;
							}
						}
					}

					/* [3] 던전 테마 및 맵 렌더링 */
					int img_tile_size = 512;
					int screen_tile_size = 48;
					int offset_x = (800 - (17 * screen_tile_size)) / 2;
					int offset_y = (600 - (13 * screen_tile_size)) / 2;
					int theme_y = difficulty * img_tile_size;

					for (int cy = 0; cy < 13; cy++)
					{
						for (int cx = 0; cx < 17; cx++)
						{
							int map_x = player.x - 8 + cx;
							int map_y = player.y - 6 + cy;
							int draw_x = offset_x + (cx * screen_tile_size);
							int draw_y = offset_y + (cy * screen_tile_size);

							if (map_x >= 0 && map_x < MAP_WIDTH && map_y >= 0 && map_y < MAP_HEIGHT)
							{
								int tile_type = g_dungeon_map[map_y][map_x];
								int src_x = 0;
								if (tile_type == TILE_WALL)
								{
									src_x = img_tile_size;
								}
								else if (tile_type == TILE_STAIRS)
								{
									src_x = img_tile_size * 2;
								}

								SDL_Rect src = { src_x, theme_y, img_tile_size, img_tile_size };
								SDL_Rect dst = { draw_x, draw_y, screen_tile_size, screen_tile_size };
								SDL_RenderCopy(renderer, map_tex, &src, &dst);
							}
						}
					}

					int char_src_x = player.dir * (img_tile_size * 2) + (player.anim_frame * img_tile_size);
					SDL_Rect c_src = { char_src_x % 2048, 1536, img_tile_size, img_tile_size };
					SDL_Rect c_dst = { offset_x + (8 * screen_tile_size), offset_y + (6 * screen_tile_size),
						screen_tile_size, screen_tile_size };
					SDL_RenderCopy(renderer, char_tex, &c_src, &c_dst);
				}

				else if (game_state == 3)
				{
					if (KEY == SDLK_UP) { if(menu_index > 0) menu_index--; }
					else if (KEY == SDLK_DOWN) { if(menu_index < battle_menu_count - 1) menu_index++; }
					else if (KEY == SDLK_RETURN)
					{
						switch (menu_index)
						{
							case 0:
								{
									int final_dmg = GET_RAND(player.atk - 5, player.atk + 5);
									current_monster->hp -= final_dmg;
									printf("\n[공격] %s에게 %d의 데미지!\n", current_monster->name, final_dmg);

									if (current_monster->hp <= 0)
									{
										current_monster->hp = 0;
										printf("%s를 처치 했습니다! 전투 승리!\n", current_monster->name);
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
											m_dmg /= 2;
											printf("[방어 성공] %s의 공격을 막아냈다! %d의 피해!\n", current_monster->name, m_dmg);
											player.is_defending = 0; // 방어 해제
										}
										else
										{
											printf("%s의 반격! %d의 피해!\n", current_monster->name, m_dmg);
										}

										player.hp -= m_dmg;
										if (player.hp <= 0)
										{
											player.hp = 0;
											printf("전투에서 패배했습니다...\n");
											game_state = 7;
										}
									}
									break;
								}
							case 1: game_state = 4; menu_index = 0; break;
							case 2: game_state = 5; menu_index = 0; break;
							case 3: 
								{
									player.is_defending = 1;
									printf("\n[방어] 방어 태세! 몬스터의 공격을 대비합니다.\n");

									// 방어 즉시 몬스터가 때림 (턴 소모)
									int m_dmg = GET_RAND(current_monster->atk - 2, current_monster->atk + 2);
									m_dmg /= 2; // 방어 중이므로 무조건 데미지 반감
									printf("[방어 성공] %s의 공격을 막아냈다! %d의 피해!\n", current_monster->name, m_dmg);
									player.hp -= m_dmg;
									player.is_defending = 0; // 맞고 나서 방어 해제

									if (player.hp <= 0)
									{
										player.hp = 0;
										printf("전투에서 패배했습니다...\n");
										game_state = 7;
									}
									break;
								}
							case 4: game_state = 6; menu_index = 0; break;
						}

					}
				}
				else if (game_state == 4) //스킬 창 로직
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
							printf("\n[스킬] %s 발동! %s에게 %d의 피해! (소모 MP: %d)\n", current_skills[menu_index], current_monster->name, skill_dmg, mp_cost);

							if (current_monster->hp <= 0)
							{
								// 1. 몬스터가 죽었을 때 (맵으로 복귀)
								current_monster->hp = 0;
								printf("%s를 처치 했습니다! 전투 승리!\n", current_monster->name);
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
								printf("%s의 반격! %d의 피해!\n", current_monster->name, m_dmg);
								player.hp -= m_dmg;

								if (player.hp <= 0)
								{
									// 2. 플레이어가 죽었을 때 (게임 오버)
									player.hp = 0;
									printf("전투에서 패배했습니다...\n");
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
							printf("\n[시스템] MP가 부족합니다! (현재 MP: %d)\n", player.mp);
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

						if (menu_index == 0) // HP 포션
						{
							if (hp_potions > 0)
							{
								hp_potions--;
								player.hp += 50;
								if (player.hp > player.max_hp) player.hp = player.max_hp;
								printf("\n[아이템] HP 포션 사용! (현재 HP: %d/%d) [남은 개수: %d]\n", player.hp, player.max_hp, hp_potions);
								item_used = 1;
							}
							else { printf("\n[시스템] 보유하신 HP 포션이 없습니다.\n"); }
						}
						else if (menu_index == 1) // MP 포션
						{
							if(mp_potions > 0)
							{
								mp_potions--;
								player.mp += 30;
								if (player.mp > player.max_mp) player.mp = player.max_mp;
								printf("\n[아이템] MP 포션 사용! (현재 MP: %d/%d) [남은 개수: %d]\n", player.mp, player.max_mp, mp_potions);
								item_used = 1;
							}
							else { printf("\n[시스템] 보유하신 MP 포션이 없습니다.\n"); }
						}

						// 아이템을 썼을 때만 몬스터가 때림 (턴 소모)
						if (item_used)
						{
							int m_dmg = GET_RAND(current_monster->atk - 2, current_monster->atk + 2);
							printf("%s의 공격! %d의 피해!\n", current_monster->name, m_dmg);
							player.hp -= m_dmg;

							if (player.hp <= 0)
							{
								player.hp = 0;
								printf("전투에서 패배했습니다...\n");
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
							printf("무사히 도망쳤다!\n");

							// 보스가 아닐 때만 무적 시간 부여 (보스는 즉시 재조우 가능하게)
							if (current_monster != NULL)
							{
								if (!is_boss) 
								{
									current_monster->ignore_turns = 5; 
								}
								else 
								{
									current_monster->ignore_turns = 0; // 보스는 무적 없음
								}
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

		/* 난이도 선택 (game_state == 2) */
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



		/* [던전 탐험 렌더링] */
		else if (game_state == 99)
		{
			int t_size = 512;
			int c_size = 256;
			int s_size = 48;
			int ox = (800 - (17 * s_size)) / 2;
			int oy = (600 - (13 * s_size)) / 2;
			int ty = difficulty * t_size;

			for (int cy = 0; cy < 13; cy++)
			{
				for (int cx = 0; cx < 17; cx++)
				{
					int mx = player.x - 8 + cx;
					int my = player.y - 6 + cy;
					if (mx >= 0 && mx < MAP_WIDTH && my >= 0 && my < MAP_HEIGHT)
					{
						int t = g_dungeon_map[my][mx];
						int sx = (t == TILE_WALL) ? t_size : (t == TILE_STAIRS ? t_size * 2 : 0);
						SDL_Rect s_r = { sx, ty, t_size, t_size };
						SDL_Rect d_r = { ox + cx * s_size, oy + cy * s_size, s_size, s_size };
						SDL_RenderCopy(renderer, tileset_tex, &s_r, &d_r);

						/* 몬스터 한 마리씩만 정밀 렌더링 */
						if (t == TILE_MONSTER || t == TILE_BOSS)
						{
							SDL_Rect m_s = { (t_size * 3) + (player.anim_frame * c_size), ty, c_size, c_size };
							SDL_RenderCopy(renderer, tileset_tex, &m_s, &d_r);
						}
					}
				}
			}
			/* 캐릭터 방향 정밀 조준 (방향 깨짐 해결) */
			SDL_Rect cp_s = { (player.dir * 2 + player.anim_frame) * c_size, 1536, c_size, c_size };
			SDL_Rect cp_d = { ox + 8 * s_size, oy + 6 * s_size, s_size, s_size };
			SDL_RenderCopy(renderer, tileset_tex, &cp_s, &cp_d);
		}

		/* [전투 화면 렌더링 중 몬스터 이미지] */
		else if (game_state == 3)
		{
			/* (전투 배경 및 윈도우 로직 생략...) */
			if (current_monster != NULL)
			{
				/* 현재 테마의 몬스터 이미지를 가져와서 중앙에 크게 출력 */
				int theme_y = difficulty * 512;
				SDL_Rect m_src = { 1536, theme_y, 512, 512 };
				SDL_Rect m_dst = { 275, 120, 250, 250 };
				SDL_RenderCopy(renderer, tileset_tex, &m_src, &m_dst);
			}
		}

		else if (game_state == 3)
		{
			/* 1. 전투 배경 (완전한 검은색) */
			SET_COLOR(0, 0, 0);
			SDL_Rect battle_bg = {0, 0, 800, 600};
			FILL_RECT(battle_bg);

			/* 2. 중앙에 몬스터 크게 렌더링 (임시로 타일셋 이미지 확대 출력) */
			if (current_monster != NULL)
			{
				draw_tile(renderer, tileset_tex, 0, 152, 300, 150, 200); 
			}

			/* 3. 커맨드 윈도우 (좌측 상단) */
			draw_dw_window(renderer, 20, 20, 200, 260);
			render_text(renderer, "COMMAND", 50, 40, 255, 255, 255);

			/* 전투 메뉴 출력 및 커서 하이라이트 */
			char* b_menu[] = {"ATTACK", "SKILL", "ITEM", "DEFEND", "RUN"};
			for (int i = 0; i < 5; i++)
			{
				if (menu_index == i) 
				{
					render_text(renderer, ">", 35, 90 + (i * 35), 255, 255, 0);
					render_text(renderer, b_menu[i], 60, 90 + (i * 35), 255, 255, 0);
				}
				else 
				{
					render_text(renderer, b_menu[i], 60, 90 + (i * 35), 255, 255, 255);
				}
			}

			/* 4. 스테이터스 윈도우 (우측 상단) */
			draw_dw_window(renderer, 550, 20, 220, 200);
			render_text(renderer, "STATUS", 600, 40, 255, 255, 255);

			char stat_buf[32];
			sprintf(stat_buf, "HP:  %d", player.hp);
			render_text(renderer, stat_buf, 580, 90, 255, 255, 255);
			sprintf(stat_buf, "MP:  %d", player.mp);
			render_text(renderer, stat_buf, 580, 130, 255, 255, 255);
			sprintf(stat_buf, "ATK: %d", player.atk);
			render_text(renderer, stat_buf, 580, 170, 255, 255, 255);

			/* 5. 메시지 윈도우 (하단) */
			draw_dw_window(renderer, 100, 420, 600, 140);
			if (current_monster != NULL)
			{
				char msg_buf[128];
				sprintf(msg_buf, "%s APPEARED!", current_monster->name);
				render_text(renderer, msg_buf, 130, 460, 255, 255, 255);
			}
		}


		else if (game_state >= 4 && game_state <= 6)
		{
			SET_COLOR(30, 10, 10);
			SDL_Rect bg = {0, 0, 800, 600};
			FILL_RECT(bg);

			if (game_state == 4) SET_COLOR(20, 40, 80);      // 스킬창 (파랑)
			else if (game_state == 5) SET_COLOR(20, 80, 40); // 아이템창 (초록)
			else SET_COLOR(60, 40, 20);                      // 도망창 (갈색)

			SDL_Rect popup = {250, 200, 300, 200};
			FILL_RECT(popup);

			if (game_state == 4) // 스킬 선택 (정확한 이름!)
			{
				render_text(renderer, "--- SKILL LIST ---", 285, 210, 255, 255, 255);
				char* s_list[] = {"1. POWER STRIKE", "2. WHIRLWIND"};
				for (int i = 0; i < 2; i++)
				{
					SDL_Rect opt = {300, 260 + (i * 60), 200, 40};
					if (menu_index == i) SET_COLOR(255, 255, 0);
					else SET_COLOR(150, 150, 150);
					FILL_RECT(opt);
					render_text(renderer, s_list[i], 310, 270 + (i * 60), 0, 0, 0);
				}
			}
			else if (game_state == 5) // 아이템 선택 (이름 명시!)
			{
				render_text(renderer, "--- ITEM LIST ---", 290, 210, 255, 255, 255);
				char* i_list[] = {"1. HP POTION", "2. MP POTION"};
				for (int i = 0; i < 2; i++)
				{
					SDL_Rect opt = {300, 260 + (i * 60), 200, 40};
					if (menu_index == i) SET_COLOR(255, 255, 0);
					else SET_COLOR(150, 150, 150);
					FILL_RECT(opt);
					render_text(renderer, i_list[i], 310, 270 + (i * 60), 0, 0, 0);
				}
			}
			else if (game_state == 6) // 도망 확인 (텍스트 추가!)
			{
				render_text(renderer, "REALLY RUN AWAY?", 280, 230, 255, 255, 255);
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
		}
		draw_scanlines(renderer, 800, 600);
		SDL_RenderPresent(renderer);			//모니터 화면 출력
	}
	//메모리 해제 (메모리 누수 방지)
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	TTF_Quit();
	IMG_Quit();
	SDL_Quit();

	return 0;
}
