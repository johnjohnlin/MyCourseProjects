#ifndef __CLSTRUCT_H__
#define __CLSTRUCT_H__

#include <cstdlib>
#include <CL/cl.h>
#include <cstdarg>

struct CLStruct {
	cl_platform_id platform;
	cl_device_id device;
	cl_context context;
	cl_command_queue cqueue;
	cl_program program;
	cl_kernel gaussian, diff;
};

static void ABORT_IF(bool cond, const char *fmt, ...)
{
	va_list v;
	va_start(v, fmt);
	
	if (cond) {
		vprintf(fmt, v);
		abort();
	}

	va_end(v);
}

#endif /* end of include guard: __CLSTRUCT_H__ */
