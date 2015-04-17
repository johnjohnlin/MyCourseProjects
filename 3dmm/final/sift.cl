#include "clshare.h"

__kernel void conv_and_trans(
	__global float *dst, __global float *src,
	__global const float *kern, int kernSiz,
	int w, int h
)
{
	int X = get_global_id(0);
	if (X >= w) {
		return ;
	}
	__global const float *row = src;
	__global float *col = dst + X*h;
	for	(int i = 0; i < h; ++i) {
		float sum = 0.0f;
		for (int j = -kernSiz; j <= kernSiz; ++j) {
			int x = X+j;
			x = max(x, 0);
			x = min(x, w-1);
			sum += row[x] * kern[j+kernSiz];
			// sum += row[x];
		}
		*col = sum;
		++col;
		row += w;
	}
}

__kernel void conv_and_trans2(
	__global float *dst, __global float *src,
	__global const float *kern, const int kernSiz,
	int w, int h
)
{
	int X = get_global_id(0);
	if (X >= w) {
		return ;
	}

	float buffer[2*MAX_KERNSIZ+1];
	__global float *src_tmp = src + X;
	buffer[0] = *src_tmp;
	for (int i = 1; i <= kernSiz; ++i) {
		buffer[i] = buffer[0];
	}
	for (int i = 1; i <= kernSiz; ++i) {
		src_tmp += w;
		buffer[kernSiz+i] = *src_tmp;
	}

	int start = 0;
	__global float *dst_tmp = dst + h*X;
	for (int i = 0; i < h; ++i) {
		float sum = 0.0f;
		for (int j = 0; j <= 2*kernSiz; ++j) {
			int ind = j+start;
			if (ind > 2*kernSiz) {
				ind -= 2*kernSiz+1;
			}
			sum += kern[j] * buffer[ind];
		}
		*dst_tmp = sum;
		/*
		int ind = kernSiz+start;
		if (ind > 2*kernSiz) {
			ind -= 2*kernSiz+1;
		}
		*dst_tmp = buffer[ind];
		*/

		++dst_tmp;
		if (i < h-kernSiz-1) {
			src_tmp += w;
		}
		buffer[start] = *src_tmp;

		++start;
		if (start > 2*kernSiz) {
			start = 0;
		}
	}
}

__kernel void diff(
	__global float *dog, __global float *gauss,
	int s, int w, int h
)
{
	int X = get_global_id(0);
	if (X >= w) {
		return ;
	}
	__global float *src1 = gauss + X;
	__global float *src2 = gauss + X + w*h;
	__global float *dst = dog + X;
	for	(int i = 0; i < (s-1)*h; ++i) {
		*dst = *src2 - *src1;

		src1 += w;
		src2 += w;
		dst += w;
	}
}
