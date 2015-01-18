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
#include <assert.h>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>

#include "global.h"
#include "tile.h"
#include "util.h"
#include "sound.h"

#define TILE_MAX 64
#define TILE_LINK_REDUCTION  10   /* link collision, need a small tile-boundingbox:  5 reduction= TILEBB -  5*2 pixel */

static short int tile_idx    = 0; /* level tile map    */
static short int tile_bk_idx = 0; /* bkground tile map */

struct tile_t tile_tile[TILE_MAX];
struct tile_t tile_tile_bk[TILE_MAX];

void tile_Init(void) 
{
	tile_idx    = 0;
	tile_bk_idx = 0;	
}

void tile_Free(void) 
{
	short int i;

	for (i=0; i<tile_idx; ++i) {
		if (tile_tile[i].Tile)  {
			SDL_FreeSurface(tile_tile[i].Tile);
			tile_tile[i].Tile = NULL;	
			tile_tile[i].Anim = SDL_FALSE;
			tile_tile[i].KeyFrame   = 1; 
			tile_tile[i].TickToNext = 0;
			tile_tile[i].TickFrame  = 0;
			tile_tile[i].MaxFrame   = 1;
			tile_tile[i].Class      = TILE_SOLID;			
			
			tile_tile[i].BBoxx = 0;
			tile_tile[i].BBoxy = 0;
			tile_tile[i].BBoxw = TILE_SIZE;
			tile_tile[i].BBoxh = TILE_SIZE;

			if (tile_tile[i].HitSound) {
				sound_SfxDirectUnload(tile_tile[i].HitSound);
			}
			tile_tile[i].HitSound = NULL;
			tile_tile[i].HitPoint = 0;
			
			if (tile_tile[i].LinkSound) {
				sound_SfxDirectUnload(tile_tile[i].LinkSound);
			}
			tile_tile[i].LinkSound = NULL;
			
		}
	}
	tile_idx = 0;

	for (i=0; i<tile_bk_idx; ++i) {
		if (tile_tile_bk[i].Tile)  {
			SDL_FreeSurface(tile_tile_bk[i].Tile);
			tile_tile_bk[i].Tile = NULL;	
			tile_tile_bk[i].Anim = SDL_FALSE;
			tile_tile_bk[i].KeyFrame   = 1; 
			tile_tile_bk[i].TickToNext = 0;
			tile_tile_bk[i].TickFrame  = 0;
			tile_tile_bk[i].MaxFrame   = 1;
			tile_tile_bk[i].MaxFrame   = TILE_SOLID;			

			tile_tile_bk[i].BBoxx = 0;
			tile_tile_bk[i].BBoxy = 0;
			tile_tile_bk[i].BBoxw = TILE_SIZE;
			tile_tile_bk[i].BBoxh = TILE_SIZE;

			if (tile_tile_bk[i].HitSound) {
				sound_SfxDirectUnload(tile_tile_bk[i].HitSound);
			}
			tile_tile_bk[i].HitSound = NULL;
			tile_tile_bk[i].HitPoint = 0;			
			
			if (tile_tile_bk[i].LinkSound) {
				sound_SfxDirectUnload(tile_tile_bk[i].LinkSound);
			}
			tile_tile_bk[i].LinkSound = NULL;
			
		}
	}
	tile_bk_idx = 0;
	
}

void tile_Load(char *TileName, enum tile_class_t class, unsigned int hitpoint, char *HitSndName, long int tick, long int bbx, long int bby, long int bbw, long int bbh)
{
	short int ix, iy;
	SDL_Surface *TmpSrf;
		
	assert(tile_idx < TILE_MAX);
	TmpSrf = IMG_Load(TileName);
	if (!TmpSrf) {
		printf("Can not find tile data: %s\n", SDL_GetError());
		exit(EXIT_FAILURE);
	}
	tile_tile[tile_idx].Tile     = SDL_DisplayFormatAlpha(TmpSrf);
	tile_tile[tile_idx].Class    = class;
	tile_tile[tile_idx].KeyFrame = 1; 
		
	ix = tile_tile[tile_idx].Tile->w / TILE_SIZE;
	iy = tile_tile[tile_idx].Tile->h / TILE_SIZE;

	/* tile is animated? */
	if ((tile_tile[tile_idx].Tile->w != TILE_SIZE) || (tile_tile[tile_idx].Tile->h != TILE_SIZE)) {
		tile_tile[tile_idx].Anim = SDL_TRUE;
		tile_tile[tile_idx].MaxFrame = MAX(ix, iy);		
	} else {
		tile_tile[tile_idx].Anim = SDL_FALSE;
		tile_tile[tile_idx].MaxFrame = 1;
	}
	tile_tile[tile_idx].TickFrame  = tick;
	tile_tile[tile_idx].TickToNext = tile_tile[tile_idx].TickFrame;	

	tile_tile[tile_idx].BBoxx = bbx;
	tile_tile[tile_idx].BBoxy = bby;
	tile_tile[tile_idx].BBoxw = bbw;
	tile_tile[tile_idx].BBoxh = bbh;

	if (HitSndName) {
		tile_tile[tile_idx].HitSound = sound_SfxDirectLoad(HitSndName);
	} else {
		tile_tile[tile_idx].HitSound = NULL;	
	}

	tile_tile[tile_idx].HitPoint = hitpoint;
	
	tile_idx++;
	SDL_FreeSurface(TmpSrf);
}

void tile_bk_Load(char *TileName, long int tick) 
{
	short int ix, iy;
	SDL_Surface *TmpSrf;
		
	assert(tile_bk_idx < TILE_MAX);
	TmpSrf = IMG_Load(TileName);
	if (!TmpSrf) {
		printf("Can not find tile data: %s\n", SDL_GetError());
		exit(EXIT_FAILURE);
	}
	tile_tile_bk[tile_bk_idx].Tile = SDL_DisplayFormatAlpha(TmpSrf);
	tile_tile_bk[tile_bk_idx].KeyFrame = 1; 
		
	ix = tile_tile_bk[tile_bk_idx].Tile->w / TILE_SIZE;
	iy = tile_tile_bk[tile_bk_idx].Tile->h / TILE_SIZE;

	/* tile is animated? */
	if ((tile_tile_bk[tile_bk_idx].Tile->w != TILE_SIZE) || (tile_tile_bk[tile_bk_idx].Tile->h != TILE_SIZE)) {
		tile_tile_bk[tile_bk_idx].Anim = SDL_TRUE;
		tile_tile_bk[tile_bk_idx].MaxFrame = MAX(ix, iy);		
	} else {
		tile_tile_bk[tile_bk_idx].Anim = SDL_FALSE;
		tile_tile_bk[tile_bk_idx].MaxFrame = 1;
	}
	tile_tile_bk[tile_bk_idx].TickFrame  = tick;
	tile_tile_bk[tile_bk_idx].TickToNext = tile_tile_bk[tile_bk_idx].TickFrame;	
	
	tile_tile_bk[tile_bk_idx].BBoxx = 0;
	tile_tile_bk[tile_bk_idx].BBoxy = 0;
	tile_tile_bk[tile_bk_idx].BBoxw = TILE_SIZE;
	tile_tile_bk[tile_bk_idx].BBoxh = TILE_SIZE;

	tile_tile_bk[tile_bk_idx].HitSound  = NULL;
	tile_tile_bk[tile_bk_idx].HitPoint = 0;
	
	tile_bk_idx++;
	SDL_FreeSurface(TmpSrf);
}

void tile_Tick(void)
{
	short int i;
	for (i=0; i<tile_idx; ++i) {
		if (tile_tile[i].Tile && tile_tile[i].Anim)  {
			if (tile_tile[i].TickToNext <= 0) {
				tile_tile[i].TickToNext = tile_tile[i].TickFrame;

				tile_tile[i].KeyFrame++;
				if (tile_tile[i].KeyFrame > tile_tile[i].MaxFrame) {
					tile_tile[i].KeyFrame = 1; 
				}
				
			} else {
				tile_tile[i].TickToNext--;
			}

		}
	}

	for (i=0; i<tile_bk_idx; ++i) {
		if (tile_tile_bk[i].Tile && tile_tile_bk[i].Anim)  {
			if (tile_tile_bk[i].TickToNext <= 0) {
				tile_tile_bk[i].TickToNext = tile_tile_bk[i].TickFrame;

				tile_tile_bk[i].KeyFrame++;
				if (tile_tile_bk[i].KeyFrame > tile_tile_bk[i].MaxFrame) {
					tile_tile_bk[i].KeyFrame = 1; 
				}
				
			} else {
				tile_tile_bk[i].TickToNext--;
			}

		}
	}

}

short int tile_MaxTile(void) {
	return tile_idx;
}

inline void tile_GetBBox(long int i, long int *ax, long int *ay, long int *bx, long int *by) 
{

	*ax = (long int)(tile_tile[i].BBoxx);
	*ay = (long int)(tile_tile[i].BBoxy);

	*bx = *ax + tile_tile[i].BBoxw -1;
	*by = *ay + tile_tile[i].BBoxh -1;

}

inline void tile_GetBBoxLinkX(long int i, long int *ax, long int *ay, long int *bx, long int *by) 
{

	*ax = (long int)(tile_tile[i].BBoxx);
	*ay = (long int)(tile_tile[i].BBoxy);

	*bx = *ax + tile_tile[i].BBoxw -1;
	*by = *ay + tile_tile[i].BBoxh -1;

	/* cut some part of bbox */
	*ax += TILE_LINK_REDUCTION;	
	*bx -= TILE_LINK_REDUCTION;
	
	assert (tile_tile[i].BBoxw - 2 * TILE_LINK_REDUCTION - 1 >= 4 ); /* Tile-link is too small */
}

inline void tile_GetBBoxLinkY(long int i, long int *ax, long int *ay, long int *bx, long int *by) 
{

	*ax = (long int)(tile_tile[i].BBoxx);
	*ay = (long int)(tile_tile[i].BBoxy);

	*bx = *ax + tile_tile[i].BBoxw -1;
	*by = *ay + tile_tile[i].BBoxh -1;

	/* cut some part of bbox */
	*ay += TILE_LINK_REDUCTION;	
	*by -= TILE_LINK_REDUCTION;	

	assert (tile_tile[i].BBoxh - 2 * TILE_LINK_REDUCTION - 1 >= 4 ); /* Tile-link is too small  */
}

