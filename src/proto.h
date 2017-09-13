#ifndef proto_H
#define proto_H

#include "emerald.h"

/* ========== game.c ========== */

extern void game(void);
extern void menu(void);
extern void menu_main_js(void);

/* ========== file.c ========== */

/* command line arguments */

extern char *arg_display;
extern char *arg_geometry;
extern int arg_install;
extern int arg_fullscreen;

/* hash table of strings */

struct hash { char **name; int length; int alloc; };

extern struct hash arg_cave;
extern struct hash arg_path;
extern struct hash arg_audio;
extern struct hash arg_joystick;
extern struct hash arg_keyboard;
extern struct hash arg_name;
extern int arg_select;

/* manage config file */

extern void file_token_read(struct hash *arg, int index, char **token, int cnt);
extern void file_token_write(struct hash *arg, int index, char **token, int cnt);

extern int file_open(void);
extern int file_close(void);

extern void file_config_read(void);
extern void file_config_write(void);

/* read cave file and directory */

extern char file_cave_buffer[4096];

extern void file_cave_refresh(void);
extern int file_select_cave_level(int cave, int level);
extern int getMaxLevel(void);

/* ========== datamap.c ========== */

/* map tiles to graphics */

extern const short object_map[XMAX][8];
extern const short sprite_map[SPRITE_MAX][2][8];
extern const short font_map[128];
extern const short score_map[128];

/* ========== datagfx.c ========== */

/* contrasting colours are first to look better on small colourmaps */

enum {
	COLOUR_black,
	COLOUR_white,
	COLOUR_red,
	COLOUR_MAX = 84
};

extern const unsigned long colours_xpm[COLOUR_MAX];

/* graphics is objects, sprites, and fonts */

#define GFX_WIDTH 544
#define GFX_HEIGHT 512

extern const char *graphics_xpm[GFX_WIDTH];

#define SCROLL_WIDTH 336
#define SCROLL_HEIGHT 256

#define SCORE_WIDTH 320

/* ========== datasnd.c ========== */

/* lower numbered sounds have playing priority */

extern const unsigned char *sounds_data[SOUND_MAX];
extern const int sounds_length[SOUND_MAX];

/* maximum number of sounds to play at once */

#define MIXER_MAX 4

/* ========== dataulaw.c ========== */

extern const short ulaw_to_linear[256];
extern const unsigned char linear_to_ulaw[16384];

/* ========== x11.c ========== */


/* external events */

#define EVENT_ERROR 1
#define EVENT_GAME_START 2
#define EVENT_GAME_END 3

/* window and event loop */

extern int keyboard_raw;
extern int keyboard_cook;

extern int display_open(void);
extern void display_read(void);
extern int display_close(void);

/* keyboard controller */

extern char keyboard_input[2][INPUT_MAX];
extern int keyboard_keycode[2][INPUT_MAX];

extern int keyboard_open(void);
extern void keyboard_read(void);
extern int keyboard_close(void);

extern char *keyboard_to_name(int kc);
extern int keyboard_to_keycode(char *kn);

/* joystick controller */

extern char joystick_input[2][INPUT_MAX];

extern int joystick_open(void);
extern void joystick_read(void);
extern int joystick_close(void);

/* audio thread */

extern int audio_open(void);
extern void audio_write(struct LOGIC *lev);
extern int audio_close(void);

/* render the graphics */

extern int screen_open(void);
extern int screen_write(void);
extern int screen_close(void);

/* drawing functions */

extern void x11_game_clear(void);
extern void x11_game_scroll(struct LOGIC *lev);
extern void x11_game_object(struct LOGIC *lev);
extern void x11_game_player(struct LOGIC *lev);
extern void x11_game_score(struct LOGIC *lev);
extern void x11_game_to_screen(void);

extern void x11_score_clear(void);
extern void x11_score_print(char str[40]);

extern void x11_menu_clear(void);
extern void x11_menu_print(char str[26], int top);
extern void x11_menu_ants(int left, int right, int top);
extern void x11_menu_to_screen(void);
extern void x11_quit(void);
extern void x11_event(int event,int data);


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
extern void js_updateScore(int score, int diamonds, int time);
extern void js_drawObject(int tile, int x, int y);
extern void js_blit(char source, int srcX, int srcY, int srcW, int srcH, char destination , int dstX, int dstY, int dstW, int dstH);
extern void js_fillRect(char color, int x, int y, int w, int h);
extern void js_drawGameScreen(int x, int y, int width, int height);
extern void js_setObjectPixelRGB(int r, int g, int b, int x, int y);
extern void js_playAudio(int index);
extern void js_quit(void);
extern void js_event(int event, int data);
extern void js_tick(int left, int up, int right, int down, int shoot);
extern int js_getSeed(void);

#endif
