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

void dump_hist(const unsigned *in)
{
	printf("Hist:\n");
	for (int i = 0; i < 256; ++i) {
		printf("%u\n", in[i]);
	}
	printf("----\n");
}

void calc_hist(const unsigned char *in, unsigned *const hist, const int siz)
{
	memset(hist, 0, 256*sizeof(unsigned));
	for (int i = 0; i < siz; ++i) {
		++(hist[*in]);
		++in;
	}
}

void transfunc_mapping(unsigned char *out, const unsigned char *in, unsigned *const trFunc, const int siz)
{
	for (int i = 0; i < siz; ++i) {
		*out = trFunc[*in];
		++in;
		++out;
	}
}

void divided_by2(unsigned char *out, const unsigned char *in, const int w, const int h)
{
	int iSiz = w*h;
	for (int i = 0; i < iSiz; ++i) {
		unsigned short t = *in;
		*out = (t + 1) >> 1;
		++in;
		++out;
	}

	{
		unsigned hist[256];
		calc_hist(out-iSiz, hist, iSiz);
		dump_hist(hist);
	}
}

void ghist_eq(unsigned char *out, const unsigned char *in, const int w, const int h)
{
	unsigned hist[256];
	int iSiz = w*h;

	calc_hist(in, hist, iSiz);

	for (int i = 1; i < 256; ++i) {
		hist[i] += hist[i-1];
	}

	for (int i = 0; i < 255; ++i) {
		hist[i] *= 256;
		hist[i] /= hist[255];
	}
	hist[255] = 255;

	transfunc_mapping(out, in, hist, iSiz);

	{
		unsigned hist[256];
		calc_hist(out, hist, iSiz);
		dump_hist(hist);
	}
}

void lhist_eq(unsigned char *out, const unsigned char *in, const int w, const int h, const int win)
{
	int ww = 2*win+1;
	ww = ww*ww;
	for (int i = 0; i < h; ++i) {
		for (int j = 0; j < w; ++j) {
			int cnt = 0;
			for (int k = i-win; k <= i+win; ++k) {
				for (int l = j-win; l <= j+win; ++l) {
					int K = clamp(k, 0, h-1);
					int L = clamp(l, 0, w-1);
					if (in[w*i+j] > in[w*K+L]) {
						++cnt;
					}
				}
			}
			out[w*i+j] = cnt*255/ww;
		}
	}

	{
		unsigned hist[256];
		calc_hist(out, hist, w*h);
		dump_hist(hist);
	}
}

void power_law(unsigned char *out, const unsigned char *in, const int w, const int h)
{
	unsigned trFunc[256];
	int iSiz = w*h;

	for (int i = 0; i < 256; ++i) {
		unsigned y = powf(i/256.0f, 0.35f) * 256.0f;
		trFunc[i] = y;
	}

	transfunc_mapping(out, in, trFunc, iSiz);

	{
		unsigned hist[256];
		calc_hist(out, hist, w*h);
		dump_hist(hist);
	}
}

void exp_law(unsigned char *out, const unsigned char *in, const int w, const int h)
{
	unsigned trFunc[256];
	int iSiz = w*h;

	for (int i = 0; i < 256; ++i) {
		static const float mulFactor1 = 1.0f/25600.0f;
		static const float mulFactor2 = 255.0f / (exp2f(255*mulFactor1) - 1);
		unsigned y = (exp2f(i*mulFactor1) - 1.0f) * mulFactor2;
		trFunc[i] = y;
	}

	transfunc_mapping(out, in, trFunc, iSiz);

	{
		unsigned hist[256];
		calc_hist(out, hist, w*h);
		dump_hist(hist);
	}
}

void log_law(unsigned char *out, const unsigned char *in, const int w, const int h)
{
	unsigned trFunc[256];
	int iSiz = w*h;

	for (int i = 0; i < 256; ++i) {
		static const float offset = 10.0f;
		static const float mulFactor = 255.0f / log2f((255+offset)/offset);
		static const float subFactor = log2f(offset);
		unsigned y = (log2f(i+offset) - subFactor) * mulFactor;
		trFunc[i] = y;
	}

	transfunc_mapping(out, in, trFunc, iSiz);

	{
		unsigned hist[256];
		calc_hist(out, hist, w*h);
		dump_hist(hist);
	}
}

void otsu_threshold(unsigned char *out, const unsigned char *in, const int w, const int h)
{
	unsigned hist[256];
	unsigned xhist[256];
	int iSiz = w*h;

	calc_hist(in, hist, iSiz);

	unsigned sumNX = 0, sumN = 0;
	for (int i = 0; i < 256; ++i) {
		xhist[i] = hist[i] * i;
		sumN += hist[i];
		sumNX += xhist[i];
	}

	unsigned char th;
	for (unsigned char i = 1; i < 255; ++i) {
		static float maxG = -1.0f;
		static unsigned sumN0 = hist[0];
		static unsigned sumNX0 = xhist[0];
		unsigned sumN1 = sumN - sumN0;
		unsigned sumNX1 = sumNX - sumNX0;
		if (sumN0 == 0 || sumN1 == 0) {
			break;
		}
		float du = (float)sumNX0/sumN0 - (float)sumNX1/sumN1;
		float g = (sumN0 * sumN1) * du * du;
		if (g > maxG) {
			th = i;
			maxG = g;
		}
		sumN0 += hist[i];
		sumNX0 += xhist[i];
	}

	for (int i = 0; i < iSiz; ++i) {
		*out = (*in < th) ? 0: 255;
		++in;
		++out;
	}
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
	unsigned char *imgD = new unsigned char[w*h];
	unsigned char *imgH = new unsigned char[w*h];
	unsigned char *imgL = new unsigned char[w*h];
	unsigned char *imgP1 = new unsigned char[w*h];
	unsigned char *imgP2 = new unsigned char[w*h];
	unsigned char *imgP3 = new unsigned char[w*h];
	unsigned char *imgO = new unsigned char[w*h];

	// read file
	FILE *fpIn = fopen(argv[1], "rb");
	fread(imgI, w*h, 1, fpIn);
	fclose(fpIn);

	{
		unsigned hist[256];
		calc_hist(imgI, hist, w*h);
		dump_hist(hist);
	}

	// divided by 2
	divided_by2(imgD, imgI, w, h);

	// global histogram
	ghist_eq(imgH, imgD, w, h);

	// local histogram
	lhist_eq(imgL, imgD, w, h, 7);

	// power law etc
	power_law(imgP1, imgD, w, h);
	exp_law  (imgP2, imgD, w, h);
	log_law  (imgP3, imgD, w, h);

	// Otsu
	otsu_threshold(imgO, imgI, w, h);


	FILE *fpPgmI = fopen("I.pgm", "wb");
	FILE *fpPgmD = fopen("D.pgm", "wb");
	FILE *fpPgmH = fopen("H.pgm", "wb");
	FILE *fpPgmL = fopen("L.pgm", "wb");
	FILE *fpPgmO = fopen("O.pgm", "wb");
	FILE *fpPgmP1 = fopen("P1.pgm", "wb");
	FILE *fpPgmP2 = fopen("P2.pgm", "wb");
	FILE *fpPgmP3 = fopen("P3.pgm", "wb");
	save_P5_pgm(fpPgmI, w, h, imgI);
	save_P5_pgm(fpPgmD, w, h, imgD);
	save_P5_pgm(fpPgmH, w, h, imgH);
	save_P5_pgm(fpPgmL, w, h, imgL);
	save_P5_pgm(fpPgmP1, w, h, imgP1);
	save_P5_pgm(fpPgmP2, w, h, imgP2);
	save_P5_pgm(fpPgmP3, w, h, imgP3);
	save_P5_pgm(fpPgmO, w, h, imgO);
	fclose(fpPgmI);
	fclose(fpPgmD);
	fclose(fpPgmH);
	fclose(fpPgmL);
	fclose(fpPgmO);
	fclose(fpPgmP1);
	fclose(fpPgmP2);
	fclose(fpPgmP3);
	delete[] imgI, imgD, imgH, imgL, imgP1, imgP2, imgP3, imgO;
	return 0;
}
