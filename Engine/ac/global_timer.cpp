
#include "ac/global_timer.h"
#include "acrun/ac_rundefines.h"
#include "ac/ac_common.h"
#include "acrun/ac_gamestate.h"

extern GameState play;

void script_SetTimer(int tnum,int timeout) {
    if ((tnum < 1) || (tnum >= MAX_TIMERS))
        quit("!StartTimer: invalid timer number");
    play.script_timers[tnum] = timeout;
}
