#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdio.h>
#include <SDL.h>

#define TOTAL_TRIANGLE_COUNT 14

#define TEXTURE_SIZE 100

#define WINDOW_WIDTH 1000

#define RADIUS 250
#define FOV 1

#define AREA_INACCURACY 1.0f

#define MAXIMUM_VALUE(a, b, c) (((a) > (b)) ? (((c) > (a)) ? (c) : (a)) : (((c) > (b)) ? (c) : (b)))
#define MINIMUM_VALUE(a, b, c) (((a) < (b)) ? (((c) < (a)) ? (c) : (a)) : (((c) < (b)) ? (c) : (b)))

char *input_texture_file = "texture0006.ppm";

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
    char *TEXTURE;
    int IS_TEXTURE;
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

void zbuffer(struct TRIANGLE_3D_FLOAT *list, int list_length, float *depth_buffer, struct COLOR_MIX *frame_buffer){
	int i;
	int j;
	int k;
	int x_max, x_min, y_max, y_min;
	float a, b, c;
	float d;
	int rect_con = 0;
	struct POINT_3D_FLOAT s;
	struct POINT_3D_FLOAT *u;
	struct POINT_3D_FLOAT *v;
	struct POINT_3D_FLOAT uv;
	struct POINT_3D_FLOAT us;

	//Set depth and frame buffer
	for (i=0; i<(RADIUS*RADIUS*4); ++i){
	    depth_buffer[i] = 100000.0f;
		frame_buffer[i].RED = 255;
		frame_buffer[i].GREEN = 255;
		frame_buffer[i].BLUE = 255;
	}

	//For all triangles
	for (i=0; i<list_length; ++i){
	    struct POINT_3D_FLOAT *p = list[i].TRIANGLE;
	    if (p == NULL) continue; //No triangle in the list

        //All points behind the camera
	    if (p[0].Z <= 0.0f && p[1].Z <= 0.0f && p[2].Z <= 0.0f){
			continue;
		}

		//Initially setup for choosing all pixels
	    x_max = (RADIUS*2) - 1;
	    x_min = 0;
	    y_max = (RADIUS*2) - 1;
	    y_min = 0;

        //If all the points of the triangle are in front of the camera
	    if (p[0].Z > 0.0f && p[1].Z > 0.0f && p[2].Z > 0.0f){
		    float x_0, x_1, x_2, y_0, y_1, y_2;
		    //Project the points
		    x_0 = RADIUS + (RADIUS * FOV * p[0].X)/p[0].Z;
		    x_1 = RADIUS + (RADIUS * FOV * p[1].X)/p[1].Z;
		    x_2 = RADIUS + (RADIUS * FOV * p[2].X)/p[2].Z;
		    y_0 = RADIUS - (RADIUS * FOV * p[0].Y)/p[0].Z;
		    y_1 = RADIUS - (RADIUS * FOV * p[1].Y)/p[1].Z;
		    y_2 = RADIUS - (RADIUS * FOV * p[2].Y)/p[2].Z;
		    x_max = (int) round(MAXIMUM_VALUE(x_0, x_1, x_2));
		    x_min = (int) round(MINIMUM_VALUE(x_0, x_1, x_2));
	    	y_max = (int) round(MAXIMUM_VALUE(y_0, y_1, y_2));
    		y_min = (int) round(MINIMUM_VALUE(y_0, y_1, y_2));

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
	    }

	    //For all pixels in range
    	for(j=x_min; j<=x_max; ++j){
    	    for (k=y_min; k<=y_max; ++k){
				normal_vector(p, p + 1, p + 2, &a, &b, &c);
				d = a*p[0].X + b*p[0].Y + c*p[0].Z;
				s.X = d/(((b*(RADIUS-k) + c*RADIUS*FOV)/(j-RADIUS))+a);
				s.Y = d/(((a*(j-RADIUS) + c*RADIUS*FOV)/(RADIUS-k))+b);
				s.Z = d/(((a*(j-RADIUS) + b*(RADIUS-k))/(RADIUS * FOV)) + c);

				if (inside_triangle(p, &s)){
					if (depth_buffer[j*RADIUS*2 + k] > s.Z){

						if (list[i].IS_TEXTURE == 0){
                            memcpy(frame_buffer + (j*RADIUS*2 + k), &(list[i].COLOR), sizeof(struct COLOR_MIX));
						}
						else {
							rect_con = list[i].IS_TEXTURE - 1;
						    u = list[i-rect_con].TRIANGLE;
						    v = (list[i-rect_con].TRIANGLE) + 1;
						    uv.X = v->X - u->X;
    						uv.Y = v->Y - u->Y;
    						uv.Z = v->Z - u->Z;
    						us.X = s.X - u->X;
    						us.Y = s.Y - u->Y;
    						us.Z = s.Z - u->Z;
    						float m_us = us.X * us.X + us.Y * us.Y + us.Z * us.Z;
    						float m_uv = uv.X * uv.X + uv.Y * uv.Y + uv.Z * uv.Z;
    						float uv_dot_us = uv.X * us.X + uv.Y * us.Y + uv.Z * us.Z;
    						if (m_us <= 0) continue;
    						if (m_uv <= 0) continue;
    						if (uv_dot_us <= 0.0) uv_dot_us = 0.0f;
    						float h = sqrt(m_us);
						    float cos_theta = (uv_dot_us)/(h * (sqrt(m_uv)));
						    float sin_theta = sqrt(1 - (cos_theta * cos_theta));
						    int x_sc = (int)(sin_theta*h);
						    x_sc = x_sc % TEXTURE_SIZE;

						    int y_sc = (int)(cos_theta*h);
						    y_sc = y_sc % TEXTURE_SIZE;

						    frame_buffer[j*RADIUS*2 + k].RED = list[i-rect_con].TEXTURE[12+y_sc*TEXTURE_SIZE*3 + x_sc*3];
						    frame_buffer[j*RADIUS*2 + k].GREEN = list[i-rect_con].TEXTURE[12+y_sc*TEXTURE_SIZE*3 + x_sc*3 + 1];
						    frame_buffer[j*RADIUS*2 + k].BLUE = list[i-rect_con].TEXTURE[12+y_sc*TEXTURE_SIZE*3 + x_sc*3 + 2];

						}
						depth_buffer[j*RADIUS*2 + k] = s.Z;

					}
				}
			}
		}
	}
}

void rotate_camera(struct TRIANGLE_3D_FLOAT *list, int index, int size, float angle){
	int i;
	int j;
	float x;
	for (i=0; i<size; ++i){
        for (j=0; j<3; ++j){
            float x = list[index + i].TRIANGLE[j].X*cos(angle)-list[index + i].TRIANGLE[j].Z*sin(angle);
            list[index + i].TRIANGLE[j].Z = list[index + i].TRIANGLE[j].X*sin(angle)+list[index + i].TRIANGLE[j].Z*cos(angle);
            list[index + i].TRIANGLE[j].X = x;
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
				memset(list[j].TRIANGLE, 0, 3 * sizeof(struct POINT_3D_FLOAT));
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

void obj_road(struct TRIANGLE_3D_FLOAT *list, int index, float x, float y, float z, float l, float b){
    list[index].TRIANGLE[0].X = x;
    list[index].TRIANGLE[0].Y = y;
    list[index].TRIANGLE[0].Z = z;

    list[index].TRIANGLE[1].X = x;
    list[index].TRIANGLE[1].Y = y;
    list[index].TRIANGLE[1].Z = z+l;

    list[index].TRIANGLE[2].X = x+b;
    list[index].TRIANGLE[2].Y = y;
    list[index].TRIANGLE[2].Z = z;

    list[index+1].TRIANGLE[0].X = x;
    list[index+1].TRIANGLE[0].Y = y;
    list[index+1].TRIANGLE[0].Z = z+l;

    list[index+1].TRIANGLE[1].X = x+b;
    list[index+1].TRIANGLE[1].Y = y;
    list[index+1].TRIANGLE[1].Z = z;

    list[index+1].TRIANGLE[2].X = x+b;
    list[index+1].TRIANGLE[2].Y = y;
    list[index+1].TRIANGLE[2].Z = z+l;
}


void obj_textured_rectangle(struct TRIANGLE_3D_FLOAT *list, char *texture, int index,
                        float ax, float ay, float az,
                        float bx, float by, float bz,
                        float cx, float cy, float cz,
                        float dx, float dy, float dz){

    list[index].TEXTURE = texture;
    list[index].IS_TEXTURE = 1;
    list[index+1].IS_TEXTURE = 2;
    list[index+1].TEXTURE = NULL;

    list[index].TRIANGLE[0].X = ax;
    list[index].TRIANGLE[0].Y = ay;
    list[index].TRIANGLE[0].Z = az;

    list[index].TRIANGLE[1].X = bx;
    list[index].TRIANGLE[1].Y = by;
    list[index].TRIANGLE[1].Z = bz;

    list[index].TRIANGLE[2].X = cx;
    list[index].TRIANGLE[2].Y = cy;
    list[index].TRIANGLE[2].Z = cz;

    list[index+1].TRIANGLE[0].X = bx;
    list[index+1].TRIANGLE[0].Y = by;
    list[index+1].TRIANGLE[0].Z = bz;

    list[index+1].TRIANGLE[1].X = cx;
    list[index+1].TRIANGLE[1].Y = cy;
    list[index+1].TRIANGLE[1].Z = cz;

    list[index+1].TRIANGLE[2].X = dx;
    list[index+1].TRIANGLE[2].Y = dy;
    list[index+1].TRIANGLE[2].Z = dz;
}

void obj_skinned_box(struct TRIANGLE_3D_FLOAT *list, char *texture, int index, float x, float y, float z, float l, float b, float h){
    obj_textured_rectangle(list, texture, index,
                       x, y, z,
                       x, y+h, z,
                       x+l, y, z,
                       x+l, y+h, z);
    obj_textured_rectangle(list, texture, index+2,
                       x, y, z,
                       x, y+h, z,
                       x, y, z+b,
                       x, y+h, z+b);
    obj_textured_rectangle(list, texture, index+4,
                       x, y, z,
                       x, y, z+b,
                       x+l, y, z,
                       x+l, y, z+b);

    obj_textured_rectangle(list, texture, index+6,
                       x, y, z+b,
                       x, y+h, z+b,
                       x+l, y, z+b,
                       x+l, y+h, z+b);
    obj_textured_rectangle(list, texture, index+8,
                       x+l, y, z,
                       x+l, y+h, z,
                       x+l, y, z+b,
                       x+l, y+h, z+b);
    obj_textured_rectangle(list, texture, index+10,
                       x, y+h, z,
                       x, y+h, z+b,
                       x+l, y+h, z,
                       x+l, y+h, z+b);
}

void fill_color(struct TRIANGLE_3D_FLOAT *list, int index, int size, unsigned char red, unsigned char green, unsigned char blue){
	int i;
	for (i=0; i<size; ++i){
		list[index + i].COLOR.RED = red;
		list[index + i].COLOR.GREEN = green;
		list[index + i].COLOR.BLUE = blue;
	}
}

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

int main(int argc, char *argv[]) {

    //Initial setup for SDL
    SDL_Event event;
    SDL_Renderer *renderer;
    SDL_Window *window;

    SDL_Init(SDL_INIT_VIDEO);
    SDL_CreateWindowAndRenderer(RADIUS*2, RADIUS*2, 0, &window, &renderer);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderClear(renderer);

    //Allocate depth and frame buffer
    struct COLOR_MIX *frame_buffer = malloc(RADIUS*RADIUS*4 * sizeof(struct COLOR_MIX));
	float *depth_buffer = malloc(RADIUS*RADIUS*4 * sizeof(float));

    //Allocate memory for triangle list and clear it
	struct TRIANGLE_3D_FLOAT *list = malloc(TOTAL_TRIANGLE_COUNT * sizeof(struct TRIANGLE_3D_FLOAT));
	memset((void *)list, 0, TOTAL_TRIANGLE_COUNT * sizeof(struct TRIANGLE_3D_FLOAT));
	int index = new_index(list, TOTAL_TRIANGLE_COUNT, TOTAL_TRIANGLE_COUNT);

	//Load texture from file
	char *texture_map = malloc(3 * TEXTURE_SIZE*TEXTURE_SIZE + 12);
	FILE *fPtr;
	if ((fPtr = fopen(input_texture_file, "rb")) == NULL){
	    printf("Error\n");
	    return 0;
	}
	fread(texture_map, 3 * TEXTURE_SIZE*TEXTURE_SIZE, 1, fPtr);
	fclose(fPtr);

	//Draw the box
	obj_skinned_box(list, texture_map, index, -50.0f, -150.0f, 200.0f, 100.0f, 50.0f, 50.0f);

    obj_road(list, 12, -100.0f, -150.0f, 150.0f, 250.0f, 200.0f);
    fill_color(list, 12, 1, 250, 0, 0);
    fill_color(list, 13, 1, 0, 250, 0);

    zbuffer(list, 14, depth_buffer, frame_buffer);
    draw_buffer(renderer, frame_buffer);
    while (1) {
        rotate_camera(list, index, 14, 0.01f);
        zbuffer(list, 14, depth_buffer, frame_buffer);
        draw_buffer(renderer, frame_buffer);
        SDL_Delay(100);
        if (SDL_PollEvent(&event) && event.type == SDL_QUIT)
            break;
    }

    delete_index(list, index, 14);

    free(texture_map);
	free(list);
	free(depth_buffer);
	free(frame_buffer);

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}

