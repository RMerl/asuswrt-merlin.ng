/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2012-2014  Intel Corporation. All rights reserved.
 *
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <endian.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/stat.h>

#include "src/shared/btsnoop.h"

struct btsnoop_hdr {
	uint8_t		id[8];		/* Identification Pattern */
	uint32_t	version;	/* Version Number = 1 */
	uint32_t	type;		/* Datalink Type */
} __attribute__ ((packed));
#define BTSNOOP_HDR_SIZE (sizeof(struct btsnoop_hdr))

struct btsnoop_pkt {
	uint32_t	size;		/* Original Length */
	uint32_t	len;		/* Included Length */
	uint32_t	flags;		/* Packet Flags */
	uint32_t	drops;		/* Cumulative Drops */
	uint64_t	ts;		/* Timestamp microseconds */
	uint8_t		data[0];	/* Packet Data */
} __attribute__ ((packed));
#define BTSNOOP_PKT_SIZE (sizeof(struct btsnoop_pkt))

static const uint8_t btsnoop_id[] = { 0x62, 0x74, 0x73, 0x6e,
				      0x6f, 0x6f, 0x70, 0x00 };

static const uint32_t btsnoop_version = 1;

struct pklg_pkt {
	uint32_t	len;
	uint64_t	ts;
	uint8_t		type;
} __attribute__ ((packed));
#define PKLG_PKT_SIZE (sizeof(struct pklg_pkt))

struct btsnoop {
	int ref_count;
	int fd;
	unsigned long flags;
	uint32_t format;
	uint16_t index;
	bool aborted;
	bool pklg_format;
	bool pklg_v2;
};

struct btsnoop *btsnoop_open(const char *path, unsigned long flags)
{
	struct btsnoop *btsnoop;
	struct btsnoop_hdr hdr;
	ssize_t len;

	btsnoop = calloc(1, sizeof(*btsnoop));
	if (!btsnoop)
		return NULL;

	btsnoop->fd = open(path, O_RDONLY | O_CLOEXEC);
	if (btsnoop->fd < 0) {
		free(btsnoop);
		return NULL;
	}

	btsnoop->flags = flags;

	len = read(btsnoop->fd, &hdr, BTSNOOP_HDR_SIZE);
	if (len < 0 || len != BTSNOOP_HDR_SIZE)
		goto failed;

	if (!memcmp(hdr.id, btsnoop_id, sizeof(btsnoop_id))) {
		/* Check for BTSnoop version 1 format */
		if (be32toh(hdr.version) != btsnoop_version)
			goto failed;

		btsnoop->format = be32toh(hdr.type);
		btsnoop->index = 0xffff;
	} else {
		if (!(btsnoop->flags & BTSNOOP_FLAG_PKLG_SUPPORT))
			goto failed;

		/* Check for Apple Packet Logger format */
		if (hdr.id[0] != 0x00 ||
				(hdr.id[1] != 0x00 && hdr.id[1] != 0x01))
			goto failed;

		btsnoop->format = BTSNOOP_FORMAT_MONITOR;
		btsnoop->index = 0xffff;
		btsnoop->pklg_format = true;
		btsnoop->pklg_v2 = (hdr.id[1] == 0x01);

		/* Apple Packet Logger format has no header */
		lseek(btsnoop->fd, 0, SEEK_SET);
	}

	return btsnoop_ref(btsnoop);

failed:
	close(btsnoop->fd);
	free(btsnoop);

	return NULL;
}

struct btsnoop *btsnoop_create(const char *path, uint32_t format)
{
	struct btsnoop *btsnoop;
	struct btsnoop_hdr hdr;
	ssize_t written;

	btsnoop = calloc(1, sizeof(*btsnoop));
	if (!btsnoop)
		return NULL;

	btsnoop->fd = open(path, O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC,
					S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
	if (btsnoop->fd < 0) {
		free(btsnoop);
		return NULL;
	}

	btsnoop->format = format;
	btsnoop->index = 0xffff;

	memcpy(hdr.id, btsnoop_id, sizeof(btsnoop_id));
	hdr.version = htobe32(btsnoop_version);
	hdr.type = htobe32(btsnoop->format);

	written = write(btsnoop->fd, &hdr, BTSNOOP_HDR_SIZE);
	if (written < 0) {
		close(btsnoop->fd);
		free(btsnoop);
		return NULL;
	}

	return btsnoop_ref(btsnoop);
}

struct btsnoop *btsnoop_ref(struct btsnoop *btsnoop)
{
	if (!btsnoop)
		return NULL;

	__sync_fetch_and_add(&btsnoop->ref_count, 1);

	return btsnoop;
}

void btsnoop_unref(struct btsnoop *btsnoop)
{
	if (!btsnoop)
		return;

	if (__sync_sub_and_fetch(&btsnoop->ref_count, 1))
		return;

	if (btsnoop->fd >= 0)
		close(btsnoop->fd);

	free(btsnoop);
}

uint32_t btsnoop_get_format(struct btsnoop *btsnoop)
{
	if (!btsnoop)
		return BTSNOOP_FORMAT_INVALID;

	return btsnoop->format;
}

bool btsnoop_write(struct btsnoop *btsnoop, struct timeval *tv,
			uint32_t flags, uint32_t drops, const void *data,
			uint16_t size)
{
	struct btsnoop_pkt pkt;
	uint64_t ts;
	ssize_t written;

	if (!btsnoop || !tv)
		return false;

	ts = (tv->tv_sec - 946684800ll) * 1000000ll + tv->tv_usec;

	pkt.size  = htobe32(size);
	pkt.len   = htobe32(size);
	pkt.flags = htobe32(flags);
	pkt.drops = htobe32(drops);
	pkt.ts    = htobe64(ts + 0x00E03AB44A676000ll);

	written = write(btsnoop->fd, &pkt, BTSNOOP_PKT_SIZE);
	if (written < 0)
		return false;

	if (data && size > 0) {
		written = write(btsnoop->fd, data, size);
		if (written < 0)
			return false;
	}

	return true;
}

static uint32_t get_flags_from_opcode(uint16_t opcode)
{
	switch (opcode) {
	case BTSNOOP_OPCODE_NEW_INDEX:
	case BTSNOOP_OPCODE_DEL_INDEX:
		break;
	case BTSNOOP_OPCODE_COMMAND_PKT:
		return 0x02;
	case BTSNOOP_OPCODE_EVENT_PKT:
		return 0x03;
	case BTSNOOP_OPCODE_ACL_TX_PKT:
		return 0x00;
	case BTSNOOP_OPCODE_ACL_RX_PKT:
		return 0x01;
	case BTSNOOP_OPCODE_SCO_TX_PKT:
	case BTSNOOP_OPCODE_SCO_RX_PKT:
		break;
	case BTSNOOP_OPCODE_OPEN_INDEX:
	case BTSNOOP_OPCODE_CLOSE_INDEX:
		break;
	}

	return 0xff;
}

bool btsnoop_write_hci(struct btsnoop *btsnoop, struct timeval *tv,
			uint16_t index, uint16_t opcode, uint32_t drops,
			const void *data, uint16_t size)
{
	uint32_t flags;

	if (!btsnoop)
		return false;

	switch (btsnoop->format) {
	case BTSNOOP_FORMAT_HCI:
		if (btsnoop->index == 0xffff)
			btsnoop->index = index;

		if (index != btsnoop->index)
			return false;

		flags = get_flags_from_opcode(opcode);
		if (flags == 0xff)
			return false;
		break;

	case BTSNOOP_FORMAT_MONITOR:
		flags = (index << 16) | opcode;
		break;

	default:
		return false;
	}

	return btsnoop_write(btsnoop, tv, flags, drops, data, size);
}

bool btsnoop_write_phy(struct btsnoop *btsnoop, struct timeval *tv,
			uint16_t frequency, const void *data, uint16_t size)
{
	uint32_t flags;

	if (!btsnoop)
		return false;

	switch (btsnoop->format) {
	case BTSNOOP_FORMAT_SIMULATOR:
		flags = (1 << 16) | frequency;
		break;

	default:
		return false;
	}

	return btsnoop_write(btsnoop, tv, flags, 0, data, size);
}

static bool pklg_read_hci(struct btsnoop *btsnoop, struct timeval *tv,
					uint16_t *index, uint16_t *opcode,
					void *data, uint16_t *size)
{
	struct pklg_pkt pkt;
	uint32_t toread;
	uint64_t ts;
	ssize_t len;

	len = read(btsnoop->fd, &pkt, PKLG_PKT_SIZE);
	if (len == 0)
		return false;

	if (len < 0 || len != PKLG_PKT_SIZE) {
		btsnoop->aborted = true;
		return false;
	}

	if (btsnoop->pklg_v2) {
		toread = le32toh(pkt.len) - (PKLG_PKT_SIZE - 4);

		ts = le64toh(pkt.ts);
		tv->tv_sec = ts & 0xffffffff;
		tv->tv_usec = ts >> 32;
	} else {
		toread = be32toh(pkt.len) - (PKLG_PKT_SIZE - 4);

		ts = be64toh(pkt.ts);
		tv->tv_sec = ts >> 32;
		tv->tv_usec = ts & 0xffffffff;
	}

	switch (pkt.type) {
	case 0x00:
		*index = 0x0000;
		*opcode = BTSNOOP_OPCODE_COMMAND_PKT;
		break;
	case 0x01:
		*index = 0x0000;
		*opcode = BTSNOOP_OPCODE_EVENT_PKT;
		break;
	case 0x02:
		*index = 0x0000;
		*opcode = BTSNOOP_OPCODE_ACL_TX_PKT;
		break;
	case 0x03:
		*index = 0x0000;
		*opcode = BTSNOOP_OPCODE_ACL_RX_PKT;
		break;
	case 0x0b:
		*index = 0x0000;
		*opcode = BTSNOOP_OPCODE_VENDOR_DIAG;
		break;
	case 0xfc:
		*index = 0xffff;
		*opcode = BTSNOOP_OPCODE_SYSTEM_NOTE;
		break;
	default:
		*index = 0xffff;
		*opcode = 0xffff;
		break;
	}

	len = read(btsnoop->fd, data, toread);
	if (len < 0) {
		btsnoop->aborted = true;
		return false;
	}

	*size = toread;

	return true;
}

static uint16_t get_opcode_from_flags(uint8_t type, uint32_t flags)
{
	switch (type) {
	case 0x01:
		return BTSNOOP_OPCODE_COMMAND_PKT;
	case 0x02:
		if (flags & 0x01)
			return BTSNOOP_OPCODE_ACL_RX_PKT;
		else
			return BTSNOOP_OPCODE_ACL_TX_PKT;
	case 0x03:
		if (flags & 0x01)
			return BTSNOOP_OPCODE_SCO_RX_PKT;
		else
			return BTSNOOP_OPCODE_SCO_TX_PKT;
	case 0x04:
		return BTSNOOP_OPCODE_EVENT_PKT;
	case 0xff:
		if (flags & 0x02) {
			if (flags & 0x01)
				return BTSNOOP_OPCODE_EVENT_PKT;
			else
				return BTSNOOP_OPCODE_COMMAND_PKT;
		} else {
			if (flags & 0x01)
				return BTSNOOP_OPCODE_ACL_RX_PKT;
			else
				return BTSNOOP_OPCODE_ACL_TX_PKT;
		}
		break;
	}

	return 0xffff;
}

bool btsnoop_read_hci(struct btsnoop *btsnoop, struct timeval *tv,
					uint16_t *index, uint16_t *opcode,
					void *data, uint16_t *size)
{
	struct btsnoop_pkt pkt;
	uint32_t toread, flags;
	uint64_t ts;
	uint8_t pkt_type;
	ssize_t len;

	if (!btsnoop || btsnoop->aborted)
		return false;

	if (btsnoop->pklg_format)
		return pklg_read_hci(btsnoop, tv, index, opcode, data, size);

	len = read(btsnoop->fd, &pkt, BTSNOOP_PKT_SIZE);
	if (len == 0)
		return false;

	if (len < 0 || len != BTSNOOP_PKT_SIZE) {
		btsnoop->aborted = true;
		return false;
	}

	toread = be32toh(pkt.size);
	if (toread > BTSNOOP_MAX_PACKET_SIZE) {
		btsnoop->aborted = true;
		return false;
	}

	flags = be32toh(pkt.flags);

	ts = be64toh(pkt.ts) - 0x00E03AB44A676000ll;
	tv->tv_sec = (ts / 1000000ll) + 946684800ll;
	tv->tv_usec = ts % 1000000ll;

	switch (btsnoop->format) {
	case BTSNOOP_FORMAT_HCI:
		*index = 0;
		*opcode = get_opcode_from_flags(0xff, flags);
		break;

	case BTSNOOP_FORMAT_UART:
		len = read(btsnoop->fd, &pkt_type, 1);
		if (len < 0) {
			btsnoop->aborted = true;
			return false;
		}
		toread--;

		*index = 0;
		*opcode = get_opcode_from_flags(pkt_type, flags);
		break;

	case BTSNOOP_FORMAT_MONITOR:
		*index = flags >> 16;
		*opcode = flags & 0xffff;
		break;

	default:
		btsnoop->aborted = true;
		return false;
	}

	len = read(btsnoop->fd, data, toread);
	if (len < 0) {
		btsnoop->aborted = true;
		return false;
	}

	*size = toread;

	return true;
}

bool btsnoop_read_phy(struct btsnoop *btsnoop, struct timeval *tv,
			uint16_t *frequency, void *data, uint16_t *size)
{
	return false;
}
