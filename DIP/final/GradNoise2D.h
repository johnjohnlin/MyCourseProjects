#ifndef __PERLIN2D_H__
#define __PERLIN2D_H__

class GradNoise2D {
public:
	int seed;
	float gen(int x, int y);
	GradNoise2D(int s = 0) : seed(s) {};
	int level;
private:
	inline float interpolate(int gridx, int gridy, int dx, int dy);
};

#endif
