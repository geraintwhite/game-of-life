#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ncurses.h>

#include "shapes.c"


#define HEIGHT (LINES)
#define WIDTH (COLS)

#define WIN_HEIGHT (10)
#define WIN_WIDTH (10)
#define WIN_STARTX (WIDTH - WIN_WIDTH)
#define WIN_STARTY (0)

#define COORD(y, x) ((y) * WIDTH + (x))

#define N_BUFFERS (10)


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
  Cells buffers[N_BUFFERS];
  Cells tmp_buf;
  Cells * head;
} CellBuffers;

typedef struct
{
  bool stats;
  bool line;
  bool trace;
  int line_sy;
  int line_sx;
} State;

static int DOT = COLOR_PAIR(1) | ' ';


void
update_cell(Cells * cells, int y, int x, bool state)
{
  Cell * cell = cells->cells + COORD(y, x);
  cell->state = state;
  mvaddch(y, x, state ? DOT : ' ');
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
update_stats(State * state)
{
  mvprintw(WIN_STARTY + 0, WIN_STARTX + 0, "STATE");
  mvprintw(WIN_STARTY + 2, WIN_STARTX + 0, "LINE %d", state->line);
  mvprintw(WIN_STARTY + 3, WIN_STARTX + 0, "TRACE %d", state->trace);
}

void
clear_stats()
{
  int y, x;
  for (y = 0; y < WIN_HEIGHT; y++)
  {
    for (x = 0; x < WIN_WIDTH; x++)
    {
      mvaddch(WIN_STARTY + y, WIN_STARTX + x, ' ');
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

  int len = sqrt(w*w + h*h);

  int i;
  for (i = 0; i <= len; i++)
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

  memcpy(cell_buffers->tmp_buf.cells, cell_buffers->head->cells, cell_buffers->buffer_size);

  for (y = 0; y < HEIGHT; y++)
  {
    for (x = 0; x < WIDTH; x++)
    {
      // set 'alive' state of each cell depending on the number of neighbours
      n = neighbours(cell_buffers->head, y, x);

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

  memcpy(cell_buffers->head->cells, cell_buffers->tmp_buf.cells, cell_buffers->buffer_size);

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
      Cell * cursor_cell = cell_buffers->head->cells + COORD(y, x);
      update_cell(cell_buffers->head, y, x, !(cursor_cell->state));
    } break;
    case 't':
    {
      state->trace = !state->trace;
    } break;
    case 's':
    {
      memcpy((cell_buffers->head+1)->cells, cell_buffers->head->cells, cell_buffers->buffer_size);
      cell_buffers->head++;
    } break;
    case 'l':
    {
      if (state->line) add_line(cell_buffers->head, state->line_sy, state->line_sx, y, x);
      state->line_sy = y;
      state->line_sx = x;
      state->line = !state->line;
    } break;
    case 'c':
    {
      reset_cells(cell_buffers->head, false);
    } break;
    case 10:
    {
      tick(cell_buffers);
    } break;
    case 'q':
    {
      return false;
    } break;
    case '?':
    {
      state->stats = !state->stats;
      clear_stats();
    } break;
    default:
    {
      if ('0' <= c && c <= '9')
      {
        cell_buffers->head = cell_buffers->buffers + (c - '0');
        draw_buffer(cell_buffers->head);
      }
    }
  }

  if (state->trace) update_cell(cell_buffers->head, y, x, true);
  if (state->stats) update_stats(state);

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
  state->stats = true;
  state->line = false;
  state->trace = false;

  cell_buffers->buffer_size = WIDTH * HEIGHT * sizeof(int);

  int buf_index;
  for (buf_index = 0;
       buf_index < N_BUFFERS;
       ++buf_index)
  {
    new_cell_buffer((cell_buffers->buffers + buf_index), cell_buffers->buffer_size);
  }

  new_cell_buffer(&(cell_buffers->tmp_buf), cell_buffers->buffer_size);

  cell_buffers->head = cell_buffers->buffers;

  add_circle(cell_buffers->head, 1 * HEIGHT / 4, 1 * WIDTH / 4, (2 * HEIGHT > WIDTH ? WIDTH / 4 : HEIGHT) / 4);
  add_circle(cell_buffers->head, 3 * HEIGHT / 4, 1 * WIDTH / 4, (2 * HEIGHT > WIDTH ? WIDTH / 4 : HEIGHT) / 4);
  add_circle(cell_buffers->head, 3 * HEIGHT / 4, 3 * WIDTH / 4, (2 * HEIGHT > WIDTH ? WIDTH / 4 : HEIGHT) / 4);
  add_circle(cell_buffers->head, 1 * HEIGHT / 4, 3 * WIDTH / 4, (2 * HEIGHT > WIDTH ? WIDTH / 4 : HEIGHT) / 4);
  add_circle(cell_buffers->head, 2 * HEIGHT / 4, 2 * WIDTH / 4, (2 * HEIGHT > WIDTH ? WIDTH / 4 : HEIGHT) / 4);

  update_stats(state);

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
  CellBuffers cell_buffers;
  State state;

  init_curses();
  init_game(&state, &cell_buffers);

  while (keyboard(&state, &cell_buffers, getch()));

  deinit(&cell_buffers);

  return 0;
}
