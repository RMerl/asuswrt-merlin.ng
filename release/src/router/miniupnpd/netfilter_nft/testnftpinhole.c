/* $Id: testnftpinhole.c,v 1.2 2019/06/30 19:49:18 nanard Exp $ */
/* MiniUPnP project
 * http://miniupnp.free.fr/ or http://miniupnp.tuxfamily.org/
 * (c) 2012-2019 Thomas Bernard
 * This software is subject to the conditions detailed
 * in the LICENCE file provided within the distribution */

#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <syslog.h>

#include "../config.h"
#include "nftpinhole.h"
#include "../commonrdr.h"


int main(int argc, char * * argv)
{
	int uid;

	openlog("testnftpinhole", LOG_PERROR|LOG_CONS, LOG_LOCAL0);

	uid = add_pinhole("eth0", NULL, 0, "ff::123", 54321, IPPROTO_TCP);
	return 0;
}

