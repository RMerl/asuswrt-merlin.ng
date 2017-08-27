/*
 * netlink/route/sch/codel.h     CODEL Qdisc
 */

#ifndef NETLINK_CODEL_H_
#define NETLINK_CODEL_H_

#include <netlink/netlink.h>
#include <netlink/route/tc.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Target Delay in usecs - Default 5 * 1000us */
extern int	rtnl_codel_qdisc_get_target_usecs(struct rtnl_qdisc *);
extern int	rtnl_codel_qdisc_set_target_usecs(struct rtnl_qdisc *,
						  uint32_t);
/* Maximum number of enqueued packets before tail drop begins */
extern int	rtnl_codel_qdisc_get_packet_limit(struct rtnl_qdisc *);
extern int	rtnl_codel_qdisc_set_packet_limit(struct rtnl_qdisc *,
						  uint32_t);
/* Moving window for calculating target delay in usecs - Default 100 * 1000us */
extern int	rtnl_codel_qdisc_get_interval(struct rtnl_qdisc *);
extern int	rtnl_codel_qdisc_set_interval(struct rtnl_qdisc *, uint32_t);
/* Determines whether or not drops can be emulated via ECN - Default False*/
extern int	rtnl_codel_qdisc_get_ecn(struct rtnl_qdisc *);
extern int	rtnl_codel_qdisc_set_ecn(struct rtnl_qdisc *, uint32_t);

#ifdef __cplusplus
}
#endif

#endif
