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

/* graph.h */
#ifndef GRAPH_H
#define GRAPH_H

extern SDL_Surface *graph_screen;
extern int graph_RESX;
extern int graph_RESY;
extern int graph_BPP;

struct graph_vec2_t {
	float x;
	float y;
};

void graph_Init(void);
void graph_Free(void);
inline void graph_Flip(void);
void graph_ScreenShot (void);
void graph_Fade2Black(Uint32 msec);
void graph_ToggleFullScreen(void);
inline Uint32 graph_getpixel(SDL_Surface *surface, int x, int y);
inline void graph_putpixel(SDL_Surface *surface, int x, int y, Uint32 pixel);

#endif

