/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2014, Bin Meng <bmeng.cn@gmail.com>
 *
 * Adapted from coreboot src/include/device/pnp_def.h
 * and arch/x86/include/arch/io.h
 */

#ifndef _ASM_PNP_DEF_H_
#define _ASM_PNP_DEF_H_

#include <asm/io.h>

#define PNP_IDX_EN   0x30
#define PNP_IDX_IO0  0x60
#define PNP_IDX_IO1  0x62
#define PNP_IDX_IO2  0x64
#define PNP_IDX_IO3  0x66
#define PNP_IDX_IRQ0 0x70
#define PNP_IDX_IRQ1 0x72
#define PNP_IDX_DRQ0 0x74
#define PNP_IDX_DRQ1 0x75
#define PNP_IDX_MSC0 0xf0
#define PNP_IDX_MSC1 0xf1

/* Generic functions for pnp devices */

/*
 * pnp device is a 16-bit integer composed of its i/o port address at high byte
 * and logic function number at low byte.
 */
#define PNP_DEV(PORT, FUNC) (((PORT) << 8) | (FUNC))

static inline void pnp_write_config(uint16_t dev, uint8_t reg, uint8_t value)
{
	uint8_t port = dev >> 8;

	outb(reg, port);
	outb(value, port + 1);
}

static inline uint8_t pnp_read_config(uint16_t dev, uint8_t reg)
{
	uint8_t port = dev >> 8;

	outb(reg, port);
	return inb(port + 1);
}

static inline void pnp_set_logical_device(uint16_t dev)
{
	uint8_t device = dev & 0xff;

	pnp_write_config(dev, 0x07, device);
}

static inline void pnp_set_enable(uint16_t dev, int enable)
{
	pnp_write_config(dev, PNP_IDX_EN, enable ? 1 : 0);
}

static inline int pnp_read_enable(uint16_t dev)
{
	return !!pnp_read_config(dev, PNP_IDX_EN);
}

static inline void pnp_set_iobase(uint16_t dev, uint8_t index, uint16_t iobase)
{
	pnp_write_config(dev, index + 0, (iobase >> 8) & 0xff);
	pnp_write_config(dev, index + 1, iobase & 0xff);
}

static inline uint16_t pnp_read_iobase(uint16_t dev, uint8_t index)
{
	return ((uint16_t)(pnp_read_config(dev, index)) << 8) |
		pnp_read_config(dev, index + 1);
}

static inline void pnp_set_irq(uint16_t dev, uint8_t index, unsigned irq)
{
	pnp_write_config(dev, index, irq);
}

static inline void pnp_set_drq(uint16_t dev, uint8_t index, unsigned drq)
{
	pnp_write_config(dev, index, drq & 0xff);
}

#endif /* _ASM_PNP_DEF_H_ */
