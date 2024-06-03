// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000-2003
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * Copyright (C) 2004-2007 Freescale Semiconductor, Inc.
 * TsiChung Liew (Tsi-Chung.Liew@freescale.com)
 */

#include <common.h>

#include <asm/immap.h>

#ifndef CONFIG_SYS_FLASH_CFI
typedef unsigned short FLASH_PORT_WIDTH;
typedef volatile unsigned short FLASH_PORT_WIDTHV;

#define FPW             FLASH_PORT_WIDTH
#define FPWV            FLASH_PORT_WIDTHV

#define FLASH_CYCLE1    0x5555
#define FLASH_CYCLE2    0x2aaa

#define SYNC			__asm__("nop")

/*-----------------------------------------------------------------------
 * Functions
 */

ulong flash_get_size(FPWV * addr, flash_info_t * info);
int flash_get_offsets(ulong base, flash_info_t * info);
int write_word(flash_info_t * info, FPWV * dest, u16 data);
static inline void spin_wheel(void);

flash_info_t flash_info[CONFIG_SYS_MAX_FLASH_BANKS];

ulong flash_init(void)
{
	ulong size = 0;
	ulong fbase = 0;

	fbase = (ulong) CONFIG_SYS_FLASH_BASE;
	flash_get_size((FPWV *) fbase, &flash_info[0]);
	flash_get_offsets((ulong) fbase, &flash_info[0]);
	fbase += flash_info[0].size;
	size += flash_info[0].size;

	/* Protect monitor and environment sectors */
	flash_protect(FLAG_PROTECT_SET,
		      CONFIG_SYS_MONITOR_BASE,
		      CONFIG_SYS_MONITOR_BASE + monitor_flash_len - 1, &flash_info[0]);

	return size;
}

int flash_get_offsets(ulong base, flash_info_t * info)
{
	int i;

	if ((info->flash_id & FLASH_VENDMASK) == FLASH_MAN_SST) {

		info->start[0] = base;
		info->protect[0] = 0;
		for (i = 1; i < CONFIG_SYS_SST_SECT; i++) {
			info->start[i] = info->start[i - 1]
						+ CONFIG_SYS_SST_SECTSZ;
			info->protect[i] = 0;
		}
	}

	return ERR_OK;
}

void flash_print_info(flash_info_t * info)
{
	int i;

	switch (info->flash_id & FLASH_VENDMASK) {
	case FLASH_MAN_SST:
		printf("SST ");
		break;
	default:
		printf("Unknown Vendor ");
		break;
	}

	switch (info->flash_id & FLASH_TYPEMASK) {
	case FLASH_SST6401B:
		printf("SST39VF6401B\n");
		break;
	default:
		printf("Unknown Chip Type\n");
		return;
	}

	if (info->size > 0x100000) {
		int remainder;

		printf("  Size: %ld", info->size >> 20);

		remainder = (info->size % 0x100000);
		if (remainder) {
			remainder >>= 10;
			remainder = (int)((float)
					  (((float)remainder / (float)1024) *
					   10000));
			printf(".%d ", remainder);
		}

		printf("MB in %d Sectors\n", info->sector_count);
	} else
		printf("  Size: %ld KB in %d Sectors\n",
		       info->size >> 10, info->sector_count);

	printf("  Sector Start Addresses:");
	for (i = 0; i < info->sector_count; ++i) {
		if ((i % 5) == 0)
			printf("\n   ");
		printf(" %08lX%s",
		       info->start[i], info->protect[i] ? " (RO)" : "     ");
	}
	printf("\n");
}

/*
 * The following code cannot be run from FLASH!
 */
ulong flash_get_size(FPWV * addr, flash_info_t * info)
{
	u16 value;

	addr[FLASH_CYCLE1] = (FPWV) 0x00AA00AA;	/* for Atmel, Intel ignores this */
	addr[FLASH_CYCLE2] = (FPWV) 0x00550055;	/* for Atmel, Intel ignores this */
	addr[FLASH_CYCLE1] = (FPWV) 0x00900090;	/* selects Intel or Atmel */

	switch (addr[0] & 0xffff) {
	case (u8) SST_MANUFACT:
		info->flash_id = FLASH_MAN_SST;
		value = addr[1];
		break;
	default:
		printf("Unknown Flash\n");
		info->flash_id = FLASH_UNKNOWN;
		info->sector_count = 0;
		info->size = 0;

		*addr = (FPW) 0x00F000F0;
		return (0);	/* no or unknown flash  */
	}

	switch (value) {
	case (u16) SST_ID_xF6401B:
		info->flash_id += FLASH_SST6401B;
		break;
	default:
		info->flash_id = FLASH_UNKNOWN;
		break;
	}

	info->sector_count = 0;
	info->size = 0;
	info->sector_count = CONFIG_SYS_SST_SECT;
	info->size = CONFIG_SYS_SST_SECT * CONFIG_SYS_SST_SECTSZ;

	/* reset ID mode */
	*addr = (FPWV) 0x00F000F0;

	if (info->sector_count > CONFIG_SYS_MAX_FLASH_SECT) {
		printf("** ERROR: sector count %d > max (%d) **\n",
		       info->sector_count, CONFIG_SYS_MAX_FLASH_SECT);
		info->sector_count = CONFIG_SYS_MAX_FLASH_SECT;
	}

	return (info->size);
}

int flash_erase(flash_info_t * info, int s_first, int s_last)
{
	FPWV *addr;
	int flag, prot, sect, count;
	ulong type, start;
	int rcode = 0, flashtype = 0;

	if ((s_first < 0) || (s_first > s_last)) {
		if (info->flash_id == FLASH_UNKNOWN)
			printf("- missing\n");
		else
			printf("- no sectors to erase\n");
		return 1;
	}

	type = (info->flash_id & FLASH_VENDMASK);

	switch (type) {
	case FLASH_MAN_SST:
		flashtype = 1;
		break;
	default:
		type = (info->flash_id & FLASH_VENDMASK);
		printf("Can't erase unknown flash type %08lx - aborted\n",
		       info->flash_id);
		return 1;
	}

	prot = 0;
	for (sect = s_first; sect <= s_last; ++sect) {
		if (info->protect[sect]) {
			prot++;
		}
	}

	if (prot)
		printf("- Warning: %d protected sectors will not be erased!\n",
		       prot);
	else
		printf("\n");

	flag = disable_interrupts();

	start = get_timer(0);

	if ((s_last - s_first) == (CONFIG_SYS_SST_SECT - 1)) {
		if (prot == 0) {
			addr = (FPWV *) info->start[0];

			addr[FLASH_CYCLE1] = 0x00AA;	/* unlock */
			addr[FLASH_CYCLE2] = 0x0055;	/* unlock */
			addr[FLASH_CYCLE1] = 0x0080;	/* erase mode */
			addr[FLASH_CYCLE1] = 0x00AA;	/* unlock */
			addr[FLASH_CYCLE2] = 0x0055;	/* unlock */
			*addr = 0x0030;	/* erase chip */

			count = 0;
			start = get_timer(0);

			while ((*addr & 0x0080) != 0x0080) {
				if (count++ > 0x10000) {
					spin_wheel();
					count = 0;
				}

				if (get_timer(start) > CONFIG_SYS_FLASH_ERASE_TOUT) {
					printf("Timeout\n");
					*addr = 0x00F0;	/* reset to read mode */

					return 1;
				}
			}

			*addr = 0x00F0;	/* reset to read mode */

			printf("\b. done\n");

			if (flag)
				enable_interrupts();

			return 0;
		} else if (prot == CONFIG_SYS_SST_SECT) {
			return 1;
		}
	}

	/* Start erase on unprotected sectors */
	for (sect = s_first; sect <= s_last; sect++) {
		if (info->protect[sect] == 0) {	/* not protected */

			addr = (FPWV *) (info->start[sect]);

			printf(".");

			/* arm simple, non interrupt dependent timer */
			start = get_timer(0);

			switch (flashtype) {
			case 1:
				{
					FPWV *base;	/* first address in bank */

					flag = disable_interrupts();

					base = (FPWV *) (CONFIG_SYS_FLASH_BASE);	/* First sector */

					base[FLASH_CYCLE1] = 0x00AA;	/* unlock */
					base[FLASH_CYCLE2] = 0x0055;	/* unlock */
					base[FLASH_CYCLE1] = 0x0080;	/* erase mode */
					base[FLASH_CYCLE1] = 0x00AA;	/* unlock */
					base[FLASH_CYCLE2] = 0x0055;	/* unlock */
					*addr = 0x0050;	/* erase sector */

					if (flag)
						enable_interrupts();

					while ((*addr & 0x0080) != 0x0080) {
						if (get_timer(start) >
						    CONFIG_SYS_FLASH_ERASE_TOUT) {
							printf("Timeout\n");
							*addr = 0x00F0;	/* reset to read mode */

							rcode = 1;
							break;
						}
					}

					*addr = 0x00F0;	/* reset to read mode */
					break;
				}
			}	/* switch (flashtype) */
		}
	}
	printf(" done\n");

	if (flag)
		enable_interrupts();

	return rcode;
}

int write_buff(flash_info_t * info, uchar * src, ulong addr, ulong cnt)
{
	ulong wp, count;
	u16 data;
	int rc;

	if (info->flash_id == FLASH_UNKNOWN)
		return 4;

	/* get lower word aligned address */
	wp = addr;

	/* handle unaligned start bytes */
	if (wp & 1) {
		data = *((FPWV *) wp);
		data = (data << 8) | *src;

		if ((rc = write_word(info, (FPWV *) wp, data)) != 0)
			return (rc);

		wp++;
		cnt -= 1;
		src++;
	}

	while (cnt >= 2) {
		/*
		 * handle word aligned part
		 */
		count = 0;
		data = *((FPWV *) src);

		if ((rc = write_word(info, (FPWV *) wp, data)) != 0)
			return (rc);

		wp += 2;
		src += 2;
		cnt -= 2;

		if (count++ > 0x800) {
			spin_wheel();
			count = 0;
		}
	}
	/* handle word aligned part */
	if (cnt) {
		/* handle word aligned part */
		count = 0;
		data = *((FPWV *) wp);

		data = (data & 0x00FF) | (*src << 8);

		if ((rc = write_word(info, (FPWV *) wp, data)) != 0)
			return (rc);

		wp++;
		src++;
		cnt -= 1;
		if (count++ > 0x800) {
			spin_wheel();
			count = 0;
		}
	}

	if (cnt == 0)
		return ERR_OK;

	return ERR_OK;
}

/*-----------------------------------------------------------------------
 * Write a word to Flash
 * A word is 16 bits, whichever the bus width of the flash bank
 * (not an individual chip) is.
 *
 * returns:
 * 0 - OK
 * 1 - write timeout
 * 2 - Flash not erased
 */
int write_word(flash_info_t * info, FPWV * dest, u16 data)
{
	ulong start;
	int flag;
	int res = 0;		/* result, assume success */
	FPWV *base;		/* first address in flash bank */

	/* Check if Flash is (sufficiently) erased */
	if ((*dest & (u8) data) != (u8) data) {
		return (2);
	}

	base = (FPWV *) (CONFIG_SYS_FLASH_BASE);

	/* Disable interrupts which might cause a timeout here */
	flag = disable_interrupts();

	base[FLASH_CYCLE1] = (u8) 0x00AA00AA;	/* unlock */
	base[FLASH_CYCLE2] = (u8) 0x00550055;	/* unlock */
	base[FLASH_CYCLE1] = (u8) 0x00A000A0;	/* selects program mode */

	*dest = data;		/* start programming the data */

	/* re-enable interrupts if necessary */
	if (flag)
		enable_interrupts();

	start = get_timer(0);

	/* data polling for D7 */
	while (res == 0
	       && (*dest & (u8) 0x00800080) != (data & (u8) 0x00800080)) {
		if (get_timer(start) > CONFIG_SYS_FLASH_WRITE_TOUT) {
			*dest = (u8) 0x00F000F0;	/* reset bank */
			res = 1;
		}
	}

	*dest++ = (u8) 0x00F000F0;	/* reset bank */

	return (res);
}

static inline void spin_wheel(void)
{
	static int p = 0;
	static char w[] = "\\/-";

	printf("\010%c", w[p]);
	(++p == 3) ? (p = 0) : 0;
}

#endif
