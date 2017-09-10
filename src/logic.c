/*

Copyright © David Tritscher. All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

3. All advertising materials mentioning features or use of this software
   must display the following acknowledgement: This product includes
   software developed by David Tritscher.

4. The name of the developer may not be used to endorse or promote products
   derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY DAVID TRITSCHER ''AS IS'' AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
EVENT SHALL DAVID TRITSCHER BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

/* if you see something that does not make sense, do not assume it is a mistake. */

#include "emerald.h"

#undef SPRING_GOOD /* rolling spring is like a stone */

#define RANDOM(x) (L->random = L->random * 12421 + 1, (x) * (L->random & 65535) >> 16)
#define PLAY(sample) { if((unsigned)(x - L->view_left) <= VIEW_WIDTH / 16 && (unsigned)(y - L->view_top) <= VIEW_HEIGHT / 16) L->sound[sample] = 1; }

static const char is_space[XMAX] = { [Xspace] = 1, [Xsplash_e] = 1, [Xsplash_w] = 1, };

typedef struct LOGIC *LOGIC;
typedef void(*FUNC)(LOGIC L, int x, int y);

static void Lboom_bug(LOGIC L, int x, int y)
{
	L->boom[x-1][y-1] = Xemerald;
	L->boom[x][y-1] = Xemerald;
	L->boom[x+1][y-1] = Xemerald;
	L->boom[x-1][y] = Xemerald;
	L->boom[x][y] = Xdiamond;
	L->boom[x+1][y] = Xemerald;
	L->boom[x-1][y+1] = Xemerald;
	L->boom[x][y+1] = Xemerald;
	L->boom[x+1][y+1] = Xemerald;
}
static void Lboom_tank(LOGIC L, int x, int y)
{
	L->boom[x-1][y-1] = Xspace;
	L->boom[x][y-1] = Xspace;
	L->boom[x+1][y-1] = Xspace;
	L->boom[x-1][y] = Xspace;
	L->boom[x][y] = Xspace;
	L->boom[x+1][y] = Xspace;
	L->boom[x-1][y+1] = Xspace;
	L->boom[x][y+1] = Xspace;
	L->boom[x+1][y+1] = Xspace;
}

static void Lplayer_check(LOGIC L)
{
	int x;
	int y;
	int P;
	for(P = 0; P < 2; P++) {
		if(L->hide[P]) continue;
		if(L->win[P]) continue;

		x = L->pl_right[P];
		y = L->pl_bottom[P];

		if(L->time == 0) L->win[P] = -1;

		switch(L->cave[x][y]) {
		case Xplayer:
		case Xdynamite_1: case Xdynamite_2:
		case Xdynamite_3: case Xdynamite_4:
			break;
		default:
			L->win[P] = -1;
		}
		switch(L->cave[x][y-1]) {
		case Xbug_1_n: case Xbug_2_n:
		case Xbug_1_e: case Xbug_2_e:
		case Xbug_1_s: case Xbug_2_s:
		case Xbug_1_w: case Xbug_2_w:
		case Xtank_1_n: case Xtank_2_n:
		case Xtank_1_e: case Xtank_2_e:
		case Xtank_1_s: case Xtank_2_s:
		case Xtank_1_w: case Xtank_2_w:
			L->win[P] = -1;
		}
		switch(L->cave[x+1][y]) {
		case Xbug_1_n: case Xbug_2_n:
		case Xbug_1_e: case Xbug_2_e:
		case Xbug_1_s: case Xbug_2_s:
		case Xbug_1_w: case Xbug_2_w:
		case Xtank_1_n: case Xtank_2_n:
		case Xtank_1_e: case Xtank_2_e:
		case Xtank_1_s: case Xtank_2_s:
		case Xtank_1_w: case Xtank_2_w:
			L->win[P] = -1;
		}
		switch(L->cave[x][y+1]) {
		case Xbug_1_n: case Xbug_2_n:
		case Xbug_1_e: case Xbug_2_e:
		case Xbug_1_s: case Xbug_2_s:
		case Xbug_1_w: case Xbug_2_w:
		case Xtank_1_n: case Xtank_2_n:
		case Xtank_1_e: case Xtank_2_e:
		case Xtank_1_s: case Xtank_2_s:
		case Xtank_1_w: case Xtank_2_w:
			L->win[P] = -1;
		}
		switch(L->cave[x-1][y]) {
		case Xbug_1_n: case Xbug_2_n:
		case Xbug_1_e: case Xbug_2_e:
		case Xbug_1_s: case Xbug_2_s:
		case Xbug_1_w: case Xbug_2_w:
		case Xtank_1_n: case Xtank_2_n:
		case Xtank_1_e: case Xtank_2_e:
		case Xtank_1_s: case Xtank_2_s:
		case Xtank_1_w: case Xtank_2_w:
			L->win[P] = -1;
		}
	}
	for(P = 0; P < 2; P++) {
		if(L->hide[P]) continue;

		x = L->pl_right[P];
		y = L->pl_bottom[P];

		switch(L->cave[x][y-1]) {
		case Xbug_1_n: case Xbug_2_n:
		case Xbug_1_e: case Xbug_2_e:
		case Xbug_1_s: case Xbug_2_s:
		case Xbug_1_w: case Xbug_2_w:
			L->cave[x][y-1] = Xchain;
			Lboom_bug(L, x, y-1);
			break;
		case Xtank_1_n: case Xtank_2_n:
		case Xtank_1_e: case Xtank_2_e:
		case Xtank_1_s: case Xtank_2_s:
		case Xtank_1_w: case Xtank_2_w:
			L->cave[x][y-1] = Xchain;
			Lboom_tank(L, x, y-1);
			break;
		}
		switch(L->cave[x+1][y]) {
		case Xbug_1_n: case Xbug_2_n:
		case Xbug_1_e: case Xbug_2_e:
		case Xbug_1_s: case Xbug_2_s:
		case Xbug_1_w: case Xbug_2_w:
			L->cave[x+1][y] = Xchain;
			Lboom_bug(L, x+1, y);
			break;
		case Xtank_1_n: case Xtank_2_n:
		case Xtank_1_e: case Xtank_2_e:
		case Xtank_1_s: case Xtank_2_s:
		case Xtank_1_w: case Xtank_2_w:
			L->cave[x+1][y] = Xchain;
			Lboom_tank(L, x+1, y);
			break;
		}
		switch(L->cave[x][y+1]) {
		case Xbug_1_n: case Xbug_2_n:
		case Xbug_1_e: case Xbug_2_e:
		case Xbug_1_s: case Xbug_2_s:
		case Xbug_1_w: case Xbug_2_w:
			L->cave[x][y+1] = Xchain;
			Lboom_bug(L, x, y+1);
			break;
		case Xtank_1_n: case Xtank_2_n:
		case Xtank_1_e: case Xtank_2_e:
		case Xtank_1_s: case Xtank_2_s:
		case Xtank_1_w: case Xtank_2_w:
			L->cave[x][y+1] = Xchain;
			Lboom_tank(L, x, y+1);
			break;
		}
		switch(L->cave[x-1][y]) {
		case Xbug_1_n: case Xbug_2_n:
		case Xbug_1_e: case Xbug_2_e:
		case Xbug_1_s: case Xbug_2_s:
		case Xbug_1_w: case Xbug_2_w:
			L->cave[x-1][y] = Xchain;
			Lboom_bug(L, x-1, y);
			break;
		case Xtank_1_n: case Xtank_2_n:
		case Xtank_1_e: case Xtank_2_e:
		case Xtank_1_s: case Xtank_2_s:
		case Xtank_1_w: case Xtank_2_w:
			L->cave[x-1][y] = Xchain;
			Lboom_tank(L, x-1, y);
			break;
		}
	}
	for(P = 0; P < 2; P++) {
		if(L->hide[P]) continue;
		if(L->win[P] == 0) continue;

		x = L->pl_right[P];
		y = L->pl_bottom[P];

		L->hide[P] = 1;

		L->sound[SOUND_die] = 1;
		L->boom[x][y] = Xspace;

		switch(L->cave[x][y]) {
		case Xchain:
			break;
		case Xacid_1: case Xacid_2:
		case Xacid_3: case Xacid_4:
		case Xacid_5: case Xacid_6:
		case Xacid_7: case Xacid_8:
			if(L->cave[x+1][y-1] == Xspace) L->cave[x+1][y-1] = Xsplash_e;
			if(L->cave[x-1][y-1] == Xspace) L->cave[x-1][y-1] = Xsplash_w;
			L->sound[SOUND_acid] = 1;
			break;
		default:
			L->cave[x][y] = Xboom_1;
			break;
		}
	}
}

static int Lplayer_walk_n(LOGIC L, int P)
{
	int x = L->pl_right[P];
	int y = L->pl_bottom[P];
	int temp;

	L->pl_anim[P] = SPRITE_walk_n;
	y -= 1;
	switch(L->cave[x][y]) {
	case Xboom_android:
	case Xboom_1:
	case Xacid_1: case Xacid_2:
	case Xacid_3: case Xacid_4:
	case Xacid_5: case Xacid_6:
	case Xacid_7: case Xacid_8:
	case Xbug_1_n: case Xbug_2_n:
	case Xbug_1_e: case Xbug_2_e:
	case Xbug_1_s: case Xbug_2_s:
	case Xbug_1_w: case Xbug_2_w:
	case Xtank_1_n: case Xtank_2_n:
	case Xtank_1_e: case Xtank_2_e:
	case Xtank_1_s: case Xtank_2_s:
	case Xtank_1_w: case Xtank_2_w:
		L->pl_bottom[P] -= 1;
		return(1);
	case Xspace:
	case Xsplash_e: case Xsplash_w:
		L->cave[x][y] = Xplayer;
		L->next[x][y] = Xplayer;
		L->sound[SOUND_space] = 1;
		L->pl_bottom[P] -= 1;
		return(1);
	case Xplant:
	case Yplant:
		L->cave[x][y] = Yplant;
		L->next[x][y] = Xplant;
		L->sound[SOUND_space] = 1;
		L->pl_bottom[P] -= 1;
		return(1);
	case Xgrass:
		L->cave[x][y] = Ygrass_nB;
		L->next[x][y] = Xplayer;
		L->sound[SOUND_dirt] = 1;
		L->pl_bottom[P] -= 1;
		return(1);
	case Xdirt:
		L->cave[x][y] = Ydirt_nB;
		L->next[x][y] = Xplayer;
		L->sound[SOUND_dirt] = 1;
		L->pl_bottom[P] -= 1;
		return(1);
	case Xandroid:
	case Xandroid_1_n: case Xandroid_2_n:
	case Xandroid_1_e: case Xandroid_2_e:
	case Xandroid_1_s: case Xandroid_2_s:
	case Xandroid_1_w: case Xandroid_2_w:
		L->pl_anim[P] = SPRITE_push_n;
		switch(L->cave[x][y-1]) {
		case Xacid_1: case Xacid_2:
		case Xacid_3: case Xacid_4:
		case Xacid_5: case Xacid_6:
		case Xacid_7: case Xacid_8:
			L->cave[x][y] = Yandroid_nB;
			L->next[x][y] = Xplayer;
			if(L->cave[x+1][y-2] == Xspace) L->cave[x+1][y-2] = Xsplash_e;
			if(L->cave[x-1][y-2] == Xspace) L->cave[x-1][y-2] = Xsplash_w;
			L->sound[SOUND_acid] = 1;
			L->sound[SOUND_push] = 1;
			L->pl_bottom[P] -= 1;
			return(1);
		case Xspace:
		case Xsplash_e: case Xsplash_w:
			L->cave[x][y] = Yandroid_nB;
			L->next[x][y] = Xplayer;
			L->cave[x][y-1] = Yandroid_n;
			L->next[x][y-1] = Xandroid_2_n;
			L->sound[SOUND_push] = 1;
			L->pl_bottom[P] -= 1;
			return(1);
		}
		return(1);
	case Xemerald:
	case Xemerald_pause:
		L->diamonds -= 1;
		L->score += L->emerald_score;
		L->cave[x][y] = Yemerald_space;
		L->next[x][y] = Xplayer;
		L->sound[SOUND_collect] = 1;
		L->pl_bottom[P] -= 1;
		return(1);
	case Xdiamond:
	case Xdiamond_pause:
		L->diamonds -= 3;
		L->score += L->diamond_score;
		L->cave[x][y] = Ydiamond_space;
		L->next[x][y] = Xplayer;
		L->sound[SOUND_collect] = 1;
		L->pl_bottom[P] -= 1;
		return(1);
	case Xdynamite:
		if(L->dynamite[P] < 9999) L->dynamite[P] += 1;
		L->score += L->dynamite_score;
		L->cave[x][y] = Ydynamite_space;
		L->next[x][y] = Xplayer;
		L->sound[SOUND_collect] = 1;
		L->pl_bottom[P] -= 1;
		return(1);
	case Xkey_1: temp = 1; goto key_n;
	case Xkey_2: temp = 2; goto key_n;
	case Xkey_3: temp = 4; goto key_n;
	case Xkey_4: temp = 8; goto key_n;
	case Xkey_5: temp = 16; goto key_n;
	case Xkey_6: temp = 32; goto key_n;
	case Xkey_7: temp = 64; goto key_n;
	case Xkey_8: temp = 128; goto key_n;
	key_n:
		L->keys[P] |= temp;
		L->score += L->key_score;
		L->cave[x][y] = Xpop;
		L->next[x][y] = Xplayer;
		L->sound[SOUND_collect] = 1;
		L->pl_bottom[P] -= 1;
		return(1);
	case Xdoor_1: case Xfake_door_1: temp = 1; goto door_n;
	case Xdoor_2: case Xfake_door_2: temp = 2; goto door_n;
	case Xdoor_3: case Xfake_door_3: temp = 4; goto door_n;
	case Xdoor_4: case Xfake_door_4: temp = 8; goto door_n;
	case Xdoor_5: case Xfake_door_5: temp = 16; goto door_n;
	case Xdoor_6: case Xfake_door_6: temp = 32; goto door_n;
	case Xdoor_7: case Xfake_door_7: temp = 64; goto door_n;
	case Xdoor_8: case Xfake_door_8: temp = 128; goto door_n;
	door_n:
		if(L->keys[P] & temp) {
			if(is_space[L->cave[x][y-1]]) {
				L->cave[x][y-1] = Xplayer;
				L->next[x][y-1] = Xplayer;
				L->sound[SOUND_door] = 1;
				L->pl_bottom[P] -= 2;
				return(1);
			}
		}
		break;
	case Xballoon:
		L->pl_anim[P] = SPRITE_push_n;
		switch(L->cave[x][y-1]) {
		case Xacid_1: case Xacid_2:
		case Xacid_3: case Xacid_4:
		case Xacid_5: case Xacid_6:
		case Xacid_7: case Xacid_8:
			L->cave[x][y] = Yballoon_nB;
			L->next[x][y] = Xplayer;
			if(L->cave[x+1][y-2] == Xspace) L->cave[x+1][y-2] = Xsplash_e;
			if(L->cave[x-1][y-2] == Xspace) L->cave[x-1][y-2] = Xsplash_w;
			L->sound[SOUND_acid] = 1;
			L->sound[SOUND_push] = 1;
			L->pl_bottom[P] -= 1;
			return(1);
		case Xspace:
		case Xsplash_e: case Xsplash_w:
			L->cave[x][y] = Yballoon_nB;
			L->next[x][y] = Xplayer;
			L->cave[x][y-1] = Yballoon_n;
			L->next[x][y-1] = Xballoon;
			L->sound[SOUND_push] = 1;
			L->pl_bottom[P] -= 1;
			return(1);
		}
		return(1);
	case Xwheel:
		L->pl_anim[P] = SPRITE_still;
		L->wheel_cnt = L->wheel_time;
		L->wheel_x = x;
		L->wheel_y = y;
		L->sound[SOUND_press] = 1;
		return(1);
	case Xswitch:
		L->pl_anim[P] = SPRITE_still;
		L->ball_cnt = L->ball_time;
		L->ball_state = !L->ball_state;
		L->sound[SOUND_press] = 1;
		return(1);
	case Xlenses:
		L->lenses_cnt = L->lenses_time;
		L->score += L->lenses_score;
		L->cave[x][y] = Xpop;
		L->next[x][y] = Xplayer;
		L->sound[SOUND_collect] = 1;
		L->pl_bottom[P] -= 1;
		return(1);
	case Xmagnify:
		L->magnify_cnt = L->magnify_time;
		L->score += L->magnify_score;
		L->cave[x][y] = Xpop;
		L->next[x][y] = Xplayer;
		L->sound[SOUND_collect] = 1;
		L->pl_bottom[P] -= 1;
		return(1);
	case Xwind_multi:
	case Xwind_n: temp = 0; goto wind_n;
	case Xwind_e: temp = 1; goto wind_n;
	case Xwind_s: temp = 2; goto wind_n;
	case Xwind_w: temp = 3; goto wind_n;
	wind_n:
		L->pl_anim[P] = SPRITE_still;
		L->wind_direction = temp;
		L->wind_cnt = L->wind_time;
		L->sound[SOUND_press] = 1;
		return(1);
	case Xwind_stop:
		L->pl_anim[P] = SPRITE_still;
		L->wind_cnt = 0;
		L->sound[SOUND_press] = 1;
		return(1);
	case Xexit_1: case Xexit_2: case Xexit_3:
		L->win[P] = 1;
		if(L->win[0] > 0 && L->win[1] > 0) L->score += L->time * L->exit_score / 100;
		L->sound[SOUND_exit] = 1;
		L->pl_bottom[P] -= 1;
		return(1);
	}
	return(0);
}
static int Lplayer_shoot_n(LOGIC L, int P)
{
	int x = L->pl_right[P];
	int y = L->pl_bottom[P];
	int temp;

	L->pl_anim[P] = SPRITE_walk_n;
	y -= 1;
	switch(L->cave[x][y]) {
	case Xgrass:
	case Xdirt:
		L->pl_anim[P] = SPRITE_shoot_n;
		L->cave[x][y] = Xpop;
		L->next[x][y] = Xspace;
		L->sound[SOUND_dirt] = 1;
		return(1);
	case Xemerald:
	case Xemerald_pause:
		L->diamonds -= 1;
		L->score += L->emerald_score;
		L->cave[x][y] = Yemerald_space;
		L->next[x][y] = Xspace;
		L->sound[SOUND_collect] = 1;
		return(1);
	case Xdiamond:
	case Xdiamond_pause:
		L->diamonds -= 3;
		L->score += L->diamond_score;
		L->cave[x][y] = Ydiamond_space;
		L->next[x][y] = Xspace;
		L->sound[SOUND_collect] = 1;
		return(1);
	case Xdynamite:
		if(L->dynamite[P] < 9999) L->dynamite[P] += 1;
		L->score += L->dynamite_score;
		L->cave[x][y] = Ydynamite_space;
		L->next[x][y] = Xspace;
		L->sound[SOUND_collect] = 1;
		return(1);
	case Xkey_1: temp = 1; goto key_n;
	case Xkey_2: temp = 2; goto key_n;
	case Xkey_3: temp = 4; goto key_n;
	case Xkey_4: temp = 8; goto key_n;
	case Xkey_5: temp = 16; goto key_n;
	case Xkey_6: temp = 32; goto key_n;
	case Xkey_7: temp = 64; goto key_n;
	case Xkey_8: temp = 128; goto key_n;
	key_n:
		L->keys[P] |= temp;
		L->score += L->key_score;
		L->cave[x][y] = Xpop;
		L->next[x][y] = Xspace;
		L->sound[SOUND_collect] = 1;
		return(1);
	case Xlenses:
		L->lenses_cnt = L->lenses_time;
		L->score += L->lenses_score;
		L->cave[x][y] = Xpop;
		L->next[x][y] = Xspace;
		L->sound[SOUND_collect] = 1;
		return(1);
	case Xmagnify:
		L->magnify_cnt = L->magnify_time;
		L->score += L->magnify_score;
		L->cave[x][y] = Xpop;
		L->next[x][y] = Xspace;
		L->sound[SOUND_collect] = 1;
		return(1);
	}
	return(0);
}
static int Lplayer_walk_e(LOGIC L, int P)
{
	int x = L->pl_right[P];
	int y = L->pl_bottom[P];
	int temp;

	L->pl_anim[P] = SPRITE_walk_e;
	x += 1;
	switch(L->cave[x][y]) {
	case Xboom_android:
	case Xboom_1:
	case Xacid_1: case Xacid_2:
	case Xacid_3: case Xacid_4:
	case Xacid_5: case Xacid_6:
	case Xacid_7: case Xacid_8:
	case Xbug_1_n: case Xbug_2_n:
	case Xbug_1_e: case Xbug_2_e:
	case Xbug_1_s: case Xbug_2_s:
	case Xbug_1_w: case Xbug_2_w:
	case Xtank_1_n: case Xtank_2_n:
	case Xtank_1_e: case Xtank_2_e:
	case Xtank_1_s: case Xtank_2_s:
	case Xtank_1_w: case Xtank_2_w:
		L->pl_right[P] += 1;
		return(1);
	case Xspace:
	case Xsplash_e: case Xsplash_w:
		L->cave[x][y] = Xplayer;
		L->next[x][y] = Xplayer;
		L->sound[SOUND_space] = 1;
		L->pl_right[P] += 1;
		return(1);
	case Xplant:
	case Yplant:
		L->cave[x][y] = Yplant;
		L->next[x][y] = Xplant;
		L->sound[SOUND_space] = 1;
		L->pl_right[P] += 1;
		return(1);
	case Xgrass:
		L->cave[x][y] = Ygrass_eB;
		L->next[x][y] = Xplayer;
		L->sound[SOUND_dirt] = 1;
		L->pl_right[P] += 1;
		return(1);
	case Xdirt:
		L->cave[x][y] = Ydirt_eB;
		L->next[x][y] = Xplayer;
		L->sound[SOUND_dirt] = 1;
		L->pl_right[P] += 1;
		return(1);
	case Xandroid:
	case Xandroid_1_n: case Xandroid_2_n:
	case Xandroid_1_e: case Xandroid_2_e:
	case Xandroid_1_s: case Xandroid_2_s:
	case Xandroid_1_w: case Xandroid_2_w:
		L->pl_anim[P] = SPRITE_push_e;
		switch(L->cave[x+1][y]) {
		case Xacid_1: case Xacid_2:
		case Xacid_3: case Xacid_4:
		case Xacid_5: case Xacid_6:
		case Xacid_7: case Xacid_8:
			L->cave[x][y] = Yandroid_eB;
			L->next[x][y] = Xplayer;
			if(L->cave[x+2][y-1] == Xspace) L->cave[x+2][y-1] = Xsplash_e;
			if(L->cave[x][y-1] == Xspace) L->cave[x][y-1] = Xsplash_w;
			L->sound[SOUND_acid] = 1;
			L->sound[SOUND_push] = 1;
			L->pl_right[P] += 1;
			return(1);
		case Xspace:
		case Xsplash_e: case Xsplash_w:
			L->cave[x][y] = Yandroid_eB;
			L->next[x][y] = Xplayer;
			L->cave[x+1][y] = Yandroid_e;
			L->next[x+1][y] = Xandroid_2_e;
			L->sound[SOUND_push] = 1;
			L->pl_right[P] += 1;
			return(1);
		}
		return(1);
	case Xemerald:
	case Xemerald_pause:
		L->diamonds -= 1;
		L->score += L->emerald_score;
		L->cave[x][y] = Yemerald_space;
		L->next[x][y] = Xplayer;
		L->sound[SOUND_collect] = 1;
		L->pl_right[P] += 1;
		return(1);
	case Xdiamond:
	case Xdiamond_pause:
		L->diamonds -= 3;
		L->score += L->diamond_score;
		L->cave[x][y] = Ydiamond_space;
		L->next[x][y] = Xplayer;
		L->sound[SOUND_collect] = 1;
		L->pl_right[P] += 1;
		return(1);
	case Xstone:
		L->pl_anim[P] = SPRITE_push_e;
		switch(L->cave[x+1][y]) {
		case Xacid_1: case Xacid_2:
		case Xacid_3: case Xacid_4:
		case Xacid_5: case Xacid_6:
		case Xacid_7: case Xacid_8:
			L->cave[x][y] = Ystone_eB;
			L->next[x][y] = Xplayer;
			if(L->cave[x+2][y-1] == Xspace) L->cave[x+2][y-1] = Xsplash_e;
			if(L->cave[x][y-1] == Xspace) L->cave[x][y-1] = Xsplash_w;
			L->sound[SOUND_acid] = 1;
			L->sound[SOUND_roll] = 1;
			L->pl_right[P] += 1;
			return(1);
		case Xspace:
		case Xsplash_e: case Xsplash_w:
			L->cave[x][y] = Ystone_eB;
			L->next[x][y] = Xplayer;
			L->cave[x+1][y] = Ystone_e;
			L->next[x+1][y] = Xstone_pause;
			L->sound[SOUND_roll] = 1;
			L->pl_right[P] += 1;
			return(1);
		}
		return(1);
	case Xbomb:
		L->pl_anim[P] = SPRITE_push_e;
		switch(L->cave[x+1][y]) {
		case Xacid_1: case Xacid_2:
		case Xacid_3: case Xacid_4:
		case Xacid_5: case Xacid_6:
		case Xacid_7: case Xacid_8:
			L->cave[x][y] = Ybomb_eB;
			L->next[x][y] = Xplayer;
			if(L->cave[x+2][y-1] == Xspace) L->cave[x+2][y-1] = Xsplash_e;
			if(L->cave[x][y-1] == Xspace) L->cave[x][y-1] = Xsplash_w;
			L->sound[SOUND_acid] = 1;
			L->sound[SOUND_roll] = 1;
			L->pl_right[P] += 1;
			return(1);
		case Xspace:
		case Xsplash_e: case Xsplash_w:
			L->cave[x][y] = Ybomb_eB;
			L->next[x][y] = Xplayer;
			L->cave[x+1][y] = Ybomb_e;
			L->next[x+1][y] = Xbomb_pause;
			L->sound[SOUND_roll] = 1;
			L->pl_right[P] += 1;
			return(1);
		}
		return(1);
	case Xnut:
		L->pl_anim[P] = SPRITE_push_e;
		switch(L->cave[x+1][y]) {
		case Xacid_1: case Xacid_2:
		case Xacid_3: case Xacid_4:
		case Xacid_5: case Xacid_6:
		case Xacid_7: case Xacid_8:
			L->cave[x][y] = Ynut_eB;
			L->next[x][y] = Xplayer;
			if(L->cave[x+2][y-1] == Xspace) L->cave[x+2][y-1] = Xsplash_e;
			if(L->cave[x][y-1] == Xspace) L->cave[x][y-1] = Xsplash_w;
			L->sound[SOUND_acid] = 1;
			L->sound[SOUND_roll] = 1;
			L->pl_right[P] += 1;
			return(1);
		case Xspace:
		case Xsplash_e: case Xsplash_w:
			L->cave[x][y] = Ynut_eB;
			L->next[x][y] = Xplayer;
			L->cave[x+1][y] = Ynut_e;
			L->next[x+1][y] = Xnut_pause;
			L->sound[SOUND_roll] = 1;
			L->pl_right[P] += 1;
			return(1);
		}
		return(1);
	case Xspring:
		L->pl_anim[P] = SPRITE_push_e;
		switch(L->cave[x+1][y]) {
		case Xalien:
		case Xalien_pause:
			L->score += L->spring_score;
			L->cave[x][y] = Yspring_alien_eB;
			L->next[x][y] = Xplayer;
			L->cave[x+1][y] = Yspring_alien_e;
			L->next[x+1][y] = Xspring_e;
			L->sound[SOUND_slurp] = 1;
			L->sound[SOUND_roll] = 1;
			L->pl_right[P] += 1;
			return(1);
		case Xacid_1: case Xacid_2:
		case Xacid_3: case Xacid_4:
		case Xacid_5: case Xacid_6:
		case Xacid_7: case Xacid_8:
			L->cave[x][y] = Yspring_eB;
			L->next[x][y] = Xplayer;
			if(L->cave[x+2][y-1] == Xspace) L->cave[x+2][y-1] = Xsplash_e;
			if(L->cave[x][y-1] == Xspace) L->cave[x][y-1] = Xsplash_w;
			L->sound[SOUND_acid] = 1;
			L->sound[SOUND_roll] = 1;
			L->pl_right[P] += 1;
			return(1);
		case Xspace:
		case Xsplash_e: case Xsplash_w:
			L->cave[x][y] = Yspring_eB;
			L->next[x][y] = Xplayer;
			L->cave[x+1][y] = Yspring_e;
			L->next[x+1][y] = Xspring_e;
			L->sound[SOUND_roll] = 1;
			L->pl_right[P] += 1;
			return(1);
		}
		return(1);
	case Xstone_pause:
	case Xbomb_pause:
	case Xnut_pause:
	case Xspring_pause:
	case Xspring_e: case Xspring_w:
	case Xsand_stone_1_sB: case Xsand_stone_2_sB:
	case Xsand_stone_3_sB: case Xsand_stone_4_sB:
		L->pl_anim[P] = SPRITE_push_e;
		return(1);
	case Xdynamite:
		if(L->dynamite[P] < 9999) L->dynamite[P] += 1;
		L->score += L->dynamite_score;
		L->cave[x][y] = Ydynamite_space;
		L->next[x][y] = Xplayer;
		L->sound[SOUND_collect] = 1;
		L->pl_right[P] += 1;
		return(1);
	case Xkey_1: temp = 1; goto key_e;
	case Xkey_2: temp = 2; goto key_e;
	case Xkey_3: temp = 4; goto key_e;
	case Xkey_4: temp = 8; goto key_e;
	case Xkey_5: temp = 16; goto key_e;
	case Xkey_6: temp = 32; goto key_e;
	case Xkey_7: temp = 64; goto key_e;
	case Xkey_8: temp = 128; goto key_e;
	key_e:
		L->keys[P] |= temp;
		L->score += L->key_score;
		L->cave[x][y] = Xpop;
		L->next[x][y] = Xplayer;
		L->sound[SOUND_collect] = 1;
		L->pl_right[P] += 1;
		return(1);
	case Xdoor_1: case Xfake_door_1: temp = 1; goto door_e;
	case Xdoor_2: case Xfake_door_2: temp = 2; goto door_e;
	case Xdoor_3: case Xfake_door_3: temp = 4; goto door_e;
	case Xdoor_4: case Xfake_door_4: temp = 8; goto door_e;
	case Xdoor_5: case Xfake_door_5: temp = 16; goto door_e;
	case Xdoor_6: case Xfake_door_6: temp = 32; goto door_e;
	case Xdoor_7: case Xfake_door_7: temp = 64; goto door_e;
	case Xdoor_8: case Xfake_door_8: temp = 128; goto door_e;
	door_e:
		if(L->keys[P] & temp) {
			if(is_space[L->cave[x+1][y]]) {
				L->cave[x+1][y] = Xplayer;
				L->next[x+1][y] = Xplayer;
				L->sound[SOUND_door] = 1;
				L->pl_right[P] += 2;
				return(1);
			}
		}
		break;
	case Xballoon:
		L->pl_anim[P] = SPRITE_push_e;
		switch(L->cave[x+1][y]) {
		case Xacid_1: case Xacid_2:
		case Xacid_3: case Xacid_4:
		case Xacid_5: case Xacid_6:
		case Xacid_7: case Xacid_8:
			L->cave[x][y] = Yballoon_eB;
			L->next[x][y] = Xplayer;
			if(L->cave[x+2][y-1] == Xspace) L->cave[x+2][y-1] = Xsplash_e;
			if(L->cave[x][y-1] == Xspace) L->cave[x][y-1] = Xsplash_w;
			L->sound[SOUND_acid] = 1;
			L->sound[SOUND_push] = 1;
			L->pl_right[P] += 1;
			return(1);
		case Xspace:
		case Xsplash_e: case Xsplash_w:
			L->cave[x][y] = Yballoon_eB;
			L->next[x][y] = Xplayer;
			L->cave[x+1][y] = Yballoon_e;
			L->next[x+1][y] = Xballoon;
			L->sound[SOUND_push] = 1;
			L->pl_right[P] += 1;
			return(1);
		}
		return(1);
	case Xwheel:
		L->pl_anim[P] = SPRITE_still;
		L->wheel_cnt = L->wheel_time;
		if(x > L->cave_right) x -= L->width;
		L->wheel_x = x;
		L->wheel_y = y;
		L->sound[SOUND_press] = 1;
		return(1);
	case Xswitch:
		L->pl_anim[P] = SPRITE_still;
		L->ball_cnt = L->ball_time;
		L->ball_state = !L->ball_state;
		L->sound[SOUND_press] = 1;
		return(1);
	case Xlenses:
		L->lenses_cnt = L->lenses_time;
		L->score += L->lenses_score;
		L->cave[x][y] = Xpop;
		L->next[x][y] = Xplayer;
		L->sound[SOUND_collect] = 1;
		L->pl_right[P] += 1;
		return(1);
	case Xmagnify:
		L->magnify_cnt = L->magnify_time;
		L->score += L->magnify_score;
		L->cave[x][y] = Xpop;
		L->next[x][y] = Xplayer;
		L->sound[SOUND_collect] = 1;
		L->pl_right[P] += 1;
		return(1);
	case Xwind_n: temp = 0; goto wind_e;
	case Xwind_multi:
	case Xwind_e: temp = 1; goto wind_e;
	case Xwind_s: temp = 2; goto wind_e;
	case Xwind_w: temp = 3; goto wind_e;
	wind_e:
		L->pl_anim[P] = SPRITE_still;
		L->wind_direction = temp;
		L->wind_cnt = L->wind_time;
		L->sound[SOUND_press] = 1;
		return(1);
	case Xwind_stop:
		L->pl_anim[P] = SPRITE_still;
		L->wind_cnt = 0;
		L->sound[SOUND_press] = 1;
		return(1);
	case Xexit_1: case Xexit_2: case Xexit_3:
		L->win[P] = 1;
		if(L->win[0] > 0 && L->win[1] > 0) L->score += L->time * L->exit_score / 100;
		L->sound[SOUND_exit] = 1;
		L->pl_right[P] += 1;
		return(1);
	}
	return(0);
}
static int Lplayer_shoot_e(LOGIC L, int P)
{
	int x = L->pl_right[P];
	int y = L->pl_bottom[P];
	int temp;

	L->pl_anim[P] = SPRITE_walk_e;
	x += 1;
	switch(L->cave[x][y]) {
	case Xgrass:
	case Xdirt:
		L->pl_anim[P] = SPRITE_shoot_e;
		L->cave[x][y] = Xpop;
		L->next[x][y] = Xspace;
		L->sound[SOUND_dirt] = 1;
		return(1);
	case Xemerald:
	case Xemerald_pause:
		L->diamonds -= 1;
		L->score += L->emerald_score;
		L->cave[x][y] = Yemerald_space;
		L->next[x][y] = Xspace;
		L->sound[SOUND_collect] = 1;
		return(1);
	case Xdiamond:
	case Xdiamond_pause:
		L->diamonds -= 3;
		L->score += L->diamond_score;
		L->cave[x][y] = Ydiamond_space;
		L->next[x][y] = Xspace;
		L->sound[SOUND_collect] = 1;
		return(1);
	case Xdynamite:
		if(L->dynamite[P] < 9999) L->dynamite[P] += 1;
		L->score += L->dynamite_score;
		L->cave[x][y] = Ydynamite_space;
		L->next[x][y] = Xspace;
		L->sound[SOUND_collect] = 1;
		return(1);
	case Xkey_1: temp = 1; goto key_e;
	case Xkey_2: temp = 2; goto key_e;
	case Xkey_3: temp = 4; goto key_e;
	case Xkey_4: temp = 8; goto key_e;
	case Xkey_5: temp = 16; goto key_e;
	case Xkey_6: temp = 32; goto key_e;
	case Xkey_7: temp = 64; goto key_e;
	case Xkey_8: temp = 128; goto key_e;
	key_e:
		L->keys[P] |= temp;
		L->score += L->key_score;
		L->cave[x][y] = Xpop;
		L->next[x][y] = Xspace;
		L->sound[SOUND_collect] = 1;
		return(1);
	case Xlenses:
		L->lenses_cnt = L->lenses_time;
		L->score += L->lenses_score;
		L->cave[x][y] = Xpop;
		L->next[x][y] = Xspace;
		L->sound[SOUND_collect] = 1;
		return(1);
	case Xmagnify:
		L->magnify_cnt = L->magnify_time;
		L->score += L->magnify_score;
		L->cave[x][y] = Xpop;
		L->next[x][y] = Xspace;
		L->sound[SOUND_collect] = 1;
		return(1);
	}
	return(0);
}
static int Lplayer_walk_s(LOGIC L, int P)
{
	int x = L->pl_right[P];
	int y = L->pl_bottom[P];
	int temp;

	L->pl_anim[P] = SPRITE_walk_s;
	y += 1;
	switch(L->cave[x][y]) {
	case Xboom_android:
	case Xboom_1:
	case Xacid_1: case Xacid_2:
	case Xacid_3: case Xacid_4:
	case Xacid_5: case Xacid_6:
	case Xacid_7: case Xacid_8:
	case Xbug_1_n: case Xbug_2_n:
	case Xbug_1_e: case Xbug_2_e:
	case Xbug_1_s: case Xbug_2_s:
	case Xbug_1_w: case Xbug_2_w:
	case Xtank_1_n: case Xtank_2_n:
	case Xtank_1_e: case Xtank_2_e:
	case Xtank_1_s: case Xtank_2_s:
	case Xtank_1_w: case Xtank_2_w:
		L->pl_bottom[P] += 1;
		return(1);
	case Xspace:
	case Xsplash_e: case Xsplash_w:
		L->cave[x][y] = Xplayer;
		L->next[x][y] = Xplayer;
		L->sound[SOUND_space] = 1;
		L->pl_bottom[P] += 1;
		return(1);
	case Xplant:
	case Yplant:
		L->cave[x][y] = Yplant;
		L->next[x][y] = Xplant;
		L->sound[SOUND_space] = 1;
		L->pl_bottom[P] += 1;
		return(1);
	case Xgrass:
		L->cave[x][y] = Ygrass_sB;
		L->next[x][y] = Xplayer;
		L->sound[SOUND_dirt] = 1;
		L->pl_bottom[P] += 1;
		return(1);
	case Xdirt:
		L->cave[x][y] = Ydirt_sB;
		L->next[x][y] = Xplayer;
		L->sound[SOUND_dirt] = 1;
		L->pl_bottom[P] += 1;
		return(1);
	case Xandroid:
	case Xandroid_1_n: case Xandroid_2_n:
	case Xandroid_1_e: case Xandroid_2_e:
	case Xandroid_1_s: case Xandroid_2_s:
	case Xandroid_1_w: case Xandroid_2_w:
		L->pl_anim[P] = SPRITE_push_s;
		switch(L->cave[x][y+1]) {
		case Xacid_1: case Xacid_2:
		case Xacid_3: case Xacid_4:
		case Xacid_5: case Xacid_6:
		case Xacid_7: case Xacid_8:
			L->cave[x][y] = Yandroid_sB;
			L->next[x][y] = Xplayer;
			if(L->cave[x+1][y] == Xspace) L->cave[x+1][y] = Xsplash_e;
			if(L->cave[x-1][y] == Xspace) L->cave[x-1][y] = Xsplash_w;
			L->sound[SOUND_acid] = 1;
			L->sound[SOUND_push] = 1;
			L->pl_bottom[P] += 1;
			return(1);
		case Xspace:
		case Xsplash_e: case Xsplash_w:
			L->cave[x][y] = Yandroid_sB;
			L->next[x][y] = Xplayer;
			L->cave[x][y+1] = Yandroid_s;
			L->next[x][y+1] = Xandroid_2_s;
			L->sound[SOUND_push] = 1;
			L->pl_bottom[P] += 1;
			return(1);
		}
		return(1);
	case Xemerald:
	case Xemerald_pause:
		L->diamonds -= 1;
		L->score += L->emerald_score;
		L->cave[x][y] = Yemerald_space;
		L->next[x][y] = Xplayer;
		L->sound[SOUND_collect] = 1;
		L->pl_bottom[P] += 1;
		return(1);
	case Xdiamond:
	case Xdiamond_pause:
		L->diamonds -= 3;
		L->score += L->diamond_score;
		L->cave[x][y] = Ydiamond_space;
		L->next[x][y] = Xplayer;
		L->sound[SOUND_collect] = 1;
		L->pl_bottom[P] += 1;
		return(1);
	case Xdynamite:
		if(L->dynamite[P] < 9999) L->dynamite[P] += 1;
		L->score += L->dynamite_score;
		L->cave[x][y] = Ydynamite_space;
		L->next[x][y] = Xplayer;
		L->sound[SOUND_collect] = 1;
		L->pl_bottom[P] += 1;
		return(1);
	case Xkey_1: temp = 1; goto key_s;
	case Xkey_2: temp = 2; goto key_s;
	case Xkey_3: temp = 4; goto key_s;
	case Xkey_4: temp = 8; goto key_s;
	case Xkey_5: temp = 16; goto key_s;
	case Xkey_6: temp = 32; goto key_s;
	case Xkey_7: temp = 64; goto key_s;
	case Xkey_8: temp = 128; goto key_s;
	key_s:
		L->keys[P] |= temp;
		L->score += L->key_score;
		L->cave[x][y] = Xpop;
		L->next[x][y] = Xplayer;
		L->sound[SOUND_collect] = 1;
		L->pl_bottom[P] += 1;
		return(1);
	case Xdoor_1: case Xfake_door_1: temp = 1; goto door_s;
	case Xdoor_2: case Xfake_door_2: temp = 2; goto door_s;
	case Xdoor_3: case Xfake_door_3: temp = 4; goto door_s;
	case Xdoor_4: case Xfake_door_4: temp = 8; goto door_s;
	case Xdoor_5: case Xfake_door_5: temp = 16; goto door_s;
	case Xdoor_6: case Xfake_door_6: temp = 32; goto door_s;
	case Xdoor_7: case Xfake_door_7: temp = 64; goto door_s;
	case Xdoor_8: case Xfake_door_8: temp = 128; goto door_s;
	door_s:
		if(L->keys[P] & temp) {
			if(is_space[L->cave[x][y+1]]) {
				L->cave[x][y+1] = Xplayer;
				L->next[x][y+1] = Xplayer;
				L->sound[SOUND_door] = 1;
				L->pl_bottom[P] += 2;
				return(1);
			}
		}
		break;
	case Xballoon:
		L->pl_anim[P] = SPRITE_push_s;
		switch(L->cave[x][y+1]) {
		case Xacid_1: case Xacid_2:
		case Xacid_3: case Xacid_4:
		case Xacid_5: case Xacid_6:
		case Xacid_7: case Xacid_8:
			L->cave[x][y] = Yballoon_sB;
			L->next[x][y] = Xplayer;
			if(L->cave[x+1][y] == Xspace) L->cave[x+1][y] = Xsplash_e;
			if(L->cave[x-1][y] == Xspace) L->cave[x-1][y] = Xsplash_w;
			L->sound[SOUND_acid] = 1;
			L->sound[SOUND_push] = 1;
			L->pl_bottom[P] += 1;
			return(1);
		case Xspace:
		case Xsplash_e: case Xsplash_w:
			L->cave[x][y] = Yballoon_sB;
			L->next[x][y] = Xplayer;
			L->cave[x][y+1] = Yballoon_s;
			L->next[x][y+1] = Xballoon;
			L->sound[SOUND_push] = 1;
			L->pl_bottom[P] += 1;
			return(1);
		}
		return(1);
	case Xwheel:
		L->pl_anim[P] = SPRITE_still;
		L->wheel_cnt = L->wheel_time;
		L->wheel_x = x;
		L->wheel_y = y;
		L->sound[SOUND_press] = 1;
		return(1);
	case Xswitch:
		L->pl_anim[P] = SPRITE_still;
		L->ball_cnt = L->ball_time;
		L->ball_state = !L->ball_state;
		L->sound[SOUND_press] = 1;
		return(1);
	case Xlenses:
		L->lenses_cnt = L->lenses_time;
		L->score += L->lenses_score;
		L->cave[x][y] = Xpop;
		L->next[x][y] = Xplayer;
		L->sound[SOUND_collect] = 1;
		L->pl_bottom[P] += 1;
		return(1);
	case Xmagnify:
		L->magnify_cnt = L->magnify_time;
		L->score += L->magnify_score;
		L->cave[x][y] = Xpop;
		L->next[x][y] = Xplayer;
		L->sound[SOUND_collect] = 1;
		L->pl_bottom[P] += 1;
		return(1);
	case Xwind_n: temp = 0; goto wind_s;
	case Xwind_e: temp = 1; goto wind_s;
	case Xwind_multi:
	case Xwind_s: temp = 2; goto wind_s;
	case Xwind_w: temp = 3; goto wind_s;
	wind_s:
		L->pl_anim[P] = SPRITE_still;
		L->wind_direction = temp;
		L->wind_cnt = L->wind_time;
		L->sound[SOUND_press] = 1;
		return(1);
	case Xwind_stop:
		L->pl_anim[P] = SPRITE_still;
		L->wind_cnt = 0;
		L->sound[SOUND_press] = 1;
		return(1);
	case Xexit_1: case Xexit_2: case Xexit_3:
		L->win[P] = 1;
		if(L->win[0] > 0 && L->win[1] > 0) L->score += L->time * L->exit_score / 100;
		L->sound[SOUND_exit] = 1;
		L->pl_bottom[P] += 1;
		return(1);
	}
	return(0);
}
static int Lplayer_shoot_s(LOGIC L, int P)
{
	int x = L->pl_right[P];
	int y = L->pl_bottom[P];
	int temp;

	L->pl_anim[P] = SPRITE_walk_s;
	y += 1;
	switch(L->cave[x][y]) {
	case Xgrass:
	case Xdirt:
		L->pl_anim[P] = SPRITE_shoot_s;
		L->cave[x][y] = Xpop;
		L->next[x][y] = Xspace;
		L->sound[SOUND_dirt] = 1;
		return(1);
	case Xemerald:
	case Xemerald_pause:
		L->diamonds -= 1;
		L->score += L->emerald_score;
		L->cave[x][y] = Yemerald_space;
		L->next[x][y] = Xspace;
		L->sound[SOUND_collect] = 1;
		return(1);
	case Xdiamond:
	case Xdiamond_pause:
		L->diamonds -= 3;
		L->score += L->diamond_score;
		L->cave[x][y] = Ydiamond_space;
		L->next[x][y] = Xspace;
		L->sound[SOUND_collect] = 1;
		return(1);
	case Xdynamite:
		if(L->dynamite[P] < 9999) L->dynamite[P] += 1;
		L->score += L->dynamite_score;
		L->cave[x][y] = Ydynamite_space;
		L->next[x][y] = Xspace;
		L->sound[SOUND_collect] = 1;
		return(1);
	case Xkey_1: temp = 1; goto key_s;
	case Xkey_2: temp = 2; goto key_s;
	case Xkey_3: temp = 4; goto key_s;
	case Xkey_4: temp = 8; goto key_s;
	case Xkey_5: temp = 16; goto key_s;
	case Xkey_6: temp = 32; goto key_s;
	case Xkey_7: temp = 64; goto key_s;
	case Xkey_8: temp = 128; goto key_s;
	key_s:
		L->keys[P] |= temp;
		L->score += L->key_score;
		L->cave[x][y] = Xpop;
		L->next[x][y] = Xspace;
		L->sound[SOUND_collect] = 1;
		return(1);
	case Xlenses:
		L->lenses_cnt = L->lenses_time;
		L->score += L->lenses_score;
		L->cave[x][y] = Xpop;
		L->next[x][y] = Xspace;
		L->sound[SOUND_collect] = 1;
		return(1);
	case Xmagnify:
		L->magnify_cnt = L->magnify_time;
		L->score += L->magnify_score;
		L->cave[x][y] = Xpop;
		L->next[x][y] = Xspace;
		L->sound[SOUND_collect] = 1;
		return(1);
	}
	return(0);
}
static int Lplayer_walk_w(LOGIC L, int P)
{
	int x = L->pl_right[P];
	int y = L->pl_bottom[P];
	int temp;

	L->pl_anim[P] = SPRITE_walk_w;
	x -= 1;
	switch(L->cave[x][y]) {
	case Xboom_android:
	case Xboom_1:
	case Xacid_1: case Xacid_2:
	case Xacid_3: case Xacid_4:
	case Xacid_5: case Xacid_6:
	case Xacid_7: case Xacid_8:
	case Xbug_1_n: case Xbug_2_n:
	case Xbug_1_e: case Xbug_2_e:
	case Xbug_1_s: case Xbug_2_s:
	case Xbug_1_w: case Xbug_2_w:
	case Xtank_1_n: case Xtank_2_n:
	case Xtank_1_e: case Xtank_2_e:
	case Xtank_1_s: case Xtank_2_s:
	case Xtank_1_w: case Xtank_2_w:
		L->pl_right[P] -= 1;
		return(1);
	case Xspace:
	case Xsplash_e: case Xsplash_w:
		L->cave[x][y] = Xplayer;
		L->next[x][y] = Xplayer;
		L->sound[SOUND_space] = 1;
		L->pl_right[P] -= 1;
		return(1);
	case Xplant:
	case Yplant:
		L->cave[x][y] = Yplant;
		L->next[x][y] = Xplant;
		L->sound[SOUND_space] = 1;
		L->pl_right[P] -= 1;
		return(1);
	case Xgrass:
		L->cave[x][y] = Ygrass_wB;
		L->next[x][y] = Xplayer;
		L->sound[SOUND_dirt] = 1;
		L->pl_right[P] -= 1;
		return(1);
	case Xdirt:
		L->cave[x][y] = Ydirt_wB;
		L->next[x][y] = Xplayer;
		L->sound[SOUND_dirt] = 1;
		L->pl_right[P] -= 1;
		return(1);
	case Xandroid:
	case Xandroid_1_n: case Xandroid_2_n:
	case Xandroid_1_e: case Xandroid_2_e:
	case Xandroid_1_s: case Xandroid_2_s:
	case Xandroid_1_w: case Xandroid_2_w:
		L->pl_anim[P] = SPRITE_push_w;
		switch(L->cave[x-1][y]) {
		case Xacid_1: case Xacid_2:
		case Xacid_3: case Xacid_4:
		case Xacid_5: case Xacid_6:
		case Xacid_7: case Xacid_8:
			L->cave[x][y] = Yandroid_wB;
			L->next[x][y] = Xplayer;
			if(L->cave[x][y-1] == Xspace) L->cave[x][y-1] = Xsplash_e;
			if(L->cave[x-2][y-1] == Xspace) L->cave[x-2][y-1] = Xsplash_w;
			L->sound[SOUND_acid] = 1;
			L->sound[SOUND_push] = 1;
			L->pl_right[P] -= 1;
			return(1);
		case Xspace:
		case Xsplash_e: case Xsplash_w:
			L->cave[x][y] = Yandroid_wB;
			L->next[x][y] = Xplayer;
			L->cave[x-1][y] = Yandroid_w;
			L->next[x-1][y] = Xandroid_2_w;
			L->sound[SOUND_push] = 1;
			L->pl_right[P] -= 1;
			return(1);
		}
		return(1);
	case Xemerald:
	case Xemerald_pause:
		L->diamonds -= 1;
		L->score += L->emerald_score;
		L->cave[x][y] = Yemerald_space;
		L->next[x][y] = Xplayer;
		L->sound[SOUND_collect] = 1;
		L->pl_right[P] -= 1;
		return(1);
	case Xdiamond:
	case Xdiamond_pause:
		L->diamonds -= 3;
		L->score += L->diamond_score;
		L->cave[x][y] = Ydiamond_space;
		L->next[x][y] = Xplayer;
		L->sound[SOUND_collect] = 1;
		L->pl_right[P] -= 1;
		return(1);
	case Xstone:
		L->pl_anim[P] = SPRITE_push_w;
		switch(L->cave[x-1][y]) {
		case Xacid_1: case Xacid_2:
		case Xacid_3: case Xacid_4:
		case Xacid_5: case Xacid_6:
		case Xacid_7: case Xacid_8:
			L->cave[x][y] = Ystone_wB;
			L->next[x][y] = Xplayer;
			if(L->cave[x][y-1] == Xspace) L->cave[x][y-1] = Xsplash_e;
			if(L->cave[x-2][y-1] == Xspace) L->cave[x-2][y-1] = Xsplash_w;
			L->sound[SOUND_acid] = 1;
			L->sound[SOUND_roll] = 1;
			L->pl_right[P] -= 1;
			return(1);
		case Xspace:
		case Xsplash_e: case Xsplash_w:
			L->cave[x][y] = Ystone_wB;
			L->next[x][y] = Xplayer;
			L->cave[x-1][y] = Ystone_w;
			L->next[x-1][y] = Xstone_pause;
			L->sound[SOUND_roll] = 1;
			L->pl_right[P] -= 1;
			return(1);
		}
		return(1);
	case Xbomb:
		L->pl_anim[P] = SPRITE_push_w;
		switch(L->cave[x-1][y]) {
		case Xacid_1: case Xacid_2:
		case Xacid_3: case Xacid_4:
		case Xacid_5: case Xacid_6:
		case Xacid_7: case Xacid_8:
			L->cave[x][y] = Ybomb_wB;
			L->next[x][y] = Xplayer;
			if(L->cave[x][y-1] == Xspace) L->cave[x][y-1] = Xsplash_e;
			if(L->cave[x-2][y-1] == Xspace) L->cave[x-2][y-1] = Xsplash_w;
			L->sound[SOUND_acid] = 1;
			L->sound[SOUND_roll] = 1;
			L->pl_right[P] -= 1;
			return(1);
		case Xspace:
		case Xsplash_e: case Xsplash_w:
			L->cave[x][y] = Ybomb_wB;
			L->next[x][y] = Xplayer;
			L->cave[x-1][y] = Ybomb_w;
			L->next[x-1][y] = Xbomb_pause;
			L->sound[SOUND_roll] = 1;
			L->pl_right[P] -= 1;
			return(1);
		}
		return(1);
	case Xnut:
		L->pl_anim[P] = SPRITE_push_w;
		switch(L->cave[x-1][y]) {
		case Xacid_1: case Xacid_2:
		case Xacid_3: case Xacid_4:
		case Xacid_5: case Xacid_6:
		case Xacid_7: case Xacid_8:
			L->cave[x][y] = Ynut_wB;
			L->next[x][y] = Xplayer;
			if(L->cave[x][y-1] == Xspace) L->cave[x][y-1] = Xsplash_e;
			if(L->cave[x-2][y-1] == Xspace) L->cave[x-2][y-1] = Xsplash_w;
			L->sound[SOUND_acid] = 1;
			L->sound[SOUND_roll] = 1;
			L->pl_right[P] -= 1;
			return(1);
		case Xspace:
		case Xsplash_e: case Xsplash_w:
			L->cave[x][y] = Ynut_wB;
			L->next[x][y] = Xplayer;
			L->cave[x-1][y] = Ynut_w;
			L->next[x-1][y] = Xnut_pause;
			L->sound[SOUND_roll] = 1;
			L->pl_right[P] -= 1;
			return(1);
		}
		return(1);
	case Xspring:
		L->pl_anim[P] = SPRITE_push_w;
		switch(L->cave[x-1][y]) {
		case Xalien:
		case Xalien_pause:
			L->score += L->spring_score;
			L->cave[x][y] = Yspring_alien_wB;
			L->next[x][y] = Xplayer;
			L->cave[x-1][y] = Yspring_alien_w;
			L->next[x-1][y] = Xspring_w;
			L->sound[SOUND_slurp] = 1;
			L->sound[SOUND_roll] = 1;
			L->pl_right[P] -= 1;
			return(1);
		case Xacid_1: case Xacid_2:
		case Xacid_3: case Xacid_4:
		case Xacid_5: case Xacid_6:
		case Xacid_7: case Xacid_8:
			L->cave[x][y] = Yspring_wB;
			L->next[x][y] = Xplayer;
			if(L->cave[x][y-1] == Xspace) L->cave[x][y-1] = Xsplash_e;
			if(L->cave[x-2][y-1] == Xspace) L->cave[x-2][y-1] = Xsplash_w;
			L->sound[SOUND_acid] = 1;
			L->sound[SOUND_roll] = 1;
			L->pl_right[P] -= 1;
			return(1);
		case Xspace:
		case Xsplash_e: case Xsplash_w:
			L->cave[x][y] = Yspring_wB;
			L->next[x][y] = Xplayer;
			L->cave[x-1][y] = Yspring_w;
			L->next[x-1][y] = Xspring_w;
			L->sound[SOUND_roll] = 1;
			L->pl_right[P] -= 1;
			return(1);
		}
		return(1);
	case Xstone_pause:
	case Xbomb_pause:
	case Xnut_pause:
	case Xspring_pause:
	case Xspring_e: case Xspring_w:
	case Xsand_stone_1_sB: case Xsand_stone_2_sB:
	case Xsand_stone_3_sB: case Xsand_stone_4_sB:
		L->pl_anim[P] = SPRITE_push_w;
		return(1);
	case Xdynamite:
		if(L->dynamite[P] < 9999) L->dynamite[P] += 1;
		L->score += L->dynamite_score;
		L->cave[x][y] = Ydynamite_space;
		L->next[x][y] = Xplayer;
		L->sound[SOUND_collect] = 1;
		L->pl_right[P] -= 1;
		return(1);
	case Xkey_1: temp = 1; goto key_w;
	case Xkey_2: temp = 2; goto key_w;
	case Xkey_3: temp = 4; goto key_w;
	case Xkey_4: temp = 8; goto key_w;
	case Xkey_5: temp = 16; goto key_w;
	case Xkey_6: temp = 32; goto key_w;
	case Xkey_7: temp = 64; goto key_w;
	case Xkey_8: temp = 128; goto key_w;
	key_w:
		L->keys[P] |= temp;
		L->score += L->key_score;
		L->cave[x][y] = Xpop;
		L->next[x][y] = Xplayer;
		L->sound[SOUND_collect] = 1;
		L->pl_right[P] -= 1;
		return(1);
	case Xdoor_1: case Xfake_door_1: temp = 1; goto door_w;
	case Xdoor_2: case Xfake_door_2: temp = 2; goto door_w;
	case Xdoor_3: case Xfake_door_3: temp = 4; goto door_w;
	case Xdoor_4: case Xfake_door_4: temp = 8; goto door_w;
	case Xdoor_5: case Xfake_door_5: temp = 16; goto door_w;
	case Xdoor_6: case Xfake_door_6: temp = 32; goto door_w;
	case Xdoor_7: case Xfake_door_7: temp = 64; goto door_w;
	case Xdoor_8: case Xfake_door_8: temp = 128; goto door_w;
	door_w:
		if(L->keys[P] & temp) {
			if(is_space[L->cave[x-1][y]]) {
				L->cave[x-1][y] = Xplayer;
				L->next[x-1][y] = Xplayer;
				L->sound[SOUND_door] = 1;
				L->pl_right[P] -= 2;
				return(1);
			}
		}
		break;
	case Xballoon:
		L->pl_anim[P] = SPRITE_push_w;
		switch(L->cave[x-1][y]) {
		case Xacid_1: case Xacid_2:
		case Xacid_3: case Xacid_4:
		case Xacid_5: case Xacid_6:
		case Xacid_7: case Xacid_8:
			L->cave[x][y] = Yballoon_wB;
			L->next[x][y] = Xplayer;
			if(L->cave[x][y-1] == Xspace) L->cave[x][y-1] = Xsplash_e;
			if(L->cave[x-2][y-1] == Xspace) L->cave[x-2][y-1] = Xsplash_w;
			L->sound[SOUND_acid] = 1;
			L->sound[SOUND_push] = 1;
			L->pl_right[P] -= 1;
			return(1);
		case Xspace:
		case Xsplash_e: case Xsplash_w:
			L->cave[x][y] = Yballoon_wB;
			L->next[x][y] = Xplayer;
			L->cave[x-1][y] = Yballoon_w;
			L->next[x-1][y] = Xballoon;
			L->sound[SOUND_push] = 1;
			L->pl_right[P] -= 1;
			return(1);
		}
		return(1);
	case Xwheel:
		L->pl_anim[P] = SPRITE_still;
		L->wheel_cnt = L->wheel_time;
		if(x < L->cave_left) x += L->width;
		L->wheel_x = x;
		L->wheel_y = y;
		L->sound[SOUND_press] = 1;
		return(1);
	case Xswitch:
		L->pl_anim[P] = SPRITE_still;
		L->ball_cnt = L->ball_time;
		L->ball_state = !L->ball_state;
		L->sound[SOUND_press] = 1;
		return(1);
	case Xlenses:
		L->lenses_cnt = L->lenses_time;
		L->score += L->lenses_score;
		L->cave[x][y] = Xpop;
		L->next[x][y] = Xplayer;
		L->sound[SOUND_collect] = 1;
		L->pl_right[P] -= 1;
		return(1);
	case Xmagnify:
		L->magnify_cnt = L->magnify_time;
		L->score += L->magnify_score;
		L->cave[x][y] = Xpop;
		L->next[x][y] = Xplayer;
		L->sound[SOUND_collect] = 1;
		L->pl_right[P] -= 1;
		return(1);
	case Xwind_n: temp = 0; goto wind_w;
	case Xwind_e: temp = 1; goto wind_w;
	case Xwind_s: temp = 2; goto wind_w;
	case Xwind_multi:
	case Xwind_w: temp = 3; goto wind_w;
	wind_w:
		L->pl_anim[P] = SPRITE_still;
		L->wind_direction = temp;
		L->wind_cnt = L->wind_time;
		L->sound[SOUND_press] = 1;
		return(1);
	case Xwind_stop:
		L->pl_anim[P] = SPRITE_still;
		L->wind_cnt = 0;
		L->sound[SOUND_press] = 1;
		return(1);
	case Xexit_1: case Xexit_2: case Xexit_3:
		L->win[P] = 1;
		if(L->win[0] > 0 && L->win[1] > 0) L->score += L->time * L->exit_score / 100;
		L->sound[SOUND_exit] = 1;
		L->pl_right[P] -= 1;
		return(1);
	}
	return(0);
}
static int Lplayer_shoot_w(LOGIC L, int P)
{
	int x = L->pl_right[P];
	int y = L->pl_bottom[P];
	int temp;

	L->pl_anim[P] = SPRITE_walk_w;
	x -= 1;
	switch(L->cave[x][y]) {
	case Xgrass:
	case Xdirt:
		L->pl_anim[P] = SPRITE_shoot_w;
		L->cave[x][y] = Xpop;
		L->next[x][y] = Xspace;
		L->sound[SOUND_dirt] = 1;
		return(1);
	case Xemerald:
	case Xemerald_pause:
		L->diamonds -= 1;
		L->score += L->emerald_score;
		L->cave[x][y] = Yemerald_space;
		L->next[x][y] = Xspace;
		L->sound[SOUND_collect] = 1;
		return(1);
	case Xdiamond:
	case Xdiamond_pause:
		L->diamonds -= 3;
		L->score += L->diamond_score;
		L->cave[x][y] = Ydiamond_space;
		L->next[x][y] = Xspace;
		L->sound[SOUND_collect] = 1;
		return(1);
	case Xdynamite:
		if(L->dynamite[P] < 9999) L->dynamite[P] += 1;
		L->score += L->dynamite_score;
		L->cave[x][y] = Ydynamite_space;
		L->next[x][y] = Xspace;
		L->sound[SOUND_collect] = 1;
		return(1);
	case Xkey_1: temp = 1; goto key_w;
	case Xkey_2: temp = 2; goto key_w;
	case Xkey_3: temp = 4; goto key_w;
	case Xkey_4: temp = 8; goto key_w;
	case Xkey_5: temp = 16; goto key_w;
	case Xkey_6: temp = 32; goto key_w;
	case Xkey_7: temp = 64; goto key_w;
	case Xkey_8: temp = 128; goto key_w;
	key_w:
		L->keys[P] |= temp;
		L->score += L->key_score;
		L->cave[x][y] = Xpop;
		L->next[x][y] = Xspace;
		L->sound[SOUND_collect] = 1;
		return(1);
	case Xlenses:
		L->lenses_cnt = L->lenses_time;
		L->score += L->lenses_score;
		L->cave[x][y] = Xpop;
		L->next[x][y] = Xspace;
		L->sound[SOUND_collect] = 1;
		return(1);
	case Xmagnify:
		L->magnify_cnt = L->magnify_time;
		L->score += L->magnify_score;
		L->cave[x][y] = Xpop;
		L->next[x][y] = Xspace;
		L->sound[SOUND_collect] = 1;
		return(1);
	}
	return(0);
}

static void Lplayer_move(LOGIC L, int P)
{
	int n;
	L->pl_anim[P] = SPRITE_still;

	L->pl_moved[P] = L->joystick[P][INPUT_north] | L->joystick[P][INPUT_east] | L->joystick[P][INPUT_south] | L->joystick[P][INPUT_west];
	if(!L->pl_moved[P]) {
		if(L->joystick[P][INPUT_shoot]) {
			L->pl_hold[P]++;
			if(L->pl_hold[P] == 5 && L->dynamite[P] > 0) {
				L->cave[L->pl_right[P]][L->pl_bottom[P]] = Xdynamite_1;
				L->sound[SOUND_dynamite] = 1;
				L->dynamite[P]--;
			}
			return;
		}
		RANDOM(0);
	}
	L->pl_hold[P] = 0;

	for(n = 0; n < 4; n++) {
		L->pl_spin[P] = (L->pl_spin[P] + 1) % 4;
		if(!L->joystick[P][INPUT_shoot]) {
			switch(L->pl_spin[P]) {
			case 0:
				if(L->joystick[P][INPUT_north]) if(Lplayer_walk_n(L, P)) return;
				break;
			case 1:
				if(L->joystick[P][INPUT_east]) if(Lplayer_walk_e(L, P)) return;
				break;
			case 2:
				if(L->joystick[P][INPUT_south]) if(Lplayer_walk_s(L, P)) return;
				break;
			case 3:
				if(L->joystick[P][INPUT_west]) if(Lplayer_walk_w(L, P)) return;
				break;
			}
		} else {
			switch(L->pl_spin[P]) {
			case 0:
				if(L->joystick[P][INPUT_north]) if(Lplayer_shoot_n(L, P)) return;
				break;
			case 1:
				if(L->joystick[P][INPUT_east]) if(Lplayer_shoot_e(L, P)) return;
				break;
			case 2:
				if(L->joystick[P][INPUT_south]) if(Lplayer_shoot_s(L, P)) return;
				break;
			case 3:
				if(L->joystick[P][INPUT_west]) if(Lplayer_shoot_w(L, P)) return;
				break;
			}
		}
	}
	L->pl_anim[P] = SPRITE_still;
}

static void Lplayer(LOGIC L)
{
	int x;
	int y;
	int P;
	int n;
	int shuffle[2];

	for(n = 0; n < 2; n++) shuffle[n] = n;
	for(n = 2; n > 1; ) {
		int swap = RANDOM(n);
		int temp = shuffle[--n];
		shuffle[n] = shuffle[swap];
		shuffle[swap] = temp;
	}

	for(P = 0; P < 2; P++) {
		x = L->pl_right[P];
		y = L->pl_bottom[P];
		if(x < L->cave_left) x += L->width;
		if(x > L->cave_right) x -= L->width;
		L->pl_left[P] = x;
		L->pl_top[P] = y;
		L->pl_right[P] = x;
		L->pl_bottom[P] = y;

		if(L->hide[P]) continue;

		if(L->cave[x][y] == Xspace) {
			L->cave[x][y] = Xplayer;
			L->next[x][y] = Xplayer;
		}
	}

	Lplayer_check(L);

	for(n = 0; n < 2; n++) {
		P = shuffle[n];

		if(L->hide[P]) continue;
		if(L->win[P]) continue;

		Lplayer_move(L, P);
	}

	for(P = 0; P < 2; P++) {
		if(L->hide[P]) continue;

		x = L->pl_left[P];
		y = L->pl_top[P];
		if(L->cave[x][y] == Xplayer) {
			L->cave[x][y] = Xspace;
			L->next[x][y] = Xspace;
		}
	}
	for(P = 0; P < 2; P++) {
		if(L->hide[P]) continue;

		x = L->pl_right[P];
		y = L->pl_bottom[P];
		if(L->cave[x][y] == Xspace) {
			L->cave[x][y] = Xplayer;
			L->next[x][y] = Xplayer;
		}
	}

	if(L->diamonds < 0) L->diamonds = 0;
	if(L->score > 9999) L->score = 9999;
}

static void Lchain(LOGIC L, int x, int y)
{
	L->next[x][y] = Zboom;
}
static void Lboom_1(LOGIC L, int x, int y)
{
	L->next[x][y] = Xboom_2;
	L->sound[SOUND_boom] = 1;
}
static void Lboom_2(LOGIC L, int x, int y)
{
	L->next[x][y] = L->boom[x][y];
}

static void Lacid_1(LOGIC L, int x, int y)
{
	L->next[x][y] = Xacid_2;
}
static void Lacid_2(LOGIC L, int x, int y)
{
	L->next[x][y] = Xacid_3;
}
static void Lacid_3(LOGIC L, int x, int y)
{
	L->next[x][y] = Xacid_4;
}
static void Lacid_4(LOGIC L, int x, int y)
{
	L->next[x][y] = Xacid_5;
}
static void Lacid_5(LOGIC L, int x, int y)
{
	L->next[x][y] = Xacid_6;
}
static void Lacid_6(LOGIC L, int x, int y)
{
	L->next[x][y] = Xacid_7;
}
static void Lacid_7(LOGIC L, int x, int y)
{
	L->next[x][y] = Xacid_8;
}
static void Lacid_8(LOGIC L, int x, int y)
{
	L->next[x][y] = Xacid_1;
}

static void Landroid(LOGIC L, int x, int y)
{
	int dx;
	int dy;
	int temp;

	if(L->android_clone_cnt) goto android_move;

	if(is_space[L->cave[x-1][y-1]] + is_space[L->cave[x][y-1]] +
	   is_space[L->cave[x+1][y-1]] + is_space[L->cave[x-1][y]] +
	   is_space[L->cave[x+1][y]] + is_space[L->cave[x-1][y+1]] +
	   is_space[L->cave[x][y+1]] + is_space[L->cave[x+1][y+1]] == 0) goto android_move;

	switch(RANDOM(8)) {
	case 0: temp = L->android_array[L->cave[x][y-1]]; if(temp != Xspace) goto android_clone;
	case 1: temp = L->android_array[L->cave[x-1][y]]; if(temp != Xspace) goto android_clone;
	case 2: temp = L->android_array[L->cave[x+1][y]]; if(temp != Xspace) goto android_clone;
	case 3: temp = L->android_array[L->cave[x][y+1]]; if(temp != Xspace) goto android_clone;
	case 4: temp = L->android_array[L->cave[x-1][y-1]]; if(temp != Xspace) goto android_clone;
	case 5: temp = L->android_array[L->cave[x+1][y-1]]; if(temp != Xspace) goto android_clone;
	case 6: temp = L->android_array[L->cave[x-1][y+1]]; if(temp != Xspace) goto android_clone;
	case 7: temp = L->android_array[L->cave[x+1][y+1]]; if(temp != Xspace) goto android_clone;
	case 8: temp = L->android_array[L->cave[x][y-1]]; if(temp != Xspace) goto android_clone;
	case 9: temp = L->android_array[L->cave[x-1][y]]; if(temp != Xspace) goto android_clone;
	case 10: temp = L->android_array[L->cave[x+1][y]]; if(temp != Xspace) goto android_clone;
	case 11: temp = L->android_array[L->cave[x][y+1]]; if(temp != Xspace) goto android_clone;
	case 12: temp = L->android_array[L->cave[x-1][y-1]]; if(temp != Xspace) goto android_clone;
	case 13: temp = L->android_array[L->cave[x+1][y-1]]; if(temp != Xspace) goto android_clone;
	case 14: temp = L->android_array[L->cave[x-1][y+1]]; if(temp != Xspace) goto android_clone;
	case 15: temp = L->android_array[L->cave[x+1][y+1]]; if(temp != Xspace) goto android_clone;
	}
	goto android_move;

android_clone:

	L->sound[SOUND_android] = 1;

	switch(RANDOM(8)) {
	case 0: if(is_space[L->cave[x][y-1]]) goto android_n;
	case 1: if(is_space[L->cave[x-1][y]]) goto android_w;
	case 2: if(is_space[L->cave[x+1][y]]) goto android_e;
	case 3: if(is_space[L->cave[x][y+1]]) goto android_s;
	case 4: if(is_space[L->cave[x-1][y-1]]) goto android_nw;
	case 5: if(is_space[L->cave[x+1][y-1]]) goto android_ne;
	case 6: if(is_space[L->cave[x-1][y+1]]) goto android_sw;
	case 7: if(is_space[L->cave[x+1][y+1]]) goto android_se;
	case 8: if(is_space[L->cave[x][y-1]]) goto android_n;
	case 9: if(is_space[L->cave[x-1][y]]) goto android_w;
	case 10: if(is_space[L->cave[x+1][y]]) goto android_e;
	case 11: if(is_space[L->cave[x][y+1]]) goto android_s;
	case 12: if(is_space[L->cave[x-1][y-1]]) goto android_nw;
	case 13: if(is_space[L->cave[x+1][y-1]]) goto android_ne;
	case 14: if(is_space[L->cave[x-1][y+1]]) goto android_sw;
	case 15: if(is_space[L->cave[x+1][y+1]]) goto android_se;
	}

android_move:

	if(L->android_move_cnt) return;

	if(L->cave[x-1][y-1] == Xplayer || L->cave[x][y-1] == Xplayer ||
	   L->cave[x+1][y-1] == Xplayer || L->cave[x-1][y] == Xplayer ||
	   L->cave[x+1][y] == Xplayer || L->cave[x-1][y+1] == Xplayer ||
	   L->cave[x][y+1] == Xplayer || L->cave[x+1][y+1] == Xplayer) return;

	temp = Xspace;

	if(L->win[0] == 0 && L->win[1] == 0) {
		int dist;
		dx = L->pl_right[0] - x; if(dx < 0) dx = -dx;
		dy = L->pl_bottom[0] - y; if(dy < 0) dy = -dy;
		dist = dx + dy;
		dx = L->pl_right[1] - x; if(dx < 0) dx = -dx;
		dy = L->pl_bottom[1] - y; if(dy < 0) dy = -dy;
		dist -= dx + dy;
		if(dist < 0) {
			dx = L->pl_right[0];
			dy = L->pl_bottom[0];
		} else {
			dx = L->pl_right[1];
			dy = L->pl_bottom[1];
		}
	} else if(L->win[0] == 0) {
		dx = L->pl_right[0];
		dy = L->pl_bottom[0];
	} else if(L->win[1] == 0) {
		dx = L->pl_right[1];
		dy = L->pl_bottom[1];
	} else {
		dx = 0;
		dy = 0;
	}
	if(RANDOM(2)) {
		if(y > dy) {
			if(x > dx) {
				switch(L->cave[x-1][y-1]) {
				case Xspace:
				case Xsplash_e: case Xsplash_w:
				case Xplant:
					goto android_nw;
				}
				switch(L->cave[x][y-1]) {
				case Xspace:
				case Xsplash_e: case Xsplash_w:
				case Xplant:
					goto android_n;
				}
				switch(L->cave[x-1][y]) {
				case Xspace:
				case Xsplash_e: case Xsplash_w:
				case Xplant:
					goto android_w;
				}
			} else if(x < dx) {
				switch(L->cave[x+1][y-1]) {
				case Xspace:
				case Xsplash_e: case Xsplash_w:
				case Xplant:
					goto android_ne;
				}
				switch(L->cave[x+1][y]) {
				case Xspace:
				case Xsplash_e: case Xsplash_w:
				case Xplant:
					goto android_e;
				}
				switch(L->cave[x][y-1]) {
				case Xspace:
				case Xsplash_e: case Xsplash_w:
				case Xplant:
					goto android_n;
				}
			} else {
				switch(L->cave[x][y-1]) {
				case Xspace:
				case Xsplash_e: case Xsplash_w:
				case Xplant:
					goto android_n;
				}
				switch(L->cave[x+1][y-1]) {
				case Xspace:
				case Xsplash_e: case Xsplash_w:
				case Xplant:
					goto android_ne;
				}
				switch(L->cave[x-1][y-1]) {
				case Xspace:
				case Xsplash_e: case Xsplash_w:
				case Xplant:
					goto android_nw;
				}
			}
		} else if(y < dy) {
			if(x > dx) {
				switch(L->cave[x-1][y+1]) {
				case Xspace:
				case Xsplash_e: case Xsplash_w:
				case Xplant:
					goto android_sw;
				}
				switch(L->cave[x-1][y]) {
				case Xspace:
				case Xsplash_e: case Xsplash_w:
				case Xplant:
					goto android_w;
				}
				switch(L->cave[x][y+1]) {
				case Xspace:
				case Xsplash_e: case Xsplash_w:
				case Xplant:
					goto android_s;
				}
			} else if(x < dx) {
				switch(L->cave[x+1][y+1]) {
				case Xspace:
				case Xsplash_e: case Xsplash_w:
				case Xplant:
					goto android_se;
				}
				switch(L->cave[x][y+1]) {
				case Xspace:
				case Xsplash_e: case Xsplash_w:
				case Xplant:
					goto android_s;
				}
				switch(L->cave[x+1][y]) {
				case Xspace:
				case Xsplash_e: case Xsplash_w:
				case Xplant:
					goto android_e;
				}
			} else {
				switch(L->cave[x][y+1]) {
				case Xspace:
				case Xsplash_e: case Xsplash_w:
				case Xplant:
					goto android_s;
				}
				switch(L->cave[x-1][y+1]) {
				case Xspace:
				case Xsplash_e: case Xsplash_w:
				case Xplant:
					goto android_sw;
				}
				switch(L->cave[x+1][y+1]) {
				case Xspace:
				case Xsplash_e: case Xsplash_w:
				case Xplant:
					goto android_se;
				}
			}
		} else {
			if(x > dx) {
				switch(L->cave[x-1][y]) {
				case Xspace:
				case Xsplash_e: case Xsplash_w:
				case Xplant:
					goto android_w;
				}
				switch(L->cave[x-1][y-1]) {
				case Xspace:
				case Xsplash_e: case Xsplash_w:
				case Xplant:
					goto android_nw;
				}
				switch(L->cave[x-1][y+1]) {
				case Xspace:
				case Xsplash_e: case Xsplash_w:
				case Xplant:
					goto android_sw;
				}
			} else if(x < dx) {
				switch(L->cave[x+1][y]) {
				case Xspace:
				case Xsplash_e: case Xsplash_w:
				case Xplant:
					goto android_e;
				}
				switch(L->cave[x+1][y+1]) {
				case Xspace:
				case Xsplash_e: case Xsplash_w:
				case Xplant:
					goto android_se;
				}
				switch(L->cave[x+1][y-1]) {
				case Xspace:
				case Xsplash_e: case Xsplash_w:
				case Xplant:
					goto android_ne;
				}
			}
		}
	} else {
		if(y > dy) {
			if(x > dx) {
				switch(L->cave[x-1][y-1]) {
				case Xspace:
				case Xsplash_e: case Xsplash_w:
				case Xplant:
					goto android_nw;
				}
				switch(L->cave[x-1][y]) {
				case Xspace:
				case Xsplash_e: case Xsplash_w:
				case Xplant:
					goto android_w;
				}
				switch(L->cave[x][y-1]) {
				case Xspace:
				case Xsplash_e: case Xsplash_w:
				case Xplant:
					goto android_n;
				}
			} else if(x < dx) {
				switch(L->cave[x+1][y-1]) {
				case Xspace:
				case Xsplash_e: case Xsplash_w:
				case Xplant:
					goto android_ne;
				}
				switch(L->cave[x][y-1]) {
				case Xspace:
				case Xsplash_e: case Xsplash_w:
				case Xplant:
					goto android_n;
				}
				switch(L->cave[x+1][y]) {
				case Xspace:
				case Xsplash_e: case Xsplash_w:
				case Xplant:
					goto android_e;
				}
			} else {
				switch(L->cave[x][y-1]) {
				case Xspace:
				case Xsplash_e: case Xsplash_w:
				case Xplant:
					goto android_n;
				}
				switch(L->cave[x-1][y-1]) {
				case Xspace:
				case Xsplash_e: case Xsplash_w:
				case Xplant:
					goto android_nw;
				}
				switch(L->cave[x+1][y-1]) {
				case Xspace:
				case Xsplash_e: case Xsplash_w:
				case Xplant:
					goto android_ne;
				}
			}
		} else if(y < dy) {
			if(x > dx) {
				switch(L->cave[x-1][y+1]) {
				case Xspace:
				case Xsplash_e: case Xsplash_w:
				case Xplant:
					goto android_sw;
				}
				switch(L->cave[x][y+1]) {
				case Xspace:
				case Xsplash_e: case Xsplash_w:
				case Xplant:
					goto android_s;
				}
				switch(L->cave[x-1][y]) {
				case Xspace:
				case Xsplash_e: case Xsplash_w:
				case Xplant:
					goto android_w;
				}
			} else if(x < dx) {
				switch(L->cave[x+1][y+1]) {
				case Xspace:
				case Xsplash_e: case Xsplash_w:
				case Xplant:
					goto android_se;
				}
				switch(L->cave[x+1][y]) {
				case Xspace:
				case Xsplash_e: case Xsplash_w:
				case Xplant:
					goto android_e;
				}
				switch(L->cave[x][y+1]) {
				case Xspace:
				case Xsplash_e: case Xsplash_w:
				case Xplant:
					goto android_s;
				}
			} else {
				switch(L->cave[x][y+1]) {
				case Xspace:
				case Xsplash_e: case Xsplash_w:
				case Xplant:
					goto android_s;
				}
				switch(L->cave[x+1][y+1]) {
				case Xspace:
				case Xsplash_e: case Xsplash_w:
				case Xplant:
					goto android_se;
				}
				switch(L->cave[x-1][y+1]) {
				case Xspace:
				case Xsplash_e: case Xsplash_w:
				case Xplant:
					goto android_sw;
				}
			}
		} else {
			if(x > dx) {
				switch(L->cave[x-1][y]) {
				case Xspace:
				case Xsplash_e: case Xsplash_w:
				case Xplant:
					goto android_w;
				}
				switch(L->cave[x-1][y+1]) {
				case Xspace:
				case Xsplash_e: case Xsplash_w:
				case Xplant:
					goto android_sw;
				}
				switch(L->cave[x-1][y-1]) {
				case Xspace:
				case Xsplash_e: case Xsplash_w:
				case Xplant:
					goto android_nw;
				}
			} else if(x < dx) {
				switch(L->cave[x+1][y]) {
				case Xspace:
				case Xsplash_e: case Xsplash_w:
				case Xplant:
					goto android_e;
				}
				switch(L->cave[x+1][y-1]) {
				case Xspace:
				case Xsplash_e: case Xsplash_w:
				case Xplant:
					goto android_ne;
				}
				switch(L->cave[x+1][y+1]) {
				case Xspace:
				case Xsplash_e: case Xsplash_w:
				case Xplant:
					goto android_se;
				}
			}
		}
	}
	return;

android_n:
	L->cave[x][y] = Yandroid_nB;
	L->next[x][y] = temp;
	L->cave[x][y-1] = Yandroid_n;
	L->next[x][y-1] = Xandroid;
	PLAY(SOUND_tank);
	return;
android_ne:
	L->cave[x][y] = Yandroid_neB;
	L->next[x][y] = temp;
	L->cave[x+1][y-1] = Yandroid_ne;
	L->next[x+1][y-1] = Xandroid;
	PLAY(SOUND_tank);
	return;
android_e:
	L->cave[x][y] = Yandroid_eB;
	L->next[x][y] = temp;
	L->cave[x+1][y] = Yandroid_e;
	L->next[x+1][y] = Xandroid;
	PLAY(SOUND_tank);
	return;
android_se:
	L->cave[x][y] = Yandroid_seB;
	L->next[x][y] = temp;
	L->cave[x+1][y+1] = Yandroid_se;
	L->next[x+1][y+1] = Xandroid;
	PLAY(SOUND_tank);
	return;
android_s:
	L->cave[x][y] = Yandroid_sB;
	L->next[x][y] = temp;
	L->cave[x][y+1] = Yandroid_s;
	L->next[x][y+1] = Xandroid;
	PLAY(SOUND_tank);
	return;
android_sw:
	L->cave[x][y] = Yandroid_swB;
	L->next[x][y] = temp;
	L->cave[x-1][y+1] = Yandroid_sw;
	L->next[x-1][y+1] = Xandroid;
	PLAY(SOUND_tank);
	return;
android_w:
	L->cave[x][y] = Yandroid_wB;
	L->next[x][y] = temp;
	L->cave[x-1][y] = Yandroid_w;
	L->next[x-1][y] = Xandroid;
	PLAY(SOUND_tank);
	return;
android_nw:
	L->cave[x][y] = Yandroid_nwB;
	L->next[x][y] = temp;
	L->cave[x-1][y-1] = Yandroid_nw;
	L->next[x-1][y-1] = Xandroid;
	PLAY(SOUND_tank);
	return;
}
static void Landroid_1_n(LOGIC L, int x, int y)
{
	switch(L->cave[x][y-1]) {
	case Xspace:
	case Xsplash_e: case Xsplash_w:
		L->cave[x][y] = Yandroid_nB;
		L->next[x][y] = Xspace;
		L->cave[x][y-1] = Yandroid_n;
		L->next[x][y-1] = Xandroid;
		PLAY(SOUND_tank);
		return;
	case Xacid_1: case Xacid_2:
	case Xacid_3: case Xacid_4:
	case Xacid_5: case Xacid_6:
	case Xacid_7: case Xacid_8:
		L->cave[x][y] = Yandroid_nB;
		L->next[x][y] = Xspace;
		if(L->cave[x+1][y-2] == Xspace) L->cave[x+1][y-2] = Xsplash_e;
		if(L->cave[x-1][y-2] == Xspace) L->cave[x-1][y-2] = Xsplash_w;
		PLAY(SOUND_acid);
		return;
	}
	Landroid(L, x, y);
}
static void Landroid_2_n(LOGIC L, int x, int y)
{
	switch(L->cave[x][y-1]) {
	case Xspace:
	case Xsplash_e: case Xsplash_w:
		L->cave[x][y] = Yandroid_nB;
		L->next[x][y] = Xspace;
		L->cave[x][y-1] = Yandroid_n;
		L->next[x][y-1] = Xandroid_1_n;
		PLAY(SOUND_tank);
		return;
	case Xacid_1: case Xacid_2:
	case Xacid_3: case Xacid_4:
	case Xacid_5: case Xacid_6:
	case Xacid_7: case Xacid_8:
		L->cave[x][y] = Yandroid_nB;
		L->next[x][y] = Xspace;
		if(L->cave[x+1][y-2] == Xspace) L->cave[x+1][y-2] = Xsplash_e;
		if(L->cave[x-1][y-2] == Xspace) L->cave[x-1][y-2] = Xsplash_w;
		PLAY(SOUND_acid);
		return;
	}
	Landroid(L, x, y);
}
static void Landroid_1_e(LOGIC L, int x, int y)
{
	switch(L->cave[x+1][y]) {
	case Xspace:
	case Xsplash_e: case Xsplash_w:
		L->cave[x][y] = Yandroid_eB;
		L->next[x][y] = Xspace;
		L->cave[x+1][y] = Yandroid_e;
		L->next[x+1][y] = Xandroid;
		PLAY(SOUND_tank);
		return;
	case Xacid_1: case Xacid_2:
	case Xacid_3: case Xacid_4:
	case Xacid_5: case Xacid_6:
	case Xacid_7: case Xacid_8:
		L->cave[x][y] = Yandroid_eB;
		L->next[x][y] = Xspace;
		if(L->cave[x+2][y-1] == Xspace) L->cave[x+2][y-1] = Xsplash_e;
		if(L->cave[x][y-1] == Xspace) L->cave[x][y-1] = Xsplash_w;
		PLAY(SOUND_acid);
		return;
	}
	Landroid(L, x, y);
}
static void Landroid_2_e(LOGIC L, int x, int y)
{
	switch(L->cave[x+1][y]) {
	case Xspace:
	case Xsplash_e: case Xsplash_w:
		L->cave[x][y] = Yandroid_eB;
		L->next[x][y] = Xspace;
		L->cave[x+1][y] = Yandroid_e;
		L->next[x+1][y] = Xandroid_1_e;
		PLAY(SOUND_tank);
		return;
	case Xacid_1: case Xacid_2:
	case Xacid_3: case Xacid_4:
	case Xacid_5: case Xacid_6:
	case Xacid_7: case Xacid_8:
		L->cave[x][y] = Yandroid_eB;
		L->next[x][y] = Xspace;
		if(L->cave[x+2][y-1] == Xspace) L->cave[x+2][y-1] = Xsplash_e;
		if(L->cave[x][y-1] == Xspace) L->cave[x][y-1] = Xsplash_w;
		PLAY(SOUND_acid);
		return;
	}
	Landroid(L, x, y);
}
static void Landroid_1_s(LOGIC L, int x, int y)
{
	switch(L->cave[x][y+1]) {
	case Xspace:
	case Xsplash_e: case Xsplash_w:
		L->cave[x][y] = Yandroid_sB;
		L->next[x][y] = Xspace;
		L->cave[x][y+1] = Yandroid_s;
		L->next[x][y+1] = Xandroid;
		PLAY(SOUND_tank);
		return;
	case Xacid_1: case Xacid_2:
	case Xacid_3: case Xacid_4:
	case Xacid_5: case Xacid_6:
	case Xacid_7: case Xacid_8:
		L->cave[x][y] = Yandroid_sB;
		L->next[x][y] = Xspace;
		if(L->cave[x+1][y] == Xspace) L->cave[x+1][y] = Xsplash_e;
		if(L->cave[x-1][y] == Xspace) L->cave[x-1][y] = Xsplash_w;
		PLAY(SOUND_acid);
		return;
	}
	Landroid(L, x, y);
}
static void Landroid_2_s(LOGIC L, int x, int y)
{
	switch(L->cave[x][y+1]) {
	case Xspace:
	case Xsplash_e: case Xsplash_w:
		L->cave[x][y] = Yandroid_sB;
		L->next[x][y] = Xspace;
		L->cave[x][y+1] = Yandroid_s;
		L->next[x][y+1] = Xandroid_1_s;
		PLAY(SOUND_tank);
		return;
	case Xacid_1: case Xacid_2:
	case Xacid_3: case Xacid_4:
	case Xacid_5: case Xacid_6:
	case Xacid_7: case Xacid_8:
		L->cave[x][y] = Yandroid_sB;
		L->next[x][y] = Xspace;
		if(L->cave[x+1][y] == Xspace) L->cave[x+1][y] = Xsplash_e;
		if(L->cave[x-1][y] == Xspace) L->cave[x-1][y] = Xsplash_w;
		PLAY(SOUND_acid);
		return;
	}
	Landroid(L, x, y);
}
static void Landroid_1_w(LOGIC L, int x, int y)
{
	switch(L->cave[x-1][y]) {
	case Xspace:
	case Xsplash_e: case Xsplash_w:
		L->cave[x][y] = Yandroid_wB;
		L->next[x][y] = Xspace;
		L->cave[x-1][y] = Yandroid_w;
		L->next[x-1][y] = Xandroid;
		PLAY(SOUND_tank);
		return;
	case Xacid_1: case Xacid_2:
	case Xacid_3: case Xacid_4:
	case Xacid_5: case Xacid_6:
	case Xacid_7: case Xacid_8:
		L->cave[x][y] = Yandroid_wB;
		L->next[x][y] = Xspace;
		if(L->cave[x][y-1] == Xspace) L->cave[x][y-1] = Xsplash_e;
		if(L->cave[x-2][y-1] == Xspace) L->cave[x-2][y-1] = Xsplash_w;
		PLAY(SOUND_acid);
		return;
	}
	Landroid(L, x, y);
}
static void Landroid_2_w(LOGIC L, int x, int y)
{
	switch(L->cave[x-1][y]) {
	case Xspace:
	case Xsplash_e: case Xsplash_w:
		L->cave[x][y] = Yandroid_wB;
		L->next[x][y] = Xspace;
		L->cave[x-1][y] = Yandroid_w;
		L->next[x-1][y] = Xandroid_1_w;
		PLAY(SOUND_tank);
		return;
	case Xacid_1: case Xacid_2:
	case Xacid_3: case Xacid_4:
	case Xacid_5: case Xacid_6:
	case Xacid_7: case Xacid_8:
		L->cave[x][y] = Yandroid_wB;
		L->next[x][y] = Xspace;
		if(L->cave[x][y-1] == Xspace) L->cave[x][y-1] = Xsplash_e;
		if(L->cave[x-2][y-1] == Xspace) L->cave[x-2][y-1] = Xsplash_w;
		PLAY(SOUND_acid);
		return;
	}
	Landroid(L, x, y);
}

static void Leater_n(LOGIC L, int x, int y)
{
	if(L->cave[x+1][y] == Xdiamond) {
		L->cave[x+1][y] = Ydiamond_space;
		L->next[x+1][y] = Xspace;
		L->sound[SOUND_eater] = 1;
		return;
	}
	if(L->cave[x][y+1] == Xdiamond) {
		L->cave[x][y+1] = Ydiamond_space;
		L->next[x][y+1] = Xspace;
		L->sound[SOUND_eater] = 1;
		return;
	}
	if(L->cave[x-1][y] == Xdiamond) {
		L->cave[x-1][y] = Ydiamond_space;
		L->next[x-1][y] = Xspace;
		L->sound[SOUND_eater] = 1;
		return;
	}
	switch(L->cave[x][y-1]) {
	case Xplayer:
	case Xspace:
	case Xsplash_e: case Xsplash_w:
	case Xplant:
		L->cave[x][y] = Yeater_nB;
		L->next[x][y] = Xspace;
		L->cave[x][y-1] = Yeater_n;
		L->next[x][y-1] = Xeater_n;
		return;
	case Xacid_1: case Xacid_2:
	case Xacid_3: case Xacid_4:
	case Xacid_5: case Xacid_6:
	case Xacid_7: case Xacid_8:
		L->cave[x][y] = Yeater_nB;
		L->next[x][y] = Xspace;
		if(L->cave[x+1][y-2] == Xspace) L->cave[x+1][y-2] = Xsplash_e;
		if(L->cave[x-1][y-2] == Xspace) L->cave[x-1][y-2] = Xsplash_w;
		PLAY(SOUND_acid);
		return;
	case Xdiamond:
		L->cave[x][y-1] = Ydiamond_space;
		L->next[x][y-1] = Xspace;
		L->sound[SOUND_eater] = 1;
		return;
	}
	L->next[x][y] = RANDOM(2) ? Xeater_e : Xeater_w;
	PLAY(SOUND_eater);
}
static void Leater_e(LOGIC L, int x, int y)
{
	if(L->cave[x][y+1] == Xdiamond) {
		L->cave[x][y+1] = Ydiamond_space;
		L->next[x][y+1] = Xspace;
		L->sound[SOUND_eater] = 1;
		return;
	}
	if(L->cave[x-1][y] == Xdiamond) {
		L->cave[x-1][y] = Ydiamond_space;
		L->next[x-1][y] = Xspace;
		L->sound[SOUND_eater] = 1;
		return;
	}
	if(L->cave[x][y-1] == Xdiamond) {
		L->cave[x][y-1] = Ydiamond_space;
		L->next[x][y-1] = Xspace;
		L->sound[SOUND_eater] = 1;
		return;
	}
	switch(L->cave[x+1][y]) {
	case Xplayer:
	case Xspace:
	case Xsplash_e: case Xsplash_w:
	case Xplant:
		L->cave[x][y] = Yeater_eB;
		L->next[x][y] = Xspace;
		L->cave[x+1][y] = Yeater_e;
		L->next[x+1][y] = Xeater_e;
		return;
	case Xacid_1: case Xacid_2:
	case Xacid_3: case Xacid_4:
	case Xacid_5: case Xacid_6:
	case Xacid_7: case Xacid_8:
		L->cave[x][y] = Yeater_eB;
		L->next[x][y] = Xspace;
		if(L->cave[x+2][y-1] == Xspace) L->cave[x+2][y-1] = Xsplash_e;
		if(L->cave[x][y-1] == Xspace) L->cave[x][y-1] = Xsplash_w;
		PLAY(SOUND_acid);
		return;
	case Xdiamond:
		L->cave[x+1][y] = Ydiamond_space;
		L->next[x+1][y] = Xspace;
		L->sound[SOUND_eater] = 1;
		return;
	}
	L->next[x][y] = RANDOM(2) ? Xeater_n : Xeater_s;
	PLAY(SOUND_eater);
}
static void Leater_s(LOGIC L, int x, int y)
{
	if(L->cave[x-1][y] == Xdiamond) {
		L->cave[x-1][y] = Ydiamond_space;
		L->next[x-1][y] = Xspace;
		L->sound[SOUND_eater] = 1;
		return;
	}
	if(L->cave[x][y-1] == Xdiamond) {
		L->cave[x][y-1] = Ydiamond_space;
		L->next[x][y-1] = Xspace;
		L->sound[SOUND_eater] = 1;
		return;
	}
	if(L->cave[x+1][y] == Xdiamond) {
		L->cave[x+1][y] = Ydiamond_space;
		L->next[x+1][y] = Xspace;
		L->sound[SOUND_eater] = 1;
		return;
	}
	switch(L->cave[x][y+1]) {
	case Xplayer:
	case Xspace:
	case Xsplash_e: case Xsplash_w:
	case Xplant:
		L->cave[x][y] = Yeater_sB;
		L->next[x][y] = Xspace;
		L->cave[x][y+1] = Yeater_s;
		L->next[x][y+1] = Xeater_s;
		return;
	case Xacid_1: case Xacid_2:
	case Xacid_3: case Xacid_4:
	case Xacid_5: case Xacid_6:
	case Xacid_7: case Xacid_8:
		L->cave[x][y] = Yeater_sB;
		L->next[x][y] = Xspace;
		if(L->cave[x+1][y] == Xspace) L->cave[x+1][y] = Xsplash_e;
		if(L->cave[x-1][y] == Xspace) L->cave[x-1][y] = Xsplash_w;
		PLAY(SOUND_acid);
		return;
	case Xdiamond:
		L->cave[x][y+1] = Ydiamond_space;
		L->next[x][y+1] = Xspace;
		L->sound[SOUND_eater] = 1;
		return;
	}
	L->next[x][y] = RANDOM(2) ? Xeater_e : Xeater_w;
	PLAY(SOUND_eater);
}
static void Leater_w(LOGIC L, int x, int y)
{
	if(L->cave[x][y-1] == Xdiamond) {
		L->cave[x][y-1] = Ydiamond_space;
		L->next[x][y-1] = Xspace;
		L->sound[SOUND_eater] = 1;
		return;
	}
	if(L->cave[x+1][y] == Xdiamond) {
		L->cave[x+1][y] = Ydiamond_space;
		L->next[x+1][y] = Xspace;
		L->sound[SOUND_eater] = 1;
		return;
	}
	if(L->cave[x][y+1] == Xdiamond) {
		L->cave[x][y+1] = Ydiamond_space;
		L->next[x][y+1] = Xspace;
		L->sound[SOUND_eater] = 1;
		return;
	}
	switch(L->cave[x-1][y]) {
	case Xplayer:
	case Xspace:
	case Xsplash_e: case Xsplash_w:
	case Xplant:
		L->cave[x][y] = Yeater_wB;
		L->next[x][y] = Xspace;
		L->cave[x-1][y] = Yeater_w;
		L->next[x-1][y] = Xeater_w;
		return;
	case Xacid_1: case Xacid_2:
	case Xacid_3: case Xacid_4:
	case Xacid_5: case Xacid_6:
	case Xacid_7: case Xacid_8:
		L->cave[x][y] = Yeater_wB;
		L->next[x][y] = Xspace;
		if(L->cave[x][y-1] == Xspace) L->cave[x][y-1] = Xsplash_e;
		if(L->cave[x-2][y-1] == Xspace) L->cave[x-2][y-1] = Xsplash_w;
		PLAY(SOUND_acid);
		return;
	case Xdiamond:
		L->cave[x-1][y] = Ydiamond_space;
		L->next[x-1][y] = Xspace;
		L->sound[SOUND_eater] = 1;
		return;
	}
	L->next[x][y] = RANDOM(2) ? Xeater_n : Xeater_s;
	PLAY(SOUND_eater);
}

static void Lalien(LOGIC L, int x, int y)
{
	int dx;
	int dy;
	if(L->wheel_cnt) {
		dx = L->wheel_x;
		dy = L->wheel_y;
	} else if(L->win[0] == 0 && L->win[1] == 0) {
		int dist;
		dx = L->pl_right[0] - x; if(dx < 0) dx = -dx;
		dy = L->pl_bottom[0] - y; if(dy < 0) dy = -dy;
		dist = dx + dy;
		dx = L->pl_right[1] - x; if(dx < 0) dx = -dx;
		dy = L->pl_bottom[1] - y; if(dy < 0) dy = -dy;
		dist -= dx + dy;
		if(dist < 0) {
			dx = L->pl_right[0];
			dy = L->pl_bottom[0];
		} else {
			dx = L->pl_right[1];
			dy = L->pl_bottom[1];
		}
	} else if(L->win[0] == 0) {
		dx = L->pl_right[0];
		dy = L->pl_bottom[0];
	} else if(L->win[1] == 0) {
		dx = L->pl_right[1];
		dy = L->pl_bottom[1];
	} else {
		dx = 0;
		dy = 0;
	}
	if(RANDOM(2)) {
		if(y > dy) {
			switch(L->cave[x][y-1]) {
			case Xplayer:
			case Xspace:
			case Xsplash_e: case Xsplash_w:
			case Xplant:
				L->cave[x][y] = Yalien_nB;
				L->next[x][y] = Xspace;
				L->cave[x][y-1] = Yalien_n;
				L->next[x][y-1] = Xalien_pause;
				PLAY(SOUND_alien);
				return;
			case Xacid_1: case Xacid_2:
			case Xacid_3: case Xacid_4:
			case Xacid_5: case Xacid_6:
			case Xacid_7: case Xacid_8:
				L->cave[x][y] = Yalien_nB;
				L->next[x][y] = Xspace;
				if(L->cave[x+1][y-2] == Xspace) L->cave[x+1][y-2] = Xsplash_e;
				if(L->cave[x-1][y-2] == Xspace) L->cave[x-1][y-2] = Xsplash_w;
				PLAY(SOUND_acid);
				return;
			}
		} else if(y < dy) {
			switch(L->cave[x][y+1]) {
			case Xplayer:
			case Xspace:
			case Xsplash_e: case Xsplash_w:
			case Xplant:
				L->cave[x][y] = Yalien_sB;
				L->next[x][y] = Xspace;
				L->cave[x][y+1] = Yalien_s;
				L->next[x][y+1] = Xalien_pause;
				PLAY(SOUND_alien);
				return;
			case Xacid_1: case Xacid_2:
			case Xacid_3: case Xacid_4:
			case Xacid_5: case Xacid_6:
			case Xacid_7: case Xacid_8:
				L->cave[x][y] = Yalien_sB;
				L->next[x][y] = Xspace;
				if(L->cave[x+1][y] == Xspace) L->cave[x+1][y] = Xsplash_e;
				if(L->cave[x-1][y] == Xspace) L->cave[x-1][y] = Xsplash_w;
				PLAY(SOUND_acid);
				return;
			}
		}
	} else {
		if(x < dx) {
			switch(L->cave[x+1][y]) {
			case Xplayer:
			case Xspace:
			case Xsplash_e: case Xsplash_w:
			case Xplant:
				L->cave[x][y] = Yalien_eB;
				L->next[x][y] = Xspace;
				L->cave[x+1][y] = Yalien_e;
				L->next[x+1][y] = Xalien_pause;
				PLAY(SOUND_alien);
				return;
			case Xacid_1: case Xacid_2:
			case Xacid_3: case Xacid_4:
			case Xacid_5: case Xacid_6:
			case Xacid_7: case Xacid_8:
				L->cave[x][y] = Yalien_eB;
				L->next[x][y] = Xspace;
				if(L->cave[x+2][y-1] == Xspace) L->cave[x+2][y-1] = Xsplash_e;
				if(L->cave[x][y-1] == Xspace) L->cave[x][y-1] = Xsplash_w;
				PLAY(SOUND_acid);
				return;
			}
		} else if(x > dx) {
			switch(L->cave[x-1][y]) {
			case Xplayer:
			case Xspace:
			case Xsplash_e: case Xsplash_w:
			case Xplant:
				L->cave[x][y] = Yalien_wB;
				L->next[x][y] = Xspace;
				L->cave[x-1][y] = Yalien_w;
				L->next[x-1][y] = Xalien_pause;
				PLAY(SOUND_alien);
				return;
			case Xacid_1: case Xacid_2:
			case Xacid_3: case Xacid_4:
			case Xacid_5: case Xacid_6:
			case Xacid_7: case Xacid_8:
				L->cave[x][y] = Yalien_wB;
				L->next[x][y] = Xspace;
				if(L->cave[x][y-1] == Xspace) L->cave[x][y-1] = Xsplash_e;
				if(L->cave[x-2][y-1] == Xspace) L->cave[x-2][y-1] = Xsplash_w;
				PLAY(SOUND_acid);
				return;
			}
		}
	}
}
static void Lalien_pause(LOGIC L, int x, int y)
{
	L->cave[x][y] = Xalien;
	L->next[x][y] = Xalien;
}

static void Lbug_n(LOGIC L, int x, int y)
{
	switch(L->cave[x][y-1]) {
	case Xplayer:
	case Xspace:
	case Xsplash_e: case Xsplash_w:
	case Xplant:
		L->cave[x][y] = Ybug_nB;
		L->next[x][y] = Xspace;
		L->cave[x][y-1] = Ybug_n;
		L->next[x][y-1] = Xbug_1_n;
		PLAY(SOUND_bug);
		return;
	case Xacid_1: case Xacid_2:
	case Xacid_3: case Xacid_4:
	case Xacid_5: case Xacid_6:
	case Xacid_7: case Xacid_8:
		L->cave[x][y] = Ybug_nB;
		L->next[x][y] = Xspace;
		if(L->cave[x+1][y-2] == Xspace) L->cave[x+1][y-2] = Xsplash_e;
		if(L->cave[x-1][y-2] == Xspace) L->cave[x-1][y-2] = Xsplash_w;
		PLAY(SOUND_acid);
		return;
	}
	L->cave[x][y] = Ybug_n_w;
	L->next[x][y] = Xbug_2_w;
	PLAY(SOUND_bug);
}
static void Lbug_1_n(LOGIC L, int x, int y)
{
	switch(L->cave[x][y-1]) {
	case Xameuba_1: case Xameuba_2:
	case Xameuba_3: case Xameuba_4:
	case Xameuba_5: case Xameuba_6:
	case Xameuba_7: case Xameuba_8:
	case Xfake_ameuba: case Yfake_ameuba:
		L->next[x][y] = Zbug;
		return;
	}
	switch(L->cave[x][y+1]) {
	case Xameuba_1: case Xameuba_2:
	case Xameuba_3: case Xameuba_4:
	case Xameuba_5: case Xameuba_6:
	case Xameuba_7: case Xameuba_8:
	case Xfake_ameuba: case Yfake_ameuba:
		L->next[x][y] = Zbug;
		return;
	}
	switch(L->cave[x-1][y]) {
	case Xameuba_1: case Xameuba_2:
	case Xameuba_3: case Xameuba_4:
	case Xameuba_5: case Xameuba_6:
	case Xameuba_7: case Xameuba_8:
	case Xfake_ameuba: case Yfake_ameuba:
		L->next[x][y] = Zbug;
		return;
	}
	switch(L->cave[x+1][y]) {
	case Xplayer:
	case Xspace:
	case Xsplash_e: case Xsplash_w:
	case Xplant:
	case Xacid_1: case Xacid_2:
	case Xacid_3: case Xacid_4:
	case Xacid_5: case Xacid_6:
	case Xacid_7: case Xacid_8:
		L->cave[x][y] = Ybug_n_e;
		L->next[x][y] = Xbug_2_e;
		PLAY(SOUND_bug);
		return;
	case Xameuba_1: case Xameuba_2:
	case Xameuba_3: case Xameuba_4:
	case Xameuba_5: case Xameuba_6:
	case Xameuba_7: case Xameuba_8:
	case Xfake_ameuba: case Yfake_ameuba:
		L->next[x][y] = Zbug;
		return;
	}
	Lbug_n(L, x, y);
}
static void Lbug_2_n(LOGIC L, int x, int y)
{
	switch(L->cave[x][y-1]) {
	case Xameuba_1: case Xameuba_2:
	case Xameuba_3: case Xameuba_4:
	case Xameuba_5: case Xameuba_6:
	case Xameuba_7: case Xameuba_8:
	case Xfake_ameuba: case Yfake_ameuba:
		L->next[x][y] = Zbug;
		return;
	}
	switch(L->cave[x+1][y]) {
	case Xameuba_1: case Xameuba_2:
	case Xameuba_3: case Xameuba_4:
	case Xameuba_5: case Xameuba_6:
	case Xameuba_7: case Xameuba_8:
	case Xfake_ameuba: case Yfake_ameuba:
		L->next[x][y] = Zbug;
		return;
	}
	switch(L->cave[x][y+1]) {
	case Xameuba_1: case Xameuba_2:
	case Xameuba_3: case Xameuba_4:
	case Xameuba_5: case Xameuba_6:
	case Xameuba_7: case Xameuba_8:
	case Xfake_ameuba: case Yfake_ameuba:
		L->next[x][y] = Zbug;
		return;
	}
	switch(L->cave[x-1][y]) {
	case Xameuba_1: case Xameuba_2:
	case Xameuba_3: case Xameuba_4:
	case Xameuba_5: case Xameuba_6:
	case Xameuba_7: case Xameuba_8:
	case Xfake_ameuba: case Yfake_ameuba:
		L->next[x][y] = Zbug;
		return;
	}
	Lbug_n(L, x, y);
}
static void Lbug_e(LOGIC L, int x, int y)
{
	switch(L->cave[x+1][y]) {
	case Xplayer:
	case Xspace:
	case Xsplash_e: case Xsplash_w:
	case Xplant:
		L->cave[x][y] = Ybug_eB;
		L->next[x][y] = Xspace;
		L->cave[x+1][y] = Ybug_e;
		L->next[x+1][y] = Xbug_1_e;
		PLAY(SOUND_bug);
		return;
	case Xacid_1: case Xacid_2:
	case Xacid_3: case Xacid_4:
	case Xacid_5: case Xacid_6:
	case Xacid_7: case Xacid_8:
		L->cave[x][y] = Ybug_eB;
		L->next[x][y] = Xspace;
		if(L->cave[x+2][y-1] == Xspace) L->cave[x+2][y-1] = Xsplash_e;
		if(L->cave[x][y-1] == Xspace) L->cave[x][y-1] = Xsplash_w;
		PLAY(SOUND_acid);
		return;
	}
	L->cave[x][y] = Ybug_e_n;
	L->next[x][y] = Xbug_2_n;
	PLAY(SOUND_bug);
}
static void Lbug_1_e(LOGIC L, int x, int y)
{
	switch(L->cave[x][y-1]) {
	case Xameuba_1: case Xameuba_2:
	case Xameuba_3: case Xameuba_4:
	case Xameuba_5: case Xameuba_6:
	case Xameuba_7: case Xameuba_8:
	case Xfake_ameuba: case Yfake_ameuba:
		L->next[x][y] = Zbug;
		return;
	}
	switch(L->cave[x+1][y]) {
	case Xameuba_1: case Xameuba_2:
	case Xameuba_3: case Xameuba_4:
	case Xameuba_5: case Xameuba_6:
	case Xameuba_7: case Xameuba_8:
	case Xfake_ameuba: case Yfake_ameuba:
		L->next[x][y] = Zbug;
		return;
	}
	switch(L->cave[x-1][y]) {
	case Xameuba_1: case Xameuba_2:
	case Xameuba_3: case Xameuba_4:
	case Xameuba_5: case Xameuba_6:
	case Xameuba_7: case Xameuba_8:
	case Xfake_ameuba: case Yfake_ameuba:
		L->next[x][y] = Zbug;
		return;
	}
	switch(L->cave[x][y+1]) {
	case Xplayer:
	case Xspace:
	case Xsplash_e: case Xsplash_w:
	case Xplant:
	case Xacid_1: case Xacid_2:
	case Xacid_3: case Xacid_4:
	case Xacid_5: case Xacid_6:
	case Xacid_7: case Xacid_8:
		L->cave[x][y] = Ybug_e_s;
		L->next[x][y] = Xbug_2_s;
		PLAY(SOUND_bug);
		return;
	case Xameuba_1: case Xameuba_2:
	case Xameuba_3: case Xameuba_4:
	case Xameuba_5: case Xameuba_6:
	case Xameuba_7: case Xameuba_8:
	case Xfake_ameuba: case Yfake_ameuba:
		L->next[x][y] = Zbug;
		return;
	}
	Lbug_e(L, x, y);
}
static void Lbug_2_e(LOGIC L, int x, int y)
{
	switch(L->cave[x][y-1]) {
	case Xameuba_1: case Xameuba_2:
	case Xameuba_3: case Xameuba_4:
	case Xameuba_5: case Xameuba_6:
	case Xameuba_7: case Xameuba_8:
	case Xfake_ameuba: case Yfake_ameuba:
		L->next[x][y] = Zbug;
		return;
	}
	switch(L->cave[x+1][y]) {
	case Xameuba_1: case Xameuba_2:
	case Xameuba_3: case Xameuba_4:
	case Xameuba_5: case Xameuba_6:
	case Xameuba_7: case Xameuba_8:
	case Xfake_ameuba: case Yfake_ameuba:
		L->next[x][y] = Zbug;
		return;
	}
	switch(L->cave[x][y+1]) {
	case Xameuba_1: case Xameuba_2:
	case Xameuba_3: case Xameuba_4:
	case Xameuba_5: case Xameuba_6:
	case Xameuba_7: case Xameuba_8:
	case Xfake_ameuba: case Yfake_ameuba:
		L->next[x][y] = Zbug;
		return;
	}
	switch(L->cave[x-1][y]) {
	case Xameuba_1: case Xameuba_2:
	case Xameuba_3: case Xameuba_4:
	case Xameuba_5: case Xameuba_6:
	case Xameuba_7: case Xameuba_8:
	case Xfake_ameuba: case Yfake_ameuba:
		L->next[x][y] = Zbug;
		return;
	}
	Lbug_e(L, x, y);
}
static void Lbug_s(LOGIC L, int x, int y)
{
	switch(L->cave[x][y+1]) {
	case Xplayer:
	case Xspace:
	case Xsplash_e: case Xsplash_w:
	case Xplant:
		L->cave[x][y] = Ybug_sB;
		L->next[x][y] = Xspace;
		L->cave[x][y+1] = Ybug_s;
		L->next[x][y+1] = Xbug_1_s;
		PLAY(SOUND_bug);
		return;
	case Xacid_1: case Xacid_2:
	case Xacid_3: case Xacid_4:
	case Xacid_5: case Xacid_6:
	case Xacid_7: case Xacid_8:
		L->cave[x][y] = Ybug_sB;
		L->next[x][y] = Xspace;
		if(L->cave[x+1][y] == Xspace) L->cave[x+1][y] = Xsplash_e;
		if(L->cave[x-1][y] == Xspace) L->cave[x-1][y] = Xsplash_w;
		PLAY(SOUND_acid);
		return;
	}
	L->cave[x][y] = Ybug_s_e;
	L->next[x][y] = Xbug_2_e;
	PLAY(SOUND_bug);
}
static void Lbug_1_s(LOGIC L, int x, int y)
{
	switch(L->cave[x][y-1]) {
	case Xameuba_1: case Xameuba_2:
	case Xameuba_3: case Xameuba_4:
	case Xameuba_5: case Xameuba_6:
	case Xameuba_7: case Xameuba_8:
	case Xfake_ameuba: case Yfake_ameuba:
		L->next[x][y] = Zbug;
		return;
	}
	switch(L->cave[x+1][y]) {
	case Xameuba_1: case Xameuba_2:
	case Xameuba_3: case Xameuba_4:
	case Xameuba_5: case Xameuba_6:
	case Xameuba_7: case Xameuba_8:
	case Xfake_ameuba: case Yfake_ameuba:
		L->next[x][y] = Zbug;
		return;
	}
	switch(L->cave[x][y+1]) {
	case Xameuba_1: case Xameuba_2:
	case Xameuba_3: case Xameuba_4:
	case Xameuba_5: case Xameuba_6:
	case Xameuba_7: case Xameuba_8:
	case Xfake_ameuba: case Yfake_ameuba:
		L->next[x][y] = Zbug;
		return;
	}
	switch(L->cave[x-1][y]) {
	case Xplayer:
	case Xspace:
	case Xsplash_e: case Xsplash_w:
	case Xplant:
	case Xacid_1: case Xacid_2:
	case Xacid_3: case Xacid_4:
	case Xacid_5: case Xacid_6:
	case Xacid_7: case Xacid_8:
		L->cave[x][y] = Ybug_s_w;
		L->next[x][y] = Xbug_2_w;
		PLAY(SOUND_bug);
		return;
	case Xameuba_1: case Xameuba_2:
	case Xameuba_3: case Xameuba_4:
	case Xameuba_5: case Xameuba_6:
	case Xameuba_7: case Xameuba_8:
	case Xfake_ameuba: case Yfake_ameuba:
		L->next[x][y] = Zbug;
		return;
	}
	Lbug_s(L, x, y);
}
static void Lbug_2_s(LOGIC L, int x, int y)
{
	switch(L->cave[x][y-1]) {
	case Xameuba_1: case Xameuba_2:
	case Xameuba_3: case Xameuba_4:
	case Xameuba_5: case Xameuba_6:
	case Xameuba_7: case Xameuba_8:
	case Xfake_ameuba: case Yfake_ameuba:
		L->next[x][y] = Zbug;
		return;
	}
	switch(L->cave[x+1][y]) {
	case Xameuba_1: case Xameuba_2:
	case Xameuba_3: case Xameuba_4:
	case Xameuba_5: case Xameuba_6:
	case Xameuba_7: case Xameuba_8:
	case Xfake_ameuba: case Yfake_ameuba:
		L->next[x][y] = Zbug;
		return;
	}
	switch(L->cave[x][y+1]) {
	case Xameuba_1: case Xameuba_2:
	case Xameuba_3: case Xameuba_4:
	case Xameuba_5: case Xameuba_6:
	case Xameuba_7: case Xameuba_8:
	case Xfake_ameuba: case Yfake_ameuba:
		L->next[x][y] = Zbug;
		return;
	}
	switch(L->cave[x-1][y]) {
	case Xameuba_1: case Xameuba_2:
	case Xameuba_3: case Xameuba_4:
	case Xameuba_5: case Xameuba_6:
	case Xameuba_7: case Xameuba_8:
	case Xfake_ameuba: case Yfake_ameuba:
		L->next[x][y] = Zbug;
		return;
	}
	Lbug_s(L, x, y);
}
static void Lbug_w(LOGIC L, int x, int y)
{
	switch(L->cave[x-1][y]) {
	case Xplayer:
	case Xspace:
	case Xsplash_e: case Xsplash_w:
	case Xplant:
		L->cave[x][y] = Ybug_wB;
		L->next[x][y] = Xspace;
		L->cave[x-1][y] = Ybug_w;
		L->next[x-1][y] = Xbug_1_w;
		PLAY(SOUND_bug);
		return;
	case Xacid_1: case Xacid_2:
	case Xacid_3: case Xacid_4:
	case Xacid_5: case Xacid_6:
	case Xacid_7: case Xacid_8:
		L->cave[x][y] = Ybug_wB;
		L->next[x][y] = Xspace;
		if(L->cave[x][y-1] == Xspace) L->cave[x][y-1] = Xsplash_e;
		if(L->cave[x-2][y-1] == Xspace) L->cave[x-2][y-1] = Xsplash_w;
		PLAY(SOUND_acid);
		return;
	}
	L->cave[x][y] = Ybug_w_s;
	L->next[x][y] = Xbug_2_s;
	PLAY(SOUND_bug);
}
static void Lbug_1_w(LOGIC L, int x, int y)
{
	switch(L->cave[x+1][y]) {
	case Xameuba_1: case Xameuba_2:
	case Xameuba_3: case Xameuba_4:
	case Xameuba_5: case Xameuba_6:
	case Xameuba_7: case Xameuba_8:
	case Xfake_ameuba: case Yfake_ameuba:
		L->next[x][y] = Zbug;
		return;
	}
	switch(L->cave[x][y+1]) {
	case Xameuba_1: case Xameuba_2:
	case Xameuba_3: case Xameuba_4:
	case Xameuba_5: case Xameuba_6:
	case Xameuba_7: case Xameuba_8:
	case Xfake_ameuba: case Yfake_ameuba:
		L->next[x][y] = Zbug;
		return;
	}
	switch(L->cave[x-1][y]) {
	case Xameuba_1: case Xameuba_2:
	case Xameuba_3: case Xameuba_4:
	case Xameuba_5: case Xameuba_6:
	case Xameuba_7: case Xameuba_8:
	case Xfake_ameuba: case Yfake_ameuba:
		L->next[x][y] = Zbug;
		return;
	}
	switch(L->cave[x][y-1]) {
	case Xplayer:
	case Xspace:
	case Xsplash_e: case Xsplash_w:
	case Xplant:
	case Xacid_1: case Xacid_2:
	case Xacid_3: case Xacid_4:
	case Xacid_5: case Xacid_6:
	case Xacid_7: case Xacid_8:
		L->cave[x][y] = Ybug_w_n;
		L->next[x][y] = Xbug_2_n;
		PLAY(SOUND_bug);
		return;
	case Xameuba_1: case Xameuba_2:
	case Xameuba_3: case Xameuba_4:
	case Xameuba_5: case Xameuba_6:
	case Xameuba_7: case Xameuba_8:
	case Xfake_ameuba: case Yfake_ameuba:
		L->next[x][y] = Zbug;
		return;
	}
	Lbug_w(L, x, y);
}
static void Lbug_2_w(LOGIC L, int x, int y)
{
	switch(L->cave[x][y-1]) {
	case Xameuba_1: case Xameuba_2:
	case Xameuba_3: case Xameuba_4:
	case Xameuba_5: case Xameuba_6:
	case Xameuba_7: case Xameuba_8:
	case Xfake_ameuba: case Yfake_ameuba:
		L->next[x][y] = Zbug;
		return;
	}
	switch(L->cave[x+1][y]) {
	case Xameuba_1: case Xameuba_2:
	case Xameuba_3: case Xameuba_4:
	case Xameuba_5: case Xameuba_6:
	case Xameuba_7: case Xameuba_8:
	case Xfake_ameuba: case Yfake_ameuba:
		L->next[x][y] = Zbug;
		return;
	}
	switch(L->cave[x][y+1]) {
	case Xameuba_1: case Xameuba_2:
	case Xameuba_3: case Xameuba_4:
	case Xameuba_5: case Xameuba_6:
	case Xameuba_7: case Xameuba_8:
	case Xfake_ameuba: case Yfake_ameuba:
		L->next[x][y] = Zbug;
		return;
	}
	switch(L->cave[x-1][y]) {
	case Xameuba_1: case Xameuba_2:
	case Xameuba_3: case Xameuba_4:
	case Xameuba_5: case Xameuba_6:
	case Xameuba_7: case Xameuba_8:
	case Xfake_ameuba: case Yfake_ameuba:
		L->next[x][y] = Zbug;
		return;
	}
	Lbug_w(L, x, y);
}

static void Ltank_n(LOGIC L, int x, int y)
{
	switch(L->cave[x][y-1]) {
	case Xplayer:
	case Xspace:
	case Xsplash_e: case Xsplash_w:
	case Xplant:
		L->cave[x][y] = Ytank_nB;
		L->next[x][y] = Xspace;
		L->cave[x][y-1] = Ytank_n;
		L->next[x][y-1] = Xtank_1_n;
		PLAY(SOUND_tank);
		return;
	case Xacid_1: case Xacid_2:
	case Xacid_3: case Xacid_4:
	case Xacid_5: case Xacid_6:
	case Xacid_7: case Xacid_8:
		L->cave[x][y] = Ytank_nB;
		L->next[x][y] = Xspace;
		if(L->cave[x+1][y-2] == Xspace) L->cave[x+1][y-2] = Xsplash_e;
		if(L->cave[x-1][y-2] == Xspace) L->cave[x-1][y-2] = Xsplash_w;
		PLAY(SOUND_acid);
		return;
	}
	L->cave[x][y] = Ytank_n_e;
	L->next[x][y] = Xtank_2_e;
	PLAY(SOUND_tank);
}
static void Ltank_1_n(LOGIC L, int x, int y)
{
	switch(L->cave[x][y-1]) {
	case Xameuba_1: case Xameuba_2:
	case Xameuba_3: case Xameuba_4:
	case Xameuba_5: case Xameuba_6:
	case Xameuba_7: case Xameuba_8:
	case Xfake_ameuba: case Yfake_ameuba:
		L->next[x][y] = Ztank;
		return;
	}
	switch(L->cave[x+1][y]) {
	case Xameuba_1: case Xameuba_2:
	case Xameuba_3: case Xameuba_4:
	case Xameuba_5: case Xameuba_6:
	case Xameuba_7: case Xameuba_8:
	case Xfake_ameuba: case Yfake_ameuba:
		L->next[x][y] = Ztank;
		return;
	}
	switch(L->cave[x][y+1]) {
	case Xameuba_1: case Xameuba_2:
	case Xameuba_3: case Xameuba_4:
	case Xameuba_5: case Xameuba_6:
	case Xameuba_7: case Xameuba_8:
	case Xfake_ameuba: case Yfake_ameuba:
		L->next[x][y] = Ztank;
		return;
	}
	switch(L->cave[x-1][y]) {
	case Xplayer:
	case Xspace:
	case Xsplash_e: case Xsplash_w:
	case Xplant:
	case Xacid_1: case Xacid_2:
	case Xacid_3: case Xacid_4:
	case Xacid_5: case Xacid_6:
	case Xacid_7: case Xacid_8:
		L->cave[x][y] = Ytank_n_w;
		L->next[x][y] = Xtank_2_w;
		PLAY(SOUND_tank);
		return;
	case Xameuba_1: case Xameuba_2:
	case Xameuba_3: case Xameuba_4:
	case Xameuba_5: case Xameuba_6:
	case Xameuba_7: case Xameuba_8:
	case Xfake_ameuba: case Yfake_ameuba:
		L->next[x][y] = Ztank;
		return;
	}
	Ltank_n(L, x, y);
}
static void Ltank_2_n(LOGIC L, int x, int y)
{
	switch(L->cave[x][y-1]) {
	case Xameuba_1: case Xameuba_2:
	case Xameuba_3: case Xameuba_4:
	case Xameuba_5: case Xameuba_6:
	case Xameuba_7: case Xameuba_8:
	case Xfake_ameuba: case Yfake_ameuba:
		L->next[x][y] = Ztank;
		return;
	}
	switch(L->cave[x+1][y]) {
	case Xameuba_1: case Xameuba_2:
	case Xameuba_3: case Xameuba_4:
	case Xameuba_5: case Xameuba_6:
	case Xameuba_7: case Xameuba_8:
	case Xfake_ameuba: case Yfake_ameuba:
		L->next[x][y] = Ztank;
		return;
	}
	switch(L->cave[x][y+1]) {
	case Xameuba_1: case Xameuba_2:
	case Xameuba_3: case Xameuba_4:
	case Xameuba_5: case Xameuba_6:
	case Xameuba_7: case Xameuba_8:
	case Xfake_ameuba: case Yfake_ameuba:
		L->next[x][y] = Ztank;
		return;
	}
	switch(L->cave[x-1][y]) {
	case Xameuba_1: case Xameuba_2:
	case Xameuba_3: case Xameuba_4:
	case Xameuba_5: case Xameuba_6:
	case Xameuba_7: case Xameuba_8:
	case Xfake_ameuba: case Yfake_ameuba:
		L->next[x][y] = Ztank;
		return;
	}
	Ltank_n(L, x, y);
}
static void Ltank_e(LOGIC L, int x, int y)
{
	switch(L->cave[x+1][y]) {
	case Xplayer:
	case Xspace:
	case Xsplash_e: case Xsplash_w:
	case Xplant:
		L->cave[x][y] = Ytank_eB;
		L->next[x][y] = Xspace;
		L->cave[x+1][y] = Ytank_e;
		L->next[x+1][y] = Xtank_1_e;
		PLAY(SOUND_tank);
		return;
	case Xacid_1: case Xacid_2:
	case Xacid_3: case Xacid_4:
	case Xacid_5: case Xacid_6:
	case Xacid_7: case Xacid_8:
		L->cave[x][y] = Ytank_eB;
		L->next[x][y] = Xspace;
		if(L->cave[x+2][y-1] == Xspace) L->cave[x+2][y-1] = Xsplash_e;
		if(L->cave[x][y-1] == Xspace) L->cave[x][y-1] = Xsplash_w;
		PLAY(SOUND_acid);
		return;
	}
	L->cave[x][y] = Ytank_e_s;
	L->next[x][y] = Xtank_2_s;
	PLAY(SOUND_tank);
}
static void Ltank_1_e(LOGIC L, int x, int y)
{
	switch(L->cave[x+1][y]) {
	case Xameuba_1: case Xameuba_2:
	case Xameuba_3: case Xameuba_4:
	case Xameuba_5: case Xameuba_6:
	case Xameuba_7: case Xameuba_8:
	case Xfake_ameuba: case Yfake_ameuba:
		L->next[x][y] = Ztank;
		return;
	}
	switch(L->cave[x][y+1]) {
	case Xameuba_1: case Xameuba_2:
	case Xameuba_3: case Xameuba_4:
	case Xameuba_5: case Xameuba_6:
	case Xameuba_7: case Xameuba_8:
	case Xfake_ameuba: case Yfake_ameuba:
		L->next[x][y] = Ztank;
		return;
	}
	switch(L->cave[x-1][y]) {
	case Xameuba_1: case Xameuba_2:
	case Xameuba_3: case Xameuba_4:
	case Xameuba_5: case Xameuba_6:
	case Xameuba_7: case Xameuba_8:
	case Xfake_ameuba: case Yfake_ameuba:
		L->next[x][y] = Ztank;
		return;
	}
	switch(L->cave[x][y-1]) {
	case Xplayer:
	case Xspace:
	case Xsplash_e: case Xsplash_w:
	case Xplant:
	case Xacid_1: case Xacid_2:
	case Xacid_3: case Xacid_4:
	case Xacid_5: case Xacid_6:
	case Xacid_7: case Xacid_8:
		L->cave[x][y] = Ytank_e_n;
		L->next[x][y] = Xtank_2_n;
		PLAY(SOUND_tank);
		return;
	case Xameuba_1: case Xameuba_2:
	case Xameuba_3: case Xameuba_4:
	case Xameuba_5: case Xameuba_6:
	case Xameuba_7: case Xameuba_8:
	case Xfake_ameuba: case Yfake_ameuba:
		L->next[x][y] = Ztank;
		return;
	}
	Ltank_e(L, x, y);
}
static void Ltank_2_e(LOGIC L, int x, int y)
{
	switch(L->cave[x][y-1]) {
	case Xameuba_1: case Xameuba_2:
	case Xameuba_3: case Xameuba_4:
	case Xameuba_5: case Xameuba_6:
	case Xameuba_7: case Xameuba_8:
	case Xfake_ameuba: case Yfake_ameuba:
		L->next[x][y] = Ztank;
		return;
	}
	switch(L->cave[x+1][y]) {
	case Xameuba_1: case Xameuba_2:
	case Xameuba_3: case Xameuba_4:
	case Xameuba_5: case Xameuba_6:
	case Xameuba_7: case Xameuba_8:
	case Xfake_ameuba: case Yfake_ameuba:
		L->next[x][y] = Ztank;
		return;
	}
	switch(L->cave[x][y+1]) {
	case Xameuba_1: case Xameuba_2:
	case Xameuba_3: case Xameuba_4:
	case Xameuba_5: case Xameuba_6:
	case Xameuba_7: case Xameuba_8:
	case Xfake_ameuba: case Yfake_ameuba:
		L->next[x][y] = Ztank;
		return;
	}
	switch(L->cave[x-1][y]) {
	case Xameuba_1: case Xameuba_2:
	case Xameuba_3: case Xameuba_4:
	case Xameuba_5: case Xameuba_6:
	case Xameuba_7: case Xameuba_8:
	case Xfake_ameuba: case Yfake_ameuba:
		L->next[x][y] = Ztank;
		return;
	}
	Ltank_e(L, x, y);
}
static void Ltank_s(LOGIC L, int x, int y)
{
	switch(L->cave[x][y+1]) {
	case Xplayer:
	case Xspace:
	case Xsplash_e: case Xsplash_w:
	case Xplant:
		L->cave[x][y] = Ytank_sB;
		L->next[x][y] = Xspace;
		L->cave[x][y+1] = Ytank_s;
		L->next[x][y+1] = Xtank_1_s;
		PLAY(SOUND_tank);
		return;
	case Xacid_1: case Xacid_2:
	case Xacid_3: case Xacid_4:
	case Xacid_5: case Xacid_6:
	case Xacid_7: case Xacid_8:
		L->cave[x][y] = Ytank_sB;
		L->next[x][y] = Xspace;
		if(L->cave[x+1][y] == Xspace) L->cave[x+1][y] = Xsplash_e;
		if(L->cave[x-1][y] == Xspace) L->cave[x-1][y] = Xsplash_w;
		PLAY(SOUND_acid);
		return;
	}
	L->cave[x][y] = Ytank_s_w;
	L->next[x][y] = Xtank_2_w;
	PLAY(SOUND_tank);
}
static void Ltank_1_s(LOGIC L, int x, int y)
{
	switch(L->cave[x][y-1]) {
	case Xameuba_1: case Xameuba_2:
	case Xameuba_3: case Xameuba_4:
	case Xameuba_5: case Xameuba_6:
	case Xameuba_7: case Xameuba_8:
	case Xfake_ameuba: case Yfake_ameuba:
		L->next[x][y] = Ztank;
		return;
	}
	switch(L->cave[x][y+1]) {
	case Xameuba_1: case Xameuba_2:
	case Xameuba_3: case Xameuba_4:
	case Xameuba_5: case Xameuba_6:
	case Xameuba_7: case Xameuba_8:
	case Xfake_ameuba: case Yfake_ameuba:
		L->next[x][y] = Ztank;
		return;
	}
	switch(L->cave[x-1][y]) {
	case Xameuba_1: case Xameuba_2:
	case Xameuba_3: case Xameuba_4:
	case Xameuba_5: case Xameuba_6:
	case Xameuba_7: case Xameuba_8:
	case Xfake_ameuba: case Yfake_ameuba:
		L->next[x][y] = Ztank;
		return;
	}
	switch(L->cave[x+1][y]) {
	case Xplayer:
	case Xspace:
	case Xsplash_e: case Xsplash_w:
	case Xplant:
	case Xacid_1: case Xacid_2:
	case Xacid_3: case Xacid_4:
	case Xacid_5: case Xacid_6:
	case Xacid_7: case Xacid_8:
		L->cave[x][y] = Ytank_s_e;
		L->next[x][y] = Xtank_2_e;
		PLAY(SOUND_tank);
		return;
	case Xameuba_1: case Xameuba_2:
	case Xameuba_3: case Xameuba_4:
	case Xameuba_5: case Xameuba_6:
	case Xameuba_7: case Xameuba_8:
	case Xfake_ameuba: case Yfake_ameuba:
		L->next[x][y] = Ztank;
		return;
	}
	Ltank_s(L, x, y);
}
static void Ltank_2_s(LOGIC L, int x, int y)
{
	switch(L->cave[x][y-1]) {
	case Xameuba_1: case Xameuba_2:
	case Xameuba_3: case Xameuba_4:
	case Xameuba_5: case Xameuba_6:
	case Xameuba_7: case Xameuba_8:
	case Xfake_ameuba: case Yfake_ameuba:
		L->next[x][y] = Ztank;
		return;
	}
	switch(L->cave[x+1][y]) {
	case Xameuba_1: case Xameuba_2:
	case Xameuba_3: case Xameuba_4:
	case Xameuba_5: case Xameuba_6:
	case Xameuba_7: case Xameuba_8:
	case Xfake_ameuba: case Yfake_ameuba:
		L->next[x][y] = Ztank;
		return;
	}
	switch(L->cave[x][y+1]) {
	case Xameuba_1: case Xameuba_2:
	case Xameuba_3: case Xameuba_4:
	case Xameuba_5: case Xameuba_6:
	case Xameuba_7: case Xameuba_8:
	case Xfake_ameuba: case Yfake_ameuba:
		L->next[x][y] = Ztank;
		return;
	}
	switch(L->cave[x-1][y]) {
	case Xameuba_1: case Xameuba_2:
	case Xameuba_3: case Xameuba_4:
	case Xameuba_5: case Xameuba_6:
	case Xameuba_7: case Xameuba_8:
	case Xfake_ameuba: case Yfake_ameuba:
		L->next[x][y] = Ztank;
		return;
	}
	Ltank_s(L, x, y);
}
static void Ltank_w(LOGIC L, int x, int y)
{
	switch(L->cave[x-1][y]) {
	case Xplayer:
	case Xspace:
	case Xsplash_e: case Xsplash_w:
	case Xplant:
		L->cave[x][y] = Ytank_wB;
		L->next[x][y] = Xspace;
		L->cave[x-1][y] = Ytank_w;
		L->next[x-1][y] = Xtank_1_w;
		PLAY(SOUND_tank);
		return;
	case Xacid_1: case Xacid_2:
	case Xacid_3: case Xacid_4:
	case Xacid_5: case Xacid_6:
	case Xacid_7: case Xacid_8:
		L->cave[x][y] = Ytank_wB;
		L->next[x][y] = Xspace;
		if(L->cave[x][y-1] == Xspace) L->cave[x][y-1] = Xsplash_e;
		if(L->cave[x-2][y-1] == Xspace) L->cave[x-2][y-1] = Xsplash_w;
		PLAY(SOUND_acid);
		return;
	}
	L->cave[x][y] = Ytank_w_n;
	L->next[x][y] = Xtank_2_n;
	PLAY(SOUND_tank);
}
static void Ltank_1_w(LOGIC L, int x, int y)
{
	switch(L->cave[x][y-1]) {
	case Xameuba_1: case Xameuba_2:
	case Xameuba_3: case Xameuba_4:
	case Xameuba_5: case Xameuba_6:
	case Xameuba_7: case Xameuba_8:
	case Xfake_ameuba: case Yfake_ameuba:
		L->next[x][y] = Ztank;
		return;
	}
	switch(L->cave[x+1][y]) {
	case Xameuba_1: case Xameuba_2:
	case Xameuba_3: case Xameuba_4:
	case Xameuba_5: case Xameuba_6:
	case Xameuba_7: case Xameuba_8:
	case Xfake_ameuba: case Yfake_ameuba:
		L->next[x][y] = Ztank;
		return;
	}
	switch(L->cave[x-1][y]) {
	case Xameuba_1: case Xameuba_2:
	case Xameuba_3: case Xameuba_4:
	case Xameuba_5: case Xameuba_6:
	case Xameuba_7: case Xameuba_8:
	case Xfake_ameuba: case Yfake_ameuba:
		L->next[x][y] = Ztank;
		return;
	}
	switch(L->cave[x][y+1]) {
	case Xplayer:
	case Xspace:
	case Xsplash_e: case Xsplash_w:
	case Xplant:
	case Xacid_1: case Xacid_2:
	case Xacid_3: case Xacid_4:
	case Xacid_5: case Xacid_6:
	case Xacid_7: case Xacid_8:
		L->cave[x][y] = Ytank_w_s;
		L->next[x][y] = Xtank_2_s;
		PLAY(SOUND_tank);
		return;
	case Xameuba_1: case Xameuba_2:
	case Xameuba_3: case Xameuba_4:
	case Xameuba_5: case Xameuba_6:
	case Xameuba_7: case Xameuba_8:
	case Xfake_ameuba: case Yfake_ameuba:
		L->next[x][y] = Ztank;
		return;
	}
	Ltank_w(L, x, y);
}
static void Ltank_2_w(LOGIC L, int x, int y)
{
	switch(L->cave[x][y-1]) {
	case Xameuba_1: case Xameuba_2:
	case Xameuba_3: case Xameuba_4:
	case Xameuba_5: case Xameuba_6:
	case Xameuba_7: case Xameuba_8:
	case Xfake_ameuba: case Yfake_ameuba:
		L->next[x][y] = Ztank;
		return;
	}
	switch(L->cave[x+1][y]) {
	case Xameuba_1: case Xameuba_2:
	case Xameuba_3: case Xameuba_4:
	case Xameuba_5: case Xameuba_6:
	case Xameuba_7: case Xameuba_8:
	case Xfake_ameuba: case Yfake_ameuba:
		L->next[x][y] = Ztank;
		return;
	}
	switch(L->cave[x][y+1]) {
	case Xameuba_1: case Xameuba_2:
	case Xameuba_3: case Xameuba_4:
	case Xameuba_5: case Xameuba_6:
	case Xameuba_7: case Xameuba_8:
	case Xfake_ameuba: case Yfake_ameuba:
		L->next[x][y] = Ztank;
		return;
	}
	switch(L->cave[x-1][y]) {
	case Xameuba_1: case Xameuba_2:
	case Xameuba_3: case Xameuba_4:
	case Xameuba_5: case Xameuba_6:
	case Xameuba_7: case Xameuba_8:
	case Xfake_ameuba: case Yfake_ameuba:
		L->next[x][y] = Ztank;
		return;
	}
	Ltank_w(L, x, y);
}

static void Lemerald(LOGIC L, int x, int y)
{
	switch(L->cave[x][y+1]) {
	case Xspace:
	case Xsplash_e: case Xsplash_w:
		L->cave[x][y] = Yemerald_sB;
		L->next[x][y] = Xspace;
		L->cave[x][y+1] = Yemerald_s;
		L->next[x][y+1] = Xemerald_fall;
		return;
	case Xacid_1: case Xacid_2:
	case Xacid_3: case Xacid_4:
	case Xacid_5: case Xacid_6:
	case Xacid_7: case Xacid_8:
		L->cave[x][y] = Yemerald_sB;
		L->next[x][y] = Xspace;
		if(L->cave[x+1][y] == Xspace) L->cave[x+1][y] = Xsplash_e;
		if(L->cave[x-1][y] == Xspace) L->cave[x-1][y] = Xsplash_w;
		PLAY(SOUND_acid);
		return;
	case Xandroid:
	case Xandroid_1_n: case Xandroid_2_n:
	case Xandroid_1_e: case Xandroid_2_e:
	case Xandroid_1_s: case Xandroid_2_s:
	case Xandroid_1_w: case Xandroid_2_w:
	case Xemerald:
	case Xemerald_pause:
	case Xdiamond:
	case Xdiamond_pause:
	case Xstone:
	case Xstone_pause:
	case Xbomb:
	case Xbomb_pause:
	case Xnut:
	case Xnut_pause:
	case Xspring:
	case Xspring_pause:
	case Xspring_e: case Xspring_w:
	case Xkey_1: case Xkey_2:
	case Xkey_3: case Xkey_4:
	case Xkey_5: case Xkey_6:
	case Xkey_7: case Xkey_8:
	case Xballoon:
	case Xball_1: case Xball_2:
	case Yball_1: case Yball_2:
	case Xwonderwall: case Ywonderwall:
	case Xswitch: case Yswitch:
	case Xbumper: case Ybumper:
	case Xacid_nw: case Xacid_ne:
	case Xslide_ns: case Xslide_ew:
	case Xwall_1: case Xwall_2:
	case Xwall_3: case Xwall_4:
	case Xroundwall_1: case Xroundwall_2:
	case Xroundwall_3: case Xroundwall_4:
	case Xsteel_1: case Xsteel_2:
	case Xsteel_3: case Xsteel_4:
		if(RANDOM(2)) {
			if(is_space[L->cave[x+1][y]]) {
				switch(L->cave[x+1][y+1]) {
				case Xspace:
				case Xsplash_e: case Xsplash_w:
				case Xacid_1: case Xacid_2:
				case Xacid_3: case Xacid_4:
				case Xacid_5: case Xacid_6:
				case Xacid_7: case Xacid_8:
					L->cave[x][y] = Yemerald_eB;
					L->next[x][y] = Xspace;
					L->cave[x+1][y] = Yemerald_e;
					L->next[x+1][y] = Xemerald_pause;
					if(L->cave[x][y+1] == Xbumper) L->cave[x][y+1] = Ybumper;
					return;
				}
			}
			if(is_space[L->cave[x-1][y]]) {
				switch(L->cave[x-1][y+1]) {
				case Xspace:
				case Xsplash_e: case Xsplash_w:
				case Xacid_1: case Xacid_2:
				case Xacid_3: case Xacid_4:
				case Xacid_5: case Xacid_6:
				case Xacid_7: case Xacid_8:
					L->cave[x][y] = Yemerald_wB;
					L->next[x][y] = Xspace;
					L->cave[x-1][y] = Yemerald_w;
					L->next[x-1][y] = Xemerald_pause;
					if(L->cave[x][y+1] == Xbumper) L->cave[x][y+1] = Ybumper;
					return;
				}
			}
		} else {
			if(is_space[L->cave[x-1][y]]) {
				switch(L->cave[x-1][y+1]) {
				case Xspace:
				case Xsplash_e: case Xsplash_w:
				case Xacid_1: case Xacid_2:
				case Xacid_3: case Xacid_4:
				case Xacid_5: case Xacid_6:
				case Xacid_7: case Xacid_8:
					L->cave[x][y] = Yemerald_wB;
					L->next[x][y] = Xspace;
					L->cave[x-1][y] = Yemerald_w;
					L->next[x-1][y] = Xemerald_pause;
					if(L->cave[x][y+1] == Xbumper) L->cave[x][y+1] = Ybumper;
					return;
				}
			}
			if(is_space[L->cave[x+1][y]]) {
				switch(L->cave[x+1][y+1]) {
				case Xspace:
				case Xsplash_e: case Xsplash_w:
				case Xacid_1: case Xacid_2:
				case Xacid_3: case Xacid_4:
				case Xacid_5: case Xacid_6:
				case Xacid_7: case Xacid_8:
					L->cave[x][y] = Yemerald_eB;
					L->next[x][y] = Xspace;
					L->cave[x+1][y] = Yemerald_e;
					L->next[x+1][y] = Xemerald_pause;
					if(L->cave[x][y+1] == Xbumper) L->cave[x][y+1] = Ybumper;
					return;
				}
			}
		}
	}
	if(++L->shine_cnt > 50) {
		L->shine_cnt = RANDOM(8);
		L->cave[x][y] = Yemerald_shine;
	}
}
static void Lemerald_pause(LOGIC L, int x, int y)
{
	switch(L->cave[x][y+1]) {
	case Xspace:
	case Xsplash_e: case Xsplash_w:
		L->cave[x][y] = Yemerald_sB;
		L->next[x][y] = Xspace;
		L->cave[x][y+1] = Yemerald_s;
		L->next[x][y+1] = Xemerald_fall;
		return;
	case Xacid_1: case Xacid_2:
	case Xacid_3: case Xacid_4:
	case Xacid_5: case Xacid_6:
	case Xacid_7: case Xacid_8:
		L->cave[x][y] = Yemerald_sB;
		L->next[x][y] = Xspace;
		if(L->cave[x+1][y] == Xspace) L->cave[x+1][y] = Xsplash_e;
		if(L->cave[x-1][y] == Xspace) L->cave[x-1][y] = Xsplash_w;
		PLAY(SOUND_acid);
		return;
	}
	L->cave[x][y] = Xemerald;
	L->next[x][y] = Xemerald;
}
static void Lemerald_fall(LOGIC L, int x, int y)
{
	switch(L->cave[x][y+1]) {
	case Xplayer:
	case Xspace:
	case Xsplash_e: case Xsplash_w:
		L->cave[x][y] = Yemerald_sB;
		L->next[x][y] = Xspace;
		L->cave[x][y+1] = Yemerald_s;
		L->next[x][y+1] = Xemerald_fall;
		return;
	case Xacid_1: case Xacid_2:
	case Xacid_3: case Xacid_4:
	case Xacid_5: case Xacid_6:
	case Xacid_7: case Xacid_8:
		L->cave[x][y] = Yemerald_sB;
		L->next[x][y] = Xspace;
		if(L->cave[x+1][y] == Xspace) L->cave[x+1][y] = Xsplash_e;
		if(L->cave[x-1][y] == Xspace) L->cave[x-1][y] = Xsplash_w;
		PLAY(SOUND_acid);
		return;
	case Xwonderwall: case Ywonderwall:
		if(L->wonderwall_time) {
			L->wonderwall_state = 1;
			L->cave[x][y] = Yemerald_sB;
			L->next[x][y] = Xspace;
			if(is_space[L->cave[x][y+2]]) {
				L->cave[x][y+2] = Ydiamond_s;
				L->next[x][y+2] = Xdiamond_fall;
			}
			L->sound[SOUND_squash] = 1;
			return;
		}
	}
	L->cave[x][y] = Xemerald;
	L->next[x][y] = Xemerald;
	PLAY(SOUND_diamond);
}

static void Ldiamond(LOGIC L, int x, int y)
{
	switch(L->cave[x][y+1]) {
	case Xspace:
	case Xsplash_e: case Xsplash_w:
		L->cave[x][y] = Ydiamond_sB;
		L->next[x][y] = Xspace;
		L->cave[x][y+1] = Ydiamond_s;
		L->next[x][y+1] = Xdiamond_fall;
		return;
	case Xacid_1: case Xacid_2:
	case Xacid_3: case Xacid_4:
	case Xacid_5: case Xacid_6:
	case Xacid_7: case Xacid_8:
		L->cave[x][y] = Ydiamond_sB;
		L->next[x][y] = Xspace;
		if(L->cave[x+1][y] == Xspace) L->cave[x+1][y] = Xsplash_e;
		if(L->cave[x-1][y] == Xspace) L->cave[x-1][y] = Xsplash_w;
		PLAY(SOUND_acid);
		return;
	case Xandroid:
	case Xandroid_1_n: case Xandroid_2_n:
	case Xandroid_1_e: case Xandroid_2_e:
	case Xandroid_1_s: case Xandroid_2_s:
	case Xandroid_1_w: case Xandroid_2_w:
	case Xemerald:
	case Xemerald_pause:
	case Xdiamond:
	case Xdiamond_pause:
	case Xstone:
	case Xstone_pause:
	case Xbomb:
	case Xbomb_pause:
	case Xnut:
	case Xnut_pause:
	case Xspring:
	case Xspring_pause:
	case Xspring_e: case Xspring_w:
	case Xkey_1: case Xkey_2:
	case Xkey_3: case Xkey_4:
	case Xkey_5: case Xkey_6:
	case Xkey_7: case Xkey_8:
	case Xballoon:
	case Xball_1: case Xball_2:
	case Yball_1: case Yball_2:
	case Xwonderwall: case Ywonderwall:
	case Xswitch: case Yswitch:
	case Xbumper: case Ybumper:
	case Xacid_nw: case Xacid_ne:
	case Xslide_ns: case Xslide_ew:
	case Xwall_1: case Xwall_2:
	case Xwall_3: case Xwall_4:
	case Xroundwall_1: case Xroundwall_2:
	case Xroundwall_3: case Xroundwall_4:
	case Xsteel_1: case Xsteel_2:
	case Xsteel_3: case Xsteel_4:
		if(RANDOM(2)) {
			if(is_space[L->cave[x+1][y]]) {
				switch(L->cave[x+1][y+1]) {
				case Xspace:
				case Xsplash_e: case Xsplash_w:
				case Xacid_1: case Xacid_2:
				case Xacid_3: case Xacid_4:
				case Xacid_5: case Xacid_6:
				case Xacid_7: case Xacid_8:
					L->cave[x][y] = Ydiamond_eB;
					L->next[x][y] = Xspace;
					L->cave[x+1][y] = Ydiamond_e;
					L->next[x+1][y] = Xdiamond_pause;
					if(L->cave[x][y+1] == Xbumper) L->cave[x][y+1] = Ybumper;
					return;
				}
			}
			if(is_space[L->cave[x-1][y]]) {
				switch(L->cave[x-1][y+1]) {
				case Xspace:
				case Xsplash_e: case Xsplash_w:
				case Xacid_1: case Xacid_2:
				case Xacid_3: case Xacid_4:
				case Xacid_5: case Xacid_6:
				case Xacid_7: case Xacid_8:
					L->cave[x][y] = Ydiamond_wB;
					L->next[x][y] = Xspace;
					L->cave[x-1][y] = Ydiamond_w;
					L->next[x-1][y] = Xdiamond_pause;
					if(L->cave[x][y+1] == Xbumper) L->cave[x][y+1] = Ybumper;
					return;
				}
			}
		} else {
			if(is_space[L->cave[x-1][y]]) {
				switch(L->cave[x-1][y+1]) {
				case Xspace:
				case Xsplash_e: case Xsplash_w:
				case Xacid_1: case Xacid_2:
				case Xacid_3: case Xacid_4:
				case Xacid_5: case Xacid_6:
				case Xacid_7: case Xacid_8:
					L->cave[x][y] = Ydiamond_wB;
					L->next[x][y] = Xspace;
					L->cave[x-1][y] = Ydiamond_w;
					L->next[x-1][y] = Xdiamond_pause;
					if(L->cave[x][y+1] == Xbumper) L->cave[x][y+1] = Ybumper;
					return;
				}
			}
			if(is_space[L->cave[x+1][y]]) {
				switch(L->cave[x+1][y+1]) {
				case Xspace:
				case Xsplash_e: case Xsplash_w:
				case Xacid_1: case Xacid_2:
				case Xacid_3: case Xacid_4:
				case Xacid_5: case Xacid_6:
				case Xacid_7: case Xacid_8:
					L->cave[x][y] = Ydiamond_eB;
					L->next[x][y] = Xspace;
					L->cave[x+1][y] = Ydiamond_e;
					L->next[x+1][y] = Xdiamond_pause;
					if(L->cave[x][y+1] == Xbumper) L->cave[x][y+1] = Ybumper;
					return;
				}
			}
		}
	}
	if(++L->shine_cnt > 50) {
		L->shine_cnt = RANDOM(8);
		L->cave[x][y] = Ydiamond_shine;
	}
}
static void Ldiamond_pause(LOGIC L, int x, int y)
{
	switch(L->cave[x][y+1]) {
	case Xspace:
	case Xsplash_e: case Xsplash_w:
		L->cave[x][y] = Ydiamond_sB;
		L->next[x][y] = Xspace;
		L->cave[x][y+1] = Ydiamond_s;
		L->next[x][y+1] = Xdiamond_fall;
		return;
	case Xacid_1: case Xacid_2:
	case Xacid_3: case Xacid_4:
	case Xacid_5: case Xacid_6:
	case Xacid_7: case Xacid_8:
		L->cave[x][y] = Ydiamond_sB;
		L->next[x][y] = Xspace;
		if(L->cave[x+1][y] == Xspace) L->cave[x+1][y] = Xsplash_e;
		if(L->cave[x-1][y] == Xspace) L->cave[x-1][y] = Xsplash_w;
		PLAY(SOUND_acid);
		return;
	}
	L->cave[x][y] = Xdiamond;
	L->next[x][y] = Xdiamond;
}
static void Ldiamond_fall(LOGIC L, int x, int y)
{
	switch(L->cave[x][y+1]) {
	case Xplayer:
	case Xspace:
	case Xsplash_e: case Xsplash_w:
		L->cave[x][y] = Ydiamond_sB;
		L->next[x][y] = Xspace;
		L->cave[x][y+1] = Ydiamond_s;
		L->next[x][y+1] = Xdiamond_fall;
		return;
	case Xacid_1: case Xacid_2:
	case Xacid_3: case Xacid_4:
	case Xacid_5: case Xacid_6:
	case Xacid_7: case Xacid_8:
		L->cave[x][y] = Ydiamond_sB;
		L->next[x][y] = Xspace;
		if(L->cave[x+1][y] == Xspace) L->cave[x+1][y] = Xsplash_e;
		if(L->cave[x-1][y] == Xspace) L->cave[x-1][y] = Xsplash_w;
		PLAY(SOUND_acid);
		return;
	case Xwonderwall: case Ywonderwall:
		if(L->wonderwall_time) {
			L->wonderwall_state = 1;
			L->cave[x][y] = Ydiamond_sB;
			L->next[x][y] = Xspace;
			if(is_space[L->cave[x][y+2]]) {
				L->cave[x][y+2] = Ystone_s;
				L->next[x][y+2] = Xstone_fall;
			}
			L->sound[SOUND_squash] = 1;
			return;
		}
	}
	L->cave[x][y] = Xdiamond;
	L->next[x][y] = Xdiamond;
	PLAY(SOUND_diamond);
}

static void Lstone(LOGIC L, int x, int y)
{
	switch(L->cave[x][y+1]) {
	case Xspace:
	case Xsplash_e: case Xsplash_w:
	case Xplant:
		L->cave[x][y] = Ystone_sB;
		L->next[x][y] = Xspace;
		L->cave[x][y+1] = Ystone_s;
		L->next[x][y+1] = Xstone_fall;
		return;
	case Xacid_1: case Xacid_2:
	case Xacid_3: case Xacid_4:
	case Xacid_5: case Xacid_6:
	case Xacid_7: case Xacid_8:
		L->cave[x][y] = Ystone_sB;
		L->next[x][y] = Xspace;
		if(L->cave[x+1][y] == Xspace) L->cave[x+1][y] = Xsplash_e;
		if(L->cave[x-1][y] == Xspace) L->cave[x-1][y] = Xsplash_w;
		PLAY(SOUND_acid);
		return;
	case Xsand:
		L->cave[x][y] = Xsand_stone_1_sB;
		L->next[x][y] = Xsand_stone_2_sB;
		L->cave[x][y+1] = Xsand_sand_1_s;
		L->next[x][y+1] = Xsand_sand_2_s;
		return;
	case Xandroid:
	case Xandroid_1_n: case Xandroid_2_n:
	case Xandroid_1_e: case Xandroid_2_e:
	case Xandroid_1_s: case Xandroid_2_s:
	case Xandroid_1_w: case Xandroid_2_w:
	case Xemerald:
	case Xemerald_pause:
	case Xdiamond:
	case Xdiamond_pause:
	case Xstone:
	case Xstone_pause:
	case Xbomb:
	case Xbomb_pause:
	case Xnut:
	case Xnut_pause:
	case Xspring:
	case Xspring_pause:
	case Xspring_e: case Xspring_w:
	case Xkey_1: case Xkey_2:
	case Xkey_3: case Xkey_4:
	case Xkey_5: case Xkey_6:
	case Xkey_7: case Xkey_8:
	case Xballoon:
	case Xball_1: case Xball_2:
	case Yball_1: case Yball_2:
	case Xswitch: case Yswitch:
	case Xbumper: case Ybumper:
	case Xacid_nw: case Xacid_ne:
	case Xlenses:
	case Xmagnify:
	case Xslide_ns: case Xslide_ew:
	case Xroundwall_1: case Xroundwall_2:
	case Xroundwall_3: case Xroundwall_4:
		if(RANDOM(2)) {
			if(is_space[L->cave[x+1][y]]) {
				switch(L->cave[x+1][y+1]) {
				case Xspace:
				case Xsplash_e: case Xsplash_w:
				case Xacid_1: case Xacid_2:
				case Xacid_3: case Xacid_4:
				case Xacid_5: case Xacid_6:
				case Xacid_7: case Xacid_8:
					L->cave[x][y] = Ystone_eB;
					L->next[x][y] = Xspace;
					L->cave[x+1][y] = Ystone_e;
					L->next[x+1][y] = Xstone_pause;
					if(L->cave[x][y+1] == Xbumper) L->cave[x][y+1] = Ybumper;
					return;
				}
			}
			if(is_space[L->cave[x-1][y]]) {
				switch(L->cave[x-1][y+1]) {
				case Xspace:
				case Xsplash_e: case Xsplash_w:
				case Xacid_1: case Xacid_2:
				case Xacid_3: case Xacid_4:
				case Xacid_5: case Xacid_6:
				case Xacid_7: case Xacid_8:
					L->cave[x][y] = Ystone_wB;
					L->next[x][y] = Xspace;
					L->cave[x-1][y] = Ystone_w;
					L->next[x-1][y] = Xstone_pause;
					if(L->cave[x][y+1] == Xbumper) L->cave[x][y+1] = Ybumper;
					return;
				}
			}
		} else {
			if(is_space[L->cave[x-1][y]]) {
				switch(L->cave[x-1][y+1]) {
				case Xspace:
				case Xsplash_e: case Xsplash_w:
				case Xacid_1: case Xacid_2:
				case Xacid_3: case Xacid_4:
				case Xacid_5: case Xacid_6:
				case Xacid_7: case Xacid_8:
					L->cave[x][y] = Ystone_wB;
					L->next[x][y] = Xspace;
					L->cave[x-1][y] = Ystone_w;
					L->next[x-1][y] = Xstone_pause;
					if(L->cave[x][y+1] == Xbumper) L->cave[x][y+1] = Ybumper;
					return;
				}
			}
			if(is_space[L->cave[x+1][y]]) {
				switch(L->cave[x+1][y+1]) {
				case Xspace:
				case Xsplash_e: case Xsplash_w:
				case Xacid_1: case Xacid_2:
				case Xacid_3: case Xacid_4:
				case Xacid_5: case Xacid_6:
				case Xacid_7: case Xacid_8:
					L->cave[x][y] = Ystone_eB;
					L->next[x][y] = Xspace;
					L->cave[x+1][y] = Ystone_e;
					L->next[x+1][y] = Xstone_pause;
					if(L->cave[x][y+1] == Xbumper) L->cave[x][y+1] = Ybumper;
					return;
				}
			}
		}
	}
}
static void Lstone_pause(LOGIC L, int x, int y)
{
	switch(L->cave[x][y+1]) {
	case Xspace:
	case Xsplash_e: case Xsplash_w:
		L->cave[x][y] = Ystone_sB;
		L->next[x][y] = Xspace;
		L->cave[x][y+1] = Ystone_s;
		L->next[x][y+1] = Xstone_fall;
		return;
	case Xacid_1: case Xacid_2:
	case Xacid_3: case Xacid_4:
	case Xacid_5: case Xacid_6:
	case Xacid_7: case Xacid_8:
		L->cave[x][y] = Ystone_sB;
		L->next[x][y] = Xspace;
		if(L->cave[x+1][y] == Xspace) L->cave[x+1][y] = Xsplash_e;
		if(L->cave[x-1][y] == Xspace) L->cave[x-1][y] = Xsplash_w;
		PLAY(SOUND_acid);
		return;
	}
	L->cave[x][y] = Xstone;
	L->next[x][y] = Xstone;
}
static void Lstone_fall(LOGIC L, int x, int y)
{
	switch(L->cave[x][y+1]) {
	case Xplayer:
	case Xspace:
	case Xsplash_e: case Xsplash_w:
		L->cave[x][y] = Ystone_sB;
		L->next[x][y] = Xspace;
		L->cave[x][y+1] = Ystone_s;
		L->next[x][y+1] = Xstone_fall;
		return;
	case Xacid_1: case Xacid_2:
	case Xacid_3: case Xacid_4:
	case Xacid_5: case Xacid_6:
	case Xacid_7: case Xacid_8:
		L->cave[x][y] = Ystone_sB;
		L->next[x][y] = Xspace;
		if(L->cave[x+1][y] == Xspace) L->cave[x+1][y] = Xsplash_e;
		if(L->cave[x-1][y] == Xspace) L->cave[x-1][y] = Xsplash_w;
		PLAY(SOUND_acid);
		return;
	case Xeater_n: case Xeater_e:
	case Xeater_s: case Xeater_w:
		L->cave[x][y] = Ystone_sB;
		L->next[x][y] = Xspace;
		L->cave[x][y+1] = Yeater_stone;
		L->next[x][y+1] = Zeater;
		L->score += L->eater_score;
		return;
	case Xalien:
	case Xalien_pause:
		L->cave[x][y] = Ystone_sB;
		L->next[x][y] = Xspace;
		L->cave[x][y+1] = Yalien_stone;
		L->next[x][y+1] = Ztank;
		L->score += L->alien_score;
		return;
	case Xbug_1_n: case Xbug_2_n:
		L->cave[x][y] = Ystone_sB;
		L->next[x][y] = Xspace;
		L->cave[x][y+1] = Ybug_stone_n;
		L->next[x][y+1] = Zbug;
		L->score += L->bug_score;
		return;
	case Xbug_1_e: case Xbug_2_e:
		L->cave[x][y] = Ystone_sB;
		L->next[x][y] = Xspace;
		L->cave[x][y+1] = Ybug_stone_e;
		L->next[x][y+1] = Zbug;
		L->score += L->bug_score;
		return;
	case Xbug_1_s: case Xbug_2_s:
		L->cave[x][y] = Ystone_sB;
		L->next[x][y] = Xspace;
		L->cave[x][y+1] = Ybug_stone_s;
		L->next[x][y+1] = Zbug;
		L->score += L->bug_score;
		return;
	case Xbug_1_w: case Xbug_2_w:
		L->cave[x][y] = Ystone_sB;
		L->next[x][y] = Xspace;
		L->cave[x][y+1] = Ybug_stone_w;
		L->next[x][y+1] = Zbug;
		L->score += L->bug_score;
		return;
	case Xtank_1_n: case Xtank_2_n:
		L->cave[x][y] = Ystone_sB;
		L->next[x][y] = Xspace;
		L->cave[x][y+1] = Ytank_stone_n;
		L->next[x][y+1] = Ztank;
		L->score += L->tank_score;
		return;
	case Xtank_1_e: case Xtank_2_e:
		L->cave[x][y] = Ystone_sB;
		L->next[x][y] = Xspace;
		L->cave[x][y+1] = Ytank_stone_e;
		L->next[x][y+1] = Ztank;
		L->score += L->tank_score;
		return;
	case Xtank_1_s: case Xtank_2_s:
		L->cave[x][y] = Ystone_sB;
		L->next[x][y] = Xspace;
		L->cave[x][y+1] = Ytank_stone_s;
		L->next[x][y+1] = Ztank;
		L->score += L->tank_score;
		return;
	case Xtank_1_w: case Xtank_2_w:
		L->cave[x][y] = Ystone_sB;
		L->next[x][y] = Xspace;
		L->cave[x][y+1] = Ytank_stone_w;
		L->next[x][y+1] = Ztank;
		L->score += L->tank_score;
		return;
	case Xdiamond:
	case Xdiamond_pause:
		switch(L->cave[x][y+2]) {
		case Xplayer:
		case Xspace:
		case Xsplash_e: case Xsplash_w:
		case Xplant:
		case Yplant:
		case Xacid_1: case Xacid_2:
		case Xacid_3: case Xacid_4:
		case Xacid_5: case Xacid_6:
		case Xacid_7: case Xacid_8:
		case Xandroid:
		case Xandroid_1_n: case Xandroid_2_n:
		case Xandroid_1_e: case Xandroid_2_e:
		case Xandroid_1_s: case Xandroid_2_s:
		case Xandroid_1_w: case Xandroid_2_w:
		case Xbug_1_n: case Xbug_2_n:
		case Xbug_1_e: case Xbug_2_e:
		case Xbug_1_s: case Xbug_2_s:
		case Xbug_1_w: case Xbug_2_w:
		case Xtank_1_n: case Xtank_2_n:
		case Xtank_1_e: case Xtank_2_e:
		case Xtank_1_s: case Xtank_2_s:
		case Xtank_1_w: case Xtank_2_w:
		case Xemerald_fall:
		case Xdiamond_fall:
		case Xstone_fall:
		case Xbomb_fall:
		case Xnut_fall:
		case Xspring_fall:
		case Xacid_s:
			L->cave[x][y] = Xstone;
			L->next[x][y] = Xstone;
			PLAY(SOUND_stone);
			return;
		}
		L->cave[x][y] = Ystone_sB;
		L->next[x][y] = Xspace;
		L->cave[x][y+1] = Ydiamond_stone;
		L->next[x][y+1] = Xstone_pause;
		L->sound[SOUND_squash] = 1;
		return;
	case Xbomb:
	case Xbomb_pause:
		L->cave[x][y] = Xstone;
		L->next[x][y] = Xstone;
		L->cave[x][y+1] = Ybomb_space;
		L->next[x][y+1] = Ztank;
		return;
	case Xnut:
	case Xnut_pause:
		L->cave[x][y] = Xstone;
		L->next[x][y] = Xstone;
		L->cave[x][y+1] = Ynut_stone;
		L->next[x][y+1] = Xemerald;
		L->sound[SOUND_crack] = 1;
		L->score += L->nut_score;
		return;
	case Xspring:
		if(RANDOM(2)) {
			L->cave[x][y+1] = Xspring_w;
			switch(L->cave[x+1][y+1]) {
			case Xspace:
			case Xsplash_e: case Xsplash_w:
			case Xacid_1: case Xacid_2:
			case Xacid_3: case Xacid_4:
			case Xacid_5: case Xacid_6:
			case Xacid_7: case Xacid_8:
			case Xalien:
			case Xalien_pause:
			case Yalien_n: case Yalien_nB:
			case Yalien_e: case Yalien_eB:
			case Yalien_s: case Yalien_sB:
			case Yalien_w: case Yalien_wB:
				L->cave[x][y+1] = Xspring_e;
			}
		} else {
			L->cave[x][y+1] = Xspring_e;
			switch(L->cave[x-1][y+1]) {
			case Xspace:
			case Xsplash_e: case Xsplash_w:
			case Xacid_1: case Xacid_2:
			case Xacid_3: case Xacid_4:
			case Xacid_5: case Xacid_6:
			case Xacid_7: case Xacid_8:
			case Xalien:
			case Xalien_pause:
			case Yalien_n: case Yalien_nB:
			case Yalien_e: case Yalien_eB:
			case Yalien_s: case Yalien_sB:
			case Yalien_w: case Yalien_wB:
				L->cave[x][y+1] = Xspring_w;
			}
		}
		L->cave[x][y] = Xstone;
		L->next[x][y] = Xstone;
		PLAY(SOUND_spring);
		return;
	case Xwonderwall: case Ywonderwall:
		if(L->wonderwall_time) {
			L->wonderwall_state = 1;
			L->cave[x][y] = Ystone_sB;
			L->next[x][y] = Xspace;
			if(is_space[L->cave[x][y+2]]) {
				L->cave[x][y+2] = Yemerald_s;
				L->next[x][y+2] = Xemerald_fall;
			}
			L->sound[SOUND_squash] = 1;
			return;
		}
	}
	L->cave[x][y] = Xstone;
	L->next[x][y] = Xstone;
	PLAY(SOUND_stone);
}

static void Lbomb(LOGIC L, int x, int y)
{
	switch(L->cave[x][y+1]) {
	case Xspace:
	case Xsplash_e: case Xsplash_w:
		L->cave[x][y] = Ybomb_sB;
		L->next[x][y] = Xspace;
		L->cave[x][y+1] = Ybomb_s;
		L->next[x][y+1] = Xbomb_fall;
		return;
	case Xacid_1: case Xacid_2:
	case Xacid_3: case Xacid_4:
	case Xacid_5: case Xacid_6:
	case Xacid_7: case Xacid_8:
		L->cave[x][y] = Ybomb_sB;
		L->next[x][y] = Xspace;
		if(L->cave[x+1][y] == Xspace) L->cave[x+1][y] = Xsplash_e;
		if(L->cave[x-1][y] == Xspace) L->cave[x-1][y] = Xsplash_w;
		PLAY(SOUND_acid);
		return;
	case Xandroid:
	case Xandroid_1_n: case Xandroid_2_n:
	case Xandroid_1_e: case Xandroid_2_e:
	case Xandroid_1_s: case Xandroid_2_s:
	case Xandroid_1_w: case Xandroid_2_w:
	case Xemerald:
	case Xemerald_pause:
	case Xdiamond:
	case Xdiamond_pause:
	case Xstone:
	case Xstone_pause:
	case Xbomb:
	case Xbomb_pause:
	case Xnut:
	case Xnut_pause:
	case Xspring:
	case Xspring_pause:
	case Xspring_e: case Xspring_w:
	case Xkey_1: case Xkey_2:
	case Xkey_3: case Xkey_4:
	case Xkey_5: case Xkey_6:
	case Xkey_7: case Xkey_8:
	case Xballoon:
	case Xball_1: case Xball_2:
	case Yball_1: case Yball_2:
	case Xswitch: case Yswitch:
	case Xbumper: case Ybumper:
	case Xacid_nw: case Xacid_ne:
	case Xslide_ns: case Xslide_ew:
	case Xroundwall_1: case Xroundwall_2:
	case Xroundwall_3: case Xroundwall_4:
		if(RANDOM(2)) {
			if(is_space[L->cave[x+1][y]]) {
				switch(L->cave[x+1][y+1]) {
				case Xspace:
				case Xsplash_e: case Xsplash_w:
				case Xacid_1: case Xacid_2:
				case Xacid_3: case Xacid_4:
				case Xacid_5: case Xacid_6:
				case Xacid_7: case Xacid_8:
					L->cave[x][y] = Ybomb_eB;
					L->next[x][y] = Xspace;
					L->cave[x+1][y] = Ybomb_e;
					L->next[x+1][y] = Xbomb_pause;
					if(L->cave[x][y+1] == Xbumper) L->cave[x][y+1] = Ybumper;
					return;
				}
			}
			if(is_space[L->cave[x-1][y]]) {
				switch(L->cave[x-1][y+1]) {
				case Xspace:
				case Xsplash_e: case Xsplash_w:
				case Xacid_1: case Xacid_2:
				case Xacid_3: case Xacid_4:
				case Xacid_5: case Xacid_6:
				case Xacid_7: case Xacid_8:
					L->cave[x][y] = Ybomb_wB;
					L->next[x][y] = Xspace;
					L->cave[x-1][y] = Ybomb_w;
					L->next[x-1][y] = Xbomb_pause;
					if(L->cave[x][y+1] == Xbumper) L->cave[x][y+1] = Ybumper;
					return;
				}
			}
		} else {
			if(is_space[L->cave[x-1][y]]) {
				switch(L->cave[x-1][y+1]) {
				case Xspace:
				case Xsplash_e: case Xsplash_w:
				case Xacid_1: case Xacid_2:
				case Xacid_3: case Xacid_4:
				case Xacid_5: case Xacid_6:
				case Xacid_7: case Xacid_8:
					L->cave[x][y] = Ybomb_wB;
					L->next[x][y] = Xspace;
					L->cave[x-1][y] = Ybomb_w;
					L->next[x-1][y] = Xbomb_pause;
					if(L->cave[x][y+1] == Xbumper) L->cave[x][y+1] = Ybumper;
					return;
				}
			}
			if(is_space[L->cave[x+1][y]]) {
				switch(L->cave[x+1][y+1]) {
				case Xspace:
				case Xsplash_e: case Xsplash_w:
				case Xacid_1: case Xacid_2:
				case Xacid_3: case Xacid_4:
				case Xacid_5: case Xacid_6:
				case Xacid_7: case Xacid_8:
					L->cave[x][y] = Ybomb_eB;
					L->next[x][y] = Xspace;
					L->cave[x+1][y] = Ybomb_e;
					L->next[x+1][y] = Xbomb_pause;
					if(L->cave[x][y+1] == Xbumper) L->cave[x][y+1] = Ybumper;
					return;
				}
			}
		}
	}
}
static void Lbomb_pause(LOGIC L, int x, int y)
{
	switch(L->cave[x][y+1]) {
	case Xspace:
	case Xsplash_e: case Xsplash_w:
		L->cave[x][y] = Ybomb_sB;
		L->next[x][y] = Xspace;
		L->cave[x][y+1] = Ybomb_s;
		L->next[x][y+1] = Xbomb_fall;
		return;
	case Xacid_1: case Xacid_2:
	case Xacid_3: case Xacid_4:
	case Xacid_5: case Xacid_6:
	case Xacid_7: case Xacid_8:
		L->cave[x][y] = Ybomb_sB;
		L->next[x][y] = Xspace;
		if(L->cave[x+1][y] == Xspace) L->cave[x+1][y] = Xsplash_e;
		if(L->cave[x-1][y] == Xspace) L->cave[x-1][y] = Xsplash_w;
		PLAY(SOUND_acid);
		return;
	}
	L->cave[x][y] = Xbomb;
	L->next[x][y] = Xbomb;
}
static void Lbomb_fall(LOGIC L, int x, int y)
{
	switch(L->cave[x][y+1]) {
	case Xspace:
	case Xsplash_e: case Xsplash_w:
		L->cave[x][y] = Ybomb_sB;
		L->next[x][y] = Xspace;
		L->cave[x][y+1] = Ybomb_s;
		L->next[x][y+1] = Xbomb_fall;
		return;
	case Xacid_1: case Xacid_2:
	case Xacid_3: case Xacid_4:
	case Xacid_5: case Xacid_6:
	case Xacid_7: case Xacid_8:
		L->cave[x][y] = Ybomb_sB;
		L->next[x][y] = Xspace;
		if(L->cave[x+1][y] == Xspace) L->cave[x+1][y] = Xsplash_e;
		if(L->cave[x-1][y] == Xspace) L->cave[x-1][y] = Xsplash_w;
		PLAY(SOUND_acid);
		return;
	}
	L->cave[x][y] = Ybomb_space;
	L->next[x][y] = Ztank;
}

static void Lnut(LOGIC L, int x, int y)
{
	switch(L->cave[x][y+1]) {
	case Xspace:
	case Xsplash_e: case Xsplash_w:
		L->cave[x][y] = Ynut_sB;
		L->next[x][y] = Xspace;
		L->cave[x][y+1] = Ynut_s;
		L->next[x][y+1] = Xnut_fall;
		return;
	case Xacid_1: case Xacid_2:
	case Xacid_3: case Xacid_4:
	case Xacid_5: case Xacid_6:
	case Xacid_7: case Xacid_8:
		L->cave[x][y] = Ynut_sB;
		L->next[x][y] = Xspace;
		if(L->cave[x+1][y] == Xspace) L->cave[x+1][y] = Xsplash_e;
		if(L->cave[x-1][y] == Xspace) L->cave[x-1][y] = Xsplash_w;
		PLAY(SOUND_acid);
		return;
	case Xandroid:
	case Xandroid_1_n: case Xandroid_2_n:
	case Xandroid_1_e: case Xandroid_2_e:
	case Xandroid_1_s: case Xandroid_2_s:
	case Xandroid_1_w: case Xandroid_2_w:
	case Xemerald:
	case Xemerald_pause:
	case Xdiamond:
	case Xdiamond_pause:
	case Xstone:
	case Xstone_pause:
	case Xbomb:
	case Xbomb_pause:
	case Xnut:
	case Xnut_pause:
	case Xspring:
	case Xspring_pause:
	case Xspring_e: case Xspring_w:
	case Xkey_1: case Xkey_2:
	case Xkey_3: case Xkey_4:
	case Xkey_5: case Xkey_6:
	case Xkey_7: case Xkey_8:
	case Xballoon:
	case Xball_1: case Xball_2:
	case Yball_1: case Yball_2:
	case Xswitch: case Yswitch:
	case Xbumper: case Ybumper:
	case Xacid_nw: case Xacid_ne:
	case Xslide_ns: case Xslide_ew:
	case Xroundwall_1: case Xroundwall_2:
	case Xroundwall_3: case Xroundwall_4:
		if(RANDOM(2)) {
			if(is_space[L->cave[x+1][y]]) {
				switch(L->cave[x+1][y+1]) {
				case Xspace:
				case Xsplash_e: case Xsplash_w:
				case Xacid_1: case Xacid_2:
				case Xacid_3: case Xacid_4:
				case Xacid_5: case Xacid_6:
				case Xacid_7: case Xacid_8:
					L->cave[x][y] = Ynut_eB;
					L->next[x][y] = Xspace;
					L->cave[x+1][y] = Ynut_e;
					L->next[x+1][y] = Xnut_pause;
					if(L->cave[x][y+1] == Xbumper) L->cave[x][y+1] = Ybumper;
					return;
				}
			}
			if(is_space[L->cave[x-1][y]]) {
				switch(L->cave[x-1][y+1]) {
				case Xspace:
				case Xsplash_e: case Xsplash_w:
				case Xacid_1: case Xacid_2:
				case Xacid_3: case Xacid_4:
				case Xacid_5: case Xacid_6:
				case Xacid_7: case Xacid_8:
					L->cave[x][y] = Ynut_wB;
					L->next[x][y] = Xspace;
					L->cave[x-1][y] = Ynut_w;
					L->next[x-1][y] = Xnut_pause;
					if(L->cave[x][y+1] == Xbumper) L->cave[x][y+1] = Ybumper;
					return;
				}
			}
		} else {
			if(is_space[L->cave[x-1][y]]) {
				switch(L->cave[x-1][y+1]) {
				case Xspace:
				case Xsplash_e: case Xsplash_w:
				case Xacid_1: case Xacid_2:
				case Xacid_3: case Xacid_4:
				case Xacid_5: case Xacid_6:
				case Xacid_7: case Xacid_8:
					L->cave[x][y] = Ynut_wB;
					L->next[x][y] = Xspace;
					L->cave[x-1][y] = Ynut_w;
					L->next[x-1][y] = Xnut_pause;
					if(L->cave[x][y+1] == Xbumper) L->cave[x][y+1] = Ybumper;
					return;
				}
			}
			if(is_space[L->cave[x+1][y]]) {
				switch(L->cave[x+1][y+1]) {
				case Xspace:
				case Xsplash_e: case Xsplash_w:
				case Xacid_1: case Xacid_2:
				case Xacid_3: case Xacid_4:
				case Xacid_5: case Xacid_6:
				case Xacid_7: case Xacid_8:
					L->cave[x][y] = Ynut_eB;
					L->next[x][y] = Xspace;
					L->cave[x+1][y] = Ynut_e;
					L->next[x+1][y] = Xnut_pause;
					if(L->cave[x][y+1] == Xbumper) L->cave[x][y+1] = Ybumper;
					return;
				}
			}
		}
	}
}
static void Lnut_pause(LOGIC L, int x, int y)
{
	switch(L->cave[x][y+1]) {
	case Xspace:
	case Xsplash_e: case Xsplash_w:
		L->cave[x][y] = Ynut_sB;
		L->next[x][y] = Xspace;
		L->cave[x][y+1] = Ynut_s;
		L->next[x][y+1] = Xnut_fall;
		return;
	case Xacid_1: case Xacid_2:
	case Xacid_3: case Xacid_4:
	case Xacid_5: case Xacid_6:
	case Xacid_7: case Xacid_8:
		L->cave[x][y] = Ynut_sB;
		L->next[x][y] = Xspace;
		if(L->cave[x+1][y] == Xspace) L->cave[x+1][y] = Xsplash_e;
		if(L->cave[x-1][y] == Xspace) L->cave[x-1][y] = Xsplash_w;
		PLAY(SOUND_acid);
		return;
	}
	L->cave[x][y] = Xnut;
	L->next[x][y] = Xnut;
}
static void Lnut_fall(LOGIC L, int x, int y)
{
	switch(L->cave[x][y+1]) {
	case Xplayer:
	case Xspace:
	case Xsplash_e: case Xsplash_w:
		L->cave[x][y] = Ynut_sB;
		L->next[x][y] = Xspace;
		L->cave[x][y+1] = Ynut_s;
		L->next[x][y+1] = Xnut_fall;
		return;
	case Xacid_1: case Xacid_2:
	case Xacid_3: case Xacid_4:
	case Xacid_5: case Xacid_6:
	case Xacid_7: case Xacid_8:
		L->cave[x][y] = Ynut_sB;
		L->next[x][y] = Xspace;
		if(L->cave[x+1][y] == Xspace) L->cave[x+1][y] = Xsplash_e;
		if(L->cave[x-1][y] == Xspace) L->cave[x-1][y] = Xsplash_w;
		PLAY(SOUND_acid);
		return;
	}
	L->cave[x][y] = Xnut;
	L->next[x][y] = Xnut;
	PLAY(SOUND_nut);
}

static void Lspring(LOGIC L, int x, int y)
{
	switch(L->cave[x][y+1]) {
	case Xspace:
	case Xsplash_e: case Xsplash_w:
	case Xplant:
		L->cave[x][y] = Yspring_sB;
		L->next[x][y] = Xspace;
		L->cave[x][y+1] = Yspring_s;
		L->next[x][y+1] = Xspring_fall;
		return;
	case Xacid_1: case Xacid_2:
	case Xacid_3: case Xacid_4:
	case Xacid_5: case Xacid_6:
	case Xacid_7: case Xacid_8:
		L->cave[x][y] = Yspring_sB;
		L->next[x][y] = Xspace;
		if(L->cave[x+1][y] == Xspace) L->cave[x+1][y] = Xsplash_e;
		if(L->cave[x-1][y] == Xspace) L->cave[x-1][y] = Xsplash_w;
		PLAY(SOUND_acid);
		return;
	case Xandroid:
	case Xandroid_1_n: case Xandroid_2_n:
	case Xandroid_1_e: case Xandroid_2_e:
	case Xandroid_1_s: case Xandroid_2_s:
	case Xandroid_1_w: case Xandroid_2_w:
	case Xemerald:
	case Xemerald_pause:
	case Xdiamond:
	case Xdiamond_pause:
	case Xstone:
	case Xstone_pause:
	case Xbomb:
	case Xbomb_pause:
	case Xnut:
	case Xnut_pause:
	case Xspring:
	case Xspring_pause:
	case Xspring_e: case Xspring_w:
	case Xkey_1: case Xkey_2:
	case Xkey_3: case Xkey_4:
	case Xkey_5: case Xkey_6:
	case Xkey_7: case Xkey_8:
	case Xballoon:
	case Xball_1: case Xball_2:
	case Yball_1: case Yball_2:
	case Xswitch: case Yswitch:
	case Xbumper: case Ybumper:
	case Xacid_nw: case Xacid_ne:
	case Xslide_ns: case Xslide_ew:
	case Xroundwall_1: case Xroundwall_2:
	case Xroundwall_3: case Xroundwall_4:
		if(RANDOM(2)) {
			if(is_space[L->cave[x+1][y]]) {
				switch(L->cave[x+1][y+1]) {
				case Xspace:
				case Xsplash_e: case Xsplash_w:
				case Xacid_1: case Xacid_2:
				case Xacid_3: case Xacid_4:
				case Xacid_5: case Xacid_6:
				case Xacid_7: case Xacid_8:
					L->cave[x][y] = Yspring_eB;
					L->next[x][y] = Xspace;
					L->cave[x+1][y] = Yspring_e;
					L->next[x+1][y] = Xspring_e;
#ifdef SPRING_GOOD
					L->next[x+1][y] = Xspring_pause;
#endif
					if(L->cave[x][y+1] == Xbumper) L->cave[x][y+1] = Ybumper;
					return;
				}
			}
			if(is_space[L->cave[x-1][y]]) {
				switch(L->cave[x-1][y+1]) {
				case Xspace:
				case Xsplash_e: case Xsplash_w:
				case Xacid_1: case Xacid_2:
				case Xacid_3: case Xacid_4:
				case Xacid_5: case Xacid_6:
				case Xacid_7: case Xacid_8:
					L->cave[x][y] = Yspring_wB;
					L->next[x][y] = Xspace;
					L->cave[x-1][y] = Yspring_w;
					L->next[x-1][y] = Xspring_w;
#ifdef SPRING_GOOD
					L->next[x-1][y] = Xspring_pause;
#endif
					if(L->cave[x][y+1] == Xbumper) L->cave[x][y+1] = Ybumper;
					return;
				}
			}
		} else {
			if(is_space[L->cave[x-1][y]]) {
				switch(L->cave[x-1][y+1]) {
				case Xspace:
				case Xsplash_e: case Xsplash_w:
				case Xacid_1: case Xacid_2:
				case Xacid_3: case Xacid_4:
				case Xacid_5: case Xacid_6:
				case Xacid_7: case Xacid_8:
					L->cave[x][y] = Yspring_wB;
					L->next[x][y] = Xspace;
					L->cave[x-1][y] = Yspring_w;
					L->next[x-1][y] = Xspring_w;
#ifdef SPRING_GOOD
					L->next[x-1][y] = Xspring_pause;
#endif
					if(L->cave[x][y+1] == Xbumper) L->cave[x][y+1] = Ybumper;
					return;
				}
			}
			if(is_space[L->cave[x+1][y]]) {
				switch(L->cave[x+1][y+1]) {
				case Xspace:
				case Xsplash_e: case Xsplash_w:
				case Xacid_1: case Xacid_2:
				case Xacid_3: case Xacid_4:
				case Xacid_5: case Xacid_6:
				case Xacid_7: case Xacid_8:
					L->cave[x][y] = Yspring_eB;
					L->next[x][y] = Xspace;
					L->cave[x+1][y] = Yspring_e;
					L->next[x+1][y] = Xspring_e;
#ifdef SPRING_GOOD
					L->next[x+1][y] = Xspring_pause;
#endif
					if(L->cave[x][y+1] == Xbumper) L->cave[x][y+1] = Ybumper;
					return;
				}
			}
		}
	}
}
static void Lspring_pause(LOGIC L, int x, int y)
{
	switch(L->cave[x][y+1]) {
	case Xspace:
	case Xsplash_e: case Xsplash_w:
		L->cave[x][y] = Yspring_sB;
		L->next[x][y] = Xspace;
		L->cave[x][y+1] = Yspring_s;
		L->next[x][y+1] = Xspring_fall;
		return;
	case Xacid_1: case Xacid_2:
	case Xacid_3: case Xacid_4:
	case Xacid_5: case Xacid_6:
	case Xacid_7: case Xacid_8:
		L->cave[x][y] = Yspring_sB;
		L->next[x][y] = Xspace;
		if(L->cave[x+1][y] == Xspace) L->cave[x+1][y] = Xsplash_e;
		if(L->cave[x-1][y] == Xspace) L->cave[x-1][y] = Xsplash_w;
		PLAY(SOUND_acid);
		return;
	}
	L->cave[x][y] = Xspring;
	L->next[x][y] = Xspring;
}
static void Lspring_fall(LOGIC L, int x, int y)
{
	switch(L->cave[x][y+1]) {
	case Xplayer:
	case Xspace:
	case Xsplash_e: case Xsplash_w:
		L->cave[x][y] = Yspring_sB;
		L->next[x][y] = Xspace;
		L->cave[x][y+1] = Yspring_s;
		L->next[x][y+1] = Xspring_fall;
		return;
	case Xacid_1: case Xacid_2:
	case Xacid_3: case Xacid_4:
	case Xacid_5: case Xacid_6:
	case Xacid_7: case Xacid_8:
		L->cave[x][y] = Yspring_sB;
		L->next[x][y] = Xspace;
		if(L->cave[x+1][y] == Xspace) L->cave[x+1][y] = Xsplash_e;
		if(L->cave[x-1][y] == Xspace) L->cave[x-1][y] = Xsplash_w;
		PLAY(SOUND_acid);
		return;
	case Xeater_n: case Xeater_e:
	case Xeater_s: case Xeater_w:
		L->cave[x][y] = Yspring_sB;
		L->next[x][y] = Xspace;
		L->cave[x][y+1] = Yeater_spring;
		L->next[x][y+1] = Zeater;
		L->score += L->eater_score;
		return;
	case Xalien:
	case Xalien_pause:
		L->cave[x][y] = Yspring_sB;
		L->next[x][y] = Xspace;
		L->cave[x][y+1] = Yalien_spring;
		L->next[x][y+1] = Ztank;
		L->score += L->alien_score;
		return;
	case Xbug_1_n: case Xbug_2_n:
		L->cave[x][y] = Yspring_sB;
		L->next[x][y] = Xspace;
		L->cave[x][y+1] = Ybug_spring_n;
		L->next[x][y+1] = Zbug;
		L->score += L->bug_score;
		return;
	case Xbug_1_e: case Xbug_2_e:
		L->cave[x][y] = Yspring_sB;
		L->next[x][y] = Xspace;
		L->cave[x][y+1] = Ybug_spring_e;
		L->next[x][y+1] = Zbug;
		L->score += L->bug_score;
		return;
	case Xbug_1_s: case Xbug_2_s:
		L->cave[x][y] = Yspring_sB;
		L->next[x][y] = Xspace;
		L->cave[x][y+1] = Ybug_spring_s;
		L->next[x][y+1] = Zbug;
		L->score += L->bug_score;
		return;
	case Xbug_1_w: case Xbug_2_w:
		L->cave[x][y] = Yspring_sB;
		L->next[x][y] = Xspace;
		L->cave[x][y+1] = Ybug_spring_w;
		L->next[x][y+1] = Zbug;
		L->score += L->bug_score;
		return;
	case Xtank_1_n: case Xtank_2_n:
		L->cave[x][y] = Yspring_sB;
		L->next[x][y] = Xspace;
		L->cave[x][y+1] = Ytank_spring_n;
		L->next[x][y+1] = Ztank;
		L->score += L->tank_score;
		return;
	case Xtank_1_e: case Xtank_2_e:
		L->cave[x][y] = Yspring_sB;
		L->next[x][y] = Xspace;
		L->cave[x][y+1] = Ytank_spring_e;
		L->next[x][y+1] = Ztank;
		L->score += L->tank_score;
		return;
	case Xtank_1_s: case Xtank_2_s:
		L->cave[x][y] = Yspring_sB;
		L->next[x][y] = Xspace;
		L->cave[x][y+1] = Ytank_spring_s;
		L->next[x][y+1] = Ztank;
		L->score += L->tank_score;
		return;
	case Xtank_1_w: case Xtank_2_w:
		L->cave[x][y] = Yspring_sB;
		L->next[x][y] = Xspace;
		L->cave[x][y+1] = Ytank_spring_w;
		L->next[x][y+1] = Ztank;
		L->score += L->tank_score;
		return;
	case Xbomb:
	case Xbomb_pause:
		L->cave[x][y] = Xspring;
		L->next[x][y] = Xspring;
		L->cave[x][y+1] = Ybomb_space;
		L->next[x][y+1] = Ztank;
		return;
	}
	L->cave[x][y] = Xspring;
	L->next[x][y] = Xspring;
	PLAY(SOUND_spring);
}
static void Lspring_e(LOGIC L, int x, int y)
{
	switch(L->cave[x][y+1]) {
	case Xspace:
	case Xsplash_e: case Xsplash_w:
		L->cave[x][y] = Yspring_sB;
		L->next[x][y] = Xspace;
		L->cave[x][y+1] = Yspring_s;
		L->next[x][y+1] = Xspring_fall;
		return;
	case Xacid_1: case Xacid_2:
	case Xacid_3: case Xacid_4:
	case Xacid_5: case Xacid_6:
	case Xacid_7: case Xacid_8:
		L->cave[x][y] = Yspring_sB;
		L->next[x][y] = Xspace;
		if(L->cave[x+1][y] == Xspace) L->cave[x+1][y] = Xsplash_e;
		if(L->cave[x-1][y] == Xspace) L->cave[x-1][y] = Xsplash_w;
		PLAY(SOUND_acid);
		return;
	case Xbumper:
		L->cave[x][y+1] = Ybumper;
	}
	switch(L->cave[x+1][y]) {
	case Xspace:
	case Xsplash_e: case Xsplash_w:
	case Yalien_nB: case Yalien_eB:
	case Yalien_sB: case Yalien_wB:
		L->cave[x][y] = Yspring_eB;
		L->next[x][y] = Xspace;
		L->cave[x+1][y] = Yspring_e;
		L->next[x+1][y] = Xspring_e;
		return;
	case Xacid_1: case Xacid_2:
	case Xacid_3: case Xacid_4:
	case Xacid_5: case Xacid_6:
	case Xacid_7: case Xacid_8:
		L->cave[x][y] = Yspring_eB;
		L->next[x][y] = Xspace;
		if(L->cave[x+2][y-1] == Xspace) L->cave[x+2][y-1] = Xsplash_e;
		if(L->cave[x][y-1] == Xspace) L->cave[x][y-1] = Xsplash_w;
		PLAY(SOUND_acid);
		return;
	case Xalien:
	case Xalien_pause:
	case Yalien_n:
	case Yalien_e:
	case Yalien_s:
	case Yalien_w:
		L->cave[x][y] = Yspring_alien_eB;
		L->next[x][y] = Xspace;
		L->cave[x+1][y] = Yspring_alien_e;
		L->next[x+1][y] = Xspring_e;
		L->sound[SOUND_slurp] = 1;
		L->score += L->spring_score;
		return;
	case Xbumper: case Ybumper:
		L->cave[x+1][y] = Ybumper;
		L->next[x][y] = Xspring_w;
		PLAY(SOUND_spring);
		return;
	}
	L->cave[x][y] = Xspring;
	L->next[x][y] = Xspring;
	PLAY(SOUND_spring);
}
static void Lspring_w(LOGIC L, int x, int y)
{
	switch(L->cave[x][y+1]) {
	case Xspace:
	case Xsplash_e: case Xsplash_w:
		L->cave[x][y] = Yspring_sB;
		L->next[x][y] = Xspace;
		L->cave[x][y+1] = Yspring_s;
		L->next[x][y+1] = Xspring_fall;
		return;
	case Xacid_1: case Xacid_2:
	case Xacid_3: case Xacid_4:
	case Xacid_5: case Xacid_6:
	case Xacid_7: case Xacid_8:
		L->cave[x][y] = Yspring_sB;
		L->next[x][y] = Xspace;
		if(L->cave[x+1][y] == Xspace) L->cave[x+1][y] = Xsplash_e;
		if(L->cave[x-1][y] == Xspace) L->cave[x-1][y] = Xsplash_w;
		PLAY(SOUND_acid);
		return;
	case Xbumper:
		L->cave[x][y+1] = Ybumper;
	}
	switch(L->cave[x-1][y]) {
	case Xspace:
	case Xsplash_e: case Xsplash_w:
	case Yalien_nB: case Yalien_eB:
	case Yalien_sB: case Yalien_wB:
		L->cave[x][y] = Yspring_wB;
		L->next[x][y] = Xspace;
		L->cave[x-1][y] = Yspring_w;
		L->next[x-1][y] = Xspring_w;
		return;
	case Xacid_1: case Xacid_2:
	case Xacid_3: case Xacid_4:
	case Xacid_5: case Xacid_6:
	case Xacid_7: case Xacid_8:
		L->cave[x][y] = Yspring_wB;
		L->next[x][y] = Xspace;
		if(L->cave[x][y-1] == Xspace) L->cave[x][y-1] = Xsplash_e;
		if(L->cave[x-2][y-1] == Xspace) L->cave[x-2][y-1] = Xsplash_w;
		PLAY(SOUND_acid);
		return;
	case Xalien:
	case Xalien_pause:
	case Yalien_n:
	case Yalien_e:
	case Yalien_s:
	case Yalien_w:
		L->cave[x][y] = Yspring_alien_wB;
		L->next[x][y] = Xspace;
		L->cave[x-1][y] = Yspring_alien_w;
		L->next[x-1][y] = Xspring_w;
		L->sound[SOUND_slurp] = 1;
		L->score += L->spring_score;
		return;
	case Xbumper: case Ybumper:
		L->cave[x-1][y] = Ybumper;
		L->next[x][y] = Xspring_e;
		PLAY(SOUND_spring);
		return;
	}
	L->cave[x][y] = Xspring;
	L->next[x][y] = Xspring;
	PLAY(SOUND_spring);
}

static void Lpush_emerald_e(LOGIC L, int x, int y)
{
	L->cave[x][y] = Yemerald_eB;
	L->next[x][y] = Xspace;
	switch(L->cave[x+1][y]) {
	case Zborder:
	case Zboom:
	case Zbug:
	case Ztank:
	case Zeater:
	case Zdynamite:
	case Xchain:
		return;
	case Xacid_1: case Xacid_2:
	case Xacid_3: case Xacid_4:
	case Xacid_5: case Xacid_6:
	case Xacid_7: case Xacid_8:
		if(L->cave[x+2][y-1] == Xspace) L->cave[x+2][y-1] = Xsplash_e;
		if(L->cave[x][y-1] == Xspace) L->cave[x][y-1] = Xsplash_w;
		PLAY(SOUND_acid);
		return;
	}
	L->cave[x+1][y] = Yemerald_e;
	L->next[x+1][y] = Xemerald_pause;
}
static void Lpush_emerald_w(LOGIC L, int x, int y)
{
	L->cave[x][y] = Yemerald_wB;
	L->next[x][y] = Xspace;
	switch(L->cave[x-1][y]) {
	case Zborder:
	case Zboom:
	case Zbug:
	case Ztank:
	case Zeater:
	case Zdynamite:
	case Xchain:
		return;
	case Xacid_1: case Xacid_2:
	case Xacid_3: case Xacid_4:
	case Xacid_5: case Xacid_6:
	case Xacid_7: case Xacid_8:
		if(L->cave[x][y-1] == Xspace) L->cave[x][y-1] = Xsplash_e;
		if(L->cave[x-2][y-1] == Xspace) L->cave[x-2][y-1] = Xsplash_w;
		PLAY(SOUND_acid);
		return;
	}
	L->cave[x-1][y] = Yemerald_w;
	L->next[x-1][y] = Xemerald_pause;
}
static void Lpush_diamond_e(LOGIC L, int x, int y)
{
	L->cave[x][y] = Ydiamond_eB;
	L->next[x][y] = Xspace;
	switch(L->cave[x+1][y]) {
	case Zborder:
	case Zboom:
	case Zbug:
	case Ztank:
	case Zeater:
	case Zdynamite:
	case Xchain:
		return;
	case Xacid_1: case Xacid_2:
	case Xacid_3: case Xacid_4:
	case Xacid_5: case Xacid_6:
	case Xacid_7: case Xacid_8:
		if(L->cave[x+2][y-1] == Xspace) L->cave[x+2][y-1] = Xsplash_e;
		if(L->cave[x][y-1] == Xspace) L->cave[x][y-1] = Xsplash_w;
		PLAY(SOUND_acid);
		return;
	}
	L->cave[x+1][y] = Ydiamond_e;
	L->next[x+1][y] = Xdiamond_pause;
}
static void Lpush_diamond_w(LOGIC L, int x, int y)
{
	L->cave[x][y] = Ydiamond_wB;
	L->next[x][y] = Xspace;
	switch(L->cave[x-1][y]) {
	case Zborder:
	case Zboom:
	case Zbug:
	case Ztank:
	case Zeater:
	case Zdynamite:
	case Xchain:
		return;
	case Xacid_1: case Xacid_2:
	case Xacid_3: case Xacid_4:
	case Xacid_5: case Xacid_6:
	case Xacid_7: case Xacid_8:
		if(L->cave[x][y-1] == Xspace) L->cave[x][y-1] = Xsplash_e;
		if(L->cave[x-2][y-1] == Xspace) L->cave[x-2][y-1] = Xsplash_w;
		PLAY(SOUND_acid);
		return;
	}
	L->cave[x-1][y] = Ydiamond_w;
	L->next[x-1][y] = Xdiamond_pause;
}
static void Lpush_stone_e(LOGIC L, int x, int y)
{
	L->cave[x][y] = Ystone_eB;
	L->next[x][y] = Xspace;
	switch(L->cave[x+1][y]) {
	case Zborder:
	case Zboom:
	case Zbug:
	case Ztank:
	case Zeater:
	case Zdynamite:
	case Xchain:
		return;
	case Xacid_1: case Xacid_2:
	case Xacid_3: case Xacid_4:
	case Xacid_5: case Xacid_6:
	case Xacid_7: case Xacid_8:
		if(L->cave[x+2][y-1] == Xspace) L->cave[x+2][y-1] = Xsplash_e;
		if(L->cave[x][y-1] == Xspace) L->cave[x][y-1] = Xsplash_w;
		PLAY(SOUND_acid);
		return;
	}
	L->cave[x+1][y] = Ystone_e;
	L->next[x+1][y] = Xstone_pause;
}
static void Lpush_stone_w(LOGIC L, int x, int y)
{
	L->cave[x][y] = Ystone_wB;
	L->next[x][y] = Xspace;
	switch(L->cave[x-1][y]) {
	case Zborder:
	case Zboom:
	case Zbug:
	case Ztank:
	case Zeater:
	case Zdynamite:
	case Xchain:
		return;
	case Xacid_1: case Xacid_2:
	case Xacid_3: case Xacid_4:
	case Xacid_5: case Xacid_6:
	case Xacid_7: case Xacid_8:
		if(L->cave[x][y-1] == Xspace) L->cave[x][y-1] = Xsplash_e;
		if(L->cave[x-2][y-1] == Xspace) L->cave[x-2][y-1] = Xsplash_w;
		PLAY(SOUND_acid);
		return;
	}
	L->cave[x-1][y] = Ystone_w;
	L->next[x-1][y] = Xstone_pause;
}
static void Lpush_bomb_e(LOGIC L, int x, int y)
{
	L->cave[x][y] = Ybomb_eB;
	L->next[x][y] = Xspace;
	switch(L->cave[x+1][y]) {
	case Zborder:
	case Zboom:
	case Zbug:
	case Ztank:
	case Zeater:
	case Zdynamite:
	case Xchain:
		return;
	case Xacid_1: case Xacid_2:
	case Xacid_3: case Xacid_4:
	case Xacid_5: case Xacid_6:
	case Xacid_7: case Xacid_8:
		if(L->cave[x+2][y-1] == Xspace) L->cave[x+2][y-1] = Xsplash_e;
		if(L->cave[x][y-1] == Xspace) L->cave[x][y-1] = Xsplash_w;
		PLAY(SOUND_acid);
		return;
	}
	L->cave[x+1][y] = Ybomb_e;
	L->next[x+1][y] = Xbomb_pause;
}
static void Lpush_bomb_w(LOGIC L, int x, int y)
{
	L->cave[x][y] = Ybomb_wB;
	L->next[x][y] = Xspace;
	switch(L->cave[x-1][y]) {
	case Zborder:
	case Zboom:
	case Zbug:
	case Ztank:
	case Zeater:
	case Zdynamite:
	case Xchain:
		return;
	case Xacid_1: case Xacid_2:
	case Xacid_3: case Xacid_4:
	case Xacid_5: case Xacid_6:
	case Xacid_7: case Xacid_8:
		if(L->cave[x][y-1] == Xspace) L->cave[x][y-1] = Xsplash_e;
		if(L->cave[x-2][y-1] == Xspace) L->cave[x-2][y-1] = Xsplash_w;
		PLAY(SOUND_acid);
		return;
	}
	L->cave[x-1][y] = Ybomb_w;
	L->next[x-1][y] = Xbomb_pause;
}
static void Lpush_nut_e(LOGIC L, int x, int y)
{
	L->cave[x][y] = Ynut_eB;
	L->next[x][y] = Xspace;
	switch(L->cave[x+1][y]) {
	case Zborder:
	case Zboom:
	case Zbug:
	case Ztank:
	case Zeater:
	case Zdynamite:
	case Xchain:
		return;
	case Xacid_1: case Xacid_2:
	case Xacid_3: case Xacid_4:
	case Xacid_5: case Xacid_6:
	case Xacid_7: case Xacid_8:
		if(L->cave[x+2][y-1] == Xspace) L->cave[x+2][y-1] = Xsplash_e;
		if(L->cave[x][y-1] == Xspace) L->cave[x][y-1] = Xsplash_w;
		PLAY(SOUND_acid);
		return;
	}
	L->cave[x+1][y] = Ynut_e;
	L->next[x+1][y] = Xnut_pause;
}
static void Lpush_nut_w(LOGIC L, int x, int y)
{
	L->cave[x][y] = Ynut_wB;
	L->next[x][y] = Xspace;
	switch(L->cave[x-1][y]) {
	case Zborder:
	case Zboom:
	case Zbug:
	case Ztank:
	case Zeater:
	case Zdynamite:
	case Xchain:
		return;
	case Xacid_1: case Xacid_2:
	case Xacid_3: case Xacid_4:
	case Xacid_5: case Xacid_6:
	case Xacid_7: case Xacid_8:
		if(L->cave[x][y-1] == Xspace) L->cave[x][y-1] = Xsplash_e;
		if(L->cave[x-2][y-1] == Xspace) L->cave[x-2][y-1] = Xsplash_w;
		PLAY(SOUND_acid);
		return;
	}
	L->cave[x-1][y] = Ynut_w;
	L->next[x-1][y] = Xnut_pause;
}
static void Lpush_spring_e(LOGIC L, int x, int y)
{
	L->cave[x][y] = Yspring_eB;
	L->next[x][y] = Xspace;
	switch(L->cave[x+1][y]) {
	case Zborder:
	case Zboom:
	case Zbug:
	case Ztank:
	case Zeater:
	case Zdynamite:
	case Xchain:
		return;
	case Xacid_1: case Xacid_2:
	case Xacid_3: case Xacid_4:
	case Xacid_5: case Xacid_6:
	case Xacid_7: case Xacid_8:
		if(L->cave[x+2][y-1] == Xspace) L->cave[x+2][y-1] = Xsplash_e;
		if(L->cave[x][y-1] == Xspace) L->cave[x][y-1] = Xsplash_w;
		PLAY(SOUND_acid);
		return;
	}
	L->cave[x+1][y] = Yspring_e;
	L->next[x+1][y] = Xspring_e;
}
static void Lpush_spring_w(LOGIC L, int x, int y)
{
	L->cave[x][y] = Yspring_wB;
	L->next[x][y] = Xspace;
	switch(L->cave[x-1][y]) {
	case Zborder:
	case Zboom:
	case Zbug:
	case Ztank:
	case Zeater:
	case Zdynamite:
	case Xchain:
		return;
	case Xacid_1: case Xacid_2:
	case Xacid_3: case Xacid_4:
	case Xacid_5: case Xacid_6:
	case Xacid_7: case Xacid_8:
		if(L->cave[x][y-1] == Xspace) L->cave[x][y-1] = Xsplash_e;
		if(L->cave[x-2][y-1] == Xspace) L->cave[x-2][y-1] = Xsplash_w;
		PLAY(SOUND_acid);
		return;
	}
	L->cave[x-1][y] = Yspring_w;
	L->next[x-1][y] = Xspring_w;
}

static void Ldynamite_1(LOGIC L, int x, int y)
{
	L->next[x][y] = Xdynamite_2;
	L->sound[SOUND_tick] = 1;
}
static void Ldynamite_2(LOGIC L, int x, int y)
{
	L->next[x][y] = Xdynamite_3;
	L->sound[SOUND_tick] = 1;
}
static void Ldynamite_3(LOGIC L, int x, int y)
{
	L->next[x][y] = Xdynamite_4;
	L->sound[SOUND_tick] = 1;
}
static void Ldynamite_4(LOGIC L, int x, int y)
{
	L->next[x][y] = Zdynamite;
	L->sound[SOUND_tick] = 1;
}

static void Lfake_door_1(LOGIC L, int x, int y)
{
	if(L->magnify_cnt) L->cave[x][y] = Xdoor_1;
}
static void Lfake_door_2(LOGIC L, int x, int y)
{
	if(L->magnify_cnt) L->cave[x][y] = Xdoor_2;
}
static void Lfake_door_3(LOGIC L, int x, int y)
{
	if(L->magnify_cnt) L->cave[x][y] = Xdoor_3;
}
static void Lfake_door_4(LOGIC L, int x, int y)
{
	if(L->magnify_cnt) L->cave[x][y] = Xdoor_4;
}
static void Lfake_door_5(LOGIC L, int x, int y)
{
	if(L->magnify_cnt) L->cave[x][y] = Xdoor_5;
}
static void Lfake_door_6(LOGIC L, int x, int y)
{
	if(L->magnify_cnt) L->cave[x][y] = Xdoor_6;
}
static void Lfake_door_7(LOGIC L, int x, int y)
{
	if(L->magnify_cnt) L->cave[x][y] = Xdoor_7;
}
static void Lfake_door_8(LOGIC L, int x, int y)
{
	if(L->magnify_cnt) L->cave[x][y] = Xdoor_8;
}

static void Lballoon(LOGIC L, int x, int y)
{
	if(L->wind_cnt == 0) return;
	switch(L->wind_direction) {
	case 0: /* north */
		switch(L->cave[x][y-1]) {
		case Xspace:
		case Xsplash_e: case Xsplash_w:
			L->cave[x][y] = Yballoon_nB;
			L->next[x][y] = Xspace;
			L->cave[x][y-1] = Yballoon_n;
			L->next[x][y-1] = Xballoon;
			return;
		case Xacid_1: case Xacid_2:
		case Xacid_3: case Xacid_4:
		case Xacid_5: case Xacid_6:
		case Xacid_7: case Xacid_8:
			L->cave[x][y] = Yballoon_nB;
			L->next[x][y] = Xspace;
			if(L->cave[x+1][y-2] == Xspace) L->cave[x+1][y-2] = Xsplash_e;
			if(L->cave[x-1][y-2] == Xspace) L->cave[x-1][y-2] = Xsplash_w;
			PLAY(SOUND_acid);
			return;
		}
		break;
	case 1: /* east */
		switch(L->cave[x+1][y]) {
		case Xspace:
		case Xsplash_e: case Xsplash_w:
			L->cave[x][y] = Yballoon_eB;
			L->next[x][y] = Xspace;
			L->cave[x+1][y] = Yballoon_e;
			L->next[x+1][y] = Xballoon;
			return;
		case Xacid_1: case Xacid_2:
		case Xacid_3: case Xacid_4:
		case Xacid_5: case Xacid_6:
		case Xacid_7: case Xacid_8:
			L->cave[x][y] = Yballoon_eB;
			L->next[x][y] = Xspace;
			if(L->cave[x+2][y-1] == Xspace) L->cave[x+2][y-1] = Xsplash_e;
			if(L->cave[x][y-1] == Xspace) L->cave[x][y-1] = Xsplash_w;
			PLAY(SOUND_acid);
			return;
		}
		break;
	case 2: /* south */
		switch(L->cave[x][y+1]) {
		case Xspace:
		case Xsplash_e: case Xsplash_w:
			L->cave[x][y] = Yballoon_sB;
			L->next[x][y] = Xspace;
			L->cave[x][y+1] = Yballoon_s;
			L->next[x][y+1] = Xballoon;
			return;
		case Xacid_1: case Xacid_2:
		case Xacid_3: case Xacid_4:
		case Xacid_5: case Xacid_6:
		case Xacid_7: case Xacid_8:
			L->cave[x][y] = Yballoon_sB;
			L->next[x][y] = Xspace;
			if(L->cave[x+1][y] == Xspace) L->cave[x+1][y] = Xsplash_e;
			if(L->cave[x-1][y] == Xspace) L->cave[x-1][y] = Xsplash_w;
			PLAY(SOUND_acid);
			return;
		}
		break;
	case 3: /* west */
		switch(L->cave[x-1][y]) {
		case Xspace:
		case Xsplash_e: case Xsplash_w:
			L->cave[x][y] = Yballoon_wB;
			L->next[x][y] = Xspace;
			L->cave[x-1][y] = Yballoon_w;
			L->next[x-1][y] = Xballoon;
			return;
		case Xacid_1: case Xacid_2:
		case Xacid_3: case Xacid_4:
		case Xacid_5: case Xacid_6:
		case Xacid_7: case Xacid_8:
			L->cave[x][y] = Yballoon_wB;
			L->next[x][y] = Xspace;
			if(L->cave[x][y-1] == Xspace) L->cave[x][y-1] = Xsplash_e;
			if(L->cave[x-2][y-1] == Xspace) L->cave[x-2][y-1] = Xsplash_w;
			PLAY(SOUND_acid);
			return;
		}
		break;
	}
}

static void Lball_common(LOGIC L, int x, int y)
{
	L->sound[SOUND_ball] = 1;
	if(L->ball_random) {
		switch(RANDOM(8)) {
		case 0:
			if(L->ball_array[0] != Xspace && is_space[L->cave[x-1][y-1]]) {
				L->cave[x-1][y-1] = Xpop;
				L->next[x-1][y-1] = L->ball_array[0];
			}
			break;
		case 1:
			if(L->ball_array[1] != Xspace && is_space[L->cave[x][y-1]]) {
				L->cave[x][y-1] = Xpop;
				L->next[x][y-1] = L->ball_array[1];
			}
			break;
		case 2:
			if(L->ball_array[2] != Xspace && is_space[L->cave[x+1][y-1]]) {
				L->cave[x+1][y-1] = Xpop;
				L->next[x+1][y-1] = L->ball_array[2];
			}
			break;
		case 3:
			if(L->ball_array[3] != Xspace && is_space[L->cave[x-1][y]]) {
				L->cave[x-1][y] = Xpop;
				L->next[x-1][y] = L->ball_array[3];
			}
			break;
		case 4:
			if(L->ball_array[4] != Xspace && is_space[L->cave[x+1][y]]) {
				L->cave[x+1][y] = Xpop;
				L->next[x+1][y] = L->ball_array[4];
			}
			break;
		case 5:
			if(L->ball_array[5] != Xspace && is_space[L->cave[x-1][y+1]]) {
				L->cave[x-1][y+1] = Xpop;
				L->next[x-1][y+1] = L->ball_array[5];
			}
			break;
		case 6:
			if(L->ball_array[6] != Xspace && is_space[L->cave[x][y+1]]) {
				L->cave[x][y+1] = Xpop;
				L->next[x][y+1] = L->ball_array[6];
			}
			break;
		case 7:
			if(L->ball_array[7] != Xspace && is_space[L->cave[x+1][y+1]]) {
				L->cave[x+1][y+1] = Xpop;
				L->next[x+1][y+1] = L->ball_array[7];
			}
			break;
		}
	} else {
		if(L->ball_array[0] != Xspace && is_space[L->cave[x-1][y-1]]) {
			L->cave[x-1][y-1] = Xpop;
			L->next[x-1][y-1] = L->ball_array[0];
		}
		if(L->ball_array[1] != Xspace && is_space[L->cave[x][y-1]]) {
			L->cave[x][y-1] = Xpop;
			L->next[x][y-1] = L->ball_array[1];
		}
		if(L->ball_array[2] != Xspace && is_space[L->cave[x+1][y-1]]) {
			L->cave[x+1][y-1] = Xpop;
			L->next[x+1][y-1] = L->ball_array[2];
		}
		if(L->ball_array[3] != Xspace && is_space[L->cave[x-1][y]]) {
			L->cave[x-1][y] = Xpop;
			L->next[x-1][y] = L->ball_array[3];
		}
		if(L->ball_array[4] != Xspace && is_space[L->cave[x+1][y]]) {
			L->cave[x+1][y] = Xpop;
			L->next[x+1][y] = L->ball_array[4];
		}
		if(L->ball_array[5] != Xspace && is_space[L->cave[x-1][y+1]]) {
			L->cave[x-1][y+1] = Xpop;
			L->next[x-1][y+1] = L->ball_array[5];
		}
		if(L->ball_array[6] != Xspace && is_space[L->cave[x][y+1]]) {
			L->cave[x][y+1] = Xpop;
			L->next[x][y+1] = L->ball_array[6];
		}
		if(L->ball_array[7] != Xspace && is_space[L->cave[x+1][y+1]]) {
			L->cave[x+1][y+1] = Xpop;
			L->next[x+1][y+1] = L->ball_array[7];
		}
	}
}
static void Lball_1(LOGIC L, int x, int y)
{
	if(L->ball_state == 0) return;
	L->cave[x][y] = Yball_1;
	L->next[x][y] = Xball_2;
	if(L->ball_cnt) return;
	Lball_common(L, x, y);
}
static void Lball_2(LOGIC L, int x, int y)
{
	if(L->ball_state == 0) return;
	L->cave[x][y] = Yball_2;
	L->next[x][y] = Xball_1;
	if(L->ball_cnt) return;
	Lball_common(L, x, y);
}

static void Ldrip(LOGIC L, int x, int y)
{
	L->next[x][y] = Xdrip_1_sB;
}
static void Ldrip_1_sB(LOGIC L, int x, int y)
{
	int temp;
	switch(L->cave[x][y+1]) {
	case Xplayer:
	case Xspace:
	case Xsplash_e: case Xsplash_w:
	case Xplant:
		L->cave[x][y] = Xdrip_1_sB;
		L->next[x][y] = Xdrip_2_sB;
		L->cave[x][y+1] = Xdrip_1_s;
		L->next[x][y+1] = Xdrip_2_s;
		return;
	case Xacid_1: case Xacid_2:
	case Xacid_3: case Xacid_4:
	case Xacid_5: case Xacid_6:
	case Xacid_7: case Xacid_8:
		L->cave[x][y] = Xdrip_1_sB;
		L->next[x][y] = Xdrip_2_sB;
		if(L->cave[x+1][y] == Xspace) L->cave[x+1][y] = Xsplash_e;
		if(L->cave[x-1][y] == Xspace) L->cave[x-1][y] = Xsplash_w;
		PLAY(SOUND_acid);
		return;
	}
	temp = Xfake_ameuba;
	switch(RANDOM(8)) {
	case 0: temp = Xameuba_1; break;
	case 1: temp = Xameuba_2; break;
	case 2: temp = Xameuba_3; break;
	case 3: temp = Xameuba_4; break;
	case 4: temp = Xameuba_5; break;
	case 5: temp = Xameuba_6; break;
	case 6: temp = Xameuba_7; break;
	case 7: temp = Xameuba_8; break;
	}
	L->cave[x][y] = temp;
	L->next[x][y] = temp;
	L->sound[SOUND_drip] = 1;
}
static void Ldrip_2_sB(LOGIC L, int x, int y)
{
	L->next[x][y] = Xspace;
}
static void Ldrip_1_s(LOGIC L, int x, int y)
{
	L->next[x][y] = Xdrip_2_s;
}
static void Ldrip_2_s(LOGIC L, int x, int y)
{
	L->next[x][y] = Xdrip_1_sB;
}

static void Lwonderwall(LOGIC L, int x, int y)
{
	if(L->wonderwall_time && L->wonderwall_state) L->cave[x][y] = Ywonderwall;
}

static void Lwheel(LOGIC L, int x, int y)
{
	if(L->wheel_cnt && x == L->wheel_x && y == L->wheel_y) L->cave[x][y] = Ywheel;
}

static void Lswitch(LOGIC L, int x, int y)
{
	if(L->ball_state) L->cave[x][y] = Yswitch;
}

static void Lfake_space(LOGIC L, int x, int y)
{
	if(L->lenses_cnt) L->cave[x][y] = Yfake_space;
}
static void Lfake_grass(LOGIC L, int x, int y)
{
	if(L->magnify_cnt) L->cave[x][y] = Yfake_grass;
}
static void Lfake_ameuba(LOGIC L, int x, int y)
{
	if(L->lenses_cnt) L->cave[x][y] = Yfake_ameuba;
}

static void Lsand_stone(LOGIC L, int x, int y)
{
	switch(L->cave[x][y+1]) {
	case Xspace:
	case Xsplash_e: case Xsplash_w:
		L->cave[x][y] = Xsand_sand_3_sB;
		L->next[x][y] = Xsand_sand_4_sB;
		L->cave[x][y+1] = Xsand_stone_1_s;
		L->next[x][y+1] = Xsand_stone_2_s;
		return;
	case Xacid_1: case Xacid_2:
	case Xacid_3: case Xacid_4:
	case Xacid_5: case Xacid_6:
	case Xacid_7: case Xacid_8:
		L->cave[x][y] = Xsand_sand_3_sB;
		L->next[x][y] = Xsand_sand_4_sB;
		if(L->cave[x+1][y] == Xspace) L->cave[x+1][y] = Xsplash_e;
		if(L->cave[x-1][y] == Xspace) L->cave[x-1][y] = Xsplash_w;
		PLAY(SOUND_acid);
		return;
	case Xsand:
		L->cave[x][y] = Xsand_sand_1_sB;
		L->next[x][y] = Xsand_sand_2_sB;
		L->cave[x][y+1] = Xsand_sand_1_s;
		L->next[x][y+1] = Xsand_sand_2_s;
		return;
	}
}
static void Lsand_stone_1_sB(LOGIC L, int x, int y)
{
	L->next[x][y] = Xsand_stone_2_sB;
}
static void Lsand_stone_2_sB(LOGIC L, int x, int y)
{
	L->next[x][y] = Xsand_stone_3_sB;
}
static void Lsand_stone_3_sB(LOGIC L, int x, int y)
{
	L->next[x][y] = Xsand_stone_4_sB;
}
static void Lsand_stone_4_sB(LOGIC L, int x, int y)
{
	L->next[x][y] = Xspace;
}
static void Lsand_sand_1_s(LOGIC L, int x, int y)
{
	L->next[x][y] = Xsand_sand_2_s;
}
static void Lsand_sand_2_s(LOGIC L, int x, int y)
{
	L->next[x][y] = Xsand_sand_3_s;
}
static void Lsand_sand_3_s(LOGIC L, int x, int y)
{
	L->next[x][y] = Xsand_sand_4_s;
}
static void Lsand_sand_4_s(LOGIC L, int x, int y)
{
	L->next[x][y] = Xsand_stone;
}
static void Lsand_sand_1_sB(LOGIC L, int x, int y)
{
	L->next[x][y] = Xsand_sand_2_sB;
}
static void Lsand_sand_2_sB(LOGIC L, int x, int y)
{
	L->next[x][y] = Xsand_sand_3_sB;
}
static void Lsand_sand_3_sB(LOGIC L, int x, int y)
{
	L->next[x][y] = Xsand_sand_4_sB;
}
static void Lsand_sand_4_sB(LOGIC L, int x, int y)
{
	L->next[x][y] = Xsand;
}
static void Lsand_stone_1_s(LOGIC L, int x, int y)
{
	L->next[x][y] = Xsand_stone_2_s;
}
static void Lsand_stone_2_s(LOGIC L, int x, int y)
{
	L->next[x][y] = Xstone_fall;
}

static void Lslide_ns(LOGIC L, int x, int y)
{
	if(is_space[L->cave[x][y-1]]) {
		L->cave[x][y-1] = Yslide_ns;
		L->next[x][y-1] = Xslide_ns;
		L->sound[SOUND_slide] = 1;
	}
	if(is_space[L->cave[x][y+1]]) {
		L->cave[x][y+1] = Yslide_ns;
		L->next[x][y+1] = Xslide_ns;
		L->sound[SOUND_slide] = 1;
	}
}
static void Lslide_ew(LOGIC L, int x, int y)
{
	if(is_space[L->cave[x+1][y]]) {
		L->cave[x+1][y] = Yslide_ew;
		L->next[x+1][y] = Xslide_ew;
		L->sound[SOUND_slide] = 1;
	}
	if(is_space[L->cave[x-1][y]]) {
		L->cave[x-1][y] = Yslide_ew;
		L->next[x-1][y] = Xslide_ew;
		L->sound[SOUND_slide] = 1;
	}
}

static void Lexit(LOGIC L, int x, int y)
{
	if(L->diamonds) return;
	switch(RANDOM(3)) {
	case 0: L->cave[x][y] = Xexit_1; L->next[x][y] = Xexit_2; break;
	case 1: L->cave[x][y] = Xexit_2; L->next[x][y] = Xexit_3; break;
	case 2: L->cave[x][y] = Xexit_3; L->next[x][y] = Xexit_1; break;
	}
}
static void Lexit_1(LOGIC L, int x, int y)
{
	L->next[x][y] = Xexit_2;
}
static void Lexit_2(LOGIC L, int x, int y)
{
	L->next[x][y] = Xexit_3;
}
static void Lexit_3(LOGIC L, int x, int y)
{
	L->next[x][y] = Xexit_1;
}

static void Lpause(LOGIC L, int x, int y)
{
	L->next[x][y] = Xspace;
}

static const FUNC Lfunc[XMAX] = {
	[Xchain] = Lchain,
	[Xboom_android] = Lboom_1,
	[Xboom_1] = Lboom_1,
	[Xboom_2] = Lboom_2,

	[Xacid_1] = Lacid_1,
	[Xacid_2] = Lacid_2,
	[Xacid_3] = Lacid_3,
	[Xacid_4] = Lacid_4,
	[Xacid_5] = Lacid_5,
	[Xacid_6] = Lacid_6,
	[Xacid_7] = Lacid_7,
	[Xacid_8] = Lacid_8,

	[Xandroid] = Landroid,
	[Xandroid_1_n] = Landroid_1_n,
	[Xandroid_2_n] = Landroid_2_n,
	[Xandroid_1_e] = Landroid_1_e,
	[Xandroid_2_e] = Landroid_2_e,
	[Xandroid_1_s] = Landroid_1_s,
	[Xandroid_2_s] = Landroid_2_s,
	[Xandroid_1_w] = Landroid_1_w,
	[Xandroid_2_w] = Landroid_2_w,

	[Xeater_n] = Leater_n,
	[Xeater_e] = Leater_e,
	[Xeater_s] = Leater_s,
	[Xeater_w] = Leater_w,

	[Xalien] = Lalien,
	[Xalien_pause] = Lalien_pause,

	[Xbug_1_n] = Lbug_1_n,
	[Xbug_2_n] = Lbug_2_n,
	[Xbug_1_e] = Lbug_1_e,
	[Xbug_2_e] = Lbug_2_e,
	[Xbug_1_s] = Lbug_1_s,
	[Xbug_2_s] = Lbug_2_s,
	[Xbug_1_w] = Lbug_1_w,
	[Xbug_2_w] = Lbug_2_w,

	[Xtank_1_n] = Ltank_1_n,
	[Xtank_2_n] = Ltank_2_n,
	[Xtank_1_e] = Ltank_1_e,
	[Xtank_2_e] = Ltank_2_e,
	[Xtank_1_s] = Ltank_1_s,
	[Xtank_2_s] = Ltank_2_s,
	[Xtank_1_w] = Ltank_1_w,
	[Xtank_2_w] = Ltank_2_w,

	[Xemerald] = Lemerald,
	[Xemerald_pause] = Lemerald_pause,
	[Xemerald_fall] = Lemerald_fall,

	[Xdiamond] = Ldiamond,
	[Xdiamond_pause] = Ldiamond_pause,
	[Xdiamond_fall] = Ldiamond_fall,

	[Xstone] = Lstone,
	[Xstone_pause] = Lstone_pause,
	[Xstone_fall] = Lstone_fall,

	[Xbomb] = Lbomb,
	[Xbomb_pause] = Lbomb_pause,
	[Xbomb_fall] = Lbomb_fall,

	[Xnut] = Lnut,
	[Xnut_pause] = Lnut_pause,
	[Xnut_fall] = Lnut_fall,

	[Xspring] = Lspring,
	[Xspring_pause] = Lspring_pause,
	[Xspring_fall] = Lspring_fall,
	[Xspring_e] = Lspring_e,
	[Xspring_w] = Lspring_w,

	[Xpush_emerald_e] = Lpush_emerald_e,
	[Xpush_emerald_w] = Lpush_emerald_w,
	[Xpush_diamond_e] = Lpush_diamond_e,
	[Xpush_diamond_w] = Lpush_diamond_w,
	[Xpush_stone_e] = Lpush_stone_e,
	[Xpush_stone_w] = Lpush_stone_w,
	[Xpush_bomb_e] = Lpush_bomb_e,
	[Xpush_bomb_w] = Lpush_bomb_w,
	[Xpush_nut_e] = Lpush_nut_e,
	[Xpush_nut_w] = Lpush_nut_w,
	[Xpush_spring_e] = Lpush_spring_e,
	[Xpush_spring_w] = Lpush_spring_w,

	[Xdynamite_1] = Ldynamite_1,
	[Xdynamite_2] = Ldynamite_2,
	[Xdynamite_3] = Ldynamite_3,
	[Xdynamite_4] = Ldynamite_4,

	[Xfake_door_1] = Lfake_door_1,
	[Xfake_door_2] = Lfake_door_2,
	[Xfake_door_3] = Lfake_door_3,
	[Xfake_door_4] = Lfake_door_4,
	[Xfake_door_5] = Lfake_door_5,
	[Xfake_door_6] = Lfake_door_6,
	[Xfake_door_7] = Lfake_door_7,
	[Xfake_door_8] = Lfake_door_8,

	[Xballoon] = Lballoon,

	[Xball_1] = Lball_1,
	[Xball_2] = Lball_2,

	[Xdrip] = Ldrip,
	[Xdrip_1_sB] = Ldrip_1_sB,
	[Xdrip_2_sB] = Ldrip_2_sB,
	[Xdrip_1_s] = Ldrip_1_s,
	[Xdrip_2_s] = Ldrip_2_s,

	[Xwonderwall] = Lwonderwall,
	[Xwheel] = Lwheel,
	[Xswitch] = Lswitch,

	[Xfake_space] = Lfake_space,
	[Xfake_grass] = Lfake_grass,
	[Xfake_ameuba] = Lfake_ameuba,

	[Xsand_stone] = Lsand_stone,
	[Xsand_stone_1_sB] = Lsand_stone_1_sB,
	[Xsand_stone_2_sB] = Lsand_stone_2_sB,
	[Xsand_stone_3_sB] = Lsand_stone_3_sB,
	[Xsand_stone_4_sB] = Lsand_stone_4_sB,
	[Xsand_sand_1_s] = Lsand_sand_1_s,
	[Xsand_sand_2_s] = Lsand_sand_2_s,
	[Xsand_sand_3_s] = Lsand_sand_3_s,
	[Xsand_sand_4_s] = Lsand_sand_4_s,
	[Xsand_sand_1_sB] = Lsand_sand_1_sB,
	[Xsand_sand_2_sB] = Lsand_sand_2_sB,
	[Xsand_sand_3_sB] = Lsand_sand_3_sB,
	[Xsand_sand_4_sB] = Lsand_sand_4_sB,
	[Xsand_stone_1_s] = Lsand_stone_1_s,
	[Xsand_stone_2_s] = Lsand_stone_2_s,

	[Xslide_ns] = Lslide_ns,
	[Xslide_ew] = Lslide_ew,

	[Xexit] = Lexit,
	[Xexit_1] = Lexit_1,
	[Xexit_2] = Lexit_2,
	[Xexit_3] = Lexit_3,

	[Xpause] = Lpause,
};

static void Lbeast(LOGIC L)
{
	int x;
	int y;
	int n;
	int shuffle[CAVE_WIDTH];

	for(n = 0; n < L->width; n++) shuffle[n] = L->cave_left + n;
	for(n = L->width; n > 1; ) {
		int swap = RANDOM(n);
		int temp = shuffle[--n];
		shuffle[n] = shuffle[swap];
		shuffle[swap] = temp;
	}

	for(y = L->cave_top; y <= L->cave_bottom; y++) {
		for(n = 0; n < L->width; n++) {
			x = shuffle[n];
			{
				FUNC f = Lfunc[L->cave[x][y]];
				if(f)(*f)(L,x,y);
			}
		}
	}
}

static void Lameuba(LOGIC L)
{
	int x;
	int y;
	int n;
	for(n = 0; n < L->ameuba_time; n++) {
		x = L->cave_left + RANDOM(L->width);
		y = L->cave_top + RANDOM(L->height);
		switch(L->next[x][y]) {
		case Xspace:
		case Xsplash_e: case Xsplash_w:
		case Xplant:
		case Xgrass:
		case Xdirt:
		case Xsand:
			switch(L->next[x][y-1]) {
			case Xameuba_1: case Xameuba_2:
			case Xameuba_3: case Xameuba_4:
			case Xameuba_5: case Xameuba_6:
			case Xameuba_7: case Xameuba_8:
			case Xfake_ameuba: case Yfake_ameuba:
				L->next[x][y] = Xdrip;
			}
			switch(L->next[x+1][y]) {
			case Xameuba_1: case Xameuba_2:
			case Xameuba_3: case Xameuba_4:
			case Xameuba_5: case Xameuba_6:
			case Xameuba_7: case Xameuba_8:
			case Xfake_ameuba: case Yfake_ameuba:
				L->next[x][y] = Xdrip;
			}
			switch(L->next[x][y+1]) {
			case Xameuba_1: case Xameuba_2:
			case Xameuba_3: case Xameuba_4:
			case Xameuba_5: case Xameuba_6:
			case Xameuba_7: case Xameuba_8:
			case Xfake_ameuba: case Yfake_ameuba:
				L->next[x][y] = Xdrip;
			}
			switch(L->next[x-1][y]) {
			case Xameuba_1: case Xameuba_2:
			case Xameuba_3: case Xameuba_4:
			case Xameuba_5: case Xameuba_6:
			case Xameuba_7: case Xameuba_8:
			case Xfake_ameuba: case Yfake_ameuba:
				L->next[x][y] = Xdrip;
			}
		}
	}
}

static void Lboom_one(LOGIC L, int x, int y, char dynamite)
{
	switch(L->next[x][y]) {
	case Zborder:
	case Zboom:
	case Zbug:
	case Ztank:
	case Zeater:
	case Zdynamite:
	case Xchain:
	case Xboom_android:
	case Xboom_1:
	case Xacid_1: case Xacid_2:
	case Xacid_3: case Xacid_4:
	case Xacid_5: case Xacid_6:
	case Xacid_7: case Xacid_8:
	case Xplant:
	case Yplant:
	case Xdoor_1: case Xdoor_2:
	case Xdoor_3: case Xdoor_4:
	case Xdoor_5: case Xdoor_6:
	case Xdoor_7: case Xdoor_8:
	case Xfake_door_1: case Xfake_door_2:
	case Xfake_door_3: case Xfake_door_4:
	case Xfake_door_5: case Xfake_door_6:
	case Xfake_door_7: case Xfake_door_8:
	case Xacid_s:
	case Xacid_nw: case Xacid_ne:
	case Xacid_sw: case Xacid_se:
	case Xsteel_1: case Xsteel_2:
	case Xsteel_3: case Xsteel_4:
		return;
	case Xandroid:
	case Xandroid_1_n: case Xandroid_2_n:
	case Xandroid_1_e: case Xandroid_2_e:
	case Xandroid_1_s: case Xandroid_2_s:
	case Xandroid_1_w: case Xandroid_2_w:
		if(dynamite) L->next[x][y] = Xboom_android;
		return;
	case Xbug_1_n: case Xbug_2_n:
	case Xbug_1_e: case Xbug_2_e:
	case Xbug_1_s: case Xbug_2_s:
	case Xbug_1_w: case Xbug_2_w:
		L->next[x][y] = Xchain;
		Lboom_bug(L, x, y);
		return;
	case Xbomb:
	case Xbomb_pause:
	case Xbomb_fall:
		L->next[x][y] = Xchain;
		Lboom_tank(L, x, y);
		return;
	}
	L->next[x][y] = Xboom_1;
}
static void Lboom_nine(LOGIC L, int x, int y, char dynamite)
{
	Lboom_one(L, x, y-1, dynamite);
	Lboom_one(L, x-1, y, dynamite);
	Lboom_one(L, x+1, y, dynamite);
	Lboom_one(L, x, y+1, dynamite);
	Lboom_one(L, x-1, y-1, dynamite);
	Lboom_one(L, x+1, y-1, dynamite);
	Lboom_one(L, x-1, y+1, dynamite);
	Lboom_one(L, x+1, y+1, dynamite);
	L->next[x][y] = Xboom_1;
}
static void Lexplode(LOGIC L)
{
	int x;
	int y;
	for(y = L->cave_top; y <= L->cave_bottom; y++) for(x = L->cave_left; x <= L->cave_right; x++) {
		switch(L->next[x][y]) {
		case Zboom:
			Lboom_nine(L, x, y, 0);
			break;
		case Zbug:
			Lboom_bug(L, x, y);
			Lboom_nine(L, x, y, 0);
			break;
		case Ztank:
			Lboom_tank(L, x, y);
			Lboom_nine(L, x, y, 0);
			break;
		case Zeater:
			L->boom[x-1][y-1] = L->eater_array[L->eater_pos][0];
			L->boom[x][y-1] = L->eater_array[L->eater_pos][1];
			L->boom[x+1][y-1] = L->eater_array[L->eater_pos][2];
			L->boom[x-1][y] = L->eater_array[L->eater_pos][3];
			L->boom[x][y] = L->eater_array[L->eater_pos][4];
			L->boom[x+1][y] = L->eater_array[L->eater_pos][5];
			L->boom[x-1][y+1] = L->eater_array[L->eater_pos][6];
			L->boom[x][y+1] = L->eater_array[L->eater_pos][7];
			L->boom[x+1][y+1] = L->eater_array[L->eater_pos][8];
			L->eater_pos = (L->eater_pos + 1) % 8;
			Lboom_nine(L, x, y, 0);
			break;
		case Zdynamite:
			Lboom_tank(L, x, y);
			Lboom_nine(L, x, y, 1);
			break;
		}
	}
}

static void Lcounter(LOGIC L)
{
	if(L->android_move_cnt > 0) {
		L->android_move_cnt--;
	} else {
		L->android_move_cnt = L->android_move_time;
	}
	if(L->android_clone_cnt > 0) {
		L->android_clone_cnt--;
	} else {
		L->android_clone_cnt = L->android_clone_time;
	}
	if(L->ball_state) {
		if(L->ball_cnt > 0) {
			L->ball_cnt--;
		} else {
			L->ball_cnt = L->ball_time;
		}
	}
	if(L->lenses_cnt > 0) {
		L->lenses_cnt--;
	}
	if(L->magnify_cnt > 0) {
		L->magnify_cnt--;
	}
	if(L->wheel_cnt > 0) {
		L->wheel_cnt--;
		L->sound[SOUND_wheel] = 1;
	}
	if(L->wind_cnt > 0) {
		L->wind_cnt--;
	}
	if(L->wonderwall_state) {
		if(L->wonderwall_time > 0) {
			L->wonderwall_time--;
			L->sound[SOUND_wonder] = 1;
		}
	}
	if(L->win[0] == 0 || L->win[1] == 0) {
		if(L->time > 0) {
			L->time--;
			if(L->time <= 100 && L->time % 10 == 0) L->sound[SOUND_time] = 1;
		}
	}
	if(L->score > 9999) L->score = 9999;
}

static void Lposition(LOGIC L)
{
	int x;
	int y;
	int P;

	for(P = 0; P < 2; P++) {
		x = L->pl_left[P] * 16 + (L->pl_right[P] - L->pl_left[P]) * 16 * (L->frame + 1) / 8;
		y = L->pl_top[P] * 16 + (L->pl_bottom[P] - L->pl_top[P]) * 16 * (L->frame + 1) / 8;

		L->pl_pixel_left[P] = x;
		L->pl_pixel_top[P] = y;
		L->pl_pixel_right[P] = x + 15;
		L->pl_pixel_bottom[P] = y + 15;
	}

	if(L->win[0] == 0 && L->win[1] == 0) {
		x = (L->pl_pixel_left[0] + L->pl_pixel_left[1]) / 2;
		y = (L->pl_pixel_top[0] + L->pl_pixel_top[1]) / 2;
	} else if(L->win[0] == 0) {
		x = L->pl_pixel_left[0];
		y = L->pl_pixel_top[0];
	} else if(L->win[1] == 0) {
		x = L->pl_pixel_left[1];
		y = L->pl_pixel_top[1];
	} else {
		x = L->pl_pixel_left[0];
		y = L->pl_pixel_top[0];
	}

	x += (VIEW_WIDTH + 15) / 2;
	y += (VIEW_HEIGHT + 15) / 2;
	if(x > L->cave_right * 16 + 15) x = L->cave_right * 16 + 15;
	if(y > L->cave_bottom * 16 + 15) y = L->cave_bottom * 16 + 15;
	if(x < L->cave_left * 16 + VIEW_WIDTH - 1) x = L->cave_left * 16 + VIEW_WIDTH - 1;
	if(y < L->cave_top * 16 + VIEW_HEIGHT - 1) y = L->cave_top * 16 + VIEW_HEIGHT - 1;

	L->pixel_right = x;
	L->pixel_bottom = y;
	L->view_right = x / 16;
	L->view_bottom = y / 16;

	x -= VIEW_WIDTH - 1;
	y -= VIEW_HEIGHT - 1;

	L->pixel_left = x;
	L->pixel_top = y;
	L->view_left = x / 16;
	L->view_top = y / 16;
}

#include <string.h>

void logic(struct LOGIC *L)
{
	if(L->joystick[0][INPUT_abort] && L->win[0] == 0) L->win[0] = -1;
	if(L->joystick[1][INPUT_abort] && L->win[1] == 0) L->win[1] = -1;

	memset(L->sound, 0, sizeof(L->sound));
	if(L->frame == 0) {
		memcpy(L->cavebuf, L->nextbuf, sizeof(L->cavebuf));
		Lplayer(L);
		memset(L->joystick, 0, sizeof(L->joystick));
	}
	Lposition(L);
	if(L->frame == 0) {
		Lbeast(L);
		Lcounter(L);
	}
	if(L->frame == 1) {
		Lameuba(L);
		Lexplode(L);
	}
}
