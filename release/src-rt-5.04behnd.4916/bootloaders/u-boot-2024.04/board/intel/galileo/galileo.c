// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2015, Bin Meng <bmeng.cn@gmail.com>
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/device.h>
#include <asm/arch/quark.h>

/*
 * Intel Galileo gen2 board uses GPIO Resume Well bank pin0 as the PERST# pin.
 *
 * We cannot use any public GPIO APIs in <asm-generic/gpio.h> to control this
 * pin, as these APIs will eventually call into gpio_ich6_of_to_plat()
 * in the Intel ICH6 GPIO driver where it calls PCI configuration space access
 * APIs which will trigger PCI enumeration process.
 *
 * Check <asm/arch-quark/quark.h> for more details.
 */
void board_assert_perst(void)
{
	u32 base, port, val;

	/* retrieve the GPIO IO base */
	qrk_pci_read_config_dword(QUARK_LEGACY_BRIDGE, LB_GBA, &base);
	base = (base & 0xffff) & ~0x7f;

	/* enable the pin */
	port = base + 0x20;
	val = inl(port);
	val |= (1 << 0);
	outl(val, port);

	/* configure the pin as output */
	port = base + 0x24;
	val = inl(port);
	val &= ~(1 << 0);
	outl(val, port);

	/* pull it down (assert) */
	port = base + 0x28;
	val = inl(port);
	val &= ~(1 << 0);
	outl(val, port);
}

void board_deassert_perst(void)
{
	u32 base, port, val;

	/* retrieve the GPIO IO base */
	qrk_pci_read_config_dword(QUARK_LEGACY_BRIDGE, LB_GBA, &base);
	base = (base & 0xffff) & ~0x7f;

	/* pull it up (de-assert) */
	port = base + 0x28;
	val = inl(port);
	val |= (1 << 0);
	outl(val, port);
}
