/*
 *  Copyright (C) 2013-2015 Chelsio Communications.  All rights reserved.
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms and conditions of the GNU General Public License,
 *  version 2, as published by the Free Software Foundation.
 *
 *  This program is distributed in the hope it will be useful, but WITHOUT
 *  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 *  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 *  more details.
 *
 *  The full GNU General Public License is included in this distribution in
 *  the file called "COPYING".
 *
 */

#include <linux/firmware.h>
#include <linux/mdio.h>

#include "cxgb4.h"
#include "t4_regs.h"
#include "t4fw_api.h"

#define EEPROM_MAGIC 0x38E2F10C

static u32 get_msglevel(struct net_device *dev)
{
	return netdev2adap(dev)->msg_enable;
}

static void set_msglevel(struct net_device *dev, u32 val)
{
	netdev2adap(dev)->msg_enable = val;
}

static const char stats_strings[][ETH_GSTRING_LEN] = {
	"TxOctetsOK         ",
	"TxFramesOK         ",
	"TxBroadcastFrames  ",
	"TxMulticastFrames  ",
	"TxUnicastFrames    ",
	"TxErrorFrames      ",

	"TxFrames64         ",
	"TxFrames65To127    ",
	"TxFrames128To255   ",
	"TxFrames256To511   ",
	"TxFrames512To1023  ",
	"TxFrames1024To1518 ",
	"TxFrames1519ToMax  ",

	"TxFramesDropped    ",
	"TxPauseFrames      ",
	"TxPPP0Frames       ",
	"TxPPP1Frames       ",
	"TxPPP2Frames       ",
	"TxPPP3Frames       ",
	"TxPPP4Frames       ",
	"TxPPP5Frames       ",
	"TxPPP6Frames       ",
	"TxPPP7Frames       ",

	"RxOctetsOK         ",
	"RxFramesOK         ",
	"RxBroadcastFrames  ",
	"RxMulticastFrames  ",
	"RxUnicastFrames    ",

	"RxFramesTooLong    ",
	"RxJabberErrors     ",
	"RxFCSErrors        ",
	"RxLengthErrors     ",
	"RxSymbolErrors     ",
	"RxRuntFrames       ",

	"RxFrames64         ",
	"RxFrames65To127    ",
	"RxFrames128To255   ",
	"RxFrames256To511   ",
	"RxFrames512To1023  ",
	"RxFrames1024To1518 ",
	"RxFrames1519ToMax  ",

	"RxPauseFrames      ",
	"RxPPP0Frames       ",
	"RxPPP1Frames       ",
	"RxPPP2Frames       ",
	"RxPPP3Frames       ",
	"RxPPP4Frames       ",
	"RxPPP5Frames       ",
	"RxPPP6Frames       ",
	"RxPPP7Frames       ",

	"RxBG0FramesDropped ",
	"RxBG1FramesDropped ",
	"RxBG2FramesDropped ",
	"RxBG3FramesDropped ",
	"RxBG0FramesTrunc   ",
	"RxBG1FramesTrunc   ",
	"RxBG2FramesTrunc   ",
	"RxBG3FramesTrunc   ",

	"TSO                ",
	"TxCsumOffload      ",
	"RxCsumGood         ",
	"VLANextractions    ",
	"VLANinsertions     ",
	"GROpackets         ",
	"GROmerged          ",
	"WriteCoalSuccess   ",
	"WriteCoalFail      ",
};

static int get_sset_count(struct net_device *dev, int sset)
{
	switch (sset) {
	case ETH_SS_STATS:
		return ARRAY_SIZE(stats_strings);
	default:
		return -EOPNOTSUPP;
	}
}

static int get_regs_len(struct net_device *dev)
{
	struct adapter *adap = netdev2adap(dev);

	return t4_get_regs_len(adap);
}

static int get_eeprom_len(struct net_device *dev)
{
	return EEPROMSIZE;
}

static void get_drvinfo(struct net_device *dev, struct ethtool_drvinfo *info)
{
	struct adapter *adapter = netdev2adap(dev);
	u32 exprom_vers;

	strlcpy(info->driver, cxgb4_driver_name, sizeof(info->driver));
	strlcpy(info->version, cxgb4_driver_version,
		sizeof(info->version));
	strlcpy(info->bus_info, pci_name(adapter->pdev),
		sizeof(info->bus_info));

	if (adapter->params.fw_vers)
		snprintf(info->fw_version, sizeof(info->fw_version),
			 "%u.%u.%u.%u, TP %u.%u.%u.%u",
			 FW_HDR_FW_VER_MAJOR_G(adapter->params.fw_vers),
			 FW_HDR_FW_VER_MINOR_G(adapter->params.fw_vers),
			 FW_HDR_FW_VER_MICRO_G(adapter->params.fw_vers),
			 FW_HDR_FW_VER_BUILD_G(adapter->params.fw_vers),
			 FW_HDR_FW_VER_MAJOR_G(adapter->params.tp_vers),
			 FW_HDR_FW_VER_MINOR_G(adapter->params.tp_vers),
			 FW_HDR_FW_VER_MICRO_G(adapter->params.tp_vers),
			 FW_HDR_FW_VER_BUILD_G(adapter->params.tp_vers));

	if (!t4_get_exprom_version(adapter, &exprom_vers))
		snprintf(info->erom_version, sizeof(info->erom_version),
			 "%u.%u.%u.%u",
			 FW_HDR_FW_VER_MAJOR_G(exprom_vers),
			 FW_HDR_FW_VER_MINOR_G(exprom_vers),
			 FW_HDR_FW_VER_MICRO_G(exprom_vers),
			 FW_HDR_FW_VER_BUILD_G(exprom_vers));
}

static void get_strings(struct net_device *dev, u32 stringset, u8 *data)
{
	if (stringset == ETH_SS_STATS)
		memcpy(data, stats_strings, sizeof(stats_strings));
}

/* port stats maintained per queue of the port. They should be in the same
 * order as in stats_strings above.
 */
struct queue_port_stats {
	u64 tso;
	u64 tx_csum;
	u64 rx_csum;
	u64 vlan_ex;
	u64 vlan_ins;
	u64 gro_pkts;
	u64 gro_merged;
};

static void collect_sge_port_stats(const struct adapter *adap,
				   const struct port_info *p,
				   struct queue_port_stats *s)
{
	int i;
	const struct sge_eth_txq *tx = &adap->sge.ethtxq[p->first_qset];
	const struct sge_eth_rxq *rx = &adap->sge.ethrxq[p->first_qset];

	memset(s, 0, sizeof(*s));
	for (i = 0; i < p->nqsets; i++, rx++, tx++) {
		s->tso += tx->tso;
		s->tx_csum += tx->tx_cso;
		s->rx_csum += rx->stats.rx_cso;
		s->vlan_ex += rx->stats.vlan_ex;
		s->vlan_ins += tx->vlan_ins;
		s->gro_pkts += rx->stats.lro_pkts;
		s->gro_merged += rx->stats.lro_merged;
	}
}

static void get_stats(struct net_device *dev, struct ethtool_stats *stats,
		      u64 *data)
{
	struct port_info *pi = netdev_priv(dev);
	struct adapter *adapter = pi->adapter;
	u32 val1, val2;

	t4_get_port_stats(adapter, pi->tx_chan, (struct port_stats *)data);

	data += sizeof(struct port_stats) / sizeof(u64);
	collect_sge_port_stats(adapter, pi, (struct queue_port_stats *)data);
	data += sizeof(struct queue_port_stats) / sizeof(u64);
	if (!is_t4(adapter->params.chip)) {
		t4_write_reg(adapter, SGE_STAT_CFG_A, STATSOURCE_T5_V(7));
		val1 = t4_read_reg(adapter, SGE_STAT_TOTAL_A);
		val2 = t4_read_reg(adapter, SGE_STAT_MATCH_A);
		*data = val1 - val2;
		data++;
		*data = val2;
		data++;
	} else {
		memset(data, 0, 2 * sizeof(u64));
		*data += 2;
	}
}

static void get_regs(struct net_device *dev, struct ethtool_regs *regs,
		     void *buf)
{
	struct adapter *adap = netdev2adap(dev);
	size_t buf_size;

	buf_size = t4_get_regs_len(adap);
	regs->version = mk_adap_vers(adap);
	t4_get_regs(adap, buf, buf_size);
}

static int restart_autoneg(struct net_device *dev)
{
	struct port_info *p = netdev_priv(dev);

	if (!netif_running(dev))
		return -EAGAIN;
	if (p->link_cfg.autoneg != AUTONEG_ENABLE)
		return -EINVAL;
	t4_restart_aneg(p->adapter, p->adapter->fn, p->tx_chan);
	return 0;
}

static int identify_port(struct net_device *dev,
			 enum ethtool_phys_id_state state)
{
	unsigned int val;
	struct adapter *adap = netdev2adap(dev);

	if (state == ETHTOOL_ID_ACTIVE)
		val = 0xffff;
	else if (state == ETHTOOL_ID_INACTIVE)
		val = 0;
	else
		return -EINVAL;

	return t4_identify_port(adap, adap->fn, netdev2pinfo(dev)->viid, val);
}

static unsigned int from_fw_linkcaps(enum fw_port_type type, unsigned int caps)
{
	unsigned int v = 0;

	if (type == FW_PORT_TYPE_BT_SGMII || type == FW_PORT_TYPE_BT_XFI ||
	    type == FW_PORT_TYPE_BT_XAUI) {
		v |= SUPPORTED_TP;
		if (caps & FW_PORT_CAP_SPEED_100M)
			v |= SUPPORTED_100baseT_Full;
		if (caps & FW_PORT_CAP_SPEED_1G)
			v |= SUPPORTED_1000baseT_Full;
		if (caps & FW_PORT_CAP_SPEED_10G)
			v |= SUPPORTED_10000baseT_Full;
	} else if (type == FW_PORT_TYPE_KX4 || type == FW_PORT_TYPE_KX) {
		v |= SUPPORTED_Backplane;
		if (caps & FW_PORT_CAP_SPEED_1G)
			v |= SUPPORTED_1000baseKX_Full;
		if (caps & FW_PORT_CAP_SPEED_10G)
			v |= SUPPORTED_10000baseKX4_Full;
	} else if (type == FW_PORT_TYPE_KR) {
		v |= SUPPORTED_Backplane | SUPPORTED_10000baseKR_Full;
	} else if (type == FW_PORT_TYPE_BP_AP) {
		v |= SUPPORTED_Backplane | SUPPORTED_10000baseR_FEC |
		     SUPPORTED_10000baseKR_Full | SUPPORTED_1000baseKX_Full;
	} else if (type == FW_PORT_TYPE_BP4_AP) {
		v |= SUPPORTED_Backplane | SUPPORTED_10000baseR_FEC |
		     SUPPORTED_10000baseKR_Full | SUPPORTED_1000baseKX_Full |
		     SUPPORTED_10000baseKX4_Full;
	} else if (type == FW_PORT_TYPE_FIBER_XFI ||
		   type == FW_PORT_TYPE_FIBER_XAUI ||
		   type == FW_PORT_TYPE_SFP ||
		   type == FW_PORT_TYPE_QSFP_10G ||
		   type == FW_PORT_TYPE_QSA) {
		v |= SUPPORTED_FIBRE;
		if (caps & FW_PORT_CAP_SPEED_1G)
			v |= SUPPORTED_1000baseT_Full;
		if (caps & FW_PORT_CAP_SPEED_10G)
			v |= SUPPORTED_10000baseT_Full;
	} else if (type == FW_PORT_TYPE_BP40_BA ||
		   type == FW_PORT_TYPE_QSFP) {
		v |= SUPPORTED_40000baseSR4_Full;
		v |= SUPPORTED_FIBRE;
	}

	if (caps & FW_PORT_CAP_ANEG)
		v |= SUPPORTED_Autoneg;
	return v;
}

static unsigned int to_fw_linkcaps(unsigned int caps)
{
	unsigned int v = 0;

	if (caps & ADVERTISED_100baseT_Full)
		v |= FW_PORT_CAP_SPEED_100M;
	if (caps & ADVERTISED_1000baseT_Full)
		v |= FW_PORT_CAP_SPEED_1G;
	if (caps & ADVERTISED_10000baseT_Full)
		v |= FW_PORT_CAP_SPEED_10G;
	if (caps & ADVERTISED_40000baseSR4_Full)
		v |= FW_PORT_CAP_SPEED_40G;
	return v;
}

static int get_settings(struct net_device *dev, struct ethtool_cmd *cmd)
{
	const struct port_info *p = netdev_priv(dev);

	if (p->port_type == FW_PORT_TYPE_BT_SGMII ||
	    p->port_type == FW_PORT_TYPE_BT_XFI ||
	    p->port_type == FW_PORT_TYPE_BT_XAUI) {
		cmd->port = PORT_TP;
	} else if (p->port_type == FW_PORT_TYPE_FIBER_XFI ||
		   p->port_type == FW_PORT_TYPE_FIBER_XAUI) {
		cmd->port = PORT_FIBRE;
	} else if (p->port_type == FW_PORT_TYPE_SFP ||
		   p->port_type == FW_PORT_TYPE_QSFP_10G ||
		   p->port_type == FW_PORT_TYPE_QSA ||
		   p->port_type == FW_PORT_TYPE_QSFP) {
		if (p->mod_type == FW_PORT_MOD_TYPE_LR ||
		    p->mod_type == FW_PORT_MOD_TYPE_SR ||
		    p->mod_type == FW_PORT_MOD_TYPE_ER ||
		    p->mod_type == FW_PORT_MOD_TYPE_LRM)
			cmd->port = PORT_FIBRE;
		else if (p->mod_type == FW_PORT_MOD_TYPE_TWINAX_PASSIVE ||
			 p->mod_type == FW_PORT_MOD_TYPE_TWINAX_ACTIVE)
			cmd->port = PORT_DA;
		else
			cmd->port = PORT_OTHER;
	} else {
		cmd->port = PORT_OTHER;
	}

	if (p->mdio_addr >= 0) {
		cmd->phy_address = p->mdio_addr;
		cmd->transceiver = XCVR_EXTERNAL;
		cmd->mdio_support = p->port_type == FW_PORT_TYPE_BT_SGMII ?
			MDIO_SUPPORTS_C22 : MDIO_SUPPORTS_C45;
	} else {
		cmd->phy_address = 0;  /* not really, but no better option */
		cmd->transceiver = XCVR_INTERNAL;
		cmd->mdio_support = 0;
	}

	cmd->supported = from_fw_linkcaps(p->port_type, p->link_cfg.supported);
	cmd->advertising = from_fw_linkcaps(p->port_type,
					    p->link_cfg.advertising);
	ethtool_cmd_speed_set(cmd,
			      netif_carrier_ok(dev) ? p->link_cfg.speed : 0);
	cmd->duplex = DUPLEX_FULL;
	cmd->autoneg = p->link_cfg.autoneg;
	cmd->maxtxpkt = 0;
	cmd->maxrxpkt = 0;
	return 0;
}

static unsigned int speed_to_caps(int speed)
{
	if (speed == 100)
		return FW_PORT_CAP_SPEED_100M;
	if (speed == 1000)
		return FW_PORT_CAP_SPEED_1G;
	if (speed == 10000)
		return FW_PORT_CAP_SPEED_10G;
	if (speed == 40000)
		return FW_PORT_CAP_SPEED_40G;
	return 0;
}

static int set_settings(struct net_device *dev, struct ethtool_cmd *cmd)
{
	unsigned int cap;
	struct port_info *p = netdev_priv(dev);
	struct link_config *lc = &p->link_cfg;
	u32 speed = ethtool_cmd_speed(cmd);

	if (cmd->duplex != DUPLEX_FULL)     /* only full-duplex supported */
		return -EINVAL;

	if (!(lc->supported & FW_PORT_CAP_ANEG)) {
		/* PHY offers a single speed.  See if that's what's
		 * being requested.
		 */
		if (cmd->autoneg == AUTONEG_DISABLE &&
		    (lc->supported & speed_to_caps(speed)))
			return 0;
		return -EINVAL;
	}

	if (cmd->autoneg == AUTONEG_DISABLE) {
		cap = speed_to_caps(speed);

		if (!(lc->supported & cap) ||
		    (speed == 1000) ||
		    (speed == 10000) ||
		    (speed == 40000))
			return -EINVAL;
		lc->requested_speed = cap;
		lc->advertising = 0;
	} else {
		cap = to_fw_linkcaps(cmd->advertising);
		if (!(lc->supported & cap))
			return -EINVAL;
		lc->requested_speed = 0;
		lc->advertising = cap | FW_PORT_CAP_ANEG;
	}
	lc->autoneg = cmd->autoneg;

	if (netif_running(dev))
		return t4_link_start(p->adapter, p->adapter->fn, p->tx_chan,
				     lc);
	return 0;
}

static void get_pauseparam(struct net_device *dev,
			   struct ethtool_pauseparam *epause)
{
	struct port_info *p = netdev_priv(dev);

	epause->autoneg = (p->link_cfg.requested_fc & PAUSE_AUTONEG) != 0;
	epause->rx_pause = (p->link_cfg.fc & PAUSE_RX) != 0;
	epause->tx_pause = (p->link_cfg.fc & PAUSE_TX) != 0;
}

static int set_pauseparam(struct net_device *dev,
			  struct ethtool_pauseparam *epause)
{
	struct port_info *p = netdev_priv(dev);
	struct link_config *lc = &p->link_cfg;

	if (epause->autoneg == AUTONEG_DISABLE)
		lc->requested_fc = 0;
	else if (lc->supported & FW_PORT_CAP_ANEG)
		lc->requested_fc = PAUSE_AUTONEG;
	else
		return -EINVAL;

	if (epause->rx_pause)
		lc->requested_fc |= PAUSE_RX;
	if (epause->tx_pause)
		lc->requested_fc |= PAUSE_TX;
	if (netif_running(dev))
		return t4_link_start(p->adapter, p->adapter->fn, p->tx_chan,
				     lc);
	return 0;
}

static void get_sge_param(struct net_device *dev, struct ethtool_ringparam *e)
{
	const struct port_info *pi = netdev_priv(dev);
	const struct sge *s = &pi->adapter->sge;

	e->rx_max_pending = MAX_RX_BUFFERS;
	e->rx_mini_max_pending = MAX_RSPQ_ENTRIES;
	e->rx_jumbo_max_pending = 0;
	e->tx_max_pending = MAX_TXQ_ENTRIES;

	e->rx_pending = s->ethrxq[pi->first_qset].fl.size - 8;
	e->rx_mini_pending = s->ethrxq[pi->first_qset].rspq.size;
	e->rx_jumbo_pending = 0;
	e->tx_pending = s->ethtxq[pi->first_qset].q.size;
}

static int set_sge_param(struct net_device *dev, struct ethtool_ringparam *e)
{
	int i;
	const struct port_info *pi = netdev_priv(dev);
	struct adapter *adapter = pi->adapter;
	struct sge *s = &adapter->sge;

	if (e->rx_pending > MAX_RX_BUFFERS || e->rx_jumbo_pending ||
	    e->tx_pending > MAX_TXQ_ENTRIES ||
	    e->rx_mini_pending > MAX_RSPQ_ENTRIES ||
	    e->rx_mini_pending < MIN_RSPQ_ENTRIES ||
	    e->rx_pending < MIN_FL_ENTRIES || e->tx_pending < MIN_TXQ_ENTRIES)
		return -EINVAL;

	if (adapter->flags & FULL_INIT_DONE)
		return -EBUSY;

	for (i = 0; i < pi->nqsets; ++i) {
		s->ethtxq[pi->first_qset + i].q.size = e->tx_pending;
		s->ethrxq[pi->first_qset + i].fl.size = e->rx_pending + 8;
		s->ethrxq[pi->first_qset + i].rspq.size = e->rx_mini_pending;
	}
	return 0;
}

/**
 * set_rx_intr_params - set a net devices's RX interrupt holdoff paramete!
 * @dev: the network device
 * @us: the hold-off time in us, or 0 to disable timer
 * @cnt: the hold-off packet count, or 0 to disable counter
 *
 * Set the RX interrupt hold-off parameters for a network device.
 */
static int set_rx_intr_params(struct net_device *dev,
			      unsigned int us, unsigned int cnt)
{
	int i, err;
	struct port_info *pi = netdev_priv(dev);
	struct adapter *adap = pi->adapter;
	struct sge_eth_rxq *q = &adap->sge.ethrxq[pi->first_qset];

	for (i = 0; i < pi->nqsets; i++, q++) {
		err = cxgb4_set_rspq_intr_params(&q->rspq, us, cnt);
		if (err)
			return err;
	}
	return 0;
}

static int set_adaptive_rx_setting(struct net_device *dev, int adaptive_rx)
{
	int i;
	struct port_info *pi = netdev_priv(dev);
	struct adapter *adap = pi->adapter;
	struct sge_eth_rxq *q = &adap->sge.ethrxq[pi->first_qset];

	for (i = 0; i < pi->nqsets; i++, q++)
		q->rspq.adaptive_rx = adaptive_rx;

	return 0;
}

static int get_adaptive_rx_setting(struct net_device *dev)
{
	struct port_info *pi = netdev_priv(dev);
	struct adapter *adap = pi->adapter;
	struct sge_eth_rxq *q = &adap->sge.ethrxq[pi->first_qset];

	return q->rspq.adaptive_rx;
}

static int set_coalesce(struct net_device *dev, struct ethtool_coalesce *c)
{
	set_adaptive_rx_setting(dev, c->use_adaptive_rx_coalesce);
	return set_rx_intr_params(dev, c->rx_coalesce_usecs,
				  c->rx_max_coalesced_frames);
}

static int get_coalesce(struct net_device *dev, struct ethtool_coalesce *c)
{
	const struct port_info *pi = netdev_priv(dev);
	const struct adapter *adap = pi->adapter;
	const struct sge_rspq *rq = &adap->sge.ethrxq[pi->first_qset].rspq;

	c->rx_coalesce_usecs = qtimer_val(adap, rq);
	c->rx_max_coalesced_frames = (rq->intr_params & QINTR_CNT_EN) ?
		adap->sge.counter_val[rq->pktcnt_idx] : 0;
	c->use_adaptive_rx_coalesce = get_adaptive_rx_setting(dev);
	return 0;
}

/**
 *	eeprom_ptov - translate a physical EEPROM address to virtual
 *	@phys_addr: the physical EEPROM address
 *	@fn: the PCI function number
 *	@sz: size of function-specific area
 *
 *	Translate a physical EEPROM address to virtual.  The first 1K is
 *	accessed through virtual addresses starting at 31K, the rest is
 *	accessed through virtual addresses starting at 0.
 *
 *	The mapping is as follows:
 *	[0..1K) -> [31K..32K)
 *	[1K..1K+A) -> [31K-A..31K)
 *	[1K+A..ES) -> [0..ES-A-1K)
 *
 *	where A = @fn * @sz, and ES = EEPROM size.
 */
static int eeprom_ptov(unsigned int phys_addr, unsigned int fn, unsigned int sz)
{
	fn *= sz;
	if (phys_addr < 1024)
		return phys_addr + (31 << 10);
	if (phys_addr < 1024 + fn)
		return 31744 - fn + phys_addr - 1024;
	if (phys_addr < EEPROMSIZE)
		return phys_addr - 1024 - fn;
	return -EINVAL;
}

/* The next two routines implement eeprom read/write from physical addresses.
 */
static int eeprom_rd_phys(struct adapter *adap, unsigned int phys_addr, u32 *v)
{
	int vaddr = eeprom_ptov(phys_addr, adap->fn, EEPROMPFSIZE);

	if (vaddr >= 0)
		vaddr = pci_read_vpd(adap->pdev, vaddr, sizeof(u32), v);
	return vaddr < 0 ? vaddr : 0;
}

static int eeprom_wr_phys(struct adapter *adap, unsigned int phys_addr, u32 v)
{
	int vaddr = eeprom_ptov(phys_addr, adap->fn, EEPROMPFSIZE);

	if (vaddr >= 0)
		vaddr = pci_write_vpd(adap->pdev, vaddr, sizeof(u32), &v);
	return vaddr < 0 ? vaddr : 0;
}

#define EEPROM_MAGIC 0x38E2F10C

static int get_eeprom(struct net_device *dev, struct ethtool_eeprom *e,
		      u8 *data)
{
	int i, err = 0;
	struct adapter *adapter = netdev2adap(dev);
	u8 *buf = kmalloc(EEPROMSIZE, GFP_KERNEL);

	if (!buf)
		return -ENOMEM;

	e->magic = EEPROM_MAGIC;
	for (i = e->offset & ~3; !err && i < e->offset + e->len; i += 4)
		err = eeprom_rd_phys(adapter, i, (u32 *)&buf[i]);

	if (!err)
		memcpy(data, buf + e->offset, e->len);
	kfree(buf);
	return err;
}

static int set_eeprom(struct net_device *dev, struct ethtool_eeprom *eeprom,
		      u8 *data)
{
	u8 *buf;
	int err = 0;
	u32 aligned_offset, aligned_len, *p;
	struct adapter *adapter = netdev2adap(dev);

	if (eeprom->magic != EEPROM_MAGIC)
		return -EINVAL;

	aligned_offset = eeprom->offset & ~3;
	aligned_len = (eeprom->len + (eeprom->offset & 3) + 3) & ~3;

	if (adapter->fn > 0) {
		u32 start = 1024 + adapter->fn * EEPROMPFSIZE;

		if (aligned_offset < start ||
		    aligned_offset + aligned_len > start + EEPROMPFSIZE)
			return -EPERM;
	}

	if (aligned_offset != eeprom->offset || aligned_len != eeprom->len) {
		/* RMW possibly needed for first or last words.
		 */
		buf = kmalloc(aligned_len, GFP_KERNEL);
		if (!buf)
			return -ENOMEM;
		err = eeprom_rd_phys(adapter, aligned_offset, (u32 *)buf);
		if (!err && aligned_len > 4)
			err = eeprom_rd_phys(adapter,
					     aligned_offset + aligned_len - 4,
					     (u32 *)&buf[aligned_len - 4]);
		if (err)
			goto out;
		memcpy(buf + (eeprom->offset & 3), data, eeprom->len);
	} else {
		buf = data;
	}

	err = t4_seeprom_wp(adapter, false);
	if (err)
		goto out;

	for (p = (u32 *)buf; !err && aligned_len; aligned_len -= 4, p++) {
		err = eeprom_wr_phys(adapter, aligned_offset, *p);
		aligned_offset += 4;
	}

	if (!err)
		err = t4_seeprom_wp(adapter, true);
out:
	if (buf != data)
		kfree(buf);
	return err;
}

static int set_flash(struct net_device *netdev, struct ethtool_flash *ef)
{
	int ret;
	const struct firmware *fw;
	struct adapter *adap = netdev2adap(netdev);
	unsigned int mbox = PCIE_FW_MASTER_M + 1;

	ef->data[sizeof(ef->data) - 1] = '\0';
	ret = request_firmware(&fw, ef->data, adap->pdev_dev);
	if (ret < 0)
		return ret;

	/* If the adapter has been fully initialized then we'll go ahead and
	 * try to get the firmware's cooperation in upgrading to the new
	 * firmware image otherwise we'll try to do the entire job from the
	 * host ... and we always "force" the operation in this path.
	 */
	if (adap->flags & FULL_INIT_DONE)
		mbox = adap->mbox;

	ret = t4_fw_upgrade(adap, mbox, fw->data, fw->size, 1);
	release_firmware(fw);
	if (!ret)
		dev_info(adap->pdev_dev,
			 "loaded firmware %s, reload cxgb4 driver\n", ef->data);
	return ret;
}

#define WOL_SUPPORTED (WAKE_BCAST | WAKE_MAGIC)
#define BCAST_CRC 0xa0ccc1a6

static void get_wol(struct net_device *dev, struct ethtool_wolinfo *wol)
{
	wol->supported = WAKE_BCAST | WAKE_MAGIC;
	wol->wolopts = netdev2adap(dev)->wol;
	memset(&wol->sopass, 0, sizeof(wol->sopass));
}

static int set_wol(struct net_device *dev, struct ethtool_wolinfo *wol)
{
	int err = 0;
	struct port_info *pi = netdev_priv(dev);

	if (wol->wolopts & ~WOL_SUPPORTED)
		return -EINVAL;
	t4_wol_magic_enable(pi->adapter, pi->tx_chan,
			    (wol->wolopts & WAKE_MAGIC) ? dev->dev_addr : NULL);
	if (wol->wolopts & WAKE_BCAST) {
		err = t4_wol_pat_enable(pi->adapter, pi->tx_chan, 0xfe, ~0ULL,
					~0ULL, 0, false);
		if (!err)
			err = t4_wol_pat_enable(pi->adapter, pi->tx_chan, 1,
						~6ULL, ~0ULL, BCAST_CRC, true);
	} else {
		t4_wol_pat_enable(pi->adapter, pi->tx_chan, 0, 0, 0, 0, false);
	}
	return err;
}

static u32 get_rss_table_size(struct net_device *dev)
{
	const struct port_info *pi = netdev_priv(dev);

	return pi->rss_size;
}

static int get_rss_table(struct net_device *dev, u32 *p, u8 *key, u8 *hfunc)
{
	const struct port_info *pi = netdev_priv(dev);
	unsigned int n = pi->rss_size;

	if (hfunc)
		*hfunc = ETH_RSS_HASH_TOP;
	if (!p)
		return 0;
	while (n--)
		p[n] = pi->rss[n];
	return 0;
}

static int set_rss_table(struct net_device *dev, const u32 *p, const u8 *key,
			 const u8 hfunc)
{
	unsigned int i;
	struct port_info *pi = netdev_priv(dev);

	/* We require at least one supported parameter to be changed and no
	 * change in any of the unsupported parameters
	 */
	if (key ||
	    (hfunc != ETH_RSS_HASH_NO_CHANGE && hfunc != ETH_RSS_HASH_TOP))
		return -EOPNOTSUPP;
	if (!p)
		return 0;

	for (i = 0; i < pi->rss_size; i++)
		pi->rss[i] = p[i];
	if (pi->adapter->flags & FULL_INIT_DONE)
		return cxgb4_write_rss(pi, pi->rss);
	return 0;
}

static int get_rxnfc(struct net_device *dev, struct ethtool_rxnfc *info,
		     u32 *rules)
{
	const struct port_info *pi = netdev_priv(dev);

	switch (info->cmd) {
	case ETHTOOL_GRXFH: {
		unsigned int v = pi->rss_mode;

		info->data = 0;
		switch (info->flow_type) {
		case TCP_V4_FLOW:
			if (v & FW_RSS_VI_CONFIG_CMD_IP4FOURTUPEN_F)
				info->data = RXH_IP_SRC | RXH_IP_DST |
					     RXH_L4_B_0_1 | RXH_L4_B_2_3;
			else if (v & FW_RSS_VI_CONFIG_CMD_IP4TWOTUPEN_F)
				info->data = RXH_IP_SRC | RXH_IP_DST;
			break;
		case UDP_V4_FLOW:
			if ((v & FW_RSS_VI_CONFIG_CMD_IP4FOURTUPEN_F) &&
			    (v & FW_RSS_VI_CONFIG_CMD_UDPEN_F))
				info->data = RXH_IP_SRC | RXH_IP_DST |
					     RXH_L4_B_0_1 | RXH_L4_B_2_3;
			else if (v & FW_RSS_VI_CONFIG_CMD_IP4TWOTUPEN_F)
				info->data = RXH_IP_SRC | RXH_IP_DST;
			break;
		case SCTP_V4_FLOW:
		case AH_ESP_V4_FLOW:
		case IPV4_FLOW:
			if (v & FW_RSS_VI_CONFIG_CMD_IP4TWOTUPEN_F)
				info->data = RXH_IP_SRC | RXH_IP_DST;
			break;
		case TCP_V6_FLOW:
			if (v & FW_RSS_VI_CONFIG_CMD_IP6FOURTUPEN_F)
				info->data = RXH_IP_SRC | RXH_IP_DST |
					     RXH_L4_B_0_1 | RXH_L4_B_2_3;
			else if (v & FW_RSS_VI_CONFIG_CMD_IP6TWOTUPEN_F)
				info->data = RXH_IP_SRC | RXH_IP_DST;
			break;
		case UDP_V6_FLOW:
			if ((v & FW_RSS_VI_CONFIG_CMD_IP6FOURTUPEN_F) &&
			    (v & FW_RSS_VI_CONFIG_CMD_UDPEN_F))
				info->data = RXH_IP_SRC | RXH_IP_DST |
					     RXH_L4_B_0_1 | RXH_L4_B_2_3;
			else if (v & FW_RSS_VI_CONFIG_CMD_IP6TWOTUPEN_F)
				info->data = RXH_IP_SRC | RXH_IP_DST;
			break;
		case SCTP_V6_FLOW:
		case AH_ESP_V6_FLOW:
		case IPV6_FLOW:
			if (v & FW_RSS_VI_CONFIG_CMD_IP6TWOTUPEN_F)
				info->data = RXH_IP_SRC | RXH_IP_DST;
			break;
		}
		return 0;
	}
	case ETHTOOL_GRXRINGS:
		info->data = pi->nqsets;
		return 0;
	}
	return -EOPNOTSUPP;
}

static const struct ethtool_ops cxgb_ethtool_ops = {
	.get_settings      = get_settings,
	.set_settings      = set_settings,
	.get_drvinfo       = get_drvinfo,
	.get_msglevel      = get_msglevel,
	.set_msglevel      = set_msglevel,
	.get_ringparam     = get_sge_param,
	.set_ringparam     = set_sge_param,
	.get_coalesce      = get_coalesce,
	.set_coalesce      = set_coalesce,
	.get_eeprom_len    = get_eeprom_len,
	.get_eeprom        = get_eeprom,
	.set_eeprom        = set_eeprom,
	.get_pauseparam    = get_pauseparam,
	.set_pauseparam    = set_pauseparam,
	.get_link          = ethtool_op_get_link,
	.get_strings       = get_strings,
	.set_phys_id       = identify_port,
	.nway_reset        = restart_autoneg,
	.get_sset_count    = get_sset_count,
	.get_ethtool_stats = get_stats,
	.get_regs_len      = get_regs_len,
	.get_regs          = get_regs,
	.get_wol           = get_wol,
	.set_wol           = set_wol,
	.get_rxnfc         = get_rxnfc,
	.get_rxfh_indir_size = get_rss_table_size,
	.get_rxfh	   = get_rss_table,
	.set_rxfh	   = set_rss_table,
	.flash_device      = set_flash,
};

void cxgb4_set_ethtool_ops(struct net_device *netdev)
{
	netdev->ethtool_ops = &cxgb_ethtool_ops;
}
