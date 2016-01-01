#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ncurses.h>

#include "shapes.c"


#define HEIGHT (LINES)
#define WIDTH (COLS)

#define WIN_HEIGHT (HEIGHT)
#define WIN_WIDTH (25)
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
  int next_buff;
  Cells buffers[N_BUFFERS];
  Cells tmp_buf;
  Cells head;
} CellBuffers;

typedef struct
{
  bool stats;
  bool line;
  bool trace;
  int line_sy;
  int line_sx;
  int line_ey;
  int line_ex;

  bool circle;
  int circle_y;
  int circle_x;
  int circle_r;
} State;

static int DOT = COLOR_PAIR(1) | ' ';
static int GUIDE = COLOR_PAIR(2) | ' ';


void
update_cell(Cells * cells, int y, int x, int state, int chr)
{
  if (y >= 0 &&
      x >= 0 &&
      y < HEIGHT &&
      x < WIDTH)
  {
    if (cells != NULL)
    {
      Cell * cell = cells->cells + COORD(y, x);
      if (state == 2)
      {
        state = cell->state;
      }
      else
      {
        cell->state = state;
      }
    }
    mvaddch(y, x, state ? chr : ' ');
  }
}

void
reset_cells(Cells * cells, int state)
{
  int y, x;
  for (y = 0; y < HEIGHT; y++)
  {
    for (x = 0; x < WIDTH; x++)
    {
      update_cell(cells, y, x, state, DOT);
    }
  }
}

void
draw_buffer_range(Cells * cells, int sy, int sx, int ey, int ex)
{
  ey += SIGN(ey - sy);
  ex += SIGN(ex - sx);

  if (sy > ey)
  {
    int tmp = sy;
    sy = ey;
    ey = tmp;
  }
  if (sx > ex)
  {
    int tmp = sx;
    sx = ex;
    ex = tmp;
  }

  int y, x;
  for (y = sy; y <= ey; y++)
  {
    for (x = sx; x <= ex; x++)
    {
      Cell * cell = cells->cells + COORD(y, x);
      mvaddch(y, x, cell->state ? DOT : ' ');
    }
  }
}

void
draw_buffer(Cells * cells)
{
  draw_buffer_range(cells, 0, 0, HEIGHT, WIDTH);
}

void
update_stats(State * state)
{
  mvprintw(WIN_STARTY + 0, WIN_STARTX, "STATE");
  mvprintw(WIN_STARTY + 2, WIN_STARTX, "trace %d", state->trace);
  mvprintw(WIN_STARTY + 3, WIN_STARTX, "line %d", state->line);
  mvprintw(WIN_STARTY + 4, WIN_STARTX, "circle %d", state->circle);
  mvprintw(WIN_STARTY + 7, WIN_STARTX, "HELP");
  mvprintw(WIN_STARTY + 9, WIN_STARTX, "trace (t)");
  mvprintw(WIN_STARTY + 10, WIN_STARTX, "circle (o)");
  mvprintw(WIN_STARTY + 11, WIN_STARTX, "line (l)");
  mvprintw(WIN_STARTY + 12, WIN_STARTX, "clear (c)");
  mvprintw(WIN_STARTY + 13, WIN_STARTX, "save to buffer (s)");
  mvprintw(WIN_STARTY + 14, WIN_STARTX, "load from buffer (0-9)");
  mvprintw(WIN_STARTY + 15, WIN_STARTX, "next frame (enter)");
  mvprintw(WIN_STARTY + 16, WIN_STARTX, "quit (q)");
  mvprintw(WIN_STARTY + 17, WIN_STARTX, "toggle sidebar (?)");
}

void
add_circle(Cells * cells, int y, int x, int radius, int chr, int state)
{
  int i;

  Point points[360];
  get_points((Circle) {radius, (Point) {y, x}}, points);

  for (i = 0; i < 360; i++)
  {
    update_cell(cells, points[i].y, points[i].x, state, chr);
  }
}

void
add_line(Cells * cells, int sy, int sx, int ey, int ex, int chr, int state)
{
  int w = ex - sx;
  int h = ey - sy;
  w += SIGN(w);
  h += SIGN(h);

  int len = sqrt(w*w + h*h);

  int i;
  for (i = 0; i < len; i++)
  {
    int dy = (float)(h * i) / len;
    int dx = (float)(w * i) / len;
    update_cell(cells, sy + dy, sx + dx, state, chr);
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
      update_cell(&(cell_buffers->head), y, x, !(cursor_cell->state), DOT);
    } break;
    case 't':
    {
      state->trace = !state->trace;
    } break;
    case 's':
    {
      memcpy((cell_buffers->buffers + ++cell_buffers->next_buff)->cells, cell_buffers->head.cells, cell_buffers->buffer_size);
    } break;
    case 'l':
    {
      if (state->line)
      {
        add_line(&(cell_buffers->head), state->line_sy, state->line_sx, state->line_ey, state->line_ex, DOT, 1);
        state->line = false;
      }
      else
      {
        state->line_sy = y;
        state->line_sx = x;
        state->line_ey = y;
        state->line_ex = x;
        state->line = true;
      }
    } break;
    case 'o':
    {
      if (state->circle)
      {
        add_circle(&(cell_buffers->head), state->circle_y, state->circle_x, state->circle_r, DOT, 1);
        state->circle = false;
      }
      else
      {
        state->circle_y = y;
        state->circle_x = x;
        state->circle_r = 0;
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
    case '?':
    {
      state->stats = !state->stats;
      draw_buffer_range(&(cell_buffers->head), WIN_STARTY, WIN_STARTX, WIN_STARTY + HEIGHT, WIN_STARTX + WIDTH);
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

  if (state->trace) update_cell(&(cell_buffers->head), y, x, true, DOT);
  if (state->stats) update_stats(state);

  if (state->circle)
  {
    add_circle(&(cell_buffers->head), state->circle_y, state->circle_x, state->circle_r, DOT, 2);
  }
  if (state->line)
  {
    add_line(&(cell_buffers->head), state->line_sy, state->line_sx, state->line_ey, state->line_ex, DOT, 2);
  }

  if (state->line)
  {
    state->line_ey = y;
    state->line_ex = x;
    add_line(NULL, state->line_sy, state->line_sx, state->line_ey, state->line_ex, GUIDE, 1);
  }

  if (state->circle)
  {
    state->circle_r = sqrt(pow(y - state->circle_y, 2) + pow((x - state->circle_x) / 2, 2));
    add_circle(NULL, state->circle_y, state->circle_x, state->circle_r, GUIDE, 1);
  }

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
  init_pair(2, COLOR_BLACK, COLOR_YELLOW);

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
  state->circle = false;
  state->trace = false;

  cell_buffers->next_buff = -1;
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

  add_circle(&(cell_buffers->head), 1 * HEIGHT / 4, 1 * WIDTH / 4, (2 * HEIGHT > WIDTH ? WIDTH / 4 : HEIGHT) / 4, DOT, 1);
  add_circle(&(cell_buffers->head), 3 * HEIGHT / 4, 1 * WIDTH / 4, (2 * HEIGHT > WIDTH ? WIDTH / 4 : HEIGHT) / 4, DOT, 1);
  add_circle(&(cell_buffers->head), 3 * HEIGHT / 4, 3 * WIDTH / 4, (2 * HEIGHT > WIDTH ? WIDTH / 4 : HEIGHT) / 4, DOT, 1);
  add_circle(&(cell_buffers->head), 1 * HEIGHT / 4, 3 * WIDTH / 4, (2 * HEIGHT > WIDTH ? WIDTH / 4 : HEIGHT) / 4, DOT, 1);
  add_circle(&(cell_buffers->head), 2 * HEIGHT / 4, 2 * WIDTH / 4, (2 * HEIGHT > WIDTH ? WIDTH / 4 : HEIGHT) / 4, DOT, 1);

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
