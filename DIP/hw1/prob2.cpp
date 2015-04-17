#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <algorithm>
#include "pgm.h"

inline int clamp(int x, const int l, const int h)
{
	return std::max(std::min(x, h), l);
}

inline unsigned char clip(int x)
{
	return x&(~0xff) ? ((-x)>>31)&0xff : x;
}

inline float gaussian_pdf(const float sigma)
{
	float s = 0.0f;
	for (int i = 0; i < 12; ++i) {
		s += (float)rand()/RAND_MAX;
	}
	s -= 6.0f;
	s *= sigma;
	return s;
}

inline int ps_pdf(const unsigned char in, const int TH)
{
	int r = rand();
	if (r < TH) {
		return 255;
	}
	if (r < (TH<<1)) {
		return 0;
	}
	return in;
}

void g_noise(unsigned char *out, const unsigned char *in, const int w, const int h)
{
	for (int i = 0; i < w*h; ++i) {
		int x = *in + gaussian_pdf(10.0f);
		*out = clip(x);
		++in;
		++out;
	}
}

void ps_noise(unsigned char *out, const unsigned char *in, const int w, const int h)
{
	const int TH = RAND_MAX/100;
	for (int i = 0; i < w*h; ++i) {
		*out = ps_pdf(*in, TH);
		++in;
		++out;
	}
}

void median_filter(unsigned char *out, const unsigned char *in, const int w, const int h)
{
	for (int i = 0; i < h; ++i) {
		for (int j = 0; j < w; ++j) {
			int im = i == 0? i: i-1;
			int ip = i < h-1 ? i+1: i;
			int jm = j == 0? j: j-1;
			int jp = j < w-1 ? j+1: j;
			unsigned char neigh[9] = {
				in[w*im+jm], in[w*im+j], in[w*im+jp],
				in[w*i +jm], in[w*i +j], in[w*i +jp],
				in[w*ip+jm], in[w*ip+j], in[w*ip+jp]
			};
			std::partial_sort(neigh+0, neigh+5, neigh+9);
#define USE_COND_MEDIAN
#ifdef USE_COND_MEDIAN
			int origVal = in[w*i+j];
			int newVal = neigh[4];

			if (abs(origVal - newVal) > 15) {
				out[w*i+j] = newVal;
			} else {
				out[w*i+j] = origVal;
			}
#else
			out[w*i+j] = neigh[4];
#endif
		}
	}
}

void biliteral_fileter(
		unsigned char *out, const unsigned char *in, const int w, const int h,
		const float sigX, const float sigC
)
{
	int win = sigX*2.0f;
	float gTableC[256];
	float *gTableX = new float[1+win];

	// build table
	for (int i = 0; i < 256; ++i) {
		const static float sigC_SqRcp = 1.0f/(sigC*sigC);
		gTableC[i] = expf((i*i)*(-sigC_SqRcp));
	}

	for (int i = 0; i <= win; ++i) {
		const static float sigX_SqRcp = 1.0f/(sigX*sigX);
		gTableX[i] = expf((i*i)*(-sigX_SqRcp));
	}

	for (int i = 0; i < h; ++i) {
		for (int j = 0; j < w; ++j) {
			float sumW = 0.0f, sumWX = 0.0f;
			unsigned char cur = in[w*i+j];
			// open a window
			for (int k = -win; k <= win; ++k) {
				for (int l = -win; l <= win; ++l) {
					int K = clamp(i+k, 0, h-1);
					int L = clamp(j+l, 0, w-1);
					int x = in[w*K+L];
					int dC = x - cur;
					float w = gTableC[abs(dC)] * gTableX[abs(k)] * gTableX[abs(l)];
					sumW += w;
					sumWX += w*x;
				}
			}
			int x = sumWX / sumW;
			out[w*i+j] = clip(x);
		}
	}
	delete[] gTableX;
}

int main(int argc, char const* argv[])
{
	if (argc != 4) {
		printf("Usage: %s filename w h\n", argv[0]);
		abort();
	}

	int w, h;
	w = atoi(argv[2]);
	h = atoi(argv[3]);

	unsigned char *imgI = new unsigned char[w*h];
	unsigned char *imgNG = new unsigned char[w*h];
	unsigned char *imgNP = new unsigned char[w*h];
	unsigned char *imgNB = new unsigned char[w*h];
	unsigned char *imgRG = new unsigned char[w*h];
	unsigned char *imgRP = new unsigned char[w*h];
	unsigned char *imgRB = new unsigned char[w*h];

	// read file
	FILE *fpIn = fopen(argv[1], "rb");
	fread(imgI, w*h, 1, fpIn);
	fclose(fpIn);

	g_noise(imgNG, imgI, w, h);
	ps_noise(imgNP, imgI, w, h);
	g_noise(imgNB, imgNP, w, h);

	biliteral_fileter(imgRG, imgNG, w, h, 3.0f, 30.0f);
	median_filter(imgRP, imgNP, w, h);

	// use imgI for buffer!
	median_filter(imgI, imgNB, w, h);
	biliteral_fileter(imgRB, imgI, w, h, 3.0f, 30.0f);

	FILE *fpPgmNG = fopen("NG.pgm", "wb");
	FILE *fpPgmNP = fopen("NP.pgm", "wb");
	FILE *fpPgmNB = fopen("NB.pgm", "wb");
	FILE *fpPgmRG = fopen("RG.pgm", "wb");
	FILE *fpPgmRP = fopen("RP.pgm", "wb");
	FILE *fpPgmRB = fopen("RB.pgm", "wb");
	save_P5_pgm(fpPgmNG, w, h, imgNG);
	save_P5_pgm(fpPgmNP, w, h, imgNP);
	save_P5_pgm(fpPgmNB, w, h, imgNB);
	save_P5_pgm(fpPgmRG, w, h, imgRG);
	save_P5_pgm(fpPgmRP, w, h, imgRP);
	save_P5_pgm(fpPgmRB, w, h, imgRB);
	fclose(fpPgmNG);
	fclose(fpPgmNP);
	fclose(fpPgmNB);
	fclose(fpPgmRG);
	fclose(fpPgmRP);
	fclose(fpPgmRB);
	delete[] imgNG, imgNP, imgNB, imgRG, imgRP, imgRB;
	return 0;
}
