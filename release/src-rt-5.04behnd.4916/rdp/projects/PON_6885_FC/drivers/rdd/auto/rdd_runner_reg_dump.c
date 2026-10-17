/*
   Copyright (c) 2025 Broadcom
   All Rights Reserved

    <:label-BRCM:2025:DUAL/GPL:standard

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

void dump_RDD_IPTV_DDR_CONTEXT_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	int i,j;

	bdmf_session_print(session, "  Register IPTV_DDR_CONTEXT_ENTRY\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p), 7, 1, (r));
	bdmf_session_print(session, "\tvalid                    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p), 6, 1, (r));
	bdmf_session_print(session, "\tip_ver                   = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p), 5, 1, (r));
	bdmf_session_print(session, "\tany_vid                  = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p), 4, 1, (r));
	bdmf_session_print(session, "\tany_src_ip               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_16((uint8_t *)(p), 0, 12, (r));
	bdmf_session_print(session, "\tvid                      = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 2, (r));
	bdmf_session_print(session, "\tnext_entry_idx           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 4, (r));
	bdmf_session_print(session, "\tssid_vector_0_or_flooding_vport= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 6, (r));
	bdmf_session_print(session, "\tssid_vector_1            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 8, (r));
	bdmf_session_print(session, "\tssid_vector_2            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 10, 5, 3, (r));
	bdmf_session_print(session, "\treserved4                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 10, 0, 5, (r));
	bdmf_session_print(session, "\trdd_vport                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 11, (r));
	bdmf_session_print(session, "\treserved3                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 12, (r));
	bdmf_session_print(session, "\treserved2                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 13, 6, 2, (r));
	bdmf_session_print(session, "\tpool_num                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 13, 0, 6, (r));
	bdmf_session_print(session, "\treplications             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 14, 2, 6, (r));
	bdmf_session_print(session, "\treserved1                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_16((uint8_t *)(p) + 14, 0, 10, (r));
	bdmf_session_print(session, "\tcntr_id                  = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 16, (r));
	bdmf_session_print(session, "\tegress_ports_vector      = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 20, (r));
	bdmf_session_print(session, "\tsrc_ip                   = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	bdmf_session_print(session, "\tsrc_ipv6_addr            =\n\t");
	for (i=0,j=0; i<16; i++)
	{
		MREAD_I_8((uint8_t *)(p) + 24, i, (r));
		bdmf_session_print(session, "0x%08x ", (unsigned int)r);
		j++;
		if (j >= 8)
		{
			j = 0;
			bdmf_session_print(session, "\n\t");
		}
	}
	bdmf_session_print(session, "\n");
	bdmf_session_print(session, "\tdst_ipv6_addr            =\n\t");
	for (i=0,j=0; i<16; i++)
	{
		MREAD_I_8((uint8_t *)(p) + 40, i, (r));
		bdmf_session_print(session, "0x%08x ", (unsigned int)r);
		j++;
		if (j >= 8)
		{
			j = 0;
			bdmf_session_print(session, "\n\t");
		}
	}
	bdmf_session_print(session, "\n");
	bdmf_session_print(session, "\tresult                   =\n\t");
	for (i=0,j=0; i<32; i++)
	{
		MREAD_I_8((uint8_t *)(p) + 56, i, (r));
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

void set_RDD_IPTV_DDR_CONTEXT_ENTRY(bdmf_session_handle session, unsigned char *p, const bdmfmon_parm_value_t parm[], int write_index)
{
	int i = 0;
	int j;
	j = i;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p), 7, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p), 6, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p), 5, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p), 4, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_16((uint8_t *)(p), 0, 12, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 2, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 4, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 6, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 8, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 10, 5, 3, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 10, 0, 5, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_8((uint8_t *)(p) + 11, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_8((uint8_t *)(p) + 12, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 13, 6, 2, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 13, 0, 6, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 14, 2, 6, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_16((uint8_t *)(p) + 14, 0, 10, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 16, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 20, (parm[j].number));
	j++;
	for (i=0; i<16; i++)
	{
		if ((write_index == -1) || (write_index == j))
			MWRITE_I_8((uint8_t *)(p) + 24, i, (parm[j].number));
		j++;
	}
	
	for (i=0; i<16; i++)
	{
		if ((write_index == -1) || (write_index == j))
			MWRITE_I_8((uint8_t *)(p) + 40, i, (parm[j].number));
		j++;
	}
	
	for (i=0; i<32; i++)
	{
		if ((write_index == -1) || (write_index == j))
			MWRITE_I_8((uint8_t *)(p) + 56, i, (parm[j].number));
		j++;
	}
	
}

void dump_RDD_POLICER_PARAMS_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register POLICER_PARAMS_ENTRY\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p), 1, 7, (r));
	bdmf_session_print(session, "\trl_overhead              = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p), 0, 1, (r));
	bdmf_session_print(session, "\tdei_mode                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 1, 7, 1, (r));
	bdmf_session_print(session, "\tcolor_aware_enabled      = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 1, 0, 7, (r));
	bdmf_session_print(session, "\treserved4                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void set_RDD_POLICER_PARAMS_ENTRY(bdmf_session_handle session, unsigned char *p, const bdmfmon_parm_value_t parm[], int write_index)
{
	int i = 0;
	int j;
	j = i;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p), 1, 7, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p), 0, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 1, 7, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 1, 0, 7, (parm[j].number));
	j++;
}

void dump_RDD_BYTE_1(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register BYTE_1\n");

	MREAD_8((uint8_t *)(p), (r));
	bdmf_session_print(session, "\tbits                     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void set_RDD_BYTE_1(bdmf_session_handle session, unsigned char *p, const bdmfmon_parm_value_t parm[], int write_index)
{
	int i = 0;
	int j;
	j = i;
	if ((write_index == -1) || (write_index == j))
		MWRITE_8((uint8_t *)(p), (parm[j].number));
	j++;
}

void dump_RDD_SYSTEM_CONFIGURATION_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register SYSTEM_CONFIGURATION_ENTRY\n");

	MREAD_16((uint8_t *)(p), (r));
	bdmf_session_print(session, "\tds_drop_precedence_eligibility_vector= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 2, (r));
	bdmf_session_print(session, "\tus_drop_precedence_eligibility_vector= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 4, 7, 1, (r));
	bdmf_session_print(session, "\tfull_flow_cache_mode     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 4, 6, 1, (r));
	bdmf_session_print(session, "\tl2_flow_cache_mode       = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 4, 5, 1, (r));
	bdmf_session_print(session, "\tiptv_lookup_miss_action  = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 4, 4, 1, (r));
	bdmf_session_print(session, "\treserved0                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 4, 3, 1, (r));
	bdmf_session_print(session, "\tg9991_skip_parser_error_or_generic_class_extra_enable= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 4, 2, 1, (r));
	bdmf_session_print(session, "\tdp_eligibility_vector_used= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 4, 1, 1, (r));
	bdmf_session_print(session, "\tingress_packet_based_mapping_ds_or_us= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 4, 0, 1, (r));
	bdmf_session_print(session, "\ttcp_pure_ack_flows       = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 5, 7, 1, (r));
	bdmf_session_print(session, "\tct_lookup_enabled        = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 5, 6, 1, (r));
	bdmf_session_print(session, "\tct_expect_lookup_enabled = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 5, 5, 1, (r));
	bdmf_session_print(session, "\tpbit_key_mode            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 5, 4, 1, (r));
	bdmf_session_print(session, "\tdbg_power_measure_mode   = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_32((uint8_t *)(p) + 4, 0, 20, (r));
	bdmf_session_print(session, "\treserved1                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void set_RDD_SYSTEM_CONFIGURATION_ENTRY(bdmf_session_handle session, unsigned char *p, const bdmfmon_parm_value_t parm[], int write_index)
{
	int i = 0;
	int j;
	j = i;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p), (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 2, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 4, 7, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 4, 6, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 4, 5, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 4, 4, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 4, 3, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 4, 2, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 4, 1, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 4, 0, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 5, 7, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 5, 6, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 5, 5, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 5, 4, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_32((uint8_t *)(p) + 4, 0, 20, (parm[j].number));
	j++;
}

void dump_RDD_SPDTEST_GEN_CFG(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register SPDTEST_GEN_CFG\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p), 4, 4, (r));
	bdmf_session_print(session, "\ttest_type                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p), 3, 1, (r));
	bdmf_session_print(session, "\tis_on                    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p), 2, 1, (r));
	bdmf_session_print(session, "\tis_endless_tx            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p), 1, 1, (r));
	bdmf_session_print(session, "\tnot_valid_license        = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p), 0, 1, (r));
	bdmf_session_print(session, "\tiperf3_64bit_pktid       = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 1, 5, 3, (r));
	bdmf_session_print(session, "\treserved0                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 1, 0, 5, (r));
	bdmf_session_print(session, "\tinterrupt_id             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 2, (r));
	bdmf_session_print(session, "\ttunnel_offset            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 3, (r));
	bdmf_session_print(session, "\treserved1                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void set_RDD_SPDTEST_GEN_CFG(bdmf_session_handle session, unsigned char *p, const bdmfmon_parm_value_t parm[], int write_index)
{
	int i = 0;
	int j;
	j = i;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p), 4, 4, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p), 3, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p), 2, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p), 1, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p), 0, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 1, 5, 3, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 1, 0, 5, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_8((uint8_t *)(p) + 2, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_8((uint8_t *)(p) + 3, (parm[j].number));
	j++;
}

void dump_RDD_VPORT_CFG_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register VPORT_CFG_ENTRY\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p), 7, 1, (r));
	bdmf_session_print(session, "\texception                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p), 6, 1, (r));
	bdmf_session_print(session, "\tcongestion_flow_control  = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p), 0, 6, (r));
	bdmf_session_print(session, "\tingress_filter_profile   = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 1, 5, 3, (r));
	bdmf_session_print(session, "\treserved0                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 1, 1, 4, (r));
	bdmf_session_print(session, "\tviq                      = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 1, 0, 1, (r));
	bdmf_session_print(session, "\tport_dbg_stat_en         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 2, 7, 1, (r));
	bdmf_session_print(session, "\tmcast_whitelist_skip     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 2, 6, 1, (r));
	bdmf_session_print(session, "\tis_lan                   = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 2, 0, 6, (r));
	bdmf_session_print(session, "\tbb_rx_id                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 3, (r));
	bdmf_session_print(session, "\tcntr_id                  = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void set_RDD_VPORT_CFG_ENTRY(bdmf_session_handle session, unsigned char *p, const bdmfmon_parm_value_t parm[], int write_index)
{
	int i = 0;
	int j;
	j = i;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p), 7, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p), 6, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p), 0, 6, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 1, 5, 3, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 1, 1, 4, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 1, 0, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 2, 7, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 2, 6, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 2, 0, 6, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_8((uint8_t *)(p) + 3, (parm[j].number));
	j++;
}

void dump_RDD_BYTES_4(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register BYTES_4\n");

	MREAD_32((uint8_t *)(p), (r));
	bdmf_session_print(session, "\tbits                     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void set_RDD_BYTES_4(bdmf_session_handle session, unsigned char *p, const bdmfmon_parm_value_t parm[], int write_index)
{
	int i = 0;
	int j;
	j = i;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p), (parm[j].number));
	j++;
}

void dump_RDD_TRACE_EVENT(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register TRACE_EVENT\n");

	RDP_FIELD_MREAD_16((uint8_t *)(p), 4, 12, (r));
	bdmf_session_print(session, "\ttimestamp                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 1, 2, 2, (r));
	bdmf_session_print(session, "\tevent_id                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_32((uint8_t *)(p) + 0, 0, 18, (r));
	bdmf_session_print(session, "\ttrace_event_info         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void set_RDD_TRACE_EVENT(bdmf_session_handle session, unsigned char *p, const bdmfmon_parm_value_t parm[], int write_index)
{
	int i = 0;
	int j;
	j = i;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_16((uint8_t *)(p), 4, 12, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 1, 2, 2, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_32((uint8_t *)(p) + 0, 0, 18, (parm[j].number));
	j++;
}

void dump_RDD_TUNNELS_PARSING_CFG(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register TUNNELS_PARSING_CFG\n");

	MREAD_8((uint8_t *)(p), (r));
	bdmf_session_print(session, "\treserved0                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 1, (r));
	bdmf_session_print(session, "\ttunneling_enable         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 2, (r));
	bdmf_session_print(session, "\tres_offset               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 4, (r));
	bdmf_session_print(session, "\treserved1                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void set_RDD_TUNNELS_PARSING_CFG(bdmf_session_handle session, unsigned char *p, const bdmfmon_parm_value_t parm[], int write_index)
{
	int i = 0;
	int j;
	j = i;
	if ((write_index == -1) || (write_index == j))
		MWRITE_8((uint8_t *)(p), (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_8((uint8_t *)(p) + 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 2, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 4, (parm[j].number));
	j++;
}

void dump_RDD_QUEUE_DYNAMIC_MNG_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register QUEUE_DYNAMIC_MNG_ENTRY\n");

	MREAD_16((uint8_t *)(p), (r));
	bdmf_session_print(session, "\tqm_queue_us_start        = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 2, (r));
	bdmf_session_print(session, "\tqm_queue_us_end          = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 4, (r));
	bdmf_session_print(session, "\tqm_queue_ds_start        = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 6, (r));
	bdmf_session_print(session, "\tqm_queue_ds_end          = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 8, (r));
	bdmf_session_print(session, "\tqm_queue_epon_start      = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 10, (r));
	bdmf_session_print(session, "\tqm_queue_sq_start        = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void set_RDD_QUEUE_DYNAMIC_MNG_ENTRY(bdmf_session_handle session, unsigned char *p, const bdmfmon_parm_value_t parm[], int write_index)
{
	int i = 0;
	int j;
	j = i;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p), (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 2, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 4, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 6, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 8, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 10, (parm[j].number));
	j++;
}

void dump_RDD_BYTES_2(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register BYTES_2\n");

	MREAD_16((uint8_t *)(p), (r));
	bdmf_session_print(session, "\tbits                     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void set_RDD_BYTES_2(bdmf_session_handle session, unsigned char *p, const bdmfmon_parm_value_t parm[], int write_index)
{
	int i = 0;
	int j;
	j = i;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p), (parm[j].number));
	j++;
}

void dump_RDD_VPORT_CFG_EX_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register VPORT_CFG_EX_ENTRY\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p), 7, 1, (r));
	bdmf_session_print(session, "\tloopback_en              = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p), 6, 1, (r));
	bdmf_session_print(session, "\tmirroring_en             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p), 5, 1, (r));
	bdmf_session_print(session, "\tingress_rate_limit       = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p), 1, 4, (r));
	bdmf_session_print(session, "\temac_idx                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p), 0, 1, (r));
	bdmf_session_print(session, "\tdos_attack_drop_disable  = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 1, 7, 1, (r));
	bdmf_session_print(session, "\tprop_tag_enable          = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 1, 0, 7, (r));
	bdmf_session_print(session, "\treserved1                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void set_RDD_VPORT_CFG_EX_ENTRY(bdmf_session_handle session, unsigned char *p, const bdmfmon_parm_value_t parm[], int write_index)
{
	int i = 0;
	int j;
	j = i;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p), 7, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p), 6, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p), 5, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p), 1, 4, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p), 0, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 1, 7, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 1, 0, 7, (parm[j].number));
	j++;
}

void dump_RDD_IPTV_CFG(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register IPTV_CFG\n");

	MREAD_16((uint8_t *)(p), (r));
	bdmf_session_print(session, "\tres_offset               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 2, (r));
	bdmf_session_print(session, "\tcontext_offset           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 4, (r));
	bdmf_session_print(session, "\tkey_offset               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 6, (r));
	bdmf_session_print(session, "\thash_tbl_idx             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 7, (r));
	bdmf_session_print(session, "\tl2_mcast                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void set_RDD_IPTV_CFG(bdmf_session_handle session, unsigned char *p, const bdmfmon_parm_value_t parm[], int write_index)
{
	int i = 0;
	int j;
	j = i;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p), (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 2, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 4, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_8((uint8_t *)(p) + 6, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_8((uint8_t *)(p) + 7, (parm[j].number));
	j++;
}

void dump_RDD_TR471_SPDSVC_RX_PKT_ID(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register TR471_SPDSVC_RX_PKT_ID\n");

	MREAD_32((uint8_t *)(p), (r));
	bdmf_session_print(session, "\tsrc_ipaddr               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 4, (r));
	bdmf_session_print(session, "\tdst_ipaddr               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 8, (r));
	bdmf_session_print(session, "\tsrc_port                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 10, (r));
	bdmf_session_print(session, "\tdst_port                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void set_RDD_TR471_SPDSVC_RX_PKT_ID(bdmf_session_handle session, unsigned char *p, const bdmfmon_parm_value_t parm[], int write_index)
{
	int i = 0;
	int j;
	j = i;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p), (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 4, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 8, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 10, (parm[j].number));
	j++;
}

void dump_RDD_DEBUG_PRINT_INFO(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register DEBUG_PRINT_INFO\n");

	MREAD_32((uint8_t *)(p), (r));
	bdmf_session_print(session, "\taddr_low                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 4, (r));
	bdmf_session_print(session, "\taddr_high                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 8, (r));
	bdmf_session_print(session, "\tsize                     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 12, (r));
	bdmf_session_print(session, "\ttable_id                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_32((uint8_t *)(p) + 12, 0, 24, (r));
	bdmf_session_print(session, "\treserved                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void set_RDD_DEBUG_PRINT_INFO(bdmf_session_handle session, unsigned char *p, const bdmfmon_parm_value_t parm[], int write_index)
{
	int i = 0;
	int j;
	j = i;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p), (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 4, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 8, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_8((uint8_t *)(p) + 12, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_32((uint8_t *)(p) + 12, 0, 24, (parm[j].number));
	j++;
}

void dump_RDD_FPM_POOL_NUMBER(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register FPM_POOL_NUMBER\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p), 2, 6, (r));
	bdmf_session_print(session, "\treserved                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p), 0, 2, (r));
	bdmf_session_print(session, "\tpool_number              = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void set_RDD_FPM_POOL_NUMBER(bdmf_session_handle session, unsigned char *p, const bdmfmon_parm_value_t parm[], int write_index)
{
	int i = 0;
	int j;
	j = i;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p), 2, 6, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p), 0, 2, (parm[j].number));
	j++;
}

void dump_RDD_TCAM_GENERIC(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register TCAM_GENERIC\n");

	MREAD_8((uint8_t *)(p), (r));
	bdmf_session_print(session, "\toffset                   = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 1, 2, 6, (r));
	bdmf_session_print(session, "\treserved                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 1, 0, 2, (r));
	bdmf_session_print(session, "\tlayer                    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void set_RDD_TCAM_GENERIC(bdmf_session_handle session, unsigned char *p, const bdmfmon_parm_value_t parm[], int write_index)
{
	int i = 0;
	int j;
	j = i;
	if ((write_index == -1) || (write_index == j))
		MWRITE_8((uint8_t *)(p), (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 1, 2, 6, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 1, 0, 2, (parm[j].number));
	j++;
}

void dump_RDD_IPTV_DDR_CTX_TABLE_ADDRESS(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register IPTV_DDR_CTX_TABLE_ADDRESS\n");

	MREAD_32((uint8_t *)(p), (r));
	bdmf_session_print(session, "\tlow                      = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 4, (r));
	bdmf_session_print(session, "\thigh                     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void set_RDD_IPTV_DDR_CTX_TABLE_ADDRESS(bdmf_session_handle session, unsigned char *p, const bdmfmon_parm_value_t parm[], int write_index)
{
	int i = 0;
	int j;
	j = i;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p), (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 4, (parm[j].number));
	j++;
}

void dump_RDD_BUFMNG_DESCRIPTOR_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register BUFMNG_DESCRIPTOR_ENTRY\n");

	MREAD_8((uint8_t *)(p), (r));
	bdmf_session_print(session, "\tbufmng_idx               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 1, (r));
	bdmf_session_print(session, "\tbufmng_drop_cntr_idx     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void set_RDD_BUFMNG_DESCRIPTOR_ENTRY(bdmf_session_handle session, unsigned char *p, const bdmfmon_parm_value_t parm[], int write_index)
{
	int i = 0;
	int j;
	j = i;
	if ((write_index == -1) || (write_index == j))
		MWRITE_8((uint8_t *)(p), (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_8((uint8_t *)(p) + 1, (parm[j].number));
	j++;
}

void dump_RDD_BUFMNG_STATUS_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	int i,j;

	bdmf_session_print(session, "  Register BUFMNG_STATUS_ENTRY\n");

	bdmf_session_print(session, "\tdhd_tx_queue_occupancy   =\n\t");
	for (i=0,j=0; i<4; i++)
	{
		MREAD_I_32((uint8_t *)(p), i, (r));
		bdmf_session_print(session, "0x%08x ", (unsigned int)r);
		j++;
	}
	bdmf_session_print(session, "\n");
	bdmf_session_print(session, "\tmax_dhd_tx_queue_occupancy=\n\t");
	for (i=0,j=0; i<4; i++)
	{
		MREAD_I_32((uint8_t *)(p) + 16, i, (r));
		bdmf_session_print(session, "0x%08x ", (unsigned int)r);
		j++;
	}
	bdmf_session_print(session, "\n");
}

void set_RDD_BUFMNG_STATUS_ENTRY(bdmf_session_handle session, unsigned char *p, const bdmfmon_parm_value_t parm[], int write_index)
{
	int i = 0;
	int j;
	j = i;
	for (i=0; i<4; i++)
	{
		if ((write_index == -1) || (write_index == j))
			MWRITE_I_32((uint8_t *)(p), i, (parm[j].number));
		j++;
	}
	
	for (i=0; i<4; i++)
	{
		if ((write_index == -1) || (write_index == j))
			MWRITE_I_32((uint8_t *)(p) + 16, i, (parm[j].number));
		j++;
	}
	
}

void dump_RDD_VPORT_TO_RL_OVERHEAD_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register VPORT_TO_RL_OVERHEAD_ENTRY\n");

	MREAD_8((uint8_t *)(p), (r));
	bdmf_session_print(session, "\trl_overhead              = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void set_RDD_VPORT_TO_RL_OVERHEAD_ENTRY(bdmf_session_handle session, unsigned char *p, const bdmfmon_parm_value_t parm[], int write_index)
{
	int i = 0;
	int j;
	j = i;
	if ((write_index == -1) || (write_index == j))
		MWRITE_8((uint8_t *)(p), (parm[j].number));
	j++;
}

void dump_RDD_DHD_HW_CONFIGURATION(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	int i,j;

	bdmf_session_print(session, "  Register DHD_HW_CONFIGURATION\n");

	MREAD_32((uint8_t *)(p), (r));
	bdmf_session_print(session, "\taggr_timer_period        = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 4, 3, 5, (r));
	bdmf_session_print(session, "\treserved0                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_16((uint8_t *)(p) + 4, 0, 11, (r));
	bdmf_session_print(session, "\tddr_sop_offset           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 6, (r));
	bdmf_session_print(session, "\trx_post_fpm_pool         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 7, (r));
	bdmf_session_print(session, "\toffload_config           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 8, (r));
	bdmf_session_print(session, "\tfpm_thresholds_low       = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 10, (r));
	bdmf_session_print(session, "\tfpm_thresholds_high      = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 12, (r));
	bdmf_session_print(session, "\tfpm_thresholds_excl      = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 14, (r));
	bdmf_session_print(session, "\ttx_headroom              = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	bdmf_session_print(session, "\tradio_to_db_idx_map      =\n\t");
	for (i=0,j=0; i<4; i++)
	{
		MREAD_I_8((uint8_t *)(p) + 16, i, (r));
		bdmf_session_print(session, "0x%08x ", (unsigned int)r);
		j++;
	}
	bdmf_session_print(session, "\n");
}

void set_RDD_DHD_HW_CONFIGURATION(bdmf_session_handle session, unsigned char *p, const bdmfmon_parm_value_t parm[], int write_index)
{
	int i = 0;
	int j;
	j = i;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p), (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 4, 3, 5, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_16((uint8_t *)(p) + 4, 0, 11, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_8((uint8_t *)(p) + 6, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_8((uint8_t *)(p) + 7, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 8, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 10, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 12, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 14, (parm[j].number));
	j++;
	for (i=0; i<4; i++)
	{
		if ((write_index == -1) || (write_index == j))
			MWRITE_I_8((uint8_t *)(p) + 16, i, (parm[j].number));
		j++;
	}
	
}

void dump_RDD_MIRRORING_DESCRIPTOR(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register MIRRORING_DESCRIPTOR\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p), 7, 1, (r));
	bdmf_session_print(session, "\treserved                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p), 1, 6, (r));
	bdmf_session_print(session, "\tdst_vport                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_16((uint8_t *)(p), 0, 9, (r));
	bdmf_session_print(session, "\tdst_queue                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void set_RDD_MIRRORING_DESCRIPTOR(bdmf_session_handle session, unsigned char *p, const bdmfmon_parm_value_t parm[], int write_index)
{
	int i = 0;
	int j;
	j = i;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p), 7, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p), 1, 6, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_16((uint8_t *)(p), 0, 9, (parm[j].number));
	j++;
}

void dump_RDD_NAT_CACHE_CFG(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register NAT_CACHE_CFG\n");

	MREAD_16((uint8_t *)(p), (r));
	bdmf_session_print(session, "\tres_offset               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 2, (r));
	bdmf_session_print(session, "\tcontext_offset           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 4, (r));
	bdmf_session_print(session, "\tkey_offset               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 6, 3, 5, (r));
	bdmf_session_print(session, "\treserved                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 6, 2, 1, (r));
	bdmf_session_print(session, "\tthree_tuple_enable       = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 6, 1, 1, (r));
	bdmf_session_print(session, "\tesp_filter_action        = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 6, 0, 1, (r));
	bdmf_session_print(session, "\tesp_filter_enable        = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void set_RDD_NAT_CACHE_CFG(bdmf_session_handle session, unsigned char *p, const bdmfmon_parm_value_t parm[], int write_index)
{
	int i = 0;
	int j;
	j = i;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p), (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 2, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 4, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 6, 3, 5, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 6, 2, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 6, 1, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 6, 0, 1, (parm[j].number));
	j++;
}

void dump_RDD_IPTV_CLASSIFICATION_CFG_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register IPTV_CLASSIFICATION_CFG_ENTRY\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p), 7, 1, (r));
	bdmf_session_print(session, "\tda_prefix_mode_mac       = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p), 6, 1, (r));
	bdmf_session_print(session, "\tda_prefix_mode_ip        = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p), 5, 1, (r));
	bdmf_session_print(session, "\tiptv_en                  = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p), 0, 5, (r));
	bdmf_session_print(session, "\treserved1                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void set_RDD_IPTV_CLASSIFICATION_CFG_ENTRY(bdmf_session_handle session, unsigned char *p, const bdmfmon_parm_value_t parm[], int write_index)
{
	int i = 0;
	int j;
	j = i;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p), 7, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p), 6, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p), 5, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p), 0, 5, (parm[j].number));
	j++;
}

void dump_RDD_GDX_PARAMS(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register GDX_PARAMS\n");

	MREAD_32((uint8_t *)(p), (r));
	bdmf_session_print(session, "\tbase_addr_low            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 4, (r));
	bdmf_session_print(session, "\tbase_addr_high           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 8, (r));
	bdmf_session_print(session, "\treserved                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 10, (r));
	bdmf_session_print(session, "\thit_value                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 11, (r));
	bdmf_session_print(session, "\tdrop_value               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void set_RDD_GDX_PARAMS(bdmf_session_handle session, unsigned char *p, const bdmfmon_parm_value_t parm[], int write_index)
{
	int i = 0;
	int j;
	j = i;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p), (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 4, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 8, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_8((uint8_t *)(p) + 10, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_8((uint8_t *)(p) + 11, (parm[j].number));
	j++;
}

void dump_RDD_FPM_GLOBAL_CFG(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register FPM_GLOBAL_CFG\n");

	MREAD_32((uint8_t *)(p), (r));
	bdmf_session_print(session, "\tfpm_base_low             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 4, (r));
	bdmf_session_print(session, "\tfpm_base_high            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 8, (r));
	bdmf_session_print(session, "\tfpm_token_size           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 10, (r));
	bdmf_session_print(session, "\tfpm_token_shift          = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 11, (r));
	bdmf_session_print(session, "\tfpm_token_add_shift      = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 12, (r));
	bdmf_session_print(session, "\tfpm_token_inv_mant       = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 14, (r));
	bdmf_session_print(session, "\tfpm_token_inv_exp        = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 15, (r));
	bdmf_session_print(session, "\treserved1                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 16, (r));
	bdmf_session_print(session, "\tddr_token_info_low       = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 20, (r));
	bdmf_session_print(session, "\tddr_token_info_high      = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 24, (r));
	bdmf_session_print(session, "\tfpm_token_num_pool0      = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 25, (r));
	bdmf_session_print(session, "\tfpm_token_num_pool1      = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 26, (r));
	bdmf_session_print(session, "\tfpm_token_num_pool2      = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 27, (r));
	bdmf_session_print(session, "\tfpm_token_num_pool3      = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void set_RDD_FPM_GLOBAL_CFG(bdmf_session_handle session, unsigned char *p, const bdmfmon_parm_value_t parm[], int write_index)
{
	int i = 0;
	int j;
	j = i;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p), (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 4, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 8, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_8((uint8_t *)(p) + 10, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_8((uint8_t *)(p) + 11, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 12, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_8((uint8_t *)(p) + 14, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_8((uint8_t *)(p) + 15, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 16, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 20, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_8((uint8_t *)(p) + 24, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_8((uint8_t *)(p) + 25, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_8((uint8_t *)(p) + 26, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_8((uint8_t *)(p) + 27, (parm[j].number));
	j++;
}

void dump_RDD_PACKET_BUFFER(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	int i,j;

	bdmf_session_print(session, "  Register PACKET_BUFFER\n");

	MREAD_32((uint8_t *)(p), (r));
	bdmf_session_print(session, "\tPACKET_HEADER_EH_ING_timestamp= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 4, 3, 5, (r));
	bdmf_session_print(session, "\tPACKET_HEADER_EH_ING_reserved_1= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_16((uint8_t *)(p) + 4, 0, 11, (r));
	bdmf_session_print(session, "\tPACKET_HEADER_EH_ING_trap_info= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 6, 5, 3, (r));
	bdmf_session_print(session, "\tPACKET_HEADER_EH_ING_reserved_0= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_16((uint8_t *)(p) + 6, 0, 13, (r));
	bdmf_session_print(session, "\tPACKET_HEADER_EH_ING_mac_flow_gem_port_index_llid= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	bdmf_session_print(session, "\tPACKET_HEADER_packet_header=\n\t");
	for (i=0,j=0; i<152; i++)
	{
		MREAD_I_8((uint8_t *)(p) + 8, i, (r));
		bdmf_session_print(session, "0x%08x ", (unsigned int)r);
		j++;
		if (j >= 8)
		{
			j = 0;
			bdmf_session_print(session, "\n\t");
		}
	}
	bdmf_session_print(session, "\n");
	RDP_FIELD_MREAD_8((uint8_t *)(p) + 160, 7, 1, (r));
	bdmf_session_print(session, "\tPARSER_RESULTS_PARSER_SUMMARY_da_filter_match= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 160, 6, 1, (r));
	bdmf_session_print(session, "\tPARSER_RESULTS_PARSER_SUMMARY_ip_fragment= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 160, 5, 1, (r));
	bdmf_session_print(session, "\tPARSER_RESULTS_PARSER_SUMMARY_l4_1588= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 160, 4, 1, (r));
	bdmf_session_print(session, "\tPARSER_RESULTS_PARSER_SUMMARY_tcp_udp= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 160, 3, 1, (r));
	bdmf_session_print(session, "\tPARSER_RESULTS_PARSER_SUMMARY_mc_l3= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 160, 2, 1, (r));
	bdmf_session_print(session, "\tPARSER_RESULTS_PARSER_SUMMARY_broadcast= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 160, 1, 1, (r));
	bdmf_session_print(session, "\tPARSER_RESULTS_PARSER_SUMMARY_multicast= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 160, 0, 1, (r));
	bdmf_session_print(session, "\tPARSER_RESULTS_PARSER_SUMMARY_dos_attack= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 161, 7, 1, (r));
	bdmf_session_print(session, "\tPARSER_RESULTS_PARSER_SUMMARY_mc_l3_ctl= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 161, 6, 1, (r));
	bdmf_session_print(session, "\tPARSER_RESULTS_PARSER_SUMMARY_ip_length_error= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 161, 5, 1, (r));
	bdmf_session_print(session, "\tPARSER_RESULTS_PARSER_SUMMARY_icmpv6= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 161, 4, 1, (r));
	bdmf_session_print(session, "\tPARSER_RESULTS_PARSER_SUMMARY_header_length_error= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 161, 3, 1, (r));
	bdmf_session_print(session, "\tPARSER_RESULTS_PARSER_SUMMARY_checksum_error= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 161, 2, 1, (r));
	bdmf_session_print(session, "\tPARSER_RESULTS_PARSER_SUMMARY_version_mismatch= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 161, 0, 2, (r));
	bdmf_session_print(session, "\tPARSER_RESULTS_PARSER_SUMMARY_l3_protocol= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 162, 7, 1, (r));
	bdmf_session_print(session, "\tPARSER_RESULTS_PARSER_SUMMARY_ipv6_ext_header_filter= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 162, 6, 1, (r));
	bdmf_session_print(session, "\tPARSER_RESULTS_PARSER_SUMMARY_tcp_flag= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 162, 5, 1, (r));
	bdmf_session_print(session, "\tPARSER_RESULTS_PARSER_SUMMARY_exception= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 162, 4, 1, (r));
	bdmf_session_print(session, "\tPARSER_RESULTS_PARSER_SUMMARY_ip_mc_l2= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 162, 3, 1, (r));
	bdmf_session_print(session, "\tPARSER_RESULTS_PARSER_SUMMARY_dhcp= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 162, 2, 1, (r));
	bdmf_session_print(session, "\tPARSER_RESULTS_PARSER_SUMMARY_error= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 162, 1, 1, (r));
	bdmf_session_print(session, "\tPARSER_RESULTS_PARSER_SUMMARY_tunnel= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 162, 0, 1, (r));
	bdmf_session_print(session, "\tPARSER_RESULTS_PARSER_SUMMARY_non_l4_protocol= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 163, 7, 1, (r));
	bdmf_session_print(session, "\tPARSER_RESULTS_PARSER_SUMMARY_exception2= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 163, 6, 1, (r));
	bdmf_session_print(session, "\tPARSER_RESULTS_PARSER_SUMMARY_udp_cs_zero= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 163, 5, 1, (r));
	bdmf_session_print(session, "\tPARSER_RESULTS_PARSER_SUMMARY_first_ip_fragment= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 163, 0, 5, (r));
	bdmf_session_print(session, "\tPARSER_RESULTS_PARSER_SUMMARY_l2_protocol= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 164, 4, 4, (r));
	bdmf_session_print(session, "\tPARSER_RESULTS_PARSER_l4_protocol= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 164, 3, 1, (r));
	bdmf_session_print(session, "\tPARSER_RESULTS_PARSER_v6_ah= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 164, 2, 1, (r));
	bdmf_session_print(session, "\tPARSER_RESULTS_PARSER_v6_dest_opt= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 164, 1, 1, (r));
	bdmf_session_print(session, "\tPARSER_RESULTS_PARSER_v6_route= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 164, 0, 1, (r));
	bdmf_session_print(session, "\tPARSER_RESULTS_PARSER_v6_hop= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 165, (r));
	bdmf_session_print(session, "\tPARSER_RESULTS_PARSER_generic_field= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 166, (r));
	bdmf_session_print(session, "\tPARSER_RESULTS_PARSER_layer3_offset= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 167, (r));
	bdmf_session_print(session, "\tPARSER_RESULTS_PARSER_layer4_offset= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 168, (r));
	bdmf_session_print(session, "\tPARSER_RESULTS_PARSER_outer_vlan= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 170, (r));
	bdmf_session_print(session, "\tPARSER_RESULTS_PARSER_inner_vlan= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 172, 6, 2, (r));
	bdmf_session_print(session, "\tPARSER_RESULTS_PARSER_ethernet_version= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 172, 5, 1, (r));
	bdmf_session_print(session, "\tPARSER_RESULTS_PARSER_p_tag= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 172, 4, 1, (r));
	bdmf_session_print(session, "\tPARSER_RESULTS_PARSER_vid_filter_hit= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 172, 0, 4, (r));
	bdmf_session_print(session, "\tPARSER_RESULTS_PARSER_vid_filter_match= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 173, 7, 1, (r));
	bdmf_session_print(session, "\tPARSER_RESULTS_PARSER_ipv6= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 173, 4, 3, (r));
	bdmf_session_print(session, "\tPARSER_RESULTS_PARSER_tpid_vlan_0= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 173, 3, 1, (r));
	bdmf_session_print(session, "\tPARSER_RESULTS_PARSER_ipv4= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 173, 0, 3, (r));
	bdmf_session_print(session, "\tPARSER_RESULTS_PARSER_tpid_vlan_1= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 174, 7, 1, (r));
	bdmf_session_print(session, "\tPARSER_RESULTS_PARSER_dns= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 174, 4, 3, (r));
	bdmf_session_print(session, "\tPARSER_RESULTS_PARSER_tunnel_type= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 174, 2, 2, (r));
	bdmf_session_print(session, "\tPARSER_RESULTS_PARSER_reserved0= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 174, 1, 1, (r));
	bdmf_session_print(session, "\tPARSER_RESULTS_PARSER_gre_protocol_err= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 174, 0, 1, (r));
	bdmf_session_print(session, "\tPARSER_RESULTS_PARSER_v6_mld= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 175, (r));
	bdmf_session_print(session, "\tPARSER_RESULTS_PARSER_layer2_offset= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 176, 7, 1, (r));
	bdmf_session_print(session, "\tPARSER_RESULTS_PARSER_ip_filter_match= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 176, 6, 1, (r));
	bdmf_session_print(session, "\tPARSER_RESULTS_PARSER_ip_filter_num= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 176, 3, 3, (r));
	bdmf_session_print(session, "\tPARSER_RESULTS_PARSER_reserved2= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 176, 0, 3, (r));
	bdmf_session_print(session, "\tPARSER_RESULTS_PARSER_number_of_vlans= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 177, (r));
	bdmf_session_print(session, "\tPARSER_RESULTS_PARSER_tos= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 178, (r));
	bdmf_session_print(session, "\tPARSER_RESULTS_PARSER_frag_header_offset= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 179, 4, 4, (r));
	bdmf_session_print(session, "\tPARSER_RESULTS_PARSER_da_filter_num= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 179, 0, 4, (r));
	bdmf_session_print(session, "\tPARSER_RESULTS_PARSER_dos_attack_reason= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 180, (r));
	bdmf_session_print(session, "\tPARSER_RESULTS_PARSER_icmpv6_type= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 181, (r));
	bdmf_session_print(session, "\tPARSER_RESULTS_PARSER_ip_ttl= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 182, (r));
	bdmf_session_print(session, "\tPARSER_RESULTS_PARSER_ip_length= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_32((uint8_t *)(p) + 184, 8, 24, (r));
	bdmf_session_print(session, "\tPARSER_RESULTS_PARSER_tunnel_id= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 187, (r));
	bdmf_session_print(session, "\tPARSER_RESULTS_PARSER_reserved1= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 188, (r));
	bdmf_session_print(session, "\tPARSER_RESULTS_PARSER_L2_LKP_ENTRY_da_crc= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 192, (r));
	bdmf_session_print(session, "\tPARSER_RESULTS_PARSER_L2_LKP_ENTRY_sa_crc= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 196, (r));
	bdmf_session_print(session, "\tPARSER_RESULTS_PARSER_L2_LKP_ENTRY_vlan_etype_crc= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 200, (r));
	bdmf_session_print(session, "\tPARSER_RESULTS_PARSER_L2_LKP_ENTRY_tos= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 201, 3, 5, (r));
	bdmf_session_print(session, "\tPARSER_RESULTS_PARSER_L2_LKP_ENTRY_lookup_port= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 201, 0, 3, (r));
	bdmf_session_print(session, "\tPARSER_RESULTS_PARSER_L2_LKP_ENTRY_vlans_num= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 202, 7, 1, (r));
	bdmf_session_print(session, "\tPARSER_RESULTS_PARSER_L2_LKP_ENTRY_valid= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 202, 6, 1, (r));
	bdmf_session_print(session, "\tPARSER_RESULTS_PARSER_L2_LKP_ENTRY_tcp_pure_ack= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 202, 5, 1, (r));
	bdmf_session_print(session, "\tPARSER_RESULTS_PARSER_L2_LKP_ENTRY_llc_snap= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_16((uint8_t *)(p) + 202, 4, 9, (r));
	bdmf_session_print(session, "\tPARSER_RESULTS_PARSER_L2_LKP_ENTRY_unused= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 203, 0, 4, (r));
	bdmf_session_print(session, "\tPARSER_RESULTS_PARSER_L2_LKP_ENTRY_zero= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 204, (r));
	bdmf_session_print(session, "\tPARSER_RESULTS_PARSER_L3_LKP_ENTRY_src_ip= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 208, (r));
	bdmf_session_print(session, "\tPARSER_RESULTS_PARSER_L3_LKP_ENTRY_dst_ip= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 212, (r));
	bdmf_session_print(session, "\tPARSER_RESULTS_PARSER_L3_LKP_ENTRY_src_port= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 214, (r));
	bdmf_session_print(session, "\tPARSER_RESULTS_PARSER_L3_LKP_ENTRY_dst_port= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 216, (r));
	bdmf_session_print(session, "\tPARSER_RESULTS_PARSER_L3_LKP_ENTRY_tos= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 217, 3, 5, (r));
	bdmf_session_print(session, "\tPARSER_RESULTS_PARSER_L3_LKP_ENTRY_lookup_port= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 217, 0, 3, (r));
	bdmf_session_print(session, "\tPARSER_RESULTS_PARSER_L3_LKP_ENTRY_vlans_num= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 218, 7, 1, (r));
	bdmf_session_print(session, "\tPARSER_RESULTS_PARSER_L3_LKP_ENTRY_valid= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 218, 6, 1, (r));
	bdmf_session_print(session, "\tPARSER_RESULTS_PARSER_L3_LKP_ENTRY_tcp_pure_ack= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 218, 5, 1, (r));
	bdmf_session_print(session, "\tPARSER_RESULTS_PARSER_L3_LKP_ENTRY_ipv6= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 218, 4, 1, (r));
	bdmf_session_print(session, "\tPARSER_RESULTS_PARSER_L3_LKP_ENTRY_zero1= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_16((uint8_t *)(p) + 218, 4, 8, (r));
	bdmf_session_print(session, "\tPARSER_RESULTS_PARSER_L3_LKP_ENTRY_protocol= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 219, 0, 4, (r));
	bdmf_session_print(session, "\tPARSER_RESULTS_PARSER_L3_LKP_ENTRY_zero2= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 220, (r));
	bdmf_session_print(session, "\tPARSER_RESULTS_PARSER_EXTENSION_ethernet_type= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 222, (r));
	bdmf_session_print(session, "\tPARSER_RESULTS_PARSER_EXTENSION_third_vlan= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 224, (r));
	bdmf_session_print(session, "\tPARSER_RESULTS_PARSER_IC_LKP_ENTRY_ic_key_0= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 228, (r));
	bdmf_session_print(session, "\tPARSER_RESULTS_PARSER_IC_LKP_ENTRY_ic_key_1= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 232, (r));
	bdmf_session_print(session, "\tPARSER_RESULTS_PARSER_IC_LKP_ENTRY_ic_key_2= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 236, (r));
	bdmf_session_print(session, "\tPARSER_RESULTS_PARSER_IC_LKP_ENTRY_ic_key_3= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 240, (r));
	bdmf_session_print(session, "\tPARSER_RESULTS_PARSER_IC_LKP_ENTRY_ic_key_4= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 244, (r));
	bdmf_session_print(session, "\tPARSER_RESULTS_PARSER_IC_LKP_ENTRY_ic_key_5= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 248, (r));
	bdmf_session_print(session, "\tPARSER_RESULTS_PARSER_IC_LKP_ENTRY_ic_key_6= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 252, (r));
	bdmf_session_print(session, "\tPARSER_RESULTS_PARSER_IC_LKP_ENTRY_ic_key_7= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 256, 7, 1, (r));
	bdmf_session_print(session, "\tPROCESSING_RX_DESCRIPTOR_requeue= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 256, 3, 4, (r));
	bdmf_session_print(session, "\tPROCESSING_RX_DESCRIPTOR_err_type= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_16((uint8_t *)(p) + 256, 2, 9, (r));
	bdmf_session_print(session, "\tPROCESSING_RX_DESCRIPTOR_disp_bb_id= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_32((uint8_t *)(p) + 256, 11, 7, (r));
	bdmf_session_print(session, "\tPROCESSING_RX_DESCRIPTOR_reserved2= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 258, 1, 2, (r));
	bdmf_session_print(session, "\tPROCESSING_RX_DESCRIPTOR_cong_ind= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 258, 0, 1, (r));
	bdmf_session_print(session, "\tPROCESSING_RX_DESCRIPTOR_is_emac= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 259, (r));
	bdmf_session_print(session, "\tPROCESSING_RX_DESCRIPTOR_source_port= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_16((uint8_t *)(p) + 260, 5, 11, (r));
	bdmf_session_print(session, "\tPROCESSING_RX_DESCRIPTOR_serial_num= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_32((uint8_t *)(p) + 260, 14, 7, (r));
	bdmf_session_print(session, "\tPROCESSING_RX_DESCRIPTOR_header_number_number_of_sbpm_bns= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_16((uint8_t *)(p) + 262, 0, 14, (r));
	bdmf_session_print(session, "\tPROCESSING_RX_DESCRIPTOR_second_token= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 264, 6, 2, (r));
	bdmf_session_print(session, "\tPROCESSING_RX_DESCRIPTOR_cong_state= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 264, (r));
	bdmf_session_print(session, "\tPROCESSING_RX_DESCRIPTOR_rx_tx_sn_400g_reorder= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_16((uint8_t *)(p) + 266, 0, 14, (r));
	bdmf_session_print(session, "\tPROCESSING_RX_DESCRIPTOR_packet_length= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 268, 7, 1, (r));
	bdmf_session_print(session, "\tPROCESSING_RX_DESCRIPTOR_error= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 268, 6, 1, (r));
	bdmf_session_print(session, "\tPROCESSING_RX_DESCRIPTOR_ing_congestion= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 268, 2, 4, (r));
	bdmf_session_print(session, "\tPROCESSING_RX_DESCRIPTOR_disp_viq= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_16((uint8_t *)(p) + 268, 6, 4, (r));
	bdmf_session_print(session, "\tPROCESSING_RX_DESCRIPTOR_ing_ch= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_32((uint8_t *)(p) + 268, 15, 7, (r));
	bdmf_session_print(session, "\tPROCESSING_RX_DESCRIPTOR_sop_offset= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 270, 6, 1, (r));
	bdmf_session_print(session, "\tPROCESSING_RX_DESCRIPTOR_reserved1= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_16((uint8_t *)(p) + 270, 0, 14, (r));
	bdmf_session_print(session, "\tPROCESSING_RX_DESCRIPTOR_bn0_first= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 272, 7, 1, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_ENTRY_NATC_CONTROL_ENTRY_done= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 272, 6, 1, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_ENTRY_NATC_CONTROL_ENTRY_natc_hit= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 272, 5, 1, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_ENTRY_NATC_CONTROL_ENTRY_cache_hit= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 272, 0, 5, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_ENTRY_NATC_CONTROL_ENTRY_hw_reserved0= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 273, 4, 4, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_ENTRY_NATC_CONTROL_ENTRY_hash_iter= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 273, 2, 2, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_ENTRY_NATC_CONTROL_ENTRY_hw_reserved1= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_32((uint8_t *)(p) + 272, 0, 18, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_ENTRY_NATC_CONTROL_ENTRY_hash_val= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 276, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_ENTRY_q_bytes_cnt= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 277, 7, 1, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_ENTRY_valid= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 277, 6, 1, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_ENTRY_multicast_flag= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 277, 5, 1, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_ENTRY_is_routed= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 277, 4, 1, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_ENTRY_is_tos_mangle= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 277, 3, 1, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_ENTRY_is_l2_accel= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 277, 2, 1, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_ENTRY_is_wred_high_prio= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 277, 1, 1, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_ENTRY_connection_direction= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 277, 0, 1, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_ENTRY_is_hw_cso= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 278, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_ENTRY_pathstat_idx= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 279, 5, 3, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_ENTRY_spdtest_stream_id= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 279, 0, 5, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_ENTRY_command_list_length_32= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 280, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_ENTRY_lag_union= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 281, 7, 1, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_ENTRY_is_unicast_wfd_nic= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 281, 6, 1, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_ENTRY_is_unicast_wfd_any= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 281, 2, 4, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_ENTRY_priority= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_32((uint8_t *)(p) + 280, 0, 18, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_ENTRY_link_specific_union= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 284, 7, 1, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_ENTRY_wfd_prio= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 284, 5, 2, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_ENTRY_wfd_idx= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 284, 0, 5, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_ENTRY_service_queue_id= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 285, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_ENTRY_pre_exception_actions_union= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 286, 6, 2, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_ENTRY_dhd_flow_priority= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_16((uint8_t *)(p) + 286, 0, 14, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_ENTRY_max_pkt_len= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 288, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_ENTRY_score_union= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 290, 7, 1, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_ENTRY_tcpspdtest_is_upload= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 290, 0, 7, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_ENTRY_vport= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 291, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_ENTRY_tx_adjust= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 292, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_ENTRY_score_dcp_bitmask= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 293, 7, 1, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_ENTRY_is_ingqos_high_prio= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 293, 6, 1, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_ENTRY_coupled_classic_queue= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 293, 0, 6, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_ENTRY_cpu_reason= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 294, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_ENTRY_post_exception_actions_union= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 295, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_ENTRY_policer_id= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_ENTRY_command_list=\n\t");
	for (i=0,j=0; i<100; i++)
	{
		MREAD_I_8((uint8_t *)(p) + 296, i, (r));
		bdmf_session_print(session, "0x%08x ", (unsigned int)r);
		j++;
		if (j >= 8)
		{
			j = 0;
			bdmf_session_print(session, "\n\t");
		}
	}
	bdmf_session_print(session, "\n");
	MREAD_32((uint8_t *)(p) + 272, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_TUNNEL_ENTRY_reserved0= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 276, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_TUNNEL_ENTRY_reserved1= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 280, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_TUNNEL_ENTRY_reserved2= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 284, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_TUNNEL_ENTRY_reserved3= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 288, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_TUNNEL_ENTRY_reserved4= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 292, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_TUNNEL_ENTRY_reserved5= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 296, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_TUNNEL_ENTRY_tunnel_key_offset= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 297, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_TUNNEL_ENTRY_tunnel_inner_pkt_offset= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 298, 2, 6, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_TUNNEL_ENTRY_reserved9= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 298, 1, 1, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_TUNNEL_ENTRY_esp_o_udp= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 298, 0, 1, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_TUNNEL_ENTRY_is_inner_hdr_l3= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 299, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_TUNNEL_ENTRY_reserved10= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 300, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_TUNNEL_ENTRY_tunnel_key_mask= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 304, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_TUNNEL_ENTRY_tunnel_flowid= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 308, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_TUNNEL_ENTRY_reserv_crc_input_2= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 312, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_TUNNEL_ENTRY_reserv_crc_input_3= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 316, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_TUNNEL_ENTRY_reserv_8b_alignment= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 320, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_TUNNEL_ENTRY_crc32_output= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_TUNNEL_ENTRY_reserved11=\n\t");
	for (i=0,j=0; i<72; i++)
	{
		MREAD_I_8((uint8_t *)(p) + 324, i, (r));
		bdmf_session_print(session, "0x%08x ", (unsigned int)r);
		j++;
		if (j >= 8)
		{
			j = 0;
			bdmf_session_print(session, "\n\t");
		}
	}
	bdmf_session_print(session, "\n");
	MREAD_32((uint8_t *)(p) + 272, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_NATC_RESULT_FC_UCAST_FLOOD_CONTEXT_ENTRY_reserved0= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_16((uint8_t *)(p) + 276, 6, 10, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_NATC_RESULT_FC_UCAST_FLOOD_CONTEXT_ENTRY_reserved1= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 277, 5, 1, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_NATC_RESULT_FC_UCAST_FLOOD_CONTEXT_ENTRY_is_routed= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_32((uint8_t *)(p) + 276, 0, 21, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_NATC_RESULT_FC_UCAST_FLOOD_CONTEXT_ENTRY_reserved2= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 280, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_NATC_RESULT_FC_UCAST_FLOOD_CONTEXT_ENTRY_reserved3= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_32((uint8_t *)(p) + 284, 15, 17, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_NATC_RESULT_FC_UCAST_FLOOD_CONTEXT_ENTRY_reserved4= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 286, 6, 1, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_NATC_RESULT_FC_UCAST_FLOOD_CONTEXT_ENTRY_fwd_and_trap= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_16((uint8_t *)(p) + 286, 0, 14, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_NATC_RESULT_FC_UCAST_FLOOD_CONTEXT_ENTRY_max_pkt_len= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_NATC_RESULT_FC_UCAST_FLOOD_CONTEXT_ENTRY_clients_vector=\n\t");
	for (i=0,j=0; i<2; i++)
	{
		MREAD_I_32((uint8_t *)(p) + 288, i, (r));
		bdmf_session_print(session, "0x%08x ", (unsigned int)r);
		j++;
	}
	bdmf_session_print(session, "\n");
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_NATC_RESULT_FC_UCAST_FLOOD_CONTEXT_ENTRY_reserved5=\n\t");
	for (i=0,j=0; i<100; i++)
	{
		MREAD_I_8((uint8_t *)(p) + 296, i, (r));
		bdmf_session_print(session, "0x%08x ", (unsigned int)r);
		j++;
		if (j >= 8)
		{
			j = 0;
			bdmf_session_print(session, "\n\t");
		}
	}
	bdmf_session_print(session, "\n");
	MREAD_32((uint8_t *)(p) + 272, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_ETH_XTM_ENTRY_reserved1= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 276, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_ETH_XTM_ENTRY_reserved2= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_16((uint8_t *)(p) + 280, 3, 13, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_ETH_XTM_ENTRY_reserved3= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 281, 2, 1, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_ETH_XTM_ENTRY_coupled_classic_queue= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 281, 0, 2, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_ETH_XTM_ENTRY_egress_info= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 282, 1, 7, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_ETH_XTM_ENTRY_egress_port= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_16((uint8_t *)(p) + 282, 0, 9, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_ETH_XTM_ENTRY_queue= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 272, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_WFD_NIC_ENTRY_reserved1= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 276, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_WFD_NIC_ENTRY_reserved2= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_16((uint8_t *)(p) + 280, 1, 15, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_WFD_NIC_ENTRY_reserved3= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 281, 0, 1, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_WFD_NIC_ENTRY_is_mcast= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 282, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_WFD_NIC_ENTRY_chain_idx= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 272, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_WFD_DHD_ENTRY_reserved1= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 276, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_WFD_DHD_ENTRY_reserved2= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_16((uint8_t *)(p) + 280, 1, 15, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_WFD_DHD_ENTRY_reserved3= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 281, 0, 1, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_WFD_DHD_ENTRY_is_mcast= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 282, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_WFD_DHD_ENTRY_fr_id_chain_id= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 272, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_RNR_DHD_ENTRY_reserved1= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 276, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_RNR_DHD_ENTRY_reserved2= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_16((uint8_t *)(p) + 280, 1, 15, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_RNR_DHD_ENTRY_reserved3= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 281, 0, 1, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_RNR_DHD_ENTRY_llcsnap_flag= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 282, 6, 2, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_RNR_DHD_ENTRY_radio_idx= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 282, 2, 4, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_RNR_DHD_ENTRY_wifi_ssid= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_16((uint8_t *)(p) + 282, 0, 10, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_RNR_DHD_ENTRY_flow_ring_id= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 272, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_GDX_ENTRY_reserved1= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 276, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_GDX_ENTRY_reserved2= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_16((uint8_t *)(p) + 280, 1, 15, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_GDX_ENTRY_reserved3= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 281, 0, 1, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_GDX_ENTRY_gdx_priority= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 282, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_GDX_ENTRY_gdx_ctx_data= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 272, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_NATC_RESULT_MASTER_MC_FLOW_CACHE_CONTEXT_ENTRY_natc_control= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 276, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_NATC_RESULT_MASTER_MC_FLOW_CACHE_CONTEXT_ENTRY_q_bytes_cnt= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_32((uint8_t *)(p) + 276, 0, 24, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_NATC_RESULT_MASTER_MC_FLOW_CACHE_CONTEXT_ENTRY_reserved0= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 280, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_NATC_RESULT_MASTER_MC_FLOW_CACHE_CONTEXT_ENTRY_pathstat_idx= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 281, 7, 1, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_NATC_RESULT_MASTER_MC_FLOW_CACHE_CONTEXT_ENTRY_is_service_queue= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 281, 2, 5, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_NATC_RESULT_MASTER_MC_FLOW_CACHE_CONTEXT_ENTRY_service_queue= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 281, 1, 1, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_NATC_RESULT_MASTER_MC_FLOW_CACHE_CONTEXT_ENTRY_is_routed= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 281, 0, 1, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_NATC_RESULT_MASTER_MC_FLOW_CACHE_CONTEXT_ENTRY_reserved1= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 282, 7, 1, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_NATC_RESULT_MASTER_MC_FLOW_CACHE_CONTEXT_ENTRY_fwd_and_trap= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 282, 6, 1, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_NATC_RESULT_MASTER_MC_FLOW_CACHE_CONTEXT_ENTRY_reserved2= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 282, 0, 6, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_NATC_RESULT_MASTER_MC_FLOW_CACHE_CONTEXT_ENTRY_reserved3= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 283, 7, 1, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_NATC_RESULT_MASTER_MC_FLOW_CACHE_CONTEXT_ENTRY_is_ctx_ext= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 283, 0, 7, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_NATC_RESULT_MASTER_MC_FLOW_CACHE_CONTEXT_ENTRY_reserved4= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 284, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_NATC_RESULT_MASTER_MC_FLOW_CACHE_CONTEXT_ENTRY_wifi_ssid_vector_0= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 286, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_NATC_RESULT_MASTER_MC_FLOW_CACHE_CONTEXT_ENTRY_wifi_ssid_vector_1= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 288, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_NATC_RESULT_MASTER_MC_FLOW_CACHE_CONTEXT_ENTRY_wifi_ssid_vector_2= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 290, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_NATC_RESULT_MASTER_MC_FLOW_CACHE_CONTEXT_ENTRY_max_pkt_len= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 292, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_NATC_RESULT_MASTER_MC_FLOW_CACHE_CONTEXT_ENTRY_reserved5= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 293, 7, 1, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_NATC_RESULT_MASTER_MC_FLOW_CACHE_CONTEXT_ENTRY_policer_enable= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_32((uint8_t *)(p) + 292, 15, 8, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_NATC_RESULT_MASTER_MC_FLOW_CACHE_CONTEXT_ENTRY_policer_id= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_16((uint8_t *)(p) + 294, 0, 15, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_NATC_RESULT_MASTER_MC_FLOW_CACHE_CONTEXT_ENTRY_reserved6= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 296, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_NATC_RESULT_MASTER_MC_FLOW_CACHE_CONTEXT_ENTRY_reserved7= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_NATC_RESULT_MASTER_MC_FLOW_CACHE_CONTEXT_ENTRY_client_idx_vector=\n\t");
	for (i=0,j=0; i<8; i++)
	{
		MREAD_I_32((uint8_t *)(p) + 300, i, (r));
		bdmf_session_print(session, "0x%08x ", (unsigned int)r);
		j++;
		if (j >= 8)
		{
			j = 0;
			bdmf_session_print(session, "\n\t");
		}
	}
	bdmf_session_print(session, "\n");
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_NATC_RESULT_FLOW_CONTEXT_OVERFLOW_reserved3=\n\t");
	for (i=0,j=0; i<112; i++)
	{
		MREAD_I_8((uint8_t *)(p) + 272, i, (r));
		bdmf_session_print(session, "0x%08x ", (unsigned int)r);
		j++;
		if (j >= 8)
		{
			j = 0;
			bdmf_session_print(session, "\n\t");
		}
	}
	bdmf_session_print(session, "\n");
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_NATC_RESULT_FLOW_CONTEXT_OVERFLOW_start=\n\t");
	for (i=0,j=0; i<12; i++)
	{
		MREAD_I_8((uint8_t *)(p) + 384, i, (r));
		bdmf_session_print(session, "0x%08x ", (unsigned int)r);
		j++;
		if (j >= 8)
		{
			j = 0;
			bdmf_session_print(session, "\n\t");
		}
	}
	bdmf_session_print(session, "\n");
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_NATC_RESULT_NATC_RESULT_ALIGNMENT_reserved3=\n\t");
	for (i=0,j=0; i<4; i++)
	{
		MREAD_I_8((uint8_t *)(p) + 396, i, (r));
		bdmf_session_print(session, "0x%08x ", (unsigned int)r);
		j++;
	}
	bdmf_session_print(session, "\n");
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_NATC_RESULT_EXT_natc_result_buff=\n\t");
	for (i=0,j=0; i<104; i++)
	{
		MREAD_I_8((uint8_t *)(p) + 400, i, (r));
		bdmf_session_print(session, "0x%08x ", (unsigned int)r);
		j++;
		if (j >= 8)
		{
			j = 0;
			bdmf_session_print(session, "\n\t");
		}
	}
	bdmf_session_print(session, "\n");
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_SCORE_SCRATCH_score_report_scratch=\n\t");
	for (i=0,j=0; i<104; i++)
	{
		MREAD_I_8((uint8_t *)(p) + 400, i, (r));
		bdmf_session_print(session, "0x%08x ", (unsigned int)r);
		j++;
		if (j >= 8)
		{
			j = 0;
			bdmf_session_print(session, "\n\t");
		}
	}
	bdmf_session_print(session, "\n");
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_TCAM_result=\n\t");
	for (i=0,j=0; i<48; i++)
	{
		MREAD_I_8((uint8_t *)(p) + 400, i, (r));
		bdmf_session_print(session, "0x%08x ", (unsigned int)r);
		j++;
		if (j >= 8)
		{
			j = 0;
			bdmf_session_print(session, "\n\t");
		}
	}
	bdmf_session_print(session, "\n");
	MREAD_32((uint8_t *)(p) + 448, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_TCAM_PARSER_IC_LKP_ENTRY_ic_key_0= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 452, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_TCAM_PARSER_IC_LKP_ENTRY_ic_key_1= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 456, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_TCAM_PARSER_IC_LKP_ENTRY_ic_key_2= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 460, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_TCAM_PARSER_IC_LKP_ENTRY_ic_key_3= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 464, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_TCAM_PARSER_IC_LKP_ENTRY_ic_key_4= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 468, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_TCAM_PARSER_IC_LKP_ENTRY_ic_key_5= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 472, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_TCAM_PARSER_IC_LKP_ENTRY_ic_key_6= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 476, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_TCAM_PARSER_IC_LKP_ENTRY_ic_key_7= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 504, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_SBPM_ALLOC_REPLY_reserved0= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_32((uint8_t *)(p) + 508, 15, 17, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_SBPM_ALLOC_REPLY_reserved1= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_16((uint8_t *)(p) + 510, 1, 14, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_SBPM_ALLOC_REPLY_alloc_bn= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 511, 0, 1, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_SBPM_ALLOC_REPLY_bn_valid= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 504, 6, 2, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_POLICER_RESULT_ENTRY_color= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_32((uint8_t *)(p) + 504, 0, 30, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_POLICER_RESULT_ENTRY_reserved0= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_NATC_RESULT_EXT_1_natc_result_buff=\n\t");
	for (i=0,j=0; i<8; i++)
	{
		MREAD_I_8((uint8_t *)(p) + 504, i, (r));
		bdmf_session_print(session, "0x%08x ", (unsigned int)r);
		j++;
		if (j >= 8)
		{
			j = 0;
			bdmf_session_print(session, "\n\t");
		}
	}
	bdmf_session_print(session, "\n");
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_SCORE_SCRATCH_1_score_report_scratch_1=\n\t");
	for (i=0,j=0; i<8; i++)
	{
		MREAD_I_8((uint8_t *)(p) + 504, i, (r));
		bdmf_session_print(session, "0x%08x ", (unsigned int)r);
		j++;
		if (j >= 8)
		{
			j = 0;
			bdmf_session_print(session, "\n\t");
		}
	}
	bdmf_session_print(session, "\n");
	bdmf_session_print(session, "\tSCRATCH_RX_MIRROR_FLOW_COPY_128_SCRATCH_reserved0=\n\t");
	for (i=0,j=0; i<128; i++)
	{
		MREAD_I_8((uint8_t *)(p) + 272, i, (r));
		bdmf_session_print(session, "0x%08x ", (unsigned int)r);
		j++;
		if (j >= 8)
		{
			j = 0;
			bdmf_session_print(session, "\n\t");
		}
	}
	bdmf_session_print(session, "\n");
	MREAD_32((uint8_t *)(p) + 400, (r));
	bdmf_session_print(session, "\tSCRATCH_RX_MIRROR_FLOW_SBPM_ALLOC_REPLY_reserved0= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_32((uint8_t *)(p) + 404, 15, 17, (r));
	bdmf_session_print(session, "\tSCRATCH_RX_MIRROR_FLOW_SBPM_ALLOC_REPLY_reserved1= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_16((uint8_t *)(p) + 406, 1, 14, (r));
	bdmf_session_print(session, "\tSCRATCH_RX_MIRROR_FLOW_SBPM_ALLOC_REPLY_alloc_bn= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 407, 0, 1, (r));
	bdmf_session_print(session, "\tSCRATCH_RX_MIRROR_FLOW_SBPM_ALLOC_REPLY_bn_valid= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 272, 7, 1, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_ENTRY_NATC_CONTROL_ENTRY_done= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 272, 6, 1, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_ENTRY_NATC_CONTROL_ENTRY_natc_hit= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 272, 5, 1, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_ENTRY_NATC_CONTROL_ENTRY_cache_hit= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 272, 0, 5, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_ENTRY_NATC_CONTROL_ENTRY_hw_reserved0= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 273, 4, 4, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_ENTRY_NATC_CONTROL_ENTRY_hash_iter= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 273, 2, 2, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_ENTRY_NATC_CONTROL_ENTRY_hw_reserved1= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_32((uint8_t *)(p) + 272, 0, 18, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_ENTRY_NATC_CONTROL_ENTRY_hash_val= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 276, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_ENTRY_q_bytes_cnt= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 277, 7, 1, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_ENTRY_valid= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 277, 6, 1, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_ENTRY_multicast_flag= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 277, 5, 1, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_ENTRY_is_routed= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 277, 4, 1, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_ENTRY_is_tos_mangle= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 277, 3, 1, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_ENTRY_is_l2_accel= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 277, 2, 1, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_ENTRY_is_wred_high_prio= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 277, 1, 1, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_ENTRY_connection_direction= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 277, 0, 1, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_ENTRY_is_hw_cso= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 278, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_ENTRY_pathstat_idx= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 279, 5, 3, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_ENTRY_spdtest_stream_id= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 279, 0, 5, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_ENTRY_command_list_length_32= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 280, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_ENTRY_lag_union= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 281, 7, 1, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_ENTRY_is_unicast_wfd_nic= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 281, 6, 1, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_ENTRY_is_unicast_wfd_any= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 281, 2, 4, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_ENTRY_priority= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_32((uint8_t *)(p) + 280, 0, 18, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_ENTRY_link_specific_union= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 284, 7, 1, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_ENTRY_wfd_prio= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 284, 5, 2, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_ENTRY_wfd_idx= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 284, 0, 5, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_ENTRY_service_queue_id= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 285, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_ENTRY_pre_exception_actions_union= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 286, 6, 2, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_ENTRY_dhd_flow_priority= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_16((uint8_t *)(p) + 286, 0, 14, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_ENTRY_max_pkt_len= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 288, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_ENTRY_score_union= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 290, 7, 1, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_ENTRY_tcpspdtest_is_upload= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 290, 0, 7, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_ENTRY_vport= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 291, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_ENTRY_tx_adjust= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 292, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_ENTRY_score_dcp_bitmask= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 293, 7, 1, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_ENTRY_is_ingqos_high_prio= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 293, 6, 1, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_ENTRY_coupled_classic_queue= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 293, 0, 6, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_ENTRY_cpu_reason= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 294, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_ENTRY_post_exception_actions_union= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 295, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_ENTRY_policer_id= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_ENTRY_command_list=\n\t");
	for (i=0,j=0; i<100; i++)
	{
		MREAD_I_8((uint8_t *)(p) + 296, i, (r));
		bdmf_session_print(session, "0x%08x ", (unsigned int)r);
		j++;
		if (j >= 8)
		{
			j = 0;
			bdmf_session_print(session, "\n\t");
		}
	}
	bdmf_session_print(session, "\n");
	MREAD_32((uint8_t *)(p) + 272, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_TUNNEL_ENTRY_reserved0= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 276, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_TUNNEL_ENTRY_reserved1= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 280, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_TUNNEL_ENTRY_reserved2= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 284, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_TUNNEL_ENTRY_reserved3= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 288, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_TUNNEL_ENTRY_reserved4= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 292, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_TUNNEL_ENTRY_reserved5= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 296, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_TUNNEL_ENTRY_tunnel_key_offset= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 297, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_TUNNEL_ENTRY_tunnel_inner_pkt_offset= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 298, 2, 6, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_TUNNEL_ENTRY_reserved9= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 298, 1, 1, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_TUNNEL_ENTRY_esp_o_udp= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 298, 0, 1, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_TUNNEL_ENTRY_is_inner_hdr_l3= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 299, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_TUNNEL_ENTRY_reserved10= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 300, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_TUNNEL_ENTRY_tunnel_key_mask= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 304, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_TUNNEL_ENTRY_tunnel_flowid= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 308, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_TUNNEL_ENTRY_reserv_crc_input_2= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 312, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_TUNNEL_ENTRY_reserv_crc_input_3= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 316, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_TUNNEL_ENTRY_reserv_8b_alignment= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 320, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_TUNNEL_ENTRY_crc32_output= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_TUNNEL_ENTRY_reserved11=\n\t");
	for (i=0,j=0; i<72; i++)
	{
		MREAD_I_8((uint8_t *)(p) + 324, i, (r));
		bdmf_session_print(session, "0x%08x ", (unsigned int)r);
		j++;
		if (j >= 8)
		{
			j = 0;
			bdmf_session_print(session, "\n\t");
		}
	}
	bdmf_session_print(session, "\n");
	MREAD_32((uint8_t *)(p) + 272, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_NATC_RESULT_FC_UCAST_FLOOD_CONTEXT_ENTRY_reserved0= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_16((uint8_t *)(p) + 276, 6, 10, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_NATC_RESULT_FC_UCAST_FLOOD_CONTEXT_ENTRY_reserved1= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 277, 5, 1, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_NATC_RESULT_FC_UCAST_FLOOD_CONTEXT_ENTRY_is_routed= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_32((uint8_t *)(p) + 276, 0, 21, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_NATC_RESULT_FC_UCAST_FLOOD_CONTEXT_ENTRY_reserved2= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 280, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_NATC_RESULT_FC_UCAST_FLOOD_CONTEXT_ENTRY_reserved3= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_32((uint8_t *)(p) + 284, 15, 17, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_NATC_RESULT_FC_UCAST_FLOOD_CONTEXT_ENTRY_reserved4= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 286, 6, 1, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_NATC_RESULT_FC_UCAST_FLOOD_CONTEXT_ENTRY_fwd_and_trap= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_16((uint8_t *)(p) + 286, 0, 14, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_NATC_RESULT_FC_UCAST_FLOOD_CONTEXT_ENTRY_max_pkt_len= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_NATC_RESULT_FC_UCAST_FLOOD_CONTEXT_ENTRY_clients_vector=\n\t");
	for (i=0,j=0; i<2; i++)
	{
		MREAD_I_32((uint8_t *)(p) + 288, i, (r));
		bdmf_session_print(session, "0x%08x ", (unsigned int)r);
		j++;
	}
	bdmf_session_print(session, "\n");
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_NATC_RESULT_FC_UCAST_FLOOD_CONTEXT_ENTRY_reserved5=\n\t");
	for (i=0,j=0; i<100; i++)
	{
		MREAD_I_8((uint8_t *)(p) + 296, i, (r));
		bdmf_session_print(session, "0x%08x ", (unsigned int)r);
		j++;
		if (j >= 8)
		{
			j = 0;
			bdmf_session_print(session, "\n\t");
		}
	}
	bdmf_session_print(session, "\n");
	MREAD_32((uint8_t *)(p) + 272, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_ETH_XTM_ENTRY_reserved1= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 276, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_ETH_XTM_ENTRY_reserved2= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_16((uint8_t *)(p) + 280, 3, 13, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_ETH_XTM_ENTRY_reserved3= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 281, 2, 1, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_ETH_XTM_ENTRY_coupled_classic_queue= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 281, 0, 2, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_ETH_XTM_ENTRY_egress_info= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 282, 1, 7, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_ETH_XTM_ENTRY_egress_port= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_16((uint8_t *)(p) + 282, 0, 9, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_ETH_XTM_ENTRY_queue= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 272, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_WFD_NIC_ENTRY_reserved1= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 276, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_WFD_NIC_ENTRY_reserved2= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_16((uint8_t *)(p) + 280, 1, 15, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_WFD_NIC_ENTRY_reserved3= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 281, 0, 1, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_WFD_NIC_ENTRY_is_mcast= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 282, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_WFD_NIC_ENTRY_chain_idx= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 272, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_WFD_DHD_ENTRY_reserved1= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 276, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_WFD_DHD_ENTRY_reserved2= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_16((uint8_t *)(p) + 280, 1, 15, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_WFD_DHD_ENTRY_reserved3= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 281, 0, 1, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_WFD_DHD_ENTRY_is_mcast= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 282, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_WFD_DHD_ENTRY_fr_id_chain_id= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 272, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_RNR_DHD_ENTRY_reserved1= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 276, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_RNR_DHD_ENTRY_reserved2= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_16((uint8_t *)(p) + 280, 1, 15, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_RNR_DHD_ENTRY_reserved3= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 281, 0, 1, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_RNR_DHD_ENTRY_llcsnap_flag= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 282, 6, 2, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_RNR_DHD_ENTRY_radio_idx= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 282, 2, 4, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_RNR_DHD_ENTRY_wifi_ssid= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_16((uint8_t *)(p) + 282, 0, 10, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_RNR_DHD_ENTRY_flow_ring_id= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 272, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_GDX_ENTRY_reserved1= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 276, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_GDX_ENTRY_reserved2= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_16((uint8_t *)(p) + 280, 1, 15, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_GDX_ENTRY_reserved3= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 281, 0, 1, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_GDX_ENTRY_gdx_priority= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 282, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_GDX_ENTRY_gdx_ctx_data= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 272, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_NATC_RESULT_MASTER_MC_FLOW_CACHE_CONTEXT_ENTRY_natc_control= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 276, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_NATC_RESULT_MASTER_MC_FLOW_CACHE_CONTEXT_ENTRY_q_bytes_cnt= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_32((uint8_t *)(p) + 276, 0, 24, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_NATC_RESULT_MASTER_MC_FLOW_CACHE_CONTEXT_ENTRY_reserved0= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 280, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_NATC_RESULT_MASTER_MC_FLOW_CACHE_CONTEXT_ENTRY_pathstat_idx= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 281, 7, 1, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_NATC_RESULT_MASTER_MC_FLOW_CACHE_CONTEXT_ENTRY_is_service_queue= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 281, 2, 5, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_NATC_RESULT_MASTER_MC_FLOW_CACHE_CONTEXT_ENTRY_service_queue= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 281, 1, 1, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_NATC_RESULT_MASTER_MC_FLOW_CACHE_CONTEXT_ENTRY_is_routed= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 281, 0, 1, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_NATC_RESULT_MASTER_MC_FLOW_CACHE_CONTEXT_ENTRY_reserved1= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 282, 7, 1, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_NATC_RESULT_MASTER_MC_FLOW_CACHE_CONTEXT_ENTRY_fwd_and_trap= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 282, 6, 1, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_NATC_RESULT_MASTER_MC_FLOW_CACHE_CONTEXT_ENTRY_reserved2= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 282, 0, 6, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_NATC_RESULT_MASTER_MC_FLOW_CACHE_CONTEXT_ENTRY_reserved3= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 283, 7, 1, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_NATC_RESULT_MASTER_MC_FLOW_CACHE_CONTEXT_ENTRY_is_ctx_ext= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 283, 0, 7, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_NATC_RESULT_MASTER_MC_FLOW_CACHE_CONTEXT_ENTRY_reserved4= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 284, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_NATC_RESULT_MASTER_MC_FLOW_CACHE_CONTEXT_ENTRY_wifi_ssid_vector_0= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 286, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_NATC_RESULT_MASTER_MC_FLOW_CACHE_CONTEXT_ENTRY_wifi_ssid_vector_1= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 288, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_NATC_RESULT_MASTER_MC_FLOW_CACHE_CONTEXT_ENTRY_wifi_ssid_vector_2= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 290, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_NATC_RESULT_MASTER_MC_FLOW_CACHE_CONTEXT_ENTRY_max_pkt_len= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 292, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_NATC_RESULT_MASTER_MC_FLOW_CACHE_CONTEXT_ENTRY_reserved5= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 293, 7, 1, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_NATC_RESULT_MASTER_MC_FLOW_CACHE_CONTEXT_ENTRY_policer_enable= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_32((uint8_t *)(p) + 292, 15, 8, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_NATC_RESULT_MASTER_MC_FLOW_CACHE_CONTEXT_ENTRY_policer_id= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_16((uint8_t *)(p) + 294, 0, 15, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_NATC_RESULT_MASTER_MC_FLOW_CACHE_CONTEXT_ENTRY_reserved6= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 296, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_NATC_RESULT_MASTER_MC_FLOW_CACHE_CONTEXT_ENTRY_reserved7= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_NATC_RESULT_MASTER_MC_FLOW_CACHE_CONTEXT_ENTRY_client_idx_vector=\n\t");
	for (i=0,j=0; i<8; i++)
	{
		MREAD_I_32((uint8_t *)(p) + 300, i, (r));
		bdmf_session_print(session, "0x%08x ", (unsigned int)r);
		j++;
		if (j >= 8)
		{
			j = 0;
			bdmf_session_print(session, "\n\t");
		}
	}
	bdmf_session_print(session, "\n");
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_NATC_RESULT_FLOW_CONTEXT_OVERFLOW_reserved3=\n\t");
	for (i=0,j=0; i<112; i++)
	{
		MREAD_I_8((uint8_t *)(p) + 272, i, (r));
		bdmf_session_print(session, "0x%08x ", (unsigned int)r);
		j++;
		if (j >= 8)
		{
			j = 0;
			bdmf_session_print(session, "\n\t");
		}
	}
	bdmf_session_print(session, "\n");
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_NATC_RESULT_FLOW_CONTEXT_OVERFLOW_start=\n\t");
	for (i=0,j=0; i<12; i++)
	{
		MREAD_I_8((uint8_t *)(p) + 384, i, (r));
		bdmf_session_print(session, "0x%08x ", (unsigned int)r);
		j++;
		if (j >= 8)
		{
			j = 0;
			bdmf_session_print(session, "\n\t");
		}
	}
	bdmf_session_print(session, "\n");
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_NATC_RESULT_NATC_RESULT_ALIGNMENT_reserved3=\n\t");
	for (i=0,j=0; i<4; i++)
	{
		MREAD_I_8((uint8_t *)(p) + 396, i, (r));
		bdmf_session_print(session, "0x%08x ", (unsigned int)r);
		j++;
	}
	bdmf_session_print(session, "\n");
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_MC_FLOW_SCRATCH_natc_command_data=\n\t");
	for (i=0,j=0; i<36; i++)
	{
		MREAD_I_8((uint8_t *)(p) + 400, i, (r));
		bdmf_session_print(session, "0x%08x ", (unsigned int)r);
		j++;
		if (j >= 8)
		{
			j = 0;
			bdmf_session_print(session, "\n\t");
		}
	}
	bdmf_session_print(session, "\n");
	MREAD_16((uint8_t *)(p) + 436, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_MC_FLOW_SCRATCH_ttl_proto_backup= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 438, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_MC_FLOW_SCRATCH_ip_csum_backup= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_MC_FLOW_SCRATCH_header_buffer=\n\t");
	for (i=0,j=0; i<32; i++)
	{
		MREAD_I_8((uint8_t *)(p) + 440, i, (r));
		bdmf_session_print(session, "0x%08x ", (unsigned int)r);
		j++;
		if (j >= 8)
		{
			j = 0;
			bdmf_session_print(session, "\n\t");
		}
	}
	bdmf_session_print(session, "\n");
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_MC_FLOW_SCRATCH_client_indexes=\n\t");
	for (i=0,j=0; i<8; i++)
	{
		MREAD_I_32((uint8_t *)(p) + 472, i, (r));
		bdmf_session_print(session, "0x%08x ", (unsigned int)r);
		j++;
		if (j >= 8)
		{
			j = 0;
			bdmf_session_print(session, "\n\t");
		}
	}
	bdmf_session_print(session, "\n");
	RDP_FIELD_MREAD_8((uint8_t *)(p) + 400, 7, 1, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_IPTV_DDR_CONTEXT_ENTRY_valid= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 400, 6, 1, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_IPTV_DDR_CONTEXT_ENTRY_ip_ver= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 400, 5, 1, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_IPTV_DDR_CONTEXT_ENTRY_any_vid= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 400, 4, 1, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_IPTV_DDR_CONTEXT_ENTRY_any_src_ip= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_16((uint8_t *)(p) + 400, 0, 12, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_IPTV_DDR_CONTEXT_ENTRY_vid= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 402, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_IPTV_DDR_CONTEXT_ENTRY_next_entry_idx= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 404, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_IPTV_DDR_CONTEXT_ENTRY_ssid_vector_0_or_flooding_vport= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 406, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_IPTV_DDR_CONTEXT_ENTRY_ssid_vector_1= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 408, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_IPTV_DDR_CONTEXT_ENTRY_ssid_vector_2= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 410, 5, 3, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_IPTV_DDR_CONTEXT_ENTRY_reserved4= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 410, 0, 5, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_IPTV_DDR_CONTEXT_ENTRY_rdd_vport= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 411, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_IPTV_DDR_CONTEXT_ENTRY_reserved3= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 412, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_IPTV_DDR_CONTEXT_ENTRY_reserved2= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 413, 6, 2, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_IPTV_DDR_CONTEXT_ENTRY_pool_num= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 413, 0, 6, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_IPTV_DDR_CONTEXT_ENTRY_replications= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 414, 2, 6, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_IPTV_DDR_CONTEXT_ENTRY_reserved1= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_16((uint8_t *)(p) + 414, 0, 10, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_IPTV_DDR_CONTEXT_ENTRY_cntr_id= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 416, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_IPTV_DDR_CONTEXT_ENTRY_egress_ports_vector= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 420, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_IPTV_DDR_CONTEXT_ENTRY_src_ip= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_IPTV_DDR_CONTEXT_ENTRY_src_ipv6_addr=\n\t");
	for (i=0,j=0; i<16; i++)
	{
		MREAD_I_8((uint8_t *)(p) + 424, i, (r));
		bdmf_session_print(session, "0x%08x ", (unsigned int)r);
		j++;
		if (j >= 8)
		{
			j = 0;
			bdmf_session_print(session, "\n\t");
		}
	}
	bdmf_session_print(session, "\n");
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_IPTV_DDR_CONTEXT_ENTRY_dst_ipv6_addr=\n\t");
	for (i=0,j=0; i<16; i++)
	{
		MREAD_I_8((uint8_t *)(p) + 440, i, (r));
		bdmf_session_print(session, "0x%08x ", (unsigned int)r);
		j++;
		if (j >= 8)
		{
			j = 0;
			bdmf_session_print(session, "\n\t");
		}
	}
	bdmf_session_print(session, "\n");
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_IPTV_DDR_CONTEXT_ENTRY_result=\n\t");
	for (i=0,j=0; i<32; i++)
	{
		MREAD_I_8((uint8_t *)(p) + 456, i, (r));
		bdmf_session_print(session, "0x%08x ", (unsigned int)r);
		j++;
		if (j >= 8)
		{
			j = 0;
			bdmf_session_print(session, "\n\t");
		}
	}
	bdmf_session_print(session, "\n");
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_COPY_64_SCRATCH_reserved0=\n\t");
	for (i=0,j=0; i<64; i++)
	{
		MREAD_I_8((uint8_t *)(p) + 400, i, (r));
		bdmf_session_print(session, "0x%08x ", (unsigned int)r);
		j++;
		if (j >= 8)
		{
			j = 0;
			bdmf_session_print(session, "\n\t");
		}
	}
	bdmf_session_print(session, "\n");
	MREAD_32((uint8_t *)(p) + 504, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_SBPM_ALLOC_REPLY_reserved0= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_32((uint8_t *)(p) + 508, 15, 17, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_SBPM_ALLOC_REPLY_reserved1= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_16((uint8_t *)(p) + 510, 1, 14, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_SBPM_ALLOC_REPLY_alloc_bn= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 511, 0, 1, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_SBPM_ALLOC_REPLY_bn_valid= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void set_RDD_PACKET_BUFFER(bdmf_session_handle session, unsigned char *p, const bdmfmon_parm_value_t parm[], int write_index)
{
	int i = 0;
	int j;
	j = i;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p), (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 4, 3, 5, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_16((uint8_t *)(p) + 4, 0, 11, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 6, 5, 3, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_16((uint8_t *)(p) + 6, 0, 13, (parm[j].number));
	j++;
	for (i=0; i<152; i++)
	{
		if ((write_index == -1) || (write_index == j))
			MWRITE_I_8((uint8_t *)(p) + 8, i, (parm[j].number));
		j++;
	}
	
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 160, 7, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 160, 6, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 160, 5, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 160, 4, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 160, 3, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 160, 2, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 160, 1, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 160, 0, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 161, 7, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 161, 6, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 161, 5, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 161, 4, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 161, 3, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 161, 2, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 161, 0, 2, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 162, 7, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 162, 6, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 162, 5, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 162, 4, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 162, 3, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 162, 2, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 162, 1, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 162, 0, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 163, 7, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 163, 6, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 163, 5, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 163, 0, 5, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 164, 4, 4, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 164, 3, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 164, 2, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 164, 1, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 164, 0, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_8((uint8_t *)(p) + 165, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_8((uint8_t *)(p) + 166, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_8((uint8_t *)(p) + 167, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 168, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 170, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 172, 6, 2, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 172, 5, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 172, 4, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 172, 0, 4, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 173, 7, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 173, 4, 3, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 173, 3, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 173, 0, 3, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 174, 7, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 174, 4, 3, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 174, 2, 2, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 174, 1, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 174, 0, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_8((uint8_t *)(p) + 175, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 176, 7, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 176, 6, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 176, 3, 3, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 176, 0, 3, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_8((uint8_t *)(p) + 177, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_8((uint8_t *)(p) + 178, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 179, 4, 4, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 179, 0, 4, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_8((uint8_t *)(p) + 180, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_8((uint8_t *)(p) + 181, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 182, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_32((uint8_t *)(p) + 184, 8, 24, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_8((uint8_t *)(p) + 187, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 188, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 192, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 196, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_8((uint8_t *)(p) + 200, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 201, 3, 5, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 201, 0, 3, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 202, 7, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 202, 6, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 202, 5, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_16((uint8_t *)(p) + 202, 4, 9, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 203, 0, 4, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 204, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 208, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 212, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 214, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_8((uint8_t *)(p) + 216, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 217, 3, 5, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 217, 0, 3, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 218, 7, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 218, 6, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 218, 5, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 218, 4, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_16((uint8_t *)(p) + 218, 4, 8, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 219, 0, 4, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 220, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 222, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 224, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 228, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 232, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 236, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 240, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 244, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 248, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 252, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 256, 7, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 256, 3, 4, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_16((uint8_t *)(p) + 256, 2, 9, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_32((uint8_t *)(p) + 256, 11, 7, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 258, 1, 2, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 258, 0, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_8((uint8_t *)(p) + 259, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_16((uint8_t *)(p) + 260, 5, 11, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_32((uint8_t *)(p) + 260, 14, 7, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_16((uint8_t *)(p) + 262, 0, 14, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 264, 6, 2, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 264, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_16((uint8_t *)(p) + 266, 0, 14, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 268, 7, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 268, 6, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 268, 2, 4, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_16((uint8_t *)(p) + 268, 6, 4, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_32((uint8_t *)(p) + 268, 15, 7, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 270, 6, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_16((uint8_t *)(p) + 270, 0, 14, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 272, 7, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 272, 6, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 272, 5, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 272, 0, 5, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 273, 4, 4, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 273, 2, 2, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_32((uint8_t *)(p) + 272, 0, 18, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_8((uint8_t *)(p) + 276, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 277, 7, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 277, 6, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 277, 5, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 277, 4, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 277, 3, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 277, 2, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 277, 1, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 277, 0, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_8((uint8_t *)(p) + 278, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 279, 5, 3, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 279, 0, 5, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_8((uint8_t *)(p) + 280, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 281, 7, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 281, 6, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 281, 2, 4, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_32((uint8_t *)(p) + 280, 0, 18, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 284, 7, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 284, 5, 2, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 284, 0, 5, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_8((uint8_t *)(p) + 285, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 286, 6, 2, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_16((uint8_t *)(p) + 286, 0, 14, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 288, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 290, 7, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 290, 0, 7, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_8((uint8_t *)(p) + 291, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_8((uint8_t *)(p) + 292, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 293, 7, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 293, 6, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 293, 0, 6, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_8((uint8_t *)(p) + 294, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_8((uint8_t *)(p) + 295, (parm[j].number));
	j++;
	for (i=0; i<100; i++)
	{
		if ((write_index == -1) || (write_index == j))
			MWRITE_I_8((uint8_t *)(p) + 296, i, (parm[j].number));
		j++;
	}
	
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 272, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 276, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 280, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 284, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 288, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 292, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_8((uint8_t *)(p) + 296, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_8((uint8_t *)(p) + 297, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 298, 2, 6, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 298, 1, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 298, 0, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_8((uint8_t *)(p) + 299, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 300, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 304, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 308, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 312, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 316, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 320, (parm[j].number));
	j++;
	for (i=0; i<72; i++)
	{
		if ((write_index == -1) || (write_index == j))
			MWRITE_I_8((uint8_t *)(p) + 324, i, (parm[j].number));
		j++;
	}
	
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 272, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_16((uint8_t *)(p) + 276, 6, 10, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 277, 5, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_32((uint8_t *)(p) + 276, 0, 21, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 280, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_32((uint8_t *)(p) + 284, 15, 17, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 286, 6, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_16((uint8_t *)(p) + 286, 0, 14, (parm[j].number));
	j++;
	for (i=0; i<2; i++)
	{
		if ((write_index == -1) || (write_index == j))
			MWRITE_I_32((uint8_t *)(p) + 288, i, (parm[j].number));
		j++;
	}
	
	for (i=0; i<100; i++)
	{
		if ((write_index == -1) || (write_index == j))
			MWRITE_I_8((uint8_t *)(p) + 296, i, (parm[j].number));
		j++;
	}
	
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 272, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 276, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_16((uint8_t *)(p) + 280, 3, 13, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 281, 2, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 281, 0, 2, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 282, 1, 7, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_16((uint8_t *)(p) + 282, 0, 9, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 272, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 276, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_16((uint8_t *)(p) + 280, 1, 15, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 281, 0, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 282, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 272, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 276, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_16((uint8_t *)(p) + 280, 1, 15, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 281, 0, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 282, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 272, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 276, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_16((uint8_t *)(p) + 280, 1, 15, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 281, 0, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 282, 6, 2, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 282, 2, 4, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_16((uint8_t *)(p) + 282, 0, 10, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 272, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 276, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_16((uint8_t *)(p) + 280, 1, 15, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 281, 0, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 282, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 272, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_8((uint8_t *)(p) + 276, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_32((uint8_t *)(p) + 276, 0, 24, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_8((uint8_t *)(p) + 280, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 281, 7, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 281, 2, 5, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 281, 1, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 281, 0, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 282, 7, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 282, 6, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 282, 0, 6, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 283, 7, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 283, 0, 7, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 284, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 286, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 288, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 290, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_8((uint8_t *)(p) + 292, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 293, 7, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_32((uint8_t *)(p) + 292, 15, 8, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_16((uint8_t *)(p) + 294, 0, 15, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 296, (parm[j].number));
	j++;
	for (i=0; i<8; i++)
	{
		if ((write_index == -1) || (write_index == j))
			MWRITE_I_32((uint8_t *)(p) + 300, i, (parm[j].number));
		j++;
	}
	
	for (i=0; i<112; i++)
	{
		if ((write_index == -1) || (write_index == j))
			MWRITE_I_8((uint8_t *)(p) + 272, i, (parm[j].number));
		j++;
	}
	
	for (i=0; i<12; i++)
	{
		if ((write_index == -1) || (write_index == j))
			MWRITE_I_8((uint8_t *)(p) + 384, i, (parm[j].number));
		j++;
	}
	
	for (i=0; i<4; i++)
	{
		if ((write_index == -1) || (write_index == j))
			MWRITE_I_8((uint8_t *)(p) + 396, i, (parm[j].number));
		j++;
	}
	
	for (i=0; i<104; i++)
	{
		if ((write_index == -1) || (write_index == j))
			MWRITE_I_8((uint8_t *)(p) + 400, i, (parm[j].number));
		j++;
	}
	
	for (i=0; i<104; i++)
	{
		if ((write_index == -1) || (write_index == j))
			MWRITE_I_8((uint8_t *)(p) + 400, i, (parm[j].number));
		j++;
	}
	
	for (i=0; i<48; i++)
	{
		if ((write_index == -1) || (write_index == j))
			MWRITE_I_8((uint8_t *)(p) + 400, i, (parm[j].number));
		j++;
	}
	
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 448, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 452, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 456, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 460, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 464, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 468, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 472, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 476, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 504, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_32((uint8_t *)(p) + 508, 15, 17, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_16((uint8_t *)(p) + 510, 1, 14, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 511, 0, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 504, 6, 2, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_32((uint8_t *)(p) + 504, 0, 30, (parm[j].number));
	j++;
	for (i=0; i<8; i++)
	{
		if ((write_index == -1) || (write_index == j))
			MWRITE_I_8((uint8_t *)(p) + 504, i, (parm[j].number));
		j++;
	}
	
	for (i=0; i<8; i++)
	{
		if ((write_index == -1) || (write_index == j))
			MWRITE_I_8((uint8_t *)(p) + 504, i, (parm[j].number));
		j++;
	}
	
	for (i=0; i<128; i++)
	{
		if ((write_index == -1) || (write_index == j))
			MWRITE_I_8((uint8_t *)(p) + 272, i, (parm[j].number));
		j++;
	}
	
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 400, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_32((uint8_t *)(p) + 404, 15, 17, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_16((uint8_t *)(p) + 406, 1, 14, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 407, 0, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 272, 7, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 272, 6, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 272, 5, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 272, 0, 5, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 273, 4, 4, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 273, 2, 2, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_32((uint8_t *)(p) + 272, 0, 18, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_8((uint8_t *)(p) + 276, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 277, 7, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 277, 6, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 277, 5, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 277, 4, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 277, 3, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 277, 2, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 277, 1, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 277, 0, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_8((uint8_t *)(p) + 278, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 279, 5, 3, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 279, 0, 5, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_8((uint8_t *)(p) + 280, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 281, 7, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 281, 6, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 281, 2, 4, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_32((uint8_t *)(p) + 280, 0, 18, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 284, 7, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 284, 5, 2, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 284, 0, 5, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_8((uint8_t *)(p) + 285, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 286, 6, 2, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_16((uint8_t *)(p) + 286, 0, 14, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 288, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 290, 7, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 290, 0, 7, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_8((uint8_t *)(p) + 291, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_8((uint8_t *)(p) + 292, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 293, 7, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 293, 6, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 293, 0, 6, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_8((uint8_t *)(p) + 294, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_8((uint8_t *)(p) + 295, (parm[j].number));
	j++;
	for (i=0; i<100; i++)
	{
		if ((write_index == -1) || (write_index == j))
			MWRITE_I_8((uint8_t *)(p) + 296, i, (parm[j].number));
		j++;
	}
	
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 272, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 276, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 280, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 284, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 288, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 292, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_8((uint8_t *)(p) + 296, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_8((uint8_t *)(p) + 297, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 298, 2, 6, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 298, 1, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 298, 0, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_8((uint8_t *)(p) + 299, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 300, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 304, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 308, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 312, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 316, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 320, (parm[j].number));
	j++;
	for (i=0; i<72; i++)
	{
		if ((write_index == -1) || (write_index == j))
			MWRITE_I_8((uint8_t *)(p) + 324, i, (parm[j].number));
		j++;
	}
	
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 272, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_16((uint8_t *)(p) + 276, 6, 10, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 277, 5, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_32((uint8_t *)(p) + 276, 0, 21, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 280, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_32((uint8_t *)(p) + 284, 15, 17, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 286, 6, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_16((uint8_t *)(p) + 286, 0, 14, (parm[j].number));
	j++;
	for (i=0; i<2; i++)
	{
		if ((write_index == -1) || (write_index == j))
			MWRITE_I_32((uint8_t *)(p) + 288, i, (parm[j].number));
		j++;
	}
	
	for (i=0; i<100; i++)
	{
		if ((write_index == -1) || (write_index == j))
			MWRITE_I_8((uint8_t *)(p) + 296, i, (parm[j].number));
		j++;
	}
	
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 272, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 276, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_16((uint8_t *)(p) + 280, 3, 13, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 281, 2, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 281, 0, 2, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 282, 1, 7, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_16((uint8_t *)(p) + 282, 0, 9, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 272, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 276, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_16((uint8_t *)(p) + 280, 1, 15, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 281, 0, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 282, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 272, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 276, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_16((uint8_t *)(p) + 280, 1, 15, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 281, 0, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 282, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 272, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 276, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_16((uint8_t *)(p) + 280, 1, 15, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 281, 0, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 282, 6, 2, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 282, 2, 4, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_16((uint8_t *)(p) + 282, 0, 10, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 272, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 276, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_16((uint8_t *)(p) + 280, 1, 15, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 281, 0, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 282, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 272, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_8((uint8_t *)(p) + 276, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_32((uint8_t *)(p) + 276, 0, 24, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_8((uint8_t *)(p) + 280, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 281, 7, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 281, 2, 5, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 281, 1, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 281, 0, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 282, 7, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 282, 6, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 282, 0, 6, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 283, 7, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 283, 0, 7, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 284, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 286, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 288, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 290, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_8((uint8_t *)(p) + 292, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 293, 7, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_32((uint8_t *)(p) + 292, 15, 8, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_16((uint8_t *)(p) + 294, 0, 15, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 296, (parm[j].number));
	j++;
	for (i=0; i<8; i++)
	{
		if ((write_index == -1) || (write_index == j))
			MWRITE_I_32((uint8_t *)(p) + 300, i, (parm[j].number));
		j++;
	}
	
	for (i=0; i<112; i++)
	{
		if ((write_index == -1) || (write_index == j))
			MWRITE_I_8((uint8_t *)(p) + 272, i, (parm[j].number));
		j++;
	}
	
	for (i=0; i<12; i++)
	{
		if ((write_index == -1) || (write_index == j))
			MWRITE_I_8((uint8_t *)(p) + 384, i, (parm[j].number));
		j++;
	}
	
	for (i=0; i<4; i++)
	{
		if ((write_index == -1) || (write_index == j))
			MWRITE_I_8((uint8_t *)(p) + 396, i, (parm[j].number));
		j++;
	}
	
	for (i=0; i<36; i++)
	{
		if ((write_index == -1) || (write_index == j))
			MWRITE_I_8((uint8_t *)(p) + 400, i, (parm[j].number));
		j++;
	}
	
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 436, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 438, (parm[j].number));
	j++;
	for (i=0; i<32; i++)
	{
		if ((write_index == -1) || (write_index == j))
			MWRITE_I_8((uint8_t *)(p) + 440, i, (parm[j].number));
		j++;
	}
	
	for (i=0; i<8; i++)
	{
		if ((write_index == -1) || (write_index == j))
			MWRITE_I_32((uint8_t *)(p) + 472, i, (parm[j].number));
		j++;
	}
	
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 400, 7, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 400, 6, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 400, 5, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 400, 4, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_16((uint8_t *)(p) + 400, 0, 12, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 402, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 404, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 406, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 408, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 410, 5, 3, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 410, 0, 5, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_8((uint8_t *)(p) + 411, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_8((uint8_t *)(p) + 412, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 413, 6, 2, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 413, 0, 6, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 414, 2, 6, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_16((uint8_t *)(p) + 414, 0, 10, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 416, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 420, (parm[j].number));
	j++;
	for (i=0; i<16; i++)
	{
		if ((write_index == -1) || (write_index == j))
			MWRITE_I_8((uint8_t *)(p) + 424, i, (parm[j].number));
		j++;
	}
	
	for (i=0; i<16; i++)
	{
		if ((write_index == -1) || (write_index == j))
			MWRITE_I_8((uint8_t *)(p) + 440, i, (parm[j].number));
		j++;
	}
	
	for (i=0; i<32; i++)
	{
		if ((write_index == -1) || (write_index == j))
			MWRITE_I_8((uint8_t *)(p) + 456, i, (parm[j].number));
		j++;
	}
	
	for (i=0; i<64; i++)
	{
		if ((write_index == -1) || (write_index == j))
			MWRITE_I_8((uint8_t *)(p) + 400, i, (parm[j].number));
		j++;
	}
	
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 504, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_32((uint8_t *)(p) + 508, 15, 17, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_16((uint8_t *)(p) + 510, 1, 14, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 511, 0, 1, (parm[j].number));
	j++;
}

void dump_RDD_TCAM_TABLE_CFG(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register TCAM_TABLE_CFG\n");

	MREAD_8((uint8_t *)(p), (r));
	bdmf_session_print(session, "\tall_fields               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void set_RDD_TCAM_TABLE_CFG(bdmf_session_handle session, unsigned char *p, const bdmfmon_parm_value_t parm[], int write_index)
{
	int i = 0;
	int j;
	j = i;
	if ((write_index == -1) || (write_index == j))
		MWRITE_8((uint8_t *)(p), (parm[j].number));
	j++;
}

void dump_RDD_INGRESS_FILTER_CFG(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register INGRESS_FILTER_CFG\n");

	MREAD_16((uint8_t *)(p), (r));
	bdmf_session_print(session, "\tres_offset               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void set_RDD_INGRESS_FILTER_CFG(bdmf_session_handle session, unsigned char *p, const bdmfmon_parm_value_t parm[], int write_index)
{
	int i = 0;
	int j;
	j = i;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p), (parm[j].number));
	j++;
}

void dump_RDD_MULTICAST_KEY_MASK_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register MULTICAST_KEY_MASK_ENTRY\n");

	MREAD_16((uint8_t *)(p), (r));
	bdmf_session_print(session, "\tvlan_mask                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 2, (r));
	bdmf_session_print(session, "\tetype_mask               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void set_RDD_MULTICAST_KEY_MASK_ENTRY(bdmf_session_handle session, unsigned char *p, const bdmfmon_parm_value_t parm[], int write_index)
{
	int i = 0;
	int j;
	j = i;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p), (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 2, (parm[j].number));
	j++;
}

void dump_RDD_RX_FLOW_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register RX_FLOW_ENTRY\n");

	MREAD_8((uint8_t *)(p), (r));
	bdmf_session_print(session, "\tcntr_id                  = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 1, 7, 1, (r));
	bdmf_session_print(session, "\tflow_dest                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 1, 0, 7, (r));
	bdmf_session_print(session, "\treserved                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void set_RDD_RX_FLOW_ENTRY(bdmf_session_handle session, unsigned char *p, const bdmfmon_parm_value_t parm[], int write_index)
{
	int i = 0;
	int j;
	j = i;
	if ((write_index == -1) || (write_index == j))
		MWRITE_8((uint8_t *)(p), (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 1, 7, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 1, 0, 7, (parm[j].number));
	j++;
}

void dump_RDD_TX_FLOW_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register TX_FLOW_ENTRY\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p), 7, 1, (r));
	bdmf_session_print(session, "\tvalid                    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p), 5, 2, (r));
	bdmf_session_print(session, "\treserved                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p), 0, 5, (r));
	bdmf_session_print(session, "\tqos_table_ptr            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void set_RDD_TX_FLOW_ENTRY(bdmf_session_handle session, unsigned char *p, const bdmfmon_parm_value_t parm[], int write_index)
{
	int i = 0;
	int j;
	j = i;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p), 7, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p), 5, 2, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p), 0, 5, (parm[j].number));
	j++;
}

void dump_RDD_PARSER_KEY_BUILD_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	int i,j;

	bdmf_session_print(session, "  Register PARSER_KEY_BUILD_ENTRY\n");

	bdmf_session_print(session, "\tconfig                   =\n\t");
	for (i=0,j=0; i<2; i++)
	{
		MREAD_I_32((uint8_t *)(p), i, (r));
		bdmf_session_print(session, "0x%08x ", (unsigned int)r);
		j++;
	}
	bdmf_session_print(session, "\n");
	bdmf_session_print(session, "\tkey_cmds                 =\n\t");
	for (i=0,j=0; i<7; i++)
	{
		MREAD_I_32((uint8_t *)(p) + 8, i, (r));
		bdmf_session_print(session, "0x%08x ", (unsigned int)r);
		j++;
	}
	bdmf_session_print(session, "\n");
}

void set_RDD_PARSER_KEY_BUILD_ENTRY(bdmf_session_handle session, unsigned char *p, const bdmfmon_parm_value_t parm[], int write_index)
{
	int i = 0;
	int j;
	j = i;
	for (i=0; i<2; i++)
	{
		if ((write_index == -1) || (write_index == j))
			MWRITE_I_32((uint8_t *)(p), i, (parm[j].number));
		j++;
	}
	
	for (i=0; i<7; i++)
	{
		if ((write_index == -1) || (write_index == j))
			MWRITE_I_32((uint8_t *)(p) + 8, i, (parm[j].number));
		j++;
	}
	
}

void dump_RDD_INGRESS_FILTER_CTRL(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register INGRESS_FILTER_CTRL\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p), 7, 1, (r));
	bdmf_session_print(session, "\treserved                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_32((uint8_t *)(p), 0, 31, (r));
	bdmf_session_print(session, "\tenable_vector            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 4, 7, 1, (r));
	bdmf_session_print(session, "\tcpu_bypass               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_32((uint8_t *)(p) + 4, 0, 31, (r));
	bdmf_session_print(session, "\taction_vector            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void set_RDD_INGRESS_FILTER_CTRL(bdmf_session_handle session, unsigned char *p, const bdmfmon_parm_value_t parm[], int write_index)
{
	int i = 0;
	int j;
	j = i;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p), 7, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_32((uint8_t *)(p), 0, 31, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 4, 7, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_32((uint8_t *)(p) + 4, 0, 31, (parm[j].number));
	j++;
}

void dump_RDD_PROCESSING_RX_DESCRIPTOR(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register PROCESSING_RX_DESCRIPTOR\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p), 7, 1, (r));
	bdmf_session_print(session, "\trequeue                  = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p), 3, 4, (r));
	bdmf_session_print(session, "\terr_type                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_16((uint8_t *)(p), 2, 9, (r));
	bdmf_session_print(session, "\tdisp_bb_id               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_32((uint8_t *)(p) + 0, 11, 7, (r));
	bdmf_session_print(session, "\treserved2                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 2, 1, 2, (r));
	bdmf_session_print(session, "\tcong_ind                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 2, 0, 1, (r));
	bdmf_session_print(session, "\tis_emac                  = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 3, (r));
	bdmf_session_print(session, "\tsource_port              = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_16((uint8_t *)(p) + 4, 5, 11, (r));
	bdmf_session_print(session, "\tserial_num               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_32((uint8_t *)(p) + 4, 14, 7, (r));
	bdmf_session_print(session, "\theader_number_number_of_sbpm_bns= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_16((uint8_t *)(p) + 6, 0, 14, (r));
	bdmf_session_print(session, "\tsecond_token             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 8, 6, 2, (r));
	bdmf_session_print(session, "\tcong_state               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 8, (r));
	bdmf_session_print(session, "\trx_tx_sn_400g_reorder    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_16((uint8_t *)(p) + 10, 0, 14, (r));
	bdmf_session_print(session, "\tpacket_length            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 12, 7, 1, (r));
	bdmf_session_print(session, "\terror                    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 12, 6, 1, (r));
	bdmf_session_print(session, "\ting_congestion           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 12, 2, 4, (r));
	bdmf_session_print(session, "\tdisp_viq                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_16((uint8_t *)(p) + 12, 6, 4, (r));
	bdmf_session_print(session, "\ting_ch                   = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_32((uint8_t *)(p) + 12, 15, 7, (r));
	bdmf_session_print(session, "\tsop_offset               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 14, 6, 1, (r));
	bdmf_session_print(session, "\treserved1                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_16((uint8_t *)(p) + 14, 0, 14, (r));
	bdmf_session_print(session, "\tbn0_first                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void set_RDD_PROCESSING_RX_DESCRIPTOR(bdmf_session_handle session, unsigned char *p, const bdmfmon_parm_value_t parm[], int write_index)
{
	int i = 0;
	int j;
	j = i;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p), 7, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p), 3, 4, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_16((uint8_t *)(p), 2, 9, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_32((uint8_t *)(p) + 0, 11, 7, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 2, 1, 2, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 2, 0, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_8((uint8_t *)(p) + 3, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_16((uint8_t *)(p) + 4, 5, 11, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_32((uint8_t *)(p) + 4, 14, 7, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_16((uint8_t *)(p) + 6, 0, 14, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 8, 6, 2, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 8, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_16((uint8_t *)(p) + 10, 0, 14, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 12, 7, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 12, 6, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 12, 2, 4, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_16((uint8_t *)(p) + 12, 6, 4, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_32((uint8_t *)(p) + 12, 15, 7, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 14, 6, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_16((uint8_t *)(p) + 14, 0, 14, (parm[j].number));
	j++;
}

void dump_RDD_PAUSE_QUANTA_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register PAUSE_QUANTA_ENTRY\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p), 7, 1, (r));
	bdmf_session_print(session, "\tignore                   = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_16((uint8_t *)(p), 0, 15, (r));
	bdmf_session_print(session, "\ttime_unit                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void set_RDD_PAUSE_QUANTA_ENTRY(bdmf_session_handle session, unsigned char *p, const bdmfmon_parm_value_t parm[], int write_index)
{
	int i = 0;
	int j;
	j = i;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p), 7, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_16((uint8_t *)(p), 0, 15, (parm[j].number));
	j++;
}

void dump_RDD_MAC_TYPE_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register MAC_TYPE_ENTRY\n");

	MREAD_8((uint8_t *)(p), (r));
	bdmf_session_print(session, "\ttype                     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void set_RDD_MAC_TYPE_ENTRY(bdmf_session_handle session, unsigned char *p, const bdmfmon_parm_value_t parm[], int write_index)
{
	int i = 0;
	int j;
	j = i;
	if ((write_index == -1) || (write_index == j))
		MWRITE_8((uint8_t *)(p), (parm[j].number));
	j++;
}

void dump_RDD_TCPSPDTEST_STREAM(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	int i,j;

	bdmf_session_print(session, "  Register TCPSPDTEST_STREAM\n");

	MREAD_8((uint8_t *)(p), (r));
	bdmf_session_print(session, "\tTCPSPDTEST_ENGINE_CONN_INFO_cpu_rx_rdd_queue= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 1, (r));
	bdmf_session_print(session, "\tTCPSPDTEST_ENGINE_CONN_INFO_up_pppoe_hdr_ofs= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 2, (r));
	bdmf_session_print(session, "\tTCPSPDTEST_ENGINE_CONN_INFO_up_peer_rx_scale= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 3, (r));
	bdmf_session_print(session, "\tTCPSPDTEST_ENGINE_CONN_INFO_sack_permitted= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 4, (r));
	bdmf_session_print(session, "\tTCPSPDTEST_ENGINE_CONN_INFO_up_tx_mss= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 6, (r));
	bdmf_session_print(session, "\tTCPSPDTEST_ENGINE_CONN_INFO_up_tx_max_pd_len= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 8, (r));
	bdmf_session_print(session, "\tTCPSPDTEST_ENGINE_TCB_rx_pkts= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 12, (r));
	bdmf_session_print(session, "\tTCPSPDTEST_ENGINE_TCB_txed_pkts= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 16, (r));
	bdmf_session_print(session, "\tTCPSPDTEST_ENGINE_TCB_freed_pkts= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 20, (r));
	bdmf_session_print(session, "\tTCPSPDTEST_ENGINE_TCB_ack_seq= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 24, (r));
	bdmf_session_print(session, "\tTCPSPDTEST_ENGINE_TCB_tx_seq= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 28, (r));
	bdmf_session_print(session, "\tTCPSPDTEST_ENGINE_TCB_bad_pkts= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 29, (r));
	bdmf_session_print(session, "\tTCPSPDTEST_ENGINE_TCB_up_cwnd_control= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 30, (r));
	bdmf_session_print(session, "\tTCPSPDTEST_ENGINE_TCB_src_port= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 32, (r));
	bdmf_session_print(session, "\tTCPSPDTEST_ENGINE_TCB_rx_bytes= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 36, (r));
	bdmf_session_print(session, "\tTCPSPDTEST_ENGINE_TCB_rx_bytes_1= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 40, (r));
	bdmf_session_print(session, "\tTCPSPDTEST_ENGINE_TCB_expected_bytes= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 44, (r));
	bdmf_session_print(session, "\tTCPSPDTEST_ENGINE_TCB_expected_bytes_1= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 48, (r));
	bdmf_session_print(session, "\tTCPSPDTEST_ENGINE_TCB_up_to_send_bytes= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 52, (r));
	bdmf_session_print(session, "\tTCPSPDTEST_ENGINE_TCB_up_to_send_bytes_1= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 56, (r));
	bdmf_session_print(session, "\tTCPSPDTEST_ENGINE_TCB_no_dispatcher_credits= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 58, (r));
	bdmf_session_print(session, "\tTCPSPDTEST_ENGINE_TCB_no_pktgen_tx_credits= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 60, (r));
	bdmf_session_print(session, "\tTCPSPDTEST_ENGINE_TCB_up_cwnd= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 64, (r));
	bdmf_session_print(session, "\tTCPSPDTEST_ENGINE_TCB_up_cwnd_thr= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 68, (r));
	bdmf_session_print(session, "\tTCPSPDTEST_ENGINE_TCB_up_cwnd_initial= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 72, (r));
	bdmf_session_print(session, "\tTCPSPDTEST_ENGINE_TCB_up_cwnd_max= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 76, (r));
	bdmf_session_print(session, "\tTCPSPDTEST_ENGINE_TCB_up_cwnd_budget= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 80, (r));
	bdmf_session_print(session, "\tTCPSPDTEST_ENGINE_TCB_up_last_ack_seq= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 84, (r));
	bdmf_session_print(session, "\tTCPSPDTEST_ENGINE_TCB_up_last_ack_time= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 88, (r));
	bdmf_session_print(session, "\tTCPSPDTEST_ENGINE_TCB_up_sack_bytes= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 92, (r));
	bdmf_session_print(session, "\tTCPSPDTEST_ENGINE_TCB_up_fast_retrans_trig_cnt= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 96, (r));
	bdmf_session_print(session, "\tTCPSPDTEST_ENGINE_TCB_up_dup_ack_cnt= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 100, (r));
	bdmf_session_print(session, "\tTCPSPDTEST_ENGINE_TCB_up_dup_ack_seq_done= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 104, (r));
	bdmf_session_print(session, "\tTCPSPDTEST_ENGINE_TCB_up_dup_ack_total_pkts_cnt= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 106, (r));
	bdmf_session_print(session, "\tTCPSPDTEST_ENGINE_TCB_up_fast_retrans_cnt= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 108, (r));
	bdmf_session_print(session, "\tTCPSPDTEST_ENGINE_TCB_up_err_dn_pkt_cnt= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 109, (r));
	bdmf_session_print(session, "\tTCPSPDTEST_ENGINE_TCB_dn_err_up_pkt_cnt= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 110, (r));
	bdmf_session_print(session, "\tTCPSPDTEST_ENGINE_TCB_up_cwnd_rtt_factor= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 111, (r));
	bdmf_session_print(session, "\tTCPSPDTEST_ENGINE_TCB_err_dst_port_cnt= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 112, (r));
	bdmf_session_print(session, "\tTCPSPDTEST_ENGINE_TCB_dynack_pkts_thr= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 113, (r));
	bdmf_session_print(session, "\tTCPSPDTEST_ENGINE_TCB_dynack_pkts_thr_cnt= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 114, (r));
	bdmf_session_print(session, "\tTCPSPDTEST_ENGINE_TCB_dynack_thr_upd_rate= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 115, (r));
	bdmf_session_print(session, "\tTCPSPDTEST_ENGINE_TCB_dynack_thr_upd_rate_cnt= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 116, (r));
	bdmf_session_print(session, "\tTCPSPDTEST_ENGINE_TCB_is_stream_active= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 117, (r));
	bdmf_session_print(session, "\tTCPSPDTEST_ENGINE_TCB_reserved1= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 118, (r));
	bdmf_session_print(session, "\tTCPSPDTEST_ENGINE_TCB_up_dup_ack_state= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 119, 6, 2, (r));
	bdmf_session_print(session, "\tTCPSPDTEST_ENGINE_TCB_reserved2= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 119, 5, 1, (r));
	bdmf_session_print(session, "\tTCPSPDTEST_ENGINE_TCB_dn_is_pkt_drop_driver_wakeup= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 119, 4, 1, (r));
	bdmf_session_print(session, "\tTCPSPDTEST_ENGINE_TCB_dn_is_pkt_drop_force_ack= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 119, 3, 1, (r));
	bdmf_session_print(session, "\tTCPSPDTEST_ENGINE_TCB_up_is_rx_win_stop= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 119, 2, 1, (r));
	bdmf_session_print(session, "\tTCPSPDTEST_ENGINE_TCB_up_is_timeout= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 119, 1, 1, (r));
	bdmf_session_print(session, "\tTCPSPDTEST_ENGINE_TCB_up_rto_event= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 119, 0, 1, (r));
	bdmf_session_print(session, "\tTCPSPDTEST_ENGINE_TCB_is_upload= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 120, (r));
	bdmf_session_print(session, "\tTCPSPDTEST_ENGINE_PKT_DROP_drop_state= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 121, (r));
	bdmf_session_print(session, "\tTCPSPDTEST_ENGINE_PKT_DROP_is_win_full= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 122, (r));
	bdmf_session_print(session, "\tTCPSPDTEST_ENGINE_PKT_DROP_num_errs= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 124, (r));
	bdmf_session_print(session, "\tTCPSPDTEST_ENGINE_PKT_DROP_wr_ofs= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 126, (r));
	bdmf_session_print(session, "\tTCPSPDTEST_ENGINE_PKT_DROP_rd_ofs= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 128, (r));
	bdmf_session_print(session, "\tTCPSPDTEST_ENGINE_PKT_DROP_ack_seq_err_start= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 132, (r));
	bdmf_session_print(session, "\tTCPSPDTEST_ENGINE_PKT_DROP_saved_rx_l4_seq= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 136, (r));
	bdmf_session_print(session, "\tTCPSPDTEST_ENGINE_PKT_DROP_saved_ring_rd_wr_ofs= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 140, (r));
	bdmf_session_print(session, "\tTCPSPDTEST_ENGINE_PKT_DROP_saved_ring_rd_wr_ofs_1= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 144, (r));
	bdmf_session_print(session, "\tTCPSPDTEST_ENGINE_PKT_DROP_saved_rx_l3_hdr_len= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 148, (r));
	bdmf_session_print(session, "\tTCPSPDTEST_ENGINE_PKT_DROP_saved_rx_l3_payload_len= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 152, (r));
	bdmf_session_print(session, "\tTCPSPDTEST_ENGINE_PKT_DROP_saved_packet_descriptor_0= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 156, (r));
	bdmf_session_print(session, "\tTCPSPDTEST_ENGINE_PKT_DROP_saved_packet_descriptor_0_1= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 160, (r));
	bdmf_session_print(session, "\tTCPSPDTEST_ENGINE_PKT_DROP_saved_packet_descriptor_2= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 164, (r));
	bdmf_session_print(session, "\tTCPSPDTEST_ENGINE_PKT_DROP_saved_packet_descriptor_2_1= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 168, (r));
	bdmf_session_print(session, "\tTCPSPDTEST_ENGINE_RING_rto_tx_seq_addr_low= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 172, (r));
	bdmf_session_print(session, "\tTCPSPDTEST_ENGINE_RING_rto_tx_seq_addr_high= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 176, (r));
	bdmf_session_print(session, "\tTCPSPDTEST_ENGINE_RING_rto_tx_seq_send_cnt= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 180, (r));
	bdmf_session_print(session, "\tTCPSPDTEST_ENGINE_RING_rto_tx_seq_drop_cnt= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 184, (r));
	bdmf_session_print(session, "\tTCPSPDTEST_ENGINE_RING_rto_retrans_addr_low= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 188, (r));
	bdmf_session_print(session, "\tTCPSPDTEST_ENGINE_RING_rto_retrans_addr_high= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 192, (r));
	bdmf_session_print(session, "\tTCPSPDTEST_ENGINE_RING_sack_opt_addr_low= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 196, (r));
	bdmf_session_print(session, "\tTCPSPDTEST_ENGINE_RING_sack_opt_addr_high= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 200, (r));
	bdmf_session_print(session, "\tTCPSPDTEST_ENGINE_RING_sack_opt_send_cnt= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 204, (r));
	bdmf_session_print(session, "\tTCPSPDTEST_ENGINE_RING_sack_opt_drop_cnt= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 208, (r));
	bdmf_session_print(session, "\tTCPSPDTEST_ENGINE_RING_pkt_drop_rx_seq_addr_low= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 212, (r));
	bdmf_session_print(session, "\tTCPSPDTEST_ENGINE_RING_pkt_drop_rx_seq_addr_high= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 216, (r));
	bdmf_session_print(session, "\tTCPSPDTEST_ENGINE_RING_pkt_drop_rx_seq_send_cnt= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 220, (r));
	bdmf_session_print(session, "\tTCPSPDTEST_ENGINE_RING_pkt_drop_rx_seq_drop_cnt= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 224, (r));
	bdmf_session_print(session, "\tTCPSPDTEST_ENGINE_RING_sack_opt_total_parsed_cnt= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 228, (r));
	bdmf_session_print(session, "\tTCPSPDTEST_ENGINE_RING_rto_retrans_receive_cnt= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 232, (r));
	bdmf_session_print(session, "\tTCPSPDTEST_ENGINE_RING_rto_tx_seq_scratchpad_metadata_wr_ofs= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 234, (r));
	bdmf_session_print(session, "\tTCPSPDTEST_ENGINE_RING_reserved1= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 236, (r));
	bdmf_session_print(session, "\tTCPSPDTEST_ENGINE_RING_rto_tx_seq_coalescing_timeout_cnt= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 240, (r));
	bdmf_session_print(session, "\tTCPSPDTEST_ENGINE_RING_rto_tx_seq_coalescing_fill_pkt_cnt= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 244, (r));
	bdmf_session_print(session, "\tTCPSPDTEST_ENGINE_RING_rto_tx_seq_coalescing_last_time= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	bdmf_session_print(session, "\tTCPSPDTEST_ENGINE_RING_scratchpad=\n\t");
	for (i=0,j=0; i<8; i++)
	{
		MREAD_I_32((uint8_t *)(p) + 248, i, (r));
		bdmf_session_print(session, "0x%08x ", (unsigned int)r);
		j++;
		if (j >= 8)
		{
			j = 0;
			bdmf_session_print(session, "\n\t");
		}
	}
	bdmf_session_print(session, "\n");
	bdmf_session_print(session, "\tTCPSPDTEST_ENGINE_RING_rto_tx_seq_scratchpad_metadata=\n\t");
	for (i=0,j=0; i<16; i++)
	{
		MREAD_I_32((uint8_t *)(p) + 280, i, (r));
		bdmf_session_print(session, "0x%08x ", (unsigned int)r);
		j++;
		if (j >= 8)
		{
			j = 0;
			bdmf_session_print(session, "\n\t");
		}
	}
	bdmf_session_print(session, "\n");
	bdmf_session_print(session, "\tTCPSPDTEST_ENGINE_RING_sack_opt_scratchpad_metadata=\n\t");
	for (i=0,j=0; i<8; i++)
	{
		MREAD_I_32((uint8_t *)(p) + 344, i, (r));
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

void set_RDD_TCPSPDTEST_STREAM(bdmf_session_handle session, unsigned char *p, const bdmfmon_parm_value_t parm[], int write_index)
{
	int i = 0;
	int j;
	j = i;
	if ((write_index == -1) || (write_index == j))
		MWRITE_8((uint8_t *)(p), (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_8((uint8_t *)(p) + 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_8((uint8_t *)(p) + 2, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_8((uint8_t *)(p) + 3, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 4, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 6, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 8, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 12, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 16, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 20, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 24, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_8((uint8_t *)(p) + 28, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_8((uint8_t *)(p) + 29, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 30, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 32, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 36, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 40, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 44, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 48, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 52, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 56, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 58, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 60, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 64, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 68, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 72, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 76, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 80, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 84, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 88, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 92, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 96, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 100, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 104, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 106, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_8((uint8_t *)(p) + 108, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_8((uint8_t *)(p) + 109, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_8((uint8_t *)(p) + 110, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_8((uint8_t *)(p) + 111, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_8((uint8_t *)(p) + 112, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_8((uint8_t *)(p) + 113, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_8((uint8_t *)(p) + 114, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_8((uint8_t *)(p) + 115, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_8((uint8_t *)(p) + 116, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_8((uint8_t *)(p) + 117, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_8((uint8_t *)(p) + 118, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 119, 6, 2, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 119, 5, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 119, 4, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 119, 3, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 119, 2, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 119, 1, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 119, 0, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_8((uint8_t *)(p) + 120, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_8((uint8_t *)(p) + 121, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 122, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 124, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 126, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 128, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 132, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 136, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 140, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 144, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 148, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 152, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 156, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 160, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 164, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 168, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 172, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 176, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 180, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 184, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 188, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 192, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 196, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 200, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 204, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 208, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 212, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 216, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 220, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 224, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 228, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 232, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 234, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 236, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 240, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 244, (parm[j].number));
	j++;
	for (i=0; i<8; i++)
	{
		if ((write_index == -1) || (write_index == j))
			MWRITE_I_32((uint8_t *)(p) + 248, i, (parm[j].number));
		j++;
	}
	
	for (i=0; i<16; i++)
	{
		if ((write_index == -1) || (write_index == j))
			MWRITE_I_32((uint8_t *)(p) + 280, i, (parm[j].number));
		j++;
	}
	
	for (i=0; i<8; i++)
	{
		if ((write_index == -1) || (write_index == j))
			MWRITE_I_32((uint8_t *)(p) + 344, i, (parm[j].number));
		j++;
	}
	
}

void dump_RDD_BYTES_8(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	int i,j;

	bdmf_session_print(session, "  Register BYTES_8\n");

	bdmf_session_print(session, "\tbits                     =\n\t");
	for (i=0,j=0; i<2; i++)
	{
		MREAD_I_32((uint8_t *)(p), i, (r));
		bdmf_session_print(session, "0x%08x ", (unsigned int)r);
		j++;
	}
	bdmf_session_print(session, "\n");
}

void set_RDD_BYTES_8(bdmf_session_handle session, unsigned char *p, const bdmfmon_parm_value_t parm[], int write_index)
{
	int i = 0;
	int j;
	j = i;
	for (i=0; i<2; i++)
	{
		if ((write_index == -1) || (write_index == j))
			MWRITE_I_32((uint8_t *)(p), i, (parm[j].number));
		j++;
	}
	
}

void dump_RDD_SPDSVC_ANALYZER_PARAMS(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register SPDSVC_ANALYZER_PARAMS\n");

	MREAD_32((uint8_t *)(p), (r));
	bdmf_session_print(session, "\tstream                   = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 4, (r));
	bdmf_session_print(session, "\tstream1                  = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 8, (r));
	bdmf_session_print(session, "\tstream2                  = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 12, (r));
	bdmf_session_print(session, "\tstream3                  = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void set_RDD_SPDSVC_ANALYZER_PARAMS(bdmf_session_handle session, unsigned char *p, const bdmfmon_parm_value_t parm[], int write_index)
{
	int i = 0;
	int j;
	j = i;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p), (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 4, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 8, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 12, (parm[j].number));
	j++;
}

void dump_RDD_SPDSVC_TCPSPDTEST_COMMON_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register SPDSVC_TCPSPDTEST_COMMON_ENTRY\n");

	MREAD_8((uint8_t *)(p), (r));
	bdmf_session_print(session, "\ttcpspdtest_upload_is_tcpspdtest_timeout_wakeup= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 1, (r));
	bdmf_session_print(session, "\ttcpspdtest_download_is_tcpspdtest_timeout_wakeup= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void set_RDD_SPDSVC_TCPSPDTEST_COMMON_ENTRY(bdmf_session_handle session, unsigned char *p, const bdmfmon_parm_value_t parm[], int write_index)
{
	int i = 0;
	int j;
	j = i;
	if ((write_index == -1) || (write_index == j))
		MWRITE_8((uint8_t *)(p), (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_8((uint8_t *)(p) + 1, (parm[j].number));
	j++;
}

void dump_RDD_PROCESSING_TX_DESCRIPTOR(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register PROCESSING_TX_DESCRIPTOR\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p), 7, 1, (r));
	bdmf_session_print(session, "\trequeue                  = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p), 6, 1, (r));
	bdmf_session_print(session, "\ttruncated                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p), 5, 1, (r));
	bdmf_session_print(session, "\tecn_capable              = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p), 4, 1, (r));
	bdmf_session_print(session, "\tdo_not_agg               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p), 3, 1, (r));
	bdmf_session_print(session, "\tmulticast_first_copy     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p), 2, 1, (r));
	bdmf_session_print(session, "\tmulticast_packet_copy    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p), 1, 1, (r));
	bdmf_session_print(session, "\tforce_copy               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_16((uint8_t *)(p), 7, 2, (r));
	bdmf_session_print(session, "\tpriority                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 1, 6, 1, (r));
	bdmf_session_print(session, "\tcolor                    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_32((uint8_t *)(p) + 0, 7, 15, (r));
	bdmf_session_print(session, "\tfirst_level_q            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 3, 6, 1, (r));
	bdmf_session_print(session, "\tmirror_packet            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 3, 0, 6, (r));
	bdmf_session_print(session, "\ting_port_id              = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_16((uint8_t *)(p) + 4, 5, 11, (r));
	bdmf_session_print(session, "\tserial_num               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_32((uint8_t *)(p) + 4, 14, 7, (r));
	bdmf_session_print(session, "\theader_number_number_of_sbpm_bns= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_16((uint8_t *)(p) + 6, 0, 14, (r));
	bdmf_session_print(session, "\tsecond_token             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 8, 7, 1, (r));
	bdmf_session_print(session, "\tingress_multicast_3rd    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 8, 6, 1, (r));
	bdmf_session_print(session, "\tdont_inc_sn              = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 8, (r));
	bdmf_session_print(session, "\trx_tx_sn_400g_reorder    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_16((uint8_t *)(p) + 10, 0, 14, (r));
	bdmf_session_print(session, "\tpacket_length            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 12, 7, 1, (r));
	bdmf_session_print(session, "\terr_exception            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 12, 6, 1, (r));
	bdmf_session_print(session, "\ting_mcast_copy           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 12, 2, 4, (r));
	bdmf_session_print(session, "\tdisp_viq                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_16((uint8_t *)(p) + 12, 6, 4, (r));
	bdmf_session_print(session, "\ting_ch                   = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_32((uint8_t *)(p) + 12, 15, 7, (r));
	bdmf_session_print(session, "\tsop                      = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 14, 6, 1, (r));
	bdmf_session_print(session, "\theadroom_select          = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_16((uint8_t *)(p) + 14, 0, 14, (r));
	bdmf_session_print(session, "\tbn0_first                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void set_RDD_PROCESSING_TX_DESCRIPTOR(bdmf_session_handle session, unsigned char *p, const bdmfmon_parm_value_t parm[], int write_index)
{
	int i = 0;
	int j;
	j = i;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p), 7, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p), 6, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p), 5, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p), 4, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p), 3, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p), 2, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p), 1, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_16((uint8_t *)(p), 7, 2, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 1, 6, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_32((uint8_t *)(p) + 0, 7, 15, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 3, 6, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 3, 0, 6, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_16((uint8_t *)(p) + 4, 5, 11, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_32((uint8_t *)(p) + 4, 14, 7, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_16((uint8_t *)(p) + 6, 0, 14, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 8, 7, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 8, 6, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 8, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_16((uint8_t *)(p) + 10, 0, 14, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 12, 7, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 12, 6, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 12, 2, 4, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_16((uint8_t *)(p) + 12, 6, 4, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_32((uint8_t *)(p) + 12, 15, 7, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 14, 6, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_16((uint8_t *)(p) + 14, 0, 14, (parm[j].number));
	j++;
}

void dump_RDD_SPDSVC_GEN_PARAMS(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register SPDSVC_GEN_PARAMS\n");

	MREAD_32((uint8_t *)(p), (r));
	bdmf_session_print(session, "\tprocessing_tx_pd_0       = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 4, (r));
	bdmf_session_print(session, "\tprocessing_tx_pd_1       = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 8, (r));
	bdmf_session_print(session, "\treserved0                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 12, (r));
	bdmf_session_print(session, "\treserved1                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 16, (r));
	bdmf_session_print(session, "\treserved2                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 20, (r));
	bdmf_session_print(session, "\treserved3                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 24, (r));
	bdmf_session_print(session, "\tprocessing_tx_pd_2       = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 28, (r));
	bdmf_session_print(session, "\tprocessing_tx_pd_3       = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 32, (r));
	bdmf_session_print(session, "\ttotal_copies             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 36, (r));
	bdmf_session_print(session, "\tbucket                   = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 40, (r));
	bdmf_session_print(session, "\ttimer_period             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 42, (r));
	bdmf_session_print(session, "\ttotal_length             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 44, (r));
	bdmf_session_print(session, "\ttx_packets               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 48, (r));
	bdmf_session_print(session, "\ttx_dropped               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 52, (r));
	bdmf_session_print(session, "\ttokens                   = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 56, (r));
	bdmf_session_print(session, "\tbucket_size              = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void set_RDD_SPDSVC_GEN_PARAMS(bdmf_session_handle session, unsigned char *p, const bdmfmon_parm_value_t parm[], int write_index)
{
	int i = 0;
	int j;
	j = i;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p), (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 4, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 8, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 12, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 16, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 20, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 24, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 28, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 32, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 36, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 40, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 42, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 44, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 48, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 52, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 56, (parm[j].number));
	j++;
}

void dump_RDD_UDPSPDT_SCRATCH_IPERF3_RX(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register UDPSPDT_SCRATCH_IPERF3_RX\n");

	MREAD_32((uint8_t *)(p), (r));
	bdmf_session_print(session, "\trx_packets_0             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 4, (r));
	bdmf_session_print(session, "\trx_packets_1             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void set_RDD_UDPSPDT_SCRATCH_IPERF3_RX(bdmf_session_handle session, unsigned char *p, const bdmfmon_parm_value_t parm[], int write_index)
{
	int i = 0;
	int j;
	j = i;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p), (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 4, (parm[j].number));
	j++;
}

void dump_RDD_UPDATE_FIFO_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register UPDATE_FIFO_ENTRY\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p), 7, 1, (r));
	bdmf_session_print(session, "\tvalid                    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p), 6, 1, (r));
	bdmf_session_print(session, "\treserved1                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_16((uint8_t *)(p), 0, 14, (r));
	bdmf_session_print(session, "\tpd_fifo_write_ptr        = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 2, 1, 7, (r));
	bdmf_session_print(session, "\treserved0                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_16((uint8_t *)(p) + 2, 0, 9, (r));
	bdmf_session_print(session, "\tqueue_number             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void set_RDD_UPDATE_FIFO_ENTRY(bdmf_session_handle session, unsigned char *p, const bdmfmon_parm_value_t parm[], int write_index)
{
	int i = 0;
	int j;
	j = i;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p), 7, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p), 6, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_16((uint8_t *)(p), 0, 14, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 2, 1, 7, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_16((uint8_t *)(p) + 2, 0, 9, (parm[j].number));
	j++;
}

void dump_RDD_TCPSPDTEST_ENGINE_GLOBAL_INFO(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register TCPSPDTEST_ENGINE_GLOBAL_INFO\n");

	MREAD_8((uint8_t *)(p), (r));
	bdmf_session_print(session, "\tnum_streams              = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 1, (r));
	bdmf_session_print(session, "\twakeup_stream_id         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 2, (r));
	bdmf_session_print(session, "\tdn_pkt_drop_fw_wakeup_vector= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 3, (r));
	bdmf_session_print(session, "\tnum_bns                  = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 4, (r));
	bdmf_session_print(session, "\tstream_vector            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 6, (r));
	bdmf_session_print(session, "\tup_tx_stream_cnt         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 7, (r));
	bdmf_session_print(session, "\tup_next_stream_id        = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 8, (r));
	bdmf_session_print(session, "\tup_bucket_tokens         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 12, (r));
	bdmf_session_print(session, "\tup_bucket_full_tokens    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 16, (r));
	bdmf_session_print(session, "\tup_bucket_tokens_fill_rate= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 18, (r));
	bdmf_session_print(session, "\treserved1                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 19, 1, 7, (r));
	bdmf_session_print(session, "\treserved2                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 19, 0, 1, (r));
	bdmf_session_print(session, "\tup_is_timer_active       = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void set_RDD_TCPSPDTEST_ENGINE_GLOBAL_INFO(bdmf_session_handle session, unsigned char *p, const bdmfmon_parm_value_t parm[], int write_index)
{
	int i = 0;
	int j;
	j = i;
	if ((write_index == -1) || (write_index == j))
		MWRITE_8((uint8_t *)(p), (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_8((uint8_t *)(p) + 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_8((uint8_t *)(p) + 2, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_8((uint8_t *)(p) + 3, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 4, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_8((uint8_t *)(p) + 6, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_8((uint8_t *)(p) + 7, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 8, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 12, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 16, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_8((uint8_t *)(p) + 18, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 19, 1, 7, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 19, 0, 1, (parm[j].number));
	j++;
}

void dump_RDD_UDPSPDT_STREAM_RX_STAT(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register UDPSPDT_STREAM_RX_STAT\n");

	MREAD_32((uint8_t *)(p), (r));
	bdmf_session_print(session, "\trx_bytes_1               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 4, (r));
	bdmf_session_print(session, "\trx_bytes_0               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 8, (r));
	bdmf_session_print(session, "\trx_packets_1             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 12, (r));
	bdmf_session_print(session, "\trx_packets_0             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 16, (r));
	bdmf_session_print(session, "\tts_first                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 20, (r));
	bdmf_session_print(session, "\tts_last_1                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 24, (r));
	bdmf_session_print(session, "\tts_last_0                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 28, 7, 1, (r));
	bdmf_session_print(session, "\tts_first_set             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_16((uint8_t *)(p) + 28, 0, 15, (r));
	bdmf_session_print(session, "\treserved                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 30, (r));
	bdmf_session_print(session, "\tbad_proto_cntr           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 32, (r));
	bdmf_session_print(session, "\tiperf3_rx_packet_lost_1  = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 36, (r));
	bdmf_session_print(session, "\tiperf3_rx_packet_lost_0  = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 40, (r));
	bdmf_session_print(session, "\tiperf3_rx_out_of_order_1 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 44, (r));
	bdmf_session_print(session, "\tiperf3_rx_out_of_order_0 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void set_RDD_UDPSPDT_STREAM_RX_STAT(bdmf_session_handle session, unsigned char *p, const bdmfmon_parm_value_t parm[], int write_index)
{
	int i = 0;
	int j;
	j = i;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p), (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 4, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 8, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 12, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 16, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 20, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 24, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 28, 7, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_16((uint8_t *)(p) + 28, 0, 15, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 30, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 32, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 36, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 40, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 44, (parm[j].number));
	j++;
}

void dump_RDD_UDPSPDT_STREAM_TX_STAT(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register UDPSPDT_STREAM_TX_STAT\n");

	MREAD_32((uint8_t *)(p), (r));
	bdmf_session_print(session, "\ttx_packets_1             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 4, (r));
	bdmf_session_print(session, "\ttx_packets_0             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 8, (r));
	bdmf_session_print(session, "\ttx_no_sbpm               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 12, (r));
	bdmf_session_print(session, "\tfirst_ts                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 16, (r));
	bdmf_session_print(session, "\tlast_ts_1                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 20, (r));
	bdmf_session_print(session, "\tlast_ts_0                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 24, (r));
	bdmf_session_print(session, "\tiperf3_ts_sec            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 28, (r));
	bdmf_session_print(session, "\tiperf3_ts_usec           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 32, (r));
	bdmf_session_print(session, "\tbad_proto_cntr           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 34, (r));
	bdmf_session_print(session, "\ttime_slice_expire        = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 36, (r));
	bdmf_session_print(session, "\ttx_drops_no_sbpm_timer_stop= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_16((uint8_t *)(p) + 38, 1, 15, (r));
	bdmf_session_print(session, "\treserved                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 39, 0, 1, (r));
	bdmf_session_print(session, "\tfirst_ts_set             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void set_RDD_UDPSPDT_STREAM_TX_STAT(bdmf_session_handle session, unsigned char *p, const bdmfmon_parm_value_t parm[], int write_index)
{
	int i = 0;
	int j;
	j = i;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p), (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 4, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 8, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 12, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 16, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 20, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 24, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 28, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 32, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 34, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 36, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_16((uint8_t *)(p) + 38, 1, 15, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 39, 0, 1, (parm[j].number));
	j++;
}

void dump_RDD_UDPSPDT_TX_PARAMS(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register UDPSPDT_TX_PARAMS\n");

	MREAD_32((uint8_t *)(p), (r));
	bdmf_session_print(session, "\ttotal_num_of_pkts        = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 4, (r));
	bdmf_session_print(session, "\tpacket_size              = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 6, (r));
	bdmf_session_print(session, "\tlast_packet_size         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 8, (r));
	bdmf_session_print(session, "\tcurr_bucket              = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 12, (r));
	bdmf_session_print(session, "\tbucket_budget            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 16, (r));
	bdmf_session_print(session, "\tlast_packet_start_idx    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 20, (r));
	bdmf_session_print(session, "\tmax_bucket_size          = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void set_RDD_UDPSPDT_TX_PARAMS(bdmf_session_handle session, unsigned char *p, const bdmfmon_parm_value_t parm[], int write_index)
{
	int i = 0;
	int j;
	j = i;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p), (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 4, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 6, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 8, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 12, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 16, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 20, (parm[j].number));
	j++;
}

void dump_RDD_GENERAL_TIMER_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register GENERAL_TIMER_ENTRY\n");

	MREAD_16((uint8_t *)(p), (r));
	bdmf_session_print(session, "\tcounter                  = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 2, (r));
	bdmf_session_print(session, "\ttimeout                  = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 4, (r));
	bdmf_session_print(session, "\tfunc_ptr                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 6, (r));
	bdmf_session_print(session, "\tenable                   = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 7, (r));
	bdmf_session_print(session, "\treserved                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void set_RDD_GENERAL_TIMER_ENTRY(bdmf_session_handle session, unsigned char *p, const bdmfmon_parm_value_t parm[], int write_index)
{
	int i = 0;
	int j;
	j = i;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p), (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 2, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 4, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_8((uint8_t *)(p) + 6, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_8((uint8_t *)(p) + 7, (parm[j].number));
	j++;
}

void dump_RDD_DDR_ADDRESS(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register DDR_ADDRESS\n");

	MREAD_32((uint8_t *)(p), (r));
	bdmf_session_print(session, "\tlow                      = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 4, (r));
	bdmf_session_print(session, "\thigh                     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void set_RDD_DDR_ADDRESS(bdmf_session_handle session, unsigned char *p, const bdmfmon_parm_value_t parm[], int write_index)
{
	int i = 0;
	int j;
	j = i;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p), (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 4, (parm[j].number));
	j++;
}

void dump_RDD_CPU_INTERRUPT_COALESCING_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register CPU_INTERRUPT_COALESCING_ENTRY\n");

	MREAD_32((uint8_t *)(p), (r));
	bdmf_session_print(session, "\ttimer_period             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 4, (r));
	bdmf_session_print(session, "\tnot_empty_or_int_vec     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 8, (r));
	bdmf_session_print(session, "\tscratch                  = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 12, (r));
	bdmf_session_print(session, "\trecycle_timer_cnt        = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 14, (r));
	bdmf_session_print(session, "\trecycle_timer_factor     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void set_RDD_CPU_INTERRUPT_COALESCING_ENTRY(bdmf_session_handle session, unsigned char *p, const bdmfmon_parm_value_t parm[], int write_index)
{
	int i = 0;
	int j;
	j = i;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p), (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 4, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 8, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 12, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 14, (parm[j].number));
	j++;
}

void dump_RDD_SG_DESC_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	int i,j;

	bdmf_session_print(session, "  Register SG_DESC_ENTRY\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p), 7, 1, (r));
	bdmf_session_print(session, "\tis_allocated             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p), 6, 1, (r));
	bdmf_session_print(session, "\tis_csum_offload          = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p), 5, 1, (r));
	bdmf_session_print(session, "\tis_udp                   = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p), 0, 5, (r));
	bdmf_session_print(session, "\treserv0                  = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 1, (r));
	bdmf_session_print(session, "\tnr_frags                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 2, (r));
	bdmf_session_print(session, "\ttotal_len                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 4, (r));
	bdmf_session_print(session, "\tpseudo_hdr_csum          = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 6, (r));
	bdmf_session_print(session, "\tcsum_sop                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 7, (r));
	bdmf_session_print(session, "\tcsum_offset              = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	bdmf_session_print(session, "\tfrag_data                =\n\t");
	for (i=0,j=0; i<8; i++)
	{
		MREAD_I_32((uint8_t *)(p) + 8, i, (r));
		bdmf_session_print(session, "0x%08x ", (unsigned int)r);
		j++;
		if (j >= 8)
		{
			j = 0;
			bdmf_session_print(session, "\n\t");
		}
	}
	bdmf_session_print(session, "\n");
	bdmf_session_print(session, "\tfrag_len                 =\n\t");
	for (i=0,j=0; i<4; i++)
	{
		MREAD_I_16((uint8_t *)(p) + 40, i, (r));
		bdmf_session_print(session, "0x%08x ", (unsigned int)r);
		j++;
	}
	bdmf_session_print(session, "\n");
}

void set_RDD_SG_DESC_ENTRY(bdmf_session_handle session, unsigned char *p, const bdmfmon_parm_value_t parm[], int write_index)
{
	int i = 0;
	int j;
	j = i;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p), 7, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p), 6, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p), 5, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p), 0, 5, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_8((uint8_t *)(p) + 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 2, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 4, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_8((uint8_t *)(p) + 6, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_8((uint8_t *)(p) + 7, (parm[j].number));
	j++;
	for (i=0; i<8; i++)
	{
		if ((write_index == -1) || (write_index == j))
			MWRITE_I_32((uint8_t *)(p) + 8, i, (parm[j].number));
		j++;
	}
	
	for (i=0; i<4; i++)
	{
		if ((write_index == -1) || (write_index == j))
			MWRITE_I_16((uint8_t *)(p) + 40, i, (parm[j].number));
		j++;
	}
	
}

void dump_RDD_CPU_TX_RING_INDICES(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register CPU_TX_RING_INDICES\n");

	MREAD_16((uint8_t *)(p), (r));
	bdmf_session_print(session, "\tread_idx                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 2, (r));
	bdmf_session_print(session, "\twrite_idx                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void set_RDD_CPU_TX_RING_INDICES(bdmf_session_handle session, unsigned char *p, const bdmfmon_parm_value_t parm[], int write_index)
{
	int i = 0;
	int j;
	j = i;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p), (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 2, (parm[j].number));
	j++;
}

void dump_RDD_CPU_RING_INTERRUPT_COUNTER_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register CPU_RING_INTERRUPT_COUNTER_ENTRY\n");

	MREAD_32((uint8_t *)(p), (r));
	bdmf_session_print(session, "\tcounter                  = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 4, (r));
	bdmf_session_print(session, "\tmax_size                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void set_RDD_CPU_RING_INTERRUPT_COUNTER_ENTRY(bdmf_session_handle session, unsigned char *p, const bdmfmon_parm_value_t parm[], int write_index)
{
	int i = 0;
	int j;
	j = i;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p), (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 4, (parm[j].number));
	j++;
}

void dump_RDD_RING_CPU_TX_DESCRIPTOR(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register RING_CPU_TX_DESCRIPTOR\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p), 7, 1, (r));
	bdmf_session_print(session, "\tis_egress                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_16((uint8_t *)(p), 6, 9, (r));
	bdmf_session_print(session, "\tegress_or_ingress_1      = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_32((uint8_t *)(p) + 0, 8, 14, (r));
	bdmf_session_print(session, "\tpacket_length            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 3, (r));
	bdmf_session_print(session, "\tsk_buf_ptr_high          = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 4, (r));
	bdmf_session_print(session, "\tsk_buf_ptr_low_or_data_1588= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 8, 7, 1, (r));
	bdmf_session_print(session, "\tcolor                    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 8, 6, 1, (r));
	bdmf_session_print(session, "\tdo_not_recycle           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 8, 5, 1, (r));
	bdmf_session_print(session, "\tflag_1588                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 8, 4, 1, (r));
	bdmf_session_print(session, "\tis_emac                  = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_16((uint8_t *)(p) + 8, 4, 8, (r));
	bdmf_session_print(session, "\twan_flow_source_port_union= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 9, 3, 1, (r));
	bdmf_session_print(session, "\tfpm_fallback             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 9, 2, 1, (r));
	bdmf_session_print(session, "\tsbpm_copy                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 9, 1, 1, (r));
	bdmf_session_print(session, "\ttgtmem_or_l3pkt          = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 9, 0, 1, (r));
	bdmf_session_print(session, "\tabs                      = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 10, (r));
	bdmf_session_print(session, "\tegress_or_ingress_2      = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 11, (r));
	bdmf_session_print(session, "\tpkt_buf_ptr_high         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 12, (r));
	bdmf_session_print(session, "\tpkt_buf_ptr_low_or_fpm_bn0= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void set_RDD_RING_CPU_TX_DESCRIPTOR(bdmf_session_handle session, unsigned char *p, const bdmfmon_parm_value_t parm[], int write_index)
{
	int i = 0;
	int j;
	j = i;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p), 7, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_16((uint8_t *)(p), 6, 9, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_32((uint8_t *)(p) + 0, 8, 14, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_8((uint8_t *)(p) + 3, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 4, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 8, 7, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 8, 6, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 8, 5, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 8, 4, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_16((uint8_t *)(p) + 8, 4, 8, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 9, 3, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 9, 2, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 9, 1, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 9, 0, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_8((uint8_t *)(p) + 10, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_8((uint8_t *)(p) + 11, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 12, (parm[j].number));
	j++;
}

void dump_RDD_CPU_RING_DESCRIPTOR(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register CPU_RING_DESCRIPTOR\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p), 3, 5, (r));
	bdmf_session_print(session, "\tsize_of_entry            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_16((uint8_t *)(p), 0, 11, (r));
	bdmf_session_print(session, "\tnumber_of_entries        = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 2, (r));
	bdmf_session_print(session, "\tinterrupt_id             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 4, (r));
	bdmf_session_print(session, "\tdrop_counter             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 6, (r));
	bdmf_session_print(session, "\twrite_idx                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 8, (r));
	bdmf_session_print(session, "\tbase_addr_low            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 12, (r));
	bdmf_session_print(session, "\tread_idx                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 14, (r));
	bdmf_session_print(session, "\treserved0                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 15, (r));
	bdmf_session_print(session, "\tbase_addr_high           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void set_RDD_CPU_RING_DESCRIPTOR(bdmf_session_handle session, unsigned char *p, const bdmfmon_parm_value_t parm[], int write_index)
{
	int i = 0;
	int j;
	j = i;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p), 3, 5, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_16((uint8_t *)(p), 0, 11, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 2, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 4, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 6, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 8, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 12, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_8((uint8_t *)(p) + 14, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_8((uint8_t *)(p) + 15, (parm[j].number));
	j++;
}

void dump_RDD_SG_CONTEXT_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register SG_CONTEXT_ENTRY\n");

	MREAD_32((uint8_t *)(p), (r));
	bdmf_session_print(session, "\treserved_scratchpad      = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 4, (r));
	bdmf_session_print(session, "\treserved1_scratchpad     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 6, (r));
	bdmf_session_print(session, "\tcsum_value               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 8, (r));
	bdmf_session_print(session, "\treserv2                  = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 12, (r));
	bdmf_session_print(session, "\trx_frag_packets          = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 16, (r));
	bdmf_session_print(session, "\trx_frag_octets           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 20, (r));
	bdmf_session_print(session, "\ttx_frag_packets          = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 24, (r));
	bdmf_session_print(session, "\ttx_frag_octets           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 28, (r));
	bdmf_session_print(session, "\tdropped_packets          = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 32, (r));
	bdmf_session_print(session, "\tdropped_no_sbpm          = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 34, (r));
	bdmf_session_print(session, "\tdropped_no_fpm           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 36, (r));
	bdmf_session_print(session, "\tdropped_sram_copy_failed = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 38, (r));
	bdmf_session_print(session, "\tdropped_invalid_sg_desc  = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 40, (r));
	bdmf_session_print(session, "\tdropped_invalid_frag_len = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 42, (r));
	bdmf_session_print(session, "\tdropped_csum_failed      = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 44, (r));
	bdmf_session_print(session, "\tpkt_csum                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 46, (r));
	bdmf_session_print(session, "\treserved_4               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 48, (r));
	bdmf_session_print(session, "\ttotal_len                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 50, (r));
	bdmf_session_print(session, "\tcsum_sop                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 51, (r));
	bdmf_session_print(session, "\tcsum_offset              = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 52, (r));
	bdmf_session_print(session, "\tpseudo_hdr_csum          = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 54, (r));
	bdmf_session_print(session, "\treserve_3                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 56, (r));
	bdmf_session_print(session, "\toffload_req_id_mismatch  = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 58, (r));
	bdmf_session_print(session, "\twakeup_reenable          = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 60, (r));
	bdmf_session_print(session, "\tnr_frags                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 61, 7, 1, (r));
	bdmf_session_print(session, "\tgso_reserve              = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 61, 0, 7, (r));
	bdmf_session_print(session, "\toffload_req_id           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 62, (r));
	bdmf_session_print(session, "\tgso_reserve_32bit_align_mark= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void set_RDD_SG_CONTEXT_ENTRY(bdmf_session_handle session, unsigned char *p, const bdmfmon_parm_value_t parm[], int write_index)
{
	int i = 0;
	int j;
	j = i;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p), (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 4, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 6, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 8, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 12, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 16, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 20, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 24, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 28, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 32, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 34, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 36, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 38, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 40, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 42, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 44, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 46, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 48, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_8((uint8_t *)(p) + 50, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_8((uint8_t *)(p) + 51, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 52, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 54, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 56, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 58, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_8((uint8_t *)(p) + 60, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 61, 7, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 61, 0, 7, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 62, (parm[j].number));
	j++;
}

void dump_RDD_CPU_TX_SYNC_FIFO_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register CPU_TX_SYNC_FIFO_ENTRY\n");

	MREAD_16((uint8_t *)(p), (r));
	bdmf_session_print(session, "\twrite_ptr                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 2, (r));
	bdmf_session_print(session, "\tread_ptr                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 4, (r));
	bdmf_session_print(session, "\tfifo                     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 6, (r));
	bdmf_session_print(session, "\treserved                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void set_RDD_CPU_TX_SYNC_FIFO_ENTRY(bdmf_session_handle session, unsigned char *p, const bdmfmon_parm_value_t parm[], int write_index)
{
	int i = 0;
	int j;
	j = i;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p), (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 2, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 4, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 6, (parm[j].number));
	j++;
}

void dump_RDD_DHD_COMPLETE_COMMON_RADIO_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	int i,j;

	bdmf_session_print(session, "  Register DHD_COMPLETE_COMMON_RADIO_ENTRY\n");

	MREAD_32((uint8_t *)(p), (r));
	bdmf_session_print(session, "\trx_complete_fr_base_ptr_low= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 4, (r));
	bdmf_session_print(session, "\trx_complete_fr_base_ptr_high= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 8, (r));
	bdmf_session_print(session, "\trx_complete_fr_rd_idx_ptr_low= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 12, (r));
	bdmf_session_print(session, "\trx_complete_fr_rd_idx_ptr_high= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 16, (r));
	bdmf_session_print(session, "\trx_complete_fr_wr_idx_ptr_low= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 20, (r));
	bdmf_session_print(session, "\trx_complete_fr_wr_idx_ptr_high= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 24, (r));
	bdmf_session_print(session, "\ttx_complete_fr_base_ptr_low= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 28, (r));
	bdmf_session_print(session, "\ttx_complete_fr_base_ptr_high= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 32, (r));
	bdmf_session_print(session, "\ttx_complete_fr_rd_idx_ptr_low= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 36, (r));
	bdmf_session_print(session, "\ttx_complete_fr_rd_idx_ptr_high= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 40, (r));
	bdmf_session_print(session, "\ttx_complete_fr_wr_idx_ptr_low= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 44, (r));
	bdmf_session_print(session, "\ttx_complete_fr_wr_idx_ptr_high= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 48, (r));
	bdmf_session_print(session, "\trx_post_fr_base_ptr_low  = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 52, (r));
	bdmf_session_print(session, "\trx_post_fr_base_ptr_high = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 56, (r));
	bdmf_session_print(session, "\trx_post_fr_rd_idx_ptr_low= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 60, (r));
	bdmf_session_print(session, "\trx_post_fr_rd_idx_ptr_high= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 64, (r));
	bdmf_session_print(session, "\trx_post_fr_wr_idx_ptr_low= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 68, (r));
	bdmf_session_print(session, "\trx_post_fr_wr_idx_ptr_high= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 72, (r));
	bdmf_session_print(session, "\tdhd_doorbell_low         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 76, (r));
	bdmf_session_print(session, "\tdhd_doorbell_high        = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 80, (r));
	bdmf_session_print(session, "\trx_complete_rd_idx       = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 82, (r));
	bdmf_session_print(session, "\trx_complete_wr_idx       = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 84, (r));
	bdmf_session_print(session, "\trx_complete_wr_idx_rsrv  = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 86, (r));
	bdmf_session_print(session, "\ttx_complete_rd_idx       = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 88, (r));
	bdmf_session_print(session, "\ttx_complete_wr_idx       = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 90, (r));
	bdmf_session_print(session, "\ttx_complete_wr_idx_rsrv  = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 92, (r));
	bdmf_session_print(session, "\trx_post_wr_idx           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 94, (r));
	bdmf_session_print(session, "\trx_post_wr_idx_rsrv      = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 96, 5, 3, (r));
	bdmf_session_print(session, "\tflow_ring_format         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 96, 4, 1, (r));
	bdmf_session_print(session, "\tidma_active              = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 96, 3, 1, (r));
	bdmf_session_print(session, "\trx_post_refill_enable    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 96, 2, 1, (r));
	bdmf_session_print(session, "\ttx_cmpl_ext_wi           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 96, 0, 2, (r));
	bdmf_session_print(session, "\treserved                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 97, (r));
	bdmf_session_print(session, "\trch_max_seg              = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 98, (r));
	bdmf_session_print(session, "\trx_post_rd_idx           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 100, (r));
	bdmf_session_print(session, "\ttx_pcap_seq_num          = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 102, (r));
	bdmf_session_print(session, "\trx_pcap_seq_num          = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	bdmf_session_print(session, "\ttx_scratchpad            =\n\t");
	for (i=0,j=0; i<4; i++)
	{
		MREAD_I_32((uint8_t *)(p) + 104, i, (r));
		bdmf_session_print(session, "0x%08x ", (unsigned int)r);
		j++;
	}
	bdmf_session_print(session, "\n");
	bdmf_session_print(session, "\trx_scratchpad            =\n\t");
	for (i=0,j=0; i<32; i++)
	{
		MREAD_I_32((uint8_t *)(p) + 120, i, (r));
		bdmf_session_print(session, "0x%08x ", (unsigned int)r);
		j++;
		if (j >= 8)
		{
			j = 0;
			bdmf_session_print(session, "\n\t");
		}
	}
	bdmf_session_print(session, "\n");
	bdmf_session_print(session, "\trch_buff                 =\n\t");
	for (i=0,j=0; i<18; i++)
	{
		MREAD_I_32((uint8_t *)(p) + 248, i, (r));
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

void set_RDD_DHD_COMPLETE_COMMON_RADIO_ENTRY(bdmf_session_handle session, unsigned char *p, const bdmfmon_parm_value_t parm[], int write_index)
{
	int i = 0;
	int j;
	j = i;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p), (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 4, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 8, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 12, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 16, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 20, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 24, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 28, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 32, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 36, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 40, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 44, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 48, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 52, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 56, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 60, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 64, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 68, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 72, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 76, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 80, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 82, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 84, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 86, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 88, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 90, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 92, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 94, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 96, 5, 3, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 96, 4, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 96, 3, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 96, 2, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 96, 0, 2, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_8((uint8_t *)(p) + 97, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 98, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 100, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 102, (parm[j].number));
	j++;
	for (i=0; i<4; i++)
	{
		if ((write_index == -1) || (write_index == j))
			MWRITE_I_32((uint8_t *)(p) + 104, i, (parm[j].number));
		j++;
	}
	
	for (i=0; i<32; i++)
	{
		if ((write_index == -1) || (write_index == j))
			MWRITE_I_32((uint8_t *)(p) + 120, i, (parm[j].number));
		j++;
	}
	
	for (i=0; i<18; i++)
	{
		if ((write_index == -1) || (write_index == j))
			MWRITE_I_32((uint8_t *)(p) + 248, i, (parm[j].number));
		j++;
	}
	
}

void dump_RDD_DHD_RX_COMPLETE_DESCRIPTOR(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register DHD_RX_COMPLETE_DESCRIPTOR\n");

	MREAD_8((uint8_t *)(p), (r));
	bdmf_session_print(session, "\tmsg_type                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 1, (r));
	bdmf_session_print(session, "\tif_id                    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 2, (r));
	bdmf_session_print(session, "\tcommon_hdr_flags         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 3, (r));
	bdmf_session_print(session, "\tepoch                    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 4, (r));
	bdmf_session_print(session, "\trequest_id               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 8, (r));
	bdmf_session_print(session, "\tcompl_msg_hdr_status     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 10, (r));
	bdmf_session_print(session, "\tflow_ring_id             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 12, (r));
	bdmf_session_print(session, "\tmeta_buf_len             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 14, (r));
	bdmf_session_print(session, "\tdata_len                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 16, (r));
	bdmf_session_print(session, "\tdata_offset              = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 18, (r));
	bdmf_session_print(session, "\tflags                    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 20, (r));
	bdmf_session_print(session, "\trx_status_0              = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 24, (r));
	bdmf_session_print(session, "\trx_status_1              = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 28, (r));
	bdmf_session_print(session, "\tdma_done_mark            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void set_RDD_DHD_RX_COMPLETE_DESCRIPTOR(bdmf_session_handle session, unsigned char *p, const bdmfmon_parm_value_t parm[], int write_index)
{
	int i = 0;
	int j;
	j = i;
	if ((write_index == -1) || (write_index == j))
		MWRITE_8((uint8_t *)(p), (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_8((uint8_t *)(p) + 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_8((uint8_t *)(p) + 2, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_8((uint8_t *)(p) + 3, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 4, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 8, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 10, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 12, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 14, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 16, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 18, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 20, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 24, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 28, (parm[j].number));
	j++;
}

void dump_RDD_DHD_DOORBELL(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register DHD_DOORBELL\n");

	MREAD_32((uint8_t *)(p), (r));
	bdmf_session_print(session, "\tdoorbell_value           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void set_RDD_DHD_DOORBELL(bdmf_session_handle session, unsigned char *p, const bdmfmon_parm_value_t parm[], int write_index)
{
	int i = 0;
	int j;
	j = i;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p), (parm[j].number));
	j++;
}

void dump_RDD_DHD_RX_POST_DESCRIPTOR(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register DHD_RX_POST_DESCRIPTOR\n");

	MREAD_8((uint8_t *)(p), (r));
	bdmf_session_print(session, "\tmsg_type                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 1, (r));
	bdmf_session_print(session, "\tif_id                    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 2, (r));
	bdmf_session_print(session, "\tcommon_hdr_flags         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 3, (r));
	bdmf_session_print(session, "\tepoch                    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 4, (r));
	bdmf_session_print(session, "\trequest_id               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 8, (r));
	bdmf_session_print(session, "\tmeta_buf_len             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 10, (r));
	bdmf_session_print(session, "\tdata_len                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 12, (r));
	bdmf_session_print(session, "\treserved                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 16, (r));
	bdmf_session_print(session, "\tmetadata_buf_addr_low    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 20, (r));
	bdmf_session_print(session, "\tmetadata_buf_addr_hi     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 24, (r));
	bdmf_session_print(session, "\tdata_buf_addr_low        = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 28, (r));
	bdmf_session_print(session, "\tdata_buf_addr_hi         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void set_RDD_DHD_RX_POST_DESCRIPTOR(bdmf_session_handle session, unsigned char *p, const bdmfmon_parm_value_t parm[], int write_index)
{
	int i = 0;
	int j;
	j = i;
	if ((write_index == -1) || (write_index == j))
		MWRITE_8((uint8_t *)(p), (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_8((uint8_t *)(p) + 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_8((uint8_t *)(p) + 2, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_8((uint8_t *)(p) + 3, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 4, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 8, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 10, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 12, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 16, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 20, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 24, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 28, (parm[j].number));
	j++;
}

void dump_RDD_DHD_RING_SIZE(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register DHD_RING_SIZE\n");

	MREAD_16((uint8_t *)(p), (r));
	bdmf_session_print(session, "\tvalue                    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void set_RDD_DHD_RING_SIZE(bdmf_session_handle session, unsigned char *p, const bdmfmon_parm_value_t parm[], int write_index)
{
	int i = 0;
	int j;
	j = i;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p), (parm[j].number));
	j++;
}

void dump_RDD_DHD_PCAP_CONFIG(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register DHD_PCAP_CONFIG\n");

	MREAD_8((uint8_t *)(p), (r));
	bdmf_session_print(session, "\tpcap_rxcopy_en           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 1, (r));
	bdmf_session_print(session, "\tpcap_txcopy_en           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 2, (r));
	bdmf_session_print(session, "\tpcap_maxlen              = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void set_RDD_DHD_PCAP_CONFIG(bdmf_session_handle session, unsigned char *p, const bdmfmon_parm_value_t parm[], int write_index)
{
	int i = 0;
	int j;
	j = i;
	if ((write_index == -1) || (write_index == j))
		MWRITE_8((uint8_t *)(p), (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_8((uint8_t *)(p) + 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 2, (parm[j].number));
	j++;
}

void dump_RDD_DHD_MONITOR_PKT_INFO(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	int i,j;

	bdmf_session_print(session, "  Register DHD_MONITOR_PKT_INFO\n");

	MREAD_16((uint8_t *)(p), (r));
	bdmf_session_print(session, "\tmax_pkt_delay            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 2, (r));
	bdmf_session_print(session, "\tmin_pkt_delay            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 4, (r));
	bdmf_session_print(session, "\taver_pkt_delay           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 8, (r));
	bdmf_session_print(session, "\taccum_pkt_delay          = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 12, (r));
	bdmf_session_print(session, "\taccum_pkt_samples        = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 16, (r));
	bdmf_session_print(session, "\tts_now                   = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	bdmf_session_print(session, "\tpkt_delay_hist           =\n\t");
	for (i=0,j=0; i<8; i++)
	{
		MREAD_I_32((uint8_t *)(p) + 20, i, (r));
		bdmf_session_print(session, "0x%08x ", (unsigned int)r);
		j++;
		if (j >= 8)
		{
			j = 0;
			bdmf_session_print(session, "\n\t");
		}
	}
	bdmf_session_print(session, "\n");
	MREAD_8((uint8_t *)(p) + 52, (r));
	bdmf_session_print(session, "\thist_bin_shift           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	bdmf_session_print(session, "\treserved1                =\n\t");
	for (i=0,j=0; i<3; i++)
	{
		MREAD_I_8((uint8_t *)(p) + 53, i, (r));
		bdmf_session_print(session, "0x%08x ", (unsigned int)r);
		j++;
	}
	bdmf_session_print(session, "\n");
}

void set_RDD_DHD_MONITOR_PKT_INFO(bdmf_session_handle session, unsigned char *p, const bdmfmon_parm_value_t parm[], int write_index)
{
	int i = 0;
	int j;
	j = i;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p), (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 2, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 4, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 8, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 12, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 16, (parm[j].number));
	j++;
	for (i=0; i<8; i++)
	{
		if ((write_index == -1) || (write_index == j))
			MWRITE_I_32((uint8_t *)(p) + 20, i, (parm[j].number));
		j++;
	}
	
	if ((write_index == -1) || (write_index == j))
		MWRITE_8((uint8_t *)(p) + 52, (parm[j].number));
	j++;
	for (i=0; i<3; i++)
	{
		if ((write_index == -1) || (write_index == j))
			MWRITE_I_8((uint8_t *)(p) + 53, i, (parm[j].number));
		j++;
	}
	
}

void dump_RDD_DHD_TX_COMPLETE_DESCRIPTOR(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register DHD_TX_COMPLETE_DESCRIPTOR\n");

	MREAD_8((uint8_t *)(p), (r));
	bdmf_session_print(session, "\tmsg_type                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 1, (r));
	bdmf_session_print(session, "\tif_id                    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 2, (r));
	bdmf_session_print(session, "\tcommon_hdr_flags         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 3, (r));
	bdmf_session_print(session, "\tepoch                    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 4, (r));
	bdmf_session_print(session, "\trequest_id               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 8, (r));
	bdmf_session_print(session, "\tinfo                     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 10, (r));
	bdmf_session_print(session, "\tflow_ring_id             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 12, (r));
	bdmf_session_print(session, "\tdma_done_mark            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void set_RDD_DHD_TX_COMPLETE_DESCRIPTOR(bdmf_session_handle session, unsigned char *p, const bdmfmon_parm_value_t parm[], int write_index)
{
	int i = 0;
	int j;
	j = i;
	if ((write_index == -1) || (write_index == j))
		MWRITE_8((uint8_t *)(p), (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_8((uint8_t *)(p) + 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_8((uint8_t *)(p) + 2, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_8((uint8_t *)(p) + 3, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 4, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 8, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 10, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 12, (parm[j].number));
	j++;
}

void dump_RDD_TM_FLOW_CNTR_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register TM_FLOW_CNTR_ENTRY\n");

	MREAD_8((uint8_t *)(p), (r));
	bdmf_session_print(session, "\tcntr_id                  = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void set_RDD_TM_FLOW_CNTR_ENTRY(bdmf_session_handle session, unsigned char *p, const bdmfmon_parm_value_t parm[], int write_index)
{
	int i = 0;
	int j;
	j = i;
	if ((write_index == -1) || (write_index == j))
		MWRITE_8((uint8_t *)(p), (parm[j].number));
	j++;
}

void dump_RDD_GSU_UPDATE_RL_STATUS_BB_MSG(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register GSU_UPDATE_RL_STATUS_BB_MSG\n");

	MREAD_32((uint8_t *)(p), (r));
	bdmf_session_print(session, "\treserved0                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_32((uint8_t *)(p) + 4, 14, 18, (r));
	bdmf_session_print(session, "\treserved1                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_16((uint8_t *)(p) + 6, 0, 14, (r));
	bdmf_session_print(session, "\tnext_node_pointer        = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void set_RDD_GSU_UPDATE_RL_STATUS_BB_MSG(bdmf_session_handle session, unsigned char *p, const bdmfmon_parm_value_t parm[], int write_index)
{
	int i = 0;
	int j;
	j = i;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p), (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_32((uint8_t *)(p) + 4, 14, 18, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_16((uint8_t *)(p) + 6, 0, 14, (parm[j].number));
	j++;
}

void dump_RDD_GSU_UPDATE_AQM_STATUS_BB_MSG(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register GSU_UPDATE_AQM_STATUS_BB_MSG\n");

	RDP_FIELD_MREAD_32((uint8_t *)(p), 11, 21, (r));
	bdmf_session_print(session, "\treserved0                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_16((uint8_t *)(p) + 2, 0, 11, (r));
	bdmf_session_print(session, "\trelative_q_num           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_16((uint8_t *)(p) + 4, 3, 13, (r));
	bdmf_session_print(session, "\treserved1                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 5, 2, 1, (r));
	bdmf_session_print(session, "\tvalid                    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 5, 1, 1, (r));
	bdmf_session_print(session, "\tmulticast                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 5, 0, 1, (r));
	bdmf_session_print(session, "\tq_empty                  = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 6, 2, 6, (r));
	bdmf_session_print(session, "\treserved2                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_16((uint8_t *)(p) + 6, 0, 10, (r));
	bdmf_session_print(session, "\ttimestamp                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void set_RDD_GSU_UPDATE_AQM_STATUS_BB_MSG(bdmf_session_handle session, unsigned char *p, const bdmfmon_parm_value_t parm[], int write_index)
{
	int i = 0;
	int j;
	j = i;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_32((uint8_t *)(p), 11, 21, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_16((uint8_t *)(p) + 2, 0, 11, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_16((uint8_t *)(p) + 4, 3, 13, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 5, 2, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 5, 1, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 5, 0, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 6, 2, 6, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_16((uint8_t *)(p) + 6, 0, 10, (parm[j].number));
	j++;
}

void dump_RDD_AQM_PROBABILITY_CALC_DESCRIPTOR(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register AQM_PROBABILITY_CALC_DESCRIPTOR\n");

	MREAD_16((uint8_t *)(p), (r));
	bdmf_session_print(session, "\ttimer_value              = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 2, (r));
	bdmf_session_print(session, "\talpha_coeff              = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 4, (r));
	bdmf_session_print(session, "\tbeta_coeff               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 6, (r));
	bdmf_session_print(session, "\ttarget_delay             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 7, (r));
	bdmf_session_print(session, "\tlaqm_coupling_factor     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 8, (r));
	bdmf_session_print(session, "\ttimer_value_minor        = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 10, (r));
	bdmf_session_print(session, "\treserved                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 12, (r));
	bdmf_session_print(session, "\treserved1                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void set_RDD_AQM_PROBABILITY_CALC_DESCRIPTOR(bdmf_session_handle session, unsigned char *p, const bdmfmon_parm_value_t parm[], int write_index)
{
	int i = 0;
	int j;
	j = i;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p), (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 2, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 4, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_8((uint8_t *)(p) + 6, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_8((uint8_t *)(p) + 7, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 8, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 10, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 12, (parm[j].number));
	j++;
}

void dump_RDD_RL_UPDATE_TASK_DESCRIPTOR(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	int i,j;

	bdmf_session_print(session, "  Register RL_UPDATE_TASK_DESCRIPTOR\n");

	MREAD_16((uint8_t *)(p), (r));
	bdmf_session_print(session, "\ttimer_value              = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 2, (r));
	bdmf_session_print(session, "\ttimer_value_minor        = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	bdmf_session_print(session, "\tnum_updates_per_req      =\n\t");
	for (i=0,j=0; i<5; i++)
	{
		MREAD_I_32((uint8_t *)(p) + 4, i, (r));
		bdmf_session_print(session, "0x%08x ", (unsigned int)r);
		j++;
	}
	bdmf_session_print(session, "\n");
}

void set_RDD_RL_UPDATE_TASK_DESCRIPTOR(bdmf_session_handle session, unsigned char *p, const bdmfmon_parm_value_t parm[], int write_index)
{
	int i = 0;
	int j;
	j = i;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p), (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 2, (parm[j].number));
	j++;
	for (i=0; i<5; i++)
	{
		if ((write_index == -1) || (write_index == j))
			MWRITE_I_32((uint8_t *)(p) + 4, i, (parm[j].number));
		j++;
	}
	
}

void dump_RDD_BBH_TX_WAKE_UP_DATA_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register BBH_TX_WAKE_UP_DATA_ENTRY\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p), 3, 5, (r));
	bdmf_session_print(session, "\treserved0                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_32((uint8_t *)(p), 8, 19, (r));
	bdmf_session_print(session, "\tbytes_in_bbh             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 3, (r));
	bdmf_session_print(session, "\tqueue                    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 4, (r));
	bdmf_session_print(session, "\treserved1                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void set_RDD_BBH_TX_WAKE_UP_DATA_ENTRY(bdmf_session_handle session, unsigned char *p, const bdmfmon_parm_value_t parm[], int write_index)
{
	int i = 0;
	int j;
	j = i;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p), 3, 5, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_32((uint8_t *)(p), 8, 19, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_8((uint8_t *)(p) + 3, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 4, (parm[j].number));
	j++;
}

void dump_RDD_AQM_QUEUE_DESCRIPTOR(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register AQM_QUEUE_DESCRIPTOR\n");

	MREAD_16((uint8_t *)(p), (r));
	bdmf_session_print(session, "\tdropmark_probability     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_16((uint8_t *)(p) + 2, 6, 10, (r));
	bdmf_session_print(session, "\tprevious_q_delay         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 3, 3, 3, (r));
	bdmf_session_print(session, "\taqm_type                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 3, 2, 1, (r));
	bdmf_session_print(session, "\toverload                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 3, 1, 1, (r));
	bdmf_session_print(session, "\texcessive_delay          = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 3, 0, 1, (r));
	bdmf_session_print(session, "\treserved                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void set_RDD_AQM_QUEUE_DESCRIPTOR(bdmf_session_handle session, unsigned char *p, const bdmfmon_parm_value_t parm[], int write_index)
{
	int i = 0;
	int j;
	j = i;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p), (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_16((uint8_t *)(p) + 2, 6, 10, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 3, 3, 3, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 3, 2, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 3, 1, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 3, 0, 1, (parm[j].number));
	j++;
}

void dump_RDD_GSU_SETTING(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register GSU_SETTING\n");

	MREAD_16((uint8_t *)(p), (r));
	bdmf_session_print(session, "\tgsu_bb_id                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 2, (r));
	bdmf_session_print(session, "\taqm_q_desc_offset        = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 4, (r));
	bdmf_session_print(session, "\treserved2                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 6, (r));
	bdmf_session_print(session, "\tmax_used_relative_q_num  = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 8, (r));
	bdmf_session_print(session, "\tenable_timer_tasks       = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_32((uint8_t *)(p) + 8, 0, 24, (r));
	bdmf_session_print(session, "\treserved                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 12, 6, 2, (r));
	bdmf_session_print(session, "\treserved1                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_16((uint8_t *)(p) + 12, 4, 10, (r));
	bdmf_session_print(session, "\texcessive_delay_threshold_pi2= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_32((uint8_t *)(p) + 12, 10, 10, (r));
	bdmf_session_print(session, "\texcessive_delay_threshold_laqm= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_16((uint8_t *)(p) + 14, 0, 10, (r));
	bdmf_session_print(session, "\texcessive_delay_threshold_noaqm= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void set_RDD_GSU_SETTING(bdmf_session_handle session, unsigned char *p, const bdmfmon_parm_value_t parm[], int write_index)
{
	int i = 0;
	int j;
	j = i;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p), (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 2, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 4, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 6, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_8((uint8_t *)(p) + 8, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_32((uint8_t *)(p) + 8, 0, 24, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 12, 6, 2, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_16((uint8_t *)(p) + 12, 4, 10, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_32((uint8_t *)(p) + 12, 10, 10, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_16((uint8_t *)(p) + 14, 0, 10, (parm[j].number));
	j++;
}

void dump_RDD_QM_UPDATE_FIFO_BB_MSG(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register QM_UPDATE_FIFO_BB_MSG\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p), 6, 2, (r));
	bdmf_session_print(session, "\tpd_type                  = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p), 4, 2, (r));
	bdmf_session_print(session, "\treserved0                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_32((uint8_t *)(p), 13, 15, (r));
	bdmf_session_print(session, "\treal_queue_num           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 2, 4, 1, (r));
	bdmf_session_print(session, "\tmulticast                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 2, 3, 1, (r));
	bdmf_session_print(session, "\taqm_enable               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 2, 2, 1, (r));
	bdmf_session_print(session, "\treserved1                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_16((uint8_t *)(p) + 2, 0, 10, (r));
	bdmf_session_print(session, "\ttimestamp_delta          = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 4, 7, 1, (r));
	bdmf_session_print(session, "\tvld                      = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 4, 6, 1, (r));
	bdmf_session_print(session, "\tqm_us                    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 4, 5, 1, (r));
	bdmf_session_print(session, "\tecn_en                   = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_16((uint8_t *)(p) + 4, 6, 7, (r));
	bdmf_session_print(session, "\treserved2                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 5, 2, 4, (r));
	bdmf_session_print(session, "\tfw_prop_field            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_32((uint8_t *)(p) + 4, 3, 15, (r));
	bdmf_session_print(session, "\taqm_context_ptr          = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 7, 0, 3, (r));
	bdmf_session_print(session, "\treserved3                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 8, (r));
	bdmf_session_print(session, "\treserved4                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_32((uint8_t *)(p) + 12, 14, 18, (r));
	bdmf_session_print(session, "\treserved5                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_16((uint8_t *)(p) + 14, 0, 14, (r));
	bdmf_session_print(session, "\taggregated_plen          = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void set_RDD_QM_UPDATE_FIFO_BB_MSG(bdmf_session_handle session, unsigned char *p, const bdmfmon_parm_value_t parm[], int write_index)
{
	int i = 0;
	int j;
	j = i;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p), 6, 2, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p), 4, 2, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_32((uint8_t *)(p), 13, 15, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 2, 4, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 2, 3, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 2, 2, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_16((uint8_t *)(p) + 2, 0, 10, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 4, 7, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 4, 6, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 4, 5, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_16((uint8_t *)(p) + 4, 6, 7, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 5, 2, 4, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_32((uint8_t *)(p) + 4, 3, 15, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 7, 0, 3, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 8, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_32((uint8_t *)(p) + 12, 14, 18, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_16((uint8_t *)(p) + 14, 0, 14, (parm[j].number));
	j++;
}

void dump_RDD_BBH_QUEUE_TO_BBH_QUEUE_DESC_MAPPING_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	int i,j;

	bdmf_session_print(session, "  Register BBH_QUEUE_TO_BBH_QUEUE_DESC_MAPPING_ENTRY\n");

	bdmf_session_print(session, "\tindex                    =\n\t");
	for (i=0,j=0; i<40; i++)
	{
		MREAD_I_8((uint8_t *)(p), i, (r));
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

void set_RDD_BBH_QUEUE_TO_BBH_QUEUE_DESC_MAPPING_ENTRY(bdmf_session_handle session, unsigned char *p, const bdmfmon_parm_value_t parm[], int write_index)
{
	int i = 0;
	int j;
	j = i;
	for (i=0; i<40; i++)
	{
		if ((write_index == -1) || (write_index == j))
			MWRITE_I_8((uint8_t *)(p), i, (parm[j].number));
		j++;
	}
	
}

void dump_RDD_TRANSMIT_TASK_DESCRIPTOR(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register TRANSMIT_TASK_DESCRIPTOR\n");

	MREAD_16((uint8_t *)(p), (r));
	bdmf_session_print(session, "\tgsu_bb_id                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 2, (r));
	bdmf_session_print(session, "\tbbhtx_bb_id              = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 4, (r));
	bdmf_session_print(session, "\tchannel_id               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 6, (r));
	bdmf_session_print(session, "\tgsu_rel_id               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 8, (r));
	bdmf_session_print(session, "\tmirrnr_pdfifo_address    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 10, (r));
	bdmf_session_print(session, "\tmirrnr_wrptr_address     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 12, (r));
	bdmf_session_print(session, "\tmirrnr_bb_id             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 14, (r));
	bdmf_session_print(session, "\tmirror_mode              = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 15, (r));
	bdmf_session_print(session, "\tis_ack_msg_sent          = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void set_RDD_TRANSMIT_TASK_DESCRIPTOR(bdmf_session_handle session, unsigned char *p, const bdmfmon_parm_value_t parm[], int write_index)
{
	int i = 0;
	int j;
	j = i;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p), (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 2, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 4, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 6, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 8, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 10, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 12, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_8((uint8_t *)(p) + 14, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_8((uint8_t *)(p) + 15, (parm[j].number));
	j++;
}

void dump_RDD_BBHTX_WAKE_UP_BB_MSG(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register BBHTX_WAKE_UP_BB_MSG\n");

	RDP_FIELD_MREAD_32((uint8_t *)(p), 6, 26, (r));
	bdmf_session_print(session, "\treserved0                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 3, 0, 6, (r));
	bdmf_session_print(session, "\ttcont_number             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 4, 3, 5, (r));
	bdmf_session_print(session, "\treserved1                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 4, (r));
	bdmf_session_print(session, "\tnum_bytes                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_16((uint8_t *)(p) + 6, 3, 8, (r));
	bdmf_session_print(session, "\tnum_pd                   = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 7, 0, 3, (r));
	bdmf_session_print(session, "\tcmd                      = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void set_RDD_BBHTX_WAKE_UP_BB_MSG(bdmf_session_handle session, unsigned char *p, const bdmfmon_parm_value_t parm[], int write_index)
{
	int i = 0;
	int j;
	j = i;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_32((uint8_t *)(p), 6, 26, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 3, 0, 6, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 4, 3, 5, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 4, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_16((uint8_t *)(p) + 6, 3, 8, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 7, 0, 3, (parm[j].number));
	j++;
}

void dump_RDD_GSU_TX_STATUS_BB_MSG(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register GSU_TX_STATUS_BB_MSG\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p), 7, 1, (r));
	bdmf_session_print(session, "\tnack                     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p), 6, 1, (r));
	bdmf_session_print(session, "\tnack_and_single_tcont    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p), 5, 1, (r));
	bdmf_session_print(session, "\tack                      = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p), 2, 3, (r));
	bdmf_session_print(session, "\treserved0                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_16((uint8_t *)(p), 4, 6, (r));
	bdmf_session_print(session, "\ttcont_id                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 1, 1, 3, (r));
	bdmf_session_print(session, "\treserved1                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_32((uint8_t *)(p) + 0, 0, 17, (r));
	bdmf_session_print(session, "\tnum_bytes_transmitted    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 4, 3, 5, (r));
	bdmf_session_print(session, "\treserved2                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_16((uint8_t *)(p) + 4, 0, 11, (r));
	bdmf_session_print(session, "\tnum_pds_mult_8           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 6, 5, 3, (r));
	bdmf_session_print(session, "\tack_nack_status          = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 6, 0, 5, (r));
	bdmf_session_print(session, "\treserved3                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 7, (r));
	bdmf_session_print(session, "\tnum_pd                   = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void set_RDD_GSU_TX_STATUS_BB_MSG(bdmf_session_handle session, unsigned char *p, const bdmfmon_parm_value_t parm[], int write_index)
{
	int i = 0;
	int j;
	j = i;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p), 7, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p), 6, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p), 5, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p), 2, 3, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_16((uint8_t *)(p), 4, 6, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 1, 1, 3, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_32((uint8_t *)(p) + 0, 0, 17, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 4, 3, 5, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_16((uint8_t *)(p) + 4, 0, 11, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 6, 5, 3, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 6, 0, 5, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_8((uint8_t *)(p) + 7, (parm[j].number));
	j++;
}

void dump_RDD_UPDATE_FIFO_TASK_DESCRIPTOR(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register UPDATE_FIFO_TASK_DESCRIPTOR\n");

	MREAD_16((uint8_t *)(p), (r));
	bdmf_session_print(session, "\tgsu_bb_id                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 2, (r));
	bdmf_session_print(session, "\tgsu_rel_id               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 3, (r));
	bdmf_session_print(session, "\tqm_id                    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 4, (r));
	bdmf_session_print(session, "\tnum_entries              = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 5, (r));
	bdmf_session_print(session, "\tnum_entries_per_wakeup   = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 6, (r));
	bdmf_session_print(session, "\tis_task_started          = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 7, (r));
	bdmf_session_print(session, "\treserved1                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void set_RDD_UPDATE_FIFO_TASK_DESCRIPTOR(bdmf_session_handle session, unsigned char *p, const bdmfmon_parm_value_t parm[], int write_index)
{
	int i = 0;
	int j;
	j = i;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p), (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_8((uint8_t *)(p) + 2, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_8((uint8_t *)(p) + 3, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_8((uint8_t *)(p) + 4, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_8((uint8_t *)(p) + 5, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_8((uint8_t *)(p) + 6, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_8((uint8_t *)(p) + 7, (parm[j].number));
	j++;
}

void dump_RDD_PACKETS_AND_BYTES(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register PACKETS_AND_BYTES\n");

	MREAD_32((uint8_t *)(p), (r));
	bdmf_session_print(session, "\tpackets                  = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 4, (r));
	bdmf_session_print(session, "\tbytes                    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void set_RDD_PACKETS_AND_BYTES(bdmf_session_handle session, unsigned char *p, const bdmfmon_parm_value_t parm[], int write_index)
{
	int i = 0;
	int j;
	j = i;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p), (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 4, (parm[j].number));
	j++;
}

void dump_RDD_QEMU_DATA(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register QEMU_DATA\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p), 7, 1, (r));
	bdmf_session_print(session, "\tstart_runner_trace       = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p), 6, 1, (r));
	bdmf_session_print(session, "\tstop_runner_trace        = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p), 5, 1, (r));
	bdmf_session_print(session, "\tstart_capture_waves      = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p), 4, 1, (r));
	bdmf_session_print(session, "\tstop_capture_waves       = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p), 3, 1, (r));
	bdmf_session_print(session, "\tstart_send_1_packet_once = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p), 2, 1, (r));
	bdmf_session_print(session, "\tstart_send_infinise_packet= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p), 1, 1, (r));
	bdmf_session_print(session, "\tdata_path_ready          = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p), 0, 1, (r));
	bdmf_session_print(session, "\treserved                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void set_RDD_QEMU_DATA(bdmf_session_handle session, unsigned char *p, const bdmfmon_parm_value_t parm[], int write_index)
{
	int i = 0;
	int j;
	j = i;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p), 7, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p), 6, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p), 5, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p), 4, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p), 3, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p), 2, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p), 1, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p), 0, 1, (parm[j].number));
	j++;
}

void dump_RDD_PKTGEN_SBPM_HDR_BN(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register PKTGEN_SBPM_HDR_BN\n");

	RDP_FIELD_MREAD_16((uint8_t *)(p), 2, 14, (r));
	bdmf_session_print(session, "\tfirst_bn                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 1, 0, 2, (r));
	bdmf_session_print(session, "\text_idx                  = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void set_RDD_PKTGEN_SBPM_HDR_BN(bdmf_session_handle session, unsigned char *p, const bdmfmon_parm_value_t parm[], int write_index)
{
	int i = 0;
	int j;
	j = i;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_16((uint8_t *)(p), 2, 14, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 1, 0, 2, (parm[j].number));
	j++;
}

void dump_RDD_PKTGEN_SBPM_EXT(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register PKTGEN_SBPM_EXT\n");

	RDP_FIELD_MREAD_16((uint8_t *)(p), 2, 14, (r));
	bdmf_session_print(session, "\treserved0                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_32((uint8_t *)(p) + 0, 14, 4, (r));
	bdmf_session_print(session, "\tnum_of_bns               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_16((uint8_t *)(p) + 2, 0, 14, (r));
	bdmf_session_print(session, "\tbn1                      = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void set_RDD_PKTGEN_SBPM_EXT(bdmf_session_handle session, unsigned char *p, const bdmfmon_parm_value_t parm[], int write_index)
{
	int i = 0;
	int j;
	j = i;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_16((uint8_t *)(p), 2, 14, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_32((uint8_t *)(p) + 0, 14, 4, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_16((uint8_t *)(p) + 2, 0, 14, (parm[j].number));
	j++;
}

void dump_RDD_DHD_FLOW_RING_CACHE_CTX_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register DHD_FLOW_RING_CACHE_CTX_ENTRY\n");

	MREAD_32((uint8_t *)(p), (r));
	bdmf_session_print(session, "\tflow_ring_base_low       = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 4, (r));
	bdmf_session_print(session, "\tflow_ring_base_high      = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 5, (r));
	bdmf_session_print(session, "\treserved                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 6, (r));
	bdmf_session_print(session, "\tshadow_wr_idx            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 8, (r));
	bdmf_session_print(session, "\tsize                     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 10, (r));
	bdmf_session_print(session, "\tssid                     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 11, (r));
	bdmf_session_print(session, "\tflags                    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 12, (r));
	bdmf_session_print(session, "\trd_idx                   = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 14, (r));
	bdmf_session_print(session, "\twr_idx                   = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void set_RDD_DHD_FLOW_RING_CACHE_CTX_ENTRY(bdmf_session_handle session, unsigned char *p, const bdmfmon_parm_value_t parm[], int write_index)
{
	int i = 0;
	int j;
	j = i;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p), (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_8((uint8_t *)(p) + 4, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_8((uint8_t *)(p) + 5, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 6, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 8, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_8((uint8_t *)(p) + 10, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_8((uint8_t *)(p) + 11, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 12, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 14, (parm[j].number));
	j++;
}

void dump_RDD_DHD_AUX_INFO_CACHE_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register DHD_AUX_INFO_CACHE_ENTRY\n");

	MREAD_16((uint8_t *)(p), (r));
	bdmf_session_print(session, "\tbackup_first_index       = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 2, (r));
	bdmf_session_print(session, "\tbackup_last_index        = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 4, (r));
	bdmf_session_print(session, "\tbackup_num_entries       = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 6, (r));
	bdmf_session_print(session, "\tphy_size                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 8, (r));
	bdmf_session_print(session, "\twindow_ts                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 10, (r));
	bdmf_session_print(session, "\tdrop_interval            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 12, (r));
	bdmf_session_print(session, "\tcodel_control_word       = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 14, (r));
	bdmf_session_print(session, "\tcodel_drop_counter       = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void set_RDD_DHD_AUX_INFO_CACHE_ENTRY(bdmf_session_handle session, unsigned char *p, const bdmfmon_parm_value_t parm[], int write_index)
{
	int i = 0;
	int j;
	j = i;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p), (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 2, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 4, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 6, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 8, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 10, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 12, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 14, (parm[j].number));
	j++;
}

void dump_RDD_DHD_POST_COMMON_RADIO_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	int i,j;

	bdmf_session_print(session, "  Register DHD_POST_COMMON_RADIO_ENTRY\n");

	MREAD_32((uint8_t *)(p), (r));
	bdmf_session_print(session, "\ttx_post_mgmt_fr_base_ptr_low= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 4, (r));
	bdmf_session_print(session, "\ttx_post_mgmt_fr_base_ptr_high= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 8, (r));
	bdmf_session_print(session, "\ttx_post_fr_wr_idx_base_ptr_low= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 12, (r));
	bdmf_session_print(session, "\ttx_post_fr_wr_idx_base_ptr_high= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 16, (r));
	bdmf_session_print(session, "\ttx_post_fr_rd_idx_base_ptr_low= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 20, (r));
	bdmf_session_print(session, "\ttx_post_fr_rd_idx_base_ptr_high= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 24, (r));
	bdmf_session_print(session, "\tbackup_base_low          = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 28, (r));
	bdmf_session_print(session, "\tbackup_base_high         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 32, (r));
	bdmf_session_print(session, "\tbackup_index_fifo_base_low= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 36, (r));
	bdmf_session_print(session, "\tbackup_index_fifo_base_high= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 40, (r));
	bdmf_session_print(session, "\tdhd_doorbell_low         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 44, (r));
	bdmf_session_print(session, "\tdhd_doorbell_high        = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 48, (r));
	bdmf_session_print(session, "\ttx_post_wr_idx           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 50, (r));
	bdmf_session_print(session, "\ttx_post_qd               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 52, (r));
	bdmf_session_print(session, "\ttx_post_rd_idx           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 54, (r));
	bdmf_session_print(session, "\tidma_last_group_fr       = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 56, (r));
	bdmf_session_print(session, "\tcam_rslt                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 60, 7, 1, (r));
	bdmf_session_print(session, "\tadd_llcsnap_header       = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 60, 5, 2, (r));
	bdmf_session_print(session, "\treserved                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 60, 2, 3, (r));
	bdmf_session_print(session, "\tflow_ring_format         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 60, 1, 1, (r));
	bdmf_session_print(session, "\tidma_active              = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 60, 0, 1, (r));
	bdmf_session_print(session, "\tcodel_active             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 61, (r));
	bdmf_session_print(session, "\tidma_last_group_id       = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 62, (r));
	bdmf_session_print(session, "\tcache_ctx_next_write_idx = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 63, (r));
	bdmf_session_print(session, "\tbackup_cache_idx         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 64, (r));
	bdmf_session_print(session, "\tcodel_max_seq_drops      = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 66, (r));
	bdmf_session_print(session, "\tfr_ptrs_size_shift       = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 67, (r));
	bdmf_session_print(session, "\tidma_group_shift         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 68, (r));
	bdmf_session_print(session, "\tdoorbell_value_address   = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	bdmf_session_print(session, "\tscratchpad               =\n\t");
	for (i=0,j=0; i<7; i++)
	{
		MREAD_I_32((uint8_t *)(p) + 72, i, (r));
		bdmf_session_print(session, "0x%08x ", (unsigned int)r);
		j++;
	}
	bdmf_session_print(session, "\n");
	MREAD_16((uint8_t *)(p) + 100, (r));
	bdmf_session_print(session, "\tbackup_rd_idx            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 102, (r));
	bdmf_session_print(session, "\tbackup_wr_idx            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 104, 7, 1, (r));
	bdmf_session_print(session, "\tpacket_valid             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 104, 6, 1, (r));
	bdmf_session_print(session, "\tpacket_abs_addr          = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_16((uint8_t *)(p) + 104, 0, 14, (r));
	bdmf_session_print(session, "\tpacket_data_len          = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 106, 5, 3, (r));
	bdmf_session_print(session, "\tpacket_prio              = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 106, 4, 1, (r));
	bdmf_session_print(session, "\tbuffer_prio              = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 106, 3, 1, (r));
	bdmf_session_print(session, "\tbcmc_packet              = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 106, 0, 3, (r));
	bdmf_session_print(session, "\treserved6                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 107, (r));
	bdmf_session_print(session, "\trsv_address_high         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 108, (r));
	bdmf_session_print(session, "\tfpm_address_low          = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 112, (r));
	bdmf_session_print(session, "\trequest_id               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 116, (r));
	bdmf_session_print(session, "\tbcmc_seqnum              = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 118, (r));
	bdmf_session_print(session, "\tcpu_msg_done             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 119, (r));
	bdmf_session_print(session, "\tcoalescing_max_count     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	bdmf_session_print(session, "\tdoorbell_counters        =\n\t");
	for (i=0,j=0; i<64; i++)
	{
		MREAD_I_8((uint8_t *)(p) + 120, i, (r));
		bdmf_session_print(session, "0x%08x ", (unsigned int)r);
		j++;
		if (j >= 8)
		{
			j = 0;
			bdmf_session_print(session, "\n\t");
		}
	}
	bdmf_session_print(session, "\n");
	MREAD_8((uint8_t *)(p) + 184, (r));
	bdmf_session_print(session, "\treserved2                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 185, (r));
	bdmf_session_print(session, "\tcoalescing_timeout       = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 186, (r));
	bdmf_session_print(session, "\tcoalescing_timeout_cntr  = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 187, (r));
	bdmf_session_print(session, "\tcur_frg_id               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 188, 7, 1, (r));
	bdmf_session_print(session, "\ttx_mirroring_en          = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 188, 1, 6, (r));
	bdmf_session_print(session, "\tdst_vport                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_16((uint8_t *)(p) + 188, 0, 9, (r));
	bdmf_session_print(session, "\tdst_queue                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 190, (r));
	bdmf_session_print(session, "\ttx_mirror_truncate       = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 192, (r));
	bdmf_session_print(session, "\tbq_num_entries           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 194, (r));
	bdmf_session_print(session, "\tcur_fr_id                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 196, (r));
	bdmf_session_print(session, "\tcur_monitor_profile      = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	bdmf_session_print(session, "\treserved4                =\n\t");
	for (i=0,j=0; i<3; i++)
	{
		MREAD_I_8((uint8_t *)(p) + 197, i, (r));
		bdmf_session_print(session, "0x%08x ", (unsigned int)r);
		j++;
	}
	bdmf_session_print(session, "\n");
}

void set_RDD_DHD_POST_COMMON_RADIO_ENTRY(bdmf_session_handle session, unsigned char *p, const bdmfmon_parm_value_t parm[], int write_index)
{
	int i = 0;
	int j;
	j = i;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p), (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 4, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 8, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 12, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 16, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 20, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 24, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 28, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 32, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 36, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 40, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 44, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 48, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 50, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 52, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 54, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 56, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 60, 7, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 60, 5, 2, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 60, 2, 3, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 60, 1, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 60, 0, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_8((uint8_t *)(p) + 61, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_8((uint8_t *)(p) + 62, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_8((uint8_t *)(p) + 63, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 64, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_8((uint8_t *)(p) + 66, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_8((uint8_t *)(p) + 67, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 68, (parm[j].number));
	j++;
	for (i=0; i<7; i++)
	{
		if ((write_index == -1) || (write_index == j))
			MWRITE_I_32((uint8_t *)(p) + 72, i, (parm[j].number));
		j++;
	}
	
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 100, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 102, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 104, 7, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 104, 6, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_16((uint8_t *)(p) + 104, 0, 14, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 106, 5, 3, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 106, 4, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 106, 3, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 106, 0, 3, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_8((uint8_t *)(p) + 107, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 108, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 112, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 116, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_8((uint8_t *)(p) + 118, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_8((uint8_t *)(p) + 119, (parm[j].number));
	j++;
	for (i=0; i<64; i++)
	{
		if ((write_index == -1) || (write_index == j))
			MWRITE_I_8((uint8_t *)(p) + 120, i, (parm[j].number));
		j++;
	}
	
	if ((write_index == -1) || (write_index == j))
		MWRITE_8((uint8_t *)(p) + 184, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_8((uint8_t *)(p) + 185, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_8((uint8_t *)(p) + 186, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_8((uint8_t *)(p) + 187, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 188, 7, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 188, 1, 6, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_16((uint8_t *)(p) + 188, 0, 9, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 190, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 192, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 194, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_8((uint8_t *)(p) + 196, (parm[j].number));
	j++;
	for (i=0; i<3; i++)
	{
		if ((write_index == -1) || (write_index == j))
			MWRITE_I_8((uint8_t *)(p) + 197, i, (parm[j].number));
		j++;
	}
	
}

void dump_RDD_DHD_MONITOR_BQ_INFO(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register DHD_MONITOR_BQ_INFO\n");

	MREAD_16((uint8_t *)(p), (r));
	bdmf_session_print(session, "\tfr_id                    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 2, (r));
	bdmf_session_print(session, "\tradio_id                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 4, (r));
	bdmf_session_print(session, "\tmax_bq_amount            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 6, (r));
	bdmf_session_print(session, "\tmin_bq_amount            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 8, (r));
	bdmf_session_print(session, "\taver_bq_amount           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 10, (r));
	bdmf_session_print(session, "\tmax_bq_delay             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 12, (r));
	bdmf_session_print(session, "\tmin_bq_delay             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 14, (r));
	bdmf_session_print(session, "\taver_bq_delay            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 16, (r));
	bdmf_session_print(session, "\taccum_bq_amount          = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 20, (r));
	bdmf_session_print(session, "\taccum_bq_delay           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 24, (r));
	bdmf_session_print(session, "\taccum_samples            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 28, (r));
	bdmf_session_print(session, "\tts_now                   = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void set_RDD_DHD_MONITOR_BQ_INFO(bdmf_session_handle session, unsigned char *p, const bdmfmon_parm_value_t parm[], int write_index)
{
	int i = 0;
	int j;
	j = i;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p), (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 2, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 4, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 6, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 8, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 10, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 12, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 14, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 16, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 20, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 24, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 28, (parm[j].number));
	j++;
}

void dump_RDD_DHD_BACKUP_IDX_CACHE_TABLE(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	int i,j;

	bdmf_session_print(session, "  Register DHD_BACKUP_IDX_CACHE_TABLE\n");

	bdmf_session_print(session, "\tindexes                  =\n\t");
	for (i=0,j=0; i<16; i++)
	{
		MREAD_I_16((uint8_t *)(p), i, (r));
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

void set_RDD_DHD_BACKUP_IDX_CACHE_TABLE(bdmf_session_handle session, unsigned char *p, const bdmfmon_parm_value_t parm[], int write_index)
{
	int i = 0;
	int j;
	j = i;
	for (i=0; i<16; i++)
	{
		if ((write_index == -1) || (write_index == j))
			MWRITE_I_16((uint8_t *)(p), i, (parm[j].number));
		j++;
	}
	
}

void dump_RDD_PKTGEN_TX_STREAM_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register PKTGEN_TX_STREAM_ENTRY\n");

	MREAD_16((uint8_t *)(p), (r));
	bdmf_session_print(session, "\tREF_PKT_HDR_size         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 2, (r));
	bdmf_session_print(session, "\tREF_PKT_HDR_offset       = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 4, (r));
	bdmf_session_print(session, "\tREF_PKT_HDR_reserved     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 8, (r));
	bdmf_session_print(session, "\tREF_PKT_HDR_hdr          = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 12, (r));
	bdmf_session_print(session, "\tREF_PKT_HDR_hdr_1        = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 16, (r));
	bdmf_session_print(session, "\tREF_PKT_HDR_hdr_2        = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 20, (r));
	bdmf_session_print(session, "\tREF_PKT_HDR_hdr_3        = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 24, (r));
	bdmf_session_print(session, "\tREF_PKT_HDR_hdr_4        = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 28, (r));
	bdmf_session_print(session, "\tREF_PKT_HDR_hdr_5        = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 32, (r));
	bdmf_session_print(session, "\tREF_PKT_HDR_hdr_6        = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 36, (r));
	bdmf_session_print(session, "\tREF_PKT_HDR_hdr_7        = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 40, (r));
	bdmf_session_print(session, "\tREF_PKT_HDR_hdr_8        = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 44, (r));
	bdmf_session_print(session, "\tREF_PKT_HDR_hdr_9        = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 48, (r));
	bdmf_session_print(session, "\tREF_PKT_HDR_hdr_10       = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 52, (r));
	bdmf_session_print(session, "\tREF_PKT_HDR_hdr_11       = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 56, (r));
	bdmf_session_print(session, "\tREF_PKT_HDR_hdr_12       = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 60, (r));
	bdmf_session_print(session, "\tREF_PKT_HDR_hdr_13       = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 64, (r));
	bdmf_session_print(session, "\tREF_PKT_HDR_hdr_14       = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 68, (r));
	bdmf_session_print(session, "\tREF_PKT_HDR_hdr_15       = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 72, (r));
	bdmf_session_print(session, "\tREF_PKT_HDR_hdr_16       = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 76, (r));
	bdmf_session_print(session, "\tREF_PKT_HDR_hdr_17       = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 80, (r));
	bdmf_session_print(session, "\tREF_PKT_HDR_hdr_18       = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 84, (r));
	bdmf_session_print(session, "\tREF_PKT_HDR_hdr_19       = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 88, (r));
	bdmf_session_print(session, "\tREF_PKT_HDR_hdr_20       = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 92, (r));
	bdmf_session_print(session, "\tREF_PKT_HDR_hdr_21       = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 96, (r));
	bdmf_session_print(session, "\tREF_PKT_HDR_hdr_22       = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 100, (r));
	bdmf_session_print(session, "\tREF_PKT_HDR_hdr_23       = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 104, (r));
	bdmf_session_print(session, "\tREF_PKT_HDR_hdr_24       = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 108, (r));
	bdmf_session_print(session, "\tREF_PKT_HDR_hdr_25       = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 112, (r));
	bdmf_session_print(session, "\tREF_PKT_HDR_hdr_26       = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 116, (r));
	bdmf_session_print(session, "\tREF_PKT_HDR_hdr_27       = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 120, (r));
	bdmf_session_print(session, "\tREF_PKT_HDR_hdr_28       = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 124, (r));
	bdmf_session_print(session, "\tREF_PKT_HDR_hdr_29       = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 128, (r));
	bdmf_session_print(session, "\tENTRY_PARMS_wan_flow     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 130, (r));
	bdmf_session_print(session, "\tENTRY_PARMS_reserved     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 132, (r));
	bdmf_session_print(session, "\tENTRY_PARMS_tcp_udp_parms= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void set_RDD_PKTGEN_TX_STREAM_ENTRY(bdmf_session_handle session, unsigned char *p, const bdmfmon_parm_value_t parm[], int write_index)
{
	int i = 0;
	int j;
	j = i;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p), (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 2, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 4, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 8, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 12, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 16, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 20, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 24, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 28, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 32, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 36, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 40, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 44, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 48, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 52, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 56, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 60, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 64, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 68, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 72, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 76, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 80, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 84, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 88, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 92, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 96, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 100, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 104, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 108, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 112, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 116, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 120, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 124, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 128, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 130, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 132, (parm[j].number));
	j++;
}

void dump_RDD_CPU_RX_METER_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register CPU_RX_METER_ENTRY\n");

	MREAD_16((uint8_t *)(p), (r));
	bdmf_session_print(session, "\tcurrent_budget           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 2, (r));
	bdmf_session_print(session, "\tbudget_limit             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 4, (r));
	bdmf_session_print(session, "\tallocated_budget         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 6, (r));
	bdmf_session_print(session, "\tbudget_residue           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 7, (r));
	bdmf_session_print(session, "\tcurrent_cycle            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void set_RDD_CPU_RX_METER_ENTRY(bdmf_session_handle session, unsigned char *p, const bdmfmon_parm_value_t parm[], int write_index)
{
	int i = 0;
	int j;
	j = i;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p), (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 2, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 4, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_8((uint8_t *)(p) + 6, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_8((uint8_t *)(p) + 7, (parm[j].number));
	j++;
}

void dump_RDD_DHD_TX_POST_DESCRIPTOR(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register DHD_TX_POST_DESCRIPTOR\n");

	MREAD_8((uint8_t *)(p), (r));
	bdmf_session_print(session, "\tmsg_type                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 1, (r));
	bdmf_session_print(session, "\tif_id                    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 2, (r));
	bdmf_session_print(session, "\tcommon_hdr_flags         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 3, (r));
	bdmf_session_print(session, "\tepoch                    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 4, (r));
	bdmf_session_print(session, "\trequest_id               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 8, (r));
	bdmf_session_print(session, "\ttx_eth_hdr_0             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 12, (r));
	bdmf_session_print(session, "\ttx_eth_hdr_1             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 16, (r));
	bdmf_session_print(session, "\ttx_eth_hdr_2             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 20, (r));
	bdmf_session_print(session, "\ttx_eth_hdr_3             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 22, (r));
	bdmf_session_print(session, "\tflags                    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 23, (r));
	bdmf_session_print(session, "\tseg_cnt                  = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 24, (r));
	bdmf_session_print(session, "\tmetadata_buf_addr_low    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 28, (r));
	bdmf_session_print(session, "\tmetadata_buf_addr_hi     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 32, (r));
	bdmf_session_print(session, "\tdata_buf_addr_low        = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 36, (r));
	bdmf_session_print(session, "\tdata_buf_addr_hi         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 40, (r));
	bdmf_session_print(session, "\tmeta_buf_len             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 42, (r));
	bdmf_session_print(session, "\tdata_len                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 44, (r));
	bdmf_session_print(session, "\treserved                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void set_RDD_DHD_TX_POST_DESCRIPTOR(bdmf_session_handle session, unsigned char *p, const bdmfmon_parm_value_t parm[], int write_index)
{
	int i = 0;
	int j;
	j = i;
	if ((write_index == -1) || (write_index == j))
		MWRITE_8((uint8_t *)(p), (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_8((uint8_t *)(p) + 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_8((uint8_t *)(p) + 2, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_8((uint8_t *)(p) + 3, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 4, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 8, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 12, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 16, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 20, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_8((uint8_t *)(p) + 22, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_8((uint8_t *)(p) + 23, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 24, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 28, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 32, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 36, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 40, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 42, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 44, (parm[j].number));
	j++;
}

void dump_RDD_PKTGEN_TX_PARAMS(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register PKTGEN_TX_PARAMS\n");

	MREAD_32((uint8_t *)(p), (r));
	bdmf_session_print(session, "\tbbmsg_sbpm_mcast_inc_req_0= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 4, (r));
	bdmf_session_print(session, "\tbbmsg_sbpm_mcast_inc_req_1= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 8, (r));
	bdmf_session_print(session, "\tref_pd_0                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 12, (r));
	bdmf_session_print(session, "\tref_pd_1                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 16, (r));
	bdmf_session_print(session, "\tref_pd_2                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 20, (r));
	bdmf_session_print(session, "\tref_pd_3                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 24, 4, 4, (r));
	bdmf_session_print(session, "\ttask_num                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_32((uint8_t *)(p) + 24, 0, 28, (r));
	bdmf_session_print(session, "\treserved1                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 28, (r));
	bdmf_session_print(session, "\treserved2                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 30, (r));
	bdmf_session_print(session, "\tbad_tx_num_of_bns        = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void set_RDD_PKTGEN_TX_PARAMS(bdmf_session_handle session, unsigned char *p, const bdmfmon_parm_value_t parm[], int write_index)
{
	int i = 0;
	int j;
	j = i;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p), (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 4, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 8, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 12, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 16, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 20, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 24, 4, 4, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_32((uint8_t *)(p) + 24, 0, 28, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 28, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 30, (parm[j].number));
	j++;
}

void dump_RDD_CSO_CONTEXT_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	int i,j;

	bdmf_session_print(session, "  Register CSO_CONTEXT_ENTRY\n");

	MREAD_32((uint8_t *)(p), (r));
	bdmf_session_print(session, "\treserved                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 4, (r));
	bdmf_session_print(session, "\treserved1                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 6, (r));
	bdmf_session_print(session, "\tcsum_value               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 8, (r));
	bdmf_session_print(session, "\tcam_result               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 10, (r));
	bdmf_session_print(session, "\treserved2                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 12, (r));
	bdmf_session_print(session, "\tcrc_result               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 16, (r));
	bdmf_session_print(session, "\tsaved_pkt_desc_0         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 20, (r));
	bdmf_session_print(session, "\tsaved_pkt_desc_1         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 24, (r));
	bdmf_session_print(session, "\tsaved_pkt_desc_2         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 28, (r));
	bdmf_session_print(session, "\tsaved_pkt_desc_3         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	bdmf_session_print(session, "\tparser_result            =\n\t");
	for (i=0,j=0; i<24; i++)
	{
		MREAD_I_32((uint8_t *)(p) + 32, i, (r));
		bdmf_session_print(session, "0x%08x ", (unsigned int)r);
		j++;
		if (j >= 8)
		{
			j = 0;
			bdmf_session_print(session, "\n\t");
		}
	}
	bdmf_session_print(session, "\n");
	MREAD_32((uint8_t *)(p) + 128, (r));
	bdmf_session_print(session, "\tsaved_ret_address        = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 132, (r));
	bdmf_session_print(session, "\tgood_csum_packets        = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 136, (r));
	bdmf_session_print(session, "\tno_cso_support_packets   = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 140, (r));
	bdmf_session_print(session, "\tforward_cso_packets      = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 144, (r));
	bdmf_session_print(session, "\tforward_cso_ver_error    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 148, (r));
	bdmf_session_print(session, "\tbad_ipv4_hdr_csum_packets= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 152, (r));
	bdmf_session_print(session, "\tbad_tcp_udp_csum_packets = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 156, (r));
	bdmf_session_print(session, "\tinvalid_offload_result   = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 160, (r));
	bdmf_session_print(session, "\toffload_req_id_mismatch  = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 164, (r));
	bdmf_session_print(session, "\twakeup_reenable          = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 168, (r));
	bdmf_session_print(session, "\tsaved_buffer_number      = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 170, (r));
	bdmf_session_print(session, "\tsaved_packet_length      = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 172, 1, 7, (r));
	bdmf_session_print(session, "\toffload_req_id           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_32((uint8_t *)(p) + 172, 0, 25, (r));
	bdmf_session_print(session, "\treserved3                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void set_RDD_CSO_CONTEXT_ENTRY(bdmf_session_handle session, unsigned char *p, const bdmfmon_parm_value_t parm[], int write_index)
{
	int i = 0;
	int j;
	j = i;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p), (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 4, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 6, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 8, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 10, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 12, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 16, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 20, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 24, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 28, (parm[j].number));
	j++;
	for (i=0; i<24; i++)
	{
		if ((write_index == -1) || (write_index == j))
			MWRITE_I_32((uint8_t *)(p) + 32, i, (parm[j].number));
		j++;
	}
	
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 128, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 132, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 136, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 140, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 144, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 148, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 152, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 156, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 160, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 164, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 168, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 170, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 172, 1, 7, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_32((uint8_t *)(p) + 172, 0, 25, (parm[j].number));
	j++;
}

void dump_RDD_DHD_FLOW_RING_CACHE_LKP_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register DHD_FLOW_RING_CACHE_LKP_ENTRY\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p), 7, 1, (r));
	bdmf_session_print(session, "\tinvalid                  = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p), 2, 5, (r));
	bdmf_session_print(session, "\treserved                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_16((uint8_t *)(p), 0, 10, (r));
	bdmf_session_print(session, "\tflow_ring_id             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void set_RDD_DHD_FLOW_RING_CACHE_LKP_ENTRY(bdmf_session_handle session, unsigned char *p, const bdmfmon_parm_value_t parm[], int write_index)
{
	int i = 0;
	int j;
	j = i;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p), 7, 1, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p), 2, 5, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_16((uint8_t *)(p), 0, 10, (parm[j].number));
	j++;
}

void dump_RDD_PKTGEN_FPM_UG_MGMT_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register PKTGEN_FPM_UG_MGMT_ENTRY\n");

	MREAD_32((uint8_t *)(p), (r));
	bdmf_session_print(session, "\tfpm_ug_cnt_dummy         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 4, (r));
	bdmf_session_print(session, "\tfpm_ug_cnt               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 8, (r));
	bdmf_session_print(session, "\tfpm_ug_cnt_reg_addr      = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_32((uint8_t *)(p) + 12, 4, 28, (r));
	bdmf_session_print(session, "\tfpm_ug_threshold         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 15, 0, 4, (r));
	bdmf_session_print(session, "\tfpm_tokens_quantum       = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 16, (r));
	bdmf_session_print(session, "\tbudget                   = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void set_RDD_PKTGEN_FPM_UG_MGMT_ENTRY(bdmf_session_handle session, unsigned char *p, const bdmfmon_parm_value_t parm[], int write_index)
{
	int i = 0;
	int j;
	j = i;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p), (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 4, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 8, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_32((uint8_t *)(p) + 12, 4, 28, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		RDP_FIELD_MWRITE_8((uint8_t *)(p) + 15, 0, 4, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p) + 16, (parm[j].number));
	j++;
}

void dump_RDD_SPDSVC_WLAN_TXPOST_PARAMS(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register SPDSVC_WLAN_TXPOST_PARAMS\n");

	MREAD_16((uint8_t *)(p), (r));
	bdmf_session_print(session, "\tspdsvc_free_idx          = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void set_RDD_SPDSVC_WLAN_TXPOST_PARAMS(bdmf_session_handle session, unsigned char *p, const bdmfmon_parm_value_t parm[], int write_index)
{
	int i = 0;
	int j;
	j = i;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p), (parm[j].number));
	j++;
}

void dump_RDD_PKTGEN_TX_STREAM_SCRATCH_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register PKTGEN_TX_STREAM_SCRATCH_ENTRY\n");

	MREAD_32((uint8_t *)(p), (r));
	bdmf_session_print(session, "\treserved0                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 4, (r));
	bdmf_session_print(session, "\tcsum_value               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 6, (r));
	bdmf_session_print(session, "\treserved1                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void set_RDD_PKTGEN_TX_STREAM_SCRATCH_ENTRY(bdmf_session_handle session, unsigned char *p, const bdmfmon_parm_value_t parm[], int write_index)
{
	int i = 0;
	int j;
	j = i;
	if ((write_index == -1) || (write_index == j))
		MWRITE_32((uint8_t *)(p), (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 4, (parm[j].number));
	j++;
	if ((write_index == -1) || (write_index == j))
		MWRITE_16((uint8_t *)(p) + 6, (parm[j].number));
	j++;
}

