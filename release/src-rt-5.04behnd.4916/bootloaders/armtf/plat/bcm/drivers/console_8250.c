#include <drivers/console.h>
/* uart register defines */
#define UART_RHR	0x0
#define UART_THR	0x0
#define UART_IER	0x4
#define UART_ISR	0x8
#define UART_FCR	0x8
#define UART_LCR	0xc
#define UART_MCR	0x10
#define UART_LSR	0x14
#define UART_MSR	0x18
#define UART_SPR	0x1c

/* uart status register bits */
#define LSR_TEMT	0x40 /* Transmitter empty */
#define LSR_THRE	0x20 /* Transmit-hold-register empty */
#define LSR_EMPTY	(LSR_TEMT | LSR_THRE)
#define LSR_DR		0x01 /* DATA Ready */


static void console_8250_flush(console_t *console)
{
	uintptr_t base = console->base;

	while (1) {
		uint32_t state = *(uint32_t*)(base + UART_LSR);

		/* Wait until transmit FIFO is empty */
		if ((state & LSR_EMPTY) == LSR_EMPTY)
			break;
	}
}

static int console_8250_have_rx_data(console_t *console)
{
	uintptr_t base = console->base;

	return (*(uint32_t*)(base + UART_LSR) & LSR_DR);
}

static int console_8250_getc(console_t *console)
{
	uintptr_t base = console->base;

	while (!console_8250_have_rx_data(console)) {
		/* Transmit FIFO is empty, waiting again */
		;
	}
	return *(uint32_t*)(base + UART_RHR) & 0xff;
}

static void console_8250_putc(int c, console_t *console)
{
	uintptr_t base = console->base;

	console_8250_flush(console);

	*(uint32_t*)(base + UART_THR) = c;
	if (c == '\n')
		*(uint32_t*)(base + UART_THR) = '\r';
}

void console_8250_register(uintptr_t baseaddr, console_t *console)
{
	uintptr_t *fp;

	console->flags = CONSOLE_FLAG_BOOT;

	/* Register callback functions */
	fp = (uintptr_t*)&console->putc;
	*fp = (uintptr_t)console_8250_putc;

	fp = (uintptr_t*)&console->getc;
	*fp = (uintptr_t)console_8250_getc;

	fp = (uintptr_t*)&console->flush;
	*fp = (uintptr_t)console_8250_flush;

	console->base = baseaddr;
	console_register(console);
}
