/*
 * Copyright (C) 2017 Lubomir Rintel
 * Copyright (C) 2013-2023 Tobias Brunner
 * Copyright (C) 2008-2009 Martin Willi
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

#include <stdio.h>
#include <inttypes.h>
#include <net/if.h>

#include "nm_service.h"

#include <daemon.h>
#include <networking/host.h>
#include <utils/identification.h>
#include <config/peer_cfg.h>
#include <credentials/certificates/x509.h>
#include <networking/tun_device.h>
#include <plugins/kernel_netlink/kernel_netlink_xfrmi.h>

#define XFRMI_DEFAULT_MTU 1400

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
	/* manager for XFRM interfaces, if supported */
	kernel_netlink_xfrmi_t *xfrmi_manager;
	/* interface ID of XFRM interface */
	uint32_t xfrmi_id;
	/* name of XFRM interface if one is used */
	char *xfrmi;
	/* dummy TUN device if not using XFRM interface */
	tun_device_t *tun;
	/* name of the connection */
	char *name;
} NMStrongswanPluginPrivate;

G_DEFINE_TYPE_WITH_PRIVATE(NMStrongswanPlugin, nm_strongswan_plugin, NM_TYPE_VPN_SERVICE_PLUGIN)

#define NM_STRONGSWAN_PLUGIN_GET_PRIVATE(o) \
			((NMStrongswanPluginPrivate*) \
				nm_strongswan_plugin_get_instance_private (o))

/**
 * Convert an address chunk to a GValue
 */
static GVariant *addr_to_variant(chunk_t addr)
{
	GVariantBuilder builder;
	int i;

	switch (addr.len)
	{
		case 4:
			return g_variant_new_uint32 (*(uint32_t*)addr.ptr);
		case 16:
			g_variant_builder_init (&builder, G_VARIANT_TYPE ("ay"));
			for (i = 0; i < addr.len; i++)
			{
				g_variant_builder_add (&builder, "y", addr.ptr[i]);

			}
			return g_variant_builder_end (&builder);
		default:
			return NULL;
	}
}

/**
 * Convert a host to a GValue
 */
static GVariant *host_to_variant(host_t *host)
{
	return addr_to_variant(host->get_address(host));
}

/**
 * Convert enumerated handler chunks to a GValue
 */
static GVariant* handler_to_variant(nm_handler_t *handler, char *variant_type,
							 configuration_attribute_type_t type)
{
	GVariantBuilder builder;
	enumerator_t *enumerator;
	chunk_t *chunk;

	g_variant_builder_init (&builder, G_VARIANT_TYPE (variant_type));

	enumerator = handler->create_enumerator(handler, type);
	while (enumerator->enumerate(enumerator, &chunk))
	{
		g_variant_builder_add_value (&builder, addr_to_variant(*chunk));
	}
	enumerator->destroy(enumerator);

	return g_variant_builder_end (&builder);
}

/**
 * Destroy any allocated XFRM or TUN interface
 */
static void delete_interface(NMStrongswanPluginPrivate *priv)
{
	if (priv->xfrmi)
	{
		priv->xfrmi_manager->delete(priv->xfrmi_manager, priv->xfrmi);
		free(priv->xfrmi);
		priv->xfrmi = NULL;
	}
	if (priv->tun)
	{
		priv->tun->destroy(priv->tun);
		priv->tun = NULL;
	}
}

/**
 * Create an XFRM or TUN interface
 */
static void create_interface(NMStrongswanPluginPrivate *priv,
							 const char *interface_name)
{
	if (priv->xfrmi_manager)
	{
		char name[IFNAMSIZ];
		int mtu;

		/* allocate a random interface ID */
		priv->xfrmi_id = random();

		if (interface_name)
		{	/* use the preferred interface name if one is provided */
			snprintf(name, sizeof(name), "%s", interface_name);
		}
		else
		{
			/* use the interface ID to get a unique name, fine if it's cut off */
			snprintf(name, sizeof(name), "nm-xfrm-%" PRIu32, priv->xfrmi_id);
		}

		mtu = lib->settings->get_int(lib->settings, "charon-nm.mtu",
									 XFRMI_DEFAULT_MTU);

		if (priv->xfrmi_manager->create(priv->xfrmi_manager, name,
										priv->xfrmi_id, NULL, mtu))
		{
			priv->xfrmi = strdup(name);
		}
		else
		{
			priv->xfrmi_id = 0;
		}
	}
	if (!priv->xfrmi)
	{	/* use a TUN device as fallback */
		priv->tun = tun_device_create(NULL);
	}

	if (priv->xfrmi)
	{
		DBG1(DBG_CFG, "created XFRM interface %s for NetworkManager connection "
			 "%s", priv->xfrmi, priv->name);
	}
	else if (priv->tun)
	{
		DBG1(DBG_CFG, "created TUN device %s for NetworkManager connection "
			 "%s", priv->tun->get_name(priv->tun), priv->name);
	}
	else
	{
		DBG1(DBG_CFG, "failed to create XFRM or dummy TUN device, might affect "
			 "DNS server installation negatively");
	}
}

/**
 * Signal IP config to NM, set connection as established
 */
static void signal_ip_config(NMVpnServicePlugin *plugin,
							 ike_sa_t *ike_sa, child_sa_t *child_sa)
{
	NMStrongswanPlugin *pub = (NMStrongswanPlugin*)plugin;
	NMStrongswanPluginPrivate *priv = NM_STRONGSWAN_PLUGIN_GET_PRIVATE(pub);
	GVariantBuilder builder, ip4builder, ip6builder;
	GVariant *ip4config, *ip6config;
	enumerator_t *enumerator;
	host_t *me, *other, *vip4 = NULL, *vip6 = NULL;
	nm_handler_t *handler;

	g_variant_builder_init (&builder, G_VARIANT_TYPE_VARDICT);
	g_variant_builder_init (&ip4builder, G_VARIANT_TYPE_VARDICT);
	g_variant_builder_init (&ip6builder, G_VARIANT_TYPE_VARDICT);

	handler = priv->handler;

	/* NM apparently requires to know the gateway (it uses it to install a
	 * direct route via physical interface if conflicting routes are passed) */
	other = ike_sa->get_other_host(ike_sa);
	g_variant_builder_add (&builder, "{sv}", NM_VPN_PLUGIN_CONFIG_EXT_GATEWAY,
						   host_to_variant(other));

	if (priv->xfrmi)
	{
		g_variant_builder_add (&builder, "{sv}", NM_VPN_PLUGIN_CONFIG_TUNDEV,
							   g_variant_new_string (priv->xfrmi));
	}
	else if (priv->tun)
	{
		g_variant_builder_add (&builder, "{sv}", NM_VPN_PLUGIN_CONFIG_TUNDEV,
							   g_variant_new_string (priv->tun->get_name(priv->tun)));
	}

	/* pass the first virtual IPs we got or use the physical IP */
	enumerator = ike_sa->create_virtual_ip_enumerator(ike_sa, TRUE);
	while (enumerator->enumerate(enumerator, &me))
	{
		switch (me->get_family(me))
		{
			case AF_INET:
				if (!vip4)
				{
					vip4 = me;
				}
				break;
			case AF_INET6:
				if (!vip6)
				{
					vip6 = me;
				}
				break;
		}
	}
	enumerator->destroy(enumerator);
	if (!vip4 && !vip6)
	{
		me = ike_sa->get_my_host(ike_sa);
		switch (me->get_family(me))
		{
			case AF_INET:
				vip4 = me;
				break;
			case AF_INET6:
				vip6 = me;
				break;
		}
	}

	if (vip4)
	{
		g_variant_builder_add (&ip4builder, "{sv}", NM_VPN_PLUGIN_IP4_CONFIG_ADDRESS,
							   host_to_variant(vip4));
		g_variant_builder_add (&ip4builder, "{sv}", NM_VPN_PLUGIN_IP4_CONFIG_PREFIX,
							   g_variant_new_uint32 (vip4->get_address(vip4).len * 8));
		g_variant_builder_add (&ip4builder, "{sv}", NM_VPN_PLUGIN_IP4_CONFIG_DNS,
							   handler_to_variant(handler, "au", INTERNAL_IP4_DNS));
		g_variant_builder_add (&ip4builder, "{sv}", NM_VPN_PLUGIN_IP4_CONFIG_NBNS,
							   handler_to_variant(handler, "au", INTERNAL_IP4_NBNS));

		/* prevent NM from changing the default route, as we set our own routes
		 * in a separate routing table
		 */
		g_variant_builder_add (&ip4builder, "{sv}", NM_VPN_PLUGIN_IP4_CONFIG_NEVER_DEFAULT,
							   g_variant_new_boolean (TRUE));
	}

	if (vip6)
	{
		g_variant_builder_add (&ip6builder, "{sv}", NM_VPN_PLUGIN_IP6_CONFIG_ADDRESS,
							   host_to_variant(vip6));
		g_variant_builder_add (&ip6builder, "{sv}", NM_VPN_PLUGIN_IP6_CONFIG_PREFIX,
							   g_variant_new_uint32 (vip6->get_address(vip6).len * 8));
		g_variant_builder_add (&ip6builder, "{sv}", NM_VPN_PLUGIN_IP6_CONFIG_DNS,
							   handler_to_variant(handler, "aay", INTERNAL_IP6_DNS));
		/* NM_VPN_PLUGIN_IP6_CONFIG_NBNS is not defined */

		g_variant_builder_add (&ip6builder, "{sv}", NM_VPN_PLUGIN_IP6_CONFIG_NEVER_DEFAULT,
							   g_variant_new_boolean (TRUE));
	}

	ip4config = g_variant_builder_end (&ip4builder);
	if (g_variant_n_children (ip4config))
	{
		g_variant_builder_add (&builder, "{sv}", NM_VPN_PLUGIN_CONFIG_HAS_IP4,
							   g_variant_new_boolean (TRUE));
	}
	else
	{
		g_variant_unref (ip4config);
		ip4config = NULL;
	}

	ip6config = g_variant_builder_end (&ip6builder);
	if (g_variant_n_children (ip6config))
	{
		g_variant_builder_add (&builder, "{sv}", NM_VPN_PLUGIN_CONFIG_HAS_IP6,
							   g_variant_new_boolean (TRUE));
	}
	else
	{
		g_variant_unref (ip6config);
		ip6config = NULL;
	}

	handler->reset(handler);

	nm_vpn_service_plugin_set_config (plugin, g_variant_builder_end (&builder));
	if (ip4config)
	{
		nm_vpn_service_plugin_set_ip4_config (plugin, ip4config);
	}
	if (ip6config)
	{
		nm_vpn_service_plugin_set_ip6_config (plugin, ip6config);
	}
}

/**
 * signal failure to NM, connecting failed
 */
static void signal_failure(NMVpnServicePlugin *plugin, NMVpnPluginFailure failure)
{
	NMStrongswanPlugin *pub = (NMStrongswanPlugin*)plugin;
	nm_handler_t *handler = NM_STRONGSWAN_PLUGIN_GET_PRIVATE(pub)->handler;

	handler->reset(handler);

	nm_vpn_service_plugin_failure(plugin, failure);
}

METHOD(listener_t, ike_state_change, bool,
	NMStrongswanPluginPrivate *this, ike_sa_t *ike_sa, ike_sa_state_t state)
{
	if (this->ike_sa == ike_sa && state == IKE_DESTROYING)
	{
		signal_failure(this->plugin, NM_VPN_PLUGIN_FAILURE_LOGIN_FAILED);
	}
	return TRUE;
}

METHOD(listener_t, child_state_change, bool,
	NMStrongswanPluginPrivate *this, ike_sa_t *ike_sa, child_sa_t *child_sa,
	child_sa_state_t state)
{
	if (this->ike_sa == ike_sa && state == CHILD_DESTROYING)
	{
		signal_failure(this->plugin, NM_VPN_PLUGIN_FAILURE_CONNECT_FAILED);
	}
	return TRUE;
}

METHOD(listener_t, ike_rekey, bool,
	NMStrongswanPluginPrivate *this, ike_sa_t *old, ike_sa_t *new)
{
	if (this->ike_sa == old)
	{	/* follow a rekeyed IKE_SA */
		this->ike_sa = new;
	}
	return TRUE;
}

METHOD(listener_t, ike_reestablish_pre, bool,
	NMStrongswanPluginPrivate *this, ike_sa_t *old, ike_sa_t *new)
{
	if (this->ike_sa == old)
	{	/* ignore child state changes during redirects etc. (task migration) */
		this->listener.child_state_change = NULL;
	}
	return TRUE;
}

METHOD(listener_t, ike_reestablish_post, bool,
	NMStrongswanPluginPrivate *this, ike_sa_t *old, ike_sa_t *new,
	bool initiated)
{
	if (this->ike_sa == old && initiated)
	{	/* if we get redirected during IKE_AUTH we just migrate to the new SA */
		this->ike_sa = new;
		/* re-register hooks to detect initiation failures */
		this->listener.ike_state_change = _ike_state_change;
		this->listener.child_state_change = _child_state_change;
	}
	return TRUE;
}

METHOD(listener_t, child_updown, bool,
	NMStrongswanPluginPrivate *this, ike_sa_t *ike_sa, child_sa_t *child_sa,
	bool up)
{
	if (this->ike_sa == ike_sa && up)
	{
		/* disable initiate-failure-detection hooks */
		this->listener.ike_state_change = NULL;
		this->listener.child_state_change = NULL;
		signal_ip_config(this->plugin, ike_sa, child_sa);
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
 * Add a client auth config for certificate authentication
 */
static bool add_auth_cfg_cert(NMStrongswanPluginPrivate *priv,
							  NMSettingVpn *vpn, peer_cfg_t *peer_cfg,
							  GError **err)
{
	identification_t *id = NULL;
	certificate_t *cert = NULL;
	auth_cfg_t *auth;
	const char *str, *method, *cert_source;

	method = nm_setting_vpn_get_data_item(vpn, "method");
	cert_source = nm_setting_vpn_get_data_item(vpn, "cert-source") ?: method;

	if (streq(cert_source, "smartcard"))
	{
		char *pin;

		pin = (char*)nm_setting_vpn_get_secret(vpn, "password");
		if (pin)
		{
			id = find_smartcard_key(priv, pin);
		}
		if (!id)
		{
			g_set_error(err, NM_VPN_PLUGIN_ERROR,
						NM_VPN_PLUGIN_ERROR_BAD_ARGUMENTS,
						"No usable smartcard certificate found.");
			return FALSE;
		}
	}
	/* ... or certificate/private key authentication */
	else if ((str = nm_setting_vpn_get_data_item(vpn, "usercert")))
	{
		public_key_t *public;
		private_key_t *private = NULL;

		bool agent = streq(cert_source, "agent");

		cert = lib->creds->create(lib->creds, CRED_CERTIFICATE, CERT_X509,
								  BUILD_FROM_FILE, str, BUILD_END);
		if (!cert)
		{
			g_set_error(err, NM_VPN_PLUGIN_ERROR,
						NM_VPN_PLUGIN_ERROR_BAD_ARGUMENTS,
						"Loading peer certificate failed.");
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
			id = cert->get_subject(cert);
			id = id->clone(id);
			priv->creds->set_cert_and_key(priv->creds, cert, private);
		}
		else
		{
			DESTROY_IF(cert);
			return FALSE;
		}
	}
	else
	{
		g_set_error(err, NM_VPN_PLUGIN_ERROR, NM_VPN_PLUGIN_ERROR_BAD_ARGUMENTS,
					"Certificate is missing.");
		return FALSE;
	}

	auth = auth_cfg_create();
	if (streq(method, "eap-tls"))
	{
		auth->add(auth, AUTH_RULE_AUTH_CLASS, AUTH_CLASS_EAP);
		auth->add(auth, AUTH_RULE_EAP_TYPE, EAP_TLS);
		auth->add(auth, AUTH_RULE_AAA_IDENTITY,
				  identification_create_from_string("%any"));
	}
	else
	{
		auth->add(auth, AUTH_RULE_AUTH_CLASS, AUTH_CLASS_PUBKEY);
	}
	if (cert)
	{
		auth->add(auth, AUTH_RULE_SUBJECT_CERT, cert->get_ref(cert));
	}
	str = nm_setting_vpn_get_data_item(vpn, "local-identity");
	if (str)
	{
		identification_t *local_id;

		local_id = identification_create_from_string((char*)str);
		if (local_id)
		{
			id->destroy(id);
			id = local_id;
		}
	}
	auth->add(auth, AUTH_RULE_IDENTITY, id);
	peer_cfg->add_auth_cfg(peer_cfg, auth, TRUE);
	return TRUE;
}

/**
 * Add a client auth config for username/password authentication
 */
static bool add_auth_cfg_pw(NMStrongswanPluginPrivate *priv,
							NMSettingVpn *vpn, peer_cfg_t *peer_cfg,
							GError **err)
{
	identification_t *user = NULL, *id = NULL;
	auth_cfg_t *auth;
	const char *str, *method;

	method = nm_setting_vpn_get_data_item(vpn, "method");

	str = nm_setting_vpn_get_data_item(vpn, "user");
	if (str)
	{
		user = identification_create_from_string((char*)str);
	}
	else
	{
		user = identification_create_from_string("%any");
	}
	str = nm_setting_vpn_get_data_item(vpn, "local-identity");
	if (str)
	{
		id = identification_create_from_string((char*)str);
	}
	else
	{
		id = user->clone(user);
	}
	str = nm_setting_vpn_get_secret(vpn, "password");
	if (streq(method, "psk"))
	{
		if (strlen(str) < 20)
		{
			g_set_error(err, NM_VPN_PLUGIN_ERROR,
						NM_VPN_PLUGIN_ERROR_BAD_ARGUMENTS,
						"Pre-shared key is too short.");
			user->destroy(user);
			id->destroy(id);
			return FALSE;
		}
		priv->creds->set_username_password(priv->creds, id, (char*)str);
	}
	else
	{
		priv->creds->set_username_password(priv->creds, user, (char*)str);
	}

	auth = auth_cfg_create();
	auth->add(auth, AUTH_RULE_AUTH_CLASS,
			  streq(method, "psk") ? AUTH_CLASS_PSK : AUTH_CLASS_EAP);
	/* in case EAP-PEAP or EAP-TTLS is used we currently accept any identity */
	auth->add(auth, AUTH_RULE_AAA_IDENTITY,
			  identification_create_from_string("%any"));
	auth->add(auth, AUTH_RULE_EAP_IDENTITY, user);
	auth->add(auth, AUTH_RULE_IDENTITY, id);
	peer_cfg->add_auth_cfg(peer_cfg, auth, TRUE);
	return TRUE;
}

/**
 * Connect function called from NM via DBUS
 */
static gboolean connect_(NMVpnServicePlugin *plugin, NMConnection *connection,
						 GError **err)
{
	NMStrongswanPlugin *pub = (NMStrongswanPlugin*)plugin;
	NMStrongswanPluginPrivate *priv;
	NMSettingConnection *conn;
	NMSettingVpn *vpn;
	enumerator_t *enumerator;
	identification_t *gateway = NULL;
	const char *str, *method;
	bool virtual, proposal;
	proposal_t *prop;
	ike_cfg_t *ike_cfg;
	peer_cfg_t *peer_cfg;
	child_cfg_t *child_cfg;
	traffic_selector_t *ts;
	ike_sa_t *ike_sa;
	auth_cfg_t *auth;
	certificate_t *cert = NULL;
	x509_t *x509;
	bool loose_gateway_id = FALSE;
	ike_cfg_create_t ike = {
		.version = IKEV2,
		.local = "%any",
		.local_port = charon->socket->get_port(charon->socket, FALSE),
		.remote_port = IKEV2_UDP_PORT,
		.fragmentation = FRAGMENTATION_YES,
	};
	peer_cfg_create_t peer = {
		.cert_policy = CERT_SEND_IF_ASKED,
		.unique = UNIQUE_REPLACE,
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
		.dpd_action = ACTION_START,
		.close_action = ACTION_START,
	};

	/**
	 * Read parameters
	 */
	priv = NM_STRONGSWAN_PLUGIN_GET_PRIVATE(pub);
	conn = NM_SETTING_CONNECTION(nm_connection_get_setting(connection,
												NM_TYPE_SETTING_CONNECTION));
	vpn = NM_SETTING_VPN(nm_connection_get_setting(connection,
												NM_TYPE_SETTING_VPN));
	free(priv->name);
	priv->name = strdup(nm_setting_connection_get_id(conn));
	DBG1(DBG_CFG, "received initiate for NetworkManager connection %s",
		 priv->name);
	DBG3(DBG_CFG, "%s",
		 nm_setting_to_string(NM_SETTING(conn)));
	DBG4(DBG_CFG, "%s",
		 nm_setting_to_string(NM_SETTING(vpn)));

	ike.remote = (char*)nm_setting_vpn_get_data_item(vpn, "address");
	if (!ike.remote || !*ike.remote)
	{
		g_set_error(err, NM_VPN_PLUGIN_ERROR, NM_VPN_PLUGIN_ERROR_BAD_ARGUMENTS,
					"Gateway address missing.");
		return FALSE;
	}
	str = nm_setting_vpn_get_data_item(vpn, "server-port");
	if (str && strlen(str))
	{
		ike.remote_port = settings_value_as_int((char*)str, ike.remote_port);
	}
	str = nm_setting_vpn_get_data_item(vpn, "virtual");
	virtual = streq(str, "yes");
	str = nm_setting_vpn_get_data_item(vpn, "encap");
	ike.force_encap = streq(str, "yes");
	str = nm_setting_vpn_get_data_item(vpn, "ipcomp");
	child.options |= streq(str, "yes") ? OPT_IPCOMP : 0;

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
	}
	else
	{
		/* no certificate defined, fall back to system-wide CA certificates */
		priv->creds->load_ca_dir(priv->creds, lib->settings->get_str(
								 lib->settings, "charon-nm.ca_dir", NM_CA_DIR));
	}

	str = nm_setting_vpn_get_data_item(vpn, "remote-identity");
	if (str)
	{
		gateway = identification_create_from_string((char*)str);
	}
	else if (cert)
	{
		x509 = (x509_t*)cert;
		if (!(x509->get_flags(x509) & X509_CA))
		{	/* for server certificates, we use the subject as identity */
			gateway = cert->get_subject(cert);
			gateway = gateway->clone(gateway);
		}
	}
	if (!gateway || gateway->get_type(gateway) == ID_ANY)
	{
		/* if the user configured a CA certificate (or an invalid identity),
		 * we use the IP/hostname of the server */
		gateway = identification_create_from_string(ike.remote);
		loose_gateway_id = TRUE;
	}
	DBG1(DBG_CFG, "using gateway identity '%Y'", gateway);

	/**
	 * Set up configurations
	 */
	ike_cfg = ike_cfg_create(&ike);

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
		peer_cfg->add_virtual_ip(peer_cfg, host_create_any(AF_INET));
		peer_cfg->add_virtual_ip(peer_cfg, host_create_any(AF_INET6));
	}

	method = nm_setting_vpn_get_data_item(vpn, "method");
	if (streq(method, "cert") ||
		streq(method, "eap-tls") ||
		streq(method, "key") ||
		streq(method, "agent") ||
		streq(method, "smartcard"))
	{
		if (!add_auth_cfg_cert (priv, vpn, peer_cfg, err))
		{
			peer_cfg->destroy(peer_cfg);
			ike_cfg->destroy(ike_cfg);
			gateway->destroy(gateway);
			return FALSE;
		}
	}
	else if (streq(method, "eap") ||
			 streq(method, "psk"))
	{
		if (!add_auth_cfg_pw(priv, vpn, peer_cfg, err))
		{
			peer_cfg->destroy(peer_cfg);
			ike_cfg->destroy(ike_cfg);
			gateway->destroy(gateway);
			return FALSE;
		}
	}
	else
	{
		g_set_error(err, NM_VPN_PLUGIN_ERROR, NM_VPN_PLUGIN_ERROR_BAD_ARGUMENTS,
					"Configuration parameters missing.");
		peer_cfg->destroy(peer_cfg);
		ike_cfg->destroy(ike_cfg);
		gateway->destroy(gateway);
		return FALSE;
	}

	auth = auth_cfg_create();
	if (streq(method, "psk"))
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

	/* systemd-resolved requires a device to properly install DNS servers, but
	 * Netkey does not require one.  Passing the physical interface is not ideal,
	 * as NM fiddles around with it and systemd-resolved likes a separate
	 * device. So we pass either an XFRM interface or a dummy TUN device along
	 * for NM etc. to play with...
	 */
	delete_interface(priv);
	create_interface(priv, nm_setting_connection_get_interface_name(conn));
	if (priv->xfrmi_id)
	{	/* set the same mark as for IKE packets on the ESP packets so no routing
		 * loop is created if the TS covers the VPN server's IP */
		child.set_mark_out = (mark_t){
			.value = 220,
			.mask = 0xffffffff,
		};
		child.if_id_in = child.if_id_out = priv->xfrmi_id;
	}

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
		child_cfg->add_proposal(child_cfg, proposal_create_default_aead(PROTO_ESP));
		child_cfg->add_proposal(child_cfg, proposal_create_default(PROTO_ESP));
	}
	ts = traffic_selector_create_dynamic(0, 0, 65535);
	child_cfg->add_traffic_selector(child_cfg, TRUE, ts);
	str = nm_setting_vpn_get_data_item(vpn, "remote-ts");
	if (str && strlen(str))
	{
		enumerator = enumerator_create_token(str, ";", "");
		while (enumerator->enumerate(enumerator, &str))
		{
			ts = traffic_selector_create_from_cidr((char*)str, 0, 0, 65535);
			if (!ts)
			{
				g_set_error(err, NM_VPN_PLUGIN_ERROR,
							NM_VPN_PLUGIN_ERROR_LAUNCH_FAILED,
							"Invalid remote traffic selector.");
				enumerator->destroy(enumerator);
				child_cfg->destroy(child_cfg);
				peer_cfg->destroy(peer_cfg);
				return FALSE;
			}
			child_cfg->add_traffic_selector(child_cfg, FALSE, ts);
		}
		enumerator->destroy(enumerator);
	}
	else
	{
		ts = traffic_selector_create_from_cidr("0.0.0.0/0", 0, 0, 65535);
		child_cfg->add_traffic_selector(child_cfg, FALSE, ts);
		ts = traffic_selector_create_from_cidr("::/0", 0, 0, 65535);
		child_cfg->add_traffic_selector(child_cfg, FALSE, ts);
	}
	peer_cfg->add_child_cfg(peer_cfg, child_cfg);

	/**
	 * Prepare IKE_SA
	 */
	ike_sa = charon->ike_sa_manager->checkout_by_config(charon->ike_sa_manager,
														peer_cfg);
	peer_cfg->destroy(peer_cfg);
	if (!ike_sa)
	{
		g_set_error(err, NM_VPN_PLUGIN_ERROR, NM_VPN_PLUGIN_ERROR_LAUNCH_FAILED,
					"IKE version not supported.");
		return FALSE;
	}

	/**
	 * Register listener, enable  initiate-failure-detection hooks
	 */
	priv->ike_sa = ike_sa;
	priv->listener.ike_state_change = _ike_state_change;
	priv->listener.child_state_change = _child_state_change;

	/**
	 * Initiate
	 */
	child_cfg->get_ref(child_cfg);
	if (ike_sa->initiate(ike_sa, child_cfg, NULL) != SUCCESS)
	{
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
	const char *method, *cert_source, *path;
	bool need_secret = FALSE;

	settings = NM_SETTING_VPN(nm_connection_get_setting(connection,
														NM_TYPE_SETTING_VPN));
	method = nm_setting_vpn_get_data_item(settings, "method");
	if (method)
	{
		if (streq(method, "cert") ||
			streq(method, "eap-tls") ||
			streq(method, "key") ||
			streq(method, "agent") ||
			streq(method, "smartcard"))
		{
			cert_source = nm_setting_vpn_get_data_item(settings, "cert-source");
			if (!cert_source)
			{
				cert_source = method;
			}
			if (streq(cert_source, "agent"))
			{
				need_secret = !nm_setting_vpn_get_secret(settings, "agent");
			}
			else if (streq(cert_source, "smartcard"))
			{
				need_secret = !nm_setting_vpn_get_secret(settings, "password");
			}
			else
			{
				need_secret = TRUE;
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
						need_secret = FALSE;
					}
					else if (nm_setting_vpn_get_secret(settings, "password"))
					{
						need_secret = FALSE;
					}
				}
			}
		}
		else if (streq(method, "eap") ||
				 streq(method, "psk"))
		{
			need_secret = !nm_setting_vpn_get_secret(settings, "password");
		}
	}
	*setting_name = NM_SETTING_VPN_SETTING_NAME;
	return need_secret;
}

/**
 * The actual disconnection
 */
static gboolean do_disconnect(gpointer plugin)
{
	NMStrongswanPluginPrivate *priv = NM_STRONGSWAN_PLUGIN_GET_PRIVATE(plugin);
	enumerator_t *enumerator;
	ike_sa_t *ike_sa;
	u_int id = 0;

	/* our ike_sa pointer might be invalid, lookup sa */
	enumerator = charon->controller->create_ike_sa_enumerator(
													charon->controller, TRUE);
	while (enumerator->enumerate(enumerator, &ike_sa))
	{
		if (priv->ike_sa == ike_sa)
		{
			id = ike_sa->get_unique_id(ike_sa);
			break;
		}
	}
	enumerator->destroy(enumerator);

	if (id)
	{
		charon->controller->terminate_ike(charon->controller, id, FALSE,
									controller_cb_empty, NULL, LEVEL_SILENT, 0);
	}
	else
	{
		g_debug("Connection not found.");
	}

	/* clear secrets as we are asked for new secrets (where we'd find the cached
	 * secrets from earlier connections) before we clear them in connect() */
	priv->creds->clear(priv->creds);

	/* delete any allocated interface */
	delete_interface(priv);
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
	priv->listener.child_updown = _child_updown;
	priv->listener.ike_rekey = _ike_rekey;
	priv->listener.ike_reestablish_pre = _ike_reestablish_pre;
	priv->listener.ike_reestablish_post = _ike_reestablish_post;
	charon->bus->add_listener(charon->bus, &priv->listener);
	priv->xfrmi_manager = lib->get(lib, KERNEL_NETLINK_XFRMI_MANAGER);
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
	delete_interface(priv);
	free(priv->name);
	G_OBJECT_CLASS (nm_strongswan_plugin_parent_class)->dispose (obj);
}

/**
 * Class constructor
 */
static void nm_strongswan_plugin_class_init(
									NMStrongswanPluginClass *strongswan_class)
{
	NMVpnServicePluginClass *parent_class = NM_VPN_SERVICE_PLUGIN_CLASS(strongswan_class);

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
