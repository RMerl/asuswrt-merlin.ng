/*
 * lib/route/link/inet.c	AF_INET link operations
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2010 Thomas Graf <tgraf@suug.ch>
 */

/**
 * @ingroup link_API
 * @defgroup link_inet IPv4 Link Module
 * @brief Implementation of IPv4 specific link attributes
 *
 *
 *
 * @par Example: Reading the value of IPV4_DEVCONF_FORWARDING
 * @code
 * struct nl_cache *cache;
 * struct rtnl_link *link;
 * uint32_t value;
 *
 * // Allocate a link cache
 * rtnl_link_alloc_cache(sock, AF_UNSPEC, &cache);
 *
 * // Search for the link we wish to see the value from
 * link = rtnl_link_get_by_name(cache, "eth0");
 *
 * // Read the value of the setting IPV4_DEVCONF_FORWARDING
 * if (rtnl_link_inet_get_conf(link, IPV4_DEVCONF_FORWARDING, &value) < 0)
 *         // Error: Unable to read config setting
 *
 * printf("forwarding is %s\n", value ? "enabled" : "disabled");
 * @endcode
 *
 * @par Example: Changing the value of IPV4_DEVCONF_FOWARDING
 * @code
 * //
 * // ... Continueing from the previous example ...
 * //
 *
 * struct rtnl_link *new;
 *
 * // Allocate a new link to store the changes we wish to make.
 * new = rtnl_link_alloc();
 *
 * // Set IPV4_DEVCONF_FORWARDING to '1'
 * rtnl_link_inet_set_conf(new, IPV4_DEVCONF_FORWARDING, 1);
 *
 * // Send the change request to the kernel.
 * rtnl_link_change(sock, link, new, 0);
 * @endcode
 *
 * @{
 */


#include <netlink-private/netlink.h>
#include <netlink/netlink.h>
#include <netlink/attr.h>
#include <netlink/route/rtnl.h>
#include <netlink-private/route/link/api.h>

/** @cond SKIP */
struct inet_data
{
	uint8_t			i_confset[IPV4_DEVCONF_MAX];
	uint32_t		i_conf[IPV4_DEVCONF_MAX];
};
/** @endcond */

static void *inet_alloc(struct rtnl_link *link)
{
	return calloc(1, sizeof(struct inet_data));
}

static void *inet_clone(struct rtnl_link *link, void *data)
{
	struct inet_data *id;

	if ((id = inet_alloc(link)))
		memcpy(id, data, sizeof(*id));

	return id;
}

static void inet_free(struct rtnl_link *link, void *data)
{
	free(data);
}

static struct nla_policy inet_policy[IFLA_INET6_MAX+1] = {
	[IFLA_INET_CONF]	= { .minlen = 4 },
};

static int inet_parse_af(struct rtnl_link *link, struct nlattr *attr, void *data)
{
	struct inet_data *id = data;
	struct nlattr *tb[IFLA_INET_MAX+1];
	int err;

	err = nla_parse_nested(tb, IFLA_INET_MAX, attr, inet_policy);
	if (err < 0)
		return err;
	if (tb[IFLA_INET_CONF] && nla_len(tb[IFLA_INET_CONF]) % 4)
		return -EINVAL;

	if (tb[IFLA_INET_CONF]) {
		int i;
		int len = min_t(int, IPV4_DEVCONF_MAX, nla_len(tb[IFLA_INET_CONF]) / 4);

		for (i = 0; i < len; i++)
			id->i_confset[i] = 1;
		nla_memcpy(&id->i_conf, tb[IFLA_INET_CONF], sizeof(id->i_conf));
	}

	return 0;
}

static int inet_fill_af(struct rtnl_link *link, struct nl_msg *msg, void *data)
{
	struct inet_data *id = data;
	struct nlattr *nla;
	int i;

	if (!(nla = nla_nest_start(msg, IFLA_INET_CONF)))
		return -NLE_MSGSIZE;

	for (i = 0; i < IPV4_DEVCONF_MAX; i++)
		if (id->i_confset[i])
			NLA_PUT_U32(msg, i+1, id->i_conf[i]);

	nla_nest_end(msg, nla);

	return 0;

nla_put_failure:
	return -NLE_MSGSIZE;
}

static const struct trans_tbl inet_devconf[] = {
	__ADD(IPV4_DEVCONF_FORWARDING, forwarding)
	__ADD(IPV4_DEVCONF_MC_FORWARDING, mc_forwarding)
	__ADD(IPV4_DEVCONF_PROXY_ARP, proxy_arp)
	__ADD(IPV4_DEVCONF_ACCEPT_REDIRECTS, accept_redirects)
	__ADD(IPV4_DEVCONF_SECURE_REDIRECTS, secure_redirects)
	__ADD(IPV4_DEVCONF_SEND_REDIRECTS, send_redirects)
	__ADD(IPV4_DEVCONF_SHARED_MEDIA, shared_media)
	__ADD(IPV4_DEVCONF_RP_FILTER, rp_filter)
	__ADD(IPV4_DEVCONF_ACCEPT_SOURCE_ROUTE, accept_source_route)
	__ADD(IPV4_DEVCONF_BOOTP_RELAY, bootp_relay)
	__ADD(IPV4_DEVCONF_LOG_MARTIANS, log_martians)
	__ADD(IPV4_DEVCONF_TAG, tag)
	__ADD(IPV4_DEVCONF_ARPFILTER, arpfilter)
	__ADD(IPV4_DEVCONF_MEDIUM_ID, medium_id)
	__ADD(IPV4_DEVCONF_NOXFRM, noxfrm)
	__ADD(IPV4_DEVCONF_NOPOLICY, nopolicy)
	__ADD(IPV4_DEVCONF_FORCE_IGMP_VERSION, force_igmp_version)
	__ADD(IPV4_DEVCONF_ARP_ANNOUNCE, arp_announce)
	__ADD(IPV4_DEVCONF_ARP_IGNORE, arp_ignore)
	__ADD(IPV4_DEVCONF_PROMOTE_SECONDARIES, promote_secondaries)
	__ADD(IPV4_DEVCONF_ARP_ACCEPT, arp_accept)
	__ADD(IPV4_DEVCONF_ARP_NOTIFY, arp_notify)
	__ADD(IPV4_DEVCONF_ACCEPT_LOCAL, accept_local)
	__ADD(IPV4_DEVCONF_SRC_VMARK, src_vmark)
	__ADD(IPV4_DEVCONF_PROXY_ARP_PVLAN, proxy_arp_pvlan)
	__ADD(IPV4_DEVCONF_ROUTE_LOCALNET, route_localnet)
	__ADD(IPV4_DEVCONF_IGMPV2_UNSOLICITED_REPORT_INTERVAL, igmpv2_unsolicited_report_interval)
	__ADD(IPV4_DEVCONF_IGMPV3_UNSOLICITED_REPORT_INTERVAL, igmpv3_unsolicited_report_interval)
};

const char *rtnl_link_inet_devconf2str(int type, char *buf, size_t len)
{
	return __type2str(type, buf, len, inet_devconf,
			  ARRAY_SIZE(inet_devconf));
}

int rtnl_link_inet_str2devconf(const char *name)
{
	return __str2type(name, inet_devconf, ARRAY_SIZE(inet_devconf));
}

static void inet_dump_details(struct rtnl_link *link,
			      struct nl_dump_params *p, void *data)
{
	struct inet_data *id = data;
	char buf[64];
	int i, n = 0;

	nl_dump_line(p, "    ipv4 devconf:\n");
	nl_dump_line(p, "      ");

	for (i = 0; i < IPV4_DEVCONF_MAX; i++) {
		nl_dump_line(p, "%-19s %3u",
			rtnl_link_inet_devconf2str(i+1, buf, sizeof(buf)),
			id->i_confset[i] ? id->i_conf[i] : 0);

		if (++n == 3) {
			nl_dump(p, "\n");
			nl_dump_line(p, "      ");
			n = 0;
		} else
			nl_dump(p, "  ");
	}

	if (n != 0)
		nl_dump(p, "\n");
}

static struct rtnl_link_af_ops inet_ops = {
	.ao_family			= AF_INET,
	.ao_alloc			= &inet_alloc,
	.ao_clone			= &inet_clone,
	.ao_free			= &inet_free,
	.ao_parse_af			= &inet_parse_af,
	.ao_fill_af			= &inet_fill_af,
	.ao_dump[NL_DUMP_DETAILS]	= &inet_dump_details,
};

/**
 * Get value of a ipv4 link configuration setting
 * @arg link		Link object
 * @arg cfgid		Configuration identifier
 * @arg res		Result pointer
 *
 * Stores the value of the specified configuration setting in the provided
 * result pointer.
 *
 * @return 0 on success or a negative error code.
 * @return -NLE_RANGE cfgid is out of range, 1..IPV4_DEVCONF_MAX
 * @return -NLE_NOATTR configuration setting not available
 * @return -NLE_INVAL cfgid not set. If the link was received via netlink,
 *                    it means that the cfgid is not supported.
 */
int rtnl_link_inet_get_conf(struct rtnl_link *link, const unsigned int cfgid,
			    uint32_t *res)
{
	struct inet_data *id;

	if (cfgid == 0 || cfgid > IPV4_DEVCONF_MAX)
		return -NLE_RANGE;

	if (!(id = rtnl_link_af_alloc(link, &inet_ops)))
		return -NLE_NOATTR;

	if (!id->i_confset[cfgid - 1])
		return -NLE_INVAL;
	*res = id->i_conf[cfgid - 1];

	return 0;
}

/**
 * Change value of a ipv4 link configuration setting
 * @arg link		Link object
 * @arg cfgid		Configuration identifier
 * @arg value		New value
 *
 * Changes the value in the per link ipv4 configuration array. 
 *
 * @return 0 on success or a negative error code.
 * @return -NLE_RANGE cfgid is out of range, 1..IPV4_DEVCONF_MAX
 * @return -NLE_NOMEM memory allocation failed
 */
int rtnl_link_inet_set_conf(struct rtnl_link *link, const unsigned int cfgid,
			    uint32_t value)
{
	struct inet_data *id;

	if (!(id = rtnl_link_af_alloc(link, &inet_ops)))
		return -NLE_NOMEM;

	if (cfgid == 0 || cfgid > IPV4_DEVCONF_MAX)
		return -NLE_RANGE;

	id->i_confset[cfgid - 1] = 1;
	id->i_conf[cfgid - 1] = value;

	return 0;
}


static void __init inet_init(void)
{
	rtnl_link_af_register(&inet_ops);
}

static void __exit inet_exit(void)
{
	rtnl_link_af_unregister(&inet_ops);
}

/** @} */
