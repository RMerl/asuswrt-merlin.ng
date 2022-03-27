/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2000-2002  Maxim Krasnyansky <maxk@qualcomm.com>
 *  Copyright (C) 2003-2011  Marcel Holtmann <marcel@holtmann.org>
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
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <poll.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>

#include "parser/parser.h"
#include "parser/sdp.h"

#include "lib/hci.h"
#include "lib/hci_lib.h"

#define SNAP_LEN	HCI_MAX_FRAME_SIZE

/* Modes */
enum {
	PARSE,
	READ,
	WRITE,
	PPPDUMP,
	AUDIO
};

/* Default options */
static int  snap_len = SNAP_LEN;
static int  mode = PARSE;
static char *dump_file = NULL;
static char *pppdump_file = NULL;
static char *audio_file = NULL;

struct hcidump_hdr {
	uint16_t	len;
	uint8_t		in;
	uint8_t		pad;
	uint32_t	ts_sec;
	uint32_t	ts_usec;
} __attribute__ ((packed));
#define HCIDUMP_HDR_SIZE (sizeof(struct hcidump_hdr))

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

static uint8_t btsnoop_id[] = { 0x62, 0x74, 0x73, 0x6e, 0x6f, 0x6f, 0x70, 0x00 };

static uint32_t btsnoop_version = 0;
static uint32_t btsnoop_type = 0;

struct pktlog_hdr {
	uint32_t	len;
	uint64_t	ts;
	uint8_t		type;
} __attribute__ ((packed));
#define PKTLOG_HDR_SIZE (sizeof(struct pktlog_hdr))

static inline int read_n(int fd, char *buf, int len)
{
	int t = 0, w;

	while (len > 0) {
		if ((w = read(fd, buf, len)) < 0) {
			if (errno == EINTR || errno == EAGAIN)
				continue;
			return -1;
		}
		if (!w)
			return 0;
		len -= w; buf += w; t += w;
	}
	return t;
}

static inline int write_n(int fd, char *buf, int len)
{
	int t = 0, w;

	while (len > 0) {
		if ((w = write(fd, buf, len)) < 0) {
			if (errno == EINTR || errno == EAGAIN)
				continue;
			return -1;
		}
		if (!w)
			return 0;
		len -= w; buf += w; t += w;
	}
	return t;
}

static int process_frames(int dev, int sock, int fd, unsigned long flags)
{
	struct cmsghdr *cmsg;
	struct msghdr msg;
	struct iovec  iv;
	struct hcidump_hdr *dh;
	struct btsnoop_pkt *dp;
	struct frame frm;
	struct pollfd fds[2];
	int nfds = 0;
	char *buf;
	char ctrl[100];
	int len, hdr_size = HCIDUMP_HDR_SIZE;

	if (sock < 0)
		return -1;

	if (snap_len < SNAP_LEN)
		snap_len = SNAP_LEN;

	if (flags & DUMP_BTSNOOP)
		hdr_size = BTSNOOP_PKT_SIZE;

	buf = malloc(snap_len + hdr_size);
	if (!buf) {
		perror("Can't allocate data buffer");
		return -1;
	}

	dh = (void *) buf;
	dp = (void *) buf;
	frm.data = buf + hdr_size;

	if (dev == HCI_DEV_NONE)
		printf("system: ");
	else
		printf("device: hci%d ", dev);

	printf("snap_len: %d filter: 0x%lx\n", snap_len, parser.filter);

	memset(&msg, 0, sizeof(msg));

	fds[nfds].fd = sock;
	fds[nfds].events = POLLIN;
	fds[nfds].revents = 0;
	nfds++;

	while (1) {
		int i, n = poll(fds, nfds, -1);
		if (n <= 0)
			continue;

		for (i = 0; i < nfds; i++) {
			if (fds[i].revents & (POLLHUP | POLLERR | POLLNVAL)) {
				if (fds[i].fd == sock)
					printf("device: disconnected\n");
				else
					printf("client: disconnect\n");
				return 0;
			}
		}

		iv.iov_base = frm.data;
		iv.iov_len  = snap_len;

		msg.msg_iov = &iv;
		msg.msg_iovlen = 1;
		msg.msg_control = ctrl;
		msg.msg_controllen = 100;

		len = recvmsg(sock, &msg, MSG_DONTWAIT);
		if (len < 0) {
			if (errno == EAGAIN || errno == EINTR)
				continue;
			perror("Receive failed");
			return -1;
		}

		/* Process control message */
		frm.data_len = len;
		frm.dev_id = dev;
		frm.in = 0;
		frm.pppdump_fd = parser.pppdump_fd;
		frm.audio_fd   = parser.audio_fd;

		cmsg = CMSG_FIRSTHDR(&msg);
		while (cmsg) {
			int dir;
			switch (cmsg->cmsg_type) {
			case HCI_CMSG_DIR:
				memcpy(&dir, CMSG_DATA(cmsg), sizeof(int));
				frm.in = (uint8_t) dir;
				break;
			case HCI_CMSG_TSTAMP:
				memcpy(&frm.ts, CMSG_DATA(cmsg),
						sizeof(struct timeval));
				break;
			}
			cmsg = CMSG_NXTHDR(&msg, cmsg);
		}

		frm.ptr = frm.data;
		frm.len = frm.data_len;

		switch (mode) {
		case WRITE:
			/* Save or send dump */
			if (flags & DUMP_BTSNOOP) {
				uint64_t ts;
				uint8_t pkt_type = ((uint8_t *) frm.data)[0];
				dp->size = htobe32(frm.data_len);
				dp->len  = dp->size;
				dp->flags = be32toh(frm.in & 0x01);
				dp->drops = 0;
				ts = (frm.ts.tv_sec - 946684800ll) * 1000000ll + frm.ts.tv_usec;
				dp->ts = htobe64(ts + 0x00E03AB44A676000ll);
				if (pkt_type == HCI_COMMAND_PKT ||
						pkt_type == HCI_EVENT_PKT)
					dp->flags |= be32toh(0x02);
			} else {
				dh->len = htobs(frm.data_len);
				dh->in  = frm.in;
				dh->ts_sec  = htobl(frm.ts.tv_sec);
				dh->ts_usec = htobl(frm.ts.tv_usec);
			}

			if (write_n(fd, buf, frm.data_len + hdr_size) < 0) {
				perror("Write error");
				return -1;
			}
			break;

		default:
			/* Parse and print */
			parse(&frm);
			break;
		}
	}

	return 0;
}

static void read_dump(int fd)
{
	struct hcidump_hdr dh;
	struct btsnoop_pkt dp;
	struct pktlog_hdr ph;
	struct frame frm;
	int err;

	frm.data = malloc(HCI_MAX_FRAME_SIZE);
	if (!frm.data) {
		perror("Can't allocate data buffer");
		exit(1);
	}

	while (1) {
		if (parser.flags & DUMP_PKTLOG)
			err = read_n(fd, (void *) &ph, PKTLOG_HDR_SIZE);
		else if (parser.flags & DUMP_BTSNOOP)
			err = read_n(fd, (void *) &dp, BTSNOOP_PKT_SIZE);
		else
			err = read_n(fd, (void *) &dh, HCIDUMP_HDR_SIZE);

		if (err < 0)
			goto failed;
		if (!err)
			goto done;

		if (parser.flags & DUMP_PKTLOG) {
			switch (ph.type) {
			case 0x00:
				((uint8_t *) frm.data)[0] = HCI_COMMAND_PKT;
				frm.in = 0;
				break;
			case 0x01:
				((uint8_t *) frm.data)[0] = HCI_EVENT_PKT;
				frm.in = 1;
				break;
			case 0x02:
				((uint8_t *) frm.data)[0] = HCI_ACLDATA_PKT;
				frm.in = 0;
				break;
			case 0x03:
				((uint8_t *) frm.data)[0] = HCI_ACLDATA_PKT;
				frm.in = 1;
				break;
			default:
				lseek(fd, be32toh(ph.len) - 9, SEEK_CUR);
				continue;
			}

			frm.data_len = be32toh(ph.len) - 8;
			err = read_n(fd, frm.data + 1, frm.data_len - 1);
		} else if (parser.flags & DUMP_BTSNOOP) {
			uint32_t opcode;
			uint8_t pkt_type;

			switch (btsnoop_type) {
			case 1001:
				if (be32toh(dp.flags) & 0x02) {
					if (be32toh(dp.flags) & 0x01)
						pkt_type = HCI_EVENT_PKT;
					else
						pkt_type = HCI_COMMAND_PKT;
				} else
					pkt_type = HCI_ACLDATA_PKT;

				((uint8_t *) frm.data)[0] = pkt_type;

				frm.data_len = be32toh(dp.len) + 1;
				err = read_n(fd, frm.data + 1, frm.data_len - 1);
				break;

			case 1002:
				frm.data_len = be32toh(dp.len);
				err = read_n(fd, frm.data, frm.data_len);
				break;

			case 2001:
				opcode = be32toh(dp.flags) & 0xffff;

				switch (opcode) {
				case 2:
					pkt_type = HCI_COMMAND_PKT;
					frm.in = 0;
					break;
				case 3:
					pkt_type = HCI_EVENT_PKT;
					frm.in = 1;
					break;
				case 4:
					pkt_type = HCI_ACLDATA_PKT;
					frm.in = 0;
					break;
				case 5:
					pkt_type = HCI_ACLDATA_PKT;
					frm.in = 1;
					break;
				case 6:
					pkt_type = HCI_SCODATA_PKT;
					frm.in = 0;
					break;
				case 7:
					pkt_type = HCI_SCODATA_PKT;
					frm.in = 1;
					break;
				default:
					pkt_type = 0xff;
					break;
				}

				((uint8_t *) frm.data)[0] = pkt_type;

				frm.data_len = be32toh(dp.len) + 1;
				err = read_n(fd, frm.data + 1, frm.data_len - 1);
			}
		} else {
			frm.data_len = btohs(dh.len);
			err = read_n(fd, frm.data, frm.data_len);
		}

		if (err < 0)
			goto failed;
		if (!err)
			goto done;

		frm.ptr = frm.data;
		frm.len = frm.data_len;

		if (parser.flags & DUMP_PKTLOG) {
			uint64_t ts;
			ts = be64toh(ph.ts);
			frm.ts.tv_sec = ts >> 32;
			frm.ts.tv_usec = ts & 0xffffffff;
		} else if (parser.flags & DUMP_BTSNOOP) {
			uint64_t ts;
			frm.in = be32toh(dp.flags) & 0x01;
			ts = be64toh(dp.ts) - 0x00E03AB44A676000ll;
			frm.ts.tv_sec = (ts / 1000000ll) + 946684800ll;
			frm.ts.tv_usec = ts % 1000000ll;
		} else {
			frm.in = dh.in;
			frm.ts.tv_sec  = btohl(dh.ts_sec);
			frm.ts.tv_usec = btohl(dh.ts_usec);
		}

		parse(&frm);
	}

done:
	free(frm.data);
	return;

failed:
	perror("Read failed");
	free(frm.data);
	exit(1);
}

static int open_file(char *file, int mode, unsigned long flags)
{
	unsigned char buf[BTSNOOP_HDR_SIZE];
	struct btsnoop_hdr *hdr = (struct btsnoop_hdr *) buf;
	int fd, len, open_flags;

	if (mode == WRITE || mode == PPPDUMP || mode == AUDIO)
		open_flags = O_WRONLY | O_CREAT | O_TRUNC;
	else
		open_flags = O_RDONLY;

	fd = open(file, open_flags, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
	if (fd < 0) {
		perror("Can't open dump file");
		exit(1);
	}

	if (mode == READ) {
		len = read(fd, buf, BTSNOOP_HDR_SIZE);
		if (len != BTSNOOP_HDR_SIZE) {
			lseek(fd, 0, SEEK_SET);
			return fd;
		}

		if (!memcmp(hdr->id, btsnoop_id, sizeof(btsnoop_id))) {
			parser.flags |= DUMP_BTSNOOP;

			btsnoop_version = be32toh(hdr->version);
			btsnoop_type = be32toh(hdr->type);

			printf("btsnoop version: %d datalink type: %d\n",
						btsnoop_version, btsnoop_type);

			if (btsnoop_version != 1) {
				fprintf(stderr, "Unsupported BTSnoop version\n");
				exit(1);
			}

			if (btsnoop_type != 1001 && btsnoop_type != 1002 &&
							btsnoop_type != 2001) {
				fprintf(stderr, "Unsupported BTSnoop datalink type\n");
				exit(1);
			}
		} else {
			if (buf[0] == 0x00 && buf[1] == 0x00) {
				parser.flags |= DUMP_PKTLOG;
				printf("packet logger data format\n");
			}

			parser.flags &= ~DUMP_BTSNOOP;
			lseek(fd, 0, SEEK_SET);
			return fd;
		}
	} else {
		if (flags & DUMP_BTSNOOP) {
			btsnoop_version = 1;
			btsnoop_type = 1002;

			memcpy(hdr->id, btsnoop_id, sizeof(btsnoop_id));
			hdr->version = htobe32(btsnoop_version);
			hdr->type = htobe32(btsnoop_type);

			printf("btsnoop version: %d datalink type: %d\n",
						btsnoop_version, btsnoop_type);

			len = write(fd, buf, BTSNOOP_HDR_SIZE);
			if (len < 0) {
				perror("Can't create dump header");
				exit(1);
			}

			if (len != BTSNOOP_HDR_SIZE) {
				fprintf(stderr, "Header size mismatch\n");
				exit(1);
			}
		}
	}

	return fd;
}

static int open_socket(int dev, unsigned long flags)
{
	struct sockaddr_hci addr;
	struct hci_filter flt;
	int sk, opt;

	/* Create HCI socket */
	sk = socket(AF_BLUETOOTH, SOCK_RAW, BTPROTO_HCI);
	if (sk < 0) {
		perror("Can't create raw socket");
		return -1;
	}

	opt = 1;
	if (setsockopt(sk, SOL_HCI, HCI_DATA_DIR, &opt, sizeof(opt)) < 0) {
		perror("Can't enable data direction info");
		goto fail;
	}

	opt = 1;
	if (setsockopt(sk, SOL_HCI, HCI_TIME_STAMP, &opt, sizeof(opt)) < 0) {
		perror("Can't enable time stamp");
		goto fail;
	}

	/* Setup filter */
	hci_filter_clear(&flt);
	hci_filter_all_ptypes(&flt);
	hci_filter_all_events(&flt);
	if (setsockopt(sk, SOL_HCI, HCI_FILTER, &flt, sizeof(flt)) < 0) {
		perror("Can't set filter");
		goto fail;
	}

	/* Bind socket to the HCI device */
	memset(&addr, 0, sizeof(addr));
	addr.hci_family = AF_BLUETOOTH;
	addr.hci_dev = dev;
	if (bind(sk, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
		printf("Can't attach to device hci%d. %s(%d)\n",
					dev, strerror(errno), errno);
		goto fail;
	}

	return sk;

fail:
	close(sk);
	return -1;
}

static struct {
	char *name;
	int  flag;
} filters[] = {
	{ "lmp",	FILT_LMP	},
	{ "hci",	FILT_HCI	},
	{ "sco",	FILT_SCO	},
	{ "l2cap",	FILT_L2CAP	},
	{ "a2mp",	FILT_A2MP	},
	{ "rfcomm",	FILT_RFCOMM	},
	{ "sdp",	FILT_SDP	},
	{ "bnep",	FILT_BNEP	},
	{ "cmtp",	FILT_CMTP	},
	{ "hidp",	FILT_HIDP	},
	{ "hcrp",	FILT_HCRP	},
	{ "att",	FILT_ATT	},
	{ "smp",	FILT_SMP	},
	{ "avdtp",	FILT_AVDTP	},
	{ "avctp",	FILT_AVCTP	},
	{ "obex",	FILT_OBEX	},
	{ "capi",	FILT_CAPI	},
	{ "ppp",	FILT_PPP	},
	{ "sap",	FILT_SAP	},
	{ "csr",	FILT_CSR	},
	{ "dga",	FILT_DGA	},
	{ 0 }
};

static unsigned long parse_filter(int argc, char **argv)
{
	unsigned long filter = 0;
	int i,n;

	for (i = 0; i < argc; i++) {
		for (n = 0; filters[n].name; n++) {
			if (!strcasecmp(filters[n].name, argv[i])) {
				filter |= filters[n].flag;
				break;
			}
		}
	}

	return filter;
}

static void usage(void)
{
	printf(
	"Usage: hcidump [OPTION...] [filter]\n"
	"  -i, --device=hci_dev       HCI device\n"
	"  -l, --snap-len=len         Snap len (in bytes)\n"
	"  -p, --psm=psm              Default PSM\n"
	"  -m, --manufacturer=compid  Default manufacturer\n"
	"  -w, --save-dump=file       Save dump to a file\n"
	"  -r, --read-dump=file       Read dump from a file\n"
	"  -t, --ts                   Display time stamps\n"
	"  -a, --ascii                Dump data in ascii\n"
	"  -x, --hex                  Dump data in hex\n"
	"  -X, --ext                  Dump data in hex and ascii\n"
	"  -R, --raw                  Dump raw data\n"
	"  -C, --cmtp=psm             PSM for CMTP\n"
	"  -H, --hcrp=psm             PSM for HCRP\n"
	"  -O, --obex=port            Channel/PSM for OBEX\n"
	"  -P, --ppp=channel          Channel for PPP\n"
	"  -S, --sap=channel          Channel for SAP\n"
	"  -D, --pppdump=file         Extract PPP traffic\n"
	"  -A, --audio=file           Extract SCO audio data\n"
	"  -Y, --novendor             No vendor commands or events\n"
	"  -h, --help                 Give this help list\n"
	"  -v, --version              Give version information\n"
	"      --usage                Give a short usage message\n"
	);
}

static struct option main_options[] = {
	{ "device",		1, 0, 'i' },
	{ "snap-len",		1, 0, 'l' },
	{ "psm",		1, 0, 'p' },
	{ "manufacturer",	1, 0, 'm' },
	{ "save-dump",		1, 0, 'w' },
	{ "read-dump",		1, 0, 'r' },
	{ "timestamp",		0, 0, 't' },
	{ "ascii",		0, 0, 'a' },
	{ "hex",		0, 0, 'x' },
	{ "ext",		0, 0, 'X' },
	{ "raw",		0, 0, 'R' },
	{ "cmtp",		1, 0, 'C' },
	{ "hcrp",		1, 0, 'H' },
	{ "obex",		1, 0, 'O' },
	{ "ppp",		1, 0, 'P' },
	{ "sap",		1, 0, 'S' },
	{ "pppdump",		1, 0, 'D' },
	{ "audio",		1, 0, 'A' },
	{ "novendor",		0, 0, 'Y' },
	{ "help",		0, 0, 'h' },
	{ "version",		0, 0, 'v' },
	{ 0 }
};

int main(int argc, char *argv[])
{
	unsigned long flags = 0;
	unsigned long filter = 0;
	int device = 0;
	int defpsm = 0;
	int defcompid = DEFAULT_COMPID;
	int opt, pppdump_fd = -1, audio_fd = -1;
	uint16_t obex_port;

	while ((opt = getopt_long(argc, argv,
				"i:l:p:m:w:r:taxXRC:H:O:P:S:D:A:Yhv",
				main_options, NULL)) != -1) {
		switch(opt) {
		case 'i':
			if (strcasecmp(optarg, "none") && strcasecmp(optarg, "system"))
				device = atoi(optarg + 3);
			else
				device = HCI_DEV_NONE;
			break;

		case 'l':
			snap_len = atoi(optarg);
			break;

		case 'p':
			defpsm = atoi(optarg);
			break;

		case 'm':
			defcompid = atoi(optarg);
			break;

		case 'w':
			mode = WRITE;
			dump_file = strdup(optarg);
			break;

		case 'r':
			mode = READ;
			dump_file = strdup(optarg);
			break;

		case 't':
			flags |= DUMP_TSTAMP;
			break;

		case 'a':
			flags |= DUMP_ASCII;
			break;

		case 'x':
			flags |= DUMP_HEX;
			break;

		case 'X':
			flags |= DUMP_EXT;
			break;

		case 'R':
			flags |= DUMP_RAW;
			break;

		case 'C':
			set_proto(0, atoi(optarg), 0, SDP_UUID_CMTP);
			break;

		case 'H':
			set_proto(0, atoi(optarg), 0, SDP_UUID_HARDCOPY_CONTROL_CHANNEL);
			break;

		case 'O':
			obex_port = atoi(optarg);
			if (obex_port > 31)
				set_proto(0, obex_port, 0, SDP_UUID_OBEX);
			else
				set_proto(0, 0, obex_port, SDP_UUID_OBEX);
			break;

		case 'P':
			set_proto(0, 0, atoi(optarg), SDP_UUID_LAN_ACCESS_PPP);
			break;

		case 'S':
			set_proto(0, 0, atoi(optarg), SDP_UUID_SIM_ACCESS);
			break;

		case 'D':
			pppdump_file = strdup(optarg);
			break;

		case 'A':
			audio_file = strdup(optarg);
			break;

		case 'Y':
			flags |= DUMP_NOVENDOR;
			break;

		case 'v':
			printf("%s\n", VERSION);
			exit(0);

		case 'h':
		default:
			usage();
			exit(0);
		}
	}

	argc -= optind;
	argv += optind;
	optind = 0;

	printf("HCI sniffer - Bluetooth packet analyzer ver %s\n", VERSION);

	if (argc > 0)
		filter = parse_filter(argc, argv);

	/* Default settings */
	if (!filter)
		filter = ~0L;

	if (pppdump_file)
		pppdump_fd = open_file(pppdump_file, PPPDUMP, flags);

	if (audio_file)
		audio_fd = open_file(audio_file, AUDIO, flags);

	switch (mode) {
	case PARSE:
		flags |= DUMP_VERBOSE;
		init_parser(flags, filter, defpsm, defcompid,
							pppdump_fd, audio_fd);
		process_frames(device, open_socket(device, flags), -1, flags);
		break;

	case READ:
		flags |= DUMP_VERBOSE;
		init_parser(flags, filter, defpsm, defcompid,
							pppdump_fd, audio_fd);
		read_dump(open_file(dump_file, mode, flags));
		break;

	case WRITE:
		flags |= DUMP_BTSNOOP;
		process_frames(device, open_socket(device, flags),
				open_file(dump_file, mode, flags), flags);
		break;
	}

	return 0;
}
