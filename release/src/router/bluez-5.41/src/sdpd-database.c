/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2001-2002  Nokia Corporation
 *  Copyright (C) 2002-2003  Maxim Krasnyansky <maxk@qualcomm.com>
 *  Copyright (C) 2002-2010  Marcel Holtmann <marcel@holtmann.org>
 *  Copyright (C) 2002-2003  Stephen Crane <steve.crane@rococosoft.com>
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

#include <stdlib.h>
#include <stdbool.h>

#include "lib/bluetooth.h"
#include "lib/sdp.h"
#include "lib/sdp_lib.h"

#include "sdpd.h"
#include "log.h"

static sdp_list_t *service_db;
static sdp_list_t *access_db;

typedef struct {
	uint32_t handle;
	bdaddr_t device;
} sdp_access_t;

/*
 * Ordering function called when inserting a service record.
 * The service repository is a linked list in sorted order
 * and the service record handle is the sort key
 */
int record_sort(const void *r1, const void *r2)
{
	const sdp_record_t *rec1 = r1;
	const sdp_record_t *rec2 = r2;

	if (!rec1 || !rec2) {
		error("NULL RECORD LIST FATAL");
		return -1;
	}

	return rec1->handle - rec2->handle;
}

static int access_sort(const void *r1, const void *r2)
{
	const sdp_access_t *rec1 = r1;
	const sdp_access_t *rec2 = r2;

	if (!rec1 || !rec2) {
		error("NULL RECORD LIST FATAL");
		return -1;
	}

	return rec1->handle - rec2->handle;
}

static void access_free(void *p)
{
	free(p);
}

/*
 * Reset the service repository by deleting its contents
 */
void sdp_svcdb_reset(void)
{
	sdp_list_free(service_db, (sdp_free_func_t) sdp_record_free);
	service_db = NULL;

	sdp_list_free(access_db, access_free);
	access_db = NULL;
}

typedef struct _indexed {
	int sock;
	sdp_record_t *record;
} sdp_indexed_t;

static sdp_list_t *socket_index;

/*
 * collect all services registered over this socket
 */
void sdp_svcdb_collect_all(int sock)
{
	sdp_list_t *p, *q;

	for (p = socket_index, q = 0; p; ) {
		sdp_indexed_t *item = p->data;
		if (item->sock == sock) {
			sdp_list_t *next = p->next;
			sdp_record_remove(item->record->handle);
			sdp_record_free(item->record);
			free(item);
			if (q)
				q->next = next;
			else
				socket_index = next;
			free(p);
			p = next;
		} else if (item->sock > sock)
			return;
		else {
			q = p;
			p = p->next;
		}
	}
}

void sdp_svcdb_collect(sdp_record_t *rec)
{
	sdp_list_t *p, *q;

	for (p = socket_index, q = 0; p; q = p, p = p->next) {
		sdp_indexed_t *item = p->data;
		if (rec == item->record) {
			free(item);
			if (q)
				q->next = p->next;
			else
				socket_index = p->next;
			free(p);
			return;
		}
	}
}

static int compare_indices(const void *i1, const void *i2)
{
	const sdp_indexed_t *s1 = i1;
	const sdp_indexed_t *s2 = i2;
	return s1->sock - s2->sock;
}

void sdp_svcdb_set_collectable(sdp_record_t *record, int sock)
{
	sdp_indexed_t *item = malloc(sizeof(sdp_indexed_t));

	if (!item) {
		SDPDBG("No memory");
		return;
	}

	item->sock = sock;
	item->record = record;
	socket_index = sdp_list_insert_sorted(socket_index, item, compare_indices);
}

/*
 * Add a service record to the repository
 */
void sdp_record_add(const bdaddr_t *device, sdp_record_t *rec)
{
	sdp_access_t *dev;

	SDPDBG("Adding rec : 0x%lx", (long) rec);
	SDPDBG("with handle : 0x%x", rec->handle);

	service_db = sdp_list_insert_sorted(service_db, rec, record_sort);

	dev = malloc(sizeof(*dev));
	if (!dev)
		return;

	bacpy(&dev->device, device);
	dev->handle = rec->handle;

	access_db = sdp_list_insert_sorted(access_db, dev, access_sort);
}

static sdp_list_t *record_locate(uint32_t handle)
{
	if (service_db) {
		sdp_list_t *p;
		sdp_record_t r;

		r.handle = handle;
		p = sdp_list_find(service_db, &r, record_sort);
		return p;
	}

	SDPDBG("Could not find svcRec for : 0x%x", handle);
	return NULL;
}

static sdp_list_t *access_locate(uint32_t handle)
{
	if (access_db) {
		sdp_list_t *p;
		sdp_access_t a;

		a.handle = handle;
		p = sdp_list_find(access_db, &a, access_sort);
		return p;
	}

	SDPDBG("Could not find access data for : 0x%x", handle);
	return NULL;
}

/*
 * Given a service record handle, find the record associated with it.
 */
sdp_record_t *sdp_record_find(uint32_t handle)
{
	sdp_list_t *p = record_locate(handle);

	if (!p) {
		SDPDBG("Couldn't find record for : 0x%x", handle);
		return 0;
	}

	return p->data;
}

/*
 * Given a service record handle, remove its record from the repository
 */
int sdp_record_remove(uint32_t handle)
{
	sdp_list_t *p = record_locate(handle);
	sdp_record_t *r;
	sdp_access_t *a;

	if (!p) {
		error("Remove : Couldn't find record for : 0x%x", handle);
		return -1;
	}

	r = p->data;
	if (r)
		service_db = sdp_list_remove(service_db, r);

	p = access_locate(handle);
	if (p == NULL || p->data == NULL)
		return 0;

	a = p->data;

	access_db = sdp_list_remove(access_db, a);
	access_free(a);

	return 0;
}

/*
 * Return a pointer to the linked list containing the records in sorted order
 */
sdp_list_t *sdp_get_record_list(void)
{
	return service_db;
}

int sdp_check_access(uint32_t handle, bdaddr_t *device)
{
	sdp_list_t *p = access_locate(handle);
	sdp_access_t *a;

	if (!p)
		return 1;

	a = p->data;
	if (!a)
		return 1;

	if (bacmp(&a->device, device) &&
			bacmp(&a->device, BDADDR_ANY) &&
			bacmp(device, BDADDR_ANY))
		return 0;

	return 1;
}

uint32_t sdp_next_handle(void)
{
	uint32_t handle = 0x10000;

	while (sdp_record_find(handle))
		handle++;

	return handle;
}
