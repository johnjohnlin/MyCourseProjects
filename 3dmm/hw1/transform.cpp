#include "transform.hpp"
#include <algorithm>
#include <cfloat>
#include <cmath>
using std::max;
using std::min;

void centerize(float *vertice, unsigned n, float xs, float ys, float zs)
{
	Vec3 vmin, vmax;
	get_bbox(vertice, n, vmin, vmax);
	translate(
		vertice, n,
		(vmin.x + vmax.x) * -0.5f,
		(vmin.y + vmax.y) * -0.5f,
		(vmin.z + vmax.z) * -0.5f
	);

	float diag =
		  (vmax.x - vmin.x) * (vmax.x - vmin.x)
		+ (vmax.y - vmin.y) * (vmax.y - vmin.y)
		+ (vmax.z - vmin.z) * (vmax.z - vmin.z);
	float norm = 2.0f/sqrtf(diag);
	scale(vertice, n, norm*xs, norm*ys, norm*zs);
}

void get_bbox(float *vertice, unsigned n, Vec3 &vmin, Vec3 &vmax)
{
	vmin.x = FLT_MAX;
	vmin.y = FLT_MAX;
	vmin.z = FLT_MAX;
	vmax.x = FLT_MIN;
	vmax.y = FLT_MIN;
	vmax.z = FLT_MIN;
	for (int i = 0; i < n; ++i) {
		vmin.x = min(vmin.x, *vertice);
		vmax.x = max(vmax.x, *vertice);
		++vertice;
		vmin.y = min(vmin.y, *vertice);
		vmax.y = max(vmax.y, *vertice);
		++vertice;
		vmin.z = min(vmin.z, *vertice);
		vmax.z = max(vmax.z, *vertice);
		++vertice;
	}
}

void scale(float *vertice, unsigned n, float xs, float ys, float zs)
{
	for (int i = 0; i < n; ++i) {
		*vertice *= xs;
		++vertice;
		*vertice *= ys;
		++vertice;
		*vertice *= zs;
		++vertice;
	}
}

void translate(float *vertice, unsigned n, float dx, float dy, float dz)
{
	for (int i = 0; i < n; ++i) {
		*vertice += dx;
		++vertice;
		*vertice += dy;
		++vertice;
		*vertice += dz;
		++vertice;
	}
}

void rotate(float *v, unsigned n, float tx, float ty, float tz)
{
	float ctx = cosf(tx);
	float cty = cosf(ty);
	float ctz = cosf(tz);
	float stx = sinf(tx);
	float sty = sinf(ty);
	float stz = sinf(tz);
	for (int i = 0; i < n; ++i) {
		float buf;
		if (tx != 0.0f) {
			buf  = ctx * v[1] - stx * v[2];
			v[2] = stx * v[1] + ctx * v[2];
			v[1] = buf;
		}
		if (ty != 0.0f) {
			buf  = cty * v[2] - sty * v[0];
			v[0] = sty * v[2] + cty * v[0];
			v[2] = buf;
		}
		if (tz != 0.0f) {
			buf  = ctz * v[0] - stz * v[1];
			v[1] = stz * v[0] + ctz * v[1];
			v[0] = buf;
		}
		v += 3;
	}
}
