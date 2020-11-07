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

#include <linux/netdevice.h>
#include "bcmenet_common.h"
#include "bcmenet_dma.h"
#include <linux/ethtool.h>
#include "bcmnet.h"
#include "bcmenet_ethtool.h"

static int enet_ethtool_get_settings(struct net_device *dev, struct ethtool_cmd *ecmd)
{

  BcmEnet_devctrl *pDevCtrl = netdev_priv(dev);

  switch(pDevCtrl->MibInfo.ulIfSpeed) {
    case SPEED_2500MBIT:
      ecmd->speed = SPEED_2500;
      break;
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
       WARN_ONCE(1, "Unknown ethernet speed (%lld)\n",pDevCtrl->MibInfo.ulIfSpeed);
       return -1;
        
  }
    
  if (pDevCtrl->MibInfo.ulIfDuplex) {
    ecmd->duplex = DUPLEX_FULL;
  }else {
    ecmd->duplex = DUPLEX_HALF;
  }

  return 0;
    return -1;
}

static void enet_get_ethtool_stats(struct net_device *dev, struct ethtool_stats *stats, u64 *data)
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
            WARN_ONCE(1, "[%s.%d]: Unrecognized speed for %p (%5s): %lld\n", __func__, __LINE__, dev, dev->name, pDevCtrl->MibInfo.ulIfSpeed);
            break;
    }
    data[ET_TX_PACKETS] =   ethStats->tx_packets;
    data[ET_TX_ERRORS] =    ethStats->tx_errors;
    data[ET_RX_BYTES] =     ethStats->rx_bytes;
    data[ET_RX_PACKETS] =   ethStats->rx_packets;
    data[ET_RX_ERRORS] =    ethStats->rx_errors;
}

const struct ethtool_ops enet_ethtool_ops = {
    .get_settings =         enet_ethtool_get_settings,
    .get_ethtool_stats =    enet_get_ethtool_stats,
};



