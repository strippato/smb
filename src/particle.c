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
#include <math.h>
#include <assert.h>
#include <SDL.h>
#include <SDL_image.h>
#include <libconfig.h>

#include "global.h"
#include "graph.h"
#include "particle.h"
#include "util.h"
#include "camera.h"

#include "input.h"
#include "game.h"
#include "link.h"
#include "avatar.h"

/* PARTICLE STAR */
#define PARTICLE_STAR_TTL        45       /* time to live in tick */
#define PARTICLE_STAR_TTL_RND    20       /* random ttl */
#define PARTICLE_STAR_FRICTION   0.02f    /* friction */ 
#define PARTICLE_STAR_G          0.04f    /* gforce */
#define PARTICLE_STAR_MULTISPAWN 4        /* how many particle create, in a mutispawn */
#define PARTICLE_STAR_APERTURE   M_PI*2.0 /* angle in radians */


/* PARTICLE BREAK */
#define PARTICLE_BREAK_TTL       100       /* time to live in tick */
#define PARTICLE_BREAK_TTL_RND    30       /* random ttl */
#define PARTICLE_BREAK_FRICTION   0.01f    /* friction */ 
#define PARTICLE_BREAK_G          0.06f    /* gforce */
#define PARTICLE_BREAK_MULTISPAWN 8        /* how many particle create, in a mutispawn */
#define PARTICLE_BREAK_APERTURE   M_PI*2.0 /* angle in radians */


/* PARTICLE FOG */
#define PARTICLE_FOG_TTL        90       /* time to live in tick */
#define PARTICLE_FOG_TTL_RND    60       /* random ttl */
#define PARTICLE_FOG_FRICTION   0.015f   /* friction */ 
#define PARTICLE_FOG_G          0.015f   /* gforce */
#define PARTICLE_FOG_MULTISPAWN 3        /* how many particle create, in a mutispawn */
#define PARTICLE_FOG_APERTURE   M_PI/4.0 /* angle in radians */

struct particle_t {
	SDL_Surface *particle;
	enum particle_class_t class;
	unsigned int ttl, maxttl;
	
	float x;
	float y;

	float vx;
	float vy;
	float friction;
	
	float g;

	unsigned short int keyframe;
	unsigned short int maxframe;
	unsigned short int framesize;			
	
	struct particle_t *next;
	struct particle_t *prev;	
};


static struct particle_t *particle_top    = NULL;
static struct particle_t *particle_bottom = NULL;

static SDL_Surface *particle_star	= NULL;
static SDL_Surface *particle_break	= NULL;
static SDL_Surface *particle_fog	= NULL;

/* forward declaration */
static void particle_Kill(struct particle_t *part);
static void particle_Add_Star (struct graph_vec2_t posv, struct graph_vec2_t velv);
static void particle_Add_Break(struct graph_vec2_t posv, struct graph_vec2_t velv);
static void particle_Add_Fog	(struct graph_vec2_t posv, struct graph_vec2_t velv);

void particle_Init(void) 
{
	SDL_Surface *tmpsrf = NULL;

	/* avoid mem leak */
	assert(!particle_top);
	assert(!particle_bottom);
	assert(!particle_star);
	assert(!particle_break);
	assert(!particle_fog);
	
	particle_top	= NULL;
	particle_bottom	= NULL;
	particle_star	= NULL;
	particle_break	= NULL;	
	particle_fog	= NULL;	
	
	tmpsrf = IMG_Load(PARTICLE_GFX_DIR PARTICLE_GFX_STAR);
	if (!tmpsrf) {
		printf("Can not find particle data: %s\n", SDL_GetError());
		exit(EXIT_FAILURE);
	}
	particle_star = SDL_DisplayFormatAlpha(tmpsrf);
	SDL_FreeSurface(tmpsrf);

	tmpsrf = IMG_Load(PARTICLE_GFX_DIR PARTICLE_GFX_BREAK);
	if (!tmpsrf) {
		printf("Can not find particle data: %s\n", SDL_GetError());
		exit(EXIT_FAILURE);
	}
	particle_break = SDL_DisplayFormatAlpha(tmpsrf);
	SDL_FreeSurface(tmpsrf);

	tmpsrf = IMG_Load(PARTICLE_GFX_DIR PARTICLE_GFX_FOG);
	if (!tmpsrf) {
		printf("Can not find particle data: %s\n", SDL_GetError());
		exit(EXIT_FAILURE);
	}
	particle_fog = SDL_DisplayFormatAlpha(tmpsrf);
	SDL_FreeSurface(tmpsrf);
	
}

void particle_Free(void) 
{
	struct particle_t *particle;
	struct particle_t *particle_nxt;	

	particle = particle_top;
	
	while (particle) {
		particle_nxt = particle->next;
		particle_Kill(particle);
		particle = particle_nxt;		
	}

	particle_top    = NULL;
	particle_bottom = NULL;

	SDL_FreeSurface(particle_star);
	particle_star = NULL;

	SDL_FreeSurface(particle_break);
	particle_break = NULL;

	SDL_FreeSurface(particle_fog);
	particle_fog = NULL;
	
}

void particle_Clear(void)
{
	struct particle_t *particle;
	struct particle_t *particle_nxt;

	particle = particle_top;
	
	while (particle) {
		particle_nxt = particle->next;	
		particle_Kill(particle);
		particle = particle_nxt;		
	}

	particle_top    = NULL;
	particle_bottom = NULL;
}

void particle_Tick(void)
{
	struct particle_t *particle;
	struct particle_t *particle_nxt;
	
	particle = particle_top;
	
	while (particle) {
		particle_nxt = particle->next;
		particle->ttl--;		
		if (particle->ttl > 0)  {
			if (particle->vy > 0) {
				particle->vy = posval(particle->vy - particle->friction);
			}
			if (particle->vy < 0) {
				particle->vy = negval(particle->vy + particle->friction);
			}
			particle->vy = particle->vy + particle->g;

			if (particle->vx > 0) {
				particle->vx = posval(particle->vx - particle->friction);
			}
			if (particle->vx < 0) {
				particle->vx = negval(particle->vx + particle->friction);
			}
						
			particle->x = particle->x + particle->vx;
			particle->y = particle->y + particle->vy;

			if (particle->maxframe > 1) {
				particle->keyframe = (particle->maxttl - particle->ttl) / (particle->maxttl / particle->maxframe);
				particle->keyframe = MIN(particle->keyframe,  particle->maxframe-1);
				assert(particle->keyframe < particle->maxframe);
			}

		} else {
			particle_Kill(particle);
		}

		particle = particle_nxt;		
		
	}

}

void particle_Blit(void) 
{
	struct particle_t *particle;
	SDL_Rect drect, srect;
	
	particle = particle_top;
	
	while (particle) {

		drect.x = particle->x - camera_x;
		drect.y = particle->y - camera_y;
		drect.w = drect.h = 0;

		srect.x = srect.y = 0;		
		if (particle->maxframe > 1) {
			
			if (particle_star->w > particle_star->h) {
				srect.x = particle->keyframe * particle->framesize;
			} else {
				srect.y = particle->keyframe * particle->framesize;
			}
		}
		srect.w = srect.h = particle->framesize;
		
		SDL_BlitSurface(particle->particle, &srect, graph_screen, &drect);
		particle = particle->next;
	}
}


void particle_Spawn(enum particle_class_t class, struct graph_vec2_t posv, struct graph_vec2_t velv) 
{
	switch (class) {
	case PARTICLE_STAR:
		/* try to center */
		posv.x = posv.x - (particle_star->w > particle_star->h ? particle_star->h: particle_star->w)/2.0;
		posv.y = posv.y - (particle_star->w > particle_star->h ? particle_star->h: particle_star->w)/2.0;		
		particle_Add_Star(posv, velv);		
		break;		

	case PARTICLE_BREAK:
		/* try to center */
		posv.x = posv.x - (particle_break->w > particle_break->h ? particle_break->h: particle_break->w)/2.0;
		posv.y = posv.y - (particle_break->w > particle_break->h ? particle_break->h: particle_break->w)/2.0;		
		particle_Add_Break(posv, velv);		
	case PARTICLE_FOG:
		/* try to center */
		posv.x = posv.x - (particle_fog->w > particle_fog->h ? particle_fog->h: particle_fog->w)/2.0;
		posv.y = posv.y - (particle_fog->w > particle_fog->h ? particle_fog->h: particle_fog->w)/2.0;		
		particle_Add_Fog(posv, velv);		
		break;
		
	case PARTICLE_UNDEFINED:
	default:
		printf("Particle class undefined\n");
		exit(EXIT_FAILURE);
		break;
	}

}

void particle_MultiSpawn(enum particle_class_t class, struct graph_vec2_t posv, struct graph_vec2_t velv) 
{
	int i;
	float len, alfa, amin;
	switch (class) {
	case PARTICLE_STAR:
		/* try to center */
		len = sqrtf(velv.x * velv.x + velv.y * velv.y );
		posv.x = posv.x - (particle_star->w > particle_star->h ? particle_star->h: particle_star->w)/2.0;
		posv.y = posv.y - (particle_star->w > particle_star->h ? particle_star->h: particle_star->w)/2.0;

		alfa = atan2(velv.y, velv.x);

		amin = alfa - PARTICLE_STAR_APERTURE/2.0;
		
		for (i=0; i < PARTICLE_STAR_MULTISPAWN; ++i) {
			alfa = amin + RND(PARTICLE_STAR_APERTURE);
			velv.x = len * cos(alfa);
			velv.y = len * sin(alfa);			
			particle_Add_Star(posv, velv);
					
		}
		break;		

	case PARTICLE_BREAK:
		/* try to center */
		len = sqrtf(velv.x * velv.x + velv.y * velv.y );
		posv.x = posv.x - (particle_break->w > particle_break->h ? particle_break->h: particle_break->w)/2.0;
		posv.y = posv.y - (particle_break->w > particle_break->h ? particle_break->h: particle_break->w)/2.0;

		alfa = atan2(velv.y, velv.x);

		amin = alfa - PARTICLE_BREAK_APERTURE/2.0;
		
		for (i=0; i < PARTICLE_BREAK_MULTISPAWN; ++i) {
			alfa = amin + RND(PARTICLE_BREAK_APERTURE);
			velv.x = len * cos(alfa);
			velv.y = len * sin(alfa);			
			particle_Add_Break(posv, velv);
		}
		break;		

	case PARTICLE_FOG:
		/* try to center */
		len = sqrtf(velv.x * velv.x + velv.y * velv.y );
		posv.x = posv.x - (particle_fog->w > particle_fog->h ? particle_fog->h: particle_fog->w)/2.0;
		posv.y = posv.y - (particle_fog->w > particle_fog->h ? particle_fog->h: particle_fog->w)/2.0;

		alfa = atan2(velv.y, velv.x);

		amin = alfa - PARTICLE_FOG_APERTURE/2.0;
		
		for (i=0; i < PARTICLE_FOG_MULTISPAWN; ++i) {
			alfa = amin + RND(PARTICLE_FOG_APERTURE);
			velv.x = len * cos(alfa);
			velv.y = len * sin(alfa);			
			particle_Add_Fog(posv, velv);
		}
		break;
		
	case PARTICLE_UNDEFINED:
	default:
		printf("Particle class undefined\n");
		exit(EXIT_FAILURE);
		break;
	}

}

static void particle_Kill(struct particle_t *part)
{
	assert(part);

	part->particle = NULL;
	
	/* inside ?*/
	if ((particle_top != part) && (particle_bottom != part)) {
		if (part->prev) part->prev->next = part->next;			
		if (part->next) part->next->prev = part->prev;
	}
	
	/* on top ?*/
	if (particle_top == part) {
		particle_top = part->next;
		if (particle_top) particle_top->prev = NULL;
	}
	/* on bottom ?*/	
	if (particle_bottom == part) {
		particle_bottom = part->prev;
		if (particle_bottom) particle_bottom->next = NULL;		
	}
	free(part);

	part = NULL;
}

static void particle_Add_Star(struct graph_vec2_t posv, struct graph_vec2_t velv) 
{
	struct particle_t *top    = NULL;

	top = malloc(sizeof(struct particle_t));

	assert(top);
	top->next = particle_top;
	top->prev = NULL;
	
	if (particle_top) {
		particle_top->prev = top;
	} else {
		particle_bottom = top;	
	}		
	particle_top = top;
	
	particle_top->particle = particle_star;
	particle_top->x     = posv.x;
	particle_top->y     = posv.y;	
	
	particle_top->class = PARTICLE_STAR;
	particle_top->friction = PARTICLE_STAR_FRICTION;			
	particle_top->ttl   = PARTICLE_STAR_TTL + RND(PARTICLE_STAR_TTL_RND);
	particle_top->maxttl= particle_top->ttl;
	particle_top->g     = PARTICLE_STAR_G;
	particle_top->vx    = velv.x;
	particle_top->vy    = velv.y;

	particle_top->framesize = (particle_top->particle->w > particle_top->particle->h ? particle_top->particle->h: particle_top->particle->w);
	particle_top->keyframe = 0;
	particle_top->maxframe = (particle_top->particle->w > particle_top->particle->h ? particle_top->particle->w/particle_top->particle->h: particle_top->particle->h / particle_top->particle->w);

}

static void particle_Add_Break(struct graph_vec2_t posv, struct graph_vec2_t velv) 
{
	struct particle_t *top    = NULL;

	top = malloc(sizeof(struct particle_t));

	assert(top);
	top->next = particle_top;
	top->prev = NULL;
	
	if (particle_top) {
		particle_top->prev = top;
	} else {
		particle_bottom = top;	
	}		
	particle_top = top;
	
	particle_top->particle = particle_break;
	particle_top->x        = posv.x;
	particle_top->y        = posv.y;	
	
	particle_top->class = PARTICLE_BREAK;
	particle_top->friction = PARTICLE_BREAK_FRICTION;	
	particle_top->ttl   = PARTICLE_BREAK_TTL + RND(PARTICLE_BREAK_TTL_RND);
	particle_top->maxttl= particle_top->ttl;
	particle_top->g     = PARTICLE_BREAK_G;
	particle_top->vx    = velv.x;
	particle_top->vy    = velv.y;

	particle_top->framesize = (particle_top->particle->w > particle_top->particle->h ? particle_top->particle->h: particle_top->particle->w);
	particle_top->keyframe = 0;
	particle_top->maxframe = (particle_top->particle->w > particle_top->particle->h ? particle_top->particle->w/particle_top->particle->h: particle_top->particle->h / particle_top->particle->w);

}

static void particle_Add_Fog(struct graph_vec2_t posv, struct graph_vec2_t velv) 
{
	struct particle_t *top    = NULL;

	top = malloc(sizeof(struct particle_t));

	assert(top);
	top->next = particle_top;
	top->prev = NULL;
	
	if (particle_top) {
		particle_top->prev = top;
	} else {
		particle_bottom = top;	
	}		
	particle_top = top;
	
	particle_top->particle = particle_fog;
	particle_top->x        = posv.x;
	particle_top->y        = posv.y;	
	
	particle_top->class = PARTICLE_FOG;
	particle_top->friction = PARTICLE_FOG_FRICTION;	
	particle_top->ttl   = PARTICLE_FOG_TTL + RND(PARTICLE_FOG_TTL_RND);
	particle_top->maxttl= particle_top->ttl;
	particle_top->g     = PARTICLE_FOG_G;
	particle_top->vx    = velv.x;
	particle_top->vy    = velv.y;

	particle_top->framesize = (particle_top->particle->w > particle_top->particle->h ? particle_top->particle->h: particle_top->particle->w);
	particle_top->keyframe = 0;
	particle_top->maxframe = (particle_top->particle->w > particle_top->particle->h ? particle_top->particle->w/particle_top->particle->h: particle_top->particle->h / particle_top->particle->w);

}

