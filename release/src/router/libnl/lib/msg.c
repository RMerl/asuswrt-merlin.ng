/*
 * lib/msg.c		Netlink Messages Interface
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2003-2012 Thomas Graf <tgraf@suug.ch>
 */

/**
 * @ingroup core
 * @defgroup msg Message Construction & Parsing
 * Netlink Message Construction/Parsing Interface
 * 
 * Related sections in the development guide:
 * - @core_doc{_message_parsing_amp_construction,Message Parsing & Construction}
 *
 * @{
 *
 * Header
 * ------
 * ~~~~{.c}
 * #include <netlink/msg.h>
 * ~~~~
 */

#include <netlink-private/netlink.h>
#include <netlink/netlink.h>
#include <netlink/utils.h>
#include <netlink/cache.h>
#include <netlink/attr.h>
#include <linux/socket.h>

static size_t default_msg_size;

static void __init init_msg_size(void)
{
	default_msg_size = getpagesize();
}

/**
 * @name Size Calculations
 * @{
 */

/**
 * Calculates size of netlink message based on payload length.
 * @arg payload		Length of payload
 *
 * @return size of netlink message without padding.
 */
int nlmsg_size(int payload)
{
	return NLMSG_HDRLEN + payload;
}

static int nlmsg_msg_size(int payload)
{
	return nlmsg_size(payload);
}

/**
 * Calculates size of netlink message including padding based on payload length
 * @arg payload		Length of payload
 *
 * This function is idential to nlmsg_size() + nlmsg_padlen().
 *
 * @return Size of netlink message including padding.
 */
int nlmsg_total_size(int payload)
{
	return NLMSG_ALIGN(nlmsg_msg_size(payload));
}

/**
 * Size of padding that needs to be added at end of message
 * @arg payload		Length of payload
 *
 * Calculates the number of bytes of padding which is required to be added to
 * the end of the message to ensure that the next netlink message header begins
 * properly aligned to NLMSG_ALIGNTO.
 *
 * @return Number of bytes of padding needed.
 */
int nlmsg_padlen(int payload)
{
	return nlmsg_total_size(payload) - nlmsg_msg_size(payload);
}

/** @} */

/**
 * @name Access to Message Payload
 * @{
 */

/**
 * Return pointer to message payload
 * @arg nlh		Netlink message header
 *
 * @return Pointer to start of message payload.
 */
void *nlmsg_data(const struct nlmsghdr *nlh)
{
	return (unsigned char *) nlh + NLMSG_HDRLEN;
}

void *nlmsg_tail(const struct nlmsghdr *nlh)
{
	return (unsigned char *) nlh + NLMSG_ALIGN(nlh->nlmsg_len);
}

/**
 * Return length of message payload
 * @arg nlh		Netlink message header
 *
 * @return Length of message payload in bytes.
 */
int nlmsg_datalen(const struct nlmsghdr *nlh)
{
	return nlh->nlmsg_len - NLMSG_HDRLEN;
}

static int nlmsg_len(const struct nlmsghdr *nlh)
{
	return nlmsg_datalen(nlh);
}

/** @} */

/**
 * @name Attribute Access
 * @{
 */

/**
 * head of attributes data
 * @arg nlh		netlink message header
 * @arg hdrlen		length of family specific header
 */
struct nlattr *nlmsg_attrdata(const struct nlmsghdr *nlh, int hdrlen)
{
	unsigned char *data = nlmsg_data(nlh);
	return (struct nlattr *) (data + NLMSG_ALIGN(hdrlen));
}

/**
 * length of attributes data
 * @arg nlh		netlink message header
 * @arg hdrlen		length of family specific header
 */
int nlmsg_attrlen(const struct nlmsghdr *nlh, int hdrlen)
{
	return max_t(int, nlmsg_len(nlh) - NLMSG_ALIGN(hdrlen), 0);
}

/** @} */

/**
 * @name Message Parsing
 * @{
 */

int nlmsg_valid_hdr(const struct nlmsghdr *nlh, int hdrlen)
{
	if (nlh->nlmsg_len < nlmsg_msg_size(hdrlen))
		return 0;

	return 1;
}

/**
 * check if the netlink message fits into the remaining bytes
 * @arg nlh		netlink message header
 * @arg remaining	number of bytes remaining in message stream
 */
int nlmsg_ok(const struct nlmsghdr *nlh, int remaining)
{
	return (remaining >= (int)sizeof(struct nlmsghdr) &&
		nlh->nlmsg_len >= sizeof(struct nlmsghdr) &&
		nlh->nlmsg_len <= remaining);
}

/**
 * next netlink message in message stream
 * @arg nlh		netlink message header
 * @arg remaining	number of bytes remaining in message stream
 *
 * @returns the next netlink message in the message stream and
 * decrements remaining by the size of the current message.
 */
struct nlmsghdr *nlmsg_next(struct nlmsghdr *nlh, int *remaining)
{
	int totlen = NLMSG_ALIGN(nlh->nlmsg_len);

	*remaining -= totlen;

	return (struct nlmsghdr *) ((unsigned char *) nlh + totlen);
}

/**
 * parse attributes of a netlink message
 * @arg nlh		netlink message header
 * @arg hdrlen		length of family specific header
 * @arg tb		destination array with maxtype+1 elements
 * @arg maxtype		maximum attribute type to be expected
 * @arg policy		validation policy
 *
 * See nla_parse()
 */
int nlmsg_parse(struct nlmsghdr *nlh, int hdrlen, struct nlattr *tb[],
		int maxtype, struct nla_policy *policy)
{
	if (!nlmsg_valid_hdr(nlh, hdrlen))
		return -NLE_MSG_TOOSHORT;

	return nla_parse(tb, maxtype, nlmsg_attrdata(nlh, hdrlen),
			 nlmsg_attrlen(nlh, hdrlen), policy);
}

/**
 * nlmsg_find_attr - find a specific attribute in a netlink message
 * @arg nlh		netlink message header
 * @arg hdrlen		length of familiy specific header
 * @arg attrtype	type of attribute to look for
 *
 * Returns the first attribute which matches the specified type.
 */
struct nlattr *nlmsg_find_attr(struct nlmsghdr *nlh, int hdrlen, int attrtype)
{
	return nla_find(nlmsg_attrdata(nlh, hdrlen),
			nlmsg_attrlen(nlh, hdrlen), attrtype);
}

/**
 * nlmsg_validate - validate a netlink message including attributes
 * @arg nlh		netlinket message header
 * @arg hdrlen		length of familiy specific header
 * @arg maxtype		maximum attribute type to be expected
 * @arg policy		validation policy
 */
int nlmsg_validate(struct nlmsghdr *nlh, int hdrlen, int maxtype,
		   struct nla_policy *policy)
{
	if (!nlmsg_valid_hdr(nlh, hdrlen))
		return -NLE_MSG_TOOSHORT;

	return nla_validate(nlmsg_attrdata(nlh, hdrlen),
			    nlmsg_attrlen(nlh, hdrlen), maxtype, policy);
}

/** @} */

/**
 * @name Message Building/Access
 * @{
 */

static struct nl_msg *__nlmsg_alloc(size_t len)
{
	struct nl_msg *nm;

	if (len < sizeof(struct nlmsghdr))
		len = sizeof(struct nlmsghdr);

	nm = calloc(1, sizeof(*nm));
	if (!nm)
		goto errout;

	nm->nm_refcnt = 1;

	nm->nm_nlh = calloc(1, len);
	if (!nm->nm_nlh)
		goto errout;

	nm->nm_protocol = -1;
	nm->nm_size = len;
	nm->nm_nlh->nlmsg_len = nlmsg_total_size(0);

	NL_DBG(2, "msg %p: Allocated new message, maxlen=%zu\n", nm, len);

	return nm;
errout:
	free(nm);
	return NULL;
}

/**
 * Allocate a new netlink message with the default maximum payload size.
 *
 * Allocates a new netlink message without any further payload. The
 * maximum payload size defaults to PAGESIZE or as otherwise specified
 * with nlmsg_set_default_size().
 *
 * @return Newly allocated netlink message or NULL.
 */
struct nl_msg *nlmsg_alloc(void)
{
	return __nlmsg_alloc(default_msg_size);
}

/**
 * Allocate a new netlink message with maximum payload size specified.
 */
struct nl_msg *nlmsg_alloc_size(size_t max)
{
	return __nlmsg_alloc(max);
}

/**
 * Allocate a new netlink message and inherit netlink message header
 * @arg hdr		Netlink message header template
 *
 * Allocates a new netlink message and inherits the original message
 * header. If \a hdr is not NULL it will be used as a template for
 * the netlink message header, otherwise the header is left blank.
 * 
 * @return Newly allocated netlink message or NULL
 */ 
struct nl_msg *nlmsg_inherit(struct nlmsghdr *hdr)
{
	struct nl_msg *nm;

	nm = nlmsg_alloc();
	if (nm && hdr) {
		struct nlmsghdr *new = nm->nm_nlh;

		new->nlmsg_type = hdr->nlmsg_type;
		new->nlmsg_flags = hdr->nlmsg_flags;
		new->nlmsg_seq = hdr->nlmsg_seq;
		new->nlmsg_pid = hdr->nlmsg_pid;
	}

	return nm;
}

/**
 * Allocate a new netlink message
 * @arg nlmsgtype	Netlink message type
 * @arg flags		Message flags.
 *
 * @return Newly allocated netlink message or NULL.
 */
struct nl_msg *nlmsg_alloc_simple(int nlmsgtype, int flags)
{
	struct nl_msg *msg;
	struct nlmsghdr nlh = {
		.nlmsg_type = nlmsgtype,
		.nlmsg_flags = flags,
	};

	msg = nlmsg_inherit(&nlh);
	if (msg)
		NL_DBG(2, "msg %p: Allocated new simple message\n", msg);

	return msg;
}

/**
 * Set the default maximum message payload size for allocated messages
 * @arg max		Size of payload in bytes.
 */
void nlmsg_set_default_size(size_t max)
{
	if (max < nlmsg_total_size(0))
		max = nlmsg_total_size(0);

	default_msg_size = max;
}

/**
 * Convert a netlink message received from a netlink socket to a nl_msg
 * @arg hdr		Netlink message received from netlink socket.
 *
 * Allocates a new netlink message and copies all of the data pointed to
 * by \a hdr into the new message object.
 *
 * @return Newly allocated netlink message or NULL.
 */
struct nl_msg *nlmsg_convert(struct nlmsghdr *hdr)
{
	struct nl_msg *nm;

	nm = __nlmsg_alloc(NLMSG_ALIGN(hdr->nlmsg_len));
	if (!nm)
		return NULL;

	memcpy(nm->nm_nlh, hdr, hdr->nlmsg_len);

	return nm;
}

/**
 * Reserve room for additional data in a netlink message
 * @arg n		netlink message
 * @arg len		length of additional data to reserve room for
 * @arg pad		number of bytes to align data to
 *
 * Reserves room for additional data at the tail of the an
 * existing netlink message. Eventual padding required will
 * be zeroed out.
 *
 * @return Pointer to start of additional data tailroom or NULL.
 */
void *nlmsg_reserve(struct nl_msg *n, size_t len, int pad)
{
	void *buf = n->nm_nlh;
	size_t nlmsg_len = n->nm_nlh->nlmsg_len;
	size_t tlen;

	tlen = pad ? ((len + (pad - 1)) & ~(pad - 1)) : len;

	if ((tlen + nlmsg_len) > n->nm_size)
		return NULL;

	buf += nlmsg_len;
	n->nm_nlh->nlmsg_len += tlen;

	if (tlen > len)
		memset(buf + len, 0, tlen - len);

	NL_DBG(2, "msg %p: Reserved %zu (%zu) bytes, pad=%d, nlmsg_len=%d\n",
		  n, tlen, len, pad, n->nm_nlh->nlmsg_len);

	return buf;
}

/**
 * Append data to tail of a netlink message
 * @arg n		netlink message
 * @arg data		data to add
 * @arg len		length of data
 * @arg pad		Number of bytes to align data to.
 *
 * Extends the netlink message as needed and appends the data of given
 * length to the message. 
 *
 * @return 0 on success or a negative error code
 */
int nlmsg_append(struct nl_msg *n, void *data, size_t len, int pad)
{
	void *tmp;

	tmp = nlmsg_reserve(n, len, pad);
	if (tmp == NULL)
		return -NLE_NOMEM;

	memcpy(tmp, data, len);
	NL_DBG(2, "msg %p: Appended %zu bytes with padding %d\n", n, len, pad);

	return 0;
}

/**
 * Expand maximum payload size of a netlink message
 * @arg n		Netlink message.
 * @arg newlen		New maximum payload size.
 *
 * Reallocates the payload section of a netlink message and increases
 * the maximum payload size of the message.
 *
 * @note Any pointers pointing to old payload block will be stale and
 *       need to be refetched. Therfore, do not expand while constructing
 *       nested attributes or while reserved data blocks are held.
 *
 * @return 0 on success or a negative error code.
 */
int nlmsg_expand(struct nl_msg *n, size_t newlen)
{
	void *tmp;

	if (newlen <= n->nm_size)
		return -NLE_INVAL;

	tmp = realloc(n->nm_nlh, newlen);
	if (tmp == NULL)
		return -NLE_NOMEM;

	n->nm_nlh = tmp;
	n->nm_size = newlen;

	return 0;
}

/**
 * Add a netlink message header to a netlink message
 * @arg n		netlink message
 * @arg pid		netlink process id or NL_AUTO_PID
 * @arg seq		sequence number of message or NL_AUTO_SEQ
 * @arg type		message type
 * @arg payload		length of message payload
 * @arg flags		message flags
 *
 * Adds or overwrites the netlink message header in an existing message
 * object. If \a payload is greater-than zero additional room will be
 * reserved, f.e. for family specific headers. It can be accesed via
 * nlmsg_data().
 *
 * @return A pointer to the netlink message header or NULL.
 */
struct nlmsghdr *nlmsg_put(struct nl_msg *n, uint32_t pid, uint32_t seq,
			   int type, int payload, int flags)
{
	struct nlmsghdr *nlh;

	if (n->nm_nlh->nlmsg_len < NLMSG_HDRLEN)
		BUG();

	nlh = (struct nlmsghdr *) n->nm_nlh;
	nlh->nlmsg_type = type;
	nlh->nlmsg_flags = flags;
	nlh->nlmsg_pid = pid;
	nlh->nlmsg_seq = seq;

	NL_DBG(2, "msg %p: Added netlink header type=%d, flags=%d, pid=%d, "
		  "seq=%d\n", n, type, flags, pid, seq);

	if (payload > 0 &&
	    nlmsg_reserve(n, payload, NLMSG_ALIGNTO) == NULL)
		return NULL;

	return nlh;
}

/**
 * Return actual netlink message
 * @arg n		netlink message
 * 
 * Returns the actual netlink message casted to the type of the netlink
 * message header.
 * 
 * @return A pointer to the netlink message.
 */
struct nlmsghdr *nlmsg_hdr(struct nl_msg *n)
{
	return n->nm_nlh;
}

/**
 * Acquire a reference on a netlink message
 * @arg msg		message to acquire reference from
 */
void nlmsg_get(struct nl_msg *msg)
{
	msg->nm_refcnt++;
	NL_DBG(4, "New reference to message %p, total %d\n",
	       msg, msg->nm_refcnt);
}

/**
 * Release a reference from an netlink message
 * @arg msg		message to release reference from
 *
 * Frees memory after the last reference has been released.
 */
void nlmsg_free(struct nl_msg *msg)
{
	if (!msg)
		return;

	msg->nm_refcnt--;
	NL_DBG(4, "Returned message reference %p, %d remaining\n",
	       msg, msg->nm_refcnt);

	if (msg->nm_refcnt < 0)
		BUG();

	if (msg->nm_refcnt <= 0) {
		free(msg->nm_nlh);
		NL_DBG(2, "msg %p: Freed\n", msg);
		free(msg);
	}
}

/** @} */

/**
 * @name Attributes
 * @{
 */

void nlmsg_set_proto(struct nl_msg *msg, int protocol)
{
	msg->nm_protocol = protocol;
}

int nlmsg_get_proto(struct nl_msg *msg)
{
	return msg->nm_protocol;
}

size_t nlmsg_get_max_size(struct nl_msg *msg)
{
	return msg->nm_size;
}

void nlmsg_set_src(struct nl_msg *msg, struct sockaddr_nl *addr)
{
	memcpy(&msg->nm_src, addr, sizeof(*addr));
}

struct sockaddr_nl *nlmsg_get_src(struct nl_msg *msg)
{
	return &msg->nm_src;
}

void nlmsg_set_dst(struct nl_msg *msg, struct sockaddr_nl *addr)
{
	memcpy(&msg->nm_dst, addr, sizeof(*addr));
}

struct sockaddr_nl *nlmsg_get_dst(struct nl_msg *msg)
{
	return &msg->nm_dst;
}

void nlmsg_set_creds(struct nl_msg *msg, struct ucred *creds)
{
	memcpy(&msg->nm_creds, creds, sizeof(*creds));
	msg->nm_flags |= NL_MSG_CRED_PRESENT;
}

struct ucred *nlmsg_get_creds(struct nl_msg *msg)
{
	if (msg->nm_flags & NL_MSG_CRED_PRESENT)
		return &msg->nm_creds;
	return NULL;
}

/** @} */

/**
 * @name Netlink Message Type Translations
 * @{
 */

static const struct trans_tbl nl_msgtypes[] = {
	__ADD(NLMSG_NOOP,NOOP)
	__ADD(NLMSG_ERROR,ERROR)
	__ADD(NLMSG_DONE,DONE)
	__ADD(NLMSG_OVERRUN,OVERRUN)
};

char *nl_nlmsgtype2str(int type, char *buf, size_t size)
{
	return __type2str(type, buf, size, nl_msgtypes,
			  ARRAY_SIZE(nl_msgtypes));
}

int nl_str2nlmsgtype(const char *name)
{
	return __str2type(name, nl_msgtypes, ARRAY_SIZE(nl_msgtypes));
}

/** @} */

/**
 * @name Netlink Message Flags Translations
 * @{
 */

char *nl_nlmsg_flags2str(int flags, char *buf, size_t len)
{
	memset(buf, 0, len);

#define PRINT_FLAG(f) \
	if (flags & NLM_F_##f) { \
		flags &= ~NLM_F_##f; \
		strncat(buf, #f, len - strlen(buf) - 1); \
		if (flags) \
			strncat(buf, ",", len - strlen(buf) - 1); \
	}
	
	PRINT_FLAG(REQUEST);
	PRINT_FLAG(MULTI);
	PRINT_FLAG(ACK);
	PRINT_FLAG(ECHO);
	PRINT_FLAG(ROOT);
	PRINT_FLAG(MATCH);
	PRINT_FLAG(ATOMIC);
	PRINT_FLAG(REPLACE);
	PRINT_FLAG(EXCL);
	PRINT_FLAG(CREATE);
	PRINT_FLAG(APPEND);

	if (flags) {
		char s[32];
		snprintf(s, sizeof(s), "0x%x", flags);
		strncat(buf, s, len - strlen(buf) - 1);
	}
#undef PRINT_FLAG

	return buf;
}

/** @} */

/**
 * @name Direct Parsing
 * @{
 */

/** @cond SKIP */
struct dp_xdata {
	void (*cb)(struct nl_object *, void *);
	void *arg;
};
/** @endcond */

static int parse_cb(struct nl_object *obj, struct nl_parser_param *p)
{
	struct dp_xdata *x = p->pp_arg;

	x->cb(obj, x->arg);
	return 0;
}

int nl_msg_parse(struct nl_msg *msg, void (*cb)(struct nl_object *, void *),
		 void *arg)
{
	struct nl_cache_ops *ops;
	struct nl_parser_param p = {
		.pp_cb = parse_cb
	};
	struct dp_xdata x = {
		.cb = cb,
		.arg = arg,
	};
	int err;

	ops = nl_cache_ops_associate_safe(nlmsg_get_proto(msg),
					  nlmsg_hdr(msg)->nlmsg_type);
	if (ops == NULL)
		return -NLE_MSGTYPE_NOSUPPORT;
	p.pp_arg = &x;

	err = nl_cache_parse(ops, NULL, nlmsg_hdr(msg), &p);
	nl_cache_ops_put(ops);

	return err;
}

/** @} */

/**
 * @name Dumping
 * @{
 */

static void prefix_line(FILE *ofd, int prefix)
{
	int i;

	for (i = 0; i < prefix; i++)
		fprintf(ofd, "  ");
}

static inline void dump_hex(FILE *ofd, char *start, int len, int prefix)
{
	int i, a, c, limit;
	char ascii[21] = {0};

	limit = 16 - (prefix * 2);
	prefix_line(ofd, prefix);
	fprintf(ofd, "    ");

	for (i = 0, a = 0, c = 0; i < len; i++) {
		int v = *(uint8_t *) (start + i);

		fprintf(ofd, "%02x ", v);
		ascii[a++] = isprint(v) ? v : '.';

		if (++c >= limit) {
			fprintf(ofd, "%s\n", ascii);
			if (i < (len - 1)) {
				prefix_line(ofd, prefix);
				fprintf(ofd, "    ");
			}
			a = c = 0;
			memset(ascii, 0, sizeof(ascii));
		}
	}

	if (c != 0) {
		for (i = 0; i < (limit - c); i++)
			fprintf(ofd, "   ");
		fprintf(ofd, "%s\n", ascii);
	}
}

static void print_hdr(FILE *ofd, struct nl_msg *msg)
{
	struct nlmsghdr *nlh = nlmsg_hdr(msg);
	struct nl_cache_ops *ops;
	struct nl_msgtype *mt;
	char buf[128];

	fprintf(ofd, "    .nlmsg_len = %d\n", nlh->nlmsg_len);

	ops = nl_cache_ops_associate_safe(nlmsg_get_proto(msg), nlh->nlmsg_type);
	if (ops) {
		mt = nl_msgtype_lookup(ops, nlh->nlmsg_type);
		if (!mt)
			BUG();

		snprintf(buf, sizeof(buf), "%s::%s", ops->co_name, mt->mt_name);
		nl_cache_ops_put(ops);
	} else
		nl_nlmsgtype2str(nlh->nlmsg_type, buf, sizeof(buf));

	fprintf(ofd, "    .type = %d <%s>\n", nlh->nlmsg_type, buf);
	fprintf(ofd, "    .flags = %d <%s>\n", nlh->nlmsg_flags,
		nl_nlmsg_flags2str(nlh->nlmsg_flags, buf, sizeof(buf)));
	fprintf(ofd, "    .seq = %d\n", nlh->nlmsg_seq);
	fprintf(ofd, "    .port = %d\n", nlh->nlmsg_pid);

}

static void print_genl_hdr(FILE *ofd, void *start)
{
	struct genlmsghdr *ghdr = start;

	fprintf(ofd, "  [GENERIC NETLINK HEADER] %zu octets\n", GENL_HDRLEN);
	fprintf(ofd, "    .cmd = %u\n", ghdr->cmd);
	fprintf(ofd, "    .version = %u\n", ghdr->version);
	fprintf(ofd, "    .unused = %#x\n", ghdr->reserved);
}

static void *print_genl_msg(struct nl_msg *msg, FILE *ofd, struct nlmsghdr *hdr,
			    struct nl_cache_ops *ops, int *payloadlen)
{
	void *data = nlmsg_data(hdr);

	if (*payloadlen < GENL_HDRLEN)
		return data;

	print_genl_hdr(ofd, data);

	*payloadlen -= GENL_HDRLEN;
	data += GENL_HDRLEN;

	if (ops) {
		int hdrsize = ops->co_hdrsize - GENL_HDRLEN;

		if (hdrsize > 0) {
			if (*payloadlen < hdrsize)
				return data;

			fprintf(ofd, "  [HEADER] %d octets\n", hdrsize);
			dump_hex(ofd, data, hdrsize, 0);

			*payloadlen -= hdrsize;
			data += hdrsize;
		}
	}

	return data;
}

static void dump_attr(FILE *ofd, struct nlattr *attr, int prefix)
{
	int len = nla_len(attr);

	dump_hex(ofd, nla_data(attr), len, prefix);
}

static void dump_attrs(FILE *ofd, struct nlattr *attrs, int attrlen,
		       int prefix)
{
	int rem;
	struct nlattr *nla;

	nla_for_each_attr(nla, attrs, attrlen, rem) {
		int padlen, alen = nla_len(nla);

		prefix_line(ofd, prefix);

		if (nla->nla_type == 0)
			fprintf(ofd, "  [ATTR PADDING] %d octets\n", alen);
		else
			fprintf(ofd, "  [ATTR %02d%s] %d octets\n", nla_type(nla),
				nla_is_nested(nla) ? " NESTED" : "",
				alen);

		if (nla_is_nested(nla))
			dump_attrs(ofd, nla_data(nla), alen, prefix+1);
		else
			dump_attr(ofd, nla, prefix);

		padlen = nla_padlen(alen);
		if (padlen > 0) {
			prefix_line(ofd, prefix);
			fprintf(ofd, "  [PADDING] %d octets\n",
				padlen);
			dump_hex(ofd, nla_data(nla) + alen,
				 padlen, prefix);
		}
	}

	if (rem) {
		prefix_line(ofd, prefix);
		fprintf(ofd, "  [LEFTOVER] %d octets\n", rem);
	}
}

static void dump_error_msg(struct nl_msg *msg, FILE *ofd)
{
	struct nlmsghdr *hdr = nlmsg_hdr(msg);
	struct nlmsgerr *err = nlmsg_data(hdr);

	fprintf(ofd, "  [ERRORMSG] %zu octets\n", sizeof(*err));

	if (nlmsg_len(hdr) >= sizeof(*err)) {
		char buf[256];
		struct nl_msg *errmsg;

		fprintf(ofd, "    .error = %d \"%s\"\n", err->error,
			strerror_r(-err->error, buf, sizeof(buf)));
		fprintf(ofd, "  [ORIGINAL MESSAGE] %zu octets\n", sizeof(*hdr));

		errmsg = nlmsg_inherit(&err->msg);
		print_hdr(ofd, errmsg);
		nlmsg_free(errmsg);
	}
}

static void print_msg(struct nl_msg *msg, FILE *ofd, struct nlmsghdr *hdr)
{
	struct nl_cache_ops *ops;
	int payloadlen = nlmsg_len(hdr);
	int attrlen = 0;
	void *data;

	data = nlmsg_data(hdr);
	ops = nl_cache_ops_associate_safe(nlmsg_get_proto(msg),
					  hdr->nlmsg_type);
	if (ops) {
		attrlen = nlmsg_attrlen(hdr, ops->co_hdrsize);
		payloadlen -= attrlen;
	}

	if (msg->nm_protocol == NETLINK_GENERIC)
		data = print_genl_msg(msg, ofd, hdr, ops, &payloadlen);

	if (payloadlen) {
		fprintf(ofd, "  [PAYLOAD] %d octets\n", payloadlen);
		dump_hex(ofd, data, payloadlen, 0);
	}

	if (attrlen) {
		struct nlattr *attrs;
		int attrlen;
		
		attrs = nlmsg_attrdata(hdr, ops->co_hdrsize);
		attrlen = nlmsg_attrlen(hdr, ops->co_hdrsize);
		dump_attrs(ofd, attrs, attrlen, 0);
	}

	if (ops)
		nl_cache_ops_put(ops);
}

/**
 * Dump message in human readable format to file descriptor
 * @arg msg		Message to print
 * @arg ofd		File descriptor.
 */
void nl_msg_dump(struct nl_msg *msg, FILE *ofd)
{
	struct nlmsghdr *hdr = nlmsg_hdr(msg);
	
	fprintf(ofd, 
	"--------------------------   BEGIN NETLINK MESSAGE ---------------------------\n");

	fprintf(ofd, "  [NETLINK HEADER] %zu octets\n", sizeof(struct nlmsghdr));
	print_hdr(ofd, msg);

	if (hdr->nlmsg_type == NLMSG_ERROR)
		dump_error_msg(msg, ofd);
	else if (nlmsg_len(hdr) > 0)
		print_msg(msg, ofd, hdr);

	fprintf(ofd, 
	"---------------------------  END NETLINK MESSAGE   ---------------------------\n");
}

/** @} */

/** @} */
