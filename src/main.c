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
      wattron(win, A_REVERSE | A_BOLD | A_UNDERLINE); // FIXME: why isn't it A_UNDERLINE when active? (https://www.google.com/url?sa=i&url=https%3A%2F%2Fgnosis.cx%2Fpublish%2Fprogramming%2Fcharming_python_6.html&psig=AOvVaw2sEX3ovu3axRFTijJ0H9cI&ust=1738076623421000&source=images&cd=vfe&opi=89978449&ved=0CBcQjhxqFwoTCLDnrY-WlosDFQAAAAAdAAAAABBY)
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
  getmaxyx(win, max_y, max_x);

  int box_height = 5;
  int box_width = 30;
  int start_x = (max_x - box_width) / 2;
  int start_y = (max_y - box_height) / 2;

  WINDOW *exit_win = newwin(box_height, box_width, start_y, start_x);
  box(exit_win, 0, 0);

  const char *question = "Do you really want to exit?";
  mvwprintw(exit_win, 1, 1, question);

  wrefresh(exit_win);

  int current_choice = 1; // yes
  int ch = 0;

  for (;;) {
    const char *yes_option = "[Yes]";
    const char *no_option = "[No]";

    mvwprintw(exit_win, 3, (box_width - strlen(yes_option) - strlen(no_option) - 1) / 2, "%s", yes_option);
    mvwprintw(exit_win, 3, (box_width + strlen(yes_option) + 1) / 2, "%s", no_option);

    if (current_choice == 1) {
      wattron(exit_win, A_REVERSE);
      mvwprintw(exit_win, 3, (box_width - strlen(yes_option) - strlen(no_option) - 1) / 2, "%s", yes_option);
      wattroff(exit_win, A_REVERSE);
    } else {
      wattron(exit_win, A_REVERSE);
      mvwprintw(exit_win, 3, (box_width + strlen(yes_option) + 1) / 2, "%s", no_option);
      wattroff(exit_win, A_REVERSE);
    }

    mvwprintw(exit_win, 4, 1, "Key pressed: %d", ch);

    wrefresh(exit_win);

    ch = wgetch(exit_win);
    switch (ch) {
      case KEY_LEFT:
        current_choice = 1; // yes
        break;
      case KEY_RIGHT:
        current_choice = 0; // no
        break;
      case '\n':
      case KEY_ENTER:
        delwin(exit_win);
        return current_choice == 1;
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

  setup_sqlite_database("recipes.db", &db);

  const char *insert_query = "INSERT INTO recipes (id, name, description, ingredients, steps, metadata) VALUES (1, 'Test Recipe', 'A simple test recipe', 'Flour, Sugar', 'Mix and bake', 'None');";
  sqlite3_exec(db, insert_query, NULL, NULL, NULL);

  init_ncurses();
  atexit(clean);

  window = setup_window();

  display_title(window);
  display_menu(window, selected_menu_item);

  const char *sql_query = "SELECT * FROM recipes";
  display_table(window, db, sql_query);

  char ch;
  while ((ch = tolower(wgetch(window)))) {
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
      case 'q': // quit
        if (confirm_exit(window))
          break;
        else
          continue;
        break;
      default:
        selected_menu_item = -1;
        break;
    }
    display_menu(window, selected_menu_item);
  }

  sqlite3_close(db);
  return 0;
}

