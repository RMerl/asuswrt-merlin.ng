/* SPDX-License-Identifier: GPL-2.0+
  *
  * Copyright 2019 Broadcom Ltd.
 */
#ifndef _ITC_MSG_DEFS_H_
#define _ITC_MSG_DEFS_H_

#include <stddef.h>
#include <stdint.h>
#include <linux/types.h>

typedef struct {
	uint32_t header;
	uint32_t data[3];
} rpc_msg;

#define RPC_MAX_FUNCTIONS	256

/*
 * Message header encoding in first 32bit word
 * |   6   |   1   |   1   |   8   |   8    |   8   |
 *  Version Request  Reply  Service Function Counter
 */

static inline uint8_t rpc_msg_version(rpc_msg *msg)
{
	return ((msg->header >> 26) & 0x3f);
}
static inline uint8_t rpc_msg_request(rpc_msg *msg)
{
	return ((msg->header >> 25) & 0x01);
}
static inline uint8_t rpc_msg_reply(rpc_msg *msg)
{
	return ((msg->header >> 24) & 0x01);
}
static inline uint8_t rpc_msg_service(rpc_msg *msg)
{
	return ((msg->header >> 16) & 0xff);
}
static inline uint8_t rpc_msg_function(rpc_msg *msg)
{
	return ((msg->header >>  8) & 0xff);
}
static inline uint8_t rpc_msg_counter(rpc_msg *msg)
{
	return ((msg->header >>  0) & 0xff);
}

/* xid = all except request and reply bits */
static inline uint32_t rpc_msg_xid(rpc_msg *msg)
{
	return (msg->header & ~(3 << 24));
}

static inline void rpc_msg_set_version(rpc_msg *msg, uint8_t v)
{
	(msg->header = (msg->header & ~(0x3f << 26)) | (v << 26));
}
static inline void rpc_msg_set_request(rpc_msg *msg, uint8_t v)
{
	(msg->header = (msg->header & ~(0x01 << 25)) | (v << 25));
}
static inline void rpc_msg_set_reply(rpc_msg *msg, uint8_t v)
{
	(msg->header = (msg->header & ~(0x01 << 24)) | (v << 24));
}
static inline void rpc_msg_set_service(rpc_msg *msg, uint8_t v)
{
	(msg->header = (msg->header & ~(0xff << 16)) | (v << 16));
}
static inline void rpc_msg_set_function(rpc_msg *msg, uint8_t v)
{
	(msg->header = (msg->header & ~(0xff <<  8)) | (v <<  8));
}
static inline void rpc_msg_set_counter(rpc_msg *msg, uint8_t v)
{
	(msg->header = (msg->header & ~(0xff <<  0)) | (v <<  0));
}

#define RPC_MSG_MAKE_HEADER(version, req, rep, serv, func, reqcnt)( \
		(version << 26) |	\
		(req << 25) |	\
		(rep << 24) |	\
		(serv << 16) |	\
		(func << 8) |	\
		(reqcnt) )

#define RPC_MSG_INIT_CODE0		0xbeef0000
#define RPC_MSG_INIT_CODE1		0xbeef0001
#define RPC_MSG_INIT_CODE2		0xbeef0002

#endif
