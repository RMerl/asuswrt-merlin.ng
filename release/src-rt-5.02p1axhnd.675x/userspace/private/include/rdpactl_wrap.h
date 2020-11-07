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
*      This file implements the BCM rdpactl utility wrapper functions
*      with uni port index (a.k.a oam index) as input.
*      The oam index represents the physical Ethernet UNI port numbering
*      order on the CPE device face plate, the numbering is used by the
*      management protocols such as OMCI, EPON OAM, etc. A dynamically
*      created virtual interface or an internal interface does not have
*      OAM index. Typically, the OAM index value is 0-based.
*
*      This file is also a place holder for certain Ethernet-like
*      configurations on a non-Ethernet WAN interface. Such WAN interface is
*      not a real physical Ethernet port, and may not have a directly
*      associated object in the management data model. One example is the
*      MAC address look up configuration on the RDPA wan0 port in the
*      GPON configuration. For those functions, WAN type is used as input.
*
*****************************************************************************/

#ifndef _RDPACTL_WRAP_H_
#define _RDPACTL_WRAP_H_

/* ---- Include Files ----------------------------------------------------- */

#include "rdpactl_api.h"
#include "ethswctl_api.h"


/* ---- Constants and Types ----------------------------------------------- */


/* ---- Macro API definitions --------------------------------------------- */

/* #define CC_RDPACTL_DEBUG */

#if defined(CC_RDPACTL_DEBUG)
#define rdpaCtlTrace(fmt, arg...) \
  printf("[RDPACTL_WRAP] %s(): " fmt "\n", __FUNCTION__, ##arg)
#else
#define rdpaCtlTrace(fmt, arg...)
#endif

#define RDPAIF_RC_DECL(rdpaIf, rc) \
  rdpa_if rdpaIf; \
  int rc = 0; \

#define portIdx2RdpaIfRetOnErr(i, r, rc) \
  rc = bcm_enet_map_oam_idx_to_rdpaif(i); \
  if (rc < 0) \
  { \
      return rc; \
  } \
  r = (rdpa_if)rc;

#define rdpaCtlGetPortParamRetOnErr(rdpaIf, param, rc) \
  rc = rdpaCtl_get_port_param(rdpaIf, &param); \
  if (rc < 0) \
  { \
      printf("rdpaCtl_get_port_param() failed, rdpaIf=%d\n", rdpaIf); \
      return rc; \
  }

#define rdpaCtlSetPortParam(rdpaIf, param, rc) \
  rc = rdpaCtl_set_port_param(rdpaIf, &param); \
  if (rc < 0) \
  { \
      printf("rdpaCtl_set_port_param() failed, rdpaIf=%d\n", rdpaIf); \
  }

#define rdpaInfoTrace(pi, ri, rc) \
  rdpaCtlTrace("(PI%d:RI%d), rc=%d", pi, ri, rc)


/* ---- Variable Externs -------------------------------------------------- */


/* ---- Inline functions -------------------------------------------------- */

static inline int set_port_unknownmac_discard(rdpa_if rdpaIf, 
  unsigned char discard)
{
    rdpa_drv_ioctl_port_param_t param;
    int rc = 0;

    rdpaCtlGetPortParamRetOnErr(rdpaIf, param, rc);

    if (discard == TRUE)
    {
        param.dal_miss_action = rdpa_forward_action_drop;
    }
    else
    {
        /* Let Linux perform the flooding. */
        param.dal_miss_action = rdpa_forward_action_host;
    }

    rdpaCtlSetPortParam(rdpaIf, param, rc);
    return rc;
}

static inline int rdpaCtl_set_port_sa_limit_wrap(int portIdx, 
  unsigned short max_limit)
{
    RDPAIF_RC_DECL(rdpaIf, rc);

    portIdx2RdpaIfRetOnErr(portIdx, rdpaIf, rc);
    rc = rdpaCtl_set_port_sa_limit(rdpaIf, max_limit);
    rdpaInfoTrace(portIdx, rdpaIf, rc);
    return rc;
}

static inline int rdpaCtl_get_port_sal_miss_action_wrap(int portIdx,
  rdpa_forward_action *act)
{
    RDPAIF_RC_DECL(rdpaIf, rc);

    portIdx2RdpaIfRetOnErr(portIdx, rdpaIf, rc);
    rc = rdpaCtl_get_port_sal_miss_action(rdpaIf, act);
    rdpaInfoTrace(portIdx, rdpaIf, rc);
    return rc;
}

static inline int rdpaCtl_get_port_dal_miss_action_wrap(int portIdx,
  rdpa_forward_action *act)
{
    RDPAIF_RC_DECL(rdpaIf, rc);

    portIdx2RdpaIfRetOnErr(portIdx, rdpaIf, rc);
    rc = rdpaCtl_get_port_dal_miss_action(rdpaIf, act);
    rdpaInfoTrace(portIdx, rdpaIf, rc);
    return rc;
}

static inline int rdpaCtl_set_port_sal_miss_action_wrap(int portIdx,
  rdpa_forward_action act)
{
    RDPAIF_RC_DECL(rdpaIf, rc);

    portIdx2RdpaIfRetOnErr(portIdx, rdpaIf, rc);
    rc = rdpaCtl_set_port_sal_miss_action(rdpaIf, act);
    rdpaInfoTrace(portIdx, rdpaIf, rc);
    return rc;
}

static inline int rdpaCtl_set_port_dal_miss_action_wrap(int portIdx,
  rdpa_forward_action act)
{
    RDPAIF_RC_DECL(rdpaIf, rc);

    portIdx2RdpaIfRetOnErr(portIdx, rdpaIf, rc);
    rc = rdpaCtl_set_port_dal_miss_action(rdpaIf, act);
    rdpaInfoTrace(portIdx, rdpaIf, rc);
    return rc;
}

static inline int rdpaCtl_set_port_unknownmac_discard_wrap(int portIdx,
  unsigned char discard)
{
    RDPAIF_RC_DECL(rdpaIf, rc);

    portIdx2RdpaIfRetOnErr(portIdx, rdpaIf, rc);
    rc = set_port_unknownmac_discard(rdpaIf, discard);
    rdpaInfoTrace(portIdx, rdpaIf, rc);
    return rc;
}

/* WAN-side configuration. */
static inline int rdpaCtl_set_wan_learning_ind_wrap(rdpa_wan_type wanType,
  unsigned char learningInd)
{
    rdpa_if wanIf;
    rdpa_drv_ioctl_port_param_t param;
    int rc = 0;

    wanIf = rdpa_wan_type_to_if(wanType);

    rdpaCtlGetPortParamRetOnErr(wanIf, param, rc);

    if (learningInd == TRUE)
    {
        param.sal_enable = TRUE;
        param.dal_enable = TRUE;
        param.sal_miss_action = rdpa_forward_action_host;
        param.dal_miss_action = rdpa_forward_action_host;
    }
    else
    {
        param.sal_enable = FALSE;
        param.dal_enable = FALSE;
        param.sal_miss_action = rdpa_forward_action_forward;
        param.dal_miss_action = rdpa_forward_action_forward;
    }

    rdpaCtlSetPortParam(wanIf, param, rc);
    rdpaInfoTrace(wanType, wanIf, rc);
    return rc;
}

static inline int rdpaCtl_set_wan_unknownmac_discard_wrap(
  rdpa_wan_type wanType, unsigned char discard)
{
    RDPAIF_RC_DECL(wanIf, rc);

    wanIf = rdpa_wan_type_to_if(wanType);
    rc = set_port_unknownmac_discard(wanIf, discard);
    rdpaInfoTrace(wanType, wanIf, rc);
    return rc;
}

static inline int rdpaCtl_set_wan_dal_miss_action_wrap(
  rdpa_wan_type wanType, rdpa_forward_action act)
{
    RDPAIF_RC_DECL(wanIf, rc);

    wanIf = rdpa_wan_type_to_if(wanType);
    rc = rdpaCtl_set_port_dal_miss_action(wanIf, act);
    rdpaInfoTrace(wanType, wanIf, rc);
    return rc;
}

static inline int rdpaCtl_get_wan_dal_miss_action_wrap(
  rdpa_wan_type wanType, rdpa_forward_action *act)
{
    RDPAIF_RC_DECL(wanIf, rc);

    wanIf = rdpa_wan_type_to_if(wanType);
    rc = rdpaCtl_get_port_dal_miss_action(wanIf, act);
    rdpaInfoTrace(wanType, wanIf, rc);
    return rc;
}

#endif /* _RDPACTL_WRAP_H_ */
