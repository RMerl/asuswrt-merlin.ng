/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2018 MediaTek Inc.
 */

#ifndef __PRELOADER_H_
#define __PRELOADER_H_

enum forbidden_mode {
	F_FACTORY_MODE = 0x0001
};

union lk_hdr {
	struct {
		u32 magic;
		u32 size;
		char name[32];
		u32 loadaddr;
	};

	u8 data[512];
};

struct sec_limit {
	unsigned int magic_num;
	enum forbidden_mode forbid_mode;
};

enum bootmode {
	NORMAL_BOOT = 0,
	META_BOOT = 1,
	RECOVERY_BOOT = 2,
	SW_REBOOT = 3,
	FACTORY_BOOT = 4,
	ADVMETA_BOOT = 5,
	ATE_FACTORY_BOOT = 6,
	ALARM_BOOT = 7,

	KERNEL_POWER_OFF_CHARGING_BOOT = 8,
	LOW_POWER_OFF_CHARGING_BOOT = 9,

	FAST_BOOT = 99,
	DOWNLOAD_BOOT = 100,
	UNKNOWN_BOOT
};

enum boot_reason {
	BR_POWER_KEY = 0,
	BR_USB,
	BR_RTC,
	BR_WDT,
	BR_WDT_BY_PASS_PWK,
	BR_TOOL_BY_PASS_PWK,
	BR_2SEC_REBOOT,
	BR_UNKNOWN
};

enum meta_com_type {
	META_UNKNOWN_COM = 0,
	META_UART_COM,
	META_USB_COM
};

struct da_info_t {
	u32 addr;
	u32 arg1;
	u32 arg2;
	u32 len;
	u32 sig_len;
};

struct boot_argument {
	u32 magic;
	enum bootmode boot_mode;
	u32 e_flag;
	u32 log_port;
	u32 log_baudrate;
	u8 log_enable;
	u8 part_num;
	u8 reserved[2];
	u32 dram_rank_num;
	u32 dram_rank_size[4];
	u32 boot_reason;
	enum meta_com_type meta_com_type;
	u32 meta_com_id;
	u32 boot_time;
	struct da_info_t da_info;
	struct sec_limit sec_limit;
	union lk_hdr *part_info;
	u8 md_type[4];
	u32 ddr_reserve_enable;
	u32 ddr_reserve_success;
	u32 chip_ver;
	char pl_version[8];
};

#define BOOT_ARGUMENT_MAGIC	0x504c504c

#endif /* __PRELOADER_H_ */
