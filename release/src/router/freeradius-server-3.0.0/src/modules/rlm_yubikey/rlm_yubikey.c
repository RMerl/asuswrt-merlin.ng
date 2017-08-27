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
 * @file rlm_yubikey.c
 * @brief Authentication for yubikey OTP tokens.
 *
 * @author Arran Cudbard-Bell <a.cudbardb@networkradius.com>
 * @copyright 2013 The FreeRADIUS server project
 * @copyright 2013 Network RADIUS <info@networkradius.com>
 */
RCSID("$Id$")

#include "rlm_yubikey.h"

/*
 *	A mapping of configuration file names to internal variables.
 *
 *	Note that the string is dynamically allocated, so it MUST
 *	be freed.  When the configuration file parse re-reads the string,
 *	it free's the old one, and strdup's the new one, placing the pointer
 *	to the strdup'd string into 'config.string'.  This gets around
 *	buffer over-flows.
 */

#ifdef HAVE_YKCLIENT
static const CONF_PARSER validation_config[] = {
	{ "client_id", PW_TYPE_INTEGER, offsetof(rlm_yubikey_t, client_id), NULL, 0},
	{ "api_key", PW_TYPE_STRING_PTR, offsetof(rlm_yubikey_t, api_key), NULL, NULL},
	{ NULL, -1, 0, NULL, NULL }		/* end the list */
};
#endif

static const CONF_PARSER module_config[] = {
	{ "id_length", PW_TYPE_INTEGER, offsetof(rlm_yubikey_t, id_len), NULL, "12" },
	{ "decrypt", PW_TYPE_BOOLEAN, offsetof(rlm_yubikey_t, decrypt), NULL, "no" },
	{ "validate", PW_TYPE_BOOLEAN, offsetof(rlm_yubikey_t, validate), NULL, "no" },
#ifdef HAVE_YKCLIENT
	{ "validation", PW_TYPE_SUBSECTION, 0, NULL, (void const *) validation_config },
#endif
	{ NULL, -1, 0, NULL, NULL }		/* end the list */
};

static char const *modhextab = "cbdefghijklnrtuv";
static char const *hextab = "0123456789abcdef";

#define is_modhex(x) (memchr(modhextab, tolower(x), 16))

/** Convert yubikey modhex to normal hex
 *
 * The same buffer may be passed as modhex and hex to convert the modhex in place.
 *
 * Modhex and hex must be the same size.
 *
 * @param[in] modhex data.
 * @param[in] len of input and output buffers.
 * @param[out] hex where to write the standard hexits.
 * @return The number of bytes written to the output buffer, or -1 on error.
 */
static ssize_t modhex2hex(char const *modhex, uint8_t *hex, size_t len)
{
	size_t i;
	char *c1, *c2;

	for (i = 0; i < len; i++) {
		if (modhex[i << 1] == '\0') {
			break;
		}

		/*
		 *	We only deal with whole bytes
		 */
		if (modhex[(i << 1) + 1] == '\0')
			return -1;

		if (!(c1 = memchr(modhextab, tolower((int) modhex[i << 1]), 16)) ||
		    !(c2 = memchr(modhextab, tolower((int) modhex[(i << 1) + 1]), 16)))
			return -1;

		hex[i] = hextab[c1 - modhextab];
		hex[i + 1] = hextab[c2 - modhextab];
	}

	return i;
}

/**
 * @brief Convert Yubikey modhex to standard hex
 *
 * Example: "%{modhextohex:vvrbuctetdhc}" == "ffc1e0d3d260"
 */
static ssize_t modhex_to_hex_xlat(UNUSED void *instance, REQUEST *request, char const *fmt, char *out, size_t outlen)
{
	ssize_t len;

	if (outlen < strlen(fmt)) {
		*out = '\0';
		return 0;
	}

	/*
	 *	mod2hex allows conversions in place
	 */
	len = modhex2hex(fmt, (uint8_t *) out, strlen(fmt));
	if (len <= 0) {
		*out = '\0';
		REDEBUG("Modhex string invalid");

		return -1;
	}

	return len;
}

/*
 *	Do any per-module initialization that is separate to each
 *	configured instance of the module.  e.g. set up connections
 *	to external databases, read configuration files, set up
 *	dictionary entries, etc.
 *
 *	If configuration information is given in the config section
 *	that must be referenced in later calls, store a handle to it
 *	in *instance otherwise put a null pointer there.
 */
static int mod_instantiate(CONF_SECTION *conf, void *instance)
{
	rlm_yubikey_t *inst = instance;

	inst->name = cf_section_name2(conf);
	if (!inst->name) {
		inst->name = cf_section_name1(conf);
	}

#ifndef HAVE_YUBIKEY
	if (inst->decrypt) {
		EDEBUG("rlm_yubikey (%s): Requires libyubikey for OTP decryption", inst->name);
		return -1;
	}
#endif

	if (inst->validate) {
#ifdef HAVE_YKCLIENT
		CONF_SECTION *cs;

		cs = cf_section_sub_find(conf, "validation");
		if (!cs) {
			EDEBUG("rlm_yubikey (%s): Missing validation section", inst->name);
			return -1;
		}

		if (rlm_yubikey_ykclient_init(cs, inst) < 0) {
			return -1;
		}
#else
		EDEBUG("rlm_yubikey (%s): Requires libykclient for OTP validation against Yubicloud servers", inst->name);
		return -1;
#endif
	}

	xlat_register("modhextohex", modhex_to_hex_xlat, NULL, inst);

	return 0;
}

/*
 *	Only free memory we allocated.  The strings allocated via
 *	cf_section_parse() do not need to be freed.
 */
#ifdef HAVE_YKCLIENT
static int mod_detach(void *instance)
{
	rlm_yubikey_ykclient_detach((rlm_yubikey_t *) instance);
	return 0;
}
#endif

/*
 *	Find the named user in this modules database.  Create the set
 *	of attribute-value pairs to check and reply with for this user
 *	from the database. The authentication code only needs to check
 *	the password, the rest is done here.
 */
static rlm_rcode_t mod_authorize(void *instance, REQUEST *request)
{
	rlm_yubikey_t *inst = instance;

	DICT_VALUE *dval;
	char const *passcode;
	size_t i, len;
	VALUE_PAIR *vp;

	/*
	 *	Can't do yubikey auth if there's no password.
	 */
	if (!request->password || (request->password->da->attr != PW_USER_PASSWORD)) {
		/*
		 *	Don't print out debugging messages if we know
		 *	they're useless.
		 */
		if (request->packet->code == PW_ACCESS_CHALLENGE) {
			return RLM_MODULE_NOOP;
		}

		RDEBUG2("No Clear-Text password in the request. Can't do Yubikey authentication.");

		return RLM_MODULE_NOOP;
	}

	passcode = request->password->vp_strvalue;
	len = request->password->length;
	/*
	 *	Now see if the passcode is the correct length (in its raw
	 *	modhex encoded form).
	 *
	 *	<public_id (6-16 bytes)> + <aes-block (32 bytes)>
	 *
	 */
	if (len != (inst->id_len + 32)) {
		RDEBUG2("User-Password value is not the correct length, expected %u, got %zu", inst->id_len + 32, len);
		return RLM_MODULE_NOOP;
	}

	for (i = inst->id_len; i < len; i++) {
		if (!is_modhex(*passcode)) {
			RDEBUG2("User-Password (aes-block) value contains non modhex chars");

			return RLM_MODULE_NOOP;
		}
	}

	dval = dict_valbyname(PW_AUTH_TYPE, 0, inst->name);
	if (dval) {
		vp = radius_paircreate(request, &request->config_items, PW_AUTH_TYPE, 0);
		vp->vp_integer = dval->value;
	}

	/*
	 *	Split out the Public ID in case another module in authorize
	 *	needs to verify it's associated with the user.
	 *
	 *	It's left up to the user if they want to decode it or not.
	 *
	 *	@todo: is this even right?  Is there anything in passcode other than
	 *	the public ID?  Why pairmake(...NULL) instead of pairmake(..., passcode) ?
	 */
	if (inst->id_len) {
		vp = pairmake(request, &request->packet->vps, "Yubikey-Public-ID", NULL, T_OP_SET);
		if (!vp) {
			REDEBUG("Failed creating Yubikey-Public-ID");

			return RLM_MODULE_FAIL;
		}

		pairstrcpy(vp, passcode);
	}

	return RLM_MODULE_OK;
}

/*
 *	Authenticate the user with the given password.
 */
static rlm_rcode_t mod_authenticate(void *instance, REQUEST *request)
{
#if defined(HAVE_YUBIKEY) || defined(HAVE_YKCLIENT)
	rlm_rcode_t rcode = RLM_MODULE_NOOP;
#endif
	rlm_yubikey_t *inst = instance;
	char const *passcode;
	size_t i, len;

	/*
	 *	Can't do yubikey auth if there's no password.
	 */
	if (!request->password || (request->password->da->attr != PW_USER_PASSWORD)) {
		REDEBUG("No Clear-Text password in the request. Can't do Yubikey authentication.");
		return RLM_MODULE_INVALID;
	}

	passcode = request->password->vp_strvalue;
	len = request->password->length;
	/*
	 *	Verify the passcode is the correct length (in its raw
	 *	modhex encoded form).
	 *
	 *	<public_id (6-16 bytes)> + <aes-block (32 bytes)>
	 */
	if (len != (inst->id_len + 32)) {
		REDEBUG("User-Password value is not the correct length, expected %u, got %zu", inst->id_len + 32, len);
		return RLM_MODULE_INVALID;
	}

	for (i = inst->id_len; i < len; i++) {
		if (!is_modhex(*passcode)) {
			RDEBUG2("User-Password (aes-block) value contains non modhex chars");
			return RLM_MODULE_INVALID;
		}
	}

#ifdef HAVE_YUBIKEY
	if (inst->decrypt) {
		rcode = rlm_yubikey_decrypt(inst, request, request->password);
		if (rcode != RLM_MODULE_OK) {
			return rcode;
		}
	}
#endif

#ifdef HAVE_YKCLIENT
	if (inst->validate) {
		return rlm_yubikey_validate(inst, request, request->password);
	}
#endif
	return RLM_MODULE_NOOP;
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
module_t rlm_yubikey = {
	RLM_MODULE_INIT,
	"yubikey",
	RLM_TYPE_THREAD_SAFE,		/* type */
	sizeof(rlm_yubikey_t),
	module_config,
	mod_instantiate,		/* instantiation */
#ifdef HAVE_YKCLIENT
	mod_detach,			/* detach */
#else
	NULL,
#endif
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
