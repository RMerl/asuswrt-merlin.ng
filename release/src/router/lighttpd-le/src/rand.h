#ifndef LI_RAND_H_
#define LI_RAND_H_
#include "first.h"

int li_rand_pseudo_bytes (void);
void li_rand_reseed (void);
int li_rand_bytes (unsigned char *buf, int num);
void li_rand_cleanup (void);

#endif
