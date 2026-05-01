#ifndef _STRCASESTR_H_
#define _STRCASESTR_H_

#if defined(WIFI7_SDK_20250506)
extern char * strcasestr(const char* haystack, const char* needle);
#else
extern char * strcasestr(char* haystack, char* needle);
#endif

#endif
