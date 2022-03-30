// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2013 Broadcom
 */
/*
    
*/

/**************************************************************************/
/*									  */
/* File Description:							  */
/*									  */
/* This file contains the implementation of the Runner CPU ring interface */
/*									  */
/******************************************************************************/

#ifndef _RDP_CPU_RING_H_
#define _RDP_CPU_RING_H_

#include "rdp_subsystem_common.h"

#include "rdpa_types.h"
#include "rdd.h"
#include "rdp_cpu_ring_defs.h"


typedef struct {
	uint8_t *data_ptr;
	uint8_t data_offset;
	uint16_t packet_size;
	uint16_t flow_id;
	uint16_t reason;
	uint16_t src_bridge_port;
	uint16_t dst_ssid;
	uint32_t wl_metadata;
	uint16_t ptp_index;
	uint16_t free_index;
	uint8_t is_rx_offload;
	uint8_t is_ipsec_upstream;
	uint8_t is_ucast;
	uint8_t is_exception;
	uint8_t is_csum_verified;
	uint8_t mcast_tx_prio;
} CPU_RX_PARAMS;

int rdp_cpu_ring_read_packet_copy(uint32_t ringId, CPU_RX_PARAMS *rxParams);

#endif /* _RDP_CPU_RING_H_ */
