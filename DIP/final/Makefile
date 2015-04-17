# CFLAG=-c -g -DNDEBUG -Wall
CFLAG=-c -O3 -DNDEBUG -Wall

perlin: main.o pgm.o GradNoise2D.o -lGL -lglut
	g++ $^ -o $@

main.o: main.cpp pgm.h GradNoise2D.h
	g++ $(CFLAG) $<

%.o: %.cpp %.h
	g++ $(CFLAG) $<
