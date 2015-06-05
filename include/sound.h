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

/* sound.h */
#ifndef SOUND_H
#define SOUND_H

typedef enum
{
		SND_NO_MUSIC,
		SND_NORMAL_MUSIC,
		SND_HURRYUP_MUSIC,
		SND_FAST_MUSIC
} sound_music_t;

typedef enum {
	SND_JUMP_01, SND_JUMP_02, SND_JUMP_03, SND_JUMP_04,
	SND_STEP_01, SND_STEP_02, SND_STEP_03, SND_STEP_04,
	SND_LIFEUP,
	SND_DEAD,
	SND_LINK,
	SND_MAX
} sound_sfx_t;

void sound_Init(void);
void sound_Free(void);
void sound_Tick(void);
void sound_Stop(void);
void sound_StopMusic(void);
void sound_StopSfx(void);
void sound_MusicLoad(void);
void sound_MusicUnload(void);
void sound_MusicPlay(sound_music_t music);
void sound_SfxLoad(sound_sfx_t sfx, char *filename);
void sound_SfxUnload(sound_sfx_t sfx);
void sound_SfxPlay(sound_sfx_t sfx);
void sound_SfxPlayTimed(sound_sfx_t sfx, int ticks);

Mix_Chunk *sound_SfxDirectLoad(const char *filename);
void sound_SfxDirectUnload(Mix_Chunk *sound);
void sound_SfxDirectPlay(Mix_Chunk *sound);

Mix_Music *sound_MusicDirectLoad(const char *filename);
void sound_MusicDirectUnload(Mix_Music *music);
void sound_MusicDirectPlay(Mix_Music *music);
void sound_MusicFade(Uint32 msec);
#endif

