#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdio.h>
#include <SDL.h>

//Length and breadth of square texture or image
//We read it in the form of binary ppm file
#define TEXTURE_LENGTH 100

//Radius of a circle inside the squared screen
#define RADIUS 250

//It is the tan(angle of view/2)
//This half tan value of the half of cone
#define FOV 1

//Floating point error while checking if point is inside a triangle or not
#define AREA_INACCURACY 1.0f

//The formula used for guessing pixel range wont work with z value with near to zero and negative values
//Use ray method for making triangles
#define Z_LIMIT 1.0f

//Finding maximum and minimum value from three numbers
#define MAXIMUM_VALUE(a, b, c) (((a) > (b)) ? (((c) > (a)) ? (c) : (a)) : (((c) > (b)) ? (c) : (b)))
#define MINIMUM_VALUE(a, b, c) (((a) < (b)) ? (((c) < (a)) ? (c) : (a)) : (((c) < (b)) ? (c) : (b)))

//Load the texture file for the box
char *input_texture_file = "texture0006.ppm";

//Stores a point
struct POINT_3D_FLOAT {
	float X;
	float Y;
	float Z;
};

//A color
struct COLOR_MIX {
	unsigned char RED;
	unsigned char GREEN;
	unsigned char BLUE;
};

//Stores the data of a 3D triangle in the game
//Stores pointer to texture
//Which number triangle an texture
//Color used if no texture
//List of three points for triangle
struct TRIANGLE_3D_FLOAT {
    char *TEXTURE;
    int IS_TEXTURE;
	struct COLOR_MIX COLOR;
	struct POINT_3D_FLOAT *TRIANGLE;
};

//Find the vector perpendicular to the plane which is formed by three points
void normal_vector(struct POINT_3D_FLOAT *p, struct POINT_3D_FLOAT *q, struct POINT_3D_FLOAT *r, float *a, float *b, float *c){
	*a = -(r->Y - p->Y) * (q->Z - p->Z) + (q->Y - p->Y) * (r->Z - p->Z);
	*b = -(q->X - p->X) * (r->Z - p->Z) + (r->X - p->X) * (q->Z - p->Z);
	*c = -(r->X - p->X) * (q->Y - p->Y) + (q->X - p->X) * (r->Y - p->Y);
}

//Area of a 3D triangle with coordinates given
float area_triangle(struct POINT_3D_FLOAT *p, struct POINT_3D_FLOAT *q, struct POINT_3D_FLOAT *r){
	float a, b, c;
	normal_vector(p, q, r, &a, &b, &c);
	float result = 0.5 * sqrt(a*a + b*b + c*c);
	return result;
}

//Check if a point is inside a triangle
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

//Draw different triangle and make image
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

	//Clear the frame buffer with white background color
	//Clear the depth buffer with maximum distance
	for (i=0; i<(RADIUS*RADIUS*4); ++i){
	    depth_buffer[i] = 100000.0f;
		frame_buffer[i].RED = 255;
		frame_buffer[i].GREEN = 255;
		frame_buffer[i].BLUE = 255;
	}

    //For all triangles in the list
	for (i=0; i<list_length; ++i){
	    struct POINT_3D_FLOAT *p = list[i].TRIANGLE;
        //If the current address does not have a triangle go to next
	    if (p == NULL) continue;
	    //If whole of the triangle is behind the camera ignore it
	    if (p[0].Z <= 0.0f && p[1].Z <= 0.0f && p[2].Z <= 0.0f){
			continue;
		}

		//By default go through all the pixels
	    x_max = (RADIUS*2) - 1;
	    x_min = 0;
	    y_max = (RADIUS*2) - 1;
	    y_min = 0;

        //If all the pixels of the triangle are in front of the camera
	    if (p[0].Z > Z_LIMIT && p[1].Z > Z_LIMIT && p[2].Z > Z_LIMIT){
		    float x_0, x_1, x_2, y_0, y_1, y_2;
		    //Use an mathematical technique to project the points of the triangle in 2D
		    x_0 = RADIUS + (RADIUS * FOV * p[0].X)/p[0].Z;
		    x_1 = RADIUS + (RADIUS * FOV * p[1].X)/p[1].Z;
		    x_2 = RADIUS + (RADIUS * FOV * p[2].X)/p[2].Z;
		    y_0 = RADIUS - (RADIUS * FOV * p[0].Y)/p[0].Z;
		    y_1 = RADIUS - (RADIUS * FOV * p[1].Y)/p[1].Z;
		    y_2 = RADIUS - (RADIUS * FOV * p[2].Y)/p[2].Z;

		    //See from which pixel triangle start and end where
		    x_max = (int) round(MAXIMUM_VALUE(x_0, x_1, x_2));
		    x_min = (int) round(MINIMUM_VALUE(x_0, x_1, x_2));
	    	y_max = (int) round(MAXIMUM_VALUE(y_0, y_1, y_2));
    		y_min = (int) round(MINIMUM_VALUE(y_0, y_1, y_2));

    		//If the range of pixel outside the screen then bring it in
    		//Sometimes if the triangle is outside all the pixels may be ignored
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
                //Draw normal to the plane and find the coefficient of the plane
				normal_vector(p, p + 1, p + 2, &a, &b, &c);
				d = a*p[0].X + b*p[0].Y + c*p[0].Z;
                //Draw a ray from the pixels to the plane
                //The pixels are converted to the form where center is the origin
				s.X = d/(((b*(RADIUS-k) + c*RADIUS*FOV)/(j-RADIUS))+a);
				s.Y = d/(((a*(j-RADIUS) + c*RADIUS*FOV)/(RADIUS-k))+b);
				s.Z = d/(((a*(j-RADIUS) + b*(RADIUS-k))/(RADIUS * FOV)) + c);

                //If the intersection point is inside the triangle
				if (inside_triangle(p, &s)){

                    //If points visible
					if (depth_buffer[j*RADIUS*2 + k] > s.Z){
					    //If there is no texture only color then use it
						if (list[i].IS_TEXTURE == 0){
                            memcpy(frame_buffer + (j*RADIUS*2 + k), &(list[i].COLOR), sizeof(struct COLOR_MIX));
						}
                        //Else if texture is there
						else {
                            //rect con will change the texture point as of the previous triangle
                            //if we are looking at the second triangle of the rectangle
							rect_con = list[i].IS_TEXTURE - 1;
						    u = list[i-rect_con].TRIANGLE;
						    v = (list[i-rect_con].TRIANGLE) + 1;
                            //Make vector of one side of the triangle
						    uv.X = v->X - u->X;
    						uv.Y = v->Y - u->Y;
    						uv.Z = v->Z - u->Z;
    						//Make vector from origin point to the intersection point of the ray
    						us.X = s.X - u->X;
    						us.Y = s.Y - u->Y;
    						us.Z = s.Z - u->Z;

    						float m_us = us.X * us.X + us.Y * us.Y + us.Z * us.Z;
    						float m_uv = uv.X * uv.X + uv.Y * uv.Y + uv.Z * uv.Z;
    						float uv_dot_us = uv.X * us.X + uv.Y * us.Y + uv.Z * us.Z;
    						//Remove less than zero value in squareroots
    						if (m_us <= 0) continue;
    						if (m_uv <= 0) continue;
    						if (uv_dot_us <= 0.0) uv_dot_us = 0.0f;
    						//Use a mathematical technique to convert 3D coordinates in the vertice of rectangle
    						//To convert to texture location
    						float h = sqrt(m_us);
						    float cos_theta = (uv_dot_us)/(h * (sqrt(m_uv)));
						    float sin_theta = sqrt(1 - (cos_theta * cos_theta));

						    //Make the texture repeat over time
						    int x_sc = (int)(sin_theta*h);
						    x_sc = x_sc % TEXTURE_LENGTH;

						    int y_sc = (int)(cos_theta*h);
						    y_sc = y_sc % TEXTURE_LENGTH;

						    //Ignore the heading in the ppm file and put pixel to the frame buffer
						    frame_buffer[j*RADIUS*2 + k].RED = list[i-rect_con].TEXTURE[12+y_sc*TEXTURE_LENGTH*3 + x_sc*3];
						    frame_buffer[j*RADIUS*2 + k].GREEN = list[i-rect_con].TEXTURE[12+y_sc*TEXTURE_LENGTH*3 + x_sc*3 + 1];
						    frame_buffer[j*RADIUS*2 + k].BLUE = list[i-rect_con].TEXTURE[12+y_sc*TEXTURE_LENGTH*3 + x_sc*3 + 2];

						}

                        //Store the depth or the value of z
						depth_buffer[j*RADIUS*2 + k] = s.Z;

					}
				}
			}
		}
	}
}

//Rotate the camera by rotating all the points in the list
void rotate_camera(struct TRIANGLE_3D_FLOAT *list, int index, int size, float angle){
	int i;
	int j;
	float x;
	for (i=0; i<size; ++i){
        for (j=0; j<3; ++j){
            //Rotate the point using 2D rotation matrix again a mathematical technique
            x = list[index + i].TRIANGLE[j].X*cos(angle)-list[index + i].TRIANGLE[j].Z*sin(angle);
            list[index + i].TRIANGLE[j].Z = list[index + i].TRIANGLE[j].X*sin(angle)+list[index + i].TRIANGLE[j].Z*cos(angle);
            list[index + i].TRIANGLE[j].X = x;
        }
	}
}

//Allocate memory to save location of triangle
int new_index(struct TRIANGLE_3D_FLOAT *list, int size, int limit){
	int i, j;
	int streak=0;
	for (i=0; i<limit; ++i){
        //If empty spaces found of the certain size store inside it
		if (list[i].TRIANGLE == NULL){
			++streak;
		} else {
			streak = 0;
		}
		if (streak == size){
			for (j=i-size+1; j<=i; ++j){
			    //Allocate memory and fill with zero
				list[j].TRIANGLE = calloc(3, sizeof(struct POINT_3D_FLOAT));
				memset(list[j].TRIANGLE, 0, 3 * sizeof(struct POINT_3D_FLOAT));
			}
			return i-size+1;
		}
	}
	return -1;
}

//Remove the memory
void delete_index(struct TRIANGLE_3D_FLOAT *list, int index, int size){
	int i;
	for (i=0; i<size; ++i){
		free(list[index + i].TRIANGLE);
		list[index + i].TRIANGLE = NULL;
	}
}

//A simple rectangle with a texture with four points
void obj_rectangle_0001(struct TRIANGLE_3D_FLOAT *list, char *texture, int index, struct POINT_3D_FLOAT *point){
    list[index].TEXTURE = texture;
    list[index].IS_TEXTURE = 1;
    list[index+1].IS_TEXTURE = 2;
    list[index+1].TEXTURE = NULL;
    memcpy(list[index].TRIANGLE, point, 3 * sizeof(struct POINT_3D_FLOAT));
    memcpy(list[index+1].TRIANGLE, point+1, 3 * sizeof(struct POINT_3D_FLOAT));
}

//Draw rectangle with four points
//First triangle 0->1->2
//Second triangle 1->2->3
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

//Draw textured rectangle with origin and dimensions
void obj_rectangle_0003(struct TRIANGLE_3D_FLOAT *list, int index, float x, float y, float z, float l, float b){
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

//Draw a wall with textured rectangles
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

//Draw cuboid with plain colors
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

	//Opposite faces
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

//Fill the same color in a triangle
void fill_color(struct TRIANGLE_3D_FLOAT *list, int index, int size, unsigned char red, unsigned char green, unsigned char blue){
	int i;
	for (i=0; i<size; ++i){
		list[index + i].COLOR.RED = red;
		list[index + i].COLOR.GREEN = green;
		list[index + i].COLOR.BLUE = blue;
	}
}

//Fill colors alternately in a cuboid
void fill_color_cuboid_0001(struct TRIANGLE_3D_FLOAT *list, int index){
	fill_color(list, index+0, 2, 255, 0, 0);
	fill_color(list, index+6, 2, 255, 0, 0);
	fill_color(list, index+2, 2, 0, 255, 0);
	fill_color(list, index+8, 2, 0, 255, 0);
	fill_color(list, index+4, 2, 0, 0, 255);
	fill_color(list, index+10, 2, 0, 0, 255);
}

//Draw a wall with brick made with cuboids
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

//Make a 90 degree rotated version of wall
//Number of triangle which wall occupy
#define WALL_0002_SIZE(a, b) (12 * 2 * (b + 1) * a)
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

//Put buffer using SDL on the screen
void draw_buffer(SDL_Renderer *renderer, struct COLOR_MIX *frame_buffer){
	SDL_Texture *Tile = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, RADIUS*2, RADIUS*2);
	unsigned char *bytes;
    int pitch;
    SDL_LockTexture(Tile, NULL, (void **)&bytes, &pitch);
    int i, j;
    for (i=0; i<RADIUS*2; ++i){
    	for (j=0; j<RADIUS*2; ++j){
            //No alpha
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
    SDL_Event event;
    SDL_Renderer *renderer;
    SDL_Window *window;

    SDL_Init(SDL_INIT_VIDEO);
    SDL_CreateWindowAndRenderer(RADIUS*2, RADIUS*2, 0, &window, &renderer);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderClear(renderer);

    //Frame buffer and depth buffer
    struct COLOR_MIX *frame_buffer = malloc(RADIUS*RADIUS*4 * sizeof(struct COLOR_MIX));
	float *depth_buffer = malloc(RADIUS*RADIUS*4 * sizeof(float));

	//List of triangle locations
	struct TRIANGLE_3D_FLOAT *list = malloc(14 * sizeof(struct TRIANGLE_3D_FLOAT));
	memset((void *)list, 0, 14 * sizeof(struct TRIANGLE_3D_FLOAT));

	int index = new_index(list, 14, 14);

	char *texture_map = malloc(3*TEXTURE_LENGTH*TEXTURE_LENGTH + 12);
	FILE *fPtr;

	if ((fPtr = fopen(input_texture_file, "rb")) == NULL){
	    printf("Error\n");
	    return 0;
	}
	fread(texture_map, TEXTURE_LENGTH*TEXTURE_LENGTH*3+12, 1, fPtr);
	fclose(fPtr);

	//Wall with orange fruit texture
	obj_wall_0003(list, texture_map, index, -50.0f, -150.0f, 200.0f, 100.0f, 50.0f, 50.0f);

	//Draw platform with red and green
    obj_rectangle_0003(list, 12, -100.0f, -150.0f, 150.0f, 250.0f, 200.0f);
    fill_color(list, 12, 1, 250, 0, 0);
    fill_color(list, 13, 1, 0, 250, 0);

    //Z buffer and draw it
    zbuffer(list, 14, depth_buffer, frame_buffer);
    draw_buffer(renderer, frame_buffer);

    while (1) {
        //Keep rotating the camera bit by bit
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

