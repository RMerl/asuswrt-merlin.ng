#ifndef _DATE_H_
#define _DATE_H_

#include <sys/time.h>

int do_gettimeofday(void);
void gettimeofday_cached(struct timeval *tv);
int time_cached(void);

#endif
