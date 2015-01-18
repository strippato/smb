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
#include <SDL_mixer.h>
#include <libconfig.h>

#include "global.h"
#include "camera.h"
#include "input.h"
#include "graph.h"
#include "tile.h"
#include "game.h"
#include "link.h"
#include "avatar.h"
#include "map.h"
#include "util.h"


#define CAMERA_RANGE_X    35
#define CAMERA_RANGE_Y    50
#define CAMERA_MAX_SPEEDX 3.5f
#define CAMERA_MAX_SPEEDY 5.0f

#define CAMERA_STOP_SPEED	0.015f

#define CAMERA_SPEED_FACTORX	0.00015f
#define CAMERA_SPEED_FACTORY	0.00020f

long int camera_x = 0;
long int camera_y = 0;

static long int camera_old_x = 0;
static long int camera_old_y = 0;

static float camera_vx = 0.0;
static float camera_vy = 0.0;

static enum camera_look_at_t camera_look_at = CAMERA_PLAYER_CENTER;

void camera_Init(void) 
{
	float virtual_player_x,  virtual_player_y;
	float playercount;	
	int i;

	virtual_player_x = 0;
	virtual_player_y = 0;
	playercount = 0;
	for (i = 0; i < GAME_MAX_PLAYER; ++i) {
		if (avatar[i].enabled) {
			playercount++;
			virtual_player_x += avatar_Get_x(i);
			virtual_player_y += avatar_Get_y(i);
		}
	}
	virtual_player_x = virtual_player_x/playercount;
	virtual_player_y = virtual_player_y/playercount;

	camera_x = posval(virtual_player_x + AVATAR_SIZE/2 - graph_RESX/2);
	camera_x = MIN(camera_x, map.size_w * TILE_SIZE);

	camera_y = posval(virtual_player_y + AVATAR_SIZE/2 - graph_RESY/2);
	camera_y = MIN(camera_y, map.size_h * TILE_SIZE);

	camera_old_x = camera_x;
	camera_old_y = camera_y;

	camera_vx = 0.0;
	camera_vy = 0.0;

	camera_look_at = CAMERA_PLAYER_CENTER;
}

void camera_Free(void) 
{
	camera_x = 0;
	camera_y = 0;	
	camera_old_x = 0;
	camera_old_y = 0;
	camera_vx = 0.0;
	camera_vy = 0.0;	
	camera_look_at = CAMERA_PLAYER_CENTER;	
}

void camera_Tick(void) 
{
	long int x, y;
	long int center_x, center_y;
	long int range_x, range_y;
	
	float virtual_player_x,  virtual_player_y;
	float playercount;	

	int i;

	camera_old_x = camera_x;
	camera_old_y = camera_y;

	switch (camera_look_at) {
		case CAMERA_DEBUG:
			if (input_key[SDLK_RIGHT]) camera_x++;
			if (input_key[SDLK_LEFT])  camera_x--;		

			if (input_key[SDLK_DOWN])  camera_y++;
			if (input_key[SDLK_UP])    camera_y--;
			break;


		case CAMERA_PLAYER_CENTER:
			virtual_player_x = virtual_player_y = 0;
			playercount = 0;
			
			for (i = 0; i < GAME_MAX_PLAYER; ++i) {
				if (avatar[i].enabled) {
					playercount++;
					virtual_player_x += avatar_Get_x(i);
					virtual_player_y += avatar_Get_y(i);					
				}
			}
			virtual_player_x = virtual_player_x/playercount;
			virtual_player_y = virtual_player_y/playercount;

			x = virtual_player_x + AVATAR_SIZE/2;
			y = virtual_player_y + AVATAR_SIZE/2;
			
			center_x = camera_x + graph_RESX/2;
			center_y = camera_y + graph_RESY/2;
			
			range_x = ((graph_RESX/2.0) * (CAMERA_RANGE_X/100.0));
			range_y = ((graph_RESY/2.0) * (CAMERA_RANGE_Y/100.0));
			
			if (fabs(center_x - x) > range_x) {
				if (center_x < x) {
					camera_vx += fabs(center_x - x) * CAMERA_SPEED_FACTORX;
				} else {
					camera_vx -= fabs(center_x - x) * CAMERA_SPEED_FACTORX;
				}
			} else {
				if (camera_vx > 0) {
					camera_vx -= CAMERA_STOP_SPEED; 
					if (camera_vx < 0 ) camera_vx = 0;							
				}
				if (camera_vx < 0) {
					camera_vx += CAMERA_STOP_SPEED; 
					if (camera_vx > 0 ) camera_vx = 0;							
				}
			}

			if (fabs(center_y - y) > range_y) {
				if (center_y < y) {
					camera_vy += fabs(center_y - y) * CAMERA_SPEED_FACTORY;
				} else {
					camera_vy -= fabs(center_y - y) * CAMERA_SPEED_FACTORY;
				}
			} else {
				if (center_y > 0) {
					camera_vy -= CAMERA_STOP_SPEED; 
					if (camera_vy < 0 ) camera_vy = 0;							
				}
				if (camera_vy < 0) {
					camera_vy += CAMERA_STOP_SPEED; 
					if (camera_vy > 0 ) camera_vy = 0;							
				}
			}

			camera_vx = (fabs(camera_vx) <= CAMERA_MAX_SPEEDX ? camera_vx : SGN(camera_vx) * CAMERA_MAX_SPEEDX);
			camera_vy = (fabs(camera_vy) <= CAMERA_MAX_SPEEDY ? camera_vy : SGN(camera_vy) * CAMERA_MAX_SPEEDY);
			
			camera_x += camera_vx;
			camera_y += camera_vy;									
			break;

		default:
			break;
	}

	/* outside map */
	if (camera_x < 0) camera_x = 0;
	if (camera_y < 0) camera_y = 0;
	
	if (camera_x > map.size_w * TILE_SIZE - graph_RESX) camera_x = map.size_w * TILE_SIZE - graph_RESX;
	if (camera_y > map.size_h * TILE_SIZE - graph_RESY) camera_y = map.size_h * TILE_SIZE - graph_RESY;
	
}

void camera_LookAt(enum camera_look_at_t camera_actor) 
{
	camera_look_at = camera_actor;
}


void camera_Look(void) 
{
	float virtual_player_x,  virtual_player_y;
	float playercount;
	int i;

	virtual_player_x = 0;
	virtual_player_y = 0;
	playercount = 0;
	for (i = 0; i < GAME_MAX_PLAYER; ++i) {
		if (avatar[i].enabled) {
			playercount++;
			virtual_player_x += avatar_Get_x(i);
			virtual_player_y += avatar_Get_y(i);
		}
	}
	virtual_player_x = virtual_player_x/playercount;
	virtual_player_y = virtual_player_y/playercount;

	camera_x = posval(virtual_player_x + AVATAR_SIZE/2 - graph_RESX/2);
	camera_x = MIN(camera_x, map.size_w * TILE_SIZE);

	camera_y = posval(virtual_player_y + AVATAR_SIZE/2 - graph_RESY/2);
	camera_y = MIN(camera_y, map.size_h * TILE_SIZE);

	camera_old_x = camera_x;
	camera_old_y = camera_y;

	camera_vx = 0.0;
	camera_vy = 0.0;
	
	/* don't go outside map */
	if (camera_x < 0) camera_x = 0;
	if (camera_y < 0) camera_y = 0;
	
	if (camera_x > map.size_w * TILE_SIZE - graph_RESX) camera_x = map.size_w * TILE_SIZE - graph_RESX;
	if (camera_y > map.size_h * TILE_SIZE - graph_RESY) camera_y = map.size_h * TILE_SIZE - graph_RESY;
	
}

