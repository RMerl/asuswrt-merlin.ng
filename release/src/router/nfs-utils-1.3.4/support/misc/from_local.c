 /*
  * Check if an address belongs to the local system. Adapted from:
  * 
  * @(#)pmap_svc.c 1.32 91/03/11 Copyright 1984,1990 Sun Microsystems, Inc.
  * @(#)get_myaddress.c  2.1 88/07/29 4.0 RPCSRC.
  */

/*
 * Copyright (c) 2009, Sun Microsystems, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * - Redistributions of source code must retain the above copyright notice,
 *   this list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 * - Neither the name of Sun Microsystems, Inc. nor the names of its
 *   contributors may be used to endorse or promote products derived
 *   from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#if 0
static char sccsid[] = "@(#) from_local.c 1.3 96/05/31 15:52:57";
#endif

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <sys/types.h>
#include <sys/socket.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <string.h>

#include "sockaddr.h"
#include "tcpwrapper.h"
#include "xlog.h"

#ifndef TRUE
#define	TRUE	1
#define FALSE	0
#endif

#ifdef HAVE_GETIFADDRS

#include <ifaddrs.h>
#include <time.h>

/**
 * from_local - determine whether request comes from the local system
 * @sap: pointer to socket address to check
 *
 * With virtual hosting, each hardware network interface can have
 * multiple network addresses. On such machines the number of machine
 * addresses can be surprisingly large.
 *
 * We also expect the local network configuration to change over time,
 * so call getifaddrs(3) more than once, but not too often.
 *
 * Returns TRUE if the sockaddr contains an address of one of the local
 * network interfaces.  Otherwise FALSE is returned.
 */
int
from_local(const struct sockaddr *sap)
{
	static struct ifaddrs *ifaddr = NULL;
	static time_t last_update = 0;
	struct ifaddrs *ifa;
	unsigned int count;
	time_t now;

	if (time(&now) == ((time_t)-1)) {
		xlog(L_ERROR, "%s: time(2): %m", __func__);

		/* If we don't know what time it is, use the
		 * existing ifaddr list, if one exists  */
		now = last_update;
		if (ifaddr == NULL)
			now++;
	}
	if (now != last_update) {
		xlog(D_GENERAL, "%s: updating local if addr list", __func__);

		if (ifaddr)
			freeifaddrs(ifaddr);

		if (getifaddrs(&ifaddr) == -1) {
			xlog(L_ERROR, "%s: getifaddrs(3): %m", __func__);
			return FALSE;
		}

		last_update = now;
	}

	count = 0;
	for (ifa = ifaddr; ifa; ifa = ifa->ifa_next) {
		if ((ifa->ifa_flags & IFF_UP) &&
		    nfs_compare_sockaddr(sap, ifa->ifa_addr)) {
			xlog(D_GENERAL, "%s: incoming address matches "
					"local interface address", __func__);
			return TRUE;
		} else
			count++;
	}

	xlog(D_GENERAL, "%s: checked %u local if addrs; "
			"incoming address not found", __func__, count);
	return FALSE;
}

#else	/* !HAVE_GETIFADDRS */

static int num_local;
static int num_addrs;
static struct in_addr *addrs;

/* grow_addrs - extend list of local interface addresses */

static int grow_addrs(void)
{
    struct in_addr *new_addrs;
    int     new_num;

    /*
     * Keep the previous result if we run out of memory. The system would
     * really get hosed if we simply give up.
     */
    new_num = (addrs == 0) ? 1 : num_addrs + num_addrs;
    new_addrs = (struct in_addr *) malloc(sizeof(*addrs) * new_num);
    if (new_addrs == 0) {
	xlog_warn("%s: out of memory", __func__);
	return (0);
    } else {
	if (addrs != 0) {
	    memcpy((char *) new_addrs, (char *) addrs,
		   sizeof(*addrs) * num_addrs);
	    free((char *) addrs);
	}
	num_addrs = new_num;
	addrs = new_addrs;
	return (1);
    }
}

/* find_local - find all IP addresses for this host */
static int
find_local(void)
{
    struct ifconf ifc;
    struct ifreq ifreq;
    struct ifreq *ifr;
    struct ifreq *the_end;
    int     sock;
    char    buf[BUFSIZ];

    /*
     * Get list of network interfaces. We use a huge buffer to allow for the
     * presence of non-IP interfaces.
     */

    if ((sock = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
	xlog_warn("%s: socket(2): %m", __func__);
	return (0);
    }
    ifc.ifc_len = sizeof(buf);
    ifc.ifc_buf = buf;
    if (ioctl(sock, SIOCGIFCONF, (char *) &ifc) < 0) {
	xlog_warn("%s: ioctl(SIOCGIFCONF): %m", __func__);
	(void) close(sock);
	return (0);
    }
    /* Get IP address of each active IP network interface. */

    the_end = (struct ifreq *) (ifc.ifc_buf + ifc.ifc_len);
    num_local = 0;
    for (ifr = ifc.ifc_req; ifr < the_end; ifr++) {
	if (ifr->ifr_addr.sa_family == AF_INET) {	/* IP net interface */
	    ifreq = *ifr;
	    if (ioctl(sock, SIOCGIFFLAGS, (char *) &ifreq) < 0) {
		xlog_warn("%s: ioctl(SIOCGIFFLAGS): %m", __func__);
	    } else if (ifreq.ifr_flags & IFF_UP) {	/* active interface */
		if (ioctl(sock, SIOCGIFADDR, (char *) &ifreq) < 0) {
		    xlog_warn("%s: ioctl(SIOCGIFADDR): %m", __func__);
		} else {
		    if (num_local >= num_addrs)
			if (grow_addrs() == 0)
			    break;
		    addrs[num_local++] = ((struct sockaddr_in *)
					  & ifreq.ifr_addr)->sin_addr;
		}
	    }
	}
	/* Support for variable-length addresses. */
#ifdef HAS_SA_LEN
	ifr = (struct ifreq *) ((caddr_t) ifr
		      + ifr->ifr_addr.sa_len - sizeof(struct sockaddr));
#endif
    }
    (void) close(sock);
    return (num_local);
}

/**
 * from_local - determine whether request comes from the local system
 * @sap: pointer to socket address to check
 *
 * With virtual hosting, each hardware network interface can have
 * multiple network addresses. On such machines the number of machine
 * addresses can be surprisingly large.
 *
 * Returns TRUE if the sockaddr contains an address of one of the local
 * network interfaces.  Otherwise FALSE is returned.
 */
int
from_local(const struct sockaddr *sap)
{
    const struct sockaddr_in *addr = (const struct sockaddr_in *)sap;
    int     i;

    if (sap->sa_family != AF_INET)
	return (FALSE);

    if (addrs == 0 && find_local() == 0)
	xlog(L_ERROR, "Cannot find any active local network interfaces");

    for (i = 0; i < num_local; i++) {
	if (memcmp((char *) &(addr->sin_addr), (char *) &(addrs[i]),
		   sizeof(struct in_addr)) == 0)
	    return (TRUE);
    }
    return (FALSE);
}

#ifdef TEST

int main(void)
{
    int     i;

    find_local();
    for (i = 0; i < num_local; i++)
	printf("%s\n", inet_ntoa(addrs[i]));
}

#endif	/* TEST */

#endif	/* !HAVE_GETIFADDRS */
