/* freebsd8 is a superset of freebsd7 */
#include "freebsd7.h"
#define freebsd7 freebsd7

/*
 * Not completely sure when these fields got
 * added to FreeBSD, but FreeBSD 8 is about the oldest
 * one we care about, so add them here.
 */
#undef UTMP_HAS_NO_TYPE
#undef UTMP_HAS_NO_PID
