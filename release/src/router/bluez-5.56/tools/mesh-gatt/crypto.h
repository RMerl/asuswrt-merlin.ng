/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2017  Intel Corporation. All rights reserved.
 *
 *
 */

#include <stdbool.h>
#include <stdint.h>

bool mesh_crypto_aes_ccm_encrypt(const uint8_t nonce[13], const uint8_t key[16],
					const uint8_t *aad, uint16_t aad_len,
					const uint8_t *msg, uint16_t msg_len,
					uint8_t *out_msg, void *out_mic,
					size_t mic_size);
bool mesh_crypto_aes_ccm_decrypt(const uint8_t nonce[13], const uint8_t key[16],
				const uint8_t *aad, uint16_t aad_len,
				const uint8_t *enc_msg, uint16_t enc_msg_len,
				uint8_t *out_msg, void *out_mic,
				size_t mic_size);
bool mesh_crypto_nkik(const uint8_t network_key[16], uint8_t identity_key[16]);
bool mesh_crypto_nkbk(const uint8_t network_key[16], uint8_t beacon_key[16]);
bool mesh_crypto_identity(const uint8_t net_key[16], uint16_t addr,
							uint8_t id[16]);
bool mesh_crypto_identity_check(const uint8_t net_key[16], uint16_t addr,
							uint8_t id[16]);
bool mesh_crypto_beacon_cmac(const uint8_t encryption_key[16],
				const uint8_t network_id[16],
				uint32_t iv_index, bool kr, bool iu,
				uint64_t *cmac);
bool mesh_crypto_network_nonce(bool frnd, uint8_t ttl, uint32_t seq,
				uint16_t src, uint32_t iv_index,
				uint8_t nonce[13]);
bool mesh_crypto_network_encrypt(bool ctl, uint8_t ttl,
				uint32_t seq, uint16_t src,
				uint32_t iv_index,
				const uint8_t net_key[16],
				const uint8_t *enc_msg, uint8_t enc_msg_len,
				uint8_t *out, void *net_mic);
bool mesh_crypto_network_decrypt(bool frnd, uint8_t ttl,
				uint32_t seq, uint16_t src,
				uint32_t iv_index,
				const uint8_t net_key[16],
				const uint8_t *enc_msg, uint8_t enc_msg_len,
				uint8_t *out, void *net_mic, size_t mic_size);
bool mesh_crypto_application_nonce(uint32_t seq, uint16_t src,
				uint16_t dst, uint32_t iv_index,
				bool aszmic, uint8_t nonce[13]);
bool mesh_crypto_device_nonce(uint32_t seq, uint16_t src,
				uint16_t dst, uint32_t iv_index,
				bool aszmic, uint8_t nonce[13]);
bool mesh_crypto_application_encrypt(uint8_t akf, uint32_t seq, uint16_t src,
					uint16_t dst, uint32_t iv_index,
					const uint8_t app_key[16],
					const uint8_t *aad, uint8_t aad_len,
					const uint8_t *msg, uint8_t msg_len,
					uint8_t *out, void *app_mic,
					size_t mic_size);
bool mesh_crypto_application_decrypt(uint8_t akf, uint32_t seq, uint16_t src,
				uint16_t dst, uint32_t iv_index,
				const uint8_t app_key[16],
				const uint8_t *aad, uint8_t aad_len,
				const uint8_t *enc_msg, uint8_t enc_msg_len,
				uint8_t *out, void *app_mic, size_t mic_size);
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
bool mesh_crypto_packet_encode(uint8_t *packet, uint8_t packet_len,
				const uint8_t network_key[16],
				uint32_t iv_index,
				const uint8_t privacy_key[16]);
bool mesh_crypto_packet_decode(const uint8_t *packet, uint8_t packet_len,
				bool proxy, uint8_t *out, uint32_t iv_index,
				const uint8_t network_key[16],
				const uint8_t privacy_key[16]);

bool mesh_crypto_aes_cmac(const uint8_t key[16], const uint8_t *msg,
					size_t msg_len, uint8_t res[16]);

bool mesh_get_random_bytes(void *buf, size_t num_bytes);
