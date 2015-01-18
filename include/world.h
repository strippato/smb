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

/* world.h */
#ifndef WORLD_H
#define WORLD_H

#define WORLD_DIR      "data/map/"
#define WORLD_GFX_DIR  "data/map/gfx/"
#define WORLD_SFX_DIR  "data/map/sfx/"

extern config_t *world_config;
extern config_t world_config_cfg;

void world_Init(char *WorldName);
void world_Free(void);
void world_Tick(void);
void world_Blit(void);
void world_Run (void);

#endif

