#include <ncurses.h>
#include <string.h>

enum dialog_option {
  YES,
  NO,
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

  for (;;) {
    int first_option_pos = (box_width - strlen(options[0]) - strlen(options[1]) - 1) / 2;
    int second_option_pos = (box_width + strlen(options[0]) + 1) / 2;

    //if (initial_text_load) {
    mvwprintw(exit_win, 3, first_option_pos, "%s", options[0]);
    mvwprintw(exit_win, 3, second_option_pos, "%s", options[1]);
    //initial_text_load = false;
    //}

    wattron(exit_win, A_REVERSE);
    mvwprintw(exit_win, 3,
      (current_choice == YES) ? first_option_pos : second_option_pos,
      "%s", options[current_choice]);
    wattroff(exit_win, A_REVERSE);

    mvwprintw(exit_win, 4, 1, "Key pressed: %d", ch);

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
  keypad(stdscr, TRUE);
  confirm_exit();
  endwin();
}
