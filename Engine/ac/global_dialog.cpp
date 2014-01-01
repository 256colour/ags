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

#include "ac/global_dialog.h"
#include "ac/common.h"
#include "ac/dialog.h"
#include "debug/debug_log.h"
#include "debug/debugger.h"
#include "game/game_objects.h"
#include "script/script.h"

ScriptPosition last_in_dialog_request_script_pos;
void RunDialog(int tum) {
    if ((tum<0) | (tum>=game.DialogCount))
        quit("!RunDialog: invalid topic number specified");

    can_run_delayed_command();

    if (play.StopDialogAtEnd != DIALOG_NONE) {
        if (play.StopDialogAtEnd == DIALOG_RUNNING)
            play.StopDialogAtEnd = DIALOG_NEWTOPIC + tum;
        else
            quitprintf("!RunDialog: two NewRoom/RunDialog/StopDialog requests within dialog; last was called in \"%s\", line %d",
                        last_in_dialog_request_script_pos.Section.GetCStr(), last_in_dialog_request_script_pos.Line);
        return;
    }

    get_script_position(last_in_dialog_request_script_pos);

    if (inside_script) 
        curscript->queue_action(ePSARunDialog, tum, "RunDialog");
    else
        do_conversation(tum);
}


void StopDialog() {
  if (play.StopDialogAtEnd == DIALOG_NONE) {
    debug_log("StopDialog called, but was not in a dialog");
    DEBUG_CONSOLE("StopDialog called but no dialog");
    return;
  }
  get_script_position(last_in_dialog_request_script_pos);
  play.StopDialogAtEnd = DIALOG_STOP;
}

void SetDialogOption(int dlg,int opt,int onoroff) {
  if ((dlg<0) | (dlg>=game.DialogCount))
    quit("!SetDialogOption: Invalid topic number specified");
  if ((opt<1) | (opt>dialog[dlg].OptionCount))
    quit("!SetDialogOption: Invalid option number specified");
  opt--;

  dialog[dlg].Options[opt].Flags&=~Common::kDialogOption_IsOn;
  if ((onoroff==1) & ((dialog[dlg].Options[opt].Flags & Common::kDialogOption_IsPermanentlyOff)==0))
    dialog[dlg].Options[opt].Flags|=Common::kDialogOption_IsOn;
  else if (onoroff==2)
    dialog[dlg].Options[opt].Flags|=Common::kDialogOption_IsPermanentlyOff;
}

int GetDialogOption (int dlg, int opt) {
  if ((dlg<0) | (dlg>=game.DialogCount))
    quit("!GetDialogOption: Invalid topic number specified");
  if ((opt<1) | (opt>dialog[dlg].OptionCount))
    quit("!GetDialogOption: Invalid option number specified");
  opt--;

  if (dialog[dlg].Options[opt].Flags & Common::kDialogOption_IsPermanentlyOff)
    return 2;
  if (dialog[dlg].Options[opt].Flags & Common::kDialogOption_IsOn)
    return 1;
  return 0;
}
