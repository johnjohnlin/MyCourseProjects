#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "pgm.h"
#include "texture.h"

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
	unsigned char *imgTS = new unsigned char[w*h];
	Feature *feats = new Feature[w*h];
	FILE *fpIn = fopen(argv[1], "rb");
	fread(img, w*h, 1, fpIn);
	fclose(fpIn);

	Feature::from_pixels(feats, img, w, h);
	for (int i = 0; i < w*h; ++i) {
		feats[i].sift_normalize();
	}
	printf("---\nStart running kmeans with 5 classes\n---\n");
	int nIter = Feature::k_means(feats, w*h, 5, 100, 0.0f);
	printf("K-means stops after %d iteration...\n", nIter);

	for (int i = 0; i < w*h; ++i) {
		imgTS[i] = feats[i].cls * 63;
	}

	FILE *fpPgmTS = fopen("TexSeg.pgm", "wb");
	save_P5_pgm(fpPgmTS, w, h, imgTS);
	fclose(fpPgmTS);
	delete[] img, imgTS, feats;
	return 0;
}
