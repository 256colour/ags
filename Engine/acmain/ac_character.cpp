
#include "acmain/ac_maindefines.h"


void SetCharacterBaseline (int obn, int basel) {
    if (!is_valid_character(obn)) quit("!SetCharacterBaseline: invalid object number specified");

    Character_SetBaseline(&game.chars[obn], basel);
}

// pass trans=0 for fully solid, trans=100 for fully transparent
void SetCharacterTransparency(int obn,int trans) {
    if (!is_valid_character(obn))
        quit("!SetCharTransparent: invalid character number specified");

    Character_SetTransparency(&game.chars[obn], trans);
}

void scAnimateCharacter (int chh, int loopn, int sppd, int rept) {
    if (!is_valid_character(chh))
        quit("AnimateCharacter: invalid character");

    animate_character(&game.chars[chh], loopn, sppd, rept);
}

void AnimateCharacterEx(int chh, int loopn, int sppd, int rept, int direction, int blocking) {
    if ((direction < 0) || (direction > 1))
        quit("!AnimateCharacterEx: invalid direction");
    if (!is_valid_character(chh))
        quit("AnimateCharacter: invalid character");

    if (direction)
        direction = BACKWARDS;
    else
        direction = FORWARDS;

    if (blocking)
        blocking = BLOCKING;
    else
        blocking = IN_BACKGROUND;

    Character_Animate(&game.chars[chh], loopn, sppd, rept, blocking, direction);

}

void animate_character(CharacterInfo *chap, int loopn,int sppd,int rept, int noidleoverride, int direction) {

    if ((chap->view < 0) || (chap->view > game.numviews)) {
        quitprintf("!AnimateCharacter: you need to set the view number first\n"
            "(trying to animate '%s' using loop %d. View is currently %d).",chap->name,loopn,chap->view+1);
    }
    DEBUG_CONSOLE("%s: Start anim view %d loop %d, spd %d, repeat %d", chap->scrname, chap->view+1, loopn, sppd, rept);
    if ((chap->idleleft < 0) && (noidleoverride == 0)) {
        // if idle view in progress for the character (and this is not the
        // "start idle animation" animate_character call), stop the idle anim
        Character_UnlockView(chap);
        chap->idleleft=chap->idletime;
    }
    if ((loopn < 0) || (loopn >= views[chap->view].numLoops))
        quit("!AnimateCharacter: invalid loop number specified");
    Character_StopMoving(chap);
    chap->animating=1;
    if (rept) chap->animating |= CHANIM_REPEAT;
    if (direction) chap->animating |= CHANIM_BACKWARDS;

    chap->animating|=((sppd << 8) & 0xff00);
    chap->loop=loopn;

    if (direction) {
        chap->frame = views[chap->view].loops[loopn].numFrames - 1;
    }
    else
        chap->frame=0;

    chap->wait = sppd + views[chap->view].loops[loopn].frames[chap->frame].speed;
    CheckViewFrameForCharacter(chap);
}


void SetPlayerCharacter(int newchar) {
  if (!is_valid_character(newchar))
    quit("!SetPlayerCharacter: Invalid character specified");

  Character_SetAsPlayer(&game.chars[newchar]);
}

void FollowCharacterEx(int who, int tofollow, int distaway, int eagerness) {
  if (!is_valid_character(who))
    quit("!FollowCharacter: Invalid character specified");
  CharacterInfo *chtofollow;
  if (tofollow == -1)
    chtofollow = NULL;
  else if (!is_valid_character(tofollow))
    quit("!FollowCharacterEx: invalid character to follow");
  else
    chtofollow = &game.chars[tofollow];

  Character_FollowCharacter(&game.chars[who], chtofollow, distaway, eagerness);
}

void FollowCharacter(int who, int tofollow) {
  FollowCharacterEx(who,tofollow,10,97);
  }

void SetCharacterIgnoreLight (int who, int yesorno) {
  if (!is_valid_character(who))
    quit("!SetCharacterIgnoreLight: Invalid character specified");

  Character_SetIgnoreLighting(&game.chars[who], yesorno);
}




void MoveCharacter(int cc,int xx,int yy) {
  walk_character(cc,xx,yy,0, true);
}
void MoveCharacterDirect(int cc,int xx, int yy) {
  walk_character(cc,xx,yy,1, true);
}
void MoveCharacterStraight(int cc,int xx, int yy) {
  if (!is_valid_character(cc))
    quit("!MoveCharacterStraight: invalid character specified");
  
  Character_WalkStraight(&game.chars[cc], xx, yy, IN_BACKGROUND);
}

// Append to character path
void MoveCharacterPath (int chac, int tox, int toy) {
  if (!is_valid_character(chac))
    quit("!MoveCharacterPath: invalid character specified");

  Character_AddWaypoint(&game.chars[chac], tox, toy);
}


int GetPlayerCharacter() {
  return game.playercharacter;
  }

void SetCharacterSpeedEx(int chaa, int xspeed, int yspeed) {
  if (!is_valid_character(chaa))
    quit("!SetCharacterSpeedEx: invalid character");

  Character_SetSpeed(&game.chars[chaa], xspeed, yspeed);

}

void SetCharacterSpeed(int chaa,int nspeed) {
  SetCharacterSpeedEx(chaa, nspeed, nspeed);
}

void SetTalkingColor(int chaa,int ncol) {
  if (!is_valid_character(chaa)) quit("!SetTalkingColor: invalid character");
  
  Character_SetSpeechColor(&game.chars[chaa], ncol);
}

void SetCharacterSpeechView (int chaa, int vii) {
  if (!is_valid_character(chaa))
    quit("!SetCharacterSpeechView: invalid character specified");
  
  Character_SetSpeechView(&game.chars[chaa], vii);
}

void SetCharacterBlinkView (int chaa, int vii, int intrv) {
  if (!is_valid_character(chaa))
    quit("!SetCharacterBlinkView: invalid character specified");

  Character_SetBlinkView(&game.chars[chaa], vii);
  Character_SetBlinkInterval(&game.chars[chaa], intrv);
}

void SetCharacterView(int chaa,int vii) {
  if (!is_valid_character(chaa))
    quit("!SetCharacterView: invalid character specified");
  
  Character_LockView(&game.chars[chaa], vii);
}

void SetCharacterFrame(int chaa, int view, int loop, int frame) {

  Character_LockViewFrame(&game.chars[chaa], view, loop, frame);
}

// similar to SetCharView, but aligns the frame to make it line up
void SetCharacterViewEx (int chaa, int vii, int loop, int align) {
  
  Character_LockViewAligned(&game.chars[chaa], vii, loop, align);
}

void SetCharacterViewOffset (int chaa, int vii, int xoffs, int yoffs) {

  Character_LockViewOffset(&game.chars[chaa], vii, xoffs, yoffs);
}


void ChangeCharacterView(int chaa,int vii) {
  if (!is_valid_character(chaa))
    quit("!ChangeCharacterView: invalid character specified");
  
  Character_ChangeView(&game.chars[chaa], vii);
}

void SetCharacterClickable (int cha, int clik) {
  if (!is_valid_character(cha))
    quit("!SetCharacterClickable: Invalid character specified");
  // make the character clicklabe (reset "No interaction" bit)
  game.chars[cha].flags&=~CHF_NOINTERACT;
  // if they don't want it clickable, set the relevant bit
  if (clik == 0)
    game.chars[cha].flags|=CHF_NOINTERACT;
  }

void SetCharacterIgnoreWalkbehinds (int cha, int clik) {
  if (!is_valid_character(cha))
    quit("!SetCharacterIgnoreWalkbehinds: Invalid character specified");

  Character_SetIgnoreWalkbehinds(&game.chars[cha], clik);
}


void MoveCharacterToObject(int chaa,int obbj) {
  // invalid object, do nothing
  // this allows MoveCharacterToObject(EGO, GetObjectAt(...));
  if (!is_valid_object(obbj))
    return;

  walk_character(chaa,objs[obbj].x+5,objs[obbj].y+6,0, true);
  do_main_cycle(UNTIL_MOVEEND,(int)&game.chars[chaa].walking);
}

void MoveCharacterToHotspot(int chaa,int hotsp) {
  if ((hotsp<0) || (hotsp>=MAX_HOTSPOTS))
    quit("!MovecharacterToHotspot: invalid hotspot");
  if (thisroom.hswalkto[hotsp].x<1) return;
  walk_character(chaa,thisroom.hswalkto[hotsp].x,thisroom.hswalkto[hotsp].y,0, true);
  do_main_cycle(UNTIL_MOVEEND,(int)&game.chars[chaa].walking);
  }

void MoveCharacterBlocking(int chaa,int xx,int yy,int direct) {
  if (!is_valid_character (chaa))
    quit("!MoveCharacterBlocking: invalid character");

  // check if they try to move the player when Hide Player Char is
  // ticked -- otherwise this will hang the game
  if (game.chars[chaa].on != 1)
    quit("!MoveCharacterBlocking: character is turned off (is Hide Player Character selected?) and cannot be moved");

  if (direct)
    MoveCharacterDirect(chaa,xx,yy);
  else
    MoveCharacter(chaa,xx,yy);
  do_main_cycle(UNTIL_MOVEEND,(int)&game.chars[chaa].walking);
  }

