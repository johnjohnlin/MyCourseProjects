#include "GradNoise2D.h"
#include <cassert>
#include <cmath>

static const unsigned char perm[256] = {
	151, 160, 137, 91, 90, 15, 131, 13, 201, 95, 96, 53, 194, 233, 7, 225,
	140, 36, 103, 30, 69, 142, 8, 99, 37, 240, 21, 10, 23, 190, 6, 148,
	247, 120, 234, 75, 0, 26, 197, 62, 94, 252, 219, 203, 117, 35, 11, 32,
	57, 177, 33, 88, 237, 149, 56, 87, 174, 20, 125, 136, 171, 168, 68, 175,
	74, 165, 71, 134, 139, 48, 27, 166, 77, 146, 158, 231, 83, 111, 229, 122,
	60, 211, 133, 230, 220, 105, 92, 41, 55, 46, 245, 40, 244, 102, 143, 54,
	65, 25, 63, 161, 1, 216, 80, 73, 209, 76, 132, 187, 208, 89, 18, 169,
	200, 196, 135, 130, 116, 188, 159, 86, 164, 100, 109, 198, 173, 186, 3, 64,
	52, 217, 226, 250, 124, 123, 5, 202, 38, 147, 118, 126, 255, 82, 85, 212,
	207, 206, 59, 227, 47, 16, 58, 17, 182, 189, 28, 42, 223, 183, 170, 213,
	119, 248, 152, 2, 44, 154, 163, 70, 221, 153, 101, 155, 167, 43, 172, 9,
	129, 22, 39, 253, 19, 98, 108, 110, 79, 113, 224, 232, 178, 185, 112, 104,
	218, 246, 97, 228, 251, 34, 242, 193, 238, 210, 144, 12, 191, 179, 162, 241,
	81, 51, 145, 235, 249, 14, 239, 107, 49, 192, 214, 31, 181, 199, 106, 157,
	184, 84, 204, 176, 115, 121, 50, 45, 127, 4, 150, 254, 138, 236, 205, 93,
	222, 114, 67, 29, 24, 72, 243, 141, 128, 195, 78, 66, 215, 61, 156, 180
};

static const float gtab[][2] = {
	{ 2.0f,  1.0f},
	{ 2.0f, -1.0f},
	{-2.0f,  1.0f},
	{-2.0f, -1.0f},
	{ 1.0f,  2.0f},
	{ 1.0f, -2.0f},
	{-1.0f,  2.0f},
	{-1.0f, -2.0f}
};

inline float interpfunc(float t)
{
	float t2 = t*t;
	float t4 = t2*t2;
	return 6*t4*t-15*t4+10*t2*t;
}

unsigned char randgen(int x, int y, int seed)
{
	// different from the original method
	// but used in libnoise
	static const int X_MUL = 31337;
	static const int Y_MUL = 6971;
	static const int S_MUL = 1013;
	static const int SHAMT = 13;
	int ind = X_MUL*x + Y_MUL*y + S_MUL*seed;
	ind ^= ind >> SHAMT;
	return perm[ind & 0xff];
}

inline float dot2(const float *v, const float x, const float y)
{
	return v[0]*x+v[1]*y;
}

inline float GradNoise2D::interpolate(int gridx, int gridy, int dx, int dy)
{
	const float* g0 = gtab[7 & randgen(gridx  , gridy  , seed)];
	const float* g1 = gtab[7 & randgen(gridx+1, gridy  , seed)];
	const float* g2 = gtab[7 & randgen(gridx  , gridy+1, seed)];
	const float* g3 = gtab[7 & randgen(gridx+1, gridy+1, seed)];

	const float dx1f  = ldexpf(dx, -level);
	const float dy1f  = ldexpf(dy, -level);
	const float dx2f = dx1f - 1.0f;
	const float dy2f = dy1f - 1.0f;
	assert(-0.0001f < dx1f && dx1f < 1.0001f);
	assert(-0.0001f < dy1f && dy1f < 1.0001f);
	assert(-1.0001f < dx2f && dx2f < 0.0001f);
	assert(-1.0001f < dy2f && dy2f < 0.0001f);

	const float v0i = dot2(g0, dx1f, dy1f);
	const float v1i = dot2(g1, dx2f, dy1f);
	const float v2i = dot2(g2, dx1f, dy2f);
	const float v3i = dot2(g3, dx2f, dy2f);

	const float ip1 = interpfunc(dx1f);
	const float v01i = v0i*(1.0f-ip1) + v1i*ip1;
	const float v23i = v2i*(1.0f-ip1) + v3i*ip1;

	const float ip2 = interpfunc(dy1f);
	const float v0123i = v01i*(1.0f-ip2) + v23i*ip2;

	return v0123i * 0.33f;
}

float GradNoise2D::gen(int x, int y)
{
	int gridx = x>>level;
	int gridy = y>>level;
	int dx = x - (gridx<<level);
	int dy = y - (gridy<<level);
	assert((dx>>level)==0 && dx>=0 && (dy>>level)==0 && dy>=0);
	return interpolate(gridx, gridy, dx, dy);
}