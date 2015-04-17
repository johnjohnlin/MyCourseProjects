#include "ME.hpp"
#include <climits>
#include <cstdio>
#include <algorithm>
#include <cassert>

unsigned long long MEClass::diff(XY a1, XY a2, const unsigned long long th)
{
	unsigned long long ssd = 0;
	for (int i = 0; i < blkSiz; i++) {
		for (int j = 0; j < blkSiz; j++) {
			int d = (int)i1[w*(a1.y+i) + a1.x+j] - i2[w*(a2.y+i) + a2.x+j];
			ssd += d*d;
			if (ssd >= th) {
				return ULLONG_MAX;
			}
		}
	}
	return ssd;
}

void MEClass::motion_estimation(std::vector<XY> &mvs)
{
	int nBlkX = w/blkSiz, nBlkY = h/blkSiz;
	mvs.resize(nBlkX * nBlkY);
	std::vector<XY>::iterator mvsIter = mvs.begin();

	for (int i = 0; i < nBlkY; i++) {
		for (int j = 0; j < nBlkX; j++) {
			XY maxMV;
			unsigned long long curMinDiff = ULLONG_MAX;

			XY a1 = {j*blkSiz, i*blkSiz};
			int syMin = std::max(       0, a1.y-searchRange);
			int syMax = std::min(h-blkSiz, a1.y+searchRange);
			int sxMin = std::max(       0, a1.x-searchRange);
			int sxMax = std::min(w-blkSiz, a1.x+searchRange);
			for (int k = syMin; k < syMax; k++) {
				for (int l = sxMin; l < sxMax; l++) {
					XY a2 = {l, k};
					unsigned long long blkDiff = diff(a1, a2, curMinDiff);
					if (blkDiff < curMinDiff) {
						maxMV = a2-a1;
						curMinDiff = blkDiff;
					}
				}
			}
			*mvsIter = maxMV;
			++mvsIter;
		}
	}

	assert(mvsIter == mvs.end());
}
