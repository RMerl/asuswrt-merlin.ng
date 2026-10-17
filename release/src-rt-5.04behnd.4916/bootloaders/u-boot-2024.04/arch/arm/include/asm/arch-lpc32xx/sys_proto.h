/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2011 Vladimir Zapolskiy <vz@mleia.com>
 */

#ifndef _LPC32XX_SYS_PROTO_H
#define _LPC32XX_SYS_PROTO_H

#include <asm/arch/emc.h>

void lpc32xx_uart_init(unsigned int uart_id);
void lpc32xx_dma_init(void);
void lpc32xx_mac_init(void);
void lpc32xx_mlc_nand_init(void);
void lpc32xx_slc_nand_init(void);
void lpc32xx_i2c_init(unsigned int devnum);
void lpc32xx_ssp_init(void);
void lpc32xx_usb_init(void);
#if defined(CONFIG_SPL_BUILD)
void ddr_init(const struct emc_dram_settings *dram);
#endif
#endif /* _LPC32XX_SYS_PROTO_H */
