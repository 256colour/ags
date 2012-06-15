
#include "acmain/ac_maindefines.h"


int GetGUIAt (int xx,int yy) {
    multiply_up_coordinates(&xx, &yy);

    int aa, ll;
    for (ll = game.numgui - 1; ll >= 0; ll--) {
        aa = play.gui_draw_order[ll];
        if (guis[aa].on<1) continue;
        if (guis[aa].flags & GUIF_NOCLICK) continue;
        if ((xx>=guis[aa].x) & (yy>=guis[aa].y) &
            (xx<=guis[aa].x+guis[aa].wid) & (yy<=guis[aa].y+guis[aa].hit))
            return aa;
    }
    return -1;
}

ScriptGUI *GetGUIAtLocation(int xx, int yy) {
    int guiid = GetGUIAt(xx, yy);
    if (guiid < 0)
        return NULL;
    return &scrGui[guiid];
}


int isposinbox(int mmx,int mmy,int lf,int tp,int rt,int bt) {
    if ((mmx>=lf) & (mmx<=rt) & (mmy>=tp) & (mmy<=bt)) return TRUE;
    else return FALSE;
}

// xx,yy is the position in room co-ordinates that we are checking
// arx,ary is the sprite x/y co-ordinates
int is_pos_in_sprite(int xx,int yy,int arx,int ary, block sprit, int spww,int sphh, int flipped = 0) {
    if (spww==0) spww = divide_down_coordinate(sprit->w) - 1;
    if (sphh==0) sphh = divide_down_coordinate(sprit->h) - 1;

    if (isposinbox(xx,yy,arx,ary,arx+spww,ary+sphh)==FALSE)
        return FALSE;

    if (game.options[OPT_PIXPERFECT]) 
    {
        // if it's transparent, or off the edge of the sprite, ignore
        int xpos = multiply_up_coordinate(xx - arx);
        int ypos = multiply_up_coordinate(yy - ary);

        if (gfxDriver->HasAcceleratedStretchAndFlip())
        {
            // hardware acceleration, so the sprite in memory will not have
            // been stretched, it will be original size. Thus, adjust our
            // calculations to compensate
            multiply_up_coordinates(&spww, &sphh);

            if (spww != sprit->w)
                xpos = (xpos * sprit->w) / spww;
            if (sphh != sprit->h)
                ypos = (ypos * sprit->h) / sphh;
        }

        if (flipped)
            xpos = (sprit->w - 1) - xpos;

        int gpcol = my_getpixel(sprit, xpos, ypos);

        if ((gpcol == bitmap_mask_color(sprit)) || (gpcol == -1))
            return FALSE;
    }
    return TRUE;
}



// Used for deciding whether a char or obj was closer
int char_lowest_yp, obj_lowest_yp;

int GetObjectAt(int xx,int yy) {
    int aa,bestshotyp=-1,bestshotwas=-1;
    // translate screen co-ordinates to room co-ordinates
    xx += divide_down_coordinate(offsetx);
    yy += divide_down_coordinate(offsety);
    // Iterate through all objects in the room
    for (aa=0;aa<croom->numobj;aa++) {
        if (objs[aa].on != 1) continue;
        if (objs[aa].flags & OBJF_NOINTERACT)
            continue;
        int xxx=objs[aa].x,yyy=objs[aa].y;
        int isflipped = 0;
        int spWidth = divide_down_coordinate(objs[aa].get_width());
        int spHeight = divide_down_coordinate(objs[aa].get_height());
        if (objs[aa].view >= 0)
            isflipped = views[objs[aa].view].loops[objs[aa].loop].frames[objs[aa].frame].flags & VFLG_FLIPSPRITE;

        block theImage = GetObjectImage(aa, &isflipped);

        if (is_pos_in_sprite(xx, yy, xxx, yyy - spHeight, theImage,
            spWidth, spHeight, isflipped) == FALSE)
            continue;

        int usebasel = objs[aa].get_baseline();   
        if (usebasel < bestshotyp) continue;

        bestshotwas = aa;
        bestshotyp = usebasel;
    }
    obj_lowest_yp = bestshotyp;
    return bestshotwas;
}

ScriptObject *GetObjectAtLocation(int xx, int yy) {
    int hsnum = GetObjectAt(xx, yy);
    if (hsnum < 0)
        return NULL;
    return &scrObj[hsnum];
}


// X and Y co-ordinates must be in 320x200 format
int check_click_on_object(int xx,int yy,int mood) {
    int aa = GetObjectAt(xx - divide_down_coordinate(offsetx), yy - divide_down_coordinate(offsety));
    if (aa < 0) return 0;
    RunObjectInteraction(aa, mood);
    return 1;
}

int is_pos_on_character(int xx,int yy) {
    int cc,sppic,lowestyp=0,lowestwas=-1;
    for (cc=0;cc<game.numcharacters;cc++) {
        if (game.chars[cc].room!=displayed_room) continue;
        if (game.chars[cc].on==0) continue;
        if (game.chars[cc].flags & CHF_NOINTERACT) continue;
        if (game.chars[cc].view < 0) continue;
        CharacterInfo*chin=&game.chars[cc];

        if ((chin->view < 0) || 
            (chin->loop >= views[chin->view].numLoops) ||
            (chin->frame >= views[chin->view].loops[chin->loop].numFrames))
        {
            continue;
        }

        sppic=views[chin->view].loops[chin->loop].frames[chin->frame].pic;
        int usewid = charextra[cc].width;
        int usehit = charextra[cc].height;
        if (usewid==0) usewid=spritewidth[sppic];
        if (usehit==0) usehit=spriteheight[sppic];
        int xxx = chin->x - divide_down_coordinate(usewid) / 2;
        int yyy = chin->get_effective_y() - divide_down_coordinate(usehit);

        int mirrored = views[chin->view].loops[chin->loop].frames[chin->frame].flags & VFLG_FLIPSPRITE;
        block theImage = GetCharacterImage(cc, &mirrored);

        if (is_pos_in_sprite(xx,yy,xxx,yyy, theImage,
            divide_down_coordinate(usewid),
            divide_down_coordinate(usehit), mirrored) == FALSE)
            continue;

        int use_base = chin->get_baseline();
        if (use_base < lowestyp) continue;
        lowestyp=use_base;
        lowestwas=cc;
    }
    char_lowest_yp = lowestyp;
    return lowestwas;
}

int GetCharacterAt (int xx, int yy) {
    xx += divide_down_coordinate(offsetx);
    yy += divide_down_coordinate(offsety);
    return is_pos_on_character(xx,yy);
}

CharacterInfo *GetCharacterAtLocation(int xx, int yy) {
    int hsnum = GetCharacterAt(xx, yy);
    if (hsnum < 0)
        return NULL;
    return &game.chars[hsnum];
}


// allowHotspot0 defines whether Hotspot 0 returns LOCTYPE_HOTSPOT
// or whether it returns 0
int __GetLocationType(int xxx,int yyy, int allowHotspot0) {
  getloctype_index = 0;
  // If it's not in ProcessClick, then return 0 when over a GUI
  if ((GetGUIAt(xxx, yyy) >= 0) && (getloctype_throughgui == 0))
    return 0;

  getloctype_throughgui = 0;

  xxx += divide_down_coordinate(offsetx);
  yyy += divide_down_coordinate(offsety);
  if ((xxx>=thisroom.width) | (xxx<0) | (yyy<0) | (yyy>=thisroom.height))
    return 0;

  // check characters, objects and walkbehinds, work out which is
  // foremost visible to the player
  int charat = is_pos_on_character(xxx,yyy);
  int hsat = get_hotspot_at(xxx,yyy);
  int objat = GetObjectAt(xxx - divide_down_coordinate(offsetx), yyy - divide_down_coordinate(offsety));

  multiply_up_coordinates(&xxx, &yyy);

  int wbat = getpixel(thisroom.object, xxx, yyy);

  if (wbat <= 0) wbat = 0;
  else wbat = croom->walkbehind_base[wbat];

  int winner = 0;
  // if it's an Ignore Walkbehinds object, then ignore the walkbehind
  if ((objat >= 0) && ((objs[objat].flags & OBJF_NOWALKBEHINDS) != 0))
    wbat = 0;
  if ((charat >= 0) && ((game.chars[charat].flags & CHF_NOWALKBEHINDS) != 0))
    wbat = 0;
  
  if ((charat >= 0) && (objat >= 0)) {
    if ((wbat > obj_lowest_yp) && (wbat > char_lowest_yp))
      winner = LOCTYPE_HOTSPOT;
    else if (obj_lowest_yp > char_lowest_yp)
      winner = LOCTYPE_OBJ;
    else
      winner = LOCTYPE_CHAR;
  }
  else if (charat >= 0) {
    if (wbat > char_lowest_yp)
      winner = LOCTYPE_HOTSPOT;
    else
      winner = LOCTYPE_CHAR;
  }
  else if (objat >= 0) {
    if (wbat > obj_lowest_yp)
      winner = LOCTYPE_HOTSPOT;
    else
      winner = LOCTYPE_OBJ;
  }

  if (winner == 0) {
    if (hsat >= 0)
      winner = LOCTYPE_HOTSPOT;
  }

  if ((winner == LOCTYPE_HOTSPOT) && (!allowHotspot0) && (hsat == 0))
    winner = 0;

  if (winner == LOCTYPE_HOTSPOT)
    getloctype_index = hsat;
  else if (winner == LOCTYPE_CHAR)
    getloctype_index = charat;
  else if (winner == LOCTYPE_OBJ)
    getloctype_index = objat;

  return winner;
}

// GetLocationType exported function - just call through
// to the main function with default 0
int GetLocationType(int xxx,int yyy) {
  return __GetLocationType(xxx, yyy, 0);
}


void SaveCursorForLocationChange() {
  // update the current location name
  char tempo[100];
  GetLocationName(divide_down_coordinate(mousex), divide_down_coordinate(mousey), tempo);

  if (play.get_loc_name_save_cursor != play.get_loc_name_last_time) {
    play.get_loc_name_save_cursor = play.get_loc_name_last_time;
    play.restore_cursor_mode_to = GetCursorMode();
    play.restore_cursor_image_to = GetMouseCursor();
    DEBUG_CONSOLE("Saving mouse: mode %d cursor %d", play.restore_cursor_mode_to, play.restore_cursor_image_to);
  }
}


void GetLocationName(int xxx,int yyy,char*tempo) {
  if (displayed_room < 0)
    quit("!GetLocationName: no room has been loaded");

  VALIDATE_STRING(tempo);
  
  if (GetGUIAt(xxx, yyy) >= 0) {
    tempo[0]=0;
    int mover = GetInvAt (xxx, yyy);
    if (mover > 0) {
      if (play.get_loc_name_last_time != 1000 + mover)
        guis_need_update = 1;
      play.get_loc_name_last_time = 1000 + mover;
      strcpy(tempo,get_translation(game.invinfo[mover].name));
    }
    else if ((play.get_loc_name_last_time > 1000) && (play.get_loc_name_last_time < 1000 + MAX_INV)) {
      // no longer selecting an item
      guis_need_update = 1;
      play.get_loc_name_last_time = -1;
    }
    return;
  }
  int loctype = GetLocationType (xxx, yyy);
  xxx += divide_down_coordinate(offsetx); 
  yyy += divide_down_coordinate(offsety);
  tempo[0]=0;
  if ((xxx>=thisroom.width) | (xxx<0) | (yyy<0) | (yyy>=thisroom.height))
    return;

  int onhs,aa;
  if (loctype == 0) {
    if (play.get_loc_name_last_time != 0) {
      play.get_loc_name_last_time = 0;
      guis_need_update = 1;
    }
    return;
  }

  // on character
  if (loctype == LOCTYPE_CHAR) {
    onhs = getloctype_index;
    strcpy(tempo,get_translation(game.chars[onhs].name));
    if (play.get_loc_name_last_time != 2000+onhs)
      guis_need_update = 1;
    play.get_loc_name_last_time = 2000+onhs;
    return;
  }
  // on object
  if (loctype == LOCTYPE_OBJ) {
    aa = getloctype_index;
    strcpy(tempo,get_translation(thisroom.objectnames[aa]));
    if (play.get_loc_name_last_time != 3000+aa)
      guis_need_update = 1;
    play.get_loc_name_last_time = 3000+aa;
    return;
  }
  onhs = getloctype_index;
  if (onhs>0) strcpy(tempo,get_translation(thisroom.hotspotnames[onhs]));
  if (play.get_loc_name_last_time != onhs)
    guis_need_update = 1;
  play.get_loc_name_last_time = onhs;
}

const char* Game_GetLocationName(int x, int y) {
  char buffer[STD_BUFFER_SIZE];
  GetLocationName(x, y, buffer);
  return CreateNewScriptString(buffer);
}
