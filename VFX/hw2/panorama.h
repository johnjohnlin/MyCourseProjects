#ifndef __PANORAMA_H_
#define __PANORAMA_H_
#include <vector>
#include <string>
#include <stdint.h>
#include <cstdio>

using namespace std;

struct ImageInfo {
	int w, h;
};

struct Feature {
	/** id of images */
	int iid;
	//@{
	/** coordinates of features in its image */
	float x, y;
	//@}
};

struct MatchPair {
	//@{
	/** H * f1 ~ f2 if perfectly matched */
	Feature f1, f2;
	//@}
};

struct ImageMatchInfo {
	int iid1, iid2;
	int matchNum;
	int numInliers;
	float H[9];
};

class Panorama {
public:
	Panorama():saveMedium(true) {};
	void add_file(const string& s) {
		imageNames.push_back(s);
	}
	void run();
private:
	vector<string> imageNames; ///< name of images
	vector<MatchPair> matchs; ///< vector of feature matchs
	vector<uint8_t*> images; ///< vector of pointer to images
	vector<Feature> features; ///< vector of features
	vector<ImageInfo> imageInfos; ///< width and height of images
	vector<ImageMatchInfo> imageMatchInfos;
	vector<int> featureData; ///< the 128 dimensions of features, size is <c>128*features.size()</c>
	int *numFeatures; ///< #feature of each image
	int *numFeaturesOffset; ///< <c>numFeaturesOffset[i] = sum_{j = 0..(i+1)} numFeatures[j]</c>
	uint16_t *imageOut; ///< the output panorama image
	bool saveMedium;
	// @{
	/// variables related to bundle adjustment
	float *A, *A_plus_uC; ///< (16 * #img * #img) float
	float *g, *dp, *pCur, *pNew; ///< (4 * #img) float
	float *ep, *epNew, *fp; ///< (2 * #match) float
	float *RBuf; ///< (2 * #match) float
	bool *imgIsAdd; ///< describe whether image is added to BA
	int *addOrder, *appendOrder, *rMap; ///< describe the order that image is added to BA
	float *floatBuf; ///< The actual buffer
	int *intBuf; ///< The actual buffer
	float avgF;
	int numImgAdded;
	float rMat[9], lMat[9];
	// }@

	// toplevel processing
	void open_images();
	void calc_features();
	void create_matchs();
	void remove_outliers();
	void calc_camera_params();
	void calc_gains();
	void blend_images();

	// second level processing
	void remove_too_few_match();
	void remove_ransac_outliers();
	void ransac_proc_match_ij(int op, int ed, int id);
	void ransac();
	void fit_prob_model();
	void create_match_info();
	void BA();

	// routine
	inline void sift_desc_push(FILE *fp, int iid, float x, float y, float *in);
	inline void remove_outliers_routine();
	int cal_BA_add_order(float &firstF);
	void prepare_BA_memory();
	void init_BA();
	void free_BA_memory();

	// for BA calculation
	void x_minus_fp();
	float norm2(float *ptr);
	float err_func();
	float dp_mul_udp_plus_g(float mu);
	void cal_A_plus_uC();
	void cal_JTJ();
	void cal_fp(float *p);
	void cal_JTr();
	void p_add_dp();
};

inline void Panorama::sift_desc_push(FILE *fp, int iid, float x, float y, float *in)
{
	Feature tmpFeature = {iid, x, y};
	features.push_back(tmpFeature);
	for (int i = 0; i < 128; ++i) {
		int tmpInt = in[i] * 1024.0f;
		fprintf(fp, " %d", tmpInt);
		featureData.push_back(tmpInt);
	}
}

inline void Panorama::remove_outliers_routine()
{
	int readID, writeID;
	readID = writeID = 0;
	for (int i = 0; i < imageMatchInfos.size(); ++i) {
		if (imageMatchInfos[i].matchNum > 0) {
			if (readID == writeID) {
				readID  += imageMatchInfos[i].matchNum;
				writeID += imageMatchInfos[i].matchNum;
			} else {
				// maybe use std::copy?
				for (int j = 0; j < imageMatchInfos[i].matchNum; ++j) {
					matchs[writeID] = matchs[readID];
					++readID;
					++writeID;
				}
			}
		} else {
			readID -= imageMatchInfos[i].matchNum;
		}
	}
}

inline void Panorama::prepare_BA_memory()
{
	int N = imageNames.size();
	int M = matchs.size();
	imgIsAdd = new bool[N];
	intBuf = new int[3*N - 2];
	floatBuf = new float[((N*N)<<4) + (M*6) + (N*14)];

	// integer pointers
	addOrder = intBuf;
	appendOrder = addOrder + N - 1;
	rMap = appendOrder + N - 1;
	// float pointers
	A = floatBuf;
	A_plus_uC = A + ((N*N)<<4);
	g = A_plus_uC + ((N*N)<<4);
	dp = g + (N<<2);
	pCur = dp + (N<<2);
	pNew = pCur + (N<<2);
	ep = pNew + (N<<2);
	epNew = ep + (M<<1);
	fp = epNew + (M<<1);
	RBuf = fp + (M<<1);
}

inline void Panorama::free_BA_memory()
{
	delete[] imgIsAdd;
	delete[] intBuf;
	delete[] floatBuf;
}
#endif
