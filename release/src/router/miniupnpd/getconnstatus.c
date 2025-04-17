/* $Id: getconnstatus.c,v 1.7 2025/04/03 21:11:35 nanard Exp $ */
/* MiniUPnP project
 * http://miniupnp.free.fr/ or https://miniupnp.tuxfamily.org/
 * (c) 2011-2025 Thomas Bernard
 * This software is subject to the conditions detailed
 * in the LICENCE file provided within the distribution */

#include <stdio.h>
#include <sys/types.h>
#include <netinet/in.h>
#include "getconnstatus.h"
#include "getifaddr.h"

int
get_wan_connection_status(const char * ifname)
{
	char addr[INET_ADDRSTRLEN];
	int r;

	/*! \todo we need a better implementation here.
	 * I'm afraid it should be device specific */
	r = getifaddr(ifname, addr, INET_ADDRSTRLEN, NULL, NULL);
	return (r < 0) ? STATUS_DISCONNECTED : STATUS_CONNECTED;
}

const char *
get_wan_connection_status_str(const char * ifname)
{
	int status;
	const char * str = NULL;

	status = get_wan_connection_status(ifname);
	switch(status) {
	case 0:
		str = "Unconfigured";
		break;
	case 1:
		str = "Connecting";
		break;
	case 2:
		str = "Connected";
		break;
	case 3:
		str = "PendingDisconnect";
		break;
	case 4:
		str = "Disconnecting";
		break;
	case 5:
		str = "Disconnected";
		break;
	}
	return str;
}
