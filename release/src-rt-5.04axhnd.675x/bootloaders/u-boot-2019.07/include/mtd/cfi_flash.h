/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2009
 * Stefan Roese, DENX Software Engineering, sr@denx.de.
 */

#ifndef __CFI_FLASH_H__
#define __CFI_FLASH_H__

#define FLASH_CMD_CFI			0x98
#define FLASH_CMD_READ_ID		0x90
#define FLASH_CMD_RESET			0xff
#define FLASH_CMD_BLOCK_ERASE		0x20
#define FLASH_CMD_ERASE_CONFIRM		0xD0
#define FLASH_CMD_WRITE			0x40
#define FLASH_CMD_PROTECT		0x60
#define FLASH_CMD_SETUP			0x60
#define FLASH_CMD_SET_CR_CONFIRM	0x03
#define FLASH_CMD_PROTECT_SET		0x01
#define FLASH_CMD_PROTECT_CLEAR		0xD0
#define FLASH_CMD_CLEAR_STATUS		0x50
#define FLASH_CMD_READ_STATUS		0x70
#define FLASH_CMD_WRITE_TO_BUFFER	0xE8
#define FLASH_CMD_WRITE_BUFFER_PROG	0xE9
#define FLASH_CMD_WRITE_BUFFER_CONFIRM	0xD0

#define FLASH_STATUS_DONE		0x80
#define FLASH_STATUS_ESS		0x40
#define FLASH_STATUS_ECLBS		0x20
#define FLASH_STATUS_PSLBS		0x10
#define FLASH_STATUS_VPENS		0x08
#define FLASH_STATUS_PSS		0x04
#define FLASH_STATUS_DPS		0x02
#define FLASH_STATUS_R			0x01
#define FLASH_STATUS_PROTECT		0x01

#define AMD_CMD_RESET			0xF0
#define AMD_CMD_WRITE			0xA0
#define AMD_CMD_ERASE_START		0x80
#define AMD_CMD_ERASE_SECTOR		0x30
#define AMD_CMD_UNLOCK_START		0xAA
#define AMD_CMD_UNLOCK_ACK		0x55
#define AMD_CMD_WRITE_TO_BUFFER		0x25
#define AMD_CMD_WRITE_BUFFER_CONFIRM	0x29
#define AMD_CMD_SET_PPB_ENTRY		0xC0
#define AMD_CMD_SET_PPB_EXIT_BC1	0x90
#define AMD_CMD_SET_PPB_EXIT_BC2	0x00
#define AMD_CMD_PPB_UNLOCK_BC1		0x80
#define AMD_CMD_PPB_UNLOCK_BC2		0x30
#define AMD_CMD_PPB_LOCK_BC1		0xA0
#define AMD_CMD_PPB_LOCK_BC2		0x00

#define AMD_STATUS_TOGGLE		0x40
#define AMD_STATUS_ERROR		0x20

#define ATM_CMD_UNLOCK_SECT		0x70
#define ATM_CMD_SOFTLOCK_START		0x80
#define ATM_CMD_LOCK_SECT		0x40

#define FLASH_CONTINUATION_CODE		0x7F

#define FLASH_OFFSET_MANUFACTURER_ID	0x00
#define FLASH_OFFSET_DEVICE_ID		0x01
#define FLASH_OFFSET_LOWER_SW_BITS	0x0C
#define FLASH_OFFSET_DEVICE_ID2		0x0E
#define FLASH_OFFSET_DEVICE_ID3		0x0F
#define FLASH_OFFSET_CFI		0x55
#define FLASH_OFFSET_CFI_ALT		0x555
#define FLASH_OFFSET_CFI_RESP		0x10
#define FLASH_OFFSET_PRIMARY_VENDOR	0x13
/* extended query table primary address */
#define FLASH_OFFSET_EXT_QUERY_T_P_ADDR	0x15
#define FLASH_OFFSET_WTOUT		0x1F
#define FLASH_OFFSET_WBTOUT		0x20
#define FLASH_OFFSET_ETOUT		0x21
#define FLASH_OFFSET_CETOUT		0x22
#define FLASH_OFFSET_WMAX_TOUT		0x23
#define FLASH_OFFSET_WBMAX_TOUT		0x24
#define FLASH_OFFSET_EMAX_TOUT		0x25
#define FLASH_OFFSET_CEMAX_TOUT		0x26
#define FLASH_OFFSET_SIZE		0x27
#define FLASH_OFFSET_INTERFACE		0x28
#define FLASH_OFFSET_BUFFER_SIZE	0x2A
#define FLASH_OFFSET_NUM_ERASE_REGIONS	0x2C
#define FLASH_OFFSET_ERASE_REGIONS	0x2D
#define FLASH_OFFSET_PROTECT		0x02
#define FLASH_OFFSET_USER_PROTECTION	0x85
#define FLASH_OFFSET_INTEL_PROTECTION	0x81

#define CFI_CMDSET_NONE			0
#define CFI_CMDSET_INTEL_EXTENDED	1
#define CFI_CMDSET_AMD_STANDARD		2
#define CFI_CMDSET_INTEL_STANDARD	3
#define CFI_CMDSET_AMD_EXTENDED		4
#define CFI_CMDSET_MITSU_STANDARD	256
#define CFI_CMDSET_MITSU_EXTENDED	257
#define CFI_CMDSET_SST			258
#define CFI_CMDSET_INTEL_PROG_REGIONS	512

#ifdef CONFIG_SYS_FLASH_CFI_AMD_RESET /* needed for STM_ID_29W320DB on UC100 */
# undef  FLASH_CMD_RESET
# define FLASH_CMD_RESET	AMD_CMD_RESET /* use AMD-Reset instead */
#endif

#define NUM_ERASE_REGIONS	4 /* max. number of erase regions */

typedef union {
	u8 w8;
	u16 w16;
	u32 w32;
	u64 w64;
} cfiword_t;

/* CFI standard query structure */
/* The offsets and sizes of this packed structure members correspond
 * to the actual layout in CFI Flash chips. Some 16- and 32-bit members
 * are unaligned and must be accessed with explicit unaligned access macros.
 */
struct cfi_qry {
	u8	qry[3];
	u16	p_id;			/* unaligned */
	u16	p_adr;			/* unaligned */
	u16	a_id;			/* unaligned */
	u16	a_adr;			/* unaligned */
	u8	vcc_min;
	u8	vcc_max;
	u8	vpp_min;
	u8	vpp_max;
	u8	word_write_timeout_typ;
	u8	buf_write_timeout_typ;
	u8	block_erase_timeout_typ;
	u8	chip_erase_timeout_typ;
	u8	word_write_timeout_max;
	u8	buf_write_timeout_max;
	u8	block_erase_timeout_max;
	u8	chip_erase_timeout_max;
	u8	dev_size;
	u16	interface_desc;		/* aligned */
	u16	max_buf_write_size;	/* aligned */
	u8	num_erase_regions;
	u32	erase_region_info[NUM_ERASE_REGIONS];	/* unaligned */
} __attribute__((packed));

struct cfi_pri_hdr {
	u8	pri[3];
	u8	major_version;
	u8	minor_version;
} __attribute__((packed));

#ifndef CONFIG_SYS_FLASH_BANKS_LIST
#define CONFIG_SYS_FLASH_BANKS_LIST { CONFIG_SYS_FLASH_BASE }
#endif

/*
 * CFI_MAX_FLASH_BANKS only used for flash_info struct declaration.
 *
 * Use CONFIG_SYS_MAX_FLASH_BANKS_DETECT if defined
 */
#if defined(CONFIG_SYS_MAX_FLASH_BANKS_DETECT)
#define CONFIG_SYS_MAX_FLASH_BANKS	(cfi_flash_num_flash_banks)
#define CFI_MAX_FLASH_BANKS	CONFIG_SYS_MAX_FLASH_BANKS_DETECT
/* board code can update this variable before CFI detection */
extern int cfi_flash_num_flash_banks;
#else
#define CFI_MAX_FLASH_BANKS	CONFIG_SYS_MAX_FLASH_BANKS
#endif

phys_addr_t cfi_flash_bank_addr(int i);
unsigned long cfi_flash_bank_size(int i);
void flash_cmd_reset(flash_info_t *info);

#ifdef CONFIG_CFI_FLASH_USE_WEAK_ACCESSORS
void flash_write8(u8 value, void *addr);
void flash_write16(u16 value, void *addr);
void flash_write32(u32 value, void *addr);
void flash_write64(u64 value, void *addr);
u8 flash_read8(void *addr);
u16 flash_read16(void *addr);
u32 flash_read32(void *addr);
u64 flash_read64(void *addr);
#endif

#endif /* __CFI_FLASH_H__ */
