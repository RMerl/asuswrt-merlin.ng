#ifndef DROPBEAR_SK_ECDSA_H_
#define DROPBEAR_SK_ECDSA_H_

#include "includes.h"

#if DROPBEAR_SK_ECDSA

#include "buffer.h"
#include "signkey.h"

int buf_sk_ecdsa_verify(buffer *buf, const ecc_key *key, const buffer *data_buf,
			const char* app, unsigned int applen,
			unsigned char sk_flags_mask);

#endif

#endif /* DROPBEAR_SK_ECDSA_H_ */
