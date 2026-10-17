// SPDX-License-Identifier: GPL-2.0+

#include <common.h>
#include <asm/io.h>
#include <asm/arch/hardware.h>
#include <asm/arch/at91_gpbr.h>

/*
 * We combine the CONFIG_SYS_BOOTCOUNT_MAGIC and bootcount in one 32-bit
 * register. This is done so we need to use only one of the four GPBR
 * registers.
 */
void bootcount_store(ulong a)
{
	at91_gpbr_t *gpbr = (at91_gpbr_t *) ATMEL_BASE_GPBR;

	writel((CONFIG_SYS_BOOTCOUNT_MAGIC & 0xffff0000) | (a & 0x0000ffff),
	       &gpbr->reg[AT91_GPBR_INDEX_BOOTCOUNT]);
}

ulong bootcount_load(void)
{
	at91_gpbr_t *gpbr = (at91_gpbr_t *) ATMEL_BASE_GPBR;

	ulong val = readl(&gpbr->reg[AT91_GPBR_INDEX_BOOTCOUNT]);
	if ((val & 0xffff0000) != (CONFIG_SYS_BOOTCOUNT_MAGIC & 0xffff0000))
		return 0;
	else
		return val & 0x0000ffff;
}
