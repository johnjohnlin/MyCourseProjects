#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "pgm.h"
#include "texture.h"

int main(int argc, char const* argv[])
{
	int w, h, n;
	if (argc < 4) {
		printf("Usage: %s nFile w h fileList\n", argv[0]);
		abort();
	}
	n = atoi(argv[1]);
	if (argc != n+4) {
		printf("# files in list mismatch!\n", argv[0]);
		abort();
	}
	w = atoi(argv[2]);
	h = atoi(argv[3]);

	unsigned char *img = new unsigned char[w*h];
	Feature *feats = new Feature[n];

	printf("---\nStart Law's feature extraction\n---\n");
	for (int i = 0; i < n; ++i) {
		FILE *fpIn = fopen(argv[i+4], "rb");
		fread(img, w*h, 1, fpIn);
		fclose(fpIn);
		feats[i].from_image(img, w, h);
		printf("Feature of %s: ", argv[4+i]);
		feats[i].dump();
	}

	printf("---\nStart running kmeans with 4 classes, %d images...\n---\n", n);
	int nIter = Feature::k_means(feats, n, 4, 100, 1.0f);

	printf("After %d iteration...\n", nIter);
	for (int i = 0; i < n; ++i) {
		printf("%s belongs to class %d\n", argv[4+i], feats[i].cls);
	}

	delete[] img, feats;
	return 0;
}
