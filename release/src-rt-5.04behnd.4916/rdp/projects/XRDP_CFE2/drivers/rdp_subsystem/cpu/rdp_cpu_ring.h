/*
    <:copyright-BRCM:2013:DUAL/GPL:standard
    
       Copyright (c) 2013 Broadcom 
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

/******************************************************************************/
/*                                                                            */
/* File Description:                                                          */
/*                                                                            */
/* This file contains the implementation of the Runner CPU ring interface     */
/*                                                                            */
/******************************************************************************/

#ifndef _RDP_CPU_RING_H_
#define _RDP_CPU_RING_H_

#include <bcm_mm.h>
#include "rdd.h"
#include "rdp_cpu_ring_defs.h"

typedef struct
{
   uint8_t* data_ptr;
   uint8_t data_offset;
   uint16_t packet_size;
   uint16_t flow_id;
   uint16_t reason;
   uint16_t src_bridge_port;
   uint16_t dst_ssid;
   uint32_t wl_metadata;
   uint16_t ptp_index;
   uint16_t free_index;
   uint8_t  is_rx_offload;
   uint8_t  is_ipsec_upstream;
   uint8_t  is_ucast;
   uint8_t  is_exception;
   uint8_t  is_csum_verified;
#ifdef XRDP
   uint8_t  mcast_tx_prio;
#endif
#ifdef CONFIG_CPU_REDIRECT_MODE_SUPPORT
   uint8_t  cpu_redirect_egress_queue;
   uint8_t  cpu_redirect_wan_flow;
#endif
}
CPU_RX_PARAMS;


int rdp_cpu_ring_read_packet_copy(uint32_t ringId, CPU_RX_PARAMS* rxParams);


#endif /* _RDP_CPU_RING_H_ */
