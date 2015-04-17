#include "sift.h"
#include "utils.h"
#include "pgm.h"
#include "clstruct.h"

#include <ctime>
#include <cstring>
#ifdef _MSC_VER
#define _USE_MATH_DEFINES
#endif
#include <math.h>
#include <cstdio>
#include <cfloat>
#include <omp.h>

Sift::Sift(
	float *_img, int _w, int _h, AccerModel _accel,
	int _octMin, int _numOct, int _lvPerScale,
	bool _dumpImage
)
{
	img = _img;
	wmax = imgw = _w;
	hmax = imgh = _h;
	octMin = _octMin;
	if (octMin < 0) {
		wmax <<= -octMin;
		hmax <<= -octMin;
	} else if (octMin > 0) {
		wmax >>= octMin;
		hmax >>= octMin;
	}
	numOct = _numOct;
	lvPerScale = _lvPerScale;
	sigma0 = 1.6f * powf(2.0f, 1.0f/lvPerScale);
	hasGaussian = hasGrads = false;
	accel = _accel;
	dumpImage = _dumpImage;

	cls = new CLStruct;
	if (accel == Accel_OCL) {
		cl_int cle;

		cle = clGetPlatformIDs(1, &cls->platform, NULL);
		ABORT_IF(cle != CL_SUCCESS, "Cannot get OpenCL platform ID\n");

		cle = clGetDeviceIDs(cls->platform, CL_DEVICE_TYPE_GPU, 1, &cls->device, NULL);
		ABORT_IF(cle != CL_SUCCESS, "Cannot get OpenCL device ID\n");

		cls->context = clCreateContext(0, 1, &cls->device, NULL, NULL, &cle);
		ABORT_IF(cle != CL_SUCCESS || !cls->context, "Cannot create OpenCL context\n");

		cls->cqueue = clCreateCommandQueue(cls->context, cls->device, 0, &cle);
		ABORT_IF(cle != CL_SUCCESS || !cls->cqueue, "Cannot create OpenCL Command queue\n");

		FILE *shaderFp = fopen("sift.cl", "r");
		ABORT_IF(!shaderFp, "Cannot open OpenCL shader\n");

		fseek(shaderFp, 0, SEEK_END);
		int fleng = ftell(shaderFp);
		fseek(shaderFp, 0, SEEK_SET);
		char *shader = new char [fleng+1];
		shader[fleng] = '\0';
		fread(shader, 1, fleng, shaderFp);

		cls->program = clCreateProgramWithSource(cls->context, 1, (const char**)&shader, NULL, &cle);
		ABORT_IF(cle != CL_SUCCESS || !cls->program, "Cannot create program\n");

		cle = clBuildProgram(cls->program, 0, NULL, NULL, NULL, NULL);
		if (cle != CL_SUCCESS) {
			size_t len;
			char buffer[2048];

			printf("Cannot build program\n");
			clGetProgramBuildInfo(cls->program, cls->device, CL_PROGRAM_BUILD_LOG, sizeof(buffer), buffer, &len);
			printf("%s\n", buffer);
			abort();
		}

		cls->gaussian = clCreateKernel(cls->program, "conv_and_trans2", &cle);
		ABORT_IF(cle != CL_SUCCESS || !cls->gaussian, "Cannot find \"conv\" OCL kernel\n");

		cls->diff = clCreateKernel(cls->program, "diff", &cle);
		ABORT_IF(cle != CL_SUCCESS || !cls->diff, "Cannot find \"diff\" OCL kernel\n");

		delete[] shader;
	}
	init_gaussian();
}

Sift::~Sift()
{
	delete cls;
	if (hasGaussian) {
		delete[] buffer;
		for (int i = 0; i < numOct; ++i) {
			delete[] blurred[i];
			delete[] dogs[i];
		}

		delete[] blurred;
		delete[] dogs;
	}
	if (hasGrads) {
		delete[] magAndThetas;
	}
}

void Sift::init_gaussian_mem()
{
	int wtmp = wmax;
	int htmp = hmax;
	buffer = new float[wtmp*htmp];
	blurred = new float*[numOct];
	dogs = new float*[numOct];
	for (int i = 0; i < numOct; ++i) {
		blurred[i] = new float[wtmp * htmp * (lvPerScale+3)];
		dogs[i] = new float[wtmp * htmp * (lvPerScale+2)];
		wtmp >>= 1;
		htmp >>= 1;
	}
}

void Sift::init_gaussian_first()
{
	if(octMin < 0) {
		upSample2(blurred[0], img, buffer, imgw, imgh);
		for (int o = 1; o < -octMin; ++o) {
			upSample2(blurred[0], blurred[0], buffer, imgw<<o, imgh<<o);
		}
	} else if(octMin > 0) {
		downSample(blurred[0], img, wmax, hmax, 1<<octMin);
	} else {
		memcpy(blurred[0], img, imgw*imgh*sizeof(float));
	}
}

void Sift::init_gaussian_build()
{
	int wtmp = wmax;
	int htmp = hmax;

	float sigmak = powf(2.0f, 1.0f/lvPerScale);
	float dsigma0 = sigma0 * sqrtf(1.0f - 1.0f/(sigmak*sigmak));
	float dsigmar = sigmak;
	cl_mem mem_img;
	cl_mem mem_buf;
	if (accel == Accel_OCL) {
		mem_img = clCreateBuffer(cls->context, CL_MEM_READ_WRITE, sizeof(float)*wmax*hmax, NULL, NULL);
		mem_buf = clCreateBuffer(cls->context, CL_MEM_READ_WRITE, sizeof(float)*wmax*hmax, NULL, NULL);
	}
	double total_time = 0.0f;
	for (int o = 0; o < numOct; ++o) {
		float sigma = dsigma0;
		int imsiz = wtmp*htmp;
		for(int s = 0 ; s <= lvPerScale+1; ++s) {
			double c1, c2;
			c1 = omp_get_wtime();
			switch (accel) {
			case Accel_None:
				gaussian_blur(
					blurred[o]+imsiz*(s+1), blurred[o]+imsiz*s,
					buffer, wtmp, htmp, sigma
				);
				break;
			case Accel_OMP:
				gaussian_blur_OMP(
					blurred[o]+imsiz*(s+1), blurred[o]+imsiz*s,
					buffer, wtmp, htmp, sigma
				);
				break;
			case Accel_OCL:
				cl_int cle;
				if (s == 0) {
					cle = clEnqueueWriteBuffer(cls->cqueue, mem_img, CL_TRUE, 0, sizeof(float)*wtmp*htmp, blurred[o], 0, NULL, NULL);
					ABORT_IF(cle != CL_SUCCESS, "Cannot write gaussian base image\n");
				}
				gaussian_blur_OCL(
					mem_img, mem_img, mem_buf,
					wtmp, htmp, sigma, cls
				);
				cle = clEnqueueReadBuffer(cls->cqueue, mem_img, CL_TRUE, 0, sizeof(float)*wtmp*htmp, blurred[o]+wtmp*htmp*(1+s), 0, NULL, NULL);
				ABORT_IF(cle != CL_SUCCESS, "Cannot read blurred result\n");
				break;
			}
			c2 = omp_get_wtime();
			printf("Calculate Gaussian blur (o=%d, s=%d): %lf (sec)\n", o, s, c2-c1);
			total_time += c2 - c1;
			sigma *= dsigmar;
		}

		if (o != numOct-1) {
			downSample(blurred[o+1], blurred[o]+imsiz*lvPerScale, wtmp, htmp, 2);
		}

		wtmp >>= 1;
		htmp >>= 1;
	}

	printf("Total time for Gaussian blur: %lf (sec)\n\n", total_time);

	if (accel == Accel_OCL) {
		clReleaseMemObject(mem_img);
		clReleaseMemObject(mem_buf);
	}
	if (dumpImage) {
		dump_gaussian_build();
	}
}

void Sift::init_gaussian_dog()
{
	int wtmp = wmax;
	int htmp = hmax;

	double total_time = 0.0f;
	for (int o = 0; o < numOct; ++o) {
		double c1, c2;
		c1 = omp_get_wtime();
		switch (accel) {
		case Accel_None:
			diff(dogs[o], blurred[o], lvPerScale+3, wtmp, htmp);
			break;
		case Accel_OMP:
			diff_OMP(dogs[o], blurred[o], lvPerScale+3, wtmp, htmp);
			break;
		case Accel_OCL:
			diff_OCL(dogs[o], blurred[o], lvPerScale+3, wtmp, htmp, cls);
			break;
		}
		wtmp >>= 1;
		htmp >>= 1;
		c2 = omp_get_wtime();
		printf("Calculate diff kernel (o=%d): %lf (sec)\n", o, c2-c1);
		total_time += c2 - c1;
	}
	printf("Total time for diff kernel %lf (sec)\n\n", total_time);

	if (dumpImage) {
		dump_gaussian_dog();
	}
}

void Sift::dump_gaussian_build()
{
	int wtmp = wmax;
	int htmp = hmax;
	unsigned char *wrbuf = new unsigned char[wmax*hmax];
	for (int o = 0; o < numOct; ++o) {
		int imgSiz = wtmp*htmp;
		for(int s = 0 ; s <= lvPerScale+2; ++s) {
			char buf[128];
			sprintf(buf, "g_%d_%d.pgm", o, s);
			FILE *fp = fopen(buf, "wb");
			float *based = blurred[o] + s*imgSiz;
			for (int i = 0; i < imgSiz; ++i) {
				wrbuf[i] = based[i]*256.0f;
			}
			save_P5_pgm(fp, wtmp, htmp, wrbuf);
			fclose(fp);
		}
		wtmp >>= 1;
		htmp >>= 1;
	}
	delete[] wrbuf;
}

void Sift::dump_gaussian_dog()
{
	int wtmp = wmax;
	int htmp = hmax;
	unsigned char *wrbuf = new unsigned char[wmax*hmax];
	for (int o = 0; o < numOct; ++o) {
		int imgSiz = wtmp*htmp;
		for(int s = 0 ; s < lvPerScale+2; ++s) {
			char buf[128];
			sprintf(buf, "d_%d_%d.pgm", o, s);
			FILE *fp = fopen(buf, "wb");
			float *based = dogs[o] + s*imgSiz;
			for (int i = 0; i < imgSiz; ++i) {
				wrbuf[i] = based[i]*256.0f + 128.0f;
			}
			save_P5_pgm(fp, wtmp, htmp, wrbuf);
			fclose(fp);
		}
		wtmp >>= 1;
		htmp >>= 1;
	}
	delete[] wrbuf;
}

void Sift::init_gaussian()
{
	hasGaussian = true;
	init_gaussian_mem();
	init_gaussian_first();
	init_gaussian_build();
	init_gaussian_dog();

}

void Sift::detect_raw_keypoints()
{
	int wtmp = wmax;
	int htmp = hmax;
	for (int o = 0; o < numOct; ++o) {
		// detect feature points (search for 26 neighboring points)
		int image_size = wtmp*htmp;
		int line_size = wtmp;
		for (int s = 1; s < lvPerScale+1; ++s) {
			for (int i = 1; i < (htmp - 1); ++i) {
				for (int j = 1; j < (wtmp - 1); ++j) {
					float *imgbase = dogs[o]+(s*wtmp*htmp);
#define CHECK_EXTREMA(OPER, THRES, PTR, IMAGE_SIZE, LINE_SIZE) ( \
	(*(PTR) OPER THRES)                               &&         \
	(*(PTR) OPER *(PTR - LINE_SIZE - 1))              &&         \
	(*(PTR) OPER *(PTR - LINE_SIZE))                  &&         \
	(*(PTR) OPER *(PTR - LINE_SIZE + 1))              &&         \
	(*(PTR) OPER *(PTR - 1))                          &&         \
	(*(PTR) OPER *(PTR + 1))                          &&         \
	(*(PTR) OPER *(PTR + LINE_SIZE - 1))              &&         \
	(*(PTR) OPER *(PTR + LINE_SIZE))                  &&         \
	(*(PTR) OPER *(PTR + LINE_SIZE + 1))              &&         \
	(*(PTR) OPER *(PTR - IMAGE_SIZE - LINE_SIZE - 1)) &&         \
	(*(PTR) OPER *(PTR - IMAGE_SIZE - LINE_SIZE))     &&         \
	(*(PTR) OPER *(PTR - IMAGE_SIZE - LINE_SIZE + 1)) &&         \
	(*(PTR) OPER *(PTR - IMAGE_SIZE - 1))             &&         \
	(*(PTR) OPER *(PTR - IMAGE_SIZE + 1))             &&         \
	(*(PTR) OPER *(PTR - IMAGE_SIZE + LINE_SIZE - 1)) &&         \
	(*(PTR) OPER *(PTR - IMAGE_SIZE + LINE_SIZE))     &&         \
	(*(PTR) OPER *(PTR - IMAGE_SIZE + LINE_SIZE + 1)) &&         \
	(*(PTR) OPER *(PTR + IMAGE_SIZE - LINE_SIZE - 1)) &&         \
	(*(PTR) OPER *(PTR + IMAGE_SIZE - LINE_SIZE))     &&         \
	(*(PTR) OPER *(PTR + IMAGE_SIZE - LINE_SIZE + 1)) &&         \
	(*(PTR) OPER *(PTR + IMAGE_SIZE - 1))             &&         \
	(*(PTR) OPER *(PTR + IMAGE_SIZE + 1))             &&         \
	(*(PTR) OPER *(PTR + IMAGE_SIZE + LINE_SIZE - 1)) &&         \
	(*(PTR) OPER *(PTR + IMAGE_SIZE + LINE_SIZE))     &&         \
	(*(PTR) OPER *(PTR + IMAGE_SIZE + LINE_SIZE + 1)))

					float *ptr = imgbase + i*wtmp + j;
					if (CHECK_EXTREMA(>,  mth, ptr, image_size, line_size) ||
						CHECK_EXTREMA(<, -mth, ptr, image_size, line_size)) {
						Keypoint key;
						key.o = o;
						key.ix = j;
						key.iy = i;
						key.is = s-1;
						kps.push_back(key);
					}
				}
			}
		}
		wtmp >>= 1;
		htmp >>= 1;
	}
	printf("%lu raw kps\n", kps.size());
}

void Sift::refine_keypoints()
{
	int o, s, w, h, x, y;
	float* point; // position of the key point
	float delta[3]; // displacement of point
	float gradient[3]; // 1st-order derivative
	float hessian[9]; // 2nd-order derivative
	float hessianInv[9]; // inverse of hessian

	int dst = 0;
	for (size_t i = 0; i < kps.size(); ++i) {
		o = kps[i].o;
		x = kps[i].ix;
		y = kps[i].iy;
		s = kps[i].is;
		w = wmax >> o;
		h = hmax >> o;
		point = dogs[o] + (s+1)*w*h + (y*w+x);

		build_gradient(gradient, point, w, h);
		build_hessian(hessian, point, w, h);
		inv_3d_matrix(hessianInv, hessian);

		// delta = inv((d/dr)^2 D) * (dD/dr)
		// r' = r - delta
		matrix_multiply(delta, hessianInv, gradient);

		float period = powf(2.0f, o+octMin);
		kps[i].x = (x - delta[0]) * period;
		kps[i].y = (y - delta[1]) * period;
		float s_unscale = s - delta[2];
		kps[i].sigma = 1.6 * powf(2.0f, o + (s_unscale / lvPerScale));
		if (kps[i].sigma < 1) {
			kps[i].sigma = 1;
		}


		// reject edge response, [ dxx dxy ]
		//                       [ dyx dyy ], 'edge' if (trace^2/det) > (th+1)^2/th
		float trace = hessian[0] + hessian[4];
		float det = hessian[0] * hessian[4] - hessian[1] * hessian[1];
		if ((trace*trace/det)<((eth+1)*(eth+1)/eth)) {
			kps[dst] = kps[i];
			++dst;
		}
	}
	kps.resize(dst);
	printf("%lu kps after refine\n", kps.size());
}

const vector<Keypoint> &Sift::extract_keypoints(float _mth, float _eth)
{
	mth = _mth;
	eth = _eth;
	detect_raw_keypoints();
	refine_keypoints();
	return kps;
}

void Sift::init_gradient_mem()
{
	int wtmp = wmax;
	int htmp = hmax;
	magAndThetas = new float*[numOct];
	for (int i = 0; i < numOct; ++i) {
		magAndThetas[i] = new float[2*wtmp*htmp*lvPerScale];
		wtmp >>= 1;
		htmp >>= 1;
	}
}

void Sift::init_gradient_build()
{
	int wtmp = wmax;
	int htmp = hmax;
	for (int o = 0; o < numOct; ++o) {
		int imgsiz = wtmp*htmp;
		build_gradient_map(magAndThetas[o], blurred[o]+imgsiz, lvPerScale, wtmp, htmp);
		wtmp >>= 1;
		htmp >>= 1;
	}
}

void Sift::init_gradient()
{
	hasGrads = true;
	init_gradient_mem();
	init_gradient_build();
}

void Sift::calc_kp_angle(Keypoint &kp)
{
	if (!hasGrads) {
		init_gradient();
	}

	int o = kp.o;
	int s = kp.is;
	float period = powf(2.0f, o);
	float sigmaW = 1.5f * (kp.sigma / period);
	int windowSize = 3 * sigmaW;
	if (windowSize <= 0) {
		windowSize = 1;
	}
	int w = wmax>>o;
	int h = hmax>>o;
	const float* gradImage = magAndThetas[o] + 2*s*w*h;
	float floatX = kp.x / period;
	float floatY = kp.y / period;
	int intX = (int)(floatX + 0.5);
	int intY = (int)(floatY + 0.5);

	if (intX < 1 || intX >= w || intY < 1 || intY >= h) {
		printf("??\n");
		return;
	}

	// compute histogram
	const int histSize = 36;
	float hist[histSize];
	memset(hist, 0, histSize * sizeof(float));
	for (int j=MAX(1-intY, -windowSize); j < MIN(h-1-intY, windowSize+1); ++j) {
		for (int i=MAX(1-intX, -windowSize); i < MIN(w-1-intX, windowSize+1); ++i) {
			float dx = i + intX - floatX;
			float dy = j + intY - floatY;
			float r2 = dx*dx + dy*dy;
			if (r2 >= (windowSize * windowSize) + 0.5) { // only compute within circle
				continue;
			}
			float weight = expf(-r2 / (2*sigmaW*sigmaW));
			float magnitude = gradImage[((intX + i) + (intY + j) * w) * 2];
			float angle = gradImage[((intX + i) + (intY + j) * w) * 2 + 1];
			int binIndex = (int)(angle / (2 * M_PI) * histSize) % histSize;
			hist[binIndex] += magnitude * weight;
		}
	}

	// box filter smoothing
	for (int i = 0; i < 5; ++i) {
		float prevCache;
		float currCache = hist[histSize - 1];
		float first = hist[0];
		int j;
		for (j = 0; j < (histSize - 1); ++j) {
			prevCache = currCache;
			currCache = hist[j];
			hist[j] = (prevCache + hist[j] + hist[j+1]) / 3.0f;
		}
		hist[j] = (currCache + hist[j] + first) / 3.0f;
	}

	// find histogram maximum
	int maxBinIndex;
	float maxBallot = -FLT_MAX;
	for (int i = 0; i < histSize; ++i) {
		if (hist[i] > maxBallot) {
			maxBallot = hist[i];
			maxBinIndex = i;
		}
	}

	// quadratic interpolation
	float self = hist[maxBinIndex];
	float left = (maxBinIndex == 0)? hist[histSize-1] : hist[maxBinIndex-1];
	float right = (maxBinIndex == histSize-1)? hist[0] : hist[maxBinIndex+1];
	float dx = 0.5 * (right - left);
	float dxx = (right + left - (2 * self));
	kp.orient = (maxBinIndex + 0.5 - (dx / dxx)) * (2 * M_PI / histSize);
#undef histSize
}

void Sift::calc_kp_angles(Keypoint *kps, int n)
{
	double c1, c2;
	c1 = omp_get_wtime();
	switch (accel) {
	case Accel_None:
	case Accel_OCL:
		for (int i = 0; i < n; ++i) {
			calc_kp_angle(*kps);
			++kps;
		}
		break;
	case Accel_OMP:
		if (!hasGrads) {
			init_gradient();
		}
#pragma omp parallel for schedule(dynamic, 32)
		for (int i = 0; i < n; ++i) {
			calc_kp_angle(kps[i]);
		}
		break;
	}
	c2 = omp_get_wtime();
	printf("Calculate angles %lf\n", c2-c1);
}

void Descriptor::normalize()
{
	float sqsum = 0.0f;
	for (int i = 0; i < 128; ++i) {
		sqsum += v[i] * v[i];
	}
	float inv = 1.0f/sqrt(sqsum);
	for (int i = 0; i < 128; ++i) {
		v[i] *= inv;
	}
}

void Descriptor::regularize()
{
	normalize();
	for (int i = 0; i < 128; ++i) {
		if (v[i] > 0.2f) {
			v[i] = 0.2f;
		}
	}
	normalize();
}

void Sift::calc_kp_descriptor(const Keypoint &kp, Descriptor &des)
{
	if (!hasGrads) {
		init_gradient();
	}
	int o = kp.o;
	int s = kp.is;
	float period = powf(2.0f, o);
	int w = wmax >> o;
	int h = hmax >> o;
	float* gradImage = magAndThetas[o] + s*w*h*2;
	float sigma = kp.sigma / period;
	float floatX = kp.x / period;
	float floatY = kp.y / period;
	int intX = (int)(floatX + 0.5);
	int intY = (int)(floatY + 0.5);

	if (intX < 1 || intX >= h || intY < 1 || intY >= w) {
		return;
	}

	// 8 orientations x 4 x 4 histogram array = 128 dimensions
	const int NBO = 8; // number of orientations
	const int NBP = 4; // number of small blocks
	const float SBP = (3.0f * sigma); // size of a small block
	const int W = (int)floor((SBP * (NBP+1) * sqrtf(0.5)) + 0.5); // W=SBP(NBP+1)/sqrt(2)

	float angle0 = kp.orient;

	const float st0 = sinf(angle0);
	const float ct0 = cosf(angle0);

	const int binto = 1;
	const int binyo = NBO * NBP;
	const int binxo = NBO;

	memset(des.v, 0, NBO*NBP*NBP*sizeof(float));

	/* Center the scale space and the descriptor on the current keypoint. 
	 * Note that dpt is pointing to the bin of center (SBP/2,SBP/2,0).
	 */
	float const * pt = gradImage + (intX + intY * w) * 2;
	float*       dpt = des.v + (NBP/2) * binyo + (NBP/2) * binxo ;

#define atd(dbinx,dbiny,dbint) *(dpt + (dbint)*binto + (dbiny)*binyo + (dbinx)*binxo)


	for (int i = MAX(-W, 1-intY); i < MIN(W+1, h-1-intY); ++i) {
		for (int j = MAX(-W, 1-intX); j < MIN(W+1, w-1-intX); ++j) {
			// start copying siftpp() ... so 'dy' and 'dx' conform to its convention
			float dx = j + intX - floatX;
			float dy = i + intY - floatY;

			// for the sample point
			float mod = gradImage[((intY + i) * w + (intX + j)) * 2];
			float angle = gradImage[((intY + i)*w + (intX + j)) * 2+1];
			float theta = (-angle + angle0);
			if (theta < 0) {
				theta += 2 * M_PI;
			}

			// get the displacement normalized w.r.t. the keypoint
			// orientation and extension.
			float nx = ( ct0 * dx + st0 * dy) / SBP ;
			float ny = (-st0 * dx + ct0 * dy) / SBP ; 
			float nt = NBO * theta / (2*M_PI) ;

			// Get the gaussian weight of the sample. The gaussian window
			// has a standard deviation equal to NBP/2. Note that dx and dy
			// are in the normalized frame, so that -NBP/2 <= dx <= NBP/2.
			float const wsigma = NBP/2 ;
			float win = expf(-(nx*nx + ny*ny)/(2.0 * wsigma * wsigma)) ;

			// The sample will be distributed in 8 adjacent bins.
			// We start from the ``lower-left'' bin.
			int binx = floorf( nx - 0.5 ) ;
			int biny = floorf( ny - 0.5 ) ;
			int bint = floorf( nt ) ;

			//rbinx net result ==> (nx-0.5)-floor(nx-0.5)
			//rbiny net result ==> (ny-0.5)-floor(ny-0.5)
			float rbinx = nx - (binx+0.5) ;
			float rbiny = ny - (biny+0.5) ;
			float rbint = nt - bint ;
			int dbinx ;
			int dbiny ;
			int dbint ;

			// Distribute the current sample into the 8 adjacent bins
			for(dbinx = 0 ; dbinx < 2 ; ++dbinx) {
				for(dbiny = 0 ; dbiny < 2 ; ++dbiny) {
					for(dbint = 0 ; dbint < 2 ; ++dbint) {

						if( binx+dbinx >= -(NBP/2) &&
							binx+dbinx <   (NBP/2) &&
							biny+dbiny >= -(NBP/2) &&
							biny+dbiny <   (NBP/2) ) {
							float weight = win 
								* mod 
								* fabsf (1 - dbinx - rbinx)
								* fabsf (1 - dbiny - rbiny)
								* fabsf (1 - dbint - rbint) ;

							atd(binx+dbinx, biny+dbiny, (bint+dbint) % NBO) += weight ;
						}
					}
				}
			}
		}
	}
	/* End of copying siftpp */

	des.regularize();
}

void Sift::calc_kp_descriptors(const Keypoint *kps, int n, Descriptor *dess)
{
	double c1, c2;
	c1 = omp_get_wtime();
	switch (accel) {
	case Accel_None:
		for (int i = 0; i < n; ++i) {
			calc_kp_descriptor(*kps, *dess);
			++kps;
			++dess;
		}
		break;
	case Accel_OMP:
	case Accel_OCL:
#pragma omp parallel for schedule(dynamic, 64)
		for (int i = 0; i < n; ++i) {
			calc_kp_descriptor(kps[i], dess[i]);
		}
	}
	c2 = omp_get_wtime();
	printf("Calculate descriptors %lf\n", c2-c1);
}

