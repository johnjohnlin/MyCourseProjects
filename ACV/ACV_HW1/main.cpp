#include <cstdio>
#include "ME.hpp"

int main()
{
	const int w = 386, h = 386;
	FILE *fp1 = fopen("trucka.im", "rb");
	FILE *fp2 = fopen("truckb.im", "rb");

	MEClass mec;

	mec.w = w;
	mec.h = h;
	mec.blkSiz = 8;
	mec.searchRange = 16;

	mec.i1 = new unsigned char[w*h];
	mec.i2 = new unsigned char[w*h];
	fseek(fp1, 216, SEEK_SET);
	fseek(fp2, 216, SEEK_SET);
	fread(mec.i1, 1, w*h, fp1);
	fread(mec.i2, 1, w*h, fp2);

	std::vector<XY> mvs;
	mec.motion_estimation(mvs);
	for (size_t i = 0; i < mvs.size(); i++) {
		printf("%4d,%4d\n", mvs[i].x, mvs[i].y);
	}

	fclose(fp1);
	fclose(fp2);

	delete[] mec.i1, mec.i2;
	return 0;
}
