#include <ncurses.h>
#include <stdbool.h>
#include <string.h>
#include <sqlite3.h>
#include <ctype.h>
#include <stdlib.h>
#include <panel.h>

#define MAX_ROWS 20
#define MAX_COLS 6

#define KEY_ESC 27

enum dialog_option {
  YES = 0,
  NO = 1,
};

void init_ncurses()
{
  initscr();
  cbreak();
  curs_set(0);
  noecho();
  keypad(stdscr, true);
}

WINDOW *setup_window()
{
  int max_x, max_y;
  int top_y, bottom_y, top_x, bottom_x;

  getmaxyx(stdscr, max_y, max_x);

  bottom_y = max_y - 2;
  bottom_x = max_x - 4;
  top_y = 1;
  top_x = 2;

  WINDOW *win = newwin(bottom_y, bottom_x, top_y, top_x);
  box(win, 0, 0);
  wrefresh(win);

  return win;
}

void display_table(WINDOW *win, sqlite3 *db, const char *sql_query)
{
  sqlite3_stmt *stmt;
  int rc;
  int row = 0;

  rc = sqlite3_prepare_v2(db, sql_query, -1, &stmt, NULL);
  if (rc != SQLITE_OK) {
    fprintf(stderr, "Failed to fetch table info: %s\n", sqlite3_errmsg(db));
    sqlite3_close(db);
    return;
  }

  int num_cols = sqlite3_column_count(stmt);

  /**
    * TODO: Get a table.
    */

  sqlite3_finalize(stmt);
  wrefresh(win);
}

void display_title(WINDOW *win)
{
  const char *title = "Recipe Manager";
  int title_length = strlen(title);

  int max_x = getmaxx(win); // stdscr
  int title_x_pos = (max_x - title_length) / 2;

  mvwprintw(win, 0, title_x_pos, "%s", title);
  wrefresh(win);
}

void display_menu(WINDOW *win, int selected_menu_item)
{
  char *menu_options[] = {
    "Show",
    "Add",
    "Edit",
    "Delete",
    "Copy",
  };

  int max_y = getmaxy(win);
  int menu_count = sizeof(menu_options) / sizeof(menu_options[0]);
  int menu_start_y = max_y - 1;
  int menu_start_x = 4;

  for (int i = 0; i < menu_count; ++i) {
    if (i == selected_menu_item) {
      wattron(win, A_REVERSE | A_BOLD | A_UNDERLINE); // todo: is A_UNDERLINE doing anything?
      mvwprintw(win, menu_start_y, menu_start_x, "%c", menu_options[i][0]);
      wattroff(win, A_REVERSE | A_BOLD | A_UNDERLINE);
      mvwprintw(win, menu_start_y, menu_start_x + 1, "%s", menu_options[i] + 1);
    } else {
      wattron(win, A_BOLD | A_UNDERLINE);
      mvwprintw(win, menu_start_y, menu_start_x, "%c", menu_options[i][0]);
      wattroff(win, A_BOLD | A_UNDERLINE);
      mvwprintw(win, menu_start_y, menu_start_x + 1, "%s", menu_options[i] + 1);
    }

    menu_start_x += strlen(menu_options[i]) + 4;
  }

  wrefresh(win);
}

bool confirm_exit(WINDOW *win)
{
  int max_y, max_x;
  getmaxyx(stdscr, max_y, max_x);

  int box_height = 5, box_width = 40;
  int start_x = (max_x - box_width) / 2;
  int start_y = (max_y - box_height) / 2;

  WINDOW *exit_win = newwin(box_height, box_width, start_y, start_x);
  PANEL *exit_panel = new_panel(exit_win);
  box(exit_win, 0, 0);

  const char *question = "Are you sure you want to exit?";
  int question_x = (box_width - strlen(question)) / 2;
  mvwprintw(exit_win, 1, question_x, question);

  PANEL *background_panel = new_panel(win);
  bottom_panel(background_panel);
  top_panel(exit_panel);

  update_panels();
  doupdate();

  enum dialog_option current_choice = YES;
  int ch = 0;
  const char *options[] = { "[Yes]", "[No]" };

  keypad(exit_win, TRUE);

  int option_spacing = 4;
  int total_options_width = strlen(options[0]) + strlen(options[1]) + option_spacing;
  int start_options_x = question_x + (strlen(question) - total_options_width) / 2;

  int first_option_pos = start_options_x;
  int second_option_pos = first_option_pos + strlen(options[0]) + option_spacing;

  for (;;) {
    mvwprintw(exit_win, 3, first_option_pos, "%s", options[0]);
    mvwprintw(exit_win, 3, second_option_pos, "%s", options[1]);

    wattron(exit_win, A_REVERSE);
    mvwprintw(exit_win, 3,
      (current_choice == YES) ? first_option_pos : second_option_pos,
      "%s", options[current_choice]);
    wattroff(exit_win, A_REVERSE);

    update_panels();
    doupdate();

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
        del_panel(exit_panel);
        delwin(exit_win);
        update_panels();
        doupdate();
        return current_choice == YES;
    }
  }
}

void setup_sqlite_database(const char *db_name, sqlite3 **db)
{
  int rc = sqlite3_open(db_name, db);
  if (rc != SQLITE_OK) {
    fprintf(stderr, "Cannot open database %s\n", sqlite3_errmsg(*db));
    sqlite3_close(*db);
    exit(EXIT_FAILURE);
  }

  const char sql[] = \
    "CREATE TABLE IF NOT EXISTS recipes ("  \
    "id INTEGER PRIMARY KEY AUTOINCREMENT," \
    "name TEXT," \
    "description TEXT," \
    "ingredients TEXT," \
    "steps TEXT," \
    "metadata TEXT);";

  rc = sqlite3_exec(*db, sql, NULL, NULL, NULL);
  if (rc != SQLITE_OK) {
    fprintf(stderr, "Cannot execute statement %s\n", sqlite3_errmsg(*db));
    sqlite3_close(*db);
    exit(EXIT_FAILURE);
  }
}

void clean(void)
{
  echo();
  nl();
  nocbreak();
  endwin();
}

int main(void)
{
  WINDOW *window;
  sqlite3 *db;
  int selected_menu_item = -1;
  bool should_exit;

  setup_sqlite_database("recipes.db", &db);

  // for debugging purposes
  const char *insert_query = "INSERT INTO recipes (id, name, description, ingredients, steps, metadata) VALUES (1, 'Test Recipe', 'A simple test recipe', 'Flour, Sugar', 'Mix and bake', 'None');";
  sqlite3_exec(db, insert_query, NULL, NULL, NULL);

  init_ncurses();
  atexit(clean);

  window = setup_window();

  display_title(window);
  display_menu(window, selected_menu_item);

  const char *sql_query = "SELECT * FROM recipes";
  display_table(window, db, sql_query);

  // TODO: top left add current window size (limit how small you can go)
  // this should constantly be updating

  int max_x, max_y;
  int ch;
  while ((ch = tolower(wgetch(window)))) {
    //if (ch == KEY_RESIZE) {
    //  delwin(window);
    //  window = setup_window();
    //  display_title(window);
    //  display_menu(window, selected_menu_item);
    //  clear();
    //  refresh();
    //  continue;
    //}

    //getmaxyx(stdscr, max_y, max_x);
    //mvprintw(0, 0, "Size: %dx%d ", max_x, max_y);
    //refresh();

    switch (ch) {
      case 's': // show
        selected_menu_item = 0;
        break;
      case 'a': // add
        selected_menu_item = 1;
        break;
      case 'e': // edit
        selected_menu_item = 2;
        break;
      case 'd': // delete
        selected_menu_item = 3;
        break;
      case 'c': // copy
        selected_menu_item = 4;
        break;
      case KEY_ESC:
      case 'q': // quit
        should_exit = confirm_exit(window);
        if (should_exit) {
          delwin(window);
          endwin();
          sqlite3_close(db);
          return EXIT_SUCCESS;
        }
        break;
      default:
        selected_menu_item = -1;
        break;
    }
    display_menu(window, selected_menu_item);
  }

  delwin(window);
  endwin();
  sqlite3_close(db);
  return 0;
}

