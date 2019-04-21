/*
 * (C) 2005-2011 by Pablo Neira Ayuso <pablo@netfilter.org>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <stdlib.h>
#include <stdbool.h>
#include <string.h> /* for memset */
#include <errno.h>
#include <assert.h>

#include "internal/internal.h"

/**
 * \mainpage
 *
 * libnetfilter_conntrack is a userspace library providing a programming
 * interface (API) to the in-kernel connection tracking state table. The
 * library libnetfilter_conntrack has been previously known as
 * libnfnetlink_conntrack and libctnetlink. This library is currently used by
 * conntrack-tools among many other applications.
 *
 * libnetfilter_conntrack homepage is:
 *      http://netfilter.org/projects/libnetfilter_conntrack/
 *
 * \section Dependencies
 * libnetfilter_conntrack requires libnfnetlink and a kernel that includes the
 * nf_conntrack_netlink subsystem (i.e. 2.6.14 or later, >= 2.6.18 recommended).
 *
 * \section Main Features
 *  - listing/retrieving entries from the kernel connection tracking table.
 *  - inserting/modifying/deleting entries from the kernel connection tracking
 *    table.
 *  - listing/retrieving entries from the kernel expect table.
 *  - inserting/modifying/deleting entries from the kernel expect table.
 * \section Git Tree
 * The current development version of libnetfilter_conntrack can be accessed at
 * https://git.netfilter.org/cgi-bin/gitweb.cgi?p=libnetfilter_conntrack.git
 *
 * \section Privileges
 * You need the CAP_NET_ADMIN capability in order to allow your application
 * to receive events from and to send commands to kernel-space, excepting
 * the conntrack table dumping operation.
 *
 * \section using Using libnetfilter_conntrack
 * To write your own program using libnetfilter_conntrack, you should start by
 * reading the doxygen documentation (start by \link LibrarySetup \endlink page)
 * and check examples available under utils/ in the libnetfilter_conntrack
 * source code tree. You can compile these examples by invoking `make check'.
 *
 * \section Authors
 * libnetfilter_conntrack has been almost entirely written by Pablo Neira Ayuso.
 *
 * \section python Python Binding
 * pynetfilter_conntrack is a Python binding of libnetfilter_conntrack written
 * by Victor Stinner. You can visit his official web site at
 * http://software.inl.fr/trac/trac.cgi/wiki/pynetfilter_conntrack.
 */

/**
 * \defgroup ct Conntrack object handling
 * @{
 */

/**
 * nfct_conntrack_new - allocate a new conntrack
 *
 * In case of success, this function returns a valid pointer to a memory blob,
 * otherwise NULL is returned and errno is set appropiately.
 */
struct nf_conntrack *nfct_new(void)
{
	struct nf_conntrack *ct;

	ct = malloc(sizeof(struct nf_conntrack));
	if (!ct)
		return NULL;

	memset(ct, 0, sizeof(struct nf_conntrack));

	return ct;
}

/**
 * nf_conntrack_destroy - release a conntrack object
 * \param ct pointer to the conntrack object
 */
void nfct_destroy(struct nf_conntrack *ct)
{
	assert(ct != NULL);
	if (ct->secctx)
		free(ct->secctx);
	if (ct->helper_info)
		free(ct->helper_info);
	if (ct->connlabels)
		nfct_bitmask_destroy(ct->connlabels);
	if (ct->connlabels_mask)
		nfct_bitmask_destroy(ct->connlabels_mask);
	free(ct);
	ct = NULL; /* bugtrap */
}

/**
 * nf_sizeof - return the size in bytes of a certain conntrack object
 * \param ct pointer to the conntrack object
 *
 * This function is DEPRECATED, don't use it in your code.
 */
size_t nfct_sizeof(const struct nf_conntrack *ct)
{
	assert(ct != NULL);
	return sizeof(*ct);
}

/**
 * nfct_maxsize - return the maximum size in bytes of a conntrack object
 *
 * Use this function if you want to allocate a conntrack object in the stack
 * instead of the heap. For example:
 * \verbatim
	char buf[nfct_maxsize()];
	struct nf_conntrack *ct = (struct nf_conntrack *) buf;
	memset(ct, 0, nfct_maxsize());
\endverbatim
 * Note: As for now this function returns the same size that nfct_sizeof(ct)
 * does although _this could change in the future_. Therefore, do not assume
 * that nfct_sizeof(ct) == nfct_maxsize().
 *
 * This function is DEPRECATED, don't use it in your code.
 */
size_t nfct_maxsize(void)
{
	return sizeof(struct nf_conntrack);
}

/**
 * nfct_clone - clone a conntrack object
 * \param ct pointer to a valid conntrack object
 *
 * On error, NULL is returned and errno is appropiately set. Otherwise,
 * a valid pointer to the clone conntrack is returned.
 */
struct nf_conntrack *nfct_clone(const struct nf_conntrack *ct)
{
	struct nf_conntrack *clone;

	assert(ct != NULL);

	if ((clone = nfct_new()) == NULL)
		return NULL;
	nfct_copy(clone, ct, NFCT_CP_OVERRIDE);

	return clone;
}

/**
 * nfct_setobjopt - set a certain option for a conntrack object
 * \param ct conntrack object
 * \param option option parameter
 *
 * In case of error, -1 is returned and errno is appropiately set. On success,
 * 0 is returned.
 */
int nfct_setobjopt(struct nf_conntrack *ct, unsigned int option)
{
	assert(ct != NULL);

	if (unlikely(option > NFCT_SOPT_MAX)) {
		errno = EOPNOTSUPP;
		return -1;
	}

	return __setobjopt(ct, option);
}

/**
 * nfct_getobjopt - get a certain option for a conntrack object
 * \param ct conntrack object
 * \param option option parameter
 *
 * In case of error, -1 is returned and errno is appropiately set. On success,
 * 0 is returned.
 */
int nfct_getobjopt(const struct nf_conntrack *ct, unsigned int option)
{
	assert(ct != NULL);

	if (unlikely(option > NFCT_GOPT_MAX)) {
		errno = EOPNOTSUPP;
		return -1;
	}

	return __getobjopt(ct, option);
}

/**
 * @}
 */

/**
 * \defgroup LibrarySetup Library setup
 * @{
 */

/**
 * nf_callback_register - register a callback
 * \param h library handler
 * \param type message type (see enum nf_conntrack_msg_type definition)
 * \param cb callback used to process conntrack received
 * \param data data used by the callback, if any.
 *
 * This function register a callback to handle the conntrack received, 
 * in case of error -1 is returned and errno is set appropiately, otherwise
 * 0 is returned.
 *
 * Note that the data parameter is optional, if you do not want to pass any
 * data to your callback, then use NULL.
 */
int nfct_callback_register(struct nfct_handle *h,
			   enum nf_conntrack_msg_type type,
			   int (*cb)(enum nf_conntrack_msg_type type,
			   	     struct nf_conntrack *ct, 
				     void *data),
			   void *data)
{
	struct __data_container *container;

	assert(h != NULL);

	container = malloc(sizeof(struct __data_container));
	if (!container)
		return -1;
	memset(container, 0, sizeof(struct __data_container));

	h->cb = cb;
	container->h = h;
	container->type = type;
	container->data = data;

	h->nfnl_cb_ct.call = __callback;
	h->nfnl_cb_ct.data = container;
	h->nfnl_cb_ct.attr_count = CTA_MAX;

	nfnl_callback_register(h->nfnlssh_ct, 
			       IPCTNL_MSG_CT_NEW,
			       &h->nfnl_cb_ct);

	nfnl_callback_register(h->nfnlssh_ct,
			       IPCTNL_MSG_CT_DELETE,
			       &h->nfnl_cb_ct);

	return 0;
}

/**
 * nfct_callback_unregister - unregister a callback
 * \param h library handler
 */
void nfct_callback_unregister(struct nfct_handle *h)
{
	assert(h != NULL);

	nfnl_callback_unregister(h->nfnlssh_ct, IPCTNL_MSG_CT_NEW);
	nfnl_callback_unregister(h->nfnlssh_ct, IPCTNL_MSG_CT_DELETE);

	h->cb = NULL;
	free(h->nfnl_cb_ct.data);

	h->nfnl_cb_ct.call = NULL;
	h->nfnl_cb_ct.data = NULL;
	h->nfnl_cb_ct.attr_count = 0;
}

/**
 * nf_callback_register2 - register a callback
 * \param h library handler
 * \param cb callback used to process conntrack received
 * \param data data used by the callback, if any.
 *
 * This function register a callback to handle the conntrack received, 
 * in case of error -1 is returned and errno is set appropiately, otherwise
 * 0 is returned.
 *
 * Note that the data parameter is optional, if you do not want to pass any
 * data to your callback, then use NULL.
 *
 * NOTICE: The difference with nf_callback_register() is that this function
 * uses the new callback interface that includes the Netlink header.
 *
 * WARNING: Don't mix nf_callback_register() and nf_callback_register2()
 * calls, use only once at a time.
 */
int nfct_callback_register2(struct nfct_handle *h,
			    enum nf_conntrack_msg_type type,
			    int (*cb)(const struct nlmsghdr *nlh,
			    	      enum nf_conntrack_msg_type type,
				      struct nf_conntrack *ct, 
				      void *data),
			   void *data)
{
	struct __data_container *container;

	assert(h != NULL);

	container = calloc(sizeof(struct __data_container), 1);
	if (container == NULL)
		return -1;

	h->cb2 = cb;
	container->h = h;
	container->type = type;
	container->data = data;

	h->nfnl_cb_ct.call = __callback;
	h->nfnl_cb_ct.data = container;
	h->nfnl_cb_ct.attr_count = CTA_MAX;

	nfnl_callback_register(h->nfnlssh_ct, 
			       IPCTNL_MSG_CT_NEW,
			       &h->nfnl_cb_ct);

	nfnl_callback_register(h->nfnlssh_ct,
			       IPCTNL_MSG_CT_DELETE,
			       &h->nfnl_cb_ct);

	return 0;
}

/**
 * nfct_callback_unregister2 - unregister a callback
 * \param h library handler
 */
void nfct_callback_unregister2(struct nfct_handle *h)
{
	assert(h != NULL);

	nfnl_callback_unregister(h->nfnlssh_ct, IPCTNL_MSG_CT_NEW);
	nfnl_callback_unregister(h->nfnlssh_ct, IPCTNL_MSG_CT_DELETE);

	h->cb2 = NULL;
	free(h->nfnl_cb_ct.data);

	h->nfnl_cb_ct.call = NULL;
	h->nfnl_cb_ct.data = NULL;
	h->nfnl_cb_ct.attr_count = 0;
}

/**
 * @}
 */

/**
 * \defgroup ct Conntrack object handling
 * @{
 */

/**
 * nfct_set_attr_l - set the value of a certain conntrack attribute
 * \param ct pointer to a valid conntrack
 * \param type attribute type
 * \param pointer to attribute value
 * \param length of attribute value (in bytes)
 */
void
nfct_set_attr_l(struct nf_conntrack *ct, const enum nf_conntrack_attr type,
		const void *value, size_t len)
{
	assert(ct != NULL);
	assert(value != NULL);

	if (unlikely(type >= ATTR_MAX))
		return;

	if (set_attr_array[type]) {
		set_attr_array[type](ct, value, len);
		set_bit(type, ct->head.set);
	}
}

/**
 * nfct_set_attr - set the value of a certain conntrack attribute
 * \param ct pointer to a valid conntrack
 * \param type attribute type
 * \param value pointer to the attribute value
 *
 * Note that certain attributes are unsettable:
 * 	- ATTR_USE
 * 	- ATTR_ID
 * 	- ATTR_*_COUNTER_*
 *	- ATTR_SECCTX
 *	- ATTR_TIMESTAMP_*
 * The call of this function for such attributes do nothing.
 */
void nfct_set_attr(struct nf_conntrack *ct,
		   const enum nf_conntrack_attr type, 
		   const void *value)
{
	/* We assume the setter knows the size of the passed pointer. */
	nfct_set_attr_l(ct, type, value, 0);
}

/**
 * nfct_set_attr_u8 - set the value of a certain conntrack attribute
 * \param ct pointer to a valid conntrack
 * \param type attribute type
 * \param value unsigned 8 bits attribute value
 */
void nfct_set_attr_u8(struct nf_conntrack *ct,
		      const enum nf_conntrack_attr type, 
		      uint8_t value)
{
	nfct_set_attr_l(ct, type, &value, sizeof(uint8_t));
}

/**
 * nfct_set_attr_u16 - set the value of a certain conntrack attribute
 * \param ct pointer to a valid conntrack
 * \param type attribute type
 * \param value unsigned 16 bits attribute value
 */
void nfct_set_attr_u16(struct nf_conntrack *ct,
		       const enum nf_conntrack_attr type, 
		       uint16_t value)
{
	nfct_set_attr_l(ct, type, &value, sizeof(uint16_t));
}

/**
 * nfct_set_attr_u32 - set the value of a certain conntrack attribute
 * \param ct pointer to a valid conntrack
 * \param type attribute type
 * \param value unsigned 32 bits attribute value
 */
void nfct_set_attr_u32(struct nf_conntrack *ct,
		       const enum nf_conntrack_attr type, 
		       uint32_t value)
{
	nfct_set_attr_l(ct, type, &value, sizeof(uint32_t));
}

/**
 * nfct_set_attr_u64 - set the value of a certain conntrack attribute
 * \param ct pointer to a valid conntrack
 * \param type attribute type
 * \param value unsigned 64 bits attribute value
 */
void nfct_set_attr_u64(struct nf_conntrack *ct,
		       const enum nf_conntrack_attr type, 
		       uint64_t value)
{
	nfct_set_attr_l(ct, type, &value, sizeof(uint64_t));
}

/**
 * nfct_get_attr - get a conntrack attribute
 * \param ct pointer to a valid conntrack
 * \param type attribute type
 *
 * In case of success a valid pointer to the attribute requested is returned,
 * on error NULL is returned and errno is set appropiately.
 */
const void *nfct_get_attr(const struct nf_conntrack *ct,
			  const enum nf_conntrack_attr type)
{
	assert(ct != NULL);

	if (unlikely(type >= ATTR_MAX)) {
		errno = EINVAL;
		return NULL;
	}

	if (!test_bit(type, ct->head.set)) {
		errno = ENODATA;
		return NULL;
	}

	assert(get_attr_array[type]);

	return get_attr_array[type](ct);
}

/**
 * nfct_get_attr_u8 - get attribute of unsigned 8-bits long
 * \param ct pointer to a valid conntrack
 * \param type attribute type
 *
 * Returns the value of the requested attribute, if the attribute is not 
 * set, 0 is returned. In order to check if the attribute is set or not,
 * use nfct_attr_is_set.
 */
uint8_t nfct_get_attr_u8(const struct nf_conntrack *ct,
			  const enum nf_conntrack_attr type)
{
	const uint8_t *ret = nfct_get_attr(ct, type);
	return ret == NULL ? 0 : *ret;
}

/**
 * nfct_get_attr_u16 - get attribute of unsigned 16-bits long
 * \param ct pointer to a valid conntrack
 * \param type attribute type
 *
 * Returns the value of the requested attribute, if the attribute is not 
 * set, 0 is returned. In order to check if the attribute is set or not,
 * use nfct_attr_is_set.
 */
uint16_t nfct_get_attr_u16(const struct nf_conntrack *ct,
			    const enum nf_conntrack_attr type)
{
	const uint16_t *ret = nfct_get_attr(ct, type);
	return ret == NULL ? 0 : *ret;
}

/**
 * nfct_get_attr_u32 - get attribute of unsigned 32-bits long
 * \param ct pointer to a valid conntrack
 * \param type attribute type
 *
 * Returns the value of the requested attribute, if the attribute is not 
 * set, 0 is returned. In order to check if the attribute is set or not,
 * use nfct_attr_is_set.
 */
uint32_t nfct_get_attr_u32(const struct nf_conntrack *ct,
			    const enum nf_conntrack_attr type)
{
	const uint32_t *ret = nfct_get_attr(ct, type);
	return ret == NULL ? 0 : *ret;
}

/**
 * nfct_get_attr_u64 - get attribute of unsigned 32-bits long
 * \param ct pointer to a valid conntrack
 * \param type attribute type
 *
 * Returns the value of the requested attribute, if the attribute is not 
 * set, 0 is returned. In order to check if the attribute is set or not,
 * use nfct_attr_is_set.
 */
uint64_t nfct_get_attr_u64(const struct nf_conntrack *ct,
			    const enum nf_conntrack_attr type)
{
	const uint64_t *ret = nfct_get_attr(ct, type);
	return ret == NULL ? 0 : *ret;
}

/**
 * nfct_attr_is_set - check if a certain attribute is set
 * \param ct pointer to a valid conntrack object
 * \param type attribute type
 *
 * On error, -1 is returned and errno is set appropiately, otherwise
 * the value of the attribute is returned.
 */
int nfct_attr_is_set(const struct nf_conntrack *ct,
		     const enum nf_conntrack_attr type)
{
	assert(ct != NULL);

	if (unlikely(type >= ATTR_MAX)) {
		errno = EINVAL;
		return -1;
	}
	return test_bit(type, ct->head.set);
}

/**
 * nfct_attr_is_set_array - check if an array of attribute types is set
 * \param ct pointer to a valid conntrack object
 * \param array attribute type array
 * \param size size of the array
 *
 * On error, -1 is returned and errno is set appropiately, otherwise
 * the value of the attribute is returned.
 */
int nfct_attr_is_set_array(const struct nf_conntrack *ct,
			   const enum nf_conntrack_attr *type_array,
			   int size)
{
	int i;

	assert(ct != NULL);

	for (i=0; i<size; i++) {
		if (unlikely(type_array[i] >= ATTR_MAX)) {
			errno = EINVAL;
			return -1;
		}
		if (!test_bit(type_array[i], ct->head.set))
			return 0;
	}
	return 1;
}

/**
 * nfct_attr_unset - unset a certain attribute
 * \param type attribute type
 * \param ct pointer to a valid conntrack object
 *
 * On error, -1 is returned and errno is set appropiately, otherwise
 * 0 is returned.
 */
int nfct_attr_unset(struct nf_conntrack *ct,
		    const enum nf_conntrack_attr type)
{
	assert(ct != NULL);

	if (unlikely(type >= ATTR_MAX)) {
		errno = EINVAL;
		return -1;
	}
	unset_bit(type, ct->head.set);

	return 0;
}

/**
 * nfct_set_attr_grp - set a group of attributes
 * \param ct pointer to a valid conntrack object
 * \param type attribute group (see ATTR_GRP_*)
 * \param data pointer to struct (see struct nfct_attr_grp_*)
 *
 * Note that calling this function for ATTR_GRP_COUNTER_* and ATTR_GRP_ADDR_*
 * have no effect.
 */
void nfct_set_attr_grp(struct nf_conntrack *ct,
		       const enum nf_conntrack_attr_grp type,
		       const void *data)
{
	assert(ct != NULL);

	if (unlikely(type >= ATTR_GRP_MAX))
		return;

	if (set_attr_grp_array[type]) {
		set_attr_grp_array[type](ct, data);
		set_bitmask_u32(ct->head.set,
				attr_grp_bitmask[type].bitmask, __NFCT_BITSET);
	}
}

/**
 * nfct_get_attr_grp - get an attribute group
 * \param ct pointer to a valid conntrack object
 * \param type attribute group (see ATTR_GRP_*)
 * \param data pointer to struct (see struct nfct_attr_grp_*)
 *
 * On error, it returns -1 and errno is appropriately set. On success, the
 * data pointer contains the attribute group.
 */
int nfct_get_attr_grp(const struct nf_conntrack *ct,
		      const enum nf_conntrack_attr_grp type,
		      void *data)
{
	assert(ct != NULL);

	if (unlikely(type >= ATTR_GRP_MAX)) {
		errno = EINVAL;
		return -1;
	}
	switch(attr_grp_bitmask[type].type) {
	case NFCT_BITMASK_AND:
		if (!test_bitmask_u32(ct->head.set,
				      attr_grp_bitmask[type].bitmask,
				      __NFCT_BITSET)) {
			errno = ENODATA;
			return -1;
		}
		break;
	case NFCT_BITMASK_OR:
		if (!test_bitmask_u32_or(ct->head.set,
					 attr_grp_bitmask[type].bitmask,
					 __NFCT_BITSET)) {
			errno = ENODATA;
			return -1;
		}
		break;
	}
	assert(get_attr_grp_array[type]);
	get_attr_grp_array[type](ct, data);
	return 0;
}

/**
 * nfct_attr_grp_is_set - check if an attribute group is set
 * \param ct pointer to a valid conntrack object
 * \param type attribute group (see ATTR_GRP_*)
 *
 * If the attribute group is set, this function returns 1, otherwise 0.
 */
int nfct_attr_grp_is_set(const struct nf_conntrack *ct,
			 const enum nf_conntrack_attr_grp type)
{
	assert(ct != NULL);

	if (unlikely(type >= ATTR_GRP_MAX)) {
		errno = EINVAL;
		return -1;
	}
	switch(attr_grp_bitmask[type].type) {
	case NFCT_BITMASK_AND:
		if (test_bitmask_u32(ct->head.set,
				     attr_grp_bitmask[type].bitmask,
				     __NFCT_BITSET)) {
			return 1;
		}
		break;
	case NFCT_BITMASK_OR:
		if (test_bitmask_u32_or(ct->head.set,
					attr_grp_bitmask[type].bitmask,
					__NFCT_BITSET)) {
			return 1;
		}
		break;
	}
	return 0;
}

/**
 * nfct_attr_grp_unset - unset an attribute group
 * \param ct pointer to a valid conntrack object
 * \param type attribute group (see ATTR_GRP_*)
 *
 * On error, it returns -1 and errno is appropriately set. On success, 
 * this function returns 0.
 */
int nfct_attr_grp_unset(struct nf_conntrack *ct,
			const enum nf_conntrack_attr_grp type)
{
	assert(ct != NULL);

	if (unlikely(type >= ATTR_GRP_MAX)) {
		errno = EINVAL;
		return -1;
	}
	unset_bitmask_u32(ct->head.set, attr_grp_bitmask[type].bitmask,
			  __NFCT_BITSET);

	return 0;
}

/**
 * @}
 */

/**
 * \defgroup nl Low level object to Netlink message
 * @{
 */

/**
 * nfct_build_conntrack - build a netlink message from a conntrack object
 * \param ssh nfnetlink subsystem handler
 * \param req buffer used to build the netlink message
 * \param size size of the buffer passed
 * \param type netlink message type
 * \param flags netlink flags
 * \param ct pointer to a conntrack object
 *
 * This is a low level function for those that require to be close to
 * netlink details via libnfnetlink. If you do want to obviate the netlink
 * details then we suggest you to use nfct_query.
 *
 * On error, -1 is returned and errno is appropiately set.
 * On success, 0 is returned.
 */
int nfct_build_conntrack(struct nfnl_subsys_handle *ssh,
			 void *req,
			 size_t size,
			 uint16_t type,
			 uint16_t flags,
			 const struct nf_conntrack *ct)
{
	assert(ssh != NULL);
	assert(req != NULL);
	assert(ct != NULL);

	return __build_conntrack(ssh, req, size, type, flags, ct);
}

static int
__build_query_ct(struct nfnl_subsys_handle *ssh,
		 const enum nf_conntrack_query qt,
		 const void *data, void *buffer, unsigned int size)
{
	struct nfnlhdr *req = buffer;
	const uint32_t *family = data;

	assert(ssh != NULL);
	assert(data != NULL);
	assert(req != NULL);

	memset(req, 0, size);

	switch(qt) {
	case NFCT_Q_CREATE:
		__build_conntrack(ssh, req, size, IPCTNL_MSG_CT_NEW, NLM_F_REQUEST|NLM_F_CREATE|NLM_F_ACK|NLM_F_EXCL, data);
		break;
	case NFCT_Q_UPDATE:
		__build_conntrack(ssh, req, size, IPCTNL_MSG_CT_NEW, NLM_F_REQUEST|NLM_F_ACK, data);
		break;
	case NFCT_Q_DESTROY:
		__build_conntrack(ssh, req, size, IPCTNL_MSG_CT_DELETE, NLM_F_REQUEST|NLM_F_ACK, data);
		break;
	case NFCT_Q_GET:
		__build_conntrack(ssh, req, size, IPCTNL_MSG_CT_GET, NLM_F_REQUEST|NLM_F_ACK, data);
		break;
	case NFCT_Q_FLUSH:
		nfnl_fill_hdr(ssh, &req->nlh, 0, *family, 0, IPCTNL_MSG_CT_DELETE, NLM_F_REQUEST|NLM_F_ACK);
		break;
	case NFCT_Q_DUMP:
		nfnl_fill_hdr(ssh, &req->nlh, 0, *family, 0, IPCTNL_MSG_CT_GET, NLM_F_REQUEST|NLM_F_DUMP);
		break;
	case NFCT_Q_DUMP_RESET:
		nfnl_fill_hdr(ssh, &req->nlh, 0, *family, 0, IPCTNL_MSG_CT_GET_CTRZERO, NLM_F_REQUEST|NLM_F_DUMP);
		break;
	case NFCT_Q_CREATE_UPDATE:
		__build_conntrack(ssh, req, size, IPCTNL_MSG_CT_NEW, NLM_F_REQUEST|NLM_F_CREATE|NLM_F_ACK, data);
		break;
	case NFCT_Q_DUMP_FILTER:
		nfnl_fill_hdr(ssh, &req->nlh, 0, AF_UNSPEC, 0, IPCTNL_MSG_CT_GET, NLM_F_REQUEST|NLM_F_DUMP);
		__build_filter_dump(req, size, data);
		break;
	case NFCT_Q_DUMP_FILTER_RESET:
		nfnl_fill_hdr(ssh, &req->nlh, 0, AF_UNSPEC, 0, IPCTNL_MSG_CT_GET_CTRZERO, NLM_F_REQUEST|NLM_F_DUMP);
		__build_filter_dump(req, size, data);
		break;
	default:
		errno = ENOTSUP;
		return -1;
	}
	return 1;
}

/**
 * nfct_build_query - build a query in netlink message format for ctnetlink
 * \param ssh nfnetlink subsystem handler
 * \param qt query type
 * \param data data required to build the query
 * \param req buffer to build the netlink message
 * \param size size of the buffer passed
 *
 * This is a low level function, use it if you want to require to work
 * with netlink details via libnfnetlink, otherwise we suggest you to
 * use nfct_query.
 *
 * The pointer to data can be a conntrack object or the protocol family
 * depending on the request.
 *
 * For query types:
 * 	- NFCT_Q_CREATE: add a new conntrack, if it exists, fail
 * 	- NFCT_O_CREATE_UPDATE: add a new conntrack, if it exists, update it
 * 	- NFCT_Q_UPDATE: update a conntrack
 * 	- NFCT_Q_DESTROY: destroy a conntrack
 * 	- NFCT_Q_GET: get a conntrack
 *
 * Pass a valid pointer to a conntrack object.
 *
 * For query types:
 * 	- NFCT_Q_FLUSH: flush the conntrack table
 * 	- NFCT_Q_DUMP: dump the conntrack table
 * 	- NFCT_Q_DUMP_RESET: dump the conntrack table and reset counters
 * 	- NFCT_Q_DUMP_FILTER: dump the conntrack table
 * 	- NFCT_Q_DUMP_FILTER_RESET: dump the conntrack table and reset counters
 *
 * Pass a valid pointer to the protocol family (uint32_t)
 *
 * On success, 0 is returned. On error, -1 is returned and errno is set
 * appropiately.
 */
int nfct_build_query(struct nfnl_subsys_handle *ssh,
		     const enum nf_conntrack_query qt,
		     const void *data,
		     void *buffer,
		     unsigned int size)
{
	return __build_query_ct(ssh, qt, data, buffer, size);
}

/**
 * nfct_parse_conntrack - translate a netlink message to a conntrack object
 * \param type do the translation iif the message type is of a certain type
 * \param nlh pointer to the netlink message
 * \param ct pointer to the conntrack object
 *
 * This is a low level function, use it in case that you require to work
 * with netlink details via libnfnetlink. Otherwise, we suggest you to
 * use the high level API.
 *
 * The message types are:
 *
 * - NFCT_T_NEW: parse messages with new conntracks
 * - NFCT_T_UPDATE: parse messages with conntrack updates
 * - NFCT_T_DESTROY: parse messages with conntrack destroy
 * - NFCT_T_ALL: all message types
 *
 * The message type is a flag, therefore the can be combined, ie.
 * NFCT_T_NEW | NFCT_T_DESTROY to parse only new and destroy messages
 *
 * On error, NFCT_T_ERROR is returned and errno is set appropiately. If 
 * the message received is not of the requested type then 0 is returned, 
 * otherwise this function returns the message type parsed.
 */
int nfct_parse_conntrack(enum nf_conntrack_msg_type type,
			 const struct nlmsghdr *nlh,
			 struct nf_conntrack *ct)
{
	unsigned int flags;
	int len = nlh->nlmsg_len;
	struct nfgenmsg *nfhdr = NLMSG_DATA(nlh);
	struct nfattr *cda[CTA_MAX];

	assert(nlh != NULL);
	assert(ct != NULL);

	len -= NLMSG_LENGTH(sizeof(struct nfgenmsg));
	if (len < 0) {
		errno = EINVAL;
		return NFCT_T_ERROR;
	}

	flags = __parse_message_type(nlh);
	if (!(flags & type))
		return 0;

	nfnl_parse_attr(cda, CTA_MAX, NFA_DATA(nfhdr), len);

	__parse_conntrack(nlh, cda, ct);

	return flags;
}

/**
 * @}
 */

/**
 * \defgroup cmd Send commands to kernel-space and receive replies
 * @{
 */

/**
 * nfct_query - send a query to ctnetlink and handle the reply
 * \param h library handler
 * \param qt query type
 * \param data data required to send the query
 *
 * On error, -1 is returned and errno is explicitely set. On success, 0
 * is returned.
 */
int nfct_query(struct nfct_handle *h,
	       const enum nf_conntrack_query qt,
	       const void *data)
{
	const size_t size = 4096;	/* enough for now */
	union {
		char buffer[size];
		struct nfnlhdr req;
	} u;

	assert(h != NULL);
	assert(data != NULL);

	if (__build_query_ct(h->nfnlssh_ct, qt, data, &u.req, size) == -1)
		return -1;

	return nfnl_query(h->nfnlh, &u.req.nlh);
}

/**
 * nfct_send - send a query to ctnetlink
 * \param h library handler
 * \param qt query type
 * \param data data required to send the query
 *
 * Like nfct_query but we do not wait for the reply from ctnetlink. 
 * You can use nfct_send() and nfct_catch() to emulate nfct_query().
 * This is particularly useful when the socket is non-blocking.
 *
 * On error, -1 is returned and errno is explicitely set. On success, 0
 * is returned.
 */
int nfct_send(struct nfct_handle *h,
	      const enum nf_conntrack_query qt,
	      const void *data)
{
	const size_t size = 4096;	/* enough for now */
	union {
		char buffer[size];
		struct nfnlhdr req;
	} u;

	assert(h != NULL);
	assert(data != NULL);

	if (__build_query_ct(h->nfnlssh_ct, qt, data, &u.req, size) == -1)
		return -1;

	return nfnl_send(h->nfnlh, &u.req.nlh);
}


/**
 * nfct_catch - catch events
 * \param h library handler
 *
 * This function receives the event from the kernel and it invokes the
 * callback that was registered to this handle.
 *
 * On error, -1 is returned and errno is set appropiately. On success,
 * a value greater or equal to 0 is returned indicating the callback
 * verdict: NFCT_CB_STOP, NFCT_CB_CONTINUE or NFCT_CB_STOLEN.
 *
 * Beware that this function also handles expectation events, in case they are
 * received through this handle.
 */
int nfct_catch(struct nfct_handle *h)
{
	assert(h != NULL);

	return nfnl_catch(h->nfnlh);
}

/**
 * @}
 */

/**
 * \defgroup ct Conntrack object handling
 * @{
 */

/**
 * nfct_snprintf - print a conntrack object to a buffer
 * \param buf buffer used to build the printable conntrack
 * \param size size of the buffer
 * \param ct pointer to a valid conntrack object
 * \param message_type print message type (NFCT_T_UNKNOWN, NFCT_T_NEW,...)
 * \param output_type print type (NFCT_O_DEFAULT, NFCT_O_XML, ...)
 * \param flags extra flags for the output type (NFCT_OF_LAYER3)
 *
 * If you are listening to events, probably you want to display the message 
 * type as well. In that case, set the message type parameter to any of the
 * known existing types, ie. NFCT_T_NEW, NFCT_T_UPDATE, NFCT_T_DESTROY.
 * If you pass NFCT_T_UNKNOWN, the message type will not be output. 
 *
 * Currently, the output available are:
 * 	- NFCT_O_DEFAULT: default /proc-like output
 * 	- NFCT_O_XML: XML output
 *
 * The output flags are:
 * 	- NFCT_OF_SHOW_LAYER3: include layer 3 information in the output, 
 * 	this is *only* required by NFCT_O_DEFAULT.
 * 	- NFCT_OF_TIME: display current time.
 * 	- NFCT_OF_ID: display the ID number.
 * 	- NFCT_OF_TIMESTAMP: display creation and (if exists) deletion time.
 *
 * To use NFCT_OF_TIMESTAMP, you have to:
 * \verbatim
 *  $ echo 1 > /proc/sys/net/netfilter/nf_conntrack_timestamp
\endverbatim
 * This requires a Linux kernel >= 2.6.38.
 *
 * Note that NFCT_OF_TIME displays the current time when nfct_snprintf() has
 * been called. Thus, it can be used to know when a flow was destroy if you
 * print the message just after you receive the destroy event. If you want
 * more accurate timestamping, use NFCT_OF_TIMESTAMP.
 *
 * This function returns the size of the information that _would_ have been 
 * written to the buffer, even if there was no room for it. Thus, the
 * behaviour is similar to snprintf.
 */
int nfct_snprintf(char *buf,
		  unsigned int size,
		  const struct nf_conntrack *ct,
		  unsigned int msg_type,
		  unsigned int out_type,
		  unsigned int flags)
{
	assert(buf != NULL);
	assert(size > 0);
	assert(ct != NULL);

	return __snprintf_conntrack(buf, size, ct, msg_type, out_type, flags, NULL);
}

/**
 * nfct_snprintf_labels - print a bitmask object to a buffer including labels
 * \param buf buffer used to build the printable conntrack
 * \param size size of the buffer
 * \param ct pointer to a valid conntrack object
 * \param message_type print message type (NFCT_T_UNKNOWN, NFCT_T_NEW,...)
 * \param output_type print type (NFCT_O_DEFAULT, NFCT_O_XML, ...)
 * \param flags extra flags for the output type (NFCT_OF_LAYER3)
 * \param map nfct_labelmap describing the connlabel translation, or NULL.
 *
 * When map is NULL, the function is equal to nfct_snprintf().
 * Otherwise, if the conntrack object has a connlabel attribute, the active
 * labels are translated using the label map and added to the buffer.
 */
int nfct_snprintf_labels(char *buf,
			 unsigned int size,
			 const struct nf_conntrack *ct,
			 unsigned int msg_type,
			 unsigned int out_type,
			 unsigned int flags,
			 struct nfct_labelmap *map)
{
	return __snprintf_conntrack(buf, size, ct, msg_type, out_type, flags, map);
}

/**
 * nfct_compare - compare two conntrack objects
 * \param ct1 pointer to a valid conntrack object
 * \param ct2 pointer to a valid conntrack object
 *
 * This function only compare attribute set in both objects, ie. if a certain
 * attribute is not set in ct1 but it is in ct2, then the value of such 
 * attribute is not used in the comparison.
 *
 * If both conntrack object are equal, this function returns 1, otherwise
 * 0 is returned.
 *
 * NOTICE: The use nfct_cmp is preferred.
 */
int nfct_compare(const struct nf_conntrack *ct1, 
		 const struct nf_conntrack *ct2)
{
	assert(ct1 != NULL);
	assert(ct2 != NULL);

	return __compare(ct1, ct2, NFCT_CMP_ALL);
}

/**
 * nfct_cmp - compare two conntrack objects
 * \param ct1 pointer to a valid conntrack object
 * \param ct2 pointer to a valid conntrack object
 * \param flags flags
 *
 * This function only compare attribute set in both objects, by default 
 * the comparison is not strict, ie. if a certain attribute is not set in one
 * of the objects, then such attribute is not used in the comparison.
 * If you want more strict comparisons, you can use the appropriate flags
 * to modify this behaviour (see NFCT_CMP_STRICT and NFCT_CMP_MASK).
 *
 * The available flags are:
 *
 * 	- NFCT_CMP_STRICT: the compared objects must have the same attributes
 * 	and the same values, otherwise it returns that the objects are 
 * 	different.
 * 	- NFCT_CMP_MASK: the first object is used as mask, this means that 
 * 	if an attribute is present in ct1 but not in ct2, this function 
 * 	returns that the objects are different.
 * 	- NFCT_CMP_ALL: full comparison of both objects
 * 	- NFCT_CMP_ORIG: it only compares the source and destination address;
 * 	source and destination ports; the layer 3 and 4 protocol numbers
 * 	of the original direction; and the id (if present).
 * 	- NFCT_CMP_REPL: like NFCT_CMP_REPL but it compares the flow
 * 	information that goes in the reply direction.
 * 	- NFCT_CMP_TIMEOUT_EQ: timeout(ct1) == timeout(ct2)
 * 	- NFCT_CMP_TIMEOUT_GT: timeout(ct1) > timeout(ct2)
 * 	- NFCT_CMP_TIMEOUT_LT: timeout(ct1) < timeout(ct2)
 * 	- NFCT_CMP_TIMEOUT_GE: timeout(ct1) >= timeout(ct2)
 * 	- NFCT_CMP_TIMEOUT_LE: timeout(ct1) <= timeout(ct2)
 *
 * The status bits comparison is status(ct1) & status(ct2) == status(ct1).
 *
 * If both conntrack object are equal, this function returns 1, otherwise
 * 0 is returned.
 */
int nfct_cmp(const struct nf_conntrack *ct1, 
	     const struct nf_conntrack *ct2,
	     unsigned int flags)
{
	assert(ct1 != NULL);
	assert(ct2 != NULL);

	return __compare(ct1, ct2, flags);
}

/**
 * nfct_copy - copy part of one source object to another
 * \param ct1 destination object
 * \param ct2 source object
 * \param flags flags
 *
 * This function copies one part of the source object to the target.
 * It behaves like clone but:
 *
 * 1) You have to pass an already allocated space for the target object
 * 2) You can copy only a part of the source object to the target
 *
 * The current supported flags are:
 * 	- NFCT_CP_ALL: that copies the object entirely.
 * 	- NFCT_CP_ORIG and NFCT_CP_REPL: that can be used to copy the
 * 	information that identifies a flow in the original and the reply
 * 	direction. This information is usually composed of: source and
 * 	destination IP address; source and destination ports; layer 3
 * 	and 4 protocol number.
 * 	- NFCT_CP_META: that copies the metainformation 
 * 	(all the attributes >= ATTR_TCP_STATE)
 *	- NFCT_CP_OVERRIDE: changes the default behaviour of nfct_copy() since
 *	it overrides the destination object. After the copy, the destination
 *	is a clone of the origin. This flag provides faster copying.
 */
void nfct_copy(struct nf_conntrack *ct1,
	       const struct nf_conntrack *ct2,
	       unsigned int flags)
{
	int i;

	assert(ct1 != NULL);
	assert(ct2 != NULL);

	if (flags & NFCT_CP_OVERRIDE) {
		__copy_fast(ct1, ct2);
		return;
	}
	if (flags == NFCT_CP_ALL) {
		for (i=0; i<ATTR_MAX; i++) {
			if (test_bit(i, ct2->head.set)) {
				assert(copy_attr_array[i]);
				copy_attr_array[i](ct1, ct2);
				set_bit(i, ct1->head.set);
			}
		}
		return;
	}

	static const int cp_orig_mask[] = {
		ATTR_ORIG_IPV4_SRC,
		ATTR_ORIG_IPV4_DST,
		ATTR_ORIG_IPV6_SRC,
		ATTR_ORIG_IPV6_DST,
		ATTR_ORIG_PORT_SRC,
		ATTR_ORIG_PORT_DST,
		ATTR_ICMP_TYPE,
		ATTR_ICMP_CODE,
		ATTR_ICMP_ID,
		ATTR_ORIG_L3PROTO,
		ATTR_ORIG_L4PROTO,
	};
	#define __CP_ORIG_MAX sizeof(cp_orig_mask)/sizeof(int)

	if (flags & NFCT_CP_ORIG) {
		for (i=0; i<__CP_ORIG_MAX; i++) {
			if (test_bit(cp_orig_mask[i], ct2->head.set)) {
				assert(copy_attr_array[i]);
				copy_attr_array[cp_orig_mask[i]](ct1, ct2);
				set_bit(cp_orig_mask[i], ct1->head.set);
			}
		}
	}

	static const int cp_repl_mask[] = {
		ATTR_REPL_IPV4_SRC,
		ATTR_REPL_IPV4_DST,
		ATTR_REPL_IPV6_SRC,
		ATTR_REPL_IPV6_DST,
		ATTR_REPL_PORT_SRC,
		ATTR_REPL_PORT_DST,
		ATTR_REPL_L3PROTO,
		ATTR_REPL_L4PROTO,
	};
	#define __CP_REPL_MAX sizeof(cp_repl_mask)/sizeof(int)

	if (flags & NFCT_CP_REPL) {
		for (i=0; i<__CP_REPL_MAX; i++) {
			if (test_bit(cp_repl_mask[i], ct2->head.set)) {
				assert(copy_attr_array[i]);
				copy_attr_array[cp_repl_mask[i]](ct1, ct2);
				set_bit(cp_repl_mask[i], ct1->head.set);
			}
		}
	}

	if (flags & NFCT_CP_META) {
		for (i=ATTR_TCP_STATE; i<ATTR_MAX; i++) {
			if (test_bit(i, ct2->head.set)) {
				assert(copy_attr_array[i]),
				copy_attr_array[i](ct1, ct2);
				set_bit(i, ct1->head.set);
			}
		}
	}
}

/**
 * nfct_copy_attr - copy an attribute of one source object to another
 * \param ct1 destination object
 * \param ct2 source object
 * \param flags flags
 *
 * This function copies one attribute (if present) to another object.
 */
void nfct_copy_attr(struct nf_conntrack *ct1,
		    const struct nf_conntrack *ct2,
		    const enum nf_conntrack_attr type)
{
	if (test_bit(type, ct2->head.set)) {
		assert(copy_attr_array[type]);
		copy_attr_array[type](ct1, ct2);
		set_bit(type, ct1->head.set);
	}
}

/**
 * @}
 */

/**
 * \defgroup bsf Kernel-space filtering for events
 *
 * @{
 */

/**
 * nfct_filter_create - create a filter
 *
 * This function returns a valid pointer on success, otherwise NULL is
 * returned and errno is appropriately set.
 */
struct nfct_filter *nfct_filter_create(void)
{
	return calloc(sizeof(struct nfct_filter), 1);
}

/**
 * nfct_filter_destroy - destroy a filter
 * \param filter filter that we want to destroy
 *
 * This function releases the memory that is used by the filter object. 
 * However, please note that this function does *not* detach an already
 * attached filter.
 */
void nfct_filter_destroy(struct nfct_filter *filter)
{
	assert(filter != NULL);
	free(filter);
	filter = NULL;
}

/**
 * nfct_filter_add_attr - add a filter attribute of the filter object
 * \param filter filter object that we want to modify
 * \param type filter attribute type
 * \param value pointer to the value of the filter attribute
 *
 * Limitations: You can add up to 127 IPv4 addresses and masks for 
 * NFCT_FILTER_SRC_IPV4 and, similarly, 127 for NFCT_FILTER_DST_IPV4.
 */
void nfct_filter_add_attr(struct nfct_filter *filter,
			  const enum nfct_filter_attr type, 
			  const void *value)
{
	assert(filter != NULL);
	assert(value != NULL);

	if (unlikely(type >= NFCT_FILTER_MAX))
		return;

	if (filter_attr_array[type]) {
		filter_attr_array[type](filter, value);
		set_bit(type, filter->set);
	}
}

/**
 * nfct_filter_add_attr_u32 - add an u32 filter attribute of the filter object
 * \param filter filter object that we want to modify
 * \param type filter attribute type
 * \param value value of the filter attribute using unsigned int (32 bits).
 *
 * Limitations: You can add up to 255 protocols which is a reasonable limit.
 */
void nfct_filter_add_attr_u32(struct nfct_filter *filter,
			      const enum nfct_filter_attr type,
			      uint32_t value)
{
	nfct_filter_add_attr(filter, type, &value);
}

/**
 * nfct_filter_set_logic - set the filter logic for an attribute type
 * \param filter filter object that we want to modify
 * \param type filter attribute type
 * \param logic filter logic that we want to use
 *
 * You can only use this function once to set the filtering logic for 
 * one attribute. You can define two logics: NFCT_FILTER_LOGIC_POSITIVE
 * that accept events that match the filter, and NFCT_FILTER_LOGIC_NEGATIVE
 * that rejects events that match the filter. Default filtering logic is
 * NFCT_FILTER_LOGIC_POSITIVE.
 *
 * On error, it returns -1 and errno is appropriately set. On success, it 
 * returns 0.
 */
int nfct_filter_set_logic(struct nfct_filter *filter,
			  const enum nfct_filter_attr type,
			  const enum nfct_filter_logic logic)
{
	if (unlikely(type >= NFCT_FILTER_MAX)) {
		errno = ENOTSUP;
                return -1;
	}

	if (filter->logic[type]) {
		errno = EBUSY;
		return -1;
	}

	filter->logic[type] = logic;

	return 0;
}

/**
 * nfct_filter_attach - attach a filter to a socket descriptor
 * \param fd socket descriptor
 * \param filter filter that we want to attach to the socket
 *
 * This function returns -1 on error and set errno appropriately. If the
 * function returns EINVAL probably you have found a bug in it. Please,
 * report this.
 */
int nfct_filter_attach(int fd, struct nfct_filter *filter)
{
	assert(filter != NULL);

	return __setup_netlink_socket_filter(fd, filter);
}

/**
 * nfct_filter_detach - detach an existing filter
 * \param fd socket descriptor
 *
 * This function returns -1 on error and set errno appropriately.
 */
int nfct_filter_detach(int fd)
{
	int val = 0;

	return setsockopt(fd, SOL_SOCKET, SO_DETACH_FILTER, &val, sizeof(val));
}

/**
 * @}
 */

/**
 * \defgroup dumpfilter Kernel-space filtering for dumping
 *
 * @{
 */

/**
 * nfct_filter_dump_create - create a dump filter
 *
 * This function returns a valid pointer on success, otherwise NULL is
 * returned and errno is appropriately set.
 */
struct nfct_filter_dump *nfct_filter_dump_create(void)
{
	return calloc(sizeof(struct nfct_filter_dump), 1);
}

/**
 * nfct_filter_dump_destroy - destroy a dump filter
 * \param filter filter that we want to destroy
 *
 * This function releases the memory that is used by the filter object.
 */
void nfct_filter_dump_destroy(struct nfct_filter_dump *filter)
{
	assert(filter != NULL);
	free(filter);
	filter = NULL;
}

/**
 * nfct_filter_dump_attr_set - set filter attribute
 * \param filter dump filter object that we want to modify
 * \param type filter attribute type
 * \param value pointer to the value of the filter attribute
 */
void nfct_filter_dump_set_attr(struct nfct_filter_dump *filter_dump,
			       const enum nfct_filter_dump_attr type,
			       const void *value)
{
	assert(filter_dump != NULL);
	assert(value != NULL);

	if (unlikely(type >= NFCT_FILTER_DUMP_MAX))
		return;

	if (set_filter_dump_attr_array[type]) {
		set_filter_dump_attr_array[type](filter_dump, value);
		filter_dump->set |= (1 << type);
	}
}

/**
 * nfct_filter_dump_attr_set_u8 - set u8 dump filter attribute
 * \param filter dump filter object that we want to modify
 * \param type filter attribute type
 * \param value value of the filter attribute using unsigned int (32 bits).
 */
void nfct_filter_dump_set_attr_u8(struct nfct_filter_dump *filter_dump,
				  const enum nfct_filter_dump_attr type,
				  uint8_t value)
{
	nfct_filter_dump_set_attr(filter_dump, type, &value);
}

/**
 * @}
 */

/**
 * \defgroup label Conntrack labels
 *
 * @{
 */

/**
 * nfct_labels_get_path - get name of default config path
 *
 * returns a pointer to a immutable (static) string containing
 * the default connlabel.conf file location.
 */
const char *nfct_labels_get_path(void)
{
	return __labels_get_path();
}

/**
 * nfct_labelmap_get_name - get name of the label bit
 *
 * \param m label map obtained from nfct_label_open
 * \param bit whose name should be returned
 *
 * returns a pointer to the name associated with the label.
 * If no name has been configured, the empty string is returned.
 * If bit is out of range, NULL is returned.
 */
const char *nfct_labelmap_get_name(struct nfct_labelmap *m, unsigned int bit)
{
	return __labelmap_get_name(m, bit);
}

/**
 * nfct_labelmap_get_bit - get bit associated with the name
 *
 * \param h label handle obtained from nfct_labelmap_new
 * \param name name of the label
 *
 * returns the bit associated with the name, or negative value on error.
 */
int nfct_labelmap_get_bit(struct nfct_labelmap *m, const char *name)
{
	return __labelmap_get_bit(m, name);
}

/**
 * nfct_labelmap_new - create a new label map
 *
 * \param mapfile the file containing the bit <-> name mapping
 *
 * If mapfile is NULL, the default mapping file is used.
 * returns a new label map, or NULL on error.
 */
struct nfct_labelmap *nfct_labelmap_new(const char *mapfile)
{
	return __labelmap_new(mapfile);
}

/**
 * nfct_labelmap_destroy - destroy nfct_labelmap object
 *
 * \param map the label object to destroy.
 *
 * This function releases the memory that is used by the labelmap object.
 */
void nfct_labelmap_destroy(struct nfct_labelmap *map)
{
	__labelmap_destroy(map);
}

/**
 * @}
 */

/*
 * \defgroup bitmask bitmask object
 *
 * @{
 */

/**
 * nfct_bitmask_new - allocate a new bitmask
 *
 * \param max highest valid bit that can be set/unset.
 *
 * In case of success, this function returns a valid pointer to a memory blob,
 * otherwise NULL is returned and errno is set appropiately.
 */
struct nfct_bitmask *nfct_bitmask_new(unsigned int max)
{
	struct nfct_bitmask *b;
	unsigned int bytes, words;

	if (max > 0xffff)
		return NULL;

	words = DIV_ROUND_UP(max+1, 32);
	bytes = words * sizeof(b->bits[0]);

	b = malloc(sizeof(*b) + bytes);
	if (b) {
		memset(b->bits, 0, bytes);
		b->words = words;
	}
	return b;
}

/*
 * nfct_bitmask_clone - duplicate a bitmask object
 *
 * \param b pointer to the bitmask object to duplicate
 *
 * returns an identical copy of the bitmask.
 */
struct nfct_bitmask *nfct_bitmask_clone(const struct nfct_bitmask *b)
{
	unsigned int bytes = b->words * sizeof(b->bits[0]);
	struct nfct_bitmask *copy;

	bytes += sizeof(*b);

	copy = malloc(bytes);
	if (copy)
		memcpy(copy, b, bytes);
	return copy;
}

/*
 * nfct_bitmask_set_bit - set bit in the bitmask
 *
 * \param b pointer to the bitmask object
 * \param bit the bit to set
 */
void nfct_bitmask_set_bit(struct nfct_bitmask *b, unsigned int bit)
{
	unsigned int bits = b->words * 32;
	if (bit < bits)
		set_bit(bit, b->bits);
}

/*
 * nfct_bitmask_test_bit - test if a bit in the bitmask is set
 *
 * \param b pointer to the bitmask object
 * \param bit the bit to test
 *
 * returns 0 if the bit is not set.
 */
int nfct_bitmask_test_bit(const struct nfct_bitmask *b, unsigned int bit)
{
	unsigned int bits = b->words * 32;
	return bit < bits && test_bit(bit, b->bits);
}

/*
 * nfct_bitmask_unset_bit - unset bit in the bitmask
 *
 * \param b pointer to the bitmask object
 * \param bit the bit to clear
 */
void nfct_bitmask_unset_bit(struct nfct_bitmask *b, unsigned int bit)
{
	unsigned int bits = b->words * 32;
	if (bit < bits)
		unset_bit(bit, b->bits);
}

/*
 * nfct_bitmask_maxbit - return highest bit that may be set/unset
 *
 * \param b pointer to the bitmask object
 */
unsigned int nfct_bitmask_maxbit(const struct nfct_bitmask *b)
{
	return (b->words * 32) - 1;
}

/*
 * nfct_bitmask_destroy - destroy bitmask object
 *
 * \param b pointer to the bitmask object
 *
 * This function releases the memory that is used by the bitmask object.
 *
 * If you assign a bitmask object to a nf_conntrack object using
 * nfct_set_attr ATTR_CONNLABEL, then the ownership of the bitmask
 * object passes on to the nf_conntrack object. The nfct_bitmask object
 * will be destroyed when the nf_conntrack object is destroyed.
 */
void nfct_bitmask_destroy(struct nfct_bitmask *b)
{
	free(b);
}

/*
 * nfct_bitmask_clear - clear a bitmask object
 *
 * \param b pointer to the bitmask object to clear
 */
void nfct_bitmask_clear(struct nfct_bitmask *b)
{
	unsigned int bytes = b->words * sizeof(b->bits[0]);
	memset(b->bits, 0, bytes);
}

/*
 * nfct_bitmask_equal - compare two bitmask objects
 *
 * \param b1 pointer to a valid bitmask object
 * \param b2 pointer to a valid bitmask object
 *
 * If both bitmask object are equal, this function returns true, otherwise
 * false is returned.
 */
bool nfct_bitmask_equal(const struct nfct_bitmask *b1, const struct nfct_bitmask *b2)
{
	if (b1->words != b2->words)
		return false;

	return memcmp(b1->bits, b2->bits, b1->words * sizeof(b1->bits[0])) == 0;
}

/**
 * @}
 */
