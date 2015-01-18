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

/* wii.h */

#ifdef _WII_MOTE

#ifndef WII_H
#define WII_H

#define WII_MAX_WIIMOTE 4

enum wii_key_t {
	WII_KEY_NONE,
	WII_KEY_UP,
	WII_KEY_DOWN,
	WII_KEY_LEFT,
	WII_KEY_RIGHT,
	WII_KEY_A,
	WII_KEY_B,
	WII_KEY_1,
	WII_KEY_2,
	WII_KEY_HOME,
	WII_KEY_PLUS,
	WII_KEY_MINUS,
	WII_KEY_MAX	
};

struct wii_t { 
	SDL_bool connected;
	SDL_bool bind;
	SDL_bool error;
	
	enum wii_key_t up;
	enum wii_key_t down;
	enum wii_key_t left;
	enum wii_key_t right;
	enum wii_key_t jump;
};

extern int wii_wfound;
extern wiimote_t wii_wmote[WII_MAX_WIIMOTE];
extern struct wii_t wii[WII_MAX_WIIMOTE];

void wii_Init(void);
void wii_Free(void);
void wii_Bind(int wind);
void wii_ClearAll(void);
#endif

#endif

