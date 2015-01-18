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
#include <libconfig.h>
#include <SDL.h>
#include <assert.h>

#ifdef _WII_MOTE
#include <wiimote_api.h>
#endif

#include "global.h"
#include "input.h"
#include "joy.h"
#include "wii.h"
#include "app.h"


/* user input keyboard/mouse */
int input_key[DEV_MAX][KEY_MAX];
int input_key_old[DEV_MAX][KEY_MAX];

/* keyboard console */
SDLKey input_key_system[SDLK_LAST];
SDLKey input_key_system_old[SDLK_LAST];

SDLKey input_mouse_x;
SDLKey input_mouse_y;
SDLKey input_mouse_btn_l;
SDLKey input_mouse_btn_r;

SDLKey input_mouse_x_old;
SDLKey input_mouse_y_old;
SDLKey input_mouse_btn_l_old;
SDLKey input_mouse_btn_r_old;

/* keyboard config: map each key to
/			-> enabled
/                       -> device ()
/			-> key (keyup, keyjump...) 
*/
static struct input_key_config_t input_key_config[SDLK_LAST];

void input_Tick(void)
{
	int i, ii;
	SDL_Event event;

#ifdef _WII_MOTE	
	SDL_bool pressed;
#endif

	for (i = 0; i < DEV_MAX; ++i) {
		for (ii = 0; ii < KEY_MAX; ++ii) {
			input_key_old[i][ii] = input_key[i][ii];
		}
	}	
	
	for (i = SDLK_FIRST; i < SDLK_LAST; ++i) input_key_system_old[i] = input_key_system[i];
	
	input_mouse_x_old = input_mouse_x;
	input_mouse_y_old = input_mouse_y;
	input_mouse_btn_l_old = input_mouse_btn_l;
	input_mouse_btn_r_old = input_mouse_btn_r;


	while (SDL_PollEvent (&event)) {
		switch (event.type) {
		/* Keyboard */
		case SDL_KEYDOWN:
			if (input_key_config[event.key.keysym.sym].enabled) {
				input_key[input_key_config[event.key.keysym.sym].device][input_key_config[event.key.keysym.sym].key] = 1; /* Pressed */
			}
			input_key_system[event.key.keysym.sym] = 1;
			break;

		case SDL_KEYUP:
			if (input_key_config[event.key.keysym.sym].enabled) {
				input_key[input_key_config[event.key.keysym.sym].device][input_key_config[event.key.keysym.sym].key] = 0; /* Free */
			}
			input_key_system[event.key.keysym.sym] = 0;
			break;

		/* mouse */
		case SDL_MOUSEMOTION:
			input_mouse_x = event.motion.x;
			input_mouse_y = event.motion.y;
			break;

		case SDL_MOUSEBUTTONUP:
			if (event.button.button == SDL_BUTTON_LEFT)  {
				input_mouse_btn_l = 0;
			}
			if (event.button.button == SDL_BUTTON_RIGHT) {
				input_mouse_btn_r = 0;
			}
			break;

		case SDL_MOUSEBUTTONDOWN:
			if (event.button.button == SDL_BUTTON_LEFT) {
				input_mouse_btn_l = 1;
			}
			if (event.button.button == SDL_BUTTON_RIGHT) {
				input_mouse_btn_r = 1;
			}
			break;

		/* joypad */
		case SDL_JOYBUTTONDOWN:
			if (event.jbutton.button == joy[event.jbutton.which].jump) {
				input_key[DEV_JOYPAD0 + event.jbutton.which][KEY_JUMP] = 1;
			}
			break;

		case SDL_JOYBUTTONUP:
			if (event.jbutton.button == joy[event.jbutton.which].jump) {
				input_key[DEV_JOYPAD0 + event.jbutton.which][KEY_JUMP] = 0;
			}
			break;

		case SDL_JOYAXISMOTION:
			if (event.jaxis.axis == joy[event.jbutton.which].x) {
				/* left */
				if (event.jaxis.value < -joy[event.jaxis.which].threshold) {
					input_key[DEV_JOYPAD0 + event.jaxis.which][KEY_LEFT] = 1;
				} else {
					input_key[DEV_JOYPAD0 + event.jaxis.which][KEY_LEFT] = 0;
				}
				/* right */				
				if (event.jaxis.value > joy[event.jaxis.which].threshold) {
					input_key[DEV_JOYPAD0 + event.jaxis.which][KEY_RIGHT] = 1;
				} else {
					input_key[DEV_JOYPAD0 + event.jaxis.which][KEY_RIGHT] = 0;
				}
				
			}
			
			if (event.jaxis.axis == joy[event.jbutton.which].y) {
				/*  up  */
				if (event.jaxis.value < -joy[event.jaxis.which].threshold) {
					input_key[DEV_JOYPAD0 + event.jaxis.which][KEY_UP] = 1;
				} else {
					input_key[DEV_JOYPAD0 + event.jaxis.which][KEY_UP] = 0;
				}
				/* down */
				if (event.jaxis.value > joy[event.jaxis.which].threshold) {
					input_key[DEV_JOYPAD0 + event.jaxis.which][KEY_DOWN] = 1;
				} else {
					input_key[DEV_JOYPAD0 + event.jaxis.which][KEY_DOWN] = 0;
				}
			}

			break;

		default:
			break;
		}
	}

	
#ifdef _WII_MOTE	
	pressed = SDL_FALSE;
	/* input from wiimote */
	for (i = 0; i < wii_wfound; ++i) {
		if ((!wii[i].connected) || (!wii[i].bind) || (wii[i].error)) continue;

		if (wiimote_is_open(&wii_wmote[i])) {
			if (!wiimote_pending(&wii_wmote[i])) {
				continue;
			}

			if (wiimote_update(&wii_wmote[i]) < 0) {
				printf("Unable to update wiimote[%i]: %s\n", i, wiimote_get_error());
				wiimote_disconnect(&wii_wmote[i]);
			} else {

				/* UP */
				pressed = SDL_FALSE;
				switch (wii[i].up) {
					case WII_KEY_UP:
						pressed = (wii_wmote[i].keys.up ? SDL_TRUE: SDL_FALSE);
						break;
					case WII_KEY_DOWN:
						pressed = (wii_wmote[i].keys.down ? SDL_TRUE: SDL_FALSE);
						break;
					case WII_KEY_LEFT:
						pressed = (wii_wmote[i].keys.left ? SDL_TRUE: SDL_FALSE);
						break;
					case WII_KEY_RIGHT:
						pressed = (wii_wmote[i].keys.right ? SDL_TRUE: SDL_FALSE);
						break;
					case WII_KEY_A:
						pressed = (wii_wmote[i].keys.a ? SDL_TRUE: SDL_FALSE);
						break;
					case WII_KEY_B:
						pressed = (wii_wmote[i].keys.b ? SDL_TRUE: SDL_FALSE);
						break;
					case WII_KEY_1:
						pressed = (wii_wmote[i].keys.one ? SDL_TRUE: SDL_FALSE);
						break;
					case WII_KEY_2:
						pressed = (wii_wmote[i].keys.two ? SDL_TRUE: SDL_FALSE);
						break;
					case WII_KEY_HOME:
						pressed = (wii_wmote[i].keys.home ? SDL_TRUE: SDL_FALSE);
						break;
					case WII_KEY_PLUS:
						pressed = (wii_wmote[i].keys.plus ? SDL_TRUE: SDL_FALSE);
						break;
					case WII_KEY_MINUS:
						pressed = (wii_wmote[i].keys.minus ? SDL_TRUE: SDL_FALSE);
						break;
					default:
						break;
				}
				input_key[DEV_WIIMOTE0 + i][KEY_UP] = (pressed ? 1 : 0);

				/* down */
				pressed = SDL_FALSE;
				switch (wii[i].down) {
					case WII_KEY_UP:
						pressed = (wii_wmote[i].keys.up ? SDL_TRUE: SDL_FALSE);
						break;
					case WII_KEY_DOWN:
						pressed = (wii_wmote[i].keys.down ? SDL_TRUE: SDL_FALSE);
						break;
					case WII_KEY_LEFT:
						pressed = (wii_wmote[i].keys.left ? SDL_TRUE: SDL_FALSE);
						break;
					case WII_KEY_RIGHT:
						pressed = (wii_wmote[i].keys.right ? SDL_TRUE: SDL_FALSE);
						break;
					case WII_KEY_A:
						pressed = (wii_wmote[i].keys.a ? SDL_TRUE: SDL_FALSE);
						break;
					case WII_KEY_B:
						pressed = (wii_wmote[i].keys.b ? SDL_TRUE: SDL_FALSE);
						break;
					case WII_KEY_1:
						pressed = (wii_wmote[i].keys.one ? SDL_TRUE: SDL_FALSE);
						break;
					case WII_KEY_2:
						pressed = (wii_wmote[i].keys.two ? SDL_TRUE: SDL_FALSE);
						break;
					case WII_KEY_HOME:
						pressed = (wii_wmote[i].keys.home ? SDL_TRUE: SDL_FALSE);
						break;
					case WII_KEY_PLUS:
						pressed = (wii_wmote[i].keys.plus ? SDL_TRUE: SDL_FALSE);
						break;
					case WII_KEY_MINUS:
						pressed = (wii_wmote[i].keys.minus ? SDL_TRUE: SDL_FALSE);
						break;
					default:
						break;
				}
				input_key[DEV_WIIMOTE0 + i][KEY_DOWN] = (pressed ? 1 : 0);

				/* left */
				pressed = SDL_FALSE;
				switch (wii[i].left) {
					case WII_KEY_UP:
						pressed = (wii_wmote[i].keys.up ? SDL_TRUE: SDL_FALSE);
						break;
					case WII_KEY_DOWN:
						pressed = (wii_wmote[i].keys.down ? SDL_TRUE: SDL_FALSE);
						break;
					case WII_KEY_LEFT:
						pressed = (wii_wmote[i].keys.left ? SDL_TRUE: SDL_FALSE);
						break;
					case WII_KEY_RIGHT:
						pressed = (wii_wmote[i].keys.right ? SDL_TRUE: SDL_FALSE);
						break;
					case WII_KEY_A:
						pressed = (wii_wmote[i].keys.a ? SDL_TRUE: SDL_FALSE);
						break;
					case WII_KEY_B:
						pressed = (wii_wmote[i].keys.b ? SDL_TRUE: SDL_FALSE);
						break;
					case WII_KEY_1:
						pressed = (wii_wmote[i].keys.one ? SDL_TRUE: SDL_FALSE);
						break;
					case WII_KEY_2:
						pressed = (wii_wmote[i].keys.two ? SDL_TRUE: SDL_FALSE);
						break;
					case WII_KEY_HOME:
						pressed = (wii_wmote[i].keys.home ? SDL_TRUE: SDL_FALSE);
						break;
					case WII_KEY_PLUS:
						pressed = (wii_wmote[i].keys.plus ? SDL_TRUE: SDL_FALSE);
						break;
					case WII_KEY_MINUS:
						pressed = (wii_wmote[i].keys.minus ? SDL_TRUE: SDL_FALSE);
						break;
					default:
						break;
				}
				input_key[DEV_WIIMOTE0 + i][KEY_LEFT] = (pressed ? 1 : 0);
				
				/* right */
				pressed = SDL_FALSE;
				switch (wii[i].right) {
					case WII_KEY_UP:
						pressed = (wii_wmote[i].keys.up ? SDL_TRUE: SDL_FALSE);
						break;
					case WII_KEY_DOWN:
						pressed = (wii_wmote[i].keys.down ? SDL_TRUE: SDL_FALSE);
						break;
					case WII_KEY_LEFT:
						pressed = (wii_wmote[i].keys.left ? SDL_TRUE: SDL_FALSE);
						break;
					case WII_KEY_RIGHT:
						pressed = (wii_wmote[i].keys.right ? SDL_TRUE: SDL_FALSE);
						break;
					case WII_KEY_A:
						pressed = (wii_wmote[i].keys.a ? SDL_TRUE: SDL_FALSE);
						break;
					case WII_KEY_B:
						pressed = (wii_wmote[i].keys.b ? SDL_TRUE: SDL_FALSE);
						break;
					case WII_KEY_1:
						pressed = (wii_wmote[i].keys.one ? SDL_TRUE: SDL_FALSE);
						break;
					case WII_KEY_2:
						pressed = (wii_wmote[i].keys.two ? SDL_TRUE: SDL_FALSE);
						break;
					case WII_KEY_HOME:
						pressed = (wii_wmote[i].keys.home ? SDL_TRUE: SDL_FALSE);
						break;
					case WII_KEY_PLUS:
						pressed = (wii_wmote[i].keys.plus ? SDL_TRUE: SDL_FALSE);
						break;
					case WII_KEY_MINUS:
						pressed = (wii_wmote[i].keys.minus ? SDL_TRUE: SDL_FALSE);
						break;
					default:
						break;
				}
				input_key[DEV_WIIMOTE0 + i][KEY_RIGHT] = (pressed ? 1 : 0);


				/* jump */
				pressed = SDL_FALSE;
				switch (wii[i].jump) {
					case WII_KEY_UP:
						pressed = (wii_wmote[i].keys.up ? SDL_TRUE: SDL_FALSE);
						break;
					case WII_KEY_DOWN:
						pressed = (wii_wmote[i].keys.down ? SDL_TRUE: SDL_FALSE);
						break;
					case WII_KEY_LEFT:
						pressed = (wii_wmote[i].keys.left ? SDL_TRUE: SDL_FALSE);
						break;
					case WII_KEY_RIGHT:
						pressed = (wii_wmote[i].keys.right ? SDL_TRUE: SDL_FALSE);
						break;
					case WII_KEY_A:
						pressed = (wii_wmote[i].keys.a ? SDL_TRUE: SDL_FALSE);
						break;
					case WII_KEY_B:
						pressed = (wii_wmote[i].keys.b ? SDL_TRUE: SDL_FALSE);
						break;
					case WII_KEY_1:
						pressed = (wii_wmote[i].keys.one ? SDL_TRUE: SDL_FALSE);
						break;
					case WII_KEY_2:
						pressed = (wii_wmote[i].keys.two ? SDL_TRUE: SDL_FALSE);
						break;
					case WII_KEY_HOME:
						pressed = (wii_wmote[i].keys.home ? SDL_TRUE: SDL_FALSE);
						break;
					case WII_KEY_PLUS:
						pressed = (wii_wmote[i].keys.plus ? SDL_TRUE: SDL_FALSE);
						break;
					case WII_KEY_MINUS:
						pressed = (wii_wmote[i].keys.minus ? SDL_TRUE: SDL_FALSE);
						break;
					default:
						break;
				}
				input_key[DEV_WIIMOTE0 + i][KEY_JUMP] = (pressed ? 1 : 0);

			}
		}
	}
#endif

}

void input_Clear(void)
{
	int i, ii;

	for (i = 0; i < DEV_MAX; ++i) {		
		for (ii = 0; ii < KEY_MAX; ++ii) {
			input_key_old[i][ii] = input_key[i][ii] = 0;
		}
	}	

	for (i = SDLK_FIRST; i < SDLK_LAST; ++i) input_key_system_old[i] = input_key_system[i] = 0;

	input_mouse_x = input_mouse_x_old = 0;
	input_mouse_y = input_mouse_y_old = 0;
	input_mouse_btn_l = input_mouse_btn_l_old = 0;
	input_mouse_btn_r = input_mouse_btn_r_old = 0;
#ifdef _WII_MOTE	
	wii_ClearAll();
#endif		
}

void input_Init(void)
{
	config_setting_t *setting;
	int keycode;
	long int i, elem;

	input_Clear();
	
	setting = config_lookup(app_config, "input.keyboard");
	assert(setting != NULL);

	elem = config_setting_length(setting);
	assert(elem != 0);
	assert(elem < DEV_KEYMAX);	

	for(i = 0; i < elem; ++i) {
		
		config_setting_t *elem = config_setting_get_elem(setting, i);

		assert((DEV_KEYBOARD0 + i) < DEV_KEYMAX);
			
		assert(config_setting_lookup_int(elem, "KEY_UP", &keycode));
		input_key_config[keycode].enabled = SDL_TRUE;
		input_key_config[keycode].device  = DEV_KEYBOARD0 + i;
		input_key_config[keycode].key = KEY_UP;			

		assert(config_setting_lookup_int(elem, "KEY_DOWN", &keycode));
		input_key_config[keycode].enabled = SDL_TRUE;
		input_key_config[keycode].device  = DEV_KEYBOARD0 + i;
		input_key_config[keycode].key = KEY_DOWN;			

		assert(config_setting_lookup_int(elem, "KEY_LEFT", &keycode));
		input_key_config[keycode].enabled = SDL_TRUE;
		input_key_config[keycode].device  = DEV_KEYBOARD0 + i;
		input_key_config[keycode].key = KEY_LEFT;			

		assert(config_setting_lookup_int(elem, "KEY_RIGHT", &keycode));
		input_key_config[keycode].enabled = SDL_TRUE;
		input_key_config[keycode].device  = DEV_KEYBOARD0 + i;
		input_key_config[keycode].key = KEY_RIGHT;			

		assert(config_setting_lookup_int(elem, "KEY_JUMP", &keycode));
		input_key_config[keycode].enabled = SDL_TRUE;
		input_key_config[keycode].device  = DEV_KEYBOARD0 + i;
		input_key_config[keycode].key = KEY_JUMP;			
	
	}
	joy_Init();
}

void input_Free(void)
{
	int i;
	for(i = 0; i < DEV_MAX; ++i) {
		input_key_config[i].enabled = SDL_FALSE;
		input_key_config[i].device  = DEV_NONE;
		input_key_config[i].key = KEY_NONE;
	}
	joy_Free();
	input_Clear();
}


