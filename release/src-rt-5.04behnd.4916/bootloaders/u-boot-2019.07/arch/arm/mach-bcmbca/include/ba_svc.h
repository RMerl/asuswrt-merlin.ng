/* SPDX-License-Identifier: GPL-2.0+
  *
  * Copyright 2019 Broadcom Ltd.
 */

#ifndef _BA_SVC_H_
#define _BA_SVC_H_

#include <linux/types.h>
#include "itc_rpc.h"

enum ba_svc_func_idx
{
    BA_ARMTF_UBOOT_BEGIN = 20,
    BA_SVC_BOOT_FROM_ADDR = BA_ARMTF_UBOOT_BEGIN,

    /* ATTENTION:
     *
     * All RPC commands shared between ARMTF and Uboot should be added above this line
     * and synced with .../armtf/plat/bcm/itc_rpc/include/ba_svc.h
     *
     * */
    BA_SEC_BEGIN = 30,
    BA_SVC_GET_SEC_STATE = BA_SEC_BEGIN,
    BA_SVC_GET_DEV_SPEC_KEY,
    BA_SVC_SEC_HANDLE_CERTIFICATE,
    BA_SVC_SEC_GET_SKS_STATS,
    BA_SVC_SEC_RSA_SIG_VERIFY,
    BA_SVC_SEC_AES_CRYPT,
    BA_SVC_SEC_GET_KEY_SIZE,

    /* ATTENTION:
     *
     * All Security related RPC commands should be added above this line
     *
     * */
    BA_UBOOT_LINUX_BEGIN = 50,
    BA_SVC_XPORT_SET_PWR = BA_UBOOT_LINUX_BEGIN,
    BA_SVC_GET_SMCBL_VER,
    BA_SVC_GET_SMCBL_VER_HASH,
    BA_SVC_GET_SMCOS_VER,
    BA_SVC_GET_SMCOS_VER_HASH,
    BA_SVC_RPRT_BOOT_SUCCESS,

    /* ATTENTION:
     *
     * All RPC commands shared between Uboot and Linux should be added above this line
     * and synced with bcmdrivers/opensource/include/bcm683xx/ba_rpc_svc.h
     *
     * */
    BA_SVC_FUNC_MAX
};

enum ba_req_rs_rsp {
	BA_SVC_RESPONSE_READY,
	BA_SVC_RESPONSE_BUSY,
	BA_SVC_RESPONSE_MAX
};

#define BA_SVC_RS_OFF		"OFF"
#define BA_SVC_RS_RESET		"RESET"
#define BA_SVC_RS_BOOT		"BOOT"
#define BA_SVC_RS_SHUTDOWN	"SHUTDOWN"
#define BA_SVC_RS_RUNNING	"RUNNING"
#define BA_SVC_RS_READY		"READY"

extern uint32_t ba_rs_off;
extern uint32_t ba_rs_reset;
extern uint32_t ba_rs_boot;
extern uint32_t ba_rs_shutdown;
extern uint32_t ba_rs_running;
extern uint32_t ba_rs_ready;

#define INVALID_ID	(0xffffffff)

struct ba_msg {
	uint32_t	hdr;
	union {
		uint32_t	rsvd0;
		struct {
			uint8_t	cpu_id;
			uint8_t	rs_id;
			union {
				uint8_t	be_rude:1;
				uint8_t	rsvd1:7;

				uint8_t	response:4;
				uint8_t rsvd2:4;
			};
			uint8_t	rc:8;
		};
	};
	union {
		uint32_t	rsvd3[2];
		char		name[8];
	};
};


/* 
    This is configurable parameter in git core.abbrev parameter currently it configure to 9 use more 3 byte for spare
    In case of changing lenght of this parameters need  to change it also in the SMC_OS and SMC_OS
*/
#define HASH_SHORT_SIZE         (12)

typedef struct 
{
    uint16_t       smcos_major_ver;
    uint16_t       smcos_minor_ver;
    uint16_t       smcos_rev;
    uint16_t       pon_major_ver;
    uint16_t       pon_minor_ver;
    uint16_t       pon_patch_ver; 
    char           smcos_ver_hash[HASH_SHORT_SIZE]; 
} smcos_ver_t;

typedef struct 
{
    uint16_t       smcbl_major_ver;
    uint16_t       smcbl_minor_ver;
    uint16_t       smcbl_rev;
    uint16_t       ponbl_major_ver;
    uint16_t       ponbl_minor_ver;
    char           smcbl_ver_hash[HASH_SHORT_SIZE];
} smcbl_ver_t;

#define BA_SVC_RESET_BOOT_WDOG	1
#define BA_SVC_RESET_BOOT_COUNT	2

/* ba_svc rpc message manipulation helpers */
static inline uint8_t ba_svc_msg_get_retcode(rpc_msg *msg)
{
	struct ba_msg *ba_msg = (struct ba_msg *)msg;
	return ba_msg->rc;
}
static inline void ba_svc_msg_set_retcode(rpc_msg *msg, uint8_t v)
{
	struct ba_msg *ba_msg = (struct ba_msg *)msg;
	ba_msg->rc = v;
}

/* ba svc functions */
int ba_svc_boot_secondary(uint32_t cpu_mask, uint32_t vector);
int ba_xport_set_state(uint8_t port_id, uint8_t enable);
int ba_get_smcbl_ver(smcbl_ver_t  *smcbl_ver);
int ba_get_smcos_ver(smcos_ver_t  *smcos_ver);
int bcm_rpc_ba_report_boot_success(uint32_t flags);
int bcm_rpc_ba_get_sec_state(void);
int ba_get_dev_spec_key(void** dev_key, int* ek_size, int* iv_size);



/* 
    ba svc security functions 
*/

/**
 * struct sks_stats - Secure Key Store Statistics
 * 
 * @version: Version of the Secure Key Store
 * @entries: Quantity of Crypto Materials Entries
 * @epoch: Security Epoch Level
 */
struct sks_stats {
    uint32_t version;
    uint32_t entries;
    uint32_t epoch;
} __attribute__ ((packed));

/**
 * struct sec_rsa_sig_verify_descriptor - RSA Signature Verification Request
 *
 * @key_name_hint: Key Name Hint
 * @crypto_options: Options to be used for Crypto Operation
 * @data_addr: Address of the Data Buffer to be verified
 * @data_size: Size of the Data Buffer
 * @signature_addr: Address of the Signature Buffer
 * @signature_size: Size of the Signature Buffer
 */
struct sec_rsa_sig_verify_descriptor {
    uint32_t key_name_hint;
    uint32_t crypto_options;
    uint64_t data_addr;
    uint32_t data_size;
    uint64_t signature_addr;
    uint32_t signature_size;
} __attribute__ ((packed));

/**
 * struct sec_aes_crypt_descriptor - AES Encryption/Decryption Crypto Request Descriptor
 *
 * @key_name_hint: Key Name Hint
 * @crypto_options: Options to be used for Crypto Operation
 * @data_src_addr: Address of the Data Buffer to be verified
 * @data_dst_addr: Address of the Data Buffer to be verified
 * @data_size: Size of the Data Buffer
 */
struct sec_aes_crypt_descriptor {
    uint32_t key_name_hint;
    uint32_t crypto_options;
    uint64_t data_src_addr;
    uint64_t data_dst_addr;
    uint32_t data_size;
} __attribute__ ((packed));

enum sec_aes_crypto_options {
    SEC_AES_CRYPT_DECRYPT       = (1 << 0),
    SEC_AES_CRYPT_ENCRYPT       = (1 << 1),
};

int bcm_rpc_sec_handle_ksm_certificate(const uint8_t *certificate, uint32_t certificate_size);
int bcm_rpc_sec_get_sks_stats(struct sks_stats *stats);
int bcm_rpc_sec_rsa_sig_verify(struct sec_rsa_sig_verify_descriptor *crypto_desc);
int bcm_rpc_sec_aes_crypt(struct sec_aes_crypt_descriptor *crypto_desc);
int bcm_rpc_sec_get_key_size(uint32_t key, uint32_t *size);

#endif
