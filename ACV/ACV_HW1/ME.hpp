#ifndef  __ME_H__
#define __ME_H__

#include <vector>

struct XY {
	int x, y;
	const XY operator-(const XY &r) const
	{
		XY ret = *this;
		ret.x -= r.x;
		ret.y -= r.y;
		return ret;
	}
};

struct MEClass {
	void motion_estimation(std::vector<XY> &mvs);
	unsigned char *i1, *i2;
	int w, h, blkSiz, searchRange;
private:
	unsigned long long diff(XY a1, XY a2, const unsigned long long curMin);
};

#endif
