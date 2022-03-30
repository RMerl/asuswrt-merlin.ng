/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2015 Linaro
 * Peter Griffin <peter.griffin@linaro.org>
 */

#ifndef __HI6220_H__
#define __HI6220_H__

#include "hi6220_regs_alwayson.h"

#define HI6220_MMC0_BASE			0xF723D000
#define HI6220_MMC1_BASE			0xF723E000

#define HI6220_UART0_BASE			0xF8015000
#define HI6220_UART3_BASE			0xF7113000

#define HI6220_PMUSSI_BASE			0xF8000000

#define HI6220_PERI_BASE			0xF7030000

struct peri_sc_periph_regs {
	u32 ctrl1;		/*0x0*/
	u32 ctrl2;
	u32 ctrl3;
	u32 ctrl4;
	u32 ctrl5;
	u32 ctrl6;
	u32 ctrl8;
	u32 ctrl9;
	u32 ctrl10;
	u32 ctrl12;
	u32 ctrl13;
	u32 ctrl14;

	u32 unknown_1[8];

	u32 ddr_ctrl0;		/*0x50*/

	u32 unknown_2[16];

	u32 stat1;		/*0x94*/

	u32 unknown_3[90];

	u32 clk0_en;		/*0x200*/
	u32 clk0_dis;
	u32 clk0_stat;

	u32 unknown_4;

	u32 clk1_en;		/*0x210*/
	u32 clk1_dis;
	u32 clk1_stat;

	u32 unknown_5;

	u32 clk2_en;		/*0x220*/
	u32 clk2_dis;
	u32 clk2_stat;

	u32 unknown_6;

	u32 clk3_en;		/*0x230*/
	u32 clk3_dis;
	u32 clk3_stat;

	u32 unknown_7;

	u32 clk8_en;		/*0x240*/
	u32 clk8_dis;
	u32 clk8_stat;

	u32 unknown_8;

	u32 clk9_en;		/*0x250*/
	u32 clk9_dis;
	u32 clk9_stat;

	u32 unknown_9;

	u32 clk10_en;		/*0x260*/
	u32 clk10_dis;
	u32 clk10_stat;

	u32 unknown_10;

	u32 clk12_en;		/*0x270*/
	u32 clk12_dis;
	u32 clk12_stat;

	u32 unknown_11[33];

	u32 rst0_en;		/*0x300*/
	u32 rst0_dis;
	u32 rst0_stat;

	u32 unknown_12;

	u32 rst1_en;		/*0x310*/
	u32 rst1_dis;
	u32 rst1_stat;

	u32 unknown_13;

	u32 rst2_en;		/*0x320*/
	u32 rst2_dis;
	u32 rst2_stat;

	u32 unknown_14;

	u32 rst3_en;		/*0x330*/
	u32 rst3_dis;
	u32 rst3_stat;

	u32 unknown_15;

	u32 rst8_en;		/*0x340*/
	u32 rst8_dis;
	u32 rst8_stat;

	u32 unknown_16[45];

	u32 clk0_sel;		/*0x400*/

	u32 unknown_17[36];

	u32 clkcfg8bit1;	/*0x494*/
	u32 clkcfg8bit2;

	u32 unknown_18[538];

	u32 reserved8_addr;	/*0xd04*/
};


/* CTRL1 bit definitions */

#define PERI_CTRL1_ETR_AXI_CSYSREQ_N			(1 << 0)
#define PERI_CTRL1_HIFI_INT_MASK			(1 << 1)
#define PERI_CTRL1_HIFI_ALL_INT_MASK			(1 << 2)
#define PERI_CTRL1_ETR_AXI_CSYSREQ_N_MSK		(1 << 16)
#define PERI_CTRL1_HIFI_INT_MASK_MSK			(1 << 17)
#define PERI_CTRL1_HIFI_ALL_INT_MASK_MSK		(1 << 18)


/* CTRL2 bit definitions */

#define PERI_CTRL2_MMC_CLK_PHASE_BYPASS_EN_MMC0		(1 << 0)
#define PERI_CTRL2_MMC_CLK_PHASE_BYPASS_EN_MMC1		(1 << 2)
#define PERI_CTRL2_NAND_SYS_MEM_SEL			(1 << 6)
#define PERI_CTRL2_G3D_DDRT_AXI_SEL			(1 << 7)
#define PERI_CTRL2_GU_MDM_BBP_TESTPIN_SEL		(1 << 8)
#define PERI_CTRL2_CODEC_SSI_MASTER_CHECK		(1 << 9)
#define PERI_CTRL2_FUNC_TEST_SOFT			(1 << 12)
#define PERI_CTRL2_CSSYS_TS_ENABLE			(1 << 15)
#define PERI_CTRL2_HIFI_RAMCTRL_S_EMA			(1 << 16)
#define PERI_CTRL2_HIFI_RAMCTRL_S_EMAW			(1 << 20)
#define PERI_CTRL2_HIFI_RAMCTRL_S_EMAS			(1 << 22)
#define PERI_CTRL2_HIFI_RAMCTRL_S_RET1N			(1 << 26)
#define PERI_CTRL2_HIFI_RAMCTRL_S_RET2N			(1 << 27)
#define PERI_CTRL2_HIFI_RAMCTRL_S_PGEN			(1 << 28)

/* CTRL3 bit definitions */

#define PERI_CTRL3_HIFI_DDR_HARQMEM_ADDR		(1 << 0)
#define PERI_CTRL3_HIFI_HARQMEMRMP_EN			(1 << 12)
#define PERI_CTRL3_HARQMEM_SYS_MED_SEL			(1 << 13)
#define PERI_CTRL3_SOC_AP_OCCUPY_GRP1			(1 << 14)
#define PERI_CTRL3_SOC_AP_OCCUPY_GRP2			(1 << 16)
#define PERI_CTRL3_SOC_AP_OCCUPY_GRP3			(1 << 18)
#define PERI_CTRL3_SOC_AP_OCCUPY_GRP4			(1 << 20)
#define PERI_CTRL3_SOC_AP_OCCUPY_GRP5			(1 << 22)
#define PERI_CTRL3_SOC_AP_OCCUPY_GRP6			(1 << 24)

/* CTRL4 bit definitions */

#define PERI_CTRL4_PICO_FSELV				(1 << 0)
#define PERI_CTRL4_FPGA_EXT_PHY_SEL			(1 << 3)
#define PERI_CTRL4_PICO_REFCLKSEL			(1 << 4)
#define PERI_CTRL4_PICO_SIDDQ				(1 << 6)
#define PERI_CTRL4_PICO_SUSPENDM_SLEEPM			(1 << 7)
#define PERI_CTRL4_PICO_OGDISABLE			(1 << 8)
#define PERI_CTRL4_PICO_COMMONONN			(1 << 9)
#define PERI_CTRL4_PICO_VBUSVLDEXT			(1 << 10)
#define PERI_CTRL4_PICO_VBUSVLDEXTSEL			(1 << 11)
#define PERI_CTRL4_PICO_VATESTENB			(1 << 12)
#define PERI_CTRL4_PICO_SUSPENDM			(1 << 14)
#define PERI_CTRL4_PICO_SLEEPM				(1 << 15)
#define PERI_CTRL4_BC11_C				(1 << 16)
#define PERI_CTRL4_BC11_B				(1 << 17)
#define PERI_CTRL4_BC11_A				(1 << 18)
#define PERI_CTRL4_BC11_GND				(1 << 19)
#define PERI_CTRL4_BC11_FLOAT				(1 << 20)
#define PERI_CTRL4_OTG_PHY_SEL				(1 << 21)
#define PERI_CTRL4_USB_OTG_SS_SCALEDOWN_MODE		(1 << 22)
#define PERI_CTRL4_OTG_DM_PULLDOWN			(1 << 24)
#define PERI_CTRL4_OTG_DP_PULLDOWN			(1 << 25)
#define PERI_CTRL4_OTG_IDPULLUP				(1 << 26)
#define PERI_CTRL4_OTG_DRVBUS				(1 << 27)
#define PERI_CTRL4_OTG_SESSEND				(1 << 28)
#define PERI_CTRL4_OTG_BVALID				(1 << 29)
#define PERI_CTRL4_OTG_AVALID				(1 << 30)
#define PERI_CTRL4_OTG_VBUSVALID			(1 << 31)

/* CTRL5 bit definitions */

#define PERI_CTRL5_USBOTG_RES_SEL			(1 << 3)
#define PERI_CTRL5_PICOPHY_ACAENB			(1 << 4)
#define PERI_CTRL5_PICOPHY_BC_MODE			(1 << 5)
#define PERI_CTRL5_PICOPHY_CHRGSEL			(1 << 6)
#define PERI_CTRL5_PICOPHY_VDATSRCEND			(1 << 7)
#define PERI_CTRL5_PICOPHY_VDATDETENB			(1 << 8)
#define PERI_CTRL5_PICOPHY_DCDENB			(1 << 9)
#define PERI_CTRL5_PICOPHY_IDDIG			(1 << 10)
#define PERI_CTRL5_DBG_MUX				(1 << 11)

/* CTRL6 bit definitions */

#define PERI_CTRL6_CSSYSOFF_RAMCTRL_S_EMA		(1 << 0)
#define PERI_CTRL6_CSSYSOFF_RAMCTRL_S_EMAW		(1 << 4)
#define PERI_CTRL6_CSSYSOFF_RAMCTRL_S_EMAS		(1 << 6)
#define PERI_CTRL6_CSSYSOFF_RAMCTRL_S_RET1N		(1 << 10)
#define PERI_CTRL6_CSSYSOFF_RAMCTRL_S_RET2N		(1 << 11)
#define PERI_CTRL6_CSSYSOFF_RAMCTRL_S_PGEN		(1 << 12)

/* CTRL8 bit definitions */

#define PERI_CTRL8_PICOPHY_TXRISETUNE0			(1 << 0)
#define PERI_CTRL8_PICOPHY_TXPREEMPAMPTUNE0		(1 << 2)
#define PERI_CTRL8_PICOPHY_TXRESTUNE0			(1 << 4)
#define PERI_CTRL8_PICOPHY_TXHSSVTUNE0			(1 << 6)
#define PERI_CTRL8_PICOPHY_COMPDISTUNE0			(1 << 8)
#define PERI_CTRL8_PICOPHY_TXPREEMPPULSETUNE0		(1 << 11)
#define PERI_CTRL8_PICOPHY_OTGTUNE0			(1 << 12)
#define PERI_CTRL8_PICOPHY_SQRXTUNE0			(1 << 16)
#define PERI_CTRL8_PICOPHY_TXVREFTUNE0			(1 << 20)
#define PERI_CTRL8_PICOPHY_TXFSLSTUNE0			(1 << 28)

/* CTRL9 bit definitions */

#define PERI_CTRL9_PICOPLY_TESTCLKEN			(1 << 0)
#define PERI_CTRL9_PICOPLY_TESTDATAOUTSEL		(1 << 1)
#define PERI_CTRL9_PICOPLY_TESTADDR			(1 << 4)
#define PERI_CTRL9_PICOPLY_TESTDATAIN			(1 << 8)

/* CLK0 EN/DIS/STAT bit definitions */

#define PERI_CLK0_MMC0					(1 << 0)
#define PERI_CLK0_MMC1					(1 << 1)
#define PERI_CLK0_MMC2					(1 << 2)
#define PERI_CLK0_NANDC					(1 << 3)
#define PERI_CLK0_USBOTG				(1 << 4)
#define PERI_CLK0_PICOPHY				(1 << 5)
#define PERI_CLK0_PLL					(1 << 6)

/* CLK1 EN/DIS/STAT bit definitions */

#define PERI_CLK1_HIFI					(1 << 0)
#define PERI_CLK1_DIGACODEC				(1 << 5)

/* CLK2 EN/DIS/STAT bit definitions */

#define PERI_CLK2_IPF					(1 << 0)
#define PERI_CLK2_SOCP					(1 << 1)
#define PERI_CLK2_DMAC					(1 << 2)
#define PERI_CLK2_SECENG				(1 << 3)
#define PERI_CLK2_HPM0					(1 << 5)
#define PERI_CLK2_HPM1					(1 << 6)
#define PERI_CLK2_HPM2					(1 << 7)
#define PERI_CLK2_HPM3					(1 << 8)

/* CLK8 EN/DIS/STAT bit definitions */

#define PERI_CLK8_RS0					(1 << 0)
#define PERI_CLK8_RS2					(1 << 1)
#define PERI_CLK8_RS3					(1 << 2)
#define PERI_CLK8_MS0					(1 << 3)
#define PERI_CLK8_MS2					(1 << 5)
#define PERI_CLK8_XG2RAM0				(1 << 6)
#define PERI_CLK8_X2SRAM				(1 << 7)
#define PERI_CLK8_SRAM					(1 << 8)
#define PERI_CLK8_ROM					(1 << 9)
#define PERI_CLK8_HARQ					(1 << 10)
#define PERI_CLK8_MMU					(1 << 11)
#define PERI_CLK8_DDRC					(1 << 12)
#define PERI_CLK8_DDRPHY				(1 << 13)
#define PERI_CLK8_DDRPHY_REF				(1 << 14)
#define PERI_CLK8_X2X_SYSNOC				(1 << 15)
#define PERI_CLK8_X2X_CCPU				(1 << 16)
#define PERI_CLK8_DDRT					(1 << 17)
#define PERI_CLK8_DDRPACK_RS				(1 << 18)

/* CLK9 EN/DIS/STAT bit definitions */

#define PERI_CLK9_CARM_DAP				(1 << 0)
#define PERI_CLK9_CARM_ATB				(1 << 1)
#define PERI_CLK9_CARM_LBUS				(1 << 2)
#define PERI_CLK9_CARM_KERNEL				(1 << 3)

/* CLK10 EN/DIS/STAT bit definitions */

#define PERI_CLK10_IPF_CCPU				(1 << 0)
#define PERI_CLK10_SOCP_CCPU				(1 << 1)
#define PERI_CLK10_SECENG_CCPU				(1 << 2)
#define PERI_CLK10_HARQ_CCPU				(1 << 3)
#define PERI_CLK10_IPF_MCU				(1 << 16)
#define PERI_CLK10_SOCP_MCU				(1 << 17)
#define PERI_CLK10_SECENG_MCU				(1 << 18)
#define PERI_CLK10_HARQ_MCU				(1 << 19)

/* CLK12 EN/DIS/STAT bit definitions */

#define PERI_CLK12_HIFI_SRC				(1 << 0)
#define PERI_CLK12_MMC0_SRC				(1 << 1)
#define PERI_CLK12_MMC1_SRC				(1 << 2)
#define PERI_CLK12_MMC2_SRC				(1 << 3)
#define PERI_CLK12_SYSPLL_DIV				(1 << 4)
#define PERI_CLK12_TPIU_SRC				(1 << 5)
#define PERI_CLK12_MMC0_HF				(1 << 6)
#define PERI_CLK12_MMC1_HF				(1 << 7)
#define PERI_CLK12_PLL_TEST_SRC				(1 << 8)
#define PERI_CLK12_CODEC_SOC				(1 << 9)
#define PERI_CLK12_MEDIA				(1 << 10)

/* RST0 EN/DIS/STAT bit definitions */

#define PERI_RST0_MMC0					(1 << 0)
#define PERI_RST0_MMC1					(1 << 1)
#define PERI_RST0_MMC2					(1 << 2)
#define PERI_RST0_NANDC					(1 << 3)
#define PERI_RST0_USBOTG_BUS				(1 << 4)
#define PERI_RST0_POR_PICOPHY				(1 << 5)
#define PERI_RST0_USBOTG				(1 << 6)
#define PERI_RST0_USBOTG_32K				(1 << 7)

/* RST1 EN/DIS/STAT bit definitions */

#define PERI_RST1_HIFI					(1 << 0)
#define PERI_RST1_DIGACODEC				(1 << 5)

/* RST2 EN/DIS/STAT bit definitions */

#define PERI_RST2_IPF					(1 << 0)
#define PERI_RST2_SOCP					(1 << 1)
#define PERI_RST2_DMAC					(1 << 2)
#define PERI_RST2_SECENG				(1 << 3)
#define PERI_RST2_ABB					(1 << 4)
#define PERI_RST2_HPM0					(1 << 5)
#define PERI_RST2_HPM1					(1 << 6)
#define PERI_RST2_HPM2					(1 << 7)
#define PERI_RST2_HPM3					(1 << 8)

/* RST3 EN/DIS/STAT bit definitions */

#define PERI_RST3_CSSYS					(1 << 0)
#define PERI_RST3_I2C0					(1 << 1)
#define PERI_RST3_I2C1					(1 << 2)
#define PERI_RST3_I2C2					(1 << 3)
#define PERI_RST3_I2C3					(1 << 4)
#define PERI_RST3_UART1					(1 << 5)
#define PERI_RST3_UART2					(1 << 6)
#define PERI_RST3_UART3					(1 << 7)
#define PERI_RST3_UART4					(1 << 8)
#define PERI_RST3_SSP					(1 << 9)
#define PERI_RST3_PWM					(1 << 10)
#define PERI_RST3_BLPWM					(1 << 11)
#define PERI_RST3_TSENSOR				(1 << 12)
#define PERI_RST3_DAPB					(1 << 18)
#define PERI_RST3_HKADC					(1 << 19)
#define PERI_RST3_CODEC					(1 << 20)

/* RST8 EN/DIS/STAT bit definitions */

#define PERI_RST8_RS0					(1 << 0)
#define PERI_RST8_RS2					(1 << 1)
#define PERI_RST8_RS3					(1 << 2)
#define PERI_RST8_MS0					(1 << 3)
#define PERI_RST8_MS2					(1 << 5)
#define PERI_RST8_XG2RAM0				(1 << 6)
#define PERI_RST8_X2SRAM_TZMA				(1 << 7)
#define PERI_RST8_SRAM					(1 << 8)
#define PERI_RST8_HARQ					(1 << 10)
#define PERI_RST8_DDRC					(1 << 12)
#define PERI_RST8_DDRC_APB				(1 << 13)
#define PERI_RST8_DDRPACK_APB				(1 << 14)
#define PERI_RST8_DDRT					(1 << 17)

#endif /*__HI62220_H__*/
