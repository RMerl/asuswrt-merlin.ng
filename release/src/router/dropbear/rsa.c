/*
 * Dropbear - a SSH2 server
 * 
 * Copyright (c) 2002,2003 Matt Johnston
 * All rights reserved.
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
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE. */

/* Perform RSA operations on data, including reading keys, signing and
 * verification.
 *
 * The format is specified in rfc2437, Applied Cryptography or The Handbook of
 * Applied Cryptography detail the general algorithm. */

#include "includes.h"
#include "dbutil.h"
#include "bignum.h"
#include "rsa.h"
#include "buffer.h"
#include "ssh.h"
#include "dbrandom.h"
#include "signkey.h"

#if DROPBEAR_RSA 

#if !(DROPBEAR_RSA_SHA1 || DROPBEAR_RSA_SHA256)
#error Somehow RSA was enabled with neither DROPBEAR_RSA_SHA1 nor DROPBEAR_RSA_SHA256
#endif

static void rsa_pad_em(const dropbear_rsa_key * key,
	const buffer *data_buf, mp_int * rsa_em, enum signature_type sigtype);

/* Load a public rsa key from a buffer, initialising the values.
 * The key will have the same format as buf_put_rsa_key.
 * These should be freed with rsa_key_free.
 * Returns DROPBEAR_SUCCESS or DROPBEAR_FAILURE */
int buf_get_rsa_pub_key(buffer* buf, dropbear_rsa_key *key) {

	int ret = DROPBEAR_FAILURE;
	TRACE(("enter buf_get_rsa_pub_key"))
	dropbear_assert(key != NULL);
	m_mp_alloc_init_multi(&key->e, &key->n, NULL);
	key->d = NULL;
	key->p = NULL;
	key->q = NULL;

	buf_incrpos(buf, 4+SSH_SIGNKEY_RSA_LEN); /* int + "ssh-rsa" */

	if (buf_getmpint(buf, key->e) == DROPBEAR_FAILURE
	 || buf_getmpint(buf, key->n) == DROPBEAR_FAILURE) {
		TRACE(("leave buf_get_rsa_pub_key: failure"))
		goto out;
	}

	if (mp_count_bits(key->n) < MIN_RSA_KEYLEN) {
		dropbear_log(LOG_WARNING, "RSA key too short");
		goto out;
	}

	/* 64 bit is limit used by openssl, so we won't block any keys in the wild */
	if (mp_count_bits(key->e) > 64) {
		dropbear_log(LOG_WARNING, "RSA key bad e");
		goto out;
	}

	TRACE(("leave buf_get_rsa_pub_key: success"))
	ret = DROPBEAR_SUCCESS;
out:
	if (ret == DROPBEAR_FAILURE) {
		m_mp_free_multi(&key->e, &key->n, NULL);
	}
	return ret;
}

/* Same as buf_get_rsa_pub_key, but reads private bits at the end.
 * Loads a private rsa key from a buffer
 * Returns DROPBEAR_SUCCESS or DROPBEAR_FAILURE */
int buf_get_rsa_priv_key(buffer* buf, dropbear_rsa_key *key) {
	int ret = DROPBEAR_FAILURE;

	TRACE(("enter buf_get_rsa_priv_key"))
	dropbear_assert(key != NULL);

	if (buf_get_rsa_pub_key(buf, key) == DROPBEAR_FAILURE) {
		TRACE(("leave buf_get_rsa_priv_key: pub: ret == DROPBEAR_FAILURE"))
		return DROPBEAR_FAILURE;
	}
	
	key->d = NULL;
	key->p = NULL;
	key->q = NULL;

	m_mp_alloc_init_multi(&key->d, NULL);
	if (buf_getmpint(buf, key->d) == DROPBEAR_FAILURE) {
		TRACE(("leave buf_get_rsa_priv_key: d: ret == DROPBEAR_FAILURE"))
		goto out;
	}

	if (buf->pos == buf->len) {
		/* old Dropbear private keys didn't keep p and q, so we will ignore them*/
	} else {
		m_mp_alloc_init_multi(&key->p, &key->q, NULL);

		if (buf_getmpint(buf, key->p) == DROPBEAR_FAILURE) {
			TRACE(("leave buf_get_rsa_priv_key: p: ret == DROPBEAR_FAILURE"))
			goto out;
		}

		if (buf_getmpint(buf, key->q) == DROPBEAR_FAILURE) {
			TRACE(("leave buf_get_rsa_priv_key: q: ret == DROPBEAR_FAILURE"))
			goto out;
		}
	}

	ret = DROPBEAR_SUCCESS;
out:
	if (ret == DROPBEAR_FAILURE) {
		m_mp_free_multi(&key->d, &key->p, &key->q, NULL);
	}
	TRACE(("leave buf_get_rsa_priv_key"))
	return ret;
}
	

/* Clear and free the memory used by a public or private key */
void rsa_key_free(dropbear_rsa_key *key) {

	TRACE2(("enter rsa_key_free"))

	if (key == NULL) {
		TRACE2(("leave rsa_key_free: key == NULL"))
		return;
	}
	m_mp_free_multi(&key->d, &key->e, &key->p, &key->q, &key->n, NULL);
	m_free(key);
	TRACE2(("leave rsa_key_free"))
}

/* Put the public rsa key into the buffer in the required format:
 *
 * string	"ssh-rsa"
 * mp_int	e
 * mp_int	n
 */
void buf_put_rsa_pub_key(buffer* buf, const dropbear_rsa_key *key) {

	TRACE(("enter buf_put_rsa_pub_key"))
	dropbear_assert(key != NULL);

	buf_putstring(buf, SSH_SIGNKEY_RSA, SSH_SIGNKEY_RSA_LEN);
	buf_putmpint(buf, key->e);
	buf_putmpint(buf, key->n);

	TRACE(("leave buf_put_rsa_pub_key"))

}

/* Same as buf_put_rsa_pub_key, but with the private "x" key appended */
void buf_put_rsa_priv_key(buffer* buf, const dropbear_rsa_key *key) {

	TRACE(("enter buf_put_rsa_priv_key"))

	dropbear_assert(key != NULL);
	buf_put_rsa_pub_key(buf, key);
	buf_putmpint(buf, key->d);

	/* new versions have p and q, old versions don't */
	if (key->p) {
		buf_putmpint(buf, key->p);
	}
	if (key->q) {
		buf_putmpint(buf, key->q);
	}


	TRACE(("leave buf_put_rsa_priv_key"))

}

#if DROPBEAR_SIGNKEY_VERIFY
/* Verify a signature in buf, made on data by the key given.
 * Returns DROPBEAR_SUCCESS or DROPBEAR_FAILURE */
int buf_rsa_verify(buffer * buf, const dropbear_rsa_key *key, 
		enum signature_type sigtype, const buffer *data_buf) {
	unsigned int slen;
	DEF_MP_INT(rsa_s);
	DEF_MP_INT(rsa_mdash);
	DEF_MP_INT(rsa_em);
	int ret = DROPBEAR_FAILURE;

	TRACE(("enter buf_rsa_verify"))

	dropbear_assert(key != NULL);

	m_mp_init_multi(&rsa_mdash, &rsa_s, &rsa_em, NULL);

	slen = buf_getint(buf);
	if (slen != (unsigned int)mp_ubin_size(key->n)) {
		TRACE(("bad size"))
		goto out;
	}

	if (mp_from_ubin(&rsa_s, buf_getptr(buf, buf->len - buf->pos),
				buf->len - buf->pos) != MP_OKAY) {
		TRACE(("failed reading rsa_s"))
		goto out;
	}

	/* check that s <= n-1 */
	if (mp_cmp(&rsa_s, key->n) != MP_LT) {
		TRACE(("s > n-1"))
		goto out;
	}

	/* create the magic PKCS padded value */
	rsa_pad_em(key, data_buf, &rsa_em, sigtype);

	if (mp_exptmod(&rsa_s, key->e, key->n, &rsa_mdash) != MP_OKAY) {
		TRACE(("failed exptmod rsa_s"))
		goto out;
	}

	if (mp_cmp(&rsa_em, &rsa_mdash) == MP_EQ) {
		/* signature is valid */
		TRACE(("success!"))
		ret = DROPBEAR_SUCCESS;
	}

out:
	mp_clear_multi(&rsa_mdash, &rsa_s, &rsa_em, NULL);
	TRACE(("leave buf_rsa_verify: ret %d", ret))
	return ret;
}

#endif /* DROPBEAR_SIGNKEY_VERIFY */

/* Sign the data presented with key, writing the signature contents
 * to the buffer */
void buf_put_rsa_sign(buffer* buf, const dropbear_rsa_key *key, 
		enum signature_type sigtype, const buffer *data_buf) {
	const char *name = NULL;
	unsigned int nsize, ssize, namelen = 0;
	unsigned int i;
	size_t written;
	DEF_MP_INT(rsa_s);
	DEF_MP_INT(rsa_tmp1);
	DEF_MP_INT(rsa_tmp2);
	DEF_MP_INT(rsa_tmp3);
	
	TRACE(("enter buf_put_rsa_sign"))
	dropbear_assert(key != NULL);

	m_mp_init_multi(&rsa_s, &rsa_tmp1, &rsa_tmp2, &rsa_tmp3, NULL);

	rsa_pad_em(key, data_buf, &rsa_tmp1, sigtype);

	/* the actual signing of the padded data */

#if DROPBEAR_RSA_BLINDING

	/* With blinding, s = (r^(-1))((em)*r^e)^d mod n */

	/* generate the r blinding value */
	/* rsa_tmp2 is r */
	gen_random_mpint(key->n, &rsa_tmp2);

	/* rsa_tmp1 is em */
	/* em' = em * r^e mod n */

	/* rsa_s used as a temp var*/
	if (mp_exptmod(&rsa_tmp2, key->e, key->n, &rsa_s) != MP_OKAY) {
		dropbear_exit("RSA error");
	}
	if (mp_invmod(&rsa_tmp2, key->n, &rsa_tmp3) != MP_OKAY) {
		dropbear_exit("RSA error");
	}
	if (mp_mulmod(&rsa_tmp1, &rsa_s, key->n, &rsa_tmp2) != MP_OKAY) {
		dropbear_exit("RSA error");
	}

	/* rsa_tmp2 is em' */
	/* s' = (em')^d mod n */
	if (mp_exptmod(&rsa_tmp2, key->d, key->n, &rsa_tmp1) != MP_OKAY) {
		dropbear_exit("RSA error");
	}

	/* rsa_tmp1 is s' */
	/* rsa_tmp3 is r^(-1) mod n */
	/* s = (s')r^(-1) mod n */
	if (mp_mulmod(&rsa_tmp1, &rsa_tmp3, key->n, &rsa_s) != MP_OKAY) {
		dropbear_exit("RSA error");
	}

#else

	/* s = em^d mod n */
	/* rsa_tmp1 is em */
	if (mp_exptmod(&rsa_tmp1, key->d, key->n, &rsa_s) != MP_OKAY) {
		dropbear_exit("RSA error");
	}

#endif /* DROPBEAR_RSA_BLINDING */

	mp_clear_multi(&rsa_tmp1, &rsa_tmp2, &rsa_tmp3, NULL);
	
	/* create the signature to return */
	name = signature_name_from_type(sigtype, &namelen);
	buf_putstring(buf, name, namelen);

	nsize = mp_ubin_size(key->n);

	/* string rsa_signature_blob length */
	buf_putint(buf, nsize);
	/* pad out s to same length as n */
	ssize = mp_ubin_size(&rsa_s);
	dropbear_assert(ssize <= nsize);
	for (i = 0; i < nsize-ssize; i++) {
		buf_putbyte(buf, 0x00);
	}

	if (mp_to_ubin(&rsa_s, buf_getwriteptr(buf, ssize), ssize, &written) != MP_OKAY) {
		dropbear_exit("RSA error");
	}
	buf_incrwritepos(buf, written);
	mp_clear(&rsa_s);

#if defined(DEBUG_RSA) && DEBUG_TRACE
	if (!debug_trace) {
		printhex("RSA sig", buf->data, buf->len);
	}
#endif
	

	TRACE(("leave buf_put_rsa_sign"))
}

/* Creates the message value as expected by PKCS, 
   see rfc8017 section 9.2 */
static void rsa_pad_em(const dropbear_rsa_key * key,
	const buffer *data_buf, mp_int * rsa_em, enum signature_type sigtype) {
    /* EM = 0x00 || 0x01 || PS || 0x00 || T 
	   PS is padding of 0xff to make EM the size of key->n

	   T is the DER encoding of the hash alg (sha1 or sha256)
	*/

	/* From rfc8017 page 46 */
#if DROPBEAR_RSA_SHA1
	const unsigned char T_sha1[] =
		{0x30, 0x21, 0x30, 0x09, 0x06, 0x05, 0x2b,
		 0x0e, 0x03, 0x02, 0x1a, 0x05, 0x00, 0x04, 0x14};
#endif
#if DROPBEAR_RSA_SHA256
	const unsigned char T_sha256[] =
		{0x30, 0x31, 0x30, 0x0d, 0x06, 0x09, 0x60, 0x86, 0x48, 0x01,
		 0x65, 0x03, 0x04, 0x02, 0x01, 0x05, 0x00, 0x04, 0x20};
#endif

    int Tlen = 0;
    const unsigned char *T = NULL;
	const struct ltc_hash_descriptor *hash_desc = NULL;
	buffer * rsa_EM = NULL;
	hash_state hs;
	unsigned int nsize;

	switch (sigtype) {
#if DROPBEAR_RSA_SHA1
		case DROPBEAR_SIGNATURE_RSA_SHA1:
			Tlen = sizeof(T_sha1);
			T = T_sha1;
			hash_desc = &sha1_desc;
			break;
#endif
#if DROPBEAR_RSA_SHA256
		case DROPBEAR_SIGNATURE_RSA_SHA256:
			Tlen = sizeof(T_sha256);
			T = T_sha256;
			hash_desc = &sha256_desc;
			break;
#endif
		default:
			assert(0);
	}
	

	nsize = mp_ubin_size(key->n);

	rsa_EM = buf_new(nsize);
	/* type byte */
	buf_putbyte(rsa_EM, 0x00);
	buf_putbyte(rsa_EM, 0x01);
	/* Padding with PS 0xFF bytes */
	while(rsa_EM->pos != rsa_EM->size - (1 + Tlen + hash_desc->hashsize)) {
		buf_putbyte(rsa_EM, 0xff);
	}
	buf_putbyte(rsa_EM, 0x00);
	/* Magic ASN1 stuff */
	buf_putbytes(rsa_EM, T, Tlen);

	/* The hash of the data */
	hash_desc->init(&hs);
	hash_desc->process(&hs, data_buf->data, data_buf->len);
	hash_desc->done(&hs, buf_getwriteptr(rsa_EM, hash_desc->hashsize));
	buf_incrwritepos(rsa_EM, hash_desc->hashsize);

	dropbear_assert(rsa_EM->pos == rsa_EM->size);

	/* Create the mp_int from the encoded bytes */
	buf_setpos(rsa_EM, 0);
	bytes_to_mp(rsa_em, buf_getptr(rsa_EM, rsa_EM->size),
			rsa_EM->size);
	buf_free(rsa_EM);
}

#endif /* DROPBEAR_RSA */
