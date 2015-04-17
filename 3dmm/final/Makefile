CFLAGS=-c -g -fopenmp
INC=-I/opt/cuda/include

all: main.o sift.o utils.o pgm.o
	g++ *.o -fopenmp -lOpenCL

main.o: main.cpp sift.h
	g++ $< $(CFLAGS)

sift.o: sift.cpp utils.h clstruct.h
	g++ $< $(CFLAGS) $(INC)

utils.o: utils.cpp utils.h clstruct.h
	g++ $< $(CFLAGS) $(INC)

pgm.o: pgm.cpp pgm.h
	g++ $< $(CFLAGS)
