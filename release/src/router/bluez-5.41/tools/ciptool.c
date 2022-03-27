/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
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
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>
#include <signal.h>
#include <poll.h>
#include <sys/ioctl.h>
#include <sys/socket.h>

#include "lib/bluetooth.h"
#include "lib/hci.h"
#include "lib/hci_lib.h"
#include "lib/l2cap.h"
#include "lib/sdp.h"
#include "lib/sdp_lib.h"
#include "lib/cmtp.h"

static volatile sig_atomic_t __io_canceled = 0;

static void sig_hup(int sig)
{
	return;
}

static void sig_term(int sig)
{
	__io_canceled = 1;
}

static char *cmtp_state[] = {
	"unknown",
	"connected",
	"open",
	"bound",
	"listening",
	"connecting",
	"connecting",
	"config",
	"disconnecting",
	"closed"
};

static char *cmtp_flagstostr(uint32_t flags)
{
	static char str[100] = "";

	strcat(str, "[");

	if (flags & (1 << CMTP_LOOPBACK))
		strcat(str, "loopback");

	strcat(str, "]");

	return str;
}

static int get_psm(bdaddr_t *src, bdaddr_t *dst, unsigned short *psm)
{
	sdp_session_t *s;
	sdp_list_t *srch, *attrs, *rsp;
	uuid_t svclass;
	uint16_t attr;
	int err;

	if (!(s = sdp_connect(src, dst, 0)))
		return -1;

	sdp_uuid16_create(&svclass, CIP_SVCLASS_ID);
	srch = sdp_list_append(NULL, &svclass);

	attr = SDP_ATTR_PROTO_DESC_LIST;
	attrs = sdp_list_append(NULL, &attr);

	err = sdp_service_search_attr_req(s, srch, SDP_ATTR_REQ_INDIVIDUAL, attrs, &rsp);

	sdp_close(s);

	if (err)
		return 0;

	for (; rsp; rsp = rsp->next) {
		sdp_record_t *rec = (sdp_record_t *) rsp->data;
		sdp_list_t *protos;

		if (!sdp_get_access_protos(rec, &protos)) {
			unsigned short p = sdp_get_proto_port(protos, L2CAP_UUID);
			if (p > 0) {
				*psm = p;
				return 1;
			}
		}
	}

	return 0;
}

static int do_connect(int ctl, int dev_id, bdaddr_t *src, bdaddr_t *dst, unsigned short psm, uint32_t flags)
{
	struct cmtp_connadd_req req;
	struct hci_dev_info di;
	struct sockaddr_l2 addr;
	struct l2cap_options opts;
	socklen_t size;
	int sk;

	hci_devinfo(dev_id, &di);
	if (!(di.link_policy & HCI_LP_RSWITCH)) {
		printf("Local device is not accepting role switch\n");
	}

	if ((sk = socket(AF_BLUETOOTH, SOCK_SEQPACKET, BTPROTO_L2CAP)) < 0) {
		perror("Can't create L2CAP socket");
		exit(1);
	}

	memset(&addr, 0, sizeof(addr));
	addr.l2_family = AF_BLUETOOTH;
	bacpy(&addr.l2_bdaddr, src);

	if (bind(sk, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		perror("Can't bind L2CAP socket");
		close(sk);
		exit(1);
	}

	memset(&opts, 0, sizeof(opts));
	size = sizeof(opts);

	if (getsockopt(sk, SOL_L2CAP, L2CAP_OPTIONS, &opts, &size) < 0) {
		perror("Can't get L2CAP options");
		close(sk);
		exit(1);
	}

	opts.imtu = CMTP_DEFAULT_MTU;
	opts.omtu = CMTP_DEFAULT_MTU;
	opts.flush_to = 0xffff;

	if (setsockopt(sk, SOL_L2CAP, L2CAP_OPTIONS, &opts, sizeof(opts)) < 0) {
		perror("Can't set L2CAP options");
		close(sk);
		exit(1);
	}

	memset(&addr, 0, sizeof(addr));
	addr.l2_family = AF_BLUETOOTH;
	bacpy(&addr.l2_bdaddr, dst);
	addr.l2_psm = htobs(psm);

	if (connect(sk, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		perror("Can't connect L2CAP socket");
		close(sk);
		exit(1);
	}

	req.sock = sk;
	req.flags = flags;

	if (ioctl(ctl, CMTPCONNADD, &req) < 0) {
		perror("Can't create connection");
		exit(1);
	}

	return sk;
}

static void cmd_show(int ctl, bdaddr_t *bdaddr, int argc, char **argv)
{
	struct cmtp_connlist_req req;
	struct cmtp_conninfo ci[16];
	char addr[18];
	unsigned int i;

	req.cnum = 16;
	req.ci   = ci;

	if (ioctl(ctl, CMTPGETCONNLIST, &req) < 0) {
		perror("Can't get connection list");
		exit(1);
	}

	for (i = 0; i < req.cnum; i++) {
		ba2str(&ci[i].bdaddr, addr);
		printf("%d %s %s %s\n", ci[i].num, addr,
			cmtp_state[ci[i].state],
			ci[i].flags ? cmtp_flagstostr(ci[i].flags) : "");
	}
}

static void cmd_search(int ctl, bdaddr_t *bdaddr, int argc, char **argv)
{
	inquiry_info *info = NULL;
	bdaddr_t src, dst;
	unsigned short psm;
	int i, dev_id, num_rsp, length, flags;
	char addr[18];
	uint8_t class[3];

	ba2str(bdaddr, addr);
	dev_id = hci_devid(addr);
	if (dev_id < 0) {
		dev_id = hci_get_route(NULL);
		hci_devba(dev_id, &src);
	} else
		bacpy(&src, bdaddr);

	length  = 8;	/* ~10 seconds */
	num_rsp = 0;
	flags   = 0;

	printf("Searching ...\n");

	num_rsp = hci_inquiry(dev_id, length, num_rsp, NULL, &info, flags);

	for (i = 0; i < num_rsp; i++) {
		memcpy(class, (info+i)->dev_class, 3);
		if ((class[1] == 2) && ((class[0] / 4) == 5)) {
			bacpy(&dst, &(info+i)->bdaddr);
			ba2str(&dst, addr);

			printf("\tChecking service for %s\n", addr);
			if (!get_psm(&src, &dst, &psm))
				continue;

			bt_free(info);

			printf("\tConnecting to device %s\n", addr);
			do_connect(ctl, dev_id, &src, &dst, psm, 0);
			return;
		}
	}

	bt_free(info);
	fprintf(stderr, "\tNo devices in range or visible\n");
	exit(1);
}

static void cmd_create(int ctl, bdaddr_t *bdaddr, int argc, char **argv)
{
	bdaddr_t src, dst;
	unsigned short psm;
	int dev_id;
	char addr[18];

	if (argc < 2)
		return;

	str2ba(argv[1], &dst);

	ba2str(bdaddr, addr);
	dev_id = hci_devid(addr);
	if (dev_id < 0) {
		dev_id = hci_get_route(&dst);
		hci_devba(dev_id, &src);
	} else
		bacpy(&src, bdaddr);

	if (argc < 3) {
		if (!get_psm(&src, &dst, &psm))
			psm = 4099;
	} else
		psm = atoi(argv[2]);

	do_connect(ctl, dev_id, &src, &dst, psm, 0);
}

static void cmd_release(int ctl, bdaddr_t *bdaddr, int argc, char **argv)
{
	struct cmtp_conndel_req req;
	struct cmtp_connlist_req cl;
	struct cmtp_conninfo ci[16];

	if (argc < 2) {
		cl.cnum = 16;
		cl.ci   = ci;

		if (ioctl(ctl, CMTPGETCONNLIST, &cl) < 0) {
			perror("Can't get connection list");
			exit(1);
		}

		if (cl.cnum == 0)
			return;

		if (cl.cnum != 1) {
			fprintf(stderr, "You have to specifiy the device address.\n");
			exit(1);
		}

		bacpy(&req.bdaddr, &ci[0].bdaddr);
	} else
		str2ba(argv[1], &req.bdaddr);

	if (ioctl(ctl, CMTPCONNDEL, &req) < 0) {
		perror("Can't release connection");
		exit(1);
	}
}

static void cmd_loopback(int ctl, bdaddr_t *bdaddr, int argc, char **argv)
{
	struct cmtp_conndel_req req;
	struct sigaction sa;
	struct pollfd p;
	sigset_t sigs;
	bdaddr_t src, dst;
	unsigned short psm;
	int dev_id, sk;
	char addr[18];

	if (argc < 2)
		return;

	str2ba(argv[1], &dst);

	ba2str(bdaddr, addr);
	dev_id = hci_devid(addr);
	if (dev_id < 0) {
		dev_id = hci_get_route(&dst);
		hci_devba(dev_id, &src);
	} else
		bacpy(&src, bdaddr);

	ba2str(&dst, addr);
	printf("Connecting to %s in loopback mode\n", addr);

	if (argc < 3) {
		if (!get_psm(&src, &dst, &psm))
			psm = 4099;
	} else
		psm = atoi(argv[2]);

	sk = do_connect(ctl, dev_id, &src, &dst, psm, (1 << CMTP_LOOPBACK));

	printf("Press CTRL-C for hangup\n");

	memset(&sa, 0, sizeof(sa));
	sa.sa_flags   = SA_NOCLDSTOP;
	sa.sa_handler = SIG_IGN;
	sigaction(SIGCHLD, &sa, NULL);
	sigaction(SIGPIPE, &sa, NULL);

	sa.sa_handler = sig_term;
	sigaction(SIGTERM, &sa, NULL);
	sigaction(SIGINT,  &sa, NULL);

	sa.sa_handler = sig_hup;
	sigaction(SIGHUP, &sa, NULL);

	sigfillset(&sigs);
	sigdelset(&sigs, SIGCHLD);
	sigdelset(&sigs, SIGPIPE);
	sigdelset(&sigs, SIGTERM);
	sigdelset(&sigs, SIGINT);
	sigdelset(&sigs, SIGHUP);

	p.fd = sk;
	p.events = POLLERR | POLLHUP;

	while (!__io_canceled) {
		p.revents = 0;
		if (ppoll(&p, 1, NULL, &sigs) > 0)
			break;
	}

	bacpy(&req.bdaddr, &dst);
	ioctl(ctl, CMTPCONNDEL, &req);
}

static struct {
	char *cmd;
	char *alt;
	void (*func)(int ctl, bdaddr_t *bdaddr, int argc, char **argv);
	char *opt;
	char *doc;
} command[] = {
	{ "show",     "list",       cmd_show,     0,          "Show remote connections"      },
	{ "search",   "scan",       cmd_search,   0,          "Search for a remote device"   },
	{ "connect",  "create",     cmd_create,   "<bdaddr>", "Connect a remote device"      },
	{ "release",  "disconnect", cmd_release,  "[bdaddr]", "Disconnect the remote device" },
	{ "loopback", "test",       cmd_loopback, "<bdaddr>", "Loopback test of a device"    },
	{ NULL, NULL, NULL, 0, 0 }
};

static void usage(void)
{
	int i;

	printf("ciptool - Bluetooth Common ISDN Access Profile (CIP)\n\n");

	printf("Usage:\n"
		"\tciptool [options] [command]\n"
		"\n");

	printf("Options:\n"
		"\t-i [hciX|bdaddr]   Local HCI device or BD Address\n"
		"\t-h, --help         Display help\n"
		"\n");

	printf("Commands:\n");
	for (i = 0; command[i].cmd; i++)
		printf("\t%-8s %-10s\t%s\n", command[i].cmd,
		command[i].opt ? command[i].opt : " ",
		command[i].doc);
	printf("\n");
}

static struct option main_options[] = {
	{ "help",	0, 0, 'h' },
	{ "device",	1, 0, 'i' },
	{ 0, 0, 0, 0 }
};

int main(int argc, char *argv[])
{
	bdaddr_t bdaddr;
	int i, opt, ctl;

	bacpy(&bdaddr, BDADDR_ANY);

	while ((opt = getopt_long(argc, argv, "+i:h", main_options, NULL)) != -1) {
		switch(opt) {
		case 'i':
			if (!strncmp(optarg, "hci", 3))
				hci_devba(atoi(optarg + 3), &bdaddr);
			else
				str2ba(optarg, &bdaddr);
			break;
		case 'h':
			usage();
			exit(0);
		default:
			exit(0);
		}
	}

	argc -= optind;
	argv += optind;
	optind = 0;

	if (argc < 1) {
		usage();
		return 0;
	}

	if ((ctl = socket(AF_BLUETOOTH, SOCK_RAW, BTPROTO_CMTP)) < 0 ) {
		perror("Can't open CMTP control socket");
		exit(1);
	}

	for (i = 0; command[i].cmd; i++) {
		if (strncmp(command[i].cmd, argv[0], 4) && strncmp(command[i].alt, argv[0], 4))
			continue;
		command[i].func(ctl, &bdaddr, argc, argv);
		close(ctl);
		exit(0);
	}

	usage();

	close(ctl);

	return 0;
}
