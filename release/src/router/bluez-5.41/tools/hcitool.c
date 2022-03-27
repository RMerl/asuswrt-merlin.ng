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
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <sys/param.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <signal.h>

#include "lib/bluetooth.h"
#include "lib/hci.h"
#include "lib/hci_lib.h"

#include "src/oui.h"

#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

/* Unofficial value, might still change */
#define LE_LINK		0x80

#define FLAGS_AD_TYPE 0x01
#define FLAGS_LIMITED_MODE_BIT 0x01
#define FLAGS_GENERAL_MODE_BIT 0x02

#define EIR_FLAGS                   0x01  /* flags */
#define EIR_UUID16_SOME             0x02  /* 16-bit UUID, more available */
#define EIR_UUID16_ALL              0x03  /* 16-bit UUID, all listed */
#define EIR_UUID32_SOME             0x04  /* 32-bit UUID, more available */
#define EIR_UUID32_ALL              0x05  /* 32-bit UUID, all listed */
#define EIR_UUID128_SOME            0x06  /* 128-bit UUID, more available */
#define EIR_UUID128_ALL             0x07  /* 128-bit UUID, all listed */
#define EIR_NAME_SHORT              0x08  /* shortened local name */
#define EIR_NAME_COMPLETE           0x09  /* complete local name */
#define EIR_TX_POWER                0x0A  /* transmit power level */
#define EIR_DEVICE_ID               0x10  /* device ID */

#define for_each_opt(opt, long, short) while ((opt=getopt_long(argc, argv, short ? short:"+", long, NULL)) != -1)

static volatile int signal_received = 0;

static void usage(void);

static int str2buf(const char *str, uint8_t *buf, size_t blen)
{
	int i, dlen;

	if (str == NULL)
		return -EINVAL;

	memset(buf, 0, blen);

	dlen = MIN((strlen(str) / 2), blen);

	for (i = 0; i < dlen; i++)
		sscanf(str + (i * 2), "%02hhX", &buf[i]);

	return 0;
}

static int dev_info(int s, int dev_id, long arg)
{
	struct hci_dev_info di = { .dev_id = dev_id };
	char addr[18];

	if (ioctl(s, HCIGETDEVINFO, (void *) &di))
		return 0;

	ba2str(&di.bdaddr, addr);
	printf("\t%s\t%s\n", di.name, addr);
	return 0;
}

static void helper_arg(int min_num_arg, int max_num_arg, int *argc,
			char ***argv, const char *usage)
{
	*argc -= optind;
	/* too many arguments, but when "max_num_arg < min_num_arg" then no
		 limiting (prefer "max_num_arg=-1" to gen infinity)
	*/
	if ( (*argc > max_num_arg) && (max_num_arg >= min_num_arg ) ) {
		fprintf(stderr, "%s: too many arguments (maximal: %i)\n",
				*argv[0], max_num_arg);
		printf("%s", usage);
		exit(1);
	}

	/* print usage */
	if (*argc < min_num_arg) {
		fprintf(stderr, "%s: too few arguments (minimal: %i)\n",
				*argv[0], min_num_arg);
		printf("%s", usage);
		exit(0);
	}

	*argv += optind;
}

static char *type2str(uint8_t type)
{
	switch (type) {
	case SCO_LINK:
		return "SCO";
	case ACL_LINK:
		return "ACL";
	case ESCO_LINK:
		return "eSCO";
	case LE_LINK:
		return "LE";
	default:
		return "Unknown";
	}
}

static int conn_list(int s, int dev_id, long arg)
{
	struct hci_conn_list_req *cl;
	struct hci_conn_info *ci;
	int id = arg;
	int i;

	if (id != -1 && dev_id != id)
		return 0;

	if (!(cl = malloc(10 * sizeof(*ci) + sizeof(*cl)))) {
		perror("Can't allocate memory");
		exit(1);
	}
	cl->dev_id = dev_id;
	cl->conn_num = 10;
	ci = cl->conn_info;

	if (ioctl(s, HCIGETCONNLIST, (void *) cl)) {
		perror("Can't get connection list");
		exit(1);
	}

	for (i = 0; i < cl->conn_num; i++, ci++) {
		char addr[18];
		char *str;
		ba2str(&ci->bdaddr, addr);
		str = hci_lmtostr(ci->link_mode);
		printf("\t%s %s %s handle %d state %d lm %s\n",
			ci->out ? "<" : ">", type2str(ci->type),
			addr, ci->handle, ci->state, str);
		bt_free(str);
	}

	free(cl);
	return 0;
}

static int find_conn(int s, int dev_id, long arg)
{
	struct hci_conn_list_req *cl;
	struct hci_conn_info *ci;
	int i;

	if (!(cl = malloc(10 * sizeof(*ci) + sizeof(*cl)))) {
		perror("Can't allocate memory");
		exit(1);
	}
	cl->dev_id = dev_id;
	cl->conn_num = 10;
	ci = cl->conn_info;

	if (ioctl(s, HCIGETCONNLIST, (void *) cl)) {
		perror("Can't get connection list");
		exit(1);
	}

	for (i = 0; i < cl->conn_num; i++, ci++)
		if (!bacmp((bdaddr_t *) arg, &ci->bdaddr)) {
			free(cl);
			return 1;
		}

	free(cl);
	return 0;
}

static void hex_dump(char *pref, int width, unsigned char *buf, int len)
{
	register int i,n;

	for (i = 0, n = 1; i < len; i++, n++) {
		if (n == 1)
			printf("%s", pref);
		printf("%2.2X ", buf[i]);
		if (n == width) {
			printf("\n");
			n = 0;
		}
	}
	if (i && n!=1)
		printf("\n");
}

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
		static char cls_str[48]; cls_str[0] = 0;

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

static char *major_classes[] = {
	"Miscellaneous", "Computer", "Phone", "LAN Access",
	"Audio/Video", "Peripheral", "Imaging", "Uncategorized"
};

/* Display local devices */

static struct option dev_options[] = {
	{ "help",	0, 0, 'h' },
	{0, 0, 0, 0 }
};

static const char *dev_help =
	"Usage:\n"
	"\tdev\n";

static void cmd_dev(int dev_id, int argc, char **argv)
{
	int opt;

	for_each_opt(opt, dev_options, NULL) {
		switch (opt) {
		default:
			printf("%s", dev_help);
			return;
		}
	}
	helper_arg(0, 0, &argc, &argv, dev_help);

	printf("Devices:\n");

	hci_for_each_dev(HCI_UP, dev_info, 0);
}

/* Inquiry */

static struct option inq_options[] = {
	{ "help",	0, 0, 'h' },
	{ "length",	1, 0, 'l' },
	{ "numrsp",	1, 0, 'n' },
	{ "iac",	1, 0, 'i' },
	{ "flush",	0, 0, 'f' },
	{ 0, 0, 0, 0 }
};

static const char *inq_help =
	"Usage:\n"
	"\tinq [--length=N] maximum inquiry duration in 1.28 s units\n"
	"\t    [--numrsp=N] specify maximum number of inquiry responses\n"
	"\t    [--iac=lap]  specify the inquiry access code\n"
	"\t    [--flush]    flush the inquiry cache\n";

static void cmd_inq(int dev_id, int argc, char **argv)
{
	inquiry_info *info = NULL;
	uint8_t lap[3] = { 0x33, 0x8b, 0x9e };
	int num_rsp, length, flags;
	char addr[18];
	int i, l, opt;

	length  = 8;	/* ~10 seconds */
	num_rsp = 0;
	flags   = 0;

	for_each_opt(opt, inq_options, NULL) {
		switch (opt) {
		case 'l':
			length = atoi(optarg);
			break;

		case 'n':
			num_rsp = atoi(optarg);
			break;

		case 'i':
			l = strtoul(optarg, 0, 16);
			if (!strcasecmp(optarg, "giac")) {
				l = 0x9e8b33;
			} else if (!strcasecmp(optarg, "liac")) {
				l = 0x9e8b00;
			} if (l < 0x9e8b00 || l > 0x9e8b3f) {
				printf("Invalid access code 0x%x\n", l);
				exit(1);
			}
			lap[0] = (l & 0xff);
			lap[1] = (l >> 8) & 0xff;
			lap[2] = (l >> 16) & 0xff;
			break;

		case 'f':
			flags |= IREQ_CACHE_FLUSH;
			break;

		default:
			printf("%s", inq_help);
			return;
		}
	}
	helper_arg(0, 0, &argc, &argv, inq_help);

	printf("Inquiring ...\n");

	num_rsp = hci_inquiry(dev_id, length, num_rsp, lap, &info, flags);
	if (num_rsp < 0) {
		perror("Inquiry failed.");
		exit(1);
	}

	for (i = 0; i < num_rsp; i++) {
		ba2str(&(info+i)->bdaddr, addr);
		printf("\t%s\tclock offset: 0x%4.4x\tclass: 0x%2.2x%2.2x%2.2x\n",
			addr, btohs((info+i)->clock_offset),
			(info+i)->dev_class[2],
			(info+i)->dev_class[1],
			(info+i)->dev_class[0]);
	}

	bt_free(info);
}

/* Device scanning */

static struct option scan_options[] = {
	{ "help",	0, 0, 'h' },
	{ "length",	1, 0, 'l' },
	{ "numrsp",	1, 0, 'n' },
	{ "iac",	1, 0, 'i' },
	{ "flush",	0, 0, 'f' },
	{ "class",	0, 0, 'C' },
	{ "info",	0, 0, 'I' },
	{ "oui",	0, 0, 'O' },
	{ "all",	0, 0, 'A' },
	{ "ext",	0, 0, 'A' },
	{ 0, 0, 0, 0 }
};

static const char *scan_help =
	"Usage:\n"
	"\tscan [--length=N] [--numrsp=N] [--iac=lap] [--flush] [--class] [--info] [--oui] [--refresh]\n";

static void cmd_scan(int dev_id, int argc, char **argv)
{
	inquiry_info *info = NULL;
	uint8_t lap[3] = { 0x33, 0x8b, 0x9e };
	int num_rsp, length, flags;
	uint8_t cls[3], features[8];
	char addr[18], name[249], *comp;
	struct hci_version version;
	struct hci_dev_info di;
	struct hci_conn_info_req *cr;
	int extcls = 0, extinf = 0, extoui = 0;
	int i, n, l, opt, dd, cc;

	length  = 8;	/* ~10 seconds */
	num_rsp = 0;
	flags   = 0;

	for_each_opt(opt, scan_options, NULL) {
		switch (opt) {
		case 'l':
			length = atoi(optarg);
			break;

		case 'n':
			num_rsp = atoi(optarg);
			break;

		case 'i':
			l = strtoul(optarg, 0, 16);
			if (!strcasecmp(optarg, "giac")) {
				l = 0x9e8b33;
			} else if (!strcasecmp(optarg, "liac")) {
				l = 0x9e8b00;
			} else if (l < 0x9e8b00 || l > 0x9e8b3f) {
				printf("Invalid access code 0x%x\n", l);
				exit(1);
			}
			lap[0] = (l & 0xff);
			lap[1] = (l >> 8) & 0xff;
			lap[2] = (l >> 16) & 0xff;
			break;

		case 'f':
			flags |= IREQ_CACHE_FLUSH;
			break;

		case 'C':
			extcls = 1;
			break;

		case 'I':
			extinf = 1;
			break;

		case 'O':
			extoui = 1;
			break;

		case 'A':
			extcls = 1;
			extinf = 1;
			extoui = 1;
			break;

		default:
			printf("%s", scan_help);
			return;
		}
	}
	helper_arg(0, 0, &argc, &argv, scan_help);

	if (dev_id < 0) {
		dev_id = hci_get_route(NULL);
		if (dev_id < 0) {
			perror("Device is not available");
			exit(1);
		}
	}

	if (hci_devinfo(dev_id, &di) < 0) {
		perror("Can't get device info");
		exit(1);
	}

	printf("Scanning ...\n");
	num_rsp = hci_inquiry(dev_id, length, num_rsp, lap, &info, flags);
	if (num_rsp < 0) {
		perror("Inquiry failed");
		exit(1);
	}

	dd = hci_open_dev(dev_id);
	if (dd < 0) {
		perror("HCI device open failed");
		free(info);
		exit(1);
	}

	if (extcls || extinf || extoui)
		printf("\n");

	for (i = 0; i < num_rsp; i++) {
		uint16_t handle = 0;

		if (!extcls && !extinf && !extoui) {
			ba2str(&(info+i)->bdaddr, addr);

			if (hci_read_remote_name_with_clock_offset(dd,
					&(info+i)->bdaddr,
					(info+i)->pscan_rep_mode,
					(info+i)->clock_offset | 0x8000,
					sizeof(name), name, 100000) < 0)
				strcpy(name, "n/a");

			for (n = 0; n < 248 && name[n]; n++) {
				if ((unsigned char) name[i] < 32 || name[i] == 127)
					name[i] = '.';
			}

			name[248] = '\0';

			printf("\t%s\t%s\n", addr, name);
			continue;
		}

		ba2str(&(info+i)->bdaddr, addr);
		printf("BD Address:\t%s [mode %d, clkoffset 0x%4.4x]\n", addr,
			(info+i)->pscan_rep_mode, btohs((info+i)->clock_offset));

		if (extoui) {
			comp = batocomp(&(info+i)->bdaddr);
			if (comp) {
				char oui[9];
				ba2oui(&(info+i)->bdaddr, oui);
				printf("OUI company:\t%s (%s)\n", comp, oui);
				free(comp);
			}
		}

		cc = 0;

		if (extinf) {
			cr = malloc(sizeof(*cr) + sizeof(struct hci_conn_info));
			if (cr) {
				bacpy(&cr->bdaddr, &(info+i)->bdaddr);
				cr->type = ACL_LINK;
				if (ioctl(dd, HCIGETCONNINFO, (unsigned long) cr) < 0) {
					handle = 0;
					cc = 1;
				} else {
					handle = htobs(cr->conn_info->handle);
					cc = 0;
				}
				free(cr);
			}

			if (cc) {
				if (hci_create_connection(dd, &(info+i)->bdaddr,
						htobs(di.pkt_type & ACL_PTYPE_MASK),
						(info+i)->clock_offset | 0x8000,
						0x01, &handle, 25000) < 0) {
					handle = 0;
					cc = 0;
				}
			}
		}

		if (hci_read_remote_name_with_clock_offset(dd,
					&(info+i)->bdaddr,
					(info+i)->pscan_rep_mode,
					(info+i)->clock_offset | 0x8000,
					sizeof(name), name, 100000) < 0) {
		} else {
			for (n = 0; n < 248 && name[n]; n++) {
				if ((unsigned char) name[i] < 32 || name[i] == 127)
					name[i] = '.';
			}

			name[248] = '\0';
		}

		if (strlen(name) > 0)
			printf("Device name:\t%s\n", name);

		if (extcls) {
			memcpy(cls, (info+i)->dev_class, 3);
			printf("Device class:\t");
			if ((cls[1] & 0x1f) > sizeof(major_classes) / sizeof(char *))
				printf("Invalid");
			else
				printf("%s, %s", major_classes[cls[1] & 0x1f],
					get_minor_device_name(cls[1] & 0x1f, cls[0] >> 2));
			printf(" (0x%2.2x%2.2x%2.2x)\n", cls[2], cls[1], cls[0]);
		}

		if (extinf && handle > 0) {
			if (hci_read_remote_version(dd, handle, &version, 20000) == 0) {
				char *ver = lmp_vertostr(version.lmp_ver);
				printf("Manufacturer:\t%s (%d)\n",
					bt_compidtostr(version.manufacturer),
					version.manufacturer);
				printf("LMP version:\t%s (0x%x) [subver 0x%x]\n",
					ver ? ver : "n/a",
					version.lmp_ver, version.lmp_subver);
				if (ver)
					bt_free(ver);
			}

			if (hci_read_remote_features(dd, handle, features, 20000) == 0) {
				char *tmp = lmp_featurestostr(features, "\t\t", 63);
				printf("LMP features:\t0x%2.2x 0x%2.2x 0x%2.2x 0x%2.2x"
					" 0x%2.2x 0x%2.2x 0x%2.2x 0x%2.2x\n",
					features[0], features[1],
					features[2], features[3],
					features[4], features[5],
					features[6], features[7]);
				printf("%s\n", tmp);
				bt_free(tmp);
			}

			if (cc) {
				usleep(10000);
				hci_disconnect(dd, handle, HCI_OE_USER_ENDED_CONNECTION, 10000);
			}
		}

		printf("\n");
	}

	bt_free(info);

	hci_close_dev(dd);
}

/* Remote name */

static struct option name_options[] = {
	{ "help",	0, 0, 'h' },
	{ 0, 0, 0, 0 }
};

static const char *name_help =
	"Usage:\n"
	"\tname <bdaddr>\n";

static void cmd_name(int dev_id, int argc, char **argv)
{
	bdaddr_t bdaddr;
	char name[248];
	int opt, dd;

	for_each_opt(opt, name_options, NULL) {
		switch (opt) {
		default:
			printf("%s", name_help);
			return;
		}
	}
	helper_arg(1, 1, &argc, &argv, name_help);

	str2ba(argv[0], &bdaddr);

	if (dev_id < 0) {
		dev_id = hci_get_route(&bdaddr);
		if (dev_id < 0) {
			fprintf(stderr, "Device is not available.\n");
			exit(1);
		}
	}

	dd = hci_open_dev(dev_id);
	if (dd < 0) {
		perror("HCI device open failed");
		exit(1);
	}

	if (hci_read_remote_name(dd, &bdaddr, sizeof(name), name, 25000) == 0)
		printf("%s\n", name);

	hci_close_dev(dd);
}

/* Info about remote device */

static struct option info_options[] = {
	{ "help",	0, 0, 'h' },
	{ 0, 0, 0, 0 }
};

static const char *info_help =
	"Usage:\n"
	"\tinfo <bdaddr>\n";

static void cmd_info(int dev_id, int argc, char **argv)
{
	bdaddr_t bdaddr;
	uint16_t handle;
	uint8_t features[8], max_page = 0;
	char name[249], *comp, *tmp;
	struct hci_version version;
	struct hci_dev_info di;
	struct hci_conn_info_req *cr;
	int i, opt, dd, cc = 0;

	for_each_opt(opt, info_options, NULL) {
		switch (opt) {
		default:
			printf("%s", info_help);
			return;
		}
	}
	helper_arg(1, 1, &argc, &argv, info_help);

	str2ba(argv[0], &bdaddr);

	if (dev_id < 0)
		dev_id = hci_for_each_dev(HCI_UP, find_conn, (long) &bdaddr);

	if (dev_id < 0)
		dev_id = hci_get_route(&bdaddr);

	if (dev_id < 0) {
		fprintf(stderr, "Device is not available or not connected.\n");
		exit(1);
	}

	if (hci_devinfo(dev_id, &di) < 0) {
		perror("Can't get device info");
		exit(1);
	}

	printf("Requesting information ...\n");

	dd = hci_open_dev(dev_id);
	if (dd < 0) {
		perror("HCI device open failed");
		exit(1);
	}

	cr = malloc(sizeof(*cr) + sizeof(struct hci_conn_info));
	if (!cr) {
		perror("Can't get connection info");
		close(dd);
		exit(1);
	}

	bacpy(&cr->bdaddr, &bdaddr);
	cr->type = ACL_LINK;
	if (ioctl(dd, HCIGETCONNINFO, (unsigned long) cr) < 0) {
		if (hci_create_connection(dd, &bdaddr,
					htobs(di.pkt_type & ACL_PTYPE_MASK),
					0, 0x01, &handle, 25000) < 0) {
			perror("Can't create connection");
			free(cr);
			close(dd);
			exit(1);
		}
		sleep(1);
		cc = 1;
	} else
		handle = htobs(cr->conn_info->handle);

	free(cr);

	printf("\tBD Address:  %s\n", argv[0]);

	comp = batocomp(&bdaddr);
	if (comp) {
		char oui[9];
		ba2oui(&bdaddr, oui);
		printf("\tOUI Company: %s (%s)\n", comp, oui);
		free(comp);
	}

	if (hci_read_remote_name(dd, &bdaddr, sizeof(name), name, 25000) == 0)
		printf("\tDevice Name: %s\n", name);

	if (hci_read_remote_version(dd, handle, &version, 20000) == 0) {
		char *ver = lmp_vertostr(version.lmp_ver);
		printf("\tLMP Version: %s (0x%x) LMP Subversion: 0x%x\n"
			"\tManufacturer: %s (%d)\n",
			ver ? ver : "n/a",
			version.lmp_ver,
			version.lmp_subver,
			bt_compidtostr(version.manufacturer),
			version.manufacturer);
		if (ver)
			bt_free(ver);
	}

	memset(features, 0, sizeof(features));
	hci_read_remote_features(dd, handle, features, 20000);

	if ((di.features[7] & LMP_EXT_FEAT) && (features[7] & LMP_EXT_FEAT))
		hci_read_remote_ext_features(dd, handle, 0, &max_page,
							features, 20000);

	if (max_page < 1 && (features[6] & LMP_SIMPLE_PAIR))
		max_page = 1;

	printf("\tFeatures%s: 0x%2.2x 0x%2.2x 0x%2.2x 0x%2.2x "
				"0x%2.2x 0x%2.2x 0x%2.2x 0x%2.2x\n",
		(max_page > 0) ? " page 0" : "",
		features[0], features[1], features[2], features[3],
		features[4], features[5], features[6], features[7]);

	tmp = lmp_featurestostr(features, "\t\t", 63);
	printf("%s\n", tmp);
	bt_free(tmp);

	for (i = 1; i <= max_page; i++) {
		if (hci_read_remote_ext_features(dd, handle, i, NULL,
							features, 20000) < 0)
			continue;

		printf("\tFeatures page %d: 0x%2.2x 0x%2.2x 0x%2.2x 0x%2.2x "
					"0x%2.2x 0x%2.2x 0x%2.2x 0x%2.2x\n", i,
			features[0], features[1], features[2], features[3],
			features[4], features[5], features[6], features[7]);
	}

	if (cc) {
		usleep(10000);
		hci_disconnect(dd, handle, HCI_OE_USER_ENDED_CONNECTION, 10000);
	}

	hci_close_dev(dd);
}

/* Start periodic inquiry */

static struct option spinq_options[] = {
	{ "help",	0, 0, 'h' },
	{ 0, 0, 0, 0 }
};

static const char *spinq_help =
	"Usage:\n"
	"\tspinq\n";

static void cmd_spinq(int dev_id, int argc, char **argv)
{
	uint8_t lap[3] = { 0x33, 0x8b, 0x9e };
	struct hci_request rq;
	periodic_inquiry_cp cp;
	int opt, dd;

	for_each_opt(opt, spinq_options, NULL) {
		switch (opt) {
		default:
			printf("%s", spinq_help);
			return;
		}
	}
	helper_arg(0, 0, &argc, &argv, spinq_help);

	if (dev_id < 0)
		dev_id = hci_get_route(NULL);

	dd = hci_open_dev(dev_id);
	if (dd < 0) {
		perror("Device open failed");
		exit(EXIT_FAILURE);
	}

	memset(&cp, 0, sizeof(cp));
	memcpy(cp.lap, lap, 3);
	cp.max_period = htobs(16);
	cp.min_period = htobs(10);
	cp.length     = 8;
	cp.num_rsp    = 0;

	memset(&rq, 0, sizeof(rq));
	rq.ogf    = OGF_LINK_CTL;
	rq.ocf    = OCF_PERIODIC_INQUIRY;
	rq.cparam = &cp;
	rq.clen   = PERIODIC_INQUIRY_CP_SIZE;

	if (hci_send_req(dd, &rq, 100) < 0) {
		perror("Periodic inquiry failed");
		exit(EXIT_FAILURE);
	}

	hci_close_dev(dd);
}

/* Exit periodic inquiry */

static struct option epinq_options[] = {
	{ "help",	0, 0, 'h' },
	{ 0, 0, 0, 0 }
};

static const char *epinq_help =
	"Usage:\n"
	"\tepinq\n";

static void cmd_epinq(int dev_id, int argc, char **argv)
{
	int opt, dd;

	for_each_opt(opt, epinq_options, NULL) {
		switch (opt) {
		default:
			printf("%s", epinq_help);
			return;
		}
	}
	helper_arg(0, 0, &argc, &argv, epinq_help);

	if (dev_id < 0)
		dev_id = hci_get_route(NULL);

	dd = hci_open_dev(dev_id);
	if (dd < 0) {
		perror("Device open failed");
		exit(EXIT_FAILURE);
	}

	if (hci_send_cmd(dd, OGF_LINK_CTL,
				OCF_EXIT_PERIODIC_INQUIRY, 0, NULL) < 0) {
		perror("Exit periodic inquiry failed");
		exit(EXIT_FAILURE);
	}

	hci_close_dev(dd);
}

/* Send arbitrary HCI commands */

static struct option cmd_options[] = {
	{ "help",	0, 0, 'h' },
	{ 0, 0, 0, 0 }
};

static const char *cmd_help =
	"Usage:\n"
	"\tcmd <ogf> <ocf> [parameters]\n"
	"Example:\n"
	"\tcmd 0x03 0x0013 0x41 0x42 0x43 0x44\n";

static void cmd_cmd(int dev_id, int argc, char **argv)
{
	unsigned char buf[HCI_MAX_EVENT_SIZE], *ptr = buf;
	struct hci_filter flt;
	hci_event_hdr *hdr;
	int i, opt, len, dd;
	uint16_t ocf;
	uint8_t ogf;

	for_each_opt(opt, cmd_options, NULL) {
		switch (opt) {
		default:
			printf("%s", cmd_help);
			return;
		}
	}
	helper_arg(2, -1, &argc, &argv, cmd_help);

	if (dev_id < 0)
		dev_id = hci_get_route(NULL);

	errno = 0;
	ogf = strtol(argv[0], NULL, 16);
	ocf = strtol(argv[1], NULL, 16);
	if (errno == ERANGE || (ogf > 0x3f) || (ocf > 0x3ff)) {
		printf("%s", cmd_help);
		return;
	}

	for (i = 2, len = 0; i < argc && len < (int) sizeof(buf); i++, len++)
		*ptr++ = (uint8_t) strtol(argv[i], NULL, 16);

	dd = hci_open_dev(dev_id);
	if (dd < 0) {
		perror("Device open failed");
		exit(EXIT_FAILURE);
	}

	/* Setup filter */
	hci_filter_clear(&flt);
	hci_filter_set_ptype(HCI_EVENT_PKT, &flt);
	hci_filter_all_events(&flt);
	if (setsockopt(dd, SOL_HCI, HCI_FILTER, &flt, sizeof(flt)) < 0) {
		perror("HCI filter setup failed");
		exit(EXIT_FAILURE);
	}

	printf("< HCI Command: ogf 0x%02x, ocf 0x%04x, plen %d\n", ogf, ocf, len);
	hex_dump("  ", 20, buf, len); fflush(stdout);

	if (hci_send_cmd(dd, ogf, ocf, len, buf) < 0) {
		perror("Send failed");
		exit(EXIT_FAILURE);
	}

	len = read(dd, buf, sizeof(buf));
	if (len < 0) {
		perror("Read failed");
		exit(EXIT_FAILURE);
	}

	hdr = (void *)(buf + 1);
	ptr = buf + (1 + HCI_EVENT_HDR_SIZE);
	len -= (1 + HCI_EVENT_HDR_SIZE);

	printf("> HCI Event: 0x%02x plen %d\n", hdr->evt, hdr->plen);
	hex_dump("  ", 20, ptr, len); fflush(stdout);

	hci_close_dev(dd);
}

/* Display active connections */

static struct option con_options[] = {
	{ "help",	0, 0, 'h' },
	{ 0, 0, 0, 0 }
};

static const char *con_help =
	"Usage:\n"
	"\tcon\n";

static void cmd_con(int dev_id, int argc, char **argv)
{
	int opt;

	for_each_opt(opt, con_options, NULL) {
		switch (opt) {
		default:
			printf("%s", con_help);
			return;
		}
	}
	helper_arg(0, 0, &argc, &argv, con_help);

	printf("Connections:\n");

	hci_for_each_dev(HCI_UP, conn_list, dev_id);
}

/* Create connection */

static struct option cc_options[] = {
	{ "help",	0, 0, 'h' },
	{ "role",	1, 0, 'r' },
	{ "ptype",	1, 0, 'p' },
	{ 0, 0, 0, 0 }
};

static const char *cc_help =
	"Usage:\n"
	"\tcc [--role=m|s] [--ptype=pkt_types] <bdaddr>\n"
	"Example:\n"
	"\tcc --ptype=dm1,dh3,dh5 01:02:03:04:05:06\n"
	"\tcc --role=m 01:02:03:04:05:06\n";

static void cmd_cc(int dev_id, int argc, char **argv)
{
	bdaddr_t bdaddr;
	uint16_t handle;
	uint8_t role;
	unsigned int ptype;
	int dd, opt;

	role = 0x01;
	ptype = HCI_DM1 | HCI_DM3 | HCI_DM5 | HCI_DH1 | HCI_DH3 | HCI_DH5;

	for_each_opt(opt, cc_options, NULL) {
		switch (opt) {
		case 'p':
			hci_strtoptype(optarg, &ptype);
			break;

		case 'r':
			role = optarg[0] == 'm' ? 0 : 1;
			break;

		default:
			printf("%s", cc_help);
			return;
		}
	}
	helper_arg(1, 1, &argc, &argv, cc_help);

	str2ba(argv[0], &bdaddr);

	if (dev_id < 0) {
		dev_id = hci_get_route(&bdaddr);
		if (dev_id < 0) {
			fprintf(stderr, "Device is not available.\n");
			exit(1);
		}
	}

	dd = hci_open_dev(dev_id);
	if (dd < 0) {
		perror("HCI device open failed");
		exit(1);
	}

	if (hci_create_connection(dd, &bdaddr, htobs(ptype),
				htobs(0x0000), role, &handle, 25000) < 0)
		perror("Can't create connection");

	hci_close_dev(dd);
}

/* Close connection */

static struct option dc_options[] = {
	{ "help",	0, 0, 'h' },
	{ 0, 0, 0, 0 }
};

static const char *dc_help =
	"Usage:\n"
	"\tdc <bdaddr> [reason]\n";

static void cmd_dc(int dev_id, int argc, char **argv)
{
	struct hci_conn_info_req *cr;
	bdaddr_t bdaddr;
	uint8_t reason;
	int opt, dd;

	for_each_opt(opt, dc_options, NULL) {
		switch (opt) {
		default:
			printf("%s", dc_help);
			return;
		}
	}
	helper_arg(1, 2, &argc, &argv, dc_help);

	str2ba(argv[0], &bdaddr);
	reason = (argc > 1) ? atoi(argv[1]) : HCI_OE_USER_ENDED_CONNECTION;

	if (dev_id < 0) {
		dev_id = hci_for_each_dev(HCI_UP, find_conn, (long) &bdaddr);
		if (dev_id < 0) {
			fprintf(stderr, "Not connected.\n");
			exit(1);
		}
	}

	dd = hci_open_dev(dev_id);
	if (dd < 0) {
		perror("HCI device open failed");
		exit(1);
	}

	cr = malloc(sizeof(*cr) + sizeof(struct hci_conn_info));
	if (!cr) {
		perror("Can't allocate memory");
		exit(1);
	}

	bacpy(&cr->bdaddr, &bdaddr);
	cr->type = ACL_LINK;
	if (ioctl(dd, HCIGETCONNINFO, (unsigned long) cr) < 0) {
		perror("Get connection info failed");
		exit(1);
	}

	if (hci_disconnect(dd, htobs(cr->conn_info->handle),
						reason, 10000) < 0)
		perror("Disconnect failed");

	free(cr);

	hci_close_dev(dd);
}

/* Role switch */

static struct option sr_options[] = {
	{ "help",	0, 0, 'h' },
	{ 0, 0, 0, 0 }
};

static const char *sr_help =
	"Usage:\n"
	"\tsr <bdaddr> <role>\n";

static void cmd_sr(int dev_id, int argc, char **argv)
{
	bdaddr_t bdaddr;
	uint8_t role;
	int opt, dd;

	for_each_opt(opt, sr_options, NULL) {
		switch (opt) {
		default:
			printf("%s", sr_help);
			return;
		}
	}
	helper_arg(2, 2, &argc, &argv, sr_help);

	str2ba(argv[0], &bdaddr);
	switch (argv[1][0]) {
	case 'm':
		role = 0;
		break;
	case 's':
		role = 1;
		break;
	default:
		role = atoi(argv[1]);
		break;
	}

	if (dev_id < 0) {
		dev_id = hci_for_each_dev(HCI_UP, find_conn, (long) &bdaddr);
		if (dev_id < 0) {
			fprintf(stderr, "Not connected.\n");
			exit(1);
		}
	}

	dd = hci_open_dev(dev_id);
	if (dd < 0) {
		perror("HCI device open failed");
		exit(1);
	}

	if (hci_switch_role(dd, &bdaddr, role, 10000) < 0) {
		perror("Switch role request failed");
		exit(1);
	}

	hci_close_dev(dd);
}

/* Read RSSI */

static struct option rssi_options[] = {
	{ "help",	0, 0, 'h' },
	{ 0, 0, 0, 0 }
};

static const char *rssi_help =
	"Usage:\n"
	"\trssi <bdaddr>\n";

static void cmd_rssi(int dev_id, int argc, char **argv)
{
	struct hci_conn_info_req *cr;
	bdaddr_t bdaddr;
	int8_t rssi;
	int opt, dd;

	for_each_opt(opt, rssi_options, NULL) {
		switch (opt) {
		default:
			printf("%s", rssi_help);
			return;
		}
	}
	helper_arg(1, 1, &argc, &argv, rssi_help);

	str2ba(argv[0], &bdaddr);

	if (dev_id < 0) {
		dev_id = hci_for_each_dev(HCI_UP, find_conn, (long) &bdaddr);
		if (dev_id < 0) {
			fprintf(stderr, "Not connected.\n");
			exit(1);
		}
	}

	dd = hci_open_dev(dev_id);
	if (dd < 0) {
		perror("HCI device open failed");
		exit(1);
	}

	cr = malloc(sizeof(*cr) + sizeof(struct hci_conn_info));
	if (!cr) {
		perror("Can't allocate memory");
		exit(1);
	}

	bacpy(&cr->bdaddr, &bdaddr);
	cr->type = ACL_LINK;
	if (ioctl(dd, HCIGETCONNINFO, (unsigned long) cr) < 0) {
		perror("Get connection info failed");
		exit(1);
	}

	if (hci_read_rssi(dd, htobs(cr->conn_info->handle), &rssi, 1000) < 0) {
		perror("Read RSSI failed");
		exit(1);
	}

	printf("RSSI return value: %d\n", rssi);

	free(cr);

	hci_close_dev(dd);
}

/* Get link quality */

static struct option lq_options[] = {
	{ "help",	0, 0, 'h' },
	{ 0, 0, 0, 0 }
};

static const char *lq_help =
	"Usage:\n"
	"\tlq <bdaddr>\n";

static void cmd_lq(int dev_id, int argc, char **argv)
{
	struct hci_conn_info_req *cr;
	bdaddr_t bdaddr;
	uint8_t lq;
	int opt, dd;

	for_each_opt(opt, lq_options, NULL) {
		switch (opt) {
		default:
			printf("%s", lq_help);
			return;
		}
	}
	helper_arg(1, 1, &argc, &argv, lq_help);

	str2ba(argv[0], &bdaddr);

	if (dev_id < 0) {
		dev_id = hci_for_each_dev(HCI_UP, find_conn, (long) &bdaddr);
		if (dev_id < 0) {
			fprintf(stderr, "Not connected.\n");
			exit(1);
		}
	}

	dd = hci_open_dev(dev_id);
	if (dd < 0) {
		perror("HCI device open failed");
		exit(1);
	}

	cr = malloc(sizeof(*cr) + sizeof(struct hci_conn_info));
	if (!cr) {
		perror("Can't allocate memory");
		exit(1);
	}

	bacpy(&cr->bdaddr, &bdaddr);
	cr->type = ACL_LINK;
	if (ioctl(dd, HCIGETCONNINFO, (unsigned long) cr) < 0) {
		perror("Get connection info failed");
		exit(1);
	}

	if (hci_read_link_quality(dd, htobs(cr->conn_info->handle), &lq, 1000) < 0) {
		perror("HCI read_link_quality request failed");
		exit(1);
	}

	printf("Link quality: %d\n", lq);

	free(cr);

	hci_close_dev(dd);
}

/* Get transmit power level */

static struct option tpl_options[] = {
	{ "help",	0, 0, 'h' },
	{ 0, 0, 0, 0 }
};

static const char *tpl_help =
	"Usage:\n"
	"\ttpl <bdaddr> [type]\n";

static void cmd_tpl(int dev_id, int argc, char **argv)
{
	struct hci_conn_info_req *cr;
	bdaddr_t bdaddr;
	uint8_t type;
	int8_t level;
	int opt, dd;

	for_each_opt(opt, tpl_options, NULL) {
		switch (opt) {
		default:
			printf("%s", tpl_help);
			return;
		}
	}
	helper_arg(1, 2, &argc, &argv, tpl_help);

	str2ba(argv[0], &bdaddr);
	type = (argc > 1) ? atoi(argv[1]) : 0;

	if (dev_id < 0) {
		dev_id = hci_for_each_dev(HCI_UP, find_conn, (long) &bdaddr);
		if (dev_id < 0) {
			fprintf(stderr, "Not connected.\n");
			exit(1);
		}
	}

	dd = hci_open_dev(dev_id);
	if (dd < 0) {
		perror("HCI device open failed");
		exit(1);
	}

	cr = malloc(sizeof(*cr) + sizeof(struct hci_conn_info));
	if (!cr) {
		perror("Can't allocate memory");
		exit(1);
	}

	bacpy(&cr->bdaddr, &bdaddr);
	cr->type = ACL_LINK;
	if (ioctl(dd, HCIGETCONNINFO, (unsigned long) cr) < 0) {
		perror("Get connection info failed");
		exit(1);
	}

	if (hci_read_transmit_power_level(dd, htobs(cr->conn_info->handle), type, &level, 1000) < 0) {
		perror("HCI read transmit power level request failed");
		exit(1);
	}

	printf("%s transmit power level: %d\n",
		(type == 0) ? "Current" : "Maximum", level);

	free(cr);

	hci_close_dev(dd);
}

/* Get AFH channel map */

static struct option afh_options[] = {
	{ "help",	0, 0, 'h' },
	{ 0, 0, 0, 0 }
};

static const char *afh_help =
	"Usage:\n"
	"\tafh <bdaddr>\n";

static void cmd_afh(int dev_id, int argc, char **argv)
{
	struct hci_conn_info_req *cr;
	bdaddr_t bdaddr;
	uint16_t handle;
	uint8_t mode, map[10];
	int opt, dd;

	for_each_opt(opt, afh_options, NULL) {
		switch (opt) {
		default:
			printf("%s", afh_help);
			return;
		}
	}
	helper_arg(1, 1, &argc, &argv, afh_help);

	str2ba(argv[0], &bdaddr);

	if (dev_id < 0) {
		dev_id = hci_for_each_dev(HCI_UP, find_conn, (long) &bdaddr);
		if (dev_id < 0) {
			fprintf(stderr, "Not connected.\n");
			exit(1);
		}
	}

	dd = hci_open_dev(dev_id);
	if (dd < 0) {
		perror("HCI device open failed");
		exit(1);
	}

	cr = malloc(sizeof(*cr) + sizeof(struct hci_conn_info));
	if (!cr) {
		perror("Can't allocate memory");
		exit(1);
	}

	bacpy(&cr->bdaddr, &bdaddr);
	cr->type = ACL_LINK;
	if (ioctl(dd, HCIGETCONNINFO, (unsigned long) cr) < 0) {
		perror("Get connection info failed");
		exit(1);
	}

	handle = htobs(cr->conn_info->handle);

	if (hci_read_afh_map(dd, handle, &mode, map, 1000) < 0) {
		perror("HCI read AFH map request failed");
		exit(1);
	}

	if (mode == 0x01) {
		int i;
		printf("AFH map: 0x");
		for (i = 0; i < 10; i++)
			printf("%02x", map[i]);
		printf("\n");
	} else
		printf("AFH disabled\n");

	free(cr);

	hci_close_dev(dd);
}

/* Set connection packet type */

static struct option cpt_options[] = {
	{ "help",	0, 0, 'h' },
	{ 0, 0, 0, 0 }
};

static const char *cpt_help =
	"Usage:\n"
	"\tcpt <bdaddr> <packet_types>\n";

static void cmd_cpt(int dev_id, int argc, char **argv)
{
	struct hci_conn_info_req *cr;
	struct hci_request rq;
	set_conn_ptype_cp cp;
	evt_conn_ptype_changed rp;
	bdaddr_t bdaddr;
	unsigned int ptype;
	int dd, opt;

	for_each_opt(opt, cpt_options, NULL) {
		switch (opt) {
		default:
			printf("%s", cpt_help);
			return;
		}
	}
	helper_arg(2, 2, &argc, &argv, cpt_help);

	str2ba(argv[0], &bdaddr);
	hci_strtoptype(argv[1], &ptype);

	if (dev_id < 0) {
		dev_id = hci_for_each_dev(HCI_UP, find_conn, (long) &bdaddr);
		if (dev_id < 0) {
			fprintf(stderr, "Not connected.\n");
			exit(1);
		}
	}

	dd = hci_open_dev(dev_id);
	if (dd < 0) {
		perror("HCI device open failed");
		exit(1);
	}

	cr = malloc(sizeof(*cr) + sizeof(struct hci_conn_info));
	if (!cr) {
		perror("Can't allocate memory");
		exit(1);
	}

	bacpy(&cr->bdaddr, &bdaddr);
	cr->type = ACL_LINK;
	if (ioctl(dd, HCIGETCONNINFO, (unsigned long) cr) < 0) {
		perror("Get connection info failed");
		exit(1);
	}

	cp.handle   = htobs(cr->conn_info->handle);
	cp.pkt_type = ptype;

	memset(&rq, 0, sizeof(rq));
	rq.ogf    = OGF_LINK_CTL;
	rq.ocf    = OCF_SET_CONN_PTYPE;
	rq.cparam = &cp;
	rq.clen   = SET_CONN_PTYPE_CP_SIZE;
	rq.rparam = &rp;
	rq.rlen   = EVT_CONN_PTYPE_CHANGED_SIZE;
	rq.event  = EVT_CONN_PTYPE_CHANGED;

	if (hci_send_req(dd, &rq, 100) < 0) {
		perror("Packet type change failed");
		exit(1);
	}

	free(cr);

	hci_close_dev(dd);
}

/* Get/Set link policy settings */

static struct option lp_options[] = {
	{ "help",	0, 0, 'h' },
	{ 0, 0, 0, 0 }
};

static const char *lp_help =
	"Usage:\n"
	"\tlp <bdaddr> [link policy]\n";

static void cmd_lp(int dev_id, int argc, char **argv)
{
	struct hci_conn_info_req *cr;
	bdaddr_t bdaddr;
	uint16_t policy;
	int opt, dd;

	for_each_opt(opt, lp_options, NULL) {
		switch (opt) {
		default:
			printf("%s", lp_help);
			return;
		}
	}
	helper_arg(1, 2, &argc, &argv, lp_help);

	str2ba(argv[0], &bdaddr);

	if (dev_id < 0) {
		dev_id = hci_for_each_dev(HCI_UP, find_conn, (long) &bdaddr);
		if (dev_id < 0) {
			fprintf(stderr, "Not connected.\n");
			exit(1);
		}
	}

	dd = hci_open_dev(dev_id);
	if (dd < 0) {
		perror("HCI device open failed");
		exit(1);
	}

	cr = malloc(sizeof(*cr) + sizeof(struct hci_conn_info));
	if (!cr) {
		perror("Can't allocate memory");
		exit(1);
	}

	bacpy(&cr->bdaddr, &bdaddr);
	cr->type = ACL_LINK;
	if (ioctl(dd, HCIGETCONNINFO, (unsigned long) cr) < 0) {
		perror("Get connection info failed");
		exit(1);
	}

	if (argc == 1) {
		char *str;
		if (hci_read_link_policy(dd, htobs(cr->conn_info->handle),
							&policy, 1000) < 0) {
			perror("HCI read_link_policy_settings request failed");
			exit(1);
		}

		policy = btohs(policy);
		str = hci_lptostr(policy);
		if (str) {
			printf("Link policy settings: %s\n", str);
			bt_free(str);
		} else {
			fprintf(stderr, "Invalig settings\n");
			exit(1);
		}
	} else {
		unsigned int val;
		if (hci_strtolp(argv[1], &val) < 0) {
			fprintf(stderr, "Invalig arguments\n");
			exit(1);
		}
		policy = val;

		if (hci_write_link_policy(dd, htobs(cr->conn_info->handle),
						htobs(policy), 1000) < 0) {
			perror("HCI write_link_policy_settings request failed");
			exit(1);
		}
	}

	free(cr);

	hci_close_dev(dd);
}

/* Get/Set link supervision timeout */

static struct option lst_options[] = {
	{ "help",	0, 0, 'h' },
	{ 0, 0, 0, 0 }
};

static const char *lst_help =
	"Usage:\n"
	"\tlst <bdaddr> [new value in slots]\n";

static void cmd_lst(int dev_id, int argc, char **argv)
{
	struct hci_conn_info_req *cr;
	bdaddr_t bdaddr;
	uint16_t timeout;
	int opt, dd;

	for_each_opt(opt, lst_options, NULL) {
		switch (opt) {
		default:
			printf("%s", lst_help);
			return;
		}
	}
	helper_arg(1, 2, &argc, &argv, lst_help);

	str2ba(argv[0], &bdaddr);

	if (dev_id < 0) {
		dev_id = hci_for_each_dev(HCI_UP, find_conn, (long) &bdaddr);
		if (dev_id < 0) {
			fprintf(stderr, "Not connected.\n");
			exit(1);
		}
	}

	dd = hci_open_dev(dev_id);
	if (dd < 0) {
		perror("HCI device open failed");
		exit(1);
	}

	cr = malloc(sizeof(*cr) + sizeof(struct hci_conn_info));
	if (!cr) {
		perror("Can't allocate memory");
		exit(1);
	}

	bacpy(&cr->bdaddr, &bdaddr);
	cr->type = ACL_LINK;
	if (ioctl(dd, HCIGETCONNINFO, (unsigned long) cr) < 0) {
		perror("Get connection info failed");
		exit(1);
	}

	if (argc == 1) {
		if (hci_read_link_supervision_timeout(dd, htobs(cr->conn_info->handle),
							&timeout, 1000) < 0) {
			perror("HCI read_link_supervision_timeout request failed");
			exit(1);
		}

		timeout = btohs(timeout);

		if (timeout)
			printf("Link supervision timeout: %u slots (%.2f msec)\n",
				timeout, (float) timeout * 0.625);
		else
			printf("Link supervision timeout never expires\n");
	} else {
		timeout = strtol(argv[1], NULL, 10);

		if (hci_write_link_supervision_timeout(dd, htobs(cr->conn_info->handle),
							htobs(timeout), 1000) < 0) {
			perror("HCI write_link_supervision_timeout request failed");
			exit(1);
		}
	}

	free(cr);

	hci_close_dev(dd);
}

/* Request authentication */

static struct option auth_options[] = {
	{ "help",	0, 0, 'h' },
	{ 0, 0, 0, 0 }
};

static const char *auth_help =
	"Usage:\n"
	"\tauth <bdaddr>\n";

static void cmd_auth(int dev_id, int argc, char **argv)
{
	struct hci_conn_info_req *cr;
	bdaddr_t bdaddr;
	int opt, dd;

	for_each_opt(opt, auth_options, NULL) {
		switch (opt) {
		default:
			printf("%s", auth_help);
			return;
		}
	}
	helper_arg(1, 1, &argc, &argv, auth_help);

	str2ba(argv[0], &bdaddr);

	if (dev_id < 0) {
		dev_id = hci_for_each_dev(HCI_UP, find_conn, (long) &bdaddr);
		if (dev_id < 0) {
			fprintf(stderr, "Not connected.\n");
			exit(1);
		}
	}

	dd = hci_open_dev(dev_id);
	if (dd < 0) {
		perror("HCI device open failed");
		exit(1);
	}

	cr = malloc(sizeof(*cr) + sizeof(struct hci_conn_info));
	if (!cr) {
		perror("Can't allocate memory");
		exit(1);
	}

	bacpy(&cr->bdaddr, &bdaddr);
	cr->type = ACL_LINK;
	if (ioctl(dd, HCIGETCONNINFO, (unsigned long) cr) < 0) {
		perror("Get connection info failed");
		exit(1);
	}

	if (hci_authenticate_link(dd, htobs(cr->conn_info->handle), 25000) < 0) {
		perror("HCI authentication request failed");
		exit(1);
	}

	free(cr);

	hci_close_dev(dd);
}

/* Activate encryption */

static struct option enc_options[] = {
	{ "help",	0, 0, 'h' },
	{ 0, 0, 0, 0 }
};

static const char *enc_help =
	"Usage:\n"
	"\tenc <bdaddr> [encrypt enable]\n";

static void cmd_enc(int dev_id, int argc, char **argv)
{
	struct hci_conn_info_req *cr;
	bdaddr_t bdaddr;
	uint8_t encrypt;
	int opt, dd;

	for_each_opt(opt, enc_options, NULL) {
		switch (opt) {
		default:
			printf("%s", enc_help);
			return;
		}
	}
	helper_arg(1, 2, &argc, &argv, enc_help);

	str2ba(argv[0], &bdaddr);

	if (dev_id < 0) {
		dev_id = hci_for_each_dev(HCI_UP, find_conn, (long) &bdaddr);
		if (dev_id < 0) {
			fprintf(stderr, "Not connected.\n");
			exit(1);
		}
	}

	dd = hci_open_dev(dev_id);
	if (dd < 0) {
		perror("HCI device open failed");
		exit(1);
	}

	cr = malloc(sizeof(*cr) + sizeof(struct hci_conn_info));
	if (!cr) {
		perror("Can't allocate memory");
		exit(1);
	}

	bacpy(&cr->bdaddr, &bdaddr);
	cr->type = ACL_LINK;
	if (ioctl(dd, HCIGETCONNINFO, (unsigned long) cr) < 0) {
		perror("Get connection info failed");
		exit(1);
	}

	encrypt = (argc > 1) ? atoi(argv[1]) : 1;

	if (hci_encrypt_link(dd, htobs(cr->conn_info->handle), encrypt, 25000) < 0) {
		perror("HCI set encryption request failed");
		exit(1);
	}

	free(cr);

	hci_close_dev(dd);
}

/* Change connection link key */

static struct option key_options[] = {
	{ "help",	0, 0, 'h' },
	{ 0, 0, 0, 0 }
};

static const char *key_help =
	"Usage:\n"
	"\tkey <bdaddr>\n";

static void cmd_key(int dev_id, int argc, char **argv)
{
	struct hci_conn_info_req *cr;
	bdaddr_t bdaddr;
	int opt, dd;

	for_each_opt(opt, key_options, NULL) {
		switch (opt) {
		default:
			printf("%s", key_help);
			return;
		}
	}
	helper_arg(1, 1, &argc, &argv, key_help);

	str2ba(argv[0], &bdaddr);

	if (dev_id < 0) {
		dev_id = hci_for_each_dev(HCI_UP, find_conn, (long) &bdaddr);
		if (dev_id < 0) {
			fprintf(stderr, "Not connected.\n");
			exit(1);
		}
	}

	dd = hci_open_dev(dev_id);
	if (dd < 0) {
		perror("HCI device open failed");
		exit(1);
	}

	cr = malloc(sizeof(*cr) + sizeof(struct hci_conn_info));
	if (!cr) {
		perror("Can't allocate memory");
		exit(1);
	}

	bacpy(&cr->bdaddr, &bdaddr);
	cr->type = ACL_LINK;
	if (ioctl(dd, HCIGETCONNINFO, (unsigned long) cr) < 0) {
		perror("Get connection info failed");
		exit(1);
	}

	if (hci_change_link_key(dd, htobs(cr->conn_info->handle), 25000) < 0) {
		perror("Changing link key failed");
		exit(1);
	}

	free(cr);

	hci_close_dev(dd);
}

/* Read clock offset */

static struct option clkoff_options[] = {
	{ "help",	0, 0, 'h' },
	{ 0, 0, 0, 0 }
};

static const char *clkoff_help =
	"Usage:\n"
	"\tclkoff <bdaddr>\n";

static void cmd_clkoff(int dev_id, int argc, char **argv)
{
	struct hci_conn_info_req *cr;
	bdaddr_t bdaddr;
	uint16_t offset;
	int opt, dd;

	for_each_opt(opt, clkoff_options, NULL) {
		switch (opt) {
		default:
			printf("%s", clkoff_help);
			return;
		}
	}
	helper_arg(1, 1, &argc, &argv, clkoff_help);

	str2ba(argv[0], &bdaddr);

	if (dev_id < 0) {
		dev_id = hci_for_each_dev(HCI_UP, find_conn, (long) &bdaddr);
		if (dev_id < 0) {
			fprintf(stderr, "Not connected.\n");
			exit(1);
		}
	}

	dd = hci_open_dev(dev_id);
	if (dd < 0) {
		perror("HCI device open failed");
		exit(1);
	}

	cr = malloc(sizeof(*cr) + sizeof(struct hci_conn_info));
	if (!cr) {
		perror("Can't allocate memory");
		exit(1);
	}

	bacpy(&cr->bdaddr, &bdaddr);
	cr->type = ACL_LINK;
	if (ioctl(dd, HCIGETCONNINFO, (unsigned long) cr) < 0) {
		perror("Get connection info failed");
		exit(1);
	}

	if (hci_read_clock_offset(dd, htobs(cr->conn_info->handle), &offset, 1000) < 0) {
		perror("Reading clock offset failed");
		exit(1);
	}

	printf("Clock offset: 0x%4.4x\n", btohs(offset));

	free(cr);

	hci_close_dev(dd);
}

/* Read clock */

static struct option clock_options[] = {
	{ "help",	0, 0, 'h' },
	{ 0, 0, 0, 0 }
};

static const char *clock_help =
	"Usage:\n"
	"\tclock [bdaddr] [which clock]\n";

static void cmd_clock(int dev_id, int argc, char **argv)
{
	struct hci_conn_info_req *cr;
	bdaddr_t bdaddr;
	uint8_t which;
	uint32_t handle, clock;
	uint16_t accuracy;
	int opt, dd;

	for_each_opt(opt, clock_options, NULL) {
		switch (opt) {
		default:
			printf("%s", clock_help);
			return;
		}
	}
	helper_arg(0, 2, &argc, &argv, clock_help);

	if (argc > 0)
		str2ba(argv[0], &bdaddr);
	else
		bacpy(&bdaddr, BDADDR_ANY);

	if (dev_id < 0 && !bacmp(&bdaddr, BDADDR_ANY))
		dev_id = hci_get_route(NULL);

	if (dev_id < 0) {
		dev_id = hci_for_each_dev(HCI_UP, find_conn, (long) &bdaddr);
		if (dev_id < 0) {
			fprintf(stderr, "Not connected.\n");
			exit(1);
		}
	}

	dd = hci_open_dev(dev_id);
	if (dd < 0) {
		perror("HCI device open failed");
		exit(1);
	}

	if (bacmp(&bdaddr, BDADDR_ANY)) {
		cr = malloc(sizeof(*cr) + sizeof(struct hci_conn_info));
		if (!cr) {
			perror("Can't allocate memory");
			exit(1);
		}

		bacpy(&cr->bdaddr, &bdaddr);
		cr->type = ACL_LINK;
		if (ioctl(dd, HCIGETCONNINFO, (unsigned long) cr) < 0) {
			perror("Get connection info failed");
			free(cr);
			exit(1);
		}

		handle = htobs(cr->conn_info->handle);
		which = (argc > 1) ? atoi(argv[1]) : 0x01;

		free(cr);
	} else {
		handle = 0x00;
		which = 0x00;
	}

	if (hci_read_clock(dd, handle, which, &clock, &accuracy, 1000) < 0) {
		perror("Reading clock failed");
		exit(1);
	}

	accuracy = btohs(accuracy);

	printf("Clock:    0x%4.4x\n", btohl(clock));
	printf("Accuracy: %.2f msec\n", (float) accuracy * 0.3125);

	hci_close_dev(dd);
}

static int read_flags(uint8_t *flags, const uint8_t *data, size_t size)
{
	size_t offset;

	if (!flags || !data)
		return -EINVAL;

	offset = 0;
	while (offset < size) {
		uint8_t len = data[offset];
		uint8_t type;

		/* Check if it is the end of the significant part */
		if (len == 0)
			break;

		if (len + offset > size)
			break;

		type = data[offset + 1];

		if (type == FLAGS_AD_TYPE) {
			*flags = data[offset + 2];
			return 0;
		}

		offset += 1 + len;
	}

	return -ENOENT;
}

static int check_report_filter(uint8_t procedure, le_advertising_info *info)
{
	uint8_t flags;

	/* If no discovery procedure is set, all reports are treat as valid */
	if (procedure == 0)
		return 1;

	/* Read flags AD type value from the advertising report if it exists */
	if (read_flags(&flags, info->data, info->length))
		return 0;

	switch (procedure) {
	case 'l': /* Limited Discovery Procedure */
		if (flags & FLAGS_LIMITED_MODE_BIT)
			return 1;
		break;
	case 'g': /* General Discovery Procedure */
		if (flags & (FLAGS_LIMITED_MODE_BIT | FLAGS_GENERAL_MODE_BIT))
			return 1;
		break;
	default:
		fprintf(stderr, "Unknown discovery procedure\n");
	}

	return 0;
}

static void sigint_handler(int sig)
{
	signal_received = sig;
}

static void eir_parse_name(uint8_t *eir, size_t eir_len,
						char *buf, size_t buf_len)
{
	size_t offset;

	offset = 0;
	while (offset < eir_len) {
		uint8_t field_len = eir[0];
		size_t name_len;

		/* Check for the end of EIR */
		if (field_len == 0)
			break;

		if (offset + field_len > eir_len)
			goto failed;

		switch (eir[1]) {
		case EIR_NAME_SHORT:
		case EIR_NAME_COMPLETE:
			name_len = field_len - 1;
			if (name_len > buf_len)
				goto failed;

			memcpy(buf, &eir[2], name_len);
			return;
		}

		offset += field_len + 1;
		eir += field_len + 1;
	}

failed:
	snprintf(buf, buf_len, "(unknown)");
}

static int print_advertising_devices(int dd, uint8_t filter_type)
{
	unsigned char buf[HCI_MAX_EVENT_SIZE], *ptr;
	struct hci_filter nf, of;
	struct sigaction sa;
	socklen_t olen;
	int len;

	olen = sizeof(of);
	if (getsockopt(dd, SOL_HCI, HCI_FILTER, &of, &olen) < 0) {
		printf("Could not get socket options\n");
		return -1;
	}

	hci_filter_clear(&nf);
	hci_filter_set_ptype(HCI_EVENT_PKT, &nf);
	hci_filter_set_event(EVT_LE_META_EVENT, &nf);

	if (setsockopt(dd, SOL_HCI, HCI_FILTER, &nf, sizeof(nf)) < 0) {
		printf("Could not set socket options\n");
		return -1;
	}

	memset(&sa, 0, sizeof(sa));
	sa.sa_flags = SA_NOCLDSTOP;
	sa.sa_handler = sigint_handler;
	sigaction(SIGINT, &sa, NULL);

	while (1) {
		evt_le_meta_event *meta;
		le_advertising_info *info;
		char addr[18];

		while ((len = read(dd, buf, sizeof(buf))) < 0) {
			if (errno == EINTR && signal_received == SIGINT) {
				len = 0;
				goto done;
			}

			if (errno == EAGAIN || errno == EINTR)
				continue;
			goto done;
		}

		ptr = buf + (1 + HCI_EVENT_HDR_SIZE);
		len -= (1 + HCI_EVENT_HDR_SIZE);

		meta = (void *) ptr;

		if (meta->subevent != 0x02)
			goto done;

		/* Ignoring multiple reports */
		info = (le_advertising_info *) (meta->data + 1);
		if (check_report_filter(filter_type, info)) {
			char name[30];

			memset(name, 0, sizeof(name));

			ba2str(&info->bdaddr, addr);
			eir_parse_name(info->data, info->length,
							name, sizeof(name) - 1);

			printf("%s %s RSSI:%d\n", addr, name, (int8_t )info->data[info->length]);
		}
	}

done:
	setsockopt(dd, SOL_HCI, HCI_FILTER, &of, sizeof(of));

	if (len < 0)
		return -1;

	return 0;
}

static struct option lescan_options[] = {
	{ "help",	0, 0, 'h' },
	{ "static",	0, 0, 's' },
	{ "privacy",	0, 0, 'p' },
	{ "passive",	0, 0, 'P' },
	{ "whitelist",	0, 0, 'w' },
	{ "discovery",	1, 0, 'd' },
	{ "duplicates",	0, 0, 'D' },
	{ 0, 0, 0, 0 }
};

static const char *lescan_help =
	"Usage:\n"
	"\tlescan [--privacy] enable privacy\n"
	"\tlescan [--passive] set scan type passive (default active)\n"
	"\tlescan [--whitelist] scan for address in the whitelist only\n"
	"\tlescan [--discovery=g|l] enable general or limited discovery"
		"procedure\n"
	"\tlescan [--duplicates] don't filter duplicates\n";

static void cmd_lescan(int dev_id, int argc, char **argv)
{
	int err, opt, dd;
	uint8_t own_type = LE_PUBLIC_ADDRESS;
	uint8_t scan_type = 0x01;
	uint8_t filter_type = 0;
	uint8_t filter_policy = 0x00;
	uint16_t interval = htobs(0x0010);
	uint16_t window = htobs(0x0010);
	uint8_t filter_dup = 0x01;

	for_each_opt(opt, lescan_options, NULL) {
		switch (opt) {
		case 's':
			own_type = LE_RANDOM_ADDRESS;
			break;
		case 'p':
			own_type = LE_RANDOM_ADDRESS;
			break;
		case 'P':
			scan_type = 0x00; /* Passive */
			break;
		case 'w':
			filter_policy = 0x01; /* Whitelist */
			break;
		case 'd':
			filter_type = optarg[0];
			if (filter_type != 'g' && filter_type != 'l') {
				fprintf(stderr, "Unknown discovery procedure\n");
				exit(1);
			}

			interval = htobs(0x0012);
			window = htobs(0x0012);
			break;
		case 'D':
			filter_dup = 0x00;
			break;
		default:
			printf("%s", lescan_help);
			return;
		}
	}
	helper_arg(0, 1, &argc, &argv, lescan_help);

	if (dev_id < 0)
		dev_id = hci_get_route(NULL);

	dd = hci_open_dev(dev_id);
	if (dd < 0) {
		perror("Could not open device");
		exit(1);
	}

	err = hci_le_set_scan_parameters(dd, scan_type, interval, window,
						own_type, filter_policy, 10000);
	if (err < 0) {
		perror("Set scan parameters failed");
		exit(1);
	}

	err = hci_le_set_scan_enable(dd, 0x01, filter_dup, 10000);
	if (err < 0) {
		perror("Enable scan failed");
		exit(1);
	}

	printf("LE Scan ...\n");

	err = print_advertising_devices(dd, filter_type);
	if (err < 0) {
		perror("Could not receive advertising events");
		exit(1);
	}

	err = hci_le_set_scan_enable(dd, 0x00, filter_dup, 10000);
	if (err < 0) {
		perror("Disable scan failed");
		exit(1);
	}

	hci_close_dev(dd);
}

static struct option leinfo_options[] = {
	{ "help",	0, 0, 'h' },
	{ "static",	0, 0, 's' },
	{ "random",	0, 0, 'r' },
	{ 0, 0, 0, 0 }
};

static const char *leinfo_help =
	"Usage:\n"
	"\tleinfo [--static] [--random] <bdaddr>\n";

static void cmd_leinfo(int dev_id, int argc, char **argv)
{
	bdaddr_t bdaddr;
	uint16_t handle;
	uint8_t features[8];
	struct hci_version version;
	uint16_t interval, latency, max_ce_length, max_interval, min_ce_length;
	uint16_t min_interval, supervision_timeout, window;
	uint8_t initiator_filter, own_bdaddr_type, peer_bdaddr_type;
	int opt, err, dd;

	own_bdaddr_type = LE_PUBLIC_ADDRESS;
	peer_bdaddr_type = LE_PUBLIC_ADDRESS;

	for_each_opt(opt, leinfo_options, NULL) {
		switch (opt) {
		case 's':
			own_bdaddr_type = LE_RANDOM_ADDRESS;
			break;
		case 'r':
			peer_bdaddr_type = LE_RANDOM_ADDRESS;
			break;
		default:
			printf("%s", leinfo_help);
			return;
		}
	}
	helper_arg(1, 1, &argc, &argv, leinfo_help);

	str2ba(argv[0], &bdaddr);

	printf("Requesting information ...\n");

	if (dev_id < 0)
		dev_id = hci_get_route(NULL);

	dd = hci_open_dev(dev_id);
	if (dd < 0) {
		perror("Could not open device");
		exit(1);
	}

	interval = htobs(0x0004);
	window = htobs(0x0004);
	initiator_filter = 0;
	min_interval = htobs(0x000F);
	max_interval = htobs(0x000F);
	latency = htobs(0x0000);
	supervision_timeout = htobs(0x0C80);
	min_ce_length = htobs(0x0000);
	max_ce_length = htobs(0x0000);

	err = hci_le_create_conn(dd, interval, window, initiator_filter,
			peer_bdaddr_type, bdaddr, own_bdaddr_type, min_interval,
			max_interval, latency, supervision_timeout,
			min_ce_length, max_ce_length, &handle, 25000);
	if (err < 0) {
		perror("Could not create connection");
		exit(1);
	}

	printf("\tHandle: %d (0x%04x)\n", handle, handle);

	if (hci_read_remote_version(dd, handle, &version, 20000) == 0) {
		char *ver = lmp_vertostr(version.lmp_ver);
		printf("\tLMP Version: %s (0x%x) LMP Subversion: 0x%x\n"
			"\tManufacturer: %s (%d)\n",
			ver ? ver : "n/a",
			version.lmp_ver,
			version.lmp_subver,
			bt_compidtostr(version.manufacturer),
			version.manufacturer);
		if (ver)
			bt_free(ver);
	}

	memset(features, 0, sizeof(features));
	hci_le_read_remote_features(dd, handle, features, 20000);

	printf("\tFeatures: 0x%2.2x 0x%2.2x 0x%2.2x 0x%2.2x "
				"0x%2.2x 0x%2.2x 0x%2.2x 0x%2.2x\n",
		features[0], features[1], features[2], features[3],
		features[4], features[5], features[6], features[7]);

	usleep(10000);
	hci_disconnect(dd, handle, HCI_OE_USER_ENDED_CONNECTION, 10000);

	hci_close_dev(dd);
}

static struct option lecc_options[] = {
	{ "help",	0, 0, 'h' },
	{ "static",	0, 0, 's' },
	{ "random",	0, 0, 'r' },
	{ "whitelist",	0, 0, 'w' },
	{ 0, 0, 0, 0 }
};

static const char *lecc_help =
	"Usage:\n"
	"\tlecc [--static] [--random] <bdaddr>\n"
	"\tlecc --whitelist\n";

static void cmd_lecc(int dev_id, int argc, char **argv)
{
	int err, opt, dd;
	bdaddr_t bdaddr;
	uint16_t interval, latency, max_ce_length, max_interval, min_ce_length;
	uint16_t min_interval, supervision_timeout, window, handle;
	uint8_t initiator_filter, own_bdaddr_type, peer_bdaddr_type;

	own_bdaddr_type = LE_PUBLIC_ADDRESS;
	peer_bdaddr_type = LE_PUBLIC_ADDRESS;
	initiator_filter = 0; /* Use peer address */

	for_each_opt(opt, lecc_options, NULL) {
		switch (opt) {
		case 's':
			own_bdaddr_type = LE_RANDOM_ADDRESS;
			break;
		case 'r':
			peer_bdaddr_type = LE_RANDOM_ADDRESS;
			break;
		case 'w':
			initiator_filter = 0x01; /* Use white list */
			break;
		default:
			printf("%s", lecc_help);
			return;
		}
	}
	helper_arg(0, 1, &argc, &argv, lecc_help);

	if (dev_id < 0)
		dev_id = hci_get_route(NULL);

	dd = hci_open_dev(dev_id);
	if (dd < 0) {
		perror("Could not open device");
		exit(1);
	}

	memset(&bdaddr, 0, sizeof(bdaddr_t));
	if (argv[0])
		str2ba(argv[0], &bdaddr);

	interval = htobs(0x0004);
	window = htobs(0x0004);
	min_interval = htobs(0x000F);
	max_interval = htobs(0x000F);
	latency = htobs(0x0000);
	supervision_timeout = htobs(0x0C80);
	min_ce_length = htobs(0x0001);
	max_ce_length = htobs(0x0001);

	err = hci_le_create_conn(dd, interval, window, initiator_filter,
			peer_bdaddr_type, bdaddr, own_bdaddr_type, min_interval,
			max_interval, latency, supervision_timeout,
			min_ce_length, max_ce_length, &handle, 25000);
	if (err < 0) {
		perror("Could not create connection");
		exit(1);
	}

	printf("Connection handle %d\n", handle);

	hci_close_dev(dd);
}

static struct option lewladd_options[] = {
	{ "help",	0, 0, 'h' },
	{ "random",	0, 0, 'r' },
	{ 0, 0, 0, 0 }
};

static const char *lewladd_help =
	"Usage:\n"
	"\tlewladd [--random] <bdaddr>\n";

static void cmd_lewladd(int dev_id, int argc, char **argv)
{
	int err, opt, dd;
	bdaddr_t bdaddr;
	uint8_t bdaddr_type = LE_PUBLIC_ADDRESS;

	for_each_opt(opt, lewladd_options, NULL) {
		switch (opt) {
		case 'r':
			bdaddr_type = LE_RANDOM_ADDRESS;
			break;
		default:
			printf("%s", lewladd_help);
			return;
		}
	}

	helper_arg(1, 1, &argc, &argv, lewladd_help);

	if (dev_id < 0)
		dev_id = hci_get_route(NULL);

	dd = hci_open_dev(dev_id);
	if (dd < 0) {
		perror("Could not open device");
		exit(1);
	}

	str2ba(argv[0], &bdaddr);

	err = hci_le_add_white_list(dd, &bdaddr, bdaddr_type, 1000);
	hci_close_dev(dd);

	if (err < 0) {
		err = -errno;
		fprintf(stderr, "Can't add to white list: %s(%d)\n",
							strerror(-err), -err);
		exit(1);
	}
}

static struct option lewlrm_options[] = {
	{ "help",	0, 0, 'h' },
	{ 0, 0, 0, 0 }
};

static const char *lewlrm_help =
	"Usage:\n"
	"\tlewlrm <bdaddr>\n";

static void cmd_lewlrm(int dev_id, int argc, char **argv)
{
	int err, opt, dd;
	bdaddr_t bdaddr;

	for_each_opt(opt, lewlrm_options, NULL) {
		switch (opt) {
		default:
			printf("%s", lewlrm_help);
			return;
		}
	}

	helper_arg(1, 1, &argc, &argv, lewlrm_help);

	if (dev_id < 0)
		dev_id = hci_get_route(NULL);

	dd = hci_open_dev(dev_id);
	if (dd < 0) {
		perror("Could not open device");
		exit(1);
	}

	str2ba(argv[0], &bdaddr);

	err = hci_le_rm_white_list(dd, &bdaddr, LE_PUBLIC_ADDRESS, 1000);
	hci_close_dev(dd);

	if (err < 0) {
		err = errno;
		fprintf(stderr, "Can't remove from white list: %s(%d)\n",
							strerror(err), err);
		exit(1);
	}
}

static struct option lewlsz_options[] = {
	{ "help",	0, 0, 'h' },
	{ 0, 0, 0, 0 }
};

static const char *lewlsz_help =
	"Usage:\n"
	"\tlewlsz\n";

static void cmd_lewlsz(int dev_id, int argc, char **argv)
{
	int err, dd, opt;
	uint8_t size;

	for_each_opt(opt, lewlsz_options, NULL) {
		switch (opt) {
		default:
			printf("%s", lewlsz_help);
			return;
		}
	}

	helper_arg(0, 0, &argc, &argv, lewlsz_help);

	if (dev_id < 0)
		dev_id = hci_get_route(NULL);

	dd = hci_open_dev(dev_id);
	if (dd < 0) {
		perror("Could not open device");
		exit(1);
	}

	err = hci_le_read_white_list_size(dd, &size, 1000);
	hci_close_dev(dd);

	if (err < 0) {
		err = -errno;
		fprintf(stderr, "Can't read white list size: %s(%d)\n",
							strerror(-err), -err);
		exit(1);
	}

	printf("White list size: %d\n", size);
}

static struct option lewlclr_options[] = {
	{ "help",	0, 0, 'h' },
	{ 0, 0, 0, 0 }
};

static const char *lewlclr_help =
	"Usage:\n"
	"\tlewlclr\n";

static void cmd_lewlclr(int dev_id, int argc, char **argv)
{
	int err, dd, opt;

	for_each_opt(opt, lewlclr_options, NULL) {
		switch (opt) {
		default:
			printf("%s", lewlclr_help);
			return;
		}
	}

	helper_arg(0, 0, &argc, &argv, lewlclr_help);

	if (dev_id < 0)
		dev_id = hci_get_route(NULL);

	dd = hci_open_dev(dev_id);
	if (dd < 0) {
		perror("Could not open device");
		exit(1);
	}

	err = hci_le_clear_white_list(dd, 1000);
	hci_close_dev(dd);

	if (err < 0) {
		err = -errno;
		fprintf(stderr, "Can't clear white list: %s(%d)\n",
							strerror(-err), -err);
		exit(1);
	}
}

static struct option lerladd_options[] = {
	{ "help",	0, 0, 'h' },
	{ "random",	0, 0, 'r' },
	{ "local",	1, 0, 'l' },
	{ "peer",	1, 0, 'p' },
	{ 0, 0, 0, 0 }
};

static const char *lerladd_help =
	"Usage:\n"
	"\tlerladd [--local irk] [--peer irk] [--random] <bdaddr>\n";

static void cmd_lerladd(int dev_id, int argc, char **argv)
{
	int err, opt, dd;
	bdaddr_t bdaddr;
	uint8_t bdaddr_type = LE_PUBLIC_ADDRESS;
	uint8_t local_irk[16], peer_irk[16];

	memset(local_irk, 0, 16);
	memset(peer_irk, 0, 16);

	for_each_opt(opt, lerladd_options, NULL) {
		switch (opt) {
		case 'r':
			bdaddr_type = LE_RANDOM_ADDRESS;
			break;
		case 'l':
			str2buf(optarg, local_irk, 16);
			break;
		case 'p':
			str2buf(optarg, peer_irk, 16);
			break;
		default:
			printf("%s", lerladd_help);
			return;
		}
	}

	helper_arg(1, 1, &argc, &argv, lerladd_help);

	if (dev_id < 0)
		dev_id = hci_get_route(NULL);

	dd = hci_open_dev(dev_id);
	if (dd < 0) {
		perror("Could not open device");
		exit(1);
	}

	str2ba(argv[0], &bdaddr);

	err = hci_le_add_resolving_list(dd, &bdaddr, bdaddr_type,
						peer_irk, local_irk, 1000);
	hci_close_dev(dd);

	if (err < 0) {
		err = -errno;
		fprintf(stderr, "Can't add to resolving list: %s(%d)\n",
							strerror(-err), -err);
		exit(1);
	}
}

static struct option lerlrm_options[] = {
	{ "help",	0, 0, 'h' },
	{ 0, 0, 0, 0 }
};

static const char *lerlrm_help =
	"Usage:\n"
	"\tlerlrm <bdaddr>\n";

static void cmd_lerlrm(int dev_id, int argc, char **argv)
{
	int err, opt, dd;
	bdaddr_t bdaddr;

	for_each_opt(opt, lerlrm_options, NULL) {
		switch (opt) {
		default:
			printf("%s", lerlrm_help);
			return;
		}
	}

	helper_arg(1, 1, &argc, &argv, lerlrm_help);

	if (dev_id < 0)
		dev_id = hci_get_route(NULL);

	dd = hci_open_dev(dev_id);
	if (dd < 0) {
		perror("Could not open device");
		exit(1);
	}

	str2ba(argv[0], &bdaddr);

	err = hci_le_rm_resolving_list(dd, &bdaddr, LE_PUBLIC_ADDRESS, 1000);
	hci_close_dev(dd);

	if (err < 0) {
		err = errno;
		fprintf(stderr, "Can't remove from resolving list: %s(%d)\n",
							strerror(err), err);
		exit(1);
	}
}

static struct option lerlclr_options[] = {
	{ "help",	0, 0, 'h' },
	{ 0, 0, 0, 0 }
};

static const char *lerlclr_help =
	"Usage:\n"
	"\tlerlclr\n";

static void cmd_lerlclr(int dev_id, int argc, char **argv)
{
	int err, dd, opt;

	for_each_opt(opt, lerlclr_options, NULL) {
		switch (opt) {
		default:
			printf("%s", lerlclr_help);
			return;
		}
	}

	helper_arg(0, 0, &argc, &argv, lerlclr_help);

	if (dev_id < 0)
		dev_id = hci_get_route(NULL);

	dd = hci_open_dev(dev_id);
	if (dd < 0) {
		perror("Could not open device");
		exit(1);
	}

	err = hci_le_clear_resolving_list(dd, 1000);
	hci_close_dev(dd);

	if (err < 0) {
		err = -errno;
		fprintf(stderr, "Can't clear resolving list: %s(%d)\n",
							strerror(-err), -err);
		exit(1);
	}
}

static struct option lerlsz_options[] = {
	{ "help",	0, 0, 'h' },
	{ 0, 0, 0, 0 }
};

static const char *lerlsz_help =
	"Usage:\n"
	"\tlerlsz\n";

static void cmd_lerlsz(int dev_id, int argc, char **argv)
{
	int err, dd, opt;
	uint8_t size;

	for_each_opt(opt, lerlsz_options, NULL) {
		switch (opt) {
		default:
			printf("%s", lerlsz_help);
			return;
		}
	}

	helper_arg(0, 0, &argc, &argv, lerlsz_help);

	if (dev_id < 0)
		dev_id = hci_get_route(NULL);

	dd = hci_open_dev(dev_id);
	if (dd < 0) {
		perror("Could not open device");
		exit(1);
	}

	err = hci_le_read_resolving_list_size(dd, &size, 1000);
	hci_close_dev(dd);

	if (err < 0) {
		err = -errno;
		fprintf(stderr, "Can't read resolving list size: %s(%d)\n",
							strerror(-err), -err);
		exit(1);
	}

	printf("Resolving list size: %d\n", size);
}

static struct option lerlon_options[] = {
	{ "help",	0, 0, 'h' },
	{ 0, 0, 0, 0 }
};

static const char *lerlon_help =
	"Usage:\n"
	"\tlerlon\n";

static void cmd_lerlon(int dev_id, int argc, char **argv)
{
	int err, dd, opt;

	for_each_opt(opt, lerlon_options, NULL) {
		switch (opt) {
		default:
			printf("%s", lerlon_help);
			return;
		}
	}

	helper_arg(0, 0, &argc, &argv, lerlon_help);

	if (dev_id < 0)
		dev_id = hci_get_route(NULL);

	dd = hci_open_dev(dev_id);
	if (dd < 0) {
		perror("Could not open device");
		exit(1);
	}

	err = hci_le_set_address_resolution_enable(dd, 0x01, 1000);
	hci_close_dev(dd);

	if (err < 0) {
		err = -errno;
		fprintf(stderr, "Can't set address resolution enable: %s(%d)\n",
							strerror(-err), -err);
		exit(1);
	}
}

static struct option lerloff_options[] = {
	{ "help",	0, 0, 'h' },
	{ 0, 0, 0, 0 }
};

static const char *lerloff_help =
	"Usage:\n"
	"\tlerloff\n";

static void cmd_lerloff(int dev_id, int argc, char **argv)
{
	int err, dd, opt;

	for_each_opt(opt, lerloff_options, NULL) {
		switch (opt) {
		default:
			printf("%s", lerloff_help);
			return;
		}
	}

	helper_arg(0, 0, &argc, &argv, lerloff_help);

	if (dev_id < 0)
		dev_id = hci_get_route(NULL);

	dd = hci_open_dev(dev_id);
	if (dd < 0) {
		perror("Could not open device");
		exit(1);
	}

	err = hci_le_set_address_resolution_enable(dd, 0x00, 1000);
	hci_close_dev(dd);

	if (err < 0) {
		err = -errno;
		fprintf(stderr, "Can't set address resolution enable: %s(%d)\n",
							strerror(-err), -err);
		exit(1);
	}
}

static struct option ledc_options[] = {
	{ "help",	0, 0, 'h' },
	{ 0, 0, 0, 0 }
};

static const char *ledc_help =
	"Usage:\n"
	"\tledc <handle> [reason]\n";

static void cmd_ledc(int dev_id, int argc, char **argv)
{
	int err, opt, dd;
	uint16_t handle;
	uint8_t reason;

	for_each_opt(opt, ledc_options, NULL) {
		switch (opt) {
		default:
			printf("%s", ledc_help);
			return;
		}
	}
	helper_arg(1, 2, &argc, &argv, ledc_help);

	if (dev_id < 0)
		dev_id = hci_get_route(NULL);

	dd = hci_open_dev(dev_id);
	if (dd < 0) {
		perror("Could not open device");
		exit(1);
	}

	handle = atoi(argv[0]);

	reason = (argc > 1) ? atoi(argv[1]) : HCI_OE_USER_ENDED_CONNECTION;

	err = hci_disconnect(dd, handle, reason, 10000);
	if (err < 0) {
		perror("Could not disconnect");
		exit(1);
	}

	hci_close_dev(dd);
}

static struct option lecup_options[] = {
	{ "help",	0, 0, 'h' },
	{ "handle",	1, 0, 'H' },
	{ "min",	1, 0, 'm' },
	{ "max",	1, 0, 'M' },
	{ "latency",	1, 0, 'l' },
	{ "timeout",	1, 0, 't' },
	{ 0, 0, 0, 0 }
};

static const char *lecup_help =
	"Usage:\n"
	"\tlecup <handle> <min> <max> <latency> <timeout>\n"
	"\tOptions:\n"
	"\t    --handle=<0xXXXX>  LE connection handle\n"
	"\t    --min=<interval>   Range: 0x0006 to 0x0C80\n"
	"\t    --max=<interval>   Range: 0x0006 to 0x0C80\n"
	"\t    --latency=<range>  Slave latency. Range: 0x0000 to 0x03E8\n"
	"\t    --timeout=<time>   N * 10ms. Range: 0x000A to 0x0C80\n"
	"\n\t min/max range: 7.5ms to 4s. Multiply factor: 1.25ms"
	"\n\t timeout range: 100ms to 32.0s. Larger than max interval\n";

static void cmd_lecup(int dev_id, int argc, char **argv)
{
	uint16_t handle = 0, min, max, latency, timeout;
	int opt, dd;
	int options = 0;

	/* Aleatory valid values */
	min = 0x0C8;
	max = 0x0960;
	latency = 0x0007;
	timeout = 0x0C80;

	for_each_opt(opt, lecup_options, NULL) {
		switch (opt) {
		case 'H':
			handle = strtoul(optarg, NULL, 0);
			break;
		case 'm':
			min = strtoul(optarg, NULL, 0);
			break;
		case 'M':
			max = strtoul(optarg, NULL, 0);
			break;
		case 'l':
			latency = strtoul(optarg, NULL, 0);
			break;
		case 't':
			timeout = strtoul(optarg, NULL, 0);
			break;
		default:
			printf("%s", lecup_help);
			return;
		}

		options = 1;
	}

	if (options == 0) {
		helper_arg(5, 5, &argc, &argv, lecup_help);

		handle = strtoul(argv[0], NULL, 0);
		min = strtoul(argv[1], NULL, 0);
		max = strtoul(argv[2], NULL, 0);
		latency = strtoul(argv[3], NULL, 0);
		timeout = strtoul(argv[4], NULL, 0);
	}

	if (handle == 0) {
		printf("%s", lecup_help);
		return;
	}

	if (dev_id < 0)
		dev_id = hci_get_route(NULL);

	dd = hci_open_dev(dev_id);
	if (dd < 0) {
		fprintf(stderr, "HCI device open failed\n");
		exit(1);
	}

	if (hci_le_conn_update(dd, htobs(handle), htobs(min), htobs(max),
				htobs(latency), htobs(timeout), 5000) < 0) {
		int err = -errno;
		fprintf(stderr, "Could not change connection params: %s(%d)\n",
							strerror(-err), -err);
	}

	hci_close_dev(dd);
}

static struct {
	char *cmd;
	void (*func)(int dev_id, int argc, char **argv);
	char *doc;
} command[] = {
	{ "dev",      cmd_dev,     "Display local devices"                },
	{ "inq",      cmd_inq,     "Inquire remote devices"               },
	{ "scan",     cmd_scan,    "Scan for remote devices"              },
	{ "name",     cmd_name,    "Get name from remote device"          },
	{ "info",     cmd_info,    "Get information from remote device"   },
	{ "spinq",    cmd_spinq,   "Start periodic inquiry"               },
	{ "epinq",    cmd_epinq,   "Exit periodic inquiry"                },
	{ "cmd",      cmd_cmd,     "Submit arbitrary HCI commands"        },
	{ "con",      cmd_con,     "Display active connections"           },
	{ "cc",       cmd_cc,      "Create connection to remote device"   },
	{ "dc",       cmd_dc,      "Disconnect from remote device"        },
	{ "sr",       cmd_sr,      "Switch master/slave role"             },
	{ "cpt",      cmd_cpt,     "Change connection packet type"        },
	{ "rssi",     cmd_rssi,    "Display connection RSSI"              },
	{ "lq",       cmd_lq,      "Display link quality"                 },
	{ "tpl",      cmd_tpl,     "Display transmit power level"         },
	{ "afh",      cmd_afh,     "Display AFH channel map"              },
	{ "lp",       cmd_lp,      "Set/display link policy settings"     },
	{ "lst",      cmd_lst,     "Set/display link supervision timeout" },
	{ "auth",     cmd_auth,    "Request authentication"               },
	{ "enc",      cmd_enc,     "Set connection encryption"            },
	{ "key",      cmd_key,     "Change connection link key"           },
	{ "clkoff",   cmd_clkoff,  "Read clock offset"                    },
	{ "clock",    cmd_clock,   "Read local or remote clock"           },
	{ "lescan",   cmd_lescan,  "Start LE scan"                        },
	{ "leinfo",   cmd_leinfo,  "Get LE remote information"            },
	{ "lewladd",  cmd_lewladd, "Add device to LE White List"          },
	{ "lewlrm",   cmd_lewlrm,  "Remove device from LE White List"     },
	{ "lewlsz",   cmd_lewlsz,  "Read size of LE White List"           },
	{ "lewlclr",  cmd_lewlclr, "Clear LE White List"                  },
	{ "lerladd",  cmd_lerladd, "Add device to LE Resolving List"      },
	{ "lerlrm",   cmd_lerlrm,  "Remove device from LE Resolving List" },
	{ "lerlclr",  cmd_lerlclr, "Clear LE Resolving List"              },
	{ "lerlsz",   cmd_lerlsz,  "Read size of LE Resolving List"       },
	{ "lerlon",   cmd_lerlon,  "Enable LE Address Resolution"         },
	{ "lerloff",  cmd_lerloff, "Disable LE Address Resolution"        },
	{ "lecc",     cmd_lecc,    "Create a LE Connection"               },
	{ "ledc",     cmd_ledc,    "Disconnect a LE Connection"           },
	{ "lecup",    cmd_lecup,   "LE Connection Update"                 },
	{ NULL, NULL, 0 }
};

static void usage(void)
{
	int i;

	printf("hcitool - HCI Tool ver %s\n", VERSION);
	printf("Usage:\n"
		"\thcitool [options] <command> [command parameters]\n");
	printf("Options:\n"
		"\t--help\tDisplay help\n"
		"\t-i dev\tHCI device\n");
	printf("Commands:\n");
	for (i = 0; command[i].cmd; i++)
		printf("\t%-4s\t%s\n", command[i].cmd,
		command[i].doc);
	printf("\n"
		"For more information on the usage of each command use:\n"
		"\thcitool <command> --help\n" );
}

static struct option main_options[] = {
	{ "help",	0, 0, 'h' },
	{ "device",	1, 0, 'i' },
	{ 0, 0, 0, 0 }
};

int main(int argc, char *argv[])
{
	int opt, i, dev_id = -1;
	bdaddr_t ba;

	while ((opt=getopt_long(argc, argv, "+i:h", main_options, NULL)) != -1) {
		switch (opt) {
		case 'i':
			dev_id = hci_devid(optarg);
			if (dev_id < 0) {
				perror("Invalid device");
				exit(1);
			}
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

	if (argc < 1) {
		usage();
		exit(0);
	}

	if (dev_id != -1 && hci_devba(dev_id, &ba) < 0) {
		perror("Device is not available");
		exit(1);
	}

	for (i = 0; command[i].cmd; i++) {
		if (strncmp(command[i].cmd,
				argv[0], strlen(command[i].cmd)))
			continue;

		command[i].func(dev_id, argc, argv);
		break;
	}

	if (command[i].cmd == 0) {
		fprintf(stderr, "Unknown command - \"%s\"\n", *argv);
		exit(1);
	}

	return 0;
}
