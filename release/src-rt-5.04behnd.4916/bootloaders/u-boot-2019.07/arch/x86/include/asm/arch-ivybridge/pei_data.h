/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2011, Google Inc.
 */

#ifndef ASM_ARCH_PEI_DATA_H
#define ASM_ARCH_PEI_DATA_H

#include <linux/linkage.h>

struct pch_usb3_controller_settings {
	/* 0: Disable, 1: Enable, 2: Auto, 3: Smart Auto */
	uint16_t mode;
	/* 4 bit mask, 1: switchable, 0: not switchable */
	uint16_t hs_port_switch_mask;
	/* 0: No xHCI preOS driver, 1: xHCI preOS driver */
	uint16_t preboot_support;
	/* 0: Disable, 1: Enable */
	uint16_t xhci_streams;
};

typedef asmlinkage void (*tx_byte_func)(unsigned char byte);

#define PEI_VERSION 6

struct __packed pei_data {
	uint32_t pei_version;
	uint32_t mchbar;
	uint32_t dmibar;
	uint32_t epbar;
	uint32_t pciexbar;
	uint16_t smbusbar;
	uint32_t wdbbar;
	uint32_t wdbsize;
	uint32_t hpet_address;
	uint32_t rcba;
	uint32_t pmbase;
	uint32_t gpiobase;
	uint32_t thermalbase;
	uint32_t system_type; /* 0 Mobile, 1 Desktop/Server */
	uint32_t tseg_size;
	uint8_t spd_addresses[4];
	uint8_t ts_addresses[4];
	int boot_mode;
	int ec_present;
	int gbe_enable;
	/*
	 * 0 = leave channel enabled
	 * 1 = disable dimm 0 on channel
	 * 2 = disable dimm 1 on channel
	 * 3 = disable dimm 0+1 on channel
	 */
	int dimm_channel0_disabled;
	int dimm_channel1_disabled;
	/* Seed values saved in CMOS */
	uint32_t scrambler_seed;
	uint32_t scrambler_seed_s3;
	/* Data read from flash and passed into MRC */
	unsigned char *mrc_input;
	unsigned int mrc_input_len;
	/* Data from MRC that should be saved to flash */
	unsigned char *mrc_output;
	unsigned int mrc_output_len;
	/*
	 * Max frequency DDR3 could be ran at. Could be one of four values:
	 * 800, 1067, 1333, 1600
	 */
	uint32_t max_ddr3_freq;
	/*
	 * USB Port Configuration:
	 *  [0] = enable
	 *  [1] = overcurrent pin
	 *  [2] = length
	 *
	 * Ports 0-7 can be mapped to OC0-OC3
	 * Ports 8-13 can be mapped to OC4-OC7
	 *
	 * Port Length
	 *  MOBILE:
	 *   < 0x050 = Setting 1 (back panel, 1-5in, lowest tx amplitude)
	 *   < 0x140 = Setting 2 (back panel, 5-14in, highest tx amplitude)
	 *  DESKTOP:
	 *   < 0x080 = Setting 1 (front/back panel, <8in, lowest tx amplitude)
	 *   < 0x130 = Setting 2 (back panel, 8-13in, higher tx amplitude)
	 *   < 0x150 = Setting 3 (back panel, 13-15in, higest tx amplitude)
	 */
	uint16_t usb_port_config[16][3];
	/* See the usb3 struct above for details */
	struct pch_usb3_controller_settings usb3;
	/*
	 * SPD data array for onboard RAM. Specify address 0xf0,
	 * 0xf1, 0xf2, 0xf3 to index one of the 4 slots in
	 * spd_address for a given "DIMM".
	 */
	uint8_t spd_data[4][256];
	tx_byte_func tx_byte;
	int ddr3lv_support;
	/*
	 * pcie_init needs to be set to 1 to have the system agent initialise
	 * PCIe. Note: This should only be required if your system has Gen3
	 * devices and it will increase your boot time by at least 100ms.
	 */
	int pcie_init;
	/*
	 * N mode functionality. Leave this setting at 0.
	 * 0 Auto
	 * 1 1N
	 * 2 2N
	 */
	int nmode;
	/*
	 * DDR refresh rate config. JEDEC Standard No.21-C Annex K allows
	 * for DIMM SPD data to specify whether double-rate is required for
	 * extended operating temperature range.
	 * 0 Enable double rate based upon temperature thresholds
	 * 1 Normal rate
	 * 2 Always enable double rate
	 */
	int ddr_refresh_rate_config;
};

#endif
