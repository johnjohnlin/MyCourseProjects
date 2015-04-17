#ifndef __FFT_H__
#define __FFT_H__

#include <complex>

typedef std::complex<float> Complex;

void fft_1pass_and_transpose_p2(const Complex *in, Complex *out, const int w, const int h, const bool inverse);
void dft_1pass_and_transpose_p2(const Complex *in, Complex *out, const int w, const int h, const bool inverse);
void ft_2d_p2(const Complex *in, Complex *out, const int w, const int h, bool useFFT, const bool inverse);
void circ_shift(const Complex *in, Complex *out, const int w, const int h, const int sx, const int sy);
inline void fft_shift(const Complex *in, Complex *out, const int w, const int h)
{
	circ_shift(in, out, w, h, w/2, h/2);
}

#endif /* end of include guard */
