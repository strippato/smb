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
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <ctype.h>
#include <math.h>
#include <string.h>

#include "global.h"
#include "outro.h"
#include "graph.h"
#include "sound.h"

#define OUTRO_DATADIR		"data/outro/"
#define OUTRO_MUSIC		OUTRO_DATADIR "outro.ogg"
#define OUTRO_FONT		OUTRO_DATADIR "outfont.png"
#define OUTRO_TOP		OUTRO_DATADIR "top.png"
#define OUTRO_BOTTOM		OUTRO_DATADIR "bottom.png"
#define OUTRO_CREDITS		OUTRO_DATADIR "credits.txt"

#define OUTRO_DELAY		350000 

/* warning:text speed is frame depended*/
#define OUTRO_FPS  		60
#define OUTRO_FPS_TICK	((1.0/OUTRO_FPS) * 1000.0)

#define OUTRO_SIZE_FONT	30

#define OUTRO_FADEOUT		1500

#define OUTRO_BG_TIMER	2000

static char *outro_bg[] = {	OUTRO_DATADIR "outro0.png",
					OUTRO_DATADIR "outro1.png",
					OUTRO_DATADIR "outro2.png",				
					OUTRO_DATADIR "outro3.png",
					OUTRO_DATADIR "outro4.png",
					OUTRO_DATADIR "outro5.png",
					OUTRO_DATADIR "outro6.png",																
					NULL
				};
		  
/* forward declaration */
static SDL_bool outro_gkey(void);
static void outro_wait_next_frame(Uint32 last_frame_tick);

void outro_Run(void)
{
	Mix_Music *music;
	FILE *fcredits;

	double wave = 0;
	double dfactor;
	
	char *text;
	char c = '\0';

	int i, nextline, oldline , scroll, cidx, spacepage;
	int crsrx, crsry, ilen, tmpidx, indx, indy, bg = 0;
	long int textlen;

	Uint32 old_time;	
	Uint32 changebg_timer;
	Uint32 last_frame_tick;
	
	SDL_Rect rect_src, rect_char, dest_char, rect_up, rect_dest;

	SDL_Surface *tmp;
	SDL_Surface *fontmap;
	SDL_Surface *top;
	SDL_Surface *bottom;	
	SDL_Surface *background; 	

	SDL_bool found;

	music = sound_MusicDirectLoad(OUTRO_MUSIC);

	sound_MusicDirectPlay(music);
			
	tmp = IMG_Load(OUTRO_FONT);	
	if (!tmp) {
		printf("Can't find outro data: %s\n", SDL_GetError());
		exit(EXIT_FAILURE);
	}
	fontmap = SDL_DisplayFormatAlpha(tmp);
	SDL_FreeSurface(tmp);

	tmp = IMG_Load(outro_bg[bg]);
	if (!tmp) {
		printf("Can't find outro data: %s\n", SDL_GetError());
		exit(EXIT_FAILURE);
	}
	background = SDL_DisplayFormat(tmp);
	SDL_FreeSurface(tmp);
	
	tmp = IMG_Load(OUTRO_BOTTOM);	
	if (!tmp) {
		printf("Can't find outro data: %s\n", SDL_GetError());
		exit(EXIT_FAILURE);
	}
	bottom = SDL_DisplayFormatAlpha(tmp);
	SDL_FreeSurface(tmp);
	
	tmp = IMG_Load(OUTRO_TOP);		
	if (!tmp) {
		printf("Can't find outro data: %s\n", SDL_GetError());
		exit(EXIT_FAILURE);
	}
	top = SDL_DisplayFormatAlpha(tmp);
	SDL_FreeSurface(tmp);

	spacepage = graph_RESY/OUTRO_SIZE_FONT;
	if (graph_RESY%OUTRO_SIZE_FONT!=0) spacepage++;
	
	if (!(fcredits = fopen(OUTRO_CREDITS, "r"))) {
		printf("Can not find: %s\n", OUTRO_CREDITS);
		exit(EXIT_FAILURE);
	}
	fseek(fcredits, 0L, SEEK_END);
	textlen = ftell(fcredits);
	fseek(fcredits, 0L, SEEK_SET);	

	text = calloc(1, textlen + spacepage +1);
	
	for (i=0; i < spacepage; ++i) {
		text[i] = '\n'; 
	}

	fread(&text[spacepage], 1, textlen, fcredits);
	
	fclose(fcredits);	
	
	last_frame_tick = old_time = SDL_GetTicks();	

	rect_up.x = rect_up.y = 0;
	rect_up.w = graph_RESX;
	rect_up.h = graph_RESY;

	rect_dest.x = 0;
	rect_dest.y = graph_RESY - bottom->h;
	rect_dest.w = rect_dest.h = 0;

	rect_src.x = rect_src.y = 0;
	rect_src.w = graph_RESX;
	rect_src.h = graph_RESY;

	nextline = scroll = cidx = oldline = 0;
	
	changebg_timer = OUTRO_BG_TIMER;		
	/* warning:text speed is frame depended*/
	while ((SDL_GetTicks() < old_time + OUTRO_DELAY) && !(outro_gkey())) { /* OutOfTime or AnyKey */

		SDL_BlitSurface(background, &rect_src, graph_screen, NULL);
		
		outro_wait_next_frame(last_frame_tick);
		last_frame_tick += OUTRO_FPS_TICK;
/*		
		 matrix display blit
		+----..
		|0123 X
		|1
		|2
		|
		|Y
		..
*/		
		crsrx = crsry = 0; /* cursor */
		oldline = cidx;
		found = SDL_FALSE;
		wave -= .12;

		/* lenght of line */
		ilen = 0;
		tmpidx = cidx;
		while (tmpidx < textlen + spacepage) {
			c = text[tmpidx];
			if (c == '\n') {
				break;
			} else {
				ilen++;
			}
			tmpidx++;
		}
		
		while (crsry <= graph_RESY) { 
			/* print if inside */
			c = toupper(text[cidx]);			
			indx = (c-32) % (fontmap->w/OUTRO_SIZE_FONT);
			indy = (c-32) / (fontmap->w/OUTRO_SIZE_FONT);		
			rect_char.x = indx * OUTRO_SIZE_FONT;
			rect_char.y = indy * OUTRO_SIZE_FONT;
			rect_char.h = rect_char.w = OUTRO_SIZE_FONT;

			dfactor = sin(wave+ ((double)crsrx/(double)graph_RESX) * M_PI * 2.0) * 10.0;
			
			/* must center the banner */
			dest_char.x = crsrx + ((graph_RESX - (ilen * OUTRO_SIZE_FONT))/2.0);
			dest_char.y = crsry - scroll + dfactor;
			dest_char.h = dest_char.w = OUTRO_SIZE_FONT;
			
			if ((dest_char.x >= graph_RESX) || (dest_char.x + OUTRO_SIZE_FONT <= 0) || (dest_char.y >= graph_RESY) || (dest_char.y + OUTRO_SIZE_FONT <= 0)) {
				/* out of screen */
			} else {
				SDL_BlitSurface(fontmap, &rect_char, graph_screen, &dest_char);
			}
			if (c != '\n') {
				crsrx += OUTRO_SIZE_FONT;
			} else {
				if (found == SDL_FALSE) {
					nextline = cidx + 1;
					found = SDL_TRUE;
				}
				crsrx  = 0;
				crsry += OUTRO_SIZE_FONT;

				/* lenght of line */
				ilen = 0;
				tmpidx = cidx + 1;
				while (tmpidx < textlen + spacepage) {
					c = text[tmpidx];
					if (c == '\n') {
						break;
					} else {
						ilen++;
					}
					tmpidx++;
				}
			}
			cidx++;
			if (cidx >= textlen + spacepage) {
				if (found == SDL_FALSE) {
					nextline = 0;
					found = SDL_TRUE;
				}

				cidx = crsrx = 0;
				crsry += OUTRO_SIZE_FONT;
				
			}
		}

		scroll++;
		scroll %= OUTRO_SIZE_FONT; 
		if (scroll == 0) {
			cidx = nextline;
		} else {
			cidx = oldline;
		}

		SDL_BlitSurface(top, &rect_up, graph_screen, NULL);
		SDL_BlitSurface(bottom, &rect_up, graph_screen, &rect_dest);

		/* change bg ? */
		if (changebg_timer <= 0) {
			changebg_timer = OUTRO_BG_TIMER;					
			bg++;
			if (!outro_bg[bg]) bg = 0;

			tmp = IMG_Load(outro_bg[bg]);
			if (!tmp) {
				printf("Can't find outro data: %s\n", SDL_GetError());
				exit(EXIT_FAILURE);
			}
			SDL_FreeSurface(background);					
			background = SDL_DisplayFormat(tmp);
			SDL_FreeSurface(tmp);
			
		} else {
			changebg_timer--;
		}

		graph_Flip(); 
	}
	free(text);
	sound_MusicFade(OUTRO_FADEOUT);	
	graph_Fade2Black(OUTRO_FADEOUT);

	SDL_FreeSurface(background);
	SDL_FreeSurface(fontmap);
	SDL_FreeSurface(top);
	SDL_FreeSurface(bottom);
	
	sound_MusicDirectUnload(music);
}

static SDL_bool outro_gkey(void)
{
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		switch (event.type) {
		case SDL_KEYDOWN:
			return SDL_TRUE;
		}
	}
	return SDL_FALSE;	
}

static void outro_wait_next_frame(Uint32 last_frame_tick)
{
	Uint32 delta_tick;

	delta_tick = SDL_GetTicks() - last_frame_tick;
	
	if (delta_tick < OUTRO_FPS_TICK) {
		SDL_Delay(OUTRO_FPS_TICK - delta_tick);
	}
}


