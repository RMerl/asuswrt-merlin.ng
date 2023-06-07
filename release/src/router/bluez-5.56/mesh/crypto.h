/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2018  Intel Corporation. All rights reserved.
 *
 *
 */

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

bool mesh_crypto_aes_ccm_encrypt(const uint8_t nonce[13], const uint8_t key[16],
					const uint8_t *aad, uint16_t aad_len,
					const void *msg, uint16_t msg_len,
					void *out_msg,
					void *out_mic, size_t mic_size);
bool mesh_crypto_aes_ccm_decrypt(const uint8_t nonce[13], const uint8_t key[16],
				const uint8_t *aad, uint16_t aad_len,
				const void *enc_msg, uint16_t enc_msg_len,
				void *out_msg,
				void *out_mic, size_t mic_size);
bool mesh_aes_ecb_one(const uint8_t key[16],
			const uint8_t plaintext[16], uint8_t encrypted[16]);
bool mesh_crypto_nkik(const uint8_t network_key[16], uint8_t identity_key[16]);
bool mesh_crypto_nkbk(const uint8_t network_key[16], uint8_t beacon_key[16]);
bool mesh_crypto_nkpk(const uint8_t network_key[16], uint8_t proxy_key[16]);
bool mesh_crypto_identity(const uint8_t net_key[16], uint16_t addr,
							uint8_t id[16]);
bool mesh_crypto_beacon_cmac(const uint8_t encryption_key[16],
				const uint8_t network_id[16],
				uint32_t iv_index, bool kr,
				bool iu, uint64_t *cmac);
bool mesh_crypto_device_key(const uint8_t secret[32],
						const uint8_t salt[16],
						uint8_t device_key[16]);
bool mesh_crypto_virtual_addr(const uint8_t virtual_label[16],
						uint16_t *v_addr);
bool mesh_crypto_nonce(const uint8_t secret[32],
					const uint8_t salt[16],
					uint8_t nonce[13]);
bool mesh_crypto_k1(const uint8_t ikm[16], const uint8_t salt[16],
		const void *info, size_t info_len, uint8_t okm[16]);
bool mesh_crypto_k2(const uint8_t n[16], const uint8_t *p, size_t p_len,
							uint8_t net_id[1],
							uint8_t enc_key[16],
							uint8_t priv_key[16]);
bool mesh_crypto_k3(const uint8_t n[16], uint8_t out64[8]);
bool mesh_crypto_k4(const uint8_t a[16], uint8_t out5[1]);
bool mesh_crypto_s1(const void *info, size_t len, uint8_t salt[16]);
bool mesh_crypto_prov_prov_salt(const uint8_t conf_salt[16],
					const uint8_t prov_rand[16],
					const uint8_t dev_rand[16],
					uint8_t prov_salt[16]);
bool mesh_crypto_prov_conf_key(const uint8_t secret[32],
					const uint8_t salt[16],
					uint8_t conf_key[16]);
bool mesh_crypto_session_key(const uint8_t secret[32],
					const uint8_t salt[16],
					uint8_t session_key[16]);
bool mesh_crypto_packet_build(bool ctl, uint8_t ttl,
				uint32_t seq,
				uint16_t src, uint16_t dst,
				uint8_t opcode,
				bool segmented, uint8_t key_aid,
				bool szmic, bool relay, uint16_t seqZero,
				uint8_t segO, uint8_t segN,
				const uint8_t *payload, uint8_t payload_len,
				uint8_t *packet, uint8_t *packet_len);
bool mesh_crypto_packet_parse(const uint8_t *packet, uint8_t packet_len,
				bool *ctl, uint8_t *ttl, uint32_t *seq,
				uint16_t *src, uint16_t *dst,
				uint32_t *cookie, uint8_t *opcode,
				bool *segmented, uint8_t *key_aid,
				bool *szmic, bool *relay, uint16_t *seqZero,
				uint8_t *segO, uint8_t *segN,
				const uint8_t **payload, uint8_t *payload_len);
bool mesh_crypto_payload_encrypt(uint8_t *aad, const uint8_t *payload,
				uint8_t *out, uint16_t payload_len,
				uint16_t src, uint16_t dst, uint8_t key_aid,
				uint32_t seq_num, uint32_t iv_index,
				bool aszmic,
				const uint8_t application_key[16]);
bool mesh_crypto_payload_decrypt(uint8_t *aad, uint16_t aad_len,
				const uint8_t *payload, uint16_t payload_len,
				bool szmict,
				uint16_t src, uint16_t dst, uint8_t key_aid,
				uint32_t seq_num, uint32_t iv_index,
				uint8_t *out,
				const uint8_t application_key[16]);
bool mesh_crypto_packet_encode(uint8_t *packet, uint8_t packet_len,
				uint32_t iv_index,
				const uint8_t network_key[16],
				const uint8_t privacy_key[16]);
bool mesh_crypto_packet_decode(const uint8_t *packet, uint8_t packet_len,
				bool proxy, uint8_t *out, uint32_t iv_index,
				const uint8_t network_key[16],
				const uint8_t privacy_key[16]);
bool mesh_crypto_packet_label(uint8_t *packet, uint8_t packet_len,
				uint16_t iv_index, uint8_t network_id);

uint8_t mesh_crypto_compute_fcs(const uint8_t *packet, uint8_t packet_len);
bool mesh_crypto_check_fcs(const uint8_t *packet, uint8_t packet_len,
							uint8_t received_fcs);
bool mesh_crypto_aes_cmac(const uint8_t key[16], const uint8_t *msg,
					size_t msg_len, uint8_t res[16]);
bool mesh_crypto_check_avail(void);
