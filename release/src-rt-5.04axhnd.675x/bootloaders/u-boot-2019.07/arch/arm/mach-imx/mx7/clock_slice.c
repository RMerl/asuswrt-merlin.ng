// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2015 Freescale Semiconductor, Inc.
 *
 * Author:
 *	Peng Fan <Peng.Fan@freescale.com>
 */

#include <common.h>
#include <div64.h>
#include <asm/io.h>
#include <linux/errno.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/crm_regs.h>
#include <asm/arch/clock.h>
#include <asm/arch/sys_proto.h>

struct mxc_ccm_reg *imx_ccm = (struct mxc_ccm_reg *)CCM_BASE_ADDR;

static struct clk_root_map root_array[] = {
	{ARM_A7_CLK_ROOT, CCM_CORE_CHANNEL,
	 {OSC_24M_CLK, PLL_ARM_MAIN_800M_CLK, PLL_ENET_MAIN_500M_CLK,
	  PLL_DRAM_MAIN_1066M_CLK, PLL_SYS_MAIN_480M_CLK,
	  PLL_SYS_PFD0_392M_CLK, PLL_AUDIO_MAIN_CLK, PLL_USB_MAIN_480M_CLK}
	},
	{ARM_M4_CLK_ROOT, CCM_BUS_CHANNEL,
	 {OSC_24M_CLK, PLL_SYS_MAIN_240M_CLK, PLL_ENET_MAIN_250M_CLK,
	  PLL_SYS_PFD2_270M_CLK, PLL_DRAM_MAIN_533M_CLK, PLL_AUDIO_MAIN_CLK,
	  PLL_VIDEO_MAIN_CLK, PLL_USB_MAIN_480M_CLK}
	},
	{ARM_M0_CLK_ROOT, CCM_BUS_CHANNEL,
	 {OSC_24M_CLK, PLL_SYS_MAIN_120M_CLK, PLL_ENET_MAIN_125M_CLK,
	  PLL_SYS_PFD2_135M_CLK, PLL_DRAM_MAIN_533M_CLK, PLL_AUDIO_MAIN_CLK,
	  PLL_VIDEO_MAIN_CLK, PLL_USB_MAIN_480M_CLK}
	},
	{MAIN_AXI_CLK_ROOT, CCM_BUS_CHANNEL,
	 {OSC_24M_CLK, PLL_SYS_PFD1_332M_CLK, PLL_DRAM_MAIN_533M_CLK,
	  PLL_ENET_MAIN_250M_CLK, PLL_SYS_PFD5_CLK, PLL_AUDIO_MAIN_CLK,
	  PLL_VIDEO_MAIN_CLK, PLL_SYS_PFD7_CLK}
	},
	{DISP_AXI_CLK_ROOT, CCM_BUS_CHANNEL,
	 {OSC_24M_CLK, PLL_SYS_PFD1_332M_CLK, PLL_DRAM_MAIN_533M_CLK,
	  PLL_ENET_MAIN_250M_CLK, PLL_SYS_PFD6_CLK, PLL_SYS_PFD7_CLK,
	  PLL_AUDIO_MAIN_CLK, PLL_VIDEO_MAIN_CLK}
	},
	{ENET_AXI_CLK_ROOT, CCM_IP_CHANNEL,
	 {OSC_24M_CLK, PLL_SYS_PFD2_270M_CLK, PLL_DRAM_MAIN_533M_CLK,
	  PLL_ENET_MAIN_250M_CLK, PLL_SYS_MAIN_240M_CLK, PLL_AUDIO_MAIN_CLK,
	  PLL_VIDEO_MAIN_CLK, PLL_SYS_PFD4_CLK}
	},
	{NAND_USDHC_BUS_CLK_ROOT, CCM_IP_CHANNEL,
	 {OSC_24M_CLK, PLL_SYS_PFD2_270M_CLK, PLL_DRAM_MAIN_533M_CLK,
	  PLL_SYS_MAIN_240M_CLK, PLL_SYS_PFD2_135M_CLK, PLL_SYS_PFD6_CLK,
	  PLL_ENET_MAIN_250M_CLK, PLL_AUDIO_MAIN_CLK}
	},
	{AHB_CLK_ROOT, CCM_AHB_CHANNEL,
	 {OSC_24M_CLK, PLL_SYS_PFD2_270M_CLK, PLL_DRAM_MAIN_533M_CLK,
	  PLL_SYS_PFD0_392M_CLK, PLL_ENET_MAIN_125M_CLK, PLL_USB_MAIN_480M_CLK,
	  PLL_AUDIO_MAIN_CLK, PLL_VIDEO_MAIN_CLK}
	},
	{DRAM_PHYM_CLK_ROOT, CCM_DRAM_PHYM_CHANNEL,
	 {PLL_DRAM_MAIN_1066M_CLK, DRAM_PHYM_ALT_CLK_ROOT}
	},
	{DRAM_CLK_ROOT, CCM_DRAM_CHANNEL,
	 {PLL_DRAM_MAIN_1066M_CLK, DRAM_ALT_CLK_ROOT}
	},
	{DRAM_PHYM_ALT_CLK_ROOT, CCM_IP_CHANNEL,
	 {OSC_24M_CLK, PLL_DRAM_MAIN_533M_CLK, PLL_SYS_MAIN_480M_CLK,
	  PLL_ENET_MAIN_500M_CLK, PLL_USB_MAIN_480M_CLK, PLL_SYS_PFD7_CLK,
	  PLL_AUDIO_MAIN_CLK, PLL_VIDEO_MAIN_CLK}
	},
	{DRAM_ALT_CLK_ROOT, CCM_IP_CHANNEL,
	 {OSC_24M_CLK, PLL_DRAM_MAIN_533M_CLK, PLL_SYS_MAIN_480M_CLK,
	  PLL_ENET_MAIN_500M_CLK, PLL_ENET_MAIN_250M_CLK,
	  PLL_SYS_PFD0_392M_CLK, PLL_AUDIO_MAIN_CLK, PLL_SYS_PFD2_270M_CLK}
	},
	{USB_HSIC_CLK_ROOT, CCM_IP_CHANNEL,
	 {OSC_24M_CLK, PLL_SYS_MAIN_480M_CLK, PLL_USB_MAIN_480M_CLK,
	  PLL_SYS_PFD3_CLK, PLL_SYS_PFD4_CLK, PLL_SYS_PFD5_CLK,
	  PLL_SYS_PFD6_CLK, PLL_SYS_PFD7_CLK}
	},
	{PCIE_CTRL_CLK_ROOT, CCM_IP_CHANNEL,
	 {OSC_24M_CLK, PLL_ENET_MAIN_250M_CLK, PLL_SYS_MAIN_240M_CLK,
	  PLL_SYS_PFD2_270M_CLK, PLL_DRAM_MAIN_533M_CLK,
	  PLL_ENET_MAIN_500M_CLK, PLL_SYS_PFD1_332M_CLK, PLL_SYS_PFD6_CLK}
	},
	{PCIE_PHY_CLK_ROOT, CCM_IP_CHANNEL,
	 {OSC_24M_CLK, PLL_ENET_MAIN_100M_CLK, PLL_ENET_MAIN_500M_CLK,
	  EXT_CLK_1, EXT_CLK_2, EXT_CLK_3,
	  EXT_CLK_4, PLL_SYS_PFD0_392M_CLK}
	},
	{EPDC_PIXEL_CLK_ROOT, CCM_IP_CHANNEL,
	 {OSC_24M_CLK, PLL_SYS_PFD1_332M_CLK, PLL_DRAM_MAIN_533M_CLK,
	  PLL_SYS_MAIN_480M_CLK, PLL_SYS_PFD5_CLK, PLL_SYS_PFD6_CLK,
	  PLL_SYS_PFD7_CLK, PLL_VIDEO_MAIN_CLK}
	},
	{LCDIF_PIXEL_CLK_ROOT, CCM_IP_CHANNEL,
	 {OSC_24M_CLK, PLL_SYS_PFD5_CLK, PLL_DRAM_MAIN_533M_CLK,
	  EXT_CLK_3, PLL_SYS_PFD4_CLK, PLL_SYS_PFD2_270M_CLK,
	  PLL_VIDEO_MAIN_CLK, PLL_USB_MAIN_480M_CLK}
	},
	{MIPI_DSI_EXTSER_CLK_ROOT, CCM_IP_CHANNEL,
	 {OSC_24M_CLK, PLL_SYS_PFD5_CLK, PLL_SYS_PFD3_CLK,
	  PLL_SYS_MAIN_480M_CLK, PLL_SYS_PFD0_196M_CLK, PLL_DRAM_MAIN_533M_CLK,
	  PLL_VIDEO_MAIN_CLK, PLL_AUDIO_MAIN_CLK}
	},
	{MIPI_CSI_WARP_CLK_ROOT, CCM_IP_CHANNEL,
	 {OSC_24M_CLK, PLL_SYS_PFD4_CLK, PLL_SYS_PFD3_CLK,
	  PLL_SYS_MAIN_480M_CLK, PLL_SYS_PFD0_196M_CLK, PLL_DRAM_MAIN_533M_CLK,
	  PLL_VIDEO_MAIN_CLK, PLL_AUDIO_MAIN_CLK}
	},
	{MIPI_DPHY_REF_CLK_ROOT, CCM_IP_CHANNEL,
	 {OSC_24M_CLK, PLL_SYS_MAIN_120M_CLK, PLL_DRAM_MAIN_533M_CLK,
	  PLL_SYS_PFD5_CLK, REF_1M_CLK, EXT_CLK_2,
	  PLL_VIDEO_MAIN_CLK, EXT_CLK_3}
	},
	{SAI1_CLK_ROOT, CCM_IP_CHANNEL,
	 {OSC_24M_CLK, PLL_SYS_PFD2_135M_CLK, PLL_AUDIO_MAIN_CLK,
	  PLL_DRAM_MAIN_533M_CLK, PLL_VIDEO_MAIN_CLK, PLL_SYS_PFD4_CLK,
	  PLL_ENET_MAIN_125M_CLK, EXT_CLK_2}
	},
	{SAI2_CLK_ROOT, CCM_IP_CHANNEL,
	 {OSC_24M_CLK, PLL_SYS_PFD2_135M_CLK, PLL_AUDIO_MAIN_CLK,
	  PLL_DRAM_MAIN_533M_CLK, PLL_VIDEO_MAIN_CLK, PLL_SYS_PFD4_CLK,
	  PLL_ENET_MAIN_125M_CLK, EXT_CLK_2}
	},
	{SAI3_CLK_ROOT, CCM_IP_CHANNEL,
	 {OSC_24M_CLK, PLL_SYS_PFD2_135M_CLK, PLL_AUDIO_MAIN_CLK,
	  PLL_DRAM_MAIN_533M_CLK, PLL_VIDEO_MAIN_CLK, PLL_SYS_PFD4_CLK,
	  PLL_ENET_MAIN_125M_CLK, EXT_CLK_3}
	},
	{SPDIF_CLK_ROOT, CCM_IP_CHANNEL,
	 {OSC_24M_CLK, PLL_SYS_PFD2_135M_CLK, PLL_AUDIO_MAIN_CLK,
	  PLL_DRAM_MAIN_533M_CLK, PLL_VIDEO_MAIN_CLK, PLL_SYS_PFD4_CLK,
	  PLL_ENET_MAIN_125M_CLK, EXT_CLK_3}
	},
	{ENET1_REF_CLK_ROOT, CCM_IP_CHANNEL,
	 {OSC_24M_CLK, PLL_ENET_MAIN_125M_CLK, PLL_ENET_MAIN_50M_CLK,
	  PLL_ENET_MAIN_25M_CLK, PLL_SYS_MAIN_120M_CLK, PLL_AUDIO_MAIN_CLK,
	  PLL_VIDEO_MAIN_CLK, EXT_CLK_4}
	},
	{ENET1_TIME_CLK_ROOT, CCM_IP_CHANNEL,
	 {OSC_24M_CLK, PLL_ENET_MAIN_100M_CLK, PLL_AUDIO_MAIN_CLK,
	  EXT_CLK_1, EXT_CLK_2, EXT_CLK_3,
	  EXT_CLK_4, PLL_VIDEO_MAIN_CLK}
	},
	{ENET2_REF_CLK_ROOT, CCM_IP_CHANNEL,
	 {OSC_24M_CLK, PLL_ENET_MAIN_125M_CLK, PLL_ENET_MAIN_50M_CLK,
	  PLL_ENET_MAIN_25M_CLK, PLL_SYS_MAIN_120M_CLK, PLL_AUDIO_MAIN_CLK,
	  PLL_VIDEO_MAIN_CLK, EXT_CLK_4}
	},
	{ENET2_TIME_CLK_ROOT, CCM_IP_CHANNEL,
	 {OSC_24M_CLK, PLL_ENET_MAIN_100M_CLK, PLL_AUDIO_MAIN_CLK,
	  EXT_CLK_1, EXT_CLK_2, EXT_CLK_3,
	  EXT_CLK_4, PLL_VIDEO_MAIN_CLK}
	},
	{ENET_PHY_REF_CLK_ROOT, CCM_IP_CHANNEL,
	 {OSC_24M_CLK, PLL_ENET_MAIN_25M_CLK, PLL_ENET_MAIN_50M_CLK,
	  PLL_ENET_MAIN_125M_CLK, PLL_DRAM_MAIN_533M_CLK, PLL_AUDIO_MAIN_CLK,
	  PLL_VIDEO_MAIN_CLK, PLL_SYS_PFD3_CLK}
	},
	{EIM_CLK_ROOT, CCM_IP_CHANNEL,
	 {OSC_24M_CLK, PLL_SYS_PFD2_135M_CLK, PLL_SYS_MAIN_120M_CLK,
	  PLL_DRAM_MAIN_533M_CLK, PLL_SYS_PFD2_270M_CLK, PLL_SYS_PFD3_CLK,
	  PLL_ENET_MAIN_125M_CLK, PLL_USB_MAIN_480M_CLK}
	},
	{NAND_CLK_ROOT, CCM_IP_CHANNEL,
	 {OSC_24M_CLK, PLL_SYS_MAIN_480M_CLK, PLL_DRAM_MAIN_533M_CLK,
	  PLL_SYS_PFD0_392M_CLK, PLL_SYS_PFD3_CLK, PLL_ENET_MAIN_500M_CLK,
	  PLL_ENET_MAIN_250M_CLK, PLL_VIDEO_MAIN_CLK}
	},
	{QSPI_CLK_ROOT, CCM_IP_CHANNEL,
	 {OSC_24M_CLK, PLL_SYS_PFD4_CLK, PLL_DRAM_MAIN_533M_CLK,
	  PLL_ENET_MAIN_500M_CLK, PLL_SYS_PFD3_CLK, PLL_SYS_PFD2_270M_CLK,
	  PLL_SYS_PFD6_CLK, PLL_SYS_PFD7_CLK}
	},
	{USDHC1_CLK_ROOT, CCM_IP_CHANNEL,
	 {OSC_24M_CLK, PLL_SYS_PFD0_392M_CLK, PLL_DRAM_MAIN_533M_CLK,
	  PLL_ENET_MAIN_500M_CLK, PLL_SYS_PFD4_CLK, PLL_SYS_PFD2_270M_CLK,
	  PLL_SYS_PFD6_CLK, PLL_SYS_PFD7_CLK}
	},
	{USDHC2_CLK_ROOT, CCM_IP_CHANNEL,
	 {OSC_24M_CLK, PLL_SYS_PFD0_392M_CLK, PLL_DRAM_MAIN_533M_CLK,
	  PLL_ENET_MAIN_500M_CLK, PLL_SYS_PFD4_CLK, PLL_SYS_PFD2_270M_CLK,
	  PLL_SYS_PFD6_CLK, PLL_SYS_PFD7_CLK}
	},
	{USDHC3_CLK_ROOT, CCM_IP_CHANNEL,
	 {OSC_24M_CLK, PLL_SYS_PFD0_392M_CLK, PLL_DRAM_MAIN_533M_CLK,
	  PLL_ENET_MAIN_500M_CLK, PLL_SYS_PFD4_CLK, PLL_SYS_PFD2_270M_CLK,
	  PLL_SYS_PFD6_CLK, PLL_SYS_PFD7_CLK}
	},
	{CAN1_CLK_ROOT, CCM_IP_CHANNEL,
	 {OSC_24M_CLK, PLL_SYS_MAIN_120M_CLK, PLL_DRAM_MAIN_533M_CLK,
	  PLL_SYS_MAIN_480M_CLK, PLL_ENET_MAIN_40M_CLK, PLL_USB_MAIN_480M_CLK,
	  EXT_CLK_1, EXT_CLK_4}
	},
	{CAN2_CLK_ROOT, CCM_IP_CHANNEL,
	 {OSC_24M_CLK, PLL_SYS_MAIN_120M_CLK, PLL_DRAM_MAIN_533M_CLK,
	  PLL_SYS_MAIN_480M_CLK, PLL_ENET_MAIN_40M_CLK, PLL_USB_MAIN_480M_CLK,
	  EXT_CLK_1, EXT_CLK_3}
	},
	{I2C1_CLK_ROOT, CCM_IP_CHANNEL,
	 {OSC_24M_CLK, PLL_SYS_MAIN_120M_CLK, PLL_ENET_MAIN_50M_CLK,
	  PLL_DRAM_MAIN_533M_CLK, PLL_AUDIO_MAIN_CLK, PLL_VIDEO_MAIN_CLK,
	  PLL_USB_MAIN_480M_CLK, PLL_SYS_PFD2_135M_CLK}
	},
	{I2C2_CLK_ROOT, CCM_IP_CHANNEL,
	 {OSC_24M_CLK, PLL_SYS_MAIN_120M_CLK, PLL_ENET_MAIN_50M_CLK,
	  PLL_DRAM_MAIN_533M_CLK, PLL_AUDIO_MAIN_CLK, PLL_VIDEO_MAIN_CLK,
	  PLL_USB_MAIN_480M_CLK, PLL_SYS_PFD2_135M_CLK}
	},
	{I2C3_CLK_ROOT, CCM_IP_CHANNEL,
	 {OSC_24M_CLK, PLL_SYS_MAIN_120M_CLK, PLL_ENET_MAIN_50M_CLK,
	  PLL_DRAM_MAIN_533M_CLK, PLL_AUDIO_MAIN_CLK, PLL_VIDEO_MAIN_CLK,
	  PLL_USB_MAIN_480M_CLK, PLL_SYS_PFD2_135M_CLK}
	},
	{I2C4_CLK_ROOT, CCM_IP_CHANNEL,
	 {OSC_24M_CLK, PLL_SYS_MAIN_120M_CLK, PLL_ENET_MAIN_50M_CLK,
	  PLL_DRAM_MAIN_533M_CLK, PLL_AUDIO_MAIN_CLK, PLL_VIDEO_MAIN_CLK,
	  PLL_USB_MAIN_480M_CLK, PLL_SYS_PFD2_135M_CLK}
	},
	{UART1_CLK_ROOT, CCM_IP_CHANNEL,
	 {OSC_24M_CLK, PLL_SYS_MAIN_240M_CLK, PLL_ENET_MAIN_40M_CLK,
	  PLL_ENET_MAIN_100M_CLK, PLL_SYS_MAIN_480M_CLK, EXT_CLK_2,
	  EXT_CLK_4, PLL_USB_MAIN_480M_CLK}
	},
	{UART2_CLK_ROOT, CCM_IP_CHANNEL,
	 {OSC_24M_CLK, PLL_SYS_MAIN_240M_CLK, PLL_ENET_MAIN_40M_CLK,
	  PLL_ENET_MAIN_100M_CLK, PLL_SYS_MAIN_480M_CLK, EXT_CLK_2,
	  EXT_CLK_3, PLL_USB_MAIN_480M_CLK}
	},
	{UART3_CLK_ROOT, CCM_IP_CHANNEL,
	 {OSC_24M_CLK, PLL_SYS_MAIN_240M_CLK, PLL_ENET_MAIN_40M_CLK,
	  PLL_ENET_MAIN_100M_CLK, PLL_SYS_MAIN_480M_CLK, EXT_CLK_2,
	  EXT_CLK_4, PLL_USB_MAIN_480M_CLK}
	},
	{UART4_CLK_ROOT, CCM_IP_CHANNEL,
	 {OSC_24M_CLK, PLL_SYS_MAIN_240M_CLK, PLL_ENET_MAIN_40M_CLK,
	  PLL_ENET_MAIN_100M_CLK, PLL_SYS_MAIN_480M_CLK, EXT_CLK_2,
	  EXT_CLK_3, PLL_USB_MAIN_480M_CLK}
	},
	{UART5_CLK_ROOT, CCM_IP_CHANNEL,
	 {OSC_24M_CLK, PLL_SYS_MAIN_240M_CLK, PLL_ENET_MAIN_40M_CLK,
	  PLL_ENET_MAIN_100M_CLK, PLL_SYS_MAIN_480M_CLK, EXT_CLK_2,
	  EXT_CLK_4, PLL_USB_MAIN_480M_CLK}
	},
	{UART6_CLK_ROOT, CCM_IP_CHANNEL,
	 {OSC_24M_CLK, PLL_SYS_MAIN_240M_CLK, PLL_ENET_MAIN_40M_CLK,
	  PLL_ENET_MAIN_100M_CLK, PLL_SYS_MAIN_480M_CLK, EXT_CLK_2,
	  EXT_CLK_3, PLL_USB_MAIN_480M_CLK}
	},
	{UART7_CLK_ROOT, CCM_IP_CHANNEL,
	 {OSC_24M_CLK, PLL_SYS_MAIN_240M_CLK, PLL_ENET_MAIN_40M_CLK,
	  PLL_ENET_MAIN_100M_CLK, PLL_SYS_MAIN_480M_CLK, EXT_CLK_2,
	  EXT_CLK_4, PLL_USB_MAIN_480M_CLK}
	},
	{ECSPI1_CLK_ROOT, CCM_IP_CHANNEL,
	 {OSC_24M_CLK, PLL_SYS_MAIN_240M_CLK, PLL_ENET_MAIN_40M_CLK,
	  PLL_SYS_MAIN_120M_CLK, PLL_SYS_MAIN_480M_CLK, PLL_SYS_PFD4_CLK,
	  PLL_ENET_MAIN_250M_CLK, PLL_USB_MAIN_480M_CLK}
	},
	{ECSPI2_CLK_ROOT, CCM_IP_CHANNEL,
	 {OSC_24M_CLK, PLL_SYS_MAIN_240M_CLK, PLL_ENET_MAIN_40M_CLK,
	  PLL_SYS_MAIN_120M_CLK, PLL_SYS_MAIN_480M_CLK, PLL_SYS_PFD4_CLK,
	  PLL_ENET_MAIN_250M_CLK, PLL_USB_MAIN_480M_CLK}
	},
	{ECSPI3_CLK_ROOT, CCM_IP_CHANNEL,
	 {OSC_24M_CLK, PLL_SYS_MAIN_240M_CLK, PLL_ENET_MAIN_40M_CLK,
	  PLL_SYS_MAIN_120M_CLK, PLL_SYS_MAIN_480M_CLK, PLL_SYS_PFD4_CLK,
	  PLL_ENET_MAIN_250M_CLK, PLL_USB_MAIN_480M_CLK}
	},
	{ECSPI4_CLK_ROOT, CCM_IP_CHANNEL,
	 {OSC_24M_CLK, PLL_SYS_MAIN_240M_CLK, PLL_ENET_MAIN_40M_CLK,
	  PLL_SYS_MAIN_120M_CLK, PLL_SYS_MAIN_480M_CLK, PLL_SYS_PFD4_CLK,
	  PLL_ENET_MAIN_250M_CLK, PLL_USB_MAIN_480M_CLK}
	},
	{PWM1_CLK_ROOT, CCM_IP_CHANNEL,
	 {OSC_24M_CLK, PLL_ENET_MAIN_100M_CLK, PLL_SYS_MAIN_120M_CLK,
	  PLL_ENET_MAIN_40M_CLK, PLL_AUDIO_MAIN_CLK, EXT_CLK_1,
	  REF_1M_CLK, PLL_VIDEO_MAIN_CLK}
	},
	{PWM2_CLK_ROOT, CCM_IP_CHANNEL,
	 {OSC_24M_CLK, PLL_ENET_MAIN_100M_CLK, PLL_SYS_MAIN_120M_CLK,
	  PLL_ENET_MAIN_40M_CLK, PLL_AUDIO_MAIN_CLK, EXT_CLK_1,
	  REF_1M_CLK, PLL_VIDEO_MAIN_CLK}
	},
	{PWM3_CLK_ROOT, CCM_IP_CHANNEL,
	 {OSC_24M_CLK, PLL_ENET_MAIN_100M_CLK, PLL_SYS_MAIN_120M_CLK,
	  PLL_ENET_MAIN_40M_CLK, PLL_AUDIO_MAIN_CLK, EXT_CLK_2,
	  REF_1M_CLK, PLL_VIDEO_MAIN_CLK}
	},
	{PWM4_CLK_ROOT, CCM_IP_CHANNEL,
	 {OSC_24M_CLK, PLL_ENET_MAIN_100M_CLK, PLL_SYS_MAIN_120M_CLK,
	  PLL_ENET_MAIN_40M_CLK, PLL_AUDIO_MAIN_CLK, EXT_CLK_2,
	  REF_1M_CLK, PLL_VIDEO_MAIN_CLK}
	},
	{FLEXTIMER1_CLK_ROOT, CCM_IP_CHANNEL,
	 {OSC_24M_CLK, PLL_ENET_MAIN_100M_CLK, PLL_SYS_MAIN_120M_CLK,
	  PLL_ENET_MAIN_40M_CLK, PLL_AUDIO_MAIN_CLK, EXT_CLK_3,
	  REF_1M_CLK, PLL_VIDEO_MAIN_CLK}
	},
	{FLEXTIMER2_CLK_ROOT, CCM_IP_CHANNEL,
	 {OSC_24M_CLK, PLL_ENET_MAIN_100M_CLK, PLL_SYS_MAIN_120M_CLK,
	  PLL_ENET_MAIN_40M_CLK, PLL_AUDIO_MAIN_CLK, EXT_CLK_3,
	  REF_1M_CLK, PLL_VIDEO_MAIN_CLK}
	},
	{SIM1_CLK_ROOT, CCM_IP_CHANNEL,
	 {OSC_24M_CLK, PLL_SYS_PFD2_135M_CLK, PLL_SYS_MAIN_120M_CLK,
	  PLL_DRAM_MAIN_533M_CLK, PLL_USB_MAIN_480M_CLK, PLL_AUDIO_MAIN_CLK,
	  PLL_ENET_MAIN_125M_CLK, PLL_SYS_PFD7_CLK}
	},
	{SIM2_CLK_ROOT, CCM_IP_CHANNEL,
	 {OSC_24M_CLK, PLL_SYS_PFD2_135M_CLK, PLL_SYS_MAIN_120M_CLK,
	  PLL_DRAM_MAIN_533M_CLK, PLL_USB_MAIN_480M_CLK, PLL_VIDEO_MAIN_CLK,
	  PLL_ENET_MAIN_125M_CLK, PLL_SYS_PFD7_CLK}
	},
	{GPT1_CLK_ROOT, CCM_IP_CHANNEL,
	 {OSC_24M_CLK, PLL_ENET_MAIN_100M_CLK, PLL_SYS_PFD0_392M_CLK,
	  PLL_ENET_MAIN_40M_CLK, PLL_VIDEO_MAIN_CLK, REF_1M_CLK,
	  PLL_AUDIO_MAIN_CLK, EXT_CLK_1}
	},
	{GPT2_CLK_ROOT, CCM_IP_CHANNEL,
	 {OSC_24M_CLK, PLL_ENET_MAIN_100M_CLK, PLL_SYS_PFD0_392M_CLK,
	  PLL_ENET_MAIN_40M_CLK, PLL_VIDEO_MAIN_CLK, REF_1M_CLK,
	  PLL_AUDIO_MAIN_CLK, EXT_CLK_2}
	},
	{GPT3_CLK_ROOT, CCM_IP_CHANNEL,
	 {OSC_24M_CLK, PLL_ENET_MAIN_100M_CLK, PLL_SYS_PFD0_392M_CLK,
	  PLL_ENET_MAIN_40M_CLK, PLL_VIDEO_MAIN_CLK, REF_1M_CLK,
	  PLL_AUDIO_MAIN_CLK, EXT_CLK_3}
	},
	{GPT4_CLK_ROOT, CCM_IP_CHANNEL,
	 {OSC_24M_CLK, PLL_ENET_MAIN_100M_CLK, PLL_SYS_PFD0_392M_CLK,
	  PLL_ENET_MAIN_40M_CLK, PLL_VIDEO_MAIN_CLK, REF_1M_CLK,
	  PLL_AUDIO_MAIN_CLK, EXT_CLK_4}
	},
	{TRACE_CLK_ROOT, CCM_IP_CHANNEL,
	 {OSC_24M_CLK, PLL_SYS_PFD2_135M_CLK, PLL_SYS_MAIN_120M_CLK,
	  PLL_DRAM_MAIN_533M_CLK, PLL_ENET_MAIN_125M_CLK, PLL_USB_MAIN_480M_CLK,
	  EXT_CLK_1, EXT_CLK_3}
	},
	{WDOG_CLK_ROOT, CCM_IP_CHANNEL,
	 {OSC_24M_CLK, PLL_SYS_PFD2_135M_CLK, PLL_SYS_MAIN_120M_CLK,
	  PLL_DRAM_MAIN_533M_CLK, PLL_ENET_MAIN_125M_CLK, PLL_USB_MAIN_480M_CLK,
	  REF_1M_CLK, PLL_SYS_PFD1_166M_CLK}
	},
	{CSI_MCLK_CLK_ROOT, CCM_IP_CHANNEL,
	 {OSC_24M_CLK, PLL_SYS_PFD2_135M_CLK, PLL_SYS_MAIN_120M_CLK,
	  PLL_DRAM_MAIN_533M_CLK, PLL_ENET_MAIN_125M_CLK, PLL_AUDIO_MAIN_CLK,
	  PLL_VIDEO_MAIN_CLK, PLL_USB_MAIN_480M_CLK}
	},
	{AUDIO_MCLK_CLK_ROOT, CCM_IP_CHANNEL,
	 {OSC_24M_CLK, PLL_SYS_PFD2_135M_CLK, PLL_SYS_MAIN_120M_CLK,
	  PLL_DRAM_MAIN_533M_CLK, PLL_ENET_MAIN_125M_CLK, PLL_AUDIO_MAIN_CLK,
	  PLL_VIDEO_MAIN_CLK, PLL_USB_MAIN_480M_CLK}
	},
	{WRCLK_CLK_ROOT, CCM_IP_CHANNEL,
	 {OSC_24M_CLK, PLL_ENET_MAIN_40M_CLK, PLL_DRAM_MAIN_533M_CLK,
	  PLL_USB_MAIN_480M_CLK, PLL_SYS_MAIN_240M_CLK, PLL_SYS_PFD2_270M_CLK,
	  PLL_ENET_MAIN_500M_CLK, PLL_SYS_PFD7_CLK}
	},
	{IPP_DO_CLKO1, CCM_IP_CHANNEL,
	 {OSC_24M_CLK, PLL_SYS_MAIN_480M_CLK, PLL_SYS_MAIN_240M_CLK,
	  PLL_SYS_PFD0_196M_CLK, PLL_SYS_PFD3_CLK, PLL_ENET_MAIN_500M_CLK,
	  PLL_DRAM_MAIN_533M_CLK, REF_1M_CLK}
	},
	{IPP_DO_CLKO2, CCM_IP_CHANNEL,
	 {OSC_24M_CLK, PLL_SYS_MAIN_240M_CLK, PLL_SYS_PFD0_392M_CLK,
	  PLL_SYS_PFD1_166M_CLK, PLL_SYS_PFD4_CLK, PLL_AUDIO_MAIN_CLK,
	  PLL_VIDEO_MAIN_CLK, OSC_32K_CLK}
	},
};

/* select which entry of root_array */
static int select(enum clk_root_index clock_id)
{
	int i, size;
	struct clk_root_map *p = root_array;

	size = ARRAY_SIZE(root_array);

	for (i = 0; i < size; i++, p++) {
		if (clock_id == p->entry)
			return i;
	}

	return -EINVAL;
}

static int src_supported(int entry, enum clk_root_src clock_src)
{
	int i, size;
	struct clk_root_map *p = &root_array[entry];

	if ((p->type == CCM_DRAM_PHYM_CHANNEL) || (p->type == CCM_DRAM_CHANNEL))
		size = 2;
	else
		size = 8;

	for (i = 0; i < size; i++) {
		if (p->src_mux[i] == clock_src)
			return i;
	}

	return -EINVAL;
}

/* Set src for clock root slice. */
int clock_set_src(enum clk_root_index clock_id, enum clk_root_src clock_src)
{
	int root_entry, src_entry;
	u32 reg;

	if (clock_id >= CLK_ROOT_MAX)
		return -EINVAL;

	root_entry = select(clock_id);
	if (root_entry < 0)
		return -EINVAL;

	src_entry = src_supported(root_entry, clock_src);
	if (src_entry < 0)
		return -EINVAL;

	reg = __raw_readl(&imx_ccm->root[clock_id].target_root);
	reg &= ~CLK_ROOT_MUX_MASK;
	reg |= src_entry << CLK_ROOT_MUX_SHIFT;
	__raw_writel(reg, &imx_ccm->root[clock_id].target_root);

	return 0;
}

/* Get src of a clock root slice. */
int clock_get_src(enum clk_root_index clock_id, enum clk_root_src *p_clock_src)
{
	u32 val;
	int root_entry;
	struct clk_root_map *p;

	if (clock_id >= CLK_ROOT_MAX)
		return -EINVAL;

	val = __raw_readl(&imx_ccm->root[clock_id].target_root);
	val &= CLK_ROOT_MUX_MASK;
	val >>= CLK_ROOT_MUX_SHIFT;

	root_entry = select(clock_id);
	if (root_entry < 0)
		return -EINVAL;

	p = &root_array[root_entry];
	*p_clock_src = p->src_mux[val];

	return 0;
}

int clock_set_prediv(enum clk_root_index clock_id, enum root_pre_div pre_div)
{
	int root_entry;
	struct clk_root_map *p;
	u32 reg;

	if (clock_id >= CLK_ROOT_MAX)
		return -EINVAL;

	root_entry = select(clock_id);
	if (root_entry < 0)
		return -EINVAL;

	p = &root_array[root_entry];

	if ((p->type == CCM_CORE_CHANNEL) ||
	    (p->type == CCM_DRAM_PHYM_CHANNEL) ||
	    (p->type == CCM_DRAM_CHANNEL)) {
		if (pre_div != CLK_ROOT_PRE_DIV1) {
			printf("Error pre div!\n");
			return -EINVAL;
		}
	}

	reg = __raw_readl(&imx_ccm->root[clock_id].target_root);
	reg &= ~CLK_ROOT_PRE_DIV_MASK;
	reg |= pre_div << CLK_ROOT_PRE_DIV_SHIFT;
	__raw_writel(reg, &imx_ccm->root[clock_id].target_root);

	return 0;
}

int clock_get_prediv(enum clk_root_index clock_id, enum root_pre_div *pre_div)
{
	u32 val;
	int root_entry;
	struct clk_root_map *p;

	if (clock_id >= CLK_ROOT_MAX)
		return -EINVAL;

	root_entry = select(clock_id);
	if (root_entry < 0)
		return -EINVAL;

	p = &root_array[root_entry];

	if ((p->type == CCM_CORE_CHANNEL) ||
	    (p->type == CCM_DRAM_PHYM_CHANNEL) ||
	    (p->type == CCM_DRAM_CHANNEL)) {
		*pre_div = 0;
		return 0;
	}

	val = __raw_readl(&imx_ccm->root[clock_id].target_root);
	val &= CLK_ROOT_PRE_DIV_MASK;
	val >>= CLK_ROOT_PRE_DIV_SHIFT;

	*pre_div = val;

	return 0;
}

int clock_set_postdiv(enum clk_root_index clock_id, enum root_post_div div)
{
	u32 reg;

	if (clock_id >= CLK_ROOT_MAX)
		return -EINVAL;

	if (clock_id == DRAM_PHYM_CLK_ROOT) {
		if (div != CLK_ROOT_POST_DIV1) {
			printf("Error post div!\n");
			return -EINVAL;
		}
	}

	/* Only 3 bit post div. */
	if ((clock_id == DRAM_CLK_ROOT) && (div > CLK_ROOT_POST_DIV7)) {
		printf("Error post div!\n");
		return -EINVAL;
	}

	reg = __raw_readl(&imx_ccm->root[clock_id].target_root);
	reg &= ~CLK_ROOT_POST_DIV_MASK;
	reg |= div << CLK_ROOT_POST_DIV_SHIFT;
	__raw_writel(reg, &imx_ccm->root[clock_id].target_root);

	return 0;
}

int clock_get_postdiv(enum clk_root_index clock_id, enum root_post_div *div)
{
	u32 val;

	if (clock_id >= CLK_ROOT_MAX)
		return -EINVAL;

	if (clock_id == DRAM_PHYM_CLK_ROOT) {
		*div = 0;
		return 0;
	}

	val = __raw_readl(&imx_ccm->root[clock_id].target_root);
	if (clock_id == DRAM_CLK_ROOT)
		val &= DRAM_CLK_ROOT_POST_DIV_MASK;
	else
		val &= CLK_ROOT_POST_DIV_MASK;
	val >>= CLK_ROOT_POST_DIV_SHIFT;

	*div = val;

	return 0;
}

int clock_set_autopostdiv(enum clk_root_index clock_id, enum root_auto_div div,
			  int auto_en)
{
	u32 val;
	int root_entry;
	struct clk_root_map *p;

	if (clock_id >= CLK_ROOT_MAX)
		return -EINVAL;

	root_entry = select(clock_id);
	if (root_entry < 0)
		return -EINVAL;

	p = &root_array[root_entry];

	if ((p->type != CCM_BUS_CHANNEL) && (p->type != CCM_AHB_CHANNEL)) {
		printf("Auto postdiv not supported.!\n");
		return -EINVAL;
	}

	/*
	 * Each time only one filed can be changed, no use target_root_set.
	 */
	val = __raw_readl(&imx_ccm->root[clock_id].target_root);
	val &= ~CLK_ROOT_AUTO_DIV_MASK;
	val |= (div << CLK_ROOT_AUTO_DIV_SHIFT);

	if (auto_en)
		val |= CLK_ROOT_AUTO_EN;
	else
		val &= ~CLK_ROOT_AUTO_EN;

	__raw_writel(val, &imx_ccm->root[clock_id].target_root);

	return 0;
}

int clock_get_autopostdiv(enum clk_root_index clock_id, enum root_auto_div *div,
			  int *auto_en)
{
	u32 val;
	int root_entry;
	struct clk_root_map *p;

	if (clock_id >= CLK_ROOT_MAX)
		return -EINVAL;

	root_entry = select(clock_id);
	if (root_entry < 0)
		return -EINVAL;

	p = &root_array[root_entry];

	/*
	 * Only bus/ahb channel supports auto div.
	 * If unsupported, just set auto_en and div with 0.
	 */
	if ((p->type != CCM_BUS_CHANNEL) && (p->type != CCM_AHB_CHANNEL)) {
		*auto_en = 0;
		*div = 0;
		return 0;
	}

	val = __raw_readl(&imx_ccm->root[clock_id].target_root);
	if ((val & CLK_ROOT_AUTO_EN_MASK) == 0)
		*auto_en = 0;
	else
		*auto_en = 1;

	val &= CLK_ROOT_AUTO_DIV_MASK;
	val >>= CLK_ROOT_AUTO_DIV_SHIFT;

	*div = val;

	return 0;
}

int clock_get_target_val(enum clk_root_index clock_id, u32 *val)
{
	if (clock_id >= CLK_ROOT_MAX)
		return -EINVAL;

	*val = __raw_readl(&imx_ccm->root[clock_id].target_root);

	return 0;
}

int clock_set_target_val(enum clk_root_index clock_id, u32 val)
{
	if (clock_id >= CLK_ROOT_MAX)
		return -EINVAL;

	__raw_writel(val, &imx_ccm->root[clock_id].target_root);

	return 0;
}

/* Auto_div and auto_en is ignored, they are rarely used. */
int clock_root_cfg(enum clk_root_index clock_id, enum root_pre_div pre_div,
		   enum root_post_div post_div, enum clk_root_src clock_src)
{
	u32 val;
	int root_entry, src_entry;
	struct clk_root_map *p;

	if (clock_id >= CLK_ROOT_MAX)
		return -EINVAL;

	root_entry = select(clock_id);
	if (root_entry < 0)
		return -EINVAL;

	p = &root_array[root_entry];

	if ((p->type == CCM_CORE_CHANNEL) ||
	    (p->type == CCM_DRAM_PHYM_CHANNEL) ||
	    (p->type == CCM_DRAM_CHANNEL)) {
		if (pre_div != CLK_ROOT_PRE_DIV1) {
			printf("Error pre div!\n");
			return -EINVAL;
		}
	}

	/* Only 3 bit post div. */
	if (p->type == CCM_DRAM_CHANNEL) {
		if (post_div > CLK_ROOT_POST_DIV7) {
			printf("Error post div!\n");
			return -EINVAL;
		}
	}

	if (p->type == CCM_DRAM_PHYM_CHANNEL) {
		if (post_div != CLK_ROOT_POST_DIV1) {
			printf("Error post div!\n");
			return -EINVAL;
		}
	}

	src_entry = src_supported(root_entry, clock_src);
	if (src_entry < 0)
		return -EINVAL;

	val = CLK_ROOT_ON | pre_div << CLK_ROOT_PRE_DIV_SHIFT |
	      post_div << CLK_ROOT_POST_DIV_SHIFT |
	      src_entry << CLK_ROOT_MUX_SHIFT;

	__raw_writel(val, &imx_ccm->root[clock_id].target_root);

	return 0;
}

int clock_root_enabled(enum clk_root_index clock_id)
{
	u32 val;

	if (clock_id >= CLK_ROOT_MAX)
		return -EINVAL;

	/*
	 * No enable bit for DRAM controller and PHY. Just return enabled.
	 */
	if ((clock_id == DRAM_PHYM_CLK_ROOT) || (clock_id == DRAM_CLK_ROOT))
		return 1;

	val = __raw_readl(&imx_ccm->root[clock_id].target_root);

	return (val & CLK_ROOT_ENABLE_MASK) ? 1 : 0;
}

/* CCGR gate operation */
int clock_enable(enum clk_ccgr_index index, bool enable)
{
	if (index >= CCGR_MAX)
		return -EINVAL;

	if (enable)
		__raw_writel(CCM_CLK_ON_MSK,
			     &imx_ccm->ccgr_array[index].ccgr_set);
	else
		__raw_writel(CCM_CLK_ON_MSK,
			     &imx_ccm->ccgr_array[index].ccgr_clr);

	return 0;
}
