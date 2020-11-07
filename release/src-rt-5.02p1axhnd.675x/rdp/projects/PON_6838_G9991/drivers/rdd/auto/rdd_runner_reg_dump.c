/*
   Copyright (c) 2015 Broadcom
   All Rights Reserved

    <:label-BRCM:2015:DUAL/GPL:standard

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



/* This is an automated file. Do not edit its contents. */


#include "bdmf_shell.h"
#include "access_macros.h"
#include "rdd_runner_reg_dump.h"

void dump_RDD_IH_BUFFER(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	int i,j;

	bdmf_session_print(session, "  Register IH_BUFFER\n");

	bdmf_session_print(session, "\treserved_fw_only         =\n\t");
	for (i=0,j=0; i<64; i++)
	{
		MREAD_I_32((uint8_t *)p, i, r);
		bdmf_session_print(session, "0x%08x ", (unsigned int)r);
		j++;
		if (j >= 8)
		{
			j = 0;
			bdmf_session_print(session, "\n\t");
		}
	}
	bdmf_session_print(session, "\n");
}

void dump_RDD_PACKET_DESCRIPTOR(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register PACKET_DESCRIPTOR\n");

	MREAD_32((uint8_t *)p, r);
	bdmf_session_print(session, "\treserved0                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 4, r);
	bdmf_session_print(session, "\treserved1                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 6, r);
	bdmf_session_print(session, "\tnext_packet_descriptor_pointer= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_BBH_TX_DESCRIPTOR(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register BBH_TX_DESCRIPTOR\n");

	FIELD_MREAD_8((uint8_t *)p, 7, 1, r);
	bdmf_session_print(session, "\tcrc_calc                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p, 3, 12, r);
	bdmf_session_print(session, "\tgem_port                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_32((uint8_t *)p + 0, 14, 5, r);
	bdmf_session_print(session, "\tsource_bridge_port       = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p + 2, 0, 14, r);
	bdmf_session_print(session, "\tpacket_length            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 4, 7, 1, r);
	bdmf_session_print(session, "\tabsolute_normal          = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 4, 6, 1, r);
	bdmf_session_print(session, "\tlast_indication          = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 4, 4, 2, r);
	bdmf_session_print(session, "\tpti                      = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 4, 3, 1, r);
	bdmf_session_print(session, "\t_1588                    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 4, 2, 1, r);
	bdmf_session_print(session, "\tadd_indication           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p + 4, 7, 3, r);
	bdmf_session_print(session, "\theader_number            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 5, 0, 7, r);
	bdmf_session_print(session, "\tpayload_offset           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 6, 7, 1, r);
	bdmf_session_print(session, "\ttarget_memory            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 6, 6, 1, r);
	bdmf_session_print(session, "\treserved0                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p + 6, 0, 14, r);
	bdmf_session_print(session, "\tpacket_location          = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_SIXTEEN_BYTES(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	int i,j;

	bdmf_session_print(session, "  Register SIXTEEN_BYTES\n");

	bdmf_session_print(session, "\treserved_fw_only         =\n\t");
	for (i=0,j=0; i<4; i++)
	{
		MREAD_I_32((uint8_t *)p, i, r);
		bdmf_session_print(session, "0x%08x ", (unsigned int)r);
		j++;
		if (j >= 8)
		{
			j = 0;
			bdmf_session_print(session, "\n\t");
		}
	}
	bdmf_session_print(session, "\n");
}

#if defined OREN
void dump_RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY\n");

	FIELD_MREAD_16((uint8_t *)p, 3, 13, r);
	bdmf_session_print(session, "\tvlan_index_table_ptr     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 1, 2, 1, r);
	bdmf_session_print(session, "\topbit_remark_mode        = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 1, 1, 1, r);
	bdmf_session_print(session, "\tipbit_remark_mode        = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_32((uint8_t *)p + 0, 13, 4, r);
	bdmf_session_print(session, "\twifi_ssid                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 2, 4, 1, r);
	bdmf_session_print(session, "\tdei_remark_enable        = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 2, 3, 1, r);
	bdmf_session_print(session, "\tdei_value                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 2, 2, 1, r);
	bdmf_session_print(session, "\tservice_queue_mode       = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 2, 1, 1, r);
	bdmf_session_print(session, "\treserved0                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 2, 0, 1, r);
	bdmf_session_print(session, "\tforward_mode             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 3, 4, 4, r);
	bdmf_session_print(session, "\tegress_port              = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 3, 3, 1, r);
	bdmf_session_print(session, "\tqos_mapping_mode         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 3, 0, 3, r);
	bdmf_session_print(session, "\ttraffic_class            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 4, 7, 1, r);
	bdmf_session_print(session, "\tdrop                     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 4, 6, 1, r);
	bdmf_session_print(session, "\tcpu                      = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 4, 5, 1, r);
	bdmf_session_print(session, "\tpolicer_mode             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 4, 1, 4, r);
	bdmf_session_print(session, "\tpolicer_id               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 4, 0, 1, r);
	bdmf_session_print(session, "\trate_shaping_mode        = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 5, 7, 1, r);
	bdmf_session_print(session, "\tcpu_mirroring            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 5, 6, 1, r);
	bdmf_session_print(session, "\tic_ip_flow               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 5, 1, 5, r);
	bdmf_session_print(session, "\tservice_queue            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_32((uint8_t *)p + 4, 15, 2, r);
	bdmf_session_print(session, "\tsubnet_id                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 6, 4, 3, r);
	bdmf_session_print(session, "\touter_pbit               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 6, 1, 3, r);
	bdmf_session_print(session, "\tinner_pbit               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 6, 0, 1, r);
	bdmf_session_print(session, "\tdscp_remarking_mode      = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 7, 2, 6, r);
	bdmf_session_print(session, "\tdscp                     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 7, 0, 2, r);
	bdmf_session_print(session, "\tecn                      = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

#endif

void dump_RDD_INGRESS_FILTERS_LOOKUP_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register INGRESS_FILTERS_LOOKUP_ENTRY\n");

	FIELD_MREAD_8((uint8_t *)p, 4, 4, r);
	bdmf_session_print(session, "\tl4_protocol              = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p, 3, 1, r);
	bdmf_session_print(session, "\terror                    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p, 2, 1, r);
	bdmf_session_print(session, "\tptag                     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p, 0, 2, r);
	bdmf_session_print(session, "\tnumber_of_vlans          = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 1, 7, 1, r);
	bdmf_session_print(session, "\tbroadcast                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 1, 6, 1, r);
	bdmf_session_print(session, "\tmulticast                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 1, 4, 2, r);
	bdmf_session_print(session, "\tl3_protocol              = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 1, 0, 4, r);
	bdmf_session_print(session, "\tl2_protocol              = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 2, 4, 4, r);
	bdmf_session_print(session, "\tl4_protocol_mask         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 2, 3, 1, r);
	bdmf_session_print(session, "\terror_mask               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 2, 2, 1, r);
	bdmf_session_print(session, "\tptag_mask                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 2, 0, 2, r);
	bdmf_session_print(session, "\tnumber_of_vlans_mask     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 3, 7, 1, r);
	bdmf_session_print(session, "\tbroadcast_mask           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 3, 6, 1, r);
	bdmf_session_print(session, "\tmulticast_mask           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 3, 4, 2, r);
	bdmf_session_print(session, "\tl3_protocol_mask         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 3, 0, 4, r);
	bdmf_session_print(session, "\tl2_protocol_mask         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_ETH_TX_RS_QUEUE_DESCRIPTOR(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register ETH_TX_RS_QUEUE_DESCRIPTOR\n");

	MREAD_16((uint8_t *)p, r);
	bdmf_session_print(session, "\thead_ptr                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 2, r);
	bdmf_session_print(session, "\ttail_ptr                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 4, r);
	bdmf_session_print(session, "\tingress_packet_counter   = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 6, r);
	bdmf_session_print(session, "\tegress_packet_counter    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 8, r);
	bdmf_session_print(session, "\tpacket_threshold         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 10, r);
	bdmf_session_print(session, "\tprofile_ptr              = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 12, r);
	bdmf_session_print(session, "\trs_status_offset         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 13, r);
	bdmf_session_print(session, "\trs_group_status_offset   = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 14, r);
	bdmf_session_print(session, "\tstatus_offset            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 15, r);
	bdmf_session_print(session, "\treserved0                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_PBITS_TO_PBITS_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register PBITS_TO_PBITS_ENTRY\n");

	MREAD_8((uint8_t *)p, r);
	bdmf_session_print(session, "\tpbits                    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_DS_WAN_FLOW_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register DS_WAN_FLOW_ENTRY\n");

	FIELD_MREAD_8((uint8_t *)p, 7, 1, r);
	bdmf_session_print(session, "\treserved                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p, 1, 6, r);
	bdmf_session_print(session, "\tcpu_reason               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p, 0, 1, r);
	bdmf_session_print(session, "\tingress_classify_mode    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 1, r);
	bdmf_session_print(session, "\tingress_flow             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_IPTV_COUNTER_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register IPTV_COUNTER_ENTRY\n");

	MREAD_16((uint8_t *)p, r);
	bdmf_session_print(session, "\tcounter                  = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_ETH_TX_LOCAL_REGISTERS_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register ETH_TX_LOCAL_REGISTERS_ENTRY\n");

	MREAD_32((uint8_t *)p, r);
	bdmf_session_print(session, "\tunion_field1             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 4, r);
	bdmf_session_print(session, "\tunion_field2             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_QUEUE_PROFILE(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register QUEUE_PROFILE\n");

	FIELD_MREAD_8((uint8_t *)p, 7, 1, r);
	bdmf_session_print(session, "\tus_flow_control_mode     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p, 0, 15, r);
	bdmf_session_print(session, "\treserved0                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 2, r);
	bdmf_session_print(session, "\tmax_low_threshold        = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 4, r);
	bdmf_session_print(session, "\tmin_high_threshold       = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 6, r);
	bdmf_session_print(session, "\tmax_high_threshold       = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 8, 7, 1, r);
	bdmf_session_print(session, "\tlow_large_interval_flag  = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p + 8, 0, 15, r);
	bdmf_session_print(session, "\treserved1                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 10, r);
	bdmf_session_print(session, "\tlow_drop_constant        = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 12, 7, 1, r);
	bdmf_session_print(session, "\thigh_large_interval_flag = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p + 12, 0, 15, r);
	bdmf_session_print(session, "\treserved2                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 14, r);
	bdmf_session_print(session, "\thigh_drop_constant       = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_PBITS_PARAMETER_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register PBITS_PARAMETER_ENTRY\n");

	MREAD_8((uint8_t *)p, r);
	bdmf_session_print(session, "\touter_pbits              = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 1, r);
	bdmf_session_print(session, "\tinner_pbits              = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_IPTV_SSID_EXTENSION_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register IPTV_SSID_EXTENSION_ENTRY\n");

	MREAD_16((uint8_t *)p, r);
	bdmf_session_print(session, "\twlan_mcast_index         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_VLAN_PARAMETER_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register VLAN_PARAMETER_ENTRY\n");

	FIELD_MREAD_8((uint8_t *)p, 7, 1, r);
	bdmf_session_print(session, "\touter_tpid_overwrite_enable= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p, 6, 1, r);
	bdmf_session_print(session, "\tinner_tpid_overwrite_enable= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p, 3, 3, r);
	bdmf_session_print(session, "\touter_tpid_id            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p, 0, 3, r);
	bdmf_session_print(session, "\tinner_tpid_id            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_32((uint8_t *)p + 0, 12, 12, r);
	bdmf_session_print(session, "\touter_vid                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p + 2, 0, 12, r);
	bdmf_session_print(session, "\tinner_vid                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_CPU_REASON_TO_METER_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register CPU_REASON_TO_METER_ENTRY\n");

	MREAD_8((uint8_t *)p, r);
	bdmf_session_print(session, "\tcpu_meter                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_IPTV_INGRESS_CLASSIFICATION_CONTEXT_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register IPTV_INGRESS_CLASSIFICATION_CONTEXT_ENTRY\n");

	MREAD_8((uint8_t *)p, r);
	bdmf_session_print(session, "\tcontext                  = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_TIMER_TASK_DESCRIPTOR_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register TIMER_TASK_DESCRIPTOR_ENTRY\n");

	MREAD_8((uint8_t *)p, r);
	bdmf_session_print(session, "\tperiod                   = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 1, r);
	bdmf_session_print(session, "\tcounter_reload           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 2, r);
	bdmf_session_print(session, "\tfirmware_ptr             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 4, r);
	bdmf_session_print(session, "\treserved                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_SBPM_REPLY_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	int i,j;

	bdmf_session_print(session, "  Register SBPM_REPLY_ENTRY\n");

	bdmf_session_print(session, "\treserved_fw_only         =\n\t");
	for (i=0,j=0; i<32; i++)
	{
		MREAD_I_32((uint8_t *)p, i, r);
		bdmf_session_print(session, "0x%08x ", (unsigned int)r);
		j++;
		if (j >= 8)
		{
			j = 0;
			bdmf_session_print(session, "\n\t");
		}
	}
	bdmf_session_print(session, "\n");
}

void dump_RDD_CONNECTION_CONTEXT_BUFFER_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	int i,j;

	bdmf_session_print(session, "  Register CONNECTION_CONTEXT_BUFFER_ENTRY\n");

	bdmf_session_print(session, "\treserved_fw_only         =\n\t");
	for (i=0,j=0; i<16; i++)
	{
		MREAD_I_32((uint8_t *)p, i, r);
		bdmf_session_print(session, "0x%08x ", (unsigned int)r);
		j++;
		if (j >= 8)
		{
			j = 0;
			bdmf_session_print(session, "\n\t");
		}
	}
	bdmf_session_print(session, "\n");
}

void dump_RDD_ETH_TX_QUEUE_DESCRIPTOR(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register ETH_TX_QUEUE_DESCRIPTOR\n");

	MREAD_16((uint8_t *)p, r);
	bdmf_session_print(session, "\thead_ptr                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 2, r);
	bdmf_session_print(session, "\ttail_ptr                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 4, r);
	bdmf_session_print(session, "\tingress_packet_counter   = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 6, r);
	bdmf_session_print(session, "\tegress_packet_counter    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 8, r);
	bdmf_session_print(session, "\tpacket_threshold         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 10, r);
	bdmf_session_print(session, "\tprofile_ptr              = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 12, r);
	bdmf_session_print(session, "\treserved0                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 13, r);
	bdmf_session_print(session, "\tcounter_number           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 14, r);
	bdmf_session_print(session, "\tqueue_mask               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 15, r);
	bdmf_session_print(session, "\trate_limit_override      = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_SERVICE_QUEUES_RATE_LIMITER_DESCRIPTOR(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register SERVICE_QUEUES_RATE_LIMITER_DESCRIPTOR\n");

	MREAD_32((uint8_t *)p, r);
	bdmf_session_print(session, "\tcurrent_peak_budget      = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 4, 6, 2, r);
	bdmf_session_print(session, "\tallocated_peak_budget_exponent= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p + 4, 0, 14, r);
	bdmf_session_print(session, "\tallocated_peak_budget    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 6, 6, 2, r);
	bdmf_session_print(session, "\tpeak_budget_limit_exponent= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p + 6, 0, 14, r);
	bdmf_session_print(session, "\tpeak_budget_limit        = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 8, r);
	bdmf_session_print(session, "\tcurrent_sustain_budget   = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 12, r);
	bdmf_session_print(session, "\tallocated_sustain_budget = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 16, r);
	bdmf_session_print(session, "\trate_limiter_mask        = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 20, r);
	bdmf_session_print(session, "\tpeak_burst_counter       = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 22, r);
	bdmf_session_print(session, "\tpeak_weight              = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 23, 1, 7, r);
	bdmf_session_print(session, "\treserved0                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 23, 0, 1, r);
	bdmf_session_print(session, "\tpeak_burst_flag          = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_PBITS_PRIMITIVE_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register PBITS_PRIMITIVE_ENTRY\n");

	MREAD_16((uint8_t *)p, r);
	bdmf_session_print(session, "\tprimitive_address        = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_FOUR_BYTES(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register FOUR_BYTES\n");

	MREAD_32((uint8_t *)p, r);
	bdmf_session_print(session, "\treserved_fw_only         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_RATE_LIMITER_REMAINDER_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register RATE_LIMITER_REMAINDER_ENTRY\n");

	MREAD_8((uint8_t *)p, r);
	bdmf_session_print(session, "\tallocated_budget         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 1, r);
	bdmf_session_print(session, "\tcurrent_budget           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_RATE_LIMITER_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register RATE_LIMITER_ENTRY\n");

	MREAD_32((uint8_t *)p, r);
	bdmf_session_print(session, "\tcurrent_budget           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 4, 7, 1, r);
	bdmf_session_print(session, "\tbudget_limit_exp         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p + 4, 0, 15, r);
	bdmf_session_print(session, "\tbudget_limit             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 6, 7, 1, r);
	bdmf_session_print(session, "\tallocated_budget_exp     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p + 6, 0, 15, r);
	bdmf_session_print(session, "\tallocated_budget         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_VLAN_COMMAND_ENRTY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register VLAN_COMMAND_ENRTY\n");

	MREAD_8((uint8_t *)p, r);
	bdmf_session_print(session, "\tvlan_untagged_command    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 1, r);
	bdmf_session_print(session, "\tpbits_untagged_command   = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 2, r);
	bdmf_session_print(session, "\tvlan_single_tagged_command= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 3, r);
	bdmf_session_print(session, "\tpbits_single_tagged_command= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 4, r);
	bdmf_session_print(session, "\tvlan_double_tagged_command= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 5, r);
	bdmf_session_print(session, "\tpbits_double_tagged_command= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 6, r);
	bdmf_session_print(session, "\tvlan_priority_tagged_command= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 7, r);
	bdmf_session_print(session, "\tpbits_priority_tagged_command= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_VID_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register VID_ENTRY\n");

	MREAD_16((uint8_t *)p, r);
	bdmf_session_print(session, "\tvid                      = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_POLICER_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register POLICER_ENTRY\n");

	MREAD_32((uint8_t *)p, r);
	bdmf_session_print(session, "\tcurrent_budget           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 4, r);
	bdmf_session_print(session, "\texponent                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 6, r);
	bdmf_session_print(session, "\tcommited_rate            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 8, r);
	bdmf_session_print(session, "\treserved                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 10, r);
	bdmf_session_print(session, "\tbucket_size              = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 12, r);
	bdmf_session_print(session, "\tcommited_burst           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 14, r);
	bdmf_session_print(session, "\tdrop_counter             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_CPU_TX_BBH_DESCRIPTORS_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	int i,j;

	bdmf_session_print(session, "  Register CPU_TX_BBH_DESCRIPTORS_ENTRY\n");

	bdmf_session_print(session, "\treserved_fw_only         =\n\t");
	for (i=0,j=0; i<2; i++)
	{
		MREAD_I_32((uint8_t *)p, i, r);
		bdmf_session_print(session, "0x%08x ", (unsigned int)r);
		j++;
		if (j >= 8)
		{
			j = 0;
			bdmf_session_print(session, "\n\t");
		}
	}
	bdmf_session_print(session, "\n");
}

void dump_RDD_ETH_TX_QUEUE_POINTERS_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register ETH_TX_QUEUE_POINTERS_ENTRY\n");

	MREAD_16((uint8_t *)p, r);
	bdmf_session_print(session, "\teth_mac_pointer          = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 2, r);
	bdmf_session_print(session, "\ttx_queue_pointer         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_TWO_BYTES(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register TWO_BYTES\n");

	MREAD_16((uint8_t *)p, r);
	bdmf_session_print(session, "\treserved_fw_only         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_EIGHT_BYTES(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	int i,j;

	bdmf_session_print(session, "  Register EIGHT_BYTES\n");

	bdmf_session_print(session, "\treserved_fw_only         =\n\t");
	for (i=0,j=0; i<2; i++)
	{
		MREAD_I_32((uint8_t *)p, i, r);
		bdmf_session_print(session, "0x%08x ", (unsigned int)r);
		j++;
		if (j >= 8)
		{
			j = 0;
			bdmf_session_print(session, "\n\t");
		}
	}
	bdmf_session_print(session, "\n");
}

void dump_RDD_INGRESS_FILTERS_PARAMETER_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register INGRESS_FILTERS_PARAMETER_ENTRY\n");

	FIELD_MREAD_8((uint8_t *)p, 7, 1, r);
	bdmf_session_print(session, "\tcpu_trap                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p, 6, 1, r);
	bdmf_session_print(session, "\tdrop                     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p, 0, 6, r);
	bdmf_session_print(session, "\tparameter                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_IPTV_SSID_EXTENSION_TABLE_CAM(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	int i,j;

	bdmf_session_print(session, "  Register IPTV_SSID_EXTENSION_TABLE_CAM\n");

	bdmf_session_print(session, "\treserved_fw_only         =\n\t");
	for (i=0,j=0; i<16; i++)
	{
		MREAD_I_32((uint8_t *)p, i, r);
		bdmf_session_print(session, "0x%08x ", (unsigned int)r);
		j++;
		if (j >= 8)
		{
			j = 0;
			bdmf_session_print(session, "\n\t");
		}
	}
	bdmf_session_print(session, "\n");
}

void dump_RDD_FORWARDING_MATRIX_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register FORWARDING_MATRIX_ENTRY\n");

	MREAD_8((uint8_t *)p, r);
	bdmf_session_print(session, "\tenable                   = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_TPID_OVERWRITE_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register TPID_OVERWRITE_ENTRY\n");

	MREAD_16((uint8_t *)p, r);
	bdmf_session_print(session, "\ttpid                     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_INGRESS_CLASSIFICATION_GENERIC_RULE_CFG_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register INGRESS_CLASSIFICATION_GENERIC_RULE_CFG_ENTRY\n");

	FIELD_MREAD_8((uint8_t *)p, 6, 2, r);
	bdmf_session_print(session, "\tgeneric_rule_type        = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_32((uint8_t *)p, 7, 23, r);
	bdmf_session_print(session, "\treserved1                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 3, 0, 7, r);
	bdmf_session_print(session, "\tgeneric_rule_offset      = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 4, r);
	bdmf_session_print(session, "\tgeneric_rule_mask        = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_LAYER4_FILTERS_LOOKUP_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register LAYER4_FILTERS_LOOKUP_ENTRY\n");

	FIELD_MREAD_8((uint8_t *)p, 7, 1, r);
	bdmf_session_print(session, "\t_5_tupple_valid          = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p, 6, 1, r);
	bdmf_session_print(session, "\tip_first_fragment        = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p, 5, 1, r);
	bdmf_session_print(session, "\tip_fragment              = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p, 4, 1, r);
	bdmf_session_print(session, "\tip_filter_match          = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p, 2, 2, r);
	bdmf_session_print(session, "\tip_filter_match_num      = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p, 1, 1, r);
	bdmf_session_print(session, "\ttcp_udp                  = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p, 0, 1, r);
	bdmf_session_print(session, "\tipv6_ext_header_filter   = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 1, 7, 1, r);
	bdmf_session_print(session, "\ttcp_flag                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 1, 6, 1, r);
	bdmf_session_print(session, "\twan                      = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 1, 5, 1, r);
	bdmf_session_print(session, "\tvid_fit                  = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 1, 4, 1, r);
	bdmf_session_print(session, "\texception                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 1, 1, 3, r);
	bdmf_session_print(session, "\tda_filter_number         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 1, 0, 1, r);
	bdmf_session_print(session, "\tda_filter                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 2, 4, 4, r);
	bdmf_session_print(session, "\tl4_protocol              = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 2, 3, 1, r);
	bdmf_session_print(session, "\terror                    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 2, 2, 1, r);
	bdmf_session_print(session, "\tptag                     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 2, 0, 2, r);
	bdmf_session_print(session, "\tnumber_of_vlans          = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 3, 7, 1, r);
	bdmf_session_print(session, "\tbroadcast                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 3, 6, 1, r);
	bdmf_session_print(session, "\tmulticast                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 3, 4, 2, r);
	bdmf_session_print(session, "\tl3_protocol              = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 3, 0, 4, r);
	bdmf_session_print(session, "\tl2_protocol              = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 4, 7, 1, r);
	bdmf_session_print(session, "\t_5_tupple_valid_mask     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 4, 6, 1, r);
	bdmf_session_print(session, "\tip_first_fragment_mask   = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 4, 5, 1, r);
	bdmf_session_print(session, "\tip_fragment_mask         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 4, 4, 1, r);
	bdmf_session_print(session, "\tip_filter_match_mask     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 4, 2, 2, r);
	bdmf_session_print(session, "\tip_filter_match_num_mask = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 4, 1, 1, r);
	bdmf_session_print(session, "\ttcp_udp_mask             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 4, 0, 1, r);
	bdmf_session_print(session, "\tipv6_ext_header_filter_mask= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 5, 7, 1, r);
	bdmf_session_print(session, "\ttcp_flag_mask            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 5, 6, 1, r);
	bdmf_session_print(session, "\twan_mask                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 5, 5, 1, r);
	bdmf_session_print(session, "\tvid_fit_mask             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 5, 4, 1, r);
	bdmf_session_print(session, "\texception_mask           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 5, 1, 3, r);
	bdmf_session_print(session, "\tda_filter_number_mask    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 5, 0, 1, r);
	bdmf_session_print(session, "\tda_filter_mask           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 6, 4, 4, r);
	bdmf_session_print(session, "\tl4_protocol_mask         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 6, 3, 1, r);
	bdmf_session_print(session, "\terror_mask               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 6, 2, 1, r);
	bdmf_session_print(session, "\tptag_mask                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 6, 0, 2, r);
	bdmf_session_print(session, "\tnumber_of_vlans_mask     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 7, 7, 1, r);
	bdmf_session_print(session, "\tbroadcast_mask           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 7, 6, 1, r);
	bdmf_session_print(session, "\tmulticast_mask           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 7, 4, 2, r);
	bdmf_session_print(session, "\tl3_protocol_mask         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 7, 0, 4, r);
	bdmf_session_print(session, "\tl2_protocol_mask         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_CPU_RX_METER_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register CPU_RX_METER_ENTRY\n");

	MREAD_16((uint8_t *)p, r);
	bdmf_session_print(session, "\tcurrent_budget           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 2, r);
	bdmf_session_print(session, "\tbudget_limit             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 4, r);
	bdmf_session_print(session, "\tallocated_budget         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 6, r);
	bdmf_session_print(session, "\treserved                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_BRIDGE_CONFIGURATION_REGISTER(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	int i,j;

	bdmf_session_print(session, "  Register BRIDGE_CONFIGURATION_REGISTER\n");

	MREAD_8((uint8_t *)p, r);
	bdmf_session_print(session, "\tds_miss_eth_flow         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 1, r);
	bdmf_session_print(session, "\tds_untagged_eth_flow     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 2, r);
	bdmf_session_print(session, "\twan_to_wan_ingress_flow  = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 3, r);
	bdmf_session_print(session, "\trate_limit_overhead      = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 4, r);
	bdmf_session_print(session, "\tsubnet_classification_mode= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 5, r);
	bdmf_session_print(session, "\tglobal_ingress_config    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 6, r);
	bdmf_session_print(session, "\tdrop_precedence_eligibility_vector= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 8, r);
	bdmf_session_print(session, "\twan_unknown_sa_command   = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 9, r);
	bdmf_session_print(session, "\teth0_unknown_sa_command  = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 10, r);
	bdmf_session_print(session, "\teth1_unknown_sa_command  = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 11, r);
	bdmf_session_print(session, "\teth2_unknown_sa_command  = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 12, r);
	bdmf_session_print(session, "\teth3_unknown_sa_command  = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 13, r);
	bdmf_session_print(session, "\teth4_unknown_sa_command  = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 14, r);
	bdmf_session_print(session, "\twan_router_unknown_sa_command= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 15, r);
	bdmf_session_print(session, "\twan_iptv_unknown_sa_command= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 16, r);
	bdmf_session_print(session, "\tpci_unknown_sa_command   = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 17, r);
	bdmf_session_print(session, "\tus_unknown_da_flooding_bridge_port= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 18, r);
	bdmf_session_print(session, "\twan_unknown_da_command   = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 19, r);
	bdmf_session_print(session, "\teth0_unknown_da_command  = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 20, r);
	bdmf_session_print(session, "\teth1_unknown_da_command  = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 21, r);
	bdmf_session_print(session, "\teth2_unknown_da_command  = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 22, r);
	bdmf_session_print(session, "\teth3_unknown_da_command  = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 23, r);
	bdmf_session_print(session, "\teth4_unknown_da_command  = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 24, r);
	bdmf_session_print(session, "\twan_router_unknown_da_command= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 25, r);
	bdmf_session_print(session, "\tbroadcom_switch_port     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 26, r);
	bdmf_session_print(session, "\tpci_unknown_da_command   = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 27, r);
	bdmf_session_print(session, "\tflooding_bridge_ports_vector= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 28, r);
	bdmf_session_print(session, "\tegress_ether_type_1      = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 30, r);
	bdmf_session_print(session, "\tegress_ether_type_2      = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 32, r);
	bdmf_session_print(session, "\tegress_ether_type_3      = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 34, r);
	bdmf_session_print(session, "\tus_rate_controller_timer = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 36, r);
	bdmf_session_print(session, "\tus_g9991_mtu_max_fragments= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 37, r);
	bdmf_session_print(session, "\tus_g9991_mtu_max_eof_length= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 38, r);
	bdmf_session_print(session, "\tus_tx_queue_flow_control_enable= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 39, r);
	bdmf_session_print(session, "\tpacket_buffer_size_asr_8 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 40, r);
	bdmf_session_print(session, "\tmtu                      = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 42, r);
	bdmf_session_print(session, "\treserved1                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 44, r);
	bdmf_session_print(session, "\tpci_ls_dp_eligibility_vector= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 46, r);
	bdmf_session_print(session, "\tip_sync_1588_mode        = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 47, r);
	bdmf_session_print(session, "\tiptv_classification_method= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 48, r);
	bdmf_session_print(session, "\tds_connection_miss_action= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 49, r);
	bdmf_session_print(session, "\tdscp_to_wan_flow_control = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 50, r);
	bdmf_session_print(session, "\tipv6_enable              = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 51, r);
	bdmf_session_print(session, "\thash_based_forwarding_port_count= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 52, r);
	bdmf_session_print(session, "\tus_padding_max_size      = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 54, r);
	bdmf_session_print(session, "\tus_padding_cpu_max_size  = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 56, r);
	bdmf_session_print(session, "\tactive_policers_vector   = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 58, r);
	bdmf_session_print(session, "\tmirroring_port           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 59, r);
	bdmf_session_print(session, "\tvlan_binding_mode        = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 60, r);
	bdmf_session_print(session, "\tpolicers_period          = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 62, r);
	bdmf_session_print(session, "\ttimer_scheduler_period   = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 64, r);
	bdmf_session_print(session, "\ttpid_detect_value        = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 66, r);
	bdmf_session_print(session, "\tflooding_wifi_ssid_vector= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 68, r);
	bdmf_session_print(session, "\tinter_lan_scheduling_mode= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 69, r);
	bdmf_session_print(session, "\tipv6_ecn_remark          = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 70, r);
	bdmf_session_print(session, "\tds_ingress_policers_mode = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 71, r);
	bdmf_session_print(session, "\tdebug_mode               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 72, r);
	bdmf_session_print(session, "\tus_rate_limit_sustain_budget_limit= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 73, r);
	bdmf_session_print(session, "\tforce_dscp_to_pbit       = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 74, r);
	bdmf_session_print(session, "\twan_channel_mapping      = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 75, r);
	bdmf_session_print(session, "\tack_prioritization       = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 76, r);
	bdmf_session_print(session, "\tack_packet_size_threshold= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	bdmf_session_print(session, "\treserved_bytes           =\n\t");
	for (i=0,j=0; i<179; i++)
	{
		MREAD_I_8((uint8_t *)p + 77, i, r);
		bdmf_session_print(session, "0x%08x ", (unsigned int)r);
		j++;
		if (j >= 8)
		{
			j = 0;
			bdmf_session_print(session, "\n\t");
		}
	}
	bdmf_session_print(session, "\n");
}

void dump_RDD_INGRESS_CLASSIFICATION_RULE_CFG_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register INGRESS_CLASSIFICATION_RULE_CFG_ENTRY\n");

	FIELD_MREAD_8((uint8_t *)p, 3, 5, r);
	bdmf_session_print(session, "\tnext_rule_cfg_id         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p, 6, 5, r);
	bdmf_session_print(session, "\tnext_group_id            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 1, 3, 3, r);
	bdmf_session_print(session, "\trule_type                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 1, 1, 2, r);
	bdmf_session_print(session, "\tlookup_mode              = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 1, 0, 1, r);
	bdmf_session_print(session, "\thit_action               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 2, 7, 1, r);
	bdmf_session_print(session, "\tmiss_action              = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p + 2, 0, 15, r);
	bdmf_session_print(session, "\treserved1                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 4, 4, 4, r);
	bdmf_session_print(session, "\treserved2                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 4, 2, 2, r);
	bdmf_session_print(session, "\tgeneric_rule_index_1     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 4, 0, 2, r);
	bdmf_session_print(session, "\tgeneric_rule_index_2     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_32((uint8_t *)p + 4, 0, 24, r);
	bdmf_session_print(session, "\tkey_mask                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_PROFILING_BUFFER_PICO_RUNNER(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	int i,j;

	bdmf_session_print(session, "  Register PROFILING_BUFFER_PICO_RUNNER\n");

	bdmf_session_print(session, "\treserved_fw_only         =\n\t");
	for (i=0,j=0; i<64; i++)
	{
		MREAD_I_32((uint8_t *)p, i, r);
		bdmf_session_print(session, "0x%08x ", (unsigned int)r);
		j++;
		if (j >= 8)
		{
			j = 0;
			bdmf_session_print(session, "\n\t");
		}
	}
	bdmf_session_print(session, "\n");
}

void dump_RDD_VLAN_OPTIMIZATION_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register VLAN_OPTIMIZATION_ENTRY\n");

	MREAD_8((uint8_t *)p, r);
	bdmf_session_print(session, "\toptimize_enable          = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_PCI_MULTICAST_SCRATCHPAD(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	int i,j;

	bdmf_session_print(session, "  Register PCI_MULTICAST_SCRATCHPAD\n");

	bdmf_session_print(session, "\treserved_fw_only         =\n\t");
	for (i=0,j=0; i<32; i++)
	{
		MREAD_I_32((uint8_t *)p, i, r);
		bdmf_session_print(session, "0x%08x ", (unsigned int)r);
		j++;
		if (j >= 8)
		{
			j = 0;
			bdmf_session_print(session, "\n\t");
		}
	}
	bdmf_session_print(session, "\n");
}

void dump_RDD_EMAC_SKB_ENQUEUED_INDEXES_FIFO_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	int i,j;

	bdmf_session_print(session, "  Register EMAC_SKB_ENQUEUED_INDEXES_FIFO_ENTRY\n");

	bdmf_session_print(session, "\treserved_fw_only         =\n\t");
	for (i=0,j=0; i<16; i++)
	{
		MREAD_I_16((uint8_t *)p, i, r);
		bdmf_session_print(session, "0x%08x ", (unsigned int)r);
		j++;
		if (j >= 8)
		{
			j = 0;
			bdmf_session_print(session, "\n\t");
		}
	}
	bdmf_session_print(session, "\n");
}

void dump_RDD_FAST_RUNNER_GLOBAL_REGISTERS_INIT_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register FAST_RUNNER_GLOBAL_REGISTERS_INIT_ENTRY\n");

	MREAD_32((uint8_t *)p, r);
	bdmf_session_print(session, "\treserved_fw_only         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_QUEUE_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register QUEUE_ENTRY\n");

	MREAD_8((uint8_t *)p, r);
	bdmf_session_print(session, "\tqueue                    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_TIMER_SCHEDULER_PRIMITIVE_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register TIMER_SCHEDULER_PRIMITIVE_ENTRY\n");

	MREAD_16((uint8_t *)p, r);
	bdmf_session_print(session, "\tprimitive_address        = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_RUNNER_FLOW_HEADER_BUFFER(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	int i,j;

	bdmf_session_print(session, "  Register RUNNER_FLOW_HEADER_BUFFER\n");

	bdmf_session_print(session, "\treserved_fw_only         =\n\t");
	for (i=0,j=0; i<32; i++)
	{
		MREAD_I_32((uint8_t *)p, i, r);
		bdmf_session_print(session, "0x%08x ", (unsigned int)r);
		j++;
		if (j >= 8)
		{
			j = 0;
			bdmf_session_print(session, "\n\t");
		}
	}
	bdmf_session_print(session, "\n");
}

void dump_RDD_ONE_BYTE(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register ONE_BYTE\n");

	MREAD_8((uint8_t *)p, r);
	bdmf_session_print(session, "\treserved_fw_only         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_DSCP_TO_PBITS_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register DSCP_TO_PBITS_ENTRY\n");

	MREAD_8((uint8_t *)p, r);
	bdmf_session_print(session, "\tpbits                    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_VLAN_PRIMITIVE_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register VLAN_PRIMITIVE_ENTRY\n");

	MREAD_16((uint8_t *)p, r);
	bdmf_session_print(session, "\tprimitive_address        = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_PACKET_SRAM_TO_DDR_COPY_BUFFER(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	int i,j;

	bdmf_session_print(session, "  Register PACKET_SRAM_TO_DDR_COPY_BUFFER\n");

	bdmf_session_print(session, "\treserved_fw_only         =\n\t");
	for (i=0,j=0; i<32; i++)
	{
		MREAD_I_32((uint8_t *)p, i, r);
		bdmf_session_print(session, "0x%08x ", (unsigned int)r);
		j++;
		if (j >= 8)
		{
			j = 0;
			bdmf_session_print(session, "\n\t");
		}
	}
	bdmf_session_print(session, "\n");
}

void dump_RDD_INGRESS_QUEUE_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register INGRESS_QUEUE_ENTRY\n");

	MREAD_8((uint8_t *)p, r);
	bdmf_session_print(session, "\treserved_fw_only         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_CPU_TX_MESSAGE_DATA_BUFFER_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	int i,j;

	bdmf_session_print(session, "  Register CPU_TX_MESSAGE_DATA_BUFFER_ENTRY\n");

	bdmf_session_print(session, "\treserved_fw_only         =\n\t");
	for (i=0,j=0; i<16; i++)
	{
		MREAD_I_32((uint8_t *)p, i, r);
		bdmf_session_print(session, "0x%08x ", (unsigned int)r);
		j++;
		if (j >= 8)
		{
			j = 0;
			bdmf_session_print(session, "\n\t");
		}
	}
	bdmf_session_print(session, "\n");
}

void dump_RDD_VLAN_ACTION_BUFFER(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	int i,j;

	bdmf_session_print(session, "  Register VLAN_ACTION_BUFFER\n");

	bdmf_session_print(session, "\treserved_fw_only         =\n\t");
	for (i=0,j=0; i<16; i++)
	{
		MREAD_I_32((uint8_t *)p, i, r);
		bdmf_session_print(session, "0x%08x ", (unsigned int)r);
		j++;
		if (j >= 8)
		{
			j = 0;
			bdmf_session_print(session, "\n\t");
		}
	}
	bdmf_session_print(session, "\n");
}

void dump_RDD_PICO_RUNNER_GLOBAL_REGISTERS_INIT_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register PICO_RUNNER_GLOBAL_REGISTERS_INIT_ENTRY\n");

	MREAD_32((uint8_t *)p, r);
	bdmf_session_print(session, "\treserved_fw_only         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_BPM_DDR_OPTIMIZED_BUFFERS_WITHOUT_HEADROOM_BASE(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register BPM_DDR_OPTIMIZED_BUFFERS_WITHOUT_HEADROOM_BASE\n");

	MREAD_32((uint8_t *)p, r);
	bdmf_session_print(session, "\treserved_fw_only         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_CPU_PARAMETERS_BLOCK_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register CPU_PARAMETERS_BLOCK_ENTRY\n");

	MREAD_16((uint8_t *)p, r);
	bdmf_session_print(session, "\tssid_vector              = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 2, r);
	bdmf_session_print(session, "\twlan_info                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 4, r);
	bdmf_session_print(session, "\theadroom_size            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 5, r);
	bdmf_session_print(session, "\tsrc_bridge_port          = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 6, r);
	bdmf_session_print(session, "\tdma_sync                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 7, r);
	bdmf_session_print(session, "\ttype                     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_FREE_SKB_INDEXES_FIFO_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register FREE_SKB_INDEXES_FIFO_ENTRY\n");

	FIELD_MREAD_8((uint8_t *)p, 7, 1, r);
	bdmf_session_print(session, "\townership                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p, 6, 1, r);
	bdmf_session_print(session, "\treserved                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p, 0, 14, r);
	bdmf_session_print(session, "\tskb_index                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_DEBUG_BUFFER_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register DEBUG_BUFFER_ENTRY\n");

	MREAD_32((uint8_t *)p, r);
	bdmf_session_print(session, "\treserved_fw_only         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_HASH_BUFFER(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	int i,j;

	bdmf_session_print(session, "  Register HASH_BUFFER\n");

	bdmf_session_print(session, "\treserved_fw_only         =\n\t");
	for (i=0,j=0; i<4; i++)
	{
		MREAD_I_32((uint8_t *)p, i, r);
		bdmf_session_print(session, "0x%08x ", (unsigned int)r);
		j++;
		if (j >= 8)
		{
			j = 0;
			bdmf_session_print(session, "\n\t");
		}
	}
	bdmf_session_print(session, "\n");
}

void dump_RDD_IPTV_DMA_LKP_KEY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	int i,j;

	bdmf_session_print(session, "  Register IPTV_DMA_LKP_KEY\n");

	bdmf_session_print(session, "\treserved_fw_only         =\n\t");
	for (i=0,j=0; i<4; i++)
	{
		MREAD_I_32((uint8_t *)p, i, r);
		bdmf_session_print(session, "0x%08x ", (unsigned int)r);
		j++;
		if (j >= 8)
		{
			j = 0;
			bdmf_session_print(session, "\n\t");
		}
	}
	bdmf_session_print(session, "\n");
}

void dump_RDD_MULTICAST_HEADER_BUFFER(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	int i,j;

	bdmf_session_print(session, "  Register MULTICAST_HEADER_BUFFER\n");

	bdmf_session_print(session, "\treserved_fw_only         =\n\t");
	for (i=0,j=0; i<8; i++)
	{
		MREAD_I_32((uint8_t *)p, i, r);
		bdmf_session_print(session, "0x%08x ", (unsigned int)r);
		j++;
		if (j >= 8)
		{
			j = 0;
			bdmf_session_print(session, "\n\t");
		}
	}
	bdmf_session_print(session, "\n");
}

void dump_RDD_IPV6_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	int i,j;

	bdmf_session_print(session, "  Register IPV6_ENTRY\n");

	bdmf_session_print(session, "\tipv6                     =\n\t");
	for (i=0,j=0; i<16; i++)
	{
		MREAD_I_8((uint8_t *)p, i, r);
		bdmf_session_print(session, "0x%08x ", (unsigned int)r);
		j++;
		if (j >= 8)
		{
			j = 0;
			bdmf_session_print(session, "\n\t");
		}
	}
	bdmf_session_print(session, "\n");
}

void dump_RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register INGRESS_FILTERS_CONFIGURATION_ENTRY\n");

	MREAD_32((uint8_t *)p, r);
	bdmf_session_print(session, "\tingress_filters          = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_BPM_EXTRA_DDR_BUFFERS_BASE(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register BPM_EXTRA_DDR_BUFFERS_BASE\n");

	MREAD_32((uint8_t *)p, r);
	bdmf_session_print(session, "\treserved_fw_only         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_LAYER4_FILTERS_CONTEXT_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register LAYER4_FILTERS_CONTEXT_ENTRY\n");

	FIELD_MREAD_8((uint8_t *)p, 7, 1, r);
	bdmf_session_print(session, "\tcpu_trap                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p, 6, 1, r);
	bdmf_session_print(session, "\tdrop                     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p, 0, 6, r);
	bdmf_session_print(session, "\tparameter                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_FIREWALL_CONFIGURATION_REGISTER_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register FIREWALL_CONFIGURATION_REGISTER_ENTRY\n");

	MREAD_32((uint8_t *)p, r);
	bdmf_session_print(session, "\trules_map_table_address  = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 4, r);
	bdmf_session_print(session, "\trules_table_address      = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_IPTV_DMA_RW_BUFFER(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	int i,j;

	bdmf_session_print(session, "  Register IPTV_DMA_RW_BUFFER\n");

	bdmf_session_print(session, "\treserved_fw_only         =\n\t");
	for (i=0,j=0; i<4; i++)
	{
		MREAD_I_32((uint8_t *)p, i, r);
		bdmf_session_print(session, "0x%08x ", (unsigned int)r);
		j++;
		if (j >= 8)
		{
			j = 0;
			bdmf_session_print(session, "\n\t");
		}
	}
	bdmf_session_print(session, "\n");
}

void dump_RDD_ETH_TX_QUEUE_DUMMY_DESCRIPTOR(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	int i,j;

	bdmf_session_print(session, "  Register ETH_TX_QUEUE_DUMMY_DESCRIPTOR\n");

	bdmf_session_print(session, "\treserved_fw_only         =\n\t");
	for (i=0,j=0; i<4; i++)
	{
		MREAD_I_32((uint8_t *)p, i, r);
		bdmf_session_print(session, "0x%08x ", (unsigned int)r);
		j++;
		if (j >= 8)
		{
			j = 0;
			bdmf_session_print(session, "\n\t");
		}
	}
	bdmf_session_print(session, "\n");
}

void dump_RDD_RUNNER_FLOW_HEADER_DESCRIPTOR(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	int i,j;

	bdmf_session_print(session, "  Register RUNNER_FLOW_HEADER_DESCRIPTOR\n");

	bdmf_session_print(session, "\treserved_fw_only         =\n\t");
	for (i=0,j=0; i<2; i++)
	{
		MREAD_I_32((uint8_t *)p, i, r);
		bdmf_session_print(session, "0x%08x ", (unsigned int)r);
		j++;
		if (j >= 8)
		{
			j = 0;
			bdmf_session_print(session, "\n\t");
		}
	}
	bdmf_session_print(session, "\n");
}

void dump_RDD_MULTICAST_VECTOR_TO_PORT_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register MULTICAST_VECTOR_TO_PORT_ENTRY\n");

	MREAD_8((uint8_t *)p, r);
	bdmf_session_print(session, "\tbridge_port              = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_SC_BUFFER(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	int i,j;

	bdmf_session_print(session, "  Register SC_BUFFER\n");

	bdmf_session_print(session, "\treserved_fw_only         =\n\t");
	for (i=0,j=0; i<3; i++)
	{
		MREAD_I_32((uint8_t *)p, i, r);
		bdmf_session_print(session, "0x%08x ", (unsigned int)r);
		j++;
		if (j >= 8)
		{
			j = 0;
			bdmf_session_print(session, "\n\t");
		}
	}
	bdmf_session_print(session, "\n");
}

void dump_RDD_IPTV_SSM_CONTEXT_TABLE_PTR(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register IPTV_SSM_CONTEXT_TABLE_PTR\n");

	MREAD_32((uint8_t *)p, r);
	bdmf_session_print(session, "\treserved_fw_only         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_MULTICAST_DUMMY_VLAN_INDEXES_TABLE(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	int i,j;

	bdmf_session_print(session, "  Register MULTICAST_DUMMY_VLAN_INDEXES_TABLE\n");

	bdmf_session_print(session, "\treserved_fw_only         =\n\t");
	for (i=0,j=0; i<2; i++)
	{
		MREAD_I_32((uint8_t *)p, i, r);
		bdmf_session_print(session, "0x%08x ", (unsigned int)r);
		j++;
		if (j >= 8)
		{
			j = 0;
			bdmf_session_print(session, "\n\t");
		}
	}
	bdmf_session_print(session, "\n");
}

void dump_RDD_IPTV_PTR_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register IPTV_PTR_ENTRY\n");

	MREAD_32((uint8_t *)p, r);
	bdmf_session_print(session, "\treserved_fw_only         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_IPTV_CONTEXT_PTR_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register IPTV_CONTEXT_PTR_ENTRY\n");

	MREAD_32((uint8_t *)p, r);
	bdmf_session_print(session, "\treserved_fw_only         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_BPM_DDR_OPTIMIZED_BUFFERS_BASE(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register BPM_DDR_OPTIMIZED_BUFFERS_BASE\n");

	MREAD_32((uint8_t *)p, r);
	bdmf_session_print(session, "\treserved_fw_only         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_BPM_DDR_BUFFERS_BASE(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register BPM_DDR_BUFFERS_BASE\n");

	MREAD_32((uint8_t *)p, r);
	bdmf_session_print(session, "\treserved_fw_only         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_CONNECTION_TABLE_CONFIG(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register CONNECTION_TABLE_CONFIG\n");

	MREAD_32((uint8_t *)p, r);
	bdmf_session_print(session, "\treserved_fw_only         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_CONTEXT_TABLE_CONFIG(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register CONTEXT_TABLE_CONFIG\n");

	MREAD_32((uint8_t *)p, r);
	bdmf_session_print(session, "\treserved_fw_only         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

#if defined OREN
void dump_RDD_PARALLEL_PROCESSING_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register PARALLEL_PROCESSING_ENTRY\n");

	MREAD_8((uint8_t *)p, r);
	bdmf_session_print(session, "\tih_buffer                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

#endif

#if defined OREN
void dump_RDD_PARALLEL_PROCESSING_TASK_REORDER_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register PARALLEL_PROCESSING_TASK_REORDER_ENTRY\n");

	MREAD_8((uint8_t *)p, r);
	bdmf_session_print(session, "\treserved_fw_only         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

#endif

#if defined OREN
void dump_RDD_ETH_TX_EMACS_STATUS_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register ETH_TX_EMACS_STATUS_ENTRY\n");

	MREAD_8((uint8_t *)p, r);
	bdmf_session_print(session, "\tstatus_vector            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

#endif

void dump_RDD_TIMER_CONTROL_DESCRIPTOR(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register TIMER_CONTROL_DESCRIPTOR\n");

	MREAD_16((uint8_t *)p, r);
	bdmf_session_print(session, "\tnumber_of_active_tasks   = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_HASH_BASED_FORWARDING_PORT_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register HASH_BASED_FORWARDING_PORT_ENTRY\n");

	MREAD_8((uint8_t *)p, r);
	bdmf_session_print(session, "\tegress_port              = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_IPTV_COUNTERS_BUFFER(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register IPTV_COUNTERS_BUFFER\n");

	MREAD_16((uint8_t *)p, r);
	bdmf_session_print(session, "\treserved_fw_only         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_FIREWALL_IPV6_R16_BUFFER_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register FIREWALL_IPV6_R16_BUFFER_ENTRY\n");

	MREAD_32((uint8_t *)p, r);
	bdmf_session_print(session, "\treserved_fw_only         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_CPU_TX_PICO_INGRESS_QUEUE_PTR(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register CPU_TX_PICO_INGRESS_QUEUE_PTR\n");

	MREAD_16((uint8_t *)p, r);
	bdmf_session_print(session, "\treserved_fw_only         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_FREE_PACKET_DESCRIPTORS_POOL_THRESHOLD(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register FREE_PACKET_DESCRIPTORS_POOL_THRESHOLD\n");

	MREAD_16((uint8_t *)p, r);
	bdmf_session_print(session, "\treserved_fw_only         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_CPU_RX_PICO_INGRESS_QUEUE_PTR(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register CPU_RX_PICO_INGRESS_QUEUE_PTR\n");

	MREAD_16((uint8_t *)p, r);
	bdmf_session_print(session, "\treserved_fw_only         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_BPM_DDR_BUFFER_HEADROOM_SIZE(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register BPM_DDR_BUFFER_HEADROOM_SIZE\n");

	MREAD_16((uint8_t *)p, r);
	bdmf_session_print(session, "\treserved_fw_only         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_BPM_DDR_BUFFER_HEADROOM_SIZE_2_BYTE_RESOLUTION(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register BPM_DDR_BUFFER_HEADROOM_SIZE_2_BYTE_RESOLUTION\n");

	MREAD_16((uint8_t *)p, r);
	bdmf_session_print(session, "\treserved_fw_only         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_CPU_RX_FAST_INGRESS_QUEUE_PTR(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register CPU_RX_FAST_INGRESS_QUEUE_PTR\n");

	MREAD_16((uint8_t *)p, r);
	bdmf_session_print(session, "\treserved_fw_only         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_MICROCODE_VERSION_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register MICROCODE_VERSION_ENTRY\n");

	MREAD_32((uint8_t *)p, r);
	bdmf_session_print(session, "\treserved_fw_only         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_RATE_LIMITER_COUNTER_BUFFER_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register RATE_LIMITER_COUNTER_BUFFER_ENTRY\n");

	MREAD_32((uint8_t *)p, r);
	bdmf_session_print(session, "\treserved_fw_only         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

#if defined OREN
void dump_RDD_PARALLEL_PROCESSING_IH_BUFFER_PTR(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register PARALLEL_PROCESSING_IH_BUFFER_PTR\n");

	MREAD_16((uint8_t *)p, r);
	bdmf_session_print(session, "\treserved_fw_only         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

#endif

void dump_RDD_RUNNER_CONGESTION_STATE_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register RUNNER_CONGESTION_STATE_ENTRY\n");

	MREAD_16((uint8_t *)p, r);
	bdmf_session_print(session, "\treserved_fw_only         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_FIREWALL_RULE_MAP_ENTRY_BUFFER(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register FIREWALL_RULE_MAP_ENTRY_BUFFER\n");

	MREAD_16((uint8_t *)p, r);
	bdmf_session_print(session, "\treserved_fw_only         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_DUMMY_STORE_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register DUMMY_STORE_ENTRY\n");

	MREAD_8((uint8_t *)p, r);
	bdmf_session_print(session, "\tdummy_store              = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

#if defined OREN
void dump_RDD_ETH_TX_INTER_LAN_SCHEDULING_OFFSET_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register ETH_TX_INTER_LAN_SCHEDULING_OFFSET_ENTRY\n");

	MREAD_8((uint8_t *)p, r);
	bdmf_session_print(session, "\temac_offset              = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

#endif

#if defined OREN
void dump_RDD_PARALLEL_PROCESSING_SLAVE_VECTOR(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register PARALLEL_PROCESSING_SLAVE_VECTOR\n");

	FIELD_MREAD_8((uint8_t *)p, 4, 4, r);
	bdmf_session_print(session, "\treserved0                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p, 3, 1, r);
	bdmf_session_print(session, "\tavailable_slave3         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p, 2, 1, r);
	bdmf_session_print(session, "\tavailable_slave2         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p, 1, 1, r);
	bdmf_session_print(session, "\tavailable_slave1         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p, 0, 1, r);
	bdmf_session_print(session, "\tavailable_slave0         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

#endif

void dump_RDD_CPU_TX_DESCRIPTOR(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register CPU_TX_DESCRIPTOR\n");

	MREAD_32((uint8_t *)p, r);
	bdmf_session_print(session, "\treserved0                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 4, r);
	bdmf_session_print(session, "\treserved1                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_CPU_TX_DESCRIPTOR_CORE(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register CPU_TX_DESCRIPTOR_CORE\n");

	FIELD_MREAD_8((uint8_t *)p, 7, 1, r);
	bdmf_session_print(session, "\tvalid                    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p, 4, 3, r);
	bdmf_session_print(session, "\tcommand                  = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p, 3, 1, r);
	bdmf_session_print(session, "\tds_drop_precedence       = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p, 6, 5, r);
	bdmf_session_print(session, "\temac                     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 1, 3, 3, r);
	bdmf_session_print(session, "\ttx_queue                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_32((uint8_t *)p + 0, 14, 5, r);
	bdmf_session_print(session, "\tsrc_bridge_port          = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p + 2, 0, 14, r);
	bdmf_session_print(session, "\tpacket_length            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 4, 7, 1, r);
	bdmf_session_print(session, "\tabs_flag                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 4, 3, 4, r);
	bdmf_session_print(session, "\tssid                     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p + 4, 7, 4, r);
	bdmf_session_print(session, "\tih_class                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 5, 0, 7, r);
	bdmf_session_print(session, "\tpayload_offset           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 6, 7, 1, r);
	bdmf_session_print(session, "\treserved1                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 6, 6, 1, r);
	bdmf_session_print(session, "\tgso                      = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p + 6, 0, 14, r);
	bdmf_session_print(session, "\treserved2                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_CPU_TX_DESCRIPTOR_BPM(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register CPU_TX_DESCRIPTOR_BPM\n");

	MREAD_32((uint8_t *)p, r);
	bdmf_session_print(session, "\treserved0                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_32((uint8_t *)p + 4, 14, 18, r);
	bdmf_session_print(session, "\treserved1                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p + 6, 0, 14, r);
	bdmf_session_print(session, "\tbuffer_number            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_CPU_TX_DESCRIPTOR_ABS(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register CPU_TX_DESCRIPTOR_ABS\n");

	MREAD_32((uint8_t *)p, r);
	bdmf_session_print(session, "\treserved0                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_32((uint8_t *)p + 4, 14, 18, r);
	bdmf_session_print(session, "\treserved1                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p + 6, 0, 14, r);
	bdmf_session_print(session, "\tskb_index                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_CPU_TX_DESCRIPTOR_DS_FAST(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register CPU_TX_DESCRIPTOR_DS_FAST\n");

	FIELD_MREAD_16((uint8_t *)p, 6, 10, r);
	bdmf_session_print(session, "\treserved0                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_32((uint8_t *)p + 0, 14, 8, r);
	bdmf_session_print(session, "\tdownstream_wan_flow      = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p + 2, 0, 14, r);
	bdmf_session_print(session, "\treserved1                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 4, r);
	bdmf_session_print(session, "\treserved2                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_CPU_TX_DESCRIPTOR_US_FAST(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register CPU_TX_DESCRIPTOR_US_FAST\n");

	FIELD_MREAD_8((uint8_t *)p, 4, 4, r);
	bdmf_session_print(session, "\treserved0                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p, 4, 8, r);
	bdmf_session_print(session, "\tupstream_gem_flow        = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 1, 3, 1, r);
	bdmf_session_print(session, "\tus_drop_precedence       = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_32((uint8_t *)p + 0, 0, 19, r);
	bdmf_session_print(session, "\treserved1                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p + 4, 7, 9, r);
	bdmf_session_print(session, "\ttx_queue                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_32((uint8_t *)p + 4, 0, 23, r);
	bdmf_session_print(session, "\treserved2                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_CPU_TX_DESCRIPTOR_DS_PICO(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register CPU_TX_DESCRIPTOR_DS_PICO\n");

	MREAD_32((uint8_t *)p, r);
	bdmf_session_print(session, "\treserved0                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 4, 4, 4, r);
	bdmf_session_print(session, "\treserved1                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 4, 3, 1, r);
	bdmf_session_print(session, "\ten_1588                  = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_32((uint8_t *)p + 4, 0, 27, r);
	bdmf_session_print(session, "\treserved2                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_CPU_TX_DESCRIPTOR_DS_PICO_WIFI(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register CPU_TX_DESCRIPTOR_DS_PICO_WIFI\n");

	MREAD_32((uint8_t *)p, r);
	bdmf_session_print(session, "\treserved0                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 4, 7, 1, r);
	bdmf_session_print(session, "\treserved1                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 4, 6, 1, r);
	bdmf_session_print(session, "\tssid_multicast           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 4, 2, 4, r);
	bdmf_session_print(session, "\tssid                     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_32((uint8_t *)p + 4, 0, 26, r);
	bdmf_session_print(session, "\treserved2                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_CPU_TX_MESSAGE_DESCRIPTOR(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register CPU_TX_MESSAGE_DESCRIPTOR\n");

	FIELD_MREAD_8((uint8_t *)p, 7, 1, r);
	bdmf_session_print(session, "\tvalid                    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p, 4, 3, r);
	bdmf_session_print(session, "\tcommand                  = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_32((uint8_t *)p, 0, 28, r);
	bdmf_session_print(session, "\treserved0                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_32((uint8_t *)p + 4, 4, 28, r);
	bdmf_session_print(session, "\treserved1                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 7, 0, 4, r);
	bdmf_session_print(session, "\tmessage_type             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_CPU_TX_DHD_DESCRIPTOR(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register CPU_TX_DHD_DESCRIPTOR\n");

	MREAD_32((uint8_t *)p, r);
	bdmf_session_print(session, "\treserved0                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 4, 7, 1, r);
	bdmf_session_print(session, "\treserved1                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 4, 3, 4, r);
	bdmf_session_print(session, "\tssid                     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 4, 2, 1, r);
	bdmf_session_print(session, "\tssid_multicast           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 4, 0, 2, r);
	bdmf_session_print(session, "\tradio_idx                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_32((uint8_t *)p + 4, 14, 10, r);
	bdmf_session_print(session, "\tflow_ring_id             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p + 6, 0, 14, r);
	bdmf_session_print(session, "\treserved2                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_CPU_TX_DHD_MESSAGE_DESCRIPTOR(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register CPU_TX_DHD_MESSAGE_DESCRIPTOR\n");

	MREAD_32((uint8_t *)p, r);
	bdmf_session_print(session, "\treserved0                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 4, 6, 2, r);
	bdmf_session_print(session, "\tdhd_msg_type             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 4, 4, 2, r);
	bdmf_session_print(session, "\tradio_idx                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p + 4, 2, 10, r);
	bdmf_session_print(session, "\tflow_ring_id             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 5, 1, 1, r);
	bdmf_session_print(session, "\tdisabled                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 5, 0, 1, r);
	bdmf_session_print(session, "\tread_idx_valid           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p + 6, 6, 10, r);
	bdmf_session_print(session, "\tread_idx                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 7, 0, 6, r);
	bdmf_session_print(session, "\treserved2                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

#if defined OREN
void dump_RDD_ETH_TX_MAC_DESCRIPTOR(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register ETH_TX_MAC_DESCRIPTOR\n");

	MREAD_8((uint8_t *)p, r);
	bdmf_session_print(session, "\tegress_counter           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 1, r);
	bdmf_session_print(session, "\tloopback_mode            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 2, r);
	bdmf_session_print(session, "\treserved7                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 4, r);
	bdmf_session_print(session, "\tingress_counter          = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 5, r);
	bdmf_session_print(session, "\trate_limiter_id          = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 6, r);
	bdmf_session_print(session, "\ttx_task_number           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	{ uint32_t temp; FIELD_MREAD_32(((uint8_t *)p + 4), 0, 8, temp); r = temp << 24; FIELD_MREAD_32(((uint8_t *)p + 8), 8, 24, temp); r = r | temp; };
	bdmf_session_print(session, "\treserved8                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	{ uint32_t temp; FIELD_MREAD_32(((uint8_t *)p + 8), 0, 8, temp); r = temp << 24; FIELD_MREAD_32(((uint8_t *)p + 12), 8, 24, temp); r = r | temp; };
	bdmf_session_print(session, "\treserved9                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 15, r);
	bdmf_session_print(session, "\treserved10               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 16, r);
	bdmf_session_print(session, "\treserved11               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 20, r);
	bdmf_session_print(session, "\treserved12               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 24, r);
	bdmf_session_print(session, "\treserved13               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 28, r);
	bdmf_session_print(session, "\treserved14               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 32, r);
	bdmf_session_print(session, "\temac_mask                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 33, r);
	bdmf_session_print(session, "\ttx_queues_status         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 34, r);
	bdmf_session_print(session, "\tpacket_counters_ptr      = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 36, r);
	bdmf_session_print(session, "\tgpio_flow_control_vector_ptr= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 38, r);
	bdmf_session_print(session, "\treserved0                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 40, r);
	bdmf_session_print(session, "\treserved1                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 44, r);
	bdmf_session_print(session, "\treserved2                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 48, r);
	bdmf_session_print(session, "\treserved3                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 52, r);
	bdmf_session_print(session, "\treserved4                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 56, r);
	bdmf_session_print(session, "\treserved5                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 60, r);
	bdmf_session_print(session, "\treserved6                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

#endif

void dump_RDD_FREE_PACKET_DESCRIPTORS_POOL_DESCRIPTOR_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register FREE_PACKET_DESCRIPTORS_POOL_DESCRIPTOR_ENTRY\n");

	MREAD_16((uint8_t *)p, r);
	bdmf_session_print(session, "\thead_pointer             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 2, r);
	bdmf_session_print(session, "\ttail_pointer             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 4, r);
	bdmf_session_print(session, "\tegress_counter           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 6, r);
	bdmf_session_print(session, "\tingress_counter          = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_BBH_RX_DESCRIPTOR(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register BBH_RX_DESCRIPTOR\n");

	FIELD_MREAD_16((uint8_t *)p, 6, 10, r);
	bdmf_session_print(session, "\tlast_sbn                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_32((uint8_t *)p + 0, 14, 8, r);
	bdmf_session_print(session, "\tflow_id                  = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p + 2, 0, 14, r);
	bdmf_session_print(session, "\tpacket_length            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 4, 7, 1, r);
	bdmf_session_print(session, "\terror                    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 4, 6, 1, r);
	bdmf_session_print(session, "\tploam                    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p + 4, 6, 8, r);
	bdmf_session_print(session, "\terror_type               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 5, 0, 6, r);
	bdmf_session_print(session, "\tih_buffer_number         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 6, 7, 1, r);
	bdmf_session_print(session, "\ttarget_memory            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 6, 6, 1, r);
	bdmf_session_print(session, "\treserved0                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p + 6, 0, 14, r);
	bdmf_session_print(session, "\tbuffer_number            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_PCI_TX_FIFO_DESCRIPTOR_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register PCI_TX_FIFO_DESCRIPTOR_ENTRY\n");

	MREAD_16((uint8_t *)p, r);
	bdmf_session_print(session, "\thead_ptr                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 2, r);
	bdmf_session_print(session, "\tegress_counter           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 4, r);
	bdmf_session_print(session, "\ttail_ptr                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 6, r);
	bdmf_session_print(session, "\tingress_counter          = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_PCI_TX_QUEUES_VECTOR(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register PCI_TX_QUEUES_VECTOR\n");

	MREAD_32((uint8_t *)p, r);
	bdmf_session_print(session, "\treserved_fw_only         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_PCI_TX_FIFO_FULL_VECTOR_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register PCI_TX_FIFO_FULL_VECTOR_ENTRY\n");

	FIELD_MREAD_8((uint8_t *)p, 4, 4, r);
	bdmf_session_print(session, "\treserved0                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p, 3, 1, r);
	bdmf_session_print(session, "\tfifo_3_not_full_bit      = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p, 2, 1, r);
	bdmf_session_print(session, "\tfifo_2_not_full_bit      = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p, 1, 1, r);
	bdmf_session_print(session, "\tfifo_1_not_full_bit      = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p, 0, 1, r);
	bdmf_session_print(session, "\tfifo_0_not_full_bit      = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_RUNNER_FLOW_IH_RESPONSE(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	int i,j;

	bdmf_session_print(session, "  Register RUNNER_FLOW_IH_RESPONSE\n");

	bdmf_session_print(session, "\treserved_fw_only         =\n\t");
	for (i=0,j=0; i<2; i++)
	{
		MREAD_I_32((uint8_t *)p, i, r);
		bdmf_session_print(session, "0x%08x ", (unsigned int)r);
		j++;
		if (j >= 8)
		{
			j = 0;
			bdmf_session_print(session, "\n\t");
		}
	}
	bdmf_session_print(session, "\n");
}

void dump_RDD_DOWNSTREAM_DMA_PIPE_BUFFER(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register DOWNSTREAM_DMA_PIPE_BUFFER\n");

	MREAD_32((uint8_t *)p, r);
	bdmf_session_print(session, "\treserved_fw_only         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_US_INGRESS_CLASSIFICATION_CONTEXT_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register US_INGRESS_CLASSIFICATION_CONTEXT_ENTRY\n");

	FIELD_MREAD_8((uint8_t *)p, 1, 7, r);
	bdmf_session_print(session, "\tvlan_cmd_index           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p, 0, 1, r);
	bdmf_session_print(session, "\tqos_rule_match           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 1, 7, 1, r);
	bdmf_session_print(session, "\tqos_rule_overrun_wan_flow_mode= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 1, 6, 1, r);
	bdmf_session_print(session, "\twan_flow_mapping_mode    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 1, 3, 3, r);
	bdmf_session_print(session, "\twan_flow_mapping_table   = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 1, 2, 1, r);
	bdmf_session_print(session, "\topbit_remark_mode        = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 1, 1, 1, r);
	bdmf_session_print(session, "\tipbit_remark_mode        = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_32((uint8_t *)p + 0, 9, 8, r);
	bdmf_session_print(session, "\twan_flow                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 2, 0, 1, r);
	bdmf_session_print(session, "\tqos_mapping_mode         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 3, 5, 3, r);
	bdmf_session_print(session, "\ttraffic_class            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 3, 0, 5, r);
	bdmf_session_print(session, "\trate_controller          = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 4, 7, 1, r);
	bdmf_session_print(session, "\tdrop                     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 4, 6, 1, r);
	bdmf_session_print(session, "\tcpu                      = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 4, 5, 1, r);
	bdmf_session_print(session, "\tic_ip_flow               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 4, 0, 5, r);
	bdmf_session_print(session, "\treserved1                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 5, 7, 1, r);
	bdmf_session_print(session, "\tdei_remark_enable        = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 5, 6, 1, r);
	bdmf_session_print(session, "\tdei_value                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 5, 5, 1, r);
	bdmf_session_print(session, "\tpolicer_mode             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 5, 1, 4, r);
	bdmf_session_print(session, "\tpolicer_id               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_32((uint8_t *)p + 4, 15, 2, r);
	bdmf_session_print(session, "\treserved2                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 6, 4, 3, r);
	bdmf_session_print(session, "\touter_pbit               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 6, 1, 3, r);
	bdmf_session_print(session, "\tinner_pbit               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 6, 0, 1, r);
	bdmf_session_print(session, "\tdscp_remarking_mode      = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 7, 2, 6, r);
	bdmf_session_print(session, "\tdscp                     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 7, 0, 2, r);
	bdmf_session_print(session, "\tecn                      = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_WAN_CHANNEL_8_39_DESCRIPTOR(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	int i,j;

	bdmf_session_print(session, "  Register WAN_CHANNEL_8_39_DESCRIPTOR\n");

	FIELD_MREAD_8((uint8_t *)p, 2, 6, r);
	bdmf_session_print(session, "\treserved0                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p, 1, 1, r);
	bdmf_session_print(session, "\trate_limiter_priority    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p, 0, 1, r);
	bdmf_session_print(session, "\trate_limiter_mode        = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 1, r);
	bdmf_session_print(session, "\tschedule                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 2, r);
	bdmf_session_print(session, "\tbbh_destination          = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 4, r);
	bdmf_session_print(session, "\trate_controllers_status  = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 8, r);
	bdmf_session_print(session, "\trate_controllers_sustain_vector= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 12, r);
	bdmf_session_print(session, "\trate_controllers_peak_vector= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 16, r);
	bdmf_session_print(session, "\tpeak_burst_counter       = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 18, r);
	bdmf_session_print(session, "\tpeak_offset              = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 19, 7, 1, r);
	bdmf_session_print(session, "\tpeak_scheduling_mode     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 19, 2, 5, r);
	bdmf_session_print(session, "\treserved1                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 19, 1, 1, r);
	bdmf_session_print(session, "\tack_pending_epon         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 19, 0, 1, r);
	bdmf_session_print(session, "\tack_pending              = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 20, r);
	bdmf_session_print(session, "\tbyte_counter             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	bdmf_session_print(session, "\trate_controller_addr     =\n\t");
	for (i=0,j=0; i<4; i++)
	{
		MREAD_I_16((uint8_t *)p + 24, i, r);
		bdmf_session_print(session, "0x%08x ", (unsigned int)r);
		j++;
		if (j >= 8)
		{
			j = 0;
			bdmf_session_print(session, "\n\t");
		}
	}
	bdmf_session_print(session, "\n");
}

void dump_RDD_US_QUEUE_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register US_QUEUE_ENTRY\n");

	FIELD_MREAD_8((uint8_t *)p, 5, 3, r);
	bdmf_session_print(session, "\tqueue                    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p, 0, 5, r);
	bdmf_session_print(session, "\trate_controller          = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_US_WAN_FLOW_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register US_WAN_FLOW_ENTRY\n");

	FIELD_MREAD_8((uint8_t *)p, 4, 4, r);
	bdmf_session_print(session, "\treserved0                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p, 1, 3, r);
	bdmf_session_print(session, "\thdr_type                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p, 6, 3, r);
	bdmf_session_print(session, "\ttraffic_class_to_queue_table_index= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 1, 0, 6, r);
	bdmf_session_print(session, "\twan_channel_id           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 2, 5, 3, r);
	bdmf_session_print(session, "\tpbits_to_queue_table_index= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 2, 4, 1, r);
	bdmf_session_print(session, "\tcrc_calc                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p + 2, 0, 12, r);
	bdmf_session_print(session, "\twan_port_id              = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_WAN_CHANNEL_0_7_DESCRIPTOR(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	int i,j;

	bdmf_session_print(session, "  Register WAN_CHANNEL_0_7_DESCRIPTOR\n");

	FIELD_MREAD_8((uint8_t *)p, 2, 6, r);
	bdmf_session_print(session, "\treserved0                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p, 1, 1, r);
	bdmf_session_print(session, "\trate_limiter_priority    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p, 0, 1, r);
	bdmf_session_print(session, "\trate_limiter_mode        = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 1, r);
	bdmf_session_print(session, "\tschedule                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 2, r);
	bdmf_session_print(session, "\tbbh_destination          = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 4, r);
	bdmf_session_print(session, "\trate_controllers_status  = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 8, r);
	bdmf_session_print(session, "\trate_controllers_sustain_vector= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 12, r);
	bdmf_session_print(session, "\trate_controllers_peak_vector= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 16, r);
	bdmf_session_print(session, "\tpeak_burst_counter       = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 18, r);
	bdmf_session_print(session, "\tpeak_offset              = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 19, 7, 1, r);
	bdmf_session_print(session, "\tpeak_scheduling_mode     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 19, 2, 5, r);
	bdmf_session_print(session, "\treserved1                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 19, 1, 1, r);
	bdmf_session_print(session, "\tack_pending_epon         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 19, 0, 1, r);
	bdmf_session_print(session, "\tack_pending              = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 20, r);
	bdmf_session_print(session, "\tbyte_counter             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	bdmf_session_print(session, "\trate_controller_addr     =\n\t");
	for (i=0,j=0; i<32; i++)
	{
		MREAD_I_16((uint8_t *)p + 24, i, r);
		bdmf_session_print(session, "0x%08x ", (unsigned int)r);
		j++;
		if (j >= 8)
		{
			j = 0;
			bdmf_session_print(session, "\n\t");
		}
	}
	bdmf_session_print(session, "\n");
}

void dump_RDD_SMART_CARD_DESCRIPTOR_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	int i,j;

	bdmf_session_print(session, "  Register SMART_CARD_DESCRIPTOR_ENTRY\n");

	MREAD_32((uint8_t *)p, r);
	bdmf_session_print(session, "\twaiting_time             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 4, r);
	bdmf_session_print(session, "\tguard_time               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 5, r);
	bdmf_session_print(session, "\tetu                      = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 6, r);
	bdmf_session_print(session, "\tsampling_time            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 7, r);
	bdmf_session_print(session, "\tmax_retransmit           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 8, r);
	bdmf_session_print(session, "\ttask_type                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 9, r);
	bdmf_session_print(session, "\tstatus_byte              = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 10, r);
	bdmf_session_print(session, "\treserved                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 11, r);
	bdmf_session_print(session, "\theader_bytes             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 12, r);
	bdmf_session_print(session, "\tsend_length              = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 14, r);
	bdmf_session_print(session, "\treceive_length           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	bdmf_session_print(session, "\tdata_bytes               =\n\t");
	for (i=0,j=0; i<256; i++)
	{
		MREAD_I_8((uint8_t *)p + 16, i, r);
		bdmf_session_print(session, "0x%08x ", (unsigned int)r);
		j++;
		if (j >= 8)
		{
			j = 0;
			bdmf_session_print(session, "\n\t");
		}
	}
	bdmf_session_print(session, "\n");
}

void dump_RDD_BUDGET_ALLOCATOR_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register BUDGET_ALLOCATOR_ENTRY\n");

	MREAD_16((uint8_t *)p, r);
	bdmf_session_print(session, "\tactive_rate_controllers  = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_GPON_ABSOLUTE_TX_COUNTER(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register GPON_ABSOLUTE_TX_COUNTER\n");

	MREAD_8((uint8_t *)p, r);
	bdmf_session_print(session, "\treserved_fw_only         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_INGRESS_RATE_LIMITER_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register INGRESS_RATE_LIMITER_ENTRY\n");

	MREAD_32((uint8_t *)p, r);
	bdmf_session_print(session, "\tcurrent_budget           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 4, r);
	bdmf_session_print(session, "\tdrop_threshold           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 8, r);
	bdmf_session_print(session, "\txoff_threshold           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p + 12, 2, 14, r);
	bdmf_session_print(session, "\tallocated_budget_mantisa = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 13, 0, 2, r);
	bdmf_session_print(session, "\tallocated_budget_exponent= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 14, r);
	bdmf_session_print(session, "\tactive_pause_flag        = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 15, r);
	bdmf_session_print(session, "\tbbh_rx_address           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_ETH_RX_DESCRIPTORS(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	int i,j;

	bdmf_session_print(session, "  Register ETH_RX_DESCRIPTORS\n");

	bdmf_session_print(session, "\treserved_fw_only         =\n\t");
	for (i=0,j=0; i<2; i++)
	{
		MREAD_I_32((uint8_t *)p, i, r);
		bdmf_session_print(session, "0x%08x ", (unsigned int)r);
		j++;
		if (j >= 8)
		{
			j = 0;
			bdmf_session_print(session, "\n\t");
		}
	}
	bdmf_session_print(session, "\n");
}

void dump_RDD_SPEED_SERVICE_PARAMETERS(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register SPEED_SERVICE_PARAMETERS\n");

	MREAD_32((uint8_t *)p, r);
	bdmf_session_print(session, "\tbbh_descriptor_0         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 4, r);
	bdmf_session_print(session, "\tbbh_descriptor_1         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 8, r);
	bdmf_session_print(session, "\ttimer_period             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 10, r);
	bdmf_session_print(session, "\tskb_free_index           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 12, r);
	bdmf_session_print(session, "\tcopies_in_transit        = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 16, r);
	bdmf_session_print(session, "\ttotal_copies             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 20, r);
	bdmf_session_print(session, "\ttx_queue_discards        = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 24, r);
	bdmf_session_print(session, "\ttx_queue_writes          = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 28, r);
	bdmf_session_print(session, "\ttx_queue_reads           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 32, r);
	bdmf_session_print(session, "\tbucket                   = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 36, r);
	bdmf_session_print(session, "\tbucket_size              = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 38, r);
	bdmf_session_print(session, "\ttotal_length             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 40, r);
	bdmf_session_print(session, "\ttokens                   = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_SMART_CARD_ERROR_COUNTERS_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register SMART_CARD_ERROR_COUNTERS_ENTRY\n");

	MREAD_32((uint8_t *)p, r);
	bdmf_session_print(session, "\treceive_errors           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 4, r);
	bdmf_session_print(session, "\ttransmit_errors          = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_US_OVERALL_RATE_LIMITER_WAN_CHANNEL_PTR_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register US_OVERALL_RATE_LIMITER_WAN_CHANNEL_PTR_ENTRY\n");

	MREAD_16((uint8_t *)p, r);
	bdmf_session_print(session, "\twan_channel_ptr          = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_LAN_INGRESS_FIFO_DESCRIPTOR_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register LAN_INGRESS_FIFO_DESCRIPTOR_ENTRY\n");

	MREAD_16((uint8_t *)p, r);
	bdmf_session_print(session, "\tingress_fifo_ptr         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 2, r);
	bdmf_session_print(session, "\tbroadcom_switch_task_wakeup_value= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_US_INGRESS_CLASSIFICATION_DEFAULT_FLOWS_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register US_INGRESS_CLASSIFICATION_DEFAULT_FLOWS_ENTRY\n");

	MREAD_8((uint8_t *)p, r);
	bdmf_session_print(session, "\tflow_id                  = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_IPV6_LOCAL_IP(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	int i,j;

	bdmf_session_print(session, "  Register IPV6_LOCAL_IP\n");

	bdmf_session_print(session, "\treserved_fw_only         =\n\t");
	for (i=0,j=0; i<14; i++)
	{
		MREAD_I_8((uint8_t *)p, i, r);
		bdmf_session_print(session, "0x%08x ", (unsigned int)r);
		j++;
		if (j >= 8)
		{
			j = 0;
			bdmf_session_print(session, "\n\t");
		}
	}
	bdmf_session_print(session, "\n");
}

void dump_RDD_BROADCOM_SWITCH_PORT_MAPPING(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register BROADCOM_SWITCH_PORT_MAPPING\n");

	MREAD_8((uint8_t *)p, r);
	bdmf_session_print(session, "\tphysical_port            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_VLAN_SWITCHING_ISOLATION_CONFIG_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register VLAN_SWITCHING_ISOLATION_CONFIG_ENTRY\n");

	FIELD_MREAD_8((uint8_t *)p, 2, 6, r);
	bdmf_session_print(session, "\treserved                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p, 1, 1, r);
	bdmf_session_print(session, "\tegress_cfg               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p, 0, 1, r);
	bdmf_session_print(session, "\tingress_cfg              = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_LOCAL_SWITCHING_MULTICAST_LAN_ENQUEUE_INGRESS_QUEUE_PTR(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register LOCAL_SWITCHING_MULTICAST_LAN_ENQUEUE_INGRESS_QUEUE_PTR\n");

	MREAD_16((uint8_t *)p, r);
	bdmf_session_print(session, "\tentry                    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_LOCAL_SWITCHING_LAN_ENQUEUE_INGRESS_QUEUE_PTR(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register LOCAL_SWITCHING_LAN_ENQUEUE_INGRESS_QUEUE_PTR\n");

	MREAD_16((uint8_t *)p, r);
	bdmf_session_print(session, "\treserved_fw_only         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_BBH_TX_WAN_CHANNEL_INDEX(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	int i,j;

	bdmf_session_print(session, "  Register BBH_TX_WAN_CHANNEL_INDEX\n");

	bdmf_session_print(session, "\treserved_fw_only         =\n\t");
	for (i=0,j=0; i<2; i++)
	{
		MREAD_I_32((uint8_t *)p, i, r);
		bdmf_session_print(session, "0x%08x ", (unsigned int)r);
		j++;
		if (j >= 8)
		{
			j = 0;
			bdmf_session_print(session, "\n\t");
		}
	}
	bdmf_session_print(session, "\n");
}

void dump_RDD_MAC_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register MAC_ENTRY\n");

	FIELD_MREAD_16((uint8_t *)p, 4, 12, r);
	bdmf_session_print(session, "\tuser_defined             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_32((uint8_t *)p + 0, 12, 8, r);
	bdmf_session_print(session, "\tmac_addr0                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p + 2, 4, 8, r);
	bdmf_session_print(session, "\tmac_addr1                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	{ uint32_t temp; FIELD_MREAD_32((uint8_t *)p, 0, 4, temp); r = temp << 4; FIELD_MREAD_32(((uint8_t *)p + 4), 28, 4, temp); r = r | temp; };
	bdmf_session_print(session, "\tmac_addr2                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p + 4, 4, 8, r);
	bdmf_session_print(session, "\tmac_addr3                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_32((uint8_t *)p + 4, 12, 8, r);
	bdmf_session_print(session, "\tmac_addr4                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p + 6, 4, 8, r);
	bdmf_session_print(session, "\tmac_addr5                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 7, 3, 1, r);
	bdmf_session_print(session, "\treserved                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 7, 2, 1, r);
	bdmf_session_print(session, "\taging                    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 7, 1, 1, r);
	bdmf_session_print(session, "\tskip                     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 7, 0, 1, r);
	bdmf_session_print(session, "\tvalid                    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_IPTV_L2_LOOKUP_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register IPTV_L2_LOOKUP_ENTRY\n");

	FIELD_MREAD_16((uint8_t *)p, 4, 12, r);
	bdmf_session_print(session, "\tvid                      = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_32((uint8_t *)p + 0, 12, 8, r);
	bdmf_session_print(session, "\tmac_addr0                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p + 2, 4, 8, r);
	bdmf_session_print(session, "\tmac_addr1                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	{ uint32_t temp; FIELD_MREAD_32((uint8_t *)p, 0, 4, temp); r = temp << 4; FIELD_MREAD_32(((uint8_t *)p + 4), 28, 4, temp); r = r | temp; };
	bdmf_session_print(session, "\tmac_addr2                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p + 4, 4, 8, r);
	bdmf_session_print(session, "\tmac_addr3                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_32((uint8_t *)p + 4, 12, 8, r);
	bdmf_session_print(session, "\tmac_addr4                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p + 6, 4, 8, r);
	bdmf_session_print(session, "\tmac_addr5                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 7, 3, 1, r);
	bdmf_session_print(session, "\texcl                     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 7, 2, 1, r);
	bdmf_session_print(session, "\taging                    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 7, 1, 1, r);
	bdmf_session_print(session, "\tskip                     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 7, 0, 1, r);
	bdmf_session_print(session, "\tvalid                    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_IPTV_L3_LOOKUP_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register IPTV_L3_LOOKUP_ENTRY\n");

	FIELD_MREAD_8((uint8_t *)p, 4, 4, r);
	bdmf_session_print(session, "\treserved                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p, 2, 10, r);
	bdmf_session_print(session, "\tcontext_table            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 1, 1, 1, r);
	bdmf_session_print(session, "\tcontext_valid            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 1, 0, 1, r);
	bdmf_session_print(session, "\tany                      = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p + 2, 4, 12, r);
	bdmf_session_print(session, "\tvid                      = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	{ uint32_t temp; FIELD_MREAD_32((uint8_t *)p, 0, 4, temp); r = temp << 4; FIELD_MREAD_32(((uint8_t *)p + 4), 28, 4, temp); r = r | temp; };
	bdmf_session_print(session, "\tdest_ip0                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p + 4, 4, 8, r);
	bdmf_session_print(session, "\tdest_ip1                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_32((uint8_t *)p + 4, 12, 8, r);
	bdmf_session_print(session, "\tdest_ip2                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p + 6, 4, 8, r);
	bdmf_session_print(session, "\tdest_ip3                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 7, 3, 1, r);
	bdmf_session_print(session, "\texcl                     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 7, 2, 1, r);
	bdmf_session_print(session, "\taging                    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 7, 1, 1, r);
	bdmf_session_print(session, "\tskip                     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 7, 0, 1, r);
	bdmf_session_print(session, "\tvalid                    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_MAC_CONTEXT_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register MAC_CONTEXT_ENTRY\n");

	FIELD_MREAD_8((uint8_t *)p, 7, 1, r);
	bdmf_session_print(session, "\tmulticast                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p, 5, 2, r);
	bdmf_session_print(session, "\tmulticast_vector         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p, 0, 5, r);
	bdmf_session_print(session, "\tegress_port              = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 1, 7, 1, r);
	bdmf_session_print(session, "\tmove_indication          = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 1, 6, 1, r);
	bdmf_session_print(session, "\tmac_type                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 1, 3, 3, r);
	bdmf_session_print(session, "\tda_action                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 1, 0, 3, r);
	bdmf_session_print(session, "\tsa_action                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

#if defined OREN
void dump_RDD_IPTV_CONTEXT_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register IPTV_CONTEXT_ENTRY\n");

	MREAD_16((uint8_t *)p, r);
	bdmf_session_print(session, "\tport_vector_info         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

#endif

void dump_RDD_DDR_QUEUE_DESCRIPTOR(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register DDR_QUEUE_DESCRIPTOR\n");

	MREAD_16((uint8_t *)p, r);
	bdmf_session_print(session, "\thead_idx                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 2, r);
	bdmf_session_print(session, "\ttail_idx                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 4, r);
	bdmf_session_print(session, "\tpacket_counter           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 6, 4, 4, r);
	bdmf_session_print(session, "\tvalid_entries_number     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p + 6, 6, 6, r);
	bdmf_session_print(session, "\ttail_entry               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 7, 0, 6, r);
	bdmf_session_print(session, "\thead_entry               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 8, r);
	bdmf_session_print(session, "\tpacket_threshold         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 10, 7, 1, r);
	bdmf_session_print(session, "\tqueue_state              = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 10, 4, 3, r);
	bdmf_session_print(session, "\tprofile_id               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p + 10, 6, 6, r);
	bdmf_session_print(session, "\ttail_base_entry          = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 11, 0, 6, r);
	bdmf_session_print(session, "\thead_base_entry          = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 12, 7, 1, r);
	bdmf_session_print(session, "\tprofile_en               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 12, 0, 7, r);
	bdmf_session_print(session, "\tunion_field1             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 13, r);
	bdmf_session_print(session, "\tqueue_mask               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 14, r);
	bdmf_session_print(session, "\tcache_ptr                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_INTERRUPT_COALESCING_CONFIG(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register INTERRUPT_COALESCING_CONFIG\n");

	FIELD_MREAD_16((uint8_t *)p, 6, 10, r);
	bdmf_session_print(session, "\tcurrent_timeout          = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 1, 0, 6, r);
	bdmf_session_print(session, "\tcurrent_packet_count     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p + 2, 6, 10, r);
	bdmf_session_print(session, "\tconfigured_timeout       = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 3, 0, 6, r);
	bdmf_session_print(session, "\tconfigured_max_packet_count= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_INTERRUPT_COALESCING_TIMER_CONFIG(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register INTERRUPT_COALESCING_TIMER_CONFIG\n");

	MREAD_16((uint8_t *)p, r);
	bdmf_session_print(session, "\ttimer_period             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 2, r);
	bdmf_session_print(session, "\ttimer_armed              = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_FREE_SKB_INDEXES_FIFO_TAIL(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register FREE_SKB_INDEXES_FIFO_TAIL\n");

	MREAD_32((uint8_t *)p, r);
	bdmf_session_print(session, "\treserved_fw_only         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_PBITS_TO_QOS_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register PBITS_TO_QOS_ENTRY\n");

	MREAD_8((uint8_t *)p, r);
	bdmf_session_print(session, "\tqos                      = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_CPU_REASON_TO_CPU_RX_QUEUE_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register CPU_REASON_TO_CPU_RX_QUEUE_ENTRY\n");

	MREAD_8((uint8_t *)p, r);
	bdmf_session_print(session, "\tcpu_rx_queue             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_BPM_CONGESTION_CONTROL_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register BPM_CONGESTION_CONTROL_ENTRY\n");

	MREAD_32((uint8_t *)p, r);
	bdmf_session_print(session, "\tbpm_counter              = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 4, r);
	bdmf_session_print(session, "\tbpm_threshold            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 8, r);
	bdmf_session_print(session, "\tbpm_high_priority_threshold= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_IPTV_L3_SRC_IP_LOOKUP_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register IPTV_L3_SRC_IP_LOOKUP_ENTRY\n");

	FIELD_MREAD_8((uint8_t *)p, 4, 4, r);
	bdmf_session_print(session, "\tsrc_ip_12                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p, 4, 8, r);
	bdmf_session_print(session, "\tsrc_ip_13                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_32((uint8_t *)p + 0, 12, 8, r);
	bdmf_session_print(session, "\tsrc_ip_14                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p + 2, 4, 8, r);
	bdmf_session_print(session, "\tsrc_ip_15                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	{ uint32_t temp; FIELD_MREAD_32((uint8_t *)p, 0, 4, temp); r = temp << 4; FIELD_MREAD_32(((uint8_t *)p + 4), 28, 4, temp); r = r | temp; };
	bdmf_session_print(session, "\tsrc_ip_0                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p + 4, 4, 8, r);
	bdmf_session_print(session, "\tsrc_ip_1                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_32((uint8_t *)p + 4, 12, 8, r);
	bdmf_session_print(session, "\tsrc_ip_2                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p + 6, 4, 8, r);
	bdmf_session_print(session, "\tsrc_ip_3                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 7, 3, 1, r);
	bdmf_session_print(session, "\texcl                     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 7, 2, 1, r);
	bdmf_session_print(session, "\taging                    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 7, 1, 1, r);
	bdmf_session_print(session, "\tskip                     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 7, 0, 1, r);
	bdmf_session_print(session, "\tvalid                    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_LAN_VID_CONTEXT_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register LAN_VID_CONTEXT_ENTRY\n");

	FIELD_MREAD_8((uint8_t *)p, 6, 2, r);
	bdmf_session_print(session, "\treserved0                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p, 0, 6, r);
	bdmf_session_print(session, "\tisolation_mode_port_vector= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 1, 6, 2, r);
	bdmf_session_print(session, "\taggregated_vid_idx       = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 1, 0, 6, r);
	bdmf_session_print(session, "\taggregation_mode_port_vector= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_RUNNER_SCRATCHPAD(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	int i,j;

	bdmf_session_print(session, "  Register RUNNER_SCRATCHPAD\n");

	bdmf_session_print(session, "\treserved_fw_only         =\n\t");
	for (i=0,j=0; i<64; i++)
	{
		MREAD_I_32((uint8_t *)p, i, r);
		bdmf_session_print(session, "0x%08x ", (unsigned int)r);
		j++;
		if (j >= 8)
		{
			j = 0;
			bdmf_session_print(session, "\n\t");
		}
	}
	bdmf_session_print(session, "\n");
}

void dump_RDD_CPU_RX_DESCRIPTOR(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register CPU_RX_DESCRIPTOR\n");

	FIELD_MREAD_16((uint8_t *)p, 4, 12, r);
	bdmf_session_print(session, "\tflow_id                  = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 1, 3, 1, r);
	bdmf_session_print(session, "\treserved0                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_32((uint8_t *)p + 0, 14, 5, r);
	bdmf_session_print(session, "\tsource_bridge_port       = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p + 2, 0, 14, r);
	bdmf_session_print(session, "\tpacket_length            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 4, 7, 1, r);
	bdmf_session_print(session, "\tpayload_offset_flag      = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 4, 1, 6, r);
	bdmf_session_print(session, "\treason                   = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_32((uint8_t *)p + 4, 9, 16, r);
	bdmf_session_print(session, "\tdst_ssid                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p + 6, 4, 5, r);
	bdmf_session_print(session, "\treserved2                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 7, 0, 4, r);
	bdmf_session_print(session, "\tdescriptor_type          = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 8, 7, 1, r);
	bdmf_session_print(session, "\townership                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 8, 5, 2, r);
	bdmf_session_print(session, "\treserved3                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_32((uint8_t *)p + 8, 0, 29, r);
	bdmf_session_print(session, "\thost_data_buffer_pointer = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 12, 7, 1, r);
	bdmf_session_print(session, "\treserved4                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 12, 6, 1, r);
	bdmf_session_print(session, "\tis_ucast                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 12, 2, 4, r);
	bdmf_session_print(session, "\twl_tx_prio               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p + 12, 4, 6, r);
	bdmf_session_print(session, "\treserved5                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 13, 0, 4, r);
	bdmf_session_print(session, "\tip_sync_1588_entry_index = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 14, r);
	bdmf_session_print(session, "\twl_info                  = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_MAC_EXTENSION_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register MAC_EXTENSION_ENTRY\n");

	MREAD_8((uint8_t *)p, r);
	bdmf_session_print(session, "\textension_entry          = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_SERVICE_QUEUES_CFG_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register SERVICE_QUEUES_CFG_ENTRY\n");

	FIELD_MREAD_8((uint8_t *)p, 7, 1, r);
	bdmf_session_print(session, "\tsustain_scheduling_mode  = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p, 6, 1, r);
	bdmf_session_print(session, "\tpeak_scheduling_mode     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p, 5, 1, r);
	bdmf_session_print(session, "\toverall_rate_limiter_mode= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p, 0, 13, r);
	bdmf_session_print(session, "\treserved                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 2, r);
	bdmf_session_print(session, "\tpeak_ffi_offset          = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 3, r);
	bdmf_session_print(session, "\tsustain_ffi_offset       = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 4, r);
	bdmf_session_print(session, "\tservice_queues_status    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 8, r);
	bdmf_session_print(session, "\trate_limiter_status      = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 12, r);
	bdmf_session_print(session, "\tsustain_vector           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 16, r);
	bdmf_session_print(session, "\tpeak_vector              = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_MAC_UNKNOWN_DA_FORWARDING_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register MAC_UNKNOWN_DA_FORWARDING_ENTRY\n");

	MREAD_8((uint8_t *)p, r);
	bdmf_session_print(session, "\tflood_bridge_port        = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 1, r);
	bdmf_session_print(session, "\tbridge_port              = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 2, r);
	bdmf_session_print(session, "\tssid_vector              = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 4, 6, 2, r);
	bdmf_session_print(session, "\treserved1                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 4, 5, 1, r);
	bdmf_session_print(session, "\tpolicer_enable           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 4, 1, 4, r);
	bdmf_session_print(session, "\tpolicer_id               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 4, 0, 1, r);
	bdmf_session_print(session, "\tfilter_enable            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_32((uint8_t *)p + 4, 0, 24, r);
	bdmf_session_print(session, "\tmac_prefix               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_BPM_REPLY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	int i,j;

	bdmf_session_print(session, "  Register BPM_REPLY\n");

	bdmf_session_print(session, "\treserved_fw_only         =\n\t");
	for (i=0,j=0; i<12; i++)
	{
		MREAD_I_32((uint8_t *)p, i, r);
		bdmf_session_print(session, "0x%08x ", (unsigned int)r);
		j++;
		if (j >= 8)
		{
			j = 0;
			bdmf_session_print(session, "\n\t");
		}
	}
	bdmf_session_print(session, "\n");
}

void dump_RDD_DSCP_TO_PBITS_DEI_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register DSCP_TO_PBITS_DEI_ENTRY\n");

	FIELD_MREAD_8((uint8_t *)p, 4, 4, r);
	bdmf_session_print(session, "\treserved1                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p, 1, 3, r);
	bdmf_session_print(session, "\tpbits                    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p, 0, 1, r);
	bdmf_session_print(session, "\tdei                      = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_GPON_SKB_ENQUEUED_INDEXES_FIFO_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	int i,j;

	bdmf_session_print(session, "  Register GPON_SKB_ENQUEUED_INDEXES_FIFO_ENTRY\n");

	bdmf_session_print(session, "\treserved_fw_only         =\n\t");
	for (i=0,j=0; i<24; i++)
	{
		MREAD_I_16((uint8_t *)p, i, r);
		bdmf_session_print(session, "0x%08x ", (unsigned int)r);
		j++;
		if (j >= 8)
		{
			j = 0;
			bdmf_session_print(session, "\n\t");
		}
	}
	bdmf_session_print(session, "\n");
}

void dump_RDD_DDR_QUEUE_ADDRESS_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register DDR_QUEUE_ADDRESS_ENTRY\n");

	MREAD_16((uint8_t *)p, r);
	bdmf_session_print(session, "\tsize                     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 2, r);
	bdmf_session_print(session, "\treserved0                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 3, r);
	bdmf_session_print(session, "\tcounter_number           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 4, r);
	bdmf_session_print(session, "\taddr                     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_PM_COUNTERS_BUFFER(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	int i,j;

	bdmf_session_print(session, "  Register PM_COUNTERS_BUFFER\n");

	bdmf_session_print(session, "\treserved_fw_only         =\n\t");
	for (i=0,j=0; i<8; i++)
	{
		MREAD_I_32((uint8_t *)p, i, r);
		bdmf_session_print(session, "0x%08x ", (unsigned int)r);
		j++;
		if (j >= 8)
		{
			j = 0;
			bdmf_session_print(session, "\n\t");
		}
	}
	bdmf_session_print(session, "\n");
}

void dump_RDD_INGRESS_CLASSIFICATION_LONG_LOOKUP_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register INGRESS_CLASSIFICATION_LONG_LOOKUP_ENTRY\n");

	FIELD_MREAD_8((uint8_t *)p, 4, 4, r);
	bdmf_session_print(session, "\tkey_index                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_32((uint8_t *)p, 4, 24, r);
	bdmf_session_print(session, "\tkey_0                    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	{ uint32_t temp; FIELD_MREAD_32((uint8_t *)p, 0, 4, temp); r = temp << 28; FIELD_MREAD_32(((uint8_t *)p + 4), 4, 28, temp); r = r | temp; };
	bdmf_session_print(session, "\tkey_1                    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 7, 3, 1, r);
	bdmf_session_print(session, "\treserved0                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 7, 2, 1, r);
	bdmf_session_print(session, "\taging                    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 7, 1, 1, r);
	bdmf_session_print(session, "\tskip                     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 7, 0, 1, r);
	bdmf_session_print(session, "\tvalid                    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 8, r);
	bdmf_session_print(session, "\tcontext                  = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_32((uint8_t *)p + 8, 0, 24, r);
	bdmf_session_print(session, "\tkey_2                    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 12, r);
	bdmf_session_print(session, "\tkey_3                    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_PM_COUNTERS(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	int i,j;

	bdmf_session_print(session, "  Register PM_COUNTERS\n");

	bdmf_session_print(session, "\treserved_fw_only         =\n\t");
	for (i=0,j=0; i<1536; i++)
	{
		MREAD_I_32((uint8_t *)p, i, r);
		bdmf_session_print(session, "0x%08x ", (unsigned int)r);
		j++;
		if (j >= 8)
		{
			j = 0;
			bdmf_session_print(session, "\n\t");
		}
	}
	bdmf_session_print(session, "\n");
}

void dump_RDD_RING_DESCRIPTOR(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register RING_DESCRIPTOR\n");

	FIELD_MREAD_8((uint8_t *)p, 4, 4, r);
	bdmf_session_print(session, "\treserved0                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p, 0, 12, r);
	bdmf_session_print(session, "\tentries_counter          = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 2, 4, 4, r);
	bdmf_session_print(session, "\treserved1                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p + 2, 0, 12, r);
	bdmf_session_print(session, "\tnumber_of_entries        = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 4, r);
	bdmf_session_print(session, "\tring_pointer             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 8, r);
	bdmf_session_print(session, "\tinterrupt_id             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 10, r);
	bdmf_session_print(session, "\tdrop_counter             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_32((uint8_t *)p + 12, 5, 27, r);
	bdmf_session_print(session, "\treserved2                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 15, 0, 5, r);
	bdmf_session_print(session, "\tsize_of_entry            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_CONNECTION_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register CONNECTION_ENTRY\n");

	FIELD_MREAD_8((uint8_t *)p, 7, 1, r);
	bdmf_session_print(session, "\tvalid                    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p, 0, 15, r);
	bdmf_session_print(session, "\tcontext_index            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 2, 5, 3, r);
	bdmf_session_print(session, "\tbucket_overflow_counter  = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 2, 0, 5, r);
	bdmf_session_print(session, "\tkey_extend               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 3, r);
	bdmf_session_print(session, "\tprotocol                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 4, r);
	bdmf_session_print(session, "\tsrc_port                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 6, r);
	bdmf_session_print(session, "\tdst_port                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 8, r);
	bdmf_session_print(session, "\tsrc_ip                   = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 12, r);
	bdmf_session_print(session, "\tdst_ip                   = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_TUNNEL_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	int i,j;

	bdmf_session_print(session, "  Register TUNNEL_ENTRY\n");

	MREAD_32((uint8_t *)p, r);
	bdmf_session_print(session, "\tlocal_ip                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 4, r);
	bdmf_session_print(session, "\ttunnel_type              = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 5, r);
	bdmf_session_print(session, "\ttunnel_header_length     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 6, 7, 1, r);
	bdmf_session_print(session, "\tip_family                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 6, 0, 7, r);
	bdmf_session_print(session, "\treserved0                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 7, r);
	bdmf_session_print(session, "\tlayer3_offset            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	bdmf_session_print(session, "\ttunnel_header            =\n\t");
	for (i=0,j=0; i<80; i++)
	{
		MREAD_I_8((uint8_t *)p + 8, i, r);
		bdmf_session_print(session, "0x%08x ", (unsigned int)r);
		j++;
		if (j >= 8)
		{
			j = 0;
			bdmf_session_print(session, "\n\t");
		}
	}
	bdmf_session_print(session, "\n");
}

void dump_RDD_WIFI_SSID_FORWARDING_MATRIX_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register WIFI_SSID_FORWARDING_MATRIX_ENTRY\n");

	MREAD_16((uint8_t *)p, r);
	bdmf_session_print(session, "\twifi_ssid_vector         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_DUAL_STACK_LITE_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	int i,j;

	bdmf_session_print(session, "  Register DUAL_STACK_LITE_ENTRY\n");

	MREAD_32((uint8_t *)p, r);
	bdmf_session_print(session, "\tvesion_class_flow_label  = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 4, r);
	bdmf_session_print(session, "\tpayload_length_next_header_hop_limit= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	bdmf_session_print(session, "\tsrc_ip                   =\n\t");
	for (i=0,j=0; i<16; i++)
	{
		MREAD_I_8((uint8_t *)p + 8, i, r);
		bdmf_session_print(session, "0x%08x ", (unsigned int)r);
		j++;
		if (j >= 8)
		{
			j = 0;
			bdmf_session_print(session, "\n\t");
		}
	}
	bdmf_session_print(session, "\n");
	bdmf_session_print(session, "\tdst_ip                   =\n\t");
	for (i=0,j=0; i<16; i++)
	{
		MREAD_I_8((uint8_t *)p + 24, i, r);
		bdmf_session_print(session, "0x%08x ", (unsigned int)r);
		j++;
		if (j >= 8)
		{
			j = 0;
			bdmf_session_print(session, "\n\t");
		}
	}
	bdmf_session_print(session, "\n");
	bdmf_session_print(session, "\treserved                 =\n\t");
	for (i=0,j=0; i<24; i++)
	{
		MREAD_I_8((uint8_t *)p + 40, i, r);
		bdmf_session_print(session, "0x%08x ", (unsigned int)r);
		j++;
		if (j >= 8)
		{
			j = 0;
			bdmf_session_print(session, "\n\t");
		}
	}
	bdmf_session_print(session, "\n");
}

void dump_RDD_DS_INGRESS_CLASSIFICATION_IH_LOOKUP_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register DS_INGRESS_CLASSIFICATION_IH_LOOKUP_ENTRY\n");

	MREAD_8((uint8_t *)p, r);
	bdmf_session_print(session, "\tcontext                  = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_32((uint8_t *)p + 0, 0, 24, r);
	bdmf_session_print(session, "\treserved0                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 4, 4, 4, r);
	bdmf_session_print(session, "\treserved1                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 4, 1, 3, r);
	bdmf_session_print(session, "\tpbits                    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 4, 0, 1, r);
	bdmf_session_print(session, "\tno_outer                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_32((uint8_t *)p + 4, 12, 12, r);
	bdmf_session_print(session, "\tvlan                     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p + 6, 4, 8, r);
	bdmf_session_print(session, "\tgem_flow_extend          = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 7, 3, 1, r);
	bdmf_session_print(session, "\treserved2                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 7, 2, 1, r);
	bdmf_session_print(session, "\taging                    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 7, 1, 1, r);
	bdmf_session_print(session, "\tskip                     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 7, 0, 1, r);
	bdmf_session_print(session, "\tvalid                    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_US_INGRESS_CLASSIFICATION_IH_LOOKUP_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register US_INGRESS_CLASSIFICATION_IH_LOOKUP_ENTRY\n");

	MREAD_8((uint8_t *)p, r);
	bdmf_session_print(session, "\tcontext                  = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_32((uint8_t *)p + 0, 0, 24, r);
	bdmf_session_print(session, "\treserved0                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 4, 4, 4, r);
	bdmf_session_print(session, "\treserved1                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 4, 1, 3, r);
	bdmf_session_print(session, "\tpbits                    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 4, 0, 1, r);
	bdmf_session_print(session, "\tno_outer                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_32((uint8_t *)p + 4, 12, 12, r);
	bdmf_session_print(session, "\tvlan                     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 6, 1, 3, r);
	bdmf_session_print(session, "\treserved2                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p + 6, 4, 5, r);
	bdmf_session_print(session, "\tsrc_port_extend          = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 7, 3, 1, r);
	bdmf_session_print(session, "\treserved3                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 7, 2, 1, r);
	bdmf_session_print(session, "\taging                    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 7, 1, 1, r);
	bdmf_session_print(session, "\tskip                     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 7, 0, 1, r);
	bdmf_session_print(session, "\tvalid                    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_DS_INGRESS_CLASSIFICATION_OPTIMIZED_LOOKUP_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register DS_INGRESS_CLASSIFICATION_OPTIMIZED_LOOKUP_ENTRY\n");

	MREAD_8((uint8_t *)p, r);
	bdmf_session_print(session, "\tcontext                  = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 1, 4, 4, r);
	bdmf_session_print(session, "\tkey_index                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_32((uint8_t *)p + 0, 12, 8, r);
	bdmf_session_print(session, "\treserved0                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p + 2, 4, 8, r);
	bdmf_session_print(session, "\tgem_flow                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 3, 1, 3, r);
	bdmf_session_print(session, "\tinner_pbits              = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 3, 0, 1, r);
	bdmf_session_print(session, "\tno_inner                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p + 4, 4, 12, r);
	bdmf_session_print(session, "\tinner_vlan               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 5, 1, 3, r);
	bdmf_session_print(session, "\touter_pbits              = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 5, 0, 1, r);
	bdmf_session_print(session, "\tno_outer                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p + 6, 4, 12, r);
	bdmf_session_print(session, "\touter_vlan               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 7, 3, 1, r);
	bdmf_session_print(session, "\treserved1                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 7, 2, 1, r);
	bdmf_session_print(session, "\taging                    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 7, 1, 1, r);
	bdmf_session_print(session, "\tskip                     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 7, 0, 1, r);
	bdmf_session_print(session, "\tvalid                    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_US_INGRESS_CLASSIFICATION_OPTIMIZED_LOOKUP_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register US_INGRESS_CLASSIFICATION_OPTIMIZED_LOOKUP_ENTRY\n");

	MREAD_8((uint8_t *)p, r);
	bdmf_session_print(session, "\tcontext                  = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 1, 4, 4, r);
	bdmf_session_print(session, "\tkey_index                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 1, 0, 4, r);
	bdmf_session_print(session, "\treserved0                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 2, 4, 4, r);
	bdmf_session_print(session, "\tssid                     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p + 2, 4, 8, r);
	bdmf_session_print(session, "\tsrc_bridge_port          = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 3, 1, 3, r);
	bdmf_session_print(session, "\tinner_pbits              = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 3, 0, 1, r);
	bdmf_session_print(session, "\tno_inner                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p + 4, 4, 12, r);
	bdmf_session_print(session, "\tinner_vlan               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 5, 1, 3, r);
	bdmf_session_print(session, "\touter_pbits              = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 5, 0, 1, r);
	bdmf_session_print(session, "\tno_outer                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p + 6, 4, 12, r);
	bdmf_session_print(session, "\touter_vlan               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 7, 3, 1, r);
	bdmf_session_print(session, "\treserved1                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 7, 2, 1, r);
	bdmf_session_print(session, "\taging                    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 7, 1, 1, r);
	bdmf_session_print(session, "\tskip                     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 7, 0, 1, r);
	bdmf_session_print(session, "\tvalid                    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_INGRESS_CLASSIFICATION_SHORT_LOOKUP_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register INGRESS_CLASSIFICATION_SHORT_LOOKUP_ENTRY\n");

	MREAD_8((uint8_t *)p, r);
	bdmf_session_print(session, "\tcontext                  = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 1, 4, 4, r);
	bdmf_session_print(session, "\tkey_index                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_32((uint8_t *)p + 0, 4, 16, r);
	bdmf_session_print(session, "\tkey_0                    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	{ uint32_t temp; FIELD_MREAD_32((uint8_t *)p, 0, 4, temp); r = temp << 28; FIELD_MREAD_32(((uint8_t *)p + 4), 4, 28, temp); r = r | temp; };
	bdmf_session_print(session, "\tkey_1                    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 7, 3, 1, r);
	bdmf_session_print(session, "\treserved0                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 7, 2, 1, r);
	bdmf_session_print(session, "\taging                    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 7, 1, 1, r);
	bdmf_session_print(session, "\tskip                     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 7, 0, 1, r);
	bdmf_session_print(session, "\tvalid                    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

#if defined OREN
void dump_RDD_VLAN_COMMAND_INDEX_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register VLAN_COMMAND_INDEX_ENTRY\n");

	MREAD_8((uint8_t *)p, r);
	bdmf_session_print(session, "\tpci0_vlan_command_id     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 1, r);
	bdmf_session_print(session, "\teth0_vlan_command_id     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 2, r);
	bdmf_session_print(session, "\teth1_vlan_command_id     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 3, r);
	bdmf_session_print(session, "\teth2_vlan_command_id     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 4, r);
	bdmf_session_print(session, "\teth3_vlan_command_id     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 5, r);
	bdmf_session_print(session, "\teth4_vlan_command_id     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 6, r);
	bdmf_session_print(session, "\treserved0                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

#endif

void dump_RDD_IP_SYNC_1588_DESCRIPTOR_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register IP_SYNC_1588_DESCRIPTOR_ENTRY\n");

	MREAD_32((uint8_t *)p, r);
	bdmf_session_print(session, "\ttod_high                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 4, r);
	bdmf_session_print(session, "\ttod_low                  = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p + 8, 4, 12, r);
	bdmf_session_print(session, "\tlocal_time_delta         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_32((uint8_t *)p + 8, 0, 20, r);
	bdmf_session_print(session, "\treserved0                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 12, r);
	bdmf_session_print(session, "\treserved1                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_DUMMY_RATE_CONTROLLER_DESCRIPTOR(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	int i,j;

	bdmf_session_print(session, "  Register DUMMY_RATE_CONTROLLER_DESCRIPTOR\n");

	bdmf_session_print(session, "\treserved_fw_only         =\n\t");
	for (i=0,j=0; i<16; i++)
	{
		MREAD_I_32((uint8_t *)p, i, r);
		bdmf_session_print(session, "0x%08x ", (unsigned int)r);
		j++;
		if (j >= 8)
		{
			j = 0;
			bdmf_session_print(session, "\n\t");
		}
	}
	bdmf_session_print(session, "\n");
}

void dump_RDD_WAN_TX_QUEUE_DESCRIPTOR(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register WAN_TX_QUEUE_DESCRIPTOR\n");

	MREAD_16((uint8_t *)p, r);
	bdmf_session_print(session, "\thead_ptr                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 2, r);
	bdmf_session_print(session, "\ttail_ptr                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 4, r);
	bdmf_session_print(session, "\tpacket_counter           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 6, r);
	bdmf_session_print(session, "\treserved                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 7, r);
	bdmf_session_print(session, "\tcounter_number           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 8, r);
	bdmf_session_print(session, "\tpacket_threshold         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 10, r);
	bdmf_session_print(session, "\tprofile_ptr              = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 12, r);
	bdmf_session_print(session, "\trate_controller_ptr      = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 14, r);
	bdmf_session_print(session, "\tqueue_mask               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_US_RATE_CONTROLLER_DESCRIPTOR(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	int i,j;

	bdmf_session_print(session, "  Register US_RATE_CONTROLLER_DESCRIPTOR\n");

	MREAD_32((uint8_t *)p, r);
	bdmf_session_print(session, "\tcurrent_peak_budget      = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 4, 6, 2, r);
	bdmf_session_print(session, "\tallocated_peak_budget_exponent= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p + 4, 0, 14, r);
	bdmf_session_print(session, "\tallocated_peak_budget    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 6, 6, 2, r);
	bdmf_session_print(session, "\tpeak_budget_limit_exponent= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p + 6, 0, 14, r);
	bdmf_session_print(session, "\tpeak_budget_limit        = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 8, r);
	bdmf_session_print(session, "\tcurrent_sustain_budget   = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 12, r);
	bdmf_session_print(session, "\tallocated_sustain_budget = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 16, r);
	bdmf_session_print(session, "\trate_controller_mask     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 20, r);
	bdmf_session_print(session, "\twan_channel_ptr          = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 22, r);
	bdmf_session_print(session, "\tpriority_queues_status   = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 24, r);
	bdmf_session_print(session, "\tpeak_burst_counter       = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 26, r);
	bdmf_session_print(session, "\tpeak_weight              = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 27, 1, 7, r);
	bdmf_session_print(session, "\treserved0                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 27, 0, 1, r);
	bdmf_session_print(session, "\tpeak_burst_flag          = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	bdmf_session_print(session, "\ttx_queue_addr            =\n\t");
	for (i=0,j=0; i<8; i++)
	{
		MREAD_I_16((uint8_t *)p + 28, i, r);
		bdmf_session_print(session, "0x%08x ", (unsigned int)r);
		j++;
		if (j >= 8)
		{
			j = 0;
			bdmf_session_print(session, "\n\t");
		}
	}
	bdmf_session_print(session, "\n");
	MREAD_32((uint8_t *)p + 44, r);
	bdmf_session_print(session, "\treserved1                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_SRC_MAC_ANTI_SPOOFING_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register SRC_MAC_ANTI_SPOOFING_ENTRY\n");

	MREAD_32((uint8_t *)p, r);
	bdmf_session_print(session, "\tmac_prefix               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_TUNNEL_DYNAMIC_FIELDS_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register TUNNEL_DYNAMIC_FIELDS_ENTRY\n");

	MREAD_16((uint8_t *)p, r);
	bdmf_session_print(session, "\tl3_total_len             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 2, r);
	bdmf_session_print(session, "\tl3_chsum                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_DUMMY_WAN_TX_QUEUE_DESCRIPTOR(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	int i,j;

	bdmf_session_print(session, "  Register DUMMY_WAN_TX_QUEUE_DESCRIPTOR\n");

	bdmf_session_print(session, "\treserved_fw_only         =\n\t");
	for (i=0,j=0; i<4; i++)
	{
		MREAD_I_32((uint8_t *)p, i, r);
		bdmf_session_print(session, "0x%08x ", (unsigned int)r);
		j++;
		if (j >= 8)
		{
			j = 0;
			bdmf_session_print(session, "\n\t");
		}
	}
	bdmf_session_print(session, "\n");
}

void dump_RDD_SPEED_SERVICE_RX_TIMESTAMPS(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register SPEED_SERVICE_RX_TIMESTAMPS\n");

	MREAD_32((uint8_t *)p, r);
	bdmf_session_print(session, "\tstart_ts                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 4, r);
	bdmf_session_print(session, "\tlast_ts                  = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_BRIDGE_PORT_TO_BROADCOM_SWITCH_PORT_MAPPING_TABLE_PTR(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register BRIDGE_PORT_TO_BROADCOM_SWITCH_PORT_MAPPING_TABLE_PTR\n");

	MREAD_16((uint8_t *)p, r);
	bdmf_session_print(session, "\treserved_fw_only         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_LAN_INGRESS_FIFO_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	int i,j;

	bdmf_session_print(session, "  Register LAN_INGRESS_FIFO_ENTRY\n");

	bdmf_session_print(session, "\treserved_fw_only         =\n\t");
	for (i=0,j=0; i<32; i++)
	{
		MREAD_I_16((uint8_t *)p, i, r);
		bdmf_session_print(session, "0x%08x ", (unsigned int)r);
		j++;
		if (j >= 8)
		{
			j = 0;
			bdmf_session_print(session, "\n\t");
		}
	}
	bdmf_session_print(session, "\n");
}

void dump_RDD_WAN_PHYSICAL_PORT(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register WAN_PHYSICAL_PORT\n");

	MREAD_16((uint8_t *)p, r);
	bdmf_session_print(session, "\treserved_fw_only         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_RATE_CONTROLLER_EXPONENT_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register RATE_CONTROLLER_EXPONENT_ENTRY\n");

	MREAD_8((uint8_t *)p, r);
	bdmf_session_print(session, "\texponent                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_MULTICAST_ACTIVE_PORTS_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register MULTICAST_ACTIVE_PORTS_ENTRY\n");

	MREAD_8((uint8_t *)p, r);
	bdmf_session_print(session, "\tactive_ports_number      = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_FLOW_CACHE_CONTEXT_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	int i,j;

	bdmf_session_print(session, "  Register FLOW_CACHE_CONTEXT_ENTRY\n");

	MREAD_8((uint8_t *)p, r);
	bdmf_session_print(session, "\treserved                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_32((uint8_t *)p + 0, 0, 24, r);
	bdmf_session_print(session, "\tvalid_packets_counter    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 4, r);
	bdmf_session_print(session, "\tvalid_bytes_counter      = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 8, 7, 1, r);
	bdmf_session_print(session, "\tfwd_action               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 8, 6, 1, r);
	bdmf_session_print(session, "\tservice_queue_mode_miss_union= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 8, 0, 6, r);
	bdmf_session_print(session, "\taction_union             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 9, 2, 6, r);
	bdmf_session_print(session, "\tdscp_value               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 9, 0, 2, r);
	bdmf_session_print(session, "\tecn_value                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 10, r);
	bdmf_session_print(session, "\tnat_port                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 12, r);
	bdmf_session_print(session, "\tip_checksum_delta        = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 14, r);
	bdmf_session_print(session, "\tlayer4_checksum_delta    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 16, r);
	bdmf_session_print(session, "\tnat_ip                   = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 20, r);
	bdmf_session_print(session, "\tegress_port              = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 21, 7, 1, r);
	bdmf_session_print(session, "\tip_version               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 21, 3, 4, r);
	bdmf_session_print(session, "\twifi_ssid                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 21, 1, 2, r);
	bdmf_session_print(session, "\tdrop_eligibility         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 21, 0, 1, r);
	bdmf_session_print(session, "\tis_wfd                   = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 22, r);
	bdmf_session_print(session, "\tegress_params_union      = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 24, 5, 3, r);
	bdmf_session_print(session, "\touter_vid_offset         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 24, 0, 5, r);
	bdmf_session_print(session, "\tpolicer_id               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 25, 7, 1, r);
	bdmf_session_print(session, "\toverflow                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 25, 5, 2, r);
	bdmf_session_print(session, "\touter_pbit_remap_action  = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 25, 3, 2, r);
	bdmf_session_print(session, "\tinner_pbit_remap_action  = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 25, 2, 1, r);
	bdmf_session_print(session, "\tqos_mapping_mode         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 25, 0, 2, r);
	bdmf_session_print(session, "\tlayer2_header_number_of_tags= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 26, r);
	bdmf_session_print(session, "\tl2_offset                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 27, r);
	bdmf_session_print(session, "\tl2_size                  = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 28, r);
	bdmf_session_print(session, "\tactions_vector           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 30, 7, 1, r);
	bdmf_session_print(session, "\tconnection_direction     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p + 30, 0, 15, r);
	bdmf_session_print(session, "\tconnection_table_index   = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	bdmf_session_print(session, "\tl2_header                =\n\t");
	for (i=0,j=0; i<32; i++)
	{
		MREAD_I_8((uint8_t *)p + 32, i, r);
		bdmf_session_print(session, "0x%08x ", (unsigned int)r);
		j++;
		if (j >= 8)
		{
			j = 0;
			bdmf_session_print(session, "\n\t");
		}
	}
	bdmf_session_print(session, "\n");
}

void dump_RDD_FIREWALL_RULE_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register FIREWALL_RULE_ENTRY\n");

	FIELD_MREAD_8((uint8_t *)p, 3, 5, r);
	bdmf_session_print(session, "\treserved0                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p, 2, 1, r);
	bdmf_session_print(session, "\tcheck_mask_src_ip        = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p, 1, 1, r);
	bdmf_session_print(session, "\tcheck_src_ip             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p, 0, 1, r);
	bdmf_session_print(session, "\tcheck_dst_ip             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 1, r);
	bdmf_session_print(session, "\tnext_rule                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 2, r);
	bdmf_session_print(session, "\tsrc_ip_mask              = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 3, r);
	bdmf_session_print(session, "\treserved2                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 4, r);
	bdmf_session_print(session, "\tdst_ip                   = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 8, r);
	bdmf_session_print(session, "\tsrc_ip                   = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 12, r);
	bdmf_session_print(session, "\treserved1                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

#if defined OREN
void dump_RDD_IPTV_DDR_CONTEXT_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register IPTV_DDR_CONTEXT_ENTRY\n");

	MREAD_16((uint8_t *)p, r);
	bdmf_session_print(session, "\twlan_mcast_index         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 2, r);
	bdmf_session_print(session, "\tcounter                  = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 4, r);
	bdmf_session_print(session, "\treserved0                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 5, r);
	bdmf_session_print(session, "\tingress_classification_context= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 6, 3, 5, r);
	bdmf_session_print(session, "\treserved1                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 6, 2, 1, r);
	bdmf_session_print(session, "\tvalid                    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 6, 1, 1, r);
	bdmf_session_print(session, "\tcache_valid              = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p + 6, 0, 9, r);
	bdmf_session_print(session, "\tcache_index              = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 8, r);
	bdmf_session_print(session, "\treserved2                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 12, r);
	bdmf_session_print(session, "\tport_vector_info         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

#endif

void dump_RDD_IPTV_L2_DDR_LOOKUP_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register IPTV_L2_DDR_LOOKUP_ENTRY\n");

	FIELD_MREAD_8((uint8_t *)p, 7, 1, r);
	bdmf_session_print(session, "\tvalid                    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_32((uint8_t *)p, 0, 31, r);
	bdmf_session_print(session, "\treserved_0               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 4, r);
	bdmf_session_print(session, "\treserved_1               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 8, 4, 4, r);
	bdmf_session_print(session, "\treserved_2               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p + 8, 0, 12, r);
	bdmf_session_print(session, "\tvid                      = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 10, r);
	bdmf_session_print(session, "\tmac_addr0                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 11, r);
	bdmf_session_print(session, "\tmac_addr1                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 12, r);
	bdmf_session_print(session, "\tmac_addr2                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 13, r);
	bdmf_session_print(session, "\tmac_addr3                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 14, r);
	bdmf_session_print(session, "\tmac_addr4                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 15, r);
	bdmf_session_print(session, "\tmac_addr5                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_IPTV_L3_DDR_LOOKUP_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register IPTV_L3_DDR_LOOKUP_ENTRY\n");

	FIELD_MREAD_8((uint8_t *)p, 7, 1, r);
	bdmf_session_print(session, "\tvalid                    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_32((uint8_t *)p, 0, 31, r);
	bdmf_session_print(session, "\treserved_0               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 4, r);
	bdmf_session_print(session, "\tdst_ip12                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 5, r);
	bdmf_session_print(session, "\tdst_ip13                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 6, r);
	bdmf_session_print(session, "\tdst_ip14                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 7, r);
	bdmf_session_print(session, "\tdst_ip15                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 8, r);
	bdmf_session_print(session, "\treserved_1               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 12, r);
	bdmf_session_print(session, "\tdst_ip0                  = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 13, r);
	bdmf_session_print(session, "\tdst_ip1                  = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 14, r);
	bdmf_session_print(session, "\tdst_ip2                  = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 15, r);
	bdmf_session_print(session, "\tdst_ip3                  = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_IPTV_L3_SSM_DDR_LOOKUP_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register IPTV_L3_SSM_DDR_LOOKUP_ENTRY\n");

	FIELD_MREAD_8((uint8_t *)p, 7, 1, r);
	bdmf_session_print(session, "\tvalid                    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p, 4, 3, r);
	bdmf_session_print(session, "\treserved_0               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p, 2, 10, r);
	bdmf_session_print(session, "\tcontext_table            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 1, 1, 1, r);
	bdmf_session_print(session, "\tcontext_valid            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 1, 0, 1, r);
	bdmf_session_print(session, "\tany                      = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 2, r);
	bdmf_session_print(session, "\treserved_1               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 4, r);
	bdmf_session_print(session, "\tdst_ip12                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 5, r);
	bdmf_session_print(session, "\tdst_ip13                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 6, r);
	bdmf_session_print(session, "\tdst_ip14                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 7, r);
	bdmf_session_print(session, "\tdst_ip15                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_32((uint8_t *)p + 8, 12, 20, r);
	bdmf_session_print(session, "\treserved_2               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p + 10, 0, 12, r);
	bdmf_session_print(session, "\tvid                      = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 12, r);
	bdmf_session_print(session, "\tdst_ip0                  = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 13, r);
	bdmf_session_print(session, "\tdst_ip1                  = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 14, r);
	bdmf_session_print(session, "\tdst_ip2                  = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 15, r);
	bdmf_session_print(session, "\tdst_ip3                  = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_FIREWALL_RULES_MAP_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register FIREWALL_RULES_MAP_ENTRY\n");

	MREAD_8((uint8_t *)p, r);
	bdmf_session_print(session, "\trule_index               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

#if defined G9991
void dump_RDD_VLAN_COMMAND_INDEX_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register VLAN_COMMAND_INDEX_ENTRY\n");

	MREAD_8((uint8_t *)p, r);
	bdmf_session_print(session, "\teth0_vlan_command_id     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 1, r);
	bdmf_session_print(session, "\teth1_vlan_command_id     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 2, r);
	bdmf_session_print(session, "\teth2_vlan_command_id     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 3, r);
	bdmf_session_print(session, "\teth3_vlan_command_id     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 4, r);
	bdmf_session_print(session, "\teth4_vlan_command_id     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 5, r);
	bdmf_session_print(session, "\teth5_vlan_command_id     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 6, r);
	bdmf_session_print(session, "\teth6_vlan_command_id     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 7, r);
	bdmf_session_print(session, "\teth7_vlan_command_id     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 8, r);
	bdmf_session_print(session, "\teth8_vlan_command_id     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 9, r);
	bdmf_session_print(session, "\teth9_vlan_command_id     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 10, r);
	bdmf_session_print(session, "\teth10_vlan_command_id    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 11, r);
	bdmf_session_print(session, "\teth11_vlan_command_id    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 12, r);
	bdmf_session_print(session, "\teth12_vlan_command_id    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 13, r);
	bdmf_session_print(session, "\teth13_vlan_command_id    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 14, r);
	bdmf_session_print(session, "\teth14_vlan_command_id    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 15, r);
	bdmf_session_print(session, "\teth15_vlan_command_id    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 16, r);
	bdmf_session_print(session, "\teth16_vlan_command_id    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 17, r);
	bdmf_session_print(session, "\teth17_vlan_command_id    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 18, r);
	bdmf_session_print(session, "\teth18_vlan_command_id    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 19, r);
	bdmf_session_print(session, "\teth19_vlan_command_id    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 20, r);
	bdmf_session_print(session, "\teth20_vlan_command_id    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 21, r);
	bdmf_session_print(session, "\teth21_vlan_command_id    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 22, r);
	bdmf_session_print(session, "\teth22_vlan_command_id    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 23, r);
	bdmf_session_print(session, "\teth23_vlan_command_id    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

#endif

#if defined G9991
void dump_RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY\n");

	FIELD_MREAD_16((uint8_t *)p, 3, 13, r);
	bdmf_session_print(session, "\tvlan_index_table_ptr     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 1, 2, 1, r);
	bdmf_session_print(session, "\topbit_remark_mode        = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 1, 1, 1, r);
	bdmf_session_print(session, "\tipbit_remark_mode        = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_32((uint8_t *)p + 0, 13, 4, r);
	bdmf_session_print(session, "\twifi_ssid                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 2, 4, 1, r);
	bdmf_session_print(session, "\tdei_remark_enable        = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 2, 3, 1, r);
	bdmf_session_print(session, "\tdei_value                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 2, 2, 1, r);
	bdmf_session_print(session, "\tservice_queue_mode       = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 2, 1, 1, r);
	bdmf_session_print(session, "\tforward_mode             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p + 2, 4, 5, r);
	bdmf_session_print(session, "\tegress_port              = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 3, 3, 1, r);
	bdmf_session_print(session, "\tqos_mapping_mode         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 3, 0, 3, r);
	bdmf_session_print(session, "\ttraffic_class            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 4, 7, 1, r);
	bdmf_session_print(session, "\tdrop                     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 4, 6, 1, r);
	bdmf_session_print(session, "\tcpu                      = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 4, 5, 1, r);
	bdmf_session_print(session, "\tpolicer_mode             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 4, 1, 4, r);
	bdmf_session_print(session, "\tpolicer_id               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 4, 0, 1, r);
	bdmf_session_print(session, "\trate_shaping_mode        = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 5, 7, 1, r);
	bdmf_session_print(session, "\tcpu_mirroring            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 5, 6, 1, r);
	bdmf_session_print(session, "\tic_ip_flow               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 5, 1, 5, r);
	bdmf_session_print(session, "\tservice_queue            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_32((uint8_t *)p + 4, 15, 2, r);
	bdmf_session_print(session, "\tsubnet_id                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 6, 4, 3, r);
	bdmf_session_print(session, "\touter_pbit               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 6, 1, 3, r);
	bdmf_session_print(session, "\tinner_pbit               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 6, 0, 1, r);
	bdmf_session_print(session, "\tdscp_remarking_mode      = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 7, 2, 6, r);
	bdmf_session_print(session, "\tdscp                     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 7, 0, 2, r);
	bdmf_session_print(session, "\tecn                      = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

#endif

#if defined G9991
void dump_RDD_ETH_TX_MAC_DESCRIPTOR(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register ETH_TX_MAC_DESCRIPTOR\n");

	MREAD_8((uint8_t *)p, r);
	bdmf_session_print(session, "\tegress_counter           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 1, r);
	bdmf_session_print(session, "\tloopback_mode            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 2, r);
	bdmf_session_print(session, "\treserved6                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 4, r);
	bdmf_session_print(session, "\tingress_counter          = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 5, r);
	bdmf_session_print(session, "\trate_limiter_id          = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 6, r);
	bdmf_session_print(session, "\ttx_task_number           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	{ uint32_t temp; FIELD_MREAD_32(((uint8_t *)p + 4), 0, 8, temp); r = temp << 24; FIELD_MREAD_32(((uint8_t *)p + 8), 8, 24, temp); r = r | temp; };
	bdmf_session_print(session, "\treserved7                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	{ uint32_t temp; FIELD_MREAD_32(((uint8_t *)p + 8), 0, 8, temp); r = temp << 24; FIELD_MREAD_32(((uint8_t *)p + 12), 8, 24, temp); r = r | temp; };
	bdmf_session_print(session, "\treserved8                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 15, r);
	bdmf_session_print(session, "\treserved9                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 16, r);
	bdmf_session_print(session, "\treserved10               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 20, r);
	bdmf_session_print(session, "\treserved11               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 24, r);
	bdmf_session_print(session, "\treserved12               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 28, r);
	bdmf_session_print(session, "\treserved13               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 32, r);
	bdmf_session_print(session, "\teof_flag                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 33, r);
	bdmf_session_print(session, "\ttx_queues_status         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 34, r);
	bdmf_session_print(session, "\tpacket_counters_ptr      = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 36, r);
	bdmf_session_print(session, "\tgpio_flow_control_vector_ptr= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 38, r);
	bdmf_session_print(session, "\tlast_queue_served        = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 39, r);
	bdmf_session_print(session, "\tphyiscal_port            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 40, r);
	bdmf_session_print(session, "\temac_mask                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 44, r);
	bdmf_session_print(session, "\treserved1                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 48, r);
	bdmf_session_print(session, "\treserved2                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 52, r);
	bdmf_session_print(session, "\treserved3                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 56, r);
	bdmf_session_print(session, "\treserved4                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 60, r);
	bdmf_session_print(session, "\treserved5                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

#endif

#if defined G9991
void dump_RDD_ETH_TX_EMACS_STATUS_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register ETH_TX_EMACS_STATUS_ENTRY\n");

	MREAD_32((uint8_t *)p, r);
	bdmf_session_print(session, "\tstatus_vector            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

#endif

#if defined G9991
void dump_RDD_ETH_TX_INTER_LAN_SCHEDULING_OFFSET_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register ETH_TX_INTER_LAN_SCHEDULING_OFFSET_ENTRY\n");

	MREAD_32((uint8_t *)p, r);
	bdmf_session_print(session, "\temac_offset              = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

#endif

#if defined G9991
void dump_RDD_US_SID_CONTEXT_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register US_SID_CONTEXT_ENTRY\n");

	FIELD_MREAD_8((uint8_t *)p, 7, 1, r);
	bdmf_session_print(session, "\treserved0                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p, 5, 2, r) /*defined by rdd_us_sid_state_machine enumeration*/;
	bdmf_session_print(session, "\tstate_machine            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p, 0, 5, r);
	bdmf_session_print(session, "\tfragment_count           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_32((uint8_t *)p + 0, 0, 24, r);
	bdmf_session_print(session, "\treserved1                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_32((uint8_t *)p + 4, 8, 24, r);
	bdmf_session_print(session, "\treserved2                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 7, r);
	bdmf_session_print(session, "\tcpu_rx_cpu_reason        = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_32((uint8_t *)p + 8, 14, 18, r);
	bdmf_session_print(session, "\tcpu_rx_pd_0              = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p + 10, 0, 14, r);
	bdmf_session_print(session, "\tcpu_rx_length            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_32((uint8_t *)p + 12, 14, 18, r);
	bdmf_session_print(session, "\tcpu_rx_pd_1              = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p + 14, 0, 14, r);
	bdmf_session_print(session, "\tcpu_rx_bpm_number        = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 16, 5, 3, r);
	bdmf_session_print(session, "\treserved3                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 16, 2, 3, r);
	bdmf_session_print(session, "\tupstream_vlan_vlan_tags  = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 16, 0, 2, r);
	bdmf_session_print(session, "\tupstream_vlan_l3_protocol= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 17, r);
	bdmf_session_print(session, "\tupstream_vlan_dscp       = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 18, r);
	bdmf_session_print(session, "\tupstream_outer_vlan      = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 20, r);
	bdmf_session_print(session, "\tupstream_ingress_flow_entry= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_32((uint8_t *)p + 24, 14, 18, r);
	bdmf_session_print(session, "\tupstream_vlan_pd_0       = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p + 26, 0, 14, r);
	bdmf_session_print(session, "\tupstream_vlan_length     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_32((uint8_t *)p + 28, 14, 18, r);
	bdmf_session_print(session, "\tupstream_vlan_pd_1       = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p + 30, 0, 14, r);
	bdmf_session_print(session, "\tupstream_vlan_bpm_number = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

#endif

#if defined G9991
void dump_RDD_IPTV_CONTEXT_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register IPTV_CONTEXT_ENTRY\n");

	MREAD_32((uint8_t *)p, r);
	bdmf_session_print(session, "\tport_vector_info         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

#endif

#if defined G9991
void dump_RDD_IPTV_DDR_CONTEXT_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register IPTV_DDR_CONTEXT_ENTRY\n");

	MREAD_16((uint8_t *)p, r);
	bdmf_session_print(session, "\twlan_mcast_index         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 2, r);
	bdmf_session_print(session, "\tcounter                  = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 4, r);
	bdmf_session_print(session, "\tbridge_port              = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 5, r);
	bdmf_session_print(session, "\tingress_classification_context= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 6, 3, 5, r);
	bdmf_session_print(session, "\treserved0                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 6, 2, 1, r);
	bdmf_session_print(session, "\tvalid                    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 6, 1, 1, r);
	bdmf_session_print(session, "\tcache_valid              = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p + 6, 0, 9, r);
	bdmf_session_print(session, "\tcache_index              = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 8, r);
	bdmf_session_print(session, "\treserved1                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 12, r);
	bdmf_session_print(session, "\tport_vector_info         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

#endif

