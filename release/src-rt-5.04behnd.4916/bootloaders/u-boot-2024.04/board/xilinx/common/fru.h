/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2019 Xilinx, Inc.
 * Siva Durga Prasad Paladugu <siva.durga.prasad.paladugu@amd.com>>
 */

#ifndef __FRU_H
#define __FRU_H
#include <net.h>

struct fru_common_hdr {
	u8 version;
	u8 off_internal;
	u8 off_chassis;
	u8 off_board;
	u8 off_product;
	u8 off_multirec;
	u8 pad;
	u8 crc;
};

#define FRU_BOARD_MAX_LEN	32
#define FRU_MAX_NO_OF_MAC_ADDR	4

struct __packed fru_board_info_header {
	u8 ver;
	u8 len;
	u8 lang_code;
	u8 time[3];
};

struct __packed fru_board_info_member {
	u8 type_len;
	u8 *name;
};

struct fru_board_data {
	u8 ver;
	u8 len;
	u8 lang_code;
	u8 time[3];
	u8 manufacturer_type_len;
	u8 manufacturer_name[FRU_BOARD_MAX_LEN];
	u8 product_name_type_len;
	u8 product_name[FRU_BOARD_MAX_LEN];
	u8 serial_number_type_len;
	u8 serial_number[FRU_BOARD_MAX_LEN];
	u8 part_number_type_len;
	u8 part_number[FRU_BOARD_MAX_LEN];
	u8 file_id_type_len;
	u8 file_id[FRU_BOARD_MAX_LEN];
	/* Xilinx custom fields */
	u8 rev_type_len;
	u8 rev[FRU_BOARD_MAX_LEN];
	u8 pcie_type_len;
	u8 pcie[FRU_BOARD_MAX_LEN];
	u8 uuid_type_len;
	u8 uuid[FRU_BOARD_MAX_LEN];
};

struct fru_multirec_hdr {
	u8 rec_type;
	u8 type;
	u8 len;
	u8 csum;
	u8 hdr_csum;
};

struct fru_multirec_mac {
	u8 xlnx_iana_id[3];
	u8 ver;
	u8 macid[FRU_MAX_NO_OF_MAC_ADDR][ETH_ALEN];
};

struct fru_table {
	struct fru_common_hdr hdr;
	struct fru_board_data brd;
	struct fru_multirec_mac mac;
	bool captured;
};

#define FRU_TYPELEN_CODE_MASK	0xC0
#define FRU_TYPELEN_LEN_MASK	0x3F
#define FRU_COMMON_HDR_VER_MASK		0xF
#define FRU_COMMON_HDR_LEN_MULTIPLIER	8
#define FRU_LANG_CODE_ENGLISH		0
#define FRU_LANG_CODE_ENGLISH_1		25
#define FRU_TYPELEN_EOF			0xC1
#define FRU_MULTIREC_TYPE_OEM		0xD2
#define FRU_MULTIREC_MAC_OFFSET		4
#define FRU_LAST_REC			BIT(7)
#define FRU_DUT_MACID			0x31
#define FRU_SC_MACID			0x11

/* This should be minimum of fields */
#define FRU_BOARD_AREA_TOTAL_FIELDS	5
#define FRU_TYPELEN_TYPE_SHIFT		6
#define FRU_TYPELEN_TYPE_BINARY		0
#define FRU_TYPELEN_TYPE_ASCII8		3

int fru_display(int verbose);
int fru_capture(unsigned long addr);
int fru_generate(unsigned long addr, char *manufacturer, char *board_name,
		 char *serial_no, char *part_no, char *revision);
u8 fru_checksum(u8 *addr, u8 len);

extern struct fru_table fru_data;

#endif /* FRU_H */
