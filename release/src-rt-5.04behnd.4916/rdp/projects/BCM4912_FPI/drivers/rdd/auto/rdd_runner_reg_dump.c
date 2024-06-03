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

void dump_RDD_IPTV_DDR_PORT_BUFFER_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	int i,j;

	bdmf_session_print(session, "  Register IPTV_DDR_PORT_BUFFER_ENTRY\n");

	bdmf_session_print(session, "\theader                   =\n\t");
	for (i=0,j=0; i<56; i++)
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
	MREAD_32((uint8_t *)(p) + 56, (r));
	bdmf_session_print(session, "\tcontext_0                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 60, (r));
	bdmf_session_print(session, "\tcontext_1                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_PROCESSING_TX_DESCRIPTOR(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register PROCESSING_TX_DESCRIPTOR\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p), 7, 1, (r));
	bdmf_session_print(session, "\tvalid                    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p), 6, 1, (r));
	bdmf_session_print(session, "\theadroom                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p), 5, 1, (r));
	bdmf_session_print(session, "\tdont_agg                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p), 4, 1, (r));
	bdmf_session_print(session, "\tmc_copy                  = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p), 3, 1, (r));
	bdmf_session_print(session, "\treprocess                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p), 2, 1, (r));
	bdmf_session_print(session, "\tcolor                    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p), 1, 1, (r));
	bdmf_session_print(session, "\tforce_copy               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_16((uint8_t *)(p), 0, 9, (r));
	bdmf_session_print(session, "\tsecond_level_q_aqm_ts_spdsvs= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_16((uint8_t *)(p) + 2, 7, 9, (r));
	bdmf_session_print(session, "\tfirst_level_q            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 3, 6, 1, (r));
	bdmf_session_print(session, "\tflag_1588                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 3, 5, 1, (r));
	bdmf_session_print(session, "\tcoherent                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 3, 0, 5, (r));
	bdmf_session_print(session, "\thn                       = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_16((uint8_t *)(p) + 4, 6, 10, (r));
	bdmf_session_print(session, "\tserial_num               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 5, 5, 1, (r));
	bdmf_session_print(session, "\tpriority                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 5, 4, 1, (r));
	bdmf_session_print(session, "\tingress_cong             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 5, 3, 1, (r));
	bdmf_session_print(session, "\tabs                      = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 5, 2, 1, (r));
	bdmf_session_print(session, "\tgdx_rx_dma_done          = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 5, 1, 1, (r));
	bdmf_session_print(session, "\tcpu_tx_or_is_hw_cso      = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 5, 0, 1, (r));
	bdmf_session_print(session, "\tis_common_reprocessing   = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 6, 7, 1, (r));
	bdmf_session_print(session, "\tis_spdsvc_abs            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 6, 6, 1, (r));
	bdmf_session_print(session, "\treserved_1_1             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_16((uint8_t *)(p) + 6, 0, 14, (r));
	bdmf_session_print(session, "\tpacket_length            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 8, 7, 1, (r));
	bdmf_session_print(session, "\tdrop                     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 8, 6, 1, (r));
	bdmf_session_print(session, "\ttarget_mem_1             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 8, 4, 2, (r));
	bdmf_session_print(session, "\tcong_state_stream        = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 8, 3, 1, (r));
	bdmf_session_print(session, "\tis_emac                  = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_16((uint8_t *)(p) + 8, 3, 8, (r));
	bdmf_session_print(session, "\tingress_port             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_32((uint8_t *)(p) + 8, 0, 19, (r));
	bdmf_session_print(session, "\tunion3                   = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 12, 7, 1, (r));
	bdmf_session_print(session, "\tagg_pd                   = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 12, 6, 1, (r));
	bdmf_session_print(session, "\ttarget_mem_0             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_32((uint8_t *)(p) + 12, 0, 30, (r));
	bdmf_session_print(session, "\tpayload_offset_sop       = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_SCHEDULING_QUEUE_DESCRIPTOR(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register SCHEDULING_QUEUE_DESCRIPTOR\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p), 7, 1, (r));
	bdmf_session_print(session, "\tprefetch_pd              = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p), 6, 1, (r));
	bdmf_session_print(session, "\tcodel_dropped_recently   = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p), 0, 6, (r));
	bdmf_session_print(session, "\tbbh_queue_index          = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 1, 7, 1, (r));
	bdmf_session_print(session, "\tblock_type               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 1, 6, 1, (r));
	bdmf_session_print(session, "\tenable                   = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 1, 5, 1, (r));
	bdmf_session_print(session, "\trate_limit_enable        = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 1, 4, 1, (r));
	bdmf_session_print(session, "\tcodel_enable             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 1, 3, 1, (r));
	bdmf_session_print(session, "\tpi2_enable               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 1, 2, 1, (r));
	bdmf_session_print(session, "\taqm_enable               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 1, 1, 1, (r));
	bdmf_session_print(session, "\tlaqm_enable              = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 1, 0, 1, (r));
	bdmf_session_print(session, "\treserved1                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 2, 7, 1, (r));
	bdmf_session_print(session, "\tcodel_dropping           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 2, 0, 7, (r));
	bdmf_session_print(session, "\tscheduler_index          = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 3, (r));
	bdmf_session_print(session, "\tqueue_bit_mask           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 4, (r));
	bdmf_session_print(session, "\trate_limiter_index       = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 5, (r));
	bdmf_session_print(session, "\tquantum_number           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 6, (r));
	bdmf_session_print(session, "\tdeficit_counter          = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_BASIC_SCHEDULER_DESCRIPTOR(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	int i,j;

	bdmf_session_print(session, "  Register BASIC_SCHEDULER_DESCRIPTOR\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p), 7, 1, (r));
	bdmf_session_print(session, "\trate_limit_enable        = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p), 6, 1, (r));
	bdmf_session_print(session, "\tcomplex_scheduler_exists = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p), 4, 2, (r));
	bdmf_session_print(session, "\tdwrr_offset              = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p), 0, 4, (r));
	bdmf_session_print(session, "\tparent_index_2           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 1, (r));
	bdmf_session_print(session, "\tslot_budget_bit_vector   = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 2, 7, 1, (r));
	bdmf_session_print(session, "\tis_positive_budget       = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 2, 3, 4, (r));
	bdmf_session_print(session, "\tparent_index_1           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 2, 0, 3, (r));
	bdmf_session_print(session, "\tlast_served_queue        = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 3, (r));
	bdmf_session_print(session, "\tstatus_bit_vector        = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 4, (r));
	bdmf_session_print(session, "\trate_limiter_index       = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 5, (r));
	bdmf_session_print(session, "\tquantum_number           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 6, (r));
	bdmf_session_print(session, "\tdeficit_counter          = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	bdmf_session_print(session, "\tqueue_index              =\n\t");
	for (i=0,j=0; i<8; i++)
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
}

void dump_RDD_PACKET_BUFFER(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	int i,j;

	bdmf_session_print(session, "  Register PACKET_BUFFER\n");

	bdmf_session_print(session, "\tpacket_header            =\n\t");
	for (i=0,j=0; i<160; i++)
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
	bdmf_session_print(session, "\tPARSER_RESULTS_PARSER_SUMMARY_dhcp= 0x%08x", (unsigned int)r);
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
	bdmf_session_print(session, "\tPARSER_RESULTS_PARSER_SUMMARY_mc_l3= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 162, 2, 1, (r));
	bdmf_session_print(session, "\tPARSER_RESULTS_PARSER_SUMMARY_error= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 162, 1, 1, (r));
	bdmf_session_print(session, "\tPARSER_RESULTS_PARSER_SUMMARY_tunnel= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 162, 0, 1, (r));
	bdmf_session_print(session, "\tPARSER_RESULTS_PARSER_SUMMARY__5_tup_valid= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 163, 6, 2, (r));
	bdmf_session_print(session, "\tPARSER_RESULTS_PARSER_SUMMARY_ethernet_version= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 163, 5, 1, (r));
	bdmf_session_print(session, "\tPARSER_RESULTS_PARSER_SUMMARY_dns= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 163, 4, 1, (r));
	bdmf_session_print(session, "\tPARSER_RESULTS_PARSER_SUMMARY_first_ip_fragment= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 163, 0, 4, (r));
	bdmf_session_print(session, "\tPARSER_RESULTS_PARSER_SUMMARY_l2_protocol= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 164, 4, 4, (r)) /*defined by rdd_parser_l4_protocol enumeration*/;
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
	bdmf_session_print(session, "\tPARSER_RESULTS_PARSER_tcp_flags= 0x%08x", (unsigned int)r);
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
	bdmf_session_print(session, "\tPARSER_RESULTS_PARSER_ethenrnet_version= 0x%08x", (unsigned int)r);
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

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 173, 6, 2, (r));
	bdmf_session_print(session, "\tPARSER_RESULTS_PARSER_tag_type= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 173, 3, 3, (r));
	bdmf_session_print(session, "\tPARSER_RESULTS_PARSER_tpid_vlan_0= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 173, 0, 3, (r));
	bdmf_session_print(session, "\tPARSER_RESULTS_PARSER_tpid_vlan_1= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 174, 4, 4, (r));
	bdmf_session_print(session, "\tPARSER_RESULTS_PARSER_da_filter_number= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 174, 0, 4, (r));
	bdmf_session_print(session, "\tPARSER_RESULTS_PARSER_unused= 0x%08x", (unsigned int)r);
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

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 176, 1, 5, (r));
	bdmf_session_print(session, "\tPARSER_RESULTS_PARSER_unused1= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 176, 0, 1, (r));
	bdmf_session_print(session, "\tPARSER_RESULTS_PARSER_ipv6= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 177, (r));
	bdmf_session_print(session, "\tPARSER_RESULTS_PARSER_protocol= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 178, (r));
	bdmf_session_print(session, "\tPARSER_RESULTS_PARSER_fragment_header_offset= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 179, 4, 4, (r));
	bdmf_session_print(session, "\tPARSER_RESULTS_PARSER_unused3= 0x%08x", (unsigned int)r);
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

	MREAD_32((uint8_t *)(p) + 184, (r));
	bdmf_session_print(session, "\tPARSER_RESULTS_PARSER_not_written= 0x%08x", (unsigned int)r);
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

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 202, 4, 1, (r));
	bdmf_session_print(session, "\tPARSER_RESULTS_PARSER_L2_LKP_ENTRY_ctx_ext= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_16((uint8_t *)(p) + 202, 4, 8, (r));
	bdmf_session_print(session, "\tPARSER_RESULTS_PARSER_L2_LKP_ENTRY_client_idx= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 203, 0, 4, (r));
	bdmf_session_print(session, "\tPARSER_RESULTS_PARSER_L2_LKP_ENTRY_var_len_ctx= 0x%08x", (unsigned int)r);
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

	MREAD_32((uint8_t *)(p) + 216, (r));
	bdmf_session_print(session, "\tPARSER_RESULTS_PARSER_L3_LKP_ENTRY_L3_LKP_TOS= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 220, (r));
	bdmf_session_print(session, "\tPARSER_RESULTS_PARSER_L3_LKP_ENTRY_not_written= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 224, (r));
	bdmf_session_print(session, "\tPARSER_RESULTS_PARSER_IC_LKP_ENTRY_da_crc= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 228, (r));
	bdmf_session_print(session, "\tPARSER_RESULTS_PARSER_IC_LKP_ENTRY_sa_crc= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 232, (r));
	bdmf_session_print(session, "\tPARSER_RESULTS_PARSER_IC_LKP_ENTRY_src_ip= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 236, (r));
	bdmf_session_print(session, "\tPARSER_RESULTS_PARSER_IC_LKP_ENTRY_dst_ip= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 240, (r));
	bdmf_session_print(session, "\tPARSER_RESULTS_PARSER_IC_LKP_ENTRY_src_port= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 242, (r));
	bdmf_session_print(session, "\tPARSER_RESULTS_PARSER_IC_LKP_ENTRY_dst_port= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 244, (r));
	bdmf_session_print(session, "\tPARSER_RESULTS_PARSER_IC_LKP_ENTRY_ethernet_type= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 246, (r));
	bdmf_session_print(session, "\tPARSER_RESULTS_PARSER_IC_LKP_ENTRY_protocol= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 247, 5, 3, (r));
	bdmf_session_print(session, "\tPARSER_RESULTS_PARSER_IC_LKP_ENTRY_tpid_vlan_0= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 247, 2, 3, (r));
	bdmf_session_print(session, "\tPARSER_RESULTS_PARSER_IC_LKP_ENTRY_tpid_vlan_1= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 247, 1, 1, (r));
	bdmf_session_print(session, "\tPARSER_RESULTS_PARSER_IC_LKP_ENTRY_ipv6= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 247, 0, 1, (r));
	bdmf_session_print(session, "\tPARSER_RESULTS_PARSER_IC_LKP_ENTRY_ipv4= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 248, 5, 3, (r));
	bdmf_session_print(session, "\tPARSER_RESULTS_PARSER_IC_LKP_ENTRY_outer_pbit= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 248, 4, 1, (r));
	bdmf_session_print(session, "\tPARSER_RESULTS_PARSER_IC_LKP_ENTRY_outer_cfi= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_16((uint8_t *)(p) + 248, 0, 12, (r));
	bdmf_session_print(session, "\tPARSER_RESULTS_PARSER_IC_LKP_ENTRY_outer_vid= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 250, 5, 3, (r));
	bdmf_session_print(session, "\tPARSER_RESULTS_PARSER_IC_LKP_ENTRY_inner_pbit= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 250, 4, 1, (r));
	bdmf_session_print(session, "\tPARSER_RESULTS_PARSER_IC_LKP_ENTRY_inner_cfi= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_16((uint8_t *)(p) + 250, 0, 12, (r));
	bdmf_session_print(session, "\tPARSER_RESULTS_PARSER_IC_LKP_ENTRY_inner_vid= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 252, (r));
	bdmf_session_print(session, "\tPARSER_RESULTS_PARSER_IC_LKP_ENTRY_tos= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 253, 3, 5, (r));
	bdmf_session_print(session, "\tPARSER_RESULTS_PARSER_IC_LKP_ENTRY_lookup_port= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 253, 0, 3, (r));
	bdmf_session_print(session, "\tPARSER_RESULTS_PARSER_IC_LKP_ENTRY_vlans_num= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 254, 7, 1, (r));
	bdmf_session_print(session, "\tPARSER_RESULTS_PARSER_IC_LKP_ENTRY_mc_l3= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 254, 6, 1, (r));
	bdmf_session_print(session, "\tPARSER_RESULTS_PARSER_IC_LKP_ENTRY_bc= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 254, 5, 1, (r));
	bdmf_session_print(session, "\tPARSER_RESULTS_PARSER_IC_LKP_ENTRY_mc= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 254, 4, 1, (r));
	bdmf_session_print(session, "\tPARSER_RESULTS_PARSER_IC_LKP_ENTRY_network_layer= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 254, 0, 4, (r));
	bdmf_session_print(session, "\tPARSER_RESULTS_PARSER_IC_LKP_ENTRY_reserved3= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 255, (r));
	bdmf_session_print(session, "\tPARSER_RESULTS_PARSER_IC_LKP_ENTRY_gem_ssid= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 256, (r));
	bdmf_session_print(session, "\tPROCESSING_RX_DESCRIPTOR_pd_info= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_16((uint8_t *)(p) + 260, 6, 10, (r));
	bdmf_session_print(session, "\tPROCESSING_RX_DESCRIPTOR_serial_num= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 261, 5, 1, (r));
	bdmf_session_print(session, "\tPROCESSING_RX_DESCRIPTOR_ploam= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 261, 4, 1, (r));
	bdmf_session_print(session, "\tPROCESSING_RX_DESCRIPTOR_ingress_cong= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 261, 3, 1, (r));
	bdmf_session_print(session, "\tPROCESSING_RX_DESCRIPTOR_abs_or_dsl= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 261, 2, 1, (r));
	bdmf_session_print(session, "\tPROCESSING_RX_DESCRIPTOR_l3_packet= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_32((uint8_t *)(p) + 260, 14, 4, (r));
	bdmf_session_print(session, "\tPROCESSING_RX_DESCRIPTOR_error_type_or_cpu_tx= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_16((uint8_t *)(p) + 262, 0, 14, (r));
	bdmf_session_print(session, "\tPROCESSING_RX_DESCRIPTOR_packet_length= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 264, 7, 1, (r));
	bdmf_session_print(session, "\tPROCESSING_RX_DESCRIPTOR_error= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 264, 6, 1, (r));
	bdmf_session_print(session, "\tPROCESSING_RX_DESCRIPTOR_target_mem_1= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 264, 4, 2, (r));
	bdmf_session_print(session, "\tPROCESSING_RX_DESCRIPTOR_cong_state= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 264, 3, 1, (r));
	bdmf_session_print(session, "\tPROCESSING_RX_DESCRIPTOR_is_emac= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_16((uint8_t *)(p) + 264, 3, 8, (r));
	bdmf_session_print(session, "\tPROCESSING_RX_DESCRIPTOR_ingress_vport_or_flow= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_32((uint8_t *)(p) + 264, 0, 19, (r));
	bdmf_session_print(session, "\tPROCESSING_RX_DESCRIPTOR_bn1_last_or_abs1= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 268, 7, 1, (r));
	bdmf_session_print(session, "\tPROCESSING_RX_DESCRIPTOR_agg_pd= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 268, 6, 1, (r));
	bdmf_session_print(session, "\tPROCESSING_RX_DESCRIPTOR_target_mem_0= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_32((uint8_t *)(p) + 268, 0, 30, (r));
	bdmf_session_print(session, "\tPROCESSING_RX_DESCRIPTOR_payload_offset_sop= 0x%08x", (unsigned int)r);
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
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_ENTRY_NATC_CONTROL_ENTRY_has_iter= 0x%08x", (unsigned int)r);
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
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_ENTRY_drop= 0x%08x", (unsigned int)r);
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

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 280, 7, 1, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_ENTRY_is_unicast_wfd_nic= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 280, 6, 1, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_ENTRY_is_unicast_wfd_any= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 280, 2, 4, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_ENTRY_priority= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 280, 1, 1, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_ENTRY_wfd_prio= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_16((uint8_t *)(p) + 280, 7, 2, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_ENTRY_wfd_idx= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 281, 6, 1, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_ENTRY_is_wred_high_prio= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 281, 4, 2, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_ENTRY_reserved_1= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 281, 3, 1, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_ENTRY_is_ingqos_high_prio= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 281, 2, 1, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_ENTRY_coupled_classic_queue= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_32((uint8_t *)(p) + 280, 0, 18, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_ENTRY_link_specific_union= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 284, 5, 3, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_ENTRY_ip_addresses_table_index= 0x%08x", (unsigned int)r);
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
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_ENTRY_reserved_3= 0x%08x", (unsigned int)r);
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

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 292, 7, 1, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_ENTRY_reserved_4= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 292, 2, 5, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_ENTRY_tunnel_index_ref= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 292, 0, 2, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_ENTRY_reserved_5= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 293, 6, 2, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_ENTRY_reserved_2= 0x%08x", (unsigned int)r);
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

	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_TUNNEL_ENTRY_tunnel_key=\n\t");
	for (i=0,j=0; i<4; i++)
	{
		MREAD_I_32((uint8_t *)(p) + 296, i, (r));
		bdmf_session_print(session, "0x%08x ", (unsigned int)r);
		j++;
	}
	bdmf_session_print(session, "\n");
	MREAD_32((uint8_t *)(p) + 312, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_TUNNEL_ENTRY_key_0_mask= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 316, 4, 4, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_TUNNEL_ENTRY_reserved6= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 316, 0, 4, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_TUNNEL_ENTRY_tunnel_type= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 317, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_TUNNEL_ENTRY_inner_packet_offset= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 318, 6, 2, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_TUNNEL_ENTRY_reserved7= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 318, 5, 1, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_TUNNEL_ENTRY_esp_o_udp= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 318, 4, 1, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_TUNNEL_ENTRY_l2_l3_indication= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 318, 0, 4, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_TUNNEL_ENTRY_reserved8= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 319, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_TUNNEL_ENTRY_key_0_offset= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_TUNNEL_ENTRY_reserved9=\n\t");
	for (i=0,j=0; i<76; i++)
	{
		MREAD_I_8((uint8_t *)(p) + 320, i, (r));
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
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_FPI_ENTRY_reserved0= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 276, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_FPI_ENTRY_reserved1= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 280, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_FPI_ENTRY_reserved2= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 284, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_FPI_ENTRY_reserved3= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 288, 5, 3, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_FPI_ENTRY_reserved4= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 288, 4, 1, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_FPI_ENTRY_vtag_check= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_16((uint8_t *)(p) + 288, 0, 12, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_FPI_ENTRY_vtag= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 290, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_FPI_ENTRY_reserved5= 0x%08x", (unsigned int)r);
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
	RDP_FIELD_MREAD_8((uint8_t *)(p) + 400, 6, 2, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_TCAM_RESULT_TCAM_CONTEXT_action= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 400, 3, 3, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_TCAM_RESULT_TCAM_CONTEXT_trap_reason= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_16((uint8_t *)(p) + 400, 0, 11, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_TCAM_RESULT_TCAM_CONTEXT_cntr_id= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 402, 7, 1, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_TCAM_RESULT_TCAM_CONTEXT_cntr_disable= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_16((uint8_t *)(p) + 402, 4, 11, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_TCAM_RESULT_TCAM_CONTEXT_bytes_cntr_id= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 403, 3, 1, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_TCAM_RESULT_TCAM_CONTEXT_bytes_cntr_disable= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 403, 2, 1, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_TCAM_RESULT_TCAM_CONTEXT_iq_priority= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 403, 1, 1, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_TCAM_RESULT_TCAM_CONTEXT_ct_override= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 403, 0, 1, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_TCAM_RESULT_TCAM_CONTEXT_reserved1= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_TCAM_RESULT_TCAM_CONTEXT_reserved2=\n\t");
	for (i=0,j=0; i<12; i++)
	{
		MREAD_I_8((uint8_t *)(p) + 404, i, (r));
		bdmf_session_print(session, "0x%08x ", (unsigned int)r);
		j++;
		if (j >= 8)
		{
			j = 0;
			bdmf_session_print(session, "\n\t");
		}
	}
	bdmf_session_print(session, "\n");
	RDP_FIELD_MREAD_8((uint8_t *)(p) + 416, 7, 1, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_TCAM_RESULT_TCAM_MATCH_match= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_16((uint8_t *)(p) + 416, 5, 10, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_TCAM_RESULT_TCAM_MATCH_match_index= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 417, 0, 5, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_TCAM_RESULT_TCAM_MATCH_reserved= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 418, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_TCAM_RESULT_TCAM_MATCH_reserved1= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 504, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_SBPM_GET_NEXT_REPLY_reserved= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 508, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_SBPM_GET_NEXT_REPLY_reserved2= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 509, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_SBPM_GET_NEXT_REPLY_mcast_of_next= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 510, 7, 1, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_SBPM_GET_NEXT_REPLY_next_is_null= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_16((uint8_t *)(p) + 510, 1, 14, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_SBPM_GET_NEXT_REPLY_next_bn= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 511, 0, 1, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_SBPM_GET_NEXT_REPLY_valid= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 504, 7, 1, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_BBMSG_BUFMNG_REPLY_result= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 504, 6, 1, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_BBMSG_BUFMNG_REPLY_valid0= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_32((uint8_t *)(p) + 504, 12, 18, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_BBMSG_BUFMNG_REPLY_counter0= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 506, 3, 1, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_BBMSG_BUFMNG_REPLY_valid1= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_16((uint8_t *)(p) + 506, 0, 11, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_BBMSG_BUFMNG_REPLY_counter1_1= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 508, 1, 7, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_BBMSG_BUFMNG_REPLY_counter1_0= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 508, 0, 1, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_BBMSG_BUFMNG_REPLY_valid2= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_32((uint8_t *)(p) + 508, 6, 18, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_BBMSG_BUFMNG_REPLY_counter2= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 511, 0, 6, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_BBMSG_BUFMNG_REPLY_reserved= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 504, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_SBPM_ALLOC_REPLY_reserved= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_32((uint8_t *)(p) + 508, 15, 17, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_SBPM_ALLOC_REPLY_reserved2= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_16((uint8_t *)(p) + 510, 1, 14, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_SBPM_ALLOC_REPLY_alloc_bn= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 511, 0, 1, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_SBPM_ALLOC_REPLY_bn_valid= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 504, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_POLICER_RESULT_ENTRY_color= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_32((uint8_t *)(p) + 504, 0, 24, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_POLICER_RESULT_ENTRY_reserved0= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 508, (r));
	bdmf_session_print(session, "\tSCRATCH_UNICAST_FLOW_POLICER_RESULT_ENTRY_reserved1= 0x%08x", (unsigned int)r);
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
	bdmf_session_print(session, "\tSCRATCH_DS_TUNNEL_FLOW_TUNNEL_SCRATCH_sbpm_alloc_reply=\n\t");
	for (i=0,j=0; i<8; i++)
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
	MREAD_32((uint8_t *)(p) + 280, (r));
	bdmf_session_print(session, "\tSCRATCH_DS_TUNNEL_FLOW_TUNNEL_SCRATCH_ip_crc_result= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 284, (r));
	bdmf_session_print(session, "\tSCRATCH_DS_TUNNEL_FLOW_TUNNEL_SCRATCH_reserved= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 288, (r));
	bdmf_session_print(session, "\tSCRATCH_DS_TUNNEL_FLOW_TUNNEL_SCRATCH_cam_lkp_result= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 292, (r));
	bdmf_session_print(session, "\tSCRATCH_DS_TUNNEL_FLOW_SBPM_GET_NEXT_REPLY_reserved= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 296, (r));
	bdmf_session_print(session, "\tSCRATCH_DS_TUNNEL_FLOW_SBPM_GET_NEXT_REPLY_reserved2= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 297, (r));
	bdmf_session_print(session, "\tSCRATCH_DS_TUNNEL_FLOW_SBPM_GET_NEXT_REPLY_mcast_of_next= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 298, 7, 1, (r));
	bdmf_session_print(session, "\tSCRATCH_DS_TUNNEL_FLOW_SBPM_GET_NEXT_REPLY_next_is_null= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_16((uint8_t *)(p) + 298, 1, 14, (r));
	bdmf_session_print(session, "\tSCRATCH_DS_TUNNEL_FLOW_SBPM_GET_NEXT_REPLY_next_bn= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 299, 0, 1, (r));
	bdmf_session_print(session, "\tSCRATCH_DS_TUNNEL_FLOW_SBPM_GET_NEXT_REPLY_valid= 0x%08x", (unsigned int)r);
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
	bdmf_session_print(session, "\tSCRATCH_RX_MIRROR_FLOW_SBPM_GET_NEXT_REPLY_reserved= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 404, (r));
	bdmf_session_print(session, "\tSCRATCH_RX_MIRROR_FLOW_SBPM_GET_NEXT_REPLY_reserved2= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 405, (r));
	bdmf_session_print(session, "\tSCRATCH_RX_MIRROR_FLOW_SBPM_GET_NEXT_REPLY_mcast_of_next= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 406, 7, 1, (r));
	bdmf_session_print(session, "\tSCRATCH_RX_MIRROR_FLOW_SBPM_GET_NEXT_REPLY_next_is_null= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_16((uint8_t *)(p) + 406, 1, 14, (r));
	bdmf_session_print(session, "\tSCRATCH_RX_MIRROR_FLOW_SBPM_GET_NEXT_REPLY_next_bn= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 407, 0, 1, (r));
	bdmf_session_print(session, "\tSCRATCH_RX_MIRROR_FLOW_SBPM_GET_NEXT_REPLY_valid= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 400, (r));
	bdmf_session_print(session, "\tSCRATCH_RX_MIRROR_FLOW_SBPM_ALLOC_REPLY_reserved= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_32((uint8_t *)(p) + 404, 15, 17, (r));
	bdmf_session_print(session, "\tSCRATCH_RX_MIRROR_FLOW_SBPM_ALLOC_REPLY_reserved2= 0x%08x", (unsigned int)r);
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
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_ENTRY_NATC_CONTROL_ENTRY_has_iter= 0x%08x", (unsigned int)r);
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
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_ENTRY_drop= 0x%08x", (unsigned int)r);
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

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 280, 7, 1, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_ENTRY_is_unicast_wfd_nic= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 280, 6, 1, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_ENTRY_is_unicast_wfd_any= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 280, 2, 4, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_ENTRY_priority= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 280, 1, 1, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_ENTRY_wfd_prio= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_16((uint8_t *)(p) + 280, 7, 2, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_ENTRY_wfd_idx= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 281, 6, 1, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_ENTRY_is_wred_high_prio= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 281, 4, 2, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_ENTRY_reserved_1= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 281, 3, 1, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_ENTRY_is_ingqos_high_prio= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 281, 2, 1, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_ENTRY_coupled_classic_queue= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_32((uint8_t *)(p) + 280, 0, 18, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_ENTRY_link_specific_union= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 284, 5, 3, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_ENTRY_ip_addresses_table_index= 0x%08x", (unsigned int)r);
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
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_ENTRY_reserved_3= 0x%08x", (unsigned int)r);
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

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 292, 7, 1, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_ENTRY_reserved_4= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 292, 2, 5, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_ENTRY_tunnel_index_ref= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 292, 0, 2, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_ENTRY_reserved_5= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 293, 6, 2, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_ENTRY_reserved_2= 0x%08x", (unsigned int)r);
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

	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_TUNNEL_ENTRY_tunnel_key=\n\t");
	for (i=0,j=0; i<4; i++)
	{
		MREAD_I_32((uint8_t *)(p) + 296, i, (r));
		bdmf_session_print(session, "0x%08x ", (unsigned int)r);
		j++;
	}
	bdmf_session_print(session, "\n");
	MREAD_32((uint8_t *)(p) + 312, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_TUNNEL_ENTRY_key_0_mask= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 316, 4, 4, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_TUNNEL_ENTRY_reserved6= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 316, 0, 4, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_TUNNEL_ENTRY_tunnel_type= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 317, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_TUNNEL_ENTRY_inner_packet_offset= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 318, 6, 2, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_TUNNEL_ENTRY_reserved7= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 318, 5, 1, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_TUNNEL_ENTRY_esp_o_udp= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 318, 4, 1, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_TUNNEL_ENTRY_l2_l3_indication= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 318, 0, 4, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_TUNNEL_ENTRY_reserved8= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 319, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_TUNNEL_ENTRY_key_0_offset= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_TUNNEL_ENTRY_reserved9=\n\t");
	for (i=0,j=0; i<76; i++)
	{
		MREAD_I_8((uint8_t *)(p) + 320, i, (r));
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
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_FPI_ENTRY_reserved0= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 276, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_FPI_ENTRY_reserved1= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 280, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_FPI_ENTRY_reserved2= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 284, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_FPI_ENTRY_reserved3= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 288, 5, 3, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_FPI_ENTRY_reserved4= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 288, 4, 1, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_FPI_ENTRY_vtag_check= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_16((uint8_t *)(p) + 288, 0, 12, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_FPI_ENTRY_vtag= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 290, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_NATC_RESULT_FC_UCAST_FLOW_CONTEXT_FPI_ENTRY_reserved5= 0x%08x", (unsigned int)r);
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
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_SBPM_GET_NEXT_REPLY_reserved= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 508, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_SBPM_GET_NEXT_REPLY_reserved2= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 509, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_SBPM_GET_NEXT_REPLY_mcast_of_next= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 510, 7, 1, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_SBPM_GET_NEXT_REPLY_next_is_null= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_16((uint8_t *)(p) + 510, 1, 14, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_SBPM_GET_NEXT_REPLY_next_bn= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 511, 0, 1, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_SBPM_GET_NEXT_REPLY_valid= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 504, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_SBPM_ALLOC_REPLY_reserved= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_32((uint8_t *)(p) + 508, 15, 17, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_SBPM_ALLOC_REPLY_reserved2= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_16((uint8_t *)(p) + 510, 1, 14, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_SBPM_ALLOC_REPLY_alloc_bn= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 511, 0, 1, (r));
	bdmf_session_print(session, "\tSCRATCH_MULTICAST_FLOW_SBPM_ALLOC_REPLY_bn_valid= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_BASIC_RATE_LIMITER_DESCRIPTOR(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register BASIC_RATE_LIMITER_DESCRIPTOR\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p), 7, 1, (r));
	bdmf_session_print(session, "\trl_type                  = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_32((uint8_t *)(p), 10, 21, (r));
	bdmf_session_print(session, "\treserved2                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 2, 0, 2, (r));
	bdmf_session_print(session, "\tblock_type               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 3, (r));
	bdmf_session_print(session, "\tblock_index              = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 4, (r));
	bdmf_session_print(session, "\tcurrent_budget           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_16((uint8_t *)(p) + 8, 2, 14, (r));
	bdmf_session_print(session, "\talloc_mantissa           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 9, 0, 2, (r));
	bdmf_session_print(session, "\talloc_exponent           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_16((uint8_t *)(p) + 10, 2, 14, (r));
	bdmf_session_print(session, "\tlimit_mantissa           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 11, 0, 2, (r));
	bdmf_session_print(session, "\tlimit_exponent           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_32((uint8_t *)(p) + 12, 7, 25, (r));
	bdmf_session_print(session, "\treserved                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 15, 0, 7, (r));
	bdmf_session_print(session, "\talloc_residue            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

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

void dump_RDD_BYTE_1(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register BYTE_1\n");

	MREAD_8((uint8_t *)(p), (r));
	bdmf_session_print(session, "\tbits                     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

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

	MREAD_16((uint8_t *)(p) + 2, (r));
	bdmf_session_print(session, "\treserved1                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

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
	bdmf_session_print(session, "\tnatc_tbl_id              = 0x%08x", (unsigned int)r);
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

void dump_RDD_BYTES_4(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register BYTES_4\n");

	MREAD_32((uint8_t *)(p), (r));
	bdmf_session_print(session, "\tbits                     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_TM_FLOW_CNTR_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register TM_FLOW_CNTR_ENTRY\n");

	MREAD_8((uint8_t *)(p), (r));
	bdmf_session_print(session, "\tcntr_id                  = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_BUFFER_CONG_MGT(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register BUFFER_CONG_MGT\n");

	MREAD_32((uint8_t *)(p), (r));
	bdmf_session_print(session, "\tdqm_not_empty_address    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 4, (r));
	bdmf_session_print(session, "\tdqm_valid_ctr_address    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 8, (r));
	bdmf_session_print(session, "\tug_counter_address       = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 12, (r));
	bdmf_session_print(session, "\tug_threshold_high        = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 16, (r));
	bdmf_session_print(session, "\tug_threshold_low         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 20, (r));
	bdmf_session_print(session, "\tenabled                  = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 21, (r));
	bdmf_session_print(session, "\tstart_queue              = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 22, (r));
	bdmf_session_print(session, "\tend_queue                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 23, (r));
	bdmf_session_print(session, "\tfw_state                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 24, (r));
	bdmf_session_print(session, "\ttimer_duration           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 26, (r));
	bdmf_session_print(session, "\treserved1                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 28, (r));
	bdmf_session_print(session, "\tsq_start_queue           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 29, (r));
	bdmf_session_print(session, "\tnum_pds_to_flush         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 30, (r));
	bdmf_session_print(session, "\tflush_wakeup             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 32, (r));
	bdmf_session_print(session, "\tflush_cfg_address        = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 34, (r));
	bdmf_session_print(session, "\tflush_enable_address     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 36, (r));
	bdmf_session_print(session, "\tsq_flush_wakeup          = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 40, (r));
	bdmf_session_print(session, "\tsq_flush_wakeup_value    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 44, (r));
	bdmf_session_print(session, "\tsq_flush_cfg_address     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 48, (r));
	bdmf_session_print(session, "\tsq_flush_enable_address  = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 52, (r));
	bdmf_session_print(session, "\tcong_queue_idx           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 54, (r));
	bdmf_session_print(session, "\tcong_queue_occupancy     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 56, (r));
	bdmf_session_print(session, "\tcong_detected            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 60, (r));
	bdmf_session_print(session, "\tcong_cleared             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 64, (r));
	bdmf_session_print(session, "\tcong_detection_time      = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_FLUSH_CFG_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register FLUSH_CFG_ENTRY\n");

	MREAD_8((uint8_t *)(p), (r));
	bdmf_session_print(session, "\tpd_num                   = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 1, (r));
	bdmf_session_print(session, "\tflush_aggr               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 2, 7, 1, (r));
	bdmf_session_print(session, "\tenable                   = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 2, 6, 1, (r));
	bdmf_session_print(session, "\tdrain_q                  = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 2, 5, 1, (r));
	bdmf_session_print(session, "\thw_flush_en              = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 2, 4, 1, (r));
	bdmf_session_print(session, "\tbuffer_cong_mgt          = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 2, 1, 3, (r));
	bdmf_session_print(session, "\treserved                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_16((uint8_t *)(p) + 2, 0, 9, (r));
	bdmf_session_print(session, "\tqm_queue                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 4, (r));
	bdmf_session_print(session, "\tug_counter_address       = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 8, (r));
	bdmf_session_print(session, "\tug_threshold_low         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 12, (r));
	bdmf_session_print(session, "\tug_counter               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

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

void dump_RDD_AQM_QUEUE_DESCRIPTOR(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register AQM_QUEUE_DESCRIPTOR\n");

	MREAD_16((uint8_t *)(p), (r));
	bdmf_session_print(session, "\tCODEL_QUEUE_DESCRIPTOR_window_ts= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 2, (r));
	bdmf_session_print(session, "\tCODEL_QUEUE_DESCRIPTOR_drop_interval= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p), (r));
	bdmf_session_print(session, "\tPI2_QUEUE_DESCRIPTOR_probability= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 1, (r));
	bdmf_session_print(session, "\tPI2_QUEUE_DESCRIPTOR_packet_select_probability= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 2, (r));
	bdmf_session_print(session, "\tPI2_QUEUE_DESCRIPTOR_q_delay= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 3, (r));
	bdmf_session_print(session, "\tPI2_QUEUE_DESCRIPTOR_prev_q_delay= 0x%08x", (unsigned int)r);
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
		MREAD_I_32((uint8_t *)(p), i, (r));
		bdmf_session_print(session, "0x%08x ", (unsigned int)r);
		j++;
	}
	bdmf_session_print(session, "\n");
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
	bdmf_session_print(session, "\tcongestion_ctrl          = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 4, 3, 1, (r));
	bdmf_session_print(session, "\tcpu_tx_mcore_off         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 4, 2, 1, (r));
	bdmf_session_print(session, "\ttcp_pure_ack_flows       = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 4, 1, 1, (r));
	bdmf_session_print(session, "\tct_lookup_enabled        = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 4, 0, 1, (r));
	bdmf_session_print(session, "\tct_expect_lookup_enabled = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 5, 7, 1, (r));
	bdmf_session_print(session, "\tpbit_key_mode            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_32((uint8_t *)(p) + 4, 0, 23, (r));
	bdmf_session_print(session, "\treserved1                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

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

void dump_RDD_AQM_SQ_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register AQM_SQ_ENTRY\n");

	MREAD_16((uint8_t *)(p), (r));
	bdmf_session_print(session, "\tts                       = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_COMPLEX_SCHEDULER_DESCRIPTOR(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	int i,j;

	bdmf_session_print(session, "  Register COMPLEX_SCHEDULER_DESCRIPTOR\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p), 7, 1, (r));
	bdmf_session_print(session, "\treserved0                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p), 6, 1, (r));
	bdmf_session_print(session, "\tis_positive_budget       = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p), 0, 6, (r));
	bdmf_session_print(session, "\tbbh_queue                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 1, 7, 1, (r));
	bdmf_session_print(session, "\treserved1                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 1, 0, 7, (r));
	bdmf_session_print(session, "\trate_limiter_index       = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 2, 5, 3, (r));
	bdmf_session_print(session, "\tdwrr_offset_pir          = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 2, 2, 3, (r));
	bdmf_session_print(session, "\tdwrr_offset_sir          = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_16((uint8_t *)(p) + 2, 5, 5, (r));
	bdmf_session_print(session, "\tlast_served_block_pir    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 3, 0, 5, (r));
	bdmf_session_print(session, "\tlast_served_block_sir    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 4, (r));
	bdmf_session_print(session, "\tstatus_bit_vector        = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 8, (r));
	bdmf_session_print(session, "\tslot_budget_bit_vector_0 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 12, (r));
	bdmf_session_print(session, "\tslot_budget_bit_vector_1 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 16, (r));
	bdmf_session_print(session, "\tis_scheduler_slot        = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 20, (r));
	bdmf_session_print(session, "\tis_scheduler_basic       = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 24, 7, 1, (r));
	bdmf_session_print(session, "\tovl_rl_en                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 24, 6, 1, (r));
	bdmf_session_print(session, "\trate_limit_enable        = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 24, 5, 1, (r));
	bdmf_session_print(session, "\tparent_scheduler_exists  = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 24, 1, 4, (r));
	bdmf_session_print(session, "\tparent_scheduler_index   = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_16((uint8_t *)(p) + 24, 4, 5, (r));
	bdmf_session_print(session, "\tparent_scheduler_slot_index= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 25, 0, 4, (r));
	bdmf_session_print(session, "\treserved2                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 26, (r));
	bdmf_session_print(session, "\tdeficit_counter          = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 28, (r));
	bdmf_session_print(session, "\tquantum_number           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	bdmf_session_print(session, "\treserved3                =\n\t");
	for (i=0,j=0; i<3; i++)
	{
		MREAD_I_8((uint8_t *)(p) + 29, i, (r));
		bdmf_session_print(session, "0x%08x ", (unsigned int)r);
		j++;
	}
	bdmf_session_print(session, "\n");
	bdmf_session_print(session, "\tblock_index              =\n\t");
	for (i=0,j=0; i<32; i++)
	{
		MREAD_I_8((uint8_t *)(p) + 32, i, (r));
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

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 1, 2, 5, (r));
	bdmf_session_print(session, "\tvirtual_port             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 1, 0, 2, (r));
	bdmf_session_print(session, "\treserved                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

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

void dump_RDD_BUFFER_CONG_Q_OCCUPANCY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register BUFFER_CONG_Q_OCCUPANCY\n");

	MREAD_32((uint8_t *)(p), (r));
	bdmf_session_print(session, "\tpacket_cnt               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 4, (r));
	bdmf_session_print(session, "\tbyte_cnt                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

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

void dump_RDD_BYTES_2(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register BYTES_2\n");

	MREAD_16((uint8_t *)(p), (r));
	bdmf_session_print(session, "\tbits                     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

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

void dump_RDD_NATC_TBL_CONFIGURATION(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	int i,j;

	bdmf_session_print(session, "  Register NATC_TBL_CONFIGURATION\n");

	MREAD_32((uint8_t *)(p), (r));
	bdmf_session_print(session, "\tkey_addr_low             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 4, (r));
	bdmf_session_print(session, "\tkey_addr_high            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 8, (r));
	bdmf_session_print(session, "\tres_addr_low             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 12, (r));
	bdmf_session_print(session, "\tres_addr_high            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 16, 1, 7, (r));
	bdmf_session_print(session, "\treserved                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 16, 0, 1, (r));
	bdmf_session_print(session, "\tmiss_cache_enable        = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 17, (r));
	bdmf_session_print(session, "\tkey_size                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 18, (r));
	bdmf_session_print(session, "\tcontext_size             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 19, (r));
	bdmf_session_print(session, "\treserved0                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	bdmf_session_print(session, "\treserved1                =\n\t");
	for (i=0,j=0; i<4; i++)
	{
		MREAD_I_8((uint8_t *)(p) + 20, i, (r));
		bdmf_session_print(session, "0x%08x ", (unsigned int)r);
		j++;
	}
	bdmf_session_print(session, "\n");
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

void dump_RDD_PI2_PROBABILITY_CALC_DESCRIPTOR(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	int i,j;

	bdmf_session_print(session, "  Register PI2_PROBABILITY_CALC_DESCRIPTOR\n");

	bdmf_session_print(session, "\tpi2_queue_mask           =\n\t");
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
	MREAD_16((uint8_t *)(p) + 32, (r));
	bdmf_session_print(session, "\ttimer_value              = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 34, (r));
	bdmf_session_print(session, "\tcalc_iter                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 35, (r));
	bdmf_session_print(session, "\ttarget_delay             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 36, (r));
	bdmf_session_print(session, "\talpha_coeff              = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 38, (r));
	bdmf_session_print(session, "\tbeta_coeff               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 40, (r));
	bdmf_session_print(session, "\tticks_per_run            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 41, (r));
	bdmf_session_print(session, "\tticks_left               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 42, (r));
	bdmf_session_print(session, "\tds_start_iter            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 43, (r));
	bdmf_session_print(session, "\tsr_start_iter            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 44, (r));
	bdmf_session_print(session, "\tus_aqm_q_table           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 46, (r));
	bdmf_session_print(session, "\tds_aqm_q_table           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 48, (r));
	bdmf_session_print(session, "\tsr_aqm_q_table           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 50, (r));
	bdmf_session_print(session, "\treserved1                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_CODEL_BIAS_SLOPE(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register CODEL_BIAS_SLOPE\n");

	MREAD_16((uint8_t *)(p), (r));
	bdmf_session_print(session, "\tbias                     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 2, (r));
	bdmf_session_print(session, "\tslope                    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_FPI_CFG(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register FPI_CFG\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p), 5, 3, (r));
	bdmf_session_print(session, "\tmode                     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p), 2, 3, (r));
	bdmf_session_print(session, "\tlookup_mode              = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_16((uint8_t *)(p), 6, 4, (r));
	bdmf_session_print(session, "\treserved                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 1, 0, 6, (r));
	bdmf_session_print(session, "\tdefault_priority         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

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

void dump_RDD_FLUSH_CFG_ENABLE_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register FLUSH_CFG_ENABLE_ENTRY\n");

	MREAD_8((uint8_t *)(p), (r));
	bdmf_session_print(session, "\tenable_fw                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 1, (r));
	bdmf_session_print(session, "\tenable_cpu               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

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

void dump_RDD_BBH_QUEUE_DESCRIPTOR(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register BBH_QUEUE_DESCRIPTOR\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p), 2, 6, (r));
	bdmf_session_print(session, "\twan_type_union           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p), 1, 1, (r));
	bdmf_session_print(session, "\tmirroring_en             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p), 0, 1, (r));
	bdmf_session_print(session, "\tpriority                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 1, 1, 7, (r));
	bdmf_session_print(session, "\tscheduler_index          = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 1, 0, 1, (r));
	bdmf_session_print(session, "\tscheduler_type           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 2, (r));
	bdmf_session_print(session, "\tbb_destination           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 3, (r));
	bdmf_session_print(session, "\tingress_counter          = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

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
}

void dump_RDD_VPORT_TO_RL_OVERHEAD_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register VPORT_TO_RL_OVERHEAD_ENTRY\n");

	MREAD_8((uint8_t *)(p), (r));
	bdmf_session_print(session, "\trl_overhead              = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

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

void dump_RDD_CPU_REDIRECT_MODE_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register CPU_REDIRECT_MODE_ENTRY\n");

	MREAD_8((uint8_t *)(p), (r));
	bdmf_session_print(session, "\tmode                     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_CODEL_DROP_DESCRIPTOR(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register CODEL_DROP_DESCRIPTOR\n");

	MREAD_16((uint8_t *)(p), (r));
	bdmf_session_print(session, "\tmax_seq_drops            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 2, (r));
	bdmf_session_print(session, "\tflush_task_wakeup_value  = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 4, (r));
	bdmf_session_print(session, "\tflush_cfg_ptr            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 6, (r));
	bdmf_session_print(session, "\tflush_enable_ptr         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 8, (r));
	bdmf_session_print(session, "\tflush_packet_counter     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 12, (r));
	bdmf_session_print(session, "\tflush_enable             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 16, (r));
	bdmf_session_print(session, "\tscratchpad_1             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 18, (r));
	bdmf_session_print(session, "\tscratchpad_2             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 19, (r));
	bdmf_session_print(session, "\treserved1                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_MIRRORING_DESCRIPTOR(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register MIRRORING_DESCRIPTOR\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p), 7, 1, (r));
	bdmf_session_print(session, "\tmirror_en                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p), 1, 6, (r));
	bdmf_session_print(session, "\tdst_vport                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_16((uint8_t *)(p), 0, 9, (r));
	bdmf_session_print(session, "\tdst_queue                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_TCAM_TABLE_CFG(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register TCAM_TABLE_CFG\n");

	MREAD_8((uint8_t *)(p), (r));
	bdmf_session_print(session, "\tall_fields               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

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

void dump_RDD_HW_IPTV_CONFIGURATION(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register HW_IPTV_CONFIGURATION\n");

	MREAD_16((uint8_t *)(p), (r));
	bdmf_session_print(session, "\tfpm_base_token_size      = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 2, 7, 1, (r));
	bdmf_session_print(session, "\treserved0                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 2, 0, 7, (r));
	bdmf_session_print(session, "\thn_size1                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 3, 7, 1, (r));
	bdmf_session_print(session, "\treserved1                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 3, 0, 7, (r));
	bdmf_session_print(session, "\thn_size0                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 4, 3, 5, (r));
	bdmf_session_print(session, "\treserved2                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_16((uint8_t *)(p) + 4, 0, 11, (r));
	bdmf_session_print(session, "\tddr_sop_offset0          = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 6, 3, 5, (r));
	bdmf_session_print(session, "\treserved3                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_16((uint8_t *)(p) + 6, 0, 11, (r));
	bdmf_session_print(session, "\tddr_sop_offset1          = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 8, (r));
	bdmf_session_print(session, "\tfpm_pool1_stat2_addr     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

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

void dump_RDD_SPDSVC_BBH_TX_PARAMS(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register SPDSVC_BBH_TX_PARAMS\n");

	MREAD_32((uint8_t *)(p), (r));
	bdmf_session_print(session, "\tprocessing_tx_pd_3       = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 4, (r));
	bdmf_session_print(session, "\tspdsvc_sent              = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 8, (r));
	bdmf_session_print(session, "\tspdsvc_bbh_id            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

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

void dump_RDD_INGRESS_FILTER_CFG(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register INGRESS_FILTER_CFG\n");

	MREAD_16((uint8_t *)(p), (r));
	bdmf_session_print(session, "\tres_offset               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

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

void dump_RDD_FC_FLOW_IP_ADDRESSES_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	int i,j;

	bdmf_session_print(session, "  Register FC_FLOW_IP_ADDRESSES_ENTRY\n");

	bdmf_session_print(session, "\tsa_da_addresses          =\n\t");
	for (i=0,j=0; i<32; i++)
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
	bdmf_session_print(session, "\treserved0                =\n\t");
	for (i=0,j=0; i<4; i++)
	{
		MREAD_I_8((uint8_t *)(p) + 32, i, (r));
		bdmf_session_print(session, "0x%08x ", (unsigned int)r);
		j++;
	}
	bdmf_session_print(session, "\n");
	MREAD_32((uint8_t *)(p) + 36, (r));
	bdmf_session_print(session, "\treference_count          = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	bdmf_session_print(session, "\treserved1                =\n\t");
	for (i=0,j=0; i<4; i++)
	{
		MREAD_I_8((uint8_t *)(p) + 40, i, (r));
		bdmf_session_print(session, "0x%08x ", (unsigned int)r);
		j++;
	}
	bdmf_session_print(session, "\n");
	MREAD_32((uint8_t *)(p) + 44, (r));
	bdmf_session_print(session, "\tis_ipv6_address          = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_PROCESSING_RX_DESCRIPTOR(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register PROCESSING_RX_DESCRIPTOR\n");

	MREAD_32((uint8_t *)(p), (r));
	bdmf_session_print(session, "\tpd_info                  = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_16((uint8_t *)(p) + 4, 6, 10, (r));
	bdmf_session_print(session, "\tserial_num               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 5, 5, 1, (r));
	bdmf_session_print(session, "\tploam                    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 5, 4, 1, (r));
	bdmf_session_print(session, "\tingress_cong             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 5, 3, 1, (r));
	bdmf_session_print(session, "\tabs_or_dsl               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 5, 2, 1, (r));
	bdmf_session_print(session, "\tl3_packet                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_32((uint8_t *)(p) + 4, 14, 4, (r));
	bdmf_session_print(session, "\terror_type_or_cpu_tx     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_16((uint8_t *)(p) + 6, 0, 14, (r));
	bdmf_session_print(session, "\tpacket_length            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 8, 7, 1, (r));
	bdmf_session_print(session, "\terror                    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 8, 6, 1, (r));
	bdmf_session_print(session, "\ttarget_mem_1             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 8, 4, 2, (r));
	bdmf_session_print(session, "\tcong_state               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 8, 3, 1, (r));
	bdmf_session_print(session, "\tis_emac                  = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_16((uint8_t *)(p) + 8, 3, 8, (r));
	bdmf_session_print(session, "\tingress_vport_or_flow    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_32((uint8_t *)(p) + 8, 0, 19, (r));
	bdmf_session_print(session, "\tbn1_last_or_abs1         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 12, 7, 1, (r));
	bdmf_session_print(session, "\tagg_pd                   = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 12, 6, 1, (r));
	bdmf_session_print(session, "\ttarget_mem_0             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_32((uint8_t *)(p) + 12, 0, 30, (r));
	bdmf_session_print(session, "\tpayload_offset_sop       = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_OVERALL_RATE_LIMITER_DESCRIPTOR(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register OVERALL_RATE_LIMITER_DESCRIPTOR\n");

	MREAD_32((uint8_t *)(p), (r));
	bdmf_session_print(session, "\tbbh_queue_en_vec         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 4, (r));
	bdmf_session_print(session, "\tcurrent_budget           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_16((uint8_t *)(p) + 8, 2, 14, (r));
	bdmf_session_print(session, "\talloc_mantissa           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 9, 0, 2, (r));
	bdmf_session_print(session, "\talloc_exponent           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_16((uint8_t *)(p) + 10, 2, 14, (r));
	bdmf_session_print(session, "\tlimit_mantissa           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 11, 0, 2, (r));
	bdmf_session_print(session, "\tlimit_exponent           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 12, (r));
	bdmf_session_print(session, "\treserved1                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_MAC_TYPE_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register MAC_TYPE_ENTRY\n");

	MREAD_8((uint8_t *)(p), (r));
	bdmf_session_print(session, "\ttype                     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

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

void dump_RDD_CPU_TX_DBG_CNTRS(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	int i,j;

	bdmf_session_print(session, "  Register CPU_TX_DBG_CNTRS\n");

	MREAD_32((uint8_t *)(p), (r));
	bdmf_session_print(session, "\tSBPM_NO_NEXT             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 4, (r));
	bdmf_session_print(session, "\tSBPM_NO_FIRST            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 8, (r));
	bdmf_session_print(session, "\tTX_FLOW_DISABLE          = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 12, (r));
	bdmf_session_print(session, "\tNO_FPM                   = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 16, (r));
	bdmf_session_print(session, "\tTASK_EXIT                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 20, (r));
	bdmf_session_print(session, "\tSYNC_WAIT                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 24, (r));
	bdmf_session_print(session, "\tNO_DISPATCHER_SCHEDULER  = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 28, (r));
	bdmf_session_print(session, "\tRECYCLE_FIFO_FULL        = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 32, (r));
	bdmf_session_print(session, "\tDROP_PKT                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 36, (r));
	bdmf_session_print(session, "\tTASK_START               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 40, (r));
	bdmf_session_print(session, "\tNO_FWD                   = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	bdmf_session_print(session, "\treserved                 =\n\t");
	for (i=0,j=0; i<5; i++)
	{
		MREAD_I_32((uint8_t *)(p) + 44, i, (r));
		bdmf_session_print(session, "0x%08x ", (unsigned int)r);
		j++;
	}
	bdmf_session_print(session, "\n");
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

void dump_RDD_SPDSVC_RX_TS_STAT(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register SPDSVC_RX_TS_STAT\n");

	MREAD_32((uint8_t *)(p), (r));
	bdmf_session_print(session, "\tts_first                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 4, (r));
	bdmf_session_print(session, "\tts_last                  = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_32((uint8_t *)(p) + 8, 1, 31, (r));
	bdmf_session_print(session, "\treserved                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 11, 0, 1, (r));
	bdmf_session_print(session, "\tts_first_set             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

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

void dump_RDD_DHD_DOORBELL(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register DHD_DOORBELL\n");

	MREAD_32((uint8_t *)(p), (r));
	bdmf_session_print(session, "\tdoorbell_value           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

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
	bdmf_session_print(session, "\tbackup_index_fifo_base_low= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 28, (r));
	bdmf_session_print(session, "\tbackup_index_fifo_base_high= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 32, (r));
	bdmf_session_print(session, "\tdhd_doorbell_low         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 36, (r));
	bdmf_session_print(session, "\tdhd_doorbell_high        = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 40, (r));
	bdmf_session_print(session, "\ttx_post_wr_idx           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 42, (r));
	bdmf_session_print(session, "\ttx_post_qd               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 44, (r));
	bdmf_session_print(session, "\ttx_post_rd_idx           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 46, (r));
	bdmf_session_print(session, "\tidma_last_group_fr       = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 48, (r));
	bdmf_session_print(session, "\tcam_rslt                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 52, 7, 1, (r));
	bdmf_session_print(session, "\tadd_llcsnap_header       = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 52, 5, 2, (r));
	bdmf_session_print(session, "\treserved                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 52, 2, 3, (r));
	bdmf_session_print(session, "\tflow_ring_format         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 52, 1, 1, (r));
	bdmf_session_print(session, "\tidma_active              = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 52, 0, 1, (r));
	bdmf_session_print(session, "\tcodel_active             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 53, (r));
	bdmf_session_print(session, "\tidma_last_group_id       = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 54, (r));
	bdmf_session_print(session, "\tcache_ctx_next_write_idx = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 55, (r));
	bdmf_session_print(session, "\tbackup_cache_idx         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 56, (r));
	bdmf_session_print(session, "\tcodel_max_seq_drops      = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 58, (r));
	bdmf_session_print(session, "\tfr_ptrs_size_shift       = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 59, (r));
	bdmf_session_print(session, "\tidma_group_shift         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 60, (r));
	bdmf_session_print(session, "\tdoorbell_value_address   = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	bdmf_session_print(session, "\tscratchpad               =\n\t");
	for (i=0,j=0; i<7; i++)
	{
		MREAD_I_32((uint8_t *)(p) + 64, i, (r));
		bdmf_session_print(session, "0x%08x ", (unsigned int)r);
		j++;
	}
	bdmf_session_print(session, "\n");
	MREAD_16((uint8_t *)(p) + 92, (r));
	bdmf_session_print(session, "\tbackup_rd_idx            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 94, (r));
	bdmf_session_print(session, "\tbackup_wr_idx            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 96, 7, 1, (r));
	bdmf_session_print(session, "\tpacket_valid             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 96, 6, 1, (r));
	bdmf_session_print(session, "\tpacket_abs_addr          = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_16((uint8_t *)(p) + 96, 0, 14, (r));
	bdmf_session_print(session, "\tpacket_data_len          = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 98, 5, 3, (r));
	bdmf_session_print(session, "\tpacket_prio              = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 98, 4, 1, (r));
	bdmf_session_print(session, "\tbuffer_prio              = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 98, 3, 1, (r));
	bdmf_session_print(session, "\tbcmc_packet              = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 98, 0, 3, (r));
	bdmf_session_print(session, "\treserved6                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 99, (r));
	bdmf_session_print(session, "\trsv_address_high         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 100, (r));
	bdmf_session_print(session, "\tfpm_address_low          = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 104, (r));
	bdmf_session_print(session, "\trequest_id               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 108, (r));
	bdmf_session_print(session, "\tbcmc_seqnum              = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 110, (r));
	bdmf_session_print(session, "\tcpu_msg_done             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 111, (r));
	bdmf_session_print(session, "\tcoalescing_max_count     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	bdmf_session_print(session, "\tdoorbell_counters        =\n\t");
	for (i=0,j=0; i<64; i++)
	{
		MREAD_I_8((uint8_t *)(p) + 112, i, (r));
		bdmf_session_print(session, "0x%08x ", (unsigned int)r);
		j++;
		if (j >= 8)
		{
			j = 0;
			bdmf_session_print(session, "\n\t");
		}
	}
	bdmf_session_print(session, "\n");
	MREAD_8((uint8_t *)(p) + 176, (r));
	bdmf_session_print(session, "\treserved3                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 177, (r));
	bdmf_session_print(session, "\tcoalescing_timeout       = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 178, (r));
	bdmf_session_print(session, "\tcoalescing_timeout_cntr  = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 179, (r));
	bdmf_session_print(session, "\tcur_frg_id               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 180, 7, 1, (r));
	bdmf_session_print(session, "\ttx_mirroring_en          = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 180, 1, 6, (r));
	bdmf_session_print(session, "\tdst_vport                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_16((uint8_t *)(p) + 180, 0, 9, (r));
	bdmf_session_print(session, "\tdst_queue                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	bdmf_session_print(session, "\treserved5                =\n\t");
	for (i=0,j=0; i<2; i++)
	{
		MREAD_I_8((uint8_t *)(p) + 182, i, (r));
		bdmf_session_print(session, "0x%08x ", (unsigned int)r);
		j++;
	}
	bdmf_session_print(session, "\n");
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

void dump_RDD_SPDSVC_WLAN_TXPOST_PARAMS(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register SPDSVC_WLAN_TXPOST_PARAMS\n");

	MREAD_16((uint8_t *)(p), (r));
	bdmf_session_print(session, "\tspdsvc_free_idx          = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_DHD_CODEL_BIAS_SLOPE(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register DHD_CODEL_BIAS_SLOPE\n");

	MREAD_16((uint8_t *)(p), (r));
	bdmf_session_print(session, "\tbias                     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 2, (r));
	bdmf_session_print(session, "\tslope                    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

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
	bdmf_session_print(session, "\treserved2                = 0x%08x", (unsigned int)r);
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

	MREAD_16((uint8_t *)(p) + 28, (r));
	bdmf_session_print(session, "\tTCPSPDTEST_ENGINE_TCB_bad_pkts= 0x%08x", (unsigned int)r);
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

	MREAD_16((uint8_t *)(p) + 112, (r));
	bdmf_session_print(session, "\tTCPSPDTEST_ENGINE_TCB_dynack_bytes_thr= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 114, (r));
	bdmf_session_print(session, "\tTCPSPDTEST_ENGINE_TCB_dynack_bytes_thr_cnt= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 116, (r));
	bdmf_session_print(session, "\tTCPSPDTEST_ENGINE_TCB_dynack_thr_upd_rate= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 117, (r));
	bdmf_session_print(session, "\tTCPSPDTEST_ENGINE_TCB_dynack_thr_upd_rate_cnt= 0x%08x", (unsigned int)r);
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
	bdmf_session_print(session, "\tTCPSPDTEST_ENGINE_TCB_is_stream_active= 0x%08x", (unsigned int)r);
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

void dump_RDD_CRYPTO_SESSION_PARAMS(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register CRYPTO_SESSION_PARAMS\n");

	MREAD_32((uint8_t *)(p), (r));
	bdmf_session_print(session, "\tspu_fixed_hdr_pd_lo      = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 4, (r));
	bdmf_session_print(session, "\tspu_fixed_hdr_pd_hi      = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 8, 7, 1, (r));
	bdmf_session_print(session, "\tis_esn                   = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 8, 6, 1, (r));
	bdmf_session_print(session, "\tesp_o_udp                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_16((uint8_t *)(p) + 8, 0, 14, (r));
	bdmf_session_print(session, "\treserved                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 10, (r));
	bdmf_session_print(session, "\tdigest_size              = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

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

void dump_RDD_TX_ABS_RECYCLE_COUNTERS_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register TX_ABS_RECYCLE_COUNTERS_ENTRY\n");

	MREAD_32((uint8_t *)(p), (r));
	bdmf_session_print(session, "\ttotal                    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 4, (r));
	bdmf_session_print(session, "\tspdsvc                   = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

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

void dump_RDD_CRYPTO_SESSION_SEQ_INFO(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	int i,j;

	bdmf_session_print(session, "  Register CRYPTO_SESSION_SEQ_INFO\n");

	MREAD_32((uint8_t *)(p), (r));
	bdmf_session_print(session, "\tcurlft_bytes_hi          = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 4, (r));
	bdmf_session_print(session, "\tcurlft_bytes_lo          = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 8, (r));
	bdmf_session_print(session, "\tcurlft_pkts_hi           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 12, (r));
	bdmf_session_print(session, "\tcurlft_pkts_lo           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 16, (r));
	bdmf_session_print(session, "\tseq_hi_oseq_hi           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 20, (r));
	bdmf_session_print(session, "\tseq_oseq_lo              = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 24, (r));
	bdmf_session_print(session, "\terr_replay               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 28, (r));
	bdmf_session_print(session, "\terr_repl_window          = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 32, (r));
	bdmf_session_print(session, "\ttype_base_chksm          = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 36, (r));
	bdmf_session_print(session, "\texception                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 40, (r));
	bdmf_session_print(session, "\tbitmap_hard_byte_hi      = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 44, (r));
	bdmf_session_print(session, "\thard_byte_limit_lo       = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 48, (r));
	bdmf_session_print(session, "\thard_pkt_limit_hi        = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 52, (r));
	bdmf_session_print(session, "\thard_pkt_limit_lo        = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 56, (r));
	bdmf_session_print(session, "\tsoft_byte_limit_hi       = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 60, (r));
	bdmf_session_print(session, "\tsoft_byte_limit_lo       = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 64, (r));
	bdmf_session_print(session, "\tsoft_pkt_limit_hi        = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 68, (r));
	bdmf_session_print(session, "\tsoft_pkt_limit_lo        = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 72, (r));
	bdmf_session_print(session, "\tbitmap_seqhi             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 76, (r));
	bdmf_session_print(session, "\talign1                   = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	bdmf_session_print(session, "\tfiller                   =\n\t");
	for (i=0,j=0; i<12; i++)
	{
		MREAD_I_32((uint8_t *)(p) + 80, i, (r));
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

void dump_RDD_SPU_OFFLOAD_PARAMS(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register SPU_OFFLOAD_PARAMS\n");

	MREAD_32((uint8_t *)(p), (r));
	bdmf_session_print(session, "\tring_bd_base_lo          = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 4, (r));
	bdmf_session_print(session, "\tring_bd_base_hi          = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 8, (r));
	bdmf_session_print(session, "\tread_idx_ddr_addr_lo     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 12, (r));
	bdmf_session_print(session, "\tread_idx_ddr_addr_hi     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 16, (r));
	bdmf_session_print(session, "\tsession_info_base_lo     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 20, (r));
	bdmf_session_print(session, "\tsession_info_base_hi     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 24, (r));
	bdmf_session_print(session, "\tdoorbell_reg             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 28, (r));
	bdmf_session_print(session, "\twrite_idx                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 30, (r));
	bdmf_session_print(session, "\tshadow_read_idx          = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 32, (r));
	bdmf_session_print(session, "\tdiscarded                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 34, (r));
	bdmf_session_print(session, "\tirq_id                   = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 36, (r));
	bdmf_session_print(session, "\treqid                    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_SPDSVC_TCPSPDTEST_COMMON_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register SPDSVC_TCPSPDTEST_COMMON_ENTRY\n");

	MREAD_8((uint8_t *)(p), (r));
	bdmf_session_print(session, "\tcpu_rx_copy_is_tcpspdtest_timeout_wakeup= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

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
	bdmf_session_print(session, "\ttx_drops_no_spbm         = 0x%08x", (unsigned int)r);
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

	MREAD_32((uint8_t *)(p) + 4, (r));
	bdmf_session_print(session, "\tstream_vector            = 0x%08x", (unsigned int)r);
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
	bdmf_session_print(session, "\tup_next_stream_id        = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 19, 1, 7, (r));
	bdmf_session_print(session, "\treserved1                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 19, 0, 1, (r));
	bdmf_session_print(session, "\tup_is_timer_active       = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

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

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 96, 0, 3, (r));
	bdmf_session_print(session, "\treserved                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)(p) + 97, (r));
	bdmf_session_print(session, "\trch_max_seg              = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 98, (r));
	bdmf_session_print(session, "\trx_post_rd_idx           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 100, (r));
	bdmf_session_print(session, "\treserved1                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	bdmf_session_print(session, "\tscratchpad               =\n\t");
	for (i=0,j=0; i<3; i++)
	{
		MREAD_I_32((uint8_t *)(p) + 104, i, (r));
		bdmf_session_print(session, "0x%08x ", (unsigned int)r);
		j++;
	}
	bdmf_session_print(session, "\n");
	bdmf_session_print(session, "\treserved2                =\n\t");
	for (i=0,j=0; i<4; i++)
	{
		MREAD_I_8((uint8_t *)(p) + 116, i, (r));
		bdmf_session_print(session, "0x%08x ", (unsigned int)r);
		j++;
	}
	bdmf_session_print(session, "\n");
	bdmf_session_print(session, "\trch_buff                 =\n\t");
	for (i=0,j=0; i<18; i++)
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
}

void dump_RDD_FPM_RING_CFG_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	int i,j;

	bdmf_session_print(session, "  Register FPM_RING_CFG_ENTRY\n");

	bdmf_session_print(session, "\tbase_addr                =\n\t");
	for (i=0,j=0; i<2; i++)
	{
		MREAD_I_32((uint8_t *)(p), i, (r));
		bdmf_session_print(session, "0x%08x ", (unsigned int)r);
		j++;
	}
	bdmf_session_print(session, "\n");
	bdmf_session_print(session, "\tidx_addr                 =\n\t");
	for (i=0,j=0; i<2; i++)
	{
		MREAD_I_32((uint8_t *)(p) + 8, i, (r));
		bdmf_session_print(session, "0x%08x ", (unsigned int)r);
		j++;
	}
	bdmf_session_print(session, "\n");
	MREAD_16((uint8_t *)(p) + 16, (r));
	bdmf_session_print(session, "\tclr_tail_offset          = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 18, (r));
	bdmf_session_print(session, "\tclr_tail_size            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 20, 7, 1, (r));
	bdmf_session_print(session, "\tis_valid                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_8((uint8_t *)(p) + 20, 6, 1, (r));
	bdmf_session_print(session, "\tprio                     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	RDP_FIELD_MREAD_16((uint8_t *)(p) + 20, 0, 14, (r));
	bdmf_session_print(session, "\tbuffer_nums              = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 22, (r));
	bdmf_session_print(session, "\tbuffer_size              = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

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

void dump_RDD_DHD_RING_SIZE(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register DHD_RING_SIZE\n");

	MREAD_16((uint8_t *)(p), (r));
	bdmf_session_print(session, "\tvalue                    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_SPU_RESPONSE_PARAMS(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register SPU_RESPONSE_PARAMS\n");

	MREAD_32((uint8_t *)(p), (r));
	bdmf_session_print(session, "\tread_idx_ddr_addr_lo     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 4, (r));
	bdmf_session_print(session, "\tread_idx_ddr_addr_hi     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 8, (r));
	bdmf_session_print(session, "\tcmpl_base_lo             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 12, (r));
	bdmf_session_print(session, "\tcmpl_base_hi             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 16, (r));
	bdmf_session_print(session, "\tring_bd_base_lo          = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 20, (r));
	bdmf_session_print(session, "\tring_bd_base_hi          = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 24, (r));
	bdmf_session_print(session, "\tsession_info_base_lo     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 28, (r));
	bdmf_session_print(session, "\tsession_info_base_hi     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 32, (r));
	bdmf_session_print(session, "\tcmpl_wr_ptr_reg          = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 36, (r));
	bdmf_session_print(session, "\tcmpl_rd_offset           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 40, (r));
	bdmf_session_print(session, "\tcmpl_count               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 44, (r));
	bdmf_session_print(session, "\tring_rd_idx              = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 48, (r));
	bdmf_session_print(session, "\treserved                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 50, (r));
	bdmf_session_print(session, "\tirq_id                   = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 52, (r));
	bdmf_session_print(session, "\tcmpl_err                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 54, (r));
	bdmf_session_print(session, "\tseqno_err                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

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
	bdmf_session_print(session, "\tstatus                   = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 10, (r));
	bdmf_session_print(session, "\tflow_ring_id             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 12, (r));
	bdmf_session_print(session, "\tdma_done_mark            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_SPDSVC_WLAN_GEN_PARAMS(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register SPDSVC_WLAN_GEN_PARAMS\n");

	MREAD_32((uint8_t *)(p), (r));
	bdmf_session_print(session, "\tcomplete_tracked         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)(p) + 4, (r));
	bdmf_session_print(session, "\ttx_copies                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)(p) + 8, (r));
	bdmf_session_print(session, "\tspdsvc_free_idx          = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

