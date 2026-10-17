// SPDX-License-Identifier: GPL-2.0+

#include <common.h>
#include <cpu_func.h>
#include <asm/processor.h>
#include <asm/system.h>
#include <asm/io.h>

#define WDT_BASE	WTCNT

#define WDT_WD		(1 << 6)
#define WDT_RST_P	(0)
#define WDT_RST_M	(1 << 5)
#define WDT_ENABLE	(1 << 7)

#if defined(CONFIG_WATCHDOG)
static unsigned char csr_read(void)
{
	return inb(WDT_BASE + 0x04);
}

static void cnt_write(unsigned char value)
{
	outl((unsigned short)value | 0x5A00, WDT_BASE + 0x00);
}

static void csr_write(unsigned char value)
{
	outl((unsigned short)value | 0xA500, WDT_BASE + 0x04);
}

void watchdog_reset(void)
{
	outl(0x55000000, WDT_BASE + 0x08);
}

int watchdog_init(void)
{
	/* Set overflow time*/
	cnt_write(0);
	/* Power on reset */
	csr_write(WDT_WD|WDT_RST_P|WDT_ENABLE);

	return 0;
}

int watchdog_disable(void)
{
	csr_write(csr_read() & ~WDT_ENABLE);
	return 0;
}
#endif

void reset_cpu(void)
{
	/* Address error with SR.BL=1 first. */
	trigger_address_error();

	while (1)
		;
}
