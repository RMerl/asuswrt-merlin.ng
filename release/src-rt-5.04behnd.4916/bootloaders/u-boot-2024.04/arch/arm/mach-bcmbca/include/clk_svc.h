/* SPDX-License-Identifier: GPL-2.0+
  *
  * Copyright 2019 Broadcom Ltd.
 */

#ifndef _CLK_SVC_H_
#define _CLK_SVC_H_

#include <linux/types.h>
#include "itc_rpc.h"

enum clk_svc_func_idx {
	CLK_SVC_DOMAIN_ID,
	CLK_SVC_DOMAIN_NAME,
	CLK_SVC_GET_DOMAIN_STATE,
	CLK_SVC_SET_DOMAIN_STATE,
	CLK_SVC_FUNC_MAX
};

#define CLK_SVC_NAME_CHARS 9
struct clk_msg {
	uint32_t hdr;
	struct {
		uint8_t id;
		struct {
			uint8_t enable:1;
			uint8_t rsvd0:7;
		};
		uint8_t rsvd1;
		uint8_t rc;
	};
	union {
		char name[CLK_SVC_NAME_CHARS];
		struct {
			uint32_t rate; /* kHz */
			uint32_t rsvd2;
		};
	};
};

#define CLK_SVC_MAX_ID (1 << (sizeof(((struct clk_msg *)0)->id) << 3))

/* clk_svc rpc message manipulation helpers */
static inline uint8_t clk_svc_msg_get_retcode(rpc_msg *msg)
{
	struct clk_msg *clk_msg = (struct clk_msg *)msg;
	return clk_msg->rc;
}
static inline void clk_svc_msg_set_retcode(rpc_msg *msg, uint8_t v)
{
	struct clk_msg *clk_msg = (struct clk_msg *)msg;
	clk_msg->rc = v;
}

/* clk svc functions */
int clk_svc_domain_id(const char *name, uint8_t *id);
int clk_svc_domain_name(uint8_t id, char *name);
int clk_svc_get_domain_state(uint8_t id, bool *enabled, uint32_t *rate);
int clk_svc_set_domain_state(uint8_t id, bool enable, uint32_t rate);
int clk_svc_rate_get(uint8_t id, uint64_t *rate64);
int clk_svc_rate_set(uint8_t id, uint64_t rate64);
int clk_svc_id_by_name(const char *name);

#endif
