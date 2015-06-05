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

/* avatar.h */
#ifndef AVATAR_H
#define AVATAR_H

#define AVATAR_SPRITE0 "data/avatar/avatar0.png"
#define AVATAR_SPRITE1 "data/avatar/avatar1.png"
#define AVATAR_SPRITE2 "data/avatar/avatar2.png"
#define AVATAR_SPRITE3 "data/avatar/avatar3.png"

#define AVATAR_SIZE    32

enum avatar_lookat_t {
	AVATAR_DX,
	AVATAR_SX
};

enum avatar_state_t {
	AVATAR_STAY,
	AVATAR_RUN,
	AVATAR_JUMP,
	AVATAR_LINKIN,
	AVATAR_LINKOUT,
	AVATAR_DEAD
};

struct avatar_t {
	SDL_bool enabled;
	SDL_bool visible;
	SDL_Surface *sprite;
	char *name;

	short int life;
	unsigned long  int score;
	unsigned short int coin;

	float x;
	float y;
	float old_x;
	float old_y;

	float vx;
	float vy;

	float old_vx;
	float old_vy;

	/* avatar 2 avatar reactions */
	float react_vx;
	float react_vy;

	enum input_device_t  device;
	enum avatar_lookat_t lookat;
	enum avatar_state_t state;

	int easy_jump;

	/* animated sprite key frame*/
	float kf_age;
	int   kf;

	/* text */
	SDL_Surface *text;
	SDL_Surface *textbk;
	SDL_Surface *textbox;
	int text_ttl;
	int text_fadein;

	/* afk */
	int afk;

	/* state-related */
	long int isdying;

	/* inlink timer */
	long int linktimer;
	/* link */
	struct link_t *link;
};

extern struct avatar_t avatar[GAME_MAX_PLAYER];

void  avatar_Init(void);
void  avatar_Free(void);
void  avatar_Tick(void);
void  avatar_Blit(void);
float avatar_Get_x (int player);
float avatar_Get_y (int player);
float avatar_Get_vx(int player);
float avatar_Get_vy(int player);
void  avatar_Add(int i, char *name, const char *filename, enum input_device_t device, int life);
void  avatar_PlayerHome(void);
void  avatar_RespawnAll(void);
void  avatar_Respawn(int i);
void  avatar_AddCoin(int player);
void  avatar_AddPoint(int player, unsigned int);
void  avatar_AddLife(int player, int life);
void avatar_Say(int i, char *text, int ttl);
void avatar_SayAll(char *text);
void avatar_Kill(int player);
void avatar_KillAll(void);
SDL_bool avatar_SomeoneIsDying(void);
SDL_bool avatar_SomeoneInLink(void);
int avatar_PlayerInGame(void);
void avatar_StartToDie(int player);
void avatar_StartToDieAll(void);
void avatar_LinkAll(struct link_t *link);
void avatar_Link(int i, struct link_t *link);
int avatar_InGame(void);

#endif

