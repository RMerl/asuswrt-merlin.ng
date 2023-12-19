/*
 * prettymsg.h - human readable message dump
 *
 * Support for pretty print of an ethtool netlink message
 */

#ifndef ETHTOOL_NETLINK_PRETTYMSG_H__
#define ETHTOOL_NETLINK_PRETTYMSG_H__

#include <linux/netlink.h>

/* data structures for message format descriptions */

enum pretty_nla_format {
	NLA_INVALID,
	NLA_BINARY,
	NLA_U8,
	NLA_U16,
	NLA_U32,
	NLA_U64,
	NLA_X8,
	NLA_X16,
	NLA_X32,
	NLA_X64,
	NLA_S8,
	NLA_S16,
	NLA_S32,
	NLA_S64,
	NLA_STRING,
	NLA_FLAG,
	NLA_BOOL,
	NLA_NESTED,
	NLA_ARRAY,
	NLA_U8_ENUM,
	NLA_U32_ENUM,
};

struct pretty_nla_desc {
	enum pretty_nla_format		format;
	const char			*name;
	union {
		const struct pretty_nla_desc	*children;
		const char			*const *names;
	};
	union {
		unsigned int			n_children;
		unsigned int			n_names;
	};
};

struct pretty_nlmsg_desc {
	const char			*name;
	const struct pretty_nla_desc	*attrs;
	unsigned int			n_attrs;
};

/* helper macros for message format descriptions */

#define NLATTR_DESC(_name, _fmt) \
	[_name] = { \
		.format = _fmt, \
		.name = #_name, \
	}

#define NLATTR_DESC_INVALID(_name)	NLATTR_DESC(_name, NLA_INVALID)
#define NLATTR_DESC_U8(_name)		NLATTR_DESC(_name, NLA_U8)
#define NLATTR_DESC_U16(_name)		NLATTR_DESC(_name, NLA_U16)
#define NLATTR_DESC_U32(_name)		NLATTR_DESC(_name, NLA_U32)
#define NLATTR_DESC_U64(_name)		NLATTR_DESC(_name, NLA_U64)
#define NLATTR_DESC_X8(_name)		NLATTR_DESC(_name, NLA_X8)
#define NLATTR_DESC_X16(_name)		NLATTR_DESC(_name, NLA_X16)
#define NLATTR_DESC_X32(_name)		NLATTR_DESC(_name, NLA_X32)
#define NLATTR_DESC_X64(_name)		NLATTR_DESC(_name, NLA_X64)
#define NLATTR_DESC_S8(_name)		NLATTR_DESC(_name, NLA_U8)
#define NLATTR_DESC_S16(_name)		NLATTR_DESC(_name, NLA_U16)
#define NLATTR_DESC_S32(_name)		NLATTR_DESC(_name, NLA_U32)
#define NLATTR_DESC_S64(_name)		NLATTR_DESC(_name, NLA_S64)
#define NLATTR_DESC_STRING(_name)	NLATTR_DESC(_name, NLA_STRING)
#define NLATTR_DESC_FLAG(_name)		NLATTR_DESC(_name, NLA_FLAG)
#define NLATTR_DESC_BOOL(_name)		NLATTR_DESC(_name, NLA_BOOL)
#define NLATTR_DESC_BINARY(_name)	NLATTR_DESC(_name, NLA_BINARY)

#define NLATTR_DESC_NESTED(_name, _children_desc) \
	[_name] = { \
		.format = NLA_NESTED, \
		.name = #_name, \
		.children = __ ## _children_desc ## _desc, \
		.n_children = ARRAY_SIZE(__ ## _children_desc ## _desc), \
	}
#define NLATTR_DESC_NESTED_NODESC(_name) NLATTR_DESC(_name, NLA_NESTED)
#define NLATTR_DESC_ARRAY(_name, _children_desc) \
	[_name] = { \
		.format = NLA_ARRAY, \
		.name = #_name, \
		.children = __ ## _children_desc ## _desc, \
		.n_children = 1, \
	}
#define NLATTR_DESC_U8_ENUM(_name, _names_table) \
	[_name] = { \
		.format = NLA_U8_ENUM, \
		.name = #_name, \
		.names = __ ## _names_table ## _names, \
		.n_children = ARRAY_SIZE(__ ## _names_table ## _names), \
	}
#define NLATTR_DESC_U32_ENUM(_name, _names_table) \
	[_name] = { \
		.format = NLA_U32_ENUM, \
		.name = #_name, \
		.names = __ ## _names_table ## _names, \
		.n_children = ARRAY_SIZE(__ ## _names_table ## _names), \
	}

#define NLMSG_DESC(_name, _attrs) \
	[_name] = { \
		.name = #_name, \
		.attrs = __ ## _attrs ## _desc, \
		.n_attrs = ARRAY_SIZE(__ ## _attrs ## _desc), \
	}

#define NLMSG_DESC_INVALID(_name) \
	[_name] = { \
		.name = #_name, \
	}

/* function to pretty print a genetlink message */
int pretty_print_genlmsg(const struct nlmsghdr *nlhdr,
			 const struct pretty_nlmsg_desc *desc,
			 unsigned int ndesc, unsigned int err_offset);
int pretty_print_rtnlmsg(const struct nlmsghdr *nlhdr, unsigned int err_offset);

/* message descriptions */

extern const struct pretty_nlmsg_desc ethnl_umsg_desc[];
extern const unsigned int ethnl_umsg_n_desc;
extern const struct pretty_nlmsg_desc ethnl_kmsg_desc[];
extern const unsigned int ethnl_kmsg_n_desc;

extern const struct pretty_nlmsg_desc genlctrl_msg_desc[];
extern const unsigned int genlctrl_msg_n_desc;

extern const struct pretty_nlmsg_desc rtnl_msg_desc[];
extern const unsigned int rtnl_msg_n_desc;
extern const unsigned short rtnl_msghdr_lengths[];
extern const unsigned int rtnl_msghdr_n_len;

#endif /* ETHTOOL_NETLINK_PRETTYMSG_H__ */
