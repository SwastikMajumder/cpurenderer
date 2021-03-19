#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdio.h>

#define TS 100

#define WINDOW_WIDTH 1000

#define RADIUS 250
#define FOV 1

#define AREA_INACCURACY 1.0f

#define Z_LIMIT 1.0f

#define WALL_0002_SIZE(a, b) (12 * 2 * (b + 1) * a)

#define MAXIMUM_VALUE(a, b, c) (((a) > (b)) ? (((c) > (a)) ? (c) : (a)) : (((c) > (b)) ? (c) : (b)))
#define MINIMUM_VALUE(a, b, c) (((a) < (b)) ? (((c) < (a)) ? (c) : (a)) : (((c) < (b)) ? (c) : (b)))

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
	for (i=0; i<(RADIUS*RADIUS*4); ++i){
	    depth_buffer[i] = 100000.0f;
		frame_buffer[i].RED = 255;
		frame_buffer[i].GREEN = 255;
		frame_buffer[i].BLUE = 255;
	}

	for (i=0; i<list_length; ++i){
	    struct POINT_3D_FLOAT *p = list[i].TRIANGLE;
	    if (p == NULL) continue;
	    if (p[0].Z <= 0.0f && p[1].Z <= 0.0f && p[2].Z <= 0.0f){
			continue;
		}
	    x_max = (RADIUS*2) - 1;
	    x_min = 0;
	    y_max = (RADIUS*2) - 1;
	    y_min = 0;
	   // printf("%3.3f ", MINIMUM_VALUE(0.3, 1.5, -1.3));
	    if (p[0].Z > Z_LIMIT && p[1].Z > Z_LIMIT && p[2].Z > Z_LIMIT){
		    float x_0, x_1, x_2, y_0, y_1, y_2;
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
	//printf("%d %d %d %d ", x_max, x_min, y_max, y_min);
    	for(j=x_min; j<=x_max; ++j){
    	    for (k=y_min; k<=y_max; ++k){
				normal_vector(p, p + 1, p + 2, &a, &b, &c);
				d = a*p[0].X + b*p[0].Y + c*p[0].Z;
				s.X = d/(((b*(RADIUS-k) + c*RADIUS*FOV)/(j-RADIUS))+a);
				s.Y = d/(((a*(j-RADIUS) + c*RADIUS*FOV)/(RADIUS-k))+b);
				s.Z = d/(((a*(j-RADIUS) + b*(RADIUS-k))/(RADIUS * FOV)) + c);
				//printf("msg ");

				if (inside_triangle(p, &s)){
					if (depth_buffer[j*RADIUS*2 + k] > s.Z){

						//memcpy(frame_buffer + (j*RADIUS*2 + k), &(list[i].COLOR), sizeof(struct COLOR_MIX));
						if (list[i].IS_TEXTURE){
							rect_con = list[i].IS_TEXTURE - 1;
						    u = list[i-rect_con].TRIANGLE;
						    v = (list[i-rect_con].TRIANGLE) + 1;
						    uv.X = v->X - u->X;
    						uv.Y = v->Y - u->Y;
    						uv.Z = v->Z - u->Z;
    						us.X = s.X - u->X;
    						us.Y = s.Y - u->Y;
    						us.Z = s.Z - u->Z;
    						float h = sqrt(us.X * us.X + us.Y * us.Y + us.Z * us.Z);
						    float cos_theta = (uv.X * us.X + uv.Y * us.Y + uv.Z * us.Z)/(h * (sqrt(uv.X * uv.X + uv.Y * uv.Y + uv.Z * uv.Z)));
						    float sin_theta = sqrt(1 - (cos_theta * cos_theta));

						    int x_sc = (int)(sin_theta*h);
						    x_sc = x_sc % TS;

						    int y_sc = (int)(cos_theta*h);
						    y_sc = y_sc % TS;

						    frame_buffer[j*RADIUS*2 + k].RED = list[i-rect_con].TEXTURE[13+y_sc*TS*3 + x_sc*3];
						    frame_buffer[j*RADIUS*2 + k].GREEN = list[i-rect_con].TEXTURE[13+y_sc*TS*3 + x_sc*3 + 1];
						    frame_buffer[j*RADIUS*2 + k].BLUE = list[i-rect_con].TEXTURE[13+y_sc*TS*3 + x_sc*3 + 2];

						}
						depth_buffer[j*RADIUS*2 + k] = s.Z;

					}
				}
			}
		}
	}
}

int ascii_decimal(char *str, int n){
	int i;
	int ary[3];
	for (i=0; i<3; ++i){
		ary[i] = n%10;
		n -= n%10;
		n /= 10;
	}
	if (ary[2] == 0){
		if (ary[1] == 0){
			str[0] = ary[0] + '0';
			return 1;
		} else {
			str[1] = ary[0] + '0';
			str[0] = ary[1] + '0';
			return 2;
		}
	} else {
		str[2] = ary[0] + '0';
		str[1] = ary[1] + '0';
		str[0] = ary[2] + '0';
		return 3;
	}
}

void draw_buffer(struct COLOR_MIX *frame_buffer){
    unsigned long int  i, j, index=0;
	char *data = malloc(RADIUS*RADIUS*4*12);
	for (i=0; i<(RADIUS*2); ++i){
		for (j=0; j<(RADIUS*2); ++j){
			index += ascii_decimal(data + index,frame_buffer[j*RADIUS*2+i].RED);
			data[index++] = ' ';
			index += ascii_decimal(data + index,frame_buffer[j*RADIUS*2+i].GREEN);
			data[index++] = ' ';
			index += ascii_decimal(data + index,frame_buffer[j*RADIUS*2+i].BLUE);
			data[index++] = ' ';
		}
	}
	FILE *fPtr;
	fPtr = fopen("file37.txt", "w");
	if (fPtr == NULL){
		printf("Error!\n");
	}
	fputs(data, fPtr);
	fclose(fPtr);
	free(data);
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

void obj_rectangle_0001(struct TRIANGLE_3D_FLOAT *list, char *texture, int index, struct POINT_3D_FLOAT *point){
    list[index].TEXTURE = texture;
    list[index].IS_TEXTURE = 1;
    list[index+1].IS_TEXTURE = 2;
    list[index+1].TEXTURE = NULL;
    memcpy(list[index].TRIANGLE, point, 3 * sizeof(struct POINT_3D_FLOAT));
    memcpy(list[index+1].TRIANGLE, point+1, 3 * sizeof(struct POINT_3D_FLOAT));
}

void obj_rectangle_0002(struct TRIANGLE_3D_FLOAT *list, char *texture, int index,
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

void obj_wall_0003(struct TRIANGLE_3D_FLOAT *list, char *texture, int index, float x, float y, float z, float l, float b, float h){
    obj_rectangle_0002(list, texture, index,
                       x, y, z,
                       x, y+h, z,
                       x+l, y, z,
                       x+l, y+h, z);
    obj_rectangle_0002(list, texture, index+2,
                       x, y, z,
                       x, y+h, z,
                       x, y, z+b,
                       x, y+h, z+b);
    obj_rectangle_0002(list, texture, index+4,
                       x, y, z,
                       x, y, z+b,
                       x+l, y, z,
                       x+l, y, z+b);

    obj_rectangle_0002(list, texture, index+6,
                       x, y, z+b,
                       x, y+h, z+b,
                       x+l, y, z+b,
                       x+l, y+h, z+b);
    obj_rectangle_0002(list, texture, index+8,
                       x+l, y, z,
                       x+l, y+h, z,
                       x+l, y, z+b,
                       x+l, y+h, z+b);
    obj_rectangle_0002(list, texture, index+10,
                       x, y+h, z,
                       x, y+h, z+b,
                       x+l, y+h, z,
                       x+l, y+h, z+b);
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

int main() {

	struct COLOR_MIX *frame_buffer = malloc(RADIUS*RADIUS*4 * sizeof(struct COLOR_MIX));
	float *depth_buffer = malloc(RADIUS*RADIUS*4 * sizeof(float));

	struct TRIANGLE_3D_FLOAT *list = malloc(12 * sizeof(struct TRIANGLE_3D_FLOAT));
	memset((void *)list, 0, 12 * sizeof(struct TRIANGLE_3D_FLOAT));
	int index = new_index(list, 12, 12);
	char *texture_map = malloc(3*TS*TS + 13);
	FILE *fPtr;

	if ((fPtr = fopen("texture0007.ppm", "rb")) == NULL){
	    printf("Error\n");
	    return 0;
	}
	fread(texture_map, TS*TS*3+13, 1, fPtr);
	fclose(fPtr);

	/*
	int i;
	for (i=13; i<TS*TS*3+13; i+=3){
		printf("%d ",(int) texture_map[i]);
		printf("%d ",(int) texture_map[i+1]);
		printf("%d  ",(int) texture_map[i+1]);
	}
*/
/*
	struct POINT_3D_FLOAT rectangle[] =
	{
	    [0] = {.X = -200.0f, .Y = 0.0f, .Z = 250.0f},
	    [1] = {.X = -200.0f, .Y = 100.0f, .Z = 250.0f},
	    [2] = {.X = -100.0f, .Y = 0.0f, .Z = 250.0f},
	    [3] = {.X = -100.0f, .Y = 100.0f, .Z = 250.0f}
	}; */
	obj_wall_0003(list, texture_map, index, -200.0f, 0.0f, 250.0f, 150.0f, 50.0f, 100.0f);
	//printf("Bye");
	rotate_camera(list, index, 12, 0.0f, (10 * M_PI)/180, 0.0f);

	zbuffer(list, 12, depth_buffer, frame_buffer);
	draw_buffer(frame_buffer);

	delete_index(list, index, 12);

    free(texture_map);
	free(list);
	free(depth_buffer);
	free(frame_buffer);

   return EXIT_SUCCESS;
}
