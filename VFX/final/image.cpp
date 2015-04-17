#include <algorithm>
#include "image.hpp"

Image::Image()
{
  header = NULL;
  alpha = NULL;
  isGray = false;
  headerLen = 0;
  w = 0;
  h = 0;
}

Image::~Image()
{
  clear();
}

float* Image::read_bmp(const char *name, const bool toGray)
{
  clear();
  isGray = toGray;

  FILE* inFile = fopen(name, "r");
  if (inFile == NULL) {
    fprintf(stderr, "Error: cannot open input file \'%s\'\n", name);
    return 0;
  }

  /* Get bmp size. */
  int bmpSize = 0;
  fseek(inFile, 0x2, SEEK_SET);
  for (int i = 0; i < 4; ++i)
    bmpSize += ((int)fgetc(inFile) << (i * 8));

  /* Get bmp width, height. */
  w = 0;
  h = 0;
  fseek(inFile, 0x12, SEEK_SET);
  for (int i = 0; i < 4; ++i)
    w += ((int)fgetc(inFile) << (i * 8));
  for (int i = 0; i < 4; ++i)
    h += ((int)fgetc(inFile) << (i * 8));

  /* Get bmp offset pixel. */
  int bmpOffset = 0;
  fseek(inFile, 0xa, SEEK_SET);
  for (int i = 0; i < 4; ++i)
    bmpOffset += ((int)fgetc(inFile) << (i * 8));

  /* Get bmp color depth (24 for RGB, or 32 for aRGB.) */
  int bmpColorDepth = 0;
  fseek(inFile, 0x1c, SEEK_SET);
  for (int i = 0; i < 4; ++i)
    bmpColorDepth += ((int)fgetc(inFile) << (i * 8));

  /* Save the header. */
  headerLen = bmpOffset;
  header = new char[headerLen];
  fseek(inFile, 0, SEEK_SET);
  for (int i = 0; i < headerLen; ++i)
    header[i] = fgetc(inFile);

  /* Read each pixel. */
  float *image = new float[h * w * 3];
  alpha = new float[h * w];
#ifndef imgAt
#define imgAt(x, y, z) (*(image + (x)*w*3 + (y)*3 + (z)))
  int iCol;
  fseek(inFile, bmpOffset, SEEK_SET);
  for (int i = 0; i < h; ++i) {
    iCol = 0;
    /* Pixel value. */
    for (int j = 0; j < w; ++j) {
      imgAt(h-i-1, j, 2) = (float)fgetc(inFile);
      imgAt(h-i-1, j, 1) = (float)fgetc(inFile);
      imgAt(h-i-1, j, 0) = (float)fgetc(inFile);
      iCol += 3;

      if (bmpColorDepth == 32) { // alpha channel if exist
        alpha[i*w + j] = (float)fgetc(inFile);
        ++iCol;
      }
      else
        alpha[i*w + j] = 1;
    }
    /* Padding. (row size to multiple of 4 Bytes) */
    for (; (iCol % 4) != 0; ++iCol)
      fgetc(inFile);
  }
  fclose(inFile);

  /* Convert to grayscale if needed. */
  if (isGray) {
#ifndef grayAt
#define grayAt(x, y) (*(gray + (x)*w + (y)))
    float *gray = new float[w * h];
    for (int i = 0; i < h; ++i) {
      for (int j = 0; j < w; ++j) {
        float sum = 0;
        for (int k = 0; k < 3; ++k)
          sum += imgAt(i, j, k);
        grayAt(i, j) = sum / 3;
      }
    }
#undef grayAt
#endif
    delete[] image;
    return gray;
  }

#undef imgAt
#endif

  return image;
}

void Image::write_bmp(const char *name, const float *image)
{
  FILE* outFile = fopen(name, "w");
  if (outFile == NULL) {
    fprintf(stderr, "Error: cannot open output file \'%s\'\n", name);
    return;
  }

  /* Write header. */
  for (int i = 0; i < headerLen; ++i)
    fputc((int)header[i], outFile);

  /* Get bmp color depth (24 for RGB, or 32 for aRGB.) */
  int bmpColorDepth = 0;
  for (int i = 0; i < 4; ++i)
    bmpColorDepth += ((int)header[0x1c + i] << (i * 8));

  /* Write pixel value. */
#ifndef imgAt
#define imgAt(x, y, z) (*(image + (x)*w*3 + (y)*3 + (z)))
  int iCol;
  for (int i = 0; i < h; ++i) {
    iCol = 0;
    /* Pixel value. */
    for (int j = 0; j < w; ++j) {
      if (isGray) {
        int p = image[(h-i-1)*w + j];
        p = std::max(0, p);
        p = std::min(p, 255);
        for (int k = 0; k < 3; ++k) {
          fputc(p, outFile);
        }
      }
      else {
        int p = imgAt(h-i-1, j, 2);
        p = std::max(0, p);
        p = std::min(p, 255);
        fputc(p, outFile);
        p = imgAt(h-i-1, j, 1);
        p = std::max(0, p);
        p = std::min(p, 255);
        fputc(p, outFile);
        p = imgAt(h-i-1, j, 0);
        p = std::max(0, p);
        p = std::min(p, 255);
        fputc(p, outFile);
      }
      iCol += 3;
      if (bmpColorDepth == 32) { // alpha channel if exist
        fputc((int)alpha[i*w + j], outFile);
        ++iCol;
      }
    }
    /* Padding. (row size to multiple of 4 Bytes) */
    for (; (iCol % 4) != 0; ++iCol)
      fputc(0, outFile);
  }
#undef imgAt
#endif
  fclose(outFile);
}

void Image::clear()
{
  if (header != NULL)
    delete[] header;
  header = NULL;

  if (alpha != NULL)
    delete[] alpha;
  alpha = NULL;

  isGray = false;
  headerLen = 0;
  w = 0;
  h = 0;
}
