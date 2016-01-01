#include "shapes.h"
#include "maths.h"

void
get_points(Circle circle, Point * points)
{
  int y, x, height, width, deg;

  height = circle.radius;
  width = 2 * circle.radius;

  Point offset;
  offset.y = circle.centre.y - height;
  offset.x = circle.centre.x - width;

  for (deg = 0; deg < 360; deg++)
  {
    points[deg].y = (int) offset.y + height + (height * sin(DEGTORAD(deg)));
    points[deg].x = (int) offset.x + width + (width * cos(DEGTORAD(deg)));
  }
}
