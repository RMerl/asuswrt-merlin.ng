/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
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
#include <getopt.h>
#include <syslog.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/socket.h>

#include "lib/bluetooth.h"
#include "lib/sco.h"

#include "src/shared/util.h"

/* Test modes */
enum {
	SEND,
	RECV,
	RECONNECT,
	MULTY,
	DUMP,
	CONNECT
};

static unsigned char *buf;

/* Default data size */
static long data_size = 672;

static bdaddr_t bdaddr;

static int defer_setup = 0;
static int voice = 0;

static float tv2fl(struct timeval tv)
{
	return (float)tv.tv_sec + (float)(tv.tv_usec/1000000.0);
}

static int do_connect(char *svr)
{
	struct sockaddr_sco addr;
	struct sco_conninfo conn;
	socklen_t optlen;
	int sk;

	/* Create socket */
	sk = socket(PF_BLUETOOTH, SOCK_SEQPACKET, BTPROTO_SCO);
	if (sk < 0) {
		syslog(LOG_ERR, "Can't create socket: %s (%d)",
							strerror(errno), errno);
		return -1;
	}

	/* Bind to local address */
	memset(&addr, 0, sizeof(addr));
	addr.sco_family = AF_BLUETOOTH;
	bacpy(&addr.sco_bdaddr, &bdaddr);

	if (bind(sk, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
		syslog(LOG_ERR, "Can't bind socket: %s (%d)",
							strerror(errno), errno);
		goto error;
	}

	if (voice) {
		struct bt_voice opts;

		/* SCO voice setting */
		memset(&opts, 0, sizeof(opts));
		opts.setting = voice;
		if (setsockopt(sk, SOL_BLUETOOTH, BT_VOICE, &opts, sizeof(opts)) < 0) {
			syslog(LOG_ERR,
				"Can't set voice socket option: %s (%d)",
				strerror(errno), errno);
			goto error;
		}
	}

	/* Connect to remote device */
	memset(&addr, 0, sizeof(addr));
	addr.sco_family = AF_BLUETOOTH;
	str2ba(svr, &addr.sco_bdaddr);

	if (connect(sk, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
		syslog(LOG_ERR, "Can't connect: %s (%d)",
							strerror(errno), errno);
		goto error;
	}

	/* Get connection information */
	memset(&conn, 0, sizeof(conn));
	optlen = sizeof(conn);

	if (getsockopt(sk, SOL_SCO, SCO_CONNINFO, &conn, &optlen) < 0) {
		syslog(LOG_ERR, "Can't get SCO connection information: %s (%d)",
							strerror(errno), errno);
		goto error;
	}

	syslog(LOG_INFO, "Connected [handle %d, class 0x%02x%02x%02x]",
		conn.hci_handle,
		conn.dev_class[2], conn.dev_class[1], conn.dev_class[0]);

	return sk;

error:
	close(sk);
	return -1;
}

static void do_listen(void (*handler)(int sk))
{
	struct sockaddr_sco addr;
	struct sco_conninfo conn;
	socklen_t optlen;
	int sk, nsk;
	char ba[18];

	/* Create socket */
	sk = socket(PF_BLUETOOTH, SOCK_SEQPACKET, BTPROTO_SCO);
	if (sk < 0) {
		syslog(LOG_ERR, "Can't create socket: %s (%d)",
							strerror(errno), errno);
		exit(1);
	}

	/* Bind to local address */
	memset(&addr, 0, sizeof(addr));
	addr.sco_family = AF_BLUETOOTH;
	bacpy(&addr.sco_bdaddr, &bdaddr);

	if (bind(sk, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
		syslog(LOG_ERR, "Can't bind socket: %s (%d)",
							strerror(errno), errno);
		goto error;
	}

	/* Enable deferred setup */
	if (defer_setup && setsockopt(sk, SOL_BLUETOOTH, BT_DEFER_SETUP,
				&defer_setup, sizeof(defer_setup)) < 0) {
		syslog(LOG_ERR, "Can't enable deferred setup : %s (%d)",
							strerror(errno), errno);
		goto error;
	}

	/* Listen for connections */
	if (listen(sk, 10)) {
		syslog(LOG_ERR,"Can not listen on the socket: %s (%d)",
							strerror(errno), errno);
		goto error;
	}

	syslog(LOG_INFO,"Waiting for connection ...");

	while (1) {
		memset(&addr, 0, sizeof(addr));
		optlen = sizeof(addr);

		nsk = accept(sk, (struct sockaddr *) &addr, &optlen);
		if (nsk < 0) {
			syslog(LOG_ERR,"Accept failed: %s (%d)",
							strerror(errno), errno);
			goto error;
		}
		if (fork()) {
			/* Parent */
			close(nsk);
			continue;
		}
		/* Child */
		close(sk);

		/* Get connection information */
		memset(&conn, 0, sizeof(conn));
		optlen = sizeof(conn);

		if (getsockopt(nsk, SOL_SCO, SCO_CONNINFO, &conn, &optlen) < 0) {
			syslog(LOG_ERR, "Can't get SCO connection information: %s (%d)",
							strerror(errno), errno);
			if (!defer_setup) {
				close(nsk);
				exit(1);
			}
		}

		ba2str(&addr.sco_bdaddr, ba);
		syslog(LOG_INFO, "Connect from %s [handle %d, class 0x%02x%02x%02x]",
			ba, conn.hci_handle,
			conn.dev_class[2], conn.dev_class[1], conn.dev_class[0]);

		/* Handle deferred setup */
		if (defer_setup) {
			syslog(LOG_INFO, "Waiting for %d seconds",
							abs(defer_setup) - 1);
			sleep(abs(defer_setup) - 1);

			if (defer_setup < 0) {
				close(nsk);
				exit(1);
			}
		}

		handler(nsk);

		syslog(LOG_INFO, "Disconnect");
		exit(0);
	}

error:
	close(sk);
	exit(1);
}

static void dump_mode(int sk)
{
	struct bt_voice opts;
	int len;

	/* SCO voice setting */
	memset(&opts, 0, sizeof(opts));
	opts.setting = voice;
	if (setsockopt(sk, SOL_BLUETOOTH, BT_VOICE, &opts, sizeof(opts)) < 0)
		syslog(LOG_ERR, "Can't set socket options: %s (%d)",
							strerror(errno), errno);

	if (defer_setup) {
		len = read(sk, buf, data_size);
		if (len < 0)
			syslog(LOG_ERR, "Initial read error: %s (%d)",
						strerror(errno), errno);
		else
			syslog(LOG_INFO, "Initial bytes %d", len);
	}

	syslog(LOG_INFO,"Receiving ...");
	while ((len = read(sk, buf, data_size)) > 0)
		syslog(LOG_INFO, "Recevied %d bytes", len);
}

static void recv_mode(int sk)
{
	struct timeval tv_beg,tv_end,tv_diff;
	struct bt_voice opts;
	long total;
	int len;

	/* SCO voice setting */
	memset(&opts, 0, sizeof(opts));
	opts.setting = voice;
	if (setsockopt(sk, SOL_BLUETOOTH, BT_VOICE, &opts, sizeof(opts)) < 0)
		syslog(LOG_ERR, "Can't set socket options: %s (%d)",
							strerror(errno), errno);

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
		gettimeofday(&tv_beg, NULL);
		total = 0;
		while (total < data_size) {
			int r;
			if ((r = recv(sk, buf, data_size, 0)) <= 0) {
				if (r < 0)
					syslog(LOG_ERR, "Read failed: %s (%d)",
							strerror(errno), errno);
				if (errno != ENOTCONN)
					return;
				r = 0;
			}
			total += r;
		}
		gettimeofday(&tv_end, NULL);

		timersub(&tv_end, &tv_beg, &tv_diff);

		syslog(LOG_INFO,"%ld bytes in %.2fm speed %.2f kb", total,
			tv2fl(tv_diff) / 60.0,
			(float)( total / tv2fl(tv_diff) ) / 1024.0 );
	}
}

static void send_mode(char *svr)
{
	struct sco_options so;
	socklen_t len;
	uint32_t seq;
	int i, sk;

	if ((sk = do_connect(svr)) < 0) {
		syslog(LOG_ERR, "Can't connect to the server: %s (%d)",
							strerror(errno), errno);
		exit(1);
	}

	len = sizeof(so);
	if (getsockopt(sk, SOL_SCO, SCO_OPTIONS, &so, &len) < 0) {
		syslog(LOG_ERR, "Can't get SCO options: %s (%d)",
							strerror(errno), errno);
		exit(1);
	}

	syslog(LOG_INFO,"Sending ...");

	for (i = 6; i < so.mtu; i++)
		buf[i] = 0x7f;

	seq = 0;
	while (1) {
		put_le32(seq, buf);
		put_le16(data_size, buf + 4);

		seq++;

		if (send(sk, buf, so.mtu, 0) <= 0) {
			syslog(LOG_ERR, "Send failed: %s (%d)",
							strerror(errno), errno);
			exit(1);
		}

		usleep(1);
	}
}

static void reconnect_mode(char *svr)
{
	while (1) {
		int sk;

		if ((sk = do_connect(svr)) < 0) {
			syslog(LOG_ERR, "Can't connect to the server: %s (%d)",
							strerror(errno), errno);
			exit(1);
		}

		close(sk);

		sleep(5);
	}
}

static void multy_connect_mode(char *svr)
{
	while (1) {
		int i, sk;

		for (i = 0; i < 10; i++){
			if (fork())
				continue;

			/* Child */
			sk = do_connect(svr);
			if (sk < 0) {
				syslog(LOG_ERR, "Can't connect to the server: %s (%d)",
							strerror(errno), errno);
			}
			close(sk);
			exit(0);
		}

		sleep(19);
	}
}

static void usage(void)
{
	printf("scotest - SCO testing\n"
		"Usage:\n");
	printf("\tscotest <mode> [options] [bd_addr]\n");
	printf("Modes:\n"
		"\t-d dump (server)\n"
		"\t-c reconnect (client)\n"
		"\t-m multiple connects (client)\n"
		"\t-r receive (server)\n"
		"\t-s connect and send (client)\n"
		"\t-n connect and be silent (client)\n"
		"Options:\n"
		"\t[-b bytes]\n"
		"\t[-W seconds] enable deferred setup\n"
		"\t[-V voice] select SCO voice setting (0x0060 cvsd, 0x0003 transparent)\n");
}

int main(int argc ,char *argv[])
{
	struct sigaction sa;
	int opt, sk, mode = RECV;

	while ((opt = getopt(argc, argv, "rdscmnb:W:V:")) != EOF) {
		switch(opt) {
		case 'r':
			mode = RECV;
			break;

		case 's':
			mode = SEND;
			break;

		case 'd':
			mode = DUMP;
			break;

		case 'c':
			mode = RECONNECT;
			break;

		case 'm':
			mode = MULTY;
			break;

		case 'n':
			mode = CONNECT;
			break;

		case 'b':
			data_size = atoi(optarg);
			break;

		case 'W':
			defer_setup = atoi(optarg);
			break;

		case 'V':
			voice = strtol(optarg, NULL, 0);
			break;

		default:
			usage();
			exit(1);
		}
	}

	if (!(argc - optind) && (mode != RECV && mode != DUMP)) {
		usage();
		exit(1);
	}

	if (!(buf = malloc(data_size))) {
		perror("Can't allocate data buffer");
		exit(1);
	}

	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = SIG_IGN;
	sa.sa_flags   = SA_NOCLDSTOP;
	sigaction(SIGCHLD, &sa, NULL);

	openlog("scotest", LOG_PERROR | LOG_PID, LOG_LOCAL0);

	switch( mode ){
		case RECV:
			do_listen(recv_mode);
			break;

		case DUMP:
			do_listen(dump_mode);
			break;

		case SEND:
			send_mode(argv[optind]);
			break;

		case RECONNECT:
			reconnect_mode(argv[optind]);
			break;

		case MULTY:
			multy_connect_mode(argv[optind]);
			break;

		case CONNECT:
			sk = do_connect(argv[optind]);
			if (sk < 0)
				exit(1);
			dump_mode(sk);
			break;
	}

	syslog(LOG_INFO, "Exit");

	closelog();

	return 0;
}
