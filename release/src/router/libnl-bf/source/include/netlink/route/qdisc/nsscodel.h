/*
 * netlink/route/sch/nsscodel.h	NSSCODEL Qdisc
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

#ifndef NETLINK_NSSCODEL_H_
#define NETLINK_NSSCODEL_H_

#include <netlink/netlink.h>
#include <netlink/route/tc.h>

#ifdef __cplusplus
extern "C" {
#endif

extern int rtnl_nsscodel_set_limit(struct rtnl_qdisc *qdisc, uint32_t limit);
extern uint32_t rtnl_nsscodel_get_limit(struct rtnl_qdisc *qdisc);

extern int rtnl_nsscodel_set_target(struct rtnl_qdisc *qdisc, uint32_t target);
extern uint32_t rtnl_nsscodel_get_target(struct rtnl_qdisc *qdisc);

extern int rtnl_nsscodel_set_interval(struct rtnl_qdisc *qdisc, uint32_t interval);
extern uint32_t rtnl_nsscodel_get_interval(struct rtnl_qdisc *qdisc);

extern int rtnl_nsscodel_set_default(struct rtnl_qdisc *qdisc, uint8_t set_default);
extern uint8_t rtnl_nsscodel_get_default(struct rtnl_qdisc *qdisc);

#ifdef __cplusplus
}
#endif

#endif
