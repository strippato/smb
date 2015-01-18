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
#include <SDL_ttf.h>
#include <libconfig.h>

#include "global.h"
#include "graph.h"
#include "input.h"
#include "game.h"
#include "link.h"
#include "avatar.h"
#include "hud.h"

#define HUD_FONT	"data/hud/comic.ttf"
#define HUD_TIME_BKG	"data/hud/hudtime.png"
#define HUD_BKG  	"data/hud/hud.png"

#define HUD_FONT_SIZE 20


#define HUD_LIFE_X    34
#define HUD_LIFE_Y     1

#define HUD_COIN_X    92
#define HUD_COIN_Y     1

#define HUD_SCORE_X  158
#define HUD_SCORE_Y    1

#define HUD_TIME_X     2
#define HUD_TIME_Y     1 

#define HUD_SHADOW_D   2

static TTF_Font    *hud_font;

static SDL_Color hud_color_black = { 0x00, 0x00, 0x00, 0x00};
static SDL_Color hud_color_white = { 0xFF, 0xFF, 0xFF, 0x00};


struct hud_t hud[GAME_MAX_PLAYER];

static long int hud_time;
static SDL_bool hud_time_update;
static SDL_Surface *hud_time_base_panel;
static SDL_Surface *hud_time_panel;

void hud_Init(void) 
{
	int i;
	SDL_Surface *tmp;
	hud_font = TTF_OpenFont(HUD_FONT, HUD_FONT_SIZE);
    	if (!hud_font) {
		printf("Could not find HUD font: %s\n", HUD_FONT);
		exit(EXIT_FAILURE);
	}

	tmp = IMG_Load(HUD_TIME_BKG);
	if (!tmp) {
		printf("Can not find hud data: %s\n", SDL_GetError());
		exit(EXIT_FAILURE);
	}

	hud_time_base_panel = SDL_DisplayFormatAlpha(tmp);
	hud_time_panel      = SDL_DisplayFormatAlpha(tmp);

	SDL_FreeSurface(tmp);
	hud_time_update = SDL_TRUE;
	hud_time  = 0;

	for (i = 0; i < GAME_MAX_PLAYER; ++i) {
		hud[i].update= SDL_TRUE;	
		hud[i].life  = 0;
		hud[i].coin  = 0;
		hud[i].score = 0;						
		hud[i].base_panel = NULL;
		hud[i].panel = NULL;
		
	}
}

void hud_Free(void) 
{
	int i;
	
	TTF_CloseFont(hud_font);	

	if (hud_time_base_panel) SDL_FreeSurface(hud_time_base_panel);
	hud_time_base_panel = NULL;

	if (hud_time_panel) SDL_FreeSurface(hud_time_panel);
	hud_time_panel = NULL;

	hud_time_update = SDL_TRUE;	
	hud_time  = 0;

	for (i = 0; i < GAME_MAX_PLAYER; ++i) {
		hud[i].update  = SDL_TRUE;		
		hud[i].life  = 0;
		hud[i].coin  = 0;
		hud[i].score = 0;						
		if (hud[i].base_panel) SDL_FreeSurface(hud[i].base_panel);
		hud[i].base_panel = NULL;

		if (hud[i].panel) SDL_FreeSurface(hud[i].panel);
		hud[i].panel = NULL;
	}
}

void hud_Blit(void) 
{
	int i, y;
	SDL_Surface *hud_txt;
	SDL_Rect rect;
	char message[20];

	/* TIME */
	if (hud_time_update) {
		hud_time_update = SDL_FALSE;	  		

		if (hud_time_panel) SDL_FreeSurface(hud_time_panel);
		hud_time_panel = NULL;
		hud_time_panel = SDL_DisplayFormatAlpha(hud_time_base_panel);
	  	SDL_BlitSurface(hud_time_base_panel, NULL, hud_time_panel, NULL);
	  			
		sprintf(message, "Time %li", hud_time);
				
		rect.x = HUD_TIME_X + HUD_SHADOW_D;
		rect.y = HUD_TIME_Y + HUD_SHADOW_D;	
		hud_txt = TTF_RenderText_Solid(hud_font, message, hud_color_black);
		SDL_BlitSurface(hud_txt, NULL, hud_time_panel, &rect);
		SDL_FreeSurface(hud_txt);

		rect.x = HUD_TIME_X;
		rect.y = HUD_TIME_Y;
		hud_txt = TTF_RenderText_Solid(hud_font, message, hud_color_white);
		SDL_BlitSurface(hud_txt, NULL, hud_time_panel, &rect);
		SDL_FreeSurface(hud_txt);
	}
	rect.x = graph_RESX/2 - hud_time_panel->w/2;
	rect.y = 0;

	SDL_BlitSurface(hud_time_panel, NULL, graph_screen, &rect);

	
	for (i = 0; i < GAME_MAX_PLAYER; ++i) {
		if (!avatar[i].enabled) continue;
		
		if (hud[i].update) {
			hud[i].update = SDL_FALSE;

			if (hud[i].panel) SDL_FreeSurface(hud[i].panel);
			hud[i].panel = NULL;
			hud[i].panel = SDL_DisplayFormatAlpha(hud[i].base_panel);

			/* LIFE */
			sprintf(message, "%.2i", hud[i].life);
			rect.x = HUD_LIFE_X + HUD_SHADOW_D;
			rect.y = HUD_LIFE_Y + HUD_SHADOW_D;
			hud_txt = TTF_RenderText_Solid(hud_font, message, hud_color_black);
			SDL_BlitSurface(hud_txt, NULL, hud[i].panel, &rect);
			SDL_FreeSurface(hud_txt);

			rect.x = HUD_LIFE_X;
			rect.y = HUD_LIFE_Y;	
			hud_txt = TTF_RenderText_Solid(hud_font, message, hud_color_white);
			SDL_BlitSurface(hud_txt, NULL, hud[i].panel, &rect);
			SDL_FreeSurface(hud_txt);

			/* COIN */
			sprintf(message, "%.2i", hud[i].coin);				
			rect.x = HUD_COIN_X + HUD_SHADOW_D;
			rect.y = HUD_COIN_Y + HUD_SHADOW_D;
			hud_txt = TTF_RenderText_Solid(hud_font, message, hud_color_black);
			SDL_BlitSurface(hud_txt, NULL, hud[i].panel, &rect);
			SDL_FreeSurface(hud_txt);

			rect.x = HUD_COIN_X;
			rect.y = HUD_COIN_Y;
			hud_txt = TTF_RenderText_Solid(hud_font, message, hud_color_white);
			SDL_BlitSurface(hud_txt, NULL, hud[i].panel, &rect);
			SDL_FreeSurface(hud_txt);

			/* SCORE */
			sprintf(message, "%.7li", hud[i].score);		
			rect.x = HUD_SCORE_X + HUD_SHADOW_D;
			rect.y = HUD_SCORE_Y + HUD_SHADOW_D;
			hud_txt = TTF_RenderText_Solid(hud_font, message, hud_color_black);
			SDL_BlitSurface(hud_txt, NULL, hud[i].panel, &rect);
			SDL_FreeSurface(hud_txt);

			rect.x = HUD_SCORE_X;
			rect.y = HUD_SCORE_Y;
			hud_txt = TTF_RenderText_Solid(hud_font, message, hud_color_white);
			SDL_BlitSurface(hud_txt, NULL, hud[i].panel, &rect);
			SDL_FreeSurface(hud_txt);
			
		}

		y = i / 4;
		switch (i%4) {
		case 0:
			rect.x = 0;
			rect.y = y * hud[i].panel->h;
			break;		
		case 1:
			rect.x = graph_RESX - hud[i].panel->w;
			rect.y = y * hud[i].panel->h;	
			break;				
		case 2:
			rect.x = graph_RESX - hud[i].panel->w;
			rect.y = (graph_RESY - hud[i].panel->h) - y * hud[i].panel->h;
			break;				
		case 3:
			rect.x = 0;
			rect.y = (graph_RESY - hud[i].panel->h) - y * hud[i].panel->h;
			break;				
		}

		SDL_BlitSurface(hud[i].panel, NULL, graph_screen, &rect);		
	}

}


void hud_Tick(long int time) 
{
	int i;
	if (hud_time != time) {
		hud_time = time;
		hud_time_update = SDL_TRUE;			
	}
	
	for (i = 0; i < GAME_MAX_PLAYER; ++i) {
		if (avatar[i].enabled) {
			if ((avatar[i].life != hud[i].life) || (avatar[i].coin != hud[i].coin) || (avatar[i].score != hud[i].score)) {		
				hud[i].update = SDL_TRUE;		
				hud[i].life   = avatar[i].life;
				hud[i].coin   = avatar[i].coin;
				hud[i].score  = avatar[i].score;						
			}			
		}			
	}
	
}

void hud_Add(int i) 
{
	SDL_Surface *tmp;

	tmp = IMG_Load(HUD_BKG);
	if (!tmp) {
		printf("Can not find hud data: %s\n", SDL_GetError());
		exit(EXIT_FAILURE);
	}

	hud[i].update = SDL_TRUE;		
	hud[i].life   = avatar[i].life;
	hud[i].coin   = avatar[i].coin;
	hud[i].score  = avatar[i].score;						
	hud[i].base_panel = SDL_DisplayFormatAlpha(tmp);
	hud[i].panel      = SDL_DisplayFormatAlpha(tmp);

	SDL_FreeSurface(tmp);	
}


