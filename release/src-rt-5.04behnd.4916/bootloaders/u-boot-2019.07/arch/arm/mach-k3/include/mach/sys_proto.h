/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2018 Texas Instruments Incorporated - http://www.ti.com/
 *	Andreas Dannenberg <dannenberg@ti.com>
 */

#ifndef _SYS_PROTO_H_
#define _SYS_PROTO_H_

void sdelay(unsigned long loops);
u32 wait_on_value(u32 read_bit_mask, u32 match_value, void *read_addr,
		  u32 bound);
struct ti_sci_handle *get_ti_sci_handle(void);
int fdt_fixup_msmc_ram(void *blob, char *parent_path, char *node_name);
#endif
