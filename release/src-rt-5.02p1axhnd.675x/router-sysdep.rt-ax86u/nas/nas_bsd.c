/*
 * Network Authentication Service deamon (Linux)
 *
 * Copyright 2019 Broadcom
 *
 * This program is the proprietary software of Broadcom and/or
 * its licensors, and may only be used, duplicated, modified or distributed
 * pursuant to the terms and conditions of a separate, written license
 * agreement executed between you and Broadcom (an "Authorized License").
 * Except as set forth in an Authorized License, Broadcom grants no license
 * (express or implied), right to use, or waiver of any kind with respect to
 * the Software, and Broadcom expressly reserves all rights in and to the
 * Software and all intellectual property rights therein.  IF YOU HAVE NO
 * AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY
 * WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF
 * THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use
 * all reasonable efforts to protect the confidentiality thereof, and to
 * use this information only in connection with your use of Broadcom
 * integrated circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES,
 * REPRESENTATIONS OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR
 * OTHERWISE, WITH RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY
 * DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY,
 * NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES,
 * ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR
 * CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING
 * OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL
 * BROADCOM OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL,
 * SPECIAL, INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR
 * IN ANY WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
 * IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii)
 * ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF
 * OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY
 * NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
 * $Id: nas_linux.c 241388 2011-02-18 03:33:22Z $
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <arpa/inet.h>
#include <signal.h>
#include <fcntl.h>

#include <typedefs.h>
#include <bcmutils.h>
#include <bcmnvram.h>
#include <ethernet.h>
#include <eapol.h>
#include <md5.h>
#include <wlutils.h>

#include <nas_wksp.h>
#include <eapd.h>

static nas_wksp_t * nas_nwksp = NULL;

void
nas_sleep_ms(uint ms)
{
	usleep(1000*ms);
}

void
nas_rand128(uint8 *rand128)
{
	static int dev_random_fd = -1;
	struct timeval tv;
	struct timezone tz;
	MD5_CTX md5;
	if (dev_random_fd == -1)
		/* Use /dev/urandom because /dev/random, when it works at all,
		 * won't return anything when its entropy pool runs short and
		 * we can't control that.  /dev/urandom look good enough.
		 */
		dev_random_fd = open("/dev/urandom", O_RDONLY|O_NONBLOCK);
	if (dev_random_fd != -1)
		read(dev_random_fd, rand128, 16);
	else {
		gettimeofday(&tv, &tz);
		tv.tv_sec ^= rand();
		MD5Init(&md5);
		MD5Update(&md5, (unsigned char *) &tv, sizeof(tv));
		MD5Update(&md5, (unsigned char *) &tz, sizeof(tz));
		MD5Final(rand128, &md5);
	}
}

static void
hup_hdlr(int sig)
{
	if (nas_nwksp)
		nas_nwksp->flags = NAS_WKSP_FLAG_SHUTDOWN;
	return;
}

/*
 * Configuration APIs
 */
int
nas_safe_get_conf(char *outval, int outval_size, char *name)
{
	char *val;

	if (name == NULL || outval == NULL) {
		if (outval)
			memset(outval, 0, outval_size);
		return -1;
	}

	val = nvram_safe_get(name);
	if (!strcmp(val, ""))
		memset(outval, 0, outval_size);
	else
		snprintf(outval, outval_size, "%s", val);
	return 0;
}

/* service main entry */
int
main(int argc, char *argv[])
{
	/* display usage if nothing is specified */
	if (argc == 2 &&
	    (!strncmp(argv[1], "-h", 2) ||
	     !strncmp(argv[1], "-H", 2))) {
		nas_wksp_display_usage();
		return 0;
	}

	/* alloc nas/wpa work space */
	if (!(nas_nwksp = nas_wksp_alloc_workspace())) {
		NASMSG("Unable to allocate work space memory. Quitting...\n");
		return 0;
	}

	if (argc > 1 && nas_wksp_parse_cmd(argc, argv, nas_nwksp)) {
		NASMSG("Command line parsing error. Quitting...\n");
		nas_wksp_free_workspace(nas_nwksp);
		return 0;
	}
	else {
#ifdef BCMDBG
		/* verbose - 0:no | others:yes */
		/* for workspace */
		char debug[8];
		if (nas_safe_get_conf(debug, sizeof(debug), "nas_dbg") == 0)
			debug_nwksp = (int)atoi(debug);
#endif // endif
	}

	/* establish a handler to handle SIGTERM. */
	signal(SIGTERM, hup_hdlr);

	/* run main loop to dispatch messages */
	nas_wksp_main_loop(nas_nwksp);

	return 0;
}

void
nas_reset_board()
{
	kill(1, SIGTERM);
	return;
}
