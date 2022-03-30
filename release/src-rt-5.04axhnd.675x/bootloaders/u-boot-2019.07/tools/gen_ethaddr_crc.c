// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2016
 * Olliver Schinagl <oliver@schinagl.nl>
 */

#include <ctype.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <u-boot/crc.h>

#define ARP_HLEN 6 /* Length of hardware address */
#define ARP_HLEN_ASCII (ARP_HLEN * 2) + (ARP_HLEN - 1) /* with separators */
#define ARP_HLEN_LAZY (ARP_HLEN * 2) /* separatorless hardware address length */

uint8_t nibble_to_hex(const char *nibble, bool lo)
{
	return (strtol(nibble, NULL, 16) << (lo ? 0 : 4)) & (lo ? 0x0f : 0xf0);
}

int process_mac(const char *mac_address)
{
	uint8_t ethaddr[ARP_HLEN + 1] = { 0x00 };
	uint_fast8_t i = 0;

	while (*mac_address != '\0') {
		char nibble[2] = { 0x00, '\n' }; /* for strtol */

		nibble[0] = *mac_address++;
		if (isxdigit(nibble[0])) {
			if (isupper(nibble[0]))
				nibble[0] = tolower(nibble[0]);
			ethaddr[i >> 1] |= nibble_to_hex(nibble, (i % 2) != 0);
			i++;
		}
	}

	for (i = 0; i < ARP_HLEN; i++)
		printf("%.2x", ethaddr[i]);
	printf("%.2x\n", crc8(0, ethaddr, ARP_HLEN));

	return 0;
}

void print_usage(char *cmdname)
{
	printf("Usage: %s <mac_address>\n", cmdname);
	puts("<mac_address> may be with or without separators.");
	puts("Valid seperators are ':' and '-'.");
	puts("<mac_address> digits are in base 16.\n");
}

int main(int argc, char *argv[])
{
	if (argc < 2) {
		print_usage(argv[0]);
		return 1;
	}

	if (!((strlen(argv[1]) == ARP_HLEN_ASCII) || (strlen(argv[1]) == ARP_HLEN_LAZY))) {
		puts("The MAC address is not valid.\n");
		print_usage(argv[0]);
		return 1;
	}

	if (process_mac(argv[1])) {
		puts("Failed to calculate the MAC's checksum.");
		return 1;
	}

	return 0;
}
