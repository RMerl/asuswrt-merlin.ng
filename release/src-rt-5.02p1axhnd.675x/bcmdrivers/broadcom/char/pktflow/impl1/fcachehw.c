/*
<:copyright-BRCM:2007:proprietary:standard

   Copyright (c) 2007 Broadcom 
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

/*
 *******************************************************************************
 * File Name  : fcachehw.c
 * This file is fcache interface to HW Accelerators.
 *******************************************************************************
 */
#include <bcmenet_common.h>
#include <linux/nbuff.h>
#include <linux/blog.h>
#include <linux/blog_net.h>
#include "fcache.h"
#include "fcachehw.h"
#include "idx_pool_util.h"

extern int  fcache_max_ent(void);

/*----- Forward declaration -----*/

/*----- APIs exported to ONLY fcachedrv.c fhw_xyz() -----*/
/*----- helper functions in fcachedrv.c -----*/

/*
 *------------------------------------------------------------------------------
 * Flow cache design debugging.
 *------------------------------------------------------------------------------
 */
#undef PKT_DBG_SUPPORTED
#define CLRsys              CLRb
#define DBGsys              "[FHW] "
#if defined(CC_CONFIG_FHW_DBGLVL)
#define PKT_DBG_SUPPORTED 
#define PKT_ASSERT_SUPPORTED
static int pktDbgLvl = CC_CONFIG_FHW_DBGLVL;
#endif
#if defined(CC_CONFIG_FHW_COLOR)
#define PKT_DBG_COLOR_SUPPORTED
#endif
#include "pktDbg.h"
#include <linux/bcm_log.h>

#if defined(CC_CONFIG_FCACHE_DEBUG)    /* Runtime debug level setting */
int fcacheFhwDebug(int lvl) { dbg_config( lvl ); return lvl; }
#endif

#if defined(CONFIG_BCM_RDPA) || defined(CONFIG_BCM_RDPA_MODULE)
#include <rdpa_api.h>
#include <bdmf_interface.h>
#include <rdpa_types.h>
#include <rdpa_cpu.h>
#include <rdpa_ag_cpu.h>
#endif

/*----- Globals -----*/

static uint32_t g_fhw_hw_accel_enabled = 1; /* Global control to enable/disable HW acceleration */

typedef struct {
    uint32_t cumm_hw_hits;
    unsigned long long cumm_hw_bytes;
} ExtFlow_t;

typedef struct {
    /*
     * hooks initialized by HW Packet Accelerators (CMF, FAP, etc.) 
     * FHW makes upcalls into HW Packet Accelerators via theses hooks
     */
    FhwBindHwHooks_t hwHooks;
    IdxPool_t      freeIdxPool;
    HwEnt_t        *hwtbl;
} HwAcc_t;

typedef struct {
    uint32_t    bindCount;
    int         status;           
    int         unused;           
    ExtFlow_t   *fctbl;
    FhwHwAccPrio_t cap2HwAccMap[HW_CAP_MAX][FHW_PRIO_MAX];
    HwAcc_t     hwAcc[FHW_PRIO_MAX] _FCALIGN_;
} __attribute__((aligned(16))) Fhw_t;


Fhw_t fhw; /* Global FHW context */
static FC_CLEAR_HOOK fhw_fc_clear_hook = (FC_CLEAR_HOOK)NULL;

uint32_t inline calcU32RollOverDiff(uint32_t currVal, uint32_t prevVal, uint32_t maxVal)
{
    uint32_t diffVal;

    if (currVal >= prevVal) 
    {
       diffVal = currVal - prevVal;
    }
    else
    {
       diffVal = (maxVal - prevVal) + currVal;
    }
    return diffVal;
}

#if defined(CONFIG_BCM_XRDP)
#define HW_NUM_OF_BITS_IN_HIT_COUNT 28
#define HW_NUM_MAX_PKT_HIT_COUNT ((1U<< HW_NUM_OF_BITS_IN_HIT_COUNT ) - 1)
#else
#define HW_NUM_OF_BITS_IN_HIT_COUNT 32
#define HW_NUM_MAX_PKT_HIT_COUNT U32_MAX
#endif

#define HW_NUM_MAX_BYTE_HIT_COUNT U32_MAX

uint32_t inline calcHwPktHitDiff(uint32_t currVal, uint32_t prevVal)
{
    return calcU32RollOverDiff(currVal, prevVal, HW_NUM_MAX_PKT_HIT_COUNT);
}

uint32_t inline calcHwByteHitDiff(uint32_t currVal, uint32_t prevVal)
{
    return calcU32RollOverDiff(currVal, prevVal, HW_NUM_MAX_BYTE_HIT_COUNT);
}

/*
 *------------------------------------------------------------------------------
 * Function     : fhw_alloc_hwtbl
 * Description  : Allocates the HW entries table for the HWACC
 *------------------------------------------------------------------------------
 */
static int fhw_alloc_hwtbl(FhwHwAccPrio_t hwAccIx, int max_ent)
{
    int idx;
    HwEnt_t *hwEnt_p = NULL;
    char fhw_name[16];

    dbgl_print( DBG_EXTIF, "hwAccIx<%d> max_ent<%d>", hwAccIx, max_ent );

    /* one extra entry to cache line align (16B) */
    hwEnt_p = (HwEnt_t *) 
        ( ( (uintptr_t) kmalloc( (sizeof(HwEnt_t) * (max_ent+1) ),
        GFP_ATOMIC) ) & ~0x0F);

    if (hwEnt_p == NULL)
    {
        fc_error("Out of memory for HW Table" );   
        return FHW_ERROR;
    }
    memset( (void*)hwEnt_p, 0, (sizeof(HwEnt_t) * max_ent) );
    fhw.hwAcc[hwAccIx].hwtbl = hwEnt_p; 

    /* Initialize each HW entry */
    for (idx=0; idx < max_ent; idx++)
    {
        hwEnt_p = &fhw.hwAcc[hwAccIx].hwtbl[idx]; 
        hwEnt_p->hw_key = FHW_TUPLE_INVALID;
    }
    /* Initialize the index pool for HW Table (hwtbl) */
    snprintf(fhw_name, sizeof(fhw_name), "FHW[%d]",hwAccIx);

    if ( idx_pool_init(&fhw.hwAcc[hwAccIx].freeIdxPool, max_ent, fhw_name) )
    {
        return FHW_ERROR;
    }
    return FHW_SUCCESS;
}


/*
 *------------------------------------------------------------------------------
 * Function     : fhw_free_hwtbl
 * Description  : Frees all the HW entries for the HWACC
 *------------------------------------------------------------------------------
 */
static int fhw_free_hwtbl(FhwHwAccPrio_t hwAccIx)
{
    /* Release the index pool for hwtbl */
    idx_pool_exit(&fhw.hwAcc[hwAccIx].freeIdxPool);

    if (fhw.hwAcc[hwAccIx].hwtbl == NULL)
        return FHW_SUCCESS;

    kfree(fhw.hwAcc[hwAccIx].hwtbl);

    fhw.hwAcc[hwAccIx].hwtbl = NULL;

    return FHW_SUCCESS;
}

static int fhw_clear( uint32_t key, const FlowScope_t scope );

int fhw_unbind_hw(FhwHwAccPrio_t hwAccIx)
{
    dbg_assertr( (hwAccIx < FHW_PRIO_MAX), FHW_ERROR );
    dbgl_print( DBG_EXTIF, "hwAccIx<%d>", hwAccIx );

    if (hwAccIx >= FHW_PRIO_MAX)
        return FHW_ERROR;

    if (fhw.bindCount > 0 && fhw.hwAcc[hwAccIx].hwtbl != NULL)
    {
        fhw_free_hwtbl(hwAccIx); 
        memset(&fhw.hwAcc[hwAccIx].hwHooks, 0, sizeof(FhwBindHwHooks_t));

        fhw.bindCount--;
        fhw_bind_fc(fhw.bindCount);
    }
    else
    {
        printk( CLRbold2 PKTFLOW_MODNAME "HW acceleration already disabled." CLRnl );
    }

    return FHW_SUCCESS;
}


/*
 *------------------------------------------------------------------------------
 * Function     : fhw_bind_hw
 * Description  : Binds the HW hooks to activate, deactivate and refresh / refresh_pathstat.
 *                Passes a pointer to fhw_clear() functions.
 *------------------------------------------------------------------------------
 */
int fhw_bind_hw(FhwHwAccPrio_t hwAccIx, FhwBindHwHooks_t *hwHooks_p)
{
    dbg_assertr( (hwAccIx < FHW_PRIO_MAX), FHW_ERROR );

    if (hwAccIx >= FHW_PRIO_MAX)
    {        
        fc_error("fhw_bind_hw invalid parameter hwAccIx=%d",hwAccIx );   
        goto bind_hw_error;
    }

    if ( hwHooks_p->activate_fn == (HOOKP32) NULL )
    {
        *hwHooks_p->fhw_clear_fn = (FC_CLEAR_HOOK)NULL;
        return fhw_unbind_hw(hwAccIx);
    }

    if (fhw.hwAcc[hwAccIx].hwtbl == NULL)
    {
        if (fhw_alloc_hwtbl(hwAccIx, hwHooks_p->max_ent) != FHW_SUCCESS)
        {
            fc_error("fhw_alloc_hwtbl invalid parameter max_ent=%d",hwHooks_p->max_ent );  
            goto bind_hw_error;
        }

        memcpy(&fhw.hwAcc[hwAccIx].hwHooks, hwHooks_p, sizeof(FhwBindHwHooks_t));

        if ( hwHooks_p->activate_fn != (HOOKP32) NULL )
            *hwHooks_p->fhw_clear_fn = fhw_clear;    /* downcall hook from HWACC */
        else
            *hwHooks_p->fhw_clear_fn = (FC_CLEAR_HOOK)NULL;

        fhw.bindCount++;
        fhw_bind_fc(fhw.bindCount);

        /* Update hw support for active flows
           This is necessary when disabling and reenabling hw acceleration */
        fc_update_hw_support();
    }
    else
    {
        printk( CLRbold PKTFLOW_MODNAME "Hardware acceleration already enabled." CLRnl );
    }

    return FHW_SUCCESS;

bind_hw_error:
    fc_error("fhw_bind_hw invalid parameter" );   
    return FHW_ERROR;
}


static inline ExtFlow_t * _efc_entry(uint32_t flowIx)
{
    ExtFlow_t * eflow_p = &fhw.fctbl[flowIx];
    return eflow_p;
}

static inline void _fhw_new_flow(uint32_t flowIx, uint32_t flow_type)

{
    ExtFlow_t * eflow_p = _efc_entry(flowIx);

    if (flow_type == HW_CAP_MCAST_WHITELIST)
        return;

    dbgl_print( DBG_CTLFL, "flowIx<%d>", flowIx );

    eflow_p->cumm_hw_hits = 0;
    eflow_p->cumm_hw_bytes = 0;
}

/*
 *------------------------------------------------------------------------------
 * Function   : fhw_cur_stats_hw
 * Description: Determine the HW hits from previous invocation
 * Returns    : Number of hits in HW hardware.
 *------------------------------------------------------------------------------
 */
uint32_t fhw_cur_stats_hw(FhwKey_t key, uint32_t *hw_hits_p, 
                          unsigned long long *hw_bytes_p)
{
    FhwHwAccPrio_t hwAccIx = key.id.accl;
    uint32_t entIx;
    HwEnt_t *hwEnt_p = NULL;

    dbgl_print( DBG_CTLFL, "fhw<0x%8x>", key.id.fhw);
    dbg_assertr( (hw_hits_p != NULL), 0 );
    dbg_assertr( (hw_bytes_p != NULL), 0 );

    if ( key.word == FHW_TUPLE_INVALID )     /* caller checks already */
        return 0;

    dbg_assertr( (hwAccIx < FHW_PRIO_MAX), 0);

    entIx = key.id.fhw;
    hwEnt_p = &fhw.hwAcc[hwAccIx].hwtbl[entIx]; 

    dbgl_print( DBG_INTIF, "fhw<0x%8x> hwAccIx<%d>, entIx<%d> hwEnt_p<%p>", 
                           key.id.fhw, hwAccIx, entIx, hwEnt_p);

    *hw_hits_p = hwEnt_p->hw_hits_cumm;
    *hw_bytes_p = hwEnt_p->hw_bytes_cumm;

    dbgl_print( DBG_INTIF, "fhw<0x%8x> hw_hits<%u>, hw_bytes<%u>", 
                           key.id.fhw, 
                           *(uint32_t *)hw_hits_p, *(uint32_t *)hw_bytes_p);
    return (*hw_hits_p);
}


static inline void _fhw_update_cumm_hw_stats(uint32_t flowIx, 
                                             uint32_t hw_hits, uint32_t hw_bytes)
{
    ExtFlow_t *eflow_p = _efc_entry(flowIx);

    eflow_p->cumm_hw_hits += hw_hits;
    eflow_p->cumm_hw_bytes += hw_bytes;
}

#if defined(CONFIG_BCM_GMAC)
/* Is the GMAC port active ?
 * i.e. The chip supports GMAC (runtime decision) &&
 * currnetly GMAC port is configured as WAN and   &&
 * GMAC is enabled to caryy traffic (i.e Not in ROBO mode) */
inline int fhwChkGmacActive( void )
{
    bcmFun_t *gmacActiveFn = bcmFun_get(BCM_FUN_ID_ENET_GMAC_ACTIVE);

    if (gmacActiveFn)
    {
        return gmacActiveFn(NULL);
    }
    else
        return 0;
}

#else
#define fhwChkGmacActive() 0
#endif



/*
 *------------------------------------------------------------------------------
 * Function     : fhw_mcast_dflt_hw
 * Description  : Check if the HW support multicast default drop rule
 *------------------------------------------------------------------------------
 */
unsigned int fhw_mcast_dflt_hw( void )
{
    return 0;
}

#if (defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE)) && defined(CONFIG_BCM_EXT_SWITCH)
/* FAP cannot accelerate traffic when multicast clients on external & internal switch ports */
int fhw_chk_int_ext_sw_ports_conflict( Blog_t * blog_p )
{
    uint32_t other_unit = 0;
    uint16_t other_unit_port_map = 0;

    other_unit = LOGICAL_PORT_TO_UNIT_NUMBER( blog_p->tx.info.channel ) ? 0 : 1;
    other_unit_port_map = GET_PORTMAP_FROM_LOGICAL_PORTMAP( blog_p->mcast_port_map, other_unit );

    return ( other_unit_port_map );
}
#else
#define fhw_chk_int_ext_sw_ports_conflict( blog_p ) 0
#endif


#define FHW_IS_RX_DEV_WAN(blog_p) ( ((struct net_device *)(blog_p)->rx_dev_p)->priv_flags & IFF_WANDEV )
#define FHW_IS_TX_DEV_WAN(blog_p) ( ((struct net_device *)(blog_p)->tx_dev_p)->priv_flags & IFF_WANDEV )
#define FHW_IS_ANY_DEV_WAN(blog_p) ( FHW_IS_RX_DEV_WAN(blog_p) || FHW_IS_TX_DEV_WAN(blog_p) )

/*
 *------------------------------------------------------------------------------
 * Function     : fhw_get_gre_flow_support
 * Description  : Check if the HW support various type of GRE acceleration
 *------------------------------------------------------------------------------
 */
static int fhw_get_gre_flow_support(Blog_t * blog_p ) 
{
#if defined(CONFIG_BCM_RDPA) || defined(CONFIG_BCM_RDPA_MODULE)
#if !defined(CONFIG_BCM963138) && !defined(CONFIG_BCM963148) && !defined(CONFIG_BCM94908) && !defined(CONFIG_BCM963158)
    /* 
        Support GRE termination:
        the transmit should be different from recieve for perfrom termination
    */
    /* L2 acceleration GRE pass thru case should be accelerated */
    if ((blog_p->rx.info.bmap.PLD_L2 == 1) && (blog_p->rx.info.bmap.GRE == 1) && (blog_p->tx.info.bmap.GRE == 1))
        return 1;

    if (blog_p->grerx.gre_flags.ver == 1 && blog_p->gretx.gre_flags.ver == 1)
        return 1;

    /* Runner support L2GRE_4in4 and L3GRE_4in4 full acceleration */
    if (TG24in4DN(blog_p) || TG24in4UP(blog_p))
        return 1;
    
    /* GRE_tunnel_in_tunnel_out */
    if (TOTG4(blog_p))
        return 1;

#if defined(CONFIG_BCM_XRDP)
    /* L3GRE_4in6 */
    if ((TG4in6DN(blog_p) || TG4in6UP(blog_p)) && !RX_GRE_ETH(blog_p))
        return 1;    

    /* L3GRE_6in4 */
    if ((TG6in4DN(blog_p) || TG6in4UP(blog_p)) && !RX_GRE_ETH(blog_p))
        return 1;     
#endif    
#else
    /* L2 acceleration GRE pass thru case should be accelerated */
    if ((blog_p->rx.info.bmap.PLD_L2 == 1) && (blog_p->rx.info.bmap.GRE == 1) && (blog_p->tx.info.bmap.GRE == 1))
        return 1;

    /* GRE Acceleration. No support for GRE flags (C, K, S, etc.) */
    if ((RX_GRE(blog_p) && !TX_GRE(blog_p) && blog_p->grerx.gre_flags.u16) || 
        (!RX_GRE(blog_p) && TX_GRE(blog_p) && blog_p->gretx.gre_flags.u16) ||
        ((TOTG4(blog_p) || TOTG6(blog_p)) && (blog_p->grerx.gre_flags.u16 || blog_p->gretx.gre_flags.u16)))
        return 0;

    /* White list */
    /* Runner support L3GRE_4in4 acceleration */
    if (TG4in4DN(blog_p) || TG4in4UP(blog_p))
        return 1;
    
    /* L2GRE_4in4 */
    if (TG2in4DN(blog_p) || (TG2in4UP(blog_p) && blog_p->l2_ipv4))
        return 1;
    
    /* GRE_tunnel_in_tunnel_out */
    if (TOTG4(blog_p))
        return 1;

    /* L3GRE_4in6 */
    if ((TG4in6DN(blog_p) || TG4in6UP(blog_p)) && !RX_GRE_ETH(blog_p))
        return 1;    

    /* L3GRE_6in4 */
#if defined(CONFIG_BCM963158)
    if ((TG6in4DN(blog_p) || TG6in4UP(blog_p)) && !RX_GRE_ETH(blog_p))
        return 1;     
#endif
#endif
#endif

#if defined(CONFIG_BCM_ARCHER) || defined(CONFIG_BCM_ARCHER_MODULE)
    /* L2 acceleration GRE pass thru case should be accelerated */
    if ((blog_p->rx.info.bmap.PLD_L2 == 1) && (blog_p->rx.info.bmap.GRE == 1) && (blog_p->tx.info.bmap.GRE == 1))
        return 1;

    /* GRE Acceleration. No support for GRE flags (C, K, S, etc.) */
    if ((RX_GRE(blog_p) && !TX_GRE(blog_p) && blog_p->grerx.gre_flags.u16) || 
        (!RX_GRE(blog_p) && TX_GRE(blog_p) && blog_p->gretx.gre_flags.u16) ||
        ((TOTG4(blog_p) || TOTG6(blog_p)) && (blog_p->grerx.gre_flags.u16 || blog_p->gretx.gre_flags.u16)))
        return 0;

    /* White list */
    /* Runner support L3GRE_4in4 acceleration */
    if (TG4in4DN(blog_p) || TG4in4UP(blog_p))
        return 1;
    
    /* L2GRE_4in4 */
    if (TG2in4DN(blog_p) || (TG2in4UP(blog_p) && blog_p->l2_ipv4))
        return 1;
    
    /* GRE_tunnel_in_tunnel_out */
    if (TOTG4(blog_p))
        return 1;

    /* L3GRE_4in6 */
    if ((TG4in6DN(blog_p) || TG4in6UP(blog_p)) && !RX_GRE_ETH(blog_p))
        return 1;    

    /* L3GRE_6in4 */
    if ((TG6in4DN(blog_p) || TG6in4UP(blog_p)) && !RX_GRE_ETH(blog_p))
        return 1;     
#endif

    return 0;
}

static int fhw_is_L2L_supported( void )
{
#if (defined(CONFIG_BCM_RDPA) || defined(CONFIG_BCM_RDPA_MODULE))
    /* On some Runner based platforms Ethernet LAN-to-LAN unicast flows are not accelerated in runner. */
#if defined(CONFIG_BCM_PON)
     /* Depends on port configuration. We promote this flow to runner in any case.
        Runner will check on runtime the dst or src port configuration and decide to use FC or bridge logic */
     return 1;
#else
     /* 63138/63148 - Runner FW does not support and expects integration L2 switch will take care */
     return 0;
#endif
#endif
     /* When GMAC is active, LAN-to-LAN unicast flows are accelerated by fcache */
     if (fhwChkGmacActive())
     {
         return 0;
     }
     return 1;
}

/*
 *------------------------------------------------------------------------------
 * Function Name: fhw_is_xoa_flw
 * Description  : Return true if this is a ATM flow. HW acceleration is not
 *                possible currently for XoA flows on Runner platforms.
 *                Examples are PPPoA, IPoA.
 *------------------------------------------------------------------------------
 */
static inline int fhw_is_xoa_flw(Blog_t *blog_p)
{
#if defined(CONFIG_BCM_RDPA) || defined(CONFIG_BCM_RDPA_MODULE)
     struct net_device *xtm_dev_p = NULL;

     if (blog_p->rx.info.bmap.BCM_XPHY)
     {
         xtm_dev_p = blog_p->rx_dev_p;
     }
     else if (blog_p->tx.info.bmap.BCM_XPHY)
     {
         xtm_dev_p = blog_p->tx_dev_p;
     }
     if (xtm_dev_p)
     {
         uint32_t phyType = netdev_path_get_hw_port_type(xtm_dev_p); 

         phyType = BLOG_GET_HW_ACT(phyType);
         if ( (phyType == VC_MUX_PPPOA) ||
              (phyType == VC_MUX_IPOA) ||
              (phyType == LLC_SNAP_ROUTE_IP) ||
              (phyType == LLC_ENCAPS_PPP) )
         {
             return 1;
         }
     }
#endif
     return 0;
}

/* Below compile flag disables the Multicast Acceleration of WLAN traffic
 * This flag is introduced to keep the backward compatibility with earlier
 * releases (where WLAN multicast was never accelerated in HW on DSL platforms). */
//#define BCM_DISABLE_WLAN_MCAST_HW_ACCELERATION 

//static void inline fhw_set_hybrid(uint32_t *hybrid_p)
void inline fhw_set_hybrid(uint32_t *hybrid_p)
{
#if defined(CONFIG_BCM_FCACHE_CLASSIFICATION_BYPASS)
#if !defined(CONFIG_BCM_PON)
    *hybrid_p = 1;
#endif
#endif
}

/*
 *------------------------------------------------------------------------------
 * Function Name: fhw_chk_support_hw_hybrid
 * Description  : Exported fcache API to determine whether "Hybrid" HW acceleration
 *                is supported for this flow. "Hybrid" HW acceleration means
 *                that HW is only performing the flow classification but
 *                modification & forwarding is done by flow-cache.
 *------------------------------------------------------------------------------
 */
int fhw_chk_support_hw_hybrid( Blog_t * blog_p )
{
#if defined(CONFIG_BCM_FCACHE_CLASSIFICATION_BYPASS)

    /* Add specific check for each traffic scenario that are tested and add */

#endif /* defined(CONFIG_BCM_FCACHE_CLASSIFICATION_BYPASS) */
    return 0;
}

/*
 *------------------------------------------------------------------------------
 * Function Name: fhw_chk_support_hw
 * Description  : Exported fcache API to determine whether HW acceleration
 *                is supported.
 *------------------------------------------------------------------------------
 */
int fhw_chk_support_hw( Blog_t * blog_p, uint32_t *out_hybrid_p )
{

    *out_hybrid_p = 0;
    /* No HW Accelerator platforms */
#if defined(CONFIG_BCM963381)
    return 0;
#endif

    if (!g_fhw_hw_accel_enabled) 
        return 0;

#if defined(CONFIG_BCM_PON)

    /* IPv6 over IPv4 tunnel pass through is not supported by NP PON FW */  
    if ( RX_IP6in4(blog_p) )
        return 0;

    /* sdnat is not supported by NP PON FW */ 
    if (blog_p->rx.tuple.saddr != blog_p->tx.tuple.saddr &&
        blog_p->rx.tuple.daddr != blog_p->tx.tuple.daddr && !(MAPT(blog_p)) && !(RX_GRE(blog_p) || TX_GRE(blog_p)))
    {
        return 0;
    }

    /* dnat between two LAN  subnets is not supported by NP PON FW */
    if (!FHW_IS_RX_DEV_WAN(blog_p) &&
        blog_p->rx.tuple.daddr != blog_p->tx.tuple.daddr && !(MAPT(blog_p)) && !(RX_GRE(blog_p) || TX_GRE(blog_p)))
    {
        return 0;
    }
#endif

    if (BROADSTREAM_IQOS_ENABLE() && FHW_IS_ANY_DEV_WAN(blog_p))
    {
        return 0;
    }

    /* Force disable WLAN multicast acceleration - local #define */
#if defined(BCM_DISABLE_WLAN_MCAST_HW_ACCELERATION)
    if (blog_p->rx.multicast && blog_p->tx.info.phyHdrType == BLOG_WLANPHY)
    {
        return 0;
    }
#endif

#if !(defined(CONFIG_BCM963138) || defined(CONFIG_BCM963148) || defined(CONFIG_BCM94908) || defined(CONFIG_BCM963158) || defined(CONFIG_BCM_XRDP) || defined(CONFIG_BCM_ARCHER) || defined(CONFIG_BCM_ARCHER_MODULE))
    /* No HW Accelerator support for MAPT */
    if ( MAPT(blog_p) )
    {
        return 0;
    }
#endif

    /* Any flow to/from WLAN should only be HW accelerated
       if "is_xx_hw_acc_en" is set for corresponding interface */
    if ( (BLOG_IS_TX_HWACC_ENABLED_WLAN_PHY(blog_p->tx.info.phyHdrType) && !blog_p->wl_hw_support.is_tx_hw_acc_en) ||
         (blog_p->rx.info.phyHdrType == BLOG_WLANPHY && !blog_p->wl_hw_support.is_rx_hw_acc_en) )
    {
        /* TODO : Hybrid possible when TX is not offloaded as well */
        return 0;
    }

    /* Local terminated TCP flows */
    if (blog_p->tx.info.phyHdrType == BLOG_TCP4_LOCALPHY) 
    {
        /* TODO : Hybrid possible; need adjustments in pktrunner */
        return 0;
    }

    /* WLAN EXTRA PHYs*/
    if ( BLOG_IS_HWACC_DISABLED_WLAN_EXTRAPHY(blog_p->rx.info.phyHdrType,blog_p->tx.info.phyHdrType)) 
    {
        return 0;
    }

    /* L2TP don't activate flow in HW */
    if ( TX_L2TP(blog_p) || RX_L2TP(blog_p) ) 
    {
        /* TODO : Hybrid possible when TX_L2TP and !RX_L2TP */
        return 0;
    }

    /* PPTP don't activate flow in HW */
    if( TX_PPTP(blog_p) || RX_PPTP(blog_p) )
    {
        /* TODO : Hybrid? What is PPTP? */
        return 0;
    }
    
    /* MAPE don't activate flow in HW */
    if(TX_IPV6_MAPE(blog_p))
    {
        return 0;
    }

    if( TX_ESP(blog_p) || RX_ESP(blog_p) )
    {
        /* if the packet is originated from or destined to a BLOG_SPU device,
           it's not an ESP passthru packet and therefore skip fcache in hw. */
        if ( (blog_p->rx.info.phyHdrType == BLOG_SPU_DS) ||
             (blog_p->rx.info.phyHdrType == BLOG_SPU_US) ||
             (blog_p->tx.info.phyHdrType == BLOG_SPU_DS) ||
             (blog_p->tx.info.phyHdrType == BLOG_SPU_US) )
        {
            /* TODO : Hybrid possible when TX_ESP and !RX_ESP */
            return 0;
        }
        else 
        {
#if defined(CONFIG_BCM_PON)
            return 1;
#else
            /* For DSL platforms, accelerate L2 IPSEC ESP pass-through flow only */
            if ((blog_p->rx.info.bmap.PLD_L2 == 1) && TX_ESP(blog_p) && RX_ESP(blog_p) )
                return 1;
            else
                return 0;
#endif
        }
    }
    /* HW acceleration is possible only for RX port directly connected to accelerators (FAP/Runner) */
    if ( blog_p->rx.info.bmap.BCM_SWC == 0   &&     /* Not a Switch/Runner port : ENET, GPON, EPON */
         blog_p->rx.info.bmap.BCM_XPHY == 0         /* Not a XTM Port */
#if defined(CONFIG_BCM_RDPA) || defined(CONFIG_BCM_RDPA_MODULE) /* Only Runner can receive WLAN packet directly */
         && (blog_p->rx.info.phyHdrType != BLOG_WLANPHY)        /* Dongle-Offload and WLAN-RX-ACCELERATION */
         && (blog_p->rx.info.phyHdrType != BLOG_NETXLPHY)        /* Dongle-Offload and WLAN-RX-ACCELERATION */
#endif
#if defined(CONFIG_BCM_ARCHER_WLAN)
         && (blog_p->rx.info.phyHdrType != BLOG_WLANPHY)        /* Archer WLAN-RX-ACCELERATION */
#endif
        )
    {
        return 0;
    }

    if ((blog_p->rx.info.phyHdrType == BLOG_SPDTST || blog_p->tx.info.phyHdrType == BLOG_SPDTST) && !(blog_p->spdtst & 1)) 
    {
        return 0;
    }

    /* Runner acceleration of ATM flows is not supported */
    if (fhw_is_xoa_flw(blog_p))
    {
        return 0;
    }

    /* HW acceleration of flows to USB not supported; RX already covered in above condition */
    if ( blog_p->tx.info.phyHdrType == BLOG_USBPHY )
    {
        /* TODO : Hybrid? What does USBPHY mean? */
        return 0;
    }

    /* HW acceleration of flows from/to LTE WAN not supported */
    if ( (blog_p->rx.info.phyHdrType == BLOG_LTEPHY) || (blog_p->tx.info.phyHdrType == BLOG_LTEPHY) )
    {
        /* TODO : TX_LTE and !RX_LTE */
        return 0;
    }

    if ( !blog_p->rx.multicast ) /* Unicast flows */
    {
        /* Ethernet LAN-to-LAN flows */
        if ( (blog_p->rx.info.phyHdrType == BLOG_ENETPHY && blog_p->tx.info.phyHdrType == BLOG_ENETPHY) && /* ENET-to-ENET */
             !FHW_IS_ANY_DEV_WAN(blog_p) )  /* None is WAN i.e. both are LAN */
        {

            if ( !fhw_is_L2L_supported() )
            {
                /* TODO: Need change in PKT_RUNNER */
                return 0;
            }
        }
    }
    else
    {
        if (blog_p->rtp_seq_chk)
        {
            /* TODO : Hybrid possible when multicast-hybrid gets supported */
            return 0;
        }

        if (TX_GRE(blog_p))
        {
            /* Multicast GRE Hardware acceleration not supported */
            return 0;
        }

        if ( FHW_IS_RX_DEV_WAN(blog_p) ) /* Multicast RX from WAN */
        {
            /* Multicast from WAN - check if HW supports internal & external switch port conflict */
            if ( fhw_chk_int_ext_sw_ports_conflict( blog_p ) )
            {
                return 0;
            }
        }
#if !(defined(CONFIG_BCM_ARL) || defined(CONFIG_BCM_ARL_MODULE))
        /* acceleration for LAN to LAN mutlicast requires ARL */
        else
        {
            return 0;
        }
#endif
    }
    /* GRE Flows -- check platform specific capabilities */
    if ( ((blog_p->rx.info.bmap.GRE == 1) || (blog_p->tx.info.bmap.GRE == 1)) && !fhw_get_gre_flow_support(blog_p) )
    {
        /* fhw_set_hybrid(out_hybrid_p); */
        /* TODO - Enable to test GRE-hybrid */
        return 0;
    }

    /* FAP: Don't accelerate non-TCP/UDP flows when Jumbo Frame support is enabled */
#if defined(CONFIG_BCM_JUMBO_FRAME) && !(defined(CONFIG_BCM_RDPA) || defined(CONFIG_BCM_RDPA_MODULE) || defined(CONFIG_BCM_ARCHER) || defined(CONFIG_BCM_ARCHER_MODULE))
    if (!((blog_p->key.protocol == BLOG_IPPROTO_TCP) || (blog_p->key.protocol == BLOG_IPPROTO_UDP)))
    {
        return 0;
    }
#endif

#if !defined(CONFIG_BCM963158)
    /* WAN to WAN flows */
    if ( FHW_IS_RX_DEV_WAN(blog_p) &&  FHW_IS_TX_DEV_WAN(blog_p) )
    {
        /* WAN to WAN hardware acceleration not supported on other platforms */
        if (blog_p->tx.info.phyHdrType != BLOG_SPDTST)
            return 0;
    }
#endif

    return 1;
}

static uint32_t fhw_flow_type(Blog_t *blog_p)
{
    BlogInfo_t * bInfo_p;
    uint32_t flow_type = HW_CAP_NONE;
    
    bInfo_p = &blog_p->rx.info;

    if ((bInfo_p->bmap.PLD_L2 == 1) && (blog_p->rx.multicast != 1))
        flow_type = HW_CAP_L2_UCAST;
    else if (blog_p->tupleV6.tunnel == 1)
        flow_type = HW_CAP_IPV6_TUNNEL;
    else if (bInfo_p->bmap.PLD_IPv4 == 1)
    {
        if (blog_p->rx.multicast == 1)
            flow_type = HW_CAP_IPV4_MCAST;
        else
            flow_type = HW_CAP_IPV4_UCAST;
    }
    else if (bInfo_p->bmap.PLD_IPv6 == 1)
    {
        if (blog_p->rx.multicast == 1)
            flow_type = HW_CAP_IPV6_MCAST;
        else
            flow_type = HW_CAP_IPV6_UCAST;
    }

    return flow_type;
}

/*
 *------------------------------------------------------------------------------
 * Function   : fhw_is_hw_cap_enabled
 * Description: Checks if all the HW capabilities in mask are enabled (subset)
 *              for the HW accelerators.
 * Return Val : 1 if the requested HW cap are subset of the HW Accelerators cap.
 *            : 0 otherwise.
 *------------------------------------------------------------------------------
 */
uint32_t fhw_is_hw_cap_enabled(uint32_t cap_mask)
{
    FhwHwAccPrio_t hwIx;

    if (!cap_mask)
        return 0;

    for (hwIx = FHW_PRIO_0; hwIx < FHW_PRIO_MAX; hwIx++) 
    {        
        if ((fhw.hwAcc[hwIx].hwHooks.cap & cap_mask) == cap_mask)
            return 1;
    }

    return 0;
}

/*
 *------------------------------------------------------------------------------
 * Function   : fhw_get_hw_accel_avail
 * Description: Get whether HW acceleration is enabled and hooks bound to fcache.
 * Return Val : 0(No), 1(Yes)
 *------------------------------------------------------------------------------
 */
int fhw_get_hw_accel_avail(void)
{
    return fhw.bindCount && g_fhw_hw_accel_enabled;
}

/*
 *------------------------------------------------------------------------------
 * Function   : fhw_set_hw_accel
 * Description: Enable/Disable acceleration of flows by HW.
 * Return Val : Always 1
 *------------------------------------------------------------------------------
 */
uint32_t fhw_set_hw_accel(uint32_t enable)
{
    g_fhw_hw_accel_enabled = enable ? 1 : 0;
    return 1;
}
/*
 *------------------------------------------------------------------------------
 * Function   : fhw_get_hw_accel
 * Description: Get current status of Enable/Disable acceleration of flows by HW.
 * Return Val : 0(Disable), 1(Enable)
 *------------------------------------------------------------------------------
 */
uint32_t fhw_get_hw_accel(void)
{
    return g_fhw_hw_accel_enabled;
}

static uint32_t _fhw_activate_hw(uint32_t flow_type, uint32_t flowIx, unsigned long blog,
                                 unsigned long new_flow, unsigned long key_in)
{
    FhwHwAccPrio_t hwIx, hwAccIx;
    Blog_t *blog_p = (Blog_t *) blog; 
    FhwKey_t fhwKey_in = *(FhwKey_t*)(&key_in);

    dbgl_print( DBG_EXTIF, "flowIx<%d> blog_p<0x%p>", flowIx, blog_p );

    for (hwIx = FHW_PRIO_0; hwIx < FHW_PRIO_MAX; hwIx++) 
    {        
        hwAccIx = fhw.cap2HwAccMap[flow_type][hwIx];
        if (hwAccIx >= FHW_PRIO_MAX)
            break;

        /*
         * HW accelerator which cannot accelerate mcast traffic 
         * returns 0xFFFFFFFF for mcast configuration
         */
        if ( likely(fhw.hwAcc[hwAccIx].hwHooks.activate_fn != (HOOKP32)NULL) )
        {
            uint32_t tuple = FHW_TUPLE_INVALID;
            uint32_t tuple_in = FHW_TUPLE_INVALID;
            int hwTblIdx = 0; /* signed int to catch the error code */

            if ( !(fhw.hwAcc[hwAccIx].hwHooks.cap & (1<<flow_type) ) )
                continue;

            /* First allocate an index to store the mapping table - only for new_flow
             * when multicast client is added, new_flow is not set */

            if (fhwKey_in.word == FHW_TUPLE_INVALID)
            {
                hwTblIdx = idx_pool_get_index(&fhw.hwAcc[hwAccIx].freeIdxPool);
                if (hwTblIdx < 0)
                {
                    /* No available HW flows available */
                    return FHW_TUPLE_INVALID;
                }
            }
            else
            {
                /* Reuse the already assigned hwTblIdx */
                hwTblIdx = fhwKey_in.id.fhw;
                if (!idx_pool_index_in_use(&fhw.hwAcc[hwAccIx].freeIdxPool, hwTblIdx))
                {
                    /* Invalid Fhw Key */
                    fc_error("fhw key Index %u not allocated, max %u\n",hwTblIdx,fhw.hwAcc[hwAccIx].freeIdxPool.pool_size);
                    return FHW_TUPLE_INVALID;
                }
                /* reuse the already allocated hw_key */
                tuple_in = fhw.hwAcc[hwAccIx].hwtbl[hwTblIdx].hw_key;
            }
            /* Do upcall into HW to activate a learnt connection */
            tuple = fhw.hwAcc[hwAccIx].hwHooks.activate_fn( (void*)blog_p, tuple_in );

            if ( tuple != FHW_TUPLE_INVALID )
            {
                FhwKey_t key = {};
                HwEnt_t *hwEnt_p;

                if (tuple_in != FHW_TUPLE_INVALID && tuple_in != tuple)
                {
                    fc_error("tuple_in <0x%08x> tuple <0x%08x> not same\n",tuple_in, tuple);
                }
                hwEnt_p = &fhw.hwAcc[hwAccIx].hwtbl[hwTblIdx];

                if (new_flow) 
                    _fhw_new_flow(flowIx, flow_type);
                /* hwEnt_p stores the actual HW-Flow-Key */
                hwEnt_p->hw_key = tuple;
                hwEnt_p->hw_hits_curr = 0;
                hwEnt_p->hw_hits_cumm = 0;
                hwEnt_p->hw_bytes_curr = 0;
                hwEnt_p->hw_bytes_cumm = 0;
                hwEnt_p->flow_type = flow_type;
                /* Build the FHW key for flow-cache flow */
                key.id.accl = hwAccIx;
                key.id.fhw = hwTblIdx;
                dbgl_print( DBG_EXTIF, "new flowIx<%d> fhw<%u:%u> hw<0x%8x>", flowIx, key.id.accl, key.id.fhw, tuple);
                return key.word;
            }
            /* Potential issue of index leak and dormant flow in HW
             * Upon return the flow-cache flow is going to overwrite the Fhw_Key as invalid 
             */
            if (tuple_in != FHW_TUPLE_INVALID)
            {
                fc_error("tuple_in <0x%08x> ; flow-modification failed\n",tuple_in);
            }
            if (fhwKey_in.word == FHW_TUPLE_INVALID)
            {
                /* HW Flow activation failed - return the index back to pool */
                idx_pool_return_index(&fhw.hwAcc[hwAccIx].freeIdxPool, hwTblIdx);
            }
        }
    }

    return FHW_TUPLE_INVALID;
}

/*
 *------------------------------------------------------------------------------
 * Function   : fhw_activate_hw
 * Description: Activates a flow in hardware HW via activate_fn.
 *------------------------------------------------------------------------------
 */
static uint32_t fhw_activate_hw(uint32_t flowIx, unsigned long blog, unsigned long new_flow,
                                unsigned long key_in)
{
    Blog_t *blog_p = (Blog_t *) blog; 
    uint32_t flow_type = fhw_flow_type(blog_p);

    dbgl_print( DBG_EXTIF, "flowIx<%d> blog_p<0x%p>", flowIx, blog_p );
    return _fhw_activate_hw(flow_type, flowIx, blog, new_flow, key_in);
}

/*
 *------------------------------------------------------------------------------
 * Function   : fhw_deactivate_hw
 * Description: Deactivates a flow in hardware HW via deactivate_fn.
 *------------------------------------------------------------------------------
 */
static int fhw_deactivate_hw(uint32_t flowIx, FhwKey_t key, unsigned long blog)
{
    FhwHwAccPrio_t hwAccIx = key.id.accl;
    struct blog_t * blog_p = (struct blog_t *) blog;
    int active = 0;

    dbgl_print( DBG_EXTIF, "flowIx<%d> fhw<0x%8x> blog_p<0x%p>", 
        flowIx, key.id.fhw, blog_p );

    if ( unlikely(key.word == FHW_TUPLE_INVALID) )
        return active;

    dbg_assertr( (hwAccIx < FHW_PRIO_MAX), 0 );

    if ( likely(fhw.hwAcc[hwAccIx].hwHooks.deactivate_fn != (HOOK4PARM)NULL) )
    {
        const uint32_t entIx = key.id.fhw;
        HwEnt_t *hwEnt_p = &fhw.hwAcc[hwAccIx].hwtbl[entIx]; 
        uint32_t prev_octets;
        uint32_t prev_hits;

        prev_octets = hwEnt_p->hw_bytes_curr;
        prev_hits = hwEnt_p->hw_hits_curr;

        /* Do upcall into HW to deactivate a learnt connection */
        /*
         * For HW multicast acceleration feature, hwHooks.deactivate_fn
         * must return the number of active ports remaining in HW entry.
         */
        dbgl_print( DBG_EXTIF, "remove flowIx<%d> fhw<%u:%u> hw<0x%8x>", flowIx, key.id.accl, key.id.fhw, hwEnt_p->hw_key);
        active = fhw.hwAcc[hwAccIx].hwHooks.deactivate_fn( 
                                                hwEnt_p->hw_key,
                                                (unsigned long)(&hwEnt_p->hw_hits_curr),
                                                (unsigned long)(&hwEnt_p->hw_bytes_curr),
                                                (unsigned long)blog_p );

        if ( !active )  /* Set HW id to invalid only if no active port in HW */
        {
            hwEnt_p->hw_hits_cumm += calcHwPktHitDiff(hwEnt_p->hw_hits_curr, prev_hits);
            hwEnt_p->hw_bytes_cumm += calcHwByteHitDiff(hwEnt_p->hw_bytes_curr, prev_octets);

            _fhw_update_cumm_hw_stats(flowIx, 
                                      hwEnt_p->hw_hits_cumm, hwEnt_p->hw_bytes_cumm);
            idx_pool_return_index(&fhw.hwAcc[hwAccIx].freeIdxPool, key.id.fhw);
            hwEnt_p->hw_key = FHW_TUPLE_INVALID;
        }
    }

    dbgl_print( DBG_EXTIF, "flowIx<%d> active<%d>", flowIx, active );
    return active;
}

/*
 *------------------------------------------------------------------------------
 * Function   : fhw_update_hw
 * Description: Updates a flow in hardware via update_fn.
 *------------------------------------------------------------------------------
 */
static int fhw_update_hw(uint32_t update, FhwKey_t key, unsigned long blog)
{
    FhwHwAccPrio_t hwAccIx = key.id.accl;

    dbgl_print( DBG_EXTIF, "update<%d> fhw<0x%8x> blog_p<0x%p>", 
                update, key.id.fhw, (struct blog_t *)blog );

    dbg_assertr( (key.word != FHW_TUPLE_INVALID), 0 );
    dbg_assertr( (hwAccIx < FHW_PRIO_MAX), 0 );

    if ( likely(fhw.hwAcc[hwAccIx].hwHooks.update_fn != (HOOK3PARM)NULL) )
    {
        /* Do upcall into HW to update a learnt connection */
        fhw.hwAcc[hwAccIx].hwHooks.update_fn((uint32_t)update, fhw.hwAcc[hwAccIx].hwtbl[key.id.fhw].hw_key, blog);
    }

    return FHW_SUCCESS;
}

/*
 *------------------------------------------------------------------------------
 * Function   : fhw_refresh_hw
 * Description: Determine the HW hits from previous invocation
 * Returns    : Number of hits in HW hardware.
 *------------------------------------------------------------------------------
 */
static uint32_t fhw_refresh_hw(FhwKey_t key, unsigned long hw_hits, 
                               unsigned long hw_bytes)
{
    FhwHwAccPrio_t hwAccIx = key.id.accl;
    dbg_assertr( (hwAccIx < FHW_PRIO_MAX), 0);

    if ( likely(fhw.hwAcc[hwAccIx].hwHooks.refresh_fn != (HOOK3PARM)NULL) )
    {
        uint32_t *hw_hits_p = (uint32_t *) hw_hits; 
        uint32_t *hw_bytes_p = (uint32_t *) hw_bytes; 
        uint32_t prev_hits;
        uint32_t prev_octets;
        uint32_t entIx = key.id.fhw;
        HwEnt_t *hwEnt_p = &fhw.hwAcc[hwAccIx].hwtbl[entIx]; 

        prev_hits = hwEnt_p->hw_hits_curr;
        prev_octets = hwEnt_p->hw_bytes_curr;

        /* Do upcall into HW to fetch hit count of a learnt connection */
        fhw.hwAcc[hwAccIx].hwHooks.refresh_fn(hwEnt_p->hw_key,
                                    (unsigned long)(&hwEnt_p->hw_hits_curr),
                                    (unsigned long)(&hwEnt_p->hw_bytes_curr) );

        *hw_hits_p = calcHwPktHitDiff(hwEnt_p->hw_hits_curr, prev_hits);
        *hw_bytes_p = calcHwByteHitDiff(hwEnt_p->hw_bytes_curr, prev_octets);
        hwEnt_p->hw_bytes_cumm += *hw_bytes_p;
        hwEnt_p->hw_hits_cumm += *hw_hits_p;
        return *hw_hits_p;
    }
    else
        return 0;
}

/*
 *------------------------------------------------------------------------------
 * Function   : fhw_refresh_pathstat_hw
 * Description: Determine the HW hits from previous invocation
 * Returns    : Number of hits in HW hardware.
 *------------------------------------------------------------------------------
 */
static void fhw_refresh_pathstat_hw(uint8_t pathstat_idx, uint32_t *hw_hits_p, 
    uint32_t *hw_bytes_p)
{
    *hw_hits_p = 0;
    *hw_bytes_p = 0;    

    if ( likely(fhw.hwAcc[FHW_PRIO_0].hwHooks.refresh_pathstat_fn != (HOOK3PARM)NULL) )
    {
        /* Do upcall into HW to fetch hit count of a learnt connection */
        fhw.hwAcc[FHW_PRIO_0].hwHooks.refresh_pathstat_fn( pathstat_idx, 
                               (uintptr_t)(hw_hits_p), (uintptr_t)(hw_bytes_p) );
    }
    return;
}

/*
 *------------------------------------------------------------------------------
 * Function   : fhw_get_path_num
 * Description: Report number of hw pathstat resource available in hwacc
 * Returns    : Number of pathstat counters in HW hardware.
 *------------------------------------------------------------------------------
 */
static uint32_t fhw_get_path_num(void)
{
    return fhw.hwAcc[FHW_PRIO_0].hwHooks.max_hw_pathstat;
}

/*
 *------------------------------------------------------------------------------
 * Function   : fhw_stats_hw
 * Description: Determine the cumulative HW hits
 * Returns    : Number of hits in HW hardware.
 *------------------------------------------------------------------------------
 */
uint32_t fhw_stats_hw(uint32_t flowIx, FhwKey_t key, unsigned long hw_hits_p, 
    unsigned long long *hw_bytes_p)
{
    ExtFlow_t *eflow_p = _efc_entry(flowIx);
    uint32_t hw_hits = 0;
    unsigned long long hw_bytes = 0;

    fhw_cur_stats_hw(key, &hw_hits, &hw_bytes);

    *(uint32_t *) hw_hits_p = eflow_p->cumm_hw_hits + hw_hits;
    *(unsigned long long *) hw_bytes_p = eflow_p->cumm_hw_bytes + hw_bytes;

    dbgl_print( DBG_EXTIF, "flowIx<%d> fhw<0x%8x> "
                           "tot_hw_hits<%u>, tot_hw_bytes<%u>", 
                           flowIx, key.id.fhw, 
                           *(uint32_t *)hw_hits_p, *(uint32_t *)hw_bytes_p);
    return (*(uint32_t *) hw_hits_p);
}


/*
 *------------------------------------------------------------------------------
 * Function   : fhw_reset_stats_hw
 * Description: Reset the stats of the flow
 * Returns    : 
 *------------------------------------------------------------------------------
 */
uint32_t fhw_reset_stats_hw(uint32_t flowIx, FhwKey_t key)
{
    FhwHwAccPrio_t hwAccIx = key.id.accl;
    ExtFlow_t *eflow_p = _efc_entry(flowIx);

    eflow_p->cumm_hw_hits = 0;
    eflow_p->cumm_hw_bytes = 0;

    if ( key.word == FHW_TUPLE_INVALID )     /* caller checks already */
        return 0;

    dbg_assertr( (hwAccIx < FHW_PRIO_MAX), 0);

    if ( likely(fhw.hwAcc[hwAccIx].hwHooks.reset_stats_fn != (HOOK32)NULL) )
    {
        uint32_t entIx = key.id.fhw;
        HwEnt_t *hwEnt_p = &fhw.hwAcc[hwAccIx].hwtbl[entIx]; 

        /* Do upcall into HW to fetch hit count of a learnt connection */
        fhw.hwAcc[hwAccIx].hwHooks.reset_stats_fn(hwEnt_p->hw_key);

        hwEnt_p->hw_hits_cumm = 0;
        hwEnt_p->hw_hits_curr = 0;
        hwEnt_p->hw_bytes_curr = 0;
        hwEnt_p->hw_bytes_cumm = 0;
    }

    return 0;
}


/*
 *------------------------------------------------------------------------------
 * Function   : fhw_clear_hw
 * Description: Clears a flow in hardware HW via hwHooks.clear_fn.
 *------------------------------------------------------------------------------
 */
static void fhw_clear_hw(FhwKey_t key)
{
    FhwHwAccPrio_t hwAccIx = key.id.accl;

    if ( key.word == FHW_TUPLE_INVALID )     /* caller checks already */
        return;

    dbg_assertv( (hwAccIx < FHW_PRIO_MAX) );

    if ( likely(fhw.hwAcc[hwAccIx].hwHooks.clear_fn != (HOOK32)NULL) )
    {
        /* Do upcall into HW to clear an evicted connection */
        fhw.hwAcc[hwAccIx].hwHooks.clear_fn(fhw.hwAcc[hwAccIx].hwtbl[key.id.fhw].hw_key);
    }
    return;
}

/*
 *------------------------------------------------------------------------------
 * Function   : fhw_get_hw_host_mac_mgmt_avail
 * Description: Get whether HW host MAC managment hooks bound to fcache.
 * Return Val : 0(No), 1(Yes)
 *------------------------------------------------------------------------------
 */
int fhw_get_hw_host_mac_mgmt_avail(void)
{
    return fhw.bindCount && fhw.hwAcc[FHW_PRIO_0].hwHooks.add_host_mac_fn != (HOOKP)NULL &&
        fhw.hwAcc[FHW_PRIO_0].hwHooks.del_host_mac_fn != (HOOKP)NULL;
}

/*
 *------------------------------------------------------------------------------
 * Function   : fhw_add_host_mac_hw
 * Description: Adds a host MAC address in hardware HW via add_host_mac_fn.
 * Design Note  : Assuming hook is bound to first HW accelerator.
 *------------------------------------------------------------------------------
 */
static int fhw_add_host_mac_hw(char *mac_p)
{
    /* Do upcall into HW to add host MAC address */
    if ( fhw.hwAcc[FHW_PRIO_0].hwHooks.add_host_mac_fn == (HOOKP)NULL ||
         fhw.hwAcc[FHW_PRIO_0].hwHooks.add_host_mac_fn( mac_p ) != 0 )
        return FCACHE_ERROR;

    return FCACHE_SUCCESS;
}

/*
 *------------------------------------------------------------------------------
 * Function   : fhw_del_host_mac_hw
 * Description: Deletes a host MAC address in hardware HW via del_host_mac_fn.
 * Design Note  : Assuming hook is bound to first HW accelerator.
 *------------------------------------------------------------------------------
 */
static int fhw_del_host_mac_hw(char *mac_p)
{
    /* Do upcall into HW to delete host MAC address */
    if ( fhw.hwAcc[FHW_PRIO_0].hwHooks.del_host_mac_fn == (HOOKP)NULL ||
         fhw.hwAcc[FHW_PRIO_0].hwHooks.del_host_mac_fn( mac_p ) != 0 )
        return FCACHE_ERROR;

    return FCACHE_SUCCESS;
}

/*
 *------------------------------------------------------------------------------
 * Function   : fhw_get_hw_tuple
 * Description: returns the hw_tuple corresponding to the Fhw_Key.
 *------------------------------------------------------------------------------
 */
static uint32_t fhw_get_hw_tuple(FhwKey_t key)
{
    FhwHwAccPrio_t hwAccIx = key.id.accl;

    if ( unlikely(key.word == FHW_TUPLE_INVALID) )     /* caller checks already */
        return FHW_TUPLE_INVALID;

    dbg_assertr( (hwAccIx < FHW_PRIO_MAX) , FHW_TUPLE_INVALID );

    if ( likely(fhw.status) )
    {
        /* Do upcall into HW to clear an evicted connection */
        return fhw.hwAcc[hwAccIx].hwtbl[key.id.fhw].hw_key;
    }
    return FHW_TUPLE_INVALID;
}


/*
 *------------------------------------------------------------------------------
 * Function   : fhw_mcast_whitelist_add_hw
 * Description: Add a mcast_whiteliset entry in hardware HW via activate_fn.
 *------------------------------------------------------------------------------
 */
static uint32_t fhw_mcast_whitelist_add_hw(uint32_t flowIx, unsigned long blog,
                                           unsigned long new_flow,
                                           unsigned long key_in)
{
    Blog_t *blog_p = (Blog_t *) blog; 
    uint32_t flow_type = fhw_flow_type(blog_p);

    if ((flow_type == HW_CAP_IPV4_MCAST) || (flow_type == HW_CAP_IPV6_MCAST))
        flow_type = HW_CAP_MCAST_WHITELIST;
    else
        return FHW_TUPLE_INVALID;

    dbgl_print( DBG_EXTIF, "flowIx<%d> blog_p<0x%p>", flowIx, blog_p );
    return _fhw_activate_hw(flow_type, flowIx, blog, new_flow, key_in);
}

/*
 *------------------------------------------------------------------------------
 * Function     : fhw_bind_fc
 * Description  : Permits manual enabling|disabling of bloging for FlowCache.
 * Parameter    :
 *                To enable:    enable_flag > 0
 *                To disable:   enable_flag = 0
 *------------------------------------------------------------------------------
 */
void fhw_bind_fc(int enable_flag)
{
    FcBindFhwHooks_t fhw_hooks, *fhw_hooks_p = &fhw_hooks;

    if ( enable_flag )
    {
        /* Bind HWACC handlers to fc*/
        fhw_hooks_p->activate_fn       = (HOOK4PARM) fhw_activate_hw;
        fhw_hooks_p->deactivate_fn     = (HOOK3PARM) fhw_deactivate_hw;
        fhw_hooks_p->update_fn         = (HOOK3PARM) fhw_update_hw;
        fhw_hooks_p->refresh_fn        = (HOOK3PARM) fhw_refresh_hw; 
        fhw_hooks_p->refresh_pathstat_fn  = (HOOK3PARM) fhw_refresh_pathstat_hw;
        fhw_hooks_p->reset_stats_fn    = (HOOK2PARM) fhw_reset_stats_hw; 
        fhw_hooks_p->add_host_mac_fn   = (HOOKP) fhw_add_host_mac_hw;
        fhw_hooks_p->del_host_mac_fn   = (HOOKP) fhw_del_host_mac_hw;
        fhw_hooks_p->clear_fn          = (HOOK32) fhw_clear_hw;
        fhw_hooks_p->fc_clear_fn       = &fhw_fc_clear_hook;
        fhw_hooks_p->stats_fn          = (HOOK4PARM) fhw_stats_hw;
        fhw_hooks_p->hwsupport_fn      = (HOOKP2)fhw_chk_support_hw;
        fhw_hooks_p->hybrid_hwsupport_fn      = (HOOKP)fhw_chk_support_hw_hybrid;
        fhw_hooks_p->mcast_dflt_fn     = (HOOKV)fhw_mcast_dflt_hw;
        fhw_hooks_p->get_path_num_fn   = (HOOKV)fhw_get_path_num; 
        fhw_hooks_p->get_hw_tuple_fn   = (HOOK32)fhw_get_hw_tuple; 
        fhw_hooks_p->mcast_whitelist_add_fn = (HOOK4PARM)fhw_mcast_whitelist_add_hw;
        fhw_hooks_p->mcast_whitelist_rem_fn = (HOOK3PARM)fhw_deactivate_hw;
        fc_bind_fhw( fhw_hooks_p );

        printk( CLRbold PKTFLOW_MODNAME "HW acceleration enabled." CLRnl );
    }
    else
    {
        fhw_hooks_p->activate_fn       = (HOOK4PARM) NULL;
        fhw_hooks_p->deactivate_fn     = (HOOK3PARM) NULL;
        fhw_hooks_p->update_fn         = (HOOK3PARM) NULL;
        fhw_hooks_p->refresh_fn        = (HOOK3PARM) NULL;
        fhw_hooks_p->refresh_pathstat_fn  = (HOOK3PARM) NULL; 
        fhw_hooks_p->reset_stats_fn    = (HOOK2PARM) NULL; 
        fhw_hooks_p->add_host_mac_fn   = (HOOKP) NULL;
        fhw_hooks_p->del_host_mac_fn   = (HOOKP) NULL;
        fhw_hooks_p->clear_fn          = (HOOK32) NULL;
        fhw_hooks_p->fc_clear_fn       = &fhw_fc_clear_hook;
        fhw_hooks_p->stats_fn          = (HOOK4PARM) NULL;
        fhw_hooks_p->hwsupport_fn      = (HOOKP2)fhw_chk_support_hw;
        fhw_hooks_p->hybrid_hwsupport_fn      = (HOOKP)fhw_chk_support_hw_hybrid;
        fhw_hooks_p->mcast_dflt_fn     = (HOOKV)NULL;
        fhw_hooks_p->get_path_num_fn   = (HOOKV)NULL;
        fhw_hooks_p->get_hw_tuple_fn   = (HOOK32)NULL; 
        fhw_hooks_p->mcast_whitelist_add_fn = (HOOK4PARM)NULL;
        fhw_hooks_p->mcast_whitelist_rem_fn = (HOOK3PARM)NULL;

        /* Unbind hooks with fc */
        fc_bind_fhw( fhw_hooks_p );
        printk( CLRbold2 PKTFLOW_MODNAME "HW acceleration disabled." CLRnl );
    }

    fhw.status = enable_flag;
}

static int fhw_clear( uint32_t key, uint32_t scope )
{
    if (fhw_fc_clear_hook != (FC_CLEAR_HOOK)NULL)
            fhw_fc_clear_hook(key, scope);

    return 0;
}


/*
 *------------------------------------------------------------------------------
 * Function     : fhw_alloc_fctbl
 * Description  : Allocates Ext Fcache table 
 *------------------------------------------------------------------------------
 */
static int fhw_alloc_fctbl( void )
{
    int max_ent;
    ExtFlow_t *fent_p = NULL;

    max_ent = fcache_max_ent();
    dbgl_print( DBG_EXTIF, "max_ent<%d>", max_ent); 

    /* one extra entry to cache line align (16B) */
    fent_p = (ExtFlow_t *) 
        ( ( (uintptr_t) kmalloc( (sizeof(ExtFlow_t) * (max_ent+1) ),
        GFP_ATOMIC) ) & ~0x0F);

    if (fent_p == NULL)
    {
        fc_error("Out of memory for Ext Fcache Table" );   
        return FHW_ERROR;
    }
    memset( (void*)fent_p, 0, (sizeof(ExtFlow_t) * max_ent) );
    fhw.fctbl = fent_p; 

    return FHW_SUCCESS;
}

/*
 *------------------------------------------------------------------------------
 * Function     : fhw_set_cap2HwAcc
 * Description  : Sets the cap to HwAccIx Map
 *                If the default cap to HW Accelerator mapping is not
 *                acceptable, the new mappings can be set here.
 *------------------------------------------------------------------------------
 */
static void fhw_set_cap2HwAcc(HwCap_t cap, 
        FhwHwAccPrio_t hwAcc0, FhwHwAccPrio_t hwAcc1)
{
    dbgl_print( DBG_EXTIF, "cap<%d> hwAcc0<%d> hwAcc1<%d>", 
                cap, hwAcc0, hwAcc1 ); 

    if ( (hwAcc0 >= FHW_PRIO_MAX) || (hwAcc1 >= FHW_PRIO_MAX) )
        fc_error("out of range hwAccIx" );   

    fhw.cap2HwAccMap[cap][0] = hwAcc0;
    fhw.cap2HwAccMap[cap][1] = hwAcc1;
}

/*
 *------------------------------------------------------------------------------
 * Function     : fhw_init_cap2HwAccMap
 * Description  : Initializes the default cap to HwAccIx Map
 *                By default, all the cap (flow types) will be checked for
 *                the first accelerator and then second in case of multiple
 *                HW accelerators.
 *------------------------------------------------------------------------------
 */
static void fhw_init_cap2HwAccMap( void )
{
    HwCap_t cap;

    for (cap = HW_CAP_NONE; cap < HW_CAP_MAX; cap++)
    {
        fhw_set_cap2HwAcc(cap, FHW_PRIO_0, FHW_PRIO_0);
        fhw_set_cap2HwAcc(cap, FHW_PRIO_0, FHW_PRIO_1);
    }
}



/*
 *------------------------------------------------------------------------------
 * Function     : fhw_construct
 * Description  : Construction of HW accelerator subsystem.
 * Design Note  : 
 *------------------------------------------------------------------------------
 */
int fhw_construct(void)
{
    dbg_config( CC_CONFIG_FHW_DBGLVL );
    dbgl_func( DBG_BASIC );

    memset( (void*)&fhw, 0, sizeof(Fhw_t) );
    fhw_alloc_fctbl();
    fhw_init_cap2HwAccMap();

    printk( CLRbold "Initialized Fcache HW accelerator layer state" CLRnl );
    return 0;
}

/*
 *------------------------------------------------------------------------------
 * Function     : fhw_destruct
 * Description  : Destruction of flow cache subsystem.
 * Design Note  : Invoked by pktflow_destruct() in fcachedrv.c
 *------------------------------------------------------------------------------
 */
void fhw_destruct(void)
{
    dbgl_func( DBG_BASIC );

    fhw_bind_fc( 0 );

    printk( CLRbold2 PKTFLOW_MODNAME "Reset HW acceleration state" CLRnl );
}

EXPORT_SYMBOL(fhw_bind_hw);

