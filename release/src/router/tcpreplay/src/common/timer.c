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

#include "timer.h"
#include "config.h"
#include <stdlib.h>

/* Miscellaneous timeval routines */

/* Divide tvs by div, storing the result in tvs */
void
timesdiv_float(struct timespec *tvs, float div)
{
    double interval;

    if (div == 0.0 || div == 1.0)
        return;

    interval = ((double)tvs->tv_sec * 1000000000.0 + (double)tvs->tv_nsec) / (double)div;
    tvs->tv_sec = (time_t)interval / (time_t)1000000000;
    tvs->tv_nsec = (time_t)interval - (tvs->tv_sec * 1000000000);
}

void
init_timestamp(timestamp_t *ctx)
{
    timerclear(ctx);
}
