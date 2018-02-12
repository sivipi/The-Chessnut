all: chessprog

clean:
	-rm chessprog.o minimax.o console.o gui.o files.o chessprog

chessprog: chessprog.o minimax.o console.o gui.o files.o
	gcc  -o chessprog chessprog.o minimax.o console.o gui.o files.o -lm -std=c99 -pedantic-errors -g `sdl-config --libs`

chessprog.o: chessprog.c
	gcc  -std=c99 -pedantic-errors -c -Wall -g -lm chessprog.c

minimax.o: minimax.c chessprog.o
	gcc  -std=c99 -pedantic-errors -c -Wall -g -lm minimax.c

gui.o: gui.c chessprog.o
	gcc  -std=c99 -pedantic-errors -c -Wall -g -lm gui.c `sdl-config --cflags`

console.o: console.c chessprog.o
	gcc  -std=c99 -pedantic-errors -c -Wall -g -lm console.c

files.o: files.c chessprog.o
	gcc  -std=c99 -pedantic-errors -c -Wall -g -lm files.c
