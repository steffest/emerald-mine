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

#ifndef cave_H
#define cave_H

/* common cave structure */

enum {
	Cspace,
	Cgrass,
	Cdirt,
	Cplant,
	Cwall_1, Cwall_2,
	Cwall_3, Cwall_4,
	Croundwall_1, Croundwall_2,
	Croundwall_3, Croundwall_4,
	Csteel_1, Csteel_2,
	Csteel_3, Csteel_4,
	Candroid,
	Ceater_n, Ceater_e,
	Ceater_s, Ceater_w,
	Calien,
	Cbug_1_n, Cbug_2_n,
	Cbug_1_e, Cbug_2_e,
	Cbug_1_s, Cbug_2_s,
	Cbug_1_w, Cbug_2_w,
	Ctank_1_n, Ctank_2_n,
	Ctank_1_e, Ctank_2_e,
	Ctank_1_s, Ctank_2_s,
	Ctank_1_w, Ctank_2_w,
	Cemerald,
	Cdiamond,
	Cstone,
	Cbomb,
	Cnut,
	Cspring,
	Cpush_emerald_e, Cpush_emerald_w,
	Cpush_diamond_e, Cpush_diamond_w,
	Cpush_stone_e, Cpush_stone_w,
	Cpush_bomb_e, Cpush_bomb_w,
	Cpush_nut_e, Cpush_nut_w,
	Cpush_spring_e, Cpush_spring_w,
	Cdynamite,
	Cdynamite_1, Cdynamite_2,
	Cdynamite_3, Cdynamite_4,
	Ckey_1, Ckey_2,
	Ckey_3, Ckey_4,
	Ckey_5, Ckey_6,
	Ckey_7, Ckey_8,
	Cdoor_1, Cdoor_2,
	Cdoor_3, Cdoor_4,
	Cdoor_5, Cdoor_6,
	Cdoor_7, Cdoor_8,
	Cfake_door_1, Cfake_door_2,
	Cfake_door_3, Cfake_door_4,
	Cfake_door_5, Cfake_door_6,
	Cfake_door_7, Cfake_door_8,
	Cballoon,
	Cball_1, Cball_2,
	Cameuba_1, Cameuba_2,
	Cameuba_3, Cameuba_4,
	Cameuba_5, Cameuba_6,
	Cameuba_7, Cameuba_8,
	Cdrip,
	Cwonderwall,
	Cwheel,
	Cswitch,
	Cbumper,
	Cacid_s,
	Cacid_nw, Cacid_ne,
	Cacid_sw, Cacid_se,
	Cacid_1, Cacid_2,
	Cacid_3, Cacid_4,
	Cacid_5, Cacid_6,
	Cacid_7, Cacid_8,
	Cfake_space,
	Cfake_grass,
	Cfake_ameuba,
	Clenses,
	Cmagnify,
	Csand,
	Csand_stone,
	Cslide_ns, Cslide_ew,
	Cwind_n, Cwind_e,
	Cwind_s, Cwind_w,
	Cwind_multi, Cwind_stop,
	Cexit,
	Cexit_1, Cexit_2, Cexit_3,
	Cpause,
	Cdecor_1, Cdecor_2,
	Cdecor_3, Cdecor_4,
	Cdecor_5, Cdecor_6,
	Cdecor_7, Cdecor_8,
	Cdecor_9, Cdecor_10,
	Cdecor_11, Cdecor_12,
	Calpha_excla, Calpha_apost,
	Calpha_comma, Calpha_minus,
	Calpha_perio, Calpha_colon,
	Calpha_quest, Calpha_arrow_e,
	Calpha_arrow_w, Calpha_copyr,
	Calpha_a, Calpha_b,
	Calpha_c, Calpha_d,
	Calpha_e, Calpha_f,
	Calpha_g, Calpha_h,
	Calpha_i, Calpha_j,
	Calpha_k, Calpha_l,
	Calpha_m, Calpha_n,
	Calpha_o, Calpha_p,
	Calpha_q, Calpha_r,
	Calpha_s, Calpha_t,
	Calpha_u, Calpha_v,
	Calpha_w, Calpha_x,
	Calpha_y, Calpha_z,
	Calpha_0, Calpha_1,
	Calpha_2, Calpha_3,
	Calpha_4, Calpha_5,
	Calpha_6, Calpha_7,
	Calpha_8, Calpha_9,
	CMAX
};

struct CAVE {
	int time;
	int diamonds;
	int seed;

	int width; /* cave width */
	int height; /* cave height */
	int player_x[2]; /* player x pos */
	int player_y[2]; /* player y pos */

	int eater_score; /* eater popped by stone/spring score */
	int alien_score; /* alien popped by stone/spring score */
	int bug_score; /* bug popped by stone/spring score */
	int tank_score; /* tank popped by stone/spring score */
	int emerald_score; /* emerald collect score */
	int diamond_score; /* diamond collect score */
	int nut_score; /* nut crack score */
	int spring_score; /* spring slurps alien score */
	int dynamite_score; /* dynamite collect score */
	int key_score; /* key collect score */
	int lenses_score; /* lenses collect score */
	int magnify_score; /* magnify collect score */
	int exit_score; /* exit score */

	int android_move_time; /* android move reset time */
	int android_clone_time; /* android clone reset time */
	int ball_time; /* ball reset time */
	int ameuba_time; /* ameuba speed */
	int wonderwall_time; /* wonderwall time */
	int wheel_time; /* wheel reset time */
	int lenses_time; /* lenses reset time */
	int magnify_time; /* magnify reset time */
	int wind_time; /* wind reset time */

	int wheel_x; /* wheel x pos */
	int wheel_y; /* wheel y pos */
	int wind_direction; /* wind direction */

	int teamwork:1; /* two player mode */
	int infinite:1; /* cave is infinitely wide */

	int ball_random:1; /* ball random flag */
	int ball_active:1; /* ball spinning flag */
	int wonderwall_active:1; /* wonderwall spinning flag */
	int wheel_active:1; /* wheel spinning flag */
	int lenses_active:1; /* lenses visible flag */
	int magnify_active:1; /* magnify visible flag */
	int wind_active:1; /* wind blowing flag */

	int android_eater:1; /* android clone data */
	int android_alien:1;
	int android_bug:1;
	int android_tank:1;
	int android_emerald:1;
	int android_diamond:1;
	int android_stone:1;
	int android_bomb:1;
	int android_nut:1;
	int android_spring:1;
	int android_dynamite:1;
	int android_balloon:1;
	int android_ameuba:1;

	short eater_array[8][9]; /* eater data */
	short ball_array[8]; /* ball data */
	short cave[100][100]; /* cave data */
};

#endif
