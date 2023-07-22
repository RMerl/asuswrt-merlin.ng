/*
<:copyright-BRCM:2019:DUAL/GPL:standard 

   Copyright (c) 2019 Broadcom 
   All Rights Reserved

Unless you and Broadcom execute a separate written software license
agreement governing use of this software, this software is licensed
to you under the terms of the GNU General Public License version 2
(the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
with the following added to such license:

   As a special exception, the copyright holders of this software give
   you permission to link this software with independent modules, and
   to copy and distribute the resulting executable under terms of your
   choice, provided that you also meet, for each linked independent
   module, the terms and conditions of the license of that module.
   An independent module is a module which is not derived from this
   software.  The special exception does not apply to any modifications
   of the software.

Not withstanding the above, under no circumstances may you combine
this software in any way with any other Broadcom software provided
under a license other than the GPL, without Broadcom's express prior
written consent.

:> 
*/
#ifndef TEE_SOTP_SER_CORE_H
#define TEE_SOTP_SER_CORE_H

#include <platform_config.h>
#include <bcm_sotp.h>

#define SOTP_SER_START_ROW		0
#define SOTP_SER_END_ROW		111
#define SOTP_SER_MAX_ROWS	(SOTP_SER_END_ROW - SOTP_SER_START_ROW + 1)
#define SOTP_SER_START_SECTION	 SOTP_STATUS_1__SECTOR7_CRC_FAIL

#define SOTP_SER_WRAPPER_ROW_0		8
#define SOTP_SER_WRAPPER_ROW_1		9
#define SOTP_SER_PROG_ROW_0		10
#define SOTP_SER_PROG_ROW_1		11

#define SOTP_SER_MAX_LOCK_ROWS		12
#define SOTP_SER_ROWS_PER_REGION	4
#define SOTP_SER_MAX_REGION_PER_LOCK_REG 21
#define SOTP_SER_CONFIG_REGION		7
#define SOTP_SER_DWORD0_REGION		13
#define SOTP_SER_DWORD1_REGION		19
#define SOTP_SER_MAX_REGION		25
#define SOTP_SER_LOCK_BIT_SIZE		41
#define SOTP_SER_REG_PER_LOCK_REG	16

#define SOTP_SER_KEY_SIZE		32
#define SOTP_SER_KEY_ROWS_COUNT	8
#define SOTP_SER_CONFIG_ROW_SIZE	4
#define SOTP_SER_DEV_CONFIG_0_ROW	12
#define SOTP_SER_DEV_CONFIG_1_ROW	13
#define SOTP_SER_DEV_SEC_CONFIG_ROW	16
#define SOTP_SER_PROD_ID_ROW		17
#define SOTP_SER_CUST_CONFIG_0_ROW	20
#define SOTP_SER_CUST_CONFIG_1_ROW	21
#define SOTP_SER_CUST_CONFIG_2_ROW	22
#define SOTP_SER_CUST_CONFIG_3_ROW	23
#define SOTP_SER_CUST_DEV_CONFIG_ROW	24
/*
 * Keys section have different offsets in the IPROC family of processors.
 * So, the current option is to define the start of key offset in
 * platform_config.h file. In future, the key offset could be obtained from
 * the components like boot1 or ATF at run-time.
 */
#define SOTP_SER_DAUTH_ROW_BEGIN	(SOTP_KEY_SEC_START)
#define SOTP_SER_DAUTH_ROW_END		(SOTP_SER_DAUTH_ROW_BEGIN + \
					SOTP_KEY_SEC_SIZE - 1)
#define SOTP_SER_HMAC_ROW_BEGIN		(SOTP_SER_DAUTH_ROW_END + 1)
#define SOTP_SER_HMAC_ROW_END		(SOTP_SER_HMAC_ROW_BEGIN + \
					SOTP_KEY_SEC_SIZE - 1)
#define SOTP_SER_AES_ROW_BEGIN		(SOTP_SER_HMAC_ROW_END + 1)
#define SOTP_SER_AES_ROW_END		(SOTP_SER_AES_ROW_BEGIN + \
					SOTP_KEY_SEC_SIZE - 1)
#define SOTP_SER_CUST_REG_1_BEGIN	(SOTP_SER_AES_ROW_END + 1)
#define SOTP_SER_CUST_REG_1_END		(SOTP_SER_CUST_REG_1_BEGIN + \
					SOTP_KEY_SEC_SIZE - 1)
#define SOTP_SER_CUST_REG_2_BEGIN	(SOTP_SER_CUST_REG_1_END + 1)
#define SOTP_SER_CUST_REG_2_END		(SOTP_SER_CUST_REG_2_BEGIN + \
					SOTP_KEY_SEC_SIZE - 1)
#define SOTP_SER_CUST_REG_3_BEGIN	(SOTP_SER_CUST_REG_2_END + 1)
#define SOTP_SER_CUST_REG_3_END		(SOTP_SER_CUST_REG_3_BEGIN + \
					SOTP_KEY_SEC_SIZE - 1)
#define SOTP_SER_CUST_REG_4_BEGIN	(SOTP_SER_CUST_REG_3_END + 1)
#define SOTP_SER_CUST_REG_4_END		(SOTP_SER_CUST_REG_4_BEGIN + \
					SOTP_KEY_SEC_SIZE - 1)

#define SOTP_DEV_CFG_ISAO		0x300000

#define SOTP_SER_FLAGS_GENERATE		0x00000001
#define SOTP_SER_FLAGS_WRITE_LOCK	0x00000002
#define SOTP_SER_FLAGS_READ_LOCK	0x00000004
#define SOTP_SER_FLAGS_SKIP_ECC		0x00000008

#define SOTP_SER_WR_LOCK_BIT_0		0x1
#define SOTP_SER_WR_LOCK_BIT_1		0x2
#define SOTP_SER_WR_LOCK_BITS		(SOTP_SER_WR_LOCK_BIT_0 | \
					SOTP_SER_WR_LOCK_BIT_1)

#define SOTP_SER_KEY_REG_0		1
#define SOTP_SER_KEY_REG_1		2
#define SOTP_SER_KEY_REG_2		4

#define SOTP_FAIL_BITS			0x18000000000

#define SOTP_SER_CID_TYPE_MASK		(0xF << 28)
#define SOTP_SER_CID_TYPE_DEV		(0x9 << 28)
#define SOTP_SER_CID_TYPE_PROD		(0x6 << 28)

#define SOTP_SER_CFG_ABPROD_CUSTID	0x00000000000000cf
#define SOTP_SER_CFG_ABPROD_ENC_TYPE	0x000001e799800000
#define SOTP_SER_CFG_ABDEV_CUSTID	0x00000000000003cc
#define SOTP_SER_CFG_ABDEV_ENC_TYPE	0x19999800000

#define SOTP_SER_KEY_INFO_RSA2048	0x0010
#define SOTP_SER_KEY_ALG_RSA		0x0001
#define SOTP_SER_KEY_RSA2048		(SOTP_SER_KEY_ALG_RSA | \
					SOTP_SER_KEY_INFO_RSA2048)

#define SOTP_SER_SHA256_HASH_SIZE	32
#define SOTP_SER_RSA_KEY_MOD_SIZE	256

#define SOTP_SER_DAUTH_BLOB_SIZE	((3 * sizeof(uint32_t)) + \
					SOTP_SER_RSA_KEY_MOD_SIZE)

#define MAX_ASN1_SEQUENCE_SIZE		40
#define MAX_PUB_KEY_LEN			(SOTP_SER_RSA_KEY_MOD_SIZE + \
					MAX_ASN1_SEQUENCE_SIZE)

typedef enum {
	SOTP_SER_OK,
	SOTP_SER_ROW_EMPTY,
	SOTP_SER_ROW_VALID,
	SOTP_SER_NOT_SUPPORTED = 11,
	SOTP_SER_ACCESS_ERROR,
	SOTP_SER_INVALID_COMMAND,
	SOTP_SER_INVALID_ROW_ADDR,
	SOTP_SER_INVALID_NUM_OF_ROWS,
	SOTP_SER_INVALID_FLAGS,
	SOTP_SER_INIT_ERROR,
	SOTP_SER_VERIFY_DATA_ERROR = 120,
	SOTP_SER_VERIFY_CRC_ERROR,
	SOTP_SER_VERIFY_ECC_ERROR,
	SOTP_SER_WRITE_DATA_ERROR = 130,
	SOTP_SER_WRITE_ECC_ERROR,
	SOTP_SER_WRITE_CRC_ERROR,
	SOTP_SER_WRITE_GEN_ERROR,
	SOTP_SER_WRITE_NOT_BLANK_ERROR,
	SOTP_SER_LOCK_ERROR = 140,
	SOTP_SER_INVALID_RET_VAL_ADDR,
	SOTP_SER_ROW_READ_ERROR,
	SOTP_SER_KEY_READ_ERROR,
	SOTP_SER_KEY_WRITE_ERROR,
	SOTP_SER_DAUTH_CREATE_ERROR,
	SOTP_SER_DAUTH_UPDATE_ERROR,
	SOTP_SER_CUST_ID_UPDATE_ERROR,
	SOTP_SER_DEV_STATUS_UPDATE_ERROR,
	SOTP_SER_SBL_CONFIG_UPDATE_ERROR,
	SOTP_SER_INVALID_ARGS,
	SOTP_SER_CUST_CONFIG_UPDATE_ERROR,
} sotp_ser_e;

typedef enum {
	OTP_SECTION_INVALID,
	OTP_SECTION_CFG,
	OTP_SECTION_BOOT_KEY,
	OTP_SECTION_CUST_KEY,
	OTP_SECTION_RESD_KEY,
} sotp_section_type_e;

typedef enum {
	SOTP_CRC_STATUS_OK,
	SOTP_CRC_STATUS_INVALID_ROW,
	SOTP_CRC_STATUS_ERROR
} sotp_crc_status_e;

typedef enum {
	OTPKEY_AES,
	OTPKEY_HMAC_SHA256,
	OTPKEY_DAUTH,
	OTPKEY_DevIdentity,
	OTPKEY_Binding
} otp_key_type_e;

typedef union {
	uint8_t data8[8];
	uint16_t data16[4];
	uint32_t data32[2];
	uint64_t data64;
} sotp_ser_data_t;

typedef struct {
	int32_t sotp_init;
	int32_t sotp_access;
	vaddr_t *sotp_base;
	sotp_ser_data_t row;
} sotp_state_t;

typedef struct {
	uint16_t brcm_rev_id;
	uint16_t dev_sec_config;
	uint16_t product_id;
	uint16_t Reserved;
} dev_config_t;

typedef enum {
	IPROC_OTP_INVALID,
	IPROC_OTP_VALID,
	IPROC_OTP_ALLONE,
	IPROC_OTP_ERASED,
	IPROC_OTP_FASTAUTH,
	IPROC_OTP_NOFASTAUTH,
	IPROC_OTP_NONACCESSIBLE
} OTP_STATUS;

//uint32_t bcm_get_dauth_blob_len(uint32_t key_len);
//#ifdef CFG_WITH_ARM_TRUSTED_FW
//uint32_t bcm_create_dauth_blob_atf(uint8_t *dauth_blob, uint8_t *pub_key_mod,
//		uint32_t pub_key_mod_len);
//#else
//TEE_Result bcm_create_dauth_blob(const uint8_t *pub_key_mod,
//				const uint32_t pub_key_mod_len,
//				const uint32_t cust_id,
//				const uint16_t prod_id,
//				const uint16_t brcm_rev_id,
//				uint8_t *dauth_blob, uint32_t dauth_len);
//#endif
TEE_Result bcm_check_sotp_access(sotp_state_t *sotp_state);
#ifdef USE_HW_CRC		
TEE_Result bcm_sotp_get_crc_status( int row);
#endif
//TEE_Result bcm_get_sha256_hash(uint8_t *input, uint32_t input_len,
//				uint8_t *output, uint32_t *output_len);
//OTP_STATUS bcm_read_dev_cfg( dev_config_t *dev_config);
//OTP_STATUS bcm_read_cust_id( uint32_t *cust_id);
//OTP_STATUS bcm_read_cust_config( uint16_t *cust_rev_id,
//				uint16_t *sbi_rev_id);
//OTP_STATUS bcm_set_cust_id(sotp_state_t *sotp_state, uint32_t cust_id);
//OTP_STATUS bcm_set_sbl_config(sotp_state_t *sotp_state, uint32_t sbl_config);
//OTP_STATUS bcm_set_cust_dev_cfg(sotp_state_t *sotp_state,
//				uint16_t cust_rev_id, uint16_t sbi_rev_id);
//OTP_STATUS bcm_set_config_abprod(void);
//OTP_STATUS bcm_set_config_abdev(void);
//OTP_STATUS bcm_read_keys(sotp_state_t *sotp_state, uint8_t *key,
//				uint16_t *key_size, otp_key_type_e type);
//OTP_STATUS bcm_set_keys(sotp_state_t *sotp_state, uint32_t *key,
//				uint16_t key_size, otp_key_type_e type);
OTP_STATUS bcm_read_data_internal(sotp_state_t *sotp_state, uint8_t *data,
				uint32_t num_of_rows, uint32_t row_addr,
				uint32_t row_type, uint32_t skip_ecc);
OTP_STATUS write_data_internal(sotp_state_t *sotp_state, uint32_t *data,
				uint32_t num_of_rows, uint32_t row_addr,
				uint32_t row_type, uint32_t skip_ecc);
uint32_t bcm_set_lock_internal(sotp_state_t *sotp_state, uint32_t row,
				 uint32_t num_of_rows, uint32_t flags);
uint32_t map_addr_2_section(uint32_t row_addr, uint32_t num_of_rows);

#endif /* TEE_SOTP_SER_CORE_H */
