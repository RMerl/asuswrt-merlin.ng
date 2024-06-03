// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2011
 * eInfochips Ltd. <www.einfochips.com>
 * Written-by: Ajay Bhargav <contact@8051projects.net>
 *
 * (C) Copyright 2010
 * Marvell Semiconductor <www.marvell.com>
 * Contributor: Mahavir Jain <mjain@marvell.com>
 */

#include <common.h>
#include <net.h>
#include <malloc.h>
#include <miiphy.h>
#include <netdev.h>
#include <asm/types.h>
#include <asm/byteorder.h>
#include <linux/err.h>
#include <linux/mii.h>
#include <asm/io.h>
#include <asm/arch/armada100.h>
#include "armada100_fec.h"

#define  PHY_ADR_REQ     0xFF	/* Magic number to read/write PHY address */

#ifdef DEBUG
static int eth_dump_regs(struct eth_device *dev)
{
	struct armdfec_device *darmdfec = to_darmdfec(dev);
	struct armdfec_reg *regs = darmdfec->regs;
	unsigned int i = 0;

	printf("\noffset: phy_adr, value: 0x%x\n", readl(&regs->phyadr));
	printf("offset: smi, value: 0x%x\n", readl(&regs->smi));
	for (i = 0x400; i <= 0x4e4; i += 4)
		printf("offset: 0x%x, value: 0x%x\n",
			i, readl(ARMD1_FEC_BASE + i));
	return 0;
}
#endif

static int armdfec_phy_timeout(u32 *reg, u32 flag, int cond)
{
	u32 timeout = PHY_WAIT_ITERATIONS;
	u32 reg_val;

	while (--timeout) {
		reg_val = readl(reg);
		if (cond && (reg_val & flag))
			break;
		else if (!cond && !(reg_val & flag))
			break;
		udelay(PHY_WAIT_MICRO_SECONDS);
	}
	return !timeout;
}

static int smi_reg_read(struct mii_dev *bus, int phy_addr, int devad,
			int phy_reg)
{
	u16 value = 0;
	struct eth_device *dev = eth_get_dev_by_name(bus->name);
	struct armdfec_device *darmdfec = to_darmdfec(dev);
	struct armdfec_reg *regs = darmdfec->regs;
	u32 val;

	if (phy_addr == PHY_ADR_REQ && phy_reg == PHY_ADR_REQ) {
		val = readl(&regs->phyadr);
		value = val & 0x1f;
		return value;
	}

	/* check parameters */
	if (phy_addr > PHY_MASK) {
		printf("ARMD100 FEC: (%s) Invalid phy address: 0x%X\n",
				__func__, phy_addr);
		return -EINVAL;
	}
	if (phy_reg > PHY_MASK) {
		printf("ARMD100 FEC: (%s) Invalid register offset: 0x%X\n",
				__func__, phy_reg);
		return -EINVAL;
	}

	/* wait for the SMI register to become available */
	if (armdfec_phy_timeout(&regs->smi, SMI_BUSY, false)) {
		printf("ARMD100 FEC: (%s) PHY busy timeout\n",	__func__);
		return -1;
	}

	writel((phy_addr << 16) | (phy_reg << 21) | SMI_OP_R, &regs->smi);

	/* now wait for the data to be valid */
	if (armdfec_phy_timeout(&regs->smi, SMI_R_VALID, true)) {
		val = readl(&regs->smi);
		printf("ARMD100 FEC: (%s) PHY Read timeout, val=0x%x\n",
				__func__, val);
		return -1;
	}
	val = readl(&regs->smi);
	value = val & 0xffff;

	return value;
}

static int smi_reg_write(struct mii_dev *bus, int phy_addr, int devad,
			 int phy_reg, u16 value)
{
	struct eth_device *dev = eth_get_dev_by_name(bus->name);
	struct armdfec_device *darmdfec = to_darmdfec(dev);
	struct armdfec_reg *regs = darmdfec->regs;

	if (phy_addr == PHY_ADR_REQ && phy_reg == PHY_ADR_REQ) {
		clrsetbits_le32(&regs->phyadr, 0x1f, value & 0x1f);
		return 0;
	}

	/* check parameters */
	if (phy_addr > PHY_MASK) {
		printf("ARMD100 FEC: (%s) Invalid phy address\n", __func__);
		return -EINVAL;
	}
	if (phy_reg > PHY_MASK) {
		printf("ARMD100 FEC: (%s) Invalid register offset\n", __func__);
		return -EINVAL;
	}

	/* wait for the SMI register to become available */
	if (armdfec_phy_timeout(&regs->smi, SMI_BUSY, false)) {
		printf("ARMD100 FEC: (%s) PHY busy timeout\n",	__func__);
		return -1;
	}

	writel((phy_addr << 16) | (phy_reg << 21) | SMI_OP_W | (value & 0xffff),
			&regs->smi);
	return 0;
}

/*
 * Abort any transmit and receive operations and put DMA
 * in idle state. AT and AR bits are cleared upon entering
 * in IDLE state. So poll those bits to verify operation.
 */
static void abortdma(struct eth_device *dev)
{
	struct armdfec_device *darmdfec = to_darmdfec(dev);
	struct armdfec_reg *regs = darmdfec->regs;
	int delay;
	int maxretries = 40;
	u32 tmp;

	while (--maxretries) {
		writel(SDMA_CMD_AR | SDMA_CMD_AT, &regs->sdma_cmd);
		udelay(100);

		delay = 10;
		while (--delay) {
			tmp = readl(&regs->sdma_cmd);
			if (!(tmp & (SDMA_CMD_AR | SDMA_CMD_AT)))
				break;
			udelay(10);
		}
		if (delay)
			break;
	}

	if (!maxretries)
		printf("ARMD100 FEC: (%s) DMA Stuck\n", __func__);
}

static inline u32 nibble_swapping_32_bit(u32 x)
{
	return ((x & 0xf0f0f0f0) >> 4) | ((x & 0x0f0f0f0f) << 4);
}

static inline u32 nibble_swapping_16_bit(u32 x)
{
	return ((x & 0x0000f0f0) >> 4) | ((x & 0x00000f0f) << 4);
}

static inline u32 flip_4_bits(u32 x)
{
	return ((x & 0x01) << 3) | ((x & 0x002) << 1)
		| ((x & 0x04) >> 1) | ((x & 0x008) >> 3);
}

/*
 * This function will calculate the hash function of the address.
 * depends on the hash mode and hash size.
 * Inputs
 * mach             - the 2 most significant bytes of the MAC address.
 * macl             - the 4 least significant bytes of the MAC address.
 * Outputs
 * return the calculated entry.
 */
static u32 hash_function(u32 mach, u32 macl)
{
	u32 hashresult;
	u32 addrh;
	u32 addrl;
	u32 addr0;
	u32 addr1;
	u32 addr2;
	u32 addr3;
	u32 addrhswapped;
	u32 addrlswapped;

	addrh = nibble_swapping_16_bit(mach);
	addrl = nibble_swapping_32_bit(macl);

	addrhswapped = flip_4_bits(addrh & 0xf)
		+ ((flip_4_bits((addrh >> 4) & 0xf)) << 4)
		+ ((flip_4_bits((addrh >> 8) & 0xf)) << 8)
		+ ((flip_4_bits((addrh >> 12) & 0xf)) << 12);

	addrlswapped = flip_4_bits(addrl & 0xf)
		+ ((flip_4_bits((addrl >> 4) & 0xf)) << 4)
		+ ((flip_4_bits((addrl >> 8) & 0xf)) << 8)
		+ ((flip_4_bits((addrl >> 12) & 0xf)) << 12)
		+ ((flip_4_bits((addrl >> 16) & 0xf)) << 16)
		+ ((flip_4_bits((addrl >> 20) & 0xf)) << 20)
		+ ((flip_4_bits((addrl >> 24) & 0xf)) << 24)
		+ ((flip_4_bits((addrl >> 28) & 0xf)) << 28);

	addrh = addrhswapped;
	addrl = addrlswapped;

	addr0 = (addrl >> 2) & 0x03f;
	addr1 = (addrl & 0x003) | (((addrl >> 8) & 0x7f) << 2);
	addr2 = (addrl >> 15) & 0x1ff;
	addr3 = ((addrl >> 24) & 0x0ff) | ((addrh & 1) << 8);

	hashresult = (addr0 << 9) | (addr1 ^ addr2 ^ addr3);
	hashresult = hashresult & 0x07ff;
	return hashresult;
}

/*
 * This function will add an entry to the address table.
 * depends on the hash mode and hash size that was initialized.
 * Inputs
 * mach - the 2 most significant bytes of the MAC address.
 * macl - the 4 least significant bytes of the MAC address.
 * skip - if 1, skip this address.
 * rd   - the RD field in the address table.
 * Outputs
 * address table entry is added.
 * 0 if success.
 * -ENOSPC if table full
 */
static int add_del_hash_entry(struct armdfec_device *darmdfec, u32 mach,
			      u32 macl, u32 rd, u32 skip, int del)
{
	struct addr_table_entry_t *entry, *start;
	u32 newhi;
	u32 newlo;
	u32 i;

	newlo = (((mach >> 4) & 0xf) << 15)
		| (((mach >> 0) & 0xf) << 11)
		| (((mach >> 12) & 0xf) << 7)
		| (((mach >> 8) & 0xf) << 3)
		| (((macl >> 20) & 0x1) << 31)
		| (((macl >> 16) & 0xf) << 27)
		| (((macl >> 28) & 0xf) << 23)
		| (((macl >> 24) & 0xf) << 19)
		| (skip << HTESKIP) | (rd << HTERDBIT)
		| HTEVALID;

	newhi = (((macl >> 4) & 0xf) << 15)
		| (((macl >> 0) & 0xf) << 11)
		| (((macl >> 12) & 0xf) << 7)
		| (((macl >> 8) & 0xf) << 3)
		| (((macl >> 21) & 0x7) << 0);

	/*
	 * Pick the appropriate table, start scanning for free/reusable
	 * entries at the index obtained by hashing the specified MAC address
	 */
	start = (struct addr_table_entry_t *)(darmdfec->htpr);
	entry = start + hash_function(mach, macl);
	for (i = 0; i < HOP_NUMBER; i++) {
		if (!(entry->lo & HTEVALID)) {
			break;
		} else {
			/* if same address put in same position */
			if (((entry->lo & 0xfffffff8) == (newlo & 0xfffffff8))
					&& (entry->hi == newhi))
				break;
		}
		if (entry == start + 0x7ff)
			entry = start;
		else
			entry++;
	}

	if (((entry->lo & 0xfffffff8) != (newlo & 0xfffffff8)) &&
		(entry->hi != newhi) && del)
		return 0;

	if (i == HOP_NUMBER) {
		if (!del) {
			printf("ARMD100 FEC: (%s) table section is full\n",
					__func__);
			return -ENOSPC;
		} else {
			return 0;
		}
	}

	/*
	 * Update the selected entry
	 */
	if (del) {
		entry->hi = 0;
		entry->lo = 0;
	} else {
		entry->hi = newhi;
		entry->lo = newlo;
	}

	return 0;
}

/*
 *  Create an addressTable entry from MAC address info
 *  found in the specifed net_device struct
 *
 *  Input : pointer to ethernet interface network device structure
 *  Output : N/A
 */
static void update_hash_table_mac_address(struct armdfec_device *darmdfec,
					  u8 *oaddr, u8 *addr)
{
	u32 mach;
	u32 macl;

	/* Delete old entry */
	if (oaddr) {
		mach = (oaddr[0] << 8) | oaddr[1];
		macl = (oaddr[2] << 24) | (oaddr[3] << 16) |
			(oaddr[4] << 8) | oaddr[5];
		add_del_hash_entry(darmdfec, mach, macl, 1, 0, HASH_DELETE);
	}

	/* Add new entry */
	mach = (addr[0] << 8) | addr[1];
	macl = (addr[2] << 24) | (addr[3] << 16) | (addr[4] << 8) | addr[5];
	add_del_hash_entry(darmdfec, mach, macl, 1, 0, HASH_ADD);
}

/* Address Table Initialization */
static void init_hashtable(struct eth_device *dev)
{
	struct armdfec_device *darmdfec = to_darmdfec(dev);
	struct armdfec_reg *regs = darmdfec->regs;
	memset(darmdfec->htpr, 0, HASH_ADDR_TABLE_SIZE);
	writel((u32)darmdfec->htpr, &regs->htpr);
}

/*
 * This detects PHY chip from address 0-31 by reading PHY status
 * registers. PHY chip can be connected at any of this address.
 */
static int ethernet_phy_detect(struct eth_device *dev)
{
	u32 val;
	u16 tmp, mii_status;
	u8 addr;

	for (addr = 0; addr < 32; addr++) {
		if (miiphy_read(dev->name, addr, MII_BMSR, &mii_status)	!= 0)
			/* try next phy */
			continue;

		/* invalid MII status. More validation required here... */
		if (mii_status == 0 || mii_status == 0xffff)
			/* try next phy */
			continue;

		if (miiphy_read(dev->name, addr, MII_PHYSID1, &tmp) != 0)
			/* try next phy */
			continue;

		val = tmp << 16;
		if (miiphy_read(dev->name, addr, MII_PHYSID2, &tmp) != 0)
			/* try next phy */
			continue;

		val |= tmp;

		if ((val & 0xfffffff0) != 0)
			return addr;
	}
	return -1;
}

static void armdfec_init_rx_desc_ring(struct armdfec_device *darmdfec)
{
	struct rx_desc *p_rx_desc;
	int i;

	/* initialize the Rx descriptors ring */
	p_rx_desc = darmdfec->p_rxdesc;
	for (i = 0; i < RINGSZ; i++) {
		p_rx_desc->cmd_sts = BUF_OWNED_BY_DMA | RX_EN_INT;
		p_rx_desc->buf_size = PKTSIZE_ALIGN;
		p_rx_desc->byte_cnt = 0;
		p_rx_desc->buf_ptr = darmdfec->p_rxbuf + i * PKTSIZE_ALIGN;
		if (i == (RINGSZ - 1)) {
			p_rx_desc->nxtdesc_p = darmdfec->p_rxdesc;
		} else {
			p_rx_desc->nxtdesc_p = (struct rx_desc *)
			    ((u32)p_rx_desc + ARMDFEC_RXQ_DESC_ALIGNED_SIZE);
			p_rx_desc = p_rx_desc->nxtdesc_p;
		}
	}
	darmdfec->p_rxdesc_curr = darmdfec->p_rxdesc;
}

static int armdfec_init(struct eth_device *dev, bd_t *bd)
{
	struct armdfec_device *darmdfec = to_darmdfec(dev);
	struct armdfec_reg *regs = darmdfec->regs;
	int phy_adr;
	u32 temp;

	armdfec_init_rx_desc_ring(darmdfec);

	/* Disable interrupts */
	writel(0, &regs->im);
	writel(0, &regs->ic);
	/* Write to ICR to clear interrupts. */
	writel(0, &regs->iwc);

	/*
	 * Abort any transmit and receive operations and put DMA
	 * in idle state.
	 */
	abortdma(dev);

	/* Initialize address hash table */
	init_hashtable(dev);

	/* SDMA configuration */
	writel(SDCR_BSZ8 |	/* Burst size = 32 bytes */
		SDCR_RIFB |	/* Rx interrupt on frame */
		SDCR_BLMT |	/* Little endian transmit */
		SDCR_BLMR |	/* Little endian receive */
		SDCR_RC_MAX_RETRANS,	/* Max retransmit count */
		&regs->sdma_conf);
	/* Port Configuration */
	writel(PCR_HS, &regs->pconf);	/* Hash size is 1/2kb */

	/* Set extended port configuration */
	writel(PCXR_2BSM |		/* Two byte suffix aligns IP hdr */
		PCXR_DSCP_EN |		/* Enable DSCP in IP */
		PCXR_MFL_1536 |		/* Set MTU = 1536 */
		PCXR_FLP |		/* do not force link pass */
		PCXR_TX_HIGH_PRI,	/* Transmit - high priority queue */
		&regs->pconf_ext);

	update_hash_table_mac_address(darmdfec, NULL, dev->enetaddr);

	/* Update TX and RX queue descriptor register */
	temp = (u32)&regs->txcdp[TXQ];
	writel((u32)darmdfec->p_txdesc, temp);
	temp = (u32)&regs->rxfdp[RXQ];
	writel((u32)darmdfec->p_rxdesc, temp);
	temp = (u32)&regs->rxcdp[RXQ];
	writel((u32)darmdfec->p_rxdesc_curr, temp);

	/* Enable Interrupts */
	writel(ALL_INTS, &regs->im);

	/* Enable Ethernet Port */
	setbits_le32(&regs->pconf, PCR_EN);

	/* Enable RX DMA engine */
	setbits_le32(&regs->sdma_cmd, SDMA_CMD_ERD);

#ifdef DEBUG
	eth_dump_regs(dev);
#endif

#if (defined(CONFIG_MII) || defined(CONFIG_CMD_MII))

#if defined(CONFIG_PHY_BASE_ADR)
	miiphy_write(dev->name, PHY_ADR_REQ, PHY_ADR_REQ, CONFIG_PHY_BASE_ADR);
#else
	/* Search phy address from range 0-31 */
	phy_adr = ethernet_phy_detect(dev);
	if (phy_adr < 0) {
		printf("ARMD100 FEC: PHY not detected at address range 0-31\n");
		return -1;
	} else {
		debug("ARMD100 FEC: PHY detected at addr %d\n", phy_adr);
		miiphy_write(dev->name, PHY_ADR_REQ, PHY_ADR_REQ, phy_adr);
	}
#endif

#if defined(CONFIG_SYS_FAULT_ECHO_LINK_DOWN)
	/* Wait up to 5s for the link status */
	for (i = 0; i < 5; i++) {
		u16 phy_adr;

		miiphy_read(dev->name, 0xFF, 0xFF, &phy_adr);
		/* Return if we get link up */
		if (miiphy_link(dev->name, phy_adr))
			return 0;
		udelay(1000000);
	}

	printf("ARMD100 FEC: No link on %s\n", dev->name);
	return -1;
#endif
#endif
	return 0;
}

static void armdfec_halt(struct eth_device *dev)
{
	struct armdfec_device *darmdfec = to_darmdfec(dev);
	struct armdfec_reg *regs = darmdfec->regs;

	/* Stop RX DMA */
	clrbits_le32(&regs->sdma_cmd, SDMA_CMD_ERD);

	/*
	 * Abort any transmit and receive operations and put DMA
	 * in idle state.
	 */
	abortdma(dev);

	/* Disable interrupts */
	writel(0, &regs->im);
	writel(0, &regs->ic);
	writel(0, &regs->iwc);

	/* Disable Port */
	clrbits_le32(&regs->pconf, PCR_EN);
}

static int armdfec_send(struct eth_device *dev, void *dataptr, int datasize)
{
	struct armdfec_device *darmdfec = to_darmdfec(dev);
	struct armdfec_reg *regs = darmdfec->regs;
	struct tx_desc *p_txdesc = darmdfec->p_txdesc;
	void *p = (void *)dataptr;
	int retry = PHY_WAIT_ITERATIONS * PHY_WAIT_MICRO_SECONDS;
	u32 cmd_sts, temp;

	/* Copy buffer if it's misaligned */
	if ((u32)dataptr & 0x07) {
		if (datasize > PKTSIZE_ALIGN) {
			printf("ARMD100 FEC: Non-aligned data too large (%d)\n",
					datasize);
			return -1;
		}
		memcpy(darmdfec->p_aligned_txbuf, p, datasize);
		p = darmdfec->p_aligned_txbuf;
	}

	p_txdesc->cmd_sts = TX_ZERO_PADDING | TX_GEN_CRC;
	p_txdesc->cmd_sts |= TX_FIRST_DESC | TX_LAST_DESC;
	p_txdesc->cmd_sts |= BUF_OWNED_BY_DMA;
	p_txdesc->cmd_sts |= TX_EN_INT;
	p_txdesc->buf_ptr = p;
	p_txdesc->byte_cnt = datasize;

	/* Apply send command using high priority TX queue */
	temp = (u32)&regs->txcdp[TXQ];
	writel((u32)p_txdesc, temp);
	writel(SDMA_CMD_TXDL | SDMA_CMD_TXDH | SDMA_CMD_ERD, &regs->sdma_cmd);

	/*
	 * wait for packet xmit completion
	 */
	cmd_sts = readl(&p_txdesc->cmd_sts);
	while (cmd_sts & BUF_OWNED_BY_DMA) {
		/* return fail if error is detected */
		if ((cmd_sts & (TX_ERROR | TX_LAST_DESC)) ==
			(TX_ERROR | TX_LAST_DESC)) {
			printf("ARMD100 FEC: (%s) in xmit packet\n", __func__);
			return -1;
		}
		cmd_sts = readl(&p_txdesc->cmd_sts);
		if (!(retry--)) {
			printf("ARMD100 FEC: (%s) xmit packet timeout!\n",
					__func__);
			return -1;
		}
	}

	return 0;
}

static int armdfec_recv(struct eth_device *dev)
{
	struct armdfec_device *darmdfec = to_darmdfec(dev);
	struct rx_desc *p_rxdesc_curr = darmdfec->p_rxdesc_curr;
	u32 cmd_sts;
	u32 timeout = 0;
	u32 temp;

	/* wait untill rx packet available or timeout */
	do {
		if (timeout < PHY_WAIT_ITERATIONS * PHY_WAIT_MICRO_SECONDS) {
			timeout++;
		} else {
			debug("ARMD100 FEC: %s time out...\n", __func__);
			return -1;
		}
	} while (readl(&p_rxdesc_curr->cmd_sts) & BUF_OWNED_BY_DMA);

	if (p_rxdesc_curr->byte_cnt != 0) {
		debug("ARMD100 FEC: %s: Received %d byte Packet @ 0x%x"
				"(cmd_sts= %08x)\n", __func__,
				(u32)p_rxdesc_curr->byte_cnt,
				(u32)p_rxdesc_curr->buf_ptr,
				(u32)p_rxdesc_curr->cmd_sts);
	}

	/*
	 * In case received a packet without first/last bits on
	 * OR the error summary bit is on,
	 * the packets needs to be dropeed.
	 */
	cmd_sts = readl(&p_rxdesc_curr->cmd_sts);

	if ((cmd_sts & (RX_FIRST_DESC | RX_LAST_DESC)) !=
			(RX_FIRST_DESC | RX_LAST_DESC)) {
		printf("ARMD100 FEC: (%s) Dropping packet spread on"
			" multiple descriptors\n", __func__);
	} else if (cmd_sts & RX_ERROR) {
		printf("ARMD100 FEC: (%s) Dropping packet with errors\n",
				__func__);
	} else {
		/* !!! call higher layer processing */
		debug("ARMD100 FEC: (%s) Sending Received packet to"
		      " upper layer (net_process_received_packet)\n", __func__);

		/*
		 * let the upper layer handle the packet, subtract offset
		 * as two dummy bytes are added in received buffer see
		 * PORT_CONFIG_EXT register bit TWO_Byte_Stuff_Mode bit.
		 */
		net_process_received_packet(
			p_rxdesc_curr->buf_ptr + RX_BUF_OFFSET,
			(int)(p_rxdesc_curr->byte_cnt - RX_BUF_OFFSET));
	}
	/*
	 * free these descriptors and point next in the ring
	 */
	p_rxdesc_curr->cmd_sts = BUF_OWNED_BY_DMA | RX_EN_INT;
	p_rxdesc_curr->buf_size = PKTSIZE_ALIGN;
	p_rxdesc_curr->byte_cnt = 0;

	temp = (u32)&darmdfec->p_rxdesc_curr;
	writel((u32)p_rxdesc_curr->nxtdesc_p, temp);

	return 0;
}

int armada100_fec_register(unsigned long base_addr)
{
	struct armdfec_device *darmdfec;
	struct eth_device *dev;

	darmdfec = malloc(sizeof(struct armdfec_device));
	if (!darmdfec)
		goto error;

	memset(darmdfec, 0, sizeof(struct armdfec_device));

	darmdfec->htpr = memalign(8, HASH_ADDR_TABLE_SIZE);
	if (!darmdfec->htpr)
		goto error1;

	darmdfec->p_rxdesc = memalign(PKTALIGN,
			ARMDFEC_RXQ_DESC_ALIGNED_SIZE * RINGSZ + 1);

	if (!darmdfec->p_rxdesc)
		goto error1;

	darmdfec->p_rxbuf = memalign(PKTALIGN, RINGSZ * PKTSIZE_ALIGN + 1);
	if (!darmdfec->p_rxbuf)
		goto error1;

	darmdfec->p_aligned_txbuf = memalign(8, PKTSIZE_ALIGN);
	if (!darmdfec->p_aligned_txbuf)
		goto error1;

	darmdfec->p_txdesc = memalign(PKTALIGN, sizeof(struct tx_desc) + 1);
	if (!darmdfec->p_txdesc)
		goto error1;

	dev = &darmdfec->dev;
	/* Assign ARMADA100 Fast Ethernet Controller Base Address */
	darmdfec->regs = (void *)base_addr;

	/* must be less than sizeof(dev->name) */
	strcpy(dev->name, "armd-fec0");

	dev->init = armdfec_init;
	dev->halt = armdfec_halt;
	dev->send = armdfec_send;
	dev->recv = armdfec_recv;

	eth_register(dev);

#if defined(CONFIG_MII) || defined(CONFIG_CMD_MII)
	int retval;
	struct mii_dev *mdiodev = mdio_alloc();
	if (!mdiodev)
		return -ENOMEM;
	strncpy(mdiodev->name, dev->name, MDIO_NAME_LEN);
	mdiodev->read = smi_reg_read;
	mdiodev->write = smi_reg_write;

	retval = mdio_register(mdiodev);
	if (retval < 0)
		return retval;
#endif
	return 0;

error1:
	free(darmdfec->p_aligned_txbuf);
	free(darmdfec->p_rxbuf);
	free(darmdfec->p_rxdesc);
	free(darmdfec->htpr);
error:
	free(darmdfec);
	printf("AMD100 FEC: (%s) Failed to allocate memory\n", __func__);
	return -1;
}
