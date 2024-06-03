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
#define DDR_REFRESH(x)          ((x) & DDR_REFRESH_M)
#define DDR_REFRESH_VAL         (DDR_REFRESH_EN | DDR_REFRESH(312))

#define DDR_TRAS_S              0
#define DDR_TRAS_M              0x1f
#define DDR_TRAS(x)             (((x) & DDR_TRAS_M) << DDR_TRAS_S)
#define DDR_TRCD_M              0xf
#define DDR_TRCD_S              5
#define DDR_TRCD(x)             (((x) & DDR_TRCD_M) << DDR_TRCD_S)
#define DDR_TRP_M               0xf
#define DDR_TRP_S               9
#define DDR_TRP(x)              (((x) & DDR_TRP_M) << DDR_TRP_S)
#define DDR_TRRD_M              0xf
#define DDR_TRRD_S              13
#define DDR_TRRD(x)             (((x) & DDR_TRRD_M) << DDR_TRRD_S)
#define DDR_TRFC_M              0x7f
#define DDR_TRFC_S              17
#define DDR_TRFC(x)             (((x) & DDR_TRFC_M) << DDR_TRFC_S)
#define DDR_TMRD_M              0xf
#define DDR_TMRD_S              23
#define DDR_TMRD(x)             (((x) & DDR_TMRD_M) << DDR_TMRD_S)
#define DDR_CAS_L_M             0x17
#define DDR_CAS_L_S             27
#define DDR_CAS_L(x)            (((x) & DDR_CAS_L_M) << DDR_CAS_L_S)
#define DDR_OPEN                BIT(30)
#define DDR1_CONF_REG_VAL       (DDR_TRAS(16) | DDR_TRCD(6) | \
				 DDR_TRP(6) | DDR_TRRD(4) | \
				 DDR_TRFC(7) | DDR_TMRD(5) | \
				 DDR_CAS_L(7) | DDR_OPEN)
#define DDR2_CONF_REG_VAL       (DDR_TRAS(27) | DDR_TRCD(9) | \
				 DDR_TRP(9) | DDR_TRRD(7) | \
				 DDR_TRFC(21) | DDR_TMRD(15) | \
				 DDR_CAS_L(17) | DDR_OPEN)

#define DDR_BURST_LEN_S         0
#define DDR_BURST_LEN_M         0xf
#define DDR_BURST_LEN(x)        ((x) << DDR_BURST_LEN_S)
#define DDR_BURST_TYPE          BIT(4)
#define DDR_CNTL_OE_EN          BIT(5)
#define DDR_PHASE_SEL           BIT(6)
#define DDR_CKE                 BIT(7)
#define DDR_TWR_S               8
#define DDR_TWR_M               0xf
#define DDR_TWR(x)              (((x) & DDR_TWR_M) << DDR_TWR_S)
#define DDR_TRTW_S              12
#define DDR_TRTW_M              0x1f
#define DDR_TRTW(x)             (((x) & DDR_TRTW_M) << DDR_TRTW_S)
#define DDR_TRTP_S              17
#define DDR_TRTP_M              0xf
#define DDR_TRTP(x)             (((x) & DDR_TRTP_M) << DDR_TRTP_S)
#define DDR_TWTR_S              21
#define DDR_TWTR_M              0x1f
#define DDR_TWTR(x)             (((x) & DDR_TWTR_M) << DDR_TWTR_S)
#define DDR_G_OPEN_L_S          26
#define DDR_G_OPEN_L_M          0xf
#define DDR_G_OPEN_L(x)         ((x) << DDR_G_OPEN_L_S)
#define DDR_HALF_WIDTH_LOW      BIT(31)
#define DDR1_CONF2_REG_VAL      (DDR_BURST_LEN(8) | DDR_CNTL_OE_EN | \
				 DDR_CKE | DDR_TWR(13) | DDR_TRTW(14) | \
				 DDR_TRTP(8) | DDR_TWTR(14) | \
				 DDR_G_OPEN_L(6) | DDR_HALF_WIDTH_LOW)
#define DDR2_CONF2_REG_VAL      (DDR_BURST_LEN(8) | DDR_CNTL_OE_EN | \
				 DDR_CKE | DDR_TWR(1) | DDR_TRTW(14) | \
				 DDR_TRTP(9) | DDR_TWTR(21) | \
				 DDR_G_OPEN_L(8) | DDR_HALF_WIDTH_LOW)

#define DDR_TWR_MSB             BIT(3)
#define DDR_TRAS_MSB            BIT(2)
#define DDR_TRFC_MSB_M          0x3
#define DDR_TRFC_MSB(x)         (x)
#define DDR1_CONF3_REG_VAL      0
#define DDR2_CONF3_REG_VAL      (DDR_TWR_MSB | DDR_TRFC_MSB(2))

#define DDR_CTL_SRAM_TSEL       BIT(30)
#define DDR_CTL_SRAM_GE0_SYNC   BIT(20)
#define DDR_CTL_SRAM_GE1_SYNC   BIT(19)
#define DDR_CTL_SRAM_USB_SYNC   BIT(18)
#define DDR_CTL_SRAM_PCIE_SYNC  BIT(17)
#define DDR_CTL_SRAM_WMAC_SYNC  BIT(16)
#define DDR_CTL_SRAM_MISC1_SYNC BIT(15)
#define DDR_CTL_SRAM_MISC2_SYNC BIT(14)
#define DDR_CTL_PAD_DDR2_SEL    BIT(6)
#define DDR_CTL_HALF_WIDTH      BIT(1)
#define DDR_CTL_CONFIG_VAL      (DDR_CTL_SRAM_TSEL | \
				 DDR_CTL_SRAM_GE0_SYNC | \
				 DDR_CTL_SRAM_GE1_SYNC | \
				 DDR_CTL_SRAM_USB_SYNC | \
				 DDR_CTL_SRAM_PCIE_SYNC | \
				 DDR_CTL_SRAM_WMAC_SYNC | \
				 DDR_CTL_HALF_WIDTH)

#define DDR_BURST_GE0_MAX_BL_S  0
#define DDR_BURST_GE0_MAX_BL_M  0xf
#define DDR_BURST_GE0_MAX_BL(x) \
	(((x) & DDR_BURST_GE0_MAX_BL_M) << DDR_BURST_GE0_MAX_BL_S)
#define DDR_BURST_GE1_MAX_BL_S  4
#define DDR_BURST_GE1_MAX_BL_M  0xf
#define DDR_BURST_GE1_MAX_BL(x) \
	(((x) & DDR_BURST_GE1_MAX_BL_M) << DDR_BURST_GE1_MAX_BL_S)
#define DDR_BURST_PCIE_MAX_BL_S 8
#define DDR_BURST_PCIE_MAX_BL_M 0xf
#define DDR_BURST_PCIE_MAX_BL(x) \
	(((x) & DDR_BURST_PCIE_MAX_BL_M) << DDR_BURST_PCIE_MAX_BL_S)
#define DDR_BURST_USB_MAX_BL_S  12
#define DDR_BURST_USB_MAX_BL_M  0xf
#define DDR_BURST_USB_MAX_BL(x) \
	(((x) & DDR_BURST_USB_MAX_BL_M) << DDR_BURST_USB_MAX_BL_S)
#define DDR_BURST_CPU_MAX_BL_S  16
#define DDR_BURST_CPU_MAX_BL_M  0xf
#define DDR_BURST_CPU_MAX_BL(x) \
	(((x) & DDR_BURST_CPU_MAX_BL_M) << DDR_BURST_CPU_MAX_BL_S)
#define DDR_BURST_RD_MAX_BL_S   20
#define DDR_BURST_RD_MAX_BL_M   0xf
#define DDR_BURST_RD_MAX_BL(x) \
	(((x) & DDR_BURST_RD_MAX_BL_M) << DDR_BURST_RD_MAX_BL_S)
#define DDR_BURST_WR_MAX_BL_S   24
#define DDR_BURST_WR_MAX_BL_M   0xf
#define DDR_BURST_WR_MAX_BL(x) \
	(((x) & DDR_BURST_WR_MAX_BL_M) << DDR_BURST_WR_MAX_BL_S)
#define DDR_BURST_RWP_MASK_EN_S 28
#define DDR_BURST_RWP_MASK_EN_M 0x3
#define DDR_BURST_RWP_MASK_EN(x) \
	(((x) & DDR_BURST_RWP_MASK_EN_M) << DDR_BURST_RWP_MASK_EN_S)
#define DDR_BURST_CPU_PRI_BE    BIT(30)
#define DDR_BURST_CPU_PRI       BIT(31)
#define DDR_BURST_VAL           (DDR_BURST_CPU_PRI_BE | \
				 DDR_BURST_RWP_MASK_EN(3) | \
				 DDR_BURST_WR_MAX_BL(4) | \
				 DDR_BURST_RD_MAX_BL(4) | \
				 DDR_BURST_CPU_MAX_BL(4) | \
				 DDR_BURST_USB_MAX_BL(4) | \
				 DDR_BURST_PCIE_MAX_BL(4) | \
				 DDR_BURST_GE1_MAX_BL(4) | \
				 DDR_BURST_GE0_MAX_BL(4))

#define DDR_BURST_WMAC_MAX_BL_S 0
#define DDR_BURST_WMAC_MAX_BL_M 0xf
#define DDR_BURST_WMAC_MAX_BL(x) \
	(((x) & DDR_BURST_WMAC_MAX_BL_M) << DDR_BURST_WMAC_MAX_BL_S)
#define DDR_BURST2_VAL          DDR_BURST_WMAC_MAX_BL(4)

#define DDR2_CONF_TWL_S         10
#define DDR2_CONF_TWL_M         0xf
#define DDR2_CONF_TWL(x) \
	(((x) & DDR2_CONF_TWL_M) << DDR2_CONF_TWL_S)
#define DDR2_CONF_ODT           BIT(9)
#define DDR2_CONF_TFAW_S        2
#define DDR2_CONF_TFAW_M        0x3f
#define DDR2_CONF_TFAW(x) \
	(((x) & DDR2_CONF_TFAW_M) << DDR2_CONF_TFAW_S)
#define DDR2_CONF_EN            BIT(0)
#define DDR2_CONF_VAL           (DDR2_CONF_TWL(5) | \
				 DDR2_CONF_TFAW(31) | \
				 DDR2_CONF_ODT | \
				 DDR2_CONF_EN)

#define DDR1_EXT_MODE_VAL       0
#define DDR2_EXT_MODE_VAL       0x402
#define DDR2_EXT_MODE_OCD_VAL   0x782
#define DDR1_MODE_DLL_VAL       0x133
#define DDR2_MODE_DLL_VAL       0x143
#define DDR1_MODE_VAL           0x33
#define DDR2_MODE_VAL           0x43
#define DDR1_TAP_VAL            0x20
#define DDR2_TAP_VAL            0x10

#define DDR_REG_BIST_MASK_ADDR_0        0x2c
#define DDR_REG_BIST_MASK_ADDR_1        0x30
#define DDR_REG_BIST_MASK_AHB_GE0_0     0x34
#define DDR_REG_BIST_COMP_AHB_GE0_0     0x38
#define DDR_REG_BIST_MASK_AHB_GE1_0     0x3c
#define DDR_REG_BIST_COMP_AHB_GE1_0     0x40
#define DDR_REG_BIST_COMP_ADDR_0        0x64
#define DDR_REG_BIST_COMP_ADDR_1        0x68
#define DDR_REG_BIST_MASK_AHB_GE0_1     0x6c
#define DDR_REG_BIST_COMP_AHB_GE0_1     0x70
#define DDR_REG_BIST_MASK_AHB_GE1_1     0x74
#define DDR_REG_BIST_COMP_AHB_GE1_1     0x78
#define DDR_REG_BIST                    0x11c
#define DDR_REG_BIST_STATUS             0x120

#define DDR_BIST_COMP_CNT_S     1
#define DDR_BIST_COMP_CNT_M     0xff
#define DDR_BIST_COMP_CNT(x) \
	(((x) & DDR_BIST_COMP_CNT_M) << DDR_BIST_COMP_CNT_S)
#define DDR_BIST_COMP_CNT_MASK \
	(DDR_BIST_COMP_CNT_M << DDR_BIST_COMP_CNT_S)
#define DDR_BIST_TEST_START     BIT(0)
#define DDR_BIST_STATUS_DONE    BIT(0)

/* 4 Row Address Bits, 4 Column Address Bits, 2 BA bits */
#define DDR_BIST_MASK_ADDR_VAL  0xfa5de83f

#define DDR_TAP_MAGIC_VAL       0xaa55aa55
#define DDR_TAP_MAX_VAL         0x40

void ddr_init(void)
{
	void __iomem *regs;
	u32 val;

	regs = map_physmem(AR71XX_DDR_CTRL_BASE, AR71XX_DDR_CTRL_SIZE,
			   MAP_NOCACHE);
	val = ath79_get_bootstrap();
	if (val & QCA953X_BOOTSTRAP_DDR1) {
		writel(DDR_CTL_CONFIG_VAL, regs + QCA953X_DDR_REG_CTL_CONF);
		udelay(10);

		/* For 16-bit DDR */
		writel(0xffff, regs + AR71XX_DDR_REG_RD_CYCLE);
		udelay(100);

		/* Burst size */
		writel(DDR_BURST_VAL, regs + QCA953X_DDR_REG_BURST);
		udelay(100);
		writel(DDR_BURST2_VAL, regs + QCA953X_DDR_REG_BURST2);
		udelay(100);

		/* AHB maximum timeout */
		writel(0xfffff, regs + QCA953X_DDR_REG_TIMEOUT_MAX);
		udelay(100);

		/* DRAM timing */
		writel(DDR1_CONF_REG_VAL, regs + AR71XX_DDR_REG_CONFIG);
		udelay(100);
		writel(DDR1_CONF2_REG_VAL, regs + AR71XX_DDR_REG_CONFIG2);
		udelay(100);
		writel(DDR1_CONF3_REG_VAL, regs + QCA953X_DDR_REG_CONFIG3);
		udelay(100);

		/* Precharge All */
		writel(DDR_CTRL_PRECHARGE, regs + AR71XX_DDR_REG_CONTROL);
		udelay(100);

		/* ODT disable, Full strength, Enable DLL */
		writel(DDR1_EXT_MODE_VAL, regs + AR71XX_DDR_REG_EMR);
		udelay(100);

		/* Update Extended Mode Register Set (EMRS) */
		writel(DDR_CTRL_UPD_EMRS, regs + AR71XX_DDR_REG_CONTROL);
		udelay(100);

		/* Reset DLL, CAS Latency 3, Burst Length 8 */
		writel(DDR1_MODE_DLL_VAL, regs + AR71XX_DDR_REG_MODE);
		udelay(100);

		/* Update Mode Register Set (MRS) */
		writel(DDR_CTRL_UPD_MRS, regs + AR71XX_DDR_REG_CONTROL);
		udelay(100);

		/* Precharge All */
		writel(DDR_CTRL_PRECHARGE, regs + AR71XX_DDR_REG_CONTROL);
		udelay(100);

		/* Auto Refresh */
		writel(DDR_CTRL_AUTO_REFRESH, regs + AR71XX_DDR_REG_CONTROL);
		udelay(100);
		writel(DDR_CTRL_AUTO_REFRESH, regs + AR71XX_DDR_REG_CONTROL);
		udelay(100);

		/* Normal DLL, CAS Latency 3, Burst Length 8 */
		writel(DDR1_MODE_VAL, regs + AR71XX_DDR_REG_MODE);
		udelay(100);

		/* Update Mode Register Set (MRS) */
		writel(DDR_CTRL_UPD_MRS, regs + AR71XX_DDR_REG_CONTROL);
		udelay(100);

		/* Refresh time control */
		writel(DDR_REFRESH_VAL, regs + AR71XX_DDR_REG_REFRESH);
		udelay(100);

		/* DQS 0 Tap Control */
		writel(DDR1_TAP_VAL, regs + AR71XX_DDR_REG_TAP_CTRL0);

		/* DQS 1 Tap Control */
		writel(DDR1_TAP_VAL, regs + AR71XX_DDR_REG_TAP_CTRL1);
	} else {
		writel(DDR_CTRL_UPD_EMR2S, regs + AR71XX_DDR_REG_CONTROL);
		udelay(10);
		writel(DDR_CTRL_UPD_EMR3S, regs + AR71XX_DDR_REG_CONTROL);
		udelay(10);
		writel(DDR_CTL_CONFIG_VAL | DDR_CTL_PAD_DDR2_SEL,
		       regs + QCA953X_DDR_REG_CTL_CONF);
		udelay(10);

		/* For 16-bit DDR */
		writel(0xffff, regs + AR71XX_DDR_REG_RD_CYCLE);
		udelay(100);

		/* Burst size */
		writel(DDR_BURST_VAL, regs + QCA953X_DDR_REG_BURST);
		udelay(100);
		writel(DDR_BURST2_VAL, regs + QCA953X_DDR_REG_BURST2);
		udelay(100);

		/* AHB maximum timeout */
		writel(0xfffff, regs + QCA953X_DDR_REG_TIMEOUT_MAX);
		udelay(100);

		/* DRAM timing */
		writel(DDR2_CONF_REG_VAL, regs + AR71XX_DDR_REG_CONFIG);
		udelay(100);
		writel(DDR2_CONF2_REG_VAL, regs + AR71XX_DDR_REG_CONFIG2);
		udelay(100);
		writel(DDR2_CONF3_REG_VAL, regs + QCA953X_DDR_REG_CONFIG3);
		udelay(100);

		/* Enable DDR2 */
		writel(DDR2_CONF_VAL, regs + QCA953X_DDR_REG_DDR2_CONFIG);
		udelay(100);

		/* Precharge All */
		writel(DDR_CTRL_PRECHARGE, regs + AR71XX_DDR_REG_CONTROL);
		udelay(100);

		/* Update Extended Mode Register 2 Set (EMR2S) */
		writel(DDR_CTRL_UPD_EMR2S, regs + AR71XX_DDR_REG_CONTROL);
		udelay(100);

		/* Update Extended Mode Register 3 Set (EMR3S) */
		writel(DDR_CTRL_UPD_EMR3S, regs + AR71XX_DDR_REG_CONTROL);
		udelay(100);

		/* 150 ohm, Reduced strength, Enable DLL */
		writel(DDR2_EXT_MODE_VAL, regs + AR71XX_DDR_REG_EMR);
		udelay(100);

		/* Update Extended Mode Register Set (EMRS) */
		writel(DDR_CTRL_UPD_EMRS, regs + AR71XX_DDR_REG_CONTROL);
		udelay(100);

		/* Reset DLL, CAS Latency 4, Burst Length 8 */
		writel(DDR2_MODE_DLL_VAL, regs + AR71XX_DDR_REG_MODE);
		udelay(100);

		/* Update Mode Register Set (MRS) */
		writel(DDR_CTRL_UPD_MRS, regs + AR71XX_DDR_REG_CONTROL);
		udelay(100);

		/* Precharge All */
		writel(DDR_CTRL_PRECHARGE, regs + AR71XX_DDR_REG_CONTROL);
		udelay(100);

		/* Auto Refresh */
		writel(DDR_CTRL_AUTO_REFRESH, regs + AR71XX_DDR_REG_CONTROL);
		udelay(100);
		writel(DDR_CTRL_AUTO_REFRESH, regs + AR71XX_DDR_REG_CONTROL);
		udelay(100);

		/* Normal DLL, CAS Latency 4, Burst Length 8 */
		writel(DDR2_MODE_VAL, regs + AR71XX_DDR_REG_MODE);
		udelay(100);

		/* Mode Register Set (MRS) */
		writel(DDR_CTRL_UPD_MRS, regs + AR71XX_DDR_REG_CONTROL);
		udelay(100);

		/* Enable OCD, Enable DLL, Reduced Drive Strength */
		writel(DDR2_EXT_MODE_OCD_VAL, regs + AR71XX_DDR_REG_EMR);
		udelay(100);

		/* Update Extended Mode Register Set (EMRS) */
		writel(DDR_CTRL_UPD_EMRS, regs + AR71XX_DDR_REG_CONTROL);
		udelay(100);

		/* OCD diable, Enable DLL, Reduced Drive Strength */
		writel(DDR2_EXT_MODE_VAL, regs + AR71XX_DDR_REG_EMR);
		udelay(100);

		/* Update Extended Mode Register Set (EMRS) */
		writel(DDR_CTRL_UPD_EMRS, regs + AR71XX_DDR_REG_CONTROL);
		udelay(100);

		/* Refresh time control */
		writel(DDR_REFRESH_VAL, regs + AR71XX_DDR_REG_REFRESH);
		udelay(100);

		/* DQS 0 Tap Control */
		writel(DDR2_TAP_VAL, regs + AR71XX_DDR_REG_TAP_CTRL0);

		/* DQS 1 Tap Control */
		writel(DDR2_TAP_VAL, regs + AR71XX_DDR_REG_TAP_CTRL1);
	}
}

void ddr_tap_tuning(void)
{
	void __iomem *regs;
	u32 val, pass, tap, cnt, tap_val, last, first;

	regs = map_physmem(AR71XX_DDR_CTRL_BASE, AR71XX_DDR_CTRL_SIZE,
			   MAP_NOCACHE);

	tap_val = readl(regs + AR71XX_DDR_REG_TAP_CTRL0);
	first = DDR_TAP_MAGIC_VAL;
	last = 0;
	cnt = 0;
	tap = 0;

	do {
		writel(tap, regs + AR71XX_DDR_REG_TAP_CTRL0);
		writel(tap, regs + AR71XX_DDR_REG_TAP_CTRL1);

		writel(DDR_BIST_COMP_CNT(8), regs + DDR_REG_BIST_COMP_ADDR_1);
		writel(DDR_BIST_MASK_ADDR_VAL, regs + DDR_REG_BIST_MASK_ADDR_0);
		writel(0xffff, regs + DDR_REG_BIST_COMP_AHB_GE0_1);
		writel(0xffff, regs + DDR_REG_BIST_COMP_AHB_GE1_0);
		writel(0xffff, regs + DDR_REG_BIST_COMP_AHB_GE1_1);
		writel(0xffff, regs + DDR_REG_BIST_MASK_AHB_GE0_0);
		writel(0xffff, regs + DDR_REG_BIST_MASK_AHB_GE0_1);
		writel(0xffff, regs + DDR_REG_BIST_MASK_AHB_GE1_0);
		writel(0xffff, regs + DDR_REG_BIST_MASK_AHB_GE1_1);
		writel(0xffff, regs + DDR_REG_BIST_COMP_AHB_GE0_0);

		/* Start BIST test */
		writel(DDR_BIST_TEST_START, regs + DDR_REG_BIST);

		do {
			val = readl(regs + DDR_REG_BIST_STATUS);
		} while (!(val & DDR_BIST_STATUS_DONE));

		/* Stop BIST test */
		writel(0, regs + DDR_REG_BIST);

		pass = val & DDR_BIST_COMP_CNT_MASK;
		pass ^= DDR_BIST_COMP_CNT(8);
		if (!pass) {
			if (first != DDR_TAP_MAGIC_VAL) {
				last = tap;
			} else  {
				first = tap;
				last = tap;
			}
			cnt++;
		}
		tap++;
	} while (tap < DDR_TAP_MAX_VAL);

	if (cnt) {
		tap_val = (first + last) / 2;
		tap_val %= DDR_TAP_MAX_VAL;
	}

	writel(tap_val, regs + AR71XX_DDR_REG_TAP_CTRL0);
	writel(tap_val, regs + AR71XX_DDR_REG_TAP_CTRL1);
}
