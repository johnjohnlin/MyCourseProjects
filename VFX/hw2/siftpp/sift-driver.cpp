// file:        sift-driver.cpp
// author:      Andrea Vedaldi
// description: SIFT command line utility implementation

// AUTORIGTHS

#include"sift.hpp"

#include<string>
#include<cstdio>
#include<iostream>
#include<iomanip>
#include<fstream>
#include<sstream>
#include<algorithm>
#include<stdint.h>
extern "C" {
#include<getopt.h>
#if defined (VL_MAC)
#include<libgen.h>
#else
#include<string.h>
#endif
#include<assert.h>
}
#include<memory>

using namespace std ;

size_t const not_found = numeric_limits<size_t>::max() - 1 ;

/** @brief Case insensitive character comparison
 **
 ** This predicate returns @c true if @a a and @a b are equal up to
 ** case.
 **
 ** @return predicate value.
 **/
	inline
bool ciIsEqual(char a, char b)
{
	return 
	tolower((char unsigned)a) == 
	tolower((char unsigned)b) ;
}

/** @brief Case insensitive extension removal
 **
 ** The function returns @a name with the suffix $a ext removed.  The
 ** suffix is matched case-insensitve.
 **
 ** @return @a name without @a ext.
 **/
	string
removeExtension(string name, string ext)
{
	string::iterator pos = 
	find_end(name.begin(),name.end(),ext.begin(),ext.end(),ciIsEqual) ;

	// make sure the occurence is at the end
	if(pos+ext.size() == name.end()) {
		return name.substr(0, pos-name.begin()) ;
	} else {
		return name ;
	}
}


/** @brief Insert descriptor into stream
 **
 ** The function writes a descriptor in ASCII/binary format
 ** and in integer/floating point format into the stream.
 **
 ** @param os file pointer
 ** @param descr_pt descriptor (floating point)
 **/
void insertDescriptor(FILE *fp, VL::float_t const * descr_pt)
{
	for(int i = 0 ; i < 128 ; ++i) {
		printf(" %u", (uint32_t)descr_pt[i]);
	}
}

/* keypoint list */
typedef vector<pair<VL::Sift::Keypoint,VL::float_t> > Keypoints ;

/* predicate used to order keypoints by increasing scale */
bool cmpKeypoints (Keypoints::value_type const&a,
	Keypoints::value_type const&b) {
	return a.first.sigma < b.first.sigma ;
}

// -------------------------------------------------------------------
//                                                                main
// -------------------------------------------------------------------
int
main(int argc, char** argv)
{
	// initialize scalespace
	int S = 3;
	int omin = 0;
	float const sigman = .5f ;
	float const sigma0 = 1.6f * powf(2.0f, 1.0f / S) ;
	float *data;
	int width, height;
	VL::Sift sift(data, width, height, sigman, sigma0, 3, S, omin, -1, S+1);

	float const threshold = 0.00666667f;
	float const edgeThreshold = 10.0f;
	sift.detectKeypoints(threshold, edgeThreshold);

	float const magnif = 3.0;
	sift.setNormalizeDescriptor(true) ;
	sift.setMagnification( magnif ) ;

	FILE *fout = fopen("test", "w");
	// -------------------------------------------------------------
	//            Run detector, compute orientations and descriptors
	// -------------------------------------------------------------
	for(VL::Sift::KeypointsConstIter iter = sift.keypointsBegin();
		iter != sift.keypointsEnd() ; ++iter) {

		// detect orientations
		VL::float_t angles[4] ;
		int nangles;
		nangles = sift.computeKeypointOrientations(angles, *iter) ;

		// compute descriptors
		// for(int a = 0 ; a < nangles ; ++a)
		{

			fprintf(fout, "%2f %2f %2f %2f", iter->x, iter->y, iter->sigma, angles[a]);

			/* compute descriptor */
			VL::float_t descr_pt[128];
			sift.computeKeypointDescriptor(descr_pt, *iter, angles[a]);

			/* save descriptor to to appropriate file */
			insertDescriptor(fout, descr_pt);

			fputs("\n", fout);
		} // next angle
	} // next keypoint

	fclose(fout);

	return 0;
}
