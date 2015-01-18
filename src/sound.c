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
#include <SDL_mixer.h>
#include <libconfig.h>

#include "global.h"
#include "app.h"
#include "sound.h"
#include "world.h"
#include "input.h"
#include "game.h"
#include "link.h"
#include "avatar.h"
#include "map.h"
#include "assert.h"

#define SND_MUSIC_DIR		"data/sound/music/"
#define SND_SFX_DIR		"data/sound/sfx/"
#define SND_HUP			"hup.ogg"

static SDL_bool sound_enabled = SDL_FALSE;

static Uint16 sound_form = MIX_DEFAULT_FORMAT;

static unsigned int sound_rate = 44100;
static int sound_chan = 2;
static int sound_buff = 4096;
static int sound_music_volume = MIX_MAX_VOLUME;
static int sound_sfx_volume   = MIX_MAX_VOLUME;
static int sound_sfx_chan     = 1;

static Mix_Music *sound_music      = NULL;
static Mix_Music *sound_music_fast = NULL;
static Mix_Music *sound_music_hup  = NULL;

static Mix_Chunk *sound_sfx[SND_MAX];

static sound_music_t sound_current = SND_NO_MUSIC;
static sound_music_t sound_wanted  = SND_NO_MUSIC;

void sound_Init(void) 
{	
	int i = 0;
	double dval = 0;
	
	if (config_lookup_bool(app_config, "sound.enabled", &i)) { 
		sound_enabled = i;
	}

	if (config_lookup_float(app_config, "sound.rate", &dval)) { 
		sound_rate = (unsigned int)dval;
	}
	
	if (config_lookup_int(app_config, "sound.channel", &i)) { 
		sound_chan = i;
	}
	if (config_lookup_int(app_config, "sound.buffer", &i)) { 
		sound_buff = i;
	}
	if (config_lookup_int(app_config, "sound.format", &i)) { 
		sound_form = i;
	}

	if (config_lookup_int(app_config, "sound.music_volume", &i)) { 
		sound_music_volume = i;
	} else {
		sound_music_volume = MIX_MAX_VOLUME;
	}
	if (config_lookup_int(app_config, "sound.sfx_volume", &i)) { 
		sound_sfx_volume = i;
	} else {
		sound_sfx_volume = MIX_MAX_VOLUME;	
	}

	if (config_lookup_int(app_config, "sound.sfx_channel", &i)) { 
		sound_sfx_chan = i;
	} else {
		sound_sfx_chan = 1;
	}
	
	if (sound_enabled) {
		SDL_InitSubSystem(SDL_INIT_AUDIO);
		if (Mix_OpenAudio(sound_rate, sound_form, sound_chan, sound_buff)) {
			printf("OpenAudio failed: %s\n", SDL_GetError());
			exit(EXIT_FAILURE);
		}
		assert(!sound_music);

		sound_music = NULL;
		sound_music_fast = NULL;
		sound_music_hup = Mix_LoadMUS(SND_MUSIC_DIR SND_HUP);

		/* music */
		assert(sound_music_hup); 
		sound_current = SND_NO_MUSIC;
		sound_wanted  = SND_NO_MUSIC;
		Mix_VolumeMusic(sound_music_volume);
		
		/* sfx */
		Mix_AllocateChannels(sound_sfx_chan);
		Mix_Volume(-1, sound_sfx_volume);
		

	}
}

void sound_Free(void) 
{
	int i;
	if (sound_enabled) {
		if (Mix_PlayingMusic()) Mix_HaltMusic();
		
		if (sound_music) Mix_FreeMusic(sound_music);
		if (sound_music_fast) Mix_FreeMusic(sound_music_fast);
		if (sound_music_hup) Mix_FreeMusic(sound_music_hup);
		
		sound_music = NULL;
		sound_music_fast = NULL;
		sound_music_hup  = NULL;

		Mix_HaltChannel(-1);

		/* Unload sfx */
		for (i = 0; i < SND_MAX; ++i ) {
			if (sound_sfx[i]) Mix_FreeChunk(sound_sfx[i]);
			sound_sfx[i] = NULL;
		}
				
		Mix_CloseAudio();	
		SDL_QuitSubSystem(SDL_INIT_AUDIO);
		sound_current = SND_NO_MUSIC;
		sound_wanted  = SND_NO_MUSIC;
		sound_enabled = SDL_FALSE;
		sound_music_volume = MIX_MAX_VOLUME;
		sound_sfx_volume = MIX_MAX_VOLUME;
		sound_sfx_chan = 1;

	}
}

void sound_Tick(void)
{
	if (sound_enabled) {
		
		/* set the wanted music */
		if (avatar_SomeoneIsDying()) {
			/* music off when someone is dying */
			sound_MusicPlay(SND_NO_MUSIC);
		} else {
			if (map.timer > map.timer_hurryup) {
				sound_MusicPlay(SND_NORMAL_MUSIC);
			} else {
				sound_MusicPlay(SND_FAST_MUSIC);
			}
		}
		switch (sound_wanted) {
		case SND_NO_MUSIC:
			if (Mix_PlayingMusic()) Mix_HaltMusic();
			sound_current = SND_NO_MUSIC;
			break;

		case SND_NORMAL_MUSIC:
			if (sound_current != SND_NORMAL_MUSIC) {
				if (Mix_PlayingMusic()) Mix_HaltMusic();
				Mix_PlayMusic(sound_music, -1);
				sound_current = SND_NORMAL_MUSIC;
			}
			break;
			
		case SND_FAST_MUSIC:
			if (sound_current != SND_FAST_MUSIC) {
				if (sound_current == SND_HURRYUP_MUSIC) {
					if (!Mix_PlayingMusic()) {
						if (sound_music_fast) { 
							/* only use if specified in the map file*/
							Mix_PlayMusic(sound_music_fast, -1);
						} else {
							/* fallback to normal music */
							Mix_PlayMusic(sound_music, -1);
						}
						sound_current = SND_FAST_MUSIC;
					}
				} else {
					if (Mix_PlayingMusic()) Mix_HaltMusic();
					Mix_PlayMusic(sound_music_hup, 1);
					sound_current = SND_HURRYUP_MUSIC;
				}
			}
			break;
			
		default:
			break;
		}

	}

}

void sound_MusicLoad(void) 
{
	const char *data = NULL;
	char *fullname;
	
	if (sound_enabled) {
		sound_current = SND_NO_MUSIC;
		sound_wanted  = SND_NO_MUSIC;
		if (Mix_PlayingMusic()) Mix_HaltMusic();
		
		/* avoid memleak */
		if (sound_music) Mix_FreeMusic(sound_music);
		if (sound_music_fast) Mix_FreeMusic(sound_music_fast);

		sound_music = NULL;
		sound_music_fast = NULL;

		if (config_lookup_string(world_config, "map.music.normal", &data)) { 

			fullname = malloc(strlen(SND_MUSIC_DIR) + strlen(data) +1);
			strcpy(fullname, SND_MUSIC_DIR);
			strcat(fullname, data);
			sound_music = Mix_LoadMUS(fullname);
			free(fullname);
		}
		assert(sound_music);
		
		if (config_lookup_string(world_config, "map.music.fast", &data)) { 
			fullname = malloc(strlen(SND_MUSIC_DIR) + strlen(data) +1);
			strcpy(fullname, SND_MUSIC_DIR);
			strcat(fullname, data);
			sound_music_fast = Mix_LoadMUS(fullname);
			free(fullname);
		}

		assert(sound_music);
	}
}

void sound_Stop(void)
{
	sound_StopMusic();
	sound_StopSfx();
}

void sound_StopMusic(void) 
{
	if (sound_enabled) {
		if (Mix_PlayingMusic()) Mix_HaltMusic();
	}
}

void sound_StopSfx(void) 
{
	if (sound_enabled) {
		Mix_HaltChannel(-1);
	}
}

void sound_MusicPlay(sound_music_t music)
{
	sound_wanted = music;
}


void sound_MusicFade(Uint32 msec)
{
	if (sound_enabled) {
		if (Mix_PlayingMusic()) {
			Mix_FadeOutMusic(msec);
		}
	}
}

void sound_SfxLoad(sound_sfx_t sfx, char *filename) 
{
	char *fullname;
	
	if (sound_enabled) {
		Mix_HaltChannel(-1);

		/* avoid memleak */
		if (sound_sfx[sfx]) Mix_FreeChunk(sound_sfx[sfx]);
		sound_sfx[sfx] = NULL;

		fullname = malloc(strlen(SND_SFX_DIR) + strlen(filename) +1);
		strcpy(fullname, SND_SFX_DIR);
		strcat(fullname, filename);
		sound_sfx[sfx] = Mix_LoadWAV(fullname);
		free(fullname);

		assert(sound_sfx[sfx]);

	}
}

void sound_SfxUnload(sound_sfx_t sfx) 
{
	if (sound_enabled) {
		Mix_HaltChannel(-1);
		if (sound_sfx[sfx]) Mix_FreeChunk(sound_sfx[sfx]);
		sound_sfx[sfx] = NULL;
	}
}

void sound_SfxPlay(sound_sfx_t sfx)
{
	if (sound_enabled) {
		Mix_PlayChannel(-1, sound_sfx[sfx], 0);
	}
}

void sound_SfxPlayTimed(sound_sfx_t sfx, int ticks)
{
	if (sound_enabled) {
		Mix_PlayChannelTimed(-1, sound_sfx[sfx], -1 , ticks);

	}
}

void sound_MusicUnload(void) 
{
	if (sound_enabled) {
		if (Mix_PlayingMusic()) Mix_HaltMusic();

		if (sound_music)      Mix_FreeMusic(sound_music);
		if (sound_music_fast) Mix_FreeMusic(sound_music_fast);
		if (sound_music_hup)  Mix_FreeMusic(sound_music_hup);

		sound_music = NULL;
		sound_music_fast = NULL;
		sound_music_hup  = NULL;
	}
}



Mix_Chunk *sound_SfxDirectLoad(const char *filename) 
{
	Mix_Chunk *sound = NULL;
	
	if (sound_enabled) {
		Mix_HaltChannel(-1);
		sound = Mix_LoadWAV(filename);
		if (!sound) {
			printf("Sound Loading failed (%s): %s\n", filename, SDL_GetError());
			exit(EXIT_FAILURE);
		}
		return sound;
	}
	return NULL;
}

void sound_SfxDirectUnload(Mix_Chunk *sound) 
{
	if (sound_enabled) {
		assert(sound);
		Mix_HaltChannel(-1);
		if (sound) Mix_FreeChunk(sound);
	}
}

inline void sound_SfxDirectPlay(Mix_Chunk *sound)
{
	if (sound_enabled) {
		assert(sound);
		if (Mix_PlayChannel(-1, sound, 0) == -1) {
			/* if mixer complain about this, try to add more sfx_channel in smb.cnf*/
			printf("sound_DirectPlay Warnig: %s\n",Mix_GetError());
		}
	}
}

Mix_Music *sound_MusicDirectLoad(const char *filename) 
{
	Mix_Music *music = NULL;
	
	if (sound_enabled) {
		if (Mix_PlayingMusic()) Mix_HaltMusic();	

		if (filename) {
			music = Mix_LoadMUS(filename);

			if (!music) {
				printf("Sound Loading failed (%s): %s\n", filename, SDL_GetError());
				exit(EXIT_FAILURE);
			}
			return music;
		}
	}
	return NULL;

}

void sound_MusicDirectUnload(Mix_Music *music) 
{
	if (sound_enabled) {
		if (Mix_PlayingMusic()) Mix_HaltMusic();
		if (music) Mix_FreeMusic(music);
	}
}

inline void sound_MusicDirectPlay(Mix_Music *music)
{
	if (sound_enabled) {
		if (music) {
			Mix_PlayMusic(music, -1);
		}
	}
}

