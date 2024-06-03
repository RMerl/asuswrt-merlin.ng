// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2002-2004
 * Brad Kemp, Seranoa Networks, Brad.Kemp@seranoa.com
 *
 * Copyright (C) 2003 Arabella Software Ltd.
 * Yuli Barcohen <yuli@arabellasw.com>
 *
 * Copyright (C) 2004
 * Ed Okerson
 *
 * Copyright (C) 2006
 * Tolunay Orkun <listmember@orkun.us>
 */

/* The DEBUG define must be before common to enable debugging */
/* #define DEBUG	*/

#include <common.h>
#include <console.h>
#include <dm.h>
#include <errno.h>
#include <fdt_support.h>
#include <asm/processor.h>
#include <asm/io.h>
#include <asm/byteorder.h>
#include <asm/unaligned.h>
#include <environment.h>
#include <mtd/cfi_flash.h>
#include <watchdog.h>

/*
 * This file implements a Common Flash Interface (CFI) driver for
 * U-Boot.
 *
 * The width of the port and the width of the chips are determined at
 * initialization.  These widths are used to calculate the address for
 * access CFI data structures.
 *
 * References
 * JEDEC Standard JESD68 - Common Flash Interface (CFI)
 * JEDEC Standard JEP137-A Common Flash Interface (CFI) ID Codes
 * Intel Application Note 646 Common Flash Interface (CFI) and Command Sets
 * Intel 290667-008 3 Volt Intel StrataFlash Memory datasheet
 * AMD CFI Specification, Release 2.0 December 1, 2001
 * AMD/Spansion Application Note: Migration from Single-byte to Three-byte
 *   Device IDs, Publication Number 25538 Revision A, November 8, 2001
 *
 * Define CONFIG_SYS_WRITE_SWAPPED_DATA, if you have to swap the Bytes between
 * reading and writing ... (yes there is such a Hardware).
 */

DECLARE_GLOBAL_DATA_PTR;

static uint flash_offset_cfi[2] = { FLASH_OFFSET_CFI, FLASH_OFFSET_CFI_ALT };
#ifdef CONFIG_FLASH_CFI_MTD
static uint flash_verbose = 1;
#else
#define flash_verbose 1
#endif

flash_info_t flash_info[CFI_MAX_FLASH_BANKS];	/* FLASH chips info */

/*
 * Check if chip width is defined. If not, start detecting with 8bit.
 */
#ifndef CONFIG_SYS_FLASH_CFI_WIDTH
#define CONFIG_SYS_FLASH_CFI_WIDTH	FLASH_CFI_8BIT
#endif

#ifdef CONFIG_CFI_FLASH_USE_WEAK_ACCESSORS
#define __maybe_weak __weak
#else
#define __maybe_weak static
#endif

/*
 * 0xffff is an undefined value for the configuration register. When
 * this value is returned, the configuration register shall not be
 * written at all (default mode).
 */
static u16 cfi_flash_config_reg(int i)
{
#ifdef CONFIG_SYS_CFI_FLASH_CONFIG_REGS
	return ((u16 [])CONFIG_SYS_CFI_FLASH_CONFIG_REGS)[i];
#else
	return 0xffff;
#endif
}

#if defined(CONFIG_SYS_MAX_FLASH_BANKS_DETECT)
int cfi_flash_num_flash_banks = CONFIG_SYS_MAX_FLASH_BANKS_DETECT;
#else
int cfi_flash_num_flash_banks;
#endif

#ifdef CONFIG_CFI_FLASH /* for driver model */
static void cfi_flash_init_dm(void)
{
	struct udevice *dev;

	cfi_flash_num_flash_banks = 0;
	/*
	 * The uclass_first_device() will probe the first device and
	 * uclass_next_device() will probe the rest if they exist. So
	 * that cfi_flash_probe() will get called assigning the base
	 * addresses that are available.
	 */
	for (uclass_first_device(UCLASS_MTD, &dev);
	     dev;
	     uclass_next_device(&dev)) {
	}
}

phys_addr_t cfi_flash_bank_addr(int i)
{
	return flash_info[i].base;
}
#else
__weak phys_addr_t cfi_flash_bank_addr(int i)
{
	return ((phys_addr_t [])CONFIG_SYS_FLASH_BANKS_LIST)[i];
}
#endif

__weak unsigned long cfi_flash_bank_size(int i)
{
#ifdef CONFIG_SYS_FLASH_BANKS_SIZES
	return ((unsigned long [])CONFIG_SYS_FLASH_BANKS_SIZES)[i];
#else
	return 0;
#endif
}

__maybe_weak void flash_write8(u8 value, void *addr)
{
	__raw_writeb(value, addr);
}

__maybe_weak void flash_write16(u16 value, void *addr)
{
	__raw_writew(value, addr);
}

__maybe_weak void flash_write32(u32 value, void *addr)
{
	__raw_writel(value, addr);
}

__maybe_weak void flash_write64(u64 value, void *addr)
{
	/* No architectures currently implement __raw_writeq() */
	*(volatile u64 *)addr = value;
}

__maybe_weak u8 flash_read8(void *addr)
{
	return __raw_readb(addr);
}

__maybe_weak u16 flash_read16(void *addr)
{
	return __raw_readw(addr);
}

__maybe_weak u32 flash_read32(void *addr)
{
	return __raw_readl(addr);
}

__maybe_weak u64 flash_read64(void *addr)
{
	/* No architectures currently implement __raw_readq() */
	return *(volatile u64 *)addr;
}

/*-----------------------------------------------------------------------
 */
#if defined(CONFIG_ENV_IS_IN_FLASH) || defined(CONFIG_ENV_ADDR_REDUND) || \
	(CONFIG_SYS_MONITOR_BASE >= CONFIG_SYS_FLASH_BASE)
static flash_info_t *flash_get_info(ulong base)
{
	int i;
	flash_info_t *info;

	for (i = 0; i < CONFIG_SYS_MAX_FLASH_BANKS; i++) {
		info = &flash_info[i];
		if (info->size && info->start[0] <= base &&
		    base <= info->start[0] + info->size - 1)
			return info;
	}

	return NULL;
}
#endif

unsigned long flash_sector_size(flash_info_t *info, flash_sect_t sect)
{
	if (sect != (info->sector_count - 1))
		return info->start[sect + 1] - info->start[sect];
	else
		return info->start[0] + info->size - info->start[sect];
}

/*-----------------------------------------------------------------------
 * create an address based on the offset and the port width
 */
static inline void *
flash_map(flash_info_t *info, flash_sect_t sect, uint offset)
{
	unsigned int byte_offset = offset * info->portwidth;

	return (void *)(info->start[sect] + byte_offset);
}

static inline void flash_unmap(flash_info_t *info, flash_sect_t sect,
			       unsigned int offset, void *addr)
{
}

/*-----------------------------------------------------------------------
 * make a proper sized command based on the port and chip widths
 */
static void flash_make_cmd(flash_info_t *info, u32 cmd, void *cmdbuf)
{
	int i;
	int cword_offset;
	int cp_offset;
#if defined(__LITTLE_ENDIAN) || defined(CONFIG_SYS_WRITE_SWAPPED_DATA)
	u32 cmd_le = cpu_to_le32(cmd);
#endif
	uchar val;
	uchar *cp = (uchar *) cmdbuf;

	for (i = info->portwidth; i > 0; i--) {
		cword_offset = (info->portwidth - i) % info->chipwidth;
#if defined(__LITTLE_ENDIAN) || defined(CONFIG_SYS_WRITE_SWAPPED_DATA)
		cp_offset = info->portwidth - i;
		val = *((uchar *)&cmd_le + cword_offset);
#else
		cp_offset = i - 1;
		val = *((uchar *)&cmd + sizeof(u32) - cword_offset - 1);
#endif
		cp[cp_offset] = (cword_offset >= sizeof(u32)) ? 0x00 : val;
	}
}

#ifdef DEBUG
/*-----------------------------------------------------------------------
 * Debug support
 */
static void print_longlong(char *str, unsigned long long data)
{
	int i;
	char *cp;

	cp = (char *)&data;
	for (i = 0; i < 8; i++)
		sprintf(&str[i * 2], "%2.2x", *cp++);
}

static void flash_printqry(struct cfi_qry *qry)
{
	u8 *p = (u8 *)qry;
	int x, y;

	for (x = 0; x < sizeof(struct cfi_qry); x += 16) {
		debug("%02x : ", x);
		for (y = 0; y < 16; y++)
			debug("%2.2x ", p[x + y]);
		debug(" ");
		for (y = 0; y < 16; y++) {
			unsigned char c = p[x + y];

			if (c >= 0x20 && c <= 0x7e)
				debug("%c", c);
			else
				debug(".");
		}
		debug("\n");
	}
}
#endif

/*-----------------------------------------------------------------------
 * read a character at a port width address
 */
static inline uchar flash_read_uchar(flash_info_t *info, uint offset)
{
	uchar *cp;
	uchar retval;

	cp = flash_map(info, 0, offset);
#if defined(__LITTLE_ENDIAN) || defined(CONFIG_SYS_WRITE_SWAPPED_DATA)
	retval = flash_read8(cp);
#else
	retval = flash_read8(cp + info->portwidth - 1);
#endif
	flash_unmap(info, 0, offset, cp);
	return retval;
}

/*-----------------------------------------------------------------------
 * read a word at a port width address, assume 16bit bus
 */
static inline ushort flash_read_word(flash_info_t *info, uint offset)
{
	ushort *addr, retval;

	addr = flash_map(info, 0, offset);
	retval = flash_read16(addr);
	flash_unmap(info, 0, offset, addr);
	return retval;
}

/*-----------------------------------------------------------------------
 * read a long word by picking the least significant byte of each maximum
 * port size word. Swap for ppc format.
 */
static ulong flash_read_long (flash_info_t *info, flash_sect_t sect,
			      uint offset)
{
	uchar *addr;
	ulong retval;

#ifdef DEBUG
	int x;
#endif
	addr = flash_map(info, sect, offset);

#ifdef DEBUG
	debug("long addr is at %p info->portwidth = %d\n", addr,
	      info->portwidth);
	for (x = 0; x < 4 * info->portwidth; x++)
		debug("addr[%x] = 0x%x\n", x, flash_read8(addr + x));
#endif
#if defined(__LITTLE_ENDIAN) || defined(CONFIG_SYS_WRITE_SWAPPED_DATA)
	retval = ((flash_read8(addr) << 16) |
		  (flash_read8(addr + info->portwidth) << 24) |
		  (flash_read8(addr + 2 * info->portwidth)) |
		  (flash_read8(addr + 3 * info->portwidth) << 8));
#else
	retval = ((flash_read8(addr + 2 * info->portwidth - 1) << 24) |
		  (flash_read8(addr + info->portwidth - 1) << 16) |
		  (flash_read8(addr + 4 * info->portwidth - 1) << 8) |
		  (flash_read8(addr + 3 * info->portwidth - 1)));
#endif
	flash_unmap(info, sect, offset, addr);

	return retval;
}

/*
 * Write a proper sized command to the correct address
 */
static void flash_write_cmd(flash_info_t *info, flash_sect_t sect,
			    uint offset, u32 cmd)
{
	void *addr;
	cfiword_t cword;

	addr = flash_map(info, sect, offset);
	flash_make_cmd(info, cmd, &cword);
	switch (info->portwidth) {
	case FLASH_CFI_8BIT:
		debug("fwc addr %p cmd %x %x 8bit x %d bit\n", addr, cmd,
		      cword.w8, info->chipwidth << CFI_FLASH_SHIFT_WIDTH);
		flash_write8(cword.w8, addr);
		break;
	case FLASH_CFI_16BIT:
		debug("fwc addr %p cmd %x %4.4x 16bit x %d bit\n", addr,
		      cmd, cword.w16,
		      info->chipwidth << CFI_FLASH_SHIFT_WIDTH);
		flash_write16(cword.w16, addr);
		break;
	case FLASH_CFI_32BIT:
		debug("fwc addr %p cmd %x %8.8x 32bit x %d bit\n", addr,
		      cmd, cword.w32,
		      info->chipwidth << CFI_FLASH_SHIFT_WIDTH);
		flash_write32(cword.w32, addr);
		break;
	case FLASH_CFI_64BIT:
#ifdef DEBUG
		{
			char str[20];

			print_longlong(str, cword.w64);

			debug("fwrite addr %p cmd %x %s 64 bit x %d bit\n",
			      addr, cmd, str,
			      info->chipwidth << CFI_FLASH_SHIFT_WIDTH);
		}
#endif
		flash_write64(cword.w64, addr);
		break;
	}

	/* Ensure all the instructions are fully finished */
	sync();

	flash_unmap(info, sect, offset, addr);
}

static void flash_unlock_seq(flash_info_t *info, flash_sect_t sect)
{
	flash_write_cmd(info, sect, info->addr_unlock1, AMD_CMD_UNLOCK_START);
	flash_write_cmd(info, sect, info->addr_unlock2, AMD_CMD_UNLOCK_ACK);
}

/*-----------------------------------------------------------------------
 */
static int flash_isequal(flash_info_t *info, flash_sect_t sect, uint offset,
			 uchar cmd)
{
	void *addr;
	cfiword_t cword;
	int retval;

	addr = flash_map(info, sect, offset);
	flash_make_cmd(info, cmd, &cword);

	debug("is= cmd %x(%c) addr %p ", cmd, cmd, addr);
	switch (info->portwidth) {
	case FLASH_CFI_8BIT:
		debug("is= %x %x\n", flash_read8(addr), cword.w8);
		retval = (flash_read8(addr) == cword.w8);
		break;
	case FLASH_CFI_16BIT:
		debug("is= %4.4x %4.4x\n", flash_read16(addr), cword.w16);
		retval = (flash_read16(addr) == cword.w16);
		break;
	case FLASH_CFI_32BIT:
		debug("is= %8.8x %8.8x\n", flash_read32(addr), cword.w32);
		retval = (flash_read32(addr) == cword.w32);
		break;
	case FLASH_CFI_64BIT:
#ifdef DEBUG
		{
			char str1[20];
			char str2[20];

			print_longlong(str1, flash_read64(addr));
			print_longlong(str2, cword.w64);
			debug("is= %s %s\n", str1, str2);
		}
#endif
		retval = (flash_read64(addr) == cword.w64);
		break;
	default:
		retval = 0;
		break;
	}
	flash_unmap(info, sect, offset, addr);

	return retval;
}

/*-----------------------------------------------------------------------
 */
static int flash_isset(flash_info_t *info, flash_sect_t sect, uint offset,
		       uchar cmd)
{
	void *addr;
	cfiword_t cword;
	int retval;

	addr = flash_map(info, sect, offset);
	flash_make_cmd(info, cmd, &cword);
	switch (info->portwidth) {
	case FLASH_CFI_8BIT:
		retval = ((flash_read8(addr) & cword.w8) == cword.w8);
		break;
	case FLASH_CFI_16BIT:
		retval = ((flash_read16(addr) & cword.w16) == cword.w16);
		break;
	case FLASH_CFI_32BIT:
		retval = ((flash_read32(addr) & cword.w32) == cword.w32);
		break;
	case FLASH_CFI_64BIT:
		retval = ((flash_read64(addr) & cword.w64) == cword.w64);
		break;
	default:
		retval = 0;
		break;
	}
	flash_unmap(info, sect, offset, addr);

	return retval;
}

/*-----------------------------------------------------------------------
 */
static int flash_toggle(flash_info_t *info, flash_sect_t sect, uint offset,
			uchar cmd)
{
	u8 *addr;
	cfiword_t cword;
	int retval;

	addr = flash_map(info, sect, offset);
	flash_make_cmd(info, cmd, &cword);
	switch (info->portwidth) {
	case FLASH_CFI_8BIT:
		retval = flash_read8(addr) != flash_read8(addr);
		break;
	case FLASH_CFI_16BIT:
		retval = flash_read16(addr) != flash_read16(addr);
		break;
	case FLASH_CFI_32BIT:
		retval = flash_read32(addr) != flash_read32(addr);
		break;
	case FLASH_CFI_64BIT:
		retval = ((flash_read32(addr) != flash_read32(addr)) ||
			   (flash_read32(addr + 4) != flash_read32(addr + 4)));
		break;
	default:
		retval = 0;
		break;
	}
	flash_unmap(info, sect, offset, addr);

	return retval;
}

/*
 * flash_is_busy - check to see if the flash is busy
 *
 * This routine checks the status of the chip and returns true if the
 * chip is busy.
 */
static int flash_is_busy(flash_info_t *info, flash_sect_t sect)
{
	int retval;

	switch (info->vendor) {
	case CFI_CMDSET_INTEL_PROG_REGIONS:
	case CFI_CMDSET_INTEL_STANDARD:
	case CFI_CMDSET_INTEL_EXTENDED:
		retval = !flash_isset(info, sect, 0, FLASH_STATUS_DONE);
		break;
	case CFI_CMDSET_AMD_STANDARD:
	case CFI_CMDSET_AMD_EXTENDED:
#ifdef CONFIG_FLASH_CFI_LEGACY
	case CFI_CMDSET_AMD_LEGACY:
#endif
		if (info->sr_supported) {
			flash_write_cmd(info, sect, info->addr_unlock1,
					FLASH_CMD_READ_STATUS);
			retval = !flash_isset(info, sect, 0,
					      FLASH_STATUS_DONE);
		} else {
			retval = flash_toggle(info, sect, 0,
					      AMD_STATUS_TOGGLE);
		}

		break;
	default:
		retval = 0;
	}
	debug("%s: %d\n", __func__, retval);
	return retval;
}

/*-----------------------------------------------------------------------
 *  wait for XSR.7 to be set. Time out with an error if it does not.
 *  This routine does not set the flash to read-array mode.
 */
static int flash_status_check(flash_info_t *info, flash_sect_t sector,
			      ulong tout, char *prompt)
{
	ulong start;

#if CONFIG_SYS_HZ != 1000
	/* Avoid overflow for large HZ */
	if ((ulong)CONFIG_SYS_HZ > 100000)
		tout *= (ulong)CONFIG_SYS_HZ / 1000;
	else
		tout = DIV_ROUND_UP(tout * (ulong)CONFIG_SYS_HZ, 1000);
#endif

	/* Wait for command completion */
#ifdef CONFIG_SYS_LOW_RES_TIMER
	reset_timer();
#endif
	start = get_timer(0);
	WATCHDOG_RESET();
	while (flash_is_busy(info, sector)) {
		if (get_timer(start) > tout) {
			printf("Flash %s timeout at address %lx data %lx\n",
			       prompt, info->start[sector],
			       flash_read_long(info, sector, 0));
			flash_write_cmd(info, sector, 0, info->cmd_reset);
			udelay(1);
			return ERR_TIMEOUT;
		}
		udelay(1);		/* also triggers watchdog */
	}
	return ERR_OK;
}

/*-----------------------------------------------------------------------
 * Wait for XSR.7 to be set, if it times out print an error, otherwise
 * do a full status check.
 *
 * This routine sets the flash to read-array mode.
 */
static int flash_full_status_check(flash_info_t *info, flash_sect_t sector,
				   ulong tout, char *prompt)
{
	int retcode;

	retcode = flash_status_check(info, sector, tout, prompt);
	switch (info->vendor) {
	case CFI_CMDSET_INTEL_PROG_REGIONS:
	case CFI_CMDSET_INTEL_EXTENDED:
	case CFI_CMDSET_INTEL_STANDARD:
		if (retcode == ERR_OK &&
		    !flash_isset(info, sector, 0, FLASH_STATUS_DONE)) {
			retcode = ERR_INVAL;
			printf("Flash %s error at address %lx\n", prompt,
			       info->start[sector]);
			if (flash_isset(info, sector, 0, FLASH_STATUS_ECLBS |
					 FLASH_STATUS_PSLBS)) {
				puts("Command Sequence Error.\n");
			} else if (flash_isset(info, sector, 0,
						FLASH_STATUS_ECLBS)) {
				puts("Block Erase Error.\n");
				retcode = ERR_NOT_ERASED;
			} else if (flash_isset(info, sector, 0,
						FLASH_STATUS_PSLBS)) {
				puts("Locking Error\n");
			}
			if (flash_isset(info, sector, 0, FLASH_STATUS_DPS)) {
				puts("Block locked.\n");
				retcode = ERR_PROTECTED;
			}
			if (flash_isset(info, sector, 0, FLASH_STATUS_VPENS))
				puts("Vpp Low Error.\n");
		}
		flash_write_cmd(info, sector, 0, info->cmd_reset);
		udelay(1);
		break;
	default:
		break;
	}
	return retcode;
}

static int use_flash_status_poll(flash_info_t *info)
{
#ifdef CONFIG_SYS_CFI_FLASH_STATUS_POLL
	if (info->vendor == CFI_CMDSET_AMD_EXTENDED ||
	    info->vendor == CFI_CMDSET_AMD_STANDARD)
		return 1;
#endif
	return 0;
}

static int flash_status_poll(flash_info_t *info, void *src, void *dst,
			     ulong tout, char *prompt)
{
#ifdef CONFIG_SYS_CFI_FLASH_STATUS_POLL
	ulong start;
	int ready;

#if CONFIG_SYS_HZ != 1000
	/* Avoid overflow for large HZ */
	if ((ulong)CONFIG_SYS_HZ > 100000)
		tout *= (ulong)CONFIG_SYS_HZ / 1000;
	else
		tout = DIV_ROUND_UP(tout * (ulong)CONFIG_SYS_HZ, 1000);
#endif

	/* Wait for command completion */
#ifdef CONFIG_SYS_LOW_RES_TIMER
	reset_timer();
#endif
	start = get_timer(0);
	WATCHDOG_RESET();
	while (1) {
		switch (info->portwidth) {
		case FLASH_CFI_8BIT:
			ready = flash_read8(dst) == flash_read8(src);
			break;
		case FLASH_CFI_16BIT:
			ready = flash_read16(dst) == flash_read16(src);
			break;
		case FLASH_CFI_32BIT:
			ready = flash_read32(dst) == flash_read32(src);
			break;
		case FLASH_CFI_64BIT:
			ready = flash_read64(dst) == flash_read64(src);
			break;
		default:
			ready = 0;
			break;
		}
		if (ready)
			break;
		if (get_timer(start) > tout) {
			printf("Flash %s timeout at address %lx data %lx\n",
			       prompt, (ulong)dst, (ulong)flash_read8(dst));
			return ERR_TIMEOUT;
		}
		udelay(1);		/* also triggers watchdog */
	}
#endif /* CONFIG_SYS_CFI_FLASH_STATUS_POLL */
	return ERR_OK;
}

/*-----------------------------------------------------------------------
 */
static void flash_add_byte(flash_info_t *info, cfiword_t *cword, uchar c)
{
#if defined(__LITTLE_ENDIAN) && !defined(CONFIG_SYS_WRITE_SWAPPED_DATA)
	unsigned short	w;
	unsigned int	l;
	unsigned long long ll;
#endif

	switch (info->portwidth) {
	case FLASH_CFI_8BIT:
		cword->w8 = c;
		break;
	case FLASH_CFI_16BIT:
#if defined(__LITTLE_ENDIAN) && !defined(CONFIG_SYS_WRITE_SWAPPED_DATA)
		w = c;
		w <<= 8;
		cword->w16 = (cword->w16 >> 8) | w;
#else
		cword->w16 = (cword->w16 << 8) | c;
#endif
		break;
	case FLASH_CFI_32BIT:
#if defined(__LITTLE_ENDIAN) && !defined(CONFIG_SYS_WRITE_SWAPPED_DATA)
		l = c;
		l <<= 24;
		cword->w32 = (cword->w32 >> 8) | l;
#else
		cword->w32 = (cword->w32 << 8) | c;
#endif
		break;
	case FLASH_CFI_64BIT:
#if defined(__LITTLE_ENDIAN) && !defined(CONFIG_SYS_WRITE_SWAPPED_DATA)
		ll = c;
		ll <<= 56;
		cword->w64 = (cword->w64 >> 8) | ll;
#else
		cword->w64 = (cword->w64 << 8) | c;
#endif
		break;
	}
}

/*
 * Loop through the sector table starting from the previously found sector.
 * Searches forwards or backwards, dependent on the passed address.
 */
static flash_sect_t find_sector(flash_info_t *info, ulong addr)
{
	static flash_sect_t saved_sector; /* previously found sector */
	static flash_info_t *saved_info; /* previously used flash bank */
	flash_sect_t sector = saved_sector;

	if (info != saved_info || sector >= info->sector_count)
		sector = 0;

	while ((sector < info->sector_count - 1) &&
	       (info->start[sector] < addr))
		sector++;
	while ((info->start[sector] > addr) && (sector > 0))
		/*
		 * also decrements the sector in case of an overshot
		 * in the first loop
		 */
		sector--;

	saved_sector = sector;
	saved_info = info;
	return sector;
}

/*-----------------------------------------------------------------------
 */
static int flash_write_cfiword(flash_info_t *info, ulong dest, cfiword_t cword)
{
	void *dstaddr = (void *)dest;
	int flag;
	flash_sect_t sect = 0;
	char sect_found = 0;

	/* Check if Flash is (sufficiently) erased */
	switch (info->portwidth) {
	case FLASH_CFI_8BIT:
		flag = ((flash_read8(dstaddr) & cword.w8) == cword.w8);
		break;
	case FLASH_CFI_16BIT:
		flag = ((flash_read16(dstaddr) & cword.w16) == cword.w16);
		break;
	case FLASH_CFI_32BIT:
		flag = ((flash_read32(dstaddr) & cword.w32) == cword.w32);
		break;
	case FLASH_CFI_64BIT:
		flag = ((flash_read64(dstaddr) & cword.w64) == cword.w64);
		break;
	default:
		flag = 0;
		break;
	}
	if (!flag)
		return ERR_NOT_ERASED;

	/* Disable interrupts which might cause a timeout here */
	flag = disable_interrupts();

	switch (info->vendor) {
	case CFI_CMDSET_INTEL_PROG_REGIONS:
	case CFI_CMDSET_INTEL_EXTENDED:
	case CFI_CMDSET_INTEL_STANDARD:
		flash_write_cmd(info, 0, 0, FLASH_CMD_CLEAR_STATUS);
		flash_write_cmd(info, 0, 0, FLASH_CMD_WRITE);
		break;
	case CFI_CMDSET_AMD_EXTENDED:
	case CFI_CMDSET_AMD_STANDARD:
		sect = find_sector(info, dest);
		flash_unlock_seq(info, sect);
		flash_write_cmd(info, sect, info->addr_unlock1, AMD_CMD_WRITE);
		sect_found = 1;
		break;
#ifdef CONFIG_FLASH_CFI_LEGACY
	case CFI_CMDSET_AMD_LEGACY:
		sect = find_sector(info, dest);
		flash_unlock_seq(info, 0);
		flash_write_cmd(info, 0, info->addr_unlock1, AMD_CMD_WRITE);
		sect_found = 1;
		break;
#endif
	}

	switch (info->portwidth) {
	case FLASH_CFI_8BIT:
		flash_write8(cword.w8, dstaddr);
		break;
	case FLASH_CFI_16BIT:
		flash_write16(cword.w16, dstaddr);
		break;
	case FLASH_CFI_32BIT:
		flash_write32(cword.w32, dstaddr);
		break;
	case FLASH_CFI_64BIT:
		flash_write64(cword.w64, dstaddr);
		break;
	}

	/* re-enable interrupts if necessary */
	if (flag)
		enable_interrupts();

	if (!sect_found)
		sect = find_sector(info, dest);

	if (use_flash_status_poll(info))
		return flash_status_poll(info, &cword, dstaddr,
					 info->write_tout, "write");
	else
		return flash_full_status_check(info, sect,
					       info->write_tout, "write");
}

#ifdef CONFIG_SYS_FLASH_USE_BUFFER_WRITE

static int flash_write_cfibuffer(flash_info_t *info, ulong dest, uchar *cp,
				 int len)
{
	flash_sect_t sector;
	int cnt;
	int retcode;
	u8 *src = cp;
	u8 *dst = (u8 *)dest;
	u8 *dst2 = dst;
	int flag = 1;
	uint offset = 0;
	unsigned int shift;
	uchar write_cmd;

	switch (info->portwidth) {
	case FLASH_CFI_8BIT:
		shift = 0;
		break;
	case FLASH_CFI_16BIT:
		shift = 1;
		break;
	case FLASH_CFI_32BIT:
		shift = 2;
		break;
	case FLASH_CFI_64BIT:
		shift = 3;
		break;
	default:
		retcode = ERR_INVAL;
		goto out_unmap;
	}

	cnt = len >> shift;

	while ((cnt-- > 0) && (flag == 1)) {
		switch (info->portwidth) {
		case FLASH_CFI_8BIT:
			flag = ((flash_read8(dst2) & flash_read8(src)) ==
				flash_read8(src));
			src += 1, dst2 += 1;
			break;
		case FLASH_CFI_16BIT:
			flag = ((flash_read16(dst2) & flash_read16(src)) ==
				flash_read16(src));
			src += 2, dst2 += 2;
			break;
		case FLASH_CFI_32BIT:
			flag = ((flash_read32(dst2) & flash_read32(src)) ==
				flash_read32(src));
			src += 4, dst2 += 4;
			break;
		case FLASH_CFI_64BIT:
			flag = ((flash_read64(dst2) & flash_read64(src)) ==
				flash_read64(src));
			src += 8, dst2 += 8;
			break;
		}
	}
	if (!flag) {
		retcode = ERR_NOT_ERASED;
		goto out_unmap;
	}

	src = cp;
	sector = find_sector(info, dest);

	switch (info->vendor) {
	case CFI_CMDSET_INTEL_PROG_REGIONS:
	case CFI_CMDSET_INTEL_STANDARD:
	case CFI_CMDSET_INTEL_EXTENDED:
		write_cmd = (info->vendor == CFI_CMDSET_INTEL_PROG_REGIONS) ?
			    FLASH_CMD_WRITE_BUFFER_PROG :
			    FLASH_CMD_WRITE_TO_BUFFER;
		flash_write_cmd(info, sector, 0, FLASH_CMD_CLEAR_STATUS);
		flash_write_cmd(info, sector, 0, FLASH_CMD_READ_STATUS);
		flash_write_cmd(info, sector, 0, write_cmd);
		retcode = flash_status_check(info, sector,
					     info->buffer_write_tout,
					     "write to buffer");
		if (retcode == ERR_OK) {
			/* reduce the number of loops by the width of
			 * the port
			 */
			cnt = len >> shift;
			flash_write_cmd(info, sector, 0, cnt - 1);
			while (cnt-- > 0) {
				switch (info->portwidth) {
				case FLASH_CFI_8BIT:
					flash_write8(flash_read8(src), dst);
					src += 1, dst += 1;
					break;
				case FLASH_CFI_16BIT:
					flash_write16(flash_read16(src), dst);
					src += 2, dst += 2;
					break;
				case FLASH_CFI_32BIT:
					flash_write32(flash_read32(src), dst);
					src += 4, dst += 4;
					break;
				case FLASH_CFI_64BIT:
					flash_write64(flash_read64(src), dst);
					src += 8, dst += 8;
					break;
				default:
					retcode = ERR_INVAL;
					goto out_unmap;
				}
			}
			flash_write_cmd(info, sector, 0,
					FLASH_CMD_WRITE_BUFFER_CONFIRM);
			retcode = flash_full_status_check(
				info, sector, info->buffer_write_tout,
				"buffer write");
		}

		break;

	case CFI_CMDSET_AMD_STANDARD:
	case CFI_CMDSET_AMD_EXTENDED:
		flash_unlock_seq(info, sector);

#ifdef CONFIG_FLASH_SPANSION_S29WS_N
		offset = ((unsigned long)dst - info->start[sector]) >> shift;
#endif
		flash_write_cmd(info, sector, offset, AMD_CMD_WRITE_TO_BUFFER);
		cnt = len >> shift;
		flash_write_cmd(info, sector, offset, cnt - 1);

		switch (info->portwidth) {
		case FLASH_CFI_8BIT:
			while (cnt-- > 0) {
				flash_write8(flash_read8(src), dst);
				src += 1, dst += 1;
			}
			break;
		case FLASH_CFI_16BIT:
			while (cnt-- > 0) {
				flash_write16(flash_read16(src), dst);
				src += 2, dst += 2;
			}
			break;
		case FLASH_CFI_32BIT:
			while (cnt-- > 0) {
				flash_write32(flash_read32(src), dst);
				src += 4, dst += 4;
			}
			break;
		case FLASH_CFI_64BIT:
			while (cnt-- > 0) {
				flash_write64(flash_read64(src), dst);
				src += 8, dst += 8;
			}
			break;
		default:
			retcode = ERR_INVAL;
			goto out_unmap;
		}

		flash_write_cmd(info, sector, 0, AMD_CMD_WRITE_BUFFER_CONFIRM);
		if (use_flash_status_poll(info))
			retcode = flash_status_poll(info, src - (1 << shift),
						    dst - (1 << shift),
						    info->buffer_write_tout,
						    "buffer write");
		else
			retcode = flash_full_status_check(info, sector,
							  info->buffer_write_tout,
							  "buffer write");
		break;

	default:
		debug("Unknown Command Set\n");
		retcode = ERR_INVAL;
		break;
	}

out_unmap:
	return retcode;
}
#endif /* CONFIG_SYS_FLASH_USE_BUFFER_WRITE */

/*-----------------------------------------------------------------------
 */
int flash_erase(flash_info_t *info, int s_first, int s_last)
{
	int rcode = 0;
	int prot;
	flash_sect_t sect;
	int st;

	if (info->flash_id != FLASH_MAN_CFI) {
		puts("Can't erase unknown flash type - aborted\n");
		return 1;
	}
	if (s_first < 0 || s_first > s_last) {
		puts("- no sectors to erase\n");
		return 1;
	}

	prot = 0;
	for (sect = s_first; sect <= s_last; ++sect)
		if (info->protect[sect])
			prot++;
	if (prot) {
		printf("- Warning: %d protected sectors will not be erased!\n",
		       prot);
	} else if (flash_verbose) {
		putc('\n');
	}

	for (sect = s_first; sect <= s_last; sect++) {
		if (ctrlc()) {
			printf("\n");
			return 1;
		}

		if (info->protect[sect] == 0) { /* not protected */
#ifdef CONFIG_SYS_FLASH_CHECK_BLANK_BEFORE_ERASE
			int k;
			int size;
			int erased;
			u32 *flash;

			/*
			 * Check if whole sector is erased
			 */
			size = flash_sector_size(info, sect);
			erased = 1;
			flash = (u32 *)info->start[sect];
			/* divide by 4 for longword access */
			size = size >> 2;
			for (k = 0; k < size; k++) {
				if (flash_read32(flash++) != 0xffffffff) {
					erased = 0;
					break;
				}
			}
			if (erased) {
				if (flash_verbose)
					putc(',');
				continue;
			}
#endif
			switch (info->vendor) {
			case CFI_CMDSET_INTEL_PROG_REGIONS:
			case CFI_CMDSET_INTEL_STANDARD:
			case CFI_CMDSET_INTEL_EXTENDED:
				flash_write_cmd(info, sect, 0,
						FLASH_CMD_CLEAR_STATUS);
				flash_write_cmd(info, sect, 0,
						FLASH_CMD_BLOCK_ERASE);
				flash_write_cmd(info, sect, 0,
						FLASH_CMD_ERASE_CONFIRM);
				break;
			case CFI_CMDSET_AMD_STANDARD:
			case CFI_CMDSET_AMD_EXTENDED:
				flash_unlock_seq(info, sect);
				flash_write_cmd(info, sect,
						info->addr_unlock1,
						AMD_CMD_ERASE_START);
				flash_unlock_seq(info, sect);
				flash_write_cmd(info, sect, 0,
						info->cmd_erase_sector);
				break;
#ifdef CONFIG_FLASH_CFI_LEGACY
			case CFI_CMDSET_AMD_LEGACY:
				flash_unlock_seq(info, 0);
				flash_write_cmd(info, 0, info->addr_unlock1,
						AMD_CMD_ERASE_START);
				flash_unlock_seq(info, 0);
				flash_write_cmd(info, sect, 0,
						AMD_CMD_ERASE_SECTOR);
				break;
#endif
			default:
				debug("Unknown flash vendor %d\n",
				      info->vendor);
				break;
			}

			if (use_flash_status_poll(info)) {
				cfiword_t cword;
				void *dest;

				cword.w64 = 0xffffffffffffffffULL;
				dest = flash_map(info, sect, 0);
				st = flash_status_poll(info, &cword, dest,
						       info->erase_blk_tout,
						       "erase");
				flash_unmap(info, sect, 0, dest);
			} else {
				st = flash_full_status_check(info, sect,
							     info->erase_blk_tout,
							     "erase");
			}

			if (st)
				rcode = 1;
			else if (flash_verbose)
				putc('.');
		}
	}

	if (flash_verbose)
		puts(" done\n");

	return rcode;
}

#ifdef CONFIG_SYS_FLASH_EMPTY_INFO
static int sector_erased(flash_info_t *info, int i)
{
	int k;
	int size;
	u32 *flash;

	/*
	 * Check if whole sector is erased
	 */
	size = flash_sector_size(info, i);
	flash = (u32 *)info->start[i];
	/* divide by 4 for longword access */
	size = size >> 2;

	for (k = 0; k < size; k++) {
		if (flash_read32(flash++) != 0xffffffff)
			return 0;	/* not erased */
	}

	return 1;			/* erased */
}
#endif /* CONFIG_SYS_FLASH_EMPTY_INFO */

void flash_print_info(flash_info_t *info)
{
	int i;

	if (info->flash_id != FLASH_MAN_CFI) {
		puts("missing or unknown FLASH type\n");
		return;
	}

	printf("%s flash (%d x %d)",
	       info->name,
	       (info->portwidth << 3), (info->chipwidth << 3));
	if (info->size < 1024 * 1024)
		printf("  Size: %ld kB in %d Sectors\n",
		       info->size >> 10, info->sector_count);
	else
		printf("  Size: %ld MB in %d Sectors\n",
		       info->size >> 20, info->sector_count);
	printf("  ");
	switch (info->vendor) {
	case CFI_CMDSET_INTEL_PROG_REGIONS:
		printf("Intel Prog Regions");
		break;
	case CFI_CMDSET_INTEL_STANDARD:
		printf("Intel Standard");
		break;
	case CFI_CMDSET_INTEL_EXTENDED:
		printf("Intel Extended");
		break;
	case CFI_CMDSET_AMD_STANDARD:
		printf("AMD Standard");
		break;
	case CFI_CMDSET_AMD_EXTENDED:
		printf("AMD Extended");
		break;
#ifdef CONFIG_FLASH_CFI_LEGACY
	case CFI_CMDSET_AMD_LEGACY:
		printf("AMD Legacy");
		break;
#endif
	default:
		printf("Unknown (%d)", info->vendor);
		break;
	}
	printf(" command set, Manufacturer ID: 0x%02X, Device ID: 0x",
	       info->manufacturer_id);
	printf(info->chipwidth == FLASH_CFI_16BIT ? "%04X" : "%02X",
	       info->device_id);
	if ((info->device_id & 0xff) == 0x7E) {
		printf(info->chipwidth == FLASH_CFI_16BIT ? "%04X" : "%02X",
		       info->device_id2);
	}
	if (info->vendor == CFI_CMDSET_AMD_STANDARD && info->legacy_unlock)
		printf("\n  Advanced Sector Protection (PPB) enabled");
	printf("\n  Erase timeout: %ld ms, write timeout: %ld ms\n",
	       info->erase_blk_tout, info->write_tout);
	if (info->buffer_size > 1) {
		printf("  Buffer write timeout: %ld ms, ",
		       info->buffer_write_tout);
		printf("buffer size: %d bytes\n", info->buffer_size);
	}

	puts("\n  Sector Start Addresses:");
	for (i = 0; i < info->sector_count; ++i) {
		if (ctrlc())
			break;
		if ((i % 5) == 0)
			putc('\n');
#ifdef CONFIG_SYS_FLASH_EMPTY_INFO
		/* print empty and read-only info */
		printf("  %08lX %c %s ",
		       info->start[i],
		       sector_erased(info, i) ? 'E' : ' ',
		       info->protect[i] ? "RO" : "  ");
#else	/* ! CONFIG_SYS_FLASH_EMPTY_INFO */
		printf("  %08lX   %s ",
		       info->start[i],
		       info->protect[i] ? "RO" : "  ");
#endif
	}
	putc('\n');
}

/*-----------------------------------------------------------------------
 * This is used in a few places in write_buf() to show programming
 * progress.  Making it a function is nasty because it needs to do side
 * effect updates to digit and dots.  Repeated code is nasty too, so
 * we define it once here.
 */
#ifdef CONFIG_FLASH_SHOW_PROGRESS
#define FLASH_SHOW_PROGRESS(scale, dots, digit, dots_sub) \
	if (flash_verbose) { \
		dots -= dots_sub; \
		if (scale > 0 && dots <= 0) { \
			if ((digit % 5) == 0) \
				printf("%d", digit / 5); \
			else \
				putc('.'); \
			digit--; \
			dots += scale; \
		} \
	}
#else
#define FLASH_SHOW_PROGRESS(scale, dots, digit, dots_sub)
#endif

/*-----------------------------------------------------------------------
 * Copy memory to flash, returns:
 * 0 - OK
 * 1 - write timeout
 * 2 - Flash not erased
 */
int write_buff(flash_info_t *info, uchar *src, ulong addr, ulong cnt)
{
	ulong wp;
	uchar *p;
	int aln;
	cfiword_t cword;
	int i, rc;
#ifdef CONFIG_SYS_FLASH_USE_BUFFER_WRITE
	int buffered_size;
#endif
#ifdef CONFIG_FLASH_SHOW_PROGRESS
	int digit = CONFIG_FLASH_SHOW_PROGRESS;
	int scale = 0;
	int dots  = 0;

	/*
	 * Suppress if there are fewer than CONFIG_FLASH_SHOW_PROGRESS writes.
	 */
	if (cnt >= CONFIG_FLASH_SHOW_PROGRESS) {
		scale = (int)((cnt + CONFIG_FLASH_SHOW_PROGRESS - 1) /
			CONFIG_FLASH_SHOW_PROGRESS);
	}
#endif

	/* get lower aligned address */
	wp = (addr & ~(info->portwidth - 1));

	/* handle unaligned start */
	aln = addr - wp;
	if (aln != 0) {
		cword.w32 = 0;
		p = (uchar *)wp;
		for (i = 0; i < aln; ++i)
			flash_add_byte(info, &cword, flash_read8(p + i));

		for (; (i < info->portwidth) && (cnt > 0); i++) {
			flash_add_byte(info, &cword, *src++);
			cnt--;
		}
		for (; (cnt == 0) && (i < info->portwidth); ++i)
			flash_add_byte(info, &cword, flash_read8(p + i));

		rc = flash_write_cfiword(info, wp, cword);
		if (rc != 0)
			return rc;

		wp += i;
		FLASH_SHOW_PROGRESS(scale, dots, digit, i);
	}

	/* handle the aligned part */
#ifdef CONFIG_SYS_FLASH_USE_BUFFER_WRITE
	buffered_size = (info->portwidth / info->chipwidth);
	buffered_size *= info->buffer_size;
	while (cnt >= info->portwidth) {
		/* prohibit buffer write when buffer_size is 1 */
		if (info->buffer_size == 1) {
			cword.w32 = 0;
			for (i = 0; i < info->portwidth; i++)
				flash_add_byte(info, &cword, *src++);
			rc = flash_write_cfiword(info, wp, cword);
			if (rc != 0)
				return rc;
			wp += info->portwidth;
			cnt -= info->portwidth;
			continue;
		}

		/* write buffer until next buffered_size aligned boundary */
		i = buffered_size - (wp % buffered_size);
		if (i > cnt)
			i = cnt;
		rc = flash_write_cfibuffer(info, wp, src, i);
		if (rc != ERR_OK)
			return rc;
		i -= i & (info->portwidth - 1);
		wp += i;
		src += i;
		cnt -= i;
		FLASH_SHOW_PROGRESS(scale, dots, digit, i);
		/* Only check every once in a while */
		if ((cnt & 0xFFFF) < buffered_size && ctrlc())
			return ERR_ABORTED;
	}
#else
	while (cnt >= info->portwidth) {
		cword.w32 = 0;
		for (i = 0; i < info->portwidth; i++)
			flash_add_byte(info, &cword, *src++);
		rc = flash_write_cfiword(info, wp, cword);
		if (rc != 0)
			return rc;
		wp += info->portwidth;
		cnt -= info->portwidth;
		FLASH_SHOW_PROGRESS(scale, dots, digit, info->portwidth);
		/* Only check every once in a while */
		if ((cnt & 0xFFFF) < info->portwidth && ctrlc())
			return ERR_ABORTED;
	}
#endif /* CONFIG_SYS_FLASH_USE_BUFFER_WRITE */

	if (cnt == 0)
		return (0);

	/*
	 * handle unaligned tail bytes
	 */
	cword.w32 = 0;
	p = (uchar *)wp;
	for (i = 0; (i < info->portwidth) && (cnt > 0); ++i) {
		flash_add_byte(info, &cword, *src++);
		--cnt;
	}
	for (; i < info->portwidth; ++i)
		flash_add_byte(info, &cword, flash_read8(p + i));

	return flash_write_cfiword(info, wp, cword);
}

static inline int manufact_match(flash_info_t *info, u32 manu)
{
	return info->manufacturer_id == ((manu & FLASH_VENDMASK) >> 16);
}

/*-----------------------------------------------------------------------
 */
#ifdef CONFIG_SYS_FLASH_PROTECTION

static int cfi_protect_bugfix(flash_info_t *info, long sector, int prot)
{
	if (manufact_match(info, INTEL_MANUFACT) &&
	    info->device_id == NUMONYX_256MBIT) {
		/*
		 * see errata called
		 * "Numonyx Axcell P33/P30 Specification Update" :)
		 */
		flash_write_cmd(info, sector, 0, FLASH_CMD_READ_ID);
		if (!flash_isequal(info, sector, FLASH_OFFSET_PROTECT,
				   prot)) {
			/*
			 * cmd must come before FLASH_CMD_PROTECT + 20us
			 * Disable interrupts which might cause a timeout here.
			 */
			int flag = disable_interrupts();
			unsigned short cmd;

			if (prot)
				cmd = FLASH_CMD_PROTECT_SET;
			else
				cmd = FLASH_CMD_PROTECT_CLEAR;

			flash_write_cmd(info, sector, 0, FLASH_CMD_PROTECT);
			flash_write_cmd(info, sector, 0, cmd);
			/* re-enable interrupts if necessary */
			if (flag)
				enable_interrupts();
		}
		return 1;
	}
	return 0;
}

int flash_real_protect(flash_info_t *info, long sector, int prot)
{
	int retcode = 0;

	switch (info->vendor) {
	case CFI_CMDSET_INTEL_PROG_REGIONS:
	case CFI_CMDSET_INTEL_STANDARD:
	case CFI_CMDSET_INTEL_EXTENDED:
		if (!cfi_protect_bugfix(info, sector, prot)) {
			flash_write_cmd(info, sector, 0,
					FLASH_CMD_CLEAR_STATUS);
			flash_write_cmd(info, sector, 0,
					FLASH_CMD_PROTECT);
			if (prot)
				flash_write_cmd(info, sector, 0,
						FLASH_CMD_PROTECT_SET);
			else
				flash_write_cmd(info, sector, 0,
						FLASH_CMD_PROTECT_CLEAR);
		}
		break;
	case CFI_CMDSET_AMD_EXTENDED:
	case CFI_CMDSET_AMD_STANDARD:
		/* U-Boot only checks the first byte */
		if (manufact_match(info, ATM_MANUFACT)) {
			if (prot) {
				flash_unlock_seq(info, 0);
				flash_write_cmd(info, 0,
						info->addr_unlock1,
						ATM_CMD_SOFTLOCK_START);
				flash_unlock_seq(info, 0);
				flash_write_cmd(info, sector, 0,
						ATM_CMD_LOCK_SECT);
			} else {
				flash_write_cmd(info, 0,
						info->addr_unlock1,
						AMD_CMD_UNLOCK_START);
				if (info->device_id == ATM_ID_BV6416)
					flash_write_cmd(info, sector,
							0, ATM_CMD_UNLOCK_SECT);
			}
		}
		if (info->legacy_unlock) {
			int flag = disable_interrupts();
			int lock_flag;

			flash_unlock_seq(info, 0);
			flash_write_cmd(info, 0, info->addr_unlock1,
					AMD_CMD_SET_PPB_ENTRY);
			lock_flag = flash_isset(info, sector, 0, 0x01);
			if (prot) {
				if (lock_flag) {
					flash_write_cmd(info, sector, 0,
							AMD_CMD_PPB_LOCK_BC1);
					flash_write_cmd(info, sector, 0,
							AMD_CMD_PPB_LOCK_BC2);
				}
				debug("sector %ld %slocked\n", sector,
				      lock_flag ? "" : "already ");
			} else {
				if (!lock_flag) {
					debug("unlock %ld\n", sector);
					flash_write_cmd(info, 0, 0,
							AMD_CMD_PPB_UNLOCK_BC1);
					flash_write_cmd(info, 0, 0,
							AMD_CMD_PPB_UNLOCK_BC2);
				}
				debug("sector %ld %sunlocked\n", sector,
				      !lock_flag ? "" : "already ");
			}
			if (flag)
				enable_interrupts();

			if (flash_status_check(info, sector,
					       info->erase_blk_tout,
					       prot ? "protect" : "unprotect"))
				printf("status check error\n");

			flash_write_cmd(info, 0, 0,
					AMD_CMD_SET_PPB_EXIT_BC1);
			flash_write_cmd(info, 0, 0,
					AMD_CMD_SET_PPB_EXIT_BC2);
		}
		break;
#ifdef CONFIG_FLASH_CFI_LEGACY
	case CFI_CMDSET_AMD_LEGACY:
		flash_write_cmd(info, sector, 0, FLASH_CMD_CLEAR_STATUS);
		flash_write_cmd(info, sector, 0, FLASH_CMD_PROTECT);
		if (prot)
			flash_write_cmd(info, sector, 0,
					FLASH_CMD_PROTECT_SET);
		else
			flash_write_cmd(info, sector, 0,
					FLASH_CMD_PROTECT_CLEAR);
#endif
	};

	/*
	 * Flash needs to be in status register read mode for
	 * flash_full_status_check() to work correctly
	 */
	flash_write_cmd(info, sector, 0, FLASH_CMD_READ_STATUS);
	retcode = flash_full_status_check(info, sector, info->erase_blk_tout,
					  prot ? "protect" : "unprotect");
	if (retcode == 0) {
		info->protect[sector] = prot;

		/*
		 * On some of Intel's flash chips (marked via legacy_unlock)
		 * unprotect unprotects all locking.
		 */
		if (prot == 0 && info->legacy_unlock) {
			flash_sect_t i;

			for (i = 0; i < info->sector_count; i++) {
				if (info->protect[i])
					flash_real_protect(info, i, 1);
			}
		}
	}
	return retcode;
}

/*-----------------------------------------------------------------------
 * flash_read_user_serial - read the OneTimeProgramming cells
 */
void flash_read_user_serial(flash_info_t *info, void *buffer, int offset,
			    int len)
{
	uchar *src;
	uchar *dst;

	dst = buffer;
	src = flash_map(info, 0, FLASH_OFFSET_USER_PROTECTION);
	flash_write_cmd(info, 0, 0, FLASH_CMD_READ_ID);
	memcpy(dst, src + offset, len);
	flash_write_cmd(info, 0, 0, info->cmd_reset);
	udelay(1);
	flash_unmap(info, 0, FLASH_OFFSET_USER_PROTECTION, src);
}

/*
 * flash_read_factory_serial - read the device Id from the protection area
 */
void flash_read_factory_serial(flash_info_t *info, void *buffer, int offset,
			       int len)
{
	uchar *src;

	src = flash_map(info, 0, FLASH_OFFSET_INTEL_PROTECTION);
	flash_write_cmd(info, 0, 0, FLASH_CMD_READ_ID);
	memcpy(buffer, src + offset, len);
	flash_write_cmd(info, 0, 0, info->cmd_reset);
	udelay(1);
	flash_unmap(info, 0, FLASH_OFFSET_INTEL_PROTECTION, src);
}

#endif /* CONFIG_SYS_FLASH_PROTECTION */

/*-----------------------------------------------------------------------
 * Reverse the order of the erase regions in the CFI QRY structure.
 * This is needed for chips that are either a) correctly detected as
 * top-boot, or b) buggy.
 */
static void cfi_reverse_geometry(struct cfi_qry *qry)
{
	unsigned int i, j;
	u32 tmp;

	for (i = 0, j = qry->num_erase_regions - 1; i < j; i++, j--) {
		tmp = get_unaligned(&qry->erase_region_info[i]);
		put_unaligned(get_unaligned(&qry->erase_region_info[j]),
			      &qry->erase_region_info[i]);
		put_unaligned(tmp, &qry->erase_region_info[j]);
	}
}

/*-----------------------------------------------------------------------
 * read jedec ids from device and set corresponding fields in info struct
 *
 * Note: assume cfi->vendor, cfi->portwidth and cfi->chipwidth are correct
 *
 */
static void cmdset_intel_read_jedec_ids(flash_info_t *info)
{
	flash_write_cmd(info, 0, 0, FLASH_CMD_RESET);
	udelay(1);
	flash_write_cmd(info, 0, 0, FLASH_CMD_READ_ID);
	udelay(1000); /* some flash are slow to respond */
	info->manufacturer_id = flash_read_uchar(info,
						 FLASH_OFFSET_MANUFACTURER_ID);
	info->device_id = (info->chipwidth == FLASH_CFI_16BIT) ?
			flash_read_word(info, FLASH_OFFSET_DEVICE_ID) :
			flash_read_uchar(info, FLASH_OFFSET_DEVICE_ID);
	flash_write_cmd(info, 0, 0, FLASH_CMD_RESET);
}

static int cmdset_intel_init(flash_info_t *info, struct cfi_qry *qry)
{
	info->cmd_reset = FLASH_CMD_RESET;

	cmdset_intel_read_jedec_ids(info);
	flash_write_cmd(info, 0, info->cfi_offset, FLASH_CMD_CFI);

#ifdef CONFIG_SYS_FLASH_PROTECTION
	/* read legacy lock/unlock bit from intel flash */
	if (info->ext_addr) {
		info->legacy_unlock =
			flash_read_uchar(info, info->ext_addr + 5) & 0x08;
	}
#endif

	return 0;
}

static void cmdset_amd_read_jedec_ids(flash_info_t *info)
{
	ushort bank_id = 0;
	uchar  manu_id;
	uchar  feature;

	flash_write_cmd(info, 0, 0, AMD_CMD_RESET);
	flash_unlock_seq(info, 0);
	flash_write_cmd(info, 0, info->addr_unlock1, FLASH_CMD_READ_ID);
	udelay(1000); /* some flash are slow to respond */

	manu_id = flash_read_uchar(info, FLASH_OFFSET_MANUFACTURER_ID);
	/* JEDEC JEP106Z specifies ID codes up to bank 7 */
	while (manu_id == FLASH_CONTINUATION_CODE && bank_id < 0x800) {
		bank_id += 0x100;
		manu_id = flash_read_uchar(info,
					   bank_id | FLASH_OFFSET_MANUFACTURER_ID);
	}
	info->manufacturer_id = manu_id;

	debug("info->ext_addr = 0x%x, cfi_version = 0x%x\n",
	      info->ext_addr, info->cfi_version);
	if (info->ext_addr && info->cfi_version >= 0x3134) {
		/* read software feature (at 0x53) */
		feature = flash_read_uchar(info, info->ext_addr + 0x13);
		debug("feature = 0x%x\n", feature);
		info->sr_supported = feature & 0x1;
	}

	switch (info->chipwidth) {
	case FLASH_CFI_8BIT:
		info->device_id = flash_read_uchar(info,
						   FLASH_OFFSET_DEVICE_ID);
		if (info->device_id == 0x7E) {
			/* AMD 3-byte (expanded) device ids */
			info->device_id2 = flash_read_uchar(info,
							    FLASH_OFFSET_DEVICE_ID2);
			info->device_id2 <<= 8;
			info->device_id2 |= flash_read_uchar(info,
						FLASH_OFFSET_DEVICE_ID3);
		}
		break;
	case FLASH_CFI_16BIT:
		info->device_id = flash_read_word(info,
						  FLASH_OFFSET_DEVICE_ID);
		if ((info->device_id & 0xff) == 0x7E) {
			/* AMD 3-byte (expanded) device ids */
			info->device_id2 = flash_read_uchar(info,
							    FLASH_OFFSET_DEVICE_ID2);
			info->device_id2 <<= 8;
			info->device_id2 |= flash_read_uchar(info,
						FLASH_OFFSET_DEVICE_ID3);
		}
		break;
	default:
		break;
	}
	flash_write_cmd(info, 0, 0, AMD_CMD_RESET);
	udelay(1);
}

static int cmdset_amd_init(flash_info_t *info, struct cfi_qry *qry)
{
	info->cmd_reset = AMD_CMD_RESET;
	info->cmd_erase_sector = AMD_CMD_ERASE_SECTOR;

	cmdset_amd_read_jedec_ids(info);
	flash_write_cmd(info, 0, info->cfi_offset, FLASH_CMD_CFI);

#ifdef CONFIG_SYS_FLASH_PROTECTION
	if (info->ext_addr) {
		/* read sector protect/unprotect scheme (at 0x49) */
		if (flash_read_uchar(info, info->ext_addr + 9) == 0x8)
			info->legacy_unlock = 1;
	}
#endif

	return 0;
}

#ifdef CONFIG_FLASH_CFI_LEGACY
static void flash_read_jedec_ids(flash_info_t *info)
{
	info->manufacturer_id = 0;
	info->device_id       = 0;
	info->device_id2      = 0;

	switch (info->vendor) {
	case CFI_CMDSET_INTEL_PROG_REGIONS:
	case CFI_CMDSET_INTEL_STANDARD:
	case CFI_CMDSET_INTEL_EXTENDED:
		cmdset_intel_read_jedec_ids(info);
		break;
	case CFI_CMDSET_AMD_STANDARD:
	case CFI_CMDSET_AMD_EXTENDED:
		cmdset_amd_read_jedec_ids(info);
		break;
	default:
		break;
	}
}

/*-----------------------------------------------------------------------
 * Call board code to request info about non-CFI flash.
 * board_flash_get_legacy needs to fill in at least:
 * info->portwidth, info->chipwidth and info->interface for Jedec probing.
 */
static int flash_detect_legacy(phys_addr_t base, int banknum)
{
	flash_info_t *info = &flash_info[banknum];

	if (board_flash_get_legacy(base, banknum, info)) {
		/* board code may have filled info completely. If not, we
		 * use JEDEC ID probing.
		 */
		if (!info->vendor) {
			int modes[] = {
				CFI_CMDSET_AMD_STANDARD,
				CFI_CMDSET_INTEL_STANDARD
			};
			int i;

			for (i = 0; i < ARRAY_SIZE(modes); i++) {
				info->vendor = modes[i];
				info->start[0] =
					(ulong)map_physmem(base,
							   info->portwidth,
							   MAP_NOCACHE);
				if (info->portwidth == FLASH_CFI_8BIT &&
				    info->interface == FLASH_CFI_X8X16) {
					info->addr_unlock1 = 0x2AAA;
					info->addr_unlock2 = 0x5555;
				} else {
					info->addr_unlock1 = 0x5555;
					info->addr_unlock2 = 0x2AAA;
				}
				flash_read_jedec_ids(info);
				debug("JEDEC PROBE: ID %x %x %x\n",
				      info->manufacturer_id,
				      info->device_id,
				      info->device_id2);
				if (jedec_flash_match(info, info->start[0]))
					break;

				unmap_physmem((void *)info->start[0],
					      info->portwidth);
			}
		}

		switch (info->vendor) {
		case CFI_CMDSET_INTEL_PROG_REGIONS:
		case CFI_CMDSET_INTEL_STANDARD:
		case CFI_CMDSET_INTEL_EXTENDED:
			info->cmd_reset = FLASH_CMD_RESET;
			break;
		case CFI_CMDSET_AMD_STANDARD:
		case CFI_CMDSET_AMD_EXTENDED:
		case CFI_CMDSET_AMD_LEGACY:
			info->cmd_reset = AMD_CMD_RESET;
			break;
		}
		info->flash_id = FLASH_MAN_CFI;
		return 1;
	}
	return 0; /* use CFI */
}
#else
static inline int flash_detect_legacy(phys_addr_t base, int banknum)
{
	return 0; /* use CFI */
}
#endif

/*-----------------------------------------------------------------------
 * detect if flash is compatible with the Common Flash Interface (CFI)
 * http://www.jedec.org/download/search/jesd68.pdf
 */
static void flash_read_cfi(flash_info_t *info, void *buf, unsigned int start,
			   size_t len)
{
	u8 *p = buf;
	unsigned int i;

	for (i = 0; i < len; i++)
		p[i] = flash_read_uchar(info, start + i);
}

static void __flash_cmd_reset(flash_info_t *info)
{
	/*
	 * We do not yet know what kind of commandset to use, so we issue
	 * the reset command in both Intel and AMD variants, in the hope
	 * that AMD flash roms ignore the Intel command.
	 */
	flash_write_cmd(info, 0, 0, AMD_CMD_RESET);
	udelay(1);
	flash_write_cmd(info, 0, 0, FLASH_CMD_RESET);
}

void flash_cmd_reset(flash_info_t *info)
	__attribute__((weak, alias("__flash_cmd_reset")));

static int __flash_detect_cfi(flash_info_t *info, struct cfi_qry *qry)
{
	int cfi_offset;

	/* Issue FLASH reset command */
	flash_cmd_reset(info);

	for (cfi_offset = 0; cfi_offset < ARRAY_SIZE(flash_offset_cfi);
	     cfi_offset++) {
		flash_write_cmd(info, 0, flash_offset_cfi[cfi_offset],
				FLASH_CMD_CFI);
		if (flash_isequal(info, 0, FLASH_OFFSET_CFI_RESP, 'Q') &&
		    flash_isequal(info, 0, FLASH_OFFSET_CFI_RESP + 1, 'R') &&
		    flash_isequal(info, 0, FLASH_OFFSET_CFI_RESP + 2, 'Y')) {
			flash_read_cfi(info, qry, FLASH_OFFSET_CFI_RESP,
				       sizeof(struct cfi_qry));
			info->interface	= le16_to_cpu(qry->interface_desc);

			info->cfi_offset = flash_offset_cfi[cfi_offset];
			debug("device interface is %d\n",
			      info->interface);
			debug("found port %d chip %d ",
			      info->portwidth, info->chipwidth);
			debug("port %d bits chip %d bits\n",
			      info->portwidth << CFI_FLASH_SHIFT_WIDTH,
			      info->chipwidth << CFI_FLASH_SHIFT_WIDTH);

			/* calculate command offsets as in the Linux driver */
			info->addr_unlock1 = 0x555;
			info->addr_unlock2 = 0x2aa;

			/*
			 * modify the unlock address if we are
			 * in compatibility mode
			 */
			if (/* x8/x16 in x8 mode */
			    (info->chipwidth == FLASH_CFI_BY8 &&
				info->interface == FLASH_CFI_X8X16) ||
			    /* x16/x32 in x16 mode */
			    (info->chipwidth == FLASH_CFI_BY16 &&
				info->interface == FLASH_CFI_X16X32)) {
				info->addr_unlock1 = 0xaaa;
				info->addr_unlock2 = 0x555;
			}

			info->name = "CFI conformant";
			return 1;
		}
	}

	return 0;
}

static int flash_detect_cfi(flash_info_t *info, struct cfi_qry *qry)
{
	debug("flash detect cfi\n");

	for (info->portwidth = CONFIG_SYS_FLASH_CFI_WIDTH;
	     info->portwidth <= FLASH_CFI_64BIT; info->portwidth <<= 1) {
		for (info->chipwidth = FLASH_CFI_BY8;
		     info->chipwidth <= info->portwidth;
		     info->chipwidth <<= 1)
			if (__flash_detect_cfi(info, qry))
				return 1;
	}
	debug("not found\n");
	return 0;
}

/*
 * Manufacturer-specific quirks. Add workarounds for geometry
 * reversal, etc. here.
 */
static void flash_fixup_amd(flash_info_t *info, struct cfi_qry *qry)
{
	/* check if flash geometry needs reversal */
	if (qry->num_erase_regions > 1) {
		/* reverse geometry if top boot part */
		if (info->cfi_version < 0x3131) {
			/* CFI < 1.1, try to guess from device id */
			if ((info->device_id & 0x80) != 0)
				cfi_reverse_geometry(qry);
		} else if (flash_read_uchar(info, info->ext_addr + 0xf) == 3) {
			/* CFI >= 1.1, deduct from top/bottom flag */
			/* note: ext_addr is valid since cfi_version > 0 */
			cfi_reverse_geometry(qry);
		}
	}
}

static void flash_fixup_atmel(flash_info_t *info, struct cfi_qry *qry)
{
	int reverse_geometry = 0;

	/* Check the "top boot" bit in the PRI */
	if (info->ext_addr && !(flash_read_uchar(info, info->ext_addr + 6) & 1))
		reverse_geometry = 1;

	/* AT49BV6416(T) list the erase regions in the wrong order.
	 * However, the device ID is identical with the non-broken
	 * AT49BV642D they differ in the high byte.
	 */
	if (info->device_id == 0xd6 || info->device_id == 0xd2)
		reverse_geometry = !reverse_geometry;

	if (reverse_geometry)
		cfi_reverse_geometry(qry);
}

static void flash_fixup_stm(flash_info_t *info, struct cfi_qry *qry)
{
	/* check if flash geometry needs reversal */
	if (qry->num_erase_regions > 1) {
		/* reverse geometry if top boot part */
		if (info->cfi_version < 0x3131) {
			/* CFI < 1.1, guess by device id */
			if (info->device_id == 0x22CA || /* M29W320DT */
			    info->device_id == 0x2256 || /* M29W320ET */
			    info->device_id == 0x22D7) { /* M29W800DT */
				cfi_reverse_geometry(qry);
			}
		} else if (flash_read_uchar(info, info->ext_addr + 0xf) == 3) {
			/* CFI >= 1.1, deduct from top/bottom flag */
			/* note: ext_addr is valid since cfi_version > 0 */
			cfi_reverse_geometry(qry);
		}
	}
}

static void flash_fixup_sst(flash_info_t *info, struct cfi_qry *qry)
{
	/*
	 * SST, for many recent nor parallel flashes, says they are
	 * CFI-conformant. This is not true, since qry struct.
	 * reports a std. AMD command set (0x0002), while SST allows to
	 * erase two different sector sizes for the same memory.
	 * 64KB sector (SST call it block)  needs 0x30 to be erased.
	 * 4KB  sector (SST call it sector) needs 0x50 to be erased.
	 * Since CFI query detect the 4KB number of sectors, users expects
	 * a sector granularity of 4KB, and it is here set.
	 */
	if (info->device_id == 0x5D23 || /* SST39VF3201B */
	    info->device_id == 0x5C23) { /* SST39VF3202B */
		/* set sector granularity to 4KB */
		info->cmd_erase_sector = 0x50;
	}
}

static void flash_fixup_num(flash_info_t *info, struct cfi_qry *qry)
{
	/*
	 * The M29EW devices seem to report the CFI information wrong
	 * when it's in 8 bit mode.
	 * There's an app note from Numonyx on this issue.
	 * So adjust the buffer size for M29EW while operating in 8-bit mode
	 */
	if (qry->max_buf_write_size > 0x8 &&
	    info->device_id == 0x7E &&
	    (info->device_id2 == 0x2201 ||
	     info->device_id2 == 0x2301 ||
	     info->device_id2 == 0x2801 ||
	     info->device_id2 == 0x4801)) {
		debug("Adjusted buffer size on Numonyx flash");
		debug(" M29EW family in 8 bit mode\n");
		qry->max_buf_write_size = 0x8;
	}
}

/*
 * The following code cannot be run from FLASH!
 *
 */
ulong flash_get_size(phys_addr_t base, int banknum)
{
	flash_info_t *info = &flash_info[banknum];
	int i, j;
	flash_sect_t sect_cnt;
	phys_addr_t sector;
	unsigned long tmp;
	int size_ratio;
	uchar num_erase_regions;
	int erase_region_size;
	int erase_region_count;
	struct cfi_qry qry;
	unsigned long max_size;

	memset(&qry, 0, sizeof(qry));

	info->ext_addr = 0;
	info->cfi_version = 0;
#ifdef CONFIG_SYS_FLASH_PROTECTION
	info->legacy_unlock = 0;
#endif

	info->start[0] = (ulong)map_physmem(base, info->portwidth, MAP_NOCACHE);

	if (flash_detect_cfi(info, &qry)) {
		info->vendor = le16_to_cpu(get_unaligned(&qry.p_id));
		info->ext_addr = le16_to_cpu(get_unaligned(&qry.p_adr));
		num_erase_regions = qry.num_erase_regions;

		if (info->ext_addr) {
			info->cfi_version = (ushort)flash_read_uchar(info,
						info->ext_addr + 3) << 8;
			info->cfi_version |= (ushort)flash_read_uchar(info,
						info->ext_addr + 4);
		}

#ifdef DEBUG
		flash_printqry(&qry);
#endif

		switch (info->vendor) {
		case CFI_CMDSET_INTEL_PROG_REGIONS:
		case CFI_CMDSET_INTEL_STANDARD:
		case CFI_CMDSET_INTEL_EXTENDED:
			cmdset_intel_init(info, &qry);
			break;
		case CFI_CMDSET_AMD_STANDARD:
		case CFI_CMDSET_AMD_EXTENDED:
			cmdset_amd_init(info, &qry);
			break;
		default:
			printf("CFI: Unknown command set 0x%x\n",
			       info->vendor);
			/*
			 * Unfortunately, this means we don't know how
			 * to get the chip back to Read mode. Might
			 * as well try an Intel-style reset...
			 */
			flash_write_cmd(info, 0, 0, FLASH_CMD_RESET);
			return 0;
		}

		/* Do manufacturer-specific fixups */
		switch (info->manufacturer_id) {
		case 0x0001: /* AMD */
		case 0x0037: /* AMIC */
			flash_fixup_amd(info, &qry);
			break;
		case 0x001f:
			flash_fixup_atmel(info, &qry);
			break;
		case 0x0020:
			flash_fixup_stm(info, &qry);
			break;
		case 0x00bf: /* SST */
			flash_fixup_sst(info, &qry);
			break;
		case 0x0089: /* Numonyx */
			flash_fixup_num(info, &qry);
			break;
		}

		debug("manufacturer is %d\n", info->vendor);
		debug("manufacturer id is 0x%x\n", info->manufacturer_id);
		debug("device id is 0x%x\n", info->device_id);
		debug("device id2 is 0x%x\n", info->device_id2);
		debug("cfi version is 0x%04x\n", info->cfi_version);

		size_ratio = info->portwidth / info->chipwidth;
		/* if the chip is x8/x16 reduce the ratio by half */
		if (info->interface == FLASH_CFI_X8X16 &&
		    info->chipwidth == FLASH_CFI_BY8) {
			size_ratio >>= 1;
		}
		debug("size_ratio %d port %d bits chip %d bits\n",
		      size_ratio, info->portwidth << CFI_FLASH_SHIFT_WIDTH,
		      info->chipwidth << CFI_FLASH_SHIFT_WIDTH);
		info->size = 1 << qry.dev_size;
		/* multiply the size by the number of chips */
		info->size *= size_ratio;
		max_size = cfi_flash_bank_size(banknum);
		if (max_size && info->size > max_size) {
			debug("[truncated from %ldMiB]", info->size >> 20);
			info->size = max_size;
		}
		debug("found %d erase regions\n", num_erase_regions);
		sect_cnt = 0;
		sector = base;
		for (i = 0; i < num_erase_regions; i++) {
			if (i > NUM_ERASE_REGIONS) {
				printf("%d erase regions found, only %d used\n",
				       num_erase_regions, NUM_ERASE_REGIONS);
				break;
			}

			tmp = le32_to_cpu(get_unaligned(
						&qry.erase_region_info[i]));
			debug("erase region %u: 0x%08lx\n", i, tmp);

			erase_region_count = (tmp & 0xffff) + 1;
			tmp >>= 16;
			erase_region_size =
				(tmp & 0xffff) ? ((tmp & 0xffff) * 256) : 128;
			debug("erase_region_count = %d ", erase_region_count);
			debug("erase_region_size = %d\n", erase_region_size);
			for (j = 0; j < erase_region_count; j++) {
				if (sector - base >= info->size)
					break;
				if (sect_cnt >= CONFIG_SYS_MAX_FLASH_SECT) {
					printf("ERROR: too many flash sectors\n");
					break;
				}
				info->start[sect_cnt] =
					(ulong)map_physmem(sector,
							   info->portwidth,
							   MAP_NOCACHE);
				sector += (erase_region_size * size_ratio);

				/*
				 * Only read protection status from
				 * supported devices (intel...)
				 */
				switch (info->vendor) {
				case CFI_CMDSET_INTEL_PROG_REGIONS:
				case CFI_CMDSET_INTEL_EXTENDED:
				case CFI_CMDSET_INTEL_STANDARD:
					/*
					 * Set flash to read-id mode. Otherwise
					 * reading protected status is not
					 * guaranteed.
					 */
					flash_write_cmd(info, sect_cnt, 0,
							FLASH_CMD_READ_ID);
					info->protect[sect_cnt] =
						flash_isset(info, sect_cnt,
							    FLASH_OFFSET_PROTECT,
							    FLASH_STATUS_PROTECT);
					flash_write_cmd(info, sect_cnt, 0,
							FLASH_CMD_RESET);
					break;
				case CFI_CMDSET_AMD_EXTENDED:
				case CFI_CMDSET_AMD_STANDARD:
					if (!info->legacy_unlock) {
						/* default: not protected */
						info->protect[sect_cnt] = 0;
						break;
					}

					/* Read protection (PPB) from sector */
					flash_write_cmd(info, 0, 0,
							info->cmd_reset);
					flash_unlock_seq(info, 0);
					flash_write_cmd(info, 0,
							info->addr_unlock1,
							FLASH_CMD_READ_ID);
					info->protect[sect_cnt] =
						flash_isset(
							info, sect_cnt,
							FLASH_OFFSET_PROTECT,
							FLASH_STATUS_PROTECT);
					break;
				default:
					/* default: not protected */
					info->protect[sect_cnt] = 0;
				}

				sect_cnt++;
			}
		}

		info->sector_count = sect_cnt;
		info->buffer_size = 1 << le16_to_cpu(qry.max_buf_write_size);
		tmp = 1 << qry.block_erase_timeout_typ;
		info->erase_blk_tout = tmp *
			(1 << qry.block_erase_timeout_max);
		tmp = (1 << qry.buf_write_timeout_typ) *
			(1 << qry.buf_write_timeout_max);

		/* round up when converting to ms */
		info->buffer_write_tout = (tmp + 999) / 1000;
		tmp = (1 << qry.word_write_timeout_typ) *
			(1 << qry.word_write_timeout_max);
		/* round up when converting to ms */
		info->write_tout = (tmp + 999) / 1000;
		info->flash_id = FLASH_MAN_CFI;
		if (info->interface == FLASH_CFI_X8X16 &&
		    info->chipwidth == FLASH_CFI_BY8) {
			/* XXX - Need to test on x8/x16 in parallel. */
			info->portwidth >>= 1;
		}

		flash_write_cmd(info, 0, 0, info->cmd_reset);
	}

	return (info->size);
}

#ifdef CONFIG_FLASH_CFI_MTD
void flash_set_verbose(uint v)
{
	flash_verbose = v;
}
#endif

static void cfi_flash_set_config_reg(u32 base, u16 val)
{
#ifdef CONFIG_SYS_CFI_FLASH_CONFIG_REGS
	/*
	 * Only set this config register if really defined
	 * to a valid value (0xffff is invalid)
	 */
	if (val == 0xffff)
		return;

	/*
	 * Set configuration register. Data is "encrypted" in the 16 lower
	 * address bits.
	 */
	flash_write16(FLASH_CMD_SETUP, (void *)(base + (val << 1)));
	flash_write16(FLASH_CMD_SET_CR_CONFIRM, (void *)(base + (val << 1)));

	/*
	 * Finally issue reset-command to bring device back to
	 * read-array mode
	 */
	flash_write16(FLASH_CMD_RESET, (void *)base);
#endif
}

/*-----------------------------------------------------------------------
 */

static void flash_protect_default(void)
{
#if defined(CONFIG_SYS_FLASH_AUTOPROTECT_LIST)
	int i;
	struct apl_s {
		ulong start;
		ulong size;
	} apl[] = CONFIG_SYS_FLASH_AUTOPROTECT_LIST;
#endif

	/* Monitor protection ON by default */
#if (CONFIG_SYS_MONITOR_BASE >= CONFIG_SYS_FLASH_BASE) && \
	(!defined(CONFIG_MONITOR_IS_IN_RAM))
	flash_protect(FLAG_PROTECT_SET,
		      CONFIG_SYS_MONITOR_BASE,
		      CONFIG_SYS_MONITOR_BASE + monitor_flash_len  - 1,
		      flash_get_info(CONFIG_SYS_MONITOR_BASE));
#endif

	/* Environment protection ON by default */
#ifdef CONFIG_ENV_IS_IN_FLASH
	flash_protect(FLAG_PROTECT_SET,
		      CONFIG_ENV_ADDR,
		      CONFIG_ENV_ADDR + CONFIG_ENV_SECT_SIZE - 1,
		      flash_get_info(CONFIG_ENV_ADDR));
#endif

	/* Redundant environment protection ON by default */
#ifdef CONFIG_ENV_ADDR_REDUND
	flash_protect(FLAG_PROTECT_SET,
		      CONFIG_ENV_ADDR_REDUND,
		      CONFIG_ENV_ADDR_REDUND + CONFIG_ENV_SECT_SIZE - 1,
		      flash_get_info(CONFIG_ENV_ADDR_REDUND));
#endif

#if defined(CONFIG_SYS_FLASH_AUTOPROTECT_LIST)
	for (i = 0; i < ARRAY_SIZE(apl); i++) {
		debug("autoprotecting from %08lx to %08lx\n",
		      apl[i].start, apl[i].start + apl[i].size - 1);
		flash_protect(FLAG_PROTECT_SET,
			      apl[i].start,
			      apl[i].start + apl[i].size - 1,
			      flash_get_info(apl[i].start));
	}
#endif
}

unsigned long flash_init(void)
{
	unsigned long size = 0;
	int i;

#ifdef CONFIG_SYS_FLASH_PROTECTION
	/* read environment from EEPROM */
	char s[64];

	env_get_f("unlock", s, sizeof(s));
#endif

#ifdef CONFIG_CFI_FLASH /* for driver model */
	cfi_flash_init_dm();
#endif

	/* Init: no FLASHes known */
	for (i = 0; i < CONFIG_SYS_MAX_FLASH_BANKS; ++i) {
		flash_info[i].flash_id = FLASH_UNKNOWN;

		/* Optionally write flash configuration register */
		cfi_flash_set_config_reg(cfi_flash_bank_addr(i),
					 cfi_flash_config_reg(i));

		if (!flash_detect_legacy(cfi_flash_bank_addr(i), i))
			flash_get_size(cfi_flash_bank_addr(i), i);
		size += flash_info[i].size;
		if (flash_info[i].flash_id == FLASH_UNKNOWN) {
#ifndef CONFIG_SYS_FLASH_QUIET_TEST
			printf("## Unknown flash on Bank %d ", i + 1);
			printf("- Size = 0x%08lx = %ld MB\n",
			       flash_info[i].size,
			       flash_info[i].size >> 20);
#endif /* CONFIG_SYS_FLASH_QUIET_TEST */
		}
#ifdef CONFIG_SYS_FLASH_PROTECTION
		else if (strcmp(s, "yes") == 0) {
			/*
			 * Only the U-Boot image and it's environment
			 * is protected, all other sectors are
			 * unprotected (unlocked) if flash hardware
			 * protection is used (CONFIG_SYS_FLASH_PROTECTION)
			 * and the environment variable "unlock" is
			 * set to "yes".
			 */
			if (flash_info[i].legacy_unlock) {
				int k;

				/*
				 * Disable legacy_unlock temporarily,
				 * since flash_real_protect would
				 * relock all other sectors again
				 * otherwise.
				 */
				flash_info[i].legacy_unlock = 0;

				/*
				 * Legacy unlocking (e.g. Intel J3) ->
				 * unlock only one sector. This will
				 * unlock all sectors.
				 */
				flash_real_protect(&flash_info[i], 0, 0);

				flash_info[i].legacy_unlock = 1;

				/*
				 * Manually mark other sectors as
				 * unlocked (unprotected)
				 */
				for (k = 1; k < flash_info[i].sector_count; k++)
					flash_info[i].protect[k] = 0;
			} else {
				/*
				 * No legancy unlocking -> unlock all sectors
				 */
				flash_protect(FLAG_PROTECT_CLEAR,
					      flash_info[i].start[0],
					      flash_info[i].start[0]
					      + flash_info[i].size - 1,
					      &flash_info[i]);
			}
		}
#endif /* CONFIG_SYS_FLASH_PROTECTION */
	}

	flash_protect_default();
#ifdef CONFIG_FLASH_CFI_MTD
	cfi_mtd_init();
#endif

	return (size);
}

#ifdef CONFIG_CFI_FLASH /* for driver model */
static int cfi_flash_probe(struct udevice *dev)
{
	const fdt32_t *cell;
	int addrc, sizec;
	int len, idx;

	addrc = dev_read_addr_cells(dev);
	sizec = dev_read_size_cells(dev);

	/* decode regs; there may be multiple reg tuples. */
	cell = dev_read_prop(dev, "reg", &len);
	if (!cell)
		return -ENOENT;
	idx = 0;
	len /= sizeof(fdt32_t);
	while (idx < len) {
		phys_addr_t addr;

		addr = dev_translate_address(dev, cell + idx);

		flash_info[cfi_flash_num_flash_banks].dev = dev;
		flash_info[cfi_flash_num_flash_banks].base = addr;
		cfi_flash_num_flash_banks++;

		idx += addrc + sizec;
	}
	gd->bd->bi_flashstart = flash_info[0].base;

	return 0;
}

static const struct udevice_id cfi_flash_ids[] = {
	{ .compatible = "cfi-flash" },
	{ .compatible = "jedec-flash" },
	{}
};

U_BOOT_DRIVER(cfi_flash) = {
	.name	= "cfi_flash",
	.id	= UCLASS_MTD,
	.of_match = cfi_flash_ids,
	.probe = cfi_flash_probe,
};
#endif /* CONFIG_CFI_FLASH */
