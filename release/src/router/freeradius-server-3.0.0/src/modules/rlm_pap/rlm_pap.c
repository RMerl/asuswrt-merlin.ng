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
 * @file rlm_pap.c
 * @brief Hashes plaintext passwords to compare against a prehashed reference.
 *
 * @copyright 2001-2012  The FreeRADIUS server project.
 * @copyright 2012       Matthew Newton <matthew@newtoncomputing.co.uk>
 * @copyright 2001       Kostas Kalevras <kkalev@noc.ntua.gr>
 */
RCSID("$Id$")

#include <freeradius-devel/radiusd.h>
#include <freeradius-devel/modules.h>
#include <freeradius-devel/base64.h>

#include <ctype.h>

#include "../../include/md5.h"
#include "../../include/sha1.h"

/*
 *      Define a structure for our module configuration.
 *
 *      These variables do not need to be in a structure, but it's
 *      a lot cleaner to do so, and a pointer to the structure can
 *      be used as the instance handle.
 */
typedef struct rlm_pap_t {
	char const *name;	/* CONF_SECTION->name, not strdup'd */
	bool auto_header;
	int auth_type;
	bool normify;
} rlm_pap_t;

/*
 *      A mapping of configuration file names to internal variables.
 *
 *      Note that the string is dynamically allocated, so it MUST
 *      be freed.  When the configuration file parse re-reads the string,
 *      it free's the old one, and strdup's the new one, placing the pointer
 *      to the strdup'd string into 'config.string'.  This gets around
 *      buffer over-flows.
 */
static const CONF_PARSER module_config[] = {
	{ "auto_header", PW_TYPE_BOOLEAN, offsetof(rlm_pap_t,auto_header), NULL, "no" },
	{ "normalise", PW_TYPE_BOOLEAN, offsetof(rlm_pap_t,normify), NULL, "yes" },
	{ NULL, -1, 0, NULL, NULL }
};


/*
 *	For auto-header discovery.
 */
static const FR_NAME_NUMBER header_names[] = {
	{ "{clear}",		PW_CLEARTEXT_PASSWORD },
	{ "{cleartext}",	PW_CLEARTEXT_PASSWORD },
	{ "{md5}",		PW_MD5_PASSWORD },
	{ "{BASE64_MD5}",	PW_MD5_PASSWORD },
	{ "{smd5}",		PW_SMD5_PASSWORD },
	{ "{crypt}",		PW_CRYPT_PASSWORD },
	{ "{sha}",		PW_SHA_PASSWORD },
	{ "{ssha}",		PW_SSHA_PASSWORD },
	{ "{nt}",		PW_NT_PASSWORD },
	{ "{nthash}",		PW_NT_PASSWORD },
	{ "{x-nthash}",		PW_NT_PASSWORD },
	{ "{ns-mta-md5}",	PW_NS_MTA_MD5_PASSWORD },
	{ "{x- orcllmv}",	PW_LM_PASSWORD },
	{ "{X- ORCLNTV}",	PW_NT_PASSWORD },
	{ NULL, 0 }
};


static int mod_instantiate(CONF_SECTION *conf, void *instance)
{
	rlm_pap_t *inst = instance;
	DICT_VALUE *dval;

	inst->name = cf_section_name2(conf);
	if (!inst->name) {
		inst->name = cf_section_name1(conf);
	}

	dval = dict_valbyname(PW_AUTH_TYPE, 0, inst->name);
	if (dval) {
		inst->auth_type = dval->value;
	} else {
		inst->auth_type = 0;
	}

	return 0;
}

/*
 *	Hex or base64 or bin auto-discovery.
 */
static void normify(REQUEST *request, VALUE_PAIR *vp, size_t min_length)
{

	uint8_t buffer[64];

	if (min_length >= sizeof(buffer)) return; /* paranoia */

	/*
	 *	Hex encoding.
	 */
	if (vp->length >= (2 * min_length)) {
		size_t decoded;
		decoded = fr_hex2bin(buffer, vp->vp_strvalue, vp->length >> 1);
		if (decoded == (vp->length >> 1)) {
			RDEBUG2("Normalizing %s from hex encoding", vp->da->name);
			pairmemcpy(vp, buffer, decoded);
			return;
		}
	}

	/*
	 *	Base 64 encoding.  It's at least 4/3 the original size,
	 *	and we want to avoid division...
	 */
	if ((vp->length * 3) >= ((min_length * 4))) {
		ssize_t decoded;
		decoded = fr_base64_decode(vp->vp_strvalue, vp->length, buffer,
					   sizeof(buffer));
		if (decoded < 0) return;
		if (decoded >= (ssize_t) min_length) {
			RDEBUG2("Normalizing %s from base64 encoding", vp->da->name);
			pairmemcpy(vp, buffer, decoded);
			return;
		}
	}

	/*
	 *	Else unknown encoding, or already binary.  Leave it.
	 */
}


/*
 *	Authorize the user for PAP authentication.
 *
 *	This isn't strictly necessary, but it does make the
 *	server simpler to configure.
 */
static rlm_rcode_t mod_authorize(void *instance, REQUEST *request)
{
	rlm_pap_t *inst = instance;
	int auth_type = false;
	int found_pw = false;
	VALUE_PAIR *vp;
	vp_cursor_t cursor;

	for (vp = paircursor(&cursor, &request->config_items);
	     vp;
	     vp = pairnext(&cursor)) {
		switch (vp->da->attr) {
		case PW_USER_PASSWORD: /* deprecated */
			RDEBUG("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
			RDEBUG("!!! Please update your configuration so that the \"known !!!");
			RDEBUG("!!! good\" clear text password is in Cleartext-Password, !!!");
			RDEBUG("!!! and NOT in User-Password.			         !!!");
			RDEBUG("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
			break;

		case PW_PASSWORD_WITH_HEADER: /* preferred */
		{
			int attr;
			char *p;
			char const *q;
			uint8_t *b, binbuf[128];
			char charbuf[128];
			VALUE_PAIR *new_vp;

			found_pw = true;
		redo:
			q = vp->vp_strvalue;
			p = strchr(q + 1, '}');
			if (!p) {
				ssize_t decoded;

				/*
				 *	Password already exists: use
				 *	that instead of this one.
				 */
				if (pairfind(request->config_items, PW_CLEARTEXT_PASSWORD, 0, TAG_ANY)) {
					RDEBUG("Config already contains \"reference\" password.  Ignoring Password-With-Header");
					break;
				}

				/*
				 *	If it's binary, it may be
				 *	base64 encoded.  Decode it,
				 *	and re-write the attribute to
				 *	have the decoded value.
				 */
				decoded = fr_base64_decode(vp->vp_strvalue,
							   vp->length,
							   binbuf,
							   sizeof(binbuf));
				if ((decoded > 0) && (binbuf[0] == '{') &&
				     memchr(binbuf, '}', decoded)) {
					pairmemcpy(vp, binbuf, decoded);
					goto redo;
				}

				RDEBUG("Failed to decode Password-With-Header = \"%s\"", vp->vp_strvalue);
				break;
			}

			if ((size_t) (p - q) > sizeof(charbuf)) break;

			memcpy(charbuf, q, p - q + 1);
			charbuf[p - q + 1] = '\0';

			attr = fr_str2int(header_names, charbuf, 0);
			if (!attr) {
				RDEBUG2("Found unknown header {%s}: Not doing anything", charbuf);
				break;
			}

			new_vp = radius_paircreate(request,
						   &request->config_items,
						   attr, 0);

			/*
			 *	The data after the '}' may be binary,
			 *	so we copy it via memcpy.
			 */
			new_vp->length = vp->length;
			new_vp->length -= (p - q + 1);
			new_vp->vp_octets = b = talloc_array(new_vp, uint8_t, new_vp->length);
			memcpy(b, p + 1, new_vp->length);
		}
			break;

		case PW_CLEARTEXT_PASSWORD:
		case PW_CRYPT_PASSWORD:
		case PW_NS_MTA_MD5_PASSWORD:
			found_pw = true;
			break;	/* don't touch these */

		case PW_MD5_PASSWORD:
		case PW_SMD5_PASSWORD:
		case PW_NT_PASSWORD:
		case PW_LM_PASSWORD:
			if (inst->normify)
				normify(request, vp, 16); /* ensure it's in the right format */
			found_pw = true;
			break;

		case PW_SHA_PASSWORD:
		case PW_SSHA_PASSWORD:
			if (inst->normify)
				normify(request, vp, 20); /* ensure it's in the right format */
			found_pw = true;
			break;

			/*
			 *	If it's proxied somewhere, don't complain
			 *	about not having passwords or Auth-Type.
			 */
		case PW_PROXY_TO_REALM:
		{
			REALM *realm = realm_find(vp->vp_strvalue);
			if (realm && realm->auth_pool) {
				return RLM_MODULE_NOOP;
			}
			break;
		}

		case PW_AUTH_TYPE:
			auth_type = true;

			/*
			 *	Auth-Type := Accept
			 *	Auth-Type := Reject
			 */
			if ((vp->vp_integer == 254) ||
			    (vp->vp_integer == 4)) {
			    found_pw = 1;
			}
			break;

		default:
			break;	/* ignore it */

		}
	}

	/*
	 *	Print helpful warnings if there was no password.
	 */
	if (!found_pw) {
		/*
		 *	Likely going to be proxied.  Avoid printing
		 *	warning message.
		 */
		if (pairfind(request->config_items, PW_REALM, 0, TAG_ANY) ||
		    (pairfind(request->config_items, PW_PROXY_TO_REALM, 0, TAG_ANY))) {
			return RLM_MODULE_NOOP;
		}

		/*
		 *	The TLS types don't need passwords.
		 */
		vp = pairfind(request->packet->vps, PW_EAP_TYPE, 0, TAG_ANY);
		if (vp &&
		    ((vp->vp_integer == 13) || /* EAP-TLS */
		     (vp->vp_integer == 21) || /* EAP-TTLS */
		     (vp->vp_integer == 25))) {	/* PEAP */
			return RLM_MODULE_NOOP;
		}

		RWDEBUG("No \"known good\" password found for the user.  Not setting Auth-Type.");
		RWDEBUG("Authentication will fail unless a \"known good\" password is available.");
		return RLM_MODULE_NOOP;
	}

	/*
	 *	Don't touch existing Auth-Types.
	 */
	if (auth_type) {
		RWDEBUG2("Auth-Type already set.  Not setting to PAP");
		return RLM_MODULE_NOOP;
	}

	/*
	 *	Can't do PAP if there's no password.
	 */
	if (!request->password ||
	    (request->password->da->attr != PW_USER_PASSWORD)) {
		/*
		 *	Don't print out debugging messages if we know
		 *	they're useless.
		 */
		if (request->packet->code == PW_ACCESS_CHALLENGE) {
			return RLM_MODULE_NOOP;
		}

		RDEBUG2("No clear-text password in the request.  Not performing PAP.");
		return RLM_MODULE_NOOP;
	}

	if (inst->auth_type) {
		vp = radius_paircreate(request, &request->config_items,
				       PW_AUTH_TYPE, 0);
		vp->vp_integer = inst->auth_type;
	}

	return RLM_MODULE_UPDATED;
}

/*
 *	PAP authentication functions
 */

static int pap_auth_clear(UNUSED rlm_pap_t *inst, REQUEST *request, VALUE_PAIR *vp)
{
	RDEBUG("Using clear text password \"%s\"", vp->vp_strvalue);

	if ((vp->length != request->password->length) ||
	    (rad_digest_cmp(vp->vp_octets,
			    request->password->vp_octets,
			    vp->length) != 0)) {
		REDEBUG("CLEAR TEXT password check failed");
		return RLM_MODULE_REJECT;
	}
	return RLM_MODULE_OK;
}

static int pap_auth_crypt(UNUSED rlm_pap_t *inst, REQUEST *request, VALUE_PAIR *vp)
{
	RDEBUG("Using CRYPT password \"%s\"", vp->vp_strvalue);

	if (fr_crypt_check(request->password->vp_strvalue,
			   vp->vp_strvalue) != 0) {
		REDEBUG("CRYPT password check failed");
		return RLM_MODULE_REJECT;
	}
	return RLM_MODULE_OK;
}

static int pap_auth_md5(rlm_pap_t *inst, REQUEST *request, VALUE_PAIR *vp)
{
	FR_MD5_CTX md5_context;
	uint8_t binbuf[128];

	RDEBUG("Using MD5 encryption.");

	if (inst->normify)
		normify(request, vp, 16);
	if (vp->length != 16) {
		REDEBUG("Configured MD5 password has incorrect length");
		return RLM_MODULE_REJECT;
	}

	fr_MD5Init(&md5_context);
	fr_MD5Update(&md5_context, request->password->vp_octets,
		     request->password->length);
	fr_MD5Final(binbuf, &md5_context);

	if (rad_digest_cmp(binbuf, vp->vp_octets, vp->length) != 0) {
		REDEBUG("MD5 password check failed");
		return RLM_MODULE_REJECT;
	}

	return RLM_MODULE_OK;
}


static int pap_auth_smd5(rlm_pap_t *inst, REQUEST *request, VALUE_PAIR *vp)
{
	FR_MD5_CTX md5_context;
	uint8_t binbuf[128];

	RDEBUG("Using SMD5 encryption.");

	if (inst->normify)
		normify(request, vp, 16);
	if (vp->length <= 16) {
		REDEBUG("Configured SMD5 password has incorrect length");
		return RLM_MODULE_REJECT;
	}

	fr_MD5Init(&md5_context);
	fr_MD5Update(&md5_context, request->password->vp_octets,
		     request->password->length);
	fr_MD5Update(&md5_context, &vp->vp_octets[16], vp->length - 16);
	fr_MD5Final(binbuf, &md5_context);

	/*
	 *	Compare only the MD5 hash results, not the salt.
	 */
	if (rad_digest_cmp(binbuf, vp->vp_octets, 16) != 0) {
		REDEBUG("SMD5 password check failed");
		return RLM_MODULE_REJECT;
	}

	return RLM_MODULE_OK;
}

static int pap_auth_sha(rlm_pap_t *inst, REQUEST *request, VALUE_PAIR *vp)
{
	fr_SHA1_CTX sha1_context;
	uint8_t binbuf[128];

	RDEBUG("Using SHA1 encryption.");

	if (inst->normify)
		normify(request, vp, 20);
	if (vp->length != 20) {
		REDEBUG("SHA1 password has incorrect length");
		return RLM_MODULE_REJECT;
	}

	fr_SHA1Init(&sha1_context);
	fr_SHA1Update(&sha1_context, request->password->vp_octets,
		      request->password->length);
	fr_SHA1Final(binbuf,&sha1_context);

	if (rad_digest_cmp(binbuf, vp->vp_octets, vp->length) != 0) {
		REDEBUG("SHA1 password check failed");
		return RLM_MODULE_REJECT;
	}

	return RLM_MODULE_OK;
}

static int pap_auth_ssha(rlm_pap_t *inst, REQUEST *request, VALUE_PAIR *vp)
{
	fr_SHA1_CTX sha1_context;
	uint8_t binbuf[128];

	RDEBUG("Using SSHA encryption.");

	if (inst->normify)
		normify(request, vp, 20);
	if (vp->length <= 20) {
		REDEBUG("SSHA password has incorrect length");
		return RLM_MODULE_REJECT;
	}

	fr_SHA1Init(&sha1_context);
	fr_SHA1Update(&sha1_context, request->password->vp_octets,
		      request->password->length);
	fr_SHA1Update(&sha1_context, &vp->vp_octets[20], vp->length - 20);
	fr_SHA1Final(binbuf,&sha1_context);

	if (rad_digest_cmp(binbuf, vp->vp_octets, 20) != 0) {
		REDEBUG("SSHA password check failed");
		return RLM_MODULE_REJECT;
	}

	return RLM_MODULE_OK;
}

static int pap_auth_nt(rlm_pap_t *inst, REQUEST *request, VALUE_PAIR *vp)
{
	uint8_t binbuf[16];
	char charbuf[32 + 1];

	RDEBUG("Using NT encryption.");

	if (inst->normify)
		normify(request, vp, 16);
	if (vp->length != 16) {
		REDEBUG("Configured NT-Password has incorrect length");
		return RLM_MODULE_REJECT;
	}

	if (radius_xlat(charbuf, sizeof(charbuf), request, "%{mschap:NT-Hash %{User-Password}}", NULL, NULL) < 0){
		return RLM_MODULE_REJECT;
	}

	if ((fr_hex2bin(binbuf, charbuf, sizeof(binbuf)) != vp->length) ||
	    (rad_digest_cmp(binbuf, vp->vp_octets, vp->length) != 0)) {
		REDEBUG("NT password check failed");
		return RLM_MODULE_REJECT;
	}

	return RLM_MODULE_OK;
}


static int pap_auth_lm(rlm_pap_t *inst, REQUEST *request, VALUE_PAIR *vp)
{
	uint8_t binbuf[16];
	char charbuf[32 + 1];

	RDEBUG("Using LM encryption.");

	if (inst->normify)
		normify(request, vp, 16);
	if (vp->length != 16) {
		REDEBUG("Configure LM-Password has incorrect length");
		return RLM_MODULE_REJECT;
	}

	if (radius_xlat(charbuf, sizeof(charbuf), request, "%{mschap:LM-Hash %{User-Password}}", NULL, NULL) < 0){
		return RLM_MODULE_REJECT;
	}

	if ((fr_hex2bin(binbuf, charbuf, sizeof(binbuf)) != vp->length) ||
	    (rad_digest_cmp(binbuf, vp->vp_octets, vp->length) != 0)) {
		REDEBUG("LM password check failed");
		return RLM_MODULE_REJECT;
	}

	return RLM_MODULE_OK;
}

static int pap_auth_ns_mta_md5(UNUSED rlm_pap_t *inst, REQUEST *request, VALUE_PAIR *vp)
{
	FR_MD5_CTX md5_context;
	uint8_t binbuf[128];
	uint8_t buff[MAX_STRING_LEN];
	char buff2[MAX_STRING_LEN + 50];

	RDEBUG("Using NT-MTA-MD5 password");

	if (vp->length != 64) {
		REDEBUG("Configured NS-MTA-MD5-Password has incorrect length");
		return RLM_MODULE_REJECT;
	}

	/*
	 *	Sanity check the value of NS-MTA-MD5-Password
	 */
	if (fr_hex2bin(binbuf, vp->vp_strvalue, 32) != 16) {
		REDEBUG("Configured NS-MTA-MD5-Password has invalid value");
		return RLM_MODULE_REJECT;
	}

	/*
	 *	Ensure we don't have buffer overflows.
	 *
	 *	This really: sizeof(buff) - 2 - 2*32 - strlen(passwd)
	 */
	if (request->password->length >= (sizeof(buff) - 2 - 2 * 32)) {
		REDEBUG("Configured password is too long");
		return RLM_MODULE_REJECT;
	}

	/*
	 *	Set up the algorithm.
	 */
	{
		char *p = buff2;

		memcpy(p, &vp->vp_octets[32], 32);
		p += 32;
		*(p++) = 89;
		strcpy(p, request->password->vp_strvalue);
		p += strlen(p);
		*(p++) = 247;
		memcpy(p, &vp->vp_octets[32], 32);
		p += 32;

		fr_MD5Init(&md5_context);
		fr_MD5Update(&md5_context, (uint8_t *) buff2, p - buff2);
		fr_MD5Final(buff, &md5_context);
	}

	if (rad_digest_cmp(binbuf, buff, 16) != 0) {
		REDEBUG("NS-MTA-MD5 password check failed");
		return RLM_MODULE_REJECT;
	}

	return RLM_MODULE_OK;
}


/*
 *	Authenticate the user via one of any well-known password.
 */
static rlm_rcode_t mod_authenticate(void *instance, REQUEST *request)
{
	rlm_pap_t *inst = instance;
	VALUE_PAIR *vp;
	rlm_rcode_t rc = RLM_MODULE_INVALID;
	vp_cursor_t cursor;
	int (*auth_func)(rlm_pap_t *, REQUEST *, VALUE_PAIR *) = NULL;


	if (!request->password ||
	    (request->password->da->attr != PW_USER_PASSWORD)) {
		REDEBUG("You set 'Auth-Type = PAP' for a request that does not contain a User-Password attribute!");
		return RLM_MODULE_INVALID;
	}

	/*
	 *	The user MUST supply a non-zero-length password.
	 */
	if (request->password->length == 0) {
		REDEBUG("Password must not be empty");
		return RLM_MODULE_INVALID;
	}

	RDEBUG("login attempt with password \"%s\"", request->password->vp_strvalue);

	/*
	 *	Auto-detect passwords, by attribute in the
	 *	config items, to find out which authentication
	 *	function to call.
	 */
	for (vp = paircursor(&cursor, &request->config_items);
	     vp;
	     vp = pairnext(&cursor)) {
		if (!vp->da->vendor) switch (vp->da->attr) {
		case PW_CLEARTEXT_PASSWORD:
			auth_func = &pap_auth_clear;
			break;

		case PW_CRYPT_PASSWORD:
			auth_func = &pap_auth_crypt;
			break;

		case PW_MD5_PASSWORD:
			auth_func = &pap_auth_md5;
			break;

		case PW_SMD5_PASSWORD:
			auth_func = &pap_auth_smd5;
			break;

		case PW_SHA_PASSWORD:
			auth_func = &pap_auth_sha;
			break;

		case PW_SSHA_PASSWORD:
			auth_func = &pap_auth_ssha;
			break;

		case PW_NT_PASSWORD:
			auth_func = &pap_auth_nt;
			break;

		case PW_LM_PASSWORD:
			auth_func = &pap_auth_lm;
			break;

		case PW_NS_MTA_MD5_PASSWORD:
			auth_func = &pap_auth_ns_mta_md5;
			break;

		default:
			break;
		}

		if (auth_func != NULL) break;
	}

	/*
	 *	No attribute was found that looked like a password to match.
	 */
	if (!auth_func) {
		RDEBUG("No password configured for the user.  Cannot do authentication");
		return RLM_MODULE_FAIL;
	}

	/*
	 *	Authenticate, and return.
	 */
	rc = auth_func(inst, request, vp);

	if (rc == RLM_MODULE_REJECT) {
		RDEBUG("Passwords don't match");
	}

	if (rc == RLM_MODULE_OK) {
		RDEBUG("User authenticated successfully");
	}

	return rc;
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
module_t rlm_pap = {
	RLM_MODULE_INIT,
	"PAP",
	RLM_TYPE_CHECK_CONFIG_SAFE | RLM_TYPE_HUP_SAFE,   	/* type */
	sizeof(rlm_pap_t),
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
