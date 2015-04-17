#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <ctime>
#include "pgm.h"
#include "fft.h"

template <class T>
inline unsigned char clip255(const T x)
{
	return x&(~0xff) ? ((-x)>>31)&0xff : x;
}

inline bool ispow2(const int n)
{
	return n>0 && (n&(n-1)) == 0;
}

void dump_routine1(const char *fname, const int w, const int h, const Complex *ptr, unsigned char *buf)
{
	for (int i = 0; i < w*h; ++i) {
		buf[i] = ptr[i].real();
	}
	FILE *fp = fopen(fname, "wb");
	save_P5_pgm(fp, w, h, buf);
	fclose(fp);
}

void dump_routine2(const char *fname, const int w, const int h, const Complex *ptr, unsigned char *buf)
{
	for (int i = 0; i < w*h; ++i) {
			int converted = 100*log(1.0f + 0.01f*std::abs(ptr[i]));
			buf[i] = clip255<int>(converted);
	}
	FILE *fp = fopen(fname, "wb");
	save_P5_pgm(fp, w, h, buf);
	fclose(fp);
}

int main(int argc, char const* argv[])
{
	if (argc != 4) {
		printf("Usage: %s filename w h\n", argv[0]);
		abort();
	}

	int w, h;
	w = atoi(argv[2]);
	h = atoi(argv[3]);
	if (!(ispow2(w) && ispow2(h))) {
		printf("Width and Height might be power of 2\n");
		abort();
	}
	const int iSiz = w*h;

	unsigned char *img = new unsigned char[iSiz];
	Complex *imgC   = new Complex[iSiz];
	Complex *imgFFT = new Complex[iSiz];
	Complex *imgSFFT = new Complex[iSiz];
	Complex *imgDFT = new Complex[iSiz];
	Complex *imgDRec = new Complex[iSiz];
	Complex *imgFRec = new Complex[iSiz];
	Complex *imgSFRec = new Complex[iSiz];

	FILE *fpIn = fopen(argv[1], "rb");
	fread(img, iSiz, 1, fpIn);
	fclose(fpIn);
	
	for (int i = 0; i < iSiz; ++i) {
		imgC[i] = Complex((float)img[i]);
	}

// #define PROFILE
#ifdef PROFILE
#define PROFILE_FOR for (int i = 0; i < 10; ++i)
#else
#define PROFILE_FOR
#endif

	clock_t c1, c2;

	printf("start dft..."); c1 = clock();
	PROFILE_FOR
	ft_2d_p2(  imgC,  imgDFT, w, h, false, false);
	c2 = clock(); printf("%f sec\n", (float)(c2-c1)/CLOCKS_PER_SEC);
	dump_routine2("DFT.pgm", w, h, imgDFT, img);

	printf("start idft..."); c1 = clock();
	PROFILE_FOR
	ft_2d_p2(imgDFT, imgDRec, w, h, false,  true);
	c2 = clock(); printf("%f sec\n", (float)(c2-c1)/CLOCKS_PER_SEC);
	dump_routine1("DFTRec.pgm", w, h, imgDRec, img);

	printf("start fft..."); c1 = clock();
	PROFILE_FOR
	ft_2d_p2(  imgC,  imgFFT, w, h, true, false);
	c2 = clock(); printf("%f sec\n", (float)(c2-c1)/CLOCKS_PER_SEC);
	dump_routine2("FFT.pgm", w, h, imgFFT, img);

	printf("start ifft..."); c1 = clock();
	PROFILE_FOR
	ft_2d_p2(imgFFT, imgFRec, w, h, true,  true);
	c2 = clock(); printf("%f sec\n", (float)(c2-c1)/CLOCKS_PER_SEC);
	dump_routine1("FFTRec.pgm", w, h, imgFRec, img);

	fft_shift(imgFFT, imgSFFT, w, h);
	ft_2d_p2(imgSFFT, imgSFRec, w, h, true, true);
	dump_routine2("SFFT.pgm", w, h, imgSFFT, img);
	dump_routine1("SFFTRec.pgm", w, h, imgSFRec, img);

	delete[] img, imgC, imgFFT, imgDFT, imgDRec, imgFRec, imgSFFT, imgSFRec;
	return 0;
}
