/*
 * Broadcom UPnP library device specific functions
 *
 * Copyright (C) 2015, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: upnp_device.c 569344 2015-07-08 02:04:32Z $
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
				strncpy(dst->uuid, new_uuid, sizeof(dst->uuid));
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

	upnp_syslog(LOG_INFO, "%s: detach %s", ifp->ifname, device->root_device_xml);

	if (!ifp->device || ifp->device != device)
		return;

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
