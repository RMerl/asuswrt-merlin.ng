/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2013-2016, Freescale Semiconductor, Inc.
 */

#ifndef __ASM_ARCH_IMX_REGS_H__
#define __ASM_ARCH_IMX_REGS_H__

#define ARCH_MXC

#define IRAM_BASE_ADDR      0x3E800000	/* internal ram */
#define IRAM_SIZE           0x00400000	/* 4MB */

#define AIPS0_BASE_ADDR     (0x40000000UL)
#define AIPS1_BASE_ADDR     (0x40080000UL)

/* AIPS 0 */
#define AXBS_BASE_ADDR					(AIPS0_BASE_ADDR + 0x00000000)
#define CSE3_BASE_ADDR					(AIPS0_BASE_ADDR + 0x00001000)
#define EDMA_BASE_ADDR					(AIPS0_BASE_ADDR + 0x00002000)
#define XRDC_BASE_ADDR					(AIPS0_BASE_ADDR + 0x00004000)
#define SWT0_BASE_ADDR					(AIPS0_BASE_ADDR + 0x0000A000)
#define SWT1_BASE_ADDR					(AIPS0_BASE_ADDR + 0x0000B000)
#define STM0_BASE_ADDR					(AIPS0_BASE_ADDR + 0x0000D000)
#define NIC301_BASE_ADDR				(AIPS0_BASE_ADDR + 0x00010000)
#define GC3000_BASE_ADDR				(AIPS0_BASE_ADDR + 0x00020000)
#define DEC200_DECODER_BASE_ADDR		(AIPS0_BASE_ADDR + 0x00026000)
#define DEC200_ENCODER_BASE_ADDR		(AIPS0_BASE_ADDR + 0x00027000)
#define TWOD_ACE_BASE_ADDR				(AIPS0_BASE_ADDR + 0x00028000)
#define MIPI_CSI0_BASE_ADDR				(AIPS0_BASE_ADDR + 0x00030000)
#define DMAMUX0_BASE_ADDR				(AIPS0_BASE_ADDR + 0x00031000)
#define ENET_BASE_ADDR					(AIPS0_BASE_ADDR + 0x00032000)
#define FLEXRAY_BASE_ADDR				(AIPS0_BASE_ADDR + 0x00034000)
#define MMDC0_BASE_ADDR					(AIPS0_BASE_ADDR + 0x00036000)
#define MEW0_BASE_ADDR					(AIPS0_BASE_ADDR + 0x00037000)
#define MONITOR_DDR0_BASE_ADDR			(AIPS0_BASE_ADDR + 0x00038000)
#define MONITOR_CCI0_BASE_ADDR			(AIPS0_BASE_ADDR + 0x00039000)
#define PIT0_BASE_ADDR					(AIPS0_BASE_ADDR + 0x0003A000)
#define MC_CGM0_BASE_ADDR				(AIPS0_BASE_ADDR + 0x0003C000)
#define MC_CGM1_BASE_ADDR				(AIPS0_BASE_ADDR + 0x0003F000)
#define MC_CGM2_BASE_ADDR				(AIPS0_BASE_ADDR + 0x00042000)
#define MC_CGM3_BASE_ADDR				(AIPS0_BASE_ADDR + 0x00045000)
#define MC_RGM_BASE_ADDR				(AIPS0_BASE_ADDR + 0x00048000)
#define MC_ME_BASE_ADDR					(AIPS0_BASE_ADDR + 0x0004A000)
#define MC_PCU_BASE_ADDR				(AIPS0_BASE_ADDR + 0x0004B000)
#define ADC0_BASE_ADDR					(AIPS0_BASE_ADDR + 0x0004D000)
#define FLEXTIMER_BASE_ADDR				(AIPS0_BASE_ADDR + 0x0004F000)
#define I2C0_BASE_ADDR					(AIPS0_BASE_ADDR + 0x00051000)
#define LINFLEXD0_BASE_ADDR				(AIPS0_BASE_ADDR + 0x00053000)
#define FLEXCAN0_BASE_ADDR				(AIPS0_BASE_ADDR + 0x00055000)
#define SPI0_BASE_ADDR					(AIPS0_BASE_ADDR + 0x00057000)
#define SPI2_BASE_ADDR					(AIPS0_BASE_ADDR + 0x00059000)
#define CRC0_BASE_ADDR					(AIPS0_BASE_ADDR + 0x0005B000)
#define USDHC_BASE_ADDR					(AIPS0_BASE_ADDR + 0x0005D000)
#define OCOTP_CONTROLLER_BASE_ADDR		(AIPS0_BASE_ADDR + 0x0005F000)
#define WKPU_BASE_ADDR					(AIPS0_BASE_ADDR + 0x00063000)
#define VIU0_BASE_ADDR					(AIPS0_BASE_ADDR + 0x00064000)
#define HPSMI_SRAM_CONTROLLER_BASE_ADDR	(AIPS0_BASE_ADDR + 0x00068000)
#define SIUL2_BASE_ADDR					(AIPS0_BASE_ADDR + 0x0006C000)
#define SIPI_BASE_ADDR					(AIPS0_BASE_ADDR + 0x00074000)
#define LFAST_BASE_ADDR					(AIPS0_BASE_ADDR + 0x00078000)
#define SSE_BASE_ADDR					(AIPS0_BASE_ADDR + 0x00079000)
#define SRC_SOC_BASE_ADDR				(AIPS0_BASE_ADDR + 0x0007C000)

/* AIPS 1 */
#define ERM_BASE_ADDR					(AIPS1_BASE_ADDR + 0X000000000)
#define MSCM_BASE_ADDR					(AIPS1_BASE_ADDR + 0X000001000)
#define SEMA42_BASE_ADDR				(AIPS1_BASE_ADDR + 0X000002000)
#define INTC_MON_BASE_ADDR				(AIPS1_BASE_ADDR + 0X000003000)
#define SWT2_BASE_ADDR					(AIPS1_BASE_ADDR + 0X000004000)
#define SWT3_BASE_ADDR					(AIPS1_BASE_ADDR + 0X000005000)
#define SWT4_BASE_ADDR					(AIPS1_BASE_ADDR + 0X000006000)
#define STM1_BASE_ADDR					(AIPS1_BASE_ADDR + 0X000007000)
#define EIM_BASE_ADDR					(AIPS1_BASE_ADDR + 0X000008000)
#define APB_BASE_ADDR					(AIPS1_BASE_ADDR + 0X000009000)
#define XBIC_BASE_ADDR					(AIPS1_BASE_ADDR + 0X000012000)
#define MIPI_BASE_ADDR					(AIPS1_BASE_ADDR + 0X000020000)
#define DMAMUX1_BASE_ADDR				(AIPS1_BASE_ADDR + 0X000021000)
#define MMDC1_BASE_ADDR					(AIPS1_BASE_ADDR + 0X000022000)
#define MEW1_BASE_ADDR					(AIPS1_BASE_ADDR + 0X000023000)
#define DDR1_BASE_ADDR					(AIPS1_BASE_ADDR + 0X000024000)
#define CCI1_BASE_ADDR					(AIPS1_BASE_ADDR + 0X000025000)
#define QUADSPI0_BASE_ADDR				(AIPS1_BASE_ADDR + 0X000026000)
#define PIT1_BASE_ADDR					(AIPS1_BASE_ADDR + 0X00002A000)
#define FCCU_BASE_ADDR					(AIPS1_BASE_ADDR + 0X000030000)
#define FLEXTIMER_FTM1_BASE_ADDR		(AIPS1_BASE_ADDR + 0X000036000)
#define I2C1_BASE_ADDR					(AIPS1_BASE_ADDR + 0X000038000)
#define I2C2_BASE_ADDR					(AIPS1_BASE_ADDR + 0X00003A000)
#define LINFLEXD1_BASE_ADDR				(AIPS1_BASE_ADDR + 0X00003C000)
#define FLEXCAN1_BASE_ADDR				(AIPS1_BASE_ADDR + 0X00003E000)
#define SPI1_BASE_ADDR					(AIPS1_BASE_ADDR + 0X000040000)
#define SPI3_BASE_ADDR					(AIPS1_BASE_ADDR + 0X000042000)
#define IPL_BASE_ADDR					(AIPS1_BASE_ADDR + 0X000043000)
#define CGM_CMU_BASE_ADDR				(AIPS1_BASE_ADDR + 0X000044000)
#define PMC_BASE_ADDR					(AIPS1_BASE_ADDR + 0X000048000)
#define CRC1_BASE_ADDR					(AIPS1_BASE_ADDR + 0X00004C000)
#define TMU_BASE_ADDR					(AIPS1_BASE_ADDR + 0X00004E000)
#define VIU1_BASE_ADDR					(AIPS1_BASE_ADDR + 0X000050000)
#define JPEG_BASE_ADDR					(AIPS1_BASE_ADDR + 0X000054000)
#define H264_DEC_BASE_ADDR				(AIPS1_BASE_ADDR + 0X000058000)
#define H264_ENC_BASE_ADDR				(AIPS1_BASE_ADDR + 0X00005C000)
#define MEMU_BASE_ADDR					(AIPS1_BASE_ADDR + 0X000060000)
#define STCU_BASE_ADDR					(AIPS1_BASE_ADDR + 0X000064000)
#define SLFTST_CTRL_BASE_ADDR			(AIPS1_BASE_ADDR + 0X000066000)
#define MCT_BASE_ADDR					(AIPS1_BASE_ADDR + 0X000068000)
#define REP_BASE_ADDR					(AIPS1_BASE_ADDR + 0X00006A000)
#define MBIST_CONTROLLER_BASE_ADDR		(AIPS1_BASE_ADDR + 0X00006C000)
#define BOOT_LOADER_BASE_ADDR			(AIPS1_BASE_ADDR + 0X00006F000)

/* TODO Remove this after the IOMUX framework is implemented */
#define IOMUXC_BASE_ADDR SIUL2_BASE_ADDR

/* MUX mode and PAD ctrl are in one register */
#define CONFIG_IOMUX_SHARE_CONF_REG

#define FEC_QUIRK_ENET_MAC
#define I2C_QUIRK_REG

/* MSCM interrupt router */
#define MSCM_IRSPRC_CPn_EN		3
#define MSCM_IRSPRC_NUM			176
#define MSCM_CPXTYPE_RYPZ_MASK		0xFF
#define MSCM_CPXTYPE_RYPZ_OFFSET	0
#define MSCM_CPXTYPE_PERS_MASK		0xFFFFFF00
#define MSCM_CPXTYPE_PERS_OFFSET	8
#define MSCM_CPXTYPE_PERS_A53		0x413533
#define MSCM_CPXTYPE_PERS_CM4		0x434d34

#if !(defined(__KERNEL_STRICT_NAMES) || defined(__ASSEMBLY__))
#include <asm/types.h>

/* System Reset Controller (SRC) */
struct src {
	u32 bmr1;
	u32 bmr2;
	u32 gpr1_boot;
	u32 reserved_0x00C[61];
	u32 gpr1;
	u32 gpr2;
	u32 gpr3;
	u32 gpr4;
	u32 gpr5;
	u32 gpr6;
	u32 gpr7;
	u32 reserved_0x11C[1];
	u32 gpr9;
	u32 gpr10;
	u32 gpr11;
	u32 gpr12;
	u32 gpr13;
	u32 gpr14;
	u32 gpr15;
	u32 gpr16;
	u32 reserved_0x140[1];
	u32 gpr17;
	u32 gpr18;
	u32 gpr19;
	u32 gpr20;
	u32 gpr21;
	u32 gpr22;
	u32 gpr23;
	u32 gpr24;
	u32 gpr25;
	u32 gpr26;
	u32 gpr27;
	u32 reserved_0x16C[5];
	u32 pcie_config1;
	u32 ddr_self_ref_ctrl;
	u32 pcie_config0;
	u32 reserved_0x18C[4];
	u32 soc_misc_config2;
};

/* SRC registers definitions */

/* SRC_GPR1 */
#define SRC_GPR1_PLL_SOURCE(pll,val)( ((val) & SRC_GPR1_PLL_SOURCE_MASK) << \
										(SRC_GPR1_PLL_OFFSET + (pll)) )
#define SRC_GPR1_PLL_SOURCE_MASK	(0x1)

#define SRC_GPR1_PLL_OFFSET			(27)
#define SRC_GPR1_FIRC_CLK_SOURCE	(0x0)
#define SRC_GPR1_XOSC_CLK_SOURCE	(0x1)

/* Periodic Interrupt Timer (PIT) */
struct pit_reg {
	u32 mcr;
	u32 recv0[55];
	u32 ltmr64h;
	u32 ltmr64l;
	u32 recv1[6];
	u32 ldval0;
	u32 cval0;
	u32 tctrl0;
	u32 tflg0;
	u32 ldval1;
	u32 cval1;
	u32 tctrl1;
	u32 tflg1;
	u32 ldval2;
	u32 cval2;
	u32 tctrl2;
	u32 tflg2;
	u32 ldval3;
	u32 cval3;
	u32 tctrl3;
	u32 tflg3;
	u32 ldval4;
	u32 cval4;
	u32 tctrl4;
	u32 tflg4;
	u32 ldval5;
	u32 cval5;
	u32 tctrl5;
	u32 tflg5;
};

/* Watchdog Timer (WDOG) */
struct wdog_regs {
	u32 cr;
	u32 ir;
	u32 to;
	u32 wn;
	u32 sr;
	u32 co;
	u32 sk;
};

/* UART */
struct linflex_fsl {
	u32 lincr1;
	u32 linier;
	u32 linsr;
	u32 linesr;
	u32 uartcr;
	u32 uartsr;
	u32 lintcsr;
	u32 linocr;
	u32 lintocr;
	u32 linfbrr;
	u32 linibrr;
	u32 lincfr;
	u32 lincr2;
	u32 bidr;
	u32 bdrl;
	u32 bdrm;
	u32 ifer;
	u32 ifmi;
	u32 ifmr;
	u32 ifcr0;
	u32 ifcr1;
	u32 ifcr2;
	u32 ifcr3;
	u32 ifcr4;
	u32 ifcr5;
	u32 ifcr6;
	u32 ifcr7;
	u32 ifcr8;
	u32 ifcr9;
	u32 ifcr10;
	u32 ifcr11;
	u32 ifcr12;
	u32 ifcr13;
	u32 ifcr14;
	u32 ifcr15;
	u32 gcr;
	u32 uartpto;
	u32 uartcto;
	u32 dmatxe;
	u32 dmarxe;
};

/* MSCM Interrupt Router */
struct mscm_ir {
	u32 cpxtype;		/* Processor x Type Register                    */
	u32 cpxnum;		/* Processor x Number Register                  */
	u32 cpxmaster;		/* Processor x Master Number Register   */
	u32 cpxcount;		/* Processor x Count Register                   */
	u32 cpxcfg0;		/* Processor x Configuration 0 Register */
	u32 cpxcfg1;		/* Processor x Configuration 1 Register */
	u32 cpxcfg2;		/* Processor x Configuration 2 Register */
	u32 cpxcfg3;		/* Processor x Configuration 3 Register */
	u32 cp0type;		/* Processor 0 Type Register                    */
	u32 cp0num;		/* Processor 0 Number Register                  */
	u32 cp0master;		/* Processor 0 Master Number Register   */
	u32 cp0count;		/* Processor 0 Count Register                   */
	u32 cp0cfg0;		/* Processor 0 Configuration 0 Register */
	u32 cp0cfg1;		/* Processor 0 Configuration 1 Register */
	u32 cp0cfg2;		/* Processor 0 Configuration 2 Register */
	u32 cp0cfg3;		/* Processor 0 Configuration 3 Register */
	u32 cp1type;		/* Processor 1 Type Register                    */
	u32 cp1num;		/* Processor 1 Number Register                  */
	u32 cp1master;		/* Processor 1 Master Number Register   */
	u32 cp1count;		/* Processor 1 Count Register                   */
	u32 cp1cfg0;		/* Processor 1 Configuration 0 Register */
	u32 cp1cfg1;		/* Processor 1 Configuration 1 Register */
	u32 cp1cfg2;		/* Processor 1 Configuration 2 Register */
	u32 cp1cfg3;		/* Processor 1 Configuration 3 Register */
	u32 reserved_0x060[232];
	u32 ocmdr0;		/* On-Chip Memory Descriptor Register   */
	u32 reserved_0x404[2];
	u32 ocmdr3;		/* On-Chip Memory Descriptor Register   */
	u32 reserved_0x410[28];
	u32 tcmdr[4];		/* Generic Tightly Coupled Memory Descriptor Register   */
	u32 reserved_0x490[28];
	u32 cpce0;		/* Core Parity Checking Enable Register 0                               */
	u32 reserved_0x504[191];
	u32 ircp0ir;		/* Interrupt Router CP0 Interrupt Register                              */
	u32 ircp1ir;		/* Interrupt Router CP1 Interrupt Register                              */
	u32 reserved_0x808[6];
	u32 ircpgir;		/* Interrupt Router CPU Generate Interrupt Register             */
	u32 reserved_0x824[23];
	u16 irsprc[176];	/* Interrupt Router Shared Peripheral Routing Control Register  */
	u32 reserved_0x9e0[136];
	u32 iahbbe0;		/* Gasket Burst Enable Register                                                 */
	u32 reserved_0xc04[63];
	u32 ipcge;		/* Interconnect Parity Checking Global Enable Register  */
	u32 reserved_0xd04[3];
	u32 ipce[4];		/* Interconnect Parity Checking Enable Register                 */
	u32 reserved_0xd20[8];
	u32 ipcgie;		/* Interconnect Parity Checking Global Injection Enable Register        */
	u32 reserved_0xd44[3];
	u32 ipcie[4];		/* Interconnect Parity Checking Injection Enable Register       */
};

#endif /* __ASSEMBLER__ */

#endif /* __ASM_ARCH_IMX_REGS_H__ */
