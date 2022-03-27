/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2000-2001  Qualcomm Incorporated
 *  Copyright (C) 2002-2003  Maxim Krasnyansky <maxk@qualcomm.com>
 *  Copyright (C) 2002-2010  Marcel Holtmann <marcel@holtmann.org>
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
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <sys/param.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/stat.h>

#include "lib/bluetooth.h"
#include "lib/hci.h"
#include "lib/hci_lib.h"

#include "src/textfile.h"
#include "src/shared/util.h"
#include "tools/csr.h"

static struct hci_dev_info di;
static int all;

static void print_dev_hdr(struct hci_dev_info *di);
static void print_dev_info(int ctl, struct hci_dev_info *di);

static void print_dev_list(int ctl, int flags)
{
	struct hci_dev_list_req *dl;
	struct hci_dev_req *dr;
	int i;

	if (!(dl = malloc(HCI_MAX_DEV * sizeof(struct hci_dev_req) +
		sizeof(uint16_t)))) {
		perror("Can't allocate memory");
		exit(1);
	}
	dl->dev_num = HCI_MAX_DEV;
	dr = dl->dev_req;

	if (ioctl(ctl, HCIGETDEVLIST, (void *) dl) < 0) {
		perror("Can't get device list");
		free(dl);
		exit(1);
	}

	for (i = 0; i< dl->dev_num; i++) {
		di.dev_id = (dr+i)->dev_id;
		if (ioctl(ctl, HCIGETDEVINFO, (void *) &di) < 0)
			continue;
		print_dev_info(ctl, &di);
	}

	free(dl);
}

static void print_pkt_type(struct hci_dev_info *di)
{
	char *str;
	str = hci_ptypetostr(di->pkt_type);
	printf("\tPacket type: %s\n", str);
	bt_free(str);
}

static void print_link_policy(struct hci_dev_info *di)
{
	printf("\tLink policy: %s\n", hci_lptostr(di->link_policy));
}

static void print_link_mode(struct hci_dev_info *di)
{
	char *str;
	str =  hci_lmtostr(di->link_mode);
	printf("\tLink mode: %s\n", str);
	bt_free(str);
}

static void print_dev_features(struct hci_dev_info *di, int format)
{
	printf("\tFeatures: 0x%2.2x 0x%2.2x 0x%2.2x 0x%2.2x "
				"0x%2.2x 0x%2.2x 0x%2.2x 0x%2.2x\n",
		di->features[0], di->features[1], di->features[2],
		di->features[3], di->features[4], di->features[5],
		di->features[6], di->features[7]);

	if (format) {
		char *tmp = lmp_featurestostr(di->features, "\t\t", 63);
		printf("%s\n", tmp);
		bt_free(tmp);
	}
}

static void print_le_states(uint64_t states)
{
	int i;
	const char *le_states[] = {
		"Non-connectable Advertising State" ,
		"Scannable Advertising State",
		"Connectable Advertising State",
		"Directed Advertising State",
		"Passive Scanning State",
		"Active Scanning State",
		"Initiating State/Connection State in Master Role",
		"Connection State in the Slave Role",
		"Non-connectable Advertising State and Passive Scanning State combination",
		"Scannable Advertising State and Passive Scanning State combination",
		"Connectable Advertising State and Passive Scanning State combination",
		"Directed Advertising State and Passive Scanning State combination",
		"Non-connectable Advertising State and Active Scanning State combination",
		"Scannable Advertising State and Active Scanning State combination",
		"Connectable Advertising State and Active Scanning State combination",
		"Directed Advertising State and Active Scanning State combination",
		"Non-connectable Advertising State and Initiating State combination",
		"Scannable Advertising State and Initiating State combination",
		"Non-connectable Advertising State and Master Role combination",
		"Scannable Advertising State and Master Role combination",
		"Non-connectable Advertising State and Slave Role combination",
		"Scannable Advertising State and Slave Role combination",
		"Passive Scanning State and Initiating State combination",
		"Active Scanning State and Initiating State combination",
		"Passive Scanning State and Master Role combination",
		"Active Scanning State and Master Role combination",
		"Passive Scanning State and Slave Role combination",
		"Active Scanning State and Slave Role combination",
		"Initiating State and Master Role combination/Master Role and Master Role combination",
		NULL
	};

	printf("Supported link layer states:\n");
	for (i = 0; le_states[i]; i++) {
		const char *status;

		status = states & (1 << i) ? "YES" : "NO ";
		printf("\t%s %s\n", status, le_states[i]);
	}
}

static void cmd_rstat(int ctl, int hdev, char *opt)
{
	/* Reset HCI device stat counters */
	if (ioctl(ctl, HCIDEVRESTAT, hdev) < 0) {
		fprintf(stderr, "Can't reset stats counters hci%d: %s (%d)\n",
						hdev, strerror(errno), errno);
		exit(1);
	}
}

static void cmd_scan(int ctl, int hdev, char *opt)
{
	struct hci_dev_req dr;

	dr.dev_id  = hdev;
	dr.dev_opt = SCAN_DISABLED;
	if (!strcmp(opt, "iscan"))
		dr.dev_opt = SCAN_INQUIRY;
	else if (!strcmp(opt, "pscan"))
		dr.dev_opt = SCAN_PAGE;
	else if (!strcmp(opt, "piscan"))
		dr.dev_opt = SCAN_PAGE | SCAN_INQUIRY;

	if (ioctl(ctl, HCISETSCAN, (unsigned long) &dr) < 0) {
		fprintf(stderr, "Can't set scan mode on hci%d: %s (%d)\n",
						hdev, strerror(errno), errno);
		exit(1);
	}
}

static void cmd_le_addr(int ctl, int hdev, char *opt)
{
	struct hci_request rq;
	le_set_random_address_cp cp;
	uint8_t status;
	int dd, err, ret;

	if (!opt)
		return;

	if (hdev < 0)
		hdev = hci_get_route(NULL);

	dd = hci_open_dev(hdev);
	if (dd < 0) {
		err = -errno;
		fprintf(stderr, "Could not open device: %s(%d)\n",
							strerror(-err), -err);
		exit(1);
	}

	memset(&cp, 0, sizeof(cp));

	str2ba(opt, &cp.bdaddr);

	memset(&rq, 0, sizeof(rq));
	rq.ogf = OGF_LE_CTL;
	rq.ocf = OCF_LE_SET_RANDOM_ADDRESS;
	rq.cparam = &cp;
	rq.clen = LE_SET_RANDOM_ADDRESS_CP_SIZE;
	rq.rparam = &status;
	rq.rlen = 1;

	ret = hci_send_req(dd, &rq, 1000);
	if (status || ret < 0) {
		err = -errno;
		fprintf(stderr, "Can't set random address for hci%d: "
				"%s (%d)\n", hdev, strerror(-err), -err);
	}

	hci_close_dev(dd);
}

static void cmd_le_adv(int ctl, int hdev, char *opt)
{
	struct hci_request rq;
	le_set_advertise_enable_cp advertise_cp;
	le_set_advertising_parameters_cp adv_params_cp;
	uint8_t status;
	int dd, ret;

	if (hdev < 0)
		hdev = hci_get_route(NULL);

	dd = hci_open_dev(hdev);
	if (dd < 0) {
		perror("Could not open device");
		exit(1);
	}

	memset(&adv_params_cp, 0, sizeof(adv_params_cp));
	adv_params_cp.min_interval = htobs(0x0800);
	adv_params_cp.max_interval = htobs(0x0800);
	if (opt)
		adv_params_cp.advtype = atoi(opt);
	adv_params_cp.chan_map = 7;

	memset(&rq, 0, sizeof(rq));
	rq.ogf = OGF_LE_CTL;
	rq.ocf = OCF_LE_SET_ADVERTISING_PARAMETERS;
	rq.cparam = &adv_params_cp;
	rq.clen = LE_SET_ADVERTISING_PARAMETERS_CP_SIZE;
	rq.rparam = &status;
	rq.rlen = 1;

	ret = hci_send_req(dd, &rq, 1000);
	if (ret < 0)
		goto done;

	memset(&advertise_cp, 0, sizeof(advertise_cp));
	advertise_cp.enable = 0x01;

	memset(&rq, 0, sizeof(rq));
	rq.ogf = OGF_LE_CTL;
	rq.ocf = OCF_LE_SET_ADVERTISE_ENABLE;
	rq.cparam = &advertise_cp;
	rq.clen = LE_SET_ADVERTISE_ENABLE_CP_SIZE;
	rq.rparam = &status;
	rq.rlen = 1;

	ret = hci_send_req(dd, &rq, 1000);

done:
	hci_close_dev(dd);

	if (ret < 0) {
		fprintf(stderr, "Can't set advertise mode on hci%d: %s (%d)\n",
						hdev, strerror(errno), errno);
		exit(1);
	}

	if (status) {
		fprintf(stderr,
			"LE set advertise enable on hci%d returned status %d\n",
								hdev, status);
		exit(1);
	}
}

static void cmd_no_le_adv(int ctl, int hdev, char *opt)
{
	struct hci_request rq;
	le_set_advertise_enable_cp advertise_cp;
	uint8_t status;
	int dd, ret;

	if (hdev < 0)
		hdev = hci_get_route(NULL);

	dd = hci_open_dev(hdev);
	if (dd < 0) {
		perror("Could not open device");
		exit(1);
	}

	memset(&advertise_cp, 0, sizeof(advertise_cp));

	memset(&rq, 0, sizeof(rq));
	rq.ogf = OGF_LE_CTL;
	rq.ocf = OCF_LE_SET_ADVERTISE_ENABLE;
	rq.cparam = &advertise_cp;
	rq.clen = LE_SET_ADVERTISE_ENABLE_CP_SIZE;
	rq.rparam = &status;
	rq.rlen = 1;

	ret = hci_send_req(dd, &rq, 1000);

	hci_close_dev(dd);

	if (ret < 0) {
		fprintf(stderr, "Can't set advertise mode on hci%d: %s (%d)\n",
						hdev, strerror(errno), errno);
		exit(1);
	}

	if (status) {
		fprintf(stderr, "LE set advertise enable on hci%d returned status %d\n",
						hdev, status);
		exit(1);
	}
}

static void cmd_le_states(int ctl, int hdev, char *opt)
{
	le_read_supported_states_rp rp;
	struct hci_request rq;
	int err, dd;

	if (hdev < 0)
		hdev = hci_get_route(NULL);

	dd = hci_open_dev(hdev);
	if (dd < 0) {
		fprintf(stderr, "Can't open device hci%d: %s (%d)\n",
						hdev, strerror(errno), errno);
		exit(1);
	}

	memset(&rp, 0, sizeof(rp));
	memset(&rq, 0, sizeof(rq));

	rq.ogf    = OGF_LE_CTL;
	rq.ocf    = OCF_LE_READ_SUPPORTED_STATES;
	rq.rparam = &rp;
	rq.rlen   = LE_READ_SUPPORTED_STATES_RP_SIZE;

	err = hci_send_req(dd, &rq, 1000);

	hci_close_dev(dd);

	if (err < 0) {
		fprintf(stderr, "Can't read LE supported states on hci%d:"
				" %s(%d)\n", hdev, strerror(errno), errno);
		exit(1);
	}

	if (rp.status) {
		fprintf(stderr, "Read LE supported states on hci%d"
				" returned status %d\n", hdev, rp.status);
		exit(1);
	}

	print_le_states(rp.states);
}

static void cmd_iac(int ctl, int hdev, char *opt)
{
	int s = hci_open_dev(hdev);

	if (s < 0) {
		fprintf(stderr, "Can't open device hci%d: %s (%d)\n",
						hdev, strerror(errno), errno);
		exit(1);
	}
	if (opt) {
		int l = strtoul(opt, 0, 16);
		uint8_t lap[3];
		if (!strcasecmp(opt, "giac")) {
			l = 0x9e8b33;
		} else if (!strcasecmp(opt, "liac")) {
			l = 0x9e8b00;
		} else if (l < 0x9e8b00 || l > 0x9e8b3f) {
			printf("Invalid access code 0x%x\n", l);
			exit(1);
		}
		lap[0] = (l & 0xff);
		lap[1] = (l >> 8) & 0xff;
		lap[2] = (l >> 16) & 0xff;
		if (hci_write_current_iac_lap(s, 1, lap, 1000) < 0) {
			printf("Failed to set IAC on hci%d: %s\n", hdev, strerror(errno));
			exit(1);
		}
	} else {
		uint8_t lap[3 * MAX_IAC_LAP];
		int i, j;
		uint8_t n;
		if (hci_read_current_iac_lap(s, &n, lap, 1000) < 0) {
			printf("Failed to read IAC from hci%d: %s\n", hdev, strerror(errno));
			exit(1);
		}
		print_dev_hdr(&di);
		printf("\tIAC: ");
		for (i = 0; i < n; i++) {
			printf("0x");
			for (j = 3; j--; )
				printf("%02x", lap[j + 3 * i]);
			if (i < n - 1)
				printf(", ");
		}
		printf("\n");
	}
	close(s);
}

static void cmd_auth(int ctl, int hdev, char *opt)
{
	struct hci_dev_req dr;

	dr.dev_id = hdev;
	if (!strcmp(opt, "auth"))
		dr.dev_opt = AUTH_ENABLED;
	else
		dr.dev_opt = AUTH_DISABLED;

	if (ioctl(ctl, HCISETAUTH, (unsigned long) &dr) < 0) {
		fprintf(stderr, "Can't set auth on hci%d: %s (%d)\n",
						hdev, strerror(errno), errno);
		exit(1);
	}
}

static void cmd_encrypt(int ctl, int hdev, char *opt)
{
	struct hci_dev_req dr;

	dr.dev_id = hdev;
	if (!strcmp(opt, "encrypt"))
		dr.dev_opt = ENCRYPT_P2P;
	else
		dr.dev_opt = ENCRYPT_DISABLED;

	if (ioctl(ctl, HCISETENCRYPT, (unsigned long) &dr) < 0) {
		fprintf(stderr, "Can't set encrypt on hci%d: %s (%d)\n",
						hdev, strerror(errno), errno);
		exit(1);
	}
}

static void cmd_up(int ctl, int hdev, char *opt)
{
	/* Start HCI device */
	if (ioctl(ctl, HCIDEVUP, hdev) < 0) {
		if (errno == EALREADY)
			return;
		fprintf(stderr, "Can't init device hci%d: %s (%d)\n",
						hdev, strerror(errno), errno);
		exit(1);
	}
}

static void cmd_down(int ctl, int hdev, char *opt)
{
	/* Stop HCI device */
	if (ioctl(ctl, HCIDEVDOWN, hdev) < 0) {
		fprintf(stderr, "Can't down device hci%d: %s (%d)\n",
						hdev, strerror(errno), errno);
		exit(1);
	}
}

static void cmd_reset(int ctl, int hdev, char *opt)
{
	/* Reset HCI device */
#if 0
	if (ioctl(ctl, HCIDEVRESET, hdev) < 0 ){
		fprintf(stderr, "Reset failed for device hci%d: %s (%d)\n",
						hdev, strerror(errno), errno);
		exit(1);
	}
#endif
	cmd_down(ctl, hdev, "down");
	cmd_up(ctl, hdev, "up");
}

static void cmd_ptype(int ctl, int hdev, char *opt)
{
	struct hci_dev_req dr;

	dr.dev_id = hdev;

	if (hci_strtoptype(opt, &dr.dev_opt)) {
		if (ioctl(ctl, HCISETPTYPE, (unsigned long) &dr) < 0) {
			fprintf(stderr, "Can't set pkttype on hci%d: %s (%d)\n",
						hdev, strerror(errno), errno);
			exit(1);
		}
	} else {
		print_dev_hdr(&di);
		print_pkt_type(&di);
	}
}

static void cmd_lp(int ctl, int hdev, char *opt)
{
	struct hci_dev_req dr;

	dr.dev_id = hdev;

	if (hci_strtolp(opt, &dr.dev_opt)) {
		if (ioctl(ctl, HCISETLINKPOL, (unsigned long) &dr) < 0) {
			fprintf(stderr, "Can't set link policy on hci%d: %s (%d)\n",
						hdev, strerror(errno), errno);
			exit(1);
		}
	} else {
		print_dev_hdr(&di);
		print_link_policy(&di);
	}
}

static void cmd_lm(int ctl, int hdev, char *opt)
{
	struct hci_dev_req dr;

	dr.dev_id = hdev;

	if (hci_strtolm(opt, &dr.dev_opt)) {
		if (ioctl(ctl, HCISETLINKMODE, (unsigned long) &dr) < 0) {
			fprintf(stderr, "Can't set default link mode on hci%d: %s (%d)\n",
						hdev, strerror(errno), errno);
			exit(1);
		}
	} else {
		print_dev_hdr(&di);
		print_link_mode(&di);
	}
}

static void cmd_aclmtu(int ctl, int hdev, char *opt)
{
	struct hci_dev_req dr = { .dev_id = hdev };
	uint16_t mtu, mpkt;

	if (!opt)
		return;

	if (sscanf(opt, "%4hu:%4hu", &mtu, &mpkt) != 2)
		return;

	dr.dev_opt = htobl(htobs(mpkt) | (htobs(mtu) << 16));

	if (ioctl(ctl, HCISETACLMTU, (unsigned long) &dr) < 0) {
		fprintf(stderr, "Can't set ACL mtu on hci%d: %s(%d)\n",
						hdev, strerror(errno), errno);
		exit(1);
	}
}

static void cmd_scomtu(int ctl, int hdev, char *opt)
{
	struct hci_dev_req dr = { .dev_id = hdev };
	uint16_t mtu, mpkt;

	if (!opt)
		return;

	if (sscanf(opt, "%4hu:%4hu", &mtu, &mpkt) != 2)
		return;

	dr.dev_opt = htobl(htobs(mpkt) | (htobs(mtu) << 16));

	if (ioctl(ctl, HCISETSCOMTU, (unsigned long) &dr) < 0) {
		fprintf(stderr, "Can't set SCO mtu on hci%d: %s (%d)\n",
						hdev, strerror(errno), errno);
		exit(1);
	}
}

static void cmd_features(int ctl, int hdev, char *opt)
{
	uint8_t features[8], max_page = 0;
	char *tmp;
	int i, dd;

	if (!(di.features[7] & LMP_EXT_FEAT)) {
		print_dev_hdr(&di);
		print_dev_features(&di, 1);
		return;
	}

	dd = hci_open_dev(hdev);
	if (dd < 0) {
		fprintf(stderr, "Can't open device hci%d: %s (%d)\n",
						hdev, strerror(errno), errno);
		exit(1);
	}

	if (hci_read_local_ext_features(dd, 0, &max_page, features, 1000) < 0) {
		fprintf(stderr, "Can't read extended features hci%d: %s (%d)\n",
						hdev, strerror(errno), errno);
		exit(1);
	}

	if (max_page < 1 && (features[6] & LMP_SIMPLE_PAIR))
		max_page = 1;

	print_dev_hdr(&di);
	printf("\tFeatures%s: 0x%2.2x 0x%2.2x 0x%2.2x 0x%2.2x "
				"0x%2.2x 0x%2.2x 0x%2.2x 0x%2.2x\n",
		(max_page > 0) ? " page 0" : "",
		features[0], features[1], features[2], features[3],
		features[4], features[5], features[6], features[7]);

	tmp = lmp_featurestostr(di.features, "\t\t", 63);
	printf("%s\n", tmp);
	bt_free(tmp);

	for (i = 1; i <= max_page; i++) {
		if (hci_read_local_ext_features(dd, i, NULL,
							features, 1000) < 0)
			continue;

		printf("\tFeatures page %d: 0x%2.2x 0x%2.2x 0x%2.2x 0x%2.2x "
					"0x%2.2x 0x%2.2x 0x%2.2x 0x%2.2x\n", i,
			features[0], features[1], features[2], features[3],
			features[4], features[5], features[6], features[7]);
	}

	hci_close_dev(dd);
}

static void cmd_name(int ctl, int hdev, char *opt)
{
	int dd;

	dd = hci_open_dev(hdev);
	if (dd < 0) {
		fprintf(stderr, "Can't open device hci%d: %s (%d)\n",
						hdev, strerror(errno), errno);
		exit(1);
	}

	if (opt) {
		if (hci_write_local_name(dd, opt, 2000) < 0) {
			fprintf(stderr, "Can't change local name on hci%d: %s (%d)\n",
						hdev, strerror(errno), errno);
			exit(1);
		}
	} else {
		char name[249];
		int i;

		if (hci_read_local_name(dd, sizeof(name), name, 1000) < 0) {
			fprintf(stderr, "Can't read local name on hci%d: %s (%d)\n",
						hdev, strerror(errno), errno);
			exit(1);
		}

		for (i = 0; i < 248 && name[i]; i++) {
			if ((unsigned char) name[i] < 32 || name[i] == 127)
				name[i] = '.';
		}

		name[248] = '\0';

		print_dev_hdr(&di);
		printf("\tName: '%s'\n", name);
	}

	hci_close_dev(dd);
}

/*
 * see http://www.bluetooth.org/assigned-numbers/baseband.htm --- all
 * strings are reproduced verbatim
 */
static char *get_minor_device_name(int major, int minor)
{
	switch (major) {
	case 0:	/* misc */
		return "";
	case 1:	/* computer */
		switch (minor) {
		case 0:
			return "Uncategorized";
		case 1:
			return "Desktop workstation";
		case 2:
			return "Server";
		case 3:
			return "Laptop";
		case 4:
			return "Handheld";
		case 5:
			return "Palm";
		case 6:
			return "Wearable";
		}
		break;
	case 2:	/* phone */
		switch (minor) {
		case 0:
			return "Uncategorized";
		case 1:
			return "Cellular";
		case 2:
			return "Cordless";
		case 3:
			return "Smart phone";
		case 4:
			return "Wired modem or voice gateway";
		case 5:
			return "Common ISDN Access";
		case 6:
			return "Sim Card Reader";
		}
		break;
	case 3:	/* lan access */
		if (minor == 0)
			return "Uncategorized";
		switch (minor / 8) {
		case 0:
			return "Fully available";
		case 1:
			return "1-17% utilized";
		case 2:
			return "17-33% utilized";
		case 3:
			return "33-50% utilized";
		case 4:
			return "50-67% utilized";
		case 5:
			return "67-83% utilized";
		case 6:
			return "83-99% utilized";
		case 7:
			return "No service available";
		}
		break;
	case 4:	/* audio/video */
		switch (minor) {
		case 0:
			return "Uncategorized";
		case 1:
			return "Device conforms to the Headset profile";
		case 2:
			return "Hands-free";
			/* 3 is reserved */
		case 4:
			return "Microphone";
		case 5:
			return "Loudspeaker";
		case 6:
			return "Headphones";
		case 7:
			return "Portable Audio";
		case 8:
			return "Car Audio";
		case 9:
			return "Set-top box";
		case 10:
			return "HiFi Audio Device";
		case 11:
			return "VCR";
		case 12:
			return "Video Camera";
		case 13:
			return "Camcorder";
		case 14:
			return "Video Monitor";
		case 15:
			return "Video Display and Loudspeaker";
		case 16:
			return "Video Conferencing";
			/* 17 is reserved */
		case 18:
			return "Gaming/Toy";
		}
		break;
	case 5:	/* peripheral */ {
		static char cls_str[48];

		cls_str[0] = '\0';

		switch (minor & 48) {
		case 16:
			strncpy(cls_str, "Keyboard", sizeof(cls_str));
			break;
		case 32:
			strncpy(cls_str, "Pointing device", sizeof(cls_str));
			break;
		case 48:
			strncpy(cls_str, "Combo keyboard/pointing device", sizeof(cls_str));
			break;
		}
		if ((minor & 15) && (strlen(cls_str) > 0))
			strcat(cls_str, "/");

		switch (minor & 15) {
		case 0:
			break;
		case 1:
			strncat(cls_str, "Joystick",
					sizeof(cls_str) - strlen(cls_str) - 1);
			break;
		case 2:
			strncat(cls_str, "Gamepad",
					sizeof(cls_str) - strlen(cls_str) - 1);
			break;
		case 3:
			strncat(cls_str, "Remote control",
					sizeof(cls_str) - strlen(cls_str) - 1);
			break;
		case 4:
			strncat(cls_str, "Sensing device",
					sizeof(cls_str) - strlen(cls_str) - 1);
			break;
		case 5:
			strncat(cls_str, "Digitizer tablet",
					sizeof(cls_str) - strlen(cls_str) - 1);
			break;
		case 6:
			strncat(cls_str, "Card reader",
					sizeof(cls_str) - strlen(cls_str) - 1);
			break;
		default:
			strncat(cls_str, "(reserved)",
					sizeof(cls_str) - strlen(cls_str) - 1);
			break;
		}
		if (strlen(cls_str) > 0)
			return cls_str;
		break;
	}
	case 6:	/* imaging */
		if (minor & 4)
			return "Display";
		if (minor & 8)
			return "Camera";
		if (minor & 16)
			return "Scanner";
		if (minor & 32)
			return "Printer";
		break;
	case 7: /* wearable */
		switch (minor) {
		case 1:
			return "Wrist Watch";
		case 2:
			return "Pager";
		case 3:
			return "Jacket";
		case 4:
			return "Helmet";
		case 5:
			return "Glasses";
		}
		break;
	case 8: /* toy */
		switch (minor) {
		case 1:
			return "Robot";
		case 2:
			return "Vehicle";
		case 3:
			return "Doll / Action Figure";
		case 4:
			return "Controller";
		case 5:
			return "Game";
		}
		break;
	case 63:	/* uncategorised */
		return "";
	}
	return "Unknown (reserved) minor device class";
}

static void cmd_class(int ctl, int hdev, char *opt)
{
	static const char *services[] = { "Positioning",
					"Networking",
					"Rendering",
					"Capturing",
					"Object Transfer",
					"Audio",
					"Telephony",
					"Information" };
	static const char *major_devices[] = { "Miscellaneous",
					"Computer",
					"Phone",
					"LAN Access",
					"Audio/Video",
					"Peripheral",
					"Imaging",
					"Uncategorized" };
	int s = hci_open_dev(hdev);

	if (s < 0) {
		fprintf(stderr, "Can't open device hci%d: %s (%d)\n",
						hdev, strerror(errno), errno);
		exit(1);
	}
	if (opt) {
		uint32_t cod = strtoul(opt, NULL, 16);
		if (hci_write_class_of_dev(s, cod, 2000) < 0) {
			fprintf(stderr, "Can't write local class of device on hci%d: %s (%d)\n",
						hdev, strerror(errno), errno);
			exit(1);
		}
	} else {
		uint8_t cls[3];
		if (hci_read_class_of_dev(s, cls, 1000) < 0) {
			fprintf(stderr, "Can't read class of device on hci%d: %s (%d)\n",
						hdev, strerror(errno), errno);
			exit(1);
		}
		print_dev_hdr(&di);
		printf("\tClass: 0x%02x%02x%02x\n", cls[2], cls[1], cls[0]);
		printf("\tService Classes: ");
		if (cls[2]) {
			unsigned int i;
			int first = 1;
			for (i = 0; i < (sizeof(services) / sizeof(*services)); i++)
				if (cls[2] & (1 << i)) {
					if (!first)
						printf(", ");
					printf("%s", services[i]);
					first = 0;
				}
		} else
			printf("Unspecified");
		printf("\n\tDevice Class: ");
		if ((cls[1] & 0x1f) >= sizeof(major_devices) / sizeof(*major_devices))
			printf("Invalid Device Class!\n");
		else
			printf("%s, %s\n", major_devices[cls[1] & 0x1f],
				get_minor_device_name(cls[1] & 0x1f, cls[0] >> 2));
	}

	hci_close_dev(s);
}

static void cmd_voice(int ctl, int hdev, char *opt)
{
	static char *icf[] = {	"Linear",
				"u-Law",
				"A-Law",
				"Reserved" };

	static char *idf[] = {	"1's complement",
				"2's complement",
				"Sign-Magnitude",
				"Reserved" };

	static char *iss[] = {	"8 bit",
				"16 bit" };

	static char *acf[] = {	"CVSD",
				"u-Law",
				"A-Law",
				"Reserved" };

	int s = hci_open_dev(hdev);

	if (s < 0) {
		fprintf(stderr, "Can't open device hci%d: %s (%d)\n",
						hdev, strerror(errno), errno);
		exit(1);
	}
	if (opt) {
		uint16_t vs = htobs(strtoul(opt, NULL, 16));
		if (hci_write_voice_setting(s, vs, 2000) < 0) {
			fprintf(stderr, "Can't write voice setting on hci%d: %s (%d)\n",
						hdev, strerror(errno), errno);
			exit(1);
		}
	} else {
		uint16_t vs;
		uint8_t ic;
		if (hci_read_voice_setting(s, &vs, 1000) < 0) {
			fprintf(stderr, "Can't read voice setting on hci%d: %s (%d)\n",
						hdev, strerror(errno), errno);
			exit(1);
		}
		vs = htobs(vs);
		ic = (vs & 0x0300) >> 8;
		print_dev_hdr(&di);
		printf("\tVoice setting: 0x%04x%s\n", vs,
			((vs & 0x03fc) == 0x0060) ? " (Default Condition)" : "");
		printf("\tInput Coding: %s\n", icf[ic]);
		printf("\tInput Data Format: %s\n", idf[(vs & 0xc0) >> 6]);

		if (!ic) {
			printf("\tInput Sample Size: %s\n",
				iss[(vs & 0x20) >> 5]);
			printf("\t# of bits padding at MSB: %d\n",
				(vs & 0x1c) >> 2);
		}
		printf("\tAir Coding Format: %s\n", acf[vs & 0x03]);
	}

	hci_close_dev(s);
}

static void cmd_delkey(int ctl, int hdev, char *opt)
{
	bdaddr_t bdaddr;
	uint8_t all;
	int dd;

	if (!opt)
		return;

	dd = hci_open_dev(hdev);
	if (dd < 0) {
		fprintf(stderr, "Can't open device hci%d: %s (%d)\n",
						hdev, strerror(errno), errno);
		exit(1);
	}

	if (!strcasecmp(opt, "all")) {
		bacpy(&bdaddr, BDADDR_ANY);
		all = 1;
	} else {
		str2ba(opt, &bdaddr);
		all = 0;
	}

	if (hci_delete_stored_link_key(dd, &bdaddr, all, 1000) < 0) {
		fprintf(stderr, "Can't delete stored link key on hci%d: %s (%d)\n",
						hdev, strerror(errno), errno);
		exit(1);
	}

	hci_close_dev(dd);
}

static void cmd_oob_data(int ctl, int hdev, char *opt)
{
	uint8_t hash[16], randomizer[16];
	int i, dd;

	dd = hci_open_dev(hdev);
	if (dd < 0) {
		fprintf(stderr, "Can't open device hci%d: %s (%d)\n",
						hdev, strerror(errno), errno);
		exit(1);
	}

	if (hci_read_local_oob_data(dd, hash, randomizer, 1000) < 0) {
		fprintf(stderr, "Can't read local OOB data on hci%d: %s (%d)\n",
						hdev, strerror(errno), errno);
		exit(1);
	}

	print_dev_hdr(&di);
	printf("\tOOB Hash:  ");
	for (i = 0; i < 16; i++)
		printf(" %02x", hash[i]);
	printf("\n\tRandomizer:");
	for (i = 0; i < 16; i++)
		printf(" %02x", randomizer[i]);
	printf("\n");

	hci_close_dev(dd);
}

static void cmd_commands(int ctl, int hdev, char *opt)
{
	uint8_t cmds[64];
	char *str;
	int i, n, dd;

	dd = hci_open_dev(hdev);
	if (dd < 0) {
		fprintf(stderr, "Can't open device hci%d: %s (%d)\n",
						hdev, strerror(errno), errno);
		exit(1);
	}

	if (hci_read_local_commands(dd, cmds, 1000) < 0) {
		fprintf(stderr, "Can't read support commands on hci%d: %s (%d)\n",
						hdev, strerror(errno), errno);
		exit(1);
	}

	print_dev_hdr(&di);
	for (i = 0; i < 64; i++) {
		if (!cmds[i])
			continue;

		printf("%s Octet %-2d = 0x%02x (Bit",
			i ? "\t\t ": "\tCommands:", i, cmds[i]);
		for (n = 0; n < 8; n++)
			if (cmds[i] & (1 << n))
				printf(" %d", n);
		printf(")\n");
	}

	str = hci_commandstostr(cmds, "\t", 71);
	printf("%s\n", str);
	bt_free(str);

	hci_close_dev(dd);
}

static void cmd_version(int ctl, int hdev, char *opt)
{
	struct hci_version ver;
	char *hciver, *lmpver;
	int dd;

	dd = hci_open_dev(hdev);
	if (dd < 0) {
		fprintf(stderr, "Can't open device hci%d: %s (%d)\n",
						hdev, strerror(errno), errno);
		exit(1);
	}

	if (hci_read_local_version(dd, &ver, 1000) < 0) {
		fprintf(stderr, "Can't read version info hci%d: %s (%d)\n",
						hdev, strerror(errno), errno);
		exit(1);
	}

	hciver = hci_vertostr(ver.hci_ver);
	if (((di.type & 0x30) >> 4) == HCI_PRIMARY)
		lmpver = lmp_vertostr(ver.lmp_ver);
	else
		lmpver = pal_vertostr(ver.lmp_ver);

	print_dev_hdr(&di);
	printf("\tHCI Version: %s (0x%x)  Revision: 0x%x\n"
		"\t%s Version: %s (0x%x)  Subversion: 0x%x\n"
		"\tManufacturer: %s (%d)\n",
		hciver ? hciver : "n/a", ver.hci_ver, ver.hci_rev,
		(((di.type & 0x30) >> 4) == HCI_PRIMARY) ? "LMP" : "PAL",
		lmpver ? lmpver : "n/a", ver.lmp_ver, ver.lmp_subver,
		bt_compidtostr(ver.manufacturer), ver.manufacturer);

	if (hciver)
		bt_free(hciver);
	if (lmpver)
		bt_free(lmpver);

	hci_close_dev(dd);
}

static void cmd_inq_tpl(int ctl, int hdev, char *opt)
{
	int dd;

	dd = hci_open_dev(hdev);
	if (dd < 0) {
		fprintf(stderr, "Can't open device hci%d: %s (%d)\n",
						hdev, strerror(errno), errno);
		exit(1);
	}

	if (opt) {
		int8_t level = atoi(opt);

		if (hci_write_inquiry_transmit_power_level(dd, level, 2000) < 0) {
			fprintf(stderr, "Can't set inquiry transmit power level on hci%d: %s (%d)\n",
						hdev, strerror(errno), errno);
			exit(1);
		}
	} else {
		int8_t level;

		if (hci_read_inq_response_tx_power_level(dd, &level, 1000) < 0) {
			fprintf(stderr, "Can't read inquiry transmit power level on hci%d: %s (%d)\n",
						hdev, strerror(errno), errno);
			exit(1);
		}

		print_dev_hdr(&di);
		printf("\tInquiry transmit power level: %d\n", level);
	}

	hci_close_dev(dd);
}

static void cmd_inq_mode(int ctl, int hdev, char *opt)
{
	int dd;

	dd = hci_open_dev(hdev);
	if (dd < 0) {
		fprintf(stderr, "Can't open device hci%d: %s (%d)\n",
						hdev, strerror(errno), errno);
		exit(1);
	}

	if (opt) {
		uint8_t mode = atoi(opt);

		if (hci_write_inquiry_mode(dd, mode, 2000) < 0) {
			fprintf(stderr, "Can't set inquiry mode on hci%d: %s (%d)\n",
						hdev, strerror(errno), errno);
			exit(1);
		}
	} else {
		uint8_t mode;

		if (hci_read_inquiry_mode(dd, &mode, 1000) < 0) {
			fprintf(stderr, "Can't read inquiry mode on hci%d: %s (%d)\n",
						hdev, strerror(errno), errno);
			exit(1);
		}

		print_dev_hdr(&di);
		printf("\tInquiry mode: ");
		switch (mode) {
		case 0:
			printf("Standard Inquiry\n");
			break;
		case 1:
			printf("Inquiry with RSSI\n");
			break;
		case 2:
			printf("Inquiry with RSSI or Extended Inquiry\n");
			break;
		default:
			printf("Unknown (0x%02x)\n", mode);
			break;
		}
	}

	hci_close_dev(dd);
}

static void cmd_inq_data(int ctl, int hdev, char *opt)
{
	int i, dd;

	dd = hci_open_dev(hdev);
	if (dd < 0) {
		fprintf(stderr, "Can't open device hci%d: %s (%d)\n",
						hdev, strerror(errno), errno);
		exit(1);
	}

	if (opt) {
		uint8_t fec = 0, data[HCI_MAX_EIR_LENGTH];
		char tmp[3];
		int i, size;

		memset(data, 0, sizeof(data));

		memset(tmp, 0, sizeof(tmp));
		size = (strlen(opt) + 1) / 2;
		if (size > HCI_MAX_EIR_LENGTH)
			size = HCI_MAX_EIR_LENGTH;

		for (i = 0; i < size; i++) {
			memcpy(tmp, opt + (i * 2), 2);
			data[i] = strtol(tmp, NULL, 16);
		}

		if (hci_write_ext_inquiry_response(dd, fec, data, 2000) < 0) {
			fprintf(stderr, "Can't set extended inquiry response on hci%d: %s (%d)\n",
						hdev, strerror(errno), errno);
			exit(1);
		}
	} else {
		uint8_t fec, data[HCI_MAX_EIR_LENGTH], len, type, *ptr;
		char *str;

		if (hci_read_ext_inquiry_response(dd, &fec, data, 1000) < 0) {
			fprintf(stderr, "Can't read extended inquiry response on hci%d: %s (%d)\n",
						hdev, strerror(errno), errno);
			exit(1);
		}

		print_dev_hdr(&di);
		printf("\tFEC %s\n\t\t", fec ? "enabled" : "disabled");
		for (i = 0; i < HCI_MAX_EIR_LENGTH; i++)
			printf("%02x%s%s", data[i], (i + 1) % 8 ? "" : " ",
				(i + 1) % 16 ? " " : (i < 239 ? "\n\t\t" : "\n"));

		ptr = data;
		while (*ptr) {
			len = *ptr++;
			type = *ptr++;
			switch (type) {
			case 0x01:
				printf("\tFlags:");
				for (i = 0; i < len - 1; i++)
					printf(" 0x%2.2x", *((uint8_t *) (ptr + i)));
				printf("\n");
				break;
			case 0x02:
			case 0x03:
				printf("\t%s service classes:",
					type == 0x02 ? "Shortened" : "Complete");
				for (i = 0; i < (len - 1) / 2; i++) {
					uint16_t val = get_le16((ptr + (i * 2)));
					printf(" 0x%4.4x", val);
				}
				printf("\n");
				break;
			case 0x08:
			case 0x09:
				str = malloc(len);
				if (str) {
					snprintf(str, len, "%s", ptr);
					for (i = 0; i < len - 1; i++) {
						if ((unsigned char) str[i] < 32 || str[i] == 127)
							str[i] = '.';
					}
					printf("\t%s local name: \'%s\'\n",
						type == 0x08 ? "Shortened" : "Complete", str);
					free(str);
				}
				break;
			case 0x0a:
				printf("\tTX power level: %d\n", *((int8_t *) ptr));
				break;
			case 0x10:
				printf("\tDevice ID with %d bytes data\n",
								len - 1);
				break;
			default:
				printf("\tUnknown type 0x%02x with %d bytes data\n",
								type, len - 1);
				break;
			}

			ptr += (len - 1);
		}

		printf("\n");
	}

	hci_close_dev(dd);
}

static void cmd_inq_type(int ctl, int hdev, char *opt)
{
	int dd;

	dd = hci_open_dev(hdev);
	if (dd < 0) {
		fprintf(stderr, "Can't open device hci%d: %s (%d)\n",
						hdev, strerror(errno), errno);
		exit(1);
	}

	if (opt) {
		uint8_t type = atoi(opt);

		if (hci_write_inquiry_scan_type(dd, type, 2000) < 0) {
			fprintf(stderr, "Can't set inquiry scan type on hci%d: %s (%d)\n",
						hdev, strerror(errno), errno);
			exit(1);
		}
	} else {
		uint8_t type;

		if (hci_read_inquiry_scan_type(dd, &type, 1000) < 0) {
			fprintf(stderr, "Can't read inquiry scan type on hci%d: %s (%d)\n",
						hdev, strerror(errno), errno);
			exit(1);
		}

		print_dev_hdr(&di);
		printf("\tInquiry scan type: %s\n",
			type == 1 ? "Interlaced Inquiry Scan" : "Standard Inquiry Scan");
	}

	hci_close_dev(dd);
}

static void cmd_inq_parms(int ctl, int hdev, char *opt)
{
	struct hci_request rq;
	int s;

	if ((s = hci_open_dev(hdev)) < 0) {
		fprintf(stderr, "Can't open device hci%d: %s (%d)\n",
						hdev, strerror(errno), errno);
		exit(1);
	}

	memset(&rq, 0, sizeof(rq));

	if (opt) {
		unsigned int window, interval;
		write_inq_activity_cp cp;

		if (sscanf(opt,"%4u:%4u", &window, &interval) != 2) {
			printf("Invalid argument format\n");
			exit(1);
		}

		rq.ogf = OGF_HOST_CTL;
		rq.ocf = OCF_WRITE_INQ_ACTIVITY;
		rq.cparam = &cp;
		rq.clen = WRITE_INQ_ACTIVITY_CP_SIZE;

		cp.window = htobs((uint16_t) window);
		cp.interval = htobs((uint16_t) interval);

		if (window < 0x12 || window > 0x1000)
			printf("Warning: inquiry window out of range!\n");

		if (interval < 0x12 || interval > 0x1000)
			printf("Warning: inquiry interval out of range!\n");

		if (hci_send_req(s, &rq, 2000) < 0) {
			fprintf(stderr, "Can't set inquiry parameters name on hci%d: %s (%d)\n",
						hdev, strerror(errno), errno);
			exit(1);
		}
	} else {
		uint16_t window, interval;
		read_inq_activity_rp rp;

		rq.ogf = OGF_HOST_CTL;
		rq.ocf = OCF_READ_INQ_ACTIVITY;
		rq.rparam = &rp;
		rq.rlen = READ_INQ_ACTIVITY_RP_SIZE;

		if (hci_send_req(s, &rq, 1000) < 0) {
			fprintf(stderr, "Can't read inquiry parameters on hci%d: %s (%d)\n",
						hdev, strerror(errno), errno);
			exit(1);
		}
		if (rp.status) {
			printf("Read inquiry parameters on hci%d returned status %d\n",
							hdev, rp.status);
			exit(1);
		}
		print_dev_hdr(&di);

		window   = btohs(rp.window);
		interval = btohs(rp.interval);
		printf("\tInquiry interval: %u slots (%.2f ms), window: %u slots (%.2f ms)\n",
				interval, (float)interval * 0.625, window, (float)window * 0.625);
	}

	hci_close_dev(s);
}

static void cmd_page_parms(int ctl, int hdev, char *opt)
{
	struct hci_request rq;
	int s;

	if ((s = hci_open_dev(hdev)) < 0) {
		fprintf(stderr, "Can't open device hci%d: %s (%d)\n",
						hdev, strerror(errno), errno);
		exit(1);
	}

	memset(&rq, 0, sizeof(rq));

	if (opt) {
		unsigned int window, interval;
		write_page_activity_cp cp;

		if (sscanf(opt,"%4u:%4u", &window, &interval) != 2) {
			printf("Invalid argument format\n");
			exit(1);
		}

		rq.ogf = OGF_HOST_CTL;
		rq.ocf = OCF_WRITE_PAGE_ACTIVITY;
		rq.cparam = &cp;
		rq.clen = WRITE_PAGE_ACTIVITY_CP_SIZE;

		cp.window = htobs((uint16_t) window);
		cp.interval = htobs((uint16_t) interval);

		if (window < 0x12 || window > 0x1000)
			printf("Warning: page window out of range!\n");

		if (interval < 0x12 || interval > 0x1000)
			printf("Warning: page interval out of range!\n");

		if (hci_send_req(s, &rq, 2000) < 0) {
			fprintf(stderr, "Can't set page parameters name on hci%d: %s (%d)\n",
						hdev, strerror(errno), errno);
			exit(1);
		}
	} else {
		uint16_t window, interval;
		read_page_activity_rp rp;

		rq.ogf = OGF_HOST_CTL;
		rq.ocf = OCF_READ_PAGE_ACTIVITY;
		rq.rparam = &rp;
		rq.rlen = READ_PAGE_ACTIVITY_RP_SIZE;

		if (hci_send_req(s, &rq, 1000) < 0) {
			fprintf(stderr, "Can't read page parameters on hci%d: %s (%d)\n",
						hdev, strerror(errno), errno);
			exit(1);
		}
		if (rp.status) {
			printf("Read page parameters on hci%d returned status %d\n",
							hdev, rp.status);
			exit(1);
		}
		print_dev_hdr(&di);

		window   = btohs(rp.window);
		interval = btohs(rp.interval);
		printf("\tPage interval: %u slots (%.2f ms), "
			"window: %u slots (%.2f ms)\n",
			interval, (float)interval * 0.625,
			window, (float)window * 0.625);
	}

	hci_close_dev(s);
}

static void cmd_page_to(int ctl, int hdev, char *opt)
{
	struct hci_request rq;
	int s;

	if ((s = hci_open_dev(hdev)) < 0) {
		fprintf(stderr, "Can't open device hci%d: %s (%d)\n",
						hdev, strerror(errno), errno);
		exit(1);
	}

	memset(&rq, 0, sizeof(rq));

	if (opt) {
		unsigned int timeout;
		write_page_timeout_cp cp;

		if (sscanf(opt,"%5u", &timeout) != 1) {
			printf("Invalid argument format\n");
			exit(1);
		}

		rq.ogf = OGF_HOST_CTL;
		rq.ocf = OCF_WRITE_PAGE_TIMEOUT;
		rq.cparam = &cp;
		rq.clen = WRITE_PAGE_TIMEOUT_CP_SIZE;

		cp.timeout = htobs((uint16_t) timeout);

		if (timeout < 0x01 || timeout > 0xFFFF)
			printf("Warning: page timeout out of range!\n");

		if (hci_send_req(s, &rq, 2000) < 0) {
			fprintf(stderr, "Can't set page timeout on hci%d: %s (%d)\n",
						hdev, strerror(errno), errno);
			exit(1);
		}
	} else {
		uint16_t timeout;
		read_page_timeout_rp rp;

		rq.ogf = OGF_HOST_CTL;
		rq.ocf = OCF_READ_PAGE_TIMEOUT;
		rq.rparam = &rp;
		rq.rlen = READ_PAGE_TIMEOUT_RP_SIZE;

		if (hci_send_req(s, &rq, 1000) < 0) {
			fprintf(stderr, "Can't read page timeout on hci%d: %s (%d)\n",
						hdev, strerror(errno), errno);
			exit(1);
		}
		if (rp.status) {
			printf("Read page timeout on hci%d returned status %d\n",
							hdev, rp.status);
			exit(1);
		}
		print_dev_hdr(&di);

		timeout = btohs(rp.timeout);
		printf("\tPage timeout: %u slots (%.2f ms)\n",
				timeout, (float)timeout * 0.625);
	}

	hci_close_dev(s);
}

static void cmd_afh_mode(int ctl, int hdev, char *opt)
{
	int dd;

	dd = hci_open_dev(hdev);
	if (dd < 0) {
		fprintf(stderr, "Can't open device hci%d: %s (%d)\n",
						hdev, strerror(errno), errno);
		exit(1);
	}

	if (opt) {
		uint8_t mode = atoi(opt);

		if (hci_write_afh_mode(dd, mode, 2000) < 0) {
			fprintf(stderr, "Can't set AFH mode on hci%d: %s (%d)\n",
					hdev, strerror(errno), errno);
			exit(1);
		}
	} else {
		uint8_t mode;

		if (hci_read_afh_mode(dd, &mode, 1000) < 0) {
			fprintf(stderr, "Can't read AFH mode on hci%d: %s (%d)\n",
					hdev, strerror(errno), errno);
			exit(1);
		}

		print_dev_hdr(&di);
		printf("\tAFH mode: %s\n", mode == 1 ? "Enabled" : "Disabled");
	}

	hci_close_dev(dd);
}

static void cmd_ssp_mode(int ctl, int hdev, char *opt)
{
	int dd;

	dd = hci_open_dev(hdev);
	if (dd < 0) {
		fprintf(stderr, "Can't open device hci%d: %s (%d)\n",
						hdev, strerror(errno), errno);
		exit(1);
	}

	if (opt) {
		uint8_t mode = atoi(opt);

		if (hci_write_simple_pairing_mode(dd, mode, 2000) < 0) {
			fprintf(stderr, "Can't set Simple Pairing mode on hci%d: %s (%d)\n",
					hdev, strerror(errno), errno);
			exit(1);
		}
	} else {
		uint8_t mode;

		if (hci_read_simple_pairing_mode(dd, &mode, 1000) < 0) {
			fprintf(stderr, "Can't read Simple Pairing mode on hci%d: %s (%d)\n",
					hdev, strerror(errno), errno);
			exit(1);
		}

		print_dev_hdr(&di);
		printf("\tSimple Pairing mode: %s\n",
			mode == 1 ? "Enabled" : "Disabled");
	}

	hci_close_dev(dd);
}

static void print_rev_ericsson(int dd)
{
	struct hci_request rq;
	unsigned char buf[102];

	memset(&rq, 0, sizeof(rq));
	rq.ogf    = OGF_VENDOR_CMD;
	rq.ocf    = 0x000f;
	rq.cparam = NULL;
	rq.clen   = 0;
	rq.rparam = &buf;
	rq.rlen   = sizeof(buf);

	if (hci_send_req(dd, &rq, 1000) < 0) {
		printf("\nCan't read revision info: %s (%d)\n",
			strerror(errno), errno);
		return;
	}

	printf("\t%s\n", buf + 1);
}

static void print_rev_csr(int dd, uint16_t rev)
{
	uint16_t buildid, chipver, chiprev, maxkeylen, mapsco;

	if (csr_read_varid_uint16(dd, 0, CSR_VARID_BUILDID, &buildid) < 0) {
		printf("\t%s\n", csr_buildidtostr(rev));
		return;
	}

	printf("\t%s\n", csr_buildidtostr(buildid));

	if (!csr_read_varid_uint16(dd, 1, CSR_VARID_CHIPVER, &chipver)) {
		if (csr_read_varid_uint16(dd, 2, CSR_VARID_CHIPREV, &chiprev) < 0)
			chiprev = 0;
		printf("\tChip version: %s\n", csr_chipvertostr(chipver, chiprev));
	}

	if (!csr_read_varid_uint16(dd, 3, CSR_VARID_MAX_CRYPT_KEY_LENGTH, &maxkeylen))
		printf("\tMax key size: %d bit\n", maxkeylen * 8);

	if (!csr_read_pskey_uint16(dd, 4, CSR_PSKEY_HOSTIO_MAP_SCO_PCM, 0x0000, &mapsco))
		printf("\tSCO mapping:  %s\n", mapsco ? "PCM" : "HCI");
}

static void print_rev_digianswer(int dd)
{
	struct hci_request rq;
	unsigned char req[] = { 0x07 };
	unsigned char buf[102];

	memset(&rq, 0, sizeof(rq));
	rq.ogf    = OGF_VENDOR_CMD;
	rq.ocf    = 0x000e;
	rq.cparam = req;
	rq.clen   = sizeof(req);
	rq.rparam = &buf;
	rq.rlen   = sizeof(buf);

	if (hci_send_req(dd, &rq, 1000) < 0) {
		printf("\nCan't read revision info: %s (%d)\n",
			strerror(errno), errno);
		return;
	}

	printf("\t%s\n", buf + 1);
}

static void print_rev_broadcom(uint16_t hci_rev, uint16_t lmp_subver)
{
	printf("\tFirmware %d.%d / %d\n",
		hci_rev & 0xff, lmp_subver >> 8, lmp_subver & 0xff);
}

static void print_rev_avm(uint16_t hci_rev, uint16_t lmp_subver)
{
	if (lmp_subver == 0x01)
		printf("\tFirmware 03.%d.%d\n", hci_rev >> 8, hci_rev & 0xff);
	else
		printf("\tUnknown type\n");
}

static void cmd_revision(int ctl, int hdev, char *opt)
{
	struct hci_version ver;
	int dd;

	dd = hci_open_dev(hdev);
	if (dd < 0) {
		fprintf(stderr, "Can't open device hci%d: %s (%d)\n",
						hdev, strerror(errno), errno);
		return;
	}

	if (hci_read_local_version(dd, &ver, 1000) < 0) {
		fprintf(stderr, "Can't read version info for hci%d: %s (%d)\n",
						hdev, strerror(errno), errno);
		return;
	}

	print_dev_hdr(&di);
	switch (ver.manufacturer) {
	case 0:
	case 37:
	case 48:
		print_rev_ericsson(dd);
		break;
	case 10:
		print_rev_csr(dd, ver.hci_rev);
		break;
	case 12:
		print_rev_digianswer(dd);
		break;
	case 15:
		print_rev_broadcom(ver.hci_rev, ver.lmp_subver);
		break;
	case 31:
		print_rev_avm(ver.hci_rev, ver.lmp_subver);
		break;
	default:
		printf("\tUnsupported manufacturer\n");
		break;
	}

	hci_close_dev(dd);

	return;
}

static void cmd_block(int ctl, int hdev, char *opt)
{
	bdaddr_t bdaddr;
	int dd;

	if (!opt)
		return;

	dd = hci_open_dev(hdev);
	if (dd < 0) {
		fprintf(stderr, "Can't open device hci%d: %s (%d)\n",
						hdev, strerror(errno), errno);
		exit(1);
	}

	str2ba(opt, &bdaddr);

	if (ioctl(dd, HCIBLOCKADDR, &bdaddr) < 0) {
		perror("ioctl(HCIBLOCKADDR)");
		exit(1);
	}

	hci_close_dev(dd);
}

static void cmd_unblock(int ctl, int hdev, char *opt)
{
	bdaddr_t bdaddr;
	int dd;

	if (!opt)
		return;

	dd = hci_open_dev(hdev);
	if (dd < 0) {
		fprintf(stderr, "Can't open device hci%d: %s (%d)\n",
						hdev, strerror(errno), errno);
		exit(1);
	}

	if (!strcasecmp(opt, "all"))
		bacpy(&bdaddr, BDADDR_ANY);
	else
		str2ba(opt, &bdaddr);

	if (ioctl(dd, HCIUNBLOCKADDR, &bdaddr) < 0) {
		perror("ioctl(HCIUNBLOCKADDR)");
		exit(1);
	}

	hci_close_dev(dd);
}

static void print_dev_hdr(struct hci_dev_info *di)
{
	static int hdr = -1;
	char addr[18];

	if (hdr == di->dev_id)
		return;
	hdr = di->dev_id;

	ba2str(&di->bdaddr, addr);

	printf("%s:\tType: %s  Bus: %s\n", di->name,
					hci_typetostr((di->type & 0x30) >> 4),
					hci_bustostr(di->type & 0x0f));
	printf("\tBD Address: %s  ACL MTU: %d:%d  SCO MTU: %d:%d\n",
					addr, di->acl_mtu, di->acl_pkts,
						di->sco_mtu, di->sco_pkts);
}

static void print_dev_info(int ctl, struct hci_dev_info *di)
{
	struct hci_dev_stats *st = &di->stat;
	char *str;

	print_dev_hdr(di);

	str = hci_dflagstostr(di->flags);
	printf("\t%s\n", str);
	bt_free(str);

	printf("\tRX bytes:%d acl:%d sco:%d events:%d errors:%d\n",
		st->byte_rx, st->acl_rx, st->sco_rx, st->evt_rx, st->err_rx);

	printf("\tTX bytes:%d acl:%d sco:%d commands:%d errors:%d\n",
		st->byte_tx, st->acl_tx, st->sco_tx, st->cmd_tx, st->err_tx);

	if (all && !hci_test_bit(HCI_RAW, &di->flags)) {
		print_dev_features(di, 0);

		if (((di->type & 0x30) >> 4) == HCI_PRIMARY) {
			print_pkt_type(di);
			print_link_policy(di);
			print_link_mode(di);

			if (hci_test_bit(HCI_UP, &di->flags)) {
				cmd_name(ctl, di->dev_id, NULL);
				cmd_class(ctl, di->dev_id, NULL);
			}
		}

		if (hci_test_bit(HCI_UP, &di->flags))
			cmd_version(ctl, di->dev_id, NULL);
	}

	printf("\n");
}

static struct {
	char *cmd;
	void (*func)(int ctl, int hdev, char *opt);
	char *opt;
	char *doc;
} command[] = {
	{ "up",		cmd_up,		0,		"Open and initialize HCI device" },
	{ "down",	cmd_down,	0,		"Close HCI device" },
	{ "reset",	cmd_reset,	0,		"Reset HCI device" },
	{ "rstat",	cmd_rstat,	0,		"Reset statistic counters" },
	{ "auth",	cmd_auth,	0,		"Enable Authentication" },
	{ "noauth",	cmd_auth,	0,		"Disable Authentication" },
	{ "encrypt",	cmd_encrypt,	0,		"Enable Encryption" },
	{ "noencrypt",	cmd_encrypt,	0,		"Disable Encryption" },
	{ "piscan",	cmd_scan,	0,		"Enable Page and Inquiry scan" },
	{ "noscan",	cmd_scan,	0,		"Disable scan" },
	{ "iscan",	cmd_scan,	0,		"Enable Inquiry scan" },
	{ "pscan",	cmd_scan,	0,		"Enable Page scan" },
	{ "ptype",	cmd_ptype,	"[type]",	"Get/Set default packet type" },
	{ "lm",		cmd_lm,		"[mode]",	"Get/Set default link mode"   },
	{ "lp",		cmd_lp,		"[policy]",	"Get/Set default link policy" },
	{ "name",	cmd_name,	"[name]",	"Get/Set local name" },
	{ "class",	cmd_class,	"[class]",	"Get/Set class of device" },
	{ "voice",	cmd_voice,	"[voice]",	"Get/Set voice setting" },
	{ "iac",	cmd_iac,	"[iac]",	"Get/Set inquiry access code" },
	{ "inqtpl",	cmd_inq_tpl,	"[level]",	"Get/Set inquiry transmit power level" },
	{ "inqmode",	cmd_inq_mode,	"[mode]",	"Get/Set inquiry mode" },
	{ "inqdata",	cmd_inq_data,	"[data]",	"Get/Set inquiry data" },
	{ "inqtype",	cmd_inq_type,	"[type]",	"Get/Set inquiry scan type" },
	{ "inqparms",	cmd_inq_parms,	"[win:int]",	"Get/Set inquiry scan window and interval" },
	{ "pageparms",	cmd_page_parms,	"[win:int]",	"Get/Set page scan window and interval" },
	{ "pageto",	cmd_page_to,	"[to]",		"Get/Set page timeout" },
	{ "afhmode",	cmd_afh_mode,	"[mode]",	"Get/Set AFH mode" },
	{ "sspmode",	cmd_ssp_mode,	"[mode]",	"Get/Set Simple Pairing Mode" },
	{ "aclmtu",	cmd_aclmtu,	"<mtu:pkt>",	"Set ACL MTU and number of packets" },
	{ "scomtu",	cmd_scomtu,	"<mtu:pkt>",	"Set SCO MTU and number of packets" },
	{ "delkey",	cmd_delkey,	"<bdaddr>",	"Delete link key from the device" },
	{ "oobdata",	cmd_oob_data,	0,		"Get local OOB data" },
	{ "commands",	cmd_commands,	0,		"Display supported commands" },
	{ "features",	cmd_features,	0,		"Display device features" },
	{ "version",	cmd_version,	0,		"Display version information" },
	{ "revision",	cmd_revision,	0,		"Display revision information" },
	{ "block",	cmd_block,	"<bdaddr>",	"Add a device to the blacklist" },
	{ "unblock",	cmd_unblock,	"<bdaddr>",	"Remove a device from the blacklist" },
	{ "lerandaddr", cmd_le_addr,	"<bdaddr>",	"Set LE Random Address" },
	{ "leadv",	cmd_le_adv,	"[type]",	"Enable LE advertising"
		"\n\t\t\t0 - Connectable undirected advertising (default)"
		"\n\t\t\t3 - Non connectable undirected advertising"},
	{ "noleadv",	cmd_no_le_adv,	0,		"Disable LE advertising" },
	{ "lestates",	cmd_le_states,	0,		"Display the supported LE states" },
	{ NULL, NULL, 0 }
};

static void usage(void)
{
	int i;

	printf("hciconfig - HCI device configuration utility\n");
	printf("Usage:\n"
		"\thciconfig\n"
		"\thciconfig [-a] hciX [command ...]\n");
	printf("Commands:\n");
	for (i = 0; command[i].cmd; i++)
		printf("\t%-10s %-8s\t%s\n", command[i].cmd,
		command[i].opt ? command[i].opt : " ",
		command[i].doc);
}

static struct option main_options[] = {
	{ "help",	0, 0, 'h' },
	{ "all",	0, 0, 'a' },
	{ 0, 0, 0, 0 }
};

int main(int argc, char *argv[])
{
	int opt, ctl, i, cmd = 0;

	while ((opt = getopt_long(argc, argv, "ah", main_options, NULL)) != -1) {
		switch (opt) {
		case 'a':
			all = 1;
			break;

		case 'h':
		default:
			usage();
			exit(0);
		}
	}

	argc -= optind;
	argv += optind;
	optind = 0;

	/* Open HCI socket  */
	if ((ctl = socket(AF_BLUETOOTH, SOCK_RAW, BTPROTO_HCI)) < 0) {
		perror("Can't open HCI socket.");
		exit(1);
	}

	if (argc < 1) {
		print_dev_list(ctl, 0);
		exit(0);
	}

	di.dev_id = atoi(argv[0] + 3);
	argc--; argv++;

	if (ioctl(ctl, HCIGETDEVINFO, (void *) &di)) {
		perror("Can't get device info");
		exit(1);
	}

	while (argc > 0) {
		for (i = 0; command[i].cmd; i++) {
			if (strncmp(command[i].cmd,
					*argv, strlen(command[i].cmd)))
				continue;

			if (command[i].opt) {
				argc--; argv++;
			}

			command[i].func(ctl, di.dev_id, *argv);
			cmd = 1;
			break;
		}

		if (command[i].cmd == 0)
			fprintf(stderr, "Warning: unknown command - \"%s\"\n",
					*argv);

		argc--; argv++;
	}

	if (!cmd)
		print_dev_info(ctl, &di);

	close(ctl);
	return 0;
}
