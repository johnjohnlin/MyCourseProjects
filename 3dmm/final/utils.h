#ifndef __UTILS_H__
#define __UTILS_H__

#define MAX(x, y) ((x)>(y)?(x):(y))
#define MIN(x, y) ((x)<(y)?(x):(y))

#include <CL/cl.h>

struct CLStruct;

void diff(float *dog, float *blurred, int s, int w, int h);
void diff_OMP(float *dog, float *blurred, int s, int w, int h);
void diff_OCL(float *dog, const float *blurred, int s, int w, int h, CLStruct *cls);
void build_gradient_map(float *map, float *dog, int s, int w, int h);
void upSample2(float *dst, float *src, float *buf, int w, int h);
void downSample(float *dst, float *src, int w, int h, int d);
void conv1D_symm_and_transpose(
	float *out, float *in, int w, int h,
	int kernelSize, float *kernel);
void conv1D_symm_and_transpose_OMP(
	float *out, float *in, int w, int h,
	int kernelSize, float *kernel);
void conv1D_symm_and_transpose_OCL(
	cl_mem out, cl_mem in, int w, int h,
	int kernelSize, float *kernel, CLStruct *cls);
void gaussian_blur(float *out, float *in, float *buf, int w, int h, float sigma);
void gaussian_blur_OMP(float *out, float *in, float *buf, int w, int h, float sigma);
void gaussian_blur_OCL(cl_mem out, cl_mem in, cl_mem buf, int w, int h, float sigma, CLStruct *cls);
void matrix_multiply(float* y, float* A, float* x);
void rotate_point(float& dstX, float& dstY, float srcX, float srcY,float theta);
void build_hessian(float* hessian, float* point, int w, int h);
void build_gradient(float* gradient, float* point, int w, int h);
void inv_3d_matrix(float* dst, float* src);

#endif
