#include <ncurses.h>

struct attr_pair {
  int attr;
  const char *name;
};

struct attr_pair attrs[] = {
  { A_NORMAL,       "A_NORMAL" },
  { A_STANDOUT,     "A_STANDOUT" },
  { A_UNDERLINE,    "A_UNDERLINE" },
  { A_REVERSE,      "A_REVERSE" },
  { A_BLINK,        "A_BLINK" },
  { A_DIM,          "A_DIM" },
  { A_BOLD,         "A_BOLD" },
  { A_PROTECT,      "A_PROTECT" },
  { A_INVIS,        "A_INVIS" },
  { A_ALTCHARSET,   "A_ALTCHARSET" },
  { A_CHARTEXT,     "A_CHARTEXT" },
};

size_t n = sizeof(attrs) / sizeof(attrs[0]);

int main(void)
{
  initscr();
  cbreak();
  noecho();

  for (size_t i = 0; i < n; ++i) {
    attron(attrs[i].attr);
    printw("This is %s\n", attrs[i].name);
    attroff(attrs[i].attr);
  }

  refresh();
  getch();
  endwin();
  return 0;
}
