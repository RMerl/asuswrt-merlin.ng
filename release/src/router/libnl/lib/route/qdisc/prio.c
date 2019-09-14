/*
 * lib/route/qdisc/prio.c		PRIO Qdisc/Class
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2003-2011 Thomas Graf <tgraf@suug.ch>
 */

/**
 * @ingroup qdisc
 * @defgroup qdisc_prio (Fast) Prio
 * @brief
 *
 * @par 1) Typical PRIO configuration
 * @code
 * // Specify the maximal number of bands to be used for this PRIO qdisc.
 * rtnl_qdisc_prio_set_bands(qdisc, QDISC_PRIO_DEFAULT_BANDS);
 *
 * // Provide a map assigning each priority to a band number.
 * uint8_t map[] = QDISC_PRIO_DEFAULT_PRIOMAP;
 * rtnl_qdisc_prio_set_priomap(qdisc, map, sizeof(map));
 * @endcode
 * @{
 */

#include <netlink-private/netlink.h>
#include <netlink-private/tc.h>
#include <netlink/netlink.h>
#include <netlink/utils.h>
#include <netlink-private/route/tc-api.h>
#include <netlink/route/qdisc.h>
#include <netlink/route/qdisc/prio.h>

/** @cond SKIP */
#define SCH_PRIO_ATTR_BANDS	1
#define SCH_PRIO_ATTR_PRIOMAP	2
/** @endcond */

static int prio_msg_parser(struct rtnl_tc *tc, void *data)
{
	struct rtnl_prio *prio = data;
	struct tc_prio_qopt *opt;

	if (tc->tc_opts->d_size < sizeof(*opt))
		return -NLE_INVAL;

	opt = (struct tc_prio_qopt *) tc->tc_opts->d_data;
	prio->qp_bands = opt->bands;
	memcpy(prio->qp_priomap, opt->priomap, sizeof(prio->qp_priomap));
	prio->qp_mask = (SCH_PRIO_ATTR_BANDS | SCH_PRIO_ATTR_PRIOMAP);
	
	return 0;
}

static void prio_dump_line(struct rtnl_tc *tc, void *data,
			   struct nl_dump_params *p)
{
	struct rtnl_prio *prio = data;

	if (prio)
		nl_dump(p, " bands %u", prio->qp_bands);
}

static void prio_dump_details(struct rtnl_tc *tc, void *data,
			      struct nl_dump_params *p)
{
	struct rtnl_prio *prio = data;
	int i, hp;

	if (!prio)
		return;

	nl_dump(p, "priomap [");
	
	for (i = 0; i <= TC_PRIO_MAX; i++)
		nl_dump(p, "%u%s", prio->qp_priomap[i],
			i < TC_PRIO_MAX ? " " : "");

	nl_dump(p, "]\n");
	nl_new_line(p);

	hp = (((TC_PRIO_MAX/2) + 1) & ~1);

	for (i = 0; i < hp; i++) {
		char a[32];
		nl_dump(p, "    %18s => %u",
			rtnl_prio2str(i, a, sizeof(a)),
			prio->qp_priomap[i]);
		if (hp+i <= TC_PRIO_MAX) {
			nl_dump(p, " %18s => %u",
				rtnl_prio2str(hp+i, a, sizeof(a)),
				prio->qp_priomap[hp+i]);
			if (i < (hp - 1)) {
				nl_dump(p, "\n");
				nl_new_line(p);
			}
		}
	}
}

static int prio_msg_fill(struct rtnl_tc *tc, void *data, struct nl_msg *msg)
{
	struct rtnl_prio *prio = data;
	struct tc_prio_qopt opts;

	if (!prio || !(prio->qp_mask & SCH_PRIO_ATTR_PRIOMAP))
		BUG();

	opts.bands = prio->qp_bands;
	memcpy(opts.priomap, prio->qp_priomap, sizeof(opts.priomap));

	return nlmsg_append(msg, &opts, sizeof(opts), NL_DONTPAD);
}

/**
 * @name Attribute Modification
 * @{
 */

/**
 * Set number of bands of PRIO qdisc.
 * @arg qdisc		PRIO qdisc to be modified.
 * @arg bands		New number of bands.
 * @return 0 on success or a negative error code.
 */
void rtnl_qdisc_prio_set_bands(struct rtnl_qdisc *qdisc, int bands)
{
	struct rtnl_prio *prio;

	if (!(prio = rtnl_tc_data(TC_CAST(qdisc))))
		BUG();

	prio->qp_bands = bands;
	prio->qp_mask |= SCH_PRIO_ATTR_BANDS;
}

/**
 * Get number of bands of PRIO qdisc.
 * @arg qdisc		PRIO qdisc.
 * @return Number of bands or a negative error code.
 */
int rtnl_qdisc_prio_get_bands(struct rtnl_qdisc *qdisc)
{
	struct rtnl_prio *prio;

	if (!(prio = rtnl_tc_data(TC_CAST(qdisc))))
		BUG();

	if (prio->qp_mask & SCH_PRIO_ATTR_BANDS)
		return prio->qp_bands;
	else
		return -NLE_NOMEM;
}

/**
 * Set priomap of the PRIO qdisc.
 * @arg qdisc		PRIO qdisc to be modified.
 * @arg priomap		New priority mapping.
 * @arg len		Length of priomap (# of elements).
 * @return 0 on success or a negative error code.
 */
int rtnl_qdisc_prio_set_priomap(struct rtnl_qdisc *qdisc, uint8_t priomap[],
				int len)
{
	struct rtnl_prio *prio;
	int i;

	if (!(prio = rtnl_tc_data(TC_CAST(qdisc))))
		BUG();

	if (!(prio->qp_mask & SCH_PRIO_ATTR_BANDS))
		return -NLE_MISSING_ATTR;

	if ((len / sizeof(uint8_t)) > (TC_PRIO_MAX+1))
		return -NLE_RANGE;

	for (i = 0; i <= TC_PRIO_MAX; i++) {
		if (priomap[i] > prio->qp_bands)
			return -NLE_RANGE;
	}

	memcpy(prio->qp_priomap, priomap, len);
	prio->qp_mask |= SCH_PRIO_ATTR_PRIOMAP;

	return 0;
}

/**
 * Get priomap of a PRIO qdisc.
 * @arg qdisc		PRIO qdisc.
 * @return Priority mapping as array of size TC_PRIO_MAX+1
 *         or NULL if an error occured.
 */
uint8_t *rtnl_qdisc_prio_get_priomap(struct rtnl_qdisc *qdisc)
{
	struct rtnl_prio *prio;

	if (!(prio = rtnl_tc_data(TC_CAST(qdisc))))
		BUG();

	if (prio->qp_mask & SCH_PRIO_ATTR_PRIOMAP)
		return prio->qp_priomap;
	else
		return NULL;
}

/** @} */

/**
 * @name Priority Band Translations
 * @{
 */

static const struct trans_tbl prios[] = {
	__ADD(TC_PRIO_BESTEFFORT,besteffort)
	__ADD(TC_PRIO_FILLER,filler)
	__ADD(TC_PRIO_BULK,bulk)
	__ADD(TC_PRIO_INTERACTIVE_BULK,interactive_bulk)
	__ADD(TC_PRIO_INTERACTIVE,interactive)
	__ADD(TC_PRIO_CONTROL,control)
};

/**
 * Convert priority to character string.
 * @arg prio		Priority.
 * @arg buf		Destination buffer
 * @arg size		Size of destination buffer.
 *
 * Converts a priority to a character string and stores the result in
 * the specified destination buffer.
 *
 * @return Name of priority as character string.
 */
char * rtnl_prio2str(int prio, char *buf, size_t size)
{
	return __type2str(prio, buf, size, prios, ARRAY_SIZE(prios));
}

/**
 * Convert character string to priority.
 * @arg name		Name of priority.
 *
 * Converts the provided character string specifying a priority
 * to the corresponding numeric value.
 *
 * @return Numeric priority or a negative value if no match was found.
 */
int rtnl_str2prio(const char *name)
{
	return __str2type(name, prios, ARRAY_SIZE(prios));
}

/** @} */

static struct rtnl_tc_ops prio_ops = {
	.to_kind		= "prio",
	.to_type		= RTNL_TC_TYPE_QDISC,
	.to_size		= sizeof(struct rtnl_prio),
	.to_msg_parser		= prio_msg_parser,
	.to_dump = {
	    [NL_DUMP_LINE]	= prio_dump_line,
	    [NL_DUMP_DETAILS]	= prio_dump_details,
	},
	.to_msg_fill		= prio_msg_fill,
};

static struct rtnl_tc_ops pfifo_fast_ops = {
	.to_kind		= "pfifo_fast",
	.to_type		= RTNL_TC_TYPE_QDISC,
	.to_size		= sizeof(struct rtnl_prio),
	.to_msg_parser		= prio_msg_parser,
	.to_dump = {
	    [NL_DUMP_LINE]	= prio_dump_line,
	    [NL_DUMP_DETAILS]	= prio_dump_details,
	},
	.to_msg_fill		= prio_msg_fill,
};

static void __init prio_init(void)
{
	rtnl_tc_register(&prio_ops);
	rtnl_tc_register(&pfifo_fast_ops);
}

static void __exit prio_exit(void)
{
	rtnl_tc_unregister(&prio_ops);
	rtnl_tc_unregister(&pfifo_fast_ops);
}

/** @} */
