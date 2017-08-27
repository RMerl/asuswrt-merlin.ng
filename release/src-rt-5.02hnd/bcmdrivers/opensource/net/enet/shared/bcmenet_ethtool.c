/*
<:copyright-BRCM:2013:DUAL/GPL:standard

   Copyright (c) 2013 Broadcom 
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

#include <linux/netdevice.h>
#include "bcmenet_common.h"
#if !defined(CONFIG_BCM_RDPA) && !defined(CONFIG_BCM_RDPA_MODULE)
#include "bcmenet_dma.h"
#else
#include "bcmenet_runner.h"
#endif
#include <linux/ethtool.h>
#include "bcmnet.h"


#if defined(SUPPORT_ETHTOOL)

enum {
    ET_TX_BYTES = 0,
    ET_TX_PACKETS,
    ET_TX_ERRORS,
    ET_TX_CAPACITY,
    ET_RX_BYTES,
    ET_RX_PACKETS, 
    ET_RX_ERRORS,
    ET_MAX
} Bcm63xxEnetStats;

static char ethtool_stats_strings[][ETH_GSTRING_LEN] = {
    [ET_TX_BYTES] =     "TxOctetsOK          ",
    [ET_TX_PACKETS] =   "TxPacketsOK         ",
    [ET_TX_ERRORS] =    "TxErrors            ",
    [ET_TX_CAPACITY] =  "TxCapacity          ",
    [ET_RX_BYTES] =     "RxOctetsOK          ",
    [ET_RX_PACKETS] =   "RxPacketsOK         ",
    [ET_RX_ERRORS] =    "RxErrors            "
};

#define COMPILE_TIME_CHECK(condition) ((void)sizeof(char[1 - 2*(!(condition))]))

static int bcm63xx_ethtool_get_settings(struct net_device *dev, struct ethtool_cmd *ecmd);
static void bcm63xx_get_ethtool_stats(struct net_device *dev, struct ethtool_stats *stats, u64 *data);
static int bcm63xx_get_ethtool_sset_count(struct net_device *dev, int sset);
static void bcm63xx_get_ethtool_strings(struct net_device *netdev, u32 stringset, u8 *data);

const struct ethtool_ops bcm63xx_enet_ethtool_ops = {
    .get_settings =         bcm63xx_ethtool_get_settings,
    .get_ethtool_stats =    bcm63xx_get_ethtool_stats,
    .get_sset_count =       bcm63xx_get_ethtool_sset_count,
    .get_strings =          bcm63xx_get_ethtool_strings,
    .get_link     =         ethtool_op_get_link,
};

void bcm63xx_ethtool_dummy(void)
{
    // this function is never invoked.  It is being used as a placeholder for the
    // compile time check
    COMPILE_TIME_CHECK(ARRAY_SIZE(ethtool_stats_strings) == ET_MAX);  // these two should be kept in sync
}


static int bcm63xx_ethtool_get_settings(struct net_device *dev, struct ethtool_cmd *ecmd)
{
  BcmEnet_devctrl *pDevCtrl = netdev_priv(dev);

  switch(pDevCtrl->MibInfo.ulIfSpeed) {
    case SPEED_1000MBIT:
       ecmd->speed = SPEED_1000;
       break;
    case SPEED_100MBIT:
       ecmd->speed = SPEED_100;
       break;
    case SPEED_10MBIT:
       ecmd->speed = SPEED_10;
       break;
    case 0:
       // it is possible the enet is not fully up yet.
       return -1;
    default:
       // unsupported speed
       WARN_ONCE(1, "Unknown ethernet speed (%d)\n",pDevCtrl->MibInfo.ulIfSpeed);
       return -1;
        
  }
    
  if (pDevCtrl->MibInfo.ulIfDuplex) {
    ecmd->duplex = DUPLEX_FULL;
  }else {
    ecmd->duplex = DUPLEX_HALF;
  }

  return 0;
}


static int bcm63xx_get_ethtool_sset_count(struct net_device *dev, int sset)
{
    switch (sset) {
    case ETH_SS_STATS:
        return ARRAY_SIZE(ethtool_stats_strings);
    default:
        return -EOPNOTSUPP;
    }
}

static void bcm63xx_get_ethtool_strings(struct net_device *netdev, u32 stringset, u8 *data)
{
	switch (stringset) {
	case ETH_SS_STATS:
		memcpy(data, *ethtool_stats_strings, sizeof(ethtool_stats_strings));
		break;
	}
}

static void bcm63xx_get_ethtool_stats(struct net_device *dev, struct ethtool_stats *stats, u64 *data)
{
    const struct rtnl_link_stats64 *ethStats;
    struct rtnl_link_stats64 temp;

    BcmEnet_devctrl *pDevCtrl = netdev_priv(dev);
    ethStats = dev_get_stats(dev, &temp);
    
    data[ET_TX_BYTES] =     ethStats->tx_bytes;
    /* Note: capacity is in bytes per second */
    switch(pDevCtrl->MibInfo.ulIfSpeed) {
        case SPEED_1000MBIT: data[ET_TX_CAPACITY] = 1000000000L/8;    break;
        case SPEED_100MBIT:  data[ET_TX_CAPACITY] = 100000000L/8;     break;
        case SPEED_10MBIT:   data[ET_TX_CAPACITY] = 10000000L/8;      break;
        case 0:              data[ET_TX_CAPACITY] = 0;       break;
        default:             
            data[ET_TX_CAPACITY] = pDevCtrl->MibInfo.ulIfSpeed/8; 
            WARN_ONCE(1, "[%s.%d]: Unrecognized speed for %p (%5s): %d\n", __func__, __LINE__, dev, dev->name, pDevCtrl->MibInfo.ulIfSpeed);
            break;
    }
    data[ET_TX_PACKETS] =   ethStats->tx_packets;
    data[ET_TX_ERRORS] =    ethStats->tx_errors;
    data[ET_RX_BYTES] =     ethStats->rx_bytes;
    data[ET_RX_PACKETS] =   ethStats->rx_packets;
    data[ET_RX_ERRORS] =    ethStats->rx_errors;
}

#endif // ETHTOOL_SUPPORT

