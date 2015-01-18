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

#include "global.h"
#include "util.h"

inline float posval(float x) 
{
	return (x>0 ? x:0);
}

inline float negval(float x) 
{
	return (x<0 ? x:0);
}

inline SDL_bool Collide(long int ax1, long int ay1, long int ax2, long int ay2,
			      long int bx1, long int by1, long int bx2, long int by2) 
{
	if (by1 > ay2) return SDL_FALSE;	
	if (by2 < ay1) return SDL_FALSE;
	if (bx2 < ax1) return SDL_FALSE;
	if (bx1 > ax2) return SDL_FALSE;

	return SDL_TRUE;
	
}


