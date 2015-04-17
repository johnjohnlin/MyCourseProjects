#include "panorama.h"
#include "geometry.h"
#include "kdtree.h"
#include "siftpp/sift.hpp"
#include "pgm.h"

#include <cstdio>
#include <cassert>
#include <cstdlib>
#include <cstring>

class VarCal {
public:
	VarCal()
	{
		reset();
	}
	void reset() {
		sqSum = sum = n = 0;
	}
	void add(float x)
	{
		sum += x;
		sqSum += x * x;
		++n;
	}
	float get_var()
	{
		float avg = sum/n;
		float sqAvg = sqSum/n;
		return sqAvg - avg * avg;
	}
private:
	float sum, sqSum;
	int n;
};

void Panorama::open_images()
{
	FILE *log = fopen("imageID.txt", "w");
	for (int i = 0; i < imageNames.size(); ++i) {
		fprintf(log, "%d %s\n", i, imageNames[i].c_str());
		FILE *imgFp = fopen(imageNames[i].c_str(), "r");
		if (imgFp == NULL) {
			fprintf(stderr, "cannot open image %s", imageNames[i].c_str());
			assert(imgFp);
		}
		uint8_t *imgData;
		ImageInfo iif;
		load_P5_pgm(imgFp, &(iif.w), &(iif.h), &imgData);
		images.push_back(imgData);
		imageInfos.push_back(iif);
		fclose(imgFp);
	}
}

void Panorama::calc_features()
{
	const int S = 3;
	const int omin = 0;
	const float sigman = .5f ;
	const float sigma0 = 1.6f * powf(2.0f, 1.0f / S) ;
	const float threshold = 0.03f;
	const float edgeThreshold = 10.0f;
	const float magnif = 3.0f;

	FILE *featuresFp = fopen("Features.txt", "w");

	for (int i = 0; i < imageNames.size(); ++i) {
		printf("Find features in image[%d]...\n", i);
		int img_wxh = imageInfos[i].w * imageInfos[i].h;
		float w2 = imageInfos[i].w/2.0, h2 = imageInfos[i].h/2.0;
		float *data = new float[img_wxh];
		for (int j = 0; j < img_wxh; ++j) {
			data[j] = ((float)images[i][j])/(255.0f);
		}
		int kpNum = 0;
		static int kpSum = 0;
		VL::Sift sift(data, imageInfos[i].w, imageInfos[i].h, sigman, sigma0, 3, S, omin, -1, S+1);

		sift.detectKeypoints(threshold, edgeThreshold);

		sift.setNormalizeDescriptor(true);
		sift.setMagnification(magnif);

		for(VL::Sift::KeypointsConstIter iter = sift.keypointsBegin();
			iter != sift.keypointsEnd() ; ++iter) {

			// detect orientations
			VL::float_t angles[4] ;
			int nangles;
			nangles = sift.computeKeypointOrientations(angles, *iter) ;

			// compute descriptors
			for(int a = 0 ; a < nangles ; ++a) {
				float fixX = iter->x - w2;
				float fixY = iter->y - h2;

				fprintf(featuresFp, "%d %2f %2f %2f %2f", i, fixX, fixY, iter->sigma, angles[0]);

				/* compute descriptor */
				VL::float_t descr_pt[128];
				sift.computeKeypointDescriptor(descr_pt, *iter, angles[0]);

				/* save descriptor to to appropriate file */
				sift_desc_push(featuresFp, i, fixX, fixY, descr_pt);

				fputs("\n", featuresFp);

				++kpNum;
			} // next angle
		} // next keypoint
		kpSum += kpNum;
		// numFeatures[i] = kpNum;
		// numFeaturesOffset[i] = kpSum;
		printf("%d features are extracted from image %d\n", kpNum, i);
		delete[] data;
	} // next image

	printf("totally %u features are extracted\n", (unsigned)features.size());

	fclose(featuresFp);
}

static bool compare_match(MatchPair m1, MatchPair m2)
{
	return m1.f2.iid < m2.f2.iid;
}

inline void print_matchs(FILE *fp, vector<MatchPair> &matchs)
{
	fprintf(fp, "%u\n", (unsigned)matchs.size());
	for (int i = 0; i < matchs.size(); ++i) {
		fprintf(fp, "%d %f %f %d %f %f\n",
			matchs[i].f1.iid, matchs[i].f1.x, matchs[i].f1.y,
			matchs[i].f2.iid, matchs[i].f2.x, matchs[i].f2.y);
	}
}

inline void print_imatchs(FILE *fp, vector<ImageMatchInfo> &imi)
{
	fprintf(fp, "%u\n", (unsigned)imi.size());
	for (int i = 0; i < imi.size(); ++i) {
		fprintf(fp, "%d %d %d\n", imi[i].iid1, imi[i].iid2, imi[i].matchNum);
	}
}

void Panorama::create_matchs()
{
	KDTree kd;
	Node *nodes = new Node[features.size()];
	kd.KDTree_build(nodes, &(featureData[0]), features.size(), 128);

	int *numMatchOf_i = new int[images.size()]();

#define _K 4
	int max_match = min(_K, (int)images.size()-1);
	for (int i = 0; i < features.size(); ++i) {
		int *target = &featureData[128*i];
		int indice[_K];
		kd.find_kNN(indice, target, max_match, i);
		for (int j = 0; j < max_match; ++j) {
			int iMatch = indice[j];
			Feature f1 = features[i];
			Feature f2 = features[iMatch];
			if (f1.iid != f2.iid) {
				MatchPair match = {f1, f2};
				matchs.push_back(match);
				++(numMatchOf_i[f1.iid]);
			}
		}
#undef _K
	}

	vector<MatchPair>::iterator op = matchs.begin();
	vector<MatchPair>::iterator ed = matchs.begin();
	for (int i = 0; i < imageNames.size(); ++i) {
		op = ed;
		ed += numMatchOf_i[i];
		sort(op, ed, compare_match);
	}
	create_match_info();

	FILE *fp = fopen("matchs_0.txt", "w");
	print_matchs(fp, matchs);
	fclose(fp);

	fp = fopen("imatchs_0.txt", "w");
	print_imatchs(fp, imageMatchInfos);
	fclose(fp);
	delete[] numMatchOf_i;

	// featureData is not needed anymore, it is very large, so free its memory
	// Note: clear() will not free the memory
	vector<int> emptyVec(0);
	emptyVec.swap(featureData);
	delete[] nodes;
}

void Panorama::create_match_info()
{
	// TODO: make this function more readable
	int currI = matchs[0].f1.iid, currJ = matchs[0].f2.iid;
	int numMatchs = 0;

	// create match info between images
	for (int i = 0; i < matchs.size(); ++i) {

		if (currJ != matchs[i].f2.iid || currI != matchs[i].f1.iid) {
			ImageMatchInfo imi = {currI, currJ, numMatchs};
			imageMatchInfos.push_back(imi);

			currI = matchs[i].f1.iid;
			currJ = matchs[i].f2.iid;
			numMatchs = 0;
		}

		++numMatchs;
	}

	// the last image match...
	ImageMatchInfo imi = {currI, currJ, numMatchs};
	imageMatchInfos.push_back(imi);
}

void Panorama::remove_too_few_match()
{
#define _M 6
	int numMatch_ij[_M];
	int maxMatch_j[_M];
	int i = 0, i_prev;

	// find out max match between images
	int currI = -1, currJ = -1;
	i_prev = i = 0;
	currI = 0;
	while (true) {
		// done
		if (i == imageMatchInfos.size()) {
			break;
		}

		if (imageMatchInfos[i].iid1 != currI) {
			for (int j = i_prev; j < i; ++j) {
				if (maxMatch_j + _M == find(maxMatch_j, maxMatch_j + _M, imageMatchInfos[j].iid2)) {
					imageMatchInfos[i].matchNum = -imageMatchInfos[i].matchNum;
				}
			}
			fill(numMatch_ij, numMatch_ij + _M, -1);
			fill(maxMatch_j, maxMatch_j + _M, -1);
			currI = imageMatchInfos[i].iid1;
			i_prev = i;
			continue;
		}

		// this part is somehow similar to some part of KDTree::consider_node
		// try to write this part in a function
		for (int j = 0; j < _M; ++j) {
			if (imageMatchInfos[i].matchNum > numMatch_ij[j]) {
				for (int k = _M-1; k > j; --k) {
					numMatch_ij[k] = numMatch_ij[k-1];
					maxMatch_j[k] = maxMatch_j[k-1];
				}
				numMatch_ij[j] = imageMatchInfos[i].matchNum;
				maxMatch_j[j] = imageMatchInfos[i].iid2;
				break;
			}
		}
		++i;
	}

	// actually remove the unwanted matchs
	remove_outliers_routine();

	// update imageMatchInfos
	int readID, writeID;
	readID = writeID = 0;
	while (readID < imageMatchInfos.size()) {
		if (imageMatchInfos[readID].matchNum < 0) {
			++readID;
		} else {
			if (readID != writeID) {
				imageMatchInfos[writeID] = imageMatchInfos[readID];
			}
			++readID;
			++writeID;
		}
	}
	imageMatchInfos.resize(writeID);

	FILE *fp = fopen("matchs_1.txt", "w");
	print_matchs(fp, matchs);
	fclose(fp);

	fp = fopen("imatchs_1.txt", "w");
	print_imatchs(fp, imageMatchInfos);
	fclose(fp);
#undef _M
}

/** @brief remove the ransac outliers
 *
 *  This is done after all of the the homography were been calculated
 */
void Panorama::remove_ransac_outliers()
{
	int op = 0, ed = 0;
	int readID = 0, writeID = 0;
	for (int i = 0; i < imageMatchInfos.size(); ++i) {
		op = ed;
		ed += imageMatchInfos[i].matchNum;
		for (int j = op; j < ed; ++j) {
			float dist = Geometry::proj_dist(matchs[j], imageMatchInfos[i].H);
			if (dist > 25.0) {
				++readID;
			} else {
				if (readID != writeID) {
					matchs[writeID] = matchs[readID];
				}
				++readID;
				++writeID;
			}
		}
		imageMatchInfos[i].matchNum = imageMatchInfos[i].numInliers;
	}
	matchs.resize(writeID);
}

/** @brief generate random, mutually differernt integers
 *
 *  This function isn't suitable for large size
 *
 *  @param[out] dst the result
 *  @param[in] num of ints desired
 *  @param[in] max ints will in <c>[0,max)</c>
 */
static inline void gen_randints(int *dst, int num, int max)
{
	assert(max >= num);
	for (int i = 0; i < num; ++i) {
		int tmp;
		int j;
		do {
_rand_again:
			tmp = rand() % max;
			for (j = 0; j < i; ++j) {
				if (tmp == dst[j]) {
					goto _rand_again;
				}
			}
			break;
		} while (true);
		dst[i] = tmp;
	}
}

/** @brief calculate the best homography using ransac between image (i, j)
 *
 *  This function isn't suitable for large size
 *
 *  @param[in] op imply the index of the start of matchs between (i, j)
 *  @param[in] ed imply the index of the end of matchs between (i, j)
 *  @param[in] id the ID of imageMatchInfos
 */
void Panorama::ransac_proc_match_ij(int op, int ed, int id)
{
#define _R 4
#define _N 500
	int randints[_R];
	ImageMatchInfo &imi = imageMatchInfos[id];
	imi.numInliers = -1;
	float hinti = (float)((imageInfos[imi.iid1].w + imageInfos[imi.iid1].h + 1)>>1);
	float hintj = (float)((imageInfos[imi.iid2].w + imageInfos[imi.iid2].h + 1)>>1);
	for (int i = 0; i < _N; ++i) {
		gen_randints(randints, _R, ed - op);
		for (int j = 0; j < _R; ++j) {
			randints[j] += op;
		}
		float H[9];
		int count = 0;
		int ret = Geometry::solve_H_from_4matchs(H,
			matchs[randints[0]], matchs[randints[1]],
			matchs[randints[2]], matchs[randints[3]],
			hinti, hintj);
		for (int j = op; j < ed; ++j) {
			float dist = Geometry::proj_dist(matchs[j], H);
			if (dist < 25.0) {
				++count;
			}
		}
		if (count > imi.numInliers) {
			imi.numInliers = count;
			memcpy(imi.H, H, 9*sizeof(float));
		}
	}
#undef _R
#undef _N
}

/** @brief process the ransac stage
 */
void Panorama::ransac()
{
	int op = 0, ed = 0;
	for (int i = 0; i < imageMatchInfos.size(); ++i) {
		op = ed;
		ed += imageMatchInfos[i].matchNum;
		ransac_proc_match_ij(op, ed, i);
		// printf("%d\n", imageMatchInfos[i].numInliers);
	}
	remove_ransac_outliers();
	FILE *fp = fopen("matchs_2.txt", "w");
	print_matchs(fp, matchs);
	fclose(fp);
	fp = fopen("imatchs_2.txt", "w");
	print_imatchs(fp, imageMatchInfos);
	fclose(fp);
}

void Panorama::fit_prob_model()
{
	return;
	/*
	int op = 0, ed = 0;
	for (int i = 0; i < imageMatchInfos.size(); ++i) {
		int ni = imageMatchInfos[i].matchNum;
		int nf;
		ed = op;
		op += ni;
		if () {
		}
	}
	remove_outliers_routine();
	FILE *fp = fopen("matchs_3.txt", "w");
	print_matchs(fp, matchs);
	fclose(fp);
	fp = fopen("imatchs_3.txt", "w");
	print_imatchs(fp, imageMatchInfos);
	fclose(fp);
	*/
}

void Panorama::remove_outliers()
{
	remove_too_few_match();
	ransac();
	fit_prob_model();
}

void Panorama::init_BA()
{
	int N = imageMatchInfos.size();
	// calculating average F
	{
		float fSum = 0.0;
		float *f = pCur + 3;
		for (int i = 0; i < numImgAdded; ++i) {
			fSum += *f;
			f += 4;
		}
		avgF = fSum / numImgAdded;
	}
	// prepare R
	{
		float *param = pCur;
		for (int i = 0; i < numImgAdded; ++i) {
			Geometry::cal_R_from_param(RBuf + rMap[i]*9, param);
			param += 4;
		}
	}
}

void Panorama::BA()
{
#define _KMAX 100
	const float ep1 = 1.0E-15, tow = 1.0E-3;
	float mu = 0, rho, nu = 2.0;
	float errValue, errValueNew;
	/* A = J'*J; epNew = x-f(p); g = J'*ep; */
	cal_JTJ();
	cal_fp(pCur);
	x_minus_fp();
	errValueNew = err_func();
	cal_JTr();
	/* stop if gradient is small */
	if (*max_element(g, g+(numImgAdded<<2)) < ep1) {
		return;
	}
	/* mu = max(A_ii) * tow; */
	{
		float *p = A;
		for (int i = 0; i < (numImgAdded<<2); i++){
			if(*p > mu) {
				mu = *p;
			}
			p += (numImgAdded<<2) + 1;
		}
		mu *= tow;
	}
	for (int i = 0; i < _KMAX; ++i) {
		/* repeat */
		do {
			/* solve (A+uC)*dp = g */
			cal_A_plus_uC();
			// TODO: INVERSE();
			/* stop if delta is small */
			if (norm2(dp) <= ep1 * norm2(pCur)) {
				return;
			}
			/* p_new = p + dp */
			p_add_dp();
			/* ep_new = x - f(p_new) */
			cal_fp(pNew);
			x_minus_fp();
			/* rho = blahblah */
			rho = (errValue - errValueNew);
			if (rho > 0.0) {
				/* p = p_new */
				std::swap(pCur, pNew);
				/* A = J'*J */
				cal_JTJ();
				/* ep = ep_new */
				std::swap(ep, epNew);
				/* g = J' * ep*/
				cal_JTr();
				/* stop if gradient is small */
				if (*max_element(g, g+(numImgAdded<<2)) < ep1) {
					return;
				}
				// we abate the error, so next iteration now
				nu = 2;
				mu *= 0.5;
				break;
			} else {
				mu *= nu;
				nu *= 2;
			}
		} while (true);
	}
LM_Done:
	;
#undef _KMAX
}

/** @brief calculate how should images be added to BA
 *
 *  @ret image ID with max matchs
 */
int Panorama::cal_BA_add_order(float &fj)
{
	int ret;
	fill(imgIsAdd, imgIsAdd + imageNames.size(), false);
	// first image
	{
		int maxMatchID;
		int maxMatchNum = -1;
		for (int i = 0; i < imageMatchInfos.size(); ++i) {
			ImageMatchInfo &imi = imageMatchInfos[i];
			if (imi.matchNum > maxMatchNum) {
				maxMatchID = i;
				maxMatchNum = imi.matchNum;
			}
		}
		int iid1 = imageMatchInfos[maxMatchID].iid1;
		float w = imageInfos[iid1].w;
		float h = imageInfos[iid1].h;
		// Geometry::solve_fj_from_H(fj, imageMatchInfos[maxMatchID].H, (w+h) * 0.5);
		fj = (w+h) * 0.5;
		imgIsAdd[iid1] = true;
		rMap[iid1] = 0;
		ret = iid1;
	}
	// the other images
	for (int i = 0; i < imageNames.size()-1; ++i) {
		int maxMatchNum = -1;
		int maxMatchID;
		for (int j = 0; j < imageMatchInfos.size(); ++j) {
			ImageMatchInfo &imi = imageMatchInfos[j];
			bool isPartlyAdd = imgIsAdd[imi.iid1] ^ imgIsAdd[imi.iid2];
			if (!isPartlyAdd) {
				continue;
			}
			if (imi.matchNum > maxMatchNum) {
				maxMatchNum = imi.matchNum;
				maxMatchID = j;
			}
		}
		if (maxMatchNum == -1) {
			printf("Something error: Some images are disjointed?\n");
			exit(1);
		}

		int add;
		int append;
		int iid1 = imageMatchInfos[maxMatchID].iid1;
		int iid2 = imageMatchInfos[maxMatchID].iid2;
		if (imgIsAdd[iid1]) {
			add = iid2;
			append = iid1;
		} else {
			add = iid1;
			append = iid2;
		}
		addOrder[i] = add;
		appendOrder[i] = append;
		imgIsAdd[add] = true;
		rMap[iid1] = i + 1;
	}
	fill(imgIsAdd, imgIsAdd + imageNames.size(), false);
	return ret;
}

void Panorama::calc_camera_params()
{
	// prepare memory
	prepare_BA_memory();
	// init first image
	int firstImage = cal_BA_add_order(avgF);

	for (int i = 0; i < imageNames.size()-1; ++i) {
		printf("%3d %3d\n", addOrder[i], appendOrder[i]);
	}
	printf("rMap:");
	for (int i = 0; i < imageNames.size(); ++i) {
		printf(" %d", rMap[i]);
	}
	printf("\n");

	std::fill(pCur, pCur + 3, 0);
	pCur[4] = avgF;
	numImgAdded = 1;
	// BA loop
	for (int i = 0; i < imageNames.size()-1; ++i) {
		// add image with most match
		int toAdd = addOrder[i];
		imgIsAdd[toAdd] = true;
		init_BA();
		// camParam[i] = ...
		// BA();
	}
	free_BA_memory();
}

void Panorama::calc_gains()
{
}

void Panorama::blend_images()
{
}

void Panorama::run()
{
	open_images();
	calc_features();
	create_matchs();
	remove_outliers();
	calc_camera_params();
	calc_gains();
	blend_images();
}

void Panorama::x_minus_fp()
{
	float *epNewPtr = epNew;
	float *fpPtr = fp;
	for (int i = 0; i < matchs.size(); ++i) {
		if (imgIsAdd[matchs[i].f1.iid] && imgIsAdd[matchs[i].f2.iid]) {
			*(epNewPtr  ) = matchs[i].f2.x - *(fpPtr  );
			*(epNewPtr+1) = matchs[i].f2.y - *(fpPtr+1);
		}
		fpPtr += 2;
		epNewPtr += 2;
	}
}

float Panorama::norm2(float *ptr)
{
	float sum = 0;
	for (int i = 0; i < (numImgAdded<<2); ++i) {
		sum += (*ptr) * (*ptr);
		++ptr;
	}
}

float Panorama::err_func()
{
	const float sigma = 2, sigma2 = sigma * sigma;
	float sum = 0;
	float *epNewPtr = epNew;
	for (int i = 0; i < matchs.size(); ++i) {
		if (imgIsAdd[matchs[i].f1.iid] && imgIsAdd[matchs[i].f2.iid]) {
			float dx = *(epNewPtr  );
			float dy = *(epNewPtr+1);
			float r2 = dx*dx + dy*dy;
			if (r2 > sigma2) {
				sum += 2 * sigma * sqrt(r2) - sigma2;
			} else {
				sum += r2;
			}
		}
		epNewPtr += 2;
	}
}

float Panorama::dp_mul_udp_plus_g(float mu)
{
	float sum = 0;
	float *gPtr = g;
	float *dPtr = dp;
	for (int i = 0; i < (numImgAdded<<2); ++i) {
		sum += (*dPtr) * ((*dPtr) + (*gPtr) * mu);
		++dPtr;
		++gPtr;
	}
}

void A_plus_uC()
{
}

void cal_JTJ()
{
}

void cal_fp(float *p)
{
}

void x_minus_fp()
{
}

void cal_JTr()
{
}
