// SPDX-License-Identifier: GPL-2.0+
/*
 * Ethernet driver for TI TMS320DM644x (DaVinci) chips.
 *
 * Copyright (C) 2007 Sergey Kubushyn <ksi@koi8.net>
 *
 * Parts shamelessly stolen from TI's dm644x_emac.c. Original copyright
 * follows:
 *
 * ----------------------------------------------------------------------------
 *
 * dm644x_emac.c
 *
 * TI DaVinci (DM644X) EMAC peripheral driver source for DV-EVM
 *
 * Copyright (C) 2005 Texas Instruments.
 *
 * ----------------------------------------------------------------------------
 *
 * Modifications:
 * ver. 1.0: Sep 2005, Anant Gole - Created EMAC version for uBoot.
 * ver  1.1: Nov 2005, Anant Gole - Extended the RX logic for multiple descriptors
 */
#include <common.h>
#include <command.h>
#include <net.h>
#include <miiphy.h>
#include <malloc.h>
#include <netdev.h>
#include <linux/compiler.h>
#include <asm/arch/emac_defs.h>
#include <asm/io.h>
#include "davinci_emac.h"

unsigned int	emac_dbg = 0;
#define debug_emac(fmt,args...)	if (emac_dbg) printf(fmt,##args)

#ifdef EMAC_HW_RAM_ADDR
static inline unsigned long BD_TO_HW(unsigned long x)
{
	if (x == 0)
		return 0;

	return x - EMAC_WRAPPER_RAM_ADDR + EMAC_HW_RAM_ADDR;
}

static inline unsigned long HW_TO_BD(unsigned long x)
{
	if (x == 0)
		return 0;

	return x - EMAC_HW_RAM_ADDR + EMAC_WRAPPER_RAM_ADDR;
}
#else
#define BD_TO_HW(x)	(x)
#define HW_TO_BD(x)	(x)
#endif

#ifdef DAVINCI_EMAC_GIG_ENABLE
#define emac_gigabit_enable(phy_addr)	davinci_eth_gigabit_enable(phy_addr)
#else
#define emac_gigabit_enable(phy_addr)	/* no gigabit to enable */
#endif

#if !defined(CONFIG_SYS_EMAC_TI_CLKDIV)
#define CONFIG_SYS_EMAC_TI_CLKDIV	((EMAC_MDIO_BUS_FREQ / \
		EMAC_MDIO_CLOCK_FREQ) - 1)
#endif

static void davinci_eth_mdio_enable(void);

static int gen_init_phy(int phy_addr);
static int gen_is_phy_connected(int phy_addr);
static int gen_get_link_speed(int phy_addr);
static int gen_auto_negotiate(int phy_addr);

void eth_mdio_enable(void)
{
	davinci_eth_mdio_enable();
}

/* EMAC Addresses */
static volatile emac_regs	*adap_emac = (emac_regs *)EMAC_BASE_ADDR;
static volatile ewrap_regs	*adap_ewrap = (ewrap_regs *)EMAC_WRAPPER_BASE_ADDR;
static volatile mdio_regs	*adap_mdio = (mdio_regs *)EMAC_MDIO_BASE_ADDR;

/* EMAC descriptors */
static volatile emac_desc	*emac_rx_desc = (emac_desc *)(EMAC_WRAPPER_RAM_ADDR + EMAC_RX_DESC_BASE);
static volatile emac_desc	*emac_tx_desc = (emac_desc *)(EMAC_WRAPPER_RAM_ADDR + EMAC_TX_DESC_BASE);
static volatile emac_desc	*emac_rx_active_head = 0;
static volatile emac_desc	*emac_rx_active_tail = 0;
static int			emac_rx_queue_active = 0;

/* Receive packet buffers */
static unsigned char emac_rx_buffers[EMAC_MAX_RX_BUFFERS * EMAC_RXBUF_SIZE]
				__aligned(ARCH_DMA_MINALIGN);

#ifndef CONFIG_SYS_DAVINCI_EMAC_PHY_COUNT
#define CONFIG_SYS_DAVINCI_EMAC_PHY_COUNT	3
#endif

/* PHY address for a discovered PHY (0xff - not found) */
static u_int8_t	active_phy_addr[CONFIG_SYS_DAVINCI_EMAC_PHY_COUNT];

/* number of PHY found active */
static u_int8_t	num_phy;

phy_t				phy[CONFIG_SYS_DAVINCI_EMAC_PHY_COUNT];

static int davinci_eth_set_mac_addr(struct eth_device *dev)
{
	unsigned long		mac_hi;
	unsigned long		mac_lo;

	/*
	 * Set MAC Addresses & Init multicast Hash to 0 (disable any multicast
	 * receive)
	 *  Using channel 0 only - other channels are disabled
	 *  */
	writel(0, &adap_emac->MACINDEX);
	mac_hi = (dev->enetaddr[3] << 24) |
		 (dev->enetaddr[2] << 16) |
		 (dev->enetaddr[1] << 8)  |
		 (dev->enetaddr[0]);
	mac_lo = (dev->enetaddr[5] << 8) |
		 (dev->enetaddr[4]);

	writel(mac_hi, &adap_emac->MACADDRHI);
#if defined(DAVINCI_EMAC_VERSION2)
	writel(mac_lo | EMAC_MAC_ADDR_IS_VALID | EMAC_MAC_ADDR_MATCH,
	       &adap_emac->MACADDRLO);
#else
	writel(mac_lo, &adap_emac->MACADDRLO);
#endif

	writel(0, &adap_emac->MACHASH1);
	writel(0, &adap_emac->MACHASH2);

	/* Set source MAC address - REQUIRED */
	writel(mac_hi, &adap_emac->MACSRCADDRHI);
	writel(mac_lo, &adap_emac->MACSRCADDRLO);


	return 0;
}

static void davinci_eth_mdio_enable(void)
{
	u_int32_t	clkdiv;

	clkdiv = CONFIG_SYS_EMAC_TI_CLKDIV;

	writel((clkdiv & 0xff) |
	       MDIO_CONTROL_ENABLE |
	       MDIO_CONTROL_FAULT |
	       MDIO_CONTROL_FAULT_ENABLE,
	       &adap_mdio->CONTROL);

	while (readl(&adap_mdio->CONTROL) & MDIO_CONTROL_IDLE)
		;
}

/*
 * Tries to find an active connected PHY. Returns 1 if address if found.
 * If no active PHY (or more than one PHY) found returns 0.
 * Sets active_phy_addr variable.
 */
static int davinci_eth_phy_detect(void)
{
	u_int32_t	phy_act_state;
	int		i;
	int		j;
	unsigned int	count = 0;

	for (i = 0; i < CONFIG_SYS_DAVINCI_EMAC_PHY_COUNT; i++)
		active_phy_addr[i] = 0xff;

	udelay(1000);
	phy_act_state = readl(&adap_mdio->ALIVE);

	if (phy_act_state == 0)
		return 0;		/* No active PHYs */

	debug_emac("davinci_eth_phy_detect(), ALIVE = 0x%08x\n", phy_act_state);

	for (i = 0, j = 0; i < 32; i++)
		if (phy_act_state & (1 << i)) {
			count++;
			if (count <= CONFIG_SYS_DAVINCI_EMAC_PHY_COUNT) {
				active_phy_addr[j++] = i;
			} else {
				printf("%s: to many PHYs detected.\n",
					__func__);
				count = 0;
				break;
			}
		}

	num_phy = count;

	return count;
}


/* Read a PHY register via MDIO inteface. Returns 1 on success, 0 otherwise */
int davinci_eth_phy_read(u_int8_t phy_addr, u_int8_t reg_num, u_int16_t *data)
{
	int	tmp;

	while (readl(&adap_mdio->USERACCESS0) & MDIO_USERACCESS0_GO)
		;

	writel(MDIO_USERACCESS0_GO |
	       MDIO_USERACCESS0_WRITE_READ |
	       ((reg_num & 0x1f) << 21) |
	       ((phy_addr & 0x1f) << 16),
	       &adap_mdio->USERACCESS0);

	/* Wait for command to complete */
	while ((tmp = readl(&adap_mdio->USERACCESS0)) & MDIO_USERACCESS0_GO)
		;

	if (tmp & MDIO_USERACCESS0_ACK) {
		*data = tmp & 0xffff;
		return 1;
	}

	return 0;
}

/* Write to a PHY register via MDIO inteface. Blocks until operation is complete. */
int davinci_eth_phy_write(u_int8_t phy_addr, u_int8_t reg_num, u_int16_t data)
{

	while (readl(&adap_mdio->USERACCESS0) & MDIO_USERACCESS0_GO)
		;

	writel(MDIO_USERACCESS0_GO |
	       MDIO_USERACCESS0_WRITE_WRITE |
	       ((reg_num & 0x1f) << 21) |
	       ((phy_addr & 0x1f) << 16) |
	       (data & 0xffff),
	       &adap_mdio->USERACCESS0);

	/* Wait for command to complete */
	while (readl(&adap_mdio->USERACCESS0) & MDIO_USERACCESS0_GO)
		;

	return 1;
}

/* PHY functions for a generic PHY */
static int gen_init_phy(int phy_addr)
{
	int	ret = 1;

	if (gen_get_link_speed(phy_addr)) {
		/* Try another time */
		ret = gen_get_link_speed(phy_addr);
	}

	return(ret);
}

static int gen_is_phy_connected(int phy_addr)
{
	u_int16_t	dummy;

	return davinci_eth_phy_read(phy_addr, MII_PHYSID1, &dummy);
}

static int get_active_phy(void)
{
	int i;

	for (i = 0; i < num_phy; i++)
		if (phy[i].get_link_speed(active_phy_addr[i]))
			return i;

	return -1;	/* Return error if no link */
}

static int gen_get_link_speed(int phy_addr)
{
	u_int16_t	tmp;

	if (davinci_eth_phy_read(phy_addr, MII_STATUS_REG, &tmp) &&
			(tmp & 0x04)) {
#if defined(CONFIG_DRIVER_TI_EMAC_USE_RMII) && \
		defined(CONFIG_MACH_DAVINCI_DA850_EVM)
		davinci_eth_phy_read(phy_addr, MII_LPA, &tmp);

		/* Speed doesn't matter, there is no setting for it in EMAC. */
		if (tmp & (LPA_100FULL | LPA_10FULL)) {
			/* set EMAC for Full Duplex  */
			writel(EMAC_MACCONTROL_MIIEN_ENABLE |
					EMAC_MACCONTROL_FULLDUPLEX_ENABLE,
					&adap_emac->MACCONTROL);
		} else {
			/*set EMAC for Half Duplex  */
			writel(EMAC_MACCONTROL_MIIEN_ENABLE,
					&adap_emac->MACCONTROL);
		}

		if (tmp & (LPA_100FULL | LPA_100HALF))
			writel(readl(&adap_emac->MACCONTROL) |
					EMAC_MACCONTROL_RMIISPEED_100,
					 &adap_emac->MACCONTROL);
		else
			writel(readl(&adap_emac->MACCONTROL) &
					~EMAC_MACCONTROL_RMIISPEED_100,
					 &adap_emac->MACCONTROL);
#endif
		return(1);
	}

	return(0);
}

static int gen_auto_negotiate(int phy_addr)
{
	u_int16_t	tmp;
	u_int16_t	val;
	unsigned long	cntr = 0;

	if (!davinci_eth_phy_read(phy_addr, MII_BMCR, &tmp))
		return 0;

	val = tmp | BMCR_FULLDPLX | BMCR_ANENABLE |
						BMCR_SPEED100;
	davinci_eth_phy_write(phy_addr, MII_BMCR, val);

	if (!davinci_eth_phy_read(phy_addr, MII_ADVERTISE, &val))
		return 0;

	val |= (ADVERTISE_100FULL | ADVERTISE_100HALF | ADVERTISE_10FULL |
							ADVERTISE_10HALF);
	davinci_eth_phy_write(phy_addr, MII_ADVERTISE, val);

	if (!davinci_eth_phy_read(phy_addr, MII_BMCR, &tmp))
		return(0);

#ifdef DAVINCI_EMAC_GIG_ENABLE
	davinci_eth_phy_read(phy_addr, MII_CTRL1000, &val);
	val |= PHY_1000BTCR_1000FD;
	val &= ~PHY_1000BTCR_1000HD;
	davinci_eth_phy_write(phy_addr, MII_CTRL1000, val);
	davinci_eth_phy_read(phy_addr, MII_CTRL1000, &val);
#endif

	/* Restart Auto_negotiation  */
	tmp |= BMCR_ANRESTART;
	davinci_eth_phy_write(phy_addr, MII_BMCR, tmp);

	/*check AutoNegotiate complete */
	do {
		udelay(40000);
		if (!davinci_eth_phy_read(phy_addr, MII_BMSR, &tmp))
			return 0;

		if (tmp & BMSR_ANEGCOMPLETE)
			break;

		cntr++;
	} while (cntr < 200);

	if (!davinci_eth_phy_read(phy_addr, MII_BMSR, &tmp))
		return(0);

	if (!(tmp & BMSR_ANEGCOMPLETE))
		return(0);

	return(gen_get_link_speed(phy_addr));
}
/* End of generic PHY functions */


#if defined(CONFIG_MII) || defined(CONFIG_CMD_MII)
static int davinci_mii_phy_read(struct mii_dev *bus, int addr, int devad,
				int reg)
{
	unsigned short value = 0;
	int retval = davinci_eth_phy_read(addr, reg, &value);

	return retval ? value : -EIO;
}

static int davinci_mii_phy_write(struct mii_dev *bus, int addr, int devad,
				 int reg, u16 value)
{
	return davinci_eth_phy_write(addr, reg, value) ? 0 : 1;
}
#endif

static void  __attribute__((unused)) davinci_eth_gigabit_enable(int phy_addr)
{
	u_int16_t data;

	if (davinci_eth_phy_read(phy_addr, 0, &data)) {
		if (data & (1 << 6)) { /* speed selection MSB */
			/*
			 * Check if link detected is giga-bit
			 * If Gigabit mode detected, enable gigbit in MAC
			 */
			writel(readl(&adap_emac->MACCONTROL) |
				EMAC_MACCONTROL_GIGFORCE |
				EMAC_MACCONTROL_GIGABIT_ENABLE,
				&adap_emac->MACCONTROL);
		}
	}
}

/* Eth device open */
static int davinci_eth_open(struct eth_device *dev, bd_t *bis)
{
	dv_reg_p		addr;
	u_int32_t		clkdiv, cnt, mac_control;
	uint16_t		__maybe_unused lpa_val;
	volatile emac_desc	*rx_desc;
	int			index;

	debug_emac("+ emac_open\n");

	/* Reset EMAC module and disable interrupts in wrapper */
	writel(1, &adap_emac->SOFTRESET);
	while (readl(&adap_emac->SOFTRESET) != 0)
		;
#if defined(DAVINCI_EMAC_VERSION2)
	writel(1, &adap_ewrap->softrst);
	while (readl(&adap_ewrap->softrst) != 0)
		;
#else
	writel(0, &adap_ewrap->EWCTL);
	for (cnt = 0; cnt < 5; cnt++) {
		clkdiv = readl(&adap_ewrap->EWCTL);
	}
#endif

#if defined(CONFIG_DRIVER_TI_EMAC_USE_RMII) && \
	defined(CONFIG_MACH_DAVINCI_DA850_EVM)
	adap_ewrap->c0rxen = adap_ewrap->c1rxen = adap_ewrap->c2rxen = 0;
	adap_ewrap->c0txen = adap_ewrap->c1txen = adap_ewrap->c2txen = 0;
	adap_ewrap->c0miscen = adap_ewrap->c1miscen = adap_ewrap->c2miscen = 0;
#endif
	rx_desc = emac_rx_desc;

	writel(1, &adap_emac->TXCONTROL);
	writel(1, &adap_emac->RXCONTROL);

	davinci_eth_set_mac_addr(dev);

	/* Set DMA 8 TX / 8 RX Head pointers to 0 */
	addr = &adap_emac->TX0HDP;
	for (cnt = 0; cnt < 8; cnt++)
		writel(0, addr++);

	addr = &adap_emac->RX0HDP;
	for (cnt = 0; cnt < 8; cnt++)
		writel(0, addr++);

	/* Clear Statistics (do this before setting MacControl register) */
	addr = &adap_emac->RXGOODFRAMES;
	for(cnt = 0; cnt < EMAC_NUM_STATS; cnt++)
		writel(0, addr++);

	/* No multicast addressing */
	writel(0, &adap_emac->MACHASH1);
	writel(0, &adap_emac->MACHASH2);

	/* Create RX queue and set receive process in place */
	emac_rx_active_head = emac_rx_desc;
	for (cnt = 0; cnt < EMAC_MAX_RX_BUFFERS; cnt++) {
		rx_desc->next = BD_TO_HW((u_int32_t)(rx_desc + 1));
		rx_desc->buffer = &emac_rx_buffers[cnt * EMAC_RXBUF_SIZE];
		rx_desc->buff_off_len = EMAC_MAX_ETHERNET_PKT_SIZE;
		rx_desc->pkt_flag_len = EMAC_CPPI_OWNERSHIP_BIT;
		rx_desc++;
	}

	/* Finalize the rx desc list */
	rx_desc--;
	rx_desc->next = 0;
	emac_rx_active_tail = rx_desc;
	emac_rx_queue_active = 1;

	/* Enable TX/RX */
	writel(EMAC_MAX_ETHERNET_PKT_SIZE, &adap_emac->RXMAXLEN);
	writel(0, &adap_emac->RXBUFFEROFFSET);

	/*
	 * No fancy configs - Use this for promiscous debug
	 *   - EMAC_RXMBPENABLE_RXCAFEN_ENABLE
	 */
	writel(EMAC_RXMBPENABLE_RXBROADEN, &adap_emac->RXMBPENABLE);

	/* Enable ch 0 only */
	writel(1, &adap_emac->RXUNICASTSET);

	/* Init MDIO & get link state */
	clkdiv = CONFIG_SYS_EMAC_TI_CLKDIV;
	writel((clkdiv & 0xff) | MDIO_CONTROL_ENABLE | MDIO_CONTROL_FAULT,
	       &adap_mdio->CONTROL);

	/* We need to wait for MDIO to start */
	udelay(1000);

	index = get_active_phy();
	if (index == -1)
		return(0);

	/* Enable MII interface */
	mac_control = EMAC_MACCONTROL_MIIEN_ENABLE;
#ifdef DAVINCI_EMAC_GIG_ENABLE
	davinci_eth_phy_read(active_phy_addr[index], MII_STAT1000, &lpa_val);
	if (lpa_val & PHY_1000BTSR_1000FD) {
		debug_emac("eth_open : gigabit negotiated\n");
		mac_control |= EMAC_MACCONTROL_FULLDUPLEX_ENABLE;
		mac_control |= EMAC_MACCONTROL_GIGABIT_ENABLE;
	}
#endif

	davinci_eth_phy_read(active_phy_addr[index], MII_LPA, &lpa_val);
	if (lpa_val & (LPA_100FULL | LPA_10FULL))
		/* set EMAC for Full Duplex  */
		mac_control |= EMAC_MACCONTROL_FULLDUPLEX_ENABLE;
#if defined(CONFIG_SOC_DA8XX) || \
	(defined(CONFIG_OMAP34XX) && defined(CONFIG_DRIVER_TI_EMAC_USE_RMII))
	mac_control |= EMAC_MACCONTROL_RMIISPEED_100;
#endif
	writel(mac_control, &adap_emac->MACCONTROL);
	/* Start receive process */
	writel(BD_TO_HW((u_int32_t)emac_rx_desc), &adap_emac->RX0HDP);

	debug_emac("- emac_open\n");

	return(1);
}

/* EMAC Channel Teardown */
static void davinci_eth_ch_teardown(int ch)
{
	dv_reg		dly = 0xff;
	dv_reg		cnt;

	debug_emac("+ emac_ch_teardown\n");

	if (ch == EMAC_CH_TX) {
		/* Init TX channel teardown */
		writel(0, &adap_emac->TXTEARDOWN);
		do {
			/*
			 * Wait here for Tx teardown completion interrupt to
			 * occur. Note: A task delay can be called here to pend
			 * rather than occupying CPU cycles - anyway it has
			 * been found that teardown takes very few cpu cycles
			 * and does not affect functionality
			 */
			dly--;
			udelay(1);
			if (dly == 0)
				break;
			cnt = readl(&adap_emac->TX0CP);
		} while (cnt != 0xfffffffc);
		writel(cnt, &adap_emac->TX0CP);
		writel(0, &adap_emac->TX0HDP);
	} else {
		/* Init RX channel teardown */
		writel(0, &adap_emac->RXTEARDOWN);
		do {
			/*
			 * Wait here for Rx teardown completion interrupt to
			 * occur. Note: A task delay can be called here to pend
			 * rather than occupying CPU cycles - anyway it has
			 * been found that teardown takes very few cpu cycles
			 * and does not affect functionality
			 */
			dly--;
			udelay(1);
			if (dly == 0)
				break;
			cnt = readl(&adap_emac->RX0CP);
		} while (cnt != 0xfffffffc);
		writel(cnt, &adap_emac->RX0CP);
		writel(0, &adap_emac->RX0HDP);
	}

	debug_emac("- emac_ch_teardown\n");
}

/* Eth device close */
static void davinci_eth_close(struct eth_device *dev)
{
	debug_emac("+ emac_close\n");

	davinci_eth_ch_teardown(EMAC_CH_TX);	/* TX Channel teardown */
	if (readl(&adap_emac->RXCONTROL) & 1)
		davinci_eth_ch_teardown(EMAC_CH_RX); /* RX Channel teardown */

	/* Reset EMAC module and disable interrupts in wrapper */
	writel(1, &adap_emac->SOFTRESET);
#if defined(DAVINCI_EMAC_VERSION2)
	writel(1, &adap_ewrap->softrst);
#else
	writel(0, &adap_ewrap->EWCTL);
#endif

#if defined(CONFIG_DRIVER_TI_EMAC_USE_RMII) && \
	defined(CONFIG_MACH_DAVINCI_DA850_EVM)
	adap_ewrap->c0rxen = adap_ewrap->c1rxen = adap_ewrap->c2rxen = 0;
	adap_ewrap->c0txen = adap_ewrap->c1txen = adap_ewrap->c2txen = 0;
	adap_ewrap->c0miscen = adap_ewrap->c1miscen = adap_ewrap->c2miscen = 0;
#endif
	debug_emac("- emac_close\n");
}

static int tx_send_loop = 0;

/*
 * This function sends a single packet on the network and returns
 * positive number (number of bytes transmitted) or negative for error
 */
static int davinci_eth_send_packet (struct eth_device *dev,
					void *packet, int length)
{
	int ret_status = -1;
	int index;
	tx_send_loop = 0;

	index = get_active_phy();
	if (index == -1) {
		printf(" WARN: emac_send_packet: No link\n");
		return (ret_status);
	}

	/* Check packet size and if < EMAC_MIN_ETHERNET_PKT_SIZE, pad it up */
	if (length < EMAC_MIN_ETHERNET_PKT_SIZE) {
		length = EMAC_MIN_ETHERNET_PKT_SIZE;
	}

	/* Populate the TX descriptor */
	emac_tx_desc->next = 0;
	emac_tx_desc->buffer = (u_int8_t *) packet;
	emac_tx_desc->buff_off_len = (length & 0xffff);
	emac_tx_desc->pkt_flag_len = ((length & 0xffff) |
				      EMAC_CPPI_SOP_BIT |
				      EMAC_CPPI_OWNERSHIP_BIT |
				      EMAC_CPPI_EOP_BIT);

	flush_dcache_range((unsigned long)packet,
			   (unsigned long)packet + ALIGN(length, PKTALIGN));

	/* Send the packet */
	writel(BD_TO_HW((unsigned long)emac_tx_desc), &adap_emac->TX0HDP);

	/* Wait for packet to complete or link down */
	while (1) {
		if (!phy[index].get_link_speed(active_phy_addr[index])) {
			davinci_eth_ch_teardown (EMAC_CH_TX);
			return (ret_status);
		}

		if (readl(&adap_emac->TXINTSTATRAW) & 0x01) {
			ret_status = length;
			break;
		}
		tx_send_loop++;
	}

	return (ret_status);
}

/*
 * This function handles receipt of a packet from the network
 */
static int davinci_eth_rcv_packet (struct eth_device *dev)
{
	volatile emac_desc *rx_curr_desc;
	volatile emac_desc *curr_desc;
	volatile emac_desc *tail_desc;
	int status, ret = -1;

	rx_curr_desc = emac_rx_active_head;
	if (!rx_curr_desc)
		return 0;
	status = rx_curr_desc->pkt_flag_len;
	if ((status & EMAC_CPPI_OWNERSHIP_BIT) == 0) {
		if (status & EMAC_CPPI_RX_ERROR_FRAME) {
			/* Error in packet - discard it and requeue desc */
			printf ("WARN: emac_rcv_pkt: Error in packet\n");
		} else {
			unsigned long tmp = (unsigned long)rx_curr_desc->buffer;
			unsigned short len =
				rx_curr_desc->buff_off_len & 0xffff;

			invalidate_dcache_range(tmp, tmp + ALIGN(len, PKTALIGN));
			net_process_received_packet(rx_curr_desc->buffer, len);
			ret = len;
		}

		/* Ack received packet descriptor */
		writel(BD_TO_HW((ulong)rx_curr_desc), &adap_emac->RX0CP);
		curr_desc = rx_curr_desc;
		emac_rx_active_head =
			(volatile emac_desc *) (HW_TO_BD(rx_curr_desc->next));

		if (status & EMAC_CPPI_EOQ_BIT) {
			if (emac_rx_active_head) {
				writel(BD_TO_HW((ulong)emac_rx_active_head),
				       &adap_emac->RX0HDP);
			} else {
				emac_rx_queue_active = 0;
				printf ("INFO:emac_rcv_packet: RX Queue not active\n");
			}
		}

		/* Recycle RX descriptor */
		rx_curr_desc->buff_off_len = EMAC_MAX_ETHERNET_PKT_SIZE;
		rx_curr_desc->pkt_flag_len = EMAC_CPPI_OWNERSHIP_BIT;
		rx_curr_desc->next = 0;

		if (emac_rx_active_head == 0) {
			printf ("INFO: emac_rcv_pkt: active queue head = 0\n");
			emac_rx_active_head = curr_desc;
			emac_rx_active_tail = curr_desc;
			if (emac_rx_queue_active != 0) {
				writel(BD_TO_HW((ulong)emac_rx_active_head),
				       &adap_emac->RX0HDP);
				printf ("INFO: emac_rcv_pkt: active queue head = 0, HDP fired\n");
				emac_rx_queue_active = 1;
			}
		} else {
			tail_desc = emac_rx_active_tail;
			emac_rx_active_tail = curr_desc;
			tail_desc->next = BD_TO_HW((ulong) curr_desc);
			status = tail_desc->pkt_flag_len;
			if (status & EMAC_CPPI_EOQ_BIT) {
				writel(BD_TO_HW((ulong)curr_desc),
				       &adap_emac->RX0HDP);
				status &= ~EMAC_CPPI_EOQ_BIT;
				tail_desc->pkt_flag_len = status;
			}
		}
		return (ret);
	}
	return (0);
}

/*
 * This function initializes the emac hardware. It does NOT initialize
 * EMAC modules power or pin multiplexors, that is done by board_init()
 * much earlier in bootup process. Returns 1 on success, 0 otherwise.
 */
int davinci_emac_initialize(void)
{
	u_int32_t	phy_id;
	u_int16_t	tmp;
	int		i;
	int		ret;
	struct eth_device *dev;

	dev = malloc(sizeof *dev);

	if (dev == NULL)
		return -1;

	memset(dev, 0, sizeof *dev);
	strcpy(dev->name, "DaVinci-EMAC");

	dev->iobase = 0;
	dev->init = davinci_eth_open;
	dev->halt = davinci_eth_close;
	dev->send = davinci_eth_send_packet;
	dev->recv = davinci_eth_rcv_packet;
	dev->write_hwaddr = davinci_eth_set_mac_addr;

	eth_register(dev);

	davinci_eth_mdio_enable();

	/* let the EMAC detect the PHYs */
	udelay(5000);

	for (i = 0; i < 256; i++) {
		if (readl(&adap_mdio->ALIVE))
			break;
		udelay(1000);
	}

	if (i >= 256) {
		printf("No ETH PHY detected!!!\n");
		return(0);
	}

	/* Find if PHY(s) is/are connected */
	ret = davinci_eth_phy_detect();
	if (!ret)
		return(0);
	else
		debug_emac(" %d ETH PHY detected\n", ret);

	/* Get PHY ID and initialize phy_ops for a detected PHY */
	for (i = 0; i < num_phy; i++) {
		if (!davinci_eth_phy_read(active_phy_addr[i], MII_PHYSID1,
							&tmp)) {
			active_phy_addr[i] = 0xff;
			continue;
		}

		phy_id = (tmp << 16) & 0xffff0000;

		if (!davinci_eth_phy_read(active_phy_addr[i], MII_PHYSID2,
							&tmp)) {
			active_phy_addr[i] = 0xff;
			continue;
		}

		phy_id |= tmp & 0x0000ffff;

		sprintf(phy[i].name, "GENERIC @ 0x%02x",
			active_phy_addr[i]);
		phy[i].init = gen_init_phy;
		phy[i].is_phy_connected = gen_is_phy_connected;
		phy[i].get_link_speed = gen_get_link_speed;
		phy[i].auto_negotiate = gen_auto_negotiate;

		debug("Ethernet PHY: %s\n", phy[i].name);

		int retval;
		struct mii_dev *mdiodev = mdio_alloc();
		if (!mdiodev)
			return -ENOMEM;
		strncpy(mdiodev->name, phy[i].name, MDIO_NAME_LEN);
		mdiodev->read = davinci_mii_phy_read;
		mdiodev->write = davinci_mii_phy_write;

		retval = mdio_register(mdiodev);
		if (retval < 0)
			return retval;
#ifdef DAVINCI_EMAC_GIG_ENABLE
#define PHY_CONF_REG	22
		/* Enable PHY to clock out TX_CLK */
		davinci_eth_phy_read(active_phy_addr[i], PHY_CONF_REG, &tmp);
		tmp |= PHY_CONF_TXCLKEN;
		davinci_eth_phy_write(active_phy_addr[i], PHY_CONF_REG, tmp);
		davinci_eth_phy_read(active_phy_addr[i], PHY_CONF_REG, &tmp);
#endif
	}

#if defined(CONFIG_TI816X) || (defined(CONFIG_DRIVER_TI_EMAC_USE_RMII) && \
		defined(CONFIG_MACH_DAVINCI_DA850_EVM) && \
			!defined(CONFIG_DRIVER_TI_EMAC_RMII_NO_NEGOTIATE))
	for (i = 0; i < num_phy; i++) {
		if (phy[i].is_phy_connected(i))
			phy[i].auto_negotiate(i);
	}
#endif
	return(1);
}
