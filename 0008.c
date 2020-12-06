#include <stdlib.h>
#include <SDL2/SDL.h>

#define WINDOW_WIDTH 600

#define MAX_X 500
#define MAX_Y 500

#define RADIUS 1
#define FOV 1

struct POINTF {
	float X;
	float Y;
	float Z;
};

struct TRIF {
	struct POINTF *T;
	int RED;
	int GREEN;
	int BLUE;
};

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

float pvol(float ax, float bx, float cx, float dx, float ay, float by, float cy, float dy, float az, float bz, float cz, float dz){
	float x1 = bx - ax;
	float y1 = by - ay;
	float z1 = bz - az;
	float x2 = cx - ax;
	float y2 = cy - ay;
	float z2 = cz - az;
	float a = y1*z2 - y2*z1;
	float b = x2 * z1 - x1*z2;
	float c = x1 * y2 - x2*y1;
	float d = -(a*ax + b*ay + c*az);
	float vol = a*dx + b*dy + c * dz + d;
	if (vol < 0.0f){
		vol = -vol;
	}
	return vol;
}

int block(struct POINTF *s, struct POINTF *t, struct POINT *t2){
	int i;
	char c;
	for (i=0; i<3; ++i){
		if (inside((int)(s[i].X/s[i].Z), (int)(s[i].Y/s[i].Z), t2)){
			float a = pvol(s[i].X, 0, t[0].X, t[1].X, s[i].Y, 0, t[0].Y, t[1].Y, s[i].Z, 0, t[0].Z, t[1].Z);
			float b = pvol(s[i].X, 0, t[1].X, t[2].X, s[i].Y, 0, t[1].Y, t[2].Y, s[i].Z, 0, t[1].Z, t[2].Z);
			float c = pvol(s[i].X, 0, t[2].X, t[0].X, s[i].Y, 0, t[2].Y, t[0].Y, s[i].Z, 0, t[2].Z, t[0].Z);
			float d = pvol(t[1].X, 0, t[2].X, t[0].X, t[1].Y, 0, t[2].Y, t[0].Y, t[1].Z, 0, t[2].Z, t[0].Z);
			if ((a + b + c) < d){
				return 1; // s is blocker
			}
		}
	}
	return 0;
}

void generate(struct TRIF *tfList, struct TRI *tList, int maxtri){
	int i,j;
	for (i=0; i<maxtri; ++i){
		for (j=0; j<3; ++j){
			tList[i].T[j].X = (int)(tfList[i].T[j].X / tfList[i].T[j].Z);                  
			tList[i].T[j].Y = (int)(tfList[i].T[j].Y / tfList[i].T[j].Z);
		}
		tList[i].RED = tfList[i].RED;
		tList[i].GREEN = tfList[i].GREEN;
		tList[i].BLUE = tfList[i].BLUE;
	}
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


int main(int argc, char *argv[]) {
	 SDL_Event event;
    SDL_Renderer *renderer;
    SDL_Window *window;

    SDL_Init(SDL_INIT_VIDEO);
    SDL_CreateWindowAndRenderer(WINDOW_WIDTH, WINDOW_WIDTH, 0, &window, &renderer);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderClear(renderer);				

	struct POINTF triangle_1[] =
	{
	            [0] = { .X = 0.0f, .Y = 0.0f, .Z = 1.0f},
	            [1] = { .X = 100.0f, .Y = 100.0f, .Z= 1.0f},
	            [2] = { .X = 200.0f, .Y = 0.0f, .Z= 1.0f}
	};
	struct POINTF triangle_2[] =
	{
	            [1] = { .X = 100.0f, .Y = 0.0f, .Z = 1.3f},
	            [0] = { .X = 200.0f, .Y = 100.0f, .Z = 1.3f},
	            [2] = { .X = 300.0f, .Y = 0.0f, .Z = 1.3f}
	};
	
	struct TRIF tfList[2];
	tfList[0].T = triangle_2;
	tfList[0].RED = 0;
	tfList[0].GREEN = 255;
	tfList[0].BLUE = 0;
	tfList[1].T = triangle_1;
	tfList[1].RED = 255;
	tfList[1].GREEN = 0;
	tfList[1].BLUE = 0;
	
	struct TRI tList[2];
	tList[0].T = malloc(sizeof(struct POINT) * 3);
	tList[1].T = malloc(sizeof(struct POINT) * 3);
	generate(tfList, tList, 2);
	//if (block(tfList[0].T, tfList[1].T, tList[1].T))
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