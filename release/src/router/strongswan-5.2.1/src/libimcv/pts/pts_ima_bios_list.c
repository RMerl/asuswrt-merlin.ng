/*
 * Copyright (C) 2011-2014 Andreas Steffen
 * HSR Hochschule fuer Technik Rapperswil
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.  See <http://www.fsf.org/copyleft/gpl.txt>.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 */

#include "pts_ima_bios_list.h"

#include <utils/debug.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

typedef struct private_pts_ima_bios_list_t private_pts_ima_bios_list_t;
typedef struct bios_entry_t bios_entry_t;
typedef enum event_type_t event_type_t;

enum event_type_t {
	/* BIOS Events (TCG PC Client Specification for Conventional BIOS 1.21) */
	EV_PREBOOT_CERT =                  0x00000000,
	EV_POST_CODE =                     0x00000001,
	EV_UNUSED =                        0x00000002,
	EV_NO_ACTION =                     0x00000003,
	EV_SEPARATOR =                     0x00000004,
	EV_ACTION =                        0x00000005,
	EV_EVENT_TAG =                     0x00000006,
	EV_S_CRTM_CONTENTS =               0x00000007,
	EV_S_CRTM_VERSION =                0x00000008,
	EV_CPU_MICROCODE =                 0x00000009,
	EV_PLATFORM_CONFIG_FLAGS =         0x0000000A,
	EV_TABLE_OF_DEVICES =              0x0000000B,
	EV_COMPACT_HASH =                  0x0000000C,
	EV_IPL =                           0x0000000D,
	EV_IPL_PARTITION_DATA =            0x0000000E,
	EV_NONHOST_CODE =                  0x0000000F,
	EV_NONHOST_CONFIG =                0x00000010,
	EV_NONHOST_INFO =                  0x00000011,
	EV_OMIT_BOOT_DEVICE_EVENTS =       0x00000012,

	/* EFI Events (TCG EFI Platform Specification 1.22) */
	EV_EFI_EVENT_BASE =                0x80000000,
	EV_EFI_VARIABLE_DRIVER_CONFIG =    0x80000001,
	EV_EFI_VARIABLE_BOOT =             0x80000002,
	EV_EFI_BOOT_SERVICES_APPLICATION = 0x80000003,
	EV_EFI_BOOT_SERVICES_DRIVER =      0x80000004,
	EV_EFI_RUNTIME_SERVICES_DRIVER =   0x80000005,
	EV_EFI_GPT_EVENT =                 0x80000006,
	EV_EFI_ACTION =                    0x80000007,
	EV_EFI_PLATFORM_FIRMWARE_BLOB =    0x80000008,
	EV_EFI_HANDOFF_TABLES =            0x80000009,

	EV_EFI_VARIABLE_AUTHORITY =        0x800000E0
};

ENUM_BEGIN(event_type_names, EV_PREBOOT_CERT, EV_OMIT_BOOT_DEVICE_EVENTS,
	"Preboot Cert",
	"POST Code",
	"Unused",
	"No Action",
	"Separator",
	"Action",
	"Event Tag",
	"S-CRTM Contents",
	"S-CRTM Version",
	"CPU Microcode",
	"Platform Config Flags",
	"Table of Devices",
	"Compact Hash",
	"IPL",
	"IPL Partition Data",
	"Nonhost Code",
	"Nonhost Config",
	"Nonhost Info",
	"Omit Boot Device Events"
);

ENUM_NEXT(event_type_names, EV_EFI_EVENT_BASE, EV_EFI_HANDOFF_TABLES,
							EV_OMIT_BOOT_DEVICE_EVENTS,
	"EFI Event Base",
	"EFI Variable Driver Config",
	"EFI Variable Boot",
	"EFI Boot Services Application",
	"EFI Boot Services Driver",
	"EFI Runtime Services Driver",
	"EFI GPT Event",
	"EFI Action",
	"EFI Platform Firmware Blob",
	"EFI Handoff Tables"
);
ENUM_NEXT(event_type_names, EV_EFI_VARIABLE_AUTHORITY, EV_EFI_VARIABLE_AUTHORITY,
							EV_EFI_HANDOFF_TABLES,
	"EFI Variable Authority"
);
ENUM_END(event_type_names, EV_EFI_VARIABLE_AUTHORITY);

/**
 * Private data of a pts_ima_bios_list_t object.
 *
 */
struct private_pts_ima_bios_list_t {

	/**
	 * Public pts_ima_bios_list_t interface.
	 */
	pts_ima_bios_list_t public;

	/**
	 * List of BIOS measurement entries
	 */
	linked_list_t *list;

	/**
	 * Time when BIOS measurements were taken
	 */
	time_t creation_time;

};

/**
 * Linux IMA BIOS measurement entry
 */
struct bios_entry_t {

	/**
	 * PCR register
	 */
	uint32_t pcr;

	/**
	 * SHA1 measurement hash
	 */
	chunk_t measurement;
};

/**
 * Free a bios_entry_t object
 */
static void free_bios_entry(bios_entry_t *this)
{
	free(this->measurement.ptr);
	free(this);
}

METHOD(pts_ima_bios_list_t, get_time, time_t,
	private_pts_ima_bios_list_t *this)
{
	return this->creation_time;
}

METHOD(pts_ima_bios_list_t, get_count, int,
	private_pts_ima_bios_list_t *this)
{
	return this->list->get_count(this->list);
}

METHOD(pts_ima_bios_list_t, get_next, status_t,
	private_pts_ima_bios_list_t *this, uint32_t *pcr, chunk_t *measurement)
{
	bios_entry_t *entry;
	status_t status;

	status = this->list->remove_first(this->list, (void**)&entry);
	*pcr = entry->pcr;
	*measurement = entry->measurement;
	free(entry);

	return status;
}

METHOD(pts_ima_bios_list_t, destroy, void,
	private_pts_ima_bios_list_t *this)
{
	this->list->destroy_function(this->list, (void *)free_bios_entry);
	free(this);
}

/**
 * See header
 */
pts_ima_bios_list_t* pts_ima_bios_list_create(char *file)
{
	private_pts_ima_bios_list_t *this;
	uint32_t pcr, event_type, event_len, seek_len;
	uint32_t buf_len = 2048;
	uint8_t event_buf[buf_len];
	chunk_t event;
	bios_entry_t *entry;
	struct stat st;
	ssize_t res;
	int fd;

	fd = open(file, O_RDONLY);
	if (fd == -1)
	{
		DBG1(DBG_PTS, "opening '%s' failed: %s", file, strerror(errno));
		return NULL;
	}

	if (fstat(fd, &st) == -1)
	{
		DBG1(DBG_PTS, "getting statistics of '%s' failed: %s", file,
			 strerror(errno));
		close(fd);
		return FALSE;
	}

	INIT(this,
		.public = {
			.get_time = _get_time,
			.get_count = _get_count,
			.get_next = _get_next,
			.destroy = _destroy,
		},
		.creation_time = st.st_ctime,
		.list = linked_list_create(),
	);

	DBG2(DBG_PTS, "PCR Event Type  (Size)");
	while (TRUE)
	{
		res = read(fd, &pcr, 4);
		if (res == 0)
		{
			DBG2(DBG_PTS, "loaded bios measurements '%s' (%d entries)",
				 file, this->list->get_count(this->list));
			close(fd);
			return &this->public;
		}

		entry = malloc_thing(bios_entry_t);
		entry->pcr = pcr;
		entry->measurement = chunk_alloc(HASH_SIZE_SHA1);

		if (res != 4)
		{
			break;
		}
		if (read(fd, &event_type, 4) != 4)
		{
			break;
		}
		if (read(fd, entry->measurement.ptr, HASH_SIZE_SHA1) != HASH_SIZE_SHA1)
		{
			break;
		}
		if (read(fd, &event_len, 4) != 4)
		{
			break;
		}
		DBG2(DBG_PTS, "%2u  %N  (%u bytes)", pcr, event_type_names, event_type,
											 event_len);

		seek_len = (event_len > buf_len) ? event_len - buf_len : 0;
		event_len -= seek_len;

		if (read(fd, event_buf, event_len) != event_len)
		{
			break;
		}
		event = chunk_create(event_buf, event_len);
		DBG3(DBG_PTS,"%B", &event);

		if (event_type == EV_ACTION || event_type == EV_EFI_ACTION)
		{
			DBG2(DBG_PTS, "     '%.*s'", event_len, event_buf);
		}

		if (seek_len > 0 && lseek(fd, seek_len, SEEK_CUR) == -1)
		{
				break;
		}
		this->list->insert_last(this->list, entry);
	}

	DBG1(DBG_PTS, "loading bios measurements '%s' failed: %s", file,
		 strerror(errno));
	free_bios_entry(entry);
	close(fd);
	destroy(this);

	return NULL;
}
