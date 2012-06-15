
#include "acmain/ac_maindefines.h"



void FlipScreen(int amount) {
  if ((amount<0) | (amount>3)) quit("!FlipScreen: invalid argument (0-3)");
  play.screen_flipped=amount;
}



void ShakeScreen(int severe) {
  EndSkippingUntilCharStops();

  if (play.fast_forward)
    return;

  int hh;
  block oldsc=abuf;
  severe = multiply_up_coordinate(severe);

  if (gfxDriver->RequiresFullRedrawEachFrame())
  {
    play.shakesc_length = 10;
    play.shakesc_delay = 2;
    play.shakesc_amount = severe;
    play.mouse_cursor_hidden++;

    for (hh = 0; hh < 40; hh++) {
      loopcounter++;
      platform->Delay(50);

      render_graphics();

      update_polled_stuff_if_runtime();
    }

    play.mouse_cursor_hidden--;
    clear_letterbox_borders();
    play.shakesc_length = 0;
  }
  else
  {
    block tty = create_bitmap(scrnwid, scrnhit);
    gfxDriver->GetCopyOfScreenIntoBitmap(tty);
    for (hh=0;hh<40;hh++) {
      platform->Delay(50);

      if (hh % 2 == 0) 
        render_to_screen(tty, 0, 0);
      else
        render_to_screen(tty, 0, severe);

      update_polled_stuff_if_runtime();
    }
    clear_letterbox_borders();
    render_to_screen(tty, 0, 0);
    wfreeblock(tty);
  }

  abuf=oldsc;
}

void ShakeScreenBackground (int delay, int amount, int length) {
  if (delay < 2) 
    quit("!ShakeScreenBackground: invalid delay parameter");

  if (amount < play.shakesc_amount)
  {
    // from a bigger to smaller shake, clear up the borders
    clear_letterbox_borders();
  }

  play.shakesc_amount = amount;
  play.shakesc_delay = delay;
  play.shakesc_length = length;
}
