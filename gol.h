#define SIZE (LINES * COLS * sizeof(int))

#define COORD(y, x) ((y) * COLS + (x))
#define CELL(y, x) (cells[COORD((y), (x))])
#define BUFFER(y, x) (buffer[COORD((y), (x))])

#define ALIVE(y, x) (CELL((y), (x)))

#define true 1
#define false 0
