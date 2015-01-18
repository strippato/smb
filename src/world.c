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

#include "global.h"
#include "graph.h"
#include "input.h"
#include "game.h"
#include "link.h"
#include "avatar.h"
#include "map.h"
#include "hud.h"
#include "sound.h"
#include "world.h"
#include "camera.h"
#include "cheat.h"
#include "particle.h"
#include "post.h"

static Uint32 world_last_tick		= 0;
static Uint32 world_start_tick	= 0;
static Uint32 world_frame_counter	= 0;
static SDL_bool world_updated		= SDL_FALSE;

config_t *world_config;
config_t world_config_cfg;

void world_Init(char *WorldName) 
{
	char *fullname;
	printf("Loading world: %s\n", WorldName);

	fullname = malloc(strlen(WORLD_DIR) + strlen(WorldName) +1);
	strcpy(fullname, WORLD_DIR);
	strcat(fullname, WorldName);

	world_config = &world_config_cfg;
	config_init(world_config);

	if (!config_read_file(world_config, fullname)) {
		printf("Config error in file: %s\n", fullname);
		config_destroy(world_config);
		exit(EXIT_FAILURE);
	}

	free(fullname);

	map_Init();/* map */

	avatar_RespawnAll();
	
	camera_Init();/* camera */
	cheat_Init(); /* cheats */

	world_start_tick = SDL_GetTicks()/GAME_TICK_LEN;
	world_last_tick  = world_start_tick;
	world_frame_counter = 0;

	sound_MusicLoad();
	particle_Clear();
	post_Clear();
	post_Set(POST_MOSAIC);
}

void world_Free(void) 
{
	post_Clear();
	input_Clear();
	map_Free();   /* map    */
	camera_Free();/* camera */

	cheat_Free(); /* cheats */
	sound_MusicUnload();
	
	world_start_tick = 0;
	world_last_tick  = 0;
	world_frame_counter = 0;

	config_destroy(world_config);
}

void world_Tick(void) 
{
		
	++world_last_tick;
	input_Tick();	
	hud_Tick(map.timer/100);
	map_Tick();
	camera_Tick();
	avatar_Tick();
	cheat_Tick();
	particle_Tick();
	sound_Tick();
	post_Tick();
	world_updated = SDL_TRUE;
}

void world_Blit(void) 
{
	
	++world_frame_counter;
	map_Blit();
	avatar_Blit();
	hud_Blit(); 
	cheat_Blit();
	particle_Blit();
	post_Blit();
	
	graph_Flip();
	world_updated = SDL_FALSE;

	/* show FPS */
/*
	static Uint32 last_print = 0;
	if ((SDL_GetTicks() - last_print) >= 1000) {
		printf("\n FPS %u", (world_frame_counter * 1000) / (SDL_GetTicks() - world_start_ticks * GAME_TICK_LEN)); 
		last_print = SDL_GetTicks();
	}
*/

}

void world_Run(void) 
{
	
	world_updated = SDL_FALSE;	
	
	sound_MusicPlay(SND_NORMAL_MUSIC);	
	
	while (!(game_game_state & (GAME_QUIT | GAME_LEVEL_COMPLETED | GAME_LEVEL_TIMEOUT))) { 
		while ((world_last_tick + 1) * GAME_TICK_LEN <= SDL_GetTicks()) {

			if (input_key_system[SDLK_ESCAPE]) game_game_state |= GAME_QUIT;

/* dbugme
			if ((input_key_system[SDLK_F10]) && (!input_key_system_old[SDLK_F10])) avatar_StartToDieAll() ;
*/
			
			if ((input_key_system[SDLK_F12]) && (!input_key_system_old[SDLK_F12])) {
				graph_ToggleFullScreen();
			}

			if ((input_key_system[SDLK_F11]) && (!input_key_system_old[SDLK_F11])) {
				graph_ScreenShot();
			}

			world_Tick();

			if (avatar_PlayerInGame() == 0) game_game_state = GAME_QUIT;
			
		}

		if (world_updated) {
			world_Blit();
		} else {
			SDL_Delay (4);
		}
	}

	sound_MusicPlay(SND_NO_MUSIC);
	sound_Stop();
}

