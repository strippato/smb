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

#ifdef _WII_MOTE

#include <stdlib.h>
#include <assert.h>
#include <SDL.h>
#include <libconfig.h>
#include <wiimote_api.h>

#include "global.h"
#include "input.h"
#include "wii.h"
#include "joy.h"
#include "app.h"

/* Compile: gcc wiitest.c -I/usr/include/libcwiimote -lcwiimote -lbluetooth -lm */

wiimote_t wii_wmote[WII_MAX_WIIMOTE];
struct wii_t wii[WII_MAX_WIIMOTE];

int wii_wfound = 0;

/* forward */
static void wii_Open(void);
static void wii_Close(int i);
static void wii_CloseAll(void);
static void wii_Clear(int i);

void wii_Init(void)
{
	int i;
	config_setting_t *setting;
	long int elem, keycode;

	assert(wii_wfound == 0); /* call wii_Free() */
	wii_wfound = 0;
	wii_ClearAll();
	wii_Open();

	
	setting = config_lookup(app_config, "input.wiimote");
	assert(setting != NULL);

	elem = config_setting_length(setting);

	assert(elem <= WII_MAX_WIIMOTE);

	for(i = 0; i < elem; ++i) {
		
		config_setting_t *elem = config_setting_get_elem(setting, i);

		wii[i].up = WII_KEY_NONE;
		wii[i].down = WII_KEY_NONE;
		wii[i].left = WII_KEY_NONE;
		wii[i].right = WII_KEY_NONE;
		wii[i].jump = WII_KEY_NONE;

		assert(config_setting_lookup_int(elem, "KEY_UP", &keycode));
		wii[i].up= keycode;

		assert(config_setting_lookup_int(elem, "KEY_DOWN", &keycode));
		wii[i].down = keycode;

		assert(config_setting_lookup_int(elem, "KEY_LEFT", &keycode));
		wii[i].left = keycode;

		assert(config_setting_lookup_int(elem, "KEY_RIGHT", &keycode));
		wii[i].right = keycode;

		assert(config_setting_lookup_int(elem, "KEY_JUMP", &keycode));
		wii[i].jump = keycode;

	}

}

void wii_Free(void)
{
	wii_CloseAll();
	wii_wfound = 0;
}

void wii_Bind(int wind)
{
	if (wind >= wii_wfound) {
		printf("Wiimote[%i] was never found!\n", wind);	
		wii[wind].error = SDL_TRUE;		
		exit(EXIT_FAILURE);		
	} else if (!wii[wind].connected) {
		printf("Wiimote[%i] was never connected!\n", wind);
		wii[wind].error = SDL_TRUE;				
		exit(EXIT_FAILURE);						
	} else if (!wiimote_is_open(&wii_wmote[wind])) {
		printf("Wiimote[%i] is closed!\n", wind);		
		wii[wind].error = SDL_TRUE;
		exit(EXIT_FAILURE);						
	} else if (wii[wind].error){
		printf("Wiimote[%i] is error closed!\n", wind);				
		wii[wind].error = SDL_TRUE;
		exit(EXIT_FAILURE);		
	} else if (wii[wind].bind){
		printf("Wiimote[%i] already bind!\n", wind);				
		wii[wind].error = SDL_TRUE;	
	} else {
		wii[wind].bind = SDL_TRUE;
	}
}

static void wii_Close(int i) 
{
	if wiimote_is_open(&wii_wmote[i]) {
		wiimote_disconnect(&wii_wmote[i]);
	}
	wii_Clear(i);
	wii[i].connected = SDL_FALSE;
}

static void wii_CloseAll(void) 
{
	int i;
	for (i = 0; i < WII_MAX_WIIMOTE; ++i) {
		wii_Close(i);
	}
}

static void wii_Open(void)
{
	int i;
	
	assert(wii_wfound == 0); /* call wii_CloseAll() */
	
	printf("+-------------------------------------+ \n");
	printf("|       Searching for wiimotes        |+\n");
	printf("|   PLEASE PRESS 1+2 ON YOUR WIIMOTE  ||\n");
	printf("+-------------------------------------+|\n");
	printf("  +------------------------------------+\n");
	
	wii_wfound = wiimote_discover(wii_wmote, WII_MAX_WIIMOTE);

	if (wii_wfound <= 0) {
		printf("No wiimotes found\n");
	} else {
		for (i = 0; i < wii_wfound; ++i) {
			printf("Found wiimote[%i][%s]\n", i, wii_wmote[i].link.r_addr);
			if (wiimote_connect(&wii_wmote[i], wii_wmote[i].link.r_addr) < 0) {
				printf("Unable connect wiimote[%i]:%s\n", i, wiimote_get_error());
				wii[i].error = SDL_TRUE;
			} else {
				wii_wmote[i].led.bits  = 1 << i;
				wii_wmote[i].mode.acc  = 0;
				printf("Connected to wiimote[%i][%s] on local[%s]\n", i, wii_wmote[i].link.r_addr, wii_wmote[i].link.l_addr);

				if (wiimote_update(&wii_wmote[i]) < 0) {
					printf("Unable to update wiimote led[%i]: %s\n", i, wiimote_get_error());
					wiimote_disconnect(&wii_wmote[i]);
					wii[i].error = SDL_TRUE;
				} else {
					wii[i].connected = SDL_TRUE;
				}
			}
		}
	}
}

static void wii_Clear(int i)
{
	wii[i].bind = SDL_FALSE;
	wii[i].error = SDL_FALSE;
}

void wii_ClearAll(void)
{
	int i;
	for (i = 0; i < WII_MAX_WIIMOTE; ++i) {
		wii_Clear(i);
	}
}

#endif

