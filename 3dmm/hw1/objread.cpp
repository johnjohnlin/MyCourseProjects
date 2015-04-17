#include "objread.hpp"
#include "portab.hpp"
#include <cstdlib>
#include <cstdarg>
#include <cstring>

static void parseerror(const char *fmt, ...)
{
	va_list vl;
	va_start(vl, fmt);
	vfprintf(stderr, fmt, vl);
	va_end(vl);
	putchar('\n');
	puts("Parsing exits with some errors!!");
	exit(2);
}

inline int fpeek(FILE *fp)
{
	int c;
	c = fgetc(fp);
	ungetc(c, fp);
	return c;
}

unsigned* read_unsigned_array(FILE *fp, const unsigned num)
{
	unsigned *ret = new unsigned[num];
	unsigned *dst = ret;
	for (unsigned i = 0; i < num; ++i) {
		fscanf(fp, "%u", dst);
		++dst;
	}
	return ret;
}

float* read_float_array(FILE *fp, const unsigned num)
{
	float *ret = new float[num];
	float *dst = ret;
	for (unsigned i = 0; i < num; ++i) {
		fscanf(fp, "%f", dst);
		++dst;
	}
	return ret;
}

void read_hw1_obj_file(
	FILE *fp, float* &coord, float* &color, unsigned* &indices,
	unsigned &numVert, unsigned &numTri)
{
	char buf[128];

	// Vertice
	fscanf(fp, "%127s", buf);
	if (strcmp("Vertices", buf) != 0) {
		parseerror("Expected Vertices identfier (%s is get)", buf);
	}
	fscanf(fp, "%u", &numVert);
	printf("Reading %u vertices\n", numVert);
	coord = read_float_array(fp, numVert * 3);

	// Color
	fscanf(fp, "%127s", buf);
	if (strcmp("Colors", buf) != 0) {
		color = NULL;
		printf("No color information\n");
		goto noColor;
	} else {
		unsigned numColor;
		fscanf(fp, "%u", &numColor);
		printf("Reading %u vertex colors\n", numColor);
		if (numColor != numVert) {
			parseerror("#vertice != #color");
		}
		color = read_float_array(fp, numColor * 3);
	}

	// Triangle indice
	fscanf(fp, "%127s", buf);
noColor:
	if (strcmp("Triangle_list", buf) != 0) {
		parseerror("Expected Colors or Triangle_list identfier (%s is get)", buf);
	}
	fscanf(fp, "%u", &numTri);
	printf("Reading %u triangles\n", numTri);
	indices = read_unsigned_array(fp, numTri * 3);
}
