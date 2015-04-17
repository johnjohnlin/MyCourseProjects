#include <stdio.h>
#include <stdlib.h>

#include "pgm.h"
/** @brief load P5 format pgm
  *
  */
void load_P5_pgm(FILE *fp, int *w, int *h, unsigned char **ptr)
{
	unsigned norm;
	fscanf(fp, "P5\n%d %d\n%u\n", w, h, &norm);
	*ptr = (unsigned char*)malloc((*w)*(*h)*sizeof(unsigned char));
	if (norm == 255) {
		fread(*ptr, 1, (*w)*(*h)*sizeof(unsigned char), fp);
	} else {
		unsigned i;
		for (i = 0; i < (*w)*(*h); ++i) {
			unsigned char c = fgetc(fp);
			(*ptr)[i] = c*255/norm;
		}
	}
}

void save_P5_pgm(FILE *fp, int w, int h, unsigned char  *ptr)
{
	fprintf(fp, "P5\n%d %d\n255\n", w, h);
	fwrite(ptr, 1, w*h*sizeof(unsigned char), fp);
}

