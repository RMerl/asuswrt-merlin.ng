/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2009-2011 Freescale Semiconductor, Inc.
 */

#ifndef _ASM_CONFIG_H_
#define _ASM_CONFIG_H_

#ifdef CONFIG_MPC85xx
#include <asm/config_mpc85xx.h>
#endif

#ifndef HWCONFIG_BUFFER_SIZE
  #define HWCONFIG_BUFFER_SIZE 256
#endif

#ifndef CFG_MAX_MEM_MAPPED
#if	defined(CONFIG_E500)		|| \
	defined(CONFIG_MPC86xx)		|| \
	defined(CONFIG_E300)
#define CFG_MAX_MEM_MAPPED	((phys_size_t)2 << 30)
#else
#define CFG_MAX_MEM_MAPPED	(256 << 20)
#endif
#endif

/*
 * Provide a default boot page translation virtual address that lines up with
 * Freescale's default e500 reset page.
 */
#if (defined(CONFIG_E500) && defined(CONFIG_MP))
#define BPTR_VIRT_ADDR	0xfffff000
#endif

/* The TSEC driver uses the PHYLIB infrastructure */
#if defined(CONFIG_TSEC_ENET) && defined(CONFIG_PHYLIB)
#include <config_phylib_all_drivers.h>
#endif /* TSEC_ENET */

/* The FMAN driver uses the PHYLIB infrastructure */

#if defined(CONFIG_DM_SERIAL) && !defined(CONFIG_CLK_MPC83XX)
/*
 * TODO: Convert this to a clock driver exists that can give us the UART
 * clock here.
 */
#define CFG_SYS_NS16550_CLK		get_serial_clock()
#endif

#endif /* _ASM_CONFIG_H_ */
