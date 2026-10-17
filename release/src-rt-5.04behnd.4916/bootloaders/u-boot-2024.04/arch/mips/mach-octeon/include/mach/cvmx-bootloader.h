/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2020 Marvell International Ltd.
 */

/*
 * Bootloader definitions that are shared with other programs
 */

#ifndef __CVMX_BOOTLOADER__
#define __CVMX_BOOTLOADER__

/*
 * The bootloader_header_t structure defines the header that is present
 * at the start of binary u-boot images.  This header is used to locate
 * the bootloader image in NAND, and also to allow verification of images
 * for normal NOR booting. This structure is placed at the beginning of a
 * bootloader binary image, and remains in the executable code.
 */
#define BOOTLOADER_HEADER_MAGIC		0x424f4f54	/* "BOOT" in ASCII */

#define BOOTLOADER_HEADER_COMMENT_LEN	64
#define BOOTLOADER_HEADER_VERSION_LEN	64
/* limited by the space to the next exception handler */
#define BOOTLOADER_HEADER_MAX_SIZE	0x200

#define BOOTLOADER_HEADER_CURRENT_MAJOR_REV 1
#define BOOTLOADER_HEADER_CURRENT_MINOR_REV 2
/*
 * Revision history
 * 1.1  Initial released revision. (SDK 1.9)
 * 1.2  TLB based relocatable image (SDK 2.0)
 */

#ifndef __ASSEMBLY__
struct bootloader_header {
	uint32_t jump_instr;	/*
				 * Jump to executable code following the
				 * header.  This allows this header to be
				 * (and remain) part of the executable image)
				 */
	uint32_t nop_instr;	/* Must be 0x0 */
	uint32_t magic;		/* Magic number to identify header */
	uint32_t hcrc;		/* CRC of all of header excluding this field */

	uint16_t hlen;		/* Length of header in bytes */
	uint16_t maj_rev;	/* Major revision */
	uint16_t min_rev;	/* Minor revision */
	uint16_t board_type;	/* Board type that the image is for */

	uint32_t dlen;		/* Length of data (following header) in bytes */
	uint32_t dcrc;		/* CRC of data */
	uint64_t address;	/* Mips virtual address */
	uint32_t flags;
	uint16_t image_type;	/* Defined in bootloader_image_t enum */
	uint16_t resv0;		/* pad */

	uint32_t reserved1;
	uint32_t reserved2;
	uint32_t reserved3;
	uint32_t reserved4;

	/* Optional, for descriptive purposes */
	char comment_string[BOOTLOADER_HEADER_COMMENT_LEN];
	/* Optional, for descriptive purposes */
	char version_string[BOOTLOADER_HEADER_VERSION_LEN];
} __packed;

/* Defines for flag field */
#define BL_HEADER_FLAG_FAILSAFE		1

enum bootloader_image {
	BL_HEADER_IMAGE_UNKNOWN = 0x0,
	BL_HEADER_IMAGE_STAGE2,		/* Binary bootloader stage2 image */
	BL_HEADER_IMAGE_STAGE3,		/* Binary bootloader stage3 image */
	BL_HEADER_IMAGE_NOR,		/* Binary bootloader for NOR boot */
	BL_HEADER_IMAGE_PCIBOOT,	/* Binary bootloader for PCI boot */
	BL_HEADER_IMAGE_UBOOT_ENV,	/* Environment for u-boot */
	/* Bootloader before U-Boot (stage 1/1.5) */
	BL_HEADER_IMAGE_PRE_UBOOT,
	BL_HEADER_IMAGE_STAGE1,		/* NOR stage 1 bootloader */
	BL_HEADER_IMAGE_MAX,
	/* Range for customer private use.  Will not be used by Cavium Inc. */
	BL_HEADER_IMAGE_CUST_RESERVED_MIN = 0x1000,
	BL_HEADER_IMAGE_CUST_RESERVED_MAX = 0x1fff
};

#endif /* __ASSEMBLY__ */

/*
 * Maximum address searched for NAND boot images and environments.
 * This is used by stage1 and stage2.
 */
#define MAX_NAND_SEARCH_ADDR	0x800000

/* Maximum address to look for start of normal bootloader */
#define MAX_NOR_SEARCH_ADDR	0x400000

/*
 * Defines for RAM based environment set by the host or the previous
 * bootloader in a chain boot configuration.
 */

#define U_BOOT_RAM_ENV_ADDR	0x1000
#define U_BOOT_RAM_ENV_SIZE	0x1000
#define U_BOOT_RAM_ENV_CRC_SIZE	0x4
#define U_BOOT_RAM_ENV_ADDR_2	(U_BOOT_RAM_ENV_ADDR + U_BOOT_RAM_ENV_SIZE)
/* Address of environment in L2 cache if booted from cache */
#define U_BOOT_CACHE_ENV_ADDR	0x000ff000
/* Size of environment in L2 cache */
#define U_BOOT_CACHE_ENV_SIZE	0x1000

/* Board numbers and names */

/* Type defines for board and chip types */
enum cvmx_board_types_enum {
	CVMX_BOARD_TYPE_NULL = 0,
	CVMX_BOARD_TYPE_SIM = 1,
	/* Special 'generic' board type, supports many boards */
	CVMX_BOARD_TYPE_GENERIC = 28,
	CVMX_BOARD_TYPE_EBB7304 = 76,
	CVMX_BOARD_TYPE_MAX,
	/* NOTE:  256-257 are being used by a customer. */

	/*
	 * The range from CVMX_BOARD_TYPE_MAX to
	 * CVMX_BOARD_TYPE_CUST_DEFINED_MIN is reserved
	 * for future SDK use.
	 */

	/*
	 * Set aside a range for customer boards. These numbers are managed
	 * by Cavium.
	 */
	CVMX_BOARD_TYPE_CUST_DEFINED_MIN = 10000,
	CVMX_BOARD_TYPE_CUST_DEFINED_MAX = 20000,

	/*
	 * Set aside a range for customer private use.  The SDK won't
	 * use any numbers in this range.
	 */
	CVMX_BOARD_TYPE_CUST_PRIVATE_MIN = 20001,
	CVMX_BOARD_TYPE_CUST_PRIVATE_MAX = 30000,
};

/* Functions to return string based on type */
/* Skip CVMX_BOARD_TYPE_ */
#define ENUM_BRD_TYPE_CASE(x)	case x: return(#x + 16)

static inline const char
*cvmx_board_type_to_string(enum cvmx_board_types_enum type)
{
	switch (type) {
		ENUM_BRD_TYPE_CASE(CVMX_BOARD_TYPE_NULL);
		ENUM_BRD_TYPE_CASE(CVMX_BOARD_TYPE_SIM);
		ENUM_BRD_TYPE_CASE(CVMX_BOARD_TYPE_GENERIC);
		ENUM_BRD_TYPE_CASE(CVMX_BOARD_TYPE_EBB7304);
		ENUM_BRD_TYPE_CASE(CVMX_BOARD_TYPE_MAX);

		/* Customer boards listed here */
		ENUM_BRD_TYPE_CASE(CVMX_BOARD_TYPE_CUST_DEFINED_MIN);
		ENUM_BRD_TYPE_CASE(CVMX_BOARD_TYPE_CUST_DEFINED_MAX);

		/* Customer private range */
		ENUM_BRD_TYPE_CASE(CVMX_BOARD_TYPE_CUST_PRIVATE_MIN);
		ENUM_BRD_TYPE_CASE(CVMX_BOARD_TYPE_CUST_PRIVATE_MAX);
	}

	return "Unsupported Board";
}

#endif /* __CVMX_BOOTLOADER__ */
