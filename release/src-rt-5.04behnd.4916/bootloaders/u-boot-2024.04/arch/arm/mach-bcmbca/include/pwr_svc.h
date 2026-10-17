/* SPDX-License-Identifier: GPL-2.0+
  *
  * Copyright 2019 Broadcom Ltd.
 */

#ifndef _PWR_SVC_H_
#define _PWR_SVC_H_

#include <linux/types.h>
#include "itc_rpc.h"

enum pwr_svc_func_idx {
	PWR_SVC_DOMAIN_ID,
	PWR_SVC_DOMAIN_NAME,
	PWR_SVC_GET_DOMAIN_STATE,
	PWR_SVC_SET_DOMAIN_STATE,
	PWR_SVC_FUNC_MAX
};

struct pwr_msg {
	uint32_t hdr;
	struct {
		uint8_t id;
		uint8_t state;
		struct {
			uint8_t reset:2;
			uint8_t rsvd:6;
		};
		uint8_t rc;
	};
	char name[8];
};

#define PWR_SVC_MAX_ID (1 << (sizeof(((struct pwr_msg *)0)->id) << 3))

enum pwr_dom_state {
	PWR_DOM_STATE_OFF       = 0,
	PWR_DOM_STATE_ON        = 0xf,
	PWR_DOM_STATE_UNCHANGED = 0xfe,
	PWR_DOM_STATE_UNKNOWN   = 0xff
};

enum pwr_dom_reset {
	PWR_DOM_RESET_DEASSERT  = 0,
	PWR_DOM_RESET_ASSERT    = 1,
	PWR_DOM_RESET_PULSE     = 2,
	PWR_DOM_RESET_MAX
};

/* pwr_svc rpc message manipulation helpers */
static inline uint8_t pwr_svc_msg_get_retcode(rpc_msg *msg)
{
	struct pwr_msg *pwr_msg = (struct pwr_msg *)msg;
	return pwr_msg->rc;
}
static inline void pwr_svc_msg_set_retcode(rpc_msg *msg, uint8_t v)
{
	struct pwr_msg *pwr_msg = (struct pwr_msg *)msg;
	pwr_msg->rc = v;
}

/* pwr svc functions */
int pwr_svc_domain_id(char *name, uint8_t *id);
int pwr_svc_domain_name(uint8_t id, char *name);
int pwr_svc_get_domain_state(uint8_t id, enum pwr_dom_state *state,
			     enum pwr_dom_reset *reset);
int pwr_svc_set_domain_state(uint8_t id, enum pwr_dom_state state,
			     enum pwr_dom_reset reset);

#endif
