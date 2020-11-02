/*
 * Wake-on-Wireless related header file
 *
 * Copyright 2020 Broadcom
 *
 * This program is the proprietary software of Broadcom and/or
 * its licensors, and may only be used, duplicated, modified or distributed
 * pursuant to the terms and conditions of a separate, written license
 * agreement executed between you and Broadcom (an "Authorized License").
 * Except as set forth in an Authorized License, Broadcom grants no license
 * (express or implied), right to use, or waiver of any kind with respect to
 * the Software, and Broadcom expressly reserves all rights in and to the
 * Software and all intellectual property rights therein.  IF YOU HAVE NO
 * AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY
 * WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF
 * THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use
 * all reasonable efforts to protect the confidentiality thereof, and to
 * use this information only in connection with your use of Broadcom
 * integrated circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES,
 * REPRESENTATIONS OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR
 * OTHERWISE, WITH RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY
 * DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY,
 * NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES,
 * ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR
 * CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING
 * OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL
 * BROADCOM OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL,
 * SPECIAL, INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR
 * IN ANY WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
 * IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii)
 * ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF
 * OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY
 * NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
 * $Id: wlc_wowlpf.h 708017 2017-06-29 14:11:45Z $
*/

#ifndef _wlc_wowlpf_h_
#define _wlc_wowlpf_h_

#ifdef WOWLPF

#include <bcmcrypto/aes.h>

#ifdef SECURE_WOWL
#include <bcmcrypto/prf.h>
#include <proto/bcmip.h>
#include <proto/bcmtcp.h>
#include <bcmutils.h>
#endif /* #ifdef SECURE_WOWL */

#define WOWL_TM_DONGLE_DOWN                  1
#define WOWL_TM_PACKET_ACK                   2
#define WOWL_TM_PACKET_KEEPALIVE             3

#define TLS_MAX_KEY_LENGTH	48
#define TLS_MAX_MAC_KEY_LENGTH	32
#define TLS_MAX_IV_LENGTH	32
#define TLS_MAX_DEGIST_LENGTH	32
#define TLS_MAX_SEQUENCE_LENGTH	8

/* add supported cipher suite according rfc5246#appendix-A.5 */
typedef enum {
	TLS_RSA_WITH_AES_128_CBC_SHA     =     0x002F,
	TLS_RSA_WITH_AES_256_CBC_SHA     =     0x0035
} Cipher_Suite_e;
typedef enum {
    CONTENTTYPE_CHANGE_CIPHER_SPEC = 20,
    CONTENTTYPE_ALERT,
    CONTENTTYPE_HANDSHAKE,
    CONTENTTYPE_APPLICATION_DATA
} ContentType_e;
typedef enum {
	COMPRESSIONMETHOD_NULL = 0
} CompressionMethod_e;
typedef enum {
	BULKCIPHERALGORITHM_NULL,
	BULKCIPHERALGORITHM_RC4,
	BULKCIPHERALGORITHM_3DES,
	BULKCIPHERALGORITHM_AES
} BulkCipherAlgorithm_e;
typedef enum {
	CIPHERTYPE_STREAM = 1,
	CIPHERTYPE_BLOCK,
	CIPHERTYPE_AEAD
} CipherType_e;
typedef enum {
	MACALGORITHM_NULL,
	MACALGORITHM_HMAC_MD5,
	MACALGORITHM_HMAC_SHA1,
	MACALGORITHM_HMAC_SHA256,
	MACALGORITHM_HMAC_SHA384,
	MACALGORITHM_HMAC_SHA512
} MACAlgorithm_e;

typedef struct {
	uint8 major;
	uint8 minor;
} ProtocolVersion;

typedef struct {
	ProtocolVersion version;
	CompressionMethod_e compression_algorithm;
	BulkCipherAlgorithm_e cipher_algorithm;
	CipherType_e cipher_type;
	MACAlgorithm_e mac_algorithm;
	uint32 keepalive_interval; /* keepalive interval, in seconds */
	uint8 read_master_key[TLS_MAX_KEY_LENGTH];
	uint32 read_master_key_len;
	uint8 read_iv[TLS_MAX_IV_LENGTH];
	uint32 read_iv_len;
	uint8 read_mac_key[TLS_MAX_MAC_KEY_LENGTH];
	uint32 read_mac_key_len;
	uint8 read_sequence[TLS_MAX_SEQUENCE_LENGTH];
	uint32 read_sequence_len;
	uint8 write_master_key[TLS_MAX_KEY_LENGTH];
	uint32 write_master_key_len;
	uint8 write_iv[TLS_MAX_IV_LENGTH];
	uint32 write_iv_len;
	uint8 write_mac_key[TLS_MAX_MAC_KEY_LENGTH];
	uint32 write_mac_key_len;
	uint8 write_sequence[TLS_MAX_SEQUENCE_LENGTH];
	uint32 write_sequence_len;
	uint32 tcp_ack_num;
	uint32 tcp_seq_num;
	uint8 local_ip[IPV4_ADDR_LEN];
	uint8 remote_ip[IPV4_ADDR_LEN];
	uint16 local_port;
	uint16 remote_port;
	uint8 local_mac_addr[ETHER_ADDR_LEN];
	uint8 remote_mac_addr[ETHER_ADDR_LEN];
	uint32 app_syncid;
} tls_param_info_t;

typedef struct {
	wlc_info_t *wlc;

	uint32 tlsparam_size;
	tls_param_info_t *tlsparam;
	uint32 size_bytes;
	uint8 *mask_and_pattern;
	uint8 block_length;
	uint8 iv_length;
	uint8 explicit_iv_length;
	uint8 digest_length;
	uint8 mac_key_length;
	uint32 read_ks[4 * (AES_MAXROUNDS + 1)];
	uint32 write_ks[4 * (AES_MAXROUNDS + 1)];
} tls_info_t;

#define TLS_RECORD_HEADER_LENGTH		5
#define TLS_RECORD_HEADER_CONTENTTYPE_LENGTH	1
#define TLS_RECORD_HEADER_VERSION_LENGTH	2

#define TLS_OFFSET_CONTENTTYPE			0
#define TLS_OFFSET_VERSION_MAJOR		1
#define TLS_OFFSET_VERSION_MINOR		2
#define TLS_OFFSET_LENGTH_HIGH			3
#define TLS_OFFSET_LENGTH_LOW			4

extern wowlpf_info_t *wlc_wowlpf_attach(wlc_info_t *wlc);
extern void wlc_wowlpf_detach(wowlpf_info_t *wowl);
extern bool wlc_wowlpf_cap(struct wlc_info *wlc);
extern bool wlc_wowlpf_enable(wowlpf_info_t *wowl);
extern uint32 wlc_wowlpf_clear(wowlpf_info_t *wowl);
extern bool wlc_wowlpf_pktfilter_cb(wlc_info_t *wlc,
	uint32 type, uint32 id, const void *patt, const void *sdu);
extern bool wlc_wowlpf_event_cb(wlc_info_t *wlc, uint32 event, uint32 reason);
#endif /* WOWLPF */

/* number of WOWL patterns supported */
#ifndef MAXPATTERNS
#define MAXPATTERNS 8
#endif /* MAXPATTERNS */

#endif /* _wlc_wowlpf_h_ */
