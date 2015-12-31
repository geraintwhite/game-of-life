CC         = gcc
CFLAGS     = -g -Werror
LIBS       = -lm -lncurses
SOURCES    = main.c
EXECUTABLE = game-of-life

all:
	$(CC) $(CFLAGS) $(SOURCES) $(LIBS) -o $(EXECUTABLE)

clean:
	find . -name '*.o' -type f -delete
	rm $(EXECUTABLE)
