#include <cstdio>
#include <iostream>
#include <cstring>
#include <cstdlib>
#include "pgm.h"
#include "ocr.h"
#include "hw1_util.h"
#include "bbox.h"

int main(int argc, char const* argv[])
{
	if (argc != 4) {
		printf("Usage: %s <raw file> <w> <h>\n", argv[0]);
		abort();
	}

	unsigned char imgChar[128*128];
	unsigned char *img;    // input image
	unsigned char *imgBuf; // general buffer...
	int w, h;
	int ww = atoi(argv[2]);
	int hh = atoi(argv[3]);
	unsigned char *imgAll = new unsigned char [ww*hh];

	{
	FILE *fp = fopen(argv[1], "rb");
	fread(imgAll, ww*hh, sizeof(unsigned char), fp);
	fclose(fp);
	}

	std::vector<SubImageIndex> captchaBboxs;
	{
	std::vector<Range1D> r1s = divide_v(imgAll, ww, hh, ww, 250);
	printf("Captcha BBox detection...\n");
	for (int i = 0; i < r1s.size(); ++i) {
		printf("%d %d\n", r1s[i].op, r1s[i].ed);
		std::vector<Range1D> r2s = divide_h(imgAll+ww*r1s[i].op, ww, r1s[i].ed - r1s[i].op, ww, 250);
		Range1D r1_shrink = {r1s[i].op+1, r1s[i].ed-1};
		for (int j = 0; j < r2s.size(); ++j) {
			printf("  %d %d\n", r2s[j].op, r2s[j].ed);
			Range1D r2_shrink = {r2s[j].op+1, r2s[j].ed-1};
			captchaBboxs.push_back(SubImageIndex(r2_shrink, r1_shrink));
		}
	}
	}

	/************* Use this parameter to select captaha ID !! ***************/
	const int p = 0;

	printf("Processing captcha %d...\n", p);

	w = captchaBboxs[p].w;
	h = captchaBboxs[p].h;
	imgBuf = new unsigned char[w*h];
	img = new unsigned char[w*h];
	sample_subimg(img, captchaBboxs[p], imgAll, ww, hh);
	delete[] imgAll;
	{
	FILE *fp1 = fopen("CAP_0_sub.pgm", "wb");
	save_P5_pgm(fp1, w, h, img);
	fclose(fp1);
	}

	median_filter(imgBuf, img, w, h);
	{
	FILE *fp1 = fopen("CAP_1_med.pgm", "wb");
	save_P5_pgm(fp1, w, h, imgBuf);
	fclose(fp1);
	}

	otsu_threshold(img, imgBuf, w, h);
	{
	FILE *fp1 = fopen("CAP_2_Otsu.pgm", "wb");
	save_P5_pgm(fp1, w, h, img);
	fclose(fp1);
	}

	std::vector<SubImageIndex> charBboxs;
	{
	printf("Character BBox detection...\n");
	std::vector<Range1D> r1s = divide_h(img, w, h, w, 128);
	for (int i = 0; i < r1s.size(); ++i) {
		printf("%d %d\n", r1s[i].op, r1s[i].ed);
		std::vector<Range1D> r2s = divide_v(img+r1s[i].op, r1s[i].ed - r1s[i].op, h, w, 128);
		for (int j = 0; j < r2s.size(); ++j) {
			printf("  %d %d\n", r2s[j].op, r2s[j].ed);
			SubImageIndex s(r1s[i], r2s[j]);
			s.dx = s.w / 128.0;
			s.dy = s.h / 128.0;
			s.w = s.h = 128;
			charBboxs.push_back(s);
		}
	}
	}

	printf("Start to match characters\n");
	for (int i = 0; i < charBboxs.size(); ++i) {
		sample_subimg(imgChar, charBboxs[i], img, w, h);
		downsample2(imgChar, imgChar, 128, 128);
		downsample2(imgChar, imgChar,  64,  64);
		char fname[32];
		sprintf(fname, "CAP_3_split%d.pgm", i);
		FILE *fp1 = fopen(fname, "wb");
		save_P5_pgm(fp1, 32, 32, imgChar);
		fclose(fp1);
		Feature f = build_feature32(imgChar);
		printf("%c ", match32(f));
		std::cout << f << std::endl;
	}

	delete[] img, imgBuf;
	return 0;
}
