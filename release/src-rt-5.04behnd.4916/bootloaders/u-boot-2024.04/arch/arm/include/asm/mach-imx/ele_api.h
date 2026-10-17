/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2021 NXP
 */

#ifndef __ELE_API_H__
#define __ELE_API_H__

#define ELE_VERSION    0x6
#define ELE_CMD_TAG    0x17
#define ELE_RESP_TAG   0xe1

/* ELE commands */
#define ELE_PING_REQ (0x01)
#define ELE_FW_AUTH_REQ (0x02)
#define ELE_RESTART_RST_TIMER_REQ (0x04)
#define ELE_DUMP_DEBUG_BUFFER_REQ (0x21)
#define ELE_OEM_CNTN_AUTH_REQ (0x87)
#define ELE_VERIFY_IMAGE_REQ (0x88)
#define ELE_RELEASE_CONTAINER_REQ (0x89)
#define ELE_WRITE_SECURE_FUSE_REQ (0x91)
#define ELE_FWD_LIFECYCLE_UP_REQ (0x95)
#define ELE_READ_FUSE_REQ (0x97)
#define ELE_GET_FW_VERSION_REQ (0x9D)
#define ELE_RET_LIFECYCLE_UP_REQ (0xA0)
#define ELE_GET_EVENTS_REQ (0xA2)
#define ELE_START_RNG (0xA3)
#define ELE_GENERATE_DEK_BLOB (0xAF)
#define ELE_ENABLE_PATCH_REQ (0xC3)
#define ELE_RELEASE_RDC_REQ (0xC4)
#define ELE_GET_FW_STATUS_REQ (0xC5)
#define ELE_ENABLE_OTFAD_REQ (0xC6)
#define ELE_RESET_REQ (0xC7)
#define ELE_UPDATE_OTP_CLKDIV_REQ (0xD0)
#define ELE_POWER_DOWN_REQ (0xD1)
#define ELE_ENABLE_APC_REQ (0xD2)
#define ELE_ENABLE_RTC_REQ (0xD3)
#define ELE_DEEP_POWER_DOWN_REQ (0xD4)
#define ELE_STOP_RST_TIMER_REQ (0xD5)
#define ELE_WRITE_FUSE_REQ (0xD6)
#define ELE_RELEASE_CAAM_REQ (0xD7)
#define ELE_RESET_A35_CTX_REQ (0xD8)
#define ELE_MOVE_TO_UNSECURED_REQ (0xD9)
#define ELE_GET_INFO_REQ (0xDA)
#define ELE_ATTEST_REQ (0xDB)
#define ELE_RELEASE_PATCH_REQ (0xDC)
#define ELE_OTP_SEQ_SWITH_REQ (0xDD)

/* ELE failure indications */
#define ELE_ROM_PING_FAILURE_IND (0x0A)
#define ELE_FW_PING_FAILURE_IND (0x1A)
#define ELE_BAD_SIGNATURE_FAILURE_IND (0xF0)
#define ELE_BAD_HASH_FAILURE_IND (0xF1)
#define ELE_INVALID_LIFECYCLE_IND (0xF2)
#define ELE_PERMISSION_DENIED_FAILURE_IND (0xF3)
#define ELE_INVALID_MESSAGE_FAILURE_IND (0xF4)
#define ELE_BAD_VALUE_FAILURE_IND (0xF5)
#define ELE_BAD_FUSE_ID_FAILURE_IND (0xF6)
#define ELE_BAD_CONTAINER_FAILURE_IND (0xF7)
#define ELE_BAD_VERSION_FAILURE_IND (0xF8)
#define ELE_INVALID_KEY_FAILURE_IND (0xF9)
#define ELE_BAD_KEY_HASH_FAILURE_IND (0xFA)
#define ELE_NO_VALID_CONTAINER_FAILURE_IND (0xFB)
#define ELE_BAD_CERTIFICATE_FAILURE_IND (0xFC)
#define ELE_BAD_UID_FAILURE_IND (0xFD)
#define ELE_BAD_MONOTONIC_COUNTER_FAILURE_IND (0xFE)
#define ELE_MUST_SIGNED_FAILURE_IND (0xE0)
#define ELE_NO_AUTHENTICATION_FAILURE_IND (0xEE)
#define ELE_BAD_SRK_SET_FAILURE_IND (0xEF)
#define ELE_UNALIGNED_PAYLOAD_FAILURE_IND (0xA6)
#define ELE_WRONG_SIZE_FAILURE_IND (0xA7)
#define ELE_ENCRYPTION_FAILURE_IND (0xA8)
#define ELE_DECRYPTION_FAILURE_IND (0xA9)
#define ELE_OTP_PROGFAIL_FAILURE_IND (0xAA)
#define ELE_OTP_LOCKED_FAILURE_IND (0xAB)
#define ELE_OTP_INVALID_IDX_FAILURE_IND (0xAD)
#define ELE_TIME_OUT_FAILURE_IND (0xB0)
#define ELE_BAD_PAYLOAD_FAILURE_IND (0xB1)
#define ELE_WRONG_ADDRESS_FAILURE_IND (0xB4)
#define ELE_DMA_FAILURE_IND (0xB5)
#define ELE_DISABLED_FEATURE_FAILURE_IND (0xB6)
#define ELE_MUST_ATTEST_FAILURE_IND (0xB7)
#define ELE_RNG_NOT_STARTED_FAILURE_IND (0xB8)
#define ELE_CRC_ERROR_IND (0xB9)
#define ELE_AUTH_SKIPPED_OR_FAILED_FAILURE_IND (0xBB)
#define ELE_INCONSISTENT_PAR_FAILURE_IND (0xBC)
#define ELE_RNG_INST_FAILURE_FAILURE_IND (0xBD)
#define ELE_LOCKED_REG_FAILURE_IND (0xBE)
#define ELE_BAD_ID_FAILURE_IND (0xBF)
#define ELE_INVALID_OPERATION_FAILURE_IND (0xC0)
#define ELE_NON_SECURE_STATE_FAILURE_IND (0xC1)
#define ELE_MSG_TRUNCATED_IND (0xC2)
#define ELE_BAD_IMAGE_NUM_FAILURE_IND (0xC3)
#define ELE_BAD_IMAGE_ADDR_FAILURE_IND (0xC4)
#define ELE_BAD_IMAGE_PARAM_FAILURE_IND (0xC5)
#define ELE_BAD_IMAGE_TYPE_FAILURE_IND (0xC6)
#define ELE_CORRUPTED_SRK_FAILURE_IND (0xD0)
#define ELE_OUT_OF_MEMORY_IND (0xD1)
#define ELE_CSTM_FAILURE_IND (0xCF)
#define ELE_OLD_VERSION_FAILURE_IND (0xCE)
#define ELE_WRONG_BOOT_MODE_FAILURE_IND (0xCD)
#define ELE_APC_ALREADY_ENABLED_FAILURE_IND (0xCB)
#define ELE_RTC_ALREADY_ENABLED_FAILURE_IND (0xCC)
#define ELE_ABORT_IND (0xFF)

/* ELE IPC identifier */
#define ELE_IPC_MU_RTD (0x1)
#define ELE_IPC_MU_APD (0x2)

/* ELE Status*/
#define ELE_SUCCESS_IND (0xD6)
#define ELE_FAILURE_IND (0x29)

#define ELE_MAX_MSG          255U

struct ele_msg {
	u8 version;
	u8 size;
	u8 command;
	u8 tag;
	u32 data[(ELE_MAX_MSG - 1U)];
};

struct ele_get_info_data {
	u32 hdr;
	u32 soc;
	u32 lc;
	u32 uid[4];
	u32 sha256_rom_patch[8];
	u32 sha_fw[8];
	u32 oem_srkh[16];
	u32 state;
};

int ele_release_rdc(u8 core_id, u8 xrdc, u32 *response);
int ele_auth_oem_ctnr(ulong ctnr_addr, u32 *response);
int ele_release_container(u32 *response);
int ele_verify_image(u32 img_id, u32 *response);
int ele_forward_lifecycle(u16 life_cycle, u32 *response);
int ele_write_fuse(u16 fuse_id, u32 fuse_val, bool lock, u32 *response);
int ele_read_common_fuse(u16 fuse_id, u32 *fuse_words, u32 fuse_num, u32 *response);
int ele_release_caam(u32 core_did, u32 *response);
int ele_get_fw_version(u32 *fw_version, u32 *sha1, u32 *response);
int ele_get_events(u32 *events, u32 *events_cnt, u32 *response);
int ele_generate_dek_blob(u32 key_id, u32 src_paddr, u32 dst_paddr, u32 max_output_size);
int ele_dump_buffer(u32 *buffer, u32 buffer_length);
int ele_get_info(struct ele_get_info_data *info, u32 *response);
int ele_get_fw_status(u32 *status, u32 *response);
int ele_release_m33_trout(void);
int ele_write_secure_fuse(ulong signed_msg_blk, u32 *response);
int ele_return_lifecycle_update(ulong signed_msg_blk, u32 *response);
int ele_start_rng(void);
#endif
