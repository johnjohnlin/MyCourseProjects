#include "morph.h"
#include <cstring>
#include <algorithm>
#include <vector>
#include <cstdio>
using namespace std;

void morph_edge(const unsigned char *img, unsigned char *boundary, const int w, const int h)
{
	memset(boundary, 0, sizeof(unsigned char)*w*h);

	for (int i = 0; i < h; ++i) {
		for (int j = 0; j < w; ++j) {
			if (img[w*i+j] > 128) {
				goto morph_edge_loopEnd;
			}
			for (int ii = max(0, i-1); ii <= min(i+1, h); ++ii) {
				for (int jj = max(0, j-1); jj <= min(j+1, w); ++jj) {
					if (img[w*ii+jj] > 128) {
						*boundary = 255;
						goto morph_edge_loopEnd;
					}
				}
			}
morph_edge_loopEnd:
			++boundary;
		}
	}
}

struct XY {int x, y;};

static inline void push_connect_and_color(
	vector<XY> &doStack, const unsigned char *img, unsigned char *lb,
	const XY xy, const int w, const int h, const unsigned char GL
)
{
	if (xy.x >= 0 && xy.x < h && xy.y >= 0 && xy.y < w) {
		int ind = w*xy.x+xy.y;
		if (lb[ind] == 0 && img[ind] > 128) {
			lb[ind] = GL;
			doStack.push_back(xy);
		}
	}
}

static inline void flood_fill(
	const int w, const int h, const unsigned char *img,
	unsigned char *e, const int x, const int y, const unsigned char GL)
{
	XY xyCur = {x, y};
	vector<XY> doStack;
	doStack.push_back(xyCur);

	do {
		XY xy = doStack.back();
		doStack.pop_back();

		// 8-connective
		xy.x += 1;
		push_connect_and_color(doStack, img, e, xy, w, h, GL);
		xy.y += 1;
		push_connect_and_color(doStack, img, e, xy, w, h, GL);
		xy.x -= 1;
		push_connect_and_color(doStack, img, e, xy, w, h, GL);
		xy.x -= 1;
		push_connect_and_color(doStack, img, e, xy, w, h, GL);
		xy.y -= 1;
		push_connect_and_color(doStack, img, e, xy, w, h, GL);
		xy.y -= 1;
		push_connect_and_color(doStack, img, e, xy, w, h, GL);
		xy.x += 1;
		push_connect_and_color(doStack, img, e, xy, w, h, GL);
		xy.x += 1;
		push_connect_and_color(doStack, img, e, xy, w, h, GL);
	} while (!doStack.empty());
}

int label_image(const unsigned char *img, unsigned char *lb, const int w, const int h)
{
	unsigned char GL = 0;
	const unsigned char *lb_tmp = lb;
	const unsigned char *img_tmp = img;

	memset(lb, 0, sizeof(unsigned char)*w*h);
	for (int i = 0; i < h; ++i) {
		for (int j = 0; j < w; ++j) {
			// Object and not traversed
			if (*img_tmp > 128 && !(*lb_tmp) ) {
				++GL;
				flood_fill(w, h, img, lb, i, j, GL);
			}
			++lb_tmp;
			++img_tmp;
		}
	}

	unsigned char step = 255/GL;
	for (int i = 0; i < w*h; ++i) {
		*lb *= step;
		++lb;
	}

	return GL;
}

int erosion_object(const unsigned char *img, const int w, const int h, const int n)
{
	const int iSiz = w*h;
	unsigned char *buf1 = new unsigned char[w*h];
	unsigned char *buf2 = new unsigned char[w*h];
	memcpy(buf1, img, sizeof(unsigned char)*iSiz);
	memset(buf2, 0xff, sizeof(unsigned char)*iSiz);

	for (int nIter = 0; nIter < n; ++nIter) {
		for (int i = 0; i < h; ++i) {
			for (int j = 0; j < w; ++j) {
				for (int ii = max(0, i-1); ii <= min(i+1, h); ++ii) {
					for (int jj = max(0, j-1); jj <= min(j+1, w); ++jj) {
						if (buf1[w*ii+jj] < 128) {
							*buf2 = 0;
							goto erosion_object_loopEnd;
						}
					}
				}
erosion_object_loopEnd:
				++buf2;
			}
		}
		buf2 -= iSiz;
		swap(buf1, buf2);
	}

	int r = label_image(buf2, buf1, w, h);
	delete[] buf1, buf2;
	return r;
}
