/***********************************************************************
 *
 *  Copyright (c) 2020  Broadcom Ltd
 *  All Rights Reserved
 *
<:label-BRCM:2020:DUAL/GPL:standard

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License, version 2, as published by
the Free Software Foundation (the "GPL").

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.


A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.

:>
 *
 ************************************************************************/

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <dlfcn.h>

#include "bcm_ulog.h"
#include "bcm_timestamp.h"


static int timestamp_initialized = 0;
static int (*_bcm_clock_gettime_fn) (clockid_t, struct timespec *);

/* Work around to avoid glibc symbols being hijacked by libshared.so, which is
 * a library in wifi userspace code. The correct way would be modify wifi code
 * to remove those redefinition of glibc functions. Until that happens, we use
 * dlsym to force symbol resole to glibc.
 */
int bcm_libc_clock_gettime(clockid_t clkid, struct timespec *ts)
{
    if (timestamp_initialized == 0)
    {
        void *handle, *p;

        handle = dlopen ("libc.so.6", RTLD_LAZY);
        if (handle != NULL)
        {
            p = dlsym(handle, "clock_gettime");
            if (p != NULL)
            {
                _bcm_clock_gettime_fn = p;
            }
        }
        timestamp_initialized = 1;
    /* dlclose closes the shared library (assuming it is the only reference to it), 
     * which means that the OS will unmap the shared library, 
     * and dlsym(handle, "clock_gettime") -- _bcm_clock_gettime_fn is most likely no longer in memory. */
    /* coverity[leaked_storage] */
    }

    if (_bcm_clock_gettime_fn != NULL)
    {
        return (*_bcm_clock_gettime_fn)(clkid, ts);
    }
    else
    {
        return clock_gettime(clkid, ts);
    }
}


void bcmTms_getISO8601DateTimeGmt(time_t t, char *buf, size_t bufLen)
{
    return (bcmTms_getXSIDateTimeGmt(t, buf, bufLen));
}

void bcmTms_getXSIDateTimeGmt(time_t t, char *buf, size_t bufLen)
{
    struct tm gmt_tm;
    size_t s;

    if (buf == NULL || bufLen == 0)
    {
        return;
    }

    if (t == 0)
    {
        // caller did not supply the time, so get current time.
        time(&t);
    }

    memset(buf, 0, bufLen);
    memset(&gmt_tm, 0, sizeof(gmt_tm));
    gmtime_r(&t, &gmt_tm);
    s = strftime(buf, bufLen, "%Y-%m-%dT%H:%M:%SZ", &gmt_tm);
    if (s == 0)
    {
        bcmuLog_error("bufLen=%zu is too short!", bufLen);
    }

    return;
}

