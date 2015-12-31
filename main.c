#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ncurses.h>

#include "shapes.c"
#include "gol.h"

#define N_BUFFERS 10


typedef struct
{
  bool state;
} Cell;

typedef struct
{
  int size;
  Cell * cells;
} Cells;

typedef struct
{
  Cells buffers[N_BUFFERS];
  Cells tmp_buf;
  int head;
} CellBuffers;

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
tick(CellBuffers * cell_buffers)
{
  int y, x, n;

  Cells * head_buf = cell_buffers->buffers + cell_buffers->head;
  memcpy(cell_buffers->tmp_buf.cells, head_buf->cells, cell_buffers->tmp_buf.size);

  for (y = 0; y < HEIGHT; y++)
  {
    for (x = 0; x < WIDTH; x++)
    {
      // set 'alive' state of each cell depending on the number of neighbours
      n = neighbours(head_buf, y, x);

      Cell * current_cell = cell_buffers->tmp_buf.cells + COORD(y, x);

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

  memcpy(head_buf->cells, cell_buffers->tmp_buf.cells, head_buf->size);

  refresh();
}

bool
keyboard(int c, CellBuffers * cell_buffers)
{
  int y, x;
  getyx(stdscr, y, x);

  Cells * head_buf = cell_buffers->buffers + cell_buffers->head;

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
      Cell * cursor_cell = head_buf->cells + COORD(y, x);
      update_cell(head_buf, y, x, !(cursor_cell->state));
    } break;
    case 't':
    {
      draw_mode = !draw_mode;
    } break;
    case 10:
    {
      tick(cell_buffers);
    } break;
    default:
    {
      return false;
    }
  }

  if (draw_mode) update_cell(head_buf, y, x, true);

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
new_cell_buffer(Cells * cells, int size)
{
  cells->size = size;
  cells->cells = malloc(cells->size);
}

void
init_game(CellBuffers * cell_buffers)
{
  int size = WIDTH * HEIGHT * sizeof(int);

  int buf_index;
  for (buf_index = 0;
       buf_index < N_BUFFERS;
       ++buf_index)
  {
    new_cell_buffer((cell_buffers->buffers + buf_index), size);
  }

  new_cell_buffer(&(cell_buffers->tmp_buf), size);

  cell_buffers->head = 0;

  add_circle(cell_buffers->buffers + cell_buffers->head, 1 * HEIGHT / 4, 1 * WIDTH / 4, (2 * HEIGHT > WIDTH ? WIDTH / 4 : HEIGHT) / 4);
  add_circle(cell_buffers->buffers + cell_buffers->head, 3 * HEIGHT / 4, 1 * WIDTH / 4, (2 * HEIGHT > WIDTH ? WIDTH / 4 : HEIGHT) / 4);
  add_circle(cell_buffers->buffers + cell_buffers->head, 3 * HEIGHT / 4, 3 * WIDTH / 4, (2 * HEIGHT > WIDTH ? WIDTH / 4 : HEIGHT) / 4);
  add_circle(cell_buffers->buffers + cell_buffers->head, 1 * HEIGHT / 4, 3 * WIDTH / 4, (2 * HEIGHT > WIDTH ? WIDTH / 4 : HEIGHT) / 4);
  add_circle(cell_buffers->buffers + cell_buffers->head, 2 * HEIGHT / 4, 2 * WIDTH / 4, (2 * HEIGHT > WIDTH ? WIDTH / 4 : HEIGHT) / 4);

  move(0, 0);
}

void
deinit(CellBuffers * cell_buffers)
{
  endwin();

  int buf_index;
  for (buf_index = 0;
       buf_index < N_BUFFERS;
       ++buf_index)
  {
    free((cell_buffers->buffers + buf_index)->cells);
  }
  free(cell_buffers->tmp_buf.cells);
}

int
main()
{
  init_curses();

  CellBuffers cell_buffers;
  init_game(&cell_buffers);

  while (keyboard(getch(), &cell_buffers));

  deinit(&cell_buffers);

  return 0;
}
