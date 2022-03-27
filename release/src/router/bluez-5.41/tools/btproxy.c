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
#include <ctype.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <alloca.h>
#include <getopt.h>
#include <stdbool.h>
#include <termios.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/un.h>

#include <netdb.h>
#include <arpa/inet.h>

#include "src/shared/util.h"
#include "src/shared/mainloop.h"
#include "src/shared/ecc.h"
#include "monitor/bt.h"

#define HCI_PRIMARY	0x00
#define HCI_AMP		0x01

#define BTPROTO_HCI	1
struct sockaddr_hci {
	sa_family_t	hci_family;
	unsigned short	hci_dev;
	unsigned short  hci_channel;
};
#define HCI_CHANNEL_USER	1

static uint16_t hci_index = 0;
static bool client_active = false;
static bool debug_enabled = false;
static bool emulate_ecc = false;

static void hexdump_print(const char *str, void *user_data)
{
	printf("%s%s\n", (char *) user_data, str);
}

struct proxy {
	/* Receive commands, ACL and SCO data */
	int host_fd;
	uint8_t host_buf[4096];
	uint16_t host_len;
	bool host_shutdown;

	/* Receive events, ACL and SCO data */
	int dev_fd;
	uint8_t dev_buf[4096];
	uint16_t dev_len;
	bool dev_shutdown;

	/* ECC emulation */
	uint8_t event_mask[8];
	uint8_t local_sk256[32];
};

static bool write_packet(int fd, const void *data, size_t size,
							void *user_data)
{
	while (size > 0) {
		ssize_t written;

		written = write(fd, data, size);
		if (written < 0) {
			if (errno == EAGAIN || errno == EINTR)
				continue;
			return false;
		}

		if (debug_enabled)
			util_hexdump('<', data, written, hexdump_print,
								user_data);

		data += written;
		size -= written;
	}

	return true;
}

static void host_write_packet(struct proxy *proxy, void *buf, uint16_t len)
{
	if (!write_packet(proxy->dev_fd, buf, len, "D: ")) {
		fprintf(stderr, "Write to device descriptor failed\n");
		mainloop_remove_fd(proxy->dev_fd);
	}
}

static void dev_write_packet(struct proxy *proxy, void *buf, uint16_t len)
{
	if (!write_packet(proxy->host_fd, buf, len, "H: ")) {
		fprintf(stderr, "Write to host descriptor failed\n");
		mainloop_remove_fd(proxy->host_fd);
	}
}

static void cmd_status(struct proxy *proxy, uint8_t status, uint16_t opcode)
{
	size_t buf_size = 1 + sizeof(struct bt_hci_evt_hdr) +
					sizeof(struct bt_hci_evt_cmd_status);
	void *buf = alloca(buf_size);
	struct bt_hci_evt_hdr *hdr = buf + 1;
	struct bt_hci_evt_cmd_status *cs = buf + 1 + sizeof(*hdr);

	*((uint8_t *) buf) = BT_H4_EVT_PKT;

	hdr->evt = BT_HCI_EVT_CMD_STATUS;
	hdr->plen = sizeof(*cs);

	cs->status = status;
	cs->ncmd = 0x01;
	cs->opcode = cpu_to_le16(opcode);

	dev_write_packet(proxy, buf, buf_size);
}

static void le_meta_event(struct proxy *proxy, uint8_t event,
						void *data, uint8_t len)
{
	size_t buf_size = 1 + sizeof(struct bt_hci_evt_hdr) + 1 + len;
	void *buf = alloca(buf_size);
	struct bt_hci_evt_hdr *hdr = buf + 1;

	*((uint8_t *) buf) = BT_H4_EVT_PKT;

	hdr->evt = BT_HCI_EVT_LE_META_EVENT;
	hdr->plen = 1 + len;

	*((uint8_t *) (buf + 1 + sizeof(*hdr))) = event;

	if (len > 0)
		memcpy(buf + 1 + sizeof(*hdr) + 1, data, len);

	dev_write_packet(proxy, buf, buf_size);
}

static void host_emulate_ecc(struct proxy *proxy, void *buf, uint16_t len)
{
	uint8_t pkt_type = *((uint8_t *) buf);
	struct bt_hci_cmd_hdr *hdr = buf + 1;
	struct bt_hci_cmd_le_set_event_mask *lsem;
	struct bt_hci_cmd_le_generate_dhkey *lgd;
	struct bt_hci_evt_le_read_local_pk256_complete lrlpkc;
	struct bt_hci_evt_le_generate_dhkey_complete lgdc;

	if (pkt_type != BT_H4_CMD_PKT) {
		host_write_packet(proxy, buf, len);
		return;
	}

	switch (le16_to_cpu(hdr->opcode)) {
	case BT_HCI_CMD_LE_SET_EVENT_MASK:
		lsem = buf + 1 + sizeof(*hdr);
		memcpy(proxy->event_mask, lsem->mask, 8);

		lsem->mask[0] &= ~0x80;		/* P-256 Public Key Complete */
		lsem->mask[1] &= ~0x01;		/* Generate DHKey Complete */

		host_write_packet(proxy, buf, len);
		break;

	case BT_HCI_CMD_LE_READ_LOCAL_PK256:
		if (!ecc_make_key(lrlpkc.local_pk256, proxy->local_sk256)) {
			cmd_status(proxy, BT_HCI_ERR_COMMAND_DISALLOWED,
					BT_HCI_CMD_LE_READ_LOCAL_PK256);
			break;
		}
		cmd_status(proxy, BT_HCI_ERR_SUCCESS,
					BT_HCI_CMD_LE_READ_LOCAL_PK256);

		if (!(proxy->event_mask[0] & 0x80))
			break;

		lrlpkc.status = BT_HCI_ERR_SUCCESS;
		le_meta_event(proxy, BT_HCI_EVT_LE_READ_LOCAL_PK256_COMPLETE,
						&lrlpkc, sizeof(lrlpkc));
		break;

	case BT_HCI_CMD_LE_GENERATE_DHKEY:
		lgd = buf + 1 + sizeof(*hdr);
		if (!ecdh_shared_secret(lgd->remote_pk256, proxy->local_sk256,
								lgdc.dhkey)) {
			cmd_status(proxy, BT_HCI_ERR_COMMAND_DISALLOWED,
						BT_HCI_CMD_LE_GENERATE_DHKEY);
			break;
		}
		cmd_status(proxy, BT_HCI_ERR_SUCCESS,
					BT_HCI_CMD_LE_GENERATE_DHKEY);

		if (!(proxy->event_mask[1] & 0x01))
			break;

		lgdc.status = BT_HCI_ERR_SUCCESS;
		le_meta_event(proxy, BT_HCI_EVT_LE_GENERATE_DHKEY_COMPLETE,
							&lgdc, sizeof(lgdc));
		break;

	default:
		host_write_packet(proxy, buf, len);
		break;
	}
}

static void dev_emulate_ecc(struct proxy *proxy, void *buf, uint16_t len)
{
	uint8_t pkt_type = *((uint8_t *) buf);
	struct bt_hci_evt_hdr *hdr = buf + 1;
	struct bt_hci_evt_cmd_complete *cc;
	struct bt_hci_rsp_read_local_commands *rlc;

	if (pkt_type != BT_H4_EVT_PKT) {
		dev_write_packet(proxy, buf, len);
		return;
	}

	switch (hdr->evt) {
	case BT_HCI_EVT_CMD_COMPLETE:
		cc = buf + 1 + sizeof(*hdr);

		switch (le16_to_cpu(cc->opcode)) {
		case BT_HCI_CMD_READ_LOCAL_COMMANDS:
			rlc = buf + 1 + sizeof(*hdr) + sizeof(*cc);
			rlc->commands[34] |= 0x02;	/* P-256 Public Key */
			rlc->commands[34] |= 0x04;	/* Generate DHKey */
			break;
		}

		dev_write_packet(proxy, buf, len);
		break;

	default:
		dev_write_packet(proxy, buf, len);
		break;
	}
}

static void host_read_destroy(void *user_data)
{
	struct proxy *proxy = user_data;

	printf("Closing host descriptor\n");

	if (proxy->host_shutdown)
		shutdown(proxy->host_fd, SHUT_RDWR);

	close(proxy->host_fd);
	proxy->host_fd = -1;

	if (proxy->dev_fd < 0) {
		client_active = false;
		free(proxy);
	} else
		mainloop_remove_fd(proxy->dev_fd);
}

static void host_read_callback(int fd, uint32_t events, void *user_data)
{
	struct proxy *proxy = user_data;
	struct bt_hci_cmd_hdr *cmd_hdr;
	struct bt_hci_acl_hdr *acl_hdr;
	struct bt_hci_sco_hdr *sco_hdr;
	ssize_t len;
	uint16_t pktlen;

	if (events & (EPOLLERR | EPOLLHUP)) {
		fprintf(stderr, "Error from host descriptor\n");
		mainloop_remove_fd(proxy->host_fd);
		return;
	}

	if (events & EPOLLRDHUP) {
		fprintf(stderr, "Remote hangup of host descriptor\n");
		mainloop_remove_fd(proxy->host_fd);
		return;
	}

	len = read(proxy->host_fd, proxy->host_buf + proxy->host_len,
				sizeof(proxy->host_buf) - proxy->host_len);
	if (len < 0) {
		if (errno == EAGAIN || errno == EINTR)
			return;

		fprintf(stderr, "Read from host descriptor failed\n");
		mainloop_remove_fd(proxy->host_fd);
		return;
	}

	if (debug_enabled)
		util_hexdump('>', proxy->host_buf + proxy->host_len, len,
						hexdump_print, "H: ");

	proxy->host_len += len;

process_packet:
	if (proxy->host_len < 1)
		return;

	switch (proxy->host_buf[0]) {
	case BT_H4_CMD_PKT:
		if (proxy->host_len < 1 + sizeof(*cmd_hdr))
			return;

		cmd_hdr = (void *) (proxy->host_buf + 1);
		pktlen = 1 + sizeof(*cmd_hdr) + cmd_hdr->plen;
		break;
	case BT_H4_ACL_PKT:
		if (proxy->host_len < 1 + sizeof(*acl_hdr))
			return;

		acl_hdr = (void *) (proxy->host_buf + 1);
		pktlen = 1 + sizeof(*acl_hdr) + cpu_to_le16(acl_hdr->dlen);
		break;
	case BT_H4_SCO_PKT:
		if (proxy->host_len < 1 + sizeof(*sco_hdr))
			return;

		sco_hdr = (void *) (proxy->host_buf + 1);
		pktlen = 1 + sizeof(*sco_hdr) + sco_hdr->dlen;
		break;
	case 0xff:
		/* Notification packet from /dev/vhci - ignore */
		proxy->host_len = 0;
		return;
	default:
		fprintf(stderr, "Received unknown host packet type 0x%02x\n",
							proxy->host_buf[0]);
		mainloop_remove_fd(proxy->host_fd);
		return;
	}

	if (proxy->host_len < pktlen)
		return;

	if (emulate_ecc)
		host_emulate_ecc(proxy, proxy->host_buf, pktlen);
	else
		host_write_packet(proxy, proxy->host_buf, pktlen);

	if (proxy->host_len > pktlen) {
		memmove(proxy->host_buf, proxy->host_buf + pktlen,
						proxy->host_len - pktlen);
		proxy->host_len -= pktlen;
		goto process_packet;
	}

	proxy->host_len = 0;
}

static void dev_read_destroy(void *user_data)
{
	struct proxy *proxy = user_data;

	printf("Closing device descriptor\n");

	if (proxy->dev_shutdown)
		shutdown(proxy->dev_fd, SHUT_RDWR);

	close(proxy->dev_fd);
	proxy->dev_fd = -1;

	if (proxy->host_fd < 0) {
		client_active = false;
		free(proxy);
	} else
		mainloop_remove_fd(proxy->host_fd);
}

static void dev_read_callback(int fd, uint32_t events, void *user_data)
{
	struct proxy *proxy = user_data;
	struct bt_hci_evt_hdr *evt_hdr;
	struct bt_hci_acl_hdr *acl_hdr;
	struct bt_hci_sco_hdr *sco_hdr;
	ssize_t len;
	uint16_t pktlen;

	if (events & (EPOLLERR | EPOLLHUP)) {
		fprintf(stderr, "Error from device descriptor\n");
		mainloop_remove_fd(proxy->dev_fd);
		return;
	}

	if (events & EPOLLRDHUP) {
		fprintf(stderr, "Remote hangup of device descriptor\n");
		mainloop_remove_fd(proxy->host_fd);
		return;
	}

	len = read(proxy->dev_fd, proxy->dev_buf + proxy->dev_len,
				sizeof(proxy->dev_buf) - proxy->dev_len);
	if (len < 0) {
		if (errno == EAGAIN || errno == EINTR)
			return;

		fprintf(stderr, "Read from device descriptor failed\n");
		mainloop_remove_fd(proxy->dev_fd);
		return;
	}

	if (debug_enabled)
		util_hexdump('>', proxy->dev_buf + proxy->dev_len, len,
						hexdump_print, "D: ");

	proxy->dev_len += len;

process_packet:
	if (proxy->dev_len < 1)
		return;

	switch (proxy->dev_buf[0]) {
	case BT_H4_EVT_PKT:
		if (proxy->dev_len < 1 + sizeof(*evt_hdr))
			return;

		evt_hdr = (void *) (proxy->dev_buf + 1);
		pktlen = 1 + sizeof(*evt_hdr) + evt_hdr->plen;
		break;
	case BT_H4_ACL_PKT:
		if (proxy->dev_len < 1 + sizeof(*acl_hdr))
			return;

		acl_hdr = (void *) (proxy->dev_buf + 1);
		pktlen = 1 + sizeof(*acl_hdr) + cpu_to_le16(acl_hdr->dlen);
		break;
	case BT_H4_SCO_PKT:
		if (proxy->dev_len < 1 + sizeof(*sco_hdr))
			return;

		sco_hdr = (void *) (proxy->dev_buf + 1);
		pktlen = 1 + sizeof(*sco_hdr) + sco_hdr->dlen;
		break;
	default:
		fprintf(stderr, "Received unknown device packet type 0x%02x\n",
							proxy->dev_buf[0]);
		mainloop_remove_fd(proxy->dev_fd);
		return;
	}

	if (proxy->dev_len < pktlen)
		return;

	if (emulate_ecc)
		dev_emulate_ecc(proxy, proxy->dev_buf, pktlen);
	else
		dev_write_packet(proxy, proxy->dev_buf, pktlen);

	if (proxy->dev_len > pktlen) {
		memmove(proxy->dev_buf, proxy->dev_buf + pktlen,
						proxy->dev_len - pktlen);
		proxy->dev_len -= pktlen;
		goto process_packet;
	}

	proxy->dev_len = 0;
}

static bool setup_proxy(int host_fd, bool host_shutdown,
						int dev_fd, bool dev_shutdown)
{
	struct proxy *proxy;

	proxy = new0(struct proxy, 1);
	if (!proxy)
		return false;

	if (emulate_ecc)
		printf("Enabling ECC emulation\n");

	proxy->host_fd = host_fd;
	proxy->host_shutdown = host_shutdown;

	proxy->dev_fd = dev_fd;
	proxy->dev_shutdown = dev_shutdown;

	mainloop_add_fd(proxy->host_fd, EPOLLIN | EPOLLRDHUP,
				host_read_callback, proxy, host_read_destroy);

	mainloop_add_fd(proxy->dev_fd, EPOLLIN | EPOLLRDHUP,
				dev_read_callback, proxy, dev_read_destroy);

	return true;
}

static int open_channel(uint16_t index)
{
	struct sockaddr_hci addr;
	int fd;

	printf("Opening user channel for hci%u\n", hci_index);

	fd = socket(PF_BLUETOOTH, SOCK_RAW | SOCK_CLOEXEC, BTPROTO_HCI);
	if (fd < 0) {
		perror("Failed to open Bluetooth socket");
		return -1;
	}

	memset(&addr, 0, sizeof(addr));
	addr.hci_family = AF_BLUETOOTH;
	addr.hci_dev = index;
	addr.hci_channel = HCI_CHANNEL_USER;

	if (bind(fd, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
		close(fd);
		perror("Failed to bind Bluetooth socket");
		return -1;
	}

	return fd;
}

static void server_callback(int fd, uint32_t events, void *user_data)
{
	union {
		struct sockaddr common;
		struct sockaddr_un sun;
		struct sockaddr_in sin;
	} addr;
	socklen_t len;
	int host_fd, dev_fd;

	if (events & (EPOLLERR | EPOLLHUP)) {
		mainloop_quit();
		return;
	}

	memset(&addr, 0, sizeof(addr));
	len = sizeof(addr);

	if (getsockname(fd, &addr.common, &len) < 0) {
		perror("Failed to get socket name");
		return;
	}

	host_fd = accept(fd, &addr.common, &len);
	if (host_fd < 0) {
		perror("Failed to accept client socket");
		return;
	}

	if (client_active) {
		fprintf(stderr, "Active client already present\n");
		close(host_fd);
		return;
	}

	dev_fd = open_channel(hci_index);
	if (dev_fd < 0) {
		close(host_fd);
		return;
	}

	printf("New client connected\n");

	if (!setup_proxy(host_fd, true, dev_fd, false)) {
		close(dev_fd);
		close(host_fd);
		return;
	}

	client_active = true;
}

static int open_unix(const char *path)
{
	struct sockaddr_un addr;
	int fd;

	unlink(path);

	fd = socket(PF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0);
	if (fd < 0) {
		perror("Failed to open Unix server socket");
		return -1;
	}

	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	strcpy(addr.sun_path, path);

	if (bind(fd, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
		perror("Failed to bind Unix server socket");
		close(fd);
		return -1;
	}

	if (listen(fd, 1) < 0) {
		perror("Failed to listen Unix server socket");
		close(fd);
		return -1;
	}

	if (chmod(path, 0666) < 0)
		perror("Failed to change mode");

	return fd;
}

static int open_tcp(const char *address, unsigned int port)
{
	struct sockaddr_in addr;
	int fd, opt = 1;

	fd = socket(PF_INET, SOCK_STREAM | SOCK_CLOEXEC, 0);
	if (fd < 0) {
		perror("Failed to open TCP server socket");
		return -1;
	}

	setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(address);
	addr.sin_port = htons(port);

	if (bind(fd, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
		perror("Failed to bind TCP server socket");
		close(fd);
		return -1;
	}

	if (listen(fd, 1) < 0) {
		perror("Failed to listen TCP server socket");
		close(fd);
		return -1;
	}

	return fd;
}

static int connect_tcp(const char *address, unsigned int port)
{
	struct sockaddr_in addr;
	int fd;

	fd = socket(PF_INET, SOCK_STREAM | SOCK_CLOEXEC, 0);
	if (fd < 0) {
		perror("Failed to open TCP client socket");
		return -1;
	}

	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(address);
	addr.sin_port = htons(port);

	if (connect(fd, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
		perror("Failed to connect TCP client socket");
		close(fd);
		return -1;
	}

	return fd;
}

static int open_vhci(uint8_t type)
{
	uint8_t create_req[2] = { 0xff, type };
	ssize_t written;
	int fd;

	fd = open("/dev/vhci", O_RDWR | O_CLOEXEC);
	if (fd < 0) {
		perror("Failed to open /dev/vhci device");
		return -1;
	}

	written = write(fd, create_req, sizeof(create_req));
	if (written < 0) {
		perror("Failed to set device type");
		close(fd);
		return -1;
	}

	return fd;
}

static void signal_callback(int signum, void *user_data)
{
	switch (signum) {
	case SIGINT:
	case SIGTERM:
		mainloop_quit();
		break;
	}
}

static void usage(void)
{
	printf("btproxy - Bluetooth controller proxy\n"
		"Usage:\n");
	printf("\tbtproxy [options]\n");
	printf("Options:\n"
		"\t-c, --connect <address>     Connect to server\n"
		"\t-l, --listen [address]      Use TCP server\n"
		"\t-u, --unix [path]           Use Unix server\n"
		"\t-p, --port <port>           Use specified TCP port\n"
		"\t-i, --index <num>           Use specified controller\n"
		"\t-a, --amp                   Create AMP controller\n"
		"\t-e, --ecc                   Emulate ECC support\n"
		"\t-d, --debug                 Enable debugging output\n"
		"\t-h, --help                  Show help options\n");
}

static const struct option main_options[] = {
	{ "redirect", no_argument,       NULL, 'r' },
	{ "connect",  required_argument, NULL, 'c' },
	{ "listen",   optional_argument, NULL, 'l' },
	{ "unix",     optional_argument, NULL, 'u' },
	{ "port",     required_argument, NULL, 'p' },
	{ "index",    required_argument, NULL, 'i' },
	{ "amp",      no_argument,       NULL, 'a' },
	{ "ecc",      no_argument,       NULL, 'e' },
	{ "debug",    no_argument,       NULL, 'd' },
	{ "version",  no_argument,       NULL, 'v' },
	{ "help",     no_argument,       NULL, 'h' },
	{ }
};

int main(int argc, char *argv[])
{
	const char *connect_address = NULL;
	const char *server_address = NULL;
	const char *unix_path = NULL;
	unsigned short tcp_port = 0xb1ee;	/* 45550 */
	bool use_redirect = false;
	uint8_t type = HCI_PRIMARY;
	const char *str;
	sigset_t mask;

	for (;;) {
		int opt;

		opt = getopt_long(argc, argv, "rc:l::u::p:i:aedvh",
						main_options, NULL);
		if (opt < 0)
			break;

		switch (opt) {
		case 'r':
			use_redirect = true;
			break;
		case 'c':
			connect_address = optarg;
			break;
		case 'l':
			if (optarg)
				server_address = optarg;
			else
				server_address = "0.0.0.0";
			break;
		case 'u':
			if (optarg)
				unix_path = optarg;
			else
				unix_path = "/tmp/bt-server-bredr";
			break;
		case 'p':
			tcp_port = atoi(optarg);
			break;
		case 'i':
			if (strlen(optarg) > 3 && !strncmp(optarg, "hci", 3))
				str = optarg + 3;
			else
				str = optarg;
			if (!isdigit(*str)) {
				usage();
				return EXIT_FAILURE;
			}
			hci_index = atoi(str);
			break;
		case 'a':
			type = HCI_AMP;
			break;
		case 'e':
			emulate_ecc = true;
			break;
		case 'd':
			debug_enabled = true;
			break;
		case 'v':
			printf("%s\n", VERSION);
			return EXIT_SUCCESS;
		case 'h':
			usage();
			return EXIT_SUCCESS;
		default:
			return EXIT_FAILURE;
		}
	}

	if (argc - optind > 0) {
		fprintf(stderr, "Invalid command line parameters\n");
		return EXIT_FAILURE;
	}

	if (unix_path && (server_address || use_redirect)) {
		fprintf(stderr, "Invalid to specify TCP and Unix servers\n");
		return EXIT_FAILURE;
	}

	if (connect_address && (unix_path || server_address || use_redirect)) {
		fprintf(stderr, "Invalid to specify client and server mode\n");
		return EXIT_FAILURE;
	}

	mainloop_init();

	sigemptyset(&mask);
	sigaddset(&mask, SIGINT);
	sigaddset(&mask, SIGTERM);

	mainloop_set_signal(&mask, signal_callback, NULL, NULL);

	if (connect_address || use_redirect) {
		int host_fd, dev_fd;

		if (use_redirect) {
			printf("Creating local redirect\n");

			dev_fd = open_channel(hci_index);
		} else {
			printf("Connecting to %s:%u\n", connect_address,
								tcp_port);

			dev_fd = connect_tcp(connect_address, tcp_port);
		}

		if (dev_fd < 0)
			return EXIT_FAILURE;

		printf("Opening virtual device\n");

		host_fd = open_vhci(type);
		if (host_fd < 0) {
			close(dev_fd);
			return EXIT_FAILURE;
		}

		if (!setup_proxy(host_fd, false, dev_fd, true)) {
			close(dev_fd);
			close(host_fd);
			return EXIT_FAILURE;
		}
	} else {
		int server_fd;

		if (unix_path) {
			printf("Listening on %s\n", unix_path);

			server_fd = open_unix(unix_path);
		} else if (server_address) {
			printf("Listening on %s:%u\n", server_address,
								tcp_port);

			server_fd = open_tcp(server_address, tcp_port);
		} else {
			fprintf(stderr, "Missing emulator device\n");
			return EXIT_FAILURE;
		}

		if (server_fd < 0)
			return EXIT_FAILURE;

		mainloop_add_fd(server_fd, EPOLLIN, server_callback,
							NULL, NULL);
	}

	return mainloop_run();
}
