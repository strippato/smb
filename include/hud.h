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

/* hud.h */
#ifndef HUD_H
#define HUD_H

struct hud_t 
{
	SDL_bool update;
	
	short int life;
	short int coin;
	unsigned long int score;
	
	SDL_Surface *base_panel;
	SDL_Surface *panel;
};

extern struct hud_t hud[GAME_MAX_PLAYER];

void hud_Init(void);
void hud_Free(void);
void hud_Blit(void);
void hud_Tick(long int time);
void hud_Add(int i); 

#endif

