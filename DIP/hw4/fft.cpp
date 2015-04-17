#include <cmath>
#include <cstdio>
#include <algorithm>
#include "fft.h"

inline const Complex cexpf(const float theta)
{
	return Complex(cosf(theta), sinf(theta));
}

void dft_1pass_and_transpose_p2(const Complex *in, Complex *out, const int w, const int h, const bool inverse)
{
	Complex *coeff = new Complex[w];
	const int mask = w-1; // only works for w = pow of 2
	for (int i = 0; i < w; ++i) {
		static const float inc = inverse? -2.0f*M_PI/w: 2.0f*M_PI/w;
		coeff[i] = cexpf(inc*i);
	}

	for (int i = 0; i < h; ++i) {
		const Complex *row = in + i*w;
		for (int j = 0; j < w; ++j) {
			Complex sum;
			for (int k = 0; k < w; ++k) {
				const int c = j*k;
				sum += coeff[c & mask] * row[k];
			}
			out[j*w+i] = sum;
		}
	}

	if (inverse) {
		float iw = 1.0f / w;
		for (int i = 0; i < w*h; ++i) {
			out[i] *= iw;
		}
	}

	delete[] coeff;
}

static void fft_col(Complex *in, const int n, const Complex *coeff)
{
	const int mask = n-1; // only works for w = pow of 2
	for (int chunk = 1, N = n/2; chunk < n; chunk *= 2, N /= 2) {
		for (int i = 0; i < n; i += 2*chunk) {
			for (int j = 0; j < chunk; ++j) {
				int k = i+j;
				Complex E = in[k      ];
				Complex O = in[k+chunk];
				int c = N*k;
				in[k      ] = E + O*coeff[c & mask];
				in[k+chunk] = E - O*coeff[c & mask];
			}
		}
	}
}

static inline int rev_bit(int in, const unsigned n)
{
	int ret = 0;
	for (int i = 0; i < n; ++i) {
		ret <<= 1;
		ret |= (in&1);
		in >>= 1;
	}
	return ret;
}

static unsigned log2int(unsigned in)
{
	// http://graphics.stanford.edu/~seander/bithacks.html
	static const int multiplyDeBruijnBitPosition2[32] = {
		0, 1, 28, 2, 29, 14, 24, 3, 30, 22, 20, 15, 25, 17, 4, 8,
		31, 27, 13, 23, 21, 19, 16, 7, 26, 12, 18, 6, 11, 5, 10, 9
	};
	return multiplyDeBruijnBitPosition2[ ((in * 0x077CB531u) >> 27 ) & 0x1f];
}

static void gen_radix_rev(int *radRev, const int n)
{
	unsigned nBit = log2int(n);
	for (int i = 0; i < n; ++i) {
		radRev[i] = rev_bit(i, nBit);
	}
}

void fft_1pass_and_transpose_p2(const Complex *in, Complex *out, const int w, const int h, const bool inverse)
{
	Complex *coeff = new Complex[w];
	for (int i = 0; i < w; ++i) {
		static const float inc = inverse? -2.0f*M_PI/w: 2.0f*M_PI/w;
		coeff[i] = cexpf(inc*i);
	}


	Complex *row = new Complex[w];
	int *radRev = new int [w];
	gen_radix_rev(radRev, w);

	for (int i = 0; i < h; ++i) {
		for (int j = 0; j < w; ++j) {
			row[j] = in[i*w+radRev[j]];
		}
		fft_col(row, w, coeff);
		for (int j = 0; j < w; ++j) {
			out[j*w+i] = row[j];
		}
	}

	if (inverse) {
		float iw = 1.0f / w;
		for (int i = 0; i < w*h; ++i) {
			out[i] *= iw;
		}
	}

	delete[] row, radRev;
}

void ft_2d_p2(const Complex *in, Complex *out, const int w, const int h, bool useFFT, const bool inverse)
{
	Complex *buf = new Complex[w*h];

	if (w == 1) {
		return;
	}

	if (useFFT) {
		fft_1pass_and_transpose_p2(in, buf, w, h, inverse);
		fft_1pass_and_transpose_p2(buf, out, h, w, inverse);
	} else {
		dft_1pass_and_transpose_p2(in, buf, w, h, inverse);
		dft_1pass_and_transpose_p2(buf, out, h, w, inverse);
	}
	delete[] buf;
}

void circ_shift(const Complex *in, Complex *out, const int w, const int h, const int sx, const int sy)
{
	int sx_clamp = sx % w;
	int sy_clamp = sy % h;

	for (int i = 0; i < h; ++i) {
		for (int j = 0; j < w; ++j) {
			int iDst = i+sy;
			if (iDst >= w) {
				iDst -= w;
			}
			Complex *dstCol = out + w*iDst;
			std::copy(in+sx, in+w, dstCol);
			std::copy(in, in+sx, dstCol+w-sx);
		}
		in += w;
	}
}
