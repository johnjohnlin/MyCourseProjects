#include <algorithm>
#include <cstdio>
#include <cassert>
#include "bbox.h"

inline int clamp(int x, const int l, const int h)
{
	return std::max(std::min(x, h), l);
}

static std::vector<Range1D> detect_cont_routine(bool *salFlags, const int l)
{
	std::vector<Range1D> ret;
	bool prev = salFlags[0];
	int start;
	for (int i = 0; i <= l; ++i) {
		bool cur = salFlags[i+1];
		if (prev ^ cur) {
			if (prev) {
				Range1D r = {start, i};
				ret.push_back(r);
			} else {
				start = i;
			}
		}
		prev = cur;
	}
	delete[] salFlags;

	return ret;
}

std::vector<Range1D> divide_v(const unsigned char *img, const int w, const int h, const int ws, const int th)
{
	bool *salFlags = new bool[h+2];
	std::fill(salFlags, salFlags+h+2, false);

	for (int i = 0; i < h; ++i) {
		const unsigned char *imgTmp = img;
		for (int j = 0; j < w; ++j) {
			if (*imgTmp < th) {
				salFlags[i+1] = true;
				break;
			}
			++imgTmp;
		}
		img += ws;
	}

	return detect_cont_routine(salFlags, h);
}

std::vector<Range1D> divide_h(const unsigned char *img, const int w, const int h, const int ws, const int th)
{
	SubImageIndex ret;
	bool *salFlags = new bool[w+2];
	std::fill(salFlags, salFlags+w+2, false);

	for (int i = 0; i < h; ++i) {
		for (int j = 0; j < w; ++j) {
			if (*img < th) {
				salFlags[j+1] = true;
			}
			++img;
		}
	}

	return detect_cont_routine(salFlags, w);
}

void sample_subimg(unsigned char *out, const SubImageIndex &s, const unsigned char *img, const int w, const int h)
{
	float x = s.ox;
	float y = s.oy;

	for (int i = 0; i < s.h; ++i) {
		float xtmp = x;
		for (int j = 0; j < s.w; ++j) {
			int I = y;
			int J = xtmp;
			I = clamp(I, 0, h-1);
			J = clamp(J, 0, w-1);
			*out = img[w*I+J];
			++out;
			xtmp += s.dx;
		}
		y += s.dy;
	}
}
