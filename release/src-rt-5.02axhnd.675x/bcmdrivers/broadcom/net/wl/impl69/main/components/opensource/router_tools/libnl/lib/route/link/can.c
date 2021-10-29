/*
 * lib/route/link/can.c		CAN Link Info
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2012 Benedikt Spranger <b.spranger@linutronix.de>
 */

/**
 * @ingroup link
 * @defgroup can CAN
 * Controller Area Network link module
 *
 * @details
 * \b Link Type Name: "can"
 *
 * @route_doc{link_can, CAN Documentation}
 *
 * @{
 */

#include <netlink-private/netlink.h>
#include <netlink/netlink.h>
#include <netlink/attr.h>
#include <netlink/utils.h>
#include <netlink/object.h>
#include <netlink/route/rtnl.h>
#include <netlink-private/route/link/api.h>
#include <netlink/route/link/can.h>

#include <linux/can/netlink.h>

/** @cond SKIP */
#define CAN_HAS_BITTIMING		(1<<0)
#define CAN_HAS_BITTIMING_CONST		(1<<1)
#define CAN_HAS_CLOCK			(1<<2)
#define CAN_HAS_STATE			(1<<3)
#define CAN_HAS_CTRLMODE		(1<<4)
#define CAN_HAS_RESTART_MS		(1<<5)
#define CAN_HAS_RESTART			(1<<6)
#define CAN_HAS_BERR_COUNTER		(1<<7)

struct can_info {
	uint32_t			ci_state;
	uint32_t			ci_restart;
	uint32_t			ci_restart_ms;
	struct can_ctrlmode		ci_ctrlmode;
	struct can_bittiming		ci_bittiming;
	struct can_bittiming_const	ci_bittiming_const;
	struct can_clock		ci_clock;
	struct can_berr_counter		ci_berr_counter;
	uint32_t			ci_mask;
};

/** @endcond */

static struct nla_policy can_policy[IFLA_CAN_MAX + 1] = {
	[IFLA_CAN_STATE]	= { .type = NLA_U32 },
	[IFLA_CAN_CTRLMODE]	= { .minlen = sizeof(struct can_ctrlmode) },
	[IFLA_CAN_RESTART_MS]	= { .type = NLA_U32 },
	[IFLA_CAN_RESTART]	= { .type = NLA_U32 },
	[IFLA_CAN_BITTIMING]	= { .minlen = sizeof(struct can_bittiming) },
	[IFLA_CAN_BITTIMING_CONST]
				= { .minlen = sizeof(struct can_bittiming_const) },
	[IFLA_CAN_CLOCK]	= { .minlen = sizeof(struct can_clock) },
	[IFLA_CAN_BERR_COUNTER]	= { .minlen = sizeof(struct can_berr_counter) },
};

static int can_alloc(struct rtnl_link *link)
{
	struct can_info *ci;

	ci = calloc(1, sizeof(*ci));
	if (!ci)
		return -NLE_NOMEM;

	link->l_info = ci;

	return 0;
}

static int can_parse(struct rtnl_link *link, struct nlattr *data,
		     struct nlattr *xstats)
{
	struct nlattr *tb[IFLA_CAN_MAX+1];
	struct can_info *ci;
	int err;

	NL_DBG(3, "Parsing CAN link info");

	if ((err = nla_parse_nested(tb, IFLA_CAN_MAX, data, can_policy)) < 0)
		goto errout;

	if ((err = can_alloc(link)) < 0)
		goto errout;

	ci = link->l_info;

	if (tb[IFLA_CAN_STATE]) {
		ci->ci_state = nla_get_u32(tb[IFLA_CAN_STATE]);
		ci->ci_mask |= CAN_HAS_STATE;
	}

	if (tb[IFLA_CAN_RESTART]) {
		ci->ci_restart = nla_get_u32(tb[IFLA_CAN_RESTART]);
		ci->ci_mask |= CAN_HAS_RESTART;
	}

	if (tb[IFLA_CAN_RESTART_MS]) {
		ci->ci_restart_ms = nla_get_u32(tb[IFLA_CAN_RESTART_MS]);
		ci->ci_mask |= CAN_HAS_RESTART_MS;
	}

	if (tb[IFLA_CAN_CTRLMODE]) {
		nla_memcpy(&ci->ci_ctrlmode, tb[IFLA_CAN_CTRLMODE],
			   sizeof(ci->ci_ctrlmode));
		ci->ci_mask |= CAN_HAS_CTRLMODE;
	}

	if (tb[IFLA_CAN_BITTIMING]) {
		nla_memcpy(&ci->ci_bittiming, tb[IFLA_CAN_BITTIMING],
			   sizeof(ci->ci_bittiming));
		ci->ci_mask |= CAN_HAS_BITTIMING;
	}

	if (tb[IFLA_CAN_BITTIMING_CONST]) {
		nla_memcpy(&ci->ci_bittiming_const,
			   tb[IFLA_CAN_BITTIMING_CONST],
			   sizeof(ci->ci_bittiming_const));
		ci->ci_mask |= CAN_HAS_BITTIMING_CONST;
	}

	if (tb[IFLA_CAN_CLOCK]) {
		nla_memcpy(&ci->ci_clock, tb[IFLA_CAN_CLOCK],
			   sizeof(ci->ci_clock));
		ci->ci_mask |= CAN_HAS_CLOCK;
	}

	if (tb[IFLA_CAN_BERR_COUNTER]) {
		nla_memcpy(&ci->ci_berr_counter, tb[IFLA_CAN_BERR_COUNTER],
			   sizeof(ci->ci_berr_counter));
		ci->ci_mask |= CAN_HAS_BERR_COUNTER;
	}

	err = 0;
errout:
	return err;
}

static void can_free(struct rtnl_link *link)
{
	struct can_info *ci = link->l_info;

	free(ci);
	link->l_info = NULL;
}

static char *print_can_state (uint32_t state)
{
	char *text;

	switch (state)
	{
	case CAN_STATE_ERROR_ACTIVE:
		text = "error active";
		break;
	case CAN_STATE_ERROR_WARNING:
		text = "error warning";
		break;
	case CAN_STATE_ERROR_PASSIVE:
		text = "error passive";
		break;
	case CAN_STATE_BUS_OFF:
		text = "bus off";
		break;
	case CAN_STATE_STOPPED:
		text = "stopped";
		break;
	case CAN_STATE_SLEEPING:
		text = "sleeping";
		break;
	default:
		text = "unknown state";
	}

	return text;
}

static void can_dump_line(struct rtnl_link *link, struct nl_dump_params *p)
{
	struct can_info *ci = link->l_info;
	char buf [64];

	rtnl_link_can_ctrlmode2str(ci->ci_ctrlmode.flags, buf, sizeof(buf));
	nl_dump(p, "bitrate %d %s <%s>",
		ci->ci_bittiming.bitrate, print_can_state(ci->ci_state), buf);
}

static void can_dump_details(struct rtnl_link *link, struct nl_dump_params *p)
{
	struct can_info *ci = link->l_info;
	char buf [64];

	rtnl_link_can_ctrlmode2str(ci->ci_ctrlmode.flags, buf, sizeof(buf));
	nl_dump(p, "    bitrate %d %s <%s>",
		ci->ci_bittiming.bitrate, print_can_state(ci->ci_state), buf);

	if (ci->ci_mask & CAN_HAS_RESTART) {
		if (ci->ci_restart)
			nl_dump_line(p,"    restarting\n");
	}

	if (ci->ci_mask & CAN_HAS_RESTART_MS) {
		nl_dump_line(p,"    restart interval %d ms\n",
			     ci->ci_restart_ms);
	}

	if (ci->ci_mask & CAN_HAS_BITTIMING) {
		nl_dump_line(p,"    sample point %f %%\n",
			     ((float) ci->ci_bittiming.sample_point)/10);
		nl_dump_line(p,"    time quanta %d ns\n",
			     ci->ci_bittiming.tq);
		nl_dump_line(p,"    propagation segment %d tq\n",
			     ci->ci_bittiming.prop_seg);
		nl_dump_line(p,"    phase buffer segment1 %d tq\n",
			     ci->ci_bittiming.phase_seg1);
		nl_dump_line(p,"    phase buffer segment2 %d tq\n",
			     ci->ci_bittiming.phase_seg2);
		nl_dump_line(p,"    synchronisation jump width %d tq\n",
			     ci->ci_bittiming.sjw);
		nl_dump_line(p,"    bitrate prescaler %d\n",
			     ci->ci_bittiming.brp);
	}

	if (ci->ci_mask & CAN_HAS_BITTIMING_CONST) {
		nl_dump_line(p,"    minimum tsig1 %d tq\n",
			     ci->ci_bittiming_const.tseg1_min);
		nl_dump_line(p,"    maximum tsig1 %d tq\n",
			     ci->ci_bittiming_const.tseg1_max);
		nl_dump_line(p,"    minimum tsig2 %d tq\n",
			     ci->ci_bittiming_const.tseg2_min);
		nl_dump_line(p,"    maximum tsig2 %d tq\n",
			     ci->ci_bittiming_const.tseg2_max);
		nl_dump_line(p,"    maximum sjw %d tq\n",
			     ci->ci_bittiming_const.sjw_max);
		nl_dump_line(p,"    minimum brp %d\n",
			     ci->ci_bittiming_const.brp_min);
		nl_dump_line(p,"    maximum brp %d\n",
			     ci->ci_bittiming_const.brp_max);
		nl_dump_line(p,"    brp increment %d\n",
			     ci->ci_bittiming_const.brp_inc);
	}

	if (ci->ci_mask & CAN_HAS_CLOCK) {
		nl_dump_line(p,"    base freq %d Hz\n", ci->ci_clock);

	}

	if (ci->ci_mask & CAN_HAS_BERR_COUNTER) {
		nl_dump_line(p,"    bus error RX %d\n",
			     ci->ci_berr_counter.rxerr);
		nl_dump_line(p,"    bus error TX %d\n",
			     ci->ci_berr_counter.txerr);
	}

	return;
}

static int can_clone(struct rtnl_link *dst, struct rtnl_link *src)
{
	struct can_info *cdst, *csrc = src->l_info;
	int ret;

	dst->l_info = NULL;
	ret = rtnl_link_set_type(dst, "can");
	if (ret < 0)
		return ret;

	cdst = malloc(sizeof(*cdst));
	if (!cdst)
		return -NLE_NOMEM;

	*cdst = *csrc;
	dst->l_info = cdst;

	return 0;
}

static int can_put_attrs(struct nl_msg *msg, struct rtnl_link *link)
{
	struct can_info *ci = link->l_info;
	struct nlattr *data;

	data = nla_nest_start(msg, IFLA_INFO_DATA);
	if (!data)
		return -NLE_MSGSIZE;

	if (ci->ci_mask & CAN_HAS_RESTART)
		NLA_PUT_U32(msg, CAN_HAS_RESTART, ci->ci_restart);

	if (ci->ci_mask & CAN_HAS_RESTART_MS)
		NLA_PUT_U32(msg, CAN_HAS_RESTART_MS, ci->ci_restart_ms);

	if (ci->ci_mask & CAN_HAS_CTRLMODE)
		NLA_PUT(msg, CAN_HAS_CTRLMODE, sizeof(ci->ci_ctrlmode),
			&ci->ci_ctrlmode);

	if (ci->ci_mask & CAN_HAS_BITTIMING)
		NLA_PUT(msg, CAN_HAS_BITTIMING, sizeof(ci->ci_bittiming),
			&ci->ci_bittiming);

	if (ci->ci_mask & CAN_HAS_BITTIMING_CONST)
		NLA_PUT(msg, CAN_HAS_BITTIMING_CONST,
			sizeof(ci->ci_bittiming_const),
			&ci->ci_bittiming_const);

	if (ci->ci_mask & CAN_HAS_CLOCK)
		NLA_PUT(msg, CAN_HAS_CLOCK, sizeof(ci->ci_clock),
			&ci->ci_clock);

	nla_nest_end(msg, data);

nla_put_failure:

	return 0;
}

static struct rtnl_link_info_ops can_info_ops = {
	.io_name		= "can",
	.io_alloc		= can_alloc,
	.io_parse		= can_parse,
	.io_dump = {
	    [NL_DUMP_LINE]	= can_dump_line,
	    [NL_DUMP_DETAILS]	= can_dump_details,
	},
	.io_clone		= can_clone,
	.io_put_attrs		= can_put_attrs,
	.io_free		= can_free,
};

/** @cond SKIP */
#define IS_CAN_LINK_ASSERT(link) \
	if ((link)->l_info_ops != &can_info_ops) { \
		APPBUG("Link is not a CAN link. set type \"can\" first."); \
		return -NLE_OPNOTSUPP; \
	}
/** @endcond */

/**
 * @name CAN Object
 * @{
 */

/**
 * Check if link is a CAN link
 * @arg link		Link object
 *
 * @return True if link is a CAN link, otherwise false is returned.
 */
int rtnl_link_is_can(struct rtnl_link *link)
{
	return link->l_info_ops && !strcmp(link->l_info_ops->io_name, "can");
}

/**
 * Restart CAN device
 * @arg link            Link object
 *
 * @return 0 on success or a negative error code
 */
int rtnl_link_can_restart(struct rtnl_link *link)
{
	struct can_info *ci = link->l_info;

	IS_CAN_LINK_ASSERT(link);

	ci->ci_restart = 1;
	ci->ci_restart |= CAN_HAS_RESTART;

	return 0;
}

/**
 * Get CAN base frequency
 * @arg link            Link object
 * @arg freq		frequency in Hz
 *
 * @return 0 on success or a negative error code
 */
int rtnl_link_can_freq(struct rtnl_link *link, uint32_t *freq)
{
	struct can_info *ci = link->l_info;

	IS_CAN_LINK_ASSERT(link);
	if (!freq)
		return -NLE_INVAL;

	if (ci->ci_mask & CAN_HAS_CLOCK)
		*freq = ci->ci_clock.freq;
	else
		return -NLE_AGAIN;

	return 0;
}

/**
 * Get CAN state
 * @arg link            Link object
 * @arg state		CAN bus state
 * @return 0 on success or a negative error code
 */
int rtnl_link_can_state(struct rtnl_link *link, uint32_t *state)
{
	struct can_info *ci = link->l_info;

	IS_CAN_LINK_ASSERT(link);
	if (!state)
		return -NLE_INVAL;

	*state = ci->ci_state;

	return 0;
}

/**
 * Get CAN RX bus error count
 * @arg link            Link object
 *
 * @return RX bus error count on success or a negative error code
 */
int rtnl_link_can_berr_rx(struct rtnl_link *link)
{
	struct can_info *ci = link->l_info;

	IS_CAN_LINK_ASSERT(link);

	if (ci->ci_mask & CAN_HAS_BERR_COUNTER)
		return ci->ci_berr_counter.rxerr;
	else
		return -NLE_AGAIN;
}

/**
 * Get CAN TX bus error count
 * @arg link            Link object
 *
 * @return TX bus error count on success or a negative error code
 */
int rtnl_link_can_berr_tx(struct rtnl_link *link)
{
	struct can_info *ci = link->l_info;

	IS_CAN_LINK_ASSERT(link);

	if (ci->ci_mask & CAN_HAS_BERR_COUNTER)
		return ci->ci_berr_counter.txerr;
	else
		return -NLE_AGAIN;
}

/**
 * Get CAN bus error count
 * @arg link            Link object
 * @arg berr		Bus error count
 *
 * @return 0 on success or a negative error code
 */
int rtnl_link_can_berr(struct rtnl_link *link, struct can_berr_counter *berr)
{
	struct can_info *ci = link->l_info;

	IS_CAN_LINK_ASSERT(link);
	if (!berr)
		return -NLE_INVAL;

	if (ci->ci_mask & CAN_HAS_BERR_COUNTER)
		*berr = ci->ci_berr_counter;
	else
		return -NLE_AGAIN;

	return 0;
}

/**
 * Get CAN harware-dependent bit-timing constant
 * @arg link            Link object
 * @arg bt_const	Bit-timing constant
 *
 * @return 0 on success or a negative error code
 */
int rtnl_link_can_get_bt_const(struct rtnl_link *link,
			       struct can_bittiming_const *bt_const)
{
	struct can_info *ci = link->l_info;

	IS_CAN_LINK_ASSERT(link);
	if (!bt_const)
		return -NLE_INVAL;

	if (ci->ci_mask & CAN_HAS_BITTIMING_CONST)
		*bt_const = ci->ci_bittiming_const;
	else
		return -NLE_AGAIN;

	return 0;
}

/**
 * Get CAN device bit-timing
 * @arg link            Link object
 * @arg bit_timing	CAN bit-timing
 *
 * @return 0 on success or a negative error code
 */
int rtnl_link_can_get_bittiming(struct rtnl_link *link,
				struct can_bittiming *bit_timing)
{
	struct can_info *ci = link->l_info;

	IS_CAN_LINK_ASSERT(link);
	if (!bit_timing)
		return -NLE_INVAL;

	if (ci->ci_mask & CAN_HAS_BITTIMING)
		*bit_timing = ci->ci_bittiming;
	else
		return -NLE_AGAIN;

	return 0;
}

/**
 * Set CAN device bit-timing
 * @arg link            Link object
 * @arg bit_timing	CAN bit-timing
 *
 * @return 0 on success or a negative error code
 */
int rtnl_link_can_set_bittiming(struct rtnl_link *link,
				struct can_bittiming *bit_timing)
{
	struct can_info *ci = link->l_info;

	IS_CAN_LINK_ASSERT(link);
	if (!bit_timing)
		return -NLE_INVAL;

	ci->ci_bittiming = *bit_timing;
	ci->ci_mask |= CAN_HAS_BITTIMING;

	return 0;
}

/**
 * Get CAN device bit-timing
 * @arg link            Link object
 * @arg bitrate		CAN bitrate
 *
 * @return 0 on success or a negative error code
 */
int rtnl_link_can_get_bitrate(struct rtnl_link *link, uint32_t *bitrate)
{
	struct can_info *ci = link->l_info;

	IS_CAN_LINK_ASSERT(link);
	if (!bitrate)
		return -NLE_INVAL;

	if (ci->ci_mask & CAN_HAS_BITTIMING)
		*bitrate = ci->ci_bittiming.bitrate;
	else
		return -NLE_AGAIN;

	return 0;
}

/**
 * Set CAN device bit-rate
 * @arg link            Link object
 * @arg bitrate		CAN bitrate
 *
 * @return 0 on success or a negative error code
 */
int rtnl_link_can_set_bitrate(struct rtnl_link *link, uint32_t bitrate)
{
	struct can_info *ci = link->l_info;

	IS_CAN_LINK_ASSERT(link);

	ci->ci_bittiming.bitrate = bitrate;
	ci->ci_mask |= CAN_HAS_BITTIMING;

	return 0;
}

/**
 * Get CAN device sample point
 * @arg link            Link object
 * @arg sp		CAN sample point
 *
 * @return 0 on success or a negative error code
 */
int rtnl_link_can_get_sample_point(struct rtnl_link *link, uint32_t *sp)
{
	struct can_info *ci = link->l_info;

	IS_CAN_LINK_ASSERT(link);
	if (!sp)
		return -NLE_INVAL;

	if (ci->ci_mask & CAN_HAS_BITTIMING)
		*sp = ci->ci_bittiming.sample_point;
	else
		return -NLE_AGAIN;

	return 0;
}

/**
 * Set CAN device sample point
 * @arg link            Link object
 * @arg sp		CAN sample point
 *
 * @return 0 on success or a negative error code
 */
int rtnl_link_can_set_sample_point(struct rtnl_link *link, uint32_t sp)
{
	struct can_info *ci = link->l_info;

	IS_CAN_LINK_ASSERT(link);

	ci->ci_bittiming.sample_point = sp;
	ci->ci_mask |= CAN_HAS_BITTIMING;

	return 0;
}

/**
 * Get CAN device restart intervall
 * @arg link            Link object
 * @arg interval	Restart intervall in ms
 *
 * @return 0 on success or a negative error code
 */
int rtnl_link_can_get_restart_ms(struct rtnl_link *link, uint32_t *interval)
{
	struct can_info *ci = link->l_info;

	IS_CAN_LINK_ASSERT(link);
	if (!interval)
		return -NLE_INVAL;

	if (ci->ci_mask & CAN_HAS_RESTART_MS)
		*interval = ci->ci_restart_ms;
	else
		return -NLE_AGAIN;

	return 0;
}

/**
 * Set CAN device restart intervall
 * @arg link            Link object
 * @arg interval	Restart intervall in ms
 *
 * @return 0 on success or a negative error code
 */
int rtnl_link_can_set_restart_ms(struct rtnl_link *link, uint32_t interval)
{
	struct can_info *ci = link->l_info;

	IS_CAN_LINK_ASSERT(link);

	ci->ci_restart_ms = interval;
	ci->ci_mask |= CAN_HAS_RESTART_MS;

	return 0;
}

/**
 * Get CAN control mode
 * @arg link            Link object
 * @arg ctrlmode	CAN control mode
 *
 * @return 0 on success or a negative error code
 */
int rtnl_link_can_get_ctrlmode(struct rtnl_link *link, uint32_t *ctrlmode)
{
	struct can_info *ci = link->l_info;

	IS_CAN_LINK_ASSERT(link);
	if (!ctrlmode)
		return -NLE_INVAL;

	if (ci->ci_mask & CAN_HAS_CTRLMODE)
		*ctrlmode = ci->ci_ctrlmode.flags;
	else
		return -NLE_AGAIN;

	return 0;
}

/**
 * Set a CAN Control Mode
 * @arg link            Link object
 * @arg ctrlmode	CAN control mode
 *
 * @return 0 on success or a negative error code
 */
int rtnl_link_can_set_ctrlmode(struct rtnl_link *link, uint32_t ctrlmode)
{
	struct can_info *ci = link->l_info;

	IS_CAN_LINK_ASSERT(link);

	ci->ci_ctrlmode.flags |= ctrlmode;
	ci->ci_ctrlmode.mask |= ctrlmode;
	ci->ci_mask |= CAN_HAS_CTRLMODE;

	return 0;
}

/**
 * Unset a CAN Control Mode
 * @arg link            Link object
 * @arg ctrlmode	CAN control mode
 *
 * @return 0 on success or a negative error code
 */
int rtnl_link_can_unset_ctrlmode(struct rtnl_link *link, uint32_t ctrlmode)
{
	struct can_info *ci = link->l_info;

	IS_CAN_LINK_ASSERT(link);

	ci->ci_ctrlmode.flags &= ~ctrlmode;
	ci->ci_ctrlmode.mask |= ctrlmode;
	ci->ci_mask |= CAN_HAS_CTRLMODE;

	return 0;
}

/** @} */

/**
 * @name Control Mode Translation
 * @{
 */

static const struct trans_tbl can_ctrlmode[] = {
	__ADD(CAN_CTRLMODE_LOOPBACK, loopback)
	__ADD(CAN_CTRLMODE_LISTENONLY, listen-only)
	__ADD(CAN_CTRLMODE_3_SAMPLES, triple-sampling)
	__ADD(CAN_CTRLMODE_ONE_SHOT, one-shot)
	__ADD(CAN_CTRLMODE_BERR_REPORTING, berr-reporting)
};

char *rtnl_link_can_ctrlmode2str(int ctrlmode, char *buf, size_t len)
{
	return __flags2str(ctrlmode, buf, len, can_ctrlmode,
			   ARRAY_SIZE(can_ctrlmode));
}

int rtnl_link_can_str2ctrlmode(const char *name)
{
	return __str2flags(name, can_ctrlmode, ARRAY_SIZE(can_ctrlmode));
}

/** @} */

static void __init can_init(void)
{
	rtnl_link_register_info(&can_info_ops);
}

static void __exit can_exit(void)
{
	rtnl_link_unregister_info(&can_info_ops);
}

/** @} */
