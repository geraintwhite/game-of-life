#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <ncurses.h>

#include "gol.h"
#include "shapes.c"


static int * cells;

int main();
void init();
void deinit();
void initcurses();
void tick();
void add_circle(int y, int x, int radius);


int main()
{
  init();

  add_circle(LINES / 2, COLS / 2, (2 * LINES > COLS ? COLS / 2 : LINES) / 2);

  tick();
  getch();

  deinit();

  return 0;
}

void init()
{
  initcurses();
  cells = malloc(SIZE);
}

void initcurses()
{
  initscr();
  curs_set(0);
  start_color();
  use_default_colors();

  raw();
  noecho();
  keypad(stdscr, TRUE);
}

void deinit()
{
  free(cells);
  endwin();
}

void tick()
{
  int y, x;

  for (y = 0; y < LINES; y++) {
    for (x = 0; x < COLS; x++) {
      if (CELL(y, x)) {
        mvaddch(y, x, 'X');
      }
    }
  }

  refresh();
}

void add_circle(int y, int x, int radius)
{
  int i;

  Point * points = malloc(sizeof(Point) * 360);
  get_points((Circle) {radius, (Point) {y, x}}, points);

  for(i = 0; i < 360; i++)
  {
    LIVE(points[i].y, points[i].x);
  }

  free(points);
}
