// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) 2017 Marvell International Ltd.
 * (C) 2021 Pali Rohár <pali@kernel.org>
 */

#include <config.h>
#include <common.h>
#include <asm/io.h>
#include <linux/delay.h>
#include <mach/mbox.h>
#include <mach/soc.h>

#define OTP_NB_REG_BASE		((void __iomem *)MVEBU_REGISTER(0x12600))
#define OTP_SB_REG_BASE		((void __iomem *)MVEBU_REGISTER(0x1A200))

#define OTP_CONTROL_OFF		0x00
#define   OTP_MODE_BIT		BIT(15)
#define   OTP_RPTR_RST_BIT	BIT(14)
#define   OTP_POR_B_BIT		BIT(13)
#define   OTP_PRDT_BIT		BIT(3)
#define OTP_READ_PORT_OFF	0x04
#define OTP_READ_POINTER_OFF	0x08
#define   OTP_PTR_INC_BIT	BIT(8)

static void otp_read_parallel(void __iomem *base, u32 *data, u32 count)
{
	u32 regval;

	/* 1. Clear OTP_MODE_NB to parallel mode */
	regval = readl(base + OTP_CONTROL_OFF);
	regval &= ~OTP_MODE_BIT;
	writel(regval, base + OTP_CONTROL_OFF);

	/* 2. Set OTP_POR_B_NB enter normal operation */
	regval = readl(base + OTP_CONTROL_OFF);
	regval |= OTP_POR_B_BIT;
	writel(regval, base + OTP_CONTROL_OFF);

	/* 3. Set OTP_PTR_INC_NB to auto-increment pointer after each read */
	regval = readl(base + OTP_READ_POINTER_OFF);
	regval |= OTP_PTR_INC_BIT;
	writel(regval, base + OTP_READ_POINTER_OFF);

	/* 4. Set OTP_RPTR_RST_NB, then clear the same field */
	regval = readl(base + OTP_CONTROL_OFF);
	regval |= OTP_RPTR_RST_BIT;
	writel(regval, base + OTP_CONTROL_OFF);

	regval = readl(base + OTP_CONTROL_OFF);
	regval &= ~OTP_RPTR_RST_BIT;
	writel(regval, base + OTP_CONTROL_OFF);

	/* 5. Toggle OTP_PRDT_NB
	 * a. Set OTP_PRDT_NB to 1.
	 * b. Clear OTP_PRDT_NB to 0.
	 * c. Wait for a minimum of 100 ns.
	 * d. Set OTP_PRDT_NB to 1
	 */
	regval = readl(base + OTP_CONTROL_OFF);
	regval |= OTP_PRDT_BIT;
	writel(regval, base + OTP_CONTROL_OFF);

	regval = readl(base + OTP_CONTROL_OFF);
	regval &= ~OTP_PRDT_BIT;
	writel(regval, base + OTP_CONTROL_OFF);

	ndelay(100);

	regval = readl(base + OTP_CONTROL_OFF);
	regval |= OTP_PRDT_BIT;
	writel(regval, base + OTP_CONTROL_OFF);

	while (count-- > 0) {
		/* 6. Read the content of OTP 32-bits at a time */
		ndelay(100000);
		*(data++) = readl(base + OTP_READ_PORT_OFF);
	}
}

static int rwtm_otp_read(u8 row, u32 word, u32 *data)
{
	u32 out[3];
	u32 in[2];
	int res = -EINVAL;

	if (word < 2) {
		/*
		 * MBOX_CMD_OTP_READ_32B command is supported by Marvell
		 * fuse.bin firmware and also by new CZ.NIC wtmi firmware.
		 * This command returns raw bits without ECC corrections.
		 * It does not provide access to the lock bit.
		 */
		in[0] = row;
		in[1] = word * 32;
		res = mbox_do_cmd(MBOX_CMD_OTP_READ_32B, in, 2, out, 1);
		if (!res)
			*data = out[0];
	} else if (word == 2) {
		/*
		 * MBOX_CMD_OTP_READ command is supported only by new CZ.NIC
		 * wtmi firmware and provides access to all bits, including
		 * lock bit without doing ECC corrections. For compatibility
		 * with Marvell fuse.bin firmware, use this command only for
		 * accessing lock bit.
		 */
		in[0] = row;
		res = mbox_do_cmd(MBOX_CMD_OTP_READ, in, 1, out, 3);
		if (!res)
			*data = out[2];
	}

	return res;
}

static int rwtm_otp_write(u8 row, u32 word, u32 data)
{
	u32 in[4];
	int res = -EINVAL;

	if (word < 2) {
		/*
		 * MBOX_CMD_OTP_WRITE_32B command is supported by Marvell
		 * fuse.bin firmware and also by new CZ.NIC wtmi firmware.
		 * This command writes only selected bits to OTP and does
		 * not calculate ECC bits. It does not allow to write the
		 * lock bit.
		 */
		in[0] = row;
		in[1] = word * 32;
		in[2] = data;
		res = mbox_do_cmd(MBOX_CMD_OTP_WRITE_32B, in, 3, NULL, 0);
	} else if (word == 2 && !(data & ~0x1)) {
		/*
		 * MBOX_CMD_OTP_WRITE command is supported only by new CZ.NIC
		 * wtmi firmware and allows to write any bit to OTP, including
		 * the lock bit. It does not calculate or write ECC bits too.
		 * For compatibility with Marvell fuse.bin firmware, use this
		 * command only for writing the lock bit.
		 */
		in[0] = row;
		in[1] = 0;
		in[2] = 0;
		in[3] = data;
		res = mbox_do_cmd(MBOX_CMD_OTP_WRITE, in, 4, NULL, 0);
	}

	return res;
}

/*
 * Banks 0-43 are used for accessing Security OTP (44 rows with 67 bits via 44 banks and words 0-2)
 * Bank 44 is used for accessing North Bridge OTP (69 bits via words 0-2)
 * Bank 45 is used for accessing South Bridge OTP (97 bits via words 0-3)
 */

#define RWTM_ROWS	44
#define RWTM_MAX_BANK	(RWTM_ROWS - 1)
#define RWTM_ROW_WORDS	3
#define OTP_NB_BANK	RWTM_ROWS
#define OTP_NB_WORDS	3
#define OTP_SB_BANK	(RWTM_ROWS + 1)
#define OTP_SB_WORDS	4

int fuse_read(u32 bank, u32 word, u32 *val)
{
	if (bank <= RWTM_MAX_BANK) {
		if (word >= RWTM_ROW_WORDS)
			return -EINVAL;
		return rwtm_otp_read(bank, word, val);
	} else if (bank == OTP_NB_BANK) {
		u32 data[OTP_NB_WORDS];
		if (word >= OTP_NB_WORDS)
			return -EINVAL;
		otp_read_parallel(OTP_NB_REG_BASE, data, OTP_NB_WORDS);
		*val = data[word];
		return 0;
	} else if (bank == OTP_SB_BANK) {
		u32 data[OTP_SB_WORDS];
		if (word >= OTP_SB_WORDS)
			return -EINVAL;
		otp_read_parallel(OTP_SB_REG_BASE, data, OTP_SB_WORDS);
		*val = data[word];
		return 0;
	} else {
		return -EINVAL;
	}
}

int fuse_prog(u32 bank, u32 word, u32 val)
{
	if (bank <= RWTM_MAX_BANK) {
		if (word >= RWTM_ROW_WORDS)
			return -EINVAL;
		return rwtm_otp_write(bank, word, val);
	} else if (bank == OTP_NB_BANK) {
		/* TODO: not implemented yet */
		return -ENOSYS;
	} else if (bank == OTP_SB_BANK) {
		/* TODO: not implemented yet */
		return -ENOSYS;
	} else {
		return -EINVAL;
	}
}

int fuse_sense(u32 bank, u32 word, u32 *val)
{
	/* not supported */
	return -ENOSYS;
}

int fuse_override(u32 bank, u32 word, u32 val)
{
	/* not supported */
	return -ENOSYS;
}
