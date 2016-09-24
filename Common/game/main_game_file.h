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
// This unit provides functions for reading main game file into appropriate
// data structures. Main game file contains general game data, such as global
// options, lists of static game entities and compiled scripts modules.
//
//=============================================================================

#ifndef __AGS_CN_GAME__MAINGAMEFILE_H
#define __AGS_CN_GAME__MAINGAMEFILE_H

#include "util/stdtr1compat.h"
#include TR1INCLUDE(memory)
#include "ac/game_version.h"
#include "game/plugininfo.h"
#include "script/cc_script.h"
#include "util/stream.h"
#include "util/string.h"
#include "util/version.h"

struct GameSetupStruct;
struct DialogTopic;
struct ViewStruct;

namespace AGS
{
namespace Common
{

// Error codes for main game file reading
enum MainGameFileError
{
    kMGFErr_NoError,
    kMGFErr_FileNotFound,
    kMGFErr_NoStream,
    kMGFErr_SignatureFailed,
    // separate error given for "too old" format to provide clarifying message
    kMGFErr_FormatVersionTooOld,
    kMGFErr_FormatVersionNotSupported,
    kMGFErr_InvalidNativeResolution,
    kMGFErr_TooManyFonts,
    kMGFErr_TooManySprites,
    kMGFErr_TooManyCursors,
    kMGFErr_InvalidPropertySchema,
    kMGFErr_InvalidPropertyValues,
    kMGFErr_NoGlobalScript,
    kMGFErr_CreateGlobalScriptFailed,
    kMGFErr_CreateDialogScriptFailed,
    kMGFErr_CreateScriptModuleFailed,
    kMGFErr_PluginDataFmtNotSupported,
    kMGFErr_PluginDataSizeTooLarge
};

typedef stdtr1compat::shared_ptr<Stream> PStream;

// MainGameSource defines a successfully opened main game file
struct MainGameSource
{
    // Standart main game file names for 3.* and 2.* games respectively
    static const String DefaultFilename_v3;
    static const String DefaultFilename_v2;
    // Signature of the current game format
    static const String Signature;

    // Name of the asset file
    String              Filename;
    // Savegame format version
    GameDataVersion     DataVersion;
    // Engine version this game was intended for
    Version             EngineVersion;
    // A ponter to the opened stream
    PStream             InputStream;

    MainGameSource();
};

// LoadedGameEntities is meant for keeping objects loaded from the game file.
// Because copying/assignment methods are not properly implemented for some
// of these objects yet, they have to be attached using references to be read
// directly. This is temporary solution that has to be resolved by the future
// code refactoring.
struct LoadedGameEntities
{
    GameSetupStruct        &Game;
    DialogTopic           *&Dialogs;
    ViewStruct            *&Views;
    PScript                 GlobalScript;
    PScript                 DialogScript;
    std::vector<PScript>    ScriptModules;
    std::vector<PluginInfo> PluginInfos;

    // Old dialog support
    std::vector< stdtr1compat::shared_ptr<unsigned char> > OldDialogScripts;
    std::vector<String>     OldSpeechLines;

    LoadedGameEntities(GameSetupStruct &game, DialogTopic *&dialogs, ViewStruct *&views);
};

String             GetMainGameFileErrorText(MainGameFileError err);
// Tells if the given path (library filename) contains main game file
bool               IsMainGameLibrary(const String &filename);
// Opens main game file for reading from an arbitrary file
MainGameFileError  OpenMainGameFile(const String &filename, MainGameSource &src);
// Opens main game file for reading from the asset library (uses default asset name)
MainGameFileError  OpenMainGameFileFromDefaultAsset(MainGameSource &src);
// Reads game data, applies necessary conversions to match current format version
MainGameFileError  ReadGameData(LoadedGameEntities &ents, Stream *in, GameDataVersion data_ver);
// Applies necessary updates, conversions and fixups to the loaded data
// making it compatible with current engine
MainGameFileError  UpdateGameData(LoadedGameEntities &ents, GameDataVersion data_ver);

} // namespace Common
} // namespace AGS

#endif // __AGS_CN_GAME__MAINGAMEFILE_H
