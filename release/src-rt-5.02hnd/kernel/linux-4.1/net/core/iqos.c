#if defined(CONFIG_BCM_KF_NBUFF)
/*
<:copyright-BRCM:2009:DUAL/GPL:standard

   Copyright (c) 2009 Broadcom 
   All Rights Reserved

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License, version 2, as published by
the Free Software Foundation (the "GPL").

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.


A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.

:>
*/


/*
 *******************************************************************************
 * File Name  : iqos.c
 *******************************************************************************
 */
#include <linux/version.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/workqueue.h>
#include <linux/iqos.h>
#include <linux/bcm_colors.h>
#include <linux/ip.h>
#include <linux/in.h>
#include <linux/tcp.h>
#include <linux/udp.h>
#include <net/ip.h>
#if defined(CONFIG_BCM_KF_LOG)
#include <linux/bcm_log.h>
#endif


/* Debug macros */
#if defined(CC_IQOS_SUPPORT_DEBUG)
#define iqos_print(fmt, arg...)                                         \
    if ( iqos_debug_g )                                                     \
    printk( CLRc "IQOS %s :" fmt CLRnl, __FUNCTION__, ##arg )
#define iqos_assertv(cond)                                              \
    if ( !cond ) {                                                      \
        printk( CLRerr "IQOS ASSERT %s : " #cond CLRnl, __FUNCTION__ ); \
        return;                                                         \
    }
#define iqos_assertr(cond, rtn)                                         \
    if ( !cond ) {                                                      \
        printk( CLRerr "IQOS ASSERT %s : " #cond CLRnl, __FUNCTION__ ); \
        return rtn;                                                     \
    }
#define IQOS_DBG(debug_code)    do { debug_code } while(0)
#else
#ifndef NULL_STMT
#define NULL_STMT		do { /* NULL BODY */ } while (0)
#endif
#define iqos_print(fmt, arg...) NULL_STMT
#define iqos_assertv(cond)      NULL_STMT
#define iqos_assertr(cond, rtn) NULL_STMT
#define IQOS_DBG(debug_code)    NULL_STMT
#endif

#define iqos_error(fmt, arg...)                                         \
    printk( CLRerr "IQOS ERROR %s :" fmt CLRnl, __FUNCTION__, ##arg)

#undef  IQOS_DECL
#define IQOS_DECL(x)        #x,         /* string declaration */


/*--- globals ---*/
uint32_t iqos_enable_g = 0;      /* Enable Ingress QoS feature */
uint32_t iqos_cpu_cong_g = 0;
uint32_t iqos_debug_g = 0;

DEFINE_SPINLOCK(iqos_lock_g);
EXPORT_SYMBOL(iqos_lock_g);


/*
 *------------------------------------------------------------------------------
 * Default Ingress QoS hooks.
 *------------------------------------------------------------------------------
 */
static iqos_add_L4port_hook_t iqos_add_L4port_hook_g = 
                                            (iqos_add_L4port_hook_t) NULL;
static iqos_rem_L4port_hook_t iqos_rem_L4port_hook_g =
                                            (iqos_rem_L4port_hook_t) NULL;
static iqos_prio_L4port_hook_t iqos_prio_L4port_hook_g =
                                            (iqos_prio_L4port_hook_t) NULL;


/*
 *------------------------------------------------------------------------------
 * Get the Ingress QoS priority for L4 Dest port (layer4 UDP or TCP)
 *------------------------------------------------------------------------------
 */
int iqos_prio_L4port( iqos_ipproto_t ipProto, uint16_t destPort )
{
    unsigned long flags;
    uint8_t prio = IQOS_PRIO_LOW;

    if ( unlikely(iqos_prio_L4port_hook_g == (iqos_prio_L4port_hook_t)NULL) )
    {
         if ((ipProto != IQOS_IPPROTO_UDP) && (ipProto != IQOS_IPPROTO_TCP))
            prio = IQOS_PRIO_HIGH;
         goto iqos_prio_L4port_exit;
    }

    IQOS_LOCK_IRQSAVE();
    prio = iqos_prio_L4port_hook_g( ipProto, destPort ); 
    IQOS_UNLOCK_IRQRESTORE();

iqos_prio_L4port_exit:
    return prio;
}


/*
 *------------------------------------------------------------------------------
 * Add the Ingress QoS priority, and type of entry for L4 Dest port. 
 *------------------------------------------------------------------------------
 */
uint8_t iqos_add_L4port( iqos_ipproto_t ipProto, uint16_t destPort, 
        iqos_ent_t ent, iqos_prio_t prio )
{
    unsigned long flags;
    uint8_t addIx = IQOS_INVALID_NEXT_IX;

#if defined(CONFIG_BCM_KF_LOG)
    BCM_LOG_DEBUG( BCM_LOG_ID_IQ, 
            "AddPort ent<%d> ipProto<%d> dport<%d> prio<%d> ", 
            ent, ipProto, destPort, prio );  
#endif

    if ( unlikely(iqos_add_L4port_hook_g == (iqos_add_L4port_hook_t)NULL) )
        goto iqos_add_L4port_exit;

    IQOS_LOCK_IRQSAVE();
    addIx = iqos_add_L4port_hook_g( ipProto, destPort, ent, prio ); 
    IQOS_UNLOCK_IRQRESTORE();

iqos_add_L4port_exit:
#if defined(CONFIG_BCM_KF_LOG)
    BCM_LOG_DEBUG( BCM_LOG_ID_IQ, "addIx<%d>", addIx );  
#endif
    return addIx;
}


/*
 *------------------------------------------------------------------------------
 * Remove the L4 dest port from the Ingress QoS priority table
 *------------------------------------------------------------------------------
 */
uint8_t iqos_rem_L4port( iqos_ipproto_t ipProto, uint16_t destPort, 
        iqos_ent_t ent )
{
    unsigned long flags;
    uint8_t remIx = IQOS_INVALID_NEXT_IX;

#if defined(CONFIG_BCM_KF_LOG)
    BCM_LOG_DEBUG( BCM_LOG_ID_IQ, "RemPort ent<%d> ipProto<%d> dport<%d> ", 
                ent, ipProto, destPort);  
#endif

    if ( unlikely(iqos_rem_L4port_hook_g == (iqos_rem_L4port_hook_t)NULL) )
        goto iqos_rem_L4port_exit;

    IQOS_LOCK_IRQSAVE();
    remIx = iqos_rem_L4port_hook_g( ipProto, destPort, ent ); 
    IQOS_UNLOCK_IRQRESTORE();

iqos_rem_L4port_exit:
#if defined(CONFIG_BCM_KF_LOG)
    BCM_LOG_DEBUG( BCM_LOG_ID_IQ, "remIx<%d> ", remIx);  
#endif
    return remIx;
}


/*
 *------------------------------------------------------------------------------
 * Function     : iqos_bind
 * Description  : Override default hooks.
 *  iqos_add    : Function pointer to be invoked in iqos_add_L4port
 *  iqos_rem    : Function pointer to be invoked in iqos_rem_L4port
 *  iqos_prio   : Function pointer to be invoked in iqos_prio_L4port
 *------------------------------------------------------------------------------
 */
void iqos_bind( iqos_add_L4port_hook_t  iqos_add, 
                iqos_rem_L4port_hook_t  iqos_rem,
                iqos_prio_L4port_hook_t iqos_prio )
{
    iqos_print( "Bind add[<%08x>] rem[<%08x>] prio[<%08x>]", 
                (int)iqos_add, (int)iqos_rem, (int)iqos_prio );

    iqos_add_L4port_hook_g = iqos_add;
    iqos_rem_L4port_hook_g = iqos_rem;
    iqos_prio_L4port_hook_g = iqos_prio;
}

EXPORT_SYMBOL(iqos_cpu_cong_g);
EXPORT_SYMBOL(iqos_enable_g);
EXPORT_SYMBOL(iqos_debug_g);
EXPORT_SYMBOL(iqos_add_L4port);
EXPORT_SYMBOL(iqos_rem_L4port);
EXPORT_SYMBOL(iqos_prio_L4port);
EXPORT_SYMBOL(iqos_bind);

/*
 *------------------------------------------------------------------------------
 * Function     : __init_iqos
 * Description  : Static construction of ingress QoS subsystem.
 *------------------------------------------------------------------------------
 */
static int __init __init_iqos( void )
{
    printk( IQOS_MODNAME IQOS_VER_STR " initialized\n" );
    return 0;
}

subsys_initcall(__init_iqos);

#if (defined(CONFIG_BCM_INGQOS) || defined(CONFIG_BCM_INGQOS_MODULE))


/* Hooks for getting/dumping the Ingress QoS status */
iqos_status_hook_t iqos_enet_status_hook_g = (iqos_status_hook_t)NULL;

#if defined(CONFIG_BCM_KF_FAP) && (defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE))
iqos_status_hook_t iqos_fap_status_hook_g = (iqos_status_hook_t)NULL;
#endif

#if defined(CONFIG_BCM_XTMCFG) || defined(CONFIG_BCM_XTMCFG_MODULE)
iqos_status_hook_t iqos_xtm_status_hook_g = (iqos_status_hook_t)NULL;
#endif


#if defined(CONFIG_BCM_KF_FAP) && (defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE))
/* Hooks for getting the current RX DQM queue depth */
iqos_fap_ethRxDqmQueue_hook_t iqos_fap_ethRxDqmQueue_hook_g = NULL;
#if defined(CONFIG_BCM_XTMCFG) || defined(CONFIG_BCM_XTMCFG_MODULE)
iqos_fap_xtmRxDqmQueue_hook_t iqos_fap_xtmRxDqmQueue_hook_g = NULL;
#endif

iqos_fap_set_status_hook_t    iqos_fap_set_status_hook_g = NULL;
iqos_fap_add_L4port_hook_t    iqos_fap_add_L4port_hook_g = NULL;
iqos_fap_rem_L4port_hook_t    iqos_fap_rem_L4port_hook_g = NULL;
iqos_fap_dump_porttbl_hook_t  iqos_fap_dump_porttbl_hook_g = NULL;
#endif


/* get the congestion status for system */ 
iqos_cong_status_t iqos_get_sys_cong_status( void )
{
    return  ((iqos_cpu_cong_g) ? IQOS_CONG_STATUS_HI : IQOS_CONG_STATUS_LO);
}


/* get the congestion status for an RX channel of an interface */ 
iqos_cong_status_t iqos_get_cong_status( iqos_if_t iface, uint32_t chnl )
{
    return ((iqos_cpu_cong_g & (1<<(iface + chnl))) ? 
                                IQOS_CONG_STATUS_HI : IQOS_CONG_STATUS_LO);
}


/* set/reset the congestion status for an RX channel of an interface */ 
uint32_t  iqos_set_cong_status( iqos_if_t iface, uint32_t chnl, 
        iqos_cong_status_t status )
{
    unsigned long flags;

    IQOS_LOCK_IRQSAVE();

    if (status == IQOS_CONG_STATUS_HI)
        iqos_cpu_cong_g |= (1<<(iface + chnl));
    else
        iqos_cpu_cong_g &= ~(1<<(iface + chnl));

    IQOS_UNLOCK_IRQRESTORE();

    return iqos_cpu_cong_g;
}

EXPORT_SYMBOL(iqos_get_cong_status);
EXPORT_SYMBOL(iqos_set_cong_status);
EXPORT_SYMBOL(iqos_enet_status_hook_g);

#if defined(CONFIG_BCM_KF_FAP) && (defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE))
EXPORT_SYMBOL(iqos_fap_status_hook_g);
EXPORT_SYMBOL(iqos_fap_set_status_hook_g);
#if defined(CONFIG_BCM_XTMCFG) || defined(CONFIG_BCM_XTMCFG_MODULE)
EXPORT_SYMBOL(iqos_fap_xtmRxDqmQueue_hook_g);
#endif
EXPORT_SYMBOL(iqos_fap_ethRxDqmQueue_hook_g);
EXPORT_SYMBOL(iqos_fap_add_L4port_hook_g);
EXPORT_SYMBOL(iqos_fap_rem_L4port_hook_g);
EXPORT_SYMBOL(iqos_fap_dump_porttbl_hook_g);
#endif

#if defined(CONFIG_BCM_XTMCFG) || defined(CONFIG_BCM_XTMCFG_MODULE)
EXPORT_SYMBOL(iqos_xtm_status_hook_g);
#endif

#endif /* (defined(CONFIG_BCM_INGQOS) || defined(CONFIG_BCM_INGQOS_MODULE)) */

#endif

