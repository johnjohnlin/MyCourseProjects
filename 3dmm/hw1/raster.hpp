#ifndef __RASTER_H__
#define __RASTER_H__

#include "defs.hpp"
#include "portab.hpp"
#include <cstring>
#include <cstdio>
#include <algorithm>
#include <cfloat>

using std::fill;

class Raster {
public:
	inline Raster(int _w, int _h, int _nch = 3);
	inline ~Raster();
	inline void reset(int _w, int _h, int _nch);
	inline void init(int _w, int _h, int _nch);
	inline void release();
	inline void clear_color(const float *attr = NULL, bool clearz = true);
	void line(const float *_p1, const float *_p2, const float *attr1, const float *attr2);
	void triangle(
		const float *_p1, const float *_p2, const float *_p3,
		const float *attr1, const float *attr2, const float *attr3);

	void line_x_major();
	void line_y_major();

	// 1                    2                  3                    4
	// top edge flat        lower edge flat    y-mid point @ right  y-mid @ left
	//
	// 1**********2               1*                    1*                   1*
	//   *      *                * *                   * *                  * *
	//     *   *               *   *                 *   *                *   *
	//       **             2*******3              *  ****2            2***   *
	//        3                                 3*****                     ****3
	// use these carefully!
	void triangle_1();
	void triangle_2();
	void triangle_3();
	void triangle_4();

	inline const float *get_zbuf();
	inline const float *get_frame();

	void ndc2ras();
private:
	int nch, w, h;
	Vec3 p1, p2, p3;
	const float *attr1, *attr2, *attr3;
	float *zbuf, *frame;
	float *midAttr;
};

inline Raster::Raster(int _w, int _h, int _nch)
{
	init(_w, _h, _nch);
}

inline Raster::~Raster()
{
	release();
}

inline void Raster::init(int _w, int _h, int _nch)
{
	w = _w;
	h = _h;
	nch = _nch;

	if (w < 0 | h < 0 | nch < 0) {
		puts("Invalid frame buffer size, set to 64*64*3");
		w = h = 64;
		nch = 3;
	}
	zbuf = new float [w*h];
	frame = new float [w*h*nch];
	midAttr = new float [nch];
}

inline void Raster::release()
{
	delete[] zbuf;
	delete[] frame;
	delete[] midAttr;
}

inline void Raster::reset(int _w, int _h, int _nch)
{
	release();
	init(_w, _h, _nch);
}

inline const float *Raster::get_zbuf()
{
	return zbuf;
}

inline const float *Raster::get_frame()
{
	return frame;
}

inline void Raster::clear_color(const float *attr, bool clearz)
{
	if (attr == NULL) {
		memset(frame, 0, sizeof(float)*nch*w*h);
	} else {
		for (int i = 0; i < w*h; ++i) {
			for (int j = 0; j < nch; ++j) {
				frame[i*nch+j] = attr[j];
			}
		}
	}

	if (clearz) {
		fill(zbuf, zbuf+w*h, FLT_MIN);
	}
}

#endif /* end of include guard: __RASTER_H__ */
