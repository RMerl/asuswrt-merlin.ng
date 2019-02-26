/*
 * Copyright (C) 2013 Endian srl
 * Author: Andrea Bonomi - <a.bonomi@endian.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#define _GNU_SOURCE
#include <stdio.h>

#include "xauth_pam_listener.h"

#include <daemon.h>
#include <library.h>

#include <security/pam_appl.h>

typedef struct private_xauth_pam_listener_t private_xauth_pam_listener_t;

/**
 * Private data of an xauth_pam_listener_t object.
 */
struct private_xauth_pam_listener_t {

	/**
	 * Public xauth_pam_listener_t interface.
	 */
	xauth_pam_listener_t public;

	/**
	 * PAM service
	 */
	char *service;
};

/**
 * PAM conv callback function
 */
static int conv(int num_msg, const struct pam_message **msg,
				struct pam_response **resp, void *data)
{
	int i;

	for (i = 0; i < num_msg; i++)
	{
		/* ignore any text info, but fail on any interaction request */
		if (msg[i]->msg_style != PAM_TEXT_INFO)
		{
			return PAM_CONV_ERR;
		}
	}
	return PAM_SUCCESS;
}

METHOD(listener_t, ike_updown, bool,
	private_xauth_pam_listener_t *this, ike_sa_t *ike_sa, bool up)
{
	struct pam_conv null_conv = {
		.conv = conv,
	};
	pam_handle_t *pamh = NULL;
	char *user;
	int ret;

	if (asprintf(&user, "%Y", ike_sa->get_other_eap_id(ike_sa)) != -1)
	{
		ret = pam_start(this->service, user, &null_conv, &pamh);
		if (ret == PAM_SUCCESS)
		{
			if (up)
			{
				ret = pam_open_session(pamh, 0);
				if (ret != PAM_SUCCESS)
				{
					DBG1(DBG_IKE, "XAuth pam_open_session for '%s' failed: %s",
						 user, pam_strerror(pamh, ret));
				}
			}
			else
			{
				ret = pam_close_session(pamh, 0);
				if (ret != PAM_SUCCESS)
				{
					DBG1(DBG_IKE, "XAuth pam_close_session for '%s' failed: %s",
						 user, pam_strerror(pamh, ret));
				}
			}
		}
		else
		{
			DBG1(DBG_IKE, "XAuth pam_start for '%s' failed: %s",
				 user, pam_strerror(pamh, ret));
		}
		pam_end(pamh, ret);
		free(user);
	}
	return TRUE;
}

METHOD(xauth_pam_listener_t, listener_destroy, void,
	private_xauth_pam_listener_t *this)
{
	free(this);
}

xauth_pam_listener_t *xauth_pam_listener_create()
{
	private_xauth_pam_listener_t *this;

	INIT(this,
		.public = {
			.listener = {
				.ike_updown = _ike_updown,
			},
			.destroy = _listener_destroy,
		},
		/* Look for PAM service, with a legacy fallback for the eap-gtc plugin.
		 * Default to "login". */
		.service = lib->settings->get_str(lib->settings,
						"%s.plugins.xauth-pam.pam_service",
							lib->settings->get_str(lib->settings,
								"%s.plugins.eap-gtc.pam_service",
							"login", lib->ns),
						lib->ns),
	);

	return &this->public;
}
