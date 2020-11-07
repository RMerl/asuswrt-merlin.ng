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

#if defined DSL_63138
void dump_RDD_BBH_TX_DESCRIPTOR(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register BBH_TX_DESCRIPTOR\n");

	FIELD_MREAD_8((uint8_t *)p, 7, 1, r);
	bdmf_session_print(session, "\tcrc_calc                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p, 3, 12, r);
	bdmf_session_print(session, "\twan_port_or_fstat        = 0x%08x", (unsigned int)r);
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

	FIELD_MREAD_16((uint8_t *)p + 6, 0, 15, r);
	bdmf_session_print(session, "\tpacket_location          = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

#endif

void dump_RDD_SERVICE_QUEUE_DESCRIPTOR(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register SERVICE_QUEUE_DESCRIPTOR\n");

	FIELD_MREAD_16((uint8_t *)p, 1, 15, r);
	bdmf_session_print(session, "\tunion_field1             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_32((uint8_t *)p + 0, 14, 3, r);
	bdmf_session_print(session, "\tegress_port              = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p + 2, 0, 14, r);
	bdmf_session_print(session, "\tpacket_length            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 4, 7, 1, r);
	bdmf_session_print(session, "\tabsolute_normal          = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 4, 6, 1, r);
	bdmf_session_print(session, "\tmulticast_ssid           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 4, 5, 1, r);
	bdmf_session_print(session, "\twred_bit                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 4, 2, 3, r);
	bdmf_session_print(session, "\ttx_queue                 = 0x%08x", (unsigned int)r);
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
	bdmf_session_print(session, "\treserved2                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p + 6, 0, 14, r);
	bdmf_session_print(session, "\tbuffer_number            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_CONNECTION_CONTEXT_BUFFER_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	int i,j;

	bdmf_session_print(session, "  Register CONNECTION_CONTEXT_BUFFER_ENTRY\n");

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

void dump_RDD_GSO_BUFFER_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	int i,j;

	bdmf_session_print(session, "  Register GSO_BUFFER_ENTRY\n");

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

void dump_RDD_GSO_PSEUDO_HEADER_BUFFER_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	int i,j;

	bdmf_session_print(session, "  Register GSO_PSEUDO_HEADER_BUFFER_ENTRY\n");

	bdmf_session_print(session, "\treserved_fw_only         =\n\t");
	for (i=0,j=0; i<10; i++)
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

void dump_RDD_TWO_BYTES(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register TWO_BYTES\n");

	MREAD_16((uint8_t *)p, r);
	bdmf_session_print(session, "\treserved_fw_only         = 0x%08x", (unsigned int)r);
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

void dump_RDD_CPU_REASON_TO_METER_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register CPU_REASON_TO_METER_ENTRY\n");

	MREAD_8((uint8_t *)p, r);
	bdmf_session_print(session, "\tcpu_meter                = 0x%08x", (unsigned int)r);
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

void dump_RDD_IPSEC_DS_BUFFER(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	int i,j;

	bdmf_session_print(session, "  Register IPSEC_DS_BUFFER\n");

	bdmf_session_print(session, "\treserved                 =\n\t");
	for (i=0,j=0; i<176; i++)
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

void dump_RDD_IPSEC_SA_DESC(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	int i,j;

	bdmf_session_print(session, "  Register IPSEC_SA_DESC\n");

	MREAD_32((uint8_t *)p, r);
	bdmf_session_print(session, "\tspi                      = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 4, r);
	bdmf_session_print(session, "\teng_cfg                  = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 6, r);
	bdmf_session_print(session, "\treserved1                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 7, 7, 1, r);
	bdmf_session_print(session, "\thmac_dis                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 7, 6, 1, r);
	bdmf_session_print(session, "\tauth_key_fetch_dis       = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 7, 2, 4, r);
	bdmf_session_print(session, "\treserved2                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 7, 0, 2, r);
	bdmf_session_print(session, "\tauth_alg                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 8, r);
	bdmf_session_print(session, "\tnxt_hdr                  = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 9, 5, 3, r);
	bdmf_session_print(session, "\taes_key_size             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 9, 2, 3, r);
	bdmf_session_print(session, "\tdes_dec_vec              = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 9, 0, 2, r);
	bdmf_session_print(session, "\tdes_iters                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 10, 7, 1, r);
	bdmf_session_print(session, "\trng_clk_en               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 10, 6, 1, r);
	bdmf_session_print(session, "\treserved3                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 10, 5, 1, r);
	bdmf_session_print(session, "\trng_seed                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 10, 0, 5, r);
	bdmf_session_print(session, "\trng_sample_num           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 11, 7, 1, r);
	bdmf_session_print(session, "\treserved4                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 11, 6, 1, r);
	bdmf_session_print(session, "\tcrypt_key_fetch_dis      = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 11, 5, 1, r);
	bdmf_session_print(session, "\twrite_dis                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 11, 3, 2, r);
	bdmf_session_print(session, "\tmech                     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 11, 2, 1, r);
	bdmf_session_print(session, "\tdecrypt                  = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 11, 0, 2, r);
	bdmf_session_print(session, "\tcrypt_alg                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p + 12, 3, 13, r);
	bdmf_session_print(session, "\treserved5                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 13, 2, 1, r);
	bdmf_session_print(session, "\tclustr_ovrrd             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 13, 1, 1, r);
	bdmf_session_print(session, "\tread_clustr_sel          = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 13, 0, 1, r);
	bdmf_session_print(session, "\twrite_clustr_sel         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 14, r);
	bdmf_session_print(session, "\tadd                      = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	bdmf_session_print(session, "\tauth_key                 =\n\t");
	for (i=0,j=0; i<32; i++)
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
	bdmf_session_print(session, "\tcrypt_key                =\n\t");
	for (i=0,j=0; i<32; i++)
	{
		MREAD_I_8((uint8_t *)p + 48, i, r);
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

void dump_RDD_TIMER_SCHEDULER_PRIMITIVE_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register TIMER_SCHEDULER_PRIMITIVE_ENTRY\n");

	MREAD_16((uint8_t *)p, r);
	bdmf_session_print(session, "\tprimitive_address        = 0x%08x", (unsigned int)r);
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

void dump_RDD_SPDSVC_CONTEXT_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register SPDSVC_CONTEXT_ENTRY\n");

	MREAD_32((uint8_t *)p, r);
	bdmf_session_print(session, "\tbbh_descriptor_0         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 4, r);
	bdmf_session_print(session, "\tbbh_descriptor_1         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 8, r);
	bdmf_session_print(session, "\treserved0                = 0x%08x", (unsigned int)r);
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

	FIELD_MREAD_8((uint8_t *)p + 20, 7, 1, r);
	bdmf_session_print(session, "\tterminate                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p + 20, 0, 15, r);
	bdmf_session_print(session, "\treserved1                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 22, r);
	bdmf_session_print(session, "\ttotal_length             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 24, r);
	bdmf_session_print(session, "\ttokens                   = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 26, r);
	bdmf_session_print(session, "\tbucket_size              = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 28, r);
	bdmf_session_print(session, "\tbucket                   = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 32, r);
	bdmf_session_print(session, "\ttx_queue_discards        = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 36, r);
	bdmf_session_print(session, "\ttx_queue_writes          = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 40, r);
	bdmf_session_print(session, "\ttx_queue_reads           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 44, r);
	bdmf_session_print(session, "\tstart_time_usec          = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 48, r);
	bdmf_session_print(session, "\tend_time_usec            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 52, r);
	bdmf_session_print(session, "\tdata_buf_ptr             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 56, r);
	bdmf_session_print(session, "\teth_header_0             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 60, r);
	bdmf_session_print(session, "\teth_header_1             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 64, r);
	bdmf_session_print(session, "\teth_header_2             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 68, r);
	bdmf_session_print(session, "\teth_header_3             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 72, r);
	bdmf_session_print(session, "\teth_header_4             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 76, r);
	bdmf_session_print(session, "\teth_header_5             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 78, r);
	bdmf_session_print(session, "\treserved2                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_FC_L2_UCAST_CONNECTION_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register FC_L2_UCAST_CONNECTION_ENTRY\n");

	FIELD_MREAD_8((uint8_t *)p, 7, 1, r);
	bdmf_session_print(session, "\tvalid                    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p, 0, 15, r);
	bdmf_session_print(session, "\tcontext_index            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 2, 5, 3, r);
	bdmf_session_print(session, "\tbucket_overflow_counter  = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 2, 1, 4, r);
	bdmf_session_print(session, "\tcommand_list_length_64   = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 2, 0, 1, r);
	bdmf_session_print(session, "\tkey_extend               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 3, 7, 1, r);
	bdmf_session_print(session, "\tis_multicast             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 3, 0, 7, r);
	bdmf_session_print(session, "\tprotocol                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_32((uint8_t *)p + 4, 9, 23, r);
	bdmf_session_print(session, "\treserved                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 6, 0, 1, r);
	bdmf_session_print(session, "\ttcp_pure_ack             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 7, r);
	bdmf_session_print(session, "\ttos                      = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 8, r);
	bdmf_session_print(session, "\tsrc_mac_crc              = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 12, r);
	bdmf_session_print(session, "\tdst_mac_crc              = 0x%08x", (unsigned int)r);
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

void dump_RDD_WLAN_MCAST_CONTROL_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	int i,j;

	bdmf_session_print(session, "  Register WLAN_MCAST_CONTROL_ENTRY\n");

	MREAD_32((uint8_t *)p, r);
	bdmf_session_print(session, "\tbbh_descriptor_0         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 4, r);
	bdmf_session_print(session, "\tbbh_descriptor_1         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 8, r);
	bdmf_session_print(session, "\tl2_buffer_0              = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 12, r);
	bdmf_session_print(session, "\tl2_buffer_1              = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 16, r);
	bdmf_session_print(session, "\tl2_buffer_2              = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 20, r);
	bdmf_session_print(session, "\tl2_buffer_3              = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 24, r);
	bdmf_session_print(session, "\tl2_buffer_4              = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 28, r);
	bdmf_session_print(session, "\tl2_buffer_5              = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 30, r);
	bdmf_session_print(session, "\tingress_queue_write_ptr  = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 32, r);
	bdmf_session_print(session, "\tbpm_buffer_number_0      = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 36, r);
	bdmf_session_print(session, "\tbpm_buffer_number_1      = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 40, r);
	bdmf_session_print(session, "\tbpm_buffer_number_2      = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 44, r);
	bdmf_session_print(session, "\tbpm_buffer_number_3      = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 48, r);
	bdmf_session_print(session, "\tbpm_buffer_address_0     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 52, r);
	bdmf_session_print(session, "\tbpm_buffer_address_1     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 56, r);
	bdmf_session_print(session, "\tbpm_buffer_address_2     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 60, r);
	bdmf_session_print(session, "\tbpm_buffer_address_3     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 64, r);
	bdmf_session_print(session, "\trequest_id               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 68, r);
	bdmf_session_print(session, "\tdata_buf_ptr             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 72, r);
	bdmf_session_print(session, "\tdhd_list_base_address    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 76, r);
	bdmf_session_print(session, "\tingress_queue_pd_write_ptr= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 78, r);
	bdmf_session_print(session, "\tingress_queue_pd_read_ptr= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 80, r);
	bdmf_session_print(session, "\tis_proxy_enabled         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 81, r);
	bdmf_session_print(session, "\tdhd_list_size            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 82, r);
	bdmf_session_print(session, "\tdhd_list_ptr             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	bdmf_session_print(session, "\tdhd_list_table           =\n\t");
	for (i=0,j=0; i<64; i++)
	{
		MREAD_I_8((uint8_t *)p + 84, i, r);
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

void dump_RDD_DS_WAN_UDP_FILTER_CONTROL_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register DS_WAN_UDP_FILTER_CONTROL_ENTRY\n");

	MREAD_32((uint8_t *)p, r);
	bdmf_session_print(session, "\tvalid_mask               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_TOTAL_PPS_RATE_LIMITER_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register TOTAL_PPS_RATE_LIMITER_ENTRY\n");

	MREAD_16((uint8_t *)p, r);
	bdmf_session_print(session, "\ttokens                   = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 2, r);
	bdmf_session_print(session, "\tbucket                   = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 4, r);
	bdmf_session_print(session, "\treserved                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 6, r);
	bdmf_session_print(session, "\tbucket_size              = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_FC_L2_UCAST_TUPLE_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register FC_L2_UCAST_TUPLE_ENTRY\n");

	MREAD_16((uint8_t *)p, r);
	bdmf_session_print(session, "\treserved_0               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 2, r);
	bdmf_session_print(session, "\tdst_mac                  = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 4, r);
	bdmf_session_print(session, "\tdst_mac_lsw              = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 8, r);
	bdmf_session_print(session, "\treserved_1               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 12, r);
	bdmf_session_print(session, "\treserved_2               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 16, r);
	bdmf_session_print(session, "\tvtag0_tpid               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 18, r);
	bdmf_session_print(session, "\tvtag0_tci                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 20, r);
	bdmf_session_print(session, "\tvtag1_tpid               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 22, r);
	bdmf_session_print(session, "\tvtag1_tci                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 24, r);
	bdmf_session_print(session, "\tsrc_mac                  = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 28, r);
	bdmf_session_print(session, "\tsrc_mac_lshw             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 30, r);
	bdmf_session_print(session, "\teth_type                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

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

	FIELD_MREAD_8((uint8_t *)p + 1, 0, 1, r);
	bdmf_session_print(session, "\tqos_mapping_mode         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 2, 4, 4, r);
	bdmf_session_print(session, "\twifi_ssid                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 2, 0, 4, r);
	bdmf_session_print(session, "\tegress_port              = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 3, 5, 3, r);
	bdmf_session_print(session, "\ttraffic_class            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 3, 4, 1, r);
	bdmf_session_print(session, "\tforward_mode             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 3, 3, 1, r);
	bdmf_session_print(session, "\tservice_queue_mode       = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 3, 0, 3, r);
	bdmf_session_print(session, "\tdest                     = 0x%08x", (unsigned int)r);
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

	FIELD_MREAD_8((uint8_t *)p + 5, 2, 5, r);
	bdmf_session_print(session, "\tservice_queue            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 5, 1, 1, r);
	bdmf_session_print(session, "\tdei_remark_enable        = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 5, 0, 1, r);
	bdmf_session_print(session, "\tdei_value                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 6, 7, 1, r);
	bdmf_session_print(session, "\tic_ip_flow               = 0x%08x", (unsigned int)r);
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

void dump_RDD_FC_MCAST_CONNECTION2_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register FC_MCAST_CONNECTION2_ENTRY\n");

	FIELD_MREAD_8((uint8_t *)p, 4, 4, r);
	bdmf_session_print(session, "\treserved1                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p, 0, 12, r);
	bdmf_session_print(session, "\tvid0                     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 2, 4, 4, r);
	bdmf_session_print(session, "\treserved2                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p + 2, 0, 12, r);
	bdmf_session_print(session, "\tvid1                     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 4, 7, 1, r);
	bdmf_session_print(session, "\tvalid                    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p + 4, 0, 15, r);
	bdmf_session_print(session, "\tcontext_index            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 6, r);
	bdmf_session_print(session, "\trx_if                    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 7, 7, 1, r);
	bdmf_session_print(session, "\treserved4                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 7, 0, 7, r);
	bdmf_session_print(session, "\tnext_entry               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 8, r);
	bdmf_session_print(session, "\tip_sa                    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 12, r);
	bdmf_session_print(session, "\treserved3                = 0x%08x", (unsigned int)r);
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

void dump_RDD_FOUR_BYTES(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register FOUR_BYTES\n");

	MREAD_32((uint8_t *)p, r);
	bdmf_session_print(session, "\treserved_fw_only         = 0x%08x", (unsigned int)r);
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
	bdmf_session_print(session, "\trs_status_offset         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 13, r);
	bdmf_session_print(session, "\trs_group_status_offset   = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 14, r);
	bdmf_session_print(session, "\tqueue_mask               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 15, r);
	bdmf_session_print(session, "\tindex                    = 0x%08x", (unsigned int)r);
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

void dump_RDD_FORWARDING_MATRIX_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register FORWARDING_MATRIX_ENTRY\n");

	MREAD_8((uint8_t *)p, r);
	bdmf_session_print(session, "\tenable                   = 0x%08x", (unsigned int)r);
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

void dump_RDD_DHD_TX_COMPLETE_DESCRIPTOR(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register DHD_TX_COMPLETE_DESCRIPTOR\n");

	MREAD_8((uint8_t *)p, r);
	bdmf_session_print(session, "\tmsg_type                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 1, r);
	bdmf_session_print(session, "\tif_id                    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 2, r);
	bdmf_session_print(session, "\tcommon_hdr_flags         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 3, r);
	bdmf_session_print(session, "\tepoch                    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 4, r);
	bdmf_session_print(session, "\trequest_id               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 8, r);
	bdmf_session_print(session, "\tstatus                   = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 10, r);
	bdmf_session_print(session, "\tflow_ring_id             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 12, r);
	bdmf_session_print(session, "\tdma_done_mark            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_DHD_TX_COMPLETE_DESCRIPTOR_CWI32(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register DHD_TX_COMPLETE_DESCRIPTOR_CWI32\n");

	MREAD_32((uint8_t *)p, r);
	bdmf_session_print(session, "\trequest_id               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 4, r);
	bdmf_session_print(session, "\tflow_ring_id             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 6, r);
	bdmf_session_print(session, "\tif_id                    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 7, r);
	bdmf_session_print(session, "\treserved                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_ETH_TX_MAC_DESCRIPTOR(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register ETH_TX_MAC_DESCRIPTOR\n");

	MREAD_8((uint8_t *)p, r);
	bdmf_session_print(session, "\tegress_counter           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_32((uint8_t *)p + 0, 0, 24, r);
	bdmf_session_print(session, "\treserved0                = 0x%08x", (unsigned int)r);
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

	MREAD_8((uint8_t *)p + 7, r);
	bdmf_session_print(session, "\treserved1                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 8, r);
	bdmf_session_print(session, "\temac_mask                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 9, r);
	bdmf_session_print(session, "\ttx_queues_status         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 10, r);
	bdmf_session_print(session, "\tgpio_flow_control_vector_ptr= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 12, r);
	bdmf_session_print(session, "\tpacket_counters_ptr_0    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 14, r);
	bdmf_session_print(session, "\treserved2                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 15, r);
	bdmf_session_print(session, "\tbbh_destination_0        = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 16, r);
	bdmf_session_print(session, "\tpacket_counters_ptr_1    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 18, r);
	bdmf_session_print(session, "\treserved3                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 19, r);
	bdmf_session_print(session, "\tbbh_destination_1        = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 20, r);
	bdmf_session_print(session, "\tpacket_counters_ptr_2    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 22, r);
	bdmf_session_print(session, "\treserved4                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 23, r);
	bdmf_session_print(session, "\tbbh_destination_2        = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 24, r);
	bdmf_session_print(session, "\tegress_port              = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_32((uint8_t *)p + 24, 0, 24, r);
	bdmf_session_print(session, "\treserved5                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 28, r);
	bdmf_session_print(session, "\treserved6                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_DHD_COMPLETE_RING_DESCRIPTOR(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register DHD_COMPLETE_RING_DESCRIPTOR\n");

	MREAD_32((uint8_t *)p, r);
	bdmf_session_print(session, "\tring_ptr                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 4, r);
	bdmf_session_print(session, "\tring_size                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 8, r);
	bdmf_session_print(session, "\tring_base                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 12, r);
	bdmf_session_print(session, "\tring_end                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_GRE_RUNNER_FLOW_HEADER_BUFFER(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	int i,j;

	bdmf_session_print(session, "  Register GRE_RUNNER_FLOW_HEADER_BUFFER\n");

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

void dump_RDD_DS_WAN_UDP_FILTER_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register DS_WAN_UDP_FILTER_ENTRY\n");

	MREAD_32((uint8_t *)p, r);
	bdmf_session_print(session, "\toffset                   = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 4, r);
	bdmf_session_print(session, "\tvalue                    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 8, r);
	bdmf_session_print(session, "\tmask                     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 12, r);
	bdmf_session_print(session, "\thits                     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_FC_MCAST_PORT_HEADER_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register FC_MCAST_PORT_HEADER_ENTRY\n");

	MREAD_8((uint8_t *)p, r);
	bdmf_session_print(session, "\tu8                       = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_CONNECTION_CONTEXT_MULTICAST_BUFFER_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	int i,j;

	bdmf_session_print(session, "  Register CONNECTION_CONTEXT_MULTICAST_BUFFER_ENTRY\n");

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

#if defined DSL_63138
void dump_RDD_BBH_RX_DESCRIPTOR(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register BBH_RX_DESCRIPTOR\n");

	FIELD_MREAD_16((uint8_t *)p, 6, 10, r);
	bdmf_session_print(session, "\tlast_sbn                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 1, 5, 1, r);
	bdmf_session_print(session, "\tfstat_cell               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_32((uint8_t *)p + 0, 14, 7, r);
	bdmf_session_print(session, "\tflow_id                  = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p + 2, 0, 14, r);
	bdmf_session_print(session, "\tpacket_length            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 4, 7, 1, r);
	bdmf_session_print(session, "\terror                    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 4, 6, 1, r);
	bdmf_session_print(session, "\treserved0                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 4, 5, 1, r);
	bdmf_session_print(session, "\tfstat_error              = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p + 4, 6, 7, r);
	bdmf_session_print(session, "\terror_type               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 5, 0, 6, r);
	bdmf_session_print(session, "\tih_buffer_number         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 6, 7, 1, r);
	bdmf_session_print(session, "\ttarget_memory            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p + 6, 0, 15, r);
	bdmf_session_print(session, "\tbuffer_number            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

#endif

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

	FIELD_MREAD_8((uint8_t *)p + 2, 1, 4, r);
	bdmf_session_print(session, "\tcommand_list_length_64   = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 2, 0, 1, r);
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
	bdmf_session_print(session, "\tis_wred_high_prio        = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p, 2, 1, r);
	bdmf_session_print(session, "\tgso                      = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p, 6, 4, r);
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

	FIELD_MREAD_8((uint8_t *)p + 4, 6, 1, r);
	bdmf_session_print(session, "\treserved1                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 4, 4, 2, r);
	bdmf_session_print(session, "\tlag_port_pti             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p + 4, 7, 5, r);
	bdmf_session_print(session, "\tmessage_parameter        = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 5, 0, 7, r);
	bdmf_session_print(session, "\tpayload_offset           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 6, r);
	bdmf_session_print(session, "\treserved3                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

#if defined DSL_63138
void dump_RDD_CPU_TX_DESCRIPTOR_BPM(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register CPU_TX_DESCRIPTOR_BPM\n");

	MREAD_32((uint8_t *)p, r);
	bdmf_session_print(session, "\treserved0                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_32((uint8_t *)p + 4, 15, 17, r);
	bdmf_session_print(session, "\treserved1                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p + 6, 0, 15, r);
	bdmf_session_print(session, "\tbuffer_number            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

#endif

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

	FIELD_MREAD_8((uint8_t *)p, 3, 1, r);
	bdmf_session_print(session, "\tis_wred_high_prio        = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p, 3, 8, r);
	bdmf_session_print(session, "\tupstream_gem_flow        = 0x%08x", (unsigned int)r);
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

void dump_RDD_DHD_TX_POST_DESCRIPTOR(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register DHD_TX_POST_DESCRIPTOR\n");

	MREAD_8((uint8_t *)p, r);
	bdmf_session_print(session, "\tmsg_type                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 1, r);
	bdmf_session_print(session, "\tif_id                    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 2, r);
	bdmf_session_print(session, "\tcommon_hdr_flags         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 3, r);
	bdmf_session_print(session, "\tepoch                    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 4, r);
	bdmf_session_print(session, "\trequest_id               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 8, r);
	bdmf_session_print(session, "\ttx_eth_hdr_0             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 12, r);
	bdmf_session_print(session, "\ttx_eth_hdr_1             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 16, r);
	bdmf_session_print(session, "\ttx_eth_hdr_2             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 20, r);
	bdmf_session_print(session, "\ttx_eth_hdr_3             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 22, r);
	bdmf_session_print(session, "\tflags                    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 23, r);
	bdmf_session_print(session, "\tseg_cnt                  = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 24, r);
	bdmf_session_print(session, "\tmetadata_buf_addr_low    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 28, r);
	bdmf_session_print(session, "\tmetadata_buf_addr_hi     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 32, r);
	bdmf_session_print(session, "\tdata_buf_addr_low        = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 36, r);
	bdmf_session_print(session, "\tdata_buf_addr_hi         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 40, r);
	bdmf_session_print(session, "\tmeta_buf_len             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 42, r);
	bdmf_session_print(session, "\tdata_len                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 44, r);
	bdmf_session_print(session, "\treserved                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_DHD_TX_POST_DESCRIPTOR_CWI32(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register DHD_TX_POST_DESCRIPTOR_CWI32\n");

	MREAD_32((uint8_t *)p, r);
	bdmf_session_print(session, "\trequest_id               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 4, 5, 3, r);
	bdmf_session_print(session, "\tprio                     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 4, 0, 5, r);
	bdmf_session_print(session, "\tif_id                    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 5, 1, 7, r);
	bdmf_session_print(session, "\tflags                    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 5, 0, 1, r);
	bdmf_session_print(session, "\tcopy                     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 6, r);
	bdmf_session_print(session, "\tdata_len                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 8, r);
	bdmf_session_print(session, "\tdata_buf_addr_low        = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 12, r);
	bdmf_session_print(session, "\tdata_buf_addr_high       = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 16, r);
	bdmf_session_print(session, "\ttx_eth_hdr_0             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 20, r);
	bdmf_session_print(session, "\ttx_eth_hdr_1             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 24, r);
	bdmf_session_print(session, "\ttx_eth_hdr_2             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 28, r);
	bdmf_session_print(session, "\ttx_eth_hdr_3             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p + 30, 4, 12, r);
	bdmf_session_print(session, "\tflowid_override          = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 31, 0, 4, r);
	bdmf_session_print(session, "\tinfo                     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_MULTICAST_HEADER_BUFFER(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	int i,j;

	bdmf_session_print(session, "  Register MULTICAST_HEADER_BUFFER\n");

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

void dump_RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register INGRESS_FILTERS_CONFIGURATION_ENTRY\n");

	MREAD_32((uint8_t *)p, r);
	bdmf_session_print(session, "\tingress_filters          = 0x%08x", (unsigned int)r);
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

void dump_RDD_CPU_TX_DESCRIPTOR_IPSEC(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register CPU_TX_DESCRIPTOR_IPSEC\n");

	FIELD_MREAD_8((uint8_t *)p, 7, 1, r);
	bdmf_session_print(session, "\tvalid                    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p, 4, 3, r);
	bdmf_session_print(session, "\tcommand                  = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p, 3, 1, r);
	bdmf_session_print(session, "\treserved0                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p, 2, 1, r);
	bdmf_session_print(session, "\tipsec                    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p, 6, 4, r);
	bdmf_session_print(session, "\tcpu_rx_queue             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 1, 5, 1, r);
	bdmf_session_print(session, "\tupstream                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 1, 4, 1, r);
	bdmf_session_print(session, "\tsa_update                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_32((uint8_t *)p + 0, 14, 6, r);
	bdmf_session_print(session, "\tsa_index                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p + 2, 0, 14, r);
	bdmf_session_print(session, "\tpacket_length            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 4, 7, 1, r);
	bdmf_session_print(session, "\tabs_flag                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 4, 1, 6, r);
	bdmf_session_print(session, "\treserved1                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p + 4, 6, 3, r);
	bdmf_session_print(session, "\terror                    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 5, 0, 6, r);
	bdmf_session_print(session, "\tesphdr_offset            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 6, 7, 1, r);
	bdmf_session_print(session, "\treserved2                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p + 6, 0, 15, r);
	bdmf_session_print(session, "\tabs_address_index        = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_CPU_RX_DESCRIPTOR_IPSEC(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register CPU_RX_DESCRIPTOR_IPSEC\n");

	FIELD_MREAD_8((uint8_t *)p, 7, 1, r);
	bdmf_session_print(session, "\treserved0                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p, 4, 3, r);
	bdmf_session_print(session, "\terror                    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p, 2, 2, r);
	bdmf_session_print(session, "\treserved1                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p, 6, 4, r);
	bdmf_session_print(session, "\tcpu_rx_queue             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 1, 5, 1, r);
	bdmf_session_print(session, "\tupstream                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_32((uint8_t *)p + 0, 14, 7, r);
	bdmf_session_print(session, "\treserved2                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p + 2, 0, 14, r);
	bdmf_session_print(session, "\tpacket_length            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 4, r);
	bdmf_session_print(session, "\treserved3                = 0x%08x", (unsigned int)r);
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

void dump_RDD_PICO_RUNNER_GLOBAL_REGISTERS_INIT_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register PICO_RUNNER_GLOBAL_REGISTERS_INIT_ENTRY\n");

	MREAD_32((uint8_t *)p, r);
	bdmf_session_print(session, "\treserved_fw_only         = 0x%08x", (unsigned int)r);
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

void dump_RDD_INGRESS_QUEUE_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register INGRESS_QUEUE_ENTRY\n");

	MREAD_8((uint8_t *)p, r);
	bdmf_session_print(session, "\treserved_fw_only         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_GPE_COMMAND_PRIMITIVE_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register GPE_COMMAND_PRIMITIVE_ENTRY\n");

	MREAD_16((uint8_t *)p, r);
	bdmf_session_print(session, "\tprimitive_address        = 0x%08x", (unsigned int)r);
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

	FIELD_MREAD_8((uint8_t *)p + 4, 6, 1, r);
	bdmf_session_print(session, "\tssid_multicast           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 4, 2, 4, r);
	bdmf_session_print(session, "\tssid                     = 0x%08x", (unsigned int)r);
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

void dump_RDD_CPU_TX_UPDATE_PD_POOL_QUOTA_MESSAGE_DESCRIPTOR(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register CPU_TX_UPDATE_PD_POOL_QUOTA_MESSAGE_DESCRIPTOR\n");

	MREAD_32((uint8_t *)p, r);
	bdmf_session_print(session, "\treserved0                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 4, 7, 1, r);
	bdmf_session_print(session, "\tguaranteed_free_count_incr= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p + 4, 0, 15, r);
	bdmf_session_print(session, "\tguaranteed_free_count_delta= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 6, r);
	bdmf_session_print(session, "\treserved1                = 0x%08x", (unsigned int)r);
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

void dump_RDD_DHD_COMPLETE_RING_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register DHD_COMPLETE_RING_ENTRY\n");

	MREAD_32((uint8_t *)p, r);
	bdmf_session_print(session, "\tring_value               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 4, r);
	bdmf_session_print(session, "\tstatus                   = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 6, r);
	bdmf_session_print(session, "\tflow_ring_id             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_BYTE_1(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register BYTE_1\n");

	MREAD_8((uint8_t *)p, r);
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

	MREAD_8((uint8_t *)p + 2, r);
	bdmf_session_print(session, "\tring_id                  = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 3, r);
	bdmf_session_print(session, "\twifi_queue               = 0x%08x", (unsigned int)r);
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

	MREAD_32((uint8_t *)p + 8, r);
	bdmf_session_print(session, "\tbbh_descriptor_0         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 12, r);
	bdmf_session_print(session, "\tbbh_descriptor_1         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 16, r);
	bdmf_session_print(session, "\tchain_id                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 18, r);
	bdmf_session_print(session, "\treserved                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_WLAN_MCAST_SSID_STATS_STATE_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register WLAN_MCAST_SSID_STATS_STATE_ENTRY\n");

	MREAD_16((uint8_t *)p, r);
	bdmf_session_print(session, "\twfd_0_ssid_state_vector  = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 2, r);
	bdmf_session_print(session, "\twfd_1_ssid_state_vector  = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 4, r);
	bdmf_session_print(session, "\twfd_2_ssid_state_vector  = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_TIMER_CONTROL_DESCRIPTOR(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register TIMER_CONTROL_DESCRIPTOR\n");

	MREAD_16((uint8_t *)p, r);
	bdmf_session_print(session, "\tnumber_of_active_tasks   = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_FC_MCAST_CONNECTION_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register FC_MCAST_CONNECTION_ENTRY\n");

	FIELD_MREAD_8((uint8_t *)p, 7, 1, r);
	bdmf_session_print(session, "\tvalid                    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p, 0, 7, r);
	bdmf_session_print(session, "\tvlan_head_index          = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 1, r);
	bdmf_session_print(session, "\treserved1                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 2, 5, 3, r);
	bdmf_session_print(session, "\tbucket_overflow_counter  = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 2, 1, 4, r);
	bdmf_session_print(session, "\tcommand_list_length_64   = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 2, 0, 1, r);
	bdmf_session_print(session, "\tkey_extend               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 3, 7, 1, r);
	bdmf_session_print(session, "\tis_multicast             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 3, 0, 7, r);
	bdmf_session_print(session, "\tprotocol                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 4, 6, 2, r);
	bdmf_session_print(session, "\tnumber_of_tags           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_32((uint8_t *)p + 4, 0, 30, r);
	bdmf_session_print(session, "\treserved2                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 8, r);
	bdmf_session_print(session, "\tsrc_ip                   = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 12, r);
	bdmf_session_print(session, "\tdst_ip                   = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_FC_MCAST_PORT_CONTEXT_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register FC_MCAST_PORT_CONTEXT_ENTRY\n");

	FIELD_MREAD_8((uint8_t *)p, 7, 1, r);
	bdmf_session_print(session, "\tstate                    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p, 0, 7, r);
	bdmf_session_print(session, "\treserved2                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 1, 6, 2, r);
	bdmf_session_print(session, "\tlag_port                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 1, 5, 1, r);
	bdmf_session_print(session, "\tis_wred_high_prio        = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 1, 0, 5, r);
	bdmf_session_print(session, "\tl2_header_length         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 2, 7, 1, r);
	bdmf_session_print(session, "\tl2_push                  = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 2, 0, 7, r);
	bdmf_session_print(session, "\tl2_command_list_length   = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 3, 5, 3, r);
	bdmf_session_print(session, "\tqueue                    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 3, 0, 5, r);
	bdmf_session_print(session, "\tl2_offset                = 0x%08x", (unsigned int)r);
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

void dump_RDD_PARALLEL_PROCESSING_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register PARALLEL_PROCESSING_ENTRY\n");

	MREAD_8((uint8_t *)p, r);
	bdmf_session_print(session, "\tih_buffer                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_DHD_L2_HEADER_BUFFER_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	int i,j;

	bdmf_session_print(session, "  Register DHD_L2_HEADER_BUFFER_ENTRY\n");

	bdmf_session_print(session, "\tdhd_l2_buffer_data       =\n\t");
	for (i=0,j=0; i<32; i++)
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
	bdmf_session_print(session, "\tguaranteed_free_count    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 6, r);
	bdmf_session_print(session, "\tnon_guaranteed_free_count= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 8, r);
	bdmf_session_print(session, "\tguaranteed_threshold     = 0x%08x", (unsigned int)r);
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

void dump_RDD_PARALLEL_PROCESSING_TASK_REORDER_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register PARALLEL_PROCESSING_TASK_REORDER_ENTRY\n");

	MREAD_8((uint8_t *)p, r);
	bdmf_session_print(session, "\treserved_fw_only         = 0x%08x", (unsigned int)r);
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

void dump_RDD_GSO_CONTEXT_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register GSO_CONTEXT_ENTRY\n");

	MREAD_32((uint8_t *)p, r);
	bdmf_session_print(session, "\trx_bbh_descriptor_0      = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 4, r);
	bdmf_session_print(session, "\trx_bbh_descriptor_1      = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 8, r);
	bdmf_session_print(session, "\ttx_bbh_descriptor_0      = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 12, r);
	bdmf_session_print(session, "\ttx_bbh_descriptor_1      = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 16, r);
	bdmf_session_print(session, "\trx_packets               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 20, r);
	bdmf_session_print(session, "\trx_octets                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 24, r);
	bdmf_session_print(session, "\ttx_packets               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 28, r);
	bdmf_session_print(session, "\ttx_octets                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 32, r);
	bdmf_session_print(session, "\tdropped_packets          = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 36, r);
	bdmf_session_print(session, "\tdropped_no_bpm_buffer    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 38, r);
	bdmf_session_print(session, "\tdropped_parse_failed     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 40, r);
	bdmf_session_print(session, "\tdropped_linear_length_invalid= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 42, r);
	bdmf_session_print(session, "\tqueue_full               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 44, r);
	bdmf_session_print(session, "\tsummary                  = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 48, r);
	bdmf_session_print(session, "\tip_header_offset         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 49, r);
	bdmf_session_print(session, "\tip_header_length         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 50, r);
	bdmf_session_print(session, "\tip_total_length          = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 52, r);
	bdmf_session_print(session, "\tip_id                    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 54, 7, 1, r);
	bdmf_session_print(session, "\tip_fragment_flags_reserved= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 54, 6, 1, r);
	bdmf_session_print(session, "\tip_fragment_flags_df     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 54, 5, 1, r);
	bdmf_session_print(session, "\tip_fragment_flags_mf     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p + 54, 0, 13, r);
	bdmf_session_print(session, "\tip_fragment_offset       = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 56, r);
	bdmf_session_print(session, "\tip_flags                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 57, r);
	bdmf_session_print(session, "\tip_protocol              = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 58, r);
	bdmf_session_print(session, "\tipv4_csum                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 60, r);
	bdmf_session_print(session, "\tpacket_header_length     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 61, r);
	bdmf_session_print(session, "\tseg_count                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 62, r);
	bdmf_session_print(session, "\tnr_frags                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 63, r);
	bdmf_session_print(session, "\tfrag_index               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 64, r);
	bdmf_session_print(session, "\ttcp_udp_header_offset    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 65, r);
	bdmf_session_print(session, "\ttcp_udp_header_length    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 66, r);
	bdmf_session_print(session, "\ttcp_udp_total_length     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 68, r);
	bdmf_session_print(session, "\ttcp_sequence             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 72, r);
	bdmf_session_print(session, "\ttcp_flags                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 73, r);
	bdmf_session_print(session, "\tversion                  = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 74, r);
	bdmf_session_print(session, "\ttcp_udp_csum             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 76, r);
	bdmf_session_print(session, "\tmss                      = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 78, r);
	bdmf_session_print(session, "\tmss_adjust               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 80, r);
	bdmf_session_print(session, "\tseg_length               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 82, r);
	bdmf_session_print(session, "\tseg_bytes_left           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 84, r);
	bdmf_session_print(session, "\tmax_chunk_length         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 85, r);
	bdmf_session_print(session, "\tchunk_bytes_left         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 86, r);
	bdmf_session_print(session, "\tpayload_bytes_left       = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 88, r);
	bdmf_session_print(session, "\tpayload_ptr              = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 92, r);
	bdmf_session_print(session, "\tpayload_length           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 94, r);
	bdmf_session_print(session, "\tlinear_length            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 96, r);
	bdmf_session_print(session, "\ttx_packet_ptr            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 100, r);
	bdmf_session_print(session, "\ttx_packet_length         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 102, r);
	bdmf_session_print(session, "\tudp_first_packet_length  = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 104, r);
	bdmf_session_print(session, "\tudp_first_packet_ptr     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 108, r);
	bdmf_session_print(session, "\tudp_first_packet_buffer_number= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 112, r);
	bdmf_session_print(session, "\tbpm_buffer_number        = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 116, r);
	bdmf_session_print(session, "\tpacket_length            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 118, r);
	bdmf_session_print(session, "\treserved0                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 120, r);
	bdmf_session_print(session, "\tipv6_ip_id               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 124, r);
	bdmf_session_print(session, "\tauth_state_3             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 128, r);
	bdmf_session_print(session, "\tdebug_0                  = 0x%08x", (unsigned int)r);
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

void dump_RDD_BROADCOM_SWITCH_PORT_MAPPING(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register BROADCOM_SWITCH_PORT_MAPPING\n");

	MREAD_8((uint8_t *)p, r);
	bdmf_session_print(session, "\tphysical_port            = 0x%08x", (unsigned int)r);
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

void dump_RDD_ENQUEUE_PCI_PACKET_CONTEXT_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register ENQUEUE_PCI_PACKET_CONTEXT_ENTRY\n");

	MREAD_16((uint8_t *)p, r);
	bdmf_session_print(session, "\tdhd_context_ptr          = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 2, r);
	bdmf_session_print(session, "\tdhd_host_buf_ptr         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 4, r);
	bdmf_session_print(session, "\tdhd_l2_buf               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 6, r);
	bdmf_session_print(session, "\tret_addr                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_GSO_DESC_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	int i,j;

	bdmf_session_print(session, "  Register GSO_DESC_ENTRY\n");

	MREAD_32((uint8_t *)p, r);
	bdmf_session_print(session, "\tdata                     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 4, r);
	bdmf_session_print(session, "\tlen                      = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 6, r);
	bdmf_session_print(session, "\tlinear_len               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 8, r);
	bdmf_session_print(session, "\tmss                      = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 10, 1, 7, r);
	bdmf_session_print(session, "\treserved0                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 10, 0, 1, r);
	bdmf_session_print(session, "\tis_allocated             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 11, r);
	bdmf_session_print(session, "\tnr_frags                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 12, r);
	bdmf_session_print(session, "\treserved1                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	bdmf_session_print(session, "\tfrag_data                =\n\t");
	for (i=0,j=0; i<18; i++)
	{
		MREAD_I_32((uint8_t *)p + 16, i, r);
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
	for (i=0,j=0; i<18; i++)
	{
		FIELD_MREAD_I_32((uint8_t *)p + 88, 8, 16, i, r);
		bdmf_session_print(session, "0x%08x ", (unsigned int)r);
		j++;
		if (j >= 8)
		{
			j = 0;
			bdmf_session_print(session, "\n\t");
		}
	}
	bdmf_session_print(session, "\n");
	MREAD_32((uint8_t *)p + 124, r);
	bdmf_session_print(session, "\treserved2                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_SYSTEM_CONFIGURATION(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register SYSTEM_CONFIGURATION\n");

	MREAD_16((uint8_t *)p, r);
	bdmf_session_print(session, "\tmtu_minus_40             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 2, r);
	bdmf_session_print(session, "\tus_padding_max_size      = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 4, r);
	bdmf_session_print(session, "\tus_padding_cpu_max_size  = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 6, r);
	bdmf_session_print(session, "\tdrop_precedence_eligibility_vector= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 8, r);
	bdmf_session_print(session, "\tegress_ether_type_1      = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 10, r);
	bdmf_session_print(session, "\tegress_ether_type_2      = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 12, r);
	bdmf_session_print(session, "\tegress_ether_type_3      = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 14, r);
	bdmf_session_print(session, "\ttimer_scheduler_period   = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 16, r);
	bdmf_session_print(session, "\tactive_policers_vector   = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 18, r);
	bdmf_session_print(session, "\tpolicers_period          = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 20, r);
	bdmf_session_print(session, "\tds_rate_shaper_timer     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 22, r);
	bdmf_session_print(session, "\tus_rate_controller_timer = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 24, r);
	bdmf_session_print(session, "\tus_rate_limit_sustain_budget_limit= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 25, r);
	bdmf_session_print(session, "\thash_based_forwarding_port_count= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 26, r);
	bdmf_session_print(session, "\tinter_lan_scheduling_mode= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 27, r);
	bdmf_session_print(session, "\tbroadcom_switch_port     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 28, r);
	bdmf_session_print(session, "\tmirroring_port           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 29, r);
	bdmf_session_print(session, "\tds_connection_miss_action= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 30, r);
	bdmf_session_print(session, "\tus_pci_flow_cache_enable = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 31, r);
	bdmf_session_print(session, "\tglobal_ingress_config    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 32, r);
	bdmf_session_print(session, "\tpci_ls_dp_eligibility_vector= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 34, r);
	bdmf_session_print(session, "\tds_ingress_policers_mode = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 35, r);
	bdmf_session_print(session, "\tdebug_mode               = 0x%08x", (unsigned int)r);
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

void dump_RDD_IPSEC_SA_DESC_CAM(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register IPSEC_SA_DESC_CAM\n");

	MREAD_16((uint8_t *)p, r);
	bdmf_session_print(session, "\tsa_desc_idx              = 0x%08x", (unsigned int)r);
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

void dump_RDD_DHD_TX_POST_CONTEXT_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register DHD_TX_POST_CONTEXT_ENTRY\n");

	MREAD_32((uint8_t *)p, r);
	bdmf_session_print(session, "\trequest_id               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 4, r);
	bdmf_session_print(session, "\tdata_buf_ptr             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 8, r);
	bdmf_session_print(session, "\tdata_buf_len             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 10, r);
	bdmf_session_print(session, "\tl2_buffer                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 12, 6, 2, r);
	bdmf_session_print(session, "\tradio_idx                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 12, 2, 4, r);
	bdmf_session_print(session, "\twifi_ssid                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p + 12, 0, 10, r);
	bdmf_session_print(session, "\tflow_ring_id             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 14, 1, 7, r);
	bdmf_session_print(session, "\treserved1                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 14, 0, 1, r);
	bdmf_session_print(session, "\tspd_svc_flag             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 15, 3, 5, r);
	bdmf_session_print(session, "\treserved2                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 15, 0, 3, r);
	bdmf_session_print(session, "\teth_tx_queue             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_ETH_TX_EMACS_STATUS_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register ETH_TX_EMACS_STATUS_ENTRY\n");

	MREAD_8((uint8_t *)p, r);
	bdmf_session_print(session, "\tstatus_vector            = 0x%08x", (unsigned int)r);
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

void dump_RDD_HASH_BASED_FORWARDING_PORT_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register HASH_BASED_FORWARDING_PORT_ENTRY\n");

	MREAD_8((uint8_t *)p, r);
	bdmf_session_print(session, "\tegress_port              = 0x%08x", (unsigned int)r);
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

void dump_RDD_PARALLEL_PROCESSING_IH_BUFFER_PTR(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register PARALLEL_PROCESSING_IH_BUFFER_PTR\n");

	MREAD_16((uint8_t *)p, r);
	bdmf_session_print(session, "\treserved_fw_only         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_ANY_SRC_PORT_FLOW_COUNTER(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register ANY_SRC_PORT_FLOW_COUNTER\n");

	MREAD_16((uint8_t *)p, r);
	bdmf_session_print(session, "\treserved_fw_only         = 0x%08x", (unsigned int)r);
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

void dump_RDD_RUNNER_CONGESTION_STATE_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register RUNNER_CONGESTION_STATE_ENTRY\n");

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

void dump_RDD_ETH_TX_INTER_LAN_SCHEDULING_OFFSET_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register ETH_TX_INTER_LAN_SCHEDULING_OFFSET_ENTRY\n");

	MREAD_8((uint8_t *)p, r);
	bdmf_session_print(session, "\temac_offset              = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_FC_GLOBAL_CFG_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register FC_GLOBAL_CFG_ENTRY\n");

	FIELD_MREAD_8((uint8_t *)p, 2, 6, r);
	bdmf_session_print(session, "\treserved                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p, 1, 1, r);
	bdmf_session_print(session, "\tfc_tcp_ack_mflows        = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p, 0, 1, r);
	bdmf_session_print(session, "\tfc_accel_mode            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

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

void dump_RDD_CSO_BUFFER_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	int i,j;

	bdmf_session_print(session, "  Register CSO_BUFFER_ENTRY\n");

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

void dump_RDD_CSO_PSEUDO_HEADER_BUFFER_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	int i,j;

	bdmf_session_print(session, "  Register CSO_PSEUDO_HEADER_BUFFER_ENTRY\n");

	bdmf_session_print(session, "\treserved_fw_only         =\n\t");
	for (i=0,j=0; i<10; i++)
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

void dump_RDD_WAN_TX_SERVICE_QUEUE_SCHEDULER_DESCRIPTOR(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	int i,j;

	bdmf_session_print(session, "  Register WAN_TX_SERVICE_QUEUE_SCHEDULER_DESCRIPTOR\n");

	MREAD_32((uint8_t *)p, r);
	bdmf_session_print(session, "\tqueues_status            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 4, r);
	bdmf_session_print(session, "\trate_limiter_status      = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 8, r);
	bdmf_session_print(session, "\tsustain_vector           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 12, r);
	bdmf_session_print(session, "\tpeak_vector              = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	bdmf_session_print(session, "\ttx_queue_addr            =\n\t");
	for (i=0,j=0; i<32; i++)
	{
		MREAD_I_16((uint8_t *)p + 16, i, r);
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
	for (i=0,j=0; i<12; i++)
	{
		MREAD_I_32((uint8_t *)p + 80, i, r);
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

void dump_RDD_US_WAN_FLOW_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register US_WAN_FLOW_ENTRY\n");

	FIELD_MREAD_8((uint8_t *)p, 5, 3, r);
	bdmf_session_print(session, "\treserved0                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p, 2, 3, r);
	bdmf_session_print(session, "\thdr_type                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p, 1, 1, r);
	bdmf_session_print(session, "\tptm_bonding              = 0x%08x", (unsigned int)r);
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
	bdmf_session_print(session, "\twan_port_id_or_fstat     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

#if defined DSL_63138
void dump_RDD_DSL_PTM_BOND_TX_HDR_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register DSL_PTM_BOND_TX_HDR_ENTRY\n");

	FIELD_MREAD_8((uint8_t *)p, 7, 1, r);
	bdmf_session_print(session, "\tport_sel                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p, 5, 2, r);
	bdmf_session_print(session, "\treserved0                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p, 4, 1, r);
	bdmf_session_print(session, "\tpkt_eop                  = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p, 0, 12, r);
	bdmf_session_print(session, "\tfrag_size                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

#endif

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

	FIELD_MREAD_8((uint8_t *)p + 1, 0, 1, r);
	bdmf_session_print(session, "\tqos_mapping_mode         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 2, r);
	bdmf_session_print(session, "\twan_flow                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 3, 5, 3, r);
	bdmf_session_print(session, "\ttraffic_class            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 3, 3, 2, r);
	bdmf_session_print(session, "\treserved1                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 3, 0, 3, r);
	bdmf_session_print(session, "\ttrap_reason              = 0x%08x", (unsigned int)r);
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

	FIELD_MREAD_16((uint8_t *)p + 4, 7, 2, r);
	bdmf_session_print(session, "\treserved2                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 5, 2, 5, r);
	bdmf_session_print(session, "\trate_controller          = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 5, 1, 1, r);
	bdmf_session_print(session, "\tdei_remark_enable        = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 5, 0, 1, r);
	bdmf_session_print(session, "\tdei_value                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 6, 7, 1, r);
	bdmf_session_print(session, "\tic_ip_flow               = 0x%08x", (unsigned int)r);
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

void dump_RDD_DHD_RX_POST_DESCRIPTOR(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register DHD_RX_POST_DESCRIPTOR\n");

	MREAD_8((uint8_t *)p, r);
	bdmf_session_print(session, "\tmsg_type                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 1, r);
	bdmf_session_print(session, "\tif_id                    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 2, r);
	bdmf_session_print(session, "\tcommon_hdr_flags         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 3, r);
	bdmf_session_print(session, "\tepoch                    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 4, r);
	bdmf_session_print(session, "\trequest_id               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 8, r);
	bdmf_session_print(session, "\tmeta_buf_len             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 10, r);
	bdmf_session_print(session, "\tdata_len                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 12, r);
	bdmf_session_print(session, "\treserved                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 16, r);
	bdmf_session_print(session, "\tmetadata_buf_addr_low    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 20, r);
	bdmf_session_print(session, "\tmetadata_buf_addr_hi     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 24, r);
	bdmf_session_print(session, "\tdata_buf_addr_low        = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 28, r);
	bdmf_session_print(session, "\tdata_buf_addr_hi         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_DHD_RX_COMPLETE_DESCRIPTOR(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register DHD_RX_COMPLETE_DESCRIPTOR\n");

	MREAD_8((uint8_t *)p, r);
	bdmf_session_print(session, "\tmsg_type                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 1, r);
	bdmf_session_print(session, "\tif_id                    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 2, r);
	bdmf_session_print(session, "\tcommon_hdr_flags         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 3, r);
	bdmf_session_print(session, "\tepoch                    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 4, r);
	bdmf_session_print(session, "\trequest_id               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 8, r);
	bdmf_session_print(session, "\tcompl_msg_hdr_status     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 10, r);
	bdmf_session_print(session, "\tflow_ring_id             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 12, r);
	bdmf_session_print(session, "\tmeta_buf_len             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 14, r);
	bdmf_session_print(session, "\tdata_len                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 16, r);
	bdmf_session_print(session, "\tdata_offset              = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 18, r);
	bdmf_session_print(session, "\tflags                    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 20, r);
	bdmf_session_print(session, "\trx_status_0              = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 24, r);
	bdmf_session_print(session, "\trx_status_1              = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 28, r);
	bdmf_session_print(session, "\tdma_done_mark            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_DHD_RX_POST_DESCRIPTOR_CWI32(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register DHD_RX_POST_DESCRIPTOR_CWI32\n");

	MREAD_32((uint8_t *)p, r);
	bdmf_session_print(session, "\trequest_id               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 4, r);
	bdmf_session_print(session, "\tdata_buf_addr_low        = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_DHD_RX_COMPLETE_DESCRIPTOR_CWI32(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register DHD_RX_COMPLETE_DESCRIPTOR_CWI32\n");

	FIELD_MREAD_8((uint8_t *)p, 5, 3, r);
	bdmf_session_print(session, "\tflags                    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p, 0, 5, r);
	bdmf_session_print(session, "\tif_id                    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 1, r);
	bdmf_session_print(session, "\tdata_offset              = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 2, r);
	bdmf_session_print(session, "\tdata_len                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 4, r);
	bdmf_session_print(session, "\trequest_id               = 0x%08x", (unsigned int)r);
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

void dump_RDD_GPON_ABSOLUTE_TX_COUNTER(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register GPON_ABSOLUTE_TX_COUNTER\n");

	MREAD_8((uint8_t *)p, r);
	bdmf_session_print(session, "\treserved_fw_only         = 0x%08x", (unsigned int)r);
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

void dump_RDD_CSO_CONTEXT_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register CSO_CONTEXT_ENTRY\n");

	MREAD_32((uint8_t *)p, r);
	bdmf_session_print(session, "\tsummary                  = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 4, r);
	bdmf_session_print(session, "\tlinear_length            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 6, r);
	bdmf_session_print(session, "\tpacket_length            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 8, r);
	bdmf_session_print(session, "\tpacket_header_length     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 9, r);
	bdmf_session_print(session, "\tip_protocol              = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 10, r);
	bdmf_session_print(session, "\tip_header_offset         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 11, r);
	bdmf_session_print(session, "\tip_header_length         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 12, r);
	bdmf_session_print(session, "\tip_total_length          = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 14, r);
	bdmf_session_print(session, "\tipv4_csum                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 16, r);
	bdmf_session_print(session, "\ttcp_udp_header_offset    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 17, r);
	bdmf_session_print(session, "\ttcp_udp_header_length    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 18, r);
	bdmf_session_print(session, "\ttcp_udp_total_length     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 20, r);
	bdmf_session_print(session, "\ttcp_udp_csum             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 22, r);
	bdmf_session_print(session, "\tmax_chunk_length         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 23, r);
	bdmf_session_print(session, "\tchunk_bytes_left         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 24, r);
	bdmf_session_print(session, "\tnr_frags                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 25, r);
	bdmf_session_print(session, "\tfrag_index               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 26, r);
	bdmf_session_print(session, "\tfrag_len                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 28, r);
	bdmf_session_print(session, "\tfrag_data                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 32, r);
	bdmf_session_print(session, "\tgood_csum_packets        = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 36, r);
	bdmf_session_print(session, "\tno_csum_packets          = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 40, r);
	bdmf_session_print(session, "\tbad_ipv4_hdr_csum_packets= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 44, r);
	bdmf_session_print(session, "\tbad_tcp_udp_csum_packets = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 48, r);
	bdmf_session_print(session, "\tfail_code                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 49, r);
	bdmf_session_print(session, "\tdma_sync                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 50, r);
	bdmf_session_print(session, "\tseg_length               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 52, r);
	bdmf_session_print(session, "\tseg_bytes_left           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 54, r);
	bdmf_session_print(session, "\tpayload_length           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 56, r);
	bdmf_session_print(session, "\tpayload_bytes_left       = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 58, r);
	bdmf_session_print(session, "\treserved_0               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 60, r);
	bdmf_session_print(session, "\tpayload_ptr              = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_32((uint8_t *)p + 64, 8, 24, r);
	bdmf_session_print(session, "\treserved_1               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 67, r);
	bdmf_session_print(session, "\tddr_payload_offset       = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 68, r);
	bdmf_session_print(session, "\tddr_src_address          = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 72, r);
	bdmf_session_print(session, "\tsaved_ih_buffer_number   = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 76, r);
	bdmf_session_print(session, "\tsaved_csum32_ret_addr    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 80, r);
	bdmf_session_print(session, "\tsaved_r16                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_US_FREE_PACKET_DESCRIPTORS_POOL_DESCRIPTOR_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register US_FREE_PACKET_DESCRIPTORS_POOL_DESCRIPTOR_ENTRY\n");

	MREAD_16((uint8_t *)p, r);
	bdmf_session_print(session, "\tguaranteed_free_count    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 2, r);
	bdmf_session_print(session, "\tnon_guaranteed_free_count= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 4, r);
	bdmf_session_print(session, "\tguaranteed_threshold     = 0x%08x", (unsigned int)r);
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

void dump_RDD_RATE_CONTROLLER_EXPONENT_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register RATE_CONTROLLER_EXPONENT_ENTRY\n");

	MREAD_8((uint8_t *)p, r);
	bdmf_session_print(session, "\texponent                 = 0x%08x", (unsigned int)r);
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

void dump_RDD_WAN_ENQUEUE_INGRESS_QUEUE_PTR_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register WAN_ENQUEUE_INGRESS_QUEUE_PTR_ENTRY\n");

	MREAD_16((uint8_t *)p, r);
	bdmf_session_print(session, "\treserved_fw_only         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_BBH_TX_WAN_CHANNEL_INDEX(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register BBH_TX_WAN_CHANNEL_INDEX\n");

	MREAD_32((uint8_t *)p, r);
	bdmf_session_print(session, "\treserved_fw_only         = 0x%08x", (unsigned int)r);
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

void dump_RDD_WLAN_MCAST_DHD_STATION_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register WLAN_MCAST_DHD_STATION_ENTRY\n");

	MREAD_16((uint8_t *)p, r);
	bdmf_session_print(session, "\tmac_address_high         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 2, r);
	bdmf_session_print(session, "\tmac_address_mid          = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 4, r);
	bdmf_session_print(session, "\tmac_address_low          = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 6, 6, 2, r);
	bdmf_session_print(session, "\tradio_index              = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 6, 2, 4, r);
	bdmf_session_print(session, "\tssid                     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p + 6, 0, 10, r);
	bdmf_session_print(session, "\tflowring_index           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 8, 3, 5, r);
	bdmf_session_print(session, "\treserved1                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 8, 0, 3, r);
	bdmf_session_print(session, "\ttx_priority              = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 9, r);
	bdmf_session_print(session, "\treference_count          = 0x%08x", (unsigned int)r);
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

#if defined DSL_63138
void dump_RDD_ETHWAN2_RX_DESCRIPTOR(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register ETHWAN2_RX_DESCRIPTOR\n");

	FIELD_MREAD_16((uint8_t *)p, 5, 11, r);
	bdmf_session_print(session, "\treserved0                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_32((uint8_t *)p + 0, 14, 7, r);
	bdmf_session_print(session, "\tpayload_offset           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p + 2, 0, 14, r);
	bdmf_session_print(session, "\tpacket_length            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 4, 7, 1, r);
	bdmf_session_print(session, "\terror                    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 4, 5, 2, r);
	bdmf_session_print(session, "\treserved1                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p + 4, 6, 7, r);
	bdmf_session_print(session, "\terror_type               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 5, 0, 6, r);
	bdmf_session_print(session, "\tsource_bridge_port       = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 6, 7, 1, r);
	bdmf_session_print(session, "\ttarget_memory            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p + 6, 0, 15, r);
	bdmf_session_print(session, "\tbuffer_number            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

#endif

void dump_RDD_TRACE_C_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register TRACE_C_ENTRY\n");

	FIELD_MREAD_8((uint8_t *)p, 7, 1, r);
	bdmf_session_print(session, "\ttrace_on                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p, 6, 1, r);
	bdmf_session_print(session, "\ttrace_off_perm           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_32((uint8_t *)p, 0, 30, r);
	bdmf_session_print(session, "\tunused                   = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 4, r);
	bdmf_session_print(session, "\tallocs                   = 0x%08x", (unsigned int)r);
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

void dump_RDD_DHD_BACKUP_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register DHD_BACKUP_ENTRY\n");

	MREAD_32((uint8_t *)p, r);
	bdmf_session_print(session, "\tbq_next_idx_or_cpu_tx_frid= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 4, r);
	bdmf_session_print(session, "\trequest_id               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 8, r);
	bdmf_session_print(session, "\twifi_egress_params_u     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 12, r);
	bdmf_session_print(session, "\tdata_buf_addr_low        = 0x%08x", (unsigned int)r);
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

void dump_RDD_BYTES_2(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register BYTES_2\n");

	MREAD_16((uint8_t *)p, r);
	bdmf_session_print(session, "\treserved_fw_only         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_WLAN_MCAST_FWD_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register WLAN_MCAST_FWD_ENTRY\n");

	FIELD_MREAD_8((uint8_t *)p, 7, 1, r);
	bdmf_session_print(session, "\tvalid                    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p, 0, 7, r);
	bdmf_session_print(session, "\tdhd_list_size            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 1, 7, 1, r);
	bdmf_session_print(session, "\treserved1                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 1, 6, 1, r);
	bdmf_session_print(session, "\tis_proxy_enabled         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 1, 3, 3, r);
	bdmf_session_print(session, "\ttx_priority              = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 1, 2, 1, r);
	bdmf_session_print(session, "\twfd_0_priority           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 1, 1, 1, r);
	bdmf_session_print(session, "\twfd_1_priority           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 1, 0, 1, r);
	bdmf_session_print(session, "\twfd_2_priority           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 2, r);
	bdmf_session_print(session, "\twfd_0_ssid_vector        = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 4, r);
	bdmf_session_print(session, "\twfd_1_ssid_vector        = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 6, r);
	bdmf_session_print(session, "\twfd_2_ssid_vector        = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

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

	FIELD_MREAD_8((uint8_t *)p + 10, 2, 5, r);
	bdmf_session_print(session, "\tprofile_id               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p + 10, 6, 4, r);
	bdmf_session_print(session, "\treserved                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 11, 0, 6, r);
	bdmf_session_print(session, "\ttail_base_entry          = 0x%08x", (unsigned int)r);
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

void dump_RDD_WLAN_MCAST_SSID_MAC_ADDRESS_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register WLAN_MCAST_SSID_MAC_ADDRESS_ENTRY\n");

	MREAD_32((uint8_t *)p, r);
	bdmf_session_print(session, "\tmac_address_high         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 4, r);
	bdmf_session_print(session, "\tmac_address_low          = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 6, r);
	bdmf_session_print(session, "\treference_count          = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 7, r);
	bdmf_session_print(session, "\treserved1                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_WLAN_MCAST_SSID_STATS_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register WLAN_MCAST_SSID_STATS_ENTRY\n");

	MREAD_32((uint8_t *)p, r);
	bdmf_session_print(session, "\tpackets                  = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 4, r);
	bdmf_session_print(session, "\tbytes                    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_DHD_RADIO_INSTANCE_COMMON_A_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	int i,j;

	bdmf_session_print(session, "  Register DHD_RADIO_INSTANCE_COMMON_A_ENTRY\n");

	MREAD_32((uint8_t *)p, r);
	bdmf_session_print(session, "\tds_dhd_doorbell_post     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 4, r);
	bdmf_session_print(session, "\tds_dhd_doorbell_complete = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 8, r);
	bdmf_session_print(session, "\tds_rd_fr_r2d_indexes     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 10, r);
	bdmf_session_print(session, "\tds_wr_fr_r2d_indexes     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 12, r);
	bdmf_session_print(session, "\ttx_rd_fr_d2r_indexes     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 14, r);
	bdmf_session_print(session, "\ttx_wr_fr_d2r_indexes     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 16, r);
	bdmf_session_print(session, "\ttx_complete_packet_counter= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 18, 5, 3, r);
	bdmf_session_print(session, "\tflow_ring_format         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 18, 4, 1, r);
	bdmf_session_print(session, "\tidma_active              = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p + 18, 0, 12, r);
	bdmf_session_print(session, "\treserved1                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 20, r);
	bdmf_session_print(session, "\tcoalescing_max_count     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 21, r);
	bdmf_session_print(session, "\tcur_frg_id               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 22, r);
	bdmf_session_print(session, "\tcoalescing_timeout       = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 23, r);
	bdmf_session_print(session, "\tcoalescing_timeout_cntr  = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	bdmf_session_print(session, "\treserved2                =\n\t");
	for (i=0,j=0; i<8; i++)
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
}

void dump_RDD_DHD_FLOW_RING_CACHE_LKP_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register DHD_FLOW_RING_CACHE_LKP_ENTRY\n");

	FIELD_MREAD_8((uint8_t *)p, 7, 1, r);
	bdmf_session_print(session, "\tinvalid                  = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p, 4, 3, r);
	bdmf_session_print(session, "\treserved                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p, 2, 2, r);
	bdmf_session_print(session, "\tradio_idx                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p, 0, 10, r);
	bdmf_session_print(session, "\tflow_ring_id             = 0x%08x", (unsigned int)r);
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
	for (i=0,j=0; i<1552; i++)
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

void dump_RDD_CPU_RX_DESCRIPTOR(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register CPU_RX_DESCRIPTOR\n");

	FIELD_MREAD_16((uint8_t *)p, 4, 12, r);
	bdmf_session_print(session, "\tflow_id                  = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 1, 3, 1, r);
	bdmf_session_print(session, "\tis_chksum_verified       = 0x%08x", (unsigned int)r);
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
	bdmf_session_print(session, "\tdest_ssid                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p + 6, 5, 4, r);
	bdmf_session_print(session, "\treserved2                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 7, 4, 1, r);
	bdmf_session_print(session, "\tis_exception             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 7, 0, 4, r);
	bdmf_session_print(session, "\tdescriptor_type          = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 8, 7, 1, r);
	bdmf_session_print(session, "\townership                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_32((uint8_t *)p + 8, 0, 31, r);
	bdmf_session_print(session, "\thost_data_buffer_pointer = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p + 12, 4, 12, r);
	bdmf_session_print(session, "\treserved4                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 13, 0, 4, r);
	bdmf_session_print(session, "\tip_sync_1588_entry_index = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 14, r);
	bdmf_session_print(session, "\twl_metadata              = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_DDR_QUEUE_ADDRESS_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register DDR_QUEUE_ADDRESS_ENTRY\n");

	MREAD_16((uint8_t *)p, r);
	bdmf_session_print(session, "\tsize                     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 2, r);
	bdmf_session_print(session, "\treserved0                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 4, r);
	bdmf_session_print(session, "\taddr                     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_RING_DESCRIPTOR(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register RING_DESCRIPTOR\n");

	FIELD_MREAD_8((uint8_t *)p, 3, 5, r);
	bdmf_session_print(session, "\treserved0                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p, 0, 11, r);
	bdmf_session_print(session, "\tentries_counter          = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 2, 3, 5, r);
	bdmf_session_print(session, "\tsize_of_entry            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p + 2, 0, 11, r);
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

	MREAD_32((uint8_t *)p + 12, r);
	bdmf_session_print(session, "\treserved1                = 0x%08x", (unsigned int)r);
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

void dump_RDD_DHD_TX_POST_FLOW_RING_MGMT_DESCRIPTOR(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register DHD_TX_POST_FLOW_RING_MGMT_DESCRIPTOR\n");

	MREAD_32((uint8_t *)p, r);
	bdmf_session_print(session, "\tflow_ring_base_low       = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 4, r);
	bdmf_session_print(session, "\tflow_ring_base_high      = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 8, r);
	bdmf_session_print(session, "\tsize                     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 10, 5, 3, r);
	bdmf_session_print(session, "\treserved_for_lock_flag   = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 10, 0, 5, r);
	bdmf_session_print(session, "\tssid                     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 11, r);
	bdmf_session_print(session, "\tflags                    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 12, r);
	bdmf_session_print(session, "\tbackup_first_index       = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 14, r);
	bdmf_session_print(session, "\tbackup_last_index        = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 16, r);
	bdmf_session_print(session, "\tbackup_num_entries       = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 18, r);
	bdmf_session_print(session, "\tphy_size                 = 0x%08x", (unsigned int)r);
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

void dump_RDD_CPU_TX_POST_REQUEST_QUEUE_IDX_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register CPU_TX_POST_REQUEST_QUEUE_IDX_ENTRY\n");

	MREAD_16((uint8_t *)p, r);
	bdmf_session_print(session, "\tread_idx                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 2, r);
	bdmf_session_print(session, "\twrite_idx                = 0x%08x", (unsigned int)r);
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

void dump_RDD_WAN_TX_SERVICE_QUEUE_DESCRIPTOR(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register WAN_TX_SERVICE_QUEUE_DESCRIPTOR\n");

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
	bdmf_session_print(session, "\tindex                    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 7, 4, 4, r);
	bdmf_session_print(session, "\treserved                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 7, 3, 1, r);
	bdmf_session_print(session, "\tuse_as_scheduler         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 7, 0, 3, r);
	bdmf_session_print(session, "\tscheduler_index          = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 8, r);
	bdmf_session_print(session, "\tpacket_threshold         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 10, r);
	bdmf_session_print(session, "\tprofile_ptr              = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 12, r);
	bdmf_session_print(session, "\tqueue_mask               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 16, r);
	bdmf_session_print(session, "\ttotal_pkt_counter        = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 20, r);
	bdmf_session_print(session, "\tsvc_queue_sched_ptr      = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 22, r);
	bdmf_session_print(session, "\treserved0                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 24, r);
	bdmf_session_print(session, "\treserved1                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 28, r);
	bdmf_session_print(session, "\treserved2                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_DHD_BACKUP_INFO_CACHE_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register DHD_BACKUP_INFO_CACHE_ENTRY\n");

	MREAD_16((uint8_t *)p, r);
	bdmf_session_print(session, "\tbackup_first_index       = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 2, r);
	bdmf_session_print(session, "\tbackup_last_index        = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 4, r);
	bdmf_session_print(session, "\tbackup_num_entries       = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 6, r);
	bdmf_session_print(session, "\tphy_size                 = 0x%08x", (unsigned int)r);
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
	bdmf_session_print(session, "\tindex                    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 7, 4, 4, r);
	bdmf_session_print(session, "\treserved                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 7, 3, 1, r);
	bdmf_session_print(session, "\tuse_as_scheduler         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 7, 0, 3, r);
	bdmf_session_print(session, "\tscheduler_index          = 0x%08x", (unsigned int)r);
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

void dump_RDD_DHD_RADIO_INSTANCE_COMMON_B_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	int i,j;

	bdmf_session_print(session, "  Register DHD_RADIO_INSTANCE_COMMON_B_ENTRY\n");

	MREAD_32((uint8_t *)p, r);
	bdmf_session_print(session, "\tus_dhd_doorbell_post     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 4, r);
	bdmf_session_print(session, "\tus_dhd_doorbell_complete = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 8, r);
	bdmf_session_print(session, "\tr2d_wr_fr_desc_base_ptr  = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 12, r);
	bdmf_session_print(session, "\td2r_rd_fr_desc_base_ptr  = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 16, r);
	bdmf_session_print(session, "\tr2d_rd_fr_desc_base_ptr  = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 20, r);
	bdmf_session_print(session, "\td2r_wr_fr_desc_base_ptr  = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 24, r);
	bdmf_session_print(session, "\trx_post_fr_base_ptr      = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 28, r);
	bdmf_session_print(session, "\ttx_complete_fr_base_ptr  = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 32, r);
	bdmf_session_print(session, "\trx_complete_fr_base_ptr  = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 36, r);
	bdmf_session_print(session, "\ttx_post_mgmt_fr_base_ptr = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 40, r);
	bdmf_session_print(session, "\trx_post_r2d_index        = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 42, r);
	bdmf_session_print(session, "\tus_rd_fr_r2d_indexes     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 44, r);
	bdmf_session_print(session, "\tus_wr_fr_r2d_indexes     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 46, r);
	bdmf_session_print(session, "\trx_rd_fr_d2r_indexes     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 48, r);
	bdmf_session_print(session, "\trx_wr_fr_d2r_indexes     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 50, 7, 1, r);
	bdmf_session_print(session, "\tadd_llcsnap_header       = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 50, 6, 1, r);
	bdmf_session_print(session, "\taggregation_bypass_cpu_tx= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 50, 5, 1, r);
	bdmf_session_print(session, "\taggregation_bypass_non_udp_tcp= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 50, 4, 1, r);
	bdmf_session_print(session, "\treserved1                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 50, 3, 1, r);
	bdmf_session_print(session, "\tidma_active              = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 50, 0, 3, r);
	bdmf_session_print(session, "\tflow_ring_format         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 51, r);
	bdmf_session_print(session, "\taggregation_bypass_tcp_pktlen= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	bdmf_session_print(session, "\tper_ac_aggregation_thresholds=\n\t");
	for (i=0,j=0; i<4; i++)
	{
		MREAD_I_8((uint8_t *)p + 52, i, r);
		bdmf_session_print(session, "0x%08x ", (unsigned int)r);
		j++;
		if (j >= 8)
		{
			j = 0;
			bdmf_session_print(session, "\n\t");
		}
	}
	bdmf_session_print(session, "\n");
	bdmf_session_print(session, "\tper_ac_aggregation_timeouts=\n\t");
	for (i=0,j=0; i<4; i++)
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
	bdmf_session_print(session, "\tper_ac_aggregation_timeout_cntrs=\n\t");
	for (i=0,j=0; i<4; i++)
	{
		MREAD_I_8((uint8_t *)p + 60, i, r);
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

void dump_RDD_DHD_FLOW_RING_CACHE_CTX_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register DHD_FLOW_RING_CACHE_CTX_ENTRY\n");

	MREAD_32((uint8_t *)p, r);
	bdmf_session_print(session, "\tflow_ring_base_low       = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 4, r);
	bdmf_session_print(session, "\tflow_ring_base_high      = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 8, r);
	bdmf_session_print(session, "\tsize                     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 10, 7, 1, r);
	bdmf_session_print(session, "\tlock                     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 10, 5, 2, r);
	bdmf_session_print(session, "\treserved                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 10, 0, 5, r);
	bdmf_session_print(session, "\tssid                     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 11, r);
	bdmf_session_print(session, "\tflags                    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 12, r);
	bdmf_session_print(session, "\trd_idx                   = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 14, r);
	bdmf_session_print(session, "\twr_idx                   = 0x%08x", (unsigned int)r);
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
	bdmf_session_print(session, "\treserved0                =\n\t");
	for (i=0,j=0; i<4; i++)
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
	MREAD_32((uint8_t *)p + 36, r);
	bdmf_session_print(session, "\treference_count          = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	bdmf_session_print(session, "\treserved1                =\n\t");
	for (i=0,j=0; i<4; i++)
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
	MREAD_32((uint8_t *)p + 40, r);
	bdmf_session_print(session, "\tis_ipv6_address          = 0x%08x", (unsigned int)r);
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

void dump_RDD_DHD_BACKUP_QUEUE_MANAGEMENT_INFO_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register DHD_BACKUP_QUEUE_MANAGEMENT_INFO_ENTRY\n");

	MREAD_32((uint8_t *)p, r);
	bdmf_session_print(session, "\tdhd_backup_queues_ddr_ptr= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 4, r);
	bdmf_session_print(session, "\tdhd_bq_index_stack_ddr_ptr= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 8, r);
	bdmf_session_print(session, "\tdhd_bq_index_total_entry_count= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 10, r);
	bdmf_session_print(session, "\tdhd_bq_index_used_entry_count= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 12, r);
	bdmf_session_print(session, "\tdhd_bq_index_cache_a_cur_offset= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 14, r);
	bdmf_session_print(session, "\tdhd_bq_index_cache_b_cur_offset= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_BPM_PACKET_BUFFER(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	int i,j;

	bdmf_session_print(session, "  Register BPM_PACKET_BUFFER\n");

	bdmf_session_print(session, "\treserved_fw_only         =\n\t");
	for (i=0,j=0; i<512; i++)
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

void dump_RDD_FC_UCAST_FLOW_CONTEXT_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	int i,j;

	bdmf_session_print(session, "  Register FC_UCAST_FLOW_CONTEXT_ENTRY\n");

	MREAD_32((uint8_t *)p, r);
	bdmf_session_print(session, "\tflow_hits                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 4, r);
	bdmf_session_print(session, "\tflow_bytes               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 8, 7, 1, r);
	bdmf_session_print(session, "\tmulticast_flag           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 8, 6, 1, r);
	bdmf_session_print(session, "\toverflow                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 8, 5, 1, r);
	bdmf_session_print(session, "\tis_routed                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 8, 4, 1, r);
	bdmf_session_print(session, "\tis_tos_mangle            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 8, 3, 1, r);
	bdmf_session_print(session, "\tis_l2_accel              = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p + 8, 0, 11, r);
	bdmf_session_print(session, "\tmtu                      = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 10, r);
	bdmf_session_print(session, "\ttos                      = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 11, 7, 1, r);
	bdmf_session_print(session, "\tis_wred_high_prio        = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 11, 6, 1, r);
	bdmf_session_print(session, "\tis_mapt_us               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 11, 5, 1, r);
	bdmf_session_print(session, "\tis_df                    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 11, 0, 5, r);
	bdmf_session_print(session, "\tservice_queue_id         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 12, 7, 1, r);
	bdmf_session_print(session, "\tdrop                     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 12, 6, 1, r);
	bdmf_session_print(session, "\tis_unicast_wfd_any       = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 12, 2, 4, r);
	bdmf_session_print(session, "\tpriority                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 12, 1, 1, r);
	bdmf_session_print(session, "\twfd_prio                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p + 12, 7, 2, r);
	bdmf_session_print(session, "\twfd_idx                  = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 13, 5, 2, r);
	bdmf_session_print(session, "\tegress_phy               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 13, 2, 3, r);
	bdmf_session_print(session, "\tip_addresses_table_index = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_32((uint8_t *)p + 12, 0, 18, r);
	bdmf_session_print(session, "\tlink_specific_union      = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 16, 7, 1, r);
	bdmf_session_print(session, "\tis_hit_trap              = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 16, 6, 1, r);
	bdmf_session_print(session, "\tis_ingqos_high_prio      = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_32((uint8_t *)p + 16, 14, 16, r);
	bdmf_session_print(session, "\treserved_5               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 18, 0, 6, r);
	bdmf_session_print(session, "\tcpu_reason               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 19, r);
	bdmf_session_print(session, "\tpathstat_idx             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 20, r);
	bdmf_session_print(session, "\treserved_6               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	bdmf_session_print(session, "\tcommand_list             =\n\t");
	for (i=0,j=0; i<104; i++)
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
	MREAD_8((uint8_t *)p + 128, r);
	bdmf_session_print(session, "\tvalid                    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 129, 4, 4, r);
	bdmf_session_print(session, "\tcommand_list_length_64   = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 129, 2, 2, r);
	bdmf_session_print(session, "\treserved4                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 129, 1, 1, r);
	bdmf_session_print(session, "\tis_unicast_wfd_nic       = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 129, 0, 1, r);
	bdmf_session_print(session, "\tconnection_direction     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 130, r);
	bdmf_session_print(session, "\tconnection_table_index   = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_FC_UCAST_FLOW_CONTEXT_ETH_XTM_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register FC_UCAST_FLOW_CONTEXT_ETH_XTM_ENTRY\n");

	MREAD_32((uint8_t *)p, r);
	bdmf_session_print(session, "\treserved1                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 4, r);
	bdmf_session_print(session, "\treserved2                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 8, r);
	bdmf_session_print(session, "\treserved3                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p + 12, 2, 14, r);
	bdmf_session_print(session, "\treserved4                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 13, 0, 2, r);
	bdmf_session_print(session, "\tegress_info              = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 14, r);
	bdmf_session_print(session, "\tegress_port              = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 15, 5, 3, r);
	bdmf_session_print(session, "\ttraffic_class            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 15, 0, 5, r);
	bdmf_session_print(session, "\trate_controller          = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_FC_UCAST_FLOW_CONTEXT_WFD_NIC_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register FC_UCAST_FLOW_CONTEXT_WFD_NIC_ENTRY\n");

	MREAD_32((uint8_t *)p, r);
	bdmf_session_print(session, "\treserved1                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 4, r);
	bdmf_session_print(session, "\treserved2                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 8, r);
	bdmf_session_print(session, "\treserved3                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 12, r);
	bdmf_session_print(session, "\treserved4                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 14, r);
	bdmf_session_print(session, "\tchain_idx                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_FC_UCAST_FLOW_CONTEXT_WFD_DHD_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register FC_UCAST_FLOW_CONTEXT_WFD_DHD_ENTRY\n");

	MREAD_32((uint8_t *)p, r);
	bdmf_session_print(session, "\treserved1                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 4, r);
	bdmf_session_print(session, "\treserved2                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 8, r);
	bdmf_session_print(session, "\treserved3                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_32((uint8_t *)p + 12, 14, 18, r);
	bdmf_session_print(session, "\treserved4                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 14, 2, 4, r);
	bdmf_session_print(session, "\twifi_ssid                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p + 14, 0, 10, r);
	bdmf_session_print(session, "\tflow_ring_id             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_FC_UCAST_FLOW_CONTEXT_RNR_DHD_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register FC_UCAST_FLOW_CONTEXT_RNR_DHD_ENTRY\n");

	MREAD_32((uint8_t *)p, r);
	bdmf_session_print(session, "\treserved1                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 4, r);
	bdmf_session_print(session, "\treserved2                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 8, r);
	bdmf_session_print(session, "\treserved3                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p + 12, 2, 14, r);
	bdmf_session_print(session, "\treserved4                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 13, 0, 2, r);
	bdmf_session_print(session, "\tdhd_flow_priority        = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 14, 6, 2, r);
	bdmf_session_print(session, "\tradio_idx                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 14, 2, 4, r);
	bdmf_session_print(session, "\twifi_ssid                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p + 14, 0, 10, r);
	bdmf_session_print(session, "\tflow_ring_id             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_FC_L2_UCAST_FLOW_CONTEXT_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	int i,j;

	bdmf_session_print(session, "  Register FC_L2_UCAST_FLOW_CONTEXT_ENTRY\n");

	MREAD_32((uint8_t *)p, r);
	bdmf_session_print(session, "\tflow_hits                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 4, r);
	bdmf_session_print(session, "\tflow_bytes               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 8, 7, 1, r);
	bdmf_session_print(session, "\tmulticast_flag           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 8, 6, 1, r);
	bdmf_session_print(session, "\toverflow                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 8, 5, 1, r);
	bdmf_session_print(session, "\tis_routed                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 8, 4, 1, r);
	bdmf_session_print(session, "\tis_tos_mangle            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 8, 3, 1, r);
	bdmf_session_print(session, "\tis_l2_accel              = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p + 8, 0, 11, r);
	bdmf_session_print(session, "\tmtu                      = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 10, r);
	bdmf_session_print(session, "\ttos                      = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 11, 7, 1, r);
	bdmf_session_print(session, "\tis_wred_high_prio        = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 11, 5, 2, r);
	bdmf_session_print(session, "\treserved2                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 11, 0, 5, r);
	bdmf_session_print(session, "\tservice_queue_id         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 12, 7, 1, r);
	bdmf_session_print(session, "\tdrop                     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 12, 6, 1, r);
	bdmf_session_print(session, "\tis_unicast_wfd_any       = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 12, 2, 4, r);
	bdmf_session_print(session, "\tpriority                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 12, 1, 1, r);
	bdmf_session_print(session, "\twfd_prio                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p + 12, 7, 2, r);
	bdmf_session_print(session, "\twfd_idx                  = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 13, 5, 2, r);
	bdmf_session_print(session, "\tegress_phy               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 13, 2, 3, r);
	bdmf_session_print(session, "\tip_addresses_table_index = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_32((uint8_t *)p + 12, 0, 18, r);
	bdmf_session_print(session, "\tlink_specific_union      = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 16, 7, 1, r);
	bdmf_session_print(session, "\tis_hit_trap              = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 16, 6, 1, r);
	bdmf_session_print(session, "\tis_ingqos_high_prio      = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_32((uint8_t *)p + 16, 14, 16, r);
	bdmf_session_print(session, "\treserved_5               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 18, 0, 6, r);
	bdmf_session_print(session, "\tcpu_reason               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 19, r);
	bdmf_session_print(session, "\tpathstat_idx             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 20, r);
	bdmf_session_print(session, "\treserved_6               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	bdmf_session_print(session, "\tcommand_list             =\n\t");
	for (i=0,j=0; i<104; i++)
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
	MREAD_8((uint8_t *)p + 128, r);
	bdmf_session_print(session, "\tvalid                    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 129, 4, 4, r);
	bdmf_session_print(session, "\tcommand_list_length_64   = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 129, 2, 2, r);
	bdmf_session_print(session, "\treserved4                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 129, 1, 1, r);
	bdmf_session_print(session, "\tis_unicast_wfd_nic       = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 129, 0, 1, r);
	bdmf_session_print(session, "\tconnection_direction     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 130, r);
	bdmf_session_print(session, "\tconnection_table_index   = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_FC_MCAST_FLOW_CONTEXT_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	int i,j;

	bdmf_session_print(session, "  Register FC_MCAST_FLOW_CONTEXT_ENTRY\n");

	MREAD_32((uint8_t *)p, r);
	bdmf_session_print(session, "\tflow_hits                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 4, r);
	bdmf_session_print(session, "\tflow_bytes               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 8, 7, 1, r);
	bdmf_session_print(session, "\tmulticast_flag           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 8, 6, 1, r);
	bdmf_session_print(session, "\toverflow                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 8, 5, 1, r);
	bdmf_session_print(session, "\tis_routed                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 8, 4, 1, r);
	bdmf_session_print(session, "\tis_tos_mangle            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 8, 0, 4, r);
	bdmf_session_print(session, "\tnumber_of_ports          = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 9, r);
	bdmf_session_print(session, "\tport_mask                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 10, 3, 5, r);
	bdmf_session_print(session, "\treserved2                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p + 10, 0, 11, r);
	bdmf_session_print(session, "\tmtu                      = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 12, r);
	bdmf_session_print(session, "\ttos                      = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 13, r);
	bdmf_session_print(session, "\treserved3                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 14, r);
	bdmf_session_print(session, "\twlan_mcast_clients       = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 15, r);
	bdmf_session_print(session, "\twlan_mcast_index         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 16, r);
	bdmf_session_print(session, "\tmcast_port_header_buffer_ptr= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	bdmf_session_print(session, "\tport_context             =\n\t");
	for (i=0,j=0; i<8; i++)
	{
		MREAD_I_32((uint8_t *)p + 20, i, r);
		bdmf_session_print(session, "0x%08x ", (unsigned int)r);
		j++;
		if (j >= 8)
		{
			j = 0;
			bdmf_session_print(session, "\n\t");
		}
	}
	bdmf_session_print(session, "\n");
	bdmf_session_print(session, "\tl3_command_list          =\n\t");
	for (i=0,j=0; i<20; i++)
	{
		MREAD_I_8((uint8_t *)p + 52, i, r);
		bdmf_session_print(session, "0x%08x ", (unsigned int)r);
		j++;
		if (j >= 8)
		{
			j = 0;
			bdmf_session_print(session, "\n\t");
		}
	}
	bdmf_session_print(session, "\n");
	bdmf_session_print(session, "\treserved4                =\n\t");
	for (i=0,j=0; i<56; i++)
	{
		MREAD_I_8((uint8_t *)p + 72, i, r);
		bdmf_session_print(session, "0x%08x ", (unsigned int)r);
		j++;
		if (j >= 8)
		{
			j = 0;
			bdmf_session_print(session, "\n\t");
		}
	}
	bdmf_session_print(session, "\n");
	MREAD_8((uint8_t *)p + 128, r);
	bdmf_session_print(session, "\tvalid                    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 129, 4, 4, r);
	bdmf_session_print(session, "\tcommand_list_length_64   = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 129, 1, 3, r);
	bdmf_session_print(session, "\treserved5                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 129, 0, 1, r);
	bdmf_session_print(session, "\tconnection_direction     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 130, r);
	bdmf_session_print(session, "\tconnection_table_index   = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_WLAN_MCAST_DHD_LIST_ENTRY_ARRAY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	int i,j;

	bdmf_session_print(session, "  Register WLAN_MCAST_DHD_LIST_ENTRY_ARRAY\n");

	bdmf_session_print(session, "\tdhd_station              =\n\t");
	for (i=0,j=0; i<64; i++)
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

void dump_RDD_WLAN_MCAST_DHD_LIST_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register WLAN_MCAST_DHD_LIST_ENTRY\n");

	FIELD_MREAD_8((uint8_t *)p, 7, 1, r);
	bdmf_session_print(session, "\tvalid                    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p, 0, 7, r);
	bdmf_session_print(session, "\tindex                    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

#if defined DSL_63148
void dump_RDD_BBH_TX_DESCRIPTOR(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register BBH_TX_DESCRIPTOR\n");

	FIELD_MREAD_8((uint8_t *)p, 7, 1, r);
	bdmf_session_print(session, "\tcrc_calc                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p, 3, 12, r);
	bdmf_session_print(session, "\twan_port_or_fstat        = 0x%08x", (unsigned int)r);
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

#endif

#if defined DSL_63148
void dump_RDD_BBH_RX_DESCRIPTOR(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register BBH_RX_DESCRIPTOR\n");

	FIELD_MREAD_16((uint8_t *)p, 6, 10, r);
	bdmf_session_print(session, "\tlast_sbn                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 1, 5, 1, r);
	bdmf_session_print(session, "\tfstat_cell               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_32((uint8_t *)p + 0, 14, 7, r);
	bdmf_session_print(session, "\tflow_id                  = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p + 2, 0, 14, r);
	bdmf_session_print(session, "\tpacket_length            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 4, 7, 1, r);
	bdmf_session_print(session, "\terror                    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 4, 6, 1, r);
	bdmf_session_print(session, "\treserved0                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 4, 5, 1, r);
	bdmf_session_print(session, "\tfstat_error              = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p + 4, 6, 7, r);
	bdmf_session_print(session, "\terror_type               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 5, 0, 6, r);
	bdmf_session_print(session, "\tih_buffer_number         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 6, 7, 1, r);
	bdmf_session_print(session, "\ttarget_memory            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 6, 6, 1, r);
	bdmf_session_print(session, "\treserved1                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p + 6, 0, 14, r);
	bdmf_session_print(session, "\tbuffer_number            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

#endif

#if defined DSL_63148
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

#endif

#if defined DSL_63148
void dump_RDD_DSL_PTM_BOND_TX_HDR_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register DSL_PTM_BOND_TX_HDR_ENTRY\n");

	FIELD_MREAD_8((uint8_t *)p, 7, 1, r);
	bdmf_session_print(session, "\tport_sel                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p, 5, 2, r);
	bdmf_session_print(session, "\treserved0                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p, 4, 1, r);
	bdmf_session_print(session, "\tpkt_eop                  = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p, 0, 12, r);
	bdmf_session_print(session, "\tfrag_size                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

#endif

#if defined DSL_63148
void dump_RDD_ETHWAN2_RX_DESCRIPTOR(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register ETHWAN2_RX_DESCRIPTOR\n");

	FIELD_MREAD_16((uint8_t *)p, 5, 11, r);
	bdmf_session_print(session, "\treserved0                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_32((uint8_t *)p + 0, 14, 7, r);
	bdmf_session_print(session, "\tpayload_offset           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p + 2, 0, 14, r);
	bdmf_session_print(session, "\tpacket_length            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 4, 7, 1, r);
	bdmf_session_print(session, "\terror                    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 4, 5, 2, r);
	bdmf_session_print(session, "\treserved1                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p + 4, 6, 7, r);
	bdmf_session_print(session, "\terror_type               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 5, 0, 6, r);
	bdmf_session_print(session, "\tsource_bridge_port       = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 6, 7, 1, r);
	bdmf_session_print(session, "\ttarget_memory            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 6, 6, 1, r);
	bdmf_session_print(session, "\treserved2                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p + 6, 0, 14, r);
	bdmf_session_print(session, "\tbuffer_number            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

#endif

#if defined WL4908
void dump_RDD_BBH_TX_DESCRIPTOR(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register BBH_TX_DESCRIPTOR\n");

	FIELD_MREAD_8((uint8_t *)p, 4, 4, r);
	bdmf_session_print(session, "\treserved0                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p, 3, 1, r);
	bdmf_session_print(session, "\twred_bit_reserved0       = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p, 0, 3, r);
	bdmf_session_print(session, "\ttx_queue_reserved0       = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 1, 3, 5, r);
	bdmf_session_print(session, "\tegress_port_reserved0    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 1, 0, 3, r);
	bdmf_session_print(session, "\theader_number            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 2, 4, 4, r);
	bdmf_session_print(session, "\tsource_bridge_port       = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p + 2, 0, 12, r);
	bdmf_session_print(session, "\tpacket_length            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 4, 7, 1, r);
	bdmf_session_print(session, "\tabsolute_normal          = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 4, 4, 3, r);
	bdmf_session_print(session, "\tmisc                     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p + 4, 3, 9, r);
	bdmf_session_print(session, "\tpayload_offset           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 5, 2, 1, r);
	bdmf_session_print(session, "\ttarget_memory            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 5, 0, 2, r);
	bdmf_session_print(session, "\tddr_params               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 6, r);
	bdmf_session_print(session, "\tpacket_location          = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

#endif

#if defined WL4908
void dump_RDD_CONTEXT_CONTINUATION_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	int i,j;

	bdmf_session_print(session, "  Register CONTEXT_CONTINUATION_ENTRY\n");

	FIELD_MREAD_8((uint8_t *)p, 7, 1, r);
	bdmf_session_print(session, "\tvalid                    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p, 6, 1, r);
	bdmf_session_print(session, "\treserved1                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p, 5, 1, r);
	bdmf_session_print(session, "\tconnection_direction     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p, 4, 1, r);
	bdmf_session_print(session, "\tis_unicast_wfd_nic       = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p, 0, 12, r);
	bdmf_session_print(session, "\treserved2                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 2, r);
	bdmf_session_print(session, "\tconnection_table_index   = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_32((uint8_t *)p + 4, 15, 17, r);
	bdmf_session_print(session, "\treserved3                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p + 6, 0, 15, r);
	bdmf_session_print(session, "\tflow_index               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	bdmf_session_print(session, "\tcommand_list             =\n\t");
	for (i=0,j=0; i<68; i++)
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

#endif

#if defined WL4908
void dump_RDD_BBH_RX_DESCRIPTOR(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register BBH_RX_DESCRIPTOR\n");

	FIELD_MREAD_16((uint8_t *)p, 6, 10, r);
	bdmf_session_print(session, "\tlast_sbn                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_32((uint8_t *)p + 0, 13, 9, r);
	bdmf_session_print(session, "\tpayload_offset           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 2, 4, 1, r);
	bdmf_session_print(session, "\treserved0                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p + 2, 0, 12, r);
	bdmf_session_print(session, "\tpacket_length            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 4, 7, 1, r);
	bdmf_session_print(session, "\terror                    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 4, 5, 2, r);
	bdmf_session_print(session, "\treserved1                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 4, 1, 4, r);
	bdmf_session_print(session, "\terror_type               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p + 4, 3, 6, r);
	bdmf_session_print(session, "\tih_buffer_number         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 5, 2, 1, r);
	bdmf_session_print(session, "\ttarget_memory            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 5, 1, 1, r);
	bdmf_session_print(session, "\tddr_id                   = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 5, 0, 1, r);
	bdmf_session_print(session, "\treserved2                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 6, r);
	bdmf_session_print(session, "\tbuffer_number            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

#endif

#if defined WL4908
void dump_RDD_CPU_TX_DESCRIPTOR_BPM(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register CPU_TX_DESCRIPTOR_BPM\n");

	MREAD_32((uint8_t *)p, r);
	bdmf_session_print(session, "\treserved0                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p + 4, 2, 14, r);
	bdmf_session_print(session, "\treserved1                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_32((uint8_t *)p + 4, 0, 18, r);
	bdmf_session_print(session, "\tbuffer_number            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

#endif

void dump_RDD_BYTES_4(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register BYTES_4\n");

	MREAD_32((uint8_t *)p, r);
	bdmf_session_print(session, "\treserved_fw_only         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

#if defined WL4908
void dump_RDD_ETHWAN2_RX_DESCRIPTOR(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register ETHWAN2_RX_DESCRIPTOR\n");

	FIELD_MREAD_16((uint8_t *)p, 6, 10, r);
	bdmf_session_print(session, "\treserved0                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_32((uint8_t *)p + 0, 13, 9, r);
	bdmf_session_print(session, "\tpayload_offset           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 2, 4, 1, r);
	bdmf_session_print(session, "\treserved1                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p + 2, 0, 12, r);
	bdmf_session_print(session, "\tpacket_length            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 4, 7, 1, r);
	bdmf_session_print(session, "\terror                    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 4, 5, 2, r);
	bdmf_session_print(session, "\treserved2                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 4, 1, 4, r);
	bdmf_session_print(session, "\terror_type               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p + 4, 3, 6, r);
	bdmf_session_print(session, "\tsource_bridge_port       = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 5, 2, 1, r);
	bdmf_session_print(session, "\ttarget_memory            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 5, 1, 1, r);
	bdmf_session_print(session, "\tddr_id                   = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 5, 0, 1, r);
	bdmf_session_print(session, "\treserved3                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 6, r);
	bdmf_session_print(session, "\tbuffer_number            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

#endif

void dump_RDD_NAT_CACHE_LKP_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register NAT_CACHE_LKP_ENTRY\n");

	FIELD_MREAD_8((uint8_t *)p, 7, 1, r);
	bdmf_session_print(session, "\tvalid                    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_32((uint8_t *)p, 12, 19, r);
	bdmf_session_print(session, "\treserved                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 2, 0, 4, r);
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

void dump_RDD_NAT_CACHE_L2_LKP_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register NAT_CACHE_L2_LKP_ENTRY\n");

	FIELD_MREAD_8((uint8_t *)p, 7, 1, r);
	bdmf_session_print(session, "\tvalid                    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_32((uint8_t *)p, 12, 19, r);
	bdmf_session_print(session, "\treserved1                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 2, 0, 4, r);
	bdmf_session_print(session, "\tkey_extend               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 3, r);
	bdmf_session_print(session, "\tprotocol                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_32((uint8_t *)p + 4, 9, 23, r);
	bdmf_session_print(session, "\treserved2                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 6, 0, 1, r);
	bdmf_session_print(session, "\ttcp_pure_ack             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 7, r);
	bdmf_session_print(session, "\ttos                      = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 8, r);
	bdmf_session_print(session, "\tsrc_mac_crc              = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 12, r);
	bdmf_session_print(session, "\tdst_mac_crc              = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_FC_NATC_UCAST_FLOW_CONTEXT_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	int i,j;

	bdmf_session_print(session, "  Register FC_NATC_UCAST_FLOW_CONTEXT_ENTRY\n");

	MREAD_32((uint8_t *)p, r);
	bdmf_session_print(session, "\tstatus_0                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 4, r);
	bdmf_session_print(session, "\tflow_bytes               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 8, 7, 1, r);
	bdmf_session_print(session, "\tmulticast_flag           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 8, 6, 1, r);
	bdmf_session_print(session, "\tcontext_continuation_flag= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 8, 5, 1, r);
	bdmf_session_print(session, "\tis_routed                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 8, 4, 1, r);
	bdmf_session_print(session, "\tis_tos_mangle            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 8, 3, 1, r);
	bdmf_session_print(session, "\tis_l2_accel              = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p + 8, 0, 11, r);
	bdmf_session_print(session, "\tmtu                      = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 10, r);
	bdmf_session_print(session, "\ttos                      = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 11, 7, 1, r);
	bdmf_session_print(session, "\tis_wred_high_prio        = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 11, 6, 1, r);
	bdmf_session_print(session, "\tis_mapt_us               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 11, 5, 1, r);
	bdmf_session_print(session, "\tis_df                    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 11, 0, 5, r);
	bdmf_session_print(session, "\tservice_queue_id         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 12, 7, 1, r);
	bdmf_session_print(session, "\tdrop                     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 12, 6, 1, r);
	bdmf_session_print(session, "\tis_unicast_wfd_any       = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 12, 2, 4, r);
	bdmf_session_print(session, "\tpriority                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 12, 1, 1, r);
	bdmf_session_print(session, "\twfd_prio                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p + 12, 7, 2, r);
	bdmf_session_print(session, "\twfd_idx                  = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 13, 5, 2, r);
	bdmf_session_print(session, "\tegress_phy               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 13, 2, 3, r);
	bdmf_session_print(session, "\tip_addresses_table_index = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_32((uint8_t *)p + 12, 0, 18, r);
	bdmf_session_print(session, "\tlink_specific_union      = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 16, 7, 1, r);
	bdmf_session_print(session, "\tis_hit_trap              = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 16, 6, 1, r);
	bdmf_session_print(session, "\tis_ingqos_high_prio      = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_32((uint8_t *)p + 16, 14, 16, r);
	bdmf_session_print(session, "\treserved_5               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 18, 0, 6, r);
	bdmf_session_print(session, "\tcpu_reason               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 19, r);
	bdmf_session_print(session, "\tpathstat_idx             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 20, r);
	bdmf_session_print(session, "\treserved_6               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	bdmf_session_print(session, "\tcommand_list             =\n\t");
	for (i=0,j=0; i<36; i++)
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
	MREAD_16((uint8_t *)p + 60, r);
	bdmf_session_print(session, "\tcontext_continuation_table_index= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 62, r);
	bdmf_session_print(session, "\tcommand_list_remaining_length= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_FC_NATC_L2_UCAST_FLOW_CONTEXT_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	int i,j;

	bdmf_session_print(session, "  Register FC_NATC_L2_UCAST_FLOW_CONTEXT_ENTRY\n");

	MREAD_32((uint8_t *)p, r);
	bdmf_session_print(session, "\tstatus_0                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 4, r);
	bdmf_session_print(session, "\tflow_bytes               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 8, 7, 1, r);
	bdmf_session_print(session, "\tmulticast_flag           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 8, 6, 1, r);
	bdmf_session_print(session, "\tcontext_continuation_flag= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 8, 5, 1, r);
	bdmf_session_print(session, "\tis_routed                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 8, 4, 1, r);
	bdmf_session_print(session, "\tis_tos_mangle            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 8, 3, 1, r);
	bdmf_session_print(session, "\tis_l2_accel              = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p + 8, 0, 11, r);
	bdmf_session_print(session, "\tmtu                      = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 10, r);
	bdmf_session_print(session, "\ttos                      = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 11, 7, 1, r);
	bdmf_session_print(session, "\tis_wred_high_prio        = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 11, 5, 2, r);
	bdmf_session_print(session, "\treserved2                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 11, 0, 5, r);
	bdmf_session_print(session, "\tservice_queue_id         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 12, 7, 1, r);
	bdmf_session_print(session, "\tdrop                     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 12, 6, 1, r);
	bdmf_session_print(session, "\tis_unicast_wfd_any       = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 12, 2, 4, r);
	bdmf_session_print(session, "\tpriority                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 12, 1, 1, r);
	bdmf_session_print(session, "\twfd_prio                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p + 12, 7, 2, r);
	bdmf_session_print(session, "\twfd_idx                  = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 13, 5, 2, r);
	bdmf_session_print(session, "\tegress_phy               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 13, 2, 3, r);
	bdmf_session_print(session, "\tip_addresses_table_index = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_32((uint8_t *)p + 12, 0, 18, r);
	bdmf_session_print(session, "\tlink_specific_union      = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 16, 7, 1, r);
	bdmf_session_print(session, "\tis_hit_trap              = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 16, 6, 1, r);
	bdmf_session_print(session, "\tis_ingqos_high_prio      = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_32((uint8_t *)p + 16, 14, 16, r);
	bdmf_session_print(session, "\treserved_5               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 18, 0, 6, r);
	bdmf_session_print(session, "\tcpu_reason               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 19, r);
	bdmf_session_print(session, "\tpathstat_idx             = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 20, r);
	bdmf_session_print(session, "\treserved_6               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	bdmf_session_print(session, "\tcommand_list             =\n\t");
	for (i=0,j=0; i<36; i++)
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
	MREAD_16((uint8_t *)p + 60, r);
	bdmf_session_print(session, "\tcontext_continuation_table_index= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 62, r);
	bdmf_session_print(session, "\tcommand_list_remaining_length= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

void dump_RDD_FC_NATC_MCAST_FLOW_CONTEXT_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	int i,j;

	bdmf_session_print(session, "  Register FC_NATC_MCAST_FLOW_CONTEXT_ENTRY\n");

	MREAD_32((uint8_t *)p, r);
	bdmf_session_print(session, "\tflow_hits                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 4, r);
	bdmf_session_print(session, "\tflow_bytes               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 8, 7, 1, r);
	bdmf_session_print(session, "\tmulticast_flag           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 8, 6, 1, r);
	bdmf_session_print(session, "\tcontext_continuation_flag= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 8, 5, 1, r);
	bdmf_session_print(session, "\tis_routed                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 8, 4, 1, r);
	bdmf_session_print(session, "\tis_tos_mangle            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 8, 0, 4, r);
	bdmf_session_print(session, "\tnumber_of_ports          = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 9, r);
	bdmf_session_print(session, "\tport_mask                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 10, 3, 5, r);
	bdmf_session_print(session, "\treserved2                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p + 10, 0, 11, r);
	bdmf_session_print(session, "\tmtu                      = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 12, r);
	bdmf_session_print(session, "\ttos                      = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 13, r);
	bdmf_session_print(session, "\treserved3                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 14, r);
	bdmf_session_print(session, "\twlan_mcast_clients       = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_8((uint8_t *)p + 15, r);
	bdmf_session_print(session, "\twlan_mcast_index         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_32((uint8_t *)p + 16, r);
	bdmf_session_print(session, "\tmcast_port_header_buffer_ptr= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	bdmf_session_print(session, "\tport_context             =\n\t");
	for (i=0,j=0; i<8; i++)
	{
		MREAD_I_32((uint8_t *)p + 20, i, r);
		bdmf_session_print(session, "0x%08x ", (unsigned int)r);
		j++;
		if (j >= 8)
		{
			j = 0;
			bdmf_session_print(session, "\n\t");
		}
	}
	bdmf_session_print(session, "\n");
	bdmf_session_print(session, "\tl3_command_list          =\n\t");
	for (i=0,j=0; i<8; i++)
	{
		MREAD_I_8((uint8_t *)p + 52, i, r);
		bdmf_session_print(session, "0x%08x ", (unsigned int)r);
		j++;
		if (j >= 8)
		{
			j = 0;
			bdmf_session_print(session, "\n\t");
		}
	}
	bdmf_session_print(session, "\n");
	MREAD_16((uint8_t *)p + 60, r);
	bdmf_session_print(session, "\tcontext_continuation_table_index= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 62, r);
	bdmf_session_print(session, "\tcommand_list_remaining_length= 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

#if defined WL4908_EAP
void dump_RDD_BBH_TX_DESCRIPTOR(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register BBH_TX_DESCRIPTOR\n");

	FIELD_MREAD_8((uint8_t *)p, 4, 4, r);
	bdmf_session_print(session, "\treserved0                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p, 3, 1, r);
	bdmf_session_print(session, "\twred_bit_reserved0       = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p, 0, 3, r);
	bdmf_session_print(session, "\ttx_queue_reserved0       = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 1, 3, 5, r);
	bdmf_session_print(session, "\tegress_port_reserved0    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 1, 0, 3, r);
	bdmf_session_print(session, "\theader_number            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 2, 4, 4, r);
	bdmf_session_print(session, "\tsource_bridge_port       = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p + 2, 0, 12, r);
	bdmf_session_print(session, "\tpacket_length            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 4, 7, 1, r);
	bdmf_session_print(session, "\tabsolute_normal          = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 4, 4, 3, r);
	bdmf_session_print(session, "\tmisc                     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p + 4, 3, 9, r);
	bdmf_session_print(session, "\tpayload_offset           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 5, 2, 1, r);
	bdmf_session_print(session, "\ttarget_memory            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 5, 0, 2, r);
	bdmf_session_print(session, "\tddr_params               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 6, r);
	bdmf_session_print(session, "\tpacket_location          = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

#endif

#if defined WL4908_EAP
void dump_RDD_CONTEXT_CONTINUATION_ENTRY(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	int i,j;

	bdmf_session_print(session, "  Register CONTEXT_CONTINUATION_ENTRY\n");

	FIELD_MREAD_8((uint8_t *)p, 7, 1, r);
	bdmf_session_print(session, "\tvalid                    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p, 6, 1, r);
	bdmf_session_print(session, "\treserved1                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p, 5, 1, r);
	bdmf_session_print(session, "\tconnection_direction     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p, 4, 1, r);
	bdmf_session_print(session, "\tis_unicast_wfd_nic       = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p, 0, 12, r);
	bdmf_session_print(session, "\treserved2                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 2, r);
	bdmf_session_print(session, "\tconnection_table_index   = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_32((uint8_t *)p + 4, 15, 17, r);
	bdmf_session_print(session, "\treserved3                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p + 6, 0, 15, r);
	bdmf_session_print(session, "\tflow_index               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	bdmf_session_print(session, "\tcommand_list             =\n\t");
	for (i=0,j=0; i<68; i++)
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

#endif

#if defined WL4908_EAP
void dump_RDD_BBH_RX_DESCRIPTOR(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register BBH_RX_DESCRIPTOR\n");

	FIELD_MREAD_16((uint8_t *)p, 6, 10, r);
	bdmf_session_print(session, "\tlast_sbn                 = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_32((uint8_t *)p + 0, 13, 9, r);
	bdmf_session_print(session, "\tpayload_offset           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 2, 4, 1, r);
	bdmf_session_print(session, "\tskip                     = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p + 2, 0, 12, r);
	bdmf_session_print(session, "\tpacket_length            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 4, 7, 1, r);
	bdmf_session_print(session, "\terror                    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 4, 5, 2, r);
	bdmf_session_print(session, "\treserved1                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 4, 1, 4, r);
	bdmf_session_print(session, "\terror_type               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p + 4, 3, 6, r);
	bdmf_session_print(session, "\tih_buffer_number         = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 5, 2, 1, r);
	bdmf_session_print(session, "\ttarget_memory            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 5, 1, 1, r);
	bdmf_session_print(session, "\tddr_id                   = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 5, 0, 1, r);
	bdmf_session_print(session, "\treserved2                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 6, r);
	bdmf_session_print(session, "\tbuffer_number            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

#endif

#if defined WL4908_EAP
void dump_RDD_CPU_TX_DESCRIPTOR_BPM(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register CPU_TX_DESCRIPTOR_BPM\n");

	MREAD_32((uint8_t *)p, r);
	bdmf_session_print(session, "\treserved0                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p + 4, 2, 14, r);
	bdmf_session_print(session, "\treserved1                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_32((uint8_t *)p + 4, 0, 18, r);
	bdmf_session_print(session, "\tbuffer_number            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

#endif

#if defined WL4908_EAP
void dump_RDD_ETHWAN2_RX_DESCRIPTOR(bdmf_session_handle session, unsigned char *p)
{
	unsigned int r;
	bdmf_session_print(session, "  Register ETHWAN2_RX_DESCRIPTOR\n");

	FIELD_MREAD_16((uint8_t *)p, 6, 10, r);
	bdmf_session_print(session, "\treserved0                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_32((uint8_t *)p + 0, 13, 9, r);
	bdmf_session_print(session, "\tpayload_offset           = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 2, 4, 1, r);
	bdmf_session_print(session, "\treserved1                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p + 2, 0, 12, r);
	bdmf_session_print(session, "\tpacket_length            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 4, 7, 1, r);
	bdmf_session_print(session, "\terror                    = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 4, 5, 2, r);
	bdmf_session_print(session, "\treserved2                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 4, 1, 4, r);
	bdmf_session_print(session, "\terror_type               = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_16((uint8_t *)p + 4, 3, 6, r);
	bdmf_session_print(session, "\tsource_bridge_port       = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 5, 2, 1, r);
	bdmf_session_print(session, "\ttarget_memory            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 5, 1, 1, r);
	bdmf_session_print(session, "\tddr_id                   = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	FIELD_MREAD_8((uint8_t *)p + 5, 0, 1, r);
	bdmf_session_print(session, "\treserved3                = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

	MREAD_16((uint8_t *)p + 6, r);
	bdmf_session_print(session, "\tbuffer_number            = 0x%08x", (unsigned int)r);
	bdmf_session_print(session, "\n");

}

#endif

