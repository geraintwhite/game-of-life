#define HEIGHT (LINES)
#define WIDTH (COLS)

#define COORD(y, x) ((y) * WIDTH + (x))
#define CELL(y, x) (cells->cells[COORD((y), (x))])
#define BUFFER(y, x) (buffer[COORD((y), (x))])
