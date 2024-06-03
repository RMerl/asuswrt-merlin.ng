/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2011
 * Linaro
 * Linus Walleij <linus.walleij@linaro.org>
 * Register definitions for the External Bus Interface (EBI)
 * found in the ARM Integrator AP and CP reference designs
 */

#ifndef __ARM_EBI_H
#define __ARM_EBI_H

#define EBI_BASE		0x12000000

#define EBI_CSR0_REG		0x00 /* CS0 = Boot ROM */
#define EBI_CSR1_REG		0x04 /* CS1 = Flash */
#define EBI_CSR2_REG		0x08 /* CS2 = SSRAM */
#define EBI_CSR3_REG		0x0C /* CS3 = Expansion memory */
/*
 * The four upper bits are the waitstates for each chip select
 * 0x00 = 2 cycles, 0x10 = 3 cycles, ... 0xe0 = 16 cycles, 0xf0 = 16 cycles
 */
#define EBI_CSR_WAIT_MASK	0xF0
/* Whether memory is synchronous or asynchronous */
#define EBI_CSR_SYNC_MASK	0xF7
#define EBI_CSR_ASYNC		0x00
#define EBI_CSR_SYNC		0x08
/* Whether memory is write enabled or not */
#define EBI_CSR_WREN_MASK	0xFB
#define EBI_CSR_WREN_DISABLE	0x00
#define EBI_CSR_WREN_ENABLE	0x04
/* Memory bit width for each chip select */
#define EBI_CSR_MEMSIZE_MASK	0xFC
#define EBI_CSR_MEMSIZE_8BIT	0x00
#define EBI_CSR_MEMSIZE_16BIT	0x01
#define EBI_CSR_MEMSIZE_32BIT	0x02

/*
 * The lock register need to be written with 0xa05f before anything in the
 * EBI can be changed.
 */
#define EBI_LOCK_REG		0x20
#define EBI_UNLOCK_MAGIC	0xA05F

#endif
