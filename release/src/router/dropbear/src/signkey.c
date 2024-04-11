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

#include "includes.h"
#include "dbutil.h"
#include "signkey.h"
#include "buffer.h"
#include "ssh.h"
#include "ecdsa.h"
#include "sk-ecdsa.h"
#include "sk-ed25519.h"
#include "rsa.h"
#include "dss.h"
#include "ed25519.h"

static const char * const signkey_names[DROPBEAR_SIGNKEY_NUM_NAMED] = {
#if DROPBEAR_RSA
	"ssh-rsa",
#endif
#if DROPBEAR_DSS
	"ssh-dss",
#endif
#if DROPBEAR_ECDSA
	"ecdsa-sha2-nistp256",
	"ecdsa-sha2-nistp384",
	"ecdsa-sha2-nistp521",
#if DROPBEAR_SK_ECDSA
	"sk-ecdsa-sha2-nistp256@openssh.com",
#endif /* DROPBEAR_SK_ECDSA */
#endif /* DROPBEAR_ECDSA */
#if DROPBEAR_ED25519
	"ssh-ed25519",
#if DROPBEAR_SK_ED25519
	"sk-ssh-ed25519@openssh.com",
#endif /* DROPBEAR_SK_ED25519 */
#endif /* DROPBEAR_ED25519 */
	/* "rsa-sha2-256" is special-cased below since it is only a signature name, not key type */
};

/* malloc a new sign_key and set the dss and rsa keys to NULL */
sign_key * new_sign_key() {

	sign_key * ret;

	ret = (sign_key*)m_malloc(sizeof(sign_key));
	ret->type = DROPBEAR_SIGNKEY_NONE;
	ret->source = SIGNKEY_SOURCE_INVALID;
	return ret;
}

/* Returns key name corresponding to the type. Exits fatally
 * if the type is invalid */
const char* signkey_name_from_type(enum signkey_type type, unsigned int *namelen) {
	if (type >= DROPBEAR_SIGNKEY_NUM_NAMED) {
		dropbear_exit("Bad key type %d", type);
	}

	if (namelen) {
		*namelen = strlen(signkey_names[type]);
	}
	return signkey_names[type];
}

/* Returns DROPBEAR_SIGNKEY_NONE if none match */
enum signkey_type signkey_type_from_name(const char* name, unsigned int namelen) {
	int i;
	for (i = 0; i < DROPBEAR_SIGNKEY_NUM_NAMED; i++) {
		const char *fixed_name = signkey_names[i];
		if (namelen == strlen(fixed_name)
			&& memcmp(fixed_name, name, namelen) == 0) {

#if DROPBEAR_ECDSA
			/* Some of the ECDSA key sizes are defined even if they're not compiled in */
			if (0
#if !DROPBEAR_ECC_256
				|| i == DROPBEAR_SIGNKEY_ECDSA_NISTP256
#endif
#if !DROPBEAR_ECC_384
				|| i == DROPBEAR_SIGNKEY_ECDSA_NISTP384
#endif
#if !DROPBEAR_ECC_521
				|| i == DROPBEAR_SIGNKEY_ECDSA_NISTP521
#endif
				) {
				TRACE(("attempt to use ecdsa type %d not compiled in", i))
				return DROPBEAR_SIGNKEY_NONE;
			}
#endif

			return (enum signkey_type)i;
		}
	}

	TRACE(("signkey_type_from_name unexpected key type."))

	return DROPBEAR_SIGNKEY_NONE;
}

/* Special case for rsa-sha2-256. This could be generalised if more 
   signature names are added that aren't 1-1 with public key names */
const char* signature_name_from_type(enum signature_type type, unsigned int *namelen) {
#if DROPBEAR_RSA
#if DROPBEAR_RSA_SHA256
	if (type == DROPBEAR_SIGNATURE_RSA_SHA256) {
		if (namelen) {
			*namelen = strlen(SSH_SIGNATURE_RSA_SHA256);
		}
		return SSH_SIGNATURE_RSA_SHA256;
	}
#endif
#if DROPBEAR_RSA_SHA1
	if (type == DROPBEAR_SIGNATURE_RSA_SHA1) {
		if (namelen) {
			*namelen = strlen(SSH_SIGNKEY_RSA);
		}
		return SSH_SIGNKEY_RSA;
	}
#endif
#endif /* DROPBEAR_RSA */
	return signkey_name_from_type((enum signkey_type)type, namelen);
}

/* Returns DROPBEAR_SIGNATURE_NONE if none match */
enum signature_type signature_type_from_name(const char* name, unsigned int namelen) {
#if DROPBEAR_RSA
#if DROPBEAR_RSA_SHA256
	if (namelen == strlen(SSH_SIGNATURE_RSA_SHA256) 
		&& memcmp(name, SSH_SIGNATURE_RSA_SHA256, namelen) == 0) {
		return DROPBEAR_SIGNATURE_RSA_SHA256;
	}
#endif
#if DROPBEAR_RSA_SHA1
	if (namelen == strlen(SSH_SIGNKEY_RSA) 
		&& memcmp(name, SSH_SIGNKEY_RSA, namelen) == 0) {
		return DROPBEAR_SIGNATURE_RSA_SHA1;
	}
#endif
#endif /* DROPBEAR_RSA */
	return (enum signature_type)signkey_type_from_name(name, namelen);
}

/* Returns the signature type from a key type. Must not be called
   with RSA keytype */
enum signature_type signature_type_from_signkey(enum signkey_type keytype) {
#if DROPBEAR_RSA
	assert(keytype != DROPBEAR_SIGNKEY_RSA);
#endif
	assert(keytype < DROPBEAR_SIGNKEY_NUM_NAMED);
	return (enum signature_type)keytype;
}

enum signkey_type signkey_type_from_signature(enum signature_type sigtype) {
#if DROPBEAR_RSA
#if DROPBEAR_RSA_SHA256
	if (sigtype == DROPBEAR_SIGNATURE_RSA_SHA256) {
		return DROPBEAR_SIGNKEY_RSA;
	}
#endif
#if DROPBEAR_RSA_SHA1
	if (sigtype == DROPBEAR_SIGNATURE_RSA_SHA1) {
		return DROPBEAR_SIGNKEY_RSA;
	}
#endif
#endif /* DROPBEAR_RSA */
	assert((int)sigtype < (int)DROPBEAR_SIGNKEY_NUM_NAMED);
	return (enum signkey_type)sigtype;
}

/* Returns a pointer to the key part specific to "type".
Be sure to check both (ret != NULL) and (*ret != NULL) */
void **
signkey_key_ptr(sign_key *key, enum signkey_type type) {
	switch (type) {
#if DROPBEAR_ED25519
		case DROPBEAR_SIGNKEY_ED25519:
#if DROPBEAR_SK_ED25519
		case DROPBEAR_SIGNKEY_SK_ED25519:
#endif
			return (void**)&key->ed25519key;
#endif
#if DROPBEAR_ECDSA
#if DROPBEAR_ECC_256
		case DROPBEAR_SIGNKEY_ECDSA_NISTP256:
#if DROPBEAR_SK_ECDSA
		case DROPBEAR_SIGNKEY_SK_ECDSA_NISTP256:
#endif
			return (void**)&key->ecckey256;
#endif
#if DROPBEAR_ECC_384
		case DROPBEAR_SIGNKEY_ECDSA_NISTP384:
			return (void**)&key->ecckey384;
#endif
#if DROPBEAR_ECC_521
		case DROPBEAR_SIGNKEY_ECDSA_NISTP521:
			return (void**)&key->ecckey521;
#endif
#endif /* DROPBEAR_ECDSA */
#if DROPBEAR_RSA
		case DROPBEAR_SIGNKEY_RSA:
			return (void**)&key->rsakey;
#endif
#if DROPBEAR_DSS
		case DROPBEAR_SIGNKEY_DSS:
			return (void**)&key->dsskey;
#endif
		default:
			return NULL;
	}
}

/* returns DROPBEAR_SUCCESS on success, DROPBEAR_FAILURE on fail.
 * type should be set by the caller to specify the type to read, and
 * on return is set to the type read (useful when type = _ANY) */
int buf_get_pub_key(buffer *buf, sign_key *key, enum signkey_type *type) {

	char *ident;
	unsigned int len;
	enum signkey_type keytype;
	int ret = DROPBEAR_FAILURE;

	TRACE2(("enter buf_get_pub_key"))

	ident = buf_getstring(buf, &len);
	keytype = signkey_type_from_name(ident, len);
	m_free(ident);

	if (*type != DROPBEAR_SIGNKEY_ANY && *type != keytype) {
		TRACE(("buf_get_pub_key bad type - got %d, expected %d", keytype, *type))
		return DROPBEAR_FAILURE;
	}
	
	TRACE2(("buf_get_pub_key keytype is %d", keytype))

	*type = keytype;

	/* Rewind the buffer back before "ssh-rsa" etc */
	buf_decrpos(buf, len + 4);

#if DROPBEAR_DSS
	if (keytype == DROPBEAR_SIGNKEY_DSS) {
		dss_key_free(key->dsskey);
		key->dsskey = m_malloc(sizeof(*key->dsskey));
		ret = buf_get_dss_pub_key(buf, key->dsskey);
		if (ret == DROPBEAR_FAILURE) {
			dss_key_free(key->dsskey);
			key->dsskey = NULL;
		}
	}
#endif
#if DROPBEAR_RSA
	if (keytype == DROPBEAR_SIGNKEY_RSA) {
		rsa_key_free(key->rsakey);
		key->rsakey = m_malloc(sizeof(*key->rsakey));
		ret = buf_get_rsa_pub_key(buf, key->rsakey);
		if (ret == DROPBEAR_FAILURE) {
			rsa_key_free(key->rsakey);
			key->rsakey = NULL;
		}
	}
#endif
#if DROPBEAR_ECDSA
	if (signkey_is_ecdsa(keytype)
#if DROPBEAR_SK_ECDSA
		|| keytype == DROPBEAR_SIGNKEY_SK_ECDSA_NISTP256
#endif
	) {
		ecc_key **eck = (ecc_key**)signkey_key_ptr(key, keytype);
		if (eck) {
			if (*eck) {
				ecc_free(*eck);
				m_free(*eck);
				*eck = NULL;
			}
			*eck = buf_get_ecdsa_pub_key(buf);
			if (*eck) {
				ret = DROPBEAR_SUCCESS;
			}
		}
	}
#endif
#if DROPBEAR_ED25519
	if (keytype == DROPBEAR_SIGNKEY_ED25519
#if DROPBEAR_SK_ED25519
		|| keytype == DROPBEAR_SIGNKEY_SK_ED25519
#endif
    ) {
		ed25519_key_free(key->ed25519key);
		key->ed25519key = m_malloc(sizeof(*key->ed25519key));
		ret = buf_get_ed25519_pub_key(buf, key->ed25519key, keytype);
		if (ret == DROPBEAR_FAILURE) {
			m_free(key->ed25519key);
			key->ed25519key = NULL;
		}
	}
#endif

#if DROPBEAR_SK_ECDSA || DROPBEAR_SK_ED25519
	if (0
#if DROPBEAR_SK_ED25519
		|| keytype == DROPBEAR_SIGNKEY_SK_ED25519
#endif
#if DROPBEAR_SK_ECDSA
		|| keytype == DROPBEAR_SIGNKEY_SK_ECDSA_NISTP256
#endif
	) {
		key->sk_app = buf_getstring(buf, &key->sk_applen);
	}
#endif

	TRACE2(("leave buf_get_pub_key"))

	return ret;
}

/* returns DROPBEAR_SUCCESS on success, DROPBEAR_FAILURE on fail.
 * type should be set by the caller to specify the type to read, and
 * on return is set to the type read (useful when type = _ANY) */
int buf_get_priv_key(buffer *buf, sign_key *key, enum signkey_type *type) {

	char *ident;
	unsigned int len;
	enum signkey_type keytype;
	int ret = DROPBEAR_FAILURE;

	TRACE2(("enter buf_get_priv_key"))

	ident = buf_getstring(buf, &len);
	keytype = signkey_type_from_name(ident, len);
	m_free(ident);

	if (*type != DROPBEAR_SIGNKEY_ANY && *type != keytype) {
		TRACE(("wrong key type: %d %d", *type, keytype))
		return DROPBEAR_FAILURE;
	}

	*type = keytype;

	/* Rewind the buffer back before "ssh-rsa" etc */
	buf_decrpos(buf, len + 4);

#if DROPBEAR_DSS
	if (keytype == DROPBEAR_SIGNKEY_DSS) {
		dss_key_free(key->dsskey);
		key->dsskey = m_malloc(sizeof(*key->dsskey));
		ret = buf_get_dss_priv_key(buf, key->dsskey);
		if (ret == DROPBEAR_FAILURE) {
			dss_key_free(key->dsskey);
			key->dsskey = NULL;
		}
	}
#endif
#if DROPBEAR_RSA
	if (keytype == DROPBEAR_SIGNKEY_RSA) {
		rsa_key_free(key->rsakey);
		key->rsakey = m_malloc(sizeof(*key->rsakey));
		ret = buf_get_rsa_priv_key(buf, key->rsakey);
		if (ret == DROPBEAR_FAILURE) {
			rsa_key_free(key->rsakey);
			key->rsakey = NULL;
		}
	}
#endif
#if DROPBEAR_ECDSA
	if (signkey_is_ecdsa(keytype)) {
		ecc_key **eck = (ecc_key**)signkey_key_ptr(key, keytype);
		if (eck) {
			if (*eck) {
				ecc_free(*eck);
				m_free(*eck);
				*eck = NULL;
			}
			*eck = buf_get_ecdsa_priv_key(buf);
			if (*eck) {
				ret = DROPBEAR_SUCCESS;
			}
		}
	}
#endif
#if DROPBEAR_ED25519
	if (keytype == DROPBEAR_SIGNKEY_ED25519) {
		ed25519_key_free(key->ed25519key);
		key->ed25519key = m_malloc(sizeof(*key->ed25519key));
		ret = buf_get_ed25519_priv_key(buf, key->ed25519key);
		if (ret == DROPBEAR_FAILURE) {
			m_free(key->ed25519key);
			key->ed25519key = NULL;
		}
	}
#endif

	TRACE2(("leave buf_get_priv_key"))

	return ret;
	
}

/* type is either DROPBEAR_SIGNKEY_DSS or DROPBEAR_SIGNKEY_RSA */
void buf_put_pub_key(buffer* buf, sign_key *key, enum signkey_type type) {

	buffer *pubkeys;

	TRACE2(("enter buf_put_pub_key"))
	pubkeys = buf_new(MAX_PUBKEY_SIZE);
	
#if DROPBEAR_DSS
	if (type == DROPBEAR_SIGNKEY_DSS) {
		buf_put_dss_pub_key(pubkeys, key->dsskey);
	}
#endif
#if DROPBEAR_RSA
	if (type == DROPBEAR_SIGNKEY_RSA) {
		buf_put_rsa_pub_key(pubkeys, key->rsakey);
	}
#endif
#if DROPBEAR_ECDSA
	if (signkey_is_ecdsa(type)) {
		ecc_key **eck = (ecc_key**)signkey_key_ptr(key, type);
		if (eck && *eck) {
			buf_put_ecdsa_pub_key(pubkeys, *eck);
		}
	}
#endif
#if DROPBEAR_ED25519
	if (type == DROPBEAR_SIGNKEY_ED25519
#if DROPBEAR_SK_ED25519
		|| type == DROPBEAR_SIGNKEY_SK_ED25519
#endif
	) {
		buf_put_ed25519_pub_key(pubkeys, key->ed25519key);
	}
#endif
	if (pubkeys->len == 0) {
		dropbear_exit("Bad key types in buf_put_pub_key");
	}

	buf_putbufstring(buf, pubkeys);
	buf_free(pubkeys);
	TRACE2(("leave buf_put_pub_key"))
}

/* type is either DROPBEAR_SIGNKEY_DSS or DROPBEAR_SIGNKEY_RSA */
void buf_put_priv_key(buffer* buf, sign_key *key, enum signkey_type type) {

	TRACE(("enter buf_put_priv_key"))
	TRACE(("type is %d", type))

#if DROPBEAR_DSS
	if (type == DROPBEAR_SIGNKEY_DSS) {
		buf_put_dss_priv_key(buf, key->dsskey);
		TRACE(("leave buf_put_priv_key: dss done"))
		return;
	}
#endif
#if DROPBEAR_RSA
	if (type == DROPBEAR_SIGNKEY_RSA) {
		buf_put_rsa_priv_key(buf, key->rsakey);
		TRACE(("leave buf_put_priv_key: rsa done"))
		return;
	}
#endif
#if DROPBEAR_ECDSA
	if (signkey_is_ecdsa(type)) {
		ecc_key **eck = (ecc_key**)signkey_key_ptr(key, type);
		if (eck && *eck) {
			buf_put_ecdsa_priv_key(buf, *eck);
			TRACE(("leave buf_put_priv_key: ecdsa done"))
			return;
		}
	}
#endif
#if DROPBEAR_ED25519
	if (type == DROPBEAR_SIGNKEY_ED25519) {
		buf_put_ed25519_priv_key(buf, key->ed25519key);
		TRACE(("leave buf_put_priv_key: ed25519 done"))
		return;
	}
#endif
	dropbear_exit("Bad key types in put pub key");
}

void sign_key_free(sign_key *key) {

	TRACE2(("enter sign_key_free"))

#if DROPBEAR_DSS
	dss_key_free(key->dsskey);
	key->dsskey = NULL;
#endif
#if DROPBEAR_RSA
	rsa_key_free(key->rsakey);
	key->rsakey = NULL;
#endif
#if DROPBEAR_ECDSA
#if DROPBEAR_ECC_256
	if (key->ecckey256) {
		ecc_free(key->ecckey256);
		m_free(key->ecckey256);
		key->ecckey256 = NULL;
	}
#endif
#if DROPBEAR_ECC_384
	if (key->ecckey384) {
		ecc_free(key->ecckey384);
		m_free(key->ecckey384);
		key->ecckey384 = NULL;
	}
#endif
#if DROPBEAR_ECC_521
	if (key->ecckey521) {
		ecc_free(key->ecckey521);
		m_free(key->ecckey521);
		key->ecckey521 = NULL;
	}
#endif
#endif
#if DROPBEAR_ED25519
	ed25519_key_free(key->ed25519key);
	key->ed25519key = NULL;
#endif

	m_free(key->filename);
#if DROPBEAR_SK_ECDSA || DROPBEAR_SK_ED25519
	if (key->sk_app) {
		m_free(key->sk_app);
	}
#endif

	m_free(key);
	TRACE2(("leave sign_key_free"))
}

static char * sign_key_sha256_fingerprint(const unsigned char* keyblob,
		unsigned int keybloblen) {

	char * ret;
	hash_state hs;
	unsigned char hash[SHA256_HASH_SIZE];
	unsigned int b64chars, start;
	unsigned long b64size;
	const char *prefix = "SHA256:";
	int err;

	sha256_init(&hs);
	sha256_process(&hs, keyblob, keybloblen);
	sha256_done(&hs, hash);

	/* eg "SHA256:P9szN0L2ls6KxkVv7Bppv3asnZCn03rY7Msm/c8+ZgA"
	 * 256/6 = 42.66 => 43 base64 chars. OpenSSH discards
	 * base64 padding output. */
	start = strlen(prefix);
	b64chars = 43;
	/* space for discarded b64 padding and null terminator */
	b64size = b64chars + 4;
	ret = m_malloc(start + b64size);

	memcpy(ret, prefix, start);
	err = base64_encode(hash, SHA256_HASH_SIZE, &ret[start], &b64size);
	if (err != CRYPT_OK) {
		dropbear_exit("base64 failed");
	}
	ret[start + b64chars] = '\0';
	return ret;
}

/* This will return a freshly malloced string */
char * sign_key_fingerprint(const unsigned char* keyblob, unsigned int keybloblen) {
	return sign_key_sha256_fingerprint(keyblob, keybloblen);
}

void buf_put_sign(buffer* buf, sign_key *key, enum signature_type sigtype, 
	const buffer *data_buf) {
	buffer *sigblob = buf_new(MAX_PUBKEY_SIZE);
	enum signkey_type keytype = signkey_type_from_signature(sigtype);

#if DEBUG_TRACE > DROPBEAR_VERBOSE_LEVEL
	{
		const char* signame = signature_name_from_type(sigtype, NULL);
		TRACE(("buf_put_sign type %d %s", sigtype, signame));
	}
#endif


#if DROPBEAR_DSS
	if (keytype == DROPBEAR_SIGNKEY_DSS) {
		buf_put_dss_sign(sigblob, key->dsskey, data_buf);
	}
#endif
#if DROPBEAR_RSA
	if (keytype == DROPBEAR_SIGNKEY_RSA) {
		buf_put_rsa_sign(sigblob, key->rsakey, sigtype, data_buf);
	}
#endif
#if DROPBEAR_ECDSA
	if (signkey_is_ecdsa(keytype)) {
		ecc_key **eck = (ecc_key**)signkey_key_ptr(key, keytype);
		if (eck && *eck) {
			buf_put_ecdsa_sign(sigblob, *eck, data_buf);
		}
	}
#endif
#if DROPBEAR_ED25519
	if (keytype == DROPBEAR_SIGNKEY_ED25519) {
		buf_put_ed25519_sign(sigblob, key->ed25519key, data_buf);
	}
#endif
	if (sigblob->len == 0) {
		dropbear_exit("Non-matching signing type");
	}
	buf_putbufstring(buf, sigblob);
	buf_free(sigblob);

}

#if DROPBEAR_SIGNKEY_VERIFY

/* Return DROPBEAR_SUCCESS or DROPBEAR_FAILURE.
 * If FAILURE is returned, the position of
 * buf is undefined. If SUCCESS is returned, buf will be positioned after the
 * signature blob */
int buf_verify(buffer * buf, sign_key *key, enum signature_type expect_sigtype, const buffer *data_buf) {
	
	char *type_name = NULL;
	unsigned int type_name_len = 0;
	enum signature_type sigtype;
	enum signkey_type keytype;

	TRACE(("enter buf_verify"))

	buf_getint(buf); /* blob length */
	type_name = buf_getstring(buf, &type_name_len);
	sigtype = signature_type_from_name(type_name, type_name_len);
	m_free(type_name);

	if (expect_sigtype != sigtype) {
			dropbear_exit("Non-matching signing type");
	}

	keytype = signkey_type_from_signature(sigtype);
#if DROPBEAR_DSS
	if (keytype == DROPBEAR_SIGNKEY_DSS) {
		if (key->dsskey == NULL) {
			dropbear_exit("No DSS key to verify signature");
		}
		return buf_dss_verify(buf, key->dsskey, data_buf);
	}
#endif

#if DROPBEAR_RSA
	if (keytype == DROPBEAR_SIGNKEY_RSA) {
		if (key->rsakey == NULL) {
			dropbear_exit("No RSA key to verify signature");
		}
		return buf_rsa_verify(buf, key->rsakey, sigtype, data_buf);
	}
#endif
#if DROPBEAR_ECDSA
	if (signkey_is_ecdsa(keytype)) {
		ecc_key **eck = (ecc_key**)signkey_key_ptr(key, keytype);
		if (eck && *eck) {
			return buf_ecdsa_verify(buf, *eck, data_buf);
		}
	}
#endif
#if DROPBEAR_ED25519
	if (keytype == DROPBEAR_SIGNKEY_ED25519) {
		if (key->ed25519key == NULL) {
			dropbear_exit("No Ed25519 key to verify signature");
		}
		return buf_ed25519_verify(buf, key->ed25519key, data_buf);
	}
#endif
#if DROPBEAR_SK_ECDSA
	if (keytype == DROPBEAR_SIGNKEY_SK_ECDSA_NISTP256) {
		ecc_key **eck = (ecc_key**)signkey_key_ptr(key, keytype);
		if (eck && *eck) {
			return buf_sk_ecdsa_verify(buf, *eck, data_buf, key->sk_app, key->sk_applen, key->sk_flags_mask);
		}
	}
#endif
#if DROPBEAR_SK_ED25519
	if (keytype == DROPBEAR_SIGNKEY_SK_ED25519) {
		dropbear_ed25519_key **eck = (dropbear_ed25519_key**)signkey_key_ptr(key, keytype);
		if (eck && *eck) {
			return buf_sk_ed25519_verify(buf, *eck, data_buf, key->sk_app, key->sk_applen, key->sk_flags_mask);
		}
	}
#endif

	dropbear_exit("Non-matching signing type");
	return DROPBEAR_FAILURE;
}
#endif /* DROPBEAR_SIGNKEY_VERIFY */

#if DROPBEAR_KEY_LINES /* ie we're using authorized_keys or known_hosts */

/* Returns DROPBEAR_SUCCESS or DROPBEAR_FAILURE when given a buffer containing
 * a key, a key, and a type. The buffer is positioned at the start of the
 * base64 data, and contains no trailing data */
/* If fingerprint is non-NULL, it will be set to a malloc()ed fingerprint
   of the key if it is successfully decoded */
int cmp_base64_key(const unsigned char* keyblob, unsigned int keybloblen, 
					const unsigned char* algoname, unsigned int algolen, 
					const buffer * line, char ** fingerprint) {

	buffer * decodekey = NULL;
	int ret = DROPBEAR_FAILURE;
	unsigned int len, filealgolen;
	unsigned long decodekeylen;
	unsigned char* filealgo = NULL;

	/* now we have the actual data */
	len = line->len - line->pos;
	if (len == 0) {
		/* base64_decode doesn't like NULL argument */
		return DROPBEAR_FAILURE;
	}
	decodekeylen = len * 2; /* big to be safe */
	decodekey = buf_new(decodekeylen);

	if (base64_decode(buf_getptr(line, len), len,
				buf_getwriteptr(decodekey, decodekey->size),
				&decodekeylen) != CRYPT_OK) {
		TRACE(("checkpubkey: base64 decode failed"))
		goto out;
	}
	TRACE(("checkpubkey: base64_decode success"))
	buf_incrlen(decodekey, decodekeylen);
	
	if (fingerprint) {
		*fingerprint = sign_key_fingerprint(buf_getptr(decodekey, decodekeylen),
											decodekeylen);
	}
	
	/* compare the keys */
	if ( ( decodekeylen != keybloblen )
			|| memcmp( buf_getptr(decodekey, decodekey->len),
						keyblob, decodekey->len) != 0) {
		TRACE(("checkpubkey: compare failed"))
		goto out;
	}

	/* ... and also check that the algo specified and the algo in the key
	 * itself match */
	filealgolen = buf_getint(decodekey);
	filealgo = buf_getptr(decodekey, filealgolen);
	if (filealgolen != algolen || memcmp(filealgo, algoname, algolen) != 0) {
		TRACE(("checkpubkey: algo match failed")) 
		goto out;
	}

	/* All checks passed */
	ret = DROPBEAR_SUCCESS;

out:
	buf_free(decodekey);
	decodekey = NULL;
	return ret;
}
#endif

#if DROPBEAR_FUZZ
const char * const * fuzz_signkey_names = signkey_names;

#endif
