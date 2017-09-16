/*
 * device-inq.c: inquire SCSI device information.
 *
 * Copyright (c) 2010 EMC Corporation, Haiying Tang <Tang_Haiying@emc.com>
 * All rights reserved.
 *
 * This program refers to "SCSI Primary Commands - 3 (SPC-3)
 * at http://www.t10.org and sg_inq.c in sg3_utils-1.26 for
 * Linux OS SCSI subsystem, by D. Gilbert.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/mount.h>
#include <sys/select.h>
#include <scsi/scsi.h>
#include <scsi/scsi_ioctl.h>
#include <scsi/sg.h>

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <syslog.h>
#include <dirent.h>
#include <ctype.h>
#include <fcntl.h>
#include <libgen.h>
#include <errno.h>

#include "device-discovery.h"

#define DEF_ALLOC_LEN	255
#define MX_ALLOC_LEN	(0xc000 + 0x80)

static struct bl_serial *bl_create_scsi_string(int len, const char *bytes)
{
	struct bl_serial *s;

	s = malloc(sizeof(*s) + len);
	if (s) {
		s->data = (char *)&s[1];
		s->len = len;
		memcpy(s->data, bytes, len);
	}
	return s;
}

static void bl_free_scsi_string(struct bl_serial *str)
{
	if (str)
		free(str);
}

#define sg_io_ok(io_hdr) \
	((((io_hdr).status & 0x7e) == 0) && \
	((io_hdr).host_status == 0) && \
	(((io_hdr).driver_status & 0x0f) == 0))

static int sg_timeout = 1 * 1000;

static int bldev_inquire_page(int fd, int page, char *buffer, int len)
{
	unsigned char cmd[] = { INQUIRY, 0, 0, 0, 0, 0 };
	unsigned char sense_b[28];
	struct sg_io_hdr io_hdr;
	if (page >= 0) {
		cmd[1] = 1;
		cmd[2] = page;
	}
	cmd[3] = (unsigned char)((len >> 8) & 0xff);
	cmd[4] = (unsigned char)(len & 0xff);

	memset(&io_hdr, 0, sizeof(struct sg_io_hdr));
	io_hdr.interface_id = 'S';
	io_hdr.cmd_len = sizeof(cmd);
	io_hdr.mx_sb_len = sizeof(sense_b);
	io_hdr.dxfer_direction = SG_DXFER_FROM_DEV;
	io_hdr.dxfer_len = len;
	io_hdr.dxferp = buffer;
	io_hdr.cmdp = cmd;
	io_hdr.sbp = sense_b;
	io_hdr.timeout = sg_timeout;
	if (ioctl(fd, SG_IO, &io_hdr) < 0)
		return -1;

	if (sg_io_ok(io_hdr))
		return 0;
	return -1;
}

static int bldev_inquire_pages(int fd, int page, char **buffer)
{
	int status = 0;
	char *tmp;
	int len;

	*buffer = calloc(DEF_ALLOC_LEN, sizeof(char));
	if (!*buffer) {
		BL_LOG_ERR("%s: Out of memory!\n", __func__);
		return -ENOMEM;
	}

	status = bldev_inquire_page(fd, page, *buffer, DEF_ALLOC_LEN);
	if (status)
		goto out;

	status = -1;
	if ((*(*buffer + 1) & 0xff) != page)
		goto out;

	len = (*(*buffer + 2) << 8) + *(*buffer + 3) + 4;
	if (len > MX_ALLOC_LEN) {
		BL_LOG_ERR("SCSI response length too long: %d\n", len);
		goto out;
	}
	if (len > DEF_ALLOC_LEN) {
		tmp = realloc(*buffer, len);
		if (!tmp) {
			BL_LOG_ERR("%s: Out of memory!\n", __func__);
			status = -ENOMEM;
			goto out;
		}
		*buffer = tmp;
		status = bldev_inquire_page(fd, page, *buffer, len);
		if (status)
			goto out;
	}
	status = 0;
 out:
	return status;
}

/* For EMC multipath devices, use VPD page (0xc0) to get status.
 * For other devices, return ACTIVE for now
 */
extern enum bl_path_state_e bldev_read_ap_state(int fd)
{
	int status = 0;
	char *buffer = NULL;
	enum bl_path_state_e ap_state = BL_PATH_STATE_ACTIVE;

	status = bldev_inquire_pages(fd, 0xc0, &buffer);
	if (status)
		goto out;

	if (buffer[4] < 0x02)
		ap_state = BL_PATH_STATE_PASSIVE;
 out:
	if (buffer)
		free(buffer);
	return ap_state;
}

struct bl_serial *bldev_read_serial(int fd, const char *filename)
{
	struct bl_serial *serial_out = NULL;
	int status = 0;
	char *buffer;
	struct bl_dev_id *dev_root, *dev_id;
	unsigned int pos, len, current_id = 0;
	size_t devid_len = sizeof(struct bl_dev_id) - sizeof(unsigned char);

	status = bldev_inquire_pages(fd, 0x83, &buffer);
	if (status)
		goto out;

	dev_root = (struct bl_dev_id *)buffer;

	pos = 0;
	current_id = 0;
	len = dev_root->len;

	if (len < devid_len)
		goto out;

	while (pos < (len - devid_len)) {
		dev_id = (struct bl_dev_id *)&(dev_root->data[pos]);
		pos += (dev_id->len + devid_len);

		/* Some targets export zero length EVPD pages,
		 * skip them to not confuse the device id
		 * cache.
		 */
		if (!dev_id->len)
			continue;

		if ((dev_id->ids & 0xf) < current_id)
			continue;
		switch (dev_id->ids & 0xf) {
			/* We process SCSI ID with four ID cases: 0, 1, 2 and 3.
			 * When more than one ID is available, priority is
			 * 3>2>1>0.
			 */
		case 2:	/* EUI-64 based */
			if ((dev_id->len != 8) && (dev_id->len != 12) &&
			    (dev_id->len != 16))
				break;
		case 3:	/* NAA */
			/* TODO: NAA validity judgement too complicated,
			 * so just ingore it here.
			 */
			if ((dev_id->type & 0xf) != 1) {
				BL_LOG_ERR("Binary code_set expected\n");
				break;
			}
		case 0:	/* vendor specific */
		case 1:	/* T10 vendor identification */
			current_id = dev_id->ids & 0xf;
			if (serial_out)
				bl_free_scsi_string(serial_out);
			serial_out = bl_create_scsi_string(dev_id->len,
							   dev_id->data);
			break;
		}
		if (current_id == 3)
			break;
	}
 out:
	if (!serial_out)
		serial_out = bl_create_scsi_string(strlen(filename), filename);
	if (buffer)
		free(buffer);
	return serial_out;
}
