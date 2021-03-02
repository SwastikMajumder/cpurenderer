#include <stdlib.h>
#include <string.h>
#include <SDL2/SDL.h>

#define WINDOW_WIDTH 1000

#define RADIUS 250
#define FOV 1

#define AREA_INACCURACY 1.0f

#define Z_LIMIT 1.0f

#define WALL_0002_SIZE(a, b) (12 * 2 * (a + 1) * b)

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

void zbuffer(struct TRIANGLE_3D_FLOAT *list, int list_length, struct COLOR_MIX *frame_buffer){
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
			for (k=0; k<list_length; ++k){
				if (list[k].TRIANGLE == NULL) continue;
				
				if (list[k].TRIANGLE[0].Z > Z_LIMIT && list[k].TRIANGLE[1].Z > Z_LIMIT && list[k].TRIANGLE[2].Z > Z_LIMIT){
					if (
					(float)(i-RADIUS) < ((RADIUS * FOV) * list[k].TRIANGLE[0].X)/list[k].TRIANGLE[0].Z &&
					(float)(i-RADIUS) < ((RADIUS * FOV) * list[k].TRIANGLE[1].X)/list[k].TRIANGLE[1].Z &&
					(float)(i-RADIUS) < ((RADIUS * FOV) * list[k].TRIANGLE[2].X)/list[k].TRIANGLE[2].Z
					){
						continue;
					}
					if (
					(float)(i-RADIUS) > ((RADIUS * FOV) * list[k].TRIANGLE[0].X)/list[k].TRIANGLE[0].Z &&
					(float)(i-RADIUS) > ((RADIUS * FOV) * list[k].TRIANGLE[1].X)/list[k].TRIANGLE[1].Z &&
					(float)(i-RADIUS) > ((RADIUS * FOV) * list[k].TRIANGLE[2].X)/list[k].TRIANGLE[2].Z
					){
						continue;
					}
					if (
					(float)(RADIUS-j) < ((RADIUS * FOV) * list[k].TRIANGLE[0].Y)/list[k].TRIANGLE[0].Z &&
					(float)(RADIUS-j) < ((RADIUS * FOV) * list[k].TRIANGLE[1].Y)/list[k].TRIANGLE[1].Z &&
					(float)(RADIUS-j) < ((RADIUS * FOV) * list[k].TRIANGLE[2].Y)/list[k].TRIANGLE[2].Z
					){
						continue;
					}
					if (
					(float)(RADIUS-j) > ((RADIUS * FOV) * list[k].TRIANGLE[0].Y)/list[k].TRIANGLE[0].Z &&
					(float)(RADIUS-j) > ((RADIUS * FOV) * list[k].TRIANGLE[1].Y)/list[k].TRIANGLE[1].Z &&
					(float)(RADIUS-j) > ((RADIUS * FOV) * list[k].TRIANGLE[2].Y)/list[k].TRIANGLE[2].Z
					){
						continue;
					}
				}
				if (list[k].TRIANGLE[0].Z <= 0.0f && list[k].TRIANGLE[1].Z <= 0.0f && list[k].TRIANGLE[2].Z <= 0.0f){
					continue;
				}
				normal_vector((list[k].TRIANGLE), (list[k].TRIANGLE) + 1, (list[k].TRIANGLE) + 2, &a, &b, &c);
				d = a*list[k].TRIANGLE[0].X + b*list[k].TRIANGLE[0].Y + c*list[k].TRIANGLE[0].Z;
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
/*
char str[80];
sprintf(str, "%d %d", n_1, n_2);
SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, str, str, NULL);
*/

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

void rotate_camera(struct TRIANGLE_3D_FLOAT *list, int index, int size, float a, float b, float c){
	int i;
	int j;
	for (i=0; i<size; ++i){
		for (j=0; j<3; ++j){
			rotate(list[index + i].TRIANGLE, a, b, c);
		}
	}
}

int new_index(struct TRIANGLE_3D_FLOAT *list, int size, int limit){
	int i, j;
	int streak=0;
	for (i=0; i<limit; ++i){
		if (list[i].TRIANGLE == NULL){
			++streak;
		} else {
			streak = 0;
		}
		if (streak == size){
			for (j=i-size+1; j<=i; ++j){
				list[j].TRIANGLE = calloc(3, sizeof(struct POINT_3D_FLOAT));
			}
			return i-size+1;
		}
	}
	return -1;
}

void delete_index(struct TRIANGLE_3D_FLOAT *list, int index, int size){
	int i;
	for (i=0; i<size; ++i){
		free(list[index + i].TRIANGLE);
		list[index + i].TRIANGLE = NULL;
	}
}

void obj_cuboid_0001(struct TRIANGLE_3D_FLOAT *list, int index, float x, float y, float z, int quadrant, float a, float b, float c){
	int i, j;
	int sx=1, sy=1, sz=1;
	switch (quadrant){
		case 1:
			break;
		case 2:
			sx = -1;
			break;
		case 3:
			sx = -1;
			sy = -1;
			break;
		case 4:
			sz = -1;
			sy = -1;
			break;
		case 5:
			sz = -1;
			break;
		case 6:
			sz = -1;
			sx = -1;
			break;
		case 7:
			sz = -1;
			sx = -1;
			sy = -1;
			break;
		case 8:
			sz = -1;
			sy = -1;
			break;
	}
	
	list[index].TRIANGLE[0].X = x;
	list[index].TRIANGLE[0].Y = y;
	list[index].TRIANGLE[0].Z = z;
	
	list[index].TRIANGLE[1].X = x;
	list[index].TRIANGLE[1].Y = y;
	list[index].TRIANGLE[1].Z = sz*c + z;
	
	list[index].TRIANGLE[2].X = sx*a + x;
	list[index].TRIANGLE[2].Y = y;
	list[index].TRIANGLE[2].Z = sz*c + z;
	
	list[index + 1].TRIANGLE[0].X = x;
	list[index + 1].TRIANGLE[0].Y = y;
	list[index + 1].TRIANGLE[0].Z = z;
	
	list[index + 1].TRIANGLE[1].X = sx*a + x;
	list[index + 1].TRIANGLE[1].Y = y;
	list[index + 1].TRIANGLE[1].Z = z;
	
	list[index + 1].TRIANGLE[2].X = sx*a + x;
	list[index + 1].TRIANGLE[2].Y = y;
	list[index + 1].TRIANGLE[2].Z = sz*c + z;
	
	list[index + 2].TRIANGLE[0].X = x;
	list[index + 2].TRIANGLE[0].Y = y;
	list[index + 2].TRIANGLE[0].Z = z;
	
	list[index + 2].TRIANGLE[1].X = x;
	list[index + 2].TRIANGLE[1].Y = y;
	list[index + 2].TRIANGLE[1].Z = sz*c + z;
	
	list[index + 2].TRIANGLE[2].X = x;
	list[index + 2].TRIANGLE[2].Y = sy*b + y;
	list[index + 2].TRIANGLE[2].Z = sz*c + z;
	
	list[index + 3].TRIANGLE[0].X = x;
	list[index + 3].TRIANGLE[0].Y = y;
	list[index + 3].TRIANGLE[0].Z = z;
	
	list[index + 3].TRIANGLE[1].X = x;
	list[index + 3].TRIANGLE[1].Y = sy*b + y;
	list[index + 3].TRIANGLE[1].Z = z;
	
	list[index + 3].TRIANGLE[2].X = x;
	list[index + 3].TRIANGLE[2].Y = sy*b + y;
	list[index + 3].TRIANGLE[2].Z = sz*c + z;
	
	list[index + 4].TRIANGLE[0].X = x;
	list[index + 4].TRIANGLE[0].Y = y;
	list[index + 4].TRIANGLE[0].Z = z;
	
	list[index + 4].TRIANGLE[1].X = sx*a + x;
	list[index + 4].TRIANGLE[1].Y = sy*b + y;
	list[index + 4].TRIANGLE[1].Z = z;
	
	list[index + 4].TRIANGLE[2].X = x;
	list[index + 4].TRIANGLE[2].Y = sy*b + y;
	list[index + 4].TRIANGLE[2].Z = z;
	
	list[index + 5].TRIANGLE[0].X = x;
	list[index + 5].TRIANGLE[0].Y = y;
	list[index + 5].TRIANGLE[0].Z = z;
	
	list[index + 5].TRIANGLE[1].X = sx*a + x;
	list[index + 5].TRIANGLE[1].Y = sy*b + y;
	list[index + 5].TRIANGLE[1].Z = z;
	
	list[index + 5].TRIANGLE[2].X = sx*a + x;
	list[index + 5].TRIANGLE[2].Y = y;
	list[index + 5].TRIANGLE[2].Z = z;
	
	for (i=0; i<6; ++i){
		for (j=0; j<3; ++j){
			list[index + i + 6].TRIANGLE[j].X = list[index + i].TRIANGLE[j].X;
			list[index + i + 6].TRIANGLE[j].Y = list[index + i].TRIANGLE[j].Y;
			list[index + i + 6].TRIANGLE[j].Z = list[index + i].TRIANGLE[j].Z;
		}
	}
	for (i=0; i<2; ++i){
		for (j=0; j<3; ++j){
			list[index + i + 6].TRIANGLE[j].Y += sy*b;
		}
	}
	for (i=0; i<2; ++i){
		for (j=0; j<3; ++j){
			list[index + i + 8].TRIANGLE[j].X += sx*a;
		}
	}
	for (i=0; i<2; ++i){
		for (j=0; j<3; ++j){
			list[index + i + 10].TRIANGLE[j].Z += sz*c;
		}
	}
}

void fill_color(struct TRIANGLE_3D_FLOAT *list, int index, int size, unsigned char red, unsigned char green, unsigned char blue){
	int i;
	for (i=0; i<size; ++i){
		list[index + i].COLOR.RED = red;
		list[index + i].COLOR.GREEN = green;
		list[index + i].COLOR.BLUE = blue;
	}
}

void fill_color_cuboid_0001(struct TRIANGLE_3D_FLOAT *list, int index){
	fill_color(list, index+0, 2, 255, 0, 0);
	fill_color(list, index+6, 2, 255, 0, 0);
	fill_color(list, index+2, 2, 0, 255, 0);
	fill_color(list, index+8, 2, 0, 255, 0);
	fill_color(list, index+4, 2, 0, 0, 255);
	fill_color(list, index+10, 2, 0, 0, 255);
}


void obj_wall_0001(struct TRIANGLE_3D_FLOAT *list, int index, struct POINT_3D_FLOAT *location, int wall_height, int wall_length, unsigned char red, unsigned char green, unsigned char blue){
	int i, j;
	float x = location->X;
	float y = location->Y;
	float z = location->Z;
	int c1 = 0;
	int c2 = 1;
	for (i=0; i<wall_height; ++i){
		c1 = 0;
		c2 = 1;
		/*
char str[80];
sprintf(str, "%d", index);
SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, str, str, NULL);*/
		obj_cuboid_0001(list, index + 12 * (i * 2 * (wall_length + 1)), x, y + 25.0f * (i * 2 + 1), z, 1, 25.0f, 25.0f, 25.0f);
		c1 = !c1;
		fill_color(list, index + 12 * (i * 2 * (wall_length + 1)), 12, c1 ? red>>1 : red, c1 ? green>>1 : green, c1 ? blue>>1 : blue);
		for (j=0; j<wall_length; ++j){
			obj_cuboid_0001(list, index + 12 * (i * 2 * (wall_length + 1) + j * 2 + 1), x + (j * 50.0f), y + 25.0f * i * 2, z, 1, 50.0f, 25.0f, 25.0f);
			c2 = !c2;
			fill_color(list, index + 12 * (i * 2 * (wall_length + 1) + j * 2 + 1), 12, c2 ? red>>1 : red, c2 ? green>>1 : green, c2 ? blue>>1 : blue);
			obj_cuboid_0001(list, index + 12 * (i * 2 * (wall_length + 1) + j * 2 + 2), x + (j * 50.0f) + 25.0f, y + 25.0f * (i * 2 + 1), z, 1, 50.0f, 25.0f, 25.0f);
			c1 = !c1;
			fill_color(list, index + 12 * (i * 2 * (wall_length + 1) + j * 2 + 2), 12, c1 ? red>>1 : red, c1 ? green>>1 : green, c1 ? blue>>1 : blue);
		}
		obj_cuboid_0001(list, index + 12 * (i * 2 * (wall_length + 1) + j * 2 + 1), x + (j * 50.0f), y + 25.0f * i * 2, z, 1, 25.0f, 25.0f, 25.0f);
		c2 = !c2;
		fill_color(list, index + 12 * (i * 2 * (wall_length + 1) + j * 2 + 1), 12, c2 ? red>>1 : red, c2 ? green>>1 : green, c2 ? blue>>1 : blue);
	}
}

void obj_wall_0002(struct TRIANGLE_3D_FLOAT *list, int index, struct POINT_3D_FLOAT *location, int wall_height, int wall_length, unsigned char red, unsigned char green, unsigned char blue){
	int i;
	int j;
	obj_wall_0001(list, index, location, wall_height, wall_length, red, green, blue);
	for (i=0; i<WALL_0002_SIZE(wall_height, wall_length); ++i){
		for (j=0; j<3; ++j){
			float tmp = list[index + i].TRIANGLE[j].X;
			list[index + i].TRIANGLE[j].X = list[index + i].TRIANGLE[j].Z + location->X - location->Z;
			list[index + i].TRIANGLE[j].Z = location->X + location->Z - tmp;
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
	
	//int i;
	
	struct COLOR_MIX *frame_buffer = malloc(RADIUS*RADIUS*4 * sizeof(struct COLOR_MIX));
	struct TRIANGLE_3D_FLOAT *list = malloc(WALL_0002_SIZE(2, 2) * sizeof(struct TRIANGLE_3D_FLOAT));
	
	int index = new_index(list, WALL_0002_SIZE(2, 2), WALL_0002_SIZE(2, 2));
	
	struct POINT_3D_FLOAT location = {.X = -200.0f, .Y = 0.0f, .Z = 250.0f };

	obj_wall_0002(list, index, &location, 2, 2, 255, 0, 0);
	rotate_camera(list, index, WALL_0002_SIZE(2, 2), 0.0f, (10 * M_PI)/180, 0.0f);
	
	zbuffer(list, WALL_0002_SIZE(2, 2), frame_buffer);
	draw_buffer(renderer, frame_buffer);
	
	delete_index(list, index, WALL_0002_SIZE(2, 2));
	free(list);
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
