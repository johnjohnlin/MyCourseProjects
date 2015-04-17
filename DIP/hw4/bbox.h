#ifndef __BBOX_H__
#define __BBOX_H__

#include <vector>

struct Range1D {
	int op, ed;
};
struct SubImageIndex {
	SubImageIndex() {}
	SubImageIndex(const Range1D H, const Range1D V)
	{
		w = H.ed - H.op;
		h = V.ed - V.op;
		ox = H.op;
		oy = V.op;
		dx = 1.0;
		dy = 1.0;
	}
	float ox, oy, dx, dy;
	int w, h;
};

std::vector<Range1D> divide_v(const unsigned char *img, const int w, const int h, const int ws, const int th);
std::vector<Range1D> divide_h(const unsigned char *img, const int w, const int h, const int ws, const int th);
void sample_subimg(unsigned char *out, const SubImageIndex &s, const unsigned char *img, const int w, const int h);

#endif /* end of include guard */
