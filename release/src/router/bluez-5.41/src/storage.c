/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2006-2010  Nokia Corporation
 *  Copyright (C) 2004-2010  Marcel Holtmann <marcel@holtmann.org>
 *
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <sys/file.h>
#include <sys/stat.h>

#include <glib.h>

#include "lib/bluetooth.h"
#include "lib/sdp.h"
#include "lib/sdp_lib.h"
#include "lib/uuid.h"

#include "textfile.h"
#include "uuid-helper.h"
#include "storage.h"

/* When all services should trust a remote device */
#define GLOBAL_TRUST "[all]"

struct match {
	GSList *keys;
	char *pattern;
};

static inline int create_filename(char *buf, size_t size,
				const bdaddr_t *bdaddr, const char *name)
{
	char addr[18];

	ba2str(bdaddr, addr);

	return create_name(buf, size, STORAGEDIR, addr, name);
}

int read_discoverable_timeout(const char *src, int *timeout)
{
	char filename[PATH_MAX], *str;

	create_name(filename, PATH_MAX, STORAGEDIR, src, "config");

	str = textfile_get(filename, "discovto");
	if (!str)
		return -ENOENT;

	if (sscanf(str, "%d", timeout) != 1) {
		free(str);
		return -ENOENT;
	}

	free(str);

	return 0;
}

int read_pairable_timeout(const char *src, int *timeout)
{
	char filename[PATH_MAX], *str;

	create_name(filename, PATH_MAX, STORAGEDIR, src, "config");

	str = textfile_get(filename, "pairto");
	if (!str)
		return -ENOENT;

	if (sscanf(str, "%d", timeout) != 1) {
		free(str);
		return -ENOENT;
	}

	free(str);

	return 0;
}

int read_on_mode(const char *src, char *mode, int length)
{
	char filename[PATH_MAX], *str;

	create_name(filename, PATH_MAX, STORAGEDIR, src, "config");

	str = textfile_get(filename, "onmode");
	if (!str)
		return -ENOENT;

	strncpy(mode, str, length);
	mode[length - 1] = '\0';

	free(str);

	return 0;
}

int read_local_name(const bdaddr_t *bdaddr, char *name)
{
	char filename[PATH_MAX], *str;
	int len;

	create_filename(filename, PATH_MAX, bdaddr, "config");

	str = textfile_get(filename, "name");
	if (!str)
		return -ENOENT;

	len = strlen(str);
	if (len > HCI_MAX_NAME_LENGTH)
		str[HCI_MAX_NAME_LENGTH] = '\0';
	strcpy(name, str);

	free(str);

	return 0;
}

sdp_record_t *record_from_string(const char *str)
{
	sdp_record_t *rec;
	int size, i, len;
	uint8_t *pdata;
	char tmp[3];

	size = strlen(str)/2;
	pdata = g_malloc0(size);

	tmp[2] = 0;
	for (i = 0; i < size; i++) {
		memcpy(tmp, str + (i * 2), 2);
		pdata[i] = (uint8_t) strtol(tmp, NULL, 16);
	}

	rec = sdp_extract_pdu(pdata, size, &len);
	g_free(pdata);

	return rec;
}

sdp_record_t *find_record_in_list(sdp_list_t *recs, const char *uuid)
{
	sdp_list_t *seq;

	for (seq = recs; seq; seq = seq->next) {
		sdp_record_t *rec = (sdp_record_t *) seq->data;
		sdp_list_t *svcclass = NULL;
		char *uuid_str;

		if (sdp_get_service_classes(rec, &svcclass) < 0)
			continue;

		/* Extract the uuid */
		uuid_str = bt_uuid2string(svcclass->data);
		if (!uuid_str) {
			sdp_list_free(svcclass, free);
			continue;
		}

		if (!strcasecmp(uuid_str, uuid)) {
			sdp_list_free(svcclass, free);
			free(uuid_str);
			return rec;
		}

		sdp_list_free(svcclass, free);
		free(uuid_str);
	}
	return NULL;
}
