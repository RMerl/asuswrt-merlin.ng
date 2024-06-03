/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Keystone2: Common SoC definitions, structures etc.
 *
 * (C) Copyright 2012-2014
 *     Texas Instruments Incorporated, <www.ti.com>
 */
#ifndef __ASM_ARCH_HARDWARE_H
#define __ASM_ARCH_HARDWARE_H

#include <config.h>

#ifndef __ASSEMBLY__

#include <linux/sizes.h>
#include <asm/io.h>

#define	REG(addr)        (*(volatile unsigned int *)(addr))
#define REG_P(addr)      ((volatile unsigned int *)(addr))

typedef volatile unsigned int   dv_reg;
typedef volatile unsigned int   *dv_reg_p;

#endif

#define KS2_DDRPHY_PIR_OFFSET           0x04
#define KS2_DDRPHY_PGCR0_OFFSET         0x08
#define KS2_DDRPHY_PGCR1_OFFSET         0x0C
#define KS2_DDRPHY_PGSR0_OFFSET         0x10
#define KS2_DDRPHY_PGSR1_OFFSET         0x14
#define KS2_DDRPHY_PLLCR_OFFSET         0x18
#define KS2_DDRPHY_PTR0_OFFSET          0x1C
#define KS2_DDRPHY_PTR1_OFFSET          0x20
#define KS2_DDRPHY_PTR2_OFFSET          0x24
#define KS2_DDRPHY_PTR3_OFFSET          0x28
#define KS2_DDRPHY_PTR4_OFFSET          0x2C
#define KS2_DDRPHY_DCR_OFFSET           0x44

#define KS2_DDRPHY_DTPR0_OFFSET         0x48
#define KS2_DDRPHY_DTPR1_OFFSET         0x4C
#define KS2_DDRPHY_DTPR2_OFFSET         0x50

#define KS2_DDRPHY_MR0_OFFSET           0x54
#define KS2_DDRPHY_MR1_OFFSET           0x58
#define KS2_DDRPHY_MR2_OFFSET           0x5C
#define KS2_DDRPHY_DTCR_OFFSET          0x68
#define KS2_DDRPHY_PGCR2_OFFSET         0x8C

#define KS2_DDRPHY_ZQ0CR1_OFFSET        0x184
#define KS2_DDRPHY_ZQ1CR1_OFFSET        0x194
#define KS2_DDRPHY_ZQ2CR1_OFFSET        0x1A4
#define KS2_DDRPHY_ZQ3CR1_OFFSET        0x1B4

#define KS2_DDRPHY_DATX8_2_OFFSET       0x240
#define KS2_DDRPHY_DATX8_3_OFFSET       0x280
#define KS2_DDRPHY_DATX8_4_OFFSET       0x2C0
#define KS2_DDRPHY_DATX8_5_OFFSET       0x300
#define KS2_DDRPHY_DATX8_6_OFFSET       0x340
#define KS2_DDRPHY_DATX8_7_OFFSET       0x380
#define KS2_DDRPHY_DATX8_8_OFFSET       0x3C0

#define IODDRM_MASK                     0x00000180
#define ZCKSEL_MASK                     0x01800000
#define CL_MASK                         0x00000072
#define WR_MASK                         0x00000E00
#define BL_MASK                         0x00000003
#define RRMODE_MASK                     0x00040000
#define UDIMM_MASK                      0x20000000
#define BYTEMASK_MASK                   0x0003FC00
#define MPRDQ_MASK                      0x00000080
#define PDQ_MASK                        0x00000070
#define NOSRA_MASK                      0x08000000
#define ECC_MASK                        0x00000001
#define DXEN_MASK                       0x00000001

/* DDR3 definitions */
#define KS2_DDR3A_EMIF_CTRL_BASE	0x21010000
#define KS2_DDR3A_EMIF_DATA_BASE	0x80000000
#define KS2_DDR3A_DDRPHYC		0x02329000
#define EMIF1_BASE			KS2_DDR3A_EMIF_CTRL_BASE

#define KS2_DDR3_MIDR_OFFSET            0x00
#define KS2_DDR3_STATUS_OFFSET          0x04
#define KS2_DDR3_SDCFG_OFFSET           0x08
#define KS2_DDR3_SDRFC_OFFSET           0x10
#define KS2_DDR3_SDTIM1_OFFSET          0x18
#define KS2_DDR3_SDTIM2_OFFSET          0x1C
#define KS2_DDR3_SDTIM3_OFFSET          0x20
#define KS2_DDR3_SDTIM4_OFFSET          0x28
#define KS2_DDR3_PMCTL_OFFSET           0x38
#define KS2_DDR3_ZQCFG_OFFSET           0xC8

#define KS2_DDR3_PLLCTRL_PHY_RESET	0x80000000

/* DDR3 ECC */
#define KS2_DDR3_ECC_INT_STATUS_OFFSET			0x0AC
#define KS2_DDR3_ECC_INT_ENABLE_SET_SYS_OFFSET		0x0B4
#define KS2_DDR3_ECC_CTRL_OFFSET			0x110
#define KS2_DDR3_ECC_ADDR_RANGE1_OFFSET			0x114
#define KS2_DDR3_ONE_BIT_ECC_ERR_CNT_OFFSET		0x130
#define KS2_DDR3_ONE_BIT_ECC_ERR_ADDR_LOG_OFFSET	0x13C

/* DDR3 ECC Interrupt Status register */
#define KS2_DDR3_1B_ECC_ERR_SYS		BIT(5)
#define KS2_DDR3_2B_ECC_ERR_SYS		BIT(4)
#define KS2_DDR3_WR_ECC_ERR_SYS		BIT(3)

/* DDR3 ECC Control register */
#define KS2_DDR3_ECC_EN			BIT(31)
#define KS2_DDR3_ECC_ADDR_RNG_PROT	BIT(30)
#define KS2_DDR3_ECC_VERIFY_EN		BIT(29)
#define KS2_DDR3_ECC_RMW_EN		BIT(28)
#define KS2_DDR3_ECC_ADDR_RNG_1_EN	BIT(0)

#define KS2_DDR3_ECC_ENABLE		(KS2_DDR3_ECC_EN | \
					KS2_DDR3_ECC_ADDR_RNG_PROT | \
					KS2_DDR3_ECC_VERIFY_EN)

/* EDMA */
#define KS2_EDMA0_BASE			0x02700000

/* EDMA3 register offsets */
#define KS2_EDMA_QCHMAP0		0x0200
#define KS2_EDMA_IPR			0x1068
#define KS2_EDMA_ICR			0x1070
#define KS2_EDMA_QEECR			0x1088
#define KS2_EDMA_QEESR			0x108c
#define KS2_EDMA_PARAM_1(x)		(0x4020 + (4 * x))

/* NETCP pktdma */
#ifdef CONFIG_SOC_K2G
#define KS2_NETCP_PDMA_RX_FREE_QUEUE	113
#define KS2_NETCP_PDMA_RX_RCV_QUEUE	114
#else
#define KS2_NETCP_PDMA_RX_FREE_QUEUE	4001
#define KS2_NETCP_PDMA_RX_RCV_QUEUE	4002
#endif

/* Chip Interrupt Controller */
#define KS2_CIC2_BASE			0x02608000

/* Chip Interrupt Controller register offsets */
#define KS2_CIC_CTRL			0x04
#define KS2_CIC_HOST_CTRL		0x0C
#define KS2_CIC_GLOBAL_ENABLE		0x10
#define KS2_CIC_SYS_ENABLE_IDX_SET	0x28
#define KS2_CIC_HOST_ENABLE_IDX_SET	0x34
#define KS2_CIC_CHAN_MAP(n)		(0x0400 + (n << 2))

#define KS2_UART0_BASE                	0x02530c00
#define KS2_UART1_BASE                	0x02531000

/* Boot Config */
#define KS2_DEVICE_STATE_CTRL_BASE	0x02620000
#define KS2_JTAG_ID_REG			(KS2_DEVICE_STATE_CTRL_BASE + 0x18)
#define KS2_DEVSTAT			(KS2_DEVICE_STATE_CTRL_BASE + 0x20)
#define KS2_DEVCFG			(KS2_DEVICE_STATE_CTRL_BASE + 0x14c)
#define KS2_ETHERNET_CFG		(KS2_DEVICE_STATE_CTRL_BASE + 0xe20)
#define KS2_ETHERNET_RGMII		2

/* PSC */
#define KS2_PSC_BASE			0x02350000
#define KS2_LPSC_GEM_0			15
#define KS2_LPSC_TETRIS			52
#define KS2_TETRIS_PWR_DOMAIN		31
#define KS2_GEM_0_PWR_DOMAIN		8

/* Chip configuration unlock codes and registers */
#define KS2_KICK0			(KS2_DEVICE_STATE_CTRL_BASE + 0x38)
#define KS2_KICK1			(KS2_DEVICE_STATE_CTRL_BASE + 0x3c)
#define KS2_KICK0_MAGIC			0x83e70b13
#define KS2_KICK1_MAGIC			0x95a4f1e0

/* PLL control registers */
#define KS2_MAINPLLCTL0			(KS2_DEVICE_STATE_CTRL_BASE + 0x350)
#define KS2_MAINPLLCTL1			(KS2_DEVICE_STATE_CTRL_BASE + 0x354)
#define KS2_PASSPLLCTL0			(KS2_DEVICE_STATE_CTRL_BASE + 0x358)
#define KS2_PASSPLLCTL1			(KS2_DEVICE_STATE_CTRL_BASE + 0x35C)
#define KS2_DDR3APLLCTL0		(KS2_DEVICE_STATE_CTRL_BASE + 0x360)
#define KS2_DDR3APLLCTL1		(KS2_DEVICE_STATE_CTRL_BASE + 0x364)
#define KS2_DDR3BPLLCTL0		(KS2_DEVICE_STATE_CTRL_BASE + 0x368)
#define KS2_DDR3BPLLCTL1		(KS2_DEVICE_STATE_CTRL_BASE + 0x36C)
#define KS2_ARMPLLCTL0			(KS2_DEVICE_STATE_CTRL_BASE + 0x370)
#define KS2_ARMPLLCTL1			(KS2_DEVICE_STATE_CTRL_BASE + 0x374)
#define KS2_UARTPLLCTL0			(KS2_DEVICE_STATE_CTRL_BASE + 0x390)
#define KS2_UARTPLLCTL1			(KS2_DEVICE_STATE_CTRL_BASE + 0x394)

#define KS2_PLL_CNTRL_BASE		0x02310000
#define KS2_CLOCK_BASE			KS2_PLL_CNTRL_BASE
#define KS2_RSTCTRL_RSTYPE		(KS2_PLL_CNTRL_BASE + 0xe4)
#define KS2_RSTCTRL			(KS2_PLL_CNTRL_BASE + 0xe8)
#define KS2_RSTCTRL_RSCFG		(KS2_PLL_CNTRL_BASE + 0xec)
#define KS2_RSTCTRL_KEY			0x5a69
#define KS2_RSTCTRL_MASK		0xffff0000
#define KS2_RSTCTRL_SWRST		0xfffe0000
#define KS2_RSTYPE_PLL_SOFT		BIT(13)

/* SPI */
#ifdef CONFIG_SOC_K2G
#define KS2_SPI0_BASE			0x21805400
#define KS2_SPI1_BASE			0x21805800
#define KS2_SPI2_BASE			0x21805c00
#define KS2_SPI3_BASE			0x21806000
#else
#define KS2_SPI0_BASE			0x21000400
#define KS2_SPI1_BASE			0x21000600
#define KS2_SPI2_BASE			0x21000800
#define KS2_SPI_BASE			KS2_SPI0_BASE
#endif

/* AEMIF */
#define KS2_AEMIF_CNTRL_BASE       	0x21000a00
#define DAVINCI_ASYNC_EMIF_CNTRL_BASE   KS2_AEMIF_CNTRL_BASE

/* Flag from ks2_debug options to check if DSPs need to stay ON */
#define DBG_LEAVE_DSPS_ON		0x1

/* MSMC control */
#define KS2_MSMC_CTRL_BASE		0x0bc00000
#define KS2_MSMC_DATA_BASE		0x0c000000

/* KS2 Generic Privilege ID Settings for MSMC2 */
#define KS2_MSMC_SEGMENT_C6X_0		0
#define KS2_MSMC_SEGMENT_C6X_1		1
#define KS2_MSMC_SEGMENT_C6X_2		2
#define KS2_MSMC_SEGMENT_C6X_3		3
#define KS2_MSMC_SEGMENT_C6X_4		4
#define KS2_MSMC_SEGMENT_C6X_5		5
#define KS2_MSMC_SEGMENT_C6X_6		6
#define KS2_MSMC_SEGMENT_C6X_7		7

#define KS2_MSMC_SEGMENT_DEBUG		12

/* KS2 HK/L/E MSMC PRIVIDs  for MSMC2 */
#define K2HKLE_MSMC_SEGMENT_ARM		8
#define K2HKLE_MSMC_SEGMENT_NETCP	9
#define K2HKLE_MSMC_SEGMENT_QM_PDSP	10
#define K2HKLE_MSMC_SEGMENT_PCIE0	11

/* K2HK specific Privilege ID Settings */
#define K2HKE_MSMC_SEGMENT_HYPERLINK	14

/* K2L specific Privilege ID Settings */
#define K2L_MSMC_SEGMENT_PCIE1		14

/* K2E specific Privilege ID Settings */
#define K2E_MSMC_SEGMENT_PCIE1		13
#define K2E_MSMC_SEGMENT_TSIP		15

/* K2G specific Privilege ID Settings */
#define K2G_MSMC_SEGMENT_ARM		1
#define K2G_MSMC_SEGMENT_ICSS0		2
#define K2G_MSMC_SEGMENT_ICSS1		3
#define K2G_MSMC_SEGMENT_NSS		4
#define K2G_MSMC_SEGMENT_PCIE		5
#define K2G_MSMC_SEGMENT_USB		6
#define K2G_MSMC_SEGMENT_MLB		8
#define K2G_MSMC_SEGMENT_PMMC		9
#define K2G_MSMC_SEGMENT_DSS		10
#define K2G_MSMC_SEGMENT_MMC		11

/* MSMC segment size shift bits */
#define KS2_MSMC_SEG_SIZE_SHIFT		12
#define KS2_MSMC_MAP_SEG_NUM		(2 << (30 - KS2_MSMC_SEG_SIZE_SHIFT))
#define KS2_MSMC_DST_SEG_BASE		(CONFIG_SYS_LPAE_SDRAM_BASE >> \
					KS2_MSMC_SEG_SIZE_SHIFT)

/* Device speed */
#define KS2_REV1_DEVSPEED		(KS2_DEVICE_STATE_CTRL_BASE + 0xc98)
#define KS2_EFUSE_BOOTROM		(KS2_DEVICE_STATE_CTRL_BASE + 0xc90)
#define KS2_MISC_CTRL			(KS2_DEVICE_STATE_CTRL_BASE + 0xc7c)

/* Queue manager */
#ifdef CONFIG_SOC_K2G
#define KS2_QM_BASE_ADDRESS		0x040C0000
#define KS2_QM_CONF_BASE		0x04040000
#define KS2_QM_DESC_SETUP_BASE		0x04080000
#define KS2_QM_STATUS_RAM_BASE		0x0 /* K2G doesn't have it */
#define KS2_QM_INTD_CONF_BASE		0x0
#define KS2_QM_PDSP1_CMD_BASE		0x0
#define KS2_QM_PDSP1_CTRL_BASE		0x0
#define KS2_QM_PDSP1_IRAM_BASE		0x0
#define KS2_QM_MANAGER_QUEUES_BASE	0x040c0000
#define KS2_QM_MANAGER_Q_PROXY_BASE	0x04040200
#define KS2_QM_QUEUE_STATUS_BASE	0x04100000
#define KS2_QM_LINK_RAM_BASE		0x04020000
#define KS2_QM_REGION_NUM		8
#define KS2_QM_QPOOL_NUM		112
#else
#define KS2_QM_BASE_ADDRESS		0x23a80000
#define KS2_QM_CONF_BASE		0x02a02000
#define KS2_QM_DESC_SETUP_BASE		0x02a03000
#define KS2_QM_STATUS_RAM_BASE		0x02a06000
#define KS2_QM_INTD_CONF_BASE		0x02a0c000
#define KS2_QM_PDSP1_CMD_BASE		0x02a20000
#define KS2_QM_PDSP1_CTRL_BASE		0x02a0f000
#define KS2_QM_PDSP1_IRAM_BASE		0x02a10000
#define KS2_QM_MANAGER_QUEUES_BASE	0x02a80000
#define KS2_QM_MANAGER_Q_PROXY_BASE	0x02ac0000
#define KS2_QM_QUEUE_STATUS_BASE	0x02a40000
#define KS2_QM_LINK_RAM_BASE		0x00100000
#define KS2_QM_REGION_NUM		64
#define KS2_QM_QPOOL_NUM		4000
#endif

/* USB */
#define KS2_USB_SS_BASE			0x02680000
#define KS2_USB_HOST_XHCI_BASE		(KS2_USB_SS_BASE + 0x10000)
#define KS2_DEV_USB_PHY_BASE		0x02620738
#define KS2_USB_PHY_CFG_BASE		0x02630000

#define KS2_MAC_ID_BASE_ADDR		(KS2_DEVICE_STATE_CTRL_BASE + 0x110)

/* SGMII SerDes */
#define KS2_SGMII_SERDES_BASE		0x0232a000

/* JTAG ID register */
#define JTAGID_VARIANT_SHIFT	28
#define JTAGID_VARIANT_MASK	(0xf << 28)
#define JTAGID_PART_NUM_SHIFT	12
#define JTAGID_PART_NUM_MASK	(0xffff << 12)

/* PART NUMBER definitions */
#define CPU_66AK2Hx	0xb981
#define CPU_66AK2Ex	0xb9a6
#define CPU_66AK2Lx	0xb9a7
#define CPU_66AK2Gx	0xbb06

/* Variant definitions */
#define CPU_66AK2G1x	0x08

/* DEVSPEED register */
#define DEVSPEED_DEVSPEED_SHIFT	16
#define DEVSPEED_DEVSPEED_MASK	(0xfff << 16)
#define DEVSPEED_ARMSPEED_SHIFT	0
#define DEVSPEED_ARMSPEED_MASK	0xfff
#define DEVSPEED_NUMSPDS	12

#ifdef CONFIG_SOC_K2HK
#include <asm/arch/hardware-k2hk.h>
#endif

#ifdef CONFIG_SOC_K2E
#include <asm/arch/hardware-k2e.h>
#endif

#ifdef CONFIG_SOC_K2L
#include <asm/arch/hardware-k2l.h>
#endif

#ifdef CONFIG_SOC_K2G
#include <asm/arch/hardware-k2g.h>
#endif

#ifndef __ASSEMBLY__

static inline u16 get_part_number(void)
{
	u32 jtag_id = __raw_readl(KS2_JTAG_ID_REG);

	return (jtag_id & JTAGID_PART_NUM_MASK) >> JTAGID_PART_NUM_SHIFT;
}

static inline u8 cpu_is_k2hk(void)
{
	return get_part_number() == CPU_66AK2Hx;
}

static inline u8 cpu_is_k2e(void)
{
	return get_part_number() == CPU_66AK2Ex;
}

static inline u8 cpu_is_k2l(void)
{
	return get_part_number() == CPU_66AK2Lx;
}

static inline u8 cpu_is_k2g(void)
{
	return get_part_number() == CPU_66AK2Gx;
}

static inline u8 cpu_revision(void)
{
	u32 jtag_id	= __raw_readl(KS2_JTAG_ID_REG);
	u8 rev	= (jtag_id & JTAGID_VARIANT_MASK) >> JTAGID_VARIANT_SHIFT;

	return rev;
}

int cpu_to_bus(u32 *ptr, u32 length);
void sdelay(unsigned long);

#endif

#endif /* __ASM_ARCH_HARDWARE_H */
