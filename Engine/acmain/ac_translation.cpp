
#define USE_CLIB
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "wgt2allg.h"
#include "ac/ac_common.h"
#include "ac/gamesetup.h"
#include "ac/gamesetupstruct.h"
#include "ac/gamestate.h"
#include "acmain/ac_maindefines.h"
#include "acmain/ac_speech.h"
#include "acmain/ac_string.h"
#include "acmain/ac_translation.h"
#include "misc.h"
#include "platform/agsplatformdriver.h"
#include "plugin/agsplugin.h"

extern GameSetupStruct game;
extern GameState play;
extern GameSetup usetup;
extern int source_text_length;

TreeMap::TreeMap() {
    left = NULL;
    right = NULL;
    text = NULL;
    translation = NULL;
}
char* TreeMap::findValue (const char* key) {
    if (text == NULL)
        return NULL;

    if (strcmp(key, text) == 0)
        return translation;
    //debug_log("Compare: '%s' with '%s'", key, text);

    if (strcmp (key, text) < 0) {
        if (left == NULL)
            return NULL;
        return left->findValue (key);
    }
    else {
        if (right == NULL)
            return NULL;
        return right->findValue (key);
    }
}
void TreeMap::addText (const char* ntx, char *trans) {
    if ((ntx == NULL) || (ntx[0] == 0) ||
        ((text != NULL) && (strcmp(ntx, text) == 0)))
        // don't add if it's an empty string or if it's already here
        return;

    if (text == NULL) {
        text = (char*)malloc(strlen(ntx)+1);
        translation = (char*)malloc(strlen(trans)+1);
        if (translation == NULL)
            quit("load_translation: out of memory");
        strcpy(text, ntx);
        strcpy(translation, trans);
    }
    else if (strcmp(ntx, text) < 0) {
        // Earlier in alphabet, add to left
        if (left == NULL)
            left = new TreeMap();

        left->addText (ntx, trans);
    }
    else if (strcmp(ntx, text) > 0) {
        // Later in alphabet, add to right
        if (right == NULL)
            right = new TreeMap();

        right->addText (ntx, trans);
    }
}
void TreeMap::clear() {
    if (left) {
        left->clear();
        delete left;
    }
    if (right) {
        right->clear();
        delete right;
    }
    if (text)
        free(text);
    if (translation)
        free(translation);
    left = NULL;
    right = NULL;
    text = NULL;
    translation = NULL;
}
TreeMap::~TreeMap() {
    clear();
}



TreeMap *transtree = NULL;
long lang_offs_start = 0;
char transFileName[MAX_PATH] = "\0";

void close_translation () {
  if (transtree != NULL) {
    delete transtree;
    transtree = NULL;
  }
}

bool init_translation (const char *lang) {
  char *transFileLoc;

  if (lang == NULL) {
    sprintf(transFileName, "default.tra");
  }
  else {
    sprintf(transFileName, "%s.tra", lang);
  }

  transFileLoc = ci_find_file(usetup.data_files_dir, transFileName);

  FILE *language_file = clibfopen(transFileLoc, "rb");
  free(transFileLoc);

  if (language_file == NULL) 
  {
    if (lang != NULL)
    {
      // Just in case they're running in Debug, try compiled folder
      sprintf(transFileName, "Compiled\\%s.tra", lang);
      language_file = clibfopen(transFileName, "rb");
    }
    if (language_file == NULL)
      return false;
  }
  // in case it's inside a library file, record the offset
  lang_offs_start = ftell(language_file);

  char transsig[16];
  fread(transsig, 15, 1, language_file);
  if (strcmp(transsig, "AGSTranslation") != 0) {
    fclose(language_file);
    return false;
  }

  if (transtree != NULL)
  {
    close_translation();
  }
  transtree = new TreeMap();

  while (!feof (language_file)) {
    int blockType = getw(language_file);
    if (blockType == -1)
      break;
    // MACPORT FIX 9/6/5: remove warning
    /* int blockSize = */ getw(language_file);

    if (blockType == 1) {
      char original[STD_BUFFER_SIZE], translation[STD_BUFFER_SIZE];
      while (1) {
        read_string_decrypt (language_file, original);
        read_string_decrypt (language_file, translation);
        if ((strlen (original) < 1) && (strlen(translation) < 1))
          break;
        if (feof (language_file))
          quit("!Language file is corrupt");
        transtree->addText (original, translation);
      }

    }
    else if (blockType == 2) {
      int uidfrom;
      char wasgamename[100];
      fread (&uidfrom, 4, 1, language_file);
      read_string_decrypt (language_file, wasgamename);
      if ((uidfrom != game.uniqueid) || (strcmp (wasgamename, game.gamename) != 0)) {
        char quitmess[250];
        sprintf(quitmess,
          "!The translation file you have selected is not compatible with this game. "
          "The translation is designed for '%s'. Make sure the translation was compiled by the original game author.",
          wasgamename);
        quit(quitmess);
      }
    }
    else if (blockType == 3) {
      // game settings
      int temp = getw(language_file);
      // normal font
      if (temp >= 0)
        SetNormalFont (temp);
      temp = getw(language_file);
      // speech font
      if (temp >= 0)
        SetSpeechFont (temp);
      temp = getw(language_file);
      // text direction
      if (temp == 1) {
        play.text_align = SCALIGN_LEFT;
        game.options[OPT_RIGHTLEFTWRITE] = 0;
      }
      else if (temp == 2) {
        play.text_align = SCALIGN_RIGHT;
        game.options[OPT_RIGHTLEFTWRITE] = 1;
      }
    }
    else
      quit("Unknown block type in translation file.");
  }

  fclose (language_file);

  if (transtree->text == NULL)
    quit("!The selected translation file was empty. The translation source may have been translated incorrectly or you may have generated a blank file.");

  return true;
}

char *get_translation (const char *text) {
  if (text == NULL)
    quit("!Null string supplied to CheckForTranslations");

  source_text_length = strlen(text);
  if ((text[0] == '&') && (play.unfactor_speech_from_textlength != 0)) {
    // if there's an "&12 text" type line, remove "&12 " from the source
    // length
    int j = 0;
    while ((text[j] != ' ') && (text[j] != 0))
      j++;
    j++;
    source_text_length -= j;
  }

  // check if a plugin wants to translate it - if so, return that
  char *plResult = (char*)platform->RunPluginHooks(AGSE_TRANSLATETEXT, (int)text);
  if (plResult) {
    if (((int)plResult >= -1) && ((int)plResult < 10000))
      quit("!Plugin did not return a string for text translation");
    return plResult;
  }

  if (transtree != NULL) {
    // translate the text using the translation file
    char * transl = transtree->findValue (text);
    if (transl != NULL)
      return transl;
  }
  // return the original text
  return (char*)text;
}

int IsTranslationAvailable () {
  if (transtree != NULL)
    return 1;
  return 0;
}


int GetTranslationName (char* buffer) {
    VALIDATE_STRING (buffer);
    const char *copyFrom = transFileName;

    while (strchr(copyFrom, '\\') != NULL)
    {
        copyFrom = strchr(copyFrom, '\\') + 1;
    }
    while (strchr(copyFrom, '/') != NULL)
    {
        copyFrom = strchr(copyFrom, '/') + 1;
    }

    strcpy (buffer, copyFrom);
    // remove the ".tra" from the end of the filename
    if (strstr (buffer, ".tra") != NULL)
        strstr (buffer, ".tra")[0] = 0;

    return IsTranslationAvailable();
}

// End translation functions
