#include "hw1_util.h"
#include "ocr.h"
#include <iostream>

const char trainStr[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789!@#$%&^*";
const unsigned long long featureTab[70] = {
	4396972773632u,
	1073647616u,
	2774549109312u,
	169216u,
	1073647616u,
	3945881909824u,
	38735974464u,
	1073647616u,
	1649267196164u,
	419833069824u,
	2846093392617u,
	167936u,
	1866163152768u,
	936570303431u,
	229376u,
	3849364421184u,
	313613905920u,
	313666809856u,
	627600012239u,
	4370129084676u,
	135168u,
	2600607021152u,
	2532503851008u,
	4398046278016u,
	4378719020934u,
	3457421348144u,
	992300606u,
	1865542703006u,
	17180047936u,
	628592035807u,
	1864471412702u,
	4363685720057u,
	294210005088u,
	1864470937567u,
	2178739889566u,
	1620276430208u,
	3354140777528u,
	2846093392489u,
	1073602820u,
	258752u,
	124928u,
	2199023497216u,
	274877997056u,
	1717987178432u,
	391512617153u,
	4083329711865u,
	4096u,
	2527592626272u,
	2532503719936u,
	4398046261312u,
	4084512849912u,
	3457354239800u,
	229376u,
	1620378476932u,
	2223264204800u,
	1866148964255u,
	960000692420u,
	1589593307038u,
	1728173184990u,
	4007109054008u,
	1866081898398u,
	2531964612608u,
	4340055928766u,
	39325532224u,
	4396973015745u,
	404799894479u,
	3766457498676u,
	2198469875678u,
	132764933865u,
	4397106724864u
};

inline int myPopcnt64(unsigned long long in)
{
	in = in - ((in >> 1) & 0x5555555555555555ull);
	in = ((in >>  2) & 0x3333333333333333ull) + (in & 0x3333333333333333ull);
	in = ((in >>  4) + in) & 0x0f0f0f0f0f0f0f0full;
	in = ((in >>  8) + in) & 0x00ff00ff00ff00ffull;
	in = ((in >> 16) + in) & 0x0000ffff0000ffffull;
	in = ((in >> 32) + in) & 0x00000000ffffffffull;
	return in;
}

static void f_samelv(Feature &f, int ind, const unsigned char *p)
{
	for (int i = 1; i < 4; ++i) {
		for (int j = 0; j < i; ++j) {
			f[ind] = p[i] > p[j];
			++ind;
		}
	}
}

static void f_difflv(Feature &f, int ind, const unsigned char *p1, const unsigned char *p2)
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

Feature build_feature32(const unsigned char *patch)
{
	unsigned char v[12];
	unsigned char *p = new unsigned char [512];

	downsample2(p    , patch, 32, 32);
	downsample2(p+256, p    , 16, 16);
	v[0] = p[256+8*3+3];
	v[1] = p[256+8*3+4];
	v[2] = p[256+8*4+3];
	v[3] = p[256+8*4+4];
	downsample2(p    , p+256,  8,  8);
	v[4] = p[4*1+1];
	v[5] = p[4*1+2];
	v[6] = p[4*2+1];
	v[7] = p[4*2+2];
	downsample2(p+256, p    ,  4,  4);
	v[ 8] = p[256+2*0+0];
	v[ 9] = p[256+2*0+1];
	v[10] = p[256+2*1+0];
	v[11] = p[256+2*1+1];
	delete[] p;

	int ind = 0;
	Feature ret;
	f_samelv(ret, ind, &v[0]);
	ind += 6;
	f_samelv(ret, ind, &v[4]);
	ind += 6;
	f_samelv(ret, ind, &v[8]);
	ind += 6;
	f_difflv(ret, ind, &v[4], &v[0]);
	ind += 12;
	f_difflv(ret, ind, &v[8], &v[4]);
	return ret;
}

char match32(const Feature f)
{
	unsigned long long full = f.to_ulong();

	int minD = 64;
	int minDId;
	for (int i = 0; i < 70; ++i) {
		unsigned long long x = full ^ featureTab[i];
		int d = myPopcnt64(x);
		if (d < minD) {
			minD = d;
			minDId = i;
		}
	}
	return trainStr[minDId];
}
