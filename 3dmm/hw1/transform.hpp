#ifndef __TRANSFORM_HPP__
#define __TRANSFORM_HPP__

#include "defs.hpp"

void centerize(float *vertice, unsigned n, float xs = 1.0f, float ys = 1.0f, float zs = 1.0f);
void get_bbox(float *vertice, unsigned n, Vec3 &vmin, Vec3 &vmax);
void scale(float *vertice, unsigned n, float xs, float ys, float zs);
void translate(float *vertice, unsigned n, float dx, float dy, float dz);
void rotate(float *vertice, unsigned n, float tx, float ty, float tz);

#endif /* end of include guard: __TRANSFORM_HPP__ */
