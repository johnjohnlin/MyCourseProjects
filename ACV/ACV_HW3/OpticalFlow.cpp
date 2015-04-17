#include "OpticalFlow.hpp"
#include <cstring>

// T must be POD
template <class T>
static void pad_1pixel(T* image, const int w, const int h)
{
	for (int y = 1; y < h-1; ++y) {
		image[w*y    ] = image[w*y+  1];
		image[w*y+w-1] = image[w*y+w-2];
	}
	memcpy(image, image+w, sizeof(T)*w);
	memcpy(image+w*(h-1), image+w*(h-2), sizeof(T)*w);
}

void OpticalFlow::realloc_buffer()
{
	int s = w*h;
	free_buffer();
	motion = new FVec2[s];
	motionAvg = new FVec2[s];
	Ex = new float[s];
	Ey = new float[s];
	Et = new float[s];
}

void OpticalFlow::free_buffer()
{
	delete[] motion;
	delete[] motionAvg;
	delete[] Ex;
	delete[] Ey;
	delete[] Et;
}

void OpticalFlow::calculate_gradient(const float *img1, const float *img2)
{
	for (int y = 1; y < h-1; ++y) {
		for (int x = 1; x < w-1; ++x) {
			const int base = w*y+x;

			// calculate Ex Ey
			{
			float timeSum[2][2];
			// loop unroll?
			for (int i = 0; i < 2; i++) {
				for (int j = 0; j < 2; j++) {
					const int index = base + w*i + j;
					timeSum[i][j] = img1[index]+img2[index];
				}
			}
			Ex[base] = 0.25f * (timeSum[0][1] + timeSum[1][1] - timeSum[0][0] - timeSum[1][0]);
			Ey[base] = 0.25f * (timeSum[1][0] + timeSum[1][1] - timeSum[0][0] - timeSum[0][1]);
			}

			// calculate Et
			{
			float et = 0.0f;
			// loop unroll?
			for (int i = 0; i < 2; i++) {
				for (int j = 0; j < 2; j++) {
					const int index = base + w*i + j;
					et += img2[index] - img1[index];
				}
			}
			Et[base] = et * 0.25f;
			}
		}
	}
}

void OpticalFlow::load_images(const float *img1, const float *img2, const int _w, const int _h)
{
	w = _w;
	h = _h;
	realloc_buffer();
	calculate_gradient(img1, img2);
}

static void box_filter33(FVec2 *motionAvg, const FVec2 *motion, const int w, const int h)
{
	for (int y = 1; y < h-1; ++y) {

		int idx = 2;
		FVec2 colSumCircBuf[3];
		colSumCircBuf[0] = motion[w*y+w  ] + motion[w*y  ] + motion[w*y-w  ];
		colSumCircBuf[1] = motion[w*y+w+1] + motion[w*y+1] + motion[w*y-w+1];

		for (int x = 1; x < w-1; ++x) {
			const int base = w*y+x;
			colSumCircBuf[idx] = motion[base+w+1] + motion[base+1] + motion[base-w+1];
			idx = (idx==2)? 0: (idx+1);
			motionAvg[base] = (colSumCircBuf[0] + colSumCircBuf[1] + colSumCircBuf[2]) * (1.0f/9);
		}
	}
}

void OpticalFlow::update_motion()
{
	for (int y = 1; y < h-1; ++y) {
		for (int x = 1; x < w-1; ++x) {
			const int base = w*y+x;
			const FVec2 g = {Ex[base], Ey[base]};
			const FVec2 mv = motionAvg[base];
			const float A = g.dot(mv) + Et[base];
			const float B = 1 + lambda*g.dot(g);
			motion[base] = mv - g*(A/B);
		}
	}
}

void OpticalFlow::run_iteration()
{
	/* Description:
		We do not use the boundary pixels, and Ex, Ey, Et and motionAvg
		do not store valid value at boundary. However, motion is for
		output and average filtering so we must pad it
	*/
	box_filter33(motionAvg, motion, w, h);
	update_motion();
	pad_1pixel(motion, w, h);
}
