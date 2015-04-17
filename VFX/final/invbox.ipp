#include <algorithm>
#include <cstdio>

inline void InvBox::solve(float *img)
{
	solve_1D_and_transpose(imgTmp, img, param.w, param.h, vGECoeff, vInvDiag);
	solve_1D_and_transpose(img, imgTmp, param.h, param.w, hGECoeff, hInvDiag);
}

inline void InvBox::set_param(const Param &param, bool forceRealloc)
{
	this->param = param;
	fbuf.alloc(cal_fbuf_size(this->param), forceRealloc);
  assign_ptr();
	cal_HVCoeff();
}

inline void InvBox::clean()
{
	fbuf.clean();
}

inline size_t InvBox::cal_fbuf_size(const Param &param)
{
	size_t maxEdge = std::max(param.w, param.h);
	return maxEdge*param.sl_J
		 + (param.w+param.h-2) * (param.r_J << 1)
		 + param.w*param.h
		 + param.w + param.h;
}

inline void InvBox::assign_ptr()
{
	size_t maxEdge = std::max(param.w, param.h);
	imgTmp = fbuf.use(param.w * param.h);
	band_matrix  = fbuf.use(maxEdge * param.sl_J);
	vGECoeff = fbuf.use(param.r_J * ((param.h - 1) << 1));
	vInvDiag = fbuf.use(param.h);
	hGECoeff = fbuf.use(param.r_J * ((param.w - 1) << 1));
	hInvDiag = fbuf.use(param.w);
}

inline void InvBox::cal_HVCoeff()
{
	build_matrix(param.h);
	cal_GECoeff(param.h, vGECoeff, vInvDiag);
	build_matrix(param.w);
	cal_GECoeff(param.w, hGECoeff, hInvDiag);
}
