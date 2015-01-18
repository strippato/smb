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

/* link.h */
#ifndef LINK_H
#define LINK_H

enum link_facing_to_t {
	LINK_UNDEFINED,
	LINK_UP,
	LINK_DOWN,	
	LINK_LEFT,	
	LINK_RIGHT	
};

struct link_t {
	SDL_bool multilink;
	enum link_facing_to_t src_facing;	
	int src_tile_x;
	int src_tile_y;

	enum link_facing_to_t dst_facing;	
	int dst_tile_x;
	int dst_tile_y;
	
	struct link_t *next;
};

void link_Init(void);
void link_Free(void);
void link_Tick(void);
void link_Add( enum link_facing_to_t src_facing, int src_tile_x, int src_tile_y,
		   enum link_facing_to_t dst_facing, int dst_tile_x, int dst_tile_y, 
		   SDL_bool multilink);
struct link_t *link_Find(int src_tile_x, int src_tile_y);

#endif

