// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000-2003
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#include <common.h>
#include <console.h>

#define PHYS_FLASH_1 CONFIG_SYS_FLASH_BASE
#define FLASH_BANK_SIZE 0x200000

flash_info_t flash_info[CONFIG_SYS_MAX_FLASH_BANKS];

void flash_print_info (flash_info_t * info)
{
	int i;

	switch (info->flash_id & FLASH_VENDMASK) {
	case (AMD_MANUFACT & FLASH_VENDMASK):
		printf ("AMD: ");
		break;
	default:
		printf ("Unknown Vendor ");
		break;
	}

	switch (info->flash_id & FLASH_TYPEMASK) {
	case (AMD_ID_PL160CB & FLASH_TYPEMASK):
		printf ("AM29PL160CB (16Mbit)\n");
		break;
	default:
		printf ("Unknown Chip Type\n");
		goto Done;
		break;
	}

	printf ("  Size: %ld MB in %d Sectors\n",
		info->size >> 20, info->sector_count);

	printf ("  Sector Start Addresses:");
	for (i = 0; i < info->sector_count; i++) {
		if ((i % 5) == 0) {
			printf ("\n   ");
		}
		printf (" %08lX%s", info->start[i],
			info->protect[i] ? " (RO)" : "     ");
	}
	printf ("\n");

Done:
	return;
}


unsigned long flash_init (void)
{
	int i, j;
	ulong size = 0;

	for (i = 0; i < CONFIG_SYS_MAX_FLASH_BANKS; i++) {
		ulong flashbase = 0;

		flash_info[i].flash_id =
			(AMD_MANUFACT & FLASH_VENDMASK) |
			(AMD_ID_PL160CB & FLASH_TYPEMASK);
		flash_info[i].size = FLASH_BANK_SIZE;
		flash_info[i].sector_count = CONFIG_SYS_MAX_FLASH_SECT;
		memset (flash_info[i].protect, 0, CONFIG_SYS_MAX_FLASH_SECT);
		if (i == 0)
			flashbase = PHYS_FLASH_1;
		else
			panic ("configured to many flash banks!\n");

		for (j = 0; j < flash_info[i].sector_count; j++) {
			if (j == 0) {
				/* 1st is 16 KiB */
				flash_info[i].start[j] = flashbase;
			}
			if ((j >= 1) && (j <= 2)) {
				/* 2nd and 3rd are 8 KiB */
				flash_info[i].start[j] =
					flashbase + 0x4000 + 0x2000 * (j - 1);
			}
			if (j == 3) {
				/* 4th is 224 KiB */
				flash_info[i].start[j] = flashbase + 0x8000;
			}
			if ((j >= 4) && (j <= 10)) {
				/* rest is 256 KiB */
				flash_info[i].start[j] =
					flashbase + 0x40000 + 0x40000 * (j -
									 4);
			}
		}
		size += flash_info[i].size;
	}

	flash_protect (FLAG_PROTECT_SET,
		       CONFIG_SYS_FLASH_BASE,
		       CONFIG_SYS_FLASH_BASE + 0x3ffff, &flash_info[0]);

	return size;
}


#define CMD_READ_ARRAY		0x00F0
#define CMD_UNLOCK1		0x00AA
#define CMD_UNLOCK2		0x0055
#define CMD_ERASE_SETUP		0x0080
#define CMD_ERASE_CONFIRM	0x0030
#define CMD_PROGRAM		0x00A0
#define CMD_UNLOCK_BYPASS	0x0020

#define MEM_FLASH_ADDR1		(*(volatile u16 *)(CONFIG_SYS_FLASH_BASE + (0x00000555<<1)))
#define MEM_FLASH_ADDR2		(*(volatile u16 *)(CONFIG_SYS_FLASH_BASE + (0x000002AA<<1)))

#define BIT_ERASE_DONE		0x0080
#define BIT_RDY_MASK		0x0080
#define BIT_PROGRAM_ERROR	0x0020
#define BIT_TIMEOUT		0x80000000	/* our flag */

#define READY 1
#define ERR   2
#define TMO   4


int flash_erase (flash_info_t * info, int s_first, int s_last)
{
	ulong result;
	int iflag, cflag, prot, sect;
	int rc = ERR_OK;
	int chip1;
	ulong start;

	/* first look for protection bits */

	if (info->flash_id == FLASH_UNKNOWN)
		return ERR_UNKNOWN_FLASH_TYPE;

	if ((s_first < 0) || (s_first > s_last)) {
		return ERR_INVAL;
	}

	if ((info->flash_id & FLASH_VENDMASK) !=
	    (AMD_MANUFACT & FLASH_VENDMASK)) {
		return ERR_UNKNOWN_FLASH_VENDOR;
	}

	prot = 0;
	for (sect = s_first; sect <= s_last; ++sect) {
		if (info->protect[sect]) {
			prot++;
		}
	}
	if (prot)
		return ERR_PROTECTED;

	/*
	 * Disable interrupts which might cause a timeout
	 * here. Remember that our exception vectors are
	 * at address 0 in the flash, and we don't want a
	 * (ticker) exception to happen while the flash
	 * chip is in programming mode.
	 */

	cflag = icache_status ();
	icache_disable ();
	iflag = disable_interrupts ();

	printf ("\n");

	/* Start erase on unprotected sectors */
	for (sect = s_first; sect <= s_last && !ctrlc (); sect++) {
		printf ("Erasing sector %2d ... ", sect);

		/* arm simple, non interrupt dependent timer */
		start = get_timer(0);

		if (info->protect[sect] == 0) {	/* not protected */
			volatile u16 *addr =
				(volatile u16 *) (info->start[sect]);

			MEM_FLASH_ADDR1 = CMD_UNLOCK1;
			MEM_FLASH_ADDR2 = CMD_UNLOCK2;
			MEM_FLASH_ADDR1 = CMD_ERASE_SETUP;

			MEM_FLASH_ADDR1 = CMD_UNLOCK1;
			MEM_FLASH_ADDR2 = CMD_UNLOCK2;
			*addr = CMD_ERASE_CONFIRM;

			/* wait until flash is ready */
			chip1 = 0;

			do {
				result = *addr;

				/* check timeout */
				if (get_timer(start) > CONFIG_SYS_FLASH_ERASE_TOUT) {
					MEM_FLASH_ADDR1 = CMD_READ_ARRAY;
					chip1 = TMO;
					break;
				}

				if (!chip1
				    && (result & 0xFFFF) & BIT_ERASE_DONE)
					chip1 = READY;

			} while (!chip1);

			MEM_FLASH_ADDR1 = CMD_READ_ARRAY;

			if (chip1 == ERR) {
				rc = ERR_PROG_ERROR;
				goto outahere;
			}
			if (chip1 == TMO) {
				rc = ERR_TIMEOUT;
				goto outahere;
			}

			printf ("ok.\n");
		} else {	/* it was protected */

			printf ("protected!\n");
		}
	}

	if (ctrlc ())
		printf ("User Interrupt!\n");

      outahere:
	/* allow flash to settle - wait 10 ms */
	udelay (10000);

	if (iflag)
		enable_interrupts ();

	if (cflag)
		icache_enable ();

	return rc;
}

static int write_word (flash_info_t * info, ulong dest, ulong data)
{
	volatile u16 *addr = (volatile u16 *) dest;
	ulong result;
	int rc = ERR_OK;
	int cflag, iflag;
	int chip1;
	ulong start;

	/*
	 * Check if Flash is (sufficiently) erased
	 */
	result = *addr;
	if ((result & data) != data)
		return ERR_NOT_ERASED;


	/*
	 * Disable interrupts which might cause a timeout
	 * here. Remember that our exception vectors are
	 * at address 0 in the flash, and we don't want a
	 * (ticker) exception to happen while the flash
	 * chip is in programming mode.
	 */

	cflag = icache_status ();
	icache_disable ();
	iflag = disable_interrupts ();

	MEM_FLASH_ADDR1 = CMD_UNLOCK1;
	MEM_FLASH_ADDR2 = CMD_UNLOCK2;
	MEM_FLASH_ADDR1 = CMD_PROGRAM;
	*addr = data;

	/* arm simple, non interrupt dependent timer */
	start = get_timer(0);

	/* wait until flash is ready */
	chip1 = 0;
	do {
		result = *addr;

		/* check timeout */
		if (get_timer(start) > CONFIG_SYS_FLASH_ERASE_TOUT) {
			chip1 = ERR | TMO;
			break;
		}
		if (!chip1 && ((result & 0x80) == (data & 0x80)))
			chip1 = READY;

	} while (!chip1);

	*addr = CMD_READ_ARRAY;

	if (chip1 == ERR || *addr != data)
		rc = ERR_PROG_ERROR;

	if (iflag)
		enable_interrupts ();

	if (cflag)
		icache_enable ();

	return rc;
}


int write_buff (flash_info_t * info, uchar * src, ulong addr, ulong cnt)
{
	ulong wp, data;
	int rc;

	if (addr & 1) {
		printf ("unaligned destination not supported\n");
		return ERR_ALIGN;
	}

#if 0
	if (cnt & 1) {
		printf ("odd transfer sizes not supported\n");
		return ERR_ALIGN;
	}
#endif

	wp = addr;

	if (addr & 1) {
		data = (*((volatile u8 *) addr) << 8) | *((volatile u8 *)
							  src);
		if ((rc = write_word (info, wp - 1, data)) != 0) {
			return (rc);
		}
		src += 1;
		wp += 1;
		cnt -= 1;
	}

	while (cnt >= 2) {
		data = *((volatile u16 *) src);
		if ((rc = write_word (info, wp, data)) != 0) {
			return (rc);
		}
		src += 2;
		wp += 2;
		cnt -= 2;
	}

	if (cnt == 1) {
		data = (*((volatile u8 *) src) << 8) |
			*((volatile u8 *) (wp + 1));
		if ((rc = write_word (info, wp, data)) != 0) {
			return (rc);
		}
		src += 1;
		wp += 1;
		cnt -= 1;
	}

	return ERR_OK;
}
