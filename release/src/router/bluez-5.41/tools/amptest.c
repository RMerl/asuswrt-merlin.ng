/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2011-2012  Intel Corporation
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
#include <unistd.h>
#include <alloca.h>
#include <stdlib.h>
#include <stdbool.h>
#include <poll.h>
#include <sys/ioctl.h>
#include <sys/socket.h>

#include "lib/bluetooth.h"
#include "lib/hci.h"
#include "lib/hci_lib.h"

static int activate_amp_controller(int dev_id)
{
	struct hci_dev_info di;
	struct hci_filter flt;
	int fd;

	printf("hci%d: Activating controller\n", dev_id);

	fd = socket(AF_BLUETOOTH, SOCK_RAW, BTPROTO_HCI);
	if (fd < 0) {
		perror("Failed to open raw HCI socket");
		return -1;
	}

	di.dev_id = dev_id;

	if (ioctl(fd, HCIGETDEVINFO, (void *) &di) < 0) {
		perror("Failed to get HCI device info");
		close(fd);
		return -1;
	}

	if (!hci_test_bit(HCI_UP, &di.flags)) {
		if (ioctl(fd, HCIDEVUP, dev_id) < 0) {
			if (errno != EALREADY) {
				perror("Failed to bring up HCI device");
				close(fd);
				return -1;
			}
		}
	}

	close(fd);

	fd = hci_open_dev(dev_id);
	if (fd < 0) {
		perror("Failed to open HCI device");
		return -1;
	}

	hci_filter_clear(&flt);
	hci_filter_set_ptype(HCI_EVENT_PKT, &flt);
	hci_filter_set_event(EVT_CHANNEL_SELECTED, &flt);
	hci_filter_set_event(EVT_PHYSICAL_LINK_COMPLETE, &flt);
	hci_filter_set_event(EVT_DISCONNECT_PHYSICAL_LINK_COMPLETE, &flt);

	if (setsockopt(fd, SOL_HCI, HCI_FILTER, &flt, sizeof(flt)) < 0) {
		perror("Failed to setup HCI device filter");
		close(fd);
		return -1;
	}

	return fd;
}

static bool read_local_amp_info(int dev_id, uint16_t *max_assoc_len)
{
	read_local_amp_info_rp rp;
	struct hci_request rq;
	int fd;

	printf("hci%d: Reading local AMP information\n", dev_id);

	fd = hci_open_dev(dev_id);
	if (fd < 0) {
		perror("Failed to open HCI device");
		return false;
	}

	memset(&rp, 0, sizeof(rp));

	memset(&rq, 0, sizeof(rq));
	rq.ogf    = OGF_STATUS_PARAM;
	rq.ocf    = OCF_READ_LOCAL_AMP_INFO;
	rq.rparam = &rp;
	rq.rlen   = READ_LOCAL_AMP_INFO_RP_SIZE;

	if (hci_send_req(fd, &rq, 1000) < 0) {
		perror("Failed sending HCI request");
		hci_close_dev(fd);
		return false;
	}

	if (rp.status) {
		fprintf(stderr, "Failed HCI command: 0x%02x\n", rp.status);
		hci_close_dev(fd);
		return false;
	}

	printf("\tAMP status: 0x%02x\n", rp.amp_status);
	printf("\tController type: 0x%02x\n", rp.controller_type);
	printf("\tMax ASSOC length: %d\n", btohs(rp.max_amp_assoc_length));

	*max_assoc_len = btohs(rp.max_amp_assoc_length);

	hci_close_dev(fd);

	return true;
}

static bool read_local_amp_assoc(int dev_id, uint8_t phy_handle,
							uint16_t max_assoc_len,
							uint8_t *assoc_data,
							uint16_t *assoc_len)
{
	read_local_amp_assoc_cp cp;
	read_local_amp_assoc_rp rp;
	struct hci_request rq;
	int fd;

	printf("hci%d: Reading local AMP association\n", dev_id);

	fd = hci_open_dev(dev_id);
	if (fd < 0) {
		perror("Failed to open HCI device");
		return false;
	}

	memset(&cp, 0, sizeof(cp));
	cp.handle = phy_handle;
	cp.length_so_far = htobs(0);
	cp.assoc_length = htobs(max_assoc_len);
	memset(&rp, 0, sizeof(rp));

	memset(&rq, 0, sizeof(rq));
	rq.ogf    = OGF_STATUS_PARAM;
	rq.ocf    = OCF_READ_LOCAL_AMP_ASSOC;
	rq.cparam = &cp;
	rq.clen   = READ_LOCAL_AMP_ASSOC_CP_SIZE;
	rq.rparam = &rp;
	rq.rlen   = READ_LOCAL_AMP_ASSOC_RP_SIZE;

	if (hci_send_req(fd, &rq, 1000) < 0) {
		perror("Failed sending HCI request");
		hci_close_dev(fd);
		return false;
	}

	if (rp.status) {
		fprintf(stderr, "Failed HCI command: 0x%02x\n", rp.status);
		hci_close_dev(fd);
		return false;
	}

	printf("\tRemain ASSOC length: %d\n", btohs(rp.length));

	*assoc_len = btohs(rp.length);
	memcpy(assoc_data, rp.fragment, *assoc_len);

	hci_close_dev(fd);

	return true;
}

static bool write_remote_amp_assoc(int dev_id, uint8_t phy_handle,
							uint8_t *assoc_data,
							uint16_t assoc_len)
{
	write_remote_amp_assoc_cp cp;
	write_remote_amp_assoc_rp rp;
	struct hci_request rq;
	int fd;

	printf("hci%d: Writing remote AMP association\n", dev_id);

	fd = hci_open_dev(dev_id);
	if (fd < 0) {
		perror("Failed to open HCI device");
		return false;
	}

	memset(&cp, 0, sizeof(cp));
	cp.handle = phy_handle;
	cp.length_so_far = htobs(0);
	cp.remaining_length = htobs(assoc_len);
	memcpy(cp.fragment, assoc_data, assoc_len);
	memset(&rp, 0, sizeof(rp));

	memset(&rq, 0, sizeof(rq));
	rq.ogf    = OGF_STATUS_PARAM;
	rq.ocf    = OCF_WRITE_REMOTE_AMP_ASSOC;
	rq.cparam = &cp;
	rq.clen   = 5 + assoc_len;
	rq.rparam = &rp;
	rq.rlen   = WRITE_REMOTE_AMP_ASSOC_RP_SIZE;

	if (hci_send_req(fd, &rq, 1000) < 0) {
		perror("Failed sending HCI request");
		hci_close_dev(fd);
		return false;
	}

	if (rp.status) {
		fprintf(stderr, "Failed HCI command: 0x%02x\n", rp.status);
		hci_close_dev(fd);
		return false;
	}

	hci_close_dev(fd);

	return true;
}

static bool channel_selected_event(int dev_id, int fd, uint8_t phy_handle)
{
	printf("hci%d: Waiting for channel selected event\n", dev_id);

	while (1) {
		uint8_t buf[HCI_MAX_EVENT_SIZE];
		hci_event_hdr *hdr;
		struct pollfd p;
		int n, len;

		p.fd = fd;
		p.events = POLLIN;

		n = poll(&p, 1, 10000);
		if (n < 0) {
			if (errno == EAGAIN || errno == EINTR)
				continue;

			perror("Failed to poll HCI device");
			return false;
		}

		if (n == 0) {
			fprintf(stderr, "Failure to receive event\n");
			return false;
		}

		len = read(fd, buf, sizeof(buf));
		if (len < 0) {
			if (errno == EAGAIN || errno == EINTR)
				continue;

			perror("Failed to read from HCI device");
			return false;
		}

		hdr = (void *) (buf + 1);

		if (hdr->evt == EVT_CHANNEL_SELECTED)
			break;
	}

	return true;
}

static bool create_physical_link(int dev_id, uint8_t phy_handle)
{
	create_physical_link_cp cp;
	evt_cmd_status evt;
	struct hci_request rq;
	int i, fd;

	printf("hci%d: Creating physical link\n", dev_id);

	fd = hci_open_dev(dev_id);
	if (fd < 0) {
		perror("Failed to open HCI device");
		return false;
	}

	memset(&cp, 0, sizeof(cp));
	cp.handle = phy_handle;
	cp.key_length = 32;
	cp.key_type = 0x03;
	for (i = 0; i < cp.key_length; i++)
		cp.key[i] = 0x23;
	memset(&evt, 0, sizeof(evt));

	memset(&rq, 0, sizeof(rq));
	rq.ogf    = OGF_LINK_CTL;
	rq.ocf    = OCF_CREATE_PHYSICAL_LINK;
	rq.event  = EVT_CMD_STATUS;
	rq.cparam = &cp;
	rq.clen   = CREATE_PHYSICAL_LINK_CP_SIZE;
	rq.rparam = &evt;
	rq.rlen   = EVT_CMD_STATUS_SIZE;

	if (hci_send_req(fd, &rq, 1000) < 0) {
		perror("Failed sending HCI request");
		hci_close_dev(fd);
		return false;
	}

	if (evt.status) {
		fprintf(stderr, "Failed HCI command: 0x%02x\n", evt.status);
		hci_close_dev(fd);
		return false;
	}

	hci_close_dev(fd);

	return true;
}

static bool accept_physical_link(int dev_id, uint8_t phy_handle)
{
	accept_physical_link_cp cp;
	evt_cmd_status evt;
	struct hci_request rq;
	int i, fd;

	printf("hci%d: Accepting physical link\n", dev_id);

	fd = hci_open_dev(dev_id);
	if (fd < 0) {
		perror("Failed to open HCI device");
		return false;
	}

	memset(&cp, 0, sizeof(cp));
	cp.handle = phy_handle;
	cp.key_length = 32;
	cp.key_type = 0x03;
	for (i = 0; i < cp.key_length; i++)
		cp.key[i] = 0x23;
	memset(&evt, 0, sizeof(evt));

	memset(&rq, 0, sizeof(rq));
	rq.ogf    = OGF_LINK_CTL;
	rq.ocf    = OCF_ACCEPT_PHYSICAL_LINK;
	rq.event  = EVT_CMD_STATUS;
	rq.cparam = &cp;
	rq.clen   = ACCEPT_PHYSICAL_LINK_CP_SIZE;
	rq.rparam = &evt;
	rq.rlen   = EVT_CMD_STATUS_SIZE;

	if (hci_send_req(fd, &rq, 1000) < 0) {
		perror("Failed sending HCI request");
		hci_close_dev(fd);
		return false;
	}

	if (evt.status) {
		fprintf(stderr, "Failed HCI command: 0x%02x\n", evt.status);
		hci_close_dev(fd);
		return false;
	}

	hci_close_dev(fd);

	return true;
}

static bool disconnect_physical_link(int dev_id, uint8_t phy_handle,
							uint8_t reason)
{
	disconnect_physical_link_cp cp;
	evt_cmd_status evt;
	struct hci_request rq;
	int fd;

	printf("hci%d: Disconnecting physical link\n", dev_id);

	fd = hci_open_dev(dev_id);
	if (fd < 0) {
		perror("Failed to open HCI device");
		return false;
	}

	memset(&cp, 0, sizeof(cp));
	cp.handle = phy_handle;
	cp.reason = reason;

	memset(&rq, 0, sizeof(rq));
	rq.ogf    = OGF_LINK_CTL;
	rq.ocf    = OCF_DISCONNECT_PHYSICAL_LINK;
	rq.event  = EVT_CMD_STATUS;
	rq.cparam = &cp;
	rq.clen   = DISCONNECT_PHYSICAL_LINK_CP_SIZE;
	rq.rparam = &evt;
	rq.rlen   = EVT_CMD_STATUS_SIZE;

	if (hci_send_req(fd, &rq, 1000) < 0) {
		perror("Failed sending HCI request");
		hci_close_dev(fd);
		return false;
	}

	if (evt.status) {
		fprintf(stderr, "Failed HCI command: 0x%02x\n", evt.status);
		hci_close_dev(fd);
		return false;
	}

	hci_close_dev(fd);

	return true;
}

static bool physical_link_complete_event(int dev_id, int fd,
							uint8_t phy_handle)
{
	printf("hci%d: Waiting for physical link complete event\n", dev_id);

	while (1) {
		uint8_t buf[HCI_MAX_EVENT_SIZE];
		hci_event_hdr *hdr;
		int len;

		len = read(fd, buf, sizeof(buf));
		if (len < 0) {
			if (errno == EAGAIN || errno == EINTR)
				continue;

			perror("Failed to read from HCI device");
			return false;
		}

		hdr = (void *) (buf + 1);

		if (hdr->evt == EVT_PHYSICAL_LINK_COMPLETE)
			break;
	}

	return true;
}

static bool disconnect_physical_link_complete_event(int dev_id, int fd,
							uint8_t phy_handle)
{
	printf("hci%d: Waiting for physical link disconnect event\n", dev_id);

	while (1) {
		uint8_t buf[HCI_MAX_EVENT_SIZE];
		hci_event_hdr *hdr;
		int len;

		len = read(fd, buf, sizeof(buf));
		if (len < 0) {
			if (errno == EAGAIN || errno == EINTR)
				continue;

			perror("Failed to read from HCI device");
			return false;
		}

		hdr = (void *) (buf + 1);

		if (hdr->evt == EVT_DISCONNECT_PHYSICAL_LINK_COMPLETE)
			break;
	}

	return true;
}

static int amp1_dev_id = -1;
static int amp2_dev_id = -1;

static bool find_amp_controller(void)
{
	struct hci_dev_list_req *dl;
	struct hci_dev_req *dr;
	int fd, i;
	bool result;

	fd = socket(AF_BLUETOOTH, SOCK_RAW, BTPROTO_HCI);
	if (fd < 0) {
		perror("Failed to open raw HCI socket");
		return false;
	}

	dl = malloc(HCI_MAX_DEV * sizeof(struct hci_dev_req) + sizeof(uint16_t));
	if (!dl) {
		perror("Failed allocate HCI device request memory");
		close(fd);
		return false;
	}

	dl->dev_num = HCI_MAX_DEV;
	dr = dl->dev_req;

	if (ioctl(fd, HCIGETDEVLIST, (void *) dl) < 0) {
		perror("Failed to get HCI device list");
		result = false;
		goto done;
	}

	for (i = 0; i< dl->dev_num; i++) {
		struct hci_dev_info di;

		di.dev_id = (dr + i)->dev_id;

		if (ioctl(fd, HCIGETDEVINFO, (void *) &di) < 0)
			continue;

		if (((di.type & 0x30) >> 4) != HCI_AMP)
			continue;

		if (amp1_dev_id < 0)
			amp1_dev_id = di.dev_id;
		else if (amp2_dev_id < 0) {
			if (di.dev_id < amp1_dev_id) {
				amp2_dev_id = amp1_dev_id;
				amp1_dev_id = di.dev_id;
			} else
				amp2_dev_id = di.dev_id;
		}
	}

	result = true;

done:
	free(dl);
	close(fd);
	return result;
}

int main(int argc ,char *argv[])
{
	int amp1_event_fd, amp2_event_fd;
	uint16_t amp1_max_assoc_len, amp2_max_assoc_len;
	uint8_t *amp1_assoc_data, *amp2_assoc_data;
	uint16_t amp1_assoc_len, amp2_assoc_len;
	uint8_t amp1_phy_handle, amp2_phy_handle;

	if (!find_amp_controller())
		return EXIT_FAILURE;

	if (amp1_dev_id < 0 || amp2_dev_id < 0) {
		fprintf(stderr, "Two AMP controllers are required\n");
		return EXIT_FAILURE;
	}

	printf("hci%d: AMP initiator\n", amp1_dev_id);
	printf("hci%d: AMP acceptor\n", amp2_dev_id);

	amp1_event_fd = activate_amp_controller(amp1_dev_id);
	if (amp1_event_fd < 0)
		return EXIT_FAILURE;

	amp2_event_fd = activate_amp_controller(amp2_dev_id);
	if (amp2_event_fd < 0) {
		hci_close_dev(amp1_event_fd);
		return EXIT_FAILURE;
	}

	if (!read_local_amp_info(amp1_dev_id, &amp1_max_assoc_len))
		return EXIT_FAILURE;

	amp1_assoc_data = alloca(amp1_max_assoc_len);

	printf("--> AMP_Get_Info_Request (Amp_ID B)\n");

	if (!read_local_amp_info(amp2_dev_id, &amp2_max_assoc_len))
		return EXIT_FAILURE;

	amp2_assoc_data = alloca(amp2_max_assoc_len);

	printf("<-- AMP_Get_Info_Response (Amp_ID B, Status)\n");

	printf("--> AMP_Get_AMP_Assoc_Request (Amp_ID B)\n");

	if (!read_local_amp_assoc(amp2_dev_id, 0x00, amp2_max_assoc_len,
					amp2_assoc_data, &amp2_assoc_len))
		return EXIT_FAILURE;

	printf("<-- AMP_Get_AMP_Assoc_Response (Amp_ID B, AMP_Assoc B)\n");

	amp1_phy_handle = 0x04;

	if (!create_physical_link(amp1_dev_id, amp1_phy_handle))
		return EXIT_FAILURE;

	if (!write_remote_amp_assoc(amp1_dev_id, amp1_phy_handle,
					amp2_assoc_data, amp2_assoc_len))
		return EXIT_FAILURE;

	printf("hci%d: Signal MAC to scan\n", amp1_dev_id);

	printf("hci%d: Signal MAC to start\n", amp1_dev_id);

	if (!channel_selected_event(amp1_dev_id, amp1_event_fd,
							amp1_phy_handle))
		return EXIT_FAILURE;

	if (!read_local_amp_assoc(amp1_dev_id, amp1_phy_handle,
					amp1_max_assoc_len,
					amp1_assoc_data, &amp1_assoc_len))
		return EXIT_FAILURE;

	printf("--> AMP_Create_Physical_Link_Request (Remote-Amp-ID B, AMP_Assoc A)\n");

	amp2_phy_handle = 0x05;

	if (!accept_physical_link(amp2_dev_id, amp2_phy_handle))
		return EXIT_FAILURE;

	if (!write_remote_amp_assoc(amp2_dev_id, amp2_phy_handle,
					amp1_assoc_data, amp1_assoc_len))
		return EXIT_FAILURE;

	printf("hci%d: Signal MAC to start\n", amp2_dev_id);

	printf("<-- AMP_Create_Physical_Link_Response (Local-Amp-ID B, Status)\n");

	if (!physical_link_complete_event(amp2_dev_id, amp2_event_fd,
							amp2_phy_handle))
		return EXIT_FAILURE;

	if (!physical_link_complete_event(amp1_dev_id, amp1_event_fd,
							amp1_phy_handle))
		return EXIT_FAILURE;

	/* physical link established */

	if (!disconnect_physical_link(amp1_dev_id, amp1_phy_handle, 0x13))
		return EXIT_FAILURE;

	if (!disconnect_physical_link_complete_event(amp1_dev_id,
							amp1_event_fd,
							amp1_phy_handle))
		return EXIT_FAILURE;

	if (!disconnect_physical_link_complete_event(amp2_dev_id,
							amp2_event_fd,
							amp2_phy_handle))
		return EXIT_FAILURE;

	hci_close_dev(amp2_event_fd);
	hci_close_dev(amp1_event_fd);

	return EXIT_SUCCESS;
}
