#include "includes.h"

#if DROPBEAR_SK_ECDSA

#include "dbutil.h"
#include "ecc.h"
#include "ecdsa.h"
#include "sk-ecdsa.h"
#include "ssh.h"

int buf_sk_ecdsa_verify(buffer *buf, const ecc_key *key, const buffer *data_buf, const char* app, unsigned int applen) {
	hash_state hs;
	unsigned char subhash[SHA256_HASH_SIZE];
	buffer *sk_buffer = NULL, *sig_buffer = NULL;
	unsigned char flags;
	unsigned int counter;
	int ret;

	TRACE(("buf_sk_ecdsa_verify"))

	/* from https://github.com/openssh/openssh-portable/blob/master/PROTOCOL.u2f */
	/* ecdsa signature to verify (r, s) */
	sig_buffer = buf_getbuf(buf);

	flags = buf_getbyte (buf);
	counter = buf_getint (buf);
	/* create the message to be signed */
	sk_buffer = buf_new (2*SHA256_HASH_SIZE+5);
	sha256_init (&hs);
	sha256_process (&hs, app, applen);
	sha256_done (&hs, subhash);
	buf_putbytes (sk_buffer, subhash, sizeof (subhash));
	buf_putbyte (sk_buffer, flags);
	buf_putint (sk_buffer, counter);
	sha256_init (&hs);
	sha256_process (&hs, data_buf->data, data_buf->len);
	sha256_done (&hs, subhash);
	buf_putbytes (sk_buffer, subhash, sizeof (subhash));

	ret = buf_ecdsa_verify(sig_buffer, key, sk_buffer);
	buf_free(sk_buffer);
	buf_free(sig_buffer);

	/* TODO: allow "no-touch-required" or "verify-required" authorized_keys options */
	if (!(flags & SSH_SK_USER_PRESENCE_REQD)) {
		if (ret == DROPBEAR_SUCCESS) {
			dropbear_log(LOG_WARNING, "Rejecting, user-presence not set");
		}
		ret = DROPBEAR_FAILURE;
	}

	TRACE(("leave buf_sk_ecdsa_verify, ret=%d", ret))
	return ret;
}

#endif /* DROPBEAR_SK_ECDSA */
