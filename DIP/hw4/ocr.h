#ifndef __OCR_H__
#define __OCR_H__
#include <bitset>

typedef std::bitset<42> Feature;
char match32(const Feature f);
Feature build_feature32(const unsigned char *patch);

#endif /* end of include guard */
