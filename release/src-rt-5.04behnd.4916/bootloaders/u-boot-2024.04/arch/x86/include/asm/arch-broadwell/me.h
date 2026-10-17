/* SPDX-License-Identifier: GPL-2.0 */
/*
 * From coreboot soc/intel/broadwell/include/soc/me.h
 *
 * Copyright (C) 2014 Google Inc.
 */

#ifndef _asm_arch_me_h
#define _asm_arch_me_h

#include <asm/me_common.h>

#define  ME_INIT_STATUS_SUCCESS_OTHER 3 /* SEE ME9 BWG */

#define ME_HSIO_MESSAGE		(7 << 28)
#define ME_HSIO_CMD_GETHSIOVER	1
#define ME_HSIO_CMD_CLOSE	0

/*
 * Apparently the GMES register is renamed to HFS2 (or HFSTS2 according
 * to ME9 BWG). Sadly the PCH EDS and the ME BWG do not match on nomenclature.
 */
#define PCI_ME_HFS2		0x48
/* Infrastructure Progress Values */
#define  ME_HFS2_PHASE_ROM		0
#define  ME_HFS2_PHASE_BUP		1
#define  ME_HFS2_PHASE_UKERNEL		2
#define  ME_HFS2_PHASE_POLICY		3
#define  ME_HFS2_PHASE_MODULE_LOAD	4
#define  ME_HFS2_PHASE_UNKNOWN		5
#define  ME_HFS2_PHASE_HOST_COMM	6
/* Current State - Based on Infra Progress values. */
/*       ROM State */
#define  ME_HFS2_STATE_ROM_BEGIN 0
#define  ME_HFS2_STATE_ROM_DISABLE 6
/*       BUP State */
#define  ME_HFS2_STATE_BUP_INIT 0
#define  ME_HFS2_STATE_BUP_DIS_HOST_WAKE 1
#define  ME_HFS2_STATE_BUP_FLOW_DET 4
#define  ME_HFS2_STATE_BUP_VSCC_ERR 8
#define  ME_HFS2_STATE_BUP_CHECK_STRAP 0xa
#define  ME_HFS2_STATE_BUP_PWR_OK_TIMEOUT 0xb
#define  ME_HFS2_STATE_BUP_MANUF_OVRD_STRAP 0xd
#define  ME_HFS2_STATE_BUP_M3 0x11
#define  ME_HFS2_STATE_BUP_M0 0x12
#define  ME_HFS2_STATE_BUP_FLOW_DET_ERR 0x13
#define  ME_HFS2_STATE_BUP_M3_CLK_ERR 0x15
#define  ME_HFS2_STATE_BUP_CPU_RESET_DID_TIMEOUT_MEM_MISSING 0x17
#define  ME_HFS2_STATE_BUP_M3_KERN_LOAD 0x18
#define  ME_HFS2_STATE_BUP_T32_MISSING 0x1c
#define  ME_HFS2_STATE_BUP_WAIT_DID 0x1f
#define  ME_HFS2_STATE_BUP_WAIT_DID_FAIL 0x20
#define  ME_HFS2_STATE_BUP_DID_NO_FAIL 0x21
#define  ME_HFS2_STATE_BUP_ENABLE_UMA 0x22
#define  ME_HFS2_STATE_BUP_ENABLE_UMA_ERR 0x23
#define  ME_HFS2_STATE_BUP_SEND_DID_ACK 0x24
#define  ME_HFS2_STATE_BUP_SEND_DID_ACK_ERR 0x25
#define  ME_HFS2_STATE_BUP_M0_CLK 0x26
#define  ME_HFS2_STATE_BUP_M0_CLK_ERR 0x27
#define  ME_HFS2_STATE_BUP_TEMP_DIS 0x28
#define  ME_HFS2_STATE_BUP_M0_KERN_LOAD 0x32
/*       Policy Module State */
#define  ME_HFS2_STATE_POLICY_ENTRY 0
#define  ME_HFS2_STATE_POLICY_RCVD_S3 3
#define  ME_HFS2_STATE_POLICY_RCVD_S4 4
#define  ME_HFS2_STATE_POLICY_RCVD_S5 5
#define  ME_HFS2_STATE_POLICY_RCVD_UPD 6
#define  ME_HFS2_STATE_POLICY_RCVD_PCR 7
#define  ME_HFS2_STATE_POLICY_RCVD_NPCR 8
#define  ME_HFS2_STATE_POLICY_RCVD_HOST_WAKE 9
#define  ME_HFS2_STATE_POLICY_RCVD_AC_DC 0xa
#define  ME_HFS2_STATE_POLICY_RCVD_DID 0xb
#define  ME_HFS2_STATE_POLICY_VSCC_NOT_FOUND 0xc
#define  ME_HFS2_STATE_POLICY_VSCC_INVALID 0xd
#define  ME_HFS2_STATE_POLICY_FPB_ERR 0xe
#define  ME_HFS2_STATE_POLICY_DESCRIPTOR_ERR 0xf
#define  ME_HFS2_STATE_POLICY_VSCC_NO_MATCH 0x10
/* Current PM Event Values */
#define  ME_HFS2_PMEVENT_CLEAN_MOFF_MX_WAKE 0
#define  ME_HFS2_PMEVENT_MOFF_MX_WAKE_ERROR 1
#define  ME_HFS2_PMEVENT_CLEAN_GLOBAL_RESET 2
#define  ME_HFS2_PMEVENT_CLEAN_GLOBAL_RESET_ERROR 3
#define  ME_HFS2_PMEVENT_CLEAN_ME_RESET 4
#define  ME_HFS2_PMEVENT_ME_RESET_EXCEPTION 5
#define  ME_HFS2_PMEVENT_PSEUDO_ME_RESET 6
#define  ME_HFS2_PMEVENT_S0MO_SXM3 7
#define  ME_HFS2_PMEVENT_SXM3_S0M0 8
#define  ME_HFS2_PMEVENT_NON_PWR_CYCLE_RESET 9
#define  ME_HFS2_PMEVENT_PWR_CYCLE_RESET_M3 0xa
#define  ME_HFS2_PMEVENT_PWR_CYCLE_RESET_MOFF 0xb
#define  ME_HFS2_PMEVENT_SXMX_SXMOFF 0xc

struct me_hfs2 {
	u32 bist_in_progress:1;
	u32 reserved1:2;
	u32 invoke_mebx:1;
	u32 cpu_replaced_sts:1;
	u32 mbp_rdy:1;
	u32 mfs_failure:1;
	u32 warm_reset_request:1;
	u32 cpu_replaced_valid:1;
	u32 reserved2:4;
	u32 mbp_cleared:1;
	u32 reserved3:2;
	u32 current_state:8;
	u32 current_pmevent:4;
	u32 progress_code:4;
} __packed;

#define PCI_ME_HFS5		0x68

#define PCI_ME_H_GS2		0x70
#define   PCI_ME_MBP_GIVE_UP	0x01

/* ICC Messages */
#define ICC_SET_CLOCK_ENABLES		0x3
#define ICC_API_VERSION_LYNXPOINT	0x00030000

struct icc_header {
	u32 api_version;
	u32 icc_command;
	u32 icc_status;
	u32 length;
	u32 reserved;
} __packed;

struct icc_clock_enables_msg {
	u32 clock_enables;
	u32 clock_mask;
	u32 no_response:1;
	u32 reserved:31;
} __packed;

/*
 * ME to BIOS Payload Datastructures and definitions. The ordering of the
 * structures follows the ordering in the ME9 BWG.
 */

#define MBP_APPID_KERNEL 1
#define MBP_APPID_INTEL_AT 3
#define MBP_APPID_HWA 4
#define MBP_APPID_ICC 5
#define MBP_APPID_NFC 6
/* Kernel items: */
#define MBP_KERNEL_FW_VER_ITEM 1
#define MBP_KERNEL_FW_CAP_ITEM 2
#define MBP_KERNEL_ROM_BIST_ITEM 3
#define MBP_KERNEL_PLAT_KEY_ITEM 4
#define MBP_KERNEL_FW_TYPE_ITEM 5
#define MBP_KERNEL_MFS_FAILURE_ITEM 6
#define MBP_KERNEL_PLAT_TIME_ITEM 7
/* Intel AT items: */
#define MBP_INTEL_AT_STATE_ITEM 1
/* ICC Items: */
#define MBP_ICC_PROFILE_ITEM 1
/* HWA Items: */
#define MBP_HWA_REQUEST_ITEM 1
/* NFC Items: */
#define MBP_NFC_SUPPORT_DATA_ITEM 1

#define MBP_MAKE_IDENT(appid, item) ((appid << 8) | item)
#define MBP_IDENT(appid, item) \
	MBP_MAKE_IDENT(MBP_APPID_##appid, MBP_##appid##_##item##_ITEM)

struct mbp_fw_version_name {
	u32	major_version:16;
	u32	minor_version:16;
	u32	hotfix_version:16;
	u32	build_version:16;
} __packed;

struct icc_address_mask {
	u16 icc_start_address;
	u16 mask;
} __packed;

struct mbp_icc_profile {
	u8	num_icc_profiles;
	u8	icc_profile_soft_strap;
	u8	icc_profile_index;
	u8	reserved;
	u32	icc_reg_bundles;
	struct icc_address_mask icc_address_mask[0];
} __packed;

struct me_bios_payload {
	struct mbp_fw_version_name	*fw_version_name;
	struct mbp_mefwcaps	*fw_capabilities;
	struct mbp_rom_bist_data *rom_bist_data;
	struct mbp_platform_key *platform_key;
	struct mbp_plat_type	*fw_plat_type;
	struct mbp_icc_profile	*icc_profile;
	struct mbp_at_state	*at_state;
	u32		*mfsintegrity;
	struct mbp_plat_time	*plat_time;
	struct mbp_nfc_data	*nfc_data;
};

#endif
