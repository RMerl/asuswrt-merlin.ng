/* SPDX-License-Identifier: GPL-2.0 */
/*
 * From Coreboot src/southbridge/intel/bd82x6x/me.h
 *
 * Coreboot copies lots of code around. Here we are trying to keep the common
 * code in a separate file to reduce code duplication and hopefully make it
 * easier to add new platform.
 *
 * Copyright (C) 2016 Google, Inc
 */

#ifndef __ASM_ME_COMMON_H
#define __ASM_ME_COMMON_H

#include <linux/compiler.h>
#include <linux/types.h>
#include <pci.h>

#define MCHBAR_PEI_VERSION	0x5034

#define ME_RETRY		100000	/* 1 second */
#define ME_DELAY		10	/* 10 us */

/*
 * Management Engine PCI registers
 */

#define PCI_CPU_MEBASE_L	0x70	/* Set by MRC */
#define PCI_CPU_MEBASE_H	0x74	/* Set by MRC */

#define PCI_ME_HFS		0x40
#define  ME_HFS_CWS_RESET	0
#define  ME_HFS_CWS_INIT	1
#define  ME_HFS_CWS_REC		2
#define  ME_HFS_CWS_NORMAL	5
#define  ME_HFS_CWS_WAIT	6
#define  ME_HFS_CWS_TRANS	7
#define  ME_HFS_CWS_INVALID	8
#define  ME_HFS_STATE_PREBOOT	0
#define  ME_HFS_STATE_M0_UMA	1
#define  ME_HFS_STATE_M3	4
#define  ME_HFS_STATE_M0	5
#define  ME_HFS_STATE_BRINGUP	6
#define  ME_HFS_STATE_ERROR	7
#define  ME_HFS_ERROR_NONE	0
#define  ME_HFS_ERROR_UNCAT	1
#define  ME_HFS_ERROR_IMAGE	3
#define  ME_HFS_ERROR_DEBUG	4
#define  ME_HFS_MODE_NORMAL	0
#define  ME_HFS_MODE_DEBUG	2
#define  ME_HFS_MODE_DIS	3
#define  ME_HFS_MODE_OVER_JMPR	4
#define  ME_HFS_MODE_OVER_MEI	5
#define  ME_HFS_BIOS_DRAM_ACK	1
#define  ME_HFS_ACK_NO_DID	0
#define  ME_HFS_ACK_RESET	1
#define  ME_HFS_ACK_PWR_CYCLE	2
#define  ME_HFS_ACK_S3		3
#define  ME_HFS_ACK_S4		4
#define  ME_HFS_ACK_S5		5
#define  ME_HFS_ACK_GBL_RESET	6
#define  ME_HFS_ACK_CONTINUE	7

struct me_hfs {
	u32 working_state:4;
	u32 mfg_mode:1;
	u32 fpt_bad:1;
	u32 operation_state:3;
	u32 fw_init_complete:1;
	u32 ft_bup_ld_flr:1;
	u32 update_in_progress:1;
	u32 error_code:4;
	u32 operation_mode:4;
	u32 reserved:4;
	u32 boot_options_present:1;
	u32 ack_data:3;
	u32 bios_msg_ack:4;
} __packed;

#define PCI_ME_UMA		0x44

struct me_uma {
	u32 size:6;
	u32 reserved_1:10;
	u32 valid:1;
	u32 reserved_0:14;
	u32 set_to_one:1;
} __packed;

#define PCI_ME_H_GS		0x4c
#define  ME_INIT_DONE		1
#define  ME_INIT_STATUS_SUCCESS	0
#define  ME_INIT_STATUS_NOMEM	1
#define  ME_INIT_STATUS_ERROR	2

struct me_did {
	u32 uma_base:16;
	u32 reserved:7;
	u32 rapid_start:1;	/* Broadwell only */
	u32 status:4;
	u32 init_done:4;
} __packed;

#define PCI_ME_GMES		0x48
#define  ME_GMES_PHASE_ROM	0
#define  ME_GMES_PHASE_BUP	1
#define  ME_GMES_PHASE_UKERNEL	2
#define  ME_GMES_PHASE_POLICY	3
#define  ME_GMES_PHASE_MODULE	4
#define  ME_GMES_PHASE_UNKNOWN	5
#define  ME_GMES_PHASE_HOST	6

struct me_gmes {
	u32 bist_in_prog:1;
	u32 icc_prog_sts:2;
	u32 invoke_mebx:1;
	u32 cpu_replaced_sts:1;
	u32 mbp_rdy:1;
	u32 mfs_failure:1;
	u32 warm_rst_req_for_df:1;
	u32 cpu_replaced_valid:1;
	u32 reserved_1:2;
	u32 fw_upd_ipu:1;
	u32 reserved_2:4;
	u32 current_state:8;
	u32 current_pmevent:4;
	u32 progress_code:4;
} __packed;

#define PCI_ME_HERES		0xbc
#define  PCI_ME_EXT_SHA1	0x00
#define  PCI_ME_EXT_SHA256	0x02
#define PCI_ME_HER(x)		(0xc0+(4*(x)))

struct me_heres {
	u32 extend_reg_algorithm:4;
	u32 reserved:26;
	u32 extend_feature_present:1;
	u32 extend_reg_valid:1;
} __packed;

/*
 * Management Engine MEI registers
 */

#define MEI_H_CB_WW		0x00
#define MEI_H_CSR		0x04
#define MEI_ME_CB_RW		0x08
#define MEI_ME_CSR_HA		0x0c

struct mei_csr {
	u32 interrupt_enable:1;
	u32 interrupt_status:1;
	u32 interrupt_generate:1;
	u32 ready:1;
	u32 reset:1;
	u32 reserved:3;
	u32 buffer_read_ptr:8;
	u32 buffer_write_ptr:8;
	u32 buffer_depth:8;
} __packed;

#define MEI_ADDRESS_CORE	0x01
#define MEI_ADDRESS_AMT		0x02
#define MEI_ADDRESS_RESERVED	0x03
#define MEI_ADDRESS_WDT		0x04
#define MEI_ADDRESS_MKHI	0x07
#define MEI_ADDRESS_ICC		0x08
#define MEI_ADDRESS_THERMAL	0x09

#define MEI_HOST_ADDRESS	0

struct mei_header {
	u32 client_address:8;
	u32 host_address:8;
	u32 length:9;
	u32 reserved:6;
	u32 is_complete:1;
} __packed;

#define MKHI_GROUP_ID_CBM	0x00
#define MKHI_GROUP_ID_FWCAPS	0x03
#define MKHI_GROUP_ID_MDES	0x08
#define MKHI_GROUP_ID_GEN	0xff

#define MKHI_GET_FW_VERSION	0x02
#define MKHI_END_OF_POST	0x0c
#define MKHI_FEATURE_OVERRIDE	0x14

/* Ivybridge only: */
#define MKHI_GLOBAL_RESET	0x0b
#define MKHI_FWCAPS_GET_RULE	0x02
#define MKHI_MDES_ENABLE	0x09

/* Broadwell only: */
#define MKHI_GLOBAL_RESET	0x0b
#define MKHI_FWCAPS_GET_RULE	0x02
#define MKHI_GROUP_ID_HMRFPO	0x05
#define MKHI_HMRFPO_LOCK	0x02
#define MKHI_HMRFPO_LOCK_NOACK	0x05
#define MKHI_MDES_ENABLE	0x09
#define MKHI_END_OF_POST_NOACK	0x1a

struct mkhi_header {
	u32 group_id:8;
	u32 command:7;
	u32 is_response:1;
	u32 reserved:8;
	u32 result:8;
} __packed;

struct me_fw_version {
	u16 code_minor;
	u16 code_major;
	u16 code_build_number;
	u16 code_hot_fix;
	u16 recovery_minor;
	u16 recovery_major;
	u16 recovery_build_number;
	u16 recovery_hot_fix;
} __packed;


#define HECI_EOP_STATUS_SUCCESS       0x0
#define HECI_EOP_PERFORM_GLOBAL_RESET 0x1

#define CBM_RR_GLOBAL_RESET	0x01

#define GLOBAL_RESET_BIOS_MRC	0x01
#define GLOBAL_RESET_BIOS_POST	0x02
#define GLOBAL_RESET_MEBX	0x03

struct me_global_reset {
	u8 request_origin;
	u8 reset_type;
} __packed;

enum me_bios_path {
	ME_NORMAL_BIOS_PATH,
	ME_S3WAKE_BIOS_PATH,
	ME_ERROR_BIOS_PATH,
	ME_RECOVERY_BIOS_PATH,
	ME_DISABLE_BIOS_PATH,
	ME_FIRMWARE_UPDATE_BIOS_PATH,
};

struct __packed mefwcaps_sku {
	u32 full_net:1;
	u32 std_net:1;
	u32 manageability:1;
	u32 small_business:1;
	u32 l3manageability:1;
	u32 intel_at:1;
	u32 intel_cls:1;
	u32 reserved:3;
	u32 intel_mpc:1;
	u32 icc_over_clocking:1;
	u32 pavp:1;
	u32 reserved_1:4;
	u32 ipv6:1;
	u32 kvm:1;
	u32 och:1;
	u32 vlan:1;
	u32 tls:1;
	u32 reserved_4:1;
	u32 wlan:1;
	u32 reserved_5:8;
};

struct __packed tdt_state_flag {
	u16 lock_state:1;
	u16 authenticate_module:1;
	u16 s3authentication:1;
	u16 flash_wear_out:1;
	u16 flash_variable_security:1;
	u16 wwan3gpresent:1;	/* ivybridge only */
	u16 wwan3goob:1;	/* ivybridge only */
	u16 reserved:9;
};

struct __packed tdt_state_info {
	u8 state;
	u8 last_theft_trigger;
	struct tdt_state_flag flags;
};

struct __packed mbp_rom_bist_data {
	u16 device_id;
	u16 fuse_test_flags;
	u32 umchid[4];
};

struct __packed mbp_platform_key {
	u32 key[8];
};

struct __packed mbp_header {
	u32 mbp_size:8;
	u32 num_entries:8;
	u32 rsvd:16;
};

struct __packed mbp_item_header {
	u32 app_id:8;
	u32 item_id:8;
	u32 length:8;
	u32 rsvd:8;
};

struct __packed me_fwcaps {
	u32 id;
	u8 length;
	struct mefwcaps_sku caps_sku;
	u8 reserved[3];
};

/**
 * intel_me_status() - Check Intel Management Engine status
 *
 * @me_dev:	Management engine PCI device
 */
void intel_me_status(struct udevice *me_dev);

/**
 * intel_early_me_init() - Early Intel Management Engine init
 *
 * @me_dev:	Management engine PCI device
 * @return 0 if OK, -ve on error
 */
int intel_early_me_init(struct udevice *me_dev);

/**
 * intel_early_me_uma_size() - Get UMA size from the Intel Management Engine
 *
 * @me_dev:	Management engine PCI device
 * @return UMA size if OK, -EINVAL on error
 */
int intel_early_me_uma_size(struct udevice *me_dev);

/**
 * intel_early_me_init_done() - Complete Intel Management Engine init
 *
 * @dev:	Northbridge device
 * @me_dev:	Management engine PCI device
 * @status:	Status result (ME_INIT_...)
 * @return 0 to continue to boot, -EINVAL on unknown result data, -ETIMEDOUT
 * if ME did not respond
 */
int intel_early_me_init_done(struct udevice *dev, struct udevice *me_dev,
			     uint status);

int intel_me_hsio_version(struct udevice *dev, uint16_t *version,
			  uint16_t *checksum);

static inline void pci_read_dword_ptr(struct udevice *me_dev, void *ptr,
				      int offset)
{
	u32 dword;

	dm_pci_read_config32(me_dev, offset, &dword);
	memcpy(ptr, &dword, sizeof(dword));
}

static inline void pci_write_dword_ptr(struct udevice *me_dev, void *ptr,
				       int offset)
{
	u32 dword = 0;

	memcpy(&dword, ptr, sizeof(dword));
	dm_pci_write_config32(me_dev, offset, dword);
}
#endif
