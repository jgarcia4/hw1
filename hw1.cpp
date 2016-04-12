//Author: Jose Garcia
//Program: cs335 Spring 2016 hw1
//Purpose: This program demonstrates the waterfall model.

#include <iostream>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <cmath>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <GL/glx.h>
#include <GL/glut.h>

/*extern "C" {
    	#include "fonts.h"
}*/

#define WINDOW_WIDTH  500
#define WINDOW_HEIGHT 360

#define MAX_PARTICLES 1000
#define GRAVITY 0.5

//X Windows variables
Display *dpy;
Window win;
GLXContext glc;

//Structures

struct Vec {
	float x, y, z;
};

struct Shape {
	float width, height;
	float radius;
	Vec center;
};

struct Particle {
	Shape s;
	Vec velocity;
};

struct Game {
	Shape box;
	Particle particle[MAX_PARTICLES];
	int n;
};

//Function prototypes
void initXWindows(void);
void init_opengl(void);
void cleanupXWindows(void);
void check_mouse(XEvent *e, Game *game);
int check_keys(XEvent *e, Game *game);
void movement(Game *game);
void render(Game *game);
//void draw_text(const char *, int, int, int);


int main(int argc, char **argv)
{
	glutInit(&argc, argv);
	
	int done=0;
	srand(time(NULL));
	initXWindows();
	init_opengl();
	//declare game object
	Game game;
	game.n=MAX_PARTICLES;

	//declare a box shape
	game.box.width = 70;
	game.box.height = 10;
	game.box.center.x = -50 + 5*65;
	game.box.center.y = 500 - 5*60;

	//start animation
	while(!done) {
		while(XPending(dpy)) {
			XEvent e;
			XNextEvent(dpy, &e);
			check_mouse(&e, &game);
			done = check_keys(&e, &game);
		}
		movement(&game);
		render(&game);
		glXSwapBuffers(dpy, win);
	}
	cleanupXWindows();
	return 0;
}

/*void draw_text(const char *text, int length, int x, int y)  
{
        glMatrixMode(GL_PROJECTION);
        double *matrix = new double[16];
        glGetDoublev(GL_PROJECTION_MATRIX, matrix);
        glLoadIdentity();
        glOrtho(0, 500, 0, 360, -5, 5); 
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        glPushMatrix();
        glLoadIdentity();
        glRasterPos2i(x, y); 
        for (int i = 0; i < length; i++) 
                glutBitmapCharacter(GLUT_BITMAP_9_BY_15, (int)text[i]);
        glPopMatrix();
        glMatrixMode(GL_PROJECTION);
        glLoadMatrixd(matrix);
        glMatrixMode(GL_MODELVIEW);    
}*/

void set_title(void)
{
	//Set the window title bar.
	XMapWindow(dpy, win);
	XStoreName(dpy, win, "Waterfall");
}

void cleanupXWindows(void) {
	//do not change
	XDestroyWindow(dpy, win);
	XCloseDisplay(dpy);
}

void initXWindows(void) {
	//do not change
	GLint att[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None };
	int w=WINDOW_WIDTH, h=WINDOW_HEIGHT;
	dpy = XOpenDisplay(NULL);
	if (dpy == NULL) {
		std::cout << "\n\tcannot connect to X server\n" << std::endl;
		exit(EXIT_FAILURE);
	}
	Window root = DefaultRootWindow(dpy);
	XVisualInfo *vi = glXChooseVisual(dpy, 0, att);
	if(vi == NULL) {
		std::cout << "\n\tno appropriate visual found\n" << std::endl;
		exit(EXIT_FAILURE);
	} 
	Colormap cmap = XCreateColormap(dpy, root, vi->visual, AllocNone);
	XSetWindowAttributes swa;
	swa.colormap = cmap;
	swa.event_mask = ExposureMask | KeyPressMask | KeyReleaseMask |
							ButtonPress | ButtonReleaseMask |
							PointerMotionMask |
							StructureNotifyMask | SubstructureNotifyMask;
	win = XCreateWindow(dpy, root, 0, 0, w, h, 0, vi->depth,
					InputOutput, vi->visual, CWColormap | CWEventMask, &swa);
	set_title();
	glc = glXCreateContext(dpy, vi, NULL, GL_TRUE);
	glXMakeCurrent(dpy, win, glc);
}

void init_opengl(void)
{
	//OpenGL initialization
	glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
	//Initialize matrices
	glMatrixMode(GL_PROJECTION); glLoadIdentity();
	glMatrixMode(GL_MODELVIEW); glLoadIdentity();
	//Set 2D mode (no perspective)
	glOrtho(0, WINDOW_WIDTH, 0, WINDOW_HEIGHT, -1, 1);
	//Set the screen background color
	glClearColor(0.1, 0.1, 0.1, 1.0);
}

void makeParticle(Game *game, int x, int y) {
	if (game->n >= MAX_PARTICLES)
		return;
	//std::cout << "makeParticle()" << x << " " << y << std::endl;
	//position of particle
	Particle *p = &game->particle[game->n];
	p->s.center.x = x;
	p->s.center.y = y;
	p->velocity.y = -4.0;
	p->velocity.x =  0.5;
	game->n++;
}

void check_mouse(XEvent *e, Game *game)
{
	static int savex = 0;
	static int savey = 0;
	static int n = 0;

	if (e->type == ButtonRelease) {
		return;
	}
	if (e->type == ButtonPress) {
		if (e->xbutton.button==1) {
			//Left button was pressed
			int y = WINDOW_HEIGHT - e->xbutton.y;
			makeParticle(game, e->xbutton.x, y);
			return;
		}
		if (e->xbutton.button==3) {
			//Right button was pressed
			return;
		}
	}
	//Did the mouse move?
	if (savex != e->xbutton.x || savey != e->xbutton.y) {
		savex = e->xbutton.x;
		savey = e->xbutton.y;
		if (++n < 10)
			return;
			
		//int y = WINDOW_HEIGHT - e->xbutton.y;
		//makeParticle(game, e->xbutton.x, y);


	}
}

int check_keys(XEvent *e, Game *game)
{
	//Was there input from the keyboard?
	if (e->type == KeyPress) {
		int key = XLookupKeysym(&e->xkey, 0);
		if (key == XK_Escape) {
			return 1;
		}
		//You may check other keys here.
		
		// Was b key pressed? turn on stream  
		if (key == XK_b) {
			int y = WINDOW_HEIGHT - e->xbutton.y;
			makeParticle(game, e->xbutton.x, y);
			if (key == XK_Escape) {
				return 1;
			}
		}

	}
	return 0;
}

void movement(Game *game)
{
	Particle *p;

	if (game->n <= 0)
		return;

	for (int i=0; i<game->n; i++) {
		p = &game->particle[i];
		p->s.center.x += p->velocity.x;
		p->s.center.y += p->velocity.y;

		//gravity
		p->velocity.y -= GRAVITY;

		//check for collision with shapes...
		Shape *s;
		s = &game->box;
		if ((p->s.center.y >= s->center.y + 120 - (s->height) &&
		    p->s.center.y <= s->center.y + 120 + (s->height) &&
		    p->s.center.x >= s->center.x - 200 - (s->width) &&
		    p->s.center.x <= s->center.x - 200 + (s->width)) ||
		
			(p->s.center.y >= s->center.y + 90 - (s->height) &&
		    p->s.center.y <= s->center.y + 90 + (s->height) &&
		    p->s.center.x >= s->center.x - 150 -(s->width) &&
		    p->s.center.x <= s->center.x - 150 + (s->width)) ||
		
			(p->s.center.y >= s->center.y + 60 - (s->height) &&
		    p->s.center.y <= s->center.y + 60 + (s->height) &&
		    p->s.center.x >= s->center.x - 100 - (s->width) &&
		    p->s.center.x <= s->center.x - 100 + (s->width)) ||
		
			(p->s.center.y >= s->center.y + 30 - (s->height) &&
		    p->s.center.y <= s->center.y + 30 + (s->height) &&
		    p->s.center.x >= s->center.x - 50 - (s->width) &&
		    p->s.center.x <= s->center.x - 50 + (s->width)) ||
		
			(p->s.center.y >= s->center.y - (s->height) &&
		    p->s.center.y <= s->center.y + (s->height) &&
		    p->s.center.x >= s->center.x - (s->width) &&
		    p->s.center.x <= s->center.x + (s->width)) ){
			p->velocity.y *= -0.1;
		}

		//check for off-screen
		if (p->s.center.y < 0.0) {
			std::cout << "off screen" << std::endl;
			game->particle[i] = game->particle[game->n-1];
			game->n--;
		}
	}
}

void render(Game *game)
{
	float w, h, w2, h2, w3, h3, w4, h4, w5, h5;
	glClear(GL_COLOR_BUFFER_BIT);
	//Draw shapes...

	//draw box
	Shape *s;
	glColor3ub(90,140,90);
	s = &game->box;
	glPushMatrix();
	glTranslatef(s->center.x, s->center.y, s->center.z);
	w = s->width;
	h = s->height;
	glBegin(GL_QUADS);
		glVertex2i(-w,-h);
		glVertex2i(-w, h);
		glVertex2i( w, h);
		glVertex2i( w,-h);
	glEnd();
	glPopMatrix();
	
	Shape *s2;
	glColor3ub(90,140,90);
	s2 = &game->box;
	glPushMatrix();
	glTranslatef(s2->center.x - 50, s2->center.y + 30, s2->center.z);
	w2 = s2->width;
	h2 = s2->height;
	glBegin(GL_QUADS);
		glVertex2i(-w2,-h2);
		glVertex2i(-w2, h2);
		glVertex2i( w2, h2);
		glVertex2i( w2,-h2);
	glEnd();
	glPopMatrix();
	
	Shape *s3;
	glColor3ub(90,140,90);
	s3 = &game->box;
	glPushMatrix();
	glTranslatef(s3->center.x - 100, s3->center.y + 60, s3->center.z);
	w3 = s3->width;
	h3 = s3->height;
	glBegin(GL_QUADS);
		glVertex2i(-w3,-h3);
		glVertex2i(-w3, h3);
		glVertex2i( w3, h3);
		glVertex2i( w3,-h3);
	glEnd();
	glPopMatrix();
	
	Shape *s4;
	glColor3ub(90,140,90);
	s4 = &game->box;
	glPushMatrix();
	glTranslatef(s4->center.x - 150, s4->center.y + 90, s2->center.z);
	w4 = s4->width;
	h4 = s4->height;
	glBegin(GL_QUADS);
		glVertex2i(-w4,-h4);
		glVertex2i(-w4, h4);
		glVertex2i( w4, h4);
		glVertex2i( w4,-h4);
	glEnd();
	glPopMatrix();
	
	Shape *s5;
	glColor3ub(90,140,90);
	s5 = &game->box;
	glPushMatrix();
	glTranslatef(s5->center.x - 200, s5->center.y + 120, s5->center.z);
	w5 = s5->width;
	h5 = s5->height;
	glBegin(GL_QUADS);
		glVertex2i(-w5,-h5);
		glVertex2i(-w5, h5);
		glVertex2i( w5, h5);
		glVertex2i( w5,-h5);
	glEnd();
	glPopMatrix();

	//draw all particles here
	glPushMatrix();
	glColor3ub(150,160,220);
	for (int i=0; i<game->n; i++) {
		Vec *c = &game->particle[i].s.center;
		w = 2;
		h = 2;
		glBegin(GL_QUADS);
			glVertex2i(c->x-w, c->y-h);
			glVertex2i(c->x-w, c->y+h);
			glVertex2i(c->x+w, c->y+h);
			glVertex2i(c->x+w, c->y-h);
		glEnd();
		glPopMatrix();
	}
	
	//draw text
	/*std::string text;

	glPushMatrix();
        text = "Hold 'b' for water";
        glColor3f(1, 1, 1);
        draw_text(text.data(), text.size(), 575, WINDOW_HEIGHT - 20);
        glPopMatrix();

	glPushMatrix();
        text = "Press ESC to exit";
        glColor3f(1, 1, 1);
        draw_text(text.data(), text.size(), 575, WINDOW_HEIGHT - 35);
        glPopMatrix();
	
	glPushMatrix();
        text = "WATERFALL MODEL";
        glColor3f(1, 1, 1);
        draw_text(text.data(), text.size(), 10, WINDOW_HEIGHT - 20);
        glPopMatrix();
	 
	glPushMatrix();
        text = "Requirements";
        glColor3f(0, 1, 1);
        draw_text(text.data(), text.size(), 120, 495);
        glPopMatrix();
	 
	glPushMatrix();
        text = "Design";
        glColor3f(0, 1, 1);
        draw_text(text.data(), text.size(), 230, 420);
        glPopMatrix();
	 
	glPushMatrix();
        text = "Coding";
        glColor3f(0, 1, 1);
        draw_text(text.data(), text.size(), 305, 345);
        glPopMatrix();
	 
	glPushMatrix();
        text = "Testing";
        glColor3f(0, 1, 1);
        draw_text(text.data(), text.size(), 345, 270);
        glPopMatrix();
	
	glPushMatrix();
        text = "Maintenance";
        glColor3f(0, 1, 1);
        draw_text(text.data(), text.size(), 405, 195);
        glPopMatrix();*/
}



