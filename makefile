CC = gcc
LIBS = -lm -pthread -fopenmp

mandelbrot: main.o
	$(CC) main.o -o mandelbrot $(LIBS)

main.o: main.c
	$(CC) -c main.c

run: mandelbrot
	./mandelbrot 100 100 50 2

clean:
	rm -f *.o mandelbrot

.PHONY: run clean