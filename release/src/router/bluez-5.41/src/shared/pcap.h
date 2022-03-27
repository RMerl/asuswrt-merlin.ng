/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2012-2014  Intel Corporation. All rights reserved.
 *
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
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
