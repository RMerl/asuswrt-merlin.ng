/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
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
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <termios.h>

#include "csr.h"

static uint16_t seqnum = 0x0000;

static int fd = -1;

int csr_open_h4(char *device)
{
	struct termios ti;

	if (!device)
		device = "/dev/ttyS0";

	fd = open(device, O_RDWR | O_NOCTTY);
	if (fd < 0) {
		fprintf(stderr, "Can't open serial port: %s (%d)\n",
						strerror(errno), errno);
		return -1;
	}

	tcflush(fd, TCIOFLUSH);

	if (tcgetattr(fd, &ti) < 0) {
		fprintf(stderr, "Can't get port settings: %s (%d)\n",
						strerror(errno), errno);
		close(fd);
		return -1;
	}

	cfmakeraw(&ti);

	ti.c_cflag |= CLOCAL;
	ti.c_cflag |= CRTSCTS;

	cfsetospeed(&ti, B38400);

	if (tcsetattr(fd, TCSANOW, &ti) < 0) {
		fprintf(stderr, "Can't change port settings: %s (%d)\n",
						strerror(errno), errno);
		close(fd);
		return -1;
	}

	tcflush(fd, TCIOFLUSH);

	return 0;
}

static int do_command(uint16_t command, uint16_t seqnum, uint16_t varid, uint8_t *value, uint16_t length)
{
	unsigned char cp[254], rp[254];
	uint8_t cmd[10];
	uint16_t size;
	int len, offset = 3;

	size = (length < 8) ? 9 : ((length + 1) / 2) + 5;

	cmd[0] = command & 0xff;
	cmd[1] = command >> 8;
	cmd[2] = size & 0xff;
	cmd[3] = size >> 8;
	cmd[4] = seqnum & 0xff;
	cmd[5] = seqnum >> 8;
	cmd[6] = varid & 0xff;
	cmd[7] = varid >> 8;
	cmd[8] = 0x00;
	cmd[9] = 0x00;

	memset(cp, 0, sizeof(cp));
	cp[0] = 0x01;
	cp[1] = 0x00;
	cp[2] = 0xfc;
	cp[3] = (size * 2) + 1;
	cp[4] = 0xc2;
	memcpy(cp + 5, cmd, sizeof(cmd));
	memcpy(cp + 15, value, length);

	if (write(fd, cp, (size * 2) + 5) < 0)
		return -1;

	switch (varid) {
	case CSR_VARID_COLD_RESET:
	case CSR_VARID_WARM_RESET:
	case CSR_VARID_COLD_HALT:
	case CSR_VARID_WARM_HALT:
		return 0;
	}

	do {
		if (read(fd, rp, 1) < 1)
			return -1;
	} while (rp[0] != 0x04);

	if (read(fd, rp + 1, 2) < 2)
		return -1;

	do {
		len = read(fd, rp + offset, sizeof(rp) - offset);
		offset += len;
	} while (offset < rp[2] + 3);

	if (rp[0] != 0x04 || rp[1] != 0xff || rp[3] != 0xc2) {
		errno = EIO;
		return -1;
	}

	if ((rp[12] + (rp[13] << 8)) != 0) {
		errno = ENXIO;
		return -1;
	}

	memcpy(value, rp + 14, length);

	return 0;
}

int csr_read_h4(uint16_t varid, uint8_t *value, uint16_t length)
{
	return do_command(0x0000, seqnum++, varid, value, length);
}

int csr_write_h4(uint16_t varid, uint8_t *value, uint16_t length)
{
	return do_command(0x0002, seqnum++, varid, value, length);
}

void csr_close_h4(void)
{
	close(fd);
}
