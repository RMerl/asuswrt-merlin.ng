/*
   <:copyright-BRCM:2017:DUAL/GPL:standard
   
      Copyright (c) 2017 Broadcom 
      All Rights Reserved
   
   Unless you and Broadcom execute a separate written software license
   agreement governing use of this software, this software is licensed
   to you under the terms of the GNU General Public License version 2
   (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
   with the following added to such license:
   
      As a special exception, the copyright holders of this software give
      you permission to link this software with independent modules, and
      to copy and distribute the resulting executable under terms of your
      choice, provided that you also meet, for each linked independent
      module, the terms and conditions of the license of that module.
      An independent module is a module which is not derived from this
      software.  The special exception does not apply to any modifications
      of the software.
   
   Not withstanding the above, under no circumstances may you combine
   this software in any way with any other Broadcom software provided
   under a license other than the GPL, without Broadcom's express prior
   written consent.
   
   :>
 */

/*
 */

#include <linux/netdevice.h>
#include <linux/ethtool.h>
#include "bcmnet.h"
#include "bcmenet_ethtool.h"
#include "enet.h"
#include "port.h"
#ifdef CONFIG_BCM_PTP_1588
extern int ptp_1588_get_ts_info(struct net_device *net, struct ethtool_ts_info *info);
#endif

static void enet_get_ethtool_stats(struct net_device *dev, struct ethtool_stats *stats, u64 *data)
{
    const struct rtnl_link_stats64 *ethStats;
    struct rtnl_link_stats64 temp;
    enetx_port_t *port = ((enetx_netdev *)netdev_priv(dev))->port;
    phy_dev_t *phy_dev;
    u64 speed = 0;

    if (!port || port->port_class != PORT_CLASS_PORT)
        return;

    ethStats = dev_get_stats(dev, &temp);
    
    data[ET_TX_BYTES] =     ethStats->tx_bytes;
    data[ET_TX_PACKETS] =   ethStats->tx_packets;
    data[ET_TX_ERRORS] =    ethStats->tx_errors;
    data[ET_RX_BYTES] =     ethStats->rx_bytes;
    data[ET_RX_PACKETS] =   ethStats->rx_packets;
    data[ET_RX_ERRORS] =    ethStats->rx_errors;

    /* Note: capacity is in bytes per second */
    phy_dev = port->p.phy;
    if (phy_dev && phy_dev->link)
    {
        speed = (u64) 1000000 * phy_speed_2_mbps(phy_dev->speed);
    }
    data[ET_TX_CAPACITY] = speed / 8;
}

#define LM_SET(lm,b)    test_and_set_bit(b, lm)

static void phy_caps_to_link_mode(uint32_t caps, unsigned long* lm)
{
    if (caps&PHY_CAP_10_HALF)       LM_SET(lm,ETHTOOL_LINK_MODE_10baseT_Half_BIT);
    if (caps&PHY_CAP_10_FULL)       LM_SET(lm,ETHTOOL_LINK_MODE_10baseT_Full_BIT);
    if (caps&PHY_CAP_100_HALF)      LM_SET(lm,ETHTOOL_LINK_MODE_100baseT_Half_BIT);
    if (caps&PHY_CAP_100_FULL)      LM_SET(lm,ETHTOOL_LINK_MODE_100baseT_Full_BIT);
    if (caps&PHY_CAP_1000_HALF)     LM_SET(lm,ETHTOOL_LINK_MODE_1000baseT_Half_BIT);
    if (caps&PHY_CAP_1000_FULL)     LM_SET(lm,ETHTOOL_LINK_MODE_1000baseT_Full_BIT);
    if (caps&PHY_CAP_2500)          LM_SET(lm,ETHTOOL_LINK_MODE_2500baseT_Full_BIT);
    if (caps&PHY_CAP_5000)          LM_SET(lm,ETHTOOL_LINK_MODE_5000baseT_Full_BIT);
    if (caps&PHY_CAP_10000)         LM_SET(lm,ETHTOOL_LINK_MODE_10000baseT_Full_BIT);
    if (caps&PHY_CAP_PAUSE)         LM_SET(lm,ETHTOOL_LINK_MODE_Pause_BIT);
    if (caps&PHY_CAP_PAUSE_ASYM)    LM_SET(lm,ETHTOOL_LINK_MODE_Asym_Pause_BIT);
    if (caps&PHY_CAP_AUTONEG)       LM_SET(lm,ETHTOOL_LINK_MODE_Autoneg_BIT);
}

static int enet_ethtool_get_ksettings(struct net_device *dev, struct ethtool_link_ksettings *ecmd)
{
    enetx_port_t *port = ((enetx_netdev *)netdev_priv(dev))->port;
    phy_dev_t *phy_dev;
    uint32_t caps = 0, lp_caps = 0, supported = 0;

    if (!port || port->port_class != PORT_CLASS_PORT)
        return -EOPNOTSUPP;

    phy_dev = port->p.phy;
    if (!phy_dev)
        return -EOPNOTSUPP;

    phy_dev = cascade_phy_get_last_active(phy_dev);

    if (cascade_phy_dev_caps_get(phy_dev, CAPS_TYPE_ADVERTISE, &caps) ||
        cascade_phy_dev_caps_get(phy_dev, CAPS_TYPE_LP_ADVERTISED,  &lp_caps) ||
        cascade_phy_dev_caps_get(phy_dev, CAPS_TYPE_SUPPORTED, &supported))
        return -1;

    phy_caps_to_link_mode(caps, ecmd->link_modes.advertising);
    phy_caps_to_link_mode(lp_caps, ecmd->link_modes.lp_advertising);

    // TODO: need to get supported link mode from phy
    phy_caps_to_link_mode(supported |PHY_CAP_AUTONEG|PHY_CAP_PAUSE|PHY_CAP_PAUSE_ASYM, ecmd->link_modes.supported);
    LM_SET(ecmd->link_modes.supported, ETHTOOL_LINK_MODE_TP_BIT);

    switch(phy_dev->speed)
    {
    case PHY_SPEED_10000:
    case PHY_SPEED_5000:
    case PHY_SPEED_2500:
    case PHY_SPEED_1000:
    case PHY_SPEED_100:
    case PHY_SPEED_10:
        ecmd->base.speed = phy_speed_2_mbps(phy_dev->speed); break;
    case 0:
        break;
    default:
        enet_err("Unknown ethernet speed (%d)\n", phy_dev->speed);
        return -1;
    }

    ecmd->base.duplex = (phy_dev->duplex == PHY_DUPLEX_FULL)? DUPLEX_FULL: DUPLEX_HALF;
    ecmd->base.autoneg = (caps & PHY_CAP_AUTONEG) ? AUTONEG_ENABLE : AUTONEG_DISABLE;
    ecmd->base.phy_address = phy_dev->addr;

  return 0;
}

static int enet_ethtool_set_ksettings(struct net_device *dev, const struct ethtool_link_ksettings *ecmd)
{
    enetx_port_t *port = ((enetx_netdev *)netdev_priv(dev))->port;
    phy_dev_t *phy_dev;
    uint32_t caps;

    if (!port || port->port_class != PORT_CLASS_PORT)
        return -EOPNOTSUPP;

    phy_dev = port->p.phy;
    if (!phy_dev)
    {
        enet_err("%s has no PHY connect\n", dev->name);
        return -EOPNOTSUPP;
    }

    phy_dev = cascade_phy_get_last_active(phy_dev);

    if (cascade_phy_dev_caps_get(phy_dev, CAPS_TYPE_ADVERTISE, &caps))
        return -1;

    caps &= PHY_CAP_PAUSE | PHY_CAP_PAUSE_ASYM | PHY_CAP_REPEATER;  // preserve these caps

    if (ecmd->base.autoneg == AUTONEG_ENABLE)    caps |= PHY_CAP_AUTONEG;
    switch(ecmd->base.speed)
    {
        case SPEED_10000:   caps |= PHY_CAP_10000; break;
        case SPEED_5000:    caps |= PHY_CAP_5000;  break;
        case SPEED_2500:    caps |= PHY_CAP_2500;  break;
        case SPEED_1000:    caps |= PHY_CAP_1000_HALF | ((ecmd->base.duplex == DUPLEX_FULL) ? PHY_CAP_1000_FULL : 0); break;
        case SPEED_100:     caps |= PHY_CAP_100_HALF | ((ecmd->base.duplex == DUPLEX_FULL) ? PHY_CAP_100_FULL : 0); break;
        case SPEED_10:      caps |= PHY_CAP_10_HALF | ((ecmd->base.duplex == DUPLEX_FULL) ? PHY_CAP_10_FULL : 0); break;
        default:
            enet_err("Unknown Ethernet Speed Requested: (%d)Mbps\n", ecmd->base.speed);
            return -1;
    }

    return cascade_phy_dev_caps_set(phy_dev, caps);
}

#define BCM_ENET_DRV_VERSION "7.0"  // enet impl7

const char bcmenet_fullname[] = "Broadcom Ethernet Interface";
const char bcmenet_version[] = BCM_ENET_DRV_VERSION;

static void enet_ethtool_get_drvinfo(struct net_device *dev, struct ethtool_drvinfo *info)
{
    enetx_port_t *port = ((enetx_netdev *)netdev_priv(dev))->port;

    if (!port || port->port_class != PORT_CLASS_PORT)
        return;

    strcpy(info->driver, bcmenet_fullname);
    strcpy(info->version, bcmenet_version);
    strcpy(info->fw_version, "N/A");
    info->n_stats = ET_MAX;
    info->n_priv_flags = ET_PF_MAX;
}

static void enet_ethtool_get_pauseparam(struct net_device *dev, struct ethtool_pauseparam *info)
{
    enetx_port_t *port = ((enetx_netdev *)netdev_priv(dev))->port;
    int rx_enable, tx_enable;

    if (!port || port->port_class != PORT_CLASS_PORT)
        return;

    if (port_pause_get(port, &rx_enable, &tx_enable))
        return;
    info->rx_pause = rx_enable;
    info->tx_pause = tx_enable;
    info->autoneg  = 1;         // TODO: hardcoded to true, need to get from hw
}

static int enet_ethtool_set_pauseparam(struct net_device *dev, struct ethtool_pauseparam *info)
{
    enetx_port_t *port = ((enetx_netdev *)netdev_priv(dev))->port;

    if (!port || port->port_class != PORT_CLASS_PORT)
        return -1;

    return port_pause_set(port, info->rx_pause, info->tx_pause);
}

#if defined(CONFIG_BCM_ENERGY_EFFICIENT_ETHERNET)
static int enet_ethtool_get_eee(struct net_device *dev, struct ethtool_eee *info)
{
    enetx_port_t *port = ((enetx_netdev *)netdev_priv(dev))->port;
    phy_dev_t *phy_dev;
    int enabled;

    if (!port || port->port_class != PORT_CLASS_PORT)
        return -1;

    phy_dev = port->p.phy;
    if (!phy_dev)
    {
        enet_err("%s has no PHY connect\n", dev->name);
        return -1;
    }
    phy_dev = cascade_phy_get_last_active(phy_dev);

    phy_dev_eee_get(phy_dev, &enabled);
    info->eee_enabled = enabled;
    phy_dev_eee_resolution_get(phy_dev, &enabled);
    info->eee_active = enabled;

    info->supported = SUPPORTED_TP; // need to be non-zero to display above values;
    return 0;
}

static int enet_ethtool_set_eee(struct net_device *dev, struct ethtool_eee *info)
{
   enetx_port_t *port = ((enetx_netdev *)netdev_priv(dev))->port;
    phy_dev_t *phy_dev;

    if (!port || port->port_class != PORT_CLASS_PORT)
        return -1;

    phy_dev = port->p.phy;
    if (!phy_dev)
    {
        enet_err("%s has no PHY connect\n", dev->name);
        return -1;
    }

    return cascade_phy_dev_eee_set(phy_dev, info->eee_enabled ? 1: 0);
}
#endif

#if defined(CONFIG_BCM_I2C_CHIP) && defined(DSL_DEVICES)
#include "bcmsfp_i2c.h"
#include "crossbar_dev.h"

static int dev_get_i2c_bus(struct net_device *dev)
{
    enetx_port_t *port = ((enetx_netdev *)netdev_priv(dev))->port;
    phy_dev_t *phy;

    if (!port || port->port_class != PORT_CLASS_PORT)
        return -1;

    phy = get_active_phy(port->p.phy);
    while ( phy && phy->cascade_next)
        phy = phy->cascade_next;

    if (!phy || phy->phy_drv->phy_type != PHY_TYPE_I2C_PHY)
        return -1;

    return phy->addr;
}

static int enet_ethtool_get_module_info(struct net_device *dev, struct ethtool_modinfo *modinfo)
{
    int bus;
    uint8_t a0, a2;

    bus = dev_get_i2c_bus(dev);
    if (bus < 0)
        return -ENODEV;

    if (bcmsfp_get_page_info((uint8_t)bus, &a0, &a2) != 0)
        return -ENODEV;

    if (a2)
    {
        modinfo->type = ETH_MODULE_SFF_8472;
        modinfo->eeprom_len = ETH_MODULE_SFF_8472_LEN;
    } 
    else
    {
        modinfo->type = ETH_MODULE_SFF_8079;
        modinfo->eeprom_len = ETH_MODULE_SFF_8079_LEN;
    } 

    return 0;
}

static int enet_ethtool_get_module_eeprom(struct net_device *dev, struct ethtool_eeprom *ee, u8 *data)
{
    int bus;
    uint8_t buf[ETH_MODULE_SFF_8472_LEN];

    bus = dev_get_i2c_bus(dev);
    if (bus <0)
        return -ENODEV;

    if (bcmsfp_read_page((uint8_t)bus, SFP_CLIENT_EEPROM, buf) != 0)
        return -ENODEV;

    if (ee->offset + ee->len > ETH_MODULE_SFF_8079_LEN)
        if (bcmsfp_read_page((uint8_t)bus, SFP_CLIENT_DIAG, &buf[ETH_MODULE_SFF_8079_LEN]) != 0)
            return -ENXIO;

    // some SFP module don't specify phy dev ID, set to SFP so ethtool will parse correctly
    if (buf[0] == 0) buf[0] = 3;

    memcpy(data, &buf[ee->offset], ee->len);
    return 0;
}
#endif

const struct ethtool_ops enet_ethtool_ops =
{
    .get_drvinfo  =         enet_ethtool_get_drvinfo,
    .get_link_ksettings =   enet_ethtool_get_ksettings,
    .set_link_ksettings =   enet_ethtool_set_ksettings,
    .get_ethtool_stats =    enet_get_ethtool_stats,
    .get_pauseparam =       enet_ethtool_get_pauseparam,
    .set_pauseparam =       enet_ethtool_set_pauseparam,
#if defined(CONFIG_BCM_ENERGY_EFFICIENT_ETHERNET)
    .get_eee =              enet_ethtool_get_eee,
    .set_eee =              enet_ethtool_set_eee,
#endif
#ifdef CONFIG_BCM_PTP_1588
    .get_ts_info = ptp_1588_get_ts_info,
#endif
#if defined(CONFIG_BCM_I2C_CHIP) && defined(DSL_DEVICES)
    .get_module_info =      enet_ethtool_get_module_info,
    .get_module_eeprom =    enet_ethtool_get_module_eeprom,
#endif
};


