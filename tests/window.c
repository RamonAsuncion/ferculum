#include <ncurses.h>
#include <string.h>
#include <stdlib.h>

enum dialog_option {
  YES = 0,
  NO = 1,
};

bool confirm_exit()
{
  int max_y, max_x;
  getmaxyx(stdscr, max_y, max_x);

  int box_height = 5;
  int box_width = 30;
  int start_x = (max_x - box_width) / 2;
  int start_y = (max_y - box_height) / 2;

  WINDOW *exit_win = newwin(box_height, box_width, start_y, start_x);
  box(exit_win, 0, 0);

  const char *question = "Do you really want to exit?";
  mvwprintw(exit_win, 1, 1, question);

  wrefresh(exit_win);

  enum dialog_option current_choice = YES;
  int ch = 0;

  bool initial_text_load = true;

  const char *options[] = { "[Yes]", "[No]" };

  keypad(exit_win, TRUE);
  wrefresh(exit_win);

  for (;;) {
    int first_option_pos = (box_width - strlen(options[0]) - strlen(options[1]) - 1) / 2;
    int second_option_pos = (box_width + strlen(options[0]) + 1) / 2;

    mvwprintw(exit_win, 3, first_option_pos, "%s", options[0]);
    mvwprintw(exit_win, 3, second_option_pos, "%s", options[1]);

    wattron(exit_win, A_REVERSE);
    mvwprintw(exit_win, 3,
      (current_choice == YES) ? first_option_pos : second_option_pos,
      "%s", options[current_choice]);
    wattroff(exit_win, A_REVERSE);

    wrefresh(exit_win);

    ch = wgetch(exit_win);
    switch (ch) {
      case KEY_LEFT:
        current_choice = YES;
        break;
      case KEY_RIGHT:
        current_choice = NO;
        break;
      case '\n':
      case KEY_ENTER:
        delwin(exit_win);
        return current_choice == YES;
    }
  }
}

int main(void) {
  initscr();
  cbreak();
  noecho();
  curs_set(0);
  keypad(stdscr, TRUE);
  bool should_exit = confirm_exit();
  endwin();
  return should_exit ? EXIT_SUCCESS : EXIT_FAILURE;
}
