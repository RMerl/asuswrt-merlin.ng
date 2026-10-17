/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2008 Lyrtech <www.lyrtech.com>
 */

#ifndef __MISC_H
#define __MISC_H

#include <asm/arch/hardware.h>

/* pin muxer definitions */
#define PIN_MUX_NUM_FIELDS	8	/* Per register */
#define PIN_MUX_FIELD_SIZE	4	/* n in bits */
#define PIN_MUX_FIELD_MASK	((1 << PIN_MUX_FIELD_SIZE) - 1)

/* pin definition */
struct pinmux_config {
	dv_reg		*mux;		/* Address of mux register */
	unsigned char	value;		/* Value to set in field */
	unsigned char	field;		/* field number */
};

/* pin table definition */
struct pinmux_resource {
	const struct pinmux_config	*pins;
	const int			n_pins;
};

#define PINMUX_ITEM(item) { \
				.pins = item, \
				.n_pins = ARRAY_SIZE(item) \
			  }

struct lpsc_resource {
	const int	lpsc_no;
};

void davinci_sync_env_enetaddr(uint8_t *rom_enetaddr);
int davinci_configure_pin_mux(const struct pinmux_config *pins, int n_pins);
int davinci_configure_pin_mux_items(const struct pinmux_resource *item,
				    int n_items);
#if defined(CONFIG_DRIVER_TI_EMAC)
void davinci_emac_mii_mode_sel(int mode_sel);
#endif
void irq_init(void);
int da8xx_configure_lpsc_items(const struct lpsc_resource *item,
				    const int n_items);

#endif /* __MISC_H */
