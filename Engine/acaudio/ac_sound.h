/*
** ACSOUND - AGS sound system wrapper
** Copyright (C) 2002-2003, Chris Jones
** All Rights Reserved.
**
** This is UNPUBLISHED PROPRIETARY SOURCE CODE;
** the contents of this file may not be disclosed to third parties,
** copied or duplicated in any form, in whole or in part, without
** prior express permission from Chris Jones.
**
*/

#ifndef __AC_SOUND_H
#define __AC_SOUND_H

#include "acaudio/ac_sounddefines.h"
#include "acaudio/ac_audiodefines.h"
#include "acaudio/ac_soundclip.h"
#include "acaudio/ac_ambientsound.h"

#define MUS_MIDI 1
#define MUS_MP3  2
#define MUS_WAVE 3
#define MUS_MOD  4
#define MUS_OGG  5

SOUNDCLIP *my_load_wave(const char *filename, int voll, int loop);
SOUNDCLIP *my_load_mp3(const char *filname, int voll);
SOUNDCLIP *my_load_static_mp3(const char *filname, int voll, bool loop);
SOUNDCLIP *my_load_static_ogg(const char *filname, int voll, bool loop);
SOUNDCLIP *my_load_ogg(const char *filname, int voll);
SOUNDCLIP *my_load_midi(const char *filname, int repet);
SOUNDCLIP *my_load_mod(const char *filname, int repet);

int  init_mod_player(int numVoices);
void remove_mod_player();

void update_ambient_sound_vol ();

extern AmbientSound ambient[MAX_SOUND_CHANNELS + 1];  // + 1 just for safety on array iterations

#endif // __AC_SOUND_H