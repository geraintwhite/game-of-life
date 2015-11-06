#define SIZE (LINES * COLS * sizeof(int))

#define COORD(y, x) ((y) * COLS + (x))
#define CELL(y, x) (cells[COORD((y), (x))])

#define ALIVE(y, x) (CELL((y), (x)))
#define LIVE(y, x) (cells[COORD((y), (x))] = 1)
#define KILL(y, x) (cells[COORD((y), (x))] = 0)
