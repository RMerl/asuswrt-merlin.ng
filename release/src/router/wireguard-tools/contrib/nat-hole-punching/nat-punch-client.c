// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2015-2020 Jason A. Donenfeld <Jason@zx2c4.com>. All Rights Reserved.
 *
 * Example only. Do not run in production.
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/udp.h>
#include <netinet/ip.h>
#include <linux/filter.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdarg.h>
#include <string.h>
#include <resolv.h>
#include <stdint.h>
#include <stdbool.h>

enum { MAX_PEERS = 65536, PORT = 49918 };

static struct {
	uint8_t base64_key[45];
	bool have_seen;
} peers[MAX_PEERS];
static unsigned int total_peers;

static const char *cmd(const char *line, ...)
{
	static char buf[2048];
	char full_cmd[2048] = { 0 };
	size_t len;
	FILE *f;
	va_list args;
	va_start(args, line);
	vsnprintf(full_cmd, 2047, line, args);
	va_end(args);
	f = popen(full_cmd, "r");
	if (!f) {
		perror("popen");
		exit(errno);
	}
	if (!fgets(buf, 2048, f)) {
		pclose(f);
		return NULL;
	}
	pclose(f);
	len = strlen(buf);
	if (!len)
		return NULL;
	if (buf[len - 1] == '\n')
		buf[len - 1] = '\0';
	return buf;
}

static void read_peers(const char *interface)
{
	char full_cmd[2048] = { 0 };
	size_t len;
	FILE *f;
	snprintf(full_cmd, 2047, "wg show %s peers", interface);
	f = popen(full_cmd, "r");
	if (!f) {
		perror("popen");
		exit(errno);
	}
	for (;;) {
		if (!fgets(peers[total_peers].base64_key, 45, f))
			break;
		len = strlen(peers[total_peers].base64_key);
		if (len != 44 && len != 45)
			continue;
		if (peers[total_peers].base64_key[len - 1] == '\n')
			peers[total_peers].base64_key[len - 1] = '\0';
		++total_peers;
	}
	pclose(f);
}

static void unbase64(uint8_t dstkey[32], const char *srckey)
{
	uint8_t buf[33];
	if (b64_pton(srckey, buf, 33) != 32) {
		fprintf(stderr, "Could not parse base64 key: %s\n", srckey);
		exit(EINVAL);
	}
	memcpy(dstkey, buf, 32);
}

static void apply_bpf(int sock, uint16_t port, uint32_t ip)
{
	struct sock_filter filter[] = {
		BPF_STMT(BPF_LD + BPF_W + BPF_ABS, 12 /* src ip */),
		BPF_JUMP(BPF_JMP + BPF_JEQ + BPF_K, ip, 0, 5),
		BPF_STMT(BPF_LD + BPF_H + BPF_ABS, 20 /* src port */),
		BPF_JUMP(BPF_JMP + BPF_JEQ + BPF_K, PORT, 0, 3),
		BPF_STMT(BPF_LD + BPF_H + BPF_ABS, 22 /* dst port */),
		BPF_JUMP(BPF_JMP + BPF_JEQ + BPF_K, port, 0, 1),
		BPF_STMT(BPF_RET + BPF_K, -1),
		BPF_STMT(BPF_RET + BPF_K, 0)
	};
	struct sock_fprog filter_prog = {
		.len = sizeof(filter) / sizeof(filter[0]),
		.filter = filter
	};
	if (setsockopt(sock, SOL_SOCKET, SO_ATTACH_FILTER, &filter_prog, sizeof(filter_prog)) < 0) {
		perror("setsockopt(bpf)");
		exit(errno);
	}
}

int main(int argc, char *argv[])
{
	struct sockaddr_in addr = {
		.sin_family = AF_INET
	};
	struct {
		struct udphdr udp;
		uint8_t my_pubkey[32];
		uint8_t their_pubkey[32];
	} __attribute__((packed)) packet = {
		.udp = {
			.len = htons(sizeof(packet)),
			.dest = htons(PORT)
		}
	};
	struct {
		struct iphdr iphdr;
		struct udphdr udp;
		uint32_t ip;
		uint16_t port;
	} __attribute__((packed)) reply;
	ssize_t len;
	int sock, i;
	bool repeat;
	struct hostent *ent;
	const char *server = argv[1], *interface = argv[2];

	if (argc < 3) {
		fprintf(stderr, "Usage: %s SERVER WIREGUARD_INTERFACE\nExample:\n    %s demo.wireguard.com wg0\n", argv[0], argv[0]);
		return EINVAL;
	}

	if (getuid() != 0) {
		fprintf(stderr, "Must be root!\n");
		return EPERM;
	}

	ent = gethostbyname2(server, AF_INET);
	if (!ent) {
		herror("gethostbyname2");
		return h_errno;
	}
	addr.sin_addr = *(struct in_addr *)ent->h_addr;
	read_peers(interface);
	cmd("ip link set %s up", interface);
	unbase64(packet.my_pubkey, cmd("wg show %s public-key", interface));
	packet.udp.source = htons(atoi(cmd("wg show %s listen-port", interface)));

	/* We use raw sockets so that the WireGuard interface can actually own the real socket. */
	sock = socket(AF_INET, SOCK_RAW, IPPROTO_UDP);
	if (sock < 0) {
		perror("socket");
		return errno;
	}
	apply_bpf(sock, ntohs(packet.udp.source), ntohl(addr.sin_addr.s_addr));

check_again:
	repeat = false;
	for (i = 0; i < total_peers; ++i) {
		if (peers[i].have_seen)
			continue;
		printf("[+] Requesting IP and port of %s: ", peers[i].base64_key);
		unbase64(packet.their_pubkey, peers[i].base64_key);
		if (sendto(sock, &packet, sizeof(packet), 0, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
			putchar('\n');
			perror("sendto");
			return errno;
		}
		len = recv(sock, &reply, sizeof(reply), 0);
		if (len < 0) {
			putchar('\n');
			perror("recv");
			return errno;
		}
		if (len != sizeof(reply)) {
			printf("server does not yet have it\n");
			repeat = true;
		} else {
			printf("%s:%d\n", inet_ntoa(*(struct in_addr *)&reply.ip), ntohs(reply.port));
			peers[i].have_seen = true;
			cmd("wg set %s peer %s persistent-keepalive 25 endpoint %s:%d", interface, peers[i].base64_key, inet_ntoa(*(struct in_addr *)&reply.ip), ntohs(reply.port));
		}
	}
	if (repeat) {
		sleep(2);
		goto check_again;
	}

	close(sock);
	return 0;
}
