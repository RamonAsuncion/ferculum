#include <ncurses.h>
#include <panel.h>

/**
  * Same forum panel for edit / add with some modifications.
  */

int main(void)
{
  initscr();
  cbreak();
  noecho();
  WINDOW *win = newwin(10, 40, 5, 5); // height, width, start_y, start_x
  box(win, 0, 0);
  wrefresh(win);
  getch();
  endwin();
  return 0;
}
