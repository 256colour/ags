#include <pspsdk.h>
#include <pspthreadman.h>
#include <pspdebug.h>
#include <pspctrl.h>
#include <pspkernel.h>
#include <psputility.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

#include "../kernel/kernel.h"
#include "pe.h"


PSP_MODULE_INFO("launcher", 0, 1, 1);


typedef struct
{
  char display_name[100];
  char path[200];
} games_t;


char psp_game_file_name[256];
char file_to_exec[256];
int quit_to_menu = 1;

int count = 0;
int max_entries = 25;
games_t entries[25];




int exit_callback(int arg1, int arg2, void *common)
{
  sceKernelExitGame();
  return 0;
}



int CallbackThread(SceSize args, void *argp)
{
  int cbid;
  cbid = sceKernelCreateCallback("Launcher Exit Callback", exit_callback, NULL);
  sceKernelRegisterExitCallback(cbid);
  sceKernelSleepThreadCB();
  return 0;
}



int CompareFunction(const void* a, const void* b)
{
  return strcmp(((games_t*)a)->display_name, ((games_t*)b)->display_name);
}



int IsCompatibleDatafile(version_info_t* version_info)
{
  if (strcmp(version_info->internal_name, "acwin") != 0)
    return 0;

  int major = 0;
  int minor = 0;
  int rev = 0;
  int build = 0;
  
  sscanf(version_info->version, "%d.%d.%d.%d", &major, &minor, &rev, &build);
  
  return ((major == 3) && (minor == 2));
}


int CreateGameList()
{
  memset(entries, 0, max_entries * sizeof(games_t));
  
  version_info_t version_info;
  char buffer[200];
  DIR* fd = NULL;
  struct dirent* entry = NULL;
  struct stat info;
  if ((fd = opendir(".")))
  {
    while ((entry = readdir(fd)) && (count < max_entries))
    {
      // Exclude pseudo directories
      if ((strcmp(entry->d_name, ".") == 0) || (strcmp(entry->d_name, "..") == 0))
        continue;
    
      // Exclude files
      stat(entry->d_name, &info);
      if (!S_ISDIR(info.st_mode))
        continue;

      // Check for ac2game.dat in the folder
      strcpy(buffer, entry->d_name);
      strcat(buffer, "/ac2game.dat");
      if (stat(buffer, &info) == 0)
      {
        if (!getVersionInformation(buffer, &version_info))
          break;
  
        if (IsCompatibleDatafile(&version_info))
        {
          // Add to games list
          strcpy(entries[count].path, buffer);
          strcpy(entries[count].display_name, entry->d_name);
          count++;
          continue;
        }
      }

      // Check all ".exe" files in the folder
      DIR* fd1 = NULL;
      struct dirent* entry1 = NULL;

      if ((fd1 = opendir(entry->d_name)))
      {
        while ((entry1 = readdir(fd1)))
        {
          // Exclude the setup program
          if (stricmp(entry1->d_name, "winsetup.exe") == 0)
            continue;

          // Filename must be >= 4 chars long
          int length = strlen(entry1->d_name);
          if (length < 4)
            continue;
  
          if (stricmp(&(entry1->d_name[length - 4]), ".exe") == 0)
          {
            strcpy(buffer, entry->d_name);
            strcat(buffer, "/");
            strcat(buffer, entry1->d_name);

            if (!getVersionInformation(buffer, &version_info))
            continue;
  
            if (IsCompatibleDatafile(&version_info))
            {
              // Add to games list
              strcpy(entries[count].path, buffer);
              strcpy(entries[count].display_name, entry->d_name);
              count++;
              break;
            }
          }
        }
        closedir(fd1);
      }
    }  
    closedir(fd);
  }
  
  return 1;
}



void ShowMenu()
{
  int swap_buttons = 0;
  sceUtilityGetSystemParamInt(PSP_SYSTEMPARAM_ID_INT_UNKNOWN, &swap_buttons);
  unsigned int ok_button = swap_buttons ? PSP_CTRL_CROSS : PSP_CTRL_CIRCLE;

  pspDebugScreenInit();
  pspDebugScreenEnableBackColor(0);
  
  // Check for ac2game.dat in eboot folder and autostart if it exists
  FILE* test = fopen("ac2game.dat", "rb");
  if (test)
  {
    fclose(test);
    getcwd(psp_game_file_name, 256);
    strcat(psp_game_file_name, "/ac2game.dat");
    quit_to_menu = 0;
    return;
  }

  pspDebugScreenPrintf("Adventure Game Studio 3.21 by Chris Jones. PSP port by JJS.\n\n\n");
  pspDebugScreenPrintf("Please select a game:");
  
  if (count == 0)
  {
    pspDebugScreenPrintf("\n\nError: No games found. Quitting in 10 seconds.\n");
    sceKernelDelayThread(10 * 1000 * 1000);
    sceKernelExitGame();
  }

  pspDebugScreenSetXY(0, 33);
  pspDebugScreenPrintf("Press %s to start", swap_buttons ? "CROSS" : "CIRCLE");


  qsort(entries, count, sizeof(games_t), CompareFunction);
  
  SceCtrlData pad;
  unsigned int old_buttons = 0;
  int index = 0;
  int repeatCount = 0;
  
  while (1)
  {
    pspDebugScreenSetXY(0, 6);
  
    int i;
    for (i = 0; i < count; i++)
    {
      if (i == index)
        pspDebugScreenSetTextColor(0xFFFFFFFF);
      else
        pspDebugScreenSetTextColor(0xFF999999);

      pspDebugScreenPrintf("   %s\n", entries[i].display_name);
    }

    sceCtrlReadBufferPositive(&pad, 1);

    if ((pad.Buttons & PSP_CTRL_DOWN) && !(old_buttons & PSP_CTRL_DOWN))
    {
      index++;
      if (index >= count)
        index = 0;
    }

    if ((pad.Buttons & PSP_CTRL_UP) && !(old_buttons & PSP_CTRL_UP))
    {
      index--;
      if (index < 0)
        index = count - 1;
    }

    if ((pad.Buttons & ok_button) && !(old_buttons & ok_button))
    {
      getcwd(psp_game_file_name, 256);
      strcat(psp_game_file_name, "/");
      strcat(psp_game_file_name, entries[index].path);
      pspDebugScreenClear();
      return;
    }
  
    // Handling of button press repeating
    if (old_buttons == pad.Buttons)
      repeatCount++;
    else
      repeatCount = 0;

    if (repeatCount > 10)
    {
      pad.Buttons = 0;
      repeatCount = 0;
    }

    old_buttons = pad.Buttons;
  }  
}



int main(int argc, char *argv[])
{
  // Setup callbacks, remember thread id
  int callback_thid = sceKernelCreateThread("launcher_update_thread", CallbackThread, 0x11, 0xFA0, 0, 0);
  if (callback_thid > -1)
    sceKernelStartThread(callback_thid, 0, 0);

  // Search game files
  pspDebugScreenInit();
  pspDebugScreenPrintf("Searching for compatible games...");
  CreateGameList();

  // Display game selection screen  
  ShowMenu();
  
  // Load the kernel mode prx  
  SceUID moduleId = pspSdkLoadStartModule("kernel.prx", PSP_MEMORY_PARTITION_KERNEL);
  if (moduleId < 0)
  {
    pspDebugScreenInit();
    pspDebugScreenPrintf("Error: Cannot load \"kernel.prx\". Quitting in 10 seconds.\n");
    sceKernelDelayThread(10 * 1000 * 1000);
    sceKernelExitGame();
  }

  // Terminate the callback thread
  sceKernelWakeupThread(callback_thid);
  sceKernelWaitThreadEnd(callback_thid, NULL);
  sceKernelTerminateDeleteThread(callback_thid);

  // Execute the AGS 3.21 engine
  strcpy(file_to_exec, argv[0]);
  strcpy(&file_to_exec[strlen(file_to_exec) - strlen("EBOOT.PBP")], "ags321.prx");

  char* buffer_argv[2];
  buffer_argv[0] = psp_game_file_name;
  buffer_argv[1] = (quit_to_menu ? "menu" : "quit");
 
  // Load the prx with the kernel mode function  
  kernel_loadExec(file_to_exec, 2, buffer_argv);
  
  return 0;
}
