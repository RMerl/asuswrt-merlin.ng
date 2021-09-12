// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2015-2020 Jason A. Donenfeld <Jason@zx2c4.com>. All Rights Reserved.
 *
 * Example only. Do not run in production.
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

struct entry {
	uint8_t pubkey[32];
	uint32_t ip;
	uint16_t port;
};

enum { MAX_ENTRIES = 65536, PORT = 49918 };

static struct entry entries[MAX_ENTRIES];
static unsigned int next_entry;

/* XX: this should use a hash table */
static struct entry *find_entry(uint8_t key[32])
{
	int i;
	for (i = 0; i < MAX_ENTRIES; ++i) {
		if (!memcmp(entries[i].pubkey, key, 32))
			return &entries[i];
	}
	return NULL;
}

/* XX: this is obviously vulnerable to DoS */
static struct entry *find_or_insert_entry(uint8_t key[32])
{
	struct entry *entry = find_entry(key);
	if (!entry) {
		entry = &entries[next_entry++ % MAX_ENTRIES];
		memcpy(entry->pubkey, key, 32);
	}
	return entry;
}

int main(int argc, char *argv[])
{
	struct sockaddr_in addr = {
		.sin_family = AF_INET,
		.sin_addr = { .s_addr = htonl(INADDR_ANY) },
		.sin_port = htons(PORT)
	};
	struct {
		uint8_t my_pubkey[32];
		uint8_t their_pubkey[32];
	} __attribute__((packed)) packet;
	struct {
		uint32_t ip;
		uint16_t port;
	} __attribute__((packed)) reply;
	struct entry *entry;
	socklen_t len;
	ssize_t retlen;
	int optval;
	int sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock < 0) {
		perror("socket");
		return errno;
	}

	optval = 1;
	if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0) {
		perror("setsockopt");
		return errno;
	}

	if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		perror("bind");
		return errno;
	}

	for (;;) {
		len = sizeof(addr);
		if (recvfrom(sock, &packet, sizeof(packet), 0, (struct sockaddr *)&addr, &len) != sizeof(packet)) {
			perror("recvfrom");
			continue;
		}
		entry = find_or_insert_entry(packet.my_pubkey);
		entry->ip = addr.sin_addr.s_addr;
		entry->port = addr.sin_port;
		entry = find_entry(packet.their_pubkey);
		if (entry) {
			reply.ip = entry->ip;
			reply.port = entry->port;
			if (sendto(sock, &reply, sizeof(reply), 0, (struct sockaddr *)&addr, len) < 0) {
				perror("sendto");
				continue;
			}
		} else {
			if (sendto(sock, NULL, 0, 0, (struct sockaddr *)&addr, len) < 0) {
				perror("sendto");
				continue;
			}
		}
	}

	close(sock);
	return 0;
}
