#ifndef __TIME_UTIL_H__
#define __TIME_UTIL_H__

char* alloc_time_string(const char* tf, int is_msec, char** time_string);
void  dealloc_time_string(char* ts);
#endif
