#include <stdlib.h>
#include <string.h>
#include <SDL2/SDL.h>

#define WINDOW_WIDTH 1000

#define K 50.0f

#define MAX_X 500
#define MAX_Y 500

#define RADIUS 250
#define FOV 1

#define Z_LIMIT 1.0f

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

void zbuffer(struct TRIANGLE_3D_FLOAT *list, int list_length, float *depth_buffer, struct COLOR_MIX *frame_buffer){
	int i;
	int j;
	int tmp, tmp_5, tmp_6;
	float tmp_2, tmp_3, tmp_4;
	float a, b, c, z;
	tmp_5 = 0;
	//char str[80];
	for (i=0; i<(RADIUS*RADIUS*4); ++i){
		depth_buffer[i] = 10000.0f;
		frame_buffer[i].RED = 255;
		frame_buffer[i].GREEN = 255;
		frame_buffer[i].BLUE = 255;
	}
	for (i=0; i<2; ++i){
		struct POINT_2D_INT p[3];
		if (tmp_5 == 0){
			tmp = (list[i].TRIANGLE[0].Z <= 0.0f) + (list[i].TRIANGLE[1].Z <= 0.0f) + (list[i].TRIANGLE[2].Z <= 0.0f);
			if (tmp == 3) continue;
			else if (tmp == 2){
				for (j=0; j<3; ++j){
					if (list[i].TRIANGLE[j].Z > 0.0f){
						break;
					}
				}		

				//SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "hi", "hi", NULL);

				list[i].TRIANGLE[j<2?j+1:0].X = (Z_LIMIT - list[i].TRIANGLE[j].Z)/(list[i].TRIANGLE[j<2?j+1:0].Z - list[i].TRIANGLE[j].Z) * (list[i].TRIANGLE[j<2?j+1:0].X - list[i].TRIANGLE[j].X) + list[i].TRIANGLE[j].X;
				list[i].TRIANGLE[j<2?j+1:0].Y = (Z_LIMIT - list[i].TRIANGLE[j].Z)/(list[i].TRIANGLE[j<2?j+1:0].Z - list[i].TRIANGLE[j].Z) * (list[i].TRIANGLE[j<2?j+1:0].Y - list[i].TRIANGLE[j].Y) + list[i].TRIANGLE[j].Y;
				list[i].TRIANGLE[j<2?j+1:0].Z = Z_LIMIT;
			
			//	sprintf(str, "%d %3.3f %3.3f %3.3f", j, list[i].TRIANGLE[0].X, list[i].TRIANGLE[0].Y, list[i].TRIANGLE[0].Z);
	//			SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, str, str, NULL);
			
				list[i].TRIANGLE[j>0?j-1:2].X = (Z_LIMIT - list[i].TRIANGLE[j].Z)/(list[i].TRIANGLE[j>0?j-1:2].Z - list[i].TRIANGLE[j].Z) * (list[i].TRIANGLE[j>0?j-1:2].X - list[i].TRIANGLE[j].X) + list[i].TRIANGLE[j].X;
				list[i].TRIANGLE[j>0?j-1:2].Y = (Z_LIMIT - list[i].TRIANGLE[j].Z)/(list[i].TRIANGLE[j>0?j-1:2].Z - list[i].TRIANGLE[j].Z) * (list[i].TRIANGLE[j>0?j-1:2].Y - list[i].TRIANGLE[j].Y) + list[i].TRIANGLE[j].Y;
				list[i].TRIANGLE[j>0?j-1:2].Z = Z_LIMIT;
				
		//		sprintf(str, "%3.3f %3.3f %3.3f", list[i].TRIANGLE[1].X, list[i].TRIANGLE[1].Y, list[i].TRIANGLE[1].Z);
			//	SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, str, str, NULL);
			}
			else if (tmp == 1){
				for (j=0; j<3; ++j){
					if (list[i].TRIANGLE[j].Z <= 0.0f){
						break;
					}
				}
				
				tmp_2 = list[i].TRIANGLE[j].X;
				tmp_3 = list[i].TRIANGLE[j].Y;
				tmp_4 = list[i].TRIANGLE[j].Z;
				
				list[i].TRIANGLE[j].X = (Z_LIMIT - list[i].TRIANGLE[j].Z)/(list[i].TRIANGLE[j<2?j+1:0].Z - list[i].TRIANGLE[j].Z) * (list[i].TRIANGLE[j<2?j+1:0].X - list[i].TRIANGLE[j].X) + list[i].TRIANGLE[j].X;
				list[i].TRIANGLE[j].Y = (Z_LIMIT - list[i].TRIANGLE[j].Z)/(list[i].TRIANGLE[j<2?j+1:0].Z - list[i].TRIANGLE[j].Z) * (list[i].TRIANGLE[j<2?j+1:0].Y - list[i].TRIANGLE[j].Y) + list[i].TRIANGLE[j].Y;
				list[i].TRIANGLE[j].Z = Z_LIMIT;
			
				tmp_5 = 1;
				tmp_6 = j;
			}
		} else {
			--i;
			tmp_5 = 0;
		}
		for (j=0; j<3; ++j){
			p[j].X = RADIUS + (int)round((list[i].TRIANGLE[j].X * RADIUS * FOV) / list[i].TRIANGLE[j].Z);
			p[j].Y = RADIUS - (int)round((list[i].TRIANGLE[j].Y * RADIUS * FOV) / list[i].TRIANGLE[j].Z);
			
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
		//	sprintf(str, "%d %d", p[j].X, p[j].Y);
				//SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, str, str, NULL);
		}
		for (k=x_min; k<=x_max; ++k){
			//if (k==x_min+25 && i==1) return;
			for (l=y_min; l<=y_max; ++l){
				if (inside(k - RADIUS, RADIUS - l, p)){
					a = -(list[i].TRIANGLE[2].Y - list[i].TRIANGLE[0].Y) * (list[i].TRIANGLE[1].Z - list[i].TRIANGLE[0].Z) + (list[i].TRIANGLE[1].Y - list[i].TRIANGLE[0].Y) * (list[i].TRIANGLE[2].Z - list[i].TRIANGLE[0].Z);
					b = -(list[i].TRIANGLE[1].X - list[i].TRIANGLE[0].X) * (list[i].TRIANGLE[2].Z - list[i].TRIANGLE[0].Z) + (list[i].TRIANGLE[2].X - list[i].TRIANGLE[0].X) * (list[i].TRIANGLE[1].Z - list[i].TRIANGLE[0].Z);
					c = -(list[i].TRIANGLE[2].X - list[i].TRIANGLE[0].X) * (list[i].TRIANGLE[1].Y - list[i].TRIANGLE[0].Y) + (list[i].TRIANGLE[1].X - list[i].TRIANGLE[0].X) * (list[i].TRIANGLE[2].Y - list[i].TRIANGLE[0].Y);
					z = (a*list[i].TRIANGLE[0].X + b*list[i].TRIANGLE[0].Y + c*list[i].TRIANGLE[0].Z)/(((a*(k - RADIUS) + b*(RADIUS - l))/(RADIUS * FOV)) + c);
					if (depth_buffer[k*RADIUS*2+l] > z){
						depth_buffer[k*RADIUS*2+l] = z;
						memcpy(frame_buffer + (k*RADIUS*2 + l), &(list[i].COLOR), sizeof(struct COLOR_MIX));
					}
				}
			}
		}
		if (tmp_5){
			list[i].TRIANGLE[tmp_6].X = (Z_LIMIT - tmp_4)/(list[i].TRIANGLE[tmp_6>0?tmp_6-1:2].Z - tmp_4) * (list[i].TRIANGLE[tmp_6>0?tmp_6-1:2].X - tmp_2) + tmp_2;
			list[i].TRIANGLE[tmp_6].Y = (Z_LIMIT - tmp_4)/(list[i].TRIANGLE[tmp_6>0?tmp_6-1:2].Z - tmp_4) * (list[i].TRIANGLE[tmp_6>0?tmp_6-1:2].Y - tmp_3) + tmp_3;
		}
	}
}

//SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "hi", "hi", NULL);

//			char str[80];
//			sprintf(str, "%d %d", p[j].X, p[j].Y);
//			SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, str, str, NULL);

void draw_buffer(SDL_Renderer *renderer, struct COLOR_MIX *frame_buffer, struct COLOR_MIX *pick){
    int i, j, k;
    for (i=0; i<7; ++i){
   	SDL_SetRenderDrawColor(renderer, pick[i].RED, pick[i].GREEN, pick[i].BLUE, 255);
    	for (j=0; j<RADIUS*2; ++j){
    		for (k=0; k<RADIUS*2; ++k){
    			if (frame_buffer[j*RADIUS*2 + k].RED == pick[i].RED && frame_buffer[j*RADIUS*2 + k].GREEN == pick[i].GREEN && frame_buffer[j*RADIUS*2 + k].BLUE == pick[i].BLUE){
    				SDL_RenderDrawPoint(renderer, j, k);
    			}
    		}
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
	
	struct POINT_3D_FLOAT triangle_1[] =
	{
	            [0] = { .X = -250.0f, .Y = -150.0f, .Z= 300.0f},
	            [1] = { .X = -250.0f, .Y = -150.0f, .Z= 50.0f},
	            [2] = { .X = 250.0f, .Y = -150.0f, .Z= 300.0f}
	};
	struct POINT_3D_FLOAT triangle_2[] =
	{
	            [0] = { .X = 250.0f, .Y = -150.0f, .Z = 50.0f},
	            [1] = { .X = -250.0f, .Y = -150.0f, .Z = 50.0f},
	            [2] = { .X = 250.0f, .Y = -150.0f, .Z = 300.0f}
	};
	
	struct TRIANGLE_3D_FLOAT list[CTRIANGLE];
	
	
	
	list[0].TRIANGLE = triangle_2;
	list[1].TRIANGLE = triangle_1;
	
	for (i=0; i<CTRIANGLE; ++i){
		list[i].COLOR.RED = 0;
		list[i].COLOR.GREEN = 0;
		list[i].COLOR.BLUE = 0;
	}
	
	list[1].COLOR.GREEN = 255;
	list[0].COLOR.BLUE = 255;
	
	for(i=0; i<CTRIANGLE; ++i){
		for(j=0; j<3; ++j){
			//list[i].TRIANGLE[j].X -= 75.0f;
	//		list[i].TRIANGLE[j].Z -= 60.0f;
		}
	}
	

	struct COLOR_MIX pick[] =
	{
		[0] = {.RED = 255, .GREEN = 255, .BLUE = 255},
		[1] = {.RED = 0, .GREEN = 255, .BLUE = 0},
		[2] = {.RED = 0, .GREEN = 0, .BLUE = 255},
	};
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
	for (;;){
		zbuffer(list, CTRIANGLE, depth_buffer, frame_buffer);
		draw_buffer(renderer, frame_buffer, pick);
		SDL_RenderPresent(renderer);
		SDL_Delay(100);
		//SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "hi", "hi", NULL);
		for(i=0; i<CTRIANGLE; ++i){
			for(j=0; j<3; ++j){
				list[i].TRIANGLE[j].Z -= 5.0f;
			}
		}
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
	    SDL_RenderClear(renderer);	
}
	
	free(depth_buffer);
	free(frame_buffer);
	
    while (1) {
        if (SDL_PollEvent(&event) && event.type == SDL_QUIT)
            break;
    }
    
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
   return EXIT_SUCCESS;
}