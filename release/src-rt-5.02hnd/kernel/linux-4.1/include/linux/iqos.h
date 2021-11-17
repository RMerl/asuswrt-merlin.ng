#if defined(CONFIG_BCM_KF_NBUFF)
#ifndef __IQOS_H_INCLUDED__
#define __IQOS_H_INCLUDED__

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
 * File Name : ingqos.h
 *
 *******************************************************************************
 */
#define IQOS_VERSION             "v0.1"
#define IQOS_VER_STR             IQOS_VERSION
#define IQOS_MODNAME             "Broadcom IQoS "

#define IQOS_ERROR               (-1)
#define IQOS_SUCCESS             0

typedef enum {
    IQOS_IPPROTO_TCP = 6,
    IQOS_IPPROTO_UDP = 17,
    IQOS_IPPROTO_MAX
} iqos_ipproto_t;

typedef enum {
    IQOS_ENT_DYN,
    IQOS_ENT_STAT,
    IQOS_ENT_MAX
} iqos_ent_t;

typedef enum {
    IQOS_PRIO_LOW,
    IQOS_PRIO_HIGH,
    IQOS_PRIO_MAX
} iqos_prio_t;

typedef enum {
    IQOS_CONG_STATUS_LO,
    IQOS_CONG_STATUS_HI,
    IQOS_CONG_STATUS_MAX
} iqos_cong_status_t;

typedef enum {
    IQOS_STATUS_DISABLE,
    IQOS_STATUS_ENABLE,
    IQOS_STATUS_MAX
} iqos_status_t;



#define IQOS_INVALID_NEXT_IX      0
#define IQOS_INVALID_PORT         0

typedef uint8_t (* iqos_add_L4port_hook_t)( iqos_ipproto_t ipProto, 
        uint16_t destPort, iqos_ent_t ent, iqos_prio_t prio );

typedef uint8_t (* iqos_rem_L4port_hook_t)( iqos_ipproto_t ipProto, 
        uint16_t destPort, iqos_ent_t ent );

typedef int (* iqos_prio_L4port_hook_t)( iqos_ipproto_t ipProto, 
        uint16_t destPort );


uint8_t iqos_add_L4port( iqos_ipproto_t ipProto, uint16_t destPort, 
        iqos_ent_t ent, iqos_prio_t prio );

uint8_t iqos_rem_L4port( iqos_ipproto_t ipProto, uint16_t destPort, 
        iqos_ent_t ent );

int iqos_prio_L4port( iqos_ipproto_t ipProto, uint16_t destPort );

void iqos_bind( iqos_add_L4port_hook_t  iqos_add, 
    iqos_rem_L4port_hook_t  iqos_rem, iqos_prio_L4port_hook_t iqos_prio );


#if defined(CONFIG_SMP) || defined(CONFIG_PREEMPT)
#define IQOS_LOCK_IRQSAVE()         spin_lock_irqsave( &iqos_lock_g, flags )
#define IQOS_UNLOCK_IRQRESTORE()   spin_unlock_irqrestore( &iqos_lock_g, flags )
#define IQOS_LOCK_BH()              spin_lock_bh( &iqos_lock_g )
#define IQOS_UNLOCK_BH()            spin_unlock_bh( &iqos_lock_g )
#else
#define IQOS_LOCK_IRQSAVE()         local_irq_save( flags )
#define IQOS_UNLOCK_IRQRESTORE()    local_irq_restore( flags )
#define IQOS_LOCK_BH()              NULL_STMT
#define IQOS_UNLOCK_BH()            NULL_STMT
#endif

#if (defined(CONFIG_BCM_INGQOS) || defined(CONFIG_BCM_INGQOS_MODULE))
#define IQOS_RXCHNL_MAX              4
#define IQOS_RXCHNL_DISABLED         0
#define IQOS_RXCHNL_ENABLED          1
#define IQOS_MAX_RX_RING_SIZE        4096

typedef enum {
    IQOS_IF_ENET,
    IQOS_IF_ENET_RXCHNL0 = IQOS_IF_ENET,
    IQOS_IF_ENET_RXCHNL1,
    IQOS_IF_ENET_RXCHNL2,
    IQOS_IF_ENET_RXCHNL3,
    IQOS_IF_XTM,
    IQOS_IF_XTM_RXCHNL0 = IQOS_IF_XTM,
    IQOS_IF_XTM_RXCHNL1,
    IQOS_IF_XTM_RXCHNL2,
    IQOS_IF_XTM_RXCHNL3,
    IQOS_IF_FWD,
    IQOS_IF_FWD_RXCHNL0 = IQOS_IF_FWD,
    IQOS_IF_FWD_RXCHNL1,
    IQOS_IF_WL,
    IQOS_IF_USB,
    IQOS_IF_MAX,
} iqos_if_t;

typedef void (* iqos_status_hook_t)(void);

#if defined(CONFIG_BCM_KF_FAP) && (defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE))
typedef uint32_t (* iqos_fap_ethRxDqmQueue_hook_t)(uint32_t chnl);
#if defined(CONFIG_BCM_XTMCFG) || defined(CONFIG_BCM_XTMCFG_MODULE)
typedef uint32_t (* iqos_fap_xtmRxDqmQueue_hook_t)(uint32_t chnl);
#endif
typedef void (* iqos_fap_set_status_hook_t)(int);

typedef void (* iqos_fap_add_L4port_hook_t)( uint8_t ipProto, uint16_t dport, 
            uint8_t ent, uint8_t prio );
typedef void (* iqos_fap_rem_L4port_hook_t)( uint8_t ipProto, uint16_t dport,
            uint8_t ent );
typedef void (* iqos_fap_dump_porttbl_hook_t)( uint8_t ipProto );
#endif

iqos_cong_status_t iqos_get_sys_cong_status( void );
iqos_cong_status_t iqos_get_cong_status( iqos_if_t iface, uint32_t chnl );
uint32_t iqos_set_cong_status( iqos_if_t iface, uint32_t chnl, 
                iqos_cong_status_t status );
#endif

#endif  /* defined(__IQOS_H_INCLUDED__) */
#endif
