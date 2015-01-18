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

#include "global.h"
#include "link.h"


static struct link_t *link_top = NULL;

void link_Init(void) 
{
	assert(!link_top);
	link_top = NULL;
}

void link_Free(void) 
{
	struct link_t *l;
	struct link_t *l_nxt;	

	l = link_top;
	
	while (l) {
		l_nxt = l->next;
		free(l);
		l = l_nxt;		
	}

	link_top    = NULL;
}

void link_Add( enum link_facing_to_t src_facing, int src_tile_x, int src_tile_y,
		   enum link_facing_to_t dst_facing, int dst_tile_x, int dst_tile_y, 
		   SDL_bool multilink)
{
	struct link_t *l = NULL;
	
	l = malloc(sizeof(struct link_t));
	assert(l);

	assert(src_tile_x >= 0);
	assert(src_tile_y >= 0);	

	assert(dst_tile_x >= 0);
	assert(dst_tile_y >= 0);	

	l->multilink  = multilink;
	
		
	l->src_facing = src_facing;	
	l->src_tile_x = src_tile_x;
	l->src_tile_y = src_tile_y;
	
	l->dst_facing = dst_facing;
	l->dst_tile_x = dst_tile_x;
	l->dst_tile_y = dst_tile_y;
	
	l->next = link_top;
	link_top = l;	
}

void link_Tick(void) 
{
}

struct link_t *link_Find(int src_tile_x, int src_tile_y) 
{
	struct link_t *l;

	l = link_top;
	while (l) {
		if ((src_tile_x == l->src_tile_x) && (src_tile_y == l->src_tile_y)) {
			/* found */
			return l;
		} 
		l = l->next;		
	}
	printf("Warnig: undefined link for tile(%i, %i)\n", src_tile_x, src_tile_y);	
	return NULL;	
}

