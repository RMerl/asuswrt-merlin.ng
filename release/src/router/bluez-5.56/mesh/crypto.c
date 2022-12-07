// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2018-2019  Intel Corporation. All rights reserved.
 *
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#define _GNU_SOURCE
#include <unistd.h>
#include <sys/socket.h>
#include <ell/ell.h>

#include "mesh/mesh-defs.h"
#include "mesh/net.h"
#include "mesh/crypto.h"

/* Multiply used Zero array */
static const uint8_t zero[16] = { 0, };

static bool aes_ecb_one(const uint8_t key[16], const uint8_t in[16],
								uint8_t out[16])
{
	void *cipher;
	bool result = false;

	cipher = l_cipher_new(L_CIPHER_AES, key, 16);

	if (cipher) {
		result = l_cipher_encrypt(cipher, in, out, 16);
		l_cipher_free(cipher);
	}

	return result;
}

static bool aes_cmac(void *checksum, const uint8_t *msg,
					size_t msg_len, uint8_t res[16])
{
	if (!l_checksum_update(checksum, msg, msg_len))
		return false;

	if (16 == l_checksum_get_digest(checksum, res, 16))
		return true;

	return false;
}

static bool aes_cmac_one(const uint8_t key[16], const void *msg,
					size_t msg_len, uint8_t res[16])
{
	void *checksum;
	bool result;

	checksum = l_checksum_new_cmac_aes(key, 16);
	if (!checksum)
		return false;

	result = l_checksum_update(checksum, msg, msg_len);

	if (result) {
		ssize_t len = l_checksum_get_digest(checksum, res, 16);
		result = !!(len == 16);
	}

	l_checksum_free(checksum);

	return result;
}

bool mesh_crypto_aes_cmac(const uint8_t key[16], const uint8_t *msg,
					size_t msg_len, uint8_t res[16])
{
	return aes_cmac_one(key, msg, msg_len, res);
}

bool mesh_crypto_aes_ccm_encrypt(const uint8_t nonce[13], const uint8_t key[16],
					const uint8_t *aad, uint16_t aad_len,
					const void *msg, uint16_t msg_len,
					void *out_msg,
					void *out_mic, size_t mic_size)
{
	void *cipher;
	bool result;

	cipher = l_aead_cipher_new(L_AEAD_CIPHER_AES_CCM, key, 16, mic_size);

	result = l_aead_cipher_encrypt(cipher, msg, msg_len, aad, aad_len,
					nonce, 13, out_msg, msg_len + mic_size);

	if (result && out_mic) {
		if (mic_size == 4)
			*(uint32_t *)out_mic = l_get_be32(out_msg + msg_len);
		else
			*(uint64_t *)out_mic = l_get_be64(out_msg + msg_len);
	}

	l_aead_cipher_free(cipher);

	return result;
}

bool mesh_crypto_aes_ccm_decrypt(const uint8_t nonce[13], const uint8_t key[16],
				const uint8_t *aad, uint16_t aad_len,
				const void *enc_msg, uint16_t enc_msg_len,
				void *out_msg,
				void *out_mic, size_t mic_size)
{
	void *cipher;
	bool result;
	size_t out_msg_len = enc_msg_len - mic_size;

	cipher = l_aead_cipher_new(L_AEAD_CIPHER_AES_CCM, key, 16, mic_size);

	result = l_aead_cipher_decrypt(cipher, enc_msg, enc_msg_len,
							aad, aad_len, nonce, 13,
							out_msg, out_msg_len);

	if (result && out_mic) {
		if (mic_size == 4)
			*(uint32_t *)out_mic =
				l_get_be32(enc_msg + enc_msg_len - mic_size);
		else
			*(uint64_t *)out_mic =
				l_get_be64(enc_msg + enc_msg_len - mic_size);
	}

	l_aead_cipher_free(cipher);

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
	void *checksum;
	uint8_t output[16];
	uint8_t t[16];
	uint8_t *stage;
	bool success = false;

	stage = l_malloc(sizeof(output) + p_len + 1);
	if (!stage)
		return false;

	if (!mesh_crypto_s1("smk2", 4, stage))
		goto fail;

	if (!aes_cmac_one(stage, n, 16, t))
		goto fail;

	checksum = l_checksum_new_cmac_aes(t, 16);
	if (!checksum)
		goto fail;

	memcpy(stage, p, p_len);
	stage[p_len] = 1;

	if (!aes_cmac(checksum, stage, p_len + 1, output))
		goto done;

	net_id[0] = output[15] & 0x7f;

	memcpy(stage, output, 16);
	memcpy(stage + 16, p, p_len);
	stage[p_len + 16] = 2;

	if (!aes_cmac(checksum, stage, p_len + 16 + 1, output))
		goto done;

	memcpy(enc_key, output, 16);

	memcpy(stage, output, 16);
	memcpy(stage + 16, p, p_len);
	stage[p_len + 16] = 3;

	if (!aes_cmac(checksum, stage, p_len + 16 + 1, output))
		goto done;

	memcpy(priv_key, output, 16);
	success = true;

done:
	l_checksum_free(checksum);
fail:
	l_free(stage);

	return success;
}

static bool crypto_128(const uint8_t n[16], const char *s, uint8_t out128[16])
{
	const uint8_t id128[] = { 'i', 'd', '1', '2', '8', 0x01 };
	uint8_t salt[16];

	if (!mesh_crypto_s1(s, 4, salt))
		return false;

	return mesh_crypto_k1(n, salt, id128, sizeof(id128), out128);
}

bool mesh_crypto_nkik(const uint8_t n[16], uint8_t identity_key[16])
{
	return crypto_128(n, "nkik", identity_key);
}

bool mesh_crypto_identity(const uint8_t net_key[16], uint16_t addr,
							uint8_t id[16])
{
	uint8_t id_key[16];
	uint8_t tmp[16];

	if (!mesh_crypto_nkik(net_key, id_key))
		return false;

	if (!l_get_be64(id + 8))
		l_getrandom(id + 8, 8);

	memset(tmp, 0, sizeof(tmp));
	memcpy(tmp + 6, id + 8, 8);
	l_put_be16(addr, tmp + 14);

	if (!aes_ecb_one(id_key, tmp, tmp))
		return false;

	memcpy(id, tmp + 8, 8);
	return true;
}

bool mesh_crypto_nkbk(const uint8_t n[16], uint8_t beacon_key[16])
{
	return crypto_128(n, "nkbk", beacon_key);
}

bool mesh_crypto_nkpk(const uint8_t n[16], uint8_t proxy_key[16])
{
	return crypto_128(n, "nkpk", proxy_key);
}

bool mesh_crypto_k3(const uint8_t n[16], uint8_t out64[8])
{
	const uint8_t id64[] = { 'i', 'd', '6', '4', 0x01 };
	uint8_t tmp[16];
	uint8_t t[16];

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
	const uint8_t id6[] = { 'i', 'd', '6', 0x01 };
	uint8_t tmp[16];
	uint8_t t[16];

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
	l_put_be32(iv_index, msg + 9);

	if (!aes_cmac_one(encryption_key, msg, 13, tmp))
		return false;

	*cmac = l_get_be64(tmp);

	return true;
}

static void mesh_crypto_network_nonce(bool ctl, uint8_t ttl,
					uint32_t seq, uint16_t src,
					uint32_t iv_index, uint8_t nonce[13])
{
	nonce[0] = 0x00;
	nonce[1] = (ttl & TTL_MASK) | (ctl ? CTL : 0x00);
	nonce[2] = (seq >> 16) & 0xff;
	nonce[3] = (seq >> 8) & 0xff;
	nonce[4] = seq & 0xff;
	l_put_be16(src, nonce + 5);
	l_put_be16(0, nonce + 7);
	l_put_be32(iv_index, nonce + 9);
}

static void mesh_crypto_application_nonce(uint32_t seq, uint16_t src,
					uint16_t dst, uint32_t iv_index,
					bool aszmic, uint8_t nonce[13])
{
	nonce[0] = 0x01;
	nonce[1] = aszmic ? 0x80 : 0x00;
	nonce[2] = (seq >> 16) & 0xff;
	nonce[3] = (seq >> 8) & 0xff;
	nonce[4] = seq & 0xff;
	l_put_be16(src, nonce + 5);
	l_put_be16(dst, nonce + 7);
	l_put_be32(iv_index, nonce + 9);
}

static void mesh_crypto_device_nonce(uint32_t seq, uint16_t src,
					uint16_t dst, uint32_t iv_index,
					bool aszmic, uint8_t nonce[13])
{
	nonce[0] = 0x02;
	nonce[1] = aszmic ? 0x80 : 0x00;
	nonce[2] = (seq >> 16) & 0xff;
	nonce[3] = (seq >> 8) & 0xff;
	nonce[4] = seq & 0xff;
	l_put_be16(src, nonce + 5);
	l_put_be16(dst, nonce + 7);
	l_put_be32(iv_index, nonce + 9);
}

static void mesh_crypto_proxy_nonce(uint32_t seq, uint16_t src,
					uint32_t iv_index, uint8_t nonce[13])
{
	nonce[0] = 0x03;
	nonce[1] = 0;
	nonce[2] = (seq >> 16) & 0xff;
	nonce[3] = (seq >> 8) & 0xff;
	nonce[4] = seq & 0xff;
	l_put_be16(src, nonce + 5);
	l_put_be16(0, nonce + 7);
	l_put_be32(iv_index, nonce + 9);
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

	result = aes_cmac_one(tmp, prsn, 4, tmp);

	if (result)
		memcpy(nonce, tmp + 3, 13);

	return result;
}

bool mesh_crypto_s1(const void *info, size_t len, uint8_t salt[16])
{
	return aes_cmac_one(zero, info, len, salt);
}

bool mesh_crypto_prov_prov_salt(const uint8_t conf_salt[16],
					const uint8_t prov_rand[16],
					const uint8_t dev_rand[16],
					uint8_t prov_salt[16])
{
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

	*addr = (l_get_be16(tmp + 14) & 0x3fff) | 0x8000;

	return true;
}

static void mesh_crypto_privacy_counter(uint32_t iv_index,
						const uint8_t *payload,
						uint8_t privacy_counter[16])
{
	memset(privacy_counter, 0, 5);
	l_put_be32(iv_index, privacy_counter + 5);
	memcpy(privacy_counter + 9, payload, 7);
}

static bool mesh_crypto_pecb(const uint8_t privacy_key[16],
						uint32_t iv_index,
						const uint8_t *payload,
						uint8_t pecb[16])
{
	mesh_crypto_privacy_counter(iv_index, payload, pecb);
	return aes_ecb_one(privacy_key, pecb, pecb);
}

static bool mesh_crypto_network_obfuscate(uint8_t *packet,
						const uint8_t privacy_key[16],
						uint32_t iv_index,
						bool ctl, uint8_t ttl,
						uint32_t seq, uint16_t src)
{
	uint8_t pecb[16];
	uint8_t *net_hdr = packet + 1;
	int i;

	if (!mesh_crypto_pecb(privacy_key, iv_index, packet + 7, pecb))
		return false;

	l_put_be16(src, net_hdr + 4);
	l_put_be32(seq & SEQ_MASK, net_hdr);
	net_hdr[0] = ((!!ctl) << 7) | (ttl & TTL_MASK);

	for (i = 0; i < 6; i++)
		net_hdr[i] = pecb[i] ^ net_hdr[i];

	return true;
}

static bool mesh_crypto_network_clarify(uint8_t *packet,
						const uint8_t privacy_key[16],
						uint32_t iv_index,
						bool *ctl, uint8_t *ttl,
						uint32_t *seq, uint16_t *src)
{
	uint8_t pecb[16];
	uint8_t *net_hdr = packet + 1;
	int i;

	if (!mesh_crypto_pecb(privacy_key, iv_index, packet + 7, pecb))
		return false;

	for (i = 0; i < 6; i++)
		net_hdr[i] = pecb[i] ^ net_hdr[i];

	*src = l_get_be16(net_hdr + 4);
	*seq = l_get_be32(net_hdr) & SEQ_MASK;
	*ttl = net_hdr[0] & TTL_MASK;
	*ctl = !!(net_hdr[0] & CTL);

	return true;
}

bool mesh_crypto_packet_build(bool ctl, uint8_t ttl,
				uint32_t seq,
				uint16_t src, uint16_t dst,
				uint8_t opcode,
				bool segmented, uint8_t key_aid,
				bool szmic, bool relay, uint16_t seqZero,
				uint8_t segO, uint8_t segN,
				const uint8_t *payload, uint8_t payload_len,
				uint8_t *packet, uint8_t *packet_len)
{
	uint32_t hdr;
	size_t n;

	if (seq > SEQ_MASK)
		return false;

	packet[0] = 0;
	l_put_be32(seq, packet + 1);
	packet[1] = (ctl ? CTL : 0) | (ttl & TTL_MASK);

	l_put_be16(src, packet + 5);
	l_put_be16(dst, packet + 7);
	n = 9;

	if (!ctl) {
		hdr = segmented << SEG_HDR_SHIFT;
		hdr |= (key_aid & KEY_ID_MASK) << KEY_HDR_SHIFT;
		if (segmented) {
			hdr |= szmic << SZMIC_HDR_SHIFT;
			hdr |= (seqZero & SEQ_ZERO_MASK) << SEQ_ZERO_HDR_SHIFT;
			hdr |= (segO & SEG_MASK) << SEGO_HDR_SHIFT;
			hdr |= (segN & SEG_MASK) << SEGN_HDR_SHIFT;
		}
		l_put_be32(hdr, packet + n);

		/* Only first octet is valid for unsegmented messages */
		if (segmented)
			n += 4;
		else
			n += 1;

		memcpy(packet + n, payload, payload_len);

		l_put_be32(0x00000000, packet + payload_len + n);
		if (packet_len)
			*packet_len = payload_len + n + 4;
	} else {
		if ((opcode & OPCODE_MASK) != opcode)
			return false;

		hdr = opcode << KEY_HDR_SHIFT;
		l_put_be32(hdr, packet + n);
		n += 1;

		memcpy(packet + n, payload, payload_len);
		n += payload_len;

		l_put_be64(0x0000000000000000, packet + n);
		if (packet_len)
			*packet_len = n + 8;
	}


	return true;
}

static bool network_header_parse(const uint8_t *packet, uint8_t packet_len,
				bool *ctl, uint8_t *ttl, uint32_t *seq,
				uint16_t *src, uint16_t *dst)
{
	if (packet_len < 10)
		return false;

	/* Try to keep bits in the order they exist within the packet */
	if (ctl)
		*ctl = !!(packet[1] & CTL);

	if (ttl)
		*ttl = packet[1] & TTL_MASK;

	if (seq)
		*seq = l_get_be32(packet + 1) & SEQ_MASK;

	if (src)
		*src = l_get_be16(packet + 5);

	if (dst)
		*dst = l_get_be16(packet + 7);

	return true;

}

bool mesh_crypto_packet_parse(const uint8_t *packet, uint8_t packet_len,
				bool *ctl, uint8_t *ttl, uint32_t *seq,
				uint16_t *src, uint16_t *dst,
				uint32_t *cookie, uint8_t *opcode,
				bool *segmented, uint8_t *key_aid,
				bool *szmic, bool *relay, uint16_t *seqZero,
				uint8_t *segO, uint8_t *segN,
				const uint8_t **payload, uint8_t *payload_len)
{
	uint32_t hdr;
	uint16_t this_dst;
	bool is_segmented;

	if (!network_header_parse(packet, packet_len,
					ctl, ttl, seq, src, &this_dst))
		return false;

	if (dst)
		*dst = this_dst;

	hdr = l_get_be32(packet + 9);

	is_segmented = !!((hdr >> SEG_HDR_SHIFT) & true);
	if (segmented)
		*segmented = is_segmented;

	if (packet[1] & CTL) {
		uint8_t this_opcode = packet[9] & OPCODE_MASK;

		if (cookie)
			*cookie = l_get_be32(packet + 2) ^ packet[6];

		if (opcode)
			*opcode = this_opcode;

		if (this_dst && this_opcode == NET_OP_SEG_ACKNOWLEDGE) {
			if (relay)
				*relay = !!((hdr >> RELAY_HDR_SHIFT) & true);

			if (seqZero)
				*seqZero = (hdr >> SEQ_ZERO_HDR_SHIFT) &
								SEQ_ZERO_MASK;

			if (payload)
				*payload = packet + 9;

			if (payload_len)
				*payload_len = packet_len - 9;
		} else {
			if (payload)
				*payload = packet + 10;

			if (payload_len)
				*payload_len = packet_len - 10;
		}
	} else {
		if (cookie)
			*cookie = l_get_be32(packet + packet_len - 8);

		if (key_aid)
			*key_aid = (hdr >> KEY_HDR_SHIFT) & KEY_ID_MASK;

		if (is_segmented) {
			if (szmic)
				*szmic = !!((hdr >> SZMIC_HDR_SHIFT) & true);

			if (seqZero)
				*seqZero = (hdr >> SEQ_ZERO_HDR_SHIFT) &
								SEQ_ZERO_MASK;

			if (segO)
				*segO = (hdr >> SEGO_HDR_SHIFT) & SEG_MASK;

			if (segN)
				*segN = (hdr >> SEGN_HDR_SHIFT) & SEG_MASK;

			if (payload)
				*payload = packet + 13;

			if (payload_len)
				*payload_len = packet_len - 13;
		} else {
			if (payload)
				*payload = packet + 10;

			if (payload_len)
				*payload_len = packet_len - 10;
		}
	}

	return true;
}

bool mesh_crypto_payload_encrypt(uint8_t *aad, const uint8_t *payload,
				uint8_t *out, uint16_t payload_len,
				uint16_t src, uint16_t dst, uint8_t key_aid,
				uint32_t seq, uint32_t iv_index,
				bool aszmic,
				const uint8_t app_key[16])
{
	uint8_t nonce[13];

	if (payload_len < 1)
		return false;

	if (key_aid == APP_AID_DEV)
		mesh_crypto_device_nonce(seq, src, dst, iv_index, aszmic,
									nonce);
	else
		mesh_crypto_application_nonce(seq, src, dst, iv_index, aszmic,
									nonce);

	if (!mesh_crypto_aes_ccm_encrypt(nonce, app_key,
							aad, aad ? 16 : 0,
							payload, payload_len,
							out, NULL,
							aszmic ? 8 : 4))
		return false;

	return true;
}

bool mesh_crypto_payload_decrypt(uint8_t *aad, uint16_t aad_len,
				const uint8_t *payload, uint16_t payload_len,
				bool aszmic,
				uint16_t src, uint16_t dst,
				uint8_t key_aid, uint32_t seq,
				uint32_t iv_index, uint8_t *out,
				const uint8_t app_key[16])
{
	uint8_t nonce[13];
	uint32_t mic32;
	uint64_t mic64;

	if (payload_len < 5 || !out)
		return false;

	if (key_aid == APP_AID_DEV)
		mesh_crypto_device_nonce(seq, src, dst, iv_index, aszmic,
									nonce);
	else
		mesh_crypto_application_nonce(seq, src, dst, iv_index, aszmic,
									nonce);

	memcpy(out, payload, payload_len);

	if (aszmic) {
		if (!mesh_crypto_aes_ccm_decrypt(nonce, app_key,
					aad, aad_len,
					payload, payload_len,
					out, &mic64, sizeof(mic64)))
			return false;

		mic64 ^= l_get_be64(out + payload_len - 8);
		l_put_be64(mic64, out + payload_len - 8);

		if (mic64)
			return false;
	} else {
		if (!mesh_crypto_aes_ccm_decrypt(nonce, app_key,
					aad, aad_len,
					payload, payload_len,
					out, &mic32, sizeof(mic32)))
			return false;

		mic32 ^= l_get_be32(out + payload_len - 4);
		l_put_be32(mic32, out + payload_len - 4);

		if (mic32)
			return false;
	}

	return true;
}

static bool mesh_crypto_packet_encrypt(uint8_t *packet, uint8_t packet_len,
				const uint8_t network_key[16],
				uint32_t iv_index, bool proxy,
				bool ctl, uint8_t ttl, uint32_t seq,
				uint16_t src)
{
	uint8_t nonce[13];

	/* Detect Proxy packet by CTL == true && DST == 0x0000 */
	if (ctl && proxy)
		mesh_crypto_proxy_nonce(seq, src, iv_index, nonce);
	else
		mesh_crypto_network_nonce(ctl, ttl, seq, src, iv_index, nonce);

	/* Check for Long net-MIC */
	if (ctl) {
		if (!mesh_crypto_aes_ccm_encrypt(nonce, network_key,
					NULL, 0,
					packet + 7, packet_len - 7 - 8,
					packet + 7, NULL, 8))
			return false;
	} else {
		if (!mesh_crypto_aes_ccm_encrypt(nonce, network_key,
					NULL, 0,
					packet + 7, packet_len - 7 - 4,
					packet + 7, NULL, 4))
			return false;
	}

	return true;
}

bool mesh_crypto_packet_encode(uint8_t *packet, uint8_t packet_len,
				uint32_t iv_index,
				const uint8_t network_key[16],
				const uint8_t privacy_key[16])
{
	bool ctl;
	uint8_t ttl;
	uint32_t seq;
	uint16_t src;
	uint16_t dst;

	if (!network_header_parse(packet, packet_len,
						&ctl, &ttl, &seq, &src, &dst))
		return false;

	if (!mesh_crypto_packet_encrypt(packet, packet_len, network_key,
							iv_index, !dst,
							ctl, ttl, seq, src))

		return false;

	return mesh_crypto_network_obfuscate(packet, privacy_key, iv_index,
							ctl, ttl, seq, src);
}

static bool mesh_crypto_packet_decrypt(uint8_t *packet, uint8_t packet_len,
				const uint8_t network_key[16],
				uint32_t iv_index, bool proxy,
				bool ctl, uint8_t ttl, uint32_t seq,
				uint16_t src)
{
	uint8_t nonce[13];

	/* Pre-check SRC address for illegal values */
	if (!IS_UNICAST(src))
		return false;

	/* Detect Proxy packet by CTL == true && proxy == true */
	if (ctl & proxy)
		mesh_crypto_proxy_nonce(seq, src, iv_index, nonce);
	else
		mesh_crypto_network_nonce(ctl, ttl, seq, src, iv_index, nonce);

	/* Check for Long MIC */
	if (ctl) {
		uint64_t mic;

		if (!mesh_crypto_aes_ccm_decrypt(nonce, network_key,
					NULL, 0,
					packet + 7, packet_len - 7,
					packet + 7, &mic, sizeof(mic)))
			return false;

		mic ^= l_get_be64(packet + packet_len - 8);
		l_put_be64(mic, packet + packet_len - 8);

		if (mic)
			return false;
	} else {
		uint32_t mic;

		if (!mesh_crypto_aes_ccm_decrypt(nonce, network_key,
					NULL, 0,
					packet + 7, packet_len - 7,
					packet + 7, &mic, sizeof(mic)))
			return false;

		mic ^= l_get_be32(packet + packet_len - 4);
		l_put_be32(mic, packet + packet_len - 4);

		if (mic)
			return false;
	}

	return true;
}

bool mesh_crypto_packet_decode(const uint8_t *packet, uint8_t packet_len,
				bool proxy, uint8_t *out, uint32_t iv_index,
				const uint8_t network_key[16],
				const uint8_t privacy_key[16])
{
	bool ctl;
	uint8_t ttl;
	uint32_t seq;
	uint16_t src;

	if (packet_len < 14)
		return false;

	memcpy(out, packet, packet_len);

	if (!mesh_crypto_network_clarify(out, privacy_key, iv_index,
						&ctl, &ttl, &seq, &src))
		return false;

	return mesh_crypto_packet_decrypt(out, packet_len, network_key,
							iv_index, proxy,
							ctl, ttl, seq, src);
}

bool mesh_crypto_packet_label(uint8_t *packet, uint8_t packet_len,
				uint16_t iv_index, uint8_t network_id)
{
	packet[0] = (iv_index & 0x0001) << 7 | (network_id & 0x7f);

	return true;
}

/* reversed, 8-bit, poly=0x07 */
static const uint8_t crc_table[256] = {
	0x00, 0x91, 0xe3, 0x72, 0x07, 0x96, 0xe4, 0x75,
	0x0e, 0x9f, 0xed, 0x7c, 0x09, 0x98, 0xea, 0x7b,
	0x1c, 0x8d, 0xff, 0x6e, 0x1b, 0x8a, 0xf8, 0x69,
	0x12, 0x83, 0xf1, 0x60, 0x15, 0x84, 0xf6, 0x67,

	0x38, 0xa9, 0xdb, 0x4a, 0x3f, 0xae, 0xdc, 0x4d,
	0x36, 0xa7, 0xd5, 0x44, 0x31, 0xa0, 0xd2, 0x43,
	0x24, 0xb5, 0xc7, 0x56, 0x23, 0xb2, 0xc0, 0x51,
	0x2a, 0xbb, 0xc9, 0x58, 0x2d, 0xbc, 0xce, 0x5f,

	0x70, 0xe1, 0x93, 0x02, 0x77, 0xe6, 0x94, 0x05,
	0x7e, 0xef, 0x9d, 0x0c, 0x79, 0xe8, 0x9a, 0x0b,
	0x6c, 0xfd, 0x8f, 0x1e, 0x6b, 0xfa, 0x88, 0x19,
	0x62, 0xf3, 0x81, 0x10, 0x65, 0xf4, 0x86, 0x17,

	0x48, 0xd9, 0xab, 0x3a, 0x4f, 0xde, 0xac, 0x3d,
	0x46, 0xd7, 0xa5, 0x34, 0x41, 0xd0, 0xa2, 0x33,
	0x54, 0xc5, 0xb7, 0x26, 0x53, 0xc2, 0xb0, 0x21,
	0x5a, 0xcb, 0xb9, 0x28, 0x5d, 0xcc, 0xbe, 0x2f,

	0xe0, 0x71, 0x03, 0x92, 0xe7, 0x76, 0x04, 0x95,
	0xee, 0x7f, 0x0d, 0x9c, 0xe9, 0x78, 0x0a, 0x9b,
	0xfc, 0x6d, 0x1f, 0x8e, 0xfb, 0x6a, 0x18, 0x89,
	0xf2, 0x63, 0x11, 0x80, 0xf5, 0x64, 0x16, 0x87,

	0xd8, 0x49, 0x3b, 0xaa, 0xdf, 0x4e, 0x3c, 0xad,
	0xd6, 0x47, 0x35, 0xa4, 0xd1, 0x40, 0x32, 0xa3,
	0xc4, 0x55, 0x27, 0xb6, 0xc3, 0x52, 0x20, 0xb1,
	0xca, 0x5b, 0x29, 0xb8, 0xcd, 0x5c, 0x2e, 0xbf,

	0x90, 0x01, 0x73, 0xe2, 0x97, 0x06, 0x74, 0xe5,
	0x9e, 0x0f, 0x7d, 0xec, 0x99, 0x08, 0x7a, 0xeb,
	0x8c, 0x1d, 0x6f, 0xfe, 0x8b, 0x1a, 0x68, 0xf9,
	0x82, 0x13, 0x61, 0xf0, 0x85, 0x14, 0x66, 0xf7,

	0xa8, 0x39, 0x4b, 0xda, 0xaf, 0x3e, 0x4c, 0xdd,
	0xa6, 0x37, 0x45, 0xd4, 0xa1, 0x30, 0x42, 0xd3,
	0xb4, 0x25, 0x57, 0xc6, 0xb3, 0x22, 0x50, 0xc1,
	0xba, 0x2b, 0x59, 0xc8, 0xbd, 0x2c, 0x5e, 0xcf
};

uint8_t mesh_crypto_compute_fcs(const uint8_t *packet, uint8_t packet_len)
{
	uint8_t fcs = 0xff;
	int i;

	for (i = 0; i < packet_len; i++)
		fcs = crc_table[fcs ^ packet[i]];

	return 0xff - fcs;
}

bool mesh_crypto_check_fcs(const uint8_t *packet, uint8_t packet_len,
							uint8_t received_fcs)
{
	uint8_t fcs = 0xff;
	int i;

	for (i = 0; i < packet_len; i++)
		fcs = crc_table[fcs ^ packet[i]];

	fcs = crc_table[fcs ^ received_fcs];

	return fcs == 0xcf;
}

/* This function performs a quick-check of ELL and Kernel AEAD encryption.
 * Some kernel versions before v4.9 have a known AEAD bug. If the system
 * running this test is using a v4.8 or earlier kernel, a failure here is
 * likely unless AEAD encryption has been backported.
 */
static const uint8_t crypto_test_result[] = {
	0x75, 0x03, 0x7e, 0xe2, 0x89, 0x81, 0xbe, 0x59,
	0xbc, 0xe6, 0xdd, 0x23, 0x63, 0x5b, 0x16, 0x61,
	0xb7, 0x23, 0x92, 0xd4, 0x86, 0xee, 0x84, 0x29,
	0x9a, 0x2a, 0xbf, 0x96
};

bool mesh_crypto_check_avail()
{
	void *cipher;
	bool result;
	uint8_t i;
	union {
		struct {
			uint8_t key[16];
			uint8_t aad[16];
			uint8_t nonce[13];
			uint8_t data[20];
			uint8_t mic[8];
		} crypto;
		uint8_t bytes[73];
	} u;
	uint8_t out_msg[sizeof(u.crypto.data) + sizeof(u.crypto.mic)];

	l_debug("Testing Crypto");
	for (i = 0; i < sizeof(u); i++) {
		u.bytes[i] = 0x60 + i;
	}

	cipher = l_aead_cipher_new(L_AEAD_CIPHER_AES_CCM, u.crypto.key,
				sizeof(u.crypto.key), sizeof(u.crypto.mic));

	if (!cipher)
		return false;

	result = l_aead_cipher_encrypt(cipher,
				u.crypto.data, sizeof(u.crypto.data),
				u.crypto.aad, sizeof(u.crypto.aad),
				u.crypto.nonce, sizeof(u.crypto.nonce),
				out_msg, sizeof(out_msg));

	if (result)
		result = !memcmp(out_msg, crypto_test_result, sizeof(out_msg));

	l_aead_cipher_free(cipher);

	return result;
}
