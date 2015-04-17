#ifndef __IMAGE__
#define __IMAGE__

#include <cstdio>

class Image {
public:
  Image();
  ~Image();
  float* read_bmp(const char *name, const bool toGray);
  void   write_bmp(const char *name, const float *image);
  void   clear();

  char   *header;
  float  *alpha;
  bool   isGray;
  size_t headerLen;
  size_t w;
  size_t h;
};

#endif
