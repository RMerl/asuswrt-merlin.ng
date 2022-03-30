/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * K2G: SoC definitions
 *
 * (C) Copyright 2015
 *     Texas Instruments Incorporated, <www.ti.com>
 */

#ifndef __ASM_ARCH_HARDWARE_K2G_H
#define __ASM_ARCH_HARDWARE_K2G_H

#define KS2_NUM_DSPS			1

/* Power and Sleep Controller (PSC) Domains */
#define KS2_LPSC_ALWAYSON		0
#define KS2_LPSC_PMMC			1
#define KS2_LPSC_DEBUG			2
#define KS2_LPSC_NSS			3
#define KS2_LPSC_SA			4
#define KS2_LPSC_TERANET		5
#define KS2_LPSC_SYS_COMP		6
#define KS2_LPSC_QSPI			7
#define KS2_LPSC_MMC			8
#define KS2_LPSC_GPMC			9
#define KS2_LPSC_MLB			11
#define KS2_LPSC_EHRPWM			12
#define KS2_LPSC_EQEP			13
#define KS2_LPSC_ECAP			14
#define KS2_LPSC_MCASP			15
#define KS2_LPSC_SR			16
#define KS2_LPSC_MSMC			17
#ifdef KS2_LPSC_GEM_0
#undef KS2_LPSC_GEM_0
#endif
#define KS2_LPSC_GEM_0			18
#define KS2_LPSC_ARM			19
#define KS2_LPSC_ASRC			20
#define KS2_LPSC_ICSS			21
#define KS2_LPSC_DSS			23
#define KS2_LPSC_PCIE			24
#define KS2_LPSC_USB_0			25
#define KS2_LPSC_USB			KS2_LPSC_USB_0
#define KS2_LPSC_USB_1			26
#define KS2_LPSC_DDR3			27
#define KS2_LPSC_SPARE0_LPSC0		28
#define KS2_LPSC_SPARE0_LPSC1		29
#define KS2_LPSC_SPARE1_LPSC0		30
#define KS2_LPSC_SPARE1_LPSC1		31

#define KS2_LPSC_CPGMAC			KS2_LPSC_NSS
#define KS2_LPSC_CRYPTO			KS2_LPSC_SA

/* SGMII SerDes */
#define KS2_LANES_PER_SGMII_SERDES	4

/* NETCP pktdma */
#define KS2_NETCP_PDMA_CTRL_BASE	0x04010000
#define KS2_NETCP_PDMA_TX_BASE		0x04011000
#define KS2_NETCP_PDMA_TX_CH_NUM	21
#define KS2_NETCP_PDMA_RX_BASE		0x04012000
#define KS2_NETCP_PDMA_RX_CH_NUM	32
#define KS2_NETCP_PDMA_SCHED_BASE	0x04010100
#define KS2_NETCP_PDMA_RX_FLOW_BASE	0x04013000
#define KS2_NETCP_PDMA_RX_FLOW_NUM	32
#define KS2_NETCP_PDMA_TX_SND_QUEUE	5

/* NETCP */
#define KS2_NETCP_BASE			0x04000000

#define K2G_GPIO0_BASE			0X02603000
#define K2G_GPIO1_BASE			0X0260a000
#define K2G_GPIO0_BANK0_BASE		K2G_GPIO0_BASE + 0x10
#define K2G_GPIO1_BANK2_BASE		K2G_GPIO1_BASE + 0x38
#define K2G_GPIO_DIR_OFFSET		0x0
#define K2G_GPIO_OUTDATA_OFFSET		0x4
#define K2G_GPIO_SETDATA_OFFSET		0x8
#define K2G_GPIO_CLRDATA_OFFSET		0xC

/* BOOTCFG RESETMUX8 */
#define KS2_RSTMUX8			(KS2_DEVICE_STATE_CTRL_BASE + 0x328)

/* RESETMUX register definitions */
#define RSTMUX_LOCK8_SHIFT		0x0
#define RSTMUX_LOCK8_MASK		(0x1 << 0)
#define RSTMUX_OMODE8_SHIFT		0x1
#define RSTMUX_OMODE8_MASK		(0x7 << 1)
#define RSTMUX_OMODE8_DEV_RESET		0x2
#define RSTMUX_OMODE8_INT		0x3
#define RSTMUX_OMODE8_INT_AND_DEV_RESET	0x4

/* DEVSTAT register definition */
#define KS2_DEVSTAT_REFCLK_SHIFT	 7
#define KS2_DEVSTAT_REFCLK_MASK		(0x7 << 7)

/* GPMC */
#define KS2_GPMC_BASE			0x21818000

/* SYSCLK indexes */
#define SYSCLK_19MHz	0
#define SYSCLK_24MHz	1
#define SYSCLK_25MHz	2
#define SYSCLK_26MHz	3
#define MAX_SYSCLK	4

#ifndef __ASSEMBLY__
static inline u8 get_sysclk_index(void)
{
	u32 dev_stat = __raw_readl(KS2_DEVSTAT);
	return (dev_stat & KS2_DEVSTAT_REFCLK_MASK) >> KS2_DEVSTAT_REFCLK_SHIFT;
}
#endif
#endif /* __ASM_ARCH_HARDWARE_K2G_H */
