#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "pgm.h"
#include "edge.h"

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
	unsigned char *imgE1 = new unsigned char[w*h];
	unsigned char *imgE2 = new unsigned char[w*h];
	unsigned char *imgE3 = new unsigned char[w*h];
	FILE *fpIn = fopen(argv[1], "rb");
	fread(img, w*h, 1, fpIn);
	fclose(fpIn);

	canny(w, h, img, imgE1, 0, 13);
	sobel(w, h, img, imgE2, 15);
	zero_cross(w, h, img, imgE3, 20);

	FILE *fpPgmE1 = fopen("E1.pgm", "wb");
	FILE *fpPgmE2 = fopen("E2.pgm", "wb");
	FILE *fpPgmE3 = fopen("E3.pgm", "wb");
	save_P5_pgm(fpPgmE1, w, h, imgE1);
	save_P5_pgm(fpPgmE2, w, h, imgE2);
	save_P5_pgm(fpPgmE3, w, h, imgE3);
	fclose(fpPgmE1);
	fclose(fpPgmE2);
	fclose(fpPgmE3);
	delete[] imgE1, imgE2, imgE3;
	return 0;
}
