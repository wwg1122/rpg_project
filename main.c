#include <SDL2/SDL.h>		//SDL2 헤더 파일
#include <stdio.h>


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
	//SDL_RENDERER_ACCELERATED << 수정 필요
	SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	
	//시스템 이벤트 루프 명령어
	int quit = 0;
	SDL_Event e;
	
	//게임 종료 이벤트 결정 코드
	while(!quit)
	{
		while (SDL_PollEvent(&e) != 0)
		{
			if (e.type == SDL_QUIT)
			{
				quit = 1;
			}
		}
		SDL_SetRenderDrawColor(renderer, 0,0,0,255);    //

		SDL_RenderClear(renderer);			//

		SDL_RenderPresent(renderer);			//화면 깜빡임 방지
	}

	//메모리 해제 (메모리 누수 방지)
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}


