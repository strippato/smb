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

/* game.h */
#ifndef GAME_H
#define GAME_H

#define GAME_MAX_PLAYER  4

extern config_t *game_config;
extern unsigned short int game_player;

enum game_game_state_t {
	GAME_EMPTY           = 0,
	GAME_QUIT            = 1<<0,
	GAME_LEVEL_COMPLETED = 1<<1,
	GAME_LEVEL_TIMEOUT   = 1<<2
};

extern enum game_game_state_t game_game_state;
extern int game_easy_jmp;

void game_Init(void);
void game_Free(void);
void game_Run(char *game_map);
void game_AddPlayer(char *name, const char *filename, enum input_device_t device, int lives);

#endif

