/*
 * netlink/route/sch/drr.c	Deficit Round Robin Qdisc
 */

#ifndef NETLINK_DRR_H_
#define NETLINK_DRR_H_

#include <netlink/netlink.h>

#ifdef __cplusplus
extern "C" {
#endif

extern int rtnl_class_drr_set_quantum(struct rtnl_qdisc *, uint32_t);
extern uint32_t rtnl_class_drr_get_quantum(struct rtnl_qdisc *);

#ifdef __cplusplus
}
#endif

#endif
