// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2016, Bin Meng <bmeng.cn@gmail.com>
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <init.h>
#include <pci.h>
#include <smsc_sio1007.h>
#include <asm/ibmpc.h>
#include <asm/lpc_common.h>
#include <asm/pci.h>
#include <asm/arch/pch.h>

#define SIO1007_RUNTIME_IOPORT	0x180

int board_early_init_f(void)
{
	struct udevice *pch;
	int ret;

	ret = uclass_first_device_err(UCLASS_PCH, &pch);
	if (ret)
		return ret;

	/* Initialize LPC interface to turn on superio chipset decode range */
	dm_pci_write_config16(pch, LPC_IO_DEC, COMA_DEC_RANGE | COMB_DEC_RANGE);
	dm_pci_write_config16(pch, LPC_EN, KBC_LPC_EN | COMA_LPC_EN);
	dm_pci_write_config32(pch, LPC_GEN1_DEC, GEN_DEC_RANGE_256B |
			      (SIO1007_IOPORT3 & 0xff00) | GEN_DEC_RANGE_EN);
	dm_pci_write_config32(pch, LPC_GEN2_DEC, GEN_DEC_RANGE_16B |
			      SIO1007_RUNTIME_IOPORT | GEN_DEC_RANGE_EN);

	/* Enable legacy serial port at 0x3f8 */
	sio1007_enable_serial(SIO1007_IOPORT3, 0, UART0_BASE, UART0_IRQ);

	/* Enable SIO1007 runtime I/O port at 0x180 */
	sio1007_enable_runtime(SIO1007_IOPORT3, SIO1007_RUNTIME_IOPORT);

	/*
	 * On Cougar Canyon 2 board, the RS232 transiver connected to serial
	 * port 0 (0x3f8) is controlled by a GPIO pin (GPIO10) on the SIO1007.
	 * Set the pin value to 1 to enable the RS232 transiver.
	 */
	sio1007_gpio_config(SIO1007_IOPORT3, 0, GPIO_DIR_OUTPUT,
			    GPIO_POL_NO_INVERT, GPIO_TYPE_PUSH_PULL);
	sio1007_gpio_set_value(SIO1007_RUNTIME_IOPORT, 0, 1);

	return 0;
}
