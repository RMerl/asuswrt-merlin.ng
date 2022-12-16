#ifndef DROPBEAR_SK_ED25519_H_
#define DROPBEAR_SK_ED25519_H_

#include "includes.h"

#if DROPBEAR_SK_ED25519

#include "buffer.h"
#include "ed25519.h"

int buf_sk_ed25519_verify(buffer *buf, const dropbear_ed25519_key *key, const buffer *data_buf,
			const char* app, unsigned int applen,
			unsigned char sk_flags_mask);

#endif

#endif /* DROPBEAR_SK_ED25519_H_ */
