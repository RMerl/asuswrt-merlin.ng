/*
  A version of malloc/free/realloc written by Doug Lea and released to the
  public domain.  Send questions/comments/complaints/performance data
  to dl@cs.oswego.edu

* VERSION 2.6.6  Sun Mar  5 19:10:03 2000  Doug Lea  (dl at gee)

   Note: There may be an updated version of this malloc obtainable at
	   ftp://g.oswego.edu/pub/misc/malloc.c
	 Check before installing!

* Why use this malloc?

  This is not the fastest, most space-conserving, most portable, or
  most tunable malloc ever written. However it is among the fastest
  while also being among the most space-conserving, portable and tunable.
  Consistent balance across these factors results in a good general-purpose
  allocator. For a high-level description, see
     http://g.oswego.edu/dl/html/malloc.html

* Synopsis of public routines

  (Much fuller descriptions are contained in the program documentation below.)

  malloc(size_t n);
     Return a pointer to a newly allocated chunk of at least n bytes, or null
     if no space is available.
  free(Void_t* p);
     Release the chunk of memory pointed to by p, or no effect if p is null.
  realloc(Void_t* p, size_t n);
     Return a pointer to a chunk of size n that contains the same data
     as does chunk p up to the minimum of (n, p's size) bytes, or null
     if no space is available. The returned pointer may or may not be
     the same as p. If p is null, equivalent to malloc.  Unless the
     #define REALLOC_ZERO_BYTES_FREES below is set, realloc with a
     size argument of zero (re)allocates a minimum-sized chunk.
  memalign(size_t alignment, size_t n);
     Return a pointer to a newly allocated chunk of n bytes, aligned
     in accord with the alignment argument, which must be a power of
     two.
  valloc(size_t n);
     Equivalent to memalign(pagesize, n), where pagesize is the page
     size of the system (or as near to this as can be figured out from
     all the includes/defines below.)
  pvalloc(size_t n);
     Equivalent to valloc(minimum-page-that-holds(n)), that is,
     round up n to nearest pagesize.
  calloc(size_t unit, size_t quantity);
     Returns a pointer to quantity * unit bytes, with all locations
     set to zero.
  cfree(Void_t* p);
     Equivalent to free(p).
  malloc_trim(size_t pad);
     Release all but pad bytes of freed top-most memory back
     to the system. Return 1 if successful, else 0.
  malloc_usable_size(Void_t* p);
     Report the number usable allocated bytes associated with allocated
     chunk p. This may or may not report more bytes than were requested,
     due to alignment and minimum size constraints.
  malloc_stats();
     Prints brief summary statistics on stderr.
  mallinfo()
     Returns (by copy) a struct containing various summary statistics.
  mallopt(int parameter_number, int parameter_value)
     Changes one of the tunable parameters described below. Returns
     1 if successful in changing the parameter, else 0.

* Vital statistics:

  Alignment:                            8-byte
       8 byte alignment is currently hardwired into the design.  This
       seems to suffice for all current machines and C compilers.

  Assumed pointer representation:       4 or 8 bytes
       Code for 8-byte pointers is untested by me but has worked
       reliably by Wolfram Gloger, who contributed most of the
       changes supporting this.

  Assumed size_t  representation:       4 or 8 bytes
       Note that size_t is allowed to be 4 bytes even if pointers are 8.

  Minimum overhead per allocated chunk: 4 or 8 bytes
       Each malloced chunk has a hidden overhead of 4 bytes holding size
       and status information.

  Minimum allocated size: 4-byte ptrs:  16 bytes    (including 4 overhead)
			  8-byte ptrs:  24/32 bytes (including, 4/8 overhead)

       When a chunk is freed, 12 (for 4byte ptrs) or 20 (for 8 byte
       ptrs but 4 byte size) or 24 (for 8/8) additional bytes are
       needed; 4 (8) for a trailing size field
       and 8 (16) bytes for free list pointers. Thus, the minimum
       allocatable size is 16/24/32 bytes.

       Even a request for zero bytes (i.e., malloc(0)) returns a
       pointer to something of the minimum allocatable size.

  Maximum allocated size: 4-byte size_t: 2^31 -  8 bytes
			  8-byte size_t: 2^63 - 16 bytes

       It is assumed that (possibly signed) size_t bit values suffice to
       represent chunk sizes. `Possibly signed' is due to the fact
       that `size_t' may be defined on a system as either a signed or
       an unsigned type. To be conservative, values that would appear
       as negative numbers are avoided.
       Requests for sizes with a negative sign bit when the request
       size is treaded as a long will return null.

  Maximum overhead wastage per allocated chunk: normally 15 bytes

       Alignnment demands, plus the minimum allocatable size restriction
       make the normal worst-case wastage 15 bytes (i.e., up to 15
       more bytes will be allocated than were requested in malloc), with
       two exceptions:
	 1. Because requests for zero bytes allocate non-zero space,
	    the worst case wastage for a request of zero bytes is 24 bytes.
	 2. For requests >= mmap_threshold that are serviced via
	    mmap(), the worst case wastage is 8 bytes plus the remainder
	    from a system page (the minimal mmap unit); typically 4096 bytes.

* Limitations

    Here are some features that are NOT currently supported

    * No user-definable hooks for callbacks and the like.
    * No automated mechanism for fully checking that all accesses
      to malloced memory stay within their bounds.
    * No support for compaction.

* Synopsis of compile-time options:

    People have reported using previous versions of this malloc on all
    versions of Unix, sometimes by tweaking some of the defines
    below. It has been tested most extensively on Solaris and
    Linux. It is also reported to work on WIN32 platforms.
    People have also reported adapting this malloc for use in
    stand-alone embedded systems.

    The implementation is in straight, hand-tuned ANSI C.  Among other
    consequences, it uses a lot of macros.  Because of this, to be at
    all usable, this code should be compiled using an optimizing compiler
    (for example gcc -O2) that can simplify expressions and control
    paths.

  __STD_C                  (default: derived from C compiler defines)
     Nonzero if using ANSI-standard C compiler, a C++ compiler, or
     a C compiler sufficiently close to ANSI to get away with it.
  DEBUG                    (default: NOT defined)
     Define to enable debugging. Adds fairly extensive assertion-based
     checking to help track down memory errors, but noticeably slows down
     execution.
  REALLOC_ZERO_BYTES_FREES (default: NOT defined)
     Define this if you think that realloc(p, 0) should be equivalent
     to free(p). Otherwise, since malloc returns a unique pointer for
     malloc(0), so does realloc(p, 0).
  HAVE_MEMCPY               (default: defined)
     Define if you are not otherwise using ANSI STD C, but still
     have memcpy and memset in your C library and want to use them.
     Otherwise, simple internal versions are supplied.
  USE_MEMCPY               (default: 1 if HAVE_MEMCPY is defined, 0 otherwise)
     Define as 1 if you want the C library versions of memset and
     memcpy called in realloc and calloc (otherwise macro versions are used).
     At least on some platforms, the simple macro versions usually
     outperform libc versions.
  HAVE_MMAP                 (default: defined as 1)
     Define to non-zero to optionally make malloc() use mmap() to
     allocate very large blocks.
  HAVE_MREMAP                 (default: defined as 0 unless Linux libc set)
     Define to non-zero to optionally make realloc() use mremap() to
     reallocate very large blocks.
  malloc_getpagesize        (default: derived from system #includes)
     Either a constant or routine call returning the system page size.
  HAVE_USR_INCLUDE_MALLOC_H (default: NOT defined)
     Optionally define if you are on a system with a /usr/include/malloc.h
     that declares struct mallinfo. It is not at all necessary to
     define this even if you do, but will ensure consistency.
  INTERNAL_SIZE_T           (default: size_t)
     Define to a 32-bit type (probably `unsigned int') if you are on a
     64-bit machine, yet do not want or need to allow malloc requests of
     greater than 2^31 to be handled. This saves space, especially for
     very small chunks.
  INTERNAL_LINUX_C_LIB      (default: NOT defined)
     Defined only when compiled as part of Linux libc.
     Also note that there is some odd internal name-mangling via defines
     (for example, internally, `malloc' is named `mALLOc') needed
     when compiling in this case. These look funny but don't otherwise
     affect anything.
  WIN32                     (default: undefined)
     Define this on MS win (95, nt) platforms to compile in sbrk emulation.
  LACKS_UNISTD_H            (default: undefined if not WIN32)
     Define this if your system does not have a <unistd.h>.
  LACKS_SYS_PARAM_H         (default: undefined if not WIN32)
     Define this if your system does not have a <sys/param.h>.
  MORECORE                  (default: sbrk)
     The name of the routine to call to obtain more memory from the system.
  MORECORE_FAILURE          (default: -1)
     The value returned upon failure of MORECORE.
  MORECORE_CLEARS           (default 1)
     true (1) if the routine mapped to MORECORE zeroes out memory (which
     holds for sbrk).
  DEFAULT_TRIM_THRESHOLD
  DEFAULT_TOP_PAD
  DEFAULT_MMAP_THRESHOLD
  DEFAULT_MMAP_MAX
     Default values of tunable parameters (described in detail below)
     controlling interaction with host system routines (sbrk, mmap, etc).
     These values may also be changed dynamically via mallopt(). The
     preset defaults are those that give best performance for typical
     programs/systems.
  USE_DL_PREFIX             (default: undefined)
     Prefix all public routines with the string 'dl'.  Useful to
     quickly avoid procedure declaration conflicts and linker symbol
     conflicts with existing memory allocation routines.


*/


#ifndef __MALLOC_H__
#define __MALLOC_H__

/* Preliminaries */

#ifndef __STD_C
#ifdef __STDC__
#define __STD_C     1
#else
#if __cplusplus
#define __STD_C     1
#else
#define __STD_C     0
#endif /*__cplusplus*/
#endif /*__STDC__*/
#endif /*__STD_C*/

#ifndef Void_t
#if (__STD_C || defined(WIN32))
#define Void_t      void
#else
#define Void_t      char
#endif
#endif /*Void_t*/

#if __STD_C
#include <linux/stddef.h>	/* for size_t */
#else
#include <sys/types.h>
#endif	/* __STD_C */

#ifdef __cplusplus
extern "C" {
#endif

#if 0	/* not for U-Boot */
#include <stdio.h>	/* needed for malloc_stats */
#endif


/*
  Compile-time options
*/


/*
    Debugging:

    Because freed chunks may be overwritten with link fields, this
    malloc will often die when freed memory is overwritten by user
    programs.  This can be very effective (albeit in an annoying way)
    in helping track down dangling pointers.

    If you compile with -DDEBUG, a number of assertion checks are
    enabled that will catch more memory errors. You probably won't be
    able to make much sense of the actual assertion errors, but they
    should help you locate incorrectly overwritten memory.  The
    checking is fairly extensive, and will slow down execution
    noticeably. Calling malloc_stats or mallinfo with DEBUG set will
    attempt to check every non-mmapped allocated and free chunk in the
    course of computing the summmaries. (By nature, mmapped regions
    cannot be checked very much automatically.)

    Setting DEBUG may also be helpful if you are trying to modify
    this code. The assertions in the check routines spell out in more
    detail the assumptions and invariants underlying the algorithms.

*/

/*
  INTERNAL_SIZE_T is the word-size used for internal bookkeeping
  of chunk sizes. On a 64-bit machine, you can reduce malloc
  overhead by defining INTERNAL_SIZE_T to be a 32 bit `unsigned int'
  at the expense of not being able to handle requests greater than
  2^31. This limitation is hardly ever a concern; you are encouraged
  to set this. However, the default version is the same as size_t.
*/

#ifndef INTERNAL_SIZE_T
#define INTERNAL_SIZE_T size_t
#endif

/*
  REALLOC_ZERO_BYTES_FREES should be set if a call to
  realloc with zero bytes should be the same as a call to free.
  Some people think it should. Otherwise, since this malloc
  returns a unique pointer for malloc(0), so does realloc(p, 0).
*/


/*   #define REALLOC_ZERO_BYTES_FREES */


/*
  WIN32 causes an emulation of sbrk to be compiled in
  mmap-based options are not currently supported in WIN32.
*/

/* #define WIN32 */
#ifdef WIN32
#define MORECORE wsbrk
#define HAVE_MMAP 0

#define LACKS_UNISTD_H
#define LACKS_SYS_PARAM_H

/*
  Include 'windows.h' to get the necessary declarations for the
  Microsoft Visual C++ data structures and routines used in the 'sbrk'
  emulation.

  Define WIN32_LEAN_AND_MEAN so that only the essential Microsoft
  Visual C++ header files are included.
*/
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif


/*
  HAVE_MEMCPY should be defined if you are not otherwise using
  ANSI STD C, but still have memcpy and memset in your C library
  and want to use them in calloc and realloc. Otherwise simple
  macro versions are defined here.

  USE_MEMCPY should be defined as 1 if you actually want to
  have memset and memcpy called. People report that the macro
  versions are often enough faster than libc versions on many
  systems that it is better to use them.

*/

#define HAVE_MEMCPY

#ifndef USE_MEMCPY
#ifdef HAVE_MEMCPY
#define USE_MEMCPY 1
#else
#define USE_MEMCPY 0
#endif
#endif

#if (__STD_C || defined(HAVE_MEMCPY))

#if __STD_C
void* memset(void*, int, size_t);
void* memcpy(void*, const void*, size_t);
#else
#ifdef WIN32
/* On Win32 platforms, 'memset()' and 'memcpy()' are already declared in */
/* 'windows.h' */
#else
Void_t* memset();
Void_t* memcpy();
#endif
#endif
#endif

#if USE_MEMCPY

/* The following macros are only invoked with (2n+1)-multiples of
   INTERNAL_SIZE_T units, with a positive integer n. This is exploited
   for fast inline execution when n is small. */

#define MALLOC_ZERO(charp, nbytes)                                            \
do {                                                                          \
  INTERNAL_SIZE_T mzsz = (nbytes);                                            \
  if(mzsz <= 9*sizeof(mzsz)) {                                                \
    INTERNAL_SIZE_T* mz = (INTERNAL_SIZE_T*) (charp);                         \
    if(mzsz >= 5*sizeof(mzsz)) {     *mz++ = 0;                               \
				     *mz++ = 0;                               \
      if(mzsz >= 7*sizeof(mzsz)) {   *mz++ = 0;                               \
				     *mz++ = 0;                               \
	if(mzsz >= 9*sizeof(mzsz)) { *mz++ = 0;                               \
				     *mz++ = 0; }}}                           \
				     *mz++ = 0;                               \
				     *mz++ = 0;                               \
				     *mz   = 0;                               \
  } else memset((charp), 0, mzsz);                                            \
} while(0)

#define MALLOC_COPY(dest,src,nbytes)                                          \
do {                                                                          \
  INTERNAL_SIZE_T mcsz = (nbytes);                                            \
  if(mcsz <= 9*sizeof(mcsz)) {                                                \
    INTERNAL_SIZE_T* mcsrc = (INTERNAL_SIZE_T*) (src);                        \
    INTERNAL_SIZE_T* mcdst = (INTERNAL_SIZE_T*) (dest);                       \
    if(mcsz >= 5*sizeof(mcsz)) {     *mcdst++ = *mcsrc++;                     \
				     *mcdst++ = *mcsrc++;                     \
      if(mcsz >= 7*sizeof(mcsz)) {   *mcdst++ = *mcsrc++;                     \
				     *mcdst++ = *mcsrc++;                     \
	if(mcsz >= 9*sizeof(mcsz)) { *mcdst++ = *mcsrc++;                     \
				     *mcdst++ = *mcsrc++; }}}                 \
				     *mcdst++ = *mcsrc++;                     \
				     *mcdst++ = *mcsrc++;                     \
				     *mcdst   = *mcsrc  ;                     \
  } else memcpy(dest, src, mcsz);                                             \
} while(0)

#else /* !USE_MEMCPY */

/* Use Duff's device for good zeroing/copying performance. */

#define MALLOC_ZERO(charp, nbytes)                                            \
do {                                                                          \
  INTERNAL_SIZE_T* mzp = (INTERNAL_SIZE_T*)(charp);                           \
  long mctmp = (nbytes)/sizeof(INTERNAL_SIZE_T), mcn;                         \
  if (mctmp < 8) mcn = 0; else { mcn = (mctmp-1)/8; mctmp %= 8; }             \
  switch (mctmp) {                                                            \
    case 0: for(;;) { *mzp++ = 0;                                             \
    case 7:           *mzp++ = 0;                                             \
    case 6:           *mzp++ = 0;                                             \
    case 5:           *mzp++ = 0;                                             \
    case 4:           *mzp++ = 0;                                             \
    case 3:           *mzp++ = 0;                                             \
    case 2:           *mzp++ = 0;                                             \
    case 1:           *mzp++ = 0; if(mcn <= 0) break; mcn--; }                \
  }                                                                           \
} while(0)

#define MALLOC_COPY(dest,src,nbytes)                                          \
do {                                                                          \
  INTERNAL_SIZE_T* mcsrc = (INTERNAL_SIZE_T*) src;                            \
  INTERNAL_SIZE_T* mcdst = (INTERNAL_SIZE_T*) dest;                           \
  long mctmp = (nbytes)/sizeof(INTERNAL_SIZE_T), mcn;                         \
  if (mctmp < 8) mcn = 0; else { mcn = (mctmp-1)/8; mctmp %= 8; }             \
  switch (mctmp) {                                                            \
    case 0: for(;;) { *mcdst++ = *mcsrc++;                                    \
    case 7:           *mcdst++ = *mcsrc++;                                    \
    case 6:           *mcdst++ = *mcsrc++;                                    \
    case 5:           *mcdst++ = *mcsrc++;                                    \
    case 4:           *mcdst++ = *mcsrc++;                                    \
    case 3:           *mcdst++ = *mcsrc++;                                    \
    case 2:           *mcdst++ = *mcsrc++;                                    \
    case 1:           *mcdst++ = *mcsrc++; if(mcn <= 0) break; mcn--; }       \
  }                                                                           \
} while(0)

#endif


/*
  Define HAVE_MMAP to optionally make malloc() use mmap() to
  allocate very large blocks.  These will be returned to the
  operating system immediately after a free().
*/

/***
#ifndef HAVE_MMAP
#define HAVE_MMAP 1
#endif
***/
#undef	HAVE_MMAP	/* Not available for U-Boot */

/*
  Define HAVE_MREMAP to make realloc() use mremap() to re-allocate
  large blocks.  This is currently only possible on Linux with
  kernel versions newer than 1.3.77.
*/

/***
#ifndef HAVE_MREMAP
#ifdef INTERNAL_LINUX_C_LIB
#define HAVE_MREMAP 1
#else
#define HAVE_MREMAP 0
#endif
#endif
***/
#undef	HAVE_MREMAP	/* Not available for U-Boot */

#ifdef HAVE_MMAP

#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>

#if !defined(MAP_ANONYMOUS) && defined(MAP_ANON)
#define MAP_ANONYMOUS MAP_ANON
#endif

#endif /* HAVE_MMAP */

/*
  Access to system page size. To the extent possible, this malloc
  manages memory from the system in page-size units.

  The following mechanics for getpagesize were adapted from
  bsd/gnu getpagesize.h
*/

#define	LACKS_UNISTD_H	/* Shortcut for U-Boot */
#define	malloc_getpagesize	4096

#ifndef LACKS_UNISTD_H
#  include <unistd.h>
#endif

#ifndef malloc_getpagesize
#  ifdef _SC_PAGESIZE         /* some SVR4 systems omit an underscore */
#    ifndef _SC_PAGE_SIZE
#      define _SC_PAGE_SIZE _SC_PAGESIZE
#    endif
#  endif
#  ifdef _SC_PAGE_SIZE
#    define malloc_getpagesize sysconf(_SC_PAGE_SIZE)
#  else
#    if defined(BSD) || defined(DGUX) || defined(HAVE_GETPAGESIZE)
       extern size_t getpagesize();
#      define malloc_getpagesize getpagesize()
#    else
#      ifdef WIN32
#        define malloc_getpagesize (4096) /* TBD: Use 'GetSystemInfo' instead */
#      else
#        ifndef LACKS_SYS_PARAM_H
#          include <sys/param.h>
#        endif
#        ifdef EXEC_PAGESIZE
#          define malloc_getpagesize EXEC_PAGESIZE
#        else
#          ifdef NBPG
#            ifndef CLSIZE
#              define malloc_getpagesize NBPG
#            else
#              define malloc_getpagesize (NBPG * CLSIZE)
#            endif
#          else
#            ifdef NBPC
#              define malloc_getpagesize NBPC
#            else
#              ifdef PAGESIZE
#                define malloc_getpagesize PAGESIZE
#              else
#                define malloc_getpagesize (4096) /* just guess */
#              endif
#            endif
#          endif
#        endif
#      endif
#    endif
#  endif
#endif


/*

  This version of malloc supports the standard SVID/XPG mallinfo
  routine that returns a struct containing the same kind of
  information you can get from malloc_stats. It should work on
  any SVID/XPG compliant system that has a /usr/include/malloc.h
  defining struct mallinfo. (If you'd like to install such a thing
  yourself, cut out the preliminary declarations as described above
  and below and save them in a malloc.h file. But there's no
  compelling reason to bother to do this.)

  The main declaration needed is the mallinfo struct that is returned
  (by-copy) by mallinfo().  The SVID/XPG malloinfo struct contains a
  bunch of fields, most of which are not even meaningful in this
  version of malloc. Some of these fields are are instead filled by
  mallinfo() with other numbers that might possibly be of interest.

  HAVE_USR_INCLUDE_MALLOC_H should be set if you have a
  /usr/include/malloc.h file that includes a declaration of struct
  mallinfo.  If so, it is included; else an SVID2/XPG2 compliant
  version is declared below.  These must be precisely the same for
  mallinfo() to work.

*/

/* #define HAVE_USR_INCLUDE_MALLOC_H */

#ifdef HAVE_USR_INCLUDE_MALLOC_H
#include "/usr/include/malloc.h"
#else

/* SVID2/XPG mallinfo structure */

struct mallinfo {
  int arena;    /* total space allocated from system */
  int ordblks;  /* number of non-inuse chunks */
  int smblks;   /* unused -- always zero */
  int hblks;    /* number of mmapped regions */
  int hblkhd;   /* total space in mmapped regions */
  int usmblks;  /* unused -- always zero */
  int fsmblks;  /* unused -- always zero */
  int uordblks; /* total allocated space */
  int fordblks; /* total non-inuse space */
  int keepcost; /* top-most, releasable (via malloc_trim) space */
};

/* SVID2/XPG mallopt options */

#define M_MXFAST  1    /* UNUSED in this malloc */
#define M_NLBLKS  2    /* UNUSED in this malloc */
#define M_GRAIN   3    /* UNUSED in this malloc */
#define M_KEEP    4    /* UNUSED in this malloc */

#endif

/* mallopt options that actually do something */

#define M_TRIM_THRESHOLD    -1
#define M_TOP_PAD           -2
#define M_MMAP_THRESHOLD    -3
#define M_MMAP_MAX          -4


#ifndef DEFAULT_TRIM_THRESHOLD
#define DEFAULT_TRIM_THRESHOLD (128 * 1024)
#endif

/*
    M_TRIM_THRESHOLD is the maximum amount of unused top-most memory
      to keep before releasing via malloc_trim in free().

      Automatic trimming is mainly useful in long-lived programs.
      Because trimming via sbrk can be slow on some systems, and can
      sometimes be wasteful (in cases where programs immediately
      afterward allocate more large chunks) the value should be high
      enough so that your overall system performance would improve by
      releasing.

      The trim threshold and the mmap control parameters (see below)
      can be traded off with one another. Trimming and mmapping are
      two different ways of releasing unused memory back to the
      system. Between these two, it is often possible to keep
      system-level demands of a long-lived program down to a bare
      minimum. For example, in one test suite of sessions measuring
      the XF86 X server on Linux, using a trim threshold of 128K and a
      mmap threshold of 192K led to near-minimal long term resource
      consumption.

      If you are using this malloc in a long-lived program, it should
      pay to experiment with these values.  As a rough guide, you
      might set to a value close to the average size of a process
      (program) running on your system.  Releasing this much memory
      would allow such a process to run in memory.  Generally, it's
      worth it to tune for trimming rather tham memory mapping when a
      program undergoes phases where several large chunks are
      allocated and released in ways that can reuse each other's
      storage, perhaps mixed with phases where there are no such
      chunks at all.  And in well-behaved long-lived programs,
      controlling release of large blocks via trimming versus mapping
      is usually faster.

      However, in most programs, these parameters serve mainly as
      protection against the system-level effects of carrying around
      massive amounts of unneeded memory. Since frequent calls to
      sbrk, mmap, and munmap otherwise degrade performance, the default
      parameters are set to relatively high values that serve only as
      safeguards.

      The default trim value is high enough to cause trimming only in
      fairly extreme (by current memory consumption standards) cases.
      It must be greater than page size to have any useful effect.  To
      disable trimming completely, you can set to (unsigned long)(-1);


*/


#ifndef DEFAULT_TOP_PAD
#define DEFAULT_TOP_PAD        (0)
#endif

/*
    M_TOP_PAD is the amount of extra `padding' space to allocate or
      retain whenever sbrk is called. It is used in two ways internally:

      * When sbrk is called to extend the top of the arena to satisfy
	a new malloc request, this much padding is added to the sbrk
	request.

      * When malloc_trim is called automatically from free(),
	it is used as the `pad' argument.

      In both cases, the actual amount of padding is rounded
      so that the end of the arena is always a system page boundary.

      The main reason for using padding is to avoid calling sbrk so
      often. Having even a small pad greatly reduces the likelihood
      that nearly every malloc request during program start-up (or
      after trimming) will invoke sbrk, which needlessly wastes
      time.

      Automatic rounding-up to page-size units is normally sufficient
      to avoid measurable overhead, so the default is 0.  However, in
      systems where sbrk is relatively slow, it can pay to increase
      this value, at the expense of carrying around more memory than
      the program needs.

*/


#ifndef DEFAULT_MMAP_THRESHOLD
#define DEFAULT_MMAP_THRESHOLD (128 * 1024)
#endif

/*

    M_MMAP_THRESHOLD is the request size threshold for using mmap()
      to service a request. Requests of at least this size that cannot
      be allocated using already-existing space will be serviced via mmap.
      (If enough normal freed space already exists it is used instead.)

      Using mmap segregates relatively large chunks of memory so that
      they can be individually obtained and released from the host
      system. A request serviced through mmap is never reused by any
      other request (at least not directly; the system may just so
      happen to remap successive requests to the same locations).

      Segregating space in this way has the benefit that mmapped space
      can ALWAYS be individually released back to the system, which
      helps keep the system level memory demands of a long-lived
      program low. Mapped memory can never become `locked' between
      other chunks, as can happen with normally allocated chunks, which
      menas that even trimming via malloc_trim would not release them.

      However, it has the disadvantages that:

	 1. The space cannot be reclaimed, consolidated, and then
	    used to service later requests, as happens with normal chunks.
	 2. It can lead to more wastage because of mmap page alignment
	    requirements
	 3. It causes malloc performance to be more dependent on host
	    system memory management support routines which may vary in
	    implementation quality and may impose arbitrary
	    limitations. Generally, servicing a request via normal
	    malloc steps is faster than going through a system's mmap.

      All together, these considerations should lead you to use mmap
      only for relatively large requests.


*/


#ifndef DEFAULT_MMAP_MAX
#ifdef HAVE_MMAP
#define DEFAULT_MMAP_MAX       (64)
#else
#define DEFAULT_MMAP_MAX       (0)
#endif
#endif

/*
    M_MMAP_MAX is the maximum number of requests to simultaneously
      service using mmap. This parameter exists because:

	 1. Some systems have a limited number of internal tables for
	    use by mmap.
	 2. In most systems, overreliance on mmap can degrade overall
	    performance.
	 3. If a program allocates many large regions, it is probably
	    better off using normal sbrk-based allocation routines that
	    can reclaim and reallocate normal heap memory. Using a
	    small value allows transition into this mode after the
	    first few allocations.

      Setting to 0 disables all use of mmap.  If HAVE_MMAP is not set,
      the default value is 0, and attempts to set it to non-zero values
      in mallopt will fail.
*/


/*
    USE_DL_PREFIX will prefix all public routines with the string 'dl'.
      Useful to quickly avoid procedure declaration conflicts and linker
      symbol conflicts with existing memory allocation routines.

*/

/* #define USE_DL_PREFIX */


/*

  Special defines for linux libc

  Except when compiled using these special defines for Linux libc
  using weak aliases, this malloc is NOT designed to work in
  multithreaded applications.  No semaphores or other concurrency
  control are provided to ensure that multiple malloc or free calls
  don't run at the same time, which could be disasterous. A single
  semaphore could be used across malloc, realloc, and free (which is
  essentially the effect of the linux weak alias approach). It would
  be hard to obtain finer granularity.

*/


#ifdef INTERNAL_LINUX_C_LIB

#if __STD_C

Void_t * __default_morecore_init (ptrdiff_t);
Void_t *(*__morecore)(ptrdiff_t) = __default_morecore_init;

#else

Void_t * __default_morecore_init ();
Void_t *(*__morecore)() = __default_morecore_init;

#endif

#define MORECORE (*__morecore)
#define MORECORE_FAILURE 0
#define MORECORE_CLEARS 1

#else /* INTERNAL_LINUX_C_LIB */

#if __STD_C
extern Void_t*     sbrk(ptrdiff_t);
#else
extern Void_t*     sbrk();
#endif

#ifndef MORECORE
#define MORECORE sbrk
#endif

#ifndef MORECORE_FAILURE
#define MORECORE_FAILURE -1
#endif

#ifndef MORECORE_CLEARS
#define MORECORE_CLEARS 1
#endif

#endif /* INTERNAL_LINUX_C_LIB */

#if defined(INTERNAL_LINUX_C_LIB) && defined(__ELF__)

#define cALLOc		__libc_calloc
#define fREe		__libc_free
#define mALLOc		__libc_malloc
#define mEMALIGn	__libc_memalign
#define rEALLOc		__libc_realloc
#define vALLOc		__libc_valloc
#define pvALLOc		__libc_pvalloc
#define mALLINFo	__libc_mallinfo
#define mALLOPt		__libc_mallopt

#pragma weak calloc = __libc_calloc
#pragma weak free = __libc_free
#pragma weak cfree = __libc_free
#pragma weak malloc = __libc_malloc
#pragma weak memalign = __libc_memalign
#pragma weak realloc = __libc_realloc
#pragma weak valloc = __libc_valloc
#pragma weak pvalloc = __libc_pvalloc
#pragma weak mallinfo = __libc_mallinfo
#pragma weak mallopt = __libc_mallopt

#else

#if CONFIG_IS_ENABLED(SYS_MALLOC_SIMPLE)
#define malloc malloc_simple
#define realloc realloc_simple
#define memalign memalign_simple
static inline void free(void *ptr) {}
void *calloc(size_t nmemb, size_t size);
void *realloc_simple(void *ptr, size_t size);
void malloc_simple_info(void);
#else

# ifdef USE_DL_PREFIX
# define cALLOc		dlcalloc
# define fREe		dlfree
# define mALLOc		dlmalloc
# define mEMALIGn	dlmemalign
# define rEALLOc		dlrealloc
# define vALLOc		dlvalloc
# define pvALLOc		dlpvalloc
# define mALLINFo	dlmallinfo
# define mALLOPt		dlmallopt
# else /* USE_DL_PREFIX */
# define cALLOc		calloc
# define fREe		free
# define mALLOc		malloc
# define mEMALIGn	memalign
# define rEALLOc		realloc
# define vALLOc		valloc
# define pvALLOc		pvalloc
# define mALLINFo	mallinfo
# define mALLOPt		mallopt
# endif /* USE_DL_PREFIX */

#endif

/* Set up pre-relocation malloc() ready for use */
int initf_malloc(void);

/* Public routines */

/* Simple versions which can be used when space is tight */
void *malloc_simple(size_t size);
void *memalign_simple(size_t alignment, size_t bytes);

#pragma GCC visibility push(hidden)
# if __STD_C

Void_t* mALLOc(size_t);
void    fREe(Void_t*);
Void_t* rEALLOc(Void_t*, size_t);
Void_t* mEMALIGn(size_t, size_t);
Void_t* vALLOc(size_t);
Void_t* pvALLOc(size_t);
Void_t* cALLOc(size_t, size_t);
void    cfree(Void_t*);
int     malloc_trim(size_t);
size_t  malloc_usable_size(Void_t*);
void    malloc_stats(void);
int     mALLOPt(int, int);
struct mallinfo mALLINFo(void);
# else
Void_t* mALLOc();
void    fREe();
Void_t* rEALLOc();
Void_t* mEMALIGn();
Void_t* vALLOc();
Void_t* pvALLOc();
Void_t* cALLOc();
void    cfree();
int     malloc_trim();
size_t  malloc_usable_size();
void    malloc_stats();
int     mALLOPt();
struct mallinfo mALLINFo();
# endif
#endif
#pragma GCC visibility pop

/*
 * Begin and End of memory area for malloc(), and current "brk"
 */
extern ulong mem_malloc_start;
extern ulong mem_malloc_end;
extern ulong mem_malloc_brk;

void mem_malloc_init(ulong start, ulong size);

#ifdef __cplusplus
};  /* end of extern "C" */
#endif

#endif /* __MALLOC_H__ */
