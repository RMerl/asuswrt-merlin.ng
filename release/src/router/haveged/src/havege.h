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
#ifndef HAVEGE_H
#define HAVEGE_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif
/**
 * header/package version as a numeric major, minor, patch triple. See havege_version()
 * below for usage.
 */
#define  HAVEGE_PREP_VERSION  "1.9.18"
/**
 * Basic types
 */
#define  H_UINT   uint32_t
#define  H_UINT8  uint8_t
/**
 * Optional metering call-back. Called with event=0 at start of collection buffer fill.
 * Called with event=1 at end of collection buffer fill. The nCollect parameter indicates
 * the calling process if multiple collection processes are enabled. Use a value of 0
 * to disable metering.
 */
typedef void (*pMeter)(H_UINT nCollect, H_UINT event);
/**
 * Optional message display call-back. This printf style method is used for all diagnostic
 * output including havege_status(). Use a value of 0 to disable output.
 */
typedef void (*pMsg)(const char *format, ...);
/**
 * Injection call-back for RAW diagnostics. Use a value of 0 to disable diagnostic. Ignored
 * except for diaqnotic builds.
 */
typedef int (*pRawIn)(volatile H_UINT *pData, H_UINT szData);
/**
 * options for H_PARAMS below. Lower byte transferred from verbose settings
 * upper byte set by diagnositic run options
 */
#define H_VERBOSE         0x001           /* deprecated from ver 1.7       */
#define H_DEBUG_INFO      0x001           /* Show config info, retries     */
#define H_DEBUG_OLTR      0x002           /* Show detailed test retry info */
#define H_DEBUG_TIME      0x004           /* Show collection times         */
#define H_DEBUG_LOOP      0x008           /* Show loop parameters          */
#define H_DEBUG_COMPILE   0x010           /* Show assembly info            */
#define H_DEBUG_OLT       0x020           /* Show all test info            */
#define H_RNDADDENTROPY_INFO 0x040        /* RNDADDENTROPY info            */

#define H_DEBUG_RAW_OUT   0x100           /* diagnostic output             */
#define H_DEBUG_RAW_IN    0x200           /* diagnostic input              */
#define H_DEBUG_TEST_IN   0x400           /* input test data               */

/**
 * Initialization parameters. Use non-zero values to override default values.
 * Notes:
 *
 * 1) Correspondence between provided value and value of H_PTR members are:
 *    ioSz <==> i_readSz, collectSize <==> i_collectSz, nCores <==> n_cores,
 *    options <==> havege_opts
 * 2) ioSz is specified in bytes. collectSize sizes is specified as number
 *    of H_UINT. The default for ioSz is 1024*sizeof(H_UINT). The default
 *    for collecSize is 128K * sizeof(H_UINT).
 * 3) The icacheSize and dcacheSize override cache sizes. Both are specified in KB.
 *    Either may be specified to override the tuning value. If both are provided,
 *    tuning code is bypassed. The fallback tuning values can be overridden
 *    by defining GENERIC_DCACHE and GENERIC_ICACHE (16 will be used if not
 *    otherwise defined)
 * 4) null callback values suppress the function.
 * 5) sysFs default is '/sys', procFs default is '/proc'.
 * 6) testSpec same as haveged option "[t<x>][c<x>] x=[a[n][w]][b[w]]". If
 *    not specified (NULL) the default is "ta8b" - i.e. run the tot tests
 */
typedef struct {
   H_UINT      ioSz;                      /* size of write buffer          */
   H_UINT      collectSize;               /* size of collection buffer     */
   H_UINT      icacheSize;                /* Instruction cache size        */
   H_UINT      dcacheSize;                /* Data cache size               */
   H_UINT      options;                   /* Other options                 */
   H_UINT      nCores;                    /* If multi-core                 */
   pMeter      metering;                  /* meterming method              */
   pMsg        msg_out;                   /* output display method         */
   pRawIn      injection;                 /* injection  method             */
   char        *procFs;                   /* proc mount point override     */
   char        *sysFs;                    /* sys mount point override      */
   char        *testSpec;                 /* test specification            */
} H_PARAMS;
/**
 * Status codes used in the error member of h_anchor
 */
typedef enum {
   H_NOERR,                               /* 00 No error                         */
   H_NOHANDLE,                            /* 01 No memory for handle             */
   H_NOBUF,                               /* 02 Output buffer allocation failed  */
   H_NOINIT,                              /* 03 semaphore init failed            */
   H_NOCOLLECT,                           /* 04 H_COLLECT allocation failed      */
   H_NOWALK,                              /* 05 Walk buffer allocation failed    */
   H_NOTESTSPEC,                          /* 06 invalid test specification       */
   H_NOTESTINIT,                          /* 07 test setup failed                */
   H_NOTESTMEM,                           /* 08 Unable to allocate test memory   */
   H_NOTESTTOT,                           /* 09 tot test failed                  */
   H_NOTESTRUN,                           /* 10 production test failed           */
   H_NOCORES,                             /* 11 too many cores specified         */
   H_NOTASK,                              /* 12 Unable to create child task      */
   H_NOWAIT,                              /* 13 sem_wait failed                  */
   H_NOPOST,                              /* 14 sem_post failed                  */
   H_NODONE,                              /* 15 sem_post done failed             */
   H_NORQST,                              /* 16 sem_post request failed          */
   H_NOCOMP,                              /* 17 wait for completion failed       */
   H_EXIT,                                /* 18 Exit signal                      */
   H_NOTIMER                              /* 19 timer failed                     */
} H_ERR;
/**
 * Keep compiler honest
 */
typedef volatile void *H_VOL;
/**
 * Anchor for the RNG. Should be read only at devel level and above.
 */
typedef struct h_anchor {
   H_UINT      *io_buf;                   /* output buffer                    */
   char        *arch;                     /* "x86","sparc","ppc","ia64",etc   */
   void        *cpu;                      /* information on the cpu           */
   void        *instCache;                /* instruction cache info           */
   void        *dataCache;                /* data cache info                  */
   pMsg        print_msg;                 /* output display method            */
   pMeter      metering;                  /* metering method                  */
   pRawIn      inject;                    /* Injection diagnostic only        */
   H_VOL       collector;                 /* single thread collector          */
   H_VOL       threads;                   /* multi thread collectors          */
   void        *testData;                 /* online test data                 */
   void        *tuneData;                 /* tuning data                      */
   H_UINT      error;                     /* H_ERR enum for status            */
   H_UINT      havege_opts;               /* option flags                     */
   H_UINT      i_maxidx;                  /* maximum instruction loop index   */
   H_UINT      i_maxsz;                   /* maximum code size                */
   H_UINT      i_idx;                     /* code index used                  */
   H_UINT      i_sz;                      /* code size used                   */
   H_UINT      i_collectSz;               /* size of collection buffer        */
   H_UINT      i_readSz;                  /* size of read buffer (bytes)      */
   H_UINT      m_sz;                      /* size of thread ipc area (bytes)  */
   H_UINT      n_cores;                   /* number of cores                  */
   H_UINT      n_fills;                   /* number of buffer fills           */
   size_t      n_entropy_bytes;           /* total amount of entropy (byte)   */
} *H_PTR;
/**
 * Fail/Success counters for tot and production tests.
 */
typedef enum {
   H_OLT_TOT_A_F,       /* tot Procedure A failed   */
   H_OLT_TOT_A_P,       /* tot Procedure A passed   */
   H_OLT_TOT_B_F,       /* tot Procedure B failed   */
   H_OLT_TOT_B_P,       /* tot Procedure B passed   */
   H_OLT_PROD_A_F,      /* prod Procedure A failed  */
   H_OLT_PROD_A_P,      /* prod Procedure A passed  */
   H_OLT_PROD_B_F,      /* prod Procedure B failed  */
   H_OLT_PROD_B_P       /* prod Procedure B passed  */
} H_OLT_METERS;
/**
 * Structure used to query RNG anchor settings for information not exposed by
 * H_PTR. List formats are strings with one or more tokens separated by space.
 * Sources lists show how tuning parameters are derived. D is a build default,
 * P is a run time override, items V* come from linux virtual file system,
 * other items trace various cpuid sources. Tuning is skipped if both cache
 * sizes have 'P' sources.
 * 
 * Notes:
 *
 *    1) Build: package version of source
 *    2) Build options: compiler version followed by build configuration encoded
 *       as string of: [C][I][M][T][V] where  C=clock_gettime, I=tune with cpuid,
 *       M=multi-core, T=online-test, V=tune with vfs
 *    3) Tuning source lists: D=default, P=parameter, C=cpuid present,
 *          H=hyperthreading, A=AMD cpuid, A5=AMD fn5, A6=AMD fn6, A8=AMD fn8
 *          L2=Intel has leaf2, L4=Intel has leaf4, B=Intel leaf b,
 *          4=intel leaf4, V=virtual file system available
 *          VS=/sys/devices/system/cpu/cpu%d/cache/index<n>/level,
 *          VO=/sys/devices/system/cpu/online, VI=/proc/cpuinfo
 *          VC=/sys/devices/system/cpu
 *    4) test spec [A[1..8]][B], see H_PARAMS above.
 *    5) zero unless tests are enabled
 *    6) Last Coron's entropy estimate from Procedure B, test 8
 */
typedef struct h_status {
   const char    *version;                   /* Package version [1]             */
   const char    *buildOptions;              /* Options [2]                     */
   const char    *vendor;                    /* cpuid machines only             */
   const char    *cpuSources;                /* Tuning sources list   [3]       */
   const char    *i_cacheSources;            /* Tuning sources list   [3]       */
   const char    *d_cacheSources;            /* Tuning sources list   [3]       */
   const char    *tot_tests;                 /* tot test spec [4]               */
   const char    *prod_tests;                /* prod test spec [4]              */
   H_UINT        i_cache;                    /* size of L1 instruction cache KB */
   H_UINT        d_cache;                    /* size of L1 data cache KB        */
   H_UINT        n_tests[H_OLT_PROD_B_P+1];  /* test statistics [5]             */
   double        last_test8;                 /* last test8 result [6]           */
} *H_STATUS;
/**
 * Standard presentation formats for havege_status_dump.
 */
typedef enum {
   H_SD_TOPIC_BUILD,
/* ver: %s; arch: %s; vend: %s; build: (%s); collect: %dK */
   H_SD_TOPIC_TUNE,
/* cpu: (%s); data: %dK (%s); inst: %dK (%s); idx: %d/%d; sz: %d/%d */
   H_SD_TOPIC_TEST,
/* [tot tests (%s): A:%d/%d B: %d/%d;][continuous tests (%s): A:%d/%d B: %d/%d;][last entropy estimate %g] */
   H_SD_TOPIC_SUM,
/* fills: %d, generated: %.4g %c bytes */
} H_SD_TOPIC;
/**
 * Public prototypes. Library users note that "havege_*" is reserved for library
 * public functions. Note that the multi-core option is experimental and must
 * enabled in the build.
 */
/**
 * Create an anchor. The caller should check for a non-null return value with
 * a error value of H_NOERR. Any non-null return should be disposed of by a
 * call to havege_destroy() to free all allocated resources.
 *
 * Possible error values: H_NOERR, H_NOTESTSPEC, H_NOBUF, H_NOTESTMEM,
 *                        H_NOINIT
 */
H_PTR       havege_create(H_PARAMS *params);

/**
 * haveger_create() remembers parent pid and uses it to identify deallocating thread.
 * daemonize() forks parent and effectively loses parent thread.
 * havege_reparent(void) allows recovering new parent pid before havege_run() is started.
 */
void        havege_reparent(H_PTR hptr);

/**
 * Frees all allocated anchor resources. If the multi-core option is used, this
 * method should be called from a signal handler to prevent zombie processes.
 * If called by the process that called haveged_create(), hptr will be freed
 * when all child processes (if any) have terminated. If called by a child
 * process, H_EXIT will be set and all children awakened to exit.
 */
void        havege_destroy(H_PTR hptr);
/**
 * Read random words from an active anchor. The RNG must have been readied
 * by a previous call to havege_run(). The read must take place within the
 * allocated buffer, hptr->io_buf, and the range is specified in number of
 * H_UINT to read. If the multi-core option is used, this buffer is
 * memory-mapped between collectors.
 *
 * Returns the number of H_UINT read.
 * 
 * Possible error values: H_NOERR, H_NOTESRUN, H_NOPOST, H_NODONE,
 *                        H_NORQST, H_NOCOMP, H_EXIT
 */
int         havege_rng(H_PTR hptr, H_UINT *buf, H_UINT sz);
/**
 * Warm up the RNG and run the start-up tests. The operation suceeded if the
 * error member of the handle is H_NOERR. A failed handle should be disposed
 * of by a call to havege_destroy().
 *
 *  Returns non-zero on failure.
 *
 *  Possible error values: H_NOERR, H_NOCOLLECT, H_NOWALK, H_NOTESTMEM,
 *                         H_NOTASK, H_NOTESTTOT, H_NOWAIT,
 *                         any havege_rng error
 */
int         havege_run(H_PTR hptr);
/**
 * Fill in the h_status structure with read-only information collected from
 * the package build, run-time tuning, and test components.
 */
void        havege_status(H_PTR hptr, H_STATUS hsts);
/**
 * Call havege_status() and generate a standard presentation of H_STATUS content.
 * See the H_SD_TOPIC enum above for the formats.
 *
 * Returns the number of bytes placed in buf.
 */
int         havege_status_dump(H_PTR hptr, H_SD_TOPIC topic, char *buf, size_t len);
/**
 * Return/check library prep version. Calling havege_version() with a NULL version
 * returns the definition of HAVEGE_PREP_VERSION used to build the library. Calling
 * with HAVEGE_PREP_VERSION as the version checks if this headers definition is
 * compatible with that of the library, returning NULL if the input is incompatible
 * with the library. 
 */
const char *havege_version(const char *version);

#ifdef __cplusplus
}
#endif

#endif
