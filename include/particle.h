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
#ifndef PARTICLE_H
#define PARTICLE_H

#define PARTICLE_GFX_DIR	"data/particle/"
#define PARTICLE_GFX_STAR	"star.png"
#define PARTICLE_GFX_BREAK	"break.png"
#define PARTICLE_GFX_FOG	"fog.png"

enum particle_class_t {
	PARTICLE_UNDEFINED,
	PARTICLE_STAR,
	PARTICLE_BREAK,
	PARTICLE_FOG
};


void particle_Init(void);
void particle_Free(void);
void particle_Clear(void);
void particle_Tick(void);
void particle_Blit(void);
void particle_Spawn     (enum particle_class_t class, struct graph_vec2_t posv, struct graph_vec2_t velv);
void particle_MultiSpawn(enum particle_class_t class, struct graph_vec2_t posv, struct graph_vec2_t velv);

#endif

