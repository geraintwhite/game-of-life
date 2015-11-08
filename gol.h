#define HEIGHT (LINES)
#define WIDTH (COLS)
#define SIZE (HEIGHT * WIDTH * sizeof(int))

#define COORD(y, x) ((y) * WIDTH + (x))
#define CELL(y, x) (cells[COORD((y), (x))])
#define BUFFER(y, x) (buffer[COORD((y), (x))])

#define ALIVE(y, x) (CELL((y), (x)) == true)
