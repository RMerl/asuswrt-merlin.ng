// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2013 ADVANSEE
 * Benoît Thébaudeau <benoit.thebaudeau@advansee.com>
 *
 * Based on Dirk Behme's
 * https://github.com/dirkbehme/u-boot-imx6/blob/28b17e9/drivers/misc/imx_otp.c,
 * which is based on Freescale's
 * https://source.codeaurora.org/external/imx/uboot-imx/tree/drivers/misc/imx_otp.c?id=9aa74e6,
 * which is:
 * Copyright (C) 2011 Freescale Semiconductor, Inc.
 */

#include <common.h>
#include <fuse.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/imx-regs.h>
#include <asm/mach-imx/sys_proto.h>

#define BO_CTRL_WR_UNLOCK		16
#define BM_CTRL_WR_UNLOCK		0xffff0000
#define BV_CTRL_WR_UNLOCK_KEY		0x3e77
#define BM_CTRL_ERROR			0x00000200
#define BM_CTRL_BUSY			0x00000100
#define BO_CTRL_ADDR			0
#ifdef CONFIG_MX7
#define BM_CTRL_ADDR                    0x0000000f
#define BM_CTRL_RELOAD                  0x00000400
#elif defined(CONFIG_MX7ULP)
#define BM_CTRL_ADDR                    0x000000FF
#define BM_CTRL_RELOAD                  0x00000400
#define BM_OUT_STATUS_DED				0x00000400
#define BM_OUT_STATUS_LOCKED			0x00000800
#define BM_OUT_STATUS_PROGFAIL			0x00001000
#elif defined(CONFIG_IMX8M)
#ifdef CONFIG_IMX8MP
#undef BM_CTRL_ADDR
#undef BM_CTRL_ERROR
#undef BM_CTRL_BUSY
#define BM_CTRL_ADDR			0x000001ff
#define BM_CTRL_ERROR			0x00000400
#define BM_CTRL_BUSY			0x00000200
#else
#define BM_CTRL_ADDR			0x000000ff
#endif
#else
#define BM_CTRL_ADDR			0x0000007f
#endif

#ifdef CONFIG_MX7
#define BO_TIMING_FSOURCE               12
#define BM_TIMING_FSOURCE               0x0007f000
#define BV_TIMING_FSOURCE_NS            1001
#define BO_TIMING_PROG                  0
#define BM_TIMING_PROG                  0x00000fff
#define BV_TIMING_PROG_US               10
#else
#define BO_TIMING_STROBE_READ		16
#define BM_TIMING_STROBE_READ		0x003f0000
#define BV_TIMING_STROBE_READ_NS	37
#define BO_TIMING_RELAX			12
#define BM_TIMING_RELAX			0x0000f000
#define BV_TIMING_RELAX_NS		17
#define BO_TIMING_STROBE_PROG		0
#define BM_TIMING_STROBE_PROG		0x00000fff
#define BV_TIMING_STROBE_PROG_US	10
#endif

#define BM_READ_CTRL_READ_FUSE		0x00000001

#define BF(value, field)		(((value) << BO_##field) & BM_##field)

#define WRITE_POSTAMBLE_US		2

#if defined(CONFIG_MX6) || defined(CONFIG_VF610)
#define FUSE_BANK_SIZE	0x80
#ifdef CONFIG_MX6SL
#define FUSE_BANKS	8
#elif defined(CONFIG_MX6ULL) || defined(CONFIG_MX6SLL)
#define FUSE_BANKS	9
#else
#define FUSE_BANKS	16
#endif
#elif defined CONFIG_MX7
#define FUSE_BANK_SIZE	0x40
#define FUSE_BANKS	16
#elif defined(CONFIG_MX7ULP)
#define FUSE_BANK_SIZE	0x80
#define FUSE_BANKS	31
#elif defined(CONFIG_IMX8M)
#define FUSE_BANK_SIZE	0x40
#ifdef CONFIG_IMX8MP
#define FUSE_BANKS	96
#else
#define FUSE_BANKS	64
#endif
#else
#error "Unsupported architecture\n"
#endif

#if defined(CONFIG_MX6)

/*
 * There is a hole in shadow registers address map of size 0x100
 * between bank 5 and bank 6 on iMX6QP, iMX6DQ, iMX6SDL, iMX6SX,
 * iMX6UL, i.MX6ULL and i.MX6SLL.
 * Bank 5 ends at 0x6F0 and Bank 6 starts at 0x800. When reading the fuses,
 * we should account for this hole in address space.
 *
 * Similar hole exists between bank 14 and bank 15 of size
 * 0x80 on iMX6QP, iMX6DQ, iMX6SDL and iMX6SX.
 * Note: iMX6SL has only 0-7 banks and there is no hole.
 * Note: iMX6UL doesn't have this one.
 *
 * This function is to covert user input to physical bank index.
 * Only needed when read fuse, because we use register offset, so
 * need to calculate real register offset.
 * When write, no need to consider hole, always use the bank/word
 * index from fuse map.
 */
u32 fuse_bank_physical(int index)
{
	u32 phy_index;

	if (is_mx6sl() || is_mx7ulp()) {
		phy_index = index;
	} else if (is_mx6ul() || is_mx6ull() || is_mx6sll()) {
		if ((is_mx6ull() || is_mx6sll()) && index == 8)
			index = 7;

		if (index >= 6)
			phy_index = fuse_bank_physical(5) + (index - 6) + 3;
		else
			phy_index = index;
	} else {
		if (index >= 15)
			phy_index = fuse_bank_physical(14) + (index - 15) + 2;
		else if (index >= 6)
			phy_index = fuse_bank_physical(5) + (index - 6) + 3;
		else
			phy_index = index;
	}
	return phy_index;
}

u32 fuse_word_physical(u32 bank, u32 word_index)
{
	if (is_mx6ull() || is_mx6sll()) {
		if (bank == 8)
			word_index = word_index + 4;
	}

	return word_index;
}
#else
u32 fuse_bank_physical(int index)
{
	return index;
}

u32 fuse_word_physical(u32 bank, u32 word_index)
{
	return word_index;
}

#endif

static void wait_busy(struct ocotp_regs *regs, unsigned int delay_us)
{
	while (readl(&regs->ctrl) & BM_CTRL_BUSY)
		udelay(delay_us);
}

static void clear_error(struct ocotp_regs *regs)
{
	writel(BM_CTRL_ERROR, &regs->ctrl_clr);
}

static int prepare_access(struct ocotp_regs **regs, u32 bank, u32 word,
				int assert, const char *caller)
{
	*regs = (struct ocotp_regs *)OCOTP_BASE_ADDR;

	if (bank >= FUSE_BANKS ||
	    word >= ARRAY_SIZE((*regs)->bank[0].fuse_regs) >> 2 ||
	    !assert) {
		printf("mxc_ocotp %s(): Invalid argument\n", caller);
		return -EINVAL;
	}

	if (is_mx6ull() || is_mx6sll()) {
		if ((bank == 7 || bank == 8) &&
		    word >= ARRAY_SIZE((*regs)->bank[0].fuse_regs) >> 3) {
			printf("mxc_ocotp %s(): Invalid argument\n", caller);
			return -EINVAL;
		}
	}

	enable_ocotp_clk(1);

	wait_busy(*regs, 1);
	clear_error(*regs);

	return 0;
}

static int finish_access(struct ocotp_regs *regs, const char *caller)
{
	u32 err;

	err = !!(readl(&regs->ctrl) & BM_CTRL_ERROR);
	clear_error(regs);

#ifdef CONFIG_MX7ULP
	/* Need to power down the OTP memory */
	writel(1, &regs->pdn);
#endif
	if (err) {
		printf("mxc_ocotp %s(): Access protect error\n", caller);
		return -EIO;
	}

	return 0;
}

static int prepare_read(struct ocotp_regs **regs, u32 bank, u32 word, u32 *val,
			const char *caller)
{
	return prepare_access(regs, bank, word, val != NULL, caller);
}

int fuse_read(u32 bank, u32 word, u32 *val)
{
	struct ocotp_regs *regs;
	int ret;
	u32 phy_bank;
	u32 phy_word;

	ret = prepare_read(&regs, bank, word, val, __func__);
	if (ret)
		return ret;

	phy_bank = fuse_bank_physical(bank);
	phy_word = fuse_word_physical(bank, word);

	*val = readl(&regs->bank[phy_bank].fuse_regs[phy_word << 2]);

#ifdef CONFIG_MX7ULP
	if (readl(&regs->out_status) & BM_OUT_STATUS_DED) {
		writel(BM_OUT_STATUS_DED, &regs->out_status_clr);
		printf("mxc_ocotp %s(): fuse read wrong\n", __func__);
		return -EIO;
	}
#endif
	return finish_access(regs, __func__);
}

#ifdef CONFIG_MX7
static void set_timing(struct ocotp_regs *regs)
{
	u32 ipg_clk;
	u32 fsource, prog;
	u32 timing;

	ipg_clk = mxc_get_clock(MXC_IPG_CLK);

	fsource = DIV_ROUND_UP((ipg_clk / 1000) * BV_TIMING_FSOURCE_NS,
			+       1000000) + 1;
	prog = DIV_ROUND_CLOSEST(ipg_clk * BV_TIMING_PROG_US, 1000000) + 1;

	timing = BF(fsource, TIMING_FSOURCE) | BF(prog, TIMING_PROG);

	clrsetbits_le32(&regs->timing, BM_TIMING_FSOURCE | BM_TIMING_PROG,
			timing);
}
#elif defined(CONFIG_MX7ULP)
static void set_timing(struct ocotp_regs *regs)
{
	/* No timing set for MX7ULP */
}

#else
static void set_timing(struct ocotp_regs *regs)
{
	u32 ipg_clk;
	u32 relax, strobe_read, strobe_prog;
	u32 timing;

	ipg_clk = mxc_get_clock(MXC_IPG_CLK);

	relax = DIV_ROUND_UP(ipg_clk * BV_TIMING_RELAX_NS, 1000000000) - 1;
	strobe_read = DIV_ROUND_UP(ipg_clk * BV_TIMING_STROBE_READ_NS,
					1000000000) + 2 * (relax + 1) - 1;
	strobe_prog = DIV_ROUND_CLOSEST(ipg_clk * BV_TIMING_STROBE_PROG_US,
						1000000) + 2 * (relax + 1) - 1;

	timing = BF(strobe_read, TIMING_STROBE_READ) |
			BF(relax, TIMING_RELAX) |
			BF(strobe_prog, TIMING_STROBE_PROG);

	clrsetbits_le32(&regs->timing, BM_TIMING_STROBE_READ | BM_TIMING_RELAX |
			BM_TIMING_STROBE_PROG, timing);
}
#endif

static void setup_direct_access(struct ocotp_regs *regs, u32 bank, u32 word,
				int write)
{
	u32 wr_unlock = write ? BV_CTRL_WR_UNLOCK_KEY : 0;
#ifdef CONFIG_MX7
	u32 addr = bank;
#elif defined CONFIG_IMX8M
	u32 addr = bank << 2 | word;
#else
	u32 addr;
	/* Bank 7 and Bank 8 only supports 4 words each for i.MX6ULL */
	if ((is_mx6ull() || is_mx6sll()) && (bank > 7)) {
		bank = bank - 1;
		word += 4;
	}
	addr = bank << 3 | word;
#endif

	set_timing(regs);
	clrsetbits_le32(&regs->ctrl, BM_CTRL_WR_UNLOCK | BM_CTRL_ADDR,
			BF(wr_unlock, CTRL_WR_UNLOCK) |
			BF(addr, CTRL_ADDR));
}

int fuse_sense(u32 bank, u32 word, u32 *val)
{
	struct ocotp_regs *regs;
	int ret;

	if (is_imx8mq() && (soc_rev() >= CHIP_REV_2_1)) {
		printf("mxc_ocotp %s(): fuse sense is disabled\n", __func__);
		return -EPERM;
	}

	ret = prepare_read(&regs, bank, word, val, __func__);
	if (ret)
		return ret;

	setup_direct_access(regs, bank, word, false);
	writel(BM_READ_CTRL_READ_FUSE, &regs->read_ctrl);
	wait_busy(regs, 1);
#ifdef CONFIG_MX7
	*val = readl((&regs->read_fuse_data0) + (word << 2));
#else
	*val = readl(&regs->read_fuse_data);
#endif

#ifdef CONFIG_MX7ULP
	if (readl(&regs->out_status) & BM_OUT_STATUS_DED) {
		writel(BM_OUT_STATUS_DED, &regs->out_status_clr);
		printf("mxc_ocotp %s(): fuse read wrong\n", __func__);
		return -EIO;
	}
#endif

	return finish_access(regs, __func__);
}

static int prepare_write(struct ocotp_regs **regs, u32 bank, u32 word,
				const char *caller)
{
#ifdef CONFIG_MX7ULP
	u32 val;
	int ret;

	/* Only bank 0 and 1 are redundancy mode, others are ECC mode */
	if (bank != 0 && bank != 1) {
		if ((soc_rev() < CHIP_REV_2_0) ||
		    ((soc_rev() >= CHIP_REV_2_0) &&
		    bank != 9 && bank != 10 && bank != 28)) {
			ret = fuse_sense(bank, word, &val);
			if (ret)
				return ret;

			if (val != 0) {
				printf("mxc_ocotp: The word has been programmed, no more write\n");
				return -EPERM;
			}
		}
	}
#endif

	return prepare_access(regs, bank, word, true, caller);
}

int fuse_prog(u32 bank, u32 word, u32 val)
{
	struct ocotp_regs *regs;
	int ret;

	ret = prepare_write(&regs, bank, word, __func__);
	if (ret)
		return ret;

	setup_direct_access(regs, bank, word, true);
#ifdef CONFIG_MX7
	switch (word) {
	case 0:
		writel(0, &regs->data1);
		writel(0, &regs->data2);
		writel(0, &regs->data3);
		writel(val, &regs->data0);
		break;
	case 1:
		writel(val, &regs->data1);
		writel(0, &regs->data2);
		writel(0, &regs->data3);
		writel(0, &regs->data0);
		break;
	case 2:
		writel(0, &regs->data1);
		writel(val, &regs->data2);
		writel(0, &regs->data3);
		writel(0, &regs->data0);
		break;
	case 3:
		writel(0, &regs->data1);
		writel(0, &regs->data2);
		writel(val, &regs->data3);
		writel(0, &regs->data0);
		break;
	}
	wait_busy(regs, BV_TIMING_PROG_US);
#else
	writel(val, &regs->data);
	wait_busy(regs, BV_TIMING_STROBE_PROG_US);
#endif
	udelay(WRITE_POSTAMBLE_US);

#ifdef CONFIG_MX7ULP
	if (readl(&regs->out_status) & (BM_OUT_STATUS_PROGFAIL | BM_OUT_STATUS_LOCKED)) {
		writel((BM_OUT_STATUS_PROGFAIL | BM_OUT_STATUS_LOCKED), &regs->out_status_clr);
		printf("mxc_ocotp %s(): fuse write is failed\n", __func__);
		return -EIO;
	}
#endif

	return finish_access(regs, __func__);
}

int fuse_override(u32 bank, u32 word, u32 val)
{
	struct ocotp_regs *regs;
	int ret;
	u32 phy_bank;
	u32 phy_word;

	ret = prepare_write(&regs, bank, word, __func__);
	if (ret)
		return ret;

	phy_bank = fuse_bank_physical(bank);
	phy_word = fuse_word_physical(bank, word);

	writel(val, &regs->bank[phy_bank].fuse_regs[phy_word << 2]);

#ifdef CONFIG_MX7ULP
	if (readl(&regs->out_status) & (BM_OUT_STATUS_PROGFAIL | BM_OUT_STATUS_LOCKED)) {
		writel((BM_OUT_STATUS_PROGFAIL | BM_OUT_STATUS_LOCKED), &regs->out_status_clr);
		printf("mxc_ocotp %s(): fuse write is failed\n", __func__);
		return -EIO;
	}
#endif

	return finish_access(regs, __func__);
}
