// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2016, STMicroelectronics - All Rights Reserved
 * Author(s): Vikas Manocha, <vikas.manocha@st.com> for STMicroelectronics.
 */

#include <common.h>
#include <clk.h>
#include <dm.h>
#include <reset.h>
#include <serial.h>
#include <watchdog.h>
#include <asm/io.h>
#include <asm/arch/stm32.h>
#include "serial_stm32.h"

static void _stm32_serial_setbrg(fdt_addr_t base,
				 struct stm32_uart_info *uart_info,
				 u32 clock_rate,
				 int baudrate)
{
	bool stm32f4 = uart_info->stm32f4;
	u32 int_div, mantissa, fraction, oversampling;

	int_div = DIV_ROUND_CLOSEST(clock_rate, baudrate);

	if (int_div < 16) {
		oversampling = 8;
		setbits_le32(base + CR1_OFFSET(stm32f4), USART_CR1_OVER8);
	} else {
		oversampling = 16;
		clrbits_le32(base + CR1_OFFSET(stm32f4), USART_CR1_OVER8);
	}

	mantissa = (int_div / oversampling) << USART_BRR_M_SHIFT;
	fraction = int_div % oversampling;

	writel(mantissa | fraction, base + BRR_OFFSET(stm32f4));
}

static int stm32_serial_setbrg(struct udevice *dev, int baudrate)
{
	struct stm32x7_serial_platdata *plat = dev_get_platdata(dev);

	_stm32_serial_setbrg(plat->base, plat->uart_info,
			     plat->clock_rate, baudrate);

	return 0;
}

static int stm32_serial_setconfig(struct udevice *dev, uint serial_config)
{
	struct stm32x7_serial_platdata *plat = dev_get_platdata(dev);
	bool stm32f4 = plat->uart_info->stm32f4;
	u8 uart_enable_bit = plat->uart_info->uart_enable_bit;
	u32 cr1 = plat->base + CR1_OFFSET(stm32f4);
	u32 config = 0;
	uint parity = SERIAL_GET_PARITY(serial_config);
	uint bits = SERIAL_GET_BITS(serial_config);
	uint stop = SERIAL_GET_STOP(serial_config);

	/*
	 * only parity config is implemented, check if other serial settings
	 * are the default one.
	 * (STM32F4 serial IP didn't support parity setting)
	 */
	if (bits != SERIAL_8_BITS || stop != SERIAL_ONE_STOP || stm32f4)
		return -ENOTSUPP; /* not supported in driver*/

	clrbits_le32(cr1, USART_CR1_RE | USART_CR1_TE | BIT(uart_enable_bit));
	/* update usart configuration (uart need to be disable)
	 * PCE: parity check enable
	 * PS : '0' : Even / '1' : Odd
	 * M[1:0] = '00' : 8 Data bits
	 * M[1:0] = '01' : 9 Data bits with parity
	 */
	switch (parity) {
	default:
	case SERIAL_PAR_NONE:
		config = 0;
		break;
	case SERIAL_PAR_ODD:
		config = USART_CR1_PCE | USART_CR1_PS | USART_CR1_M0;
		break;
	case SERIAL_PAR_EVEN:
		config = USART_CR1_PCE | USART_CR1_M0;
		break;
	}

	clrsetbits_le32(cr1,
			USART_CR1_PCE | USART_CR1_PS | USART_CR1_M1 |
			USART_CR1_M0,
			config);
	setbits_le32(cr1, USART_CR1_RE | USART_CR1_TE | BIT(uart_enable_bit));

	return 0;
}

static int stm32_serial_getc(struct udevice *dev)
{
	struct stm32x7_serial_platdata *plat = dev_get_platdata(dev);
	bool stm32f4 = plat->uart_info->stm32f4;
	fdt_addr_t base = plat->base;
	u32 isr = readl(base + ISR_OFFSET(stm32f4));

	if ((isr & USART_ISR_RXNE) == 0)
		return -EAGAIN;

	if (isr & (USART_ISR_PE | USART_ISR_ORE)) {
		if (!stm32f4)
			setbits_le32(base + ICR_OFFSET,
				     USART_ICR_PCECF | USART_ICR_ORECF);
		else
			readl(base + RDR_OFFSET(stm32f4));
		return -EIO;
	}

	return readl(base + RDR_OFFSET(stm32f4));
}

static int _stm32_serial_putc(fdt_addr_t base,
			      struct stm32_uart_info *uart_info,
			      const char c)
{
	bool stm32f4 = uart_info->stm32f4;

	if ((readl(base + ISR_OFFSET(stm32f4)) & USART_ISR_TXE) == 0)
		return -EAGAIN;

	writel(c, base + TDR_OFFSET(stm32f4));

	return 0;
}

static int stm32_serial_putc(struct udevice *dev, const char c)
{
	struct stm32x7_serial_platdata *plat = dev_get_platdata(dev);

	return _stm32_serial_putc(plat->base, plat->uart_info, c);
}

static int stm32_serial_pending(struct udevice *dev, bool input)
{
	struct stm32x7_serial_platdata *plat = dev_get_platdata(dev);
	bool stm32f4 = plat->uart_info->stm32f4;
	fdt_addr_t base = plat->base;

	if (input)
		return readl(base + ISR_OFFSET(stm32f4)) &
			USART_ISR_RXNE ? 1 : 0;
	else
		return readl(base + ISR_OFFSET(stm32f4)) &
			USART_ISR_TXE ? 0 : 1;
}

static void _stm32_serial_init(fdt_addr_t base,
			       struct stm32_uart_info *uart_info)
{
	bool stm32f4 = uart_info->stm32f4;
	u8 uart_enable_bit = uart_info->uart_enable_bit;

	/* Disable uart-> enable fifo -> enable uart */
	clrbits_le32(base + CR1_OFFSET(stm32f4), USART_CR1_RE | USART_CR1_TE |
		     BIT(uart_enable_bit));
	if (uart_info->has_fifo)
		setbits_le32(base + CR1_OFFSET(stm32f4), USART_CR1_FIFOEN);
	setbits_le32(base + CR1_OFFSET(stm32f4), USART_CR1_RE | USART_CR1_TE |
		     BIT(uart_enable_bit));
}

static int stm32_serial_probe(struct udevice *dev)
{
	struct stm32x7_serial_platdata *plat = dev_get_platdata(dev);
	struct clk clk;
	struct reset_ctl reset;
	int ret;

	plat->uart_info = (struct stm32_uart_info *)dev_get_driver_data(dev);

	ret = clk_get_by_index(dev, 0, &clk);
	if (ret < 0)
		return ret;

	ret = clk_enable(&clk);
	if (ret) {
		dev_err(dev, "failed to enable clock\n");
		return ret;
	}

	ret = reset_get_by_index(dev, 0, &reset);
	if (!ret) {
		reset_assert(&reset);
		udelay(2);
		reset_deassert(&reset);
	}

	plat->clock_rate = clk_get_rate(&clk);
	if (plat->clock_rate < 0) {
		clk_disable(&clk);
		return plat->clock_rate;
	};

	_stm32_serial_init(plat->base, plat->uart_info);

	return 0;
}

static const struct udevice_id stm32_serial_id[] = {
	{ .compatible = "st,stm32-uart", .data = (ulong)&stm32f4_info},
	{ .compatible = "st,stm32f7-uart", .data = (ulong)&stm32f7_info},
	{ .compatible = "st,stm32h7-uart", .data = (ulong)&stm32h7_info},
	{}
};

static int stm32_serial_ofdata_to_platdata(struct udevice *dev)
{
	struct stm32x7_serial_platdata *plat = dev_get_platdata(dev);

	plat->base = devfdt_get_addr(dev);
	if (plat->base == FDT_ADDR_T_NONE)
		return -EINVAL;

	return 0;
}

static const struct dm_serial_ops stm32_serial_ops = {
	.putc = stm32_serial_putc,
	.pending = stm32_serial_pending,
	.getc = stm32_serial_getc,
	.setbrg = stm32_serial_setbrg,
	.setconfig = stm32_serial_setconfig
};

U_BOOT_DRIVER(serial_stm32) = {
	.name = "serial_stm32",
	.id = UCLASS_SERIAL,
	.of_match = of_match_ptr(stm32_serial_id),
	.ofdata_to_platdata = of_match_ptr(stm32_serial_ofdata_to_platdata),
	.platdata_auto_alloc_size = sizeof(struct stm32x7_serial_platdata),
	.ops = &stm32_serial_ops,
	.probe = stm32_serial_probe,
#if !CONFIG_IS_ENABLED(OF_CONTROL)
	.flags = DM_FLAG_PRE_RELOC,
#endif
};

#ifdef CONFIG_DEBUG_UART_STM32
#include <debug_uart.h>
static inline struct stm32_uart_info *_debug_uart_info(void)
{
	struct stm32_uart_info *uart_info;

#if defined(CONFIG_STM32F4)
	uart_info = &stm32f4_info;
#elif defined(CONFIG_STM32F7)
	uart_info = &stm32f7_info;
#else
	uart_info = &stm32h7_info;
#endif
	return uart_info;
}

static inline void _debug_uart_init(void)
{
	fdt_addr_t base = CONFIG_DEBUG_UART_BASE;
	struct stm32_uart_info *uart_info = _debug_uart_info();

	_stm32_serial_init(base, uart_info);
	_stm32_serial_setbrg(base, uart_info,
			     CONFIG_DEBUG_UART_CLOCK,
			     CONFIG_BAUDRATE);
}

static inline void _debug_uart_putc(int c)
{
	fdt_addr_t base = CONFIG_DEBUG_UART_BASE;
	struct stm32_uart_info *uart_info = _debug_uart_info();

	while (_stm32_serial_putc(base, uart_info, c) == -EAGAIN)
		;
}

DEBUG_UART_FUNCS
#endif
