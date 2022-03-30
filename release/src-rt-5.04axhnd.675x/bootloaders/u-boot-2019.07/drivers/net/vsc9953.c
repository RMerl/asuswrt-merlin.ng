// SPDX-License-Identifier: GPL-2.0+
/*
 *  Copyright 2014 - 2015 Freescale Semiconductor, Inc.
 *
 *  Driver for the Vitesse VSC9953 L2 Switch
 */

#include <asm/io.h>
#include <asm/fsl_serdes.h>
#include <fm_eth.h>
#include <fsl_memac.h>
#include <bitfield.h>
#include <errno.h>
#include <malloc.h>
#include <vsc9953.h>
#include <ethsw.h>

static struct vsc9953_info vsc9953_l2sw = {
		.port[0] = VSC9953_PORT_INFO_INITIALIZER(0),
		.port[1] = VSC9953_PORT_INFO_INITIALIZER(1),
		.port[2] = VSC9953_PORT_INFO_INITIALIZER(2),
		.port[3] = VSC9953_PORT_INFO_INITIALIZER(3),
		.port[4] = VSC9953_PORT_INFO_INITIALIZER(4),
		.port[5] = VSC9953_PORT_INFO_INITIALIZER(5),
		.port[6] = VSC9953_PORT_INFO_INITIALIZER(6),
		.port[7] = VSC9953_PORT_INFO_INITIALIZER(7),
		.port[8] = VSC9953_PORT_INFO_INITIALIZER(8),
		.port[9] = VSC9953_PORT_INFO_INITIALIZER(9),
};

void vsc9953_port_info_set_mdio(int port_no, struct mii_dev *bus)
{
	if (!VSC9953_PORT_CHECK(port_no))
		return;

	vsc9953_l2sw.port[port_no].bus = bus;
}

void vsc9953_port_info_set_phy_address(int port_no, int address)
{
	if (!VSC9953_PORT_CHECK(port_no))
		return;

	vsc9953_l2sw.port[port_no].phyaddr = address;
}

void vsc9953_port_info_set_phy_int(int port_no, phy_interface_t phy_int)
{
	if (!VSC9953_PORT_CHECK(port_no))
		return;

	vsc9953_l2sw.port[port_no].enet_if = phy_int;
}

void vsc9953_port_enable(int port_no)
{
	if (!VSC9953_PORT_CHECK(port_no))
		return;

	vsc9953_l2sw.port[port_no].enabled = 1;
}

void vsc9953_port_disable(int port_no)
{
	if (!VSC9953_PORT_CHECK(port_no))
		return;

	vsc9953_l2sw.port[port_no].enabled = 0;
}

static void vsc9953_mdio_write(struct vsc9953_mii_mng *phyregs, int port_addr,
		int regnum, int value)
{
	int timeout = 50000;

	out_le32(&phyregs->miimcmd, (0x1 << 31) | ((port_addr & 0x1f) << 25) |
			((regnum & 0x1f) << 20) | ((value & 0xffff) << 4) |
			(0x1 << 1));
	asm("sync");

	while ((in_le32(&phyregs->miimstatus) & 0x8) && --timeout)
		udelay(1);

	if (timeout == 0)
		debug("Timeout waiting for MDIO write\n");
}

static int vsc9953_mdio_read(struct vsc9953_mii_mng *phyregs, int port_addr,
		int regnum)
{
	int value = 0xFFFF;
	int timeout = 50000;

	while ((in_le32(&phyregs->miimstatus) & MIIMIND_OPR_PEND) && --timeout)
		udelay(1);
	if (timeout == 0) {
		debug("Timeout waiting for MDIO operation to finish\n");
		return value;
	}

	/* Put the address of the phy, and the register
	 * number into MIICMD
	 */
	out_le32(&phyregs->miimcmd, (0x1 << 31) | ((port_addr & 0x1f) << 25) |
			((regnum & 0x1f) << 20) | ((value & 0xffff) << 4) |
			(0x2 << 1));

	timeout = 50000;
	/* Wait for the the indication that the read is done */
	while ((in_le32(&phyregs->miimstatus) & 0x8) && --timeout)
		udelay(1);
	if (timeout == 0)
		debug("Timeout waiting for MDIO read\n");

	/* Grab the value read from the PHY */
	value = in_le32(&phyregs->miimdata);

	if ((value & 0x00030000) == 0)
		return value & 0x0000ffff;

	return value;
}

static int init_phy(struct eth_device *dev)
{
	struct vsc9953_port_info *l2sw_port = dev->priv;
	struct phy_device *phydev = NULL;

#ifdef CONFIG_PHYLIB
	if (!l2sw_port->bus)
		return 0;
	phydev = phy_connect(l2sw_port->bus, l2sw_port->phyaddr, dev,
			l2sw_port->enet_if);
	if (!phydev) {
		printf("Failed to connect\n");
		return -1;
	}

	phydev->supported &= SUPPORTED_10baseT_Half |
			SUPPORTED_10baseT_Full |
			SUPPORTED_100baseT_Half |
			SUPPORTED_100baseT_Full |
			SUPPORTED_1000baseT_Full;
	phydev->advertising = phydev->supported;

	l2sw_port->phydev = phydev;

	phy_config(phydev);
#endif

	return 0;
}

static int vsc9953_port_init(int port_no)
{
	struct eth_device *dev;

	/* Internal ports never have a PHY */
	if (VSC9953_INTERNAL_PORT_CHECK(port_no))
		return 0;

	/* alloc eth device */
	dev = (struct eth_device *)calloc(1, sizeof(struct eth_device));
	if (!dev)
		return -ENOMEM;

	sprintf(dev->name, "SW@PORT%d", port_no);
	dev->priv = &vsc9953_l2sw.port[port_no];
	dev->init = NULL;
	dev->halt = NULL;
	dev->send = NULL;
	dev->recv = NULL;

	if (init_phy(dev)) {
		free(dev);
		return -ENODEV;
	}

	return 0;
}

static int vsc9953_vlan_table_poll_idle(void)
{
	struct vsc9953_analyzer *l2ana_reg;
	int timeout;

	l2ana_reg = (struct vsc9953_analyzer *)(VSC9953_OFFSET +
			VSC9953_ANA_OFFSET);

	timeout = 50000;
	while (((in_le32(&l2ana_reg->ana_tables.vlan_access) &
		 VSC9953_VLAN_CMD_MASK) != VSC9953_VLAN_CMD_IDLE) && --timeout)
		udelay(1);

	return timeout ? 0 : -EBUSY;
}

#ifdef CONFIG_CMD_ETHSW
/* Add/remove a port to/from a VLAN */
static void vsc9953_vlan_table_membership_set(int vid, u32 port_no, u8 add)
{
	u32 val;
	struct vsc9953_analyzer *l2ana_reg;

	l2ana_reg = (struct vsc9953_analyzer *)(VSC9953_OFFSET +
			VSC9953_ANA_OFFSET);

	if (vsc9953_vlan_table_poll_idle() < 0) {
		debug("VLAN table timeout\n");
		return;
	}

	val = in_le32(&l2ana_reg->ana_tables.vlan_tidx);
	val = bitfield_replace_by_mask(val, VSC9953_ANA_TBL_VID_MASK, vid);
	out_le32(&l2ana_reg->ana_tables.vlan_tidx, val);

	clrsetbits_le32(&l2ana_reg->ana_tables.vlan_access,
			VSC9953_VLAN_CMD_MASK, VSC9953_VLAN_CMD_READ);

	if (vsc9953_vlan_table_poll_idle() < 0) {
		debug("VLAN table timeout\n");
		return;
	}

	val = in_le32(&l2ana_reg->ana_tables.vlan_tidx);
	val = bitfield_replace_by_mask(val, VSC9953_ANA_TBL_VID_MASK, vid);
	out_le32(&l2ana_reg->ana_tables.vlan_tidx, val);

	val = in_le32(&l2ana_reg->ana_tables.vlan_access);
	if (!add) {
		val = bitfield_replace_by_mask(val, VSC9953_VLAN_CMD_MASK,
						VSC9953_VLAN_CMD_WRITE) &
		      ~(bitfield_replace_by_mask(0, VSC9953_VLAN_PORT_MASK,
						 (1 << port_no)));
		 ;
	} else {
		val = bitfield_replace_by_mask(val, VSC9953_VLAN_CMD_MASK,
						VSC9953_VLAN_CMD_WRITE) |
		      bitfield_replace_by_mask(0, VSC9953_VLAN_PORT_MASK,
					       (1 << port_no));
	}
	out_le32(&l2ana_reg->ana_tables.vlan_access, val);

	/* wait for VLAN table command to flush */
	if (vsc9953_vlan_table_poll_idle() < 0) {
		debug("VLAN table timeout\n");
		return;
	}
}

/* show VLAN membership for a port */
static void vsc9953_vlan_membership_show(int port_no)
{
	u32 val;
	struct vsc9953_analyzer *l2ana_reg;
	u32 vid;

	l2ana_reg = (struct vsc9953_analyzer *)(VSC9953_OFFSET +
			VSC9953_ANA_OFFSET);

	printf("Port %d VLAN membership: ", port_no);

	for (vid = 0; vid < VSC9953_MAX_VLAN; vid++) {
		if (vsc9953_vlan_table_poll_idle() < 0) {
			debug("VLAN table timeout\n");
			return;
		}

		val = in_le32(&l2ana_reg->ana_tables.vlan_tidx);
		val = bitfield_replace_by_mask(val, VSC9953_ANA_TBL_VID_MASK,
					       vid);
		out_le32(&l2ana_reg->ana_tables.vlan_tidx, val);

		clrsetbits_le32(&l2ana_reg->ana_tables.vlan_access,
				VSC9953_VLAN_CMD_MASK, VSC9953_VLAN_CMD_READ);

		if (vsc9953_vlan_table_poll_idle() < 0) {
			debug("VLAN table timeout\n");
			return;
		}

		val = in_le32(&l2ana_reg->ana_tables.vlan_access);

		if (bitfield_extract_by_mask(val, VSC9953_VLAN_PORT_MASK) &
		    (1 << port_no))
			printf("%d ", vid);
	}
	printf("\n");
}
#endif

/* vlan table set/clear all membership of vid */
static void vsc9953_vlan_table_membership_all_set(int vid, int set_member)
{
	uint val;
	struct vsc9953_analyzer *l2ana_reg;

	l2ana_reg = (struct vsc9953_analyzer *)(VSC9953_OFFSET +
			VSC9953_ANA_OFFSET);

	if (vsc9953_vlan_table_poll_idle() < 0) {
		debug("VLAN table timeout\n");
		return;
	}

	/* read current vlan configuration */
	val = in_le32(&l2ana_reg->ana_tables.vlan_tidx);
	out_le32(&l2ana_reg->ana_tables.vlan_tidx,
		 bitfield_replace_by_mask(val, VSC9953_ANA_TBL_VID_MASK, vid));

	clrsetbits_le32(&l2ana_reg->ana_tables.vlan_access,
			VSC9953_VLAN_CMD_MASK, VSC9953_VLAN_CMD_READ);

	if (vsc9953_vlan_table_poll_idle() < 0) {
		debug("VLAN table timeout\n");
		return;
	}

	val = in_le32(&l2ana_reg->ana_tables.vlan_tidx);
	out_le32(&l2ana_reg->ana_tables.vlan_tidx,
		 bitfield_replace_by_mask(val, VSC9953_ANA_TBL_VID_MASK, vid));

	clrsetbits_le32(&l2ana_reg->ana_tables.vlan_access,
			VSC9953_VLAN_PORT_MASK | VSC9953_VLAN_CMD_MASK,
			VSC9953_VLAN_CMD_WRITE |
			(set_member ? VSC9953_VLAN_PORT_MASK : 0));
}

#ifdef CONFIG_CMD_ETHSW
/* Get PVID of a VSC9953 port */
static int vsc9953_port_vlan_pvid_get(int port_nr, int *pvid)
{
	u32 val;
	struct vsc9953_analyzer *l2ana_reg;

	/* Administrative down */
	if (!vsc9953_l2sw.port[port_nr].enabled) {
		printf("Port %d is administrative down\n", port_nr);
		return -1;
	}

	l2ana_reg = (struct vsc9953_analyzer *)(VSC9953_OFFSET +
				VSC9953_ANA_OFFSET);

	/* Get ingress PVID */
	val = in_le32(&l2ana_reg->port[port_nr].vlan_cfg);
	*pvid = bitfield_extract_by_mask(val, VSC9953_VLAN_CFG_VID_MASK);

	return 0;
}
#endif

/* Set PVID for a VSC9953 port */
static void vsc9953_port_vlan_pvid_set(int port_no, int pvid)
{
	uint val;
	struct vsc9953_analyzer *l2ana_reg;
	struct vsc9953_rew_reg *l2rew_reg;

	/* Administrative down */
	if (!vsc9953_l2sw.port[port_no].enabled) {
		printf("Port %d is administrative down\n", port_no);
		return;
	}

	l2ana_reg = (struct vsc9953_analyzer *)(VSC9953_OFFSET +
			VSC9953_ANA_OFFSET);
	l2rew_reg = (struct vsc9953_rew_reg *)(VSC9953_OFFSET +
			VSC9953_REW_OFFSET);

	/* Set PVID on ingress */
	val = in_le32(&l2ana_reg->port[port_no].vlan_cfg);
	val = bitfield_replace_by_mask(val, VSC9953_VLAN_CFG_VID_MASK, pvid);
	out_le32(&l2ana_reg->port[port_no].vlan_cfg, val);

	/* Set PVID on egress */
	val = in_le32(&l2rew_reg->port[port_no].port_vlan_cfg);
	val = bitfield_replace_by_mask(val, VSC9953_PORT_VLAN_CFG_VID_MASK,
				       pvid);
	out_le32(&l2rew_reg->port[port_no].port_vlan_cfg, val);
}

static void vsc9953_port_all_vlan_pvid_set(int pvid)
{
	int i;

	for (i = 0; i < VSC9953_MAX_PORTS; i++)
		vsc9953_port_vlan_pvid_set(i, pvid);
}

/* Enable/disable vlan aware of a VSC9953 port */
static void vsc9953_port_vlan_aware_set(int port_no, int enabled)
{
	struct vsc9953_analyzer *l2ana_reg;

	/* Administrative down */
	if (!vsc9953_l2sw.port[port_no].enabled) {
		printf("Port %d is administrative down\n", port_no);
		return;
	}

	l2ana_reg = (struct vsc9953_analyzer *)(VSC9953_OFFSET +
			VSC9953_ANA_OFFSET);

	if (enabled)
		setbits_le32(&l2ana_reg->port[port_no].vlan_cfg,
			     VSC9953_VLAN_CFG_AWARE_ENA);
	else
		clrbits_le32(&l2ana_reg->port[port_no].vlan_cfg,
			     VSC9953_VLAN_CFG_AWARE_ENA);
}

/* Set all VSC9953 ports' vlan aware  */
static void vsc9953_port_all_vlan_aware_set(int enabled)
{
	int i;

	for (i = 0; i < VSC9953_MAX_PORTS; i++)
		vsc9953_port_vlan_aware_set(i, enabled);
}

/* Enable/disable vlan pop count of a VSC9953 port */
static void vsc9953_port_vlan_popcnt_set(int port_no, int popcnt)
{
	uint val;
	struct vsc9953_analyzer *l2ana_reg;

	/* Administrative down */
	if (!vsc9953_l2sw.port[port_no].enabled) {
		printf("Port %d is administrative down\n", port_no);
		return;
	}

	if (popcnt > 3 || popcnt < 0) {
		printf("Invalid pop count value: %d\n", port_no);
		return;
	}

	l2ana_reg = (struct vsc9953_analyzer *)(VSC9953_OFFSET +
			VSC9953_ANA_OFFSET);

	val = in_le32(&l2ana_reg->port[port_no].vlan_cfg);
	val = bitfield_replace_by_mask(val, VSC9953_VLAN_CFG_POP_CNT_MASK,
				       popcnt);
	out_le32(&l2ana_reg->port[port_no].vlan_cfg, val);
}

/* Set all VSC9953 ports' pop count  */
static void vsc9953_port_all_vlan_poncnt_set(int popcnt)
{
	int i;

	for (i = 0; i < VSC9953_MAX_PORTS; i++)
		vsc9953_port_vlan_popcnt_set(i, popcnt);
}

/* Enable/disable learning for frames dropped due to ingress filtering */
static void vsc9953_vlan_ingr_fltr_learn_drop(int enable)
{
	struct vsc9953_analyzer *l2ana_reg;

	l2ana_reg = (struct vsc9953_analyzer *)(VSC9953_OFFSET +
			VSC9953_ANA_OFFSET);

	if (enable)
		setbits_le32(&l2ana_reg->ana.adv_learn, VSC9953_VLAN_CHK);
	else
		clrbits_le32(&l2ana_reg->ana.adv_learn, VSC9953_VLAN_CHK);
}

enum aggr_code_mode {
	AGGR_CODE_RAND = 0,
	AGGR_CODE_ALL,	/* S/D MAC, IPv4 S/D IP, IPv6 Flow Label, S/D PORT */
};

/* Set aggregation code generation mode */
static int vsc9953_aggr_code_set(enum aggr_code_mode ac)
{
	int rc;
	struct vsc9953_analyzer *l2ana_reg;

	l2ana_reg = (struct vsc9953_analyzer *)(VSC9953_OFFSET +
						VSC9953_ANA_OFFSET);

	switch (ac) {
	case AGGR_CODE_RAND:
		clrsetbits_le32(&l2ana_reg->common.aggr_cfg,
				VSC9953_AC_DMAC_ENA | VSC9953_AC_SMAC_ENA |
				VSC9953_AC_IP6_LBL_ENA |
				VSC9953_AC_IP6_TCPUDP_ENA |
				VSC9953_AC_IP4_SIPDIP_ENA |
				VSC9953_AC_IP4_TCPUDP_ENA, VSC9953_AC_RND_ENA);
		rc = 0;
		break;
	case AGGR_CODE_ALL:
		clrsetbits_le32(&l2ana_reg->common.aggr_cfg, VSC9953_AC_RND_ENA,
				VSC9953_AC_DMAC_ENA | VSC9953_AC_SMAC_ENA |
				VSC9953_AC_IP6_LBL_ENA |
				VSC9953_AC_IP6_TCPUDP_ENA |
				VSC9953_AC_IP4_SIPDIP_ENA |
				VSC9953_AC_IP4_TCPUDP_ENA);
		rc = 0;
		break;
	default:
		/* unknown mode for aggregation code */
		rc = -EINVAL;
	}

	return rc;
}

/* Egress untag modes of a VSC9953 port */
enum egress_untag_mode {
	EGRESS_UNTAG_ALL = 0,
	EGRESS_UNTAG_PVID_AND_ZERO,
	EGRESS_UNTAG_ZERO,
	EGRESS_UNTAG_NONE,
};

#ifdef CONFIG_CMD_ETHSW
/* Get egress tagging configuration for a VSC9953 port */
static int vsc9953_port_vlan_egr_untag_get(int port_no,
					   enum egress_untag_mode *mode)
{
	u32 val;
	struct vsc9953_rew_reg *l2rew_reg;

	/* Administrative down */
	if (!vsc9953_l2sw.port[port_no].enabled) {
		printf("Port %d is administrative down\n", port_no);
		return -1;
	}

	l2rew_reg = (struct vsc9953_rew_reg *)(VSC9953_OFFSET +
			VSC9953_REW_OFFSET);

	val = in_le32(&l2rew_reg->port[port_no].port_tag_cfg);

	switch (val & VSC9953_TAG_CFG_MASK) {
	case VSC9953_TAG_CFG_NONE:
		*mode = EGRESS_UNTAG_ALL;
		return 0;
	case VSC9953_TAG_CFG_ALL_BUT_PVID_ZERO:
		*mode = EGRESS_UNTAG_PVID_AND_ZERO;
		return 0;
	case VSC9953_TAG_CFG_ALL_BUT_ZERO:
		*mode = EGRESS_UNTAG_ZERO;
		return 0;
	case VSC9953_TAG_CFG_ALL:
		*mode = EGRESS_UNTAG_NONE;
		return 0;
	default:
		printf("Unknown egress tagging configuration for port %d\n",
		       port_no);
		return -1;
	}
}

/* Show egress tagging configuration for a VSC9953 port */
static void vsc9953_port_vlan_egr_untag_show(int port_no)
{
	enum egress_untag_mode mode;

	if (vsc9953_port_vlan_egr_untag_get(port_no, &mode)) {
		printf("%7d\t%17s\n", port_no, "-");
		return;
	}

	printf("%7d\t", port_no);
	switch (mode) {
	case EGRESS_UNTAG_ALL:
		printf("%17s\n", "all");
		break;
	case EGRESS_UNTAG_NONE:
		printf("%17s\n", "none");
		break;
	case EGRESS_UNTAG_PVID_AND_ZERO:
		printf("%17s\n", "PVID and 0");
		break;
	case EGRESS_UNTAG_ZERO:
		printf("%17s\n", "0");
		break;
	default:
		printf("%17s\n", "-");
	}
}
#endif

static void vsc9953_port_vlan_egr_untag_set(int port_no,
					    enum egress_untag_mode mode)
{
	struct vsc9953_rew_reg *l2rew_reg;

	/* Administrative down */
	if (!vsc9953_l2sw.port[port_no].enabled) {
		printf("Port %d is administrative down\n", port_no);
		return;
	}

	l2rew_reg = (struct vsc9953_rew_reg *)(VSC9953_OFFSET +
			VSC9953_REW_OFFSET);

	switch (mode) {
	case EGRESS_UNTAG_ALL:
		clrsetbits_le32(&l2rew_reg->port[port_no].port_tag_cfg,
				VSC9953_TAG_CFG_MASK, VSC9953_TAG_CFG_NONE);
		break;
	case EGRESS_UNTAG_PVID_AND_ZERO:
		clrsetbits_le32(&l2rew_reg->port[port_no].port_tag_cfg,
				VSC9953_TAG_CFG_MASK,
				VSC9953_TAG_CFG_ALL_BUT_PVID_ZERO);
		break;
	case EGRESS_UNTAG_ZERO:
		clrsetbits_le32(&l2rew_reg->port[port_no].port_tag_cfg,
				VSC9953_TAG_CFG_MASK,
				VSC9953_TAG_CFG_ALL_BUT_ZERO);
		break;
	case EGRESS_UNTAG_NONE:
		clrsetbits_le32(&l2rew_reg->port[port_no].port_tag_cfg,
				VSC9953_TAG_CFG_MASK, VSC9953_TAG_CFG_ALL);
		break;
	default:
		printf("Unknown untag mode for port %d\n", port_no);
	}
}

static void vsc9953_port_all_vlan_egress_untagged_set(
		enum egress_untag_mode mode)
{
	int i;

	for (i = 0; i < VSC9953_MAX_PORTS; i++)
		vsc9953_port_vlan_egr_untag_set(i, mode);
}

static int vsc9953_autoage_time_set(int age_period)
{
	u32 autoage;
	struct vsc9953_analyzer *l2ana_reg;

	l2ana_reg = (struct vsc9953_analyzer *)(VSC9953_OFFSET +
						VSC9953_ANA_OFFSET);

	if (age_period < 0 || age_period > VSC9953_AUTOAGE_PERIOD_MASK)
		return -EINVAL;

	autoage = bitfield_replace_by_mask(in_le32(&l2ana_reg->ana.auto_age),
					   VSC9953_AUTOAGE_PERIOD_MASK,
					   age_period);
	out_le32(&l2ana_reg->ana.auto_age, autoage);

	return 0;
}

#ifdef CONFIG_CMD_ETHSW

/* Enable/disable status of a VSC9953 port */
static void vsc9953_port_status_set(int port_no, u8 enabled)
{
	struct vsc9953_qsys_reg *l2qsys_reg;

	/* Administrative down */
	if (!vsc9953_l2sw.port[port_no].enabled)
		return;

	l2qsys_reg = (struct vsc9953_qsys_reg *)(VSC9953_OFFSET +
			VSC9953_QSYS_OFFSET);

	if (enabled)
		setbits_le32(&l2qsys_reg->sys.switch_port_mode[port_no],
			     VSC9953_PORT_ENA);
	else
		clrbits_le32(&l2qsys_reg->sys.switch_port_mode[port_no],
			     VSC9953_PORT_ENA);
}

/* Start autonegotiation for a VSC9953 PHY */
static void vsc9953_phy_autoneg(int port_no)
{
	if (!vsc9953_l2sw.port[port_no].phydev)
		return;

	if (vsc9953_l2sw.port[port_no].phydev->drv->startup(
			vsc9953_l2sw.port[port_no].phydev))
		printf("Failed to start PHY for port %d\n", port_no);
}

/* Print a VSC9953 port's configuration */
static void vsc9953_port_config_show(int port_no)
{
	int speed;
	int duplex;
	int link;
	u8 enabled;
	u32 val;
	struct vsc9953_qsys_reg *l2qsys_reg;

	l2qsys_reg = (struct vsc9953_qsys_reg *)(VSC9953_OFFSET +
			VSC9953_QSYS_OFFSET);

	val = in_le32(&l2qsys_reg->sys.switch_port_mode[port_no]);
	enabled = vsc9953_l2sw.port[port_no].enabled &&
		  (val & VSC9953_PORT_ENA);

	/* internal ports (8 and 9) are fixed */
	if (VSC9953_INTERNAL_PORT_CHECK(port_no)) {
		link = 1;
		speed = SPEED_2500;
		duplex = DUPLEX_FULL;
	} else {
		if (vsc9953_l2sw.port[port_no].phydev) {
			link = vsc9953_l2sw.port[port_no].phydev->link;
			speed = vsc9953_l2sw.port[port_no].phydev->speed;
			duplex = vsc9953_l2sw.port[port_no].phydev->duplex;
		} else {
			link = -1;
			speed = -1;
			duplex = -1;
		}
	}

	printf("%8d ", port_no);
	printf("%8s ", enabled == 1 ? "enabled" : "disabled");
	printf("%8s ", link == 1 ? "up" : "down");

	switch (speed) {
	case SPEED_10:
		printf("%8d ", 10);
		break;
	case SPEED_100:
		printf("%8d ", 100);
		break;
	case SPEED_1000:
		printf("%8d ", 1000);
		break;
	case SPEED_2500:
		printf("%8d ", 2500);
		break;
	case SPEED_10000:
		printf("%8d ", 10000);
		break;
	default:
		printf("%8s ", "-");
	}

	printf("%8s\n", duplex == DUPLEX_FULL ? "full" : "half");
}

/* Show VSC9953 ports' statistics */
static void vsc9953_port_statistics_show(int port_no)
{
	u32 rx_val;
	u32 tx_val;
	struct vsc9953_system_reg *l2sys_reg;

	/* Administrative down */
	if (!vsc9953_l2sw.port[port_no].enabled) {
		printf("Port %d is administrative down\n", port_no);
		return;
	}

	l2sys_reg = (struct vsc9953_system_reg *)(VSC9953_OFFSET +
			VSC9953_SYS_OFFSET);

	printf("Statistics for L2 Switch port %d:\n", port_no);

	/* Set counter view for our port */
	out_le32(&l2sys_reg->sys.stat_cfg, port_no);

#define VSC9953_STATS_PRINTF "%-15s %10u"

	/* Get number of Rx and Tx frames */
	rx_val = in_le32(&l2sys_reg->stat.rx_cntrs.c_rx_short) +
		 in_le32(&l2sys_reg->stat.rx_cntrs.c_rx_frag) +
		 in_le32(&l2sys_reg->stat.rx_cntrs.c_rx_jabber) +
		 in_le32(&l2sys_reg->stat.rx_cntrs.c_rx_long) +
		 in_le32(&l2sys_reg->stat.rx_cntrs.c_rx_sz_64) +
		 in_le32(&l2sys_reg->stat.rx_cntrs.c_rx_sz_65_127) +
		 in_le32(&l2sys_reg->stat.rx_cntrs.c_rx_sz_128_255) +
		 in_le32(&l2sys_reg->stat.rx_cntrs.c_rx_sz_256_511) +
		 in_le32(&l2sys_reg->stat.rx_cntrs.c_rx_sz_512_1023) +
		 in_le32(&l2sys_reg->stat.rx_cntrs.c_rx_sz_1024_1526) +
		 in_le32(&l2sys_reg->stat.rx_cntrs.c_rx_sz_jumbo);
	tx_val = in_le32(&l2sys_reg->stat.tx_cntrs.c_tx_sz_64) +
		 in_le32(&l2sys_reg->stat.tx_cntrs.c_tx_sz_65_127) +
		 in_le32(&l2sys_reg->stat.tx_cntrs.c_tx_sz_128_255) +
		 in_le32(&l2sys_reg->stat.tx_cntrs.c_tx_sz_256_511) +
		 in_le32(&l2sys_reg->stat.tx_cntrs.c_tx_sz_512_1023) +
		 in_le32(&l2sys_reg->stat.tx_cntrs.c_tx_sz_1024_1526) +
		 in_le32(&l2sys_reg->stat.tx_cntrs.c_tx_sz_jumbo);
	printf(VSC9953_STATS_PRINTF"\t\t"VSC9953_STATS_PRINTF"\n",
	       "Rx frames:", rx_val, "Tx frames:", tx_val);

	/* Get number of Rx and Tx bytes */
	rx_val = in_le32(&l2sys_reg->stat.rx_cntrs.c_rx_oct);
	tx_val = in_le32(&l2sys_reg->stat.tx_cntrs.c_tx_oct);
	printf(VSC9953_STATS_PRINTF"\t\t"VSC9953_STATS_PRINTF"\n",
	       "Rx bytes:", rx_val, "Tx bytes:", tx_val);

	/* Get number of Rx frames received ok and Tx frames sent ok */
	rx_val = in_le32(&l2sys_reg->stat.rx_cntrs.c_rx_yellow_prio_0) +
		 in_le32(&l2sys_reg->stat.rx_cntrs.c_rx_yellow_prio_1) +
		 in_le32(&l2sys_reg->stat.rx_cntrs.c_rx_yellow_prio_2) +
		 in_le32(&l2sys_reg->stat.rx_cntrs.c_rx_yellow_prio_3) +
		 in_le32(&l2sys_reg->stat.rx_cntrs.c_rx_yellow_prio_4) +
		 in_le32(&l2sys_reg->stat.rx_cntrs.c_rx_yellow_prio_5) +
		 in_le32(&l2sys_reg->stat.rx_cntrs.c_rx_yellow_prio_6) +
		 in_le32(&l2sys_reg->stat.rx_cntrs.c_rx_yellow_prio_7) +
		 in_le32(&l2sys_reg->stat.rx_cntrs.c_rx_green_prio_0) +
		 in_le32(&l2sys_reg->stat.rx_cntrs.c_rx_green_prio_1) +
		 in_le32(&l2sys_reg->stat.rx_cntrs.c_rx_green_prio_2) +
		 in_le32(&l2sys_reg->stat.rx_cntrs.c_rx_green_prio_3) +
		 in_le32(&l2sys_reg->stat.rx_cntrs.c_rx_green_prio_4) +
		 in_le32(&l2sys_reg->stat.rx_cntrs.c_rx_green_prio_5) +
		 in_le32(&l2sys_reg->stat.rx_cntrs.c_rx_green_prio_6) +
		 in_le32(&l2sys_reg->stat.rx_cntrs.c_rx_green_prio_7);
	tx_val = in_le32(&l2sys_reg->stat.tx_cntrs.c_tx_sz_64) +
		 in_le32(&l2sys_reg->stat.tx_cntrs.c_tx_sz_65_127) +
		 in_le32(&l2sys_reg->stat.tx_cntrs.c_tx_sz_128_255) +
		 in_le32(&l2sys_reg->stat.tx_cntrs.c_tx_sz_256_511) +
		 in_le32(&l2sys_reg->stat.tx_cntrs.c_tx_sz_512_1023) +
		 in_le32(&l2sys_reg->stat.tx_cntrs.c_tx_sz_1024_1526) +
		 in_le32(&l2sys_reg->stat.tx_cntrs.c_tx_sz_jumbo);
	printf(VSC9953_STATS_PRINTF"\t\t"VSC9953_STATS_PRINTF"\n",
	       "Rx frames ok:", rx_val, "Tx frames ok:", tx_val);

	/* Get number of Rx and Tx unicast frames */
	rx_val = in_le32(&l2sys_reg->stat.rx_cntrs.c_rx_uc);
	tx_val = in_le32(&l2sys_reg->stat.tx_cntrs.c_tx_uc);
	printf(VSC9953_STATS_PRINTF"\t\t"VSC9953_STATS_PRINTF"\n",
	       "Rx unicast:", rx_val, "Tx unicast:", tx_val);

	/* Get number of Rx and Tx broadcast frames */
	rx_val = in_le32(&l2sys_reg->stat.rx_cntrs.c_rx_bc);
	tx_val = in_le32(&l2sys_reg->stat.tx_cntrs.c_tx_bc);
	printf(VSC9953_STATS_PRINTF"\t\t"VSC9953_STATS_PRINTF"\n",
	       "Rx broadcast:", rx_val, "Tx broadcast:", tx_val);

	/* Get number of Rx and Tx frames of 64B */
	rx_val = in_le32(&l2sys_reg->stat.rx_cntrs.c_rx_sz_64);
	tx_val = in_le32(&l2sys_reg->stat.tx_cntrs.c_tx_sz_64);
	printf(VSC9953_STATS_PRINTF"\t\t"VSC9953_STATS_PRINTF"\n",
	       "Rx 64B:", rx_val, "Tx 64B:", tx_val);

	/* Get number of Rx and Tx frames with sizes between 65B and 127B */
	rx_val = in_le32(&l2sys_reg->stat.rx_cntrs.c_rx_sz_65_127);
	tx_val = in_le32(&l2sys_reg->stat.tx_cntrs.c_tx_sz_65_127);
	printf(VSC9953_STATS_PRINTF"\t\t"VSC9953_STATS_PRINTF"\n",
	       "Rx 65B-127B:", rx_val, "Tx 65B-127B:", tx_val);

	/* Get number of Rx and Tx frames with sizes between 128B and 255B */
	rx_val = in_le32(&l2sys_reg->stat.rx_cntrs.c_rx_sz_128_255);
	tx_val = in_le32(&l2sys_reg->stat.tx_cntrs.c_tx_sz_128_255);
	printf(VSC9953_STATS_PRINTF"\t\t"VSC9953_STATS_PRINTF"\n",
	       "Rx 128B-255B:", rx_val, "Tx 128B-255B:", tx_val);

	/* Get number of Rx and Tx frames with sizes between 256B and 511B */
	rx_val = in_le32(&l2sys_reg->stat.rx_cntrs.c_rx_sz_256_511);
	tx_val = in_le32(&l2sys_reg->stat.tx_cntrs.c_tx_sz_256_511);
	printf(VSC9953_STATS_PRINTF"\t\t"VSC9953_STATS_PRINTF"\n",
	       "Rx 256B-511B:", rx_val, "Tx 256B-511B:", tx_val);

	/* Get number of Rx and Tx frames with sizes between 512B and 1023B */
	rx_val = in_le32(&l2sys_reg->stat.rx_cntrs.c_rx_sz_512_1023);
	tx_val = in_le32(&l2sys_reg->stat.tx_cntrs.c_tx_sz_512_1023);
	printf(VSC9953_STATS_PRINTF"\t\t"VSC9953_STATS_PRINTF"\n",
	       "Rx 512B-1023B:", rx_val, "Tx 512B-1023B:", tx_val);

	/* Get number of Rx and Tx frames with sizes between 1024B and 1526B */
	rx_val = in_le32(&l2sys_reg->stat.rx_cntrs.c_rx_sz_1024_1526);
	tx_val = in_le32(&l2sys_reg->stat.tx_cntrs.c_tx_sz_1024_1526);
	printf(VSC9953_STATS_PRINTF"\t\t"VSC9953_STATS_PRINTF"\n",
	       "Rx 1024B-1526B:", rx_val, "Tx 1024B-1526B:", tx_val);

	/* Get number of Rx and Tx jumbo frames */
	rx_val = in_le32(&l2sys_reg->stat.rx_cntrs.c_rx_sz_jumbo);
	tx_val = in_le32(&l2sys_reg->stat.tx_cntrs.c_tx_sz_jumbo);
	printf(VSC9953_STATS_PRINTF"\t\t"VSC9953_STATS_PRINTF"\n",
	       "Rx jumbo:", rx_val, "Tx jumbo:", tx_val);

	/* Get number of Rx and Tx dropped frames */
	rx_val = in_le32(&l2sys_reg->stat.rx_cntrs.c_rx_cat_drop) +
		 in_le32(&l2sys_reg->stat.drop_cntrs.c_dr_tail) +
		 in_le32(&l2sys_reg->stat.drop_cntrs.c_dr_yellow_prio_0) +
		 in_le32(&l2sys_reg->stat.drop_cntrs.c_dr_yellow_prio_1) +
		 in_le32(&l2sys_reg->stat.drop_cntrs.c_dr_yellow_prio_2) +
		 in_le32(&l2sys_reg->stat.drop_cntrs.c_dr_yellow_prio_3) +
		 in_le32(&l2sys_reg->stat.drop_cntrs.c_dr_yellow_prio_4) +
		 in_le32(&l2sys_reg->stat.drop_cntrs.c_dr_yellow_prio_5) +
		 in_le32(&l2sys_reg->stat.drop_cntrs.c_dr_yellow_prio_6) +
		 in_le32(&l2sys_reg->stat.drop_cntrs.c_dr_yellow_prio_7) +
		 in_le32(&l2sys_reg->stat.drop_cntrs.c_dr_green_prio_0) +
		 in_le32(&l2sys_reg->stat.drop_cntrs.c_dr_green_prio_1) +
		 in_le32(&l2sys_reg->stat.drop_cntrs.c_dr_green_prio_2) +
		 in_le32(&l2sys_reg->stat.drop_cntrs.c_dr_green_prio_3) +
		 in_le32(&l2sys_reg->stat.drop_cntrs.c_dr_green_prio_4) +
		 in_le32(&l2sys_reg->stat.drop_cntrs.c_dr_green_prio_5) +
		 in_le32(&l2sys_reg->stat.drop_cntrs.c_dr_green_prio_6) +
		 in_le32(&l2sys_reg->stat.drop_cntrs.c_dr_green_prio_7);
	tx_val = in_le32(&l2sys_reg->stat.tx_cntrs.c_tx_drop) +
		 in_le32(&l2sys_reg->stat.tx_cntrs.c_tx_aged);
	printf(VSC9953_STATS_PRINTF"\t\t"VSC9953_STATS_PRINTF"\n",
	       "Rx drops:", rx_val, "Tx drops:", tx_val);

	/*
	 * Get number of Rx frames with CRC or alignment errors
	 * and number of detected Tx collisions
	 */
	rx_val = in_le32(&l2sys_reg->stat.rx_cntrs.c_rx_crc);
	tx_val = in_le32(&l2sys_reg->stat.tx_cntrs.c_tx_col);
	printf(VSC9953_STATS_PRINTF"\t\t"VSC9953_STATS_PRINTF"\n",
	       "Rx CRC&align:", rx_val, "Tx coll:", tx_val);

	/*
	 * Get number of Rx undersized frames and
	 * number of Tx aged frames
	 */
	rx_val = in_le32(&l2sys_reg->stat.rx_cntrs.c_rx_short);
	tx_val = in_le32(&l2sys_reg->stat.tx_cntrs.c_tx_aged);
	printf(VSC9953_STATS_PRINTF"\t\t"VSC9953_STATS_PRINTF"\n",
	       "Rx undersize:", rx_val, "Tx aged:", tx_val);

	/* Get number of Rx oversized frames */
	rx_val = in_le32(&l2sys_reg->stat.rx_cntrs.c_rx_long);
	printf(VSC9953_STATS_PRINTF"\n", "Rx oversized:", rx_val);

	/* Get number of Rx fragmented frames */
	rx_val = in_le32(&l2sys_reg->stat.rx_cntrs.c_rx_frag);
	printf(VSC9953_STATS_PRINTF"\n", "Rx fragments:", rx_val);

	/* Get number of Rx jabber errors */
	rx_val = in_le32(&l2sys_reg->stat.rx_cntrs.c_rx_jabber);
	printf(VSC9953_STATS_PRINTF"\n", "Rx jabbers:", rx_val);

	/*
	 * Get number of Rx frames filtered due to classification rules or
	 * no destination ports
	 */
	rx_val = in_le32(&l2sys_reg->stat.rx_cntrs.c_rx_cat_drop) +
		 in_le32(&l2sys_reg->stat.drop_cntrs.c_dr_local);
	printf(VSC9953_STATS_PRINTF"\n", "Rx filtered:", rx_val);

	printf("\n");
}

/* Clear statistics for a VSC9953 port */
static void vsc9953_port_statistics_clear(int port_no)
{
	struct vsc9953_system_reg *l2sys_reg;

	/* Administrative down */
	if (!vsc9953_l2sw.port[port_no].enabled) {
		printf("Port %d is administrative down\n", port_no);
		return;
	}

	l2sys_reg = (struct vsc9953_system_reg *)(VSC9953_OFFSET +
			VSC9953_SYS_OFFSET);

	/* Clear all counter groups for our ports */
	out_le32(&l2sys_reg->sys.stat_cfg, port_no |
		 VSC9953_STAT_CLEAR_RX | VSC9953_STAT_CLEAR_TX |
		 VSC9953_STAT_CLEAR_DR);
}

enum port_learn_mode {
	PORT_LEARN_NONE,
	PORT_LEARN_AUTO
};

/* Set learning configuration for a VSC9953 port */
static void vsc9953_port_learn_mode_set(int port_no, enum port_learn_mode mode)
{
	struct vsc9953_analyzer *l2ana_reg;

	/* Administrative down */
	if (!vsc9953_l2sw.port[port_no].enabled) {
		printf("Port %d is administrative down\n", port_no);
		return;
	}

	l2ana_reg = (struct vsc9953_analyzer *)(VSC9953_OFFSET +
			VSC9953_ANA_OFFSET);

	switch (mode) {
	case PORT_LEARN_NONE:
		clrbits_le32(&l2ana_reg->port[port_no].port_cfg,
			     VSC9953_PORT_CFG_LEARN_DROP |
			     VSC9953_PORT_CFG_LEARN_CPU |
			     VSC9953_PORT_CFG_LEARN_AUTO |
			     VSC9953_PORT_CFG_LEARN_ENA);
		break;
	case PORT_LEARN_AUTO:
		clrsetbits_le32(&l2ana_reg->port[port_no].port_cfg,
				VSC9953_PORT_CFG_LEARN_DROP |
				VSC9953_PORT_CFG_LEARN_CPU,
				VSC9953_PORT_CFG_LEARN_ENA |
				VSC9953_PORT_CFG_LEARN_AUTO);
		break;
	default:
		printf("Unknown learn mode for port %d\n", port_no);
	}
}

/* Get learning configuration for a VSC9953 port */
static int vsc9953_port_learn_mode_get(int port_no, enum port_learn_mode *mode)
{
	u32 val;
	struct vsc9953_analyzer *l2ana_reg;

	/* Administrative down */
	if (!vsc9953_l2sw.port[port_no].enabled) {
		printf("Port %d is administrative down\n", port_no);
		return -1;
	}

	l2ana_reg = (struct vsc9953_analyzer *)(VSC9953_OFFSET +
			VSC9953_ANA_OFFSET);

	/* For now we only support HW learning (auto) and no learning */
	val = in_le32(&l2ana_reg->port[port_no].port_cfg);
	if ((val & (VSC9953_PORT_CFG_LEARN_ENA |
		    VSC9953_PORT_CFG_LEARN_AUTO)) ==
	    (VSC9953_PORT_CFG_LEARN_ENA | VSC9953_PORT_CFG_LEARN_AUTO))
		*mode = PORT_LEARN_AUTO;
	else
		*mode = PORT_LEARN_NONE;

	return 0;
}

/* wait for FDB to become available */
static int vsc9953_mac_table_poll_idle(void)
{
	struct vsc9953_analyzer *l2ana_reg;
	u32 timeout;

	l2ana_reg = (struct vsc9953_analyzer *)(VSC9953_OFFSET +
			VSC9953_ANA_OFFSET);

	timeout = 50000;
	while (((in_le32(&l2ana_reg->ana_tables.mac_access) &
			 VSC9953_MAC_CMD_MASK) !=
		VSC9953_MAC_CMD_IDLE) && --timeout)
		udelay(1);

	return timeout ? 0 : -EBUSY;
}

/* enum describing available commands for the MAC table */
enum mac_table_cmd {
	MAC_TABLE_READ,
	MAC_TABLE_LOOKUP,
	MAC_TABLE_WRITE,
	MAC_TABLE_LEARN,
	MAC_TABLE_FORGET,
	MAC_TABLE_GET_NEXT,
	MAC_TABLE_AGE,
};

/* Issues a command to the FDB table */
static int vsc9953_mac_table_cmd(enum mac_table_cmd cmd)
{
	struct vsc9953_analyzer *l2ana_reg;

	l2ana_reg = (struct vsc9953_analyzer *)(VSC9953_OFFSET +
			VSC9953_ANA_OFFSET);

	switch (cmd) {
	case MAC_TABLE_READ:
		clrsetbits_le32(&l2ana_reg->ana_tables.mac_access,
				VSC9953_MAC_CMD_MASK | VSC9953_MAC_CMD_VALID,
				VSC9953_MAC_CMD_READ);
		break;
	case MAC_TABLE_LOOKUP:
		clrsetbits_le32(&l2ana_reg->ana_tables.mac_access,
				VSC9953_MAC_CMD_MASK, VSC9953_MAC_CMD_READ |
				VSC9953_MAC_CMD_VALID);
		break;
	case MAC_TABLE_WRITE:
		clrsetbits_le32(&l2ana_reg->ana_tables.mac_access,
				VSC9953_MAC_CMD_MASK |
				VSC9953_MAC_ENTRYTYPE_MASK,
				VSC9953_MAC_CMD_WRITE |
				VSC9953_MAC_ENTRYTYPE_LOCKED);
		break;
	case MAC_TABLE_LEARN:
		clrsetbits_le32(&l2ana_reg->ana_tables.mac_access,
				VSC9953_MAC_CMD_MASK |
				VSC9953_MAC_ENTRYTYPE_MASK,
				VSC9953_MAC_CMD_LEARN |
				VSC9953_MAC_ENTRYTYPE_LOCKED |
				VSC9953_MAC_CMD_VALID);
		break;
	case MAC_TABLE_FORGET:
		clrsetbits_le32(&l2ana_reg->ana_tables.mac_access,
				VSC9953_MAC_CMD_MASK |
				VSC9953_MAC_ENTRYTYPE_MASK,
				VSC9953_MAC_CMD_FORGET);
		break;
	case MAC_TABLE_GET_NEXT:
		clrsetbits_le32(&l2ana_reg->ana_tables.mac_access,
				VSC9953_MAC_CMD_MASK |
				VSC9953_MAC_ENTRYTYPE_MASK,
				VSC9953_MAC_CMD_NEXT);
		break;
	case MAC_TABLE_AGE:
		clrsetbits_le32(&l2ana_reg->ana_tables.mac_access,
				VSC9953_MAC_CMD_MASK |
				VSC9953_MAC_ENTRYTYPE_MASK,
				VSC9953_MAC_CMD_AGE);
		break;
	default:
		printf("Unknown MAC table command\n");
	}

	if (vsc9953_mac_table_poll_idle() < 0) {
		debug("MAC table timeout\n");
		return -1;
	}

	return 0;
}

/* show the FDB entries that correspond to a port and a VLAN */
static void vsc9953_mac_table_show(int port_no, int vid)
{
	int rc[VSC9953_MAX_PORTS];
	enum port_learn_mode mode[VSC9953_MAX_PORTS];
	int i;
	u32 val;
	u32 vlan;
	u32 mach;
	u32 macl;
	u32 dest_indx;
	struct vsc9953_analyzer *l2ana_reg;

	l2ana_reg = (struct vsc9953_analyzer *)(VSC9953_OFFSET +
			VSC9953_ANA_OFFSET);

	/* disable auto learning */
	if (port_no == ETHSW_CMD_PORT_ALL) {
		for (i = 0; i < VSC9953_MAX_PORTS; i++) {
			rc[i] = vsc9953_port_learn_mode_get(i, &mode[i]);
			if (!rc[i] && mode[i] != PORT_LEARN_NONE)
				vsc9953_port_learn_mode_set(i, PORT_LEARN_NONE);
		}
	} else {
		rc[port_no] = vsc9953_port_learn_mode_get(port_no,
							  &mode[port_no]);
		if (!rc[port_no] && mode[port_no] != PORT_LEARN_NONE)
			vsc9953_port_learn_mode_set(port_no, PORT_LEARN_NONE);
	}

	/* write port and vid to get selected FDB entries */
	val = in_le32(&l2ana_reg->ana.anag_efil);
	if (port_no != ETHSW_CMD_PORT_ALL) {
		val = bitfield_replace_by_mask(val, VSC9953_AGE_PORT_MASK,
					       port_no) | VSC9953_AGE_PORT_EN;
	}
	if (vid != ETHSW_CMD_VLAN_ALL) {
		val = bitfield_replace_by_mask(val, VSC9953_AGE_VID_MASK,
					       vid) | VSC9953_AGE_VID_EN;
	}
	out_le32(&l2ana_reg->ana.anag_efil, val);

	/* set MAC and VLAN to 0 to look from beginning */
	clrbits_le32(&l2ana_reg->ana_tables.mach_data,
		     VSC9953_MAC_VID_MASK | VSC9953_MAC_MACH_MASK);
	out_le32(&l2ana_reg->ana_tables.macl_data, 0);

	/* get entries */
	printf("%10s %17s %5s %4s\n", "EntryType", "MAC", "PORT", "VID");
	do {
		if (vsc9953_mac_table_cmd(MAC_TABLE_GET_NEXT) < 0) {
			debug("GET NEXT MAC table command failed\n");
			break;
		}

		val = in_le32(&l2ana_reg->ana_tables.mac_access);

		/* get out when an invalid entry is found */
		if (!(val & VSC9953_MAC_CMD_VALID))
			break;

		switch (val & VSC9953_MAC_ENTRYTYPE_MASK) {
		case VSC9953_MAC_ENTRYTYPE_NORMAL:
			printf("%10s ", "Dynamic");
			break;
		case VSC9953_MAC_ENTRYTYPE_LOCKED:
			printf("%10s ", "Static");
			break;
		case VSC9953_MAC_ENTRYTYPE_IPV4MCAST:
			printf("%10s ", "IPv4 Mcast");
			break;
		case VSC9953_MAC_ENTRYTYPE_IPV6MCAST:
			printf("%10s ", "IPv6 Mcast");
			break;
		default:
			printf("%10s ", "Unknown");
		}

		dest_indx = bitfield_extract_by_mask(val,
						     VSC9953_MAC_DESTIDX_MASK);

		val = in_le32(&l2ana_reg->ana_tables.mach_data);
		vlan = bitfield_extract_by_mask(val, VSC9953_MAC_VID_MASK);
		mach = bitfield_extract_by_mask(val, VSC9953_MAC_MACH_MASK);
		macl = in_le32(&l2ana_reg->ana_tables.macl_data);

		printf("%02x:%02x:%02x:%02x:%02x:%02x ", (mach >> 8) & 0xff,
		       mach & 0xff, (macl >> 24) & 0xff, (macl >> 16) & 0xff,
		       (macl >> 8) & 0xff, macl & 0xff);
		printf("%5d ", dest_indx);
		printf("%4d\n", vlan);
	} while (1);

	/* set learning mode to previous value */
	if (port_no == ETHSW_CMD_PORT_ALL) {
		for (i = 0; i < VSC9953_MAX_PORTS; i++) {
			if (!rc[i] && mode[i] != PORT_LEARN_NONE)
				vsc9953_port_learn_mode_set(i, mode[i]);
		}
	} else {
		/* If administrative down, skip */
		if (!rc[port_no] && mode[port_no] != PORT_LEARN_NONE)
			vsc9953_port_learn_mode_set(port_no, mode[port_no]);
	}

	/* reset FDB port and VLAN FDB selection */
	clrbits_le32(&l2ana_reg->ana.anag_efil, VSC9953_AGE_PORT_EN |
		     VSC9953_AGE_PORT_MASK | VSC9953_AGE_VID_EN |
		     VSC9953_AGE_VID_MASK);
}

/* Add a static FDB entry */
static int vsc9953_mac_table_add(u8 port_no, uchar mac[6], int vid)
{
	u32 val;
	struct vsc9953_analyzer *l2ana_reg;

	l2ana_reg = (struct vsc9953_analyzer *)(VSC9953_OFFSET +
			VSC9953_ANA_OFFSET);

	val = in_le32(&l2ana_reg->ana_tables.mach_data);
	val = bitfield_replace_by_mask(val, VSC9953_MACHDATA_VID_MASK, vid) |
	      (mac[0] << 8) | (mac[1] << 0);
	out_le32(&l2ana_reg->ana_tables.mach_data, val);

	out_le32(&l2ana_reg->ana_tables.macl_data,
		 (mac[2] << 24) | (mac[3] << 16) | (mac[4] << 8) |
		 (mac[5] << 0));

	/* set on which port is the MAC address added */
	val = in_le32(&l2ana_reg->ana_tables.mac_access);
	val = bitfield_replace_by_mask(val, VSC9953_MAC_DESTIDX_MASK, port_no);
	out_le32(&l2ana_reg->ana_tables.mac_access, val);

	if (vsc9953_mac_table_cmd(MAC_TABLE_LEARN) < 0)
		return -1;

	/* check if the MAC address was indeed added */
	val = in_le32(&l2ana_reg->ana_tables.mach_data);
	val = bitfield_replace_by_mask(val, VSC9953_MACHDATA_VID_MASK, vid) |
	      (mac[0] << 8) | (mac[1] << 0);
	out_le32(&l2ana_reg->ana_tables.mach_data, val);

	out_le32(&l2ana_reg->ana_tables.macl_data,
		 (mac[2] << 24) | (mac[3] << 16) | (mac[4] << 8) |
		 (mac[5] << 0));

	if (vsc9953_mac_table_cmd(MAC_TABLE_READ) < 0)
		return -1;

	val = in_le32(&l2ana_reg->ana_tables.mac_access);

	if ((port_no != bitfield_extract_by_mask(val,
						 VSC9953_MAC_DESTIDX_MASK))) {
		printf("Failed to add MAC address\n");
		return -1;
	}
	return 0;
}

/* Delete a FDB entry */
static int vsc9953_mac_table_del(uchar mac[6], u16 vid)
{
	u32 val;
	struct vsc9953_analyzer *l2ana_reg;

	l2ana_reg = (struct vsc9953_analyzer *)(VSC9953_OFFSET +
			VSC9953_ANA_OFFSET);

	/* check first if MAC entry is present */
	val = in_le32(&l2ana_reg->ana_tables.mach_data);
	val = bitfield_replace_by_mask(val, VSC9953_MACHDATA_VID_MASK, vid) |
	      (mac[0] << 8) | (mac[1] << 0);
	out_le32(&l2ana_reg->ana_tables.mach_data, val);

	out_le32(&l2ana_reg->ana_tables.macl_data,
		 (mac[2] << 24) | (mac[3] << 16) | (mac[4] << 8) |
		 (mac[5] << 0));

	if (vsc9953_mac_table_cmd(MAC_TABLE_LOOKUP) < 0) {
		debug("Lookup in the MAC table failed\n");
		return -1;
	}

	if (!(in_le32(&l2ana_reg->ana_tables.mac_access) &
	      VSC9953_MAC_CMD_VALID)) {
		printf("The MAC address: %02x:%02x:%02x:%02x:%02x:%02x ",
		       mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
		printf("VLAN: %d does not exist.\n", vid);
		return -1;
	}

	/* FDB entry found, proceed to delete */
	val = in_le32(&l2ana_reg->ana_tables.mach_data);
	val = bitfield_replace_by_mask(val, VSC9953_MACHDATA_VID_MASK, vid) |
	      (mac[0] << 8) | (mac[1] << 0);
	out_le32(&l2ana_reg->ana_tables.mach_data, val);

	out_le32(&l2ana_reg->ana_tables.macl_data, (mac[2] << 24) |
		 (mac[3] << 16) | (mac[4] << 8) | (mac[5] << 0));

	if (vsc9953_mac_table_cmd(MAC_TABLE_FORGET) < 0)
		return -1;

	/* check if the MAC entry is still in FDB */
	val = in_le32(&l2ana_reg->ana_tables.mach_data);
	val = bitfield_replace_by_mask(val, VSC9953_MACHDATA_VID_MASK, vid) |
	      (mac[0] << 8) | (mac[1] << 0);
	out_le32(&l2ana_reg->ana_tables.mach_data, val);

	out_le32(&l2ana_reg->ana_tables.macl_data, (mac[2] << 24) |
		 (mac[3] << 16) | (mac[4] << 8) | (mac[5] << 0));

	if (vsc9953_mac_table_cmd(MAC_TABLE_LOOKUP) < 0) {
		debug("Lookup in the MAC table failed\n");
		return -1;
	}
	if (in_le32(&l2ana_reg->ana_tables.mac_access) &
	    VSC9953_MAC_CMD_VALID) {
		printf("Failed to delete MAC address\n");
		return -1;
	}

	return 0;
}

/* age the unlocked entries in FDB */
static void vsc9953_mac_table_age(int port_no, int vid)
{
	int rc[VSC9953_MAX_PORTS];
	enum port_learn_mode mode[VSC9953_MAX_PORTS];
	u32 val;
	int i;
	struct vsc9953_analyzer *l2ana_reg;

	l2ana_reg = (struct vsc9953_analyzer *)(VSC9953_OFFSET +
			VSC9953_ANA_OFFSET);

	/* set port and VID for selective aging */
	val = in_le32(&l2ana_reg->ana.anag_efil);
	if (port_no != ETHSW_CMD_PORT_ALL) {
		/* disable auto learning */
		rc[port_no] = vsc9953_port_learn_mode_get(port_no,
							  &mode[port_no]);
		if (!rc[port_no] && mode[port_no] != PORT_LEARN_NONE)
			vsc9953_port_learn_mode_set(port_no, PORT_LEARN_NONE);

		val = bitfield_replace_by_mask(val, VSC9953_AGE_PORT_MASK,
					       port_no) | VSC9953_AGE_PORT_EN;
	} else {
		/* disable auto learning on all ports */
		for (i = 0; i < VSC9953_MAX_PORTS; i++) {
			rc[i] = vsc9953_port_learn_mode_get(i, &mode[i]);
			if (!rc[i] && mode[i] != PORT_LEARN_NONE)
				vsc9953_port_learn_mode_set(i, PORT_LEARN_NONE);
		}
	}

	if (vid != ETHSW_CMD_VLAN_ALL) {
		val = bitfield_replace_by_mask(val, VSC9953_AGE_VID_MASK, vid) |
		      VSC9953_AGE_VID_EN;
	}
	out_le32(&l2ana_reg->ana.anag_efil, val);

	/* age the dynamic FDB entries */
	vsc9953_mac_table_cmd(MAC_TABLE_AGE);

	/* clear previously set port and VID */
	clrbits_le32(&l2ana_reg->ana.anag_efil, VSC9953_AGE_PORT_EN |
		     VSC9953_AGE_PORT_MASK | VSC9953_AGE_VID_EN |
		     VSC9953_AGE_VID_MASK);

	if (port_no != ETHSW_CMD_PORT_ALL) {
		if (!rc[port_no] && mode[port_no] != PORT_LEARN_NONE)
			vsc9953_port_learn_mode_set(port_no, mode[port_no]);
	} else {
		for (i = 0; i < VSC9953_MAX_PORTS; i++) {
			if (!rc[i] && mode[i] != PORT_LEARN_NONE)
				vsc9953_port_learn_mode_set(i, mode[i]);
		}
	}
}

/* Delete all the dynamic FDB entries */
static void vsc9953_mac_table_flush(int port, int vid)
{
	vsc9953_mac_table_age(port, vid);
	vsc9953_mac_table_age(port, vid);
}

enum egress_vlan_tag {
	EGR_TAG_CLASS = 0,
	EGR_TAG_PVID,
};

/* Set egress tag mode for a VSC9953 port */
static void vsc9953_port_vlan_egress_tag_set(int port_no,
					     enum egress_vlan_tag mode)
{
	struct vsc9953_rew_reg *l2rew_reg;

	l2rew_reg = (struct vsc9953_rew_reg *)(VSC9953_OFFSET +
			VSC9953_REW_OFFSET);

	switch (mode) {
	case EGR_TAG_CLASS:
		clrbits_le32(&l2rew_reg->port[port_no].port_tag_cfg,
			     VSC9953_TAG_VID_PVID);
		break;
	case EGR_TAG_PVID:
		setbits_le32(&l2rew_reg->port[port_no].port_tag_cfg,
			     VSC9953_TAG_VID_PVID);
		break;
	default:
		printf("Unknown egress VLAN tag mode for port %d\n", port_no);
	}
}

/* Get egress tag mode for a VSC9953 port */
static void vsc9953_port_vlan_egress_tag_get(int port_no,
					     enum egress_vlan_tag *mode)
{
	u32 val;
	struct vsc9953_rew_reg *l2rew_reg;

	l2rew_reg = (struct vsc9953_rew_reg *)(VSC9953_OFFSET +
			VSC9953_REW_OFFSET);

	val = in_le32(&l2rew_reg->port[port_no].port_tag_cfg);
	if (val & VSC9953_TAG_VID_PVID)
		*mode = EGR_TAG_PVID;
	else
		*mode = EGR_TAG_CLASS;
}

/* VSC9953 VLAN learning modes */
enum vlan_learning_mode {
	SHARED_VLAN_LEARNING,
	PRIVATE_VLAN_LEARNING,
};

/* Set VLAN learning mode for VSC9953 */
static void vsc9953_vlan_learning_set(enum vlan_learning_mode lrn_mode)
{
	struct vsc9953_analyzer *l2ana_reg;

	l2ana_reg = (struct vsc9953_analyzer *)(VSC9953_OFFSET +
			VSC9953_ANA_OFFSET);

	switch (lrn_mode) {
	case SHARED_VLAN_LEARNING:
		setbits_le32(&l2ana_reg->ana.agen_ctrl, VSC9953_FID_MASK_ALL);
		break;
	case PRIVATE_VLAN_LEARNING:
		clrbits_le32(&l2ana_reg->ana.agen_ctrl, VSC9953_FID_MASK_ALL);
		break;
	default:
		printf("Unknown VLAN learn mode\n");
	}
}

/* Get VLAN learning mode for VSC9953 */
static int vsc9953_vlan_learning_get(enum vlan_learning_mode *lrn_mode)
{
	u32 val;
	struct vsc9953_analyzer *l2ana_reg;

	l2ana_reg = (struct vsc9953_analyzer *)(VSC9953_OFFSET +
			VSC9953_ANA_OFFSET);

	val = in_le32(&l2ana_reg->ana.agen_ctrl);

	if (!(val & VSC9953_FID_MASK_ALL)) {
		*lrn_mode = PRIVATE_VLAN_LEARNING;
	} else if ((val & VSC9953_FID_MASK_ALL) == VSC9953_FID_MASK_ALL) {
		*lrn_mode = SHARED_VLAN_LEARNING;
	} else {
		printf("Unknown VLAN learning mode\n");
		return -EINVAL;
	}

	return 0;
}

/* Enable/disable VLAN ingress filtering on a VSC9953 port */
static void vsc9953_port_ingress_filtering_set(int port_no, int enabled)
{
	struct vsc9953_analyzer *l2ana_reg;

	l2ana_reg = (struct vsc9953_analyzer *)(VSC9953_OFFSET +
			VSC9953_ANA_OFFSET);

	if (enabled)
		setbits_le32(&l2ana_reg->ana.vlan_mask, 1 << port_no);
	else
		clrbits_le32(&l2ana_reg->ana.vlan_mask, 1 << port_no);
}

/* Return VLAN ingress filtering on a VSC9953 port */
static int vsc9953_port_ingress_filtering_get(int port_no)
{
	u32 val;
	struct vsc9953_analyzer *l2ana_reg;

	l2ana_reg = (struct vsc9953_analyzer *)(VSC9953_OFFSET +
			VSC9953_ANA_OFFSET);

	val = in_le32(&l2ana_reg->ana.vlan_mask);
	return !!(val & (1 << port_no));
}

/* Get the aggregation group of a port */
static int vsc9953_port_aggr_grp_get(int port_no, int *aggr_grp)
{
	u32 val;
	struct vsc9953_analyzer *l2ana_reg;

	if (!VSC9953_PORT_CHECK(port_no))
		return -EINVAL;

	l2ana_reg = (struct vsc9953_analyzer *)(VSC9953_OFFSET +
						VSC9953_ANA_OFFSET);

	val = in_le32(&l2ana_reg->port[port_no].port_cfg);
	*aggr_grp = bitfield_extract_by_mask(val,
					     VSC9953_PORT_CFG_PORTID_MASK);

	return 0;
}

static void vsc9953_aggr_grp_members_get(int aggr_grp,
					 u8 aggr_membr[VSC9953_MAX_PORTS])
{
	int port_no;
	int aggr_membr_grp;

	for (port_no = 0; port_no < VSC9953_MAX_PORTS; port_no++) {
		aggr_membr[port_no] = 0;

		if (vsc9953_port_aggr_grp_get(port_no, &aggr_membr_grp))
			continue;

		if (aggr_grp == aggr_membr_grp)
			aggr_membr[port_no] = 1;
	}
}

static void vsc9953_update_dest_members_masks(int port_no, u32 membr_bitfld_old,
					      u32 membr_bitfld_new)
{
	int i;
	u32 pgid;
	struct vsc9953_analyzer *l2ana_reg;

	l2ana_reg = (struct vsc9953_analyzer *)(VSC9953_OFFSET +
						VSC9953_ANA_OFFSET);

	/*
	 * NOTE: Only the unicast destination masks are updated, since
	 * we do not support for now Layer-2 multicast entries
	 */
	for (i = 0; i < VSC9953_MAX_PORTS; i++) {
		if (i == port_no) {
			clrsetbits_le32(&l2ana_reg->port_id_tbl.port_grp_id[i],
					VSC9953_PGID_PORT_MASK,
					membr_bitfld_new);
			continue;
		}

		pgid = in_le32(&l2ana_reg->port_id_tbl.port_grp_id[i]);
		if ((u32)(1 << i) & membr_bitfld_old & VSC9953_PGID_PORT_MASK)
			pgid &= ~((u32)(1 << port_no));
		if ((u32)(1 << i) & membr_bitfld_new & VSC9953_PGID_PORT_MASK)
			pgid |= ((u32)(1 << port_no));

		out_le32(&l2ana_reg->port_id_tbl.port_grp_id[i], pgid);
	}
}

static void vsc9953_update_source_members_masks(int port_no,
						u32 membr_bitfld_old,
						u32 membr_bitfld_new)
{
	int i;
	int index;
	u32 pgid;
	struct vsc9953_analyzer *l2ana_reg;

	l2ana_reg = (struct vsc9953_analyzer *)(VSC9953_OFFSET +
						VSC9953_ANA_OFFSET);

	for (i = 0; i < VSC9953_MAX_PORTS + 1; i++) {
		index = PGID_SRC_START + i;
		pgid = in_le32(&l2ana_reg->port_id_tbl.port_grp_id[index]);
		if (i == port_no) {
			pgid = (pgid | VSC9953_PGID_PORT_MASK) &
			       ~membr_bitfld_new;
			out_le32(&l2ana_reg->port_id_tbl.port_grp_id[index],
				 pgid);
			continue;
		}

		if ((u32)(1 << i) & membr_bitfld_old & VSC9953_PGID_PORT_MASK)
			pgid |= (u32)(1 << port_no);

		if ((u32)(1 << i) & membr_bitfld_new & VSC9953_PGID_PORT_MASK)
			pgid &= ~(u32)(1 << port_no);
		out_le32(&l2ana_reg->port_id_tbl.port_grp_id[index], pgid);
	}
}

static u32 vsc9953_aggr_mask_get_next(u32 aggr_mask, u32 member_bitfield)
{
	if (!member_bitfield)
		return 0;

	if (!(aggr_mask & VSC9953_PGID_PORT_MASK))
		aggr_mask = 1;
	else
		aggr_mask <<= 1;

	while (!(aggr_mask & member_bitfield)) {
		aggr_mask <<= 1;
		if (!(aggr_mask & VSC9953_PGID_PORT_MASK))
			aggr_mask = 1;
	}

	return aggr_mask;
}

static void vsc9953_update_aggr_members_masks(int port_no, u32 membr_bitfld_old,
					      u32 membr_bitfld_new)
{
	int i;
	u32 pgid;
	u32 aggr_mask_old = 0;
	u32 aggr_mask_new = 0;
	struct vsc9953_analyzer *l2ana_reg;

	l2ana_reg = (struct vsc9953_analyzer *)(VSC9953_OFFSET +
						VSC9953_ANA_OFFSET);

	/* Update all the PGID aggregation masks */
	for (i = PGID_AGGR_START; i < PGID_SRC_START; i++) {
		pgid = in_le32(&l2ana_reg->port_id_tbl.port_grp_id[i]);

		aggr_mask_old = vsc9953_aggr_mask_get_next(aggr_mask_old,
							   membr_bitfld_old);
		pgid = (pgid & ~membr_bitfld_old) | aggr_mask_old;

		aggr_mask_new = vsc9953_aggr_mask_get_next(aggr_mask_new,
							   membr_bitfld_new);
		pgid = (pgid & ~membr_bitfld_new) | aggr_mask_new;

		out_le32(&l2ana_reg->port_id_tbl.port_grp_id[i], pgid);
	}
}

static u32 vsc9953_aggr_membr_bitfield_get(u8 member[VSC9953_MAX_PORTS])
{
	int i;
	u32 member_bitfield = 0;

	for (i = 0; i < VSC9953_MAX_PORTS; i++) {
		if (member[i])
			member_bitfield |= 1 << i;
	}
	member_bitfield &= VSC9953_PGID_PORT_MASK;

	return member_bitfield;
}

static void vsc9953_update_members_masks(int port_no,
					 u8 member_old[VSC9953_MAX_PORTS],
					 u8 member_new[VSC9953_MAX_PORTS])
{
	u32 membr_bitfld_old = vsc9953_aggr_membr_bitfield_get(member_old);
	u32 membr_bitfld_new = vsc9953_aggr_membr_bitfield_get(member_new);

	vsc9953_update_dest_members_masks(port_no, membr_bitfld_old,
					  membr_bitfld_new);
	vsc9953_update_source_members_masks(port_no, membr_bitfld_old,
					    membr_bitfld_new);
	vsc9953_update_aggr_members_masks(port_no, membr_bitfld_old,
					  membr_bitfld_new);
}

/* Set the aggregation group of a port */
static int vsc9953_port_aggr_grp_set(int port_no, int aggr_grp)
{
	u8 aggr_membr_old[VSC9953_MAX_PORTS];
	u8 aggr_membr_new[VSC9953_MAX_PORTS];
	int rc;
	int aggr_grp_old;
	u32 val;
	struct vsc9953_analyzer *l2ana_reg;

	if (!VSC9953_PORT_CHECK(port_no) || !VSC9953_PORT_CHECK(aggr_grp))
		return -EINVAL;

	l2ana_reg = (struct vsc9953_analyzer *)(VSC9953_OFFSET +
						VSC9953_ANA_OFFSET);

	rc = vsc9953_port_aggr_grp_get(port_no, &aggr_grp_old);
	if (rc)
		return rc;

	/* get all the members of the old aggregation group */
	vsc9953_aggr_grp_members_get(aggr_grp_old, aggr_membr_old);

	/* get all the members of the same aggregation group */
	vsc9953_aggr_grp_members_get(aggr_grp, aggr_membr_new);

	/* add current port as member to the new aggregation group */
	aggr_membr_old[port_no] = 0;
	aggr_membr_new[port_no] = 1;

	/* update masks */
	vsc9953_update_members_masks(port_no, aggr_membr_old, aggr_membr_new);

	/* Change logical port number */
	val = in_le32(&l2ana_reg->port[port_no].port_cfg);
	val = bitfield_replace_by_mask(val,
				       VSC9953_PORT_CFG_PORTID_MASK, aggr_grp);
	out_le32(&l2ana_reg->port[port_no].port_cfg, val);

	return 0;
}

static int vsc9953_port_status_key_func(struct ethsw_command_def *parsed_cmd)
{
	int i;
	u8 enabled;

	/* Last keyword should tell us if we should enable/disable the port */
	if (parsed_cmd->cmd_to_keywords[parsed_cmd->cmd_keywords_nr - 1] ==
	    ethsw_id_enable)
		enabled = 1;
	else if (parsed_cmd->cmd_to_keywords[parsed_cmd->cmd_keywords_nr - 1] ==
		 ethsw_id_disable)
		enabled = 0;
	else
		return CMD_RET_USAGE;

	if (parsed_cmd->port != ETHSW_CMD_PORT_ALL) {
		if (!VSC9953_PORT_CHECK(parsed_cmd->port)) {
			printf("Invalid port number: %d\n", parsed_cmd->port);
			return CMD_RET_FAILURE;
		}
		vsc9953_port_status_set(parsed_cmd->port, enabled);
	} else {
		for (i = 0; i < VSC9953_MAX_PORTS; i++)
			vsc9953_port_status_set(i, enabled);
	}

	return CMD_RET_SUCCESS;
}

static int vsc9953_port_config_key_func(struct ethsw_command_def *parsed_cmd)
{
	int i;

	if (parsed_cmd->port != ETHSW_CMD_PORT_ALL) {
		if (!VSC9953_PORT_CHECK(parsed_cmd->port)) {
			printf("Invalid port number: %d\n", parsed_cmd->port);
			return CMD_RET_FAILURE;
		}
		vsc9953_phy_autoneg(parsed_cmd->port);
		printf("%8s %8s %8s %8s %8s\n",
		       "Port", "Status", "Link", "Speed",
		       "Duplex");
		vsc9953_port_config_show(parsed_cmd->port);

	} else {
		for (i = 0; i < VSC9953_MAX_PORTS; i++)
			vsc9953_phy_autoneg(i);
		printf("%8s %8s %8s %8s %8s\n",
		       "Port", "Status", "Link", "Speed", "Duplex");
		for (i = 0; i < VSC9953_MAX_PORTS; i++)
			vsc9953_port_config_show(i);
	}

	return CMD_RET_SUCCESS;
}

static int vsc9953_port_stats_key_func(struct ethsw_command_def *parsed_cmd)
{
	int i;

	if (parsed_cmd->port != ETHSW_CMD_PORT_ALL) {
		if (!VSC9953_PORT_CHECK(parsed_cmd->port)) {
			printf("Invalid port number: %d\n", parsed_cmd->port);
			return CMD_RET_FAILURE;
		}
		vsc9953_port_statistics_show(parsed_cmd->port);
	} else {
		for (i = 0; i < VSC9953_MAX_PORTS; i++)
			vsc9953_port_statistics_show(i);
	}

	return CMD_RET_SUCCESS;
}

static int vsc9953_port_stats_clear_key_func(struct ethsw_command_def
					     *parsed_cmd)
{
	int i;

	if (parsed_cmd->port != ETHSW_CMD_PORT_ALL) {
		if (!VSC9953_PORT_CHECK(parsed_cmd->port)) {
			printf("Invalid port number: %d\n", parsed_cmd->port);
			return CMD_RET_FAILURE;
		}
		vsc9953_port_statistics_clear(parsed_cmd->port);
	} else {
		for (i = 0; i < VSC9953_MAX_PORTS; i++)
			vsc9953_port_statistics_clear(i);
	}

	return CMD_RET_SUCCESS;
}

static int vsc9953_learn_show_key_func(struct ethsw_command_def *parsed_cmd)
{
	int i;
	enum port_learn_mode mode;

	if (parsed_cmd->port != ETHSW_CMD_PORT_ALL) {
		if (!VSC9953_PORT_CHECK(parsed_cmd->port)) {
			printf("Invalid port number: %d\n", parsed_cmd->port);
			return CMD_RET_FAILURE;
		}
		if (vsc9953_port_learn_mode_get(parsed_cmd->port, &mode))
			return CMD_RET_FAILURE;
		printf("%7s %11s\n", "Port", "Learn mode");
		switch (mode) {
		case PORT_LEARN_NONE:
			printf("%7d %11s\n", parsed_cmd->port, "disable");
			break;
		case PORT_LEARN_AUTO:
			printf("%7d %11s\n", parsed_cmd->port, "auto");
			break;
		default:
			printf("%7d %11s\n", parsed_cmd->port, "-");
		}
	} else {
		printf("%7s %11s\n", "Port", "Learn mode");
		for (i = 0; i < VSC9953_MAX_PORTS; i++) {
			if (vsc9953_port_learn_mode_get(i, &mode))
				continue;
			switch (mode) {
			case PORT_LEARN_NONE:
				printf("%7d %11s\n", i, "disable");
				break;
			case PORT_LEARN_AUTO:
				printf("%7d %11s\n", i, "auto");
				break;
			default:
				printf("%7d %11s\n", i, "-");
			}
		}
	}

	return CMD_RET_SUCCESS;
}

static int vsc9953_learn_set_key_func(struct ethsw_command_def *parsed_cmd)
{
	int i;
	enum port_learn_mode mode;

	/* Last keyword should tell us the learn mode */
	if (parsed_cmd->cmd_to_keywords[parsed_cmd->cmd_keywords_nr - 1] ==
	    ethsw_id_auto)
		mode = PORT_LEARN_AUTO;
	else if (parsed_cmd->cmd_to_keywords[parsed_cmd->cmd_keywords_nr - 1] ==
		 ethsw_id_disable)
		mode = PORT_LEARN_NONE;
	else
		return CMD_RET_USAGE;

	if (parsed_cmd->port != ETHSW_CMD_PORT_ALL) {
		if (!VSC9953_PORT_CHECK(parsed_cmd->port)) {
			printf("Invalid port number: %d\n", parsed_cmd->port);
			return CMD_RET_FAILURE;
		}
		vsc9953_port_learn_mode_set(parsed_cmd->port, mode);
	} else {
		for (i = 0; i < VSC9953_MAX_PORTS; i++)
			vsc9953_port_learn_mode_set(i, mode);
	}

	return CMD_RET_SUCCESS;
}

static int vsc9953_fdb_show_key_func(struct ethsw_command_def *parsed_cmd)
{
	if (parsed_cmd->port != ETHSW_CMD_PORT_ALL &&
	    !VSC9953_PORT_CHECK(parsed_cmd->port)) {
		printf("Invalid port number: %d\n", parsed_cmd->port);
		return CMD_RET_FAILURE;
	}

	if (parsed_cmd->vid != ETHSW_CMD_VLAN_ALL &&
	    !VSC9953_VLAN_CHECK(parsed_cmd->vid)) {
		printf("Invalid VID number: %d\n", parsed_cmd->vid);
		return CMD_RET_FAILURE;
	}

	vsc9953_mac_table_show(parsed_cmd->port, parsed_cmd->vid);

	return CMD_RET_SUCCESS;
}

static int vsc9953_fdb_flush_key_func(struct ethsw_command_def *parsed_cmd)
{
	if (parsed_cmd->port != ETHSW_CMD_PORT_ALL &&
	    !VSC9953_PORT_CHECK(parsed_cmd->port)) {
		printf("Invalid port number: %d\n", parsed_cmd->port);
		return CMD_RET_FAILURE;
	}

	if (parsed_cmd->vid != ETHSW_CMD_VLAN_ALL &&
	    !VSC9953_VLAN_CHECK(parsed_cmd->vid)) {
		printf("Invalid VID number: %d\n", parsed_cmd->vid);
		return CMD_RET_FAILURE;
	}

	vsc9953_mac_table_flush(parsed_cmd->port, parsed_cmd->vid);

	return CMD_RET_SUCCESS;
}

static int vsc9953_fdb_entry_add_key_func(struct ethsw_command_def *parsed_cmd)
{
	int vid;

	/* a port number must be present */
	if (parsed_cmd->port == ETHSW_CMD_PORT_ALL) {
		printf("Please specify a port\n");
		return CMD_RET_FAILURE;
	}

	if (!VSC9953_PORT_CHECK(parsed_cmd->port)) {
		printf("Invalid port number: %d\n", parsed_cmd->port);
		return CMD_RET_FAILURE;
	}

	/* Use VLAN 1 if VID is not set */
	vid = (parsed_cmd->vid == ETHSW_CMD_VLAN_ALL ? 1 : parsed_cmd->vid);

	if (!VSC9953_VLAN_CHECK(vid)) {
		printf("Invalid VID number: %d\n", vid);
		return CMD_RET_FAILURE;
	}

	if (vsc9953_mac_table_add(parsed_cmd->port, parsed_cmd->ethaddr, vid))
		return CMD_RET_FAILURE;

	return CMD_RET_SUCCESS;
}

static int vsc9953_fdb_entry_del_key_func(struct ethsw_command_def *parsed_cmd)
{
	int vid;

	/* Use VLAN 1 if VID is not set */
	vid = (parsed_cmd->vid == ETHSW_CMD_VLAN_ALL ? 1 : parsed_cmd->vid);

	if (!VSC9953_VLAN_CHECK(vid)) {
		printf("Invalid VID number: %d\n", vid);
		return CMD_RET_FAILURE;
	}

	if (vsc9953_mac_table_del(parsed_cmd->ethaddr, vid))
		return CMD_RET_FAILURE;

	return CMD_RET_SUCCESS;
}

static int vsc9953_pvid_show_key_func(struct ethsw_command_def *parsed_cmd)
{
	int i;
	int pvid;

	if (parsed_cmd->port != ETHSW_CMD_PORT_ALL) {
		if (!VSC9953_PORT_CHECK(parsed_cmd->port)) {
			printf("Invalid port number: %d\n", parsed_cmd->port);
			return CMD_RET_FAILURE;
		}

		if (vsc9953_port_vlan_pvid_get(parsed_cmd->port, &pvid))
			return CMD_RET_FAILURE;
		printf("%7s %7s\n", "Port", "PVID");
		printf("%7d %7d\n", parsed_cmd->port, pvid);
	} else {
		printf("%7s %7s\n", "Port", "PVID");
		for (i = 0; i < VSC9953_MAX_PORTS; i++) {
			if (vsc9953_port_vlan_pvid_get(i, &pvid))
				continue;
			printf("%7d %7d\n", i, pvid);
		}
	}

	return CMD_RET_SUCCESS;
}

static int vsc9953_pvid_set_key_func(struct ethsw_command_def *parsed_cmd)
{
	/* PVID number should be set in parsed_cmd->vid */
	if (parsed_cmd->vid == ETHSW_CMD_VLAN_ALL) {
		printf("Please set a pvid value\n");
		return CMD_RET_FAILURE;
	}

	if (!VSC9953_VLAN_CHECK(parsed_cmd->vid)) {
		printf("Invalid VID number: %d\n", parsed_cmd->vid);
		return CMD_RET_FAILURE;
	}

	if (parsed_cmd->port != ETHSW_CMD_PORT_ALL) {
		if (!VSC9953_PORT_CHECK(parsed_cmd->port)) {
			printf("Invalid port number: %d\n", parsed_cmd->port);
			return CMD_RET_FAILURE;
		}
		vsc9953_port_vlan_pvid_set(parsed_cmd->port, parsed_cmd->vid);
	} else {
		vsc9953_port_all_vlan_pvid_set(parsed_cmd->vid);
	}

	return CMD_RET_SUCCESS;
}

static int vsc9953_vlan_show_key_func(struct ethsw_command_def *parsed_cmd)
{
	int i;

	if (parsed_cmd->port != ETHSW_CMD_PORT_ALL) {
		if (!VSC9953_PORT_CHECK(parsed_cmd->port)) {
			printf("Invalid port number: %d\n", parsed_cmd->port);
			return CMD_RET_FAILURE;
		}
		vsc9953_vlan_membership_show(parsed_cmd->port);
	} else {
		for (i = 0; i < VSC9953_MAX_PORTS; i++)
			vsc9953_vlan_membership_show(i);
	}

	return CMD_RET_SUCCESS;
}

static int vsc9953_vlan_set_key_func(struct ethsw_command_def *parsed_cmd)
{
	int i;
	int add;

	/* VLAN should be set in parsed_cmd->vid */
	if (parsed_cmd->vid == ETHSW_CMD_VLAN_ALL) {
		printf("Please set a vlan value\n");
		return CMD_RET_FAILURE;
	}

	if (!VSC9953_VLAN_CHECK(parsed_cmd->vid)) {
		printf("Invalid VID number: %d\n", parsed_cmd->vid);
		return CMD_RET_FAILURE;
	}

	/* keywords add/delete should be the last but one in array */
	if (parsed_cmd->cmd_to_keywords[parsed_cmd->cmd_keywords_nr - 2] ==
	    ethsw_id_add)
		add = 1;
	else if (parsed_cmd->cmd_to_keywords[parsed_cmd->cmd_keywords_nr - 2] ==
		 ethsw_id_del)
		add = 0;
	else
		return CMD_RET_USAGE;

	if (parsed_cmd->port != ETHSW_CMD_PORT_ALL) {
		if (!VSC9953_PORT_CHECK(parsed_cmd->port)) {
			printf("Invalid port number: %d\n", parsed_cmd->port);
			return CMD_RET_FAILURE;
		}
		vsc9953_vlan_table_membership_set(parsed_cmd->vid,
						  parsed_cmd->port, add);
	} else {
		for (i = 0; i < VSC9953_MAX_PORTS; i++)
			vsc9953_vlan_table_membership_set(parsed_cmd->vid, i,
							  add);
	}

	return CMD_RET_SUCCESS;
}
static int vsc9953_port_untag_show_key_func(
		struct ethsw_command_def *parsed_cmd)
{
	int i;

	printf("%7s\t%17s\n", "Port", "Untag");
	if (parsed_cmd->port != ETHSW_CMD_PORT_ALL) {
		if (!VSC9953_PORT_CHECK(parsed_cmd->port)) {
			printf("Invalid port number: %d\n", parsed_cmd->port);
			return CMD_RET_FAILURE;
		}
		vsc9953_port_vlan_egr_untag_show(parsed_cmd->port);
	} else {
		for (i = 0; i < VSC9953_MAX_PORTS; i++)
			vsc9953_port_vlan_egr_untag_show(i);
	}

	return CMD_RET_SUCCESS;
}

static int vsc9953_port_untag_set_key_func(struct ethsw_command_def *parsed_cmd)
{
	int i;
	enum egress_untag_mode mode;

	/* keywords for the untagged mode are the last in the array */
	if (parsed_cmd->cmd_to_keywords[parsed_cmd->cmd_keywords_nr - 1] ==
	    ethsw_id_all)
		mode = EGRESS_UNTAG_ALL;
	else if (parsed_cmd->cmd_to_keywords[parsed_cmd->cmd_keywords_nr - 1] ==
		 ethsw_id_none)
		mode = EGRESS_UNTAG_NONE;
	else if (parsed_cmd->cmd_to_keywords[parsed_cmd->cmd_keywords_nr - 1] ==
		 ethsw_id_pvid)
		mode = EGRESS_UNTAG_PVID_AND_ZERO;
	else
		return CMD_RET_USAGE;

	if (parsed_cmd->port != ETHSW_CMD_PORT_ALL) {
		if (!VSC9953_PORT_CHECK(parsed_cmd->port)) {
			printf("Invalid port number: %d\n", parsed_cmd->port);
			return CMD_RET_FAILURE;
		}
		vsc9953_port_vlan_egr_untag_set(parsed_cmd->port, mode);
	} else {
		for (i = 0; i < VSC9953_MAX_PORTS; i++)
			vsc9953_port_vlan_egr_untag_set(i, mode);
	}

	return CMD_RET_SUCCESS;
}

static int vsc9953_egr_vlan_tag_show_key_func(
		struct ethsw_command_def *parsed_cmd)
{
	int i;
	enum egress_vlan_tag mode;

	if (parsed_cmd->port != ETHSW_CMD_PORT_ALL) {
		if (!VSC9953_PORT_CHECK(parsed_cmd->port)) {
			printf("Invalid port number: %d\n", parsed_cmd->port);
			return CMD_RET_FAILURE;
		}
		vsc9953_port_vlan_egress_tag_get(parsed_cmd->port, &mode);
		printf("%7s\t%12s\n", "Port", "Egress VID");
		printf("%7d\t", parsed_cmd->port);
		switch (mode) {
		case EGR_TAG_CLASS:
			printf("%12s\n", "classified");
			break;
		case EGR_TAG_PVID:
			printf("%12s\n", "pvid");
			break;
		default:
			printf("%12s\n", "-");
		}
	} else {
		printf("%7s\t%12s\n", "Port", "Egress VID");
		for (i = 0; i < VSC9953_MAX_PORTS; i++) {
			vsc9953_port_vlan_egress_tag_get(i, &mode);
			switch (mode) {
			case EGR_TAG_CLASS:
				printf("%7d\t%12s\n", i, "classified");
				break;
			case EGR_TAG_PVID:
				printf("%7d\t%12s\n", i, "pvid");
				break;
			default:
				printf("%7d\t%12s\n", i, "-");
			}
		}
	}

	return CMD_RET_SUCCESS;
}

static int vsc9953_egr_vlan_tag_set_key_func(
		struct ethsw_command_def *parsed_cmd)
{
	int i;
	enum egress_vlan_tag mode;

	/* keywords for the egress vlan tag mode are the last in the array */
	if (parsed_cmd->cmd_to_keywords[parsed_cmd->cmd_keywords_nr - 1] ==
	    ethsw_id_pvid)
		mode = EGR_TAG_PVID;
	else if (parsed_cmd->cmd_to_keywords[parsed_cmd->cmd_keywords_nr - 1] ==
		 ethsw_id_classified)
		mode = EGR_TAG_CLASS;
	else
		return CMD_RET_USAGE;

	if (parsed_cmd->port != ETHSW_CMD_PORT_ALL) {
		if (!VSC9953_PORT_CHECK(parsed_cmd->port)) {
			printf("Invalid port number: %d\n", parsed_cmd->port);
			return CMD_RET_FAILURE;
		}
		vsc9953_port_vlan_egress_tag_set(parsed_cmd->port, mode);
	} else {
		for (i = 0; i < VSC9953_MAX_PORTS; i++)
			vsc9953_port_vlan_egress_tag_set(i, mode);
	}

	return CMD_RET_SUCCESS;
}

static int vsc9953_vlan_learn_show_key_func(
		struct ethsw_command_def *parsed_cmd)
{
	int rc;
	enum vlan_learning_mode mode;

	rc = vsc9953_vlan_learning_get(&mode);
	if (rc)
		return CMD_RET_FAILURE;

	switch (mode) {
	case SHARED_VLAN_LEARNING:
		printf("VLAN learning mode: shared\n");
		break;
	case PRIVATE_VLAN_LEARNING:
		printf("VLAN learning mode: private\n");
		break;
	default:
		printf("Unknown VLAN learning mode\n");
		rc = CMD_RET_FAILURE;
	}

	return CMD_RET_SUCCESS;
}

static int vsc9953_vlan_learn_set_key_func(struct ethsw_command_def *parsed_cmd)
{
	enum vlan_learning_mode mode;

	/* keywords for shared/private are the last in the array */
	if (parsed_cmd->cmd_to_keywords[parsed_cmd->cmd_keywords_nr - 1] ==
	    ethsw_id_shared)
		mode = SHARED_VLAN_LEARNING;
	else if (parsed_cmd->cmd_to_keywords[parsed_cmd->cmd_keywords_nr - 1] ==
		 ethsw_id_private)
		mode = PRIVATE_VLAN_LEARNING;
	else
		return CMD_RET_USAGE;

	vsc9953_vlan_learning_set(mode);

	return CMD_RET_SUCCESS;
}

static int vsc9953_ingr_fltr_show_key_func(struct ethsw_command_def *parsed_cmd)
{
	int i;
	int enabled;

	printf("%7s\t%18s\n", "Port", "Ingress filtering");
	if (parsed_cmd->port != ETHSW_CMD_PORT_ALL) {
		if (!VSC9953_PORT_CHECK(parsed_cmd->port)) {
			printf("Invalid port number: %d\n", parsed_cmd->port);
			return CMD_RET_FAILURE;
		}
		enabled = vsc9953_port_ingress_filtering_get(parsed_cmd->port);
		printf("%7d\t%18s\n", parsed_cmd->port, enabled ? "enable" :
								  "disable");
	} else {
		for (i = 0; i < VSC9953_MAX_PORTS; i++) {
			enabled = vsc9953_port_ingress_filtering_get(i);
			printf("%7d\t%18s\n", parsed_cmd->port, enabled ?
								"enable" :
								"disable");
		}
	}

	return CMD_RET_SUCCESS;
}

static int vsc9953_ingr_fltr_set_key_func(struct ethsw_command_def *parsed_cmd)
{
	int i;
	int enable;

	/* keywords for enabling/disabling ingress filtering
	 * are the last in the array
	 */
	if (parsed_cmd->cmd_to_keywords[parsed_cmd->cmd_keywords_nr - 1] ==
	    ethsw_id_enable)
		enable = 1;
	else if (parsed_cmd->cmd_to_keywords[parsed_cmd->cmd_keywords_nr - 1] ==
		 ethsw_id_disable)
		enable = 0;
	else
		return CMD_RET_USAGE;

	if (parsed_cmd->port != ETHSW_CMD_PORT_ALL) {
		if (!VSC9953_PORT_CHECK(parsed_cmd->port)) {
			printf("Invalid port number: %d\n", parsed_cmd->port);
			return CMD_RET_FAILURE;
		}
		vsc9953_port_ingress_filtering_set(parsed_cmd->port, enable);
	} else {
		for (i = 0; i < VSC9953_MAX_PORTS; i++)
			vsc9953_port_ingress_filtering_set(i, enable);
	}

	return CMD_RET_SUCCESS;
}

static int vsc9953_port_aggr_show_key_func(struct ethsw_command_def *parsed_cmd)
{
	int i;
	int aggr_grp;

	if (parsed_cmd->port != ETHSW_CMD_PORT_ALL) {
		if (!VSC9953_PORT_CHECK(parsed_cmd->port)) {
			printf("Invalid port number: %d\n", parsed_cmd->port);
			return CMD_RET_FAILURE;
		}

		if (vsc9953_port_aggr_grp_get(parsed_cmd->port, &aggr_grp))
			return CMD_RET_FAILURE;
		printf("%7s %10s\n", "Port", "Aggr grp");
		printf("%7d %10d\n", parsed_cmd->port, aggr_grp);
	} else {
		printf("%7s %10s\n", "Port", "Aggr grp");
		for (i = 0; i < VSC9953_MAX_PORTS; i++) {
			if (vsc9953_port_aggr_grp_get(i, &aggr_grp))
				continue;
			printf("%7d %10d\n", i, aggr_grp);
		}
	}

	return CMD_RET_SUCCESS;
}

static int vsc9953_port_aggr_set_key_func(struct ethsw_command_def *parsed_cmd)
{
	int i;

	/* Aggregation group number should be set in parsed_cmd->aggr_grp */
	if (parsed_cmd->aggr_grp == ETHSW_CMD_AGGR_GRP_NONE) {
		printf("Please set an aggregation group value\n");
		return CMD_RET_FAILURE;
	}

	if (!VSC9953_PORT_CHECK(parsed_cmd->aggr_grp)) {
		printf("Invalid aggregation group number: %d\n",
		       parsed_cmd->aggr_grp);
		return CMD_RET_FAILURE;
	}

	if (parsed_cmd->port != ETHSW_CMD_PORT_ALL) {
		if (!VSC9953_PORT_CHECK(parsed_cmd->port)) {
			printf("Invalid port number: %d\n", parsed_cmd->port);
			return CMD_RET_FAILURE;
		}
		if (vsc9953_port_aggr_grp_set(parsed_cmd->port,
					      parsed_cmd->aggr_grp)) {
			printf("Port %d: failed to set aggr group %d\n",
			       parsed_cmd->port, parsed_cmd->aggr_grp);
		}
	} else {
		for (i = 0; i < VSC9953_MAX_PORTS; i++) {
			if (vsc9953_port_aggr_grp_set(i,
						      parsed_cmd->aggr_grp)) {
				printf("Port %d: failed to set aggr group %d\n",
				       i, parsed_cmd->aggr_grp);
			}
		}
	}

	return CMD_RET_SUCCESS;
}

static struct ethsw_command_func vsc9953_cmd_func = {
		.ethsw_name = "L2 Switch VSC9953",
		.port_enable = &vsc9953_port_status_key_func,
		.port_disable = &vsc9953_port_status_key_func,
		.port_show = &vsc9953_port_config_key_func,
		.port_stats = &vsc9953_port_stats_key_func,
		.port_stats_clear = &vsc9953_port_stats_clear_key_func,
		.port_learn = &vsc9953_learn_set_key_func,
		.port_learn_show = &vsc9953_learn_show_key_func,
		.fdb_show = &vsc9953_fdb_show_key_func,
		.fdb_flush = &vsc9953_fdb_flush_key_func,
		.fdb_entry_add = &vsc9953_fdb_entry_add_key_func,
		.fdb_entry_del = &vsc9953_fdb_entry_del_key_func,
		.pvid_show = &vsc9953_pvid_show_key_func,
		.pvid_set = &vsc9953_pvid_set_key_func,
		.vlan_show = &vsc9953_vlan_show_key_func,
		.vlan_set = &vsc9953_vlan_set_key_func,
		.port_untag_show = &vsc9953_port_untag_show_key_func,
		.port_untag_set = &vsc9953_port_untag_set_key_func,
		.port_egr_vlan_show = &vsc9953_egr_vlan_tag_show_key_func,
		.port_egr_vlan_set = &vsc9953_egr_vlan_tag_set_key_func,
		.vlan_learn_show = &vsc9953_vlan_learn_show_key_func,
		.vlan_learn_set = &vsc9953_vlan_learn_set_key_func,
		.port_ingr_filt_show = &vsc9953_ingr_fltr_show_key_func,
		.port_ingr_filt_set = &vsc9953_ingr_fltr_set_key_func,
		.port_aggr_show = &vsc9953_port_aggr_show_key_func,
		.port_aggr_set = &vsc9953_port_aggr_set_key_func,
};

#endif /* CONFIG_CMD_ETHSW */

/*****************************************************************************
At startup, the default configuration would be:
	- HW learning enabled on all ports; (HW default)
	- All ports are in VLAN 1;
	- All ports are VLAN aware;
	- All ports have POP_COUNT 1;
	- All ports have PVID 1;
	- All ports have TPID 0x8100; (HW default)
	- All ports tag frames classified to all VLANs that are not PVID;
*****************************************************************************/
void vsc9953_default_configuration(void)
{
	int i;

	if (vsc9953_autoage_time_set(VSC9953_DEFAULT_AGE_TIME))
		debug("VSC9953: failed to set AGE time to %d\n",
		      VSC9953_DEFAULT_AGE_TIME);

	for (i = 0; i < VSC9953_MAX_VLAN; i++)
		vsc9953_vlan_table_membership_all_set(i, 0);
	vsc9953_port_all_vlan_aware_set(1);
	vsc9953_port_all_vlan_pvid_set(1);
	vsc9953_port_all_vlan_poncnt_set(1);
	vsc9953_vlan_table_membership_all_set(1, 1);
	vsc9953_vlan_ingr_fltr_learn_drop(1);
	vsc9953_port_all_vlan_egress_untagged_set(EGRESS_UNTAG_PVID_AND_ZERO);
	if (vsc9953_aggr_code_set(AGGR_CODE_ALL))
		debug("VSC9953: failed to set default aggregation code mode\n");
}

static void vcap_entry2cache_init(u32 target, u32 entry_words)
{
	int i;

	for (i = 0; i < entry_words; i++) {
		out_le32((unsigned int *)(VSC9953_OFFSET +
				VSC9953_VCAP_CACHE_ENTRY_DAT(target, i)), 0x00);
		out_le32((unsigned int *)(VSC9953_OFFSET +
				VSC9953_VCAP_CACHE_MASK_DAT(target, i)), 0xFF);
	}

	out_le32((unsigned int *)(VSC9953_OFFSET +
				VSC9953_VCAP_CACHE_TG_DAT(target)), 0x00);
	out_le32((unsigned int *)(VSC9953_OFFSET +
				  VSC9953_VCAP_CFG_MV_CFG(target)),
		 VSC9953_VCAP_CFG_MV_CFG_SIZE(entry_words));
}

static void vcap_action2cache_init(u32 target, u32 action_words,
				   u32 counter_words)
{
	int i;

	for (i = 0; i < action_words; i++)
		out_le32((unsigned int *)(VSC9953_OFFSET +
			       VSC9953_VCAP_CACHE_ACTION_DAT(target, i)), 0x00);

	for (i = 0; i < counter_words; i++)
		out_le32((unsigned int *)(VSC9953_OFFSET +
				  VSC9953_VCAP_CACHE_CNT_DAT(target, i)), 0x00);
}

static int vcap_cmd(u32 target, u16 ix, int cmd, int sel, int entry_count)
{
	u32 tgt = target;
	u32 value = (VSC9953_VCAP_UPDATE_CTRL_UPDATE_CMD(cmd) |
		     VSC9953_VCAP_UPDATE_CTRL_UPDATE_ADDR(ix) |
		     VSC9953_VCAP_UPDATE_CTRL_UPDATE_SHOT);

	if ((sel & TCAM_SEL_ENTRY) && ix >= entry_count)
		return CMD_RET_FAILURE;

	if (!(sel & TCAM_SEL_ENTRY))
		value |= VSC9953_VCAP_UPDATE_CTRL_UPDATE_ENTRY_DIS;

	if (!(sel & TCAM_SEL_ACTION))
		value |= VSC9953_VCAP_UPDATE_CTRL_UPDATE_ACTION_DIS;

	if (!(sel & TCAM_SEL_COUNTER))
		value |= VSC9953_VCAP_UPDATE_CTRL_UPDATE_CNT_DIS;

	out_le32((unsigned int *)(VSC9953_OFFSET +
				VSC9953_VCAP_CFG_UPDATE_CTRL(tgt)), value);

	do {
		value = in_le32((unsigned int *)(VSC9953_OFFSET +
				VSC9953_VCAP_CFG_UPDATE_CTRL(tgt)));

	} while (value & VSC9953_VCAP_UPDATE_CTRL_UPDATE_SHOT);

	return CMD_RET_SUCCESS;
}

static void vsc9953_vcap_init(void)
{
	u32 tgt = VSC9953_ES0;
	int cmd_ret;

	/* write entries */
	vcap_entry2cache_init(tgt, ENTRY_WORDS_ES0);
	cmd_ret = vcap_cmd(tgt, 0, TCAM_CMD_INITIALIZE, TCAM_SEL_ENTRY,
			   ENTRY_WORDS_ES0);
	if (cmd_ret != CMD_RET_SUCCESS)
		debug("VSC9953:%d invalid TCAM_SEL_ENTRY\n",
		      __LINE__);

	/* write actions and counters */
	vcap_action2cache_init(tgt, BITS_TO_DWORD(ES0_ACT_WIDTH),
			       BITS_TO_DWORD(ES0_CNT_WIDTH));
	out_le32((unsigned int *)(VSC9953_OFFSET +
				  VSC9953_VCAP_CFG_MV_CFG(tgt)),
		 VSC9953_VCAP_CFG_MV_CFG_SIZE(ES0_ACT_COUNT));
	cmd_ret = vcap_cmd(tgt, 0, TCAM_CMD_INITIALIZE,
			   TCAM_SEL_ACTION | TCAM_SEL_COUNTER, ENTRY_WORDS_ES0);
	if (cmd_ret != CMD_RET_SUCCESS)
		debug("VSC9953:%d invalid TCAM_SEL_ACTION | TCAM_SEL_COUNTER\n",
		      __LINE__);

	tgt = VSC9953_IS1;

	/* write entries */
	vcap_entry2cache_init(tgt, ENTRY_WORDS_IS1);
	cmd_ret = vcap_cmd(tgt, 0, TCAM_CMD_INITIALIZE, TCAM_SEL_ENTRY,
			   ENTRY_WORDS_IS1);
	if (cmd_ret != CMD_RET_SUCCESS)
		debug("VSC9953:%d invalid TCAM_SEL_ENTRY\n",
		      __LINE__);

	/* write actions and counters */
	vcap_action2cache_init(tgt, BITS_TO_DWORD(IS1_ACT_WIDTH),
			       BITS_TO_DWORD(IS1_CNT_WIDTH));
	out_le32((unsigned int *)(VSC9953_OFFSET +
				  VSC9953_VCAP_CFG_MV_CFG(tgt)),
		 VSC9953_VCAP_CFG_MV_CFG_SIZE(IS1_ACT_COUNT));
	cmd_ret = vcap_cmd(tgt, 0, TCAM_CMD_INITIALIZE,
			   TCAM_SEL_ACTION | TCAM_SEL_COUNTER, ENTRY_WORDS_IS1);
	if (cmd_ret != CMD_RET_SUCCESS)
		debug("VSC9953:%d invalid TCAM_SEL_ACTION | TCAM_SEL_COUNTER\n",
		      __LINE__);

	tgt = VSC9953_IS2;

	/* write entries */
	vcap_entry2cache_init(tgt, ENTRY_WORDS_IS2);
	cmd_ret = vcap_cmd(tgt, 0, TCAM_CMD_INITIALIZE, TCAM_SEL_ENTRY,
			   ENTRY_WORDS_IS2);
	if (cmd_ret != CMD_RET_SUCCESS)
		debug("VSC9953:%d invalid selection: TCAM_SEL_ENTRY\n",
		      __LINE__);

	/* write actions and counters */
	vcap_action2cache_init(tgt, BITS_TO_DWORD(IS2_ACT_WIDTH),
			       BITS_TO_DWORD(IS2_CNT_WIDTH));
	out_le32((unsigned int *)(VSC9953_OFFSET +
				  VSC9953_VCAP_CFG_MV_CFG(tgt)),
		 VSC9953_VCAP_CFG_MV_CFG_SIZE(IS2_ACT_COUNT));
	cmd_ret = vcap_cmd(tgt, 0, TCAM_CMD_INITIALIZE,
			   TCAM_SEL_ACTION | TCAM_SEL_COUNTER, ENTRY_WORDS_IS2);
	if (cmd_ret != CMD_RET_SUCCESS)
		debug("VSC9953:%d invalid TCAM_SEL_ACTION | TCAM_SEL_COUNTER\n",
		      __LINE__);
}

void vsc9953_init(bd_t *bis)
{
	u32 i;
	u32 hdx_cfg = 0;
	u32 phy_addr = 0;
	int timeout;
	struct vsc9953_system_reg *l2sys_reg;
	struct vsc9953_qsys_reg *l2qsys_reg;
	struct vsc9953_dev_gmii *l2dev_gmii_reg;
	struct vsc9953_analyzer *l2ana_reg;
	struct vsc9953_devcpu_gcb *l2dev_gcb;

	l2dev_gmii_reg = (struct vsc9953_dev_gmii *)(VSC9953_OFFSET +
			VSC9953_DEV_GMII_OFFSET);

	l2ana_reg = (struct vsc9953_analyzer *)(VSC9953_OFFSET +
			VSC9953_ANA_OFFSET);

	l2sys_reg = (struct vsc9953_system_reg *)(VSC9953_OFFSET +
			VSC9953_SYS_OFFSET);

	l2qsys_reg = (struct vsc9953_qsys_reg *)(VSC9953_OFFSET +
			VSC9953_QSYS_OFFSET);

	l2dev_gcb = (struct vsc9953_devcpu_gcb *)(VSC9953_OFFSET +
			VSC9953_DEVCPU_GCB);

	out_le32(&l2dev_gcb->chip_regs.soft_rst,
		 VSC9953_SOFT_SWC_RST_ENA);
	timeout = 50000;
	while ((in_le32(&l2dev_gcb->chip_regs.soft_rst) &
			VSC9953_SOFT_SWC_RST_ENA) && --timeout)
		udelay(1); /* busy wait for vsc9953 soft reset */
	if (timeout == 0)
		debug("Timeout waiting for VSC9953 to reset\n");

	out_le32(&l2sys_reg->sys.reset_cfg, VSC9953_MEM_ENABLE |
		 VSC9953_MEM_INIT);

	timeout = 50000;
	while ((in_le32(&l2sys_reg->sys.reset_cfg) &
		VSC9953_MEM_INIT) && --timeout)
		udelay(1); /* busy wait for vsc9953 memory init */
	if (timeout == 0)
		debug("Timeout waiting for VSC9953 memory to initialize\n");

	out_le32(&l2sys_reg->sys.reset_cfg, (in_le32(&l2sys_reg->sys.reset_cfg)
			| VSC9953_CORE_ENABLE));

	/* VSC9953 Setting to be done once only */
	out_le32(&l2qsys_reg->sys.ext_cpu_cfg, 0x00000b00);

	for (i = 0; i < VSC9953_MAX_PORTS; i++) {
		if (vsc9953_port_init(i))
			printf("Failed to initialize l2switch port %d\n", i);

		if (!vsc9953_l2sw.port[i].enabled)
			continue;

		/* Enable VSC9953 GMII Ports Port ID 0 - 7 */
		if (VSC9953_INTERNAL_PORT_CHECK(i)) {
			out_le32(&l2ana_reg->pfc[i].pfc_cfg,
				 VSC9953_PFC_FC_QSGMII);
			out_le32(&l2sys_reg->pause_cfg.mac_fc_cfg[i],
				 VSC9953_MAC_FC_CFG_QSGMII);
		} else {
			out_le32(&l2ana_reg->pfc[i].pfc_cfg,
				 VSC9953_PFC_FC);
			out_le32(&l2sys_reg->pause_cfg.mac_fc_cfg[i],
				 VSC9953_MAC_FC_CFG);
		}

		l2dev_gmii_reg = (struct vsc9953_dev_gmii *)
				 (VSC9953_OFFSET + VSC9953_DEV_GMII_OFFSET +
				 T1040_SWITCH_GMII_DEV_OFFSET * i);

		out_le32(&l2dev_gmii_reg->port_mode.clock_cfg,
			 VSC9953_CLOCK_CFG);
		out_le32(&l2dev_gmii_reg->mac_cfg_status.mac_ena_cfg,
			 VSC9953_MAC_ENA_CFG);
		out_le32(&l2dev_gmii_reg->mac_cfg_status.mac_mode_cfg,
			 VSC9953_MAC_MODE_CFG);
		out_le32(&l2dev_gmii_reg->mac_cfg_status.mac_ifg_cfg,
			 VSC9953_MAC_IFG_CFG);
		/* mac_hdx_cfg varies with port id*/
		hdx_cfg = VSC9953_MAC_HDX_CFG | (i << 16);
		out_le32(&l2dev_gmii_reg->mac_cfg_status.mac_hdx_cfg, hdx_cfg);
		out_le32(&l2sys_reg->sys.front_port_mode[i],
			 VSC9953_FRONT_PORT_MODE);
		setbits_le32(&l2qsys_reg->sys.switch_port_mode[i],
			     VSC9953_PORT_ENA);
		out_le32(&l2dev_gmii_reg->mac_cfg_status.mac_maxlen_cfg,
			 VSC9953_MAC_MAX_LEN);
		out_le32(&l2sys_reg->pause_cfg.pause_cfg[i],
			 VSC9953_PAUSE_CFG);
		/* WAIT FOR 2 us*/
		udelay(2);

		/* Initialize Lynx PHY Wrappers */
		phy_addr = 0;
		if (vsc9953_l2sw.port[i].enet_if ==
				PHY_INTERFACE_MODE_QSGMII)
			phy_addr = (i + 0x4) & 0x1F;
		else if (vsc9953_l2sw.port[i].enet_if ==
				PHY_INTERFACE_MODE_SGMII)
			phy_addr = (i + 1) & 0x1F;

		if (phy_addr) {
			/* SGMII IF mode + AN enable */
			vsc9953_mdio_write(&l2dev_gcb->mii_mng[0], phy_addr,
					   0x14, PHY_SGMII_IF_MODE_AN |
					   PHY_SGMII_IF_MODE_SGMII);
			/* Dev ability according to SGMII specification */
			vsc9953_mdio_write(&l2dev_gcb->mii_mng[0], phy_addr,
					   0x4, PHY_SGMII_DEV_ABILITY_SGMII);
			/* Adjust link timer for SGMII
			 * 1.6 ms in units of 8 ns = 2 * 10^5 = 0x30d40
			 */
			vsc9953_mdio_write(&l2dev_gcb->mii_mng[0], phy_addr,
					   0x13, 0x0003);
			vsc9953_mdio_write(&l2dev_gcb->mii_mng[0], phy_addr,
					   0x12, 0x0d40);
			/* Restart AN */
			vsc9953_mdio_write(&l2dev_gcb->mii_mng[0], phy_addr,
					   0x0, PHY_SGMII_CR_DEF_VAL |
					   PHY_SGMII_CR_RESET_AN);

			timeout = 50000;
			while ((vsc9953_mdio_read(&l2dev_gcb->mii_mng[0],
					phy_addr, 0x01) & 0x0020) && --timeout)
				udelay(1); /* wait for AN to complete */
			if (timeout == 0)
				debug("Timeout waiting for AN to complete\n");
		}
	}

	vsc9953_vcap_init();
	vsc9953_default_configuration();

#ifdef CONFIG_CMD_ETHSW
	if (ethsw_define_functions(&vsc9953_cmd_func) < 0)
		debug("Unable to use \"ethsw\" commands\n");
#endif

	printf("VSC9953 L2 switch initialized\n");
	return;
}
