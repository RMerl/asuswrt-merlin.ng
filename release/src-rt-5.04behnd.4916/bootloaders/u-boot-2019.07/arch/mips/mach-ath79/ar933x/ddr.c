// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2015-2016 Wills Wang <wills.wang@live.com>
 * Based on Atheros LSDK/QSDK
 */

#include <common.h>
#include <asm/io.h>
#include <asm/addrspace.h>
#include <asm/types.h>
#include <mach/ar71xx_regs.h>
#include <mach/ath79.h>

#define DDR_CTRL_UPD_EMR3S      BIT(5)
#define DDR_CTRL_UPD_EMR2S      BIT(4)
#define DDR_CTRL_PRECHARGE      BIT(3)
#define DDR_CTRL_AUTO_REFRESH   BIT(2)
#define DDR_CTRL_UPD_EMRS       BIT(1)
#define DDR_CTRL_UPD_MRS        BIT(0)

#define DDR_REFRESH_EN          BIT(14)
#define DDR_REFRESH_M           0x3ff
#define DDR_REFRESH(x)          ((x) & 0x3ff)
#define DDR_REFRESH_VAL_25M     (DDR_REFRESH_EN | DDR_REFRESH(390))
#define DDR_REFRESH_VAL_40M     (DDR_REFRESH_EN | DDR_REFRESH(624))

#define DDR_TRAS_S              0
#define DDR_TRAS_M              0x1f
#define DDR_TRAS(x)             ((x) << DDR_TRAS_S)
#define DDR_TRCD_M              0xf
#define DDR_TRCD_S              5
#define DDR_TRCD(x)             ((x) << DDR_TRCD_S)
#define DDR_TRP_M               0xf
#define DDR_TRP_S               9
#define DDR_TRP(x)              ((x) << DDR_TRP_S)
#define DDR_TRRD_M              0xf
#define DDR_TRRD_S              13
#define DDR_TRRD(x)             ((x) << DDR_TRRD_S)
#define DDR_TRFC_M              0x7f
#define DDR_TRFC_S              17
#define DDR_TRFC(x)             ((x) << DDR_TRFC_S)
#define DDR_TMRD_M              0xf
#define DDR_TMRD_S              23
#define DDR_TMRD(x)             ((x) << DDR_TMRD_S)
#define DDR_CAS_L_M             0x17
#define DDR_CAS_L_S             27
#define DDR_CAS_L(x)            (((x) & DDR_CAS_L_M) << DDR_CAS_L_S)
#define DDR_OPEN                BIT(30)
#define DDR_CONF_REG_VAL        (DDR_TRAS(16) | DDR_TRCD(6) | \
				 DDR_TRP(6) | DDR_TRRD(4) | \
				 DDR_TRFC(30) | DDR_TMRD(15) | \
				 DDR_CAS_L(7) | DDR_OPEN)

#define DDR_BURST_LEN_S         0
#define DDR_BURST_LEN_M         0xf
#define DDR_BURST_LEN(x)        ((x) << DDR_BURST_LEN_S)
#define DDR_BURST_TYPE          BIT(4)
#define DDR_CNTL_OE_EN          BIT(5)
#define DDR_PHASE_SEL           BIT(6)
#define DDR_CKE                 BIT(7)
#define DDR_TWR_S               8
#define DDR_TWR_M               0xf
#define DDR_TWR(x)              ((x) << DDR_TWR_S)
#define DDR_TRTW_S              12
#define DDR_TRTW_M              0x1f
#define DDR_TRTW(x)             ((x) << DDR_TRTW_S)
#define DDR_TRTP_S              17
#define DDR_TRTP_M              0xf
#define DDR_TRTP(x)             ((x) << DDR_TRTP_S)
#define DDR_TWTR_S              21
#define DDR_TWTR_M              0x1f
#define DDR_TWTR(x)             ((x) << DDR_TWTR_S)
#define DDR_G_OPEN_L_S          26
#define DDR_G_OPEN_L_M          0xf
#define DDR_G_OPEN_L(x)         ((x) << DDR_G_OPEN_L_S)
#define DDR_HALF_WIDTH_LOW      BIT(31)
#define DDR_CONF2_REG_VAL       (DDR_BURST_LEN(8) | DDR_CNTL_OE_EN | \
				 DDR_CKE | DDR_TWR(6) | DDR_TRTW(14) | \
				 DDR_TRTP(8) | DDR_TWTR(14) | \
				 DDR_G_OPEN_L(7) | DDR_HALF_WIDTH_LOW)

#define DDR2_CONF_TWL_S         10
#define DDR2_CONF_TWL_M         0xf
#define DDR2_CONF_TWL(x)        (((x) & DDR2_CONF_TWL_M) << DDR2_CONF_TWL_S)
#define DDR2_CONF_ODT           BIT(9)
#define DDR2_CONF_TFAW_S        2
#define DDR2_CONF_TFAW_M        0x3f
#define DDR2_CONF_TFAW(x)       (((x) & DDR2_CONF_TFAW_M) << DDR2_CONF_TFAW_S)
#define DDR2_CONF_EN            BIT(0)
#define DDR2_CONF_VAL           (DDR2_CONF_TWL(2) | DDR2_CONF_ODT | \
				 DDR2_CONF_TFAW(22) | DDR2_CONF_EN)

#define DDR1_EXT_MODE_VAL       0x02
#define DDR2_EXT_MODE_VAL       0x402
#define DDR2_EXT_MODE_OCD_VAL   0x382
#define DDR1_MODE_DLL_VAL       0x133
#define DDR2_MODE_DLL_VAL       0x100
#define DDR1_MODE_VAL           0x33
#define DDR2_MODE_VAL           0xa33
#define DDR_TAP_VAL0            0x08
#define DDR_TAP_VAL1            0x09

void ddr_init(void)
{
	void __iomem *regs;
	u32 val;

	regs = map_physmem(AR71XX_DDR_CTRL_BASE, AR71XX_DDR_CTRL_SIZE,
			   MAP_NOCACHE);

	writel(DDR_CONF_REG_VAL, regs + AR71XX_DDR_REG_CONFIG);
	writel(DDR_CONF2_REG_VAL, regs + AR71XX_DDR_REG_CONFIG2);

	val = ath79_get_bootstrap();
	if (val & AR933X_BOOTSTRAP_DDR2) {
		/* AHB maximum timeout */
		writel(0xfffff, regs + AR933X_DDR_REG_TIMEOUT_MAX);

		/* Enable DDR2 */
		writel(DDR2_CONF_VAL, regs + AR933X_DDR_REG_DDR2_CONFIG);

		/* Precharge All */
		writel(DDR_CTRL_PRECHARGE, regs + AR71XX_DDR_REG_CONTROL);

		/* Disable High Temperature Self-Refresh, Full Array */
		writel(0x00, regs + AR933X_DDR_REG_EMR2);

		/* Extended Mode Register 2 Set (EMR2S) */
		writel(DDR_CTRL_UPD_EMR2S, regs + AR71XX_DDR_REG_CONTROL);

		writel(0x00, regs + AR933X_DDR_REG_EMR3);

		/* Extended Mode Register 3 Set (EMR3S) */
		writel(DDR_CTRL_UPD_EMR3S, regs + AR71XX_DDR_REG_CONTROL);

		/* Enable DLL,  Full strength, ODT Disabled */
		writel(0x00, regs + AR71XX_DDR_REG_EMR);

		/* Extended Mode Register Set (EMRS) */
		writel(DDR_CTRL_UPD_EMRS, regs + AR71XX_DDR_REG_CONTROL);

		/* Reset DLL */
		writel(DDR2_MODE_DLL_VAL, regs + AR71XX_DDR_REG_MODE);

		/* Mode Register Set (MRS) */
		writel(DDR_CTRL_UPD_MRS, regs + AR71XX_DDR_REG_CONTROL);

		/* Precharge All */
		writel(DDR_CTRL_PRECHARGE, regs + AR71XX_DDR_REG_CONTROL);

		/* Auto Refresh */
		writel(DDR_CTRL_AUTO_REFRESH, regs + AR71XX_DDR_REG_CONTROL);
		writel(DDR_CTRL_AUTO_REFRESH, regs + AR71XX_DDR_REG_CONTROL);

		/* Write recovery (WR) 6 clock, CAS Latency 3, Burst Length 8 */
		writel(DDR2_MODE_VAL, regs + AR71XX_DDR_REG_MODE);
		/* Mode Register Set (MRS) */
		writel(DDR_CTRL_UPD_MRS, regs + AR71XX_DDR_REG_CONTROL);

		/* Enable OCD defaults, Enable DLL, Reduced Drive Strength */
		writel(DDR2_EXT_MODE_OCD_VAL, regs + AR71XX_DDR_REG_EMR);

		/* Extended Mode Register Set (EMRS) */
		writel(DDR_CTRL_UPD_EMRS, regs + AR71XX_DDR_REG_CONTROL);

		/* OCD exit, Enable DLL, Enable /DQS, Reduced Drive Strength */
		writel(DDR2_EXT_MODE_VAL, regs + AR71XX_DDR_REG_EMR);
		/* Extended Mode Register Set (EMRS) */
		writel(DDR_CTRL_UPD_EMRS, regs + AR71XX_DDR_REG_CONTROL);

		/* Refresh time control */
		if (val & AR933X_BOOTSTRAP_REF_CLK_40)
			writel(DDR_REFRESH_VAL_40M, regs +
			       AR71XX_DDR_REG_REFRESH);
		else
			writel(DDR_REFRESH_VAL_25M, regs +
			       AR71XX_DDR_REG_REFRESH);

		/* DQS 0 Tap Control */
		writel(DDR_TAP_VAL0, regs + AR71XX_DDR_REG_TAP_CTRL0);

		/* DQS 1 Tap Control */
		writel(DDR_TAP_VAL1, regs + AR71XX_DDR_REG_TAP_CTRL1);

		/* For 16-bit DDR */
		writel(0xff, regs + AR71XX_DDR_REG_RD_CYCLE);
	} else {
		/* AHB maximum timeout */
		writel(0xfffff, regs + AR933X_DDR_REG_TIMEOUT_MAX);

		/* Precharge All */
		writel(DDR_CTRL_PRECHARGE, regs + AR71XX_DDR_REG_CONTROL);

		/* Reset DLL, Burst Length 8, CAS Latency 3 */
		writel(DDR1_MODE_DLL_VAL, regs + AR71XX_DDR_REG_MODE);

		/* Forces an MRS update cycle in DDR */
		writel(DDR_CTRL_UPD_MRS, regs + AR71XX_DDR_REG_CONTROL);

		/* Enable DLL, Full strength */
		writel(DDR1_EXT_MODE_VAL, regs + AR71XX_DDR_REG_EMR);

		/* Extended Mode Register Set (EMRS) */
		writel(DDR_CTRL_UPD_EMRS, regs + AR71XX_DDR_REG_CONTROL);

		/* Precharge All */
		writel(DDR_CTRL_PRECHARGE, regs + AR71XX_DDR_REG_CONTROL);

		/* Normal DLL, Burst Length 8, CAS Latency 3 */
		writel(DDR1_MODE_VAL, regs + AR71XX_DDR_REG_MODE);

		/* Mode Register Set (MRS) */
		writel(DDR_CTRL_UPD_MRS, regs + AR71XX_DDR_REG_CONTROL);

		/* Refresh time control */
		if (val & AR933X_BOOTSTRAP_REF_CLK_40)
			writel(DDR_REFRESH_VAL_40M, regs +
			       AR71XX_DDR_REG_REFRESH);
		else
			writel(DDR_REFRESH_VAL_25M, regs +
			       AR71XX_DDR_REG_REFRESH);

		/* DQS 0 Tap Control */
		writel(DDR_TAP_VAL0, regs + AR71XX_DDR_REG_TAP_CTRL0);

		/* DQS 1 Tap Control */
		writel(DDR_TAP_VAL1, regs + AR71XX_DDR_REG_TAP_CTRL1);

		/* For 16-bit DDR */
		writel(0xff, regs + AR71XX_DDR_REG_RD_CYCLE);
	}
}

void ddr_tap_tuning(void)
{
	void __iomem *regs;
	u32 *addr_k0, *addr_k1, *addr;
	u32 val, tap, upper, lower;
	int i, j, dir, err, done;

	regs = map_physmem(AR71XX_DDR_CTRL_BASE, AR71XX_DDR_CTRL_SIZE,
			   MAP_NOCACHE);

	/* Init memory pattern */
	addr = (void *)CKSEG0ADDR(0x2000);
	for (i = 0; i < 256; i++) {
		val = 0;
		for (j = 0; j < 8; j++) {
			if (i & (1 << j)) {
				if (j % 2)
					val |= 0xffff0000;
				else
					val |= 0x0000ffff;
			}

			if (j % 2) {
				*addr++ = val;
				val = 0;
			}
		}
	}

	err = 0;
	done = 0;
	dir = 1;
	tap = readl(regs + AR71XX_DDR_REG_TAP_CTRL0);
	val = tap;
	upper = tap;
	lower = tap;
	while (!done) {
		err = 0;

		/* Update new DDR tap value */
		writel(val, regs + AR71XX_DDR_REG_TAP_CTRL0);
		writel(val, regs + AR71XX_DDR_REG_TAP_CTRL1);

		/* Compare DDR with cache */
		for (i = 0; i < 2; i++) {
			addr_k1 = (void *)CKSEG1ADDR(0x2000);
			addr_k0 = (void *)CKSEG0ADDR(0x2000);
			addr = (void *)CKSEG0ADDR(0x3000);

			while (addr_k0 < addr) {
				if (*addr_k1++ != *addr_k0++) {
					err = 1;
					break;
				}
			}

			if (err)
				break;
		}

		if (err) {
			/* Save upper/lower threshold if error  */
			if (dir) {
				dir = 0;
				val--;
				upper = val;
				val = tap;
			} else {
				val++;
				lower = val;
				done = 1;
			}
		} else {
			/* Try the next value until limitation */
			if (dir) {
				if (val < 0x20) {
					val++;
				} else {
					dir = 0;
					upper = val;
					val = tap;
				}
			} else {
				if (!val) {
					lower = val;
					done = 1;
				} else {
					val--;
				}
			}
		}
	}

	/* compute an intermediate value and write back */
	val = (upper + lower) / 2;
	writel(val, regs + AR71XX_DDR_REG_TAP_CTRL0);
	val++;
	writel(val, regs + AR71XX_DDR_REG_TAP_CTRL1);
}
