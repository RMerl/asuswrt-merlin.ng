/*
*  Copyright 2011, Broadcom Corporation
*
* <:copyright-BRCM:2011:proprietary:standard
* 
*    Copyright (c) 2011 Broadcom 
*    All Rights Reserved
* 
*  This program is the proprietary software of Broadcom and/or its
*  licensors, and may only be used, duplicated, modified or distributed pursuant
*  to the terms and conditions of a separate, written license agreement executed
*  between you and Broadcom (an "Authorized License").  Except as set forth in
*  an Authorized License, Broadcom grants no license (express or implied), right
*  to use, or waiver of any kind with respect to the Software, and Broadcom
*  expressly reserves all rights in and to the Software and all intellectual
*  property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
*  NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
*  BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
* 
*  Except as expressly set forth in the Authorized License,
* 
*  1. This program, including its structure, sequence and organization,
*     constitutes the valuable trade secrets of Broadcom, and you shall use
*     all reasonable efforts to protect the confidentiality thereof, and to
*     use this information only in connection with your use of Broadcom
*     integrated circuit products.
* 
*  2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*     AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*     WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
*     RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
*     ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
*     FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
*     COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
*     TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
*     PERFORMANCE OF THE SOFTWARE.
* 
*  3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
*     ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
*     INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
*     WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
*     IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
*     OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
*     SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
*     SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
*     LIMITED REMEDY.
* :>
*/

//**************************************************************************
// File Name  : AeDriver.c
// This file mainly used to implement AE mode driver.
// Description: Broadcom EPON  Active Ethernet Interface Driver
//               
//**************************************************************************
#include <linux/ctype.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/unistd.h>
#include <linux/string.h>
#include <linux/timer.h>
#include <asm/uaccess.h>

#include "bcm_OS_Deps.h"
#include "board.h"
#include "EponTypes.h"
#include "opticaldet.h"
#include "Laser.h"
#include "EponMac.h"
#include "Lif.h"
#ifdef CONFIG_EPON_10G_SUPPORT
#include "Xif.h"
#endif
#include "mac_drv.h"
#include "wan_drv.h"


extern mac_drv_t mac_drv_epon_ae;
static LaserRate AeRate = LaserRateOff;

#define DefaultHardCodedBurstCap  0x400
#define MaxBcm1gBurstCap 8600
#define AEDefTxOffIdle        1
#define AEDefOffTimeOffset    0
#define Com1gL2DefSize        824
#define Com10gL2DefSize       64
#define RATE_STR_LEN 2
#define AE_DEFAULT_MTU_SIZE   (BCM_ENET_DEFAULT_MTU_SIZE+ BCM_MAX_MTU_EXTRA_SIZE)
static  DEFINE_SPINLOCK(epon_ae_spinlock);

LaserRate AeGetRate (void)
{
    char buf[PSP_BUFLEN_16];

    if (AeRate == LaserRateOff)
    {
        /* default 1G */
        AeRate = LaserRate1G;
        
        if (kerSysScratchPadGet(RDPA_WAN_RATE_PSP_KEY, buf, PSP_BUFLEN_16) > 0)
            if (strlen(buf) >= RATE_STR_LEN)
            {
                if (!strncasecmp(buf, RDPA_WAN_RATE_10G, RATE_STR_LEN))
                    AeRate = LaserRate10G;
                else if(!strncasecmp(buf, RDPA_WAN_RATE_2_5G, RATE_STR_LEN))
                    AeRate = LaserRate2G;
            }
    }
    return AeRate;
}

void AeSetRate (LaserRate rate)
{
    AeRate = rate;
}

BOOL AeCheckRunnerEmpty(void)
{
    bdmf_boolean is_empty = 0;
    bdmf_error_t rc = 0;
    bdmf_object_handle port_obj = NULL;
    rdpa_if port = rdpa_wan_type_to_if(rdpa_wan_epon);

    rc = rdpa_port_get(port, &port_obj);
    if (rc)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_AE, "failed to get rdpa wan port. rc=%d\n", rc);
        return is_empty;
    }
    
    rc = rdpa_port_is_empty_get(port_obj, &is_empty);
    if (rc)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_AE, "failed to get port empty status. rc=%d\n", rc);
    }
    
    bdmf_put(port_obj);
    return is_empty;
}

void AeSetRunnerPortTraffic(BOOL enable)    
{
    bdmf_error_t rc = 0;
    bdmf_object_handle port_obj = NULL;
    rdpa_if port = rdpa_wan_type_to_if(rdpa_wan_epon);

    rc = rdpa_port_get(port, &port_obj);
    if (rc)  
        {
        BCM_LOG_ERROR(BCM_LOG_ID_AE, "Failed to get rdpa wan port. rc=%d\n", rc);
        return;
        }

    rc = rdpa_port_enable_set(port_obj, enable);
    if (rc)
        {
        BCM_LOG_ERROR(BCM_LOG_ID_AE, "Failed to enable/disable rdpa wan port. rc=%d\n", rc);
        }
    
    bdmf_put(port_obj);
}

void AeLaserTxModeSet (LaserTxMode mode)
    {
    spin_lock_bh(&epon_ae_spinlock);
#ifdef CONFIG_EPON_10G_SUPPORT   
    XifLaserTxModeSet (mode);
#endif
    LifLaserTxModeSet(mode);
    spin_unlock_bh(&epon_ae_spinlock);
    } 

static void AeLaserRxEnable(void)
    {
#ifdef CONFIG_EPON_10G_SUPPORT
    XifRxEnable();
#endif
    LifRxEnable();
    }

static void AeLaerRxDisable(void)
    {
#ifdef CONFIG_EPON_10G_SUPPORT
    XifRxDisable();
#endif
    LifRxDisable();
    DelayNs(1000); // 1 us to ensure all operatiosn are complete (XIF only?)
    }

void AeLaserRxPowerSet(bdmf_boolean on)
    {
    spin_lock_bh(&epon_ae_spinlock);
    if (on)
        AeLaserRxEnable();
    else
        AeLaerRxDisable();
    spin_unlock_bh(&epon_ae_spinlock);
    }

static void AeDisableL1L2(LinkMap links)
    {
    //L2MapOfLinks(links), link is 0
    U32  l2s = 0x1;
    U32  l1s = l2s;

    //FLush L2 FIFO
    EponRepeatlyFlushL2Fifo(links, l2s, l1s);

    // Put L1 FIFO into reset
    EponSetL1Reset (l1s);

    // Put L2 FIFO into reset
    EponSetL2Reset (l2s);
    }

////////////////////////////////////////////////////////////////////////////////
// extern
void AeStopLinks(LinkMap links)
    {
    // disable upstream transmission so that no frames are in transit
    EponUpstreamDisable(links);
    // now that no frames are coming in or going out it is safe to disable the
    // L1/L2 fifos/schedulers
    AeDisableL1L2(links);
    EponBBHUpsHaultStatusClr();
    EponBBHUpsHaultClr();
    }

///////////////////////////////////////////////////////////////////////////////
//extern
static void AeEnableL1L2(LinkMap links)
    {
    U32  l2s = 0x1;
    U32  l1s = l2s;

    // Put L2 FIFO out of reset
    EponClearL2Reset(l2s);

    // Put L1 FIFO out of reset
    EponClearL1Reset(l1s);
    }

////////////////////////////////////////////////////////////////////////////////
// extern
void AeStartLinks(LinkMap links)
    {
    AeEnableL1L2(links);
    // now that the L1/L2 are back up it's safe to transmit upstream
    EponUpstreamEnable(links);
   }

////////////////////////////////////////////////////////////////////////////////
/// \brief Applies a new burst cap to a link on the fly
///
/// \param link     Link index for burst cap
/// \param bcap     New burst cap values in a array of 16 bytes values
///
/// \return
/// None
////////////////////////////////////////////////////////////////////////////////
//extern
void AeSetBurstCap(LinkIndex link, const U16 * bcap)
    {
    U8  pri = 0;
    U32 newBcap;

        if ((bcap == NULL) || (bcap[pri] < DefaultHardCodedBurstCap))
            {
            newBcap = DefaultHardCodedBurstCap;
            }
        else
            {
            newBcap = (U32)bcap[pri];
            }

        // Apply a 10x factor when in 10/10
        if (AeGetRate() == LaserRate10G)
            {
            newBcap *= 10;
            }
 
        if (AeGetRate() == LaserRate1G)
            {
            if (newBcap > MaxBcm1gBurstCap) 
                newBcap = MaxBcm1gBurstCap;
            }
        
        EponSetBurstCap (link, newBcap);
    } //AeSetBurstCap

static Polarity AeTxPolarity (UINT8  ponIf)
    {
    TRX_SIG_ACTIVE_POLARITY lbe_polarity = TRX_ACTIVE_HIGH;
    int bus = -1;

    opticaldet_get_xpon_i2c_bus_num(&bus);
    if (0 == trx_get_lbe_polarity(bus, &lbe_polarity))
        {
        return (TRX_ACTIVE_HIGH == lbe_polarity) ? ActiveHi: ActiveLo;
        }
    else /* use default */
        {
        return (Polarity)GetEponDefTxPolarity();
        }
    }

void AeSetNumPri (void)
    {
    bdmf_error_t drv_error = BDMF_ERR_OK;
    epn_control_0 control_0;
    epn_control_1 control_1;
    epn_multi_pri_cfg_0 multi_pri_cfg_0;

    drv_error += ag_drv_epn_control_0_get(&control_0);
    drv_error += ag_drv_epn_multi_pri_cfg_0_get(&multi_pri_cfg_0);

    control_0.rptselect = 1;
    multi_pri_cfg_0.cfgrptmultipri0 = FALSE;
    multi_pri_cfg_0.cfgrptswapqs0 = FALSE;
    multi_pri_cfg_0.cfgrptgntsoutst0 = FALSE;
    multi_pri_cfg_0.cfgsharedl2 = FALSE;
    multi_pri_cfg_0.cfgsharedburstcap = FALSE;

    drv_error += ag_drv_epn_control_1_get(&control_1);
    control_1.cfgctcrpt = 0;
    drv_error += ag_drv_epn_control_1_set(&control_1);

    drv_error += ag_drv_epn_control_0_set(&control_0);
    drv_error += ag_drv_epn_multi_pri_cfg_0_set(&multi_pri_cfg_0);
    } 

static void AeQueueInit(LaserRate rate)
{
    UINT32    size = 0;
    UINT16    endAddress = 0;
    
    if (rate == LaserRate10G)
        size = Com10gL2DefSize;
     else
        size = Com1gL2DefSize;

    AeSetNumPri();
    
    endAddress = EponSetL2EventQueue (0, size, endAddress);
    EponSetBurstLimit(0, 100, 100);
}

void AeMacInit(LaserRate rate)
{
    EponTopInit();
    DelayMs(10);
    EponEpnInit (AE_DEFAULT_MTU_SIZE, FALSE, FALSE, rate);
    
    if (rate < LaserRate10G)
        {
        LifInit (AEDefOffTimeOffset, AEDefTxOffIdle, AeTxPolarity(0), rate, rate);
        }

#ifdef CONFIG_EPON_10G_SUPPORT
    else if (rate == LaserRate10G)
        {
        XifInit (AEDefOffTimeOffset, AEDefTxOffIdle, AeTxPolarity(0), rate, rate);
        }
#endif
    else
        {
        BCM_LOG_ERROR(BCM_LOG_ID_AE, "upsupported wan rate\n");
        return;
        }

    AeQueueInit(rate);
}
void AeInit(BOOL serdesInit)
{
    serdes_wan_type_t serdes_wan_type;
    int rc = 0;

    EponSetMacMode(EPON_MAC_AE_MODE);
    if (serdesInit)
    {
        switch (AeGetRate())
        {
            case LaserRate1G:
                serdes_wan_type = SERDES_WAN_TYPE_AE;
                break;
            case LaserRate2G:
                serdes_wan_type = SERDES_WAN_TYPE_AE_2_5G;
                break;
#ifdef CONFIG_EPON_10G_SUPPORT 
            case LaserRate10G:
                serdes_wan_type = SERDES_WAN_TYPE_AE_10G;
                break;
    #endif            
            default:
                serdes_wan_type = SERDES_WAN_TYPE_AE;
                break;
        }
        rc = wan_serdes_config(serdes_wan_type);
        if (rc)
        {
            BCM_LOG_ERROR(BCM_LOG_ID_AE, "WAN serdes config fails errno rc=%d\n", rc);
        }
    }
    EponMapInit();
    AeMacInit(AeGetRate());
}

void EponAeDriverInit(BOOL serdesInit)
{
    AeInit(serdesInit);
    mac_driver_set(&mac_drv_epon_ae);
    mac_driver_init(MAC_TYPE_EPON_AE);
    BCM_LOG_INFO(BCM_LOG_ID_AE, "AE driver init done");
}
