
void PauseGame();
void UnPauseGame();
int IsGamePaused();
void SetGameSpeed(int newspd);
void SetGlobalInt(int index,int valu);
int LoadSaveSlotScreenshot(int slnum, int width, int height);
int GetGameSpeed();
int load_game(int slotn, char*descrp, int *wantShot);
void save_game(int slotn, const char*descript);

extern char saveGameDirectory[260];
extern int want_quit;

