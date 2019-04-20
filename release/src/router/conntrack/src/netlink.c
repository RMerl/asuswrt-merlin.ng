/*
 * (C) 2006-2011 by Pablo Neira Ayuso <pablo@netfilter.org>
 * (C) 2011 by Vyatta Inc. <http://www.vyatta.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include "netlink.h"
#include "conntrackd.h"
#include "filter.h"
#include "log.h"

#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <libnetfilter_conntrack/libnetfilter_conntrack_tcp.h>

struct nfct_handle *nl_init_event_handler(void)
{
	struct nfct_handle *h;

	h = nfct_open(CONFIG(netlink).subsys_id, CONFIG(netlink).groups);
	if (h == NULL)
		return NULL;

	if (CONFIG(netlink).events_reliable) {
		int on = 1;

		setsockopt(nfct_fd(h), SOL_NETLINK,
			   NETLINK_BROADCAST_SEND_ERROR, &on, sizeof(int));

		setsockopt(nfct_fd(h), SOL_NETLINK,
			   NETLINK_NO_ENOBUFS, &on, sizeof(int));

		dlog(LOG_NOTICE, "reliable ctnetlink event delivery "
				 "is ENABLED.");
	}

	if (STATE(filter)) {
		if (CONFIG(filter_from_kernelspace)) {
			if (nfct_filter_attach(nfct_fd(h),
					       STATE(filter)) == -1) {
				dlog(LOG_ERR, "cannot set event filtering: %s",
				     strerror(errno));
			}
			dlog(LOG_NOTICE, "using kernel-space event filtering");
		} else
			dlog(LOG_NOTICE, "using user-space event filtering");

		nfct_filter_destroy(STATE(filter));
	}

	fcntl(nfct_fd(h), F_SETFL, O_NONBLOCK);

	/* set up socket buffer size */
	if (CONFIG(netlink_buffer_size) &&
	    CONFIG(netlink_buffer_size) <=
			CONFIG(netlink_buffer_size_max_grown)) {
		/* we divide netlink_buffer_size by 2 here since value passed
		   to kernel gets doubled in SO_RCVBUF; see net/core/sock.c */
		CONFIG(netlink_buffer_size) =
		  nfnl_rcvbufsiz(nfct_nfnlh(h), CONFIG(netlink_buffer_size)/2);
	} else {
		dlog(LOG_NOTICE, "NetlinkBufferSize is either not set or "
				 "is greater than NetlinkBufferSizeMaxGrowth. "
				 "Using current system buffer size");

		socklen_t socklen = sizeof(unsigned int);
		unsigned int read_size;

		/* get current buffer size */
		getsockopt(nfct_fd(h), SOL_SOCKET,
			   SO_RCVBUF, &read_size, &socklen);

		CONFIG(netlink_buffer_size) = read_size;
	}

	dlog(LOG_NOTICE, "netlink event socket buffer size has been set "
			 "to %u bytes", CONFIG(netlink_buffer_size));

	return h;
}

struct nlif_handle *nl_init_interface_handler(void)
{
	struct nlif_handle *h;
	h = nlif_open();
	if (h == NULL)
		return NULL;

	if (nlif_query(h) == -1) {
		free(h);
		return NULL;
	}
	fcntl(nlif_fd(h), F_SETFL, O_NONBLOCK);

	return h;
}

static int warned = 0;

void nl_resize_socket_buffer(struct nfct_handle *h)
{
	unsigned int s = CONFIG(netlink_buffer_size);

	/* already warned that we have reached the maximum buffer size */
	if (warned)
		return;

	/* since sock_setsockopt in net/core/sock.c doubles the size of socket
	   buffer passed to it using nfnl_rcvbufsiz, only call nfnl_rcvbufsiz
	   if new value is not greater than netlink_buffer_size_max_grown */
	if (s*2 > CONFIG(netlink_buffer_size_max_grown)) {
		dlog(LOG_WARNING,
		     "netlink event socket buffer size cannot "
		     "be doubled further since it will exceed "
		     "NetlinkBufferSizeMaxGrowth. We are likely to "
		     "be losing events, this may lead to "
		     "unsynchronized replicas. Please, consider "
		     "increasing netlink socket buffer size via "
		     "NetlinkBufferSize and "
		     "NetlinkBufferSizeMaxGrowth clauses in "
		     "conntrackd.conf");
		warned = 1;
		return;
	}

	CONFIG(netlink_buffer_size) = nfnl_rcvbufsiz(nfct_nfnlh(h), s);

	/* notify the sysadmin */
	dlog(LOG_NOTICE, "netlink event socket buffer size has been doubled "
			 "to %u bytes", CONFIG(netlink_buffer_size));
}

static const int family = AF_UNSPEC;

int nl_dump_conntrack_table(struct nfct_handle *h)
{
	return nfct_query(h, NFCT_Q_DUMP, &family);
}

static int
nl_flush_selective_cb(enum nf_conntrack_msg_type type,
		      struct nf_conntrack *ct, void *data)
{
	/* don't delete this conntrack, it's in the ignore filter */
	if (ct_filter_conntrack(ct, 1))
		return NFCT_CB_CONTINUE;

	switch(type) {
	case NFCT_T_UPDATE:
		nl_destroy_conntrack(STATE(flush), ct);
		break;
	default:
		STATE(stats).nl_dump_unknown_type++;
		break;
	}
	return NFCT_CB_CONTINUE;
}

int nl_flush_conntrack_table_selective(void)
{
	struct nfct_handle *h;
	int ret;

	h = nfct_open(CONNTRACK, 0);
	if (h == NULL) {
		dlog(LOG_ERR, "cannot open handle");
		return -1;
	}
	nfct_callback_register(h, NFCT_T_ALL, nl_flush_selective_cb, NULL);

	ret = nfct_query(h, NFCT_Q_DUMP, &family);

	nfct_close(h);

	return ret;
}

int nl_send_resync(struct nfct_handle *h)
{
	return nfct_send(h, NFCT_Q_DUMP, &family);
}

/* if the handle has no callback, check for existence, otherwise, update */
int nl_get_conntrack(struct nfct_handle *h, const struct nf_conntrack *ct)
{
	int ret = 1;
	struct nf_conntrack *tmp;

	tmp = nfct_new();
	if (tmp == NULL)
		return -1;

	/* use the original tuple to check if it is there */
	nfct_copy(tmp, ct, NFCT_CP_ORIG);

	if (nfct_query(h, NFCT_Q_GET, tmp) == -1)
		ret = (errno == ENOENT) ? 0 : -1;

	nfct_destroy(tmp);
	return ret;
}

static void ctd_force_tcp_be_liberal(struct nf_conntrack *ct)
{
	int attrs[4] = { ATTR_TCP_FLAGS_ORIG, ATTR_TCP_MASK_ORIG,
			 ATTR_TCP_FLAGS_REPL, ATTR_TCP_MASK_REPL };
	unsigned int i;
	uint8_t flags;

	for (i = 0; i < ARRAY_SIZE(attrs); i++) {
		flags = nfct_get_attr_u8(ct, attrs[i]);
		nfct_set_attr_u8(ct, attrs[i],
				 flags | IP_CT_TCP_FLAG_BE_LIBERAL);
	}
}

int nl_create_conntrack(struct nfct_handle *h, 
			const struct nf_conntrack *orig,
			int timeout)
{
	struct nf_conntrack *ct;
	int ret;

	ct = nfct_clone(orig);
	if (ct == NULL)
		return -1;

	if (timeout > 0)
		nfct_set_attr_u32(ct, ATTR_TIMEOUT, timeout);

	/* we hit error if we try to change the expected bit */
	if (nfct_attr_is_set(ct, ATTR_STATUS)) {
		uint32_t status = nfct_get_attr_u32(ct, ATTR_STATUS);
		status &= ~IPS_EXPECTED;
		nfct_set_attr_u32(ct, ATTR_STATUS, status);
	}

	nfct_setobjopt(ct, NFCT_SOPT_SETUP_REPLY);

	/* disable TCP window tracking for recovered connections if required */
	if (!CONFIG(sync).tcp_window_tracking)
		ctd_force_tcp_be_liberal(ct);

	ret = nfct_query(h, NFCT_Q_CREATE, ct);
	nfct_destroy(ct);

	return ret;
}

int nl_update_conntrack(struct nfct_handle *h,
			const struct nf_conntrack *orig,
			int timeout)
{
	int ret;
	struct nf_conntrack *ct;

	ct = nfct_clone(orig);
	if (ct == NULL)
		return -1;

	if (timeout > 0)
		nfct_set_attr_u32(ct, ATTR_TIMEOUT, timeout);

	/* unset NAT info, otherwise we hit error */
	nfct_attr_unset(ct, ATTR_SNAT_IPV4);
	nfct_attr_unset(ct, ATTR_DNAT_IPV4);
	nfct_attr_unset(ct, ATTR_SNAT_PORT);
	nfct_attr_unset(ct, ATTR_DNAT_PORT);

	if (nfct_attr_is_set(ct, ATTR_STATUS)) {
		uint32_t status = nfct_get_attr_u32(ct, ATTR_STATUS);
		status &= ~IPS_NAT_MASK;
		nfct_set_attr_u32(ct, ATTR_STATUS, status);
	}
	/* we have to unset the helper to avoid EBUSY in reset timers */
	if (nfct_attr_is_set(ct, ATTR_HELPER_NAME))
		nfct_attr_unset(ct, ATTR_HELPER_NAME);

	/* we hit error if we try to update the master conntrack */
	if (ct_is_related(ct)) {
		nfct_attr_unset(ct, ATTR_MASTER_L3PROTO);
		nfct_attr_unset(ct, ATTR_MASTER_L4PROTO);
		nfct_attr_unset(ct, ATTR_MASTER_IPV4_SRC);
		nfct_attr_unset(ct, ATTR_MASTER_IPV4_DST);
		nfct_attr_unset(ct, ATTR_MASTER_IPV6_SRC);
		nfct_attr_unset(ct, ATTR_MASTER_IPV6_DST);
		nfct_attr_unset(ct, ATTR_MASTER_PORT_SRC);
		nfct_attr_unset(ct, ATTR_MASTER_PORT_DST);
	}

	/* disable TCP window tracking for recovered connections if required */
	if (!CONFIG(sync).tcp_window_tracking)
		ctd_force_tcp_be_liberal(ct);

	ret = nfct_query(h, NFCT_Q_UPDATE, ct);
	nfct_destroy(ct);

	return ret;
}

int nl_destroy_conntrack(struct nfct_handle *h, const struct nf_conntrack *ct)
{
	return nfct_query(h, NFCT_Q_DESTROY, ct);
}

int nl_create_expect(struct nfct_handle *h, const struct nf_expect *orig,
		     int timeout)
{
	int ret;
	struct nf_expect *exp;

	exp = nfexp_clone(orig);
	if (exp == NULL)
		return -1;

	if (timeout > 0)
		nfexp_set_attr_u32(exp, ATTR_EXP_TIMEOUT, timeout);

	ret = nfexp_query(h, NFCT_Q_CREATE, exp);
	nfexp_destroy(exp);

	return ret;
}

int nl_destroy_expect(struct nfct_handle *h, const struct nf_expect *exp)
{
	return nfexp_query(h, NFCT_Q_DESTROY, exp);
}

/* if the handle has no callback, check for existence, otherwise, update */
int nl_get_expect(struct nfct_handle *h, const struct nf_expect *exp)
{
	int ret = 1;
	struct nf_expect *tmp;

	/* XXX: we only need the expectation, not the mask and the master. */
	tmp = nfexp_clone(exp);
	if (tmp == NULL)
		return -1;

	if (nfexp_query(h, NFCT_Q_GET, tmp) == -1)
		ret = (errno == ENOENT) ? 0 : -1;

	nfexp_destroy(tmp);
	return ret;
}

int nl_dump_expect_table(struct nfct_handle *h)
{
	return nfexp_query(h, NFCT_Q_DUMP, &family);
}

int nl_flush_expect_table(struct nfct_handle *h)
{
	return nfexp_query(h, NFCT_Q_FLUSH, &family);
}

int nl_send_expect_resync(struct nfct_handle *h)
{
	return nfexp_send(h, NFCT_Q_DUMP, &family);
}
