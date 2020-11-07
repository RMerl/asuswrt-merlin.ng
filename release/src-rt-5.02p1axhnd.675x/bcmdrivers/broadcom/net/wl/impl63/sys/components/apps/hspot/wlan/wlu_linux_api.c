/*
 * Linux specific functions.
 *
 * Copyright 2020 Broadcom
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
 * $Id:$
 */

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <linux/sockios.h>
#ifdef HOTSPOT_AP
typedef u_int64_t u64;
typedef u_int32_t u32;
typedef u_int16_t u16;
typedef u_int8_t u8;
#endif // endif
#include <linux/ethtool.h>
#include <wlioctl.h>
#include <bcmutils.h>
#include "wlu.h"
#include "wlu_remote.h"
#include "wlu_api.h"

#define DEV_TYPE_LEN					3 /* length for devtype 'wl'/'et' */
#define NO_ERROR						0

#define IOCTL_ERROR  -2		/* Error code for Ioctl failure */

int remote_type = NO_REMOTE;

static void *gWl = 0;

static struct ifreq *wlifreq[MAX_WLIF_NUM];
static int wlif_num = 0;

static void
syserr(char *s)
{
	perror(s);
	exit(errno);
}

int
wl_ioctl(void *wl, int cmd, void *buf, int len, bool set)
{
	struct ifreq *ifr = (struct ifreq *) wl;
	wl_ioctl_t ioc;
	int ret = 0;
	int s;

	/* open socket to kernel */
	if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
		syserr("socket");

	/* do it */
	ioc.cmd = cmd;
	ioc.buf = buf;
	ioc.len = len;
	ioc.set = set;
	ifr->ifr_data = (caddr_t) &ioc;
	if ((ret = ioctl(s, SIOCDEVPRIVATE, ifr)) < 0) {
		if (cmd != WLC_GET_MAGIC) {
			ret = IOCTL_ERROR;
		}
	}

	/* cleanup */
	close(s);
	return ret;
}

static int
wl_get_dev_type(char *name, void *buf, int len)
{
	int s;
	int ret;
	struct ifreq ifr;
	struct ethtool_drvinfo info;

	/* open socket to kernel */
	if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
		syserr("socket");

	/* get device type */
	memset(&info, 0, sizeof(info));
	info.cmd = ETHTOOL_GDRVINFO;
	ifr.ifr_data = (caddr_t)&info;
	strncpy(ifr.ifr_name, name, IFNAMSIZ);
	if ((ret = ioctl(s, SIOCETHTOOL, &ifr)) < 0) {

		/* print a good diagnostic if not superuser */
		if (errno == EPERM)
			syserr("wl_get_dev_type");

		*(char *)buf = '\0';
	} else {
		strncpy(buf, info.driver, len);
	}

	close(s);
	return ret;
}

static int
wl_find(void)
{
	char proc_net_dev[] = "/proc/net/dev";
	FILE *fp;
	char buf[1000], *c, *name;
	char dev_type[DEV_TYPE_LEN];
	int status;

	if (!(fp = fopen(proc_net_dev, "r")))
		return BCME_ERROR;

	/* eat first two lines */
	if (!fgets(buf, sizeof(buf), fp) ||
	    !fgets(buf, sizeof(buf), fp)) {
		fclose(fp);
		return BCME_ERROR;
	}

	while (fgets(buf, sizeof(buf), fp)) {
		c = buf;
		while (isspace(*c))
			c++;
		if (!(name = strsep(&c, ":")))
			continue;
		if (wl_get_dev_type(name, dev_type, DEV_TYPE_LEN) >= 0 &&
			!strncmp(dev_type, "wl", 2)) {
			struct ifreq *ifr;
			ifr = malloc(sizeof(struct ifreq));
			if (!ifr) {
				break;
			}
			memset(ifr, 0, sizeof(struct ifreq));
			strncpy(ifr->ifr_name, name, IFNAMSIZ-1);
			ifr->ifr_name[IFNAMSIZ-1] = 0;
			if (wl_check((void *) ifr) == 0) {
				wlifreq[wlif_num] = ifr;
				wlif_num++;
				if (wlif_num >= MAX_WLIF_NUM)
					break;
			} else {
				free(ifr);
			}
		}
	}
	if (wlif_num == 0)
		status = BCME_ERROR;
	else
		status = BCME_OK;

	fclose(fp);
	return status;
}

static int
ioctl_queryinformation_fe(void *wl, int cmd, void* input_buf, int *input_len)
{
	int error = NO_ERROR;

	if (remote_type == NO_REMOTE) {
		error = wl_ioctl(wl, cmd, input_buf, *input_len, FALSE);
	} else {
	}
	return error;
}

static int
ioctl_setinformation_fe(void *wl, int cmd, void* buf, int *len)
{
	int error = 0;

	if (remote_type == NO_REMOTE) {
		error = wl_ioctl(wl,  cmd, buf, *len, TRUE);
	} else {
	}

	return error;
}

int
wl_get(void *wl, int cmd, void *buf, int len)
{
	int error = 0;
	/* For RWL: When interfacing to a Windows client, need t add in OID_BASE */
	error = (int)ioctl_queryinformation_fe(wl, cmd, buf, &len);

	if (error == SERIAL_PORT_ERR)
		return SERIAL_PORT_ERR;
	else if (error == BCME_NODEVICE)
		return BCME_NODEVICE;
	else if (error != 0)
		return IOCTL_ERROR;

	return 0;
}

int
wl_set(void *wl, int cmd, void *buf, int len)
{
	int error = 0;

	/* For RWL: When interfacing to a Windows client, need t add in OID_BASE */
	error = (int)ioctl_setinformation_fe(wl, cmd, buf, &len);

	if (error == SERIAL_PORT_ERR)
		return SERIAL_PORT_ERR;
	else if (error == BCME_NODEVICE)
		return BCME_NODEVICE;
	else if (error != 0)
		return IOCTL_ERROR;

	return 0;
}

void *wl(void)
{
	if (gWl == 0)
		wl_open(&gWl);

	return gWl;
}

void *wlif(int index)
{
	if (index >= wlif_num)
		return NULL;

	return wlifreq[index];
}

void *wl_getifbyname(char *ifname)
{
	int i;
	for (i = 0; i < wlif_num; i++) {
		if (strcmp(wlifreq[i]->ifr_name, ifname) == 0)
			return wlifreq[i];
	}
	return NULL;
}

void wlFree(void)
{
	if (gWl != 0) {
		wl_close();
		gWl = 0;
	}
}

int wl_open(void **wl)
{
	int error = -1;
	wl_find();
	if (wlif_num) {
		*wl = wlifreq[0];
		error = 0;
	}
	return error;
}

void wl_close(void)
{
	int i;
	for (i = 0; i < wlif_num; i++) {
		free(wlifreq[i]);
	}
	wlif_num = 0;
	gWl = 0;
}

char *wl_ifname(void *wl)
{
	struct ifreq *ifr = wl;
	return ifr->ifr_name;
}
