/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2022 MediaTek Inc. All rights reserved.
 *
 * Author: Weijie Gao <weijie.gao@mediatek.com>
 */

#ifndef _MT7621_H_
#define _MT7621_H_

#define SYSCTL_BASE				0x1e000000
#define SYSCTL_SIZE				0x100
#define TIMER_BASE				0x1e000100
#define TIMER_SIZE				0x100
#define RBUS_BASE				0x1e000400
#define RBUS_SIZE				0x100
#define GPIO_BASE				0x1e000600
#define GPIO_SIZE				0x100
#define DMA_CFG_ARB_BASE			0x1e000800
#define DMA_CFG_ARB_SIZE			0x100
#define SPI_BASE				0x1e000b00
#define SPI_SIZE				0x100
#define UART1_BASE				0x1e000c00
#define UART1_SIZE				0x100
#define UART2_BASE				0x1e000d00
#define UART2_SIZE				0x100
#define UART3_BASE				0x1e000e00
#define UART3_SIZE				0x100
#define NFI_BASE				0x1e003000
#define NFI_SIZE				0x800
#define NFI_ECC_BASE				0x1e003800
#define NFI_ECC_SIZE				0x800
#define DRAMC_BASE				0x1e005000
#define DRAMC_SIZE				0x1000
#define FE_BASE					0x1e100000
#define FE_SIZE					0xe000
#define GMAC_BASE				0x1e110000
#define GMAC_SIZE				0x8000
#define SSUSB_BASE				0x1e1c0000
#define SSUSB_SIZE				0x40000

 /* GIC Base Address */
#define MIPS_GIC_BASE				0x1fbc0000

 /* CPC Base Address */
#define MIPS_CPC_BASE				0x1fbf0000

 /* Flash Memory-mapped Base Address */
#define FLASH_MMAP_BASE				0x1fc00000
#define TPL_INFO_OFFSET				0x40
#define TPL_INFO_MAGIC				0x31323637 /* Magic "7621" */

/* SRAM */
#define FE_SRAM_BASE1				0x8000
#define FE_SRAM_BASE2				0xa000

/* SYSCTL_BASE */
#define SYSCTL_CHIP_REV_ID_REG			0x0c
#define CPU_ID					0x20000
#define PKG_ID					0x10000
#define VER_ID_S				8
#define VER_ID_M				0xf00
#define ECO_ID_S				0
#define ECO_ID_M				0x0f

#define SYSCTL_SYSCFG0_REG			0x10
#define XTAL_MODE_SEL_S				6
#define XTAL_MODE_SEL_M				0x1c0
#define DRAM_TYPE				0x10
#define CHIP_MODE_S				0
#define CHIP_MODE_M				0x0f

#define BOOT_SRAM_BASE_REG			0x20

#define SYSCTL_CLKCFG0_REG			0x2c
#define CPU_CLK_SEL_S				30
#define CPU_CLK_SEL_M				0xc0000000
#define MPLL_CFG_SEL_S				23
#define MPLL_CFG_SEL_M				0x800000

#define SYSCTL_RSTCTL_REG			0x34
#define MCM_RST					0x04
#define SYS_RST					0x01

#define SYSCTL_CUR_CLK_STS_REG			0x44
#define CUR_CPU_FDIV_S				8
#define CUR_CPU_FDIV_M				0x1f00
#define CUR_CPU_FFRAC_S				0
#define CUR_CPU_FFRAC_M				0x1f

#define SYSCTL_GPIOMODE_REG			0x60
#define UART2_MODE_S				5
#define UART2_MODE_M				0x60
#define UART3_MODE_S				3
#define UART3_MODE_M				0x18
#define UART1_MODE				0x02

/* RBUS_BASE */
#define RBUS_DYN_CFG0_REG			0x0010
#define CPU_FDIV_S				8
#define CPU_FDIV_M				0x1f00
#define CPU_FFRAC_S				0
#define CPU_FFRAC_M				0x1f

/* DMA_CFG_ARB_BASE */
#define DMA_ROUTE_REG				0x000c

/* SPI_BASE */
#define SPI_SPACE_REG				0x003c
#define FS_SLAVE_SEL_S				12
#define FS_SLAVE_SEL_M				0x70000
#define FS_CLK_SEL_S				0
#define FS_CLK_SEL_M				0xfff

/* FE_BASE */
#define FE_RST_GLO_REG				0x0004
#define FE_PSE_RAM				0x04
#define FE_PSE_MEM_EN				0x02
#define FE_PSE_RESET				0x01

/* SSUSB_BASE */
#define SSUSB_MAC_CK_CTRL_REG			0x10784
#define SSUSB_MAC3_SYS_CK_GATE_MASK_TIME_S	16
#define SSUSB_MAC3_SYS_CK_GATE_MASK_TIME_M	0xff0000
#define SSUSB_MAC2_SYS_CK_GATE_MASK_TIME_S	8
#define SSUSB_MAC2_SYS_CK_GATE_MASK_TIME_M	0xff00
#define SSUSB_MAC3_SYS_CK_GATE_MODE_S		2
#define SSUSB_MAC3_SYS_CK_GATE_MODE_M		0x0c
#define SSUSB_MAC2_SYS_CK_GATE_MODE_S		0
#define SSUSB_MAC2_SYS_CK_GATE_MODE_M		0x03

#define SSUSB_B2_ROSC_0_REG			0x10a40
#define SSUSB_RING_OSC_CNTEND_S			23
#define SSUSB_RING_OSC_CNTEND_M			0xff800000
#define SSUSB_XTAL_OSC_CNTEND_S			16
#define SSUSB_XTAL_OSC_CNTEND_M			0x7f0000
#define SSUSB_RING_BYPASS_DET			0x01

#define SSUSB_B2_ROSC_1_REG			0x10a44
#define SSUSB_RING_OSC_FRC_RECAL_S		17
#define SSUSB_RING_OSC_FRC_RECAL_M		0x60000
#define SSUSB_RING_OSC_FRC_SEL			0x01

#define SSUSB_U3PHYA_1_REG			0x10b04
#define SSUSB_PCIE_CLKDRV_AMP_S			27
#define SSUSB_PCIE_CLKDRV_AMP_M			0x38000000
#define SSUSB_SYSPLL_FBSEL_S			2
#define SSUSB_SYSPLL_FBSEL_M			0x0c
#define SSUSB_SYSPLL_PREDIV_S			0
#define SSUSB_SYSPLL_PREDIV_M			0x03

#define SSUSB_U3PHYA_2_REG			0x10b08
#define SSUSB_SYSPLL_FBDIV_S			24
#define SSUSB_SYSPLL_FBDIV_M			0x7f000000
#define SSUSB_SYSPLL_VCO_DIV_SEL		0x200000
#define SSUSB_SYSPLL_FPEN			0x2000
#define SSUSB_SYSPLL_MONCK_EN			0x1000
#define SSUSB_SYSPLL_VOD_EN			0x200

#define SSUSB_U3PHYA_3_REG			0x10b10
#define SSUSB_SYSPLL_PCW_NCPO_S			1
#define SSUSB_SYSPLL_PCW_NCPO_M			0xfffffffe

#define SSUSB_U3PHYA_9_REG			0x10b24
#define SSUSB_PLL_SSC_PRD_S			0
#define SSUSB_PLL_SSC_PRD_M			0xffff

#define SSUSB_U3PHYA_11_REG			0x10b2c
#define SSUSB_EQ_CURSEL				0x1000000
#define SSUSB_RX_DAC_MUX_S			19
#define SSUSB_RX_DAC_MUX_M			0xf80000
#define SSUSB_PCIE_SIGDET_VTH_S			5
#define SSUSB_PCIE_SIGDET_VTH_M			0x60
#define SSUSB_PCIE_SIGDET_LPF_S			3
#define SSUSB_PCIE_SIGDET_LPF_M			0x18

#define DA_SSUSB_PLL_FBKDIV_REG			0x10c1c
#define SSUSB_PLL_FBKDIV_PE2H_S			24
#define SSUSB_PLL_FBKDIV_PE2H_M			0x7f000000
#define SSUSB_PLL_FBKDIV_PE1D_S			16
#define SSUSB_PLL_FBKDIV_PE1D_M			0x7f0000
#define SSUSB_PLL_FBKDIV_PE1H_S			8
#define SSUSB_PLL_FBKDIV_PE1H_M			0x7f00
#define SSUSB_PLL_FBKDIV_U3_S			0
#define SSUSB_PLL_FBKDIV_U3_M			0x7f

#define DA_SSUSB_U3PHYA_10_REG			0x10c20
#define SSUSB_PLL_PREDIV_PE1D_S			18
#define SSUSB_PLL_PREDIV_PE1D_M			0xc0000
#define SSUSB_PLL_PREDIV_U3_S			8
#define SSUSB_PLL_PREDIV_U3_M			0x300
#define SSUSB_PLL_FBKDI_S			0
#define SSUSB_PLL_FBKDI_M			0x07

#define DA_SSUSB_PLL_PCW_NCPO_REG		0x10c24
#define SSUSB_PLL_PCW_NCPO_U3_S			0
#define SSUSB_PLL_PCW_NCPO_U3_M			0x7fffffff

#define DA_SSUSB_PLL_SSC_DELTA1_REG		0x10c38
#define SSUSB_PLL_SSC_DELTA1_PE1H_S		16
#define SSUSB_PLL_SSC_DELTA1_PE1H_M		0xffff0000
#define SSUSB_PLL_SSC_DELTA1_U3_S		0
#define SSUSB_PLL_SSC_DELTA1_U3_M		0xffff

#define DA_SSUSB_U3PHYA_21_REG			0x10c40
#define SSUSB_PLL_SSC_DELTA_U3_S		16
#define SSUSB_PLL_SSC_DELTA_U3_M		0xffff0000
#define SSUSB_PLL_SSC_DELTA1_PE2D_S		0
#define SSUSB_PLL_SSC_DELTA1_PE2D_M		0xffff

/* MT7621 specific CM values */

/* GCR_REGx_BASE */
#define GCR_REG0_BASE_VALUE			0x1c000000
#define GCR_REG1_BASE_VALUE			0x60000000
#define GCR_REG2_BASE_VALUE			0x1c000000
#define GCR_REG3_BASE_VALUE			0x1c000000

/* GCR_REGx_MASK */
#define GCR_REG0_MASK_VALUE			0x0000fc00 /* 64M Bus */
#define GCR_REG1_MASK_VALUE			0x0000f000 /* 256M PCI Mem */
#define GCR_REG2_MASK_VALUE			0x0000fc00 /* unused */
#define GCR_REG3_MASK_VALUE			0x0000fc00 /* unused */

#ifndef __ASSEMBLY__
unsigned long get_xtal_mhz(void);
#endif

#endif /* _MT7621_H_ */
