/*
 * lib/route/qdisc/plug.c		PLUG Qdisc
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2012 Shriram Rajagopalan <rshriram@cs.ubc.ca>
 */

/**
 * @ingroup qdisc
 * @defgroup qdisc_plug Plug/Unplug Traffic (PLUG)
 * @brief
 *
 * Queue traffic until an explicit release command.
 *
 * There are two ways to use this qdisc:
 * 1. A simple "instantaneous" plug/unplug operation, by issuing an alternating
 *    sequence of TCQ_PLUG_BUFFER & TCQ_PLUG_RELEASE_INDEFINITE commands.
 *
 * 2. For network output buffering (a.k.a output commit) functionality.
 *    Output commit property is commonly used by applications using checkpoint
 *    based fault-tolerance to ensure that the checkpoint from which a system
 *    is being restored is consistent w.r.t outside world.
 *
 *    Consider for e.g. Remus - a Virtual Machine checkpointing system,
 *    wherein a VM is checkpointed, say every 50ms. The checkpoint is replicated
 *    asynchronously to the backup host, while the VM continues executing the
 *    next epoch speculatively.
 *
 *    The following is a typical sequence of output buffer operations:
 *       1.At epoch i, start_buffer(i)
 *       2. At end of epoch i (i.e. after 50ms):
 *          2.1 Stop VM and take checkpoint(i).
 *          2.2 start_buffer(i+1) and Resume VM
 *       3. While speculatively executing epoch(i+1), asynchronously replicate
 *          checkpoint(i) to backup host.
 *       4. When checkpoint_ack(i) is received from backup, release_buffer(i)
 *    Thus, this Qdisc would receive the following sequence of commands:
 *       TCQ_PLUG_BUFFER (epoch i)
 *       .. TCQ_PLUG_BUFFER (epoch i+1)
 *       ....TCQ_PLUG_RELEASE_ONE (epoch i)
 *       ......TCQ_PLUG_BUFFER (epoch i+2)
 *       ........
 *
 *
 * State of the queue, when used for network output buffering:
 *
 *                 plug(i+1)            plug(i)          head
 * ------------------+--------------------+---------------->
 *                   |                    |
 *                   |                    |
 * pkts_current_epoch| pkts_last_epoch    |pkts_to_release
 * ----------------->|<--------+--------->|+--------------->
 *                   v                    v
 *
 *
 * @{
 */

#include <netlink-local.h>
#include <netlink-tc.h>
#include <netlink/netlink.h>
#include <netlink/utils.h>
#include <netlink/route/tc-api.h>
#include <netlink/route/qdisc/plug.h>

static int plug_msg_fill(struct rtnl_tc *tc, void *data, struct nl_msg *msg)
{
	struct rtnl_plug *plug = data;
	struct tc_plug_qopt opts;

	if (!plug)
		return -NLE_INVAL;

	opts.action = plug->action;
	opts.limit  = plug->limit;

	return nlmsg_append(msg, &opts, sizeof(opts), NL_DONTPAD);
}

/**
 * @name Attribute Modification
 * @{
 */

/**
 * Insert a plug into the qdisc and buffer any incoming
 * network traffic.
 * @arg qdisc		PLUG qdisc to be modified.
 */
int rtnl_qdisc_plug_buffer(struct rtnl_qdisc *qdisc)
{
	struct rtnl_plug *plug;

	if (!(plug = rtnl_tc_data(TC_CAST(qdisc))))
		return -NLE_NOMEM;

	plug->action = TCQ_PLUG_BUFFER;
	return 0;
}

/**
 * Unplug the qdisc, releasing packets from queue head
 * to the last complete buffer, while new traffic
 * continues to be buffered.
 * @arg qdisc		PLUG qdisc to be modified.
 */
int rtnl_qdisc_plug_release_one(struct rtnl_qdisc *qdisc)
{
	struct rtnl_plug *plug;

	if (!(plug = rtnl_tc_data(TC_CAST(qdisc))))
		return -NLE_NOMEM;

	plug->action = TCQ_PLUG_RELEASE_ONE;
	return 0;
}

/**
 * Indefinitely unplug the qdisc, releasing all packets.
 * Network traffic will not be buffered until the next
 * buffer command is issued.
 * @arg qdisc		PLUG qdisc to be modified.
 */
int rtnl_qdisc_plug_release_indefinite(struct rtnl_qdisc *qdisc)
{
	struct rtnl_plug *plug;

	if (!(plug = rtnl_tc_data(TC_CAST(qdisc))))
		return -NLE_NOMEM;

	plug->action = TCQ_PLUG_RELEASE_INDEFINITE;
	return 0;
}

/**
 * Set limit of PLUG qdisc.
 * @arg qdisc		PLUG qdisc to be modified.
 * @arg limit		New limit.
 * @return 0 on success or a negative error code.
 */
int rtnl_qdisc_plug_set_limit(struct rtnl_qdisc *qdisc, int limit)
{
	struct rtnl_plug *plug;
	
	if (!(plug = rtnl_tc_data(TC_CAST(qdisc))))
		return -NLE_NOMEM;
		
	plug->action = TCQ_PLUG_LIMIT;
	plug->limit  = limit;

	return 0;
}

/** @} */

static struct rtnl_tc_ops plug_ops = {
	.to_kind		= "plug",
	.to_type		= RTNL_TC_TYPE_QDISC,
	.to_size		= sizeof(struct rtnl_plug),
	.to_msg_fill		= plug_msg_fill,
};

static void __init plug_init(void)
{
	rtnl_tc_register(&plug_ops);
}

static void __exit plug_exit(void)
{
	rtnl_tc_unregister(&plug_ops);
}

/** @} */
