
#include "game/game_objects.h"

AGS::Common::GameInfo game;
AGS::Common::RoomInfo thisroom;
AGS::Engine::RoomState* room_statuses[MAX_ROOMS];
AGS::Engine::RoomState* croom;
AGS::Engine::RoomState troom;
AGS::Engine::RoomObject* objs;
AGS::Engine::GameState play;
AGS::Engine::GameSetup usetup;

AGS::Common::ObjectArray<CharacterCache> charcache;
AGS::Common::ObjectArray<ObjectCache> objcache;
AGS::Common::Array<MoveList> CharMoveLists;
AGS::Common::Array<MoveList> ObjMoveLists;
