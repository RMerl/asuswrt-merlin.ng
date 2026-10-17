// SPDX-License-Identifier: GPL-2.0
/*
 * (C) Copyright 2019 - 2022, Xilinx, Inc.
 * (C) Copyright 2022 - 2023, Advanced Micro Devices, Inc.
 */

#include <common.h>
#include <cpu_func.h>
#include <env.h>
#include <fdtdec.h>
#include <log.h>
#include <malloc.h>
#include <net.h>
#include <asm/io.h>
#include <asm/arch/hardware.h>

#include "fru.h"

struct fru_table fru_data __section(".data");

static u16 fru_cal_area_len(u8 len)
{
	return len * FRU_COMMON_HDR_LEN_MULTIPLIER;
}

static u8 fru_version(u8 ver)
{
	return ver & FRU_COMMON_HDR_VER_MASK;
}

static int fru_check_language(u8 code)
{
	if (code != FRU_LANG_CODE_ENGLISH && code != FRU_LANG_CODE_ENGLISH_1) {
		printf("FRU_ERROR: Only English Language is supported\n");
		return -EINVAL;
	}

	return 0;
}

u8 fru_checksum(u8 *addr, u8 len)
{
	u8 checksum = 0;
	u8 cnt = len;

	while (len--) {
		if (*addr == 0)
			cnt--;

		checksum += *addr;
		addr++;
	}

	/* If all data bytes are 0's return error */
	if (!cnt)
		return EINVAL;

	return checksum;
}

static int fru_check_type_len(u8 type_len, u8 language, u8 *type)
{
	int len;

	*type = (type_len & FRU_TYPELEN_CODE_MASK) >> FRU_TYPELEN_TYPE_SHIFT;

	len = type_len & FRU_TYPELEN_LEN_MASK;

	return len;
}

/* Return len */
static u8 fru_gen_type_len(u8 *addr, char *name)
{
	int len = strlen(name);
	struct fru_board_info_member *member;

	member = (struct fru_board_info_member *)addr;
	member->type_len = FRU_TYPELEN_TYPE_ASCII8 << FRU_TYPELEN_TYPE_SHIFT;
	member->type_len |= len;

	debug("%lx/%lx: Add %s to 0x%lx (len 0x%x)\n", (ulong)addr,
	      (ulong)&member->type_len,  name, (ulong)&member->name, len);
	memcpy(&member->name, name, len);

	/* Add +1 for type_len parameter */
	return 1 + len;
}

int fru_generate(unsigned long addr, char *manufacturer, char *board_name,
		 char *serial_no, char *part_no, char *revision)
{
	struct fru_common_hdr *header = (struct fru_common_hdr *)addr;
	struct fru_board_info_header *board_info;
	u8 *member;
	u8 len, pad, modulo;

	header->version = 1; /* Only version 1.0 is supported now */
	header->off_internal = 0; /* not present */
	header->off_chassis = 0; /* not present */
	header->off_board = (sizeof(*header)) / 8; /* Starting offset 8 */
	header->off_product = 0; /* not present */
	header->off_multirec = 0; /* not present */
	header->pad = 0;
	/*
	 * This unsigned byte can be used to calculate a zero checksum
	 * for the data area following the header. I.e. the modulo 256 sum of
	 * the record data bytes plus the checksum byte equals zero.
	 */
	header->crc = 0; /* Clear before calculation */
	header->crc = 0 - fru_checksum((u8 *)header, sizeof(*header));

	/* board info is just right after header */
	board_info = (void *)((u8 *)header + sizeof(*header));

	debug("header %lx, board_info %lx\n", (ulong)header, (ulong)board_info);

	board_info->ver = 1; /* 1.0 spec */
	board_info->lang_code = 0; /* English */
	board_info->time[0] = 0; /* unspecified */
	board_info->time[1] = 0; /* unspecified */
	board_info->time[2] = 0; /* unspecified */

	/* Member fields are just after board_info header */
	member = (u8 *)board_info + sizeof(*board_info);

	len = fru_gen_type_len(member, manufacturer); /* Board Manufacturer */
	member += len;
	len = fru_gen_type_len(member, board_name); /* Board Product name */
	member += len;
	len = fru_gen_type_len(member, serial_no); /* Board Serial number */
	member += len;
	len = fru_gen_type_len(member, part_no); /* Board part number */
	member += len;
	len = fru_gen_type_len(member, "U-Boot generator"); /* File ID */
	member += len;
	len = fru_gen_type_len(member, revision); /* Revision */
	member += len;

	*member++ = 0xc1; /* Indication of no more fields */

	len = member - (u8 *)board_info; /* Find current length */
	len += 1; /* Add checksum there too for calculation */

	modulo = len % 8;

	if (modulo) {
		/* Do not fill last item which is checksum */
		for (pad = 0; pad < 8 - modulo; pad++)
			*member++ = 0;

		/* Increase structure size */
		len += 8 - modulo;
	}

	board_info->len = len / 8; /* Size in multiples of 8 bytes */

	*member = 0; /* Clear before calculation */
	*member = 0 - fru_checksum((u8 *)board_info, len);

	debug("checksum %x(addr %x)\n", *member, len);

	env_set_hex("fru_addr", addr);
	env_set_hex("filesize", (unsigned long)member - addr + 1);

	return 0;
}

static int fru_parse_board(unsigned long addr)
{
	u8 i, type;
	int len;
	u8 *data, *term, *limit, *next_addr, *eof;

	memcpy(&fru_data.brd.ver, (void *)addr, 6);

	/*
	 * eof marks the last data byte (without checksum). That's why checksum
	 * is address length - 1 and last data byte is length - 2.
	 */
	eof = (u8 *)(fru_data.brd.len * 8 + addr - 2);

	addr += 6;
	data = (u8 *)&fru_data.brd.manufacturer_type_len;

	/* Record max structure limit not to write data over allocated space */
	limit = (u8 *)&fru_data.brd + sizeof(struct fru_board_data);

	for (i = 0; ; i++, data += FRU_BOARD_MAX_LEN) {
		len = fru_check_type_len(*(u8 *)addr, fru_data.brd.lang_code,
					 &type);
		next_addr = (u8 *)addr + 1;

		if ((u8 *)addr >= eof) {
			debug("Reach EOF record: addr %lx, eof %lx\n", addr,
			      (unsigned long)eof);
			break;
		}

		/*
		 * Stop capture if the type is ASCII and valid field length
		 * is 1 (0xc1) and next FRU data is less than 0x20 (space " ")
		 * or it is 0x7f (delete 'DEL').
		 */
		if (type == FRU_TYPELEN_TYPE_ASCII8 && len == 1	&&
		    (*next_addr < 0x20 || *next_addr == 0x7F))
			break;

		/* Stop when amount of chars is more then fields to record */
		if (data + len > limit)
			break;
		/* This record type/len field */
		*data++ = *(u8 *)addr;

		/* Add offset to match data */
		addr += 1;

		/* If len is 0 it means empty field that's why skip writing */
		if (!len)
			continue;

		/* Record data field */
		memcpy(data, (u8 *)addr, len);
		term = data + (u8)len;
		*term = 0;
		addr += len;
	}

	if (i < FRU_BOARD_AREA_TOTAL_FIELDS) {
		printf("Board area require minimum %d fields\n",
		       FRU_BOARD_AREA_TOTAL_FIELDS);
		return -EINVAL;
	}

	return 0;
}

static int fru_parse_multirec(unsigned long addr)
{
	struct fru_multirec_hdr mrc;
	u8 checksum = 0;
	u8 hdr_len = sizeof(struct fru_multirec_hdr);
	int mac_len = 0;

	debug("%s: multirec addr %lx\n", __func__, addr);

	do {
		memcpy(&mrc.rec_type, (void *)addr, hdr_len);

		checksum = fru_checksum((u8 *)addr, hdr_len);
		if (checksum) {
			debug("%s header CRC error\n", __func__);
			return -EINVAL;
		}

		if (mrc.rec_type == FRU_MULTIREC_TYPE_OEM) {
			struct fru_multirec_mac *mac = (void *)addr + hdr_len;
			u32 type = FRU_DUT_MACID;

			if (CONFIG_IS_ENABLED(FRU_SC))
				type = FRU_SC_MACID;

			if (mac->ver == type) {
				mac_len = mrc.len - FRU_MULTIREC_MAC_OFFSET;
				memcpy(&fru_data.mac.macid, mac->macid, mac_len);
			}
		}
		addr += mrc.len + hdr_len;
	} while (!(mrc.type & FRU_LAST_REC));

	return 0;
}

int fru_capture(unsigned long addr)
{
	struct fru_common_hdr *hdr;
	u8 checksum = 0;
	unsigned long multirec_addr = addr;

	checksum = fru_checksum((u8 *)addr, sizeof(struct fru_common_hdr));
	if (checksum) {
		printf("%s Common header CRC error\n", __func__);
		return -EINVAL;
	}

	hdr = (struct fru_common_hdr *)addr;
	memset((void *)&fru_data, 0, sizeof(fru_data));
	memcpy((void *)&fru_data, (void *)hdr,
	       sizeof(struct fru_common_hdr));

	fru_data.captured = true;

	if (hdr->off_board) {
		addr += fru_cal_area_len(hdr->off_board);
		fru_parse_board(addr);
	}

	env_set_hex("fru_addr", addr);

	if (hdr->off_multirec) {
		multirec_addr += fru_cal_area_len(hdr->off_multirec);
		fru_parse_multirec(multirec_addr);
	}

	return 0;
}

static int fru_display_board(struct fru_board_data *brd, int verbose)
{
	u32 time = 0;
	u8 type;
	int len;
	u8 *data;
	static const char * const typecode[] = {
		"Binary/Unspecified",
		"BCD plus",
		"6-bit ASCII",
		"8-bit ASCII",
		"2-byte UNICODE"
	};
	static const char * const boardinfo[] = {
		"Manufacturer Name",
		"Product Name",
		"Serial No",
		"Part Number",
		"File ID",
		/* Xilinx spec */
		"Revision Number",
	};

	if (verbose) {
		printf("*****BOARD INFO*****\n");
		printf("Version:%d\n", fru_version(brd->ver));
		printf("Board Area Length:%d\n", fru_cal_area_len(brd->len));
	}

	if (fru_check_language(brd->lang_code))
		return -EINVAL;

	time = brd->time[2] << 16 | brd->time[1] << 8 |
	       brd->time[0];

	if (verbose)
		printf("Time in Minutes from 0:00hrs 1/1/96: %d\n", time);

	data = (u8 *)&brd->manufacturer_type_len;

	for (u8 i = 0; i < (sizeof(boardinfo) / sizeof(*boardinfo)); i++) {
		len = fru_check_type_len(*data++, brd->lang_code,
					 &type);

		/* Empty record has no len/type filled */
		if (!len) {
			debug("%s not found\n", boardinfo[i]);
			continue;
		}

		if (type <= FRU_TYPELEN_TYPE_ASCII8 &&
		    (brd->lang_code == FRU_LANG_CODE_ENGLISH ||
		     brd->lang_code == FRU_LANG_CODE_ENGLISH_1))
			debug("Type code: %s\n", typecode[type]);
		else
			debug("Type code: %s\n", typecode[type + 1]);

		switch (type) {
		case FRU_TYPELEN_TYPE_BINARY:
			debug("Length: %d\n", len);
			printf(" %s: 0x%x\n", boardinfo[i], *data);
			break;
		case FRU_TYPELEN_TYPE_ASCII8:
			debug("Length: %d\n", len);
			printf(" %s: %s\n", boardinfo[i], data);
			break;
		default:
			debug("Unsupported type %x\n", type);
		}

		data += FRU_BOARD_MAX_LEN;
	}

	return 0;
}

static void fru_display_common_hdr(struct fru_common_hdr *hdr, int verbose)
{
	if (!verbose)
		return;

	printf("*****COMMON HEADER*****\n");
	printf("Version:%d\n", fru_version(hdr->version));
	if (hdr->off_internal)
		printf("Internal Use Area Offset:%d\n",
		       fru_cal_area_len(hdr->off_internal));
	else
		printf("*** No Internal Area ***\n");

	if (hdr->off_chassis)
		printf("Chassis Info Area Offset:%d\n",
		       fru_cal_area_len(hdr->off_chassis));
	else
		printf("*** No Chassis Info Area ***\n");

	if (hdr->off_board)
		printf("Board Area Offset:%d\n",
		       fru_cal_area_len(hdr->off_board));
	else
		printf("*** No Board Area ***\n");

	if (hdr->off_product)
		printf("Product Info Area Offset:%d\n",
		       fru_cal_area_len(hdr->off_product));
	else
		printf("*** No Product Info Area ***\n");

	if (hdr->off_multirec)
		printf("MultiRecord Area Offset:%d\n",
		       fru_cal_area_len(hdr->off_multirec));
	else
		printf("*** No MultiRecord Area ***\n");
}

int fru_display(int verbose)
{
	if (!fru_data.captured) {
		printf("FRU data not available please run fru parse\n");
		return -EINVAL;
	}

	fru_display_common_hdr(&fru_data.hdr, verbose);

	return fru_display_board(&fru_data.brd, verbose);
}
