
#include "ac/dynobj/cc_audioclip.h"
#include "ac/ac_audioclip.h"
#include "wgt2allg.h"
#include "ac/ac_gamesetupstruct.h"

extern GameSetupStruct game;

const char *CCAudioClip::GetType() {
    return "AudioClip";
}

int CCAudioClip::Serialize(const char *address, char *buffer, int bufsize) {
    ScriptAudioClip *ach = (ScriptAudioClip*)address;
    StartSerialize(buffer);
    SerializeInt(ach->id);
    return EndSerialize();
}

void CCAudioClip::Unserialize(int index, const char *serializedData, int dataSize) {
    StartUnserialize(serializedData, dataSize);
    int id = UnserializeInt();
    ccRegisterUnserializedObject(index, &game.audioClips[id], this);
}
