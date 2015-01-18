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

/* input.h */
#ifndef INPUT_H
#define INPUT_H

enum input_key_t {
	KEY_NONE, 
	KEY_UP, 
	KEY_RIGHT, 
	KEY_DOWN, 
	KEY_LEFT,
	KEY_JUMP, 	 
        KEY_MAX
};	

enum input_device_t {
	DEV_NONE,
	DEV_KEYBOARD0, DEV_KEYBOARD1, DEV_KEYBOARD2, DEV_KEYBOARD3, DEV_KEYBOARD4, DEV_KEYMAX,
	DEV_JOYPAD0,   DEV_JOYPAD1,   DEV_JOYPAD2,   DEV_JOYPAD3,   DEV_JOYPAD4,   DEV_JOYMAX,
	DEV_WIIMOTE0,  DEV_WIIMOTE1,  DEV_WIIMOTE2,  DEV_WIIMOTE3,  DEV_WIIMAX,    	    
	DEV_MAX
};

struct input_key_config_t {
	SDL_bool enabled;
	enum input_key_t    key;
	enum input_device_t device;
};

extern int input_key[DEV_MAX][KEY_MAX];
extern SDLKey input_mouse_x;
extern SDLKey input_mouse_y;
extern SDLKey input_mouse_btn_l;
extern SDLKey input_mouse_btn_r;
extern SDLKey input_key_system[SDLK_LAST];


extern int input_key_old[DEV_MAX][KEY_MAX];
extern SDLKey input_mouse_x_old;
extern SDLKey input_mouse_y_old;
extern SDLKey input_mouse_btn_l_old;
extern SDLKey input_mouse_btn_r_old;
extern SDLKey input_key_system_old[SDLK_LAST];

void input_Init(void);
void input_Free(void);
void input_Clear(void);

void input_Tick(void);

#endif

