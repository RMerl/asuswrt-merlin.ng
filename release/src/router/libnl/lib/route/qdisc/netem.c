/*
 * lib/route/qdisc/netem.c		Network Emulator Qdisc
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
 * @defgroup qdisc_netem Network Emulator
 * @brief
 *
 * For further documentation see http://linux-net.osdl.org/index.php/Netem
 * @{
 */

#include <netlink-private/netlink.h>
#include <netlink-private/tc.h>
#include <netlink/netlink.h>
#include <netlink/utils.h>
#include <netlink-private/route/tc-api.h>
#include <netlink/route/qdisc.h>
#include <netlink/route/qdisc/netem.h>

/** @cond SKIP */
#define SCH_NETEM_ATTR_LATENCY		0x0001
#define SCH_NETEM_ATTR_LIMIT		0x0002
#define SCH_NETEM_ATTR_LOSS			0x0004
#define SCH_NETEM_ATTR_GAP			0x0008
#define SCH_NETEM_ATTR_DUPLICATE	0x0010
#define SCH_NETEM_ATTR_JITTER		0x0020
#define SCH_NETEM_ATTR_DELAY_CORR	0x0040
#define SCH_NETEM_ATTR_LOSS_CORR	0x0080
#define SCH_NETEM_ATTR_DUP_CORR		0x0100
#define SCH_NETEM_ATTR_RO_PROB		0x0200
#define SCH_NETEM_ATTR_RO_CORR		0x0400
#define SCH_NETEM_ATTR_CORRUPT_PROB	0x0800
#define SCH_NETEM_ATTR_CORRUPT_CORR	0x1000
#define SCH_NETEM_ATTR_DIST         0x2000
/** @endcond */

static struct nla_policy netem_policy[TCA_NETEM_MAX+1] = {
	[TCA_NETEM_CORR]	= { .minlen = sizeof(struct tc_netem_corr) },
	[TCA_NETEM_REORDER]	= { .minlen = sizeof(struct tc_netem_reorder) },
	[TCA_NETEM_CORRUPT]	= { .minlen = sizeof(struct tc_netem_corrupt) },
};

static int netem_msg_parser(struct rtnl_tc *tc, void *data)
{
	struct rtnl_netem *netem = data;
	struct tc_netem_qopt *opts;
	int len, err = 0;

	if (tc->tc_opts->d_size < sizeof(*opts))
		return -NLE_INVAL;

	opts = (struct tc_netem_qopt *) tc->tc_opts->d_data;
	netem->qnm_latency = opts->latency;
	netem->qnm_limit = opts->limit;
	netem->qnm_loss = opts->loss;
	netem->qnm_gap = opts->gap;
	netem->qnm_duplicate = opts->duplicate;
	netem->qnm_jitter = opts->jitter;

	netem->qnm_mask = (SCH_NETEM_ATTR_LATENCY | SCH_NETEM_ATTR_LIMIT |
			   SCH_NETEM_ATTR_LOSS | SCH_NETEM_ATTR_GAP |
			   SCH_NETEM_ATTR_DUPLICATE | SCH_NETEM_ATTR_JITTER);

	len = tc->tc_opts->d_size - sizeof(*opts);

	if (len > 0) {
		struct nlattr *tb[TCA_NETEM_MAX+1];

		err = nla_parse(tb, TCA_NETEM_MAX, (struct nlattr *)
				(tc->tc_opts->d_data + sizeof(*opts)),
				len, netem_policy);
		if (err < 0) {
			free(netem);
			return err;
		}

		if (tb[TCA_NETEM_CORR]) {
			struct tc_netem_corr cor;

			nla_memcpy(&cor, tb[TCA_NETEM_CORR], sizeof(cor));
			netem->qnm_corr.nmc_delay = cor.delay_corr;
			netem->qnm_corr.nmc_loss = cor.loss_corr;
			netem->qnm_corr.nmc_duplicate = cor.dup_corr;

			netem->qnm_mask |= (SCH_NETEM_ATTR_DELAY_CORR |
					    SCH_NETEM_ATTR_LOSS_CORR |
					SCH_NETEM_ATTR_DUP_CORR);
		}

		if (tb[TCA_NETEM_REORDER]) {
			struct tc_netem_reorder ro;

			nla_memcpy(&ro, tb[TCA_NETEM_REORDER], sizeof(ro));
			netem->qnm_ro.nmro_probability = ro.probability;
			netem->qnm_ro.nmro_correlation = ro.correlation;

			netem->qnm_mask |= (SCH_NETEM_ATTR_RO_PROB |
					    SCH_NETEM_ATTR_RO_CORR);
		}
			
		if (tb[TCA_NETEM_CORRUPT]) {
			struct tc_netem_corrupt corrupt;
						
			nla_memcpy(&corrupt, tb[TCA_NETEM_CORRUPT], sizeof(corrupt));
			netem->qnm_crpt.nmcr_probability = corrupt.probability;
			netem->qnm_crpt.nmcr_correlation = corrupt.correlation;
	
			netem->qnm_mask |= (SCH_NETEM_ATTR_CORRUPT_PROB |
						SCH_NETEM_ATTR_CORRUPT_CORR);
		}
		
		/* sch_netem does not currently dump TCA_NETEM_DELAY_DIST */
		netem->qnm_dist.dist_data = NULL;
		netem->qnm_dist.dist_size = 0;
	}

	return 0;
}

static void netem_free_data(struct rtnl_tc *tc, void *data)
{
	struct rtnl_netem *netem = data;
	
	if (!netem)
		return;
	
	free(netem->qnm_dist.dist_data);
}

static void netem_dump_line(struct rtnl_tc *tc, void *data,
			    struct nl_dump_params *p)
{
	struct rtnl_netem *netem = data;

	if (netem)
		nl_dump(p, "limit %d", netem->qnm_limit);
}

static int netem_msg_fill_raw(struct rtnl_tc *tc, void *data,
			      struct nl_msg *msg)
{
	int err = 0;
	struct tc_netem_qopt opts;
	struct tc_netem_corr cor;
	struct tc_netem_reorder reorder;
	struct tc_netem_corrupt corrupt;
	struct rtnl_netem *netem = data;
	
	unsigned char set_correlation = 0, set_reorder = 0,
		set_corrupt = 0, set_dist = 0;

	if (!netem)
		BUG();

	memset(&opts, 0, sizeof(opts));
	memset(&cor, 0, sizeof(cor));
	memset(&reorder, 0, sizeof(reorder));
	memset(&corrupt, 0, sizeof(corrupt));

	msg->nm_nlh->nlmsg_flags |= NLM_F_REQUEST;
	
	if ( netem->qnm_ro.nmro_probability != 0 ) {
		if (netem->qnm_latency == 0) {
			return -NLE_MISSING_ATTR;
		}
		if (netem->qnm_gap == 0) netem->qnm_gap = 1;
	}
	else if ( netem->qnm_gap ) { 
		return -NLE_MISSING_ATTR;
	}

	if ( netem->qnm_corr.nmc_delay != 0 ) {
		if ( netem->qnm_latency == 0 || netem->qnm_jitter == 0) {
			return -NLE_MISSING_ATTR;
		}
		set_correlation = 1;
	}
	
	if ( netem->qnm_corr.nmc_loss != 0 ) {
		if ( netem->qnm_loss == 0 ) {
			return -NLE_MISSING_ATTR;
		}
		set_correlation = 1;
	}

	if ( netem->qnm_corr.nmc_duplicate != 0 ) {
		if ( netem->qnm_duplicate == 0 ) {
			return -NLE_MISSING_ATTR;
		}
		set_correlation = 1;
	}
	
	if ( netem->qnm_ro.nmro_probability != 0 ) set_reorder = 1;
	else if ( netem->qnm_ro.nmro_correlation != 0 ) {
			return -NLE_MISSING_ATTR;
	}
	
	if ( netem->qnm_crpt.nmcr_probability != 0 ) set_corrupt = 1;
	else if ( netem->qnm_crpt.nmcr_correlation != 0 ) {
			return -NLE_MISSING_ATTR;
	}
	
	if ( netem->qnm_dist.dist_data && netem->qnm_dist.dist_size ) {
		if (netem->qnm_latency == 0 || netem->qnm_jitter == 0) {
			return -NLE_MISSING_ATTR;
	}
	else {
		/* Resize to accomodate the large distribution table */
		int new_msg_len = msg->nm_size + netem->qnm_dist.dist_size *
			sizeof(netem->qnm_dist.dist_data[0]);
		
		msg->nm_nlh = (struct nlmsghdr *) realloc(msg->nm_nlh, new_msg_len);
		if ( msg->nm_nlh == NULL )
			return -NLE_NOMEM;
		msg->nm_size = new_msg_len;
			set_dist = 1;
		}
	}
	
	opts.latency = netem->qnm_latency;
	opts.limit = netem->qnm_limit ? netem->qnm_limit : 1000;
	opts.loss = netem->qnm_loss;
	opts.gap = netem->qnm_gap;
	opts.duplicate = netem->qnm_duplicate;
	opts.jitter = netem->qnm_jitter;
	
	NLA_PUT(msg, TCA_OPTIONS, sizeof(opts), &opts);
	
	if ( set_correlation ) {
		cor.delay_corr = netem->qnm_corr.nmc_delay;
		cor.loss_corr = netem->qnm_corr.nmc_loss;
		cor.dup_corr = netem->qnm_corr.nmc_duplicate;

		NLA_PUT(msg, TCA_NETEM_CORR, sizeof(cor), &cor);
	}
	
	if ( set_reorder ) {
		reorder.probability = netem->qnm_ro.nmro_probability;
		reorder.correlation = netem->qnm_ro.nmro_correlation;

		NLA_PUT(msg, TCA_NETEM_REORDER, sizeof(reorder), &reorder);
	}
	
	if ( set_corrupt ) {
		corrupt.probability = netem->qnm_crpt.nmcr_probability;
		corrupt.correlation = netem->qnm_crpt.nmcr_correlation;

		NLA_PUT(msg, TCA_NETEM_CORRUPT, sizeof(corrupt), &corrupt);
	}
	
	if ( set_dist ) {
		NLA_PUT(msg, TCA_NETEM_DELAY_DIST,
			netem->qnm_dist.dist_size * sizeof(netem->qnm_dist.dist_data[0]),
			netem->qnm_dist.dist_data);
	}

	/* Length specified in the TCA_OPTIONS section must span the entire
	 * remainder of the message. That's just the way that sch_netem expects it.
	 * Maybe there's a more succinct way to do this at a higher level.
	 */
	struct nlattr* head = (struct nlattr *)(NLMSG_DATA(msg->nm_nlh) +
		NLMSG_LENGTH(sizeof(struct tcmsg)) - NLMSG_ALIGNTO);
		
	struct nlattr* tail = (struct nlattr *)(((void *) (msg->nm_nlh)) +
		NLMSG_ALIGN(msg->nm_nlh->nlmsg_len));
	
	int old_len = head->nla_len;
	head->nla_len = (void *)tail - (void *)head;
	msg->nm_nlh->nlmsg_len += (head->nla_len - old_len);
	
	return err;
nla_put_failure:
	return -NLE_MSGSIZE;
}

/**
 * @name Queue Limit
 * @{
 */

/**
 * Set limit of netem qdisc.
 * @arg qdisc		Netem qdisc to be modified.
 * @arg limit		New limit in bytes.
 * @return 0 on success or a negative error code.
 */
void rtnl_netem_set_limit(struct rtnl_qdisc *qdisc, int limit)
{
	struct rtnl_netem *netem;

	if (!(netem = rtnl_tc_data(TC_CAST(qdisc))))
		BUG();
	
	netem->qnm_limit = limit;
	netem->qnm_mask |= SCH_NETEM_ATTR_LIMIT;
}

/**
 * Get limit of netem qdisc.
 * @arg qdisc		Netem qdisc.
 * @return Limit in bytes or a negative error code.
 */
int rtnl_netem_get_limit(struct rtnl_qdisc *qdisc)
{
	struct rtnl_netem *netem;

	if (!(netem = rtnl_tc_data(TC_CAST(qdisc))))
		return -NLE_NOMEM;

	if (netem->qnm_mask & SCH_NETEM_ATTR_LIMIT)
		return netem->qnm_limit;
	else
		return -NLE_NOATTR;
}

/** @} */

/**
 * @name Packet Re-ordering
 * @{
 */

/**
 * Set re-ordering gap of netem qdisc.
 * @arg qdisc		Netem qdisc to be modified.
 * @arg gap		New gap in number of packets.
 * @return 0 on success or a negative error code.
 */
void rtnl_netem_set_gap(struct rtnl_qdisc *qdisc, int gap)
{
	struct rtnl_netem *netem;

	if (!(netem = rtnl_tc_data(TC_CAST(qdisc))))
		BUG();

	netem->qnm_gap = gap;
	netem->qnm_mask |= SCH_NETEM_ATTR_GAP;
}

/**
 * Get re-ordering gap of netem qdisc.
 * @arg qdisc		Netem qdisc.
 * @return Re-ordering gap in packets or a negative error code.
 */
int rtnl_netem_get_gap(struct rtnl_qdisc *qdisc)
{
	struct rtnl_netem *netem;

	if (!(netem = rtnl_tc_data(TC_CAST(qdisc))))
		return -NLE_NOMEM;

	if (netem->qnm_mask & SCH_NETEM_ATTR_GAP)
		return netem->qnm_gap;
	else
		return -NLE_NOATTR;
}

/**
 * Set re-ordering probability of netem qdisc.
 * @arg qdisc		Netem qdisc to be modified.
 * @arg prob		New re-ordering probability.
 * @return 0 on success or a negative error code.
 */
void rtnl_netem_set_reorder_probability(struct rtnl_qdisc *qdisc, int prob)
{
	struct rtnl_netem *netem;

	if (!(netem = rtnl_tc_data(TC_CAST(qdisc))))
		BUG();

	netem->qnm_ro.nmro_probability = prob;
	netem->qnm_mask |= SCH_NETEM_ATTR_RO_PROB;
}

/**
 * Get re-ordering probability of netem qdisc.
 * @arg qdisc		Netem qdisc.
 * @return Re-ordering probability or a negative error code.
 */
int rtnl_netem_get_reorder_probability(struct rtnl_qdisc *qdisc)
{
	struct rtnl_netem *netem;

	if (!(netem = rtnl_tc_data(TC_CAST(qdisc))))
		return -NLE_NOMEM;

	if (netem->qnm_mask & SCH_NETEM_ATTR_RO_PROB)
		return netem->qnm_ro.nmro_probability;
	else
		return -NLE_NOATTR;
}

/**
 * Set re-order correlation probability of netem qdisc.
 * @arg qdisc		Netem qdisc to be modified.
 * @arg prob		New re-ordering correlation probability.
 * @return 0 on success or a negative error code.
 */
void rtnl_netem_set_reorder_correlation(struct rtnl_qdisc *qdisc, int prob)
{
	struct rtnl_netem *netem;

	if (!(netem = rtnl_tc_data(TC_CAST(qdisc))))
		BUG();

	netem->qnm_ro.nmro_correlation = prob;
	netem->qnm_mask |= SCH_NETEM_ATTR_RO_CORR;
}

/**
 * Get re-ordering correlation probability of netem qdisc.
 * @arg qdisc		Netem qdisc.
 * @return Re-ordering correlation probability or a negative error code.
 */
int rtnl_netem_get_reorder_correlation(struct rtnl_qdisc *qdisc)
{
	struct rtnl_netem *netem;

	if (!(netem = rtnl_tc_data(TC_CAST(qdisc))))
		return -NLE_NOMEM;

	if (netem->qnm_mask & SCH_NETEM_ATTR_RO_CORR)
		return netem->qnm_ro.nmro_correlation;
	else
		return -NLE_NOATTR;
}

/** @} */

/**
 * @name Corruption
 * @{
 */
 
/**
 * Set corruption probability of netem qdisc.
 * @arg qdisc		Netem qdisc to be modified.
 * @arg prob		New corruption probability.
 * @return 0 on success or a negative error code.
 */
void rtnl_netem_set_corruption_probability(struct rtnl_qdisc *qdisc, int prob)
{
	struct rtnl_netem *netem;

	if (!(netem = rtnl_tc_data(TC_CAST(qdisc))))
		BUG();

	netem->qnm_crpt.nmcr_probability = prob;
	netem->qnm_mask |= SCH_NETEM_ATTR_CORRUPT_PROB;
}

/**
 * Get corruption probability of netem qdisc.
 * @arg qdisc		Netem qdisc.
 * @return Corruption probability or a negative error code.
 */
int rtnl_netem_get_corruption_probability(struct rtnl_qdisc *qdisc)
{
	struct rtnl_netem *netem;

	if (!(netem = rtnl_tc_data(TC_CAST(qdisc))))
		BUG();

	if (netem->qnm_mask & SCH_NETEM_ATTR_CORRUPT_PROB)
		return netem->qnm_crpt.nmcr_probability;
	else
		return -NLE_NOATTR;
}

/**
 * Set corruption correlation probability of netem qdisc.
 * @arg qdisc		Netem qdisc to be modified.
 * @arg prob		New corruption correlation probability.
 * @return 0 on success or a negative error code.
 */
void rtnl_netem_set_corruption_correlation(struct rtnl_qdisc *qdisc, int prob)
{
	struct rtnl_netem *netem;

	if (!(netem = rtnl_tc_data(TC_CAST(qdisc))))
		BUG();

	netem->qnm_crpt.nmcr_correlation = prob;
	netem->qnm_mask |= SCH_NETEM_ATTR_CORRUPT_CORR;
}

/**
 * Get corruption correlation probability of netem qdisc.
 * @arg qdisc		Netem qdisc.
 * @return Corruption correlation probability or a negative error code.
 */
int rtnl_netem_get_corruption_correlation(struct rtnl_qdisc *qdisc)
{
	struct rtnl_netem *netem;

	if (!(netem = rtnl_tc_data(TC_CAST(qdisc))))
		BUG();

	if (netem->qnm_mask & SCH_NETEM_ATTR_CORRUPT_CORR)
		return netem->qnm_crpt.nmcr_correlation;
	else
		return -NLE_NOATTR;
}

/** @} */

/**
 * @name Packet Loss
 * @{
 */

/**
 * Set packet loss probability of netem qdisc.
 * @arg qdisc		Netem qdisc to be modified.
 * @arg prob		New packet loss probability.
 * @return 0 on success or a negative error code.
 */
void rtnl_netem_set_loss(struct rtnl_qdisc *qdisc, int prob)
{
	struct rtnl_netem *netem;

	if (!(netem = rtnl_tc_data(TC_CAST(qdisc))))
		BUG();

	netem->qnm_loss = prob;
	netem->qnm_mask |= SCH_NETEM_ATTR_LOSS;
}

/**
 * Get packet loss probability of netem qdisc.
 * @arg qdisc		Netem qdisc.
 * @return Packet loss probability or a negative error code.
 */
int rtnl_netem_get_loss(struct rtnl_qdisc *qdisc)
{
	struct rtnl_netem *netem;

	if (!(netem = rtnl_tc_data(TC_CAST(qdisc))))
		BUG();

	if (netem->qnm_mask & SCH_NETEM_ATTR_LOSS)
		return netem->qnm_loss;
	else
		return -NLE_NOATTR;
}

/**
 * Set packet loss correlation probability of netem qdisc.
 * @arg qdisc		Netem qdisc to be modified.
 * @arg prob	New packet loss correlation.
 * @return 0 on success or a negative error code.
 */
void rtnl_netem_set_loss_correlation(struct rtnl_qdisc *qdisc, int prob)
{
	struct rtnl_netem *netem;

	if (!(netem = rtnl_tc_data(TC_CAST(qdisc))))
		BUG();

	netem->qnm_corr.nmc_loss = prob;
	netem->qnm_mask |= SCH_NETEM_ATTR_LOSS_CORR;
}

/**
 * Get packet loss correlation probability of netem qdisc.
 * @arg qdisc		Netem qdisc.
 * @return Packet loss correlation probability or a negative error code.
 */
int rtnl_netem_get_loss_correlation(struct rtnl_qdisc *qdisc)
{
	struct rtnl_netem *netem;

	if (!(netem = rtnl_tc_data(TC_CAST(qdisc))))
		BUG();

	if (netem->qnm_mask & SCH_NETEM_ATTR_LOSS_CORR)
		return netem->qnm_corr.nmc_loss;
	else
		return -NLE_NOATTR;
}

/** @} */

/**
 * @name Packet Duplication
 * @{
 */

/**
 * Set packet duplication probability of netem qdisc.
 * @arg qdisc		Netem qdisc to be modified.
 * @arg prob	New packet duplication probability.
 * @return 0 on success or a negative error code.
 */
void rtnl_netem_set_duplicate(struct rtnl_qdisc *qdisc, int prob)
{
	struct rtnl_netem *netem;

	if (!(netem = rtnl_tc_data(TC_CAST(qdisc))))
		BUG();

	netem->qnm_duplicate = prob;
	netem->qnm_mask |= SCH_NETEM_ATTR_DUPLICATE;
}

/**
 * Get packet duplication probability of netem qdisc.
 * @arg qdisc		Netem qdisc.
 * @return Packet duplication probability or a negative error code.
 */
int rtnl_netem_get_duplicate(struct rtnl_qdisc *qdisc)
{
	struct rtnl_netem *netem;

	if (!(netem = rtnl_tc_data(TC_CAST(qdisc))))
		BUG();

	if (netem->qnm_mask & SCH_NETEM_ATTR_DUPLICATE)
		return netem->qnm_duplicate;
	else
		return -NLE_NOATTR;
}

/**
 * Set packet duplication correlation probability of netem qdisc.
 * @arg qdisc		Netem qdisc to be modified.
 * @arg prob		New packet duplication correlation probability.
 * @return 0 on sucess or a negative error code.
 */
void rtnl_netem_set_duplicate_correlation(struct rtnl_qdisc *qdisc, int prob)
{
	struct rtnl_netem *netem;

	if (!(netem = rtnl_tc_data(TC_CAST(qdisc))))
		BUG();

	netem->qnm_corr.nmc_duplicate = prob;
	netem->qnm_mask |= SCH_NETEM_ATTR_DUP_CORR;
}

/**
 * Get packet duplication correlation probability of netem qdisc.
 * @arg qdisc		Netem qdisc.
 * @return Packet duplication correlation probability or a negative error code.
 */
int rtnl_netem_get_duplicate_correlation(struct rtnl_qdisc *qdisc)
{
	struct rtnl_netem *netem;

	if (!(netem = rtnl_tc_data(TC_CAST(qdisc))))
		BUG();

	if (netem->qnm_mask & SCH_NETEM_ATTR_DUP_CORR)
		return netem->qnm_corr.nmc_duplicate;
	else
		return -NLE_NOATTR;
}

/** @} */

/**
 * @name Packet Delay
 * @{
 */

/**
 * Set packet delay of netem qdisc.
 * @arg qdisc		Netem qdisc to be modified.
 * @arg delay		New packet delay in micro seconds.
 * @return 0 on success or a negative error code.
 */
void rtnl_netem_set_delay(struct rtnl_qdisc *qdisc, int delay)
{
	struct rtnl_netem *netem;

	if (!(netem = rtnl_tc_data(TC_CAST(qdisc))))
		BUG();

	netem->qnm_latency = nl_us2ticks(delay);
	netem->qnm_mask |= SCH_NETEM_ATTR_LATENCY;
}

/**
 * Get packet delay of netem qdisc.
 * @arg qdisc		Netem qdisc.
 * @return Packet delay in micro seconds or a negative error code.
 */
int rtnl_netem_get_delay(struct rtnl_qdisc *qdisc)
{
	struct rtnl_netem *netem;

	if (!(netem = rtnl_tc_data(TC_CAST(qdisc))))
		BUG();

	if (netem->qnm_mask & SCH_NETEM_ATTR_LATENCY)
		return nl_ticks2us(netem->qnm_latency);
	else
		return -NLE_NOATTR;
}

/**
 * Set packet delay jitter of netem qdisc.
 * @arg qdisc		Netem qdisc to be modified.
 * @arg jitter		New packet delay jitter in micro seconds.
 * @return 0 on success or a negative error code.
 */
void rtnl_netem_set_jitter(struct rtnl_qdisc *qdisc, int jitter)
{
	struct rtnl_netem *netem;

	if (!(netem = rtnl_tc_data(TC_CAST(qdisc))))
		BUG();

	netem->qnm_jitter = nl_us2ticks(jitter);
	netem->qnm_mask |= SCH_NETEM_ATTR_JITTER;
}

/**
 * Get packet delay jitter of netem qdisc.
 * @arg qdisc		Netem qdisc.
 * @return Packet delay jitter in micro seconds or a negative error code.
 */
int rtnl_netem_get_jitter(struct rtnl_qdisc *qdisc)
{
	struct rtnl_netem *netem;

	if (!(netem = rtnl_tc_data(TC_CAST(qdisc))))
		BUG();

	if (netem->qnm_mask & SCH_NETEM_ATTR_JITTER)
		return nl_ticks2us(netem->qnm_jitter);
	else
		return -NLE_NOATTR;
}

/**
 * Set packet delay correlation probability of netem qdisc.
 * @arg qdisc		Netem qdisc to be modified.
 * @arg prob		New packet delay correlation probability.
 */
void rtnl_netem_set_delay_correlation(struct rtnl_qdisc *qdisc, int prob)
{
	struct rtnl_netem *netem;

	if (!(netem = rtnl_tc_data(TC_CAST(qdisc))))
		BUG();

	netem->qnm_corr.nmc_delay = prob;
	netem->qnm_mask |= SCH_NETEM_ATTR_DELAY_CORR;
}

/**
 * Get packet delay correlation probability of netem qdisc.
 * @arg qdisc		Netem qdisc.
 * @return Packet delay correlation probability or a negative error code.
 */
int rtnl_netem_get_delay_correlation(struct rtnl_qdisc *qdisc)
{
	struct rtnl_netem *netem;

	if (!(netem = rtnl_tc_data(TC_CAST(qdisc))))
		BUG();

	if (netem->qnm_mask & SCH_NETEM_ATTR_DELAY_CORR)
		return netem->qnm_corr.nmc_delay;
	else
		return -NLE_NOATTR;
}

/**
 * Get the size of the distribution table.
 * @arg qdisc		Netem qdisc.
 * @return Distribution table size or a negative error code.
 */
int rtnl_netem_get_delay_distribution_size(struct rtnl_qdisc *qdisc)
{
	struct rtnl_netem *netem;

	if (!(netem = rtnl_tc_data(TC_CAST(qdisc))))
		BUG();

	if (netem->qnm_mask & SCH_NETEM_ATTR_DIST)
		return netem->qnm_dist.dist_size;
	else
		return -NLE_NOATTR;
}

/**
 * Get a pointer to the distribution table.
 * @arg qdisc		Netem qdisc.
 * @arg dist_ptr	The pointer to set.
 * @return Negative error code on failure or 0 on success.
 */
int rtnl_netem_get_delay_distribution(struct rtnl_qdisc *qdisc, int16_t **dist_ptr)
{
	struct rtnl_netem *netem;

	if (!(netem = rtnl_tc_data(TC_CAST(qdisc))))
		BUG();

	if (netem->qnm_mask & SCH_NETEM_ATTR_DIST) {
		*dist_ptr = netem->qnm_dist.dist_data;
		return 0;
	} else
		return -NLE_NOATTR;
}

/**
 * Set the delay distribution. Latency/jitter must be set before applying.
 * @arg qdisc Netem qdisc.
 * @arg dist_type The name of the distribution (type, file, path/file).
 * @return 0 on success, error code on failure.
 */
int rtnl_netem_set_delay_distribution(struct rtnl_qdisc *qdisc, const char *dist_type) {
	struct rtnl_netem *netem;

	if (!(netem = rtnl_tc_data(TC_CAST(qdisc))))
		BUG();
		
	FILE *f;
	int n = 0;
	size_t i;
	size_t len = 2048;
	char *line;
	char name[NAME_MAX];
	char dist_suffix[] = ".dist";
	
	/* If the given filename already ends in .dist, don't append it later */
	char *test_suffix = strstr(dist_type, dist_suffix);
	if (test_suffix != NULL && strlen(test_suffix) == 5)
		strcpy(dist_suffix, "");
	
	/* Check several locations for the dist file */
	char *test_path[] = { "", "./", "/usr/lib/tc/", "/usr/local/lib/tc/" };
	
	for (i = 0; i < ARRAY_SIZE(test_path); i++) {
		snprintf(name, NAME_MAX, "%s%s%s", test_path[i], dist_type, dist_suffix);
		if ((f = fopen(name, "r")))
			break;
	}
	
	if ( f == NULL )
		return -nl_syserr2nlerr(errno);
	
	netem->qnm_dist.dist_data = (int16_t *) calloc (MAXDIST, sizeof(int16_t));
	
	line = (char *) calloc (sizeof(char), len + 1);
	
	while (getline(&line, &len, f) != -1) {
		char *p, *endp;
		
		if (*line == '\n' || *line == '#')
			continue;

		for (p = line; ; p = endp) {
			long x = strtol(p, &endp, 0);
			if (endp == p) break;

			if (n >= MAXDIST) {
				free(line);
				fclose(f);
				return -NLE_INVAL;
			}
			netem->qnm_dist.dist_data[n++] = x;
		}		
	}
	
	free(line);
	
	netem->qnm_dist.dist_size = n;
	netem->qnm_mask |= SCH_NETEM_ATTR_DIST;
	
	fclose(f);
	return 0;	
}

/** @} */

static struct rtnl_tc_ops netem_ops = {
	.to_kind		= "netem",
	.to_type		= RTNL_TC_TYPE_QDISC,
	.to_size		= sizeof(struct rtnl_netem),
	.to_msg_parser		= netem_msg_parser,
	.to_free_data		= netem_free_data,
	.to_dump[NL_DUMP_LINE]	= netem_dump_line,
	.to_msg_fill_raw	= netem_msg_fill_raw,
};

static void __init netem_init(void)
{
	rtnl_tc_register(&netem_ops);
}

static void __exit netem_exit(void)
{
	rtnl_tc_unregister(&netem_ops);
}

/** @} */
