/*******************************************************************************
 *
 * Intel Ethernet Controller XL710 Family Linux Driver
 * Copyright(c) 2013 - 2015 Intel Corporation.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * The full GNU General Public License is included in this distribution in
 * the file called "COPYING".
 *
 * Contact Information:
 * e1000-devel Mailing List <e1000-devel@lists.sourceforge.net>
 * Intel Corporation, 5200 N.E. Elam Young Parkway, Hillsboro, OR 97124-6497
 *
 ******************************************************************************/

/* ethtool support for i40e */

#include "i40e.h"
#include "i40e_diag.h"

struct i40e_stats {
	char stat_string[ETH_GSTRING_LEN];
	int sizeof_stat;
	int stat_offset;
};

#define I40E_STAT(_type, _name, _stat) { \
	.stat_string = _name, \
	.sizeof_stat = FIELD_SIZEOF(_type, _stat), \
	.stat_offset = offsetof(_type, _stat) \
}

#define I40E_NETDEV_STAT(_net_stat) \
		I40E_STAT(struct rtnl_link_stats64, #_net_stat, _net_stat)
#define I40E_PF_STAT(_name, _stat) \
		I40E_STAT(struct i40e_pf, _name, _stat)
#define I40E_VSI_STAT(_name, _stat) \
		I40E_STAT(struct i40e_vsi, _name, _stat)
#define I40E_VEB_STAT(_name, _stat) \
		I40E_STAT(struct i40e_veb, _name, _stat)

static const struct i40e_stats i40e_gstrings_net_stats[] = {
	I40E_NETDEV_STAT(rx_packets),
	I40E_NETDEV_STAT(tx_packets),
	I40E_NETDEV_STAT(rx_bytes),
	I40E_NETDEV_STAT(tx_bytes),
	I40E_NETDEV_STAT(rx_errors),
	I40E_NETDEV_STAT(tx_errors),
	I40E_NETDEV_STAT(rx_dropped),
	I40E_NETDEV_STAT(tx_dropped),
	I40E_NETDEV_STAT(collisions),
	I40E_NETDEV_STAT(rx_length_errors),
	I40E_NETDEV_STAT(rx_crc_errors),
};

static const struct i40e_stats i40e_gstrings_veb_stats[] = {
	I40E_VEB_STAT("rx_bytes", stats.rx_bytes),
	I40E_VEB_STAT("tx_bytes", stats.tx_bytes),
	I40E_VEB_STAT("rx_unicast", stats.rx_unicast),
	I40E_VEB_STAT("tx_unicast", stats.tx_unicast),
	I40E_VEB_STAT("rx_multicast", stats.rx_multicast),
	I40E_VEB_STAT("tx_multicast", stats.tx_multicast),
	I40E_VEB_STAT("rx_broadcast", stats.rx_broadcast),
	I40E_VEB_STAT("tx_broadcast", stats.tx_broadcast),
	I40E_VEB_STAT("rx_discards", stats.rx_discards),
	I40E_VEB_STAT("tx_discards", stats.tx_discards),
	I40E_VEB_STAT("tx_errors", stats.tx_errors),
	I40E_VEB_STAT("rx_unknown_protocol", stats.rx_unknown_protocol),
};

static const struct i40e_stats i40e_gstrings_misc_stats[] = {
	I40E_VSI_STAT("rx_unicast", eth_stats.rx_unicast),
	I40E_VSI_STAT("tx_unicast", eth_stats.tx_unicast),
	I40E_VSI_STAT("rx_multicast", eth_stats.rx_multicast),
	I40E_VSI_STAT("tx_multicast", eth_stats.tx_multicast),
	I40E_VSI_STAT("rx_broadcast", eth_stats.rx_broadcast),
	I40E_VSI_STAT("tx_broadcast", eth_stats.tx_broadcast),
	I40E_VSI_STAT("rx_unknown_protocol", eth_stats.rx_unknown_protocol),
};

static int i40e_add_fdir_ethtool(struct i40e_vsi *vsi,
				 struct ethtool_rxnfc *cmd);

/* These PF_STATs might look like duplicates of some NETDEV_STATs,
 * but they are separate.  This device supports Virtualization, and
 * as such might have several netdevs supporting VMDq and FCoE going
 * through a single port.  The NETDEV_STATs are for individual netdevs
 * seen at the top of the stack, and the PF_STATs are for the physical
 * function at the bottom of the stack hosting those netdevs.
 *
 * The PF_STATs are appended to the netdev stats only when ethtool -S
 * is queried on the base PF netdev, not on the VMDq or FCoE netdev.
 */
static struct i40e_stats i40e_gstrings_stats[] = {
	I40E_PF_STAT("rx_bytes", stats.eth.rx_bytes),
	I40E_PF_STAT("tx_bytes", stats.eth.tx_bytes),
	I40E_PF_STAT("rx_unicast", stats.eth.rx_unicast),
	I40E_PF_STAT("tx_unicast", stats.eth.tx_unicast),
	I40E_PF_STAT("rx_multicast", stats.eth.rx_multicast),
	I40E_PF_STAT("tx_multicast", stats.eth.tx_multicast),
	I40E_PF_STAT("rx_broadcast", stats.eth.rx_broadcast),
	I40E_PF_STAT("tx_broadcast", stats.eth.tx_broadcast),
	I40E_PF_STAT("tx_errors", stats.eth.tx_errors),
	I40E_PF_STAT("rx_dropped", stats.eth.rx_discards),
	I40E_PF_STAT("tx_dropped_link_down", stats.tx_dropped_link_down),
	I40E_PF_STAT("crc_errors", stats.crc_errors),
	I40E_PF_STAT("illegal_bytes", stats.illegal_bytes),
	I40E_PF_STAT("mac_local_faults", stats.mac_local_faults),
	I40E_PF_STAT("mac_remote_faults", stats.mac_remote_faults),
	I40E_PF_STAT("tx_timeout", tx_timeout_count),
	I40E_PF_STAT("rx_csum_bad", hw_csum_rx_error),
	I40E_PF_STAT("rx_length_errors", stats.rx_length_errors),
	I40E_PF_STAT("link_xon_rx", stats.link_xon_rx),
	I40E_PF_STAT("link_xoff_rx", stats.link_xoff_rx),
	I40E_PF_STAT("link_xon_tx", stats.link_xon_tx),
	I40E_PF_STAT("link_xoff_tx", stats.link_xoff_tx),
	I40E_PF_STAT("rx_size_64", stats.rx_size_64),
	I40E_PF_STAT("rx_size_127", stats.rx_size_127),
	I40E_PF_STAT("rx_size_255", stats.rx_size_255),
	I40E_PF_STAT("rx_size_511", stats.rx_size_511),
	I40E_PF_STAT("rx_size_1023", stats.rx_size_1023),
	I40E_PF_STAT("rx_size_1522", stats.rx_size_1522),
	I40E_PF_STAT("rx_size_big", stats.rx_size_big),
	I40E_PF_STAT("tx_size_64", stats.tx_size_64),
	I40E_PF_STAT("tx_size_127", stats.tx_size_127),
	I40E_PF_STAT("tx_size_255", stats.tx_size_255),
	I40E_PF_STAT("tx_size_511", stats.tx_size_511),
	I40E_PF_STAT("tx_size_1023", stats.tx_size_1023),
	I40E_PF_STAT("tx_size_1522", stats.tx_size_1522),
	I40E_PF_STAT("tx_size_big", stats.tx_size_big),
	I40E_PF_STAT("rx_undersize", stats.rx_undersize),
	I40E_PF_STAT("rx_fragments", stats.rx_fragments),
	I40E_PF_STAT("rx_oversize", stats.rx_oversize),
	I40E_PF_STAT("rx_jabber", stats.rx_jabber),
	I40E_PF_STAT("VF_admin_queue_requests", vf_aq_requests),
	I40E_PF_STAT("rx_hwtstamp_cleared", rx_hwtstamp_cleared),
	I40E_PF_STAT("fdir_flush_cnt", fd_flush_cnt),
	I40E_PF_STAT("fdir_atr_match", stats.fd_atr_match),
	I40E_PF_STAT("fdir_sb_match", stats.fd_sb_match),

	/* LPI stats */
	I40E_PF_STAT("tx_lpi_status", stats.tx_lpi_status),
	I40E_PF_STAT("rx_lpi_status", stats.rx_lpi_status),
	I40E_PF_STAT("tx_lpi_count", stats.tx_lpi_count),
	I40E_PF_STAT("rx_lpi_count", stats.rx_lpi_count),
};

#ifdef I40E_FCOE
static const struct i40e_stats i40e_gstrings_fcoe_stats[] = {
	I40E_VSI_STAT("fcoe_bad_fccrc", fcoe_stats.fcoe_bad_fccrc),
	I40E_VSI_STAT("rx_fcoe_dropped", fcoe_stats.rx_fcoe_dropped),
	I40E_VSI_STAT("rx_fcoe_packets", fcoe_stats.rx_fcoe_packets),
	I40E_VSI_STAT("rx_fcoe_dwords", fcoe_stats.rx_fcoe_dwords),
	I40E_VSI_STAT("fcoe_ddp_count", fcoe_stats.fcoe_ddp_count),
	I40E_VSI_STAT("fcoe_last_error", fcoe_stats.fcoe_last_error),
	I40E_VSI_STAT("tx_fcoe_packets", fcoe_stats.tx_fcoe_packets),
	I40E_VSI_STAT("tx_fcoe_dwords", fcoe_stats.tx_fcoe_dwords),
};

#endif /* I40E_FCOE */
#define I40E_QUEUE_STATS_LEN(n) \
	(((struct i40e_netdev_priv *)netdev_priv((n)))->vsi->num_queue_pairs \
	    * 2 /* Tx and Rx together */                                     \
	    * (sizeof(struct i40e_queue_stats) / sizeof(u64)))
#define I40E_GLOBAL_STATS_LEN	ARRAY_SIZE(i40e_gstrings_stats)
#define I40E_NETDEV_STATS_LEN   ARRAY_SIZE(i40e_gstrings_net_stats)
#define I40E_MISC_STATS_LEN	ARRAY_SIZE(i40e_gstrings_misc_stats)
#ifdef I40E_FCOE
#define I40E_FCOE_STATS_LEN	ARRAY_SIZE(i40e_gstrings_fcoe_stats)
#define I40E_VSI_STATS_LEN(n)	(I40E_NETDEV_STATS_LEN + \
				 I40E_FCOE_STATS_LEN + \
				 I40E_MISC_STATS_LEN + \
				 I40E_QUEUE_STATS_LEN((n)))
#else
#define I40E_VSI_STATS_LEN(n)   (I40E_NETDEV_STATS_LEN + \
				 I40E_MISC_STATS_LEN + \
				 I40E_QUEUE_STATS_LEN((n)))
#endif /* I40E_FCOE */
#define I40E_PFC_STATS_LEN ( \
		(FIELD_SIZEOF(struct i40e_pf, stats.priority_xoff_rx) + \
		 FIELD_SIZEOF(struct i40e_pf, stats.priority_xon_rx) + \
		 FIELD_SIZEOF(struct i40e_pf, stats.priority_xoff_tx) + \
		 FIELD_SIZEOF(struct i40e_pf, stats.priority_xon_tx) + \
		 FIELD_SIZEOF(struct i40e_pf, stats.priority_xon_2_xoff)) \
		 / sizeof(u64))
#define I40E_VEB_STATS_LEN	ARRAY_SIZE(i40e_gstrings_veb_stats)
#define I40E_PF_STATS_LEN(n)	(I40E_GLOBAL_STATS_LEN + \
				 I40E_PFC_STATS_LEN + \
				 I40E_VSI_STATS_LEN((n)))

enum i40e_ethtool_test_id {
	I40E_ETH_TEST_REG = 0,
	I40E_ETH_TEST_EEPROM,
	I40E_ETH_TEST_INTR,
	I40E_ETH_TEST_LOOPBACK,
	I40E_ETH_TEST_LINK,
};

static const char i40e_gstrings_test[][ETH_GSTRING_LEN] = {
	"Register test  (offline)",
	"Eeprom test    (offline)",
	"Interrupt test (offline)",
	"Loopback test  (offline)",
	"Link test   (on/offline)"
};

#define I40E_TEST_LEN (sizeof(i40e_gstrings_test) / ETH_GSTRING_LEN)

static const char i40e_priv_flags_strings[][ETH_GSTRING_LEN] = {
	"NPAR",
};

#define I40E_PRIV_FLAGS_STR_LEN \
	(sizeof(i40e_priv_flags_strings) / ETH_GSTRING_LEN)

/**
 * i40e_partition_setting_complaint - generic complaint for MFP restriction
 * @pf: the PF struct
 **/
static void i40e_partition_setting_complaint(struct i40e_pf *pf)
{
	dev_info(&pf->pdev->dev,
		 "The link settings are allowed to be changed only from the first partition of a given port. Please switch to the first partition in order to change the setting.\n");
}

/**
 * i40e_get_settings_link_up - Get the Link settings for when link is up
 * @hw: hw structure
 * @ecmd: ethtool command to fill in
 * @netdev: network interface device structure
 *
 **/
static void i40e_get_settings_link_up(struct i40e_hw *hw,
				      struct ethtool_cmd *ecmd,
				      struct net_device *netdev)
{
	struct i40e_link_status *hw_link_info = &hw->phy.link_info;
	u32 link_speed = hw_link_info->link_speed;

	/* Initialize supported and advertised settings based on phy settings */
	switch (hw_link_info->phy_type) {
	case I40E_PHY_TYPE_40GBASE_CR4:
	case I40E_PHY_TYPE_40GBASE_CR4_CU:
		ecmd->supported = SUPPORTED_Autoneg |
				  SUPPORTED_40000baseCR4_Full;
		ecmd->advertising = ADVERTISED_Autoneg |
				    ADVERTISED_40000baseCR4_Full;
		break;
	case I40E_PHY_TYPE_XLAUI:
	case I40E_PHY_TYPE_XLPPI:
	case I40E_PHY_TYPE_40GBASE_AOC:
		ecmd->supported = SUPPORTED_40000baseCR4_Full;
		break;
	case I40E_PHY_TYPE_40GBASE_KR4:
		ecmd->supported = SUPPORTED_Autoneg |
				  SUPPORTED_40000baseKR4_Full;
		ecmd->advertising = ADVERTISED_Autoneg |
				    ADVERTISED_40000baseKR4_Full;
		break;
	case I40E_PHY_TYPE_40GBASE_SR4:
		ecmd->supported = SUPPORTED_40000baseSR4_Full;
		break;
	case I40E_PHY_TYPE_40GBASE_LR4:
		ecmd->supported = SUPPORTED_40000baseLR4_Full;
		break;
	case I40E_PHY_TYPE_20GBASE_KR2:
		ecmd->supported = SUPPORTED_Autoneg |
				  SUPPORTED_20000baseKR2_Full;
		ecmd->advertising = ADVERTISED_Autoneg |
				    ADVERTISED_20000baseKR2_Full;
		break;
	case I40E_PHY_TYPE_10GBASE_KX4:
		ecmd->supported = SUPPORTED_Autoneg |
				  SUPPORTED_10000baseKX4_Full;
		ecmd->advertising = ADVERTISED_Autoneg |
				    ADVERTISED_10000baseKX4_Full;
		break;
	case I40E_PHY_TYPE_10GBASE_KR:
		ecmd->supported = SUPPORTED_Autoneg |
				  SUPPORTED_10000baseKR_Full;
		ecmd->advertising = ADVERTISED_Autoneg |
				    ADVERTISED_10000baseKR_Full;
		break;
	case I40E_PHY_TYPE_10GBASE_SR:
	case I40E_PHY_TYPE_10GBASE_LR:
	case I40E_PHY_TYPE_1000BASE_SX:
	case I40E_PHY_TYPE_1000BASE_LX:
		ecmd->supported = SUPPORTED_10000baseT_Full |
				  SUPPORTED_1000baseT_Full;
		if (hw_link_info->requested_speeds & I40E_LINK_SPEED_10GB)
			ecmd->advertising |= ADVERTISED_10000baseT_Full;
		if (hw_link_info->requested_speeds & I40E_LINK_SPEED_1GB)
			ecmd->advertising |= ADVERTISED_1000baseT_Full;
		break;
	case I40E_PHY_TYPE_1000BASE_KX:
		ecmd->supported = SUPPORTED_Autoneg |
				  SUPPORTED_1000baseKX_Full;
		ecmd->advertising = ADVERTISED_Autoneg |
				    ADVERTISED_1000baseKX_Full;
		break;
	case I40E_PHY_TYPE_10GBASE_T:
	case I40E_PHY_TYPE_1000BASE_T:
	case I40E_PHY_TYPE_100BASE_TX:
		ecmd->supported = SUPPORTED_Autoneg |
				  SUPPORTED_10000baseT_Full |
				  SUPPORTED_1000baseT_Full |
				  SUPPORTED_100baseT_Full;
		ecmd->advertising = ADVERTISED_Autoneg;
		if (hw_link_info->requested_speeds & I40E_LINK_SPEED_10GB)
			ecmd->advertising |= ADVERTISED_10000baseT_Full;
		if (hw_link_info->requested_speeds & I40E_LINK_SPEED_1GB)
			ecmd->advertising |= ADVERTISED_1000baseT_Full;
		if (hw_link_info->requested_speeds & I40E_LINK_SPEED_100MB)
			ecmd->advertising |= ADVERTISED_100baseT_Full;
		break;
	case I40E_PHY_TYPE_10GBASE_CR1_CU:
	case I40E_PHY_TYPE_10GBASE_CR1:
		ecmd->supported = SUPPORTED_Autoneg |
				  SUPPORTED_10000baseT_Full;
		ecmd->advertising = ADVERTISED_Autoneg |
				    ADVERTISED_10000baseT_Full;
		break;
	case I40E_PHY_TYPE_XAUI:
	case I40E_PHY_TYPE_XFI:
	case I40E_PHY_TYPE_SFI:
	case I40E_PHY_TYPE_10GBASE_SFPP_CU:
	case I40E_PHY_TYPE_10GBASE_AOC:
		ecmd->supported = SUPPORTED_10000baseT_Full;
		break;
	case I40E_PHY_TYPE_SGMII:
		ecmd->supported = SUPPORTED_Autoneg |
				  SUPPORTED_1000baseT_Full |
				  SUPPORTED_100baseT_Full;
		if (hw_link_info->requested_speeds & I40E_LINK_SPEED_1GB)
			ecmd->advertising |= ADVERTISED_1000baseT_Full;
		if (hw_link_info->requested_speeds & I40E_LINK_SPEED_100MB)
			ecmd->advertising |= ADVERTISED_100baseT_Full;
		break;
	default:
		/* if we got here and link is up something bad is afoot */
		netdev_info(netdev, "WARNING: Link is up but PHY type 0x%x is not recognized.\n",
			    hw_link_info->phy_type);
	}

	/* Set speed and duplex */
	switch (link_speed) {
	case I40E_LINK_SPEED_40GB:
		ethtool_cmd_speed_set(ecmd, SPEED_40000);
		break;
	case I40E_LINK_SPEED_20GB:
		ethtool_cmd_speed_set(ecmd, SPEED_20000);
		break;
	case I40E_LINK_SPEED_10GB:
		ethtool_cmd_speed_set(ecmd, SPEED_10000);
		break;
	case I40E_LINK_SPEED_1GB:
		ethtool_cmd_speed_set(ecmd, SPEED_1000);
		break;
	case I40E_LINK_SPEED_100MB:
		ethtool_cmd_speed_set(ecmd, SPEED_100);
		break;
	default:
		break;
	}
	ecmd->duplex = DUPLEX_FULL;
}

/**
 * i40e_get_settings_link_down - Get the Link settings for when link is down
 * @hw: hw structure
 * @ecmd: ethtool command to fill in
 *
 * Reports link settings that can be determined when link is down
 **/
static void i40e_get_settings_link_down(struct i40e_hw *hw,
					struct ethtool_cmd *ecmd)
{
	struct i40e_link_status *hw_link_info = &hw->phy.link_info;

	/* link is down and the driver needs to fall back on
	 * device ID to determine what kinds of info to display,
	 * it's mostly a guess that may change when link is up
	 */
	switch (hw->device_id) {
	case I40E_DEV_ID_QSFP_A:
	case I40E_DEV_ID_QSFP_B:
	case I40E_DEV_ID_QSFP_C:
		/* pluggable QSFP */
		ecmd->supported = SUPPORTED_40000baseSR4_Full |
				  SUPPORTED_40000baseCR4_Full |
				  SUPPORTED_40000baseLR4_Full;
		ecmd->advertising = ADVERTISED_40000baseSR4_Full |
				    ADVERTISED_40000baseCR4_Full |
				    ADVERTISED_40000baseLR4_Full;
		break;
	case I40E_DEV_ID_KX_B:
		/* backplane 40G */
		ecmd->supported = SUPPORTED_40000baseKR4_Full;
		ecmd->advertising = ADVERTISED_40000baseKR4_Full;
		break;
	case I40E_DEV_ID_KX_C:
		/* backplane 10G */
		ecmd->supported = SUPPORTED_10000baseKR_Full;
		ecmd->advertising = ADVERTISED_10000baseKR_Full;
		break;
	case I40E_DEV_ID_10G_BASE_T:
		ecmd->supported = SUPPORTED_10000baseT_Full |
				  SUPPORTED_1000baseT_Full |
				  SUPPORTED_100baseT_Full;
		/* Figure out what has been requested */
		if (hw_link_info->requested_speeds & I40E_LINK_SPEED_10GB)
			ecmd->advertising |= ADVERTISED_10000baseT_Full;
		if (hw_link_info->requested_speeds & I40E_LINK_SPEED_1GB)
			ecmd->advertising |= ADVERTISED_1000baseT_Full;
		if (hw_link_info->requested_speeds & I40E_LINK_SPEED_100MB)
			ecmd->advertising |= ADVERTISED_100baseT_Full;
		break;
	case I40E_DEV_ID_20G_KR2:
		/* backplane 20G */
		ecmd->supported = SUPPORTED_20000baseKR2_Full;
		ecmd->advertising = ADVERTISED_20000baseKR2_Full;
		break;
	default:
		/* all the rest are 10G/1G */
		ecmd->supported = SUPPORTED_10000baseT_Full |
				  SUPPORTED_1000baseT_Full;
		/* Figure out what has been requested */
		if (hw_link_info->requested_speeds & I40E_LINK_SPEED_10GB)
			ecmd->advertising |= ADVERTISED_10000baseT_Full;
		if (hw_link_info->requested_speeds & I40E_LINK_SPEED_1GB)
			ecmd->advertising |= ADVERTISED_1000baseT_Full;
		break;
	}

	/* With no link speed and duplex are unknown */
	ethtool_cmd_speed_set(ecmd, SPEED_UNKNOWN);
	ecmd->duplex = DUPLEX_UNKNOWN;
}

/**
 * i40e_get_settings - Get Link Speed and Duplex settings
 * @netdev: network interface device structure
 * @ecmd: ethtool command
 *
 * Reports speed/duplex settings based on media_type
 **/
static int i40e_get_settings(struct net_device *netdev,
			     struct ethtool_cmd *ecmd)
{
	struct i40e_netdev_priv *np = netdev_priv(netdev);
	struct i40e_pf *pf = np->vsi->back;
	struct i40e_hw *hw = &pf->hw;
	struct i40e_link_status *hw_link_info = &hw->phy.link_info;
	bool link_up = hw_link_info->link_info & I40E_AQ_LINK_UP;

	if (link_up)
		i40e_get_settings_link_up(hw, ecmd, netdev);
	else
		i40e_get_settings_link_down(hw, ecmd);

	/* Now set the settings that don't rely on link being up/down */

	/* Set autoneg settings */
	ecmd->autoneg = ((hw_link_info->an_info & I40E_AQ_AN_COMPLETED) ?
			  AUTONEG_ENABLE : AUTONEG_DISABLE);

	switch (hw->phy.media_type) {
	case I40E_MEDIA_TYPE_BACKPLANE:
		ecmd->supported |= SUPPORTED_Autoneg |
				   SUPPORTED_Backplane;
		ecmd->advertising |= ADVERTISED_Autoneg |
				     ADVERTISED_Backplane;
		ecmd->port = PORT_NONE;
		break;
	case I40E_MEDIA_TYPE_BASET:
		ecmd->supported |= SUPPORTED_TP;
		ecmd->advertising |= ADVERTISED_TP;
		ecmd->port = PORT_TP;
		break;
	case I40E_MEDIA_TYPE_DA:
	case I40E_MEDIA_TYPE_CX4:
		ecmd->supported |= SUPPORTED_FIBRE;
		ecmd->advertising |= ADVERTISED_FIBRE;
		ecmd->port = PORT_DA;
		break;
	case I40E_MEDIA_TYPE_FIBER:
		ecmd->supported |= SUPPORTED_FIBRE;
		ecmd->port = PORT_FIBRE;
		break;
	case I40E_MEDIA_TYPE_UNKNOWN:
	default:
		ecmd->port = PORT_OTHER;
		break;
	}

	/* Set transceiver */
	ecmd->transceiver = XCVR_EXTERNAL;

	/* Set flow control settings */
	ecmd->supported |= SUPPORTED_Pause;

	switch (hw->fc.requested_mode) {
	case I40E_FC_FULL:
		ecmd->advertising |= ADVERTISED_Pause;
		break;
	case I40E_FC_TX_PAUSE:
		ecmd->advertising |= ADVERTISED_Asym_Pause;
		break;
	case I40E_FC_RX_PAUSE:
		ecmd->advertising |= (ADVERTISED_Pause |
				      ADVERTISED_Asym_Pause);
		break;
	default:
		ecmd->advertising &= ~(ADVERTISED_Pause |
				       ADVERTISED_Asym_Pause);
		break;
	}

	return 0;
}

/**
 * i40e_set_settings - Set Speed and Duplex
 * @netdev: network interface device structure
 * @ecmd: ethtool command
 *
 * Set speed/duplex per media_types advertised/forced
 **/
static int i40e_set_settings(struct net_device *netdev,
			     struct ethtool_cmd *ecmd)
{
	struct i40e_netdev_priv *np = netdev_priv(netdev);
	struct i40e_aq_get_phy_abilities_resp abilities;
	struct i40e_aq_set_phy_config config;
	struct i40e_pf *pf = np->vsi->back;
	struct i40e_vsi *vsi = np->vsi;
	struct i40e_hw *hw = &pf->hw;
	struct ethtool_cmd safe_ecmd;
	i40e_status status = 0;
	bool change = false;
	int err = 0;
	u8 autoneg;
	u32 advertise;

	/* Changing port settings is not supported if this isn't the
	 * port's controlling PF
	 */
	if (hw->partition_id != 1) {
		i40e_partition_setting_complaint(pf);
		return -EOPNOTSUPP;
	}

	if (vsi != pf->vsi[pf->lan_vsi])
		return -EOPNOTSUPP;

	if (hw->phy.media_type != I40E_MEDIA_TYPE_BASET &&
	    hw->phy.media_type != I40E_MEDIA_TYPE_FIBER &&
	    hw->phy.media_type != I40E_MEDIA_TYPE_BACKPLANE &&
	    hw->phy.link_info.link_info & I40E_AQ_LINK_UP)
		return -EOPNOTSUPP;

	/* get our own copy of the bits to check against */
	memset(&safe_ecmd, 0, sizeof(struct ethtool_cmd));
	i40e_get_settings(netdev, &safe_ecmd);

	/* save autoneg and speed out of ecmd */
	autoneg = ecmd->autoneg;
	advertise = ecmd->advertising;

	/* set autoneg and speed back to what they currently are */
	ecmd->autoneg = safe_ecmd.autoneg;
	ecmd->advertising = safe_ecmd.advertising;

	ecmd->cmd = safe_ecmd.cmd;
	/* If ecmd and safe_ecmd are not the same now, then they are
	 * trying to set something that we do not support
	 */
	if (memcmp(ecmd, &safe_ecmd, sizeof(struct ethtool_cmd)))
		return -EOPNOTSUPP;

	while (test_bit(__I40E_CONFIG_BUSY, &vsi->state))
		usleep_range(1000, 2000);

	/* Get the current phy config */
	status = i40e_aq_get_phy_capabilities(hw, false, false, &abilities,
					      NULL);
	if (status)
		return -EAGAIN;

	/* Copy abilities to config in case autoneg is not
	 * set below
	 */
	memset(&config, 0, sizeof(struct i40e_aq_set_phy_config));
	config.abilities = abilities.abilities;

	/* Check autoneg */
	if (autoneg == AUTONEG_ENABLE) {
		/* If autoneg is not supported, return error */
		if (!(safe_ecmd.supported & SUPPORTED_Autoneg)) {
			netdev_info(netdev, "Autoneg not supported on this phy\n");
			return -EINVAL;
		}
		/* If autoneg was not already enabled */
		if (!(hw->phy.link_info.an_info & I40E_AQ_AN_COMPLETED)) {
			config.abilities = abilities.abilities |
					   I40E_AQ_PHY_ENABLE_AN;
			change = true;
		}
	} else {
		/* If autoneg is supported 10GBASE_T is the only phy that
		 * can disable it, so otherwise return error
		 */
		if (safe_ecmd.supported & SUPPORTED_Autoneg &&
		    hw->phy.link_info.phy_type != I40E_PHY_TYPE_10GBASE_T) {
			netdev_info(netdev, "Autoneg cannot be disabled on this phy\n");
			return -EINVAL;
		}
		/* If autoneg is currently enabled */
		if (hw->phy.link_info.an_info & I40E_AQ_AN_COMPLETED) {
			config.abilities = abilities.abilities &
					   ~I40E_AQ_PHY_ENABLE_AN;
			change = true;
		}
	}

	if (advertise & ~safe_ecmd.supported)
		return -EINVAL;

	if (advertise & ADVERTISED_100baseT_Full)
		config.link_speed |= I40E_LINK_SPEED_100MB;
	if (advertise & ADVERTISED_1000baseT_Full ||
	    advertise & ADVERTISED_1000baseKX_Full)
		config.link_speed |= I40E_LINK_SPEED_1GB;
	if (advertise & ADVERTISED_10000baseT_Full ||
	    advertise & ADVERTISED_10000baseKX4_Full ||
	    advertise & ADVERTISED_10000baseKR_Full)
		config.link_speed |= I40E_LINK_SPEED_10GB;
	if (advertise & ADVERTISED_20000baseKR2_Full)
		config.link_speed |= I40E_LINK_SPEED_20GB;
	if (advertise & ADVERTISED_40000baseKR4_Full ||
	    advertise & ADVERTISED_40000baseCR4_Full ||
	    advertise & ADVERTISED_40000baseSR4_Full ||
	    advertise & ADVERTISED_40000baseLR4_Full)
		config.link_speed |= I40E_LINK_SPEED_40GB;

	if (change || (abilities.link_speed != config.link_speed)) {
		/* copy over the rest of the abilities */
		config.phy_type = abilities.phy_type;
		config.eee_capability = abilities.eee_capability;
		config.eeer = abilities.eeer_val;
		config.low_power_ctrl = abilities.d3_lpan;

		/* save the requested speeds */
		hw->phy.link_info.requested_speeds = config.link_speed;
		/* set link and auto negotiation so changes take effect */
		config.abilities |= I40E_AQ_PHY_ENABLE_ATOMIC_LINK;
		/* If link is up put link down */
		if (hw->phy.link_info.link_info & I40E_AQ_LINK_UP) {
			/* Tell the OS link is going down, the link will go
			 * back up when fw says it is ready asynchronously
			 */
			netdev_info(netdev, "PHY settings change requested, NIC Link is going down.\n");
			netif_carrier_off(netdev);
			netif_tx_stop_all_queues(netdev);
		}

		/* make the aq call */
		status = i40e_aq_set_phy_config(hw, &config, NULL);
		if (status) {
			netdev_info(netdev, "Set phy config failed with error %d.\n",
				    status);
			return -EAGAIN;
		}

		status = i40e_aq_get_link_info(hw, true, NULL, NULL);
		if (status)
			netdev_info(netdev, "Updating link info failed with error %d\n",
				    status);

	} else {
		netdev_info(netdev, "Nothing changed, exiting without setting anything.\n");
	}

	return err;
}

static int i40e_nway_reset(struct net_device *netdev)
{
	/* restart autonegotiation */
	struct i40e_netdev_priv *np = netdev_priv(netdev);
	struct i40e_pf *pf = np->vsi->back;
	struct i40e_hw *hw = &pf->hw;
	bool link_up = hw->phy.link_info.link_info & I40E_AQ_LINK_UP;
	i40e_status ret = 0;

	ret = i40e_aq_set_link_restart_an(hw, link_up, NULL);
	if (ret) {
		netdev_info(netdev, "link restart failed, aq_err=%d\n",
			    pf->hw.aq.asq_last_status);
		return -EIO;
	}

	return 0;
}

/**
 * i40e_get_pauseparam -  Get Flow Control status
 * Return tx/rx-pause status
 **/
static void i40e_get_pauseparam(struct net_device *netdev,
				struct ethtool_pauseparam *pause)
{
	struct i40e_netdev_priv *np = netdev_priv(netdev);
	struct i40e_pf *pf = np->vsi->back;
	struct i40e_hw *hw = &pf->hw;
	struct i40e_link_status *hw_link_info = &hw->phy.link_info;
	struct i40e_dcbx_config *dcbx_cfg = &hw->local_dcbx_config;

	pause->autoneg =
		((hw_link_info->an_info & I40E_AQ_AN_COMPLETED) ?
		  AUTONEG_ENABLE : AUTONEG_DISABLE);

	/* PFC enabled so report LFC as off */
	if (dcbx_cfg->pfc.pfcenable) {
		pause->rx_pause = 0;
		pause->tx_pause = 0;
		return;
	}

	if (hw->fc.current_mode == I40E_FC_RX_PAUSE) {
		pause->rx_pause = 1;
	} else if (hw->fc.current_mode == I40E_FC_TX_PAUSE) {
		pause->tx_pause = 1;
	} else if (hw->fc.current_mode == I40E_FC_FULL) {
		pause->rx_pause = 1;
		pause->tx_pause = 1;
	}
}

/**
 * i40e_set_pauseparam - Set Flow Control parameter
 * @netdev: network interface device structure
 * @pause: return tx/rx flow control status
 **/
static int i40e_set_pauseparam(struct net_device *netdev,
			       struct ethtool_pauseparam *pause)
{
	struct i40e_netdev_priv *np = netdev_priv(netdev);
	struct i40e_pf *pf = np->vsi->back;
	struct i40e_vsi *vsi = np->vsi;
	struct i40e_hw *hw = &pf->hw;
	struct i40e_link_status *hw_link_info = &hw->phy.link_info;
	struct i40e_dcbx_config *dcbx_cfg = &hw->local_dcbx_config;
	bool link_up = hw_link_info->link_info & I40E_AQ_LINK_UP;
	i40e_status status;
	u8 aq_failures;
	int err = 0;

	/* Changing the port's flow control is not supported if this isn't the
	 * port's controlling PF
	 */
	if (hw->partition_id != 1) {
		i40e_partition_setting_complaint(pf);
		return -EOPNOTSUPP;
	}

	if (vsi != pf->vsi[pf->lan_vsi])
		return -EOPNOTSUPP;

	if (pause->autoneg != ((hw_link_info->an_info & I40E_AQ_AN_COMPLETED) ?
	    AUTONEG_ENABLE : AUTONEG_DISABLE)) {
		netdev_info(netdev, "To change autoneg please use: ethtool -s <dev> autoneg <on|off>\n");
		return -EOPNOTSUPP;
	}

	/* If we have link and don't have autoneg */
	if (!test_bit(__I40E_DOWN, &pf->state) &&
	    !(hw_link_info->an_info & I40E_AQ_AN_COMPLETED)) {
		/* Send message that it might not necessarily work*/
		netdev_info(netdev, "Autoneg did not complete so changing settings may not result in an actual change.\n");
	}

	if (dcbx_cfg->pfc.pfcenable) {
		netdev_info(netdev,
			    "Priority flow control enabled. Cannot set link flow control.\n");
		return -EOPNOTSUPP;
	}

	if (pause->rx_pause && pause->tx_pause)
		hw->fc.requested_mode = I40E_FC_FULL;
	else if (pause->rx_pause && !pause->tx_pause)
		hw->fc.requested_mode = I40E_FC_RX_PAUSE;
	else if (!pause->rx_pause && pause->tx_pause)
		hw->fc.requested_mode = I40E_FC_TX_PAUSE;
	else if (!pause->rx_pause && !pause->tx_pause)
		hw->fc.requested_mode = I40E_FC_NONE;
	else
		 return -EINVAL;

	/* Tell the OS link is going down, the link will go back up when fw
	 * says it is ready asynchronously
	 */
	netdev_info(netdev, "Flow control settings change requested, NIC Link is going down.\n");
	netif_carrier_off(netdev);
	netif_tx_stop_all_queues(netdev);

	/* Set the fc mode and only restart an if link is up*/
	status = i40e_set_fc(hw, &aq_failures, link_up);

	if (aq_failures & I40E_SET_FC_AQ_FAIL_GET) {
		netdev_info(netdev, "Set fc failed on the get_phy_capabilities call with error %d and status %d\n",
			    status, hw->aq.asq_last_status);
		err = -EAGAIN;
	}
	if (aq_failures & I40E_SET_FC_AQ_FAIL_SET) {
		netdev_info(netdev, "Set fc failed on the set_phy_config call with error %d and status %d\n",
			    status, hw->aq.asq_last_status);
		err = -EAGAIN;
	}
	if (aq_failures & I40E_SET_FC_AQ_FAIL_UPDATE) {
		netdev_info(netdev, "Set fc failed on the get_link_info call with error %d and status %d\n",
			    status, hw->aq.asq_last_status);
		err = -EAGAIN;
	}

	if (!test_bit(__I40E_DOWN, &pf->state)) {
		/* Give it a little more time to try to come back */
		msleep(75);
		if (!test_bit(__I40E_DOWN, &pf->state))
			return i40e_nway_reset(netdev);
	}

	return err;
}

static u32 i40e_get_msglevel(struct net_device *netdev)
{
	struct i40e_netdev_priv *np = netdev_priv(netdev);
	struct i40e_pf *pf = np->vsi->back;

	return pf->msg_enable;
}

static void i40e_set_msglevel(struct net_device *netdev, u32 data)
{
	struct i40e_netdev_priv *np = netdev_priv(netdev);
	struct i40e_pf *pf = np->vsi->back;

	if (I40E_DEBUG_USER & data)
		pf->hw.debug_mask = data;
	pf->msg_enable = data;
}

static int i40e_get_regs_len(struct net_device *netdev)
{
	int reg_count = 0;
	int i;

	for (i = 0; i40e_reg_list[i].offset != 0; i++)
		reg_count += i40e_reg_list[i].elements;

	return reg_count * sizeof(u32);
}

static void i40e_get_regs(struct net_device *netdev, struct ethtool_regs *regs,
			  void *p)
{
	struct i40e_netdev_priv *np = netdev_priv(netdev);
	struct i40e_pf *pf = np->vsi->back;
	struct i40e_hw *hw = &pf->hw;
	u32 *reg_buf = p;
	int i, j, ri;
	u32 reg;

	/* Tell ethtool which driver-version-specific regs output we have.
	 *
	 * At some point, if we have ethtool doing special formatting of
	 * this data, it will rely on this version number to know how to
	 * interpret things.  Hence, this needs to be updated if/when the
	 * diags register table is changed.
	 */
	regs->version = 1;

	/* loop through the diags reg table for what to print */
	ri = 0;
	for (i = 0; i40e_reg_list[i].offset != 0; i++) {
		for (j = 0; j < i40e_reg_list[i].elements; j++) {
			reg = i40e_reg_list[i].offset
				+ (j * i40e_reg_list[i].stride);
			reg_buf[ri++] = rd32(hw, reg);
		}
	}

}

static int i40e_get_eeprom(struct net_device *netdev,
			   struct ethtool_eeprom *eeprom, u8 *bytes)
{
	struct i40e_netdev_priv *np = netdev_priv(netdev);
	struct i40e_hw *hw = &np->vsi->back->hw;
	struct i40e_pf *pf = np->vsi->back;
	int ret_val = 0, len, offset;
	u8 *eeprom_buff;
	u16 i, sectors;
	bool last;
	u32 magic;

#define I40E_NVM_SECTOR_SIZE  4096
	if (eeprom->len == 0)
		return -EINVAL;

	/* check for NVMUpdate access method */
	magic = hw->vendor_id | (hw->device_id << 16);
	if (eeprom->magic && eeprom->magic != magic) {
		struct i40e_nvm_access *cmd;
		int errno;

		/* make sure it is the right magic for NVMUpdate */
		if ((eeprom->magic >> 16) != hw->device_id)
			return -EINVAL;

		cmd = (struct i40e_nvm_access *)eeprom;
		ret_val = i40e_nvmupd_command(hw, cmd, bytes, &errno);
		if (ret_val &&
		    ((hw->aq.asq_last_status != I40E_AQ_RC_EACCES) ||
		     (hw->debug_mask & I40E_DEBUG_NVM)))
			dev_info(&pf->pdev->dev,
				 "NVMUpdate read failed err=%d status=0x%x errno=%d module=%d offset=0x%x size=%d\n",
				 ret_val, hw->aq.asq_last_status, errno,
				 (u8)(cmd->config & I40E_NVM_MOD_PNT_MASK),
				 cmd->offset, cmd->data_size);

		return errno;
	}

	/* normal ethtool get_eeprom support */
	eeprom->magic = hw->vendor_id | (hw->device_id << 16);

	eeprom_buff = kzalloc(eeprom->len, GFP_KERNEL);
	if (!eeprom_buff)
		return -ENOMEM;

	ret_val = i40e_acquire_nvm(hw, I40E_RESOURCE_READ);
	if (ret_val) {
		dev_info(&pf->pdev->dev,
			 "Failed Acquiring NVM resource for read err=%d status=0x%x\n",
			 ret_val, hw->aq.asq_last_status);
		goto free_buff;
	}

	sectors = eeprom->len / I40E_NVM_SECTOR_SIZE;
	sectors += (eeprom->len % I40E_NVM_SECTOR_SIZE) ? 1 : 0;
	len = I40E_NVM_SECTOR_SIZE;
	last = false;
	for (i = 0; i < sectors; i++) {
		if (i == (sectors - 1)) {
			len = eeprom->len - (I40E_NVM_SECTOR_SIZE * i);
			last = true;
		}
		offset = eeprom->offset + (I40E_NVM_SECTOR_SIZE * i),
		ret_val = i40e_aq_read_nvm(hw, 0x0, offset, len,
				(u8 *)eeprom_buff + (I40E_NVM_SECTOR_SIZE * i),
				last, NULL);
		if (ret_val && hw->aq.asq_last_status == I40E_AQ_RC_EPERM) {
			dev_info(&pf->pdev->dev,
				 "read NVM failed, invalid offset 0x%x\n",
				 offset);
			break;
		} else if (ret_val &&
			   hw->aq.asq_last_status == I40E_AQ_RC_EACCES) {
			dev_info(&pf->pdev->dev,
				 "read NVM failed, access, offset 0x%x\n",
				 offset);
			break;
		} else if (ret_val) {
			dev_info(&pf->pdev->dev,
				 "read NVM failed offset %d err=%d status=0x%x\n",
				 offset, ret_val, hw->aq.asq_last_status);
			break;
		}
	}

	i40e_release_nvm(hw);
	memcpy(bytes, (u8 *)eeprom_buff, eeprom->len);
free_buff:
	kfree(eeprom_buff);
	return ret_val;
}

static int i40e_get_eeprom_len(struct net_device *netdev)
{
	struct i40e_netdev_priv *np = netdev_priv(netdev);
	struct i40e_hw *hw = &np->vsi->back->hw;
	u32 val;

	val = (rd32(hw, I40E_GLPCI_LBARCTRL)
		& I40E_GLPCI_LBARCTRL_FL_SIZE_MASK)
		>> I40E_GLPCI_LBARCTRL_FL_SIZE_SHIFT;
	/* register returns value in power of 2, 64Kbyte chunks. */
	val = (64 * 1024) * (1 << val);
	return val;
}

static int i40e_set_eeprom(struct net_device *netdev,
			   struct ethtool_eeprom *eeprom, u8 *bytes)
{
	struct i40e_netdev_priv *np = netdev_priv(netdev);
	struct i40e_hw *hw = &np->vsi->back->hw;
	struct i40e_pf *pf = np->vsi->back;
	struct i40e_nvm_access *cmd;
	int ret_val = 0;
	int errno;
	u32 magic;

	/* normal ethtool set_eeprom is not supported */
	magic = hw->vendor_id | (hw->device_id << 16);
	if (eeprom->magic == magic)
		return -EOPNOTSUPP;

	/* check for NVMUpdate access method */
	if (!eeprom->magic || (eeprom->magic >> 16) != hw->device_id)
		return -EINVAL;

	if (test_bit(__I40E_RESET_RECOVERY_PENDING, &pf->state) ||
	    test_bit(__I40E_RESET_INTR_RECEIVED, &pf->state))
		return -EBUSY;

	cmd = (struct i40e_nvm_access *)eeprom;
	ret_val = i40e_nvmupd_command(hw, cmd, bytes, &errno);
	if (ret_val &&
	    ((hw->aq.asq_last_status != I40E_AQ_RC_EPERM &&
	      hw->aq.asq_last_status != I40E_AQ_RC_EBUSY) ||
	     (hw->debug_mask & I40E_DEBUG_NVM)))
		dev_info(&pf->pdev->dev,
			 "NVMUpdate write failed err=%d status=0x%x errno=%d module=%d offset=0x%x size=%d\n",
			 ret_val, hw->aq.asq_last_status, errno,
			 (u8)(cmd->config & I40E_NVM_MOD_PNT_MASK),
			 cmd->offset, cmd->data_size);

	return errno;
}

static void i40e_get_drvinfo(struct net_device *netdev,
			     struct ethtool_drvinfo *drvinfo)
{
	struct i40e_netdev_priv *np = netdev_priv(netdev);
	struct i40e_vsi *vsi = np->vsi;
	struct i40e_pf *pf = vsi->back;

	strlcpy(drvinfo->driver, i40e_driver_name, sizeof(drvinfo->driver));
	strlcpy(drvinfo->version, i40e_driver_version_str,
		sizeof(drvinfo->version));
	strlcpy(drvinfo->fw_version, i40e_fw_version_str(&pf->hw),
		sizeof(drvinfo->fw_version));
	strlcpy(drvinfo->bus_info, pci_name(pf->pdev),
		sizeof(drvinfo->bus_info));
	drvinfo->n_priv_flags = I40E_PRIV_FLAGS_STR_LEN;
}

static void i40e_get_ringparam(struct net_device *netdev,
			       struct ethtool_ringparam *ring)
{
	struct i40e_netdev_priv *np = netdev_priv(netdev);
	struct i40e_pf *pf = np->vsi->back;
	struct i40e_vsi *vsi = pf->vsi[pf->lan_vsi];

	ring->rx_max_pending = I40E_MAX_NUM_DESCRIPTORS;
	ring->tx_max_pending = I40E_MAX_NUM_DESCRIPTORS;
	ring->rx_mini_max_pending = 0;
	ring->rx_jumbo_max_pending = 0;
	ring->rx_pending = vsi->rx_rings[0]->count;
	ring->tx_pending = vsi->tx_rings[0]->count;
	ring->rx_mini_pending = 0;
	ring->rx_jumbo_pending = 0;
}

static int i40e_set_ringparam(struct net_device *netdev,
			      struct ethtool_ringparam *ring)
{
	struct i40e_ring *tx_rings = NULL, *rx_rings = NULL;
	struct i40e_netdev_priv *np = netdev_priv(netdev);
	struct i40e_vsi *vsi = np->vsi;
	struct i40e_pf *pf = vsi->back;
	u32 new_rx_count, new_tx_count;
	int i, err = 0;

	if ((ring->rx_mini_pending) || (ring->rx_jumbo_pending))
		return -EINVAL;

	if (ring->tx_pending > I40E_MAX_NUM_DESCRIPTORS ||
	    ring->tx_pending < I40E_MIN_NUM_DESCRIPTORS ||
	    ring->rx_pending > I40E_MAX_NUM_DESCRIPTORS ||
	    ring->rx_pending < I40E_MIN_NUM_DESCRIPTORS) {
		netdev_info(netdev,
			    "Descriptors requested (Tx: %d / Rx: %d) out of range [%d-%d]\n",
			    ring->tx_pending, ring->rx_pending,
			    I40E_MIN_NUM_DESCRIPTORS, I40E_MAX_NUM_DESCRIPTORS);
		return -EINVAL;
	}

	new_tx_count = ALIGN(ring->tx_pending, I40E_REQ_DESCRIPTOR_MULTIPLE);
	new_rx_count = ALIGN(ring->rx_pending, I40E_REQ_DESCRIPTOR_MULTIPLE);

	/* if nothing to do return success */
	if ((new_tx_count == vsi->tx_rings[0]->count) &&
	    (new_rx_count == vsi->rx_rings[0]->count))
		return 0;

	while (test_and_set_bit(__I40E_CONFIG_BUSY, &pf->state))
		usleep_range(1000, 2000);

	if (!netif_running(vsi->netdev)) {
		/* simple case - set for the next time the netdev is started */
		for (i = 0; i < vsi->num_queue_pairs; i++) {
			vsi->tx_rings[i]->count = new_tx_count;
			vsi->rx_rings[i]->count = new_rx_count;
		}
		goto done;
	}

	/* We can't just free everything and then setup again,
	 * because the ISRs in MSI-X mode get passed pointers
	 * to the Tx and Rx ring structs.
	 */

	/* alloc updated Tx resources */
	if (new_tx_count != vsi->tx_rings[0]->count) {
		netdev_info(netdev,
			    "Changing Tx descriptor count from %d to %d.\n",
			    vsi->tx_rings[0]->count, new_tx_count);
		tx_rings = kcalloc(vsi->alloc_queue_pairs,
				   sizeof(struct i40e_ring), GFP_KERNEL);
		if (!tx_rings) {
			err = -ENOMEM;
			goto done;
		}

		for (i = 0; i < vsi->num_queue_pairs; i++) {
			/* clone ring and setup updated count */
			tx_rings[i] = *vsi->tx_rings[i];
			tx_rings[i].count = new_tx_count;
			err = i40e_setup_tx_descriptors(&tx_rings[i]);
			if (err) {
				while (i) {
					i--;
					i40e_free_tx_resources(&tx_rings[i]);
				}
				kfree(tx_rings);
				tx_rings = NULL;

				goto done;
			}
		}
	}

	/* alloc updated Rx resources */
	if (new_rx_count != vsi->rx_rings[0]->count) {
		netdev_info(netdev,
			    "Changing Rx descriptor count from %d to %d\n",
			    vsi->rx_rings[0]->count, new_rx_count);
		rx_rings = kcalloc(vsi->alloc_queue_pairs,
				   sizeof(struct i40e_ring), GFP_KERNEL);
		if (!rx_rings) {
			err = -ENOMEM;
			goto free_tx;
		}

		for (i = 0; i < vsi->num_queue_pairs; i++) {
			/* clone ring and setup updated count */
			rx_rings[i] = *vsi->rx_rings[i];
			rx_rings[i].count = new_rx_count;
			err = i40e_setup_rx_descriptors(&rx_rings[i]);
			if (err) {
				while (i) {
					i--;
					i40e_free_rx_resources(&rx_rings[i]);
				}
				kfree(rx_rings);
				rx_rings = NULL;

				goto free_tx;
			}
		}
	}

	/* Bring interface down, copy in the new ring info,
	 * then restore the interface
	 */
	i40e_down(vsi);

	if (tx_rings) {
		for (i = 0; i < vsi->num_queue_pairs; i++) {
			i40e_free_tx_resources(vsi->tx_rings[i]);
			*vsi->tx_rings[i] = tx_rings[i];
		}
		kfree(tx_rings);
		tx_rings = NULL;
	}

	if (rx_rings) {
		for (i = 0; i < vsi->num_queue_pairs; i++) {
			i40e_free_rx_resources(vsi->rx_rings[i]);
			*vsi->rx_rings[i] = rx_rings[i];
		}
		kfree(rx_rings);
		rx_rings = NULL;
	}

	i40e_up(vsi);

free_tx:
	/* error cleanup if the Rx allocations failed after getting Tx */
	if (tx_rings) {
		for (i = 0; i < vsi->num_queue_pairs; i++)
			i40e_free_tx_resources(&tx_rings[i]);
		kfree(tx_rings);
		tx_rings = NULL;
	}

done:
	clear_bit(__I40E_CONFIG_BUSY, &pf->state);

	return err;
}

static int i40e_get_sset_count(struct net_device *netdev, int sset)
{
	struct i40e_netdev_priv *np = netdev_priv(netdev);
	struct i40e_vsi *vsi = np->vsi;
	struct i40e_pf *pf = vsi->back;

	switch (sset) {
	case ETH_SS_TEST:
		return I40E_TEST_LEN;
	case ETH_SS_STATS:
		if (vsi == pf->vsi[pf->lan_vsi] && pf->hw.partition_id == 1) {
			int len = I40E_PF_STATS_LEN(netdev);

			if (pf->lan_veb != I40E_NO_VEB)
				len += I40E_VEB_STATS_LEN;
			return len;
		} else {
			return I40E_VSI_STATS_LEN(netdev);
		}
	case ETH_SS_PRIV_FLAGS:
		return I40E_PRIV_FLAGS_STR_LEN;
	default:
		return -EOPNOTSUPP;
	}
}

static void i40e_get_ethtool_stats(struct net_device *netdev,
				   struct ethtool_stats *stats, u64 *data)
{
	struct i40e_netdev_priv *np = netdev_priv(netdev);
	struct i40e_ring *tx_ring, *rx_ring;
	struct i40e_vsi *vsi = np->vsi;
	struct i40e_pf *pf = vsi->back;
	int i = 0;
	char *p;
	int j;
	struct rtnl_link_stats64 *net_stats = i40e_get_vsi_stats_struct(vsi);
	unsigned int start;

	i40e_update_stats(vsi);

	for (j = 0; j < I40E_NETDEV_STATS_LEN; j++) {
		p = (char *)net_stats + i40e_gstrings_net_stats[j].stat_offset;
		data[i++] = (i40e_gstrings_net_stats[j].sizeof_stat ==
			sizeof(u64)) ? *(u64 *)p : *(u32 *)p;
	}
	for (j = 0; j < I40E_MISC_STATS_LEN; j++) {
		p = (char *)vsi + i40e_gstrings_misc_stats[j].stat_offset;
		data[i++] = (i40e_gstrings_misc_stats[j].sizeof_stat ==
			    sizeof(u64)) ? *(u64 *)p : *(u32 *)p;
	}
#ifdef I40E_FCOE
	for (j = 0; j < I40E_FCOE_STATS_LEN; j++) {
		p = (char *)vsi + i40e_gstrings_fcoe_stats[j].stat_offset;
		data[i++] = (i40e_gstrings_fcoe_stats[j].sizeof_stat ==
			sizeof(u64)) ? *(u64 *)p : *(u32 *)p;
	}
#endif
	rcu_read_lock();
	for (j = 0; j < vsi->num_queue_pairs; j++) {
		tx_ring = ACCESS_ONCE(vsi->tx_rings[j]);

		if (!tx_ring)
			continue;

		/* process Tx ring statistics */
		do {
			start = u64_stats_fetch_begin_irq(&tx_ring->syncp);
			data[i] = tx_ring->stats.packets;
			data[i + 1] = tx_ring->stats.bytes;
		} while (u64_stats_fetch_retry_irq(&tx_ring->syncp, start));
		i += 2;

		/* Rx ring is the 2nd half of the queue pair */
		rx_ring = &tx_ring[1];
		do {
			start = u64_stats_fetch_begin_irq(&rx_ring->syncp);
			data[i] = rx_ring->stats.packets;
			data[i + 1] = rx_ring->stats.bytes;
		} while (u64_stats_fetch_retry_irq(&rx_ring->syncp, start));
		i += 2;
	}
	rcu_read_unlock();
	if (vsi != pf->vsi[pf->lan_vsi] || pf->hw.partition_id != 1)
		return;

	if (pf->lan_veb != I40E_NO_VEB) {
		struct i40e_veb *veb = pf->veb[pf->lan_veb];
		for (j = 0; j < I40E_VEB_STATS_LEN; j++) {
			p = (char *)veb;
			p += i40e_gstrings_veb_stats[j].stat_offset;
			data[i++] = (i40e_gstrings_veb_stats[j].sizeof_stat ==
				     sizeof(u64)) ? *(u64 *)p : *(u32 *)p;
		}
	}
	for (j = 0; j < I40E_GLOBAL_STATS_LEN; j++) {
		p = (char *)pf + i40e_gstrings_stats[j].stat_offset;
		data[i++] = (i40e_gstrings_stats[j].sizeof_stat ==
			     sizeof(u64)) ? *(u64 *)p : *(u32 *)p;
	}
	for (j = 0; j < I40E_MAX_USER_PRIORITY; j++) {
		data[i++] = pf->stats.priority_xon_tx[j];
		data[i++] = pf->stats.priority_xoff_tx[j];
	}
	for (j = 0; j < I40E_MAX_USER_PRIORITY; j++) {
		data[i++] = pf->stats.priority_xon_rx[j];
		data[i++] = pf->stats.priority_xoff_rx[j];
	}
	for (j = 0; j < I40E_MAX_USER_PRIORITY; j++)
		data[i++] = pf->stats.priority_xon_2_xoff[j];
}

static void i40e_get_strings(struct net_device *netdev, u32 stringset,
			     u8 *data)
{
	struct i40e_netdev_priv *np = netdev_priv(netdev);
	struct i40e_vsi *vsi = np->vsi;
	struct i40e_pf *pf = vsi->back;
	char *p = (char *)data;
	int i;

	switch (stringset) {
	case ETH_SS_TEST:
		for (i = 0; i < I40E_TEST_LEN; i++) {
			memcpy(data, i40e_gstrings_test[i], ETH_GSTRING_LEN);
			data += ETH_GSTRING_LEN;
		}
		break;
	case ETH_SS_STATS:
		for (i = 0; i < I40E_NETDEV_STATS_LEN; i++) {
			snprintf(p, ETH_GSTRING_LEN, "%s",
				 i40e_gstrings_net_stats[i].stat_string);
			p += ETH_GSTRING_LEN;
		}
		for (i = 0; i < I40E_MISC_STATS_LEN; i++) {
			snprintf(p, ETH_GSTRING_LEN, "%s",
				 i40e_gstrings_misc_stats[i].stat_string);
			p += ETH_GSTRING_LEN;
		}
#ifdef I40E_FCOE
		for (i = 0; i < I40E_FCOE_STATS_LEN; i++) {
			snprintf(p, ETH_GSTRING_LEN, "%s",
				 i40e_gstrings_fcoe_stats[i].stat_string);
			p += ETH_GSTRING_LEN;
		}
#endif
		for (i = 0; i < vsi->num_queue_pairs; i++) {
			snprintf(p, ETH_GSTRING_LEN, "tx-%u.tx_packets", i);
			p += ETH_GSTRING_LEN;
			snprintf(p, ETH_GSTRING_LEN, "tx-%u.tx_bytes", i);
			p += ETH_GSTRING_LEN;
			snprintf(p, ETH_GSTRING_LEN, "rx-%u.rx_packets", i);
			p += ETH_GSTRING_LEN;
			snprintf(p, ETH_GSTRING_LEN, "rx-%u.rx_bytes", i);
			p += ETH_GSTRING_LEN;
		}
		if (vsi != pf->vsi[pf->lan_vsi] || pf->hw.partition_id != 1)
			return;

		if (pf->lan_veb != I40E_NO_VEB) {
			for (i = 0; i < I40E_VEB_STATS_LEN; i++) {
				snprintf(p, ETH_GSTRING_LEN, "veb.%s",
					i40e_gstrings_veb_stats[i].stat_string);
				p += ETH_GSTRING_LEN;
			}
		}
		for (i = 0; i < I40E_GLOBAL_STATS_LEN; i++) {
			snprintf(p, ETH_GSTRING_LEN, "port.%s",
				 i40e_gstrings_stats[i].stat_string);
			p += ETH_GSTRING_LEN;
		}
		for (i = 0; i < I40E_MAX_USER_PRIORITY; i++) {
			snprintf(p, ETH_GSTRING_LEN,
				 "port.tx_priority_%u_xon", i);
			p += ETH_GSTRING_LEN;
			snprintf(p, ETH_GSTRING_LEN,
				 "port.tx_priority_%u_xoff", i);
			p += ETH_GSTRING_LEN;
		}
		for (i = 0; i < I40E_MAX_USER_PRIORITY; i++) {
			snprintf(p, ETH_GSTRING_LEN,
				 "port.rx_priority_%u_xon", i);
			p += ETH_GSTRING_LEN;
			snprintf(p, ETH_GSTRING_LEN,
				 "port.rx_priority_%u_xoff", i);
			p += ETH_GSTRING_LEN;
		}
		for (i = 0; i < I40E_MAX_USER_PRIORITY; i++) {
			snprintf(p, ETH_GSTRING_LEN,
				 "port.rx_priority_%u_xon_2_xoff", i);
			p += ETH_GSTRING_LEN;
		}
		/* BUG_ON(p - data != I40E_STATS_LEN * ETH_GSTRING_LEN); */
		break;
	case ETH_SS_PRIV_FLAGS:
		for (i = 0; i < I40E_PRIV_FLAGS_STR_LEN; i++) {
			memcpy(data, i40e_priv_flags_strings[i],
			       ETH_GSTRING_LEN);
			data += ETH_GSTRING_LEN;
		}
		break;
	default:
		break;
	}
}

static int i40e_get_ts_info(struct net_device *dev,
			    struct ethtool_ts_info *info)
{
	struct i40e_pf *pf = i40e_netdev_to_pf(dev);

	/* only report HW timestamping if PTP is enabled */
	if (!(pf->flags & I40E_FLAG_PTP))
		return ethtool_op_get_ts_info(dev, info);

	info->so_timestamping = SOF_TIMESTAMPING_TX_SOFTWARE |
				SOF_TIMESTAMPING_RX_SOFTWARE |
				SOF_TIMESTAMPING_SOFTWARE |
				SOF_TIMESTAMPING_TX_HARDWARE |
				SOF_TIMESTAMPING_RX_HARDWARE |
				SOF_TIMESTAMPING_RAW_HARDWARE;

	if (pf->ptp_clock)
		info->phc_index = ptp_clock_index(pf->ptp_clock);
	else
		info->phc_index = -1;

	info->tx_types = (1 << HWTSTAMP_TX_OFF) | (1 << HWTSTAMP_TX_ON);

	info->rx_filters = (1 << HWTSTAMP_FILTER_NONE) |
			   (1 << HWTSTAMP_FILTER_PTP_V1_L4_SYNC) |
			   (1 << HWTSTAMP_FILTER_PTP_V1_L4_DELAY_REQ) |
			   (1 << HWTSTAMP_FILTER_PTP_V2_EVENT) |
			   (1 << HWTSTAMP_FILTER_PTP_V2_L2_EVENT) |
			   (1 << HWTSTAMP_FILTER_PTP_V2_L4_EVENT) |
			   (1 << HWTSTAMP_FILTER_PTP_V2_SYNC) |
			   (1 << HWTSTAMP_FILTER_PTP_V2_L2_SYNC) |
			   (1 << HWTSTAMP_FILTER_PTP_V2_L4_SYNC) |
			   (1 << HWTSTAMP_FILTER_PTP_V2_DELAY_REQ) |
			   (1 << HWTSTAMP_FILTER_PTP_V2_L2_DELAY_REQ) |
			   (1 << HWTSTAMP_FILTER_PTP_V2_L4_DELAY_REQ);

	return 0;
}

static int i40e_link_test(struct net_device *netdev, u64 *data)
{
	struct i40e_netdev_priv *np = netdev_priv(netdev);
	struct i40e_pf *pf = np->vsi->back;

	netif_info(pf, hw, netdev, "link test\n");
	if (i40e_get_link_status(&pf->hw))
		*data = 0;
	else
		*data = 1;

	return *data;
}

static int i40e_reg_test(struct net_device *netdev, u64 *data)
{
	struct i40e_netdev_priv *np = netdev_priv(netdev);
	struct i40e_pf *pf = np->vsi->back;

	netif_info(pf, hw, netdev, "register test\n");
	*data = i40e_diag_reg_test(&pf->hw);

	return *data;
}

static int i40e_eeprom_test(struct net_device *netdev, u64 *data)
{
	struct i40e_netdev_priv *np = netdev_priv(netdev);
	struct i40e_pf *pf = np->vsi->back;

	netif_info(pf, hw, netdev, "eeprom test\n");
	*data = i40e_diag_eeprom_test(&pf->hw);

	/* forcebly clear the NVM Update state machine */
	pf->hw.nvmupd_state = I40E_NVMUPD_STATE_INIT;

	return *data;
}

static int i40e_intr_test(struct net_device *netdev, u64 *data)
{
	struct i40e_netdev_priv *np = netdev_priv(netdev);
	struct i40e_pf *pf = np->vsi->back;
	u16 swc_old = pf->sw_int_count;

	netif_info(pf, hw, netdev, "interrupt test\n");
	wr32(&pf->hw, I40E_PFINT_DYN_CTL0,
	     (I40E_PFINT_DYN_CTL0_INTENA_MASK |
	      I40E_PFINT_DYN_CTL0_SWINT_TRIG_MASK |
	      I40E_PFINT_DYN_CTL0_ITR_INDX_MASK |
	      I40E_PFINT_DYN_CTL0_SW_ITR_INDX_ENA_MASK |
	      I40E_PFINT_DYN_CTL0_SW_ITR_INDX_MASK));
	usleep_range(1000, 2000);
	*data = (swc_old == pf->sw_int_count);

	return *data;
}

static int i40e_loopback_test(struct net_device *netdev, u64 *data)
{
	struct i40e_netdev_priv *np = netdev_priv(netdev);
	struct i40e_pf *pf = np->vsi->back;

	netif_info(pf, hw, netdev, "loopback test not implemented\n");
	*data = 0;

	return *data;
}

static void i40e_diag_test(struct net_device *netdev,
			   struct ethtool_test *eth_test, u64 *data)
{
	struct i40e_netdev_priv *np = netdev_priv(netdev);
	bool if_running = netif_running(netdev);
	struct i40e_pf *pf = np->vsi->back;

	if (eth_test->flags == ETH_TEST_FL_OFFLINE) {
		/* Offline tests */
		netif_info(pf, drv, netdev, "offline testing starting\n");

		set_bit(__I40E_TESTING, &pf->state);
		/* If the device is online then take it offline */
		if (if_running)
			/* indicate we're in test mode */
			dev_close(netdev);
		else
			i40e_do_reset(pf, (1 << __I40E_PF_RESET_REQUESTED));

		/* Link test performed before hardware reset
		 * so autoneg doesn't interfere with test result
		 */
		if (i40e_link_test(netdev, &data[I40E_ETH_TEST_LINK]))
			eth_test->flags |= ETH_TEST_FL_FAILED;

		if (i40e_eeprom_test(netdev, &data[I40E_ETH_TEST_EEPROM]))
			eth_test->flags |= ETH_TEST_FL_FAILED;

		if (i40e_intr_test(netdev, &data[I40E_ETH_TEST_INTR]))
			eth_test->flags |= ETH_TEST_FL_FAILED;

		if (i40e_loopback_test(netdev, &data[I40E_ETH_TEST_LOOPBACK]))
			eth_test->flags |= ETH_TEST_FL_FAILED;

		/* run reg test last, a reset is required after it */
		if (i40e_reg_test(netdev, &data[I40E_ETH_TEST_REG]))
			eth_test->flags |= ETH_TEST_FL_FAILED;

		clear_bit(__I40E_TESTING, &pf->state);
		i40e_do_reset(pf, (1 << __I40E_PF_RESET_REQUESTED));

		if (if_running)
			dev_open(netdev);
	} else {
		/* Online tests */
		netif_info(pf, drv, netdev, "online testing starting\n");

		if (i40e_link_test(netdev, &data[I40E_ETH_TEST_LINK]))
			eth_test->flags |= ETH_TEST_FL_FAILED;

		/* Offline only tests, not run in online; pass by default */
		data[I40E_ETH_TEST_REG] = 0;
		data[I40E_ETH_TEST_EEPROM] = 0;
		data[I40E_ETH_TEST_INTR] = 0;
		data[I40E_ETH_TEST_LOOPBACK] = 0;
	}

	netif_info(pf, drv, netdev, "testing finished\n");
}

static void i40e_get_wol(struct net_device *netdev,
			 struct ethtool_wolinfo *wol)
{
	struct i40e_netdev_priv *np = netdev_priv(netdev);
	struct i40e_pf *pf = np->vsi->back;
	struct i40e_hw *hw = &pf->hw;
	u16 wol_nvm_bits;

	/* NVM bit on means WoL disabled for the port */
	i40e_read_nvm_word(hw, I40E_SR_NVM_WAKE_ON_LAN, &wol_nvm_bits);
	if ((1 << hw->port) & wol_nvm_bits || hw->partition_id != 1) {
		wol->supported = 0;
		wol->wolopts = 0;
	} else {
		wol->supported = WAKE_MAGIC;
		wol->wolopts = (pf->wol_en ? WAKE_MAGIC : 0);
	}
}

/**
 * i40e_set_wol - set the WakeOnLAN configuration
 * @netdev: the netdev in question
 * @wol: the ethtool WoL setting data
 **/
static int i40e_set_wol(struct net_device *netdev, struct ethtool_wolinfo *wol)
{
	struct i40e_netdev_priv *np = netdev_priv(netdev);
	struct i40e_pf *pf = np->vsi->back;
	struct i40e_vsi *vsi = np->vsi;
	struct i40e_hw *hw = &pf->hw;
	u16 wol_nvm_bits;

	/* WoL not supported if this isn't the controlling PF on the port */
	if (hw->partition_id != 1) {
		i40e_partition_setting_complaint(pf);
		return -EOPNOTSUPP;
	}

	if (vsi != pf->vsi[pf->lan_vsi])
		return -EOPNOTSUPP;

	/* NVM bit on means WoL disabled for the port */
	i40e_read_nvm_word(hw, I40E_SR_NVM_WAKE_ON_LAN, &wol_nvm_bits);
	if (((1 << hw->port) & wol_nvm_bits))
		return -EOPNOTSUPP;

	/* only magic packet is supported */
	if (wol->wolopts && (wol->wolopts != WAKE_MAGIC))
		return -EOPNOTSUPP;

	/* is this a new value? */
	if (pf->wol_en != !!wol->wolopts) {
		pf->wol_en = !!wol->wolopts;
		device_set_wakeup_enable(&pf->pdev->dev, pf->wol_en);
	}

	return 0;
}

static int i40e_set_phys_id(struct net_device *netdev,
			    enum ethtool_phys_id_state state)
{
	struct i40e_netdev_priv *np = netdev_priv(netdev);
	struct i40e_pf *pf = np->vsi->back;
	struct i40e_hw *hw = &pf->hw;
	int blink_freq = 2;

	switch (state) {
	case ETHTOOL_ID_ACTIVE:
		pf->led_status = i40e_led_get(hw);
		return blink_freq;
	case ETHTOOL_ID_ON:
		i40e_led_set(hw, 0xF, false);
		break;
	case ETHTOOL_ID_OFF:
		i40e_led_set(hw, 0x0, false);
		break;
	case ETHTOOL_ID_INACTIVE:
		i40e_led_set(hw, pf->led_status, false);
		break;
	default:
		break;
	}

	return 0;
}

/* NOTE: i40e hardware uses a conversion factor of 2 for Interrupt
 * Throttle Rate (ITR) ie. ITR(1) = 2us ITR(10) = 20 us, and also
 * 125us (8000 interrupts per second) == ITR(62)
 */

static int i40e_get_coalesce(struct net_device *netdev,
			     struct ethtool_coalesce *ec)
{
	struct i40e_netdev_priv *np = netdev_priv(netdev);
	struct i40e_vsi *vsi = np->vsi;

	ec->tx_max_coalesced_frames_irq = vsi->work_limit;
	ec->rx_max_coalesced_frames_irq = vsi->work_limit;

	if (ITR_IS_DYNAMIC(vsi->rx_itr_setting))
		ec->use_adaptive_rx_coalesce = 1;

	if (ITR_IS_DYNAMIC(vsi->tx_itr_setting))
		ec->use_adaptive_tx_coalesce = 1;

	ec->rx_coalesce_usecs = vsi->rx_itr_setting & ~I40E_ITR_DYNAMIC;
	ec->tx_coalesce_usecs = vsi->tx_itr_setting & ~I40E_ITR_DYNAMIC;

	return 0;
}

static int i40e_set_coalesce(struct net_device *netdev,
			     struct ethtool_coalesce *ec)
{
	struct i40e_netdev_priv *np = netdev_priv(netdev);
	struct i40e_q_vector *q_vector;
	struct i40e_vsi *vsi = np->vsi;
	struct i40e_pf *pf = vsi->back;
	struct i40e_hw *hw = &pf->hw;
	u16 vector;
	int i;

	if (ec->tx_max_coalesced_frames_irq || ec->rx_max_coalesced_frames_irq)
		vsi->work_limit = ec->tx_max_coalesced_frames_irq;

	vector = vsi->base_vector;
	if ((ec->rx_coalesce_usecs >= (I40E_MIN_ITR << 1)) &&
	    (ec->rx_coalesce_usecs <= (I40E_MAX_ITR << 1))) {
		vsi->rx_itr_setting = ec->rx_coalesce_usecs;
	} else if (ec->rx_coalesce_usecs == 0) {
		vsi->rx_itr_setting = ec->rx_coalesce_usecs;
		if (ec->use_adaptive_rx_coalesce)
			netif_info(pf, drv, netdev, "rx-usecs=0, need to disable adaptive-rx for a complete disable\n");
	} else {
		netif_info(pf, drv, netdev, "Invalid value, rx-usecs range is 0-8160\n");
		return -EINVAL;
	}

	if ((ec->tx_coalesce_usecs >= (I40E_MIN_ITR << 1)) &&
	    (ec->tx_coalesce_usecs <= (I40E_MAX_ITR << 1))) {
		vsi->tx_itr_setting = ec->tx_coalesce_usecs;
	} else if (ec->tx_coalesce_usecs == 0) {
		vsi->tx_itr_setting = ec->tx_coalesce_usecs;
		if (ec->use_adaptive_tx_coalesce)
			netif_info(pf, drv, netdev, "tx-usecs=0, need to disable adaptive-tx for a complete disable\n");
	} else {
		netif_info(pf, drv, netdev,
			   "Invalid value, tx-usecs range is 0-8160\n");
		return -EINVAL;
	}

	if (ec->use_adaptive_rx_coalesce)
		vsi->rx_itr_setting |= I40E_ITR_DYNAMIC;
	else
		vsi->rx_itr_setting &= ~I40E_ITR_DYNAMIC;

	if (ec->use_adaptive_tx_coalesce)
		vsi->tx_itr_setting |= I40E_ITR_DYNAMIC;
	else
		vsi->tx_itr_setting &= ~I40E_ITR_DYNAMIC;

	for (i = 0; i < vsi->num_q_vectors; i++, vector++) {
		q_vector = vsi->q_vectors[i];
		q_vector->rx.itr = ITR_TO_REG(vsi->rx_itr_setting);
		wr32(hw, I40E_PFINT_ITRN(0, vector - 1), q_vector->rx.itr);
		q_vector->tx.itr = ITR_TO_REG(vsi->tx_itr_setting);
		wr32(hw, I40E_PFINT_ITRN(1, vector - 1), q_vector->tx.itr);
		i40e_flush(hw);
	}

	return 0;
}

/**
 * i40e_get_rss_hash_opts - Get RSS hash Input Set for each flow type
 * @pf: pointer to the physical function struct
 * @cmd: ethtool rxnfc command
 *
 * Returns Success if the flow is supported, else Invalid Input.
 **/
static int i40e_get_rss_hash_opts(struct i40e_pf *pf, struct ethtool_rxnfc *cmd)
{
	cmd->data = 0;

	if (pf->vsi[pf->lan_vsi]->rxnfc.data != 0) {
		cmd->data = pf->vsi[pf->lan_vsi]->rxnfc.data;
		cmd->flow_type = pf->vsi[pf->lan_vsi]->rxnfc.flow_type;
		return 0;
	}
	/* Report default options for RSS on i40e */
	switch (cmd->flow_type) {
	case TCP_V4_FLOW:
	case UDP_V4_FLOW:
		cmd->data |= RXH_L4_B_0_1 | RXH_L4_B_2_3;
	/* fall through to add IP fields */
	case SCTP_V4_FLOW:
	case AH_ESP_V4_FLOW:
	case AH_V4_FLOW:
	case ESP_V4_FLOW:
	case IPV4_FLOW:
		cmd->data |= RXH_IP_SRC | RXH_IP_DST;
		break;
	case TCP_V6_FLOW:
	case UDP_V6_FLOW:
		cmd->data |= RXH_L4_B_0_1 | RXH_L4_B_2_3;
	/* fall through to add IP fields */
	case SCTP_V6_FLOW:
	case AH_ESP_V6_FLOW:
	case AH_V6_FLOW:
	case ESP_V6_FLOW:
	case IPV6_FLOW:
		cmd->data |= RXH_IP_SRC | RXH_IP_DST;
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

/**
 * i40e_get_ethtool_fdir_all - Populates the rule count of a command
 * @pf: Pointer to the physical function struct
 * @cmd: The command to get or set Rx flow classification rules
 * @rule_locs: Array of used rule locations
 *
 * This function populates both the total and actual rule count of
 * the ethtool flow classification command
 *
 * Returns 0 on success or -EMSGSIZE if entry not found
 **/
static int i40e_get_ethtool_fdir_all(struct i40e_pf *pf,
				     struct ethtool_rxnfc *cmd,
				     u32 *rule_locs)
{
	struct i40e_fdir_filter *rule;
	struct hlist_node *node2;
	int cnt = 0;

	/* report total rule count */
	cmd->data = i40e_get_fd_cnt_all(pf);

	hlist_for_each_entry_safe(rule, node2,
				  &pf->fdir_filter_list, fdir_node) {
		if (cnt == cmd->rule_cnt)
			return -EMSGSIZE;

		rule_locs[cnt] = rule->fd_id;
		cnt++;
	}

	cmd->rule_cnt = cnt;

	return 0;
}

/**
 * i40e_get_ethtool_fdir_entry - Look up a filter based on Rx flow
 * @pf: Pointer to the physical function struct
 * @cmd: The command to get or set Rx flow classification rules
 *
 * This function looks up a filter based on the Rx flow classification
 * command and fills the flow spec info for it if found
 *
 * Returns 0 on success or -EINVAL if filter not found
 **/
static int i40e_get_ethtool_fdir_entry(struct i40e_pf *pf,
				       struct ethtool_rxnfc *cmd)
{
	struct ethtool_rx_flow_spec *fsp =
			(struct ethtool_rx_flow_spec *)&cmd->fs;
	struct i40e_fdir_filter *rule = NULL;
	struct hlist_node *node2;

	hlist_for_each_entry_safe(rule, node2,
				  &pf->fdir_filter_list, fdir_node) {
		if (fsp->location <= rule->fd_id)
			break;
	}

	if (!rule || fsp->location != rule->fd_id)
		return -EINVAL;

	fsp->flow_type = rule->flow_type;
	if (fsp->flow_type == IP_USER_FLOW) {
		fsp->h_u.usr_ip4_spec.ip_ver = ETH_RX_NFC_IP4;
		fsp->h_u.usr_ip4_spec.proto = 0;
		fsp->m_u.usr_ip4_spec.proto = 0;
	}

	/* Reverse the src and dest notion, since the HW views them from
	 * Tx perspective where as the user expects it from Rx filter view.
	 */
	fsp->h_u.tcp_ip4_spec.psrc = rule->dst_port;
	fsp->h_u.tcp_ip4_spec.pdst = rule->src_port;
	fsp->h_u.tcp_ip4_spec.ip4src = rule->dst_ip[0];
	fsp->h_u.tcp_ip4_spec.ip4dst = rule->src_ip[0];

	if (rule->dest_ctl == I40E_FILTER_PROGRAM_DESC_DEST_DROP_PACKET)
		fsp->ring_cookie = RX_CLS_FLOW_DISC;
	else
		fsp->ring_cookie = rule->q_index;

	if (rule->dest_vsi != pf->vsi[pf->lan_vsi]->id) {
		struct i40e_vsi *vsi;

		vsi = i40e_find_vsi_from_id(pf, rule->dest_vsi);
		if (vsi && vsi->type == I40E_VSI_SRIOV) {
			fsp->h_ext.data[1] = htonl(vsi->vf_id);
			fsp->m_ext.data[1] = htonl(0x1);
		}
	}

	return 0;
}

/**
 * i40e_get_rxnfc - command to get RX flow classification rules
 * @netdev: network interface device structure
 * @cmd: ethtool rxnfc command
 *
 * Returns Success if the command is supported.
 **/
static int i40e_get_rxnfc(struct net_device *netdev, struct ethtool_rxnfc *cmd,
			  u32 *rule_locs)
{
	struct i40e_netdev_priv *np = netdev_priv(netdev);
	struct i40e_vsi *vsi = np->vsi;
	struct i40e_pf *pf = vsi->back;
	int ret = -EOPNOTSUPP;

	switch (cmd->cmd) {
	case ETHTOOL_GRXRINGS:
		cmd->data = vsi->alloc_queue_pairs;
		ret = 0;
		break;
	case ETHTOOL_GRXFH:
		ret = i40e_get_rss_hash_opts(pf, cmd);
		break;
	case ETHTOOL_GRXCLSRLCNT:
		cmd->rule_cnt = pf->fdir_pf_active_filters;
		/* report total rule count */
		cmd->data = i40e_get_fd_cnt_all(pf);
		ret = 0;
		break;
	case ETHTOOL_GRXCLSRULE:
		ret = i40e_get_ethtool_fdir_entry(pf, cmd);
		break;
	case ETHTOOL_GRXCLSRLALL:
		ret = i40e_get_ethtool_fdir_all(pf, cmd, rule_locs);
		break;
	default:
		break;
	}

	return ret;
}

/**
 * i40e_set_rss_hash_opt - Enable/Disable flow types for RSS hash
 * @pf: pointer to the physical function struct
 * @cmd: ethtool rxnfc command
 *
 * Returns Success if the flow input set is supported.
 **/
static int i40e_set_rss_hash_opt(struct i40e_pf *pf, struct ethtool_rxnfc *nfc)
{
	struct i40e_hw *hw = &pf->hw;
	u64 hena = (u64)rd32(hw, I40E_PFQF_HENA(0)) |
		   ((u64)rd32(hw, I40E_PFQF_HENA(1)) << 32);

	/* RSS does not support anything other than hashing
	 * to queues on src and dst IPs and ports
	 */
	if (nfc->data & ~(RXH_IP_SRC | RXH_IP_DST |
			  RXH_L4_B_0_1 | RXH_L4_B_2_3))
		return -EINVAL;

	/* We need at least the IP SRC and DEST fields for hashing */
	if (!(nfc->data & RXH_IP_SRC) ||
	    !(nfc->data & RXH_IP_DST))
		return -EINVAL;

	switch (nfc->flow_type) {
	case TCP_V4_FLOW:
		switch (nfc->data & (RXH_L4_B_0_1 | RXH_L4_B_2_3)) {
		case 0:
			hena &= ~((u64)1 << I40E_FILTER_PCTYPE_NONF_IPV4_TCP);
			break;
		case (RXH_L4_B_0_1 | RXH_L4_B_2_3):
			hena |= ((u64)1 << I40E_FILTER_PCTYPE_NONF_IPV4_TCP);
			break;
		default:
			return -EINVAL;
		}
		break;
	case TCP_V6_FLOW:
		switch (nfc->data & (RXH_L4_B_0_1 | RXH_L4_B_2_3)) {
		case 0:
			hena &= ~((u64)1 << I40E_FILTER_PCTYPE_NONF_IPV6_TCP);
			break;
		case (RXH_L4_B_0_1 | RXH_L4_B_2_3):
			hena |= ((u64)1 << I40E_FILTER_PCTYPE_NONF_IPV6_TCP);
			break;
		default:
			return -EINVAL;
		}
		break;
	case UDP_V4_FLOW:
		switch (nfc->data & (RXH_L4_B_0_1 | RXH_L4_B_2_3)) {
		case 0:
			hena &= ~(((u64)1 << I40E_FILTER_PCTYPE_NONF_IPV4_UDP) |
				  ((u64)1 << I40E_FILTER_PCTYPE_FRAG_IPV4));
			break;
		case (RXH_L4_B_0_1 | RXH_L4_B_2_3):
			hena |= (((u64)1 << I40E_FILTER_PCTYPE_NONF_IPV4_UDP) |
				  ((u64)1 << I40E_FILTER_PCTYPE_FRAG_IPV4));
			break;
		default:
			return -EINVAL;
		}
		break;
	case UDP_V6_FLOW:
		switch (nfc->data & (RXH_L4_B_0_1 | RXH_L4_B_2_3)) {
		case 0:
			hena &= ~(((u64)1 << I40E_FILTER_PCTYPE_NONF_IPV6_UDP) |
				  ((u64)1 << I40E_FILTER_PCTYPE_FRAG_IPV6));
			break;
		case (RXH_L4_B_0_1 | RXH_L4_B_2_3):
			hena |= (((u64)1 << I40E_FILTER_PCTYPE_NONF_IPV6_UDP) |
				 ((u64)1 << I40E_FILTER_PCTYPE_FRAG_IPV6));
			break;
		default:
			return -EINVAL;
		}
		break;
	case AH_ESP_V4_FLOW:
	case AH_V4_FLOW:
	case ESP_V4_FLOW:
	case SCTP_V4_FLOW:
		if ((nfc->data & RXH_L4_B_0_1) ||
		    (nfc->data & RXH_L4_B_2_3))
			return -EINVAL;
		hena |= ((u64)1 << I40E_FILTER_PCTYPE_NONF_IPV4_OTHER);
		break;
	case AH_ESP_V6_FLOW:
	case AH_V6_FLOW:
	case ESP_V6_FLOW:
	case SCTP_V6_FLOW:
		if ((nfc->data & RXH_L4_B_0_1) ||
		    (nfc->data & RXH_L4_B_2_3))
			return -EINVAL;
		hena |= ((u64)1 << I40E_FILTER_PCTYPE_NONF_IPV6_OTHER);
		break;
	case IPV4_FLOW:
		hena |= ((u64)1 << I40E_FILTER_PCTYPE_NONF_IPV4_OTHER) |
			((u64)1 << I40E_FILTER_PCTYPE_FRAG_IPV4);
		break;
	case IPV6_FLOW:
		hena |= ((u64)1 << I40E_FILTER_PCTYPE_NONF_IPV6_OTHER) |
			((u64)1 << I40E_FILTER_PCTYPE_FRAG_IPV6);
		break;
	default:
		return -EINVAL;
	}

	wr32(hw, I40E_PFQF_HENA(0), (u32)hena);
	wr32(hw, I40E_PFQF_HENA(1), (u32)(hena >> 32));
	i40e_flush(hw);

	/* Save setting for future output/update */
	pf->vsi[pf->lan_vsi]->rxnfc = *nfc;

	return 0;
}

/**
 * i40e_match_fdir_input_set - Match a new filter against an existing one
 * @rule: The filter already added
 * @input: The new filter to comapre against
 *
 * Returns true if the two input set match
 **/
static bool i40e_match_fdir_input_set(struct i40e_fdir_filter *rule,
				      struct i40e_fdir_filter *input)
{
	if ((rule->dst_ip[0] != input->dst_ip[0]) ||
	    (rule->src_ip[0] != input->src_ip[0]) ||
	    (rule->dst_port != input->dst_port) ||
	    (rule->src_port != input->src_port))
		return false;
	return true;
}

/**
 * i40e_update_ethtool_fdir_entry - Updates the fdir filter entry
 * @vsi: Pointer to the targeted VSI
 * @input: The filter to update or NULL to indicate deletion
 * @sw_idx: Software index to the filter
 * @cmd: The command to get or set Rx flow classification rules
 *
 * This function updates (or deletes) a Flow Director entry from
 * the hlist of the corresponding PF
 *
 * Returns 0 on success
 **/
static int i40e_update_ethtool_fdir_entry(struct i40e_vsi *vsi,
					  struct i40e_fdir_filter *input,
					  u16 sw_idx,
					  struct ethtool_rxnfc *cmd)
{
	struct i40e_fdir_filter *rule, *parent;
	struct i40e_pf *pf = vsi->back;
	struct hlist_node *node2;
	int err = -EINVAL;

	parent = NULL;
	rule = NULL;

	hlist_for_each_entry_safe(rule, node2,
				  &pf->fdir_filter_list, fdir_node) {
		/* hash found, or no matching entry */
		if (rule->fd_id >= sw_idx)
			break;
		parent = rule;
	}

	/* if there is an old rule occupying our place remove it */
	if (rule && (rule->fd_id == sw_idx)) {
		if (input && !i40e_match_fdir_input_set(rule, input))
			err = i40e_add_del_fdir(vsi, rule, false);
		else if (!input)
			err = i40e_add_del_fdir(vsi, rule, false);
		hlist_del(&rule->fdir_node);
		kfree(rule);
		pf->fdir_pf_active_filters--;
	}

	/* If no input this was a delete, err should be 0 if a rule was
	 * successfully found and removed from the list else -EINVAL
	 */
	if (!input)
		return err;

	/* initialize node and set software index */
	INIT_HLIST_NODE(&input->fdir_node);

	/* add filter to the list */
	if (parent)
		hlist_add_behind(&input->fdir_node, &parent->fdir_node);
	else
		hlist_add_head(&input->fdir_node,
			       &pf->fdir_filter_list);

	/* update counts */
	pf->fdir_pf_active_filters++;

	return 0;
}

/**
 * i40e_del_fdir_entry - Deletes a Flow Director filter entry
 * @vsi: Pointer to the targeted VSI
 * @cmd: The command to get or set Rx flow classification rules
 *
 * The function removes a Flow Director filter entry from the
 * hlist of the corresponding PF
 *
 * Returns 0 on success
 */
static int i40e_del_fdir_entry(struct i40e_vsi *vsi,
			       struct ethtool_rxnfc *cmd)
{
	struct ethtool_rx_flow_spec *fsp =
		(struct ethtool_rx_flow_spec *)&cmd->fs;
	struct i40e_pf *pf = vsi->back;
	int ret = 0;

	if (test_bit(__I40E_RESET_RECOVERY_PENDING, &pf->state) ||
	    test_bit(__I40E_RESET_INTR_RECEIVED, &pf->state))
		return -EBUSY;

	if (test_bit(__I40E_FD_FLUSH_REQUESTED, &pf->state))
		return -EBUSY;

	ret = i40e_update_ethtool_fdir_entry(vsi, NULL, fsp->location, cmd);

	i40e_fdir_check_and_reenable(pf);
	return ret;
}

/**
 * i40e_add_fdir_ethtool - Add/Remove Flow Director filters
 * @vsi: pointer to the targeted VSI
 * @cmd: command to get or set RX flow classification rules
 *
 * Add Flow Director filters for a specific flow spec based on their
 * protocol.  Returns 0 if the filters were successfully added.
 **/
static int i40e_add_fdir_ethtool(struct i40e_vsi *vsi,
				 struct ethtool_rxnfc *cmd)
{
	struct ethtool_rx_flow_spec *fsp;
	struct i40e_fdir_filter *input;
	struct i40e_pf *pf;
	int ret = -EINVAL;
	u16 vf_id;

	if (!vsi)
		return -EINVAL;

	pf = vsi->back;

	if (!(pf->flags & I40E_FLAG_FD_SB_ENABLED))
		return -EOPNOTSUPP;

	if (pf->auto_disable_flags & I40E_FLAG_FD_SB_ENABLED)
		return -ENOSPC;

	if (test_bit(__I40E_RESET_RECOVERY_PENDING, &pf->state) ||
	    test_bit(__I40E_RESET_INTR_RECEIVED, &pf->state))
		return -EBUSY;

	if (test_bit(__I40E_FD_FLUSH_REQUESTED, &pf->state))
		return -EBUSY;

	fsp = (struct ethtool_rx_flow_spec *)&cmd->fs;

	if (fsp->location >= (pf->hw.func_caps.fd_filters_best_effort +
			      pf->hw.func_caps.fd_filters_guaranteed)) {
		return -EINVAL;
	}

	if ((fsp->ring_cookie != RX_CLS_FLOW_DISC) &&
	    (fsp->ring_cookie >= vsi->num_queue_pairs))
		return -EINVAL;

	input = kzalloc(sizeof(*input), GFP_KERNEL);

	if (!input)
		return -ENOMEM;

	input->fd_id = fsp->location;

	if (fsp->ring_cookie == RX_CLS_FLOW_DISC)
		input->dest_ctl = I40E_FILTER_PROGRAM_DESC_DEST_DROP_PACKET;
	else
		input->dest_ctl =
			     I40E_FILTER_PROGRAM_DESC_DEST_DIRECT_PACKET_QINDEX;

	input->q_index = fsp->ring_cookie;
	input->flex_off = 0;
	input->pctype = 0;
	input->dest_vsi = vsi->id;
	input->fd_status = I40E_FILTER_PROGRAM_DESC_FD_STATUS_FD_ID;
	input->cnt_index  = pf->fd_sb_cnt_idx;
	input->flow_type = fsp->flow_type;
	input->ip4_proto = fsp->h_u.usr_ip4_spec.proto;

	/* Reverse the src and dest notion, since the HW expects them to be from
	 * Tx perspective where as the input from user is from Rx filter view.
	 */
	input->dst_port = fsp->h_u.tcp_ip4_spec.psrc;
	input->src_port = fsp->h_u.tcp_ip4_spec.pdst;
	input->dst_ip[0] = fsp->h_u.tcp_ip4_spec.ip4src;
	input->src_ip[0] = fsp->h_u.tcp_ip4_spec.ip4dst;

	if (ntohl(fsp->m_ext.data[1])) {
		if (ntohl(fsp->h_ext.data[1]) >= pf->num_alloc_vfs) {
			netif_info(pf, drv, vsi->netdev, "Invalid VF id\n");
			goto free_input;
		}
		vf_id = ntohl(fsp->h_ext.data[1]);
		/* Find vsi id from vf id and override dest vsi */
		input->dest_vsi = pf->vf[vf_id].lan_vsi_id;
		if (input->q_index >= pf->vf[vf_id].num_queue_pairs) {
			netif_info(pf, drv, vsi->netdev, "Invalid queue id\n");
			goto free_input;
		}
	}

	ret = i40e_add_del_fdir(vsi, input, true);
free_input:
	if (ret)
		kfree(input);
	else
		i40e_update_ethtool_fdir_entry(vsi, input, fsp->location, NULL);

	return ret;
}

/**
 * i40e_set_rxnfc - command to set RX flow classification rules
 * @netdev: network interface device structure
 * @cmd: ethtool rxnfc command
 *
 * Returns Success if the command is supported.
 **/
static int i40e_set_rxnfc(struct net_device *netdev, struct ethtool_rxnfc *cmd)
{
	struct i40e_netdev_priv *np = netdev_priv(netdev);
	struct i40e_vsi *vsi = np->vsi;
	struct i40e_pf *pf = vsi->back;
	int ret = -EOPNOTSUPP;

	switch (cmd->cmd) {
	case ETHTOOL_SRXFH:
		ret = i40e_set_rss_hash_opt(pf, cmd);
		break;
	case ETHTOOL_SRXCLSRLINS:
		ret = i40e_add_fdir_ethtool(vsi, cmd);
		break;
	case ETHTOOL_SRXCLSRLDEL:
		ret = i40e_del_fdir_entry(vsi, cmd);
		break;
	default:
		break;
	}

	return ret;
}

/**
 * i40e_max_channels - get Max number of combined channels supported
 * @vsi: vsi pointer
 **/
static unsigned int i40e_max_channels(struct i40e_vsi *vsi)
{
	/* TODO: This code assumes DCB and FD is disabled for now. */
	return vsi->alloc_queue_pairs;
}

/**
 * i40e_get_channels - Get the current channels enabled and max supported etc.
 * @netdev: network interface device structure
 * @ch: ethtool channels structure
 *
 * We don't support separate tx and rx queues as channels. The other count
 * represents how many queues are being used for control. max_combined counts
 * how many queue pairs we can support. They may not be mapped 1 to 1 with
 * q_vectors since we support a lot more queue pairs than q_vectors.
 **/
static void i40e_get_channels(struct net_device *dev,
			       struct ethtool_channels *ch)
{
	struct i40e_netdev_priv *np = netdev_priv(dev);
	struct i40e_vsi *vsi = np->vsi;
	struct i40e_pf *pf = vsi->back;

	/* report maximum channels */
	ch->max_combined = i40e_max_channels(vsi);

	/* report info for other vector */
	ch->other_count = (pf->flags & I40E_FLAG_FD_SB_ENABLED) ? 1 : 0;
	ch->max_other = ch->other_count;

	/* Note: This code assumes DCB is disabled for now. */
	ch->combined_count = vsi->num_queue_pairs;
}

/**
 * i40e_set_channels - Set the new channels count.
 * @netdev: network interface device structure
 * @ch: ethtool channels structure
 *
 * The new channels count may not be the same as requested by the user
 * since it gets rounded down to a power of 2 value.
 **/
static int i40e_set_channels(struct net_device *dev,
			      struct ethtool_channels *ch)
{
	struct i40e_netdev_priv *np = netdev_priv(dev);
	unsigned int count = ch->combined_count;
	struct i40e_vsi *vsi = np->vsi;
	struct i40e_pf *pf = vsi->back;
	int new_count;

	/* We do not support setting channels for any other VSI at present */
	if (vsi->type != I40E_VSI_MAIN)
		return -EINVAL;

	/* verify they are not requesting separate vectors */
	if (!count || ch->rx_count || ch->tx_count)
		return -EINVAL;

	/* verify other_count has not changed */
	if (ch->other_count != ((pf->flags & I40E_FLAG_FD_SB_ENABLED) ? 1 : 0))
		return -EINVAL;

	/* verify the number of channels does not exceed hardware limits */
	if (count > i40e_max_channels(vsi))
		return -EINVAL;

	/* update feature limits from largest to smallest supported values */
	/* TODO: Flow director limit, DCB etc */

	/* use rss_reconfig to rebuild with new queue count and update traffic
	 * class queue mapping
	 */
	new_count = i40e_reconfig_rss_queues(pf, count);
	if (new_count > 0)
		return 0;
	else
		return -EINVAL;
}

#define I40E_HLUT_ARRAY_SIZE ((I40E_PFQF_HLUT_MAX_INDEX + 1) * 4)
/**
 * i40e_get_rxfh_key_size - get the RSS hash key size
 * @netdev: network interface device structure
 *
 * Returns the table size.
 **/
static u32 i40e_get_rxfh_key_size(struct net_device *netdev)
{
	return I40E_HKEY_ARRAY_SIZE;
}

/**
 * i40e_get_rxfh_indir_size - get the rx flow hash indirection table size
 * @netdev: network interface device structure
 *
 * Returns the table size.
 **/
static u32 i40e_get_rxfh_indir_size(struct net_device *netdev)
{
	return I40E_HLUT_ARRAY_SIZE;
}

static int i40e_get_rxfh(struct net_device *netdev, u32 *indir, u8 *key,
			 u8 *hfunc)
{
	struct i40e_netdev_priv *np = netdev_priv(netdev);
	struct i40e_vsi *vsi = np->vsi;
	struct i40e_pf *pf = vsi->back;
	struct i40e_hw *hw = &pf->hw;
	u32 reg_val;
	int i, j;

	if (hfunc)
		*hfunc = ETH_RSS_HASH_TOP;

	if (!indir)
		return 0;

	for (i = 0, j = 0; i <= I40E_PFQF_HLUT_MAX_INDEX; i++) {
		reg_val = rd32(hw, I40E_PFQF_HLUT(i));
		indir[j++] = reg_val & 0xff;
		indir[j++] = (reg_val >> 8) & 0xff;
		indir[j++] = (reg_val >> 16) & 0xff;
		indir[j++] = (reg_val >> 24) & 0xff;
	}

	if (key) {
		for (i = 0, j = 0; i <= I40E_PFQF_HKEY_MAX_INDEX; i++) {
			reg_val = rd32(hw, I40E_PFQF_HKEY(i));
			key[j++] = (u8)(reg_val & 0xff);
			key[j++] = (u8)((reg_val >> 8) & 0xff);
			key[j++] = (u8)((reg_val >> 16) & 0xff);
			key[j++] = (u8)((reg_val >> 24) & 0xff);
		}
	}
	return 0;
}

/**
 * i40e_set_rxfh - set the rx flow hash indirection table
 * @netdev: network interface device structure
 * @indir: indirection table
 * @key: hash key
 *
 * Returns -EINVAL if the table specifies an inavlid queue id, otherwise
 * returns 0 after programming the table.
 **/
static int i40e_set_rxfh(struct net_device *netdev, const u32 *indir,
			 const u8 *key, const u8 hfunc)
{
	struct i40e_netdev_priv *np = netdev_priv(netdev);
	struct i40e_vsi *vsi = np->vsi;
	struct i40e_pf *pf = vsi->back;
	struct i40e_hw *hw = &pf->hw;
	u32 reg_val;
	int i, j;

	if (hfunc != ETH_RSS_HASH_NO_CHANGE && hfunc != ETH_RSS_HASH_TOP)
		return -EOPNOTSUPP;

	if (!indir)
		return 0;

	for (i = 0, j = 0; i <= I40E_PFQF_HLUT_MAX_INDEX; i++) {
		reg_val = indir[j++];
		reg_val |= indir[j++] << 8;
		reg_val |= indir[j++] << 16;
		reg_val |= indir[j++] << 24;
		wr32(hw, I40E_PFQF_HLUT(i), reg_val);
	}

	if (key) {
		for (i = 0, j = 0; i <= I40E_PFQF_HKEY_MAX_INDEX; i++) {
			reg_val = key[j++];
			reg_val |= key[j++] << 8;
			reg_val |= key[j++] << 16;
			reg_val |= key[j++] << 24;
			wr32(hw, I40E_PFQF_HKEY(i), reg_val);
		}
	}
	return 0;
}

/**
 * i40e_get_priv_flags - report device private flags
 * @dev: network interface device structure
 *
 * The get string set count and the string set should be matched for each
 * flag returned.  Add new strings for each flag to the i40e_priv_flags_strings
 * array.
 *
 * Returns a u32 bitmap of flags.
 **/
static u32 i40e_get_priv_flags(struct net_device *dev)
{
	struct i40e_netdev_priv *np = netdev_priv(dev);
	struct i40e_vsi *vsi = np->vsi;
	struct i40e_pf *pf = vsi->back;
	u32 ret_flags = 0;

	ret_flags |= pf->hw.func_caps.npar_enable ?
		I40E_PRIV_FLAGS_NPAR_FLAG : 0;

	return ret_flags;
}

static const struct ethtool_ops i40e_ethtool_ops = {
	.get_settings		= i40e_get_settings,
	.set_settings		= i40e_set_settings,
	.get_drvinfo		= i40e_get_drvinfo,
	.get_regs_len		= i40e_get_regs_len,
	.get_regs		= i40e_get_regs,
	.nway_reset		= i40e_nway_reset,
	.get_link		= ethtool_op_get_link,
	.get_wol		= i40e_get_wol,
	.set_wol		= i40e_set_wol,
	.set_eeprom		= i40e_set_eeprom,
	.get_eeprom_len		= i40e_get_eeprom_len,
	.get_eeprom		= i40e_get_eeprom,
	.get_ringparam		= i40e_get_ringparam,
	.set_ringparam		= i40e_set_ringparam,
	.get_pauseparam		= i40e_get_pauseparam,
	.set_pauseparam		= i40e_set_pauseparam,
	.get_msglevel		= i40e_get_msglevel,
	.set_msglevel		= i40e_set_msglevel,
	.get_rxnfc		= i40e_get_rxnfc,
	.set_rxnfc		= i40e_set_rxnfc,
	.self_test		= i40e_diag_test,
	.get_strings		= i40e_get_strings,
	.set_phys_id		= i40e_set_phys_id,
	.get_sset_count		= i40e_get_sset_count,
	.get_ethtool_stats	= i40e_get_ethtool_stats,
	.get_coalesce		= i40e_get_coalesce,
	.set_coalesce		= i40e_set_coalesce,
	.get_rxfh_key_size	= i40e_get_rxfh_key_size,
	.get_rxfh_indir_size	= i40e_get_rxfh_indir_size,
	.get_rxfh		= i40e_get_rxfh,
	.set_rxfh		= i40e_set_rxfh,
	.get_channels		= i40e_get_channels,
	.set_channels		= i40e_set_channels,
	.get_ts_info		= i40e_get_ts_info,
	.get_priv_flags		= i40e_get_priv_flags,
};

void i40e_set_ethtool_ops(struct net_device *netdev)
{
	netdev->ethtool_ops = &i40e_ethtool_ops;
}
