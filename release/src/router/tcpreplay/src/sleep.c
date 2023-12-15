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

#include "sleep.h"
#include "config.h"
#include "common.h"
#include <string.h>
#include <sys/time.h>

#ifdef HAVE_SYS_EVENT
#include <sys/event.h>
#endif

/* necessary for ioport_sleep() functions */
#if defined HAVE_IOPORT_SLEEP__ && defined HAVE_SYS_IO_H /* Linux */
#include <sys/io.h>
#elif defined HAVE_ARCHITECTURE_I386_PIO_H /* OS X */
#include <architecture/i386/pio.h>
#endif

#if defined HAVE_IOPORT_SLEEP__
static int ioport_sleep_value;
#endif

void
ioport_sleep_init(void)
{
#if defined HAVE_IOPORT_SLEEP__
    ioperm(0x80, 1, 1);
    ioport_sleep_value = inb(0x80);
#else
    err(-1, "Platform does not support IO Port for timing");
#endif
}

void
ioport_sleep(sendpacket_t *sp _U_, const struct timespec *nap _U_, struct timeval *now _U_, bool flush _U_)
{
#if defined HAVE_IOPORT_SLEEP__
    struct timeval nap_for;
    u_int32_t usec;
    time_t i;

    TIMESPEC_TO_TIMEVAL(&nap_for, nap);

    /*
     * process the seconds, we do this in a loop so we don't have to
     * use slower 64bit integers or worry about integer overflows.
     */
    for (i = 0; i < nap_for.tv_sec; i++) {
        usec = SEC_TO_MICROSEC(nap_for.tv_sec);
        while (usec > 0) {
            usec--;
            outb(ioport_sleep_value, 0x80);
        }
    }

    /* process the usec */
    usec = nap->tv_nsec / 1000;
    usec--; /* fudge factor for all the above */
    while (usec > 0) {
        usec--;
        outb(ioport_sleep_value, 0x80);
    }
#else
    err(-1, "Platform does not support IO Port for timing");
#endif
}
