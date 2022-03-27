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
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <signal.h>
#include <termios.h>
#include <poll.h>
#include <sys/param.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/wait.h>

#include "lib/bluetooth.h"
#include "lib/hci.h"
#include "lib/hci_lib.h"
#include "lib/rfcomm.h"

static int rfcomm_raw_tty = 0;
static int auth = 0;
static int encryption = 0;
static int secure = 0;
static int master = 0;
static int linger = 0;

static char *rfcomm_state[] = {
	"unknown",
	"connected",
	"clean",
	"bound",
	"listening",
	"connecting",
	"connecting",
	"config",
	"disconnecting",
	"closed"
};

static volatile sig_atomic_t __io_canceled = 0;

static void sig_hup(int sig)
{
	return;
}

static void sig_term(int sig)
{
	__io_canceled = 1;
}

static char *rfcomm_flagstostr(uint32_t flags)
{
	static char str[100];
	str[0] = 0;

	strcat(str, "[");

	if (flags & (1 << RFCOMM_REUSE_DLC))
		strcat(str, "reuse-dlc ");

	if (flags & (1 << RFCOMM_RELEASE_ONHUP))
		strcat(str, "release-on-hup ");

	if (flags & (1 << RFCOMM_TTY_ATTACHED))
		strcat(str, "tty-attached");

	strcat(str, "]");
	return str;
}

static void print_dev_info(struct rfcomm_dev_info *di)
{
	char src[18], dst[18], addr[40];

	ba2str(&di->src, src); ba2str(&di->dst, dst);

	if (bacmp(&di->src, BDADDR_ANY) == 0)
		sprintf(addr, "%s", dst);
	else
		sprintf(addr, "%s -> %s", src, dst);

	printf("rfcomm%d: %s channel %d %s %s\n",
		di->id, addr, di->channel,
		rfcomm_state[di->state],
		di->flags ? rfcomm_flagstostr(di->flags) : "");
}

static void print_dev_list(int ctl, int flags)
{
	struct rfcomm_dev_list_req *dl;
	struct rfcomm_dev_info *di;
	int i;

	dl = malloc(sizeof(*dl) + RFCOMM_MAX_DEV * sizeof(*di));
	if (!dl) {
		perror("Can't allocate memory");
		exit(1);
	}

	dl->dev_num = RFCOMM_MAX_DEV;
	di = dl->dev_info;

	if (ioctl(ctl, RFCOMMGETDEVLIST, (void *) dl) < 0) {
		perror("Can't get device list");
		free(dl);
		exit(1);
	}

	for (i = 0; i < dl->dev_num; i++)
		print_dev_info(di + i);
	free(dl);
}

static int create_dev(int ctl, int dev, uint32_t flags, bdaddr_t *bdaddr, int argc, char **argv)
{
	struct rfcomm_dev_req req;
	int err;

	memset(&req, 0, sizeof(req));
	req.dev_id = dev;
	req.flags = flags;
	bacpy(&req.src, bdaddr);

	if (argc < 2) {
		fprintf(stderr, "Missing dev parameter");
		return -EINVAL;
	}

	str2ba(argv[1], &req.dst);

	if (argc > 2)
		req.channel = atoi(argv[2]);
	else
		req.channel = 1;

	err = ioctl(ctl, RFCOMMCREATEDEV, &req);
	if (err == -1) {
		err = -errno;

		if (err == -EOPNOTSUPP)
			fprintf(stderr, "RFCOMM TTY support not available\n");
		else
			perror("Can't create device");
	}

	return err;
}

static int release_dev(int ctl, int dev, uint32_t flags)
{
	struct rfcomm_dev_req req;
	int err;

	memset(&req, 0, sizeof(req));
	req.dev_id = dev;

	err = ioctl(ctl, RFCOMMRELEASEDEV, &req);
	if (err < 0)
		perror("Can't release device");

	return err;
}

static int release_all(int ctl)
{
	struct rfcomm_dev_list_req *dl;
	struct rfcomm_dev_info *di;
	int i;

	dl = malloc(sizeof(*dl) + RFCOMM_MAX_DEV * sizeof(*di));
	if (!dl) {
		perror("Can't allocate memory");
		exit(1);
	}

	dl->dev_num = RFCOMM_MAX_DEV;
	di = dl->dev_info;

	if (ioctl(ctl, RFCOMMGETDEVLIST, (void *) dl) < 0) {
		perror("Can't get device list");
		free(dl);
		exit(1);
	}

	for (i = 0; i < dl->dev_num; i++)
		release_dev(ctl, (di + i)->id, 0);

	free(dl);
	return 0;
}

static void run_cmdline(struct pollfd *p, sigset_t *sigs, char *devname,
			int argc, char **argv)
{
	int i;
	pid_t pid;
	char **cmdargv;

	cmdargv = malloc((argc + 1) * sizeof(char *));
	if (!cmdargv)
		return;

	for (i = 0; i < argc; i++)
		cmdargv[i] = (strcmp(argv[i], "{}") == 0) ? devname : argv[i];
	cmdargv[i] = NULL;

	pid = fork();

	switch (pid) {
	case 0:
		i = execvp(cmdargv[0], cmdargv);
		fprintf(stderr, "Couldn't execute command %s (errno=%d:%s)\n",
				cmdargv[0], errno, strerror(errno));
		break;
	case -1:
		fprintf(stderr, "Couldn't fork to execute command %s\n",
				cmdargv[0]);
		break;
	default:
		while (1) {
			int status;
			pid_t child;
			struct timespec ts;

			child = waitpid(-1, &status, WNOHANG);
			if (child == pid || (child < 0 && errno != EAGAIN))
				break;

			p->revents = 0;
			ts.tv_sec  = 0;
			ts.tv_nsec = 200;
			if (ppoll(p, 1, &ts, sigs) || __io_canceled) {
				kill(pid, SIGTERM);
				waitpid(pid, &status, 0);
				break;
			}
		}
		break;
	}

	free(cmdargv);
}

static void cmd_connect(int ctl, int dev, bdaddr_t *bdaddr, int argc, char **argv)
{
	struct sockaddr_rc laddr, raddr;
	struct rfcomm_dev_req req;
	struct termios ti;
	struct sigaction sa;
	struct pollfd p;
	sigset_t sigs;
	socklen_t alen;
	char dst[18], devname[MAXPATHLEN];
	int sk, fd, try = 30;

	laddr.rc_family = AF_BLUETOOTH;
	bacpy(&laddr.rc_bdaddr, bdaddr);
	laddr.rc_channel = 0;

	if (argc < 2) {
		fprintf(stderr, "Missing dev parameter");
		return;
	}

	raddr.rc_family = AF_BLUETOOTH;
	str2ba(argv[1], &raddr.rc_bdaddr);

	if (argc > 2)
		raddr.rc_channel = atoi(argv[2]);
	else
		raddr.rc_channel = 1;

	sk = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);
	if (sk < 0) {
		perror("Can't create RFCOMM socket");
		return;
	}

	if (linger) {
		struct linger l = { .l_onoff = 1, .l_linger = linger };

		if (setsockopt(sk, SOL_SOCKET, SO_LINGER, &l, sizeof(l)) < 0) {
			perror("Can't set linger option");
			return;
		}
	}

	if (bind(sk, (struct sockaddr *) &laddr, sizeof(laddr)) < 0) {
		perror("Can't bind RFCOMM socket");
		close(sk);
		return;
	}

	if (connect(sk, (struct sockaddr *) &raddr, sizeof(raddr)) < 0) {
		perror("Can't connect RFCOMM socket");
		close(sk);
		return;
	}

	alen = sizeof(laddr);
	if (getsockname(sk, (struct sockaddr *)&laddr, &alen) < 0) {
		perror("Can't get RFCOMM socket name");
		close(sk);
		return;
	}

	memset(&req, 0, sizeof(req));
	req.dev_id = dev;
	req.flags = (1 << RFCOMM_REUSE_DLC) | (1 << RFCOMM_RELEASE_ONHUP);

	bacpy(&req.src, &laddr.rc_bdaddr);
	bacpy(&req.dst, &raddr.rc_bdaddr);
	req.channel = raddr.rc_channel;

	dev = ioctl(sk, RFCOMMCREATEDEV, &req);
	if (dev < 0) {
		perror("Can't create RFCOMM TTY");
		close(sk);
		return;
	}

	snprintf(devname, MAXPATHLEN - 1, "/dev/rfcomm%d", dev);
	while ((fd = open(devname, O_RDONLY | O_NOCTTY)) < 0) {
		if (errno == EACCES) {
			perror("Can't open RFCOMM device");
			goto release;
		}

		snprintf(devname, MAXPATHLEN - 1, "/dev/bluetooth/rfcomm/%d", dev);
		if ((fd = open(devname, O_RDONLY | O_NOCTTY)) < 0) {
			if (try--) {
				snprintf(devname, MAXPATHLEN - 1, "/dev/rfcomm%d", dev);
				usleep(100 * 1000);
				continue;
			}
			perror("Can't open RFCOMM device");
			goto release;
		}
	}

	if (rfcomm_raw_tty) {
		tcflush(fd, TCIOFLUSH);

		cfmakeraw(&ti);
		tcsetattr(fd, TCSANOW, &ti);
	}

	close(sk);

	ba2str(&req.dst, dst);
	printf("Connected %s to %s on channel %d\n", devname, dst, req.channel);
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

	p.fd = fd;
	p.events = POLLERR | POLLHUP;

	while (!__io_canceled) {
		p.revents = 0;
		if (ppoll(&p, 1, NULL, &sigs) > 0)
			break;
	}

	printf("Disconnected\n");

	close(fd);
	return;

release:
	memset(&req, 0, sizeof(req));
	req.dev_id = dev;
	req.flags = (1 << RFCOMM_HANGUP_NOW);
	ioctl(ctl, RFCOMMRELEASEDEV, &req);

	close(sk);
}

static void cmd_listen(int ctl, int dev, bdaddr_t *bdaddr, int argc, char **argv)
{
	struct sockaddr_rc laddr, raddr;
	struct rfcomm_dev_req req;
	struct termios ti;
	struct sigaction sa;
	struct pollfd p;
	sigset_t sigs;
	socklen_t alen;
	char dst[18], devname[MAXPATHLEN];
	int sk, nsk, fd, lm, try = 30;

	laddr.rc_family = AF_BLUETOOTH;
	bacpy(&laddr.rc_bdaddr, bdaddr);
	laddr.rc_channel = (argc < 2) ? 1 : atoi(argv[1]);

	sk = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);
	if (sk < 0) {
		perror("Can't create RFCOMM socket");
		return;
	}

	lm = 0;
	if (master)
		lm |= RFCOMM_LM_MASTER;
	if (auth)
		lm |= RFCOMM_LM_AUTH;
	if (encryption)
		lm |= RFCOMM_LM_ENCRYPT;
	if (secure)
		lm |= RFCOMM_LM_SECURE;

	if (lm && setsockopt(sk, SOL_RFCOMM, RFCOMM_LM, &lm, sizeof(lm)) < 0) {
		perror("Can't set RFCOMM link mode");
		close(sk);
		return;
	}

	if (bind(sk, (struct sockaddr *)&laddr, sizeof(laddr)) < 0) {
		perror("Can't bind RFCOMM socket");
		close(sk);
		return;
	}

	printf("Waiting for connection on channel %d\n", laddr.rc_channel);

	listen(sk, 10);

	alen = sizeof(raddr);
	nsk = accept(sk, (struct sockaddr *) &raddr, &alen);

	alen = sizeof(laddr);
	if (getsockname(nsk, (struct sockaddr *)&laddr, &alen) < 0) {
		perror("Can't get RFCOMM socket name");
		close(nsk);
		return;
	}

	if (linger) {
		struct linger l = { .l_onoff = 1, .l_linger = linger };

		if (setsockopt(nsk, SOL_SOCKET, SO_LINGER, &l, sizeof(l)) < 0) {
			perror("Can't set linger option");
			close(nsk);
			return;
		}
	}

	memset(&req, 0, sizeof(req));
	req.dev_id = dev;
	req.flags = (1 << RFCOMM_REUSE_DLC) | (1 << RFCOMM_RELEASE_ONHUP);

	bacpy(&req.src, &laddr.rc_bdaddr);
	bacpy(&req.dst, &raddr.rc_bdaddr);
	req.channel = raddr.rc_channel;

	dev = ioctl(nsk, RFCOMMCREATEDEV, &req);
	if (dev < 0) {
		perror("Can't create RFCOMM TTY");
		close(sk);
		return;
	}

	snprintf(devname, MAXPATHLEN - 1, "/dev/rfcomm%d", dev);
	while ((fd = open(devname, O_RDONLY | O_NOCTTY)) < 0) {
		if (errno == EACCES) {
			perror("Can't open RFCOMM device");
			goto release;
		}

		snprintf(devname, MAXPATHLEN - 1, "/dev/bluetooth/rfcomm/%d", dev);
		if ((fd = open(devname, O_RDONLY | O_NOCTTY)) < 0) {
			if (try--) {
				snprintf(devname, MAXPATHLEN - 1, "/dev/rfcomm%d", dev);
				usleep(100 * 1000);
				continue;
			}
			perror("Can't open RFCOMM device");
			goto release;
		}
	}

	if (rfcomm_raw_tty) {
		tcflush(fd, TCIOFLUSH);

		cfmakeraw(&ti);
		tcsetattr(fd, TCSANOW, &ti);
	}

	close(sk);
	close(nsk);

	ba2str(&req.dst, dst);
	printf("Connection from %s to %s\n", dst, devname);
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

	p.fd = fd;
	p.events = POLLERR | POLLHUP;

	if (argc <= 2) {
		while (!__io_canceled) {
			p.revents = 0;
			if (ppoll(&p, 1, NULL, &sigs) > 0)
				break;
		}
	} else
		run_cmdline(&p, &sigs, devname, argc - 2, argv + 2);

	sa.sa_handler = NULL;
	sigaction(SIGTERM, &sa, NULL);
	sigaction(SIGINT,  &sa, NULL);

	printf("Disconnected\n");

	close(fd);
	return;

release:
	memset(&req, 0, sizeof(req));
	req.dev_id = dev;
	req.flags = (1 << RFCOMM_HANGUP_NOW);
	ioctl(ctl, RFCOMMRELEASEDEV, &req);

	close(sk);
}

static void cmd_watch(int ctl, int dev, bdaddr_t *bdaddr, int argc, char **argv)
{
	while (!__io_canceled) {
		cmd_listen(ctl, dev, bdaddr, argc, argv);
		usleep(10000);
	}
}

static void cmd_create(int ctl, int dev, bdaddr_t *bdaddr, int argc, char **argv)
{
	create_dev(ctl, dev, 0, bdaddr, argc, argv);
}

static void cmd_release(int ctl, int dev, bdaddr_t *bdaddr, int argc, char **argv)
{
	if (strcmp(argv[0], "all") == 0)
		release_all(ctl);
	else
		release_dev(ctl, dev, 0);
}

static void cmd_show(int ctl, int dev, bdaddr_t *bdaddr, int argc, char **argv)
{
	if (strcmp(argv[0], "all") == 0)
		print_dev_list(ctl, 0);
	else {
		struct rfcomm_dev_info di = { .id = atoi(argv[0]) };
		if (ioctl(ctl, RFCOMMGETDEVINFO, &di) < 0) {
			perror("Get info failed");
			exit(1);
		}

		print_dev_info(&di);
	}
}

struct {
	char *cmd;
	char *alt;
	void (*func)(int ctl, int dev, bdaddr_t *bdaddr, int argc, char **argv);
	char *opt;
	char *doc;
} command[] = {
	{ "bind",    "create", cmd_create,  "<dev> <bdaddr> [channel]", "Bind device"    },
	{ "release", "unbind", cmd_release, "<dev>",                    "Release device" },
	{ "show",    "info",   cmd_show,    "<dev>",                    "Show device"    },
	{ "connect", "conn",   cmd_connect, "<dev> <bdaddr> [channel]", "Connect device" },
	{ "listen",  "server", cmd_listen,  "<dev> [channel [cmd]]",    "Listen"         },
	{ "watch",   "watch",  cmd_watch,   "<dev> [channel [cmd]]",    "Watch"          },
	{ NULL, NULL, NULL, 0, 0 }
};

static void usage(void)
{
	int i;

	printf("RFCOMM configuration utility ver %s\n", VERSION);

	printf("Usage:\n"
		"\trfcomm [options] <command> <dev>\n"
		"\n");

	printf("Options:\n"
		"\t-i, --device [hciX|bdaddr]     Local HCI device or BD Address\n"
		"\t-h, --help                     Display help\n"
		"\t-r, --raw                      Switch TTY into raw mode\n"
		"\t-A, --auth                     Enable authentication\n"
		"\t-E, --encrypt                  Enable encryption\n"
		"\t-S, --secure                   Secure connection\n"
		"\t-M, --master                   Become the master of a piconet\n"
		"\t-L, --linger [seconds]         Set linger timeout\n"
		"\t-a                             Show all devices (default)\n"
		"\n");

	printf("Commands:\n");
	for (i = 0; command[i].cmd; i++)
		printf("\t%-8s %-24s\t%s\n",
			command[i].cmd,
			command[i].opt ? command[i].opt : " ",
			command[i].doc);
	printf("\n");
}

static struct option main_options[] = {
	{ "help",	0, 0, 'h' },
	{ "device",	1, 0, 'i' },
	{ "config",	1, 0, 'f' },
	{ "raw",	0, 0, 'r' },
	{ "auth",	0, 0, 'A' },
	{ "encrypt",	0, 0, 'E' },
	{ "secure",	0, 0, 'S' },
	{ "master",	0, 0, 'M' },
	{ "linger",	1, 0, 'L' },
	{ 0, 0, 0, 0 }
};

int main(int argc, char *argv[])
{
	bdaddr_t bdaddr;
	int i, opt, ctl, dev_id, show_all = 0;

	bacpy(&bdaddr, BDADDR_ANY);

	while ((opt = getopt_long(argc, argv, "+i:rahAESML:", main_options, NULL)) != -1) {
		switch(opt) {
		case 'i':
			if (strncmp(optarg, "hci", 3) == 0)
				hci_devba(atoi(optarg + 3), &bdaddr);
			else
				str2ba(optarg, &bdaddr);
			break;

		case 'r':
			rfcomm_raw_tty = 1;
			break;

		case 'a':
			show_all = 1;
			break;

		case 'h':
			usage();
			exit(0);

		case 'A':
			auth = 1;
			break;

		case 'E':
			encryption = 1;
			break;

		case 'S':
			secure = 1;
			break;

		case 'M':
			master = 1;
			break;

		case 'L':
			linger = atoi(optarg);
			break;

		default:
			exit(0);
		}
	}

	argc -= optind;
	argv += optind;
	optind = 0;

	if (argc < 2) {
		if (argc != 0) {
			usage();
			exit(1);
		} else
			show_all = 1;
	}

	ctl = socket(AF_BLUETOOTH, SOCK_RAW, BTPROTO_RFCOMM);
	if (ctl < 0) {
		perror("Can't open RFCOMM control socket");
		exit(1);
	}

	if (show_all) {
		print_dev_list(ctl, 0);
		close(ctl);
		exit(0);
	}

	if (strncmp(argv[1], "/dev/rfcomm", 11) == 0)
		dev_id = atoi(argv[1] + 11);
	else if (strncmp(argv[1], "rfcomm", 6) == 0)
		dev_id = atoi(argv[1] + 6);
	else
		dev_id = atoi(argv[1]);

	for (i = 0; command[i].cmd; i++) {
		if (strncmp(command[i].cmd, argv[0], 4) && strncmp(command[i].alt, argv[0], 4))
			continue;
		argc--;
		argv++;
		command[i].func(ctl, dev_id, &bdaddr, argc, argv);
		close(ctl);
		exit(0);
	}

	usage();

	close(ctl);

	return 0;
}
