/*
 *
 *  Embedded Linux library
 *
 *  Copyright (C) 2015  Intel Corporation. All rights reserved.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#define _GNU_SOURCE
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <alloca.h>

#include "util.h"
#include "cipher.h"
#include "private.h"
#include "random.h"
#include "missing.h"

#ifndef HAVE_LINUX_IF_ALG_H
#ifndef HAVE_LINUX_TYPES_H
typedef uint8_t __u8;
typedef uint16_t __u16;
typedef uint32_t __u32;
#else
#include <linux/types.h>
#endif

#ifndef AF_ALG
#define AF_ALG	38
#define PF_ALG	AF_ALG
#endif

struct sockaddr_alg {
	__u16	salg_family;
	__u8	salg_type[14];
	__u32	salg_feat;
	__u32	salg_mask;
	__u8	salg_name[64];
};

struct af_alg_iv {
	__u32   ivlen;
	__u8    iv[0];
};

/* Socket options */
#define ALG_SET_KEY	1
#define ALG_SET_IV	2
#define ALG_SET_OP	3

/* Operations */
#define ALG_OP_DECRYPT	0
#define ALG_OP_ENCRYPT	1
#else
#include <linux/if_alg.h>
#endif

#ifndef SOL_ALG
#define SOL_ALG 279
#endif

#ifndef ALG_SET_AEAD_ASSOCLEN
#define ALG_SET_AEAD_ASSOCLEN	4
#endif

#ifndef ALG_SET_AEAD_AUTHSIZE
#define ALG_SET_AEAD_AUTHSIZE	5
#endif

#define is_valid_type(type)  ((type) <= L_CIPHER_RC2_CBC)

static uint32_t supported_ciphers;
static uint32_t supported_aead_ciphers;

struct l_cipher {
	int type;
	const struct local_impl *local;
	union {
		int sk;
		void *local_data;
	};
};

struct l_aead_cipher {
	int type;
	int sk;
};

struct local_impl {
	void *(*cipher_new)(enum l_cipher_type,
				const void *key, size_t key_length);
	void (*cipher_free)(void *data);
	bool (*set_iv)(void *data, const uint8_t *iv, size_t iv_length);
	ssize_t (*operate)(void *data, __u32 operation,
				const struct iovec *in, size_t in_cnt,
				const struct iovec *out, size_t out_cnt);
};

static int create_alg(const char *alg_type, const char *alg_name,
			const void *key, size_t key_length, size_t tag_length)
{
	struct sockaddr_alg salg;
	int sk;
	int ret;

	sk = socket(PF_ALG, SOCK_SEQPACKET | SOCK_CLOEXEC, 0);
	if (sk < 0)
		return -errno;

	memset(&salg, 0, sizeof(salg));
	salg.salg_family = AF_ALG;
	strcpy((char *) salg.salg_type, alg_type);
	strcpy((char *) salg.salg_name, alg_name);

	if (bind(sk, (struct sockaddr *) &salg, sizeof(salg)) < 0) {
		close(sk);
		return -1;
	}

	if (setsockopt(sk, SOL_ALG, ALG_SET_KEY, key, key_length) < 0) {
		close(sk);
		return -1;
	}

	if (tag_length && setsockopt(sk, SOL_ALG, ALG_SET_AEAD_AUTHSIZE, NULL,
					tag_length)) {
		close(sk);
		return -1;
	}

	ret = accept4(sk, NULL, 0, SOCK_CLOEXEC);
	close(sk);

	return ret;
}

static const char *cipher_type_to_name(enum l_cipher_type type)
{
	switch (type) {
	case L_CIPHER_AES:
		return "ecb(aes)";
	case L_CIPHER_AES_CBC:
		return "cbc(aes)";
	case L_CIPHER_AES_CTR:
		return "ctr(aes)";
	case L_CIPHER_ARC4:
		return NULL;
	case L_CIPHER_DES:
		return "ecb(des)";
	case L_CIPHER_DES_CBC:
		return "cbc(des)";
	case L_CIPHER_DES3_EDE_CBC:
		return "cbc(des3_ede)";
	case L_CIPHER_RC2_CBC:
		return NULL;
	}

	return NULL;
}

static const struct local_impl local_arc4;
static const struct local_impl local_rc2_cbc;

static const struct local_impl *local_impl_ciphers[] = {
	[L_CIPHER_ARC4] = &local_arc4,
	[L_CIPHER_RC2_CBC] = &local_rc2_cbc,
};

#define HAVE_LOCAL_IMPLEMENTATION(type)			\
	((type) < L_ARRAY_SIZE(local_impl_ciphers) &&	\
	 local_impl_ciphers[(type)])

LIB_EXPORT struct l_cipher *l_cipher_new(enum l_cipher_type type,
						const void *key,
						size_t key_length)
{
	struct l_cipher *cipher;
	const char *uninitialized_var(alg_name);

	if (unlikely(!key))
		return NULL;

	if (!is_valid_type(type))
		return NULL;

	cipher = l_new(struct l_cipher, 1);
	cipher->type = type;
	alg_name = cipher_type_to_name(type);

	if (HAVE_LOCAL_IMPLEMENTATION(type)) {
		cipher->local = local_impl_ciphers[type];
		cipher->local_data = cipher->local->cipher_new(type,
							key, key_length);

		if (!cipher->local_data)
			goto error_free;

		return cipher;
	}

	cipher->sk = create_alg("skcipher", alg_name, key, key_length, 0);
	if (cipher->sk < 0)
		goto error_free;

	return cipher;

error_free:
	l_free(cipher);
	return NULL;
}

static const char *aead_cipher_type_to_name(enum l_aead_cipher_type type)
{
	switch (type) {
	case L_AEAD_CIPHER_AES_CCM:
		return "ccm(aes)";
	case L_AEAD_CIPHER_AES_GCM:
		return "gcm(aes)";
	}

	return NULL;
}

LIB_EXPORT struct l_aead_cipher *l_aead_cipher_new(enum l_aead_cipher_type type,
							const void *key,
							size_t key_length,
							size_t tag_length)
{
	struct l_aead_cipher *cipher;
	const char *alg_name;

	if (unlikely(!key))
		return NULL;

	if (type != L_AEAD_CIPHER_AES_CCM && type != L_AEAD_CIPHER_AES_GCM)
		return NULL;

	cipher = l_new(struct l_aead_cipher, 1);
	cipher->type = type;
	alg_name = aead_cipher_type_to_name(type);

	cipher->sk = create_alg("aead", alg_name, key, key_length, tag_length);
	if (cipher->sk >= 0)
		return cipher;

	l_free(cipher);
	return NULL;
}

LIB_EXPORT void l_cipher_free(struct l_cipher *cipher)
{
	if (unlikely(!cipher))
		return;

	if (cipher->local)
		cipher->local->cipher_free(cipher->local_data);
	else
		close(cipher->sk);

	l_free(cipher);
}

LIB_EXPORT void l_aead_cipher_free(struct l_aead_cipher *cipher)
{
	if (unlikely(!cipher))
		return;

	close(cipher->sk);

	l_free(cipher);
}

static ssize_t operate_cipher(int sk, __u32 operation,
				const void *in, size_t in_len,
				const void *ad, size_t ad_len,
				const void *iv, size_t iv_len,
				void *out, size_t out_len)
{
	char *c_msg_buf;
	size_t c_msg_size;
	struct msghdr msg;
	struct cmsghdr *c_msg;
	struct iovec iov[2];
	ssize_t result;

	c_msg_size = CMSG_SPACE(sizeof(operation));
	c_msg_size += ad_len ? CMSG_SPACE(sizeof(uint32_t)) : 0;
	c_msg_size += iv_len ?
		CMSG_SPACE(sizeof(struct af_alg_iv) + iv_len) : 0;

	c_msg_buf = alloca(c_msg_size);

	memset(c_msg_buf, 0, c_msg_size);
	memset(&msg, 0, sizeof(msg));

	msg.msg_iov = iov;

	msg.msg_control = c_msg_buf;
	msg.msg_controllen = c_msg_size;

	c_msg = CMSG_FIRSTHDR(&msg);
	c_msg->cmsg_level = SOL_ALG;
	c_msg->cmsg_type = ALG_SET_OP;
	c_msg->cmsg_len = CMSG_LEN(sizeof(operation));
	memcpy(CMSG_DATA(c_msg), &operation, sizeof(operation));

	if (ad_len) {
		uint32_t *ad_data;

		c_msg = CMSG_NXTHDR(&msg, c_msg);
		c_msg->cmsg_level = SOL_ALG;
		c_msg->cmsg_type = ALG_SET_AEAD_ASSOCLEN;
		c_msg->cmsg_len = CMSG_LEN(sizeof(*ad_data));
		ad_data = (void *) CMSG_DATA(c_msg);
		*ad_data = ad_len;

		iov[0].iov_base = (void *) ad;
		iov[0].iov_len = ad_len;
		iov[1].iov_base = (void *) in;
		iov[1].iov_len = in_len;
		msg.msg_iovlen = 2;
	} else {
		iov[0].iov_base = (void *) in;
		iov[0].iov_len = in_len;
		msg.msg_iovlen = 1;
	}

	if (iv_len) {
		struct af_alg_iv *algiv;

		c_msg = CMSG_NXTHDR(&msg, c_msg);
		c_msg->cmsg_level = SOL_ALG;
		c_msg->cmsg_type = ALG_SET_IV;
		c_msg->cmsg_len = CMSG_LEN(sizeof(*algiv) + iv_len);

		algiv = (void *)CMSG_DATA(c_msg);
		algiv->ivlen = iv_len;
		memcpy(algiv->iv, iv, iv_len);
	}

	result = sendmsg(sk, &msg, 0);
	if (result < 0)
		return -errno;

	if (ad_len) {
		/*
		 * When AEAD additional data is passed to sendmsg() for
		 * use in computing the tag, those bytes also appear at
		 * the beginning of the encrypt or decrypt results.  Rather
		 * than force the caller to pad their result buffer with
		 * the correct number of bytes for the additional data,
		 * the necessary space is allocated here and then the
		 * duplicate AAD is discarded.
		 */
		iov[0].iov_base = l_malloc(ad_len);
		iov[0].iov_len = ad_len;
		iov[1].iov_base = (void *) out;
		iov[1].iov_len = out_len;
		msg.msg_iovlen = 2;

		msg.msg_control = NULL;
		msg.msg_controllen = 0;

		result = recvmsg(sk, &msg, 0);

		if (result >= (ssize_t) ad_len)
			result -= ad_len;
		else if (result > 0)
			result = 0;

		l_free(iov[0].iov_base);
	} else {
		result = read(sk, out, out_len);
	}

	if (result < 0)
		return -errno;

	return result;
}

static ssize_t operate_cipherv(int sk, __u32 operation,
				const struct iovec *in, size_t in_cnt,
				const struct iovec *out, size_t out_cnt)
{
	char *c_msg_buf;
	size_t c_msg_size;
	struct msghdr msg;
	struct cmsghdr *c_msg;
	ssize_t result;

	c_msg_size = CMSG_SPACE(sizeof(operation));
	c_msg_buf = alloca(c_msg_size);

	memset(c_msg_buf, 0, c_msg_size);
	memset(&msg, 0, sizeof(msg));

	msg.msg_iov = (struct iovec *) in;
	msg.msg_iovlen = in_cnt;

	msg.msg_control = c_msg_buf;
	msg.msg_controllen = c_msg_size;

	c_msg = CMSG_FIRSTHDR(&msg);
	c_msg->cmsg_level = SOL_ALG;
	c_msg->cmsg_type = ALG_SET_OP;
	c_msg->cmsg_len = CMSG_LEN(sizeof(operation));
	memcpy(CMSG_DATA(c_msg), &operation, sizeof(operation));

	result = sendmsg(sk, &msg, 0);
	if (result < 0)
		return -errno;

	result = readv(sk, out, out_cnt);

	if (result < 0)
		return -errno;

	return result;
}

LIB_EXPORT bool l_cipher_encrypt(struct l_cipher *cipher,
					const void *in, void *out, size_t len)
{
	if (unlikely(!cipher))
		return false;

	if (unlikely(!in) || unlikely(!out))
		return false;

	if (cipher->local) {
		struct iovec in_iov = { (void *) in, len };
		struct iovec out_iov = { out, len };

		return cipher->local->operate(cipher->local_data,
						ALG_OP_ENCRYPT,
						&in_iov, 1, &out_iov, 1) >= 0;
	}

	return operate_cipher(cipher->sk, ALG_OP_ENCRYPT, in, len,
				NULL, 0, NULL, 0, out, len) >= 0;
}

LIB_EXPORT bool l_cipher_encryptv(struct l_cipher *cipher,
					const struct iovec *in, size_t in_cnt,
					const struct iovec *out, size_t out_cnt)
{
	if (unlikely(!cipher))
		return false;

	if (unlikely(!in) || unlikely(!out))
		return false;

	if (cipher->local)
		return cipher->local->operate(cipher->local_data,
						ALG_OP_ENCRYPT,
						in, in_cnt, out, out_cnt) >= 0;

	return operate_cipherv(cipher->sk, ALG_OP_ENCRYPT, in, in_cnt,
				out, out_cnt) >= 0;
}

LIB_EXPORT bool l_cipher_decrypt(struct l_cipher *cipher,
					const void *in, void *out, size_t len)
{
	if (unlikely(!cipher))
		return false;

	if (unlikely(!in) || unlikely(!out))
		return false;

	if (cipher->local) {
		struct iovec in_iov = { (void *) in, len };
		struct iovec out_iov = { out, len };

		return cipher->local->operate(cipher->local_data,
						ALG_OP_DECRYPT,
						&in_iov, 1, &out_iov, 1) >= 0;
	}

	return operate_cipher(cipher->sk, ALG_OP_DECRYPT, in, len,
				NULL, 0, NULL, 0, out, len) >= 0;
}

LIB_EXPORT bool l_cipher_decryptv(struct l_cipher *cipher,
					const struct iovec *in, size_t in_cnt,
					const struct iovec *out, size_t out_cnt)
{
	if (unlikely(!cipher))
		return false;

	if (unlikely(!in) || unlikely(!out))
		return false;

	if (cipher->local)
		return cipher->local->operate(cipher->local_data,
						ALG_OP_DECRYPT,
						in, in_cnt, out, out_cnt) >= 0;

	return operate_cipherv(cipher->sk, ALG_OP_DECRYPT, in, in_cnt,
				out, out_cnt) >= 0;
}

LIB_EXPORT bool l_cipher_set_iv(struct l_cipher *cipher, const uint8_t *iv,
				size_t iv_length)
{
	char c_msg_buf[CMSG_SPACE(4 + iv_length)];
	struct msghdr msg;
	struct cmsghdr *c_msg;
	uint32_t len = iv_length;

	if (unlikely(!cipher))
		return false;

	if (cipher->local) {
		if (!cipher->local->set_iv)
			return false;

		return cipher->local->set_iv(cipher->local_data, iv, iv_length);
	}

	memset(&c_msg_buf, 0, sizeof(c_msg_buf));
	memset(&msg, 0, sizeof(struct msghdr));

	msg.msg_control = c_msg_buf;
	msg.msg_controllen = sizeof(c_msg_buf);

	c_msg = CMSG_FIRSTHDR(&msg);
	c_msg->cmsg_level = SOL_ALG;
	c_msg->cmsg_type = ALG_SET_IV;
	c_msg->cmsg_len = CMSG_LEN(4 + iv_length);
	memcpy(CMSG_DATA(c_msg) + 0, &len, 4);
	memcpy(CMSG_DATA(c_msg) + 4, iv, iv_length);

	msg.msg_iov = NULL;
	msg.msg_iovlen = 0;

	if (sendmsg(cipher->sk, &msg, MSG_MORE) < 0)
		return false;

	return true;
}

#define CCM_IV_SIZE 16

static size_t l_aead_cipher_get_ivlen(struct l_aead_cipher *cipher)
{
	switch (cipher->type) {
	case L_AEAD_CIPHER_AES_CCM:
		return CCM_IV_SIZE;
	case L_AEAD_CIPHER_AES_GCM:
		return 12;
	}

	return 0;
}

/* RFC3610 Section 2.3 */
static ssize_t build_ccm_iv(const void *nonce, uint8_t nonce_len,
				uint8_t (*iv)[CCM_IV_SIZE])
{
	const size_t iv_overhead = 2;
	int lprime = 15 - nonce_len - 1;

	if (unlikely(nonce_len + iv_overhead > CCM_IV_SIZE || lprime > 7))
		return -EINVAL;

	(*iv)[0] = lprime;
	memcpy(*iv + 1, nonce, nonce_len);
	memset(*iv + 1 + nonce_len, 0, lprime + 1);

	return CCM_IV_SIZE;
}

LIB_EXPORT bool l_aead_cipher_encrypt(struct l_aead_cipher *cipher,
					const void *in, size_t in_len,
					const void *ad, size_t ad_len,
					const void *nonce, size_t nonce_len,
					void *out, size_t out_len)
{
	uint8_t ccm_iv[CCM_IV_SIZE];
	const uint8_t *iv;
	ssize_t iv_len;

	if (unlikely(!cipher))
		return false;

	if (unlikely(!in) || unlikely(!out))
		return false;

	if (cipher->type == L_AEAD_CIPHER_AES_CCM) {
		iv_len = build_ccm_iv(nonce, nonce_len, &ccm_iv);
		if (unlikely(iv_len < 0))
			return false;

		iv = ccm_iv;
	} else {
		if (unlikely(nonce_len != l_aead_cipher_get_ivlen(cipher)))
			return false;

		iv = nonce;
		iv_len = nonce_len;
	}

	return operate_cipher(cipher->sk, ALG_OP_ENCRYPT, in, in_len,
				ad, ad_len, iv, iv_len, out, out_len) ==
			(ssize_t)out_len;
}

LIB_EXPORT bool l_aead_cipher_decrypt(struct l_aead_cipher *cipher,
					const void *in, size_t in_len,
					const void *ad, size_t ad_len,
					const void *nonce, size_t nonce_len,
					void *out, size_t out_len)
{
	uint8_t ccm_iv[CCM_IV_SIZE];
	const uint8_t *iv;
	ssize_t iv_len;

	if (unlikely(!cipher))
		return false;

	if (unlikely(!in) || unlikely(!out))
		return false;

	if (cipher->type == L_AEAD_CIPHER_AES_CCM) {
		iv_len = build_ccm_iv(nonce, nonce_len, &ccm_iv);
		if (unlikely(iv_len < 0))
			return false;

		iv = ccm_iv;
	} else {
		if (unlikely(nonce_len != l_aead_cipher_get_ivlen(cipher)))
			return false;

		iv = nonce;
		iv_len = nonce_len;
	}

	return operate_cipher(cipher->sk, ALG_OP_DECRYPT, in, in_len,
				ad, ad_len, iv, iv_len, out, out_len) ==
			(ssize_t)out_len;
}

static void init_supported()
{
	static bool initialized = false;
	struct sockaddr_alg salg;
	int sk;
	enum l_cipher_type c;
	enum l_aead_cipher_type a;

	if (likely(initialized))
		return;

	initialized = true;

	sk = socket(PF_ALG, SOCK_SEQPACKET | SOCK_CLOEXEC, 0);
	if (sk < 0)
		return;

	memset(&salg, 0, sizeof(salg));
	salg.salg_family = AF_ALG;
	strcpy((char *) salg.salg_type, "skcipher");

	for (c = L_CIPHER_AES; c <= L_CIPHER_DES3_EDE_CBC; c++) {
		const char *name = cipher_type_to_name(c);

		if (!name)
			continue;
		strcpy((char *) salg.salg_name, name);

		if (bind(sk, (struct sockaddr *) &salg, sizeof(salg)) < 0)
			continue;

		supported_ciphers |= 1 << c;
	}

	for (c = 0; c < L_ARRAY_SIZE(local_impl_ciphers); c++)
		if (HAVE_LOCAL_IMPLEMENTATION(c))
			supported_ciphers |= 1 << c;

	strcpy((char *) salg.salg_type, "aead");

	for (a = L_AEAD_CIPHER_AES_CCM; a <= L_AEAD_CIPHER_AES_GCM; a++) {
		strcpy((char *) salg.salg_name, aead_cipher_type_to_name(a));

		if (bind(sk, (struct sockaddr *) &salg, sizeof(salg)) < 0)
			continue;

		supported_aead_ciphers |= 1 << a;
	}

	close(sk);
}

LIB_EXPORT bool l_cipher_is_supported(enum l_cipher_type type)
{
	if (!is_valid_type(type))
		return false;

	init_supported();

	return supported_ciphers & (1 << type);
}

LIB_EXPORT bool l_aead_cipher_is_supported(enum l_aead_cipher_type type)
{
	if (type != L_AEAD_CIPHER_AES_CCM && type != L_AEAD_CIPHER_AES_GCM)
		return false;

	init_supported();

	return supported_aead_ciphers & (1 << type);
}

/* ARC4 implementation copyright (c) 2001 Niels Möller */

#define SWAP(a, b) do { uint8_t _t = a; a = b; b = _t; } while (0)

static void arc4_set_key(uint8_t *S, const uint8_t *key, size_t key_length)
{
	unsigned int i;
	uint8_t j;

	for (i = 0; i < 256; i++)
		S[i] = i;

	for (i = j = 0; i < 256; i++) {
		j += S[i] + key[i % key_length];
		SWAP(S[i], S[j]);
	}
}

struct arc4_state {
	struct arc4_state_ctx {
		uint8_t S[256];
		uint8_t i;
		uint8_t j;
	} ctx[2];
};

static void *local_arc4_new(enum l_cipher_type type,
				const void *key, size_t key_length)
{
	struct arc4_state *s;

	if (unlikely(key_length == 0 || key_length > 256))
		return NULL;

	s = l_new(struct arc4_state, 1);
	arc4_set_key(s->ctx[0].S, key, key_length);
	s->ctx[1] = s->ctx[0];
	return s;
}

static void local_arc4_free(void *data)
{
	explicit_bzero(data, sizeof(struct arc4_state));
	l_free(data);
}

static ssize_t local_arc4_operate(void *data, __u32 operation,
					const struct iovec *in, size_t in_cnt,
					const struct iovec *out, size_t out_cnt)
{
	struct arc4_state *s = data;
	struct iovec cur_in;
	struct iovec cur_out;
	struct arc4_state_ctx *ctx =
		&s->ctx[operation == ALG_OP_ENCRYPT ? 1 : 0];

	if (!in_cnt || !out_cnt)
		return 0;

	cur_in = *in;
	cur_out = *out;

	while (1) {
		while (!cur_in.iov_len) {
			cur_in = *in++;

			if (!--in_cnt)
				return 0;
		}

		while (!cur_out.iov_len) {
			cur_out = *out++;

			if (!--out_cnt)
				return 0;
		}

		ctx->j += ctx->S[++ctx->i];
		SWAP(ctx->S[ctx->i], ctx->S[ctx->j]);
		*(uint8_t *) cur_out.iov_base++ =
			*(uint8_t *) cur_in.iov_base++ ^
			ctx->S[(ctx->S[ctx->i] + ctx->S[ctx->j]) & 0xff];
		cur_in.iov_len--;
		cur_out.iov_len--;
	}
}

static const struct local_impl local_arc4 = {
	local_arc4_new,
	local_arc4_free,
	NULL,
	local_arc4_operate,
};

struct rc2_state {
	union {
		uint16_t xkey[64];
		uint8_t xkey8[128];
	};
	struct rc2_state_ctx {
		union {
			uint16_t x[4];
			uint64_t x64;
		};
	} ctx[2];
};

/* Simplified from the 1996 public-domain implementation */
static void rc2_keyschedule(struct rc2_state *s,
				const uint8_t *key, size_t key_len,
				size_t bits)
{
	static const uint8_t permute[256] = {
		217,120,249,196, 25,221,181,237, 40,233,253,121, 74,160,216,157,
		198,126, 55,131, 43,118, 83,142, 98, 76,100,136, 68,139,251,162,
		 23,154, 89,245,135,179, 79, 19, 97, 69,109,141,  9,129,125, 50,
		189,143, 64,235,134,183,123, 11,240,149, 33, 34, 92,107, 78,130,
		 84,214,101,147,206, 96,178, 28,115, 86,192, 20,167,140,241,220,
		 18,117,202, 31, 59,190,228,209, 66, 61,212, 48,163, 60,182, 38,
		111,191, 14,218, 70,105,  7, 87, 39,242, 29,155,188,148, 67,  3,
		248, 17,199,246,144,239, 62,231,  6,195,213, 47,200,102, 30,215,
		  8,232,234,222,128, 82,238,247,132,170,114,172, 53, 77,106, 42,
		150, 26,210,113, 90, 21, 73,116, 75,159,208, 94,  4, 24,164,236,
		194,224, 65,110, 15, 81,203,204, 36,145,175, 80,161,244,112, 57,
		153,124, 58,133, 35,184,180,122,252,  2, 54, 91, 37, 85,151, 49,
		 45, 93,250,152,227,138,146,174,  5,223, 41, 16,103,108,186,201,
		211,  0,230,207,225,158,168, 44, 99, 22,  1, 63, 88,226,137,169,
		 13, 56, 52, 27,171, 51,255,176,187, 72, 12, 95,185,177,205, 46,
		197,243,219, 71,229,165,156,119, 10,166, 32,104,254,127,193,173
	};
	uint8_t x;
	unsigned int i;

	memcpy(&s->xkey8, key, key_len);

	/* Step 1: expand input key to 128 bytes */
	x = s->xkey8[key_len - 1];

	for (i = 0; key_len < 128; key_len++, i++)
		s->xkey8[key_len] = x = permute[(x + s->xkey8[i]) & 255];

	/* Step 2: reduce effective key size to "bits" */
	key_len = (bits + 7) >> 3;
	i = 128 - key_len;
	s->xkey8[i] = x = permute[s->xkey8[i] & (255 >> (7 & -bits))];

	while (i--)
		s->xkey8[i] = x = permute[x ^ s->xkey8[i + key_len]];

	/* Step 3: copy to xkey in little-endian order */
	for (i = 0; i < 64; i++)
		s->xkey[i] = L_CPU_TO_LE16(s->xkey[i]);
}

static uint64_t rc2_operate(struct rc2_state *s, uint64_t in, __u32 operation)
{
	int i;
	union {
		uint16_t x16[4];
		uint64_t x64;
	} x;

	x.x64 = in;

	if (operation == ALG_OP_ENCRYPT) {
		const uint16_t *xkey = s->xkey;

		for (i = 0; i < 16; i++) {
			x.x16[0] += (x.x16[1] & ~x.x16[3]) +
				(x.x16[2] & x.x16[3]) + *xkey++;
			x.x16[0] = (x.x16[0] << 1) | (x.x16[0] >> 15);
			x.x16[1] += (x.x16[2] & ~x.x16[0]) +
				(x.x16[3] & x.x16[0]) + *xkey++;
			x.x16[1] = (x.x16[1] << 2) | (x.x16[1] >> 14);
			x.x16[2] += (x.x16[3] & ~x.x16[1]) +
				(x.x16[0] & x.x16[1]) + *xkey++;
			x.x16[2] = (x.x16[2] << 3) | (x.x16[2] >> 13);
			x.x16[3] += (x.x16[0] & ~x.x16[2]) +
				(x.x16[1] & x.x16[2]) + *xkey++;
			x.x16[3] = (x.x16[3] << 5) | (x.x16[3] >> 11);

			if (i == 4 || i == 10) {
				x.x16[0] += s->xkey[x.x16[3] & 63];
				x.x16[1] += s->xkey[x.x16[0] & 63];
				x.x16[2] += s->xkey[x.x16[1] & 63];
				x.x16[3] += s->xkey[x.x16[2] & 63];
			}
		}
	} else {
		const uint16_t *xkey = s->xkey + 63;

		for (i = 0; i < 16; i++) {
			x.x16[3] = (x.x16[3] << 11) | (x.x16[3] >> 5);
			x.x16[3] -= (x.x16[0] & ~x.x16[2]) +
				(x.x16[1] & x.x16[2]) + *xkey--;
			x.x16[2] = (x.x16[2] << 13) | (x.x16[2] >> 3);
			x.x16[2] -= (x.x16[3] & ~x.x16[1]) +
				(x.x16[0] & x.x16[1]) + *xkey--;
			x.x16[1] = (x.x16[1] << 14) | (x.x16[1] >> 2);
			x.x16[1] -= (x.x16[2] & ~x.x16[0]) +
				(x.x16[3] & x.x16[0]) + *xkey--;
			x.x16[0] = (x.x16[0] << 15) | (x.x16[0] >> 1);
			x.x16[0] -= (x.x16[1] & ~x.x16[3]) +
				(x.x16[2] & x.x16[3]) + *xkey--;

			if (i == 4 || i == 10) {
				x.x16[3] -= s->xkey[x.x16[2] & 63];
				x.x16[2] -= s->xkey[x.x16[1] & 63];
				x.x16[1] -= s->xkey[x.x16[0] & 63];
				x.x16[0] -= s->xkey[x.x16[3] & 63];
			}
		}
	}

	return x.x64;
}

static void *local_rc2_cbc_new(enum l_cipher_type type,
				const void *key, size_t key_length)
{
	struct rc2_state *s;

	if (unlikely(key_length == 0 || key_length > 128))
		return NULL;

	/*
	 * The key length and the effective "strength" bits are separate
	 * parameters but they match in our current use cases.
	 */
	s = l_new(struct rc2_state, 1);
	rc2_keyschedule(s, key, key_length, key_length * 8);
	return s;
}

static void local_rc2_cbc_free(void *data)
{
	explicit_bzero(data, sizeof(struct rc2_state));
	l_free(data);
}

static bool local_rc2_cbc_set_iv(void *data,
				const uint8_t *iv, size_t iv_length)
{
	struct rc2_state *s = data;

	if (unlikely(iv_length != 8))
		return false;

	s->ctx[0].x[0] = l_get_le16(iv + 0);
	s->ctx[0].x[1] = l_get_le16(iv + 2);
	s->ctx[0].x[2] = l_get_le16(iv + 4);
	s->ctx[0].x[3] = l_get_le16(iv + 6);
	s->ctx[1].x64 = s->ctx[0].x64;
	return true;
}

static ssize_t local_rc2_cbc_operate(void *data, __u32 operation,
					const struct iovec *in, size_t in_cnt,
					const struct iovec *out, size_t out_cnt)
{
	struct rc2_state *s = data;
	struct iovec cur_in = {};
	struct iovec cur_out = {};
	struct rc2_state_ctx *ctx =
		&s->ctx[operation == ALG_OP_ENCRYPT ? 1 : 0];

#define CONSUME_IN(bytes, eof_ok)		\
	cur_in.iov_len -= (bytes);		\
	while (!cur_in.iov_len) {		\
		if (!in_cnt) {			\
			if (eof_ok)		\
				break;		\
			else			\
				return -1;	\
		}				\
						\
		cur_in = *in++;			\
		in_cnt--;			\
	}

#define CONSUME_OUT(bytes)			\
	cur_out.iov_len -= (bytes);		\
	while (!cur_out.iov_len) {		\
		if (!out_cnt)			\
			return 0;		\
						\
		cur_out = *out++;		\
		out_cnt--;			\
	}

	CONSUME_IN(0, true)
	CONSUME_OUT(0)

	while (cur_in.iov_len) {
		union {
			uint16_t x16[4];
			uint64_t x64;
		} inblk;

		if (cur_in.iov_len >= 8) {
#define CUR_IN16 (*(uint16_t **) &cur_in.iov_base)
			inblk.x16[0] = l_get_le16(CUR_IN16++);
			inblk.x16[1] = l_get_le16(CUR_IN16++);
			inblk.x16[2] = l_get_le16(CUR_IN16++);
			inblk.x16[3] = l_get_le16(CUR_IN16++);
			CONSUME_IN(8, true)
		} else {
			inblk.x16[0] = *(uint8_t *) cur_in.iov_base++;
			CONSUME_IN(1, false)
			inblk.x16[0] |= (*(uint8_t *) cur_in.iov_base++) << 8;
			CONSUME_IN(1, false)
			inblk.x16[1] = *(uint8_t *) cur_in.iov_base++;
			CONSUME_IN(1, false)
			inblk.x16[1] |= (*(uint8_t *) cur_in.iov_base++) << 8;
			CONSUME_IN(1, false)
			inblk.x16[2] = *(uint8_t *) cur_in.iov_base++;
			CONSUME_IN(1, false)
			inblk.x16[2] |= (*(uint8_t *) cur_in.iov_base++) << 8;
			CONSUME_IN(1, false)
			inblk.x16[3] = *(uint8_t *) cur_in.iov_base++;
			CONSUME_IN(1, false)
			inblk.x16[3] |= (*(uint8_t *) cur_in.iov_base++) << 8;
			CONSUME_IN(1, true)
		}

		if (operation == ALG_OP_ENCRYPT)
			ctx->x64 = rc2_operate(s, inblk.x64 ^ ctx->x64,
						operation);
		else
			ctx->x64 ^= rc2_operate(s, inblk.x64, operation);

		if (cur_out.iov_len >= 8) {
#define CUR_OUT16 (*(uint16_t **) &cur_out.iov_base)
			l_put_le16(ctx->x[0], CUR_OUT16++);
			l_put_le16(ctx->x[1], CUR_OUT16++);
			l_put_le16(ctx->x[2], CUR_OUT16++);
			l_put_le16(ctx->x[3], CUR_OUT16++);
			CONSUME_OUT(8)
		} else {
			*(uint8_t *) cur_out.iov_base++ = ctx->x[0];
			CONSUME_OUT(1)
			*(uint8_t *) cur_out.iov_base++ = ctx->x[0] >> 8;
			CONSUME_OUT(1)
			*(uint8_t *) cur_out.iov_base++ = ctx->x[1];
			CONSUME_OUT(1)
			*(uint8_t *) cur_out.iov_base++ = ctx->x[1] >> 8;
			CONSUME_OUT(1)
			*(uint8_t *) cur_out.iov_base++ = ctx->x[2];
			CONSUME_OUT(1)
			*(uint8_t *) cur_out.iov_base++ = ctx->x[2] >> 8;
			CONSUME_OUT(1)
			*(uint8_t *) cur_out.iov_base++ = ctx->x[3];
			CONSUME_OUT(1)
			*(uint8_t *) cur_out.iov_base++ = ctx->x[3] >> 8;
			CONSUME_OUT(1)
		}

		/* Save ciphertext as IV for next CBC block */
		if (operation == ALG_OP_DECRYPT)
			ctx->x64 = inblk.x64;

		inblk.x64 = 0;
	}

	return 0;
}

static const struct local_impl local_rc2_cbc = {
	local_rc2_cbc_new,
	local_rc2_cbc_free,
	local_rc2_cbc_set_iv,
	local_rc2_cbc_operate,
};
