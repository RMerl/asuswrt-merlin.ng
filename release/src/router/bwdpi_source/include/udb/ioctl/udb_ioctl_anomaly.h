/*
 * Copyright 2014 Trend Micro Incorporated
 * Redistribution and use in source and binary forms, with or without modification, 
 * are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright notice, 
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice, 
 *    this list of conditions and the following disclaimer in the documentation 
 *    and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its contributors 
 *    may be used to endorse or promote products derived from this software without 
 *    specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND 
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
 * IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT 
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR 
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
 * OF SUCH DAMAGE.
 */

#ifndef _UDB_IOCTL_ANOMALY_H_
#define _UDB_IOCTL_ANOMALY_H_

#include "udb/ioctl/udb_ioctl_common.h"

#define UDB_ANOMALY_LOG_SIZE 500

enum
{
	UDB_IOCTL_ANOMALY_OP_NA = 0,
	UDB_IOCTL_ANOMALY_OP_GET_LOG,
	UDB_IOCTL_ANOMALY_OP_RESET_LOG,
	UDB_IOCTL_ANOMALY_OP_GET_LOG_V2,
	UDB_IOCTL_ANOMALY_OP_MAX
};

typedef struct udb_ano_ioc_entry
{
	uint64_t time;
	uint32_t rule_id;

	uint32_t hit_cnt;
	uint8_t action; //0 means accept, 1 means block
	uint8_t mac[6];
} udb_ano_ioc_entry_t;

typedef struct udb_ano_ioc_entry_list
{
	uint32_t cnt;
	udb_ano_ioc_entry_t entry[0];
} udb_ano_ioc_entry_list_t;

typedef struct udb_ano_ioc_entry_v2
{
	uint64_t time;
	uint32_t rule_id;

	uint32_t hit_cnt;
	uint8_t action; //0 means accept, 1 means block
	uint8_t mac[6];
	
	uint8_t role; //!< 1 is attacker, 0 is victim
	uint16_t sport;
	uint16_t dport;
	uint8_t ip_ver;	
	uint8_t dip[16];	//IP_ADDR_LEN
	uint8_t sip[16];
} udb_ano_ioc_entry_v2_t;

typedef struct udb_ano_ioc_entry_list_v2
{
	uint32_t cnt;
	udb_ano_ioc_entry_v2_t entry[0];
} udb_ano_ioc_entry_list_v2_t;

#ifdef __KERNEL__
int udb_ioctl_anomaly_op_copy_out(uint8_t op, void *buf, uint32_t buf_len, uint32_t *buf_used_len);
int udb_ioctl_anomaly_op_copy_none(uint8_t op);
#endif

#endif /* _UDB_IOCTL_ANOMALY_H_ */

