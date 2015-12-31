#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ncurses.h>

#include "shapes.c"
#include "gol.h"


static int DOT = COLOR_PAIR(1) | ' ';
static int * cells;
static int * buffer;
static bool draw_mode = false;


void
update_cell(int y, int x, bool alive)
{
  CELL(y, x) = alive;
  mvaddch(y, x, alive ? DOT : ' ');
}

void
add_circle(int y, int x, int radius)
{
  int i;

  Point * points = malloc(sizeof(Point) * 360);
  get_points((Circle) {radius, (Point) {y, x}}, points);

  for (i = 0; i < 360; i++)
  {
    update_cell(points[i].y, points[i].x, true);
  }

  free(points);
}

int
neighbours(int y, int x)
{
  int dy, dx, n;

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
        n += CELL(y + dy, x + dx);
      }
    }
  }

  return n;
}

void
tick()
{
  int y, x, n;

  memcpy(buffer, cells, SIZE);

  for (y = 0; y < HEIGHT; y++)
  {
    for (x = 0; x < WIDTH; x++)
    {
      // set 'alive' state of each cell depending on the number of neighbours
      n = neighbours(y, x);

      if (n < 2 || n > 3)
      {
        BUFFER(y, x) = false;
      }
      if (n == 3)
      {
        BUFFER(y, x) = true;
      }

      mvaddch(y, x, BUFFER(y, x) ? DOT : ' ');
    }
  }

  memcpy(cells, buffer, SIZE);

  refresh();
}

bool
keyboard(int c)
{
  int y, x;
  getyx(stdscr, y, x);

  switch (c) {
    case KEY_LEFT:
      if (x > 0) x--;
      break;
    case KEY_RIGHT:
      if (x < WIDTH-1) x++;
      break;
    case KEY_UP:
      if (y > 0) y--;
      break;
    case KEY_DOWN:
      if (y < HEIGHT-1) y++;
      break;
    case ' ':
      // toggle currently selected cell 'alive' state
      update_cell(y, x, !CELL(y, x));
      break;
    case 't':
      draw_mode = !draw_mode;
      break;
    case 10:
      tick();
      break;
    default:
      return false;
  }

  if (draw_mode) update_cell(y, x, true);

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
init_game()
{
  cells = malloc(SIZE);
  buffer = malloc(SIZE);

  add_circle(1 * HEIGHT / 4, 1 * WIDTH / 4, (2 * HEIGHT > WIDTH ? WIDTH / 4 : HEIGHT) / 4);
  add_circle(3 * HEIGHT / 4, 1 * WIDTH / 4, (2 * HEIGHT > WIDTH ? WIDTH / 4 : HEIGHT) / 4);
  add_circle(3 * HEIGHT / 4, 3 * WIDTH / 4, (2 * HEIGHT > WIDTH ? WIDTH / 4 : HEIGHT) / 4);
  add_circle(1 * HEIGHT / 4, 3 * WIDTH / 4, (2 * HEIGHT > WIDTH ? WIDTH / 4 : HEIGHT) / 4);
  add_circle(2 * HEIGHT / 4, 2 * WIDTH / 4, (2 * HEIGHT > WIDTH ? WIDTH / 4 : HEIGHT) / 4);

  move(0, 0);
}

void
deinit()
{
  endwin();

  free(cells);
  free(buffer);
}

int
main()
{
  init_curses();
  init_game();

  while (keyboard(getch()));

  deinit();

  return 0;
}
