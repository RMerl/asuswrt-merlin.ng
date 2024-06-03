/*
    <:copyright-BRCM:2013:DUAL/GPL:standard

       Copyright (c) 2013 Broadcom
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
