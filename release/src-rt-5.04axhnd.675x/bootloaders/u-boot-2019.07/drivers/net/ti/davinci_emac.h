/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2011 Ilya Yanok, Emcraft Systems
 *
 * Based on: mach-davinci/emac_defs.h
 * Copyright (C) 2007 Sergey Kubushyn <ksi@koi8.net>
 */

#ifndef _DAVINCI_EMAC_H_
#define _DAVINCI_EMAC_H_
/* Ethernet Min/Max packet size */
#define EMAC_MIN_ETHERNET_PKT_SIZE	60
#define EMAC_MAX_ETHERNET_PKT_SIZE	1518
/* Buffer size (should be aligned on 32 byte and cache line) */
#define EMAC_RXBUF_SIZE	ALIGN(ALIGN(EMAC_MAX_ETHERNET_PKT_SIZE, 32),\
				ARCH_DMA_MINALIGN)

/* Number of RX packet buffers
 * NOTE: Only 1 buffer supported as of now
 */
#define EMAC_MAX_RX_BUFFERS		10


/***********************************************
 ******** Internally used macros ***************
 ***********************************************/

#define EMAC_CH_TX			1
#define EMAC_CH_RX			0

/* Each descriptor occupies 4 words, lets start RX desc's at 0 and
 * reserve space for 64 descriptors max
 */
#define EMAC_RX_DESC_BASE		0x0
#define EMAC_TX_DESC_BASE		0x1000

/* EMAC Teardown value */
#define EMAC_TEARDOWN_VALUE		0xfffffffc

/* MII Status Register */
#define MII_STATUS_REG			1
/* PHY Configuration register */
#define PHY_CONF_TXCLKEN		(1 << 5)

/* Number of statistics registers */
#define EMAC_NUM_STATS			36


/* EMAC Descriptor */
typedef volatile struct _emac_desc
{
	u_int32_t	next;		/* Pointer to next descriptor
					   in chain */
	u_int8_t	*buffer;	/* Pointer to data buffer */
	u_int32_t	buff_off_len;	/* Buffer Offset(MSW) and Length(LSW) */
	u_int32_t	pkt_flag_len;	/* Packet Flags(MSW) and Length(LSW) */
} emac_desc;

/* CPPI bit positions */
#define EMAC_CPPI_SOP_BIT		(0x80000000)
#define EMAC_CPPI_EOP_BIT		(0x40000000)
#define EMAC_CPPI_OWNERSHIP_BIT		(0x20000000)
#define EMAC_CPPI_EOQ_BIT		(0x10000000)
#define EMAC_CPPI_TEARDOWN_COMPLETE_BIT	(0x08000000)
#define EMAC_CPPI_PASS_CRC_BIT		(0x04000000)

#define EMAC_CPPI_RX_ERROR_FRAME	(0x03fc0000)

#define EMAC_MACCONTROL_MIIEN_ENABLE		(0x20)
#define EMAC_MACCONTROL_FULLDUPLEX_ENABLE	(0x1)
#define EMAC_MACCONTROL_GIGABIT_ENABLE		(1 << 7)
#define EMAC_MACCONTROL_GIGFORCE		(1 << 17)
#define EMAC_MACCONTROL_RMIISPEED_100		(1 << 15)

#define EMAC_MAC_ADDR_MATCH		(1 << 19)
#define EMAC_MAC_ADDR_IS_VALID		(1 << 20)

#define EMAC_RXMBPENABLE_RXCAFEN_ENABLE	(0x200000)
#define EMAC_RXMBPENABLE_RXBROADEN	(0x2000)


#define MDIO_CONTROL_IDLE		(0x80000000)
#define MDIO_CONTROL_ENABLE		(0x40000000)
#define MDIO_CONTROL_FAULT_ENABLE	(0x40000)
#define MDIO_CONTROL_FAULT		(0x80000)
#define MDIO_USERACCESS0_GO		(0x80000000)
#define MDIO_USERACCESS0_WRITE_READ	(0x0)
#define MDIO_USERACCESS0_WRITE_WRITE	(0x40000000)
#define MDIO_USERACCESS0_ACK		(0x20000000)

/* Ethernet MAC Registers Structure */
typedef struct  {
	dv_reg		TXIDVER;
	dv_reg		TXCONTROL;
	dv_reg		TXTEARDOWN;
	u_int8_t	RSVD0[4];
	dv_reg		RXIDVER;
	dv_reg		RXCONTROL;
	dv_reg		RXTEARDOWN;
	u_int8_t	RSVD1[100];
	dv_reg		TXINTSTATRAW;
	dv_reg		TXINTSTATMASKED;
	dv_reg		TXINTMASKSET;
	dv_reg		TXINTMASKCLEAR;
	dv_reg		MACINVECTOR;
	u_int8_t	RSVD2[12];
	dv_reg		RXINTSTATRAW;
	dv_reg		RXINTSTATMASKED;
	dv_reg		RXINTMASKSET;
	dv_reg		RXINTMASKCLEAR;
	dv_reg		MACINTSTATRAW;
	dv_reg		MACINTSTATMASKED;
	dv_reg		MACINTMASKSET;
	dv_reg		MACINTMASKCLEAR;
	u_int8_t	RSVD3[64];
	dv_reg		RXMBPENABLE;
	dv_reg		RXUNICASTSET;
	dv_reg		RXUNICASTCLEAR;
	dv_reg		RXMAXLEN;
	dv_reg		RXBUFFEROFFSET;
	dv_reg		RXFILTERLOWTHRESH;
	u_int8_t	RSVD4[8];
	dv_reg		RX0FLOWTHRESH;
	dv_reg		RX1FLOWTHRESH;
	dv_reg		RX2FLOWTHRESH;
	dv_reg		RX3FLOWTHRESH;
	dv_reg		RX4FLOWTHRESH;
	dv_reg		RX5FLOWTHRESH;
	dv_reg		RX6FLOWTHRESH;
	dv_reg		RX7FLOWTHRESH;
	dv_reg		RX0FREEBUFFER;
	dv_reg		RX1FREEBUFFER;
	dv_reg		RX2FREEBUFFER;
	dv_reg		RX3FREEBUFFER;
	dv_reg		RX4FREEBUFFER;
	dv_reg		RX5FREEBUFFER;
	dv_reg		RX6FREEBUFFER;
	dv_reg		RX7FREEBUFFER;
	dv_reg		MACCONTROL;
	dv_reg		MACSTATUS;
	dv_reg		EMCONTROL;
	dv_reg		FIFOCONTROL;
	dv_reg		MACCONFIG;
	dv_reg		SOFTRESET;
	u_int8_t	RSVD5[88];
	dv_reg		MACSRCADDRLO;
	dv_reg		MACSRCADDRHI;
	dv_reg		MACHASH1;
	dv_reg		MACHASH2;
	dv_reg		BOFFTEST;
	dv_reg		TPACETEST;
	dv_reg		RXPAUSE;
	dv_reg		TXPAUSE;
	u_int8_t	RSVD6[16];
	dv_reg		RXGOODFRAMES;
	dv_reg		RXBCASTFRAMES;
	dv_reg		RXMCASTFRAMES;
	dv_reg		RXPAUSEFRAMES;
	dv_reg		RXCRCERRORS;
	dv_reg		RXALIGNCODEERRORS;
	dv_reg		RXOVERSIZED;
	dv_reg		RXJABBER;
	dv_reg		RXUNDERSIZED;
	dv_reg		RXFRAGMENTS;
	dv_reg		RXFILTERED;
	dv_reg		RXQOSFILTERED;
	dv_reg		RXOCTETS;
	dv_reg		TXGOODFRAMES;
	dv_reg		TXBCASTFRAMES;
	dv_reg		TXMCASTFRAMES;
	dv_reg		TXPAUSEFRAMES;
	dv_reg		TXDEFERRED;
	dv_reg		TXCOLLISION;
	dv_reg		TXSINGLECOLL;
	dv_reg		TXMULTICOLL;
	dv_reg		TXEXCESSIVECOLL;
	dv_reg		TXLATECOLL;
	dv_reg		TXUNDERRUN;
	dv_reg		TXCARRIERSENSE;
	dv_reg		TXOCTETS;
	dv_reg		FRAME64;
	dv_reg		FRAME65T127;
	dv_reg		FRAME128T255;
	dv_reg		FRAME256T511;
	dv_reg		FRAME512T1023;
	dv_reg		FRAME1024TUP;
	dv_reg		NETOCTETS;
	dv_reg		RXSOFOVERRUNS;
	dv_reg		RXMOFOVERRUNS;
	dv_reg		RXDMAOVERRUNS;
	u_int8_t	RSVD7[624];
	dv_reg		MACADDRLO;
	dv_reg		MACADDRHI;
	dv_reg		MACINDEX;
	u_int8_t	RSVD8[244];
	dv_reg		TX0HDP;
	dv_reg		TX1HDP;
	dv_reg		TX2HDP;
	dv_reg		TX3HDP;
	dv_reg		TX4HDP;
	dv_reg		TX5HDP;
	dv_reg		TX6HDP;
	dv_reg		TX7HDP;
	dv_reg		RX0HDP;
	dv_reg		RX1HDP;
	dv_reg		RX2HDP;
	dv_reg		RX3HDP;
	dv_reg		RX4HDP;
	dv_reg		RX5HDP;
	dv_reg		RX6HDP;
	dv_reg		RX7HDP;
	dv_reg		TX0CP;
	dv_reg		TX1CP;
	dv_reg		TX2CP;
	dv_reg		TX3CP;
	dv_reg		TX4CP;
	dv_reg		TX5CP;
	dv_reg		TX6CP;
	dv_reg		TX7CP;
	dv_reg		RX0CP;
	dv_reg		RX1CP;
	dv_reg		RX2CP;
	dv_reg		RX3CP;
	dv_reg		RX4CP;
	dv_reg		RX5CP;
	dv_reg		RX6CP;
	dv_reg		RX7CP;
} emac_regs;

/* EMAC Wrapper Registers Structure */
typedef struct  {
#ifdef DAVINCI_EMAC_VERSION2
	dv_reg		idver;
	dv_reg		softrst;
	dv_reg		emctrl;
	dv_reg		c0rxthreshen;
	dv_reg		c0rxen;
	dv_reg		c0txen;
	dv_reg		c0miscen;
	dv_reg		c1rxthreshen;
	dv_reg		c1rxen;
	dv_reg		c1txen;
	dv_reg		c1miscen;
	dv_reg		c2rxthreshen;
	dv_reg		c2rxen;
	dv_reg		c2txen;
	dv_reg		c2miscen;
	dv_reg		c0rxthreshstat;
	dv_reg		c0rxstat;
	dv_reg		c0txstat;
	dv_reg		c0miscstat;
	dv_reg		c1rxthreshstat;
	dv_reg		c1rxstat;
	dv_reg		c1txstat;
	dv_reg		c1miscstat;
	dv_reg		c2rxthreshstat;
	dv_reg		c2rxstat;
	dv_reg		c2txstat;
	dv_reg		c2miscstat;
	dv_reg		c0rximax;
	dv_reg		c0tximax;
	dv_reg		c1rximax;
	dv_reg		c1tximax;
	dv_reg		c2rximax;
	dv_reg		c2tximax;
#else
	u_int8_t	RSVD0[4100];
	dv_reg		EWCTL;
	dv_reg		EWINTTCNT;
#endif
} ewrap_regs;

/* EMAC MDIO Registers Structure */
typedef struct  {
	dv_reg		VERSION;
	dv_reg		CONTROL;
	dv_reg		ALIVE;
	dv_reg		LINK;
	dv_reg		LINKINTRAW;
	dv_reg		LINKINTMASKED;
	u_int8_t	RSVD0[8];
	dv_reg		USERINTRAW;
	dv_reg		USERINTMASKED;
	dv_reg		USERINTMASKSET;
	dv_reg		USERINTMASKCLEAR;
	u_int8_t	RSVD1[80];
	dv_reg		USERACCESS0;
	dv_reg		USERPHYSEL0;
	dv_reg		USERACCESS1;
	dv_reg		USERPHYSEL1;
} mdio_regs;

int davinci_eth_phy_read(u_int8_t phy_addr, u_int8_t reg_num, u_int16_t *data);
int davinci_eth_phy_write(u_int8_t phy_addr, u_int8_t reg_num, u_int16_t data);

typedef struct {
	char	name[64];
	int	(*init)(int phy_addr);
	int	(*is_phy_connected)(int phy_addr);
	int	(*get_link_speed)(int phy_addr);
	int	(*auto_negotiate)(int phy_addr);
} phy_t;

#endif /* _DAVINCI_EMAC_H_ */
