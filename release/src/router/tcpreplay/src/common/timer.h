/* $Id$ */

/*
 *   Copyright (c) 2001-2010 Aaron Turner <aturner at synfin dot net>
 *   Copyright (c) 2013-2022 Fred Klassen <tcpreplay at appneta dot com> - AppNeta
 *
 *   The Tcpreplay Suite of tools is free software: you can redistribute it
 *   and/or modify it under the terms of the GNU General Public License as
 *   published by the Free Software Foundation, either version 3 of the
 *   License, or with the authors permission any later version.
 *
 *   The Tcpreplay Suite is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with the Tcpreplay Suite.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include "defines.h"
#include "config.h"
#include "common.h"
#include "tcpreplay.h"
#include <math.h>
#include <sys/time.h>
#include <time.h>

/*
 * 1 sec = 1,0000 millisec (ms)
 * 1 sec = 1,000,000 microsec (us)
 * 1 sec = 1,000,000,000 nanosec (ns)
 * 1 millisec = 1,000 microsec
 * 1 microsec = 1,000 nanosec
 */

void timesdiv_float(struct timespec *tvs, float div);

/* convert float time to struct timeval *tvp */
#ifndef float2timer
#define float2timer(time, tvp)                                                                                         \
    do {                                                                                                               \
        (tvp)->tv_sec = time;                                                                                          \
        (tvp)->tv_usec = (time - (tvp)->tv_sec) * 100000;                                                              \
    } while (0)
#endif

/* timesec to float */
#ifndef timer2float
#define timer2float(tvp, time)                                                                                         \
    do {                                                                                                               \
        time = (tvp)->tv_sec;                                                                                          \
        time += (float)((tvp)->tv_usec / 10000) * 0.01;                                                                \
    } while (0)
#endif

#ifndef TIMEVAL_TO_TIMESPEC
#define TIMEVAL_TO_TIMESPEC(tv, ts)                                                                                    \
    {                                                                                                                  \
        (ts)->tv_sec = (tv)->tv_sec;                                                                                   \
        (ts)->tv_nsec = (tv)->tv_usec * 1000;                                                                          \
    }
#endif

#ifndef TIMESPEC_TO_TIMEVAL
#define TIMESPEC_TO_TIMEVAL(tv, ts)                                                                                    \
    {                                                                                                                  \
        (tv)->tv_sec = (ts)->tv_sec;                                                                                   \
        (tv)->tv_usec = (ts)->tv_nsec / 1000;                                                                          \
    }
#endif

#ifndef ROUND_TIMESPEC_TO_MICROSEC
#define ROUND_TIMESPEC_TO_MICROSEC(ts)                                                                                 \
    do {                                                                                                               \
        (ts)->tv_nsec = ((((ts)->tv_nsec / 1000) + ((ts)->tv_nsec % 1000 >= 500 ? 1 : 0)) * 1000);                     \
    } while (0)
#endif

/* zero out a timer */
#ifndef timerclear
#define timerclear(tvp) (tvp)->tv_sec = (tvp)->tv_usec = 0
#endif

/* zero out a timespec */
#ifndef timesclear
#define timesclear(tvs) (tvs)->tv_sec = (tvs)->tv_nsec = 0
#endif

/* is timer non-zero? */
#ifndef timerisset
#define timerisset(tvp) ((tvp)->tv_sec || (tvp)->tv_usec)
#endif

/* is timespec non-zero? */
#ifndef timesisset
#define timesisset(tvs) ((tvs)->tv_sec || (tvs)->tv_nsec)
#endif

/* add tvp and uvp and store in vvp */
#ifndef timeradd
#define timeradd(tvp, uvp, vvp)                                                                                        \
    do {                                                                                                               \
        (vvp)->tv_sec = (tvp)->tv_sec + (uvp)->tv_sec;                                                                 \
        (vvp)->tv_usec = (tvp)->tv_usec + (uvp)->tv_usec;                                                              \
        if ((vvp)->tv_usec >= 1000000) {                                                                               \
            (vvp)->tv_sec++;                                                                                           \
            (vvp)->tv_usec -= 1000000;                                                                                 \
        }                                                                                                              \
    } while (0)
#endif

/* subtract uvp from tvp and store in vvp */
#ifndef timersub
#define timersub(tvp, uvp, vvp)                                                                                        \
    do {                                                                                                               \
        (vvp)->tv_sec = (tvp)->tv_sec - (uvp)->tv_sec;                                                                 \
        (vvp)->tv_usec = (tvp)->tv_usec - (uvp)->tv_usec;                                                              \
        if ((vvp)->tv_usec < 0) {                                                                                      \
            (vvp)->tv_sec--;                                                                                           \
            (vvp)->tv_usec += 1000000;                                                                                 \
        }                                                                                                              \
    } while (0)
#endif

#ifndef timessub
#define timessub(tsp, usp, vsp)                                                                                        \
    do {                                                                                                               \
        (vsp)->tv_sec = (tsp)->tv_sec - (usp)->tv_sec;                                                                 \
        (vsp)->tv_nsec = (tsp)->tv_nsec - (usp)->tv_nsec;                                                              \
        if ((vsp)->tv_nsec < 0) {                                                                                      \
            (vsp)->tv_sec--;                                                                                           \
            (vsp)->tv_nsec += 1000000000;                                                                              \
        }                                                                                                              \
    } while (0)
#endif

/* compare tvp and uvp using cmp */
#ifndef timercmp
#define timercmp(tvp, uvp, cmp)                                                                                        \
    (((tvp)->tv_sec == (uvp)->tv_sec) ? ((tvp)->tv_usec cmp(uvp)->tv_usec) : ((tvp)->tv_sec cmp(uvp)->tv_sec))
#endif

#ifndef timescmp
#define timescmp(tsp, usp, cmp)                                                                                        \
    (((tsp)->tv_sec == (usp)->tv_sec) ? ((tsp)->tv_nsec cmp(usp)->tv_nsec) : ((tsp)->tv_sec cmp(usp)->tv_sec))
#endif

typedef struct timeval timestamp_t;

void init_timestamp(timestamp_t *ctx);
