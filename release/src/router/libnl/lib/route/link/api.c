/*
 * lib/route/link/api.c		Link Info API
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2003-2008 Thomas Graf <tgraf@suug.ch>
 */

/**
 * @ingroup link
 * @defgroup link_API Link Modules API
 * @brief API for modules implementing specific link types/semantics.
 *
 * @par 1) Registering/Unregistering a new link info type
 * @code
 * static struct rtnl_link_info_ops vlan_info_ops = {
 * 	.io_name		= "vlan",
 * 	.io_alloc		= vlan_alloc,
 * 	.io_parse		= vlan_parse,
 * 	.io_dump[NL_DUMP_BRIEF]	= vlan_dump_brief,
 * 	.io_dump[NL_DUMP_FULL]	= vlan_dump_full,
 * 	.io_free		= vlan_free,
 * };
 *
 * static void __init vlan_init(void)
 * {
 * 	rtnl_link_register_info(&vlan_info_ops);
 * }
 *
 * static void __exit vlan_exit(void)
 * {
 * 	rtnl_link_unregister_info(&vlan_info_ops);
 * }
 * @endcode
 *
 * @{
 */

#include <netlink-private/netlink.h>
#include <netlink/netlink.h>
#include <netlink/utils.h>
#include <netlink/route/link.h>
#include <netlink-private/route/link/api.h>

static NL_LIST_HEAD(info_ops);

/* lock protecting info_ops and af_ops */
static NL_RW_LOCK(info_lock);

static struct rtnl_link_info_ops *__rtnl_link_info_ops_lookup(const char *name)
{
	struct rtnl_link_info_ops *ops;

	nl_list_for_each_entry(ops, &info_ops, io_list)
		if (!strcmp(ops->io_name, name))
			return ops;

	return NULL;
}

/**
 * @name Link Info Modules
 * @{
 */

/**
 * Return operations of a specific link info type
 * @arg name		Name of link info type.
 *
 * @note The returned pointer must be given back using rtnl_link_info_ops_put()
 *
 * @return Pointer to operations or NULL if unavailable.
 */
struct rtnl_link_info_ops *rtnl_link_info_ops_lookup(const char *name)
{
	struct rtnl_link_info_ops *ops;

	nl_write_lock(&info_lock);
	if ((ops = __rtnl_link_info_ops_lookup(name)))
		ops->io_refcnt++;
	nl_write_unlock(&info_lock);

	return ops;
}

/**
 * Give back reference to a set of operations.
 * @arg ops		Link info operations.
 */
void rtnl_link_info_ops_put(struct rtnl_link_info_ops *ops)
{
	if (ops)
		ops->io_refcnt--;
}

/**
 * Register operations for a link info type
 * @arg ops		Link info operations
 *
 * This function must be called by modules implementing a specific link
 * info type. It will make the operations implemented by the module
 * available for everyone else.
 *
 * @return 0 on success or a negative error code.
 * @return -NLE_INVAL Link info name not specified.
 * @return -NLE_EXIST Operations for address family already registered.
 */
int rtnl_link_register_info(struct rtnl_link_info_ops *ops)
{
	int err = 0;

	if (ops->io_name == NULL)
		return -NLE_INVAL;

	nl_write_lock(&info_lock);
	if (__rtnl_link_info_ops_lookup(ops->io_name)) {
		err = -NLE_EXIST;
		goto errout;
	}

	NL_DBG(1, "Registered link info operations %s\n", ops->io_name);

	nl_list_add_tail(&ops->io_list, &info_ops);
errout:
	nl_write_unlock(&info_lock);

	return err;
}

/**
 * Unregister operations for a link info type
 * @arg ops		Link info operations
 *
 * This function must be called if a module implementing a specific link
 * info type is unloaded or becomes unavailable. It must provide a
 * set of operations which have previously been registered using
 * rtnl_link_register_info().
 *
 * @return 0 on success or a negative error code
 * @return _NLE_OPNOTSUPP Link info operations not registered.
 * @return -NLE_BUSY Link info operations still in use.
 */
int rtnl_link_unregister_info(struct rtnl_link_info_ops *ops)
{
	struct rtnl_link_info_ops *t;
	int err = -NLE_OPNOTSUPP;

	nl_write_lock(&info_lock);

	nl_list_for_each_entry(t, &info_ops, io_list) {
		if (t == ops) {
			if (t->io_refcnt > 0) {
				err = -NLE_BUSY;
				goto errout;
			}

			nl_list_del(&t->io_list);

			NL_DBG(1, "Unregistered link info operations %s\n",
				ops->io_name);
			err = 0;
			goto errout;
		}
	}

errout:
	nl_write_unlock(&info_lock);

	return err;
}

/** @} */

/**
 * @name Link Address Family Modules
 * @{
 */

static struct rtnl_link_af_ops *af_ops[AF_MAX];

/**
 * Return operations of a specific link address family
 * @arg family		Address family
 *
 * @note The returned pointer must be given back using rtnl_link_af_ops_put()
 *
 * @return Pointer to operations or NULL if unavailable.
 */
struct rtnl_link_af_ops *rtnl_link_af_ops_lookup(const unsigned int family)
{
	if (family == AF_UNSPEC || family >= AF_MAX)
		return NULL;

	nl_write_lock(&info_lock);
	if (af_ops[family])
		af_ops[family]->ao_refcnt++;
	nl_write_unlock(&info_lock);

	return af_ops[family];
}

/**
 * Give back reference to a set of operations.
 * @arg ops		Address family operations.
 */
void rtnl_link_af_ops_put(struct rtnl_link_af_ops *ops)
{
	if (ops)
		ops->ao_refcnt--;
}

/**
 * Allocate and return data buffer for link address family modules
 * @arg link		Link object
 * @arg ops		Address family operations
 *
 * This function must be called by link address family modules in all
 * cases where the API does not provide the data buffer as argument
 * already. This typically includes set functions the module provides.
 * Calling this function is strictly required to ensure proper allocation
 * of the buffer upon first use. Link objects will NOT proactively
 * allocate a data buffer for each registered link address family.
 *
 * @return Pointer to data buffer or NULL on error.
 */
void *rtnl_link_af_alloc(struct rtnl_link *link,
			 const struct rtnl_link_af_ops *ops)
{
	int family;

	if (!link || !ops)
		BUG();

	family = ops->ao_family;

	if (!link->l_af_data[family]) {
		if (!ops->ao_alloc)
			BUG();
		
		link->l_af_data[family] = ops->ao_alloc(link);
		if (!link->l_af_data[family])
			return NULL;
	}

	return link->l_af_data[family];
}

/**
 * Return data buffer for link address family modules
 * @arg link		Link object
 * @arg ops		Address family operations
 *
 * This function returns a pointer to the data buffer for the specified link
 * address family module or NULL if the buffer was not allocated yet. This
 * function is typically used by get functions of modules which are not
 * interested in having the data buffer allocated if no values have been set
 * yet.
 *
 * @return Pointer to data buffer or NULL on error.
 */
void *rtnl_link_af_data(const struct rtnl_link *link,
			const struct rtnl_link_af_ops *ops)
{
	if (!link || !ops)
		BUG();

	return link->l_af_data[ops->ao_family];
}

/**
 * Register operations for a link address family
 * @arg ops		Address family operations
 *
 * This function must be called by modules implementing a specific link
 * address family. It will make the operations implemented by the module
 * available for everyone else.
 *
 * @return 0 on success or a negative error code.
 * @return -NLE_INVAL Address family is out of range (0..AF_MAX)
 * @return -NLE_EXIST Operations for address family already registered.
 */
int rtnl_link_af_register(struct rtnl_link_af_ops *ops)
{
	int err = 0;

	if (ops->ao_family == AF_UNSPEC || ops->ao_family >= AF_MAX)
		return -NLE_INVAL;

	nl_write_lock(&info_lock);
	if (af_ops[ops->ao_family]) {
		err = -NLE_EXIST;
		goto errout;
	}

	ops->ao_refcnt = 0;
	af_ops[ops->ao_family] = ops;

	NL_DBG(1, "Registered link address family operations %u\n",
		ops->ao_family);

errout:
	nl_write_unlock(&info_lock);

	return err;
}

/**
 * Unregister operations for a link address family
 * @arg ops		Address family operations
 *
 * This function must be called if a module implementing a specific link
 * address family is unloaded or becomes unavailable. It must provide a
 * set of operations which have previously been registered using
 * rtnl_link_af_register().
 *
 * @return 0 on success or a negative error code
 * @return -NLE_INVAL ops is NULL
 * @return -NLE_OBJ_NOTFOUND Address family operations not registered.
 * @return -NLE_BUSY Address family operations still in use.
 */
int rtnl_link_af_unregister(struct rtnl_link_af_ops *ops)
{
	int err = -NLE_INVAL;

	if (!ops)
		return err;

	nl_write_lock(&info_lock);
	if (!af_ops[ops->ao_family]) {
		err = -NLE_OBJ_NOTFOUND;
		goto errout;
	}

	if (ops->ao_refcnt > 0) {
		err = -NLE_BUSY;
		goto errout;
	}

	af_ops[ops->ao_family] = NULL;

	NL_DBG(1, "Unregistered link address family operations %u\n",
		ops->ao_family);

errout:
	nl_write_unlock(&info_lock);

	return err;
}

/**
 * Compare af data for a link address family
 * @arg a		Link object a
 * @arg b		Link object b
 * @arg family		af data family
 *
 * This function will compare af_data between two links
 * a and b of family given by arg family
 *
 * @return 0 if address family specific data matches or is not present
 * or != 0 if it mismatches.
 */
int rtnl_link_af_data_compare(struct rtnl_link *a, struct rtnl_link *b,
			      int family)
{
	struct rtnl_link_af_ops *af_ops;
	int ret = 0;

	if (!a->l_af_data[family] && !b->l_af_data[family])
		return 0;

	if (!a->l_af_data[family] || !b->l_af_data[family])
		return ~0;

	af_ops = rtnl_link_af_ops_lookup(family);
	if (!af_ops)
		return ~0;

	if (af_ops->ao_compare == NULL) {
		ret = ~0;
		goto out;
	}

	ret = af_ops->ao_compare(a, b, family, ~0, 0);

out:
	rtnl_link_af_ops_put(af_ops);

	return ret;
}

/** @} */

/** @} */

