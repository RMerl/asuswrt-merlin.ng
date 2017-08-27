/*
 * lib/cache_mngr.c	Cache Manager
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2003-2008 Thomas Graf <tgraf@suug.ch>
 */

/**
 * @ingroup cache_mngt
 * @defgroup cache_mngr Manager
 * @brief Helps keeping caches up to date.
 *
 * The purpose of a cache manager is to keep track of caches and
 * automatically receive event notifications to keep the caches
 * up to date with the kernel state. Each manager has exactly one
 * netlink socket assigned which limits the scope of each manager
 * to exactly one netlink family. Therefore all caches committed
 * to a manager must be part of the same netlink family. Due to the
 * nature of a manager, it is not possible to have a cache maintain
 * two instances of the same cache type. The socket is subscribed
 * to the event notification group of each cache and also put into
 * non-blocking mode. Functions exist to poll() on the socket to
 * wait for new events to be received.
 *
 * @code
 * App       libnl                        Kernel
 *        |                            |
 *            +-----------------+        [ notification, link change ]
 *        |   |  Cache Manager  |      | [   (IFF_UP | IFF_RUNNING)  ]
 *            |                 |                |
 *        |   |   +------------+|      |         |  [ notification, new addr ]
 *    <-------|---| route/link |<-------(async)--+  [  10.0.1.1/32 dev eth1  ]
 *        |   |   +------------+|      |                      |
 *            |   +------------+|                             |
 *    <---|---|---| route/addr |<------|-(async)--------------+
 *            |   +------------+|
 *        |   |   +------------+|      |
 *    <-------|---| ...        ||
 *        |   |   +------------+|      |
 *            +-----------------+
 *        |                            |
 * @endcode
 *
 * @par 1) Creating a new cache manager
 * @code
 * struct nl_cache_mngr *mngr;
 *
 * // Allocate a new cache manager for RTNETLINK and automatically
 * // provide the caches added to the manager.
 * mngr = nl_cache_mngr_alloc(NETLINK_ROUTE, NL_AUTO_PROVIDE);
 * @endcode
 *
 * @par 2) Keep track of a cache
 * @code
 * struct nl_cache *cache;
 *
 * // Create a new cache for links/interfaces and ask the manager to
 * // keep it up to date for us. This will trigger a full dump request
 * // to initially fill the cache.
 * cache = nl_cache_mngr_add(mngr, "route/link");
 * @endcode
 *
 * @par 3) Make the manager receive updates
 * @code
 * // Give the manager the ability to receive updates, will call poll()
 * // with a timeout of 5 seconds.
 * if (nl_cache_mngr_poll(mngr, 5000) > 0) {
 *         // Manager received at least one update, dump cache?
 *         nl_cache_dump(cache, ...);
 * }
 * @endcode
 *
 * @par 4) Release cache manager
 * @code
 * nl_cache_mngr_free(mngr);
 * @endcode
 * @{
 */

#include <netlink-local.h>
#include <netlink/netlink.h>
#include <netlink/cache.h>
#include <netlink/utils.h>

static int include_cb(struct nl_object *obj, struct nl_parser_param *p)
{
	struct nl_cache_assoc *ca = p->pp_arg;
	struct nl_cache_ops *ops = ca->ca_cache->c_ops;

	NL_DBG(2, "Including object %p into cache %p\n", obj, ca->ca_cache);
#ifdef NL_DEBUG
	if (nl_debug >= 4)
		nl_object_dump(obj, &nl_debug_dp);
#endif

	if (ops->co_event_filter)
		if (ops->co_event_filter(ca->ca_cache, obj) != NL_OK)
			return 0;

	return nl_cache_include(ca->ca_cache, obj, ca->ca_change, ca->ca_change_data);
}

static int event_input(struct nl_msg *msg, void *arg)
{
	struct nl_cache_mngr *mngr = arg;
	int protocol = nlmsg_get_proto(msg);
	int type = nlmsg_hdr(msg)->nlmsg_type;
	struct nl_cache_ops *ops;
	int i, n;
	struct nl_parser_param p = {
		.pp_cb = include_cb,
	};

	NL_DBG(2, "Cache manager %p, handling new message %p as event\n",
	       mngr, msg);
#ifdef NL_DEBUG
	if (nl_debug >= 4)
		nl_msg_dump(msg, stderr);
#endif

	if (mngr->cm_protocol != protocol)
		BUG();

	for (i = 0; i < mngr->cm_nassocs; i++) {
		if (mngr->cm_assocs[i].ca_cache) {
			ops = mngr->cm_assocs[i].ca_cache->c_ops;
			for (n = 0; ops->co_msgtypes[n].mt_id >= 0; n++)
				if (ops->co_msgtypes[n].mt_id == type)
					goto found;
		}
	}

	return NL_SKIP;

found:
	NL_DBG(2, "Associated message %p to cache %p\n",
	       msg, mngr->cm_assocs[i].ca_cache);
	p.pp_arg = &mngr->cm_assocs[i];

	return nl_cache_parse(ops, NULL, nlmsg_hdr(msg), &p);
}

/**
 * Allocate new cache manager
 * @arg sk		Netlink socket.
 * @arg protocol	Netlink Protocol this manager is used for
 * @arg flags		Flags
 * @arg result		Result pointer
 *
 * @return 0 on success or a negative error code.
 */
int nl_cache_mngr_alloc(struct nl_sock *sk, int protocol, int flags,
			struct nl_cache_mngr **result)
{
	struct nl_cache_mngr *mngr;
	int err = -NLE_NOMEM;

	if (sk == NULL)
		BUG();

	mngr = calloc(1, sizeof(*mngr));
	if (!mngr)
		goto errout;

	mngr->cm_handle = sk;
	mngr->cm_nassocs = 32;
	mngr->cm_protocol = protocol;
	mngr->cm_flags = flags;
	mngr->cm_assocs = calloc(mngr->cm_nassocs,
				 sizeof(struct nl_cache_assoc));
	if (!mngr->cm_assocs)
		goto errout;

	nl_socket_modify_cb(mngr->cm_handle, NL_CB_VALID, NL_CB_CUSTOM,
			    event_input, mngr);

	/* Required to receive async event notifications */
	nl_socket_disable_seq_check(mngr->cm_handle);

	if ((err = nl_connect(mngr->cm_handle, protocol) < 0))
		goto errout;

	if ((err = nl_socket_set_nonblocking(mngr->cm_handle) < 0))
		goto errout;

	NL_DBG(1, "Allocated cache manager %p, protocol %d, %d caches\n",
	       mngr, protocol, mngr->cm_nassocs);

	*result = mngr;
	return 0;

errout:
	nl_cache_mngr_free(mngr);
	return err;
}

/**
 * Add cache responsibility to cache manager
 * @arg mngr		Cache manager.
 * @arg name		Name of cache to keep track of
 * @arg cb		Function to be called upon changes.
 * @arg data		Argument passed on to change callback
 * @arg result		Pointer to store added cache.
 *
 * Allocates a new cache of the specified type and adds it to the manager.
 * The operation will trigger a full dump request from the kernel to
 * initially fill the contents of the cache. The manager will subscribe
 * to the notification group of the cache to keep track of any further
 * changes.
 *
 * @return 0 on success or a negative error code.
 */
int nl_cache_mngr_add(struct nl_cache_mngr *mngr, const char *name,
		      change_func_t cb, void *data, struct nl_cache **result)
{
	struct nl_cache_ops *ops;
	struct nl_cache *cache;
	struct nl_af_group *grp;
	int err, i;

	ops = nl_cache_ops_lookup(name);
	if (!ops)
		return -NLE_NOCACHE;

	if (ops->co_protocol != mngr->cm_protocol)
		return -NLE_PROTO_MISMATCH;

	if (ops->co_groups == NULL)
		return -NLE_OPNOTSUPP;

	for (i = 0; i < mngr->cm_nassocs; i++)
		if (mngr->cm_assocs[i].ca_cache &&
		    mngr->cm_assocs[i].ca_cache->c_ops == ops)
			return -NLE_EXIST;

retry:
	for (i = 0; i < mngr->cm_nassocs; i++)
		if (!mngr->cm_assocs[i].ca_cache)
			break;

	if (i >= mngr->cm_nassocs) {
		mngr->cm_nassocs += 16;
		mngr->cm_assocs = realloc(mngr->cm_assocs,
					  mngr->cm_nassocs *
					  sizeof(struct nl_cache_assoc));
		if (mngr->cm_assocs == NULL)
			return -NLE_NOMEM;
		else {
			NL_DBG(1, "Increased capacity of cache manager %p " \
				  "to %d\n", mngr, mngr->cm_nassocs);
			goto retry;
		}
	}

	cache = nl_cache_alloc(ops);
	if (!cache)
		return -NLE_NOMEM;

	for (grp = ops->co_groups; grp->ag_group; grp++) {
		err = nl_socket_add_membership(mngr->cm_handle, grp->ag_group);
		if (err < 0)
			goto errout_free_cache;
	}

	err = nl_cache_refill(mngr->cm_handle, cache);
	if (err < 0)
		goto errout_drop_membership;

	mngr->cm_assocs[i].ca_cache = cache;
	mngr->cm_assocs[i].ca_change = cb;
	mngr->cm_assocs[i].ca_change_data = data;

	if (mngr->cm_flags & NL_AUTO_PROVIDE)
		nl_cache_mngt_provide(cache);

	NL_DBG(1, "Added cache %p <%s> to cache manager %p\n",
	       cache, nl_cache_name(cache), mngr);

	*result = cache;
	return 0;

errout_drop_membership:
	for (grp = ops->co_groups; grp->ag_group; grp++)
		nl_socket_drop_membership(mngr->cm_handle, grp->ag_group);
errout_free_cache:
	nl_cache_free(cache);

	return err;
}

/**
 * Get file descriptor
 * @arg mngr		Cache Manager
 *
 * Get the file descriptor of the socket associated to the manager.
 * This can be used to change socket options or monitor activity
 * using poll()/select().
 */
int nl_cache_mngr_get_fd(struct nl_cache_mngr *mngr)
{
	return nl_socket_get_fd(mngr->cm_handle);
}

/**
 * Check for event notifications
 * @arg mngr		Cache Manager
 * @arg timeout		Upper limit poll() will block, in milliseconds.
 *
 * Causes poll() to be called to check for new event notifications
 * being available. Automatically receives and handles available
 * notifications.
 *
 * This functionally is ideally called regularly during an idle
 * period.
 *
 * @return A positive value if at least one update was handled, 0
 *         for none, or a  negative error code.
 */
int nl_cache_mngr_poll(struct nl_cache_mngr *mngr, int timeout)
{
	int ret;
	struct pollfd fds = {
		.fd = nl_socket_get_fd(mngr->cm_handle),
		.events = POLLIN,
	};

	NL_DBG(3, "Cache manager %p, poll() fd %d\n", mngr, fds.fd);
	ret = poll(&fds, 1, timeout);
	NL_DBG(3, "Cache manager %p, poll() returned %d\n", mngr, ret);
	if (ret < 0)
		return -nl_syserr2nlerr(errno);

	if (ret == 0)
		return 0;

	return nl_cache_mngr_data_ready(mngr);
}

/**
 * Receive available event notifications
 * @arg mngr		Cache manager
 *
 * This function can be called if the socket associated to the manager
 * contains updates to be received. This function should not be used
 * if nl_cache_mngr_poll() is used.
 *
 * @return A positive value if at least one update was handled, 0
 *         for none, or a  negative error code.
 */
int nl_cache_mngr_data_ready(struct nl_cache_mngr *mngr)
{
	int err;

	err = nl_recvmsgs_default(mngr->cm_handle);
	if (err < 0)
		return err;

	return 1;
}

/**
 * Free cache manager and all caches.
 * @arg mngr		Cache manager.
 *
 * Release all resources after usage of a cache manager.
 */
void nl_cache_mngr_free(struct nl_cache_mngr *mngr)
{
	int i;

	if (!mngr)
		return;

	if (mngr->cm_handle)
		nl_close(mngr->cm_handle);

	for (i = 0; i < mngr->cm_nassocs; i++) {
		if (mngr->cm_assocs[i].ca_cache) {
			nl_cache_mngt_unprovide(mngr->cm_assocs[i].ca_cache);
			nl_cache_free(mngr->cm_assocs[i].ca_cache);
		}
	}

	free(mngr->cm_assocs);
	free(mngr);

	NL_DBG(1, "Cache manager %p freed\n", mngr);
}

/** @} */
