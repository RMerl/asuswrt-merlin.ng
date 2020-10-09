
#pragma once

#if HAVE_SYS_STAT_H
#include_next <sys/stat.h>
#endif

#ifndef lstat
#define lstat stat
#endif

#ifndef S_ISLNK
#ifdef __S_IFLNK
#define S_ISLNK(mode)	__S_ISTYPE((mode), __S_IFLNK)
#else
#define S_ISLNK(mode)	0
#endif
#endif

#define link(a, b)	CreateHardLink((a), (b), NULL)
