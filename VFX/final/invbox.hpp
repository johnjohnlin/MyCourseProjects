#ifndef __INVBOX__
#define __INVBOX__

#include "invbox.tpp"

class InvBox;

class Param {
public:
	friend class InvBox;
	size_t w, h, r;   ///< box filter size = (2*r+1)**2
	float mu;
	float padding;
	Param& operator = (const Param &rhs)
  {
		w = rhs.w;
		h = rhs.h;
		r = rhs.r;
		mu = rhs.mu;
		padding = rhs.padding;
		sl_A = (r << 1) + 1;
		sl_J = (r << 2) + 1;
		r_A = r;
		r_J = r << 1;
    return *this;
	}
private:
	// Cached and memoric variables
	size_t sl_A, sl_J; ///< side length
	size_t r_A, r_J; ///< radius, namely bandwidth
};

class InvBox {
public:
	InvBox(): fbuf()
	{
	}

	InvBox(const Param &param)
	{
		set_param(param);
	}

	// No need for destructor
	// Memory free is implicitly done by the default destructor
	inline void solve(float *img);
	inline void set_param(const Param &param, bool forceRealloc=false);
	inline void clean();
private:
	Buffer<float> fbuf;
	float *imgTmp;
	float *band_matrix;
	float *hGECoeff;
	float *hInvDiag;
	float *vGECoeff;
	float *vInvDiag;
	Param param;
	void GE(float *img, size_t w, size_t h, float *coeff, float *invDiag);
	void cal_GECoeff(size_t n, float *coeff, float *invDiag);
	void build_matrix(size_t n);
	void solve_1D_and_transpose(float *dst, float *src, size_t w, size_t h,
      float *GECoeff, float *invDiag);
	void normalize_image(float *dst, float *src, size_t w, size_t h);
	void apply_box_filter(float *dst, float* img, float *intImg, size_t w,
      size_t h);
	void build_integral_image(float *img, size_t w, size_t h);
	inline size_t cal_fbuf_size(const Param &param);
	inline void assign_ptr();
	inline void cal_HVCoeff();
};

#include "invbox.ipp"

#endif
