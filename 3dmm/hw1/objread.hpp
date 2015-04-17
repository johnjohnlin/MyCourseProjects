#ifndef __OBJREAD_H__
#define __OBJREAD_H__

#include <cstdio>

unsigned* read_unsigned_array(FILE *fp, const unsigned num);
float* read_float_array(FILE *fp, const unsigned num);

void read_hw1_obj_file(
	FILE *fp, float* &coord, float* &color, unsigned* &indices,
	unsigned &numVert, unsigned &numTri);

#endif /* end of include guard: __OBJREAD_H__ */
