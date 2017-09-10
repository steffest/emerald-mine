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

#ifndef emerald_H
#define emerald_H

/* visible screen */

#define VIEW_WIDTH 320
#define VIEW_HEIGHT 240

/* maximum cave size */

#define CAVE_WIDTH 100
#define CAVE_HEIGHT 100

/* logic states */

enum {
	Zborder, /* !!! */
	Zboom, /* no picture */ /* internal chain explosion */
	Zbug, /* no picture */ /* internal bug explosion */
	Ztank, /* no picture */ /* internal tank/alien/bomb explosion */
	Zeater, /* no picture */ /* internal eater explosion */
	Zdynamite, /* no picture */ /* internal dynamite explosion */

	Xchain, /* chain explosion; transition to Zboom */
	Xboom_android, /* android explosion; transition to Xboom_2 */
	Xboom_1, /* normal explosion; transition to Xboom_2 */
	Xboom_2, /* transition to boom[] */
	Xpop,

	Xspace,
	Xsplash_e, Xsplash_w,
	Xplant,
	Yplant,
	Xacid_1, Xacid_2,
	Xacid_3, Xacid_4,
	Xacid_5, Xacid_6,
	Xacid_7, Xacid_8,

	Xgrass,
	Ygrass_nB, Ygrass_eB,
	Ygrass_sB, Ygrass_wB,
	Xdirt,
	Ydirt_nB, Ydirt_eB,
	Ydirt_sB, Ydirt_wB,
	Xandroid,
	Xandroid_1_n, Xandroid_2_n,
	Xandroid_1_e, Xandroid_2_e,
	Xandroid_1_s, Xandroid_2_s,
	Xandroid_1_w, Xandroid_2_w,
	Yandroid_n, Yandroid_nB,
	Yandroid_ne, Yandroid_neB,
	Yandroid_e, Yandroid_eB,
	Yandroid_se, Yandroid_seB,
	Yandroid_s, Yandroid_sB,
	Yandroid_sw, Yandroid_swB,
	Yandroid_w, Yandroid_wB,
	Yandroid_nw, Yandroid_nwB,
	Xeater_n, Xeater_e,
	Xeater_s, Xeater_w,
	Yeater_n, Yeater_nB,
	Yeater_e, Yeater_eB,
	Yeater_s, Yeater_sB,
	Yeater_w, Yeater_wB,
	Yeater_stone, Yeater_spring,
	Xalien,
	Xalien_pause,
	Yalien_n, Yalien_nB,
	Yalien_e, Yalien_eB,
	Yalien_s, Yalien_sB,
	Yalien_w, Yalien_wB,
	Yalien_stone, Yalien_spring,
	Xbug_1_n, Xbug_2_n,
	Xbug_1_e, Xbug_2_e,
	Xbug_1_s, Xbug_2_s,
	Xbug_1_w, Xbug_2_w,
	Ybug_n, Ybug_nB,
	Ybug_e, Ybug_eB,
	Ybug_s, Ybug_sB,
	Ybug_w, Ybug_wB,
	Ybug_w_n, Ybug_n_e,
	Ybug_e_s, Ybug_s_w,
	Ybug_e_n, Ybug_s_e,
	Ybug_w_s, Ybug_n_w,
	Ybug_stone_n, Ybug_stone_e,
	Ybug_stone_s, Ybug_stone_w,
	Ybug_spring_n, Ybug_spring_e,
	Ybug_spring_s, Ybug_spring_w,
	Xtank_1_n, Xtank_2_n,
	Xtank_1_e, Xtank_2_e,
	Xtank_1_s, Xtank_2_s,
	Xtank_1_w, Xtank_2_w,
	Ytank_n, Ytank_nB,
	Ytank_e, Ytank_eB,
	Ytank_s, Ytank_sB,
	Ytank_w, Ytank_wB,
	Ytank_w_n, Ytank_n_e,
	Ytank_e_s, Ytank_s_w,
	Ytank_e_n, Ytank_s_e,
	Ytank_w_s, Ytank_n_w,
	Ytank_stone_n, Ytank_stone_e,
	Ytank_stone_s, Ytank_stone_w,
	Ytank_spring_n, Ytank_spring_e,
	Ytank_spring_s, Ytank_spring_w,
	Xemerald,
	Xemerald_pause,
	Xemerald_fall,
	Yemerald_s, Yemerald_sB,
	Yemerald_e, Yemerald_eB,
	Yemerald_w, Yemerald_wB,
	Yemerald_shine,
	Yemerald_space,
	Xdiamond,
	Xdiamond_pause,
	Xdiamond_fall,
	Ydiamond_s, Ydiamond_sB,
	Ydiamond_e, Ydiamond_eB,
	Ydiamond_w, Ydiamond_wB,
	Ydiamond_shine,
	Ydiamond_space,
	Ydiamond_stone,
	Xstone,
	Xstone_pause,
	Xstone_fall,
	Ystone_s, Ystone_sB,
	Ystone_e, Ystone_eB,
	Ystone_w, Ystone_wB,
	Xbomb,
	Xbomb_pause,
	Xbomb_fall,
	Ybomb_s, Ybomb_sB,
	Ybomb_e, Ybomb_eB,
	Ybomb_w, Ybomb_wB,
	Ybomb_space,
	Xnut,
	Xnut_pause,
	Xnut_fall,
	Ynut_s, Ynut_sB,
	Ynut_e, Ynut_eB,
	Ynut_w, Ynut_wB,
	Ynut_stone,
	Xspring,
	Xspring_pause,
	Xspring_fall,
	Xspring_e, Xspring_w,
	Yspring_s, Yspring_sB,
	Yspring_e, Yspring_eB,
	Yspring_w, Yspring_wB,
	Yspring_alien_e, Yspring_alien_eB,
	Yspring_alien_w, Yspring_alien_wB,
	Xpush_emerald_e, Xpush_emerald_w,
	Xpush_diamond_e, Xpush_diamond_w,
	Xpush_stone_e, Xpush_stone_w,
	Xpush_bomb_e, Xpush_bomb_w,
	Xpush_nut_e, Xpush_nut_w,
	Xpush_spring_e, Xpush_spring_w,
	Xdynamite,
	Ydynamite_space,
	Xdynamite_1, Xdynamite_2,
	Xdynamite_3, Xdynamite_4,
	Xkey_1, Xkey_2,
	Xkey_3, Xkey_4,
	Xkey_5, Xkey_6,
	Xkey_7, Xkey_8,
	Xdoor_1, Xdoor_2,
	Xdoor_3, Xdoor_4,
	Xdoor_5, Xdoor_6,
	Xdoor_7, Xdoor_8,
	Xfake_door_1, Xfake_door_2,
	Xfake_door_3, Xfake_door_4,
	Xfake_door_5, Xfake_door_6,
	Xfake_door_7, Xfake_door_8,
	Xballoon,
	Yballoon_n, Yballoon_nB,
	Yballoon_e, Yballoon_eB,
	Yballoon_s, Yballoon_sB,
	Yballoon_w, Yballoon_wB,
	Xball_1, Xball_2,
	Yball_1, Yball_2,
	Xameuba_1, Xameuba_2,
	Xameuba_3, Xameuba_4,
	Xameuba_5, Xameuba_6,
	Xameuba_7, Xameuba_8,
	Xdrip,
	Xdrip_1_sB, Xdrip_2_sB,
	Xdrip_1_s, Xdrip_2_s,
	Xwonderwall, Ywonderwall,
	Xwheel, Ywheel,
	Xswitch, Yswitch,
	Xbumper, Ybumper,
	Xacid_s,
	Xacid_nw, Xacid_ne,
	Xacid_sw, Xacid_se,
	Xfake_space, Yfake_space,
	Xfake_grass, Yfake_grass,
	Xfake_ameuba, Yfake_ameuba,
	Xlenses,
	Xmagnify,
	Xsand,
	Xsand_stone,
	Xsand_stone_1_sB, Xsand_stone_2_sB,
	Xsand_stone_3_sB, Xsand_stone_4_sB,
	Xsand_sand_1_s, Xsand_sand_2_s,
	Xsand_sand_3_s, Xsand_sand_4_s,
	Xsand_sand_1_sB, Xsand_sand_2_sB,
	Xsand_sand_3_sB, Xsand_sand_4_sB,
	Xsand_stone_1_s, Xsand_stone_2_s,
	Xslide_ns, Xslide_ew,
	Yslide_ns, Yslide_ew,
	Xwind_n, Xwind_e,
	Xwind_s, Xwind_w,
	Xwind_multi, Xwind_stop,
	Xexit,
	Xexit_1, Xexit_2, Xexit_3,
	Xpause,

	Xwall_1, Xwall_2,
	Xwall_3, Xwall_4,
	Xroundwall_1, Xroundwall_2,
	Xroundwall_3, Xroundwall_4,
	Xsteel_1, Xsteel_2,
	Xsteel_3, Xsteel_4,
	Xdecor_1, Xdecor_2,
	Xdecor_3, Xdecor_4,
	Xdecor_5, Xdecor_6,
	Xdecor_7, Xdecor_8,
	Xdecor_9, Xdecor_10,
	Xdecor_11, Xdecor_12,
	Xalpha_excla, Xalpha_apost,
	Xalpha_comma, Xalpha_minus,
	Xalpha_perio, Xalpha_colon,
	Xalpha_quest, Xalpha_arrow_e,
	Xalpha_arrow_w, Xalpha_copyr,
	Xalpha_a, Xalpha_b,
	Xalpha_c, Xalpha_d,
	Xalpha_e, Xalpha_f,
	Xalpha_g, Xalpha_h,
	Xalpha_i, Xalpha_j,
	Xalpha_k, Xalpha_l,
	Xalpha_m, Xalpha_n,
	Xalpha_o, Xalpha_p,
	Xalpha_q, Xalpha_r,
	Xalpha_s, Xalpha_t,
	Xalpha_u, Xalpha_v,
	Xalpha_w, Xalpha_x,
	Xalpha_y, Xalpha_z,
	Xalpha_0, Xalpha_1,
	Xalpha_2, Xalpha_3,
	Xalpha_4, Xalpha_5,
	Xalpha_6, Xalpha_7,
	Xalpha_8, Xalpha_9,

	Xplayer,
	XMAX
};

enum {
	SOUND_exit, /* player goes in exit */
	SOUND_die, /* player dies */
	SOUND_time, /* time runs out */
	SOUND_boom, /* explosion */
	SOUND_tick, /* dynamite ticks */
	SOUND_collect, /* player collects diamond/emerald/dynamite/key/lenses/magnify */
	SOUND_roll, /* player pushes stone/bomb/nut */
	SOUND_push, /* player pushes spring/balloon/android */
	SOUND_dynamite, /* player places dynamite */
	SOUND_press, /* player presses wheel/wind/switch */
	SOUND_door, /* player goes thru door */
	SOUND_dirt, /* player digs dirt */
	SOUND_space, /* player walks on space */
	SOUND_android, /* android places something */
	SOUND_ball, /* ball places something */
	SOUND_slide, /* slide wall grows */
	SOUND_squash, /* stone squashes diamond, stone/emerald/diamond thru wonderwall */
	SOUND_crack, /* stone hits nut */
	SOUND_slurp, /* spring kills alien */
	SOUND_drip, /* drip hits ground */
	SOUND_wonder, /* wonderwall moves */
	SOUND_wheel, /* wheel moves */
	SOUND_stone, /* stone hits ground */
	SOUND_spring, /* spring hits ground/wall/bumper, stone hits spring */
	SOUND_diamond, /* diamond/emerald hits ground */
	SOUND_nut, /* nut hits ground */
	SOUND_bug, /* bug moves */
	SOUND_tank, /* tank/android moves */
	SOUND_eater, /* eater sits/eats diamond */
	SOUND_alien, /* alien moves */
	SOUND_acid, /* acid splashes */
	SOUND_MAX
};

enum {
	SPRITE_still,
	SPRITE_walk_n, SPRITE_walk_e,
	SPRITE_walk_s, SPRITE_walk_w,
	SPRITE_push_n, SPRITE_push_e,
	SPRITE_push_s, SPRITE_push_w,
	SPRITE_shoot_n, SPRITE_shoot_e,
	SPRITE_shoot_s, SPRITE_shoot_w,
	SPRITE_MAX
};

enum {
	INPUT_north,
	INPUT_east,
	INPUT_south,
	INPUT_west,
	INPUT_shoot,
	INPUT_abort,
	INPUT_MAX
};

/* structure used for gameplay */

struct LOGIC {
	long random;

	int time;
	int diamonds;
	int score;

	int win[2]; /* player -1=dead 0=alive +1=home */
	int hide[2]; /* player sprite 0=show 1=hide */

	int frame;
	int width;
	int height;

	char sound[SOUND_MAX];
	char joystick[2][INPUT_MAX];

	int cave_left, cave_top;
	int cave_right, cave_bottom;

	int view_left, view_top;
	int view_right, view_bottom;

	int pixel_left, pixel_top;
	int pixel_right, pixel_bottom;

	int pl_pixel_left[2], pl_pixel_top[2];
	int pl_pixel_right[2], pl_pixel_bottom[2];

	int pl_left[2], pl_top[2];
	int pl_right[2], pl_bottom[2];

	int pl_anim[2]; /* player sprite animation */
	int pl_moved[2]; /* player did move last time */
	int pl_spin[2]; /* player last direction moved */
	int pl_hold[2]; /* player hold button counter */

	int alien_score; /* alien popped by stone/spring score */
	int ameuba_time; /* ameuba speed */
	int android_move_cnt; /* android move time counter */
	int android_move_time; /* android move reset time */
	int android_clone_cnt; /* android clone time counter */
	int android_clone_time; /* android clone reset time */
	int ball_cnt; /* ball time counter */
	int ball_random; /* ball is random flag */
	int ball_state; /* ball currently on flag */
	int ball_time; /* ball reset time */
	int bug_score; /* bug popped by stone/spring score */
	int diamond_score; /* diamond collect score */
	int dynamite[2]; /* dynamite collected by players */
	int dynamite_score; /* dynamite collect score */
	int eater_pos; /* eater array pos */
	int eater_score; /* eater popped by stone/spring score */
	int emerald_score; /* emerald collect score */
	int exit_score; /* exit score */
	int keys[2]; /* keys collected by players */
	int key_score; /* key collect score */
	int lenses_cnt; /* lenses time counter */
	int lenses_score; /* lenses collect score */
	int lenses_time; /* lenses reset time */
	int magnify_cnt; /* magnify time counter */
	int magnify_score; /* magnify collect score */
	int magnify_time; /* magnify reset time */
	int nut_score; /* nut crack score */
	int shine_cnt; /* shine counter for emerald/diamond */
	int spring_score; /* spring slurps alien score */
	int tank_score; /* tank popped by stone/spring */
	int wheel_cnt; /* wheel time counter */
	int wheel_x; /* wheel x pos */
	int wheel_y; /* wheel y pos */
	int wheel_time; /* wheel reset time */
	int wind_cnt; /* wind time counter */
	int wind_direction; /* wind direction */
	int wind_time; /* wind reset time */
	int wonderwall_state; /* wonderwall currently on flag */
	int wonderwall_time; /* wonderwall time */
	short eater_array[8][9]; /* eater data */
	short ball_array[8]; /* ball data */
	short android_array[XMAX]; /* android clone table */

	short *cave[CAVE_WIDTH + 8];
	short *next[CAVE_WIDTH + 8];
	short *boom[CAVE_WIDTH + 8];

	short cavebuf[CAVE_WIDTH + 8][CAVE_HEIGHT + 4];
	short nextbuf[CAVE_WIDTH + 8][CAVE_HEIGHT + 4];
	short boombuf[CAVE_WIDTH + 8][CAVE_HEIGHT + 4];
};

#endif
