/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2003-2010  Marcel Holtmann <marcel@holtmann.org>
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
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "lib/bluetooth.h"
#include "lib/l2cap.h"
#include "lib/sdp.h"
#include "lib/sdp_lib.h"

#include "cups.h"

#define HCRP_PDU_CREDIT_GRANT		0x0001
#define HCRP_PDU_CREDIT_REQUEST		0x0002
#define HCRP_PDU_GET_LPT_STATUS		0x0005

#define HCRP_STATUS_FEATURE_UNSUPPORTED	0x0000
#define HCRP_STATUS_SUCCESS		0x0001
#define HCRP_STATUS_CREDIT_SYNC_ERROR	0x0002
#define HCRP_STATUS_GENERIC_FAILURE	0xffff

struct hcrp_pdu_hdr {
	uint16_t pid;
	uint16_t tid;
	uint16_t plen;
} __attribute__ ((packed));
#define HCRP_PDU_HDR_SIZE 6

struct hcrp_credit_grant_cp {
	uint32_t credit;
} __attribute__ ((packed));
#define HCRP_CREDIT_GRANT_CP_SIZE 4

struct hcrp_credit_grant_rp {
	uint16_t status;
} __attribute__ ((packed));
#define HCRP_CREDIT_GRANT_RP_SIZE 2

struct hcrp_credit_request_rp {
	uint16_t status;
	uint32_t credit;
} __attribute__ ((packed));
#define HCRP_CREDIT_REQUEST_RP_SIZE 6

struct hcrp_get_lpt_status_rp {
	uint16_t status;
	uint8_t  lpt_status;
} __attribute__ ((packed));
#define HCRP_GET_LPT_STATUS_RP_SIZE 3

static int hcrp_credit_grant(int sk, uint16_t tid, uint32_t credit)
{
	struct hcrp_pdu_hdr hdr;
	struct hcrp_credit_grant_cp cp;
	struct hcrp_credit_grant_rp rp;
	unsigned char buf[128];
	int len;

	hdr.pid = htons(HCRP_PDU_CREDIT_GRANT);
	hdr.tid = htons(tid);
	hdr.plen = htons(HCRP_CREDIT_GRANT_CP_SIZE);
	cp.credit = credit;
	memcpy(buf, &hdr, HCRP_PDU_HDR_SIZE);
	memcpy(buf + HCRP_PDU_HDR_SIZE, &cp, HCRP_CREDIT_GRANT_CP_SIZE);
	len = write(sk, buf, HCRP_PDU_HDR_SIZE + HCRP_CREDIT_GRANT_CP_SIZE);
	if (len < 0)
		return len;

	len = read(sk, buf, sizeof(buf));
	if (len < 0)
		return len;

	memcpy(&hdr, buf, HCRP_PDU_HDR_SIZE);
	memcpy(&rp, buf + HCRP_PDU_HDR_SIZE, HCRP_CREDIT_GRANT_RP_SIZE);

	if (ntohs(rp.status) != HCRP_STATUS_SUCCESS) {
		errno = EIO;
		return -1;
	}

	return 0;
}

static int hcrp_credit_request(int sk, uint16_t tid, uint32_t *credit)
{
	struct hcrp_pdu_hdr hdr;
	struct hcrp_credit_request_rp rp;
	unsigned char buf[128];
	int len;

	hdr.pid = htons(HCRP_PDU_CREDIT_REQUEST);
	hdr.tid = htons(tid);
	hdr.plen = htons(0);
	memcpy(buf, &hdr, HCRP_PDU_HDR_SIZE);
	len = write(sk, buf, HCRP_PDU_HDR_SIZE);
	if (len < 0)
		return len;

	len = read(sk, buf, sizeof(buf));
	if (len < 0)
		return len;

	memcpy(&hdr, buf, HCRP_PDU_HDR_SIZE);
	memcpy(&rp, buf + HCRP_PDU_HDR_SIZE, HCRP_CREDIT_REQUEST_RP_SIZE);

	if (ntohs(rp.status) != HCRP_STATUS_SUCCESS) {
		errno = EIO;
		return -1;
	}

	if (credit)
		*credit = ntohl(rp.credit);

	return 0;
}

static int hcrp_get_lpt_status(int sk, uint16_t tid, uint8_t *lpt_status)
{
	struct hcrp_pdu_hdr hdr;
	struct hcrp_get_lpt_status_rp rp;
	unsigned char buf[128];
	int len;

	hdr.pid = htons(HCRP_PDU_GET_LPT_STATUS);
	hdr.tid = htons(tid);
	hdr.plen = htons(0);
	memcpy(buf, &hdr, HCRP_PDU_HDR_SIZE);
	len = write(sk, buf, HCRP_PDU_HDR_SIZE);
	if (len < 0)
		return len;

	len = read(sk, buf, sizeof(buf));
	if (len < 0)
		return len;

	memcpy(&hdr, buf, HCRP_PDU_HDR_SIZE);
	memcpy(&rp, buf + HCRP_PDU_HDR_SIZE, HCRP_GET_LPT_STATUS_RP_SIZE);

	if (ntohs(rp.status) != HCRP_STATUS_SUCCESS) {
		errno = EIO;
		return -1;
	}

	if (lpt_status)
		*lpt_status = rp.lpt_status;

	return 0;
}

static inline int hcrp_get_next_tid(int tid)
{
	if (tid > 0xf000)
		return 0;
	else
		return tid + 1;
}

int hcrp_print(bdaddr_t *src, bdaddr_t *dst, unsigned short ctrl_psm, unsigned short data_psm, int fd, int copies, const char *cups_class)
{
	struct sockaddr_l2 addr;
	struct l2cap_options opts;
	socklen_t size;
	unsigned char buf[2048];
	int i, ctrl_sk, data_sk, count, len, timeout = 0;
	unsigned int mtu;
	uint8_t status;
	uint16_t tid = 0;
	uint32_t tmp, credit = 0;

	if ((ctrl_sk = socket(PF_BLUETOOTH, SOCK_SEQPACKET, BTPROTO_L2CAP)) < 0) {
		perror("ERROR: Can't create socket");
		if (cups_class)
			return CUPS_BACKEND_FAILED;
		else
			return CUPS_BACKEND_RETRY;
	}

	memset(&addr, 0, sizeof(addr));
	addr.l2_family = AF_BLUETOOTH;
	bacpy(&addr.l2_bdaddr, src);

	if (bind(ctrl_sk, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
		perror("ERROR: Can't bind socket");
		close(ctrl_sk);
		if (cups_class)
			return CUPS_BACKEND_FAILED;
		else
			return CUPS_BACKEND_RETRY;
	}

	memset(&addr, 0, sizeof(addr));
	addr.l2_family = AF_BLUETOOTH;
	bacpy(&addr.l2_bdaddr, dst);
	addr.l2_psm = htobs(ctrl_psm);

	if (connect(ctrl_sk, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
		perror("ERROR: Can't connect to device");
		close(ctrl_sk);
		if (cups_class)
			return CUPS_BACKEND_FAILED;
		else
			return CUPS_BACKEND_RETRY;
	}

	if ((data_sk = socket(PF_BLUETOOTH, SOCK_SEQPACKET, BTPROTO_L2CAP)) < 0) {
		perror("ERROR: Can't create socket");
		close(ctrl_sk);
		if (cups_class)
			return CUPS_BACKEND_FAILED;
		else
			return CUPS_BACKEND_RETRY;
	}

	memset(&addr, 0, sizeof(addr));
	addr.l2_family = AF_BLUETOOTH;
	bacpy(&addr.l2_bdaddr, src);

	if (bind(data_sk, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
		perror("ERROR: Can't bind socket");
		close(data_sk);
		close(ctrl_sk);
		if (cups_class)
			return CUPS_BACKEND_FAILED;
		else
			return CUPS_BACKEND_RETRY;
	}

	memset(&addr, 0, sizeof(addr));
	addr.l2_family = AF_BLUETOOTH;
	bacpy(&addr.l2_bdaddr, dst);
	addr.l2_psm = htobs(data_psm);

	if (connect(data_sk, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
		perror("ERROR: Can't connect to device");
		close(data_sk);
		close(ctrl_sk);
		if (cups_class)
			return CUPS_BACKEND_FAILED;
		else
			return CUPS_BACKEND_RETRY;
	}

	fputs("STATE: -connecting-to-device\n", stderr);

	memset(&opts, 0, sizeof(opts));
	size = sizeof(opts);

	if (getsockopt(data_sk, SOL_L2CAP, L2CAP_OPTIONS, &opts, &size) < 0) {
		perror("ERROR: Can't get socket options");
		close(data_sk);
		close(ctrl_sk);
		if (cups_class)
			return CUPS_BACKEND_FAILED;
		else
			return CUPS_BACKEND_RETRY;
	}

	mtu = opts.omtu;

	/* Ignore SIGTERM signals if printing from stdin */
	if (fd == 0) {
#ifdef HAVE_SIGSET
		sigset(SIGTERM, SIG_IGN);
#elif defined(HAVE_SIGACTION)
		memset(&action, 0, sizeof(action));
		sigemptyset(&action.sa_mask);
		action.sa_handler = SIG_IGN;
		sigaction(SIGTERM, &action, NULL);
#else
		signal(SIGTERM, SIG_IGN);
#endif /* HAVE_SIGSET */
	}

	tid = hcrp_get_next_tid(tid);
	if (hcrp_credit_grant(ctrl_sk, tid, 0) < 0) {
		fprintf(stderr, "ERROR: Can't grant initial credits\n");
		close(data_sk);
		close(ctrl_sk);
		if (cups_class)
			return CUPS_BACKEND_FAILED;
		else
			return CUPS_BACKEND_RETRY;
	}

	for (i = 0; i < copies; i++) {

		if (fd != 0) {
			fprintf(stderr, "PAGE: 1 1\n");
			lseek(fd, 0, SEEK_SET);
		}

		while (1) {
			if (credit < mtu) {
				tid = hcrp_get_next_tid(tid);
				if (!hcrp_credit_request(ctrl_sk, tid, &tmp)) {
					credit += tmp;
					timeout = 0;
				}
			}

			if (!credit) {
				if (timeout++ > 300) {
					tid = hcrp_get_next_tid(tid);
					if (!hcrp_get_lpt_status(ctrl_sk, tid, &status))
						fprintf(stderr, "ERROR: LPT status 0x%02x\n", status);
					break;
				}

				sleep(1);
				continue;
			}

			count = read(fd, buf, (credit > mtu) ? mtu : credit);
			if (count <= 0)
				break;

			len = write(data_sk, buf, count);
			if (len < 0) {
				perror("ERROR: Error writing to device");
				close(data_sk);
				close(ctrl_sk);
				return CUPS_BACKEND_FAILED;
			}

			if (len != count)
				fprintf(stderr, "ERROR: Can't send complete data\n");

			credit -= len;
		}

	}

	close(data_sk);
	close(ctrl_sk);

	return CUPS_BACKEND_OK;
}
