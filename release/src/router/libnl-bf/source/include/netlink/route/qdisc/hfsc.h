/*
 * netlink/route/sch/hfsc.h	HFSC Qdisc
 */

#ifndef NETLINK_HFSC_H_
#define NETLINK_HFSC_H_

#include <netlink/netlink.h>
#include <netlink/route/tc.h>

#ifdef __cplusplus
extern "C" {
#endif

struct hfsc_spec {
	unsigned int rate;
	unsigned int delay_max;
	unsigned int work_max;
};

extern uint32_t	rtnl_hfsc_get_defcls(struct rtnl_qdisc *);
extern int	rtnl_hfsc_set_defcls(struct rtnl_qdisc *, uint32_t);

/* m1-d-m2: Direct Method of Specifying service curve slopes */
extern void	rtnl_hfsc_get_realtime_sc(struct rtnl_class *,
					  struct tc_service_curve *);
extern int	rtnl_hfsc_set_realtime_sc(struct rtnl_class *,
					  struct tc_service_curve *);
extern void	rtnl_hfsc_get_fair_sc(struct rtnl_class *,
				      struct tc_service_curve *);
extern int	rtnl_hfsc_set_fair_sc(struct rtnl_class *,
				      struct tc_service_curve *);
extern void	rtnl_hfsc_get_upperlimit_sc(struct rtnl_class *,
					    struct tc_service_curve *);
extern int	rtnl_hfsc_set_upperlimit_sc(struct rtnl_class *,
					    struct tc_service_curve *);

/* umax-dmax-rate Method of specifying service curve translations */
extern int	rtnl_hfsc_spec_to_sc(struct hfsc_spec *,
				     struct tc_service_curve *);

#ifdef __cplusplus
}
#endif

#endif
