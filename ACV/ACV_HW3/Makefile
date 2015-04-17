DF=-O2
CF=-c $(DF)

all: main.o OpticalFlow.o pgm.o
	g++ $^ $(DF) -o of

main.o: main.cpp OpticalFlow.hpp VectorType.hpp
	g++ $< -o $@ $(CF)

OpticalFlow.o: OpticalFlow.cpp OpticalFlow.hpp VectorType.hpp
	g++ $< -o $@ $(CF)

pgm.o: pgm.cpp pgm.h
	g++ $< -o $@ $(CF)

clean:
	rm -f *.o of
