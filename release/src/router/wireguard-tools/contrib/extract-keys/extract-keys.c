// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2015-2020 Jason A. Donenfeld <Jason@zx2c4.com>. All Rights Reserved.
 */

#include <endian.h>
#include <errno.h>
#include <fcntl.h>
#include <resolv.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "config.h"

static int fd;

static void open_kmem(void)
{
	fd = open("/dev/kmem", O_RDONLY);
	if (fd < 0) {
		perror("open(/dev/kmem)");
		exit(errno);
	}
}

static void read_kmem(void *buffer, size_t len, unsigned long addr)
{
	if (lseek(fd, addr, SEEK_SET) == (off_t)-1) {
		perror("lseek");
		exit(errno);
	}
	if (read(fd, buffer, len) != len) {
		perror("read");
		exit(errno);
	}
}

static inline unsigned int read_int(unsigned long addr)
{
	unsigned int ret;
	read_kmem(&ret, sizeof(ret), addr);
	return ret;
}

static inline unsigned long read_long(unsigned long addr)
{
	unsigned long ret;
	read_kmem(&ret, sizeof(ret), addr);
	return ret;
}

static unsigned long find_interface(const char *interface)
{
	FILE *f = fopen("/proc/net/udp", "r");
	char line[256], *ptr;
	unsigned long addr = 0;
	char name[IFNAMSIZ + 1] = { 0 };

	if (!f) {
		perror("fopen(/proc/net/udp)");
		exit(errno);
	}
	if (!fgets(line, 256, f))
		goto out;
	while (fgets(line, 256, f)) {
		ptr = line + strlen(line) - 1;
		while (*--ptr == ' ');
		while (*--ptr != ' ');
		while (*(--ptr - 1) != ' ');
		addr = strtoul(ptr, NULL, 16);
		if (!addr)
			continue;
		addr = read_long(addr + SOCK_DEVICE_OFFSET);
		if (!addr)
			continue;
		read_kmem(name, IFNAMSIZ, addr + DEVICE_NAME_OFFSET);
		if (!strcmp(name, interface))
			goto out;
	}
	addr = 0;
out:
	fclose(f);
	return addr;
}

static bool print_key(unsigned long key)
{
	unsigned char sending[32], receiving[32];
	char sending_b64[45], receiving_b64[45];
	unsigned int local_index, remote_index;

	if (!key)
		return false;

	local_index = le32toh(read_int(key + KEY_LOCALID_OFFSET));
	remote_index = le32toh(read_int(key + KEY_REMOTEID_OFFSET));
	read_kmem(sending, 32, key + KEY_SENDING_OFFSET);
	read_kmem(receiving, 32, key + KEY_RECEIVING_OFFSET);

	b64_ntop(sending, 32, sending_b64, 45);
	b64_ntop(receiving, 32, receiving_b64, 45);

	printf("0x%08x %s\n", local_index, receiving_b64);
	printf("0x%08x %s\n", remote_index, sending_b64);
	return true;
}

static bool walk_peers(unsigned long peer_head)
{
	unsigned long peer, peer_entry;
	bool found = false;
	for (peer_entry = read_long(peer_head); peer_entry != peer_head; peer_entry = read_long(peer_entry)) {
		peer = peer_entry + PEERS_PEER_OFFSET;
		if (print_key(read_long(peer + PEER_CURRENTKEY_OFFSET)))
			found = true;
		if (print_key(read_long(peer + PEER_PREVIOUSKEY_OFFSET)))
			found = true;
		if (print_key(read_long(peer + PEER_NEXTKEY_OFFSET)))
			found = true;
	}
	return found;
}

int main(int argc, char *argv[])
{
	unsigned long wireguard_device;
	if (argc < 2) {
		fprintf(stderr, "Usage: %s WIREGUARD_INTERFACE\n", argv[0]);
		return EINVAL;
	}
	open_kmem();
	wireguard_device = find_interface(argv[1]);
	if (!wireguard_device) {
		fprintf(stderr, "Could not find interface %s\n", argv[1]);
		return EBADSLT;
	}
	if (!walk_peers(wireguard_device + DEVICE_PEERS_OFFSET)) {
		fprintf(stderr, "No active sessions\n");
		return ENOKEY;
	}
	return 0;
}
