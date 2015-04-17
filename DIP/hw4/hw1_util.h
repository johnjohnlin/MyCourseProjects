#ifndef __HW1_UTILS__
#define __HW1_UTILS__

void downsample2(unsigned char *out, const unsigned char *in, const int w, const int h);
void median_filter(unsigned char *out, const unsigned char *in, const int w, const int h);
void otsu_threshold(unsigned char *out, const unsigned char *in, const int w, const int h);

#endif /* end of include guard */
