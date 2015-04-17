inline void proj(Feature &fi, Feature &fj, float *H)
{
	float x = H[0] * fi.x + H[1] * fi.y + H[2];
	float y = H[3] * fi.x + H[4] * fi.y + H[5];
	float z = H[6] * fi.x + H[7] * fi.y + H[8];
	fj.x = x/z;
	fj.y = y/z;
}

inline int solve_Ax_eq_b(float *A, float *b, int *buf, int n)
{
	int one = 1;
	int info;
	sgesv_(&n, &one, A, &n, buf, b, &n, &info);
	return info;
}

inline void solve_fj_from_H(float &fj, float *H, float fallback)
{
	int fj2 = (H[0]+H[1]-H[3]-H[4]) / (H[6]-H[3]);
	fj = (fj2 > 0? sqrtf(fj2): fallback);
}

// Hi ~ j
inline int solve_H_from_4matchs(float *H, MatchPair m1, MatchPair m2, MatchPair m3, MatchPair m4, float hinti, float hintj)
{
	int buf[8];
	float A[64] = {};
	MatchPair ms[4] = {m1, m2, m3, m4};
	for (int i = 0; i < 4; ++i) {
		float ix = ms[i].f1.x;
		float iy = ms[i].f1.y;
		float jx = ms[i].f2.x;
		float jy = ms[i].f2.y;
		A[ 1+(i<<1)] =  hintj * ix;
		A[ 9+(i<<1)] =  hintj * iy;
		A[17+(i<<1)] =  hintj * hinti;

		A[24+(i<<1)] = -hintj * ix;
		A[32+(i<<1)] = -hintj * iy;
		A[40+(i<<1)] = -hintj * hinti;

		A[48+(i<<1)] =     jy * ix;
		A[56+(i<<1)] =     jy * iy;
		H[ 0+(i<<1)] =     jy * hinti;

		A[49+(i<<1)] = -   jx * ix;
		A[57+(i<<1)] = -   jx * iy;
		H[ 1+(i<<1)] = -   jx * hinti;
	}
	int info = solve_Ax_eq_b(A, H, buf, 8);
	H[8] = -1.0f;

	H[2] *= hintj;
	H[5] *= hintj;
	H[8] *= hintj;

	H[0] *= hinti;
	H[1] *= hinti;
	H[2] *= hinti;
	H[3] *= hinti;
	H[4] *= hinti;
	H[5] *= hinti;
	return info;
}

inline float proj_dist(MatchPair matchs, float *H)
{
	Feature fi = matchs.f1, fj;
	proj(fi, fj, H);
	return feature_dist(matchs.f2, fj);
}

inline float feature_dist(Feature &f1, Feature &f2)
{
	float diffx = f1.x - f2.x;
	float diffy = f1.y - f2.y;
	return diffx*diffx + diffy*diffy;
}

inline void cal_R_from_param(float *R, float *param)
{
	float t = sqrtf(param[0] * param[0] + param[1] * param[1] + param[2] * param[2]);
	float x, y, z;
	{
		float ti = 1.0 / t;
		x = param[0] / t;
		y = param[1] / t;
		z = param[2] / t;
	}
	t *= 0.5;
	float c = cosf(t), s = sinf(t);
#define Rat(i, j) (R[3*(i)+(j)])
	{
		float s2 = s * s * 2.0;
		float x21 = x * x - 1.0;
		float y21 = y * y - 1.0;
		float z21 = z * z - 1.0;
		float xys2 = x * y * s2, xzs2 = x * z * s2, yzs2 = y * z * s2;

		Rat(0, 1) = xys2;
		Rat(1, 0) = xys2;

		Rat(0, 2) = xzs2;
		Rat(2, 0) = xzs2;

		Rat(1, 2) = yzs2;
		Rat(2, 1) = yzs2;

		Rat(0, 0) = x21 * s2;
		Rat(1, 1) = y21 * s2;
		Rat(2, 2) = z21 * s2;
	}
	{
		float cs = c * s * 2.0;
		float csx = cs * x, csy = cs * y, csz = cs * z;
		Rat(0, 1) -= csz;
		Rat(0, 2) += csy;
		Rat(1, 0) += csz;
		Rat(1, 2) -= csx;
		Rat(2, 0) -= csy;
		Rat(2, 1) += csx;
	}

	Rat(0, 0) += 1.0f;
	Rat(1, 1) += 1.0f;
	Rat(2, 2) += 1.0f;
}
