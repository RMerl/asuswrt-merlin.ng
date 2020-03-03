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
#ifdef RDP_SIM
#include "rdd_platform.h"
#endif
#include "access_macros.h"
#include "rdd_runner_reg_dump.h"

void dump_RDD_IPTV_DDR_CONTEXT_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	int i,j;

	bdmf_session_print(session, "  Register IPTV_DDR_CONTEXT_ENTRY\n");

	FIELD_MREAD_8((uint8_t *)p, 7, 1, r);
	bdmf_session_print(session, "\tvalid                    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p, 6, 1, r);
	bdmf_session_print(session, "\tip_ver                   = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p, 5, 1, r);
	bdmf_session_print(session, "\tany_vid                  = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p, 4, 1, r);
	bdmf_session_print(session, "\tany_src_ip               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p, 0, 12, r);
	bdmf_session_print(session, "\tvid                      = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 2, r);
	bdmf_session_print(session, "\tnext_entry_idx           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 4, r);
	bdmf_session_print(session, "\tssid_vector_0_or_flooding_vport= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 6, r);
	bdmf_session_print(session, "\tssid_vector_1            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 8, r);
	bdmf_session_print(session, "\tssid_vector_2            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 10, 5, 3, r);
	bdmf_session_print(session, "\tradio_dhd_vector         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 10, 0, 5, r);
	bdmf_session_print(session, "\trx_if                    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 11, 7, 1, r);
	bdmf_session_print(session, "\twlan_mcast_proxy_enabled = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 11, 0, 7, r);
	bdmf_session_print(session, "\twlan_mcast_index         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 12, 7, 1, r);
	bdmf_session_print(session, "\treserved2                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 12, 4, 3, r);
	bdmf_session_print(session, "\twlan_mcast_tx_prio       = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 12, 0, 4, r);
	bdmf_session_print(session, "\treserved0                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 13, 6, 2, r);
	bdmf_session_print(session, "\tpool_num                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 13, 0, 6, r);
	bdmf_session_print(session, "\treplications             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 14, 2, 6, r);
	bdmf_session_print(session, "\treserved1                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p + 14, 0, 10, r);
	bdmf_session_print(session, "\tcntr_id                  = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 16, r);
	bdmf_session_print(session, "\tegress_ports_vector      = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 20, r);
	bdmf_session_print(session, "\tsrc_ip                   = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	bdmf_session_print(session, "\tsrc_ipv6_addr            =\n\t");
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
	bdmf_session_print(session, "\tdst_ipv6_addr            =\n\t");
	for (i=0,j=0; i<16; i++)
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
	bdmf_session_print(session, "\tresult                   =\n\t");
	for (i=0,j=0; i<32; i++)
	{
		MREAD_I_8((uint8_t *)p + 56, i, r);
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

void dump_RDD_PROCESSING_TX_DESCRIPTOR(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register PROCESSING_TX_DESCRIPTOR\n");

	FIELD_MREAD_8((uint8_t *)p, 7, 1, r);
	bdmf_session_print(session, "\tvalid                    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p, 6, 1, r);
	bdmf_session_print(session, "\theadroom                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p, 5, 1, r);
	bdmf_session_print(session, "\tdont_agg                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p, 4, 1, r);
	bdmf_session_print(session, "\tmc_copy                  = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p, 3, 1, r);
	bdmf_session_print(session, "\treprocess                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p, 2, 1, r);
	bdmf_session_print(session, "\tcolor                    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p, 1, 1, r);
	bdmf_session_print(session, "\tforce_copy               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p, 0, 9, r);
	bdmf_session_print(session, "\tsecond_level_q           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p + 2, 7, 9, r);
	bdmf_session_print(session, "\tfirst_level_q            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 3, 6, 1, r);
	bdmf_session_print(session, "\tflag_1588                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 3, 5, 1, r);
	bdmf_session_print(session, "\tcoherent                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 3, 0, 5, r);
	bdmf_session_print(session, "\thn                       = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p + 4, 6, 10, r);
	bdmf_session_print(session, "\tserial_num               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 5, 5, 1, r);
	bdmf_session_print(session, "\tpriority                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_32((uint8_t *)p + 4, 14, 7, r);
	bdmf_session_print(session, "\tbn_num                   = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p + 6, 0, 14, r);
	bdmf_session_print(session, "\tpacket_length            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 8, 7, 1, r);
	bdmf_session_print(session, "\tdrop                     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 8, 6, 1, r);
	bdmf_session_print(session, "\ttarget_mem_1             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 8, 3, 3, r);
	bdmf_session_print(session, "\tmcst_bcst_union          = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 8, 2, 1, r);
	bdmf_session_print(session, "\tlan                      = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p + 8, 2, 8, r);
	bdmf_session_print(session, "\tingress_port             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_32((uint8_t *)p + 8, 0, 18, r);
	bdmf_session_print(session, "\tunion3                   = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 12, 7, 1, r);
	bdmf_session_print(session, "\tagg_pd                   = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 12, 6, 1, r);
	bdmf_session_print(session, "\ttarget_mem_0             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 12, 5, 1, r);
	bdmf_session_print(session, "\tabs                      = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p + 12, 2, 11, r);
	bdmf_session_print(session, "\tpayload_offset_or_abs_1  = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_32((uint8_t *)p + 12, 0, 18, r);
	bdmf_session_print(session, "\tbuffer_number_0_or_abs_0 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_SCHEDULING_QUEUE_DESCRIPTOR(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register SCHEDULING_QUEUE_DESCRIPTOR\n");

	FIELD_MREAD_8((uint8_t *)p, 7, 1, r);
	bdmf_session_print(session, "\tprefetch_pd              = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p, 0, 7, r);
	bdmf_session_print(session, "\treserved                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 1, 7, 1, r);
	bdmf_session_print(session, "\tblock_type               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 1, 6, 1, r);
	bdmf_session_print(session, "\tenable                   = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 1, 0, 6, r);
	bdmf_session_print(session, "\tbbh_queue_index          = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 2, 7, 1, r);
	bdmf_session_print(session, "\trate_limit_enable        = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 2, 0, 7, r);
	bdmf_session_print(session, "\tscheduler_index          = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 3, r);
	bdmf_session_print(session, "\tqueue_bit_mask           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 4, r);
	bdmf_session_print(session, "\trate_limiter_index       = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 5, r);
	bdmf_session_print(session, "\tquantum_number           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 6, r);
	bdmf_session_print(session, "\tdeficit_counter          = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_TM_FLOW_CNTR_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register TM_FLOW_CNTR_ENTRY\n");

	MREAD_8((uint8_t *)p, r);
	bdmf_session_print(session, "\tcntr_id                  = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_BBH_TX_WAKE_UP_DATA_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register BBH_TX_WAKE_UP_DATA_ENTRY\n");

	FIELD_MREAD_8((uint8_t *)p, 3, 5, r);
	bdmf_session_print(session, "\treserved0                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_32((uint8_t *)p, 8, 19, r);
	bdmf_session_print(session, "\tbytes_in_bbh             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 3, r);
	bdmf_session_print(session, "\tqueue                    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 4, r);
	bdmf_session_print(session, "\treserved1                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_BBH_TX_EGRESS_COUNTER_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register BBH_TX_EGRESS_COUNTER_ENTRY\n");

	MREAD_8((uint8_t *)p, r);
	bdmf_session_print(session, "\tcounter                  = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_32((uint8_t *)p + 0, 0, 24, r);
	bdmf_session_print(session, "\treserved0                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 4, r);
	bdmf_session_print(session, "\treserved1                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_BASIC_SCHEDULER_DESCRIPTOR(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	int i,j;

	bdmf_session_print(session, "  Register BASIC_SCHEDULER_DESCRIPTOR\n");

	FIELD_MREAD_8((uint8_t *)p, 7, 1, r);
	bdmf_session_print(session, "\trate_limit_enable        = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p, 6, 1, r);
	bdmf_session_print(session, "\tcomplex_scheduler_exists = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p, 4, 2, r);
	bdmf_session_print(session, "\tdwrr_offset              = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p, 0, 4, r);
	bdmf_session_print(session, "\tparent_index_2           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 1, r);
	bdmf_session_print(session, "\tslot_budget_bit_vector   = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 2, 3, 5, r);
	bdmf_session_print(session, "\tparent_index_1           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 2, 0, 3, r);
	bdmf_session_print(session, "\tlast_served_queue        = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 3, r);
	bdmf_session_print(session, "\tstatus_bit_vector        = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 4, r);
	bdmf_session_print(session, "\trate_limiter_index       = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 5, r);
	bdmf_session_print(session, "\tquantum_number           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 6, r);
	bdmf_session_print(session, "\tdeficit_counter          = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	bdmf_session_print(session, "\tqueue_index              =\n\t");
	for (i=0,j=0; i<8; i++)
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

void dump_RDD_BASIC_RATE_LIMITER_DESCRIPTOR(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register BASIC_RATE_LIMITER_DESCRIPTOR\n");

	FIELD_MREAD_8((uint8_t *)p, 7, 1, r);
	bdmf_session_print(session, "\trl_type                  = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_32((uint8_t *)p, 10, 21, r);
	bdmf_session_print(session, "\treserved2                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 2, 0, 2, r);
	bdmf_session_print(session, "\tblock_type               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 3, r);
	bdmf_session_print(session, "\tblock_index              = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 4, r);
	bdmf_session_print(session, "\tcurrent_budget           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p + 8, 2, 14, r);
	bdmf_session_print(session, "\talloc_mantissa           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 9, 0, 2, r);
	bdmf_session_print(session, "\talloc_exponent           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p + 10, 2, 14, r);
	bdmf_session_print(session, "\tlimit_mantissa           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 11, 0, 2, r);
	bdmf_session_print(session, "\tlimit_exponent           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 12, r);
	bdmf_session_print(session, "\treserved                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_PACKETS_AND_BYTES(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register PACKETS_AND_BYTES\n");

	MREAD_32((uint8_t *)p, r);
	bdmf_session_print(session, "\tpackets                  = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 4, r);
	bdmf_session_print(session, "\tbytes                    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_BYTES_4(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register BYTES_4\n");

	MREAD_32((uint8_t *)p, r);
	bdmf_session_print(session, "\tbits                     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_G9991_FIFO_POINTER_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register G9991_FIFO_POINTER_ENTRY\n");

	MREAD_32((uint8_t *)p, r);
	bdmf_session_print(session, "\tingress_vector           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 4, r);
	bdmf_session_print(session, "\tegress_vector            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_G9991_FIFO_DESCRIPTOR_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register G9991_FIFO_DESCRIPTOR_ENTRY\n");

	FIELD_MREAD_16((uint8_t *)p, 1, 15, r);
	bdmf_session_print(session, "\tremaining_bytes          = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 1, 0, 1, r);
	bdmf_session_print(session, "\tnot_sof                  = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_NATC_TBL_CONFIGURATION(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	int i,j;

	bdmf_session_print(session, "  Register NATC_TBL_CONFIGURATION\n");

	MREAD_32((uint8_t *)p, r);
	bdmf_session_print(session, "\tkey_addr_low             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 4, r);
	bdmf_session_print(session, "\tkey_addr_high            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 8, r);
	bdmf_session_print(session, "\tres_addr_low             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 12, r);
	bdmf_session_print(session, "\tres_addr_high            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 16, 1, 7, r);
	bdmf_session_print(session, "\treserved                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 16, 0, 1, r);
	bdmf_session_print(session, "\tmiss_cache_enable        = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 17, r);
	bdmf_session_print(session, "\tkey_size                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 18, r);
	bdmf_session_print(session, "\tcontext_size             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	bdmf_session_print(session, "\treserved1                =\n\t");
	for (i=0,j=0; i<5; i++)
	{
		MREAD_I_8((uint8_t *)p + 19, i, r);
		bdmf_session_print(session, "0x%08x ", (unsigned int)r);
		j++;
	}
	bdmf_session_print(session, "\n");
}

void dump_RDD_BYTES_2(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register BYTES_2\n");

	MREAD_16((uint8_t *)p, r);
	bdmf_session_print(session, "\tbits                     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_G9991_PENDING_VECTOR_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register G9991_PENDING_VECTOR_ENTRY\n");

	MREAD_32((uint8_t *)p, r);
	bdmf_session_print(session, "\tvector                   = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_32((uint8_t *)p + 4, 8, 24, r);
	bdmf_session_print(session, "\treserved                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 7, r);
	bdmf_session_print(session, "\tlast_served_sid          = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_PROCESSING_RX_DESCRIPTOR(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register PROCESSING_RX_DESCRIPTOR\n");

	MREAD_32((uint8_t *)p, r);
	bdmf_session_print(session, "\tpd_info                  = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p + 4, 6, 10, r);
	bdmf_session_print(session, "\tserial_num               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 5, 5, 1, r);
	bdmf_session_print(session, "\tploam                    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_32((uint8_t *)p + 4, 14, 7, r);
	bdmf_session_print(session, "\tbn_num                   = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p + 6, 0, 14, r);
	bdmf_session_print(session, "\tpacket_length            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 8, 7, 1, r);
	bdmf_session_print(session, "\terror                    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 8, 6, 1, r);
	bdmf_session_print(session, "\ttarget_mem_1             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 8, 4, 2, r);
	bdmf_session_print(session, "\tcong_state               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 8, 3, 1, r);
	bdmf_session_print(session, "\tingress_cong             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 8, 2, 1, r);
	bdmf_session_print(session, "\tlan                      = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p + 8, 2, 8, r);
	bdmf_session_print(session, "\tingress_port             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_32((uint8_t *)p + 8, 0, 18, r);
	bdmf_session_print(session, "\tbn1_last_or_abs1         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 12, 7, 1, r);
	bdmf_session_print(session, "\tagg_pd                   = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 12, 6, 1, r);
	bdmf_session_print(session, "\ttarget_mem_0             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 12, 5, 1, r);
	bdmf_session_print(session, "\tabs_or_xdsl              = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 12, 1, 4, r);
	bdmf_session_print(session, "\terror_type_or_cpu_tx     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p + 12, 2, 7, r);
	bdmf_session_print(session, "\tsop                      = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_32((uint8_t *)p + 12, 0, 18, r);
	bdmf_session_print(session, "\tbn0_first                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_UPDATE_FIFO_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register UPDATE_FIFO_ENTRY\n");

	FIELD_MREAD_8((uint8_t *)p, 7, 1, r);
	bdmf_session_print(session, "\tvalid                    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p, 6, 1, r);
	bdmf_session_print(session, "\treserved1                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p, 0, 14, r);
	bdmf_session_print(session, "\tpd_fifo_write_ptr        = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 2, 1, 7, r);
	bdmf_session_print(session, "\treserved0                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p + 2, 0, 9, r);
	bdmf_session_print(session, "\tqueue_number             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_BYTE_1(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register BYTE_1\n");

	MREAD_8((uint8_t *)p, r);
	bdmf_session_print(session, "\tbits                     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_G9991_SYSTEM_PORT_BBH_CFG_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register G9991_SYSTEM_PORT_BBH_CFG_ENTRY\n");

	MREAD_16((uint8_t *)p, r);
	bdmf_session_print(session, "\tbbh_tx_id                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 2, r);
	bdmf_session_print(session, "\tbbh_id                   = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_BYTES_8(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	int i,j;

	bdmf_session_print(session, "  Register BYTES_8\n");

	bdmf_session_print(session, "\tbits                     =\n\t");
	for (i=0,j=0; i<2; i++)
	{
		MREAD_I_32((uint8_t *)p, i, r);
		bdmf_session_print(session, "0x%08x ", (unsigned int)r);
		j++;
	}
	bdmf_session_print(session, "\n");
}

void dump_RDD_QUEUE_DYNAMIC_MNG_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register QUEUE_DYNAMIC_MNG_ENTRY\n");

	MREAD_8((uint8_t *)p, r);
	bdmf_session_print(session, "\tqm_queue_us_start        = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 1, r);
	bdmf_session_print(session, "\tqm_queue_us_end          = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 2, r);
	bdmf_session_print(session, "\tqm_queue_ds_start        = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 3, r);
	bdmf_session_print(session, "\tqm_queue_ds_end          = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 4, r);
	bdmf_session_print(session, "\tqm_queue_epon_start      = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 5, r);
	bdmf_session_print(session, "\tqm_queue_sq_start        = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 6, r);
	bdmf_session_print(session, "\treserved                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_COMPLEX_SCHEDULER_DESCRIPTOR(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	int i,j;

	bdmf_session_print(session, "  Register COMPLEX_SCHEDULER_DESCRIPTOR\n");

	FIELD_MREAD_8((uint8_t *)p, 7, 1, r);
	bdmf_session_print(session, "\treserved0                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p, 6, 1, r);
	bdmf_session_print(session, "\tis_positive_budget       = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p, 5, 1, r);
	bdmf_session_print(session, "\treserved1                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p, 7, 6, r);
	bdmf_session_print(session, "\tbbh_queue                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 1, 0, 7, r);
	bdmf_session_print(session, "\trate_limiter_index       = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 2, 5, 3, r);
	bdmf_session_print(session, "\tdwrr_offset_pir          = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 2, 2, 3, r);
	bdmf_session_print(session, "\tdwrr_offset_sir          = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p + 2, 5, 5, r);
	bdmf_session_print(session, "\tlast_served_block_pir    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 3, 0, 5, r);
	bdmf_session_print(session, "\tlast_served_block_sir    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 4, r);
	bdmf_session_print(session, "\tstatus_bit_vector        = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 8, r);
	bdmf_session_print(session, "\tslot_budget_bit_vector_0 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 12, r);
	bdmf_session_print(session, "\tslot_budget_bit_vector_1 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 16, r);
	bdmf_session_print(session, "\tis_scheduler_slot        = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 20, r);
	bdmf_session_print(session, "\tis_scheduler_basic       = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 24, 7, 1, r);
	bdmf_session_print(session, "\tovl_rl_en                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 24, 6, 1, r);
	bdmf_session_print(session, "\trate_limit_enable        = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 24, 5, 1, r);
	bdmf_session_print(session, "\tparent_scheduler_exists  = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 24, 1, 4, r);
	bdmf_session_print(session, "\tparent_scheduler_index   = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p + 24, 4, 5, r);
	bdmf_session_print(session, "\tparent_scheduler_slot_index= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 25, 0, 4, r);
	bdmf_session_print(session, "\treserved2                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 26, r);
	bdmf_session_print(session, "\tdeficit_counter          = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 28, r);
	bdmf_session_print(session, "\tquantum_number           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	bdmf_session_print(session, "\treserved3                =\n\t");
	for (i=0,j=0; i<3; i++)
	{
		MREAD_I_8((uint8_t *)p + 29, i, r);
		bdmf_session_print(session, "0x%08x ", (unsigned int)r);
		j++;
	}
	bdmf_session_print(session, "\n");
	bdmf_session_print(session, "\tblock_index              =\n\t");
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

void dump_RDD_TRACE_EVENT(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register TRACE_EVENT\n");

	FIELD_MREAD_16((uint8_t *)p, 4, 12, r);
	bdmf_session_print(session, "\ttimestamp                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 1, 2, 2, r);
	bdmf_session_print(session, "\tevent_id                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_32((uint8_t *)p + 0, 0, 18, r);
	bdmf_session_print(session, "\ttrace_event_info         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_BBH_QUEUE_DESCRIPTOR(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register BBH_QUEUE_DESCRIPTOR\n");

	FIELD_MREAD_8((uint8_t *)p, 2, 6, r);
	bdmf_session_print(session, "\twan_type_union           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p, 1, 1, r);
	bdmf_session_print(session, "\tmirroring_en             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p, 0, 1, r);
	bdmf_session_print(session, "\tpriority                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 1, 1, 7, r);
	bdmf_session_print(session, "\tscheduler_index          = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 1, 0, 1, r);
	bdmf_session_print(session, "\tscheduler_type           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 2, r);
	bdmf_session_print(session, "\tbb_destination           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 3, r);
	bdmf_session_print(session, "\tingress_counter          = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_DEBUG_PRINT_INFO(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register DEBUG_PRINT_INFO\n");

	MREAD_32((uint8_t *)p, r);
	bdmf_session_print(session, "\taddr_low                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 4, r);
	bdmf_session_print(session, "\taddr_high                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 8, r);
	bdmf_session_print(session, "\tsize                     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 12, r);
	bdmf_session_print(session, "\ttable_id                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_32((uint8_t *)p + 12, 0, 24, r);
	bdmf_session_print(session, "\treserved                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_FPM_GLOBAL_CFG(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register FPM_GLOBAL_CFG\n");

	MREAD_32((uint8_t *)p, r);
	bdmf_session_print(session, "\tfpm_base_low             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 4, r);
	bdmf_session_print(session, "\tfpm_base_high            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 8, r);
	bdmf_session_print(session, "\tfpm_token_size_asr_8     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_32((uint8_t *)p + 8, 0, 24, r);
	bdmf_session_print(session, "\treserved0                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_G9991_SINGLE_FRAGMENT_CFG_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register G9991_SINGLE_FRAGMENT_CFG_ENTRY\n");

	MREAD_8((uint8_t *)p, r);
	bdmf_session_print(session, "\tenable                   = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_TX_EXCEPTION_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register TX_EXCEPTION_ENTRY\n");

	FIELD_MREAD_8((uint8_t *)p, 2, 6, r);
	bdmf_session_print(session, "\treserved1                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p, 1, 1, r);
	bdmf_session_print(session, "\tdbg_stat_en              = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p, 0, 1, r);
	bdmf_session_print(session, "\tvlan_cntr_en             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_MIRRORING_DESCRIPTOR(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register MIRRORING_DESCRIPTOR\n");

	FIELD_MREAD_8((uint8_t *)p, 7, 1, r);
	bdmf_session_print(session, "\tmirror_en                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p, 1, 6, r);
	bdmf_session_print(session, "\tdst_vport                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p, 0, 9, r);
	bdmf_session_print(session, "\tdst_queue                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_RX_FLOW_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register RX_FLOW_ENTRY\n");

	FIELD_MREAD_8((uint8_t *)p, 3, 5, r);
	bdmf_session_print(session, "\tvirtual_port             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p, 2, 1, r);
	bdmf_session_print(session, "\tflow_dest                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p, 1, 1, r);
	bdmf_session_print(session, "\texception                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p, 0, 1, r);
	bdmf_session_print(session, "\tis_lan                   = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 1, r);
	bdmf_session_print(session, "\tcntr_id                  = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_G9991_US_SID_CONTEXT_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	int i,j;

	bdmf_session_print(session, "  Register G9991_US_SID_CONTEXT_ENTRY\n");

	FIELD_MREAD_8((uint8_t *)p, 6, 2, r);
	bdmf_session_print(session, "\treserved0                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p, 0, 14, r);
	bdmf_session_print(session, "\tplen                     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 2, 7, 1, r);
	bdmf_session_print(session, "\treserved1                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 2, 0, 7, r);
	bdmf_session_print(session, "\tfragment_count           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 3, 1, 7, r);
	bdmf_session_print(session, "\treserved2                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 3, 0, 1, r);
	bdmf_session_print(session, "\tstate                    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 4, 6, 2, r);
	bdmf_session_print(session, "\treserved3                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p + 4, 0, 14, r);
	bdmf_session_print(session, "\tbn1_last                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 6, 6, 2, r);
	bdmf_session_print(session, "\treserved4                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p + 6, 0, 14, r);
	bdmf_session_print(session, "\tbn0                      = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 8, 6, 2, r);
	bdmf_session_print(session, "\treserved5                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p + 8, 0, 14, r);
	bdmf_session_print(session, "\tbn1                      = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	bdmf_session_print(session, "\treserved6                =\n\t");
	for (i=0,j=0; i<6; i++)
	{
		MREAD_I_8((uint8_t *)p + 10, i, r);
		bdmf_session_print(session, "0x%08x ", (unsigned int)r);
		j++;
	}
	bdmf_session_print(session, "\n");
}

void dump_RDD_CBS_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register CBS_ENTRY\n");

	MREAD_32((uint8_t *)p, r);
	bdmf_session_print(session, "\tcommitted_burst_size     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_CPU_TX_DBG_CNTRS(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	int i,j;

	bdmf_session_print(session, "  Register CPU_TX_DBG_CNTRS\n");

	MREAD_32((uint8_t *)p, r);
	bdmf_session_print(session, "\tSBPM_NO_NEXT             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 4, r);
	bdmf_session_print(session, "\tSBPM_NO_FIRST            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 8, r);
	bdmf_session_print(session, "\tTX_FLOW_DISABLE          = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 12, r);
	bdmf_session_print(session, "\tNO_FPM                   = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 16, r);
	bdmf_session_print(session, "\tTASK_EXIT                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 20, r);
	bdmf_session_print(session, "\tSYNC_WAIT                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 24, r);
	bdmf_session_print(session, "\tNO_DISPATCHER_SCHEDULER  = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 28, r);
	bdmf_session_print(session, "\tRECYCLE_FIFO_FULL        = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 32, r);
	bdmf_session_print(session, "\tDROP_PKT                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 36, r);
	bdmf_session_print(session, "\tTASK_START               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 40, r);
	bdmf_session_print(session, "\tNO_FWD                   = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	bdmf_session_print(session, "\treserved                 =\n\t");
	for (i=0,j=0; i<5; i++)
	{
		MREAD_I_32((uint8_t *)p + 44, i, r);
		bdmf_session_print(session, "0x%08x ", (unsigned int)r);
		j++;
	}
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

	MREAD_8((uint8_t *)p + 6, r);
	bdmf_session_print(session, "\tbudget_residue           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 7, r);
	bdmf_session_print(session, "\tcurrent_cycle            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_REPORTING_QUEUE_DESCRIPTOR(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	int i,j;

	bdmf_session_print(session, "  Register REPORTING_QUEUE_DESCRIPTOR\n");

	MREAD_32((uint8_t *)p, r);
	bdmf_session_print(session, "\treported_bytes           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 4, r);
	bdmf_session_print(session, "\treported_overhead        = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 8, r);
	bdmf_session_print(session, "\tcounter_ptr              = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	bdmf_session_print(session, "\treserved                 =\n\t");
	for (i=0,j=0; i<6; i++)
	{
		MREAD_I_8((uint8_t *)p + 10, i, r);
		bdmf_session_print(session, "0x%08x ", (unsigned int)r);
		j++;
	}
	bdmf_session_print(session, "\n");
}

void dump_RDD_TX_FLOW_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register TX_FLOW_ENTRY\n");

	FIELD_MREAD_8((uint8_t *)p, 7, 1, r);
	bdmf_session_print(session, "\tvalid                    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p, 0, 7, r);
	bdmf_session_print(session, "\tqos_table_ptr            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_FW_POLICER_BUDGET_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register FW_POLICER_BUDGET_ENTRY\n");

	FIELD_MREAD_16((uint8_t *)p, 2, 14, r);
	bdmf_session_print(session, "\tbudget_mantissa          = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 1, 0, 2, r);
	bdmf_session_print(session, "\tbudget_exponent          = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_FPM_POOL_NUMBER(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register FPM_POOL_NUMBER\n");

	FIELD_MREAD_8((uint8_t *)p, 2, 6, r);
	bdmf_session_print(session, "\treserved                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p, 0, 2, r);
	bdmf_session_print(session, "\tpool_number              = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_CPU_RING_INTERRUPT_COUNTER_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register CPU_RING_INTERRUPT_COUNTER_ENTRY\n");

	MREAD_32((uint8_t *)p, r);
	bdmf_session_print(session, "\tcounter                  = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 4, r);
	bdmf_session_print(session, "\tmax_size                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_GHOST_REPORTING_GLOBAL_CFG_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register GHOST_REPORTING_GLOBAL_CFG_ENTRY\n");

	FIELD_MREAD_8((uint8_t *)p, 5, 3, r);
	bdmf_session_print(session, "\tmac_type                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p, 4, 1, r);
	bdmf_session_print(session, "\tdisable                  = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p, 0, 12, r);
	bdmf_session_print(session, "\treserved                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 2, r);
	bdmf_session_print(session, "\ttimer_interval           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_CPU_INTERRUPT_COALESCING_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register CPU_INTERRUPT_COALESCING_ENTRY\n");

	MREAD_32((uint8_t *)p, r);
	bdmf_session_print(session, "\ttimer_period             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 4, r);
	bdmf_session_print(session, "\tnot_empty_or_int_vec     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 8, r);
	bdmf_session_print(session, "\tscratch                  = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 12, r);
	bdmf_session_print(session, "\trecycle_timer_cnt        = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 14, r);
	bdmf_session_print(session, "\trecycle_timer_factor     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_EMAC_FLOW_CTRL_BUDGET_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register EMAC_FLOW_CTRL_BUDGET_ENTRY\n");

	MREAD_32((uint8_t *)p, r);
	bdmf_session_print(session, "\tbudget                   = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_REPORTING_ACCUMULATED_DATA(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register REPORTING_ACCUMULATED_DATA\n");

	MREAD_32((uint8_t *)p, r);
	bdmf_session_print(session, "\taccumulated_bytes        = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 4, r);
	bdmf_session_print(session, "\taccumulated_overhead     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_CSO_CONTEXT_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	int i,j;

	bdmf_session_print(session, "  Register CSO_CONTEXT_ENTRY\n");

	MREAD_32((uint8_t *)p, r);
	bdmf_session_print(session, "\treserved                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 4, r);
	bdmf_session_print(session, "\tcsum_value               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 6, r);
	bdmf_session_print(session, "\tcam_result               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 8, r);
	bdmf_session_print(session, "\tcrc_result               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 12, r);
	bdmf_session_print(session, "\tsaved_ret_address        = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 16, r);
	bdmf_session_print(session, "\tsaved_pkt_desc_0         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 20, r);
	bdmf_session_print(session, "\tsaved_pkt_desc_1         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 24, r);
	bdmf_session_print(session, "\tsaved_pkt_desc_2         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 28, r);
	bdmf_session_print(session, "\tsaved_pkt_desc_3         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 32, r);
	bdmf_session_print(session, "\tgood_csum_packets        = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 36, r);
	bdmf_session_print(session, "\tno_cso_support_packets   = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 40, r);
	bdmf_session_print(session, "\tbad_ipv4_hdr_csum_packets= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 44, r);
	bdmf_session_print(session, "\tbad_tcp_udp_csum_packets = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	bdmf_session_print(session, "\tparser_result            =\n\t");
	for (i=0,j=0; i<14; i++)
	{
		MREAD_I_32((uint8_t *)p + 48, i, r);
		bdmf_session_print(session, "0x%08x ", (unsigned int)r);
		j++;
		if (j >= 8)
		{
			j = 0;
			bdmf_session_print(session, "\n\t");
		}
	}
	bdmf_session_print(session, "\n");
	MREAD_16((uint8_t *)p + 104, r);
	bdmf_session_print(session, "\tsaved_buffer_number      = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 106, r);
	bdmf_session_print(session, "\tsaved_packet_length      = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_DDR_ADDRESS(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register DDR_ADDRESS\n");

	MREAD_32((uint8_t *)p, r);
	bdmf_session_print(session, "\tlow                      = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 4, r);
	bdmf_session_print(session, "\thigh                     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_UDPSPDTEST_STREAM_RX_STAT(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register UDPSPDTEST_STREAM_RX_STAT\n");

	MREAD_32((uint8_t *)p, r);
	bdmf_session_print(session, "\trx_bytes_0               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 4, r);
	bdmf_session_print(session, "\trx_bytes_1               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 8, r);
	bdmf_session_print(session, "\trx_packets_0             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 12, r);
	bdmf_session_print(session, "\trx_packets_1             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 16, r);
	bdmf_session_print(session, "\tts_first                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 20, r);
	bdmf_session_print(session, "\tts_last_0                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 24, r);
	bdmf_session_print(session, "\tts_last_1                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 28, 7, 1, r);
	bdmf_session_print(session, "\tts_first_set             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p + 28, 0, 15, r);
	bdmf_session_print(session, "\treserved                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 30, r);
	bdmf_session_print(session, "\tbad_proto_cntr           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_RING_CPU_TX_DESCRIPTOR(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register RING_CPU_TX_DESCRIPTOR\n");

	FIELD_MREAD_8((uint8_t *)p, 7, 1, r);
	bdmf_session_print(session, "\tis_egress                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p, 6, 9, r);
	bdmf_session_print(session, "\tfirst_level_q            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_32((uint8_t *)p + 0, 8, 14, r);
	bdmf_session_print(session, "\tpacket_length            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 3, r);
	bdmf_session_print(session, "\tsk_buf_ptr_high          = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 4, r);
	bdmf_session_print(session, "\tsk_buf_ptr_low_or_data_1588= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 8, 7, 1, r);
	bdmf_session_print(session, "\tcolor                    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 8, 6, 1, r);
	bdmf_session_print(session, "\tdo_not_recycle           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 8, 5, 1, r);
	bdmf_session_print(session, "\tflag_1588                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 8, 4, 1, r);
	bdmf_session_print(session, "\tlan                      = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p + 8, 4, 8, r);
	bdmf_session_print(session, "\twan_flow_source_port_union= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 9, 3, 1, r);
	bdmf_session_print(session, "\tfpm_fallback             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 9, 2, 1, r);
	bdmf_session_print(session, "\tsbpm_copy                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 9, 1, 1, r);
	bdmf_session_print(session, "\ttarget_mem_0             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 9, 0, 1, r);
	bdmf_session_print(session, "\tabs                      = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 10, 6, 2, r);
	bdmf_session_print(session, "\tlag_index                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 10, 2, 4, r);
	bdmf_session_print(session, "\tssid                     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 10, 1, 1, r);
	bdmf_session_print(session, "\tegress_dont_drop         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 10, 0, 1, r);
	bdmf_session_print(session, "\treserved                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 11, r);
	bdmf_session_print(session, "\tpkt_buf_ptr_high         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 12, r);
	bdmf_session_print(session, "\tpkt_buf_ptr_low_or_fpm_bn0= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_CPU_RING_DESCRIPTOR(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register CPU_RING_DESCRIPTOR\n");

	FIELD_MREAD_8((uint8_t *)p, 3, 5, r);
	bdmf_session_print(session, "\tsize_of_entry            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p, 0, 11, r);
	bdmf_session_print(session, "\tnumber_of_entries        = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 2, r);
	bdmf_session_print(session, "\tinterrupt_id             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 4, r);
	bdmf_session_print(session, "\tdrop_counter             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 6, r);
	bdmf_session_print(session, "\twrite_idx                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 8, r);
	bdmf_session_print(session, "\tbase_addr_low            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 12, r);
	bdmf_session_print(session, "\tread_idx                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 14, r);
	bdmf_session_print(session, "\treserved0                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 15, r);
	bdmf_session_print(session, "\tbase_addr_high           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_CPU_TX_SYNC_FIFO_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register CPU_TX_SYNC_FIFO_ENTRY\n");

	MREAD_16((uint8_t *)p, r);
	bdmf_session_print(session, "\tWRITE_PTR                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 2, r);
	bdmf_session_print(session, "\tREAD_PTR                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 4, r);
	bdmf_session_print(session, "\tFIFO                     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 6, r);
	bdmf_session_print(session, "\treserved                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_SPDSVC_RX_TS_STAT(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register SPDSVC_RX_TS_STAT\n");

	MREAD_32((uint8_t *)p, r);
	bdmf_session_print(session, "\tts_first                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 4, r);
	bdmf_session_print(session, "\tts_last                  = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_32((uint8_t *)p + 8, 1, 31, r);
	bdmf_session_print(session, "\treserved                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 11, 0, 1, r);
	bdmf_session_print(session, "\tts_first_set             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_CPU_TX_RING_INDICES(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register CPU_TX_RING_INDICES\n");

	MREAD_16((uint8_t *)p, r);
	bdmf_session_print(session, "\tread_idx                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 2, r);
	bdmf_session_print(session, "\twrite_idx                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_OVERALL_RATE_LIMITER_DESCRIPTOR(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register OVERALL_RATE_LIMITER_DESCRIPTOR\n");

	MREAD_32((uint8_t *)p, r);
	bdmf_session_print(session, "\tbbh_queue_en_vec         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 4, r);
	bdmf_session_print(session, "\tcurrent_budget           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p + 8, 2, 14, r);
	bdmf_session_print(session, "\talloc_mantissa           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 9, 0, 2, r);
	bdmf_session_print(session, "\talloc_exponent           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p + 10, 2, 14, r);
	bdmf_session_print(session, "\tlimit_mantissa           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 11, 0, 2, r);
	bdmf_session_print(session, "\tlimit_exponent           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 12, r);
	bdmf_session_print(session, "\treserved1                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_HW_FLUSH_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register HW_FLUSH_ENTRY\n");

	MREAD_16((uint8_t *)p, r);
	bdmf_session_print(session, "\tqm_queue                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 2, 7, 1, r);
	bdmf_session_print(session, "\thw_flush_en              = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 2, 6, 1, r);
	bdmf_session_print(session, "\tus                       = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 2, 0, 6, r);
	bdmf_session_print(session, "\treserved                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 3, r);
	bdmf_session_print(session, "\tflush_aggr               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_PAUSE_QUANTA_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register PAUSE_QUANTA_ENTRY\n");

	FIELD_MREAD_8((uint8_t *)p, 7, 1, r);
	bdmf_session_print(session, "\tignore                   = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p, 0, 15, r);
	bdmf_session_print(session, "\ttime_unit                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_MAC_TYPE_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register MAC_TYPE_ENTRY\n");

	MREAD_8((uint8_t *)p, r);
	bdmf_session_print(session, "\ttype                     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_PACKET_BUFFER(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	int i,j;

	bdmf_session_print(session, "  Register PACKET_BUFFER\n");

	bdmf_session_print(session, "\tpacket_header            =\n\t");
	for (i=0,j=0; i<128; i++)
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
	bdmf_session_print(session, "\tparser_result            =\n\t");
	for (i=0,j=0; i<56; i++)
	{
		MREAD_I_8((uint8_t *)p + 128, i, r);
		bdmf_session_print(session, "0x%08x ", (unsigned int)r);
		j++;
		if (j >= 8)
		{
			j = 0;
			bdmf_session_print(session, "\n\t");
		}
	}
	bdmf_session_print(session, "\n");
	bdmf_session_print(session, "\tscratch                  =\n\t");
	for (i=0,j=0; i<136; i++)
	{
		MREAD_I_8((uint8_t *)p + 184, i, r);
		bdmf_session_print(session, "0x%08x ", (unsigned int)r);
		j++;
		if (j >= 8)
		{
			j = 0;
			bdmf_session_print(session, "\n\t");
		}
	}
	bdmf_session_print(session, "\n");
	bdmf_session_print(session, "\tpd                       =\n\t");
	for (i=0,j=0; i<16; i++)
	{
		MREAD_I_8((uint8_t *)p + 320, i, r);
		bdmf_session_print(session, "0x%08x ", (unsigned int)r);
		j++;
		if (j >= 8)
		{
			j = 0;
			bdmf_session_print(session, "\n\t");
		}
	}
	bdmf_session_print(session, "\n");
	bdmf_session_print(session, "\tresolution_context       =\n\t");
	for (i=0,j=0; i<24; i++)
	{
		MREAD_I_8((uint8_t *)p + 336, i, r);
		bdmf_session_print(session, "0x%08x ", (unsigned int)r);
		j++;
		if (j >= 8)
		{
			j = 0;
			bdmf_session_print(session, "\n\t");
		}
	}
	bdmf_session_print(session, "\n");
	bdmf_session_print(session, "\tclassification_results   =\n\t");
	for (i=0,j=0; i<6; i++)
	{
		MREAD_I_32((uint8_t *)p + 360, i, r);
		bdmf_session_print(session, "0x%08x ", (unsigned int)r);
		j++;
	}
	bdmf_session_print(session, "\n");
	bdmf_session_print(session, "\tclassification_contexts_list=\n\t");
	for (i=0,j=0; i<128; i++)
	{
		MREAD_I_8((uint8_t *)p + 384, i, r);
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

void dump_RDD_TCAM_IC_CMD(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register TCAM_IC_CMD\n");

	MREAD_16((uint8_t *)p, r);
	bdmf_session_print(session, "\thandler_ptr              = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 2, r);
	bdmf_session_print(session, "\tsrc_offset               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 3, 6, 2, r);
	bdmf_session_print(session, "\treserved                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 3, 0, 6, r);
	bdmf_session_print(session, "\tdst_offset               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_INGRESS_CONGESTION_FLOW_CTRL_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register INGRESS_CONGESTION_FLOW_CTRL_ENTRY\n");

	MREAD_8((uint8_t *)p, r);
	bdmf_session_print(session, "\tbbh_id                   = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 1, r);
	bdmf_session_print(session, "\tenable                   = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_LAYER2_HEADER_COPY_MAPPING_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register LAYER2_HEADER_COPY_MAPPING_ENTRY\n");

	MREAD_16((uint8_t *)p, r);
	bdmf_session_print(session, "\troutine                  = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 2, r);
	bdmf_session_print(session, "\tdst_buffer_offset        = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_TC_TO_QUEUE_8(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	int i,j;

	bdmf_session_print(session, "  Register TC_TO_QUEUE_8\n");

	bdmf_session_print(session, "\tqueue_offset             =\n\t");
	for (i=0,j=0; i<8; i++)
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

void dump_RDD_POLICER_PARAMS_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register POLICER_PARAMS_ENTRY\n");

	FIELD_MREAD_8((uint8_t *)p, 5, 3, r);
	bdmf_session_print(session, "\tfactor_bytes             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p, 4, 1, r);
	bdmf_session_print(session, "\tdei_mode                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p, 3, 1, r);
	bdmf_session_print(session, "\tsingle_bucket            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p, 0, 3, r);
	bdmf_session_print(session, "\treserved                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_INGRESS_FILTER_CTRL(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register INGRESS_FILTER_CTRL\n");

	FIELD_MREAD_8((uint8_t *)p, 7, 1, r);
	bdmf_session_print(session, "\treserved                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_32((uint8_t *)p, 0, 31, r);
	bdmf_session_print(session, "\tenable_vector            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 4, 7, 1, r);
	bdmf_session_print(session, "\tcpu_bypass               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_32((uint8_t *)p + 4, 0, 31, r);
	bdmf_session_print(session, "\taction_vector            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_MAC_ADDRESS(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register MAC_ADDRESS\n");

	MREAD_16((uint8_t *)p, r);
	bdmf_session_print(session, "\treserved                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 2, r);
	bdmf_session_print(session, "\tda1                      = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 3, r);
	bdmf_session_print(session, "\tda2                      = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 4, r);
	bdmf_session_print(session, "\tda3                      = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 5, r);
	bdmf_session_print(session, "\tda4                      = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 6, r);
	bdmf_session_print(session, "\tda5                      = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 7, r);
	bdmf_session_print(session, "\tda6                      = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_BRIDGE_CFG(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register BRIDGE_CFG\n");

	FIELD_MREAD_8((uint8_t *)p, 7, 1, r);
	bdmf_session_print(session, "\thit                      = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p, 6, 1, r);
	bdmf_session_print(session, "\tbridge_fw_failed         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p, 5, 1, r);
	bdmf_session_print(session, "\tvlan_aggregation         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p, 0, 5, r);
	bdmf_session_print(session, "\treserved1                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 1, 1, 7, r);
	bdmf_session_print(session, "\treserved2                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 1, 0, 1, r);
	bdmf_session_print(session, "\tbridge_results_available = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 2, r);
	bdmf_session_print(session, "\tres_offset               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 4, r);
	bdmf_session_print(session, "\tcontext_offset           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 6, r);
	bdmf_session_print(session, "\treserved                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_PBIT_TO_QUEUE_8(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	int i,j;

	bdmf_session_print(session, "  Register PBIT_TO_QUEUE_8\n");

	bdmf_session_print(session, "\tqueue_offset             =\n\t");
	for (i=0,j=0; i<8; i++)
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

void dump_RDD_DEF_FLOW_CONTEXT_DDR_ADDR(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register DEF_FLOW_CONTEXT_DDR_ADDR\n");

	MREAD_32((uint8_t *)p, r);
	bdmf_session_print(session, "\tlow                      = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 4, r);
	bdmf_session_print(session, "\thigh                     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_HW_IPTV_CONFIGURATION(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register HW_IPTV_CONFIGURATION\n");

	MREAD_16((uint8_t *)p, r);
	bdmf_session_print(session, "\tfpm_base_token_size      = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 2, 7, 1, r);
	bdmf_session_print(session, "\treserved0                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 2, 0, 7, r);
	bdmf_session_print(session, "\thn_size1                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 3, 7, 1, r);
	bdmf_session_print(session, "\treserved1                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 3, 0, 7, r);
	bdmf_session_print(session, "\thn_size0                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 4, 3, 5, r);
	bdmf_session_print(session, "\treserved2                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p + 4, 0, 11, r);
	bdmf_session_print(session, "\tddr_sop_offset0          = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 6, 3, 5, r);
	bdmf_session_print(session, "\treserved3                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p + 6, 0, 11, r);
	bdmf_session_print(session, "\tddr_sop_offset1          = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 8, r);
	bdmf_session_print(session, "\tfpm_pool1_stat2_addr     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_INGRESS_PACKET_BASED_MAPPING_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register INGRESS_PACKET_BASED_MAPPING_ENTRY\n");

	MREAD_8((uint8_t *)p, r);
	bdmf_session_print(session, "\tus                       = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 1, r);
	bdmf_session_print(session, "\tds                       = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 2, r);
	bdmf_session_print(session, "\treserved                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_VPORT_CFG_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register VPORT_CFG_ENTRY\n");

	FIELD_MREAD_8((uint8_t *)p, 7, 1, r);
	bdmf_session_print(session, "\tingress_congestion       = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p, 6, 1, r);
	bdmf_session_print(session, "\tdiscard_prty             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p, 5, 1, r);
	bdmf_session_print(session, "\tls_fc_cfg                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p, 4, 1, r);
	bdmf_session_print(session, "\tflow_control             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p, 3, 1, r);
	bdmf_session_print(session, "\tegress_isolation_en      = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p, 2, 1, r);
	bdmf_session_print(session, "\tingress_isolation_en     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p, 1, 1, r);
	bdmf_session_print(session, "\tsa_lookup_en             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p, 0, 1, r);
	bdmf_session_print(session, "\tda_lookup_en             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 1, 6, 2, r) /*defined by rdd_action enumeration*/;
	bdmf_session_print(session, "\tsa_lookup_miss_action    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 1, 4, 2, r) /*defined by rdd_action enumeration*/;
	bdmf_session_print(session, "\tda_lookup_miss_action    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 1, 0, 4, r);
	bdmf_session_print(session, "\tprotocol_filters_dis     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 2, r);
	bdmf_session_print(session, "\tbridge_id                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 3, 7, 1, r) /*defined by rdd_bridge_and_vlan_ctx_ingress_lookup_method enumeration*/;
	bdmf_session_print(session, "\tbridge_and_vlan_ingress_lookup_method= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 3, 6, 1, r) /*defined by rdd_bridge_and_vlan_ctx_egress_lookup_method enumeration*/;
	bdmf_session_print(session, "\tbridge_and_vlan_egress_lookup_method= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 3, 5, 1, r);
	bdmf_session_print(session, "\tanti_spoofing_bypass     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 3, 0, 5, r);
	bdmf_session_print(session, "\treserved                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 4, r);
	bdmf_session_print(session, "\tegress_isolation_map     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_TCAM_IC_CFG(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register TCAM_IC_CFG\n");

	MREAD_16((uint8_t *)p, r);
	bdmf_session_print(session, "\tres_offset               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 2, r);
	bdmf_session_print(session, "\tcontext_offset           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 4, r);
	bdmf_session_print(session, "\tcmd_table                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 6, r);
	bdmf_session_print(session, "\tscratch_offset           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 8, 5, 3, r);
	bdmf_session_print(session, "\ttable_id                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p + 8, 0, 13, r);
	bdmf_session_print(session, "\tkey_size                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p + 10, 1, 15, r);
	bdmf_session_print(session, "\treserved                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 11, 0, 1, r);
	bdmf_session_print(session, "\tenable                   = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 12, r);
	bdmf_session_print(session, "\tclassification_result    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_SYSTEM_CONFIGURATION_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register SYSTEM_CONFIGURATION_ENTRY\n");

	MREAD_16((uint8_t *)p, r);
	bdmf_session_print(session, "\tds_drop_precedence_eligibility_vector= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 2, r);
	bdmf_session_print(session, "\tus_drop_precedence_eligibility_vector= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 4, 7, 1, r);
	bdmf_session_print(session, "\tfull_flow_cache_mode     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 4, 6, 1, r);
	bdmf_session_print(session, "\tl2_flow_cache_mode       = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 4, 5, 1, r);
	bdmf_session_print(session, "\tiptv_lookup_miss_action  = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 4, 4, 1, r);
	bdmf_session_print(session, "\tvlan_stats_enable        = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 4, 2, 2, r);
	bdmf_session_print(session, "\treserved                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 4, 1, 1, r);
	bdmf_session_print(session, "\tingress_packet_based_mapping_ds_or_us= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 4, 0, 1, r);
	bdmf_session_print(session, "\ttcp_pure_ack_flows       = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_EMAC_FLOW_CTRL_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register EMAC_FLOW_CTRL_ENTRY\n");

	MREAD_32((uint8_t *)p, r);
	bdmf_session_print(session, "\trate                     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 4, r);
	bdmf_session_print(session, "\tmax_burst_size           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 8, r);
	bdmf_session_print(session, "\tthreshold                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 12, r);
	bdmf_session_print(session, "\treserved                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_VPORT_CFG_EX_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register VPORT_CFG_EX_ENTRY\n");

	FIELD_MREAD_8((uint8_t *)p, 7, 1, r);
	bdmf_session_print(session, "\tloopback_en              = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p, 6, 1, r);
	bdmf_session_print(session, "\tmirroring_en             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p, 0, 6, r);
	bdmf_session_print(session, "\tingress_filter_profile   = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 1, 5, 3, r);
	bdmf_session_print(session, "\tport_mac_addr_idx        = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 1, 2, 3, r);
	bdmf_session_print(session, "\temac_idx                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 1, 0, 2, r);
	bdmf_session_print(session, "\treserved                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 2, 5, 3, r);
	bdmf_session_print(session, "\tviq                      = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 2, 4, 1, r);
	bdmf_session_print(session, "\trate_limit_unknown_da    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 2, 3, 1, r);
	bdmf_session_print(session, "\trate_limit_broadcast     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 2, 2, 1, r);
	bdmf_session_print(session, "\trate_limit_multicast     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 2, 0, 2, r);
	bdmf_session_print(session, "\treserved1                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 3, 7, 1, r);
	bdmf_session_print(session, "\tport_dbg_stat_en         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 3, 0, 7, r);
	bdmf_session_print(session, "\tpolicer_idx              = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_IPTV_CFG(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register IPTV_CFG\n");

	MREAD_16((uint8_t *)p, r);
	bdmf_session_print(session, "\tres_offset               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 2, r);
	bdmf_session_print(session, "\tcontext_offset           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 4, r);
	bdmf_session_print(session, "\tkey_offset               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 6, r);
	bdmf_session_print(session, "\thash_tbl_idx             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 7, r);
	bdmf_session_print(session, "\tl2_mcast                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_IPTV_DDR_CTX_TABLE_ADDRESS(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register IPTV_DDR_CTX_TABLE_ADDRESS\n");

	MREAD_32((uint8_t *)p, r);
	bdmf_session_print(session, "\tlow                      = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 4, r);
	bdmf_session_print(session, "\thigh                     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_TUNNEL_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
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

	MREAD_16((uint8_t *)p + 8, r);
	bdmf_session_print(session, "\tlayer3_total_len         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 10, r);
	bdmf_session_print(session, "\tlayer3_chksum            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 12, r);
	bdmf_session_print(session, "\tgre_proto_offset         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_32((uint8_t *)p + 12, 0, 24, r);
	bdmf_session_print(session, "\treserved1                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_TUNNEL_HEADER_PSRAM_BUFFER(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register TUNNEL_HEADER_PSRAM_BUFFER\n");

	MREAD_32((uint8_t *)p, r);
	bdmf_session_print(session, "\tnumber                   = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_NAT_CACHE_CFG(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register NAT_CACHE_CFG\n");

	MREAD_16((uint8_t *)p, r);
	bdmf_session_print(session, "\tres_offset               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 2, r);
	bdmf_session_print(session, "\tcontext_offset           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 4, r);
	bdmf_session_print(session, "\tkey_offset               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 6, 3, 5, r);
	bdmf_session_print(session, "\treserved                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 6, 2, 1, r);
	bdmf_session_print(session, "\tthree_tuple_enable       = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 6, 1, 1, r);
	bdmf_session_print(session, "\tesp_filter_action        = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 6, 0, 1, r);
	bdmf_session_print(session, "\tesp_filter_enable        = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_ECN_IPV6_REMARK_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register ECN_IPV6_REMARK_ENTRY\n");

	FIELD_MREAD_8((uint8_t *)p, 1, 7, r);
	bdmf_session_print(session, "\treserved                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p, 0, 1, r);
	bdmf_session_print(session, "\tenabled                  = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_CPU_REDIRECT_MODE_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register CPU_REDIRECT_MODE_ENTRY\n");

	MREAD_8((uint8_t *)p, r);
	bdmf_session_print(session, "\tmode                     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_IPTV_CLASSIFICATION_CFG_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register IPTV_CLASSIFICATION_CFG_ENTRY\n");

	FIELD_MREAD_8((uint8_t *)p, 7, 1, r);
	bdmf_session_print(session, "\tda_prefix_mode_mac       = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p, 6, 1, r);
	bdmf_session_print(session, "\tda_prefix_mode_ip        = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p, 5, 1, r);
	bdmf_session_print(session, "\tiptv_en                  = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p, 0, 5, r);
	bdmf_session_print(session, "\treserved1                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_INGRESS_FILTER_CFG(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register INGRESS_FILTER_CFG\n");

	MREAD_16((uint8_t *)p, r);
	bdmf_session_print(session, "\tres_offset               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

