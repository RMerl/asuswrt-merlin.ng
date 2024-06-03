// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2008 Stefan Roese <sr@denx.de>, DENX Software Engineering
 */

#include <common.h>
#include <asm/io.h>
#include "vct.h"

static u32 ebi_read(u32 addr)
{
	addr &= ~0xFC000000;

	reg_write(EBI_CPU_IO_ACCS(EBI_BASE), EXT_DEVICE_CHANNEL_2 | addr);
	ebi_wait();

	return reg_read(EBI_IO_ACCS_DATA(EBI_BASE));
}

static int ebi_write_u16(u32 addr, u32 data, int fetchIO)
{
	u32 val = (data << 16);

	addr &= ~0xFC000000;

	ebi_wait();

	reg_write(EBI_IO_ACCS_DATA(EBI_BASE), val);
	reg_write(EBI_CPU_IO_ACCS(EBI_BASE),
		  EXT_DEVICE_CHANNEL_2 | EBI_CPU_WRITE | addr);
	ebi_wait();

	if (fetchIO) {
		u32 counter = 0;
		while (!(reg_read(EBI_SIG_LEVEL(EBI_BASE)) & EXT_CPU_IORDY_SL)) {
			if (counter++ > 0xFFFFFF)
				return 1;
		}
	}

	return 0;
}

static u16 ebi_read_u16(u32 addr)
{
	return ((ebi_read(addr) >> 16) & 0xFFFF);
}

static u8 ebi_read_u8(u32 addr)
{
	u32 val = ebi_read(addr) >> 16;

	if (addr & 0x1)
		return val & 0xff;
	else
		return (val >> 8) & 0xff;
}

/*
 * EBI initialization for NOR FLASH access
 */
int ebi_init_nor_flash(void)
{
	reg_write(EBI_DEV1_CONFIG1(EBI_BASE), 0x83000);

	reg_write(EBI_DEV2_CONFIG1(EBI_BASE), 0x400002);
	reg_write(EBI_DEV2_CONFIG2(EBI_BASE), 0x50);

	reg_write(EBI_DEV2_TIM1_RD1(EBI_BASE), 0x409113);
	reg_write(EBI_DEV2_TIM1_RD2(EBI_BASE), 0xFF01000);
	reg_write(EBI_DEV2_TIM1_WR1(EBI_BASE), 0x04003113);
	reg_write(EBI_DEV2_TIM1_WR2(EBI_BASE), 0x3FC12011);
	reg_write(EBI_DEV2_TIM_EXT(EBI_BASE), 0xFFF00000);

	return 0;
}

/*
 * Accessor functions replacing the "weak" functions in
 * drivers/mtd/cfi_flash.c
 */
void flash_write8(u8 value, void *addr)
{
	ebi_write_u16((u32)addr, value, 0);
}

void flash_write16(u16 value, void *addr)
{
	ebi_write_u16((u32)addr, value, 0);
}

u8 flash_read8(void *addr)
{
	return ebi_read_u8((u32)addr);
}

u16 flash_read16(void *addr)
{
	return ebi_read_u16((u32)addr);
}

u32 flash_read32(void *addr)
{
	return ((u32)ebi_read_u16((u32)addr) << 16) |
		ebi_read_u16((u32)addr + 2);
}

void *board_flash_read_memcpy(void *dest, const void *src, size_t count)
{
	u16 *tmp = (u16 *)dest, *s = (u16 *)src;
	int i;

	for (i = 0; i < count; i += 2)
		*tmp++ = flash_read16(s++);

	return dest;
}
