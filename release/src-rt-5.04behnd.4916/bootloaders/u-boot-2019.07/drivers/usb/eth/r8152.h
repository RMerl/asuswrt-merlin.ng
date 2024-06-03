/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2015 Realtek Semiconductor Corp. All rights reserved.
 *
  */

#ifndef _RTL8152_ETH_H
#define _RTL8152_ETH_H

#define R8152_BASE_NAME		"r8152"

#define PLA_IDR			0xc000
#define PLA_RCR			0xc010
#define PLA_RMS			0xc016
#define PLA_RXFIFO_CTRL0	0xc0a0
#define PLA_RXFIFO_CTRL1	0xc0a4
#define PLA_RXFIFO_CTRL2	0xc0a8
#define PLA_DMY_REG0		0xc0b0
#define PLA_FMC			0xc0b4
#define PLA_CFG_WOL		0xc0b6
#define PLA_TEREDO_CFG		0xc0bc
#define PLA_MAR			0xcd00
#define PLA_BACKUP		0xd000
#define PAL_BDC_CR		0xd1a0
#define PLA_TEREDO_TIMER	0xd2cc
#define PLA_REALWOW_TIMER	0xd2e8
#define PLA_LEDSEL		0xdd90
#define PLA_LED_FEATURE		0xdd92
#define PLA_PHYAR		0xde00
#define PLA_BOOT_CTRL		0xe004
#define PLA_GPHY_INTR_IMR	0xe022
#define PLA_EEE_CR		0xe040
#define PLA_EEEP_CR		0xe080
#define PLA_MAC_PWR_CTRL	0xe0c0
#define PLA_MAC_PWR_CTRL2	0xe0ca
#define PLA_MAC_PWR_CTRL3	0xe0cc
#define PLA_MAC_PWR_CTRL4	0xe0ce
#define PLA_WDT6_CTRL		0xe428
#define PLA_TCR0		0xe610
#define PLA_TCR1		0xe612
#define PLA_MTPS		0xe615
#define PLA_TXFIFO_CTRL		0xe618
#define PLA_RSTTALLY		0xe800
#define BIST_CTRL		0xe810
#define PLA_CR			0xe813
#define PLA_CRWECR		0xe81c
#define PLA_CONFIG12		0xe81e	/* CONFIG1, CONFIG2 */
#define PLA_CONFIG34		0xe820	/* CONFIG3, CONFIG4 */
#define PLA_CONFIG5		0xe822
#define PLA_PHY_PWR		0xe84c
#define PLA_OOB_CTRL		0xe84f
#define PLA_CPCR		0xe854
#define PLA_MISC_0		0xe858
#define PLA_MISC_1		0xe85a
#define PLA_OCP_GPHY_BASE	0xe86c
#define PLA_TALLYCNT		0xe890
#define PLA_SFF_STS_7		0xe8de
#define PLA_PHYSTATUS		0xe908
#define PLA_BP_BA		0xfc26
#define PLA_BP_0		0xfc28
#define PLA_BP_1		0xfc2a
#define PLA_BP_2		0xfc2c
#define PLA_BP_3		0xfc2e
#define PLA_BP_4		0xfc30
#define PLA_BP_5		0xfc32
#define PLA_BP_6		0xfc34
#define PLA_BP_7		0xfc36
#define PLA_BP_EN		0xfc38

#define USB_USB2PHY		0xb41e
#define USB_SSPHYLINK2		0xb428
#define USB_U2P3_CTRL		0xb460
#define USB_CSR_DUMMY1		0xb464
#define USB_CSR_DUMMY2		0xb466
#define USB_DEV_STAT		0xb808
#define USB_CONNECT_TIMER	0xcbf8
#define USB_BURST_SIZE		0xcfc0
#define USB_USB_CTRL		0xd406
#define USB_PHY_CTRL		0xd408
#define USB_TX_AGG		0xd40a
#define USB_RX_BUF_TH		0xd40c
#define USB_USB_TIMER		0xd428
#define USB_RX_EARLY_TIMEOUT	0xd42c
#define USB_RX_EARLY_SIZE	0xd42e
#define USB_PM_CTRL_STATUS	0xd432
#define USB_TX_DMA		0xd434
#define USB_TOLERANCE		0xd490
#define USB_LPM_CTRL		0xd41a
#define USB_UPS_CTRL		0xd800
#define USB_MISC_0		0xd81a
#define USB_POWER_CUT		0xd80a
#define USB_AFE_CTRL2		0xd824
#define USB_WDT11_CTRL		0xe43c
#define USB_BP_BA		0xfc26
#define USB_BP_0		0xfc28
#define USB_BP_1		0xfc2a
#define USB_BP_2		0xfc2c
#define USB_BP_3		0xfc2e
#define USB_BP_4		0xfc30
#define USB_BP_5		0xfc32
#define USB_BP_6		0xfc34
#define USB_BP_7		0xfc36
#define USB_BP_EN		0xfc38

/* OCP Registers */
#define OCP_ALDPS_CONFIG	0x2010
#define OCP_EEE_CONFIG1		0x2080
#define OCP_EEE_CONFIG2		0x2092
#define OCP_EEE_CONFIG3		0x2094
#define OCP_BASE_MII		0xa400
#define OCP_EEE_AR		0xa41a
#define OCP_EEE_DATA		0xa41c
#define OCP_PHY_STATUS		0xa420
#define OCP_POWER_CFG		0xa430
#define OCP_EEE_CFG		0xa432
#define OCP_SRAM_ADDR		0xa436
#define OCP_SRAM_DATA		0xa438
#define OCP_DOWN_SPEED		0xa442
#define OCP_EEE_ABLE		0xa5c4
#define OCP_EEE_ADV		0xa5d0
#define OCP_EEE_LPABLE		0xa5d2
#define OCP_PHY_STATE		0xa708		/* nway state for 8153 */
#define OCP_ADC_CFG		0xbc06

/* SRAM Register */
#define SRAM_LPF_CFG		0x8012
#define SRAM_10M_AMP1		0x8080
#define SRAM_10M_AMP2		0x8082
#define SRAM_IMPEDANCE		0x8084

/* PLA_RCR */
#define RCR_AAP			0x00000001
#define RCR_APM			0x00000002
#define RCR_AM			0x00000004
#define RCR_AB			0x00000008
#define RCR_ACPT_ALL		(RCR_AAP | RCR_APM | RCR_AM | RCR_AB)

/* PLA_RXFIFO_CTRL0 */
#define RXFIFO_THR1_NORMAL	0x00080002
#define RXFIFO_THR1_OOB		0x01800003

/* PLA_RXFIFO_CTRL1 */
#define RXFIFO_THR2_FULL	0x00000060
#define RXFIFO_THR2_HIGH	0x00000038
#define RXFIFO_THR2_OOB		0x0000004a
#define RXFIFO_THR2_NORMAL	0x00a0

/* PLA_RXFIFO_CTRL2 */
#define RXFIFO_THR3_FULL	0x00000078
#define RXFIFO_THR3_HIGH	0x00000048
#define RXFIFO_THR3_OOB		0x0000005a
#define RXFIFO_THR3_NORMAL	0x0110

/* PLA_TXFIFO_CTRL */
#define TXFIFO_THR_NORMAL	0x00400008
#define TXFIFO_THR_NORMAL2	0x01000008

/* PLA_DMY_REG0 */
#define ECM_ALDPS		0x0002

/* PLA_FMC */
#define FMC_FCR_MCU_EN		0x0001

/* PLA_EEEP_CR */
#define EEEP_CR_EEEP_TX		0x0002

/* PLA_WDT6_CTRL */
#define WDT6_SET_MODE		0x0010

/* PLA_TCR0 */
#define TCR0_TX_EMPTY		0x0800
#define TCR0_AUTO_FIFO		0x0080

/* PLA_TCR1 */
#define VERSION_MASK		0x7cf0

/* PLA_MTPS */
#define MTPS_JUMBO		(12 * 1024 / 64)
#define MTPS_DEFAULT		(6 * 1024 / 64)

/* PLA_RSTTALLY */
#define TALLY_RESET		0x0001

/* PLA_CR */
#define PLA_CR_RST		0x10
#define PLA_CR_RE		0x08
#define PLA_CR_TE		0x04

/* PLA_BIST_CTRL */
#define BIST_CTRL_SW_RESET (0x10 << 24)

/* PLA_CRWECR */
#define CRWECR_NORAML		0x00
#define CRWECR_CONFIG		0xc0

/* PLA_OOB_CTRL */
#define NOW_IS_OOB		0x80
#define TXFIFO_EMPTY		0x20
#define RXFIFO_EMPTY		0x10
#define LINK_LIST_READY		0x02
#define DIS_MCU_CLROOB		0x01
#define FIFO_EMPTY		(TXFIFO_EMPTY | RXFIFO_EMPTY)

/* PLA_PHY_PWR */
#define PLA_PHY_PWR_LLR	(LINK_LIST_READY << 24)
#define PLA_PHY_PWR_TXEMP	(TXFIFO_EMPTY << 24)

/* PLA_MISC_1 */
#define RXDY_GATED_EN		0x0008

/* PLA_SFF_STS_7 */
#define RE_INIT_LL		0x8000
#define MCU_BORW_EN		0x4000

/* PLA_CPCR */
#define CPCR_RX_VLAN		0x0040

/* PLA_CFG_WOL */
#define MAGIC_EN		0x0001

/* PLA_TEREDO_CFG */
#define TEREDO_SEL		0x8000
#define TEREDO_WAKE_MASK	0x7f00
#define TEREDO_RS_EVENT_MASK	0x00fe
#define OOB_TEREDO_EN		0x0001

/* PAL_BDC_CR */
#define ALDPS_PROXY_MODE	0x0001

/* PLA_CONFIG34 */
#define LINK_ON_WAKE_EN		0x0010
#define LINK_OFF_WAKE_EN	0x0008

/* PLA_CONFIG5 */
#define BWF_EN			0x0040
#define MWF_EN			0x0020
#define UWF_EN			0x0010
#define LAN_WAKE_EN		0x0002

/* PLA_LED_FEATURE */
#define LED_MODE_MASK		0x0700

/* PLA_PHY_PWR */
#define TX_10M_IDLE_EN		0x0080
#define PFM_PWM_SWITCH		0x0040

/* PLA_MAC_PWR_CTRL */
#define D3_CLK_GATED_EN		0x00004000
#define MCU_CLK_RATIO		0x07010f07
#define MCU_CLK_RATIO_MASK	0x0f0f0f0f
#define ALDPS_SPDWN_RATIO	0x0f87

/* PLA_MAC_PWR_CTRL2 */
#define EEE_SPDWN_RATIO		0x8007

/* PLA_MAC_PWR_CTRL3 */
#define PKT_AVAIL_SPDWN_EN	0x0100
#define SUSPEND_SPDWN_EN	0x0004
#define U1U2_SPDWN_EN		0x0002
#define L1_SPDWN_EN		0x0001

/* PLA_MAC_PWR_CTRL4 */
#define PWRSAVE_SPDWN_EN	0x1000
#define RXDV_SPDWN_EN		0x0800
#define TX10MIDLE_EN		0x0100
#define TP100_SPDWN_EN		0x0020
#define TP500_SPDWN_EN		0x0010
#define TP1000_SPDWN_EN		0x0008
#define EEE_SPDWN_EN		0x0001

/* PLA_GPHY_INTR_IMR */
#define GPHY_STS_MSK		0x0001
#define SPEED_DOWN_MSK		0x0002
#define SPDWN_RXDV_MSK		0x0004
#define SPDWN_LINKCHG_MSK	0x0008

/* PLA_PHYAR */
#define PHYAR_FLAG		0x80000000

/* PLA_EEE_CR */
#define EEE_RX_EN		0x0001
#define EEE_TX_EN		0x0002

/* PLA_BOOT_CTRL */
#define AUTOLOAD_DONE		0x0002

/* USB_USB2PHY */
#define USB2PHY_SUSPEND		0x0001
#define USB2PHY_L1		0x0002

/* USB_SSPHYLINK2 */
#define pwd_dn_scale_mask	0x3ffe
#define pwd_dn_scale(x)		((x) << 1)

/* USB_CSR_DUMMY1 */
#define DYNAMIC_BURST		0x0001

/* USB_CSR_DUMMY2 */
#define EP4_FULL_FC		0x0001

/* USB_DEV_STAT */
#define STAT_SPEED_MASK		0x0006
#define STAT_SPEED_HIGH		0x0000
#define STAT_SPEED_FULL		0x0002

/* USB_TX_AGG */
#define TX_AGG_MAX_THRESHOLD	0x03

/* USB_RX_BUF_TH */
#define RX_THR_SUPPER		0x0c350180
#define RX_THR_HIGH		0x7a120180
#define RX_THR_SLOW		0xffff0180

/* USB_TX_DMA */
#define TEST_MODE_DISABLE	0x00000001
#define TX_SIZE_ADJUST1		0x00000100

/* USB_UPS_CTRL */
#define POWER_CUT		0x0100

/* USB_PM_CTRL_STATUS */
#define RESUME_INDICATE		0x0001

/* USB_USB_CTRL */
#define RX_AGG_DISABLE		0x0010
#define RX_ZERO_EN		0x0080

/* USB_U2P3_CTRL */
#define U2P3_ENABLE		0x0001

/* USB_POWER_CUT */
#define PWR_EN			0x0001
#define PHASE2_EN		0x0008

/* USB_MISC_0 */
#define PCUT_STATUS		0x0001

/* USB_RX_EARLY_TIMEOUT */
#define COALESCE_SUPER		 85000U
#define COALESCE_HIGH		250000U
#define COALESCE_SLOW		524280U

/* USB_WDT11_CTRL */
#define TIMER11_EN		0x0001

/* USB_LPM_CTRL */
/* bit 4 ~ 5: fifo empty boundary */
#define FIFO_EMPTY_1FB		0x30	/* 0x1fb * 64 = 32448 bytes */
/* bit 2 ~ 3: LMP timer */
#define LPM_TIMER_MASK		0x0c
#define LPM_TIMER_500MS		0x04	/* 500 ms */
#define LPM_TIMER_500US		0x0c	/* 500 us */
#define ROK_EXIT_LPM		0x02

/* USB_AFE_CTRL2 */
#define SEN_VAL_MASK		0xf800
#define SEN_VAL_NORMAL		0xa000
#define SEL_RXIDLE		0x0100

/* OCP_ALDPS_CONFIG */
#define ENPWRSAVE		0x8000
#define ENPDNPS			0x0200
#define LINKENA			0x0100
#define DIS_SDSAVE		0x0010

/* OCP_PHY_STATUS */
#define PHY_STAT_MASK		0x0007
#define PHY_STAT_LAN_ON		3
#define PHY_STAT_PWRDN		5

/* OCP_POWER_CFG */
#define EEE_CLKDIV_EN		0x8000
#define EN_ALDPS		0x0004
#define EN_10M_PLLOFF		0x0001

/* OCP_EEE_CONFIG1 */
#define RG_TXLPI_MSK_HFDUP	0x8000
#define RG_MATCLR_EN		0x4000
#define EEE_10_CAP		0x2000
#define EEE_NWAY_EN		0x1000
#define TX_QUIET_EN		0x0200
#define RX_QUIET_EN		0x0100
#define sd_rise_time_mask	0x0070
#define sd_rise_time(x)		(min((x), 7) << 4)	/* bit 4 ~ 6 */
#define RG_RXLPI_MSK_HFDUP	0x0008
#define SDFALLTIME		0x0007	/* bit 0 ~ 2 */

/* OCP_EEE_CONFIG2 */
#define RG_LPIHYS_NUM		0x7000	/* bit 12 ~ 15 */
#define RG_DACQUIET_EN		0x0400
#define RG_LDVQUIET_EN		0x0200
#define RG_CKRSEL		0x0020
#define RG_EEEPRG_EN		0x0010

/* OCP_EEE_CONFIG3 */
#define fast_snr_mask		0xff80
#define fast_snr(x)		(min((x), 0x1ff) << 7)	/* bit 7 ~ 15 */
#define RG_LFS_SEL		0x0060	/* bit 6 ~ 5 */
#define MSK_PH			0x0006	/* bit 0 ~ 3 */

/* OCP_EEE_AR */
/* bit[15:14] function */
#define FUN_ADDR		0x0000
#define FUN_DATA		0x4000
/* bit[4:0] device addr */

/* OCP_EEE_CFG */
#define CTAP_SHORT_EN		0x0040
#define EEE10_EN		0x0010

/* OCP_DOWN_SPEED */
#define EN_10M_BGOFF		0x0080

/* OCP_PHY_STATE */
#define TXDIS_STATE		0x01
#define ABD_STATE		0x02

/* OCP_ADC_CFG */
#define CKADSEL_L		0x0100
#define ADC_EN			0x0080
#define EN_EMI_L		0x0040

/* SRAM_LPF_CFG */
#define LPF_AUTO_TUNE		0x8000

/* SRAM_10M_AMP1 */
#define GDAC_IB_UPALL		0x0008

/* SRAM_10M_AMP2 */
#define AMP_DN			0x0200

/* SRAM_IMPEDANCE */
#define RX_DRIVING_MASK		0x6000

#define RTL8152_MAX_TX		4
#define RTL8152_MAX_RX		10
#define INTBUFSIZE		2
#define CRC_SIZE		4
#define TX_ALIGN		4
#define RX_ALIGN		8

#define INTR_LINK		0x0004

#define RTL8152_REQT_READ	0xc0
#define RTL8152_REQT_WRITE	0x40
#define RTL8152_REQ_GET_REGS	0x05
#define RTL8152_REQ_SET_REGS	0x05

#define BYTE_EN_DWORD		0xff
#define BYTE_EN_WORD		0x33
#define BYTE_EN_BYTE		0x11
#define BYTE_EN_SIX_BYTES	0x3f
#define BYTE_EN_START_MASK	0x0f
#define BYTE_EN_END_MASK	0xf0

#define RTL8152_ETH_FRAME_LEN	1514
#define RTL8152_AGG_BUF_SZ	2048

#define RTL8152_RMS		(RTL8152_ETH_FRAME_LEN + CRC_SIZE)
#define RTL8153_RMS		(RTL8152_ETH_FRAME_LEN + CRC_SIZE)
#define RTL8152_TX_TIMEOUT	(5 * HZ)

#define MCU_TYPE_PLA			0x0100
#define MCU_TYPE_USB			0x0000

/* The forced speed, 10Mb, 100Mb, gigabit. */
#define SPEED_10                10
#define SPEED_100               100
#define SPEED_1000              1000

#define SPEED_UNKNOWN           -1

/* Duplex, half or full. */
#define DUPLEX_HALF             0x00
#define DUPLEX_FULL             0x01
#define DUPLEX_UNKNOWN          0xff

/* Enable or disable autonegotiation. */
#define AUTONEG_DISABLE         0x00
#define AUTONEG_ENABLE          0x01

/* Generic MII registers. */
#define MII_BMCR                0x00    /* Basic mode control register */
#define MII_BMSR                0x01    /* Basic mode status register  */
#define MII_PHYSID1             0x02    /* PHYS ID 1                   */
#define MII_PHYSID2             0x03    /* PHYS ID 2                   */
#define MII_ADVERTISE           0x04    /* Advertisement control reg   */
#define MII_LPA                 0x05    /* Link partner ability reg    */
#define MII_EXPANSION           0x06    /* Expansion register          */
#define MII_CTRL1000            0x09    /* 1000BASE-T control          */
#define MII_STAT1000            0x0a    /* 1000BASE-T status           */
#define MII_MMD_CTRL            0x0d    /* MMD Access Control Register */
#define MII_MMD_DATA            0x0e    /* MMD Access Data Register */
#define MII_ESTATUS             0x0f    /* Extended Status             */
#define MII_DCOUNTER            0x12    /* Disconnect counter          */
#define MII_FCSCOUNTER          0x13    /* False carrier counter       */
#define MII_NWAYTEST            0x14    /* N-way auto-neg test reg     */
#define MII_RERRCOUNTER         0x15    /* Receive error counter       */
#define MII_SREVISION           0x16    /* Silicon revision            */
#define MII_RESV1               0x17    /* Reserved...                 */
#define MII_LBRERROR            0x18    /* Lpback, rx, bypass error    */
#define MII_PHYADDR             0x19    /* PHY address                 */
#define MII_RESV2               0x1a    /* Reserved...                 */
#define MII_TPISTATUS           0x1b    /* TPI status for 10mbps       */
#define MII_NCONFIG             0x1c    /* Network interface config    */

#define TIMEOUT_RESOLUTION	50
#define PHY_CONNECT_TIMEOUT     5000
#define USB_BULK_SEND_TIMEOUT   5000
#define USB_BULK_RECV_TIMEOUT   5000
#define R8152_WAIT_TIMEOUT	2000

struct rx_desc {
	__le32 opts1;
#define RD_CRC				BIT(15)
#define RX_LEN_MASK			0x7fff

	__le32 opts2;
#define RD_UDP_CS			BIT(23)
#define RD_TCP_CS			BIT(22)
#define RD_IPV6_CS			BIT(20)
#define RD_IPV4_CS			BIT(19)

	__le32 opts3;
#define IPF				BIT(23) /* IP checksum fail */
#define UDPF				BIT(22) /* UDP checksum fail */
#define TCPF				BIT(21) /* TCP checksum fail */
#define RX_VLAN_TAG			BIT(16)

	__le32 opts4;
	__le32 opts5;
	__le32 opts6;
};

struct tx_desc {
	__le32 opts1;
#define TX_FS			BIT(31) /* First segment of a packet */
#define TX_LS			BIT(30) /* Final segment of a packet */
#define LGSEND			BIT(29)
#define GTSENDV4		BIT(28)
#define GTSENDV6		BIT(27)
#define GTTCPHO_SHIFT		18
#define GTTCPHO_MAX		0x7fU
#define TX_LEN_MAX		0x3ffffU

	__le32 opts2;
#define UDP_CS			BIT(31) /* Calculate UDP/IP checksum */
#define TCP_CS			BIT(30) /* Calculate TCP/IP checksum */
#define IPV4_CS			BIT(29) /* Calculate IPv4 checksum */
#define IPV6_CS			BIT(28) /* Calculate IPv6 checksum */
#define MSS_SHIFT		17
#define MSS_MAX			0x7ffU
#define TCPHO_SHIFT		17
#define TCPHO_MAX		0x7ffU
#define TX_VLAN_TAG		BIT(16)
};

enum rtl_version {
	RTL_VER_UNKNOWN = 0,
	RTL_VER_01,
	RTL_VER_02,
	RTL_VER_03,
	RTL_VER_04,
	RTL_VER_05,
	RTL_VER_06,
	RTL_VER_07,
	RTL_VER_MAX
};

enum rtl_register_content {
	_1000bps	= 0x10,
	_100bps		= 0x08,
	_10bps		= 0x04,
	LINK_STATUS	= 0x02,
	FULL_DUP	= 0x01,
};

struct r8152 {
	struct usb_device *udev;
	struct usb_interface *intf;
	bool supports_gmii;

	struct rtl_ops {
		void (*init)(struct r8152 *);
		int (*enable)(struct r8152 *);
		void (*disable)(struct r8152 *);
		void (*up)(struct r8152 *);
		void (*down)(struct r8152 *);
		void (*unload)(struct r8152 *);
	} rtl_ops;

	u32 coalesce;
	u16 ocp_base;

	u8 version;

#ifdef CONFIG_DM_ETH
	struct ueth_data ueth;
#endif
};

int generic_ocp_write(struct r8152 *tp, u16 index, u16 byteen,
		      u16 size, void *data, u16 type);
int generic_ocp_read(struct r8152 *tp, u16 index, u16 size,
		     void *data, u16 type);

int pla_ocp_read(struct r8152 *tp, u16 index, u16 size, void *data);
int pla_ocp_write(struct r8152 *tp, u16 index, u16 byteen,
		  u16 size, void *data);

int usb_ocp_read(struct r8152 *tp, u16 index, u16 size, void *data);
int usb_ocp_write(struct r8152 *tp, u16 index, u16 byteen,
		  u16 size, void *data);

u32 ocp_read_dword(struct r8152 *tp, u16 type, u16 index);
void ocp_write_dword(struct r8152 *tp, u16 type, u16 index, u32 data);

u16 ocp_read_word(struct r8152 *tp, u16 type, u16 index);
void ocp_write_word(struct r8152 *tp, u16 type, u16 index, u32 data);

u8 ocp_read_byte(struct r8152 *tp, u16 type, u16 index);
void ocp_write_byte(struct r8152 *tp, u16 type, u16 index, u32 data);

u16 ocp_reg_read(struct r8152 *tp, u16 addr);
void ocp_reg_write(struct r8152 *tp, u16 addr, u16 data);

void sram_write(struct r8152 *tp, u16 addr, u16 data);

int r8152_wait_for_bit(struct r8152 *tp, bool ocp_reg, u16 type, u16 index,
		       const u32 mask, bool set, unsigned int timeout);

void r8152b_firmware(struct r8152 *tp);
void r8153_firmware(struct r8152 *tp);
#endif
