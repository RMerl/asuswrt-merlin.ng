/*
 * mux.c
 *
 * Copyright (C) 2011 Texas Instruments Incorporated - http://www.ti.com/
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation version 2.
 *
 * This program is distributed "as is" WITHOUT ANY WARRANTY of any
 * kind, whether express or implied; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include <common.h>
#include <asm/arch/mux.h>
#include <asm/arch/hardware.h>
#include <asm/io.h>

/*
 * Configure the pin mux for the module
 */
void configure_module_pin_mux(struct module_pin_mux *mod_pin_mux)
{
	int i;

	if (!mod_pin_mux)
		return;

	for (i = 0; mod_pin_mux[i].reg_offset != -1; i++)
		MUX_CFG(mod_pin_mux[i].val, mod_pin_mux[i].reg_offset);
}

/*
 * provide a default over-writable definition
*/
void __weak set_uart_mux_conf(void)
{
}

/*
* provide a default over-writable definition
*/
void __weak set_mux_conf_regs(void)
{
}
