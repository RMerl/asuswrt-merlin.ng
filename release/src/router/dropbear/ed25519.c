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

/* Perform Ed25519 operations on data, including reading keys, signing and
 * verification. */

#include "includes.h"
#include "dbutil.h"
#include "buffer.h"
#include "ssh.h"
#include "curve25519.h"
#include "ed25519.h"

#if DROPBEAR_ED25519

/* Load a public ed25519 key from a buffer, initialising the values.
 * The key will have the same format as buf_put_ed25519_key.
 * These should be freed with ed25519_key_free.
 * Returns DROPBEAR_SUCCESS or DROPBEAR_FAILURE */
int buf_get_ed25519_pub_key(buffer *buf, dropbear_ed25519_key *key) {

	unsigned int len;

	TRACE(("enter buf_get_ed25519_pub_key"))
	dropbear_assert(key != NULL);

	buf_incrpos(buf, 4+SSH_SIGNKEY_ED25519_LEN); /* int + "ssh-ed25519" */

	len = buf_getint(buf);
	if (len != CURVE25519_LEN || buf->len - buf->pos < len) {
		TRACE(("leave buf_get_ed25519_pub_key: failure"))
		return DROPBEAR_FAILURE;
	}

	m_burn(key->priv, CURVE25519_LEN);
	memcpy(key->pub, buf_getptr(buf, CURVE25519_LEN), CURVE25519_LEN);
	buf_incrpos(buf, CURVE25519_LEN);

	TRACE(("leave buf_get_ed25519_pub_key: success"))
	return DROPBEAR_SUCCESS;
}

/* Same as buf_get_ed25519_pub_key, but reads private key at the end.
 * Loads a public and private ed25519 key from a buffer
 * Returns DROPBEAR_SUCCESS or DROPBEAR_FAILURE */
int buf_get_ed25519_priv_key(buffer *buf, dropbear_ed25519_key *key) {

	unsigned int len;

	TRACE(("enter buf_get_ed25519_priv_key"))
	dropbear_assert(key != NULL);

	buf_incrpos(buf, 4+SSH_SIGNKEY_ED25519_LEN); /* int + "ssh-ed25519" */

	len = buf_getint(buf);
	if (len != CURVE25519_LEN*2 || buf->len - buf->pos < len) {
		TRACE(("leave buf_get_ed25519_priv_key: failure"))
		return DROPBEAR_FAILURE;
	}

	memcpy(key->priv, buf_getptr(buf, CURVE25519_LEN), CURVE25519_LEN);
	buf_incrpos(buf, CURVE25519_LEN);
	memcpy(key->pub, buf_getptr(buf, CURVE25519_LEN), CURVE25519_LEN);
	buf_incrpos(buf, CURVE25519_LEN);

	TRACE(("leave buf_get_ed25519_priv_key: success"))
	return DROPBEAR_SUCCESS;
}

/* Clear and free the memory used by a public or private key */
void ed25519_key_free(dropbear_ed25519_key *key) {

	TRACE2(("enter ed25519_key_free"))

	if (key == NULL) {
		TRACE2(("leave ed25519_key_free: key == NULL"))
		return;
	}
	m_burn(key->priv, CURVE25519_LEN);
	m_free(key);

	TRACE2(("leave ed25519_key_free"))
}

/* Put the public ed25519 key into the buffer in the required format */
void buf_put_ed25519_pub_key(buffer *buf, const dropbear_ed25519_key *key) {

	TRACE(("enter buf_put_ed25519_pub_key"))
	dropbear_assert(key != NULL);

	buf_putstring(buf, SSH_SIGNKEY_ED25519, SSH_SIGNKEY_ED25519_LEN);
	buf_putstring(buf, key->pub, CURVE25519_LEN);

	TRACE(("leave buf_put_ed25519_pub_key"))
}

/* Put the public and private ed25519 key into the buffer in the required format */
void buf_put_ed25519_priv_key(buffer *buf, const dropbear_ed25519_key *key) {

	TRACE(("enter buf_put_ed25519_priv_key"))
	dropbear_assert(key != NULL);

	buf_putstring(buf, SSH_SIGNKEY_ED25519, SSH_SIGNKEY_ED25519_LEN);
	buf_putint(buf, CURVE25519_LEN*2);
	buf_putbytes(buf, key->priv, CURVE25519_LEN);
	buf_putbytes(buf, key->pub, CURVE25519_LEN);

	TRACE(("leave buf_put_ed25519_priv_key"))
}

/* Sign the data presented with key, writing the signature contents
 * to the buffer */
void buf_put_ed25519_sign(buffer* buf, const dropbear_ed25519_key *key, const buffer *data_buf) {

	unsigned char s[64];
	unsigned long slen = sizeof(s);

	TRACE(("enter buf_put_ed25519_sign"))
	dropbear_assert(key != NULL);

	dropbear_ed25519_sign(data_buf->data, data_buf->len, s, &slen, key->priv, key->pub);
	buf_putstring(buf, SSH_SIGNKEY_ED25519, SSH_SIGNKEY_ED25519_LEN);
	buf_putstring(buf, s, slen);

	TRACE(("leave buf_put_ed25519_sign"))
}

#if DROPBEAR_SIGNKEY_VERIFY
/* Verify a signature in buf, made on data by the key given.
 * Returns DROPBEAR_SUCCESS or DROPBEAR_FAILURE */
int buf_ed25519_verify(buffer *buf, const dropbear_ed25519_key *key, const buffer *data_buf) {

	int ret = DROPBEAR_FAILURE;
	unsigned char *s;
	unsigned long slen;

	TRACE(("enter buf_ed25519_verify"))
	dropbear_assert(key != NULL);

	slen = buf_getint(buf);
	if (slen != 64 || buf->len - buf->pos < slen) {
		TRACE(("leave buf_ed25519_verify: bad size"))
		goto out;
	}
	s = buf_getptr(buf, slen);

	if (dropbear_ed25519_verify(data_buf->data, data_buf->len,
				    s, slen, key->pub) == 0) {
		/* signature is valid */
		TRACE(("leave buf_ed25519_verify: success!"))
		ret = DROPBEAR_SUCCESS;
	}

out:
	TRACE(("leave buf_ed25519_verify: ret %d", ret))
	return ret;
}

#endif /* DROPBEAR_SIGNKEY_VERIFY */

#endif /* DROPBEAR_ED25519 */
