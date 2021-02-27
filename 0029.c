#include <stdlib.h>
#include <string.h>
#include <SDL2/SDL.h>

#define WINDOW_WIDTH 500

#define K 50.0f

#define MAX_X 500
#define MAX_Y 500

#define RADIUS 250
#define FOV 1

#define CTRIANGLE 2

struct POINT_3D_FLOAT {
	float X;
	float Y;
	float Z;
};

struct POINT_2D_INT {
	int X;
	int Y;
};

struct POINT_2D_FLOAT {
	float X;
	float Y;
};

struct COLOR_MIX {
	int RED;
	int GREEN;
	int BLUE;
};

struct TRIANGLE_3D_FLOAT {
	struct COLOR_MIX COLOR;
	struct POINT_3D_FLOAT *TRIANGLE;
};

int area(int x1, int x2, int x3, int y1, int y2, int y3){
	int t = x1*(y2 - y3) + x2*(y3 - y1) + x3*(y1 - y2);
	if (t < 0){
		t = -t;
	}
	return t;
}

int inside (int u, int v, struct POINT_2D_INT *t){
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

int zbuffer(struct TRIANGLE_3D_FLOAT *list, int list_length, float *depth_buffer, struct COLOR_MIX *frame_buffer){
	int i;
	int j;
	float a, b, c, z;
	for (i=0; i<(RADIUS*RADIUS*4); ++i){
		depth_buffer[i] = 10000.0f;
		frame_buffer[i].RED = 255;
		frame_buffer[i].GREEN = 255;
		frame_buffer[i].BLUE = 255;
	}
	for (i=0; i<2; ++i){
		struct POINT_2D_INT p[3];
		for (j=0; j<3; ++j){
			p[j].X = RADIUS + (int)round((list[i].TRIANGLE[j].X * RADIUS * FOV) / list[i].TRIANGLE[j].Z);
			p[j].Y = RADIUS - (int)round((list[i].TRIANGLE[j].Y * RADIUS * FOV) / list[i].TRIANGLE[j].Z);/*
			char str[80];
			sprintf(str, "%d %d", p[j].X, p[j].Y);
			SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, str, str, NULL);*/
		}
		int x_max = (p[0].X > p[1].X) ? ( (p[2].X > p[0].X) ? p[2].X : p[0].X ) : ( (p[2].X > p[1].X) ? p[2].X : p[1].X );
		int y_max = (p[0].Y > p[1].Y) ? ( (p[2].Y > p[0].Y) ? p[2].Y : p[0].Y ) : ( (p[2].Y > p[1].Y) ? p[2].Y : p[1].Y );
		int x_min = (p[0].X < p[1].X) ? ( (p[2].X < p[0].X) ? p[2].X : p[0].X ) : ( (p[2].X < p[1].X) ? p[2].X : p[1].X );
		int y_min = (p[0].Y < p[1].Y) ? ( (p[2].Y < p[0].Y) ? p[2].Y : p[0].Y ) : ( (p[2].Y < p[1].Y) ? p[2].Y : p[1].Y );
		
		if (x_min < 0){
			x_min = 0;
		}
		if (x_max < 0){
			x_max = 0;
		}
		if (x_min >= (RADIUS * 2)){
			x_min = (RADIUS * 2) - 1;
		}
		if (x_max >= (RADIUS*2)){
			//SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "hi", "hi", NULL);
			x_max = (RADIUS * 2) - 1;
		}
		if (y_min < 0){
			y_min = 0;
		}
		if (y_max < 0){
			y_max = 0;
		}
		if (y_min >= (RADIUS * 2)){
			y_min = (RADIUS * 2) - 1;
		}
		if (y_max >= (RADIUS*2)){
			y_max = (RADIUS * 2) - 1;
		}
		
		int k, l;
		for (j=0; j<3; ++j){
			p[j].X -= RADIUS;
			p[j].Y = RADIUS - p[j].Y;
		}
		for (k=x_min; k<=x_max; ++k){
			for (l=y_min; l<=y_max; ++l){
				if (inside(k - RADIUS, RADIUS - l, p)){
					a = -(list[i].TRIANGLE[2].Y - list[i].TRIANGLE[0].Y) * (list[i].TRIANGLE[1].Z - list[i].TRIANGLE[0].Z) + (list[i].TRIANGLE[1].Y - list[i].TRIANGLE[0].Y) * (list[i].TRIANGLE[2].Z - list[i].TRIANGLE[0].Z);
					b = -(list[i].TRIANGLE[1].X - list[i].TRIANGLE[0].X) * (list[i].TRIANGLE[2].Z - list[i].TRIANGLE[0].Z) + (list[i].TRIANGLE[2].X - list[i].TRIANGLE[0].X) * (list[i].TRIANGLE[1].Z - list[i].TRIANGLE[0].Z);
					c = -(list[i].TRIANGLE[2].X - list[i].TRIANGLE[0].X) * (list[i].TRIANGLE[1].Y - list[i].TRIANGLE[0].Y) + (list[i].TRIANGLE[1].X - list[i].TRIANGLE[0].X) * (list[i].TRIANGLE[2].Y - list[i].TRIANGLE[0].Y);
					//char str[80];
					//sprintf(str, "%2.2f %2.2f %2.2f", a, b, c);
					//SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, str, str, NULL);
					z = (a*list[i].TRIANGLE[0].X + b*list[i].TRIANGLE[0].Y + c*list[i].TRIANGLE[0].Z)/(((a*(k - RADIUS) + b*(RADIUS - l))/(RADIUS * FOV)) + c);
				//	if (k==x_min+50 && i==1) return 0;
					if (depth_buffer[k*RADIUS*2+l] > z){
						//SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "hi", "hi", NULL);
						depth_buffer[k*RADIUS*2+l] = z;
				//		frame_buffer[k*RADIUS*2 + l].RED = list[i].COLOR.RED;
		//				frame_buffer[k*RADIUS*2 + l].GREEN = list[i].COLOR.GREEN;
//						frame_buffer[k*RADIUS*2 + l].BLUE = list[i].COLOR.BLUE;
//if (i==0){
						memcpy(frame_buffer + (k*RADIUS*2 + l), &(list[i].COLOR), sizeof(struct COLOR_MIX));
				//		char str[80];
		//	sprintf(str, "%d %d %d", frame_buffer[k*RADIUS*2+l].RED, frame_buffer[k*RADIUS*2+l].GREEN,frame_buffer[k*RADIUS*2+l].BLUE);
			//SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, str, str, NULL);
						
//}
					}
				}
			}
		}
	}
}

void draw_buffer(SDL_Renderer *renderer, struct COLOR_MIX *frame_buffer){
    int i, j;
    for (i=0; i<RADIUS*2; ++i){
    	for (j=0; j<RADIUS*2; ++j){
		   	SDL_SetRenderDrawColor(renderer, frame_buffer[i*RADIUS*2+j].RED, frame_buffer[i*RADIUS*2+j].GREEN, frame_buffer[i*RADIUS+j].BLUE, 255);
		   	//SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
		   	SDL_RenderDrawPoint(renderer, i, j);
    	}
    }
}

void matrix_change(float *matrix, struct POINT_3D_FLOAT *p){
	int i;
	float result[3] = {0.0f};
	for (i=0; i<3; ++i){
		result[i] = matrix[i * 3] * p->X + matrix[i * 3 + 1] * p->Y + matrix[i * 3 + 2] * p->Z;
	}
	p->X = result[0];
	p->Y = result[1];
	p->Z = result[2];
}

void rotate(struct POINT_3D_FLOAT *p, float x, float y, float z){
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
		matrix_change(xm, p + i);
		matrix_change(ym, p + i);
		matrix_change(zm, p + i);
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
	
	int i, j;
	/*
	struct POINT_3D_FLOAT triangle_1[] =
	{
	            [0] = { .X = 0.0f, .Y = 0.0f, .Z= RADIUS},
	            [1] = { .X = K, .Y = 0.0f, .Z= RADIUS},
	            [2] = { .X = 0.0f, .Y = -K, .Z= RADIUS}
	};
	
	struct POINT_3D_FLOAT triangle_1[] =
	{
	            [0] = { .X = 3.0f, .Y = 2.0f, .Z= -1.0f},
	            [1] = { .X = -3.0f, .Y = 8.0f, .Z= -5.0f},
	            [2] = { .X = -3.0f, .Y = 2.0f, .Z= 1.0f}
	};
	struct POINT_3D_FLOAT triangle_2[] =
	{
	            [0] = { .X = 0.0f, .Y = -K, .Z = RADIUS},
	            [1] = { .X = K, .Y = 0.0f, .Z = RADIUS},
	            [2] = { .X = K, .Y = -K, .Z = RADIUS}
	};
	
	
	struct POINT_3D_FLOAT triangle_3[] =
	{
	            [0] = { .X = 0.0f, .Y = 0.0f, .Z= RADIUS},
	            [1] = { .X = K, .Y = 0.0f, .Z= RADIUS},
	            [2] = { .X = K, .Y = 0.0f, .Z= K+RADIUS}
	};
	struct POINT_3D_FLOAT triangle_4[] =
	{
	            [0] = { .X = 0.0f, .Y = 0.0f, .Z= RADIUS},
	            [1] = { .X = K, .Y = 0.0f, .Z= RADIUS+K},
	            [2] = { .X = 0.0f, .Y = 0.0f, .Z= RADIUS+K}
	};
	struct POINT_3D_FLOAT triangle_5[] =
	{
	            [0] = { .X = 0.0f, .Y = 0.0f, .Z = RADIUS},
	            [1] = { .X = 0.0f, .Y = 0.0f, .Z = RADIUS+K},
	            [2] = { .X = 0.0f, .Y = -K, .Z = RADIUS+K}
	};
	struct POINT_3D_FLOAT triangle_6[] =
	{
	            [0] = { .X = 0.0f, .Y = 0.0f, .Z= RADIUS},
	            [1] = { .X = 0.0f, .Y = -K, .Z= RADIUS+K},
	            [2] = { .X = 0.0f, .Y = -K, .Z= RADIUS}
	};
	
	struct POINT_3D_FLOAT triangle_7[3];
	struct POINT_3D_FLOAT triangle_8[3];
	struct POINT_3D_FLOAT triangle_9[3];
	struct POINT_3D_FLOAT triangle_10[3];
	struct POINT_3D_FLOAT triangle_11[3];
	struct POINT_3D_FLOAT triangle_12[3];
	
	memcpy(triangle_7, triangle_1, sizeof(struct POINT_3D_FLOAT)*3);
	memcpy(triangle_8, triangle_2, sizeof(struct POINT_3D_FLOAT)*3);
	memcpy(triangle_9, triangle_3, sizeof(struct POINT_3D_FLOAT)*3);
	memcpy(triangle_10, triangle_4, sizeof(struct POINT_3D_FLOAT)*3);
	memcpy(triangle_11, triangle_5, sizeof(struct POINT_3D_FLOAT)*3);
	memcpy(triangle_12, triangle_6, sizeof(struct POINT_3D_FLOAT)*3);
	
	for (i=0; i<3; ++i){
		triangle_7[i].Z = 2 * K;
		triangle_8[i].Z = 2 * K;
		triangle_9[i].Y = -K;
		triangle_10[i].Y = -K;
		triangle_11[i].X = K;
		triangle_12[i].X = K;
	}
	
	struct TRIANGLE_3D_FLOAT list[CTRIANGLE];
	
	for (i=0; i<CTRIANGLE; ++i){
		list[i].COLOR.RED = 0;
		list[i].COLOR.GREEN = 0;
		list[i].COLOR.BLUE = 0;
	}
	
	list[0].TRIANGLE = triangle_1;
	list[1].TRIANGLE = triangle_2;
	list[2].TRIANGLE = triangle_3;
	list[3].TRIANGLE = triangle_4;
	list[4].TRIANGLE = triangle_5;
	list[5].TRIANGLE = triangle_6;
	
	list[6].TRIANGLE = triangle_7;
	list[7].TRIANGLE = triangle_8;
	list[8].TRIANGLE = triangle_9;
	list[9].TRIANGLE = triangle_10;
	list[10].TRIANGLE = triangle_11;
	list[11].TRIANGLE = triangle_12;
	
	
	list[0].COLOR.RED = 255;
	list[1].COLOR.RED = 255;
	
	list[2].COLOR.GREEN = 255;
	list[3].COLOR.GREEN= 255;
	
	list[4].COLOR.GREEN = 255;
	list[4].COLOR.RED = 255;
	list[5].COLOR.GREEN = 255;
	list[5].COLOR.RED = 255;
	
	list[6].COLOR.GREEN = 128;
	list[6].COLOR.BLUE = 128;
	list[7].COLOR.GREEN = 128;
	list[7].COLOR.BLUE = 128;
	
	list[8].COLOR.BLUE = 255;
	list[9].COLOR.BLUE = 255;
	
	list[10].COLOR.RED = 255;
	list[10].COLOR.BLUE = 255;
	list[11].COLOR.RED = 255;
	list[11].COLOR.BLUE = 255;
	
	for(i=0; i<CTRIANGLE; ++i){
		for(j=0; j<3; ++j){
			list[i].TRIANGLE[j].X += 100.0f;
			list[i].TRIANGLE[j].Y += 100.0f;
		}
	}*/
	
	struct POINT_3D_FLOAT triangle_1[] =
	{
	            [0] = { .X = -50.0f, .Y = -50.0f, .Z= 100.0f},
	            [1] = { .X = -50.0f, .Y = -50.0f, .Z= 50.0f},
	            [2] = { .X = 50.0f, .Y = -50.0f, .Z= 100.0f}
	};
	struct POINT_3D_FLOAT triangle_2[] =
	{
	            [0] = { .X = 50.0f, .Y = -50.0f, .Z = 50.0f},
	            [1] = { .X = -50.0f, .Y = -50.0f, .Z = 50.0f},
	            [2] = { .X = 50.0f, .Y = -50.0f, .Z = 100.0f}
	};
	
	struct TRIANGLE_3D_FLOAT list[CTRIANGLE];
	
	
	
	list[0].TRIANGLE = triangle_2;
	list[1].TRIANGLE = triangle_1;
	
	for (i=0; i<CTRIANGLE; ++i){
		list[i].COLOR.RED = 0;
		list[i].COLOR.GREEN = 0;
		list[i].COLOR.BLUE = 0;
	}
	
	list[1].COLOR.RED = 255;
	list[0].COLOR.BLUE = 255;
	/*
	for(i=0; i<CTRIANGLE; ++i){
		for(j=0; j<3; ++j){
			list[i].TRIANGLE[j].X += 25.0f;
			list[i].TRIANGLE[j].Y += 25.0f;
		}
	}
	*/
	float *depth_buffer = malloc(RADIUS*RADIUS*4 * sizeof(float));
	struct COLOR_MIX *frame_buffer = malloc(RADIUS*RADIUS*4 * sizeof(struct COLOR_MIX));
	/*
	for (i=0; i<RADIUS*2; ++i){
		for (j=0; j<RADIUS*2; ++j){
			frame_buffer[i*RADIUS*2+j].RED = 255;
			frame_buffer[i*RADIUS*2+j].GREEN = 0;
			frame_buffer[i*RADIUS*2+j].BLUE = 0;
		}
	}	*/
	zbuffer(list, CTRIANGLE, depth_buffer, frame_buffer);
	draw_buffer(renderer, frame_buffer);
	
	free(depth_buffer);
	free(frame_buffer);
	
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