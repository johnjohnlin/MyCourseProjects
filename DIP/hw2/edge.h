#ifndef __EDGE_H__
#define __EDGE_H__

void canny(const int w, const int h, const unsigned char *img, unsigned char *e, const int thL, const int thH);
void sobel(const int w, const int h, const unsigned char *img, unsigned char *e, const int th);
void zero_cross(const int w, const int h, const unsigned char *img, unsigned char *e, const int th);

#endif /* end of include guard */
