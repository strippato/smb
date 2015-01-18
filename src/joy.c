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
#include <libconfig.h>
#include <assert.h>

#include "global.h"
#include "input.h"
#include "joy.h"
#include "app.h"

#define JOYMAX DEV_JOYMAX - DEV_JOYPAD0

struct joy_t joy[JOYMAX];

void joy_Init(void)
{
	int i, keycode;
	config_setting_t *setting;
	long int elem;
	
	for (i = 0; i < JOYMAX; ++i) {
		joy[i].joy = NULL;
	}
	
	setting = config_lookup(app_config, "input.joypad");
	assert(setting != NULL);

	elem = config_setting_length(setting);
	assert(elem != 0);

	assert(elem <= JOYMAX);	

	for(i = 0; i < elem; ++i) {
		
		config_setting_t *elem = config_setting_get_elem(setting, i);

		assert(config_setting_lookup_int(elem, "JOY_X", &keycode));
		joy[i].x = keycode;
		
		assert(config_setting_lookup_int(elem, "JOY_Y", &keycode));		
		joy[i].y = keycode;		

		assert(config_setting_lookup_int(elem, "JOY_JUMP", &keycode));		
		joy[i].jump = keycode;		

		assert(config_setting_lookup_int(elem, "THRESHOLD", &keycode));		
		joy[i].threshold = keycode;		

		joy[i].joy = NULL;		
	}
	SDL_JoystickEventState(SDL_DISABLE);
}

void joy_Free(void)
{
	int i;

	SDL_JoystickEventState(SDL_DISABLE);
	for (i = 0; i < JOYMAX; ++i) {
		if (joy[i].joy) {
			/* 
			Yes, valgrind complain about this and blah blah blah...
			==6701== Source and destination overlap in memcpy(0x515cb90, 0x515cb98, 16)
			==6701==    at 0x4A06A3A: memcpy (mc_replace_strmem.c:497)
			==6701==    by 0x374E834887: SDL_JoystickClose (in /usr/lib64/libSDL-1.2.so.0.11.2)
			
			SDL_joystick.c:
			void SDL_JoystickClose(SDL_Joystick *joystick)
			{
				...
				...
			
				 * Remove joystick from list *
				for ( i=0; SDL_joysticks[i]; ++i ) {
					if ( joystick == SDL_joysticks[i] ) {
						memcpy(&SDL_joysticks[i], &SDL_joysticks[i+1],
						       (SDL_numjoysticks-i)*sizeof(joystick));
						break;
					}
				}
				...
				...
			}
			*/
			
			SDL_JoystickClose(joy[i].joy);
			joy[i].joy = NULL;
		}
	}
}

void joy_Bind(int jind)
{
	SDL_JoystickEventState(SDL_ENABLE);
	if (SDL_NumJoysticks() > jind) {
		if (!SDL_JoystickOpened(jind)) {
			joy[jind].joy = SDL_JoystickOpen(jind);

			if (!joy[jind].joy) {
				printf("Can't open joystick(%i):%s\n", jind, SDL_GetError());
				exit(EXIT_FAILURE);
			} else {
				printf("joystick(%i) detected [%s]\n", jind, SDL_JoystickName(jind));
			}
		} else {
			printf("joystick(%i) already open [%s]\n", jind, SDL_JoystickName(jind));
		}
	} else {
		printf("joystick(%i/%i) not found\n", jind, SDL_NumJoysticks());
		exit(EXIT_FAILURE);
	}

}

