/*
 **************************************************************************
 * Copyright (c) 2014, The Linux Foundation. All rights reserved.
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

#ifndef NETLINK_NSSHTB_H_
#define NETLINK_NSSHTB_H_

#include <netlink/netlink.h>
#include <netlink/route/tc.h>

#ifdef __cplusplus
extern "C" {
#endif

extern int rtnl_nsshtb_qdisc_set_r2q(struct rtnl_qdisc *qdisc, uint16_t r2q);
extern uint32_t rtnl_nsshtb_qdisc_get_r2q(struct rtnl_qdisc *qdisc);

extern int rtnl_nsshtb_class_set_rate(struct rtnl_class *class, uint32_t rate);
extern uint32_t rtnl_nsshtb_class_get_rate(struct rtnl_class *class);

extern int rtnl_nsshtb_class_set_burst(struct rtnl_class *class, uint32_t burst);
extern uint32_t rtnl_nsshtb_class_get_burst(struct rtnl_class *class);

extern int rtnl_nsshtb_class_set_crate(struct rtnl_class *class, uint32_t crate);
extern uint32_t rtnl_nsshtb_class_get_crate(struct rtnl_class *class);

extern int rtnl_nsshtb_class_set_cburst(struct rtnl_class *class, uint32_t cburst);
extern uint32_t rtnl_nsshtb_class_get_cburst(struct rtnl_class *class);

extern int rtnl_nsshtb_class_set_quantum(struct rtnl_class *class, uint32_t q);
extern uint32_t rtnl_nsshtb_class_get_quantum(struct rtnl_class *class);

extern int rtnl_nsshtb_class_set_priority(struct rtnl_class *class, uint32_t priority);
extern uint32_t rtnl_nsshtb_class_get_priority(struct rtnl_class *class);

extern int rtnl_nsshtb_class_set_overhead(struct rtnl_class *class, uint32_t overhead);
extern uint32_t rtnl_nsshtb_class_get_overhead(struct rtnl_class *class);

#ifdef __cplusplus
}
#endif

#endif
