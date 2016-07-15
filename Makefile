CC=g++
CFLAGS=--std=c++11
LIBS=-lSDL2 -lSDL2_mixer -lSDL2_image
DEPS = Chip8.h

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

chip8: Chip8.o main.o
	$(CC) $(LIBS) -o chip8 Chip8.o main.o
