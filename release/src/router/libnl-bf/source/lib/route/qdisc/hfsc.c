/*
 * lib/route/qdisc/hfsc.c	HFSC Qdisc
 */

/**
 * @ingroup qdisc
 * @ingroup class
 * @defgroup qdisc_hfsc Hierachical Fair Service Curve (HFSC)
 * @{
 */

#include <netlink-local.h>
#include <netlink-tc.h>
#include <netlink/netlink.h>
#include <netlink/cache.h>
#include <netlink/utils.h>
#include <netlink/route/tc-api.h>
#include <netlink/route/qdisc.h>
#include <netlink/route/class.h>
#include <netlink/route/link.h>
#include <netlink/route/qdisc/hfsc.h>

#define USEC_PER_SEC		1000000

/** @cond SKIP */
#define SCH_HFSC_HAS_DEFCLS		0x02

#define SCH_HFSC_HAS_RT_SC		0x001
#define SCH_HFSC_HAS_FAIR_SC		0x002
#define SCH_HFSC_HAS_UPPER_SC		0x004
/** @endcond */

static struct nla_policy hfsc_policy[TCA_HFSC_MAX + 1] = {
	[TCA_HFSC_RSC]	= { .minlen = sizeof(struct tc_service_curve) },
	[TCA_HFSC_FSC]	= { .minlen = sizeof(struct tc_service_curve) },
	[TCA_HFSC_USC]	= { .minlen = sizeof(struct tc_service_curve) },
};

static int hfsc_qdisc_msg_parser(struct rtnl_tc *tc, void *data)
{
	struct rtnl_hfsc_qdisc *hfsc = data;
	struct tc_hfsc_qopt *opt;

	if (tc->tc_opts->d_size < sizeof(struct tc_hfsc_qopt))
		return -NLE_INVAL;

	opt = (struct tc_hfsc_qopt *) tc->tc_opts->d_data;
	hfsc->qsc_defcls = opt->defcls;
	hfsc->qsc_mask = SCH_HFSC_HAS_DEFCLS;

	return 0;
}

static int hfsc_class_msg_parser(struct rtnl_tc *tc, void *data)
{
	struct nlattr *tb[TCA_HFSC_MAX + 1];
	struct rtnl_hfsc_class *hfsc = data;
	int err;

	if ((err = tca_parse(tb, TCA_HFSC_MAX, tc, hfsc_policy)) < 0)
		return err;

	hfsc->csc_mask = 0;
	if (tb[TCA_HFSC_RSC]) {
		nla_memcpy(&hfsc->csc_rt_sc, tb[TCA_HFSC_RSC],
			   sizeof(struct tc_service_curve));
		hfsc->csc_mask |= SCH_HFSC_HAS_RT_SC;
	}

	if (tb[TCA_HFSC_FSC]) {
		nla_memcpy(&hfsc->csc_fair_sc, tb[TCA_HFSC_FSC],
			   sizeof(struct tc_service_curve));
		hfsc->csc_mask |= SCH_HFSC_HAS_FAIR_SC;
	}

	if (tb[TCA_HFSC_USC]) {
		nla_memcpy(&hfsc->csc_upper_sc, tb[TCA_HFSC_USC],
			   sizeof(struct tc_service_curve));
		hfsc->csc_mask |= SCH_HFSC_HAS_UPPER_SC;
	}

	return 0;
}

static void hfsc_qdisc_dump_line(struct rtnl_tc *tc, void *data,
				 struct nl_dump_params *p)
{
	struct rtnl_hfsc_qdisc *hfsc = data;

	if (!hfsc)
		return;

	if (hfsc->qsc_mask & SCH_HFSC_HAS_DEFCLS) {
		char buf[64];
		nl_dump(p, " default-class %s",
			rtnl_tc_handle2str(hfsc->qsc_defcls, buf, sizeof(buf)));
	}
}


static void hfsc_print_sc(struct nl_dump_params *p, char *name,
			  struct rtnl_curve *sc)
{
	double val;
	char *unit;

	nl_dump(p, "%s ", name);
	val = nl_cancel_down_bits(sc->sc_m1, &unit);
	nl_dump(p, " m1 %.2f%s/s", val, unit);

	val = nl_cancel_down_us(sc->sc_d, &unit);
        nl_dump(p, " d %.2f%s", val, unit);

	val = nl_cancel_down_bits(sc->sc_m2, &unit);
	nl_dump(p, " m2 %.2f%s/s", val, unit);
}


static void hfsc_class_dump_line(struct rtnl_tc *tc, void *data,
				 struct nl_dump_params *p)
{
	struct rtnl_hfsc_class *hfsc = data;

	if (!hfsc)
		return;

	if ((hfsc->csc_mask & SCH_HFSC_HAS_RT_SC) &&
	    (hfsc->csc_mask & SCH_HFSC_HAS_FAIR_SC) &&
	    (memcmp(&hfsc->csc_rt_sc,
		    &hfsc->csc_fair_sc,
		    sizeof(hfsc->csc_rt_sc)) == 0)) {
                hfsc_print_sc(p, "sc", &hfsc->csc_rt_sc);
	} else {
		if (hfsc->csc_mask & SCH_HFSC_HAS_RT_SC)
                	hfsc_print_sc(p, "rt", &hfsc->csc_rt_sc);
		if (hfsc->csc_mask & SCH_HFSC_HAS_FAIR_SC)
                	hfsc_print_sc(p, "ls", &hfsc->csc_fair_sc);
        }

	if (hfsc->csc_mask & SCH_HFSC_HAS_UPPER_SC)
               	hfsc_print_sc(p, "ul", &hfsc->csc_upper_sc);
}


static int hfsc_qdisc_msg_fill(struct rtnl_tc *tc, void *data,
			       struct nl_msg *msg)
{
	struct rtnl_hfsc_qdisc *hfsc = data;
        struct tc_hfsc_qopt opts = {0};

	if (!hfsc || !(hfsc->qsc_mask & SCH_HFSC_HAS_DEFCLS))
		return -NLE_INVAL;

	opts.defcls = hfsc->qsc_defcls;

	return nlmsg_append(msg, &opts, sizeof(opts), NL_DONTPAD);
}

/* FIXME: Verify we have at least one curve, and if upper, must have fair */
static int hfsc_class_msg_fill(struct rtnl_tc *tc, void *data,
			       struct nl_msg *msg)
{
	struct rtnl_hfsc_class *hfsc = data;
	struct tc_service_curve sc;

	if (!hfsc)
		BUG();

	if (hfsc->csc_mask & SCH_HFSC_HAS_RT_SC) {
		memcpy(&sc, &hfsc->csc_rt_sc, sizeof(sc));
	} else {
		memset(&sc, 0, sizeof(sc));
	}
	NLA_PUT(msg, TCA_HFSC_RSC, sizeof(sc), &sc);

	if (hfsc->csc_mask & SCH_HFSC_HAS_FAIR_SC) {
		memcpy(&sc, &hfsc->csc_fair_sc, sizeof(sc));
	} else {
		memset(&sc, 0, sizeof(sc));
	}
	NLA_PUT(msg, TCA_HFSC_FSC, sizeof(sc), &sc);

	if (hfsc->csc_mask & SCH_HFSC_HAS_UPPER_SC) {
		memcpy(&sc, &hfsc->csc_upper_sc, sizeof(sc));
	} else {
		memset(&sc, 0, sizeof(sc));
	}
	NLA_PUT(msg, TCA_HFSC_USC, sizeof(sc), &sc);

	return 0;

nla_put_failure:
	return -NLE_MSGSIZE;
}

static struct rtnl_tc_ops hfsc_qdisc_ops;
static struct rtnl_tc_ops hfsc_class_ops;

static struct rtnl_hfsc_qdisc *hfsc_qdisc_data(struct rtnl_qdisc *qdisc)
{
	return rtnl_tc_data_check(TC_CAST(qdisc), &hfsc_qdisc_ops);
}

static struct rtnl_hfsc_class *hfsc_class_data(struct rtnl_class *class)
{
	return rtnl_tc_data_check(TC_CAST(class), &hfsc_class_ops);
}

/**
 * @name Attribute Modifications
 * @{
 */

/**
 * Return default class of HFSC qdisc
 * @arg qdisc		hfsc qdisc object
 *
 * Returns the classid of the class where all unclassified traffic
 * goes to.
 *
 * @return classid or TC_H_UNSPEC if unspecified.
 */
uint32_t rtnl_hfsc_get_defcls(struct rtnl_qdisc *qdisc)
{
	struct rtnl_hfsc_qdisc *hfsc;

	if ((hfsc = hfsc_qdisc_data(qdisc)) &&
	    hfsc->qsc_mask & SCH_HFSC_HAS_DEFCLS)
		return hfsc->qsc_defcls;

	return TC_H_UNSPEC;
}

/**
 * Set default class of the hfsc qdisc to the specified value
 * @arg qdisc		qdisc to change
 * @arg defcls		new default class
 */
int rtnl_hfsc_set_defcls(struct rtnl_qdisc *qdisc, uint32_t defcls)
{
	struct rtnl_hfsc_qdisc *hfsc;

	if (!(hfsc = hfsc_qdisc_data(qdisc)))
		return -NLE_OPNOTSUPP;

	hfsc->qsc_defcls = defcls;
	hfsc->qsc_mask |= SCH_HFSC_HAS_DEFCLS;

	return 0;
}

/* m1-d-m2: Direct Method of Specifying service curve slopes */
void rtnl_hfsc_get_realtime_sc(struct rtnl_class *cls,
			       struct tc_service_curve *curve_out)
{
	struct rtnl_hfsc_class *hfsc;

	if ((hfsc = hfsc_class_data(cls)) &&
	    (hfsc->csc_mask & SCH_HFSC_HAS_RT_SC)) {
		memcpy(curve_out, &hfsc->csc_rt_sc, sizeof(*curve_out));
	} else {
		memset(curve_out, 0, sizeof(*curve_out));
	}
}


int rtnl_hfsc_set_realtime_sc(struct rtnl_class *cls,
			      struct tc_service_curve *curve_in)
{
	struct rtnl_hfsc_class *hfsc;

	if (!(hfsc = hfsc_class_data(cls)))
		return -NLE_OPNOTSUPP;

	memcpy(&hfsc->csc_rt_sc, curve_in, sizeof(hfsc->csc_rt_sc));
	hfsc->csc_mask |= SCH_HFSC_HAS_RT_SC;

	return 0;
}

void rtnl_hfsc_get_fair_sc(struct rtnl_class *cls,
			   struct tc_service_curve *curve_out)
{
	struct rtnl_hfsc_class *hfsc;

	if ((hfsc = hfsc_class_data(cls)) &&
	    (hfsc->csc_mask & SCH_HFSC_HAS_FAIR_SC)) {
		memcpy(curve_out, &hfsc->csc_fair_sc, sizeof(*curve_out));
	} else {
		memset(curve_out, 0, sizeof(*curve_out));
	}
}


int rtnl_hfsc_set_fair_sc(struct rtnl_class *cls,
			  struct tc_service_curve *curve_in)
{
	struct rtnl_hfsc_class *hfsc;

	if (!(hfsc = hfsc_class_data(cls)))
		return -NLE_OPNOTSUPP;

	memcpy(&hfsc->csc_fair_sc, curve_in, sizeof(hfsc->csc_fair_sc));
	hfsc->csc_mask |= SCH_HFSC_HAS_FAIR_SC;

	return 0;
}

void rtnl_hfsc_get_upperlimit_sc(struct rtnl_class *cls,
				 struct tc_service_curve *curve_out)
{
	struct rtnl_hfsc_class *hfsc;

	if ((hfsc = hfsc_class_data(cls)) &&
	    (hfsc->csc_mask & SCH_HFSC_HAS_UPPER_SC)) {
		memcpy(curve_out, &hfsc->csc_upper_sc, sizeof(*curve_out));
	} else {
		memset(curve_out, 0, sizeof(*curve_out));
	}
}


int rtnl_hfsc_set_upperlimit_sc(struct rtnl_class *cls,
				struct tc_service_curve *curve_in)
{
	struct rtnl_hfsc_class *hfsc;

	if (!(hfsc = hfsc_class_data(cls)))
		return -NLE_OPNOTSUPP;

	memcpy(&hfsc->csc_upper_sc, curve_in, sizeof(hfsc->csc_upper_sc));
	hfsc->csc_mask |= SCH_HFSC_HAS_UPPER_SC;

	return 0;
}

/* umax-dmax-rate Method of specifying service curve translations */
int rtnl_hfsc_spec_to_sc(struct hfsc_spec *spec_in,
			 struct tc_service_curve *curve_out)
{
	unsigned int umax, dmax, rate;

	umax = spec_in->work_max;
	dmax = spec_in->delay_max;
	rate = spec_in->rate;

	if (umax != 0 && dmax == 0) {
		return -EINVAL;
	}

	if (rate == 0) {
		curve_out->m1 	= 0;
		curve_out->d	= 0;
		curve_out->m2	= 0;
		return 0;
	}

	/* concave: slope of first segment is umax/dmax, intersect at dmax */
	if ((dmax != 0) && (ceil(1.0 * umax * USEC_PER_SEC / dmax) > rate)) {
		curve_out->m1	= ceil(1.0 * umax * USEC_PER_SEC / dmax);
		curve_out->d	= dmax;
		curve_out->m2	= rate;
	} else {
	/* convex: slope of first segment = 0, intersect (dmax - umax / rate */
	/* FIXME: is that calc correct?  Maybe it's (dmax - umax) / rate? */
		curve_out->m1	= 0;
		curve_out->d	= ceil(dmax - umax * USEC_PER_SEC / rate);
		curve_out->m2	= rate;
	}

	return 0;
}

/** @} */

static struct rtnl_tc_ops hfsc_qdisc_ops = {
	.to_kind		= "hfsc",
	.to_type		= RTNL_TC_TYPE_QDISC,
	.to_size		= sizeof(struct rtnl_hfsc_qdisc),
	.to_msg_parser		= hfsc_qdisc_msg_parser,
	.to_dump[NL_DUMP_LINE]	= hfsc_qdisc_dump_line,
	.to_msg_fill		= hfsc_qdisc_msg_fill,
};

static struct rtnl_tc_ops hfsc_class_ops = {
	.to_kind		= "hfsc",
	.to_type		= RTNL_TC_TYPE_CLASS,
	.to_size		= sizeof(struct rtnl_hfsc_class),
	.to_msg_parser		= hfsc_class_msg_parser,
	.to_dump[NL_DUMP_LINE]	= hfsc_class_dump_line,
	.to_msg_fill		= hfsc_class_msg_fill,
};

static void __init hfsc_init(void)
{
	rtnl_tc_register(&hfsc_qdisc_ops);
	rtnl_tc_register(&hfsc_class_ops);
}

static void __exit hfsc_exit(void)
{
	rtnl_tc_unregister(&hfsc_qdisc_ops);
	rtnl_tc_unregister(&hfsc_class_ops);
}
