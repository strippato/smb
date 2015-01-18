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

/* map.h */
#ifndef MAP_H
#define MAP_H

enum map_anchor_t {
	NE, 
	SE, 
	SW, 
	NW		
};

struct map_t {
	/* size */
	int size_w; /* map X */
	int size_h; /* map Y */

	/* timer */
	long int timer;
	long int timer_hurryup;
	long int timer_init;	
	
	/* respawn */	
	long int respawn_x;
	long int respawn_y;
	enum avatar_lookat_t respawn_lookat;
	
	/* background rgb color */
	unsigned char bkground_R;
	unsigned char bkground_G; 
	unsigned char bkground_B;

	/* level background */
	SDL_Surface *bkground;       /* sprite background */
	unsigned int bkground_speed; /* sprite background speed */
	enum map_anchor_t bkground_anchor;
	
	/* level data */
	unsigned char **level;

	/* level foreground */
	unsigned char **level_fg;

	/* level hit-state */	
	long int  **level_hit; 
	short int **level_hitby; /* Hit by player */
};

extern struct map_t map;

void map_Init(void);
void map_Free(void);
void map_Blit(void);
void map_Tick(void);
void map_SetMap(long int x, long int y, int tile);
void map_SetTime(long int time);
void map_HitTile(int player, long int x, long int y);
SDL_bool map_SolidCollision(int player, long int ax1, long int ay1, long int ax2, long int ay2);
SDL_bool map_TestSolidCollision(long int ax1, long int ay1, long int ax2, long int ay2);
SDL_bool map_CanWalkOn(long int ax1, long int ay1, long int ax2, long int ay2);
SDL_bool map_UnsolidCollision(int player, long int ax1, long int ay1, long int ax2, long int ay2);

struct link_t *map_HyperLinkUp   (long int ax1, long int ay1, long int ax2, long int ay2);
struct link_t *map_HyperLinkDown (long int ax1, long int ay1, long int ax2, long int ay2);
struct link_t *map_HyperLinkLeft (long int ax1, long int ay1, long int ax2, long int ay2);
struct link_t *map_HyperLinkRight(long int ax1, long int ay1, long int ax2, long int ay2);

#endif

