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

#ifndef _UDB_IOCTL_PATROL_TQ_H_
#define _UDB_IOCTL_PATROL_TQ_H_

#include "udb/ioctl/udb_ioctl_common.h"

enum
{
	UDB_IOCTL_PATROL_TQ_OP_NA = 0,
	UDB_IOCTL_PATROL_TQ_OP_GET,
	UDB_IOCTL_PATROL_TQ_OP_SET,
	UDB_IOCTL_PATROL_TQ_OP_GET_LOG,
	UDB_IOCTL_PATROL_TQ_OP_GET_TIME,
	UDB_IOCTL_PATROL_TQ_OP_RESET,
	UDB_IOCTL_PATROL_TQ_OP_ENABLE,
	UDB_IOCTL_PATROL_TQ_OP_DISABLE,
	UDB_IOCTL_PATROL_TQ_OP_MAX
};

#define MAX_PATROL_TQ_GRP TMCFG_E_UDB_CORE_PATROL_TIME_GRP_NUM
#define MAX_PATROL_TQ_DEV TMCFG_E_UDB_CORE_PATROL_TIME_DEV_NUM
#define MAX_PATROL_TQ_SIZE (sizeof(patrol_ioc_tq_t) + (sizeof(patrol_ioc_tq_grp_t) * MAX_PATROL_TQ_GRP) \
	+ (sizeof(patrol_ioc_tq_dev_t)* MAX_PATROL_TQ_GRP * MAX_PATROL_TQ_DEV))

typedef struct patrol_ioc_dev
{
       uint16_t grp_id; /*Range 1~32*/
       uint8_t mac[6];
} patrol_ioc_tq_dev_t;

typedef struct patrol_ioc_tq_grp
{
	uint16_t grp_id; /*Range 1~32*/
	uint16_t pro_id; /*cat group id*/
	uint8_t is_update; /* 1->Startup  policy 0->Stop policy */
	uint8_t day_id;  /*1~7 follow schedule per day configure*/
	uint8_t action; /*time-quota action 0-> allow 1->block*/
	uint8_t dev_cnt; /*max 1 ~ 6*/
	uint16_t time_quota; /*profile category limited(Minutes)*/
	uint32_t used_time;      /*During-time (Second)*/
	uint64_t bit_cat_id; /*unsigned long per-bit [cat-id 1~64]*/
	uint64_t down_quota_pkt;   /*Reserve it */
	uint64_t up_quota_pkt;             /*Reserve it */
	uint64_t down_recent_pkt;  /*Reserve it */
	uint64_t up_recent_pkt;    /*Reserve it */
	uint64_t last_uptime;   /*only for kernel*/
} patrol_ioc_tq_grp_t;

typedef struct patrol_ioc_tq
{
	uint16_t grp_cnt;
	uint16_t rsv;
	uint64_t up_time; /*Only for User space checked */
} patrol_ioc_tq_t;

/* log */
typedef struct patrol_tq_list_ioc_entry
{
        uint8_t uid;
        uint8_t mac[6];
        uint8_t cat_id;
        uint16_t app_id;
        uint64_t time;
        uint8_t flag;
        uint16_t grp_id;
        uint8_t available;
} patrol_tq_list_ioc_entry_t;

typedef struct app_time_ioc_entry
{
	uint8_t uid;
	uint8_t cat_id;
	uint16_t app_id;
	uint32_t used_time_sec;

	uint8_t available;
} app_time_ioctl_entry_t;

#ifdef __KERNEL__
int udb_ioctl_patrol_tq_op_copy_out(uint8_t op, void *buf, uint32_t buf_len, uint32_t *buf_used_len);
int udb_ioctl_patrol_tq_op_copy_in(uint8_t op, void *buf, uint32_t buf_len);
int udb_ioctl_patrol_tq_op_copy_none(uint8_t op);
#endif

#endif /* _UDB_IOCTL_PATROL_TQ_H_ */
