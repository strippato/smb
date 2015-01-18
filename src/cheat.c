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
#include <SDL.h>
#include <SDL_mixer.h>
#include <libconfig.h>

#include "global.h"
#include "app.h"
#include "cheat.h"
#include "input.h"
#include "graph.h"
#include "tile.h"
#include "camera.h"
#include "game.h"
#include "link.h"
#include "avatar.h"
#include "map.h"
#include "util.h"
#include "text.h"

#define CHEAT_CAPTION	"_,.-=° Cheat mode°=-.,_"

static SDL_bool cheat_enabled  = SDL_FALSE;
static SDL_bool cheat_cheating = SDL_FALSE;
static int cheat_idx_tile = 0;

void cheat_Init(void)
{
	int i;
	if (config_lookup_bool(app_config, "game.cheat", &i)) { 
		if (i) {
			cheat_enabled = SDL_TRUE;
		} else {
			cheat_enabled = SDL_FALSE;
		}
	}
	cheat_cheating = SDL_FALSE;	
	cheat_idx_tile = 0;
}

void cheat_Free(void)
{
	cheat_enabled  = SDL_FALSE;
	cheat_cheating = SDL_FALSE;
	cheat_idx_tile = 0;
}

void cheat_Blit(void)
{
	long int x,y;
	SDL_Rect rec;
	
	if (!cheat_enabled)  return;
	if (!cheat_cheating) return;
	
	x = (camera_x + input_mouse_x) / TILE_SIZE;
	y = (camera_y + input_mouse_y) / TILE_SIZE;

	rec.h = 0; rec.w = 0;
	rec.x = x * TILE_SIZE - camera_x;
	rec.y = y * TILE_SIZE - camera_y;

	if (cheat_idx_tile) SDL_BlitSurface(tile_tile[cheat_idx_tile-1].Tile, NULL, graph_screen, &rec);

}

void cheat_Tick(void)
{
	long int x,y;
	int i;	
	SDL_bool spawntile;

	if (!cheat_enabled) return;
	
	if (input_key_system[SDLK_TAB]) {
		if (!cheat_cheating) { 	
			cheat_idx_tile = 0;
			cheat_cheating = SDL_TRUE;
			SDL_ShowCursor (SDL_ENABLE);
			SDL_WM_SetCaption (CHEAT_CAPTION, TITLE);
			avatar_SayAll(TEXT_SOMEONEISCHEATING);
		}
	} else {
		if (cheat_cheating) { 
			cheat_idx_tile = 0;
			cheat_cheating = SDL_FALSE;
			SDL_ShowCursor (SDL_DISABLE);
			SDL_WM_SetCaption (TITLE, TITLE);
		}
	}
	if (cheat_cheating) {
		if ((input_mouse_btn_r) && (!input_mouse_btn_r_old)) {
			cheat_idx_tile++;
			cheat_idx_tile = LIM(cheat_idx_tile, tile_MaxTile());
		}
			
		if (input_mouse_btn_l) {
			spawntile = SDL_TRUE;

			x = (camera_x + input_mouse_x) / TILE_SIZE;
			y = (camera_y + input_mouse_y) / TILE_SIZE;

			/* don't spawn in avatar space */ 
			for (i = 0; i < game_player; ++i) {
				if (avatar[i].enabled) {
					if (Collide(avatar[i].x, avatar[i].y, avatar[i].x + AVATAR_SIZE -1 , avatar[i].y + AVATAR_SIZE -1, x * TILE_SIZE, y * TILE_SIZE, x * TILE_SIZE + TILE_SIZE -1, y * TILE_SIZE + TILE_SIZE -1)) {
						avatar_Say(i , TEXT_CANTPLACEHERE, 0);
						spawntile = SDL_FALSE;
					}
				}
			}
			
			if (spawntile) {
				map_SetMap(x, y, cheat_idx_tile);
			}
		}
	}

}

