// SPDX-License-Identifier: GPL-2.0+
/*
 * Freescale i.MX28 OCOTP Driver
 *
 * Copyright (C) 2014 Marek Vasut <marex@denx.de>
 *
 * Note: The i.MX23/i.MX28 OCOTP block is a predecessor to the OCOTP block
 *       used in i.MX6 . While these blocks are very similar at the first
 *       glance, by digging deeper, one will notice differences (like the
 *       tight dependence on MXS power block, some completely new registers
 *       etc.) which would make common driver an ifdef nightmare :-(
 */

#include <common.h>
#include <fuse.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/sys_proto.h>

#define MXS_OCOTP_TIMEOUT	100000

static struct mxs_ocotp_regs *ocotp_regs =
	(struct mxs_ocotp_regs *)MXS_OCOTP_BASE;
static struct mxs_power_regs *power_regs =
	(struct mxs_power_regs *)MXS_POWER_BASE;
static struct mxs_clkctrl_regs *clkctrl_regs =
	(struct mxs_clkctrl_regs *)MXS_CLKCTRL_BASE;

static int mxs_ocotp_wait_busy_clear(void)
{
	uint32_t reg;
	int timeout = MXS_OCOTP_TIMEOUT;

	while (--timeout) {
		reg = readl(&ocotp_regs->hw_ocotp_ctrl);
		if (!(reg & OCOTP_CTRL_BUSY))
			break;
		udelay(10);
	}

	if (!timeout)
		return -EINVAL;

	/* Wait a little as per FSL datasheet's 'write postamble' section. */
	udelay(10);

	return 0;
}

static void mxs_ocotp_clear_error(void)
{
	writel(OCOTP_CTRL_ERROR, &ocotp_regs->hw_ocotp_ctrl_clr);
}

static int mxs_ocotp_read_bank_open(bool open)
{
	int ret = 0;

	if (open) {
		writel(OCOTP_CTRL_RD_BANK_OPEN,
		       &ocotp_regs->hw_ocotp_ctrl_set);

		/*
		 * Wait before polling the BUSY bit, since the BUSY bit might
		 * be asserted only after a few HCLK cycles and if we were to
		 * poll immediatelly, we could miss the busy bit.
		 */
		udelay(10);
		ret = mxs_ocotp_wait_busy_clear();
	} else {
		writel(OCOTP_CTRL_RD_BANK_OPEN,
		       &ocotp_regs->hw_ocotp_ctrl_clr);
	}

	return ret;
}

static void mxs_ocotp_scale_vddio(bool enter, uint32_t *val)
{
	uint32_t scale_val;

	if (enter) {
		/*
		 * Enter the fuse programming VDDIO voltage setup. We start
		 * scaling the voltage from it's current value down to 2.8V
		 * which is the one and only correct voltage for programming
		 * the OCOTP fuses (according to datasheet).
		 */
		scale_val = readl(&power_regs->hw_power_vddioctrl);
		scale_val &= POWER_VDDIOCTRL_TRG_MASK;

		/* Return the original voltage. */
		*val = scale_val;

		/*
		 * Start scaling VDDIO down to 0x2, which is 2.8V . Actually,
		 * the value 0x0 should be 2.8V, but that's not the case on
		 * most designs due to load etc., so we play safe. Undervolt
		 * can actually cause incorrect programming of the fuses and
		 * or reboots of the board.
		 */
		while (scale_val > 2) {
			clrsetbits_le32(&power_regs->hw_power_vddioctrl,
					POWER_VDDIOCTRL_TRG_MASK, --scale_val);
			udelay(500);
		}
	} else {
		/* Start scaling VDDIO up to original value . */
		for (scale_val = 2; scale_val <= *val; scale_val++) {
			clrsetbits_le32(&power_regs->hw_power_vddioctrl,
					POWER_VDDIOCTRL_TRG_MASK, scale_val);
			udelay(500);
		}
	}

	mdelay(10);
}

static int mxs_ocotp_wait_hclk_ready(void)
{
	uint32_t reg, timeout = MXS_OCOTP_TIMEOUT;

	while (--timeout) {
		reg = readl(&clkctrl_regs->hw_clkctrl_hbus);
		if (!(reg & CLKCTRL_HBUS_ASM_BUSY))
			break;
	}

	if (!timeout)
		return -EINVAL;

	return 0;
}

static int mxs_ocotp_scale_hclk(bool enter, uint32_t *val)
{
	uint32_t scale_val;
	int ret;

	ret = mxs_ocotp_wait_hclk_ready();
	if (ret)
		return ret;

	/* Set CPU bypass */
	writel(CLKCTRL_CLKSEQ_BYPASS_CPU,
	       &clkctrl_regs->hw_clkctrl_clkseq_set);

	if (enter) {
		/* Return the original HCLK clock speed. */
		*val = readl(&clkctrl_regs->hw_clkctrl_hbus);
		*val &= CLKCTRL_HBUS_DIV_MASK;
		*val >>= CLKCTRL_HBUS_DIV_OFFSET;

		/* Scale the HCLK to 454/19 = 23.9 MHz . */
		scale_val = (~19) << CLKCTRL_HBUS_DIV_OFFSET;
		scale_val &= CLKCTRL_HBUS_DIV_MASK;
	} else {
		/* Scale the HCLK back to original frequency. */
		scale_val = (~(*val)) << CLKCTRL_HBUS_DIV_OFFSET;
		scale_val &= CLKCTRL_HBUS_DIV_MASK;
	}

	writel(CLKCTRL_HBUS_DIV_MASK,
	       &clkctrl_regs->hw_clkctrl_hbus_set);
	writel(scale_val,
	       &clkctrl_regs->hw_clkctrl_hbus_clr);

	mdelay(10);

	ret = mxs_ocotp_wait_hclk_ready();
	if (ret)
		return ret;

	/* Disable CPU bypass */
	writel(CLKCTRL_CLKSEQ_BYPASS_CPU,
	       &clkctrl_regs->hw_clkctrl_clkseq_clr);

	mdelay(10);

	return 0;
}

static int mxs_ocotp_write_fuse(uint32_t addr, uint32_t mask)
{
	uint32_t hclk_val, vddio_val;
	int ret;

	mxs_ocotp_clear_error();

	/* Make sure the banks are closed for reading. */
	ret = mxs_ocotp_read_bank_open(0);
	if (ret) {
		puts("Failed closing banks for reading!\n");
		return ret;
	}

	ret = mxs_ocotp_scale_hclk(1, &hclk_val);
	if (ret) {
		puts("Failed scaling down the HCLK!\n");
		return ret;
	}
	mxs_ocotp_scale_vddio(1, &vddio_val);

	ret = mxs_ocotp_wait_busy_clear();
	if (ret) {
		puts("Failed waiting for ready state!\n");
		goto fail;
	}

	/* Program the fuse address */
	writel(addr | OCOTP_CTRL_WR_UNLOCK_KEY, &ocotp_regs->hw_ocotp_ctrl);

	/* Program the data. */
	writel(mask, &ocotp_regs->hw_ocotp_data);

	udelay(10);

	ret = mxs_ocotp_wait_busy_clear();
	if (ret) {
		puts("Failed waiting for ready state!\n");
		goto fail;
	}

	/* Check for errors */
	if (readl(&ocotp_regs->hw_ocotp_ctrl) & OCOTP_CTRL_ERROR) {
		puts("Failed writing fuses!\n");
		ret = -EPERM;
		goto fail;
	}

fail:
	mxs_ocotp_scale_vddio(0, &vddio_val);
	if (mxs_ocotp_scale_hclk(0, &hclk_val))
		puts("Failed scaling up the HCLK!\n");

	return ret;
}

static int mxs_ocotp_read_fuse(uint32_t reg, uint32_t *val)
{
	int ret;

	/* Register offset from CUST0 */
	reg = ((uint32_t)&ocotp_regs->hw_ocotp_cust0) + (reg << 4);

	ret = mxs_ocotp_wait_busy_clear();
	if (ret) {
		puts("Failed waiting for ready state!\n");
		return ret;
	}

	mxs_ocotp_clear_error();

	ret = mxs_ocotp_read_bank_open(1);
	if (ret) {
		puts("Failed opening banks for reading!\n");
		return ret;
	}

	*val = readl(reg);

	ret = mxs_ocotp_read_bank_open(0);
	if (ret) {
		puts("Failed closing banks for reading!\n");
		return ret;
	}

	return ret;
}

static int mxs_ocotp_valid(u32 bank, u32 word)
{
	if (bank > 4)
		return -EINVAL;
	if (word > 7)
		return -EINVAL;
	return 0;
}

/*
 * The 'fuse' command API
 */
int fuse_read(u32 bank, u32 word, u32 *val)
{
	int ret;

	ret = mxs_ocotp_valid(bank, word);
	if (ret)
		return ret;

	return mxs_ocotp_read_fuse((bank << 3) | word, val);
}

int fuse_prog(u32 bank, u32 word, u32 val)
{
	int ret;

	ret = mxs_ocotp_valid(bank, word);
	if (ret)
		return ret;

	return mxs_ocotp_write_fuse((bank << 3) | word, val);
}

int fuse_sense(u32 bank, u32 word, u32 *val)
{
	/* We do not support sensing :-( */
	return -EINVAL;
}

int fuse_override(u32 bank, u32 word, u32 val)
{
	/* We do not support overriding :-( */
	return -EINVAL;
}
