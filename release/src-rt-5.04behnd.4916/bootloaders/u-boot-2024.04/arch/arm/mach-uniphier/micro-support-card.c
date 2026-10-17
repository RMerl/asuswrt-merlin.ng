// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2012-2015 Panasonic Corporation
 * Copyright (C) 2015-2020 Socionext Inc.
 *   Author: Masahiro Yamada <yamada.masahiro@socionext.com>
 */

#include <dm.h>
#include <fdt_support.h>
#include <linux/ctype.h>
#include <linux/delay.h>
#include <linux/io.h>
#include <asm/global_data.h>

#include "micro-support-card.h"

#define SMC911X_OFFSET			0x00000
#define LED_OFFSET			0x90000
#define NS16550A_OFFSET			0xb0000
#define MICRO_SUPPORT_CARD_RESET	0xd0034
#define MICRO_SUPPORT_CARD_REVISION	0xd00e0

static bool support_card_found;
static void __iomem *support_card_base;

static void support_card_detect(void)
{
	DECLARE_GLOBAL_DATA_PTR;
	const void *fdt = gd->fdt_blob;
	int offset;
	u64 addr, addr2;

	offset = fdt_node_offset_by_compatible(fdt, 0, "smsc,lan9118");
	if (offset < 0)
		return;

	addr = fdt_get_base_address(fdt, offset);
	if (addr == OF_BAD_ADDR)
		return;
	addr -= SMC911X_OFFSET;

	offset = fdt_node_offset_by_compatible(fdt, 0, "ns16550a");
	if (offset < 0)
		return;

	addr2 = fdt_get_base_address(fdt, offset);
	if (addr2 == OF_BAD_ADDR)
		return;
	addr2 -= NS16550A_OFFSET;

	/* sanity check */
	if (addr != addr2)
		return;

	support_card_base = ioremap(addr, 0x100000);

	support_card_found = true;
}

/*
 * 0: reset deassert, 1: reset
 *
 * bit[0]: LAN, I2C, LED
 * bit[1]: UART
 */
static void support_card_reset_deassert(void)
{
	writel(0x00010000, support_card_base + MICRO_SUPPORT_CARD_RESET);
}

static void support_card_reset(void)
{
	writel(0x00020003, support_card_base + MICRO_SUPPORT_CARD_RESET);
}

static int support_card_show_revision(void)
{
	u32 revision;

	revision = readl(support_card_base + MICRO_SUPPORT_CARD_REVISION);
	revision &= 0xff;

	/* revision 3.6.x card changed the revision format */
	printf("SC:    Micro Support Card (CPLD version %s%d.%d)\n",
	       revision >> 4 == 6 ? "3." : "",
	       revision >> 4, revision & 0xf);

	return 0;
}

void support_card_init(void)
{
	struct udevice *dev;
	int ret;

	/* The system bus must be initialized for access to the support card. */
	ret = uclass_get_device_by_driver(UCLASS_SIMPLE_BUS,
					  DM_DRIVER_GET(uniphier_system_bus_driver),
					  &dev);
	if (ret)
		return;

	/* Check DT to see if this board has the support card. */
	support_card_detect();

	if (!support_card_found)
		return;

	support_card_reset();
	/*
	 * After power on, we need to keep the LAN controller in reset state
	 * for a while. (200 usec)
	 */
	udelay(200);
	support_card_reset_deassert();

	support_card_show_revision();
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

	if (!support_card_found)
		return;

	if (!s)
		return;

	for (i = 0; i < 4; i++) {
		val <<= 8;
		val |= char2ledval(*s);
		if (*s != '\0')
			s++;
	}

	writel(~val, support_card_base + LED_OFFSET);
}
