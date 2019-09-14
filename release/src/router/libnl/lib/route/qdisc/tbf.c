/*
 * lib/route/qdisc/tbf.c		TBF Qdisc
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
 * @defgroup qdisc_tbf Token Bucket Filter (TBF)
 * @{
 */

#include <netlink-private/netlink.h>
#include <netlink-private/tc.h>
#include <netlink/netlink.h>
#include <netlink/cache.h>
#include <netlink/utils.h>
#include <netlink-private/route/tc-api.h>
#include <netlink/route/qdisc.h>
#include <netlink/route/class.h>
#include <netlink/route/link.h>
#include <netlink/route/qdisc/tbf.h>

/** @cond SKIP */
#define TBF_ATTR_LIMIT			0x01
#define TBF_ATTR_RATE			0x02
#define TBF_ATTR_PEAKRATE		0x10
/** @endcond */

static struct nla_policy tbf_policy[TCA_TBF_MAX+1] = {
	[TCA_TBF_PARMS]	= { .minlen = sizeof(struct tc_tbf_qopt) },
};

static int tbf_msg_parser(struct rtnl_tc *tc, void *data)
{
	struct nlattr *tb[TCA_TBF_MAX + 1];
	struct rtnl_tbf *tbf = data;
	int err;

	if ((err = tca_parse(tb, TCA_TBF_MAX, tc, tbf_policy)) < 0)
		return err;
	
	if (tb[TCA_TBF_PARMS]) {
		struct tc_tbf_qopt opts;
		int bufsize;

		nla_memcpy(&opts, tb[TCA_TBF_PARMS], sizeof(opts));
		tbf->qt_limit = opts.limit;
	
		rtnl_copy_ratespec(&tbf->qt_rate, &opts.rate);
		tbf->qt_rate_txtime = opts.buffer;
		bufsize = rtnl_tc_calc_bufsize(nl_ticks2us(opts.buffer),
					       opts.rate.rate);
		tbf->qt_rate_bucket = bufsize;

		rtnl_copy_ratespec(&tbf->qt_peakrate, &opts.peakrate);
		tbf->qt_peakrate_txtime = opts.mtu;
		bufsize = rtnl_tc_calc_bufsize(nl_ticks2us(opts.mtu),
					       opts.peakrate.rate);
		tbf->qt_peakrate_bucket = bufsize;

		rtnl_tc_set_mpu(tc, tbf->qt_rate.rs_mpu);
		rtnl_tc_set_overhead(tc, tbf->qt_rate.rs_overhead);

		tbf->qt_mask = (TBF_ATTR_LIMIT | TBF_ATTR_RATE | TBF_ATTR_PEAKRATE);
	}

	return 0;
}

static void tbf_dump_line(struct rtnl_tc *tc, void *data,
			  struct nl_dump_params *p)
{
	double r, rbit, lim;
	char *ru, *rubit, *limu;
	struct rtnl_tbf *tbf = data;

	if (!tbf)
		return;

	r = nl_cancel_down_bytes(tbf->qt_rate.rs_rate, &ru);
	rbit = nl_cancel_down_bits(tbf->qt_rate.rs_rate*8, &rubit);
	lim = nl_cancel_down_bytes(tbf->qt_limit, &limu);

	nl_dump(p, " rate %.2f%s/s (%.0f%s) limit %.2f%s",
		r, ru, rbit, rubit, lim, limu);
}

static void tbf_dump_details(struct rtnl_tc *tc, void *data,
			     struct nl_dump_params *p)
{
	struct rtnl_tbf *tbf = data;

	if (!tbf)
		return;

	if (1) {
		char *bu, *cu;
		double bs = nl_cancel_down_bytes(tbf->qt_rate_bucket, &bu);
		double cl = nl_cancel_down_bytes(1 << tbf->qt_rate.rs_cell_log,
						 &cu);

		nl_dump(p, "rate-bucket-size %1.f%s "
			   "rate-cell-size %.1f%s\n",
			bs, bu, cl, cu);

	}

	if (tbf->qt_mask & TBF_ATTR_PEAKRATE) {
		char *pru, *prbu, *bsu, *clu;
		double pr, prb, bs, cl;
		
		pr = nl_cancel_down_bytes(tbf->qt_peakrate.rs_rate, &pru);
		prb = nl_cancel_down_bits(tbf->qt_peakrate.rs_rate * 8, &prbu);
		bs = nl_cancel_down_bits(tbf->qt_peakrate_bucket, &bsu);
		cl = nl_cancel_down_bits(1 << tbf->qt_peakrate.rs_cell_log,
					 &clu);

		nl_dump_line(p, "    peak-rate %.2f%s/s (%.0f%s) "
				"bucket-size %.1f%s cell-size %.1f%s"
				"latency %.1f%s",
			     pr, pru, prb, prbu, bs, bsu, cl, clu);
	}
}

static int tbf_msg_fill(struct rtnl_tc *tc, void *data, struct nl_msg *msg)
{
	uint32_t rtab[RTNL_TC_RTABLE_SIZE], ptab[RTNL_TC_RTABLE_SIZE];
	struct tc_tbf_qopt opts;
	struct rtnl_tbf *tbf = data;
	int required = TBF_ATTR_RATE | TBF_ATTR_LIMIT;

	if ((tbf->qt_mask & required) != required)
		return -NLE_MISSING_ATTR;

	memset(&opts, 0, sizeof(opts));
	opts.limit = tbf->qt_limit;
	opts.buffer = tbf->qt_rate_txtime;

	rtnl_tc_build_rate_table(tc, &tbf->qt_rate, rtab);
	rtnl_rcopy_ratespec(&opts.rate, &tbf->qt_rate);

	if (tbf->qt_mask & TBF_ATTR_PEAKRATE) {
		opts.mtu = tbf->qt_peakrate_txtime;
		rtnl_tc_build_rate_table(tc, &tbf->qt_peakrate, ptab);
		rtnl_rcopy_ratespec(&opts.peakrate, &tbf->qt_peakrate);

	}

	NLA_PUT(msg, TCA_TBF_PARMS, sizeof(opts), &opts);
	NLA_PUT(msg, TCA_TBF_RTAB, sizeof(rtab), rtab);

	if (tbf->qt_mask & TBF_ATTR_PEAKRATE)
		NLA_PUT(msg, TCA_TBF_PTAB, sizeof(ptab), ptab);

	return 0;

nla_put_failure:
	return -NLE_MSGSIZE;
}

/**
 * @name Attribute Access
 * @{
 */

/**
 * Set limit of TBF qdisc.
 * @arg qdisc		TBF qdisc to be modified.
 * @arg limit		New limit in bytes.
 * @return 0 on success or a negative error code.
 */
void rtnl_qdisc_tbf_set_limit(struct rtnl_qdisc *qdisc, int limit)
{
	struct rtnl_tbf *tbf;
	
	if (!(tbf = rtnl_tc_data(TC_CAST(qdisc))))
		BUG();

	tbf->qt_limit = limit;
	tbf->qt_mask |= TBF_ATTR_LIMIT;
}

static inline double calc_limit(struct rtnl_ratespec *spec, int latency,
				int bucket)
{
	double limit;

	limit = (double) spec->rs_rate * ((double) latency / 1000000.);
	limit += bucket;

	return limit;
}

/**
 * Set limit of TBF qdisc by latency.
 * @arg qdisc		TBF qdisc to be modified.
 * @arg latency		Latency in micro seconds.
 *
 * Calculates and sets the limit based on the desired latency and the
 * configured rate and peak rate. In order for this operation to succeed,
 * the rate and if required the peak rate must have been set in advance.
 *
 * @f[
 *   limit_n = \frac{{rate_n} \times {latency}}{10^6}+{bucketsize}_n
 * @f]
 * @f[
 *   limit = min(limit_{rate},limit_{peak})
 * @f]
 * 
 * @return 0 on success or a negative error code.
 */
int rtnl_qdisc_tbf_set_limit_by_latency(struct rtnl_qdisc *qdisc, int latency)
{
	struct rtnl_tbf *tbf;
	double limit, limit2;

	if (!(tbf = rtnl_tc_data(TC_CAST(qdisc))))
		BUG();

	if (!(tbf->qt_mask & TBF_ATTR_RATE))
		return -NLE_MISSING_ATTR;

	limit = calc_limit(&tbf->qt_rate, latency, tbf->qt_rate_bucket);

	if (tbf->qt_mask & TBF_ATTR_PEAKRATE) {
		limit2 = calc_limit(&tbf->qt_peakrate, latency,
				    tbf->qt_peakrate_bucket);

		if (limit2 < limit)
			limit = limit2;
	}

	rtnl_qdisc_tbf_set_limit(qdisc, (int) limit);

	return 0;
}

/**
 * Get limit of TBF qdisc.
 * @arg qdisc		TBF qdisc.
 * @return Limit in bytes or a negative error code.
 */
int rtnl_qdisc_tbf_get_limit(struct rtnl_qdisc *qdisc)
{
	struct rtnl_tbf *tbf;
	
	if (!(tbf = rtnl_tc_data(TC_CAST(qdisc))))
		BUG();

	if (tbf->qt_mask & TBF_ATTR_LIMIT)
		return tbf->qt_limit;
	else
		return -NLE_NOATTR;
}

static inline int calc_cell_log(int cell, int bucket)
{
		cell = rtnl_tc_calc_cell_log(cell);
	return cell;
}

/**
 * Set rate of TBF qdisc.
 * @arg qdisc		TBF qdisc to be modified.
 * @arg rate		New rate in bytes per second.
 * @arg bucket		Size of bucket in bytes.
 * @arg cell		Size of a rate cell or 0 to get default value.
 * @return 0 on success or a negative error code.
 */
void rtnl_qdisc_tbf_set_rate(struct rtnl_qdisc *qdisc, int rate, int bucket,
			    int cell)
{
	struct rtnl_tbf *tbf;
	int cell_log;
	
	if (!(tbf = rtnl_tc_data(TC_CAST(qdisc))))
		BUG();

	if (!cell)
		cell_log = UINT8_MAX;
	else
		cell_log = rtnl_tc_calc_cell_log(cell);

	tbf->qt_rate.rs_rate = rate;
	tbf->qt_rate_bucket = bucket;
	tbf->qt_rate.rs_cell_log = cell_log;
	tbf->qt_rate_txtime = nl_us2ticks(rtnl_tc_calc_txtime(bucket, rate));
	tbf->qt_mask |= TBF_ATTR_RATE;
}

/**
 * Get rate of TBF qdisc.
 * @arg qdisc		TBF qdisc.
 * @return Rate in bytes per seconds or a negative error code.
 */
int rtnl_qdisc_tbf_get_rate(struct rtnl_qdisc *qdisc)
{
	struct rtnl_tbf *tbf;

	if (!(tbf = rtnl_tc_data(TC_CAST(qdisc))))
		BUG();

	if (tbf->qt_mask & TBF_ATTR_RATE)
		return tbf->qt_rate.rs_rate;
	else
		return -1;
}

/**
 * Get rate bucket size of TBF qdisc.
 * @arg qdisc		TBF qdisc.
 * @return Size of rate bucket or a negative error code.
 */
int rtnl_qdisc_tbf_get_rate_bucket(struct rtnl_qdisc *qdisc)
{
	struct rtnl_tbf *tbf;

	if (!(tbf = rtnl_tc_data(TC_CAST(qdisc))))
		BUG();

	if (tbf->qt_mask & TBF_ATTR_RATE)
		return tbf->qt_rate_bucket;
	else
		return -1;
}

/**
 * Get rate cell size of TBF qdisc.
 * @arg qdisc		TBF qdisc.
 * @return Size of rate cell in bytes or a negative error code.
 */
int rtnl_qdisc_tbf_get_rate_cell(struct rtnl_qdisc *qdisc)
{
	struct rtnl_tbf *tbf;

	if (!(tbf = rtnl_tc_data(TC_CAST(qdisc))))
		BUG();

	if (tbf->qt_mask & TBF_ATTR_RATE)
		return (1 << tbf->qt_rate.rs_cell_log);
	else
		return -1;
}

/**
 * Set peak rate of TBF qdisc.
 * @arg qdisc		TBF qdisc to be modified.
 * @arg rate		New peak rate in bytes per second.
 * @arg bucket		Size of peakrate bucket.
 * @arg cell		Size of a peakrate cell or 0 to get default value.
 * @return 0 on success or a negative error code.
 */
int rtnl_qdisc_tbf_set_peakrate(struct rtnl_qdisc *qdisc, int rate, int bucket,
				int cell)
{
	struct rtnl_tbf *tbf;
	int cell_log;
	
	if (!(tbf = rtnl_tc_data(TC_CAST(qdisc))))
		BUG();

	cell_log = calc_cell_log(cell, bucket);
	if (cell_log < 0)
		return cell_log;

	tbf->qt_peakrate.rs_rate = rate;
	tbf->qt_peakrate_bucket = bucket;
	tbf->qt_peakrate.rs_cell_log = cell_log;
	tbf->qt_peakrate_txtime = nl_us2ticks(rtnl_tc_calc_txtime(bucket, rate));
	
	tbf->qt_mask |= TBF_ATTR_PEAKRATE;

	return 0;
}

/**
 * Get peak rate of TBF qdisc.
 * @arg qdisc		TBF qdisc.
 * @return Peak rate in bytes per seconds or a negative error code.
 */
int rtnl_qdisc_tbf_get_peakrate(struct rtnl_qdisc *qdisc)
{
	struct rtnl_tbf *tbf;

	if (!(tbf = rtnl_tc_data(TC_CAST(qdisc))))
		BUG();

	if (tbf->qt_mask & TBF_ATTR_PEAKRATE)
		return tbf->qt_peakrate.rs_rate;
	else
		return -1;
}

/**
 * Get peak rate bucket size of TBF qdisc.
 * @arg qdisc		TBF qdisc.
 * @return Size of peak rate bucket or a negative error code.
 */
int rtnl_qdisc_tbf_get_peakrate_bucket(struct rtnl_qdisc *qdisc)
{
	struct rtnl_tbf *tbf;

	if (!(tbf = rtnl_tc_data(TC_CAST(qdisc))))
		BUG();

	if (tbf->qt_mask & TBF_ATTR_PEAKRATE)
		return tbf->qt_peakrate_bucket;
	else
		return -1;
}

/**
 * Get peak rate cell size of TBF qdisc.
 * @arg qdisc		TBF qdisc.
 * @return Size of peak rate cell in bytes or a negative error code.
 */
int rtnl_qdisc_tbf_get_peakrate_cell(struct rtnl_qdisc *qdisc)
{
	struct rtnl_tbf *tbf;

	if (!(tbf = rtnl_tc_data(TC_CAST(qdisc))))
		BUG();

	if (tbf->qt_mask & TBF_ATTR_PEAKRATE)
		return (1 << tbf->qt_peakrate.rs_cell_log);
	else
		return -1;
}

/** @} */

static struct rtnl_tc_ops tbf_tc_ops = {
	.to_kind		= "tbf",
	.to_type		= RTNL_TC_TYPE_QDISC,
	.to_size		= sizeof(struct rtnl_tbf),
	.to_msg_parser		= tbf_msg_parser,
	.to_dump = {
	    [NL_DUMP_LINE]	= tbf_dump_line,
	    [NL_DUMP_DETAILS]	= tbf_dump_details,
	},
	.to_msg_fill		= tbf_msg_fill,
};

static void __init tbf_init(void)
{
	rtnl_tc_register(&tbf_tc_ops);
}

static void __exit tbf_exit(void)
{
	rtnl_tc_unregister(&tbf_tc_ops);
}

/** @} */
