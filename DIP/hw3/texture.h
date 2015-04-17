#ifndef __TEXTURE_H__
#define __TEXTURE_H__
#include <cmath>
#include <algorithm>


struct Feature {
	float v[9];
	int cls;
	static int k_means(Feature *feats, const int n, const int nCls, const int maxIter, const float th);
	static void from_pixels(Feature *feats, const unsigned char *img, const int w, const int h);
	void from_image(const unsigned char *img, const int w, const int h);
	void dump();
	inline float norm2()
	{
		float sum = 0.0f;
		for (int i = 0; i < 9; ++i) {
			sum += v[i] * v[i];
		}
		return sum;
	}
	inline void normalize()
	{
		float n = 1.0f / sqrtf(norm2());
		for (int i = 0; i < 9; ++i) {
			v[i] *= n;
		}
	}
	inline void sift_normalize()
	{
		normalize();
		for (int i = 0; i < 9; ++i) {
			v[i] = std::min(0.4f, v[i]);
		}
		normalize();
	}
};

float euc_dist2(const float *a, const float *b, const int n);

#endif /* end of include guard */
