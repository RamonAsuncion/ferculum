#include <ncurses.h>
#include <stdbool.h>
#include <strings.h>
#include <sqlite3.h>

// return status code

// adding a recipe
// 1. name
// 2. ingredients
// 3. steps
// 4. metadata
// How do I want to display this?


// one screen with a table like? and when you press enter it splits it into
// two where you can see the different recipes on the left and the
// specific recipe on the right?

#define ARRAY_SIZE(_x) (sizeof(_x) / sizeof(_x[0]))

#define HIGHLIGHT_PAIR 1
#define NORMAL_PAIR    2

struct recipe {
  char *name;
  char *description;
  char *ingredents;
  char *steps;
};

void initialize_colors()
{
  start_color();
  init_pair(HIGHLIGHT_PAIR, COLOR_BLACK, COLOR_WHITE);
  init_pair(NORMAL_PAIR, COLOR_WHITE, COLOR_BLACK);
}

int main(void)
{
  struct recipe *recipes = NULL;
  int recipe_count = 0;

  sqlite3 *db;
  sqlite3_stmt *stmt;
  char *db_name = "test.db";
  int rc = sqlite3_open(db_name, &db);

  const char sql[] = \
      "CREATE TABLE IF NOT EXISTS RECIPES("  \
      "ID INTEGER PRIMARY KEY AUTOINCREMENT," \
      "NAME TEXT NOT NULL," \
      "DESCRIPTION TEXT NOT NULL," \
      "INGREDIENTS TEXT NOT NULL," \
      "STEPS TEXT NOT NULL," \
      "METADATA TEXT NOT NULL);";

  if (rc != SQLITE_OK) {
    fprintf(stderr, "Cannot open database %s\n", sqlite3_errmsg(db));
    sqlite3_close(db);
    return 1;
  }

  char *exec_err_msg;
  rc = sqlite3_exec(db, sql, NULL, NULL, &exec_err_msg);

  if (rc != SQLITE_OK) {
    fprintf(stderr, "Cannot execute statement %s\n", exec_err_msg);
    return 1;
  }

  WINDOW *window;
  int x_max, y_max;
  int y_top, y_bottom, x_top, x_bottom;

  initscr();
  cbreak();
  curs_set(0);
  noecho();
  keypad(stdscr, true);

  initialize_colors();

  getmaxyx(stdscr, y_max, x_max);

  y_bottom = y_max - 2;
  x_bottom = x_max - 4;
  y_top = 1;
  x_top = 2;

  window = newwin(y_bottom, x_bottom, y_top, x_top);
  box(window, 0, 0);

  const char *title = "Recipe Manager";
  int title_length = strlen(title);
  int title_x_pos = (x_max - title_length) / 2;
  mvwprintw(window, 0, title_x_pos, "%s", title);

  // wrefresh(window);

  char *menu_options[] = {
    "Show",
    "Add",
    "Edit",
    "Delete",
    "Copy",
  };
  int menu_count = ARRAY_SIZE(menu_options);
  int menu_item = 0;

  int menu_start_y = y_max - 1;
  int menu_start_x = 4; // FIXME: temporary value

  // FIXME: use menu_item to increment index
  for (int i = 0; i < menu_count; ++i) {
    mvprintw(menu_start_y, menu_start_x, "%s", menu_options[i]);
    menu_start_x += strlen(menu_options[i]) + 2;
  }

  // all the columns of the table showing
  // display database as a list on ncurses when user clicks enter it
  // splits the screen and shows the menu on the right


  // wrefresh(window);
  refresh();

  wgetch(window);
  endwin();

  sqlite3_close(db);
  return 0;
}

