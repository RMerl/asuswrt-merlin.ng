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
 * @file rlm_cram.c
 * @brief CRAM mail authentication (APOP, CRAM-MD5)
   @verbatim
	Attributes used (Vendor Code/PEN: 11406, you may change it to your own)
	101 (Sandy-Mail-Authtype), selects CRAM protocol, possible values:
		2: CRAM-MD5
		3: APOP
		8: CRAM-MD4
		9: CRAM-SHA1
	102 (Sandy-Mail-Challenge), contains server's challenge (usually
	text banner)
	103 (Sandy-Mail-Response), contains client's response, 16 octets
	for APOP/CRAM-MD5/CRAM-MD4, 20 octets for CRAM-SHA1
   @endverbatim
 * @copyright 2001,2006  The FreeRADIUS server project
 * @copyright 2002 SANDY (http://www.sandy.ru/) under GPLr
 */
RCSID("$Id$")

#include	<freeradius-devel/radiusd.h>
#include	<freeradius-devel/modules.h>

#include	<freeradius-devel/md5.h>

#include 	<ctype.h>

#define VENDORPEC_SM  11406
#define		SM_AUTHTYPE	101
#define		SM_CHALLENGE	102
#define		SM_RESPONSE     103

static void calc_apop_digest(uint8_t *buffer, uint8_t const *challenge,
			     size_t challen, char const *password)
{
	FR_MD5_CTX context;

	fr_MD5Init(&context);
	fr_MD5Update(&context, challenge, challen);
	fr_MD5Update(&context, (uint8_t const *) password, strlen(password));
	fr_MD5Final(buffer, &context);
}


static void calc_md5_digest(uint8_t *buffer, uint8_t const *challenge, size_t challen, char const *password)
{
	uint8_t buf[1024];
	int i;
	FR_MD5_CTX context;

	memset(buf, 0, 1024);
	memset(buf, 0x36, 64);
	for(i=0; i<64 && password[i]; i++) buf[i]^=password[i];
	memcpy(buf+64, challenge, challen);
	fr_MD5Init(&context);
	fr_MD5Update(&context, buf, 64+challen);
	memset(buf, 0x5c, 64);
	for(i=0; i<64 && password[i]; i++) buf[i]^=password[i];
	fr_MD5Final(buf+64,&context);
	fr_MD5Init(&context);
	fr_MD5Update(&context,buf,64+16);
	fr_MD5Final(buffer,&context);
}

static void calc_md4_digest(uint8_t *buffer, uint8_t const *challenge, size_t challen, char const *password)
{
	uint8_t buf[1024];
	int i;
	FR_MD4_CTX context;

	memset(buf, 0, 1024);
	memset(buf, 0x36, 64);
	for(i=0; i<64 && password[i]; i++) buf[i]^=password[i];
	memcpy(buf+64, challenge, challen);
	fr_MD4Init(&context);
	fr_MD4Update(&context,buf,64+challen);
	memset(buf, 0x5c, 64);
	for(i=0; i<64 && password[i]; i++) buf[i]^=password[i];
	fr_MD4Final(buf+64,&context);
	fr_MD4Init(&context);
	fr_MD4Update(&context,buf,64+16);
	fr_MD4Final(buffer,&context);
}

static void calc_sha1_digest(uint8_t *buffer, uint8_t const *challenge,
			     size_t challen, char const *password)
{
	uint8_t buf[1024];
	int i;
	fr_SHA1_CTX context;

	memset(buf, 0, 1024);
	memset(buf, 0x36, 64);
	for(i=0; i<64 && password[i]; i++) buf[i]^=password[i];
	memcpy(buf+64, challenge, challen);
	fr_SHA1Init(&context);
	fr_SHA1Update(&context,buf,64+challen);
	memset(buf, 0x5c, 64);
	for(i=0; i<64 && password[i]; i++) buf[i]^=password[i];
	fr_SHA1Final(buf+64,&context);
	fr_SHA1Init(&context);
	fr_SHA1Update(&context,buf,64+20);
	fr_SHA1Final(buffer,&context);
}


static rlm_rcode_t mod_authenticate(UNUSED void * instance, REQUEST *request)
{
	VALUE_PAIR *authtype, *challenge, *response, *password;
	uint8_t buffer[64];

	password = pairfind(request->config_items, PW_CLEARTEXT_PASSWORD, 0, TAG_ANY);
	if(!password) {
		AUTH("rlm_cram: Cleartext-Password is required for authentication.");
		return RLM_MODULE_INVALID;
	}
	authtype = pairfind(request->packet->vps, SM_AUTHTYPE, VENDORPEC_SM, TAG_ANY);
	if(!authtype) {
		AUTH("rlm_cram: Required attribute Sandy-Mail-Authtype missed");
		return RLM_MODULE_INVALID;
	}
	challenge = pairfind(request->packet->vps, SM_CHALLENGE, VENDORPEC_SM, TAG_ANY);
	if(!challenge) {
		AUTH("rlm_cram: Required attribute Sandy-Mail-Challenge missed");
		return RLM_MODULE_INVALID;
	}
	response = pairfind(request->packet->vps, SM_RESPONSE, VENDORPEC_SM, TAG_ANY);
	if(!response) {
		AUTH("rlm_cram: Required attribute Sandy-Mail-Response missed");
		return RLM_MODULE_INVALID;
	}
	switch(authtype->vp_integer){
		case 2:				/*	CRAM-MD5	*/
			if(challenge->length < 5 || response->length != 16) {
				AUTH("rlm_cram: invalid MD5 challenge/response length");
				return RLM_MODULE_INVALID;
			}
			calc_md5_digest(buffer, challenge->vp_octets, challenge->length, password->vp_strvalue);
			if(!memcmp(buffer, response->vp_octets, 16)) return RLM_MODULE_OK;
			break;
		case 3:				/*	APOP	*/
			if(challenge->length < 5 || response->length != 16) {
				AUTH("rlm_cram: invalid APOP challenge/response length");
				return RLM_MODULE_INVALID;
			}
			calc_apop_digest(buffer, challenge->vp_octets, challenge->length, password->vp_strvalue);
			if(!memcmp(buffer, response->vp_octets, 16)) return RLM_MODULE_OK;
			break;
		case 8:				/*	CRAM-MD4	*/
			if(challenge->length < 5 || response->length != 16) {
				AUTH("rlm_cram: invalid MD4 challenge/response length");
				return RLM_MODULE_INVALID;
			}
			calc_md4_digest(buffer, challenge->vp_octets, challenge->length, password->vp_strvalue);
			if(!memcmp(buffer, response->vp_octets, 16)) return RLM_MODULE_OK;
			break;
		case 9:				/*	CRAM-SHA1	*/
			if(challenge->length < 5 || response->length != 20) {
				AUTH("rlm_cram: invalid MD4 challenge/response length");
				return RLM_MODULE_INVALID;
			}
			calc_sha1_digest(buffer, challenge->vp_octets, challenge->length, password->vp_strvalue);
			if(!memcmp(buffer, response->vp_octets, 20)) return RLM_MODULE_OK;
			break;
		default:
			AUTH("rlm_cram: unsupported Sandy-Mail-Authtype");
			return RLM_MODULE_INVALID;
	}
	return RLM_MODULE_NOTFOUND;

}

module_t rlm_cram = {
	RLM_MODULE_INIT,
	"CRAM",
	RLM_TYPE_THREAD_SAFE,		/* type */
	0,
	NULL,				/* CONF_PARSER */
	NULL,				/* instantiation */
	NULL,				/* detach */
	{
		mod_authenticate,	/* authenticate */
		NULL,			/* authorize */
		NULL,			/* pre-accounting */
		NULL,			/* accounting */
		NULL,			/* checksimul */
		NULL,			/* pre-proxy */
		NULL,			/* post-proxy */
		NULL			/* post-auth */
	},
};
