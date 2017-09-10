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

/* convert cave structure to internal logic structure */

#include "emerald.h"
#include "cave.h"

static const short map[CMAX] = {
	[Cspace] = Xspace,
	[Cgrass] = Xgrass,
	[Cdirt] = Xdirt,
	[Cplant] = Xplant,
	[Cwall_1] = Xwall_1, [Cwall_2] = Xwall_2,
	[Cwall_3] = Xwall_3, [Cwall_4] = Xwall_4,
	[Croundwall_1] = Xroundwall_1, [Croundwall_2] = Xroundwall_2,
	[Croundwall_3] = Xroundwall_3, [Croundwall_4] = Xroundwall_4,
	[Csteel_1] = Xsteel_1, [Csteel_2] = Xsteel_2,
	[Csteel_3] = Xsteel_3, [Csteel_4] = Xsteel_4,
	[Candroid] = Xandroid,
	[Ceater_n] = Xeater_n, [Ceater_e] = Xeater_e,
	[Ceater_s] = Xeater_s, [Ceater_w] = Xeater_w,
	[Calien] = Xalien,
	[Cbug_1_n] = Xbug_1_n, [Cbug_2_n] = Xbug_2_n,
	[Cbug_1_e] = Xbug_1_e, [Cbug_2_e] = Xbug_2_e,
	[Cbug_1_s] = Xbug_1_s, [Cbug_2_s] = Xbug_2_s,
	[Cbug_1_w] = Xbug_1_w, [Cbug_2_w] = Xbug_2_w,
	[Ctank_1_n] = Xtank_1_n, [Ctank_2_n] = Xtank_2_n,
	[Ctank_1_e] = Xtank_1_e, [Ctank_2_e] = Xtank_2_e,
	[Ctank_1_s] = Xtank_1_s, [Ctank_2_s] = Xtank_2_s,
	[Ctank_1_w] = Xtank_1_w, [Ctank_2_w] = Xtank_2_w,
	[Cemerald] = Xemerald,
	[Cdiamond] = Xdiamond,
	[Cstone] = Xstone,
	[Cbomb] = Xbomb,
	[Cnut] = Xnut,
	[Cspring] = Xspring,
	[Cpush_emerald_e] = Xpush_emerald_e, [Cpush_emerald_w] = Xpush_emerald_w,
	[Cpush_diamond_e] = Xpush_diamond_e, [Cpush_diamond_w] = Xpush_diamond_w,
	[Cpush_stone_e] = Xpush_stone_e, [Cpush_stone_w] = Xpush_stone_w,
	[Cpush_bomb_e] = Xpush_bomb_e, [Cpush_bomb_w] = Xpush_bomb_w,
	[Cpush_nut_e] = Xpush_nut_e, [Cpush_nut_w] = Xpush_nut_w,
	[Cpush_spring_e] = Xpush_spring_e, [Cpush_spring_w] = Xpush_spring_w,
	[Cdynamite] = Xdynamite,
	[Cdynamite_1] = Xdynamite_1, [Cdynamite_2] = Xdynamite_2,
	[Cdynamite_3] = Xdynamite_3, [Cdynamite_4] = Xdynamite_4,
	[Ckey_1] = Xkey_1, [Ckey_2] = Xkey_2,
	[Ckey_3] = Xkey_3, [Ckey_4] = Xkey_4,
	[Ckey_5] = Xkey_5, [Ckey_6] = Xkey_6,
	[Ckey_7] = Xkey_7, [Ckey_8] = Xkey_8,
	[Cdoor_1] = Xdoor_1, [Cdoor_2] = Xdoor_2,
	[Cdoor_3] = Xdoor_3, [Cdoor_4] = Xdoor_4,
	[Cdoor_5] = Xdoor_5, [Cdoor_6] = Xdoor_6,
	[Cdoor_7] = Xdoor_7, [Cdoor_8] = Xdoor_8,
	[Cfake_door_1] = Xfake_door_1, [Cfake_door_2] = Xfake_door_2,
	[Cfake_door_3] = Xfake_door_3, [Cfake_door_4] = Xfake_door_4,
	[Cfake_door_5] = Xfake_door_5, [Cfake_door_6] = Xfake_door_6,
	[Cfake_door_7] = Xfake_door_7, [Cfake_door_8] = Xfake_door_8,
	[Cballoon] = Xballoon,
	[Cball_1] = Xball_1, [Cball_2] = Xball_2,
	[Cameuba_1] = Xameuba_1, [Cameuba_2] = Xameuba_2,
	[Cameuba_3] = Xameuba_3, [Cameuba_4] = Xameuba_4,
	[Cameuba_5] = Xameuba_5, [Cameuba_6] = Xameuba_6,
	[Cameuba_7] = Xameuba_7, [Cameuba_8] = Xameuba_8,
	[Cdrip] = Xdrip,
	[Cwonderwall] = Xwonderwall,
	[Cwheel] = Xwheel,
	[Cswitch] = Xswitch,
	[Cbumper] = Xbumper,
	[Cacid_s] = Xacid_s,
	[Cacid_nw] = Xacid_nw, [Cacid_ne] = Xacid_ne,
	[Cacid_sw] = Xacid_sw, [Cacid_se] = Xacid_se,
	[Cacid_1] = Xacid_1, [Cacid_2] = Xacid_2,
	[Cacid_3] = Xacid_3, [Cacid_4] = Xacid_4,
	[Cacid_5] = Xacid_5, [Cacid_6] = Xacid_6,
	[Cacid_7] = Xacid_7, [Cacid_8] = Xacid_8,
	[Cfake_space] = Xfake_space,
	[Cfake_grass] = Xfake_grass,
	[Cfake_ameuba] = Xfake_ameuba,
	[Clenses] = Xlenses,
	[Cmagnify] = Xmagnify,
	[Csand] = Xsand,
	[Csand_stone] = Xsand_stone,
	[Cslide_ns] = Xslide_ns, [Cslide_ew] = Xslide_ew,
	[Cwind_n] = Xwind_n, [Cwind_e] = Xwind_e,
	[Cwind_s] = Xwind_s, [Cwind_w] = Xwind_w,
	[Cwind_multi] = Xwind_multi, [Cwind_stop] = Xwind_stop,
	[Cexit] = Xexit,
	[Cexit_1] = Xexit_1, [Cexit_2] = Xexit_2, [Cexit_3] = Xexit_3,
	[Cpause] = Xpause,
	[Cdecor_1] = Xdecor_1, [Cdecor_2] = Xdecor_2,
	[Cdecor_3] = Xdecor_3, [Cdecor_4] = Xdecor_4,
	[Cdecor_5] = Xdecor_5, [Cdecor_6] = Xdecor_6,
	[Cdecor_7] = Xdecor_7, [Cdecor_8] = Xdecor_8,
	[Cdecor_9] = Xdecor_9, [Cdecor_10] = Xdecor_10,
	[Cdecor_11] = Xdecor_11, [Cdecor_12] = Xdecor_12,
	[Calpha_excla] = Xalpha_excla, [Calpha_apost] = Xalpha_apost,
	[Calpha_comma] = Xalpha_comma, [Calpha_minus] = Xalpha_minus,
	[Calpha_perio] = Xalpha_perio, [Calpha_colon] = Xalpha_colon,
	[Calpha_quest] = Xalpha_quest, [Calpha_arrow_e] = Xalpha_arrow_e,
	[Calpha_arrow_w] = Xalpha_arrow_w, [Calpha_copyr] = Xalpha_copyr,
	[Calpha_a] = Xalpha_a, [Calpha_b] = Xalpha_b,
	[Calpha_c] = Xalpha_c, [Calpha_d] = Xalpha_d,
	[Calpha_e] = Xalpha_e, [Calpha_f] = Xalpha_f,
	[Calpha_g] = Xalpha_g, [Calpha_h] = Xalpha_h,
	[Calpha_i] = Xalpha_i, [Calpha_j] = Xalpha_j,
	[Calpha_k] = Xalpha_k, [Calpha_l] = Xalpha_l,
	[Calpha_m] = Xalpha_m, [Calpha_n] = Xalpha_n,
	[Calpha_o] = Xalpha_o, [Calpha_p] = Xalpha_p,
	[Calpha_q] = Xalpha_q, [Calpha_r] = Xalpha_r,
	[Calpha_s] = Xalpha_s, [Calpha_t] = Xalpha_t,
	[Calpha_u] = Xalpha_u, [Calpha_v] = Xalpha_v,
	[Calpha_w] = Xalpha_w, [Calpha_x] = Xalpha_x,
	[Calpha_y] = Xalpha_y, [Calpha_z] = Xalpha_z,
	[Calpha_0] = Xalpha_0, [Calpha_1] = Xalpha_1,
	[Calpha_2] = Xalpha_2, [Calpha_3] = Xalpha_3,
	[Calpha_4] = Xalpha_4, [Calpha_5] = Xalpha_5,
	[Calpha_6] = Xalpha_6, [Calpha_7] = Xalpha_7,
	[Calpha_8] = Xalpha_8, [Calpha_9] = Xalpha_9,
};

#define clamp(a,x,b) ((a) <= (x) ? (x) <= (b) ? (x) : (b) : (a))

#include <string.h>

/* initialize entire internal structure */

void cave_to_logic(struct CAVE *cav, struct LOGIC *lev)
{
	int x;
	int y;
	int ply;

	memset(lev, 0, sizeof(*lev));

	lev->random = cav->seed;

	lev->time = clamp(0, cav->time, 9999);
	lev->diamonds = clamp(0, cav->diamonds, 9999);

	lev->frame = 7;
	lev->width = clamp(1, cav->width, CAVE_WIDTH);
	lev->height = clamp(1, cav->height, CAVE_HEIGHT);

	lev->cave_left = 4;
	lev->cave_top = 2;
	lev->cave_right = lev->cave_left + lev->width - 1;
	lev->cave_bottom = lev->cave_top + lev->height - 1;

	for(ply = 0; ply < 2; ply++) {
		x = lev->cave_left + clamp(0, cav->player_x[ply], CAVE_WIDTH - 1);
		y = lev->cave_top + clamp(0, cav->player_y[ply], CAVE_HEIGHT - 1);
		lev->pl_left[ply] = x;
		lev->pl_top[ply] = y;
		lev->pl_right[ply] = x;
		lev->pl_bottom[ply] = y;
		lev->pl_anim[ply] = SPRITE_still;
	}
	if(!cav->teamwork) {
		lev->win[1] = 1;
		lev->hide[1] = 1;
	}

	lev->eater_score = clamp(0, cav->eater_score, 9999);
	lev->alien_score = clamp(0, cav->alien_score, 9999);
	lev->bug_score = clamp(0, cav->bug_score, 9999);
	lev->tank_score = clamp(0, cav->tank_score, 9999);
	lev->emerald_score = clamp(0, cav->emerald_score, 9999);
	lev->diamond_score = clamp(0, cav->diamond_score, 9999);
	lev->nut_score = clamp(0, cav->nut_score, 9999);
	lev->spring_score = clamp(0, cav->spring_score, 9999);
	lev->dynamite_score = clamp(0, cav->dynamite_score, 9999);
	lev->key_score = clamp(0, cav->key_score, 9999);
	lev->lenses_score = clamp(0, cav->lenses_score, 9999);
	lev->magnify_score = clamp(0, cav->magnify_score, 9999);
	lev->exit_score = clamp(0, cav->exit_score, 9999);

	lev->android_move_time = clamp(0, cav->android_move_time, 9999);
	lev->android_clone_time = clamp(0, cav->android_clone_time, 9999);
	lev->ball_time = clamp(0, cav->ball_time, 9999);
	lev->ameuba_time = clamp(0, cav->ameuba_time, 999);
	lev->wonderwall_time = clamp(0, cav->wonderwall_time, 9999);
	lev->wheel_time = clamp(0, cav->wheel_time, 9999);
	lev->lenses_time = clamp(0, cav->lenses_time, 9999);
	lev->magnify_time = clamp(0, cav->magnify_time, 9999);
	lev->wind_time = clamp(0, cav->wind_time, 9999);

	lev->wheel_x = lev->cave_left + clamp(0, cav->wheel_x, CAVE_WIDTH - 1);
	lev->wheel_y = lev->cave_top + clamp(0, cav->wheel_y, CAVE_HEIGHT - 1);
	lev->wind_direction = clamp(0, cav->wind_direction, 3);

	if(cav->ball_random) lev->ball_random = 1;
	if(cav->ball_active) lev->ball_state = 1;
	if(cav->wonderwall_active) lev->wonderwall_state = 1;
	if(cav->wheel_active) lev->wheel_cnt = lev->wheel_time;
	if(cav->lenses_active) lev->lenses_cnt = lev->lenses_time;
	if(cav->magnify_active) lev->magnify_cnt = lev->magnify_time;
	if(cav->wind_active) lev->wind_cnt = lev->wind_time;

	lev->android_move_cnt = lev->android_move_time;
	lev->android_clone_cnt = lev->android_clone_time;
	lev->ball_cnt = lev->ball_time;

	for(x = 0; x < 8; x++) for(y = 0; y < 9; y++) {
		lev->eater_array[x][y] = map[clamp(0, cav->eater_array[x][y], CMAX - 1)];
	}
	for(x = 0; x < 8; x++) {
		lev->ball_array[x] = map[clamp(0, cav->ball_array[x], CMAX - 1)];
	}
	for(x = 0; x < XMAX; x++) lev->android_array[x] = Xspace;
	if(cav->android_eater) {
		lev->android_array[Xeater_n] = Xeater_n;
		lev->android_array[Yeater_nB] = Xeater_n;
		lev->android_array[Xeater_e] = Xeater_e;
		lev->android_array[Yeater_eB] = Xeater_e;
		lev->android_array[Xeater_s] = Xeater_s;
		lev->android_array[Yeater_sB] = Xeater_s;
		lev->android_array[Xeater_w] = Xeater_w;
		lev->android_array[Yeater_wB] = Xeater_w;
	}
	if(cav->android_alien) {
		lev->android_array[Xalien] = Xalien;
		lev->android_array[Xalien_pause] = Xalien;
		lev->android_array[Yalien_nB] = Xalien;
		lev->android_array[Yalien_eB] = Xalien;
		lev->android_array[Yalien_sB] = Xalien;
		lev->android_array[Yalien_wB] = Xalien;
	}
	if(cav->android_bug) {
		lev->android_array[Xbug_1_n] = Xbug_1_n;
		lev->android_array[Xbug_2_n] = Xbug_1_n;
		lev->android_array[Ybug_nB] = Xbug_1_n;
		lev->android_array[Ybug_n_e] = Xbug_1_n;
		lev->android_array[Ybug_n_w] = Xbug_1_n;
		lev->android_array[Xbug_1_e] = Xbug_1_e;
		lev->android_array[Xbug_2_e] = Xbug_1_e;
		lev->android_array[Ybug_eB] = Xbug_1_e;
		lev->android_array[Ybug_e_s] = Xbug_1_e;
		lev->android_array[Ybug_e_n] = Xbug_1_e;
		lev->android_array[Xbug_1_s] = Xbug_1_s;
		lev->android_array[Xbug_2_s] = Xbug_1_s;
		lev->android_array[Ybug_sB] = Xbug_1_s;
		lev->android_array[Ybug_s_w] = Xbug_1_s;
		lev->android_array[Ybug_s_e] = Xbug_1_s;
		lev->android_array[Xbug_1_w] = Xbug_1_w;
		lev->android_array[Xbug_2_w] = Xbug_1_w;
		lev->android_array[Ybug_wB] = Xbug_1_w;
		lev->android_array[Ybug_w_n] = Xbug_1_w;
		lev->android_array[Ybug_w_s] = Xbug_1_w;
	}
	if(cav->android_tank) {
		lev->android_array[Xtank_1_n] = Xtank_1_n;
		lev->android_array[Xtank_2_n] = Xtank_1_n;
		lev->android_array[Ytank_nB] = Xtank_1_n;
		lev->android_array[Ytank_n_e] = Xtank_1_n;
		lev->android_array[Ytank_n_w] = Xtank_1_n;
		lev->android_array[Xtank_1_e] = Xtank_1_e;
		lev->android_array[Xtank_2_e] = Xtank_1_e;
		lev->android_array[Ytank_eB] = Xtank_1_e;
		lev->android_array[Ytank_e_s] = Xtank_1_e;
		lev->android_array[Ytank_e_n] = Xtank_1_e;
		lev->android_array[Xtank_1_s] = Xtank_1_s;
		lev->android_array[Xtank_2_s] = Xtank_1_s;
		lev->android_array[Ytank_sB] = Xtank_1_s;
		lev->android_array[Ytank_s_w] = Xtank_1_s;
		lev->android_array[Ytank_s_e] = Xtank_1_s;
		lev->android_array[Xtank_1_w] = Xtank_1_w;
		lev->android_array[Xtank_2_w] = Xtank_1_w;
		lev->android_array[Ytank_wB] = Xtank_1_w;
		lev->android_array[Ytank_w_n] = Xtank_1_w;
		lev->android_array[Ytank_w_s] = Xtank_1_w;
	}
	if(cav->android_emerald) {
		lev->android_array[Xemerald] = Xemerald;
		lev->android_array[Xemerald_pause] = Xemerald;
		lev->android_array[Xemerald_fall] = Xemerald;
		lev->android_array[Yemerald_sB] = Xemerald;
		lev->android_array[Yemerald_eB] = Xemerald;
		lev->android_array[Yemerald_wB] = Xemerald;
	}
	if(cav->android_diamond) {
		lev->android_array[Xdiamond] = Xdiamond;
		lev->android_array[Xdiamond_pause] = Xdiamond;
		lev->android_array[Xdiamond_fall] = Xdiamond;
		lev->android_array[Ydiamond_sB] = Xdiamond;
		lev->android_array[Ydiamond_eB] = Xdiamond;
		lev->android_array[Ydiamond_wB] = Xdiamond;
	}
	if(cav->android_stone) {
		lev->android_array[Xstone] = Xstone;
		lev->android_array[Xstone_pause] = Xstone;
		lev->android_array[Xstone_fall] = Xstone;
		lev->android_array[Ystone_sB] = Xstone;
		lev->android_array[Ystone_eB] = Xstone;
		lev->android_array[Ystone_wB] = Xstone;
	}
	if(cav->android_bomb) {
		lev->android_array[Xbomb] = Xbomb;
		lev->android_array[Xbomb_pause] = Xbomb;
		lev->android_array[Xbomb_fall] = Xbomb;
		lev->android_array[Ybomb_sB] = Xbomb;
		lev->android_array[Ybomb_eB] = Xbomb;
		lev->android_array[Ybomb_wB] = Xbomb;
	}
	if(cav->android_nut) {
		lev->android_array[Xnut] = Xnut;
		lev->android_array[Xnut_pause] = Xnut;
		lev->android_array[Xnut_fall] = Xnut;
		lev->android_array[Ynut_sB] = Xnut;
		lev->android_array[Ynut_eB] = Xnut;
		lev->android_array[Ynut_wB] = Xnut;
	}
	if(cav->android_spring) {
		lev->android_array[Xspring] = Xspring;
		lev->android_array[Xspring_pause] = Xspring;
		lev->android_array[Xspring_fall] = Xspring;
		lev->android_array[Xspring_e] = Xspring;
		lev->android_array[Xspring_w] = Xspring;
		lev->android_array[Yspring_sB] = Xspring;
		lev->android_array[Yspring_eB] = Xspring;
		lev->android_array[Yspring_wB] = Xspring;
		lev->android_array[Yspring_alien_eB] = Xspring;
		lev->android_array[Yspring_alien_wB] = Xspring;
	}
	if(cav->android_dynamite) {
		lev->android_array[Xdynamite] = Xdynamite;
	}
	if(cav->android_balloon) {
		lev->android_array[Xballoon] = Xballoon;
		lev->android_array[Yballoon_nB] = Xballoon;
		lev->android_array[Yballoon_eB] = Xballoon;
		lev->android_array[Yballoon_sB] = Xballoon;
		lev->android_array[Yballoon_wB] = Xballoon;
	}
	if(cav->android_ameuba) {
		lev->android_array[Xfake_ameuba] = Xdrip;
		lev->android_array[Yfake_ameuba] = Xdrip;
		lev->android_array[Xameuba_1] = Xdrip;
		lev->android_array[Xameuba_2] = Xdrip;
		lev->android_array[Xameuba_3] = Xdrip;
		lev->android_array[Xameuba_4] = Xdrip;
		lev->android_array[Xameuba_5] = Xdrip;
		lev->android_array[Xameuba_6] = Xdrip;
		lev->android_array[Xameuba_7] = Xdrip;
		lev->android_array[Xameuba_8] = Xdrip;
	}

	for(x = 0; x < CAVE_WIDTH + 8; x++) {
		lev->cave[x] = lev->cavebuf[x];
		lev->next[x] = lev->nextbuf[x];
		lev->boom[x] = lev->boombuf[x];
		for(y = 0; y < CAVE_HEIGHT + 4; y++) lev->cave[x][y] = Zborder;
		for(y = 0; y < CAVE_HEIGHT + 4; y++) lev->next[x][y] = Zborder;
		for(y = 0; y < CAVE_HEIGHT + 4; y++) lev->boom[x][y] = Xspace;
	}
	if(cav->infinite) {
		for(x = 0; x < 8; x++) {
			lev->cave[lev->width + x] = lev->cave[x];
			lev->next[lev->width + x] = lev->next[x];
			lev->boom[lev->width + x] = lev->boom[x];
		}
	}
	for(x = 0; x < lev->width; x++) for(y = 0; y < lev->height; y++) {
		lev->cave[lev->cave_left + x][lev->cave_top + y] = map[clamp(0, cav->cave[x][y], CMAX - 1)];
	}
	memcpy(lev->nextbuf, lev->cavebuf, sizeof(lev->nextbuf));
}
