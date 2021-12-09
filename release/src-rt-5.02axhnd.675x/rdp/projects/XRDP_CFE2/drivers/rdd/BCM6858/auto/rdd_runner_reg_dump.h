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


#ifndef __RDD_RUNNER_REG_DUMP_H__
#define __RDD_RUNNER_REG_DUMP_H__

#include "bdmf_shell.h"

typedef struct
{
#ifndef USE_BDMF_SHELL
	void  (*callback)(unsigned char *xi_value, int rw, int unionreq);
#else
	void  (*callback)(bdmf_session_handle session, unsigned char *value);
#endif
	unsigned int starts;
} ENTRY_STRUCT;

typedef struct
{
	int length; /* length of each entry in bytes */
	ENTRY_STRUCT entries[];
} DUMP_RUNNERREG_STRUCT;

typedef struct
{
	char * table_name;
	int tbldmp; /* boolean */
	int segment;
	DUMP_RUNNERREG_STRUCT * entries;
    int size_rows; /* Number of rows in table */
    int size_rows_d2;/* If table's entries are tables by themselfves, number of rows in each entry table (dimention 2)*/
    int size_rows_d3; /* Like above, dimention 3 */
} TABLE_STRUCT;

#ifndef STT_PRINTF
	#if defined LINUX_KERNEL
		#define STT_PRINTF printk
	#else
		#define STT_PRINTF printf
	#endif
#endif


void dump_RDD_BASIC_RATE_LIMITER_DESCRIPTOR(bdmf_session_handle session, unsigned char *p);
void dump_RDD_SCHEDULING_QUEUE_DESCRIPTOR(bdmf_session_handle session, unsigned char *p);
void dump_RDD_BBH_TX_EGRESS_COUNTER_ENTRY(bdmf_session_handle session, unsigned char *p);
void dump_RDD_BYTES_4(bdmf_session_handle session, unsigned char *p);
void dump_RDD_BYTES_2(bdmf_session_handle session, unsigned char *p);
void dump_RDD_BYTE_1(bdmf_session_handle session, unsigned char *p);
void dump_RDD_TM_FLOW_CNTR_ENTRY(bdmf_session_handle session, unsigned char *p);
void dump_RDD_BBH_TX_QUEUE_ID_ENTRY(bdmf_session_handle session, unsigned char *p);
void dump_RDD_BASIC_SCHEDULER_DESCRIPTOR(bdmf_session_handle session, unsigned char *p);
void dump_RDD_PROCESSING_TX_DESCRIPTOR(bdmf_session_handle session, unsigned char *p);
void dump_RDD_UPDATE_FIFO_ENTRY(bdmf_session_handle session, unsigned char *p);
void dump_RDD_DEBUG_PRINT_INFO(bdmf_session_handle session, unsigned char *p);
void dump_RDD_FPM_GLOBAL_CFG(bdmf_session_handle session, unsigned char *p);
void dump_RDD_BBH_QUEUE_DESCRIPTOR(bdmf_session_handle session, unsigned char *p);
void dump_RDD_COMPLEX_SCHEDULER_DESCRIPTOR(bdmf_session_handle session, unsigned char *p);
void dump_RDD_TRACE_EVENT(bdmf_session_handle session, unsigned char *p);
void dump_RDD_RX_FLOW_ENTRY(bdmf_session_handle session, unsigned char *p);
void dump_RDD_CPU_RX_METER_ENTRY(bdmf_session_handle session, unsigned char *p);
void dump_RDD_CPU_RING_INTERRUPT_COUNTER_ENTRY(bdmf_session_handle session, unsigned char *p);
void dump_RDD_CPU_INTERRUPT_COALESCING_ENTRY(bdmf_session_handle session, unsigned char *p);
void dump_RDD_PROCESSING_RX_DESCRIPTOR(bdmf_session_handle session, unsigned char *p);
void dump_RDD_BYTES_8(bdmf_session_handle session, unsigned char *p);
void dump_RDD_DDR_ADDRESS(bdmf_session_handle session, unsigned char *p);
void dump_RDD_CPU_RING_DESCRIPTOR(bdmf_session_handle session, unsigned char *p);
void dump_RDD_MAC_TYPE_ENTRY(bdmf_session_handle session, unsigned char *p);
void dump_RDD_TX_FLOW_ENTRY(bdmf_session_handle session, unsigned char *p);
void dump_RDD_FPM_POOL_NUMBER(bdmf_session_handle session, unsigned char *p);
void dump_RDD_CPU_TX_DESCRIPTOR(bdmf_session_handle session, unsigned char *p);

#endif /* __RDD_RUNNER_REG_DUMP_H__ */
