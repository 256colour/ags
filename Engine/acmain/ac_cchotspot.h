#ifndef __AC_CCHOTSPOT_H
#define __AC_CCHOTSPOT_H

struct CCHotspot : AGSCCDynamicObject {

    // return the type name of the object
    virtual const char *GetType() {
        return "Hotspot";
    }

    // serialize the object into BUFFER (which is BUFSIZE bytes)
    // return number of bytes used
    virtual int Serialize(const char *address, char *buffer, int bufsize) {
        ScriptHotspot *shh = (ScriptHotspot*)address;
        StartSerialize(buffer);
        SerializeInt(shh->id);
        return EndSerialize();
    }

    virtual void Unserialize(int index, const char *serializedData, int dataSize) {
        StartUnserialize(serializedData, dataSize);
        int num = UnserializeInt();
        ccRegisterUnserializedObject(index, &scrHotspot[num], this);
    }

};

#endif // __AC_CCHOTSPOT_H