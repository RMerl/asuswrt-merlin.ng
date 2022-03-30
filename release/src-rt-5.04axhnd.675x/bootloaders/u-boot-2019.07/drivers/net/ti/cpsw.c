// SPDX-License-Identifier: GPL-2.0+
/*
 * CPSW Ethernet Switch Driver
 *
 * Copyright (C) 2010-2018 Texas Instruments Incorporated - http://www.ti.com/
 */

#include <common.h>
#include <command.h>
#include <net.h>
#include <miiphy.h>
#include <malloc.h>
#include <net.h>
#include <netdev.h>
#include <cpsw.h>
#include <linux/errno.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <phy.h>
#include <asm/arch/cpu.h>
#include <dm.h>
#include <fdt_support.h>

#include "cpsw_mdio.h"

DECLARE_GLOBAL_DATA_PTR;

#define BITMASK(bits)		(BIT(bits) - 1)
#define NUM_DESCS		(PKTBUFSRX * 2)
#define PKT_MIN			60
#define PKT_MAX			(1500 + 14 + 4 + 4)
#define CLEAR_BIT		1
#define GIGABITEN		BIT(7)
#define FULLDUPLEXEN		BIT(0)
#define MIIEN			BIT(15)
/* DMA Registers */
#define CPDMA_TXCONTROL		0x004
#define CPDMA_RXCONTROL		0x014
#define CPDMA_SOFTRESET		0x01c
#define CPDMA_RXFREE		0x0e0
#define CPDMA_TXHDP_VER1	0x100
#define CPDMA_TXHDP_VER2	0x200
#define CPDMA_RXHDP_VER1	0x120
#define CPDMA_RXHDP_VER2	0x220
#define CPDMA_TXCP_VER1		0x140
#define CPDMA_TXCP_VER2		0x240
#define CPDMA_RXCP_VER1		0x160
#define CPDMA_RXCP_VER2		0x260

/* Descriptor mode bits */
#define CPDMA_DESC_SOP		BIT(31)
#define CPDMA_DESC_EOP		BIT(30)
#define CPDMA_DESC_OWNER	BIT(29)
#define CPDMA_DESC_EOQ		BIT(28)

/*
 * This timeout definition is a worst-case ultra defensive measure against
 * unexpected controller lock ups.  Ideally, we should never ever hit this
 * scenario in practice.
 */
#define CPDMA_TIMEOUT		100 /* msecs */

struct cpsw_regs {
	u32	id_ver;
	u32	control;
	u32	soft_reset;
	u32	stat_port_en;
	u32	ptype;
};

struct cpsw_slave_regs {
	u32	max_blks;
	u32	blk_cnt;
	u32	flow_thresh;
	u32	port_vlan;
	u32	tx_pri_map;
#ifdef CONFIG_AM33XX
	u32	gap_thresh;
#elif defined(CONFIG_TI814X)
	u32	ts_ctl;
	u32	ts_seq_ltype;
	u32	ts_vlan;
#endif
	u32	sa_lo;
	u32	sa_hi;
};

struct cpsw_host_regs {
	u32	max_blks;
	u32	blk_cnt;
	u32	flow_thresh;
	u32	port_vlan;
	u32	tx_pri_map;
	u32	cpdma_tx_pri_map;
	u32	cpdma_rx_chan_map;
};

struct cpsw_sliver_regs {
	u32	id_ver;
	u32	mac_control;
	u32	mac_status;
	u32	soft_reset;
	u32	rx_maxlen;
	u32	__reserved_0;
	u32	rx_pause;
	u32	tx_pause;
	u32	__reserved_1;
	u32	rx_pri_map;
};

#define ALE_ENTRY_BITS		68
#define ALE_ENTRY_WORDS		DIV_ROUND_UP(ALE_ENTRY_BITS, 32)

/* ALE Registers */
#define ALE_CONTROL		0x08
#define ALE_UNKNOWNVLAN		0x18
#define ALE_TABLE_CONTROL	0x20
#define ALE_TABLE		0x34
#define ALE_PORTCTL		0x40

#define ALE_TABLE_WRITE		BIT(31)

#define ALE_TYPE_FREE			0
#define ALE_TYPE_ADDR			1
#define ALE_TYPE_VLAN			2
#define ALE_TYPE_VLAN_ADDR		3

#define ALE_UCAST_PERSISTANT		0
#define ALE_UCAST_UNTOUCHED		1
#define ALE_UCAST_OUI			2
#define ALE_UCAST_TOUCHED		3

#define ALE_MCAST_FWD			0
#define ALE_MCAST_BLOCK_LEARN_FWD	1
#define ALE_MCAST_FWD_LEARN		2
#define ALE_MCAST_FWD_2			3

enum cpsw_ale_port_state {
	ALE_PORT_STATE_DISABLE	= 0x00,
	ALE_PORT_STATE_BLOCK	= 0x01,
	ALE_PORT_STATE_LEARN	= 0x02,
	ALE_PORT_STATE_FORWARD	= 0x03,
};

/* ALE unicast entry flags - passed into cpsw_ale_add_ucast() */
#define ALE_SECURE	1
#define ALE_BLOCKED	2

struct cpsw_slave {
	struct cpsw_slave_regs		*regs;
	struct cpsw_sliver_regs		*sliver;
	int				slave_num;
	u32				mac_control;
	struct cpsw_slave_data		*data;
};

struct cpdma_desc {
	/* hardware fields */
	u32			hw_next;
	u32			hw_buffer;
	u32			hw_len;
	u32			hw_mode;
	/* software fields */
	u32			sw_buffer;
	u32			sw_len;
};

struct cpdma_chan {
	struct cpdma_desc	*head, *tail;
	void			*hdp, *cp, *rxfree;
};

/* AM33xx SoC specific definitions for the CONTROL port */
#define AM33XX_GMII_SEL_MODE_MII	0
#define AM33XX_GMII_SEL_MODE_RMII	1
#define AM33XX_GMII_SEL_MODE_RGMII	2

#define AM33XX_GMII_SEL_RGMII1_IDMODE	BIT(4)
#define AM33XX_GMII_SEL_RGMII2_IDMODE	BIT(5)
#define AM33XX_GMII_SEL_RMII1_IO_CLK_EN	BIT(6)
#define AM33XX_GMII_SEL_RMII2_IO_CLK_EN	BIT(7)

#define GMII_SEL_MODE_MASK		0x3

#define desc_write(desc, fld, val)	__raw_writel((u32)(val), &(desc)->fld)
#define desc_read(desc, fld)		__raw_readl(&(desc)->fld)
#define desc_read_ptr(desc, fld)	((void *)__raw_readl(&(desc)->fld))

#define chan_write(chan, fld, val)	__raw_writel((u32)(val), (chan)->fld)
#define chan_read(chan, fld)		__raw_readl((chan)->fld)
#define chan_read_ptr(chan, fld)	((void *)__raw_readl((chan)->fld))

#define for_active_slave(slave, priv) \
	slave = (priv)->slaves + ((priv)->data)->active_slave; if (slave)
#define for_each_slave(slave, priv) \
	for (slave = (priv)->slaves; slave != (priv)->slaves + \
				((priv)->data)->slaves; slave++)

struct cpsw_priv {
#ifdef CONFIG_DM_ETH
	struct udevice			*dev;
#else
	struct eth_device		*dev;
#endif
	struct cpsw_platform_data	*data;
	int				host_port;

	struct cpsw_regs		*regs;
	void				*dma_regs;
	struct cpsw_host_regs		*host_port_regs;
	void				*ale_regs;

	struct cpdma_desc		*descs;
	struct cpdma_desc		*desc_free;
	struct cpdma_chan		rx_chan, tx_chan;

	struct cpsw_slave		*slaves;
	struct phy_device		*phydev;
	struct mii_dev			*bus;

	u32				phy_mask;
};

static inline int cpsw_ale_get_field(u32 *ale_entry, u32 start, u32 bits)
{
	int idx;

	idx    = start / 32;
	start -= idx * 32;
	idx    = 2 - idx; /* flip */
	return (ale_entry[idx] >> start) & BITMASK(bits);
}

static inline void cpsw_ale_set_field(u32 *ale_entry, u32 start, u32 bits,
				      u32 value)
{
	int idx;

	value &= BITMASK(bits);
	idx    = start / 32;
	start -= idx * 32;
	idx    = 2 - idx; /* flip */
	ale_entry[idx] &= ~(BITMASK(bits) << start);
	ale_entry[idx] |=  (value << start);
}

#define DEFINE_ALE_FIELD(name, start, bits)				\
static inline int cpsw_ale_get_##name(u32 *ale_entry)			\
{									\
	return cpsw_ale_get_field(ale_entry, start, bits);		\
}									\
static inline void cpsw_ale_set_##name(u32 *ale_entry, u32 value)	\
{									\
	cpsw_ale_set_field(ale_entry, start, bits, value);		\
}

DEFINE_ALE_FIELD(entry_type,		60,	2)
DEFINE_ALE_FIELD(mcast_state,		62,	2)
DEFINE_ALE_FIELD(port_mask,		66,	3)
DEFINE_ALE_FIELD(ucast_type,		62,	2)
DEFINE_ALE_FIELD(port_num,		66,	2)
DEFINE_ALE_FIELD(blocked,		65,	1)
DEFINE_ALE_FIELD(secure,		64,	1)
DEFINE_ALE_FIELD(mcast,			40,	1)

/* The MAC address field in the ALE entry cannot be macroized as above */
static inline void cpsw_ale_get_addr(u32 *ale_entry, u8 *addr)
{
	int i;

	for (i = 0; i < 6; i++)
		addr[i] = cpsw_ale_get_field(ale_entry, 40 - 8*i, 8);
}

static inline void cpsw_ale_set_addr(u32 *ale_entry, const u8 *addr)
{
	int i;

	for (i = 0; i < 6; i++)
		cpsw_ale_set_field(ale_entry, 40 - 8*i, 8, addr[i]);
}

static int cpsw_ale_read(struct cpsw_priv *priv, int idx, u32 *ale_entry)
{
	int i;

	__raw_writel(idx, priv->ale_regs + ALE_TABLE_CONTROL);

	for (i = 0; i < ALE_ENTRY_WORDS; i++)
		ale_entry[i] = __raw_readl(priv->ale_regs + ALE_TABLE + 4 * i);

	return idx;
}

static int cpsw_ale_write(struct cpsw_priv *priv, int idx, u32 *ale_entry)
{
	int i;

	for (i = 0; i < ALE_ENTRY_WORDS; i++)
		__raw_writel(ale_entry[i], priv->ale_regs + ALE_TABLE + 4 * i);

	__raw_writel(idx | ALE_TABLE_WRITE, priv->ale_regs + ALE_TABLE_CONTROL);

	return idx;
}

static int cpsw_ale_match_addr(struct cpsw_priv *priv, const u8 *addr)
{
	u32 ale_entry[ALE_ENTRY_WORDS];
	int type, idx;

	for (idx = 0; idx < priv->data->ale_entries; idx++) {
		u8 entry_addr[6];

		cpsw_ale_read(priv, idx, ale_entry);
		type = cpsw_ale_get_entry_type(ale_entry);
		if (type != ALE_TYPE_ADDR && type != ALE_TYPE_VLAN_ADDR)
			continue;
		cpsw_ale_get_addr(ale_entry, entry_addr);
		if (memcmp(entry_addr, addr, 6) == 0)
			return idx;
	}
	return -ENOENT;
}

static int cpsw_ale_match_free(struct cpsw_priv *priv)
{
	u32 ale_entry[ALE_ENTRY_WORDS];
	int type, idx;

	for (idx = 0; idx < priv->data->ale_entries; idx++) {
		cpsw_ale_read(priv, idx, ale_entry);
		type = cpsw_ale_get_entry_type(ale_entry);
		if (type == ALE_TYPE_FREE)
			return idx;
	}
	return -ENOENT;
}

static int cpsw_ale_find_ageable(struct cpsw_priv *priv)
{
	u32 ale_entry[ALE_ENTRY_WORDS];
	int type, idx;

	for (idx = 0; idx < priv->data->ale_entries; idx++) {
		cpsw_ale_read(priv, idx, ale_entry);
		type = cpsw_ale_get_entry_type(ale_entry);
		if (type != ALE_TYPE_ADDR && type != ALE_TYPE_VLAN_ADDR)
			continue;
		if (cpsw_ale_get_mcast(ale_entry))
			continue;
		type = cpsw_ale_get_ucast_type(ale_entry);
		if (type != ALE_UCAST_PERSISTANT &&
		    type != ALE_UCAST_OUI)
			return idx;
	}
	return -ENOENT;
}

static int cpsw_ale_add_ucast(struct cpsw_priv *priv, const u8 *addr,
			      int port, int flags)
{
	u32 ale_entry[ALE_ENTRY_WORDS] = {0, 0, 0};
	int idx;

	cpsw_ale_set_entry_type(ale_entry, ALE_TYPE_ADDR);
	cpsw_ale_set_addr(ale_entry, addr);
	cpsw_ale_set_ucast_type(ale_entry, ALE_UCAST_PERSISTANT);
	cpsw_ale_set_secure(ale_entry, (flags & ALE_SECURE) ? 1 : 0);
	cpsw_ale_set_blocked(ale_entry, (flags & ALE_BLOCKED) ? 1 : 0);
	cpsw_ale_set_port_num(ale_entry, port);

	idx = cpsw_ale_match_addr(priv, addr);
	if (idx < 0)
		idx = cpsw_ale_match_free(priv);
	if (idx < 0)
		idx = cpsw_ale_find_ageable(priv);
	if (idx < 0)
		return -ENOMEM;

	cpsw_ale_write(priv, idx, ale_entry);
	return 0;
}

static int cpsw_ale_add_mcast(struct cpsw_priv *priv, const u8 *addr,
			      int port_mask)
{
	u32 ale_entry[ALE_ENTRY_WORDS] = {0, 0, 0};
	int idx, mask;

	idx = cpsw_ale_match_addr(priv, addr);
	if (idx >= 0)
		cpsw_ale_read(priv, idx, ale_entry);

	cpsw_ale_set_entry_type(ale_entry, ALE_TYPE_ADDR);
	cpsw_ale_set_addr(ale_entry, addr);
	cpsw_ale_set_mcast_state(ale_entry, ALE_MCAST_FWD_2);

	mask = cpsw_ale_get_port_mask(ale_entry);
	port_mask |= mask;
	cpsw_ale_set_port_mask(ale_entry, port_mask);

	if (idx < 0)
		idx = cpsw_ale_match_free(priv);
	if (idx < 0)
		idx = cpsw_ale_find_ageable(priv);
	if (idx < 0)
		return -ENOMEM;

	cpsw_ale_write(priv, idx, ale_entry);
	return 0;
}

static inline void cpsw_ale_control(struct cpsw_priv *priv, int bit, int val)
{
	u32 tmp, mask = BIT(bit);

	tmp  = __raw_readl(priv->ale_regs + ALE_CONTROL);
	tmp &= ~mask;
	tmp |= val ? mask : 0;
	__raw_writel(tmp, priv->ale_regs + ALE_CONTROL);
}

#define cpsw_ale_enable(priv, val)	cpsw_ale_control(priv, 31, val)
#define cpsw_ale_clear(priv, val)	cpsw_ale_control(priv, 30, val)
#define cpsw_ale_vlan_aware(priv, val)	cpsw_ale_control(priv,  2, val)

static inline void cpsw_ale_port_state(struct cpsw_priv *priv, int port,
				       int val)
{
	int offset = ALE_PORTCTL + 4 * port;
	u32 tmp, mask = 0x3;

	tmp  = __raw_readl(priv->ale_regs + offset);
	tmp &= ~mask;
	tmp |= val & mask;
	__raw_writel(tmp, priv->ale_regs + offset);
}

/* Set a self-clearing bit in a register, and wait for it to clear */
static inline void setbit_and_wait_for_clear32(void *addr)
{
	__raw_writel(CLEAR_BIT, addr);
	while (__raw_readl(addr) & CLEAR_BIT)
		;
}

#define mac_hi(mac)	(((mac)[0] << 0) | ((mac)[1] << 8) |	\
			 ((mac)[2] << 16) | ((mac)[3] << 24))
#define mac_lo(mac)	(((mac)[4] << 0) | ((mac)[5] << 8))

static void cpsw_set_slave_mac(struct cpsw_slave *slave,
			       struct cpsw_priv *priv)
{
#ifdef CONFIG_DM_ETH
	struct eth_pdata *pdata = dev_get_platdata(priv->dev);

	writel(mac_hi(pdata->enetaddr), &slave->regs->sa_hi);
	writel(mac_lo(pdata->enetaddr), &slave->regs->sa_lo);
#else
	__raw_writel(mac_hi(priv->dev->enetaddr), &slave->regs->sa_hi);
	__raw_writel(mac_lo(priv->dev->enetaddr), &slave->regs->sa_lo);
#endif
}

static int cpsw_slave_update_link(struct cpsw_slave *slave,
				   struct cpsw_priv *priv, int *link)
{
	struct phy_device *phy;
	u32 mac_control = 0;
	int ret = -ENODEV;

	phy = priv->phydev;
	if (!phy)
		goto out;

	ret = phy_startup(phy);
	if (ret)
		goto out;

	if (link)
		*link = phy->link;

	if (phy->link) { /* link up */
		mac_control = priv->data->mac_control;
		if (phy->speed == 1000)
			mac_control |= GIGABITEN;
		if (phy->duplex == DUPLEX_FULL)
			mac_control |= FULLDUPLEXEN;
		if (phy->speed == 100)
			mac_control |= MIIEN;
	}

	if (mac_control == slave->mac_control)
		goto out;

	if (mac_control) {
		printf("link up on port %d, speed %d, %s duplex\n",
				slave->slave_num, phy->speed,
				(phy->duplex == DUPLEX_FULL) ? "full" : "half");
	} else {
		printf("link down on port %d\n", slave->slave_num);
	}

	__raw_writel(mac_control, &slave->sliver->mac_control);
	slave->mac_control = mac_control;

out:
	return ret;
}

static int cpsw_update_link(struct cpsw_priv *priv)
{
	int ret = -ENODEV;
	struct cpsw_slave *slave;

	for_active_slave(slave, priv)
		ret = cpsw_slave_update_link(slave, priv, NULL);

	return ret;
}

static inline u32  cpsw_get_slave_port(struct cpsw_priv *priv, u32 slave_num)
{
	if (priv->host_port == 0)
		return slave_num + 1;
	else
		return slave_num;
}

static void cpsw_slave_init(struct cpsw_slave *slave, struct cpsw_priv *priv)
{
	u32     slave_port;

	setbit_and_wait_for_clear32(&slave->sliver->soft_reset);

	/* setup priority mapping */
	__raw_writel(0x76543210, &slave->sliver->rx_pri_map);
	__raw_writel(0x33221100, &slave->regs->tx_pri_map);

	/* setup max packet size, and mac address */
	__raw_writel(PKT_MAX, &slave->sliver->rx_maxlen);
	cpsw_set_slave_mac(slave, priv);

	slave->mac_control = 0;	/* no link yet */

	/* enable forwarding */
	slave_port = cpsw_get_slave_port(priv, slave->slave_num);
	cpsw_ale_port_state(priv, slave_port, ALE_PORT_STATE_FORWARD);

	cpsw_ale_add_mcast(priv, net_bcast_ethaddr, 1 << slave_port);

	priv->phy_mask |= 1 << slave->data->phy_addr;
}

static struct cpdma_desc *cpdma_desc_alloc(struct cpsw_priv *priv)
{
	struct cpdma_desc *desc = priv->desc_free;

	if (desc)
		priv->desc_free = desc_read_ptr(desc, hw_next);
	return desc;
}

static void cpdma_desc_free(struct cpsw_priv *priv, struct cpdma_desc *desc)
{
	if (desc) {
		desc_write(desc, hw_next, priv->desc_free);
		priv->desc_free = desc;
	}
}

static int cpdma_submit(struct cpsw_priv *priv, struct cpdma_chan *chan,
			void *buffer, int len)
{
	struct cpdma_desc *desc, *prev;
	u32 mode;

	desc = cpdma_desc_alloc(priv);
	if (!desc)
		return -ENOMEM;

	if (len < PKT_MIN)
		len = PKT_MIN;

	mode = CPDMA_DESC_OWNER | CPDMA_DESC_SOP | CPDMA_DESC_EOP;

	desc_write(desc, hw_next,   0);
	desc_write(desc, hw_buffer, buffer);
	desc_write(desc, hw_len,    len);
	desc_write(desc, hw_mode,   mode | len);
	desc_write(desc, sw_buffer, buffer);
	desc_write(desc, sw_len,    len);

	if (!chan->head) {
		/* simple case - first packet enqueued */
		chan->head = desc;
		chan->tail = desc;
		chan_write(chan, hdp, desc);
		goto done;
	}

	/* not the first packet - enqueue at the tail */
	prev = chan->tail;
	desc_write(prev, hw_next, desc);
	chan->tail = desc;

	/* next check if EOQ has been triggered already */
	if (desc_read(prev, hw_mode) & CPDMA_DESC_EOQ)
		chan_write(chan, hdp, desc);

done:
	if (chan->rxfree)
		chan_write(chan, rxfree, 1);
	return 0;
}

static int cpdma_process(struct cpsw_priv *priv, struct cpdma_chan *chan,
			 void **buffer, int *len)
{
	struct cpdma_desc *desc = chan->head;
	u32 status;

	if (!desc)
		return -ENOENT;

	status = desc_read(desc, hw_mode);

	if (len)
		*len = status & 0x7ff;

	if (buffer)
		*buffer = desc_read_ptr(desc, sw_buffer);

	if (status & CPDMA_DESC_OWNER) {
		if (chan_read(chan, hdp) == 0) {
			if (desc_read(desc, hw_mode) & CPDMA_DESC_OWNER)
				chan_write(chan, hdp, desc);
		}

		return -EBUSY;
	}

	chan->head = desc_read_ptr(desc, hw_next);
	chan_write(chan, cp, desc);

	cpdma_desc_free(priv, desc);
	return 0;
}

static int _cpsw_init(struct cpsw_priv *priv, u8 *enetaddr)
{
	struct cpsw_slave	*slave;
	int i, ret;

	/* soft reset the controller and initialize priv */
	setbit_and_wait_for_clear32(&priv->regs->soft_reset);

	/* initialize and reset the address lookup engine */
	cpsw_ale_enable(priv, 1);
	cpsw_ale_clear(priv, 1);
	cpsw_ale_vlan_aware(priv, 0); /* vlan unaware mode */

	/* setup host port priority mapping */
	__raw_writel(0x76543210, &priv->host_port_regs->cpdma_tx_pri_map);
	__raw_writel(0, &priv->host_port_regs->cpdma_rx_chan_map);

	/* disable priority elevation and enable statistics on all ports */
	__raw_writel(0, &priv->regs->ptype);

	/* enable statistics collection only on the host port */
	__raw_writel(BIT(priv->host_port), &priv->regs->stat_port_en);
	__raw_writel(0x7, &priv->regs->stat_port_en);

	cpsw_ale_port_state(priv, priv->host_port, ALE_PORT_STATE_FORWARD);

	cpsw_ale_add_ucast(priv, enetaddr, priv->host_port, ALE_SECURE);
	cpsw_ale_add_mcast(priv, net_bcast_ethaddr, 1 << priv->host_port);

	for_active_slave(slave, priv)
		cpsw_slave_init(slave, priv);

	ret = cpsw_update_link(priv);
	if (ret)
		goto out;

	/* init descriptor pool */
	for (i = 0; i < NUM_DESCS; i++) {
		desc_write(&priv->descs[i], hw_next,
			   (i == (NUM_DESCS - 1)) ? 0 : &priv->descs[i+1]);
	}
	priv->desc_free = &priv->descs[0];

	/* initialize channels */
	if (priv->data->version == CPSW_CTRL_VERSION_2) {
		memset(&priv->rx_chan, 0, sizeof(struct cpdma_chan));
		priv->rx_chan.hdp       = priv->dma_regs + CPDMA_RXHDP_VER2;
		priv->rx_chan.cp        = priv->dma_regs + CPDMA_RXCP_VER2;
		priv->rx_chan.rxfree    = priv->dma_regs + CPDMA_RXFREE;

		memset(&priv->tx_chan, 0, sizeof(struct cpdma_chan));
		priv->tx_chan.hdp       = priv->dma_regs + CPDMA_TXHDP_VER2;
		priv->tx_chan.cp        = priv->dma_regs + CPDMA_TXCP_VER2;
	} else {
		memset(&priv->rx_chan, 0, sizeof(struct cpdma_chan));
		priv->rx_chan.hdp       = priv->dma_regs + CPDMA_RXHDP_VER1;
		priv->rx_chan.cp        = priv->dma_regs + CPDMA_RXCP_VER1;
		priv->rx_chan.rxfree    = priv->dma_regs + CPDMA_RXFREE;

		memset(&priv->tx_chan, 0, sizeof(struct cpdma_chan));
		priv->tx_chan.hdp       = priv->dma_regs + CPDMA_TXHDP_VER1;
		priv->tx_chan.cp        = priv->dma_regs + CPDMA_TXCP_VER1;
	}

	/* clear dma state */
	setbit_and_wait_for_clear32(priv->dma_regs + CPDMA_SOFTRESET);

	if (priv->data->version == CPSW_CTRL_VERSION_2) {
		for (i = 0; i < priv->data->channels; i++) {
			__raw_writel(0, priv->dma_regs + CPDMA_RXHDP_VER2 + 4
					* i);
			__raw_writel(0, priv->dma_regs + CPDMA_RXFREE + 4
					* i);
			__raw_writel(0, priv->dma_regs + CPDMA_RXCP_VER2 + 4
					* i);
			__raw_writel(0, priv->dma_regs + CPDMA_TXHDP_VER2 + 4
					* i);
			__raw_writel(0, priv->dma_regs + CPDMA_TXCP_VER2 + 4
					* i);
		}
	} else {
		for (i = 0; i < priv->data->channels; i++) {
			__raw_writel(0, priv->dma_regs + CPDMA_RXHDP_VER1 + 4
					* i);
			__raw_writel(0, priv->dma_regs + CPDMA_RXFREE + 4
					* i);
			__raw_writel(0, priv->dma_regs + CPDMA_RXCP_VER1 + 4
					* i);
			__raw_writel(0, priv->dma_regs + CPDMA_TXHDP_VER1 + 4
					* i);
			__raw_writel(0, priv->dma_regs + CPDMA_TXCP_VER1 + 4
					* i);

		}
	}

	__raw_writel(1, priv->dma_regs + CPDMA_TXCONTROL);
	__raw_writel(1, priv->dma_regs + CPDMA_RXCONTROL);

	/* submit rx descs */
	for (i = 0; i < PKTBUFSRX; i++) {
		ret = cpdma_submit(priv, &priv->rx_chan, net_rx_packets[i],
				   PKTSIZE);
		if (ret < 0) {
			printf("error %d submitting rx desc\n", ret);
			break;
		}
	}

out:
	return ret;
}

static int cpsw_reap_completed_packets(struct cpsw_priv *priv)
{
	int timeout = CPDMA_TIMEOUT;

	/* reap completed packets */
	while (timeout-- &&
	       (cpdma_process(priv, &priv->tx_chan, NULL, NULL) >= 0))
		;

	return timeout;
}

static void _cpsw_halt(struct cpsw_priv *priv)
{
	cpsw_reap_completed_packets(priv);

	writel(0, priv->dma_regs + CPDMA_TXCONTROL);
	writel(0, priv->dma_regs + CPDMA_RXCONTROL);

	/* soft reset the controller and initialize priv */
	setbit_and_wait_for_clear32(&priv->regs->soft_reset);

	/* clear dma state */
	setbit_and_wait_for_clear32(priv->dma_regs + CPDMA_SOFTRESET);

}

static int _cpsw_send(struct cpsw_priv *priv, void *packet, int length)
{
	int timeout;

	flush_dcache_range((unsigned long)packet,
			   (unsigned long)packet + ALIGN(length, PKTALIGN));

	timeout = cpsw_reap_completed_packets(priv);
	if (timeout == -1) {
		printf("cpdma_process timeout\n");
		return -ETIMEDOUT;
	}

	return cpdma_submit(priv, &priv->tx_chan, packet, length);
}

static int _cpsw_recv(struct cpsw_priv *priv, uchar **pkt)
{
	void *buffer;
	int len;
	int ret;

	ret = cpdma_process(priv, &priv->rx_chan, &buffer, &len);
	if (ret < 0)
		return ret;

	invalidate_dcache_range((unsigned long)buffer,
				(unsigned long)buffer + PKTSIZE_ALIGN);
	*pkt = buffer;

	return len;
}

static void cpsw_slave_setup(struct cpsw_slave *slave, int slave_num,
			    struct cpsw_priv *priv)
{
	void			*regs = priv->regs;
	struct cpsw_slave_data	*data = priv->data->slave_data + slave_num;
	slave->slave_num = slave_num;
	slave->data	= data;
	slave->regs	= regs + data->slave_reg_ofs;
	slave->sliver	= regs + data->sliver_reg_ofs;
}

static int cpsw_phy_init(struct cpsw_priv *priv, struct cpsw_slave *slave)
{
	struct phy_device *phydev;
	u32 supported = PHY_GBIT_FEATURES;

	phydev = phy_connect(priv->bus,
			slave->data->phy_addr,
			priv->dev,
			slave->data->phy_if);

	if (!phydev)
		return -1;

	phydev->supported &= supported;
	phydev->advertising = phydev->supported;

#ifdef CONFIG_DM_ETH
	if (slave->data->phy_of_handle)
		phydev->node = offset_to_ofnode(slave->data->phy_of_handle);
#endif

	priv->phydev = phydev;
	phy_config(phydev);

	return 1;
}

static void cpsw_phy_addr_update(struct cpsw_priv *priv)
{
	struct cpsw_platform_data *data = priv->data;
	u16 alive = cpsw_mdio_get_alive(priv->bus);
	int active = data->active_slave;
	int new_addr = ffs(alive) - 1;

	/*
	 * If there is only one phy alive and its address does not match
	 * that of active slave, then phy address can safely be updated.
	 */
	if (hweight16(alive) == 1 &&
	    data->slave_data[active].phy_addr != new_addr) {
		printf("Updated phy address for CPSW#%d, old: %d, new: %d\n",
		       active, data->slave_data[active].phy_addr, new_addr);
		data->slave_data[active].phy_addr = new_addr;
	}
}

int _cpsw_register(struct cpsw_priv *priv)
{
	struct cpsw_slave	*slave;
	struct cpsw_platform_data *data = priv->data;
	void			*regs = (void *)data->cpsw_base;

	priv->slaves = malloc(sizeof(struct cpsw_slave) * data->slaves);
	if (!priv->slaves) {
		return -ENOMEM;
	}

	priv->host_port		= data->host_port_num;
	priv->regs		= regs;
	priv->host_port_regs	= regs + data->host_port_reg_ofs;
	priv->dma_regs		= regs + data->cpdma_reg_ofs;
	priv->ale_regs		= regs + data->ale_reg_ofs;
	priv->descs		= (void *)regs + data->bd_ram_ofs;

	int idx = 0;

	for_each_slave(slave, priv) {
		cpsw_slave_setup(slave, idx, priv);
		idx = idx + 1;
	}

	priv->bus = cpsw_mdio_init(priv->dev->name, data->mdio_base, 0, 0);
	if (!priv->bus)
		return -EFAULT;

	cpsw_phy_addr_update(priv);

	for_active_slave(slave, priv)
		cpsw_phy_init(priv, slave);

	return 0;
}

#ifndef CONFIG_DM_ETH
static int cpsw_init(struct eth_device *dev, bd_t *bis)
{
	struct cpsw_priv	*priv = dev->priv;

	return _cpsw_init(priv, dev->enetaddr);
}

static void cpsw_halt(struct eth_device *dev)
{
	struct cpsw_priv *priv = dev->priv;

	return _cpsw_halt(priv);
}

static int cpsw_send(struct eth_device *dev, void *packet, int length)
{
	struct cpsw_priv	*priv = dev->priv;

	return _cpsw_send(priv, packet, length);
}

static int cpsw_recv(struct eth_device *dev)
{
	struct cpsw_priv *priv = dev->priv;
	uchar *pkt = NULL;
	int len;

	len = _cpsw_recv(priv, &pkt);

	if (len > 0) {
		net_process_received_packet(pkt, len);
		cpdma_submit(priv, &priv->rx_chan, pkt, PKTSIZE);
	}

	return len;
}

int cpsw_register(struct cpsw_platform_data *data)
{
	struct cpsw_priv	*priv;
	struct eth_device	*dev;
	int ret;

	dev = calloc(sizeof(*dev), 1);
	if (!dev)
		return -ENOMEM;

	priv = calloc(sizeof(*priv), 1);
	if (!priv) {
		free(dev);
		return -ENOMEM;
	}

	priv->dev = dev;
	priv->data = data;

	strcpy(dev->name, "cpsw");
	dev->iobase	= 0;
	dev->init	= cpsw_init;
	dev->halt	= cpsw_halt;
	dev->send	= cpsw_send;
	dev->recv	= cpsw_recv;
	dev->priv	= priv;

	eth_register(dev);

	ret = _cpsw_register(priv);
	if (ret < 0) {
		eth_unregister(dev);
		free(dev);
		free(priv);
		return ret;
	}

	return 1;
}
#else
static int cpsw_eth_start(struct udevice *dev)
{
	struct eth_pdata *pdata = dev_get_platdata(dev);
	struct cpsw_priv *priv = dev_get_priv(dev);

	return _cpsw_init(priv, pdata->enetaddr);
}

static int cpsw_eth_send(struct udevice *dev, void *packet, int length)
{
	struct cpsw_priv *priv = dev_get_priv(dev);

	return _cpsw_send(priv, packet, length);
}

static int cpsw_eth_recv(struct udevice *dev, int flags, uchar **packetp)
{
	struct cpsw_priv *priv = dev_get_priv(dev);

	return _cpsw_recv(priv, packetp);
}

static int cpsw_eth_free_pkt(struct udevice *dev, uchar *packet,
				   int length)
{
	struct cpsw_priv *priv = dev_get_priv(dev);

	return cpdma_submit(priv, &priv->rx_chan, packet, PKTSIZE);
}

static void cpsw_eth_stop(struct udevice *dev)
{
	struct cpsw_priv *priv = dev_get_priv(dev);

	return _cpsw_halt(priv);
}

static const struct eth_ops cpsw_eth_ops = {
	.start		= cpsw_eth_start,
	.send		= cpsw_eth_send,
	.recv		= cpsw_eth_recv,
	.free_pkt	= cpsw_eth_free_pkt,
	.stop		= cpsw_eth_stop,
};

static inline fdt_addr_t cpsw_get_addr_by_node(const void *fdt, int node)
{
	return fdtdec_get_addr_size_auto_noparent(fdt, node, "reg", 0, NULL,
						  false);
}

static void cpsw_gmii_sel_am3352(struct cpsw_priv *priv,
				 phy_interface_t phy_mode)
{
	u32 reg;
	u32 mask;
	u32 mode = 0;
	bool rgmii_id = false;
	int slave = priv->data->active_slave;

	reg = readl(priv->data->gmii_sel);

	switch (phy_mode) {
	case PHY_INTERFACE_MODE_RMII:
		mode = AM33XX_GMII_SEL_MODE_RMII;
		break;

	case PHY_INTERFACE_MODE_RGMII:
		mode = AM33XX_GMII_SEL_MODE_RGMII;
		break;
	case PHY_INTERFACE_MODE_RGMII_ID:
	case PHY_INTERFACE_MODE_RGMII_RXID:
	case PHY_INTERFACE_MODE_RGMII_TXID:
		mode = AM33XX_GMII_SEL_MODE_RGMII;
		rgmii_id = true;
		break;

	case PHY_INTERFACE_MODE_MII:
	default:
		mode = AM33XX_GMII_SEL_MODE_MII;
		break;
	};

	mask = GMII_SEL_MODE_MASK << (slave * 2) | BIT(slave + 6);
	mode <<= slave * 2;

	if (priv->data->rmii_clock_external) {
		if (slave == 0)
			mode |= AM33XX_GMII_SEL_RMII1_IO_CLK_EN;
		else
			mode |= AM33XX_GMII_SEL_RMII2_IO_CLK_EN;
	}

	if (rgmii_id) {
		if (slave == 0)
			mode |= AM33XX_GMII_SEL_RGMII1_IDMODE;
		else
			mode |= AM33XX_GMII_SEL_RGMII2_IDMODE;
	}

	reg &= ~mask;
	reg |= mode;

	writel(reg, priv->data->gmii_sel);
}

static void cpsw_gmii_sel_dra7xx(struct cpsw_priv *priv,
				 phy_interface_t phy_mode)
{
	u32 reg;
	u32 mask;
	u32 mode = 0;
	int slave = priv->data->active_slave;

	reg = readl(priv->data->gmii_sel);

	switch (phy_mode) {
	case PHY_INTERFACE_MODE_RMII:
		mode = AM33XX_GMII_SEL_MODE_RMII;
		break;

	case PHY_INTERFACE_MODE_RGMII:
	case PHY_INTERFACE_MODE_RGMII_ID:
	case PHY_INTERFACE_MODE_RGMII_RXID:
	case PHY_INTERFACE_MODE_RGMII_TXID:
		mode = AM33XX_GMII_SEL_MODE_RGMII;
		break;

	case PHY_INTERFACE_MODE_MII:
	default:
		mode = AM33XX_GMII_SEL_MODE_MII;
		break;
	};

	switch (slave) {
	case 0:
		mask = GMII_SEL_MODE_MASK;
		break;
	case 1:
		mask = GMII_SEL_MODE_MASK << 4;
		mode <<= 4;
		break;
	default:
		dev_err(priv->dev, "invalid slave number...\n");
		return;
	}

	if (priv->data->rmii_clock_external)
		dev_err(priv->dev, "RMII External clock is not supported\n");

	reg &= ~mask;
	reg |= mode;

	writel(reg, priv->data->gmii_sel);
}

static void cpsw_phy_sel(struct cpsw_priv *priv, const char *compat,
			 phy_interface_t phy_mode)
{
	if (!strcmp(compat, "ti,am3352-cpsw-phy-sel"))
		cpsw_gmii_sel_am3352(priv, phy_mode);
	if (!strcmp(compat, "ti,am43xx-cpsw-phy-sel"))
		cpsw_gmii_sel_am3352(priv, phy_mode);
	else if (!strcmp(compat, "ti,dra7xx-cpsw-phy-sel"))
		cpsw_gmii_sel_dra7xx(priv, phy_mode);
}

static int cpsw_eth_probe(struct udevice *dev)
{
	struct cpsw_priv *priv = dev_get_priv(dev);
	struct eth_pdata *pdata = dev_get_platdata(dev);

	priv->dev = dev;
	priv->data = pdata->priv_pdata;
	ti_cm_get_macid(dev, priv->data, pdata->enetaddr);
	/* Select phy interface in control module */
	cpsw_phy_sel(priv, priv->data->phy_sel_compat,
		     pdata->phy_interface);

	return _cpsw_register(priv);
}

#if CONFIG_IS_ENABLED(OF_CONTROL)
static int cpsw_eth_ofdata_to_platdata(struct udevice *dev)
{
	struct eth_pdata *pdata = dev_get_platdata(dev);
	struct cpsw_platform_data *data;
	struct gpio_desc *mode_gpios;
	const char *phy_mode;
	const void *fdt = gd->fdt_blob;
	int node = dev_of_offset(dev);
	int subnode;
	int slave_index = 0;
	int active_slave;
	int num_mode_gpios;
	int ret;

	data = calloc(1, sizeof(struct cpsw_platform_data));
	pdata->priv_pdata = data;
	pdata->iobase = devfdt_get_addr(dev);
	data->version = CPSW_CTRL_VERSION_2;
	data->bd_ram_ofs = CPSW_BD_OFFSET;
	data->ale_reg_ofs = CPSW_ALE_OFFSET;
	data->cpdma_reg_ofs = CPSW_CPDMA_OFFSET;
	data->mdio_div = CPSW_MDIO_DIV;
	data->host_port_reg_ofs = CPSW_HOST_PORT_OFFSET,

	pdata->phy_interface = -1;

	data->cpsw_base = pdata->iobase;
	data->channels = fdtdec_get_int(fdt, node, "cpdma_channels", -1);
	if (data->channels <= 0) {
		printf("error: cpdma_channels not found in dt\n");
		return -ENOENT;
	}

	data->slaves = fdtdec_get_int(fdt, node, "slaves", -1);
	if (data->slaves <= 0) {
		printf("error: slaves not found in dt\n");
		return -ENOENT;
	}
	data->slave_data = malloc(sizeof(struct cpsw_slave_data) *
				       data->slaves);

	data->ale_entries = fdtdec_get_int(fdt, node, "ale_entries", -1);
	if (data->ale_entries <= 0) {
		printf("error: ale_entries not found in dt\n");
		return -ENOENT;
	}

	data->bd_ram_ofs = fdtdec_get_int(fdt, node, "bd_ram_size", -1);
	if (data->bd_ram_ofs <= 0) {
		printf("error: bd_ram_size not found in dt\n");
		return -ENOENT;
	}

	data->mac_control = fdtdec_get_int(fdt, node, "mac_control", -1);
	if (data->mac_control <= 0) {
		printf("error: ale_entries not found in dt\n");
		return -ENOENT;
	}

	num_mode_gpios = gpio_get_list_count(dev, "mode-gpios");
	if (num_mode_gpios > 0) {
		mode_gpios = malloc(sizeof(struct gpio_desc) *
				    num_mode_gpios);
		gpio_request_list_by_name(dev, "mode-gpios", mode_gpios,
					  num_mode_gpios, GPIOD_IS_OUT);
		free(mode_gpios);
	}

	active_slave = fdtdec_get_int(fdt, node, "active_slave", 0);
	data->active_slave = active_slave;

	fdt_for_each_subnode(subnode, fdt, node) {
		int len;
		const char *name;

		name = fdt_get_name(fdt, subnode, &len);
		if (!strncmp(name, "mdio", 4)) {
			u32 mdio_base;

			mdio_base = cpsw_get_addr_by_node(fdt, subnode);
			if (mdio_base == FDT_ADDR_T_NONE) {
				pr_err("Not able to get MDIO address space\n");
				return -ENOENT;
			}
			data->mdio_base = mdio_base;
		}

		if (!strncmp(name, "slave", 5)) {
			u32 phy_id[2];

			if (slave_index >= data->slaves)
				continue;
			phy_mode = fdt_getprop(fdt, subnode, "phy-mode", NULL);
			if (phy_mode)
				data->slave_data[slave_index].phy_if =
					phy_get_interface_by_name(phy_mode);

			data->slave_data[slave_index].phy_of_handle =
				fdtdec_lookup_phandle(fdt, subnode,
						      "phy-handle");

			if (data->slave_data[slave_index].phy_of_handle >= 0) {
				data->slave_data[slave_index].phy_addr =
						fdtdec_get_int(gd->fdt_blob,
						data->slave_data[slave_index].phy_of_handle,
							       "reg", -1);
			} else {
				fdtdec_get_int_array(fdt, subnode, "phy_id",
						     phy_id, 2);
				data->slave_data[slave_index].phy_addr =
						phy_id[1];
			}
			slave_index++;
		}

		if (!strncmp(name, "cpsw-phy-sel", 12)) {
			data->gmii_sel = cpsw_get_addr_by_node(fdt, subnode);

			if (data->gmii_sel == FDT_ADDR_T_NONE) {
				pr_err("Not able to get gmii_sel reg address\n");
				return -ENOENT;
			}

			if (fdt_get_property(fdt, subnode, "rmii-clock-ext",
					     NULL))
				data->rmii_clock_external = true;

			data->phy_sel_compat = fdt_getprop(fdt, subnode,
							   "compatible", NULL);
			if (!data->phy_sel_compat) {
				pr_err("Not able to get gmii_sel compatible\n");
				return -ENOENT;
			}
		}
	}

	data->slave_data[0].slave_reg_ofs = CPSW_SLAVE0_OFFSET;
	data->slave_data[0].sliver_reg_ofs = CPSW_SLIVER0_OFFSET;

	if (data->slaves == 2) {
		data->slave_data[1].slave_reg_ofs = CPSW_SLAVE1_OFFSET;
		data->slave_data[1].sliver_reg_ofs = CPSW_SLIVER1_OFFSET;
	}

	ret = ti_cm_get_macid_addr(dev, active_slave, data);
	if (ret < 0) {
		pr_err("cpsw read efuse mac failed\n");
		return ret;
	}

	pdata->phy_interface = data->slave_data[active_slave].phy_if;
	if (pdata->phy_interface == -1) {
		debug("%s: Invalid PHY interface '%s'\n", __func__, phy_mode);
		return -EINVAL;
	}

	return 0;
}

static const struct udevice_id cpsw_eth_ids[] = {
	{ .compatible = "ti,cpsw" },
	{ .compatible = "ti,am335x-cpsw" },
	{ }
};
#endif

int cpsw_get_slave_phy_addr(struct udevice *dev, int slave)
{
	struct cpsw_priv *priv = dev_get_priv(dev);
	struct cpsw_platform_data *data = priv->data;

	return data->slave_data[slave].phy_addr;
}

U_BOOT_DRIVER(eth_cpsw) = {
	.name	= "eth_cpsw",
	.id	= UCLASS_ETH,
#if CONFIG_IS_ENABLED(OF_CONTROL)
	.of_match = cpsw_eth_ids,
	.ofdata_to_platdata = cpsw_eth_ofdata_to_platdata,
	.platdata_auto_alloc_size = sizeof(struct eth_pdata),
#endif
	.probe	= cpsw_eth_probe,
	.ops	= &cpsw_eth_ops,
	.priv_auto_alloc_size = sizeof(struct cpsw_priv),
	.flags = DM_FLAG_ALLOC_PRIV_DMA | DM_FLAG_PRE_RELOC,
};
#endif /* CONFIG_DM_ETH */
