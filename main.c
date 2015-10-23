#include <ncurses.h>
#include <math.h>

#include "maths.h"
#include "shapes.h"
#include "shapes.c"

#define DOT 46

int
main(int argc, char * argv[])
{
  int height, width, radius;

  initscr();
  curs_set(0);
  start_color();
  use_default_colors();

//  raw();
//  noecho();
//  keypad(stdscr, TRUE);

  getmaxyx(stdscr, height, width);

  printw("%d, %d", height, width);

  radius = (2 * height > width ? width / 2 : height) / 2;

  Point centre;
  centre.y = height / 2;
  centre.x = width / 2;

  Circle circle1;
  circle1.radius = radius;
  circle1.centre = centre;

  Circle circle2;
  circle2.radius = radius / 2;
  circle2.centre = centre;

  init_pair(1, COLOR_RED, COLOR_RED);
  init_pair(2, COLOR_YELLOW, COLOR_YELLOW);

  attron(COLOR_PAIR(1));
  draw_circle(circle1, DOT);
  attron(COLOR_PAIR(2));
  draw_circle(circle2, DOT);

  refresh();
  getch();
  endwin();

  return 0;
}
