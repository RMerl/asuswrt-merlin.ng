// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2021 Vyacheslav Bocharov
 * Author: Vyacheslav Bocharov <adeep@lexina.in>
 */

#include <common.h>
#include <dm.h>
#include <init.h>
#include <net.h>
#include <asm/io.h>
#include <asm/arch/axg.h>
#include <asm/arch/sm.h>
#include <asm/arch/eth.h>
#include <asm/arch/mem.h>
#include <u-boot/crc.h>

int misc_init_r(void)
{
	u8 mac_addr[ARP_HLEN + 1];
	char serial[SM_SERIAL_SIZE];
	u32 sid;

	if (!meson_sm_get_serial(serial, SM_SERIAL_SIZE)) {
		sid = crc32(0, (unsigned char *)serial, SM_SERIAL_SIZE);
		/* Ensure the NIC specific bytes of the mac are not all 0 */
		if ((sid & 0xffff) == 0)
			sid |= 0x800000;

		/* OUI registered MAC address */
		mac_addr[0] = 0x10;
		mac_addr[1] = 0x27;
		mac_addr[2] = 0xBE;
		mac_addr[3] = (sid >> 16) & 0xff;
		mac_addr[4] = (sid >>  8) & 0xff;
		mac_addr[5] = (sid >>  0) & 0xff;
		mac_addr[ARP_HLEN] = '\0';

		eth_env_set_enetaddr("ethaddr", mac_addr);
	}

	return 0;
}
