/* SPDX-License-Identifier: GPL-2.0-or-later OR BSD-3-Clause */
/*
 * Copyright (C) STMicroelectronics 2022 - All Rights Reserved
 * Author: Gabriel Fernandez <gabriel.fernandez@foss.st.com> for STMicroelectronics.
 */

struct stm32_clock_match_data;

/**
 * struct stm32_mux_cfg - multiplexer configuration
 *
 * @parent_names:	array of string names for all possible parents
 * @num_parents:	number of possible parents
 * @reg_off:		register controlling multiplexer
 * @shift:		shift to multiplexer bit field
 * @width:		width of the multiplexer bit field
 * @mux_flags:		hardware-specific flags
 * @table:		array of register values corresponding to the parent
 *			index
 */
struct stm32_mux_cfg {
	const char * const *parent_names;
	u8 num_parents;
	u32 reg_off;
	u8 shift;
	u8 width;
	u8 mux_flags;
	u32 *table;
};

/**
 * struct stm32_gate_cfg - gating configuration
 *
 * @reg_off:	register controlling gate
 * @bit_idx:	single bit controlling gate
 * @gate_flags:	hardware-specific flags
 * @set_clr:	0 : normal gate, 1 : has a register to clear the gate
 */
struct stm32_gate_cfg {
	u32 reg_off;
	u8 bit_idx;
	u8 gate_flags;
	u8 set_clr;
};

/**
 * struct stm32_div_cfg - divider configuration
 *
 * @reg_off:	register containing the divider
 * @shift:	shift to the divider bit field
 * @width:	width of the divider bit field
 * @table:	array of value/divider pairs, last entry should have div = 0
 */
struct stm32_div_cfg {
	u32 reg_off;
	u8 shift;
	u8 width;
	u8 div_flags;
	const struct clk_div_table *table;
};

#define NO_STM32_MUX	-1
#define NO_STM32_DIV	-1
#define NO_STM32_GATE	-1

/**
 * struct stm32_composite_cfg - composite configuration
 *
 * @mux:	index of a multiplexer
 * @gate:	index of a gate
 * @div:	index of a divider
 */
struct stm32_composite_cfg {
	int mux;
	int gate;
	int div;
};

/**
 * struct clock_config - clock configuration
 *
 * @id:			binding id of the clock
 * @name:		clock name
 * @parent_name:	name of the clock parent
 * @flags:		framework-specific flags
 * @sec_id:		secure id (use to known if the clock is secured or not)
 * @clock_cfg:		specific clock data configuration
 * @setup:		specific call back to reister the clock (will use
 *			clock_cfg data as input)
 */
struct clock_config {
	unsigned long id;
	const char *name;
	const char *parent_name;
	unsigned long flags;
	int sec_id;
	void *clock_cfg;

	struct clk *(*setup)(struct udevice *dev,
			     const struct clock_config *cfg);
};

/**
 * struct clk_stm32_clock_data - clock data
 *
 * @num_gates:		number of defined gates
 * @gates:		array of gate configuration
 * @muxes:		array of multiplexer configuration
 * @dividers:		array of divider configuration
 */
struct clk_stm32_clock_data {
	unsigned int num_gates;
	const struct stm32_gate_cfg *gates;
	const struct stm32_mux_cfg *muxes;
	const struct stm32_div_cfg *dividers;
};

/**
 * struct stm32_clock_match_data - clock match data
 *
 * @num_gates:		number of clocks
 * @tab_clocks:		array of clock configuration
 * @clock_data:		definition of all gates / dividers / multiplexers
 * @check_security:	call back to check if clock is secured or not
 */
struct stm32_clock_match_data {
	unsigned int num_clocks;
	const struct clock_config *tab_clocks;
	const struct clk_stm32_clock_data *clock_data;
	int (*check_security)(void __iomem *base,
			      const struct clock_config *cfg);
};

/**
 * struct stm32mp_rcc_priv - private struct for stm32mp clocks
 *
 * @base:	base register of RCC driver
 * @gate_cpt:	array of refcounting for gate with more than one
 *		clocks as input. See explanation of Peripheral clock enabling
 *              below.
 * @data:	data for gate / divider / multiplexer configuration
 */
struct stm32mp_rcc_priv {
	void __iomem *base;
	u8 *gate_cpt;
	const struct clk_stm32_clock_data *data;
};

int stm32_rcc_init(struct udevice *dev,
		   const struct stm32_clock_match_data *data);

/**
 * STM32 Gate
 *
 *               PCE (Peripheral Clock Enabling)                  Peripheral
 *
 *                ------------------------------                   ----------
 *               |                              |                 |          |
 *               |                              |                 |   PERx   |
 * bus_ck        |                   -----      |                 |          |
 * ------------->|------------------|     |     |  ckg_bus_perx   |          |
 *               |                  | AND |-----|---------------->|          |
 *               |       -----------|     |     |                 |          |
 *               |      |            -----      |                 |          |
 *               |      |                       |                 |          |
 *               |    -----                     |                 |          |
 * Perx_EN |-----|---| GCL |  Gating            |                 |          |
 *               |    -----   Control           |                 |          |
 *               |      |     Logic             |                 |          |
 *               |      |                       |                 |          |
 *               |      |            -----      |                 |          |
 *               |       -----------|     |     |  ckg_ker_perx   |          |
 * perx_ker_ck   |                  | AND |-----|---------------->|          |
 * ------------->|------------------|     |     |                 |          |
 *               |                   -----      |                 |          |
 *               |                              |                 |          |
 *               |                              |                 |          |
 *                ------------------------------                   ----------

 * Each peripheral requires a bus interface clock, named ckg_bus_perx
 * (for peripheral ‘x’).
 * Some peripherals (SAI, UART...) need also a dedicated clock for their
 * communication interface, this clock is generally asynchronous with respect to
 * the bus interface clock, and is named kernel clock (ckg_ker_perx).

 * Both clocks can be gated by one Perx_EN enable bit.
 * Then we have to manage a refcounting on gate level to avoid gate if one
 * the bus or the Kernel was enable.
 *
 * Example:
 * 1) enable the bus clock
 *	--> bus_clk ref_counting = 1, gate_ref_count = 1
 * 2) enable the kernel clock
 *	--> perx_ker_ck ref_counting = 1, gate_ref_count = 2
 * 3) disable kernel clock
 * 	---> perx_ker_ck ref_counting = 0, gate_ref_count = 1
 * 	==> then i will not gate because gate_ref_count > 0
 * 4) disable bus clock
 *	--> bus_clk  ref_counting  = 0, gate_ref_count = 0
 *	==> then i can gate (write in the register) because
 *	    gate_ref_count = 0
 */

struct clk_stm32_gate {
	struct clk clk;
	struct stm32mp_rcc_priv *priv;
	int gate_id;
};

#define to_clk_stm32_gate(_clk) container_of(_clk, struct clk_stm32_gate, clk)

struct clk *
clk_stm32_gate_register(struct udevice *dev,
			const struct clock_config *cfg);

struct clk *
clk_stm32_register_composite(struct udevice *dev,
			     const struct clock_config *cfg);

struct stm32_clk_gate_cfg {
	int gate_id;
};

#define STM32_GATE(_id, _name, _parent, _flags, _gate_id, _sec_id) \
{ \
	.id		= _id, \
	.sec_id		= _sec_id, \
	.name		= _name, \
	.parent_name	= _parent, \
	.flags		= _flags, \
	.clock_cfg	= &(struct stm32_clk_gate_cfg) { \
		.gate_id	= _gate_id, \
	}, \
	.setup		= clk_stm32_gate_register, \
}

struct stm32_clk_composite_cfg {
	int	gate_id;
	int	mux_id;
	int	div_id;
};

#define STM32_COMPOSITE(_id, _name, _flags, _sec_id, \
			_gate_id, _mux_id, _div_id) \
{ \
	.id		= _id, \
	.name		= _name, \
	.sec_id		= _sec_id, \
	.flags		= _flags, \
	.clock_cfg	= &(struct stm32_clk_composite_cfg) { \
		.gate_id	= _gate_id, \
		.mux_id		= _mux_id, \
		.div_id		= _div_id, \
	}, \
	.setup		= clk_stm32_register_composite, \
}

#define STM32_COMPOSITE_NOMUX(_id, _name, _parent, _flags, _sec_id, \
			      _gate_id, _div_id) \
{ \
	.id		= _id, \
	.name		= _name, \
	.parent_name	= _parent, \
	.sec_id		= _sec_id, \
	.flags		= _flags, \
	.clock_cfg	= &(struct stm32_clk_composite_cfg) { \
		.gate_id	= _gate_id, \
		.mux_id		= NO_STM32_MUX, \
		.div_id		= _div_id, \
	}, \
	.setup		= clk_stm32_register_composite, \
}

extern const struct clk_ops stm32_clk_ops;

ulong clk_stm32_get_rate_by_name(const char *name);
