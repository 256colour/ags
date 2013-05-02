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
// A collection of main global game objects
//
//=============================================================================
#ifndef __AGS_EE_GAME__GAME_OBJECTS_H
#define __AGS_EE_GAME__GAME_OBJECTS_H

#include "ac/charactercache.h"
#include "ac/objectcache.h"
#include "ac/movelist.h"
#include "game/gameinfo.h"
#include "game/gamesetup.h"
#include "game/gamestate.h"
#include "game/roominfo.h"
#include "game/roomstate.h"

// static game data
extern AGS::Common::GameInfo game;
// current room's static data
// NOTE: since RoomInfo contains Allegro bitmaps, its contents MUST be released
// before Allegro is deinitialized; so far as RoomInfo is a global object we
// cannot rely on destructor time, a RoomInfo::Free() should be called
// explicitly instead.
extern AGS::Common::RoomInfo thisroom;
// dynamic room data
extern AGS::Engine::RoomState* room_statuses[MAX_ROOMS];
// pointer to the current room's dynamic data
extern AGS::Engine::RoomState* croom;
// used for non-saveable rooms, eg. intro
extern AGS::Engine::RoomState troom;
extern AGS::Engine::RoomObject* objs;
// dynamic game data
extern AGS::Engine::GameState play;
// game runtime configuration
extern AGS::Engine::GameSetup usetup;

// character cache is game-wide
extern AGS::Common::ObjectArray<CharacterCache> charcache;
// object cache is room-wide
extern AGS::Common::ObjectArray<ObjectCache> objcache;
// move-to points
extern AGS::Common::Array<MoveList> mls;

#endif // __AGS_EE_GAME__GAME_OBJECTS_H
