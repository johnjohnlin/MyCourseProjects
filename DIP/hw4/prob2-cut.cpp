#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <algorithm>
#include <vector>
#include <bitset>
#include <iostream>
#include "pgm.h"
#include "hw1_util.h"
const char str[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789!@#$%&^*";

struct BBox {
	int u, b, l, r;
};

#define N 12

struct Intensity {
	unsigned char v[N];
};

typedef std::bitset<42> Feature;

void sample(
	const unsigned char *img, unsigned char *imgBlk,
	const BBox b, const int sl, const int w
)
{
	const float invSl = 1.0f / sl;
	const float yStride = (b.b-b.u) * invSl;
	const float xStride = (b.r-b.l) * invSl;

	float fy = b.u;
	for (int i = 0; i < sl; ++i) {
		float fx = b.l;
		for (int j = 0; j < sl; ++j) {
			int ix = fx;
			int iy = fy;
			char v = img[iy*w+ix];
			imgBlk[i*sl+j] = v>>7;
			fx += xStride;
		}
		fy += yStride;
	}
}

BBox get_bbox(const unsigned char *img, const int w, const int h)
{
	BBox ret = {h, 0, w, 0};
	for (int i = 0; i < h; ++i) {
		for (int j = 0; j < w; ++j) {
			if (img[w*i+j] != 255) {
				ret.u = std::min(i, ret.u);
				ret.b = std::max(i+1, ret.b);
				ret.l = std::min(j, ret.l);
				ret.r = std::max(j+1, ret.r);
			}
		}
	}
	int d = ret.r - ret.l;
	int mind = 15;
	if (d < mind) {
		int p = (mind-d)/2;
		ret.r += p;
		ret.l -= p;
	}
	if (ret.r > w) {
		ret.l -= ret.r - w;
		ret.r = w;
	} else if (ret.l < 0) {
		ret.r -= ret.l;
		ret.l = 0;
	}
	return ret;
}

void cut(
	const unsigned char *img, unsigned char *imgBlk, unsigned char *imgLarge,
	const BBox b, const int sl, const int w
)
{
	sample(img, imgBlk, b, sl, w);
	BBox bb = get_bbox(imgBlk, sl, sl);
	sample(imgBlk, imgLarge, bb, sl, sl);
}

void dump(const unsigned char *img, const int l)
{
	for (int i = 0; i < l; ++i) {
		for (int j = 0; j < l; ++j) {
			printf("%3hhu ", *img);
			++img;
		}
		putchar('\n');
	}
}

Intensity build_intensity(const unsigned char *img)
{
	Intensity ret;
	unsigned char *p = new unsigned char [512];

	downsample2(p    ,   img, 32, 32);
	downsample2(p+256, p    , 16, 16);
	dump(p+256, 8);
	ret.v[0] = p[256+8*3+3];
	ret.v[1] = p[256+8*3+4];
	ret.v[2] = p[256+8*4+3];
	ret.v[3] = p[256+8*4+4];
	downsample2(p    , p+256,  8,  8);
	dump(p, 4);
	ret.v[4] = p[4*1+1];
	ret.v[5] = p[4*1+2];
	ret.v[6] = p[4*2+1];
	ret.v[7] = p[4*2+2];
	downsample2(p+256, p    ,  4,  4);
	dump(p+256, 2);
	ret.v[ 8] = p[256+2*0+0];
	ret.v[ 9] = p[256+2*0+1];
	ret.v[10] = p[256+2*1+0];
	ret.v[11] = p[256+2*1+1];

	delete[] p;
	return ret;
}

void f_samelv(Feature &f, int ind, const unsigned char *p)
{
	for (int i = 1; i < 4; ++i) {
		for (int j = 0; j < i; ++j) {
			f[ind] = p[i] > p[j];
			++ind;
		}
	}
}

void f_difflv(Feature &f, int ind, const unsigned char *p1, const unsigned char *p2)
{
	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 4; ++j) {
			if (i == j)
				continue;
			f[ind] = p1[i] > p2[j];
			++ind;
		}
	}
}

Feature build_feature(const Intensity &vs)
{
	int ind = 0;
	Feature ret;
	f_samelv(ret, ind, &vs.v[0]);
	ind += 6;
	f_samelv(ret, ind, &vs.v[4]);
	ind += 6;
	f_samelv(ret, ind, &vs.v[8]);
	ind += 6;
	f_difflv(ret, ind, &vs.v[4], &vs.v[0]);
	ind += 12;
	f_difflv(ret, ind, &vs.v[8], &vs.v[4]);
	return ret;
}

int main(int argc, char const* argv[])
{
	const int l = 32;
	const int w = 450;
	const int h = 248;
	const int iSiz = w*h;

	unsigned char *img = new unsigned char[iSiz];
	unsigned char *imgBlk = new unsigned char[l*l];
	unsigned char *imgBlk2 = new unsigned char[l*l];
	FILE *fpIn = fopen("training.raw", "rb");
	fread(img, iSiz, 1, fpIn);
	fclose(fpIn);

	std::vector<Feature> fvs;

	int imgID = 0;
	for (int i = 2; i < h-20; i += 47) {
		for (int j = 7; j < w-10; j += 31) {
			BBox b = {i, i+46, j, j+31};
			cut(img, imgBlk, imgBlk2, b, l, w);
			char fname[16];
			sprintf(fname, "B%d.pgm", imgID);
			++imgID;
			FILE *fp = fopen(fname, "wb");
			save_P5_pgm(fp, l, l, imgBlk2);
			Intensity i = build_intensity(imgBlk2);
			for (int k = 0; k < N; ++k) {
				printf("%3hhu ", i.v[k]);
			}
			putchar('\n');
			fvs.push_back(build_feature(i));
			fclose(fp);
		}
	}

	for (int i = 0; i < fvs.size(); ++i) {
		std::cout << str[i] << ' ' << fvs[i] << ' ' << fvs[i].to_ulong() << std::endl;
	}

	delete[] img, imgBlk, imgBlk2;
	return 0;
}
