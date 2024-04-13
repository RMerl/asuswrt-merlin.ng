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

#ifndef DROPBEAR_SIGNKEY_H_
#define DROPBEAR_SIGNKEY_H_

#include "buffer.h"

/* Forward declarations */
struct dropbear_DSS_Key;
struct dropbear_RSA_Key;
struct dropbear_ED25519_Key;

/* Must match with signature_type below */
enum signkey_type {
#if DROPBEAR_RSA
	DROPBEAR_SIGNKEY_RSA,
#endif
#if DROPBEAR_DSS
	DROPBEAR_SIGNKEY_DSS,
#endif
#if DROPBEAR_ECDSA
	DROPBEAR_SIGNKEY_ECDSA_NISTP256,
	DROPBEAR_SIGNKEY_ECDSA_NISTP384,
	DROPBEAR_SIGNKEY_ECDSA_NISTP521,
#if DROPBEAR_SK_ECDSA
	DROPBEAR_SIGNKEY_SK_ECDSA_NISTP256,
#endif /* DROPBEAR_SK_ECDSA */
#endif /* DROPBEAR_ECDSA */
#if DROPBEAR_ED25519
	DROPBEAR_SIGNKEY_ED25519,
#if DROPBEAR_SK_ED25519
	DROPBEAR_SIGNKEY_SK_ED25519,
#endif
#endif
	DROPBEAR_SIGNKEY_NUM_NAMED,
	DROPBEAR_SIGNKEY_ECDSA_KEYGEN = 70, /* just "ecdsa" for keygen */
	DROPBEAR_SIGNKEY_ANY = 80,
	DROPBEAR_SIGNKEY_NONE = 90,
};

/* Must match with signkey_type above, apart from rsa */
enum signature_type {
#if DROPBEAR_DSS
	DROPBEAR_SIGNATURE_DSS = DROPBEAR_SIGNKEY_DSS,
#endif
#if DROPBEAR_ECDSA
	DROPBEAR_SIGNATURE_ECDSA_NISTP256 = DROPBEAR_SIGNKEY_ECDSA_NISTP256,
	DROPBEAR_SIGNATURE_ECDSA_NISTP384 = DROPBEAR_SIGNKEY_ECDSA_NISTP384,
	DROPBEAR_SIGNATURE_ECDSA_NISTP521 = DROPBEAR_SIGNKEY_ECDSA_NISTP521,
#if DROPBEAR_SK_ECDSA
	DROPBEAR_SIGNATURE_SK_ECDSA_NISTP256 = DROPBEAR_SIGNKEY_SK_ECDSA_NISTP256,
#endif /* DROPBEAR_SK_ECDSA */
#endif /* DROPBEAR_ECDSA */
#if DROPBEAR_ED25519
	DROPBEAR_SIGNATURE_ED25519 = DROPBEAR_SIGNKEY_ED25519,
#if DROPBEAR_SK_ED25519
	DROPBEAR_SIGNATURE_SK_ED25519 = DROPBEAR_SIGNKEY_SK_ED25519,
#endif
#endif
#if DROPBEAR_RSA
#if DROPBEAR_RSA_SHA1
	DROPBEAR_SIGNATURE_RSA_SHA1 = 100, /* ssh-rsa signature (sha1) */
#endif
#if DROPBEAR_RSA_SHA256
	DROPBEAR_SIGNATURE_RSA_SHA256 = 101, /* rsa-sha2-256 signature. has a ssh-rsa key */
#endif
#endif /* DROPBEAR_RSA */
	DROPBEAR_SIGNATURE_NONE = DROPBEAR_SIGNKEY_NONE,
};


/* Sources for signing keys */
typedef enum {
	SIGNKEY_SOURCE_RAW_FILE,
	SIGNKEY_SOURCE_AGENT,
	SIGNKEY_SOURCE_INVALID,	
} signkey_source;

struct SIGN_key {

	enum signkey_type type;
	signkey_source source;
	char *filename;

#if DROPBEAR_DSS
	struct dropbear_DSS_Key * dsskey;
#endif
#if DROPBEAR_RSA
	struct dropbear_RSA_Key * rsakey;
#endif
#if DROPBEAR_ECDSA
#if DROPBEAR_ECC_256
	ecc_key * ecckey256;
#endif
#if DROPBEAR_ECC_384
	ecc_key * ecckey384;
#endif
#if DROPBEAR_ECC_521
	ecc_key * ecckey521;
#endif
#endif
#if DROPBEAR_ED25519
	struct dropbear_ED25519_Key * ed25519key;
#endif

#if DROPBEAR_SK_ECDSA || DROPBEAR_SK_ED25519
	/* application ID for U2F/FIDO key types, a malloced string */
	char * sk_app;
	unsigned int sk_applen;
	unsigned char sk_flags_mask;
#endif
};

typedef struct SIGN_key sign_key;

sign_key * new_sign_key(void);
const char* signkey_name_from_type(enum signkey_type type, unsigned int *namelen);
enum signkey_type signkey_type_from_name(const char* name, unsigned int namelen);
const char* signature_name_from_type(enum signature_type type, unsigned int *namelen);
enum signature_type signature_type_from_name(const char* name, unsigned int namelen);
enum signkey_type signkey_type_from_signature(enum signature_type sigtype);
enum signature_type signature_type_from_signkey(enum signkey_type keytype);

int buf_get_pub_key(buffer *buf, sign_key *key, enum signkey_type *type);
int buf_get_priv_key(buffer* buf, sign_key *key, enum signkey_type *type);
void buf_put_pub_key(buffer* buf, sign_key *key, enum signkey_type type);
void buf_put_priv_key(buffer* buf, sign_key *key, enum signkey_type type);
void sign_key_free(sign_key *key);
void buf_put_sign(buffer* buf, sign_key *key, enum signature_type sigtype, const buffer *data_buf);
#if DROPBEAR_SIGNKEY_VERIFY
int buf_verify(buffer * buf, sign_key *key, enum signature_type expect_sigtype, const buffer *data_buf);
int sk_buf_verify(buffer * buf, sign_key *key, enum signature_type expect_sigtype, const buffer *data_buf, char* app, unsigned int applen);
char * sign_key_fingerprint(const unsigned char* keyblob, unsigned int keybloblen);
#endif
int cmp_base64_key(const unsigned char* keyblob, unsigned int keybloblen, 
					const unsigned char* algoname, unsigned int algolen, 
					const buffer * line, char ** fingerprint);

void** signkey_key_ptr(sign_key *key, enum signkey_type type);

#endif /* DROPBEAR_SIGNKEY_H_ */
