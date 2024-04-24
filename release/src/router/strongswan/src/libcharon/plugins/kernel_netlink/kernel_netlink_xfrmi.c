/*
 * Copyright (C) 2019-2023 Tobias Brunner
 *
 * Copyright (C) secunet Security Networks AG
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.  See <http://www.fsf.org/copyleft/gpl.txt>.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 */

#include <net/if.h>

#include "kernel_netlink_xfrmi.h"
#include "kernel_netlink_shared.h"

#ifndef IFLA_XFRM_MAX
enum {
	IFLA_XFRM_UNSPEC,
	IFLA_XFRM_LINK,
	IFLA_XFRM_IF_ID,
	__IFLA_XFRM_MAX
};
#define IFLA_XFRM_MAX (__IFLA_XFRM_MAX - 1)
#endif

typedef struct private_kernel_netlink_xfrmi_t private_kernel_netlink_xfrmi_t;

/**
 * Private data
 */
struct private_kernel_netlink_xfrmi_t {

	/**
	 * Public interface
	 */
	kernel_netlink_xfrmi_t public;

	/**
	 * Netlink socket
	 */
	netlink_socket_t *socket;
};

/**
 * "up" the interface with the given name
 */
static bool interface_up(private_kernel_netlink_xfrmi_t *this, char *name)
{
	netlink_buf_t request;
	struct nlmsghdr *hdr;
	struct ifinfomsg *msg;

	memset(&request, 0, sizeof(request));

	hdr = &request.hdr;
	hdr->nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK;
	hdr->nlmsg_type = RTM_SETLINK;
	hdr->nlmsg_len = NLMSG_LENGTH(sizeof(struct ifinfomsg));

	msg = NLMSG_DATA(hdr);
	msg->ifi_family = AF_UNSPEC;
	msg->ifi_change |= IFF_UP;
	msg->ifi_flags |= IFF_UP;

	netlink_add_attribute(hdr, IFLA_IFNAME, chunk_from_str(name),
						  sizeof(request));

	if (this->socket->send_ack(this->socket, hdr) != SUCCESS)
	{
		DBG1(DBG_KNL, "failed to bring up XFRM interface '%s'", name);
		return FALSE;
	}
	return TRUE;
}

METHOD(kernel_netlink_xfrmi_t, create, bool,
	private_kernel_netlink_xfrmi_t *this, char *name, uint32_t if_id,
	char *phys, uint32_t mtu)
{
	netlink_buf_t request;
	struct nlmsghdr *hdr;
	struct ifinfomsg *msg;
	struct rtattr *linkinfo, *info_data;
	uint32_t ifindex = 0;

	if (phys)
	{
		ifindex = if_nametoindex(phys);
		if (!ifindex)
		{
			DBG1(DBG_KNL, "physical interface '%s' not found", phys);
			return FALSE;
		}
	}

	memset(&request, 0, sizeof(request));

	hdr = &request.hdr;
	hdr->nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK | NLM_F_CREATE | NLM_F_EXCL;
	hdr->nlmsg_type = RTM_NEWLINK;
	hdr->nlmsg_len = NLMSG_LENGTH(sizeof(struct ifinfomsg));

	msg = NLMSG_DATA(hdr);
	msg->ifi_family = AF_UNSPEC;

	netlink_add_attribute(hdr, IFLA_IFNAME, chunk_from_str(name),
						  sizeof(request));
	if (mtu)
	{
		netlink_add_attribute(hdr, IFLA_MTU, chunk_from_thing(mtu),
							  sizeof(request));
	}

	linkinfo = netlink_nested_start(hdr, sizeof(request), IFLA_LINKINFO);

	netlink_add_attribute(hdr, IFLA_INFO_KIND, chunk_from_str("xfrm"),
						  sizeof(request));

	info_data = netlink_nested_start(hdr, sizeof(request), IFLA_INFO_DATA);

	netlink_add_attribute(hdr, IFLA_XFRM_IF_ID, chunk_from_thing(if_id),
						  sizeof(request));
	if (ifindex)
	{
		netlink_add_attribute(hdr, IFLA_XFRM_LINK, chunk_from_thing(ifindex),
							  sizeof(request));
	}

	netlink_nested_end(hdr, info_data);
	netlink_nested_end(hdr, linkinfo);

	switch (this->socket->send_ack(this->socket, hdr))
	{
		case SUCCESS:
			return interface_up(this, name);
		case ALREADY_DONE:
			DBG1(DBG_KNL, "XFRM interface '%s' already exists", name);
			break;
		default:
			DBG1(DBG_KNL, "failed to create XFRM interface '%s'", name);
			break;
	}
	return FALSE;
}

/** enumerator over XFRM interfaces */
typedef struct {
	/** public interface */
	enumerator_t public;
	/** message from the kernel */
	struct nlmsghdr *msg;
	/** current message from the kernel */
	struct nlmsghdr *current;
	/** remaining length */
	size_t len;
	/** current physical interface (if any) */
	char phys[IFNAMSIZ];
} interface_enumerator_t;

METHOD(enumerator_t, destroy_enumerator, void,
	interface_enumerator_t *this)
{
	free(this->msg);
	free(this);
}

/**
 * Parse attributes nested in IFLA_INFO_DATA
 */
static void parse_info_data(struct rtattr *rta, size_t rtasize, bool *has_phys,
							char *phys, uint32_t *if_id)
{
	uint32_t ifindex;

	while (RTA_OK(rta, rtasize))
	{
		switch (rta->rta_type)
		{
			case IFLA_XFRM_IF_ID:
				if (RTA_PAYLOAD(rta) == sizeof(*if_id))
				{
					*if_id = *(uint32_t*)RTA_DATA(rta);
				}
				break;
			case IFLA_XFRM_LINK:
				if (RTA_PAYLOAD(rta) == sizeof(ifindex))
				{
					ifindex = *(uint32_t*)RTA_DATA(rta);
					if (ifindex)
					{
						if_indextoname(ifindex, phys);
						*has_phys = TRUE;
					}
				}
				break;
			default:
				break;
		}
		rta = RTA_NEXT(rta, rtasize);
	}
}

/**
 * Parse attributes nested in IFLA_LINKINFO
 */
static void parse_linkinfo(struct rtattr *rta, size_t rtasize, bool *type_match,
						   bool *has_phys, char *phys, uint32_t *if_id)
{
	while (RTA_OK(rta, rtasize))
	{
		switch (rta->rta_type)
		{
			case IFLA_INFO_KIND:
				*type_match = streq("xfrm", RTA_DATA(rta));
				break;
			case IFLA_INFO_DATA:
				parse_info_data(RTA_DATA(rta), RTA_PAYLOAD(rta), has_phys,
								phys, if_id);
				break;
			default:
				break;
		}
		rta = RTA_NEXT(rta, rtasize);
	}
}

METHOD(enumerator_t, enumerate, bool,
	interface_enumerator_t *this, va_list args)
{
	char **name;
	uint32_t *if_id, *mtu;
	char **phys;

	VA_ARGS_VGET(args, name, if_id, phys, mtu);

	if (!this->current)
	{
		this->current = this->msg;
	}
	else
	{
		this->current = NLMSG_NEXT(this->current, this->len);
	}

	while (NLMSG_OK(this->current, this->len))
	{
		switch (this->current->nlmsg_type)
		{
			case NLMSG_DONE:
				break;
			case RTM_NEWLINK:
			{
				struct ifinfomsg *msg = NLMSG_DATA(this->current);
				struct rtattr *rta = IFLA_RTA(msg);
				size_t rtasize = IFLA_PAYLOAD(this->current);
				bool type_match = FALSE, has_phys = FALSE;

				*name = NULL;

				while (RTA_OK(rta, rtasize))
				{
					switch (rta->rta_type)
					{
						case IFLA_IFNAME:
							*name = RTA_DATA(rta);
							break;
						case IFLA_MTU:
							if (mtu && RTA_PAYLOAD(rta) == sizeof(*mtu))
							{
								*mtu = *(uint32_t*)RTA_DATA(rta);
							}
							break;
						case IFLA_LINKINFO:
							parse_linkinfo(RTA_DATA(rta), RTA_PAYLOAD(rta),
										   &type_match, &has_phys, this->phys,
										   if_id);
							break;
						default:
							break;
					}
					rta = RTA_NEXT(rta, rtasize);
				}
				if (*name && type_match)
				{
					if (phys)
					{
						*phys = has_phys ? this->phys : NULL;
					}
					return TRUE;
				}
				/* fall-through */
			}
			default:
				this->current = NLMSG_NEXT(this->current, this->len);
				continue;
		}
		break;
	}
	return FALSE;
}

METHOD(kernel_netlink_xfrmi_t, create_enumerator, enumerator_t*,
	private_kernel_netlink_xfrmi_t *this)
{
	netlink_buf_t request;
	struct nlmsghdr *hdr, *out;
	struct ifinfomsg *msg;
	struct rtattr *linkinfo;
	size_t len;
	interface_enumerator_t *enumerator;

	memset(&request, 0, sizeof(request));

	hdr = &request.hdr;
	hdr->nlmsg_flags = NLM_F_REQUEST | NLM_F_DUMP;
	hdr->nlmsg_type = RTM_GETLINK;
	hdr->nlmsg_len = NLMSG_LENGTH(sizeof(struct ifinfomsg));

	msg = NLMSG_DATA(hdr);
	msg->ifi_family = AF_UNSPEC;

	/* if the kernel doesn't know the type we set here, it will just return all
	 * interfaces, so we filter the type ourselves too in the callback */
	linkinfo = netlink_nested_start(hdr, sizeof(request), IFLA_LINKINFO);

	netlink_add_attribute(hdr, IFLA_INFO_KIND, chunk_from_str("xfrm"),
						  sizeof(request));

	netlink_nested_end(hdr, linkinfo);

	if (this->socket->send(this->socket, hdr, &out, &len) != SUCCESS)
	{
		DBG2(DBG_KNL, "enumerating XFRM interfaces failed");
		return enumerator_create_empty();
	}

	INIT(enumerator,
		.public = {
			.enumerate = enumerator_enumerate_default,
			.venumerate = _enumerate,
			.destroy = _destroy_enumerator,
		},
		.msg = out,
		.len = len,
	);
	return &enumerator->public;
}

METHOD(kernel_netlink_xfrmi_t, delete, bool,
	private_kernel_netlink_xfrmi_t *this, char *name)
{
	netlink_buf_t request;
	struct nlmsghdr *hdr;
	struct ifinfomsg *msg;
	struct rtattr *linkinfo;

	memset(&request, 0, sizeof(request));

	hdr = &request.hdr;
	hdr->nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK;
	hdr->nlmsg_type = RTM_DELLINK;
	hdr->nlmsg_len = NLMSG_LENGTH(sizeof(struct ifinfomsg));

	msg = NLMSG_DATA(hdr);
	msg->ifi_family = AF_UNSPEC;

	netlink_add_attribute(hdr, IFLA_IFNAME, chunk_from_str(name),
						  sizeof(request));

	/* the type doesn't seem to matter, but let's still set it */
	linkinfo = netlink_nested_start(hdr, sizeof(request), IFLA_LINKINFO);

	netlink_add_attribute(hdr, IFLA_INFO_KIND, chunk_from_str("xfrm"),
						  sizeof(request));

	netlink_nested_end(hdr, linkinfo);

	switch (this->socket->send_ack(this->socket, hdr))
	{
		case SUCCESS:
			return TRUE;
		case NOT_FOUND:
			DBG1(DBG_KNL, "XFRM interface '%s' not found to delete", name);
			break;
		default:
			DBG1(DBG_KNL, "failed to delete XFRM interface '%s'", name);
			break;
	}
	return FALSE;
}

void kernel_netlink_xfrmi_destroy(kernel_netlink_xfrmi_t *pub)
{
	private_kernel_netlink_xfrmi_t *this = (private_kernel_netlink_xfrmi_t*)pub;

	this->socket->destroy(this->socket);
	free(this);
}

/*
 * Described in header
 */
kernel_netlink_xfrmi_t *kernel_netlink_xfrmi_create(bool test)
{
	private_kernel_netlink_xfrmi_t *this;
	char name[IFNAMSIZ] = {};
	uint32_t if_id;

	INIT(this,
		.public = {
			.create = _create,
			.create_enumerator = _create_enumerator,
			.delete = _delete,
		},
		.socket = netlink_socket_create(NETLINK_ROUTE, NULL, FALSE),
	);

	if (!this->socket)
	{
		free(this);
		return NULL;
	}
	if (test)
	{
		/* try to create an XFRM interface to see if the kernel supports it, use
		 * a random ID and name for the test to avoid conflicts */
		if_id = random();
		snprintf(name, sizeof(name), "xfrmi-test-%u", if_id);

		if (!create(this, name, if_id, NULL, 0))
		{
			kernel_netlink_xfrmi_destroy(&this->public);
			return NULL;
		}
		DBG2(DBG_KNL, "XFRM interfaces supported by kernel");
		delete(this, name);
	}
	return &this->public;
}
