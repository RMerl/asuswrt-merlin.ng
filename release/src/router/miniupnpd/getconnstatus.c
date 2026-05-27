/* $Id: getconnstatus.c,v 1.8 2025/04/08 21:28:42 nanard Exp $ */
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

/**
 * Only #STATUS_UNCONFIGURED, #STATUS_DISCONNECTED and #STATUS_CONNECTED
 */
int
get_wan_connection_status(const char * ifname)
{
	switch(getifaddr(ifname, NULL, 0, NULL, NULL)) {
	case GETIFADDR_OK:
		return STATUS_CONNECTED;
	case GETIFADDR_NO_ADDRESS:
	case GETIFADDR_IF_DOWN:
		return STATUS_DISCONNECTED;
	default:
		return STATUS_UNCONFIGURED;
	}
}

const char *
get_wan_connection_status_str(const char * ifname)
{
	int status;
	const char * str = NULL;

	status = get_wan_connection_status(ifname);
	switch(status) {
	case STATUS_UNCONFIGURED:
		str = "Unconfigured";
		break;
	case STATUS_CONNECTING:
		str = "Connecting";
		break;
	case STATUS_CONNECTED:
		str = "Connected";
		break;
	case STATUS_PENDINGDISCONNECT:
		str = "PendingDisconnect";
		break;
	case STATUS_DISCONNECTING:
		str = "Disconnecting";
		break;
	case STATUS_DISCONNECTED:
		str = "Disconnected";
		break;
	}
	return str;
}
