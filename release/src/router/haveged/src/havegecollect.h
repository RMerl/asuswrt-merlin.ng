/**
 ** Simple entropy harvester based upon the havege RNG
 **
 ** Copyright 2018-2022 Jirka Hladky hladky DOT jiri AT gmail DOT com
 ** Copyright 2009-2014 Gary Wuertz gary@issiweb.com
 ** Copyright 2011-2012 BenEleventh Consulting manolson@beneleventh.com
 **
 ** This program is free software: you can redistribute it and/or modify
 ** it under the terms of the GNU General Public License as published by
 ** the Free Software Foundation, either version 3 of the License, or
 ** (at your option) any later version.
 **
 ** This program is distributed in the hope that it will be useful,
 ** but WITHOUT ANY WARRANTY; without even the implied warranty of
 ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 ** GNU General Public License for more details.
 **
 ** You should have received a copy of the GNU General Public License
 ** along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef HAVEGECOLLECT_H
#define HAVEGECOLLECT_H
/**
 ** Definitions needed to build haveged
 */
#include "havege.h"
/**
 * The collection context
 */
typedef struct h_collect {
   void    *havege_app;                   /* Application block             */
   H_UINT   havege_idx;                   /* Identifer                     */
   H_UINT   havege_szCollect;             /* Size of collection buffer     */
   H_UINT   havege_raw;                   /* RAW mode control flags        */
   H_UINT   havege_szFill;                /* Fill size                     */
   H_UINT   havege_nptr;                  /* Input pointer                 */
   pRawIn   havege_rawInput;              /* Injection function            */
   pRawIn   havege_testInput;             /* Injection function for test   */
   H_UINT   havege_cdidx;                 /* normal mode control flags     */
   H_UINT  *havege_pwalk;                 /* Instance variable             */
   H_UINT   havege_andpt;                 /* Instance variable             */
   H_UINT   havege_PT;                    /* Instance variable             */
   H_UINT   havege_PT2;                   /* Instance variable             */
   H_UINT   havege_pt2;                   /* Instance variable             */
   H_UINT   havege_PTtest;                /* Instance variable             */
   H_UINT   havege_tic;                   /* Instance variable             */
   H_UINT  *havege_tics;                  /* loop timer noise buffer       */
   H_UINT   havege_err;                   /* H_ERR enum for status         */
   void    *havege_tests;                 /* opague test context           */
   void    *havege_extra;                 /* other allocations             */
   H_UINT   havege_bigarray[1];           /* collection buffer             */
} volatile H_COLLECT;
/**
 ** Compiler intrinsics are used to make the build more portable and stable
 ** with fall-backs provided where the intrisics cannot be used. 
 */
#ifdef __GNUC__
/* ################################################################################# */

/**
 ** For the GNU compiler, the use of a cpuid intrinsic is somewhat garbled by the
 ** fact that some distributions (Centos 5.x) carry an empty cpuid.h (in order
 ** to back patch glicb?). AFAIK cpuid did not appear in gcc until version 4.3
 ** although it was in existence before. If we do not have a valid cpuid.h,
 ** we provide our own copy of the file (from gcc 4.3)
 **
 ** Also, gcc 4.4 and later provide an optimize attribute which remedies the
 ** effect ever increasing optimization on the collection loop
 */
#define GCC_VERSION (__GNUC__ * 10000 + __GNUC_MINOR__ * 100  +  __GNUC_PATCHLEVEL__)

#define ASM __asm__ volatile
/**
 ** For the intel world...
 */
#ifdef HAVE_ISA_X86
#define ARCH "x86"

#if GCC_VERSION<40300
#undef HAVE_CPUID_H
#endif
#ifdef HAVE_CPUID_H
#include <cpuid.h>
#else
#include "cpuid-43.h"
#endif
/**
 ** Compatibility wrappers
 */
#define CPUID(level,p)\
  {\
  register int ecx asm ("ecx") = p[2];\
  __cpuid(level,p[0],p[1],p[2],p[3]);\
  (void) ecx;\
  }
#define HASCPUID(p) __get_cpuid_max(0, p)
/**
 ** The rdtsc intrinsic is called in by x86intrin.h - also a recent gcc innovation
 ** There have been some discussions of the code in 4.5 and 4.6, so you may opt
 ** to use the inline alternative based on GCC_VERSION
 */
#ifdef HAVE_X86INTRIN_H
#include <x86intrin.h>
#endif
#ifdef HAVE___RDTSC
#define HARDCLOCK(x) x=__rdtsc()
#else
#define HARDCLOCK(x) ASM("rdtsc;movl %%eax,%0":"=m"(x)::"ax","dx")
#endif
#else
/**
 * Outside the x86 family
 */
#ifdef HAVE_ISA_GENERIC
#define ARCH "generic"
#define ENABLE_CLOCKGETTIME 1
#endif

#ifdef HAVE_ISA_IA64
#define ARCH "ia64"
#define HARDCLOCK(x) ASM("mov %0=ar.itc" : "=r"(x))
#endif

#ifdef HAVE_ISA_SPARC
#define ARCH "sparc"
#define HARDCLOCK(x) ASM("rd %%tick, %0":"=r"(x):"r"(x))
#endif

#ifdef HAVE_ISA_SPARCLITE
#define ARCH "sparclite"
#define HARDCLOCK(x) ASM(".byte 0x83, 0x41, 0x00, 0x00");\
  ASM("mov   %%g1, %0" : "=r"(x))
#endif

#ifdef HAVE_ISA_PPC
#define ARCH "ppc"
#define HARDCLOCK(x) ASM("mftb %0":"=r"(x)) /* eq. to mftb %0, 268 */
#endif

#ifdef HAVE_ISA_S390
#define ARCH "s390"
#define HARDCLOCK(x) { unsigned long long tsc; ASM("stck %0":"=Q"(tsc)::"cc"); x = (unsigned int)tsc; }
#endif
/**
 * /Outside the x86 family
 */
#endif
/**
 *  Use the "&&" extension to calculate the LOOP_PT
 */
#define CODE_PT(a)   a
#define LOOP_PT(a)   &&loop##a

/* ################################################################################# */
#endif
/**
 * For the MSVC world
 */
#if _MSVC_VERS
/* ################################################################################# */
#define ARCH "x86"
/**
 * For the MSVC compilers V8 and above
 */
#include <intrin.h>
/**
 * Read the processor timestamp counter
 */
#define HARDCLOCK(x) x=__rdtsc()
/**
 * Normalize to the gcc interface
 */
#define CPUID(level,p) return __cpuidx(p, level, p[2])
#define HASCPUID(p) \
  { \
  CPUID(0,a,b,c,d) \
  }
/**
 * Use the __ReturnAddress intrinsic to calculate the LOOP_PT
 */
#define CODE_PT(a) __ReturnAddress()
#define LOOP_PT(a) 0
#endif
/* ################################################################################# */
/**
 * Configuration defaults - allow override at compile
 */
#ifndef COLLECT_BUFSIZE
#define COLLECT_BUFSIZE 128                     /* collection buffer size in KW              */
#endif
#ifndef GENERIC_DCACHE
#define GENERIC_DCACHE  16                      /* size of L1 data cache                     */
#endif
#ifndef GENERIC_ICACHE
#define GENERIC_ICACHE  16                      /* size of L1 instruction cache              */
#endif
#ifndef LOOP_CT
#define LOOP_CT 40                              /* Max interations per collection loop       */
#endif
/**
 * Other useful definitions
 */
#define BITS_PER_H_UINT (8*sizeof(H_UINT))      /* Bit packing constant                      */
#define DEFAULT_BUFSZ   1024*sizeof(H_UINT)     /* Default for ioSz                          */
#define MININITRAND     32                      /* Number of initial fills to prime RNG      */
#define NDSIZECOLLECT   (COLLECT_BUFSIZE*1024)  /* Collection size: 128K*H_UINT = .5M byte   */
/**
 ** The public collection interface
 */
H_COLLECT   *havege_ndcreate(H_PTR hptr, H_UINT nCollector);
void        havege_nddestroy(H_COLLECT *rdr);
H_UINT      havege_ndread(H_COLLECT *rdr);
void        havege_ndsetup(H_PTR hptr);

#endif
