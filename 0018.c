#include <stdlib.h>
#include <string.h>
#include <SDL2/SDL.h>

#define WINDOW_WIDTH 500

#define K 50.0f

#define MAX_X 500
#define MAX_Y 500

#define RADIUS 250
#define FOV 1

#define N 6

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
/*
float pwr(int power, float number){
	int i;
	float result=1;
	for (i=0; i<power; ++i){
		result *= number;
	}
	return result;
}

float fact(int integer){
	int answer = 1;
	for (;integer>0; --integer){
		answer *= integer;
	}
	return (float) integer;
}

#define MATH_D 1000

float sin(float angle){
	int i;
	float result = angle;
	for (i=0; i<MATH_D; ++i){
		result -= pwr(4*i+3, angle)/fact(4*i+3);
		result += pwr(4*i+5, angle)/face(4*i+5);
	}
	return result;
}

float cos(float angle){
	int i;
	float result = angle;
	for (i=0; i<MATH_D; ++i){
		result -= pwr(4*i+2, angle)/fact(4*i+2);
		result += pwr(4*i+4, angle)/face(4*i+4);
	}
	return result;
}
*/
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
	for (i=0; i<3; ++i){
		if (inside((int)((s[i].X*RADIUS)/s[i].Z), (int)((s[i].Y*RADIUS)/s[i].Z), t2)){
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

int block2(struct POINTF *t, struct POINTF *s, struct POINT *t2){
	int i;
	//t is blocker, s is blocked, t2 is the 2D blocker
	for (i=0; i<3; ++i){ // For all points in blocked
		if (inside((int)((s[i].X*RADIUS)/s[i].Z), (int)((s[i].Y*RADIUS)/s[i].Z), t2)){ // If blocked is inside blocker
			//Compare the blocked point s
			float a = pvol(s[i].X, 0, t[0].X, t[1].X, s[i].Y, 0, t[0].Y, t[1].Y, s[i].Z, 0, t[0].Z, t[1].Z);
			float b = pvol(s[i].X, 0, t[1].X, t[2].X, s[i].Y, 0, t[1].Y, t[2].Y, s[i].Z, 0, t[1].Z, t[2].Z);
			float c = pvol(s[i].X, 0, t[2].X, t[0].X, s[i].Y, 0, t[2].Y, t[0].Y, s[i].Z, 0, t[2].Z, t[0].Z);
			float d = pvol(t[1].X, 0, t[2].X, t[0].X, t[1].Y, 0, t[2].Y, t[0].Y, t[1].Z, 0, t[2].Z, t[0].Z);
			if ((a + b + c) > d){
				return 1; // t is blocker
			}
		}
	}
	return 0;
}


void generate(struct TRIF *tfList, struct TRI *tList, int maxtri){
	int i,j;
	for (i=0; i<maxtri; ++i){
		for (j=0; j<3; ++j){
			tList[i].T[j].X = (int)((tfList[i].T[j].X*RADIUS) / tfList[i].T[j].Z);                  
			tList[i].T[j].Y = (int)((tfList[i].T[j].Y*RADIUS) / tfList[i].T[j].Z);
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
		   for (k=0; k<N; ++k){
		   	if (inside(i-RADIUS, RADIUS-j, tList[k].T)){
		   		SDL_SetRenderDrawColor(renderer, tList[k].RED, tList[k].GREEN, tList[k].BLUE, 255);
		   		SDL_RenderDrawPoint(renderer, i, j);
		   		break;
		   	}
		   }
	    }
	}
}

void sorttriangle(struct TRIF *tfList, struct TRI *tList){
	int i;
	int j;
	for (i=0; i<N-1; ++i){
		for (j=1; j<(N-i); ++j){
			if (block(tfList[j].T, tfList[j-1].T, tList[j-1].T) || block2(tfList[j].T, tfList[j-1].T, tList[j].T)){
				struct TRIF tmp;
				memcpy(&tmp, tfList + (j-1), sizeof(struct TRIF));
				memcpy(tfList + (j-1), tfList + j, sizeof(struct TRIF));
				memcpy(tfList + j, &tmp, sizeof(struct TRIF));
				struct TRI tmp2;
				memcpy(&tmp2, tList + (j-1), sizeof(struct TRI));
				memcpy(tList + (j-1), tList + j, sizeof(struct TRI));
				memcpy(tList + j, &tmp2, sizeof(struct TRI));		
			}
		}
	}
}

void mchange(float *matrix, struct POINTF *p){
	int i;
	float result[3] = {0.0f};
	for (i=0; i<3; ++i){
		result[i] = matrix[i * 3] * p->X + matrix[i * 3 + 1] * p->Y + matrix[i * 3 + 2] * p->Z;
	}
	p->X = result[0];
	p->Y = result[1];
	p->Z = result[2];
}

void rotate(struct POINTF *p, float x, float y, float z){
	float xm[9] = {0.0f};
	float ym[9] = {0.0f};
	float zm[9] = {0.0f};
	
	xm[0] = 1.0f;
	xm[4] = cos(x);
	xm[5] = -sin(x);
	xm[7] = sin(x);
	xm[8] = cos(x);
	
	ym[0] = cos(y);
	ym[2] = sin(y);
	ym[4] = 1.0f;
	ym[6] = -sin(y);
	ym[8] = cos(y);
	
	zm[0] = cos(z);
	zm[1] = -sin(z);
	zm[3] = sin(z);
	zm[4] = cos(z);
	zm[8] = 1.0f;
	
	int i;
	for (i=0; i<3; ++i){
		mchange(xm, p + i);
		mchange(ym, p + i);
		mchange(zm, p + i);
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
	
	int i;
	
	struct POINTF triangle_1[] =
	{
	            [0] = { .X = 0.0f, .Y = 0.0f, .Z= RADIUS},
	            [1] = { .X = K, .Y = 0.0f, .Z= RADIUS},
	            [2] = { .X = 0.0f, .Y = -K, .Z= RADIUS}
	};
	
	struct POINTF triangle_2[] =
	{
	            [0] = { .X = 0.0f, .Y = -K, .Z = RADIUS},
	            [1] = { .X = K, .Y = 0.0f, .Z = RADIUS},
	            [2] = { .X = K, .Y = -K, .Z = RADIUS}
	};
	
	
	struct POINTF triangle_5[] =
	{
	            [0] = { .X = 0.0f, .Y = 0.0f, .Z= RADIUS},
	            [1] = { .X = K, .Y = 0.0f, .Z= RADIUS},
	            [2] = { .X = K, .Y = 0.0f, .Z= K+RADIUS}
	};
	struct POINTF triangle_6[] =
	{
	            [0] = { .X = 0.0f, .Y = 0.0f, .Z= RADIUS},
	            [1] = { .X = K, .Y = 0.0f, .Z= RADIUS+K},
	            [2] = { .X = 0.0f, .Y = 0.0f, .Z= RADIUS+K}
	};
	struct POINTF triangle_3[] =
	{
	            [0] = { .X = 0.0f, .Y = 0.0f, .Z = RADIUS},
	            [1] = { .X = 0.0f, .Y = 0.0f, .Z = RADIUS+K},
	            [2] = { .X = 0.0f, .Y = -K, .Z = RADIUS+K}
	};
	struct POINTF triangle_4[] =
	{
	            [0] = { .X = 0.0f, .Y = 0.0f, .Z= RADIUS},
	            [1] = { .X = 0.0f, .Y = -K, .Z= RADIUS+K},
	            [2] = { .X = 0.0f, .Y = -K, .Z= RADIUS}
	};
	/*
	struct POINTF triangle_7[3];
	struct POINTF triangle_8[3];
	struct POINTF triangle_9[3];
	struct POINTF triangle_10[3];
	struct POINTF triangle_11[3];
	struct POINTF triangle_12[3];
	
	memcpy(triangle_7, triangle_1, sizeof(struct POINTF)*3);
	memcpy(triangle_8, triangle_2, sizeof(struct POINTF)*3);
	memcpy(triangle_9, triangle_3, sizeof(struct POINTF)*3);
	memcpy(triangle_10, triangle_4, sizeof(struct POINTF)*3);
	memcpy(triangle_11, triangle_5, sizeof(struct POINTF)*3);
	memcpy(triangle_12, triangle_6, sizeof(struct POINTF)*3);
	
	for (i=0; i<3; ++i){
		triangle_7[i].Z = 2 * K;
		triangle_8[i].Z = 2 * K;
		triangle_9[i].Y = -K;
		triangle_10[i].Y = -K;
		triangle_11[i].X = K;
		triangle_12[i].X = K;
	}
	*/
	struct TRIF tfList[N];
	
	memset(tfList, 0, sizeof(struct TRIF) * N);
	
	tfList[0].T = triangle_1;
	tfList[1].T = triangle_2;
	tfList[2].T = triangle_3;
	tfList[3].T = triangle_4;
	tfList[4].T = triangle_5;
	tfList[5].T = triangle_6;
	/*
	tfList[6].T = triangle_7;
	tfList[7].T = triangle_8;
	tfList[8].T = triangle_9;
	tfList[9].T = triangle_10;
	tfList[10].T = triangle_11;
	tfList[11].T = triangle_12;
	
	*/
	tfList[0].RED = 255;
	tfList[1].RED = 255;
	
	tfList[2].GREEN = 255;
	tfList[3].GREEN= 255;
	
	tfList[4].GREEN = 255;
	tfList[4].RED = 255;
	tfList[5].GREEN = 255;
	tfList[5].RED = 255;
	/*
	tfList[6].GREEN = 128;
	tfList[6].BLUE = 128;
	tfList[7].GREEN = 128;
	tfList[7].BLUE = 128;
	
	tfList[8].BLUE = 255;
	tfList[9].BLUE = 255;
	
	tfList[10].RED = 255;
	tfList[10].BLUE = 255;
	tfList[11].RED = 255;
	tfList[11].BLUE = 255;
	
//#define PI_VALUE 3.141592653589793238
	*/
	struct TRI tList[N];
	for (i=0; i<N; ++i){
		tList[i].T = malloc(sizeof(struct POINT) * 3);
	}
	for(i=0; i<N; ++i){
		int j;
		for(j=0; j<3; ++j){
			tfList[i].T[j].X += 100.0f;
			tfList[i].T[j].Y -= 100.0f;
		}
		//rotate(tfList[i].T, -((float)M_PI)/12.0f, 0, 0);
		//rotate(tfList[i].T, ((float)M_PI)/10.0f, 0, 0);	
	}
	
	generate(tfList, tList, N);
	
	//sorttriangle(tfList, tList);    
	
	draw_tList(renderer, tList);
	
/*	
		struct POINTF triangle_1[] =
	{
	            [0] = { .X = 0.0f, .Y = 0.0f, .Z= 250.0f},
	            [1] = { .X = -50.0f, .Y = 0.0f, .Z= 250.0f},
	            [2] = { .X = 0.0f, .Y = 50.0f, .Z= 250.0f}
	};
	struct POINTF triangle_2[] =
	{
	            [0] = { .X = 0.0f, .Y = 0.0f, .Z = 250.0f},
	            [1] = { .X = 50.0f, .Y = 0.0f, .Z = 250.0f},
	            [2] = { .X = 0.0f, .Y = 50.0f, .Z = 250.0f}
	};
	
	struct TRIF tfList[2];
	tfList[0].T = triangle_1;
	tfList[0].RED = 0;
	tfList[0].GREEN = 255;
	tfList[0].BLUE = 0;
	tfList[1].T = triangle_2;
	tfList[1].RED = 255;
	tfList[1].GREEN = 0;
	tfList[1].BLUE = 0;
	
//#define PI_VALUE 3.141592653589793238
	
	struct TRI tList[2];
	tList[0].T = malloc(sizeof(struct POINT) * 3);
	tList[1].T = malloc(sizeof(struct POINT) * 3);
	rotate(tfList[0].T, 0, 0, M_PI/2);
	rotate(tfList[1].T, 0, 0, M_PI/2);
	generate(tfList, tList, 2);
	sorttriangle(tfList, tList);    
	draw_tList(renderer, tList);
	*/
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