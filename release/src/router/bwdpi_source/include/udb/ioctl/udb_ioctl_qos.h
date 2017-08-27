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

#ifndef _UDB_IOCTL_QOS_H_
#define _UDB_IOCTL_QOS_H_

#include "udb/ioctl/udb_ioctl_common.h"

enum
{
	UDB_IOCTL_IQOS_OP_NA = 0,
	UDB_IOCTL_IQOS_OP_ENABLE,
	UDB_IOCTL_IQOS_OP_DISABLE,
	UDB_IOCTL_IQOS_OP_GET_CONFIG,
	UDB_IOCTL_IQOS_OP_SET_CONFIG,
	UDB_IOCTL_IQOS_OP_GET_APP_INFO,
	UDB_IOCTL_IQOS_OP_MAX
};

typedef struct app_qos_ioc_entry
{
	uint8_t uid;
	uint8_t cat_id;
	uint16_t app_id;

	uint64_t down_recent_accl;
	uint64_t up_recent_accl;

//#define APP_QOS_FLAG_NONE 0
//#define APP_QOS_FLAG_RESERVED 1
	uint32_t qos_flag;
	uint8_t paid;
	uint16_t bndwidth;

	uint8_t available;
} app_qos_ioctl_t;

#ifndef IFACE_NAME_SIZE
#define IFACE_NAME_SIZE 16
#endif

typedef struct qos_conf_ioctl
{
	char wan[IFACE_NAME_SIZE];
	char lan[IFACE_NAME_SIZE];
	uint32_t bw_dl;
	uint32_t bw_ul;
} qos_conf_ioctl_t;

#ifdef __KERNEL__
int udb_ioctl_qos_op_copy_out(uint8_t op, void *buf, uint32_t buf_len, uint32_t *buf_used_len);
int udb_ioctl_qos_op_copy_in(uint8_t op, void *buf, uint32_t buf_len);
int udb_ioctl_qos_op_copy_none(uint8_t op);
#endif

#endif /* _UDB_IOCTL_QOS_H_ */

