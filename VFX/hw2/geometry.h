#ifndef __GEOMETRY_H__
#define __GEOMETRY_H__

extern "C" {
/** @brief lapack prototype definition, solve <c>A*x = b</c>
 *
 *  All type if of pointer, array is column-major
 *
 *  @param[in] m A is m*m, must > 0
 *  @param[in] n B is m*n, must > 0
 *  @param[in,out] A the matrix A, <c>size = LDA*n</c>
 *  @param[in] LDA align of matrix A
 *  @param[out] IPIV 1*n
 *  @param[in,out] B the matrix B <c> size = LDB*n</c>
 *  @param[in] LDB align of matrix B
 *  @param[out] info
 *          = 0:  successful exit
 *          < 0:  if INFO = -i, the i-th argument had an illegal value
 *          > 0:  if INFO = i, U(i,i) is exactly zero.  The factorization
 *                has been completed, but the factor U is exactly
 *                singular, so the solution could not be computed.
 */
void sgesv_(int *m, int *n, float *A, int *LDA, int *IPIV, float *B, int *LDB, int *info);
};

#include "panorama.h"
#include <cmath>

namespace Geometry {

inline void proj(Feature &fi, Feature &fj, float *H);
inline int solve_Ax_eq_b(float *A, float *b, int *buf, int n);
inline void solve_fj_from_H(float &fj, float *H, float fallback);
inline int solve_H_from_4matchs(float *H, MatchPair m1, MatchPair m2, MatchPair m3, MatchPair m4, float hinti, float hintj);
inline float proj_dist(MatchPair matchs, float *H);
inline float feature_dist(Feature &f1, Feature &f2);
inline void cal_R_from_param(float *R, float *param);

#include "geometry.ipp"
};

#endif
