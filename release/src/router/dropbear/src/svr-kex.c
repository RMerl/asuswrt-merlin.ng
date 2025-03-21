/*
 * Dropbear - a SSH2 server
 * 
 * Copyright (c) 2002,2003 Matt Johnston
 * Copyright (c) 2004 by Mihnea Stoenescu
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
#include "algo.h"
#include "buffer.h"
#include "session.h"
#include "kex.h"
#include "ssh.h"
#include "packet.h"
#include "bignum.h"
#include "dbrandom.h"
#include "runopts.h"
#include "ecc.h"
#include "gensignkey.h"

static void send_msg_kexdh_reply(mp_int *dh_e, buffer *q_c);
#if DROPBEAR_EXT_INFO
static void send_msg_ext_info(void);
#endif

/* Handle a diffie-hellman key exchange initialisation. This involves
 * calculating a session key reply value, and corresponding hash. These
 * are carried out by send_msg_kexdh_reply(). recv_msg_kexdh_init() calls
 * that function, then brings the new keys into use */
void recv_msg_kexdh_init() {
	DEF_MP_INT(dh_e);
	buffer *q_c = NULL;

	TRACE(("enter recv_msg_kexdh_init"))
	if (!ses.kexstate.recvkexinit) {
		dropbear_exit("Premature kexdh_init message received");
	}

	switch (ses.newkeys->algo_kex->mode) {
#if DROPBEAR_NORMAL_DH
		case DROPBEAR_KEX_NORMAL_DH:
			m_mp_init(&dh_e);
			if (buf_getmpint(ses.payload, &dh_e) != DROPBEAR_SUCCESS) {
				dropbear_exit("Bad kex value");
			}
			break;
#endif
#if DROPBEAR_ECDH
		case DROPBEAR_KEX_ECDH:
#endif
#if DROPBEAR_CURVE25519
		case DROPBEAR_KEX_CURVE25519:
#endif
#if DROPBEAR_PQHYBRID
		case DROPBEAR_KEX_PQHYBRID:
#endif
#if DROPBEAR_ECDH || DROPBEAR_CURVE25519 || DROPBEAR_PQHYBRID
			q_c = buf_getstringbuf(ses.payload);
			break;
#endif
	}
	if (ses.payload->pos != ses.payload->len) {
		dropbear_exit("Bad kex value");
	}

	send_msg_kexdh_reply(&dh_e, q_c);

	mp_clear(&dh_e);
	if (q_c) {
		buf_free(q_c);
		q_c = NULL;
	}

	send_msg_newkeys();

#if DROPBEAR_EXT_INFO
	/* Only send it following the first newkeys */
	if (!ses.kexstate.donesecondkex && ses.allow_ext_info) {
		send_msg_ext_info();
	}
#endif

	ses.requirenext = SSH_MSG_NEWKEYS;
	TRACE(("leave recv_msg_kexdh_init"))
}


#if DROPBEAR_DELAY_HOSTKEY

static void svr_ensure_hostkey() {

	const char* fn = NULL;
	char *expand_fn = NULL;
	enum signkey_type type = ses.newkeys->algo_hostkey;
	void **hostkey = signkey_key_ptr(svr_opts.hostkey, type);
	int ret = DROPBEAR_FAILURE;

	if (hostkey && *hostkey) {
		return;
	}

	switch (type)
	{
#if DROPBEAR_RSA
		case DROPBEAR_SIGNKEY_RSA:
			fn = RSA_PRIV_FILENAME;
			break;
#endif
#if DROPBEAR_DSS
		case DROPBEAR_SIGNKEY_DSS:
			fn = DSS_PRIV_FILENAME;
			break;
#endif
#if DROPBEAR_ECDSA
		case DROPBEAR_SIGNKEY_ECDSA_NISTP256:
		case DROPBEAR_SIGNKEY_ECDSA_NISTP384:
		case DROPBEAR_SIGNKEY_ECDSA_NISTP521:
			fn = ECDSA_PRIV_FILENAME;
			break;
#endif
#if DROPBEAR_ED25519
		case DROPBEAR_SIGNKEY_ED25519:
			fn = ED25519_PRIV_FILENAME;
			break;
#endif
		default:
			dropbear_assert(0);
	}

	expand_fn = expand_homedir_path(fn);

	ret = readhostkey(expand_fn, svr_opts.hostkey, &type);
	if (ret == DROPBEAR_SUCCESS) {
		goto out;
	}

	if (signkey_generate(type, 0, expand_fn, 1) == DROPBEAR_FAILURE) {
		goto out;
	}
	
	/* Read what we just generated (or another process raced us) */
	ret = readhostkey(expand_fn, svr_opts.hostkey, &type);

	if (ret == DROPBEAR_SUCCESS) {
		char *fp = NULL;
		unsigned int len;
		buffer *key_buf = buf_new(MAX_PUBKEY_SIZE);
		buf_put_pub_key(key_buf, svr_opts.hostkey, type);
		buf_setpos(key_buf, 4);
		len = key_buf->len - key_buf->pos;
		fp = sign_key_fingerprint(buf_getptr(key_buf, len), len);
		dropbear_log(LOG_INFO, "Generated hostkey %s, fingerprint is %s",
			expand_fn, fp);
		m_free(fp);
		buf_free(key_buf);
	}

out:
	if (ret == DROPBEAR_FAILURE) {
		dropbear_exit("Couldn't read or generate hostkey %s", expand_fn);
	}
    m_free(expand_fn);
}
#endif
	
/* Generate our side of the diffie-hellman key exchange value (dh_f), and
 * calculate the session key using the diffie-hellman algorithm. Following
 * that, the session hash is calculated, and signed with RSA or DSS. The
 * result is sent to the client. 
 *
 * See the transport RFC4253 section 8 for details
 * or RFC5656 section 4 for elliptic curve variant. */
static void send_msg_kexdh_reply(mp_int *dh_e, buffer *q_c) {
	TRACE(("enter send_msg_kexdh_reply"))

	/* we can start creating the kexdh_reply packet */
	CHECKCLEARTOWRITE();

#if DROPBEAR_DELAY_HOSTKEY
	if (svr_opts.delay_hostkey)
	{
		svr_ensure_hostkey();
	}
#endif

#if DROPBEAR_FUZZ
	if (fuzz.fuzzing && fuzz.skip_kexmaths) {
		fuzz_fake_send_kexdh_reply();
		return;
	}
#endif

	buf_putbyte(ses.writepayload, SSH_MSG_KEXDH_REPLY);
	buf_put_pub_key(ses.writepayload, svr_opts.hostkey,
			ses.newkeys->algo_hostkey);

	switch (ses.newkeys->algo_kex->mode) {
#if DROPBEAR_NORMAL_DH
		case DROPBEAR_KEX_NORMAL_DH:
			{
			struct kex_dh_param * dh_param = gen_kexdh_param();
			kexdh_comb_key(dh_param, dh_e, svr_opts.hostkey);

			/* put f */
			buf_putmpint(ses.writepayload, &dh_param->pub);
			free_kexdh_param(dh_param);
			}
			break;
#endif
#if DROPBEAR_ECDH
		case DROPBEAR_KEX_ECDH:
			{
			struct kex_ecdh_param *ecdh_param = gen_kexecdh_param();
			kexecdh_comb_key(ecdh_param, q_c, svr_opts.hostkey);

			buf_put_ecc_raw_pubkey_string(ses.writepayload, &ecdh_param->key);
			free_kexecdh_param(ecdh_param);
			}
			break;
#endif
#if DROPBEAR_CURVE25519
		case DROPBEAR_KEX_CURVE25519:
			{
			struct kex_curve25519_param *param = gen_kexcurve25519_param();
			kexcurve25519_comb_key(param, q_c, svr_opts.hostkey);

			buf_putstring(ses.writepayload, param->pub, CURVE25519_LEN);
			free_kexcurve25519_param(param);
			}
			break;
#endif
#if DROPBEAR_PQHYBRID
		case DROPBEAR_KEX_PQHYBRID:
			{
			struct kex_pqhybrid_param *param = gen_kexpqhybrid_param();
			kexpqhybrid_comb_key(param, q_c, svr_opts.hostkey);

			buf_putbufstring(ses.writepayload, param->concat_public);
			free_kexpqhybrid_param(param);
			}
			break;
#endif
	}

	/* calc the signature */
	buf_put_sign(ses.writepayload, svr_opts.hostkey, 
			ses.newkeys->algo_signature, ses.hash);

	/* the SSH_MSG_KEXDH_REPLY is done */
	encrypt_packet();

	TRACE(("leave send_msg_kexdh_reply"))
}

#if DROPBEAR_EXT_INFO
/* Only used for server-sig-algs on the server side */
static void send_msg_ext_info(void) {
	TRACE(("enter send_msg_ext_info"))

	buf_putbyte(ses.writepayload, SSH_MSG_EXT_INFO);
	/* nr-extensions */
	buf_putint(ses.writepayload, 1);

	buf_putstring(ses.writepayload, SSH_SERVER_SIG_ALGS, strlen(SSH_SERVER_SIG_ALGS));
	buf_put_algolist_all(ses.writepayload, sigalgs, 1);
	
	encrypt_packet();

	TRACE(("leave send_msg_ext_info"))
}
#endif
