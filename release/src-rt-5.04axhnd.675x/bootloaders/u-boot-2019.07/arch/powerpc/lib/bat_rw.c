// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2002
 * Rich Ireland, Enterasys Networks, rireland@enterasys.com.
 */

#include <common.h>
#include <asm/processor.h>
#include <asm/mmu.h>
#include <asm/io.h>
#include <linux/compiler.h>

#ifdef CONFIG_ADDR_MAP
#include <addr_map.h>
#endif

DECLARE_GLOBAL_DATA_PTR;

int write_bat (ppc_bat_t bat, unsigned long upper, unsigned long lower)
{
	__maybe_unused int batn = -1;

	sync();

	switch (bat) {
	case DBAT0:
		mtspr (DBAT0L, lower);
		mtspr (DBAT0U, upper);
		batn = 0;
		break;
	case IBAT0:
		mtspr (IBAT0L, lower);
		mtspr (IBAT0U, upper);
		break;
	case DBAT1:
		mtspr (DBAT1L, lower);
		mtspr (DBAT1U, upper);
		batn = 1;
		break;
	case IBAT1:
		mtspr (IBAT1L, lower);
		mtspr (IBAT1U, upper);
		break;
	case DBAT2:
		mtspr (DBAT2L, lower);
		mtspr (DBAT2U, upper);
		batn = 2;
		break;
	case IBAT2:
		mtspr (IBAT2L, lower);
		mtspr (IBAT2U, upper);
		break;
	case DBAT3:
		mtspr (DBAT3L, lower);
		mtspr (DBAT3U, upper);
		batn = 3;
		break;
	case IBAT3:
		mtspr (IBAT3L, lower);
		mtspr (IBAT3U, upper);
		break;
#ifdef CONFIG_HIGH_BATS
	case DBAT4:
		mtspr (DBAT4L, lower);
		mtspr (DBAT4U, upper);
		batn = 4;
		break;
	case IBAT4:
		mtspr (IBAT4L, lower);
		mtspr (IBAT4U, upper);
		break;
	case DBAT5:
		mtspr (DBAT5L, lower);
		mtspr (DBAT5U, upper);
		batn = 5;
		break;
	case IBAT5:
		mtspr (IBAT5L, lower);
		mtspr (IBAT5U, upper);
		break;
	case DBAT6:
		mtspr (DBAT6L, lower);
		mtspr (DBAT6U, upper);
		batn = 6;
		break;
	case IBAT6:
		mtspr (IBAT6L, lower);
		mtspr (IBAT6U, upper);
		break;
	case DBAT7:
		mtspr (DBAT7L, lower);
		mtspr (DBAT7U, upper);
		batn = 7;
		break;
	case IBAT7:
		mtspr (IBAT7L, lower);
		mtspr (IBAT7U, upper);
		break;
#endif
	default:
		return (-1);
	}

#ifdef CONFIG_ADDR_MAP
	if ((gd->flags & GD_FLG_RELOC) && (batn >= 0)) {
		phys_size_t size;
		if (!BATU_VALID(upper))
			size = 0;
		else
			size = BATU_SIZE(upper);
		addrmap_set_entry(BATU_VADDR(upper), BATL_PADDR(lower),
				  size, batn);
	}
#endif

	sync();
	isync();

	return (0);
}

int read_bat (ppc_bat_t bat, unsigned long *upper, unsigned long *lower)
{
	unsigned long register u;
	unsigned long register l;

	switch (bat) {
	case DBAT0:
		l = mfspr (DBAT0L);
		u = mfspr (DBAT0U);
		break;
	case IBAT0:
		l = mfspr (IBAT0L);
		u = mfspr (IBAT0U);
		break;
	case DBAT1:
		l = mfspr (DBAT1L);
		u = mfspr (DBAT1U);
		break;
	case IBAT1:
		l = mfspr (IBAT1L);
		u = mfspr (IBAT1U);
		break;
	case DBAT2:
		l = mfspr (DBAT2L);
		u = mfspr (DBAT2U);
		break;
	case IBAT2:
		l = mfspr (IBAT2L);
		u = mfspr (IBAT2U);
		break;
	case DBAT3:
		l = mfspr (DBAT3L);
		u = mfspr (DBAT3U);
		break;
	case IBAT3:
		l = mfspr (IBAT3L);
		u = mfspr (IBAT3U);
		break;
#ifdef CONFIG_HIGH_BATS
	case DBAT4:
		l = mfspr (DBAT4L);
		u = mfspr (DBAT4U);
		break;
	case IBAT4:
		l = mfspr (IBAT4L);
		u = mfspr (IBAT4U);
		break;
	case DBAT5:
		l = mfspr (DBAT5L);
		u = mfspr (DBAT5U);
		break;
	case IBAT5:
		l = mfspr (IBAT5L);
		u = mfspr (IBAT5U);
		break;
	case DBAT6:
		l = mfspr (DBAT6L);
		u = mfspr (DBAT6U);
		break;
	case IBAT6:
		l = mfspr (IBAT6L);
		u = mfspr (IBAT6U);
		break;
	case DBAT7:
		l = mfspr (DBAT7L);
		u = mfspr (DBAT7U);
		break;
	case IBAT7:
		l = mfspr (IBAT7L);
		u = mfspr (IBAT7U);
		break;
#endif
	default:
		return (-1);
	}

	*upper = u;
	*lower = l;

	return (0);
}

void print_bats(void)
{
	printf("BAT registers:\n");

	printf ("\tIBAT0L = 0x%08X ", mfspr (IBAT0L));
	printf ("\tIBAT0U = 0x%08X\n", mfspr (IBAT0U));
	printf ("\tDBAT0L = 0x%08X ", mfspr (DBAT0L));
	printf ("\tDBAT0U = 0x%08X\n", mfspr (DBAT0U));
	printf ("\tIBAT1L = 0x%08X ", mfspr (IBAT1L));
	printf ("\tIBAT1U = 0x%08X\n", mfspr (IBAT1U));
	printf ("\tDBAT1L = 0x%08X ", mfspr (DBAT1L));
	printf ("\tDBAT1U = 0x%08X\n", mfspr (DBAT1U));
	printf ("\tIBAT2L = 0x%08X ", mfspr (IBAT2L));
	printf ("\tIBAT2U = 0x%08X\n", mfspr (IBAT2U));
	printf ("\tDBAT2L = 0x%08X ", mfspr (DBAT2L));
	printf ("\tDBAT2U = 0x%08X\n", mfspr (DBAT2U));
	printf ("\tIBAT3L = 0x%08X ", mfspr (IBAT3L));
	printf ("\tIBAT3U = 0x%08X\n", mfspr (IBAT3U));
	printf ("\tDBAT3L = 0x%08X ", mfspr (DBAT3L));
	printf ("\tDBAT3U = 0x%08X\n", mfspr (DBAT3U));

#ifdef CONFIG_HIGH_BATS
	printf ("\tIBAT4L = 0x%08X ", mfspr (IBAT4L));
	printf ("\tIBAT4U = 0x%08X\n", mfspr (IBAT4U));
	printf ("\tDBAT4L = 0x%08X ", mfspr (DBAT4L));
	printf ("\tDBAT4U = 0x%08X\n", mfspr (DBAT4U));
	printf ("\tIBAT5L = 0x%08X ", mfspr (IBAT5L));
	printf ("\tIBAT5U = 0x%08X\n", mfspr (IBAT5U));
	printf ("\tDBAT5L = 0x%08X ", mfspr (DBAT5L));
	printf ("\tDBAT5U = 0x%08X\n", mfspr (DBAT5U));
	printf ("\tIBAT6L = 0x%08X ", mfspr (IBAT6L));
	printf ("\tIBAT6U = 0x%08X\n", mfspr (IBAT6U));
	printf ("\tDBAT6L = 0x%08X ", mfspr (DBAT6L));
	printf ("\tDBAT6U = 0x%08X\n", mfspr (DBAT6U));
	printf ("\tIBAT7L = 0x%08X ", mfspr (IBAT7L));
	printf ("\tIBAT7U = 0x%08X\n", mfspr (IBAT7U));
	printf ("\tDBAT7L = 0x%08X ", mfspr (DBAT7L));
	printf ("\tDBAT7U = 0x%08X\n", mfspr (DBAT7U));
#endif
}
