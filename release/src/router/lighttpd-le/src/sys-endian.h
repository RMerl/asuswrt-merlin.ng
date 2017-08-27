#ifndef LI_SYS_ENDIAN_H
#define LI_SYS_ENDIAN_H
#include "first.h"


#if !defined(__LITTLE_ENDIAN__) && !defined(__BIG_ENDIAN__)


/* copied from of plasma_endian.h
 * https://github.com/gstrauss/plasma/blob/master/plasma_endian.h
 * (used with permission from the author (gstrauss)) */
#if defined(__BYTE_ORDER__)              \
 && (   defined(__ORDER_LITTLE_ENDIAN__) \
     || defined(__ORDER_BIG_ENDIAN__)    \
     || defined(__ORDER_PDP_ENDIAN__)   )
  #if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
  #define __LITTLE_ENDIAN__ 1
  #elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
  #define __BIG_ENDIAN__ 1
  #endif
#elif defined(_LITTLE_ENDIAN) && !defined(_BIG_ENDIAN)
  #define __LITTLE_ENDIAN__ 1
#elif !defined(_LITTLE_ENDIAN) && defined(_BIG_ENDIAN)
  #define __BIG_ENDIAN__ 1
#elif defined(_WIN32) /* little endian on all current MS-supported platforms */
  #define __LITTLE_ENDIAN__ 1
#elif defined(__GLIBC__) || defined(__linux__)
  #include <endian.h>
  #if __BYTE_ORDER == __LITTLE_ENDIAN
  #define __LITTLE_ENDIAN__ 1
  #elif __BYTE_ORDER == __BIG_ENDIAN
  #define __BIG_ENDIAN__ 1
  #endif
#elif defined(__sun__) && defined(__SVR4)
  #include <sys/isa_defs.h>
  #if defined(_LITTLE_ENDIAN)
  #define __LITTLE_ENDIAN__ 1
  #elif defined(_BIG_ENDIAN)
  #define __BIG_ENDIAN__ 1
  #endif
#elif defined(_AIX)
  #include <sys/machine.h>
  #if BYTE_ORDER == LITTLE_ENDIAN
  #define __LITTLE_ENDIAN__ 1
  #elif BYTE_ORDER == BIG_ENDIAN
  #define __BIG_ENDIAN__ 1
  #endif
#elif defined(__APPLE__) && defined(__MACH__)
  #include <machine/endian.h>
  #if BYTE_ORDER == LITTLE_ENDIAN
  #define __LITTLE_ENDIAN__ 1
  #elif BYTE_ORDER == BIG_ENDIAN
  #define __BIG_ENDIAN__ 1
  #endif
#elif defined(__FreeBSD__) || defined(__NetBSD__) \
   || defined(__OpenBSD__) || defined(__DragonFly__)
  #include <machine/endian.h>
  #if _BYTE_ORDER == _LITTLE_ENDIAN
  #define __LITTLE_ENDIAN__ 1
  #elif _BYTE_ORDER == _BIG_ENDIAN
  #define __BIG_ENDIAN__ 1
  #endif
#else /*(else assume little endian)*/
  #define __LITTLE_ENDIAN__ 1
#endif


#endif /* !defined(__LITTLE_ENDIAN__) && !defined(__BIG_ENDIAN__) */


#endif
