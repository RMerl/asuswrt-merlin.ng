/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (c) 2015 Purna Chandra Mandal <purna.mandal@microchip.com>
 *
 */

#ifndef __MICROCHIP_PIC32_ETH_H_
#define __MICROCHIP_PIC32_ETH_H_

#include <mach/pic32.h>

/* Ethernet */
struct pic32_ectl_regs {
	struct pic32_reg_atomic con1; /* 0x00 */
	struct pic32_reg_atomic con2; /* 0x10 */
	struct pic32_reg_atomic txst; /* 0x20 */
	struct pic32_reg_atomic rxst; /* 0x30 */
	struct pic32_reg_atomic ht0;  /* 0x40 */
	struct pic32_reg_atomic ht1;  /* 0x50 */
	struct pic32_reg_atomic pmm0; /* 0x60 */
	struct pic32_reg_atomic pmm1; /* 0x70 */
	struct pic32_reg_atomic pmcs; /* 0x80 */
	struct pic32_reg_atomic pmo;  /* 0x90 */
	struct pic32_reg_atomic rxfc; /* 0xa0 */
	struct pic32_reg_atomic rxwm; /* 0xb0 */
	struct pic32_reg_atomic ien;  /* 0xc0 */
	struct pic32_reg_atomic irq;  /* 0xd0 */
	struct pic32_reg_atomic stat; /* 0xe0 */
};

struct pic32_mii_regs {
	struct pic32_reg_atomic mcfg; /* 0x280 */
	struct pic32_reg_atomic mcmd; /* 0x290 */
	struct pic32_reg_atomic madr; /* 0x2a0 */
	struct pic32_reg_atomic mwtd; /* 0x2b0 */
	struct pic32_reg_atomic mrdd; /* 0x2c0 */
	struct pic32_reg_atomic mind; /* 0x2d0 */
};

struct pic32_emac_regs {
	struct pic32_reg_atomic cfg1; /* 0x200*/
	struct pic32_reg_atomic cfg2; /* 0x210*/
	struct pic32_reg_atomic ipgt; /* 0x220*/
	struct pic32_reg_atomic ipgr; /* 0x230*/
	struct pic32_reg_atomic clrt; /* 0x240*/
	struct pic32_reg_atomic maxf; /* 0x250*/
	struct pic32_reg_atomic supp; /* 0x260*/
	struct pic32_reg_atomic test; /* 0x270*/
	struct pic32_mii_regs mii;    /* 0x280 - 0x2d0 */
	struct pic32_reg_atomic res1; /* 0x2e0 */
	struct pic32_reg_atomic res2; /* 0x2f0 */
	struct pic32_reg_atomic sa0;  /* 0x300 */
	struct pic32_reg_atomic sa1;  /* 0x310 */
	struct pic32_reg_atomic sa2;  /* 0x320 */
};

/* ETHCON1 Reg field */
#define ETHCON_BUFCDEC		BIT(0)
#define ETHCON_RXEN		BIT(8)
#define ETHCON_TXRTS		BIT(9)
#define ETHCON_ON		BIT(15)

/* ETHCON2 Reg field */
#define ETHCON_RXBUFSZ		0x7f
#define ETHCON_RXBUFSZ_SHFT	0x4

/* ETHSTAT Reg field */
#define ETHSTAT_BUSY		BIT(7)
#define ETHSTAT_BUFCNT		0x00ff0000

/* ETHRXFC Register fields */
#define ETHRXFC_BCEN		BIT(0)
#define ETHRXFC_MCEN		BIT(1)
#define ETHRXFC_UCEN		BIT(3)
#define ETHRXFC_RUNTEN		BIT(4)
#define ETHRXFC_CRCOKEN		BIT(5)

/* EMAC1CFG1 register offset */
#define PIC32_EMAC1CFG1		0x0200

/* EMAC1CFG1 register fields */
#define EMAC_RXENABLE		BIT(0)
#define EMAC_RXPAUSE		BIT(2)
#define EMAC_TXPAUSE		BIT(3)
#define EMAC_SOFTRESET		BIT(15)

/* EMAC1CFG2 register fields */
#define EMAC_FULLDUP		BIT(0)
#define EMAC_LENGTHCK		BIT(1)
#define EMAC_CRCENABLE		BIT(4)
#define EMAC_PADENABLE		BIT(5)
#define EMAC_AUTOPAD		BIT(7)
#define EMAC_EXCESS		BIT(14)

/* EMAC1IPGT register magic */
#define FULLDUP_GAP_TIME	0x15
#define HALFDUP_GAP_TIME	0x12

/* EMAC1SUPP register fields */
#define EMAC_RMII_SPD100	BIT(8)
#define EMAC_RMII_RESET		BIT(11)

/* MII Management Configuration Register */
#define MIIMCFG_RSTMGMT		BIT(15)
#define MIIMCFG_CLKSEL_DIV40	0x0020	/* 100Mhz / 40 */

/* MII Management Command Register */
#define MIIMCMD_READ		BIT(0)
#define MIIMCMD_SCAN		BIT(1)

/* MII Management Address Register */
#define MIIMADD_REGADDR		0x1f
#define MIIMADD_REGADDR_SHIFT	0
#define MIIMADD_PHYADDR_SHIFT	8

/* MII Management Indicator Register */
#define MIIMIND_BUSY		BIT(0)
#define MIIMIND_NOTVALID	BIT(2)
#define MIIMIND_LINKFAIL	BIT(3)

/* Packet Descriptor */
/* Received Packet Status */
#define _RSV1_PKT_CSUM		0xffff
#define _RSV2_CRC_ERR		BIT(20)
#define _RSV2_LEN_ERR		BIT(21)
#define _RSV2_RX_OK		BIT(23)
#define _RSV2_RX_COUNT		0xffff

#define RSV_RX_CSUM(__rsv1)	((__rsv1) & _RSV1_PKT_CSUM)
#define RSV_RX_COUNT(__rsv2)	((__rsv2) & _RSV2_RX_COUNT)
#define RSV_RX_OK(__rsv2)	((__rsv2) & _RSV2_RX_OK)
#define RSV_CRC_ERR(__rsv2)	((__rsv2) & _RSV2_CRC_ERR)

/* Ethernet Hardware Descriptor Header bits */
#define EDH_EOWN		BIT(7)
#define EDH_NPV			BIT(8)
#define EDH_STICKY		BIT(9)
#define _EDH_BCOUNT		0x07ff0000
#define EDH_EOP			BIT(30)
#define EDH_SOP			BIT(31)
#define EDH_BCOUNT_SHIFT	16
#define EDH_BCOUNT(len)		((len) << EDH_BCOUNT_SHIFT)

/* Ethernet Hardware Descriptors
 * ref: PIC32 Family Reference Manual Table 35-7
 * This structure represents the layout of the DMA
 * memory shared between the CPU and the Ethernet
 * controller.
 */
/* TX/RX DMA descriptor */
struct eth_dma_desc {
	u32 hdr;	/* header */
	u32 data_buff;	/* data buffer address */
	u32 stat1;	/* transmit/receive packet status */
	u32 stat2;	/* transmit/receive packet status */
	u32 next_ed;	/* next descriptor */
};

#define PIC32_MDIO_NAME "PIC32_EMAC"

int pic32_mdio_init(const char *name, ulong ioaddr);

#endif /* __MICROCHIP_PIC32_ETH_H_*/
