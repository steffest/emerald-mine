/* handle display, keyboard, joystick, and audio in emerald mine */

/*  bridges C to javascript calls */
/* this file: (c)2017 - Steffest */

extern char *global;

#include "proto.h"

#include <stdio.h>
#include <emscripten.h>

/* ---------------------------------------------------------------------- */
/* javascript functions declarations */

extern void js_initScreen(int width, int height);
extern void js_initKeyboard(void);
extern void js_drawMenu(void);
extern void js_drawMenuAnts(int left, int right, int top);
extern void js_createCanvas(char name, int width, int height);
extern void js_clearRect(int x, int y, int width, int height);
extern void js_menuClear(void);
extern void js_menuPrint(char str[26],int top);
extern int js_getKey(int keyCode);
extern void js_printScore(char str[40]);
extern void js_drawObject(int tile, int x, int y);
extern void js_blit(char source, int srcX, int srcY, int srcW, int srcH, char destination , int dstX, int dstY, int dstW, int dstH);
extern void js_fillRect(char color, int x, int y, int w, int h);
extern void js_drawGameScreen(int x, int y, int width, int height);
extern void js_setObjectPixelRGB(int r, int g, int b, int x, int y);
extern void js_playAudio(int index);
extern void js_quit(void);
extern void js_event(int event, int data);

/* ---------------------------------------------------------------------- */

/* create window, create palette */

static int scale_x = 2; /* default to double size because it looks better on most displays */
static int scale_y = 2;

static int center_dx, center_dy;

static unsigned char colourRed[COLOUR_MAX + 1];
static unsigned char colourGreen[COLOUR_MAX + 1];
static unsigned char colourBlue[COLOUR_MAX + 1];
static unsigned long paletteHash[128];

int display_open(void)
{
	js_initScreen(VIEW_WIDTH * scale_x,VIEW_HEIGHT * scale_y);
	return(0);
}


int keyboard_raw = -1;
int keyboard_cook = -1;

static long frame_msec;

static int cursor_timer;
static int cursor_hidden;

void display_read(void)
{
	// todo - read mouse?
}

int display_close(void)
{
	return(0);
}

/* ---------------------------------------------------------------------- */

/* handle keyboard */

char keyboard_input[2][INPUT_MAX];

int keyboard_keycode[2][INPUT_MAX];

static const int defaultKeyCode[INPUT_MAX][2] = {
	[INPUT_north] = { 38, 87 }, // up - w
	[INPUT_east] = { 39, 68}, // right - d
	[INPUT_south] = { 40, 83 }, // down - s
	[INPUT_west] = { 37, 65 }, // left - a
	[INPUT_shoot] = { 17, 13 }, // ctrl - enter
	[INPUT_abort] = { 27, 27 },
};

static char keyboardString[512][12];

int keyboard_open(void)
{
	int ply, button, kc;
	for(ply = 0; ply < 2; ply++) {
		for(button = 0; button < INPUT_MAX; button++) {
			keyboard_keycode[ply][button] = defaultKeyCode[button][ply];
		}
	}

	js_initKeyboard();

	return(0);
}

void keyboard_read(void)
{
	int ply, button;
	for(ply = 0; ply < 2; ply++) {
		for(button = 0; button < INPUT_MAX; button++) {
			int kc = keyboard_keycode[ply][button] & 511;
			keyboard_input[ply][button] = js_getKey(kc);
		}
	}

}

int keyboard_close(void)
{
	return(0);
}

int keyboard_to_keycode(char *kn)
{
	int kc = 512;
	//if(*kn) for(kc = 0; kc < 512; kc++) if(SDL_strcmp(kn, keyboardString[kc]) == 0) break;
	if(kc >= 512) kc = -1;
	return(kc);
}


/* ---------------------------------------------------------------------- */

/* handle joysticks */
// TODO - only keyboard input supported for now

char joystick_input[2][INPUT_MAX];

int joystick_open(void)
{
	// TODO - only keyboard input supported for now
	return(0);
}

void joystick_read(void)
{
	// TODO - only keyboard input supported for now
}

int joystick_close(void)
{
	// TODO - only keyboard input supported for now
	return(0);
}

/* ---------------------------------------------------------------------- */

/* handle audio */

static char sound_play[SOUND_MAX]; /* should probably be protected by a semaphore */
static int sound_ptr[SOUND_MAX]; /* play position in the sound */

int audio_open(void)
{
	// handles in javacript / WebAudio
	return(0);
}

void audio_write(struct LOGIC *lev)
{
	int i;

	for(i = 0; i < SOUND_MAX; i++) if(lev->sound[i]) {
		js_playAudio(i);
	}
}

int audio_close(void)
{
	return(0);
}


/* ---------------------------------------------------------------------- */

/* create surfaces */

int screen_open(void)
{
/*
	int i;
	for(i = 0; i < COLOUR_MAX; i++) {
		unsigned long rgbc = colours_xpm[i];

		colourRed[i + 1] = (rgbc >> 24) & 255;
		colourGreen[i + 1] = (rgbc >> 16) & 255;
        colourBlue[i + 1] = (rgbc >> 8) & 255;


		paletteHash[rgbc & 127] = i + 1;
    }
*/

	js_createCanvas("screenPixmap",(VIEW_WIDTH + SCROLL_WIDTH) * scale_x,(VIEW_HEIGHT + SCROLL_HEIGHT) * scale_y);
	js_createCanvas("objectPixmap",GFX_WIDTH * scale_x,GFX_HEIGHT * scale_y);
	js_createCanvas("scorePixmap",SCORE_WIDTH * scale_x,8 * scale_y);
	js_createCanvas("spritePixmap",16 * scale_x,16 * scale_y);

	return(screen_write());
}

/* render graphics */
// TODO move this outside EMscripten to load a png

int screen_write(void)
{
/*
	int x, in_x, out_x;
	int y, in_y, out_y;

	out_y = 0;
	for(in_y = 0; in_y < GFX_HEIGHT; in_y++) {
		out_x = 0;
		for(in_x = 0; in_x < GFX_WIDTH; in_x++) {
			char c = graphics_xpm[in_x][in_y];
			unsigned long pixel = paletteHash[c & 127];
			js_setObjectPixelRGB(colourRed[pixel],colourGreen[pixel],colourBlue[pixel],out_x,out_y);
			out_x++;
		}
		out_y++;
	}

*/
	return(0);
}

int screen_close(void)
{
	return(0);
}

/* ---------------------------------------------------------------------- */

/* game drawing functions */

static short screen_cache[(VIEW_WIDTH + SCROLL_WIDTH) / 16][(VIEW_HEIGHT + SCROLL_HEIGHT) / 16];

static int scn_pixel_left;
static int scn_pixel_top;

void x11_game_clear(void)
{
	js_clearRect(0, 0, VIEW_WIDTH * scale_x,VIEW_HEIGHT * scale_y);
}

void x11_game_scroll(struct LOGIC *lev)
{
	scn_pixel_left = lev->pixel_left % SCROLL_WIDTH;
	scn_pixel_top = lev->pixel_top % SCROLL_HEIGHT;
}

void x11_game_object(struct LOGIC *lev)
{
	int tile;
	int scn_x, lev_x;
	int scn_y, lev_y;
	int sX,sY,sW,sH;
	int dX,dY,dW,dH;

	scn_x = scn_pixel_left / 16;
	for(lev_x = lev->view_left; lev_x <= lev->view_right; lev_x++) {
		scn_y = scn_pixel_top / 16;
		for(lev_y = lev->view_top; lev_y <= lev->view_bottom; lev_y++) {
			tile = object_map[lev->cave[lev_x][lev_y]][lev->frame];
			if(screen_cache[scn_x][scn_y] != tile) {
				screen_cache[scn_x][scn_y] = tile;

				sX = (tile / GFX_HEIGHT) * 16 * scale_x;
				sY = (tile % GFX_HEIGHT) * scale_y;

				dX = scn_x * 16 * scale_x;
				dY = scn_y * 16 * scale_y;

				sW = 16 * scale_x;
				sH = 16 * scale_y;
				dW = sW;
				dH = sH;

				js_blit("objectPixmap",sX,sY,sW,sH,"screenPixmap",dX,dY,dW,dH);
			}
			scn_y++;
		}
		scn_x++;
	}
}

void x11_game_player(struct LOGIC *lev)
{

	int tile;
	int x0, x1;
	int y0, y1;
	int ply;
	int sX,sY,sW,sH;
    int dX,dY,dW,dH;

	for(ply = 0; ply < 2; ply++) {
		if(lev->hide[ply]) continue;

		x0 = lev->pl_pixel_left[ply];
		x1 = lev->pl_pixel_right[ply];
		y0 = lev->pl_pixel_top[ply];
		y1 = lev->pl_pixel_bottom[ply];
		if(!(x0 >= lev->pixel_left && x1 <= lev->pixel_right && y0 >= lev->pixel_top && y1 <= lev->pixel_bottom)) continue;

		x0 -= lev->pixel_left - scn_pixel_left;
		x1 -= lev->pixel_left - scn_pixel_left;
		y0 -= lev->pixel_top - scn_pixel_top;
		y1 -= lev->pixel_top - scn_pixel_top;

		screen_cache[x0 / 16][y0 / 16] = -1; /* mark screen over player as dirty */
		screen_cache[x1 / 16][y1 / 16] = -1;

		tile = sprite_map[lev->pl_anim[ply]][ply][lev->frame];

		sX = (tile / GFX_HEIGHT) * 16 * scale_x;
		sY = (tile % GFX_HEIGHT) * scale_y;

		dX = x0 * scale_x;
        dY = y0 * scale_y;

		sW = 16 * scale_x;
		sH = 16 * scale_y;

		dW = sW;
		dH = sH;

		js_blit("objectPixmap",sX,sY,sW,sH,"screenPixmap",dX,dY,dW,dH);

	}
}

void x11_game_score(struct LOGIC *lev)
{
	// this is handled in x11_game_to_screen
}

void x11_game_to_screen(void)
{
	js_drawGameScreen(scn_pixel_left * scale_x,scn_pixel_top * scale_y,VIEW_WIDTH * scale_x,VIEW_HEIGHT * scale_y);


}

/* ---------------------------------------------------------------------- */

/* score drawing functions */

static short score_cache[SCORE_WIDTH / 8];

void x11_score_clear(void)
{
	js_clearRect(0, 0, SCORE_WIDTH * scale_x,8 * scale_y);
}

void x11_score_print(char str[40])
{
	js_printScore(str);
}

/* ---------------------------------------------------------------------- */

/* menu drawing functions */

static int antsPos;

void x11_menu_clear(void)
{
	//js_clearRect(0, 0, VIEW_WIDTH * scale_x,VIEW_HEIGHT * scale_y);
	js_menuClear();
}

void x11_menu_print(char str[26], int top)
{

	if (top>6){

		int tile;
		int x;
		int sX, sY, sW, sH;
		int dX, dY, dW, dH;

		sX = 0;
		sY = top * 16 * scale_y;
		sW = VIEW_WIDTH * scale_x;
		sH = 16 * scale_y;

		//js_clearRect(sX,sY,sW,sH);

		for(x = 0; x < 26; x++) {
			tile = font_map[str[x] & 127];

			sX = (tile / GFX_HEIGHT) * 16 * scale_x;
			sY = (tile % GFX_HEIGHT) * scale_y;
			sW = 16 * scale_x;
			sH = 16 * scale_y;

			dX = (x * 12 + 4) * scale_x;
			dY = top * 16 * scale_y;
			dW = sW;
			dH = sH;

			if (sX>0) js_blit("objectPixmap",sX,sY,sW,sH,"ctx",dX,dY,dW,dH);
		}

	}
}

void x11_menu_ants(int left, int right, int top)
{
    	int x, x0 = left, x1 = (right + 1);
    	int y, y0 = top * 16, y1 = (top + 1) * 16;
    	int pos, inc;
    	int rx, ry, rw, rh;

    	antsPos = (antsPos + 1) % 16;
    	pos = antsPos;

    	for(x = x0; x < x1; x += inc) {
    		inc = 8 - pos % 8; if(inc > x1 - x) inc = x1 - x;
    		rx = x * scale_x;
    		rw = inc * scale_x;
    		ry = y0 * scale_y;
    		rh = 1 * scale_y;

    		js_fillRect(pos / 8 % 2 ? "white" : "red" ,rx,ry,rw,rh);
    		pos += inc;
    	}
    	for(y = y0; y < y1; y += inc) {
    		inc = 8 - pos % 8; if(inc > y1 - y) inc = y1 - y;
    		rx = (x1 - 1) * scale_x;
    		rw = 1 * scale_x;
    		ry = y * scale_y;
    		rh = inc * scale_y;
    		js_fillRect(pos / 8 % 2 ? "white" : "red" ,rx,ry,rw,rh);
    		pos += inc;
    	}
    	for(x = x1; x > x0; x -= inc) {
    		inc = 8 - pos % 8; if(inc > x - x0) inc = x - x0;
    		rx = (x - inc) * scale_x;
    		rw = inc * scale_x;
    		ry = (y1 - 1) * scale_y;
    		rh = 1 * scale_y;
    		js_fillRect(pos / 8 % 2 ? "white" : "red" ,rx,ry,rw,rh);
    		pos += inc;
    	}
    	for(y = y1; y > y0; y -= inc) {
    		inc = 8 - pos % 8; if(inc > y - y0) inc = y - y0;
    		rx = x0 * scale_x;
    		rw = 1 * scale_x;
    		ry = (y - inc) * scale_y;
    		rh = inc * scale_y;
    		js_fillRect(pos / 8 % 2 ? "white" : "red" ,rx,ry,rw,rh);
    		pos += inc;
    	}

}


void x11_menu_to_screen(void)
{
	js_drawMenu();
}


void x11_quit(void)
{
	js_quit();
}

void x11_event(int event, int data)
{
	js_event(event,data);
}


