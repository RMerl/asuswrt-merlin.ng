// SPDX-License-Identifier: GPL-2.0+

#include <debug_uart.h>
#include <asm/sbi.h>

#ifdef CONFIG_SBI_V01

static inline void _debug_uart_init(void)
{
}

static inline void _debug_uart_putc(int c)
{
	if (CONFIG_IS_ENABLED(RISCV_SMODE))
		sbi_console_putchar(c);
}

#else

static int sbi_dbcn_available __section(".data");

static inline void _debug_uart_init(void)
{
	if (CONFIG_IS_ENABLED(RISCV_SMODE))
		sbi_dbcn_available = sbi_probe_extension(SBI_EXT_DBCN);
}

static inline void _debug_uart_putc(int ch)
{
	if (CONFIG_IS_ENABLED(RISCV_SMODE) && sbi_dbcn_available)
		sbi_dbcn_write_byte(ch);
}

#endif

DEBUG_UART_FUNCS
