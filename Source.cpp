#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_image.h>
#include <math.h>

const double PI = 3.1415926;

ALLEGRO_DISPLAY *display = NULL;
ALLEGRO_DISPLAY_MODE screen;
ALLEGRO_EVENT_QUEUE *event_queue = NULL;
ALLEGRO_TIMER *timer = NULL;
ALLEGRO_BITMAP *jim = NULL;
ALLEGRO_BITMAP *cursor = NULL;

const float FPS = 60.0;
bool endgame = false;
bool redraw = false;
enum {UP, DOWN, LEFT, RIGHT, A, D};
bool key[6];
float wall_distance[2000];


//Player Position
double px = 700;
double py = 600;
double angle = PI;



//The Level
int map1[25][10] = {
{1,1,1,1,1,1,1,1,1,1},
{1,0,1,0,0,1,0,1,0,1},
{1,0,0,0,0,1,0,0,0,1},
{1,1,0,1,0,1,0,1,0,1},
{1,0,0,1,0,0,0,1,0,1},
{1,0,1,1,1,1,1,1,0,1},
{1,0,1,0,0,0,0,0,0,1},
{1,0,1,0,0,0,0,0,0,1},
{1,0,1,0,0,0,0,0,0,1},
{1,0,1,0,0,0,0,0,0,1},
{1,0,1,0,0,0,0,0,0,1},
{1,0,1,0,0,0,0,0,0,1},
{1,0,1,0,0,0,0,0,0,1},
{1,0,1,0,0,0,0,0,0,1},
{1,0,1,0,0,0,0,0,0,1},
{1,0,1,0,0,0,0,0,0,1},
{1,0,1,0,0,0,0,0,0,1},
{1,0,1,0,0,0,0,0,0,1},
{1,0,1,0,0,0,0,0,0,1},
{1,0,1,0,0,0,0,0,0,1},
{1,0,1,0,0,0,0,0,0,1},
{1,0,1,0,0,0,1,1,0,1},
{1,0,1,0,0,0,1,1,0,1},
{1,0,0,0,0,0,0,0,0,1},
{1,1,1,1,1,1,1,1,1,1}};



//Scenery Class 2nd Attempt
class Scenery {
private:
	ALLEGRO_BITMAP *image;
	int width;
	int height;
	int x;
	int y;
	int vertical;
public:
	void set_image(char *filename) {
		image = al_load_bitmap(filename);
	}
	void set_width(int w) {
		width = w;
	}
	void set_height(int h) {
		height = h;
	}
	void set_x(int a) {
		x = a;
	}
	void set_y(int b) {
		y = b;
	}
	void set_vertical(int v) {
		vertical = v;
	}
	double distance() {
		return sqrt((x - px)*(x - px) + (y - py)*(y - py));
	}
	Scenery(char* filename, int w, int h, int a, int b, int v) {
		set_image(filename);
		set_width(w);
		set_height(h);
		set_x(a);
		set_y(b);
		set_vertical(v);
	}
};



//Sprite Stuff (TODO: refactor so sprite_z is used)
void draw_sprite(ALLEGRO_BITMAP *sprite, double sprite_x, double sprite_y, double hgt_at_inf, double hgt_adj_factor) {
	double sprite_distance = sqrt((sprite_x - px)*(sprite_x - px) + (sprite_y - py)*(sprite_y - py));
	double sprite_size = 10000/sprite_distance;
	double sprite_direction;
	if (sprite_y - py > 0) {
		sprite_direction = atan((sprite_x - px)/(sprite_y - py));
	}
	else {
		sprite_direction = atan((sprite_x - px)/(sprite_y - py)) + PI;
	}
	double sprite_vertical = hgt_at_inf - hgt_adj_factor/sprite_distance;
	double difference = sprite_direction - angle;
	if (sprite_y - py < 0 && angle < 0) {
		difference -= 2*PI;
	}
	double sprite_horizontal = screen.width/2 - sprite_size/2 + (screen.width)*(difference);
	al_draw_scaled_bitmap(sprite, 0, 0 , 200, 300, sprite_horizontal, sprite_vertical, sprite_size, sprite_size, 0);
}






//Drawing Functions
void draw_wall(int x, float distance) {
	int size = 16000/distance;
	int y1 = screen.height/2 - size/2;
	size = 10000/distance;
	int y2 = screen.height/2 + size/2;
	al_draw_line(x,y1,x,y1+3,al_map_rgb(0,0,0), 3);
	al_draw_line(x,y2,x,y2+3,al_map_rgb(0,0,0), 3);
}

// Draw Line from top of screen to where the ceiling meets the wall
void draw_ceiling(int x, float distance) {
	int min_height = 16000/distance;
	int y1 = screen.height/2 - min_height/2;
	al_draw_line(x,0,x,y1,al_map_rgb(20,80,20), 3);
}

//Draw from bottom of screen up to wall
void draw_floor(int x, float distance) {
	int max_height = 10000/distance;
	int y2 = screen.height/2 + max_height/2;
	al_draw_line(x,y2,x,screen.height,al_map_rgb(50,50,180), 3);
}
	

void draw_corner(int x, float distance) {
	int size = 16000/distance;
	int y1 = screen.height/2 - size/2;
	size = 10000/distance;
	int y2 = screen.height/2 + size/2;
	al_draw_line(x,y1,x,y2,al_map_rgb(250,250,250), 1);
}











int main(int argc, char **argv) {

	//Initialize
	srand(time(0));
	al_init();
	al_init_primitives_addon();
	al_init_image_addon();
	al_install_keyboard();


	//Make Display
	al_get_display_mode(al_get_num_display_modes() - 1, &screen);
	al_set_new_display_flags(ALLEGRO_FULLSCREEN);
	display = al_create_display(screen.width, screen.height);

	
	//Make Timer
	timer = al_create_timer(1.0/FPS);


	//Make Event Queue
	event_queue = al_create_event_queue();

	//Draw Sprites
	jim = al_load_bitmap("Jim.png");
	cursor = al_load_bitmap("Cursor.png");


	//Event Sources
	al_register_event_source(event_queue, al_get_display_event_source(display));
	al_register_event_source(event_queue, al_get_timer_event_source(timer));
	al_register_event_source(event_queue, al_get_keyboard_event_source());


	//Clear BG & Start
	al_set_target_bitmap(al_get_backbuffer(display));
	al_clear_to_color(al_map_rgb(0,0,0));
	al_flip_display();
	al_start_timer(timer);


	//Game Loop
	while(!endgame) {
		ALLEGRO_EVENT ev;
		al_wait_for_event(event_queue, &ev);


		//Calculate Positions
		if(ev.type == ALLEGRO_EVENT_TIMER) {
			redraw = true;

			double wall_x = px;
			double wall_y = py;

			if (key[UP]) {
				px += 3*sin(angle);
				py += 3*cos(angle);
			}
			if (key[DOWN]) {
				px -= 3*sin(angle);
				py -= 3*cos(angle);
			}
			if (key[A]) {
				angle -= .01;
				if (angle < -PI)
					angle += 2*PI;
			}
			if (key[D]) {
				angle += .01;
				if (angle > PI)
					angle -=2*PI;
			}
			if (key[LEFT]) {
				px -= cos(angle);
				py += sin(angle);
			}
			if (key[RIGHT]) {
				px += cos(angle);
				py -= sin(angle);
			}


			//Don't Go Through Walls
				if (map1[int(px/100)][int((py-10)/100)] != 0) {
					py = wall_y;
				}
				if (map1[int(px/100)][int((py+10)/100)] != 0) {
					py = wall_y;
				}
				if (map1[int((px-10)/100)][int(py/100)] != 0) {
					px = wall_x;
				}
				if (map1[int((px+10)/100)][int(py/100)] != 0) {
					px = wall_x;
				}
		}


		//Check Inputs
		else if(ev.type == ALLEGRO_EVENT_KEY_DOWN) {
			switch (ev.keyboard.keycode) {
			case ALLEGRO_KEY_UP:
				key[UP] = true;
				break;
			case ALLEGRO_KEY_DOWN:
				key[DOWN] = true;
				break;
			case ALLEGRO_KEY_LEFT:
				key[LEFT] = true;
				break;
			case ALLEGRO_KEY_RIGHT:
				key[RIGHT] = true;
				break;
			case ALLEGRO_KEY_A:
				key[A] = true;
				break;
			case ALLEGRO_KEY_D:
				key[D] = true;
				break;
			case ALLEGRO_KEY_ESCAPE:
				endgame = true;
				break;
			case ALLEGRO_KEY_ENTER:
				endgame = true;
				break;
			}
		}
		  
		else if(ev.type == ALLEGRO_EVENT_KEY_UP) {
			switch (ev.keyboard.keycode) {
			case ALLEGRO_KEY_UP:
				key[UP] = false;
				break;
			case ALLEGRO_KEY_DOWN:
				key[DOWN] = false;
				break;
			case ALLEGRO_KEY_LEFT:
				key[LEFT] = false;
				break;
			case ALLEGRO_KEY_RIGHT:
				key[RIGHT] = false;
				break;
			case ALLEGRO_KEY_A:
				key[A] = false;
				break;
			case ALLEGRO_KEY_D:
				key[D] = false;
				break;
			}
		}


		//Draw Frame
		if(redraw && al_is_event_queue_empty(event_queue)) {
			redraw = false;
			al_clear_to_color(al_map_rgb(150, 150, 100));

			for (double i = 0; i < screen.width; i +=3) {
				double x = px;
				double y = py;
				double d = 0;
				double ray = angle + .5*((i - screen.width/2)/(screen.width/2));
				while (map1[int(x/100)][int(y/100)] != 1) {
					if (d < 10) {
						d += .05;
						x += .2*sin(ray);
						y += .2*cos(ray);
					}
					else if (d < 30) {
						d += .1;
						x += .4*sin(ray);
						y += .4*cos(ray);
					}
					else {
						d += .25;
						x += sin(ray);
						y += cos(ray);
					}
				}
				draw_ceiling(i, d);
				draw_floor(i, d);
				draw_wall(i, d);
				wall_distance[int(i)] = sqrt((x - px)*(x - px) + (y - py)*(y - py));
				wall_distance[int(i) + 1] = wall_distance[int(i)];
				wall_distance[int(i) + 2] = wall_distance[int(i)];
			}
			draw_sprite(jim, 730, 600, 375, 25000);
			draw_sprite(cursor, 730, 800, 375, 25000);

			al_flip_display();
		}
	}


	
	al_destroy_timer(timer);
	al_destroy_display(display);
	al_destroy_event_queue(event_queue);
 
	return 0;
}