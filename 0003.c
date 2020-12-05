#include <stdlib.h>

#include <SDL2/SDL.h>

#define WINDOW_WIDTH 600

#define MAX_X 500
#define MAX_Y 500

struct POINT {
	int X;
	int Y;
};

struct TRI {
	struct POINT *T;
	int RED;
	int GREEN;
	int BLUE;
};

int area(int x1, int x2, int x3, int y1, int y2, int y3){
	int t = x1*(y2 - y3) + x2*(y3 - y1) + x3*(y1 - y2);
	if (t < 0){
		t = -t;
	}
	return t;
}

int inside (int u, int v, struct POINT *t){
	int a = area(t[0].X, t[1].X, u, t[0].Y, t[1].Y, v);
	int b = area(t[1].X, t[2].X, u, t[1].Y, t[2].Y, v);
	int c = area(t[0].X, t[2].X, u, t[0].Y, t[2].Y, v);
	int d = a+b+c- area(t[0].X, t[1].X, t[2].X, t[0].Y, t[1].Y, t[2].Y);
	if (d < 0) d = -d;
	if (d==0){
		return 1;
	}
	return 0;
}

void draw_tList(SDL_Renderer *renderer, struct TRI *tList){
    int maxx = MAX_X;
    int maxy = MAX_Y;
    int i, j, k;
    for (i=0; i<maxx; ++i){
		for (j=0; j<maxy; ++j){
		   for (k=0; k<2; ++k){
		   	if (inside(i, j, tList[k].T)){
		   		SDL_SetRenderDrawColor(renderer, tList[k].RED, tList[k].GREEN, tList[k].BLUE, 255);
		   		SDL_RenderDrawPoint(renderer, i, -j+maxy);
		   		break;
		   	}
		   }
	    }
	}
}

int main(void) {
    SDL_Event event;
    SDL_Renderer *renderer;
    SDL_Window *window;

    SDL_Init(SDL_INIT_VIDEO);
    SDL_CreateWindowAndRenderer(WINDOW_WIDTH, WINDOW_WIDTH, 0, &window, &renderer);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderClear(renderer);

	struct POINT triangle_1[] =
	{
	            [0] = { .X = 200, .Y = 0 },
	            [1] = { .X = 0, .Y = 0 },
	            [2] = { .X = 0, .Y = 200 }
	};
	struct POINT triangle_2[] =
	{
	            [0] = { .X = 300, .Y = 100 },
	            [1] = { .X = 0, .Y = 100 },
	            [2] = { .X = 0, .Y = 300 }
	};
	
	struct TRI tList[2];
	tList[0].T = triangle_1;
	tList[0].RED = 255;
	tList[0].GREEN = 0;
	tList[0].BLUE = 0;
	tList[1].T = triangle_2;
	tList[1].RED = 0;
	tList[1].GREEN = 255;
	tList[1].BLUE = 0;
	
	draw_tList(renderer, tList);    
	
    SDL_RenderPresent(renderer);
    while (1) {
        if (SDL_PollEvent(&event) && event.type == SDL_QUIT)
            break;
    }
    
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return EXIT_SUCCESS;
}