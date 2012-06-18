
#include "acmain/ac_maindefines.h"
#include "acmain/ac_strings.h"


void split_lines_rightleft (char *todis, int wii, int fonnt) {
    // start on the last character
    char *thisline = todis + strlen(todis) - 1;
    char prevlwas, *prevline = NULL;
    // work backwards
    while (thisline >= todis) {

        int needBreak = 0;
        if (thisline <= todis) 
            needBreak = 1;
        // ignore \[ sequence
        else if ((thisline > todis) && (thisline[-1] == '\\')) { }
        else if (thisline[0] == '[') {
            needBreak = 1;
            thisline++;
        }
        else if (wgettextwidth_compensate(thisline, fonnt) >= wii) {
            // go 'back' to the nearest word
            while ((thisline[0] != ' ') && (thisline[0] != 0))
                thisline++;

            if (thisline[0] == 0)
                quit("!Single word too wide for window");

            thisline++;
            needBreak = 1;
        }

        if (needBreak) {
            strcpy(lines[numlines], thisline);
            removeBackslashBracket(lines[numlines]);
            numlines++;
            if (prevline) {
                prevline[0] = prevlwas;
            }
            thisline--;
            prevline = thisline;
            prevlwas = prevline[0];
            prevline[0] = 0;
        }

        thisline--;
    }
    if (prevline)
        prevline[0] = prevlwas;
}



char *reverse_text(char *text) {
    int stlen = strlen(text), rr;
    char *backwards = (char*)malloc(stlen + 1);
    for (rr = 0; rr < stlen; rr++)
        backwards[rr] = text[(stlen - rr) - 1];
    backwards[stlen] = 0;
    return backwards;
}

void wouttext_reverseifnecessary(int x, int y, int font, char *text) {
    char *backwards = NULL;
    char *otext = text;
    if (game.options[OPT_RIGHTLEFTWRITE]) {
        backwards = reverse_text(text);
        otext = backwards;
    }

    wouttext_outline(x, y, font, otext);

    if (backwards)
        free(backwards);
}

void break_up_text_into_lines(int wii,int fonnt,char*todis) {
    if (fonnt == -1)
        fonnt = play.normal_font;

    //  char sofar[100];
    if (todis[0]=='&') {
        while ((todis[0]!=' ') & (todis[0]!=0)) todis++;
        if (todis[0]==' ') todis++;
    }
    numlines=0;
    longestline=0;

    // Don't attempt to display anything if the width is tiny
    if (wii < 3)
        return;

    int rr;

    if (game.options[OPT_RIGHTLEFTWRITE] == 0)
    {
        split_lines_leftright(todis, wii, fonnt);
    }
    else {
        // Right-to-left just means reverse the text then
        // write it as normal
        char *backwards = reverse_text(todis);
        split_lines_rightleft (backwards, wii, fonnt);
        free(backwards);
    }

    for (rr=0;rr<numlines;rr++) {
        if (wgettextwidth_compensate(lines[rr],fonnt) > longestline)
            longestline = wgettextwidth_compensate(lines[rr],fonnt);
    }
}



int MAXSTRLEN = MAX_MAXSTRLEN;
void check_strlen(char*ptt) {
  MAXSTRLEN = MAX_MAXSTRLEN;
  long charstart = (long)&game.chars[0];
  long charend = charstart + sizeof(CharacterInfo)*game.numcharacters;
  if (((long)&ptt[0] >= charstart) && ((long)&ptt[0] <= charend))
    MAXSTRLEN=30;
}




/*void GetLanguageString(int indxx,char*buffr) {
  VALIDATE_STRING(buffr);
  char*bptr=get_language_text(indxx);
  if (bptr==NULL) strcpy(buffr,"[language string error]");
  else strncpy(buffr,bptr,199);
  buffr[199]=0;
  }*/

void my_strncpy(char *dest, const char *src, int len) {
  // the normal strncpy pads out the string with zeros up to the
  // max length -- we don't want that
  if (strlen(src) >= (unsigned)len) {
    strncpy(dest, src, len);
    dest[len] = 0;
  }
  else
    strcpy(dest, src);
}

void _sc_strcat(char*s1,char*s2) {
  // make sure they don't try to append a char to the string
  VALIDATE_STRING (s2);
  check_strlen(s1);
  int mosttocopy=(MAXSTRLEN-strlen(s1))-1;
//  int numbf=game.iface[4].numbuttons;
  my_strncpy(&s1[strlen(s1)], s2, mosttocopy);
}

void _sc_strcpy(char*s1,char*s2) {
  check_strlen(s1);
  my_strncpy(s1, s2, MAXSTRLEN - 1);
}

int StrContains (const char *s1, const char *s2) {
  VALIDATE_STRING (s1);
  VALIDATE_STRING (s2);
  char *tempbuf1 = (char*)malloc(strlen(s1) + 1);
  char *tempbuf2 = (char*)malloc(strlen(s2) + 1);
  strcpy(tempbuf1, s1);
  strcpy(tempbuf2, s2);
  strlwr(tempbuf1);
  strlwr(tempbuf2);

  char *offs = strstr (tempbuf1, tempbuf2);
  free(tempbuf1);
  free(tempbuf2);

  if (offs == NULL)
    return -1;

  return (offs - tempbuf1);
}

void _sc_strlower (char *desbuf) {
  VALIDATE_STRING(desbuf);
  check_strlen (desbuf);
  strlwr (desbuf);
}

void _sc_strupper (char *desbuf) {
  VALIDATE_STRING(desbuf);
  check_strlen (desbuf);
  strupr (desbuf);
}

/*int _sc_strcmp (char *s1, char *s2) {
  return strcmp (get_translation (s1), get_translation(s2));
}

int _sc_stricmp (char *s1, char *s2) {
  return stricmp (get_translation (s1), get_translation(s2));
}*/


// Custom printf, needed because floats are pushed as 8 bytes
void my_sprintf(char *buffer, const char *fmt, va_list ap) {
  int bufidx = 0;
  const char *curptr = fmt;
  const char *endptr;
  char spfbuffer[STD_BUFFER_SIZE];
  char fmtstring[100];
  int numargs = -1;

  while (1) {
    // copy across everything until the next % (or end of string)
    endptr = strchr(curptr, '%');
    if (endptr == NULL)
      endptr = &curptr[strlen(curptr)];
    while (curptr < endptr) {
      buffer[bufidx] = *curptr;
      curptr++;
      bufidx++;
    }
    // at this point, curptr and endptr should be equal and pointing
    // to the % or \0
    if (*curptr == 0)
      break;
    if (curptr[1] == '%') {
      // "%%", so just write a % to the output
      buffer[bufidx] = '%';
      bufidx++;
      curptr += 2;
      continue;
    }
    // find the end of the % clause
    while ((*endptr != 'd') && (*endptr != 'f') && (*endptr != 'c') &&
           (*endptr != 0) && (*endptr != 's') && (*endptr != 'x') &&
           (*endptr != 'X'))
      endptr++;

    if (numargs >= 0) {
      numargs--;
      // if there are not enough arguments, just copy the %d
      // to the output string rather than trying to format it
      if (numargs < 0)
        endptr = &curptr[strlen(curptr)];
    }

    if (*endptr == 0) {
      // something like %p which we don't support, so just write
      // the % to the output
      buffer[bufidx] = '%';
      bufidx++;
      curptr++;
      continue;
    }
    // move endptr to 1 after the end character
    endptr++;

    // copy the %d or whatever
    strncpy(fmtstring, curptr, (endptr - curptr));
    fmtstring[endptr - curptr] = 0;

    unsigned int theArg = va_arg(ap, unsigned int);

    // use sprintf to parse the actual %02d type thing
    if (endptr[-1] == 'f') {
      // floats are pushed as 8-bytes, so ensure that it knows this is a float
      float floatArg;
      memcpy(&floatArg, &theArg, sizeof(float));
      sprintf(spfbuffer, fmtstring, floatArg);
    }
    else if ((theArg == (int)buffer) && (endptr[-1] == 's'))
      quit("Cannot use destination as argument to StrFormat");
    else if ((theArg < 0x10000) && (endptr[-1] == 's'))
      quit("!One of the string arguments supplied was not a string");
    else if (endptr[-1] == 's')
    {
      strncpy(spfbuffer, (const char*)theArg, STD_BUFFER_SIZE);
      spfbuffer[STD_BUFFER_SIZE - 1] = 0;
    }
    else 
      sprintf(spfbuffer, fmtstring, theArg);

    // use the formatted text
    buffer[bufidx] = 0;

    if (bufidx + strlen(spfbuffer) >= STD_BUFFER_SIZE)
      quitprintf("!String.Format: buffer overrun: maximum formatted string length %d chars, this string: %d chars", STD_BUFFER_SIZE, bufidx + strlen(spfbuffer));

    strcat(buffer, spfbuffer);
    bufidx += strlen(spfbuffer);
    curptr = endptr;
  }
  buffer[bufidx] = 0;

}

void _sc_AbortGame(char*texx, ...) {
  char displbuf[STD_BUFFER_SIZE] = "!?";
  va_list ap;
  va_start(ap,texx);
  my_sprintf(&displbuf[2], get_translation(texx), ap);
  va_end(ap);

  quit(displbuf);
}

void _sc_sprintf(char*destt,char*texx, ...) {
  char displbuf[STD_BUFFER_SIZE];
  VALIDATE_STRING(destt);
  check_strlen(destt);
  va_list ap;
  va_start(ap,texx);
  my_sprintf(displbuf, get_translation(texx), ap);
  va_end(ap);

  my_strncpy(destt, displbuf, MAXSTRLEN - 1);
}

