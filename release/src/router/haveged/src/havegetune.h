/**
 ** Simple entropy harvester based upon the havege RNG
 **
 ** Copyright 2018-2022 Jirka Hladky hladky DOT jiri AT gmail DOT com
 ** Copyright 2009-2014 Gary Wuertz gary@issiweb.com
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
#ifndef HAVEGETUNE_H
#define HAVEGETUNE_H

#include "havegecollect.h"
/**
 * Some systems supply a maximum
 */
#ifndef FILENAME_MAX
#define FILENAME_MAX    256                   /* Max path length                        */
#endif
/**
 * Limits
 */
#define MAX_BIT_IDX     (256/BITS_PER_H_UINT) /* Size of resource bitmaps               */
#define MAX_CACHES      8                     /* Max cache types                        */
#define MAX_CPUS        8                     /* Max cpu types                          */
/**
 * Object used to represent a set of objects
 */
typedef struct {
  H_UINT       bits[MAX_BIT_IDX];
  int          msw;
  H_UINT       source;
} TOPO_MAP;
/**
 * Cache instance
 */
typedef struct {
   TOPO_MAP    cpuMap;                    /* what cpus have this cache                    */
   H_UINT      type;                      /* 'I'nstruction, 'D'ata, 'U'nified, 'T'race    */
   H_UINT      level;                     /* 0-15................                         */
   H_UINT      size;                      /* size in KB                                   */
} CACHE_INST;
/**
 * Sources for CACHE_INST TOPO_MAP
 */
#define  SRC_DEFAULT          0x00001
#define  SRC_PARAM            0x00002
#define  SRC_CPUID_AMD6       0x00004
#define  SRC_CPUID_AMD5       0x00008
#define  SRC_CPUID_INTEL2     0x00010
#define  SRC_CPUID_INTEL4     0x00020
#define  SRC_VFS_INDEX        0x00040
/**
 * CPU instance
 */
typedef struct {
   TOPO_MAP    cpuMap;                    /* what cpus have this config    */
   H_UINT      signature;                 /* processor signature           */
   H_UINT      flags;
   H_UINT      maxFn;
   H_UINT      maxFnx;
   char        vendor[16];
} CPU_INST;
/**
 * Sources for CPU_INST TOPO_MAP
 */
#define  SRC_CPUID_PRESENT    0x00100
#define  SRC_CPUID_HT         0x00200
#define  SRC_CPUID_AMD        0x00400
#define  SRC_CPUID_AMD8       0x00800
#define  SRC_CPUID_LEAFB      0x01000
#define  SRC_CPUID_LEAF4      0x02000
#define  SRC_VFS_STATUS       0x04000
#define  SRC_VFS_ONLINE       0x08000
#define  SRC_VFS_CPUINFO      0x10000
#define  SRC_VFS_CPUDIR       0x20000
/**
 * Size of representation fields
 */
#define  SZ_BUILDREP    32
#define  SZ_CPUREP      64
#define  SZ_CACHEREP    32
/**
 * The result of tuning
 */
typedef struct {
   char        *procfs;                   /* where proc is mounted      */
   char        *sysfs;                    /* where sys is mounted       */
   char        buildOpts[SZ_BUILDREP];    /* build options              */
   char        cpuOpts[SZ_CPUREP];        /* cpu options                */
   char        icacheOpts[SZ_CACHEREP];   /* icache options             */
   char        dcacheOpts[SZ_CACHEREP];   /* dcache options             */
   TOPO_MAP    pAllowed;                  /* allowed processors         */
   TOPO_MAP    pOnline;                   /* processors online          */
   TOPO_MAP    pCpuInfo;                  /* processors with info       */
   TOPO_MAP    pCacheInfo;                /* processors with cache info */
   TOPO_MAP    mAllowed;                  /* allowed memory             */
   H_UINT      a_cpu;                     /* suggested cpu              */
   H_UINT      i_tune;                    /* suggested i cache value    */
   H_UINT      d_tune;                    /* suggested d cache value    */
   int         ctCpu;                     /* number of cpu types        */
   int         ctCache;                   /* number of cache items      */
   CPU_INST    cpus[MAX_CPUS];            /* cpu instances              */
   CACHE_INST  caches[MAX_CACHES+2];      /* cache instances            */
} HOST_CFG;
/**
 * Tuning interface
 */
void        havege_tune(HOST_CFG *env, H_PARAMS *params);

#endif
