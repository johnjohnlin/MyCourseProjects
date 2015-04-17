#include <cstdlib>

#define CHECK_BUF
#ifdef CHECK_BUF
#  include <assert.h>
#endif

template<typename T>
struct Buffer {
	T *ptr;
	T *ptr_used_end;
	size_t bufSize;
	inline Buffer();
	inline Buffer(size_t nr);
	inline ~Buffer();
	inline void clean();
	inline void alloc(size_t nr, bool forceRealloc=true);
	inline T* use(size_t nr);
	inline void rewind();
};

template <typename T>
inline Buffer<T>::Buffer():
	bufSize(0),
	ptr(NULL)
{
}

template <typename T>
inline Buffer<T>::Buffer(size_t nr): bufSize(nr)
{
	ptr_used_end = ptr = new T[nr];
}

template <typename T>
inline Buffer<T>::~Buffer()
{
	clean();
}

template <typename T>
inline void Buffer<T>::clean()
{
	if (ptr) {
		delete[] ptr;
	}
	bufSize = 0;
}

template <typename T>
inline void Buffer<T>::alloc(size_t nr, bool forceRealloc)
{
	if (nr > this->bufSize || forceRealloc) {
		clean();
		ptr_used_end = ptr = new T[nr];
    this->bufSize = nr;
	}
}

template <typename T>
inline T* Buffer<T>::use(size_t nr)
{
	T *ret = ptr_used_end;
	ptr_used_end += nr;
#ifdef CHECK_BUF
	assert(ptr_used_end <= ptr + bufSize);
#endif
	return ret;
}

template <typename T>
inline void Buffer<T>::rewind()
{
	ptr_used_end = ptr;
}
