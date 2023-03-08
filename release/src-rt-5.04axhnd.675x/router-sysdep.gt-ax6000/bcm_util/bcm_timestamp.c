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
#include <time.h>
#include <dlfcn.h>

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
