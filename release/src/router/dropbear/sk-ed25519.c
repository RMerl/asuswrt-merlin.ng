#include "includes.h"

#if DROPBEAR_SK_ED25519

#include "dbutil.h"
#include "buffer.h"
#include "curve25519.h"
#include "ed25519.h"
#include "ssh.h"

int buf_sk_ed25519_verify(buffer *buf, const dropbear_ed25519_key *key, const buffer *data_buf, const char* app, unsigned int applen) {

	int ret = DROPBEAR_FAILURE;
	unsigned char *s;
	unsigned long slen;
	hash_state hs;
	unsigned char hash[SHA256_HASH_SIZE];
	buffer *sk_buffer = NULL;
	unsigned char flags;
	unsigned int counter;

	TRACE(("enter buf_sk_ed25519_verify"))
	dropbear_assert(key != NULL);

	slen = buf_getint(buf);
	if (slen != 64 || buf->len - buf->pos < slen) {
		TRACE(("leave buf_sk_ed25519_verify: bad size"))
		goto out;
	}
	s = buf_getptr(buf, slen);
	buf_incrpos(buf, slen);

	flags = buf_getbyte (buf);
	counter = buf_getint (buf);
	/* create the message to be signed */
	sk_buffer = buf_new (2*SHA256_HASH_SIZE+5);
	sha256_init (&hs);
	sha256_process (&hs, app, applen);
	sha256_done (&hs, hash);
	buf_putbytes (sk_buffer, hash, sizeof (hash));
	buf_putbyte (sk_buffer, flags);
	buf_putint (sk_buffer, counter);
	sha256_init (&hs);
	sha256_process (&hs, data_buf->data, data_buf->len);
	sha256_done (&hs, hash);
	buf_putbytes (sk_buffer, hash, sizeof (hash));

	if (dropbear_ed25519_verify(sk_buffer->data, sk_buffer->len,
				    s, slen, key->pub) == 0) {
		/* signature is valid */
		TRACE(("leave buf_sk_ed25519_verify: success!"))
		ret = DROPBEAR_SUCCESS;
	}

	/* TODO: allow "no-touch-required" or "verify-required" authorized_keys options */
	if (!(flags & SSH_SK_USER_PRESENCE_REQD)) {
		if (ret == DROPBEAR_SUCCESS) {
			dropbear_log(LOG_WARNING, "Rejecting, user-presence not set");
		}
		ret = DROPBEAR_FAILURE;
	}
out:
	buf_free(sk_buffer);
	TRACE(("leave buf_sk_ed25519_verify: ret %d", ret))
	return ret;
}

#endif /* DROPBEAR_SK_ED25519 */
