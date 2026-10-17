// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2017 NXP
 *
 * Peng Fan <peng.fan@nxp.com>
 */

#include <common.h>
#include <asm/arch/imx-regs.h>
#include <asm/io.h>
#include <asm/arch/sys_proto.h>
#include <errno.h>

struct imx_mac_fuse {
	u32 mac_addr0;
	u32 rsvd0[3];
	u32 mac_addr1;
	u32 rsvd1[3];
	u32 mac_addr2;
	u32 rsvd2[7];
};

#define MAC_FUSE_MX6_OFFSET	0x620
#define MAC_FUSE_MX7_OFFSET	0x640

void imx_get_mac_from_fuse(int dev_id, unsigned char *mac)
{
	struct imx_mac_fuse *fuse;
	u32 offset;
	bool has_second_mac;

	offset = is_mx6() ? MAC_FUSE_MX6_OFFSET : MAC_FUSE_MX7_OFFSET;
	fuse = (struct imx_mac_fuse *)(ulong)(OCOTP_BASE_ADDR + offset);
	has_second_mac = is_mx7() || is_mx6sx() || is_mx6ul() || is_mx6ull() || is_imx8mp();

	if (has_second_mac && dev_id == 1) {
		u32 value = readl(&fuse->mac_addr2);

		mac[0] = value >> 24;
		mac[1] = value >> 16;
		mac[2] = value >> 8;
		mac[3] = value;

		value = readl(&fuse->mac_addr1);
		mac[4] = value >> 24;
		mac[5] = value >> 16;

	} else {
		u32 value = readl(&fuse->mac_addr1);

		mac[0] = value >> 8;
		mac[1] = value;

		value = readl(&fuse->mac_addr0);
		mac[2] = value >> 24;
		mac[3] = value >> 16;
		mac[4] = value >> 8;
		mac[5] = value;
	}
}
