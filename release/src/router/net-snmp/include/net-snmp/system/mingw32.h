/*
 * The Win32 API is maintained by the MinGW developers.
 * The licensing for the Win32 API is defined in the
 * MinGW file README.w32api, which states:

   "Unless otherwise stated in individual source files,

    THIS SOFTWARE IS NOT COPYRIGHTED

    This source code is offered for use in the public domain.  You may use,
    modify or distribute it freely.

    This source code is distributed in the hope that it will be useful but
    WITHOUT ANY WARRANTY.  ALL WARRANTIES, EXPRESS OR IMPLIED ARE HEREBY
    DISCLAIMED.  This includes but is not limited to warranties of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE."
 
 */

/*
 * Make sure that the Winsock header files get included before any <sys/...>
 * header files to avoid that the build fails as follows:
 *   In file included from /usr/include/w32api/winsock2.h:56:0,
 *                    from ../include/net-snmp/types.h:24,
 *                    from snmp_client.c:87:
 *   /usr/include/w32api/psdk_inc/_fd_types.h:100:2: warning:  *warning "fd_set and associated macros have been defined in sys/types.      This can cause runtime problems with W32 sockets" [-Wcpp]
 *   #warning "fd_set and associated macros have been defined in sys/types.
 */
#include <winsock2.h>
#include <ws2tcpip.h>

#include <net-snmp/system/generic.h>
#undef bsdlike
#undef MBSTAT_SYMBOL
#undef TOTAL_MEMORY_SYMBOL
