#include <SDL2/SDL.h>		//SDL2 헤더 파일
#include <stdlib.h>
#include <time.h>
#include <stdio.h>


#define KEY e.key.keysym.sym	//75줄
#define SET_COLOR(r,g,b) SDL_SetRenderDrawColor(renderer, r, g, b, 255)
#define FILL_RECT(rect) SDL_RenderFillRect(renderer, &rect)
#define GET_RAND(min, max) (rand() % (max - min + 1 ) + min) 	//랜덤 난수 범위 구현

//난이도 설정 전역 변수
char *diff_names[] = {"Easy", "Medium", "Hard"};
//전투 시 사용 가능한 메뉴 나열
char *battle_menu[] = {"Attack", "Skill", "Item", "Defense", "Run"};
//
char *warrior_skills[] = {"Power Strike", "Whirlwind"};		//휠이 아닌 훨윈드이다.
char *mage_skills[] = 	{"Magic Arrow", "Meteor"};		
char *item_menu[] = {"HP Potion", "MP Potion"};

char **current_skills; 					 	//더블 포인터로 직업 스킬 연결

//캐릭터 구조체 생성 ( 수치 수정 가능 ) 
typedef struct
{
	int hp;
	int max_hp;
	int mp;
	int max_mp;
	int atk;
} Character;

//몬스터 구조체 생성
typedef struct
{
	int hp;
	int max_hp;
	int atk;
	char *name;
}Enemy;

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
	

	//시스템 초기화 후 부팅 확인 여부 판단
	if (SDL_Init(SDL_INIT_VIDEO) < 0){				//시스템 에러 확인
		printf("초기화 실패 에러내용: %s\n", SDL_GetError());	//에러 이유 출력
		return 1;
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
	//SDL_RENDERER_ACCELERATED
	SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	//투명도 사용을 위한 코드 설정
	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

	//사용자 정의 변수
	int menu_index = 0;	//**아마 0,1 로 게임 시작과 종료를 해둘 예정
	int game_state = 0;

	//시스템 이벤트 루프 명령어
	int quit = 0;
	SDL_Event e;
	
	//사용자 정의 데이터(캐릭터)
	Character warrior = {150, 150, 30, 30, 20};	//체력,최대체력,마나,최대마나,공격력
	Character mage = {80, 80, 120, 120, 10};
	Character player;				//실제 플레이어 데이터 설정
	
	Enemy monster = {100, 100, 15, "Slime"};	//임시 몬스터 개체값

	//게임 종료 이벤트 결정 코드
	while(!quit)
	{
		while (SDL_PollEvent(&e) != 0)
		{
			if (e.type == SDL_QUIT)
			{
				quit = 1;
			}
			
			

			//main_index = 0 게임시작 -> game_state = 1로 이동
			//main_index = 1 게임종료 -> quit = 1 로 이동 후 종료

			else if (e.type == SDL_KEYDOWN)
			{
				if (KEY == SDLK_ESCAPE)
				{
					game_state = 0;
					menu_index = 0;
				}
				if (game_state == 0)
                		{
                    			if (KEY == SDLK_UP)
                    			{
                        			menu_index = 0;
                   			}
                    			else if (KEY == SDLK_DOWN)
                    			{		
                        			menu_index = 1;
                    			}
                    			else if (KEY == SDLK_RETURN)
                    			{
                        			if (menu_index == 0)
                        			{
                            				game_state = 1;
                            				menu_index = 0; 
                        			}
                        			else
                        			{
                            				quit = 1;
                        			}
                    			}
                		}
				else if (game_state == 1)
				{
					if (KEY == SDLK_LEFT)
					{
						menu_index = 0;
					}
					else if (KEY == SDLK_RIGHT)
					{
						menu_index = 1;
					}
					else if (KEY == SDLK_RETURN)
					{
						if (menu_index == 0)
						{
							player = warrior;
							current_skills = warrior_skills;
							current_skill_count = sizeof(warrior_skills) / sizeof(warrior_skills[0]);
						}
						else
						{
							player = mage;
							current_skills = mage_skills;
							current_skill_count = sizeof(mage_skills) / sizeof(mage_skills[0]);
						}

						game_state = 2;
						menu_index = 0;
					}	
				}
				else if (game_state == 2)
				{
					if (KEY == SDLK_UP)
					{
						if(menu_index > 0) menu_index--;
					}
					else if (KEY == SDLK_DOWN)
					{
						if(menu_index < diff_count -1) menu_index++;
					}
					else if (KEY == SDLK_RETURN)
					{
						difficulty = menu_index;
						
						//나가거나 다시 입장 할 때 체력 초기화 및 포션 보충 로직
						player.hp = player.max_hp;
						player.mp = player.max_mp;
						hp_potions = 2;
						mp_potions = 2;
						monster.hp = monster.max_hp;

						game_state = 3;
						menu_index = 0;

					}
				}
				else if (game_state == 3)
				{
					if (KEY == SDLK_UP)
					{
						if(menu_index > 0) menu_index--;
					}
					else if (KEY == SDLK_DOWN)
					{
						if(menu_index < battle_menu_count - 1) menu_index++;
					}
					else if (KEY == SDLK_RETURN)
					{
						switch (menu_index)
						{
							case 0:
							{
								int final_dmg = GET_RAND(player.atk -5, player.atk + 5);
								monster.hp -= final_dmg;
								printf("\n일반 공격! %s에게  %d의  데미지!\n",monster.name, final_dmg);
								
								if (monster.hp <= 0)
								{
									monster.hp = 0;
									printf("%s를 처치 했습니다! 전투 승리!\n", monster.name);
									game_state = 7;
								}
								else
								{	
									// 반격 로직인데 큐 사용시 지우거나 변경 예정
									int m_dmg = GET_RAND(monster.atk - 2, monster.atk + 2);
									player.hp -= m_dmg;
									printf("%s의 반격 %d의 피해!\n", monster.name, m_dmg);

									if (player.hp <= 0)
									{
										player.hp = 0;
										printf("전투에서 패배했습니다...\n");
										game_state = 7;
									}
								}
								break;
							}


							case 1:
								game_state = 4; // 스킬 창 열기!
								menu_index = 0;
								break;
							case 2:
								game_state = 5; // 아이템 창 열기!
								menu_index = 0;
								break;
							case 3:
								printf("방어 태세! 몬스터의 공격을 대비한다\n");
								break;
							case 4:
								game_state = 6; // 도망 확인창으로 이동
								menu_index = 0;
								break;
						}
					}
				}
				//스킬 창 로직
				else if (game_state == 4) 
				{
					if (KEY == SDLK_UP)
					{
						if(menu_index > 0) menu_index--;
					}
					else if (KEY == SDLK_DOWN)
					{
						if(menu_index < current_skill_count - 1) menu_index++;
					}
					else if (KEY == SDLK_RETURN)
					{
						// 직업에 맞는 스킬 출력
						printf("%s 발동!\n", current_skills[menu_index]);

						// 단일/광역 데미지 로직 들어갈 공간

						game_state = 3; // 스킬 쓰고 메인 전투 메뉴로 다시 돌아가기
						menu_index = 0;
					}
					else if (KEY == SDLK_ESCAPE)
					{
						game_state = 3;
						menu_index = 1; // ESC 누르면 스킬 커서로 돌아가기
					}
				}
				else if (game_state == 5) // 아이템 창 로직
				{
					if (KEY == SDLK_UP)
					{
						if(menu_index > 0) menu_index--;
					}
					else if (KEY == SDLK_DOWN)
					{
						if(menu_index < item_menu_count - 1) menu_index++;
					}
					else if (KEY == SDLK_RETURN)
					{
						if (menu_index == 0)
						{
							if (hp_potions > 0)
							{
								printf("HP 포션 사용! (남은 개수: %d)\n", --hp_potions);
							}
							else
							{
								printf("보유하신 HP 포션이 없습니다.\n");
							}
						}
						else if (menu_index == 1)
						{
							if(mp_potions >0)
							{
								printf("MP 포션 사용! (남은 개수: %d)\n", --mp_potions);
							}
							else
							{
								printf("보유하신 MP 포션이 없습니다.\n");
							}
						}
						game_state = 3;
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
					if (KEY == SDLK_UP)
					{
						if(menu_index > 0) menu_index--;
					}
					else if (KEY == SDLK_DOWN)
					{
						if(menu_index < 1)menu_index++;
					}
					else if (KEY == SDLK_RETURN)
					{
						if(menu_index == 0)game_state = 0; 	//메인화면
						else game_state =2;			//난이도 선택
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
		SET_COLOR(0,0,0);    
		SDL_RenderClear(renderer);			
		
		if (game_state == 0)
		{
			//시작 메뉴 바 위치 설정
			SDL_Rect start_bar = {250, 320, 300, 50};
			SDL_Rect quit_bar = {250, 390, 300, 50};
				
			//시작 버튼 그리기
			if (menu_index == 0) SET_COLOR(255, 255, 0);
			else SET_COLOR(100, 100, 100);
			FILL_RECT(start_bar);

			if (menu_index == 1) SET_COLOR(255, 255, 0);
			else SET_COLOR(100, 100, 100);
			FILL_RECT(quit_bar);
		}
		else if(game_state == 1)
		{
			SET_COLOR(0, 0, 100);
			SDL_Rect full_screen = {0, 0, 800, 600};
			FILL_RECT(full_screen);

			SDL_Rect left_screen  = {0, 0, 400, 600};
			SDL_Rect right_screen = {400, 0, 400, 600};
		
			// 왼쪽 선택 중 (전사)	
			if (menu_index == 0)
			{
				SET_COLOR(0, 0, 100);
				FILL_RECT(left_screen);


				SDL_SetRenderDrawColor(renderer, 255, 255, 0, 150);
				SDL_Rect warrior_img = {100, 150, 200, 250};
				FILL_RECT(warrior_img);
			}

			// 오른쪽 선택 중 (마법사)
			else
			{
				SET_COLOR(0, 0, 100);
				FILL_RECT(right_screen);
			
				SDL_SetRenderDrawColor(renderer, 0, 255, 255, 150);
				SDL_Rect mage_img = {500, 150, 200, 250};
				FILL_RECT(mage_img);
			}


		}
		else if(game_state == 2)
		{
			SET_COLOR(20, 20, 40);
			SDL_Rect full_screen = {0, 0, 800, 600};
			FILL_RECT(full_screen);

			for(int i = 0; i < diff_count; i++)
			{
				SDL_Rect bar = {250, 150 + (i * 120), 300, 70};
				if (menu_index == i) SET_COLOR(255, 255, 0);
				else SET_COLOR(150, 150, 150);

				FILL_RECT(bar);
			}
		}
		else if(game_state == 3)
		{
			SET_COLOR(30, 10, 10);
			SDL_Rect bg = {0, 0, 800, 600}; FILL_RECT(bg);

			SET_COLOR(255, 255, 255);
			SDL_Rect ui_border = {10, 440, 700, 150}; FILL_RECT(ui_border);

			SET_COLOR(0, 0, 0);
			SDL_Rect ui_inner = {15, 445, 770, 140}; FILL_RECT(ui_inner);

			SET_COLOR(200, 0, 0);
			SDL_Rect enemy_img = {300, 100, 200, 200}; FILL_RECT(enemy_img);

			//전투 메뉴 나열 로직
			for(int i = 0; i < battle_menu_count; i++)
			{
				SDL_Rect opt = {550, 460 + (i * 25), 200, 20};
				if (menu_index == i) SET_COLOR(255, 255, 0);
				else SET_COLOR(150, 150, 150);
				FILL_RECT(opt);
			}
		}
		else if (game_state >= 4 && game_state <= 6)
		{
			SET_COLOR(30, 10, 10);
			SDL_Rect bg = {0, 0, 800, 600}; FILL_RECT(bg);

			//팝업 창? 같은 박스 **수정필요
			if (game_state == 4) SET_COLOR(20, 40, 80);
			else if (game_state == 5) SET_COLOR(20, 80, 40);
			else SET_COLOR(60, 60, 60);

			SDL_Rect popup = {250, 200, 300, 200}; FILL_RECT(popup);
			
			int count = 2;
			if(game_state == 4) count = current_skill_count;
			for(int i = 0; i <count; i++)
			{
				SDL_Rect opt = {300, 230 + (i * 50), 200, 30};
				if (menu_index == i) SET_COLOR(255, 255, 0);
				else SET_COLOR(150, 150, 150);
				FILL_RECT(opt);
			}
		}
		else if (game_state == 7)
		{
			if (monster.hp <= 0) SET_COLOR(0, 0, 100);
			else SET_COLOR(100, 0, 0);

			SDL_Rect full_screen = {0, 0, 800, 600};
			FILL_RECT(full_screen);

			SET_COLOR(255, 255, 255);
			SDL_Rect result_box = {200, 200, 400, 200};
			FILL_RECT(result_box);
		}





		SDL_RenderPresent(renderer);			//모니터 화면 출력
	}

	//메모리 해제 (메모리 누수 방지)
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}


