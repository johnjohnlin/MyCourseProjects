#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <cassert>
#include "hw1_util.h"

inline int clamp(int x, const int l, const int h)
{
	return std::max(std::min(x, h), l);
}

inline unsigned char clip(int x)
{
	return x&(~0xff) ? ((-x)>>31)&0xff : x;
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

static void calc_hist(const unsigned char *in, unsigned *const hist, const int siz)
{
	memset(hist, 0, 256*sizeof(unsigned));
	for (int i = 0; i < siz; ++i) {
		++(hist[*in]);
		++in;
	}
}

static void transfunc_mapping(unsigned char *out, const unsigned char *in, unsigned *const trFunc, const int siz)
{
	for (int i = 0; i < siz; ++i) {
		*out = trFunc[*in];
		++in;
		++out;
	}
}

void ghist_eq(unsigned char *out, const unsigned char *in, const int w, const int h)
{
	unsigned hist[256];
	int iSiz = w*h;

	calc_hist(in, hist, iSiz);

	hist[0] = 0;
	for (int i = 1; i < 256; ++i) {
		hist[i] += hist[i-1];
	}

	for (int i = 0; i < 255; ++i) {
		hist[i] *= 256;
		hist[i] /= hist[255];
	}
	hist[255] = 255;

	transfunc_mapping(out, in, hist, iSiz);
}

void downsample2(unsigned char *out, const unsigned char *in, int w, int h)
{
	int dst_w = w / 2;
	int dst_h = h / 2;
	for (int i = 0; i < dst_h; ++i) {
		for (int j = 0; j < dst_w; ++j) {
			int ind = (i * w + j) * 2;
			out[i * dst_w + j] = (in[ind] + in[ind+1] + in[ind+w] + in[ind+w+1] + 2) / 4;
		}
	}
	return;
}


void detect_spike(unsigned char *out, const unsigned char *in, const int w, const int h)
{
	memset(out, 0, w*h*sizeof(unsigned char));
	for (int i = 1; i < h-1; ++i) {
		for (int j = 1; j < w-1; ++j) {
			const unsigned char *cur = in + w*i + j;
			// vertical & horizontal
			int a, b, c, d, e, f, A1, A2, B1, B2;
			a = (int)cur[ w-1] + cur[ w] + cur[ w+1];
			b = (int)cur[  -1] + cur[ 0] + cur[   1];
			c = (int)cur[-w-1] + cur[-w] + cur[-w+1];
			d = (int)cur[ 1-w] + cur[ 1] + cur[ 1+w];
			e = (int)cur[  -w] + cur[ 0] + cur[   w];
			f = (int)cur[-1-w] + cur[-1] + cur[-1+w];
			A1 = std::abs(a+c-2*b+1)/2;
			A2 = std::abs(d+f-2*e+1)/2;
			B1 = std::abs(c-a)/2;
			B2 = std::abs(f-d)/2;
			if (A1 > 30 && A1 > A2*3 && A1 > B1|| A2 > 30 && A2 > A1*3 && A2 > B2) {
				out[i*w+j] = 255;
				continue;
			}/*
			// diagonal
			if () {
				out[i*w+j] = 255;
				continue;
			}*/
		}
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
	unsigned sumN0 = hist[0];
	unsigned sumNX0 = xhist[0];
	for (unsigned char i = 1; i < 255; ++i) {
		static float maxG = -1.0f;
		unsigned sumN1 = sumN - sumN0;
		unsigned sumNX1 = sumNX - sumNX0;
		if (sumN0 != 0 && sumN1 != 0) {
			float du = (float)sumNX0/sumN0 - (float)sumNX1/sumN1;
			float g = (sumN0 * sumN1) * du * du;
			if (g > maxG) {
				th = i;
				maxG = g;
			}
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
