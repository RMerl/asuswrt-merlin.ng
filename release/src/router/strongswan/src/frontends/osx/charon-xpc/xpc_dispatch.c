/*
 * Copyright (C) 2013 Martin Willi
 * Copyright (C) 2013 revosec AG
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

#include "xpc_dispatch.h"
#include "xpc_channels.h"

#include <xpc/xpc.h>
#include <signal.h>
#include <unistd.h>

#include <daemon.h>
#include <processing/jobs/callback_job.h>

typedef struct private_xpc_dispatch_t private_xpc_dispatch_t;

/**
 * Private data of an xpc_dispatch_t object.
 */
struct private_xpc_dispatch_t {

	/**
	 * Public xpc_dispatch_t interface.
	 */
	xpc_dispatch_t public;

	/**
	 * XPC service we offer
	 */
	xpc_connection_t service;

	/**
	 * XPC IKE_SA specific channels to App
	 */
	xpc_channels_t *channels;

	/**
	 * GCD queue for XPC events
	 */
	dispatch_queue_t queue;

	/**
	 * Number of active App connections
	 */
	refcount_t refcount;

	/**
	 * PID of main thread
	 */
	pid_t pid;
};

/**
 * Return version of this helper
 */
static void get_version(private_xpc_dispatch_t *this,
						xpc_object_t request, xpc_object_t reply)
{
	xpc_dictionary_set_string(reply, "version", PACKAGE_VERSION);
}

/**
 * Create peer config with associated ike config
 */
static peer_cfg_t* create_peer_cfg(char *name, char *host)
{
	ike_cfg_t *ike_cfg;
	peer_cfg_t *peer_cfg;
	uint16_t local_port, remote_port = IKEV2_UDP_PORT;
	peer_cfg_create_t peer = {
		.cert_policy = CERT_SEND_IF_ASKED,
		.unique = UNIQUE_REPLACE,
		.keyingtries = 1,
		.rekey_time = 36000, /* 10h */
		.jitter_time = 600, /* 10min */
		.over_time = 600, /* 10min */
		.dpd = 30,
	};

	local_port = charon->socket->get_port(charon->socket, FALSE);
	if (local_port != IKEV2_UDP_PORT)
	{
		remote_port = IKEV2_NATT_PORT;
	}
	ike_cfg = ike_cfg_create(IKEV2, FALSE, FALSE, "0.0.0.0", local_port,
							 host, remote_port, FRAGMENTATION_NO, 0);
	ike_cfg->add_proposal(ike_cfg, proposal_create_default(PROTO_IKE));
	ike_cfg->add_proposal(ike_cfg, proposal_create_default_aead(PROTO_IKE));
	peer_cfg = peer_cfg_create(name, ike_cfg, &peer);
	peer_cfg->add_virtual_ip(peer_cfg, host_create_from_string("0.0.0.0", 0));

	return peer_cfg;
}

/**
 * Add a single auth cfg of given class to peer cfg
 */
static void add_auth_cfg(peer_cfg_t *peer_cfg, bool local,
						 char *id, auth_class_t class)
{
	auth_cfg_t *auth;

	auth = auth_cfg_create();
	auth->add(auth, AUTH_RULE_AUTH_CLASS, class);
	auth->add(auth, AUTH_RULE_IDENTITY, identification_create_from_string(id));
	if (!local)
	{
		auth->add(auth, AUTH_RULE_IDENTITY_LOOSE, TRUE);
	}
	peer_cfg->add_auth_cfg(peer_cfg, auth, local);
}

/**
 * Attach child config to peer config
 */
static child_cfg_t* create_child_cfg(char *name)
{
	child_cfg_t *child_cfg;
	traffic_selector_t *ts;
	child_cfg_create_t child = {
		.lifetime = {
			.time = {
				.life = 10800 /* 3h */,
				.rekey = 10200 /* 2h50min */,
				.jitter = 300 /* 5min */
			},
		},
		.mode = MODE_TUNNEL,
	};

	child_cfg = child_cfg_create(name, &child);
	child_cfg->add_proposal(child_cfg, proposal_create_from_string(PROTO_ESP,
										"aes128gcm8-aes128gcm12-aes128gcm16-"
										"aes256gcm8-aes256gcm12-aes256gcm16"));
	child_cfg->add_proposal(child_cfg, proposal_create_default(PROTO_ESP));
	child_cfg->add_proposal(child_cfg, proposal_create_default_aead(PROTO_ESP));
	ts = traffic_selector_create_dynamic(0, 0, 65535);
	child_cfg->add_traffic_selector(child_cfg, TRUE, ts);
	ts = traffic_selector_create_from_string(0, TS_IPV4_ADDR_RANGE,
										"0.0.0.0", 0, "255.255.255.255", 65535);
	child_cfg->add_traffic_selector(child_cfg, FALSE, ts);

	return child_cfg;
}

/**
 * Controller initiate callback
 */
static bool initiate_cb(uint32_t *sa, debug_t group, level_t level,
						ike_sa_t *ike_sa, const char *message)
{
	if (ike_sa)
	{
		*sa = ike_sa->get_unique_id(ike_sa);
		return FALSE;
	}
	return TRUE;
}

/**
 * Start initiating an IKE connection
 */
void start_connection(private_xpc_dispatch_t *this,
					  xpc_object_t request, xpc_object_t reply)
{
	peer_cfg_t *peer_cfg;
	child_cfg_t *child_cfg;
	char *name, *id, *host;
	bool success = FALSE;
	xpc_endpoint_t endpoint;
	xpc_connection_t channel;
	uint32_t ike_sa;

	name = (char*)xpc_dictionary_get_string(request, "name");
	host = (char*)xpc_dictionary_get_string(request, "host");
	id = (char*)xpc_dictionary_get_string(request, "id");
	endpoint = xpc_dictionary_get_value(request, "channel");
	channel = xpc_connection_create_from_endpoint(endpoint);

	if (name && id && host && channel)
	{
		peer_cfg = create_peer_cfg(name, host);

		add_auth_cfg(peer_cfg, TRUE, id, AUTH_CLASS_EAP);
		add_auth_cfg(peer_cfg, FALSE, host, AUTH_CLASS_ANY);

		child_cfg = create_child_cfg(name);
		peer_cfg->add_child_cfg(peer_cfg, child_cfg->get_ref(child_cfg));

		if (charon->controller->initiate(charon->controller, peer_cfg, child_cfg,
				(controller_cb_t)initiate_cb, &ike_sa, 0, FALSE) == NEED_MORE)
		{
			this->channels->add(this->channels, channel, ike_sa);
			success = TRUE;
		}
	}

	xpc_dictionary_set_bool(reply, "success", success);
}

/**
 * XPC RPC command dispatch table
 */
static struct {
	char *name;
	void (*handler)(private_xpc_dispatch_t *this,
					xpc_object_t request, xpc_object_t reply);
} commands[] = {
	{ "get_version", get_version },
	{ "start_connection", start_connection },
};

/**
 * Handle a received XPC request message
 */
static void handle(private_xpc_dispatch_t *this, xpc_object_t request)
{
	xpc_connection_t client;
	xpc_object_t reply;
	const char *type, *rpc;
	bool found = FALSE;
	int i;

	type = xpc_dictionary_get_string(request, "type");
	if (type)
	{
		if (streq(type, "rpc"))
		{
			reply = xpc_dictionary_create_reply(request);
			rpc = xpc_dictionary_get_string(request, "rpc");
			if (reply && rpc)
			{
				for (i = 0; i < countof(commands); i++)
				{
					if (streq(commands[i].name, rpc))
					{
						found = TRUE;
						commands[i].handler(this, request, reply);
						break;
					}
				}
			}
			if (!found)
			{
				DBG1(DBG_CFG, "received invalid XPC rpc command: %s", rpc);
			}
			if (reply)
			{
				client = xpc_dictionary_get_remote_connection(request);
				xpc_connection_send_message(client, reply);
				xpc_release(reply);
			}
		}
		else
		{
			DBG1(DBG_CFG, "received unknown XPC message type: %s", type);
		}
	}
	else
	{
		DBG1(DBG_CFG, "received XPC message without a type");
	}
}

/**
 * Finalizer for client connections
 */
static void cleanup_connection(private_xpc_dispatch_t *this)
{
	if (ref_put(&this->refcount))
	{
		DBG1(DBG_CFG, "no XPC connections, raising SIGTERM");
		kill(this->pid, SIGTERM);
	}
}

/**
 * Set up GCD handler for XPC events
 */
static void set_handler(private_xpc_dispatch_t *this)
{
	xpc_retain(this->service);
	xpc_connection_set_event_handler(this->service, ^(xpc_object_t conn)
	{
		xpc_connection_set_event_handler(conn, ^(xpc_object_t event)
		{
			if (xpc_get_type(event) == XPC_TYPE_DICTIONARY)
			{
				handle(this, event);
			}
		});
		ref_get(&this->refcount);
		xpc_connection_set_context(conn, this);
		xpc_connection_set_finalizer_f(conn, (void*)cleanup_connection);
		xpc_connection_resume(conn);
	});
	xpc_connection_resume(this->service);
}

METHOD(xpc_dispatch_t, destroy, void,
	private_xpc_dispatch_t *this)
{
	charon->bus->remove_listener(charon->bus, &this->channels->listener);
	this->channels->destroy(this->channels);
	if (this->service)
	{
		xpc_connection_suspend(this->service);
		xpc_release(this->service);
	}
	free(this);
}

/**
 * See header
 */
xpc_dispatch_t *xpc_dispatch_create()
{
	private_xpc_dispatch_t *this;

	INIT(this,
		.public = {
			.destroy = _destroy,
		},
		.channels = xpc_channels_create(),
		.queue = dispatch_queue_create("org.strongswan.charon-xpc.q",
									DISPATCH_QUEUE_CONCURRENT),
		.pid = getpid(),
	);
	charon->bus->add_listener(charon->bus, &this->channels->listener);

	this->service = xpc_connection_create_mach_service(
									"org.strongswan.charon-xpc", this->queue,
									XPC_CONNECTION_MACH_SERVICE_LISTENER);
	if (!this->service)
	{
		destroy(this);
		return NULL;
	}

	set_handler(this);

	return &this->public;
}
