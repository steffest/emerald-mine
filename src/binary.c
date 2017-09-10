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

/* convert binary file format to cave structure */

/* the cave structure supports most binary emerald mine caves.
 *
 * different:
 * - wrap around - vertical position does not change / player does not disappear
 * - acid top - kingsoft / does not turn into fake acid
 * - acid top - noi-ris / does not turn into plant
 * - active objects - use global flags / no individual state
 * - falling states - use normal state
 * - paused states - use normal state
 * - drip states - use normal drip
 * - sand states - use normal sand
 *
 * kingsoft bugs:
 * - explosion code [very unfortunate to be in editor]
 * - stone squash diamond
 * - explosions at scroll edges
 *
 * noi-ris bugs:
 * - bug and tank codes in kingsoft eaters
 *
 * acid top is added to acid bottom in both the cave and the eaters.
 *
 * acid is handled completely different to kingsoft emerald mine.
 * acid top is a separate object, which can be put anywhere and
 * the beasts will be gobbled, even if they go in from below.
 *
 * borderless caves are supported even with variable sized caves.
 * beasts and players will wrap around to the opposite side of the
 * cave if they walk over the edge.
 *
 * time is converted to 10x kingsoft format.
 * the reason the kingsoft player used 5x time was copy protection [very clever].
 */

#include "cave.h"

#define in_byte(x) (buf[x] & 255)
#define in_half(x) ((buf[x] & 255) << 8 | (buf[x + 1] & 255))

static const short map_four[256] = {
/*  0*/	Cstone, Cstone/*stone_fall*/,
/*  2*/	Cdiamond, Cdiamond/*diamond_fall*/,
/*  4*/	Calien, Calien/*alien_pause*/,
/*  6*/	Cpause/*boom_1*/, Cpause/*boom_2*/,
/*  8*/	Ctank_1_n, Ctank_1_e,
/* 10*/	Ctank_1_s, Ctank_1_w,
/* 12*/	Ctank_2_n, Ctank_2_e,
/* 14*/	Ctank_2_s, Ctank_2_w,
/* 16*/	Cbomb, Cbomb/*bomb_fall*/,
/* 18*/	Cemerald, Cemerald/*emerald_fall*/,
/* 20*/	Cbug_1_n, Cbug_1_e,
/* 22*/	Cbug_1_s, Cbug_1_w,
/* 24*/	Cbug_2_n, Cbug_2_e,
/* 26*/	Cbug_2_s, Cbug_2_w,
/* 28*/	Cdrip, Cdrip/*drip_fall_1*/,
/* 30*/	Cspace/*drip_fall_2*/, Cdrip/*drip_fall_3*/,
/* 32*/	Cstone/*stone_pause*/, Cbomb/*bomb_pause*/,
/* 34*/	Cdiamond/*diamond_pause*/, Cemerald/*emerald_pause*/,
/* 36*/	Cwonderwall/*wonderwall*/, Cnut,
/* 38*/	Cnut/*nut_fall*/, Cnut/*nut_pause*/,
/* 40*/	Cwheel/*wheel*/, Ceater_n,
/* 42*/	Ceater_s, Ceater_w,
/* 44*/	Ceater_e, Csand_stone,
/* 46*/	Cspace/*sand_stone_2_sB*/, Cspace/*sand_stone_3_sB*/,
/* 48*/	Cspace/*sand_stone_4_sB*/, Csand/*sand_sand_2_sB*/,
/* 50*/	Csand/*sand_sand_3_sB*/, Csand/*sand_sand_4_sB*/,
/* 52*/	Csand_stone/*sand_sand_2_s*/, Csand_stone/*sand_sand_3_s*/,
/* 54*/	Csand_stone/*sand_sand_4_s*/, Csand/*sand_sand_4_sB*/,
/* 56*/	Cstone/*sand_stone_2_s*/, Cfake_space/**/,
/* 58*/	Cfake_space/**/, Cdynamite_1,
/* 60*/	Cdynamite_2, Cdynamite_3,
/* 62*/	Cdynamite_4, Cacid_s,
/* 64*/	Cexit_1, Cexit_2,
/* 66*/	Cexit_3, Cfake_space/**/,
/* 68*/	Cfake_space/**/, Cfake_space/**/,
/* 70*/	Cfake_space/**/, Cfake_space/**/,
/* 72*/	Cfake_space/**/, Cfake_space/**/,
/* 74*/	Cfake_space/**/, Cfake_space/**/,
/* 76*/	Cfake_space/**/, Cfake_space/**/,
/* 78*/	Cfake_space, Cfake_space/**/,
/* 80*/	Cfake_space/**/, Cfake_space/**/,
/* 82*/	Cfake_space/**/, Cfake_space/**/,
/* 84*/	Cfake_space/**/, Cfake_space/**/,
/* 86*/	Cfake_space/**/, Cfake_space/**/,
/* 88*/	Cfake_space/**/, Cfake_space/**/,
/* 90*/	Cfake_space/**/, Cfake_space/**/,
/* 92*/	Cfake_space/**/, Cfake_space/**/,
/* 94*/	Cfake_space/**/, Cfake_space/**/,
/* 96*/	Cfake_space/**/, Cfake_space/**/,
/* 98*/	Cfake_space/**/, Cfake_space/**/,
/*100*/	Cfake_space/**/, Cfake_space/**/,
/*102*/	Cfake_space/**/, Cfake_space/**/,
/*104*/	Cfake_space/**/, Cfake_space/**/,
/*106*/	Cfake_space/**/, Cfake_space/**/,
/*108*/	Cfake_space/**/, Cfake_space/**/,
/*110*/	Cfake_space/**/, Cfake_space/**/,
/*112*/	Cfake_space/**/, Cpause/*dynamite_space*/,
/*114*/	Cpush_nut_w, Cpush_nut_e,
/*116*/	Csteel_1, Cdynamite_4/*boom_2*/,
/*118*/	Cpause/*emerald_space*/, Cpush_bomb_w,
/*120*/	Cpush_bomb_e, Cpush_stone_w,
/*122*/	Cpush_stone_e, Cpause/*diamond_space*/,
/*124*/	Cpause/*dirt_w*/, Cpause/*dirt_e*/,
/*126*/	Cpause/*dirt_n*/, Cpause/*dirt_s*/,
/*128*/	Cspace, Croundwall_1,
/*130*/	Cgrass, Csteel_1,
/*132*/	Cwall_1, Ckey_1,
/*134*/	Ckey_2, Ckey_3,
/*136*/	Ckey_4, Cdoor_1,
/*138*/	Cdoor_2, Cdoor_3,
/*140*/	Cdoor_4, Cfake_ameuba/**/,
/*142*/	Cfake_door_1, Cfake_door_2,
/*144*/	Cfake_door_3, Cfake_door_4,
/*146*/	Cwonderwall, Cwheel,
/*148*/	Csand, Cacid_nw,
/*150*/	Cacid_ne, Cacid_sw,
/*152*/	Cacid_se, Cplant/*acid_1*/,
/*154*/	Cameuba_5, Cameuba_6,
/*156*/	Cameuba_7, Cameuba_8,
/*158*/	Cexit, Cfake_grass/*dirt*/,
/*160*/	Cfake_grass/*dirt*/, Cfake_grass/*dirt*/,
/*162*/	Cfake_grass/*dirt*/, Cfake_grass/*dirt*/,
/*164*/	Cfake_grass/*dirt*/, Cfake_grass/*dirt*/,
/*166*/	Cfake_grass/*dirt*/, Cfake_grass/*dirt*/,
/*168*/	Cfake_grass/*dirt*/, Cfake_grass/*dirt*/,
/*170*/	Cfake_grass/*dirt*/, Cfake_grass/*dirt*/,
/*172*/	Cfake_grass/*dirt*/, Cfake_grass/*dirt*/,
/*174*/	Cfake_grass/*dirt*/, Cdynamite,
/*176*/	Cfake_space/**/, Cfake_space/**/,
/*178*/	Cfake_space/**/, Cfake_space/**/,
/*180*/	Cfake_space/**/, Cfake_space/**/,
/*182*/	Cfake_space/**/, Cfake_space/**/,
/*184*/	Cfake_space/**/, Cfake_space/**/,
/*186*/	Cfake_space/**/, Cfake_space/**/,
/*188*/	Cfake_space/**/, Cfake_space/**/,
/*190*/	Cplant/**/, Cfake_space/**/,
/*192*/	Cfake_space/**/, Cfake_space/**/,
/*194*/	Cfake_space/**/, Cfake_space/**/,
/*196*/	Cfake_space/**/, Cfake_space/**/,
/*198*/	Cfake_space/**/, Cfake_space/**/,
/*200*/	Calpha_a, Calpha_b,
/*202*/	Calpha_c, Calpha_d,
/*204*/	Calpha_e, Calpha_f,
/*206*/	Calpha_g, Calpha_h,
/*208*/	Calpha_i, Calpha_j,
/*210*/	Calpha_k, Calpha_l,
/*212*/	Calpha_m, Calpha_n,
/*214*/	Calpha_o, Calpha_p,
/*216*/	Calpha_q, Calpha_r,
/*218*/	Calpha_s, Calpha_t,
/*220*/	Calpha_u, Calpha_v,
/*222*/	Calpha_w, Calpha_x,
/*224*/	Calpha_y, Calpha_z,
/*226*/	Calpha_0, Calpha_1,
/*228*/	Calpha_2, Calpha_3,
/*230*/	Calpha_4, Calpha_5,
/*232*/	Calpha_6, Calpha_7,
/*234*/	Calpha_8, Calpha_9,
/*236*/	Calpha_perio, Calpha_excla,
/*238*/	Calpha_colon, Calpha_quest,
/*240*/	Calpha_arrow_e/**/, Cdecor_1/*alpha_copyr*/,
/*242*/	Cfake_space/**/, Cfake_space/**/,
/*244*/	Cfake_space/**/, Cfake_space/**/,
/*246*/	Cfake_space/**/, Cfake_space/**/,
/*248*/	Cfake_space/**/, Cfake_space/**/,
/*250*/	Cfake_space/**/, Cfake_space/**/,
/*252*/	Cfake_space/**/, Cfake_space/**/,
/*254*/	Cfake_space/**/, Cfake_space/**/,
};
static const short map_four_eater[32] = {
/* 0*/	Cspace, Cemerald,
/* 2*/	Cdiamond, Cstone,
/* 4*/	Calien, Ctank_1_n,
/* 6*/	Cbomb, Cbug_1_n,
/* 8*/	Cdrip, Cnut,
/*10*/	Ceater_n, Csand_stone,
/*12*/	Cgrass, Croundwall_1,
/*14*/	Csteel_1, Cwall_1,
/*16*/	Ckey_1, Ckey_2,
/*18*/	Ckey_3, Ckey_4,
/*20*/	Cwonderwall, Cwheel,
/*22*/	Cdynamite, Cexit_2/**/,
/*24*/	Cexit_3/**/, Cexit_1/**/,
/*26*/	Cdiamond/*diamond_shine*/, Cemerald/*emerald_shine*/,
/*28*/	Cspace/**/, Cspace/**/,
/*30*/	Cspace/**/, Cspace/**/,
};

static void version_four(char buf[2106], struct CAVE *cav)
{
	int x;
	int y;
	int n;
	int ptr;

/* common to all caves */

	cav->width = 64;
	cav->height = 32;
	cav->infinite = 1;

	cav->time = (in_byte(2094) * 500 + 7) / 8;
	cav->diamonds = in_byte(2095);

/* scores */

	cav->eater_score = in_byte(2089);
	cav->alien_score = in_byte(2086);
	cav->bug_score = in_byte(2088);
	cav->tank_score = in_byte(2087);
	cav->emerald_score = in_byte(2084);
	cav->diamond_score = in_byte(2085);
	cav->nut_score = in_byte(2090);
	cav->dynamite_score = in_byte(2091);
	cav->key_score = in_byte(2092);
	cav->exit_score = in_byte(2093) * 8 / 5;

/* times */

	cav->ameuba_time = in_half(2100) * 8;
	cav->wonderwall_time = in_half(2102);
	cav->wheel_time = in_half(2104);

/* eaters */

	for(x = 0; x < 8; x++) {
		ptr = 2048 + (x % 4) * 9;
		for(y = 0; y < 9; y++) {
			cav->eater_array[x][y] = map_four_eater[in_byte(ptr) & 31];
			ptr++;
		}
	}

/* cave */

	ptr = 0;
	for(y = 0; y < 32; y++) for(x = 0; x < 64; x++) {
		cav->cave[x][y] = map_four[in_byte(ptr)];
		ptr++;
	}

/* acid */

	for(x = 0; x < 64; x++) for(y = 0; y < 32; y++) {
		if(y < 31 && cav->cave[x][y + 1] == Cacid_s) {
			cav->cave[x][y] = Cacid_1;
			if(y < 30 && cav->cave[x][y + 2] == Cacid_s) {
				cav->cave[x][y] = Cspace;
			}
		}
	}

/* players */

	ptr = 2096;
	for(n = 0; n < 2; n++) {
		x = in_half(ptr); x = (x >> 0 & 63);
		y = in_half(ptr); y = (y >> 6 & 31);
		cav->player_x[n] = x;
		cav->player_y[n] = y;
		cav->cave[x][y] = Cspace;
		ptr += 2;
	}

/* special cases */

	if(cav->time == 0) cav->time = 9999;
	if(cav->wheel_time == 0) cav->wheel_time = 9999;
	for(ptr = 0; ptr < 2048; ptr++) {
		switch(in_byte(ptr)) {
		case 36: /* spinning wonderwall */
			cav->wonderwall_active = 1;
			cav->wonderwall_time = 9999;
			break;
		}
	}
}

static const short map_five[256] = {
/*  0*/	Cstone, Cstone/*stone_fall*/,
/*  2*/	Cdiamond, Cdiamond/*diamond_fall*/,
/*  4*/	Calien, Calien/*alien_pause*/,
/*  6*/	Cpause/*boom_1*/, Cpause/*boom_2*/,
/*  8*/	Ctank_1_n, Ctank_1_e,
/* 10*/	Ctank_1_s, Ctank_1_w,
/* 12*/	Ctank_2_n, Ctank_2_e,
/* 14*/	Ctank_2_s, Ctank_2_w,
/* 16*/	Cbomb, Cbomb/*bomb_fall*/,
/* 18*/	Cemerald, Cemerald/*emerald_fall*/,
/* 20*/	Cbug_1_n, Cbug_1_e,
/* 22*/	Cbug_1_s, Cbug_1_w,
/* 24*/	Cbug_2_n, Cbug_2_e,
/* 26*/	Cbug_2_s, Cbug_2_w,
/* 28*/	Cdrip, Cdrip/*drip_fall_1*/,
/* 30*/	Cspace/*drip_fall_2*/, Cdrip/*drip_fall_3*/,
/* 32*/	Cstone/*stone_pause*/, Cbomb/*bomb_pause*/,
/* 34*/	Cdiamond/*diamond_pause*/, Cemerald/*emerald_pause*/,
/* 36*/	Cwonderwall/*wonderwall*/, Cnut,
/* 38*/	Cnut/*nut_fall*/, Cnut/*nut_pause*/,
/* 40*/	Cwheel/*wheel*/, Ceater_n,
/* 42*/	Ceater_s, Ceater_w,
/* 44*/	Ceater_e, Csand_stone,
/* 46*/	Cspace/*sand_stone_2_sB*/, Cspace/*sand_stone_3_sB*/,
/* 48*/	Cspace/*sand_stone_4_sB*/, Csand/*sand_sand_2_sB*/,
/* 50*/	Csand/*sand_sand_3_sB*/, Csand/*sand_sand_4_sB*/,
/* 52*/	Csand_stone/*sand_sand_2_s*/, Csand_stone/*sand_sand_3_s*/,
/* 54*/	Csand_stone/*sand_sand_4_s*/, Csand/*sand_sand_4_sB*/,
/* 56*/	Cstone/*sand_stone_2_s*/, Cslide_ew,
/* 58*/	Cslide_ns, Cdynamite_1,
/* 60*/	Cdynamite_2, Cdynamite_3,
/* 62*/	Cdynamite_4, Cacid_s,
/* 64*/	Cexit_1, Cexit_2,
/* 66*/	Cexit_3, Cballoon,
/* 68*/	Cplant, Cfake_space/**/,
/* 70*/	Cfake_space/**/, Cfake_space/**/,
/* 72*/	Cfake_space/**/, Cfake_space/**/,
/* 74*/	Cfake_space/**/, Cfake_space/**/,
/* 76*/	Cfake_space/**/, Cfake_space/**/,
/* 78*/	Cfake_space/**/, Cfake_space/**/,
/* 80*/	Cfake_space/**/, Cfake_space/**/,
/* 82*/	Cfake_space/**/, Cfake_space/**/,
/* 84*/	Cfake_space/**/, Cfake_space/**/,
/* 86*/	Cfake_space/**/, Cfake_space/**/,
/* 88*/	Cfake_space/**/, Cfake_space/**/,
/* 90*/	Cfake_space/**/, Cfake_space/**/,
/* 92*/	Cfake_space/**/, Cfake_space/**/,
/* 94*/	Cfake_space/**/, Cfake_space/**/,
/* 96*/	Cfake_space/**/, Cfake_space/**/,
/* 98*/	Cfake_space/**/, Cfake_space/**/,
/*100*/	Cfake_space/**/, Cplant/*acid_1*/,
/*102*/	Cplant/*acid_2*/, Cplant/*acid_3*/,
/*104*/	Cplant/*acid_4*/, Cplant/*acid_5*/,
/*106*/	Cplant/*acid_6*/, Cplant/*acid_7*/,
/*108*/	Cplant/*acid_8*/, Cpause/*grass_w*/,
/*110*/	Cpause/*grass_e*/, Cpause/*grass_n*/,
/*112*/	Cpause/*grass_s*/, Cpause/*dynamite_space*/,
/*114*/	Cpush_nut_w, Cpush_nut_e,
/*116*/	Csteel_2, Cdynamite_4/*boom_2*/,
/*118*/	Cpause/*emerald_space*/, Cpush_bomb_w,
/*120*/	Cpush_bomb_e, Cpush_stone_w,
/*122*/	Cpush_stone_e, Cpause/*diamond_space*/,
/*124*/	Cpause/*dirt_w*/, Cpause/*dirt_e*/,
/*126*/	Cpause/*dirt_n*/, Cpause/*dirt_s*/,
/*128*/	Cspace, Croundwall_2,
/*130*/	Cgrass, Csteel_2,
/*132*/	Cwall_2, Ckey_1,
/*134*/	Ckey_2, Ckey_3,
/*136*/	Ckey_4, Cdoor_1,
/*138*/	Cdoor_2, Cdoor_3,
/*140*/	Cdoor_4, Cfake_ameuba,
/*142*/	Cfake_door_1, Cfake_door_2,
/*144*/	Cfake_door_3, Cfake_door_4,
/*146*/	Cwonderwall, Cwheel,
/*148*/	Csand, Cacid_nw,
/*150*/	Cacid_ne, Cacid_sw,
/*152*/	Cacid_se, Cfake_space,
/*154*/	Cameuba_1, Cameuba_2,
/*156*/	Cameuba_3, Cameuba_4,
/*158*/	Cexit, Calpha_arrow_w,
/*160*/	Cfake_grass, Cfake_space/**/,
/*162*/	Cfake_space/**/, Cfake_space/**/,
/*164*/	Cfake_space/**/, Cfake_space/**/,
/*166*/	Cfake_space/**/, Cpause/**/,
/*168*/	Cdecor_8, Cdecor_9,
/*170*/	Cdecor_10, Cdecor_5,
/*172*/	Calpha_comma, Calpha_apost,
/*174*/	Calpha_minus, Cdynamite,
/*176*/	Csteel_3, Cdecor_6,
/*178*/	Cdecor_7, Csteel_1,
/*180*/	Croundwall_1, Cdecor_2,
/*182*/	Cdecor_4, Cdecor_3,
/*184*/	Cwind_multi, Cwind_e,
/*186*/	Cwind_s, Cwind_w,
/*188*/	Cwind_n, Cdirt,
/*190*/	Cplant/**/, Cfake_space/**/,
/*192*/	Cfake_space/**/, Cfake_space/**/,
/*194*/	Cfake_space/**/, Cfake_space/**/,
/*196*/	Cfake_space/**/, Cfake_space/**/,
/*198*/	Cfake_space/**/, Cfake_space/**/,
/*200*/	Calpha_a, Calpha_b,
/*202*/	Calpha_c, Calpha_d,
/*204*/	Calpha_e, Calpha_f,
/*206*/	Calpha_g, Calpha_h,
/*208*/	Calpha_i, Calpha_j,
/*210*/	Calpha_k, Calpha_l,
/*212*/	Calpha_m, Calpha_n,
/*214*/	Calpha_o, Calpha_p,
/*216*/	Calpha_q, Calpha_r,
/*218*/	Calpha_s, Calpha_t,
/*220*/	Calpha_u, Calpha_v,
/*222*/	Calpha_w, Calpha_x,
/*224*/	Calpha_y, Calpha_z,
/*226*/	Calpha_0, Calpha_1,
/*228*/	Calpha_2, Calpha_3,
/*230*/	Calpha_4, Calpha_5,
/*232*/	Calpha_6, Calpha_7,
/*234*/	Calpha_8, Calpha_9,
/*236*/	Calpha_perio, Calpha_excla,
/*238*/	Calpha_colon, Calpha_quest,
/*240*/	Calpha_arrow_e, Cdecor_1,
/*242*/	Cfake_space/**/, Cfake_space/**/,
/*244*/	Cfake_space/**/, Cfake_space/**/,
/*246*/	Cfake_space/**/, Cfake_space/**/,
/*248*/	Cfake_space/**/, Cfake_space/**/,
/*250*/	Cfake_space/**/, Cfake_space/**/,
/*252*/	Cfake_space/**/, Cfake_space/**/,
/*254*/	Cfake_space/**/, Cfake_space/**/,
};

static void version_five(char buf[2110], struct CAVE *cav)
{
	int x;
	int y;
	int n;
	int ptr;

/* common to all caves */

	cav->width = 64;
	cav->height = 32;
	cav->infinite = 1;

	cav->time = (in_byte(2094) * 500 + 7) / 8;
	cav->diamonds = in_byte(2095);

/* scores */

	cav->eater_score = in_byte(2089);
	cav->alien_score = in_byte(2086);
	cav->bug_score = in_byte(2088);
	cav->tank_score = in_byte(2087);
	cav->emerald_score = in_byte(2084);
	cav->diamond_score = in_byte(2085);
	cav->nut_score = in_byte(2090);
	cav->dynamite_score = in_byte(2091);
	cav->key_score = in_byte(2092);
	cav->exit_score = in_byte(2093) * 8 / 5;

/* times */

	cav->ameuba_time = in_half(2100) * 8;
	cav->wonderwall_time = in_half(2102);
	cav->wheel_time = in_half(2104);
	cav->wind_time = 9999;

/* eaters */

	for(x = 0; x < 8; x++) {
		ptr = 2048 + (x % 4) * 9;
		for(y = 0; y < 9; y++) {
			cav->eater_array[x][y] = map_five[in_byte(ptr)];
			ptr++;
		}
	}

/* cave */

	ptr = 0;
	for(y = 0; y < 32; y++) for(x = 0; x < 64; x++) {
		cav->cave[x][y] = map_five[in_byte(ptr)];
		ptr++;
	}

/* acid */

	for(x = 0; x < 8; x++) for(y = 0; y < 9; y++) {
		if(y < 6 && cav->eater_array[x][y + 3] == Cacid_s) {
			cav->eater_array[x][y] = Cacid_1;
			if(y < 3 && cav->eater_array[x][y + 6] == Cacid_s) {
				cav->eater_array[x][y] = Cspace;
			}
		}
	}

/* acid */

	for(x = 0; x < 64; x++) for(y = 0; y < 32; y++) {
		if(y < 31 && cav->cave[x][y + 1] == Cacid_s) {
			cav->cave[x][y] = Cacid_1;
			if(y < 30 && cav->cave[x][y + 2] == Cacid_s) {
				cav->cave[x][y] = Cspace;
			}
		}
	}

/* players */

	ptr = 2096;
	for(n = 0; n < 2; n++) {
		x = in_half(ptr); x = (x >> 0 & 63);
		y = in_half(ptr); y = (y >> 6 & 31);
		cav->player_x[n] = x;
		cav->player_y[n] = y;
		cav->cave[x][y] = Cspace;
		ptr += 2;
	}

/* special cases */

	if(cav->time == 0) cav->time = 9999;
	if(cav->wheel_time == 0) cav->wheel_time = 9999;
	for(ptr = 0; ptr < 2048; ptr++) {
		switch(in_byte(ptr)) {
		case 36: /* spinning wonderwall */
			cav->wonderwall_active = 1;
			cav->wonderwall_time = 9999;
			break;
		}
	}
}

static const short map_six[256] = {
/*  0*/	Cstone, Cstone/*stone_fall*/,
/*  2*/	Cdiamond, Cdiamond/*diamond_fall*/,
/*  4*/	Calien, Calien/*alien_pause*/,
/*  6*/	Cpause/*boom_1*/, Cpause/*boom_2*/,
/*  8*/	Ctank_1_n, Ctank_1_e,
/* 10*/	Ctank_1_s, Ctank_1_w,
/* 12*/	Ctank_2_n, Ctank_2_e,
/* 14*/	Ctank_2_s, Ctank_2_w,
/* 16*/	Cbomb, Cbomb/*bomb_fall*/,
/* 18*/	Cemerald, Cemerald/*emerald_fall*/,
/* 20*/	Cbug_1_n, Cbug_1_e,
/* 22*/	Cbug_1_s, Cbug_1_w,
/* 24*/	Cbug_2_n, Cbug_2_e,
/* 26*/	Cbug_2_s, Cbug_2_w,
/* 28*/	Cdrip, Cdrip/*drip_fall_1*/,
/* 30*/	Cspace/*drip_fall_2*/, Cdrip/*drip_fall_3*/,
/* 32*/	Cstone/*stone_pause*/, Cbomb/*bomb_pause*/,
/* 34*/	Cdiamond/*diamond_pause*/, Cemerald/*emerald_pause*/,
/* 36*/	Cwonderwall/*wonderwall*/, Cnut,
/* 38*/	Cnut/*nut_fall*/, Cnut/*nut_pause*/,
/* 40*/	Cwheel, Ceater_n,
/* 42*/	Ceater_s, Ceater_w,
/* 44*/	Ceater_e, Csand_stone,
/* 46*/	Cspace/*sand_stone_2_sB*/, Cspace/*sand_stone_3_sB*/,
/* 48*/	Cspace/*sand_stone_4_sB*/, Csand/*sand_sand_2_sB*/,
/* 50*/	Csand/*sand_sand_3_sB*/, Csand/*sand_sand_4_sB*/,
/* 52*/	Csand_stone/*sand_sand_2_s*/, Csand_stone/*sand_sand_3_s*/,
/* 54*/	Csand_stone/*sand_sand_4_s*/, Csand/*sand_sand_4_sB*/,
/* 56*/	Cstone/*sand_stone_2_s*/, Cslide_ew,
/* 58*/	Cslide_ns, Cdynamite_1,
/* 60*/	Cdynamite_2, Cdynamite_3,
/* 62*/	Cdynamite_4, Cacid_s,
/* 64*/	Cexit_1, Cexit_2,
/* 66*/	Cexit_3, Cballoon,
/* 68*/	Cplant, Cspring,
/* 70*/	Cspring/*spring_fall*/, Cpush_spring_w/*spring_w*/,
/* 72*/	Cpush_spring_e/*spring_e*/, Cball_1,
/* 74*/	Cball_2, Candroid,
/* 76*/	Cpause/**/, Candroid/*android_1_n*/,
/* 78*/	Candroid/*android_2_n*/, Candroid/*android_1_s*/,
/* 80*/	Candroid/*android_2_s*/, Candroid/*android_1_e*/,
/* 82*/	Candroid/*android_2_e*/, Candroid/*android_1_w*/,
/* 84*/	Candroid/*android_2_w*/, Cfake_space/**/,
/* 86*/	Cfake_space/**/, Cfake_space/**/,
/* 88*/	Cfake_space/**/, Cfake_space/**/,
/* 90*/	Cfake_space/**/, Cfake_space/**/,
/* 92*/	Cfake_space/**/, Cfake_space/**/,
/* 94*/	Cfake_space/**/, Cfake_space/**/,
/* 96*/	Cfake_space/**/, Cfake_space/**/,
/* 98*/	Cfake_space/**/, Cpush_spring_w,
/*100*/	Cpush_spring_e, Cplant/*acid_1*/,
/*102*/	Cplant/*acid_2*/, Cplant/*acid_3*/,
/*104*/	Cplant/*acid_4*/, Cplant/*acid_5*/,
/*106*/	Cplant/*acid_6*/, Cplant/*acid_7*/,
/*108*/	Cplant/*acid_8*/, Cpause/*grass_w*/,
/*110*/	Cpause/*grass_e*/, Cpause/*grass_n*/,
/*112*/	Cpause/*grass_s*/, Cpause/*dynamite_space*/,
/*114*/	Cpush_nut_w, Cpush_nut_e,
/*116*/	Csteel_2, Cpause/**/,
/*118*/	Cpause/*emerald_space*/, Cpush_bomb_w,
/*120*/	Cpush_bomb_e, Cpush_stone_w,
/*122*/	Cpush_stone_e, Cpause/*diamond_space*/,
/*124*/	Cpause/*dirt_w*/, Cpause/*dirt_e*/,
/*126*/	Cpause/*dirt_n*/, Cpause/*dirt_s*/,
/*128*/	Cspace, Croundwall_2,
/*130*/	Cgrass, Csteel_2,
/*132*/	Cwall_2, Ckey_1,
/*134*/	Ckey_2, Ckey_3,
/*136*/	Ckey_4, Cdoor_1,
/*138*/	Cdoor_2, Cdoor_3,
/*140*/	Cdoor_4, Cfake_ameuba,
/*142*/	Cfake_door_1, Cfake_door_2,
/*144*/	Cfake_door_3, Cfake_door_4,
/*146*/	Cwonderwall, Cwheel,
/*148*/	Csand, Cacid_nw,
/*150*/	Cacid_ne, Cacid_sw,
/*152*/	Cacid_se, Cfake_space,
/*154*/	Cameuba_1, Cameuba_2,
/*156*/	Cameuba_3, Cameuba_4,
/*158*/	Cexit, Calpha_arrow_w,
/*160*/	Cfake_grass, Clenses,
/*162*/	Cmagnify, Cfake_space/*fake_space*/,
/*164*/	Cfake_grass/*fake_grass*/, Cswitch,
/*166*/	Cswitch/*switch*/, Cpause/**/,
/*168*/	Cdecor_8, Cdecor_9,
/*170*/	Cdecor_10, Cdecor_5,
/*172*/	Calpha_comma, Calpha_apost,
/*174*/	Calpha_minus, Cdynamite,
/*176*/	Csteel_3, Cdecor_6,
/*178*/	Cdecor_7, Csteel_1,
/*180*/	Croundwall_1, Cdecor_2,
/*182*/	Cdecor_4, Cdecor_3,
/*184*/	Cwind_multi, Cwind_e,
/*186*/	Cwind_s, Cwind_w,
/*188*/	Cwind_n, Cdirt,
/*190*/	Cplant/**/, Ckey_5,
/*192*/	Ckey_6, Ckey_7,
/*194*/	Ckey_8, Cdoor_5,
/*196*/	Cdoor_6, Cdoor_7,
/*198*/	Cdoor_8, Cbumper,
/*200*/	Calpha_a, Calpha_b,
/*202*/	Calpha_c, Calpha_d,
/*204*/	Calpha_e, Calpha_f,
/*206*/	Calpha_g, Calpha_h,
/*208*/	Calpha_i, Calpha_j,
/*210*/	Calpha_k, Calpha_l,
/*212*/	Calpha_m, Calpha_n,
/*214*/	Calpha_o, Calpha_p,
/*216*/	Calpha_q, Calpha_r,
/*218*/	Calpha_s, Calpha_t,
/*220*/	Calpha_u, Calpha_v,
/*222*/	Calpha_w, Calpha_x,
/*224*/	Calpha_y, Calpha_z,
/*226*/	Calpha_0, Calpha_1,
/*228*/	Calpha_2, Calpha_3,
/*230*/	Calpha_4, Calpha_5,
/*232*/	Calpha_6, Calpha_7,
/*234*/	Calpha_8, Calpha_9,
/*236*/	Calpha_perio, Calpha_excla,
/*238*/	Calpha_colon, Calpha_quest,
/*240*/	Calpha_arrow_e, Cdecor_1,
/*242*/	Cfake_door_5, Cfake_door_6,
/*244*/	Cfake_door_7, Cfake_door_8,
/*246*/	Cfake_space/**/, Cfake_space/**/,
/*248*/	Cfake_space/**/, Cfake_space/**/,
/*250*/	Cfake_space/**/, Cfake_space/**/,
/*252*/	Cfake_space/**/, Cfake_space/**/,
/*254*/	Cfake_space/**/, Cfake_space/**/,
};

static void version_six(char buf[2172], struct CAVE *cav)
{
	int x;
	int y;
	int n;
	int ptr;

/* common to all caves */

	cav->width = 64;
	cav->height = 32;
	cav->infinite = 1;
	cav->teamwork = (in_byte(2150) & 128) ? 1 : 0;

	cav->time = (in_half(2110) * 50 + 7) / 8;
	cav->diamonds = in_byte(2095);

/* scores */

	cav->eater_score = in_byte(2089);
	cav->alien_score = in_byte(2086);
	cav->bug_score = in_byte(2088);
	cav->tank_score = in_byte(2087);
	cav->emerald_score = in_byte(2084);
	cav->diamond_score = in_byte(2085);
	cav->nut_score = in_byte(2090);
	cav->spring_score = in_byte(2153);
	cav->dynamite_score = in_byte(2091);
	cav->key_score = in_byte(2092);
	cav->lenses_score = in_byte(2151);
	cav->magnify_score = in_byte(2152);
	cav->exit_score = in_byte(2093) * 8 / 5;

/* times */

	cav->android_move_time = in_half(2164);
	cav->android_clone_time = in_half(2166);
	cav->ball_time = in_half(2160);
	cav->ameuba_time = in_half(2100) * 8;
	cav->wonderwall_time = in_half(2102);
	cav->wheel_time = in_half(2104);
	cav->lenses_time = in_half(2154);
	cav->magnify_time = in_half(2156);

/* wind */

	cav->wind_time = 9999;
	n = in_byte(2149);
	cav->wind_direction = (n & 8) ? 0 : (n & 1) ? 1 : (n & 2) ? 2 : (n & 4) ? 3 : 0;
	cav->wind_active = (n & 15) ? 1 : 0;

/* magic ball */

	for(x = 0; x < 8; x++) cav->ball_array[x] = map_six[in_byte(2159)];

	n = in_byte(2162);
	cav->ball_random = (n & 1) ? 1 : 0;
	cav->ball_active = (n & 128) ? 1 : 0;
	if((n & 1) == 0) {
		n = in_byte(2163);
		if((n & 1) == 0) cav->ball_array[1] = Cspace; /* north */
		if((n & 4) == 0) cav->ball_array[3] = Cspace; /* west */
		if((n & 8) == 0) cav->ball_array[4] = Cspace; /* east */
		if((n & 2) == 0) cav->ball_array[6] = Cspace; /* south */
		if((n & 128) == 0) cav->ball_array[0] = Cspace; /* north west */
		if((n & 64) == 0) cav->ball_array[2] = Cspace; /* north east */
		if((n & 32) == 0) cav->ball_array[5] = Cspace; /* south west */
		if((n & 16) == 0) cav->ball_array[7] = Cspace; /* south east */
	}

/* android */

	n = in_half(2168);
	cav->android_eater = (n & 64) ? 1 : 0;
	cav->android_alien = (n & 256) ? 1 : 0;
	cav->android_bug = (n & 128) ? 1 : 0;
	cav->android_tank = (n & 32) ? 1 : 0;
	cav->android_emerald = (n & 1) ? 1 : 0;
	cav->android_diamond = (n & 2) ? 1 : 0;
	cav->android_stone = (n & 4) ? 1 : 0;
	cav->android_bomb = (n & 8) ? 1 : 0;
	cav->android_nut = (n & 16) ? 1 : 0;
	cav->android_spring = (n & 512) ? 1 : 0;
	cav->android_dynamite = (n & 4096) ? 1 : 0;
	cav->android_balloon = (n & 1024) ? 1 : 0;
	cav->android_ameuba = (n & 2048) ? 1 : 0;

/* eaters */

	for(x = 0; x < 8; x++) {
		ptr = 2048 + (x / 4) * 64 + (x % 4) * 9;
		for(y = 0; y < 9; y++) {
			cav->eater_array[x][y] = map_six[in_byte(ptr)];
			ptr++;
		}
	}

/* cave */

	ptr = 0;
	for(y = 0; y < 32; y++) for(x = 0; x < 64; x++) {
		cav->cave[x][y] = map_six[in_byte(ptr)];
		ptr++;
	}

/* acid */

	for(x = 0; x < 8; x++) for(y = 0; y < 9; y++) {
		if(y < 6 && cav->eater_array[x][y + 3] == Cacid_s) {
			cav->eater_array[x][y] = Cacid_1;
			if(y < 3 && cav->eater_array[x][y + 6] == Cacid_s) {
				cav->eater_array[x][y] = Cspace;
			}
		}
	}

/* acid */

	for(x = 0; x < 64; x++) for(y = 0; y < 32; y++) {
		if(y < 31 && cav->cave[x][y + 1] == Cacid_s) {
			cav->cave[x][y] = Cacid_1;
			if(y < 30 && cav->cave[x][y + 2] == Cacid_s) {
				cav->cave[x][y] = Cspace;
			}
		}
	}

/* players */

	ptr = 2096;
	for(n = 0; n < 2; n++) {
		x = in_half(ptr); x = (x >> 0 & 63);
		y = in_half(ptr); y = (y >> 6 & 31);
		cav->player_x[n] = x;
		cav->player_y[n] = y;
		cav->cave[x][y] = Cspace;
		ptr += 2;
	}

/* special cases */

	if(cav->time == 0) cav->time = 9999;
	for(ptr = 0; ptr < 2048; ptr++) {
		switch(in_byte(ptr)) {
		case 36: /* spinning wonderwall */
			cav->wonderwall_active = 1;
			cav->wonderwall_time = 9999;
			break;
		case 40: /* spinning wheel */
			cav->wheel_active = 1;
			x = (ptr >> 0 & 63);
			y = (ptr >> 6 & 31);
			cav->wheel_x = x; cav->wheel_y = y;
			break;
		case 163: /* visible fake space */
			cav->lenses_active = 1;
			break;
		case 164: /* visible fake grass */
			cav->magnify_active = 1;
			break;
		}
	}
}

#include <string.h>

void binary_to_cave(char buf[2172], struct CAVE *cav)
{
	memset(cav, 0, sizeof(*cav));

	if(in_byte(2106) == 255 && in_byte(2107) == 54 && in_byte(2108) == 48 && in_byte(2109) == 48) {
		return(version_six(buf, cav));
	}
	if(in_byte(2106) == 255 && in_byte(2107) == 53 && in_byte(2108) == 48 && in_byte(2109) == 48) {
		return(version_five(buf, cav));
	}
	if(in_byte(1983) == 116 || in_byte(2047) == 116) {
		return(version_four(buf, cav));
	}
	if(in_byte(1983) == 27 || in_byte(2047) == 219) {
		int n; buf[0] = 131; buf[1] = 131; for(n = 2; n < 2106; n++) buf[n] = (buf[n] ^ (n * 7 + 101)) - 17; /* decrypt */
		return(version_four(buf, cav));
	}
	return;
}
