/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2011 Freescale Semiconductor, Inc. All Rights Reserved.
 */

#ifndef __ASM_ARCH_MX6_IMX_REGS_H__
#define __ASM_ARCH_MX6_IMX_REGS_H__

#define ARCH_MXC

#define ROMCP_ARB_BASE_ADDR             0x00000000
#define ROMCP_ARB_END_ADDR              0x000FFFFF

#ifdef CONFIG_MX6SL
#define GPU_2D_ARB_BASE_ADDR            0x02200000
#define GPU_2D_ARB_END_ADDR             0x02203FFF
#define OPENVG_ARB_BASE_ADDR            0x02204000
#define OPENVG_ARB_END_ADDR             0x02207FFF
#elif (defined(CONFIG_MX6SX) || defined(CONFIG_MX6UL) || defined(CONFIG_MX6ULL))
#define CAAM_ARB_BASE_ADDR              0x00100000
#define CAAM_ARB_END_ADDR               0x00107FFF
#define GPU_ARB_BASE_ADDR               0x01800000
#define GPU_ARB_END_ADDR                0x01803FFF
#define APBH_DMA_ARB_BASE_ADDR          0x01804000
#define APBH_DMA_ARB_END_ADDR           0x0180BFFF
#define M4_BOOTROM_BASE_ADDR			0x007F8000

#elif !defined(CONFIG_MX6SLL)
#define CAAM_ARB_BASE_ADDR              0x00100000
#define CAAM_ARB_END_ADDR               0x00103FFF
#define APBH_DMA_ARB_BASE_ADDR          0x00110000
#define APBH_DMA_ARB_END_ADDR           0x00117FFF
#define HDMI_ARB_BASE_ADDR              0x00120000
#define HDMI_ARB_END_ADDR               0x00128FFF
#define GPU_3D_ARB_BASE_ADDR            0x00130000
#define GPU_3D_ARB_END_ADDR             0x00133FFF
#define GPU_2D_ARB_BASE_ADDR            0x00134000
#define GPU_2D_ARB_END_ADDR             0x00137FFF
#define DTCP_ARB_BASE_ADDR              0x00138000
#define DTCP_ARB_END_ADDR               0x0013BFFF
#endif	/* CONFIG_MX6SL */

#define MXS_APBH_BASE			APBH_DMA_ARB_BASE_ADDR
#define MXS_GPMI_BASE			(APBH_DMA_ARB_BASE_ADDR + 0x02000)
#define MXS_BCH_BASE			(APBH_DMA_ARB_BASE_ADDR + 0x04000)

/* GPV - PL301 configuration ports */
#if (defined(CONFIG_MX6SX) || \
	defined(CONFIG_MX6UL) || defined(CONFIG_MX6ULL) || \
	defined(CONFIG_MX6SL) || defined(CONFIG_MX6SLL))
#define GPV2_BASE_ADDR                  0x00D00000
#define GPV3_BASE_ADDR			0x00E00000
#define GPV4_BASE_ADDR			0x00F00000
#define GPV5_BASE_ADDR			0x01000000
#define GPV6_BASE_ADDR			0x01100000
#define PCIE_ARB_BASE_ADDR              0x08000000
#define PCIE_ARB_END_ADDR               0x08FFFFFF

#else
#define GPV2_BASE_ADDR			0x00200000
#define GPV3_BASE_ADDR			0x00300000
#define GPV4_BASE_ADDR			0x00800000
#define PCIE_ARB_BASE_ADDR              0x01000000
#define PCIE_ARB_END_ADDR               0x01FFFFFF
#endif

#define IRAM_BASE_ADDR			0x00900000
#define SCU_BASE_ADDR                   0x00A00000
#define IC_INTERFACES_BASE_ADDR         0x00A00100
#define GLOBAL_TIMER_BASE_ADDR          0x00A00200
#define PRIVATE_TIMERS_WD_BASE_ADDR     0x00A00600
#define IC_DISTRIBUTOR_BASE_ADDR        0x00A01000
#define L2_PL310_BASE			0x00A02000
#define GPV0_BASE_ADDR                  0x00B00000
#define GPV1_BASE_ADDR                  0x00C00000

#define AIPS1_ARB_BASE_ADDR             0x02000000
#define AIPS1_ARB_END_ADDR              0x020FFFFF
#define AIPS2_ARB_BASE_ADDR             0x02100000
#define AIPS2_ARB_END_ADDR              0x021FFFFF
/* AIPS3 only on i.MX6SX */
#define AIPS3_ARB_BASE_ADDR             0x02200000
#define AIPS3_ARB_END_ADDR              0x022FFFFF
#ifdef CONFIG_MX6SX
#define WEIM_ARB_BASE_ADDR              0x50000000
#define WEIM_ARB_END_ADDR               0x57FFFFFF
#define QSPI0_AMBA_BASE                0x60000000
#define QSPI0_AMBA_END                 0x6FFFFFFF
#define QSPI1_AMBA_BASE                0x70000000
#define QSPI1_AMBA_END                 0x7FFFFFFF
#elif (defined(CONFIG_MX6UL) || defined(CONFIG_MX6ULL))
#define WEIM_ARB_BASE_ADDR              0x50000000
#define WEIM_ARB_END_ADDR               0x57FFFFFF
#define QSPI0_AMBA_BASE                 0x60000000
#define QSPI0_AMBA_END                  0x6FFFFFFF
#elif !defined(CONFIG_MX6SLL)
#define SATA_ARB_BASE_ADDR              0x02200000
#define SATA_ARB_END_ADDR               0x02203FFF
#define OPENVG_ARB_BASE_ADDR            0x02204000
#define OPENVG_ARB_END_ADDR             0x02207FFF
#define HSI_ARB_BASE_ADDR               0x02208000
#define HSI_ARB_END_ADDR                0x0220BFFF
#define IPU1_ARB_BASE_ADDR              0x02400000
#define IPU1_ARB_END_ADDR               0x027FFFFF
#define IPU2_ARB_BASE_ADDR              0x02800000
#define IPU2_ARB_END_ADDR               0x02BFFFFF
#define WEIM_ARB_BASE_ADDR              0x08000000
#define WEIM_ARB_END_ADDR               0x0FFFFFFF
#endif

#if (defined(CONFIG_MX6SLL) || defined(CONFIG_MX6SL) || \
	defined(CONFIG_MX6SX) || \
	defined(CONFIG_MX6UL) || defined(CONFIG_MX6ULL))
#define MMDC0_ARB_BASE_ADDR             0x80000000
#define MMDC0_ARB_END_ADDR              0xFFFFFFFF
#define MMDC1_ARB_BASE_ADDR             0xC0000000
#define MMDC1_ARB_END_ADDR              0xFFFFFFFF
#else
#define MMDC0_ARB_BASE_ADDR             0x10000000
#define MMDC0_ARB_END_ADDR              0x7FFFFFFF
#define MMDC1_ARB_BASE_ADDR             0x80000000
#define MMDC1_ARB_END_ADDR              0xFFFFFFFF
#endif

#ifndef CONFIG_MX6SX
#define IPU_SOC_BASE_ADDR		IPU1_ARB_BASE_ADDR
#define IPU_SOC_OFFSET			0x00200000
#endif

/* Defines for Blocks connected via AIPS (SkyBlue) */
#define ATZ1_BASE_ADDR              AIPS1_ARB_BASE_ADDR
#define ATZ2_BASE_ADDR              AIPS2_ARB_BASE_ADDR
#define ATZ3_BASE_ADDR              AIPS3_ARB_BASE_ADDR
#define AIPS1_BASE_ADDR             AIPS1_ON_BASE_ADDR
#define AIPS2_BASE_ADDR             AIPS2_ON_BASE_ADDR
#define AIPS3_BASE_ADDR             AIPS3_ON_BASE_ADDR

#define SPDIF_BASE_ADDR             (ATZ1_BASE_ADDR + 0x04000)
#define ECSPI1_BASE_ADDR            (ATZ1_BASE_ADDR + 0x08000)
#define ECSPI2_BASE_ADDR            (ATZ1_BASE_ADDR + 0x0C000)
#define ECSPI3_BASE_ADDR            (ATZ1_BASE_ADDR + 0x10000)
#define ECSPI4_BASE_ADDR            (ATZ1_BASE_ADDR + 0x14000)

#define MX6SL_UART5_BASE_ADDR       (ATZ1_BASE_ADDR + 0x18000)
#define MX6SLL_UART4_BASE_ADDR      (ATZ1_BASE_ADDR + 0x18000)
#define MX6UL_UART7_BASE_ADDR       (ATZ1_BASE_ADDR + 0x18000)
#define MX6SL_UART2_BASE_ADDR       (ATZ1_BASE_ADDR + 0x24000)
#define MX6SLL_UART2_BASE_ADDR      (ATZ1_BASE_ADDR + 0x24000)
#define MX6UL_UART8_BASE_ADDR       (ATZ1_BASE_ADDR + 0x24000)
#define MX6SL_UART3_BASE_ADDR       (ATZ1_BASE_ADDR + 0x34000)
#define MX6SLL_UART3_BASE_ADDR      (ATZ1_BASE_ADDR + 0x34000)
#define MX6SL_UART4_BASE_ADDR       (ATZ1_BASE_ADDR + 0x38000)

#ifndef CONFIG_MX6SX
#define ECSPI5_BASE_ADDR            (ATZ1_BASE_ADDR + 0x18000)
#endif
#define UART1_IPS_BASE_ADDR         (ATZ1_BASE_ADDR + 0x20000)
#define UART1_BASE                  (ATZ1_BASE_ADDR + 0x20000)
#define ESAI1_BASE_ADDR             (ATZ1_BASE_ADDR + 0x24000)
#define UART8_BASE                  (ATZ1_BASE_ADDR + 0x24000)
#define SSI1_BASE_ADDR              (ATZ1_BASE_ADDR + 0x28000)
#define SSI2_BASE_ADDR              (ATZ1_BASE_ADDR + 0x2C000)
#define SSI3_BASE_ADDR              (ATZ1_BASE_ADDR + 0x30000)
#define ASRC_BASE_ADDR              (ATZ1_BASE_ADDR + 0x34000)

#ifndef CONFIG_MX6SX
#define SPBA_BASE_ADDR              (ATZ1_BASE_ADDR + 0x3C000)
#define VPU_BASE_ADDR               (ATZ1_BASE_ADDR + 0x40000)
#endif
#define AIPS1_ON_BASE_ADDR          (ATZ1_BASE_ADDR + 0x7C000)

#define AIPS1_OFF_BASE_ADDR         (ATZ1_BASE_ADDR + 0x80000)
#define PWM1_BASE_ADDR              (AIPS1_OFF_BASE_ADDR + 0x0000)
#define PWM2_BASE_ADDR              (AIPS1_OFF_BASE_ADDR + 0x4000)
#define PWM3_BASE_ADDR              (AIPS1_OFF_BASE_ADDR + 0x8000)
#define PWM4_BASE_ADDR              (AIPS1_OFF_BASE_ADDR + 0xC000)
#define CAN1_BASE_ADDR              (AIPS1_OFF_BASE_ADDR + 0x10000)
#define CAN2_BASE_ADDR              (AIPS1_OFF_BASE_ADDR + 0x14000)
/* QOSC on i.MX6SLL */
#define QOSC_BASE_ADDR              (AIPS1_OFF_BASE_ADDR + 0x14000)
#define GPT1_BASE_ADDR              (AIPS1_OFF_BASE_ADDR + 0x18000)
#define GPIO1_BASE_ADDR             (AIPS1_OFF_BASE_ADDR + 0x1C000)
#define GPIO2_BASE_ADDR             (AIPS1_OFF_BASE_ADDR + 0x20000)
#define GPIO3_BASE_ADDR             (AIPS1_OFF_BASE_ADDR + 0x24000)
#define GPIO4_BASE_ADDR             (AIPS1_OFF_BASE_ADDR + 0x28000)
#define GPIO5_BASE_ADDR             (AIPS1_OFF_BASE_ADDR + 0x2C000)
#define MX6UL_SNVS_LP_BASE_ADDR     (AIPS1_OFF_BASE_ADDR + 0x30000)
#define GPIO6_BASE_ADDR             (AIPS1_OFF_BASE_ADDR + 0x30000)
#define GPIO7_BASE_ADDR             (AIPS1_OFF_BASE_ADDR + 0x34000)
#define KPP_BASE_ADDR               (AIPS1_OFF_BASE_ADDR + 0x38000)
#define WDOG1_BASE_ADDR             (AIPS1_OFF_BASE_ADDR + 0x3C000)
#define WDOG2_BASE_ADDR             (AIPS1_OFF_BASE_ADDR + 0x40000)
#define ANATOP_BASE_ADDR            (AIPS1_OFF_BASE_ADDR + 0x48000)
#define USB_PHY0_BASE_ADDR          (AIPS1_OFF_BASE_ADDR + 0x49000)
#define USB_PHY1_BASE_ADDR          (AIPS1_OFF_BASE_ADDR + 0x4a000)
#define CCM_BASE_ADDR               (AIPS1_OFF_BASE_ADDR + 0x44000)
#define SNVS_BASE_ADDR              (AIPS1_OFF_BASE_ADDR + 0x4C000)
#define EPIT1_BASE_ADDR             (AIPS1_OFF_BASE_ADDR + 0x50000)
#define EPIT2_BASE_ADDR             (AIPS1_OFF_BASE_ADDR + 0x54000)
#define SRC_BASE_ADDR               (AIPS1_OFF_BASE_ADDR + 0x58000)
#define GPC_BASE_ADDR               (AIPS1_OFF_BASE_ADDR + 0x5C000)
#define IOMUXC_BASE_ADDR            (AIPS1_OFF_BASE_ADDR + 0x60000)
#define IOMUXC_GPR_BASE_ADDR        (AIPS1_OFF_BASE_ADDR + 0x64000)
#ifdef CONFIG_MX6SLL
#define CSI_BASE_ADDR               (AIPS1_OFF_BASE_ADDR + 0x68000)
#define SDMA_PORT_HOST_BASE_ADDR    (AIPS1_OFF_BASE_ADDR + 0x6C000)
#define PXP_BASE_ADDR               (AIPS1_OFF_BASE_ADDR + 0x70000)
#define EPDC_BASE_ADDR              (AIPS1_OFF_BASE_ADDR + 0x74000)
#define DCP_BASE_ADDR               (AIPS1_OFF_BASE_ADDR + 0x7C000)
#elif defined(CONFIG_MX6SL)
#define CSI_BASE_ADDR               (AIPS1_OFF_BASE_ADDR + 0x64000)
#define SIPIX_BASE_ADDR             (AIPS1_OFF_BASE_ADDR + 0x68000)
#define SDMA_PORT_HOST_BASE_ADDR    (AIPS1_OFF_BASE_ADDR + 0x6C000)
#elif defined(CONFIG_MX6SX)
#define CANFD1_BASE_ADDR            (AIPS1_OFF_BASE_ADDR + 0x68000)
#define SDMA_BASE_ADDR              (AIPS1_OFF_BASE_ADDR + 0x6C000)
#define CANFD2_BASE_ADDR            (AIPS1_OFF_BASE_ADDR + 0x70000)
#define SEMAPHORE1_BASE_ADDR        (AIPS1_OFF_BASE_ADDR + 0x74000)
#define SEMAPHORE2_BASE_ADDR        (AIPS1_OFF_BASE_ADDR + 0x78000)
#define RDC_BASE_ADDR               (AIPS1_OFF_BASE_ADDR + 0x7C000)
#else
#define DCIC1_BASE_ADDR             (AIPS1_OFF_BASE_ADDR + 0x64000)
#define DCIC2_BASE_ADDR             (AIPS1_OFF_BASE_ADDR + 0x68000)
#define DMA_REQ_PORT_HOST_BASE_ADDR (AIPS1_OFF_BASE_ADDR + 0x6C000)
#endif

#define MX6SL_LCDIF_BASE_ADDR      (AIPS1_OFF_BASE_ADDR + 0x78000)
#define MX6SLL_LCDIF_BASE_ADDR      (AIPS1_OFF_BASE_ADDR + 0x78000)

#define AIPS2_ON_BASE_ADDR          (ATZ2_BASE_ADDR + 0x7C000)
#define AIPS2_OFF_BASE_ADDR         (ATZ2_BASE_ADDR + 0x80000)
#define AIPS3_ON_BASE_ADDR          (ATZ3_BASE_ADDR + 0x7C000)
#define AIPS3_OFF_BASE_ADDR         (ATZ3_BASE_ADDR + 0x80000)
#if defined(CONFIG_MX6UL)
#define CAAM_BASE_ADDR              (ATZ2_BASE_ADDR + 0x40000)
#else
#define CAAM_BASE_ADDR              (ATZ2_BASE_ADDR)
#endif
#define ARM_BASE_ADDR		    (ATZ2_BASE_ADDR + 0x40000)

#define CONFIG_SYS_FSL_SEC_OFFSET   0
#define CONFIG_SYS_FSL_SEC_ADDR     (CAAM_BASE_ADDR + \
				     CONFIG_SYS_FSL_SEC_OFFSET)
#define CONFIG_SYS_FSL_JR0_OFFSET   0x1000
#define CONFIG_SYS_FSL_JR0_ADDR     (CAAM_BASE_ADDR + \
				     CONFIG_SYS_FSL_JR0_OFFSET)
#define CONFIG_SYS_FSL_MAX_NUM_OF_SEC	1

#define USB_PL301_BASE_ADDR         (AIPS2_OFF_BASE_ADDR + 0x0000)
#define USB_BASE_ADDR               (AIPS2_OFF_BASE_ADDR + 0x4000)

#define ENET_BASE_ADDR              (AIPS2_OFF_BASE_ADDR + 0x8000)
#ifdef CONFIG_MX6SL
#define MSHC_IPS_BASE_ADDR          (AIPS2_OFF_BASE_ADDR + 0xC000)
#else
#define MLB_BASE_ADDR               (AIPS2_OFF_BASE_ADDR + 0xC000)
#endif

#define USDHC1_BASE_ADDR            (AIPS2_OFF_BASE_ADDR + 0x10000)
#define USDHC2_BASE_ADDR            (AIPS2_OFF_BASE_ADDR + 0x14000)
#define USDHC3_BASE_ADDR            (AIPS2_OFF_BASE_ADDR + 0x18000)
#define USDHC4_BASE_ADDR            (AIPS2_OFF_BASE_ADDR + 0x1C000)
#define I2C1_BASE_ADDR              (AIPS2_OFF_BASE_ADDR + 0x20000)
#define I2C2_BASE_ADDR              (AIPS2_OFF_BASE_ADDR + 0x24000)
#define I2C3_BASE_ADDR              (AIPS2_OFF_BASE_ADDR + 0x28000)
#define ROMCP_BASE_ADDR             (AIPS2_OFF_BASE_ADDR + 0x2C000)
#define MMDC_P0_BASE_ADDR           (AIPS2_OFF_BASE_ADDR + 0x30000)
/* i.MX6SL/SLL */
#define RNGB_IPS_BASE_ADDR          (AIPS2_OFF_BASE_ADDR + 0x34000)
#if (defined(CONFIG_MX6UL) || defined(CONFIG_MX6ULL))
#define ENET2_BASE_ADDR             (AIPS1_OFF_BASE_ADDR + 0x34000)
#else
/* i.MX6SX */
#define ENET2_BASE_ADDR             (AIPS2_OFF_BASE_ADDR + 0x34000)
#endif
/* i.MX6DQ/SDL */
#define MMDC_P1_BASE_ADDR           (AIPS2_OFF_BASE_ADDR + 0x34000)

#define WEIM_BASE_ADDR              (AIPS2_OFF_BASE_ADDR + 0x38000)
#define OCOTP_BASE_ADDR             (AIPS2_OFF_BASE_ADDR + 0x3C000)
#define CSU_BASE_ADDR               (AIPS2_OFF_BASE_ADDR + 0x40000)
#ifdef CONFIG_MX6SLL
#define IOMUXC_GPR_SNVS_BASE_ADDR    (AIPS2_OFF_BASE_ADDR + 0x44000)
#define IOMUXC_SNVS_BASE_ADDR        (AIPS2_OFF_BASE_ADDR + 0x48000)
#endif
#define IP2APB_PERFMON1_BASE_ADDR   (AIPS2_OFF_BASE_ADDR + 0x44000)
#define IP2APB_PERFMON2_BASE_ADDR   (AIPS2_OFF_BASE_ADDR + 0x48000)
#define MX6UL_LCDIF1_BASE_ADDR      (AIPS2_OFF_BASE_ADDR + 0x48000)
#define MX6ULL_LCDIF1_BASE_ADDR     (AIPS2_OFF_BASE_ADDR + 0x48000)
#ifdef CONFIG_MX6SX
#define DEBUG_MONITOR_BASE_ADDR     (AIPS2_OFF_BASE_ADDR + 0x4C000)
#else
#define IP2APB_PERFMON3_BASE_ADDR   (AIPS2_OFF_BASE_ADDR + 0x4C000)
#endif
#define IP2APB_TZASC1_BASE_ADDR     (AIPS2_OFF_BASE_ADDR + 0x50000)
#if (defined(CONFIG_MX6UL) || defined(CONFIG_MX6ULL))
#define SCTR_BASE_ADDR              (AIPS2_OFF_BASE_ADDR + 0x5C000)
#define QSPI0_BASE_ADDR             (AIPS2_OFF_BASE_ADDR + 0x60000)
#define UART6_BASE_ADDR             (AIPS2_OFF_BASE_ADDR + 0x7C000)
#elif defined(CONFIG_MX6SX)
#define SAI1_BASE_ADDR              (AIPS2_OFF_BASE_ADDR + 0x54000)
#define AUDMUX_BASE_ADDR            (AIPS2_OFF_BASE_ADDR + 0x58000)
#define SAI2_BASE_ADDR              (AIPS2_OFF_BASE_ADDR + 0x5C000)
#define QSPI0_BASE_ADDR             (AIPS2_OFF_BASE_ADDR + 0x60000)
#define QSPI1_BASE_ADDR             (AIPS2_OFF_BASE_ADDR + 0x64000)
#else
#define IP2APB_TZASC2_BASE_ADDR     (AIPS2_OFF_BASE_ADDR + 0x54000)
#define MIPI_CSI2_BASE_ADDR         (AIPS2_OFF_BASE_ADDR + 0x5C000)
#define MIPI_DSI_BASE_ADDR          (AIPS2_OFF_BASE_ADDR + 0x60000)
#define VDOA_BASE_ADDR              (AIPS2_OFF_BASE_ADDR + 0x64000)
#endif
#define MX6UL_WDOG3_BASE_ADDR       (AIPS2_OFF_BASE_ADDR + 0x64000)
#define UART2_BASE                  (AIPS2_OFF_BASE_ADDR + 0x68000)
#define UART3_BASE                  (AIPS2_OFF_BASE_ADDR + 0x6C000)
#define UART4_BASE                  (AIPS2_OFF_BASE_ADDR + 0x70000)
#define UART5_BASE                  (AIPS2_OFF_BASE_ADDR + 0x74000)
#define I2C4_BASE_ADDR              (AIPS2_OFF_BASE_ADDR + 0x78000)
#define IP2APB_USBPHY1_BASE_ADDR    (AIPS2_OFF_BASE_ADDR + 0x78000)
#define IP2APB_USBPHY2_BASE_ADDR    (AIPS2_OFF_BASE_ADDR + 0x7C000)
/* i.MX6SLL */
#define MTR_MASTER_BASE_ADDR        (AIPS2_OFF_BASE_ADDR + 0x7C000)

#ifdef CONFIG_MX6SX
#define GIS_BASE_ADDR               (AIPS3_ARB_BASE_ADDR + 0x04000)
#define DCIC1_BASE_ADDR             (AIPS3_ARB_BASE_ADDR + 0x0C000)
#define DCIC2_BASE_ADDR             (AIPS3_ARB_BASE_ADDR + 0x10000)
#define CSI1_BASE_ADDR              (AIPS3_ARB_BASE_ADDR + 0x14000)
#define PXP_BASE_ADDR               (AIPS3_ARB_BASE_ADDR + 0x18000)
#define CSI2_BASE_ADDR              (AIPS3_ARB_BASE_ADDR + 0x1C000)
#define VADC_BASE_ADDR              (AIPS3_ARB_BASE_ADDR + 0x28000)
#define VDEC_BASE_ADDR              (AIPS3_ARB_BASE_ADDR + 0x2C000)
#define SPBA_BASE_ADDR              (AIPS3_ARB_BASE_ADDR + 0x3C000)
#define AIPS3_CONFIG_BASE_ADDR      (AIPS3_ARB_BASE_ADDR + 0x7C000)
#define ADC1_BASE_ADDR              (AIPS3_ARB_BASE_ADDR + 0x80000)
#define ADC2_BASE_ADDR              (AIPS3_ARB_BASE_ADDR + 0x84000)
#define ECSPI5_BASE_ADDR            (AIPS3_ARB_BASE_ADDR + 0x8C000)
#define HS_BASE_ADDR                (AIPS3_ARB_BASE_ADDR + 0x90000)
#define MU_MCU_BASE_ADDR            (AIPS3_ARB_BASE_ADDR + 0x94000)
#define CANFD_BASE_ADDR             (AIPS3_ARB_BASE_ADDR + 0x98000)
#define MU_DSP_BASE_ADDR            (AIPS3_ARB_BASE_ADDR + 0x9C000)
#define UART6_BASE_ADDR             (AIPS3_ARB_BASE_ADDR + 0xA0000)
#define PWM5_BASE_ADDR              (AIPS3_ARB_BASE_ADDR + 0xA4000)
#define PWM6_BASE_ADDR              (AIPS3_ARB_BASE_ADDR + 0xA8000)
#define PWM7_BASE_ADDR              (AIPS3_ARB_BASE_ADDR + 0xAC000)
#define PWM8_BASE_ADDR              (AIPS3_ARB_BASE_ADDR + 0xB0000)
#elif (defined(CONFIG_MX6UL) || defined(CONFIG_MX6ULL))
#define AIPS3_CONFIG_BASE_ADDR      (AIPS3_ARB_BASE_ADDR + 0x7C000)
#define DCP_BASE_ADDR               (AIPS3_ARB_BASE_ADDR + 0x80000)
#define RNGB_BASE_ADDR              (AIPS3_ARB_BASE_ADDR + 0x84000)
#define UART8_IPS_BASE_ADDR         (AIPS3_ARB_BASE_ADDR + 0x88000)
#define EPDC_BASE_ADDR              (AIPS3_ARB_BASE_ADDR + 0x8C000)
#define IOMUXC_SNVS_BASE_ADDR       (AIPS3_ARB_BASE_ADDR + 0x90000)
#define SNVS_GPR_BASE_ADDR          (AIPS3_ARB_BASE_ADDR + 0x94000)
#endif

#define NOC_DDR_BASE_ADDR           (GPV0_BASE_ADDR + 0xB0000)

/* Only for i.MX6SX */
#define LCDIF2_BASE_ADDR            (AIPS3_ARB_BASE_ADDR + 0x24000)
#define MX6SX_LCDIF1_BASE_ADDR      (AIPS3_ARB_BASE_ADDR + 0x20000)
#define MX6SX_WDOG3_BASE_ADDR       (AIPS3_ARB_BASE_ADDR + 0x88000)

#if !(defined(CONFIG_MX6SX) || \
	defined(CONFIG_MX6UL) || defined(CONFIG_MX6ULL) || \
	defined(CONFIG_MX6SLL) || defined(CONFIG_MX6SL))
#define IRAM_SIZE                    0x00040000
#else
#define IRAM_SIZE                    0x00020000
#endif
#define FEC_QUIRK_ENET_MAC

#include <asm/mach-imx/regs-lcdif.h>
#if !(defined(__KERNEL_STRICT_NAMES) || defined(__ASSEMBLY__))
#include <asm/types.h>

/* only for i.MX6SX/UL */
#define WDOG3_BASE_ADDR (((is_mx6ul() || is_mx6ull()) ?	\
			 MX6UL_WDOG3_BASE_ADDR :  MX6SX_WDOG3_BASE_ADDR))
#define LCDIF1_BASE_ADDR ((is_cpu_type(MXC_CPU_MX6SLL)) ?	\
			  MX6SLL_LCDIF_BASE_ADDR :		\
			  (is_cpu_type(MXC_CPU_MX6SL)) ?	\
			  MX6SL_LCDIF_BASE_ADDR :		\
			  ((is_cpu_type(MXC_CPU_MX6UL)) ?	\
			  MX6UL_LCDIF1_BASE_ADDR :		\
			  ((is_mx6ull()) ?	\
			  MX6ULL_LCDIF1_BASE_ADDR : MX6SX_LCDIF1_BASE_ADDR)))


extern void imx_get_mac_from_fuse(int dev_id, unsigned char *mac);

#define SRC_SCR_CORE_1_RESET_OFFSET     14
#define SRC_SCR_CORE_1_RESET_MASK       (1<<SRC_SCR_CORE_1_RESET_OFFSET)
#define SRC_SCR_CORE_2_RESET_OFFSET     15
#define SRC_SCR_CORE_2_RESET_MASK       (1<<SRC_SCR_CORE_2_RESET_OFFSET)
#define SRC_SCR_CORE_3_RESET_OFFSET     16
#define SRC_SCR_CORE_3_RESET_MASK       (1<<SRC_SCR_CORE_3_RESET_OFFSET)
#define SRC_SCR_CORE_1_ENABLE_OFFSET    22
#define SRC_SCR_CORE_1_ENABLE_MASK      (1<<SRC_SCR_CORE_1_ENABLE_OFFSET)
#define SRC_SCR_CORE_2_ENABLE_OFFSET    23
#define SRC_SCR_CORE_2_ENABLE_MASK      (1<<SRC_SCR_CORE_2_ENABLE_OFFSET)
#define SRC_SCR_CORE_3_ENABLE_OFFSET    24
#define SRC_SCR_CORE_3_ENABLE_MASK      (1<<SRC_SCR_CORE_3_ENABLE_OFFSET)

struct rdc_regs {
	u32	vir;		/* Version information */
	u32	reserved1[8];
	u32	stat;		/* Status */
	u32	intctrl;	/* Interrupt and Control */
	u32	intstat;	/* Interrupt Status */
	u32	reserved2[116];
	u32	mda[32];	/* Master Domain Assignment */
	u32	reserved3[96];
	u32	pdap[104];	/* Peripheral Domain Access Permissions */
	u32	reserved4[88];
	struct {
		u32 mrsa;	/* Memory Region Start Address */
		u32 mrea;	/* Memory Region End Address */
		u32 mrc;	/* Memory Region Control */
		u32 mrvs;	/* Memory Region Violation Status */
	} mem_region[55];
};

struct rdc_sema_regs {
	u8	gate[64];	/* Gate */
	u16	rstgt;		/* Reset Gate */
};

/* WEIM registers */
struct weim {
	u32 cs0gcr1;
	u32 cs0gcr2;
	u32 cs0rcr1;
	u32 cs0rcr2;
	u32 cs0wcr1;
	u32 cs0wcr2;

	u32 cs1gcr1;
	u32 cs1gcr2;
	u32 cs1rcr1;
	u32 cs1rcr2;
	u32 cs1wcr1;
	u32 cs1wcr2;

	u32 cs2gcr1;
	u32 cs2gcr2;
	u32 cs2rcr1;
	u32 cs2rcr2;
	u32 cs2wcr1;
	u32 cs2wcr2;

	u32 cs3gcr1;
	u32 cs3gcr2;
	u32 cs3rcr1;
	u32 cs3rcr2;
	u32 cs3wcr1;
	u32 cs3wcr2;

	u32 unused[12];

	u32 wcr;
	u32 wiar;
	u32 ear;
};

/* System Reset Controller (SRC) */
struct src {
	u32	scr;
	u32	sbmr1;
	u32	srsr;
	u32	reserved1[2];
	u32	sisr;
	u32	simr;
	u32     sbmr2;
	u32     gpr1;
	u32     gpr2;
	u32     gpr3;
	u32     gpr4;
	u32     gpr5;
	u32     gpr6;
	u32     gpr7;
	u32     gpr8;
	u32     gpr9;
	u32     gpr10;
};

#define src_base ((struct src *)SRC_BASE_ADDR)

#define SRC_M4_REG_OFFSET		0
#define SRC_M4_ENABLE_OFFSET		22
#define SRC_M4_ENABLE_MASK		BIT(22)
#define SRC_M4C_NON_SCLR_RST_OFFSET	4
#define SRC_M4C_NON_SCLR_RST_MASK	BIT(4)

/* GPR1 bitfields */
#define IOMUXC_GPR1_APP_CLK_REQ_N		BIT(30)
#define IOMUXC_GPR1_PCIE_EXIT_L1		BIT(28)
#define IOMUXC_GPR1_PCIE_RDY_L23		BIT(27)
#define IOMUXC_GPR1_PCIE_ENTER_L1		BIT(26)
#define IOMUXC_GPR1_MIPI_COLOR_SW		BIT(25)
#define IOMUXC_GPR1_DPI_OFF			BIT(24)
#define IOMUXC_GPR1_EXC_MON_SLVE		BIT(22)
#define IOMUXC_GPR1_ENET_CLK_SEL_OFFSET		21
#define IOMUXC_GPR1_ENET_CLK_SEL_MASK		(1 << IOMUXC_GPR1_ENET_CLK_SEL_OFFSET)
#define IOMUXC_GPR1_MIPI_IPU2_MUX_IOMUX		BIT(20)
#define IOMUXC_GPR1_MIPI_IPU1_MUX_IOMUX		BIT(19)
#define IOMUXC_GPR1_PCIE_TEST_PD			BIT(18)
#define IOMUXC_GPR1_IPU_VPU_MUX_IPU2		BIT(17)
#define IOMUXC_GPR1_PCIE_REF_CLK_EN		BIT(16)
#define IOMUXC_GPR1_USB_EXP_MODE			BIT(15)
#define IOMUXC_GPR1_PCIE_INT			BIT(14)
#define IOMUXC_GPR1_USB_OTG_ID_OFFSET		13
#define IOMUXC_GPR1_USB_OTG_ID_SEL_MASK		(1 << IOMUXC_GPR1_USB_OTG_ID_OFFSET)
#define IOMUXC_GPR1_GINT				BIT(12)
#define IOMUXC_GPR1_ADDRS3_MASK			(0x3 << 10)
#define IOMUXC_GPR1_ADDRS3_32MB			(0x0 << 10)
#define IOMUXC_GPR1_ADDRS3_64MB			(0x1 << 10)
#define IOMUXC_GPR1_ADDRS3_128MB			(0x2 << 10)
#define IOMUXC_GPR1_ACT_CS3			BIT(9)
#define IOMUXC_GPR1_ADDRS2_MASK			(0x3 << 7)
#define IOMUXC_GPR1_ACT_CS2			BIT(6)
#define IOMUXC_GPR1_ADDRS1_MASK			(0x3 << 4)
#define IOMUXC_GPR1_ACT_CS1			BIT(3)
#define IOMUXC_GPR1_ADDRS0_OFFSET		(1)
#define IOMUXC_GPR1_ADDRS0_MASK			(0x3 << 1)
#define IOMUXC_GPR1_ACT_CS0			BIT(0)

/* GPR3 bitfields */
#define IOMUXC_GPR3_GPU_DBG_OFFSET		29
#define IOMUXC_GPR3_GPU_DBG_MASK		(3<<IOMUXC_GPR3_GPU_DBG_OFFSET)
#define IOMUXC_GPR3_BCH_WR_CACHE_CTL_OFFSET	28
#define IOMUXC_GPR3_BCH_WR_CACHE_CTL_MASK	(1<<IOMUXC_GPR3_BCH_WR_CACHE_CTL_OFFSET)
#define IOMUXC_GPR3_BCH_RD_CACHE_CTL_OFFSET	27
#define IOMUXC_GPR3_BCH_RD_CACHE_CTL_MASK	(1<<IOMUXC_GPR3_BCH_RD_CACHE_CTL_OFFSET)
#define IOMUXC_GPR3_uSDHCx_WR_CACHE_CTL_OFFSET	26
#define IOMUXC_GPR3_uSDHCx_WR_CACHE_CTL_MASK	(1<<IOMUXC_GPR3_uSDHCx_WR_CACHE_CTL_OFFSET)
#define IOMUXC_GPR3_uSDHCx_RD_CACHE_CTL_OFFSET	25
#define IOMUXC_GPR3_uSDHCx_RD_CACHE_CTL_MASK	(1<<IOMUXC_GPR3_uSDHCx_RD_CACHE_CTL_OFFSET)
#define IOMUXC_GPR3_OCRAM_CTL_OFFSET		21
#define IOMUXC_GPR3_OCRAM_CTL_MASK		(0xf<<IOMUXC_GPR3_OCRAM_CTL_OFFSET)
#define IOMUXC_GPR3_OCRAM_STATUS_OFFSET		17
#define IOMUXC_GPR3_OCRAM_STATUS_MASK		(0xf<<IOMUXC_GPR3_OCRAM_STATUS_OFFSET)
#define IOMUXC_GPR3_CORE3_DBG_ACK_EN_OFFSET	16
#define IOMUXC_GPR3_CORE3_DBG_ACK_EN_MASK	(1<<IOMUXC_GPR3_CORE3_DBG_ACK_EN_OFFSET)
#define IOMUXC_GPR3_CORE2_DBG_ACK_EN_OFFSET	15
#define IOMUXC_GPR3_CORE2_DBG_ACK_EN_MASK	(1<<IOMUXC_GPR3_CORE2_DBG_ACK_EN_OFFSET)
#define IOMUXC_GPR3_CORE1_DBG_ACK_EN_OFFSET	14
#define IOMUXC_GPR3_CORE1_DBG_ACK_EN_MASK	(1<<IOMUXC_GPR3_CORE1_DBG_ACK_EN_OFFSET)
#define IOMUXC_GPR3_CORE0_DBG_ACK_EN_OFFSET	13
#define IOMUXC_GPR3_CORE0_DBG_ACK_EN_MASK	(1<<IOMUXC_GPR3_CORE0_DBG_ACK_EN_OFFSET)
#define IOMUXC_GPR3_TZASC2_BOOT_LOCK_OFFSET	12
#define IOMUXC_GPR3_TZASC2_BOOT_LOCK_MASK	(1<<IOMUXC_GPR3_TZASC2_BOOT_LOCK_OFFSET)
#define IOMUXC_GPR3_TZASC1_BOOT_LOCK_OFFSET	11
#define IOMUXC_GPR3_TZASC1_BOOT_LOCK_MASK	(1<<IOMUXC_GPR3_TZASC1_BOOT_LOCK_OFFSET)
#define IOMUXC_GPR3_IPU_DIAG_OFFSET		10
#define IOMUXC_GPR3_IPU_DIAG_MASK		(1<<IOMUXC_GPR3_IPU_DIAG_OFFSET)

#define IOMUXC_GPR3_MUX_SRC_IPU1_DI0	0
#define IOMUXC_GPR3_MUX_SRC_IPU1_DI1	1
#define IOMUXC_GPR3_MUX_SRC_IPU2_DI0	2
#define IOMUXC_GPR3_MUX_SRC_IPU2_DI1	3

#define IOMUXC_GPR3_LVDS1_MUX_CTL_OFFSET	8
#define IOMUXC_GPR3_LVDS1_MUX_CTL_MASK		(3<<IOMUXC_GPR3_LVDS1_MUX_CTL_OFFSET)

#define IOMUXC_GPR3_LVDS0_MUX_CTL_OFFSET	6
#define IOMUXC_GPR3_LVDS0_MUX_CTL_MASK		(3<<IOMUXC_GPR3_LVDS0_MUX_CTL_OFFSET)

#define IOMUXC_GPR3_MIPI_MUX_CTL_OFFSET		4
#define IOMUXC_GPR3_MIPI_MUX_CTL_MASK		(3<<IOMUXC_GPR3_MIPI_MUX_CTL_OFFSET)

#define IOMUXC_GPR3_HDMI_MUX_CTL_OFFSET		2
#define IOMUXC_GPR3_HDMI_MUX_CTL_MASK		(3<<IOMUXC_GPR3_HDMI_MUX_CTL_OFFSET)

/* gpr12 bitfields */
#define IOMUXC_GPR12_ARMP_IPG_CLK_EN		BIT(27)
#define IOMUXC_GPR12_ARMP_AHB_CLK_EN		BIT(26)
#define IOMUXC_GPR12_ARMP_ATB_CLK_EN		BIT(25)
#define IOMUXC_GPR12_ARMP_APB_CLK_EN		BIT(24)
#define IOMUXC_GPR12_DEVICE_TYPE		(0xf << 12)
#define IOMUXC_GPR12_PCIE_CTL_2			BIT(10)
#define IOMUXC_GPR12_LOS_LEVEL			(0x1f << 4)

struct iomuxc {
#if (defined(CONFIG_MX6SX) || defined(CONFIG_MX6UL) || defined(CONFIG_MX6ULL))
	u8 reserved[0x4000];
#endif
	u32 gpr[14];
};

struct gpc {
	u32	cntr;
	u32	pgr;
	u32	imr1;
	u32	imr2;
	u32	imr3;
	u32	imr4;
	u32	isr1;
	u32	isr2;
	u32	isr3;
	u32	isr4;
};

#define IOMUXC_GPR2_COUNTER_RESET_VAL_OFFSET		20
#define IOMUXC_GPR2_COUNTER_RESET_VAL_MASK		(3<<IOMUXC_GPR2_COUNTER_RESET_VAL_OFFSET)
#define IOMUXC_GPR2_LVDS_CLK_SHIFT_OFFSET		16
#define IOMUXC_GPR2_LVDS_CLK_SHIFT_MASK			(7<<IOMUXC_GPR2_LVDS_CLK_SHIFT_OFFSET)

#define IOMUXC_GPR2_BGREF_RRMODE_OFFSET			15
#define IOMUXC_GPR2_BGREF_RRMODE_MASK			(1<<IOMUXC_GPR2_BGREF_RRMODE_OFFSET)
#define IOMUXC_GPR2_BGREF_RRMODE_INTERNAL_RES		(1<<IOMUXC_GPR2_BGREF_RRMODE_OFFSET)
#define IOMUXC_GPR2_BGREF_RRMODE_EXTERNAL_RES		(0<<IOMUXC_GPR2_BGREF_RRMODE_OFFSET)
#define IOMUXC_GPR2_VSYNC_ACTIVE_HIGH	0
#define IOMUXC_GPR2_VSYNC_ACTIVE_LOW	1

#define IOMUXC_GPR2_DI1_VS_POLARITY_OFFSET		10
#define IOMUXC_GPR2_DI1_VS_POLARITY_MASK		(1<<IOMUXC_GPR2_DI1_VS_POLARITY_OFFSET)
#define IOMUXC_GPR2_DI1_VS_POLARITY_ACTIVE_HIGH		(IOMUXC_GPR2_VSYNC_ACTIVE_HIGH<<IOMUXC_GPR2_DI1_VS_POLARITY_OFFSET)
#define IOMUXC_GPR2_DI1_VS_POLARITY_ACTIVE_LOW		(IOMUXC_GPR2_VSYNC_ACTIVE_LOW<<IOMUXC_GPR2_DI1_VS_POLARITY_OFFSET)

#define IOMUXC_GPR2_DI0_VS_POLARITY_OFFSET		9
#define IOMUXC_GPR2_DI0_VS_POLARITY_MASK		(1<<IOMUXC_GPR2_DI0_VS_POLARITY_OFFSET)
#define IOMUXC_GPR2_DI0_VS_POLARITY_ACTIVE_HIGH		(IOMUXC_GPR2_VSYNC_ACTIVE_HIGH<<IOMUXC_GPR2_DI0_VS_POLARITY_OFFSET)
#define IOMUXC_GPR2_DI0_VS_POLARITY_ACTIVE_LOW		(IOMUXC_GPR2_VSYNC_ACTIVE_LOW<<IOMUXC_GPR2_DI0_VS_POLARITY_OFFSET)

#define IOMUXC_GPR2_BITMAP_SPWG	0
#define IOMUXC_GPR2_BITMAP_JEIDA	1

#define IOMUXC_GPR2_BIT_MAPPING_CH1_OFFSET		8
#define IOMUXC_GPR2_BIT_MAPPING_CH1_MASK		(1<<IOMUXC_GPR2_BIT_MAPPING_CH1_OFFSET)
#define IOMUXC_GPR2_BIT_MAPPING_CH1_JEIDA		(IOMUXC_GPR2_BITMAP_JEIDA<<IOMUXC_GPR2_BIT_MAPPING_CH1_OFFSET)
#define IOMUXC_GPR2_BIT_MAPPING_CH1_SPWG		(IOMUXC_GPR2_BITMAP_SPWG<<IOMUXC_GPR2_BIT_MAPPING_CH1_OFFSET)

#define IOMUXC_GPR2_DATA_WIDTH_18	0
#define IOMUXC_GPR2_DATA_WIDTH_24	1

#define IOMUXC_GPR2_DATA_WIDTH_CH1_OFFSET		7
#define IOMUXC_GPR2_DATA_WIDTH_CH1_MASK			(1<<IOMUXC_GPR2_DATA_WIDTH_CH1_OFFSET)
#define IOMUXC_GPR2_DATA_WIDTH_CH1_18BIT		(IOMUXC_GPR2_DATA_WIDTH_18<<IOMUXC_GPR2_DATA_WIDTH_CH1_OFFSET)
#define IOMUXC_GPR2_DATA_WIDTH_CH1_24BIT		(IOMUXC_GPR2_DATA_WIDTH_24<<IOMUXC_GPR2_DATA_WIDTH_CH1_OFFSET)

#define IOMUXC_GPR2_BIT_MAPPING_CH0_OFFSET		6
#define IOMUXC_GPR2_BIT_MAPPING_CH0_MASK		(1<<IOMUXC_GPR2_BIT_MAPPING_CH0_OFFSET)
#define IOMUXC_GPR2_BIT_MAPPING_CH0_JEIDA		(IOMUXC_GPR2_BITMAP_JEIDA<<IOMUXC_GPR2_BIT_MAPPING_CH0_OFFSET)
#define IOMUXC_GPR2_BIT_MAPPING_CH0_SPWG		(IOMUXC_GPR2_BITMAP_SPWG<<IOMUXC_GPR2_BIT_MAPPING_CH0_OFFSET)

#define IOMUXC_GPR2_DATA_WIDTH_CH0_OFFSET		5
#define IOMUXC_GPR2_DATA_WIDTH_CH0_MASK			(1<<IOMUXC_GPR2_DATA_WIDTH_CH0_OFFSET)
#define IOMUXC_GPR2_DATA_WIDTH_CH0_18BIT		(IOMUXC_GPR2_DATA_WIDTH_18<<IOMUXC_GPR2_DATA_WIDTH_CH0_OFFSET)
#define IOMUXC_GPR2_DATA_WIDTH_CH0_24BIT		(IOMUXC_GPR2_DATA_WIDTH_24<<IOMUXC_GPR2_DATA_WIDTH_CH0_OFFSET)

#define IOMUXC_GPR2_SPLIT_MODE_EN_OFFSET		4
#define IOMUXC_GPR2_SPLIT_MODE_EN_MASK			(1<<IOMUXC_GPR2_SPLIT_MODE_EN_OFFSET)

#define IOMUXC_GPR2_MODE_DISABLED	0
#define IOMUXC_GPR2_MODE_ENABLED_DI0	1
#define IOMUXC_GPR2_MODE_ENABLED_DI1	3

#define IOMUXC_GPR2_LVDS_CH1_MODE_OFFSET		2
#define IOMUXC_GPR2_LVDS_CH1_MODE_MASK			(3<<IOMUXC_GPR2_LVDS_CH1_MODE_OFFSET)
#define IOMUXC_GPR2_LVDS_CH1_MODE_DISABLED		(IOMUXC_GPR2_MODE_DISABLED<<IOMUXC_GPR2_LVDS_CH1_MODE_OFFSET)
#define IOMUXC_GPR2_LVDS_CH1_MODE_ENABLED_DI0		(IOMUXC_GPR2_MODE_ENABLED_DI0<<IOMUXC_GPR2_LVDS_CH1_MODE_OFFSET)
#define IOMUXC_GPR2_LVDS_CH1_MODE_ENABLED_DI1		(IOMUXC_GPR2_MODE_ENABLED_DI1<<IOMUXC_GPR2_LVDS_CH1_MODE_OFFSET)

#define IOMUXC_GPR2_LVDS_CH0_MODE_OFFSET		0
#define IOMUXC_GPR2_LVDS_CH0_MODE_MASK			(3<<IOMUXC_GPR2_LVDS_CH0_MODE_OFFSET)
#define IOMUXC_GPR2_LVDS_CH0_MODE_DISABLED		(IOMUXC_GPR2_MODE_DISABLED<<IOMUXC_GPR2_LVDS_CH0_MODE_OFFSET)
#define IOMUXC_GPR2_LVDS_CH0_MODE_ENABLED_DI0		(IOMUXC_GPR2_MODE_ENABLED_DI0<<IOMUXC_GPR2_LVDS_CH0_MODE_OFFSET)
#define IOMUXC_GPR2_LVDS_CH0_MODE_ENABLED_DI1		(IOMUXC_GPR2_MODE_ENABLED_DI1<<IOMUXC_GPR2_LVDS_CH0_MODE_OFFSET)

/* ECSPI registers */
struct cspi_regs {
	u32 rxdata;
	u32 txdata;
	u32 ctrl;
	u32 cfg;
	u32 intr;
	u32 dma;
	u32 stat;
	u32 period;
};

/*
 * CSPI register definitions
 */
#define MXC_ECSPI
#define MXC_CSPICTRL_EN		(1 << 0)
#define MXC_CSPICTRL_MODE	(1 << 1)
#define MXC_CSPICTRL_XCH	(1 << 2)
#define MXC_CSPICTRL_MODE_MASK (0xf << 4)
#define MXC_CSPICTRL_CHIPSELECT(x)	(((x) & 0x3) << 12)
#define MXC_CSPICTRL_BITCOUNT(x)	(((x) & 0xfff) << 20)
#define MXC_CSPICTRL_PREDIV(x)	(((x) & 0xF) << 12)
#define MXC_CSPICTRL_POSTDIV(x)	(((x) & 0xF) << 8)
#define MXC_CSPICTRL_SELCHAN(x)	(((x) & 0x3) << 18)
#define MXC_CSPICTRL_MAXBITS	0xfff
#define MXC_CSPICTRL_TC		(1 << 7)
#define MXC_CSPICTRL_RXOVF	(1 << 6)
#define MXC_CSPIPERIOD_32KHZ	(1 << 15)
#define MAX_SPI_BYTES	32
#define SPI_MAX_NUM	4

/* Bit position inside CTRL register to be associated with SS */
#define MXC_CSPICTRL_CHAN	18

/* Bit position inside CON register to be associated with SS */
#define MXC_CSPICON_PHA		0  /* SCLK phase control */
#define MXC_CSPICON_POL		4  /* SCLK polarity */
#define MXC_CSPICON_SSPOL	12 /* SS polarity */
#define MXC_CSPICON_CTL		20 /* inactive state of SCLK */
#if defined(CONFIG_MX6SLL) || defined(CONFIG_MX6SL) || \
	defined(CONFIG_MX6DL) || defined(CONFIG_MX6UL) || defined(CONFIG_MX6ULL)
#define MXC_SPI_BASE_ADDRESSES \
	ECSPI1_BASE_ADDR, \
	ECSPI2_BASE_ADDR, \
	ECSPI3_BASE_ADDR, \
	ECSPI4_BASE_ADDR
#else
#define MXC_SPI_BASE_ADDRESSES \
	ECSPI1_BASE_ADDR, \
	ECSPI2_BASE_ADDR, \
	ECSPI3_BASE_ADDR, \
	ECSPI4_BASE_ADDR, \
	ECSPI5_BASE_ADDR
#endif

struct ocotp_regs {
	u32	ctrl;
	u32	ctrl_set;
	u32     ctrl_clr;
	u32	ctrl_tog;
	u32	timing;
	u32     rsvd0[3];
	u32     data;
	u32     rsvd1[3];
	u32     read_ctrl;
	u32     rsvd2[3];
	u32	read_fuse_data;
	u32     rsvd3[3];
	u32	sw_sticky;
	u32     rsvd4[3];
	u32     scs;
	u32     scs_set;
	u32     scs_clr;
	u32     scs_tog;
	u32     crc_addr;
	u32     rsvd5[3];
	u32     crc_value;
	u32     rsvd6[3];
	u32     version;
	u32     rsvd7[0xdb];

	/* fuse banks */
	struct fuse_bank {
		u32	fuse_regs[0x20];
	} bank[0];
};

struct fuse_bank0_regs {
	u32	lock;
	u32	rsvd0[3];
	u32	uid_low;
	u32	rsvd1[3];
	u32	uid_high;
	u32	rsvd2[3];
	u32	cfg2;
	u32	rsvd3[3];
	u32	cfg3;
	u32	rsvd4[3];
	u32	cfg4;
	u32	rsvd5[3];
	u32	cfg5;
	u32	rsvd6[3];
	u32	cfg6;
	u32	rsvd7[3];
};

struct fuse_bank1_regs {
	u32	mem0;
	u32	rsvd0[3];
	u32	mem1;
	u32	rsvd1[3];
	u32	mem2;
	u32	rsvd2[3];
	u32	mem3;
	u32	rsvd3[3];
	u32	mem4;
	u32	rsvd4[3];
	u32	ana0;
	u32	rsvd5[3];
	u32	ana1;
	u32	rsvd6[3];
	u32	ana2;
	u32	rsvd7[3];
};

struct fuse_bank4_regs {
	u32 sjc_resp_low;
	u32 rsvd0[3];
	u32 sjc_resp_high;
	u32 rsvd1[3];
	u32 mac_addr0;
	u32 rsvd2[3];
	u32 mac_addr1;
	u32 rsvd3[3];
	u32 mac_addr2; /*For i.MX6SX and i.MX6UL*/
	u32 rsvd4[7];
	u32 gp1;
	u32 rsvd5[3];
	u32 gp2;
	u32 rsvd6[3];
};

struct aipstz_regs {
	u32	mprot0;
	u32	mprot1;
	u32	rsvd[0xe];
	u32	opacr0;
	u32	opacr1;
	u32	opacr2;
	u32	opacr3;
	u32	opacr4;
};

struct anatop_regs {
	u32	pll_sys;		/* 0x000 */
	u32	pll_sys_set;		/* 0x004 */
	u32	pll_sys_clr;		/* 0x008 */
	u32	pll_sys_tog;		/* 0x00c */
	u32	usb1_pll_480_ctrl;	/* 0x010 */
	u32	usb1_pll_480_ctrl_set;	/* 0x014 */
	u32	usb1_pll_480_ctrl_clr;	/* 0x018 */
	u32	usb1_pll_480_ctrl_tog;	/* 0x01c */
	u32	usb2_pll_480_ctrl;	/* 0x020 */
	u32	usb2_pll_480_ctrl_set;	/* 0x024 */
	u32	usb2_pll_480_ctrl_clr;	/* 0x028 */
	u32	usb2_pll_480_ctrl_tog;	/* 0x02c */
	u32	pll_528;		/* 0x030 */
	u32	pll_528_set;		/* 0x034 */
	u32	pll_528_clr;		/* 0x038 */
	u32	pll_528_tog;		/* 0x03c */
	u32	pll_528_ss;		/* 0x040 */
	u32	rsvd0[3];
	u32	pll_528_num;		/* 0x050 */
	u32	rsvd1[3];
	u32	pll_528_denom;		/* 0x060 */
	u32	rsvd2[3];
	u32	pll_audio;		/* 0x070 */
	u32	pll_audio_set;		/* 0x074 */
	u32	pll_audio_clr;		/* 0x078 */
	u32	pll_audio_tog;		/* 0x07c */
	u32	pll_audio_num;		/* 0x080 */
	u32	rsvd3[3];
	u32	pll_audio_denom;	/* 0x090 */
	u32	rsvd4[3];
	u32	pll_video;		/* 0x0a0 */
	u32	pll_video_set;		/* 0x0a4 */
	u32	pll_video_clr;		/* 0x0a8 */
	u32	pll_video_tog;		/* 0x0ac */
	u32	pll_video_num;		/* 0x0b0 */
	u32	rsvd5[3];
	u32	pll_video_denom;	/* 0x0c0 */
	u32	rsvd6[3];
	u32	pll_mlb;		/* 0x0d0 */
	u32	pll_mlb_set;		/* 0x0d4 */
	u32	pll_mlb_clr;		/* 0x0d8 */
	u32	pll_mlb_tog;		/* 0x0dc */
	u32	pll_enet;		/* 0x0e0 */
	u32	pll_enet_set;		/* 0x0e4 */
	u32	pll_enet_clr;		/* 0x0e8 */
	u32	pll_enet_tog;		/* 0x0ec */
	u32	pfd_480;		/* 0x0f0 */
	u32	pfd_480_set;		/* 0x0f4 */
	u32	pfd_480_clr;		/* 0x0f8 */
	u32	pfd_480_tog;		/* 0x0fc */
	u32	pfd_528;		/* 0x100 */
	u32	pfd_528_set;		/* 0x104 */
	u32	pfd_528_clr;		/* 0x108 */
	u32	pfd_528_tog;		/* 0x10c */
	u32	reg_1p1;		/* 0x110 */
	u32	reg_1p1_set;		/* 0x114 */
	u32	reg_1p1_clr;		/* 0x118 */
	u32	reg_1p1_tog;		/* 0x11c */
	u32	reg_3p0;		/* 0x120 */
	u32	reg_3p0_set;		/* 0x124 */
	u32	reg_3p0_clr;		/* 0x128 */
	u32	reg_3p0_tog;		/* 0x12c */
	u32	reg_2p5;		/* 0x130 */
	u32	reg_2p5_set;		/* 0x134 */
	u32	reg_2p5_clr;		/* 0x138 */
	u32	reg_2p5_tog;		/* 0x13c */
	u32	reg_core;		/* 0x140 */
	u32	reg_core_set;		/* 0x144 */
	u32	reg_core_clr;		/* 0x148 */
	u32	reg_core_tog;		/* 0x14c */
	u32	ana_misc0;		/* 0x150 */
	u32	ana_misc0_set;		/* 0x154 */
	u32	ana_misc0_clr;		/* 0x158 */
	u32	ana_misc0_tog;		/* 0x15c */
	u32	ana_misc1;		/* 0x160 */
	u32	ana_misc1_set;		/* 0x164 */
	u32	ana_misc1_clr;		/* 0x168 */
	u32	ana_misc1_tog;		/* 0x16c */
	u32	ana_misc2;		/* 0x170 */
	u32	ana_misc2_set;		/* 0x174 */
	u32	ana_misc2_clr;		/* 0x178 */
	u32	ana_misc2_tog;		/* 0x17c */
	u32	tempsense0;		/* 0x180 */
	u32	tempsense0_set;		/* 0x184 */
	u32	tempsense0_clr;		/* 0x188 */
	u32	tempsense0_tog;		/* 0x18c */
	u32	tempsense1;		/* 0x190 */
	u32	tempsense1_set;		/* 0x194 */
	u32	tempsense1_clr;		/* 0x198 */
	u32	tempsense1_tog;		/* 0x19c */
	u32	usb1_vbus_detect;	/* 0x1a0 */
	u32	usb1_vbus_detect_set;	/* 0x1a4 */
	u32	usb1_vbus_detect_clr;	/* 0x1a8 */
	u32	usb1_vbus_detect_tog;	/* 0x1ac */
	u32	usb1_chrg_detect;	/* 0x1b0 */
	u32	usb1_chrg_detect_set;	/* 0x1b4 */
	u32	usb1_chrg_detect_clr;	/* 0x1b8 */
	u32	usb1_chrg_detect_tog;	/* 0x1bc */
	u32	usb1_vbus_det_stat;	/* 0x1c0 */
	u32	usb1_vbus_det_stat_set;	/* 0x1c4 */
	u32	usb1_vbus_det_stat_clr;	/* 0x1c8 */
	u32	usb1_vbus_det_stat_tog;	/* 0x1cc */
	u32	usb1_chrg_det_stat;	/* 0x1d0 */
	u32	usb1_chrg_det_stat_set;	/* 0x1d4 */
	u32	usb1_chrg_det_stat_clr;	/* 0x1d8 */
	u32	usb1_chrg_det_stat_tog;	/* 0x1dc */
	u32	usb1_loopback;		/* 0x1e0 */
	u32	usb1_loopback_set;	/* 0x1e4 */
	u32	usb1_loopback_clr;	/* 0x1e8 */
	u32	usb1_loopback_tog;	/* 0x1ec */
	u32	usb1_misc;		/* 0x1f0 */
	u32	usb1_misc_set;		/* 0x1f4 */
	u32	usb1_misc_clr;		/* 0x1f8 */
	u32	usb1_misc_tog;		/* 0x1fc */
	u32	usb2_vbus_detect;	/* 0x200 */
	u32	usb2_vbus_detect_set;	/* 0x204 */
	u32	usb2_vbus_detect_clr;	/* 0x208 */
	u32	usb2_vbus_detect_tog;	/* 0x20c */
	u32	usb2_chrg_detect;	/* 0x210 */
	u32	usb2_chrg_detect_set;	/* 0x214 */
	u32	usb2_chrg_detect_clr;	/* 0x218 */
	u32	usb2_chrg_detect_tog;	/* 0x21c */
	u32	usb2_vbus_det_stat;	/* 0x220 */
	u32	usb2_vbus_det_stat_set;	/* 0x224 */
	u32	usb2_vbus_det_stat_clr;	/* 0x228 */
	u32	usb2_vbus_det_stat_tog;	/* 0x22c */
	u32	usb2_chrg_det_stat;	/* 0x230 */
	u32	usb2_chrg_det_stat_set;	/* 0x234 */
	u32	usb2_chrg_det_stat_clr;	/* 0x238 */
	u32	usb2_chrg_det_stat_tog;	/* 0x23c */
	u32	usb2_loopback;		/* 0x240 */
	u32	usb2_loopback_set;	/* 0x244 */
	u32	usb2_loopback_clr;	/* 0x248 */
	u32	usb2_loopback_tog;	/* 0x24c */
	u32	usb2_misc;		/* 0x250 */
	u32	usb2_misc_set;		/* 0x254 */
	u32	usb2_misc_clr;		/* 0x258 */
	u32	usb2_misc_tog;		/* 0x25c */
	u32	digprog;		/* 0x260 */
	u32	reserved1[7];
	u32	digprog_sololite;	/* 0x280 */
};

#define ANATOP_PFD_FRAC_SHIFT(n)	((n)*8)
#define ANATOP_PFD_FRAC_MASK(n)	(0x3f<<ANATOP_PFD_FRAC_SHIFT(n))
#define ANATOP_PFD_STABLE_SHIFT(n)	(6+((n)*8))
#define ANATOP_PFD_STABLE_MASK(n)	(1<<ANATOP_PFD_STABLE_SHIFT(n))
#define ANATOP_PFD_CLKGATE_SHIFT(n)	(7+((n)*8))
#define ANATOP_PFD_CLKGATE_MASK(n)	(1<<ANATOP_PFD_CLKGATE_SHIFT(n))

struct wdog_regs {
	u16	wcr;	/* Control */
	u16	wsr;	/* Service */
	u16	wrsr;	/* Reset Status */
	u16	wicr;	/* Interrupt Control */
	u16	wmcr;	/* Miscellaneous Control */
};

#define PWMCR_PRESCALER(x)	(((x - 1) & 0xFFF) << 4)
#define PWMCR_DOZEEN		(1 << 24)
#define PWMCR_WAITEN		(1 << 23)
#define PWMCR_DBGEN		(1 << 22)
#define PWMCR_CLKSRC_IPG_HIGH	(2 << 16)
#define PWMCR_CLKSRC_IPG	(1 << 16)
#define PWMCR_EN		(1 << 0)

struct pwm_regs {
	u32	cr;
	u32	sr;
	u32	ir;
	u32	sar;
	u32	pr;
	u32	cnr;
};

/*
 * If ROM fail back to USB recover mode, USBPH0_PWD will be clear to use USB
 * If boot from the other mode, USB0_PWD will keep reset value
 */
#define	is_boot_from_usb(void) (!(readl(USB_PHY0_BASE_ADDR) & (1<<20)))

#endif /* __ASSEMBLER__*/
#endif /* __ASM_ARCH_MX6_IMX_REGS_H__ */
