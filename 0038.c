#include <stdlib.h>
#include <string.h>
#include <SDL2/SDL.h>

#define WINDOW_WIDTH 1000

#define K 50.0f

#define RADIUS 250
#define FOV 1

#define AREA_INACCURACY 1.0f

#define CTRIANGLE 2

struct POINT_3D_FLOAT {
	float X;
	float Y;
	float Z;
};

struct COLOR_MIX {
	unsigned char RED;
	unsigned char GREEN;
	unsigned char BLUE;
};

struct TRIANGLE_3D_FLOAT {
	struct COLOR_MIX COLOR;
	struct POINT_3D_FLOAT *TRIANGLE;
};

void normal_vector(struct POINT_3D_FLOAT *p, struct POINT_3D_FLOAT *q, struct POINT_3D_FLOAT *r, float *a, float *b, float *c){
	*a = -(r->Y - p->Y) * (q->Z - p->Z) + (q->Y - p->Y) * (r->Z - p->Z);
	*b = -(q->X - p->X) * (r->Z - p->Z) + (r->X - p->X) * (q->Z - p->Z);
	*c = -(r->X - p->X) * (q->Y - p->Y) + (q->X - p->X) * (r->Y - p->Y);	
}

float area_triangle(struct POINT_3D_FLOAT *p, struct POINT_3D_FLOAT *q, struct POINT_3D_FLOAT *r){
	float a, b, c;
	normal_vector(p, q, r, &a, &b, &c);
	float result = 0.5 * sqrt(a*a + b*b + c*c);
	return result;
}

int inside_triangle(struct POINT_3D_FLOAT *triangle, struct POINT_3D_FLOAT *s){
	float a = area_triangle(triangle, triangle + 1, s);
	float b = area_triangle(triangle + 1, triangle + 2, s);
	float c = area_triangle(triangle + 2, triangle, s);
	float d = a + b + c - area_triangle(triangle, triangle + 1, triangle + 2);
	if (d < 0.0f) d= -d;
	if (d < AREA_INACCURACY){
		return 1;
	}
	return 0;
}

void zbuffer(struct TRIANGLE_3D_FLOAT *list, struct COLOR_MIX *frame_buffer){
	int i;
	int j;
	int k;
	float a, b, c;
	float d;
	float depth;
	struct POINT_3D_FLOAT s;
	for (i=0; i<(RADIUS*RADIUS*4); ++i){
		frame_buffer[i].RED = 255;
		frame_buffer[i].GREEN = 255;
		frame_buffer[i].BLUE = 255;
	}
	for (i=0; i<RADIUS*2; ++i){
		for (j=0; j<RADIUS*2; ++j){
			depth = 100000.0f;
			for (k=0; k<CTRIANGLE; ++k){
				normal_vector((list[k].TRIANGLE), (list[k].TRIANGLE) + 1, (list[k].TRIANGLE) + 2, &a, &b, &c);
				d = a*list[i].TRIANGLE[0].X + b*list[i].TRIANGLE[0].Y + c*list[i].TRIANGLE[0].Z;
				s.X = d/(((b*(RADIUS-j) + c*RADIUS*FOV)/(i-RADIUS))+a);
				s.Y = d/(((a*(i-RADIUS) + c*RADIUS*FOV)/(RADIUS-j))+b);
				s.Z = d/(((a*(i-RADIUS) + b*(RADIUS-j))/(RADIUS * FOV)) + c);
				if (inside_triangle(list[k].TRIANGLE, &s)){
					if (depth > s.Z){
						memcpy(frame_buffer + (i*RADIUS*2 + j), &(list[k].COLOR), sizeof(struct COLOR_MIX));
						depth = s.Z;
					}
				}
			}
		}
	}
}

//SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "msg", "msg", NULL);

//char str[80];
//sprintf(str, "%d %d", n_1, n_2);
//SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, str, str, NULL);

void draw_buffer(SDL_Renderer *renderer, struct COLOR_MIX *frame_buffer){
	SDL_Texture *Tile = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, RADIUS*2, RADIUS*2);	
	unsigned char *bytes;
    int pitch;
    SDL_LockTexture(Tile, NULL, (void **)&bytes, &pitch);
    int i, j; 
    for (i=0; i<RADIUS*2; ++i){
    	for (j=0; j<RADIUS*2; ++j){
    		bytes[(i*RADIUS*8+j*4)] = 255;
    		bytes[(i*RADIUS*8+j*4) + 1] = frame_buffer[j*RADIUS*2+i].BLUE;
    		bytes[(i*RADIUS*8+j*4) + 2] = frame_buffer[j*RADIUS*2+i].GREEN;
    		bytes[(i*RADIUS*8+j*4) + 3] = frame_buffer[j*RADIUS*2+i].RED;
    	}
    }
    SDL_UnlockTexture(Tile);
    SDL_Rect destination = {0, 0, RADIUS*2, RADIUS*2};
    SDL_RenderCopy(renderer, Tile, NULL, &destination);
    SDL_RenderPresent(renderer);
    SDL_Delay(100);
}

/*
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
*/

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
	
	
	
	list[0].TRIANGLE = triangle_1;
	list[1].TRIANGLE = triangle_2;
	
	for (i=0; i<CTRIANGLE; ++i){
		list[i].COLOR.RED = 0;
		list[i].COLOR.GREEN = 0;
		list[i].COLOR.BLUE = 0;
	}
	
	list[0].COLOR.BLUE = 255;
	list[1].COLOR.GREEN = 255;
	
	struct COLOR_MIX *frame_buffer = malloc(RADIUS*RADIUS*4 * sizeof(struct COLOR_MIX));
	
	for (;;){
		zbuffer(list, frame_buffer);
		draw_buffer(renderer, frame_buffer);
		for(i=0; i<CTRIANGLE; ++i){
			for(j=0; j<3; ++j){
				list[i].TRIANGLE[j].Z -= 5.0f;
			}
		}
	}
	
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