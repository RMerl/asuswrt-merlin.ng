/*
 * Copyright (C) 2016-2018 Tobias Brunner
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

#include "exchange_test_helper.h"
#include "mock_dh.h"
#include "mock_ipsec.h"
#include "mock_net.h"
#include "mock_nonce_gen.h"

#include <collections/array.h>
#include <credentials/sets/mem_cred.h>

typedef struct private_exchange_test_helper_t private_exchange_test_helper_t;
typedef struct private_backend_t private_backend_t;

/**
 * Private data
 */
struct private_exchange_test_helper_t {

	/**
	 * Public interface
	 */
	exchange_test_helper_t public;

	/**
	 * Credentials
	 */
	mem_cred_t *creds;

	/**
	 * IKE_SA SPI counter
	 */
	refcount_t ike_spi;

	/**
	 * List of registered listeners
	 */
	array_t *listeners;

	/**
	 * Config backend
	 */
	private_backend_t *backend;
};

/**
 * Custom backend_t implementation
 */
struct private_backend_t {

	/**
	 * Public interface
	 */
	backend_t public;

	/**
	 * Responder ike_cfg
	 */
	ike_cfg_t *ike_cfg;

	/**
	 * Responder peer_cfg/child_cfg
	 */
	peer_cfg_t *peer_cfg;
};

CALLBACK(get_ike_spi, uint64_t,
	private_exchange_test_helper_t *this)
{
	return (uint64_t)ref_get(&this->ike_spi);
}

/*
 * Described in header
 */
exchange_test_helper_t *exchange_test_helper;

static ike_cfg_t *create_ike_cfg(bool initiator, exchange_test_sa_conf_t *conf)
{
	ike_cfg_create_t ike = {
		.version = IKEV2,
		.local = "127.0.0.1",
		.local_port = IKEV2_UDP_PORT,
		.remote = "127.0.0.1",
		.remote_port = IKEV2_UDP_PORT,
	};
	enumerator_t *enumerator;
	ike_cfg_t *ike_cfg;
	char *proposals = NULL, *proposal;

	if (conf)
	{
		ike.childless = initiator ? conf->initiator.childless
								  : conf->responder.childless;
		proposals = initiator ? conf->initiator.ike : conf->responder.ike;
	}

	ike_cfg = ike_cfg_create(&ike);
	if (proposals)
	{
		enumerator = enumerator_create_token(proposals, ",", "");
		while (enumerator->enumerate(enumerator, &proposal))
		{
			ike_cfg->add_proposal(ike_cfg,
							proposal_create_from_string(PROTO_IKE, proposal));
		}
		enumerator->destroy(enumerator);
	}
	else
	{
		ike_cfg->add_proposal(ike_cfg, proposal_create_default(PROTO_IKE));
	}
	return ike_cfg;
}

static child_cfg_t *create_child_cfg(bool initiator,
									 exchange_test_sa_conf_t *conf)
{
	enumerator_t *enumerator;
	child_cfg_t *child_cfg;
	child_cfg_create_t child = {
		.mode = MODE_TUNNEL,
	};
	char *proposals = NULL, *proposal;

	child_cfg = child_cfg_create(initiator ? "init" : "resp", &child);
	if (conf)
	{
		proposals = initiator ? conf->initiator.esp : conf->responder.esp;
	}
	if (proposals)
	{
		enumerator = enumerator_create_token(proposals, ",", "");
		while (enumerator->enumerate(enumerator, &proposal))
		{
			child_cfg->add_proposal(child_cfg,
							proposal_create_from_string(PROTO_ESP, proposal));
		}
		enumerator->destroy(enumerator);
	}
	else
	{
		child_cfg->add_proposal(child_cfg, proposal_create_default(PROTO_ESP));
	}
	child_cfg->add_traffic_selector(child_cfg, TRUE,
								traffic_selector_create_dynamic(0, 0, 65535));
	child_cfg->add_traffic_selector(child_cfg, FALSE,
								traffic_selector_create_dynamic(0, 0, 65535));
	return child_cfg;
}

static void add_auth_cfg(peer_cfg_t *peer_cfg, bool initiator, bool local)
{
	auth_cfg_t *auth;
	char *id = "init";

	auth = auth_cfg_create();
	auth->add(auth, AUTH_RULE_AUTH_CLASS, AUTH_CLASS_PSK);
	if (initiator ^ local)
	{
		id = "resp";
	}
	auth->add(auth, AUTH_RULE_IDENTITY, identification_create_from_string(id));
	peer_cfg->add_auth_cfg(peer_cfg, auth, local);
}

static peer_cfg_t *create_peer_cfg(bool initiator,
								   exchange_test_sa_conf_t *conf)
{
	peer_cfg_t *peer_cfg;
	peer_cfg_create_t peer = {
		.cert_policy = CERT_SEND_IF_ASKED,
		.unique = UNIQUE_REPLACE,
		.keyingtries = 1,
	};

	peer_cfg = peer_cfg_create(initiator ? "init" : "resp",
							   create_ike_cfg(initiator, conf), &peer);
	add_auth_cfg(peer_cfg, initiator, TRUE);
	add_auth_cfg(peer_cfg, initiator, FALSE);
	return peer_cfg;
}

METHOD(backend_t, create_ike_cfg_enumerator, enumerator_t*,
	private_backend_t *this, host_t *me, host_t *other)
{
	return enumerator_create_single(this->ike_cfg, NULL);
}

METHOD(backend_t, create_peer_cfg_enumerator, enumerator_t*,
	private_backend_t *this, identification_t *me, identification_t *other)
{
	return enumerator_create_single(this->peer_cfg, NULL);
}

/**
 * Sets the config objects provided by the backend
 */
static void set_config(private_backend_t *this, ike_cfg_t *ike,
					   peer_cfg_t *peer)
{
	DESTROY_IF(this->ike_cfg);
	this->ike_cfg = ike;
	DESTROY_IF(this->peer_cfg);
	this->peer_cfg = peer;
}

METHOD(exchange_test_helper_t, process_message, status_t,
	private_exchange_test_helper_t *this, ike_sa_t *ike_sa, message_t *message)
{
	status_t status = FAILED;
	ike_sa_id_t *id;

	if (!message)
	{
		message = this->public.sender->dequeue(this->public.sender);
	}
	id = message->get_ike_sa_id(message);
	id = id->clone(id);
	id->switch_initiator(id);
	if (!id->get_responder_spi(id) || id->equals(id, ike_sa->get_id(ike_sa)))
	{
		charon->bus->set_sa(charon->bus, ike_sa);
		status = ike_sa->process_message(ike_sa, message);
		charon->bus->set_sa(charon->bus, NULL);
	}
	message->destroy(message);
	id->destroy(id);
	return status;
}

METHOD(exchange_test_helper_t, create_sa, child_cfg_t*,
	private_exchange_test_helper_t *this, ike_sa_t **init, ike_sa_t **resp,
	exchange_test_sa_conf_t *conf)
{
	peer_cfg_t *peer_cfg;
	child_cfg_t *child_cfg;

	*init = charon->ike_sa_manager->create_new(charon->ike_sa_manager,
											   IKEV2, TRUE);

	*resp = charon->ike_sa_manager->create_new(charon->ike_sa_manager,
											   IKEV2, FALSE);

	peer_cfg = create_peer_cfg(FALSE, conf);
	child_cfg = create_child_cfg(FALSE, conf);
	peer_cfg->add_child_cfg(peer_cfg, child_cfg->get_ref(child_cfg));
	child_cfg->destroy(child_cfg);
	set_config(this->backend,  create_ike_cfg(FALSE, conf), peer_cfg);

	peer_cfg = create_peer_cfg(TRUE, conf);
	child_cfg = create_child_cfg(TRUE, conf);
	peer_cfg->add_child_cfg(peer_cfg, child_cfg->get_ref(child_cfg));
	(*init)->set_peer_cfg(*init, peer_cfg);
	peer_cfg->destroy(peer_cfg);
	return child_cfg;
}

METHOD(exchange_test_helper_t, establish_sa, void,
	private_exchange_test_helper_t *this, ike_sa_t **init, ike_sa_t **resp,
	exchange_test_sa_conf_t *conf)
{
	ike_sa_id_t *id_i, *id_r;
	ike_sa_t *sa_i, *sa_r;
	child_cfg_t *child_i;
	proposal_t *proposal;

	child_i = create_sa(this, init, resp, conf);

	sa_i = *init;
	sa_r = *resp;

	id_i = sa_i->get_id(sa_i);
	id_r = sa_r->get_id(sa_r);

	call_ikesa(sa_i, initiate, child_i, NULL);

	/* IKE_SA_INIT --> */
	id_r->set_initiator_spi(id_r, id_i->get_initiator_spi(id_i));
	process_message(this, sa_r, NULL);
	/* <-- IKE_SA_INIT */
	id_i->set_responder_spi(id_i, id_r->get_responder_spi(id_r));
	process_message(this, sa_i, NULL);

	proposal = sa_i->get_proposal(sa_i);
	if (proposal->get_algorithm(proposal, ADDITIONAL_KEY_EXCHANGE_1, NULL,
								NULL))
	{
		/* IKE_INTERMEDIATE --> */
		process_message(this, sa_r, NULL);
		/* <-- IKE_INTERMEDIATE */
		process_message(this, sa_i, NULL);
	}

	/* IKE_AUTH --> */
	process_message(this, sa_r, NULL);
	/* <-- IKE_AUTH */
	process_message(this, sa_i, NULL);
}

METHOD(exchange_test_helper_t, add_listener, void,
	private_exchange_test_helper_t *this, listener_t *listener)
{
	array_insert_create(&this->listeners, ARRAY_TAIL, listener);
	charon->bus->add_listener(charon->bus, listener);
}

/**
 * Enable logging in charon as requested
 */
static void initialize_logging()
{
	char buf[BUF_LEN], *verbosity;
	level_t level;
	debug_t group;

	lib->settings->set_default_str(lib->settings, "%s.filelog.stderr.default",
								   "-1", lib->ns);

	verbosity = getenv("TESTS_VERBOSITY");
	if (verbosity)
	{
		level = atoi(verbosity);
		lib->settings->set_int(lib->settings, "%s.filelog.stderr.default",
							   level, lib->ns);
	}

	for (group = 0; group < DBG_MAX; group++)
	{
		snprintf(buf, sizeof(buf), "TESTS_VERBOSITY_%s",
				 enum_to_name(debug_names, group));
		verbosity = getenv(buf);
		if (verbosity)
		{
			level = atoi(verbosity);
			lib->settings->set_int(lib->settings, "%s.filelog.stderr.%N",
								   level, lib->ns, debug_lower_names, group);
		}
	}
	lib->settings->set_bool(lib->settings, "%s.filelog.stderr.ike_name", TRUE,
							lib->ns);
	charon->load_loggers(charon);
}

/**
 * Create a nonce generator with the first byte
 */
static nonce_gen_t *create_nonce_gen()
{
	return mock_nonce_gen_create(exchange_test_helper->nonce_first_byte);
}

/*
 * Described in header
 */
void exchange_test_helper_init(char *plugins)
{
	private_exchange_test_helper_t *this;
	private_backend_t *backend;
	plugin_feature_t features[] = {
		PLUGIN_REGISTER(KE, mock_dh_create),
			/* we only need to support a limited number of DH groups */
			PLUGIN_PROVIDE(KE, MODP_2048_BIT),
			PLUGIN_PROVIDE(KE, MODP_3072_BIT),
			PLUGIN_PROVIDE(KE, ECP_256_BIT),
		PLUGIN_REGISTER(NONCE_GEN, create_nonce_gen),
			PLUGIN_PROVIDE(NONCE_GEN),
				PLUGIN_DEPENDS(RNG, RNG_WEAK),
	};

	INIT(backend,
		.public = {
			.create_ike_cfg_enumerator = _create_ike_cfg_enumerator,
			.create_peer_cfg_enumerator = _create_peer_cfg_enumerator,
			.get_peer_cfg_by_name = (void*)return_null,
		},
	);

	INIT(this,
		.public = {
			.sender = mock_sender_create(),
			.establish_sa = _establish_sa,
			.create_sa = _create_sa,
			.process_message = _process_message,
			.add_listener = _add_listener,
		},
		.creds = mem_cred_create(),
		.backend = backend,
	);

	initialize_logging();
	lib->plugins->add_static_features(lib->plugins, "exchange-test-helper",
								features, countof(features), TRUE, NULL, NULL);
	/* the libcharon unit tests only load the libstrongswan plugins, unless
	 * TESTS_PLUGINS is defined */
	charon->initialize(charon, plugins);
	lib->plugins->status(lib->plugins, LEVEL_CTRL);

	/* the original sender is not initialized because there is no socket */
	charon->sender = (sender_t*)this->public.sender;
	/* and there is no kernel plugin loaded
	 * TODO: we'd have more control if we'd implement kernel_interface_t */
	charon->kernel->add_ipsec_interface(charon->kernel, mock_ipsec_create);
	charon->kernel->add_net_interface(charon->kernel, mock_net_create);
	/* like SPIs for IPsec SAs, make IKE SPIs predictable */
	charon->ike_sa_manager->set_spi_cb(charon->ike_sa_manager, get_ike_spi,
									   this);

	charon->backends->add_backend(charon->backends, &backend->public);

	lib->credmgr->add_set(lib->credmgr, &this->creds->set);

	this->creds->add_shared(this->creds,
			shared_key_create(SHARED_IKE, chunk_clone(chunk_from_str("test"))),
			identification_create_from_string("%any"), NULL);

	exchange_test_helper = &this->public;
}

/*
 * Described in header
 */
void exchange_test_helper_deinit()
{
	private_exchange_test_helper_t *this;
	listener_t *listener;

	this = (private_exchange_test_helper_t*)exchange_test_helper;

	while (array_remove(this->listeners, ARRAY_HEAD, &listener))
	{
		charon->bus->remove_listener(charon->bus, listener);
	}
	charon->backends->remove_backend(charon->backends, &this->backend->public);
	set_config(this->backend, NULL, NULL);
	free(this->backend);
	lib->credmgr->remove_set(lib->credmgr, &this->creds->set);
	this->creds->destroy(this->creds);
	/* flush SAs before destroying the sender (in case of test failures) */
	charon->ike_sa_manager->flush(charon->ike_sa_manager);
	/* charon won't destroy this as it didn't initialize the original sender */
	charon->sender->destroy(charon->sender);
	charon->sender = NULL;
	array_destroy(this->listeners);
	free(this);
}
