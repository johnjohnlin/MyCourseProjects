#include "edge.h"
#include <algorithm>
#include <cstring>
#include <vector>
#include <cmath>
using namespace std;

/*  x is current pixel
     123
     0x0
     321
 */

enum CannyDir {
	CD_0 = 0,
	CD_1,
	CD_2,
	CD_3
};

enum CannyType {
	CT_EDGE = 255,
	CT_CANDICATE = 128,
	CT_FLAT = 0
};

enum ZeroCrossSign {
	ZC_N = 0,
	ZC_Z,
	ZC_P
};

struct XY {int x, y;};

static void thresholding(int th, int iSiz, unsigned char *edgeStrength, unsigned char *e)
{
	for (int i = 0; i < iSiz; ++i) {
		e[i] = edgeStrength[i] > th? 255: 0;
	}
}

template<typename T>
inline T clamp(T in, const T l, const T h)
{
	return max(min(in, h), l);
}

template<typename T>
inline const T clamp_255(const T v)
{
	return (v&(~0xff))? ((~v)>>(sizeof(T)*8-1)): v;
}

static void blur(const int w, const int h, const unsigned char *img, unsigned char *imgBlur)
{
	for (int i = 0; i < w; ++i) {
		for (int j = 0; j < h; ++j) {
			static const unsigned char filterCoeff[5][5] = {
				{2,  4,  5,  4, 2},
				{4,  9, 12,  9, 4},
				{5, 12, 15, 12, 5},
				{4,  9, 12,  9, 4},
				{2,  4,  5,  4, 2},
			};
			unsigned short sum = 0;
			for (int ii = 0; ii < 5; ++ii) {
				int y = clamp<int>(i-2+ii, 0, h-1);
				for (int jj = 0; jj < 5; ++jj) {
					int x = clamp<int>(j-2+jj, 0, w-1);
					sum += img[y*w+x] * filterCoeff[ii][jj];
				}
			}
			imgBlur[i*w+j] = (sum+79)/159;
		}
	}
}

static void build_gradient_map(const int w, const int h, const unsigned char *blurred, unsigned char *map)
{
	memset(map, 0, sizeof(unsigned char)*w*2);
	map += w*2;
	blurred += w;

	for (int i = 1; i < (h - 1); ++i) {

		map[0] = map[1] = 0;
		map += 2;
		++blurred;

		for (int j = 1; j < (w - 1); ++j) {
			int dx = (int)blurred[1] - blurred[-1];
			int dy = (int)blurred[w] - blurred[-w];
			int dxAbs = abs(dx);
			int dyAbs = abs(dy);
			++blurred;
			CannyDir theta;
			if (2*dxAbs>5*dyAbs)
				theta = CD_0;
			else if (2*dyAbs>5*dxAbs)
				theta = CD_1;
			else if ((dx^dy)>>(sizeof(int)*8-1) == 0) // same sign
				theta = CD_2;
			else
				theta = CD_3;

			*map = 0.5f * sqrtf(dx * dx + dy * dy);
			++map;
			*map = theta;
			++map;
		}

		map[0] = map[1] = 0;
		map += 2;
		++blurred;

	}

	memset(map, 0, sizeof(unsigned char)*w*2);
	// map += w*2;
	// blurred += w;
}

static void detect_edge(
	const int w, const int h,
	const unsigned char *map, unsigned char *e,
	const int thL, const int thH
)
{
	memset(e, CT_FLAT, sizeof(unsigned char)*w*h); // ok because CT_FLAT = 0

	e += w;
	map += 2*w;

	const int offsets[4] = {1, w+1, w, w-1};

	for (int i = 1; i < (h - 1); ++i) {

		map += 2;
		++e;

		for (int j = 1; j < (w - 1); ++j) {
			int d = offsets[map[1]] * 2;
			*e = CT_FLAT;
			if (map[0] > map[d] && map[0] > map[-d]) {
				if (map[0] > thH)
					*e = CT_EDGE;
				else if (map[0] > thL)
					*e = CT_CANDICATE;
			}
			++e;
			map += 2;
		}

		map += 2;
		++e;

	}

	// e += w;
	// map += 2*w;
}

static inline void push_connect_and_color(vector<XY> &doStack, unsigned char *e, const XY xy, const int w)
{
	if (e[w*xy.x+xy.y] == CT_CANDICATE) {
		e[w*xy.x+xy.y] = CT_EDGE;
		doStack.push_back(xy);
	}
}

static inline void flood_fill_pixel(const int w, unsigned char *e, const int x, const int y)
{
	XY xyCur = {x, y};
	vector<XY> doStack;
	doStack.push_back(xyCur);

	do {
		XY xy = doStack.back();
		doStack.pop_back();

		// 8-connective
		xy.x += 1;
		push_connect_and_color(doStack, e, xy, w);
		xy.y += 1;
		push_connect_and_color(doStack, e, xy, w);
		xy.x -= 1;
		push_connect_and_color(doStack, e, xy, w);
		xy.x -= 1;
		push_connect_and_color(doStack, e, xy, w);
		xy.y -= 1;
		push_connect_and_color(doStack, e, xy, w);
		xy.y -= 1;
		push_connect_and_color(doStack, e, xy, w);
		xy.x += 1;
		push_connect_and_color(doStack, e, xy, w);
		xy.x += 1;
		push_connect_and_color(doStack, e, xy, w);
	} while (!doStack.empty());
}

static void flood_fill(const int w, const int h, unsigned char *e)
{
	for (int i = 1; i < w-1; ++i) {
		for (int j = 1; j < h-1; ++j) {
			if (e[w*i+j] == CT_EDGE) {
				flood_fill_pixel(w, e, i, j);
			}
		}
	}

	for (int i = 1; i < w-1; ++i) {
		for (int j = 1; j < h-1; ++j) {
			if (e[w*i+j] == CT_CANDICATE) {
				e[w*i+j] = CT_FLAT;
			}
		}
	}
}

void canny(const int w, const int h, const unsigned char *img, unsigned char *e, const int thL, const int thH)
{
	unsigned char *imgBlur = new unsigned char [w*h];
	unsigned char *imgGrad = new unsigned char [2*w*h];
	blur(w, h, img, imgBlur);
	build_gradient_map(w, h, imgBlur, imgGrad);
	detect_edge(w, h, imgGrad, e, thL, thH);
#define FLOODFILL
#ifdef FLOODFILL
	flood_fill(w, h, e);
#endif
	delete[] imgBlur, imgGrad;
}

void sobel(const int w, const int h, const unsigned char *img, unsigned char *e, const int th)
{
	const int thSq4 = 4*th*th;
	memset(e, CT_FLAT, sizeof(unsigned char)*w*h); // ok because CT_FLAT = 0

	e += w;
	img += w;

	for (int i = 1; i < (h - 1); ++i) {
		++e;
		++img;
		for (int j = 1; j < (w - 1); ++j) {
			int dx = (int)img[1] - img[-1];
			int dy = (int)img[w] - img[-w];
			if (dx*dx+dy*dy > thSq4) {
				*e = CT_EDGE;
			}
			++e;
			++img;
		}
		++e;
		++img;
	}

	// e += w;
	// img += w;
}

static void laplacian(
	const int w, const int h,
	const unsigned char *img, char *lap
)
{
	lap += w;
	img += w;

	for (int i = 1; i < (h - 1); ++i) {
		*lap = 0;
		++lap;
		++img;

		for (int j = 1; j < (w - 1); ++j) {
			int v = (*img)*8;
			v = v-img[-1]-img[1]-img[w-1]-img[w]-img[w+1]-img[-w-1]-img[-w]-img[-w+1];
			v += 128;
			v = clamp_255<int>(v);
			v -= 128;
			*lap = v;
			++lap;
			++img;
		}

		*lap = 0;
		++lap;
		++img;
	}

	// lap += w;
	// img += w;
}

void lap2edge(const int w, const int h, char *lap, unsigned char *e, const int th)
{
	for (int i = 0; i < w*h; ++i) {
// #define SIGNONLY
#ifdef SIGNONLY
		*e = 128+*lap;
		++e;
#endif
		if (*lap < -th) {
			*lap = ZC_N;
		} else if (*lap > th) {
			*lap = ZC_P;
		} else {
			*lap = ZC_Z;
		}
		++lap;
	}
	lap -= w*h;

#ifdef SIGNONLY
	e -= w*h;
	return ;
#endif
	memset(e, CT_FLAT, sizeof(unsigned char)*w*h); // ok because CT_FLAT = 0
	lap += w;
	e += w;

	for (int i = 1; i < (h - 1); ++i) {
		++lap;
		++e;

		for (int j = 1; j < (w - 1); ++j) {
			int nP = 0, nN = 0;
			unsigned char x;
#define AddIf if (x == ZC_P) { ++nP; } else if (x == ZC_N) { ++nN; }
			x = lap[-w-1];
			AddIf;
			x = lap[-w];
			AddIf;
			x = lap[-w+1];
			AddIf;
			x = lap[-1];
			AddIf;
			x = lap[1];
			AddIf;
			x = lap[w-1];
			AddIf;
			x = lap[w];
			AddIf;
			x = lap[w+1];
			AddIf;
			if (nP >= 2 && nN >= 2 && lap[0] == ZC_Z)
				*e = CT_EDGE;
#undef AddIf
			++lap;
			++e;
		}

		++lap;
		++e;
	}

	// lap += w;
	// e += w;
}

void zero_cross(const int w, const int h, const unsigned char *img, unsigned char *e, const int th)
{
	char *lap = new char [w*h];
	unsigned char *imgBlur = new unsigned char [w*h];
	blur(w, h, img, imgBlur);
	laplacian(w, h, imgBlur, lap);
	lap2edge(w, h, lap, e, th);
	delete lap, imgBlur;
}
