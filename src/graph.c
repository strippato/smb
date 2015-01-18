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
#include <time.h>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <assert.h>
#include <libconfig.h>

#include "global.h"
#include "app.h"
#include "graph.h"

#define SS_FONT "data/comic.ttf"

int graph_RESX;
int graph_RESY;
int graph_BPP;
SDL_Surface *graph_screen = NULL;

/* 
Single CPU
       static Uint32 graph_vidflag = SDL_SWSURFACE | SDL_DOUBLEBUF;
Multi CPU 
*/
static Uint32 graph_vidflag = SDL_SWSURFACE | SDL_DOUBLEBUF | SDL_ASYNCBLIT;

void graph_Init(void) 
{
	int i = 0;
	SDL_Surface *icon = NULL;
	
	if (config_lookup_int(app_config, "screen.X", &i)) { 
		graph_RESX = i;
	}
	if (config_lookup_int(app_config, "screen.Y", &i)) { 
		graph_RESY = i;
	}
	if (config_lookup_int(app_config, "screen.BPP", &i)) { 
		graph_BPP  = i;				
	}
	
	if (config_lookup_bool(app_config, "screen.fullscreen", &i)) { 
        	if (i) graph_vidflag |= SDL_FULLSCREEN;
	}

	if (SDL_Init (SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_JOYSTICK) < 0) {
		printf ("Could not initialize SDL: %s\n", SDL_GetError ());
		exit (EXIT_FAILURE);
	}

	atexit (SDL_Quit);

	TTF_Init ();
	atexit (TTF_Quit);

	SDL_WM_SetCaption (TITLE, TITLE);/* Caption */
	SDL_ShowCursor (SDL_DISABLE);  	 /* hide mouse */
	SDL_EnableKeyRepeat (0, SDL_DEFAULT_REPEAT_INTERVAL); /* Keyboard ssshhhhttt! */

	icon = IMG_Load ("smb.png");
	assert(icon);
	SDL_WM_SetIcon (icon, NULL);

	SDL_FreeSurface(icon);
	icon = NULL;
	
	graph_screen = SDL_SetVideoMode (graph_RESX, graph_RESY, graph_BPP, graph_vidflag);
	
	if (!graph_screen) {
		printf ("Could not create a surface: %s\n", SDL_GetError ());
		exit (EXIT_FAILURE);
	}
}

void graph_Free(void) 
{
	graph_RESX = 0;
	graph_RESY = 0;
	graph_BPP  = 0;
	graph_screen = NULL;

}

inline void graph_Flip(void) 
{
	SDL_Flip (graph_screen);	
}

void graph_ScreenShot(void) 
{
	static int ss_idx = 0;
	char ss_name[20], ss_time[25], strtime[50];
	TTF_Font *ss_font;
	SDL_Surface *txt_surface, *ss_surface;

	SDL_Rect rect_txt;
	SDL_Color color_txt_black = {0, 0, 0, 0}, color_txt_white = {255, 255, 200, 0};	
	time_t mytime;
	
	ss_font = TTF_OpenFont(SS_FONT, 12);
	if (!ss_font) {
		printf ("\nCould not find font %s\n", SS_FONT);
		exit (EXIT_FAILURE);
	}

	rect_txt.x = 1;
	rect_txt.y = graph_RESY - 16;	
	rect_txt.w = rect_txt.h = 0;	
	 
	mytime = time (NULL);
	ss_idx++;
	sprintf (ss_name, "sshot%d.bmp", ss_idx);
	memset (ss_time, '\0', 25);
	memcpy (ss_time, ctime (&mytime), 24);
	sprintf (strtime, "<%s %s> %s", TITLE, VERSION, ss_time);
	txt_surface = TTF_RenderText_Blended (ss_font, strtime, color_txt_black);
	SDL_BlitSurface (txt_surface, NULL, graph_screen, &rect_txt);
	SDL_FreeSurface (txt_surface);

	rect_txt.y = graph_RESY - 15;
	rect_txt.x = rect_txt.w = rect_txt.h = 0;
	txt_surface = TTF_RenderText_Blended (ss_font, strtime, color_txt_white);
	SDL_BlitSurface (txt_surface, NULL, graph_screen, &rect_txt);
	ss_surface = SDL_GetVideoSurface();
	SDL_SaveBMP (ss_surface, ss_name);
	TTF_CloseFont (ss_font);
	SDL_FreeSurface (txt_surface);
	SDL_FreeSurface (ss_surface);
}

void graph_Fade2Black(Uint32 msec) 
{
	SDL_Surface *tmp, *top, *bckground;	
	Uint32 now, start, rmask, gmask, bmask, amask;
	int alpha = 0;
	
	start = SDL_GetTicks();

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
	rmask = 0xff000000;
	gmask = 0x00ff0000;
	bmask = 0x0000ff00;
	amask = 0x000000ff;
#else
	rmask = 0x000000ff;
	gmask = 0x0000ff00;
	bmask = 0x00ff0000;
	amask = 0xff000000;
#endif

	tmp = SDL_CreateRGBSurface (SDL_SWSURFACE | SDL_HWACCEL | SDL_RLEACCEL | SDL_DOUBLEBUF, graph_RESX, graph_RESY, graph_BPP, rmask, gmask, bmask, amask);

	top = SDL_DisplayFormat(tmp);
	
	bckground = SDL_DisplayFormat(graph_screen);
	
	SDL_FreeSurface(tmp);
	
	while (alpha <= 255){	

		SDL_SetAlpha(bckground, SDL_SRCALPHA, 255);
		SDL_BlitSurface(bckground, NULL, graph_screen, NULL);
		
		SDL_SetAlpha(top, SDL_SRCALPHA, alpha);
		SDL_BlitSurface(top, NULL, graph_screen, NULL);
		
		graph_Flip();

		now = SDL_GetTicks();
		alpha = (int)((255.0/msec) * (now - start));
	}	
	
	SDL_FreeSurface(top);
	SDL_FreeSurface(bckground);	
}

void graph_ToggleFullScreen(void) 
{
	SDL_WM_ToggleFullScreen(graph_screen);
#ifdef _WIN32
	graph_vidflag ^= SDL_FULLSCREEN;
	graph_screen = SDL_SetVideoMode (graph_RESX, graph_RESY, graph_BPP, graph_vidflag);
#endif
}

inline Uint32 graph_getpixel(SDL_Surface *surface, int x, int y) 
{
	int bpp = surface->format->BytesPerPixel;
	Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;

	switch(bpp) {
	case 1:
		return *p;
	case 2:
		return *(Uint16 *)p;
	case 3:
#if SDL_BYTEORDER == SDL_BIG_ENDIAN		
	  return p[0] << 16 | p[1] << 8 | p[2];
#else
	  return p[0] | p[1] << 8 | p[2] << 16;
#endif
	case 4:
		return *(Uint32 *)p;
	default:
		return 0;       
	}
}

inline void graph_putpixel(SDL_Surface *surface, int x, int y, Uint32 pixel) 
{
	int bpp = surface->format->BytesPerPixel;
	Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;

	switch(bpp) {
	case 1:
		*p = pixel;
		break;
	case 2:
		*(Uint16 *)p = pixel;
		break;
	case 3:

#if SDL_BYTEORDER == SDL_BIG_ENDIAN		
		p[0] = (pixel >> 16) & 0xff;
		p[1] = (pixel >>  8) & 0xff;
		p[2] = (pixel)       & 0xff;
#else
		p[0] = (pixel)       & 0xff;
		p[1] = (pixel >>  8) & 0xff;
		p[2] = (pixel >> 16) & 0xff;
#endif
		break;
	case 4:
		*(Uint32 *)p = pixel;
		break;
	}
}

