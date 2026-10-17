// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright 2019 Ramon Fried <rfried.dev@gmail.com>
 */

#include <common.h>
#include <net.h>
#include <net/pcap.h>
#include <time.h>
#include <asm/io.h>

#define LINKTYPE_ETHERNET	1

static bool initialized;
static bool running;
static bool buffer_full;
static void *buf;
static unsigned int max_size;
static unsigned int pos;

static unsigned long incoming_count;
static unsigned long outgoing_count;

struct pcap_header {
	u32 magic;
	u16 version_major;
	u16 version_minor;
	s32 thiszone;
	u32 sigfigs;
	u32 snaplen;
	u32 network;
};

struct pcap_packet_header {
	u32 ts_sec;
	u32 ts_usec;
	u32 incl_len;
	u32 orig_len;
};

static struct pcap_header file_header = {
	.magic = 0xa1b2c3d4,
	.version_major = 2,
	.version_minor = 4,
	.snaplen = 65535,
	.network = LINKTYPE_ETHERNET,
};

int pcap_init(phys_addr_t paddr, unsigned long size)
{
	buf = map_physmem(paddr, size, 0);
	if (!buf) {
		printf("Failed mapping PCAP memory\n");
		return -ENOMEM;
	}

	printf("PCAP capture initialized: addr: 0x%lx max length: %lu\n",
	       (unsigned long)buf, size);

	memcpy(buf, &file_header, sizeof(file_header));
	pos = sizeof(file_header);
	max_size = size;
	initialized = true;
	running = false;
	buffer_full = false;
	incoming_count = 0;
	outgoing_count = 0;
	return 0;
}

int pcap_start_stop(bool start)
{
	if (!initialized) {
		printf("error: pcap was not initialized\n");
		return -ENODEV;
	}

	running = start;

	return 0;
}

int pcap_clear(void)
{
	if (!initialized) {
		printf("error: pcap was not initialized\n");
		return -ENODEV;
	}

	pos = sizeof(file_header);
	incoming_count = 0;
	outgoing_count = 0;
	buffer_full = false;

	printf("pcap capture cleared\n");
	return 0;
}

int pcap_post(const void *packet, size_t len, bool outgoing)
{
	struct pcap_packet_header header;
	u64 cur_time = timer_get_us();

	if (!initialized || !running || !buf)
		return -ENODEV;

	if (buffer_full)
		return -ENOMEM;

	if ((pos + len + sizeof(header)) >= max_size) {
		buffer_full = true;
		printf("\n!!! Buffer is full, consider increasing buffer size !!!\n");
		return -ENOMEM;
	}

	header.ts_sec = cur_time / 1000000;
	header.ts_usec = cur_time % 1000000;
	header.incl_len = len;
	header.orig_len = len;

	memcpy(buf + pos, &header, sizeof(header));
	pos += sizeof(header);
	memcpy(buf + pos, packet, len);
	pos += len;

	if (outgoing)
		outgoing_count++;
	else
		incoming_count++;

	env_set_hex("pcapsize", pos);

	return 0;
}

int pcap_print_status(void)
{
	if (!initialized) {
		printf("pcap was not initialized\n");
		return -ENODEV;
	}
	printf("PCAP status:\n");
	printf("\tInitialized addr: 0x%lx\tmax length: %u\n",
	       (unsigned long)buf, max_size);
	printf("\tStatus: %s.\t file size: %u\n", running ? "Active" : "Idle",
	       pos);
	printf("\tIncoming packets: %lu Outgoing packets: %lu\n",
	       incoming_count, outgoing_count);

	return 0;
}

bool pcap_active(void)
{
	return running;
}
