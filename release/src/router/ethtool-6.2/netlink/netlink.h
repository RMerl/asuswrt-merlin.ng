/*
 * netlink.h - common interface for all netlink code
 *
 * Declarations of data structures, global data and helpers for netlink code
 */

#ifndef ETHTOOL_NETLINK_INT_H__
#define ETHTOOL_NETLINK_INT_H__

#include <../../libmnl-1.0.4/include/libmnl/libmnl.h>
#include <linux/netlink.h>
#include <linux/genetlink.h>
#include <linux/ethtool_netlink.h>
#include "nlsock.h"

#define WILDCARD_DEVNAME "*"
#define CMDMASK_WORDS DIV_ROUND_UP(__ETHTOOL_MSG_KERNEL_CNT, 32)

enum link_mode_class {
	LM_CLASS_UNKNOWN,
	LM_CLASS_REAL,
	LM_CLASS_AUTONEG,
	LM_CLASS_PORT,
	LM_CLASS_PAUSE,
	LM_CLASS_FEC,
};

struct nl_op_info {
	uint32_t		op_flags;
	uint32_t		hdr_flags;
	uint8_t			hdr_policy_loaded:1;
};

struct nl_context {
	struct cmd_context	*ctx;
	void			*cmd_private;
	const char		*devname;
	bool			is_dump;
	int			exit_code;
	unsigned int		suppress_nlerr;
	uint16_t		ethnl_fam;
	uint32_t		ethnl_mongrp;
	struct nl_op_info	*ops_info;
	struct nl_socket	*ethnl_socket;
	struct nl_socket	*ethnl2_socket;
	struct nl_socket	*rtnl_socket;
	bool			is_monitor;
	uint32_t		filter_cmds[CMDMASK_WORDS];
	const char		*filter_devname;
	bool			no_banner;
	const char		*cmd;
	const char		*param;
	char			**argp;
	unsigned int		argc;
	bool			ioctl_fallback;
	bool			wildcard_unsupported;
};

struct attr_tb_info {
	const struct nlattr **tb;
	unsigned int max_type;
};

#define DECLARE_ATTR_TB_INFO(tbl) \
	struct attr_tb_info tbl ## _info = { (tbl), (MNL_ARRAY_SIZE(tbl) - 1) }

int nomsg_reply_cb(const struct nlmsghdr *nlhdr, void *data);
int attr_cb(const struct nlattr *attr, void *data);

int netlink_init(struct cmd_context *ctx);
bool netlink_cmd_check(struct cmd_context *ctx, unsigned int cmd,
		       bool allow_wildcard);
const char *get_dev_name(const struct nlattr *nest);
int get_dev_info(const struct nlattr *nest, int *ifindex, char *ifname);
u32 get_stats_flag(struct nl_context *nlctx, unsigned int nlcmd,
		   unsigned int hdrattr);

int linkmodes_reply_cb(const struct nlmsghdr *nlhdr, void *data);
int linkinfo_reply_cb(const struct nlmsghdr *nlhdr, void *data);
int wol_reply_cb(const struct nlmsghdr *nlhdr, void *data);
int debug_reply_cb(const struct nlmsghdr *nlhdr, void *data);
int features_reply_cb(const struct nlmsghdr *nlhdr, void *data);
int privflags_reply_cb(const struct nlmsghdr *nlhdr, void *data);
int rings_reply_cb(const struct nlmsghdr *nlhdr, void *data);
int channels_reply_cb(const struct nlmsghdr *nlhdr, void *data);
int coalesce_reply_cb(const struct nlmsghdr *nlhdr, void *data);
int pause_reply_cb(const struct nlmsghdr *nlhdr, void *data);
int eee_reply_cb(const struct nlmsghdr *nlhdr, void *data);
int cable_test_reply_cb(const struct nlmsghdr *nlhdr, void *data);
int cable_test_ntf_cb(const struct nlmsghdr *nlhdr, void *data);
int cable_test_tdr_reply_cb(const struct nlmsghdr *nlhdr, void *data);
int cable_test_tdr_ntf_cb(const struct nlmsghdr *nlhdr, void *data);
int fec_reply_cb(const struct nlmsghdr *nlhdr, void *data);
int module_reply_cb(const struct nlmsghdr *nlhdr, void *data);

/* dump helpers */

int dump_link_modes(struct nl_context *nlctx, const struct nlattr *bitset,
		    bool mask, unsigned int class, const char *before,
		    const char *between, const char *after,
		    const char *if_none);

static inline void show_u32(const char *key,
			    const char *fmt,
			    const struct nlattr *attr)
{
	if (is_json_context()) {
		if (attr)
			print_uint(PRINT_JSON, key, NULL,
				   mnl_attr_get_u32(attr));
	} else {
		if (attr)
			printf("%s%u\n", fmt, mnl_attr_get_u32(attr));
		else
			printf("%sn/a\n", fmt);
	}
}

static inline const char *u8_to_bool(const uint8_t *val)
{
	if (val)
		return *val ? "on" : "off";
	else
		return "n/a";
}

static inline void show_bool_val(const char *key, const char *fmt, uint8_t *val)
{
	if (is_json_context()) {
		if (val)
			print_bool(PRINT_JSON, key, NULL, *val);
	} else {
		print_string(PRINT_FP, NULL, fmt, u8_to_bool(val));
	}
}

static inline void show_bool(const char *key, const char *fmt,
			     const struct nlattr *attr)
{
	show_bool_val(key, fmt, attr ? mnl_attr_get_payload(attr) : NULL);
}

static inline void show_cr(void)
{
	if (!is_json_context())
		putchar('\n');
}

/* misc */

static inline void copy_devname(char *dst, const char *src)
{
	strncpy(dst, src, ALTIFNAMSIZ);
	dst[ALTIFNAMSIZ - 1] = '\0';
}

static inline bool dev_ok(const struct nl_context *nlctx)
{
	return !nlctx->filter_devname ||
	       (nlctx->devname &&
		!strcmp(nlctx->devname, nlctx->filter_devname));
}

static inline int netlink_init_ethnl2_socket(struct nl_context *nlctx)
{
	if (nlctx->ethnl2_socket)
		return 0;
	return nlsock_init(nlctx, &nlctx->ethnl2_socket, NETLINK_GENERIC);
}

static inline int netlink_init_rtnl_socket(struct nl_context *nlctx)
{
	if (nlctx->rtnl_socket)
		return 0;
	return nlsock_init(nlctx, &nlctx->rtnl_socket, NETLINK_ROUTE);
}

#endif /* ETHTOOL_NETLINK_INT_H__ */
