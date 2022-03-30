/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2000-2005
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#ifndef _FLASH_H_
#define _FLASH_H_

#ifndef CONFIG_SYS_MAX_FLASH_SECT
#define CONFIG_SYS_MAX_FLASH_SECT	512
#endif

/*-----------------------------------------------------------------------
 * FLASH Info: contains chip specific data, per FLASH bank
 */

typedef struct {
	ulong	size;			/* total bank size in bytes		*/
	ushort	sector_count;		/* number of erase units		*/
	ulong	flash_id;		/* combined device & manufacturer code	*/
	ulong	start[CONFIG_SYS_MAX_FLASH_SECT];   /* virtual sector start address */
	uchar	protect[CONFIG_SYS_MAX_FLASH_SECT]; /* sector protection status	*/
#ifdef CONFIG_SYS_FLASH_CFI
	uchar	portwidth;		/* the width of the port		*/
	uchar	chipwidth;		/* the width of the chip		*/
	ushort	buffer_size;		/* # of bytes in write buffer		*/
	ulong	erase_blk_tout;		/* maximum block erase timeout		*/
	ulong	write_tout;		/* maximum write timeout		*/
	ulong	buffer_write_tout;	/* maximum buffer write timeout		*/
	ushort	vendor;			/* the primary vendor id		*/
	ushort	cmd_reset;		/* vendor specific reset command	*/
	uchar   cmd_erase_sector;	/* vendor specific erase sect. command	*/
	ushort	interface;		/* used for x8/x16 adjustments		*/
	ushort	legacy_unlock;		/* support Intel legacy (un)locking	*/
	ushort	manufacturer_id;	/* manufacturer id			*/
	ushort	device_id;		/* device id				*/
	ushort	device_id2;		/* extended device id			*/
	ushort	ext_addr;		/* extended query table address		*/
	ushort	cfi_version;		/* cfi version				*/
	ushort	cfi_offset;		/* offset for cfi query			*/
	ulong   addr_unlock1;		/* unlock address 1 for AMD flash roms  */
	ulong   addr_unlock2;		/* unlock address 2 for AMD flash roms  */
	uchar   sr_supported;		/* status register supported            */
	const char *name;		/* human-readable name	                */
#endif
#ifdef CONFIG_MTD
	struct mtd_info *mtd;
#endif
#ifdef CONFIG_CFI_FLASH			/* DM-specific parts */
	struct udevice *dev;
	phys_addr_t base;
#endif
} flash_info_t;

extern flash_info_t flash_info[]; /* info for FLASH chips	*/

typedef unsigned long flash_sect_t;

/*
 * Values for the width of the port
 */
#define FLASH_CFI_8BIT		0x01
#define FLASH_CFI_16BIT		0x02
#define FLASH_CFI_32BIT		0x04
#define FLASH_CFI_64BIT		0x08
/*
 * Values for the width of the chip
 */
#define FLASH_CFI_BY8		0x01
#define FLASH_CFI_BY16		0x02
#define FLASH_CFI_BY32		0x04
#define FLASH_CFI_BY64		0x08
/*
 * Values for the flash device interface
 */
#define FLASH_CFI_X8		0x00
#define FLASH_CFI_X16		0x01
#define FLASH_CFI_X8X16		0x02
#define FLASH_CFI_X16X32	0x05

/* convert between bit value and numeric value */
#define CFI_FLASH_SHIFT_WIDTH	3

/* Prototypes */

extern unsigned long flash_init (void);
extern void flash_print_info (flash_info_t *);
extern int flash_erase	(flash_info_t *, int, int);
extern int flash_sect_erase (ulong addr_first, ulong addr_last);
extern int flash_sect_protect (int flag, ulong addr_first, ulong addr_last);
extern int flash_sect_roundb (ulong *addr);
extern unsigned long flash_sector_size(flash_info_t *info, flash_sect_t sect);
extern void flash_set_verbose(uint);

/* common/flash.c */
extern void flash_protect (int flag, ulong from, ulong to, flash_info_t *info);
extern int flash_write (char *, ulong, ulong);
extern flash_info_t *addr2info (ulong);
extern int write_buff (flash_info_t *info, uchar *src, ulong addr, ulong cnt);

/* drivers/mtd/cfi_mtd.c */
#ifdef CONFIG_FLASH_CFI_MTD
extern int cfi_mtd_init(void);
#endif

/* board/?/flash.c */
#if defined(CONFIG_SYS_FLASH_PROTECTION)
extern int flash_real_protect(flash_info_t *info, long sector, int prot);
extern void flash_read_user_serial(flash_info_t * info, void * buffer, int offset, int len);
extern void flash_read_factory_serial(flash_info_t * info, void * buffer, int offset, int len);
#endif	/* CONFIG_SYS_FLASH_PROTECTION */

#ifdef CONFIG_FLASH_CFI_LEGACY
extern ulong board_flash_get_legacy(ulong base, int banknum, flash_info_t *info);
extern int jedec_flash_match(flash_info_t *info, ulong base);
#define CFI_CMDSET_AMD_LEGACY		0xFFF0
#endif

/*-----------------------------------------------------------------------
 * return codes from flash_write():
 */
#define ERR_OK				0
#define ERR_TIMEOUT			1
#define ERR_NOT_ERASED			2
#define ERR_PROTECTED			4
#define ERR_INVAL			8
#define ERR_ALIGN			16
#define ERR_UNKNOWN_FLASH_VENDOR	32
#define ERR_UNKNOWN_FLASH_TYPE		64
#define ERR_PROG_ERROR			128
#define ERR_ABORTED			256

/*-----------------------------------------------------------------------
 * Protection Flags for flash_protect():
 */
#define FLAG_PROTECT_SET	0x01
#define FLAG_PROTECT_CLEAR	0x02
#define	FLAG_PROTECT_INVALID	0x03
/*-----------------------------------------------------------------------
 * Set Environment according to label:
 */
#define	FLAG_SETENV		0x80

/*-----------------------------------------------------------------------
 * Device IDs
 */

/* Manufacturers inside bank 0 have ids like 0x00xx00xx */
#define AMD_MANUFACT	0x00010001	/* AMD	   manuf. ID in D23..D16, D7..D0 */
#define FUJ_MANUFACT	0x00040004	/* FUJITSU manuf. ID in D23..D16, D7..D0 */
#define ATM_MANUFACT	0x001F001F	/* ATMEL */
#define STM_MANUFACT	0x00200020	/* STM (Thomson) manuf. ID in D23.. -"- */
#define SST_MANUFACT	0x00BF00BF	/* SST	   manuf. ID in D23..D16, D7..D0 */
#define MT_MANUFACT	0x00890089	/* MT	   manuf. ID in D23..D16, D7..D0 */
#define INTEL_MANUFACT	0x00890089	/* INTEL   manuf. ID in D23..D16, D7..D0 */
#define INTEL_ALT_MANU	0x00B000B0	/* alternate INTEL namufacturer ID	*/
#define MX_MANUFACT	0x00C200C2	/* MXIC	   manuf. ID in D23..D16, D7..D0 */
#define TOSH_MANUFACT	0x00980098	/* TOSHIBA manuf. ID in D23..D16, D7..D0 */
#define MT2_MANUFACT	0x002C002C	/* alternate MICRON manufacturer ID*/
#define EXCEL_MANUFACT	0x004A004A	/* Excel Semiconductor			*/
#define AMIC_MANUFACT	0x00370037	/* AMIC    manuf. ID in D23..D16, D7..D0 */
#define WINB_MANUFACT	0x00DA00DA	/* Winbond manuf. ID in D23..D16, D7..D0 */
#define EON_ALT_MANU	0x001C001C	/* EON     manuf. ID in D23..D16, D7..D0 */

/* Manufacturers inside bank 1 have ids like 0x01xx01xx */
#define EON_MANUFACT	0x011C011C	/* EON     manuf. ID in D23..D16, D7..D0 */

/* Manufacturers inside bank 2 have ids like 0x02xx02xx */

					/* Micron Technologies (INTEL compat.)	*/
#define MT_ID_28F400_T	0x44704470	/* 28F400B3 ID ( 4 M, top boot sector)	*/
#define MT_ID_28F400_B	0x44714471	/* 28F400B3 ID ( 4 M, bottom boot sect) */

#define AMD_ID_LV040B	0x4F		/* 29LV040B ID				*/
					/* 4 Mbit, 512K x 8,			*/
					/* 8 64K x 8 uniform sectors		*/
#define AMD_ID_F033C	0xA3		/* 29LV033C ID				*/
					/* 32 Mbit, 4Mbits x 8,			*/
					/* 64 64K x 8 uniform sectors		*/
#define AMD_ID_F065D	0x93		/* 29LV065D ID				*/
					/* 64 Mbit, 8Mbits x 8,			*/
					/* 126 64K x 8 uniform sectors		*/
#define ATM_ID_LV040	0x13		/* 29LV040B ID				*/
					/* 4 Mbit, 512K x 8,			*/
					/* 8 64K x 8 uniform sectors		*/
#define AMD_ID_F040B	0xA4		/* 29F040B ID				*/
					/* 4 Mbit, 512K x 8,			*/
					/* 8 64K x 8 uniform sectors		*/
#define STM_ID_M29W040B 0xE3		/* M29W040B ID				*/
					/* 4 Mbit, 512K x 8,			*/
					/* 8 64K x 8 uniform sectors		*/
#define AMD_ID_F080B	0xD5		/* 29F080  ID  ( 1 M)			*/
					/* 8 Mbit, 512K x 16,			*/
					/* 8 64K x 16 uniform sectors		*/
#define AMD_ID_F016D	0xAD		/* 29F016  ID  ( 2 M x 8)		*/
#define AMD_ID_F032B	0x41		/* 29F032  ID  ( 4 M x 8)		*/
#define AMD_ID_LV116DT	0xC7		/* 29LV116DT   ( 2 M x 8, top boot sect) */
#define AMD_ID_LV116DB	0x4C		/* 29LV116DB   ( 2 M x 8, bottom boot sect) */
#define AMD_ID_LV016B	0xc8		/* 29LV016 ID  ( 2 M x 8)		*/

#define AMD_ID_PL160CB	0x22452245	/* 29PL160CB ID (16 M, bottom boot sect */

#define AMD_ID_LV400T	0x22B922B9	/* 29LV400T ID ( 4 M, top boot sector)	*/
#define AMD_ID_LV400B	0x22BA22BA	/* 29LV400B ID ( 4 M, bottom boot sect) */

#define AMD_ID_LV033C	0xA3		/* 29LV033C ID ( 4 M x 8)		*/
#define AMD_ID_LV065D	0x93		/* 29LV065D ID ( 8 M x 8)		*/

#define AMD_ID_LV800T	0x22DA22DA	/* 29LV800T ID ( 8 M, top boot sector)	*/
#define AMD_ID_LV800B	0x225B225B	/* 29LV800B ID ( 8 M, bottom boot sect) */

#define AMD_ID_LV160T	0x22C422C4	/* 29LV160T ID (16 M, top boot sector)	*/
#define AMD_ID_LV160B	0x22492249	/* 29LV160B ID (16 M, bottom boot sect) */

#define AMD_ID_DL163T	0x22282228	/* 29DL163T ID (16 M, top boot sector)	*/
#define AMD_ID_DL163B	0x222B222B	/* 29DL163B ID (16 M, bottom boot sect) */

#define AMD_ID_LV320T	0x22F622F6	/* 29LV320T ID (32 M, top boot sector)	*/
#define MX_ID_LV320T	0x22A722A7	/* 29LV320T by Macronix, AMD compatible */
#define AMD_ID_LV320B	0x22F922F9	/* 29LV320B ID (32 M, bottom boot sect) */
#define MX_ID_LV320B	0x22A822A8	/* 29LV320B by Macronix, AMD compatible */

#define AMD_ID_DL322T	0x22552255	/* 29DL322T ID (32 M, top boot sector)	*/
#define AMD_ID_DL322B	0x22562256	/* 29DL322B ID (32 M, bottom boot sect) */
#define AMD_ID_DL323T	0x22502250	/* 29DL323T ID (32 M, top boot sector)	*/
#define AMD_ID_DL323B	0x22532253	/* 29DL323B ID (32 M, bottom boot sect) */
#define AMD_ID_DL324T	0x225C225C	/* 29DL324T ID (32 M, top boot sector)	*/
#define AMD_ID_DL324B	0x225F225F	/* 29DL324B ID (32 M, bottom boot sect) */

#define AMD_ID_DL640	0x227E227E	/* 29DL640D ID (64 M, dual boot sectors)*/
#define AMD_ID_MIRROR	0x227E227E	/* 1st ID word for MirrorBit family */
#define AMD_ID_DL640G_2 0x22022202	/* 2nd ID word for AM29DL640G  at 0x38 */
#define AMD_ID_DL640G_3 0x22012201	/* 3rd ID word for AM29DL640G  at 0x3c */
#define AMD_ID_LV640U_2 0x220C220C	/* 2nd ID word for AM29LV640M  at 0x38 */
#define AMD_ID_LV640U_3 0x22012201	/* 3rd ID word for AM29LV640M  at 0x3c */
#define AMD_ID_LV640MT_2 0x22102210	/* 2nd ID word for AM29LV640MT at 0x38 */
#define AMD_ID_LV640MT_3 0x22012201	/* 3rd ID word for AM29LV640MT at 0x3c */
#define AMD_ID_LV640MB_2 0x22102210	/* 2nd ID word for AM29LV640MB at 0x38 */
#define AMD_ID_LV640MB_3 0x22002200	/* 3rd ID word for AM29LV640MB at 0x3c */
#define AMD_ID_LV128U_2 0x22122212	/* 2nd ID word for AM29LV128M  at 0x38 */
#define AMD_ID_LV128U_3 0x22002200	/* 3rd ID word for AM29LV128M  at 0x3c */
#define AMD_ID_LV256U_2 0x22122212	/* 2nd ID word for AM29LV256M  at 0x38 */
#define AMD_ID_LV256U_3 0x22012201	/* 3rd ID word for AM29LV256M  at 0x3c */
#define AMD_ID_GL064M_2 0x22132213	/* 2nd ID word for S29GL064M-R6 */
#define AMD_ID_GL064M_3 0x22012201	/* 3rd ID word for S29GL064M-R6 */
#define AMD_ID_GL064MT_2 0x22102210	/* 2nd ID word for S29GL064M-R3 (top boot sector) */
#define AMD_ID_GL064MT_3 0x22012201	/* 3rd ID word for S29GL064M-R3 (top boot sector) */
#define AMD_ID_GL128N_2	0x22212221	/* 2nd ID word for S29GL128N */
#define AMD_ID_GL128N_3	0x22012201	/* 3rd ID word for S29GL128N */


#define AMD_ID_LV320B_2 0x221A221A	/* 2d ID word for AM29LV320MB at 0x38 */
#define AMD_ID_LV320B_3 0x22002200	/* 3d ID word for AM29LV320MB at 0x3c */

#define AMD_ID_LV640U	0x22D722D7	/* 29LV640U ID (64 M, uniform sectors)	*/
#define AMD_ID_LV650U	0x22D722D7	/* 29LV650U ID (64 M, uniform sectors)	*/

#define ATM_ID_BV1614	0x000000C0	/* 49BV1614  ID */
#define ATM_ID_BV1614A	0x000000C8	/* 49BV1614A ID */
#define ATM_ID_BV6416	0x000000D6	/* 49BV6416  ID */

#define FUJI_ID_29F800BA  0x22582258	/* MBM29F800BA ID  (8M) */
#define FUJI_ID_29F800TA  0x22D622D6	/* MBM29F800TA ID  (8M) */
#define FUJI_ID_29LV650UE 0x22d722d7	/* MBM29LV650UE/651UE ID (8M = 128 x 32kWord) */

#define SST_ID_xF200A	0x27892789	/* 39xF200A ID ( 2M = 128K x 16 )	*/
#define SST_ID_xF400A	0x27802780	/* 39xF400A ID ( 4M = 256K x 16 )	*/
#define SST_ID_xF800A	0x27812781	/* 39xF800A ID ( 8M = 512K x 16 )	*/
#define SST_ID_xF160A	0x27822782	/* 39xF800A ID (16M =	1M x 16 )	*/
#define SST_ID_xF1601	0x234B234B	/* 39xF1601 ID (16M =	1M x 16 )	*/
#define SST_ID_xF1602	0x234A234A	/* 39xF1602 ID (16M =	1M x 16 )	*/
#define SST_ID_xF3201	0x235B235B	/* 39xF3201 ID (32M =	2M x 16 )	*/
#define SST_ID_xF3202	0x235A235A	/* 39xF3202 ID (32M =	2M x 16 )	*/
#define SST_ID_xF6401	0x236B236B	/* 39xF6401 ID (64M =	4M x 16 )	*/
#define SST_ID_xF6402	0x236A236A	/* 39xF6402 ID (64M =	4M x 16 )	*/
#define SST_ID_xF020	0xBFD6BFD6	/* 39xF020 ID (256KB = 2Mbit x 8)	*/
#define SST_ID_xF040	0xBFD7BFD7	/* 39xF040 ID (512KB = 4Mbit x 8)	*/

#define STM_ID_F040B	0xE2		/* M29F040B ID ( 4M = 512K x 8	)	*/
					/* 8 64K x 8 uniform sectors		*/

#define STM_ID_x800AB	0x005B005B	/* M29W800AB ID (8M = 512K x 16 )	*/
#define STM_ID_29W320DT 0x22CA22CA	/* M29W320DT ID (32 M, top boot sector) */
#define STM_ID_29W320DB 0x22CB22CB	/* M29W320DB ID (32 M, bottom boot sect)	*/
#define STM_ID_29W320ET 0x22562256	/* M29W320ET ID (32 M, top boot sector) */
#define STM_ID_29W320EB 0x22572257	/* M29W320EB ID (32 M, bottom boot sect)*/
#define STM_ID_29W040B	0x00E300E3	/* M29W040B ID (4M = 512K x 8)	*/
#define FLASH_PSD4256GV 0x00E9		/* PSD4256 Flash and CPLD combination	*/

#define INTEL_ID_28F016S    0x66a066a0	/* 28F016S[VS] ID (16M = 512k x 16)	*/
#define INTEL_ID_28F800B3T  0x88928892	/*  8M = 512K x 16 top boot sector	*/
#define INTEL_ID_28F800B3B  0x88938893	/*  8M = 512K x 16 bottom boot sector	*/
#define INTEL_ID_28F160B3T  0x88908890	/*  16M = 1M x 16 top boot sector	*/
#define INTEL_ID_28F160B3B  0x88918891	/*  16M = 1M x 16 bottom boot sector	*/
#define INTEL_ID_28F320B3T  0x88968896	/*  32M = 2M x 16 top boot sector	*/
#define INTEL_ID_28F320B3B  0x88978897	/*  32M = 2M x 16 bottom boot sector	*/
#define INTEL_ID_28F640B3T  0x88988898	/*  64M = 4M x 16 top boot sector	*/
#define INTEL_ID_28F640B3B  0x88998899	/*  64M = 4M x 16 bottom boot sector	*/
#define INTEL_ID_28F160F3B  0x88F488F4	/*  16M = 1M x 16 bottom boot sector	*/

#define INTEL_ID_28F800C3T  0x88C088C0	/*  8M = 512K x 16 top boot sector	*/
#define INTEL_ID_28F800C3B  0x88C188C1	/*  8M = 512K x 16 bottom boot sector	*/
#define INTEL_ID_28F160C3T  0x88C288C2	/*  16M = 1M x 16 top boot sector	*/
#define INTEL_ID_28F160C3B  0x88C388C3	/*  16M = 1M x 16 bottom boot sector	*/
#define INTEL_ID_28F320C3T  0x88C488C4	/*  32M = 2M x 16 top boot sector	*/
#define INTEL_ID_28F320C3B  0x88C588C5	/*  32M = 2M x 16 bottom boot sector	*/
#define INTEL_ID_28F640C3T  0x88CC88CC	/*  64M = 4M x 16 top boot sector	*/
#define INTEL_ID_28F640C3B  0x88CD88CD	/*  64M = 4M x 16 bottom boot sector	*/

#define INTEL_ID_28F128J3   0x89188918	/*  16M = 8M x 16 x 128 */
#define INTEL_ID_28F320J5   0x00140014	/*  32M = 128K x  32	*/
#define INTEL_ID_28F640J5   0x00150015	/*  64M = 128K x  64	*/
#define INTEL_ID_28F320J3A  0x00160016	/*  32M = 128K x  32	*/
#define INTEL_ID_28F640J3A  0x00170017	/*  64M = 128K x  64	*/
#define INTEL_ID_28F128J3A  0x00180018	/* 128M = 128K x 128	*/
#define INTEL_ID_28F256J3A  0x001D001D	/* 256M = 128K x 256	*/
#define INTEL_ID_28F256L18T 0x880D880D	/* 256M = 128K x 255 + 32k x 4 */
#define INTEL_ID_28F64K3    0x88018801	/*  64M =  32K x 255 + 32k x 4 */
#define INTEL_ID_28F128K3   0x88028802	/* 128M =  64K x 255 + 32k x 4 */
#define INTEL_ID_28F256K3   0x88038803	/* 256M = 128K x 255 + 32k x 4 */
#define INTEL_ID_28F64P30T  0x88178817	/*  64M =  32K x 255 + 32k x 4 */
#define INTEL_ID_28F64P30B  0x881A881A	/*  64M =  32K x 255 + 32k x 4 */
#define INTEL_ID_28F128P30T 0x88188818	/* 128M =  64K x 255 + 32k x 4 */
#define INTEL_ID_28F128P30B 0x881B881B	/* 128M =  64K x 255 + 32k x 4 */
#define INTEL_ID_28F256P30T 0x88198819	/* 256M = 128K x 255 + 32k x 4 */
#define INTEL_ID_28F256P30B 0x881C881C	/* 256M = 128K x 255 + 32k x 4 */

#define INTEL_ID_28F160S3   0x00D000D0	/*  16M = 512K x  32 (64kB x 32)	*/
#define INTEL_ID_28F320S3   0x00D400D4	/*  32M = 512K x  64 (64kB x 64)	*/

/* Note that the Sharp 28F016SC is compatible with the Intel E28F016SC */
#define SHARP_ID_28F016SCL  0xAAAAAAAA	/* LH28F016SCT-L95 2Mx8, 32 64k blocks	*/
#define SHARP_ID_28F016SCZ  0xA0A0A0A0	/* LH28F016SCT-Z4  2Mx8, 32 64k blocks	*/
#define SHARP_ID_28F008SC   0xA6A6A6A6	/* LH28F008SCT-L12 1Mx8, 16 64k blocks	*/
					/* LH28F008SCR-L85 1Mx8, 16 64k blocks	*/

#define TOSH_ID_FVT160	0xC2		/* TC58FVT160 ID (16 M, top )		*/
#define TOSH_ID_FVB160	0x43		/* TC58FVT160 ID (16 M, bottom )	*/
#define NUMONYX_256MBIT	0x8922		/* Numonyx P33/30 256MBit 65nm	*/

/*-----------------------------------------------------------------------
 * Internal FLASH identification codes
 *
 * Be careful when adding new type! Odd numbers are "bottom boot sector" types!
 */

#define FLASH_AM040	0x0001		/* AMD Am29F040B, Am29LV040B		*/
					/* Bright Micro BM29F040		*/
					/* Fujitsu MBM29F040A			*/
					/* STM M29W040B				*/
					/* SGS Thomson M29F040B			*/
					/* 8 64K x 8 uniform sectors		*/
#define FLASH_AM400T	0x0002		/* AMD AM29LV400			*/
#define FLASH_AM400B	0x0003
#define FLASH_AM800T	0x0004		/* AMD AM29LV800			*/
#define FLASH_AM800B	0x0005
#define FLASH_AM116DT	0x0026		/* AMD AM29LV116DT (2Mx8bit) */
#define FLASH_AM116DB	0x0027		/* AMD AM29LV116DB (2Mx8bit) */
#define FLASH_AM160T	0x0006		/* AMD AM29LV160			*/
#define FLASH_AM160LV	0x0046		/* AMD29LV160DB (2M = 2Mx8bit ) */
#define FLASH_AM160B	0x0007
#define FLASH_AM320T	0x0008		/* AMD AM29LV320			*/
#define FLASH_AM320B	0x0009

#define FLASH_AM080	0x000A		/* AMD Am29F080B			*/
					/* 16 64K x 8 uniform sectors		*/

#define FLASH_AMDL322T	0x0010		/* AMD AM29DL322			*/
#define FLASH_AMDL322B	0x0011
#define FLASH_AMDL323T	0x0012		/* AMD AM29DL323			*/
#define FLASH_AMDL323B	0x0013
#define FLASH_AMDL324T	0x0014		/* AMD AM29DL324			*/
#define FLASH_AMDL324B	0x0015

#define FLASH_AMDLV033C 0x0018
#define FLASH_AMDLV065D 0x001A

#define FLASH_AMDL640	0x0016		/* AMD AM29DL640D			*/
#define FLASH_AMD016	0x0018		/* AMD AM29F016D			*/
#define FLASH_AMDL640MB 0x0019		/* AMD AM29LV640MB (64M, bottom boot sect)*/
#define FLASH_AMDL640MT 0x001A		/* AMD AM29LV640MT (64M, top boot sect) */

#define FLASH_SST200A	0x0040		/* SST 39xF200A ID (  2M = 128K x 16 )	*/
#define FLASH_SST400A	0x0042		/* SST 39xF400A ID (  4M = 256K x 16 )	*/
#define FLASH_SST800A	0x0044		/* SST 39xF800A ID (  8M = 512K x 16 )	*/
#define FLASH_SST160A	0x0046		/* SST 39xF160A ID ( 16M =   1M x 16 )	*/
#define FLASH_SST320	0x0048		/* SST 39xF160A ID ( 16M =   1M x 16 )	*/
#define FLASH_SST640	0x004A		/* SST 39xF160A ID ( 16M =   1M x 16 )	*/
#define FLASH_SST020	0x0024		/* SST 39xF020 ID (256KB = 2Mbit x 8 )	*/
#define FLASH_SST040	0x000E		/* SST 39xF040 ID (512KB = 4Mbit x 8 )	*/

#define FLASH_STM800AB	0x0051		/* STM M29WF800AB  (  8M = 512K x 16 )	*/
#define FLASH_STMW320DT 0x0052		/* STM M29W320DT   (32 M, top boot sector)	*/
#define FLASH_STMW320DB 0x0053		/* STM M29W320DB   (32 M, bottom boot sect)*/
#define FLASH_STM320DB	0x00CB		/* STM M29W320DB (4M = 64K x 64, bottom)*/
#define FLASH_STM800DT	0x00D7		/* STM M29W800DT (1M = 64K x 16, top)	*/
#define FLASH_STM800DB	0x005B		/* STM M29W800DB (1M = 64K x 16, bottom)*/

#define FLASH_MCHP100T	0x0060		/* MCHP internal (1M = 64K x 16) */
#define FLASH_MCHP100B	0x0061		/* MCHP internal (1M = 64K x 16) */

#define FLASH_28F400_T	0x0062		/* MT  28F400B3 ID (  4M = 256K x 16 )	*/
#define FLASH_28F400_B	0x0063		/* MT  28F400B3 ID (  4M = 256K x 16 )	*/

#define FLASH_INTEL800T 0x0074		/* INTEL 28F800B3T (  8M = 512K x 16 )	*/
#define FLASH_INTEL800B 0x0075		/* INTEL 28F800B3B (  8M = 512K x 16 )	*/
#define FLASH_INTEL160T 0x0076		/* INTEL 28F160B3T ( 16M =  1 M x 16 )	*/
#define FLASH_INTEL160B 0x0077		/* INTEL 28F160B3B ( 16M =  1 M x 16 )	*/
#define FLASH_INTEL320T 0x0078		/* INTEL 28F320B3T ( 32M =  2 M x 16 )	*/
#define FLASH_INTEL320B 0x0079		/* INTEL 28F320B3B ( 32M =  2 M x 16 )	*/
#define FLASH_INTEL640T 0x007A		/* INTEL 28F320B3T ( 64M =  4 M x 16 )	*/
#define FLASH_INTEL640B 0x007B		/* INTEL 28F320B3B ( 64M =  4 M x 16 )	*/

#define FLASH_28F008S5	0x0080		/* Intel 28F008S5  (  1M =  64K x 16 )	*/
#define FLASH_28F016SV	0x0081		/* Intel 28F016SV  ( 16M = 512k x 32 )	*/
#define FLASH_28F800_B	0x0083		/* Intel E28F800B  (  1M = ? )		*/
#define FLASH_AM29F800B 0x0084		/* AMD Am29F800BB  (  1M = ? )		*/
#define FLASH_28F320J5	0x0085		/* Intel 28F320J5  (  4M = 128K x 32 )	*/
#define FLASH_28F160S3	0x0086		/* Intel 28F160S3  ( 16M = 512K x 32 )	*/
#define FLASH_28F320S3	0x0088		/* Intel 28F320S3  ( 32M = 512K x 64 )	*/
#define FLASH_AM640U	0x0090		/* AMD Am29LV640U  ( 64M = 4M x 16 )	*/
#define FLASH_AM033C	0x0091		/* AMD AM29LV033   ( 32M = 4M x 8 )	*/
#define FLASH_LH28F016SCT 0x0092	/* Sharp 28F016SCT ( 8 Meg Flash SIMM ) */
#define FLASH_28F160F3B 0x0093		/* Intel 28F160F3B ( 16M = 1M x 16 )	*/
#define FLASH_AM065D	0x0093

#define FLASH_28F640J5	0x0099		/* INTEL 28F640J5  ( 64M = 128K x  64)	*/

#define FLASH_28F800C3T 0x009A		/* Intel 28F800C3T (  8M = 512K x 16 )	*/
#define FLASH_28F800C3B 0x009B		/* Intel 28F800C3B (  8M = 512K x 16 )	*/
#define FLASH_28F160C3T 0x009C		/* Intel 28F160C3T ( 16M = 1M x 16 )	*/
#define FLASH_28F160C3B 0x009D		/* Intel 28F160C3B ( 16M = 1M x 16 )	*/
#define FLASH_28F320C3T 0x009E		/* Intel 28F320C3T ( 32M = 2M x 16 )	*/
#define FLASH_28F320C3B 0x009F		/* Intel 28F320C3B ( 32M = 2M x 16 )	*/
#define FLASH_28F640C3T 0x00A0		/* Intel 28F640C3T ( 64M = 4M x 16 )	*/
#define FLASH_28F640C3B 0x00A1		/* Intel 28F640C3B ( 64M = 4M x 16 )	*/
#define FLASH_AMLV320U	0x00A2		/* AMD 29LV320M	   ( 32M = 2M x 16 )	*/

#define FLASH_AM033	0x00A3		/* AMD AmL033C90V1   (32M = 4M x 8)	*/
#define FLASH_AM065	0x0093		/* AMD AmL065DU12RI  (64M = 8M x 8)	*/
#define FLASH_AT040	0x00A5		/* Amtel AT49LV040   (4M = 512K x 8)	*/

#define FLASH_AMLV640U	0x00A4		/* AMD 29LV640M	   ( 64M = 4M x 16 )	*/
#define FLASH_AMLV128U	0x00A6		/* AMD 29LV128M	   ( 128M = 8M x 16 )	*/
#define FLASH_AMLV320B	0x00A7		/* AMD 29LV320MB   ( 32M = 2M x 16 )	*/
#define FLASH_AMLV320T	0x00A8		/* AMD 29LV320MT   ( 32M = 2M x 16 )	*/
#define FLASH_AMLV256U	0x00AA		/* AMD 29LV256M	   ( 256M = 16M x 16 )	*/
#define FLASH_MXLV320B	0x00AB		/* MX  29LV320MB   ( 32M = 2M x 16 )	*/
#define FLASH_MXLV320T	0x00AC		/* MX  29LV320MT   ( 32M = 2M x 16 )	*/
#define FLASH_28F256L18T 0x00B0		/* Intel 28F256L18T 256M = 128K x 255 + 32k x 4 */
#define FLASH_AMDL163T	0x00B2		/* AMD AM29DL163T (2M x 16 )			*/
#define FLASH_AMDL163B	0x00B3
#define FLASH_28F64K3	0x00B4		/* Intel 28F64K3   (  64M)		*/
#define FLASH_28F128K3	0x00B6		/* Intel 28F128K3  ( 128M = 8M x 16 )	*/
#define FLASH_28F256K3	0x00B8		/* Intel 28F256K3  ( 256M = 16M x 16 )	*/

#define FLASH_28F320J3A 0x00C0		/* INTEL 28F320J3A ( 32M = 128K x  32)	*/
#define FLASH_28F640J3A 0x00C2		/* INTEL 28F640J3A ( 64M = 128K x  64)	*/
#define FLASH_28F128J3A 0x00C4		/* INTEL 28F128J3A (128M = 128K x 128)	*/
#define FLASH_28F256J3A 0x00C6		/* INTEL 28F256J3A (256M = 128K x 256)	*/

#define FLASH_FUJLV650	0x00D0		/* Fujitsu MBM 29LV650UE/651UE		*/
#define FLASH_MT28S4M16LC 0x00E1	/* Micron MT28S4M16LC			*/
#define FLASH_S29GL064M 0x00F0		/* Spansion S29GL064M-R6		*/
#define FLASH_S29GL128N 0x00F1		/* Spansion S29GL128N			*/

#define FLASH_STM32	0x00F2		/* STM32 Embedded Flash */

#define FLASH_UNKNOWN	0xFFFF		/* unknown flash type			*/


/* manufacturer offsets
 */
#define FLASH_MAN_AMD	0x00000000	/* AMD					*/
#define FLASH_MAN_FUJ	0x00010000	/* Fujitsu				*/
#define FLASH_MAN_BM	0x00020000	/* Bright Microelectronics		*/
#define FLASH_MAN_MX	0x00030000	/* MXIC					*/
#define FLASH_MAN_STM	0x00040000
#define FLASH_MAN_TOSH	0x00050000	/* Toshiba				*/
#define FLASH_MAN_EXCEL 0x00060000	/* Excel Semiconductor			*/
#define FLASH_MAN_SST	0x00100000
#define FLASH_MAN_INTEL 0x00300000
#define FLASH_MAN_MT	0x00400000
#define FLASH_MAN_SHARP 0x00500000
#define FLASH_MAN_ATM	0x00600000
#define FLASH_MAN_CFI	0x01000000
#define FLASH_MAN_MCHP	0x02000000	/* Microchip Technology		*/

#define FLASH_TYPEMASK	0x0000FFFF	/* extract FLASH type	information	*/
#define FLASH_VENDMASK	0xFFFF0000	/* extract FLASH vendor information	*/

#define FLASH_AMD_COMP	0x000FFFFF	/* Up to this ID, FLASH is compatible	*/
					/* with AMD, Fujitsu and SST		*/
					/* (JEDEC standard commands ?)		*/

#define FLASH_BTYPE	0x0001		/* mask for bottom boot sector type	*/

/*-----------------------------------------------------------------------
 * Timeout constants:
 *
 * We can't find any specifications for maximum chip erase times,
 * so these values are guestimates.
 */
#define FLASH_ERASE_TIMEOUT	120000	/* timeout for erasing in ms		*/
#define FLASH_WRITE_TIMEOUT	500	/* timeout for writes  in ms		*/

#endif /* _FLASH_H_ */
