/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * clk-synthesizer.h
 *
 * Clock synthesizer header
 *
 * Copyright (C) 2016, Texas Instruments, Incorporated - http://www.ti.com/
 */

#ifndef __CLK_SYNTHESIZER_H
#define __CLK_SYNTHESIZER_H

#include <common.h>

#define CLK_SYNTHESIZER_ID_REG		0x0
#define CLK_SYNTHESIZER_XCSEL		0x05
#define CLK_SYNTHESIZER_MUX_REG		0x14
#define CLK_SYNTHESIZER_PDIV2_REG	0x16
#define CLK_SYNTHESIZER_PDIV3_REG	0x17

#define CLK_SYNTHESIZER_BYTE_MODE	0x80

/**
 * struct clk_synth: This structure holds data neeed for configuring
 *		     for clock synthesizer.
 * @id: The id of synthesizer
 * @capacitor: value of the capacitor attached
 * @mux: mux settings.
 * @pdiv2: Div to be applied to second output
 * @pdiv3: Div to be applied to third output
 */
struct clk_synth {
	u32 id;
	u32 capacitor;
	u32 mux;
	u32 pdiv2;
	u32 pdiv3;
};

int setup_clock_synthesizer(struct clk_synth *data);

#endif
