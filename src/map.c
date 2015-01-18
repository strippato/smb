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
#include <string.h>
#include <math.h>
#include <SDL.h>
#include <SDL_mixer.h>
#include <SDL_image.h>
#include <assert.h>
#include <libconfig.h>

#include "global.h"
#include "graph.h"
#include "tile.h"
#include "input.h"
#include "game.h"
#include "sound.h"
#include "world.h"
#include "camera.h"
#include "link.h"
#include "avatar.h"
#include "map.h"
#include "util.h"
#include "sound.h"
#include "particle.h"


#define MAP_HIT_SIZE   10
#define MAP_HIT_TIME   25

#define IS_SOLID(X) 		(!IS_NOT_SOLID(X))
#define IS_NOT_SOLID(X)	((X) == TILE_COIN || (X) == TILE_MORTAL ? SDL_TRUE:SDL_FALSE)

#define IS_LINK(X)		((X) == LINNK_UP || (X) == LINNK_DOWN || (X) == LINNK_LEFT || (X) == LINNK_RIGHT ? SDL_TRUE:SDL_FALSE)
#define IS_NOT_LINK(X) 	(!IS_LINK(X))

struct map_t map;

struct map_update_t {
	unsigned int x;
	unsigned int y;	
	
	struct map_update_t *next;
	struct map_update_t *prev;	
};

static struct map_update_t *map_update_top    = NULL;
static struct map_update_t *map_update_bottom = NULL;

/* forward  */
static void map_Load(void);

static void map_UpdateAdd(unsigned int x, unsigned int y);
static void map_UpdateClearAll(void);
static void map_UpdateClear(struct map_update_t *map_update);
static SDL_bool map_Updating(unsigned int x, unsigned int y);

void map_Init(void) 
{

	/* avoid mem leak */
	assert(!map_update_top);
	assert(!map_update_bottom);	
	
	map_UpdateClearAll();
	map_update_top    = NULL;
	map_update_bottom = NULL;
	
	map.level       = NULL;
	map.level_fg    = NULL;
	map.level_hit   = NULL;
	map.level_hitby = NULL;
	map.size_w = 0;
	map.size_h = 0;
	map.bkground_R = 0;
	map.bkground_G = 0;
	map.bkground_B = 0;
	map.bkground = NULL;
	map.bkground_speed  = 0;
	map.bkground_anchor = SW;
	
	map.timer = 0; 
	map.timer_hurryup = 0; 
	map.timer_init = 0;
		
	tile_Init();
	link_Init();
	
	map_Load();
}

void map_Free(void) 
{
	int i;
	for(i=0; i < map.size_w ; ++i) {
		free(map.level[i]);
		free(map.level_fg[i]);		
		free(map.level_hit[i]);		
		free(map.level_hitby[i]);		
	}
	free(map.level);
	map.level = NULL;

	free(map.level_fg);
	map.level_fg = NULL;

	free(map.level_hit);
	map.level_hit = NULL;

	free(map.level_hitby);
	map.level_hitby = NULL;

	map.size_w = 0;
	map.size_h = 0;	
	map.bkground_R = 0;
	map.bkground_G = 0;
	map.bkground_B = 0;
	map.bkground_speed  = 0;
	map.bkground_anchor = SW;	
	map.timer = 0; 
	map.timer_hurryup = 0; 
	map.timer_init = 0;	

	if (map.bkground) SDL_FreeSurface(map.bkground);
	map.bkground = NULL;

	map_UpdateClearAll();		

	link_Free();	
	tile_Free();
}

static void map_Load(void) 
{
	const char *data = NULL;
	config_setting_t *setting;
	int bbx, bby, bbw, bbh;	
	int xpos, ypos;
	int i, ii, tick, elem;
	int srcx, srcy, dstx, dsty;	
	int bool;
	int hitpoint;
	char *fullname;
	char *sfxhitname;	
	enum tile_class_t class;
	enum link_facing_to_t srcfacing, dstfacing;
	SDL_bool multilink;
	SDL_Surface *TmpSrf;

	/* author */
	if (config_lookup_string(world_config, "map.author.nick", &data)) { 
		printf("Author Nick: %s\n", data);
	}
	if (config_lookup_string(world_config, "map.author.name", &data)) { 
		printf("Author Name: %s\n", data);
	}

	/* timer */
	if (config_lookup_int(world_config, "map.timer", &i)) { 
		map.timer = i * GAME_TICK_LEN;
	} else {
		map.timer = 4000 * GAME_TICK_LEN; /* default 400 sec	*/
	}
	map.timer_init = map.timer;
	
	if (config_lookup_int(world_config, "map.timer_hurryup", &i)) { 
		map.timer_hurryup = i * GAME_TICK_LEN;
	} else {
		map.timer_hurryup = map.timer / 4; /* default	*/
	}
	
	/* map size */
	if (config_lookup_int(world_config, "map.size.w", &i)) { 
		map.size_w = i;
	}
	if (config_lookup_int(world_config, "map.size.h", &i)) { 
		map.size_h = i;
	}
	printf("Map size   : %i x %i\n", map.size_w, map.size_h);

	assert(map.size_w > 0); /* out of map */
	assert(map.size_h > 0); /* out of map */

	map.level = (unsigned char**)calloc(map.size_w, sizeof(unsigned char*));
	for(i=0; i < map.size_w ; ++i) {
		map.level[i] = (unsigned char*)calloc(map.size_h, sizeof(unsigned char));
	}
	assert(map.level); /* out of map */

	map.level_fg = (unsigned char**)calloc(map.size_w, sizeof(unsigned char*));
	for(i=0; i < map.size_w ; ++i) {
		map.level_fg[i] = (unsigned char*)calloc(map.size_h, sizeof(unsigned char));
	}
	assert(map.level_fg); /* out of map */

	map.level_hitby = (short int**)calloc(map.size_w, sizeof(short int*));
	for(i=0; i < map.size_w ; ++i) {
		map.level_hitby[i] = (short int*)calloc(map.size_h, sizeof(short int));
		for(ii=0; ii < map.size_h ; ++ii) {
			map.level_hitby[i][ii] = -1;
		}
	}
	assert(map.level_hitby); /* out of map */

	/* background color */
	if (config_lookup_int(world_config, "map.background.r", &i)) { 
		map.bkground_R = i;
	}
	if (config_lookup_int(world_config, "map.background.g", &i)) { 
		map.bkground_G = i;
	}
	if (config_lookup_int(world_config, "map.background.b", &i)) { 
		map.bkground_B = i;
	}
			
	/* background image */
	if (config_lookup_string(world_config, "map.background.map", &data)) {
		fullname = malloc(strlen(WORLD_GFX_DIR) + strlen(data) +1);
		strcpy(fullname, WORLD_GFX_DIR);	
		strcat(fullname, data);		
	
		TmpSrf = IMG_Load(fullname);
		if (!TmpSrf) {
			printf("Can not find tile data: %s\n", SDL_GetError());
			exit(EXIT_FAILURE);
		}
		map.bkground = SDL_DisplayFormatAlpha(TmpSrf);
		SDL_FreeSurface(TmpSrf);
		free(fullname);
	}
	assert(map.bkground);

	/* background speed */	
	if (config_lookup_int(world_config, "map.background.speed", &i)) { 
		map.bkground_speed = i;
	}
			
	/* background anchor */
	if (config_lookup_string(world_config, "map.background.anchor", &data)) { 
		
		if (strcasecmp(data, "NE") == 0) {
			map.bkground_anchor = NE;
		} else if (strcasecmp(data, "SE") == 0) {
			map.bkground_anchor = SE;
		} else if (strcasecmp(data, "SW") == 0) {
			map.bkground_anchor = SW;
		} else if (strcasecmp(data, "NW") == 0) {
			map.bkground_anchor = NW;
 		} else {
			map.bkground_anchor = SW;
		}
	} else {
		map.bkground_anchor = SW;
	}
	
	/* level */
	setting = config_lookup(&world_config_cfg, "map.level");
	assert(setting != NULL);

	elem = config_setting_length(setting);

	for(i = 0; i < elem; ++i) {

		xpos = i % map.size_w;
		ypos = i / map.size_w;		

		map.level[xpos][ypos] = config_setting_get_int_elem(setting, i);
	}

	/* level background*/
	setting = config_lookup(&world_config_cfg, "map.level_background");
	assert(setting != NULL);

	elem = config_setting_length(setting);

	for(i = 0; i < elem; ++i) {

		xpos = i % map.size_w;
		ypos = i / map.size_w;

		map.level_fg[xpos][ypos] = config_setting_get_int_elem(setting, i);
	}

	/* tileset */
	setting = config_lookup(&world_config_cfg, "map.tileset");
	assert(setting != NULL);	
	
	elem = config_setting_length(setting);
	printf("Loading tileset (%i)\n", elem);
	
	assert(elem != 0);
	for(i = 0; i < elem; ++i) {
		config_setting_t *tile_elem = config_setting_get_elem(setting, i);
		assert(config_setting_lookup_string(tile_elem, "tile", &data));

		fullname = malloc(strlen(WORLD_GFX_DIR) + strlen(data) +1);
		strcpy(fullname, WORLD_GFX_DIR);	
		strcat(fullname, data);		
		printf("Loading %s\n", fullname);

		if (config_setting_lookup_string(tile_elem, "hitsound", &data)) {
			sfxhitname = malloc(strlen(WORLD_SFX_DIR) + strlen(data) +1);
			strcpy(sfxhitname, WORLD_SFX_DIR);				
			strcat(sfxhitname, data);
		} else {
			sfxhitname = NULL;
		}
		
		if (!config_setting_lookup_int(tile_elem, "tick", &tick)) {
			tick = 0;
		}
		/* bounding box */
		if (!config_setting_lookup_int(tile_elem, "BBoxX", &bbx)) {
			bbx = 0;
		}
		if (!config_setting_lookup_int(tile_elem, "BBoxY", &bby)) {
			bby = 0;
		}
		if (!config_setting_lookup_int(tile_elem, "BBoxW", &bbw)) {
			bbw = TILE_SIZE;
		}
		if (!config_setting_lookup_int(tile_elem, "BBoxH", &bbh)) {
			bbh = TILE_SIZE;
		}

		/* class */
		if (config_setting_lookup_string(tile_elem, "class", &data)) { 
			if (strcasecmp(data, "SOLID") == 0) {
				class = TILE_SOLID;
			} else if (strcasecmp(data, "SOLID_BREAKABLE") == 0) {
				class = TILE_SOLID_BREAKABLE;
			} else if (strcasecmp(data, "SOLID_FIXED") == 0) {
				class = TILE_SOLID_FIXED;
			} else if (strcasecmp(data, "MORTAL") == 0) {
				class = TILE_MORTAL;
			} else if (strcasecmp(data, "COIN") == 0) {
				class = TILE_COIN;
			} else if (strcasecmp(data, "LINK_UP") == 0) {
				class = TILE_LINK_UP;
			} else if (strcasecmp(data, "LINK_DOWN") == 0) {
				class = TILE_LINK_DOWN;
			} else if (strcasecmp(data, "LINK_LEFT") == 0) {
				class = TILE_LINK_LEFT;
			} else if (strcasecmp(data, "LINK_RIGHT") == 0) {
				class = TILE_LINK_RIGHT;
	 		} else {
				class = TILE_UNDEFINED;
			}
		} else {
			class = TILE_SOLID;
		}
		assert (class != TILE_UNDEFINED);

		if (!config_setting_lookup_int(tile_elem, "hitpoint", &hitpoint)) {
			hitpoint = 0;
		}
		
		tile_Load(fullname, class, hitpoint, sfxhitname, tick, bbx, bby, bbw, bbh); /* tileset */

		free(fullname);

		if (sfxhitname) free(sfxhitname);
		sfxhitname = NULL;

		
	}

	/* tileset background*/
	setting = config_lookup(&world_config_cfg, "map.tileset_background");
	assert(setting != NULL);	
	
	elem = config_setting_length(setting);
	printf("Loading tileset background (%i)\n", elem);
	
	assert(elem != 0);
	for(i = 0; i < elem; ++i) {
		config_setting_t *tile_elem = config_setting_get_elem(setting, i);
		assert(config_setting_lookup_string(tile_elem, "tile", &data));

		fullname = malloc(strlen(WORLD_GFX_DIR) + strlen(data) +1);
		strcpy(fullname, WORLD_GFX_DIR);	
		strcat(fullname, data);		
		printf("Loading %s\n", fullname);

		if (!config_setting_lookup_int(tile_elem, "tick", &tick)) {
			tick = 0;
		}

		tile_bk_Load(fullname, tick); /* tileset background*/
		free(fullname);
	}

	map.level_hit = (long int**)calloc(map.size_w, sizeof(long int*));
	for(i=0; i < map.size_w ; ++i) {
		map.level_hit[i] = (long int*)calloc(map.size_h, sizeof(long int));
	}
	assert(map.level_hit); /* out of map */

	if (config_lookup_int(world_config, "map.spawn.x", &i)) { 
		map.respawn_x = i;
	}
	assert (map.respawn_x != 0);
	
	if (config_lookup_int(world_config, "map.spawn.y", &i)) { 
		map.respawn_y = i;
	}
	assert (map.respawn_y != 0);

	if (config_lookup_int(world_config, "map.spawn.lookat", &i)) {
		if (i == 0) {
			map.respawn_lookat = AVATAR_DX;		
		} else {
			map.respawn_lookat = AVATAR_SX;				
		}
	} else {
		map.respawn_lookat = AVATAR_DX;
	}

	/* link */
	setting = config_lookup(&world_config_cfg, "map.link");

	if (setting) {
		elem = config_setting_length(setting);
		printf("Loading link (%i)\n", elem);
		for(i = 0; i < elem; ++i) {
			config_setting_t *link_elem = config_setting_get_elem(setting, i);

			if (!config_setting_lookup_int(link_elem, "src_tile_x", &srcx)) {
				assert(SDL_FALSE);			
			}
			if (!config_setting_lookup_int(link_elem, "src_tile_y", &srcy)) {
				assert(SDL_FALSE);			
			}
			if (!config_setting_lookup_int(link_elem, "dst_tile_x", &dstx)) {
				assert(SDL_FALSE);			
			}
			if (!config_setting_lookup_int(link_elem, "dst_tile_y", &dsty)) {
				assert(SDL_FALSE);			
			}

			srcfacing = LINK_UNDEFINED;
			if (config_setting_lookup_string(link_elem, "src_facing", &data)) { 
				if (strcasecmp(data, "LINK_UP") == 0) {
					srcfacing = LINK_UP;
				} else if (strcasecmp(data, "LINK_DOWN") == 0) {
					srcfacing = LINK_DOWN;
				} else if (strcasecmp(data, "LINK_LEFT") == 0) {
					srcfacing = LINK_LEFT;
				} else if (strcasecmp(data, "LINK_RIGHT") == 0) {
					srcfacing = LINK_RIGHT;
				}
			}
			assert (srcfacing != LINK_UNDEFINED);
				
			dstfacing = LINK_UNDEFINED;
			if (config_setting_lookup_string(link_elem, "dst_facing", &data)) { 
				if (strcasecmp(data, "LINK_UP") == 0) {
					dstfacing = LINK_UP;
				} else if (strcasecmp(data, "LINK_DOWN") == 0) {
					dstfacing = LINK_DOWN;
				} else if (strcasecmp(data, "LINK_LEFT") == 0) {
					dstfacing = LINK_LEFT;
				} else if (strcasecmp(data, "LINK_RIGHT") == 0) {
					dstfacing = LINK_RIGHT;
				}
			}
			assert (dstfacing != LINK_UNDEFINED);

			multilink = SDL_FALSE;
			if (config_setting_lookup_bool(link_elem, "multilink", &bool)) { 
				multilink = bool;
			}
		
			link_Add (srcfacing, srcx, srcy, dstfacing, dstx, dsty, multilink);

		}
	} else {
		printf("No link found\n");
	}
	
}


void map_Blit(void) 
{
	/* lPosX, lPosY -> +-------------       */
	/*                 | world coord	*/
	/*                 | 			*/

	long int x, xini, xmod, lPX;
	long int y, yini, ymod, lPY;	
	long int idelta;
	long int hit;	
	float updownlim;	
	
	SDL_Rect recdst, recsrc;
	Uint32 color;
	Sint16 old_recX, old_recY;
	
	color = SDL_MapRGB(graph_screen->format, map.bkground_R, map.bkground_G, map.bkground_B);
	SDL_FillRect(graph_screen, NULL, color);

	/* Background code */
	if ((map.bkground_anchor == NW) || (map.bkground_anchor == SW)) {
		/*	SX	*/
		lPX = camera_x;
		xini = (lPX/map.bkground_speed) / map.bkground->w;
		xmod = (lPX/map.bkground_speed) % map.bkground->w;
	} else {
		/* 	DX	*/
		idelta = map.bkground->w - ((map.size_w * TILE_SIZE - graph_RESX) / map.bkground_speed + graph_RESX) % map.bkground->w;
		lPX = camera_x;		
		xini = ((lPX/map.bkground_speed) + idelta) / map.bkground->w;
		xmod = ((lPX/map.bkground_speed) + idelta) % map.bkground->w;
	}
	
	if ((map.bkground_anchor == NE) || (map.bkground_anchor == NW)) {
		/*	UP	*/	
		lPY = camera_y;
		yini = (lPY/map.bkground_speed) / map.bkground->h;	
		ymod = (lPY/map.bkground_speed) % map.bkground->h;	
	} else {
		/*	DOWN	*/
		idelta = map.bkground->h - ((map.size_h * TILE_SIZE - graph_RESY) / map.bkground_speed + graph_RESY) % map.bkground->h;
		lPY = camera_y;
		yini = ((lPY/map.bkground_speed) + idelta) / map.bkground->h;	
		ymod = ((lPY/map.bkground_speed) + idelta) % map.bkground->h;	
	}
	
	for (x=xini ; (x - xini) * map.bkground->w - xmod < graph_RESX ; ++x) {
		
		for (y=yini ; (y - yini) * map.bkground->h - ymod < graph_RESY; ++y) {

			recsrc.h = map.bkground->h; recsrc.w = map.bkground->w;
			recsrc.x = 0; recsrc.y = 0;
	
			recdst.h = 0; recdst.w = 0;
			recdst.x = (x - xini) * map.bkground->w - xmod;
			recdst.y = (y - yini) * map.bkground->h - ymod;

			if (x==xini) {
				recsrc.w = map.bkground->w - xmod;
				recsrc.x = xmod; 
				recdst.x = 0;
			} 
			if (y==yini) {
				recsrc.h = map.bkground->h - ymod;
				recsrc.y = ymod;
				recdst.y = 0;
			}

			SDL_BlitSurface(map.bkground, &recsrc, graph_screen, &recdst);
		}
	}

	/* map code */

	xini = camera_x / TILE_SIZE;
	xmod = camera_x % TILE_SIZE;
	
	yini = camera_y / TILE_SIZE;	
	ymod = camera_y % TILE_SIZE;	

	for (x=xini ; (x - xini) * TILE_SIZE - xmod < graph_RESX ; ++x) {
		/*assert(x < map_size_w); // out of map */
		
		if ((x < map.size_w) && (x >= 0)) { /* don't render if map is small */
		
			for (y=yini ; (y - yini) * TILE_SIZE - ymod < graph_RESY; ++y) {

				/*assert(y < map_size_h); // out of map */

				if ((y < map.size_h) && (y >= 0)) { /* don't render if map is small */
		
					recsrc.h = TILE_SIZE; recsrc.w = TILE_SIZE;
					recsrc.x = 0; recsrc.y = 0;
			
					recdst.h = 0; recdst.w = 0;
					recdst.x = (x - xini) * TILE_SIZE - xmod;
					recdst.y = (y - yini) * TILE_SIZE - ymod;

					if (x==xini) {
						recsrc.w = TILE_SIZE - xmod;
						recsrc.x = xmod; 
						recdst.x = 0;
					} 
					if (y==yini) {
						recsrc.h = TILE_SIZE - ymod;
						recsrc.y = ymod;
						recdst.y = 0;
					}

					/* background level-map */
					if (map.level_fg[x][y] != 0) {
						old_recY = recsrc.y;
						old_recX = recsrc.x;											
						if (tile_tile[map.level_fg[x][y]-1].Tile->h > tile_tile_bk[map.level_fg[x][y]-1].Tile->w) {
							recsrc.y = recsrc.y + (tile_tile_bk[map.level_fg[x][y]-1].KeyFrame-1) * TILE_SIZE;
						} else {
							recsrc.x = recsrc.x + (tile_tile_bk[map.level_fg[x][y]-1].KeyFrame-1) * TILE_SIZE;							
						}
						SDL_BlitSurface(tile_tile_bk[map.level_fg[x][y]-1].Tile, &recsrc, graph_screen, &recdst);						
						recsrc.y = old_recY;
						recsrc.x = old_recX;
					}

					/* level-map */
					if (map.level[x][y] != 0) {
						if (tile_tile[map.level[x][y]-1].Anim) {
							if (tile_tile[map.level[x][y]-1].Tile->h > tile_tile[map.level[x][y]-1].Tile->w) {
								recsrc.y = recsrc.y + (tile_tile[map.level[x][y]-1].KeyFrame-1) * TILE_SIZE;
							} else {
								recsrc.x = recsrc.x + (tile_tile[map.level[x][y]-1].KeyFrame-1) * TILE_SIZE;							
							}
						}

						if (map.level_hit[x][y] != 0) {
							updownlim = MAP_HIT_TIME / 2.0;

							if (map.level_hit[x][y] >= updownlim) {
								/* going up */
								hit = ((MAP_HIT_TIME - map.level_hit[x][y]) / updownlim) * MAP_HIT_SIZE;
							} else {
								/* going down */
								hit = MAP_HIT_SIZE + 1 - ((updownlim - map.level_hit[x][y]) / updownlim) * MAP_HIT_SIZE;															
							}
							
							hit = posval(hit);

							recdst.y -= hit;
						}
						
						SDL_BlitSurface(tile_tile[map.level[x][y]-1].Tile, &recsrc, graph_screen, &recdst);						

					}
				}					
			}
		}							
	}
}

void map_Tick(void) 
{
	unsigned int x, y;
	struct map_update_t *map_update;
	struct map_update_t *map_update_nxt;

	if (map.timer > 0) {
		
		if (!avatar_SomeoneInLink())	map.timer--;

		link_Tick();

		tile_Tick();

		map_update = map_update_top;
		while (map_update) {
			map_update_nxt = map_update->next;	

			x = map_update->x;
			y = map_update->y;

			map.level_hitby[x][y] = -1;
			
			if ((map.level[x][y] != 0) && (map.level_hit[x][y] > 0) ){
				map.level_hit[x][y]--;

				if (map.level_hit[x][y] <= 0) {
					map.level_hit[x][y] = 0;
					map_UpdateClear(map_update);
				}

			} else {
				map.level_hit[x][y] = 0;
				map_UpdateClear(map_update);				
			}			
			
			map_update = map_update_nxt;		
		}
		
	} else {
		avatar_StartToDieAll();
	}
}


SDL_bool map_SolidCollision(int player, long int ax1, long int ay1, long int ax2, long int ay2) 
{
	long int x, y;
	long int tax, tay, tbx, tby;
	SDL_bool collide = SDL_FALSE;

	/* if X is outside there isnt any collisione */
	x = ax2 / TILE_SIZE;
	if (x < 0) return SDL_FALSE;

	x = ax1 / TILE_SIZE;
	if (x >= map.size_w) return SDL_FALSE;

	/* if Y is outside there isnt any collisione */
	y = ay2 / TILE_SIZE;
	if (y < 0) return SDL_FALSE;

	y = ay1 / TILE_SIZE;
	if (y >= map.size_h) return SDL_FALSE;
	
	/* don't go outside the map */
	x = ax1 / TILE_SIZE;
	if (x < 0) ax1 = 0;
	
	x = ax2 / TILE_SIZE;
	if (x >= map.size_w) ax2 = map.size_w * TILE_SIZE -1;

	y = ay1 / TILE_SIZE;
	if (y < 0) ay1 = 0;

	y = ay2 / TILE_SIZE;
	if (y >= map.size_h) ay2 = map.size_h * TILE_SIZE -1;
	/* */

	x = ax1 / TILE_SIZE;
	y = ay1 / TILE_SIZE;

	assert(x >= 0);
	assert(y >= 0);
	assert(x < map.size_w);
	assert(y < map.size_h);
	
	if (map.level[x][y]) {
		if IS_SOLID(tile_tile[map.level[x][y]-1].Class) {		
			tile_GetBBox(map.level[x][y]-1, &tax, &tay, &tbx, &tby);	
			if (Collide(ax1, ay1, ax2, ay2, x * TILE_SIZE + tax, y * TILE_SIZE + tay, x * TILE_SIZE + tbx, y * TILE_SIZE + tby)) {
				map_HitTile(player, x, y);
				collide = SDL_TRUE;		
			}
		}
	}

	x = ax1 / TILE_SIZE;
	y = ay2 / TILE_SIZE;

	assert(x >= 0);
	assert(y >= 0);
	assert(x < map.size_w);
	assert(y < map.size_h);
	
	if (map.level[x][y]) {
		if IS_SOLID(tile_tile[map.level[x][y]-1].Class) {			
			tile_GetBBox(map.level[x][y]-1, &tax, &tay, &tbx, &tby);	
			if (Collide(ax1, ay1, ax2, ay2, x * TILE_SIZE + tax, y * TILE_SIZE + tay, x * TILE_SIZE + tbx, y * TILE_SIZE + tby)) {
				map_HitTile(player, x, y);
				collide = SDL_TRUE;
			}
		}
	}

	x = ax2 / TILE_SIZE;
	y = ay2 / TILE_SIZE;

	assert(x >= 0);
	assert(y >= 0);
	assert(x < map.size_w);
	assert(y < map.size_h);
	
	if (map.level[x][y]) {
		if IS_SOLID(tile_tile[map.level[x][y]-1].Class) {				
			tile_GetBBox(map.level[x][y]-1, &tax, &tay, &tbx, &tby);	
			if (Collide(ax1, ay1, ax2, ay2, x * TILE_SIZE + tax, y * TILE_SIZE + tay, x * TILE_SIZE + tbx, y * TILE_SIZE + tby)) {
				map_HitTile(player, x, y);
				collide = SDL_TRUE;				
			}
		}			
	}

	x = ax2 / TILE_SIZE;
	y = ay1 / TILE_SIZE;

	assert(x >= 0);
	assert(y >= 0);
	assert(x < map.size_w);
	assert(y < map.size_h);
	
	if (map.level[x][y]) {
		if IS_SOLID(tile_tile[map.level[x][y]-1].Class) {					
			tile_GetBBox(map.level[x][y]-1, &tax, &tay, &tbx, &tby);
			if (Collide(ax1, ay1, ax2, ay2, x * TILE_SIZE + tax, y * TILE_SIZE + tay, x * TILE_SIZE + tbx, y * TILE_SIZE + tby)) {
				map_HitTile(player, x, y);
				collide = SDL_TRUE;				
			}
		}			
	}

	return collide;		
}

SDL_bool map_TestSolidCollision(long int ax1, long int ay1, long int ax2, long int ay2) 
{
	long int x, y;
	long int tax, tay, tbx, tby;

	/* if X is outside there isnt any collisione */
	x = ax2 / TILE_SIZE;
	if (x < 0) return SDL_FALSE;

	x = ax1 / TILE_SIZE;
	if (x >= map.size_w) return SDL_FALSE;

	/* if Y is outside there isnt any collisione */
	y = ay2 / TILE_SIZE;
	if (y < 0) return SDL_FALSE;

	y = ay1 / TILE_SIZE;
	if (y >= map.size_h) return SDL_FALSE;
	
	/* don't go outside the map */
	x = ax1 / TILE_SIZE;
	if (x < 0) ax1 = 0;
	
	x = ax2 / TILE_SIZE;
	if (x >= map.size_w) ax2 = map.size_w * TILE_SIZE -1;

	y = ay1 / TILE_SIZE;
	if (y < 0) ay1 = 0;

	y = ay2 / TILE_SIZE;
	if (y >= map.size_h) ay2 = map.size_h * TILE_SIZE -1;


	/* now collision test */	
	x = ax1 / TILE_SIZE;
	y = ay1 / TILE_SIZE;

	assert(x >= 0);
	assert(y >= 0);
	assert(x < map.size_w);
	assert(y < map.size_h);
	
	if (map.level[x][y]) {
		if IS_SOLID(tile_tile[map.level[x][y]-1].Class) {	
			tile_GetBBox(map.level[x][y]-1, &tax, &tay, &tbx, &tby);	
			if (Collide(ax1, ay1, ax2, ay2, x * TILE_SIZE + tax, y * TILE_SIZE + tay, x * TILE_SIZE + tbx, y * TILE_SIZE + tby)) {
				return SDL_TRUE;		
			}
		}			
	}

	x = ax1 / TILE_SIZE;
	y = ay2 / TILE_SIZE;

	assert(x >= 0);
	assert(y >= 0);
	assert(x < map.size_w);
	assert(y < map.size_h);
	
	if (map.level[x][y]) {
		if IS_SOLID(tile_tile[map.level[x][y]-1].Class) {	
			tile_GetBBox(map.level[x][y]-1, &tax, &tay, &tbx, &tby);	
			if (Collide(ax1, ay1, ax2, ay2, x * TILE_SIZE + tax, y * TILE_SIZE + tay, x * TILE_SIZE + tbx, y * TILE_SIZE + tby)) {
				return SDL_TRUE;		
			}
		}			
	}

	x = ax2 / TILE_SIZE;
	y = ay2 / TILE_SIZE;

	assert(x >= 0);
	assert(y >= 0);
	assert(x < map.size_w);
	assert(y < map.size_h);
	
	if (map.level[x][y]) {
		if IS_SOLID(tile_tile[map.level[x][y]-1].Class) {	
			tile_GetBBox(map.level[x][y]-1, &tax, &tay, &tbx, &tby);	
			if (Collide(ax1, ay1, ax2, ay2, x * TILE_SIZE + tax, y * TILE_SIZE + tay, x * TILE_SIZE + tbx, y * TILE_SIZE + tby)) {
				return SDL_TRUE;		
			}				
		}				
	}

	x = ax2 / TILE_SIZE;
	y = ay1 / TILE_SIZE;

	assert(x >= 0);
	assert(y >= 0);
	assert(x < map.size_w);
	assert(y < map.size_h);
	
	if (map.level[x][y]) {
		if IS_SOLID(tile_tile[map.level[x][y]-1].Class) {	
			tile_GetBBox(map.level[x][y]-1, &tax, &tay, &tbx, &tby);
			if (Collide(ax1, ay1, ax2, ay2, x * TILE_SIZE + tax, y * TILE_SIZE + tay, x * TILE_SIZE + tbx, y * TILE_SIZE + tby)) {
				return SDL_TRUE;		
			}
		}			
	}

	return SDL_FALSE;		
}

SDL_bool map_CanWalkOn(long int ax1, long int ay1, long int ax2, long int ay2) 
{
	long int x, y;
	long int tax, tay, tbx, tby;

	/* if X is outside there isnt any collisione */
	x = ax2 / TILE_SIZE;
	if (x < 0) return SDL_FALSE;

	x = ax1 / TILE_SIZE;
	if (x >= map.size_w) return SDL_FALSE;

	/* if Y is outside there isnt any collisione */
	y = ay2 / TILE_SIZE;
	if (y < 0) return SDL_FALSE;

	y = ay1 / TILE_SIZE;
	if (y >= map.size_h) return SDL_FALSE;
	
	/* don't go outside the map */
	x = ax1 / TILE_SIZE;
	if (x < 0) ax1 = 0;
	
	x = ax2 / TILE_SIZE;
	if (x >= map.size_w) ax2 = map.size_w * TILE_SIZE -1;

	y = ay1 / TILE_SIZE;
	if (y < 0) ay1 = 0;

	y = ay2 / TILE_SIZE;
	if (y >= map.size_h) ay2 = map.size_h * TILE_SIZE -1;

	x = ax1 / TILE_SIZE;
	y = ay1 / TILE_SIZE;

	assert(x >= 0);
	assert(y >= 0);
	assert(x < map.size_w);
	assert(y < map.size_h);
	
	if (map.level[x][y]) {
		if IS_SOLID(tile_tile[map.level[x][y]-1].Class) {
			tile_GetBBox(map.level[x][y]-1, &tax, &tay, &tbx, &tby);	
			if (Collide(ax1, ay1, ax2, ay2, x * TILE_SIZE + tax, y * TILE_SIZE + tay, x * TILE_SIZE + tbx, y * TILE_SIZE + tby)) {
				return SDL_FALSE;
			}
		}			
	}

	x = ax1 / TILE_SIZE;
	y = ay2 / TILE_SIZE;

	assert(x >= 0);
	assert(y >= 0);
	assert(x < map.size_w);
	assert(y < map.size_h);
	
	if (map.level[x][y]) {
		if IS_SOLID(tile_tile[map.level[x][y]-1].Class) {
			tile_GetBBox(map.level[x][y]-1, &tax, &tay, &tbx, &tby);	
			if (Collide(ax1, ay1, ax2, ay2, x * TILE_SIZE + tax, y * TILE_SIZE + tay, x * TILE_SIZE + tbx, y * TILE_SIZE + tby)) {
				return SDL_FALSE;
			}
		}			
	}

	x = ax2 / TILE_SIZE;
	y = ay2 / TILE_SIZE;

	assert(x >= 0);
	assert(y >= 0);
	assert(x < map.size_w);
	assert(y < map.size_h);
	
	if (map.level[x][y]) {
		if IS_SOLID(tile_tile[map.level[x][y]-1].Class) {
			tile_GetBBox(map.level[x][y]-1, &tax, &tay, &tbx, &tby);	
			if (Collide(ax1, ay1, ax2, ay2, x * TILE_SIZE + tax, y * TILE_SIZE + tay, x * TILE_SIZE + tbx, y * TILE_SIZE + tby)) {
				return SDL_FALSE;
			}
		}			
	}

	x = ax2 / TILE_SIZE;
	y = ay1 / TILE_SIZE;

	assert(x >= 0);
	assert(y >= 0);
	assert(x < map.size_w);
	assert(y < map.size_h);
	
	if (map.level[x][y]) {
		if IS_SOLID(tile_tile[map.level[x][y]-1].Class) {
			tile_GetBBox(map.level[x][y]-1, &tax, &tay, &tbx, &tby);
			if (Collide(ax1, ay1, ax2, ay2, x * TILE_SIZE + tax, y * TILE_SIZE + tay, x * TILE_SIZE + tbx, y * TILE_SIZE + tby)) {
				return SDL_FALSE;
			}
		}			
	}

	/* step down */
	ay1 = ay1 + 1;
	ay2 = ay2 + 1;

	/* if X is outside there isnt any collisione */
	x = ax2 / TILE_SIZE;
	if (x < 0) return SDL_FALSE;

	x = ax1 / TILE_SIZE;
	if (x >= map.size_w) return SDL_FALSE;

	/* if Y is outside there isnt any collisione */
	y = ay2 / TILE_SIZE;
	if (y < 0) return SDL_FALSE;

	y = ay1 / TILE_SIZE;
	if (y >= map.size_h) return SDL_FALSE;
	
	/* don't go outside the map */
	x = ax1 / TILE_SIZE;
	if (x < 0) ax1 = 0;
	
	x = ax2 / TILE_SIZE;
	if (x >= map.size_w) ax2 = map.size_w * TILE_SIZE -1;

	y = ay1 / TILE_SIZE;
	if (y < 0) ay1 = 0;

	y = ay2 / TILE_SIZE;
	if (y >= map.size_h) ay2 = map.size_h * TILE_SIZE -1;

	
	
	x = ax1 / TILE_SIZE;
	y = ay1 / TILE_SIZE;

	assert(x >= 0);
	assert(y >= 0);
	assert(x < map.size_w);
	assert(y < map.size_h);
	
	if (map.level[x][y]) {
		if IS_SOLID(tile_tile[map.level[x][y]-1].Class) {
			tile_GetBBox(map.level[x][y]-1, &tax, &tay, &tbx, &tby);	
			if (Collide(ax1, ay1, ax2, ay2, x * TILE_SIZE + tax, y * TILE_SIZE + tay, x * TILE_SIZE + tbx, y * TILE_SIZE + tby)) {
				return SDL_TRUE;
			}
		}			
	}

	x = ax1 / TILE_SIZE;
	y = ay2 / TILE_SIZE;

	assert(x >= 0);
	assert(y >= 0);
	assert(x < map.size_w);
	assert(y < map.size_h);
	
	if (map.level[x][y]) {
		if IS_SOLID(tile_tile[map.level[x][y]-1].Class) {
			tile_GetBBox(map.level[x][y]-1, &tax, &tay, &tbx, &tby);	
			if (Collide(ax1, ay1, ax2, ay2, x * TILE_SIZE + tax, y * TILE_SIZE + tay, x * TILE_SIZE + tbx, y * TILE_SIZE + tby)) {
				return SDL_TRUE;
			}
		}			
	}

	x = ax2 / TILE_SIZE;
	y = ay2 / TILE_SIZE;

	assert(x >= 0);
	assert(y >= 0);
	assert(x < map.size_w);
	assert(y < map.size_h);
	
	if (map.level[x][y]) {
		if IS_SOLID(tile_tile[map.level[x][y]-1].Class) {
			tile_GetBBox(map.level[x][y]-1, &tax, &tay, &tbx, &tby);	
			if (Collide(ax1, ay1, ax2, ay2, x * TILE_SIZE + tax, y * TILE_SIZE + tay, x * TILE_SIZE + tbx, y * TILE_SIZE + tby)) {
				return SDL_TRUE;
			}
		}			
	}

	x = ax2 / TILE_SIZE;
	y = ay1 / TILE_SIZE;

	assert(x >= 0);
	assert(y >= 0);
	assert(x < map.size_w);
	assert(y < map.size_h);
	
	if (map.level[x][y]) {
		if IS_SOLID(tile_tile[map.level[x][y]-1].Class) {
			tile_GetBBox(map.level[x][y]-1, &tax, &tay, &tbx, &tby);
			if (Collide(ax1, ay1, ax2, ay2, x * TILE_SIZE + tax, y * TILE_SIZE + tay, x * TILE_SIZE + tbx, y * TILE_SIZE + tby)) {
				return SDL_TRUE;
			}
		}			
	}

	return SDL_FALSE;		
}

SDL_bool map_UnsolidCollision(int player, long int ax1, long int ay1, long int ax2, long int ay2) 
{
	long int x, y;
	long int tax, tay, tbx, tby;
	SDL_bool collide = SDL_FALSE;

	/* if X is outside there isnt any collisione */
	x = ax2 / TILE_SIZE;
	if (x < 0) return SDL_FALSE;

	x = ax1 / TILE_SIZE;
	if (x >= map.size_w) return SDL_FALSE;

	/* if Y is outside there isnt any collisione */
	y = ay2 / TILE_SIZE;
	if (y < 0) return SDL_FALSE;

	y = ay1 / TILE_SIZE;
	if (y >= map.size_h) return SDL_FALSE;
	
	/* don't go outside the map */
	x = ax1 / TILE_SIZE;
	if (x < 0) ax1 = 0;
	
	x = ax2 / TILE_SIZE;
	if (x >= map.size_w) ax2 = map.size_w * TILE_SIZE -1;

	y = ay1 / TILE_SIZE;
	if (y < 0) ay1 = 0;

	y = ay2 / TILE_SIZE;
	if (y >= map.size_h) ay2 = map.size_h * TILE_SIZE -1;

	/* */
	x = ax1 / TILE_SIZE;
	y = ay1 / TILE_SIZE;

	assert(x >= 0);
	assert(y >= 0);
	assert(x < map.size_w);
	assert(y < map.size_h);
	
	if (map.level[x][y]) {
		if IS_NOT_SOLID(tile_tile[map.level[x][y]-1].Class) {		
			tile_GetBBox(map.level[x][y]-1, &tax, &tay, &tbx, &tby);	
			if (Collide(ax1, ay1, ax2, ay2, x * TILE_SIZE + tax, y * TILE_SIZE + tay, x * TILE_SIZE + tbx, y * TILE_SIZE + tby)) {
				map_HitTile(player, x, y);
				collide = SDL_TRUE;		
			}
		}
	}

	x = ax1 / TILE_SIZE;
	y = ay2 / TILE_SIZE;

	assert(x >= 0);
	assert(y >= 0);
	assert(x < map.size_w);
	assert(y < map.size_h);
	if (map.level[x][y]) {
		if IS_NOT_SOLID(tile_tile[map.level[x][y]-1].Class) {			
			tile_GetBBox(map.level[x][y]-1, &tax, &tay, &tbx, &tby);	
			if (Collide(ax1, ay1, ax2, ay2, x * TILE_SIZE + tax, y * TILE_SIZE + tay, x * TILE_SIZE + tbx, y * TILE_SIZE + tby)) {
				map_HitTile(player, x, y);
				collide = SDL_TRUE;
			}
		}
	}

	x = ax2 / TILE_SIZE;
	y = ay2 / TILE_SIZE;

	assert(x >= 0);
	assert(y >= 0);
	assert(x < map.size_w);
	assert(y < map.size_h);

	if (map.level[x][y]) {
		if IS_NOT_SOLID(tile_tile[map.level[x][y]-1].Class) {				
			tile_GetBBox(map.level[x][y]-1, &tax, &tay, &tbx, &tby);	
			if (Collide(ax1, ay1, ax2, ay2, x * TILE_SIZE + tax, y * TILE_SIZE + tay, x * TILE_SIZE + tbx, y * TILE_SIZE + tby)) {
				map_HitTile(player, x, y);
				collide = SDL_TRUE;				
			}
		}			
	}

	x = ax2 / TILE_SIZE;
	y = ay1 / TILE_SIZE;

	assert(x >= 0);
	assert(y >= 0);
	assert(x < map.size_w);
	assert(y < map.size_h);

	if (map.level[x][y]) {
		if IS_NOT_SOLID(tile_tile[map.level[x][y]-1].Class) {					
			tile_GetBBox(map.level[x][y]-1, &tax, &tay, &tbx, &tby);
			if (Collide(ax1, ay1, ax2, ay2, x * TILE_SIZE + tax, y * TILE_SIZE + tay, x * TILE_SIZE + tbx, y * TILE_SIZE + tby)) {
				map_HitTile(player, x, y);
				collide = SDL_TRUE;				
			}
		}			
	}

	return collide;		
}

struct link_t *map_HyperLinkDown(long int ax1, long int ay1, long int ax2, long int ay2) 
{

	long int x, y;
	long int tax, tay, tbx, tby;

	/* step up */
	ay1 = ay1 - 1;
	ay2 = ay2 - 1;

	/* if X is outside there isnt any collisione */
	x = ax2 / TILE_SIZE;
	if (x < 0) return 0;

	x = ax1 / TILE_SIZE;
	if (x >= map.size_w) return 0;

	/* if Y is outside there isnt any collisione */
	y = ay2 / TILE_SIZE;
	if (y < 0) return 0;

	y = ay1 / TILE_SIZE;
	if (y >= map.size_h) return 0;
	
	/* don't go outside the map */
	x = ax1 / TILE_SIZE;
	if (x < 0) ax1 = 0;
	
	x = ax2 / TILE_SIZE;
	if (x >= map.size_w) ax2 = map.size_w * TILE_SIZE -1;

	y = ay1 / TILE_SIZE;
	if (y < 0) ay1 = 0;

	y = ay2 / TILE_SIZE;
	if (y >= map.size_h) ay2 = map.size_h * TILE_SIZE -1;
	
	x = ax1 / TILE_SIZE;
	y = ay1 / TILE_SIZE;

	assert(x >= 0);
	assert(y >= 0);
	assert(x < map.size_w);
	assert(y < map.size_h);
	
	if (map.level[x][y]) {
		if (tile_tile[map.level[x][y]-1].Class == TILE_LINK_DOWN) {
/*			tile_GetBBox(map.level[x][y]-1, &tax, &tay, &tbx, &tby);	*/
			tile_GetBBoxLinkX(map.level[x][y]-1, &tax, &tay, &tbx, &tby);	
			
			if (Collide(ax1, ay1, ax2, ay2, x * TILE_SIZE + tax, y * TILE_SIZE + tay, x * TILE_SIZE + tbx, y * TILE_SIZE + tby)) {
				return link_Find(x,y);
			}
		}			
	}

	x = ax2 / TILE_SIZE;
	y = ay1 / TILE_SIZE;

	assert(x >= 0);
	assert(y >= 0);
	assert(x < map.size_w);
	assert(y < map.size_h);
	
	if (map.level[x][y]) {
		if (tile_tile[map.level[x][y]-1].Class == TILE_LINK_DOWN) {
 /*			tile_GetBBox(map.level[x][y]-1, &tax, &tay, &tbx, &tby); */
			tile_GetBBoxLinkX(map.level[x][y]-1, &tax, &tay, &tbx, &tby);					
			if (Collide(ax1, ay1, ax2, ay2, x * TILE_SIZE + tax, y * TILE_SIZE + tay, x * TILE_SIZE + tbx, y * TILE_SIZE + tby)) {
				return link_Find(x,y);
			}
		}			
	}

	return NULL;		
}

struct link_t *map_HyperLinkUp(long int ax1, long int ay1, long int ax2, long int ay2) 
{

	long int x, y;
	long int tax, tay, tbx, tby;

	/* step down */
	ay1 = ay1 + 1;
	ay2 = ay2 + 1;

	/* if X is outside there isnt any collisione */
	x = ax2 / TILE_SIZE;
	if (x < 0) return 0;

	x = ax1 / TILE_SIZE;
	if (x >= map.size_w) return 0;

	/* if Y is outside there isnt any collisione */
	y = ay2 / TILE_SIZE;
	if (y < 0) return 0;

	y = ay1 / TILE_SIZE;
	if (y >= map.size_h) return 0;
	
	/* don't go outside the map */
	x = ax1 / TILE_SIZE;
	if (x < 0) ax1 = 0;
	
	x = ax2 / TILE_SIZE;
	if (x >= map.size_w) ax2 = map.size_w * TILE_SIZE -1;

	y = ay1 / TILE_SIZE;
	if (y < 0) ay1 = 0;

	y = ay2 / TILE_SIZE;
	if (y >= map.size_h) ay2 = map.size_h * TILE_SIZE -1;
	
	x = ax1 / TILE_SIZE;
	y = ay2 / TILE_SIZE;

	assert(x >= 0);
	assert(y >= 0);
	assert(x < map.size_w);
	assert(y < map.size_h);
	
	if (map.level[x][y]) {
		if (tile_tile[map.level[x][y]-1].Class == TILE_LINK_UP) {
/*			tile_GetBBox(map.level[x][y]-1, &tax, &tay, &tbx, &tby); */
			tile_GetBBoxLinkX(map.level[x][y]-1, &tax, &tay, &tbx, &tby);				
			if (Collide(ax1, ay1, ax2, ay2, x * TILE_SIZE + tax, y * TILE_SIZE + tay, x * TILE_SIZE + tbx, y * TILE_SIZE + tby)) {
				return link_Find(x,y);
			}
		}			
	}

	x = ax2 / TILE_SIZE;
	y = ay2 / TILE_SIZE;

	assert(x >= 0);
	assert(y >= 0);
	assert(x < map.size_w);
	assert(y < map.size_h);
	
	if (map.level[x][y]) {
		if (tile_tile[map.level[x][y]-1].Class == TILE_LINK_UP) {
/*			tile_GetBBox(map.level[x][y]-1, &tax, &tay, &tbx, &tby);	*/
			tile_GetBBoxLinkX(map.level[x][y]-1, &tax, &tay, &tbx, &tby);				
			if (Collide(ax1, ay1, ax2, ay2, x * TILE_SIZE + tax, y * TILE_SIZE + tay, x * TILE_SIZE + tbx, y * TILE_SIZE + tby)) {
				return link_Find(x,y);
			}
		}			
	}

	return NULL;		

}

struct link_t *map_HyperLinkLeft(long int ax1, long int ay1, long int ax2, long int ay2) 
{

	long int x, y;
	long int tax, tay, tbx, tby;

	/* step right */
	ax1 = ax1 + 1;
	ax2 = ax2 + 1;

	/* if X is outside there isnt any collisione */
	x = ax2 / TILE_SIZE;
	if (x < 0) return 0;

	x = ax1 / TILE_SIZE;
	if (x >= map.size_w) return 0;

	/* if Y is outside there isnt any collisione */
	y = ay2 / TILE_SIZE;
	if (y < 0) return 0;

	y = ay1 / TILE_SIZE;
	if (y >= map.size_h) return 0;
	
	/* don't go outside the map */
	x = ax1 / TILE_SIZE;
	if (x < 0) ax1 = 0;
	
	x = ax2 / TILE_SIZE;
	if (x >= map.size_w) ax2 = map.size_w * TILE_SIZE -1;

	y = ay1 / TILE_SIZE;
	if (y < 0) ay1 = 0;

	y = ay2 / TILE_SIZE;
	if (y >= map.size_h) ay2 = map.size_h * TILE_SIZE -1;
	
	x = ax2 / TILE_SIZE;
	y = ay1 / TILE_SIZE;

	assert(x >= 0);
	assert(y >= 0);
	assert(x < map.size_w);
	assert(y < map.size_h);
	
	if (map.level[x][y]) {
		if (tile_tile[map.level[x][y]-1].Class == TILE_LINK_LEFT) {
/*			tile_GetBBox(map.level[x][y]-1, &tax, &tay, &tbx, &tby); */
			tile_GetBBoxLinkY(map.level[x][y]-1, &tax, &tay, &tbx, &tby);					
			if (Collide(ax1, ay1, ax2, ay2, x * TILE_SIZE + tax, y * TILE_SIZE + tay, x * TILE_SIZE + tbx, y * TILE_SIZE + tby)) {
				return link_Find(x,y);
			}
		}			
	}

	x = ax2 / TILE_SIZE;
	y = ay2 / TILE_SIZE;

	assert(x >= 0);
	assert(y >= 0);
	assert(x < map.size_w);
	assert(y < map.size_h);
	
	if (map.level[x][y]) {
		if (tile_tile[map.level[x][y]-1].Class == TILE_LINK_LEFT) {
/*			tile_GetBBox(map.level[x][y]-1, &tax, &tay, &tbx, &tby);	*/
			tile_GetBBoxLinkY(map.level[x][y]-1, &tax, &tay, &tbx, &tby);					
			if (Collide(ax1, ay1, ax2, ay2, x * TILE_SIZE + tax, y * TILE_SIZE + tay, x * TILE_SIZE + tbx, y * TILE_SIZE + tby)) {
				return link_Find(x,y);
			}
		}			
	}

	return NULL;		

}

struct link_t *map_HyperLinkRight(long int ax1, long int ay1, long int ax2, long int ay2)  
{

	long int x, y;
	long int tax, tay, tbx, tby;

	/* step left */
	ax1 = ax1 - 1;
	ax2 = ax2 - 1;

	/* if X is outside there isnt any collisione */
	x = ax2 / TILE_SIZE;
	if (x < 0) return 0;

	x = ax1 / TILE_SIZE;
	if (x >= map.size_w) return 0;

	/* if Y is outside there isnt any collisione */
	y = ay2 / TILE_SIZE;
	if (y < 0) return 0;

	y = ay1 / TILE_SIZE;
	if (y >= map.size_h) return 0;
	
	/* don't go outside the map */
	x = ax1 / TILE_SIZE;
	if (x < 0) ax1 = 0;
	
	x = ax2 / TILE_SIZE;
	if (x >= map.size_w) ax2 = map.size_w * TILE_SIZE -1;

	y = ay1 / TILE_SIZE;
	if (y < 0) ay1 = 0;

	y = ay2 / TILE_SIZE;
	if (y >= map.size_h) ay2 = map.size_h * TILE_SIZE -1;
	
	x = ax1 / TILE_SIZE;
	y = ay1 / TILE_SIZE;

	assert(x >= 0);
	assert(y >= 0);
	assert(x < map.size_w);
	assert(y < map.size_h);
	
	if (map.level[x][y]) {
		if (tile_tile[map.level[x][y]-1].Class == TILE_LINK_RIGHT) {
/*			tile_GetBBox(map.level[x][y]-1, &tax, &tay, &tbx, &tby); */
			tile_GetBBoxLinkY(map.level[x][y]-1, &tax, &tay, &tbx, &tby);					
			if (Collide(ax1, ay1, ax2, ay2, x * TILE_SIZE + tax, y * TILE_SIZE + tay, x * TILE_SIZE + tbx, y * TILE_SIZE + tby)) {
				return link_Find(x,y);
			}
		}			
	}

	x = ax1 / TILE_SIZE;
	y = ay2 / TILE_SIZE;

	assert(x >= 0);
	assert(y >= 0);
	assert(x < map.size_w);
	assert(y < map.size_h);
	
	if (map.level[x][y]) {
		if (tile_tile[map.level[x][y]-1].Class == TILE_LINK_RIGHT) {
/*			tile_GetBBox(map.level[x][y]-1, &tax, &tay, &tbx, &tby);	*/
			tile_GetBBoxLinkY(map.level[x][y]-1, &tax, &tay, &tbx, &tby);					
			if (Collide(ax1, ay1, ax2, ay2, x * TILE_SIZE + tax, y * TILE_SIZE + tay, x * TILE_SIZE + tbx, y * TILE_SIZE + tby)) {
				return link_Find(x,y);
			}
		}			
	}

	return NULL;		

}

void map_SetMap(long int x, long int y, int tile) 
{
	map.level[x][y] = tile;
}

void map_HitTile(int player, long int x, long int y) 
{
	struct graph_vec2_t pos, vel;
	long int tax, tay, tbx, tby;	


	assert(x >= 0);
	assert(y >= 0);
	assert(x < map.size_w);
	assert(y < map.size_h);

	/* multihit, is not allowed */
	if (map.level_hitby[x][y] != player) {

		map.level_hitby[x][y] = player;

		/* don't multi-add the same maptile in the UpdateList (tick) */
		if (!map_Updating(x, y)) {
			map_UpdateAdd(x, y);
		}
		
		if (tile_tile[map.level[x][y]-1].HitSound) sound_SfxDirectPlay(tile_tile[map.level[x][y]-1].HitSound);
	
		switch (tile_tile[map.level[x][y]-1].Class) {

		case TILE_SOLID:
			avatar_AddPoint(player, tile_tile[map.level[x][y]-1].HitPoint);	
			map.level_hit[x][y] = MAP_HIT_TIME;
			break;
		
		case TILE_SOLID_BREAKABLE:
			avatar_AddPoint(player, tile_tile[map.level[x][y]-1].HitPoint);

			tile_GetBBox(map.level[x][y]-1, &tax, &tay, &tbx, &tby);
			pos.x = x * TILE_SIZE + tax + fabs(tax - tbx)/2.0; 
			pos.y = y * TILE_SIZE + tay + fabs(tay - tby)/2.0;

			vel.x = .5 + LIM(fabs(avatar[player].old_vx), 1);

			if (avatar[player].old_vy == 0) {
				/* if jump&break at the same time */
				vel.y = 1 + LIM(fabs(avatar[player].old_vy), 1);
			} else {
				vel.y = .5 + LIM(fabs(avatar[player].old_vy), 1);		
			}
				
			particle_MultiSpawn(PARTICLE_BREAK, pos, vel);

			map.level_hit[x][y] = 0;
			map.level[x][y]     = 0;
			break;		
		
		case TILE_COIN:
			avatar_AddCoin (player);
			avatar_AddPoint(player, tile_tile[map.level[x][y]-1].HitPoint);		
		
			tile_GetBBox(map.level[x][y]-1, &tax, &tay, &tbx, &tby);
			pos.x = x * TILE_SIZE + tax + fabs(tax - tbx)/2.0; 
			pos.y = y * TILE_SIZE + tay + fabs(tay - tby)/2.0;

			vel.x = LIM(fabs(avatar[player].vx), 1);
			vel.y = LIM(fabs(avatar[player].vy), 1);

			particle_MultiSpawn(PARTICLE_STAR, pos, vel);
			map.level_hit[x][y] = 0;
			map.level[x][y]     = 0;
			break;		

		case TILE_LINK_UP:
		case TILE_LINK_DOWN:
		case TILE_LINK_LEFT:						
		case TILE_LINK_RIGHT:																
		case TILE_SOLID_FIXED:
			avatar_AddPoint(player, tile_tile[map.level[x][y]-1].HitPoint);
			map.level_hit[x][y] = 0;
			break;

		case TILE_MORTAL:
			map.level_hit[x][y] = 0;
			avatar_StartToDie(player);
			break;
		
		default:
			printf("HitTile failed: class undefined!\n");
			exit(EXIT_FAILURE);
			break;
		}
	}
}


static void map_UpdateClear(struct map_update_t *map_update)
{
	assert(map_update);
	
	/* inside ?*/
	if ((map_update_top != map_update) && (map_update_bottom != map_update)) {
		if (map_update->prev) map_update->prev->next = map_update->next;			
		if (map_update->next) map_update->next->prev = map_update->prev;
	}
	
	/* on top ?*/
	if (map_update_top == map_update) {
		map_update_top = map_update->next;
		if (map_update_top) map_update_top->prev = NULL;
	}
	/* on bottom ?*/	
	if (map_update_bottom == map_update) {
		map_update_bottom = map_update->prev;
		if (map_update_bottom) map_update_bottom->next = NULL;		
	}

	free(map_update);

	map_update = NULL;
}


static void map_UpdateClearAll(void)
{
	struct map_update_t *map_update;
	struct map_update_t *map_update_nxt;

	map_update = map_update_top;
	
	while (map_update) {
		map_update_nxt = map_update->next;	
		map_UpdateClear(map_update);
		map_update = map_update_nxt;		
	}

	map_update_top    = NULL;
	map_update_bottom = NULL;
}

static void map_UpdateAdd(unsigned int x, unsigned int y) 
{
	struct map_update_t *top = NULL;

	top = malloc(sizeof(struct map_update_t));

	assert(top);
	top->next = map_update_top;
	top->prev = NULL;
	
	if (map_update_top) {
		map_update_top->prev = top;
	} else {
		map_update_bottom = top;	
	}		
	map_update_top = top;
	
	map_update_top->x = x;
	map_update_top->y = y;	

}

static SDL_bool map_Updating(unsigned int x, unsigned int y) 
{
	struct map_update_t *map_update     = NULL;
	struct map_update_t *map_update_nxt = NULL;

	map_update = map_update_top;
	while (map_update) {
		map_update_nxt = map_update->next;	
		if ((map_update->x == x) && (map_update->y == y)) {
			return SDL_TRUE;
		}

		map_update = map_update_nxt;		
	}
	return SDL_FALSE;
}


void map_SetTime(long int time) 
{
	if (time == 0) {
		map.timer = map.timer_init; 
	} else {
		map.timer = time; 	
	}
}


