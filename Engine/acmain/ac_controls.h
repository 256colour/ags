
int my_readkey();
int IsButtonDown(int which);
int IsKeyPressed (int keycode);
// check_controls: checks mouse & keyboard interface
void check_controls();

extern int restrict_until;

#define ALLEGRO_KEYBOARD_HANDLER
// KEYBOARD HANDLER
#if defined(LINUX_VERSION) || defined(MAC_VERSION)
extern int myerrno;
#else
extern int errno;
#define myerrno errno
#endif
