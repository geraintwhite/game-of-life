void
draw_circle(Circle circle, char dot)
{
  float deg;
  int y, x;
  int height = circle.radius, width = 2 * circle.radius;

  Point offset;
  offset.y = circle.centre.y - height;
  offset.x = circle.centre.x - width;

  for (deg = 0; deg < 360; deg++)
  {
    y = (int) offset.y + height + (height * sin(DEGTORAD(deg)));
    x = (int) offset.x + width + (width * cos(DEGTORAD(deg)));

    mvaddch(y, x, dot);
  }
}
