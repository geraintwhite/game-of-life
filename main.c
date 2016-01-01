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
  Cell * cells;
} Cells;

typedef struct
{
  int buffer_size;
  int next_buff;
  Cells buffers[N_BUFFERS];
  Cells tmp_buf;
  Cells head;
} CellBuffers;

typedef struct
{
  bool line;
  int line_sy;
  int line_sx;
  bool circle;
  int circle_y;
  int circle_x;
} State;

static int DOT = COLOR_PAIR(1) | ' ';
static bool draw_mode = false;


void
update_cell(Cells * cells, int y, int x, bool state)
{
  if (y >= 0 &&
      x >= 0 &&
      y < HEIGHT &&
      x < WIDTH)
  {
    Cell * cell = cells->cells + COORD(y, x);
    cell->state = state;
    mvaddch(y, x, state ? DOT : ' ');
  }
}

void
reset_cells(Cells * cells, bool state)
{
  int y, x;
  for (y = 0; y < HEIGHT; y++)
  {
    for (x = 0; x < WIDTH; x++)
    {
      update_cell(cells, y, x, state);
    }
  }
}

void
draw_buffer(Cells * cells)
{
  int y, x;
  for (y = 0; y < HEIGHT; y++)
  {
    for (x = 0; x < WIDTH; x++)
    {
      Cell * cell = cells->cells + COORD(y, x);
      mvaddch(y, x, cell->state ? DOT : ' ');
    }
  }
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

void
add_line(Cells * cells, int sy, int sx, int ey, int ex)
{
  int w = ex - sx;
  int h = ey - sy;

  w += w > 0 ? 1 : -1;
  h += h > 0 ? 1 : -1;

  int len = sqrt(w*w + h*h);

  int i;
  for (i = 0; i < len; i++)
  {
    int dy = (float)(h * i) / len;
    int dx = (float)(w * i) / len;
    update_cell(cells, sy + dy, sx + dx, true);
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

  memcpy(cell_buffers->tmp_buf.cells, cell_buffers->head.cells, cell_buffers->buffer_size);

  for (y = 0; y < HEIGHT; y++)
  {
    for (x = 0; x < WIDTH; x++)
    {
      // set 'alive' state of each cell depending on the number of neighbours
      n = neighbours(&(cell_buffers->head), y, x);

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

  memcpy(cell_buffers->head.cells, cell_buffers->tmp_buf.cells, cell_buffers->buffer_size);

  refresh();
}

bool
keyboard(State * state, CellBuffers * cell_buffers, int c)
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
      Cell * cursor_cell = cell_buffers->head.cells + COORD(y, x);
      update_cell(&(cell_buffers->head), y, x, !(cursor_cell->state));
    } break;
    case 't':
    {
      draw_mode = !draw_mode;
    } break;
    case 's':
    {
      memcpy((cell_buffers->buffers + cell_buffers->next_buff++)->cells, cell_buffers->head.cells, cell_buffers->buffer_size);
    } break;
    case 'l':
    {
      if (state->line)
      {
        add_line(&(cell_buffers->head), state->line_sy, state->line_sx, y, x);
        state->line = false;
      }
      else
      {
        state->line_sy = y;
        state->line_sx = x;
        state->line = true;
      }
    } break;
    case 'o':
    {
      if (state->circle)
      {
        int radius = sqrt(pow(y - state->circle_y, 2) + pow((x - state->circle_x) / 2, 2));
        add_circle(&(cell_buffers->head), state->circle_y, state->circle_x, radius);
        state->circle = false;
      }
      else
      {
        state->circle_y = y;
        state->circle_x = x;
        state->circle = true;
      }
    } break;
    case 'c':
    {
      reset_cells(&(cell_buffers->head), false);
    } break;
    case 10:
    {
      tick(cell_buffers);
    } break;
    case 'q':
    {
      return false;
    } break;
    default:
    {
      if ('0' <= c && c <= '9')
      {
        memcpy(cell_buffers->head.cells, (cell_buffers->buffers + (c - '0'))->cells, cell_buffers->buffer_size);
        draw_buffer(&(cell_buffers->head));
      }
    }
  }

  if (draw_mode) update_cell(&(cell_buffers->head), y, x, true);

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
  cells->cells = malloc(size);
}

void
init_game(State * state, CellBuffers * cell_buffers)
{
  state->line = false;
  state->circle = false;

  cell_buffers->next_buff = 0;
  cell_buffers->buffer_size = WIDTH * HEIGHT * sizeof(int);

  int buf_index;
  for (buf_index = 0;
       buf_index < N_BUFFERS;
       ++buf_index)
  {
    new_cell_buffer((cell_buffers->buffers + buf_index), cell_buffers->buffer_size);
  }

  new_cell_buffer(&(cell_buffers->head), cell_buffers->buffer_size);
  new_cell_buffer(&(cell_buffers->tmp_buf), cell_buffers->buffer_size);

  add_circle(&(cell_buffers->head), 1 * HEIGHT / 4, 1 * WIDTH / 4, (2 * HEIGHT > WIDTH ? WIDTH / 4 : HEIGHT) / 4);
  add_circle(&(cell_buffers->head), 3 * HEIGHT / 4, 1 * WIDTH / 4, (2 * HEIGHT > WIDTH ? WIDTH / 4 : HEIGHT) / 4);
  add_circle(&(cell_buffers->head), 3 * HEIGHT / 4, 3 * WIDTH / 4, (2 * HEIGHT > WIDTH ? WIDTH / 4 : HEIGHT) / 4);
  add_circle(&(cell_buffers->head), 1 * HEIGHT / 4, 3 * WIDTH / 4, (2 * HEIGHT > WIDTH ? WIDTH / 4 : HEIGHT) / 4);
  add_circle(&(cell_buffers->head), 2 * HEIGHT / 4, 2 * WIDTH / 4, (2 * HEIGHT > WIDTH ? WIDTH / 4 : HEIGHT) / 4);

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
  State state;
  init_game(&state, &cell_buffers);

  while (keyboard(&state, &cell_buffers, getch()));

  deinit(&cell_buffers);

  return 0;
}
