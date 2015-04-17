#include "invbox.hpp"
#include <cstring>
#include <limits>
#include <cassert>
#include <cmath>

static inline void mul_add_const(float *dst, float *src, size_t nr, float f);
static inline void mul_add_to(float *dst, float *src, size_t nr, float f);
static inline void mul(float *dst, float *src, size_t nr, float f);
static inline void add_to_row(float *dst, float *src, size_t n);
static inline float GE_one_column(float *dst, float *src, size_t nr);
static void transpose(float *dst, float *src, size_t w, size_t h);

static inline void mul_add_const(float *dst, float *src, size_t nr,
    float fmul, float fadd)
{
	assert(std::isfinite(fadd));
	assert(std::isfinite(fmul));
	for (size_t i = 0; i < nr; ++i) {
		(*dst) = (*src) * fmul + fadd;
		assert(std::isfinite(*src));
		assert(std::isfinite(*dst));
		++dst;
		++src;
	}
}

static inline void mul_add_to(float *dst, float *src, size_t nr, float f)
{
	assert(std::isfinite(f));
	for (size_t i = 0; i < nr; ++i) {
		(*dst) += (*src) * f;
		assert(std::isfinite(*dst));
		++dst;
		++src;
	}
}

static inline void mul(float *dst, float *src, size_t nr, float f)
{
	assert(std::isfinite(f));
	for (size_t i = 0; i < nr; ++i) {
		(*dst) = (*src) * f;
		assert(std::isfinite(*src));
		assert(std::isfinite(*dst));
		++dst;
		++src;
	}
}

static inline void add_to_row(float *dst, float *src, size_t n)
{
  for (int i = 0; i < n; ++i) {
    *dst += *src;
    ++dst;
    ++src;
  }
}

static inline float GE_one_column(float *dst, float *src, size_t nr)
{
	float ratio = - dst[0] / src[0];
	(*dst) = 0.0;
	for (int i = 1; i < nr; ++i) {
		++src;
		++dst;
		(*dst) += ratio * (*src);
	}
	return ratio;
}

static void transpose(float *dst, float *src, size_t w, size_t h)
{
	for (size_t i = 0; i < h; ++i) {
		float *dst_tmp = dst;
		float *src_tmp = src;
		for (size_t j = 0; j < w; ++j) {
			*dst_tmp = *src_tmp;
			dst_tmp += h;
			++src_tmp;
		}
		++dst;
		src += w;
	}
}

void InvBox::GE(float *img, size_t w, size_t h, float *coeff, float *invDiag)
{
	size_t bw = param.r_J;
#if 1
	// naive and readable implementation
	// forward
	for (int i = 0; i < h-1; ++i) {
		for (int j = i+1; j <= std::min(i+bw, h-1); ++j) {
			mul_add_to(img + w*j, img + w*i, w, coeff[j-i-1]);
		}
		coeff += bw;
	}
	// backward
	for (int i = h-1; i >= 1; --i) {
		for (int j = i-1; j >= std::max(0, (int)(i-bw)); --j) {
			mul_add_to(img + w*j, img + w*i, w, coeff[i-j-1]);
		}
		coeff += bw;
	}
	// diagonal
	for (int i = 0; i < h; ++i) {
		mul(img + w*i, img + w*i, w, invDiag[i]);
	}
#else
#endif
}

void InvBox::normalize_image(float *dst, float *src, size_t w, size_t h)
{
	float sl_Af = param.sl_A;
	size_t sl_A = param.sl_A;
	size_t r_A  = param.r_A;
	float boundary = -param.padding * r_A;
	size_t i = 0;
	for (; i < r_A; ++i) {
		mul_add_const(dst, src, w, sl_Af, boundary);
		src += w;
		dst += w;
		boundary += param.padding;
	}
	assert(i == r_A);
	for (; i < h - r_A; ++i) {
		mul(dst, src, w, sl_Af);
		src += w;
		dst += w;
	}
	assert(i == h - r_A);
	for (; i < h; ++i) {
		boundary -= param.padding;
		mul_add_const(dst, src, w, sl_Af, boundary);
		src += w;
		dst += w;
	}
}

void InvBox::solve_1D_and_transpose(float *dst, float *src, size_t w, size_t h,
    float *GECoeff, float *invDiag)
{
	// considering the border
	normalize_image(dst, src, w, h);
	// dst = integral(src)
	build_integral_image(dst, w, h);
	// src = box integrated image (calculated from intrgral image and original image)
	apply_box_filter(src, src, dst, w, h);
	// apply G.E. to src
	GE(src, w, h, GECoeff, invDiag);
	// dst = transpose(src)
	transpose(dst, src, w, h);
}

void InvBox::cal_GECoeff(size_t n, float *coeff, float *invDiag)
{
	size_t r_A = param.r_A;
	size_t r_J = param.r_J;
#if 1
#define ptr_at(y, x) (band_matrix + (y)*param.sl_J + (x))
#define at(y, x) (band_matrix[(y)*param.sl_J + (x)])
	// naive and readable implementation
	// forward
	for (size_t i = 0; i < n-1; ++i) {
		for (size_t j = i+1; j <= std::min(i+r_J, n-1); ++j) {
			size_t nr = std::min(r_J + 1, n - i);
			coeff[j-i-1] = GE_one_column(ptr_at(j, r_J-(j-i)), ptr_at(i, r_J), nr);
			assert(j-i-1 < r_J);
			assert(std::isfinite(coeff[j-i-1]));
		}
		coeff += r_J;
	}
	// backward
	for (size_t i = n-1; i >= 1; --i) {
		for (int j = i-1; j >= std::max(0, (int)i-(int)r_J); --j) {
			coeff[i-j-1] = - at(j, r_J+(i-j)) / at(i, r_J);
			assert(i-j-1 < r_J);
			assert(std::isfinite(coeff[i-j-1]));
		}
		coeff += r_J;
	}
	// diagonal
	for (size_t i = 0; i < n; ++i) {
		invDiag[i] = 1.0 / at(i, r_J);
		assert(std::isfinite(invDiag[i]));
	}
#undef ptr_at
#undef at
#else
#endif
}

void InvBox::build_matrix(size_t n)
{
  // number of each row, e.g. {1, 2, 3, 4, 6, 4, 3, 2, 1}
  size_t i, j;
  size_t r_A = param.r_A, r_J = param.r_J, sl_J = param.sl_J;
  float mu_ = param.mu * (param.sl_A * param.sl_A);
  float *num = new float[sl_J];
  for (i = 0; i <= r_J; ++i) // left-half
    num[i] = i + 1;
  num[i - 1] += mu_; // middle
  for (i = (r_J + 1); i < sl_J; ++i) // right-half
    num[i] = num[sl_J - i - 1];

  // band matrix
  float *ptr = band_matrix;
  for (i = 0; i < n; ++i) {
    memcpy((void*)ptr, (void*)num, sl_J * sizeof(float));
    ptr += sl_J;
  }

  // modify for corners
#define at(x, y) (band_matrix[(x)*param.sl_J + (y)])
#define nan std::numeric_limits<float>::quiet_NaN()
  for (i = 0; i < r_J; ++i) { // fill nan for first/last several rows
    for (j = 0; j < (r_J - i); ++j) {
      at(i, j) = nan;
      at(n - i - 1, sl_J - j - 1) = nan;
    }
  }
#undef nan
#undef at
  ptr = band_matrix + r_J;
  for (i = 0; i < r_A; ++i) {
    float *ptr_tmp = ptr;
    for (j = 0; j < r_A; ++j) {
      *ptr_tmp -= r_A - std::max(i, j);
      ++ptr_tmp;
    }
    ptr += sl_J - 1;
  }
  ptr = band_matrix + (n - r_A) * sl_J + r_J;
  for (i = 0; i < r_A; ++i) {
    float *ptr_tmp = ptr;
    for (j = 0; j < r_A; ++j) {
      *ptr_tmp -= std::min(i, j) + 1;
      ++ptr_tmp;
    }
    ptr += sl_J - 1;
  }

  delete[] num;
}

void InvBox::build_integral_image(float *img, size_t w, size_t h)
{
  for (int i = 0; i < h - 1; ++i) {
    add_to_row(img + (i + 1) * w, img + i * w, w);
  }
}

void InvBox::apply_box_filter(float *dst, float* img, float *intImg, size_t w,
    size_t h)
{
#define dst_at(x, y) (dst[(x)*w + (y)])
#define intImg_at(x, y) (intImg[(x)*w + (y)])
#define img_at(x, y) (img[(x)*w + (y)])
  size_t r_A = param.r_A;
  float mu_ = param.mu * (param.sl_A * param.sl_A);
  for (size_t i = 0; i < h; ++i) {
    for (size_t j = 0; j < w; ++j) {
      size_t up = std::min(h - 1, i + r_A);
      if (i <= r_A) {
        dst_at(i, j) = intImg_at(up, j) + mu_ * img_at(i, j);
      }
      else {
        size_t dn = i - r_A - 1;
        dst_at(i, j) = intImg_at(up, j) - intImg_at(dn, j) + mu_ * img_at(i, j);
      }
    }
  }
#undef dst_at
#undef intImg_at
#undef img_at
}
