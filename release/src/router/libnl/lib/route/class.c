/*
 * lib/route/class.c            Traffic Classes
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2003-2013 Thomas Graf <tgraf@suug.ch>
 */

/**
 * @ingroup tc
 * @defgroup class Traffic Classes
 * @{
 */

#include <netlink-private/netlink.h>
#include <netlink-private/tc.h>
#include <netlink/netlink.h>
#include <netlink-private/route/tc-api.h>
#include <netlink/route/class.h>
#include <netlink/route/qdisc.h>
#include <netlink/route/classifier.h>
#include <netlink/utils.h>

static struct nl_cache_ops rtnl_class_ops;
static struct nl_object_ops class_obj_ops;

static void class_dump_details(struct rtnl_tc *tc, struct nl_dump_params *p)
{
	struct rtnl_class *class = (struct rtnl_class *) tc;
	char buf[32];

	if (class->c_info)
		nl_dump(p, "child-qdisc %s ",
			rtnl_tc_handle2str(class->c_info, buf, sizeof(buf)));
}


static int class_msg_parser(struct nl_cache_ops *ops, struct sockaddr_nl *who,
			    struct nlmsghdr *nlh, struct nl_parser_param *pp)
{
	struct rtnl_class *class;
	int err;

	if (!(class = rtnl_class_alloc()))
		return -NLE_NOMEM;

	if ((err = rtnl_tc_msg_parse(nlh, TC_CAST(class))) < 0)
		goto errout;

	err = pp->pp_cb(OBJ_CAST(class), pp);
errout:
	rtnl_class_put(class);

	return err;
}

static int class_request_update(struct nl_cache *cache, struct nl_sock *sk)
{
	struct tcmsg tchdr = {
		.tcm_family = AF_UNSPEC,
		.tcm_ifindex = cache->c_iarg1,
	};

	return nl_send_simple(sk, RTM_GETTCLASS, NLM_F_DUMP, &tchdr,
			      sizeof(tchdr));
}

/**
 * @name Allocation/Freeing
 * @{
 */

struct rtnl_class *rtnl_class_alloc(void)
{
	struct rtnl_tc *tc;

	tc = TC_CAST(nl_object_alloc(&class_obj_ops));
	if (tc)
		tc->tc_type = RTNL_TC_TYPE_CLASS;

	return (struct rtnl_class *) tc;
}

void rtnl_class_put(struct rtnl_class *class)
{
	nl_object_put((struct nl_object *) class);
}

/** @} */


/**
 * @name Addition/Modification/Deletion
 * @{
 */

static int class_build(struct rtnl_class *class, int type, int flags,
		       struct nl_msg **result)
{
	uint32_t needed = TCA_ATTR_PARENT | TCA_ATTR_HANDLE;

	if ((class->ce_mask & needed) == needed &&
	    TC_H_MAJ(class->c_parent) && TC_H_MAJ(class->c_handle) &&
	    TC_H_MAJ(class->c_parent) != TC_H_MAJ(class->c_handle)) {
		APPBUG("TC_H_MAJ(parent) must match TC_H_MAJ(handle)");
		return -NLE_INVAL;
	}

	return rtnl_tc_msg_build(TC_CAST(class), type, flags, result);
}

/**
 * Build a netlink message requesting the addition of a traffic class
 * @arg class		Traffic class to add 
 * @arg flags		Additional netlink message flags
 * @arg result		Pointer to store resulting netlink message
 *
 * The behaviour of this function is identical to rtnl_class_add() with
 * the exception that it will not send the message but return it int the
 * provided return pointer instead.
 *
 * @see rtnl_class_add()
 *
 * @return 0 on success or a negative error code.
 */
int rtnl_class_build_add_request(struct rtnl_class *class, int flags,
				 struct nl_msg **result)
{
	return class_build(class, RTM_NEWTCLASS, flags, result);
}

/**
 * Add/Update traffic class
 * @arg sk		Netlink socket
 * @arg class		Traffic class to add 
 * @arg flags		Additional netlink message flags
 *
 * Builds a \c RTM_NEWTCLASS netlink message requesting the addition
 * of a new traffic class and sends the message to the kernel. The
 * configuration of the traffic class is derived from the attributes
 * of the specified traffic class.
 *
 * The following flags may be specified:
 *  - \c NLM_F_CREATE:  Create traffic class if it does not exist,
 *                      otherwise -NLE_OBJ_NOTFOUND is returned.
 *  - \c NLM_F_EXCL:    Return -NLE_EXISTS if a traffic class with
 *                      matching handle exists already.
 *
 * Existing traffic classes with matching handles will be updated,
 * unless the flag \c NLM_F_EXCL is specified. If no matching traffic
 * class exists, it will be created if the flag \c NLM_F_CREATE is set,
 * otherwise the error -NLE_OBJ_NOTFOUND is returned. 
 *
 * If the parent qdisc does not support classes, the error
 * \c NLE_OPNOTSUPP is returned.
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
int rtnl_class_add(struct nl_sock *sk, struct rtnl_class *class, int flags)
{
	struct nl_msg *msg;
	int err;

	if ((err = rtnl_class_build_add_request(class, flags, &msg)) < 0)
		return err;

	return nl_send_sync(sk, msg);
}

/**
 * Build netlink message requesting the deletion of a traffic class
 * @arg class		Traffic class to delete
 * @arg result		Pointer to store resulting netlink message
 *
 * The behaviour of this function is identical to rtnl_class_delete() with
 * the exception that it will not send the message but return it in the
 * provided return pointer instead.
 *
 * @see rtnl_class_delete()
 *
 * @return 0 on success or a negative error code.
 */
int rtnl_class_build_delete_request(struct rtnl_class *class, struct nl_msg **result)
{
	struct nl_msg *msg;
	struct tcmsg tchdr;
	uint32_t required = TCA_ATTR_IFINDEX | TCA_ATTR_HANDLE;

	if ((class->ce_mask & required) != required) {
		APPBUG("ifindex and handle must be specified");
		return -NLE_MISSING_ATTR;
	}

	if (!(msg = nlmsg_alloc_simple(RTM_DELTCLASS, 0)))
		return -NLE_NOMEM;

	memset(&tchdr, 0, sizeof(tchdr));
	tchdr.tcm_family = AF_UNSPEC;
	tchdr.tcm_ifindex = class->c_ifindex;
	tchdr.tcm_handle = class->c_handle;

	if (class->ce_mask & TCA_ATTR_PARENT)
		tchdr.tcm_parent = class->c_parent;

	if (nlmsg_append(msg, &tchdr, sizeof(tchdr), NLMSG_ALIGNTO) < 0) {
		nlmsg_free(msg);
		return -NLE_MSGSIZE;
	}

	*result = msg;
	return 0;
}

/**
 * Delete traffic class
 * @arg sk		Netlink socket
 * @arg class		Traffic class to delete
 *
 * Builds a \c RTM_DELTCLASS netlink message requesting the deletion
 * of a traffic class and sends the message to the kernel.
 *
 * The message is constructed out of the following attributes:
 * - \c ifindex and \c handle (required)
 * - \c parent (optional, must match if provided)
 *
 * All other class attributes including all class type specific
 * attributes are ignored.
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
int rtnl_class_delete(struct nl_sock *sk, struct rtnl_class *class)
{
	struct nl_msg *msg;
	int err;

	if ((err = rtnl_class_build_delete_request(class, &msg)) < 0)
		return err;

	return nl_send_sync(sk, msg);
}

/** @} */

/**
 * @name Leaf Qdisc
 * @{
 */

/**
 * Lookup the leaf qdisc of a traffic class
 * @arg class		the parent traffic class
 * @arg cache		a qdisc cache allocated using rtnl_qdisc_alloc_cache()
 *
 * @return Matching Qdisc or NULL if the traffic class has no leaf qdisc
 */
struct rtnl_qdisc *rtnl_class_leaf_qdisc(struct rtnl_class *class,
					 struct nl_cache *cache)
{
	struct rtnl_qdisc *leaf;

	if (!class->c_info)
		return NULL;

	leaf = rtnl_qdisc_get_by_parent(cache, class->c_ifindex,
					class->c_handle);
	if (!leaf || leaf->q_handle != class->c_info)
		return NULL;

	return leaf;
}

/** @} */

/**
 * @name Cache Related Functions
 * @{
 */

/**
 * Allocate a cache and fill it with all configured traffic classes
 * @arg sk		Netlink socket
 * @arg ifindex		Interface index of the network device
 * @arg result		Pointer to store the created cache
 *
 * Allocates a new traffic class cache and fills it with a list of all
 * configured traffic classes on a specific network device. Release the
 * cache with nl_cache_free().
 *
 * @return 0 on success or a negative error code.
 */
int rtnl_class_alloc_cache(struct nl_sock *sk, int ifindex,
			   struct nl_cache **result)
{
	struct nl_cache * cache;
	int err;

	if (!ifindex) {
		APPBUG("ifindex must be specified");
		return -NLE_INVAL;
	}
	
	if (!(cache = nl_cache_alloc(&rtnl_class_ops)))
		return -NLE_NOMEM;

	cache->c_iarg1 = ifindex;
	
	if (sk && (err = nl_cache_refill(sk, cache)) < 0) {
		nl_cache_free(cache);
		return err;
	}

	*result = cache;
	return 0;
}

/**
 * Search traffic class by interface index and handle
 * @arg cache		Traffic class cache
 * @arg ifindex		Interface index
 * @arg handle		ID of traffic class
 *
 * Searches a traffic class cache previously allocated with
 * rtnl_class_alloc_cache() and searches for a traffi class matching
 * the interface index and handle.
 *
 * The reference counter is incremented before returning the traffic
 * class, therefore the reference must be given back with rtnl_class_put()
 * after usage.
 *
 * @return Traffic class or NULL if no match was found.
 */
struct rtnl_class *rtnl_class_get(struct nl_cache *cache, int ifindex,
				  uint32_t handle)
{
	struct rtnl_class *class;
	
	if (cache->c_ops != &rtnl_class_ops)
		return NULL;

	nl_list_for_each_entry(class, &cache->c_items, ce_list) {
		if (class->c_handle == handle && class->c_ifindex == ifindex) {
			nl_object_get((struct nl_object *) class);
			return class;
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
 * Call a callback for each child of a class
 *
 * @deprecated Use of this function is deprecated, it does not allow
 *             to handle the out of memory situation that can occur.
 */
void rtnl_class_foreach_child(struct rtnl_class *class, struct nl_cache *cache,
			      void (*cb)(struct nl_object *, void *), void *arg)
{
	struct rtnl_class *filter;
	
	filter = rtnl_class_alloc();
	if (!filter)
		return;

	rtnl_tc_set_parent(TC_CAST(filter), class->c_handle);
	rtnl_tc_set_ifindex(TC_CAST(filter), class->c_ifindex);
	rtnl_tc_set_kind(TC_CAST(filter), class->c_kind);

	nl_cache_foreach_filter(cache, OBJ_CAST(filter), cb, arg);
	rtnl_class_put(filter);
}

/**
 * Call a callback for each classifier attached to the class
 *
 * @deprecated Use of this function is deprecated, it does not allow
 *             to handle the out of memory situation that can occur.
 */
void rtnl_class_foreach_cls(struct rtnl_class *class, struct nl_cache *cache,
			    void (*cb)(struct nl_object *, void *), void *arg)
{
	struct rtnl_cls *filter;

	filter = rtnl_cls_alloc();
	if (!filter)
		return;

	rtnl_tc_set_ifindex((struct rtnl_tc *) filter, class->c_ifindex);
	rtnl_tc_set_parent((struct rtnl_tc *) filter, class->c_parent);

	nl_cache_foreach_filter(cache, (struct nl_object *) filter, cb, arg);
	rtnl_cls_put(filter);
}

/** @} */

static struct rtnl_tc_type_ops class_ops = {
	.tt_type		= RTNL_TC_TYPE_CLASS,
	.tt_dump_prefix		= "class",
	.tt_dump = {
	    [NL_DUMP_DETAILS]	= class_dump_details,
	},
};

static struct nl_object_ops class_obj_ops = {
	.oo_name		= "route/class",
	.oo_size		= sizeof(struct rtnl_class),
	.oo_free_data         	= rtnl_tc_free_data,
	.oo_clone		= rtnl_tc_clone,
	.oo_dump = {
	    [NL_DUMP_LINE]	= rtnl_tc_dump_line,
	    [NL_DUMP_DETAILS]	= rtnl_tc_dump_details,
	    [NL_DUMP_STATS]	= rtnl_tc_dump_stats,
	},
	.oo_compare		= rtnl_tc_compare,
	.oo_id_attrs		= (TCA_ATTR_IFINDEX | TCA_ATTR_HANDLE),
};

static struct nl_cache_ops rtnl_class_ops = {
	.co_name		= "route/class",
	.co_hdrsize		= sizeof(struct tcmsg),
	.co_msgtypes		= {
					{ RTM_NEWTCLASS, NL_ACT_NEW, "new" },
					{ RTM_DELTCLASS, NL_ACT_DEL, "del" },
					{ RTM_GETTCLASS, NL_ACT_GET, "get" },
					END_OF_MSGTYPES_LIST,
				  },
	.co_protocol		= NETLINK_ROUTE,
	.co_groups		= tc_groups,
	.co_request_update	= &class_request_update,
	.co_msg_parser		= &class_msg_parser,
	.co_obj_ops		= &class_obj_ops,
};

static void __init class_init(void)
{
	rtnl_tc_type_register(&class_ops);
	nl_cache_mngt_register(&rtnl_class_ops);
}

static void __exit class_exit(void)
{
	nl_cache_mngt_unregister(&rtnl_class_ops);
	rtnl_tc_type_unregister(&class_ops);
}

/** @} */
