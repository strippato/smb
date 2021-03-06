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

/* camera.h */
#ifndef CAMERA_H
#define CAMERA_H

extern long int camera_x;
extern long int camera_y;

enum camera_look_at_t {
	CAMERA_DEBUG,
	CAMERA_PLAYER_CENTER
};

void camera_Init(void);
void camera_Free(void);
void camera_Tick(void);
void camera_LookAt(enum camera_look_at_t camera_actor);
void camera_Look(void);

#endif

