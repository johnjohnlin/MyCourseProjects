#include <cstdio>
#include <vector>
#include <cstring>
#include <climits>
#include <cmath>
#include "GradNoise2D.h"
#include "GL/glut.h"

using namespace std;

struct LvAndWeight{
	int level;
	float weight;
};

unsigned char *img;
vector<LvAndWeight> lws;
int w, h, X0, Y0, seed;

static const unsigned char matlabHotColorMap[64][3] = {
	{ 11,   0,   0}, { 21,   0,   0}, { 32,   0,   0}, { 43,   0,   0},
  	{ 53,   0,   0}, { 64,   0,   0}, { 74,   0,   0}, { 85,   0,   0},
  	{ 96,   0,   0}, {106,   0,   0}, {117,   0,   0}, {128,   0,   0},
  	{138,   0,   0}, {149,   0,   0}, {159,   0,   0}, {170,   0,   0},
  	{181,   0,   0}, {191,   0,   0}, {202,   0,   0}, {213,   0,   0},
  	{223,   0,   0}, {234,   0,   0}, {244,   0,   0}, {255,   0,   0},
  	{255,  11,   0}, {255,  21,   0}, {255,  32,   0}, {255,  43,   0},
  	{255,  53,   0}, {255,  64,   0}, {255,  74,   0}, {255,  85,   0},
  	{255,  96,   0}, {255, 106,   0}, {255, 117,   0}, {255, 128,   0},
  	{255, 138,   0}, {255, 149,   0}, {255, 159,   0}, {255, 170,   0},
  	{255, 181,   0}, {255, 191,   0}, {255, 202,   0}, {255, 213,   0},
  	{255, 223,   0}, {255, 234,   0}, {255, 244,   0}, {255, 255,   0},
  	{255, 255,  16}, {255, 255,  32}, {255, 255,  48}, {255, 255,  64},
  	{255, 255,  80}, {255, 255,  96}, {255, 255, 112}, {255, 255, 128},
  	{255, 255, 143}, {255, 255, 159}, {255, 255, 175}, {255, 255, 191},
  	{255, 255, 207}, {255, 255, 223}, {255, 255, 239}, {255, 255, 255}
};

template <int N>
inline int clip_pow2(const int in)
{
	const int mask = (1<<N) - 1;
	return in&~mask? ~(in>>(sizeof(int)*8-1))&mask: in;
}

inline unsigned interlace_uint(unsigned x, unsigned y)
{
	static const unsigned int B[] = {0x55555555, 0x33333333, 0x0F0F0F0F, 0x00FF00FF};
	static const unsigned int S[] = {1, 2, 4, 8};

	x &= 0xffff;
	y &= 0xffff;

	x = (x | (x << S[3])) & B[3];
	x = (x | (x << S[2])) & B[2];
	x = (x | (x << S[1])) & B[1];
	x = (x | (x << S[0])) & B[0];

	y = (y | (y << S[3])) & B[3];
	y = (y | (y << S[2])) & B[2];
	y = (y | (y << S[1])) & B[1];
	y = (y | (y << S[0])) & B[0];

	return x | (y << 1);
}

inline unsigned hash_uint(unsigned a)
{
	a -= (a<<6);
	a ^= (a>>17);
	a -= (a<<9);
	a ^= (a<<4);
	a -= (a<<3);
	a ^= (a<<10);
	a ^= (a>>15);
	return a;
}

inline unsigned hash_xy(const unsigned x, const unsigned y)
{
	return hash_uint(interlace_uint(x, y));
}

inline void terrain(
	const float humid, const float height, unsigned char *rgb
)
{
	unsigned char &r = rgb[0];
	unsigned char &g = rgb[1];
	unsigned char &b = rgb[2];
	if (height < -0.2f) {
		r = g = 0;
		b = 160;
	} else if (height < 0.0f) {
		r = g = 0;
		b = 255;
	} else if (height < 0.02f || humid < -0.2f) {
		r = g = 0xec;
		b = 0xbb;
	} else if (height > 0.2f) {
		r = g = b = 255;
	} else if (humid > 0.15f) {
		r = 0x14;
		g = 0x42;
		b = 0x24;
	} else {
		r = 0x4a;
		g = 0xb9;
		b = 0x1c;
	}
}

void gen_map()
{
	static LvAndWeight lws[] = {
		{ 4, 0.05f},
		{ 6, 0.1f},
		{ 8, 0.4f},
		{10, 0.6f},
		{12, 0.8f}
	};
	GradNoise2D p0(seed);
	GradNoise2D p1(seed+1);

	for (int i = Y0; i < Y0+h; i++) {
		for (int j = X0; j < X0+w; j++) {
			float humid = 0.0f;
			float height = 0.0f;
			for (size_t k = 0; k < sizeof(lws)/sizeof(LvAndWeight); ++k) {
				p0.level = p1.level = lws[k].level;
				float weight = lws[k].weight;
				humid  += p0.gen(i, j) * weight;
				height += p1.gen(i, j) * weight;
			}
			terrain(humid, height, &img[3*((h-1-i+Y0)*w+(j-X0))]);
		}
	}
}

void gen_pop()
{
	static LvAndWeight lws[] = {
		{ 6, 0.3f},
		{ 8, 0.5f},
		{10, 0.2f},
		{12, 0.2f}
	};
	GradNoise2D p(seed);

	for (int i = 0; i < h; i++) {
		for (int j = 0; j < w; j++) {
			float th = 0.4f;
			for (size_t k = 0; k < sizeof(lws)/sizeof(LvAndWeight); ++k) {
				p.level = lws[k].level;
				th += p.gen(i+Y0, j+X0) * lws[k].weight;
			}
			th = max(min(th, 1.0f), 0.0f);
			th = th*th*th;
			unsigned char *rgb = img + 3*((h-1-i)*w+j);
			if (hash_xy(i+Y0, j+X0) > UINT_MAX*th)
				rgb[0] = rgb[1] = rgb[2] = 255;
			else
				rgb[0] = rgb[1] = rgb[2] = 0;
		}
	}
}

void gen_black()
{
	static LvAndWeight lws[] = {
		{ 6, 0.3f},
		{ 8, 0.5f},
		{10, 0.2f},
		{12, 0.2f}
	};
	GradNoise2D p(seed);

	for (int i = 0; i < h; i++) {
		for (int j = 0; j < w; j++) {
			float th = 0.4f;
			for (size_t k = 0; k < sizeof(lws)/sizeof(LvAndWeight); ++k) {
				p.level = lws[k].level;
				th += p.gen(i+Y0, j+X0) * lws[k].weight;
			}
			th = max(min(th, 1.0f), 0.0f);
			th = th*th*th;
			unsigned char *rgb = img + 3*((h-1-i)*w+j);
			if (hash_xy(i+Y0, j+X0) > UINT_MAX*th)
				rgb[0] = rgb[1] = rgb[2] = 255;
			else
				rgb[0] = rgb[1] = rgb[2] = 0;
		}
	}
}

inline void fire(const float in, unsigned char *rgb)
{
	int ind = in * 64.0f;
	ind = clip_pow2<6>(ind);
	rgb[0] = matlabHotColorMap[ind][0];
	rgb[1] = matlabHotColorMap[ind][1];
	rgb[2] = matlabHotColorMap[ind][2];
}

void gen_fire()
{
	static LvAndWeight lws[] = {
		{ 5, 0.3f},
		{ 6, 0.5f},
		{ 7, 0.2f},
		{ 9, 0.2f}
	};
	GradNoise2D p(seed);

	for (int i = 0; i < h; i++) {
		for (int j = 0; j < w; j++) {
			float s = 0.5f;
			for (size_t k = 0; k < sizeof(lws)/sizeof(LvAndWeight); ++k) {
				p.level = lws[k].level;
				s += p.gen(i+Y0, j+X0) * lws[k].weight;
			}
			fire(s, &img[3*((h-1-i)*w+j)]);
		}
	}
}

void gen_cloud()
{
}

inline void wood(const float in, unsigned char *rgb)
{
	float dummy;
	float residue = modff(in, &dummy);
	if (residue < 0.0f) {
		residue += 1.0f;
	}
	residue *= residue;
	if (residue > 0.8f) {
		residue = 1.0f;
	}

	rgb[0] = 0xff + residue * (0xc7 - 0xff);
	rgb[1] = 0x9e + residue * (0x76 - 0x9e);
	rgb[2] = 0x00 + residue * (0x00 - 0x00);
}

void gen_wood()
{
	static LvAndWeight lws[] = {
		{ 7,  3.0f},
		{ 8, 11.0f}
	};
	GradNoise2D p(seed);

	for (int i = 0; i < h; i++) {
		for (int j = 0; j < w; j++) {
			float s = 0.0f;
			for (size_t k = 0; k < sizeof(lws)/sizeof(LvAndWeight); ++k) {
				p.level = lws[k].level;
				s += p.gen(i+Y0, j+X0) * lws[k].weight;
			}
			wood(s, &img[3*((h-1-i)*w+j)]);
		}
	}
}

#define N_TEX_MODE 5
const char __s0[] = "Map";
const char __s1[] = "Population";
const char __s2[] = "Fire";
const char __s3[] = "Cloud";
const char __s4[] = "Wood";
const char *texModeStrs[N_TEX_MODE] = {__s0, __s1, __s2, __s3, __s4};
void (*texModeFuncs[N_TEX_MODE])() = {gen_map, gen_pop, gen_fire, gen_cloud, gen_wood};
void (*texModeFunc)() = gen_map;

void cleanup()
{
	delete[] img;
}

void display()
{
	//printf("display (%d,%d)\n", w, h);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	texModeFunc();
	glDrawPixels(w, h, GL_RGB, GL_UNSIGNED_BYTE, img);
	glutSwapBuffers();
	printf("Use WASDwasd to navigate, tT to change texture mode\n");
}

void reshape(int _w, int _h)
{
	//printf("reshape (w,h)=(%d,%d)\n", _w, _h);
	if (_w*_h > w*h) {
		delete[] img;
		img = new unsigned char[_w*_h*3];
	}
	w = _w;
	h = _h;
}

void keyboard(unsigned char key, int x, int y)
{
	static int tm;
	// printf("keyboard\n");
	switch(key) {
		case 's':
			Y0 += h/4;
			break;
		case 'w':
			Y0 -= h/4;
			break;
		case 'd':
			X0 += w/4;
			break;
		case 'a':
			X0 -= w/4;
			break;
		case 'S':
			Y0 += h/2;
			break;
		case 'W':
			Y0 -= h/2;
			break;
		case 'D':
			X0 += w/2;
			break;
		case 'A':
			X0 -= w/2;
			break;
		case 't':
			tm = (tm+1) % N_TEX_MODE;
			texModeFunc = texModeFuncs[tm];
			printf("Change texture mode: %s\n", texModeStrs[tm]);
			break;
		case 'T':
			tm = (tm+N_TEX_MODE-1) % N_TEX_MODE;
			texModeFunc = texModeFuncs[tm];
			printf("Change texture mode: %s\n", texModeStrs[tm]);
			break;
		default:
			return;
	}
	glutPostRedisplay();
}

int main(int argc, char **argv)
{
	w = 913;
	h = 640;
	img = new unsigned char[w*h*3];

	glutInit(&argc, argv);
	glutInitWindowSize(w, h);
	glutInitDisplayMode(GLUT_DOUBLE);
	glutCreateWindow("Window");
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyboard);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	glutMainLoop();

	return 0;
}
