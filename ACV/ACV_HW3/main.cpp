#include "VectorType.hpp"
#include "OpticalFlow.hpp"
#include "pgm.h"
#include <cstdio>
#include <cstring>
#include <iostream>
#include <string>
#include <algorithm>
using std::string;
using std::copy;
using std::cout;
using std::endl;

int main(int argc, char const* argv[])
{ try { int w1, h1, w2, h2;
		unsigned char *im1_uc, *im2_uc;
		float *im1, *im2;

		// Open images
		if (argc != 3) {
			throw string("Argrmant number error");
		}

		FILE *fp1 = fopen(argv[1], "rb");
		FILE *fp2 = fopen(argv[2], "rb");
		if (!(fp1 && fp2)) {
			throw string("Cannot open some files");
		}
		load_P5_pgm(fp1, &w1, &h1, &im1_uc);
		load_P5_pgm(fp2, &w2, &h2, &im2_uc);
		fclose(fp1);
		fclose(fp2);
		if (w1 != w2 || h1 != h2) {
			throw string("Image size error");
		}

		// Create floating point images
		const int w = w1;
		const int h = h1;
		im1 = new float [w*h];
		im2 = new float [w*h];
		copy(im1_uc, im1_uc+w*h, im1);
		copy(im2_uc, im2_uc+w*h, im2);
		delete[] im1_uc;
		delete[] im2_uc;

		// Run optical flow
		OpticalFlow of;
		of.lambda = .1;
		of.load_images(im1, im2, w, h);
		memset(of.motion, 0, sizeof(FVec2)*w*h);
		for (int i = 0; i < 64; ++i) {
			of.run_iteration();
			if (i == 0 || i == 10 || i == 63) {
				char fname[16];
				sprintf(fname, "iter_%d.txt", i+1);
				FILE *fp = fopen(fname, "w");
				fprintf(fp, "%d %d\n", w, h);
				const FVec2 *m = of.motion;
				for (int i = 0; i < w*h; ++i) {
					fprintf(fp, "%f %f\n", (*m)[0], (*m)[1]);
					++m;
				}
				fclose(fp);
			}
		}

		delete[] im1;
		delete[] im2;

		return 0;
	} catch (string errMsg) {
		cout << "Somethine error: " << errMsg << endl;
		return -1;
	}
}
