/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2009
 * Stefano Babic, DENX Software Engineering, sbabic@denx.de.
 */

#ifndef _IMXIMAGE_H_
#define _IMXIMAGE_H_

#define MAX_HW_CFG_SIZE_V2 220 /* Max number of registers imx can set for v2 */
#define MAX_PLUGIN_CODE_SIZE (64 * 1024)
#define MAX_HW_CFG_SIZE_V1 60  /* Max number of registers imx can set for v1 */
#define APP_CODE_BARKER	0xB1
#define DCD_BARKER	0xB17219E9

/* Specify the offset of the IVT in the IMX header as expected by BootROM */
#define BOOTROM_IVT_HDR_OFFSET	0xC00

/*
 * NOTE: This file must be kept in sync with arch/arm/include/asm/\
 *       mach-imx/imximage.cfg because tools/imximage.c can not
 *       cross-include headers from arch/arm/ and vice-versa.
 */
#define CMD_DATA_STR	"DATA"

/* Initial Vector Table Offset */
#define FLASH_OFFSET_UNDEFINED	0xFFFFFFFF
#define FLASH_OFFSET_STANDARD	0x400
#define FLASH_OFFSET_NAND	FLASH_OFFSET_STANDARD
#define FLASH_OFFSET_SD		FLASH_OFFSET_STANDARD
#define FLASH_OFFSET_SPI	FLASH_OFFSET_STANDARD
#define FLASH_OFFSET_ONENAND	0x100
#define FLASH_OFFSET_NOR	0x1000
#define FLASH_OFFSET_SATA	FLASH_OFFSET_STANDARD
#define FLASH_OFFSET_QSPI	0x1000
#define FLASH_OFFSET_FLEXSPI	0x1000

/* Initial Load Region Size */
#define FLASH_LOADSIZE_UNDEFINED	0xFFFFFFFF
#define FLASH_LOADSIZE_STANDARD		0x1000
#define FLASH_LOADSIZE_NAND		FLASH_LOADSIZE_STANDARD
#define FLASH_LOADSIZE_SD		FLASH_LOADSIZE_STANDARD
#define FLASH_LOADSIZE_SPI		FLASH_LOADSIZE_STANDARD
#define FLASH_LOADSIZE_ONENAND		0x400
#define FLASH_LOADSIZE_NOR		0x0 /* entire image */
#define FLASH_LOADSIZE_SATA		FLASH_LOADSIZE_STANDARD
#define FLASH_LOADSIZE_QSPI		0x0 /* entire image */

/* Command tags and parameters */
#define IVT_HEADER_TAG			0xD1
#define IVT_VERSION			0x40
#define IVT_VERSION_V3			0x41
#define DCD_HEADER_TAG			0xD2
#define DCD_VERSION			0x40
#define DCD_WRITE_DATA_COMMAND_TAG	0xCC
#define DCD_WRITE_DATA_PARAM		0x4
#define DCD_WRITE_CLR_BIT_PARAM		0xC
#define DCD_WRITE_SET_BIT_PARAM		0x1C
#define DCD_CHECK_DATA_COMMAND_TAG	0xCF
#define DCD_CHECK_BITS_SET_PARAM	0x14
#define DCD_CHECK_BITS_CLR_PARAM	0x04

#ifndef __ASSEMBLY__
enum imximage_cmd {
	CMD_INVALID,
	CMD_IMAGE_VERSION,
	CMD_BOOT_FROM,
	CMD_BOOT_OFFSET,
	CMD_WRITE_DATA,
	CMD_WRITE_CLR_BIT,
	CMD_WRITE_SET_BIT,
	CMD_CHECK_BITS_SET,
	CMD_CHECK_BITS_CLR,
	CMD_CSF,
	CMD_PLUGIN,
	/* Follwoing on i.MX8MQ/MM */
	CMD_FIT,
	CMD_SIGNED_HDMI,
	CMD_LOADER,
	CMD_SECOND_LOADER,
	CMD_DDR_FW,
};

enum imximage_fld_types {
	CFG_INVALID = -1,
	CFG_COMMAND,
	CFG_REG_SIZE,
	CFG_REG_ADDRESS,
	CFG_REG_VALUE
};

enum imximage_version {
	IMXIMAGE_VER_INVALID = -1,
	IMXIMAGE_V1 = 1,
	IMXIMAGE_V2,
	IMXIMAGE_V3
};

typedef struct {
	uint32_t type; /* Type of pointer (byte, halfword, word, wait/read) */
	uint32_t addr; /* Address to write to */
	uint32_t value; /* Data to write */
} dcd_type_addr_data_t;

typedef struct {
	uint32_t barker; /* Barker for sanity check */
	uint32_t length; /* Device configuration length (without preamble) */
} dcd_preamble_t;

typedef struct {
	dcd_preamble_t preamble;
	dcd_type_addr_data_t addr_data[MAX_HW_CFG_SIZE_V1];
} dcd_v1_t;

typedef struct {
	uint32_t app_code_jump_vector;
	uint32_t app_code_barker;
	uint32_t app_code_csf;
	uint32_t dcd_ptr_ptr;
	uint32_t super_root_key;
	uint32_t dcd_ptr;
	uint32_t app_dest_ptr;
} flash_header_v1_t;

typedef struct {
	uint32_t length; 	/* Length of data to be read from flash */
} flash_cfg_parms_t;

typedef struct {
	flash_header_v1_t fhdr;
	dcd_v1_t dcd_table;
	flash_cfg_parms_t ext_header;
} imx_header_v1_t;

typedef struct {
	uint32_t addr;
	uint32_t value;
} dcd_addr_data_t;

typedef struct {
	uint8_t tag;
	uint16_t length;
	uint8_t version;
} __attribute__((packed)) ivt_header_t;

typedef struct {
	uint8_t tag;
	uint16_t length;
	uint8_t param;
} __attribute__((packed)) write_dcd_command_t;

struct dcd_v2_cmd {
	write_dcd_command_t write_dcd_command;
	dcd_addr_data_t addr_data[MAX_HW_CFG_SIZE_V2];
};

typedef struct {
	ivt_header_t header;
	struct dcd_v2_cmd dcd_cmd;
	uint32_t padding[1]; /* end up on an 8-byte boundary */
} dcd_v2_t;

typedef struct {
	uint32_t start;
	uint32_t size;
	uint32_t plugin;
} boot_data_t;

typedef struct {
	ivt_header_t header;
	uint32_t entry;
	uint32_t reserved1;
	uint32_t dcd_ptr;
	uint32_t boot_data_ptr;
	uint32_t self;
	uint32_t csf;
	uint32_t reserved2;
} flash_header_v2_t;

typedef struct {
	flash_header_v2_t fhdr;
	boot_data_t boot_data;
	union {
		dcd_v2_t dcd_table;
		char plugin_code[MAX_PLUGIN_CODE_SIZE];
	} data;
} imx_header_v2_t;

typedef struct {
	flash_header_v2_t fhdr;
	boot_data_t boot_data;
	uint32_t padding[5];
} imx_header_v3_t;

/* The header must be aligned to 4k on MX53 for NAND boot */
struct imx_header {
	union {
		imx_header_v1_t hdr_v1;
		imx_header_v2_t hdr_v2;
	} header;
};

typedef void (*set_dcd_val_t)(struct imx_header *imxhdr,
					char *name, int lineno,
					int fld, uint32_t value,
					uint32_t off);

typedef void (*set_dcd_param_t)(struct imx_header *imxhdr, uint32_t dcd_len,
					int32_t cmd);

typedef void (*set_dcd_rst_t)(struct imx_header *imxhdr,
					uint32_t dcd_len,
					char *name, int lineno);

typedef void (*set_imx_hdr_t)(struct imx_header *imxhdr, uint32_t dcd_len,
		uint32_t entry_point, uint32_t flash_offset);

#endif /* __ASSEMBLY__ */
#endif /* _IMXIMAGE_H_ */
