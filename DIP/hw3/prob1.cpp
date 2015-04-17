#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "pgm.h"
#include "morph.h"

int main(int argc, char const* argv[])
{
	if (argc != 4) {
		printf("Usage: %s filename w h\n", argv[0]);
		abort();
	}

	int w, h;
	w = atoi(argv[2]);
	h = atoi(argv[3]);

	unsigned char *img   = new unsigned char[w*h];
	unsigned char *imgME = new unsigned char[w*h];
	unsigned char *imgLb = new unsigned char[w*h];
	FILE *fpIn = fopen(argv[1], "rb");
	fread(img, w*h, 1, fpIn);
	fclose(fpIn);

	morph_edge(img, imgME, w, h);
	int numConn = label_image(img, imgLb, w, h);
	int numCirc = erosion_object(img, w, h, 21);
	printf("I detect %d objects and %d circles\n", numConn, numCirc);

	FILE *fpPgmME = fopen("ME.pgm", "wb");
	FILE *fpPgmLb = fopen("Lb.pgm", "wb");
	save_P5_pgm(fpPgmME, w, h, imgME);
	save_P5_pgm(fpPgmLb, w, h, imgLb);
	fclose(fpPgmME);
	fclose(fpPgmLb);
	delete[] img, imgME, imgLb;
	return 0;
}
