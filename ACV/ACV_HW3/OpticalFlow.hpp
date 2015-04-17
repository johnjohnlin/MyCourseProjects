#ifndef __OPTICALFLOW_H__
#define __OPTICALFLOW_H__

#include "VectorType.hpp"
typedef Vec<float, 2> FVec2;

class OpticalFlow {
	float *Ex, *Ey, *Et;
	FVec2 *motionAvg;
	int w, h;

	void realloc_buffer();
	void free_buffer();
	void calculate_gradient(const float *img1, const float *img2);
	void update_motion();
public:
	FVec2 *motion; // use it when initialization and getting result...
	float lambda;
	void load_images(const float *img1, const float *img2, const int _w, const int _h);
	void run_iteration();

	OpticalFlow(): Ex(0), Ey(0), Et(0), motion(0), motionAvg(0), w(0), h(0)
	{
	}

	~OpticalFlow()
	{
		free_buffer();
	}
};

#endif /* end of include guard */
