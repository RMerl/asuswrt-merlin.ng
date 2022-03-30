// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2012-2015 Panasonic Corporation
 * Copyright (C) 2015-2016 Socionext Inc.
 *   Author: Masahiro Yamada <yamada.masahiro@socionext.com>
 */

#include <common.h>
#include <linux/ctype.h>
#include <linux/io.h>

#include "micro-support-card.h"

#define MICRO_SUPPORT_CARD_BASE		0x43f00000
#define SMC911X_BASE			((MICRO_SUPPORT_CARD_BASE) + 0x00000)
#define LED_BASE			((MICRO_SUPPORT_CARD_BASE) + 0x90000)
#define NS16550A_BASE			((MICRO_SUPPORT_CARD_BASE) + 0xb0000)
#define MICRO_SUPPORT_CARD_RESET	((MICRO_SUPPORT_CARD_BASE) + 0xd0034)
#define MICRO_SUPPORT_CARD_REVISION	((MICRO_SUPPORT_CARD_BASE) + 0xd00E0)

/*
 * 0: reset deassert, 1: reset
 *
 * bit[0]: LAN, I2C, LED
 * bit[1]: UART
 */
static void support_card_reset_deassert(void)
{
	writel(0x00010000, MICRO_SUPPORT_CARD_RESET);
}

static void support_card_reset(void)
{
	writel(0x00020003, MICRO_SUPPORT_CARD_RESET);
}

static int support_card_show_revision(void)
{
	u32 revision;

	revision = readl(MICRO_SUPPORT_CARD_REVISION);
	revision &= 0xff;

	/* revision 3.6.x card changed the revision format */
	printf("SC:    Micro Support Card (CPLD version %s%d.%d)\n",
	       revision >> 4 == 6 ? "3." : "",
	       revision >> 4, revision & 0xf);

	return 0;
}

void support_card_init(void)
{
	support_card_reset();
	/*
	 * After power on, we need to keep the LAN controller in reset state
	 * for a while. (200 usec)
	 */
	udelay(200);
	support_card_reset_deassert();

	support_card_show_revision();
}

#if defined(CONFIG_SMC911X)
#include <netdev.h>

int board_eth_init(bd_t *bis)
{
	return smc911x_initialize(0, SMC911X_BASE);
}
#endif

#if defined(CONFIG_MTD_NOR_FLASH)

#include <mtd/cfi_flash.h>

struct memory_bank {
	phys_addr_t base;
	unsigned long size;
};

static int mem_is_flash(const struct memory_bank *mem)
{
	const int loop = 128;
	u32 *scratch_addr;
	u32 saved_value;
	int ret = 1;
	int i;

	/* just in case, use the tail of the memory bank */
	scratch_addr = map_physmem(mem->base + mem->size - sizeof(u32) * loop,
				   sizeof(u32) * loop, MAP_NOCACHE);

	for (i = 0; i < loop; i++, scratch_addr++) {
		saved_value = readl(scratch_addr);
		writel(~saved_value, scratch_addr);
		if (readl(scratch_addr) != saved_value) {
			/* We assume no memory or SRAM here. */
			writel(saved_value, scratch_addr);
			ret = 0;
			break;
		}
	}

	unmap_physmem(scratch_addr, MAP_NOCACHE);

	return ret;
}

/* {address, size} */
static const struct memory_bank memory_banks[] = {
	{0x42000000, 0x01f00000},
};

static const struct memory_bank
*flash_banks_list[CONFIG_SYS_MAX_FLASH_BANKS_DETECT];

phys_addr_t cfi_flash_bank_addr(int i)
{
	return flash_banks_list[i]->base;
}

unsigned long cfi_flash_bank_size(int i)
{
	return flash_banks_list[i]->size;
}

static void detect_num_flash_banks(void)
{
	const struct memory_bank *memory_bank, *end;

	cfi_flash_num_flash_banks = 0;

	memory_bank = memory_banks;
	end = memory_bank + ARRAY_SIZE(memory_banks);

	for (; memory_bank < end; memory_bank++) {
		if (cfi_flash_num_flash_banks >=
		    CONFIG_SYS_MAX_FLASH_BANKS_DETECT)
			break;

		if (mem_is_flash(memory_bank)) {
			flash_banks_list[cfi_flash_num_flash_banks] =
								memory_bank;

			debug("flash bank found: base = 0x%lx, size = 0x%lx\n",
			      (unsigned long)memory_bank->base,
			      (unsigned long)memory_bank->size);
			cfi_flash_num_flash_banks++;
		}
	}

	debug("number of flash banks: %d\n", cfi_flash_num_flash_banks);
}
#else /* CONFIG_MTD_NOR_FLASH */
static void detect_num_flash_banks(void)
{
};
#endif /* CONFIG_MTD_NOR_FLASH */

void support_card_late_init(void)
{
	detect_num_flash_banks();
}

static const u8 ledval_num[] = {
	0x7e, /* 0 */
	0x0c, /* 1 */
	0xb6, /* 2 */
	0x9e, /* 3 */
	0xcc, /* 4 */
	0xda, /* 5 */
	0xfa, /* 6 */
	0x4e, /* 7 */
	0xfe, /* 8 */
	0xde, /* 9 */
};

static const u8 ledval_alpha[] = {
	0xee, /* A */
	0xf8, /* B */
	0x72, /* C */
	0xbc, /* D */
	0xf2, /* E */
	0xe2, /* F */
	0x7a, /* G */
	0xe8, /* H */
	0x08, /* I */
	0x3c, /* J */
	0xea, /* K */
	0x70, /* L */
	0x6e, /* M */
	0xa8, /* N */
	0xb8, /* O */
	0xe6, /* P */
	0xce, /* Q */
	0xa0, /* R */
	0xc8, /* S */
	0x8c, /* T */
	0x7c, /* U */
	0x54, /* V */
	0xfc, /* W */
	0xec, /* X */
	0xdc, /* Y */
	0xa4, /* Z */
};

static u8 char2ledval(char c)
{
	if (isdigit(c))
		return ledval_num[c - '0'];
	else if (isalpha(c))
		return ledval_alpha[toupper(c) - 'A'];

	return 0;
}

void led_puts(const char *s)
{
	int i;
	u32 val = 0;

	if (!s)
		return;

	for (i = 0; i < 4; i++) {
		val <<= 8;
		val |= char2ledval(*s);
		if (*s != '\0')
			s++;
	}

	writel(~val, LED_BASE);
}
