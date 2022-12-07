/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2012-2014  Intel Corporation. All rights reserved.
 *
 *
 */

#include <stdint.h>
#include <stdbool.h>
#include <sys/time.h>

#define PCAP_TYPE_INVALID		0
#define PCAP_TYPE_USER0			147
#define PCAP_TYPE_PPI			192
#define PCAP_TYPE_BLUETOOTH_LE_LL	251

struct pcap;

struct pcap *pcap_open(const char *path);

struct pcap *pcap_ref(struct pcap *pcap);
void pcap_unref(struct pcap *pcap);

uint32_t pcap_get_type(struct pcap *pcap);
uint32_t pcap_get_snaplen(struct pcap *pcap);

bool pcap_read(struct pcap *pcap, struct timeval *tv,
				void *data, uint32_t size, uint32_t *len);
bool pcap_read_ppi(struct pcap *pcap, struct timeval *tv, uint32_t *type,
					void *data, uint32_t size,
					uint32_t *offset, uint32_t *len);
