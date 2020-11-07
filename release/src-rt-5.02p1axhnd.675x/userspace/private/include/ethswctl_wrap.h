/***********************************************************************
 *
 *  Copyright (c) 2018 Broadcom
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
*      This file implements the BCM ethswctl utility wrapper functions
*      with uni port index (a.k.a oam index) as input.
*      The oam index represents the physical Ethernet UNI port numbering
*      order on the CPE device face plate, the numbering is used by the
*      management protocols such as OMCI, EPON OAM, etc. A dynamically
*      created virtual interface or an internal interface does not have
*      OAM index. Typically, the OAM index value is 0-based.
*
*****************************************************************************/

#ifndef _ETHSWCTL_WRAP_H_
#define _ETHSWCTL_WRAP_H_

/* ---- Include Files ----------------------------------------------------- */

#if defined(BRCM_XTM_UNI)
#include "bcmxtm_uni_if_wrap.h"
#else
#include "ethswctl_api.h"
extern int bcm_get_linkspeed(char *ifname, int *speed, int *duplex, 
      enum phy_cfg_flag *phycfg, int *subport);
/* ---- Constants and Types ----------------------------------------------- */


/* ---- Macro API definitions --------------------------------------------- */

/* #define CC_ETHSWCTL_DEBUG */

#if defined(CC_ETHSWCTL_DEBUG)
#define ethSwTrace(fmt, arg...) \
  printf("[ETHSW_WRAP] %s(): " fmt "\n", __FUNCTION__, ##arg)
#else
#define ethSwTrace(fmt, arg...)
#endif

#define UNIT_PORT_RC_DECL(unit, port, rc) \
  int rc = 0; \
  int unit = 0; \
  int port = 0;

#define portIdx2UnitPortRetOnErr(i, u, p) \
  rc = bcm_enet_map_oam_idx_to_unit_port(i, u, p); \
  if (rc < 0) \
  { \
      printf("Get unit port failed, portIdx=%d\n", portIdx); \
      return -1; \
  }

#define portInfoTrace(i, u, p, rc) \
  ethSwTrace("(I%d:U%d:P%d), rc=%d", i, u, p, rc)


/* ---- Variable Externs -------------------------------------------------- */


/* ---- Inline functions -------------------------------------------------- */

static inline int bcm_oamindex_to_ifname_get_wrap(int portIdx, char *ifName)
{
    UNIT_PORT_RC_DECL(unit, port, rc);

    portIdx2UnitPortRetOnErr(portIdx, &unit, &port);

    rc = bcm_ifname_get(unit, port, ifName);
    portInfoTrace(portIdx, unit, port, rc);
    return rc;
}

static inline int bcm_phy_mode_set_wrap(int portIdx, int speed, int duplex)
{
    UNIT_PORT_RC_DECL(unit, port, rc);

    portIdx2UnitPortRetOnErr(portIdx, &unit, &port);

    rc = bcm_phy_mode_set(unit, port, speed, duplex);
    portInfoTrace(portIdx, unit, port, rc);
    return rc;
}

static inline int bcm_port_learning_ind_set_wrap(int portIdx, unsigned char
  learningInd)
{
    UNIT_PORT_RC_DECL(unit, port, rc);

    portIdx2UnitPortRetOnErr(portIdx, &unit, &port);

    rc = bcm_port_learning_ind_set(unit, port, learningInd);
    portInfoTrace(portIdx, unit, port, rc);
    return rc;
}

static inline int bcm_port_loopback_set_wrap(int portIdx, int status)
{
    UNIT_PORT_RC_DECL(unit, port, rc);

    portIdx2UnitPortRetOnErr(portIdx, &unit, &port);

    rc = bcm_port_loopback_set(unit, port, status);
    portInfoTrace(portIdx, unit, port, rc);
    return rc;
}

static inline int bcm_port_rate_egress_set_wrap(int portIdx, 
  unsigned int erc_limit, unsigned int erc_burst)
{
    UNIT_PORT_RC_DECL(unit, port, rc);

    portIdx2UnitPortRetOnErr(portIdx, &unit, &port);

    rc = bcm_port_rate_egress_set(unit, port, erc_limit, erc_burst);
    portInfoTrace(portIdx, unit, port, rc);
    return rc;
}

static inline int bcm_port_rate_ingress_set_wrap(int portIdx, 
  unsigned int kbits_sec, unsigned int kbits_burst)
{
    UNIT_PORT_RC_DECL(unit, port, rc);

    portIdx2UnitPortRetOnErr(portIdx, &unit, &port);

    rc = bcm_port_rate_ingress_set(unit, port, kbits_sec, kbits_burst);
    portInfoTrace(portIdx, unit, port, rc);
    return rc;
}

static inline int bcm_port_traffic_control_set_wrap(int portIdx, 
  int ctrl_map)
{
    UNIT_PORT_RC_DECL(unit, port, rc);

    portIdx2UnitPortRetOnErr(portIdx, &unit, &port);

    rc = bcm_port_traffic_control_set(unit, port, ctrl_map);
    portInfoTrace(portIdx, unit, port, rc);
    return rc;
}

static inline int bcm_stat_get_emac_wrap(int portIdx, 
  struct emac_stats *value)
{
    UNIT_PORT_RC_DECL(unit, port, rc);

    portIdx2UnitPortRetOnErr(portIdx, &unit, &port);

    rc = bcm_stat_get_emac(unit, port, value);
    portInfoTrace(portIdx, unit, port, rc);
    return rc;
}

static inline int bcm_port_vlan_isolation_set_wrap(int portIdx, 
  unsigned char us, unsigned char ds)
{
    UNIT_PORT_RC_DECL(unit, port, rc);

    portIdx2UnitPortRetOnErr(portIdx, &unit, &port);

    rc = bcm_port_vlan_isolation_set(unit, port, us, ds);
    portInfoTrace(portIdx, unit, port, rc);
    return rc;
}

static inline int bcm_get_linkspeed_wrap(char *ifname, int *speed, int *duplex, 
      enum phy_cfg_flag *phycfg, int *subport)
{
    int ret = 0;
    ret = bcm_get_linkspeed(ifname, speed, duplex, phycfg, subport);
    return ret;
}
#endif /*  BCM_XTM_UNI */
#endif /* _ETHSWCTL_WRAP_H_ */
