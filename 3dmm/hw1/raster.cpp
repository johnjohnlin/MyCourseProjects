#include "raster.hpp"
#include <utility>
#include <algorithm>
#include <cmath>
#include <cassert>
using std::swap;

template <class T>
inline void clamp(T& in, const T _min, const T _max)
{
	if (in < _min) {
		in = _min;
	}
	if (in > _max) {
		in = _max;
	}
}

inline void sort_by_x(Vec3 &v1, Vec3 &v2, const float *&a1, const float *&a2)
{
	// make v1.x < v2.x
	if (v1.x > v2.x) {
		swap(v1, v2);
		swap(a1, a2);
	}
}

inline void sort_by_y(Vec3 &v1, Vec3 &v2, const float *&a1, const float *&a2)
{
	// make v1.y < v2.y
	if (v1.y > v2.y) {
		swap(v1, v2);
		swap(a1, a2);
	}
}

void Raster::ndc2ras()
{
	float wf2 = w * 0.5f;
	float hf2 = h * 0.5f;
	p1.x = wf2 * (1.0f + p1.x) - 0.5f;
	p1.y = hf2 * (1.0f + p1.y) - 0.5f;
	p2.x = wf2 * (1.0f + p2.x) - 0.5f;
	p2.y = hf2 * (1.0f + p2.y) - 0.5f;
	p3.x = wf2 * (1.0f + p3.x) - 0.5f;
	p3.y = hf2 * (1.0f + p3.y) - 0.5f;
}

void Raster::triangle(
	const float *_p1, const float *_p2, const float *_p3,
	const float *_attr1, const float *_attr2, const float *_attr3)
{
	attr1 = _attr1;
	attr2 = _attr2;
	attr3 = _attr3;
	p1.x = _p1[0];
	p1.y = _p1[1];
	p1.z = _p1[2];
	p2.x = _p2[0];
	p2.y = _p2[1];
	p2.z = _p2[2];
	p3.x = _p3[0];
	p3.y = _p3[1];
	p3.z = _p3[2];

	ndc2ras();

	// triangle type selection
	
	// bubble sort by y
	sort_by_y(p2, p3, attr2, attr3);
	sort_by_y(p1, p2, attr1, attr2);
	sort_by_y(p2, p3, attr2, attr3);
	assert(p1.y <= p2.y);
	assert(p2.y <= p3.y);
	

	if (p1.y == p2.y) {
		puts("T1");
		sort_by_x(p1, p2, attr1, attr2);
		triangle_1();
	} else {
		if (p2.y == p3.y) {
			sort_by_x(p2, p3, attr2, attr3);
			puts("T2");
			triangle_2();
		} else {
			assert(p1.y <= p2.y);
			assert(p2.y <= p3.y);
			float dx12 = p2.x - p1.x;
			float dy12 = p2.y - p1.y;
			float dx13 = p3.x - p1.x;
			float dy13 = p3.y - p1.y;
			if (dx12 * dy13 > dx13 * dy12) {
				puts("T3");
				triangle_3();
			} else {
				puts("T4");
				triangle_4();
			}
		}
	}
	
}

void Raster::triangle_1()
{
	assert(p1.y == p2.y);
	if (p3.y == p1.y) {
		return;
	}

	int ystart = ceilf(p1.y);
	int yend = ceilf(p3.y);
	clamp(ystart, 0, h-1);
	clamp(yend, 0, h-1);
	assert(ystart <= yend);

	float dyinv = 1.0f / (p3.y - p1.y);
	float dx13 = p3.x - p1.x;
	float dx23 = p3.x - p2.x;
	for (int i = ystart; i < yend; ++i) {
		float yratio = (i - p1.y) * dyinv;
		float xstartf = p1.x + yratio*dx13;
		int xstart = ceilf(xstartf);
		int xend = ceilf(p2.x + yratio*dx23);
		clamp(xstart, 0, w-1);
		clamp(xend, 0, w-1);
		assert(xstart <= xend);

		float dxinv = (1.0f - yratio) * (p2.x - p1.x);
		dxinv = 1.0f/dxinv;
		for (int j = xstart; j < xend; ++j) {
			float xratio = (j - xstartf) * dxinv;
			float c1 = (1.0f - yratio) * (1.0f - xratio);
			float c2 = (1.0f - yratio) * xratio;
			float c3 = yratio;
			float zcur = c1*p1.z + c2*p2.z + c3*p3.z;
			if (zcur >= zbuf[i*w+j]) {
				continue;
			}
			zbuf[i*w+j] = zcur;
			for (int c = 0; c < nch; ++c) {
				*(frame+nch*(i*w+j)+c) = c1*attr1[c] + c2*attr2[c] + c3*attr3[c];
			}
		}
	}
}

void Raster::triangle_2()
{
	assert(p2.y == p3.y);
	if (p1.y == p2.y) {
		return;
	}

	int ystart = floorf(p1.y);
	++ystart;
	int yend = ceilf(p2.y);
	clamp(ystart, 0, h-1);
	clamp(yend, 0, h-1);

	float dyinv = 1.0f / (p2.y - p1.y);
	float dx12 = p2.x - p1.x;
	float dx13 = p3.x - p1.x;
	for (int i = ystart; i < yend; ++i) {
		float yratio = (i - p1.y) * dyinv;
		float xstartf = p1.x + yratio*dx12;
		int xstart = ceilf(xstartf);
		int xend = ceilf(p1.x + yratio*dx13);
		clamp(xstart, 0, w-1);
		clamp(xend, 0, w-1);
		assert(xstart <= xend);

		float dxinv = yratio * (p3.x - p2.x);
		dxinv = 1.0f/dxinv;
		for (int j = xstart; j < xend; ++j) {
			float xratio = (j - xstartf) * dxinv;
			float c1 = 1.0f - yratio;
			float c2 = yratio * (1.0f - xratio);
			float c3 = yratio * xratio;
			float zcur = c1*p1.z + c2*p2.z + c3*p3.z;
			if (zcur >= zbuf[i*w+j]) {
				continue;
			}
			zbuf[i*w+j] = zcur;
			for (int c = 0; c < nch; ++c) {
				*(frame+nch*(i*w+j)+c) = c1*attr1[c] + c2*attr2[c] + c3*attr3[c];
			}
		}
	}
}

void Raster::triangle_3()
{
	float dy12 = p2.y - p1.y;

	float dx13 = p3.x - p1.x;
	float dy13 = p3.y - p1.y;
	float dz13 = p3.z - p1.z;
	float yratio = dy12 / dy13;
	Vec3 pmid = {p1.x+dx13*yratio, p2.y, p1.z+dz13*yratio};
	Vec3 p3_buf = p3;
	const float *attr3_buf = attr3;
	for (int i = 0; i < nch; ++i) {
		midAttr[i] = attr1[i] + (attr3[i]-attr1[i])*yratio;
	}

	p3 = p2;
	p2 = pmid;
	attr3 = attr2;
	attr2 = midAttr;
	triangle_2();

	p1 = pmid;
	p2 = p3;
	p3 = p3_buf;
	attr1 = midAttr;
	attr2 = attr3;
	attr3 = attr3_buf;
	triangle_1();
}

void Raster::triangle_4()
{
	float dy12 = p2.y - p1.y;

	float dx13 = p3.x - p1.x;
	float dy13 = p3.y - p1.y;
	float dz13 = p3.z - p1.z;
	float yratio = dy12 / dy13;
	Vec3 pmid = {p1.x+dx13*yratio, p2.y, p1.z+dz13*yratio};
	Vec3 p3_buf = p3;
	const float *attr3_buf = attr3;
	for (int i = 0; i < nch; ++i) {
		midAttr[i] = attr1[i] + (attr3[i]-attr1[i])*yratio;
	}

	p3 = pmid;
	attr3 = midAttr;
	triangle_2();

	p1 = p2;
	p2 = pmid;
	p3 = p3_buf;
	attr1 = attr2;
	attr2 = midAttr;
	attr3 = attr3_buf;
	triangle_1();
}

void Raster::line(const float *_p1, const float *_p2, const float *_attr1, const float *_attr2)
{
	attr1 = _attr1;
	attr2 = _attr2;
	p1.x = _p1[0];
	p1.y = _p1[1];
	p1.z = _p1[2];
	p2.x = _p2[0];
	p2.y = _p2[1];
	p2.z = _p2[2];

	ndc2ras();

	// line type selection
	
	// make p1.y is smaller
	sort_by_y(p1, p2, attr1, attr2);
	float dx = p2.x - p1.x;
	float dy = p2.y - p1.y;

	if (fabsf(dx) > fabsf(dy)) {
		line_x_major();
	} else {
		line_y_major();
	}
}


void Raster::line_x_major()
{
	/*
	int xstart = floorf(p1.y);
	int xend = ceilf(p2.y);

	if (p1.x == p2.x) {
		return;
	}

	float m = (p2.y - p1.y) / (p2.x - p1.x);

	float cury;
	for (int i = xstart; i < xend; ++i) {
		int j = -roundf(-cury);
		for (int c = 0; c < nch; ++c) {
			*(frame+nch*(i*w+j)+c) = 1.0f;
		}
		cury += m;
	}
	*/
}

void Raster::line_y_major()
{
}

