// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018 Marek Behun <marek.behun@nic.cz>
 */

#include <common.h>
#include <asm/io.h>

#define RWTM_CMD_PARAM(i)	(size_t)(0xd00b0000 + (i) * 4)
#define RWTM_CMD		0xd00b0040
#define RWTM_CMD_RETSTATUS	0xd00b0080
#define RWTM_CMD_STATUS(i)	(size_t)(0xd00b0084 + (i) * 4)

#define RWTM_HOST_INT_RESET	0xd00b00c8
#define RWTM_HOST_INT_MASK	0xd00b00cc
#define SP_CMD_COMPLETE		BIT(0)

#define MBOX_STS_SUCCESS		(0x0 << 30)
#define MBOX_STS_FAIL			(0x1 << 30)
#define MBOX_STS_BADCMD			(0x2 << 30)
#define MBOX_STS_LATER			(0x3 << 30)
#define MBOX_STS_ERROR(s)		((s) & (3 << 30))
#define MBOX_STS_VALUE(s)		(((s) >> 10) & 0xfffff)
#define MBOX_STS_CMD(s)			((s) & 0x3ff)

enum mbox_cmd {
	MBOX_CMD_GET_RANDOM	= 1,
	MBOX_CMD_BOARD_INFO,
	MBOX_CMD_ECDSA_PUB_KEY,
	MBOX_CMD_HASH,
	MBOX_CMD_SIGN,
	MBOX_CMD_VERIFY,

	MBOX_CMD_OTP_READ,
	MBOX_CMD_OTP_WRITE
};

static int mbox_do_cmd(enum mbox_cmd cmd, u32 *out, int nout)
{
	const int tries = 50;
	int i;
	u32 status;

	clrbits_le32(RWTM_HOST_INT_MASK, SP_CMD_COMPLETE);

	writel(cmd, RWTM_CMD);

	for (i = 0; i < tries; ++i) {
		mdelay(10);
		if (readl(RWTM_HOST_INT_RESET) & SP_CMD_COMPLETE)
			break;
	}

	if (i == tries) {
		/* if timed out, don't read status */
		setbits_le32(RWTM_HOST_INT_RESET, SP_CMD_COMPLETE);
		return -ETIMEDOUT;
	}

	for (i = 0; i < nout; ++i)
		out[i] = readl(RWTM_CMD_STATUS(i));
	status = readl(RWTM_CMD_RETSTATUS);

	setbits_le32(RWTM_HOST_INT_RESET, SP_CMD_COMPLETE);

	if (MBOX_STS_CMD(status) != cmd)
		return -EIO;
	else if (MBOX_STS_ERROR(status) == MBOX_STS_FAIL)
		return -(int)MBOX_STS_VALUE(status);
	else if (MBOX_STS_ERROR(status) != MBOX_STS_SUCCESS)
		return -EIO;
	else
		return MBOX_STS_VALUE(status);
}

const char *mox_sp_get_ecdsa_public_key(void)
{
	static char public_key[135];
	u32 out[16];
	int res;

	if (public_key[0])
		return public_key;

	res = mbox_do_cmd(MBOX_CMD_ECDSA_PUB_KEY, out, 16);
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

int mbox_sp_get_board_info(u64 *sn, u8 *mac1, u8 *mac2, int *bv, int *ram)
{
	u32 out[8];
	int res;

	res = mbox_do_cmd(MBOX_CMD_BOARD_INFO, out, 8);
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

	return 0;
}
