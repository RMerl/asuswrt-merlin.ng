/*
 * Copyright (C) 2017 Lubomir Rintel
 *
 * Copyright (C) 2013 Tobias Brunner
 * Copyright (C) 2008-2009 Martin Willi
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

#include "nm_service.h"

#include <daemon.h>
#include <networking/host.h>
#include <utils/identification.h>
#include <config/peer_cfg.h>
#include <credentials/certificates/x509.h>

#include <stdio.h>

G_DEFINE_TYPE(NMStrongswanPlugin, nm_strongswan_plugin, NM_TYPE_VPN_SERVICE_PLUGIN)

/**
 * Private data of NMStrongswanPlugin
 */
typedef struct {
	/* implements bus listener interface */
	listener_t listener;
	/* IKE_SA we are listening on */
	ike_sa_t *ike_sa;
	/* backref to public plugin */
	NMVpnServicePlugin *plugin;
	/* credentials to use for authentication */
	nm_creds_t *creds;
	/* attribute handler for DNS/NBNS server information */
	nm_handler_t *handler;
	/* name of the connection */
	char *name;
} NMStrongswanPluginPrivate;

#define NM_STRONGSWAN_PLUGIN_GET_PRIVATE(o) \
			(G_TYPE_INSTANCE_GET_PRIVATE ((o), \
				NM_TYPE_STRONGSWAN_PLUGIN, NMStrongswanPluginPrivate))

/**
 * convert enumerated handler chunks to a UINT_ARRAY GValue
 */
static GVariant* handler_to_variant(nm_handler_t *handler,
							 configuration_attribute_type_t type)
{
	GVariantBuilder builder;
	enumerator_t *enumerator;
	chunk_t chunk;

	g_variant_builder_init (&builder, G_VARIANT_TYPE ("au"));

	enumerator = handler->create_enumerator(handler, type);
	while (enumerator->enumerate(enumerator, &chunk))
	{
		g_variant_builder_add (&builder, "u", *(uint32_t*)chunk.ptr);
	}
	enumerator->destroy(enumerator);

	return g_variant_builder_end (&builder);
}

/**
 * signal IPv4 config to NM, set connection as established
 */
static void signal_ipv4_config(NMVpnServicePlugin *plugin,
							   ike_sa_t *ike_sa, child_sa_t *child_sa)
{
	NMStrongswanPluginPrivate *priv = NM_STRONGSWAN_PLUGIN_GET_PRIVATE(plugin);
	GVariantBuilder builder;
	enumerator_t *enumerator;
	host_t *me, *other;
	nm_handler_t *handler;

	g_variant_builder_init (&builder, G_VARIANT_TYPE_VARDICT);

	handler = priv->handler;

	/* NM apparently requires to know the gateway */
	other = ike_sa->get_other_host(ike_sa);
	g_variant_builder_add (&builder, "{sv}", NM_VPN_PLUGIN_IP4_CONFIG_EXT_GATEWAY,
	                       g_variant_new_uint32 (*(uint32_t*)other->get_address(other).ptr));

	/* NM installs this IP address on the interface above, so we use the VIP if
	 * we got one.
	 */
	enumerator = ike_sa->create_virtual_ip_enumerator(ike_sa, TRUE);
	if (!enumerator->enumerate(enumerator, &me))
	{
		me = ike_sa->get_my_host(ike_sa);
	}
	enumerator->destroy(enumerator);
	g_variant_builder_add (&builder, "{sv}", NM_VPN_PLUGIN_IP4_CONFIG_ADDRESS,
	                       g_variant_new_uint32 (*(uint32_t*)other->get_address(me).ptr));

	g_variant_builder_add (&builder, "{sv}", NM_VPN_PLUGIN_IP4_CONFIG_PREFIX,
	                       g_variant_new_uint32 (me->get_address(me).len * 8));

	/* prevent NM from changing the default route. we set our own route in our
	 * own routing table
	 */
	g_variant_builder_add (&builder, "{sv}", NM_VPN_PLUGIN_IP4_CONFIG_NEVER_DEFAULT,
	                       g_variant_new_boolean (TRUE));


	g_variant_builder_add (&builder, "{sv}", NM_VPN_PLUGIN_IP4_CONFIG_DNS,
	                       handler_to_variant(handler, INTERNAL_IP4_DNS));

	g_variant_builder_add (&builder, "{sv}", NM_VPN_PLUGIN_IP4_CONFIG_NBNS,
	                       handler_to_variant(handler, INTERNAL_IP4_NBNS));

	handler->reset(handler);

	nm_vpn_service_plugin_set_ip4_config(plugin, g_variant_builder_end (&builder));
}

/**
 * signal failure to NM, connecting failed
 */
static void signal_failure(NMVpnServicePlugin *plugin, NMVpnPluginFailure failure)
{
	nm_handler_t *handler = NM_STRONGSWAN_PLUGIN_GET_PRIVATE(plugin)->handler;

	handler->reset(handler);

	nm_vpn_service_plugin_failure(plugin, failure);
}

/**
 * Implementation of listener_t.ike_state_change
 */
static bool ike_state_change(listener_t *listener, ike_sa_t *ike_sa,
							 ike_sa_state_t state)
{
	NMStrongswanPluginPrivate *private = (NMStrongswanPluginPrivate*)listener;

	if (private->ike_sa == ike_sa && state == IKE_DESTROYING)
	{
		signal_failure(private->plugin, NM_VPN_PLUGIN_FAILURE_LOGIN_FAILED);
		return FALSE;
	}
	return TRUE;
}

/**
 * Implementation of listener_t.child_state_change
 */
static bool child_state_change(listener_t *listener, ike_sa_t *ike_sa,
							   child_sa_t *child_sa, child_sa_state_t state)
{
	NMStrongswanPluginPrivate *private = (NMStrongswanPluginPrivate*)listener;

	if (private->ike_sa == ike_sa && state == CHILD_DESTROYING)
	{
		signal_failure(private->plugin, NM_VPN_PLUGIN_FAILURE_CONNECT_FAILED);
		return FALSE;
	}
	return TRUE;
}

/**
 * Implementation of listener_t.child_updown
 */
static bool child_updown(listener_t *listener, ike_sa_t *ike_sa,
						 child_sa_t *child_sa, bool up)
{
	NMStrongswanPluginPrivate *private = (NMStrongswanPluginPrivate*)listener;

	if (private->ike_sa == ike_sa)
	{
		if (up)
		{	/* disable initiate-failure-detection hooks */
			private->listener.ike_state_change = NULL;
			private->listener.child_state_change = NULL;
			signal_ipv4_config(private->plugin, ike_sa, child_sa);
		}
		else
		{
			signal_failure(private->plugin, NM_VPN_PLUGIN_FAILURE_CONNECT_FAILED);
			return FALSE;
		}
	}
	return TRUE;
}

/**
 * Implementation of listener_t.ike_rekey
 */
static bool ike_rekey(listener_t *listener, ike_sa_t *old, ike_sa_t *new)
{
	NMStrongswanPluginPrivate *private = (NMStrongswanPluginPrivate*)listener;

	if (private->ike_sa == old)
	{	/* follow a rekeyed IKE_SA */
		private->ike_sa = new;
	}
	return TRUE;
}

/**
 * Find a certificate for which we have a private key on a smartcard
 */
static identification_t *find_smartcard_key(NMStrongswanPluginPrivate *priv,
											char *pin)
{
	enumerator_t *enumerator, *sans;
	identification_t *id = NULL;
	certificate_t *cert;
	x509_t *x509;
	private_key_t *key;
	chunk_t keyid;

	enumerator = lib->credmgr->create_cert_enumerator(lib->credmgr,
											CERT_X509, KEY_ANY, NULL, FALSE);
	while (enumerator->enumerate(enumerator, &cert))
	{
		x509 = (x509_t*)cert;

		/* there might be a lot of certificates, filter them by usage */
		if ((x509->get_flags(x509) & X509_CLIENT_AUTH) &&
			!(x509->get_flags(x509) & X509_CA))
		{
			keyid = x509->get_subjectKeyIdentifier(x509);
			if (keyid.ptr)
			{
				/* try to find a private key by the certificate keyid */
				priv->creds->set_pin(priv->creds, keyid, pin);
				key = lib->creds->create(lib->creds, CRED_PRIVATE_KEY,
								KEY_ANY, BUILD_PKCS11_KEYID, keyid, BUILD_END);
				if (key)
				{
					/* prefer a more convenient subjectAltName */
					sans = x509->create_subjectAltName_enumerator(x509);
					if (!sans->enumerate(sans, &id))
					{
						id = cert->get_subject(cert);
					}
					id = id->clone(id);
					sans->destroy(sans);

					DBG1(DBG_CFG, "using smartcard certificate '%Y'", id);
					priv->creds->set_cert_and_key(priv->creds,
												  cert->get_ref(cert), key);
					break;
				}
			}
		}
	}
	enumerator->destroy(enumerator);
	return id;
}

/**
 * Connect function called from NM via DBUS
 */
static gboolean connect_(NMVpnServicePlugin *plugin, NMConnection *connection,
						 GError **err)
{
	NMStrongswanPluginPrivate *priv;
	NMSettingConnection *conn;
	NMSettingVpn *vpn;
	enumerator_t *enumerator;
	identification_t *user = NULL, *gateway = NULL;
	const char *address, *str;
	bool virtual, encap, proposal;
	proposal_t *prop;
	ike_cfg_t *ike_cfg;
	peer_cfg_t *peer_cfg;
	child_cfg_t *child_cfg;
	traffic_selector_t *ts;
	ike_sa_t *ike_sa;
	auth_cfg_t *auth;
	auth_class_t auth_class = AUTH_CLASS_EAP;
	certificate_t *cert = NULL;
	x509_t *x509;
	bool agent = FALSE, smartcard = FALSE, loose_gateway_id = FALSE;
	peer_cfg_create_t peer = {
		.cert_policy = CERT_SEND_IF_ASKED,
		.unique = UNIQUE_REPLACE,
		.keyingtries = 1,
		.rekey_time = 36000, /* 10h */
		.jitter_time = 600, /* 10min */
		.over_time = 600, /* 10min */
	};
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

	/**
	 * Read parameters
	 */
	priv = NM_STRONGSWAN_PLUGIN_GET_PRIVATE(plugin);
	conn = NM_SETTING_CONNECTION(nm_connection_get_setting(connection,
												NM_TYPE_SETTING_CONNECTION));
	vpn = NM_SETTING_VPN(nm_connection_get_setting(connection,
												NM_TYPE_SETTING_VPN));
	if (priv->name)
	{
		free(priv->name);
	}
	priv->name = strdup(nm_setting_connection_get_id(conn));
	DBG1(DBG_CFG, "received initiate for NetworkManager connection %s",
		 priv->name);
	DBG4(DBG_CFG, "%s",
		 nm_setting_to_string(NM_SETTING(vpn)));
	address = nm_setting_vpn_get_data_item(vpn, "address");
	if (!address || !*address)
	{
		g_set_error(err, NM_VPN_PLUGIN_ERROR, NM_VPN_PLUGIN_ERROR_BAD_ARGUMENTS,
					"Gateway address missing.");
		return FALSE;
	}
	str = nm_setting_vpn_get_data_item(vpn, "virtual");
	virtual = streq(str, "yes");
	str = nm_setting_vpn_get_data_item(vpn, "encap");
	encap = streq(str, "yes");
	str = nm_setting_vpn_get_data_item(vpn, "ipcomp");
	child.options |= streq(str, "yes") ? OPT_IPCOMP : 0;
	str = nm_setting_vpn_get_data_item(vpn, "method");
	if (streq(str, "psk"))
	{
		auth_class = AUTH_CLASS_PSK;
	}
	else if (streq(str, "agent"))
	{
		auth_class = AUTH_CLASS_PUBKEY;
		agent = TRUE;
	}
	else if (streq(str, "key"))
	{
		auth_class = AUTH_CLASS_PUBKEY;
	}
	else if (streq(str, "smartcard"))
	{
		auth_class = AUTH_CLASS_PUBKEY;
		smartcard = TRUE;
	}

	/**
	 * Register credentials
	 */
	priv->creds->clear(priv->creds);

	/* gateway/CA cert */
	str = nm_setting_vpn_get_data_item(vpn, "certificate");
	if (str)
	{
		cert = lib->creds->create(lib->creds, CRED_CERTIFICATE, CERT_X509,
								  BUILD_FROM_FILE, str, BUILD_END);
		if (!cert)
		{
			g_set_error(err, NM_VPN_PLUGIN_ERROR,
						NM_VPN_PLUGIN_ERROR_BAD_ARGUMENTS,
						"Loading gateway certificate failed.");
			return FALSE;
		}
		priv->creds->add_certificate(priv->creds, cert);

		x509 = (x509_t*)cert;
		if (!(x509->get_flags(x509) & X509_CA))
		{	/* For a gateway certificate, we use the cert subject as identity. */
			gateway = cert->get_subject(cert);
			gateway = gateway->clone(gateway);
			DBG1(DBG_CFG, "using gateway certificate, identity '%Y'", gateway);
		}
	}
	else
	{
		/* no certificate defined, fall back to system-wide CA certificates */
		priv->creds->load_ca_dir(priv->creds, lib->settings->get_str(
								 lib->settings, "charon-nm.ca_dir", NM_CA_DIR));
	}
	if (!gateway)
	{
		/* If the user configured a CA certificate, we use the IP/DNS
		 * of the gateway as its identity. This identity will be used for
		 * certificate lookup and requires the configured IP/DNS to be
		 * included in the gateway certificate. */
		gateway = identification_create_from_string((char*)address);
		DBG1(DBG_CFG, "using CA certificate, gateway identity '%Y'", gateway);
		loose_gateway_id = TRUE;
	}

	if (auth_class == AUTH_CLASS_EAP ||
		auth_class == AUTH_CLASS_PSK)
	{
		/* username/password or PSK authentication ... */
		str = nm_setting_vpn_get_data_item(vpn, "user");
		if (str)
		{
			user = identification_create_from_string((char*)str);
			str = nm_setting_vpn_get_secret(vpn, "password");
			if (auth_class == AUTH_CLASS_PSK &&
				strlen(str) < 20)
			{
				g_set_error(err, NM_VPN_PLUGIN_ERROR,
							NM_VPN_PLUGIN_ERROR_BAD_ARGUMENTS,
							"pre-shared key is too short.");
				gateway->destroy(gateway);
				user->destroy(user);
				return FALSE;
			}
			priv->creds->set_username_password(priv->creds, user, (char*)str);
		}
	}

	if (auth_class == AUTH_CLASS_PUBKEY)
	{
		if (smartcard)
		{
			char *pin;

			pin = (char*)nm_setting_vpn_get_secret(vpn, "password");
			if (pin)
			{
				user = find_smartcard_key(priv, pin);
			}
			if (!user)
			{
				g_set_error(err, NM_VPN_PLUGIN_ERROR,
							NM_VPN_PLUGIN_ERROR_BAD_ARGUMENTS,
							"no usable smartcard certificate found.");
				gateway->destroy(gateway);
				return FALSE;
			}
		}
		/* ... or certificate/private key authenitcation */
		else if ((str = nm_setting_vpn_get_data_item(vpn, "usercert")))
		{
			public_key_t *public;
			private_key_t *private = NULL;

			cert = lib->creds->create(lib->creds, CRED_CERTIFICATE, CERT_X509,
									  BUILD_FROM_FILE, str, BUILD_END);
			if (!cert)
			{
				g_set_error(err, NM_VPN_PLUGIN_ERROR,
							NM_VPN_PLUGIN_ERROR_BAD_ARGUMENTS,
							"Loading peer certificate failed.");
				gateway->destroy(gateway);
				return FALSE;
			}
			/* try agent */
			str = nm_setting_vpn_get_secret(vpn, "agent");
			if (agent && str)
			{
				public = cert->get_public_key(cert);
				if (public)
				{
					private = lib->creds->create(lib->creds, CRED_PRIVATE_KEY,
												 public->get_type(public),
												 BUILD_AGENT_SOCKET, str,
												 BUILD_PUBLIC_KEY, public,
												 BUILD_END);
					public->destroy(public);
				}
				if (!private)
				{
					g_set_error(err, NM_VPN_PLUGIN_ERROR,
								NM_VPN_PLUGIN_ERROR_BAD_ARGUMENTS,
								"Connecting to SSH agent failed.");
				}
			}
			/* ... or key file */
			str = nm_setting_vpn_get_data_item(vpn, "userkey");
			if (!agent && str)
			{
				char *secret;

				secret = (char*)nm_setting_vpn_get_secret(vpn, "password");
				if (secret)
				{
					priv->creds->set_key_password(priv->creds, secret);
				}
				private = lib->creds->create(lib->creds, CRED_PRIVATE_KEY,
								KEY_ANY, BUILD_FROM_FILE, str, BUILD_END);
				if (!private)
				{
					g_set_error(err, NM_VPN_PLUGIN_ERROR,
								NM_VPN_PLUGIN_ERROR_BAD_ARGUMENTS,
								"Loading private key failed.");
				}
			}
			if (private)
			{
				user = cert->get_subject(cert);
				user = user->clone(user);
				priv->creds->set_cert_and_key(priv->creds, cert, private);
			}
			else
			{
				DESTROY_IF(cert);
				gateway->destroy(gateway);
				return FALSE;
			}
		}
	}

	if (!user)
	{
		g_set_error(err, NM_VPN_PLUGIN_ERROR, NM_VPN_PLUGIN_ERROR_BAD_ARGUMENTS,
					"Configuration parameters missing.");
		gateway->destroy(gateway);
		return FALSE;
	}

	/**
	 * Set up configurations
	 */
	ike_cfg = ike_cfg_create(IKEV2, TRUE, encap, "0.0.0.0",
							 charon->socket->get_port(charon->socket, FALSE),
							(char*)address, IKEV2_UDP_PORT,
							 FRAGMENTATION_YES, 0);

	str = nm_setting_vpn_get_data_item(vpn, "proposal");
	proposal = streq(str, "yes");
	str = nm_setting_vpn_get_data_item(vpn, "ike");
	if (proposal && str && strlen(str))
	{
		enumerator = enumerator_create_token(str, ";", "");
		while (enumerator->enumerate(enumerator, &str))
		{
			prop = proposal_create_from_string(PROTO_IKE, str);
			if (!prop)
			{
				g_set_error(err, NM_VPN_PLUGIN_ERROR,
							NM_VPN_PLUGIN_ERROR_LAUNCH_FAILED,
							"Invalid IKE proposal.");
				enumerator->destroy(enumerator);
				ike_cfg->destroy(ike_cfg);
				gateway->destroy(gateway);
				user->destroy(user);
				return FALSE;
			}
			ike_cfg->add_proposal(ike_cfg, prop);
		}
		enumerator->destroy(enumerator);
	}
	else
	{
		ike_cfg->add_proposal(ike_cfg, proposal_create_default(PROTO_IKE));
		ike_cfg->add_proposal(ike_cfg, proposal_create_default_aead(PROTO_IKE));
	}

	peer_cfg = peer_cfg_create(priv->name, ike_cfg, &peer);
	if (virtual)
	{
		peer_cfg->add_virtual_ip(peer_cfg, host_create_from_string("0.0.0.0", 0));
	}
	auth = auth_cfg_create();
	auth->add(auth, AUTH_RULE_AUTH_CLASS, auth_class);
	auth->add(auth, AUTH_RULE_IDENTITY, user);
	peer_cfg->add_auth_cfg(peer_cfg, auth, TRUE);
	auth = auth_cfg_create();
	if (auth_class == AUTH_CLASS_PSK)
	{
		auth->add(auth, AUTH_RULE_AUTH_CLASS, AUTH_CLASS_PSK);
	}
	else
	{
		auth->add(auth, AUTH_RULE_AUTH_CLASS, AUTH_CLASS_PUBKEY);
	}
	auth->add(auth, AUTH_RULE_IDENTITY, gateway);
	auth->add(auth, AUTH_RULE_IDENTITY_LOOSE, loose_gateway_id);
	peer_cfg->add_auth_cfg(peer_cfg, auth, FALSE);

	child_cfg = child_cfg_create(priv->name, &child);
	str = nm_setting_vpn_get_data_item(vpn, "esp");
	if (proposal && str && strlen(str))
	{
		enumerator = enumerator_create_token(str, ";", "");
		while (enumerator->enumerate(enumerator, &str))
		{
			prop = proposal_create_from_string(PROTO_ESP, str);
			if (!prop)
			{
				g_set_error(err, NM_VPN_PLUGIN_ERROR,
							NM_VPN_PLUGIN_ERROR_LAUNCH_FAILED,
							"Invalid ESP proposal.");
				enumerator->destroy(enumerator);
				child_cfg->destroy(child_cfg);
				peer_cfg->destroy(peer_cfg);
				return FALSE;
			}
			child_cfg->add_proposal(child_cfg, prop);
		}
		enumerator->destroy(enumerator);
	}
	else
	{
		child_cfg->add_proposal(child_cfg, proposal_create_default(PROTO_ESP));
		child_cfg->add_proposal(child_cfg, proposal_create_default_aead(PROTO_ESP));
	}
	ts = traffic_selector_create_dynamic(0, 0, 65535);
	child_cfg->add_traffic_selector(child_cfg, TRUE, ts);
	ts = traffic_selector_create_from_string(0, TS_IPV4_ADDR_RANGE,
											 "0.0.0.0", 0,
											 "255.255.255.255", 65535);
	child_cfg->add_traffic_selector(child_cfg, FALSE, ts);
	peer_cfg->add_child_cfg(peer_cfg, child_cfg);

	/**
	 * Prepare IKE_SA
	 */
	ike_sa = charon->ike_sa_manager->checkout_by_config(charon->ike_sa_manager,
														peer_cfg);
	if (!ike_sa)
	{
		peer_cfg->destroy(peer_cfg);
		g_set_error(err, NM_VPN_PLUGIN_ERROR, NM_VPN_PLUGIN_ERROR_LAUNCH_FAILED,
					"IKE version not supported.");
		return FALSE;
	}
	if (!ike_sa->get_peer_cfg(ike_sa))
	{
		ike_sa->set_peer_cfg(ike_sa, peer_cfg);
	}
	peer_cfg->destroy(peer_cfg);

	/**
	 * Register listener, enable  initiate-failure-detection hooks
	 */
	priv->ike_sa = ike_sa;
	priv->listener.ike_state_change = ike_state_change;
	priv->listener.child_state_change = child_state_change;
	charon->bus->add_listener(charon->bus, &priv->listener);

	/**
	 * Initiate
	 */
	child_cfg->get_ref(child_cfg);
	if (ike_sa->initiate(ike_sa, child_cfg, 0, NULL, NULL) != SUCCESS)
	{
		charon->bus->remove_listener(charon->bus, &priv->listener);
		charon->ike_sa_manager->checkin_and_destroy(charon->ike_sa_manager, ike_sa);

		g_set_error(err, NM_VPN_PLUGIN_ERROR, NM_VPN_PLUGIN_ERROR_LAUNCH_FAILED,
					"Initiating failed.");
		return FALSE;
	}
	charon->ike_sa_manager->checkin(charon->ike_sa_manager, ike_sa);
	return TRUE;
}

/**
 * NeedSecrets called from NM via DBUS
 */
static gboolean need_secrets(NMVpnServicePlugin *plugin, NMConnection *connection,
							 const char **setting_name, GError **error)
{
	NMSettingVpn *settings;
	const char *method, *path;

	settings = NM_SETTING_VPN(nm_connection_get_setting(connection,
														NM_TYPE_SETTING_VPN));
	method = nm_setting_vpn_get_data_item(settings, "method");
	if (method)
	{
		if (streq(method, "eap") || streq(method, "psk"))
		{
			if (nm_setting_vpn_get_secret(settings, "password"))
			{
				return FALSE;
			}
		}
		else if (streq(method, "agent"))
		{
			if (nm_setting_vpn_get_secret(settings, "agent"))
			{
				return FALSE;
			}
		}
		else if (streq(method, "key"))
		{
			path = nm_setting_vpn_get_data_item(settings, "userkey");
			if (path)
			{
				private_key_t *key;

				/* try to load/decrypt the private key */
				key = lib->creds->create(lib->creds, CRED_PRIVATE_KEY,
								KEY_ANY, BUILD_FROM_FILE, path, BUILD_END);
				if (key)
				{
					key->destroy(key);
					return FALSE;
				}
				else if (nm_setting_vpn_get_secret(settings, "password"))
				{
					return FALSE;
				}
			}
		}
		else if (streq(method, "smartcard"))
		{
			if (nm_setting_vpn_get_secret(settings, "password"))
			{
				return FALSE;
			}
		}
	}
	*setting_name = NM_SETTING_VPN_SETTING_NAME;
	return TRUE;
}

/**
 * The actual disconnection
 */
static gboolean do_disconnect(gpointer plugin)
{
	NMStrongswanPluginPrivate *priv = NM_STRONGSWAN_PLUGIN_GET_PRIVATE(plugin);
	enumerator_t *enumerator;
	ike_sa_t *ike_sa;
	u_int id;

	/* our ike_sa pointer might be invalid, lookup sa */
	enumerator = charon->controller->create_ike_sa_enumerator(
													charon->controller, TRUE);
	while (enumerator->enumerate(enumerator, &ike_sa))
	{
		if (priv->ike_sa == ike_sa)
		{
			id = ike_sa->get_unique_id(ike_sa);
			enumerator->destroy(enumerator);
			charon->controller->terminate_ike(charon->controller, id, FALSE,
											  controller_cb_empty, NULL, 0);
			return FALSE;
		}
	}
	enumerator->destroy(enumerator);

	g_debug("Connection not found.");
	return FALSE;
}

/**
 * Disconnect called from NM via DBUS
 */
static gboolean disconnect(NMVpnServicePlugin *plugin, GError **err)
{
	/* enqueue the actual disconnection, because we may be called in
	 * response to a listener_t callback and the SA enumeration would
	 * possibly deadlock. */
	g_idle_add(do_disconnect, plugin);

	return TRUE;
}

/**
 * Initializer
 */
static void nm_strongswan_plugin_init(NMStrongswanPlugin *plugin)
{
	NMStrongswanPluginPrivate *priv;

	priv = NM_STRONGSWAN_PLUGIN_GET_PRIVATE(plugin);
	priv->plugin = NM_VPN_SERVICE_PLUGIN(plugin);
	memset(&priv->listener, 0, sizeof(listener_t));
	priv->listener.child_updown = child_updown;
	priv->listener.ike_rekey = ike_rekey;
	priv->name = NULL;
}

/**
 * Class constructor
 */
static void nm_strongswan_plugin_class_init(
									NMStrongswanPluginClass *strongswan_class)
{
	NMVpnServicePluginClass *parent_class = NM_VPN_SERVICE_PLUGIN_CLASS(strongswan_class);

	g_type_class_add_private(G_OBJECT_CLASS(strongswan_class),
							 sizeof(NMStrongswanPluginPrivate));
	parent_class->connect = connect_;
	parent_class->need_secrets = need_secrets;
	parent_class->disconnect = disconnect;
}

/**
 * Object constructor
 */
NMStrongswanPlugin *nm_strongswan_plugin_new(nm_creds_t *creds,
											 nm_handler_t *handler)
{
	GError *error = NULL;

	NMStrongswanPlugin *plugin = (NMStrongswanPlugin *)g_initable_new (
					NM_TYPE_STRONGSWAN_PLUGIN,
					NULL,
					&error,
					NM_VPN_SERVICE_PLUGIN_DBUS_SERVICE_NAME, NM_DBUS_SERVICE_STRONGSWAN,
					NULL);

	if (plugin)
	{
		NMStrongswanPluginPrivate *priv;

		/* the rest of the initialization happened in _init above */
		priv = NM_STRONGSWAN_PLUGIN_GET_PRIVATE(plugin);
		priv->creds = creds;
		priv->handler = handler;
	}
	else
	{
		g_warning ("Failed to initialize a plugin instance: %s", error->message);
		g_error_free (error);
	}

	return plugin;
}
