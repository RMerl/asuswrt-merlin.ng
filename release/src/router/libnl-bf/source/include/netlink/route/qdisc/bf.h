/*
 * netlink/route/sch/bf.h	BF Qdisc
 */
/*
 **************************************************************************
 * Copyright (c) 2015, The Linux Foundation. All rights reserved.
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all copies.
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 **************************************************************************
 */

#ifndef NETLINK_BF_H_
#define NETLINK_BF_H_

#include <netlink/netlink.h>
#include <netlink/route/tc.h>

#ifdef __cplusplus
extern "C" {
#endif

extern uint32_t	rtnl_bf_get_defcls(struct rtnl_qdisc *);
extern int	rtnl_bf_set_defcls(struct rtnl_qdisc *, uint32_t);
extern uint32_t rtnl_bf_get_flow_priorities(struct rtnl_qdisc *);
extern uint32_t rtnl_bf_get_node_priorities(struct rtnl_qdisc *);
extern int rtnl_bf_set_priorities(struct rtnl_qdisc *, uint32_t flow_prios,
				  uint32_t node_prios);
extern enum BF_PRIORITY_CALC rtnl_bf_get_prio_calc_method(struct rtnl_qdisc *);
extern int rtnl_bf_set_prio_calc_method(struct rtnl_qdisc *,
					enum BF_PRIORITY_CALC);
extern uint32_t rtnl_bf_get_total_bandwidth(struct rtnl_qdisc *);
extern int rtnl_bf_set_total_bandwidth(struct rtnl_qdisc *, uint32_t);

extern uint32_t	rtnl_bf_get_flow_prio(struct rtnl_class *);
extern int	rtnl_bf_set_flow_prio(struct rtnl_class *, uint32_t);
extern uint32_t	rtnl_bf_get_node_prio(struct rtnl_class *);
extern int	rtnl_bf_set_node_prio(struct rtnl_class *, uint32_t);
extern uint32_t	rtnl_bf_get_rate(struct rtnl_class *, uint32_t);
extern int	rtnl_bf_set_rates(struct rtnl_class *, uint32_t *);

#ifdef __cplusplus
}
#endif

#endif
