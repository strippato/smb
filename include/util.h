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

/* util.h */
#ifndef UTIL_H
#define UTIL_H

#define SGN(x)   (((x) < 0) ?  -1 : (((x) == 0) ? 0 : 1))
#define MIN(X,Y) ((X) < (Y) ? (X) : (Y))
#define MAX(X,Y) ((X) > (Y) ? (X) : (Y))
#define LIM(X,Y) ((X) > (Y) ? (Y) : (X))
#define RND(x) ((float)x * rand())/(RAND_MAX+1.00)

float posval(float x);
float negval(float x);
SDL_bool Collide(long int ax1, long int ay1, long int ax2, long int ay2,
		        long int bx1, long int by1, long int bx2, long int by2);

#endif

