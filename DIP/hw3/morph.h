#ifndef __MORPH_H__
#define __MORPH_H__

void morph_edge(const unsigned char *img, unsigned char *boundary, const int w, const int h);
int label_image(const unsigned char *img, unsigned char *lb, const int w, const int h);
int erosion_object(const unsigned char *img, const int w, const int h, const int nIter);

#endif /* end of include guard */
