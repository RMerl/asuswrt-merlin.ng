/*
 * lib/route/qdisc.c            Queueing Disciplines
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2003-2011 Thomas Graf <tgraf@suug.ch>
 */

/**
 * @ingroup tc
 * @defgroup qdisc Queueing Disciplines
 * @{
 */

#include <netlink-local.h>
#include <netlink-tc.h>
#include <netlink/netlink.h>
#include <netlink/utils.h>
#include <netlink/route/link.h>
#include <netlink/route/tc-api.h>
#include <netlink/route/qdisc.h>
#include <netlink/route/class.h>
#include <netlink/route/classifier.h>

static struct nl_cache_ops rtnl_qdisc_ops;
static struct nl_object_ops qdisc_obj_ops;

static int qdisc_msg_parser(struct nl_cache_ops *ops, struct sockaddr_nl *who,
			    struct nlmsghdr *n, struct nl_parser_param *pp)
{
	struct rtnl_qdisc *qdisc;
	int err;

	if (!(qdisc = rtnl_qdisc_alloc()))
		return -NLE_NOMEM;

	if ((err = rtnl_tc_msg_parse(n, TC_CAST(qdisc))) < 0)
		goto errout;

	err = pp->pp_cb(OBJ_CAST(qdisc), pp);
errout:
	rtnl_qdisc_put(qdisc);

	return err;
}

static int qdisc_request_update(struct nl_cache *c, struct nl_sock *sk)
{
	struct tcmsg tchdr = {
		.tcm_family = AF_UNSPEC,
		.tcm_ifindex = c->c_iarg1,
	};

	return nl_send_simple(sk, RTM_GETQDISC, NLM_F_DUMP, &tchdr,
			      sizeof(tchdr));
}

/**
 * @name Allocation/Freeing
 * @{
 */

struct rtnl_qdisc *rtnl_qdisc_alloc(void)
{
	struct rtnl_tc *tc;

	tc = TC_CAST(nl_object_alloc(&qdisc_obj_ops));
	if (tc)
		tc->tc_type = RTNL_TC_TYPE_QDISC;

	return (struct rtnl_qdisc *) tc;
}

void rtnl_qdisc_put(struct rtnl_qdisc *qdisc)
{
	nl_object_put((struct nl_object *) qdisc);
}

/** @} */

/**
 * @name Addition / Modification / Deletion
 * @{
 */

static int build_qdisc_msg(struct rtnl_qdisc *qdisc, int type, int flags,
			   struct nl_msg **result)
{
	if (!(qdisc->ce_mask & TCA_ATTR_IFINDEX)) {
		APPBUG("ifindex must be specified");
		return -NLE_MISSING_ATTR;
	}

	return rtnl_tc_msg_build(TC_CAST(qdisc), type, flags, result);
}

/**
 * Build a netlink message requesting the addition of a qdisc
 * @arg qdisc		Qdisc to add 
 * @arg flags		Additional netlink message flags
 * @arg result		Pointer to store resulting netlink message
 *
 * The behaviour of this function is identical to rtnl_qdisc_add() with
 * the exception that it will not send the message but return it int the
 * provided return pointer instead.
 *
 * @see rtnl_qdisc_add()
 *
 * @return 0 on success or a negative error code.
 */
int rtnl_qdisc_build_add_request(struct rtnl_qdisc *qdisc, int flags,
				 struct nl_msg **result)
{
	if (!(qdisc->ce_mask & (TCA_ATTR_HANDLE | TCA_ATTR_PARENT))) {
		APPBUG("handle or parent must be specified");
		return -NLE_MISSING_ATTR;
	}

	return build_qdisc_msg(qdisc, RTM_NEWQDISC, flags, result);
}

/**
 * Add qdisc
 * @arg sk		Netlink socket
 * @arg qdisc		Qdisc to add 
 * @arg flags		Additional netlink message flags
 *
 * Builds a \c RTM_NEWQDISC netlink message requesting the addition
 * of a new qdisc and sends the message to the kernel. The configuration
 * of the qdisc is derived from the attributes of the specified qdisc.
 *
 * The following flags may be specified:
 *  - \c NLM_F_CREATE:  Create qdisc if it does not exist, otherwise 
 *                      -NLE_OBJ_NOTFOUND is returned.
 *  - \c NLM_F_REPLACE: If another qdisc is already attached to the
 *                      parent, replace it even if the handles mismatch.
 *  - \c NLM_F_EXCL:    Return -NLE_EXISTS if a qdisc with matching
 *                      handle exists already.
 *
 * Existing qdiscs with matching handles will be updated, unless the
 * flag \c NLM_F_EXCL is specified. If their handles do not match, the
 * error -NLE_EXISTS is returned unless the flag \c NLM_F_REPLACE is
 * specified in which case the existing qdisc is replaced with the new
 * one.  If no matching qdisc exists, it will be created if the flag
 * \c NLM_F_CREATE is set, otherwise the error -NLE_OBJ_NOTFOUND is
 * returned. 
 *
 * After sending, the function will wait for the ACK or an eventual
 * error message to be received and will therefore block until the
 * operation has been completed.
 *
 * @note Disabling auto-ack (nl_socket_disable_auto_ack()) will cause
 *       this function to return immediately after sending. In this case,
 *       it is the responsibility of the caller to handle any error
 *       messages returned.
 *
 * @return 0 on success or a negative error code.
 */
int rtnl_qdisc_add(struct nl_sock *sk, struct rtnl_qdisc *qdisc, int flags)
{
	struct nl_msg *msg;
	int err;

	if ((err = rtnl_qdisc_build_add_request(qdisc, flags, &msg)) < 0)
		return err;

	return nl_send_sync(sk, msg);
}

/**
 * Build netlink message requesting the update of a qdisc
 * @arg qdisc		Qdisc to update
 * @arg new		Qdisc with updated attributes
 * @arg flags		Additional netlink message flags
 * @arg result		Pointer to store resulting netlink message
 *
 * The behaviour of this function is identical to rtnl_qdisc_update() with
 * the exception that it will not send the message but return it in the
 * provided return pointer instead.
 *
 * @see rtnl_qdisc_update()
 *
 * @return 0 on success or a negative error code.
 */
int rtnl_qdisc_build_update_request(struct rtnl_qdisc *qdisc,
				    struct rtnl_qdisc *new, int flags,
				    struct nl_msg **result)
{
	if (flags & (NLM_F_CREATE | NLM_F_EXCL)) {
		APPBUG("NLM_F_CREATE and NLM_F_EXCL may not be used here, "
		       "use rtnl_qdisc_add()");
		return -NLE_INVAL;
	}

	if (!(qdisc->ce_mask & TCA_ATTR_IFINDEX)) {
		APPBUG("ifindex must be specified");
		return -NLE_MISSING_ATTR;
	}

	if (!(qdisc->ce_mask & (TCA_ATTR_HANDLE | TCA_ATTR_PARENT))) {
		APPBUG("handle or parent must be specified");
		return -NLE_MISSING_ATTR;
	}

	rtnl_tc_set_ifindex(TC_CAST(new), qdisc->q_ifindex);

	if (qdisc->ce_mask & TCA_ATTR_HANDLE)
		rtnl_tc_set_handle(TC_CAST(new), qdisc->q_handle);

	if (qdisc->ce_mask & TCA_ATTR_PARENT)
		rtnl_tc_set_parent(TC_CAST(new), qdisc->q_parent);

	return build_qdisc_msg(new, RTM_NEWQDISC, flags, result);
}

/**
 * Update qdisc
 * @arg sk		Netlink socket
 * @arg qdisc		Qdisc to update
 * @arg new		Qdisc with updated attributes
 * @arg flags		Additional netlink message flags
 *
 * Builds a \c RTM_NEWQDISC netlink message requesting the update
 * of an existing qdisc and sends the message to the kernel.
 *
 * This function is a varation of rtnl_qdisc_add() to update qdiscs
 * if the qdisc to be updated is available as qdisc object. The
 * behaviour is identical to the one of rtnl_qdisc_add except that
 * before constructing the message, it copies the \c ifindex,
 * \c handle, and \c parent from the original \p qdisc to the \p new
 * qdisc.
 *
 * After sending, the function will wait for the ACK or an eventual
 * error message to be received and will therefore block until the
 * operation has been completed.
 *
 * @note Disabling auto-ack (nl_socket_disable_auto_ack()) will cause
 *       this function to return immediately after sending. In this case,
 *       it is the responsibility of the caller to handle any error
 *       messages returned.
 *
 * @return 0 on success or a negative error code.
 */
int rtnl_qdisc_update(struct nl_sock *sk, struct rtnl_qdisc *qdisc,
		      struct rtnl_qdisc *new, int flags)
{
	struct nl_msg *msg;
	int err;

	err = rtnl_qdisc_build_update_request(qdisc, new, flags, &msg);
	if (err < 0)
		return err;

	return nl_send_sync(sk, msg);
}

/**
 * Build netlink message requesting the deletion of a qdisc
 * @arg qdisc		Qdisc to delete
 * @arg result		Pointer to store resulting netlink message
 *
 * The behaviour of this function is identical to rtnl_qdisc_delete() with
 * the exception that it will not send the message but return it in the
 * provided return pointer instead.
 *
 * @see rtnl_qdisc_delete()
 *
 * @return 0 on success or a negative error code.
 */
int rtnl_qdisc_build_delete_request(struct rtnl_qdisc *qdisc,
				    struct nl_msg **result)
{
	struct nl_msg *msg;
	struct tcmsg tchdr;
	int required = TCA_ATTR_IFINDEX | TCA_ATTR_PARENT;

	if ((qdisc->ce_mask & required) != required) {
		APPBUG("ifindex and parent must be specified");
		return -NLE_MISSING_ATTR;
	}

	if (!(msg = nlmsg_alloc_simple(RTM_DELQDISC, 0)))
		return -NLE_NOMEM;

	memset(&tchdr, 0, sizeof(tchdr));

	tchdr.tcm_family = AF_UNSPEC;
	tchdr.tcm_ifindex = qdisc->q_ifindex;
	tchdr.tcm_parent = qdisc->q_parent;

	if (qdisc->ce_mask & TCA_ATTR_HANDLE)
		tchdr.tcm_handle = qdisc->q_handle;

	if (nlmsg_append(msg, &tchdr, sizeof(tchdr), NLMSG_ALIGNTO) < 0)
		goto nla_put_failure;

	if (qdisc->ce_mask & TCA_ATTR_KIND)
		NLA_PUT_STRING(msg, TCA_KIND, qdisc->q_kind);

	*result = msg;
	return 0;

nla_put_failure:
	nlmsg_free(msg);
	return -NLE_MSGSIZE;
}

/**
 * Delete qdisc
 * @arg sk		Netlink socket
 * @arg qdisc		Qdisc to add 
 *
 * Builds a \c RTM_NEWQDISC netlink message requesting the deletion
 * of a qdisc and sends the message to the kernel.
 *
 * The message is constructed out of the following attributes:
 * - \c ifindex and \c parent
 * - \c handle (optional, must match if provided)
 * - \c kind (optional, must match if provided)
 *
 * All other qdisc attributes including all qdisc type specific
 * attributes are ignored.
 *
 * After sending, the function will wait for the ACK or an eventual
 * error message to be received and will therefore block until the
 * operation has been completed.
 *
 * @note It is not possible to delete default qdiscs.
 *
 * @note Disabling auto-ack (nl_socket_disable_auto_ack()) will cause
 *       this function to return immediately after sending. In this case,
 *       it is the responsibility of the caller to handle any error
 *       messages returned.
 *
 * @return 0 on success or a negative error code.
 */
int rtnl_qdisc_delete(struct nl_sock *sk, struct rtnl_qdisc *qdisc)
{
	struct nl_msg *msg;
	int err;

	if ((err = rtnl_qdisc_build_delete_request(qdisc, &msg)) < 0)
		return err;

	return nl_send_sync(sk, msg);
}

/** @} */

/**
 * @name Cache Related Functions
 * @{
 */

/**
 * Allocate a cache and fill it with all configured qdiscs
 * @arg sk		Netlink socket
 * @arg result		Pointer to store the created cache
 *
 * Allocates a new qdisc cache and fills it with a list of all configured
 * qdiscs on all network devices. Release the cache with nl_cache_free().
 *
 * @return 0 on success or a negative error code.
 */
int rtnl_qdisc_alloc_cache(struct nl_sock *sk, struct nl_cache **result)
{
	return nl_cache_alloc_and_fill(&rtnl_qdisc_ops, sk, result);
}

/**
 * Search qdisc by interface index and parent
 * @arg cache		Qdisc cache
 * @arg ifindex		Interface index
 * @arg parent		Handle of parent qdisc
 *
 * Searches a qdisc cache previously allocated with rtnl_qdisc_alloc_cache()
 * and searches for a qdisc matching the interface index and parent qdisc.
 *
 * The reference counter is incremented before returning the qdisc, therefore
 * the reference must be given back with rtnl_qdisc_put() after usage.
 *
 * @return pointer to qdisc inside the cache or NULL if no match was found.
 */
struct rtnl_qdisc *rtnl_qdisc_get_by_parent(struct nl_cache *cache,
					    int ifindex, uint32_t parent)
{
	struct rtnl_qdisc *q;

	if (cache->c_ops != &rtnl_qdisc_ops)
		return NULL;

	nl_list_for_each_entry(q, &cache->c_items, ce_list) {
		if (q->q_parent == parent && q->q_ifindex == ifindex) {
			nl_object_get((struct nl_object *) q);
			return q;
		}
	}

	return NULL;
}

/**
 * Search qdisc by interface index and handle
 * @arg cache		Qdisc cache
 * @arg ifindex		Interface index
 * @arg handle		Handle
 *
 * Searches a qdisc cache previously allocated with rtnl_qdisc_alloc_cache()
 * and searches for a qdisc matching the interface index and handle.
 *
 * The reference counter is incremented before returning the qdisc, therefore
 * the reference must be given back with rtnl_qdisc_put() after usage.
 *
 * @return Qdisc or NULL if no match was found.
 */
struct rtnl_qdisc *rtnl_qdisc_get(struct nl_cache *cache, int ifindex,
				  uint32_t handle)
{
	struct rtnl_qdisc *q;

	if (cache->c_ops != &rtnl_qdisc_ops)
		return NULL;

	nl_list_for_each_entry(q, &cache->c_items, ce_list) {
		if (q->q_handle == handle && q->q_ifindex == ifindex) {
			nl_object_get((struct nl_object *) q);
			return q;
		}
	}

	return NULL;
}

/** @} */

/**
 * @name Deprecated Functions
 * @{
 */

/**
 * Call a callback for each child class of a qdisc (deprecated)
 *
 * @deprecated Use of this function is deprecated, it does not allow
 *             to handle the out of memory situation that can occur.
 */
void rtnl_qdisc_foreach_child(struct rtnl_qdisc *qdisc, struct nl_cache *cache,
			      void (*cb)(struct nl_object *, void *), void *arg)
{
	struct rtnl_class *filter;
	
	filter = rtnl_class_alloc();
	if (!filter)
		return;

	rtnl_tc_set_parent(TC_CAST(filter), qdisc->q_handle);
	rtnl_tc_set_ifindex(TC_CAST(filter), qdisc->q_ifindex);
	rtnl_tc_set_kind(TC_CAST(filter), qdisc->q_kind);

	nl_cache_foreach_filter(cache, OBJ_CAST(filter), cb, arg);

	rtnl_class_put(filter);
}

/**
 * Call a callback for each filter attached to the qdisc (deprecated)
 *
 * @deprecated Use of this function is deprecated, it does not allow
 *             to handle the out of memory situation that can occur.
 */
void rtnl_qdisc_foreach_cls(struct rtnl_qdisc *qdisc, struct nl_cache *cache,
			    void (*cb)(struct nl_object *, void *), void *arg)
{
	struct rtnl_cls *filter;

	if (!(filter = rtnl_cls_alloc()))
		return;

	rtnl_tc_set_ifindex(TC_CAST(filter), qdisc->q_ifindex);
	rtnl_tc_set_parent(TC_CAST(filter), qdisc->q_parent);

	nl_cache_foreach_filter(cache, OBJ_CAST(filter), cb, arg);
	rtnl_cls_put(filter);
}

/**
 * Build a netlink message requesting the update of a qdisc
 *
 * @deprecated Use of this function is deprecated in favour of
 *             rtnl_qdisc_build_update_request() due to the missing
 *             possibility of specifying additional flags.
 */
int rtnl_qdisc_build_change_request(struct rtnl_qdisc *qdisc,
				    struct rtnl_qdisc *new,
				    struct nl_msg **result)
{
	return rtnl_qdisc_build_update_request(qdisc, new, NLM_F_REPLACE,
					       result);
}

/**
 * Change attributes of a qdisc
 *
 * @deprecated Use of this function is deprecated in favour of
 *             rtnl_qdisc_update() due to the missing possibility of
 *             specifying additional flags.
 */
int rtnl_qdisc_change(struct nl_sock *sk, struct rtnl_qdisc *qdisc,
		      struct rtnl_qdisc *new)
{
	return rtnl_qdisc_update(sk, qdisc, new, NLM_F_REPLACE);
}

/** @} */

static void qdisc_dump_details(struct rtnl_tc *tc, struct nl_dump_params *p)
{
	struct rtnl_qdisc *qdisc = (struct rtnl_qdisc *) tc;

	nl_dump(p, "refcnt %u ", qdisc->q_info);
}

static struct rtnl_tc_type_ops qdisc_ops = {
	.tt_type		= RTNL_TC_TYPE_QDISC,
	.tt_dump_prefix		= "qdisc",
	.tt_dump = {
	    [NL_DUMP_DETAILS]	= qdisc_dump_details,
	},
};

static struct nl_cache_ops rtnl_qdisc_ops = {
	.co_name		= "route/qdisc",
	.co_hdrsize		= sizeof(struct tcmsg),
	.co_msgtypes		= {
					{ RTM_NEWQDISC, NL_ACT_NEW, "new" },
					{ RTM_DELQDISC, NL_ACT_DEL, "del" },
					{ RTM_GETQDISC, NL_ACT_GET, "get" },
					END_OF_MSGTYPES_LIST,
				  },
	.co_protocol		= NETLINK_ROUTE,
	.co_request_update	= qdisc_request_update,
	.co_msg_parser		= qdisc_msg_parser,
	.co_obj_ops		= &qdisc_obj_ops,
};

static struct nl_object_ops qdisc_obj_ops = {
	.oo_name		= "route/qdisc",
	.oo_size		= sizeof(struct rtnl_qdisc),
	.oo_free_data		= rtnl_tc_free_data,
	.oo_clone		= rtnl_tc_clone,
	.oo_dump = {
	    [NL_DUMP_LINE]	= rtnl_tc_dump_line,
	    [NL_DUMP_DETAILS]	= rtnl_tc_dump_details,
	    [NL_DUMP_STATS]	= rtnl_tc_dump_stats,
	},
	.oo_compare		= rtnl_tc_compare,
	.oo_id_attrs		= (TCA_ATTR_IFINDEX | TCA_ATTR_HANDLE),
};

static void __init qdisc_init(void)
{
	rtnl_tc_type_register(&qdisc_ops);
	nl_cache_mngt_register(&rtnl_qdisc_ops);
}

static void __exit qdisc_exit(void)
{
	nl_cache_mngt_unregister(&rtnl_qdisc_ops);
	rtnl_tc_type_unregister(&qdisc_ops);
}

/** @} */
