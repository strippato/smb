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

/* tile.h */
#ifndef TILE_H
#define TILE_H

#define TILE_SIZE 32

enum tile_class_t {
	TILE_UNDEFINED,
	TILE_SOLID,
	TILE_SOLID_BREAKABLE,
	TILE_SOLID_FIXED,
	TILE_MORTAL,	
	TILE_COIN,
	TILE_LINK_UP,
	TILE_LINK_DOWN,
	TILE_LINK_LEFT,
	TILE_LINK_RIGHT
};

struct tile_t {
	SDL_Surface *Tile;
	enum tile_class_t Class;
	SDL_bool Anim;
	short int KeyFrame; 
	unsigned short int TickToNext;
	unsigned short int TickFrame;
	unsigned short int MaxFrame;

	/* BBOX */
	unsigned short int BBoxx;
	unsigned short int BBoxy;
	unsigned short int BBoxw;
	unsigned short int BBoxh;
	
	Mix_Chunk *HitSound;
	unsigned int HitPoint;		
	
	/* only for link class*/
	Mix_Chunk *LinkSound;
};

extern struct tile_t tile_tile[];
extern struct tile_t tile_tile_bk[];

void tile_Init(void);
void tile_Free(void);
void tile_Load(char *TileName, enum tile_class_t class, unsigned int hitpoint, char *HitSndName, long int tick, long int bbx, long int bby, long int bbw, long int bbh);
void tile_bk_Load(char *TileName, long int tick);
void tile_Tick(void);
short int tile_MaxTile(void);
inline void tile_GetBBox(long int i, long int *ax, long int *ay, long int *bx, long int *by);
inline void tile_GetBBoxLinkX(long int i, long int *ax, long int *ay, long int *bx, long int *by);
inline void tile_GetBBoxLinkY(long int i, long int *ax, long int *ay, long int *bx, long int *by);
 
#endif

