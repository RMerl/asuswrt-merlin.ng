/*
 * Broadcom UPnP library init/deinit
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
 * $Id: upnp.c 551827 2015-04-24 08:59:54Z $
 */

#include <upnp.h>

/* Shutdown all the UPnP interfaces */
void
upnp_deinit(UPNP_CONTEXT *context)
{
	UPNP_INTERFACE	*ifp;

	while ((ifp = context->iflist) != 0) {

		context->focus_ifp = ifp;

		/* Unhook device database */
		upnp_device_detach(context, ifp->device);

		/* shutdown interface protocol */
		upnp_http_shutdown(context);

		ssdp_del_multi(context);

		/* Drop this link and free */
		context->iflist = ifp->next;

		free(ifp);
	}

	/* Shutdown the mutlicast receive socket */
	ssdp_shutdown(context);

	upnp_syslog(LOG_INFO, "UPnP daemon stopped");
	if (context->baseurl_postfix != NULL) {
		free(context->baseurl_postfix);
	}
	free(context);
	return;
}

/*
 * Get interface IP addresses and netmasks from interface names
 */
static int
get_if_ipaddr(UPNP_INTERFACE *ifp)
{
	if (upnp_osl_ifaddr(ifp->ifname, &ifp->ipaddr) != 0)
		return -1;

	if (upnp_osl_netmask(ifp->ifname, &ifp->netmask) != 0)
		return -1;

	if ((upnp_osl_hwaddr(ifp->ifname, ifp->mac) != 0) ||
		memcmp(ifp->mac, "\0\0\0\0\0\0", 6) == 0)
		return -1;

	return 0;
}

/* Do UPnP interface initialization */
int
upnp_ifattach(UPNP_CONTEXT *context, char *ifname, UPNP_DEVICE *device)
{
	UPNP_INTERFACE	*ifp;

	/* Allocate interface space */
	ifp = (UPNP_INTERFACE *)malloc(sizeof(*ifp));
	if (ifp == 0)
		return 0;

	memset(ifp, 0, sizeof(*ifp));

	/* Setup context */
	strncpy(ifp->ifname, ifname, sizeof(ifp->ifname));
	ifp->ifname[sizeof(ifp->ifname)-1] = '\0';
	ifp->http_sock = -1;

	if (get_if_ipaddr(ifp) != 0) {
		free(ifp);
		return 0;
	}

	/* Do prepend */
	ifp->next = context->iflist;
	context->iflist = ifp;

	/* Attache device */
	if (context->ssdp_sock == -1) {
		if (ssdp_init(context) != 0) {
			free(ifp);
			return -1;
		}
	}

	context->focus_ifp = ifp;

	/* Perform per interface protocol initialization */
	if (upnp_http_init(context) != 0) {
		upnp_syslog(LOG_ERR, "upnp_http_init::%s init error!", ifp->ifname);
		return -1;
	}

	if (ssdp_add_multi(context) == -1) {
		upnp_syslog(LOG_ERR, "ssdp_add_multi::%s error!", ifp->ifname);
		return -1;
	}

	/*
	 * Hook device table to each interface.
	 * The init function of each device
	 * intialize the event variables, and send SSDP ALIVE to each
	 * interface
	 */
	if (upnp_device_attach(context, device) == -1) {
		upnp_syslog(LOG_ERR, "upnp_device_attach::%s(%s) error!",
			device->root_device_xml, ifp->ifname);
		return -1;
	}
	return 0;
}

/* UPnP module initialization */
UPNP_CONTEXT *upnp_init(int http_port, int adv_time, char *lan_ifnamelist[],
	UPNP_DEVICE *device, char *randomstring)
{
	UPNP_CONTEXT *context;
	int i;

	if (lan_ifnamelist[0] == 0)
		return NULL;

	/* Allocate context */
	context = (UPNP_CONTEXT *)malloc(sizeof(*context));
	if (context == 0)
		return NULL;

	/* Clean up */
	memset(context, 0, sizeof(*context));

	context->http_port = http_port;
	context->adv_time = adv_time;
	context->ssdp_trains = SSDP_TRAINING_PERIOD;

	/* Do context common initialization */
	context->adv_seconds = time(0);
	context->gena_last_check = time(0);
	context->upnp_last_time = time(0);
	context->ssdp_sock = -1;

	if (randomstring != NULL) {
		context->baseurl_postfix = strdup(randomstring);
	}

	/* Attach upnp interfaces */
	for (i = 0; lan_ifnamelist[i]; i++) {
		/* Attach interface */
		if (upnp_ifattach(context, lan_ifnamelist[i], device) != 0) {
			upnp_deinit(context);
			return NULL;
		}
	}

	/* iflist validation */
	if (context->iflist == 0) {
		upnp_deinit(context);
		return NULL;
	}

	return context;
}

/* Time out handler of SSDP, GEAN and all the devices */
void
upnp_timeout(UPNP_CONTEXT *context)
{
	UPNP_INTERFACE	*ifp;
	UPNP_DEVICE	*device;

	time_t now = time(0);
	int update_ssdp;
	int update_gena;
	int delta;
	unsigned int adv_time;

	/* Special patch for NTP */
	if ((now - context->upnp_last_time) > 631123200) {
		/* Update for ssdp */
		delta = context->upnp_last_time - context->adv_seconds;
		context->adv_seconds = (now - delta) - 1;
	}

	/* Okay, it's safe to check */
	if (context->ssdp_trains) {
		adv_time = SSDP_TRAINING_TIME;
		context->ssdp_trains--;
	} else
		adv_time = context->adv_time;

	update_ssdp = ((u_long)(now - context->adv_seconds) >= adv_time - 1);
	update_gena = ((u_long)(now - context->gena_last_check) >= GENA_TIMEOUT);

	/* Add device timer here */
	for (ifp = context->iflist;
		 ifp;
		 ifp = ifp->next) {

		/* Set the focus inteface for further reference */
		context->focus_ifp = ifp;

		/* check for advertisement interval */
		if (update_ssdp)
			ssdp_timeout(context);

		/* check for subscription expirations every 30 seconds */
		if (update_gena)
			gena_timeout(context);

		/* Check device timeout */
		device = ifp->device;
		if (device && device->timeout)
			(*device->timeout)(context, now);
	}

	/* Update ssdp timer, gena timer, and current system time */
	if (update_ssdp)
		context->adv_seconds = now;

	if (update_gena)
		context->gena_last_check = now;

	context->upnp_last_time = now;
	return;
}

/* Read upnp socket */
int
upnp_fd_read(int s, char *buf, int len, int flags)
{
	long     rc;
	fd_set   ibits;
	struct   timeval tv = {5, 0};   /* wait for at most 5 seconds */

	FD_ZERO(&ibits);
	FD_SET(s, &ibits);

	rc = select(s+1, &ibits, 0, 0, &tv);
	if (rc > 0) {
		if (FD_ISSET(s, &ibits))
			return recv(s, buf, len, flags);
	}

	return -1;
}

/* Set upnp socket select set */
int
upnp_fdset(UPNP_CONTEXT *context, fd_set *fds)
{
	UPNP_INTERFACE	*ifp;
	int max_fd = -1;

	/* Set select sockets */
	if (context->ssdp_sock != -1) {
		FD_SET(context->ssdp_sock, fds);
		max_fd = context->ssdp_sock;
	}

	for (ifp = context->iflist; ifp; ifp = ifp->next) {
		FD_SET(ifp->http_sock, fds);

		if (ifp->http_sock > max_fd)
			max_fd = ifp->http_sock;
	}

	return max_fd;
}

/* check upnp socket select set */
int
upnp_fd_isset(UPNP_CONTEXT *context, fd_set *fds)
{
	UPNP_INTERFACE	*ifp;

	/* check ssdp multicast packet */
	if (context->ssdp_sock != -1 && FD_ISSET(context->ssdp_sock, fds))
		return 1;

	/* check upnp_http */
	for (ifp = context->iflist; ifp; ifp = ifp->next) {
		context->focus_ifp = ifp;

		if (FD_ISSET(ifp->http_sock, fds))
			return 1;
	}

	return 0;
}

/* Dispatch UPnP incoming messages. */
void
upnp_dispatch(UPNP_CONTEXT *context, fd_set *fds)
{
	UPNP_INTERFACE	*ifp;

	/* process ssdp multicast packet */
	if (context->ssdp_sock != -1 && FD_ISSET(context->ssdp_sock, fds)) {
		ssdp_process(context);
	}

	/* process upnp_http */
	for (ifp = context->iflist; ifp; ifp = ifp->next) {
		context->focus_ifp = ifp;

		/* process http */
		if (FD_ISSET(ifp->http_sock, fds)) {
			upnp_http_process(context);
		}
	}

	return;
}
