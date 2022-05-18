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
#ifndef HAVEGED_H
#define HAVEGED_H

#include "havege.h"
/**
 * Settings - default values declared in haveged.c
 */
struct pparams  {
   char           *daemon;          /* Daemon name - default is "haveged"           */
   H_UINT         exit_code;        /* Exit code                                    */
   H_UINT         setup;            /* setup options                                */
   H_UINT         ncores;           /* number of cores to use                       */
   H_UINT         buffersz;         /* size of collection buffer (kb)               */
   H_UINT         detached;         /* non-zero if daemonized                       */
   H_UINT         foreground;       /* non-zero if running in foreground            */
   H_UINT         once;             /* 1: refill entropy once and quit immediatelly */
   H_UINT         run_level;        /* type of run 0=daemon,1=setup,2=pip,sample kb */
   H_UINT         d_cache;          /* size of data cache (kb)                      */
   H_UINT         i_cache;          /* size of instruction cache (kb)               */
   H_UINT         low_water;        /* write threshold to set - 0 for none          */
   char           *tests_config;    /* online test configuration                    */
   char           *os_rel;          /* path to operating system release             */
   char           *pid_file;        /* name of pid file                             */
   char           *poolsize;        /* path to poolsize                             */
   char           *random_device;   /* path to random device                        */
   char           *sample_in;       /* input path for injection diagnostic          */
   char           *sample_out;      /* path to sample file                          */
   H_UINT         verbose;          /* Output level for log or stdout               */
   char           *version;         /* Our version                                  */
   char           *watermark;       /* path to write_wakeup_threshold               */
   char           *command;         /* command which will be send/received          */
  };
/**
 * Buffer size used when not running as daemon
 */
#define   APP_BUFF_SIZE    1024
#define   INPUT_DEFAULT    "data"
#define   OUTPUT_DEFAULT   "sample"
#define   PID_DEFAULT      "/var/run/haveged.pid"
/**
 * Setup options (for app)
 */
#define   RUN_AS_APP    0x001
#define   RANGE_SPEC    0x002
#define   USE_STDOUT    0x004
#define   CAPTURE       0x008
#define   INJECT        0x010
#define   RUN_IN_FG     0x020
#define   SET_LWM       0x040
#define   MULTI_CORE    0x080
#define   CMD_MODE      0x100
#define   RUN_ONCE      0x200
/**
 * Default tests settings
 */
#define  TESTS_DEFAULT_APP "ta8b"   /* startup tests                    */
#define  TESTS_DEFAULT_RUN "ta8bcb" /* startup + continuous B           */
/**
 * Run levels for diagnostic build
 */
#define  DIAG_RUN_CAPTURE  2        /* output clock ticks   */
#define  DIAG_RUN_INJECT   4        /* inject clock ticks   */
#define  DIAG_RUN_TEST     8        /* inject test data     */
/**
 * Status/monitoring information
 */
typedef struct {
   H_UINT         n_fill;           /* number times filled              */
   double         etime;            /* milliseconds for last collection */
   double         estart;           /* start time for calculation       */
} H_METER;

/**
  * Bail....
  */
void error_exit(const char *, ...);

/**
  * Execution notices - to stderr or syslog
  */
void print_msg(const char *, ...);

#endif
