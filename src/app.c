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
#include <time.h>
#include <assert.h>

#ifdef _WII_MOTE
#include <wiimote_api.h>
#endif

#include "global.h"
#include "app.h"
#include "input.h"
#include "game.h"
#include "link.h"
#include "avatar.h"
#include "graph.h"
#include "sound.h"
#include "outro.h"
#include "wii.h"

#define APP_CONFIG	"smb.cfg"

config_t *app_config;
static config_t app_config_cfg;

void app_Init(void)
{
	srand (time(NULL));

	app_config = &app_config_cfg;
	config_init(app_config);

	if (!config_read_file(app_config, APP_CONFIG)) {
		printf("Config error in file: %s\n", APP_CONFIG);
		exit(EXIT_FAILURE);
	}

	graph_Init();
	sound_Init();
#ifdef _WII_MOTE
	wii_Init();
#endif
}

void app_Free(void)
{
#ifdef _WII_MOTE
	wii_Free();
#endif
	sound_Free();
	graph_Free();
	config_destroy(app_config);
}

void app_Run(void)
{
	int  mapidx     = 0;
	char *mapname[] =	{"map_01.cfg",
				 "map_02.cfg",
				 "map_03.cfg",
				 "map_04.cfg",
				 "map_05.cfg",
				 NULL };

	game_Init();

	game_AddPlayer("Mario"   , AVATAR_SPRITE0, DEV_KEYBOARD0, 0);

/*	game_AddPlayer("Magnus"  , AVATAR_SPRITE1, DEV_KEYBOARD1, 0);*/
/*	game_AddPlayer("Gallo"   , AVATAR_SPRITE2, DEV_KEYBOARD2, 0);*/
/*	game_AddPlayer("Pedro"   , AVATAR_SPRITE3, DEV_KEYBOARD3, 0);*/
/*	game_AddPlayer("Teo"     , AVATAR_SPRITE0, DEV_KEYBOARD4, 0);*/

/*	game_AddPlayer("Baldo"   , AVATAR_SPRITE0, DEV_JOYPAD0, 0);*/
/*	game_AddPlayer("Mirco"   , AVATAR_SPRITE1, DEV_JOYPAD1, 0);*/
/*	game_AddPlayer("Gigi"    , AVATAR_SPRITE2, DEV_JOYPAD2, 0);*/
/*	game_AddPlayer("Giowile" , AVATAR_SPRITE3, DEV_JOYPAD3, 0);*/
/*	game_AddPlayer("Markuz"  , AVATAR_SPRITE3, DEV_JOYPAD4, 0);*/

/*	game_AddPlayer("Gallo"  , AVATAR_SPRITE3, DEV_WIIMOTE0, 0);*/

	/* GO! */
	while (mapname[mapidx]) {
		game_Run(mapname[mapidx]);

		/* NEXT LEVEL? */
		if (game_game_state & GAME_LEVEL_COMPLETED) ++mapidx;

		/* QUIT? */
		if (game_game_state & GAME_QUIT) break;

	}

	game_Free();

	outro_Run();
}


