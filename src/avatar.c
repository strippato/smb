/* smb
 * Copyright (c) 2010, Strippato
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 * Should you need to contact me, the author, you can do so either by
 * e-mail - mail your message to <strippato@gmail.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h>
#include <libconfig.h>
#include <assert.h>

#ifdef _WII_MOTE
#include <wiimote_api.h>
#endif

#include "global.h"
#include "input.h"
#include "game.h"
#include "link.h"
#include "avatar.h"
#include "camera.h"
#include "graph.h"
#include "input.h"
#include "joy.h"
#include "wii.h"
#include "world.h"
#include "util.h"
#include "sound.h"
#include "tile.h"
#include "map.h"
#include "text.h"
#include "particle.h"
#include "post.h"

#define AVATAR_DEFAULT_LIFE	3
#define AVATAR_MAX_X_VEL	2.30f
#define AVATAR_MAX_Y_VEL	8.00f
#define AVATAR_MAX_ACC		0.06f
#define AVATAR_MAX_DEC		0.04f

#define AVATAR_G_FORCE		0.15f

#define AVATAR_JMPVEL		6.20f

#define AVATAR_FRAME_SPEED	18.0f

#define AVATAR_AFK_TICK		2000

#define AVATAR_COINS_4_LIFE	100

/* bounding box */
#define AVATAR_BBOXX	8
#define AVATAR_BBOXY	3
#define AVATAR_BBOXW	18
#define AVATAR_BBOXH	28

/* link */
#define AVATAR_LINKIN_TIMER	70
#define AVATAR_LINKOUT_TIMER	70
#define AVATAR_LINK_REDUCTION	5	/* link collision, need a small avatar-boundingbox:  5 reduction= AVATARBBOX -  5*2 pixel */

/* text */
#define AVATAR_FONT		"data/avatar/comic.ttf"
#define AVATAR_FONT_SIZE	14
#define AVATAR_FONT_DELTAX	0
#define AVATAR_FONT_DELTAY	-3
#define AVATAR_FONT_LIFE	650
#define AVATAR_FONT_SHADOWX	2
#define AVATAR_FONT_SHADOWY	2
#define AVATAR_FONT_FADEOUT	250
#define AVATAR_FONT_FADEIN	100
#define AVATAR_FONT_ALFA	85

#define AVATAR_DEAD_TIME	350

#define AVATAR_COLLISION_FACTOR_X (95.0f/100.0f)
#define AVATAR_COLLISION_FACTOR_Y (95.0f/100.0f)
#define AVATAR_COLLISION_RECOIL 0.85f
/*#define AVATAR_COLLISION_MINVEL 0.00f */

struct avatar_t avatar[GAME_MAX_PLAYER];

static TTF_Font  *avatar_font;
static SDL_Color avatar_color_text	 = {0xFF, 0xFF, 0xFF, 0x00};
static SDL_Color avatar_color_textbk = {0x00, 0x00, 0x00, 0x00};

/* forward decl */
inline static void avatar_GetBBox     (int player,long int *ax, long int *ay, long int *bx, long int *by);
inline static void avatar_GetBBoxLinkX(int player,long int *ax, long int *ay, long int *bx, long int *by);
inline static void avatar_GetBBoxLinkY(int player,long int *ax, long int *ay, long int *bx, long int *by);

static void avatar_DeadTick(int player);
static void avatar_LinkTick(int player);

static SDL_bool avatar_XCollisionAvatar2AvatarExt(int a, SDL_bool avatar_flag[]);
static SDL_bool avatar_YCollisionAvatar2AvatarExt(int a, SDL_bool avatar_flag[]);

static SDL_bool avatar_TestCollisionAvatar2Avatar(int a, int b);
static SDL_bool avatar_CanWalkOn(int a);

void avatar_Init(void)
{
	int i;

	avatar_font = TTF_OpenFont(AVATAR_FONT, AVATAR_FONT_SIZE);
	if (!avatar_font) {
		printf("Could not find avatar font: %s\n", AVATAR_FONT);
		exit(EXIT_FAILURE);
	}

	for(i = 0; i < GAME_MAX_PLAYER; ++i) {
		avatar[i].enabled = SDL_FALSE;
		avatar[i].visible = SDL_TRUE;
		avatar[i].device  = DEV_NONE;
		avatar[i].sprite  = NULL;
		avatar[i].name = NULL;
		avatar[i].easy_jump = 0;

		avatar[i].life  = 0;
		avatar[i].score = 0;
		avatar[i].coin  = 0;

		avatar[i].vx = avatar[i].old_vx = 0;
		avatar[i].vy = avatar[i].old_vy = 0;
		avatar[i].react_vx = avatar[i].react_vy = 0;
		avatar[i].kf_age = 0.0;
		avatar[i].kf = 0;
		
		avatar[i].x = avatar[i].old_x = 0;
		avatar[i].y = avatar[i].old_y = 0;

		avatar[i].lookat = AVATAR_DX;
		avatar[i].state  = AVATAR_STAY;

		avatar[i].text_ttl = 0;
		avatar[i].text_fadein = 0;

		assert(!avatar[i].text);
		avatar[i].text = NULL;

		assert(!avatar[i].textbk);
		avatar[i].textbk = NULL;

		assert(!avatar[i].textbox);
		avatar[i].textbox = NULL;
		
		avatar[i].afk = AVATAR_AFK_TICK;
		
		avatar[i].isdying   = 0;
		avatar[i].linktimer = 0;
		avatar[i].link = NULL;
	}

	sound_SfxLoad(SND_JUMP_01, "Yo!.ogg");
	sound_SfxLoad(SND_JUMP_02, "Yah.ogg");
	sound_SfxLoad(SND_JUMP_03, "Hoo.ogg");
	sound_SfxLoad(SND_JUMP_04, "Woohoo!.ogg");

	sound_SfxLoad(SND_STEP_01, "Footstep1.ogg");
	sound_SfxLoad(SND_STEP_02, "Footstep2.ogg");
	sound_SfxLoad(SND_STEP_03, "Footstep3.ogg");
	sound_SfxLoad(SND_STEP_04, "Footstep4.ogg");

	sound_SfxLoad(SND_LIFEUP, "1up.ogg");
	sound_SfxLoad(SND_DEAD,   "dead.ogg");
	sound_SfxLoad(SND_LINK,   "link.ogg");
}

void avatar_Free(void) 
{
	int i;

	TTF_CloseFont(avatar_font);
	avatar_font = NULL;
	
	for(i = 0; i < GAME_MAX_PLAYER; ++i) {
		avatar[i].enabled = SDL_FALSE;
		avatar[i].visible = SDL_FALSE;
		avatar[i].device  = DEV_NONE;
		avatar[i].easy_jump = 0;
		
		if (avatar[i].sprite) {
			SDL_FreeSurface(avatar[i].sprite);
			avatar[i].sprite  = NULL;
		}

		if (avatar[i].name) {
			free(avatar[i].name);
			avatar[i].name = NULL;
		}

		avatar[i].life  = 0;
		avatar[i].score = 0;
		avatar[i].coin  = 0;

		avatar[i].vx = 0;
		avatar[i].vy = 0;
		avatar[i].old_vx = 0;
		avatar[i].old_vy = 0;
		avatar[i].react_vx = 0;
		avatar[i].react_vy = 0;

		avatar[i].x = 0.0;
		avatar[i].y = 0.0;
		avatar[i].old_x = 0.0;
		avatar[i].old_y = 0.0;
		
		avatar[i].kf_age = 0.0;
		avatar[i].kf     = 0;

		avatar[i].state  = AVATAR_STAY;
		avatar[i].lookat = AVATAR_DX;	

		avatar[i].text_ttl = 0;
		avatar[i].text_fadein = 0;

		if (avatar[i].text) SDL_FreeSurface(avatar[i].text);
		avatar[i].text = NULL;

		if (avatar[i].textbk) SDL_FreeSurface(avatar[i].textbk);
		avatar[i].textbk = NULL;

		if (avatar[i].textbox) SDL_FreeSurface(avatar[i].textbox);
		avatar[i].textbox = NULL;

		avatar[i].afk = AVATAR_AFK_TICK;
		avatar[i].isdying   = 0;
		avatar[i].linktimer = 0;
		avatar[i].link = NULL;
	}

	sound_SfxUnload(SND_JUMP_01);
	sound_SfxUnload(SND_JUMP_02);
	sound_SfxUnload(SND_JUMP_03);
	sound_SfxUnload(SND_JUMP_04);

	sound_SfxUnload(SND_STEP_01);
	sound_SfxUnload(SND_STEP_02);
	sound_SfxUnload(SND_STEP_03);
	sound_SfxUnload(SND_STEP_04);

	sound_SfxUnload(SND_LIFEUP);
	sound_SfxUnload(SND_DEAD);
	sound_SfxUnload(SND_LINK);
	
}

void avatar_Tick(void)
{
	SDL_bool av_collision, av_last_collision; /* avatar 2 avatar collision flag */
	SDL_bool map_collision, map_last_collision; /* avatar 2 map collision flag */
	SDL_bool collision_flag[GAME_MAX_PLAYER], last_collision_flag[GAME_MAX_PLAYER]; /* avatar 2 avatat collision flag */
	SDL_bool collision_map_x[GAME_MAX_PLAYER][GAME_MAX_PLAYER]; /* avatar 2 avatat collision map X*/
	SDL_bool collision_map_y[GAME_MAX_PLAYER][GAME_MAX_PLAYER]; /* avatar 2 avatat collision map Y*/
	int i, ii;
	long int ax, ay, bx, by;
	long int ax_old, ay_old, bx_old, by_old;
	float vy_old;
	struct link_t *link;
	struct graph_vec2_t pos, vel;
		
	int sgn_v = 0;
	SDL_bool player_jump;

	for(i = 0; i < GAME_MAX_PLAYER; ++i) {
		player_jump = SDL_FALSE;

		if (avatar[i].enabled) {
			for (ii=0; ii < GAME_MAX_PLAYER; ++ii) {
				collision_map_x[i][ii] = collision_map_y[i][ii] = SDL_FALSE;
			}

			/* save old position */
			avatar[i].old_x = avatar[i].x;
			avatar[i].old_y = avatar[i].y;

			/* save old speed */
			avatar[i].old_vx = avatar[i].vx;
			avatar[i].old_vy = avatar[i].vy;

			if ((avatar[i].state == AVATAR_LINKIN) || (avatar[i].state == AVATAR_LINKOUT)) {
				avatar_LinkTick(i);
			} else if (avatar[i].state == AVATAR_DEAD) {
				/* avatar is dying */
				if (avatar[i].isdying > 0) {
					avatar_DeadTick(i);
					map_SetTime(0);
				} else {
					avatar[i].isdying = 0;
					avatar_Kill(i);
					avatar_Respawn(i);
					camera_Look();
				}
			} else {
				/* avatar is player controlled */
				if (!input_key[avatar[i].device][KEY_JUMP]) avatar[i].easy_jump = 0;
	
				avatar_GetBBox(i, &ax, &ay, &bx, &by);
				if ((map_CanWalkOn(ax, ay , bx, by) || avatar_CanWalkOn(i)) && avatar[i].vy >= 0) {
					avatar[i].vy = 0;

					if (avatar[i].state == AVATAR_JUMP) sound_SfxPlay(SND_STEP_01+ rand()%4);
		
					switch (game_easy_jmp) {
					case 1: /* easy jump */ 
						if (input_key[avatar[i].device][KEY_JUMP]) {
							if (avatar[i].easy_jump == 0) {
								player_jump = SDL_TRUE;
								avatar[i].easy_jump = 1;
							}
						}
						break;
					case 2: /* lazy jump */
						if (input_key[avatar[i].device][KEY_JUMP]) player_jump = SDL_TRUE;
						break;
					default:
						 /* normal jump */
						if ((input_key[avatar[i].device][KEY_JUMP]) && (!input_key_old[avatar[i].device][KEY_JUMP])) player_jump = SDL_TRUE;					 
						break;
					}

					if (player_jump) {
						avatar[i].vy = -AVATAR_JMPVEL;
						avatar[i].state = AVATAR_JUMP;
						if (fabs(avatar[i].vx) >= AVATAR_MAX_X_VEL) {
							 /* hohuuuuuu */
							sound_SfxPlay(SND_JUMP_01 + rand()%4);
						} else {
							sound_SfxPlay(SND_JUMP_01 + rand()%3);
						}
					} else {
						avatar[i].state = AVATAR_STAY;
						if (input_key[avatar[i].device][KEY_RIGHT]) {
							avatar[i].vx += AVATAR_MAX_ACC;
							avatar[i].lookat = AVATAR_DX;
							avatar[i].state = AVATAR_RUN;
						}
						if (input_key[avatar[i].device][KEY_LEFT]) {
							avatar[i].vx -= AVATAR_MAX_ACC;
							avatar[i].lookat = AVATAR_SX;
							avatar[i].state = AVATAR_RUN;
						}
					}

					if (!input_key[avatar[i].device][KEY_RIGHT] && !input_key[avatar[i].device][KEY_LEFT]) {
						if (avatar[i].vx > 0) {
							avatar[i].vx -= AVATAR_MAX_DEC;
							if (avatar[i].vx < 0) avatar[i].vx = 0;
						}
						if (avatar[i].vx < 0) {
							avatar[i].vx += AVATAR_MAX_DEC;
							if (avatar[i].vx > 0) avatar[i].vx = 0;
						}
					}
					if (input_key[avatar[i].device][KEY_RIGHT] && input_key[avatar[i].device][KEY_LEFT]) {
						if (avatar[i].vx > 0) {
							avatar[i].vx -= AVATAR_MAX_DEC;
							if (avatar[i].vx < 0) avatar[i].vx = 0;
						}
						if (avatar[i].vx < 0) {
							avatar[i].vx += AVATAR_MAX_DEC;
							if (avatar[i].vx > 0) avatar[i].vx = 0;
						}
					}

				} else {
					/* here Mario is on-air*/

					/* G force */
					avatar[i].state = AVATAR_JUMP;
					avatar[i].vy += AVATAR_G_FORCE;

					/* relax some control */
					if (input_key[avatar[i].device][KEY_LEFT]) {
						avatar[i].vx -= (AVATAR_MAX_ACC / 2.9);
						avatar[i].lookat = AVATAR_SX;
					}
					if (input_key[avatar[i].device][KEY_RIGHT]) {
						avatar[i].vx += (AVATAR_MAX_ACC / 2.9);
						avatar[i].lookat = AVATAR_DX;
					}
				}

				/* V limit */
				if (fabs(avatar[i].vx) > AVATAR_MAX_X_VEL) avatar[i].vx = SGN(avatar[i].vx) * AVATAR_MAX_X_VEL;
				if (fabs(avatar[i].vy) > AVATAR_MAX_Y_VEL) avatar[i].vy = SGN(avatar[i].vy) * AVATAR_MAX_Y_VEL;

				/* spawn some fog's particle */
				if (avatar[i].state != AVATAR_JUMP) {
					/* from > to < */
					if ((avatar[i].vx >= (AVATAR_MAX_X_VEL - AVATAR_MAX_ACC)) && (input_key[avatar[i].device][KEY_LEFT]) && (!input_key_old[avatar[i].device][KEY_LEFT])) {

						pos.x = avatar[i].x + AVATAR_SIZE; 
						pos.y = avatar[i].y + AVATAR_SIZE;
						vel.x = LIM(fabs(avatar[i].vx), 1);
						vel.y = -.1;
						particle_MultiSpawn(PARTICLE_FOG, pos, vel);
					}

					/* from < to > */
					if ((avatar[i].vx <= -AVATAR_MAX_X_VEL + AVATAR_MAX_ACC) && (input_key[avatar[i].device][KEY_RIGHT]) && (!input_key_old[avatar[i].device][KEY_RIGHT])) {
						pos.x = avatar[i].x; 
						pos.y = avatar[i].y + AVATAR_SIZE;
						vel.x = -LIM(fabs(avatar[i].vx), 1);
						vel.y = -.1;
						particle_MultiSpawn(PARTICLE_FOG, pos, vel);
					}
				}

				vy_old = avatar[i].vy;

				if (avatar[i].vy) {
					sgn_v = SGN(avatar[i].vy);
					avatar[i].y += avatar[i].vy;
					avatar_GetBBox(i, &ax, &ay, &bx, &by);
					ax_old = ax;
					ay_old = ay;
					bx_old = bx;
					by_old = by;

					/* get out of solid box*/
					av_collision = av_last_collision = SDL_FALSE;
					map_collision = map_last_collision = SDL_FALSE;
					for (ii=0; ii <GAME_MAX_PLAYER; ++ii) {
						last_collision_flag[ii] = collision_flag[ii] = SDL_FALSE;
					}
					while ((map_collision = map_TestSolidCollision(ax, ay, bx, by)) || (av_collision = avatar_YCollisionAvatar2AvatarExt(i, collision_flag))) {
						avatar[i].y = floor(avatar[i].y) - sgn_v + .5;
						/* stop the player only on map-collision */
						if (map_collision) avatar[i].vy = 0;
						ax_old = ax;
						ay_old = ay;
						bx_old = bx;
						by_old = by;

						avatar_GetBBox(i, &ax, &ay, &bx, &by);

						/* copy the collision info */
						av_last_collision = av_collision;
						map_last_collision = map_collision;
						for (ii=0; ii <GAME_MAX_PLAYER; ++ii) {
							last_collision_flag[ii] = collision_flag[ii];
						}
					}
					if (av_last_collision) {
						for (ii = 0; ii < GAME_MAX_PLAYER; ++ii) {
							if (ii == i) continue;
							if (last_collision_flag[ii])	{
								collision_map_y[i][ii] = SDL_TRUE;
							}
						}
					}

					if (vy_old < 0) {
						/* hit last tile */
						map_SolidCollision(i, ax_old, ay_old, bx_old, by_old);
					}
				}

				if (avatar[i].vx) {
					sgn_v = SGN(avatar[i].vx);
					avatar[i].x += avatar[i].vx;

					/* map limits */
					if ((avatar[i].x) < 0 ) {
						avatar[i].x  = 0;
						avatar[i].vx = 0;
					}
					if ((avatar[i].x + AVATAR_SIZE) > (map.size_w * TILE_SIZE)) {
						avatar[i].x  = (map.size_w * TILE_SIZE) - AVATAR_SIZE;
						avatar[i].vx = 0;
					}
				
					avatar_GetBBox(i, &ax, &ay, &bx, &by);

					/* get out of solid box */
					av_collision = av_last_collision = SDL_FALSE;
					map_collision = map_last_collision = SDL_FALSE;
					for (ii=0; ii <GAME_MAX_PLAYER; ++ii) {
						last_collision_flag[ii] = collision_flag[ii] = SDL_FALSE;
					}
					while ((map_collision = map_TestSolidCollision(ax, ay, bx, by)) || (av_collision = avatar_XCollisionAvatar2AvatarExt(i, collision_flag))) { 
						avatar[i].x = floor(avatar[i].x) - sgn_v + .5;

						/* stop the player only on map-collision */
						if (map_collision) avatar[i].vx = 0;

						avatar_GetBBox(i, &ax, &ay, &bx, &by);

						/* copy the collision info */
						av_last_collision = av_collision;
						map_last_collision = map_collision;
						for (ii=0; ii <GAME_MAX_PLAYER; ++ii) {
							last_collision_flag[ii] = collision_flag[ii];
						}

					}
					if (av_last_collision) {
						for (ii = 0; ii < GAME_MAX_PLAYER; ++ii) {
							if (ii == i) continue;
							if (last_collision_flag[ii])	{
								collision_map_x[i][ii] = SDL_TRUE;
							}
						}
					}
				}

				avatar_GetBBox(i, &ax, &ay, &bx, &by);

				/* hit not solid tile (coins, bonus, ...) */
				map_UnsolidCollision(i, ax, ay, bx, by);
				
				/* outside map Y? Kill Mario! */
				if (by >= map.size_h * TILE_SIZE) {
					avatar_StartToDie(i);
				}

				if ((avatar[i].state == AVATAR_STAY) && (avatar[i].vx != 0)) avatar[i].state = AVATAR_RUN;
				
				if (input_key[avatar[i].device][KEY_RIGHT] && input_key[avatar[i].device][KEY_LEFT]) {
					if ((avatar[i].state == AVATAR_RUN ) && (avatar[i].vx == 0)) avatar[i].state = AVATAR_STAY;
				}
				
				/* link? */
				if (input_key[avatar[i].device][KEY_DOWN] || input_key[avatar[i].device][KEY_UP] || input_key[avatar[i].device][KEY_LEFT] || input_key[avatar[i].device][KEY_RIGHT]) {

					/* hperlink need the avatar size smaller than the real bbox */
					avatar_GetBBoxLinkX(i, &ax, &ay, &bx, &by);
					
					if (input_key[avatar[i].device][KEY_DOWN]) {
						link = map_HyperLinkUp(ax, ay, bx, by);
						if (link) {
							if (link->multilink) {
								avatar_LinkAll(link);
							} else {
								avatar_Link(i, link);
							}
						}
					}

					if (input_key[avatar[i].device][KEY_UP]) {
						link = map_HyperLinkDown(ax, ay, bx, by);
						if (link) {
							if (link->multilink) {
								avatar_LinkAll(link);
							} else {
								avatar_Link(i, link);
							}
						}
					}

					/* hperlink need the avatar-size smaller than the real bbox */
					avatar_GetBBoxLinkY(i, &ax, &ay, &bx, &by);

					if (input_key[avatar[i].device][KEY_LEFT]) {
						link = map_HyperLinkRight(ax, ay, bx, by);
						if (link) {
							if (link->multilink) {
								avatar_LinkAll(link);
							} else {
								avatar_Link(i, link);
							}
						}
					}

					if (input_key[avatar[i].device][KEY_RIGHT]) {
						link = map_HyperLinkLeft(ax, ay, bx, by);
						if (link) {
							if (link->multilink) {
								avatar_LinkAll(link);
							} else {
								avatar_Link(i, link);
							}
						}
					}
				
				}
			}

			/* avatar-text */
			if (avatar[i].text_ttl > 0) {
				avatar[i].text_ttl--;
			}
			if (avatar[i].text_fadein > 0)  {
				avatar[i].text_fadein--;
			}
			
			if (avatar[i].text || avatar[i].textbk || avatar[i].textbox) {
				if (avatar[i].text_ttl <= 0) {
					assert(avatar[i].text);
					assert(avatar[i].textbk);
					assert(avatar[i].textbox);

					SDL_FreeSurface(avatar[i].text);
					SDL_FreeSurface(avatar[i].textbk);
					SDL_FreeSurface(avatar[i].textbox);

					avatar[i].text     = NULL;
					avatar[i].textbk   = NULL;
					avatar[i].textbox  = NULL;
					avatar[i].text_ttl = 0;
				}
			}

			/* if afk, spawn some text */
			if ((avatar[i].old_x != avatar[i].x) ||(avatar[i].old_y != avatar[i].y)) {
				avatar[i].afk = AVATAR_AFK_TICK;
			} else {
				avatar[i].afk--;
				if (avatar[i].afk <= 0) {
					avatar[i].afk = AVATAR_AFK_TICK;
					avatar_Say(i, TEXT_AFK, 0);
				}
			}

		}
	}
	/* REACTION */

	/* 1) set the reaction according the collision_map */
	for(i = 0; i < GAME_MAX_PLAYER; ++i) {
		if (!avatar[i].enabled) continue;
		for(ii = i; ii < GAME_MAX_PLAYER; ++ii) {
			if (!avatar[ii].enabled) continue;
			if (i == ii) continue;
			if (collision_map_x[i][ii] || collision_map_x[ii][i]) {
/*				
				if (avatar[i].vx > 0) avatar[i].react_vx = -(AVATAR_MAX_X_VEL * AVATAR_COLLISION_FACTOR_X + avatar[ii].vx);
				if (avatar[i].vx < 0) avatar[i].react_vx =  AVATAR_MAX_X_VEL * AVATAR_COLLISION_FACTOR_X + avatar[ii].vx;


				if (avatar[i].vx > 0) avatar[ii].react_vx =  (AVATAR_MAX_X_VEL * AVATAR_COLLISION_FACTOR_X + avatar[i].vx);
				if (avatar[i].vx < 0) avatar[ii].react_vx = -(AVATAR_MAX_X_VEL * AVATAR_COLLISION_FACTOR_X - avatar[i].vx);	
*/
				avatar[ii].react_vx += avatar[i].vx  * AVATAR_COLLISION_FACTOR_X;
				avatar[i].react_vx  += avatar[ii].vx * AVATAR_COLLISION_FACTOR_X;

			}
			if (collision_map_y[i][ii] || collision_map_y[ii][i]) {
				avatar[i].react_vy  = avatar[ii].vy * AVATAR_COLLISION_FACTOR_Y;
				avatar[ii].react_vy = avatar[i].vy  * AVATAR_COLLISION_FACTOR_Y;
			}

		}
	}

	/* 2) reset the speed  */
	for(i = 0; i < GAME_MAX_PLAYER; ++i) {
		if (!avatar[i].enabled) continue;
		for(ii = i; ii < GAME_MAX_PLAYER; ++ii) {
			if (!avatar[ii].enabled) continue;
			if (i == ii) continue;
			if (collision_map_x[i][ii] || collision_map_x[ii][i]) {
				if (avatar[i].vx > 0) {
					avatar[i].vx = -AVATAR_COLLISION_RECOIL;
				} else if (avatar[i].vx < 0) {
					avatar[i].vx = AVATAR_COLLISION_RECOIL;
				}

				if (avatar[ii].vx > 0) {
					avatar[ii].vx = -AVATAR_COLLISION_RECOIL;
				} else if (avatar[ii].vx < 0)	{
					avatar[ii].vx = AVATAR_COLLISION_RECOIL;
				}

/*				avatar[ii].vx = AVATAR_COLLISION_RECOIL; */
			}

			if (collision_map_y[i][ii] || collision_map_y[ii][i]) {
				avatar[i].vy = avatar[ii].vy = 0;
			}
		}
	}

	/* 3) now apply the avatar<to>avatar collision speed to all player */
	for(i = 0; i < GAME_MAX_PLAYER; ++i) {
		if (!avatar[i].enabled) continue;
		if (avatar[i].isdying) continue;
		if ((avatar[i].state == AVATAR_LINKIN) || (avatar[i].state == AVATAR_LINKOUT)) continue;
/*
		if (fabs(avatar[i].react_vx) < AVATAR_COLLISION_MINVEL) avatar[i].react_vx = 0;
		if (fabs(avatar[i].react_vy) < AVATAR_COLLISION_MINVEL) avatar[i].react_vy = 0;
*/
		avatar[i].vx += avatar[i].react_vx;
		avatar[i].vy += avatar[i].react_vy;

		if (fabs(avatar[i].vx) > AVATAR_MAX_X_VEL) avatar[i].vx = SGN(avatar[i].vx) * AVATAR_MAX_X_VEL;
		if (fabs(avatar[i].vy) > AVATAR_MAX_Y_VEL) avatar[i].vy = SGN(avatar[i].vy) * AVATAR_MAX_Y_VEL;

		/* reaction always get "consumed" when used */
		avatar[i].react_vx = avatar[i].react_vy = 0;
	}

	/* keyframe */
	for(i = 0; i < GAME_MAX_PLAYER; ++i) {
		if (!avatar[i].enabled) continue;	
		if (!avatar[i].visible) continue;	

		switch (avatar[i].state) {
		case AVATAR_LINKIN:
		case AVATAR_LINKOUT:
		case AVATAR_DEAD:
		case AVATAR_STAY:
		case AVATAR_JUMP:
			/* no keyframe */
			avatar[i].kf = avatar[i].kf_age = 0;
			break;

		case AVATAR_RUN:
			if (avatar[i].vx != 0) {
				avatar[i].kf_age += fabs(avatar[i].x - avatar[i].old_x);
				if (fabs(avatar[i].kf_age) >= AVATAR_FRAME_SPEED) {
					avatar[i].kf_age = fabs(avatar[i].kf_age - AVATAR_FRAME_SPEED);
					avatar[i].kf++;
					avatar[i].kf %= 4;
					sound_SfxPlay(SND_STEP_01+ rand()%4);
				}
			} else {
				avatar[i].kf = avatar[i].kf_age = 0;
			}
			break;
		default:
			printf("Undefined avatar_state\n");
			exit(EXIT_FAILURE);
		}
	}
}

void avatar_Blit(void) 
{
	SDL_Rect srect, drect;
	int i, alfa;

	for(i = 0; i < GAME_MAX_PLAYER; ++i) {

		if (avatar[i].visible && avatar[i].enabled) {

			/* bound's check */
			if (avatar[i].x > camera_x + graph_RESX) continue;
			if (avatar[i].y > camera_y + graph_RESY) continue;

			if (avatar[i].x + avatar[i].sprite->w < camera_x) continue;
			if (avatar[i].y + avatar[i].sprite->h < camera_y) continue;

			srect.w = AVATAR_SIZE;
			srect.h = AVATAR_SIZE;

			drect.x = avatar[i].x - camera_x;
			drect.y = avatar[i].y - camera_y;

			/* unused */
			drect.w = avatar[i].sprite->w;
			drect.h = avatar[i].sprite->h;

			switch (avatar[i].state) {
			case AVATAR_LINKIN:

				switch (avatar[i].link->src_facing) {
				case LINK_UP:
					srect.x = 0;
					srect.w = AVATAR_SIZE;
					srect.h = ((float)avatar[i].linktimer / (float)AVATAR_LINKIN_TIMER) * AVATAR_SIZE;
					if (avatar[i].lookat == AVATAR_DX) {
						srect.y = 0;
					} else {
						srect.y = 3 * AVATAR_SIZE;
					}
					drect.y = avatar[i].y - camera_y + (AVATAR_SIZE - srect.h);
					break;

				case LINK_DOWN:
					srect.x = 0;
					srect.w = AVATAR_SIZE;
					srect.h = ((float)avatar[i].linktimer / (float)AVATAR_LINKIN_TIMER) * AVATAR_SIZE;
					if (avatar[i].lookat == AVATAR_DX) {
						srect.y = 0 + (AVATAR_SIZE - srect.h);
					} else {
						srect.y = 3 * AVATAR_SIZE + (AVATAR_SIZE - srect.h);
					}
					break;

				case LINK_LEFT:
					srect.x = 0;
					srect.w = ((float)avatar[i].linktimer / (float)AVATAR_LINKIN_TIMER) * AVATAR_SIZE;
					srect.h = AVATAR_SIZE;
					srect.y = 0;
					drect.x = avatar[i].x - camera_x + (AVATAR_SIZE-srect.w);
					break;

				case LINK_RIGHT:
					srect.w = ((float)avatar[i].linktimer / (float)AVATAR_LINKIN_TIMER) * AVATAR_SIZE;
					srect.x = AVATAR_SIZE - srect.w;
					srect.h = AVATAR_SIZE;
					srect.y = 3 * AVATAR_SIZE;
					break;

				case LINK_UNDEFINED:
					srect.x = 0;
					if (avatar[i].lookat == AVATAR_DX) {
						srect.y = 0;
					} else {
						srect.y = 3 * AVATAR_SIZE;
					}
					break;
				}
				break;

			case AVATAR_LINKOUT:
				switch (avatar[i].link->dst_facing) {
				case LINK_UP:
					srect.x = 0;
					srect.w = AVATAR_SIZE;
					srect.h = AVATAR_SIZE - ((float)avatar[i].linktimer / (float)AVATAR_LINKIN_TIMER) * AVATAR_SIZE;
					
					if (avatar[i].lookat == AVATAR_DX) {
						srect.y = 0;
					} else {
						srect.y = 3 * AVATAR_SIZE;
					}

					drect.y = avatar[i].y - camera_y + (AVATAR_SIZE - srect.h);
					break;

				case LINK_DOWN:
					srect.x = 0;
					srect.w = AVATAR_SIZE;
					srect.h = AVATAR_SIZE - ((float)avatar[i].linktimer / (float)AVATAR_LINKIN_TIMER) * AVATAR_SIZE;
					
					if (avatar[i].lookat == AVATAR_DX) {
						srect.y = 0 + (AVATAR_SIZE - srect.h);
					} else {
						srect.y = 3 * AVATAR_SIZE + (AVATAR_SIZE - srect.h);
					}
					break;
					
				case LINK_LEFT:
					srect.x = 0;
					srect.w = AVATAR_SIZE - ((float)avatar[i].linktimer / (float)AVATAR_LINKIN_TIMER) * AVATAR_SIZE;
					srect.h = AVATAR_SIZE;
					srect.y = 3 * AVATAR_SIZE;
					drect.x = avatar[i].x - camera_x + (AVATAR_SIZE-srect.w);
					break;
				
				case LINK_RIGHT:
					srect.w = AVATAR_SIZE - ((float)avatar[i].linktimer / (float)AVATAR_LINKIN_TIMER) * AVATAR_SIZE;
					srect.x = AVATAR_SIZE - srect.w;
					srect.h = AVATAR_SIZE;
					srect.y = 0;
					break;

				case LINK_UNDEFINED:
					srect.x = 0;
					if (avatar[i].lookat == AVATAR_DX) {
						srect.y = 0;
					} else {
						srect.y = 3 * AVATAR_SIZE;
					}
					break;
				}
				break;

			case AVATAR_STAY:
				srect.x = 0;
				if (avatar[i].lookat == AVATAR_DX) {
					srect.y = 0;
				} else {
					srect.y = 3 * AVATAR_SIZE;
				}
				break;

			case AVATAR_JUMP:
				srect.x = 0;
				if (avatar[i].lookat == AVATAR_DX) {
					srect.y = 2 * AVATAR_SIZE;
				} else {
					srect.y = 5 * AVATAR_SIZE;
				}
				break;

			case AVATAR_RUN:
				if (avatar[i].lookat == AVATAR_DX) {
					if (avatar[i].kf == 3) {
						srect.x = 1 * AVATAR_SIZE;
					} else {
						srect.x = avatar[i].kf * AVATAR_SIZE;
					}
					srect.y = 1 * AVATAR_SIZE;
				} else {
					if (avatar[i].kf == 3) {
						srect.x = 1 * AVATAR_SIZE;
					} else {
						srect.x = avatar[i].kf * AVATAR_SIZE;
					}
					srect.y = 4 * AVATAR_SIZE;
				}
				break;

			case AVATAR_DEAD:
				srect.x = 0;
				srect.y = 6 * AVATAR_SIZE;
				break;

			default:
				printf("Undefined avatar_state\n");
				exit(EXIT_FAILURE);
			}

			SDL_BlitSurface(avatar[i].sprite, &srect, graph_screen, &drect);

			/* hurry up player! */
			if (map.timer == map.timer_hurryup) {
				avatar_Say(i, TEXT_HURRYUP, 0);
			}

			/* text */ 
			if ((avatar[i].text_ttl <= AVATAR_FONT_FADEOUT) && (avatar[i].text_ttl > 0)) {
				/* fadeout */
				alfa = ((float)avatar[i].text_ttl / (float)AVATAR_FONT_FADEOUT) * 255;
			} else {
				if (avatar[i].text_fadein > 0) { 
					/* fadein */
					alfa = 255 - (avatar[i].text_fadein/(float)AVATAR_FONT_FADEIN * 255);
				} else {
					/* normal*/
					alfa = 255;
				}
			}

			/* if mario is link-state, disable the saytext */
			if ((avatar[i].state != AVATAR_LINKIN) && (avatar[i].state != AVATAR_LINKOUT)) {
				if (avatar[i].textbox) {
					drect.x = avatar[i].x - camera_x + AVATAR_SIZE/2.0 - avatar[i].textbox->w/2.0 + AVATAR_FONT_DELTAX;
					drect.y = avatar[i].y - camera_y - avatar[i].textbox->h + AVATAR_FONT_DELTAY;
					drect.w = drect.h = 0;
					SDL_SetAlpha(avatar[i].textbox, SDL_SRCALPHA, AVATAR_FONT_ALFA * (alfa/255.0));
					SDL_BlitSurface(avatar[i].textbox, NULL, graph_screen, &drect);
				}

				if (avatar[i].textbk) {
					drect.x = avatar[i].x - camera_x + AVATAR_SIZE/2.0 - avatar[i].textbox->w/2.0 + AVATAR_FONT_DELTAX + AVATAR_FONT_SHADOWX;
					drect.y = avatar[i].y - camera_y - avatar[i].textbox->h + AVATAR_FONT_DELTAY + AVATAR_FONT_SHADOWY;
					drect.w = drect.h = 0;
					SDL_SetAlpha(avatar[i].textbk, SDL_SRCALPHA, alfa);
					SDL_BlitSurface(avatar[i].textbk, NULL, graph_screen, &drect);
				}

				if (avatar[i].text) {
					drect.x = avatar[i].x - camera_x + AVATAR_SIZE/2.0 - avatar[i].textbox->w/2.0 + AVATAR_FONT_DELTAX;
					drect.y = avatar[i].y - camera_y - avatar[i].textbox->h + AVATAR_FONT_DELTAY;
					drect.w = drect.h = 0;
					SDL_SetAlpha(avatar[i].text, SDL_SRCALPHA, alfa);
					SDL_BlitSurface(avatar[i].text, NULL, graph_screen, &drect);
				}
			
			}
		}
	}
}

float avatar_Get_x(int player)
{
	return avatar[player].x;
}

float avatar_Get_y(int player)
{
	return avatar[player].y;
}

float avatar_Get_vx(int player)
{
	return avatar[player].vx;
}

float avatar_Get_vy(int player)
{
	return avatar[player].vy;
}

inline static void avatar_GetBBox(int player,long int *ax, long int *ay, long int *bx, long int *by) 
{
	*ax = (long int)(avatar[player].x + AVATAR_BBOXX);
	*ay = (long int)(avatar[player].y + AVATAR_BBOXY);

	*bx = *ax + AVATAR_BBOXW -1;
	*by = *ay + AVATAR_BBOXH -1;
}

inline static void avatar_GetBBoxLinkX(int player,long int *ax, long int *ay, long int *bx, long int *by) 
{
	*ax = (long int)(avatar[player].x + AVATAR_BBOXX);
	*ay = (long int)(avatar[player].y + AVATAR_BBOXY);

	*bx = *ax + AVATAR_BBOXW -1;
	*by = *ay + AVATAR_BBOXH -1;

	/* cut some part of bbox */
	*ax += AVATAR_LINK_REDUCTION;	
	*bx -= AVATAR_LINK_REDUCTION;	
}

inline static void avatar_GetBBoxLinkY(int player,long int *ax, long int *ay, long int *bx, long int *by) 
{
	*ax = (long int)(avatar[player].x + AVATAR_BBOXX);
	*ay = (long int)(avatar[player].y + AVATAR_BBOXY);

	*bx = *ax + AVATAR_BBOXW -1;
	*by = *ay + AVATAR_BBOXH -1;

	/* cut some part of bbox */
	*ay += AVATAR_LINK_REDUCTION;
	*by -= AVATAR_LINK_REDUCTION;
}

void avatar_Add(int i, char *name, const char *filename, enum input_device_t device, int life) 
{
	SDL_Surface *sprite;
	avatar[i].device  = device;
	avatar[i].enabled = SDL_TRUE;
	avatar[i].life    = (life == 0 ? AVATAR_DEFAULT_LIFE:life);
	avatar[i].name = strdup(name);
	
	/* try to open joypad */
	if ((device >= DEV_JOYPAD0) && (device < DEV_JOYMAX)) {
		printf("Binding joypad[%i]\n", device - DEV_JOYPAD0);	
		joy_Bind(device - DEV_JOYPAD0);
	}

	/* try to open wii */
	if ((device >= DEV_WIIMOTE0) && (device < DEV_WIIMAX)) {
#ifdef _WII_MOTE
		printf("Binding wiimote[%i]\n", device - DEV_WIIMOTE0);
		wii_Bind(device - DEV_WIIMOTE0);
#else
		printf("This release of smb, is compiled without the wiimote support. \n"
			 "Please recompile smb with the flag: -D_WII_MOTE\n");
		exit(EXIT_FAILURE);
#endif
	}

	printf("Adding avatar %i:  %s\n", i, filename);

	sprite = IMG_Load(filename);
	if (!sprite) {
		printf("Can't find avatar data: %s\n", SDL_GetError());
		exit(EXIT_FAILURE);
	}
	avatar[i].sprite  = SDL_DisplayFormatAlpha(sprite);
	SDL_FreeSurface(sprite);
	
}

void avatar_RespawnAll(void) 
{
	int i;
	for(i = 0; i < GAME_MAX_PLAYER; ++i) {
		avatar_Respawn(i);
	}
}

void avatar_Respawn(int i) 
{

	if (avatar[i].enabled) {
		avatar[i].visible = SDL_TRUE;
		avatar[i].easy_jump = 0;

		avatar[i].vx = avatar[i].vy = 0;
		avatar[i].react_vx = avatar[i].react_vy = 0;
		
		avatar[i].old_vx = avatar[i].old_vy = 0;

		avatar[i].x = avatar[i].old_x = map.respawn_x + (AVATAR_SIZE * i * 1.5);
		avatar[i].y = avatar[i].old_y = map.respawn_y;
		avatar[i].lookat = map.respawn_lookat;

		avatar[i].kf_age = avatar[i].kf = 0;

		avatar[i].state  = AVATAR_STAY;
		avatar[i].linktimer = 0;
		avatar[i].link = NULL;

		avatar_Say(i, avatar[i].name, 0);

		if (avatar_InGame() <= 1) {
			post_Clear();
			post_Set(POST_MOSAIC);
		}
	} else {
		avatar[i].visible = SDL_FALSE;
	}
}

inline void avatar_AddCoin(int player) 
{
	avatar[player].coin++;
	if (avatar[player].coin >= AVATAR_COINS_4_LIFE) {
		avatar[player].coin = 0;
		sound_SfxPlay(SND_LIFEUP);
		avatar_AddLife(player, 1);
	}
}

inline void avatar_AddPoint(int player, unsigned int points) 
{
	avatar[player].score += points;
}

inline void avatar_AddLife(int player, int life)
{
	avatar[player].life += life;
	avatar_Say(player, TEXT_LIFEUP, 0);
	if (avatar[player].life > 99) {
		avatar[player].life = 99;
	}
}

void avatar_SayAll(char *text) 
{
	int i;
	for(i = 0; i < GAME_MAX_PLAYER; ++i) {
		if (avatar[i].visible && avatar[i].enabled) {
			avatar_Say(i, text, 0);
		}
	}
}

void avatar_Say(int i, char *text, int ttl) 
{
	Uint32 rmask, gmask, bmask, amask;
	SDL_Surface *tmp;

	if (ttl <= 0) {
		avatar[i].text_ttl = AVATAR_FONT_LIFE;
	} else {
		avatar[i].text_ttl = ttl;
	}
	
	avatar[i].text_fadein = AVATAR_FONT_FADEIN;

	/* avoid memleak */
	if (avatar[i].text) {
		SDL_FreeSurface(avatar[i].text);
		avatar[i].text = NULL;
	}  
	if (avatar[i].textbk) {
		SDL_FreeSurface(avatar[i].textbk);
		avatar[i].textbk = NULL;
	}  
	
	if (avatar[i].textbox) {
		SDL_FreeSurface(avatar[i].textbox);
		avatar[i].textbox = NULL;
	}
	
	avatar[i].text   = TTF_RenderText_Solid(avatar_font, text, avatar_color_text);
	avatar[i].textbk = TTF_RenderText_Solid(avatar_font, text, avatar_color_textbk);

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
	rmask = 0xff000000;
	gmask = 0x00ff0000;
	bmask = 0x0000ff00;
	amask = 0x000000ff;
#else
	rmask = 0x000000ff;
	gmask = 0x0000ff00;
	bmask = 0x00ff0000;
	amask = 0xff000000;
#endif

	tmp = SDL_CreateRGBSurface(SDL_SWSURFACE, avatar[i].text->w + AVATAR_FONT_SHADOWX, avatar[i].text->h + AVATAR_FONT_SHADOWY, avatar[i].text->format->BitsPerPixel, rmask, gmask, bmask, amask);
	avatar[i].textbox = SDL_DisplayFormat(tmp);
	SDL_FreeSurface(tmp);

}

void avatar_Kill(int player) 
{
	if (avatar[player].enabled) {
		avatar[player].isdying = 0;
		if (avatar[player].life > 0) {
			avatar[player].life--;
		} else {
			avatar[player].enabled = SDL_FALSE;
		}
	}
}

void avatar_KillAll(void) 
{
	int i;
	for (i = 0; i < GAME_MAX_PLAYER; ++i) {
		avatar_Kill(i);
	}
}

int avatar_PlayerInGame(void)
{
	int i, ingame = 0;

	for (i = 0; i < GAME_MAX_PLAYER; ++i) {
		if (avatar[i].enabled) {
			ingame++;
		}
	}
	return ingame;
}

void avatar_StartToDie(int player) 
{
	if (avatar[player].enabled) {
		sound_SfxPlay(SND_DEAD);
		avatar_Say(player, TEXT_DEAD, 0);
		avatar[player].state   = AVATAR_DEAD;
		avatar[player].isdying = AVATAR_DEAD_TIME;
		avatar[player].react_vx = 0;
		avatar[player].react_vy = 0;
		post_Clear();
		post_Set(POST_BW);
	}
}

void avatar_StartToDieAll(void)
{
	int i;

	sound_SfxPlay(SND_DEAD);
	for (i = 0; i < GAME_MAX_PLAYER; ++i) {
		if (avatar[i].enabled) {
			avatar_Say(i, TEXT_DEAD, 0);
			avatar[i].state   = AVATAR_DEAD;
			avatar[i].isdying = AVATAR_DEAD_TIME;
			avatar[i].react_vx = 0;
			avatar[i].react_vy = 0;
			post_Clear();
			post_Set(POST_BW);
		}
	}
}


static void avatar_DeadTick(int player) 
{
	avatar[player].isdying--;
	if (avatar[player].isdying > AVATAR_DEAD_TIME/1.4) {
		if (avatar[player].y > 0) {
			avatar[player].y -= .8;
		}
	} else {
		avatar[player].y += 1.5;
	}	
}

SDL_bool avatar_SomeoneIsDying(void)
{
	int i;
	for (i = 0; i < GAME_MAX_PLAYER; ++i) {
		if (avatar[i].enabled) {
			if (avatar[i].isdying > 0) return SDL_TRUE;
		}
	}
	return SDL_FALSE;
}

SDL_bool avatar_SomeoneInLink(void) 
{
	int i;
	for (i = 0; i < GAME_MAX_PLAYER; ++i) {
		if (avatar[i].enabled) {
			if ((avatar[i].state == AVATAR_LINKIN) || (avatar[i].state == AVATAR_LINKOUT)) return SDL_TRUE;	
		}
	}
	return SDL_FALSE;
}

void avatar_LinkAll(struct link_t *link) 
{
	int i;
	for(i = 0; i < GAME_MAX_PLAYER; ++i) {
		avatar_Link(i, link);
	}

}

void avatar_Link(int i, struct link_t *link) 
{
	/* FIXME 
		in multiplayer: what i have to do whith player in dyning state?
	*/
	if ((avatar[i].enabled) && (!avatar[i].isdying)) {

		if (!avatar_SomeoneInLink()) {
			sound_SfxPlayTimed(SND_LINK, (AVATAR_LINKIN_TIMER + AVATAR_LINKOUT_TIMER) * GAME_TICK_LEN);
		}

		avatar[i].visible = SDL_TRUE;
		avatar[i].easy_jump = 0;
		avatar[i].vx = avatar[i].vy = 0;
		avatar[i].react_vx = avatar[i].react_vy = 0;
		avatar[i].old_vx = avatar[i].old_vy = 0;
		avatar[i].kf_age = avatar[i].kf = 0;

		avatar[i].state  = AVATAR_LINKIN;
		avatar[i].linktimer = AVATAR_LINKIN_TIMER;
		avatar[i].link = link;

	}

}

static void avatar_LinkTick(int player) 
{
	avatar[player].kf_age = avatar[player].kf = 0;
	avatar[player].visible = SDL_TRUE;
	avatar[player].easy_jump = 0;
	avatar[player].vx = avatar[player].old_vx = 0;
	avatar[player].vy = avatar[player].old_vy = 0;
	avatar[player].react_vx = avatar[player].react_vy = 0;

	/* 	FIXME: We must use the "real size" of the tile, _not_ TILE_SIZE!
		but i'm too lazy to fix this.
	*/	
	if (avatar[player].state == AVATAR_LINKIN) {
		if (avatar[player].linktimer > 0) {
			avatar[player].linktimer--;
			
			switch (avatar[player].link->src_facing) {
			case LINK_UP:
				avatar[player].x = avatar[player].old_x = avatar[player].link->src_tile_x * TILE_SIZE;
				avatar[player].y = avatar[player].old_y = avatar[player].link->src_tile_y * TILE_SIZE - TILE_SIZE;
				break;
			
			case LINK_DOWN:
				avatar[player].x = avatar[player].old_x = avatar[player].link->src_tile_x * TILE_SIZE;
				avatar[player].y = avatar[player].old_y = avatar[player].link->src_tile_y * TILE_SIZE + TILE_SIZE;
				break;
			
			case LINK_LEFT:
				avatar[player].lookat = AVATAR_DX;
				avatar[player].x = avatar[player].old_x = avatar[player].link->src_tile_x * TILE_SIZE - TILE_SIZE;
				avatar[player].y = avatar[player].old_y = avatar[player].link->src_tile_y * TILE_SIZE;
				break;

			case LINK_RIGHT:
				avatar[player].lookat = AVATAR_SX;	
				avatar[player].x = avatar[player].old_x = avatar[player].link->src_tile_x * TILE_SIZE + TILE_SIZE;
				avatar[player].y = avatar[player].old_y = avatar[player].link->src_tile_y * TILE_SIZE;
				break;
			
			case 	LINK_UNDEFINED:
				avatar[player].x = avatar[player].old_x = avatar[player].link->src_tile_x * TILE_SIZE;
				avatar[player].y = avatar[player].old_y = avatar[player].link->src_tile_y * TILE_SIZE;
				break;
			}

		} else {
			avatar[player].state     = AVATAR_LINKOUT;
			avatar[player].linktimer = AVATAR_LINKOUT_TIMER;
		}
		
	} else if (avatar[player].state == AVATAR_LINKOUT) {

		if (avatar[player].linktimer > 0) {
			avatar[player].linktimer--;

			switch (avatar[player].link->dst_facing) {
			case LINK_UP:
				avatar[player].x = avatar[player].old_x = avatar[player].link->dst_tile_x * TILE_SIZE;
				avatar[player].y = avatar[player].old_y = avatar[player].link->dst_tile_y * TILE_SIZE - TILE_SIZE;
				break;

			case LINK_DOWN:
				avatar[player].x = avatar[player].old_x = avatar[player].link->dst_tile_x * TILE_SIZE;
				avatar[player].y = avatar[player].old_y = avatar[player].link->dst_tile_y * TILE_SIZE + TILE_SIZE;
				break;

			case LINK_LEFT:
				avatar[player].lookat = AVATAR_SX;
				avatar[player].x = avatar[player].old_x = avatar[player].link->dst_tile_x * TILE_SIZE - TILE_SIZE;
				avatar[player].y = avatar[player].old_y = avatar[player].link->dst_tile_y * TILE_SIZE;
				break;

			case LINK_RIGHT:
				avatar[player].lookat = AVATAR_DX;
				avatar[player].x = avatar[player].old_x = avatar[player].link->dst_tile_x * TILE_SIZE + TILE_SIZE;
				avatar[player].y = avatar[player].old_y = avatar[player].link->dst_tile_y * TILE_SIZE;
				break;

			case 	LINK_UNDEFINED:
				avatar[player].x = avatar[player].old_x = avatar[player].link->dst_tile_x * TILE_SIZE;
				avatar[player].y = avatar[player].old_y = avatar[player].link->dst_tile_y * TILE_SIZE;
				break;
			}

		} else {
			avatar[player].state     = AVATAR_STAY;
			avatar[player].linktimer = 0;
			avatar[player].link = NULL;
			avatar_Say(player, avatar[player].name, 0);
		}
		/* must move the camera to look at the new position */
		camera_Look();
	}
}

int avatar_InGame(void)
{
	int i, ingame = 0;

	for(i = 0; i < GAME_MAX_PLAYER; ++i) {
		if (avatar[i].enabled) {
			ingame++;
		}
	}
	return ingame;
}

static SDL_bool avatar_XCollisionAvatar2AvatarExt(int a, SDL_bool avatar_flag[])
{
	int b;
	SDL_bool collision = SDL_FALSE;
	
	assert (a < GAME_MAX_PLAYER);
	
	if (!avatar[a].enabled) return SDL_FALSE;
	if (avatar[a].isdying) return SDL_FALSE;
	if ((avatar[a].state == AVATAR_LINKIN) || (avatar[a].state == AVATAR_LINKOUT)) return SDL_FALSE;

	for(b = 0; b < GAME_MAX_PLAYER; ++b) {
		avatar_flag[b] = SDL_FALSE;
		if (a == b) continue;
		if (avatar_TestCollisionAvatar2Avatar(a, b)) {
			avatar_flag[b] = SDL_TRUE;
			collision = SDL_TRUE;
		}
	}

	return collision;
}

static SDL_bool avatar_YCollisionAvatar2AvatarExt(int a, SDL_bool avatar_flag[])
{
	int b;
	SDL_bool collision = SDL_FALSE;
	
	assert (a < GAME_MAX_PLAYER);

	if (!avatar[a].enabled) return SDL_FALSE;
	if (avatar[a].isdying) return SDL_FALSE;
	if ((avatar[a].state == AVATAR_LINKIN) || (avatar[a].state == AVATAR_LINKOUT)) return SDL_FALSE;

	for(b = 0; b < GAME_MAX_PLAYER; ++b) {
		avatar_flag[b] = SDL_FALSE;
		if (a == b) continue;
		if (avatar_TestCollisionAvatar2Avatar(a, b)) {
			avatar_flag[b] = SDL_TRUE;
			collision = SDL_TRUE;
		}
	}
	return collision;

}
static SDL_bool avatar_TestCollisionAvatar2Avatar(int a, int b)
{
	long int aax, aay, abx, aby;
	long int bax, bay, bbx, bby;

	if ((!avatar[a].enabled) || (avatar[a].isdying) || (!avatar[a].visible)) return SDL_FALSE;
	if ((!avatar[b].enabled) || (avatar[b].isdying) || (!avatar[b].visible)) return SDL_FALSE;
	if ((avatar[a].state == AVATAR_LINKIN) || (avatar[a].state == AVATAR_LINKOUT)) return SDL_FALSE;
	if ((avatar[b].state == AVATAR_LINKIN) || (avatar[b].state == AVATAR_LINKOUT)) return SDL_FALSE;

	avatar_GetBBox(a, &aax, &aay, &abx, &aby);
	avatar_GetBBox(b, &bax, &bay, &bbx, &bby);

	if (Collide(aax, aay, abx, aby, bax, bay, bbx, bby)) {
		return SDL_TRUE;
	} else {
		return SDL_FALSE;
	}
}


static SDL_bool avatar_CanWalkOn(int a)
{
	int  b;

	SDL_bool canwalk = SDL_FALSE;

	if (!avatar[a].enabled) return SDL_FALSE;
	if (avatar[a].isdying) return SDL_FALSE;
	if ((avatar[a].state == AVATAR_LINKIN) || (avatar[a].state == AVATAR_LINKOUT)) return SDL_FALSE;

	for( b = 0; b < GAME_MAX_PLAYER; ++b) {
		if (a == b) continue;
		
		if (avatar_TestCollisionAvatar2Avatar(a, b)) {
			/* can't walk, we are inside a player*/
			return SDL_FALSE;
		} else {
			/* step down */
			avatar[a].y++;
			if (avatar_TestCollisionAvatar2Avatar(a, b)) {
				canwalk = SDL_TRUE;
			}
			/* restore */
			avatar[a].y--;
		}
	}
	return canwalk;

}


