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

/* tile.h */
#ifndef POST_H
#define POST_H

#define POST_DEFAULT 0

enum post_effect_t {
	POST_NONE,
	POST_MOSAIC,
	POST_BW
};

void post_Init(void);
void post_Free(void);
void post_Tick(void);
void post_Blit(void);
void post_Clear(void);
void post_Set(enum post_effect_t effect);

#endif
