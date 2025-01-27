#include <ncurses.h>
#include <stdbool.h>
#include <string.h>
#include <sqlite3.h>
#include <ctype.h>
#include <stdlib.h>

#define MAX_ROWS 20
#define MAX_COLS 6

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

  //for (int col = 0; col < num_cols; ++col) {
  //  const char *col_name = sqlite3_column_name(stmt, col);

  //  char col_name_copy[256];
  //  strncpy(col_name_copy, col_name, sizeof(col_name_copy) - 1);
  //  col_name_copy[sizeof(col_name_copy) - 1] = '\0';
  //  col_name_copy[0] = toupper(col_name_copy[0]);

  //  mvwprintw(win, row, col * 22 + 1, "%-20s", col_name_copy);
  //  mvwprintw(win, row, (col + 1) * 22, "|");
  //}

  row = 2;
  //while (sqlite3_step(stmt) == SQLITE_ROW) {
  //  if (row >= MAX_ROWS) break;

  //  for(int col = 0; col < num_cols; ++col) {
  //    if (col >= MAX_COLS) break;

  //    const char *data = (const char*)sqlite3_column_text(stmt, col);

  //    mvwprintw(win, row, col * 22 + 1, "%-*s", 20, data ? data : "N/A");
  //    mvwprintw(win, row, (col + 1) * 22, "|");

  //  }
  //  row++;
  //}

  sqlite3_finalize(stmt);
  wrefresh(win);
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
    wattron(win, A_BOLD | A_UNDERLINE);
    mvwprintw(win, menu_start_y, menu_start_x, "%c", menu_options[i][0]);
    wattroff(win, A_BOLD | A_UNDERLINE);
    mvwprintw(win, menu_start_y, menu_start_x + 1, "%s", menu_options[i] + 1);
    menu_start_x += strlen(menu_options[i]) + 4;
  }

  wrefresh(win);
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

int main(void)
{
  WINDOW *window;
  sqlite3 *db;

  setup_sqlite_database("recipes.db", &db);

  const char *insert_query = "INSERT INTO recipes (id, name, description, ingredients, steps, metadata) VALUES (1, 'Test Recipe', 'A simple test recipe', 'Flour, Sugar', 'Mix and bake', 'None');";
  sqlite3_exec(db, insert_query, NULL, NULL, NULL);

  init_ncurses();

  window = setup_window();

  display_title(window);
  display_menu(window);


  const char *sql_query = "SELECT * FROM recipes";
  display_table(window, db, sql_query);

  wgetch(window);

  endwin();
  sqlite3_close(db);

  return 0;
}

