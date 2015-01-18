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
#include <SDL.h>
#include <SDL_mixer.h>
#include <libconfig.h>
#include <assert.h>

#include "global.h"
#include "graph.h"
#include "app.h"
#include "world.h"
#include "input.h"
#include "game.h"
#include "link.h"
#include "avatar.h"
#include "hud.h"
#include "sound.h"
#include "particle.h"
#include "post.h"

#define GAME_FADE2BLACK_TIME 350

int game_easy_jmp;
unsigned short int game_player;
enum game_game_state_t game_game_state;

void game_Init(void) 
{
	int jmp;
	
	game_game_state  = GAME_EMPTY;
	game_player      = 0;

	if (config_lookup_int(app_config, "game.easy_jump", &jmp)) { 
		game_easy_jmp = jmp;
	} else {
		game_easy_jmp = 0;	
	}
	avatar_Init();
	hud_Init();
	input_Init();
	particle_Init();		
	post_Init();
}

void game_AddPlayer(char *name, const char *filename, enum input_device_t device, int lives) 
{
	assert (game_player < GAME_MAX_PLAYER);

	avatar_Add(game_player, name, filename, device, lives);
	hud_Add(game_player);
	
	game_player++;

}

void game_Free(void) 
{
	game_game_state = GAME_EMPTY;
	game_easy_jmp = 0;
	game_player   = 0;
	input_Free();	
	hud_Free(); 	
	avatar_Free();
	particle_Free();	
	post_Free();	
}

void game_Run(char *game_map) 
{
	assert(game_map);

	printf("Starting level %s\n", game_map);
	game_game_state = GAME_EMPTY;
	world_Init(game_map);
	world_Run();
	world_Free();
	graph_Fade2Black(GAME_FADE2BLACK_TIME);	
}


	

