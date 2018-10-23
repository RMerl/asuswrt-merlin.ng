/*
 * (C) 2005-2011 by Pablo Neira Ayuso <pablo@netfilter.org>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <stdlib.h>
#include <string.h> /* for memset */
#include <errno.h>
#include <assert.h>

#include "internal/internal.h"

/**
 * \defgroup exp Expect object handling
 * @{
 */

/**
 * nfexp_new - allocate a new expectation
 *
 * In case of success, this function returns a valid pointer to a memory blob,
 * otherwise NULL is returned and errno is set appropiately.
 */
struct nf_expect *nfexp_new(void)
{
	struct nf_expect *exp;

	exp = malloc(sizeof(struct nf_expect));
	if (!exp)
		return NULL;

	memset(exp, 0, sizeof(struct nf_expect));

	return exp;
}

/**
 * nfexp_destroy - release an expectation object
 * \param exp pointer to the expectation object
 */
void nfexp_destroy(struct nf_expect *exp)
{
	assert(exp != NULL);
	free(exp);
	exp = NULL; /* bugtrap */
}

/**
 * nfexp_sizeof - return the size in bytes of a certain expect object
 * \param exp pointer to the expect object
 */
size_t nfexp_sizeof(const struct nf_expect *exp)
{
	assert(exp != NULL);
	return sizeof(*exp);
}

/**
 * nfexp_maxsize - return the maximum size in bytes of a expect object
 *
 * Use this function if you want to allocate a expect object in the stack
 * instead of the heap. For example:
 * 
 * char buf[nfexp_maxsize()];
 * struct nf_expect *exp = (struct nf_expect *) buf;
 * memset(exp, 0, nfexp_maxsize());
 * 
 * Note: As for now this function returns the same size that nfexp_sizeof(exp)
 * does although _this could change in the future_. Therefore, do not assume
 * that nfexp_sizeof(exp) == nfexp_maxsize().
 */
size_t nfexp_maxsize(void)
{
	return sizeof(struct nf_expect);
}

/**
 * nfexp_clone - clone a expectation object
 * \param exp pointer to a valid expectation object
 *
 * On error, NULL is returned and errno is appropiately set. Otherwise,
 * a valid pointer to the clone expect is returned.
 */
struct nf_expect *nfexp_clone(const struct nf_expect *exp)
{
	struct nf_expect *clone;

	assert(exp != NULL);

	if ((clone = nfexp_new()) == NULL)
		return NULL;
	memcpy(clone, exp, sizeof(*exp));

	return clone;
}

/**
 * nfexp_cmp - compare two expectation objects
 * \param exp1 pointer to a valid expectation object
 * \param exp2 pointer to a valid expectation object
 * \param flags flags
 *
 * This function only compare attribute set in both objects, by default
 * the comparison is not strict, ie. if a certain attribute is not set in one
 * of the objects, then such attribute is not used in the comparison.
 * If you want more strict comparisons, you can use the appropriate flags
 * to modify this behaviour (see NFCT_CMP_STRICT and NFCT_CMP_MASK).
 *
 * The available flags are:
 *      - NFCT_CMP_STRICT: the compared objects must have the same attributes
 *      and the same values, otherwise it returns that the objects are
 *      different.
 *      - NFCT_CMP_MASK: the first object is used as mask, this means that
 *      if an attribute is present in exp1 but not in exp2, this function
 *      returns that the objects are different.
 *
 * Other existing flags that are used by nfct_cmp() are ignored.
 *
 * If both conntrack object are equal, this function returns 1, otherwise
 * 0 is returned.
 */
int nfexp_cmp(const struct nf_expect *exp1, const struct nf_expect *exp2,
	      unsigned int flags)
{
        assert(exp1 != NULL);
        assert(exp2 != NULL);

        return __cmp_expect(exp1, exp2, flags);
}

/**
 * @}
 */

/**
 * \defgroup LibrarySetup Library setup
 * @{
 */

/**
 * nfexp_callback_register - register a callback
 * \param h library handler
 * \param cb callback used to process expect received
 * \param data data used by the callback, if any.
 *
 * This function register a callback to handle the expect received, 
 * in case of error -1 is returned and errno is set appropiately, otherwise
 * 0 is returned.
 * 
 * Note that the data parameter is optional, if you do not want to pass any
 * data to your callback, then use NULL.
 */
int nfexp_callback_register(struct nfct_handle *h,
			    enum nf_conntrack_msg_type type,
			    int (*cb)(enum nf_conntrack_msg_type type,
			   	      struct nf_expect *exp, 
				      void *data),
			   void *data)
{
	struct __data_container *container;

	assert(h != NULL);

	container = malloc(sizeof(struct __data_container));
	if (!container)
		return -1;
	memset(container, 0, sizeof(struct __data_container));

	h->expect_cb = cb;
	container->h = h;
	container->type = type;
	container->data = data;

	h->nfnl_cb_exp.call = __callback;
	h->nfnl_cb_exp.data = container;
	h->nfnl_cb_exp.attr_count = CTA_EXPECT_MAX;

	nfnl_callback_register(h->nfnlssh_exp, 
			       IPCTNL_MSG_EXP_NEW,
			       &h->nfnl_cb_exp);

	nfnl_callback_register(h->nfnlssh_exp,
			       IPCTNL_MSG_EXP_DELETE,
			       &h->nfnl_cb_exp);

	return 0;
}

/**
 * nfexp_callback_unregister - unregister a callback
 * \param h library handler
 */
void nfexp_callback_unregister(struct nfct_handle *h)
{
	assert(h != NULL);

	nfnl_callback_unregister(h->nfnlssh_exp, IPCTNL_MSG_EXP_NEW);
	nfnl_callback_unregister(h->nfnlssh_exp, IPCTNL_MSG_EXP_DELETE);

	h->expect_cb = NULL;
	free(h->nfnl_cb_exp.data);

	h->nfnl_cb_exp.call = NULL;
	h->nfnl_cb_exp.data = NULL;
	h->nfnl_cb_exp.attr_count = 0;
}

/**
 * nfexp_callback_register2 - register a callback
 * \param h library handler
 * \param cb callback used to process expect received
 * \param data data used by the callback, if any.
 *
 * This function register a callback to handle the expect received, 
 * in case of error -1 is returned and errno is set appropiately, otherwise
 * 0 is returned.
 * 
 * Note that the data parameter is optional, if you do not want to pass any
 * data to your callback, then use NULL.
 * 
 * NOTICE: The difference with nfexp_callback_register() is that this function
 * uses the new callback interface that includes the Netlink header.
 * 
 * WARNING: Don't mix nfexp_callback_register() and nfexp_callback_register2()
 * calls, use only once at a time.
 */
int nfexp_callback_register2(struct nfct_handle *h,
			     enum nf_conntrack_msg_type type,
			     int (*cb)(const struct nlmsghdr *nlh,
			     	       enum nf_conntrack_msg_type type,
			   	       struct nf_expect *exp, 
				       void *data),
			     void *data)
{
	struct __data_container *container;

	assert(h != NULL);

	container = malloc(sizeof(struct __data_container));
	if (!container)
		return -1;
	memset(container, 0, sizeof(struct __data_container));

	h->expect_cb2 = cb;
	container->h = h;
	container->type = type;
	container->data = data;

	h->nfnl_cb_exp.call = __callback;
	h->nfnl_cb_exp.data = container;
	h->nfnl_cb_exp.attr_count = CTA_EXPECT_MAX;

	nfnl_callback_register(h->nfnlssh_exp, 
			       IPCTNL_MSG_EXP_NEW,
			       &h->nfnl_cb_exp);

	nfnl_callback_register(h->nfnlssh_exp,
			       IPCTNL_MSG_EXP_DELETE,
			       &h->nfnl_cb_exp);

	return 0;
}

/**
 * nfexp_callback_unregister2 - unregister a callback
 * \param h library handler
 */
void nfexp_callback_unregister2(struct nfct_handle *h)
{
	assert(h != NULL);

	nfnl_callback_unregister(h->nfnlssh_exp, IPCTNL_MSG_EXP_NEW);
	nfnl_callback_unregister(h->nfnlssh_exp, IPCTNL_MSG_EXP_DELETE);

	h->expect_cb2 = NULL;
	free(h->nfnl_cb_exp.data);

	h->nfnl_cb_exp.call = NULL;
	h->nfnl_cb_exp.data = NULL;
	h->nfnl_cb_exp.attr_count = 0;
}

/**
 * @}
 */

/**
 * \defgroup exp Expect object handling
 * @{
 */

/**
 * nfexp_set_attr - set the value of a certain expect attribute
 * \param exp pointer to a valid expect
 * \param type attribute type
 * \param value pointer to the attribute value
 *
 * Note that certain attributes are unsettable:
 * 	- ATTR_EXP_USE
 * 	- ATTR_EXP_ID
 * 	- ATTR_EXP_*_COUNTER_*
 * The call of this function for such attributes do nothing.
 */
void nfexp_set_attr(struct nf_expect *exp,
		    const enum nf_expect_attr type, 
		    const void *value)
{
	assert(exp != NULL);
	assert(value != NULL);

	if (type >= ATTR_EXP_MAX)
		return;

	if (set_exp_attr_array[type]) {
		set_exp_attr_array[type](exp, value);
		set_bit(type, exp->set);
	}
}

/**
 * nfexp_set_attr_u8 - set the value of a certain expect attribute
 * \param exp pointer to a valid expect
 * \param type attribute type
 * \param value unsigned 8 bits attribute value
 */
void nfexp_set_attr_u8(struct nf_expect *exp,
		       const enum nf_expect_attr type, 
		       uint8_t value)
{
	nfexp_set_attr(exp, type, &value);
}

/**
 * nfexp_set_attr_u16 - set the value of a certain expect attribute
 * \param exp pointer to a valid expect
 * \param type attribute type
 * \param value unsigned 16 bits attribute value
 */
void nfexp_set_attr_u16(struct nf_expect *exp,
			const enum nf_expect_attr type, 
			uint16_t value)
{
	nfexp_set_attr(exp, type, &value);
}

/**
 * nfexp_set_attr_u32 - set the value of a certain expect attribute
 * \param exp pointer to a valid expect
 * \param type attribute type
 * \param value unsigned 32 bits attribute value
 */
void nfexp_set_attr_u32(struct nf_expect *exp,
			const enum nf_expect_attr type, 
			uint32_t value)
{
	nfexp_set_attr(exp, type, &value);
}

/**
 * nfexp_get_attr - get an expect attribute
 * \param exp pointer to a valid expect
 * \param type attribute type
 *
 * In case of success a valid pointer to the attribute requested is returned,
 * on error NULL is returned and errno is set appropiately.
 */
const void *nfexp_get_attr(const struct nf_expect *exp,
			   const enum nf_expect_attr type)
{
	assert(exp != NULL);

	if (type >= ATTR_EXP_MAX) {
		errno = EINVAL;
		return NULL;
	}

	if (!test_bit(type, exp->set)) {
		errno = ENODATA;
		return NULL;
	}

	return get_exp_attr_array[type](exp);
}

/**
 * nfexp_get_attr_u8 - get attribute of unsigned 8-bits long
 * \param exp pointer to a valid expectation
 * \param type attribute type
 *
 * Returns the value of the requested attribute, if the attribute is not 
 * set, 0 is returned. In order to check if the attribute is set or not,
 * use nfexp_attr_is_set.
 */
uint8_t nfexp_get_attr_u8(const struct nf_expect *exp,
			   const enum nf_expect_attr type)
{
	const uint8_t *ret = nfexp_get_attr(exp, type);
	return ret == NULL ? 0 : *ret;
}

/**
 * nfexp_get_attr_u16 - get attribute of unsigned 16-bits long
 * \param exp pointer to a valid expectation
 * \param type attribute type
 *
 * Returns the value of the requested attribute, if the attribute is not 
 * set, 0 is returned. In order to check if the attribute is set or not,
 * use nfexp_attr_is_set.
 */
uint16_t nfexp_get_attr_u16(const struct nf_expect *exp,
			     const enum nf_expect_attr type)
{
	const uint16_t *ret = nfexp_get_attr(exp, type);
	return ret == NULL ? 0 : *ret;
}

/**
 * nfexp_get_attr_u32 - get attribute of unsigned 32-bits long
 * \param exp pointer to a valid expectation
 * \param type attribute type
 *
 * Returns the value of the requested attribute, if the attribute is not 
 * set, 0 is returned. In order to check if the attribute is set or not,
 * use nfexp_attr_is_set.
 */
uint32_t nfexp_get_attr_u32(const struct nf_expect *exp,
			    const enum nf_expect_attr type)
{
	const uint32_t *ret = nfexp_get_attr(exp, type);
	return ret == NULL ? 0 : *ret;
}

/**
 * nfexp_attr_is_set - check if a certain attribute is set
 * \param exp pointer to a valid expectation object
 * \param type attribute type
 *
 * On error, -1 is returned and errno is set appropiately, otherwise
 * the value of the attribute is returned.
 */
int nfexp_attr_is_set(const struct nf_expect *exp,
		      const enum nf_expect_attr type)
{
	assert(exp != NULL);

	if (type >= ATTR_EXP_MAX) {
		errno = EINVAL;
		return -1;
	}
	return test_bit(type, exp->set);
}

/**
 * nfexp_attr_unset - unset a certain attribute
 * \param type attribute type
 * \param exp pointer to a valid expectation object
 *
 * On error, -1 is returned and errno is set appropiately, otherwise
 * 0 is returned.
 */
int nfexp_attr_unset(struct nf_expect *exp,
		     const enum nf_expect_attr type)
{
	assert(exp != NULL);

	if (type >= ATTR_EXP_MAX) {
		errno = EINVAL;
		return -1;
	}
	unset_bit(type, exp->set);

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
 * nfexp_build_expect - build a netlink message from a conntrack object
 * \param ssh nfnetlink subsystem handler
 * \param req buffer used to build the netlink message
 * \param size size of the buffer passed
 * \param type netlink message type
 * \param flags netlink flags
 * \param exp pointer to a conntrack object
 *
 * This is a low level function for those that require to be close to
 * netlink details via libnfnetlink. If you do want to obviate the netlink
 * details then we suggest you to use nfexp_query.
 * 
 * On error, -1 is returned and errno is appropiately set.
 * On success, 0 is returned.
 */
int nfexp_build_expect(struct nfnl_subsys_handle *ssh,
		       void *req,
		       size_t size,
		       uint16_t type,
		       uint16_t flags,
		       const struct nf_expect *exp)
{
	assert(ssh != NULL);
	assert(req != NULL);
	assert(exp != NULL);

	return __build_expect(ssh, req, size, type, flags, exp);
}

static int
__build_query_exp(struct nfnl_subsys_handle *ssh,
		  const enum nf_conntrack_query qt,
		  const void *data, void *buffer, unsigned int size)
{
	struct nfnlhdr *req = buffer;
	const uint8_t *family = data;

	assert(ssh != NULL);
	assert(data != NULL);
	assert(req != NULL);

	memset(req, 0, size);

	switch(qt) {
	case NFCT_Q_CREATE:
		__build_expect(ssh, req, size, IPCTNL_MSG_EXP_NEW, NLM_F_REQUEST|NLM_F_CREATE|NLM_F_ACK|NLM_F_EXCL, data);
		break;
	case NFCT_Q_CREATE_UPDATE:
		__build_expect(ssh, req, size, IPCTNL_MSG_EXP_NEW, NLM_F_REQUEST|NLM_F_CREATE|NLM_F_ACK, data);
		break;
	case NFCT_Q_GET:
		__build_expect(ssh, req, size, IPCTNL_MSG_EXP_GET, NLM_F_REQUEST|NLM_F_ACK, data);
		break;
	case NFCT_Q_DESTROY:
		__build_expect(ssh, req, size, IPCTNL_MSG_EXP_DELETE, NLM_F_REQUEST|NLM_F_ACK, data);
		break;
	case NFCT_Q_FLUSH:
		nfnl_fill_hdr(ssh, &req->nlh, 0, *family, 0, IPCTNL_MSG_EXP_DELETE, NLM_F_REQUEST|NLM_F_ACK);
		break;
	case NFCT_Q_DUMP:
		nfnl_fill_hdr(ssh, &req->nlh, 0, *family, 0, IPCTNL_MSG_EXP_GET, NLM_F_REQUEST|NLM_F_DUMP);
		break;
	default:
		errno = ENOTSUP;
		return -1;
	}
	return 1;
}

/**
 * nfexp_build_query - build a query in netlink message format for ctnetlink
 * \param ssh nfnetlink subsystem handler
 * \param qt query type
 * \param data data required to build the query
 * \param req buffer to build the netlink message
 * \param size size of the buffer passed
 *
 * This is a low level function, use it if you want to require to work
 * with netlink details via libnfnetlink, otherwise we suggest you to
 * use nfexp_query.
 * 
 * The pointer to data can be a conntrack object or the protocol family
 * depending on the request.
 * 
 * For query types:
 * 	NFEXP_Q_CREATE
 * 	NFEXP_Q_DESTROY
 * 
 * Pass a valid pointer to an expectation object.
 * 
 * For query types:
 * 	NFEXP_Q_FLUSH
 * 	NFEXP_Q_DUMP
 * 
 * Pass a valid pointer to the protocol family (uint8_t)
 * 
 * On success, 0 is returned. On error, -1 is returned and errno is set
 * appropiately.
 */
int nfexp_build_query(struct nfnl_subsys_handle *ssh,
		      const enum nf_conntrack_query qt,
		      const void *data,
		      void *buffer,
		      unsigned int size)
{
	return __build_query_exp(ssh, qt, data, buffer, size);
}

/**
 * nfexp_parse_expect - translate a netlink message to a conntrack object
 * \param type do the translation iif the message type is of a certain type
 * \param nlh pointer to the netlink message
 * \param exp pointer to the conntrack object
 *
 * This is a low level function, use it in case that you require to work
 * with netlink details via libnfnetlink. Otherwise, we suggest you to
 * use the high level API.
 * 
 * The message types are:
 * 
 * NFEXP_T_NEW: parse messages with new conntracks
 * NFEXP_T_UPDATE: parse messages with conntrack updates
 * NFEXP_T_DESTROY: parse messages with conntrack destroy 
 * NFEXP_T_ALL: all message types
 * 
 * The message type is a flag, therefore the can be combined, ie.
 * NFEXP_T_NEW | NFEXP_T_DESTROY to parse only new and destroy messages
 * 
 * On error, NFEXP_T_ERROR is returned and errno is set appropiately. If 
 * the message received is not of the requested type then 0 is returned, 
 * otherwise this function returns the message type parsed.
 */
int nfexp_parse_expect(enum nf_conntrack_msg_type type,
		       const struct nlmsghdr *nlh,
		       struct nf_expect *exp)
{
	unsigned int flags;
	int len = nlh->nlmsg_len;
	struct nfgenmsg *nfhdr = NLMSG_DATA(nlh);
	struct nfattr *cda[CTA_EXPECT_MAX];

	assert(nlh != NULL);
	assert(exp != NULL);

	len -= NLMSG_LENGTH(sizeof(struct nfgenmsg));
	if (len < 0) {
		errno = EINVAL;
		return NFCT_T_ERROR;
	}

	flags = __parse_expect_message_type(nlh);
	if (!(flags & type))
		return 0;

	nfnl_parse_attr(cda, CTA_EXPECT_MAX, NFA_DATA(nfhdr), len);

	__parse_expect(nlh, cda, exp);

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
 * nfexp_query - send a query to ctnetlink
 * \param h library handler
 * \param qt query type
 * \param data data required to send the query
 *
 * On error, -1 is returned and errno is explicitely set. On success, 0
 * is returned.
 */
int nfexp_query(struct nfct_handle *h,
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

	if (__build_query_exp(h->nfnlssh_exp, qt, data, &u.req, size) == -1)
		return -1;

	return nfnl_query(h->nfnlh, &u.req.nlh);
}

/**
 * nfexp_send - send a query to ctnetlink
 * \param h library handler
 * \param qt query type
 * \param data data required to send the query
 *
 * Like nfexp_query but we do not wait for the reply from ctnetlink.
 * You can use nfexp_send() and nfexp_catch() to emulate nfexp_query().
 * This is particularly useful when the socket is non-blocking.
 *
 * On error, -1 is returned and errno is explicitely set. On success, 0
 * is returned.
 */
int nfexp_send(struct nfct_handle *h,
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

	if (__build_query_exp(h->nfnlssh_exp, qt, data, &u.req, size) == -1)
		return -1;

	return nfnl_send(h->nfnlh, &u.req.nlh);
}

/**
 * nfexp_catch - catch events
 * \param h library handler
 *
 * This function receives the event from the kernel and it invokes the
 * callback that was registered to this handle.
 *
 * On error, -1 is returned and errno is set appropiately. On success,
 * a value greater or equal to 0 is returned indicating the callback
 * verdict: NFCT_CB_STOP, NFCT_CB_CONTINUE or NFCT_CB_STOLEN.
 *
 * Beware that this function is equivalent to nfct_catch(), so it handles both
 * conntrack and expectation events.
 */
int nfexp_catch(struct nfct_handle *h)
{
	assert(h != NULL);

	return nfnl_catch(h->nfnlh);
}

/**
 * @}
 */

/**
 * \defgroup exp Expect object handling
 * @{
 */

/**
 * nfexp_snprintf - print a conntrack object to a buffer
 * \param buf buffer used to build the printable conntrack
 * \param size size of the buffer
 * \param exp pointer to a valid expectation object
 * \param message_type print message type (NFEXP_T_UNKNOWN, NFEXP_T_NEW,...)
 * \param output_type print type (NFEXP_O_DEFAULT, NFEXP_O_XML, ...)
 * \param flags extra flags for the output type (NFEXP_OF_LAYER3)
 *
  * If you are listening to events, probably you want to display the message 
 * type as well. In that case, set the message type parameter to any of the
 * known existing types, ie. NFEXP_T_NEW, NFEXP_T_UPDATE, NFEXP_T_DESTROY.
 * If you pass NFEXP_T_UNKNOWN, the message type will not be output. 
 * 
 * Currently, the output available are:
 * 	- NFEXP_O_DEFAULT: default /proc-like output
 * 	- NFEXP_O_XML: XML output
 * 
 * The output flags are:
 * 	- NFEXP_O_LAYER: include layer 3 information in the output, this is
 * 			*only* required by NFEXP_O_DEFAULT.
 * 
 * On error, -1 is returned and errno is set appropiately. Otherwise,
 * 0 is returned.
 */
int nfexp_snprintf(char *buf,
		  unsigned int size,
		  const struct nf_expect *exp,
		  unsigned int msg_type,
		  unsigned int out_type,
		  unsigned int flags) 
{
	assert(buf != NULL);
	assert(size > 0);
	assert(exp != NULL);

	return __snprintf_expect(buf, size, exp, msg_type, out_type, flags);
}

/**
 * @}
 */
