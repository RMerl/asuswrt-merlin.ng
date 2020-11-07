/***********************************************************************
 *
 *  Copyright (c) 2019 Broadcom
 *  All Rights Reserved
 *
<:label-BRCM:2018:proprietary:standard

 This program is the proprietary software of Broadcom and/or its
 licensors, and may only be used, duplicated, modified or distributed pursuant
 to the terms and conditions of a separate, written license agreement executed
 between you and Broadcom (an "Authorized License").  Except as set forth in
 an Authorized License, Broadcom grants no license (express or implied), right
 to use, or waiver of any kind with respect to the Software, and Broadcom
 expressly reserves all rights in and to the Software and all intellectual
 property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
 NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
 BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.

 Except as expressly set forth in the Authorized License,

 1. This program, including its structure, sequence and organization,
    constitutes the valuable trade secrets of Broadcom, and you shall use
    all reasonable efforts to protect the confidentiality thereof, and to
    use this information only in connection with your use of Broadcom
    integrated circuit products.

 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
    RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
    ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
    FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
    COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
    TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
    PERFORMANCE OF THE SOFTWARE.

 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
    ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
    INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
    WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
    IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
    OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
    SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
    SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
    LIMITED REMEDY.
:>
 *
 ************************************************************************/

/*****************************************************************************
*    Description:
*
*      This file implements the BCM xtmctl utility wrapper functions
*      with uni port index (a.k.a oam index) as input.
*      The oam index represents the physical Ethernet UNI port numbering
*      order on the CPE device face plate, the numbering is used by the
*      management protocols such as OMCI, EPON OAM, etc. A dynamically
*      created virtual interface or an internal interface does not have
*      OAM index. Typically, the OAM index value is 0-based.
*
*****************************************************************************/

#ifndef _BCMXTM_UNI_IF_WRAP_H_
#define _BCMXTM_UNI_IF_WRAP_H_

/* ---- Include Files ----------------------------------------------------- */

#include "devctl_xtm.h"

/* ---- Constants and Types ----------------------------------------------- */


/* ---- Macro API definitions --------------------------------------------- */

/* #define CC_XTMUNIIF_DEBUG */

#if defined(CC_XTMUNIIF_DEBUG)
#define xtmSwTrace(fmt, arg...) \
  printf("[XTMUNIIF_WRAP] %s(): " fmt "\n", __FUNCTION__, ##arg)
#else
#define xtmSwTrace(fmt, arg...)
#endif

/* ---- Variable Externs -------------------------------------------------- */


/* ---- Inline functions -------------------------------------------------- */

static inline int bcm_oamindex_to_ifname_get_wrap(int portIdx, char *ifName)
{
    strcpy(ifName, "ptm0");
    return 0;
}

static inline int bcm_phy_mode_set_wrap(int portIdx, int speed, int duplex)
{
    return 0;
}

static inline int bcm_port_learning_ind_set_wrap(int portIdx, unsigned char
  learningInd)
{
    return 0;
}

static inline int bcm_port_loopback_set_wrap(int portIdx, int status)
{
    return 0;
}

static inline int bcm_port_rate_egress_set_wrap(int portIdx, 
  unsigned int erc_limit, unsigned int erc_burst)
{
    return 0;
}

static inline int bcm_port_rate_ingress_set_wrap(int portIdx, 
  unsigned int kbits_sec, unsigned int kbits_burst)
{
    return 0;
}

static inline int bcm_port_traffic_control_set_wrap(int portIdx, 
  int ctrl_map)
{
    return 0;
}

static inline int bcm_stat_get_emac_wrap(int portIdx, 
  struct emac_stats *value)
{
    int rc;
    int port;
    XTM_INTERFACE_STATS IntfStats;
    port = PHY0_PATH0;
    memset(value,0,sizeof(struct emac_stats));
    memset(&IntfStats,0,sizeof(XTM_INTERFACE_STATS));
    rc = devCtl_xtmGetInterfaceStatistics(PORTID_TO_PORTMASK(port),&IntfStats,0);
    if(rc == CMSRET_SUCCESS)
    {
       value->rx_byte = (uint64_t)IntfStats.ulIfInOctets;
       value->rx_packet = (uint64_t)IntfStats.ulIfInPackets;
       value->tx_byte = (uint64_t)IntfStats.ulIfOutOctets;
       value->tx_packet = (uint64_t)IntfStats.ulIfOutPackets;
    }
    else if(rc == CMSRET_INTERNAL_ERROR)
    {
       //Failed to open the device.
       rc = -1;
    }
    return rc;
}

static inline int bcm_port_vlan_isolation_set_wrap(int portIdx, 
  unsigned char us, unsigned char ds)
{
    return 0;
}

static inline int bcm_get_linkspeed_wrap(char *ifname, int *speed, int *duplex, 
      enum phy_cfg_flag *phycfg, int *subport)
{
    *phycfg = (1 << 7);
    *speed = 4; //PHY_SPEED_2500
    *duplex = 2; //PHY_FULL_DUPLEX
    *subport = 0;
    return 0;
}
#endif /* _BCMXTM_UNI_IF_WRAP_H_ */
