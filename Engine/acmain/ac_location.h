#ifndef __AC_LOCATION_H
#define __AC_LOCATION_H

#include "ac/dynobj/scriptgui.h"
#include "ac/dynobj/scriptobject.h"
#include "ac/dynobj/scriptregion.h"
#include "ac/characterinfo.h"

int get_walkable_area_at_location(int xx, int yy);
int get_walkable_area_at_character (int charnum);


// X and Y co-ordinates must be in 320x200 format
int check_click_on_object(int xx,int yy,int mood);
int is_pos_on_character(int xx,int yy);


int __GetLocationType(int xxx,int yyy, int allowHotspot0);

int GetLocationType(int xxx,int yyy);
void SaveCursorForLocationChange();
void GetLocationName(int xxx,int yyy,char*tempo);
const char* Game_GetLocationName(int x, int y);

extern int getloctype_index, getloctype_throughgui;

#endif // __AC_LOCATION_H


