// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018 Marek Behún <kabel@kernel.org>
 */

#include <common.h>
#include <asm/arch/soc.h>
#include <asm/io.h>
#include <linux/bitops.h>
#include <linux/delay.h>
#include <mach/mbox.h>

#include "mox_sp.h"

const char *mox_sp_get_ecdsa_public_key(void)
{
	static char public_key[135];
	u32 out[16];
	int res;

	if (public_key[0])
		return public_key;

	res = mbox_do_cmd(MBOX_CMD_ECDSA_PUB_KEY, NULL, 0, out, 16);
	if (res < 0)
		return NULL;

	sprintf(public_key,
		"%06x%08x%08x%08x%08x%08x%08x%08x%08x%08x%08x%08x%08x%08x%08x%08x%08x",
		(u32)res, out[0], out[1], out[2], out[3], out[4], out[5],
		out[6], out[7], out[8], out[9], out[10], out[11], out[12],
		out[13], out[14], out[15]);

	return public_key;
}

static inline void res_to_mac(u8 *mac, u32 t1, u32 t2)
{
	mac[0] = t1 >> 8;
	mac[1] = t1;
	mac[2] = t2 >> 24;
	mac[3] = t2 >> 16;
	mac[4] = t2 >> 8;
	mac[5] = t2;
}

int mbox_sp_get_board_info(u64 *sn, u8 *mac1, u8 *mac2, int *bv, int *ram,
			   enum cznic_a3720_board *board)
{
	u32 out[9];
	int res;

	res = mbox_do_cmd(MBOX_CMD_BOARD_INFO, NULL, 0, out, 9);
	if (res < 0)
		return res;

	if (sn) {
		*sn = out[1];
		*sn <<= 32;
		*sn |= out[0];
	}

	if (bv)
		*bv = out[2];

	if (ram)
		*ram = out[3];

	if (mac1)
		res_to_mac(mac1, out[4], out[5]);

	if (mac2)
		res_to_mac(mac2, out[6], out[7]);

	if (board)
		*board = out[8] + 1;

	return 0;
}
