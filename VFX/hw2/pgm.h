#ifndef __PGM_H__
#define __PGM_H__

void load_P5_pgm(FILE *fp, int *w, int *h, unsigned char **ptr);
void save_P5_pgm(FILE *fp, int  w, int  h, unsigned char  *ptr);

#endif /* __PGM_H__ */
