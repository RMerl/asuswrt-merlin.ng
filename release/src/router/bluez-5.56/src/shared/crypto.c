// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2012-2014  Intel Corporation. All rights reserved.
 *
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>

#include "src/shared/util.h"
#include "src/shared/crypto.h"

#ifndef HAVE_LINUX_IF_ALG_H
#ifndef HAVE_LINUX_TYPES_H
typedef uint8_t __u8;
typedef uint16_t __u16;
typedef uint32_t __u32;
#else
#include <linux/types.h>
#endif

struct sockaddr_alg {
	__u16   salg_family;
	__u8    salg_type[14];
	__u32   salg_feat;
	__u32   salg_mask;
	__u8    salg_name[64];
};

struct af_alg_iv {
	__u32   ivlen;
	__u8    iv[0];
};

#define ALG_SET_KEY                     1
#define ALG_SET_IV                      2
#define ALG_SET_OP                      3

#define ALG_OP_DECRYPT                  0
#define ALG_OP_ENCRYPT                  1

#define PF_ALG		38	/* Algorithm sockets.  */
#define AF_ALG		PF_ALG
#else
#include <linux/if_alg.h>
#endif

#ifndef SOL_ALG
#define SOL_ALG		279
#endif

/* Maximum message length that can be passed to aes_cmac */
#define CMAC_MSG_MAX	80

#define ATT_SIGN_LEN	12

struct bt_crypto {
	int ref_count;
	int ecb_aes;
	int urandom;
	int cmac_aes;
};

static int urandom_setup(void)
{
	int fd;

	fd = open("/dev/urandom", O_RDONLY);
	if (fd < 0)
		return -1;

	return fd;
}

static int ecb_aes_setup(void)
{
	struct sockaddr_alg salg;
	int fd;

	fd = socket(PF_ALG, SOCK_SEQPACKET | SOCK_CLOEXEC, 0);
	if (fd < 0)
		return -1;

	memset(&salg, 0, sizeof(salg));
	salg.salg_family = AF_ALG;
	strcpy((char *) salg.salg_type, "skcipher");
	strcpy((char *) salg.salg_name, "ecb(aes)");

	if (bind(fd, (struct sockaddr *) &salg, sizeof(salg)) < 0) {
		close(fd);
		return -1;
	}

	return fd;
}

static int cmac_aes_setup(void)
{
	struct sockaddr_alg salg;
	int fd;

	fd = socket(PF_ALG, SOCK_SEQPACKET | SOCK_CLOEXEC, 0);
	if (fd < 0)
		return -1;

	memset(&salg, 0, sizeof(salg));
	salg.salg_family = AF_ALG;
	strcpy((char *) salg.salg_type, "hash");
	strcpy((char *) salg.salg_name, "cmac(aes)");

	if (bind(fd, (struct sockaddr *) &salg, sizeof(salg)) < 0) {
		close(fd);
		return -1;
	}

	return fd;
}

static struct bt_crypto *singleton;

struct bt_crypto *bt_crypto_new(void)
{
	if (singleton)
		return bt_crypto_ref(singleton);

	singleton = new0(struct bt_crypto, 1);

	singleton->ecb_aes = ecb_aes_setup();
	if (singleton->ecb_aes < 0) {
		free(singleton);
		singleton = NULL;
		return NULL;
	}

	singleton->urandom = urandom_setup();
	if (singleton->urandom < 0) {
		close(singleton->ecb_aes);
		free(singleton);
		singleton = NULL;
		return NULL;
	}

	singleton->cmac_aes = cmac_aes_setup();
	if (singleton->cmac_aes < 0) {
		close(singleton->urandom);
		close(singleton->ecb_aes);
		free(singleton);
		singleton = NULL;
		return NULL;
	}

	return bt_crypto_ref(singleton);
}

struct bt_crypto *bt_crypto_ref(struct bt_crypto *crypto)
{
	if (!crypto)
		return NULL;

	__sync_fetch_and_add(&crypto->ref_count, 1);

	return crypto;
}

void bt_crypto_unref(struct bt_crypto *crypto)
{
	if (!crypto)
		return;

	if (__sync_sub_and_fetch(&crypto->ref_count, 1))
		return;

	close(crypto->urandom);
	close(crypto->ecb_aes);
	close(crypto->cmac_aes);

	free(crypto);
	singleton = NULL;
}

bool bt_crypto_random_bytes(struct bt_crypto *crypto,
					void *buf, uint8_t num_bytes)
{
	ssize_t len;

	if (!crypto)
		return false;

	len = read(crypto->urandom, buf, num_bytes);
	if (len < num_bytes)
		return false;

	return true;
}

static int alg_new(int fd, const void *keyval, socklen_t keylen)
{
	if (setsockopt(fd, SOL_ALG, ALG_SET_KEY, keyval, keylen) < 0)
		return -1;

	/* FIXME: This should use accept4() with SOCK_CLOEXEC */
	return accept(fd, NULL, 0);
}

static bool alg_encrypt(int fd, const void *inbuf, size_t inlen,
						void *outbuf, size_t outlen)
{
	__u32 alg_op = ALG_OP_ENCRYPT;
	char cbuf[CMSG_SPACE(sizeof(alg_op))];
	struct cmsghdr *cmsg;
	struct msghdr msg;
	struct iovec iov;
	ssize_t len;

	memset(cbuf, 0, sizeof(cbuf));
	memset(&msg, 0, sizeof(msg));

	msg.msg_control = cbuf;
	msg.msg_controllen = sizeof(cbuf);

	cmsg = CMSG_FIRSTHDR(&msg);
	cmsg->cmsg_level = SOL_ALG;
	cmsg->cmsg_type = ALG_SET_OP;
	cmsg->cmsg_len = CMSG_LEN(sizeof(alg_op));
	memcpy(CMSG_DATA(cmsg), &alg_op, sizeof(alg_op));

	iov.iov_base = (void *) inbuf;
	iov.iov_len = inlen;

	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;

	len = sendmsg(fd, &msg, 0);
	if (len < 0)
		return false;

	len = read(fd, outbuf, outlen);
	if (len < 0)
		return false;

	return true;
}

static inline void swap_buf(const uint8_t *src, uint8_t *dst, uint16_t len)
{
	int i;

	for (i = 0; i < len; i++)
		dst[len - 1 - i] = src[i];
}

bool bt_crypto_sign_att(struct bt_crypto *crypto, const uint8_t key[16],
				const uint8_t *m, uint16_t m_len,
				uint32_t sign_cnt,
				uint8_t signature[ATT_SIGN_LEN])
{
	int fd;
	int len;
	uint8_t tmp[16], out[16];
	uint16_t msg_len = m_len + sizeof(uint32_t);
	uint8_t msg[msg_len];
	uint8_t msg_s[msg_len];

	if (!crypto)
		return false;

	memset(msg, 0, msg_len);
	memcpy(msg, m, m_len);

	/* Add sign_counter to the message */
	put_le32(sign_cnt, msg + m_len);

	/* The most significant octet of key corresponds to key[0] */
	swap_buf(key, tmp, 16);

	fd = alg_new(crypto->cmac_aes, tmp, 16);
	if (fd < 0)
		return false;

	/* Swap msg before signing */
	swap_buf(msg, msg_s, msg_len);

	len = send(fd, msg_s, msg_len, 0);
	if (len < 0) {
		close(fd);
		return false;
	}

	len = read(fd, out, 16);
	if (len < 0) {
		close(fd);
		return false;
	}

	close(fd);

	/*
	 * As to BT spec. 4.1 Vol[3], Part C, chapter 10.4.1 sign counter should
	 * be placed in the signature
	 */
	put_be32(sign_cnt, out + 8);

	/*
	 * The most significant octet of hash corresponds to out[0]  - swap it.
	 * Then truncate in most significant bit first order to a length of
	 * 12 octets
	 */
	swap_buf(out, tmp, 16);
	memcpy(signature, tmp + 4, ATT_SIGN_LEN);

	return true;
}

bool bt_crypto_verify_att_sign(struct bt_crypto *crypto, const uint8_t key[16],
				const uint8_t *pdu, uint16_t pdu_len)
{
	uint8_t generated_sign[ATT_SIGN_LEN];
	const uint8_t *sign;
	uint32_t sign_cnt;

	if (pdu_len < ATT_SIGN_LEN)
		return false;

	sign = pdu + pdu_len - ATT_SIGN_LEN;
	sign_cnt = get_le32(sign);

	if (!bt_crypto_sign_att(crypto, key, pdu, pdu_len - ATT_SIGN_LEN,
						sign_cnt, generated_sign))
		return false;

	return memcmp(generated_sign, sign, ATT_SIGN_LEN) == 0;
}

/*
 * Security function e
 *
 * Security function e generates 128-bit encryptedData from a 128-bit key
 * and 128-bit plaintextData using the AES-128-bit block cypher:
 *
 *   encryptedData = e(key, plaintextData)
 *
 * The most significant octet of key corresponds to key[0], the most
 * significant octet of plaintextData corresponds to in[0] and the
 * most significant octet of encryptedData corresponds to out[0].
 *
 */
bool bt_crypto_e(struct bt_crypto *crypto, const uint8_t key[16],
			const uint8_t plaintext[16], uint8_t encrypted[16])
{
	uint8_t tmp[16], in[16], out[16];
	int fd;

	if (!crypto)
		return false;

	/* The most significant octet of key corresponds to key[0] */
	swap_buf(key, tmp, 16);

	fd = alg_new(crypto->ecb_aes, tmp, 16);
	if (fd < 0)
		return false;


	/* Most significant octet of plaintextData corresponds to in[0] */
	swap_buf(plaintext, in, 16);

	if (!alg_encrypt(fd, in, 16, out, 16)) {
		close(fd);
		return false;
	}

	/* Most significant octet of encryptedData corresponds to out[0] */
	swap_buf(out, encrypted, 16);

	close(fd);

	return true;
}

/*
 * Random Address Hash function ah
 *
 * The random address hash function ah is used to generate a hash value
 * that is used in resolvable private addresses.
 *
 * The following are inputs to the random address hash function ah:
 *
 *   k is 128 bits
 *   r is 24 bits
 *   padding is 104 bits
 *
 * r is concatenated with padding to generate r' which is used as the
 * 128-bit input parameter plaintextData to security function e:
 *
 *   r' = padding || r
 *
 * The least significant octet of r becomes the least significant octet
 * of r’ and the most significant octet of padding becomes the most
 * significant octet of r'.
 *
 * For example, if the 24-bit value r is 0x423456 then r' is
 * 0x00000000000000000000000000423456.
 *
 * The output of the random address function ah is:
 *
 *   ah(k, r) = e(k, r') mod 2^24
 *
 * The output of the security function e is then truncated to 24 bits by
 * taking the least significant 24 bits of the output of e as the result
 * of ah.
 */
bool bt_crypto_ah(struct bt_crypto *crypto, const uint8_t k[16],
					const uint8_t r[3], uint8_t hash[3])
{
	uint8_t rp[16];
	uint8_t encrypted[16];

	if (!crypto)
		return false;

	/* r' = padding || r */
	memcpy(rp, r, 3);
	memset(rp + 3, 0, 13);

	/* e(k, r') */
	if (!bt_crypto_e(crypto, k, rp, encrypted))
		return false;

	/* ah(k, r) = e(k, r') mod 2^24 */
	memcpy(hash, encrypted, 3);

	return true;
}

typedef struct {
	uint64_t a, b;
} u128;

static inline void u128_xor(const uint8_t p[16], const uint8_t q[16],
								uint8_t r[16])
{
	u128 pp, qq, rr;

	memcpy(&pp, p, 16);
	memcpy(&qq, q, 16);

	rr.a = pp.a ^ qq.a;
	rr.b = pp.b ^ qq.b;

	memcpy(r, &rr, 16);
}

/*
 * Confirm value generation function c1
 *
 * During the pairing process confirm values are exchanged. This confirm
 * value generation function c1 is used to generate the confirm values.
 *
 * The following are inputs to the confirm value generation function c1:
 *
 *   k is 128 bits
 *   r is 128 bits
 *   pres is 56 bits
 *   preq is 56 bits
 *   iat is 1 bit
 *   ia is 48 bits
 *   rat is 1 bit
 *   ra is 48 bits
 *   padding is 32 bits of 0
 *
 * iat is concatenated with 7-bits of 0 to create iat' which is 8 bits
 * in length. iat is the least significant bit of iat'
 *
 * rat is concatenated with 7-bits of 0 to create rat' which is 8 bits
 * in length. rat is the least significant bit of rat'
 *
 * pres, preq, rat' and iat' are concatenated to generate p1 which is
 * XORed with r and used as 128-bit input parameter plaintextData to
 * security function e:
 *
 *   p1 = pres || preq || rat' || iat'
 *
 * The octet of iat' becomes the least significant octet of p1 and the
 * most significant octet of pres becomes the most significant octet of
 * p1.
 *
 * ra is concatenated with ia and padding to generate p2 which is XORed
 * with the result of the security function e using p1 as the input
 * paremter plaintextData and is then used as the 128-bit input
 * parameter plaintextData to security function e:
 *
 *   p2 = padding || ia || ra
 *
 * The least significant octet of ra becomes the least significant octet
 * of p2 and the most significant octet of padding becomes the most
 * significant octet of p2.
 *
 * The output of the confirm value generation function c1 is:
 *
 *   c1(k, r, preq, pres, iat, rat, ia, ra) = e(k, e(k, r XOR p1) XOR p2)
 *
 * The 128-bit output of the security function e is used as the result
 * of confirm value generation function c1.
 */
bool bt_crypto_c1(struct bt_crypto *crypto, const uint8_t k[16],
			const uint8_t r[16], const uint8_t pres[7],
			const uint8_t preq[7], uint8_t iat,
			const uint8_t ia[6], uint8_t rat,
			const uint8_t ra[6], uint8_t res[16])
{
	uint8_t p1[16], p2[16];

	/* p1 = pres || preq || _rat || _iat */
	p1[0] = iat;
	p1[1] = rat;
	memcpy(p1 + 2, preq, 7);
	memcpy(p1 + 9, pres, 7);

	/* p2 = padding || ia || ra */
	memcpy(p2, ra, 6);
	memcpy(p2 + 6, ia, 6);
	memset(p2 + 12, 0, 4);

	/* res = r XOR p1 */
	u128_xor(r, p1, res);

	/* res = e(k, res) */
	if (!bt_crypto_e(crypto, k, res, res))
		return false;

	/* res = res XOR p2 */
	u128_xor(res, p2, res);

	/* res = e(k, res) */
	return bt_crypto_e(crypto, k, res, res);
}

/*
 * Key generation function s1
 *
 * The key generation function s1 is used to generate the STK during the
 * pairing process.
 *
 * The following are inputs to the key generation function s1:
 *
 *   k is 128 bits
 *   r1 is 128 bits
 *   r2 is 128 bits
 *
 * The most significant 64-bits of r1 are discarded to generate r1' and
 * the most significant 64-bits of r2 are discarded to generate r2'.
 *
 * r1' is concatenated with r2' to generate r' which is used as the
 * 128-bit input parameter plaintextData to security function e:
 *
 *   r' = r1' || r2'
 *
 * The least significant octet of r2' becomes the least significant
 * octet of r' and the most significant octet of r1' becomes the most
 * significant octet of r'.
 *
 * The output of the key generation function s1 is:
 *
 *   s1(k, r1, r2) = e(k, r')
 *
 * The 128-bit output of the security function e is used as the result
 * of key generation function s1.
 */
bool bt_crypto_s1(struct bt_crypto *crypto, const uint8_t k[16],
			const uint8_t r1[16], const uint8_t r2[16],
			uint8_t res[16])
{
	memcpy(res, r2, 8);
	memcpy(res + 8, r1, 8);

	return bt_crypto_e(crypto, k, res, res);
}

static bool aes_cmac(struct bt_crypto *crypto, const uint8_t key[16],
			const uint8_t *msg, size_t msg_len, uint8_t res[16])
{
	uint8_t key_msb[16], out[16], msg_msb[CMAC_MSG_MAX];
	ssize_t len;
	int fd;

	if (msg_len > CMAC_MSG_MAX)
		return false;

	swap_buf(key, key_msb, 16);
	fd = alg_new(crypto->cmac_aes, key_msb, 16);
	if (fd < 0)
		return false;

	swap_buf(msg, msg_msb, msg_len);
	len = send(fd, msg_msb, msg_len, 0);
	if (len < 0) {
		close(fd);
		return false;
	}

	len = read(fd, out, 16);
	if (len < 0) {
		close(fd);
		return false;
	}

	swap_buf(out, res, 16);

	close(fd);

	return true;
}

bool bt_crypto_f4(struct bt_crypto *crypto, uint8_t u[32], uint8_t v[32],
				uint8_t x[16], uint8_t z, uint8_t res[16])
{
	uint8_t m[65];

	if (!crypto)
		return false;

	m[0] = z;
	memcpy(&m[1], v, 32);
	memcpy(&m[33], u, 32);

	return aes_cmac(crypto, x, m, sizeof(m), res);
}

bool bt_crypto_f5(struct bt_crypto *crypto, uint8_t w[32], uint8_t n1[16],
				uint8_t n2[16], uint8_t a1[7], uint8_t a2[7],
				uint8_t mackey[16], uint8_t ltk[16])
{
	uint8_t btle[4] = { 0x65, 0x6c, 0x74, 0x62 };
	uint8_t salt[16] = { 0xbe, 0x83, 0x60, 0x5a, 0xdb, 0x0b, 0x37, 0x60,
			     0x38, 0xa5, 0xf5, 0xaa, 0x91, 0x83, 0x88, 0x6c };
	uint8_t length[2] = { 0x00, 0x01 };
	uint8_t m[53], t[16];

	if (!aes_cmac(crypto, salt, w, 32, t))
		return false;

	memcpy(&m[0], length, 2);
	memcpy(&m[2], a2, 7);
	memcpy(&m[9], a1, 7);
	memcpy(&m[16], n2, 16);
	memcpy(&m[32], n1, 16);
	memcpy(&m[48], btle, 4);

	m[52] = 0; /* Counter */
	if (!aes_cmac(crypto, t, m, sizeof(m), mackey))
		return false;

	m[52] = 1; /* Counter */
	return aes_cmac(crypto, t, m, sizeof(m), ltk);
}

bool bt_crypto_f6(struct bt_crypto *crypto, uint8_t w[16], uint8_t n1[16],
			uint8_t n2[16], uint8_t r[16], uint8_t io_cap[3],
			uint8_t a1[7], uint8_t a2[7], uint8_t res[16])
{
	uint8_t m[65];

	memcpy(&m[0], a2, 7);
	memcpy(&m[7], a1, 7);
	memcpy(&m[14], io_cap, 3);
	memcpy(&m[17], r, 16);
	memcpy(&m[33], n2, 16);
	memcpy(&m[49], n1, 16);

	return aes_cmac(crypto, w, m, sizeof(m), res);
}

bool bt_crypto_g2(struct bt_crypto *crypto, uint8_t u[32], uint8_t v[32],
				uint8_t x[16], uint8_t y[16], uint32_t *val)
{
	uint8_t m[80], tmp[16];

	memcpy(&m[0], y, 16);
	memcpy(&m[16], v, 32);
	memcpy(&m[48], u, 32);

	if (!aes_cmac(crypto, x, m, sizeof(m), tmp))
		return false;

	*val = get_le32(tmp);
	*val %= 1000000;

	return true;
}

bool bt_crypto_h6(struct bt_crypto *crypto, const uint8_t w[16],
				const uint8_t keyid[4], uint8_t res[16])
{
	if (!aes_cmac(crypto, w, keyid, 4, res))
		return false;

	return true;
}

bool bt_crypto_gatt_hash(struct bt_crypto *crypto, struct iovec *iov,
				size_t iov_len, uint8_t res[16])
{
	const uint8_t key[16] = {};
	ssize_t len;
	int fd;

	if (!crypto)
		return false;

	fd = alg_new(crypto->cmac_aes, key, 16);
	if (fd < 0)
		return false;

	len = writev(fd, iov, iov_len);
	if (len < 0) {
		close(fd);
		return false;
	}

	len = read(fd, res, 16);
	if (len < 0) {
		close(fd);
		return false;
	}

	close(fd);

	return true;
}
