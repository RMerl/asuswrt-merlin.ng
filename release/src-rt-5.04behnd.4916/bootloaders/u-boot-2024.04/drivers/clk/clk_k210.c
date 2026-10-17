// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2019-20 Sean Anderson <seanga2@gmail.com>
 */
#define LOG_CATEGORY UCLASS_CLK

#include <common.h>
#include <clk.h>
#include <clk-uclass.h>
#include <div64.h>
#include <dm.h>
#include <log.h>
#include <mapmem.h>
#include <serial.h>
#include <dt-bindings/clock/k210-sysctl.h>
#include <dt-bindings/mfd/k210-sysctl.h>
#include <k210/pll.h>
#include <linux/bitfield.h>
#include <asm/barrier.h>

DECLARE_GLOBAL_DATA_PTR;

/**
 * struct k210_clk_priv - K210 clock driver private data
 * @base: The base address of the sysctl device
 * @in0: The "in0" external oscillator
 */
struct k210_clk_priv {
	void __iomem *base;
	struct clk in0;
};

/*
 * All parameters for different sub-clocks are collected into parameter arrays.
 * These parameters are then initialized by the clock which uses them during
 * probe. To save space, ids are automatically generated for each sub-clock by
 * using an enum. Instead of storing a parameter struct for each clock, even for
 * those clocks which don't use a particular type of sub-clock, we can just
 * store the parameters for the clocks which need them.
 *
 * So why do it like this? Arranging all the sub-clocks together makes it very
 * easy to find bugs in the code.
 */

/**
 * enum k210_clk_div_type - The type of divider
 * @K210_DIV_ONE: freq = parent / (reg + 1)
 * @K210_DIV_EVEN: freq = parent / 2 / (reg + 1)
 * @K210_DIV_POWER: freq = parent / (2 << reg)
 * @K210_DIV_FIXED: freq = parent / factor
 */
enum k210_clk_div_type {
	K210_DIV_ONE,
	K210_DIV_EVEN,
	K210_DIV_POWER,
	K210_DIV_FIXED,
};

/**
 * struct k210_div_params - Parameters for dividing clocks
 * @type: An &enum k210_clk_div_type specifying the dividing formula
 * @off: The offset of the divider from the sysctl base address
 * @shift: The offset of the LSB of the divider
 * @width: The number of bits in the divider
 * @div: The fixed divisor for this divider
 */
struct k210_div_params {
	u8 type;
	union {
		struct {
			u8 off;
			u8 shift;
			u8 width;
		};
		u8 div;
	};
};

#define DIV_LIST \
	DIV(K210_CLK_ACLK,   K210_SYSCTL_SEL0,  1,  2, K210_DIV_POWER) \
	DIV(K210_CLK_APB0,   K210_SYSCTL_SEL0,  3,  3, K210_DIV_ONE) \
	DIV(K210_CLK_APB1,   K210_SYSCTL_SEL0,  6,  3, K210_DIV_ONE) \
	DIV(K210_CLK_APB2,   K210_SYSCTL_SEL0,  9,  3, K210_DIV_ONE) \
	DIV(K210_CLK_SRAM0,  K210_SYSCTL_THR0,  0,  4, K210_DIV_ONE) \
	DIV(K210_CLK_SRAM1,  K210_SYSCTL_THR0,  4,  4, K210_DIV_ONE) \
	DIV(K210_CLK_AI,     K210_SYSCTL_THR0,  8,  4, K210_DIV_ONE) \
	DIV(K210_CLK_DVP,    K210_SYSCTL_THR0, 12,  4, K210_DIV_ONE) \
	DIV(K210_CLK_ROM,    K210_SYSCTL_THR0, 16,  4, K210_DIV_ONE) \
	DIV(K210_CLK_SPI0,   K210_SYSCTL_THR1,  0,  8, K210_DIV_EVEN) \
	DIV(K210_CLK_SPI1,   K210_SYSCTL_THR1,  8,  8, K210_DIV_EVEN) \
	DIV(K210_CLK_SPI2,   K210_SYSCTL_THR1, 16,  8, K210_DIV_EVEN) \
	DIV(K210_CLK_SPI3,   K210_SYSCTL_THR1, 24,  8, K210_DIV_EVEN) \
	DIV(K210_CLK_TIMER0, K210_SYSCTL_THR2,  0,  8, K210_DIV_EVEN) \
	DIV(K210_CLK_TIMER1, K210_SYSCTL_THR2,  8,  8, K210_DIV_EVEN) \
	DIV(K210_CLK_TIMER2, K210_SYSCTL_THR2, 16,  8, K210_DIV_EVEN) \
	DIV(K210_CLK_I2S0,   K210_SYSCTL_THR3,  0, 16, K210_DIV_EVEN) \
	DIV(K210_CLK_I2S1,   K210_SYSCTL_THR3, 16, 16, K210_DIV_EVEN) \
	DIV(K210_CLK_I2S2,   K210_SYSCTL_THR4,  0, 16, K210_DIV_EVEN) \
	DIV(K210_CLK_I2S0_M, K210_SYSCTL_THR4, 16,  8, K210_DIV_EVEN) \
	DIV(K210_CLK_I2S1_M, K210_SYSCTL_THR4, 24,  8, K210_DIV_EVEN) \
	DIV(K210_CLK_I2S2_M, K210_SYSCTL_THR4,  0,  8, K210_DIV_EVEN) \
	DIV(K210_CLK_I2C0,   K210_SYSCTL_THR5,  8,  8, K210_DIV_EVEN) \
	DIV(K210_CLK_I2C1,   K210_SYSCTL_THR5, 16,  8, K210_DIV_EVEN) \
	DIV(K210_CLK_I2C2,   K210_SYSCTL_THR5, 24,  8, K210_DIV_EVEN) \
	DIV(K210_CLK_WDT0,   K210_SYSCTL_THR6,  0,  8, K210_DIV_EVEN) \
	DIV(K210_CLK_WDT1,   K210_SYSCTL_THR6,  8,  8, K210_DIV_EVEN) \
	DIV_FIXED(K210_CLK_CLINT, 50) \

#define _DIVIFY(id) K210_CLK_DIV_##id
#define DIVIFY(id) _DIVIFY(id)

enum k210_div_id {
#define DIV(id, ...) DIVIFY(id),
#define DIV_FIXED DIV
	DIV_LIST
#undef DIV
#undef DIV_FIXED
	K210_CLK_DIV_NONE,
};

static const struct k210_div_params k210_divs[] = {
#define DIV(id, _off, _shift, _width, _type) \
	[DIVIFY(id)] = { \
		.type = (_type), \
		.off = (_off), \
		.shift = (_shift), \
		.width = (_width), \
	},
#define DIV_FIXED(id, _div) \
	[DIVIFY(id)] = { \
		.type = K210_DIV_FIXED, \
		.div = (_div) \
	},
	DIV_LIST
#undef DIV
#undef DIV_FIXED
};

#undef DIV
#undef DIV_LIST

/**
 * struct k210_gate_params - Parameters for gated clocks
 * @off: The offset of the gate from the sysctl base address
 * @bit_idx: The index of the bit within the register
 */
struct k210_gate_params {
	u8 off;
	u8 bit_idx;
};

#define GATE_LIST \
	GATE(K210_CLK_CPU,    K210_SYSCTL_EN_CENT,  0) \
	GATE(K210_CLK_SRAM0,  K210_SYSCTL_EN_CENT,  1) \
	GATE(K210_CLK_SRAM1,  K210_SYSCTL_EN_CENT,  2) \
	GATE(K210_CLK_APB0,   K210_SYSCTL_EN_CENT,  3) \
	GATE(K210_CLK_APB1,   K210_SYSCTL_EN_CENT,  4) \
	GATE(K210_CLK_APB2,   K210_SYSCTL_EN_CENT,  5) \
	GATE(K210_CLK_ROM,    K210_SYSCTL_EN_PERI,  0) \
	GATE(K210_CLK_DMA,    K210_SYSCTL_EN_PERI,  1) \
	GATE(K210_CLK_AI,     K210_SYSCTL_EN_PERI,  2) \
	GATE(K210_CLK_DVP,    K210_SYSCTL_EN_PERI,  3) \
	GATE(K210_CLK_FFT,    K210_SYSCTL_EN_PERI,  4) \
	GATE(K210_CLK_GPIO,   K210_SYSCTL_EN_PERI,  5) \
	GATE(K210_CLK_SPI0,   K210_SYSCTL_EN_PERI,  6) \
	GATE(K210_CLK_SPI1,   K210_SYSCTL_EN_PERI,  7) \
	GATE(K210_CLK_SPI2,   K210_SYSCTL_EN_PERI,  8) \
	GATE(K210_CLK_SPI3,   K210_SYSCTL_EN_PERI,  9) \
	GATE(K210_CLK_I2S0,   K210_SYSCTL_EN_PERI, 10) \
	GATE(K210_CLK_I2S1,   K210_SYSCTL_EN_PERI, 11) \
	GATE(K210_CLK_I2S2,   K210_SYSCTL_EN_PERI, 12) \
	GATE(K210_CLK_I2C0,   K210_SYSCTL_EN_PERI, 13) \
	GATE(K210_CLK_I2C1,   K210_SYSCTL_EN_PERI, 14) \
	GATE(K210_CLK_I2C2,   K210_SYSCTL_EN_PERI, 15) \
	GATE(K210_CLK_UART1,  K210_SYSCTL_EN_PERI, 16) \
	GATE(K210_CLK_UART2,  K210_SYSCTL_EN_PERI, 17) \
	GATE(K210_CLK_UART3,  K210_SYSCTL_EN_PERI, 18) \
	GATE(K210_CLK_AES,    K210_SYSCTL_EN_PERI, 19) \
	GATE(K210_CLK_FPIOA,  K210_SYSCTL_EN_PERI, 20) \
	GATE(K210_CLK_TIMER0, K210_SYSCTL_EN_PERI, 21) \
	GATE(K210_CLK_TIMER1, K210_SYSCTL_EN_PERI, 22) \
	GATE(K210_CLK_TIMER2, K210_SYSCTL_EN_PERI, 23) \
	GATE(K210_CLK_WDT0,   K210_SYSCTL_EN_PERI, 24) \
	GATE(K210_CLK_WDT1,   K210_SYSCTL_EN_PERI, 25) \
	GATE(K210_CLK_SHA,    K210_SYSCTL_EN_PERI, 26) \
	GATE(K210_CLK_OTP,    K210_SYSCTL_EN_PERI, 27) \
	GATE(K210_CLK_RTC,    K210_SYSCTL_EN_PERI, 29)

#define _GATEIFY(id) K210_CLK_GATE_##id
#define GATEIFY(id) _GATEIFY(id)

enum k210_gate_id {
#define GATE(id, ...) GATEIFY(id),
	GATE_LIST
#undef GATE
	K210_CLK_GATE_NONE,
};

static const struct k210_gate_params k210_gates[] = {
#define GATE(id, _off, _idx) \
	[GATEIFY(id)] = { \
		.off = (_off), \
		.bit_idx = (_idx), \
	},
	GATE_LIST
#undef GATE
};

#undef GATE_LIST

/* The most parents is PLL2 */
#define K210_CLK_MAX_PARENTS 3

/**
 * struct k210_mux_params - Parameters for muxed clocks
 * @parents: A list of parent clock ids
 * @num_parents: The number of parent clocks
 * @off: The offset of the mux from the base sysctl address
 * @shift: The offset of the LSB of the mux selector
 * @width: The number of bits in the mux selector
 */
struct k210_mux_params {
	u8 parents[K210_CLK_MAX_PARENTS];
	u8 num_parents;
	u8 off;
	u8 shift;
	u8 width;
};

#define MUX(id, reg, shift, width) \
	MUX_PARENTS(id, reg, shift, width, K210_CLK_IN0, K210_CLK_PLL0)
#define MUX_LIST \
	MUX_PARENTS(K210_CLK_PLL2, K210_SYSCTL_PLL2, 26, 2, \
		    K210_CLK_IN0, K210_CLK_PLL0, K210_CLK_PLL1) \
	MUX(K210_CLK_ACLK, K210_SYSCTL_SEL0, 0, 1) \
	MUX(K210_CLK_SPI3,   K210_SYSCTL_SEL0, 12, 1) \
	MUX(K210_CLK_TIMER0, K210_SYSCTL_SEL0, 13, 1) \
	MUX(K210_CLK_TIMER1, K210_SYSCTL_SEL0, 14, 1) \
	MUX(K210_CLK_TIMER2, K210_SYSCTL_SEL0, 15, 1)

#define _MUXIFY(id) K210_CLK_MUX_##id
#define MUXIFY(id) _MUXIFY(id)

enum k210_mux_id {
#define MUX_PARENTS(id, ...) MUXIFY(id),
	MUX_LIST
#undef MUX_PARENTS
	K210_CLK_MUX_NONE,
};

static const struct k210_mux_params k210_muxes[] = {
#define MUX_PARENTS(id, _off, _shift, _width, ...) \
	[MUXIFY(id)] = { \
		.parents = { __VA_ARGS__ }, \
		.num_parents = __count_args(__VA_ARGS__), \
		.off = (_off), \
		.shift = (_shift), \
		.width = (_width), \
	},
	MUX_LIST
#undef MUX_PARENTS
};

#undef MUX
#undef MUX_LIST

/**
 * struct k210_pll_params - K210 PLL parameters
 * @off: The offset of the PLL from the base sysctl address
 * @shift: The offset of the LSB of the lock status
 * @width: The number of bits in the lock status
 */
struct k210_pll_params {
	u8 off;
	u8 shift;
	u8 width;
};

static const struct k210_pll_params k210_plls[] = {
#define PLL(_off, _shift, _width) { \
	.off = (_off), \
	.shift = (_shift), \
	.width = (_width), \
}
	[0] = PLL(K210_SYSCTL_PLL0,  0, 2),
	[1] = PLL(K210_SYSCTL_PLL1,  8, 1),
	[2] = PLL(K210_SYSCTL_PLL2, 16, 1),
#undef PLL
};

/**
 * enum k210_clk_flags - The type of a K210 clock
 * @K210_CLKF_MUX: This clock has a mux and not a static parent
 * @K210_CLKF_PLL: This clock is a PLL
 */
enum k210_clk_flags {
	K210_CLKF_MUX = BIT(0),
	K210_CLKF_PLL = BIT(1),
};

/**
 * struct k210_clk_params - The parameters defining a K210 clock
 * @name: The name of the clock
 * @flags: A set of &enum k210_clk_flags defining which fields are valid
 * @mux: An &enum k210_mux_id of this clock's mux
 * @parent: The clock id of this clock's parent
 * @pll: The id of the PLL (if this clock is a PLL)
 * @div: An &enum k210_div_id of this clock's divider
 * @gate: An &enum k210_gate_id of this clock's gate
 */
struct k210_clk_params {
#if IS_ENABLED(CONFIG_CMD_CLK)
	const char *name;
#endif
	u8 flags;
	union {
		u8 parent;
		u8 mux;
	};
	union {
		u8 pll;
		struct {
			u8 div;
			u8 gate;
		};
	};
};

static const struct k210_clk_params k210_clks[] = {
#if IS_ENABLED(CONFIG_CMD_CLK)
#define NAME(_name) .name = (_name),
#else
#define NAME(name)
#endif
#define CLK(id, _name, _parent, _div, _gate) \
	[id] = { \
		NAME(_name) \
		.parent = (_parent), \
		.div = (_div), \
		.gate = (_gate), \
	}
#define CLK_MUX(id, _name, _mux, _div, _gate) \
	[id] = { \
		NAME(_name) \
		.flags = K210_CLKF_MUX, \
		.mux = (_mux), \
		.div = (_div), \
		.gate = (_gate), \
	}
#define CLK_PLL(id, _pll, _parent) \
	[id] = { \
		NAME("pll" #_pll) \
		.flags = K210_CLKF_PLL, \
		.parent = (_parent), \
		.pll = (_pll), \
	}
#define CLK_FULL(id, name) \
	CLK_MUX(id, name, MUXIFY(id), DIVIFY(id), GATEIFY(id))
#define CLK_NOMUX(id, name, parent) \
	CLK(id, name, parent, DIVIFY(id), GATEIFY(id))
#define CLK_DIV(id, name, parent) \
	CLK(id, name, parent, DIVIFY(id), K210_CLK_GATE_NONE)
#define CLK_GATE(id, name, parent) \
	CLK(id, name, parent, K210_CLK_DIV_NONE, GATEIFY(id))
	CLK_PLL(K210_CLK_PLL0, 0, K210_CLK_IN0),
	CLK_PLL(K210_CLK_PLL1, 1, K210_CLK_IN0),
	[K210_CLK_PLL2] = {
		NAME("pll2")
		.flags = K210_CLKF_MUX | K210_CLKF_PLL,
		.mux = MUXIFY(K210_CLK_PLL2),
		.pll = 2,
	},
	CLK_MUX(K210_CLK_ACLK, "aclk", MUXIFY(K210_CLK_ACLK),
		DIVIFY(K210_CLK_ACLK), K210_CLK_GATE_NONE),
	CLK_FULL(K210_CLK_SPI3,   "spi3"),
	CLK_FULL(K210_CLK_TIMER0, "timer0"),
	CLK_FULL(K210_CLK_TIMER1, "timer1"),
	CLK_FULL(K210_CLK_TIMER2, "timer2"),
	CLK_NOMUX(K210_CLK_SRAM0, "sram0",  K210_CLK_ACLK),
	CLK_NOMUX(K210_CLK_SRAM1, "sram1",  K210_CLK_ACLK),
	CLK_NOMUX(K210_CLK_ROM,   "rom",    K210_CLK_ACLK),
	CLK_NOMUX(K210_CLK_DVP,   "dvp",    K210_CLK_ACLK),
	CLK_NOMUX(K210_CLK_APB0,  "apb0",   K210_CLK_ACLK),
	CLK_NOMUX(K210_CLK_APB1,  "apb1",   K210_CLK_ACLK),
	CLK_NOMUX(K210_CLK_APB2,  "apb2",   K210_CLK_ACLK),
	CLK_NOMUX(K210_CLK_AI,    "ai",     K210_CLK_PLL1),
	CLK_NOMUX(K210_CLK_I2S0,  "i2s0",   K210_CLK_PLL2),
	CLK_NOMUX(K210_CLK_I2S1,  "i2s1",   K210_CLK_PLL2),
	CLK_NOMUX(K210_CLK_I2S2,  "i2s2",   K210_CLK_PLL2),
	CLK_NOMUX(K210_CLK_WDT0,  "wdt0",   K210_CLK_IN0),
	CLK_NOMUX(K210_CLK_WDT1,  "wdt1",   K210_CLK_IN0),
	CLK_NOMUX(K210_CLK_SPI0,  "spi0",   K210_CLK_PLL0),
	CLK_NOMUX(K210_CLK_SPI1,  "spi1",   K210_CLK_PLL0),
	CLK_NOMUX(K210_CLK_SPI2,  "spi2",   K210_CLK_PLL0),
	CLK_NOMUX(K210_CLK_I2C0,  "i2c0",   K210_CLK_PLL0),
	CLK_NOMUX(K210_CLK_I2C1,  "i2c1",   K210_CLK_PLL0),
	CLK_NOMUX(K210_CLK_I2C2,  "i2c2",   K210_CLK_PLL0),
	CLK_DIV(K210_CLK_I2S0_M,  "i2s0_m", K210_CLK_PLL2),
	CLK_DIV(K210_CLK_I2S1_M,  "i2s1_m", K210_CLK_PLL2),
	CLK_DIV(K210_CLK_I2S2_M,  "i2s2_m", K210_CLK_PLL2),
	CLK_DIV(K210_CLK_CLINT,   "clint",  K210_CLK_ACLK),
	CLK_GATE(K210_CLK_CPU,    "cpu",    K210_CLK_ACLK),
	CLK_GATE(K210_CLK_DMA,    "dma",    K210_CLK_ACLK),
	CLK_GATE(K210_CLK_FFT,    "fft",    K210_CLK_ACLK),
	CLK_GATE(K210_CLK_GPIO,   "gpio",   K210_CLK_APB0),
	CLK_GATE(K210_CLK_UART1,  "uart1",  K210_CLK_APB0),
	CLK_GATE(K210_CLK_UART2,  "uart2",  K210_CLK_APB0),
	CLK_GATE(K210_CLK_UART3,  "uart3",  K210_CLK_APB0),
	CLK_GATE(K210_CLK_FPIOA,  "fpioa",  K210_CLK_APB0),
	CLK_GATE(K210_CLK_SHA,    "sha",    K210_CLK_APB0),
	CLK_GATE(K210_CLK_AES,    "aes",    K210_CLK_APB1),
	CLK_GATE(K210_CLK_OTP,    "otp",    K210_CLK_APB1),
	CLK_GATE(K210_CLK_RTC,    "rtc",    K210_CLK_IN0),
#undef NAME
#undef CLK_PLL
#undef CLK
#undef CLK_FULL
#undef CLK_NOMUX
#undef CLK_DIV
#undef CLK_GATE
#undef CLK_LIST
};

#define K210_PLL_CLKR		GENMASK(3, 0)
#define K210_PLL_CLKF		GENMASK(9, 4)
#define K210_PLL_CLKOD		GENMASK(13, 10) /* Output Divider */
#define K210_PLL_BWADJ		GENMASK(19, 14) /* BandWidth Adjust */
#define K210_PLL_RESET		BIT(20)
#define K210_PLL_PWRD		BIT(21) /* PoWeReD */
#define K210_PLL_INTFB		BIT(22) /* Internal FeedBack */
#define K210_PLL_BYPASS		BIT(23)
#define K210_PLL_TEST		BIT(24)
#define K210_PLL_EN		BIT(25)
#define K210_PLL_TEST_EN	BIT(26)

#define K210_PLL_LOCK		0
#define K210_PLL_CLEAR_SLIP	2
#define K210_PLL_TEST_OUT	3

#ifdef CONFIG_CLK_K210_SET_RATE
static int k210_pll_enable(struct k210_clk_priv *priv, int id);
static int k210_pll_disable(struct k210_clk_priv *priv, int id);
static ulong k210_pll_get_rate(struct k210_clk_priv *priv, int id, ulong rate_in);

/*
 * The PLL included with the Kendryte K210 appears to be a True Circuits, Inc.
 * General-Purpose PLL. The logical layout of the PLL with internal feedback is
 * approximately the following:
 *
 *  +---------------+
 *  |reference clock|
 *  +---------------+
 *          |
 *          v
 *        +--+
 *        |/r|
 *        +--+
 *          |
 *          v
 *   +-------------+
 *   |divided clock|
 *   +-------------+
 *          |
 *          v
 *  +--------------+
 *  |phase detector|<---+
 *  +--------------+    |
 *          |           |
 *          v   +--------------+
 *        +---+ |feedback clock|
 *        |VCO| +--------------+
 *        +---+         ^
 *          |    +--+   |
 *          +--->|/f|---+
 *          |    +--+
 *          v
 *        +---+
 *        |/od|
 *        +---+
 *          |
 *          v
 *       +------+
 *       |output|
 *       +------+
 *
 * The k210 PLLs have three factors: r, f, and od. Because of the feedback mode,
 * the effect of the division by f is to multiply the input frequency. The
 * equation for the output rate is
 *   rate = (rate_in * f) / (r * od).
 * Moving knowns to one side of the equation, we get
 *   rate / rate_in = f / (r * od)
 * Rearranging slightly,
 *   abs_error = abs((rate / rate_in) - (f / (r * od))).
 * To get relative, error, we divide by the expected ratio
 *   error = abs((rate / rate_in) - (f / (r * od))) / (rate / rate_in).
 * Simplifying,
 *   error = abs(1 - f / (r * od)) / (rate / rate_in)
 *   error = abs(1 - (f * rate_in) / (r * od * rate))
 * Using the constants ratio = rate / rate_in and inv_ratio = rate_in / rate,
 *   error = abs((f * inv_ratio) / (r * od) - 1)
 * This is the error used in evaluating parameters.
 *
 * r and od are four bits each, while f is six bits. Because r and od are
 * multiplied together, instead of the full 256 values possible if both bits
 * were used fully, there are only 97 distinct products. Combined with f, there
 * are 6208 theoretical settings for the PLL. However, most of these settings
 * can be ruled out immediately because they do not have the correct ratio.
 *
 * In addition to the constraint of approximating the desired ratio, parameters
 * must also keep internal pll frequencies within acceptable ranges. The divided
 * clock's minimum and maximum frequencies have a ratio of around 128.  This
 * leaves fairly substantial room to work with, especially since the only
 * affected parameter is r. The VCO's minimum and maximum frequency have a ratio
 * of 5, which is considerably more restrictive.
 *
 * The r and od factors are stored in a table. This is to make it easy to find
 * the next-largest product. Some products have multiple factorizations, but
 * only when one factor has at least a 2.5x ratio to the factors of the other
 * factorization. This is because any smaller ratio would not make a difference
 * when ensuring the VCO's frequency is within spec.
 *
 * Throughout the calculation function, fixed point arithmetic is used. Because
 * the range of rate and rate_in may be up to 1.75 GHz, or around 2^30, 64-bit
 * 32.32 fixed-point numbers are used to represent ratios. In general, to
 * implement division, the numerator is first multiplied by 2^32. This gives a
 * result where the whole number part is in the upper 32 bits, and the fraction
 * is in the lower 32 bits.
 *
 * In general, rounding is done to the closest integer. This helps find the best
 * approximation for the ratio. Rounding in one direction (e.g down) could cause
 * the function to miss a better ratio with one of the parameters increased by
 * one.
 */

/*
 * The factors table was generated with the following python code:
 *
 * def p(x, y):
 *    return (1.0*x/y > 2.5) or (1.0*y/x > 2.5)
 *
 * factors = {}
 * for i in range(1, 17):
 *    for j in range(1, 17):
 *       fs = factors.get(i*j) or []
 *       if fs == [] or all([
 *             (p(i, x) and p(i, y)) or (p(j, x) and p(j, y))
 *             for (x, y) in fs]):
 *          fs.append((i, j))
 *          factors[i*j] = fs
 *
 * for k, l in sorted(factors.items()):
 *    for v in l:
 *       print("PACK(%s, %s)," % v)
 */
#define PACK(r, od) (((((r) - 1) & 0xF) << 4) | (((od) - 1) & 0xF))
#define UNPACK_R(val) ((((val) >> 4) & 0xF) + 1)
#define UNPACK_OD(val) (((val) & 0xF) + 1)
static const u8 factors[] = {
	PACK(1, 1),
	PACK(1, 2),
	PACK(1, 3),
	PACK(1, 4),
	PACK(1, 5),
	PACK(1, 6),
	PACK(1, 7),
	PACK(1, 8),
	PACK(1, 9),
	PACK(3, 3),
	PACK(1, 10),
	PACK(1, 11),
	PACK(1, 12),
	PACK(3, 4),
	PACK(1, 13),
	PACK(1, 14),
	PACK(1, 15),
	PACK(3, 5),
	PACK(1, 16),
	PACK(4, 4),
	PACK(2, 9),
	PACK(2, 10),
	PACK(3, 7),
	PACK(2, 11),
	PACK(2, 12),
	PACK(5, 5),
	PACK(2, 13),
	PACK(3, 9),
	PACK(2, 14),
	PACK(2, 15),
	PACK(2, 16),
	PACK(3, 11),
	PACK(5, 7),
	PACK(3, 12),
	PACK(3, 13),
	PACK(4, 10),
	PACK(3, 14),
	PACK(4, 11),
	PACK(3, 15),
	PACK(3, 16),
	PACK(7, 7),
	PACK(5, 10),
	PACK(4, 13),
	PACK(6, 9),
	PACK(5, 11),
	PACK(4, 14),
	PACK(4, 15),
	PACK(7, 9),
	PACK(4, 16),
	PACK(5, 13),
	PACK(6, 11),
	PACK(5, 14),
	PACK(6, 12),
	PACK(5, 15),
	PACK(7, 11),
	PACK(6, 13),
	PACK(5, 16),
	PACK(9, 9),
	PACK(6, 14),
	PACK(8, 11),
	PACK(6, 15),
	PACK(7, 13),
	PACK(6, 16),
	PACK(7, 14),
	PACK(9, 11),
	PACK(10, 10),
	PACK(8, 13),
	PACK(7, 15),
	PACK(9, 12),
	PACK(10, 11),
	PACK(7, 16),
	PACK(9, 13),
	PACK(8, 15),
	PACK(11, 11),
	PACK(9, 14),
	PACK(8, 16),
	PACK(10, 13),
	PACK(11, 12),
	PACK(9, 15),
	PACK(10, 14),
	PACK(11, 13),
	PACK(9, 16),
	PACK(10, 15),
	PACK(11, 14),
	PACK(12, 13),
	PACK(10, 16),
	PACK(11, 15),
	PACK(12, 14),
	PACK(13, 13),
	PACK(11, 16),
	PACK(12, 15),
	PACK(13, 14),
	PACK(12, 16),
	PACK(13, 15),
	PACK(14, 14),
	PACK(13, 16),
	PACK(14, 15),
	PACK(14, 16),
	PACK(15, 15),
	PACK(15, 16),
	PACK(16, 16),
};

TEST_STATIC int k210_pll_calc_config(u32 rate, u32 rate_in,
				     struct k210_pll_config *best)
{
	int i;
	s64 error, best_error;
	u64 ratio, inv_ratio; /* fixed point 32.32 ratio of the rates */
	u64 max_r;
	u64 r, f, od;

	/*
	 * Can't go over 1.75 GHz or under 21.25 MHz due to limitations on the
	 * VCO frequency. These are not the same limits as below because od can
	 * reduce the output frequency by 16.
	 */
	if (rate > 1750000000 || rate < 21250000)
		return -EINVAL;

	/* Similar restrictions on the input rate */
	if (rate_in > 1750000000 || rate_in < 13300000)
		return -EINVAL;

	ratio = DIV_ROUND_CLOSEST_ULL((u64)rate << 32, rate_in);
	inv_ratio = DIV_ROUND_CLOSEST_ULL((u64)rate_in << 32, rate);
	/* Can't increase by more than 64 or reduce by more than 256 */
	if (rate > rate_in && ratio > (64ULL << 32))
		return -EINVAL;
	else if (rate <= rate_in && inv_ratio > (256ULL << 32))
		return -EINVAL;

	/*
	 * The divided clock (rate_in / r) must stay between 1.75 GHz and 13.3
	 * MHz. There is no minimum, since the only way to get a higher input
	 * clock than 26 MHz is to use a clock generated by a PLL. Because PLLs
	 * cannot output frequencies greater than 1.75 GHz, the minimum would
	 * never be greater than one.
	 */
	max_r = DIV_ROUND_DOWN_ULL(rate_in, 13300000);

	/* Variables get immediately incremented, so start at -1th iteration */
	i = -1;
	f = 0;
	r = 0;
	od = 0;
	best_error = S64_MAX;
	error = best_error;
	/* do-while here so we always try at least one ratio */
	do {
		/*
		 * Whether we swapped r and od while enforcing frequency limits
		 */
		bool swapped = false;
		/*
		 * Whether the intermediate frequencies are out-of-spec
		 */
		bool out_of_spec;
		u64 last_od = od;
		u64 last_r = r;

		/*
		 * Try the next largest value for f (or r and od) and
		 * recalculate the other parameters based on that
		 */
		if (rate > rate_in) {
			/*
			 * Skip factors of the same product if we already tried
			 * out that product
			 */
			do {
				i++;
				r = UNPACK_R(factors[i]);
				od = UNPACK_OD(factors[i]);
			} while (i + 1 < ARRAY_SIZE(factors) &&
				 r * od == last_r * last_od);

			/* Round close */
			f = (r * od * ratio + BIT(31)) >> 32;
			if (f > 64)
				f = 64;
		} else {
			u64 tmp = ++f * inv_ratio;
			bool round_up = !!(tmp & BIT(31));
			u32 goal = (tmp >> 32) + round_up;
			u32 err, last_err;

			/* Get the next r/od pair in factors */
			while (r * od < goal && i + 1 < ARRAY_SIZE(factors)) {
				i++;
				r = UNPACK_R(factors[i]);
				od = UNPACK_OD(factors[i]);
			}

			/*
			 * This is a case of double rounding. If we rounded up
			 * above, we need to round down (in cases of ties) here.
			 * This prevents off-by-one errors resulting from
			 * choosing X+2 over X when X.Y rounds up to X+1 and
			 * there is no r * od = X+1. For the converse, when X.Y
			 * is rounded down to X, we should choose X+1 over X-1.
			 */
			err = abs(r * od - goal);
			last_err = abs(last_r * last_od - goal);
			if (last_err < err || (round_up && last_err == err)) {
				i--;
				r = last_r;
				od = last_od;
			}
		}

		/*
		 * Enforce limits on internal clock frequencies. If we
		 * aren't in spec, try swapping r and od. If everything is
		 * in-spec, calculate the relative error.
		 */
again:
		out_of_spec = false;
		if (r > max_r) {
			out_of_spec = true;
		} else {
			/*
			 * There is no way to only divide once; we need
			 * to examine the frequency with and without the
			 * effect of od.
			 */
			u64 vco = DIV_ROUND_CLOSEST_ULL(rate_in * f, r);

			if (vco > 1750000000 || vco < 340000000)
				out_of_spec = true;
		}

		if (out_of_spec) {
			u64 new_r, new_od;

			if (!swapped) {
				u64 tmp = r;

				r = od;
				od = tmp;
				swapped = true;
				goto again;
			}

			/*
			 * Try looking ahead to see if there are additional
			 * factors for the same product.
			 */
			if (i + 1 < ARRAY_SIZE(factors)) {
				i++;
				new_r = UNPACK_R(factors[i]);
				new_od = UNPACK_OD(factors[i]);
				if (r * od == new_r * new_od) {
					r = new_r;
					od = new_od;
					swapped = false;
					goto again;
				}
				i--;
			}

			/*
			 * Try looking back to see if there is a worse ratio
			 * that we could try anyway
			 */
			while (i > 0) {
				i--;
				new_r = UNPACK_R(factors[i]);
				new_od = UNPACK_OD(factors[i]);
				/*
				 * Don't loop over factors for the same product
				 * to avoid getting stuck because of the above
				 * clause
				 */
				if (r * od != new_r * new_od) {
					if (new_r * new_od > last_r * last_od) {
						r = new_r;
						od = new_od;
						swapped = false;
						goto again;
					}
					break;
				}
			}

			/* We ran out of things to try */
			continue;
		}

		error = DIV_ROUND_CLOSEST_ULL(f * inv_ratio, r * od);
		/* The lower 16 bits are spurious */
		error = abs64((error - BIT_ULL(32))) >> 16;

		if (error < best_error) {
			best->r = r;
			best->f = f;
			best->od = od;
			best_error = error;
		}
	} while (f < 64 && i + 1 < ARRAY_SIZE(factors) && error != 0);

	log_debug("best error %lld\n", best_error);
	if (best_error == S64_MAX)
		return -EINVAL;

	return 0;
}

static ulong k210_pll_set_rate(struct k210_clk_priv *priv, int id, ulong rate,
			       ulong rate_in)
{
	int err;
	const struct k210_pll_params *pll = &k210_plls[id];
	struct k210_pll_config config = {};
	u32 reg;
	ulong calc_rate;

	err = k210_pll_calc_config(rate, rate_in, &config);
	if (err)
		return err;
	log_debug("Got r=%u f=%u od=%u\n", config.r, config.f, config.od);

	/* Don't bother setting the rate if we're already at that rate */
	calc_rate = DIV_ROUND_DOWN_ULL(((u64)rate_in) * config.f,
				       config.r * config.od);
	if (calc_rate == k210_pll_get_rate(priv, id, rate))
		return calc_rate;

	k210_pll_disable(priv, id);

	reg = readl(priv->base + pll->off);
	reg &= ~K210_PLL_CLKR
	    &  ~K210_PLL_CLKF
	    &  ~K210_PLL_CLKOD
	    &  ~K210_PLL_BWADJ;
	reg |= FIELD_PREP(K210_PLL_CLKR, config.r - 1)
	    |  FIELD_PREP(K210_PLL_CLKF, config.f - 1)
	    |  FIELD_PREP(K210_PLL_CLKOD, config.od - 1)
	    |  FIELD_PREP(K210_PLL_BWADJ, config.f - 1);
	writel(reg, priv->base + pll->off);

	k210_pll_enable(priv, id);

	serial_setbrg();
	return k210_pll_get_rate(priv, id, rate);
}
#else
static ulong k210_pll_set_rate(struct k210_clk_priv *priv, int id, ulong rate,
			       ulong rate_in)
{
	return -ENOSYS;
}
#endif /* CONFIG_CLK_K210_SET_RATE */

static ulong k210_pll_get_rate(struct k210_clk_priv *priv, int id,
			       ulong rate_in)
{
	u64 r, f, od;
	u32 reg = readl(priv->base + k210_plls[id].off);

	if (reg & K210_PLL_BYPASS)
		return rate_in;

	if (!(reg & K210_PLL_PWRD))
		return 0;

	r = FIELD_GET(K210_PLL_CLKR, reg) + 1;
	f = FIELD_GET(K210_PLL_CLKF, reg) + 1;
	od = FIELD_GET(K210_PLL_CLKOD, reg) + 1;

	return DIV_ROUND_DOWN_ULL(((u64)rate_in) * f, r * od);
}

/*
 * Wait for the PLL to be locked. If the PLL is not locked, try clearing the
 * slip before retrying
 */
static void k210_pll_waitfor_lock(struct k210_clk_priv *priv, int id)
{
	const struct k210_pll_params *pll = &k210_plls[id];
	u32 mask = (BIT(pll->width) - 1) << pll->shift;

	while (true) {
		u32 reg = readl(priv->base + K210_SYSCTL_PLL_LOCK);

		if ((reg & mask) == mask)
			break;

		reg |= BIT(pll->shift + K210_PLL_CLEAR_SLIP);
		writel(reg, priv->base + K210_SYSCTL_PLL_LOCK);
	}
}

static bool k210_pll_enabled(u32 reg)
{
	return (reg & K210_PLL_PWRD) && (reg & K210_PLL_EN) &&
		!(reg & K210_PLL_RESET);
}

/* Adapted from sysctl_pll_enable */
static int k210_pll_enable(struct k210_clk_priv *priv, int id)
{
	const struct k210_pll_params *pll = &k210_plls[id];
	u32 reg = readl(priv->base + pll->off);

	if (k210_pll_enabled(reg))
		return 0;

	reg |= K210_PLL_PWRD;
	writel(reg, priv->base + pll->off);

	/* Ensure reset is low before asserting it */
	reg &= ~K210_PLL_RESET;
	writel(reg, priv->base + pll->off);
	reg |= K210_PLL_RESET;
	writel(reg, priv->base + pll->off);
	nop();
	nop();
	reg &= ~K210_PLL_RESET;
	writel(reg, priv->base + pll->off);

	k210_pll_waitfor_lock(priv, id);

	reg &= ~K210_PLL_BYPASS;
	reg |= K210_PLL_EN;
	writel(reg, priv->base + pll->off);

	return 0;
}

static int k210_pll_disable(struct k210_clk_priv *priv, int id)
{
	const struct k210_pll_params *pll = &k210_plls[id];
	u32 reg = readl(priv->base + pll->off);

	/*
	 * Bypassing before powering off is important so child clocks don't stop
	 * working. This is especially important for pll0, the indirect parent
	 * of the cpu clock.
	 */
	reg |= K210_PLL_BYPASS;
	writel(reg, priv->base + pll->off);

	reg &= ~K210_PLL_PWRD;
	reg &= ~K210_PLL_EN;
	writel(reg, priv->base + pll->off);
	return 0;
}

static u32 k210_clk_readl(struct k210_clk_priv *priv, u8 off, u8 shift,
			  u8 width)
{
	u32 reg = readl(priv->base + off);

	return (reg >> shift) & (BIT(width) - 1);
}

static void k210_clk_writel(struct k210_clk_priv *priv, u8 off, u8 shift,
			    u8 width, u32 val)
{
	u32 reg = readl(priv->base + off);
	u32 mask = (BIT(width) - 1) << shift;

	reg &= ~mask;
	reg |= mask & (val << shift);
	writel(reg, priv->base + off);
}

static int k210_clk_get_parent(struct k210_clk_priv *priv, int id)
{
	u32 sel;
	const struct k210_mux_params *mux;

	if (!(k210_clks[id].flags & K210_CLKF_MUX))
		return k210_clks[id].parent;
	mux = &k210_muxes[k210_clks[id].mux];

	sel = k210_clk_readl(priv, mux->off, mux->shift, mux->width);
	assert(sel < mux->num_parents);
	return mux->parents[sel];
}

static ulong do_k210_clk_get_rate(struct k210_clk_priv *priv, int id)
{
	int parent;
	u32 val;
	ulong parent_rate;
	const struct k210_div_params *div;

	if (id == K210_CLK_IN0)
		return clk_get_rate(&priv->in0);

	parent = k210_clk_get_parent(priv, id);
	parent_rate = do_k210_clk_get_rate(priv, parent);
	if (IS_ERR_VALUE(parent_rate))
		return parent_rate;

	if (k210_clks[id].flags & K210_CLKF_PLL)
		return k210_pll_get_rate(priv, k210_clks[id].pll, parent_rate);

	if (k210_clks[id].div == K210_CLK_DIV_NONE)
		return parent_rate;
	div = &k210_divs[k210_clks[id].div];

	if (div->type == K210_DIV_FIXED)
		return parent_rate / div->div;

	val = k210_clk_readl(priv, div->off, div->shift, div->width);
	switch (div->type) {
	case K210_DIV_ONE:
		return parent_rate / (val + 1);
	case K210_DIV_EVEN:
		return parent_rate / 2 / (val + 1);
	case K210_DIV_POWER:
		/* This is ACLK, which has no divider on IN0 */
		if (parent == K210_CLK_IN0)
			return parent_rate;
		return parent_rate / (2 << val);
	default:
		assert(false);
		return -EINVAL;
	};
}

static ulong k210_clk_get_rate(struct clk *clk)
{
	return do_k210_clk_get_rate(dev_get_priv(clk->dev), clk->id);
}

static int do_k210_clk_set_parent(struct k210_clk_priv *priv, int id, int new)
{
	int i;
	const struct k210_mux_params *mux;

	if (!(k210_clks[id].flags & K210_CLKF_MUX))
		return -ENOSYS;
	mux = &k210_muxes[k210_clks[id].mux];

	for (i = 0; i < mux->num_parents; i++) {
		if (mux->parents[i] == new) {
			k210_clk_writel(priv, mux->off, mux->shift, mux->width,
					i);
			return 0;
		}
	}
	return -EINVAL;
}

static int k210_clk_set_parent(struct clk *clk, struct clk *parent)
{
	return do_k210_clk_set_parent(dev_get_priv(clk->dev), clk->id,
				      parent->id);
}

static ulong k210_clk_set_rate(struct clk *clk, unsigned long rate)
{
	int parent, ret, err;
	ulong rate_in, val;
	const struct k210_div_params *div;
	struct k210_clk_priv *priv = dev_get_priv(clk->dev);

	if (clk->id == K210_CLK_IN0)
		return clk_set_rate(&priv->in0, rate);

	parent = k210_clk_get_parent(priv, clk->id);
	rate_in = do_k210_clk_get_rate(priv, parent);
	if (IS_ERR_VALUE(rate_in))
		return rate_in;

	log_debug("id=%ld rate=%lu rate_in=%lu\n", clk->id, rate, rate_in);

	if (clk->id == K210_CLK_PLL0) {
		/* Bypass ACLK so the CPU keeps going */
		ret = do_k210_clk_set_parent(priv, K210_CLK_ACLK, K210_CLK_IN0);
		if (ret)
			return ret;
	} else if (clk->id == K210_CLK_PLL1 && gd->flags & GD_FLG_RELOC) {
		/*
		 * We can't bypass the AI clock like we can ACLK, and after
		 * relocation we are using the AI ram.
		 */
		return -EPERM;
	}

	if (k210_clks[clk->id].flags & K210_CLKF_PLL) {
		ret = k210_pll_set_rate(priv, k210_clks[clk->id].pll, rate,
					rate_in);
		if (!IS_ERR_VALUE(ret) && clk->id == K210_CLK_PLL0) {
			/*
			 * This may have the side effect of reparenting ACLK,
			 * but I don't really want to keep track of what the old
			 * parent was.
			 */
			err = do_k210_clk_set_parent(priv, K210_CLK_ACLK,
						     K210_CLK_PLL0);
			if (err)
				return err;
		}
		return ret;
	}

	if (k210_clks[clk->id].div == K210_CLK_DIV_NONE)
		return -ENOSYS;
	div = &k210_divs[k210_clks[clk->id].div];

	switch (div->type) {
	case K210_DIV_ONE:
		val = DIV_ROUND_CLOSEST_ULL((u64)rate_in, rate);
		val = val ? val - 1 : 0;
		break;
	case K210_DIV_EVEN:
		val = DIV_ROUND_CLOSEST_ULL((u64)rate_in, 2 * rate);
		break;
	case K210_DIV_POWER:
		/* This is ACLK, which has no divider on IN0 */
		if (parent == K210_CLK_IN0)
			return -ENOSYS;

		val = DIV_ROUND_CLOSEST_ULL((u64)rate_in, rate);
		val = __ffs(val);
		break;
	default:
		assert(false);
		return -EINVAL;
	};

	val = val ? val - 1 : 0;
	k210_clk_writel(priv, div->off, div->shift, div->width, val);
	return do_k210_clk_get_rate(priv, clk->id);
}

static int k210_clk_endisable(struct k210_clk_priv *priv, int id, bool enable)
{
	int parent = k210_clk_get_parent(priv, id);
	const struct k210_gate_params *gate;

	if (id == K210_CLK_IN0) {
		if (enable)
			return clk_enable(&priv->in0);
		else
			return clk_disable(&priv->in0);
	}

	/* Only recursively enable clocks since we don't track refcounts */
	if (enable) {
		int ret = k210_clk_endisable(priv, parent, true);

		if (ret && ret != -ENOSYS)
			return ret;
	}

	if (k210_clks[id].flags & K210_CLKF_PLL) {
		if (enable)
			return k210_pll_enable(priv, k210_clks[id].pll);
		else
			return k210_pll_disable(priv, k210_clks[id].pll);
	}

	if (k210_clks[id].gate == K210_CLK_GATE_NONE)
		return -ENOSYS;
	gate = &k210_gates[k210_clks[id].gate];

	k210_clk_writel(priv, gate->off, gate->bit_idx, 1, enable);
	return 0;
}

static int k210_clk_enable(struct clk *clk)
{
	return k210_clk_endisable(dev_get_priv(clk->dev), clk->id, true);
}

static int k210_clk_disable(struct clk *clk)
{
	return k210_clk_endisable(dev_get_priv(clk->dev), clk->id, false);
}

static int k210_clk_request(struct clk *clk)
{
	if (clk->id >= ARRAY_SIZE(k210_clks))
		return -EINVAL;
	return 0;
}

#if IS_ENABLED(CONFIG_CMD_CLK)
static char show_enabled(struct k210_clk_priv *priv, int id)
{
	bool enabled;

	if (k210_clks[id].flags & K210_CLKF_PLL) {
		const struct k210_pll_params *pll =
			&k210_plls[k210_clks[id].pll];

		enabled = k210_pll_enabled(readl(priv->base + pll->off));
	} else if (k210_clks[id].gate == K210_CLK_GATE_NONE) {
		return '-';
	} else {
		const struct k210_gate_params *gate =
			&k210_gates[k210_clks[id].gate];

		enabled = k210_clk_readl(priv, gate->off, gate->bit_idx, 1);
	}

	return enabled ? 'y' : 'n';
}

static void show_clks(struct k210_clk_priv *priv, int id, int depth)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(k210_clks); i++) {
		if (k210_clk_get_parent(priv, i) != id)
			continue;

		printf(" %-9lu %-7c %*s%s\n", do_k210_clk_get_rate(priv, i),
		       show_enabled(priv, i), depth * 4, "",
		       k210_clks[i].name);

		show_clks(priv, i, depth + 1);
	}
}

static void k210_clk_dump(struct udevice *dev)
{
	struct k210_clk_priv *priv;

	priv = dev_get_priv(dev);

	puts(" Rate      Enabled Name\n");
	puts("------------------------\n");
	printf(" %-9lu %-7c %*s%s\n", clk_get_rate(&priv->in0), 'y', 0, "",
	       priv->in0.dev->name);
	show_clks(priv, K210_CLK_IN0, 1);
}
#endif

static const struct clk_ops k210_clk_ops = {
	.request = k210_clk_request,
	.set_rate = k210_clk_set_rate,
	.get_rate = k210_clk_get_rate,
	.set_parent = k210_clk_set_parent,
	.enable = k210_clk_enable,
	.disable = k210_clk_disable,
#if IS_ENABLED(CONFIG_CMD_CLK)
	.dump = k210_clk_dump,
#endif
};

static int k210_clk_probe(struct udevice *dev)
{
	int ret;
	struct k210_clk_priv *priv = dev_get_priv(dev);

	priv->base = dev_read_addr_ptr(dev_get_parent(dev));
	if (!priv->base)
		return -EINVAL;

	ret = clk_get_by_index(dev, 0, &priv->in0);
	if (ret)
		return ret;

	/*
	 * Force setting defaults, even before relocation. This is so we can
	 * set the clock rate for PLL1 before we relocate into aisram.
	 */
	if (!(gd->flags & GD_FLG_RELOC))
		clk_set_defaults(dev, CLK_DEFAULTS_POST_FORCE);

	return 0;
}

static const struct udevice_id k210_clk_ids[] = {
	{ .compatible = "canaan,k210-clk" },
	{ },
};

U_BOOT_DRIVER(k210_clk) = {
	.name = "k210_clk",
	.id = UCLASS_CLK,
	.of_match = k210_clk_ids,
	.ops = &k210_clk_ops,
	.probe = k210_clk_probe,
	.priv_auto = sizeof(struct k210_clk_priv),
};
