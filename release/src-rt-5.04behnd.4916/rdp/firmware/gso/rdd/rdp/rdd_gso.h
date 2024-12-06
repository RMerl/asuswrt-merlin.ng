
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

#ifndef _RDD_GSO_H_
#define _RDD_GSO_H_

#include "bdmf_dev.h"

#if !defined(FIRMWARE_INIT)

typedef struct
{
	uint32_t	rx_packets                    	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	rx_octets                     	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	tx_packets                    	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	tx_octets                     	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	dropped_packets               	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	dropped_no_bpm_buffer         	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	dropped_parse_failed          	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	dropped_linear_length_invalid 	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	queue_full                    	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved                      	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_GSO_COUNTERS_ENTRY_DTS;

extern uint32_t  g_cpu_tx_no_free_gso_desc_counter;
extern uint32_t  g_cpu_tx_sent_abs_gso_packets_counter;
extern uint32_t  g_cpu_tx_sent_abs_gso_bytes_counter;

BL_LILAC_RDD_ERROR_DTE rdd_gso_counters_get ( RDD_GSO_COUNTERS_ENTRY_DTS *xo_gso_counters_ptr );
BL_LILAC_RDD_ERROR_DTE rdd_gso_context_get ( RDD_GSO_CONTEXT_ENTRY_DTS *xo_gso_context_ptr );
BL_LILAC_RDD_ERROR_DTE rdd_gso_desc_get ( RDD_GSO_DESC_ENTRY_DTS *xo_gso_desc_ptr );

#endif /* FIRMWARE_INIT */

#ifdef USE_BDMF_SHELL
#include "bdmf_shell.h"

int p_lilac_rdd_gso_counters_get (bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms);
int p_lilac_rdd_gso_debug_info_get (bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms);
#endif

#endif /* _RDD_GSO_H_ */

