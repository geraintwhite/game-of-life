#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ncurses.h>

#include "shapes.c"
#include "gol.h"


typedef struct
{
  bool state;
} Cell;

typedef struct
{
  int size;
  Cell * cells;
} Cells;

static int DOT = COLOR_PAIR(1) | ' ';
static bool draw_mode = false;


void
update_cell(Cells * cells, int y, int x, bool alive)
{
  Cell * cell = cells->cells + COORD(y, x);
  cell->state = alive;
  mvaddch(y, x, alive ? DOT : ' ');
}

void
add_circle(Cells * cells, int y, int x, int radius)
{
  int i;

  Point points[360];
  get_points((Circle) {radius, (Point) {y, x}}, points);

  for (i = 0; i < 360; i++)
  {
    update_cell(cells, points[i].y, points[i].x, true);
  }
}

int
neighbours(Cells * cells, int y, int x)
{
  int dy, dx;
  int n = 0;

  for (dy = -1; dy <= 1; dy++)
  {
    for (dx = -1; dx <= 1; dx++)
    {
      if (!(dy == 0 && dx == 0) &&
          y + dy >= 0 &&
          x + dx >= 0 &&
          y + dy < HEIGHT &&
          x + dx < WIDTH)
      {
        Cell * current_cell = cells->cells + COORD(y + dy, x + dx);
        n += current_cell->state;
      }
    }
  }

  return n;
}

void
tick(Cells * cells, Cells * buffer)
{
  int y, x, n;

  memcpy(buffer->cells, cells->cells, cells->size);

  for (y = 0; y < HEIGHT; y++)
  {
    for (x = 0; x < WIDTH; x++)
    {
      // set 'alive' state of each cell depending on the number of neighbours
      n = neighbours(cells, y, x);

      Cell * current_cell = buffer->cells + COORD(y, x);

      if (n < 2 || n > 3)
      {
        current_cell->state = false;
      }
      if (n == 3)
      {
        current_cell->state = true;
      }

      mvaddch(y, x, current_cell->state ? DOT : ' ');
    }
  }

  memcpy(cells->cells, buffer->cells, cells->size);

  refresh();
}

bool
keyboard(int c, Cells * cells, Cells * buffer)
{
  int y, x;
  getyx(stdscr, y, x);


  switch (c) {
    case KEY_LEFT:
    {
      if (x > 0) x--;
    } break;
    case KEY_RIGHT:
    {
      if (x < WIDTH-1) x++;
    } break;
    case KEY_UP:
    {
      if (y > 0) y--;
    } break;
    case KEY_DOWN:
    {
      if (y < HEIGHT-1) y++;
    } break;
    case ' ':
    {
      // toggle currently selected cell 'alive' state
      Cell * cursor_cell = cells->cells + COORD(y, x);
      update_cell(cells, y, x, !(cursor_cell->state));
    } break;
    case 't':
    {
      draw_mode = !draw_mode;
    } break;
    case 10:
    {
      tick(cells, buffer);
    } break;
    default:
    {
      return false;
    }
  }

  if (draw_mode) update_cell(cells, y, x, true);

  move(y, x);
  refresh();

  return true;
}

void
init_curses()
{
  initscr();
  start_color();
  use_default_colors();

  init_pair(1, COLOR_BLACK, COLOR_BLUE);

  raw();
  noecho();
  keypad(stdscr, TRUE);
}

void
init_game(Cells * cells, Cells * buffer)
{
  int size = WIDTH * HEIGHT * sizeof(int);

  cells->size = size;
  cells->cells = malloc(cells->size);

  buffer->size = size;
  buffer->cells = malloc(buffer->size);

  add_circle(cells, 1 * HEIGHT / 4, 1 * WIDTH / 4, (2 * HEIGHT > WIDTH ? WIDTH / 4 : HEIGHT) / 4);
  add_circle(cells, 3 * HEIGHT / 4, 1 * WIDTH / 4, (2 * HEIGHT > WIDTH ? WIDTH / 4 : HEIGHT) / 4);
  add_circle(cells, 3 * HEIGHT / 4, 3 * WIDTH / 4, (2 * HEIGHT > WIDTH ? WIDTH / 4 : HEIGHT) / 4);
  add_circle(cells, 1 * HEIGHT / 4, 3 * WIDTH / 4, (2 * HEIGHT > WIDTH ? WIDTH / 4 : HEIGHT) / 4);
  add_circle(cells, 2 * HEIGHT / 4, 2 * WIDTH / 4, (2 * HEIGHT > WIDTH ? WIDTH / 4 : HEIGHT) / 4);

  move(0, 0);
}

void
deinit(Cells * cells, Cells * buffer)
{
  endwin();

  free(cells->cells);
  free(buffer->cells);
}

int
main()
{
  init_curses();

  Cells cells;
  Cells buffer;
  init_game(&cells, &buffer);

  while (keyboard(getch(), &cells, &buffer));

  deinit(&cells, &buffer);

  return 0;
}
