/*
 *   This program is is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License, version 2 if the
 *   License as published by the Free Software Foundation.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 */

/**
 * $Id$
 * @file rlm_chap.c
 * @brief Process chap authentication requests.
 *
 * @copyright 2001,2006  The FreeRADIUS server project
 * @copyright 2001  Kostas Kalevras <kkalev@noc.ntua.gr>
 */
RCSID("$Id$")

#include <freeradius-devel/radiusd.h>
#include <freeradius-devel/modules.h>

static rlm_rcode_t mod_authorize(UNUSED void *instance,
				  UNUSED REQUEST *request)
{
	if (!pairfind(request->packet->vps, PW_CHAP_PASSWORD, 0, TAG_ANY)) {
		return RLM_MODULE_NOOP;
	}

	if (pairfind(request->config_items, PW_AUTHTYPE, 0, TAG_ANY) != NULL) {
		RWDEBUG2("Auth-Type already set.  Not setting to CHAP");
		return RLM_MODULE_NOOP;
	}

	RDEBUG("Setting 'Auth-Type := CHAP'");
	pairmake_config("Auth-Type", "CHAP", T_OP_EQ);

	return RLM_MODULE_OK;
}


/*
 *	Find the named user in this modules database.  Create the set
 *	of attribute-value pairs to check and reply with for this user
 *	from the database. The authentication code only needs to check
 *	the password, the rest is done here.
 */
static rlm_rcode_t mod_authenticate(UNUSED void *instance,
				     UNUSED REQUEST *request)
{
	VALUE_PAIR *passwd_item, *chap;
	uint8_t pass_str[MAX_STRING_LEN];

	if (!request->username) {
		RWDEBUG("Attribute 'User-Name' is required for authentication.\n");
		return RLM_MODULE_INVALID;
	}

	chap = pairfind(request->packet->vps, PW_CHAP_PASSWORD, 0, TAG_ANY);
	if (!chap) {
		REDEBUG("You set 'Auth-Type = CHAP' for a request that does not contain a CHAP-Password attribute!");
		return RLM_MODULE_INVALID;
	}

	if (chap->length == 0) {
		REDEBUG("CHAP-Password is empty");
		return RLM_MODULE_INVALID;
	}

	if (chap->length != CHAP_VALUE_LENGTH + 1) {
		REDEBUG("CHAP-Password has invalid length");
		return RLM_MODULE_INVALID;
	}

	/*
	 *	Don't print out the CHAP password here.  It's binary crap.
	 */
	RDEBUG("Login attempt by \"%s\" with CHAP password",
		request->username->vp_strvalue);

	if ((passwd_item = pairfind(request->config_items, PW_CLEARTEXT_PASSWORD, 0, TAG_ANY)) == NULL){
		if (pairfind(request->config_items, PW_USER_PASSWORD, 0, TAG_ANY) != NULL){
			REDEBUG("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
			REDEBUG("!!! Please update your configuration so that the \"known !!!");
			REDEBUG("!!! good\" clear text password is in Cleartext-Password, !!!");
			REDEBUG("!!! and NOT in User-Password.                            !!!");
			REDEBUG("!!!						          !!!");
			REDEBUG("!!! Authentication will fail because of this.	          !!!");
			REDEBUG("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
		}

		REDEBUG("Clear-Text password is required for authentication");
		return RLM_MODULE_INVALID;
	}

	RDEBUG("Using Clear-Text password \"%s\" for user %s authentication.",
	      passwd_item->vp_strvalue, request->username->vp_strvalue);

	rad_chap_encode(request->packet,pass_str,
			chap->vp_octets[0],passwd_item);

	if (rad_digest_cmp(pass_str + 1, chap->vp_octets + 1,
			   CHAP_VALUE_LENGTH) != 0) {
		REDEBUG("Password is comparison failed: password is incorrect");
		return RLM_MODULE_REJECT;
	}

	RDEBUG("CHAP user \"%s\" authenticated successfully",
	      request->username->vp_strvalue);

	return RLM_MODULE_OK;
}

/*
 *	The module name should be the only globally exported symbol.
 *	That is, everything else should be 'static'.
 *
 *	If the module needs to temporarily modify it's instantiation
 *	data, the type should be changed to RLM_TYPE_THREAD_UNSAFE.
 *	The server will then take care of ensuring that the module
 *	is single-threaded.
 */
module_t rlm_chap = {
	 RLM_MODULE_INIT,
	"CHAP",
	RLM_TYPE_CHECK_CONFIG_SAFE,   	/* type */
	 0,
	 NULL,				/* CONF_PARSER */
	NULL,				/* instantiation */
	NULL,				/* detach */
	{
		mod_authenticate,	/* authentication */
		mod_authorize,	 	/* authorization */
		NULL,			/* preaccounting */
		NULL,			/* accounting */
		NULL,			/* checksimul */
		NULL,			/* pre-proxy */
		NULL,			/* post-proxy */
		NULL			/* post-auth */
	},
};
