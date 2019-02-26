/*
 * Copyright (C) 2008 Martin Willi
 * HSR Hochschule fuer Technik Rapperswil
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

#include "load_tester_config.h"

#include <netdb.h>

#include <daemon.h>
#include <attributes/mem_pool.h>
#include <collections/hashtable.h>
#include <threading/mutex.h>

#define UNIQUE_PORT_START 1025

typedef struct private_load_tester_config_t private_load_tester_config_t;

/**
 * Private data of an load_tester_config_t object
 */
struct private_load_tester_config_t {

	/**
	 * Public part
	 */
	load_tester_config_t public;

	/**
	 * peer config
	 */
	peer_cfg_t *peer_cfg;

	/**
	 * virtual IP, if any
	 */
	host_t *vip;

	/**
	 * Initiator address
	 */
	char *initiator;

	/**
	 * Responder address
	 */
	char *responder;

	/**
	 * IP address pool
	 */
	char *pool;

	/**
	 * IKE proposal
	 */
	proposal_t *proposal;

	/**
	 * ESP proposal
	 */
	proposal_t *esp;

	/**
	 * Authentication method(s) to use/expect from initiator
	 */
	char *initiator_auth;

	/**
	 * Authentication method(s) use/expected from responder
	 */
	char *responder_auth;

	/**
	 * Initiator ID to enforce
	 */
	char *initiator_id;

	/**
	 * Initiator ID to to match against as responder
	 */
	char *initiator_match;

	/**
	 * Responder ID to enforce
	 */
	char *responder_id;

	/**
	 * IPsec mode, tunnel|transport|beet
	 */
	char *mode;

	/**
	 * Traffic Selector on initiator side, as proposed from initiator
	 */
	char *initiator_tsi;

	/**
	 * Traffic Selector on responder side, as proposed from initiator
	 */
	char *initiator_tsr;

	/**
	 * Traffic Selector on initiator side, as narrowed by responder
	 */
	char *responder_tsi;

	/**
	 * Traffic Selector on responder side, as narrowed by responder
	 */
	char *responder_tsr;

	/**
	 * Current port for unique initiator ports
	 */
	uint16_t unique_port;

	/**
	 * IKE_SA rekeying delay
	 */
	u_int ike_rekey;

	/**
	 * CHILD_SA rekeying delay
	 */
	u_int child_rekey;

	/**
	 * DPD check delay
	 */
	u_int dpd_delay;

	/**
	 * DPD timeout (IKEv1 only)
	 */
	u_int dpd_timeout;

	/**
	 * incremental numbering of generated configs
	 */
	refcount_t num;

	/**
	 * Dynamic source port, if used
	 */
	uint16_t port;

	/**
	 * IKE version to use for load testing
	 */
	ike_version_t version;

	/**
	 * List of pools to allocate external addresses dynamically, as mem_pool_t
	 */
	linked_list_t *pools;

	/**
	 * Address prefix to use when installing dynamic addresses
	 */
	int prefix;

	/**
	 * Keep addresses until shutdown?
	 */
	bool keep;

	/**
	 * Hashtable with leases in "pools", host_t => entry_t
	 */
	hashtable_t *leases;

	/**
	 * Mutex for leases hashtable
	 */
	mutex_t *mutex;
};

/**
 * Lease entry
 */
typedef struct {
	/** host reference, equal to key */
	host_t *host;
	/** associated identity */
	identification_t *id;
} entry_t;

/**
 * Destroy an entry_t
 */
static void entry_destroy(entry_t *this)
{
	this->host->destroy(this->host);
	this->id->destroy(this->id);
	free(this);
}

/**
 * Hashtable hash function
 */
static u_int hash(host_t *key)
{
	return chunk_hash(key->get_address(key));
}

/**
 * Hashtable equals function
 */
static bool equals(host_t *a, host_t *b)
{
	return a->ip_equals(a, b);
}

/**
 * Load external addresses to use, if any
 */
static void load_addrs(private_load_tester_config_t *this)
{
	enumerator_t *enumerator, *tokens;
	host_t *from, *to;
	int bits;
	char *iface, *token, *pos;
	mem_pool_t *pool;

	this->keep = lib->settings->get_bool(lib->settings,
						"%s.plugins.load-tester.addrs_keep", FALSE, lib->ns);
	this->prefix = lib->settings->get_int(lib->settings,
						"%s.plugins.load-tester.addrs_prefix", 16, lib->ns);
	enumerator = lib->settings->create_key_value_enumerator(lib->settings,
						"%s.plugins.load-tester.addrs", lib->ns);
	while (enumerator->enumerate(enumerator, &iface, &token))
	{
		tokens = enumerator_create_token(token, ",", " ");
		while (tokens->enumerate(tokens, &token))
		{
			pos = strchr(token, '-');
			if (pos)
			{	/* range */
				*(pos++) = '\0';
				/* trim whitespace */
				while (*pos == ' ')
				{
					pos++;
				}
				while (token[strlen(token) - 1] == ' ')
				{
					token[strlen(token) - 1] = '\0';
				}
				from = host_create_from_string(token, 0);
				to = host_create_from_string(pos, 0);
				if (from && to)
				{
					pool = mem_pool_create_range(iface, from, to);
					if (pool)
					{
						DBG1(DBG_CFG, "loaded load-tester address range "
							 "%H-%H on %s", from, to, iface);
						this->pools->insert_last(this->pools, pool);
					}
					from->destroy(from);
					to->destroy(to);
				}
				else
				{
					DBG1(DBG_CFG, "parsing load-tester address range %s-%s "
						 "failed, skipped", token, pos);
					DESTROY_IF(from);
					DESTROY_IF(to);
				}
			}
			else
			{	/* subnet */
				from = host_create_from_subnet(token, &bits);
				if (from)
				{
					DBG1(DBG_CFG, "loaded load-tester address pool %H/%d on %s",
						 from, bits, iface);
					pool = mem_pool_create(iface, from, bits);
					from->destroy(from);
					this->pools->insert_last(this->pools, pool);
				}
				else
				{
					DBG1(DBG_CFG, "parsing load-tester address %s failed, "
						 "skipped", token);
				}
			}
		}
		tokens->destroy(tokens);
	}
	enumerator->destroy(enumerator);
}

/**
 * Generate auth config from string
 */
static void generate_auth_cfg(private_load_tester_config_t *this, char *str,
							  peer_cfg_t *peer_cfg, bool local, int num)
{
	enumerator_t *enumerator;
	auth_cfg_t *auth;
	identification_t *id;
	auth_class_t class;
	eap_type_t type;
	char buf[128];
	int rnd = 0;

	enumerator = enumerator_create_token(str, "|", " ");
	while (enumerator->enumerate(enumerator, &str))
	{
		id = NULL;
		auth = auth_cfg_create();
		rnd++;

		if (this->initiator_id)
		{
			if (this->initiator_match && (!local && !num))
			{	/* as responder, use the secified identity that matches
				 * all used initiator identities, if given. */
				snprintf(buf, sizeof(buf), this->initiator_match, rnd);
				id = identification_create_from_string(buf);
			}
			else if ((local && num) || (!local && !num))
			{	/* as initiator, create peer specific identities */
				snprintf(buf, sizeof(buf), this->initiator_id, num, rnd);
				id = identification_create_from_string(buf);
			}
		}
		if (this->responder_id)
		{
			if ((local && !num) || (!local && num))
			{
				snprintf(buf, sizeof(buf), this->responder_id, num, rnd);
				id = identification_create_from_string(buf);
			}
		}

		if (streq(str, "psk"))
		{	/* PSK authentication, use FQDNs */
			class = AUTH_CLASS_PSK;
			if (!id)
			{
				if ((local && !num) || (!local && num))
				{
					id = identification_create_from_string("srv.strongswan.org");
				}
				else if (local)
				{
					snprintf(buf, sizeof(buf), "c%d-r%d.strongswan.org",
							 num, rnd);
					id = identification_create_from_string(buf);
				}
				else
				{
					id = identification_create_from_string("*.strongswan.org");
				}
			}
		}
		else if (strpfx(str, "eap"))
		{	/* EAP authentication, use a NAI */
			class = AUTH_CLASS_EAP;
			if (*(str + strlen("eap")) == '-')
			{
				type = eap_type_from_string(str + strlen("eap-"));
				if (type)
				{
					auth->add(auth, AUTH_RULE_EAP_TYPE, type);
				}
			}
			if (!id)
			{
				if (local && num)
				{
					snprintf(buf, sizeof(buf), "1%.10d%.4d@strongswan.org",
							 num, rnd);
					id = identification_create_from_string(buf);
				}
				else
				{
					id = identification_create_from_encoding(ID_ANY, chunk_empty);
				}
			}
		}
		else if (strpfx(str, "xauth"))
		{	/* XAuth, use a username */
			class = AUTH_CLASS_XAUTH;
			if (*(str + strlen("xauth")) == '-')
			{
				auth->add(auth, AUTH_RULE_XAUTH_BACKEND, str + strlen("xauth-"));
			}
			if (!id)
			{
				if (local && num)
				{
					snprintf(buf, sizeof(buf), "cli-%.6d-%.2d", num, rnd);
					id = identification_create_from_string(buf);
				}
				else
				{
					id = identification_create_from_encoding(ID_ANY, chunk_empty);
				}
			}
			/* additionally set the ID as XAuth identity */
			auth->add(auth, AUTH_RULE_XAUTH_IDENTITY, id->clone(id));
		}
		else
		{
			if (!streq(str, "pubkey"))
			{
				DBG1(DBG_CFG, "invalid authentication: '%s', fallback to pubkey",
					 str);
			}
			/* certificate authentication, use distinguished names */
			class = AUTH_CLASS_PUBKEY;
			if (!id)
			{
				if ((local && !num) || (!local && num))
				{
					id = identification_create_from_string(
								"CN=srv, OU=load-test, O=strongSwan");
				}
				else if (local)
				{
					snprintf(buf, sizeof(buf),
							 "CN=c%d-r%d, OU=load-test, O=strongSwan", num, rnd);
					id = identification_create_from_string(buf);
				}
				else
				{
					id = identification_create_from_string(
									"CN=*, OU=load-test, O=strongSwan");
				}
			}
		}
		auth->add(auth, AUTH_RULE_AUTH_CLASS, class);
		auth->add(auth, AUTH_RULE_IDENTITY, id);
		peer_cfg->add_auth_cfg(peer_cfg, auth, local);
	}
	enumerator->destroy(enumerator);
}

/**
 * Parse a protoport specifier
 */
static bool parse_protoport(char *token, uint16_t *from_port,
							uint16_t *to_port, uint8_t *protocol)
{
	char *sep, *port = "", *endptr;
	struct protoent *proto;
	struct servent *svc;
	long int p;

	sep = strrchr(token, ']');
	if (!sep)
	{
		return FALSE;
	}
	*sep = '\0';

	sep = strchr(token, '/');
	if (sep)
	{	/* protocol/port */
		*sep = '\0';
		port = sep + 1;
	}

	if (streq(token, "%any"))
	{
		*protocol = 0;
	}
	else
	{
		proto = getprotobyname(token);
		if (proto)
		{
			*protocol = proto->p_proto;
		}
		else
		{
			p = strtol(token, &endptr, 0);
			if ((*token && *endptr) || p < 0 || p > 0xff)
			{
				return FALSE;
			}
			*protocol = (uint8_t)p;
		}
	}
	if (streq(port, "%any"))
	{
		*from_port = 0;
		*to_port = 0xffff;
	}
	else if (streq(port, "%opaque"))
	{
		*from_port = 0xffff;
		*to_port = 0;
	}
	else if (streq(port, "%unique"))
	{
		*from_port = *to_port = 0;
	}
	else if (*port)
	{
		svc = getservbyname(port, NULL);
		if (svc)
		{
			*from_port = *to_port = ntohs(svc->s_port);
		}
		else
		{
			p = strtol(port, &endptr, 0);
			if (p < 0 || p > 0xffff)
			{
				return FALSE;
			}
			*from_port = p;
			if (*endptr == '-')
			{
				port = endptr + 1;
				p = strtol(port, &endptr, 0);
				if (p < 0 || p > 0xffff)
				{
					return FALSE;
				}
			}
			*to_port = p;
			if (*endptr)
			{
				return FALSE;
			}
		}
	}
	return TRUE;
}

/**
 * Add a TS from a string to a child_cfg
 */
static void add_ts(private_load_tester_config_t *this,
				   char *string, child_cfg_t *cfg, bool local, bool initiator)
{
	traffic_selector_t *ts;

	if (string)
	{
		enumerator_t *enumerator;
		char *subnet, *pos;
		uint16_t from_port, to_port;
		uint8_t proto;

		enumerator = enumerator_create_token(string, ",", " ");
		while (enumerator->enumerate(enumerator, &subnet))
		{
			proto = 0;
			from_port = 0;
			to_port = 65535;

			pos = strchr(subnet, '[');
			if (pos)
			{
				*(pos++) = '\0';
				if (!parse_protoport(pos, &from_port, &to_port, &proto))
				{
					DBG1(DBG_CFG, "invalid proto/port: %s, skipped subnet",
						 pos);
					continue;
				}
			}
			if (from_port == 0 && to_port == 0)
			{	/* %unique */
				if (initiator)
				{
					from_port = this->unique_port++;
					from_port = to_port = max(from_port, UNIQUE_PORT_START);
				}
				else
				{	/* not supported as responder, use %any */
					to_port = 65535;
				}
			}
			if (streq(subnet, "%dynamic"))
			{
				ts = traffic_selector_create_dynamic(proto,
													 from_port, to_port);
			}
			else
			{
				ts = traffic_selector_create_from_cidr(subnet, proto,
													   from_port, to_port);
			}
			if (ts)
			{
				cfg->add_traffic_selector(cfg, local, ts);
			}
			else
			{
				DBG1(DBG_CFG, "invalid subnet: %s, skipped", subnet);
			}
		}
		enumerator->destroy(enumerator);
	}
	else
	{
		ts = traffic_selector_create_dynamic(0, 0, 65535);
		if (ts)
		{
			cfg->add_traffic_selector(cfg, local, ts);
		}
	}
}

/**
 * Allocate and install a dynamic external address to use
 */
static host_t *allocate_addr(private_load_tester_config_t *this, uint num)
{
	enumerator_t *enumerator;
	mem_pool_t *pool;
	host_t *found = NULL, *requested;
	identification_t *id;
	char *iface = NULL, buf[32];
	entry_t *entry;

	requested = host_create_any(AF_INET);
	snprintf(buf, sizeof(buf), "ext-%d", num);
	id = identification_create_from_string(buf);
	enumerator = this->pools->create_enumerator(this->pools);
	while (enumerator->enumerate(enumerator, &pool))
	{
		found = pool->acquire_address(pool, id, requested, MEM_POOL_NEW, NULL);
		if (found)
		{
			iface = (char*)pool->get_name(pool);
			break;
		}
	}
	enumerator->destroy(enumerator);
	requested->destroy(requested);

	if (!found)
	{
		DBG1(DBG_CFG, "no address found to install as load-tester external IP");
		id->destroy(id);
		return NULL;
	}
	if (charon->kernel->add_ip(charon->kernel, found, this->prefix,
							   iface) != SUCCESS)
	{
		DBG1(DBG_CFG, "installing load-tester IP %H on %s failed", found, iface);
		found->destroy(found);
		id->destroy(id);
		return NULL;
	}
	DBG1(DBG_CFG, "installed load-tester IP %H on %s", found, iface);
	INIT(entry,
		.host = found->clone(found),
		.id = id,
	);
	this->mutex->lock(this->mutex);
	entry = this->leases->put(this->leases, entry->host, entry);
	this->mutex->unlock(this->mutex);
	if (entry)
	{	/* shouldn't actually happen */
		entry_destroy(entry);
	}
	return found;
}

/**
 * Generate a new initiator config, num = 0 for responder config
 */
static peer_cfg_t* generate_config(private_load_tester_config_t *this, uint num)
{
	ike_cfg_t *ike_cfg;
	child_cfg_t *child_cfg;
	peer_cfg_t *peer_cfg;
	char local[32], *remote;
	host_t *addr;
	peer_cfg_create_t peer = {
		.cert_policy = CERT_SEND_IF_ASKED,
		.unique = UNIQUE_NO,
		.keyingtries = 1,
		.rekey_time = this->ike_rekey,
		.over_time = this->ike_rekey,
		.no_mobike = TRUE,
		.dpd = this->dpd_delay,
		.dpd_timeout = this->dpd_timeout,
	};
	child_cfg_create_t child = {
		.lifetime = {
			.time = {
				.life = this->child_rekey * 2,
				.rekey = this->child_rekey,
				.jitter = 0
			},
		},
		.mode = MODE_TUNNEL,
	};

	if (num)
	{	/* initiator */
		if (this->pools->get_count(this->pools))
		{	/* using dynamically installed external addresses */
			addr = allocate_addr(this, num);
			if (!addr)
			{
				DBG1(DBG_CFG, "allocating external address failed");
				return NULL;
			}
			snprintf(local, sizeof(local), "%H", addr);
			addr->destroy(addr);
		}
		else
		{
			snprintf(local, sizeof(local), "%s", this->initiator);
		}
		remote = this->responder;
	}
	else
	{
		snprintf(local, sizeof(local), "%s", this->responder);
		remote = this->initiator;
	}

	if (this->port && num)
	{
		ike_cfg = ike_cfg_create(this->version, TRUE, FALSE,
								 local, this->port + num - 1,
								 remote, IKEV2_NATT_PORT,
								 FRAGMENTATION_NO, 0);
	}
	else
	{
		ike_cfg = ike_cfg_create(this->version, TRUE, FALSE, local,
								 charon->socket->get_port(charon->socket, FALSE),
								 remote, IKEV2_UDP_PORT,
								 FRAGMENTATION_NO, 0);
	}
	ike_cfg->add_proposal(ike_cfg, this->proposal->clone(this->proposal));
	peer_cfg = peer_cfg_create("load-test", ike_cfg, &peer);

	if (this->vip)
	{
		peer_cfg->add_virtual_ip(peer_cfg, this->vip->clone(this->vip));
	}
	if (this->pool)
	{
		peer_cfg->add_pool(peer_cfg, this->pool);
	}
	if (num)
	{	/* initiator */
		generate_auth_cfg(this, this->initiator_auth, peer_cfg, TRUE, num);
		generate_auth_cfg(this, this->responder_auth, peer_cfg, FALSE, num);
	}
	else
	{	/* responder */
		generate_auth_cfg(this, this->responder_auth, peer_cfg, TRUE, num);
		generate_auth_cfg(this, this->initiator_auth, peer_cfg, FALSE, num);
	}

	if (this->mode)
	{
		if (streq(this->mode, "transport"))
		{
			child.mode = MODE_TRANSPORT;
		}
		else if (streq(this->mode, "beet"))
		{
			child.mode = MODE_BEET;
		}
	}

	child_cfg = child_cfg_create("load-test", &child);
	child_cfg->add_proposal(child_cfg, this->esp->clone(this->esp));

	if (num)
	{	/* initiator */
		if (this->vip)
		{
			add_ts(this, NULL, child_cfg, TRUE, TRUE);
		}
		else
		{
			add_ts(this, this->initiator_tsi, child_cfg, TRUE, TRUE);
		}
		add_ts(this, this->initiator_tsr, child_cfg, FALSE, TRUE);
	}
	else
	{	/* responder */
		add_ts(this, this->responder_tsr, child_cfg, TRUE, FALSE);
		add_ts(this, this->responder_tsi, child_cfg, FALSE, FALSE);
	}
	peer_cfg->add_child_cfg(peer_cfg, child_cfg);
	return peer_cfg;
}

METHOD(backend_t, create_peer_cfg_enumerator, enumerator_t*,
	private_load_tester_config_t *this,
	identification_t *me, identification_t *other)
{
	return enumerator_create_single(this->peer_cfg, NULL);
}

METHOD(backend_t, create_ike_cfg_enumerator, enumerator_t*,
	private_load_tester_config_t *this, host_t *me, host_t *other)
{
	ike_cfg_t *ike_cfg;

	ike_cfg = this->peer_cfg->get_ike_cfg(this->peer_cfg);
	return enumerator_create_single(ike_cfg, NULL);
}

METHOD(backend_t, get_peer_cfg_by_name, peer_cfg_t*,
	private_load_tester_config_t *this, char *name)
{
	if (streq(name, "load-test"))
	{
		return generate_config(this, (u_int)ref_get(&this->num));
	}
	return NULL;
}

METHOD(load_tester_config_t, delete_ip, void,
	private_load_tester_config_t *this, host_t *ip)
{
	enumerator_t *enumerator;
	mem_pool_t *pool;
	entry_t *entry;

	if (this->keep)
	{
		return;
	}

	this->mutex->lock(this->mutex);
	entry = this->leases->remove(this->leases, ip);
	this->mutex->unlock(this->mutex);

	if (entry)
	{
		enumerator = this->pools->create_enumerator(this->pools);
		while (enumerator->enumerate(enumerator, &pool))
		{
			if (pool->release_address(pool, entry->host, entry->id))
			{
				charon->kernel->del_ip(charon->kernel, entry->host,
									   this->prefix, FALSE);
				break;
			}
		}
		enumerator->destroy(enumerator);
		entry_destroy(entry);
	}
}

/**
 * Clean up leases for allocated external addresses, if have been kept
 */
static void cleanup_leases(private_load_tester_config_t *this)
{
	enumerator_t *pools, *leases;
	mem_pool_t *pool;
	identification_t *id;
	host_t *addr;
	entry_t *entry;
	bool online;

	pools = this->pools->create_enumerator(this->pools);
	while (pools->enumerate(pools, &pool))
	{
		leases = pool->create_lease_enumerator(pool);
		while (leases->enumerate(leases, &id, &addr, &online))
		{
			if (online)
			{
				charon->kernel->del_ip(charon->kernel, addr, this->prefix,
									   FALSE);
				entry = this->leases->remove(this->leases, addr);
				if (entry)
				{
					entry_destroy(entry);
				}
			}
		}
		leases->destroy(leases);
	}
	pools->destroy(pools);
}

METHOD(load_tester_config_t, destroy, void,
	private_load_tester_config_t *this)
{
	if (this->keep)
	{
		cleanup_leases(this);
	}
	this->mutex->destroy(this->mutex);
	this->leases->destroy(this->leases);
	this->pools->destroy_offset(this->pools, offsetof(mem_pool_t, destroy));
	this->peer_cfg->destroy(this->peer_cfg);
	DESTROY_IF(this->proposal);
	DESTROY_IF(this->esp);
	DESTROY_IF(this->vip);
	free(this);
}

/**
 * Described in header.
 */
load_tester_config_t *load_tester_config_create()
{
	private_load_tester_config_t *this;

	INIT(this,
		.public = {
			.backend = {
				.create_peer_cfg_enumerator = _create_peer_cfg_enumerator,
				.create_ike_cfg_enumerator = _create_ike_cfg_enumerator,
				.get_peer_cfg_by_name = _get_peer_cfg_by_name,
			},
			.delete_ip = _delete_ip,
			.destroy = _destroy,
		},
		.pools = linked_list_create(),
		.leases = hashtable_create((hashtable_hash_t)hash,
								   (hashtable_equals_t)equals, 256),
		.mutex = mutex_create(MUTEX_TYPE_DEFAULT),
		.unique_port = UNIQUE_PORT_START,
	);

	if (lib->settings->get_bool(lib->settings,
				"%s.plugins.load-tester.request_virtual_ip", FALSE, lib->ns))
	{
		this->vip = host_create_from_string("0.0.0.0", 0);
	}
	this->pool = lib->settings->get_str(lib->settings,
					"%s.plugins.load-tester.pool", NULL, lib->ns);
	this->initiator = lib->settings->get_str(lib->settings,
					"%s.plugins.load-tester.initiator", "0.0.0.0", lib->ns);
	this->responder = lib->settings->get_str(lib->settings,
					"%s.plugins.load-tester.responder", "127.0.0.1", lib->ns);

	this->proposal = proposal_create_from_string(PROTO_IKE,
				lib->settings->get_str(lib->settings,
					"%s.plugins.load-tester.proposal", "aes128-sha1-modp768",
					lib->ns));
	if (!this->proposal)
	{	/* fallback */
		this->proposal = proposal_create_from_string(PROTO_IKE,
													 "aes128-sha1-modp768");
	}
	this->esp = proposal_create_from_string(PROTO_ESP,
				lib->settings->get_str(lib->settings,
					"%s.plugins.load-tester.esp", "aes128-sha1", lib->ns));
	if (!this->esp)
	{	/* fallback */
		this->esp = proposal_create_from_string(PROTO_ESP, "aes128-sha1");
	}

	this->ike_rekey = lib->settings->get_int(lib->settings,
				"%s.plugins.load-tester.ike_rekey", 0, lib->ns);
	this->child_rekey = lib->settings->get_int(lib->settings,
				"%s.plugins.load-tester.child_rekey", 600, lib->ns);
	this->dpd_delay = lib->settings->get_int(lib->settings,
				"%s.plugins.load-tester.dpd_delay", 0, lib->ns);
	this->dpd_timeout = lib->settings->get_int(lib->settings,
				"%s.plugins.load-tester.dpd_timeout", 0, lib->ns);

	this->initiator_auth = lib->settings->get_str(lib->settings,
				"%s.plugins.load-tester.initiator_auth", "pubkey", lib->ns);
	this->responder_auth = lib->settings->get_str(lib->settings,
				"%s.plugins.load-tester.responder_auth", "pubkey", lib->ns);
	this->initiator_id = lib->settings->get_str(lib->settings,
				"%s.plugins.load-tester.initiator_id", NULL, lib->ns);
	this->initiator_match = lib->settings->get_str(lib->settings,
				"%s.plugins.load-tester.initiator_match", NULL, lib->ns);
	this->responder_id = lib->settings->get_str(lib->settings,
				"%s.plugins.load-tester.responder_id", NULL, lib->ns);

	this->mode = lib->settings->get_str(lib->settings,
				"%s.plugins.load-tester.mode", NULL, lib->ns);
	this->initiator_tsi = lib->settings->get_str(lib->settings,
				"%s.plugins.load-tester.initiator_tsi", NULL, lib->ns);
	this->responder_tsi =lib->settings->get_str(lib->settings,
				"%s.plugins.load-tester.responder_tsi",
				this->initiator_tsi, lib->ns);
	this->initiator_tsr = lib->settings->get_str(lib->settings,
				"%s.plugins.load-tester.initiator_tsr", NULL, lib->ns);
	this->responder_tsr =lib->settings->get_str(lib->settings,
				"%s.plugins.load-tester.responder_tsr",
				this->initiator_tsr, lib->ns);

	this->port = lib->settings->get_int(lib->settings,
				"%s.plugins.load-tester.dynamic_port", 0, lib->ns);
	this->version = lib->settings->get_int(lib->settings,
				"%s.plugins.load-tester.version", IKE_ANY, lib->ns);

	load_addrs(this);

	this->peer_cfg = generate_config(this, 0);

	return &this->public;
}
