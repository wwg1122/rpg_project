#include <SDL2/SDL.h>		//SDL2 헤더 파일
#include <stdio.h>

#define KEY e.key.keysym.sym	//75줄
#define SET_COLOR(r,g,b) SDL_SetRenderDrawColor(renderer, r, g, b, 255)
#define FILL_RECT(rect) SDL_RenderFillRect(renderer, &rect)


//캐릭터 구조체 생성 ( 수치 수정 가능 ) 
typedef struct
{
	int hp;
	int max_hp;
	int mp;
	int max_mp;
	int atk;
} Character;



int main(int argc, char *argv[])
{	
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
						if (menu_index == 0) player = warrior;
						else player = mage;

						game_state = 2;
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
			
			if (menu_index == 0)
			{
				SDL_SetRenderDrawColor(renderer, 255, 255, 0, 80);
				SDL_RenderFillRect(renderer, &left_screen);

				SET_COLOR(255, 0, 0);
				SDL_Rect hp_bar = {50, 450, 200, 30};
				FILL_RECT(hp_bar);
			}
			else
			{
				SDL_SetRenderDrawColor(renderer, 255, 255, 0, 80);
				SDL_RenderFillRect(renderer, &right_screen);

				SET_COLOR(0, 0, 255);
				SDL_Rect mp_bar = {450, 450, 200, 30};
				FILL_RECT(mp_bar);
			}


		}


		SDL_RenderPresent(renderer);			//모니터 화면 출력
	}

	//메모리 해제 (메모리 누수 방지)
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}


