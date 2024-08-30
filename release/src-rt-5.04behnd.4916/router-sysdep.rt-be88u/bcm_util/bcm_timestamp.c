/***********************************************************************
 *
 *  Copyright (c) 2020  Broadcom Ltd
 *  All Rights Reserved
 *
<:label-BRCM:2020:DUAL/GPL:standard

Unless you and Broadcom execute a separate written software license
agreement governing use of this software, this software is licensed
to you under the terms of the GNU General Public License version 2
(the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
with the following added to such license:

   As a special exception, the copyright holders of this software give
   you permission to link this software with independent modules, and
   to copy and distribute the resulting executable under terms of your
   choice, provided that you also meet, for each linked independent
   module, the terms and conditions of the license of that module.
   An independent module is a module which is not derived from this
   software.  The special exception does not apply to any modifications
   of the software.

Not withstanding the above, under no circumstances may you combine
this software in any way with any other Broadcom software provided
under a license other than the GPL, without Broadcom's express prior
written consent.

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

