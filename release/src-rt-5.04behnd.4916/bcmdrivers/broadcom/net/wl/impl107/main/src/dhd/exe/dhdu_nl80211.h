/*
 * Definitions for DHD nl80211 driver interface.
 *
 * Copyright (C) 2025, Broadcom. All Rights Reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 *
 * <<Broadcom-WL-IPTag/Open:>>
 *
 * $Id: dhdu_nl80211.h 821234 2023-02-06 14:16:52Z $
 */

#ifndef DHDU_NL80211_H_
#define DHDU_NL80211_H_

#ifdef NL80211

#include <netlink/genl/genl.h>
#include <netlink/genl/family.h>
#include <netlink/genl/ctrl.h>

/* libnl 1.x compatibility code */
#if !defined(CONFIG_LIBNL20) && !defined(CONFIG_LIBNL30)
#define nl_sock		nl_handle
#endif

struct dhd_netlink_info
{
	struct nl_sock *nl;
	struct nl_cb *cb;
	int nl_id;
	int ifidx;
};

int dhd_nl_sock_connect(struct dhd_netlink_info *dhd_nli);
void dhd_nl_sock_disconnect(struct dhd_netlink_info *dhd_nli);
int dhd_nl_do_vndr_cmd(struct dhd_netlink_info *dhd_nli, dhd_ioctl_t *ioc);

#endif /* NL80211 */

#endif /* DHDU_NL80211_H_ */
