#include <ncurses.h>
#include <math.h>

#include "maths.h"
#include "shapes.h"
#include "shapes.c"

#define DOT 46

int
main(int argc, char * argv[])
{
  int height, width;

  initscr();
  curs_set(0);
  start_color();
  use_default_colors();

//  raw();
//  noecho();
//  keypad(stdscr, TRUE);

  getmaxyx(stdscr, height, width);

  printw("%d, %d", height, width);

  Point centre;
  centre.y = height / 2;
  centre.x = width / 2;

  Circle circle;
  circle.radius = (2 * height > width ? width / 2 : height) / 2;
  circle.centre = centre;

  init_pair(1, COLOR_RED, COLOR_RED);
  attron(COLOR_PAIR(1));
  draw_circle(circle, DOT);
  attroff(COLOR_PAIR(1));

  refresh();
  getch();
  endwin();

  return 0;
}
