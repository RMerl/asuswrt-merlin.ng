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
 * @file rlm_otp.c
 * @brief One time password implementation.
 *
 * @copyright 2013 Network RADIUS SARL
 * @copyright 2000,2001,2002,2013  The FreeRADIUS server project
 * @copyright 2005-2007 TRI-D Systems, Inc.
 * @copyright 2001,2002  Google, Inc.
 */
RCSID("$Id$")

#include <freeradius-devel/radiusd.h>
#include <freeradius-devel/modules.h>

#include "extern.h"
#include "otp.h"

/* Global data */
static int ninstance = 0;	//!< Number of instances, for global init.

/* A mapping of configuration file names to internal variables. */
static const CONF_PARSER module_config[] = {
	{ "otpd_rp", PW_TYPE_STRING_PTR, offsetof(rlm_otp_t, otpd_rp),
	  NULL, OTP_OTPD_RP },
	{ "challenge_prompt", PW_TYPE_STRING_PTR,offsetof(rlm_otp_t, chal_prompt),
	  NULL, OTP_CHALLENGE_PROMPT },
	{ "challenge_length", PW_TYPE_INTEGER, offsetof(rlm_otp_t, challenge_len),
	  NULL, "6" },
	{ "challenge_delay", PW_TYPE_INTEGER, offsetof(rlm_otp_t, challenge_delay),
	  NULL, "30" },
	{ "allow_sync", PW_TYPE_BOOLEAN, offsetof(rlm_otp_t, allow_sync),
	  NULL, "yes" },
	{ "allow_async", PW_TYPE_BOOLEAN, offsetof(rlm_otp_t, allow_async),
	  NULL, "no" },

	{ "mschapv2_mppe", PW_TYPE_INTEGER,
	  offsetof(rlm_otp_t, mschapv2_mppe_policy), NULL, "2" },
	{ "mschapv2_mppe_bits", PW_TYPE_INTEGER,
	  offsetof(rlm_otp_t, mschapv2_mppe_types), NULL, "2" },
	{ "mschap_mppe", PW_TYPE_INTEGER,
	  offsetof(rlm_otp_t, mschap_mppe_policy), NULL, "2" },
	{ "mschap_mppe_bits", PW_TYPE_INTEGER,
	  offsetof(rlm_otp_t, mschap_mppe_types), NULL, "2" },

	{ NULL, -1, 0, NULL, NULL }		/* end the list */
};


/*
 *	Per-instance initialization
 */
static int mod_instantiate(CONF_SECTION *conf, void *instance)
{
	rlm_otp_t *inst = instance;

	/* Onetime initialization. */
	if (!ninstance) {
		/* Generate a random key, used to protect the State attribute. */
		otp_get_random(inst->hmac_key, sizeof(inst->hmac_key));

		/* Initialize the passcode encoding/checking functions. */
		otp_pwe_init();

		/*
		 * Don't do this again.
		 * Only the main thread instantiates and detaches instances,
		 * so this does not need mutex protection.
		 */
		ninstance++;
	}

	/* Verify ranges for those vars that are limited. */
	if ((inst->challenge_len < 5) ||
	    (inst->challenge_len > OTP_MAX_CHALLENGE_LEN)) {
		inst->challenge_len = 6;

		WDEBUG("invalid challenge_length %d, "
		       "range 5-%d, using default of 6",
		       inst->challenge_len, OTP_MAX_CHALLENGE_LEN);
	}

	if (!inst->allow_sync && !inst->allow_async) {
		cf_log_err_cs(conf, "at least one of {allow_async, "
			      "allow_sync} must be set");
		return -1;
	}

	if ((inst->mschapv2_mppe_policy > 2) ||
	    (inst->mschapv2_mppe_policy < 0)) {
		inst->mschapv2_mppe_policy = 2;
		WDEBUG("Invalid value for mschapv2_mppe, "
			"using default of 2");
	}

	if ((inst->mschapv2_mppe_types > 2) || (inst->mschapv2_mppe_types < 0)) {
		inst->mschapv2_mppe_types = 2;
		WDEBUG("Invalid value for "
		       "mschapv2_mppe_bits, using default of 2");
	}

	if ((inst->mschap_mppe_policy > 2) || (inst->mschap_mppe_policy < 0)) {
		inst->mschap_mppe_policy = 2;
		WDEBUG("Invalid value for mschap_mppe, "
		       "using default of 2");
  	}

	if (inst->mschap_mppe_types != 2) {
		inst->mschap_mppe_types = 2;
		WDEBUG("Invalid value for "
		       "mschap_mppe_bits, using default of 2");
	}

	/* set the instance name (for use with authorize()) */
	inst->name = cf_section_name2(conf);
	if (!inst->name) inst->name = cf_section_name1(conf);

	return 0;
}

/*
 *	Generate a challenge to be presented to the user.
 */
static rlm_rcode_t mod_authorize(void *instance, REQUEST *request)
{
	rlm_otp_t *inst = (rlm_otp_t *) instance;

	char challenge[OTP_MAX_CHALLENGE_LEN + 1];	/* +1 for '\0' terminator */
	int auth_type_found;

	/* Early exit if Auth-Type != inst->name */
	{
		VALUE_PAIR *vp;

		auth_type_found = 0;
		vp = pairfind(request->config_items, PW_AUTHTYPE, 0, TAG_ANY);
		if (vp) {
      			auth_type_found = 1;
      			if (strcmp(vp->vp_strvalue, inst->name)) {
				return RLM_MODULE_NOOP;
    			}
  		}
  	}

	/* The State attribute will be present if this is a response. */
	if (pairfind(request->packet->vps, PW_STATE, 0, TAG_ANY) != NULL) {
		DEBUG("rlm_otp: autz: Found response to Access-Challenge");

		return RLM_MODULE_OK;
	}

	/* User-Name attribute required. */
	if (!request->username) {
		RWDEBUG("Attribute \"User-Name\" "
		       "required for authentication.");

		return RLM_MODULE_INVALID;
	}

	if (otp_pwe_present(request) == 0) {
		RWDEBUG("Attribute "
			"\"User-Password\" or equivalent required "
			"for authentication.");

		return RLM_MODULE_INVALID;
	}

	/*
	 * 	We used to check for special "challenge" and "resync" passcodes
	 * 	here, but these are complicated to explain and application is
	 * 	limited.  More importantly, since we've removed all actual OTP
	 * 	code (now we ask otpd), it's awkward for us to support them.
	 * 	Should the need arise to reinstate these options, the most
	 *	likely choice is to duplicate some otpd code here.
	 */
	if (inst->allow_sync && !inst->allow_async) {
		/* This is the token sync response. */
		if (!auth_type_found) {
			pairmake_config("Auth-Type", inst->name, T_OP_EQ);
		}

		return RLM_MODULE_OK;
	}

	/*
	 *	Generate a random challenge.
	 */
	otp_async_challenge(challenge, inst->challenge_len);

	/*
	 *	Create the State attribute, which will be returned to
	 *	us along with the response.
	 *
	 *	We will need this to verify the response.
	 *
	 *	It must be hmac protected to prevent insertion of arbitrary
	 *	State by an inside attacker.
	 *
	 *	If we won't actually use the State (server config doesn't
	 *	allow async), we just use a trivial State.
	 *
	 *	We always create at least a trivial State, so mod_authorize()
	 *	can quickly pass on to mod_authenticate().
	 */
	{
		int32_t now = htonl(time(NULL)); //!< Low-order 32 bits on LP64.

		char gen_state[OTP_MAX_RADSTATE_LEN];
		size_t len;
		VALUE_PAIR *vp;

		len = otp_gen_state(gen_state, challenge, inst->challenge_len,
				    0, now, inst->hmac_key);

		vp = paircreate(request->reply, PW_STATE, 0);
		if (!vp) {
			return RLM_MODULE_FAIL;
		}

		pairmemcpy(vp, (uint8_t const *) gen_state, len);
		pairadd(&request->reply->vps, vp);
	}

	/*
	 *	Add the challenge to the reply.
	 */
	{
		VALUE_PAIR *vp;

		char *expanded = NULL;
		ssize_t len;

		/*
		 *	First add the internal OTP challenge attribute to
		 *	the reply list.
		 */
		vp = paircreate(request->reply, PW_OTP_CHALLENGE, 0);
		if (!vp) {
			return RLM_MODULE_FAIL;
		}

		pairstrcpy(vp, challenge);
		vp->op = T_OP_SET;

		pairadd(&request->reply->vps, vp);

		/*
		 *	Then add the message to the user to they known
		 *	what the challenge value is.
		 */

		len = radius_axlat(&expanded, request, inst->chal_prompt, NULL, NULL);
		if (len < 0) {
			return RLM_MODULE_FAIL;
		}

		vp = paircreate(request->reply, PW_REPLY_MESSAGE, 0);
		if (!vp) {
			talloc_free(expanded);
			return RLM_MODULE_FAIL;
		}

		(void) talloc_steal(vp, expanded);
		vp->vp_strvalue = expanded;
		vp->length = len;
		vp->op = T_OP_SET;
		vp->type = VT_DATA;

		pairadd(&request->reply->vps, vp);
	}

	/*
	 *	Mark the packet as an Access-Challenge packet.
	 * 	The server will take care of sending it to the user.
	 */
	request->reply->code = PW_ACCESS_CHALLENGE;

	DEBUG("rlm_otp: Sending Access-Challenge.");

	if (!auth_type_found) {
		pairmake_config("Auth-Type", inst->name, T_OP_EQ);
	}

	return RLM_MODULE_HANDLED;
}


/*
 *	Verify the response entered by the user.
 */
static rlm_rcode_t mod_authenticate(void *instance, REQUEST *request)
{
	rlm_otp_t *inst = instance;

	char const *username;
	int rc;
	otp_pwe_t pwe;
	VALUE_PAIR *vp;

	char challenge[OTP_MAX_CHALLENGE_LEN];	/* cf. authorize() */
	char passcode[OTP_MAX_PASSCODE_LEN + 1];

	challenge[0] = '\0';	/* initialize for otp_pw_valid() */

	/* User-Name attribute required. */
	if (!request->username) {
		RWDEBUG("Attribute \"User-Name\" required "
			"for authentication.");

		return RLM_MODULE_INVALID;
	}

	username = request->username->vp_strvalue;

	pwe = otp_pwe_present(request);
	if (pwe == 0) {
		RWDEBUG("Attribute \"User-Password\" "
			"or equivalent required for authentication.");

		return RLM_MODULE_INVALID;
	}

	/*
	 *	Retrieve the challenge (from State attribute).
	 */
	vp = pairfind(request->packet->vps, PW_STATE, 0, TAG_ANY);
	if (vp) {
		char	gen_state[OTP_MAX_RADSTATE_LEN]; //!< State as hexits
		uint8_t	bin_state[OTP_MAX_RADSTATE_LEN];

		int32_t	then;		//!< State timestamp.
		size_t	elen;		//!< Expected State length.
		size_t	len;

		/*
		 *	Set expected State length (see otp_gen_state())
		 */
		elen = (inst->challenge_len * 2) + 8 + 8 + 32;

		if (vp->length != elen) {
			REDEBUG("Bad radstate for [%s]: length", username);
			return RLM_MODULE_INVALID;
		}

		/*
		 *	Verify the state.
		 */

		/*
		 *	Convert vp state (ASCII encoded hexits in opaque bin
		 *	string) back to binary.
		 *
		 *	There are notes in otp_radstate as to why the state
		 *	value is encoded as hexits.
		 */
		len = fr_hex2bin(bin_state, vp->vp_strvalue, vp->length);
		if (len != (vp->length / 2)) {
			REDEBUG("bad radstate for [%s]: not hex", username);

			return RLM_MODULE_INVALID;
		}

		/*
		 *	Extract data from State
		 */
		memcpy(challenge, bin_state, inst->challenge_len);

		/*
		 *	Skip flag data
		 */
		memcpy(&then, bin_state + inst->challenge_len + 4, 4);

		/*
		 *	Generate new state from returned input data
		 */
		otp_gen_state(gen_state, challenge, inst->challenge_len, 0,
			      then, inst->hmac_key);

		/*
		 *	Compare generated state (in hex form)
		 *	against generated state (in hex form)
		 *	to verify hmac.
		 */
		if (memcmp(gen_state, vp->vp_octets, vp->length)) {
			REDEBUG("bad radstate for [%s]: hmac", username);

			return RLM_MODULE_REJECT;
		}

		/*
		 *	State is valid, but check expiry.
		 */
		then = ntohl(then);
		if (time(NULL) - then > inst->challenge_delay) {
			REDEBUG("bad radstate for [%s]: expired",username);

			return RLM_MODULE_REJECT;
		}
	}

	/* do it */
	rc = otp_pw_valid(request, pwe, challenge, inst, passcode);

	/* Add MPPE data as needed. */
	if (rc == RLM_MODULE_OK) {
		otp_mppe(request, pwe, inst, passcode);
	}

	return rc;
}

/*
 *	If the module needs to temporarily modify it's instantiation
 *	data, the type should be changed to RLM_TYPE_THREAD_UNSAFE.
 *	The server will then take care of ensuring that the module
 *	is single-threaded.
 */
module_t rlm_otp = {
	RLM_MODULE_INIT,
	"otp",
	RLM_TYPE_THREAD_SAFE,		/* type */
	sizeof(rlm_otp_t),
	module_config,
	mod_instantiate,		/* instantiation */
	NULL,				/* detach */
	{
		mod_authenticate,	/* authentication */
		mod_authorize,		/* authorization */
		NULL,			/* preaccounting */
		NULL,			/* accounting */
		NULL,			/* checksimul */
		NULL,			/* pre-proxy */
		NULL,			/* post-proxy */
		NULL			/* post-auth */
	},
};
