/*
 * netlink/route/cls/ematch/text.h	Text Search
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2010 Thomas Graf <tgraf@suug.ch>
 */

#ifndef NETLINK_CLS_EMATCH_TEXT_H_
#define NETLINK_CLS_EMATCH_TEXT_H_

#include <netlink/netlink.h>
#include <netlink/route/cls/ematch.h>
#include <linux/tc_ematch/tc_em_text.h>

#ifdef __cplusplus
extern "C" {
#endif

extern void	rtnl_ematch_text_set_from(struct rtnl_ematch *,
					  uint8_t, uint16_t);
extern uint16_t	rtnl_ematch_text_get_from_offset(struct rtnl_ematch *);
extern uint8_t	rtnl_ematch_text_get_from_layer(struct rtnl_ematch *);
extern void	rtnl_ematch_text_set_to(struct rtnl_ematch *,
					uint8_t, uint16_t);
extern uint16_t	rtnl_ematch_text_get_to_offset(struct rtnl_ematch *);
extern uint8_t	rtnl_ematch_text_get_to_layer(struct rtnl_ematch *);
extern void	rtnl_ematch_text_set_pattern(struct rtnl_ematch *,
					     char *, size_t);
extern char *	rtnl_ematch_text_get_pattern(struct rtnl_ematch *);
extern size_t	rtnl_ematch_text_get_len(struct rtnl_ematch *);
extern void	rtnl_ematch_text_set_algo(struct rtnl_ematch *, const char *);
extern char *	rtnl_ematch_text_get_algo(struct rtnl_ematch *);

#ifdef __cplusplus
}
#endif

#endif
