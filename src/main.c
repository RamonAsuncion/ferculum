#include <ncurses.h>
#include <stdbool.h>
#include <string.h>
#include <sqlite3.h>
#include <ctype.h>
#include <stdlib.h>

#define MAX_ROWS 20
#define MAX_COLS 10

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
  int x_max, y_max;
  int y_top, y_bottom, x_top, x_bottom;

  getmaxyx(stdscr, y_max, x_max);

  y_bottom = y_max - 2;
  x_bottom = x_max - 4;
  y_top = 1;
  x_top = 2;

  WINDOW *win = newwin(y_bottom, x_bottom, y_top, x_top);
  box(win, 0, 0);
  wrefresh(win);

  return win;
}

void format_title(char *title)
{
  if (title == NULL || title[0] == '\0') return;

  title[0] = toupper(title[0]);

  for (int i = 1; title[i] != '\0'; ++i) {
    title[i] = tolower(title[i]);
  }
}

void display_table(WINDOW *win, sqlite3 *db, const char *sql_query)
{
  sqlite3_stmt *stmt;
  int rc;
  int row = 1;

  rc = sqlite3_prepare_v2(db, sql_query, -1, &stmt, NULL);
  if (rc != SQLITE_OK) {
    fprintf(stderr, "Failed to fetch table info: %s\n", sqlite3_errmsg(db));
    sqlite3_close(db);
    return;
  }

  while (sqlite3_step(stmt) == SQLITE_ROW) {
    for(int col = 0; col < sqlite3_column_count(stmt); ++col) {
      const char *data = (const char*)sqlite3_column_text(stmt, col);
      if (data) {
        mvwprintw(win, row, col * 20, "%-*s", 20, data);
      }
    }
    row++;
    if (row > MAX_ROWS) break;
  }

  sqlite3_finalize(stmt);
  //wrefresh(win);
}

void display_title(WINDOW *win)
{
  const char *title = "Recipe Manager";
  int title_length = strlen(title);

  int x_max = getmaxx(win);
  int title_x_pos = (x_max - title_length) / 2;

  mvwprintw(win, 0, title_x_pos, "%s", title);
  wrefresh(win);
}

void display_menu(WINDOW *win)
{
  char *menu_options[] = {
    "Show",
    "Add",
    "Edit",
    "Delete",
    "Copy",
  };

  int y_max = getmaxy(win);
  int menu_count = sizeof(menu_options) / sizeof(menu_options[0]);
  int menu_start_y = y_max - 1;
  int menu_start_x = 4;

  for (int i = 0; i < menu_count; ++i) {
    mvwprintw(win, menu_start_y, menu_start_x, "%s", menu_options[i]);
    menu_start_x += strlen(menu_options[i]) + 2;
  }

  wrefresh(win);
}

void setup_sqlite_database(const char *db_name, sqlite3 **db)
{
  int rc = sqlite3_open(db_name, db);

  const char sql[] = \
    "CREATE TABLE IF NOT EXISTS RECIPES("  \
    "ID INTEGER PRIMARY KEY AUTOINCREMENT," \
    "NAME TEXT NOT NULL," \
    "DESCRIPTION TEXT NOT NULL," \
    "INGREDIENTS TEXT NOT NULL," \
    "STEPS TEXT NOT NULL," \
    "METADATA TEXT NOT NULL);";

  if (rc != SQLITE_OK) {
    fprintf(stderr, "Cannot open database %s\n", sqlite3_errmsg(*db));
    sqlite3_close(*db);
    exit(EXIT_FAILURE);
  }

  rc = sqlite3_exec(*db, sql, NULL, NULL, NULL);

  if (rc != SQLITE_OK) {
    fprintf(stderr, "Cannot execute statement %s\n", sqlite3_errmsg(*db));
    sqlite3_close(*db);
    exit(EXIT_FAILURE);
  }
}


int main(void)
{
  WINDOW *window;

  sqlite3 *db;
  const char *db_filename = "recipes.db";

  setup_sqlite_database(db_filename, &db);
  init_ncurses();

  window = setup_window();

  display_title(window);
  display_menu(window);

  wgetch(window);

  endwin();
  sqlite3_close(db);

  return EXIT_SUCCESS;
}

