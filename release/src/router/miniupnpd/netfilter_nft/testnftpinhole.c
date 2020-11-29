/* $Id: testnftpinhole.c,v 1.2 2019/06/30 19:49:18 nanard Exp $ */
/* MiniUPnP project
 * http://miniupnp.free.fr/ or http://miniupnp.tuxfamily.org/
 * (c) 2012-2020 Thomas Bernard
 * This software is subject to the conditions detailed
 * in the LICENCE file provided within the distribution */

#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <syslog.h>

#include "config.h"
#include "../miniupnpdtypes.h"
#include "nftpinhole.h"
#include "../commonrdr.h"
#include "../upnputils.h"

struct lan_addr_list lan_addrs;
time_t startup_time = 0;

int main(int argc, char * * argv)
{
	int uid;

	openlog("testnftpinhole", LOG_PERROR|LOG_CONS, LOG_LOCAL0);

	uid = add_pinhole("eth0", NULL, 0, "ff::123", 54321, IPPROTO_TCP,
	                  "dummy description", upnp_time() + 60 /* timestamp */);
	syslog(LOG_INFO, "uid=%d", uid);
	return 0;
}

