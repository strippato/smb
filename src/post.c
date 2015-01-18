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
#include <math.h>

#include "global.h"
#include "graph.h"
/* debug purpose */
/*#include "app.h" */
/*#include "input.h"*/

#include "post.h"

#define POST_MOSAIC_SIZE	24
#define POST_MOSAIC_TTL		2
#define POST_BW_TTL		350

static unsigned int post_mosaic_size = POST_MOSAIC_SIZE;
static enum post_effect_t post_effect = POST_NONE;
static unsigned int post_mosaic_ttl = 0;
static unsigned int post_bw_ttl = 0;

void post_Init(void) 
{
	post_effect = POST_NONE;
	post_mosaic_size = POST_MOSAIC_SIZE;
	post_mosaic_ttl = POST_MOSAIC_TTL;
	post_bw_ttl = POST_BW_TTL;
}

void post_Free(void) 
{
	post_Clear();
}

void post_Clear(void) 
{
	post_effect = POST_NONE;
	post_mosaic_size = POST_MOSAIC_SIZE;
	post_mosaic_ttl = POST_MOSAIC_TTL;
	post_bw_ttl = POST_BW_TTL;
}

void post_Set(enum post_effect_t effect)
{
	post_effect = effect;
	post_mosaic_size = POST_MOSAIC_SIZE;
	post_mosaic_ttl = POST_MOSAIC_TTL;
	post_bw_ttl = POST_BW_TTL;
}

void post_Tick(void)
{
	switch (post_effect) {
	case POST_NONE:
		break;
	case POST_MOSAIC:
		if (post_mosaic_ttl > 0) {
			post_mosaic_ttl--;
		} else {
			if (post_mosaic_size > 0) {
				post_mosaic_size--;
				post_mosaic_ttl = POST_MOSAIC_TTL;
			} else {
				post_effect = POST_NONE;
				post_mosaic_ttl = POST_MOSAIC_TTL;
			}
		}
		break;
	case POST_BW:
		if (post_bw_ttl > 0) {
			post_bw_ttl--;
		} else {
			post_effect = POST_NONE;
			post_bw_ttl = POST_BW_TTL;
		}
		break;
	default:
		printf("Warning: undefined post_effect in %s(line %i)\n", __FILE__, __LINE__);
		post_effect = POST_NONE;
		post_mosaic_ttl = 0;
		post_bw_ttl = 0;
		break;
	}
}

void post_Blit(void) 
{
	int pixel2;	

	unsigned int xx, yy;
	int x, y, realx, realy, mid;
	float delta, how;
	Uint8  r, g, b;
	Uint32 color, sumr, sumg, sumb;
	
/* debug purpose */
/*	
	if (input_key_system[SDLK_z]==1) pixel--;
	if (input_key_system[SDLK_x]==1) pixel++;
*/
	switch (post_effect) {
	case POST_NONE:
		break;
	case POST_BW:
		if (SDL_MUSTLOCK(graph_screen)) SDL_LockSurface(graph_screen);
		for (x=0; x < graph_RESX; ++x) {
			for (y=0; y < graph_RESY; ++y) {

				color =graph_getpixel(graph_screen, x, y);
				SDL_GetRGB(color, graph_screen->format, &r, &g, &b);
				mid = (r + g + b) / 3;
				how = (float)(POST_BW_TTL - post_bw_ttl) / POST_BW_TTL;
				
				delta = abs(r - mid) * how;
				r += (r>mid? -delta: delta);

				delta = abs(g - mid) * how;
				g += (g>mid? -delta: delta);

				delta = abs(b - mid) * how;
				b += (b>mid? -delta: delta);

				color = SDL_MapRGB(graph_screen->format, r, g, b);
				graph_putpixel(graph_screen, x, y, color);

			}
		}
		if (SDL_MUSTLOCK(graph_screen)) SDL_UnlockSurface(graph_screen);
		break;
	case POST_MOSAIC:
		if (post_mosaic_size <= 1) {
			post_mosaic_size = 1;
			/* maping 1 pixel to 1 pixel = no mapping, so go home */
			return;
		}

		pixel2 = post_mosaic_size * post_mosaic_size;

		if (SDL_MUSTLOCK(graph_screen)) SDL_LockSurface(graph_screen);

		for (x=0; x < graph_RESX; x+= post_mosaic_size) {
			for (y=0; y < graph_RESY; y+= post_mosaic_size) {
				color = sumr = sumg = sumb = 0;
				for (xx=0; xx < post_mosaic_size; ++xx) {
					for (yy=0; yy < post_mosaic_size; ++yy) {
						realx = x+xx;
						realy = y+yy;

						/* don't go outisde the screen */
						if (realx >= graph_RESX) realx = graph_RESX -1;
						if (realy >= graph_RESY) realy = graph_RESY -1;

						color =graph_getpixel(graph_screen, realx, realy);
						SDL_GetRGB(color, graph_screen->format, &r, &g, &b);
						sumr += r; sumg += g; sumb += b;
					}
				}

				r = (Uint8)(sumr/pixel2);
				g = (Uint8)(sumg/pixel2);
				b = (Uint8)(sumb/pixel2);

				for (xx=0; xx < post_mosaic_size; ++xx) {
					for (yy=0; yy < post_mosaic_size; ++yy) {
						realx = x+xx;
						realy = y+yy;

						/* don't go outisde the screen */
						if ((realx < graph_RESX) && (realy < graph_RESY)) {
							color = SDL_MapRGB(graph_screen->format, r, g, b);
							graph_putpixel(graph_screen, realx, realy, color);
						}
					}
				}
			}
		}
		if (SDL_MUSTLOCK(graph_screen)) SDL_UnlockSurface(graph_screen);	
		break;
	default:
		printf("Warning: undefined post_effect in %s(line %i)\n", __FILE__, __LINE__);
		break;
	}
}


