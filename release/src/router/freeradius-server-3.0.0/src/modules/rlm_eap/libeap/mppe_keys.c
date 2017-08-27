/*
 * mppe_keys.c
 *
 * Version:     $Id$
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 *
 * Copyright 2002  Axis Communications AB
 * Copyright 2006  The FreeRADIUS server project
 * Authors: Henrik Eriksson <henriken@axis.com> & Lars Viklund <larsv@axis.com>
 */

RCSID("$Id$")
USES_APPLE_DEPRECATED_API	/* OpenSSL API has been deprecated by Apple */

#include "eap_tls.h"
#include <openssl/hmac.h>

/*
 * TLS PRF from RFC 2246
 */
static void P_hash(EVP_MD const *evp_md,
		   unsigned char const *secret, unsigned int secret_len,
		   unsigned char const *seed,   unsigned int seed_len,
		   unsigned char *out, unsigned int out_len)
{
	HMAC_CTX ctx_a, ctx_out;
	unsigned char a[HMAC_MAX_MD_CBLOCK];
	unsigned int size;

	HMAC_CTX_init(&ctx_a);
	HMAC_CTX_init(&ctx_out);
	HMAC_Init_ex(&ctx_a, secret, secret_len, evp_md, NULL);
	HMAC_Init_ex(&ctx_out, secret, secret_len, evp_md, NULL);

	size = HMAC_size(&ctx_out);

	/* Calculate A(1) */
	HMAC_Update(&ctx_a, seed, seed_len);
	HMAC_Final(&ctx_a, a, NULL);

	while (1) {
		/* Calculate next part of output */
		HMAC_Update(&ctx_out, a, size);
		HMAC_Update(&ctx_out, seed, seed_len);

		/* Check if last part */
		if (out_len < size) {
			HMAC_Final(&ctx_out, a, NULL);
			memcpy(out, a, out_len);
			break;
		}

		/* Place digest in output buffer */
		HMAC_Final(&ctx_out, out, NULL);
		HMAC_Init_ex(&ctx_out, NULL, 0, NULL, NULL);
		out += size;
		out_len -= size;

		/* Calculate next A(i) */
		HMAC_Init_ex(&ctx_a, NULL, 0, NULL, NULL);
		HMAC_Update(&ctx_a, a, size);
		HMAC_Final(&ctx_a, a, NULL);
	}

	HMAC_CTX_cleanup(&ctx_a);
	HMAC_CTX_cleanup(&ctx_out);
	memset(a, 0, sizeof(a));
}

static void PRF(unsigned char const *secret, unsigned int secret_len,
		unsigned char const *seed,   unsigned int seed_len,
		unsigned char *out, unsigned char *buf, unsigned int out_len)
{
	unsigned int i;
	unsigned int len = (secret_len + 1) / 2;
	uint8_t const *s1 = secret;
	uint8_t const *s2 = secret + (secret_len - len);

	P_hash(EVP_md5(),  s1, len, seed, seed_len, out, out_len);
	P_hash(EVP_sha1(), s2, len, seed, seed_len, buf, out_len);

	for (i=0; i < out_len; i++) {
		out[i] ^= buf[i];
	}
}

#define EAPTLS_MPPE_KEY_LEN     32

#define FR_TLS_PRF_LABEL "ttls keying material"

/*
 *	Generate keys according to RFC 2716 and add to reply
 */
void eaptls_gen_mppe_keys(REQUEST *request, SSL *s,
			  char const *prf_label)
{
	unsigned char out[4*EAPTLS_MPPE_KEY_LEN], buf[4*EAPTLS_MPPE_KEY_LEN];
	unsigned char seed[64 + 2*SSL3_RANDOM_SIZE];
	unsigned char *p = seed;
	size_t prf_size;

	if (!s->s3) {
		EDEBUG("No SSLv3 information");
		return;
	}

	prf_size = strlen(prf_label);

	memcpy(p, prf_label, prf_size);
	p += prf_size;

	memcpy(p, s->s3->client_random, SSL3_RANDOM_SIZE);
	p += SSL3_RANDOM_SIZE;
	prf_size += SSL3_RANDOM_SIZE;

	memcpy(p, s->s3->server_random, SSL3_RANDOM_SIZE);
	prf_size += SSL3_RANDOM_SIZE;

	PRF(s->session->master_key, s->session->master_key_length,
	    seed, prf_size, out, buf, sizeof(out));

	p = out;
	eap_add_reply(request, "MS-MPPE-Recv-Key", p, EAPTLS_MPPE_KEY_LEN);
	p += EAPTLS_MPPE_KEY_LEN;
	eap_add_reply(request, "MS-MPPE-Send-Key", p, EAPTLS_MPPE_KEY_LEN);

	eap_add_reply(request, "EAP-MSK", out, 64);
	eap_add_reply(request, "EAP-EMSK", out + 64, 64);
}


#define FR_TLS_PRF_CHALLENGE	"ttls challenge"

/*
 *	Generate the TTLS challenge
 *
 *	It's in the TLS module simply because it's only a few lines
 *	of code, and it needs access to the TLS PRF functions.
 */
void eapttls_gen_challenge(SSL *s, uint8_t *buffer, size_t size)
{
	uint8_t out[32], buf[32];
	uint8_t seed[sizeof(FR_TLS_PRF_CHALLENGE)-1 + 2*SSL3_RANDOM_SIZE];
	uint8_t *p = seed;

	if (!s->s3) {
		EDEBUG("No SSLv3 information");
		return;
	}

	memcpy(p, FR_TLS_PRF_CHALLENGE, sizeof(FR_TLS_PRF_CHALLENGE)-1);
	p += sizeof(FR_TLS_PRF_CHALLENGE)-1;
	memcpy(p, s->s3->client_random, SSL3_RANDOM_SIZE);
	p += SSL3_RANDOM_SIZE;
	memcpy(p, s->s3->server_random, SSL3_RANDOM_SIZE);

	PRF(s->session->master_key, s->session->master_key_length,
	    seed, sizeof(seed), out, buf, sizeof(out));

	memcpy(buffer, out, size);
}

/*
 *	Actually generates EAP-Session-Id, which is an internal server
 *	attribute.  Not all systems want to send EAP-Key-Nam
 */
void eaptls_gen_eap_key(RADIUS_PACKET *packet, SSL *s, uint32_t header)
{
	VALUE_PAIR *vp;
	uint8_t *p;

	if (!s->s3) {
		EDEBUG("No SSLv3 information");
		return;
	}

	vp = paircreate(packet, PW_EAP_SESSION_ID, 0);
	if (!vp) return;

	vp->length = 1 + 2 * SSL3_RANDOM_SIZE;
	p = talloc_array(vp, uint8_t, vp->length);

	p[0] = header & 0xff;
	memcpy(p + 1, s->s3->client_random, SSL3_RANDOM_SIZE);
	memcpy(p + 1 + SSL3_RANDOM_SIZE,
	       s->s3->server_random, SSL3_RANDOM_SIZE);
	vp->vp_octets = p;
	pairadd(&packet->vps, vp);
}
