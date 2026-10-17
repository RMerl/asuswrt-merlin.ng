// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2021 NXP
 */

#include <common.h>
#include <div64.h>
#include <asm/io.h>
#include <errno.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/pcc.h>
#include <asm/arch/cgc.h>
#include <asm/arch/sys_proto.h>

#define cgc_clk_TYPES 2
#define cgc_clk_NUM 8

static enum cgc_clk pcc1_clksrc[][8] = {
	{
	},
	{
		DUMMY0_CLK,
		LPOSC,
		SOSC_DIV2,
		FRO_DIV2,
		CM33_BUSCLK,
		PLL1_VCO_DIV,
		PLL0_PFD2_DIV,
		PLL0_PFD1_DIV,
	}
};

static enum cgc_clk pcc3_clksrc[][8] = {
	{
	},
	{	DUMMY0_CLK,
		LPOSC,
		SOSC_DIV2,
		FRO_DIV2,
		XBAR_BUSCLK,
		PLL3_PFD1_DIV1,
		PLL3_PFD0_DIV2,
		PLL3_PFD0_DIV1
	}
};

static enum cgc_clk pcc4_clksrc[][8] = {
	{
		DUMMY0_CLK,
		SOSC_DIV1,
		FRO_DIV1,
		PLL3_PFD3_DIV2,
		PLL3_PFD3_DIV1,
		PLL3_PFD2_DIV2,
		PLL3_PFD2_DIV1,
		PLL3_PFD1_DIV2
	},
	{
		DUMMY0_CLK,
		DUMMY1_CLK,
		LPOSC,
		SOSC_DIV2,
		FRO_DIV2,
		XBAR_BUSCLK,
		PLL3_VCODIV,
		PLL3_PFD0_DIV1
	}
};

static enum cgc_clk pcc5_clksrc[][8] = {
	{
		DUMMY0_CLK,
		PLL4_PFD3_DIV2,
		PLL4_PFD2_DIV2,
		PLL4_PFD2_DIV1,
		PLL4_PFD1_DIV2,
		PLL4_PFD1_DIV1,
		PLL4_PFD0_DIV2,
		PLL4_PFD0_DIV1
	},
	{
		DUMMY0_CLK,
		DUMMY1_CLK,
		LPOSC,
		SOSC_DIV2,
		FRO_DIV2,
		LPAV_BUSCLK,
		PLL4_VCODIV,
		PLL4_PFD3_DIV1
	}
};

static struct pcc_entry pcc1_arrays[] = {
	{PCC1_RBASE, ADC1_PCC1_SLOT, CLKSRC_PER_BUS, PCC_NO_DIV, PCC_HAS_RST_B},
	{}
};

static struct pcc_entry pcc3_arrays[] = {
	{PCC3_RBASE, DMA1_MP_PCC3_SLOT,		CLKSRC_NO_PCS, PCC_NO_DIV, PCC_NO_RST_B},
	{PCC3_RBASE, DMA1_CH0_PCC3_SLOT,	CLKSRC_NO_PCS, PCC_NO_DIV, PCC_NO_RST_B},
	{PCC3_RBASE, DMA1_CH1_PCC3_SLOT,	CLKSRC_NO_PCS, PCC_NO_DIV, PCC_NO_RST_B},
	{PCC3_RBASE, DMA1_CH2_PCC3_SLOT,	CLKSRC_NO_PCS, PCC_NO_DIV, PCC_NO_RST_B},
	{PCC3_RBASE, DMA1_CH3_PCC3_SLOT,	CLKSRC_NO_PCS, PCC_NO_DIV, PCC_NO_RST_B},
	{PCC3_RBASE, DMA1_CH4_PCC3_SLOT,	CLKSRC_NO_PCS, PCC_NO_DIV, PCC_NO_RST_B},
	{PCC3_RBASE, DMA1_CH5_PCC3_SLOT,	CLKSRC_NO_PCS, PCC_NO_DIV, PCC_NO_RST_B},
	{PCC3_RBASE, DMA1_CH6_PCC3_SLOT,	CLKSRC_NO_PCS, PCC_NO_DIV, PCC_NO_RST_B},
	{PCC3_RBASE, DMA1_CH7_PCC3_SLOT,	CLKSRC_NO_PCS, PCC_NO_DIV, PCC_NO_RST_B},
	{PCC3_RBASE, DMA1_CH8_PCC3_SLOT,	CLKSRC_NO_PCS, PCC_NO_DIV, PCC_NO_RST_B},
	{PCC3_RBASE, DMA1_CH9_PCC3_SLOT,	CLKSRC_NO_PCS, PCC_NO_DIV, PCC_NO_RST_B},
	{PCC3_RBASE, DMA1_CH10_PCC3_SLOT,	CLKSRC_NO_PCS, PCC_NO_DIV, PCC_NO_RST_B},
	{PCC3_RBASE, DMA1_CH11_PCC3_SLOT,	CLKSRC_NO_PCS, PCC_NO_DIV, PCC_NO_RST_B},
	{PCC3_RBASE, DMA1_CH12_PCC3_SLOT,	CLKSRC_NO_PCS, PCC_NO_DIV, PCC_NO_RST_B},
	{PCC3_RBASE, DMA1_CH13_PCC3_SLOT,	CLKSRC_NO_PCS, PCC_NO_DIV, PCC_NO_RST_B},
	{PCC3_RBASE, DMA1_CH14_PCC3_SLOT,	CLKSRC_NO_PCS, PCC_NO_DIV, PCC_NO_RST_B},
	{PCC3_RBASE, DMA1_CH15_PCC3_SLOT,	CLKSRC_NO_PCS, PCC_NO_DIV, PCC_NO_RST_B},
	{PCC3_RBASE, DMA1_CH16_PCC3_SLOT,	CLKSRC_NO_PCS, PCC_NO_DIV, PCC_NO_RST_B},
	{PCC3_RBASE, DMA1_CH17_PCC3_SLOT,	CLKSRC_NO_PCS, PCC_NO_DIV, PCC_NO_RST_B},
	{PCC3_RBASE, DMA1_CH18_PCC3_SLOT,	CLKSRC_NO_PCS, PCC_NO_DIV, PCC_NO_RST_B},
	{PCC3_RBASE, DMA1_CH19_PCC3_SLOT,	CLKSRC_NO_PCS, PCC_NO_DIV, PCC_NO_RST_B},
	{PCC3_RBASE, DMA1_CH20_PCC3_SLOT,	CLKSRC_NO_PCS, PCC_NO_DIV, PCC_NO_RST_B},
	{PCC3_RBASE, DMA1_CH21_PCC3_SLOT,	CLKSRC_NO_PCS, PCC_NO_DIV, PCC_NO_RST_B},
	{PCC3_RBASE, DMA1_CH22_PCC3_SLOT,	CLKSRC_NO_PCS, PCC_NO_DIV, PCC_NO_RST_B},
	{PCC3_RBASE, DMA1_CH23_PCC3_SLOT,	CLKSRC_NO_PCS, PCC_NO_DIV, PCC_NO_RST_B},
	{PCC3_RBASE, DMA1_CH24_PCC3_SLOT,	CLKSRC_NO_PCS, PCC_NO_DIV, PCC_NO_RST_B},
	{PCC3_RBASE, DMA1_CH25_PCC3_SLOT,	CLKSRC_NO_PCS, PCC_NO_DIV, PCC_NO_RST_B},
	{PCC3_RBASE, DMA1_CH26_PCC3_SLOT,	CLKSRC_NO_PCS, PCC_NO_DIV, PCC_NO_RST_B},
	{PCC3_RBASE, DMA1_CH27_PCC3_SLOT,	CLKSRC_NO_PCS, PCC_NO_DIV, PCC_NO_RST_B},
	{PCC3_RBASE, DMA1_CH28_PCC3_SLOT,	CLKSRC_NO_PCS, PCC_NO_DIV, PCC_NO_RST_B},
	{PCC3_RBASE, DMA1_CH29_PCC3_SLOT,	CLKSRC_NO_PCS, PCC_NO_DIV, PCC_NO_RST_B},
	{PCC3_RBASE, DMA1_CH30_PCC3_SLOT,	CLKSRC_NO_PCS, PCC_NO_DIV, PCC_NO_RST_B},
	{PCC3_RBASE, DMA1_CH31_PCC3_SLOT,	CLKSRC_NO_PCS, PCC_NO_DIV, PCC_NO_RST_B},
	{PCC3_RBASE, MU0_B_PCC3_SLOT,		CLKSRC_NO_PCS, PCC_NO_DIV, PCC_NO_RST_B},
	{PCC3_RBASE, MU3_A_PCC3_SLOT,		CLKSRC_NO_PCS, PCC_NO_DIV, PCC_NO_RST_B},
	{PCC3_RBASE, LLWU1_PCC3_SLOT,		CLKSRC_NO_PCS, PCC_NO_DIV, PCC_NO_RST_B},
	{PCC3_RBASE, UPOWER_PCC3_SLOT,		CLKSRC_NO_PCS, PCC_NO_DIV, PCC_NO_RST_B},
	{PCC3_RBASE, WDOG3_PCC3_SLOT,		CLKSRC_PER_BUS, PCC_HAS_DIV, PCC_HAS_RST_B},
	{PCC3_RBASE, WDOG4_PCC3_SLOT,		CLKSRC_PER_BUS, PCC_HAS_DIV, PCC_HAS_RST_B},
	{PCC3_RBASE, CAAM_PCC3_SLOT,		CLKSRC_NO_PCS, PCC_NO_DIV, PCC_HAS_RST_B},
	{PCC3_RBASE, XRDC_MGR_PCC3_SLOT,	CLKSRC_NO_PCS, PCC_NO_DIV, PCC_NO_RST_B},
	{PCC3_RBASE, SEMA42_1_PCC3_SLOT,	CLKSRC_PER_BUS, PCC_NO_DIV, PCC_NO_RST_B},
	{PCC3_RBASE, ROMCP1_PCC3_SLOT,		CLKSRC_NO_PCS, PCC_NO_DIV, PCC_NO_RST_B},
	{PCC3_RBASE, LPIT1_PCC3_SLOT,		CLKSRC_PER_BUS, PCC_HAS_DIV, PCC_HAS_RST_B},
	{PCC3_RBASE, TPM4_PCC3_SLOT,		CLKSRC_PER_BUS, PCC_HAS_DIV, PCC_HAS_RST_B},
	{PCC3_RBASE, TPM5_PCC3_SLOT,		CLKSRC_PER_BUS, PCC_HAS_DIV, PCC_HAS_RST_B},
	{PCC3_RBASE, FLEXIO1_PCC3_SLOT,		CLKSRC_PER_BUS, PCC_HAS_DIV, PCC_HAS_RST_B},
	{PCC3_RBASE, I3C2_PCC3_SLOT,		CLKSRC_PER_BUS, PCC_HAS_DIV, PCC_HAS_RST_B},
	{PCC3_RBASE, LPI2C4_PCC3_SLOT,		CLKSRC_PER_BUS, PCC_HAS_DIV, PCC_HAS_RST_B},
	{PCC3_RBASE, LPI2C5_PCC3_SLOT,		CLKSRC_PER_BUS, PCC_HAS_DIV, PCC_HAS_RST_B},
	{PCC3_RBASE, LPUART4_PCC3_SLOT,		CLKSRC_PER_BUS, PCC_HAS_DIV, PCC_HAS_RST_B},
	{PCC3_RBASE, LPUART5_PCC3_SLOT,		CLKSRC_PER_BUS, PCC_HAS_DIV, PCC_HAS_RST_B},
	{PCC3_RBASE, LPSPI4_PCC3_SLOT,		CLKSRC_PER_BUS, PCC_HAS_DIV, PCC_HAS_RST_B},
	{PCC3_RBASE, LPSPI5_PCC3_SLOT,		CLKSRC_PER_BUS, PCC_HAS_DIV, PCC_HAS_RST_B},
	{}
};

static struct pcc_entry pcc4_arrays[] = {
	{PCC4_RBASE, FLEXSPI2_PCC4_SLOT,	CLKSRC_PER_PLAT, PCC_HAS_DIV, PCC_HAS_RST_B },
	{PCC4_RBASE, TPM6_PCC4_SLOT,		CLKSRC_PER_BUS, PCC_HAS_DIV, PCC_HAS_RST_B },
	{PCC4_RBASE, TPM7_PCC4_SLOT,		CLKSRC_PER_BUS, PCC_HAS_DIV, PCC_HAS_RST_B },
	{PCC4_RBASE, LPI2C6_PCC4_SLOT,		CLKSRC_PER_BUS, PCC_HAS_DIV, PCC_HAS_RST_B },
	{PCC4_RBASE, LPI2C7_PCC4_SLOT,		CLKSRC_PER_BUS, PCC_HAS_DIV, PCC_HAS_RST_B },
	{PCC4_RBASE, LPUART6_PCC4_SLOT,		CLKSRC_PER_BUS, PCC_HAS_DIV, PCC_HAS_RST_B },
	{PCC4_RBASE, LPUART7_PCC4_SLOT,		CLKSRC_PER_BUS, PCC_HAS_DIV, PCC_HAS_RST_B },
	{PCC4_RBASE, SAI4_PCC4_SLOT,		CLKSRC_NO_PCS, PCC_NO_DIV, PCC_HAS_RST_B },
	{PCC4_RBASE, SAI5_PCC4_SLOT,		CLKSRC_NO_PCS, PCC_NO_DIV, PCC_HAS_RST_B },
	{PCC4_RBASE, PCTLE_PCC4_SLOT,		CLKSRC_NO_PCS, PCC_NO_DIV, PCC_NO_RST_B },
	{PCC4_RBASE, PCTLF_PCC4_SLOT,		CLKSRC_NO_PCS, PCC_NO_DIV, PCC_NO_RST_B },
	{PCC4_RBASE, SDHC0_PCC4_SLOT,		CLKSRC_PER_PLAT, PCC_HAS_DIV, PCC_HAS_RST_B },
	{PCC4_RBASE, SDHC1_PCC4_SLOT,		CLKSRC_PER_PLAT, PCC_HAS_DIV, PCC_HAS_RST_B },
	{PCC4_RBASE, SDHC2_PCC4_SLOT,		CLKSRC_PER_PLAT, PCC_HAS_DIV, PCC_HAS_RST_B },
	{PCC4_RBASE, USB0_PCC4_SLOT,		CLKSRC_NO_PCS, PCC_NO_DIV, PCC_HAS_RST_B },
	{PCC4_RBASE, USBPHY_PCC4_SLOT,		CLKSRC_NO_PCS, PCC_NO_DIV, PCC_HAS_RST_B },
	{PCC4_RBASE, USB1_PCC4_SLOT,		CLKSRC_NO_PCS, PCC_NO_DIV, PCC_HAS_RST_B },
	{PCC4_RBASE, USB1PHY_PCC4_SLOT,		CLKSRC_NO_PCS, PCC_NO_DIV, PCC_HAS_RST_B },
	{PCC4_RBASE, USB_XBAR_PCC4_SLOT,	CLKSRC_NO_PCS, PCC_NO_DIV, PCC_NO_RST_B },
	{PCC4_RBASE, ENET_PCC4_SLOT,		CLKSRC_NO_PCS, PCC_NO_DIV, PCC_HAS_RST_B },
	{PCC4_RBASE, SFA1_PCC4_SLOT,		CLKSRC_NO_PCS, PCC_NO_DIV, PCC_NO_RST_B },
	{PCC4_RBASE, RGPIOE_PCC4_SLOT,		CLKSRC_NO_PCS, PCC_NO_DIV, PCC_NO_RST_B },
	{PCC4_RBASE, RGPIOF_PCC4_SLOT,		CLKSRC_NO_PCS, PCC_NO_DIV, PCC_NO_RST_B },
	{}
};

static struct pcc_entry pcc5_arrays[] = {
	{PCC5_RBASE, DMA2_MP_PCC5_SLOT,		CLKSRC_NO_PCS, PCC_NO_DIV, PCC_NO_RST_B },
	{PCC5_RBASE, DMA2_CH0_PCC5_SLOT,	CLKSRC_NO_PCS, PCC_NO_DIV, PCC_NO_RST_B },
	{PCC5_RBASE, DMA2_CH1_PCC5_SLOT,	CLKSRC_NO_PCS, PCC_NO_DIV, PCC_NO_RST_B },
	{PCC5_RBASE, DMA2_CH2_PCC5_SLOT,	CLKSRC_NO_PCS, PCC_NO_DIV, PCC_NO_RST_B },
	{PCC5_RBASE, DMA2_CH3_PCC5_SLOT,	CLKSRC_NO_PCS, PCC_NO_DIV, PCC_NO_RST_B },
	{PCC5_RBASE, DMA2_CH4_PCC5_SLOT,	CLKSRC_NO_PCS, PCC_NO_DIV, PCC_NO_RST_B },
	{PCC5_RBASE, DMA2_CH5_PCC5_SLOT,	CLKSRC_NO_PCS, PCC_NO_DIV, PCC_NO_RST_B },
	{PCC5_RBASE, DMA2_CH6_PCC5_SLOT,	CLKSRC_NO_PCS, PCC_NO_DIV, PCC_NO_RST_B },
	{PCC5_RBASE, DMA2_CH7_PCC5_SLOT,	CLKSRC_NO_PCS, PCC_NO_DIV, PCC_NO_RST_B },
	{PCC5_RBASE, DMA2_CH8_PCC5_SLOT,	CLKSRC_NO_PCS, PCC_NO_DIV, PCC_NO_RST_B },
	{PCC5_RBASE, DMA2_CH9_PCC5_SLOT,	CLKSRC_NO_PCS, PCC_NO_DIV, PCC_NO_RST_B },
	{PCC5_RBASE, DMA2_CH10_PCC5_SLOT,	CLKSRC_NO_PCS, PCC_NO_DIV, PCC_NO_RST_B },
	{PCC5_RBASE, DMA2_CH11_PCC5_SLOT,	CLKSRC_NO_PCS, PCC_NO_DIV, PCC_NO_RST_B },
	{PCC5_RBASE, DMA2_CH12_PCC5_SLOT,	CLKSRC_NO_PCS, PCC_NO_DIV, PCC_NO_RST_B },
	{PCC5_RBASE, DMA2_CH13_PCC5_SLOT,	CLKSRC_NO_PCS, PCC_NO_DIV, PCC_NO_RST_B },
	{PCC5_RBASE, DMA2_CH14_PCC5_SLOT,	CLKSRC_NO_PCS, PCC_NO_DIV, PCC_NO_RST_B },
	{PCC5_RBASE, DMA2_CH15_PCC5_SLOT,	CLKSRC_NO_PCS, PCC_NO_DIV, PCC_NO_RST_B },
	{PCC5_RBASE, DMA2_CH16_PCC5_SLOT,	CLKSRC_NO_PCS, PCC_NO_DIV, PCC_NO_RST_B },
	{PCC5_RBASE, DMA2_CH17_PCC5_SLOT,	CLKSRC_NO_PCS, PCC_NO_DIV, PCC_NO_RST_B },
	{PCC5_RBASE, DMA2_CH18_PCC5_SLOT,	CLKSRC_NO_PCS, PCC_NO_DIV, PCC_NO_RST_B },
	{PCC5_RBASE, DMA2_CH19_PCC5_SLOT,	CLKSRC_NO_PCS, PCC_NO_DIV, PCC_NO_RST_B },
	{PCC5_RBASE, DMA2_CH20_PCC5_SLOT,	CLKSRC_NO_PCS, PCC_NO_DIV, PCC_NO_RST_B },
	{PCC5_RBASE, DMA2_CH21_PCC5_SLOT,	CLKSRC_NO_PCS, PCC_NO_DIV, PCC_NO_RST_B },
	{PCC5_RBASE, DMA2_CH22_PCC5_SLOT,	CLKSRC_NO_PCS, PCC_NO_DIV, PCC_NO_RST_B },
	{PCC5_RBASE, DMA2_CH23_PCC5_SLOT,	CLKSRC_NO_PCS, PCC_NO_DIV, PCC_NO_RST_B },
	{PCC5_RBASE, DMA2_CH24_PCC5_SLOT,	CLKSRC_NO_PCS, PCC_NO_DIV, PCC_NO_RST_B },
	{PCC5_RBASE, DMA2_CH25_PCC5_SLOT,	CLKSRC_NO_PCS, PCC_NO_DIV, PCC_NO_RST_B },
	{PCC5_RBASE, DMA2_CH26_PCC5_SLOT,	CLKSRC_NO_PCS, PCC_NO_DIV, PCC_NO_RST_B },
	{PCC5_RBASE, DMA2_CH27_PCC5_SLOT,	CLKSRC_NO_PCS, PCC_NO_DIV, PCC_NO_RST_B },
	{PCC5_RBASE, DMA2_CH28_PCC5_SLOT,	CLKSRC_NO_PCS, PCC_NO_DIV, PCC_NO_RST_B },
	{PCC5_RBASE, DMA2_CH29_PCC5_SLOT,	CLKSRC_NO_PCS, PCC_NO_DIV, PCC_NO_RST_B },
	{PCC5_RBASE, DMA2_CH30_PCC5_SLOT,	CLKSRC_NO_PCS, PCC_NO_DIV, PCC_NO_RST_B },
	{PCC5_RBASE, DMA2_CH31_PCC5_SLOT,	CLKSRC_NO_PCS, PCC_NO_DIV, PCC_NO_RST_B },
	{PCC5_RBASE, MU2_B_PCC5_SLOT,		CLKSRC_NO_PCS, PCC_NO_DIV, PCC_NO_RST_B },
	{PCC5_RBASE, MU3_B_PCC5_SLOT,		CLKSRC_NO_PCS, PCC_NO_DIV, PCC_NO_RST_B },
	{PCC5_RBASE, SEMA42_2_PCC5_SLOT,	CLKSRC_NO_PCS, PCC_NO_DIV, PCC_NO_RST_B },
	{PCC5_RBASE, CMC2_PCC5_SLOT,		CLKSRC_NO_PCS, PCC_NO_DIV, PCC_NO_RST_B },
	{PCC5_RBASE, AVD_SIM_PCC5_SLOT,		CLKSRC_NO_PCS, PCC_NO_DIV, PCC_NO_RST_B },
	{PCC5_RBASE, LPAV_CGC_PCC5_SLOT,	CLKSRC_NO_PCS, PCC_NO_DIV, PCC_NO_RST_B },
	{PCC5_RBASE, PCC5_PCC5_SLOT,		CLKSRC_NO_PCS, PCC_NO_DIV, PCC_NO_RST_B },
	{PCC5_RBASE, TPM8_PCC5_SLOT,		CLKSRC_PER_BUS, PCC_HAS_DIV, PCC_HAS_RST_B },
	{PCC5_RBASE, SAI6_PCC5_SLOT,		CLKSRC_NO_PCS, PCC_NO_DIV, PCC_HAS_RST_B },
	{PCC5_RBASE, SAI7_PCC5_SLOT,		CLKSRC_NO_PCS, PCC_NO_DIV, PCC_HAS_RST_B },
	{PCC5_RBASE, SPDIF_PCC5_SLOT,		CLKSRC_NO_PCS, PCC_NO_DIV, PCC_HAS_RST_B },
	{PCC5_RBASE, ISI_PCC5_SLOT,		CLKSRC_NO_PCS, PCC_NO_DIV, PCC_HAS_RST_B },
	{PCC5_RBASE, CSI_REGS_PCC5_SLOT,	CLKSRC_NO_PCS, PCC_NO_DIV, PCC_HAS_RST_B },
	{PCC5_RBASE, CSI_PCC5_SLOT,		CLKSRC_PER_PLAT, PCC_HAS_DIV, PCC_HAS_RST_B },
	{PCC5_RBASE, DSI_PCC5_SLOT,		CLKSRC_PER_PLAT, PCC_HAS_DIV, PCC_HAS_RST_B },
	{PCC5_RBASE, WDOG5_PCC5_SLOT,		CLKSRC_PER_BUS, PCC_HAS_DIV, PCC_HAS_RST_B },
	{PCC5_RBASE, EPDC_PCC5_SLOT,		CLKSRC_PER_PLAT, PCC_HAS_DIV, PCC_HAS_RST_B },
	{PCC5_RBASE, PXP_PCC5_SLOT,		CLKSRC_NO_PCS, PCC_NO_DIV, PCC_HAS_RST_B },
	{PCC5_RBASE, SFA2_PCC5_SLOT,		CLKSRC_NO_PCS, PCC_NO_DIV, PCC_NO_RST_B },
	{PCC5_RBASE, GPU2D_PCC5_SLOT,		CLKSRC_PER_PLAT, PCC_HAS_DIV, PCC_HAS_RST_B },
	{PCC5_RBASE, GPU3D_PCC5_SLOT,		CLKSRC_PER_PLAT, PCC_HAS_DIV, PCC_HAS_RST_B },
	{PCC5_RBASE, DCNANO_PCC5_SLOT,		CLKSRC_PER_PLAT, PCC_HAS_DIV, PCC_HAS_RST_B },
	{PCC5_RBASE, LPDDR4_PCC5_SLOT,		CLKSRC_NO_PCS, PCC_NO_DIV, PCC_HAS_RST_B },
	{PCC5_RBASE, CSI_CLK_UI_PCC5_SLOT,	CLKSRC_PER_PLAT, PCC_HAS_DIV, PCC_NO_RST_B },
	{PCC5_RBASE, CSI_CLK_ESC_PCC5_SLOT,	CLKSRC_PER_PLAT, PCC_HAS_DIV, PCC_NO_RST_B },
	{PCC5_RBASE, RGPIOD_PCC5_SLOT,		CLKSRC_NO_PCS, PCC_NO_DIV, PCC_NO_RST_B },
	{}
};

static int find_pcc_entry(int pcc_controller, int pcc_clk_slot, struct pcc_entry **out)
{
	struct pcc_entry *pcc_array;
	int index = 0;

	switch (pcc_controller) {
	case 1:
		pcc_array = pcc1_arrays;
		*out = &pcc1_arrays[0];
		break;
	case 3:
		pcc_array = pcc3_arrays;
		*out = &pcc3_arrays[0];
		break;
	case 4:
		pcc_array = pcc4_arrays;
		*out = &pcc4_arrays[0];
		break;
	case 5:
		pcc_array = pcc5_arrays;
		*out = &pcc5_arrays[0];
		break;
	default:
		printf("Not supported pcc_controller: %d\n", pcc_controller);
		return -EINVAL;
	}

	while (pcc_array->pcc_base) {
		if (pcc_array->pcc_slot == pcc_clk_slot)
			return index;

		pcc_array++;
		index++;
	}

	return -ENOENT;
}

int pcc_clock_enable(int pcc_controller, int pcc_clk_slot, bool enable)
{
	u32 val;
	void __iomem *reg;
	int clk;
	struct pcc_entry *pcc_array;

	clk = find_pcc_entry(pcc_controller, pcc_clk_slot, &pcc_array);
	if (clk < 0)
		return -EINVAL;

	reg = (void __iomem *)(uintptr_t)(pcc_array[clk].pcc_base + pcc_array[clk].pcc_slot * 4);

	val = readl(reg);

	debug("%s: clk %d, reg 0x%p, val 0x%x, enable %d\n", __func__, clk, reg, val, enable);

	if (!(val & PCC_PR_MASK) || (val & PCC_INUSE_MASK))
		return -EPERM;

	if (enable)
		val |= PCC_CGC_MASK;
	else
		val &= ~PCC_CGC_MASK;

	writel(val, reg);

	debug("%s: val 0x%x\n", __func__, val);

	return 0;
}

/* The clock source select needs clock is disabled */
int pcc_clock_sel(int pcc_controller, int pcc_clk_slot, enum cgc_clk src)
{
	u32 val, i, clksrc_type;
	void __iomem *reg;
	struct pcc_entry *pcc_array;
	enum cgc_clk *cgc_clk_array;
	int clk;

	clk = find_pcc_entry(pcc_controller, pcc_clk_slot, &pcc_array);
	if (clk < 0)
		return -EINVAL;

	reg = (void __iomem *)(uintptr_t)(pcc_array[clk].pcc_base + pcc_array[clk].pcc_slot * 4);

	clksrc_type = pcc_array[clk].clksrc;
	if (clksrc_type >= CLKSRC_NO_PCS) {
		printf("No PCS field for the PCC %d, clksrc type %d\n",
		       clk, clksrc_type);
		return -EPERM;
	}

	if (pcc_controller == 1)
		cgc_clk_array = pcc1_clksrc[clksrc_type];
	else if (pcc_controller == 3)
		cgc_clk_array = pcc3_clksrc[clksrc_type];
	else if (pcc_controller == 4)
		cgc_clk_array = pcc4_clksrc[clksrc_type];
	else
		cgc_clk_array = pcc5_clksrc[clksrc_type];

	for (i = 0; i < cgc_clk_NUM; i++) {
		if (cgc_clk_array[i] == src) {
			/* Find the clock src, then set it to PCS */
			break;
		}
	}

	if (i == cgc_clk_NUM) {
		printf("No parent in PCS of PCC %d, invalid scg_clk %d\n", clk, src);
		return -EINVAL;
	}

	val = readl(reg);

	debug("%s: clk %d, reg 0x%p, val 0x%x, clksrc_type %d\n",
	      __func__, clk, reg, val, clksrc_type);

	if (!(val & PCC_PR_MASK) || (val & PCC_INUSE_MASK) ||
	    (val & PCC_CGC_MASK)) {
		printf("Not permit to select clock source val = 0x%x\n", val);
		return -EPERM;
	}

	val &= ~PCC_PCS_MASK;
	val |= i << PCC_PCS_OFFSET;

	writel(val, reg);

	debug("%s: val 0x%x\n", __func__, val);

	return 0;
}

int pcc_clock_div_config(int pcc_controller, int pcc_clk_slot, bool frac, u8 div)
{
	u32 val;
	void __iomem *reg;
	struct pcc_entry *pcc_array;
	int clk;

	clk = find_pcc_entry(pcc_controller, pcc_clk_slot, &pcc_array);
	if (clk < 0)
		return -EINVAL;

	reg = (void __iomem *)(uintptr_t)(pcc_array[clk].pcc_base + pcc_array[clk].pcc_slot * 4);

	if (div > 8 || (div == 1 && frac != 0))
		return -EINVAL;

	if (pcc_array[clk].div >= PCC_NO_DIV) {
		printf("No DIV/FRAC field for the PCC %d\n", clk);
		return -EPERM;
	}

	val = readl(reg);

	if (!(val & PCC_PR_MASK) || (val & PCC_INUSE_MASK) ||
	    (val & PCC_CGC_MASK)) {
		printf("Not permit to set div/frac val = 0x%x\n", val);
		return -EPERM;
	}

	if (frac)
		val |= PCC_FRAC_MASK;
	else
		val &= ~PCC_FRAC_MASK;

	val &= ~PCC_PCD_MASK;
	val |= (div - 1) & PCC_PCD_MASK;

	writel(val, reg);

	return 0;
}

bool pcc_clock_is_enable(int pcc_controller, int pcc_clk_slot)
{
	u32 val;
	void __iomem *reg;
	struct pcc_entry *pcc_array;
	int clk;

	clk = find_pcc_entry(pcc_controller, pcc_clk_slot, &pcc_array);
	if (clk < 0)
		return -EINVAL;

	reg = (void __iomem *)(uintptr_t)(pcc_array[clk].pcc_base + pcc_array[clk].pcc_slot * 4);
	val = readl(reg);

	if ((val & PCC_INUSE_MASK) || (val & PCC_CGC_MASK))
		return true;

	return false;
}

int pcc_clock_get_clksrc(int pcc_controller, int pcc_clk_slot, enum cgc_clk *src)
{
	u32 val, clksrc_type;
	void __iomem *reg;
	struct pcc_entry *pcc_array;
	int clk;
	enum cgc_clk *cgc_clk_array;

	clk = find_pcc_entry(pcc_controller, pcc_clk_slot, &pcc_array);
	if (clk < 0)
		return -EINVAL;

	clksrc_type = pcc_array[clk].clksrc;
	if (clksrc_type >= CLKSRC_NO_PCS) {
		printf("No PCS field for the PCC %d, clksrc type %d\n",
		       pcc_clk_slot, clksrc_type);
		return -EPERM;
	}

	reg = (void __iomem *)(uintptr_t)(pcc_array[clk].pcc_base + pcc_array[clk].pcc_slot * 4);

	val = readl(reg);

	debug("%s: clk %d, reg 0x%p, val 0x%x, type %d\n",
	      __func__, pcc_clk_slot, reg, val, clksrc_type);

	if (!(val & PCC_PR_MASK)) {
		printf("This pcc slot is not present = 0x%x\n", val);
		return -EPERM;
	}

	val &= PCC_PCS_MASK;
	val = (val >> PCC_PCS_OFFSET);

	if (!val) {
		printf("Clock source is off\n");
		return -EIO;
	}

	if (pcc_controller == 3)
		cgc_clk_array = pcc3_clksrc[clksrc_type];
	else if (pcc_controller == 4)
		cgc_clk_array = pcc4_clksrc[clksrc_type];
	else
		cgc_clk_array = pcc5_clksrc[clksrc_type];

	*src = cgc_clk_array[val];

	debug("%s: parent cgc1 clk %d\n", __func__, *src);

	return 0;
}

int pcc_reset_peripheral(int pcc_controller, int pcc_clk_slot, bool reset)
{
	u32 val;
	void __iomem *reg;
	struct pcc_entry *pcc_array;
	int clk;

	clk = find_pcc_entry(pcc_controller, pcc_clk_slot, &pcc_array);
	if (clk < 0)
		return -EINVAL;

	if (pcc_array[clk].rst_b == PCC_NO_RST_B)
		return 0;

	reg = (void __iomem *)(uintptr_t)(pcc_array[clk].pcc_base + pcc_array[clk].pcc_slot * 4);

	val = readl(reg);

	debug("%s: clk %d, reg 0x%p, val 0x%x\n", __func__, pcc_clk_slot, reg, val);

	if (!(val & PCC_PR_MASK)) {
		printf("This pcc slot is not present = 0x%x\n", val);
		return -EPERM;
	}

	if (reset)
		val &= ~BIT(28);
	else
		val |= BIT(28);

	writel(val, reg);

	debug("%s: clk %d, reg 0x%p, val 0x%x\n", __func__, pcc_clk_slot, reg, val);

	return 0;
}

u32 pcc_clock_get_rate(int pcc_controller, int pcc_clk_slot)
{
	u32 val, rate, frac, div;
	void __iomem *reg;
	enum cgc_clk parent;
	int ret;
	int clk;
	struct pcc_entry *pcc_array;

	clk = find_pcc_entry(pcc_controller, pcc_clk_slot, &pcc_array);
	if (clk < 0)
		return -EINVAL;

	ret = pcc_clock_get_clksrc(pcc_controller, pcc_clk_slot, &parent);
	if (ret)
		return 0;

	rate = cgc_clk_get_rate(parent);

	debug("%s: parent rate %u\n", __func__, rate);

	if (pcc_array[clk].div == PCC_HAS_DIV) {
		reg = (void __iomem *)(uintptr_t)(pcc_array[clk].pcc_base +
						  pcc_array[clk].pcc_slot * 4);
		val = readl(reg);

		frac = (val & PCC_FRAC_MASK) >> PCC_FRAC_OFFSET;
		div = (val & PCC_PCD_MASK) >> PCC_PCD_OFFSET;

		/*
		 * Theoretically don't have overflow in the calc,
		 * the rate won't exceed 2G
		 */
		rate = rate * (frac + 1) / (div + 1);
	}

	debug("%s: rate %u\n", __func__, rate);
	return rate;
}
