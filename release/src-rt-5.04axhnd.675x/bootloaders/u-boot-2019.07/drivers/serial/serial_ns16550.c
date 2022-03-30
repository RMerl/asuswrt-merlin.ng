// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000
 * Rob Taylor, Flying Pig Systems. robt@flyingpig.com.
 */

#include <common.h>
#include <linux/compiler.h>

#include <ns16550.h>
#ifdef CONFIG_NS87308
#include <ns87308.h>
#endif

#include <serial.h>

#ifndef CONFIG_NS16550_MIN_FUNCTIONS

DECLARE_GLOBAL_DATA_PTR;

#if !defined(CONFIG_CONS_INDEX)
#elif (CONFIG_CONS_INDEX < 1) || (CONFIG_CONS_INDEX > 6)
#error	"Invalid console index value."
#endif

#if CONFIG_CONS_INDEX == 1 && !defined(CONFIG_SYS_NS16550_COM1)
#error	"Console port 1 defined but not configured."
#elif CONFIG_CONS_INDEX == 2 && !defined(CONFIG_SYS_NS16550_COM2)
#error	"Console port 2 defined but not configured."
#elif CONFIG_CONS_INDEX == 3 && !defined(CONFIG_SYS_NS16550_COM3)
#error	"Console port 3 defined but not configured."
#elif CONFIG_CONS_INDEX == 4 && !defined(CONFIG_SYS_NS16550_COM4)
#error	"Console port 4 defined but not configured."
#elif CONFIG_CONS_INDEX == 5 && !defined(CONFIG_SYS_NS16550_COM5)
#error	"Console port 5 defined but not configured."
#elif CONFIG_CONS_INDEX == 6 && !defined(CONFIG_SYS_NS16550_COM6)
#error	"Console port 6 defined but not configured."
#endif

/* Note: The port number specified in the functions is 1 based.
 *	 the array is 0 based.
 */
static NS16550_t serial_ports[6] = {
#ifdef CONFIG_SYS_NS16550_COM1
	(NS16550_t)CONFIG_SYS_NS16550_COM1,
#else
	NULL,
#endif
#ifdef CONFIG_SYS_NS16550_COM2
	(NS16550_t)CONFIG_SYS_NS16550_COM2,
#else
	NULL,
#endif
#ifdef CONFIG_SYS_NS16550_COM3
	(NS16550_t)CONFIG_SYS_NS16550_COM3,
#else
	NULL,
#endif
#ifdef CONFIG_SYS_NS16550_COM4
	(NS16550_t)CONFIG_SYS_NS16550_COM4,
#else
	NULL,
#endif
#ifdef CONFIG_SYS_NS16550_COM5
	(NS16550_t)CONFIG_SYS_NS16550_COM5,
#else
	NULL,
#endif
#ifdef CONFIG_SYS_NS16550_COM6
	(NS16550_t)CONFIG_SYS_NS16550_COM6
#else
	NULL
#endif
};

#define PORT	serial_ports[port-1]

/* Multi serial device functions */
#define DECLARE_ESERIAL_FUNCTIONS(port) \
	static int  eserial##port##_init(void) \
	{ \
		int clock_divisor; \
		clock_divisor = ns16550_calc_divisor(serial_ports[port-1], \
				CONFIG_SYS_NS16550_CLK, gd->baudrate); \
		NS16550_init(serial_ports[port-1], clock_divisor); \
		return 0 ; \
	} \
	static void eserial##port##_setbrg(void) \
	{ \
		serial_setbrg_dev(port); \
	} \
	static int  eserial##port##_getc(void) \
	{ \
		return serial_getc_dev(port); \
	} \
	static int  eserial##port##_tstc(void) \
	{ \
		return serial_tstc_dev(port); \
	} \
	static void eserial##port##_putc(const char c) \
	{ \
		serial_putc_dev(port, c); \
	} \
	static void eserial##port##_puts(const char *s) \
	{ \
		serial_puts_dev(port, s); \
	}

/* Serial device descriptor */
#define INIT_ESERIAL_STRUCTURE(port, __name) {	\
	.name	= __name,			\
	.start	= eserial##port##_init,		\
	.stop	= NULL,				\
	.setbrg	= eserial##port##_setbrg,	\
	.getc	= eserial##port##_getc,		\
	.tstc	= eserial##port##_tstc,		\
	.putc	= eserial##port##_putc,		\
	.puts	= eserial##port##_puts,		\
}

static void _serial_putc(const char c, const int port)
{
	if (c == '\n')
		NS16550_putc(PORT, '\r');

	NS16550_putc(PORT, c);
}

static void _serial_puts(const char *s, const int port)
{
	while (*s) {
		_serial_putc(*s++, port);
	}
}

static int _serial_getc(const int port)
{
	return NS16550_getc(PORT);
}

static int _serial_tstc(const int port)
{
	return NS16550_tstc(PORT);
}

static void _serial_setbrg(const int port)
{
	int clock_divisor;

	clock_divisor = ns16550_calc_divisor(PORT, CONFIG_SYS_NS16550_CLK,
					     gd->baudrate);
	NS16550_reinit(PORT, clock_divisor);
}

static inline void
serial_putc_dev(unsigned int dev_index,const char c)
{
	_serial_putc(c,dev_index);
}

static inline void
serial_puts_dev(unsigned int dev_index,const char *s)
{
	_serial_puts(s,dev_index);
}

static inline int
serial_getc_dev(unsigned int dev_index)
{
	return _serial_getc(dev_index);
}

static inline int
serial_tstc_dev(unsigned int dev_index)
{
	return _serial_tstc(dev_index);
}

static inline void
serial_setbrg_dev(unsigned int dev_index)
{
	_serial_setbrg(dev_index);
}

#if defined(CONFIG_SYS_NS16550_COM1)
DECLARE_ESERIAL_FUNCTIONS(1);
struct serial_device eserial1_device =
	INIT_ESERIAL_STRUCTURE(1, "eserial0");
#endif
#if defined(CONFIG_SYS_NS16550_COM2)
DECLARE_ESERIAL_FUNCTIONS(2);
struct serial_device eserial2_device =
	INIT_ESERIAL_STRUCTURE(2, "eserial1");
#endif
#if defined(CONFIG_SYS_NS16550_COM3)
DECLARE_ESERIAL_FUNCTIONS(3);
struct serial_device eserial3_device =
	INIT_ESERIAL_STRUCTURE(3, "eserial2");
#endif
#if defined(CONFIG_SYS_NS16550_COM4)
DECLARE_ESERIAL_FUNCTIONS(4);
struct serial_device eserial4_device =
	INIT_ESERIAL_STRUCTURE(4, "eserial3");
#endif
#if defined(CONFIG_SYS_NS16550_COM5)
DECLARE_ESERIAL_FUNCTIONS(5);
struct serial_device eserial5_device =
	INIT_ESERIAL_STRUCTURE(5, "eserial4");
#endif
#if defined(CONFIG_SYS_NS16550_COM6)
DECLARE_ESERIAL_FUNCTIONS(6);
struct serial_device eserial6_device =
	INIT_ESERIAL_STRUCTURE(6, "eserial5");
#endif

__weak struct serial_device *default_serial_console(void)
{
#if CONFIG_CONS_INDEX == 1
	return &eserial1_device;
#elif CONFIG_CONS_INDEX == 2
	return &eserial2_device;
#elif CONFIG_CONS_INDEX == 3
	return &eserial3_device;
#elif CONFIG_CONS_INDEX == 4
	return &eserial4_device;
#elif CONFIG_CONS_INDEX == 5
	return &eserial5_device;
#elif CONFIG_CONS_INDEX == 6
	return &eserial6_device;
#else
#error "Bad CONFIG_CONS_INDEX."
#endif
}

void ns16550_serial_initialize(void)
{
#if defined(CONFIG_SYS_NS16550_COM1)
	serial_register(&eserial1_device);
#endif
#if defined(CONFIG_SYS_NS16550_COM2)
	serial_register(&eserial2_device);
#endif
#if defined(CONFIG_SYS_NS16550_COM3)
	serial_register(&eserial3_device);
#endif
#if defined(CONFIG_SYS_NS16550_COM4)
	serial_register(&eserial4_device);
#endif
#if defined(CONFIG_SYS_NS16550_COM5)
	serial_register(&eserial5_device);
#endif
#if defined(CONFIG_SYS_NS16550_COM6)
	serial_register(&eserial6_device);
#endif
}

#endif /* !CONFIG_NS16550_MIN_FUNCTIONS */
