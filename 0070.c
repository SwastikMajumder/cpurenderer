#include <stdlib.h>
#include <string.h>
#include <math.h>

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
	struct POINT_3D_FLOAT s;
	struct POINT_3D_FLOAT u;
	struct POINT_3D_FLOAT v;
	char texture_map[3*TS*TS + 13];
	FILE *fPtr;
	if ((fPtr = fopen("texture0005.ppm", "rb")) == NULL){
	    printf("Error\n");
	    return;
	}
	fread(texture_map, sizeof(texture_map), 1, fPtr);
	//for (i=0; i<25*25*3; ++i){
	//	printf("%d ", texture_map[i]);
	//}
	//fscanf(fPtr, "%[^ ]", texture_map);
	fclose(fPtr);
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
						float tmp = a*a +b*b + c*c;
						u.X = (d * a)/tmp;
						u.Y = (d * b)/tmp;
						u.Z = (d * c)/tmp;
						v.X = ((-c +d)*a)/tmp;
						v.Y = ((-c +d)*b)/tmp;
						v.Z = (d * c  + a*a + b*b)/tmp;
						struct POINT_3D_FLOAT tmp_2;
						struct POINT_3D_FLOAT tmp_3;
						tmp_2.X = v.X - u.X;
						tmp_2.Y = v.Y - u.Y;
						tmp_2.Z = v.Z - u.Z;
						tmp_3.X = s.X - u.X;
						tmp_3.Y = s.Y - u.Y;
						tmp_3.Z = s.Z - u.Z;
						float h = sqrt(tmp_3.X * tmp_3.X + tmp_3.Y * tmp_3.Y + tmp_3.Z * tmp_3.Z);
						float cos_theta = (tmp_2.X * tmp_3.X + tmp_2.Y * tmp_3.Y + tmp_2.Z * tmp_3.Z)/(h * (sqrt(tmp_2.X * tmp_2.X + tmp_2.Y * tmp_2.Y + tmp_2.Z * tmp_2.Z)));
						float sin_theta = sqrt(1 - (cos_theta * cos_theta));
						
						if (cos_theta > 0.0f) cos_theta = -cos_theta;
						int x_sc = (int)(sin_theta*h);
						x_sc = x_sc % TS;
						
						int y_sc = (int)(cos_theta*h);
						y_sc = y_sc % TS;
						/*
						if (y_dash >= 0){
							x_sc = (int)x_dash % 25;
						    y_sc =(int)y_dash % 25;
						} else {
							x_sc = 25 - ((int)x_dash % 25);
						    y_dash = -y_dash;
						    y_sc = 25 - ((int)y_dash % 25);
						}*/
						frame_buffer[j*RADIUS*2 + k].RED = texture_map[13+x_sc*TS*3 + y_sc*3];
						frame_buffer[j*RADIUS*2 + k].GREEN = texture_map[13+x_sc*TS*3 + y_sc*3 + 1];
						frame_buffer[j*RADIUS*2 + k].BLUE = texture_map[13+x_sc*TS*3 + y_sc*3 + 2];
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
	fPtr = fopen("file10.txt", "w");
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
	struct TRIANGLE_3D_FLOAT *list = malloc(WALL_0002_SIZE(3, 2) * sizeof(struct TRIANGLE_3D_FLOAT));
	
	int index = new_index(list, WALL_0002_SIZE(3, 2), WALL_0002_SIZE(3, 2));
	
	struct POINT_3D_FLOAT location = {.X = -200.0f, .Y = 0.0f, .Z = 250.0f };
	
	obj_wall_0002(list, index, &location, 3, 2, 255, 0, 0);
	
	rotate_camera(list, index, WALL_0002_SIZE(3, 2), 0.0f, (10 * M_PI)/180, 0.0f);
	
	zbuffer(list, WALL_0002_SIZE(3, 2), depth_buffer, frame_buffer);
	draw_buffer(frame_buffer);
	
	delete_index(list, index, WALL_0002_SIZE(1, 1));
/*	struct TRIANGLE_3D_FLOAT *list = malloc(2 * sizeof(struct TRIANGLE_3D_FLOAT));
	//int index = new_index(list, 2, 2);
	struct POINT_3D_FLOAT t_1[] =
	{
		[0] = {.X = -100.0f, .Y = -50.0f, .Z = 200.0f},
		[1] = {.X = -100.0f, .Y = -50.0f, .Z = 150.0f},
		[2] = {.X = 100.0f, .Y = -50.0f, .Z = 200.0}
	};
	struct POINT_3D_FLOAT t_2[] =
	{
		[0] = {.X = 100.0f, .Y = -50.0f, .Z = 150.0f},
		[1] = {.X = -100.0f, .Y = -50.0f, .Z = 150.0f},
		[2] = {.X = 100.0f, .Y = -50.0f, .Z = 200.0f}
	};
	list[0].TRIANGLE = t_1;
	list[1].TRIANGLE = t_2;
	list[0].COLOR.RED = 0;
	list[0].COLOR.GREEN = 0;
	list[0].COLOR.BLUE = 255;
	list[1].COLOR.RED = 0;
	list[1].COLOR.GREEN = 255;
	list[1].COLOR.BLUE = 0;

	zbuffer(list, 2, depth_buffer, frame_buffer);
	draw_buffer(frame_buffer);
*/
	free(list);
	free(depth_buffer);
	free(frame_buffer);
	
   return EXIT_SUCCESS;
}
