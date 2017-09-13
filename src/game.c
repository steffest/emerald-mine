/* emerald mine */

#include "emerald.h"
#include "cave.h"
#include "proto.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <emscripten.h>

long seed = 1684108901; /* what a nice random seed */

static struct CAVE cav;
static struct LOGIC lev;

static struct {
	int cave, level;
	int played, score, won;
} nam[10];

static int menu_option;
static int menu_team;
static int menu_select;
static int box;
static int start_pause;
static int end_pause;


/* ---------------------------------------------------------------------- */

/* process input */

static char game_joy[2][INPUT_MAX];

static char menu_joy[INPUT_MAX];
static char menu_old_joy[INPUT_MAX];

static int keyboard_old_raw;
static int keyboard_old_cook;

static char keyboard_old_input[2][INPUT_MAX];
static char joystick_old_input[2][INPUT_MAX];

static void game_input(void)
{
	int ply;

	keyboard_old_raw = keyboard_raw;
	keyboard_old_cook = keyboard_cook;
	memcpy(keyboard_old_input, keyboard_input, sizeof(keyboard_old_input));
	//memcpy(joystick_old_input, joystick_input, sizeof(joystick_old_input));
	memcpy(menu_old_joy, menu_joy, sizeof(menu_old_joy));

	//display_read();
	keyboard_read();
	//joystick_read();

	for(ply = 0; ply < 2; ply++) {
		//if(memcmp(joystick_old_input[ply], joystick_input[ply], INPUT_MAX)) {
		//	memcpy(game_joy[ply], joystick_input[ply], INPUT_MAX);
		//	memcpy(menu_joy, joystick_input[ply], INPUT_MAX);
		//}
		if(memcmp(keyboard_old_input[ply], keyboard_input[ply], INPUT_MAX)) {
			memcpy(game_joy[ply], keyboard_input[ply], INPUT_MAX);
			memcpy(menu_joy, keyboard_input[ply], INPUT_MAX);
		}

/* this is important for playability */

		if(lev.pl_moved[ply] || game_joy[ply][INPUT_north] || game_joy[ply][INPUT_east] || game_joy[ply][INPUT_south] || game_joy[ply][INPUT_west]) {
			lev.joystick[ply][INPUT_north] = game_joy[ply][INPUT_north];
			lev.joystick[ply][INPUT_east] = game_joy[ply][INPUT_east];
			lev.joystick[ply][INPUT_south] = game_joy[ply][INPUT_south];
			lev.joystick[ply][INPUT_west] = game_joy[ply][INPUT_west];
		}
		lev.joystick[ply][INPUT_shoot] = game_joy[ply][INPUT_shoot];
		lev.joystick[ply][INPUT_abort] = game_joy[ply][INPUT_abort];
	}



}

/* ---------------------------------------------------------------------- */

/* animate the score */

static int forgetPos;

static void game_score(void)
{
	char buffer[60];

	forgetPos = (forgetPos + 1) % 100;

	sprintf(buffer, " time %04d   diamonds %04d   score %04d ", (lev.time + 9) / 10, lev.diamonds, lev.score);

	if(lev.win[0] > 0 && lev.win[1] > 0) {
		strncpy(buffer + 13, "    relax    ", 13);
	} else if(lev.win[0] < 0 || lev.win[1] < 0) {
		if(forgetPos < 50)
		strncpy(buffer + 13, "  forget it  ", 13);
	} else if(lev.diamonds == 0) {
		strncpy(buffer + 13, "find the exit", 13);
	}
	js_updateScore(lev.score,lev.diamonds,lev.time);
	x11_score_print(buffer);
}


/* ---------------------------------------------------------------------- */

/* play emerald mine */

extern void binary_to_cave(char buf[2172], struct CAVE *cav);
extern void cave_to_logic(struct CAVE *cav, struct LOGIC *lev);
extern void logic(struct LOGIC *L);

void end_game(void){
	nam[menu_select].played++;
    nam[menu_select].score += lev.score;
    if(lev.win[0] > 0 && lev.win[1] > 0) {
    	nam[menu_select].won++;

    	if (nam[menu_select].level < getMaxLevel()) nam[menu_select].level++;
    }


 	// set loop back to menu
    box = 11;
	memset(menu_joy, 0, sizeof(menu_joy)); /* flush */
	emscripten_cancel_main_loop();
    emscripten_set_main_loop(menu_main_js, 0, 0);

}

void game_loop(void)
{

		int game_ended = 0;
		game_input();

		logic(&lev);

		audio_write(&lev);

		//x11_game_clear();
		game_score();

		x11_game_scroll(&lev);
		x11_game_object(&lev);
		x11_game_player(&lev);
		x11_game_score(&lev);
		x11_game_to_screen();

		if(start_pause == 0) lev.frame = (lev.frame + 1) % 8;

		if(end_pause == 0) {
			if(game_joy[0][INPUT_shoot] || game_joy[1][INPUT_shoot]) game_ended = 1;
			if(game_joy[0][INPUT_abort] || game_joy[1][INPUT_abort]) game_ended = 1;
		}

		if (game_ended == 1){
			emscripten_cancel_main_loop();
			end_game();
		}else{
			if(lev.win[0] && lev.win[1]) {
				if(end_pause) end_pause--;
			}
			if(start_pause) start_pause--;
		}


	/*

	nam[menu_select].played++;
	nam[menu_select].score += lev.score;
	if(lev.win[0] > 0 && lev.win[1] > 0) nam[menu_select].won++;

	*/
}


void init_game(void){

	emscripten_cancel_main_loop();

	start_pause = 15; /* short pause at the start of each game */
	end_pause = 30; /* short pause at the end of each game */

	if(file_select_cave_level(nam[menu_select].cave, nam[menu_select].level)){
		x11_event(EVENT_ERROR,0);
		return;
	}

	binary_to_cave(file_cave_buffer, &cav);

	cav.seed = seed;
	cav.teamwork = menu_team;

	cave_to_logic(&cav, &lev);

	x11_game_clear();
    x11_score_clear();

	x11_event(EVENT_GAME_START,0);
    emscripten_set_main_loop(game_loop, 0, 0);
}


/* ---------------------------------------------------------------------- */

/* main menu */

static char menu_draw[15];

static void menu_main_refresh(void)
{


	char buffer[60];

	if(menu_draw[0]) {
		sprintf(buffer, "     x11 emerald mine     ");
		x11_menu_print(buffer, 0);
	}
	if(menu_draw[1]) {
		sprintf(buffer, "                          ");
		x11_menu_print(buffer, 1);
	}
	if(menu_draw[2]) {
		sprintf(buffer, "klaus heinz volker wertich");
		x11_menu_print(buffer, 2);
	}
	if(menu_draw[3]) {
		sprintf(buffer, "  no one inc  ruppelware  ");
		x11_menu_print(buffer, 3);
	}
	if(menu_draw[4]) {
		sprintf(buffer, "     david tritscher      ");
		x11_menu_print(buffer, 4);
	}
	if(menu_draw[5]) {
		sprintf(buffer, "                          ");
		x11_menu_print(buffer, 5);
	}
	if(menu_draw[6]) {
		sprintf(buffer, "                          ");
		x11_menu_print(buffer, 6);
	}
	if(menu_draw[7]) {
		switch(menu_option) {
		case 0: sprintf(buffer, "        play mine         "); break;
		case 1: sprintf(buffer, "       change names       "); break;
		case 2: sprintf(buffer, "     change keyboard      "); break;
		case 3: sprintf(buffer, "           quit           "); break;
		}
		x11_menu_print(buffer, 7);
	}
	if(menu_draw[8]) {
		switch(menu_team) {
		case 0: sprintf(buffer, "      single player       "); break;
		case 1: sprintf(buffer, "         teamwork         "); break;
		}
		x11_menu_print(buffer, 8);
	}
	if(menu_draw[9]) {
		sprintf(buffer, "          name %-2d         ", menu_select);
		x11_menu_print(buffer, 9);
	}
	if(menu_draw[10]) {
		if(nam[menu_select].cave >= 0 && nam[menu_select].cave < arg_cave.length) {
			char *path = arg_cave.name[nam[menu_select].cave];
			int a, b;
			for(a = strlen(path), b = 0; a > 0 && b < 26; a--, b++) if(path[a - 1] == '/') break;
			sprintf(buffer, "                          ");
			strncpy(buffer + 13 - b / 2, path + a, b);
		} else {
			sprintf(buffer, "      no caves found      ");
		}
		x11_menu_print(buffer, 10);
	}
	if(menu_draw[11]) {
		sprintf(buffer, "         level %-4d       ", nam[menu_select].level);
		x11_menu_print(buffer, 11);
	}
	if(menu_draw[12]) {
		sprintf(buffer, "                          ");
		x11_menu_print(buffer, 12);
	}
	if(menu_draw[13]) {
		sprintf(buffer, "  played     score   won  ");
		x11_menu_print(buffer, 13);
	}
	if(menu_draw[14]) {
		sprintf(buffer, "    %4d  %8d  %4d  ", nam[menu_select].played, nam[menu_select].score, nam[menu_select].won);
		x11_menu_print(buffer, 14);
	}
	memset(menu_draw, 0, 15); /* all menu lines drawn */
}


void menu_main_js(void){

	seed++;
	x11_menu_clear();
	memset(menu_draw, 1, 15); /* draw all menu lines */

	int north, east, south, west, shoot, abort, key;

	if(menu_option > 3) menu_option = 3;
	if(menu_option < 0) menu_option = 0;
	if(menu_team > 1) menu_team = 1;
	if(menu_team < 0) menu_team = 0;
	if(menu_select > 9) menu_select = 9;
	if(menu_select < 0) menu_select = 0;

	if(menu_option != 0) box = 7;

	if(nam[menu_select].cave >= arg_cave.length) nam[menu_select].cave = arg_cave.length - 1;
	if(nam[menu_select].cave < 0) nam[menu_select].cave = 0;
	if(nam[menu_select].level > 99) nam[menu_select].level = 99;
	if(nam[menu_select].level < 0) nam[menu_select].level = 0;
	if(nam[menu_select].played > 9999) nam[menu_select].played = 9999;
	if(nam[menu_select].played < 0) nam[menu_select].played = 0;
	if(nam[menu_select].score > 99999999) nam[menu_select].score = 99999999;
	if(nam[menu_select].score < 0) nam[menu_select].score = 0;
	if(nam[menu_select].won > 9999) nam[menu_select].won = 9999;
	if(nam[menu_select].won < 0) nam[menu_select].won = 0;

	if(box > 11) box = 11;
	if(box < 7) box = 7;

    game_input();
	menu_main_refresh();

	x11_menu_ants(4, 316, box);
	x11_menu_to_screen();

	north = (!menu_old_joy[INPUT_north] && menu_joy[INPUT_north]);
	east = (!menu_old_joy[INPUT_east] && menu_joy[INPUT_east]);
	south = (!menu_old_joy[INPUT_south] && menu_joy[INPUT_south]);
	west = (!menu_old_joy[INPUT_west] && menu_joy[INPUT_west]);
	shoot = (!menu_old_joy[INPUT_shoot] && menu_joy[INPUT_shoot]);
	abort = (!menu_old_joy[INPUT_abort] && menu_joy[INPUT_abort]);
	key = (keyboard_old_cook == -1 && keyboard_cook != -1);

	if((north || east || south || west || shoot || abort || key)){

		menu_draw[box] = 1;

		if(shoot){
			fprintf(stdout, "action: %d\n", box);
			fprintf(stdout, "menu_option: %d\n", menu_option);

			if(menu_option == 0) {
					emscripten_cancel_main_loop();
					init_game();
                    return;

			}
			//if(menu_option == 1) menu_names();
			//if(menu_option == 2) menu_keyboard();
			//if(menu_option == 3) break;

			if(menu_option == 3) x11_quit();

		}else{
			//if(abort) { menu_option = 3; box = 7; menu_draw[7] = 1; continue; }

			if(isalpha(keyboard_cook)) {
				int aa = 1, bb = 1;
				int n, i;
				for(n = 0; n < arg_cave.length; n++) {
					char *str = arg_cave.name[nam[menu_select].cave];
					for(i = strlen(str); i > 0; i--) if(str[i - 1] == '/') break;
					bb = aa; aa = (tolower(toupper(str[i])) == tolower(toupper(keyboard_cook)));
					if(aa && !bb) break;
					nam[menu_select].cave = (nam[menu_select].cave + 1) % arg_cave.length;
				}
				menu_draw[10] = 1;
			}

			if(isdigit(keyboard_cook)) {
				nam[menu_select].level = (nam[menu_select].level * 10 + (keyboard_cook - '0')) % 100;
				menu_draw[11] = 1;
			}

			switch(box) {
			case 7:
				if(east) menu_option++;
				if(west) menu_option--;
				break;
			case 8:
				if(east) menu_team++;
				if(west) menu_team--;
				break;
			case 9:
				if(east) { menu_select++; menu_draw[10] = 1; menu_draw[11] = 1; menu_draw[14] = 1; }
				if(west) { menu_select--; menu_draw[10] = 1; menu_draw[11] = 1; menu_draw[14] = 1; }
				break;
			case 10:
				if(east) nam[menu_select].cave++;
				if(west) nam[menu_select].cave--;
				break;
			case 11:
				if(east) nam[menu_select].level++;
				if(west) nam[menu_select].level--;
				break;
			}

			if(north) box--;
			if(south) box++;
		}


	}






   // sdl_test(4, 316, 11);


    //x11_menu_to_screen();

}


/* ---------------------------------------------------------------------- */

/* read and write names to config file */

static char *name_tokens[5];

static void menu_names_read(void)
{
	int n;
	for(n = 0; n < 10; n++) {
		file_token_read(&arg_name, n, name_tokens, 5);
		nam[n].cave = atoi(name_tokens[0]);
		nam[n].level = atoi(name_tokens[1]);
		nam[n].played = atoi(name_tokens[2]);
		nam[n].score = atoi(name_tokens[3]);
		nam[n].won = atoi(name_tokens[4]);
	}
}

static void menu_names_write(void)
{
	char cave_str[20], level_str[20], played_str[20], score_str[20], won_str[20];
	int n;
	for(n = 0; n < 10; n++) {
		sprintf(cave_str, "%d", nam[n].cave);
		sprintf(level_str, "%d", nam[n].level);
		sprintf(played_str, "%d", nam[n].played);
		sprintf(score_str, "%d", nam[n].score);
		sprintf(won_str, "%d", nam[n].won);
		name_tokens[0] = cave_str;
		name_tokens[1] = level_str;
		name_tokens[2] = played_str;
		name_tokens[3] = score_str;
		name_tokens[4] = won_str;
		file_token_write(&arg_name, n, name_tokens, 5);
	}
}

/* change names menu */

static void menu_names(void)
{
}

/* ---------------------------------------------------------------------- */

/* read and write keyboard to config file */

static char *key_tokens[INPUT_MAX];

static void menu_keyboard_read(void)
{
	int ply, button;
	for(ply = 0; ply < 2; ply++) {
		file_token_read(&arg_keyboard, ply, key_tokens, INPUT_MAX);
		for(button = 0; button < INPUT_MAX; button++) {
			int keycode = keyboard_to_keycode(key_tokens[button]);
			if(keycode != -1) keyboard_keycode[ply][button] = keycode;
		}
	}
}

static void menu_keyboard_write(void)
{
	int ply, button;
	for(ply = 0; ply < 2; ply++) {
		for(button = 0; button < INPUT_MAX; button++) {
			int keycode = keyboard_keycode[ply][button];
			key_tokens[button] = keyboard_to_name(keycode);
		}
		file_token_write(&arg_keyboard, ply, key_tokens, INPUT_MAX);
	}
}

/* change keyboard menu */

static void menu_keyboard_refresh(void)
{
	char buffer[60];

	if(menu_draw[0]) {
		sprintf(buffer, "     change keyboard      ");
		x11_menu_print(buffer, 0);
	}
	if(menu_draw[1]) {
		sprintf(buffer, "                          ");
		x11_menu_print(buffer, 1);
	}
	if(menu_draw[2]) {
		sprintf(buffer, "player 1 north %-10.10s ", keyboard_to_name(keyboard_keycode[0][INPUT_north]));
		x11_menu_print(buffer, 2);
	}
	if(menu_draw[3]) {
		sprintf(buffer, "         east  %-10.10s ", keyboard_to_name(keyboard_keycode[0][INPUT_east]));
		x11_menu_print(buffer, 3);
	}
	if(menu_draw[4]) {
		sprintf(buffer, "         south %-10.10s ", keyboard_to_name(keyboard_keycode[0][INPUT_south]));
		x11_menu_print(buffer, 4);
	}
	if(menu_draw[5]) {
		sprintf(buffer, "         west  %-10.10s ", keyboard_to_name(keyboard_keycode[0][INPUT_west]));
		x11_menu_print(buffer, 5);
	}
	if(menu_draw[6]) {
		sprintf(buffer, "         shoot %-10.10s ", keyboard_to_name(keyboard_keycode[0][INPUT_shoot]));
		x11_menu_print(buffer, 6);
	}
	if(menu_draw[7]) {
		sprintf(buffer, "                          ");
		x11_menu_print(buffer, 7);
	}
	if(menu_draw[8]) {
		sprintf(buffer, "player 2 north %-10.10s ", keyboard_to_name(keyboard_keycode[1][INPUT_north]));
		x11_menu_print(buffer, 8);
	}
	if(menu_draw[9]) {
		sprintf(buffer, "         east  %-10.10s ", keyboard_to_name(keyboard_keycode[1][INPUT_east]));
		x11_menu_print(buffer, 9);
	}
	if(menu_draw[10]) {
		sprintf(buffer, "         south %-10.10s ", keyboard_to_name(keyboard_keycode[1][INPUT_south]));
		x11_menu_print(buffer, 10);
	}
	if(menu_draw[11]) {
		sprintf(buffer, "         west  %-10.10s ", keyboard_to_name(keyboard_keycode[1][INPUT_west]));
		x11_menu_print(buffer, 11);
	}
	if(menu_draw[12]) {
		sprintf(buffer, "         shoot %-10.10s ", keyboard_to_name(keyboard_keycode[1][INPUT_shoot]));
		x11_menu_print(buffer, 12);
	}
	if(menu_draw[13]) {
		sprintf(buffer, "                          ");
		x11_menu_print(buffer, 13);
	}
	if(menu_draw[14]) {
		sprintf(buffer, "   press escape to skip   ");
		x11_menu_print(buffer, 14);
	}
	memset(menu_draw, 0, 15); /* all menu lines drawn */
}

static void menu_keyboard(void)
{
	int ply, button;

	x11_menu_clear();
	memset(menu_draw, 1, 15); /* draw all menu lines */

	memset(menu_joy, 1, sizeof(menu_joy)); /* flush */

	for(ply = 0; ply < 2; ply++) for(button = 0; button < 5; button++) {
		int p, b;

		int abort, key;
		do {
			game_input();

			menu_keyboard_refresh();

			x11_menu_ants(4, 316, ply*6+button+2);
			x11_menu_to_screen();

			abort = (!menu_old_joy[INPUT_abort] && menu_joy[INPUT_abort]);
			key = (keyboard_old_raw == -1 && keyboard_raw != -1);
		} while(!(abort || key));

		menu_draw[ply*6+button+2] = 1;

		if(abort || game_joy[0][INPUT_abort] || game_joy[1][INPUT_abort]) continue;

		for(p = 0; p < 2; p++) for(b = 0; b < 5; b++) {
			if(keyboard_keycode[p][b] == keyboard_raw) {
				keyboard_keycode[p][b] = keyboard_keycode[ply][button];
				menu_draw[p*6+b+2] = 1;
			}
		}
		keyboard_keycode[ply][button] = keyboard_raw;
	}
}

/* ---------------------------------------------------------------------- */

void menu(void)
{
	menu_keyboard_read();
	menu_names_read();

	menu_option = 0;
	menu_team = 0;
	menu_select = arg_select;

	file_cave_refresh();

	#ifdef __EMSCRIPTEN__

		menu_option = 0;
		box = 11;

        memset(menu_joy, 0, sizeof(menu_joy)); /* flush */

		emscripten_cancel_main_loop();
		emscripten_set_main_loop(menu_main_js, 0, 0);



	#else
      for(;;) {
      		menu_option = 0;
      		menu_main();

      		if(menu_option == 0) game();
      		if(menu_option == 1) menu_names();
      		if(menu_option == 2) menu_keyboard();
      		if(menu_option == 3) break;

      		seed++;
      	}

      	arg_select = menu_select;
        menu_keyboard_write();
        menu_names_write();

    #endif



}
