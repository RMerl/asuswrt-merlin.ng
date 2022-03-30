/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2014 Google, Inc
 */

struct tegra_spi_platdata {
	enum periph_id periph_id;
	int frequency;		/* Default clock frequency, -1 for none */
	ulong base;
	uint deactivate_delay_us;	/* Delay to wait after deactivate */
};
