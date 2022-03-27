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
#include "ubcsp.h"

static uint16_t seqnum = 0x0000;

static int fd = -1;

static struct ubcsp_packet send_packet;
static uint8_t send_buffer[512];

static struct ubcsp_packet receive_packet;
static uint8_t receive_buffer[512];

int csr_open_bcsp(char *device, speed_t bcsp_rate)
{
	struct termios ti;
	uint8_t delay, activity = 0x00;
	int timeout = 0;

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

	ti.c_cflag |=  CLOCAL;
	ti.c_cflag &= ~CRTSCTS;
	ti.c_cflag |=  PARENB;
	ti.c_cflag &= ~PARODD;
	ti.c_cflag &= ~CSIZE;
	ti.c_cflag |=  CS8;
	ti.c_cflag &= ~CSTOPB;

	ti.c_cc[VMIN] = 1;
	ti.c_cc[VTIME] = 0;

	cfsetospeed(&ti, bcsp_rate);

	if (tcsetattr(fd, TCSANOW, &ti) < 0) {
		fprintf(stderr, "Can't change port settings: %s (%d)\n",
						strerror(errno), errno);
		close(fd);
		return -1;
	}

	tcflush(fd, TCIOFLUSH);

	if (fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) | O_NONBLOCK) < 0) {
		fprintf(stderr, "Can't set non blocking mode: %s (%d)\n",
						strerror(errno), errno);
		close(fd);
		return -1;
	}

	memset(&send_packet, 0, sizeof(send_packet));
	memset(&receive_packet, 0, sizeof(receive_packet));

	ubcsp_initialize();

	send_packet.length = 512;
	send_packet.payload = send_buffer;

	receive_packet.length = 512;
	receive_packet.payload = receive_buffer;

	ubcsp_receive_packet(&receive_packet);

	while (1) {
		delay = ubcsp_poll(&activity);

		if (activity & UBCSP_PACKET_SENT)
			break;

		if (delay) {
			usleep(delay * 100);

			if (timeout++ > 5000) {
				fprintf(stderr, "Initialization timed out\n");
				return -1;
			}
		}
	}

	return 0;
}

void put_uart(uint8_t ch)
{
	if (write(fd, &ch, 1) < 0)
		fprintf(stderr, "UART write error\n");
}

uint8_t get_uart(uint8_t *ch)
{
	int res = read(fd, ch, 1);
	return res > 0 ? res : 0;
}

static int do_command(uint16_t command, uint16_t seqnum, uint16_t varid, uint8_t *value, uint16_t length)
{
	unsigned char cp[254], rp[254];
	uint8_t cmd[10];
	uint16_t size;
	uint8_t delay, activity = 0x00;
	int timeout = 0, sent = 0;

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
	cp[0] = 0x00;
	cp[1] = 0xfc;
	cp[2] = (size * 2) + 1;
	cp[3] = 0xc2;
	memcpy(cp + 4, cmd, sizeof(cmd));
	memcpy(cp + 14, value, length);

	receive_packet.length = 512;
	ubcsp_receive_packet(&receive_packet);

	send_packet.channel  = 5;
	send_packet.reliable = 1;
	send_packet.length   = (size * 2) + 4;
	memcpy(send_packet.payload, cp, (size * 2) + 4);

	ubcsp_send_packet(&send_packet);

	while (1) {
		delay = ubcsp_poll(&activity);

		if (activity & UBCSP_PACKET_SENT) {
			switch (varid) {
			case CSR_VARID_COLD_RESET:
			case CSR_VARID_WARM_RESET:
			case CSR_VARID_COLD_HALT:
			case CSR_VARID_WARM_HALT:
				return 0;
			}

			sent = 1;
			timeout = 0;
		}

		if (activity & UBCSP_PACKET_RECEIVED) {
			if (sent && receive_packet.channel == 5 &&
					receive_packet.payload[0] == 0xff) {
				memcpy(rp, receive_packet.payload,
							receive_packet.length);
				break;
			}

			receive_packet.length = 512;
			ubcsp_receive_packet(&receive_packet);
			timeout = 0;
		}

		if (delay) {
			usleep(delay * 100);

			if (timeout++ > 5000) {
				fprintf(stderr, "Operation timed out\n");
				errno = ETIMEDOUT;
				return -1;
			}
		}
	}

	if (rp[0] != 0xff || rp[2] != 0xc2) {
		errno = EIO;
		return -1;
	}

	if ((rp[11] + (rp[12] << 8)) != 0) {
		errno = ENXIO;
		return -1;
	}

	memcpy(value, rp + 13, length);

	return 0;
}

int csr_read_bcsp(uint16_t varid, uint8_t *value, uint16_t length)
{
	return do_command(0x0000, seqnum++, varid, value, length);
}

int csr_write_bcsp(uint16_t varid, uint8_t *value, uint16_t length)
{
	return do_command(0x0002, seqnum++, varid, value, length);
}

void csr_close_bcsp(void)
{
	close(fd);
}
