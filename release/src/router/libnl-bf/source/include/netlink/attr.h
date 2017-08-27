/*
 * netlink/attr.h		Netlink Attributes
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2003-2008 Thomas Graf <tgraf@suug.ch>
 */

#ifndef NETLINK_ATTR_H_
#define NETLINK_ATTR_H_

#include <netlink/netlink.h>
#include <netlink/object.h>
#include <netlink/addr.h>
#include <netlink/data.h>

#ifdef __cplusplus
extern "C" {
#endif

struct nl_msg;

/**
 * @name Basic Attribute Data Types
 * @{
 */

 /**
  * @ingroup attr
  * Basic attribute data types
  *
  * See \ref attr_datatypes for more details.
  */
enum {
	NLA_UNSPEC,	/**< Unspecified type, binary data chunk */
	NLA_U8,		/**< 8 bit integer */
	NLA_U16,	/**< 16 bit integer */
	NLA_U32,	/**< 32 bit integer */
	NLA_U64,	/**< 64 bit integer */
	NLA_STRING,	/**< NUL terminated character string */
	NLA_FLAG,	/**< Flag */
	NLA_MSECS,	/**< Micro seconds (64bit) */
	NLA_NESTED,	/**< Nested attributes */
	__NLA_TYPE_MAX,
};

#define NLA_TYPE_MAX (__NLA_TYPE_MAX - 1)

/** @} */

/**
 * @ingroup attr
 * Attribute validation policy.
 *
 * See \ref attr_datatypes for more details.
 */
struct nla_policy {
	/** Type of attribute or NLA_UNSPEC */
	uint16_t	type;

	/** Minimal length of payload required */
	uint16_t	minlen;

	/** Maximal length of payload allowed */
	uint16_t	maxlen;
};

/* Size calculations */
extern int		nla_attr_size(int payload);
extern int		nla_total_size(int payload);
extern int		nla_padlen(int payload);

/* Attribute parsing */
extern int		nla_type(const struct nlattr *);
extern void *		nla_data(const struct nlattr *);
extern int		nla_len(const struct nlattr *);
extern int		nla_ok(const struct nlattr *, int);
extern struct nlattr *	nla_next(const struct nlattr *, int *);
extern int		nla_parse(struct nlattr **, int, struct nlattr *,
				  int, struct nla_policy *);
extern int		nla_validate(struct nlattr *, int, int,
				     struct nla_policy *);
extern struct nlattr *	nla_find(struct nlattr *, int, int);

/* Helper Functions */
extern int		nla_memcpy(void *, struct nlattr *, int);
extern size_t		nla_strlcpy(char *, const struct nlattr *, size_t);
extern int		nla_memcmp(const struct nlattr *, const void *, size_t);
extern int		nla_strcmp(const struct nlattr *, const char *);

/* Unspecific attribute */
extern struct nlattr *	nla_reserve(struct nl_msg *, int, int);
extern int		nla_put(struct nl_msg *, int, int, const void *);
extern int		nla_put_data(struct nl_msg *, int, struct nl_data *);
extern int		nla_put_addr(struct nl_msg *, int, struct nl_addr *);

/* Integer attribute */
extern uint8_t		nla_get_u8(struct nlattr *);
extern int		nla_put_u8(struct nl_msg *, int, uint8_t);
extern uint16_t		nla_get_u16(struct nlattr *);
extern int		nla_put_u16(struct nl_msg *, int, uint16_t);
extern uint32_t		nla_get_u32(struct nlattr *);
extern int		nla_put_u32(struct nl_msg *, int, uint32_t);
extern uint64_t		nla_get_u64(struct nlattr *);
extern int		nla_put_u64(struct nl_msg *, int, uint64_t);

/* String attribute */
extern char *		nla_get_string(struct nlattr *);
extern char *		nla_strdup(struct nlattr *);
extern int		nla_put_string(struct nl_msg *, int, const char *);

/* Flag attribute */
extern int		nla_get_flag(struct nlattr *);
extern int		nla_put_flag(struct nl_msg *, int);

/* Msec attribute */
extern unsigned long	nla_get_msecs(struct nlattr *);
extern int		nla_put_msecs(struct nl_msg *, int, unsigned long);

/* Attribute nesting */
extern int		nla_put_nested(struct nl_msg *, int, struct nl_msg *);
extern struct nlattr *	nla_nest_start(struct nl_msg *, int);
extern int		nla_nest_end(struct nl_msg *, struct nlattr *);
extern int		nla_parse_nested(struct nlattr **, int, struct nlattr *,
					 struct nla_policy *);

/**
 * @name Attribute Construction (Exception Based)
 * @{
 */

/**
 * @ingroup attr
 * Add unspecific attribute to netlink message.
 * @arg msg		Netlink message.
 * @arg attrtype	Attribute type.
 * @arg attrlen		Length of attribute payload.
 * @arg data		Head of attribute payload.
 */
#define NLA_PUT(msg, attrtype, attrlen, data) \
	do { \
		if (nla_put(msg, attrtype, attrlen, data) < 0) \
			goto nla_put_failure; \
	} while(0)

/**
 * @ingroup attr
 * Add atomic type attribute to netlink message.
 * @arg msg		Netlink message.
 * @arg type		Atomic type.
 * @arg attrtype	Attribute type.
 * @arg value		Head of attribute payload.
 */
#define NLA_PUT_TYPE(msg, type, attrtype, value) \
	do { \
		type __tmp = value; \
		NLA_PUT(msg, attrtype, sizeof(type), &__tmp); \
	} while(0)

/**
 * Add 8 bit integer attribute to netlink message.
 * @arg msg		Netlink message.
 * @arg attrtype	Attribute type.
 * @arg value		Numeric value.
 */
#define NLA_PUT_U8(msg, attrtype, value) \
	NLA_PUT_TYPE(msg, uint8_t, attrtype, value)

/**
 * Add 16 bit integer attribute to netlink message.
 * @arg msg		Netlink message.
 * @arg attrtype	Attribute type.
 * @arg value		Numeric value.
 */
#define NLA_PUT_U16(msg, attrtype, value) \
	NLA_PUT_TYPE(msg, uint16_t, attrtype, value)

/**
 * Add 32 bit integer attribute to netlink message.
 * @arg msg		Netlink message.
 * @arg attrtype	Attribute type.
 * @arg value		Numeric value.
 */
#define NLA_PUT_U32(msg, attrtype, value) \
	NLA_PUT_TYPE(msg, uint32_t, attrtype, value)

/**
 * Add 64 bit integer attribute to netlink message.
 * @arg msg		Netlink message.
 * @arg attrtype	Attribute type.
 * @arg value		Numeric value.
 */
#define NLA_PUT_U64(msg, attrtype, value) \
	NLA_PUT_TYPE(msg, uint64_t, attrtype, value)

/**
 * Add string attribute to netlink message.
 * @arg msg		Netlink message.
 * @arg attrtype	Attribute type.
 * @arg value		NUL terminated character string.
 */
#define NLA_PUT_STRING(msg, attrtype, value) \
	NLA_PUT(msg, attrtype, strlen(value) + 1, value)

/**
 * Add flag attribute to netlink message.
 * @arg msg		Netlink message.
 * @arg attrtype	Attribute type.
 */
#define NLA_PUT_FLAG(msg, attrtype) \
	NLA_PUT(msg, attrtype, 0, NULL)

/**
 * Add msecs attribute to netlink message.
 * @arg msg		Netlink message.
 * @arg attrtype	Attribute type.
 * @arg msecs		Numeric value in micro seconds.
 */
#define NLA_PUT_MSECS(msg, attrtype, msecs) \
	NLA_PUT_U64(msg, attrtype, msecs)

/**
 * Add address attribute to netlink message.
 * @arg msg		Netlink message.
 * @arg attrtype	Attribute type.
 * @arg addr		Abstract address object.
 */
#define NLA_PUT_ADDR(msg, attrtype, addr) \
	NLA_PUT(msg, attrtype, nl_addr_get_len(addr), \
		nl_addr_get_binary_addr(addr))

/**
 * Add abstract data attribute to netlink message.
 * @arg msg		Netlink message.
 * @arg attrtype	Attribute type.
 * @arg data		Abstract data object.
 */
#define NLA_PUT_DATA(msg, attrtype, data) \
	NLA_PUT(msg, attrtype, nl_data_get_size(data), \
		nl_data_get(data))

/** @} */

/**
 * @name Iterators
 * @{
 */

/**
 * @ingroup attr
 * Iterate over a stream of attributes
 * @arg pos	loop counter, set to current attribute
 * @arg head	head of attribute stream
 * @arg len	length of attribute stream
 * @arg rem	initialized to len, holds bytes currently remaining in stream
 */
#define nla_for_each_attr(pos, head, len, rem) \
	for (pos = head, rem = len; \
	     nla_ok(pos, rem); \
	     pos = nla_next(pos, &(rem)))

/**
 * @ingroup attr
 * Iterate over a stream of nested attributes
 * @arg pos	loop counter, set to current attribute
 * @arg nla	attribute containing the nested attributes
 * @arg rem	initialized to len, holds bytes currently remaining in stream
 */
#define nla_for_each_nested(pos, nla, rem) \
	for (pos = nla_data(nla), rem = nla_len(nla); \
	     nla_ok(pos, rem); \
	     pos = nla_next(pos, &(rem)))

/** @} */

#ifdef __cplusplus
}
#endif

#endif
