#include "texture.h"
#include <cstdlib>
#include <cstdio>
#include <cmath>
#include <algorithm>
#include <cstring>
#include <cassert>
#include <set>
#include <ctime>
using namespace std;

inline unsigned char clip(int x)
{
	return x&(~0xff) ? ((-x)>>31)&0xff : x;
}

inline int clamp(int x, const int l, const int h)
{
	return max(min(x, h), l);
}

template <int v1, int v2, int v3, int v4, int v5, int v6, int norm>
void filter33_2pass(
	const unsigned char *in, unsigned char *out, const int w, const int h
)
{
	for (int i = 0; i < h; ++i) {
		const int i1 = (i == h-1)? i: i+1;
		const int i2 = (i == 0  )? i: i-1;
		for (int j = 0; j < w; ++j) {
			const int j1 = (j == h-1)? j: j+1;
			const int j2 = (j == 0  )? j: j-1;
			int _s1 = v1*in[i1*w+j1] + v2*in[i1*w+j] + v3*in[i1*w+j2];
			int _s2 = v1*in[i *w+j1] + v2*in[i *w+j] + v3*in[i *w+j2];
			int _s3 = v1*in[i2*w+j1] + v2*in[i2*w+j] + v3*in[i2*w+j2];
			int sum = v4*_s1 + v5*_s2 + v6*_s3;
			out[w*i+j] = clip(abs((sum+norm/2) / norm));
		}
	}
}

static void LPF(const unsigned char *in, unsigned char *out, const int w, const int h, const int wSize)
{
	// box filter using integral image
	unsigned *iimg = new unsigned [(w+1)*(h+1)];
	memset(iimg, 0, sizeof(unsigned)*(w+1));

	// build integral image
	for (int i = 0; i < h; ++i) {
		static const unsigned char *imgTmp = in;
		unsigned *cur = iimg + 1 + (w+1)*(i+1);
		unsigned rowSum = 0;
		cur[-1] = 0;
		for (int j = 0; j < w; ++j) {
			rowSum += *imgTmp;
			cur[0] = rowSum + cur[-w-1];
			++imgTmp;
			++cur;
		}
	}

	// box filter
	for (int i = 0; i < h; ++i) {
		for (int j = 0; j < w; ++j) {
			int xh = j+1+wSize;
			int xl = j-wSize;
			int yh = i+1+wSize;
			int yl = i-wSize;
			xh = min( w, xh);
			xl = max( 0, xl);
			yh = min( h, yh);
			yl = max( 0, yl);
			unsigned sum = iimg[yh*(w+1)+xh] - iimg[yh*(w+1)+xl] + iimg[yl*(w+1)+xl] - iimg[yl*(w+1)+xh];
			int wArea = (xh-xl) * (yh-yl);
			int filterValue = (sum+wArea/2) / wArea;
			assert(filterValue >= 0 && filterValue < 256);
			out[w*i+j] = filterValue;
		}
	}

	delete[] iimg;
}

float euc_dist2(const float *a, const float *b, const int n)
{
	float s = 0.0f;
	for (unsigned i = 0; i < n; ++i) {
		float d = a[i] - b[i];
		s += d*d;
	}
	return s;
}

void Feature::dump()
{
	for (unsigned i = 0; i < 9; ++i) {
		printf("%5.1f  ", v[i]);
	}
	putchar('\n');
}

static unsigned sum_all(const unsigned char *v, const int l)
{
	unsigned sum = 0;
	for (int i = 0; i < l; ++i) {
		sum += *v;
		++v;
	}
	return sum;
}

static void make_law_images(unsigned char *buf, const unsigned char *img, const int w, const int h)
{
	int iSiz = w*h;
	filter33_2pass< 1, 2,  1,  1, 2, 1, 36>(img, buf, w, h);   buf += iSiz;
	filter33_2pass< 1, 0, -1,  1, 2, 1, 12>(img, buf, w, h);   buf += iSiz;
	filter33_2pass<-1, 2, -1,  1, 2, 1, 12>(img, buf, w, h);   buf += iSiz;
	filter33_2pass< 1, 2,  1,  1, 0, -1, 12>(img, buf, w, h);  buf += iSiz;
	filter33_2pass< 1, 0, -1,  1, 0, -1,  4>(img, buf, w, h);  buf += iSiz;
	filter33_2pass<-1, 2, -1,  1, 0, -1,  4>(img, buf, w, h);  buf += iSiz;
	filter33_2pass< 1, 2,  1, -1, 2, -1, 12>(img, buf, w, h);  buf += iSiz;
	filter33_2pass< 1, 0, -1, -1, 2, -1,  4>(img, buf, w, h);  buf += iSiz;
	filter33_2pass<-1, 2, -1, -1, 2, -1,  4>(img, buf, w, h);
}

void Feature::from_image(const unsigned char *img, const int w, const int h)
{
	const int iSiz = w*h;
	const float iSiz_i = 1.0f / iSiz;
	unsigned char *buf = new unsigned char[iSiz*9];
	make_law_images(buf, img, w, h);

	v[0] = sum_all(buf, iSiz) * iSiz_i;   buf += iSiz;
	v[1] = sum_all(buf, iSiz) * iSiz_i;   buf += iSiz;
	v[2] = sum_all(buf, iSiz) * iSiz_i;   buf += iSiz;
	v[3] = sum_all(buf, iSiz) * iSiz_i;   buf += iSiz;
	v[4] = sum_all(buf, iSiz) * iSiz_i;   buf += iSiz;
	v[5] = sum_all(buf, iSiz) * iSiz_i;   buf += iSiz;
	v[6] = sum_all(buf, iSiz) * iSiz_i;   buf += iSiz;
	v[7] = sum_all(buf, iSiz) * iSiz_i;   buf += iSiz;
	v[8] = sum_all(buf, iSiz) * iSiz_i;

	buf -= 8 * iSiz;
	delete[] buf;
}

void Feature::from_pixels(Feature *feats, const unsigned char *img, const int w, const int h)
{
	const int iSiz = w*h;
	unsigned char *buf = new unsigned char[iSiz*9];
	unsigned char *tmp = new unsigned char[iSiz];
	make_law_images(buf, img, w, h);

	for (int i = 0; i < 9; ++i) {
		unsigned char *curImg = buf+i*iSiz;
		LPF(curImg, tmp, w, h, 10);
		memcpy(curImg, tmp, sizeof(unsigned char)*iSiz);
	}

	for (int i = 0; i < iSiz; ++i) {
		for (int j = 0; j < 9; ++j) {
			feats[i].v[j] = buf[i+iSiz*j];
		}
	}

	delete[] buf, tmp;
}

struct KM_Center {
	float v[9];
	int nFeat;
};

static float km_update(Feature *feats, const KM_Center *old_centers, KM_Center *new_centers, const int n, const int nCls)
{
	for (int i = 0; i < n; ++i) {
		Feature &f = feats[i];
		float minD = euc_dist2(f.v, old_centers[0].v, 9);
		int minJ = 0;

		for (int j = 1; j < nCls; ++j) {
			float d = euc_dist2(f.v, old_centers[j].v, 9);
			if (d < minD) {
				minD = d;
				minJ = j;
			}
		}
		f.cls = minJ;
	}

	// initialize new centers
	for (int i = 0; i < nCls; ++i) {
		memset(new_centers[i].v, 0, sizeof(float)*9);
		new_centers[i].nFeat = 0;
	}

	// accumulate
	for (int i = 0; i < n; ++i) {
		Feature &f = feats[i];
		KM_Center &curCent = new_centers[f.cls];

		++curCent.nFeat;
		for (int j = 0; j < 9; ++j) {
			curCent.v[j] += f.v[j];
		}
	}

	float delta = 0.0f;
	// calculate the final value
	for (int i = 0; i < nCls; ++i) {
		for (int j = 0; j < 9; ++j) {
			new_centers[i].v[j] /= new_centers[i].nFeat;
		}
		delta += sqrtf(euc_dist2(new_centers[i].v, old_centers[i].v, 9));
	}

	return delta;
}

int Feature::k_means(Feature *feats, const int n, const int nCls, const int maxIter, const float th)
{
	int nIter = 0;
	KM_Center *old_centers = new KM_Center[nCls];
	KM_Center *new_centers = new KM_Center[nCls];


	// random choose n number, we assert n >> nCls
	{

	srand(time(0));
	set<int> s;
	while (s.size() != nCls) {
		s.insert(rand() % n);
	}

	set<int>::iterator i = s.begin();
	for (int c = 0; c < nCls; ++c) {
		memcpy(old_centers[c].v, feats[*i].v, 9*sizeof(float));
		++i;
	}
	
	}

	float delta;
	do {
		delta = km_update(feats, old_centers, new_centers, n, nCls);
		swap(old_centers, new_centers);
		++nIter;
	} while (delta > th && nIter < maxIter);

	delete[] old_centers, new_centers;

	return nIter;
}
