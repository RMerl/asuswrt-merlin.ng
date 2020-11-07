/*
 * Broadcom UPnP library device specific functions
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
 * $Id: upnp_device.c 780195 2019-10-17 19:00:17Z $
 */

#include <upnp.h>
#include <md5.h>

/* Generate the UUID for a UPnPdevice */
int
upnp_device_renew_rootxml(UPNP_CONTEXT *context, char *databuf)
{
	UPNP_INTERFACE *ifp = context->focus_ifp;
	UPNP_ADVERTISE *advertise;
	char *name;
	char *uuid;

	char *url;
	char *url_end;
	int len;
	unsigned char myaddr[sizeof("255.255.255.255:65535") + 1];
	char buf[32];

	/* Locate for device type until end */
	name = databuf;
	while ((name = strstr(name, "<deviceType>")) != NULL) {
		/* Search uuid starting from <deviceType> */
		uuid = strstr(name, "<UDN>uuid:");
		if (!uuid) {
			/* This must be something wrong */
			break;
		}

		/* Skip tag */
		name += strlen("<deviceType>");
		uuid += strlen("<UDN>uuid:");

		/* Find the deviceType in advlist,
		 * and replace the corresponding uuid
		 */
		for (advertise = ifp->advlist; advertise->name; advertise++) {
			if (memcmp(advertise->name, name, strlen(advertise->name)) == 0) {
				memcpy(uuid, advertise->uuid, strlen(advertise->uuid));
				break;
			}
		}

		/* Go next */
		name = uuid;
	}

	/* Get present URL */
	upnp_host_addr(myaddr, ifp->ipaddr, 80);
	len = snprintf(buf, sizeof(buf), "http://%s", myaddr);

	/* Search <presentationURL>, and </presentationURL> */
	url = strstr(databuf, "<presentationURL>");
	if (!url)
		return 0;

	url += strlen("<presentationURL>");

	url_end = strstr(url, "</presentationURL>");
	if (!url_end)
		return 0;

	memmove(url + len, url_end, strlen(url_end) + 1);

	/* Update URL */
	memcpy(url, buf, len);
	return 0;
}

static void
upnp_gen_uuid(char *uuid, char *deviceType, unsigned char mac[6])
{
	unsigned char new_uuid[16];

	MD5_CTX mdContext;

	/* Generate hash */
	MD5Init(&mdContext);
	MD5Update(&mdContext, mac, 6);
	MD5Update(&mdContext, (unsigned char *)deviceType, strlen(deviceType));
	MD5Final(new_uuid, &mdContext);

	snprintf(uuid, 37,
		"%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
		new_uuid[0],  new_uuid[1],  new_uuid[2],  new_uuid[3],
		new_uuid[4],  new_uuid[5],  new_uuid[6],  new_uuid[7],
		new_uuid[8],  new_uuid[9],  new_uuid[10], new_uuid[11],
		new_uuid[12], new_uuid[13], new_uuid[14], new_uuid[15]);

	return;
}

static int
upnp_device_advlist_init(UPNP_CONTEXT *context)
{
	int i;
	int size;
	UPNP_INTERFACE *ifp = context->focus_ifp;
	UPNP_ADVERTISE *src, *dst;
	char new_uuid[64];
	char old_uuid[64];

	/* Count advertise_table size */
	for (i = 0; ifp->device->advertise_table[i].name; i++)
		;

	size = (i + 1) * sizeof(UPNP_ADVERTISE);
	ifp->advlist = (UPNP_ADVERTISE *)malloc(size);
	if (ifp->advlist == NULL)
		return -1;

	memcpy(ifp->advlist, ifp->device->advertise_table, size);

	/* Renew UUIDs of this ifp */
	for (src = ifp->advlist; src->name; src++) {
		if (src->type != ADVERTISE_ROOTDEVICE &&
			src->type != ADVERTISE_DEVICE)
			continue;

		/* Generate interface uuid for this device */
		strcpy(old_uuid, src->uuid);
		upnp_gen_uuid(new_uuid, src->name, (unsigned char *)ifp->mac);

		/* Sync to device/services with this UUID */
		for (dst = ifp->advlist; dst->name; dst++) {
			if (strcmp(dst->uuid, old_uuid) == 0) {
				memcpy(dst->uuid, new_uuid, sizeof(dst->uuid));
				dst->uuid[sizeof(dst->uuid)-1] = '\0';
			}
		}
	}

	return 0;
}

static void
upnp_device_advlist_deinit(UPNP_CONTEXT *context)
{
	UPNP_INTERFACE *ifp = context->focus_ifp;

	/* Free advlist */
	if (ifp->advlist) {
		free(ifp->advlist);
		ifp->advlist = 0;
	}
	return;
}

int
upnp_device_attach(UPNP_CONTEXT *context, UPNP_DEVICE *device)
{
	UPNP_INTERFACE *ifp = context->focus_ifp;

	upnp_syslog(LOG_INFO, "%s: attach %s", ifp->ifname, device->root_device_xml);

	/* Check if the device has already attached? */
	if (ifp->device && ifp->device == device)
		return 0;

	ifp->device = device;

	/* Setup per interface UUIDs */
	upnp_device_advlist_init(context);

	/* Remove this chain, if open error */
	if (device->open && (*device->open)(context) != 0) {
		ifp->device = 0;
		return -1;
	}

	/* Initialize gena event variable */
	gena_init(context);

	/* Send alive here */
	ssdp_alive(context);

	return 0;
}

void
upnp_device_detach(UPNP_CONTEXT *context, UPNP_DEVICE *device)
{
	UPNP_INTERFACE	*ifp = context->focus_ifp;

	if (!ifp->device || ifp->device != device)
		return;

	upnp_syslog(LOG_INFO, "%s: detach %s", ifp->ifname, device->root_device_xml);

	/* Do device specific stop function */
	if (device->close)
		(*device->close)(context);

	/* Clear event variables and subscribers */
	gena_shutdown(context);

	/* Send byebye */
	ssdp_byebye(context);

	/* Free per interface advertise list */
	upnp_device_advlist_deinit(context);

	/* detach it */
	ifp->device = 0;

	return;
}
