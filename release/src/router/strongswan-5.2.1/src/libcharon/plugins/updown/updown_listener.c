/*
 * Copyright (C) 2013 Tobias Brunner
 * Copyright (C) 2008 Martin Willi
 * Hochschule fuer Technik Rapperswil
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

#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>

#include "updown_listener.h"

#include <utils/process.h>
#include <hydra.h>
#include <daemon.h>
#include <config/child_cfg.h>

typedef struct private_updown_listener_t private_updown_listener_t;

/**
 * Private data of an updown_listener_t object.
 */
struct private_updown_listener_t {

	/**
	 * Public updown_listener_t interface.
	 */
	updown_listener_t public;

	/**
	 * List of cached interface names
	 */
	linked_list_t *iface_cache;

	/**
	 * DNS attribute handler
	 */
	updown_handler_t *handler;
};

typedef struct cache_entry_t cache_entry_t;

/**
 * Cache line in the interface name cache.
 */
struct cache_entry_t {
	/** requid of the CHILD_SA */
	u_int32_t reqid;
	/** cached interface name */
	char *iface;
};

/**
 * Insert an interface name to the cache
 */
static void cache_iface(private_updown_listener_t *this, u_int32_t reqid,
						char *iface)
{
	cache_entry_t *entry = malloc_thing(cache_entry_t);

	entry->reqid = reqid;
	entry->iface = strdup(iface);

	this->iface_cache->insert_first(this->iface_cache, entry);
}

/**
 * Remove a cached interface name and return it.
 */
static char* uncache_iface(private_updown_listener_t *this, u_int32_t reqid)
{
	enumerator_t *enumerator;
	cache_entry_t *entry;
	char *iface = NULL;

	enumerator = this->iface_cache->create_enumerator(this->iface_cache);
	while (enumerator->enumerate(enumerator, &entry))
	{
		if (entry->reqid == reqid)
		{
			this->iface_cache->remove_at(this->iface_cache, enumerator);
			iface = entry->iface;
			free(entry);
			break;
		}
	}
	enumerator->destroy(enumerator);
	return iface;
}

/**
 * Allocate and push a format string to the environment
 */
static bool push_env(char *envp[], u_int count, char *fmt, ...)
{
	int i = 0;
	char *str;
	va_list args;

	while (envp[i])
	{
		if (++i + 1 >= count)
		{
			return FALSE;
		}
	}
	va_start(args, fmt);
	if (vasprintf(&str, fmt, args) >= 0)
	{
		envp[i] = str;
	}
	va_end(args);
	return envp[i] != NULL;
}

/**
 * Free all allocated environment strings
 */
static void free_env(char *envp[])
{
	int i;

	for (i = 0; envp[i]; i++)
	{
		free(envp[i]);
	}
}

/**
 * Push variables for handled DNS attributes
 */
static void push_dns_env(private_updown_listener_t *this, ike_sa_t *ike_sa,
						 char *envp[], u_int count)
{
	enumerator_t *enumerator;
	host_t *host;
	int v4 = 0, v6 = 0;

	if (this->handler)
	{
		enumerator = this->handler->create_dns_enumerator(this->handler,
											ike_sa->get_unique_id(ike_sa));
		while (enumerator->enumerate(enumerator, &host))
		{
			switch (host->get_family(host))
			{
				case AF_INET:
					push_env(envp, count, "PLUTO_DNS4_%d=%H", ++v4, host);
					break;
				case AF_INET6:
					push_env(envp, count, "PLUTO_DNS6_%d=%H", ++v6, host);
					break;
				default:
					continue;
			}
		}
		enumerator->destroy(enumerator);
	}
}

/**
 * Push variables for local virtual IPs
 */
static void push_vip_env(private_updown_listener_t *this, ike_sa_t *ike_sa,
						 char *envp[], u_int count)
{
	enumerator_t *enumerator;
	host_t *host;
	int v4 = 0, v6 = 0;
	bool first = TRUE;

	enumerator = ike_sa->create_virtual_ip_enumerator(ike_sa, TRUE);
	while (enumerator->enumerate(enumerator, &host))
	{
		if (first)
		{	/* legacy variable for first VIP */
			first = FALSE;
			push_env(envp, count, "PLUTO_MY_SOURCEIP=%H", host);
		}
		switch (host->get_family(host))
		{
			case AF_INET:
				push_env(envp, count, "PLUTO_MY_SOURCEIP4_%d=%H", ++v4, host);
				break;
			case AF_INET6:
				push_env(envp, count, "PLUTO_MY_SOURCEIP6_%d=%H", ++v6, host);
				break;
			default:
				continue;
		}
	}
	enumerator->destroy(enumerator);
}

/**
 * Determine proper values for port env variable
 */
static u_int16_t get_port(traffic_selector_t *me,
						  traffic_selector_t *other, bool local)
{
	switch (max(me->get_protocol(me), other->get_protocol(other)))
	{
		case IPPROTO_ICMP:
		case IPPROTO_ICMPV6:
		{
			u_int16_t port = me->get_from_port(me);

			port = max(port, other->get_from_port(other));
			return local ? traffic_selector_icmp_type(port)
						 : traffic_selector_icmp_code(port);
		}
	}
	return local ? me->get_from_port(me) : other->get_from_port(other);
}

/**
 * Invoke the updown script once for given traffic selectors
 */
static void invoke_once(private_updown_listener_t *this, ike_sa_t *ike_sa,
						child_sa_t *child_sa, child_cfg_t *config, bool up,
						traffic_selector_t *my_ts, traffic_selector_t *other_ts)
{
	host_t *me, *other, *host;
	char *iface;
	u_int8_t mask;
	mark_t mark;
	bool is_host, is_ipv6;
	int out;
	FILE *shell;
	process_t *process;
	char *envp[128] = {};

	me = ike_sa->get_my_host(ike_sa);
	other = ike_sa->get_other_host(ike_sa);

	push_env(envp, countof(envp), "PLUTO_VERSION=1.1");
	is_host = my_ts->is_host(my_ts, me);
	if (is_host)
	{
		is_ipv6 = me->get_family(me) == AF_INET6;
	}
	else
	{
		is_ipv6 = my_ts->get_type(my_ts) == TS_IPV6_ADDR_RANGE;
	}
	push_env(envp, countof(envp), "PLUTO_VERB=%s%s%s",
			 up ? "up" : "down",
			 is_host ? "-host" : "-client",
			 is_ipv6 ? "-v6" : "");
	push_env(envp, countof(envp), "PLUTO_CONNECTION=%s",
			 config->get_name(config));
	if (up)
	{
		if (hydra->kernel_interface->get_interface(hydra->kernel_interface,
												   me, &iface))
		{
			cache_iface(this, child_sa->get_reqid(child_sa), iface);
		}
		else
		{
			iface = NULL;
		}
	}
	else
	{
		iface = uncache_iface(this, child_sa->get_reqid(child_sa));
	}
	push_env(envp, countof(envp), "PLUTO_INTERFACE=%s",
			 iface ? iface : "unknown");
	push_env(envp, countof(envp), "PLUTO_REQID=%u",
			 child_sa->get_reqid(child_sa));
	push_env(envp, countof(envp), "PLUTO_PROTO=%s",
			 child_sa->get_protocol(child_sa) == PROTO_ESP ? "esp" : "ah");
	push_env(envp, countof(envp), "PLUTO_UNIQUEID=%u",
			 ike_sa->get_unique_id(ike_sa));
	push_env(envp, countof(envp), "PLUTO_ME=%H", me);
	push_env(envp, countof(envp), "PLUTO_MY_ID=%Y", ike_sa->get_my_id(ike_sa));
	if (my_ts->to_subnet(my_ts, &host, &mask))
	{
		push_env(envp, countof(envp), "PLUTO_MY_CLIENT=%+H/%u", host, mask);
		host->destroy(host);
	}
	push_env(envp, countof(envp), "PLUTO_MY_PORT=%u",
			 get_port(my_ts, other_ts, TRUE));
	push_env(envp, countof(envp), "PLUTO_MY_PROTOCOL=%u",
			 my_ts->get_protocol(my_ts));
	push_env(envp, countof(envp), "PLUTO_PEER=%H", other);
	push_env(envp, countof(envp), "PLUTO_PEER_ID=%Y",
			 ike_sa->get_other_id(ike_sa));
	if (other_ts->to_subnet(other_ts, &host, &mask))
	{
		push_env(envp, countof(envp), "PLUTO_PEER_CLIENT=%+H/%u", host, mask);
		host->destroy(host);
	}
	push_env(envp, countof(envp), "PLUTO_PEER_PORT=%u",
			 get_port(my_ts, other_ts, FALSE));
	push_env(envp, countof(envp), "PLUTO_PEER_PROTOCOL=%u",
			 other_ts->get_protocol(other_ts));
	if (ike_sa->has_condition(ike_sa, COND_EAP_AUTHENTICATED) ||
		ike_sa->has_condition(ike_sa, COND_XAUTH_AUTHENTICATED))
	{
		push_env(envp, countof(envp), "PLUTO_XAUTH_ID=%Y",
				 ike_sa->get_other_eap_id(ike_sa));
	}
	push_vip_env(this, ike_sa, envp, countof(envp));
	mark = config->get_mark(config, TRUE);
	if (mark.value)
	{
		push_env(envp, countof(envp), "PLUTO_MARK_IN=%u/0x%08x",
				 mark.value, mark.mask);
	}
	mark = config->get_mark(config, FALSE);
	if (mark.value)
	{
		push_env(envp, countof(envp), "PLUTO_MARK_OUT=%u/0x%08x",
				 mark.value, mark.mask);
	}
	if (ike_sa->has_condition(ike_sa, COND_NAT_ANY))
	{
		push_env(envp, countof(envp), "PLUTO_UDP_ENC=%u",
				 other->get_port(other));
	}
	if (child_sa->get_ipcomp(child_sa) != IPCOMP_NONE)
	{
		push_env(envp, countof(envp), "PLUTO_IPCOMP=1");
	}
	push_dns_env(this, ike_sa, envp, countof(envp));
	if (config->get_hostaccess(config))
	{
		push_env(envp, countof(envp), "PLUTO_HOST_ACCESS=1");
	}

	process = process_start_shell(envp, NULL, &out, NULL, "2>&1 %s",
								  config->get_updown(config));
	if (process)
	{
		shell = fdopen(out, "r");
		if (shell)
		{
			while (TRUE)
			{
				char resp[128];

				if (fgets(resp, sizeof(resp), shell) == NULL)
				{
					if (ferror(shell))
					{
						DBG1(DBG_CHD, "error reading from updown script");
					}
					break;
				}
				else
				{
					char *e = resp + strlen(resp);
					if (e > resp && e[-1] == '\n')
					{
						e[-1] = '\0';
					}
					DBG1(DBG_CHD, "updown: %s", resp);
				}
			}
			fclose(shell);
		}
		else
		{
			close(out);
		}
		process->wait(process, NULL);
	}
	free(iface);
	free_env(envp);
}

METHOD(listener_t, child_updown, bool,
	private_updown_listener_t *this, ike_sa_t *ike_sa, child_sa_t *child_sa,
	bool up)
{
	traffic_selector_t *my_ts, *other_ts;
	enumerator_t *enumerator;
	child_cfg_t *config;

	config = child_sa->get_config(child_sa);
	if (config->get_updown(config))
	{
		enumerator = child_sa->create_policy_enumerator(child_sa);
		while (enumerator->enumerate(enumerator, &my_ts, &other_ts))
		{
			invoke_once(this, ike_sa, child_sa, config, up, my_ts, other_ts);
		}
		enumerator->destroy(enumerator);
	}
	return TRUE;
}

METHOD(updown_listener_t, destroy, void,
	private_updown_listener_t *this)
{
	this->iface_cache->destroy(this->iface_cache);
	free(this);
}

/**
 * See header
 */
updown_listener_t *updown_listener_create(updown_handler_t *handler)
{
	private_updown_listener_t *this;

	INIT(this,
		.public = {
			.listener = {
				.child_updown = _child_updown,
			},
			.destroy = _destroy,
		},
		.iface_cache = linked_list_create(),
		.handler = handler,
	);

	return &this->public;
}
