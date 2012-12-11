//=============================================================================
//
// Adventure Game Studio (AGS)
//
// Copyright (C) 1999-2011 Chris Jones and 2011-20xx others
// The full list of copyright holders can be found in the Copyright.txt
// file, which is part of this source code distribution.
//
// The AGS source code is provided under the Artistic License 2.0.
// A copy of this license can be found in the file License.txt and at
// http://www.opensource.org/licenses/artistic-license-2.0.php
//
//=============================================================================
//
// Exporting Audio script functions
//
//=============================================================================

// the ^n after the function name is the number of params
// this is to allow an extra parameter to be added in a later
// version without screwing up the stack in previous versions
// (just export both the ^n and the ^m as seperate funcs)

#include "script/symbol_registry.h"

void register_audio_script_functions()
{
    ccAddExternalStaticFunction("Game::IsAudioPlaying^1",(void *)Game_IsAudioPlaying);
    ccAddExternalStaticFunction("Game::SetAudioTypeSpeechVolumeDrop^2", (void*)Game_SetAudioTypeSpeechVolumeDrop);
    ccAddExternalStaticFunction("Game::SetAudioTypeVolume^3", (void*)Game_SetAudioTypeVolume);
    ccAddExternalStaticFunction("Game::StopAudio^1",(void *)Game_StopAudio);

    ccAddExternalStaticFunction("System::get_AudioChannelCount", (void*)System_GetAudioChannelCount);
    ccAddExternalStaticFunction("System::geti_AudioChannels", (void*)System_GetAudioChannels);
}
