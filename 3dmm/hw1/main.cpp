#include "raster.hpp"
#include "objread.hpp"
#include "transform.hpp"
#include <cstdio>
#include <cmath>
#include <cstring>

#if __linux__
#include <sys/time.h>
#endif

void printbuf(const float *b)
{
	for (int i = 0; i < 16; ++i) {
		for (int j = 0; j < 16; ++j) {
			printf("%1.0f  ", b[16*i+j]);
		}
		putchar('\n');
	}
}

int main(int argc, const char *argv[])
{
#if __linux__
	timeval tv_start, tv_end;
	long long unsigned totaltime = 0;
#endif
	FILE *fp;
	FILE *configFp = fopen("config.txt", "r");
	// rotation, scale, translation
	float rx, ry, rz, sx, sy, sz;
	if (configFp) {
		puts("config.txt found");
		fscanf(
			configFp, "%f %f %f %f %f %f",
			&rx, &ry, &rz, &sx, &sy, &sz
		);
		printf(
			"Rotation (%f, %f, %f)\nScaling: (%f, %f, %f)\n",
			rx, ry, rz, sx, sy, sz
		);
		fclose(configFp);
	}
	if (argc != 2) {
		fputs("Use program with 1 argument, a config.txt can also be used\n", stderr);
		return 1;
	} else {
		fp = fopen(argv[1], "r");
		if (fp == NULL) {
			fputs("Invalid file\n", stderr);
			return 1;
		}
	}
	float *coord, *color;
	unsigned *indice;
	unsigned numVert, numTri;
#if __linux__
	gettimeofday(&tv_start, NULL);
#endif
	read_hw1_obj_file(fp, coord, color, indice, numVert, numTri);
#if __linux__
	gettimeofday(&tv_end, NULL);
	totaltime = 1000000u * (tv_end.tv_sec - tv_start.tv_sec);
	totaltime += tv_end.tv_usec - tv_start.tv_usec;

	printf("spend %llu us on reading object\n", totaltime);
#endif
	fclose(fp);
	// testing
	Raster r(1024, 1024, 3);
	// Raster r(128, 128, 3);
	r.clear_color();
#if __linux__
	gettimeofday(&tv_start, NULL);
#endif
	centerize(coord, numVert, sx, sy, sz);
	if (color == NULL) {
		color = new float[numVert*3];
		memcpy(color, coord, numVert*3*sizeof(float));
		// shift to 0~1
		translate(color, numVert, 1.0f, 1.0f, 1.0f);
		scale(color, numVert, 0.5f, 0.5f, 0.5f);
	}
	rotate(coord, numVert, rx, ry, rz);
#if __linux__
	gettimeofday(&tv_end, NULL);
	totaltime = 1000000u * (tv_end.tv_sec - tv_start.tv_sec);
	totaltime += tv_end.tv_usec - tv_start.tv_usec;

	printf("spend %llu us on rotation, translation, ...etc\n", totaltime);
#endif

#if __linux__
	gettimeofday(&tv_start, NULL);
#endif
	for (unsigned i = 0; i < numTri; ++i) {
		unsigned i1 = indice[3*i  ];
		unsigned i2 = indice[3*i+1];
		unsigned i3 = indice[3*i+2];
		r.triangle(
			coord+i1*3, coord+i2*3, coord+i3*3,
			color+i1*3, color+i2*3, color+i3*3
		);
		// if (i < 10) {
		// 	printf(
		// 		"raster (%f, %f) (%f, %f) (%f, %f)\n",
		// 		512+512*coord[i1*3], 512+512*coord[i1*3+1],
		// 		512+512*coord[i2*3], 512+512*coord[i2*3+1],
		// 		512+512*coord[i3*3], 512+512*coord[i3*3+1]
		// 	);
		// }
	}
#if __linux__
	gettimeofday(&tv_end, NULL);
	totaltime = 1000000u * (tv_end.tv_sec - tv_start.tv_sec);
	totaltime += tv_end.tv_usec - tv_start.tv_usec;

	printf("spend %llu us on rastering\n", totaltime);
#endif

	const float *buf = r.get_frame();

	FILE *imgfp = fopen("img.pgm", "w");
#if __linux__
	gettimeofday(&tv_start, NULL);
#endif
	fprintf(imgfp, "P3\n1024 1024\n255");
	// fprintf(imgfp, "P3\n128 128\n255");
	for (int i = 0; i < 1024*1024; ++i) {
	// for (int i = 0; i < 128*128; ++i) {
		if ((i&1023) == 0) {
		// if ((i&127) == 0) {
			fputc('\n', imgfp);
		}
		fprintf(imgfp, "%d %d %d ",
			(int)(fabs(buf[3*i  ]*255.0f)),
			(int)(fabs(buf[3*i+1]*255.0f)),
			(int)(fabs(buf[3*i+2]*255.0f))
		);
	}
#if __linux__
	gettimeofday(&tv_end, NULL);
	totaltime = 1000000u * (tv_end.tv_sec - tv_start.tv_sec);
	totaltime += tv_end.tv_usec - tv_start.tv_usec;

	printf("spend %llu us on writing pgm\n", totaltime);
#endif
	fclose(imgfp);


	// clean up
	delete[] coord;
	delete[] indice;
	delete[] color;

	return 0;
}
