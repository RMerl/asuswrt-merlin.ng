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
#include <stdbool.h>
#include <errno.h>
#include <ctype.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <getopt.h>
#include <syslog.h>
#include <signal.h>
#include <sys/time.h>
#include <poll.h>
#include <sys/ioctl.h>
#include <sys/socket.h>

#include "lib/bluetooth.h"
#include "lib/hci.h"
#include "lib/hci_lib.h"
#include "lib/l2cap.h"

#include "src/shared/util.h"

#define NIBBLE_TO_ASCII(c)  ((c) < 0x0a ? (c) + 0x30 : (c) + 0x57)

#define BREDR_DEFAULT_PSM	0x1011
#define LE_DEFAULT_PSM		0x0080

/* Test modes */
enum {
	SEND,
	RECV,
	RECONNECT,
	MULTY,
	DUMP,
	CONNECT,
	CRECV,
	LSEND,
	SENDDUMP,
	LSENDDUMP,
	LSENDRECV,
	CSENDRECV,
	INFOREQ,
	PAIRING,
};

static unsigned char *buf;

/* Default mtu */
static int imtu = 672;
static int omtu = 0;

/* Default FCS option */
static int fcs = 0x01;

/* Default Transmission Window */
static int txwin_size = 63;

/* Default Max Transmission */
static int max_transmit = 3;

/* Default data size */
static long data_size = -1;
static long buffer_size = 2048;

/* Default addr and psm and cid */
static bdaddr_t bdaddr;
static unsigned short psm = 0;
static unsigned short cid = 0;

/* Default number of frames to send (-1 = infinite) */
static int num_frames = -1;

/* Default number of consecutive frames before the delay */
static int count = 1;

/* Default delay after sending count number of frames */
static unsigned long send_delay = 0;

/* Default delay before receiving */
static unsigned long recv_delay = 0;

/* Default delay before disconnecting */
static unsigned long disc_delay = 0;

/* Initial sequence value when sending frames */
static int seq_start = 0;

static const char *filename = NULL;

static int rfcmode = 0;
static int master = 0;
static int auth = 0;
static int encr = 0;
static int secure = 0;
static int socktype = SOCK_SEQPACKET;
static int linger = 0;
static int reliable = 0;
static int timestamp = 0;
static int defer_setup = 0;
static int priority = -1;
static int rcvbuf = 0;
static int chan_policy = -1;
static int bdaddr_type = 0;

struct lookup_table {
	const char *name;
	int flag;
};

static struct lookup_table l2cap_modes[] = {
	{ "basic",	L2CAP_MODE_BASIC	},
	/* Not implemented
	{ "flowctl",	L2CAP_MODE_FLOWCTL	},
	{ "retrans",	L2CAP_MODE_RETRANS	},
	*/
	{ "ertm",	L2CAP_MODE_ERTM		},
	{ "streaming",	L2CAP_MODE_STREAMING	},
	{ 0 }
};

static struct lookup_table chan_policies[] = {
	{ "bredr",	BT_CHANNEL_POLICY_BREDR_ONLY		},
	{ "bredr_pref",	BT_CHANNEL_POLICY_BREDR_PREFERRED	},
	{ "amp_pref",	BT_CHANNEL_POLICY_AMP_PREFERRED		},
	{ NULL,		0					},
};

static struct lookup_table bdaddr_types[] = {
	{ "bredr",	BDADDR_BREDR		},
	{ "le_public",	BDADDR_LE_PUBLIC	},
	{ "le_random",	BDADDR_LE_RANDOM	},
	{ NULL,		0			},
};

static int get_lookup_flag(struct lookup_table *table, char *name)
{
	int i;

	for (i = 0; table[i].name; i++)
		if (!strcasecmp(table[i].name, name))
			return table[i].flag;

	return -1;
}

static const char *get_lookup_str(struct lookup_table *table, int flag)
{
	int i;

	for (i = 0; table[i].name; i++)
		if (table[i].flag == flag)
			return table[i].name;

	return NULL;
}

static void print_lookup_values(struct lookup_table *table, char *header)
{
	int i;

	printf("%s\n", header);

	for (i = 0; table[i].name; i++)
		printf("\t%s\n", table[i].name);
}

static float tv2fl(struct timeval tv)
{
	return (float)tv.tv_sec + (float)(tv.tv_usec/1000000.0);
}

static char *ltoh(unsigned long c, char *s)
{
	int c1;

	c1     = (c >> 28) & 0x0f;
	*(s++) = NIBBLE_TO_ASCII (c1);
	c1     = (c >> 24) & 0x0f;
	*(s++) = NIBBLE_TO_ASCII (c1);
	c1     = (c >> 20) & 0x0f;
	*(s++) = NIBBLE_TO_ASCII (c1);
	c1     = (c >> 16) & 0x0f;
	*(s++) = NIBBLE_TO_ASCII (c1);
	c1     = (c >> 12) & 0x0f;
	*(s++) = NIBBLE_TO_ASCII (c1);
	c1     = (c >>  8) & 0x0f;
	*(s++) = NIBBLE_TO_ASCII (c1);
	c1     = (c >>  4) & 0x0f;
	*(s++) = NIBBLE_TO_ASCII (c1);
	c1     = c & 0x0f;
	*(s++) = NIBBLE_TO_ASCII (c1);
	*s     = 0;
	return s;
}

static char *ctoh(char c, char *s)
{
	char c1;

	c1     = (c >> 4) & 0x0f;
	*(s++) = NIBBLE_TO_ASCII (c1);
	c1     = c & 0x0f;
	*(s++) = NIBBLE_TO_ASCII (c1);
	*s     = 0;
	return s;
}

static void hexdump(unsigned char *s, unsigned long l)
{
	char bfr[80];
	char *pb;
	unsigned long i, n = 0;

	if (l == 0)
		return;

	while (n < l) {
		pb = bfr;
		pb = ltoh (n, pb);
		*(pb++) = ':';
		*(pb++) = ' ';
		for (i = 0; i < 16; i++) {
			if (n + i >= l) {
				*(pb++) = ' ';
				*(pb++) = ' ';
			} else
				pb = ctoh (*(s + i), pb);
			*(pb++) = ' ';
		}
		*(pb++) = ' ';
		for (i = 0; i < 16; i++) {
			if (n + i >= l)
				break;
			else
				*(pb++) = (isprint (*(s + i)) ? *(s + i) : '.');
		}
		*pb = 0;
		n += 16;
		s += 16;
		puts(bfr);
	}
}

static int getopts(int sk, struct l2cap_options *opts, bool connected)
{
	socklen_t optlen;
	int err;

	memset(opts, 0, sizeof(*opts));

	if (bdaddr_type == BDADDR_BREDR) {
		optlen = sizeof(*opts);
		return getsockopt(sk, SOL_L2CAP, L2CAP_OPTIONS, opts, &optlen);
	}

	optlen = sizeof(opts->imtu);
	err = getsockopt(sk, SOL_BLUETOOTH, BT_RCVMTU, &opts->imtu, &optlen);
	if (err < 0 || !connected)
		return err;

	optlen = sizeof(opts->omtu);
	return getsockopt(sk, SOL_BLUETOOTH, BT_SNDMTU, &opts->omtu, &optlen);
}

static int setopts(int sk, struct l2cap_options *opts)
{
	if (bdaddr_type == BDADDR_BREDR)
		return setsockopt(sk, SOL_L2CAP, L2CAP_OPTIONS, opts,
								sizeof(*opts));

	return setsockopt(sk, SOL_BLUETOOTH, BT_RCVMTU, &opts->imtu,
							sizeof(opts->imtu));
}

static int do_connect(char *svr)
{
	struct sockaddr_l2 addr;
	struct l2cap_options opts;
	struct l2cap_conninfo conn;
	socklen_t optlen;
	int sk, opt;
	char ba[18];

	/* Create socket */
	sk = socket(PF_BLUETOOTH, socktype, BTPROTO_L2CAP);
	if (sk < 0) {
		syslog(LOG_ERR, "Can't create socket: %s (%d)",
							strerror(errno), errno);
		return -1;
	}

	/* Bind to local address */
	memset(&addr, 0, sizeof(addr));
	addr.l2_family = AF_BLUETOOTH;
	bacpy(&addr.l2_bdaddr, &bdaddr);
	addr.l2_bdaddr_type = bdaddr_type;
	if (cid)
		addr.l2_cid = htobs(cid);

	if (bind(sk, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
		syslog(LOG_ERR, "Can't bind socket: %s (%d)",
							strerror(errno), errno);
		goto error;
	}

	/* Get default options */
	if (getopts(sk, &opts, false) < 0) {
		syslog(LOG_ERR, "Can't get default L2CAP options: %s (%d)",
						strerror(errno), errno);
		goto error;
	}

	/* Set new options */
	opts.omtu = omtu;
	opts.imtu = imtu;
	opts.mode = rfcmode;

	opts.fcs = fcs;
	opts.txwin_size = txwin_size;
	opts.max_tx = max_transmit;

	if (setopts(sk, &opts) < 0) {
		syslog(LOG_ERR, "Can't set L2CAP options: %s (%d)",
							strerror(errno), errno);
		goto error;
	}

#if 0
	/* Enable SO_TIMESTAMP */
	if (timestamp) {
		int t = 1;

		if (setsockopt(sk, SOL_SOCKET, SO_TIMESTAMP, &t, sizeof(t)) < 0) {
			syslog(LOG_ERR, "Can't enable SO_TIMESTAMP: %s (%d)",
							strerror(errno), errno);
			goto error;
		}
	}
#endif

	if (chan_policy != -1) {
		if (setsockopt(sk, SOL_BLUETOOTH, BT_CHANNEL_POLICY,
				&chan_policy, sizeof(chan_policy)) < 0) {
			syslog(LOG_ERR, "Can't enable chan policy : %s (%d)",
							strerror(errno), errno);
			goto error;
		}
	}

	/* Enable SO_LINGER */
	if (linger) {
		struct linger l = { .l_onoff = 1, .l_linger = linger };

		if (setsockopt(sk, SOL_SOCKET, SO_LINGER, &l, sizeof(l)) < 0) {
			syslog(LOG_ERR, "Can't enable SO_LINGER: %s (%d)",
							strerror(errno), errno);
			goto error;
		}
	}

	/* Set link mode */
	opt = 0;
	if (reliable)
		opt |= L2CAP_LM_RELIABLE;
	if (master)
		opt |= L2CAP_LM_MASTER;
	if (auth)
		opt |= L2CAP_LM_AUTH;
	if (encr)
		opt |= L2CAP_LM_ENCRYPT;
	if (secure)
		opt |= L2CAP_LM_SECURE;

	if (setsockopt(sk, SOL_L2CAP, L2CAP_LM, &opt, sizeof(opt)) < 0) {
		syslog(LOG_ERR, "Can't set L2CAP link mode: %s (%d)",
							strerror(errno), errno);
		goto error;
	}

	/* Set receive buffer size */
	if (rcvbuf && setsockopt(sk, SOL_SOCKET, SO_RCVBUF,
						&rcvbuf, sizeof(rcvbuf)) < 0) {
		syslog(LOG_ERR, "Can't set socket rcv buf size: %s (%d)",
							strerror(errno), errno);
		goto error;
	}

	optlen = sizeof(rcvbuf);
	if (getsockopt(sk, SOL_SOCKET, SO_RCVBUF, &rcvbuf, &optlen) < 0) {
		syslog(LOG_ERR, "Can't get socket rcv buf size: %s (%d)",
							strerror(errno), errno);
		goto error;
	}

	/* Connect to remote device */
	memset(&addr, 0, sizeof(addr));
	addr.l2_family = AF_BLUETOOTH;
	str2ba(svr, &addr.l2_bdaddr);
	addr.l2_bdaddr_type = bdaddr_type;
	if (cid)
		addr.l2_cid = htobs(cid);
	else if (psm)
		addr.l2_psm = htobs(psm);
	else
		goto error;

	if (connect(sk, (struct sockaddr *) &addr, sizeof(addr)) < 0 ) {
		syslog(LOG_ERR, "Can't connect: %s (%d)",
							strerror(errno), errno);
		goto error;
	}

	/* Get current options */
	if (getopts(sk, &opts, true) < 0) {
		syslog(LOG_ERR, "Can't get L2CAP options: %s (%d)",
							strerror(errno), errno);
		goto error;
	}

	/* Get connection information */
	memset(&conn, 0, sizeof(conn));
	optlen = sizeof(conn);

	if (getsockopt(sk, SOL_L2CAP, L2CAP_CONNINFO, &conn, &optlen) < 0) {
		syslog(LOG_ERR, "Can't get L2CAP connection information: %s (%d)",
							strerror(errno), errno);
		goto error;
	}

	if (priority > 0 && setsockopt(sk, SOL_SOCKET, SO_PRIORITY, &priority,
						sizeof(priority)) < 0) {
		syslog(LOG_ERR, "Can't set socket priority: %s (%d)",
							strerror(errno), errno);
		goto error;
	}

	if (getsockopt(sk, SOL_SOCKET, SO_PRIORITY, &opt, &optlen) < 0) {
		syslog(LOG_ERR, "Can't get socket priority: %s (%d)",
							strerror(errno), errno);
		goto error;
	}

	/* Check for remote address */
	memset(&addr, 0, sizeof(addr));
	optlen = sizeof(addr);

	if (getpeername(sk, (struct sockaddr *) &addr, &optlen) < 0) {
		syslog(LOG_ERR, "Can't get socket name: %s (%d)",
							strerror(errno), errno);
		goto error;
	}

	ba2str(&addr.l2_bdaddr, ba);
	syslog(LOG_INFO, "Connected to %s (%s, psm %d, scid %d)", ba,
		get_lookup_str(bdaddr_types, addr.l2_bdaddr_type),
		addr.l2_psm, addr.l2_cid);

	/* Check for socket address */
	memset(&addr, 0, sizeof(addr));
	optlen = sizeof(addr);

	if (getsockname(sk, (struct sockaddr *) &addr, &optlen) < 0) {
		syslog(LOG_ERR, "Can't get socket name: %s (%d)",
							strerror(errno), errno);
		goto error;
	}

	ba2str(&addr.l2_bdaddr, ba);
	syslog(LOG_INFO, "Local device %s (%s, psm %d, scid %d)", ba,
		get_lookup_str(bdaddr_types, addr.l2_bdaddr_type),
		addr.l2_psm, addr.l2_cid);

	syslog(LOG_INFO, "Options [imtu %d, omtu %d, flush_to %d, "
		"mode %d, handle %d, class 0x%02x%02x%02x, priority %d, rcvbuf %d]",
		opts.imtu, opts.omtu, opts.flush_to, opts.mode, conn.hci_handle,
		conn.dev_class[2], conn.dev_class[1], conn.dev_class[0], opt,
		rcvbuf);

	omtu = (opts.omtu > buffer_size) ? buffer_size : opts.omtu;
	imtu = (opts.imtu > buffer_size) ? buffer_size : opts.imtu;

	return sk;

error:
	close(sk);
	return -1;
}

static void do_listen(void (*handler)(int sk))
{
	struct sockaddr_l2 addr;
	struct l2cap_options opts;
	struct l2cap_conninfo conn;
	socklen_t optlen;
	int sk, nsk, opt;
	char ba[18];

	/* Create socket */
	sk = socket(PF_BLUETOOTH, socktype, BTPROTO_L2CAP);
	if (sk < 0) {
		syslog(LOG_ERR, "Can't create socket: %s (%d)",
							strerror(errno), errno);
		exit(1);
	}

	/* Bind to local address */
	memset(&addr, 0, sizeof(addr));
	addr.l2_family = AF_BLUETOOTH;
	bacpy(&addr.l2_bdaddr, &bdaddr);
	addr.l2_bdaddr_type = bdaddr_type;
	if (cid)
		addr.l2_cid = htobs(cid);
	else if (psm)
		addr.l2_psm = htobs(psm);

	if (bind(sk, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
		syslog(LOG_ERR, "Can't bind socket: %s (%d)",
							strerror(errno), errno);
		goto error;
	}

	/* Set link mode */
	opt = 0;
	if (reliable)
		opt |= L2CAP_LM_RELIABLE;
	if (master)
		opt |= L2CAP_LM_MASTER;
	if (auth)
		opt |= L2CAP_LM_AUTH;
	if (encr)
		opt |= L2CAP_LM_ENCRYPT;
	if (secure)
		opt |= L2CAP_LM_SECURE;

	if (opt && setsockopt(sk, SOL_L2CAP, L2CAP_LM, &opt, sizeof(opt)) < 0) {
		syslog(LOG_ERR, "Can't set L2CAP link mode: %s (%d)",
							strerror(errno), errno);
		goto error;
	}

	/* Get default options */
	if (getopts(sk, &opts, false) < 0) {
		syslog(LOG_ERR, "Can't get default L2CAP options: %s (%d)",
							strerror(errno), errno);
		goto error;
	}

	/* Set new options */
	opts.omtu = omtu;
	opts.imtu = imtu;
	if (rfcmode > 0)
		opts.mode = rfcmode;

	opts.fcs = fcs;
	opts.txwin_size = txwin_size;
	opts.max_tx = max_transmit;

	if (setopts(sk, &opts) < 0) {
		syslog(LOG_ERR, "Can't set L2CAP options: %s (%d)",
							strerror(errno), errno);
		goto error;
	}

	if (socktype == SOCK_DGRAM) {
		handler(sk);
		close(sk);
		return;
	}

	/* Enable deferred setup */
	opt = defer_setup;

	if (opt && setsockopt(sk, SOL_BLUETOOTH, BT_DEFER_SETUP,
						&opt, sizeof(opt)) < 0) {
		syslog(LOG_ERR, "Can't enable deferred setup : %s (%d)",
							strerror(errno), errno);
		goto error;
	}


	/* Listen for connections */
	if (listen(sk, 10)) {
		syslog(LOG_ERR, "Can not listen on the socket: %s (%d)",
							strerror(errno), errno);
		goto error;
	}

	/* Check for socket address */
	memset(&addr, 0, sizeof(addr));
	optlen = sizeof(addr);

	if (getsockname(sk, (struct sockaddr *) &addr, &optlen) < 0) {
		syslog(LOG_ERR, "Can't get socket name: %s (%d)",
							strerror(errno), errno);
		goto error;
	}

	psm = btohs(addr.l2_psm);
	cid = btohs(addr.l2_cid);

	syslog(LOG_INFO, "Waiting for connection on psm %d ...", psm);

	while (1) {
		memset(&addr, 0, sizeof(addr));
		optlen = sizeof(addr);

		nsk = accept(sk, (struct sockaddr *) &addr, &optlen);
		if (nsk < 0) {
			syslog(LOG_ERR, "Accept failed: %s (%d)",
							strerror(errno), errno);
			goto error;
		}
		if (fork()) {
			/* Parent */
			close(nsk);
			continue;
		}
		/* Child */

		/* Set receive buffer size */
		if (rcvbuf && setsockopt(nsk, SOL_SOCKET, SO_RCVBUF, &rcvbuf,
							sizeof(rcvbuf)) < 0) {
			syslog(LOG_ERR, "Can't set rcv buf size: %s (%d)",
							strerror(errno), errno);
			goto error;
		}

		optlen = sizeof(rcvbuf);
		if (getsockopt(nsk, SOL_SOCKET, SO_RCVBUF, &rcvbuf, &optlen)
									< 0) {
			syslog(LOG_ERR, "Can't get rcv buf size: %s (%d)",
							strerror(errno), errno);
			goto error;
		}

		/* Get current options */
		if (getopts(nsk, &opts, true) < 0) {
			syslog(LOG_ERR, "Can't get L2CAP options: %s (%d)",
							strerror(errno), errno);
			if (!defer_setup) {
				close(nsk);
				goto error;
			}
		}

		/* Get connection information */
		memset(&conn, 0, sizeof(conn));
		optlen = sizeof(conn);

		if (getsockopt(nsk, SOL_L2CAP, L2CAP_CONNINFO, &conn, &optlen) < 0) {
			syslog(LOG_ERR, "Can't get L2CAP connection information: %s (%d)",
							strerror(errno), errno);
			if (!defer_setup) {
				close(nsk);
				goto error;
			}
		}

		if (priority > 0 && setsockopt(nsk, SOL_SOCKET, SO_PRIORITY,
					&priority, sizeof(priority)) < 0) {
			syslog(LOG_ERR, "Can't set socket priority: %s (%d)",
						strerror(errno), errno);
			close(nsk);
			goto error;
		}

		optlen = sizeof(priority);
		if (getsockopt(nsk, SOL_SOCKET, SO_PRIORITY, &opt, &optlen) < 0) {
			syslog(LOG_ERR, "Can't get socket priority: %s (%d)",
							strerror(errno), errno);
			goto error;
		}

		ba2str(&addr.l2_bdaddr, ba);
		syslog(LOG_INFO, "Connect from %s (%s, psm %d, dcid %d)", ba,
				get_lookup_str(bdaddr_types, addr.l2_bdaddr_type),
				addr.l2_psm, addr.l2_cid);

		/* Check for socket address */
		memset(&addr, 0, sizeof(addr));
		optlen = sizeof(addr);

		if (getsockname(nsk, (struct sockaddr *) &addr, &optlen) < 0) {
			syslog(LOG_ERR, "Can't get socket name: %s (%d)",
							strerror(errno), errno);
			goto error;
		}

		ba2str(&addr.l2_bdaddr, ba);
		syslog(LOG_INFO, "Local device %s (%s, psm %d, scid %d)", ba,
				get_lookup_str(bdaddr_types, addr.l2_bdaddr_type),
				addr.l2_psm, addr.l2_cid);

		syslog(LOG_INFO, "Options [imtu %d, omtu %d, "
				"flush_to %d, mode %d, handle %d, "
				"class 0x%02x%02x%02x, priority %d, rcvbuf %d]",
				opts.imtu, opts.omtu, opts.flush_to,
				opts.mode, conn.hci_handle, conn.dev_class[2],
				conn.dev_class[1], conn.dev_class[0], opt,
				rcvbuf);

		omtu = (opts.omtu > buffer_size) ? buffer_size : opts.omtu;
		imtu = (opts.imtu > buffer_size) ? buffer_size : opts.imtu;

#if 0
		/* Enable SO_TIMESTAMP */
		if (timestamp) {
			int t = 1;

			if (setsockopt(nsk, SOL_SOCKET, SO_TIMESTAMP, &t, sizeof(t)) < 0) {
				syslog(LOG_ERR, "Can't enable SO_TIMESTAMP: %s (%d)",
							strerror(errno), errno);
				goto error;
			}
		}
#endif

		/* Enable SO_LINGER */
		if (linger) {
			struct linger l = { .l_onoff = 1, .l_linger = linger };

			if (setsockopt(nsk, SOL_SOCKET, SO_LINGER, &l, sizeof(l)) < 0) {
				syslog(LOG_ERR, "Can't enable SO_LINGER: %s (%d)",
							strerror(errno), errno);
				close(nsk);
				goto error;
			}
		}

		/* Handle deferred setup */
		if (defer_setup) {
			syslog(LOG_INFO, "Waiting for %d seconds",
							abs(defer_setup) - 1);
			sleep(abs(defer_setup) - 1);

			if (defer_setup < 0) {
				close(nsk);
				goto error;
			}
		}

		handler(nsk);
		close(sk);

		syslog(LOG_INFO, "Disconnect: %m");
		exit(0);
	}

error:
	close(sk);
	exit(1);
}

static void dump_mode(int sk)
{
	socklen_t optlen;
	int opt, len;

	if (data_size < 0)
		data_size = imtu;

	if (defer_setup) {
		len = read(sk, buf, data_size);
		if (len < 0)
			syslog(LOG_ERR, "Initial read error: %s (%d)",
						strerror(errno), errno);
		else
			syslog(LOG_INFO, "Initial bytes %d", len);
	}

	syslog(LOG_INFO, "Receiving ...");
	while (1) {
		fd_set rset;

		FD_ZERO(&rset);
		FD_SET(sk, &rset);

		if (select(sk + 1, &rset, NULL, NULL, NULL) < 0)
			return;

		if (!FD_ISSET(sk, &rset))
			continue;

		len = read(sk, buf, data_size);
		if (len <= 0) {
			if (len < 0) {
				if (reliable && (errno == ECOMM)) {
					syslog(LOG_INFO, "L2CAP Error ECOMM - clearing error and continuing.");
					optlen = sizeof(opt);
					if (getsockopt(sk, SOL_SOCKET, SO_ERROR, &opt, &optlen) < 0) {
						syslog(LOG_ERR, "Couldn't getsockopt(SO_ERROR): %s (%d)",
							strerror(errno), errno);
						return;
					}
					continue;
				} else {
					syslog(LOG_ERR, "Read error: %s(%d)",
							strerror(errno), errno);
				}
			}
			return;
		}

		syslog(LOG_INFO, "Recevied %d bytes", len);
		hexdump(buf, len);
	}
}

static void recv_mode(int sk)
{
	struct timeval tv_beg, tv_end, tv_diff;
	struct pollfd p;
	char ts[30];
	long total;
	uint32_t seq;
	socklen_t optlen;
	int opt, len;

	if (data_size < 0)
		data_size = imtu;

	if (defer_setup) {
		len = read(sk, buf, data_size);
		if (len < 0)
			syslog(LOG_ERR, "Initial read error: %s (%d)",
						strerror(errno), errno);
		else
			syslog(LOG_INFO, "Initial bytes %d", len);
	}

	if (recv_delay)
		usleep(recv_delay);

	syslog(LOG_INFO, "Receiving ...");

	memset(ts, 0, sizeof(ts));

	p.fd = sk;
	p.events = POLLIN | POLLERR | POLLHUP;

	seq = 0;
	while (1) {
		gettimeofday(&tv_beg, NULL);
		total = 0;
		while (total < data_size) {
			uint32_t sq;
			uint16_t l;
			int i;

			p.revents = 0;
			if (poll(&p, 1, -1) <= 0)
				return;

			if (p.revents & (POLLERR | POLLHUP))
				return;

			len = recv(sk, buf, data_size, 0);
			if (len < 0) {
				if (reliable && (errno == ECOMM)) {
					syslog(LOG_INFO, "L2CAP Error ECOMM - clearing error and continuing.\n");
					optlen = sizeof(opt);
					if (getsockopt(sk, SOL_SOCKET, SO_ERROR, &opt, &optlen) < 0) {
						syslog(LOG_ERR, "Couldn't getsockopt(SO_ERROR): %s (%d)",
							strerror(errno), errno);
						return;
					}
					continue;
				} else {
					syslog(LOG_ERR, "Read failed: %s (%d)",
						strerror(errno), errno);
				}
			}

			if (len < 6)
				break;

			if (timestamp) {
				struct timeval tv;

				if (ioctl(sk, SIOCGSTAMP, &tv) < 0) {
					timestamp = 0;
					memset(ts, 0, sizeof(ts));
				} else {
					sprintf(ts, "[%ld.%ld] ",
							tv.tv_sec, tv.tv_usec);
				}
			}

			/* Check sequence */
			sq = get_le32(buf);
			if (seq != sq) {
				syslog(LOG_INFO, "seq missmatch: %d -> %d", seq, sq);
				seq = sq;
			}
			seq++;

			/* Check length */
			l = get_le16(buf + 4);
			if (len != l) {
				syslog(LOG_INFO, "size missmatch: %d -> %d", len, l);
				continue;
			}

			/* Verify data */
			for (i = 6; i < len; i++) {
				if (buf[i] != 0x7f)
					syslog(LOG_INFO, "data missmatch: byte %d 0x%2.2x", i, buf[i]);
			}

			total += len;
		}
		gettimeofday(&tv_end, NULL);

		timersub(&tv_end, &tv_beg, &tv_diff);

		syslog(LOG_INFO,"%s%ld bytes in %.2f sec, %.2f kB/s", ts, total,
			tv2fl(tv_diff), (float)(total / tv2fl(tv_diff) ) / 1024.0);
	}
}

static void do_send(int sk)
{
	uint32_t seq;
	int i, fd, len, buflen, size, sent;

	syslog(LOG_INFO, "Sending ...");

	if (data_size < 0)
		data_size = omtu;

	if (filename) {
		fd = open(filename, O_RDONLY);
		if (fd < 0) {
			syslog(LOG_ERR, "Open failed: %s (%d)",
							strerror(errno), errno);
			exit(1);
		}

		sent = 0;
		size = read(fd, buf, data_size);
		while (size > 0) {
			buflen = (size > omtu) ? omtu : size;

			len = send(sk, buf + sent, buflen, 0);

			sent += len;
			size -= len;
		}

		close(fd);
		return;
	} else {
		for (i = 6; i < data_size; i++)
			buf[i] = 0x7f;
	}

	if (!count && send_delay)
		usleep(send_delay);

	seq = seq_start;
	while ((num_frames == -1) || (num_frames-- > 0)) {
		put_le32(seq, buf);
		put_le16(data_size, buf + 4);

		seq++;

		sent = 0;
		size = data_size;
		while (size > 0) {
			buflen = (size > omtu) ? omtu : size;

			len = send(sk, buf, buflen, 0);
			if (len < 0 || len != buflen) {
				syslog(LOG_ERR, "Send failed: %s (%d)",
							strerror(errno), errno);
				exit(1);
			}

			sent += len;
			size -= len;
		}

		if (num_frames && send_delay && count &&
						!(seq % (count + seq_start)))
			usleep(send_delay);
	}
}

static void send_mode(int sk)
{
	do_send(sk);

	if (disc_delay)
		usleep(disc_delay);

	syslog(LOG_INFO, "Closing channel ...");
	if (shutdown(sk, SHUT_RDWR) < 0)
		syslog(LOG_INFO, "Close failed: %m");
	else
		syslog(LOG_INFO, "Done");
}

static void senddump_mode(int sk)
{
	do_send(sk);

	dump_mode(sk);
}

static void send_and_recv_mode(int sk)
{
	int flags;

	if ((flags = fcntl(sk, F_GETFL, 0)) < 0)
		flags = 0;
	fcntl(sk, F_SETFL, flags | O_NONBLOCK);

	/* fork for duplex channel */
	if (fork())
		send_mode(sk);
	else
		recv_mode(sk);
	return;
}

static void reconnect_mode(char *svr)
{
	while (1) {
		int sk = do_connect(svr);
		close(sk);
	}
}

static void connect_mode(char *svr)
{
	struct pollfd p;
	int sk;

	if ((sk = do_connect(svr)) < 0)
		exit(1);

	p.fd = sk;
	p.events = POLLERR | POLLHUP;

	while (1) {
		p.revents = 0;
		if (poll(&p, 1, 500))
			break;
	}

	syslog(LOG_INFO, "Disconnected");

	close(sk);
}

static void multi_connect_mode(int argc, char *argv[])
{
	int i, n, sk;

	while (1) {
		for (n = 0; n < argc; n++) {
			for (i = 0; i < count; i++) {
				if (fork())
					continue;

				/* Child */
				sk = do_connect(argv[n]);
				usleep(500);
				close(sk);
				exit(0);
			}
		}
		sleep(4);
	}
}

static void info_request(char *svr)
{
	unsigned char buf[48];
	l2cap_cmd_hdr *cmd = (l2cap_cmd_hdr *) buf;
	l2cap_info_req *req = (l2cap_info_req *) (buf + L2CAP_CMD_HDR_SIZE);
	l2cap_info_rsp *rsp = (l2cap_info_rsp *) (buf + L2CAP_CMD_HDR_SIZE);
	uint16_t mtu;
	uint32_t channels, mask = 0x0000;
	struct sockaddr_l2 addr;
	int sk, err;

	sk = socket(PF_BLUETOOTH, SOCK_RAW, BTPROTO_L2CAP);
	if (sk < 0) {
		perror("Can't create socket");
		return;
	}

	memset(&addr, 0, sizeof(addr));
	addr.l2_family = AF_BLUETOOTH;
	bacpy(&addr.l2_bdaddr, &bdaddr);
	addr.l2_bdaddr_type = bdaddr_type;

	if (bind(sk, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
		perror("Can't bind socket");
		goto failed;
	}

	memset(&addr, 0, sizeof(addr));
	addr.l2_family = AF_BLUETOOTH;
	str2ba(svr, &addr.l2_bdaddr);
	addr.l2_bdaddr_type = bdaddr_type;

	if (connect(sk, (struct sockaddr *) &addr, sizeof(addr)) < 0 ) {
		perror("Can't connect socket");
		goto failed;
	}

	memset(buf, 0, sizeof(buf));
	cmd->code  = L2CAP_INFO_REQ;
	cmd->ident = 141;
	cmd->len   = htobs(2);
	req->type  = htobs(0x0001);

	if (send(sk, buf, L2CAP_CMD_HDR_SIZE + L2CAP_INFO_REQ_SIZE, 0) < 0) {
		perror("Can't send info request");
		goto failed;
	}

	err = recv(sk, buf, L2CAP_CMD_HDR_SIZE + L2CAP_INFO_RSP_SIZE + 2, 0);
	if (err < 0) {
		perror("Can't receive info response");
		goto failed;
	}

	switch (btohs(rsp->result)) {
	case 0x0000:
		memcpy(&mtu, rsp->data, sizeof(mtu));
		printf("Connectionless MTU size is %d\n", btohs(mtu));
		break;
	case 0x0001:
		printf("Connectionless MTU is not supported\n");
		break;
	}

	memset(buf, 0, sizeof(buf));
	cmd->code  = L2CAP_INFO_REQ;
	cmd->ident = 142;
	cmd->len   = htobs(2);
	req->type  = htobs(0x0002);

	if (send(sk, buf, L2CAP_CMD_HDR_SIZE + L2CAP_INFO_REQ_SIZE, 0) < 0) {
		perror("Can't send info request");
		goto failed;
	}

	err = recv(sk, buf, L2CAP_CMD_HDR_SIZE + L2CAP_INFO_RSP_SIZE + 4, 0);
	if (err < 0) {
		perror("Can't receive info response");
		goto failed;
	}

	switch (btohs(rsp->result)) {
	case 0x0000:
		memcpy(&mask, rsp->data, sizeof(mask));
		printf("Extended feature mask is 0x%04x\n", btohl(mask));
		if (mask & L2CAP_FEAT_FLOWCTL)
			printf("  Flow control mode\n");
		if (mask & L2CAP_FEAT_RETRANS)
			printf("  Retransmission mode\n");
		if (mask & L2CAP_FEAT_BIDIR_QOS)
			printf("  Bi-directional QoS\n");
		if (mask & L2CAP_FEAT_ERTM)
			printf("  Enhanced Retransmission mode\n");
		if (mask & L2CAP_FEAT_STREAMING)
			printf("  Streaming mode\n");
		if (mask & L2CAP_FEAT_FCS)
			printf("  FCS Option\n");
		if (mask & L2CAP_FEAT_EXT_FLOW)
			printf("  Extended Flow Specification\n");
		if (mask & L2CAP_FEAT_FIXED_CHAN)
			printf("  Fixed Channels\n");
		if (mask & L2CAP_FEAT_EXT_WINDOW)
			printf("  Extended Window Size\n");
		if (mask & L2CAP_FEAT_UCD)
			printf("  Unicast Connectionless Data Reception\n");
		break;
	case 0x0001:
		printf("Extended feature mask is not supported\n");
		break;
	}

	if (!(mask & 0x80))
		goto failed;

	memset(buf, 0, sizeof(buf));
	cmd->code  = L2CAP_INFO_REQ;
	cmd->ident = 143;
	cmd->len   = htobs(2);
	req->type  = htobs(0x0003);

	if (send(sk, buf, L2CAP_CMD_HDR_SIZE + L2CAP_INFO_REQ_SIZE, 0) < 0) {
		perror("Can't send info request");
		goto failed;
	}

	err = recv(sk, buf, L2CAP_CMD_HDR_SIZE + L2CAP_INFO_RSP_SIZE + 8, 0);
	if (err < 0) {
		perror("Can't receive info response");
		goto failed;
	}

	switch (btohs(rsp->result)) {
	case 0x0000:
		memcpy(&channels, rsp->data, sizeof(channels));
		printf("Fixed channels list is 0x%04x\n", btohl(channels));
		break;
	case 0x0001:
		printf("Fixed channels list is not supported\n");
		break;
	}

failed:
	close(sk);
}

static void do_pairing(char *svr)
{
	struct sockaddr_l2 addr;
	int sk, opt;

	sk = socket(PF_BLUETOOTH, SOCK_RAW, BTPROTO_L2CAP);
	if (sk < 0) {
		perror("Can't create socket");
		return;
	}

	memset(&addr, 0, sizeof(addr));
	addr.l2_family = AF_BLUETOOTH;
	bacpy(&addr.l2_bdaddr, &bdaddr);
	addr.l2_bdaddr_type = bdaddr_type;

	if (bind(sk, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
		perror("Can't bind socket");
		goto failed;
	}

	if (secure)
		opt = L2CAP_LM_SECURE;
	else
		opt = L2CAP_LM_ENCRYPT;

	if (setsockopt(sk, SOL_L2CAP, L2CAP_LM, &opt, sizeof(opt)) < 0) {
		perror("Can't set link mode");
		goto failed;
	}

	memset(&addr, 0, sizeof(addr));
	addr.l2_family = AF_BLUETOOTH;
	str2ba(svr, &addr.l2_bdaddr);
	addr.l2_bdaddr_type = bdaddr_type;

	if (connect(sk, (struct sockaddr *) &addr, sizeof(addr)) < 0 ) {
		perror("Can't connect socket");
		goto failed;
	}

	printf("Pairing successful\n");

failed:
	close(sk);
}

static void usage(void)
{
	printf("l2test - L2CAP testing\n"
		"Usage:\n");
	printf("\tl2test <mode> [options] [bdaddr]\n");
	printf("Modes:\n"
		"\t-r listen and receive\n"
		"\t-w listen and send\n"
		"\t-d listen and dump incoming data\n"
		"\t-x listen, then send, then dump incoming data\n"
		"\t-t listen, then send and receive at the same time\n"
		"\t-q connect, then send and receive at the same time\n"
		"\t-s connect and send\n"
		"\t-u connect and receive\n"
		"\t-n connect and be silent\n"
		"\t-y connect, then send, then dump incoming data\n"
		"\t-c connect, disconnect, connect, ...\n"
		"\t-m multiple connects\n"
		"\t-p trigger dedicated bonding\n"
		"\t-z information request\n");

	printf("Options:\n"
		"\t[-b bytes] [-i device] [-P psm] [-J cid]\n"
		"\t[-I imtu] [-O omtu]\n"
		"\t[-L seconds] enable SO_LINGER\n"
		"\t[-W seconds] enable deferred setup\n"
		"\t[-B filename] use data packets from file\n"
		"\t[-N num] send num frames (default = infinite)\n"
		"\t[-C num] send num frames before delay (default = 1)\n"
		"\t[-D milliseconds] delay after sending num frames (default = 0)\n"
		"\t[-K milliseconds] delay before receiving (default = 0)\n"
		"\t[-g milliseconds] delay before disconnecting (default = 0)\n"
		"\t[-X mode] l2cap mode (help for list, default = basic)\n"
		"\t[-a policy] chan policy (help for list, default = bredr)\n"
		"\t[-F fcs] use CRC16 check (default = 1)\n"
		"\t[-Q num] Max Transmit value (default = 3)\n"
		"\t[-Z size] Transmission Window size (default = 63)\n"
		"\t[-Y priority] socket priority\n"
		"\t[-H size] Maximum receive buffer size\n"
		"\t[-R] reliable mode\n"
		"\t[-G] use connectionless channel (datagram)\n"
		"\t[-U] use sock stream\n"
		"\t[-A] request authentication\n"
		"\t[-E] request encryption\n"
		"\t[-S] secure connection\n"
		"\t[-M] become master\n"
		"\t[-T] enable timestamps\n"
		"\t[-V type] address type (help for list, default = bredr)\n"
		"\t[-e seq] initial sequence value (default = 0)\n");
}

int main(int argc, char *argv[])
{
	struct sigaction sa;
	int opt, sk, mode = RECV, need_addr = 0;

	bacpy(&bdaddr, BDADDR_ANY);

	while ((opt = getopt(argc, argv, "a:b:cde:g:i:mnpqrstuwxyz"
		"AB:C:D:EF:GH:I:J:K:L:MN:O:P:Q:RSTUV:W:X:Y:Z:")) != EOF) {
		switch (opt) {
		case 'r':
			mode = RECV;
			break;

		case 's':
			mode = SEND;
			need_addr = 1;
			break;

		case 'w':
			mode = LSEND;
			break;

		case 'u':
			mode = CRECV;
			need_addr = 1;
			break;

		case 'd':
			mode = DUMP;
			break;

		case 'c':
			mode = RECONNECT;
			need_addr = 1;
			break;

		case 'n':
			mode = CONNECT;
			need_addr = 1;
			break;

		case 'm':
			mode = MULTY;
			need_addr = 1;
			break;

		case 't':
			mode = LSENDRECV;
			break;

		case 'q':
			mode = CSENDRECV;
			need_addr = 1;
			break;

		case 'x':
			mode = LSENDDUMP;
			break;

		case 'y':
			mode = SENDDUMP;
			break;

		case 'z':
			mode = INFOREQ;
			need_addr = 1;
			break;

		case 'p':
			mode = PAIRING;
			need_addr = 1;
			break;

		case 'b':
			data_size = atoi(optarg);
			break;

		case 'i':
			if (!strncasecmp(optarg, "hci", 3))
				hci_devba(atoi(optarg + 3), &bdaddr);
			else
				str2ba(optarg, &bdaddr);
			break;

		case 'P':
			psm = atoi(optarg);
			break;

		case 'I':
			imtu = atoi(optarg);
			break;

		case 'O':
			omtu = atoi(optarg);
			break;

		case 'L':
			linger = atoi(optarg);
			break;

		case 'W':
			defer_setup = atoi(optarg);
			break;

		case 'B':
			filename = optarg;
			break;

		case 'N':
			num_frames = atoi(optarg);
			break;

		case 'C':
			count = atoi(optarg);
			break;

		case 'D':
			send_delay = atoi(optarg) * 1000;
			break;

		case 'K':
			recv_delay = atoi(optarg) * 1000;
			break;

		case 'X':
			rfcmode = get_lookup_flag(l2cap_modes, optarg);

			if (rfcmode == -1) {
				print_lookup_values(l2cap_modes,
						"List L2CAP modes:");
				exit(1);
			}

			break;

		case 'a':
			chan_policy = get_lookup_flag(chan_policies, optarg);

			if (chan_policy == -1) {
				print_lookup_values(chan_policies,
						"List L2CAP chan policies:");
				exit(1);
			}

			break;

		case 'Y':
			priority = atoi(optarg);
			break;

		case 'F':
			fcs = atoi(optarg);
			break;

		case 'R':
			reliable = 1;
			break;

		case 'M':
			master = 1;
			break;

		case 'A':
			auth = 1;
			break;

		case 'E':
			encr = 1;
			break;

		case 'S':
			secure = 1;
			break;

		case 'G':
			socktype = SOCK_DGRAM;
			break;

		case 'U':
			socktype = SOCK_STREAM;
			break;

		case 'T':
			timestamp = 1;
			break;

		case 'Q':
			max_transmit = atoi(optarg);
			break;

		case 'Z':
			txwin_size = atoi(optarg);
			break;

		case 'J':
			cid = atoi(optarg);
			break;

		case 'H':
			rcvbuf = atoi(optarg);
			break;

		case 'V':
			bdaddr_type = get_lookup_flag(bdaddr_types, optarg);

			if (bdaddr_type == -1) {
				print_lookup_values(bdaddr_types,
						"List Address types:");
				exit(1);
			}

			break;

		case 'e':
			seq_start = atoi(optarg);
			break;

		case 'g':
			disc_delay = atoi(optarg) * 1000;
			break;

		default:
			usage();
			exit(1);
		}
	}

	if (!psm) {
		if (bdaddr_type == BDADDR_BREDR)
			psm = BREDR_DEFAULT_PSM;
		else
			psm = LE_DEFAULT_PSM;
	}

	if (need_addr && !(argc - optind)) {
		usage();
		exit(1);
	}

	if (data_size < 0)
		buffer_size = (omtu > imtu) ? omtu : imtu;
	else
		buffer_size = data_size;

	if (!(buf = malloc(buffer_size))) {
		perror("Can't allocate data buffer");
		exit(1);
	}

	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = SIG_IGN;
	sa.sa_flags   = SA_NOCLDSTOP;
	sigaction(SIGCHLD, &sa, NULL);

	openlog("l2test", LOG_PERROR | LOG_PID, LOG_LOCAL0);

	switch (mode) {
		case RECV:
			do_listen(recv_mode);
			break;

		case CRECV:
			sk = do_connect(argv[optind]);
			if (sk < 0)
				exit(1);
			recv_mode(sk);
			break;

		case DUMP:
			do_listen(dump_mode);
			break;

		case SEND:
			sk = do_connect(argv[optind]);
			if (sk < 0)
				exit(1);
			send_mode(sk);
			break;

		case LSEND:
			do_listen(send_mode);
			break;

		case RECONNECT:
			reconnect_mode(argv[optind]);
			break;

		case MULTY:
			multi_connect_mode(argc - optind, argv + optind);
			break;

		case CONNECT:
			connect_mode(argv[optind]);
			break;

		case SENDDUMP:
			sk = do_connect(argv[optind]);
			if (sk < 0)
				exit(1);
			senddump_mode(sk);
			break;

		case LSENDDUMP:
			do_listen(senddump_mode);
			break;

		case LSENDRECV:
			do_listen(send_and_recv_mode);
			break;

		case CSENDRECV:
			sk = do_connect(argv[optind]);
			if (sk < 0)
				exit(1);

			send_and_recv_mode(sk);
			break;

		case INFOREQ:
			info_request(argv[optind]);
			exit(0);

		case PAIRING:
			do_pairing(argv[optind]);
			exit(0);
	}

	syslog(LOG_INFO, "Exit");

	closelog();

	return 0;
}
