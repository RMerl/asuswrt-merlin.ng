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

#ifndef _UDB_IOCTL_MESH_H_
#define _UDB_IOCTL_MESH_H_

#include "udb/tmcfg_udb.h"

#define MESH_EXT_MAX	100
#define MESH_USER_MAX	253

enum
{
	UDB_IOCTL_MESH_OP_NA = 0,
	UDB_IOCTL_MESH_OP_SET_USER,
	UDB_IOCTL_MESH_OP_GET_USER,
	UDB_IOCTL_MESH_OP_SET_EXTENDER,
	UDB_IOCTL_MESH_OP_GET_EXTENDER,
	UDB_IOCTL_MESH_OP_MAX
};

enum
{
	ACT_NA = 0,
	ACT_ADD,
	ACT_UPDATE,
	ACT_DELETE,
	ACT_MAX
};

/*ioctl_mesh_op_set_user*/
typedef struct mesh_user_ioc_entry
{
	uint8_t mac[6];
	uint8_t ipv4[4];
	uint8_t action;
	uint8_t rsv;
} mesh_user_ioc_entry_t;

/*ioctl_mesh_op_get_user*/
typedef struct mesh_user_ioc_list
{
	uint32_t entry_cnt;
	mesh_user_ioc_entry_t entry[0];
} mesh_user_ioc_list_t;

/*ioctl_mesh_op_set_extender*/
typedef struct mesh_ext_ioc_entry
{
	uint8_t mac[6];
	uint8_t action;
	uint8_t rsv;
} mesh_ext_ioc_entry_t;

/*ioctl_mesh_op_get_extender*/
typedef struct mesh_ext_ioc_list
{
	uint32_t entry_cnt;
	mesh_ext_ioc_entry_t entry[0];
} mesh_ext_ioc_list_t;

#ifdef __KERNEL__
int udb_ioctl_mesh_op_copy_out(uint8_t op, void *buf, uint32_t buf_len, uint32_t *buf_used_len);
int udb_ioctl_mesh_op_copy_in(uint8_t op, void *buf, uint32_t buf_len);
int udb_ioctl_mesh_op_copy_none(uint8_t op);
#endif

#endif /* _UDB_IOCTL_MESH_H_ */


