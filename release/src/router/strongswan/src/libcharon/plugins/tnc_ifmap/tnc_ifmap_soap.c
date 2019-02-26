/*
 * Copyright (C) 2011-2013 Andreas Steffen
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

#include "tnc_ifmap_soap.h"
#include "tnc_ifmap_soap_msg.h"

#include <utils/debug.h>
#include <credentials/sets/mem_cred.h>
#include <daemon.h>

#include <tls_socket.h>

#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>

#define IFMAP_NS		"http://www.trustedcomputinggroup.org/2010/IFMAP/2"
#define IFMAP_META_NS	"http://www.trustedcomputinggroup.org/2010/IFMAP-METADATA/2"
#define IFMAP_URI		"https://localhost:8444/imap"
#define IFMAP_NO_FD		-1

typedef struct private_tnc_ifmap_soap_t private_tnc_ifmap_soap_t;

/**
 * Private data of an tnc_ifmap_soap_t object.
 */
struct private_tnc_ifmap_soap_t {

	/**
	 * Public tnc_ifmap_soap_t interface.
	 */
	tnc_ifmap_soap_t public;

	/**
	 * SOAP Session ID
	 */
	xmlChar *session_id;

	/**
	 * IF-MAP Publisher ID
	 */
	xmlChar *ifmap_publisher_id;

	/**
	 * IF-MAP namespace
	 */
	xmlNsPtr ns;

	/**
	 * IF-MAP metadata namespace
	 */
	xmlNsPtr ns_meta;

	/**
	 * PEP and PDP device name
	 */
	char *device_name;

	/**
	 * HTTPS Server URI with https:// prefix removed
	 */
	char *uri;

	/**
	 * Optional base64-encoded username:password for HTTP Basic Authentication
	 */
	chunk_t user_pass;

	/**
	 * IF-MAP Server (IP address and port)
	 */
	host_t *host;

	/**
	 * TLS socket
	 */
	tls_socket_t *tls;

	/**
	 * File descriptor for secure TCP socket
	 */
	int fd;

	/**
	 * In memory credential set
	 */
	mem_cred_t *creds;

	/**
	 * reference count
	 */
	refcount_t ref;

};

METHOD(tnc_ifmap_soap_t, newSession, bool,
	private_tnc_ifmap_soap_t *this)
{
	tnc_ifmap_soap_msg_t *soap_msg;
	xmlNodePtr request, result;

	/*build newSession request */
	request = xmlNewNode(NULL, "newSession");
	this->ns = xmlNewNs(request, IFMAP_NS, "ifmap");
	xmlSetNs(request, this->ns);

	soap_msg = tnc_ifmap_soap_msg_create(this->uri, this->user_pass, this->tls);
	if (!soap_msg->post(soap_msg, request, "newSessionResult", &result))
	{
		soap_msg->destroy(soap_msg);
		return FALSE;
	}

	/* get session-id and ifmap-publisher-id properties */
	this->session_id = xmlGetProp(result, "session-id");
	this->ifmap_publisher_id = xmlGetProp(result, "ifmap-publisher-id");
	soap_msg->destroy(soap_msg);

	DBG1(DBG_TNC, "created ifmap session '%s' as publisher '%s'",
				   this->session_id, this->ifmap_publisher_id);

	/* set PEP and PDP device name (defaults to IF-MAP Publisher ID) */
	this->device_name = lib->settings->get_str(lib->settings,
										"%s.plugins.tnc-ifmap.device_name",
										 this->ifmap_publisher_id, lib->ns);
	this->device_name = strdup(this->device_name);

    return this->session_id && this->ifmap_publisher_id;
}

METHOD(tnc_ifmap_soap_t, renewSession, bool,
	private_tnc_ifmap_soap_t *this)
{
	tnc_ifmap_soap_msg_t *soap_msg;
	xmlNodePtr request;
	bool success;

	/* build renewSession request */
	request = xmlNewNode(NULL, "renewSession");
	this->ns = xmlNewNs(request, IFMAP_NS, "ifmap");
	xmlSetNs(request, this->ns);
	xmlNewProp(request, "session-id", this->session_id);

	soap_msg = tnc_ifmap_soap_msg_create(this->uri, this->user_pass, this->tls);
	success = soap_msg->post(soap_msg, request, "renewSessionResult", NULL);
	soap_msg->destroy(soap_msg);

	return success;
}

METHOD(tnc_ifmap_soap_t, purgePublisher, bool,
	private_tnc_ifmap_soap_t *this)
{
	tnc_ifmap_soap_msg_t *soap_msg;
	xmlNodePtr request;
	bool success;

	/* build purgePublisher request */
	request = xmlNewNode(NULL, "purgePublisher");
	this->ns = xmlNewNs(request, IFMAP_NS, "ifmap");
	xmlSetNs(request, this->ns);
	xmlNewProp(request, "session-id", this->session_id);
	xmlNewProp(request, "ifmap-publisher-id", this->ifmap_publisher_id);

	soap_msg = tnc_ifmap_soap_msg_create(this->uri, this->user_pass, this->tls);
	success = soap_msg->post(soap_msg, request, "purgePublisherReceived", NULL);
	soap_msg->destroy(soap_msg);

	return success;
}

/**
 * Create an access-request based on device_name and ike_sa_id
 */
static xmlNodePtr create_access_request(private_tnc_ifmap_soap_t *this,
										uint32_t id)
{
	xmlNodePtr node;
	char buf[BUF_LEN];

	node = xmlNewNode(NULL, "access-request");

	snprintf(buf, BUF_LEN, "%s:%d", this->device_name, id);
	xmlNewProp(node, "name", buf);

	return node;
}

/**
 * Create an identity
 */
static xmlNodePtr create_identity(private_tnc_ifmap_soap_t *this,
								  identification_t *id, bool is_user)
{
	xmlNodePtr node;
	char buf[BUF_LEN], *id_type;

	node = xmlNewNode(NULL, "identity");

	snprintf(buf, BUF_LEN, "%Y", id);
	xmlNewProp(node, "name", buf);

	switch (id->get_type(id))
	{
		case ID_IPV4_ADDR:
			id_type = "other";
			xmlNewProp(node, "other-type-definition", "36906:ipv4-address");
			break;
		case ID_FQDN:
			id_type = is_user ? "username" : "dns-name";
			break;
		case ID_RFC822_ADDR:
			id_type = "email-address";
			break;
		case ID_IPV6_ADDR:
			id_type = "other";
			xmlNewProp(node, "other-type-definition", "36906:ipv6-address");
			break;
		case ID_DER_ASN1_DN:
			id_type = "distinguished-name";
			break;
		case ID_KEY_ID:
			id_type = "other";
			xmlNewProp(node, "other-type-definition", "36906:key-id");
			break;
		default:
			id_type = "other";
			xmlNewProp(node, "other-type-definition", "36906:other");
	}
	xmlNewProp(node, "type", id_type);

	return node;
}

/**
 * Create enforcement-report metadata
 */
static xmlNodePtr create_enforcement_report(private_tnc_ifmap_soap_t *this,
											xmlChar *action, xmlChar *reason)
{
	xmlNodePtr node, node2, node3;

	node = xmlNewNode(NULL, "metadata");
	node2 = xmlNewNode(this->ns_meta, "enforcement-report");
	xmlAddChild(node, node2);
	xmlNewProp(node2, "ifmap-cardinality", "multiValue");

	node3 = xmlNewNode(NULL, "enforcement-action");
	xmlAddChild(node2, node3);
	xmlNodeAddContent(node3, action);

	node3 = xmlNewNode(NULL, "enforcement-reason");
	xmlAddChild(node2, node3);
	xmlNodeAddContent(node3, reason);

    return node;
}

/**
 * Create delete filter
 */
static xmlNodePtr create_delete_filter(private_tnc_ifmap_soap_t *this,
									   char *metadata)
{
	xmlNodePtr node;
	char buf[BUF_LEN];

	node = xmlNewNode(NULL, "delete");

	snprintf(buf, BUF_LEN, "meta:%s[@ifmap-publisher-id='%s']",
			 metadata, this->ifmap_publisher_id);
	xmlNewProp(node, "filter", buf);

	return node;
}

/**
 * Create a publish request
 */
static xmlNodePtr create_publish_request(private_tnc_ifmap_soap_t *this)
{
	xmlNodePtr request;

	request = xmlNewNode(NULL, "publish");
	this->ns = xmlNewNs(request, IFMAP_NS, "ifmap");
	xmlSetNs(request, this->ns);
	this->ns_meta = xmlNewNs(request, IFMAP_META_NS, "meta");
	xmlNewProp(request, "session-id", this->session_id);

	return request;
}

/**
 * Create a device
 */
static xmlNodePtr create_device(private_tnc_ifmap_soap_t *this)
{
	xmlNodePtr node, node2;

	node = xmlNewNode(NULL, "device");
	node2 = xmlNewNode(NULL, "name");
	xmlAddChild(node, node2);
	xmlNodeAddContent(node2, this->device_name);

	return node;
}

/**
 * Create an ip-address
 */
static xmlNodePtr create_ip_address(private_tnc_ifmap_soap_t *this,
									host_t *host)
{
	xmlNodePtr node;
	char buf[BUF_LEN];

	node = xmlNewNode(NULL, "ip-address");

	if (host->get_family(host) == AF_INET6)
	{
		chunk_t address;
		int len, written, i;
		char *pos;
		bool first = TRUE;

		/* output IPv6 address in canonical IF-MAP 2.0 format */
		address = host->get_address(host);
		pos = buf;
		len = sizeof(buf);

		for (i = 0; i < address.len; i = i + 2)
		{
			written = snprintf(pos, len, "%s%x", first ? "" : ":",
							   256*address.ptr[i] +  address.ptr[i+1]);
			if (written < 0 || written >= len)
			{
				break;
			}
			pos += written;
			len -= written;
			first = FALSE;
		}
	}
	else
	{
		snprintf(buf, BUF_LEN, "%H", host);
	}

	xmlNewProp(node, "value", buf);
	xmlNewProp(node, "type", host->get_family(host) == AF_INET ? "IPv4" : "IPv6");

	return node;
}

/**
 * Create metadata
 */
static xmlNodePtr create_metadata(private_tnc_ifmap_soap_t *this,
								  xmlChar *metadata)
{
	xmlNodePtr node, node2;

	node = xmlNewNode(NULL, "metadata");
	node2 = xmlNewNode(this->ns_meta, metadata);
	xmlAddChild(node, node2);
	xmlNewProp(node2, "ifmap-cardinality", "singleValue");

	return node;
}

/**
 * Create capability metadata
 */
static xmlNodePtr create_capability(private_tnc_ifmap_soap_t *this,
									identification_t *name)
{
	xmlNodePtr node, node2;
	char buf[BUF_LEN];

	node = xmlNewNode(this->ns_meta, "capability");
	xmlNewProp(node, "ifmap-cardinality", "multiValue");

	node2 = xmlNewNode(NULL, "name");
	xmlAddChild(node, node2);
	snprintf(buf, BUF_LEN, "%Y", name);
	xmlNodeAddContent(node2, buf);

	node2 = xmlNewNode(NULL, "administrative-domain");
	xmlAddChild(node, node2);
	xmlNodeAddContent(node2, "strongswan");

	return node;
}

METHOD(tnc_ifmap_soap_t, publish_ike_sa, bool,
	private_tnc_ifmap_soap_t *this, ike_sa_t *ike_sa, bool up)
{
	tnc_ifmap_soap_msg_t *soap_msg;
	xmlNodePtr request, node, node2 = NULL;
	enumerator_t *e1, *e2;
	auth_rule_t type;
	identification_t *id, *eap_id, *group;
	host_t *host;
	auth_cfg_t *auth;
	uint32_t ike_sa_id;
	bool is_user = FALSE, first = TRUE, success;

	/* extract relevant data from IKE_SA*/
	ike_sa_id = ike_sa->get_unique_id(ike_sa);
	host = ike_sa->get_other_host(ike_sa);
	id = ike_sa->get_other_id(ike_sa);
	eap_id = ike_sa->get_other_eap_id(ike_sa);

	/* in the presence of an EAP Identity, treat it as a username */
	if (!id->equals(id, eap_id))
	{
		is_user = TRUE;
		id = eap_id;
	}

	/* build publish request */
	request = create_publish_request(this);

	/* delete any existing enforcement reports */
	if (up)
	{
		node = create_delete_filter(this, "enforcement-report");
		xmlAddChild(request, node);
		xmlAddChild(node, create_ip_address(this, host));
		xmlAddChild(node, create_device(this));
	}

	/**
	 * update or delete authenticated-as metadata
	 */
	if (up)
	{
		node = xmlNewNode(NULL, "update");
	}
	else
	{
		node = create_delete_filter(this, "authenticated-as");
	}
	xmlAddChild(request, node);

	/* add access-request, identity and [if up] metadata */
	xmlAddChild(node, create_access_request(this, ike_sa_id));
	xmlAddChild(node, create_identity(this, id, is_user));
	if (up)
	{
		xmlAddChild(node, create_metadata(this, "authenticated-as"));
	}

	/**
	 * update or delete access-request-ip metadata for physical IP address
	 */
	if (up)
	{
		node = xmlNewNode(NULL, "update");
	}
	else
	{
		node = create_delete_filter(this, "access-request-ip");
	}
	xmlAddChild(request, node);

	/* add access-request, ip-address and [if up] metadata */
	xmlAddChild(node, create_access_request(this, ike_sa_id));
	xmlAddChild(node, create_ip_address(this, host));
	if (up)
	{
		xmlAddChild(node, create_metadata(this, "access-request-ip"));
	}

	/**
	 * update or delete authenticated-by metadata
	 */
	if (up)
	{
		node = xmlNewNode(NULL, "update");
	}
	else
	{
		node = create_delete_filter(this, "authenticated-by");
	}
	xmlAddChild(request, node);

	/* add access-request, device and [if up] metadata */
	xmlAddChild(node, create_access_request(this, ike_sa_id));
	xmlAddChild(node, create_device(this));
	if (up)
	{
		xmlAddChild(node, create_metadata(this, "authenticated-by"));
	}

	/**
	 * update or delete capability metadata
	 */
	e1 = ike_sa->create_auth_cfg_enumerator(ike_sa, FALSE);
	while (e1->enumerate(e1, &auth) && (first || up))
	{
		e2 = auth->create_enumerator(auth);
		while (e2->enumerate(e2, &type, &group))
		{
			/* look for group memberships */
			if (type == AUTH_RULE_GROUP)
			{
				if (first)
				{
					first = FALSE;

					if (up)
					{
						node = xmlNewNode(NULL, "update");
					}
					else
					{
						node = create_delete_filter(this, "capability");
					}
					xmlAddChild(request, node);

					/* add access-request */
					xmlAddChild(node, create_access_request(this, ike_sa_id));
					if (!up)
					{
						break;
					}
					node2 = xmlNewNode(NULL, "metadata");
					xmlAddChild(node, node2);
				}
				xmlAddChild(node2, create_capability(this, group));
			}
		}
		e2->destroy(e2);
	}
	e1->destroy(e1);

	soap_msg = tnc_ifmap_soap_msg_create(this->uri, this->user_pass, this->tls);
	success = soap_msg->post(soap_msg, request, "publishReceived", NULL);
	soap_msg->destroy(soap_msg);

	return success;
}

METHOD(tnc_ifmap_soap_t, publish_device_ip, bool,
	private_tnc_ifmap_soap_t *this, host_t *host)
{
	tnc_ifmap_soap_msg_t *soap_msg;
	xmlNodePtr request, update;
	bool success;

	/* build publish update request */
	request = create_publish_request(this);
	update = xmlNewNode(NULL, "update");
	xmlAddChild(request, update);

	/* add device, ip-address and metadata */
	xmlAddChild(update, create_device(this));
	xmlAddChild(update, create_ip_address(this, host));
	xmlAddChild(update, create_metadata(this, "device-ip"));

	soap_msg = tnc_ifmap_soap_msg_create(this->uri, this->user_pass, this->tls);
	success = soap_msg->post(soap_msg, request, "publishReceived", NULL);
	soap_msg->destroy(soap_msg);

	return success;
}

METHOD(tnc_ifmap_soap_t, publish_virtual_ips, bool,
	private_tnc_ifmap_soap_t *this, ike_sa_t *ike_sa, bool assign)
{
	tnc_ifmap_soap_msg_t *soap_msg;
	xmlNodePtr request, node;
	uint32_t ike_sa_id;
	enumerator_t *enumerator;
	host_t *vip;
	bool success;

	/* extract relevant data from IKE_SA*/
	ike_sa_id = ike_sa->get_unique_id(ike_sa);

	/* build publish request */
	request = create_publish_request(this);

	enumerator = ike_sa->create_virtual_ip_enumerator(ike_sa, FALSE);
	while (enumerator->enumerate(enumerator, &vip))
	{
		/**
		 * update or delete access-request-ip metadata for a virtual IP address
		 */
		if (assign)
		{
			node = xmlNewNode(NULL, "update");
		}
		else
		{
			node = create_delete_filter(this, "access-request-ip");
		}
		xmlAddChild(request, node);

		/* add access-request, virtual ip-address and [if assign] metadata */
			xmlAddChild(node, create_access_request(this, ike_sa_id));
			xmlAddChild(node, create_ip_address(this, vip));
			if (assign)
		{
			xmlAddChild(node, create_metadata(this, "access-request-ip"));
		}
	}
	enumerator->destroy(enumerator);

	soap_msg = tnc_ifmap_soap_msg_create(this->uri, this->user_pass, this->tls);
	success = soap_msg->post(soap_msg, request, "publishReceived", NULL);
	soap_msg->destroy(soap_msg);

	return success;
}

METHOD(tnc_ifmap_soap_t, publish_enforcement_report, bool,
	private_tnc_ifmap_soap_t *this, host_t *host, char *action, char *reason)
{
	tnc_ifmap_soap_msg_t *soap_msg;
	xmlNodePtr request, update;
	bool success;

	/* build publish update request */
	request = create_publish_request(this);
	update = xmlNewNode(NULL, "update");
	xmlAddChild(request, update);

	/* add ip-address and metadata */
	xmlAddChild(update, create_ip_address(this, host));
	xmlAddChild(update, create_device(this));
	xmlAddChild(update, create_enforcement_report(this, action, reason));

	soap_msg = tnc_ifmap_soap_msg_create(this->uri, this->user_pass, this->tls);
	success = soap_msg->post(soap_msg, request, "publishReceived", NULL);
	soap_msg->destroy(soap_msg);

	return success;
}

METHOD(tnc_ifmap_soap_t, endSession, bool,
	private_tnc_ifmap_soap_t *this)
{
	tnc_ifmap_soap_msg_t *soap_msg;
	xmlNodePtr request;
	bool success;

	/* build endSession request */
	request = xmlNewNode(NULL, "endSession");
	this->ns = xmlNewNs(request, IFMAP_NS, "ifmap");
	xmlSetNs(request, this->ns);
	xmlNewProp(request, "session-id", this->session_id);

	soap_msg = tnc_ifmap_soap_msg_create(this->uri, this->user_pass, this->tls);
	success = soap_msg->post(soap_msg, request, "endSessionResult", NULL);
	soap_msg->destroy(soap_msg);

	DBG1(DBG_TNC, "ended ifmap session '%s' as publisher '%s'",
				   this->session_id, this->ifmap_publisher_id);

	return success;
}

METHOD(tnc_ifmap_soap_t, get_session_id, char*,
	private_tnc_ifmap_soap_t *this)
{
	return this->session_id;
}

METHOD(tnc_ifmap_soap_t, orphaned, bool,
	private_tnc_ifmap_soap_t *this)
{
	return this->ref == 1;
}

METHOD(tnc_ifmap_soap_t, get_ref, tnc_ifmap_soap_t*,
	private_tnc_ifmap_soap_t *this)
{
	ref_get(&this->ref);
	return &this->public;
}

METHOD(tnc_ifmap_soap_t, destroy, void,
	private_tnc_ifmap_soap_t *this)
{
	if (ref_put(&this->ref))
	{
		if (this->session_id)
		{
			xmlFree(this->session_id);
			xmlFree(this->ifmap_publisher_id);
			free(this->device_name);
		}
		DESTROY_IF(this->tls);
		DESTROY_IF(this->host);

		if (this->fd != IFMAP_NO_FD)
		{
			close(this->fd);
		}
		lib->credmgr->remove_set(lib->credmgr, &this->creds->set);
		this->creds->destroy(this->creds);
		free(this->user_pass.ptr);
		free(this);
	}
}

static bool soap_init(private_tnc_ifmap_soap_t *this)
{
	char *server_uri, *server_str, *port_str, *uri_str;
	char *server_cert, *client_cert, *client_key, *user_pass;
	int port;
	auth_cfg_t *auth;
	certificate_t *cert;
	private_key_t *key;
	identification_t *server_id, *client_id = NULL;

	/* getting configuration parameters from strongswan.conf */
	server_uri =  lib->settings->get_str(lib->settings,
					"%s.plugins.tnc-ifmap.server_uri", IFMAP_URI, lib->ns);
	server_cert = lib->settings->get_str(lib->settings,
					"%s.plugins.tnc-ifmap.server_cert", NULL, lib->ns);
	client_cert = lib->settings->get_str(lib->settings,
					"%s.plugins.tnc-ifmap.client_cert", NULL, lib->ns);
	client_key =  lib->settings->get_str(lib->settings,
					"%s.plugins.tnc-ifmap.client_key", NULL, lib->ns);
	user_pass =   lib->settings->get_str(lib->settings,
					"%s.plugins.tnc-ifmap.username_password", NULL, lib->ns);

	/* load [self-signed] MAP server certificate */
	if (!server_cert)
	{
		DBG1(DBG_TNC, "MAP server certificate not defined");
		return FALSE;
	}
	cert = lib->creds->create(lib->creds, CRED_CERTIFICATE, CERT_X509,
							  BUILD_FROM_FILE, server_cert, BUILD_END);
	if (!cert)
	{
		DBG1(DBG_TNC, "loading MAP server certificate from '%s' failed",
					   server_cert);
		return FALSE;
	}
	DBG1(DBG_TNC, "loaded MAP server certificate from '%s'", server_cert);
	server_id = cert->get_subject(cert);
	this->creds->add_cert(this->creds, TRUE, cert);

	/* check availability of client credentials */
	if (!client_cert && !user_pass)
	{
		DBG1(DBG_TNC, "neither MAP client certificate "
					  "nor username:password defined");
		return FALSE;
	}

	if (client_cert)
	{
		/* load MAP client certificate */
		cert = lib->creds->create(lib->creds, CRED_CERTIFICATE, CERT_X509,
								  BUILD_FROM_FILE, client_cert, BUILD_END);
		if (!cert)
		{
			DBG1(DBG_TNC, "loading MAP client certificate from '%s' failed",
						   client_cert);
			return FALSE;
		}
		DBG1(DBG_TNC, "loaded MAP client certificate from '%s'", client_cert);
		cert = this->creds->add_cert_ref(this->creds, TRUE, cert);

		/* load MAP client private key */
		if (client_key)
		{
			key = lib->creds->create(lib->creds, CRED_PRIVATE_KEY, KEY_RSA,
									  BUILD_FROM_FILE, client_key, BUILD_END);
			if (!key)
			{
				DBG1(DBG_TNC, "loading MAP client private key from '%s' failed",
							   client_key);
				return FALSE;
			}
			DBG1(DBG_TNC, "loaded MAP client RSA private key from '%s'",
						   client_key);
			this->creds->add_key(this->creds, key);
		}

		/* set client ID to certificate distinguished name */
		client_id = cert->get_subject(cert);

		/* check if we have a private key matching the certificate */
		auth = auth_cfg_create();
		auth->add(auth, AUTH_RULE_SUBJECT_CERT, cert);
		key = lib->credmgr->get_private(lib->credmgr, KEY_RSA, client_id, auth);
		auth->destroy(auth);
		if (!key)
		{
			DBG1(DBG_TNC, "no RSA private key matching MAP client certificate");
			return FALSE;
		}
	}
	else
	{
		/* set base64-encoded username:password for HTTP Basic Authentication */
		this->user_pass = chunk_to_base64(chunk_from_str(user_pass), NULL);
	}

	/* remove HTTPS prefix if any */
	if (strlen(server_uri) >= 8 && strncaseeq(server_uri, "https://", 8))
	{
		server_uri += 8;
	}
	this->uri = server_uri;

	/* duplicate server string since we are going to manipulate it */
	server_str = strdup(server_uri);

	/* extract server name and port from server URI */
	port_str = strchr(server_str, ':');
	if (port_str)
	{
		*port_str++ = '\0';
		if (sscanf(port_str, "%d", &port) != 1)
		{
			DBG1(DBG_TNC, "parsing server port %s failed", port_str);
			free(server_str);
			return FALSE;
		}
	}
	else
	{
		/* use default https port */
		port = 443;
		uri_str = strchr(server_str, '/');
		if (uri_str)
		{
			*uri_str = '\0';
		}
	}

	/* open TCP socket and connect to MAP server */
	this->host = host_create_from_dns(server_str, 0, port);
	if (!this->host)
	{
		DBG1(DBG_TNC, "resolving hostname %s failed", server_str);
		free(server_str);
		return FALSE;
	}
	free(server_str);

	this->fd = socket(this->host->get_family(this->host), SOCK_STREAM, 0);
	if (this->fd == IFMAP_NO_FD)
	{
		DBG1(DBG_TNC, "opening socket failed: %s", strerror(errno));
		return FALSE;
	}

	if (connect(this->fd, this->host->get_sockaddr(this->host),
						 *this->host->get_sockaddr_len(this->host)) == -1)
	{
		DBG1(DBG_TNC, "connecting to %#H failed: %s",
					   this->host, strerror(errno));
		return FALSE;
	}

	/* open TLS socket */
	this->tls = tls_socket_create(FALSE, server_id, client_id, this->fd,
								  NULL, TLS_1_2, FALSE);
	if (!this->tls)
	{
		DBG1(DBG_TNC, "creating TLS socket failed");
		return FALSE;
	}

	return TRUE;
}

/**
 * See header
 */
tnc_ifmap_soap_t *tnc_ifmap_soap_create()
{
	private_tnc_ifmap_soap_t *this;

	INIT(this,
		.public = {
			.newSession = _newSession,
			.renewSession = _renewSession,
			.purgePublisher = _purgePublisher,
			.publish_ike_sa = _publish_ike_sa,
			.publish_device_ip = _publish_device_ip,
			.publish_virtual_ips = _publish_virtual_ips,
			.publish_enforcement_report = _publish_enforcement_report,
			.endSession = _endSession,
			.get_session_id = _get_session_id,
			.orphaned = _orphaned,
			.get_ref = _get_ref,
			.destroy = _destroy,
		},
		.fd = IFMAP_NO_FD,
		.creds = mem_cred_create(),
		.ref = 1,
	);

	lib->credmgr->add_set(lib->credmgr, &this->creds->set);

	if (!soap_init(this))
	{
		destroy(this);
		return NULL;
	}

	return &this->public;
}
