/*
 * Copyright (C) 2012 Martin Willi
 * Copyright (C) 2012 revosec AG
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

#include "xauth_pam.h"

#include <daemon.h>
#include <library.h>

#include <security/pam_appl.h>

typedef struct private_xauth_pam_t private_xauth_pam_t;

/**
 * Private data of an xauth_pam_t object.
 */
struct private_xauth_pam_t {

	/**
	 * Public interface.
	 */
	xauth_pam_t public;

	/**
	 * ID of the peer
	 */
	identification_t *peer;
};

METHOD(xauth_method_t, initiate, status_t,
	private_xauth_pam_t *this, cp_payload_t **out)
{
	cp_payload_t *cp;

	cp = cp_payload_create_type(PLV1_CONFIGURATION, CFG_REQUEST);
	cp->add_attribute(cp, configuration_attribute_create_chunk(
				PLV1_CONFIGURATION_ATTRIBUTE, XAUTH_USER_NAME, chunk_empty));
	cp->add_attribute(cp, configuration_attribute_create_chunk(
				PLV1_CONFIGURATION_ATTRIBUTE, XAUTH_USER_PASSWORD, chunk_empty));
	*out = cp;
	return NEED_MORE;
}

/**
 * PAM conv callback function
 */
static int auth_conv(int num_msg, const struct pam_message **msg,
					 struct pam_response **resp, char *password)
{
	struct pam_response *response;

	if (num_msg != 1)
	{
		return PAM_CONV_ERR;
	}
	response = malloc(sizeof(struct pam_response));
	response->resp = strdup(password);
	response->resp_retcode = 0;
	*resp = response;
	return PAM_SUCCESS;
}

/**
 * Authenticate a username/password using PAM
 */
static bool authenticate(char *service, char *user, char *password)
{
	pam_handle_t *pamh = NULL;
	static struct pam_conv conv;
	int ret;

	conv.conv = (void*)auth_conv;
	conv.appdata_ptr = password;

	ret = pam_start(service, user, &conv, &pamh);
	if (ret != PAM_SUCCESS)
	{
		DBG1(DBG_IKE, "XAuth pam_start for '%s' failed: %s",
			 user, pam_strerror(pamh, ret));
		return FALSE;
	}
	ret = pam_authenticate(pamh, 0);
	if (ret == PAM_SUCCESS)
	{
		ret = pam_acct_mgmt(pamh, 0);
		if (ret != PAM_SUCCESS)
		{
			DBG1(DBG_IKE, "XAuth pam_acct_mgmt for '%s' failed: %s",
				 user, pam_strerror(pamh, ret));
		}
	}
	else
	{
		DBG1(DBG_IKE, "XAuth pam_authenticate for '%s' failed: %s",
			 user, pam_strerror(pamh, ret));
	}
	pam_end(pamh, ret);
	return ret == PAM_SUCCESS;
}

/**
 * Convert configuration attribute content to a null-terminated string
 */
static void attr2string(char *buf, size_t len, chunk_t chunk)
{
	if (chunk.len && chunk.len < len)
	{
		chunk_t sane;

		chunk_printable(chunk, &sane, '?');
		snprintf(buf, len, "%.*s", (int)sane.len, sane.ptr);
		chunk_clear(&sane);
	}
}

METHOD(xauth_method_t, process, status_t,
	private_xauth_pam_t *this, cp_payload_t *in, cp_payload_t **out)
{
	char *service, user[128] = "", pass[128] = "", *pos;
	configuration_attribute_t *attr;
	enumerator_t *enumerator;
	chunk_t chunk;

	enumerator = in->create_attribute_enumerator(in);
	while (enumerator->enumerate(enumerator, &attr))
	{
		switch (attr->get_type(attr))
		{
			case XAUTH_USER_NAME:
				chunk = attr->get_chunk(attr);
				/* trim to username part if email address given */
				if (lib->settings->get_bool(lib->settings,
											"%s.plugins.xauth-pam.trim_email",
											TRUE, lib->ns))
				{
					pos = memchr(chunk.ptr, '@', chunk.len);
					if (pos)
					{
						chunk.len = (u_char*)pos - chunk.ptr;
					}
				}
				attr2string(user, sizeof(user), chunk);
				break;
			case XAUTH_USER_PASSWORD:
				chunk = attr->get_chunk(attr);
				if (chunk.len && chunk.ptr[chunk.len - 1] == 0)
				{	/* fix null-terminated passwords (Android etc.) */
					chunk.len -= 1;
				}
				attr2string(pass, sizeof(pass), chunk);
				break;
			default:
				break;
		}
	}
	enumerator->destroy(enumerator);

	if (!user[0] || !pass[0])
	{
		DBG1(DBG_IKE, "peer did not respond to our XAuth request");
		return FAILED;
	}

	this->peer->destroy(this->peer);
	this->peer = identification_create_from_string(user);

	/* Look for PAM service, with a legacy fallback for the eap-gtc plugin.
	 * Default to "login". */
	service = lib->settings->get_str(lib->settings,
				"%s.plugins.xauth-pam.pam_service",
					lib->settings->get_str(lib->settings,
						"%s.plugins.eap-gtc.pam_service", "login", lib->ns),
				lib->ns);

	if (authenticate(service, user, pass))
	{
		DBG1(DBG_IKE, "PAM authentication of '%s' successful", user);
		return SUCCESS;
	}
	return FAILED;
}

METHOD(xauth_method_t, get_identity, identification_t*,
	private_xauth_pam_t *this)
{
	return this->peer;
}

METHOD(xauth_method_t, destroy, void,
	private_xauth_pam_t *this)
{
	this->peer->destroy(this->peer);
	free(this);
}

/*
 * Described in header.
 */
xauth_pam_t *xauth_pam_create_server(identification_t *server,
									 identification_t *peer, char *profile)
{
	private_xauth_pam_t *this;

	INIT(this,
		.public = {
			.xauth_method = {
				.initiate = _initiate,
				.process = _process,
				.get_identity = _get_identity,
				.destroy = _destroy,
			},
		},
		.peer = peer->clone(peer),
	);

	return &this->public;
}
