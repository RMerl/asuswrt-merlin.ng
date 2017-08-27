/*
 * netlink/netfilter/log.h	Netfilter Log
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2003-2006 Thomas Graf <tgraf@suug.ch>
 * Copyright (c) 2007 Philip Craig <philipc@snapgear.com>
 * Copyright (c) 2007 Secure Computing Corporation
 * Copyright (c) 2008 Patrick McHardy <kaber@trash.net>
 */

#ifndef NETLINK_LOG_H_
#define NETLINK_LOG_H_

#include <netlink/netlink.h>

#ifdef __cplusplus
extern "C" {
#endif

struct nl_sock;
struct nlmsghdr;
struct nfnl_log;

extern struct nl_object_ops log_obj_ops;

enum nfnl_log_copy_mode {
	NFNL_LOG_COPY_NONE,
	NFNL_LOG_COPY_META,
	NFNL_LOG_COPY_PACKET,
};

enum nfnl_log_flags {
	NFNL_LOG_FLAG_SEQ		= 0x1,
	NFNL_LOG_FLAG_SEQ_GLOBAL	= 0x2,
};

/* General */
extern struct nfnl_log *	nfnl_log_alloc(void);
extern int			nfnlmsg_log_parse(struct nlmsghdr *,
						  struct nfnl_log **);

extern void			nfnl_log_get(struct nfnl_log *);
extern void			nfnl_log_put(struct nfnl_log *);

/* Attributes */
extern void			nfnl_log_set_group(struct nfnl_log *, uint16_t);
extern int			nfnl_log_test_group(const struct nfnl_log *);
extern uint16_t			nfnl_log_get_group(const struct nfnl_log *);

extern void			nfnl_log_set_copy_mode(struct nfnl_log *,
						       enum nfnl_log_copy_mode);
extern int			nfnl_log_test_copy_mode(const struct nfnl_log *);
extern enum nfnl_log_copy_mode	nfnl_log_get_copy_mode(const struct nfnl_log *);

extern char *			nfnl_log_copy_mode2str(enum nfnl_log_copy_mode,
						       char *, size_t);
extern enum nfnl_log_copy_mode	nfnl_log_str2copy_mode(const char *);

extern void			nfnl_log_set_copy_range(struct nfnl_log *, uint32_t);
extern int			nfnl_log_test_copy_range(const struct nfnl_log *);
extern uint32_t			nfnl_log_get_copy_range(const struct nfnl_log *);

extern void			nfnl_log_set_flush_timeout(struct nfnl_log *, uint32_t);
extern int			nfnl_log_test_flush_timeout(const struct nfnl_log *);
extern uint32_t			nfnl_log_get_flush_timeout(const struct nfnl_log *);

extern void			nfnl_log_set_alloc_size(struct nfnl_log *, uint32_t);
extern int			nfnl_log_test_alloc_size(const struct nfnl_log *);
extern uint32_t			nfnl_log_get_alloc_size(const struct nfnl_log *);

extern void			nfnl_log_set_queue_threshold(struct nfnl_log *, uint32_t);
extern int			nfnl_log_test_queue_threshold(const struct nfnl_log *);
extern uint32_t			nfnl_log_get_queue_threshold(const struct nfnl_log *);

extern void			nfnl_log_set_flags(struct nfnl_log *, unsigned int);
extern void			nfnl_log_unset_flags(struct nfnl_log *, unsigned int);
extern unsigned int		nfnl_log_get_flags(const struct nfnl_log *);

extern char *			nfnl_log_flags2str(unsigned int, char *, size_t);
extern unsigned int		nfnl_log_str2flags(const char *);

extern int	nfnl_log_build_pf_bind(uint8_t, struct nl_msg **);
extern int	nfnl_log_pf_bind(struct nl_sock *, uint8_t);

extern int	nfnl_log_build_pf_unbind(uint8_t, struct nl_msg **);
extern int	nfnl_log_pf_unbind(struct nl_sock *, uint8_t);

extern int	nfnl_log_build_create_request(const struct nfnl_log *,
					      struct nl_msg **);
extern int	nfnl_log_create(struct nl_sock *, const struct nfnl_log *);

extern int	nfnl_log_build_change_request(const struct nfnl_log *,
					      struct nl_msg **);
extern int	nfnl_log_change(struct nl_sock *, const struct nfnl_log *);

extern int	nfnl_log_build_delete_request(const struct nfnl_log *,
					      struct nl_msg **);
extern int	nfnl_log_delete(struct nl_sock *, const struct nfnl_log *);

#ifdef __cplusplus
}
#endif

#endif

