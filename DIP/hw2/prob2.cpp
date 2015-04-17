#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include "pgm.h"

void project(
	const unsigned char *imgI, unsigned char *imgO,
	const int wI, const int hI, const int wO, const int hO)
{
	const float rxScale = (float)wI/(hO*2);
	const float ryScale = (float)hI/(hO*2);
	const float tScale = 2.0f * M_PI / wO;
	const int wI2 = (wI+1)/2;
	const int hI2 = (hI+1)/2;
	const int wO2 = (wO+1)/2;
	const int hO2 = (hO+1)/2;

	for (int i = 0; i < hO; ++i) {
		for (int j = 0; j < wO; ++j) {
			float t = tScale * j;
			float ct = cosf(t);
			float st = sinf(t);
			int x = wI2 + (hO-i)*rxScale*ct;
			int y = hI2 + (hO-i)*ryScale*st;
			imgO[wO*i+j] = imgI[wI*y+x];
		}
	}
}

int main(int argc, char const* argv[])
{
	if (argc != 6) {
		printf("Usage: %s filename wIn hIn wOut hOut\n", argv[0]);
		abort();
	}

	int wI, hI, wO, hO;
	wI = atoi(argv[2]);
	hI = atoi(argv[3]);
	wO = atoi(argv[4]);
	hO = atoi(argv[5]);

	unsigned char *imgI = new unsigned char[wI*hI];
	unsigned char *imgO = new unsigned char[wO*hO];

	// read file
	FILE *fpIn = fopen(argv[1], "rb");
	fread(imgI, wI*hI, 1, fpIn);
	fclose(fpIn);

	project(imgI, imgO, wI, hI, wO, hO);

	FILE *fpOut = fopen("Pano.pgm", "wb");
	save_P5_pgm(fpOut, wO, hO, imgO);
	fclose(fpOut);
	delete[] imgI, imgO;
	return 0;
}
