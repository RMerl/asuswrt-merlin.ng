/*
 * Copyright (C) 2013 Tobias Brunner
 * Copyright (C) 2008-2009 Martin Willi
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

#include <nm-setting-vpn.h>
#include <nm-setting-connection.h>
#include "nm_service.h"

#include <daemon.h>
#include <networking/host.h>
#include <utils/identification.h>
#include <config/peer_cfg.h>
#include <credentials/certificates/x509.h>
#include <networking/tun_device.h>

#include <stdio.h>

G_DEFINE_TYPE(NMStrongswanPlugin, nm_strongswan_plugin, NM_TYPE_VPN_PLUGIN)

/**
 * Private data of NMStrongswanPlugin
 */
typedef struct {
	/* implements bus listener interface */
	listener_t listener;
	/* IKE_SA we are listening on */
	ike_sa_t *ike_sa;
	/* backref to public plugin */
	NMVPNPlugin *plugin;
	/* credentials to use for authentication */
	nm_creds_t *creds;
	/* attribute handler for DNS/NBNS server information */
	nm_handler_t *handler;
	/* dummy TUN device */
	tun_device_t *tun;
	/* name of the connection */
	char *name;
} NMStrongswanPluginPrivate;

#define NM_STRONGSWAN_PLUGIN_GET_PRIVATE(o) \
			(G_TYPE_INSTANCE_GET_PRIVATE ((o), \
				NM_TYPE_STRONGSWAN_PLUGIN, NMStrongswanPluginPrivate))

/**
 * convert enumerated handler chunks to a UINT_ARRAY GValue
 */
static GValue* handler_to_val(nm_handler_t *handler,
							 configuration_attribute_type_t type)
{
	GValue *val;
	GArray *array;
	enumerator_t *enumerator;
	chunk_t chunk;

	enumerator = handler->create_enumerator(handler, type);
	array = g_array_new (FALSE, TRUE, sizeof (guint32));
	while (enumerator->enumerate(enumerator, &chunk))
	{
		g_array_append_val (array, *(u_int32_t*)chunk.ptr);
	}
	enumerator->destroy(enumerator);
	val = g_slice_new0 (GValue);
	g_value_init (val, DBUS_TYPE_G_UINT_ARRAY);
	g_value_set_boxed (val, array);

	return val;
}

/**
 * signal IPv4 config to NM, set connection as established
 */
static void signal_ipv4_config(NMVPNPlugin *plugin,
							   ike_sa_t *ike_sa, child_sa_t *child_sa)
{
	NMStrongswanPluginPrivate *priv = NM_STRONGSWAN_PLUGIN_GET_PRIVATE(plugin);
	GValue *val;
	GHashTable *config;
	enumerator_t *enumerator;
	host_t *me;
	nm_handler_t *handler;

	config = g_hash_table_new(g_str_hash, g_str_equal);
	handler = priv->handler;

	/* NM requires a tundev, but netkey does not use one. Passing the physical
	 * interface does not work, as NM fiddles around with it. So we pass a dummy
	 * TUN device along for NM to play with... */
	val = g_slice_new0 (GValue);
	g_value_init (val, G_TYPE_STRING);
	g_value_set_string (val, priv->tun->get_name(priv->tun));
	g_hash_table_insert (config, NM_VPN_PLUGIN_IP4_CONFIG_TUNDEV, val);

	/* NM installs this IP address on the interface above, so we use the VIP if
	 * we got one.
	 */
	enumerator = ike_sa->create_virtual_ip_enumerator(ike_sa, TRUE);
	if (!enumerator->enumerate(enumerator, &me))
	{
		me = ike_sa->get_my_host(ike_sa);
	}
	enumerator->destroy(enumerator);
	val = g_slice_new0(GValue);
	g_value_init(val, G_TYPE_UINT);
	g_value_set_uint(val, *(u_int32_t*)me->get_address(me).ptr);
	g_hash_table_insert(config, NM_VPN_PLUGIN_IP4_CONFIG_ADDRESS, val);

	val = g_slice_new0(GValue);
	g_value_init(val, G_TYPE_UINT);
	g_value_set_uint(val, me->get_address(me).len * 8);
	g_hash_table_insert(config, NM_VPN_PLUGIN_IP4_CONFIG_PREFIX, val);

	/* prevent NM from changing the default route. we set our own route in our
	 * own routing table
	 */
	val = g_slice_new0(GValue);
	g_value_init(val, G_TYPE_BOOLEAN);
	g_value_set_boolean(val, TRUE);
	g_hash_table_insert(config, NM_VPN_PLUGIN_IP4_CONFIG_NEVER_DEFAULT, val);

	val = handler_to_val(handler, INTERNAL_IP4_DNS);
	g_hash_table_insert(config, NM_VPN_PLUGIN_IP4_CONFIG_DNS, val);

	val = handler_to_val(handler, INTERNAL_IP4_NBNS);
	g_hash_table_insert(config, NM_VPN_PLUGIN_IP4_CONFIG_NBNS, val);

	handler->reset(handler);

	nm_vpn_plugin_set_ip4_config(plugin, config);
}

/**
 * signal failure to NM, connecting failed
 */
static void signal_failure(NMVPNPlugin *plugin, NMVPNPluginFailure failure)
{
	nm_handler_t *handler = NM_STRONGSWAN_PLUGIN_GET_PRIVATE(plugin)->handler;

	handler->reset(handler);

	/* TODO: NM does not handle this failure!? */
	nm_vpn_plugin_failure(plugin, failure);
	nm_vpn_plugin_set_state(plugin, NM_VPN_SERVICE_STATE_STOPPED);
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
static gboolean connect_(NMVPNPlugin *plugin, NMConnection *connection,
						 GError **err)
{
	NMStrongswanPluginPrivate *priv;
	NMSettingConnection *conn;
	NMSettingVPN *vpn;
	identification_t *user = NULL, *gateway = NULL;
	const char *address, *str;
	bool virtual, encap, ipcomp;
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
	lifetime_cfg_t lifetime = {
		.time = {
			.life = 10800 /* 3h */,
			.rekey = 10200 /* 2h50min */,
			.jitter = 300 /* 5min */
		}
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
	if (!priv->tun)
	{
		g_set_error(err, NM_VPN_PLUGIN_ERROR, NM_VPN_PLUGIN_ERROR_LAUNCH_FAILED,
					"Failed to create dummy TUN device.");
		return FALSE;
	}
	address = nm_setting_vpn_get_data_item(vpn, "address");
	if (!address || !*address)
	{
		g_set_error(err, NM_VPN_PLUGIN_ERROR, NM_VPN_PLUGIN_ERROR_BAD_ARGUMENTS,
					"Gateway address missing.");
		return FALSE;
	}
	str = nm_setting_vpn_get_data_item(vpn, "virtual");
	virtual = str && streq(str, "yes");
	str = nm_setting_vpn_get_data_item(vpn, "encap");
	encap = str && streq(str, "yes");
	str = nm_setting_vpn_get_data_item(vpn, "ipcomp");
	ipcomp = str && streq(str, "yes");
	str = nm_setting_vpn_get_data_item(vpn, "method");
	if (str)
	{
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
		priv->creds->load_ca_dir(priv->creds, NM_CA_DIR);
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
								KEY_RSA, BUILD_FROM_FILE, str, BUILD_END);
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
							 FRAGMENTATION_NO, 0);
	ike_cfg->add_proposal(ike_cfg, proposal_create_default(PROTO_IKE));
	ike_cfg->add_proposal(ike_cfg, proposal_create_default_aead(PROTO_IKE));
	peer_cfg = peer_cfg_create(priv->name, ike_cfg,
					CERT_SEND_IF_ASKED, UNIQUE_REPLACE, 1, /* keyingtries */
					36000, 0, /* rekey 10h, reauth none */
					600, 600, /* jitter, over 10min */
					TRUE, FALSE, TRUE, /* mobike, aggressive, pull */
					0, 0, /* DPD delay, timeout */
					FALSE, NULL, NULL); /* mediation */
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

	child_cfg = child_cfg_create(priv->name, &lifetime,
								 NULL, TRUE, MODE_TUNNEL, /* updown, hostaccess */
								 ACTION_NONE, ACTION_NONE, ACTION_NONE, ipcomp,
								 0, 0, NULL, NULL, 0);
	child_cfg->add_proposal(child_cfg, proposal_create_default(PROTO_ESP));
	child_cfg->add_proposal(child_cfg, proposal_create_default_aead(PROTO_ESP));
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
static gboolean need_secrets(NMVPNPlugin *plugin, NMConnection *connection,
							 char **setting_name, GError **error)
{
	NMSettingVPN *settings;
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
								KEY_RSA, BUILD_FROM_FILE, path, BUILD_END);
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
 * Disconnect called from NM via DBUS
 */
static gboolean disconnect(NMVPNPlugin *plugin, GError **err)
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
			charon->controller->terminate_ike(charon->controller, id,
											  controller_cb_empty, NULL, 0);
			return TRUE;
		}
	}
	enumerator->destroy(enumerator);

	g_set_error(err, NM_VPN_PLUGIN_ERROR, NM_VPN_PLUGIN_ERROR_GENERAL,
				"Connection not found.");
	return FALSE;
}

/**
 * Initializer
 */
static void nm_strongswan_plugin_init(NMStrongswanPlugin *plugin)
{
	NMStrongswanPluginPrivate *priv;

	priv = NM_STRONGSWAN_PLUGIN_GET_PRIVATE(plugin);
	priv->plugin = NM_VPN_PLUGIN(plugin);
	memset(&priv->listener, 0, sizeof(listener_t));
	priv->listener.child_updown = child_updown;
	priv->listener.ike_rekey = ike_rekey;
	priv->tun = tun_device_create(NULL);
	priv->name = NULL;
}

/**
 * Destructor
 */
static void nm_strongswan_plugin_dispose(GObject *obj)
{
	NMStrongswanPlugin *plugin;
	NMStrongswanPluginPrivate *priv;

	plugin = NM_STRONGSWAN_PLUGIN(obj);
	priv = NM_STRONGSWAN_PLUGIN_GET_PRIVATE(plugin);
	if (priv->tun)
	{
		priv->tun->destroy(priv->tun);
		priv->tun = NULL;
	}
}

/**
 * Class constructor
 */
static void nm_strongswan_plugin_class_init(
									NMStrongswanPluginClass *strongswan_class)
{
	NMVPNPluginClass *parent_class = NM_VPN_PLUGIN_CLASS(strongswan_class);

	g_type_class_add_private(G_OBJECT_CLASS(strongswan_class),
							 sizeof(NMStrongswanPluginPrivate));
	parent_class->connect = connect_;
	parent_class->need_secrets = need_secrets;
	parent_class->disconnect = disconnect;
	G_OBJECT_CLASS(strongswan_class)->dispose = nm_strongswan_plugin_dispose;
}

/**
 * Object constructor
 */
NMStrongswanPlugin *nm_strongswan_plugin_new(nm_creds_t *creds,
											 nm_handler_t *handler)
{
	NMStrongswanPlugin *plugin = (NMStrongswanPlugin *)g_object_new (
					NM_TYPE_STRONGSWAN_PLUGIN,
					NM_VPN_PLUGIN_DBUS_SERVICE_NAME, NM_DBUS_SERVICE_STRONGSWAN,
					NULL);
	if (plugin)
	{
		NMStrongswanPluginPrivate *priv;

		/* the rest of the initialization happened in _init above */
		priv = NM_STRONGSWAN_PLUGIN_GET_PRIVATE(plugin);
		priv->creds = creds;
		priv->handler = handler;
	}
	return plugin;
}
