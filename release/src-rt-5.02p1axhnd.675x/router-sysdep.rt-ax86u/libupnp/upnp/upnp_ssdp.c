/*
 * Broadcom UPnP library SSDP implementation
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
 * $Id: upnp_ssdp.c 780195 2019-10-17 19:00:17Z $
 */

#include <upnp.h>

static int upnp_ssdp_fsm_init(UPNP_CONTEXT *);
static int upnp_ssdp_parse_method(UPNP_CONTEXT *);
static int upnp_ssdp_parse_uri(UPNP_CONTEXT *);
static int upnp_ssdp_fsm_deinit(UPNP_CONTEXT *);
static int ssdp_msearch(UPNP_CONTEXT *);

static struct upnp_state ssdp_fsm[] =
{
	{upnp_ssdp_fsm_init},
	{upnp_msg_head_read},
	{upnp_ssdp_parse_method},
	{upnp_ssdp_parse_uri},
	{upnp_msg_parse},
	{ssdp_msearch},
	{upnp_ssdp_fsm_deinit},
	{0}
};

/* Send out SSDP packet */
void
ssdp_send(UPNP_CONTEXT *context)
{
	struct sockaddr_in dst;

	char *buf = context->head_buffer;
	int len = strlen(buf);

	/* If ssdp socket initialized failed, just return */
	if (context->ssdp_sock == -1)
		return;

	/*
	 * if caller does not specify a unicast address, send multicast
	 * (239.255.255.250)
	 */
	dst = context->dst_addr;
	if (dst.sin_addr.s_addr == 0) {
		/* Assign mutlicast interface to send */
		struct in_addr inaddr = context->focus_ifp->ipaddr;
		char optval = 4;

		setsockopt(context->ssdp_sock,
			IPPROTO_IP, IP_MULTICAST_IF, (void *)&inaddr, sizeof(inaddr));

		setsockopt(context->ssdp_sock,
			IPPROTO_IP, IP_MULTICAST_TTL, &optval, sizeof(optval));

		/* Send to SSDP_ADDR:SSDP_PORT */
		dst.sin_family = AF_INET;
		dst.sin_port = htons(SSDP_PORT);
		inet_aton(SSDP_ADDR, &dst.sin_addr);
	}

	/* Send packet */
	sendto(context->ssdp_sock, buf, len, 0, (struct sockaddr *)&dst, sizeof(dst));
	return;
}

/* Send SSDP response for a M-SEARCH request */
void
ssdp_response(UPNP_CONTEXT *context, UPNP_ADVERTISE *advertise, int type)
{
	UPNP_INTERFACE	*ifp = context->focus_ifp;
	UPNP_DEVICE *device = ifp->device;

	unsigned char myaddr[sizeof("255.255.255.255:65535") + UPNP_URL_UUID_LEN + 1];
	char time_buf[TIME_BUF_LEN];
	char *buf = context->head_buffer;
	char *p;
	int len;

	/* Get location URI */
	upnp_host_addr(myaddr, ifp->ipaddr, context->http_port);
	if (context->baseurl_postfix != NULL) {
		/* inserting random uuid in the url */
		memcpy((char *)myaddr + strlen((char *)myaddr), "/", 2);
		memcpy((char *)myaddr + strlen((char *)myaddr),
				context->baseurl_postfix, strlen(context->baseurl_postfix) + 1);
	}

	/* Should we use local time ? */
	upnp_gmt_time(time_buf);

	/* Build headers */
	len = snprintf(buf, sizeof(context->head_buffer),
		"HTTP/1.1 200 OK\r\n"
		"Cache-Control: max-age=%d\r\n"
		"Date: %s\r\n"
		"Ext: \r\n"
		"Location: http://%s/%s\r\n"
		"Server: POSIX UPnP/1.0 %s/%s\r\n",
		context->adv_time,
		time_buf,
		myaddr,
		device->root_device_xml,
		"UPnP Stack",
		UPNP_VERSION_STR);

	p = buf + len;

	switch (type) {
	case MSEARCH_SERVICE:
		snprintf(p, sizeof(context->head_buffer) - len,
			"ST: %s:1\r\n"
			"USN: uuid:%s::%s:1\r\n"
			"\r\n",
			advertise->name, advertise->uuid, advertise->name);
		break;

	case MSEARCH_DEVICE:
		snprintf(p, sizeof(context->head_buffer) - len,
			"ST: %s:1\r\n"
			"USN: uuid:%s::%s:1\r\n"
			"\r\n",
			advertise->name, advertise->uuid, advertise->name);
		break;

	case MSEARCH_UUID:
		snprintf(p, sizeof(context->head_buffer) - len,
			"ST: uuid:%s\r\n"
			"USN: uuid:%s\r\n"
			"\r\n",
			advertise->uuid, advertise->uuid);
		break;

	case MSEARCH_ROOTDEVICE:
		snprintf(p, sizeof(context->head_buffer) - len,
			"ST: upnp:rootdevice\r\n"
			"USN: uuid:%s::upnp:rootdevice\r\n"
			"\r\n",
			advertise->uuid);
		break;
	}

	/* send packet out */
	ssdp_send(context);
	return;
}

/* Send SSDP notifications out */
void
ssdp_notify(UPNP_CONTEXT *context, UPNP_ADVERTISE *advertise, int adv_type, int ssdp_type)
{
	int len;
	char *buf;
	char *p;

	context->dst_addr.sin_addr.s_addr = 0;
	buf = context->head_buffer;

	if (ssdp_type == SSDP_ALIVE) {
		UPNP_INTERFACE	*ifp = context->focus_ifp;
		unsigned char	myaddr[sizeof("255.255.255.255:65535") + UPNP_URL_UUID_LEN + 1];

		/* Get location URI */
		upnp_host_addr(myaddr, ifp->ipaddr, context->http_port);
		if (context->baseurl_postfix != NULL) {
			/* inserting random uuid in the url */
			memcpy((char *)myaddr + strlen((char *)myaddr), "/", 2);
			memcpy((char *)myaddr + strlen((char *)myaddr),
					context->baseurl_postfix, UPNP_URL_UUID_LEN);
		}

		/* SSDP_ALIVE */
		len = snprintf(buf, sizeof(context->head_buffer),
			"NOTIFY * HTTP/1.1\r\n"
			"Host: 239.255.255.250:1900\r\n"
			"Cache-Control: max-age=%d\r\n"
			"Location: http://%s/%s\r\n"
			"NTS: ssdp:alive\r\n"
			"Server: POSIX, UPnP/1.0 %s/%s\r\n",
			context->adv_time,
			myaddr,
			ifp->device->root_device_xml,
			"UPnP Stack",
			UPNP_VERSION_STR);
	}
	else {
		/* SSDP_BYEBYE */
		len = snprintf(buf, sizeof(context->head_buffer),
			"NOTIFY * HTTP/1.1\r\n"
			"Host: 239.255.255.250:1900\r\n"
			"NTS: ssdp:byebye\r\n");
	}

	p = buf + len;

	switch (adv_type) {
	case ADVERTISE_SERVICE:
		snprintf(p, sizeof(context->head_buffer) - len,
			"NT: %s:1\r\n"
			"USN: uuid:%s::%s:1\r\n"
			"\r\n",
			advertise->name, advertise->uuid, advertise->name);
		break;

	case ADVERTISE_DEVICE:
		snprintf(p, sizeof(context->head_buffer) - len,
			"NT: %s:1\r\n"
			"USN: uuid:%s::%s:1\r\n"
			"\r\n",
			advertise->name, advertise->uuid, advertise->name);
		break;

	case ADVERTISE_UUID:
		snprintf(p, sizeof(context->head_buffer) - len,
			"NT: uuid:%s\r\n"
			"USN: uuid:%s\r\n"
			"\r\n",
			advertise->uuid, advertise->uuid);
		break;

	case ADVERTISE_ROOTDEVICE:
		snprintf(p, sizeof(context->head_buffer) - len,
			"NT: upnp:rootdevice\r\n"
			"USN: uuid:%s::upnp:rootdevice\r\n"
			"\r\n",
			advertise->uuid);
		break;
	}

	ssdp_send(context);
	return;
}

/* Send SSDP advertisement messages */
void
ssdp_adv_process(UPNP_CONTEXT *context, int ssdp_type)
{
	UPNP_ADVERTISE	*advertise;

	for (advertise = context->focus_ifp->advlist;
	     advertise->name;
	     advertise++) {

		switch (advertise->type) {
		case ADVERTISE_SERVICE:
			ssdp_notify(context, advertise, ADVERTISE_SERVICE,    ssdp_type);
			break;

		case ADVERTISE_DEVICE:
			ssdp_notify(context, advertise, ADVERTISE_DEVICE,     ssdp_type);
			ssdp_notify(context, advertise, ADVERTISE_UUID,       ssdp_type);
			break;

		default:
			ssdp_notify(context, advertise, ADVERTISE_DEVICE,     ssdp_type);
			ssdp_notify(context, advertise, ADVERTISE_UUID,       ssdp_type);
			ssdp_notify(context, advertise, ADVERTISE_ROOTDEVICE, ssdp_type);
			break;
		}
	}

	return;
}

/* Send SSDP_BYEBYE message */
void
ssdp_byebye(UPNP_CONTEXT *context)
{
	ssdp_adv_process(context, SSDP_BYEBYE);
}

/* Send SSDP_ALIVE message */
void
ssdp_alive(UPNP_CONTEXT *context)
{
	ssdp_adv_process(context, SSDP_ALIVE);
}

/* Respond to an SSDP M-SEARCH message */
void
ssdp_msearch_response(UPNP_CONTEXT *context, char *name, int type)
{
	UPNP_ADVERTISE	*advertise;

	for (advertise = context->focus_ifp->advlist;
		advertise->name;
		advertise++) {

		switch (type) {
		case MSEARCH_ROOTDEVICE:
			if (advertise->type == ADVERTISE_ROOTDEVICE) {
				ssdp_response(context, advertise, MSEARCH_ROOTDEVICE);
			}
			break;

		case MSEARCH_UUID:
			if (advertise->type != ADVERTISE_SERVICE &&
				strcmp(name, advertise->uuid) == 0) {
				ssdp_response(context, advertise, MSEARCH_UUID);

				/* Return because the desired UUID matched */
				return;
			}
			break;

		case MSEARCH_DEVICE:
			if (strcmp(name, advertise->name) == 0) {
				ssdp_response(context, advertise, MSEARCH_DEVICE);
			}
			break;

		case MSEARCH_SERVICE:
			if (strcmp(name, advertise->name) == 0) {
				ssdp_response(context, advertise, MSEARCH_SERVICE);
			}
			break;

		case MSEARCH_ALL:
			if (advertise->type == ADVERTISE_SERVICE) {
				ssdp_response(context, advertise, MSEARCH_SERVICE);
			}
			else if (advertise->type == ADVERTISE_DEVICE) {
				ssdp_response(context, advertise, MSEARCH_DEVICE);
				ssdp_response(context, advertise, MSEARCH_UUID);
			}
			else if (advertise->type == ADVERTISE_ROOTDEVICE) {
				ssdp_response(context, advertise, MSEARCH_DEVICE);
				ssdp_response(context, advertise, MSEARCH_UUID);
				ssdp_response(context, advertise, MSEARCH_ROOTDEVICE);
			}
			break;
		}
	} /* advertise */

	return;
}

/* Parse the M-SEARCH message */
static int
ssdp_msearch(UPNP_CONTEXT *context)
{
	char name[128];
	int  type;
	char *host;
	char *man;
	char *st;
	char *mx;

	/* check HOST:239.255.255.250:1900 */
	host = upnp_msg_get(context, "HOST");
	if (!host || strcmp(host, "239.255.255.250:1900") != 0)
		return -1;

	/* check MAN:"ssdp:discover" */
	man = upnp_msg_get(context, "MAN");
	if (!man || strcmp(man, "\"ssdp:discover\"") != 0)
		return -1;

	/* check MX */
	mx = upnp_msg_get(context, "MX");
	if (!mx || strlen(mx) == 0) {
		return -1;
	}
	else {
		long int val = 0;
		char *endptr = 0;

		val = strtol(mx, &endptr, 10);
		if (endptr == mx || *endptr != 0 || val <= 0)
			return -1;
	}

	/* process search target */
	st = upnp_msg_get(context, "ST");
	if (!st)
		return -1;

	if (strcmp(st, "ssdp:all") == 0) {
		type = MSEARCH_ALL;
	}
	else if (strcmp(st, "upnp:rootdevice") == 0) {
		type = MSEARCH_ROOTDEVICE;
	}
	else if (memcmp(st, "uuid:", 5) == 0) {
		/* uuid */
		type = MSEARCH_UUID;
		st += 5;
		strncpy(name, st, sizeof(name));
		name[sizeof(name)-1] = '\0';
	}
	else {
		/* check advertise_table for specify name. */
		UPNP_ADVERTISE	*advertise;
		int name_len;

		type = -1;

		for (advertise = context->focus_ifp->advlist;
			advertise->name;
			advertise++) {
			name_len = strlen(advertise->name);
			if (advertise->type != ADVERTISE_UUID &&
				memcmp(st, advertise->name, name_len) == 0 &&
				strcmp(&st[name_len], ":1") == 0) {

				/* Search target is a UPnP service */
				if (advertise->type == ADVERTISE_SERVICE) {
					type  = MSEARCH_SERVICE;
					strncpy(name, st, name_len);
					name[name_len] = 0;
					goto find;
				}
				else if (advertise->type == ADVERTISE_DEVICE ||
					advertise->type == ADVERTISE_ROOTDEVICE) {
					/* device and rootdevice are all devices */
					type  = MSEARCH_DEVICE;
					strncpy(name, st, name_len);
					name[name_len] = 0;
					goto find;
				}
			}
		} /* advertise */
find:
		if (type == -1)
			return -1;
	}

	/* do M-SEARCH response */
	ssdp_msearch_response(context, name, type);
	return 0;
}

/* Start ssdp fsm */
static int
upnp_ssdp_fsm_init(UPNP_CONTEXT *context)
{
	/* Cleanup fields */
	context->index = 0;
	context->status = 0;
	context->err_msg[0] = 0;

	upnp_msg_init(context);
	return 0;
}

/*
 * Parse the M-SEARCH.
 */
static int
upnp_ssdp_parse_method(UPNP_CONTEXT *context)
{
	int len;
	int gap;
	char *p = upnp_msg_tok(context);

	/* Locate method */
	len = strcspn(p, " \t");
	gap = strspn(p + len, " \t");
	p[len] = 0;
	if (strcmp(p, "M-SEARCH") != 0) {
		context->status = R_METHOD_NA;
		return -1;
	}

	/* Skip white space for URI */
	context->url = p + len + gap;

	/* Set method */
	context->method_id = 0;
	context->method = METHOD_MSEARCH;
	return 0;
}

static int
upnp_ssdp_parse_uri(UPNP_CONTEXT *context)
{
	int len;
	int gap;
	char *p = context->url;

	/* Locate URI */
	len = strcspn(p, " \t");
	gap = strspn(p + len, " \t");
	p[len] = 0;
	if (strcmp(context->url, "*") != 0) {
		upnp_syslog(LOG_ERR, "SSDP URL=%s failed!", context->url);
		return -1;
	}

	/* Skip URL and white spaces */
	p += len + gap;
	if (strcmp(p, "HTTP/1.1") != 0) {
		upnp_syslog(LOG_ERR, "SSDP method=%d, HTTP version=%s", context->method, p);
		return -1;
	}

	return 0;
}

static int
upnp_ssdp_fsm_deinit(UPNP_CONTEXT *context)
{
	upnp_msg_deinit(context);
	return 0;
}

/* SSDP process entry */
void
ssdp_process(UPNP_CONTEXT *context)
{
	UPNP_INTERFACE	*ifp;
	int s = context->ssdp_sock;

	struct sockaddr_in addr;
	socklen_t addr_len = sizeof(struct sockaddr_in);
	int len;
	int i;

	memset(&addr, 0, sizeof(addr));
	len = recvfrom(s, context->buf, sizeof(context->buf),
		0, (struct sockaddr *)&addr, &addr_len);
	if (len <= 0)
		return;

	for (ifp = context->iflist;
		 ifp;
		 ifp = ifp->next) {
		/*
		 * locate UPnP interface for the multicast socket
		 */
		if ((ifp->ipaddr.s_addr & ifp->netmask.s_addr) ==
			(addr.sin_addr.s_addr & ifp->netmask.s_addr)) {
			/* Set focus interface */
			context->focus_ifp = ifp;
			break;
		}
	}
	if (ifp == NULL)
		return;

	context->buf[len] = 0;
	context->end = len;

	context->dst_addr = addr;
	context->fd = s;

	/* Perform ssdp fsm */
	for (i = 0; ssdp_fsm[i].func; i++) {
		if ((*ssdp_fsm[i].func)(context) < 0) {
			upnp_ssdp_fsm_deinit(context);
			break;
		}
	}

	return;
}

/* SSDP advertising timer */
void
ssdp_timeout(UPNP_CONTEXT *context)
{
	ssdp_alive(context);
}

/* Add the interface IP to the SSDP multicast networking */
int
ssdp_add_multi(UPNP_CONTEXT *context)
{
	UPNP_INTERFACE	*ifp = context->focus_ifp;
	int ret;
	struct ip_mreq mreq;

	if (context->ssdp_sock == -1)
		return -1;

	/*
	 * Join the interface ip to the SSDP multicast group
	 */
	inet_aton(SSDP_ADDR,  &mreq.imr_multiaddr);
	mreq.imr_interface = ifp->ipaddr;
	ret = setsockopt(context->ssdp_sock, IPPROTO_IP, IP_ADD_MEMBERSHIP,
		(char *)&mreq, sizeof(mreq));
	if (ret) {
		upnp_syslog(LOG_ERR, "IP_ADD_MEMBERSHIP errno = 0x%x", errno);
		return -1;
	}

	/* Multicast group joined successfully */
	ifp->flag |= IFF_MJOINED;
	return 0;
}

/*
 * Delete mutlicast membership of a specific IP address
 * with regard to the SSDP socket.
 */
void
ssdp_del_multi(UPNP_CONTEXT *context)
{
	UPNP_INTERFACE	*ifp = context->focus_ifp;
	struct ip_mreq mreq;

	if (context->ssdp_sock == -1)
		return;

	/* For ip changed or shutdown, we have to drop membership of multicast */
	if (ifp->flag & IFF_MJOINED) {
		inet_aton(SSDP_ADDR, &mreq.imr_multiaddr);
		mreq.imr_interface = ifp->ipaddr;

		setsockopt(context->ssdp_sock,
			IPPROTO_IP, IP_DROP_MEMBERSHIP, (char *)&mreq, sizeof(mreq));
		ifp->flag &= ~IFF_MJOINED;
	}

	return;
}

/* Shutdown SSDP module */
void
ssdp_shutdown(UPNP_CONTEXT *context)
{
	if (context->ssdp_sock != -1) {
		close(context->ssdp_sock);
		context->ssdp_sock = -1;
	}

	return;
}

/* Initialize SSDP advertisement interval and open ssdp_sock */
int
ssdp_init(UPNP_CONTEXT *context)
{
	struct in_addr addr = {INADDR_ANY};

	/* save ssdp socket */
	context->ssdp_sock = upnp_open_udp_socket(addr, SSDP_PORT);
	if (context->ssdp_sock == -1) {
		upnp_syslog(LOG_ERR, "Cannot open ssdp_multi_sock");
		return -1;
	}

	return 0;
}
