/* SPDX-License-Identifier: GPL-2.0+ */
#ifndef TEN64_CNTRL_H
#define TEN64_CNTRL_H

/**
 * struct t64uc_board_info - Board Information Structure
 * @mac: Base MAC address
 * @cpuId: Microcontroller unique serial number
 * @fwversion_major: Microcontroller version number (Major)
 * @fwversion_minor: Microcontroller version number (Minor)
 * @fwversion_patch: Microcontroller version number (Patch)
 */
struct t64uc_board_info {
	u8 mac[6];
	u32 cpuId[4];
	u8 fwversion_major;
	u8 fwversion_minor;
	u8 fwversion_patch;
} __packed;

enum {
	TEN64_CNTRL_GET_BOARD_INFO,
	TEN64_CNTRL_10G_OFF,
	TEN64_CNTRL_10G_ON,
	TEN64_CNTRL_SET_NEXT_BOOTSRC
};

#endif
