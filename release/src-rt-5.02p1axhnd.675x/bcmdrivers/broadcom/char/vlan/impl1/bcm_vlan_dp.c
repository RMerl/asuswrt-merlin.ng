/*
<:copyright-BRCM:2016:proprietary:standard

   Copyright (c) 2016 Broadcom
   All Rights Reserved

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
*/
//**************************************************************************
// File Name  : bcm_vlan_dp.c
//
// Description: Broadcom VLAN Interface Driver
//
//**************************************************************************

/* Include Files. */

#include <linux/nbuff.h>
#include <linux/vlanctl_bind.h>
#include "bcm_vlan_local.h"
#include "bcm_vlan_dev.h"
#if defined(CONFIG_BCM_RDPA) || defined(CONFIG_BCM_RDPA_MODULE)
#include "rdpa_mw_qos.h"
#endif /* CONFIG_BCM_RDPA || CONFIG_BCM_RDPA_MODULE */


/* Private Constants and Types. */

/* #define VLAN_DP_DEBUG 1 */

#if defined(CONFIG_BCM_RDPA) || defined(CONFIG_BCM_RDPA_MODULE)
#define vlanFlowDp2RdpaDirConvert(v)  ((rdpadrv_dp_code)v)
#endif /* CONFIG_BCM_RDPA || CONFIG_BCM_RDPA_MODULE */


/* ---- Private Function Prototypes --------------------------------------- */

#if defined(CONFIG_BCM_RDPA) || defined(CONFIG_BCM_RDPA_MODULE)
static rdpa_traffic_dir vlanFlowDir2RdpaDirConvert(bcmVlan_flowDir_t dir);
#endif /* CONFIG_BCM_RDPA || CONFIG_BCM_RDPA_MODULE */
static UBOOL8 isPktDropEligible(bcmVlan_dpCode_t encoding, UINT16 pbit,
  UINT16 dei);
static bcmVlan_dpCode_t dp_code_get(bcmVlan_flowDir_t dir);
static int dp_code_set(bcmVlan_flowDir_t dir, bcmVlan_dpCode_t code);
static bcmVlan_dpCode_t dp_code_get_by_dev(struct net_device *dev);
static unsigned int *getTpidTable(struct net_device *dev __attribute__((unused)));
void bcmVlan_setDp(bcmVlan_iocSetDropPrecedence_t *iocSetDpPtr);
UBOOL8 bcmVlan_lookupDp(struct net_device *dev, uint8 *data, unsigned int len);


/* Private Variables. */

static int global_drop_precedence_code[2] =
  {BCM_VLAN_DP_CODE_NONE, BCM_VLAN_DP_CODE_NONE};

static unsigned int defaultTpidTable[BCM_VLAN_MAX_TPID_VALUES] =
  {0x8100, 0x88a8, 0x9100, 0x8100};


/* Functions. */

#if defined(CONFIG_BCM_RDPA) || defined(CONFIG_BCM_RDPA_MODULE)
static rdpa_traffic_dir vlanFlowDir2RdpaDirConvert(bcmVlan_flowDir_t dir)
{
   return ((dir == BCM_VLAN_FLOWDIR_US)? rdpa_dir_us: rdpa_dir_ds);
}
#endif /* CONFIG_BCM_RDPA || CONFIG_BCM_RDPA_MODULE */

static UBOOL8 isPktDropEligible(bcmVlan_dpCode_t encoding, UINT16 pbit,
  UINT16 dei)
{
    UBOOL8 val = FALSE;

    if (((encoding == BCM_VLAN_DP_CODE_DEI) &&
        (dei == 1)) ||
        ((encoding == BCM_VLAN_DP_CODE_PCP7P1D) &&
        (pbit == 4)) ||
        ((encoding == BCM_VLAN_DP_CODE_PCP6P2D) &&
        ((pbit == 2) || (pbit == 4))) ||
        ((encoding == BCM_VLAN_DP_CODE_PCP5P3D) &&
        ((pbit == 0) || (pbit == 2) || (pbit == 4))))
    {
        val = TRUE;
    }
#ifdef VLAN_DP_DEBUG
    printk("isPktDropEligible(code%x, pbit%x, dei%x), val%d\n",
      encoding, pbit, dei, val);
#endif /* VLAN_DP_DEBUG */

    return val;
}

static bcmVlan_dpCode_t dp_code_get(bcmVlan_flowDir_t dir)
{
    if (dir >= BCM_VLAN_FLOWDIR_MAX)
    {
        return BCM_VLAN_DP_CODE_NONE;
    }

    return global_drop_precedence_code[dir];
}

static int dp_code_set(bcmVlan_flowDir_t dir, bcmVlan_dpCode_t code)
{
#if defined(CONFIG_BCM_RDPA) || defined(CONFIG_BCM_RDPA_MODULE)
    struct rdpa_mw_drop_precedence_args args;
#endif /* CONFIG_BCM_RDPA || CONFIG_BCM_RDPA_MODULE */
    int rc = 0;

#ifdef VLAN_DP_DEBUG
    printk("dp_code_set(): dir=%d, code=%d\n", dir, code);
#endif /* VLAN_DP_DEBUG */

    if (dir >= BCM_VLAN_FLOWDIR_MAX)
    {
        return -1;
    }

#if defined(CONFIG_BCM_RDPA) || defined(CONFIG_BCM_RDPA_MODULE)
    args.rdpa_dir = vlanFlowDir2RdpaDirConvert(dir);
    args.dp_code = vlanFlowDp2RdpaDirConvert(code);
    vlanctl_notify(VLANCTL_BIND_DROP_PRECEDENCE_SET, &args, VLANCTL_BIND_CLIENT_RUNNER);
#endif /* CONFIG_BCM_RDPA || CONFIG_BCM_RDPA_MODULE */
    {
        global_drop_precedence_code[dir] = code;
    }
    return rc;
}

static bcmVlan_dpCode_t dp_code_get_by_dev(struct net_device *dev)
{
    unsigned int hw_port_type;
    bcmVlan_dpCode_t dpCode = BCM_VLAN_DP_CODE_NONE;

    hw_port_type = netdev_path_get_hw_port_type(dev);

    if (hw_port_type == BLOG_GPONPHY)
    {
        dpCode = dp_code_get(BCM_VLAN_FLOWDIR_US);
    }
    else if ((hw_port_type == BLOG_ENETPHY) || (hw_port_type == BLOG_SIDPHY))
    {
        dpCode = dp_code_get(BCM_VLAN_FLOWDIR_DS);
    }

    return dpCode;
}

/*
 * A slow path packet may traverse multiple VLAN interfaces. For example,
 * a packet may traverse the following chain of VLAN interfaces in the
 * downstream direction:
 *   gpondef--gpon0--gpon0.0--veip0--veip0.x--br0--ethy.0--ethy.
 * Each real VLAN interface has a configurable TPID table. However, the user
 * application may update the TPID table only on one of the VLAN interfaces
 * in the chain. Use the above case as example, the TPID value on interface
 * gpon0 and the packet may be set to 0x88A8, but the TPID value on
 * the transmit interface ethy is still 0x8100.
 * For simplification purpose (to avoid the application update),
 * defaultTpidTable[] is defined here.
 */
static unsigned int *getTpidTable(struct net_device *dev __attribute__((unused)))
{
#ifdef USE_VLANDEV_TPIDTABLE
    struct realDeviceControl *realDevCtrl;
    struct net_device *realDev;

    realDevCtrl = bcmVlan_getRealDevCtrl(dev);
    realDev = bcmVlan_getRealDeviceByName(dev->name, &realDevCtrl);
    if ((realDev == NULL) || (realDevCtrl == NULL))
    {
        BCM_LOG_ERROR(BCM_LOG_ID_VLAN, "Device %s has no VLAN interfaces",
          dev->name);
        return NULL;
    }

    return realDevCtrl->tpidTable;
#else /* USE_VLANDEV_TPIDTABLE */
    return defaultTpidTable;
#endif /* USE_VLANDEV_TPIDTABLE */
}

static UBOOL8 bcmVlan_lookupDpUtil(uint8 *data, unsigned int len
  __attribute__((unused)), unsigned int *tpidTable, bcmVlan_dpCode_t dpCode)
{
    bcmVlan_ethHeader_t *ethHeader;
    bcmVlan_vlanHeader_t *vlanHeader;
    unsigned short vid, pbit, dei;
    UBOOL8 de = FALSE;

#ifdef VLAN_DP_DEBUG
    dumpHexData(data, len);
#endif /* VLAN_DP_DEBUG */

    ethHeader = (bcmVlan_ethHeader_t*)(data);

    if (!BCM_VLAN_TPID_MATCH(tpidTable, (ntohs(ethHeader->etherType))))
    {
        return de;
    }

    /* Only need to check the outer vlan. */
    vlanHeader = &ethHeader->vlanHeader;
    vid = BCM_VLAN_GET_TCI_VID(vlanHeader);
    pbit = BCM_VLAN_GET_TCI_PBITS(vlanHeader);
    dei = BCM_VLAN_GET_TCI_CFI(vlanHeader);
    de = isPktDropEligible(dpCode, pbit, dei);

#ifdef VLAN_DP_DEBUG
    printk("TPID table: 0x%x, 0x%x, 0x%x, 0x%x\n",
      tpidTable[0], tpidTable[1], tpidTable[2], tpidTable[3]);
    printk("Packet len: %u, Outer ethertype: %04X\n\n", len,
      ntohs(ethHeader->etherType));
    printk("-> VLAN Header (0x%p)\n", vlanHeader);
    printk("VID   : %d\n", vid);
    printk("PBITS : %d\n", pbit);
    printk("CFI   : %d\n", dei);
    printk("ETHER : 0x%04X\n", ntohs(vlanHeader->etherType));
    printk("DE    : %d\n", de);
    printk("\n");
#endif /* VLAN_DP_DEBUG */

    return de;
}

/*
 * Function   : bcmVlan_setDp
 * Description:
 *  This function sets the drop precedence configuration table.
 * Parameters :
 *   iocSetDpPtr - ioctl pointer.
 * Returns:
 *   None.
 */
void bcmVlan_setDp(bcmVlan_iocSetDropPrecedence_t *iocSetDpPtr)
{
    BCM_VLAN_DP_LOCK();
    dp_code_set(iocSetDpPtr->dir, iocSetDpPtr->dpCode);
    BCM_VLAN_DP_UNLOCK();
}

/*
 * Function   : bcmVlan_lookupDp
 * Description:
 *  This function finds pbit/dei values from the outer vlan tag, and looks up
 *  the drop precedence configuration table to determine the drop eligibility.
 * Parameters :
 *   dev - network device pointer.
 *   data - packet buffer pointer.
 *   len - packet length.
 * Returns:
 *   drop eligible (1) or not (0);
 */
UBOOL8 bcmVlan_lookupDp(struct net_device *dev, uint8 *data, unsigned int len)
{
    bcmVlan_dpCode_t dpCode;
    unsigned int *tpidTable;
    UBOOL8 de = FALSE;

    BCM_VLAN_DP_LOCK();

    dpCode = dp_code_get_by_dev(dev);
    if (dpCode != BCM_VLAN_DP_CODE_NONE)
    {
        tpidTable = getTpidTable(dev);
        if (tpidTable != NULL)
        {
            de = bcmVlan_lookupDpUtil(data, len, tpidTable, dpCode);
        }
    }

    BCM_VLAN_DP_UNLOCK();
    return de;
}

EXPORT_SYMBOL(bcmVlan_lookupDp);
