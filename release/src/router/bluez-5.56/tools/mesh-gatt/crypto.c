// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2017  Intel Corporation. All rights reserved.
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

#include <linux/if_alg.h>

#include <glib.h>

#ifndef SOL_ALG
#define SOL_ALG		279
#endif

#ifndef ALG_SET_AEAD_AUTHSIZE
#define ALG_SET_AEAD_AUTHSIZE	5
#endif

#include "src/shared/util.h"

#include "tools/mesh-gatt/mesh-net.h"
#include "tools/mesh-gatt/crypto.h"

static int alg_new(int fd, const void *keyval, socklen_t keylen,
		size_t mic_size)
{
	if (setsockopt(fd, SOL_ALG, ALG_SET_KEY, keyval, keylen) < 0) {
		g_printerr("key");
		return -1;
	}

	if (mic_size &&
		setsockopt(fd, SOL_ALG,
			ALG_SET_AEAD_AUTHSIZE, NULL, mic_size) < 0) {
		g_printerr("taglen");
		return -1;
	}

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

static int aes_ecb_setup(const uint8_t key[16])
{
	struct sockaddr_alg salg;
	int fd, nfd;

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

	nfd = alg_new(fd, key, 16, 0);

	close(fd);

	return nfd;
}

static bool aes_ecb(int fd, const uint8_t plaintext[16], uint8_t encrypted[16])
{
	return alg_encrypt(fd, plaintext, 16, encrypted, 16);
}

static void aes_ecb_destroy(int fd)
{
	close(fd);
}

static bool aes_ecb_one(const uint8_t key[16],
			const uint8_t plaintext[16], uint8_t encrypted[16])
{
	bool result;
	int fd;

	fd = aes_ecb_setup(key);
	if (fd < 0)
		return false;

	result = aes_ecb(fd, plaintext, encrypted);

	aes_ecb_destroy(fd);

	return result;
}

/* Maximum message length that can be passed to aes_cmac */
#define CMAC_MSG_MAX	(64 + 64 + 17)

static int aes_cmac_setup(const uint8_t key[16])
{
	struct sockaddr_alg salg;
	int fd, nfd;

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

	nfd = alg_new(fd, key, 16, 0);

	close(fd);

	return nfd;
}

static bool aes_cmac(int fd, const uint8_t *msg,
					size_t msg_len, uint8_t res[16])
{
	ssize_t len;

	if (msg_len > CMAC_MSG_MAX)
		return false;

	len = send(fd, msg, msg_len, 0);
	if (len < 0)
		return false;

	len = read(fd, res, 16);
	if (len < 0)
		return false;

	return true;
}

static void aes_cmac_destroy(int fd)
{
	close(fd);
}

static int aes_cmac_N_start(const uint8_t N[16])
{
	int fd;

	fd = aes_cmac_setup(N);
	return fd;
}

static bool aes_cmac_one(const uint8_t key[16], const void *msg,
					size_t msg_len, uint8_t res[16])
{
	bool result;
	int fd;

	fd = aes_cmac_setup(key);
	if (fd < 0)
		return false;

	result = aes_cmac(fd, msg, msg_len, res);

	aes_cmac_destroy(fd);

	return result;
}

bool mesh_crypto_aes_cmac(const uint8_t key[16], const uint8_t *msg,
					size_t msg_len, uint8_t res[16])
{
	return aes_cmac_one(key, msg, msg_len, res);
}

bool mesh_crypto_aes_ccm_encrypt(const uint8_t nonce[13], const uint8_t key[16],
					const uint8_t *aad, uint16_t aad_len,
					const uint8_t *msg, uint16_t msg_len,
					uint8_t *out_msg, void *out_mic,
					size_t mic_size)
{
	uint8_t pmsg[16], cmic[16], cmsg[16];
	uint8_t mic[16], Xn[16];
	uint16_t blk_cnt, last_blk;
	bool result;
	size_t i, j;
	int fd;

	if (aad_len >= 0xff00) {
		g_printerr("Unsupported AAD size");
		return false;
	}

	fd = aes_ecb_setup(key);
	if (fd < 0)
		return false;

	/* C_mic = e(AppKey, 0x01 || nonce || 0x0000) */
	pmsg[0] = 0x01;
	memcpy(pmsg + 1, nonce, 13);
	put_be16(0x0000, pmsg + 14);

	result = aes_ecb(fd, pmsg, cmic);
	if (!result)
		goto done;

	/* X_0 = e(AppKey, 0x09 || nonce || length) */
	if (mic_size == sizeof(uint64_t))
		pmsg[0] = 0x19 | (aad_len ? 0x40 : 0x00);
	else
		pmsg[0] = 0x09 | (aad_len ? 0x40 : 0x00);

	memcpy(pmsg + 1, nonce, 13);
	put_be16(msg_len, pmsg + 14);

	result = aes_ecb(fd, pmsg, Xn);
	if (!result)
		goto done;

	/* If AAD is being used to authenticate, include it here */
	if (aad_len) {
		put_be16(aad_len, pmsg);

		for (i = 0; i < sizeof(uint16_t); i++)
			pmsg[i] = Xn[i] ^ pmsg[i];

		j = 0;
		aad_len += sizeof(uint16_t);
		while (aad_len > 16) {
			do {
				pmsg[i] = Xn[i] ^ aad[j];
				i++, j++;
			} while (i < 16);

			aad_len -= 16;
			i = 0;

			result = aes_ecb(fd, pmsg, Xn);
			if (!result)
				goto done;
		}

		for (i = 0; i < aad_len; i++, j++)
			pmsg[i] = Xn[i] ^ aad[j];

		for (i = aad_len; i < 16; i++)
			pmsg[i] = Xn[i];

		result = aes_ecb(fd, pmsg, Xn);
		if (!result)
			goto done;
	}

	last_blk = msg_len % 16;
	blk_cnt = (msg_len + 15) / 16;
	if (!last_blk)
		last_blk = 16;

	for (j = 0; j < blk_cnt; j++) {
		if (j + 1 == blk_cnt) {
			/* X_1 = e(AppKey, X_0 ^ Payload[0-15]) */
			for (i = 0; i < last_blk; i++)
				pmsg[i] = Xn[i] ^ msg[(j * 16) + i];
			for (i = last_blk; i < 16; i++)
				pmsg[i] = Xn[i] ^ 0x00;

			result = aes_ecb(fd, pmsg, Xn);
			if (!result)
				goto done;

			/* MIC = C_mic ^ X_1 */
			for (i = 0; i < sizeof(mic); i++)
				mic[i] = cmic[i] ^ Xn[i];

			/* C_1 = e(AppKey, 0x01 || nonce || 0x0001) */
			pmsg[0] = 0x01;
			memcpy(pmsg + 1, nonce, 13);
			put_be16(j + 1, pmsg + 14);

			result = aes_ecb(fd, pmsg, cmsg);
			if (!result)
				goto done;

			if (out_msg) {
				/* Encrypted = Payload[0-15] ^ C_1 */
				for (i = 0; i < last_blk; i++)
					out_msg[(j * 16) + i] =
						msg[(j * 16) + i] ^ cmsg[i];

			}
		} else {
			/* X_1 = e(AppKey, X_0 ^ Payload[0-15]) */
			for (i = 0; i < 16; i++)
				pmsg[i] = Xn[i] ^ msg[(j * 16) + i];

			result = aes_ecb(fd, pmsg, Xn);
			if (!result)
				goto done;

			/* C_1 = e(AppKey, 0x01 || nonce || 0x0001) */
			pmsg[0] = 0x01;
			memcpy(pmsg + 1, nonce, 13);
			put_be16(j + 1, pmsg + 14);

			result = aes_ecb(fd, pmsg, cmsg);
			if (!result)
				goto done;

			if (out_msg) {
				/* Encrypted = Payload[0-15] ^ C_N */
				for (i = 0; i < 16; i++)
					out_msg[(j * 16) + i] =
						msg[(j * 16) + i] ^ cmsg[i];
			}

		}
	}

	if (out_msg)
		memcpy(out_msg + msg_len, mic, mic_size);

	if (out_mic) {
		switch (mic_size) {
		case sizeof(uint32_t):
			*(uint32_t *)out_mic = get_be32(mic);
			break;
		case sizeof(uint64_t):
			*(uint64_t *)out_mic = get_be64(mic);
			break;
		default:
			g_printerr("Unsupported MIC size");
		}
	}

done:
	aes_ecb_destroy(fd);

	return result;
}

bool mesh_crypto_aes_ccm_decrypt(const uint8_t nonce[13], const uint8_t key[16],
				const uint8_t *aad, uint16_t aad_len,
				const uint8_t *enc_msg, uint16_t enc_msg_len,
				uint8_t *out_msg, void *out_mic,
				size_t mic_size)
{
	uint8_t msg[16], pmsg[16], cmic[16], cmsg[16], Xn[16];
	uint8_t mic[16];
	uint16_t msg_len = enc_msg_len - mic_size;
	uint16_t last_blk, blk_cnt;
	bool result;
	size_t i, j;
	int fd;

	if (enc_msg_len < 5 || aad_len >= 0xff00)
		return false;

	fd = aes_ecb_setup(key);
	if (fd < 0)
		return false;

	/* C_mic = e(AppKey, 0x01 || nonce || 0x0000) */
	pmsg[0] = 0x01;
	memcpy(pmsg + 1, nonce, 13);
	put_be16(0x0000, pmsg + 14);

	result = aes_ecb(fd, pmsg, cmic);
	if (!result)
		goto done;

	/* X_0 = e(AppKey, 0x09 || nonce || length) */
	if (mic_size == sizeof(uint64_t))
		pmsg[0] = 0x19 | (aad_len ? 0x40 : 0x00);
	else
		pmsg[0] = 0x09 | (aad_len ? 0x40 : 0x00);

	memcpy(pmsg + 1, nonce, 13);
	put_be16(msg_len, pmsg + 14);

	result = aes_ecb(fd, pmsg, Xn);
	if (!result)
		goto done;

	/* If AAD is being used to authenticate, include it here */
	if (aad_len) {
		put_be16(aad_len, pmsg);

		for (i = 0; i < sizeof(uint16_t); i++)
			pmsg[i] = Xn[i] ^ pmsg[i];

		j = 0;
		aad_len += sizeof(uint16_t);
		while (aad_len > 16) {
			do {
				pmsg[i] = Xn[i] ^ aad[j];
				i++, j++;
			} while (i < 16);

			aad_len -= 16;
			i = 0;

			result = aes_ecb(fd, pmsg, Xn);
			if (!result)
				goto done;
		}

		for (i = 0; i < aad_len; i++, j++)
			pmsg[i] = Xn[i] ^ aad[j];

		for (i = aad_len; i < 16; i++)
			pmsg[i] = Xn[i];

		result = aes_ecb(fd, pmsg, Xn);
		if (!result)
			goto done;
	}

	last_blk = msg_len % 16;
	blk_cnt = (msg_len + 15) / 16;
	if (!last_blk)
		last_blk = 16;

	for (j = 0; j < blk_cnt; j++) {
		if (j + 1 == blk_cnt) {
			/* C_1 = e(AppKey, 0x01 || nonce || 0x0001) */
			pmsg[0] = 0x01;
			memcpy(pmsg + 1, nonce, 13);
			put_be16(j + 1, pmsg + 14);

			result = aes_ecb(fd, pmsg, cmsg);
			if (!result)
				goto done;

			/* Encrypted = Payload[0-15] ^ C_1 */
			for (i = 0; i < last_blk; i++)
				msg[i] = enc_msg[(j * 16) + i] ^ cmsg[i];

			if (out_msg)
				memcpy(out_msg + (j * 16), msg, last_blk);

			/* X_1 = e(AppKey, X_0 ^ Payload[0-15]) */
			for (i = 0; i < last_blk; i++)
				pmsg[i] = Xn[i] ^ msg[i];
			for (i = last_blk; i < 16; i++)
				pmsg[i] = Xn[i] ^ 0x00;

			result = aes_ecb(fd, pmsg, Xn);
			if (!result)
				goto done;

			/* MIC = C_mic ^ X_1 */
			for (i = 0; i < sizeof(mic); i++)
				mic[i] = cmic[i] ^ Xn[i];
		} else {
			/* C_1 = e(AppKey, 0x01 || nonce || 0x0001) */
			pmsg[0] = 0x01;
			memcpy(pmsg + 1, nonce, 13);
			put_be16(j + 1, pmsg + 14);

			result = aes_ecb(fd, pmsg, cmsg);
			if (!result)
				goto done;

			/* Encrypted = Payload[0-15] ^ C_1 */
			for (i = 0; i < 16; i++)
				msg[i] = enc_msg[(j * 16) + i] ^ cmsg[i];

			if (out_msg)
				memcpy(out_msg + (j * 16), msg, 16);

			/* X_1 = e(AppKey, X_0 ^ Payload[0-15]) */
			for (i = 0; i < 16; i++)
				pmsg[i] = Xn[i] ^ msg[i];

			result = aes_ecb(fd, pmsg, Xn);
			if (!result)
				goto done;
		}
	}

	switch (mic_size) {
		case sizeof(uint32_t):
			if (out_mic)
				*(uint32_t *)out_mic = get_be32(mic);
			else if (get_be32(enc_msg + enc_msg_len - mic_size) !=
					get_be32(mic))
				result = false;
			break;

		case sizeof(uint64_t):
			if (out_mic)
				*(uint64_t *)out_mic = get_be64(mic);
			else if (get_be64(enc_msg + enc_msg_len - mic_size) !=
					get_be64(mic))
				result = false;
			break;

		default:
			g_printerr("Unsupported MIC size");
			result = false;
	}

done:
	aes_ecb_destroy(fd);

	return result;
}

bool mesh_crypto_k1(const uint8_t ikm[16], const uint8_t salt[16],
		const void *info, size_t info_len, uint8_t okm[16])
{
	uint8_t res[16];

	if (!aes_cmac_one(salt, ikm, 16, res))
		return false;

	return aes_cmac_one(res, info, info_len, okm);
}

bool mesh_crypto_k2(const uint8_t n[16], const uint8_t *p, size_t p_len,
							uint8_t net_id[1],
							uint8_t enc_key[16],
							uint8_t priv_key[16])
{
	int fd;
	uint8_t output[16];
	uint8_t t[16];
	uint8_t *stage;
	bool success = false;

	stage = g_malloc(sizeof(output) + p_len + 1);
	if (stage == NULL)
		return false;

	if (!mesh_crypto_s1("smk2", 4, stage))
		goto fail;

	if (!aes_cmac_one(stage, n, 16, t))
		goto fail;

	fd = aes_cmac_N_start(t);
	if (fd < 0)
		goto fail;

	memcpy(stage, p, p_len);
	stage[p_len] = 1;

	if(!aes_cmac(fd, stage, p_len + 1, output))
		goto done;

	net_id[0] = output[15] & 0x7f;

	memcpy(stage, output, 16);
	memcpy(stage + 16, p, p_len);
	stage[p_len + 16] = 2;

	if(!aes_cmac(fd, stage, p_len + 16 + 1, output))
		goto done;

	memcpy(enc_key, output, 16);

	memcpy(stage, output, 16);
	memcpy(stage + 16, p, p_len);
	stage[p_len + 16] = 3;

	if(!aes_cmac(fd, stage, p_len + 16 + 1, output))
		goto done;

	memcpy(priv_key, output, 16);
	success = true;

done:
	aes_cmac_destroy(fd);
fail:
	g_free(stage);

	return success;
}

static bool crypto_128(const uint8_t n[16], const char *s, uint8_t out128[16])
{
	uint8_t id128[] = { 'i', 'd', '1', '2', '8', 0x01 };
	uint8_t salt[16];

	if (!mesh_crypto_s1(s, 4, salt))
		return false;

	return mesh_crypto_k1(n, salt, id128, sizeof(id128), out128);
}

bool mesh_crypto_nkik(const uint8_t n[16], uint8_t identity_key[16])
{
	return crypto_128(n, "nkik", identity_key);
}

static bool identity_calc(const uint8_t net_key[16], uint16_t addr,
		bool check, uint8_t id[16])
{
	uint8_t id_key[16];
	uint8_t tmp[16];

	if (!mesh_crypto_nkik(net_key, id_key))
		return false;

	memset(tmp, 0, sizeof(tmp));
	put_be16(addr, tmp + 14);

	if (check) {
		memcpy(tmp + 6, id + 8, 8);
	} else {
		mesh_get_random_bytes(tmp + 6, 8);
		memcpy(id + 8, tmp + 6, 8);
	}

	if (!aes_ecb_one(id_key, tmp, tmp))
		return false;

	if (check)
		return (memcmp(id, tmp + 8, 8) == 0);

	memcpy(id, tmp + 8, 8);
	return true;
}

bool mesh_crypto_identity(const uint8_t net_key[16], uint16_t addr,
							uint8_t id[16])
{
	return identity_calc(net_key, addr, false, id);
}

bool mesh_crypto_identity_check(const uint8_t net_key[16], uint16_t addr,
							uint8_t id[16])
{
	return identity_calc(net_key, addr, true, id);
}

bool mesh_crypto_nkbk(const uint8_t n[16], uint8_t beacon_key[16])
{
	return crypto_128(n, "nkbk", beacon_key);
}

bool mesh_crypto_k3(const uint8_t n[16], uint8_t out64[8])
{
	uint8_t tmp[16];
	uint8_t t[16];
	uint8_t id64[] = { 'i', 'd', '6', '4', 0x01 };

	if (!mesh_crypto_s1("smk3", 4, tmp))
		return false;

	if (!aes_cmac_one(tmp, n, 16, t))
		return false;

	if (!aes_cmac_one(t, id64, sizeof(id64), tmp))
		return false;

	memcpy(out64, tmp + 8, 8);

	return true;
}

bool mesh_crypto_k4(const uint8_t a[16], uint8_t out6[1])
{
	uint8_t tmp[16];
	uint8_t t[16];
	uint8_t id6[] = { 'i', 'd', '6', 0x01 };

	if (!mesh_crypto_s1("smk4", 4, tmp))
		return false;

	if (!aes_cmac_one(tmp, a, 16, t))
		return false;

	if (!aes_cmac_one(t, id6, sizeof(id6), tmp))
		return false;

	out6[0] = tmp[15] & 0x3f;
	return true;
}

bool mesh_crypto_beacon_cmac(const uint8_t encryption_key[16],
				const uint8_t network_id[8],
				uint32_t iv_index, bool kr, bool iu,
				uint64_t *cmac)
{
	uint8_t msg[13], tmp[16];

	if (!cmac)
		return false;

	msg[0] = kr ? 0x01 : 0x00;
	msg[0] |= iu ? 0x02 : 0x00;
	memcpy(msg + 1, network_id, 8);
	put_be32(iv_index, msg + 9);

	if (!aes_cmac_one(encryption_key, msg, 13, tmp))
		return false;

	*cmac = get_be64(tmp);

	return true;
}

bool mesh_crypto_network_nonce(bool ctl, uint8_t ttl, uint32_t seq,
				uint16_t src, uint32_t iv_index,
				uint8_t nonce[13])
{
	nonce[0] = 0;
	nonce[1] = (ttl & TTL_MASK) | (ctl ? CTL : 0x00);
	nonce[2] = (seq >> 16) & 0xff;
	nonce[3] = (seq >> 8) & 0xff;
	nonce[4] = seq & 0xff;

	/* SRC */
	put_be16(src, nonce + 5);

	put_be16(0, nonce + 7);

	/* IV Index */
	put_be32(iv_index, nonce + 9);

	return true;
}

bool mesh_crypto_network_encrypt(bool ctl, uint8_t ttl,
				uint32_t seq, uint16_t src,
				uint32_t iv_index,
				const uint8_t net_key[16],
				const uint8_t *enc_msg, uint8_t enc_msg_len,
				uint8_t *out, void *net_mic)
{
	uint8_t nonce[13];

	if (!mesh_crypto_network_nonce(ctl, ttl, seq, src, iv_index, nonce))
		return false;

	return mesh_crypto_aes_ccm_encrypt(nonce, net_key,
				NULL, 0, enc_msg,
				enc_msg_len, out,
				net_mic,
				ctl ? sizeof(uint64_t) : sizeof(uint32_t));
}

bool mesh_crypto_network_decrypt(bool ctl, uint8_t ttl,
				uint32_t seq, uint16_t src,
				uint32_t iv_index,
				const uint8_t net_key[16],
				const uint8_t *enc_msg, uint8_t enc_msg_len,
				uint8_t *out, void *net_mic, size_t mic_size)
{
	uint8_t nonce[13];

	if (!mesh_crypto_network_nonce(ctl, ttl, seq, src, iv_index, nonce))
		return false;

	return mesh_crypto_aes_ccm_decrypt(nonce, net_key, NULL, 0,
						enc_msg, enc_msg_len, out,
						net_mic, mic_size);
}

bool mesh_crypto_application_nonce(uint32_t seq, uint16_t src,
					uint16_t dst, uint32_t iv_index,
					bool aszmic, uint8_t nonce[13])
{
	nonce[0] = 0x01;
	nonce[1] = aszmic ? 0x80 : 0x00;
	nonce[2] = (seq & 0x00ff0000) >> 16;
	nonce[3] = (seq & 0x0000ff00) >> 8;
	nonce[4] = (seq & 0x000000ff);
	nonce[5] = (src & 0xff00) >> 8;
	nonce[6] = (src & 0x00ff);
	nonce[7] = (dst & 0xff00) >> 8;
	nonce[8] = (dst & 0x00ff);
	put_be32(iv_index, nonce + 9);

	return true;
}

bool mesh_crypto_device_nonce(uint32_t seq, uint16_t src,
					uint16_t dst, uint32_t iv_index,
					bool aszmic, uint8_t nonce[13])
{
	nonce[0] = 0x02;
	nonce[1] = aszmic ? 0x80 : 0x00;
	nonce[2] = (seq & 0x00ff0000) >> 16;
	nonce[3] = (seq & 0x0000ff00) >> 8;
	nonce[4] = (seq & 0x000000ff);
	nonce[5] = (src & 0xff00) >> 8;
	nonce[6] = (src & 0x00ff);
	nonce[7] = (dst & 0xff00) >> 8;
	nonce[8] = (dst & 0x00ff);
	put_be32(iv_index, nonce + 9);

	return true;
}

bool mesh_crypto_application_encrypt(uint8_t key_id, uint32_t seq, uint16_t src,
					uint16_t dst, uint32_t iv_index,
					const uint8_t app_key[16],
					const uint8_t *aad, uint8_t aad_len,
					const uint8_t *msg, uint8_t msg_len,
					uint8_t *out, void *app_mic,
					size_t mic_size)
{
	uint8_t nonce[13];
	bool aszmic = (mic_size == sizeof(uint64_t)) ? true : false;

	if (!key_id && !mesh_crypto_device_nonce(seq, src, dst,
				iv_index, aszmic, nonce))
		return false;

	if (key_id && !mesh_crypto_application_nonce(seq, src, dst,
				iv_index, aszmic, nonce))
		return false;

	return mesh_crypto_aes_ccm_encrypt(nonce, app_key, aad, aad_len,
						msg, msg_len,
						out, app_mic, mic_size);
}

bool mesh_crypto_application_decrypt(uint8_t key_id, uint32_t seq, uint16_t src,
				uint16_t dst, uint32_t iv_index,
				const uint8_t app_key[16],
				const uint8_t *aad, uint8_t aad_len,
				const uint8_t *enc_msg, uint8_t enc_msg_len,
				uint8_t *out, void *app_mic, size_t mic_size)
{
	uint8_t nonce[13];
	bool aszmic = (mic_size == sizeof(uint64_t)) ? true : false;

	if (!key_id && !mesh_crypto_device_nonce(seq, src, dst,
				iv_index, aszmic, nonce))
		return false;

	if (key_id && !mesh_crypto_application_nonce(seq, src, dst,
				iv_index, aszmic, nonce))
		return false;

	return mesh_crypto_aes_ccm_decrypt(nonce, app_key,
						aad, aad_len, enc_msg,
						enc_msg_len, out,
						app_mic, mic_size);
}

bool mesh_crypto_session_key(const uint8_t secret[32],
					const uint8_t salt[16],
					uint8_t session_key[16])
{
	const uint8_t prsk[4] = "prsk";

	if (!aes_cmac_one(salt, secret, 32, session_key))
		return false;

	return aes_cmac_one(session_key, prsk, 4, session_key);
}

bool mesh_crypto_nonce(const uint8_t secret[32],
					const uint8_t salt[16],
					uint8_t nonce[13])
{
	const uint8_t prsn[4] = "prsn";
	uint8_t tmp[16];
	bool result;

	if (!aes_cmac_one(salt, secret, 32, tmp))
		return false;

	result =  aes_cmac_one(tmp, prsn, 4, tmp);

	if (result)
		memcpy(nonce, tmp + 3, 13);

	return result;
}

bool mesh_crypto_s1(const void *info, size_t len, uint8_t salt[16])
{
	const uint8_t zero[16] = {0};

	return aes_cmac_one(zero, info, len, salt);
}

bool mesh_crypto_prov_prov_salt(const uint8_t conf_salt[16],
					const uint8_t prov_rand[16],
					const uint8_t dev_rand[16],
					uint8_t prov_salt[16])
{
	const uint8_t zero[16] = {0};
	uint8_t tmp[16 * 3];

	memcpy(tmp, conf_salt, 16);
	memcpy(tmp + 16, prov_rand, 16);
	memcpy(tmp + 32, dev_rand, 16);

	return aes_cmac_one(zero, tmp, sizeof(tmp), prov_salt);
}

bool mesh_crypto_prov_conf_key(const uint8_t secret[32],
					const uint8_t salt[16],
					uint8_t conf_key[16])
{
	const uint8_t prck[4] = "prck";

	if (!aes_cmac_one(salt, secret, 32, conf_key))
		return false;

	return aes_cmac_one(conf_key, prck, 4, conf_key);
}

bool mesh_crypto_device_key(const uint8_t secret[32],
						const uint8_t salt[16],
						uint8_t device_key[16])
{
	const uint8_t prdk[4] = "prdk";

	if (!aes_cmac_one(salt, secret, 32, device_key))
		return false;

	return aes_cmac_one(device_key, prdk, 4, device_key);
}

bool mesh_crypto_virtual_addr(const uint8_t virtual_label[16],
						uint16_t *addr)
{
	uint8_t tmp[16];

	if (!mesh_crypto_s1("vtad", 4, tmp))
		return false;

	if (!addr || !aes_cmac_one(tmp, virtual_label, 16, tmp))
		return false;

	*addr = (get_be16(tmp + 14) & 0x3fff) | 0x8000;

	return true;
}

bool mesh_crypto_packet_encode(uint8_t *packet, uint8_t packet_len,
				const uint8_t network_key[16],
				uint32_t iv_index,
				const uint8_t privacy_key[16])
{
	uint8_t network_nonce[13] = { 0x00, 0x00 };
	uint8_t privacy_counter[16] = { 0x00, 0x00, 0x00, 0x00, 0x00, };
	uint8_t tmp[16];
	int i;

	/* Detect Proxy packet by CTL == true && DST == 0x0000 */
	if ((packet[1] & CTL) && get_be16(packet + 7) == 0)
		network_nonce[0] = 0x03;
	else
		/* CTL + TTL */
		network_nonce[1] = packet[1];

	/* Seq Num */
	network_nonce[2] = packet[2];
	network_nonce[3] = packet[3];
	network_nonce[4] = packet[4];

	/* SRC */
	network_nonce[5] = packet[5];
	network_nonce[6] = packet[6];

	/* DST not available */
	network_nonce[7] = 0;
	network_nonce[8] = 0;

	/* IV Index */
	put_be32(iv_index, network_nonce + 9);

	/* Check for Long net-MIC */
	if (packet[1] & CTL) {
		if (!mesh_crypto_aes_ccm_encrypt(network_nonce, network_key,
					NULL, 0,
					packet + 7, packet_len - 7 - 8,
					packet + 7, NULL, sizeof(uint64_t)))
			return false;
	} else {
		if (!mesh_crypto_aes_ccm_encrypt(network_nonce, network_key,
					NULL, 0,
					packet + 7, packet_len - 7 - 4,
					packet + 7, NULL, sizeof(uint32_t)))
			return false;
	}

	put_be32(iv_index, privacy_counter + 5);
	memcpy(privacy_counter + 9, packet + 7, 7);

	if (!aes_ecb_one(privacy_key, privacy_counter, tmp))
		return false;

	for (i = 0; i < 6; i++)
		packet[1 + i] ^= tmp[i];

	return true;
}

bool mesh_crypto_packet_decode(const uint8_t *packet, uint8_t packet_len,
				bool proxy, uint8_t *out, uint32_t iv_index,
				const uint8_t network_key[16],
				const uint8_t privacy_key[16])
{
	uint8_t privacy_counter[16] = { 0x00, 0x00, 0x00, 0x00, 0x00, };
	uint8_t network_nonce[13] = { 0x00, 0x00, };
	uint8_t tmp[16];
	uint16_t src;
	int i;

	if (packet_len < 14)
		return false;

	put_be32(iv_index, privacy_counter + 5);
	memcpy(privacy_counter + 9, packet + 7, 7);

	if (!aes_ecb_one(privacy_key, privacy_counter, tmp))
		return false;

	memcpy(out, packet, packet_len);
	for (i = 0; i < 6; i++)
		out[1 + i] ^= tmp[i];

	src  = get_be16(out + 5);

	/* Pre-check SRC address for illegal values */
	if (!src || src >= 0x8000)
		return false;

	/* Detect Proxy packet by CTL == true && proxy == true */
	if ((out[1] & CTL) && proxy)
		network_nonce[0] = 0x03;
	else
		/* CTL + TTL */
		network_nonce[1] = out[1];

	/* Seq Num */
	network_nonce[2] = out[2];
	network_nonce[3] = out[3];
	network_nonce[4] = out[4];

	/* SRC */
	network_nonce[5] = out[5];
	network_nonce[6] = out[6];

	/* DST not available */
	network_nonce[7] = 0;
	network_nonce[8] = 0;

	/* IV Index */
	put_be32(iv_index, network_nonce + 9);

	/* Check for Long MIC */
	if (out[1] & CTL) {
		uint64_t mic;

		if (!mesh_crypto_aes_ccm_decrypt(network_nonce, network_key,
					NULL, 0, packet + 7, packet_len - 7,
					out + 7, &mic, sizeof(mic)))
			return false;

		mic ^= get_be64(out + packet_len - 8);
		put_be64(mic, out + packet_len - 8);

		if (mic)
			return false;
	} else {
		uint32_t mic;

		if (!mesh_crypto_aes_ccm_decrypt(network_nonce, network_key,
					NULL, 0, packet + 7, packet_len - 7,
					out + 7, &mic, sizeof(mic)))
			return false;

		mic ^= get_be32(out + packet_len - 4);
		put_be32(mic, out + packet_len - 4);

		if (mic)
			return false;
	}

	return true;
}

bool mesh_get_random_bytes(void *buf, size_t num_bytes)
{
	ssize_t len;
	int fd;

	fd = open("/dev/urandom", O_RDONLY);
	if (fd < 0)
		return false;

	len = read(fd, buf, num_bytes);

	close(fd);

	if (len < 0)
		return false;

	return true;
}
