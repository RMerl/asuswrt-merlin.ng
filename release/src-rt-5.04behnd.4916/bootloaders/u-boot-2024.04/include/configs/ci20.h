/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * CI20 configuration
 *
 * Copyright (c) 2013 Imagination Technologies
 * Author: Paul Burton <paul.burton@imgtec.com>
 */

#ifndef __CONFIG_CI20_H__
#define __CONFIG_CI20_H__

/* Memory configuration */

#define CFG_SYS_SDRAM_BASE		0x80000000 /* cached (KSEG0) address */
#define CFG_SYS_INIT_SP_OFFSET	0x400000

/* NS16550-ish UARTs */
#define CFG_SYS_NS16550_CLK		48000000

#endif /* __CONFIG_CI20_H__ */
