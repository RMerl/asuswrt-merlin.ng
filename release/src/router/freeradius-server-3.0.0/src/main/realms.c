/*
 * realms.c	Realm handling code
 *
 * Version:     $Id$
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 *
 * Copyright 2007  The FreeRADIUS server project
 * Copyright 2007  Alan DeKok <aland@deployingradius.com>
 */

RCSID("$Id$")

#include <freeradius-devel/radiusd.h>
#include <freeradius-devel/rad_assert.h>

#include <sys/stat.h>

#include <ctype.h>
#include <fcntl.h>

static rbtree_t *realms_byname = NULL;

#ifdef HAVE_REGEX_H
typedef struct realm_regex_t {
	REALM	*realm;
	struct realm_regex_t *next;
} realm_regex_t;

static realm_regex_t *realms_regex = NULL;

#endif /* HAVE_REGEX_H */

typedef struct realm_config_t {
	CONF_SECTION	*cs;
	int		dead_time;
	int		retry_count;
	int		retry_delay;
	int		fallback;
	int		wake_all_if_all_dead;
} realm_config_t;

static realm_config_t *realm_config = NULL;

#ifdef WITH_PROXY
static rbtree_t	*home_servers_byaddr = NULL;
static rbtree_t	*home_servers_byname = NULL;
#ifdef WITH_STATS
static int home_server_max_number = 0;
static rbtree_t	*home_servers_bynumber = NULL;
#endif

static rbtree_t	*home_pools_byname = NULL;

/*
 *  Map the proxy server configuration parameters to variables.
 */
static const CONF_PARSER proxy_config[] = {
	{ "retry_delay",  PW_TYPE_INTEGER,
	  offsetof(realm_config_t, retry_delay),
	  NULL, STRINGIFY(RETRY_DELAY) },

	{ "retry_count",  PW_TYPE_INTEGER,
	  offsetof(realm_config_t, retry_count),
	  NULL, STRINGIFY(RETRY_COUNT) },

	{ "default_fallback", PW_TYPE_BOOLEAN,
	  offsetof(realm_config_t, fallback),
	  NULL, "no" },

	{ "dead_time",    PW_TYPE_INTEGER,
	  offsetof(realm_config_t, dead_time),
	  NULL, STRINGIFY(DEAD_TIME) },

	{ "wake_all_if_all_dead", PW_TYPE_BOOLEAN,
	  offsetof(realm_config_t, wake_all_if_all_dead),
	  NULL, "no" },

	{ NULL, -1, 0, NULL, NULL }
};
#endif

static int realm_name_cmp(void const *one, void const *two)
{
	REALM const *a = one;
	REALM const *b = two;

	return strcasecmp(a->name, b->name);
}


#ifdef WITH_PROXY
static void home_server_free(void *data)
{
	home_server *home = data;

	free(home);
}

static int home_server_name_cmp(void const *one, void const *two)
{
	home_server const *a = one;
	home_server const *b = two;

	if (a->type < b->type) return -1;
	if (a->type > b->type) return +1;

	return strcasecmp(a->name, b->name);
}

static int home_server_addr_cmp(void const *one, void const *two)
{
	int rcode;
	home_server const *a = one;
	home_server const *b = two;

	if (a->server && !b->server) return -1;
	if (!a->server && b->server) return +1;
	if (a->server && b->server) {
		rcode = a->type - b->type;
		if (rcode != 0) return rcode;
		return strcmp(a->server, b->server);
	}

	if (a->port < b->port) return -1;
	if (a->port > b->port) return +1;

#ifdef WITH_TCP
	if (a->proto < b->proto) return -1;
	if (a->proto > b->proto) return +1;
#endif

	rcode = fr_ipaddr_cmp(&a->src_ipaddr, &b->src_ipaddr);
	if (rcode != 0) return rcode;

	return fr_ipaddr_cmp(&a->ipaddr, &b->ipaddr);
}

#ifdef WITH_STATS
static int home_server_number_cmp(void const *one, void const *two)
{
	home_server const *a = one;
	home_server const *b = two;

	return (a->number - b->number);
}
#endif

static int home_pool_name_cmp(void const *one, void const *two)
{
	home_pool_t const *a = one;
	home_pool_t const *b = two;

	if (a->server_type < b->server_type) return -1;
	if (a->server_type > b->server_type) return +1;

	return strcasecmp(a->name, b->name);
}


static size_t xlat_cs(CONF_SECTION *cs, char const *fmt, char *out, size_t outlen)

{
	char const *value = NULL;

	/*
	 *	Instance name
	 */
	if (strcmp(fmt, "instance") == 0) {
		value = cf_section_name2(cs);
		if (!value) {
			*out = '\0';
			return 0;
		}
	} else {
		CONF_PAIR *cp;

		cp = cf_pair_find(cs, fmt);
		if (!cp || !(value = cf_pair_value(cp))) {
			*out = '\0';
			return 0;
		}
	}

	strlcpy(out, value, outlen);

	return strlen(out);
}


/*
 *	Xlat for %{home_server:foo}
 */
static ssize_t xlat_home_server(UNUSED void *instance, REQUEST *request,
			        char const *fmt, char *out, size_t outlen)
{
	if (!fmt || !out || (outlen < 1)) return 0;

	if (!request || !request->home_server) {
		RWDEBUG("No home_server associated with this request");

		*out = '\0';
		return 0;
	}

	return xlat_cs(request->home_server->cs, fmt, out, outlen);
}


/*
 *	Xlat for %{home_server_pool:foo}
 */
static ssize_t xlat_server_pool(UNUSED void *instance, REQUEST *request,
			        char const *fmt, char *out, size_t outlen)
{
	if (!fmt || !out || (outlen < 1)) return 0;

	if (!request || !request->home_pool) {
		RWDEBUG("No home_pool associated with this request");

		*out = '\0';
		return 0;
	}

	return xlat_cs(request->home_pool->cs, fmt, out, outlen);
}
#endif

void realms_free(void)
{
#ifdef WITH_PROXY
#ifdef WITH_STATS
	rbtree_free(home_servers_bynumber);
	home_servers_bynumber = NULL;
#endif

	rbtree_free(home_servers_byname);
	home_servers_byname = NULL;

	rbtree_free(home_servers_byaddr);
	home_servers_byaddr = NULL;

	rbtree_free(home_pools_byname);
	home_pools_byname = NULL;
#endif

	rbtree_free(realms_byname);
	realms_byname = NULL;

#ifdef HAVE_REGEX_H
	if (realms_regex) {
		realm_regex_t *this, *next;

		for (this = realms_regex; this != NULL; this = next) {
			next = this->next;
			free(this->realm);
			free(this);
		}
		realms_regex = NULL;
	}
#endif

	free(realm_config);
	realm_config = NULL;
}


#ifdef WITH_PROXY
static CONF_PARSER limit_config[] = {
	{ "max_connections", PW_TYPE_INTEGER,
	  offsetof(home_server, limit.max_connections), NULL,   "16" },

	{ "max_requests", PW_TYPE_INTEGER,
	  offsetof(home_server, limit.max_requests), NULL,   "0" },

	{ "lifetime", PW_TYPE_INTEGER,
	  offsetof(home_server, limit.lifetime), NULL,   "0" },

	{ "idle_timeout", PW_TYPE_INTEGER,
	  offsetof(home_server, limit.idle_timeout), NULL,   "0" },

	{ NULL, -1, 0, NULL, NULL }		/* end the list */
};

static struct in_addr hs_ip4addr;
static struct in6_addr hs_ip6addr;
static char *hs_srcipaddr = NULL;
static char *hs_type = NULL;
static char *hs_check = NULL;
static char *hs_virtual_server = NULL;
#ifdef WITH_TCP
static char *hs_proto = NULL;
#endif

#ifdef WITH_COA
static CONF_PARSER home_server_coa[] = {
	{ "irt",  PW_TYPE_INTEGER,
	  offsetof(home_server, coa_irt), 0, STRINGIFY(2) },
	{ "mrt",  PW_TYPE_INTEGER,
	  offsetof(home_server, coa_mrt), 0, STRINGIFY(16) },
	{ "mrc",  PW_TYPE_INTEGER,
	  offsetof(home_server, coa_mrc), 0, STRINGIFY(5) },
	{ "mrd",  PW_TYPE_INTEGER,
	  offsetof(home_server, coa_mrd), 0, STRINGIFY(30) },

	{ NULL, -1, 0, NULL, NULL }		/* end the list */
};
#endif

static CONF_PARSER home_server_config[] = {
	{ "ipaddr",  PW_TYPE_IPADDR,
	  0, &hs_ip4addr,  NULL },
	{ "ipv6addr",  PW_TYPE_IPV6ADDR,
	  0, &hs_ip6addr, NULL },
	{ "virtual_server",  PW_TYPE_STRING_PTR,
	  0, &hs_virtual_server, NULL },

	{ "port", PW_TYPE_INTEGER,
	  offsetof(home_server,port), NULL,   "0" },

	{ "type",  PW_TYPE_STRING_PTR,
	  0, &hs_type, NULL },

#ifdef WITH_TCP
	{ "proto",  PW_TYPE_STRING_PTR,
	  0, &hs_proto, NULL },
#endif

	{ "secret",  PW_TYPE_STRING_PTR,
	  offsetof(home_server,secret), NULL,  NULL},

	{ "src_ipaddr",  PW_TYPE_STRING_PTR,
	  0, &hs_srcipaddr,  NULL },

	{ "response_window", PW_TYPE_INTEGER,
	  offsetof(home_server,response_window), NULL,   "30" },
	{ "max_outstanding", PW_TYPE_INTEGER,
	  offsetof(home_server,max_outstanding), NULL,   "65536" },

	{ "zombie_period", PW_TYPE_INTEGER,
	  offsetof(home_server,zombie_period), NULL,   "40" },
	{ "status_check", PW_TYPE_STRING_PTR,
	  0, &hs_check,   "none" },
	{ "ping_check", PW_TYPE_STRING_PTR,
	  0, &hs_check,   NULL },

	{ "ping_interval", PW_TYPE_INTEGER,
	  offsetof(home_server,ping_interval), NULL,   "30" },
	{ "check_interval", PW_TYPE_INTEGER,
	  offsetof(home_server,ping_interval), NULL,   "30" },
	{ "num_answers_to_alive", PW_TYPE_INTEGER,
	  offsetof(home_server,num_pings_to_alive), NULL,   "3" },
	{ "revive_interval", PW_TYPE_INTEGER,
	  offsetof(home_server,revive_interval), NULL,   "300" },
	{ "status_check_timeout", PW_TYPE_INTEGER,
	  offsetof(home_server,ping_timeout), NULL,   "4" },

	{ "username",  PW_TYPE_STRING_PTR,
	  offsetof(home_server,ping_user_name), NULL,  NULL},
	{ "password",  PW_TYPE_STRING_PTR,
	  offsetof(home_server,ping_user_password), NULL,  NULL},

#ifdef WITH_STATS
	{ "historic_average_window", PW_TYPE_INTEGER,
	  offsetof(home_server,ema.window), NULL,  NULL },
#endif

#ifdef WITH_COA
	{  "coa", PW_TYPE_SUBSECTION, 0, NULL, (void const *) home_server_coa },
#endif

	{ "limit", PW_TYPE_SUBSECTION, 0, NULL, (void const *) limit_config },

	{ NULL, -1, 0, NULL, NULL }		/* end the list */
};


static void null_free(UNUSED void *data)
{
}

static int home_server_add(realm_config_t *rc, CONF_SECTION *cs)
{
	char const *name2;
	home_server *home;
	int dual = false;
	CONF_PAIR *cp;
	CONF_SECTION *tls;

	hs_virtual_server = NULL;

	name2 = cf_section_name2(cs);
	if (!name2) {
		cf_log_err_cs(cs,
			   "Home server section is missing a name.");
		return 0;
	}

	home = rad_malloc(sizeof(*home));
	memset(home, 0, sizeof(*home));

	home->name = name2;
	home->cs = cs;
	home->state = HOME_STATE_UNKNOWN;

	/*
	 *	Last packet sent / received are zero.
	 */

	memset(&hs_ip4addr, 0, sizeof(hs_ip4addr));
	memset(&hs_ip6addr, 0, sizeof(hs_ip6addr));
	if (cf_section_parse(cs, home, home_server_config) < 0) {
		goto error;
	}

	/*
	 *	Figure out which one to use.
	 */
	if (cf_pair_find(cs, "ipaddr")) {
		home->ipaddr.af = AF_INET;
		home->ipaddr.ipaddr.ip4addr = hs_ip4addr;

	} else if (cf_pair_find(cs, "ipv6addr")) {
		home->ipaddr.af = AF_INET6;
		home->ipaddr.ipaddr.ip6addr = hs_ip6addr;

	} else if ((cp = cf_pair_find(cs, "virtual_server")) != NULL) {
		home->ipaddr.af = AF_UNSPEC;
		home->server = cf_pair_value(cp);
		if (!home->server) {
			cf_log_err_cs(cs,
				   "Invalid value for virtual_server");
			goto error;
		}

		if (!cf_section_sub_find_name2(rc->cs, "server", home->server)) {

			cf_log_err_cs(cs,
				   "No such server %s", home->server);
			goto error;
		}

		/*
		 *	When CoA is used, the user has to specify the type
		 *	of the home server, even when they point to
		 *	virtual servers.
		 */
		home->secret = strdup("");
		goto skip_port;

	} else {
		cf_log_err_cs(cs,
			   "No ipaddr, ipv6addr, or virtual_server defined for home server \"%s\".",
			   name2);
	error:
		talloc_free(hs_type);
		hs_type = NULL;
		hs_check = NULL;
		hs_srcipaddr = NULL;
#ifdef WITH_TCP
		hs_proto = NULL;
#endif
		return 0;
	}

	if (!home->port || (home->port > 65535)) {
		cf_log_err_cs(cs,
			   "No port, or invalid port defined for home server %s.",
			   name2);
		goto error;
	}

	if (0) {
		cf_log_err_cs(cs,
			   "Fatal error!  Home server %s is ourselves!",
			   name2);
		goto error;
	}

	if (!home->secret) {
		cf_log_err_cs(cs,
			   "No shared secret defined for home server %s.",
			   name2);
		goto error;
	}

	/*
	 *	Use a reasonable default.
	 */
 skip_port:
	if (!hs_type) hs_type = talloc_strdup(cs, "auth+acct");

	if (strcasecmp(hs_type, "auth") == 0) {
		home->type = HOME_TYPE_AUTH;

	} else if (strcasecmp(hs_type, "acct") == 0) {
		home->type = HOME_TYPE_ACCT;

	} else if (strcasecmp(hs_type, "auth+acct") == 0) {
		home->type = HOME_TYPE_AUTH;
		dual = true;

#ifdef WITH_COA
	} else if (strcasecmp(hs_type, "coa") == 0) {
		home->type = HOME_TYPE_COA;
		dual = false;

		if (home->server != NULL) {
			cf_log_err_cs(cs,
				   "Home servers of type \"coa\" cannot point to a virtual server");
			goto error;
		}
#endif

	} else {
		cf_log_err_cs(cs,
			   "Invalid type \"%s\" for home server %s.",
			   hs_type, name2);
		goto error;
	}
	if (hs_type) talloc_free(hs_type);
	hs_type = NULL;

	if (!hs_check || (strcasecmp(hs_check, "none") == 0)) {
		home->ping_check = HOME_PING_CHECK_NONE;

	} else if (strcasecmp(hs_check, "status-server") == 0) {
		home->ping_check = HOME_PING_CHECK_STATUS_SERVER;

	} else if (strcasecmp(hs_check, "request") == 0) {
		home->ping_check = HOME_PING_CHECK_REQUEST;

		if (!home->ping_user_name ||
		    !*home->ping_user_name) {
			cf_log_err_cs(cs, "You must supply a 'username' to enable status_check=request");
			goto error;
		}

		if ((home->type == HOME_TYPE_AUTH) &&
		    (!home->ping_user_password ||
		     !*home->ping_user_password)) {
			cf_log_err_cs(cs, "You must supply a password to enable status_check=request");
			goto error;
		}

	} else {
		cf_log_err_cs(cs,
			   "Invalid status__check \"%s\" for home server %s.",
			   hs_check, name2);
		goto error;
	}
	hs_check = NULL;

	if ((home->ping_check != HOME_PING_CHECK_NONE) &&
	    (home->ping_check != HOME_PING_CHECK_STATUS_SERVER)) {
		if (!home->ping_user_name) {
			cf_log_err_cs(cs, "You must supply a user name to enable status_check=request");
			goto error;
		}

		if ((home->type == HOME_TYPE_AUTH) &&
		    !home->ping_user_password) {
			cf_log_err_cs(cs, "You must supply a password to enable status_check=request");
			goto error;
		}
	}

	home->proto = IPPROTO_UDP;
#ifdef WITH_TCP
	if (hs_proto) {
		if (strcmp(hs_proto, "udp") == 0) {
			hs_proto = NULL;

		} else if (strcmp(hs_proto, "tcp") == 0) {
			hs_proto = NULL;
			home->proto = IPPROTO_TCP;

			if (home->ping_check != HOME_PING_CHECK_NONE) {
				cf_log_err_cs(cs,
					   "Only 'status_check = none' is allowed for home servers with 'proto = tcp'");
				goto error;
			}

		} else {
			cf_log_err_cs(cs,
				   "Unknown proto \"%s\".", hs_proto);
			goto error;
		}
	}
#endif

	if (!home->server &&
	    rbtree_finddata(home_servers_byaddr, home)) {
		cf_log_err_cs(cs, "Duplicate home server");
		goto error;
	}

	/*
	 *	Check the TLS configuration.
	 */
	tls = cf_section_sub_find(cs, "tls");

	/*
	 *	If the home is a virtual server, don't look up source IP.
	 */
	if (!home->server) {
		rad_assert(home->ipaddr.af != AF_UNSPEC);

		/*
		 *	Otherwise look up the source IP using the same
		 *	address family as the destination IP.
		 */
		if (hs_srcipaddr) {
			if (ip_hton(hs_srcipaddr, home->ipaddr.af, &home->src_ipaddr) < 0) {
				cf_log_err_cs(cs, "Failed parsing src_ipaddr");
				goto error;
			}

		} else {
			/*
			 *	Source isn't specified: Source is
			 *	the correct address family, but all zeros.
			 */
			memset(&home->src_ipaddr, 0, sizeof(home->src_ipaddr));
			home->src_ipaddr.af = home->ipaddr.af;
		}

		if (tls && (home->proto != IPPROTO_TCP)) {
			cf_log_err_cs(cs, "TLS transport is not available for UDP sockets.");
			goto error;
		}

#ifndef WITH_TLS

		if (tls) {
			cf_log_err_cs(cs, "TLS transport is not available in this executable.");
			goto error;
		}
#else
		/*
		 *	Parse the SSL client configuration.
		 */
		if (tls) {
			home->tls = tls_client_conf_parse(tls);
			if (!home->tls) {
				goto error;
			}
		}
#endif

	} else if (tls) {
		cf_log_err_cs(cs, "Virtual home_servers cannot have a \"tls\" subsection");
		goto error;
	}

	/*
	 *	Make sure that this is set.
	 */
	if (home->src_ipaddr.af == AF_UNSPEC) {
		home->src_ipaddr.af = home->ipaddr.af;
	}

	hs_srcipaddr = NULL;

	if (rbtree_finddata(home_servers_byname, home) != NULL) {
		cf_log_err_cs(cs,
			   "Duplicate home server name %s.", name2);
		goto error;
	}

	if (!home->server &&
	    (rbtree_finddata(home_servers_byaddr, home) != NULL)) {
		cf_log_err_cs(cs,
			   "Duplicate home server IP %s.", name2);
		goto error;
	}

	if (!rbtree_insert(home_servers_byname, home)) {
		cf_log_err_cs(cs,
			   "Internal error %d adding home server %s.",
			   __LINE__, name2);
		goto error;
	}

	if (!home->server &&
	    !rbtree_insert(home_servers_byaddr, home)) {
		rbtree_deletebydata(home_servers_byname, home);
		cf_log_err_cs(cs,
			   "Internal error %d adding home server %s.",
			   __LINE__, name2);
		goto error;
	}

#ifdef WITH_STATS
	home->number = home_server_max_number++;
	if (!rbtree_insert(home_servers_bynumber, home)) {
		rbtree_deletebydata(home_servers_byname, home);
		if (home->ipaddr.af != AF_UNSPEC) {
			rbtree_deletebydata(home_servers_byname, home);
		}
		cf_log_err_cs(cs,
			   "Internal error %d adding home server %s.",
			   __LINE__, name2);
		goto error;
	}
#endif

	if (home->max_outstanding < 8) home->max_outstanding = 8;
	if (home->max_outstanding > 65536*16) home->max_outstanding = 65536*16;

	if (home->ping_interval < 6) home->ping_interval = 6;
	if (home->ping_interval > 120) home->ping_interval = 120;

	if (home->response_window < 1) home->response_window = 1;
	if (home->response_window > 60) home->response_window = 60;
	if (home->response_window > mainconfig.max_request_time) home->response_window = mainconfig.max_request_time;

	if (home->zombie_period < 1) home->zombie_period = 1;
	if (home->zombie_period > 120) home->zombie_period = 120;

	if (home->zombie_period < home->response_window) {
		home->zombie_period = home->response_window;
	}

	if (home->num_pings_to_alive < 3) home->num_pings_to_alive = 3;
	if (home->num_pings_to_alive > 10) home->num_pings_to_alive = 10;

	if (home->ping_timeout < 3) home->ping_timeout = 3;
	if (home->ping_timeout > 10) home->ping_timeout = 10;

	if (home->revive_interval < 60) home->revive_interval = 60;
	if (home->revive_interval > 3600) home->revive_interval = 3600;

#ifdef WITH_COA
	if (home->coa_irt < 1) home->coa_irt = 1;
	if (home->coa_irt > 5) home->coa_irt = 5;

	if (home->coa_mrc < 0) home->coa_mrc = 0;
	if (home->coa_mrc > 20 ) home->coa_mrc = 20;

	if (home->coa_mrt < 0) home->coa_mrt = 0;
	if (home->coa_mrt > 30 ) home->coa_mrt = 30;

	if (home->coa_mrd < 5) home->coa_mrd = 5;
	if (home->coa_mrd > 60 ) home->coa_mrd = 60;
#endif

	if (home->limit.max_connections > 1024) home->limit.max_connections = 1024;

#ifdef WITH_TCP
	/*
	 *	UDP sockets can't be connection limited.
	 */
	if (home->proto != IPPROTO_TCP) home->limit.max_connections = 0;
#endif

	if ((home->limit.idle_timeout > 0) && (home->limit.idle_timeout < 5))
		home->limit.idle_timeout = 5;
	if ((home->limit.lifetime > 0) && (home->limit.lifetime < 5))
		home->limit.lifetime = 5;
	if ((home->limit.lifetime > 0) && (home->limit.idle_timeout > home->limit.lifetime))
		home->limit.idle_timeout = 0;

	tls = cf_item_parent(cf_sectiontoitem(cs));
	if (strcmp(cf_section_name1(tls), "server") == 0) {
		home->parent_server = cf_section_name2(tls);
	}

	if (dual) {
		home_server *home2 = rad_malloc(sizeof(*home2));

		memcpy(home2, home, sizeof(*home2));

		home2->type = HOME_TYPE_ACCT;
		home2->port++;
		home2->ping_user_password = NULL;
		home2->cs = cs;
		home2->parent_server = home->parent_server;

		if (!rbtree_insert(home_servers_byname, home2)) {
			cf_log_err_cs(cs,
				   "Internal error %d adding home server %s.",
				   __LINE__, name2);
			free(home2);
			return 0;
		}

		if (!home->server &&
		    !rbtree_insert(home_servers_byaddr, home2)) {
			rbtree_deletebydata(home_servers_byname, home2);
			cf_log_err_cs(cs,
				   "Internal error %d adding home server %s.",
				   __LINE__, name2);
			free(home2);
			return 0;
		}

#ifdef WITH_STATS
		home2->number = home_server_max_number++;
		if (!rbtree_insert(home_servers_bynumber, home2)) {
			rbtree_deletebydata(home_servers_byname, home2);
			if (!home2->server) {
				rbtree_deletebydata(home_servers_byname, home2);
			}
			cf_log_err_cs(cs,
				   "Internal error %d adding home server %s.",
				   __LINE__, name2);
			free(home2);
			return 0;
		}
#endif
	}

	/*
	 *	Mark it as already processed
	 */
	cf_data_add(cs, "home_server", null_free, null_free);

	return 1;
}


static home_pool_t *server_pool_alloc(char const *name, home_pool_type_t type,
				      int server_type, int num_home_servers)
{
	home_pool_t *pool;

	pool = rad_malloc(sizeof(*pool) + (sizeof(pool->servers[0]) *
					   num_home_servers));
	if (!pool) return NULL;	/* just for pairanoia */

	memset(pool, 0, sizeof(*pool) + (sizeof(pool->servers[0]) *
					 num_home_servers));

	pool->name = name;
	pool->type = type;
	pool->server_type = server_type;
	pool->num_home_servers = num_home_servers;

	return pool;
}

/*
 * Ensure any home_server clauses in a home_server_pool section reference
 * defined home servers, which should already have been created, regardless
 * of where they appear in the configuration.
 */
static int pool_check_home_server(UNUSED realm_config_t *rc, CONF_PAIR *cp,
				  char const *name, int server_type,
				  home_server **phome)
{
	home_server myhome, *home;

	if (!name) {
		cf_log_err_cp(cp,
			   "No value given for home_server.");
		return 0;
	}

	myhome.name = name;
	myhome.type = server_type;
	home = rbtree_finddata(home_servers_byname, &myhome);
	if (home) {
		*phome = home;
		return 1;
	}

	cf_log_err_cp(cp, "Unknown home_server \"%s\".", name);
	return 0;
}


static int server_pool_add(realm_config_t *rc,
			   CONF_SECTION *cs, int server_type, int do_print)
{
	char const *name2;
	home_pool_t *pool = NULL;
	char const *value;
	CONF_PAIR *cp;
	int num_home_servers;
	home_server *home;

	name2 = cf_section_name1(cs);
	if (!name2 || ((strcasecmp(name2, "server_pool") != 0) &&
		       (strcasecmp(name2, "home_server_pool") != 0))) {
		cf_log_err_cs(cs,
			   "Section is not a home_server_pool.");
		return 0;
	}

	name2 = cf_section_name2(cs);
	if (!name2) {
		cf_log_err_cs(cs,
			   "Server pool section is missing a name.");
		return 0;
	}

	/*
	 *	Count the home servers and initalize them.
	 */
	num_home_servers = 0;
	for (cp = cf_pair_find(cs, "home_server");
	     cp != NULL;
	     cp = cf_pair_find_next(cs, cp, "home_server")) {
		num_home_servers++;

		if (!pool_check_home_server(rc, cp, cf_pair_value(cp),
					    server_type, &home)) {
			return 0;
		}
	}

	if (num_home_servers == 0) {
		cf_log_err_cs(cs,
			   "No home servers defined in pool %s",
			   name2);
		goto error;
	}

	pool = server_pool_alloc(name2, HOME_POOL_FAIL_OVER, server_type,
				 num_home_servers);
	pool->cs = cs;


	/*
	 *	Fallback servers must be defined, and must be
	 *	virtual servers.
	 */
	cp = cf_pair_find(cs, "fallback");
	if (cp) {
#ifdef WITH_COA
		if (server_type == HOME_TYPE_COA) {
			cf_log_err_cs(cs, "Home server pools of type \"coa\" cannot have a fallback virtual server.");
			goto error;
		}
#endif

		if (!pool_check_home_server(rc, cp, cf_pair_value(cp),
					    server_type, &pool->fallback)) {

			goto error;
		}

		if (!pool->fallback->server) {
			cf_log_err_cs(cs, "Fallback home_server %s does NOT contain a virtual_server directive.", pool->fallback->name);
			goto error;
		}
	}

	if (do_print) cf_log_info(cs, " home_server_pool %s {", name2);

	cp = cf_pair_find(cs, "type");
	if (cp) {
		static FR_NAME_NUMBER pool_types[] = {
			{ "load-balance", HOME_POOL_LOAD_BALANCE },

			{ "fail-over", HOME_POOL_FAIL_OVER },
			{ "fail_over", HOME_POOL_FAIL_OVER },

			{ "round-robin", HOME_POOL_LOAD_BALANCE },
			{ "round_robin", HOME_POOL_LOAD_BALANCE },

			{ "client-balance", HOME_POOL_CLIENT_BALANCE },
			{ "client-port-balance", HOME_POOL_CLIENT_PORT_BALANCE },
			{ "keyed-balance", HOME_POOL_KEYED_BALANCE },
			{ NULL, 0 }
		};

		value = cf_pair_value(cp);
		if (!value) {
			cf_log_err_cp(cp,
				   "No value given for type.");
			goto error;
		}

		pool->type = fr_str2int(pool_types, value, 0);
		if (!pool->type) {
			cf_log_err_cp(cp,
				   "Unknown type \"%s\".",
				   value);
			goto error;
		}

		if (do_print) cf_log_info(cs, "\ttype = %s", value);
	}

	cp = cf_pair_find(cs, "virtual_server");
	if (cp) {
		pool->virtual_server = cf_pair_value(cp);
		if (!pool->virtual_server) {
			cf_log_err_cp(cp, "No value given for virtual_server");
			goto error;
		}

		if (do_print) {
			cf_log_info(cs, "\tvirtual_server = %s", pool->virtual_server);
		}

		if (!cf_section_sub_find_name2(rc->cs, "server",
					       pool->virtual_server)) {
			cf_log_err_cp(cp, "No such server %s",
				   pool->virtual_server);
			goto error;
		}

	}

	num_home_servers = 0;
	for (cp = cf_pair_find(cs, "home_server");
	     cp != NULL;
	     cp = cf_pair_find_next(cs, cp, "home_server")) {
		home_server myhome;

		value = cf_pair_value(cp);

		memset(&myhome, 0, sizeof(myhome));
		myhome.name = value;
		myhome.type = server_type;

		home = rbtree_finddata(home_servers_byname, &myhome);
		if (!home) {
			DEBUG2("Internal sanity check failed");
			goto error;
		}

		if (0) {
			WDEBUG2("Duplicate home server %s in server pool %s", home->name, pool->name);
			continue;
		}

		if (do_print) cf_log_info(cs, "\thome_server = %s", home->name);
		pool->servers[num_home_servers++] = home;
	} /* loop over home_server's */

	if (pool->fallback && do_print) {
		cf_log_info(cs, "\tfallback = %s", pool->fallback->name);
	}

	if (!rbtree_insert(home_pools_byname, pool)) {
		rad_assert("Internal sanity check failed");
		goto error;
	}

	if (do_print) cf_log_info(cs, " }");

	cf_data_add(cs, "home_server_pool", pool, free);

	rad_assert(pool->server_type != 0);

	return 1;

 error:
	if (do_print) cf_log_info(cs, " }");
	free(pool);
	return 0;
}
#endif

static int old_server_add(realm_config_t *rc, CONF_SECTION *cs,
			  char const *realm,
			  char const *name, char const *secret,
			  home_pool_type_t ldflag, home_pool_t **pool_p,
			  int type, char const *server)
{
#ifdef WITH_PROXY
	int i, insert_point, num_home_servers;
	home_server myhome, *home;
	home_pool_t mypool, *pool;
	CONF_SECTION *subcs;
#else
	(void) rc;		/* -Wunused */
	(void) realm;
	(void) secret;
	(void) ldflag;
	(void) type;
	(void) server;
#endif

	/*
	 *	LOCAL realms get sanity checked, and nothing else happens.
	 */
	if (strcmp(name, "LOCAL") == 0) {
		if (*pool_p) {
			cf_log_err_cs(cs, "Realm \"%s\" cannot be both LOCAL and remote", name);
			return 0;
		}
		return 1;
	}

#ifndef WITH_PROXY
	return 0;		/* Not proxying.  Can't do non-LOCAL realms */

#else
	mypool.name = realm;
	mypool.server_type = type;
	pool = rbtree_finddata(home_pools_byname, &mypool);
	if (pool) {
		if (pool->type != ldflag) {
			cf_log_err_cs(cs, "Inconsistent ldflag for server pool \"%s\"", name);
			return 0;
		}

		if (pool->server_type != type) {
			cf_log_err_cs(cs, "Inconsistent home server type for server pool \"%s\"", name);
			return 0;
		}
	}

	myhome.name = name;
	myhome.type = type;
	home = rbtree_finddata(home_servers_byname, &myhome);
	if (home) {
		if (secret && (strcmp(home->secret, secret) != 0)) {
			cf_log_err_cs(cs, "Inconsistent shared secret for home server \"%s\"", name);
			return 0;
		}

		if (home->type != type) {
			cf_log_err_cs(cs, "Inconsistent type for home server \"%s\"", name);
			return 0;
		}

		/*
		 *	See if the home server is already listed
		 *	in the pool.  If so, do nothing else.
		 */
		if (pool) for (i = 0; i < pool->num_home_servers; i++) {
			if (pool->servers[i] == home) {
				return 1;
			}
		}
	}

	/*
	 *	If we do have a pool, check that there is room to
	 *	insert the home server we've found, or the one that we
	 *	create here.
	 *
	 *	Note that we insert it into the LAST available
	 *	position, in order to maintain the same order as in
	 *	the configuration files.
	 */
	insert_point = -1;
	if (pool) {
		for (i = pool->num_home_servers - 1; i >= 0; i--) {
			if (pool->servers[i]) break;

			if (!pool->servers[i]) {
				insert_point = i;
			}
		}

		if (insert_point < 0) {
			cf_log_err_cs(cs, "No room in pool to add home server \"%s\".  Please update the realm configuration to use the new-style home servers and server pools.", name);
			return 0;
		}
	}

	/*
	 *	No home server, allocate one.
	 */
	if (!home) {
		char const *p;
		char *q;

		home = rad_malloc(sizeof(*home));
		memset(home, 0, sizeof(*home));

		home->name = name;
		home->hostname = name;
		home->type = type;
		home->secret = secret;
		home->cs = cs;
		home->proto = IPPROTO_UDP;

		p = strchr(name, ':');
		if (!p) {
			if (type == HOME_TYPE_AUTH) {
				home->port = PW_AUTH_UDP_PORT;
			} else {
				home->port = PW_ACCT_UDP_PORT;
			}

			p = name;
			q = NULL;

		} else if (p == name) {
				cf_log_err_cs(cs,
					   "Invalid hostname %s.",
					   name);
				free(home);
				return 0;

		} else {
			home->port = atoi(p + 1);
			if ((home->port == 0) || (home->port > 65535)) {
				cf_log_err_cs(cs,
					   "Invalid port %s.",
					   p + 1);
				free(home);
				return 0;
			}

			q = rad_malloc((p - name) + 1);
			memcpy(q, name, (p - name));
			q[p - name] = '\0';
			p = q;
		}

		if (!server) {
			if (ip_hton(p, AF_UNSPEC, &home->ipaddr) < 0) {
				cf_log_err_cs(cs,
					   "Failed looking up hostname %s.",
					   p);
				free(home);
				free(q);
				return 0;
			}
			home->src_ipaddr.af = home->ipaddr.af;
		} else {
			home->ipaddr.af = AF_UNSPEC;
			home->server = server;
		}
		free(q);

		/*
		 *	Use the old-style configuration.
		 */
		home->max_outstanding = 65535*16;
		home->zombie_period = rc->retry_delay * rc->retry_count;
		if (home->zombie_period == 0) home->zombie_period =30;
		home->response_window = home->zombie_period - 1;

		home->ping_check = HOME_PING_CHECK_NONE;

		home->revive_interval = rc->dead_time;

		if (rbtree_finddata(home_servers_byaddr, home)) {
			cf_log_err_cs(cs, "Home server %s has the same IP address and/or port as another home server.", name);
			free(home);
			return 0;
		}

		if (!rbtree_insert(home_servers_byname, home)) {
			cf_log_err_cs(cs, "Internal error %d adding home server %s.", __LINE__, name);
			free(home);
			return 0;
		}

		if (!rbtree_insert(home_servers_byaddr, home)) {
			rbtree_deletebydata(home_servers_byname, home);
			cf_log_err_cs(cs, "Internal error %d adding home server %s.", __LINE__, name);
			free(home);
			return 0;
		}

#ifdef WITH_STATS
		home->number = home_server_max_number++;
		if (!rbtree_insert(home_servers_bynumber, home)) {
			rbtree_deletebydata(home_servers_byname, home);
			if (home->ipaddr.af != AF_UNSPEC) {
				rbtree_deletebydata(home_servers_byname, home);
			}
			cf_log_err_cs(cs,
				   "Internal error %d adding home server %s.",
				   __LINE__, name);
			free(home);
			return 0;
		}
#endif
	}

	/*
	 *	We now have a home server, see if we can insert it
	 *	into pre-existing pool.
	 */
	if (insert_point >= 0) {
		rad_assert(pool != NULL);
		pool->servers[insert_point] = home;
		return 1;
	}

	rad_assert(pool == NULL);
	rad_assert(home != NULL);

	/*
	 *	Count the old-style realms of this name.
	 */
	num_home_servers = 0;
	for (subcs = cf_section_find_next(cs, NULL, "realm");
	     subcs != NULL;
	     subcs = cf_section_find_next(cs, subcs, "realm")) {
		char const *this = cf_section_name2(subcs);

		if (!this || (strcmp(this, realm) != 0)) continue;
		num_home_servers++;
	}

	if (num_home_servers == 0) {
		cf_log_err_cs(cs, "Internal error counting pools for home server %s.", name);
		free(home);
		return 0;
	}

	pool = server_pool_alloc(realm, ldflag, type, num_home_servers);
	pool->cs = cs;

	pool->servers[0] = home;

	if (!rbtree_insert(home_pools_byname, pool)) {
		rad_assert("Internal sanity check failed");
		return 0;
	}

	*pool_p = pool;

	return 1;
#endif
}

static int old_realm_config(realm_config_t *rc, CONF_SECTION *cs, REALM *r)
{
	char const *host;
	char const *secret = NULL;
	home_pool_type_t ldflag;
	CONF_PAIR *cp;

	cp = cf_pair_find(cs, "ldflag");
	ldflag = HOME_POOL_FAIL_OVER;
	if (cp) {
		host = cf_pair_value(cp);
		if (!host) {
			cf_log_err_cp(cp, "No value specified for ldflag");
			return 0;
		}

		if (strcasecmp(host, "fail_over") == 0) {
			cf_log_info(cs, "\tldflag = fail_over");

		} else if (strcasecmp(host, "round_robin") == 0) {
			ldflag = HOME_POOL_LOAD_BALANCE;
			cf_log_info(cs, "\tldflag = round_robin");

		} else {
			cf_log_err_cs(cs, "Unknown value \"%s\" for ldflag", host);
			return 0;
		}
	} /* else don't print it. */

	/*
	 *	Allow old-style if it doesn't exist, or if it exists and
	 *	it's LOCAL.
	 */
	cp = cf_pair_find(cs, "authhost");
	if (cp) {
		host = cf_pair_value(cp);
		if (!host) {
			cf_log_err_cp(cp, "No value specified for authhost");
			return 0;
		}

		if (strcmp(host, "LOCAL") != 0) {
			cp = cf_pair_find(cs, "secret");
			if (!cp) {
				cf_log_err_cs(cs, "No shared secret supplied for realm: %s", r->name);
				return 0;
			}

			secret = cf_pair_value(cp);
			if (!secret) {
				cf_log_err_cp(cp, "No value specified for secret");
				return 0;
			}
		}

		cf_log_info(cs, "\tauthhost = %s",  host);

		if (!old_server_add(rc, cs, r->name, host, secret, ldflag,
				    &r->auth_pool, HOME_TYPE_AUTH, NULL)) {
			return 0;
		}
	}

	cp = cf_pair_find(cs, "accthost");
	if (cp) {
		host = cf_pair_value(cp);
		if (!host) {
			cf_log_err_cp(cp, "No value specified for accthost");
			return 0;
		}

		/*
		 *	Don't look for a secret again if it was found
		 *	above.
		 */
		if ((strcmp(host, "LOCAL") != 0) && !secret) {
			cp = cf_pair_find(cs, "secret");
			if (!cp) {
				cf_log_err_cs(cs, "No shared secret supplied for realm: %s", r->name);
				return 0;
			}

			secret = cf_pair_value(cp);
			if (!secret) {
				cf_log_err_cp(cp, "No value specified for secret");
				return 0;
			}
		}

		cf_log_info(cs, "\taccthost = %s", host);

		if (!old_server_add(rc, cs, r->name, host, secret, ldflag,
				    &r->acct_pool, HOME_TYPE_ACCT, NULL)) {
			return 0;
		}
	}

	cp = cf_pair_find(cs, "virtual_server");
	if (cp) {
		host = cf_pair_value(cp);
		if (!host) {
			cf_log_err_cp(cp, "No value specified for virtual_server");
			return 0;
		}

		cf_log_info(cs, "\tvirtual_server = %s", host);

		if (!old_server_add(rc, cs, r->name, host, "", ldflag,
				    &r->auth_pool, HOME_TYPE_AUTH, host)) {
			return 0;
		}
		if (!old_server_add(rc, cs, r->name, host, "", ldflag,
				    &r->acct_pool, HOME_TYPE_ACCT, host)) {
			return 0;
		}
	}

	if (secret) cf_log_info(cs, "\tsecret = %s", secret);

	return 1;

}


#ifdef WITH_PROXY
static int add_pool_to_realm(realm_config_t *rc, CONF_SECTION *cs,
			     char const *name, home_pool_t **dest,
			     int server_type, int do_print)
{
	home_pool_t mypool, *pool;

	mypool.name = name;
	mypool.server_type = server_type;

	pool = rbtree_finddata(home_pools_byname, &mypool);
	if (!pool) {
		CONF_SECTION *pool_cs;

		pool_cs = cf_section_sub_find_name2(rc->cs,
						    "home_server_pool",
						    name);
		if (!pool_cs) {
			pool_cs = cf_section_sub_find_name2(rc->cs,
							    "server_pool",
							    name);
		}
		if (!pool_cs) {
			cf_log_err_cs(cs, "Failed to find home_server_pool \"%s\"", name);
			return 0;
		}

		if (!server_pool_add(rc, pool_cs, server_type, do_print)) {
			return 0;
		}

		pool = rbtree_finddata(home_pools_byname, &mypool);
		if (!pool) {
			ERROR("Internal sanity check failed in add_pool_to_realm");
			return 0;
		}
	}

	if (pool->server_type != server_type) {
		cf_log_err_cs(cs, "Incompatible home_server_pool \"%s\" (mixed auth_pool / acct_pool)", name);
		return 0;
	}

	*dest = pool;

	return 1;
}
#endif


static int realm_add(realm_config_t *rc, CONF_SECTION *cs)
{
	char const *name2;
	REALM *r = NULL;
	CONF_PAIR *cp;
#ifdef WITH_PROXY
	home_pool_t *auth_pool, *acct_pool;
	char const *auth_pool_name, *acct_pool_name;
#ifdef WITH_COA
	char const *coa_pool_name;
	home_pool_t *coa_pool;
#endif
#endif

	name2 = cf_section_name1(cs);
	if (!name2 || (strcasecmp(name2, "realm") != 0)) {
		cf_log_err_cs(cs, "Section is not a realm.");
		return 0;
	}

	name2 = cf_section_name2(cs);
	if (!name2) {
		cf_log_err_cs(cs, "Realm section is missing the realm name.");
		return 0;
	}

#ifdef WITH_PROXY
	auth_pool = acct_pool = NULL;
	auth_pool_name = acct_pool_name = NULL;
#ifdef WITH_COA
	coa_pool = NULL;
	coa_pool_name = NULL;
#endif

	/*
	 *	Prefer new configuration to old one.
	 */
	cp = cf_pair_find(cs, "pool");
	if (!cp) cp = cf_pair_find(cs, "home_server_pool");
	if (cp) auth_pool_name = cf_pair_value(cp);
	if (cp && auth_pool_name) {
		acct_pool_name = auth_pool_name;
		if (!add_pool_to_realm(rc, cs,
				       auth_pool_name, &auth_pool,
				       HOME_TYPE_AUTH, 1)) {
			return 0;
		}
		if (!add_pool_to_realm(rc, cs,
				       auth_pool_name, &acct_pool,
				       HOME_TYPE_ACCT, 0)) {
			return 0;
		}
	}

	cp = cf_pair_find(cs, "auth_pool");
	if (cp) auth_pool_name = cf_pair_value(cp);
	if (cp && auth_pool_name) {
		if (auth_pool) {
			cf_log_err_cs(cs, "Cannot use \"pool\" and \"auth_pool\" at the same time.");
			return 0;
		}
		if (!add_pool_to_realm(rc, cs,
				       auth_pool_name, &auth_pool,
				       HOME_TYPE_AUTH, 1)) {
			return 0;
		}
	}

	cp = cf_pair_find(cs, "acct_pool");
	if (cp) acct_pool_name = cf_pair_value(cp);
	if (cp && acct_pool_name) {
		int do_print = true;

		if (acct_pool) {
			cf_log_err_cs(cs, "Cannot use \"pool\" and \"acct_pool\" at the same time.");
			return 0;
		}

		if (!auth_pool ||
		    (auth_pool_name &&
		     (strcmp(auth_pool_name, acct_pool_name) != 0))) {
			do_print = true;
		}

		if (!add_pool_to_realm(rc, cs,
				       acct_pool_name, &acct_pool,
				       HOME_TYPE_ACCT, do_print)) {
			return 0;
		}
	}

#ifdef WITH_COA
	cp = cf_pair_find(cs, "coa_pool");
	if (cp) coa_pool_name = cf_pair_value(cp);
	if (cp && coa_pool_name) {
		int do_print = true;

		if (!add_pool_to_realm(rc, cs,
				       coa_pool_name, &coa_pool,
				       HOME_TYPE_COA, do_print)) {
			return 0;
		}
	}
#endif
#endif

	cf_log_info(cs, " realm %s {", name2);

#ifdef WITH_PROXY
	/*
	 *	The realm MAY already exist if it's an old-style realm.
	 *	In that case, merge the old-style realm with this one.
	 */
	r = realm_find2(name2);
	if (r && (strcmp(r->name, name2) == 0)) {
		if (cf_pair_find(cs, "auth_pool") ||
		    cf_pair_find(cs, "acct_pool")) {
			cf_log_err_cs(cs, "Duplicate realm \"%s\"", name2);
			goto error;
		}

		if (!old_realm_config(rc, cs, r)) {
			goto error;
		}

		cf_log_info(cs, " } # realm %s", name2);
		return 1;
	}
#endif

#ifdef HAVE_REGEX_H
	if (name2[0] == '~') {
		int rcode;
		regex_t reg;

		/*
		 *	Include substring matches.
		 */
		rcode = regcomp(&reg, name2 + 1, REG_EXTENDED | REG_NOSUB | REG_ICASE);
		if (rcode != 0) {
			char buffer[256];

			regerror(rcode, &reg, buffer, sizeof(buffer));

			cf_log_err_cs(cs,
				   "Invalid regex \"%s\": %s",
				   name2 + 1, buffer);
			goto error;
		}
		regfree(&reg);
	}
#endif

	r = rad_malloc(sizeof(*r));
	memset(r, 0, sizeof(*r));

	r->name = name2;
	r->striprealm = 1;
#ifdef WITH_PROXY
	r->auth_pool = auth_pool;
	r->acct_pool = acct_pool;
#ifdef WITH_COA
	r->coa_pool = coa_pool;
#endif

	if (auth_pool_name &&
	    (auth_pool_name == acct_pool_name)) { /* yes, ptr comparison */
		cf_log_info(cs, "\tpool = %s", auth_pool_name);
	} else {
		if (auth_pool_name) cf_log_info(cs, "\tauth_pool = %s", auth_pool_name);
		if (acct_pool_name) cf_log_info(cs, "\tacct_pool = %s", acct_pool_name);
#ifdef WITH_COA
		if (coa_pool_name) cf_log_info(cs, "\tcoa_pool = %s", coa_pool_name);
#endif
	}
#endif

	cp = cf_pair_find(cs, "nostrip");
	if (cp && (cf_pair_value(cp) == NULL)) {
		r->striprealm = 0;
		cf_log_info(cs, "\tnostrip");
	}

	/*
	 *	We're a new-style realm.  Complain if we see the old
	 *	directives.
	 */
	if (r->auth_pool || r->acct_pool) {
		if (((cp = cf_pair_find(cs, "authhost")) != NULL) ||
		    ((cp = cf_pair_find(cs, "accthost")) != NULL) ||
		    ((cp = cf_pair_find(cs, "secret")) != NULL) ||
		    ((cp = cf_pair_find(cs, "ldflag")) != NULL)) {
			WDEBUG2("Ignoring old-style configuration entry \"%s\" in realm \"%s\"", cf_pair_attr(cp), r->name);
		}


		/*
		 *	The realm MAY be an old-style realm, as there
		 *	was no auth_pool or acct_pool.  Double-check
		 *	it, just to be safe.
		 */
	} else if (!old_realm_config(rc, cs, r)) {
		goto error;
	}

#ifdef HAVE_REGEX_H
	/*
	 *	It's a regex.  Add it to a separate list.
	 */
	if (name2[0] == '~') {
		realm_regex_t *rr, **last;

		rr = rad_malloc(sizeof(*rr));

		last = &realms_regex;
		while (*last) last = &((*last)->next);  /* O(N^2)... sue me. */

		r->name = name2;
		rr->realm = r;
		rr->next = NULL;

		*last = rr;

		cf_log_info(cs, " }");
		return 1;
	}
#endif

	if (!rbtree_insert(realms_byname, r)) {
		rad_assert("Internal sanity check failed");
		goto error;
	}

	cf_log_info(cs, " }");

	return 1;

 error:
	cf_log_info(cs, " } # realm %s", name2);
	free(r);
	return 0;
}

#ifdef WITH_COA
static const FR_NAME_NUMBER home_server_types[] = {
	{ "auth", HOME_TYPE_AUTH },
	{ "auth+acct", HOME_TYPE_AUTH },
	{ "acct", HOME_TYPE_ACCT },
	{ "coa", HOME_TYPE_COA },
	{ NULL, 0 }
};

static int pool_peek_type(CONF_SECTION *config, CONF_SECTION *cs)
{
	int home;
	char const *name, *type;
	CONF_PAIR *cp;
	CONF_SECTION *server_cs;

	cp = cf_pair_find(cs, "home_server");
	if (!cp) {
		cf_log_err_cs(cs, "Pool does not contain a \"home_server\" entry");
		return HOME_TYPE_INVALID;
	}

	name = cf_pair_value(cp);
	if (!name) {
		cf_log_err_cp(cp, "home_server entry does not reference a home server");
		return HOME_TYPE_INVALID;
	}

	server_cs = cf_section_sub_find_name2(config, "home_server", name);
	if (!server_cs) {
		cf_log_err_cp(cp, "home_server \"%s\" does not exist", name);
		return HOME_TYPE_INVALID;
	}

	cp = cf_pair_find(server_cs, "type");
	if (!cp) {
		cf_log_err_cs(server_cs, "home_server %s does not contain a \"type\" entry", name);
		return HOME_TYPE_INVALID;
	}

	type = cf_pair_value(cp);
	if (!type) {
		cf_log_err_cs(server_cs, "home_server %s contains an empty \"type\" entry", name);
		return HOME_TYPE_INVALID;
	}

	home = fr_str2int(home_server_types, type, HOME_TYPE_INVALID);
	if (home == HOME_TYPE_INVALID) {
		cf_log_err_cs(server_cs, "home_server %s contains an invalid \"type\" entry of value \"%s\"", name, type);
		return HOME_TYPE_INVALID;
	}

	return home;		/* 'cause we miss it so much */
}
#endif

int realms_init(CONF_SECTION *config)
{
	CONF_SECTION *cs;
#ifdef WITH_PROXY
	CONF_SECTION *server_cs;
#endif
	realm_config_t *rc, *old_rc;

	if (realms_byname) return 1;

	realms_byname = rbtree_create(realm_name_cmp, free, 0);
	if (!realms_byname) {
		realms_free();
		return 0;
	}

#ifdef WITH_PROXY
	home_servers_byaddr = rbtree_create(home_server_addr_cmp, home_server_free, 0);
	if (!home_servers_byaddr) {
		realms_free();
		return 0;
	}

	home_servers_byname = rbtree_create(home_server_name_cmp, NULL, 0);
	if (!home_servers_byname) {
		realms_free();
		return 0;
	}

#ifdef WITH_STATS
	home_servers_bynumber = rbtree_create(home_server_number_cmp, NULL, 0);
	if (!home_servers_bynumber) {
		realms_free();
		return 0;
	}
#endif

	home_pools_byname = rbtree_create(home_pool_name_cmp, NULL, 0);
	if (!home_pools_byname) {
		realms_free();
		return 0;
	}
#endif

	rc = rad_malloc(sizeof(*rc));
	memset(rc, 0, sizeof(*rc));
	rc->cs = config;

#ifdef WITH_PROXY
	cs = cf_subsection_find_next(config, NULL, "proxy");
	if (cs) {
		if (cf_section_parse(cs, rc, proxy_config) < 0) {
			ERROR("Failed parsing proxy section");

			free(rc);
			realms_free();
			return 0;
		}
	} else {
		rc->dead_time = DEAD_TIME;
		rc->retry_count = RETRY_COUNT;
		rc->retry_delay = RETRY_DELAY;
		rc->fallback = 0;
		rc->wake_all_if_all_dead= 0;
	}

	for (cs = cf_subsection_find_next(config, NULL, "home_server");
	     cs != NULL;
	     cs = cf_subsection_find_next(config, cs, "home_server")) {
		if (!home_server_add(rc, cs)) {
			free(rc);
			realms_free();
			return 0;
		}
	}

	/*
	 *	Loop over virtual servers to find homes which are
	 *	defined in them.
	 */
	for (server_cs = cf_subsection_find_next(config, NULL, "server");
	     server_cs != NULL;
	     server_cs = cf_subsection_find_next(config, server_cs, "server")) {
		for (cs = cf_subsection_find_next(server_cs, NULL, "home_server");
		     cs != NULL;
		     cs = cf_subsection_find_next(server_cs, cs, "home_server")) {
			if (!home_server_add(rc, cs)) {
				free(rc);
				realms_free();
				return 0;
			}
		}
	}
#endif

	for (cs = cf_subsection_find_next(config, NULL, "realm");
	     cs != NULL;
	     cs = cf_subsection_find_next(config, cs, "realm")) {
		if (!realm_add(rc, cs)) {
			free(rc);
			realms_free();
			return 0;
		}
	}

#ifdef WITH_COA
	/*
	 *	CoA pools aren't necessarily tied to realms.
	 */
	for (cs = cf_subsection_find_next(config, NULL, "home_server_pool");
	     cs != NULL;
	     cs = cf_subsection_find_next(config, cs, "home_server_pool")) {
		int type;

		/*
		 *	Pool was already loaded.
		 */
		if (cf_data_find(cs, "home_server_pool")) continue;

		type = pool_peek_type(config, cs);
		if (type == HOME_TYPE_INVALID) {
			free(rc);
			realms_free();
			return 0;
		}

		if (!server_pool_add(rc, cs, type, true)) {
			free(rc);
			realms_free();
			return 0;
		}
	}
#endif


#ifdef WITH_PROXY
	xlat_register("home_server", xlat_home_server, NULL, NULL);
	xlat_register("home_server_pool", xlat_server_pool, NULL, NULL);
#endif

	/*
	 *	Swap pointers atomically.
	 */
	old_rc = realm_config;
	realm_config = rc;
	free(old_rc);

	return 1;
}

/*
 *	Find a realm where "name" might be the regex.
 */
REALM *realm_find2(char const *name)
{
	REALM myrealm;
	REALM *realm;

	if (!name) name = "NULL";

	myrealm.name = name;
	realm = rbtree_finddata(realms_byname, &myrealm);
	if (realm) return realm;

#ifdef HAVE_REGEX_H
	if (realms_regex) {
		realm_regex_t *this;

		for (this = realms_regex; this != NULL; this = this->next) {
			if (strcmp(this->realm->name, name) == 0) {
				return this->realm;
			}
		}
	}
#endif

	/*
	 *	Couldn't find a realm.  Look for DEFAULT.
	 */
	myrealm.name = "DEFAULT";
	return rbtree_finddata(realms_byname, &myrealm);
}


/*
 *	Find a realm in the REALM list.
 */
REALM *realm_find(char const *name)
{
	REALM myrealm;
	REALM *realm;

	if (!name) name = "NULL";

	myrealm.name = name;
	realm = rbtree_finddata(realms_byname, &myrealm);
	if (realm) return realm;

#ifdef HAVE_REGEX_H
	if (realms_regex) {
		realm_regex_t *this;

		for (this = realms_regex; this != NULL; this = this->next) {
			int compare;
			regex_t reg;

			/*
			 *	Include substring matches.
			 */
			if (regcomp(&reg, this->realm->name + 1, REG_EXTENDED | REG_NOSUB | REG_ICASE) != 0) {
				continue;
			}

			compare = regexec(&reg, name, 0, NULL, 0);
			regfree(&reg);

			if (compare == 0) return this->realm;
		}
	}
#endif

	/*
	 *	Couldn't find a realm.  Look for DEFAULT.
	 */
	myrealm.name = "DEFAULT";
	return rbtree_finddata(realms_byname, &myrealm);
}


#ifdef WITH_PROXY

/*
 *	Allocate the proxy list if it doesn't already exist, and copy request
 *	VPs into it. Setup src/dst IP addresses based on home server, and
 *	calculate and add the message-authenticator.
 *
 *	This is a distinct function from home_server_ldb, as not all home_server
 *	lookups result in the *CURRENT* request being proxied,
 *	as in rlm_replicate, and this may trigger asserts elsewhere in the
 *	server.
 */
void home_server_update_request(home_server *home, REQUEST *request)
{

	/*
	 *	Allocate the proxy packet, only if it wasn't
	 *	already allocated by a module.  This check is
	 *	mainly to support the proxying of EAP-TTLS and
	 *	EAP-PEAP tunneled requests.
	 *
	 *	In those cases, the EAP module creates a
	 *	"fake" request, and recursively passes it
	 *	through the authentication stage of the
	 *	server.  The module then checks if the request
	 *	was supposed to be proxied, and if so, creates
	 *	a proxy packet from the TUNNELED request, and
	 *	not from the EAP request outside of the
	 *	tunnel.
	 *
	 *	The proxy then works like normal, except that
	 *	the response packet is "eaten" by the EAP
	 *	module, and encapsulated into an EAP packet.
	 */
	if (!request->proxy) {
		request->proxy = rad_alloc(request, true);
		if (!request->proxy) {
			ERROR("no memory");
			fr_exit(1);
		}

		/*
		 *	Copy the request, then look up name
		 *	and plain-text password in the copy.
		 *
		 *	Note that the User-Name attribute is
		 *	the *original* as sent over by the
		 *	client.  The Stripped-User-Name
		 *	attribute is the one hacked through
		 *	the 'hints' file.
		 */
		request->proxy->vps = paircopy(request->proxy,
					       request->packet->vps);
	}

	/*
	 *	Update the various fields as appropriate.
	 */
	request->proxy->src_ipaddr = home->src_ipaddr;
	request->proxy->src_port = 0;
	request->proxy->dst_ipaddr = home->ipaddr;
	request->proxy->dst_port = home->port;
	request->home_server = home;

	/*
	 *	Access-Requests have a Message-Authenticator added,
	 *	unless one already exists.
	 */
	if ((request->packet->code == PW_AUTHENTICATION_REQUEST) &&
	    !pairfind(request->proxy->vps, PW_MESSAGE_AUTHENTICATOR, 0, TAG_ANY)) {
		pairmake(request->proxy, &request->proxy->vps,
			 "Message-Authenticator", "0x00",
			 T_OP_SET);
	}
}

home_server *home_server_ldb(char const *realmname,
			     home_pool_t *pool, REQUEST *request)
{
	int		start;
	int		count;
	home_server	*found = NULL;
	home_server	*zombie = NULL;
	VALUE_PAIR	*vp;

	/*
	 *	Determine how to pick choose the home server.
	 */
	switch (pool->type) {
		uint32_t hash;

		/*
		 *	For load-balancing by client IP address, we
		 *	pick a home server by hashing the client IP.
		 *
		 *	This isn't as even a load distribution as
		 *	tracking the State attribute, but it's better
		 *	than nothing.
		 */
	case HOME_POOL_CLIENT_BALANCE:
		switch (request->packet->src_ipaddr.af) {
		case AF_INET:
			hash = fr_hash(&request->packet->src_ipaddr.ipaddr.ip4addr,
					 sizeof(request->packet->src_ipaddr.ipaddr.ip4addr));
			break;
		case AF_INET6:
			hash = fr_hash(&request->packet->src_ipaddr.ipaddr.ip6addr,
					 sizeof(request->packet->src_ipaddr.ipaddr.ip6addr));
			break;
		default:
			hash = 0;
			break;
		}
		start = hash % pool->num_home_servers;
		break;

	case HOME_POOL_CLIENT_PORT_BALANCE:
		switch (request->packet->src_ipaddr.af) {
		case AF_INET:
			hash = fr_hash(&request->packet->src_ipaddr.ipaddr.ip4addr,
					 sizeof(request->packet->src_ipaddr.ipaddr.ip4addr));
			break;
		case AF_INET6:
			hash = fr_hash(&request->packet->src_ipaddr.ipaddr.ip6addr,
					 sizeof(request->packet->src_ipaddr.ipaddr.ip6addr));
			break;
		default:
			hash = 0;
			break;
		}
		fr_hash_update(&request->packet->src_port,
				 sizeof(request->packet->src_port), hash);
		start = hash % pool->num_home_servers;
		break;

	case HOME_POOL_KEYED_BALANCE:
		if ((vp = pairfind(request->config_items, PW_LOAD_BALANCE_KEY, 0, TAG_ANY)) != NULL) {
			hash = fr_hash(vp->vp_strvalue, vp->length);
			start = hash % pool->num_home_servers;
			break;
		}
		/* FALL-THROUGH */

	case HOME_POOL_LOAD_BALANCE:
	case HOME_POOL_FAIL_OVER:
		start = 0;
		break;

	default:		/* this shouldn't happen... */
		start = 0;
		break;

	}

	/*
	 *	Starting with the home server we chose, loop through
	 *	all home servers.  If the current one is dead, skip
	 *	it.  If it is too busy, skip it.
	 *
	 *	Otherwise, use it.
	 */
	for (count = 0; count < pool->num_home_servers; count++) {
		home_server *home = pool->servers[(start + count) % pool->num_home_servers];

		if (!home) continue;

		/*
		 *	Skip dead home servers.
		 *
		 *	Home servers that are unknown, alive, or zombie
		 *	are used for proxying.
		 */
		if (home->state == HOME_STATE_IS_DEAD) {
			continue;
		}

		/*
		 *	This home server is too busy.  Choose another one.
		 */
		if (home->currently_outstanding >= home->max_outstanding) {
			continue;
		}

#ifdef WITH_DETAIL
		/*
		 *	We read the packet from a detail file, AND it
		 *	came from this server.  Don't re-proxy it
		 *	there.
		 */
		if ((request->listener->type == RAD_LISTEN_DETAIL) &&
		    (request->packet->code == PW_ACCOUNTING_REQUEST) &&
		    (fr_ipaddr_cmp(&home->ipaddr, &request->packet->src_ipaddr) == 0)) {
			continue;
		}
#endif

		/*
		 *	Default virtual: ignore homes tied to a
		 *	virtual.
		 */
		if (!request->server && home->parent_server) {
			continue;
		}

		/*
		 *	A virtual AND home is tied to virtual,
		 *	ignore ones which don't match.
		 */
		if (request->server && home->parent_server &&
		    strcmp(request->server, home->parent_server) != 0) {
			continue;
		}

		/*
		 *	Allow request->server && !home->parent_server
		 *
		 *	i.e. virtuals can proxy to globally defined
		 *	homes.
		 */

		/*
		 *	It's zombie, so we remember the first zombie
		 *	we find, but we don't mark it as a "live"
		 *	server.
		 */
		if (home->state == HOME_STATE_ZOMBIE) {
			if (!zombie) zombie = home;
			continue;
		}

		/*
		 *	We've found the first "live" one.  Use that.
		 */
		if (pool->type != HOME_POOL_LOAD_BALANCE) {
			found = home;
			break;
		}

		/*
		 *	Otherwise we're doing some kind of load balancing.
		 *	If we haven't found one yet, pick this one.
		 */
		if (!found) {
			found = home;
			continue;
		}

		RDEBUG3("PROXY %s %d\t%s %d",
		       found->name, found->currently_outstanding,
		       home->name, home->currently_outstanding);

		/*
		 *	Prefer this server if it's less busy than the
		 *	one we had previously found.
		 */
		if (home->currently_outstanding < found->currently_outstanding) {
			RDEBUG3("PROXY Choosing %s: It's less busy than %s",
			       home->name, found->name);
			found = home;
			continue;
		}

		/*
		 *	Ignore servers which are busier than the one
		 *	we found.
		 */
		if (home->currently_outstanding > found->currently_outstanding) {
			RDEBUG3("PROXY Skipping %s: It's busier than %s",
			       home->name, found->name);
			continue;
		}

		/*
		 *	From the list of servers which have the same
		 *	load, choose one at random.
		 */
		if (((count + 1) * (fr_rand() & 0xffff)) < (uint32_t) 0x10000) {
			found = home;
		}
	} /* loop over the home servers */

	/*
	 *	We have no live servers, BUT we have a zombie.  Use
	 *	the zombie as a last resort.
	 */
	if (!found && zombie) {
		found = zombie;
		zombie = NULL;
	}

	/*
	 *	There's a fallback if they're all dead.
	 */
	if (!found && pool->fallback) {
		found = pool->fallback;

		WDEBUG("Home server pool %s failing over to fallback %s",
		      pool->name, found->server);
		if (pool->in_fallback) goto update_and_return;

		pool->in_fallback = true;

		/*
		 *      Run the trigger once an hour saying that
		 *      they're all dead.
		 */
		if ((pool->time_all_dead + 3600) < request->timestamp) {
			pool->time_all_dead = request->timestamp;
			exec_trigger(request, pool->cs, "home_server_pool.fallback", false);
		}
	}

	if (found) {
	update_and_return:
		if ((found != pool->fallback) && pool->in_fallback) {
			pool->in_fallback = false;
			exec_trigger(request, pool->cs, "home_server_pool.normal", false);
		}

		return found;
	}

	/*
	 *	No live match found, and no fallback to the "DEFAULT"
	 *	realm.  We fix this by blindly marking all servers as
	 *	"live".  But only do it for ones that don't support
	 *	"pings", as they will be marked live when they
	 *	actually are live.
	 */
	if (!realm_config->fallback &&
	    realm_config->wake_all_if_all_dead) {
		for (count = 0; count < pool->num_home_servers; count++) {
			home_server *home = pool->servers[count];

			if (!home) continue;

			if ((home->state == HOME_STATE_IS_DEAD) &&
			    (home->ping_check == HOME_PING_CHECK_NONE)) {
				home->state = HOME_STATE_ALIVE;
				if (!found) found = home;
			}
		}

		if (found) goto update_and_return;
	}

	/*
	 *	Still nothing.  Look up the DEFAULT realm, but only
	 *	if we weren't looking up the NULL or DEFAULT realms.
	 */
	if (realm_config->fallback &&
	    realmname &&
	    (strcmp(realmname, "NULL") != 0) &&
	    (strcmp(realmname, "DEFAULT") != 0)) {
		REALM *rd = realm_find("DEFAULT");

		if (!rd) return NULL;

		pool = NULL;
		if (request->packet->code == PW_AUTHENTICATION_REQUEST) {
			pool = rd->auth_pool;

		} else if (request->packet->code == PW_ACCOUNTING_REQUEST) {
			pool = rd->acct_pool;
		}
		if (!pool) return NULL;

		RDEBUG2("PROXY - realm %s has no live home servers.  Falling back to the DEFAULT realm.", realmname);
		return home_server_ldb(rd->name, pool, request);
	}

	/*
	 *	Still haven't found anything.  Oh well.
	 */
	return NULL;
}


home_server *home_server_find(fr_ipaddr_t *ipaddr, int port, int proto)
{
	home_server myhome;

	memset(&myhome, 0, sizeof(myhome));
	myhome.ipaddr = *ipaddr;
	myhome.src_ipaddr.af = ipaddr->af;
	myhome.port = port;
#ifdef WITH_TCP
	myhome.proto = proto;
#else
	myhome.proto = IPPROTO_UDP;
#endif
	myhome.server = NULL;	/* we're not called for internal proxying */

	return rbtree_finddata(home_servers_byaddr, &myhome);
}

#ifdef WITH_COA
home_server *home_server_byname(char const *name, int type)
{
	home_server myhome;

	memset(&myhome, 0, sizeof(myhome));
	myhome.type = type;
	myhome.name = name;

	return rbtree_finddata(home_servers_byname, &myhome);
}
#endif

#ifdef WITH_STATS
home_server *home_server_bynumber(int number)
{
	home_server myhome;

	memset(&myhome, 0, sizeof(myhome));
	myhome.number = number;
	myhome.server = NULL;	/* we're not called for internal proxying */

	return rbtree_finddata(home_servers_bynumber, &myhome);
}
#endif

home_pool_t *home_pool_byname(char const *name, int type)
{
	home_pool_t mypool;

	memset(&mypool, 0, sizeof(mypool));
	mypool.name = name;
	mypool.server_type = type;
	return rbtree_finddata(home_pools_byname, &mypool);
}

#endif
