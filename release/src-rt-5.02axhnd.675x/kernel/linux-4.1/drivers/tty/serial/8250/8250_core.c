/*
 *  Driver for 8250/16550-type serial ports
 *
 *  Based on drivers/char/serial.c, by Linus Torvalds, Theodore Ts'o.
 *
 *  Copyright (C) 2001 Russell King.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * A note about mapbase / membase
 *
 *  mapbase is the physical address of the IO port.
 *  membase is an 'ioremapped' cookie.
 */

#if defined(CONFIG_SERIAL_8250_CONSOLE) && defined(CONFIG_MAGIC_SYSRQ)
#if defined(CONFIG_BCM_KF_CHAR_SYSRQ)
static char sysrq_start_char='^';
#endif
#define SUPPORT_SYSRQ
#endif

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/ioport.h>
#include <linux/init.h>
#include <linux/console.h>
#include <linux/sysrq.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/tty.h>
#include <linux/ratelimit.h>
#include <linux/tty_flip.h>
#include <linux/serial.h>
#include <linux/serial_8250.h>
#include <linux/nmi.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/pm_runtime.h>
#ifdef CONFIG_SPARC
#include <linux/sunserialcore.h>
#endif

#include <asm/io.h>
#include <asm/irq.h>

#include "8250.h"

/*
 * Configuration:
 *   share_irqs - whether we pass IRQF_SHARED to request_irq().  This option
 *                is unsafe when used on edge-triggered interrupts.
 */
static unsigned int share_irqs = SERIAL8250_SHARE_IRQS;

static unsigned int nr_uarts = CONFIG_SERIAL_8250_RUNTIME_UARTS;

static struct uart_driver serial8250_reg;

static int serial_index(struct uart_port *port)
{
	return port->minor - 64;
}

static unsigned int skip_txen_test; /* force skip of txen test at init time */

/*
 * Debugging.
 */
#if 0
#define DEBUG_AUTOCONF(fmt...)	printk(fmt)
#else
#define DEBUG_AUTOCONF(fmt...)	do { } while (0)
#endif

#if 0
#define DEBUG_INTR(fmt...)	printk(fmt)
#else
#define DEBUG_INTR(fmt...)	do { } while (0)
#endif

#define PASS_LIMIT	512

#define BOTH_EMPTY 	(UART_LSR_TEMT | UART_LSR_THRE)


#ifdef CONFIG_SERIAL_8250_DETECT_IRQ
#define CONFIG_SERIAL_DETECT_IRQ 1
#endif
#ifdef CONFIG_SERIAL_8250_MANY_PORTS
#define CONFIG_SERIAL_MANY_PORTS 1
#endif

/*
 * HUB6 is always on.  This will be removed once the header
 * files have been cleaned.
 */
#define CONFIG_HUB6 1

#include <asm/serial.h>
/*
 * SERIAL_PORT_DFNS tells us about built-in ports that have no
 * standard enumeration mechanism.   Platforms that can find all
 * serial ports via mechanisms like ACPI or PCI need not supply it.
 */
#ifndef SERIAL_PORT_DFNS
#define SERIAL_PORT_DFNS
#endif

static const struct old_serial_port old_serial_port[] = {
	SERIAL_PORT_DFNS /* defined in asm/serial.h */
};

#define UART_NR	CONFIG_SERIAL_8250_NR_UARTS

#ifdef CONFIG_SERIAL_8250_RSA

#define PORT_RSA_MAX 4
static unsigned long probe_rsa[PORT_RSA_MAX];
static unsigned int probe_rsa_count;
#endif /* CONFIG_SERIAL_8250_RSA  */

struct irq_info {
	struct			hlist_node node;
	int			irq;
	spinlock_t		lock;	/* Protects list not the hash */
	struct list_head	*head;
};

#define NR_IRQ_HASH		32	/* Can be adjusted later */
static struct hlist_head irq_lists[NR_IRQ_HASH];
static DEFINE_MUTEX(hash_mutex);	/* Used to walk the hash */

/*
 * Here we define the default xmit fifo size used for each type of UART.
 */
static const struct serial8250_config uart_config[] = {
	[PORT_UNKNOWN] = {
		.name		= "unknown",
		.fifo_size	= 1,
		.tx_loadsz	= 1,
	},
	[PORT_8250] = {
		.name		= "8250",
		.fifo_size	= 1,
		.tx_loadsz	= 1,
	},
	[PORT_16450] = {
		.name		= "16450",
		.fifo_size	= 1,
		.tx_loadsz	= 1,
	},
	[PORT_16550] = {
		.name		= "16550",
		.fifo_size	= 1,
		.tx_loadsz	= 1,
	},
	[PORT_16550A] = {
		.name		= "16550A",
		.fifo_size	= 16,
		.tx_loadsz	= 16,
		.fcr		= UART_FCR_ENABLE_FIFO | UART_FCR_R_TRIG_10,
		.rxtrig_bytes	= {1, 4, 8, 14},
		.flags		= UART_CAP_FIFO,
	},
	[PORT_CIRRUS] = {
		.name		= "Cirrus",
		.fifo_size	= 1,
		.tx_loadsz	= 1,
	},
	[PORT_16650] = {
		.name		= "ST16650",
		.fifo_size	= 1,
		.tx_loadsz	= 1,
		.flags		= UART_CAP_FIFO | UART_CAP_EFR | UART_CAP_SLEEP,
	},
	[PORT_16650V2] = {
		.name		= "ST16650V2",
		.fifo_size	= 32,
		.tx_loadsz	= 16,
		.fcr		= UART_FCR_ENABLE_FIFO | UART_FCR_R_TRIG_01 |
				  UART_FCR_T_TRIG_00,
		.rxtrig_bytes	= {8, 16, 24, 28},
		.flags		= UART_CAP_FIFO | UART_CAP_EFR | UART_CAP_SLEEP,
	},
	[PORT_16750] = {
		.name		= "TI16750",
		.fifo_size	= 64,
		.tx_loadsz	= 64,
		.fcr		= UART_FCR_ENABLE_FIFO | UART_FCR_R_TRIG_10 |
				  UART_FCR7_64BYTE,
		.rxtrig_bytes	= {1, 16, 32, 56},
		.flags		= UART_CAP_FIFO | UART_CAP_SLEEP | UART_CAP_AFE,
	},
	[PORT_STARTECH] = {
		.name		= "Startech",
		.fifo_size	= 1,
		.tx_loadsz	= 1,
	},
	[PORT_16C950] = {
		.name		= "16C950/954",
		.fifo_size	= 128,
		.tx_loadsz	= 128,
		.fcr		= UART_FCR_ENABLE_FIFO | UART_FCR_R_TRIG_10,
		/* UART_CAP_EFR breaks billionon CF bluetooth card. */
		.flags		= UART_CAP_FIFO | UART_CAP_SLEEP,
	},
	[PORT_16654] = {
		.name		= "ST16654",
		.fifo_size	= 64,
		.tx_loadsz	= 32,
		.fcr		= UART_FCR_ENABLE_FIFO | UART_FCR_R_TRIG_01 |
				  UART_FCR_T_TRIG_10,
		.rxtrig_bytes	= {8, 16, 56, 60},
		.flags		= UART_CAP_FIFO | UART_CAP_EFR | UART_CAP_SLEEP,
	},
	[PORT_16850] = {
		.name		= "XR16850",
		.fifo_size	= 128,
		.tx_loadsz	= 128,
		.fcr		= UART_FCR_ENABLE_FIFO | UART_FCR_R_TRIG_10,
		.flags		= UART_CAP_FIFO | UART_CAP_EFR | UART_CAP_SLEEP,
	},
	[PORT_RSA] = {
		.name		= "RSA",
		.fifo_size	= 2048,
		.tx_loadsz	= 2048,
		.fcr		= UART_FCR_ENABLE_FIFO | UART_FCR_R_TRIG_11,
		.flags		= UART_CAP_FIFO,
	},
	[PORT_NS16550A] = {
		.name		= "NS16550A",
		.fifo_size	= 16,
		.tx_loadsz	= 16,
		.fcr		= UART_FCR_ENABLE_FIFO | UART_FCR_R_TRIG_10,
		.flags		= UART_CAP_FIFO | UART_NATSEMI,
	},
	[PORT_XSCALE] = {
		.name		= "XScale",
		.fifo_size	= 32,
		.tx_loadsz	= 32,
		.fcr		= UART_FCR_ENABLE_FIFO | UART_FCR_R_TRIG_10,
		.flags		= UART_CAP_FIFO | UART_CAP_UUE | UART_CAP_RTOIE,
	},
	[PORT_OCTEON] = {
		.name		= "OCTEON",
		.fifo_size	= 64,
		.tx_loadsz	= 64,
		.fcr		= UART_FCR_ENABLE_FIFO | UART_FCR_R_TRIG_10,
		.flags		= UART_CAP_FIFO,
	},
	[PORT_AR7] = {
		.name		= "AR7",
		.fifo_size	= 16,
		.tx_loadsz	= 16,
		.fcr		= UART_FCR_ENABLE_FIFO | UART_FCR_R_TRIG_00,
		.flags		= UART_CAP_FIFO | UART_CAP_AFE,
	},
	[PORT_U6_16550A] = {
		.name		= "U6_16550A",
		.fifo_size	= 64,
		.tx_loadsz	= 64,
		.fcr		= UART_FCR_ENABLE_FIFO | UART_FCR_R_TRIG_10,
		.flags		= UART_CAP_FIFO | UART_CAP_AFE,
	},
	[PORT_TEGRA] = {
		.name		= "Tegra",
		.fifo_size	= 32,
		.tx_loadsz	= 8,
		.fcr		= UART_FCR_ENABLE_FIFO | UART_FCR_R_TRIG_01 |
				  UART_FCR_T_TRIG_01,
		.rxtrig_bytes	= {1, 4, 8, 14},
		.flags		= UART_CAP_FIFO | UART_CAP_RTOIE,
	},
	[PORT_XR17D15X] = {
		.name		= "XR17D15X",
		.fifo_size	= 64,
		.tx_loadsz	= 64,
		.fcr		= UART_FCR_ENABLE_FIFO | UART_FCR_R_TRIG_10,
		.flags		= UART_CAP_FIFO | UART_CAP_AFE | UART_CAP_EFR |
				  UART_CAP_SLEEP,
	},
	[PORT_XR17V35X] = {
		.name		= "XR17V35X",
		.fifo_size	= 256,
		.tx_loadsz	= 256,
		.fcr		= UART_FCR_ENABLE_FIFO | UART_FCR_R_TRIG_11 |
				  UART_FCR_T_TRIG_11,
		.flags		= UART_CAP_FIFO | UART_CAP_AFE | UART_CAP_EFR |
				  UART_CAP_SLEEP,
	},
	[PORT_LPC3220] = {
		.name		= "LPC3220",
		.fifo_size	= 64,
		.tx_loadsz	= 32,
		.fcr		= UART_FCR_DMA_SELECT | UART_FCR_ENABLE_FIFO |
				  UART_FCR_R_TRIG_00 | UART_FCR_T_TRIG_00,
		.flags		= UART_CAP_FIFO,
	},
	[PORT_BRCM_TRUMANAGE] = {
		.name		= "TruManage",
		.fifo_size	= 1,
		.tx_loadsz	= 1024,
		.flags		= UART_CAP_HFIFO,
	},
	[PORT_8250_CIR] = {
		.name		= "CIR port"
	},
	[PORT_ALTR_16550_F32] = {
		.name		= "Altera 16550 FIFO32",
		.fifo_size	= 32,
		.tx_loadsz	= 32,
		.fcr		= UART_FCR_ENABLE_FIFO | UART_FCR_R_TRIG_10,
		.flags		= UART_CAP_FIFO | UART_CAP_AFE,
	},
	[PORT_ALTR_16550_F64] = {
		.name		= "Altera 16550 FIFO64",
		.fifo_size	= 64,
		.tx_loadsz	= 64,
		.fcr		= UART_FCR_ENABLE_FIFO | UART_FCR_R_TRIG_10,
		.flags		= UART_CAP_FIFO | UART_CAP_AFE,
	},
	[PORT_ALTR_16550_F128] = {
		.name		= "Altera 16550 FIFO128",
		.fifo_size	= 128,
		.tx_loadsz	= 128,
		.fcr		= UART_FCR_ENABLE_FIFO | UART_FCR_R_TRIG_10,
		.flags		= UART_CAP_FIFO | UART_CAP_AFE,
	},
/* tx_loadsz is set to 63-bytes instead of 64-bytes to implement
workaround of errata A-008006 which states that tx_loadsz should  be
configured less than Maximum supported fifo bytes */
	[PORT_16550A_FSL64] = {
		.name		= "16550A_FSL64",
		.fifo_size	= 64,
		.tx_loadsz	= 63,
		.fcr		= UART_FCR_ENABLE_FIFO | UART_FCR_R_TRIG_10 |
				  UART_FCR7_64BYTE,
		.flags		= UART_CAP_FIFO,
	},
	[PORT_RT2880] = {
		.name		= "Palmchip BK-3103",
		.fifo_size	= 16,
		.tx_loadsz	= 16,
		.fcr		= UART_FCR_ENABLE_FIFO | UART_FCR_R_TRIG_10,
		.rxtrig_bytes	= {1, 4, 8, 14},
		.flags		= UART_CAP_FIFO,
	},
};

/* Uart divisor latch read */
static int default_serial_dl_read(struct uart_8250_port *up)
{
	return serial_in(up, UART_DLL) | serial_in(up, UART_DLM) << 8;
}

/* Uart divisor latch write */
static void default_serial_dl_write(struct uart_8250_port *up, int value)
{
	serial_out(up, UART_DLL, value & 0xff);
	serial_out(up, UART_DLM, value >> 8 & 0xff);
}

#if defined(CONFIG_MIPS_ALCHEMY) || defined(CONFIG_SERIAL_8250_RT288X)

/* Au1x00/RT288x UART hardware has a weird register layout */
static const s8 au_io_in_map[8] = {
	 0,	/* UART_RX  */
	 2,	/* UART_IER */
	 3,	/* UART_IIR */
	 5,	/* UART_LCR */
	 6,	/* UART_MCR */
	 7,	/* UART_LSR */
	 8,	/* UART_MSR */
	-1,	/* UART_SCR (unmapped) */
};

static const s8 au_io_out_map[8] = {
	 1,	/* UART_TX  */
	 2,	/* UART_IER */
	 4,	/* UART_FCR */
	 5,	/* UART_LCR */
	 6,	/* UART_MCR */
	-1,	/* UART_LSR (unmapped) */
	-1,	/* UART_MSR (unmapped) */
	-1,	/* UART_SCR (unmapped) */
};

static unsigned int au_serial_in(struct uart_port *p, int offset)
{
	if (offset >= ARRAY_SIZE(au_io_in_map))
		return UINT_MAX;
	offset = au_io_in_map[offset];
	if (offset < 0)
		return UINT_MAX;
	return __raw_readl(p->membase + (offset << p->regshift));
}

static void au_serial_out(struct uart_port *p, int offset, int value)
{
	if (offset >= ARRAY_SIZE(au_io_out_map))
		return;
	offset = au_io_out_map[offset];
	if (offset < 0)
		return;
	__raw_writel(value, p->membase + (offset << p->regshift));
}

/* Au1x00 haven't got a standard divisor latch */
static int au_serial_dl_read(struct uart_8250_port *up)
{
	return __raw_readl(up->port.membase + 0x28);
}

static void au_serial_dl_write(struct uart_8250_port *up, int value)
{
	__raw_writel(value, up->port.membase + 0x28);
}

#endif

static unsigned int hub6_serial_in(struct uart_port *p, int offset)
{
	offset = offset << p->regshift;
	outb(p->hub6 - 1 + offset, p->iobase);
	return inb(p->iobase + 1);
}

static void hub6_serial_out(struct uart_port *p, int offset, int value)
{
	offset = offset << p->regshift;
	outb(p->hub6 - 1 + offset, p->iobase);
	outb(value, p->iobase + 1);
}

static unsigned int mem_serial_in(struct uart_port *p, int offset)
{
	offset = offset << p->regshift;
	return readb(p->membase + offset);
}

static void mem_serial_out(struct uart_port *p, int offset, int value)
{
	offset = offset << p->regshift;
	writeb(value, p->membase + offset);
}

static void mem32_serial_out(struct uart_port *p, int offset, int value)
{
	offset = offset << p->regshift;
	writel(value, p->membase + offset);
}

static unsigned int mem32_serial_in(struct uart_port *p, int offset)
{
	offset = offset << p->regshift;
	return readl(p->membase + offset);
}

static void mem32be_serial_out(struct uart_port *p, int offset, int value)
{
	offset = offset << p->regshift;
	iowrite32be(value, p->membase + offset);
}

static unsigned int mem32be_serial_in(struct uart_port *p, int offset)
{
	offset = offset << p->regshift;
	return ioread32be(p->membase + offset);
}

static unsigned int io_serial_in(struct uart_port *p, int offset)
{
	offset = offset << p->regshift;
	return inb(p->iobase + offset);
}

static void io_serial_out(struct uart_port *p, int offset, int value)
{
	offset = offset << p->regshift;
	outb(value, p->iobase + offset);
}

static int serial8250_default_handle_irq(struct uart_port *port);
static int exar_handle_irq(struct uart_port *port);

static void set_io_from_upio(struct uart_port *p)
{
	struct uart_8250_port *up = up_to_u8250p(p);

	up->dl_read = default_serial_dl_read;
	up->dl_write = default_serial_dl_write;

	switch (p->iotype) {
	case UPIO_HUB6:
		p->serial_in = hub6_serial_in;
		p->serial_out = hub6_serial_out;
		break;

	case UPIO_MEM:
		p->serial_in = mem_serial_in;
		p->serial_out = mem_serial_out;
		break;

	case UPIO_MEM32:
		p->serial_in = mem32_serial_in;
		p->serial_out = mem32_serial_out;
		break;

	case UPIO_MEM32BE:
		p->serial_in = mem32be_serial_in;
		p->serial_out = mem32be_serial_out;
		break;

#if defined(CONFIG_MIPS_ALCHEMY) || defined(CONFIG_SERIAL_8250_RT288X)
	case UPIO_AU:
		p->serial_in = au_serial_in;
		p->serial_out = au_serial_out;
		up->dl_read = au_serial_dl_read;
		up->dl_write = au_serial_dl_write;
		break;
#endif

	default:
		p->serial_in = io_serial_in;
		p->serial_out = io_serial_out;
		break;
	}
	/* Remember loaded iotype */
	up->cur_iotype = p->iotype;
	p->handle_irq = serial8250_default_handle_irq;
}

static void
serial_port_out_sync(struct uart_port *p, int offset, int value)
{
	switch (p->iotype) {
	case UPIO_MEM:
	case UPIO_MEM32:
	case UPIO_MEM32BE:
	case UPIO_AU:
		p->serial_out(p, offset, value);
		p->serial_in(p, UART_LCR);	/* safe, no side-effects */
		break;
	default:
		p->serial_out(p, offset, value);
	}
}

/*
 * For the 16C950
 */
static void serial_icr_write(struct uart_8250_port *up, int offset, int value)
{
	serial_out(up, UART_SCR, offset);
	serial_out(up, UART_ICR, value);
}

static unsigned int serial_icr_read(struct uart_8250_port *up, int offset)
{
	unsigned int value;

	serial_icr_write(up, UART_ACR, up->acr | UART_ACR_ICRRD);
	serial_out(up, UART_SCR, offset);
	value = serial_in(up, UART_ICR);
	serial_icr_write(up, UART_ACR, up->acr);

	return value;
}

/*
 * FIFO support.
 */
static void serial8250_clear_fifos(struct uart_8250_port *p)
{
	if (p->capabilities & UART_CAP_FIFO) {
		serial_out(p, UART_FCR, UART_FCR_ENABLE_FIFO);
		serial_out(p, UART_FCR, UART_FCR_ENABLE_FIFO |
			       UART_FCR_CLEAR_RCVR | UART_FCR_CLEAR_XMIT);
		serial_out(p, UART_FCR, 0);
	}
}

void serial8250_clear_and_reinit_fifos(struct uart_8250_port *p)
{
	serial8250_clear_fifos(p);
	serial_out(p, UART_FCR, p->fcr);
}
EXPORT_SYMBOL_GPL(serial8250_clear_and_reinit_fifos);

void serial8250_rpm_get(struct uart_8250_port *p)
{
	if (!(p->capabilities & UART_CAP_RPM))
		return;
	pm_runtime_get_sync(p->port.dev);
}
EXPORT_SYMBOL_GPL(serial8250_rpm_get);

void serial8250_rpm_put(struct uart_8250_port *p)
{
	if (!(p->capabilities & UART_CAP_RPM))
		return;
	pm_runtime_mark_last_busy(p->port.dev);
	pm_runtime_put_autosuspend(p->port.dev);
}
EXPORT_SYMBOL_GPL(serial8250_rpm_put);

/*
 * These two wrappers ensure that enable_runtime_pm_tx() can be called more than
 * once and disable_runtime_pm_tx() will still disable RPM because the fifo is
 * empty and the HW can idle again.
 */
static void serial8250_rpm_get_tx(struct uart_8250_port *p)
{
	unsigned char rpm_active;

	if (!(p->capabilities & UART_CAP_RPM))
		return;

	rpm_active = xchg(&p->rpm_tx_active, 1);
	if (rpm_active)
		return;
	pm_runtime_get_sync(p->port.dev);
}

static void serial8250_rpm_put_tx(struct uart_8250_port *p)
{
	unsigned char rpm_active;

	if (!(p->capabilities & UART_CAP_RPM))
		return;

	rpm_active = xchg(&p->rpm_tx_active, 0);
	if (!rpm_active)
		return;
	pm_runtime_mark_last_busy(p->port.dev);
	pm_runtime_put_autosuspend(p->port.dev);
}

/*
 * IER sleep support.  UARTs which have EFRs need the "extended
 * capability" bit enabled.  Note that on XR16C850s, we need to
 * reset LCR to write to IER.
 */
static void serial8250_set_sleep(struct uart_8250_port *p, int sleep)
{
	unsigned char lcr = 0, efr = 0;
	/*
	 * Exar UARTs have a SLEEP register that enables or disables
	 * each UART to enter sleep mode separately.  On the XR17V35x the
	 * register is accessible to each UART at the UART_EXAR_SLEEP
	 * offset but the UART channel may only write to the corresponding
	 * bit.
	 */
	serial8250_rpm_get(p);
	if ((p->port.type == PORT_XR17V35X) ||
	   (p->port.type == PORT_XR17D15X)) {
		serial_out(p, UART_EXAR_SLEEP, sleep ? 0xff : 0);
		goto out;
	}

	if (p->capabilities & UART_CAP_SLEEP) {
		if (p->capabilities & UART_CAP_EFR) {
			lcr = serial_in(p, UART_LCR);
			efr = serial_in(p, UART_EFR);
			serial_out(p, UART_LCR, UART_LCR_CONF_MODE_B);
			serial_out(p, UART_EFR, UART_EFR_ECB);
			serial_out(p, UART_LCR, 0);
		}
		serial_out(p, UART_IER, sleep ? UART_IERX_SLEEP : 0);
		if (p->capabilities & UART_CAP_EFR) {
			serial_out(p, UART_LCR, UART_LCR_CONF_MODE_B);
			serial_out(p, UART_EFR, efr);
			serial_out(p, UART_LCR, lcr);
		}
	}
out:
	serial8250_rpm_put(p);
}

#ifdef CONFIG_SERIAL_8250_RSA
/*
 * Attempts to turn on the RSA FIFO.  Returns zero on failure.
 * We set the port uart clock rate if we succeed.
 */
static int __enable_rsa(struct uart_8250_port *up)
{
	unsigned char mode;
	int result;

	mode = serial_in(up, UART_RSA_MSR);
	result = mode & UART_RSA_MSR_FIFO;

	if (!result) {
		serial_out(up, UART_RSA_MSR, mode | UART_RSA_MSR_FIFO);
		mode = serial_in(up, UART_RSA_MSR);
		result = mode & UART_RSA_MSR_FIFO;
	}

	if (result)
		up->port.uartclk = SERIAL_RSA_BAUD_BASE * 16;

	return result;
}

static void enable_rsa(struct uart_8250_port *up)
{
	if (up->port.type == PORT_RSA) {
		if (up->port.uartclk != SERIAL_RSA_BAUD_BASE * 16) {
			spin_lock_irq(&up->port.lock);
			__enable_rsa(up);
			spin_unlock_irq(&up->port.lock);
		}
		if (up->port.uartclk == SERIAL_RSA_BAUD_BASE * 16)
			serial_out(up, UART_RSA_FRR, 0);
	}
}

/*
 * Attempts to turn off the RSA FIFO.  Returns zero on failure.
 * It is unknown why interrupts were disabled in here.  However,
 * the caller is expected to preserve this behaviour by grabbing
 * the spinlock before calling this function.
 */
static void disable_rsa(struct uart_8250_port *up)
{
	unsigned char mode;
	int result;

	if (up->port.type == PORT_RSA &&
	    up->port.uartclk == SERIAL_RSA_BAUD_BASE * 16) {
		spin_lock_irq(&up->port.lock);

		mode = serial_in(up, UART_RSA_MSR);
		result = !(mode & UART_RSA_MSR_FIFO);

		if (!result) {
			serial_out(up, UART_RSA_MSR, mode & ~UART_RSA_MSR_FIFO);
			mode = serial_in(up, UART_RSA_MSR);
			result = !(mode & UART_RSA_MSR_FIFO);
		}

		if (result)
			up->port.uartclk = SERIAL_RSA_BAUD_BASE_LO * 16;
		spin_unlock_irq(&up->port.lock);
	}
}
#endif /* CONFIG_SERIAL_8250_RSA */

/*
 * This is a quickie test to see how big the FIFO is.
 * It doesn't work at all the time, more's the pity.
 */
static int size_fifo(struct uart_8250_port *up)
{
	unsigned char old_fcr, old_mcr, old_lcr;
	unsigned short old_dl;
	int count;

	old_lcr = serial_in(up, UART_LCR);
	serial_out(up, UART_LCR, 0);
	old_fcr = serial_in(up, UART_FCR);
	old_mcr = serial_in(up, UART_MCR);
	serial_out(up, UART_FCR, UART_FCR_ENABLE_FIFO |
		    UART_FCR_CLEAR_RCVR | UART_FCR_CLEAR_XMIT);
	serial_out(up, UART_MCR, UART_MCR_LOOP);
	serial_out(up, UART_LCR, UART_LCR_CONF_MODE_A);
	old_dl = serial_dl_read(up);
	serial_dl_write(up, 0x0001);
	serial_out(up, UART_LCR, 0x03);
	for (count = 0; count < 256; count++)
		serial_out(up, UART_TX, count);
	mdelay(20);/* FIXME - schedule_timeout */
	for (count = 0; (serial_in(up, UART_LSR) & UART_LSR_DR) &&
	     (count < 256); count++)
		serial_in(up, UART_RX);
	serial_out(up, UART_FCR, old_fcr);
	serial_out(up, UART_MCR, old_mcr);
	serial_out(up, UART_LCR, UART_LCR_CONF_MODE_A);
	serial_dl_write(up, old_dl);
	serial_out(up, UART_LCR, old_lcr);

	return count;
}

/*
 * Read UART ID using the divisor method - set DLL and DLM to zero
 * and the revision will be in DLL and device type in DLM.  We
 * preserve the device state across this.
 */
static unsigned int autoconfig_read_divisor_id(struct uart_8250_port *p)
{
	unsigned char old_lcr;
	unsigned int id, old_dl;

	old_lcr = serial_in(p, UART_LCR);
	serial_out(p, UART_LCR, UART_LCR_CONF_MODE_A);
	old_dl = serial_dl_read(p);
	serial_dl_write(p, 0);
	id = serial_dl_read(p);
	serial_dl_write(p, old_dl);

	serial_out(p, UART_LCR, old_lcr);

	return id;
}

/*
 * This is a helper routine to autodetect StarTech/Exar/Oxsemi UART's.
 * When this function is called we know it is at least a StarTech
 * 16650 V2, but it might be one of several StarTech UARTs, or one of
 * its clones.  (We treat the broken original StarTech 16650 V1 as a
 * 16550, and why not?  Startech doesn't seem to even acknowledge its
 * existence.)
 *
 * What evil have men's minds wrought...
 */
static void autoconfig_has_efr(struct uart_8250_port *up)
{
	unsigned int id1, id2, id3, rev;

	/*
	 * Everything with an EFR has SLEEP
	 */
	up->capabilities |= UART_CAP_EFR | UART_CAP_SLEEP;

	/*
	 * First we check to see if it's an Oxford Semiconductor UART.
	 *
	 * If we have to do this here because some non-National
	 * Semiconductor clone chips lock up if you try writing to the
	 * LSR register (which serial_icr_read does)
	 */

	/*
	 * Check for Oxford Semiconductor 16C950.
	 *
	 * EFR [4] must be set else this test fails.
	 *
	 * This shouldn't be necessary, but Mike Hudson (Exoray@isys.ca)
	 * claims that it's needed for 952 dual UART's (which are not
	 * recommended for new designs).
	 */
	up->acr = 0;
	serial_out(up, UART_LCR, UART_LCR_CONF_MODE_B);
	serial_out(up, UART_EFR, UART_EFR_ECB);
	serial_out(up, UART_LCR, 0x00);
	id1 = serial_icr_read(up, UART_ID1);
	id2 = serial_icr_read(up, UART_ID2);
	id3 = serial_icr_read(up, UART_ID3);
	rev = serial_icr_read(up, UART_REV);

	DEBUG_AUTOCONF("950id=%02x:%02x:%02x:%02x ", id1, id2, id3, rev);

	if (id1 == 0x16 && id2 == 0xC9 &&
	    (id3 == 0x50 || id3 == 0x52 || id3 == 0x54)) {
		up->port.type = PORT_16C950;

		/*
		 * Enable work around for the Oxford Semiconductor 952 rev B
		 * chip which causes it to seriously miscalculate baud rates
		 * when DLL is 0.
		 */
		if (id3 == 0x52 && rev == 0x01)
			up->bugs |= UART_BUG_QUOT;
		return;
	}

	/*
	 * We check for a XR16C850 by setting DLL and DLM to 0, and then
	 * reading back DLL and DLM.  The chip type depends on the DLM
	 * value read back:
	 *  0x10 - XR16C850 and the DLL contains the chip revision.
	 *  0x12 - XR16C2850.
	 *  0x14 - XR16C854.
	 */
	id1 = autoconfig_read_divisor_id(up);
	DEBUG_AUTOCONF("850id=%04x ", id1);

	id2 = id1 >> 8;
	if (id2 == 0x10 || id2 == 0x12 || id2 == 0x14) {
		up->port.type = PORT_16850;
		return;
	}

	/*
	 * It wasn't an XR16C850.
	 *
	 * We distinguish between the '654 and the '650 by counting
	 * how many bytes are in the FIFO.  I'm using this for now,
	 * since that's the technique that was sent to me in the
	 * serial driver update, but I'm not convinced this works.
	 * I've had problems doing this in the past.  -TYT
	 */
	if (size_fifo(up) == 64)
		up->port.type = PORT_16654;
	else
		up->port.type = PORT_16650V2;
}

/*
 * We detected a chip without a FIFO.  Only two fall into
 * this category - the original 8250 and the 16450.  The
 * 16450 has a scratch register (accessible with LCR=0)
 */
static void autoconfig_8250(struct uart_8250_port *up)
{
	unsigned char scratch, status1, status2;

	up->port.type = PORT_8250;

	scratch = serial_in(up, UART_SCR);
	serial_out(up, UART_SCR, 0xa5);
	status1 = serial_in(up, UART_SCR);
	serial_out(up, UART_SCR, 0x5a);
	status2 = serial_in(up, UART_SCR);
	serial_out(up, UART_SCR, scratch);

	if (status1 == 0xa5 && status2 == 0x5a)
		up->port.type = PORT_16450;
}

static int broken_efr(struct uart_8250_port *up)
{
	/*
	 * Exar ST16C2550 "A2" devices incorrectly detect as
	 * having an EFR, and report an ID of 0x0201.  See
	 * http://linux.derkeiler.com/Mailing-Lists/Kernel/2004-11/4812.html
	 */
	if (autoconfig_read_divisor_id(up) == 0x0201 && size_fifo(up) == 16)
		return 1;

	return 0;
}

/*
 * We know that the chip has FIFOs.  Does it have an EFR?  The
 * EFR is located in the same register position as the IIR and
 * we know the top two bits of the IIR are currently set.  The
 * EFR should contain zero.  Try to read the EFR.
 */
static void autoconfig_16550a(struct uart_8250_port *up)
{
	unsigned char status1, status2;
	unsigned int iersave;

	up->port.type = PORT_16550A;
	up->capabilities |= UART_CAP_FIFO;

	/*
	 * XR17V35x UARTs have an extra divisor register, DLD
	 * that gets enabled with when DLAB is set which will
	 * cause the device to incorrectly match and assign
	 * port type to PORT_16650.  The EFR for this UART is
	 * found at offset 0x09. Instead check the Deice ID (DVID)
	 * register for a 2, 4 or 8 port UART.
	 */
	if (up->port.flags & UPF_EXAR_EFR) {
		status1 = serial_in(up, UART_EXAR_DVID);
		if (status1 == 0x82 || status1 == 0x84 || status1 == 0x88) {
			DEBUG_AUTOCONF("Exar XR17V35x ");
			up->port.type = PORT_XR17V35X;
			up->capabilities |= UART_CAP_AFE | UART_CAP_EFR |
						UART_CAP_SLEEP;

			return;
		}

	}

	/*
	 * Check for presence of the EFR when DLAB is set.
	 * Only ST16C650V1 UARTs pass this test.
	 */
	serial_out(up, UART_LCR, UART_LCR_CONF_MODE_A);
	if (serial_in(up, UART_EFR) == 0) {
		serial_out(up, UART_EFR, 0xA8);
		if (serial_in(up, UART_EFR) != 0) {
			DEBUG_AUTOCONF("EFRv1 ");
			up->port.type = PORT_16650;
			up->capabilities |= UART_CAP_EFR | UART_CAP_SLEEP;
		} else {
			serial_out(up, UART_LCR, 0);
			serial_out(up, UART_FCR, UART_FCR_ENABLE_FIFO |
				   UART_FCR7_64BYTE);
			status1 = serial_in(up, UART_IIR) >> 5;
			serial_out(up, UART_FCR, 0);
			serial_out(up, UART_LCR, 0);

			if (status1 == 7)
				up->port.type = PORT_16550A_FSL64;
			else
				DEBUG_AUTOCONF("Motorola 8xxx DUART ");
		}
		serial_out(up, UART_EFR, 0);
		return;
	}

	/*
	 * Maybe it requires 0xbf to be written to the LCR.
	 * (other ST16C650V2 UARTs, TI16C752A, etc)
	 */
	serial_out(up, UART_LCR, UART_LCR_CONF_MODE_B);
	if (serial_in(up, UART_EFR) == 0 && !broken_efr(up)) {
		DEBUG_AUTOCONF("EFRv2 ");
		autoconfig_has_efr(up);
		return;
	}

	/*
	 * Check for a National Semiconductor SuperIO chip.
	 * Attempt to switch to bank 2, read the value of the LOOP bit
	 * from EXCR1. Switch back to bank 0, change it in MCR. Then
	 * switch back to bank 2, read it from EXCR1 again and check
	 * it's changed. If so, set baud_base in EXCR2 to 921600. -- dwmw2
	 */
	serial_out(up, UART_LCR, 0);
	status1 = serial_in(up, UART_MCR);
	serial_out(up, UART_LCR, 0xE0);
	status2 = serial_in(up, 0x02); /* EXCR1 */

	if (!((status2 ^ status1) & UART_MCR_LOOP)) {
		serial_out(up, UART_LCR, 0);
		serial_out(up, UART_MCR, status1 ^ UART_MCR_LOOP);
		serial_out(up, UART_LCR, 0xE0);
		status2 = serial_in(up, 0x02); /* EXCR1 */
		serial_out(up, UART_LCR, 0);
		serial_out(up, UART_MCR, status1);

		if ((status2 ^ status1) & UART_MCR_LOOP) {
			unsigned short quot;

			serial_out(up, UART_LCR, 0xE0);

			quot = serial_dl_read(up);
			quot <<= 3;

			if (ns16550a_goto_highspeed(up))
				serial_dl_write(up, quot);

			serial_out(up, UART_LCR, 0);

			up->port.uartclk = 921600*16;
			up->port.type = PORT_NS16550A;
			up->capabilities |= UART_NATSEMI;
			return;
		}
	}

	/*
	 * No EFR.  Try to detect a TI16750, which only sets bit 5 of
	 * the IIR when 64 byte FIFO mode is enabled when DLAB is set.
	 * Try setting it with and without DLAB set.  Cheap clones
	 * set bit 5 without DLAB set.
	 */
	serial_out(up, UART_LCR, 0);
	serial_out(up, UART_FCR, UART_FCR_ENABLE_FIFO | UART_FCR7_64BYTE);
	status1 = serial_in(up, UART_IIR) >> 5;
	serial_out(up, UART_FCR, UART_FCR_ENABLE_FIFO);
	serial_out(up, UART_LCR, UART_LCR_CONF_MODE_A);
	serial_out(up, UART_FCR, UART_FCR_ENABLE_FIFO | UART_FCR7_64BYTE);
	status2 = serial_in(up, UART_IIR) >> 5;
	serial_out(up, UART_FCR, UART_FCR_ENABLE_FIFO);
	serial_out(up, UART_LCR, 0);

	DEBUG_AUTOCONF("iir1=%d iir2=%d ", status1, status2);

	if (status1 == 6 && status2 == 7) {
		up->port.type = PORT_16750;
		up->capabilities |= UART_CAP_AFE | UART_CAP_SLEEP;
		return;
	}

	/*
	 * Try writing and reading the UART_IER_UUE bit (b6).
	 * If it works, this is probably one of the Xscale platform's
	 * internal UARTs.
	 * We're going to explicitly set the UUE bit to 0 before
	 * trying to write and read a 1 just to make sure it's not
	 * already a 1 and maybe locked there before we even start start.
	 */
	iersave = serial_in(up, UART_IER);
	serial_out(up, UART_IER, iersave & ~UART_IER_UUE);
	if (!(serial_in(up, UART_IER) & UART_IER_UUE)) {
		/*
		 * OK it's in a known zero state, try writing and reading
		 * without disturbing the current state of the other bits.
		 */
		serial_out(up, UART_IER, iersave | UART_IER_UUE);
		if (serial_in(up, UART_IER) & UART_IER_UUE) {
			/*
			 * It's an Xscale.
			 * We'll leave the UART_IER_UUE bit set to 1 (enabled).
			 */
			DEBUG_AUTOCONF("Xscale ");
			up->port.type = PORT_XSCALE;
			up->capabilities |= UART_CAP_UUE | UART_CAP_RTOIE;
			return;
		}
	} else {
		/*
		 * If we got here we couldn't force the IER_UUE bit to 0.
		 * Log it and continue.
		 */
		DEBUG_AUTOCONF("Couldn't force IER_UUE to 0 ");
	}
	serial_out(up, UART_IER, iersave);

	/*
	 * Exar uarts have EFR in a weird location
	 */
	if (up->port.flags & UPF_EXAR_EFR) {
		DEBUG_AUTOCONF("Exar XR17D15x ");
		up->port.type = PORT_XR17D15X;
		up->capabilities |= UART_CAP_AFE | UART_CAP_EFR |
				    UART_CAP_SLEEP;

		return;
	}

	/*
	 * We distinguish between 16550A and U6 16550A by counting
	 * how many bytes are in the FIFO.
	 */
	if (up->port.type == PORT_16550A && size_fifo(up) == 64) {
		up->port.type = PORT_U6_16550A;
		up->capabilities |= UART_CAP_AFE;
	}
}

/*
 * This routine is called by rs_init() to initialize a specific serial
 * port.  It determines what type of UART chip this serial port is
 * using: 8250, 16450, 16550, 16550A.  The important question is
 * whether or not this UART is a 16550A or not, since this will
 * determine whether or not we can use its FIFO features or not.
 */
static void autoconfig(struct uart_8250_port *up)
{
	unsigned char status1, scratch, scratch2, scratch3;
	unsigned char save_lcr, save_mcr;
	struct uart_port *port = &up->port;
	unsigned long flags;
	unsigned int old_capabilities;

	if (!port->iobase && !port->mapbase && !port->membase)
		return;

	DEBUG_AUTOCONF("ttyS%d: autoconf (0x%04lx, 0x%p): ",
		       serial_index(port), port->iobase, port->membase);

	/*
	 * We really do need global IRQs disabled here - we're going to
	 * be frobbing the chips IRQ enable register to see if it exists.
	 */
	spin_lock_irqsave(&port->lock, flags);

	up->capabilities = 0;
	up->bugs = 0;

	if (!(port->flags & UPF_BUGGY_UART)) {
		/*
		 * Do a simple existence test first; if we fail this,
		 * there's no point trying anything else.
		 *
		 * 0x80 is used as a nonsense port to prevent against
		 * false positives due to ISA bus float.  The
		 * assumption is that 0x80 is a non-existent port;
		 * which should be safe since include/asm/io.h also
		 * makes this assumption.
		 *
		 * Note: this is safe as long as MCR bit 4 is clear
		 * and the device is in "PC" mode.
		 */
		scratch = serial_in(up, UART_IER);
		serial_out(up, UART_IER, 0);
#ifdef __i386__
		outb(0xff, 0x080);
#endif
		/*
		 * Mask out IER[7:4] bits for test as some UARTs (e.g. TL
		 * 16C754B) allow only to modify them if an EFR bit is set.
		 */
		scratch2 = serial_in(up, UART_IER) & 0x0f;
		serial_out(up, UART_IER, 0x0F);
#ifdef __i386__
		outb(0, 0x080);
#endif
		scratch3 = serial_in(up, UART_IER) & 0x0f;
		serial_out(up, UART_IER, scratch);
		if (scratch2 != 0 || scratch3 != 0x0F) {
			/*
			 * We failed; there's nothing here
			 */
			spin_unlock_irqrestore(&port->lock, flags);
			DEBUG_AUTOCONF("IER test failed (%02x, %02x) ",
				       scratch2, scratch3);
			goto out;
		}
	}

	save_mcr = serial_in(up, UART_MCR);
	save_lcr = serial_in(up, UART_LCR);

	/*
	 * Check to see if a UART is really there.  Certain broken
	 * internal modems based on the Rockwell chipset fail this
	 * test, because they apparently don't implement the loopback
	 * test mode.  So this test is skipped on the COM 1 through
	 * COM 4 ports.  This *should* be safe, since no board
	 * manufacturer would be stupid enough to design a board
	 * that conflicts with COM 1-4 --- we hope!
	 */
	if (!(port->flags & UPF_SKIP_TEST)) {
		serial_out(up, UART_MCR, UART_MCR_LOOP | 0x0A);
		status1 = serial_in(up, UART_MSR) & 0xF0;
		serial_out(up, UART_MCR, save_mcr);
		if (status1 != 0x90) {
			spin_unlock_irqrestore(&port->lock, flags);
			DEBUG_AUTOCONF("LOOP test failed (%02x) ",
				       status1);
			goto out;
		}
	}

	/*
	 * We're pretty sure there's a port here.  Lets find out what
	 * type of port it is.  The IIR top two bits allows us to find
	 * out if it's 8250 or 16450, 16550, 16550A or later.  This
	 * determines what we test for next.
	 *
	 * We also initialise the EFR (if any) to zero for later.  The
	 * EFR occupies the same register location as the FCR and IIR.
	 */
	serial_out(up, UART_LCR, UART_LCR_CONF_MODE_B);
	serial_out(up, UART_EFR, 0);
	serial_out(up, UART_LCR, 0);

	serial_out(up, UART_FCR, UART_FCR_ENABLE_FIFO);
	scratch = serial_in(up, UART_IIR) >> 6;

	switch (scratch) {
	case 0:
		autoconfig_8250(up);
		break;
	case 1:
		port->type = PORT_UNKNOWN;
		break;
	case 2:
		port->type = PORT_16550;
		break;
	case 3:
		autoconfig_16550a(up);
		break;
	}

#ifdef CONFIG_SERIAL_8250_RSA
	/*
	 * Only probe for RSA ports if we got the region.
	 */
	if (port->type == PORT_16550A && up->probe & UART_PROBE_RSA &&
	    __enable_rsa(up))
		port->type = PORT_RSA;
#endif

	serial_out(up, UART_LCR, save_lcr);

	port->fifosize = uart_config[up->port.type].fifo_size;
	old_capabilities = up->capabilities;
	up->capabilities = uart_config[port->type].flags;
	up->tx_loadsz = uart_config[port->type].tx_loadsz;

	if (port->type == PORT_UNKNOWN)
		goto out_lock;

	/*
	 * Reset the UART.
	 */
#ifdef CONFIG_SERIAL_8250_RSA
	if (port->type == PORT_RSA)
		serial_out(up, UART_RSA_FRR, 0);
#endif
	serial_out(up, UART_MCR, save_mcr);
	serial8250_clear_fifos(up);
	serial_in(up, UART_RX);
	if (up->capabilities & UART_CAP_UUE)
		serial_out(up, UART_IER, UART_IER_UUE);
	else
		serial_out(up, UART_IER, 0);

out_lock:
	spin_unlock_irqrestore(&port->lock, flags);
	if (up->capabilities != old_capabilities) {
		printk(KERN_WARNING
		       "ttyS%d: detected caps %08x should be %08x\n",
		       serial_index(port), old_capabilities,
		       up->capabilities);
	}
out:
	DEBUG_AUTOCONF("iir=%d ", scratch);
	DEBUG_AUTOCONF("type=%s\n", uart_config[port->type].name);
}

static void autoconfig_irq(struct uart_8250_port *up)
{
	struct uart_port *port = &up->port;
	unsigned char save_mcr, save_ier;
	unsigned char save_ICP = 0;
	unsigned int ICP = 0;
	unsigned long irqs;
	int irq;

	if (port->flags & UPF_FOURPORT) {
		ICP = (port->iobase & 0xfe0) | 0x1f;
		save_ICP = inb_p(ICP);
		outb_p(0x80, ICP);
		inb_p(ICP);
	}

	/* forget possible initially masked and pending IRQ */
	probe_irq_off(probe_irq_on());
	save_mcr = serial_in(up, UART_MCR);
	save_ier = serial_in(up, UART_IER);
	serial_out(up, UART_MCR, UART_MCR_OUT1 | UART_MCR_OUT2);

	irqs = probe_irq_on();
	serial_out(up, UART_MCR, 0);
	udelay(10);
	if (port->flags & UPF_FOURPORT) {
		serial_out(up, UART_MCR,
			    UART_MCR_DTR | UART_MCR_RTS);
	} else {
		serial_out(up, UART_MCR,
			    UART_MCR_DTR | UART_MCR_RTS | UART_MCR_OUT2);
	}
	serial_out(up, UART_IER, 0x0f);	/* enable all intrs */
	serial_in(up, UART_LSR);
	serial_in(up, UART_RX);
	serial_in(up, UART_IIR);
	serial_in(up, UART_MSR);
	serial_out(up, UART_TX, 0xFF);
	udelay(20);
	irq = probe_irq_off(irqs);

	serial_out(up, UART_MCR, save_mcr);
	serial_out(up, UART_IER, save_ier);

	if (port->flags & UPF_FOURPORT)
		outb_p(save_ICP, ICP);

	port->irq = (irq > 0) ? irq : 0;
}

static inline void __stop_tx(struct uart_8250_port *p)
{
	if (p->ier & UART_IER_THRI) {
		p->ier &= ~UART_IER_THRI;
		serial_out(p, UART_IER, p->ier);
		serial8250_rpm_put_tx(p);
	}
}

static void serial8250_stop_tx(struct uart_port *port)
{
	struct uart_8250_port *up = up_to_u8250p(port);

	serial8250_rpm_get(up);
	__stop_tx(up);

	/*
	 * We really want to stop the transmitter from sending.
	 */
	if (port->type == PORT_16C950) {
		up->acr |= UART_ACR_TXDIS;
		serial_icr_write(up, UART_ACR, up->acr);
	}
	serial8250_rpm_put(up);
}

static void serial8250_start_tx(struct uart_port *port)
{
	struct uart_8250_port *up = up_to_u8250p(port);

	serial8250_rpm_get_tx(up);

	if (up->dma && !up->dma->tx_dma(up))
		return;

	if (!(up->ier & UART_IER_THRI)) {
		up->ier |= UART_IER_THRI;
		serial_port_out(port, UART_IER, up->ier);

		if (up->bugs & UART_BUG_TXEN) {
			unsigned char lsr;
			lsr = serial_in(up, UART_LSR);
			up->lsr_saved_flags |= lsr & LSR_SAVE_FLAGS;
			if (lsr & UART_LSR_THRE)
				serial8250_tx_chars(up);
		}
	}

	/*
	 * Re-enable the transmitter if we disabled it.
	 */
	if (port->type == PORT_16C950 && up->acr & UART_ACR_TXDIS) {
		up->acr &= ~UART_ACR_TXDIS;
		serial_icr_write(up, UART_ACR, up->acr);
	}
}

static void serial8250_throttle(struct uart_port *port)
{
	port->throttle(port);
}

static void serial8250_unthrottle(struct uart_port *port)
{
	port->unthrottle(port);
}

static void serial8250_stop_rx(struct uart_port *port)
{
	struct uart_8250_port *up = up_to_u8250p(port);

	serial8250_rpm_get(up);

	up->ier &= ~(UART_IER_RLSI | UART_IER_RDI);
	up->port.read_status_mask &= ~UART_LSR_DR;
	serial_port_out(port, UART_IER, up->ier);

	serial8250_rpm_put(up);
}

static void serial8250_disable_ms(struct uart_port *port)
{
	struct uart_8250_port *up =
		container_of(port, struct uart_8250_port, port);

	/* no MSR capabilities */
	if (up->bugs & UART_BUG_NOMSR)
		return;

	up->ier &= ~UART_IER_MSI;
	serial_port_out(port, UART_IER, up->ier);
}

static void serial8250_enable_ms(struct uart_port *port)
{
	struct uart_8250_port *up = up_to_u8250p(port);

	/* no MSR capabilities */
	if (up->bugs & UART_BUG_NOMSR)
		return;

	up->ier |= UART_IER_MSI;

	serial8250_rpm_get(up);
	serial_port_out(port, UART_IER, up->ier);
	serial8250_rpm_put(up);
}

/*
 * serial8250_rx_chars: processes according to the passed in LSR
 * value, and returns the remaining LSR bits not handled
 * by this Rx routine.
 */
unsigned char
serial8250_rx_chars(struct uart_8250_port *up, unsigned char lsr)
{
	struct uart_port *port = &up->port;
	unsigned char ch;
	int max_count = 256;
	char flag;

	do {
		if (likely(lsr & UART_LSR_DR))
			ch = serial_in(up, UART_RX);
		else
			/*
			 * Intel 82571 has a Serial Over Lan device that will
			 * set UART_LSR_BI without setting UART_LSR_DR when
			 * it receives a break. To avoid reading from the
			 * receive buffer without UART_LSR_DR bit set, we
			 * just force the read character to be 0
			 */
			ch = 0;

		flag = TTY_NORMAL;
		port->icount.rx++;

		lsr |= up->lsr_saved_flags;
		up->lsr_saved_flags = 0;

		if (unlikely(lsr & UART_LSR_BRK_ERROR_BITS)) {
			if (lsr & UART_LSR_BI) {
				lsr &= ~(UART_LSR_FE | UART_LSR_PE);
				port->icount.brk++;
				/*
				 * We do the SysRQ and SAK checking
				 * here because otherwise the break
				 * may get masked by ignore_status_mask
				 * or read_status_mask.
				 */
				if (uart_handle_break(port))
					goto ignore_char;
			} else if (lsr & UART_LSR_PE)
				port->icount.parity++;
			else if (lsr & UART_LSR_FE)
				port->icount.frame++;
			if (lsr & UART_LSR_OE)
				port->icount.overrun++;

			/*
			 * Mask off conditions which should be ignored.
			 */
			lsr &= port->read_status_mask;

			if (lsr & UART_LSR_BI) {
				DEBUG_INTR("handling break....");
				flag = TTY_BREAK;
			} else if (lsr & UART_LSR_PE)
				flag = TTY_PARITY;
			else if (lsr & UART_LSR_FE)
				flag = TTY_FRAME;
		}
#if defined(CONFIG_BCM_KF_CHAR_SYSRQ) && defined(SUPPORT_SYSRQ)
		/*
		* Simple hack for substituting a regular ASCII char as the break
		* char for the start of the Magic Sysrq sequence.  This duplicates
		* some of the code in uart_handle_break() in serial_core.h
		*/
		if (up->port.sysrq == 0)
		{
			if (ch == sysrq_start_char) {
				up->port.sysrq = jiffies + HZ*5;
				goto ignore_char;
			}
		}
#endif
		if (uart_handle_sysrq_char(port, ch))
			goto ignore_char;

		uart_insert_char(port, lsr, UART_LSR_OE, ch, flag);

ignore_char:
		lsr = serial_in(up, UART_LSR);
	} while ((lsr & (UART_LSR_DR | UART_LSR_BI)) && (--max_count > 0));
	spin_unlock(&port->lock);
	tty_flip_buffer_push(&port->state->port);
	spin_lock(&port->lock);
	return lsr;
}
EXPORT_SYMBOL_GPL(serial8250_rx_chars);

void serial8250_tx_chars(struct uart_8250_port *up)
{
	struct uart_port *port = &up->port;
	struct circ_buf *xmit = &port->state->xmit;
	int count;

	if (port->x_char) {
		serial_out(up, UART_TX, port->x_char);
		port->icount.tx++;
		port->x_char = 0;
		return;
	}
	if (uart_tx_stopped(port)) {
		serial8250_stop_tx(port);
		return;
	}
	if (uart_circ_empty(xmit)) {
		__stop_tx(up);
		return;
	}

	count = up->tx_loadsz;
	do {
		serial_out(up, UART_TX, xmit->buf[xmit->tail]);
		xmit->tail = (xmit->tail + 1) & (UART_XMIT_SIZE - 1);
		port->icount.tx++;
		if (uart_circ_empty(xmit))
			break;
		if (up->capabilities & UART_CAP_HFIFO) {
			if ((serial_port_in(port, UART_LSR) & BOTH_EMPTY) !=
			    BOTH_EMPTY)
				break;
		}
	} while (--count > 0);

	if (uart_circ_chars_pending(xmit) < WAKEUP_CHARS)
		uart_write_wakeup(port);

	DEBUG_INTR("THRE...");

	/*
	 * With RPM enabled, we have to wait until the FIFO is empty before the
	 * HW can go idle. So we get here once again with empty FIFO and disable
	 * the interrupt and RPM in __stop_tx()
	 */
	if (uart_circ_empty(xmit) && !(up->capabilities & UART_CAP_RPM))
		__stop_tx(up);
}
EXPORT_SYMBOL_GPL(serial8250_tx_chars);

/* Caller holds uart port lock */
unsigned int serial8250_modem_status(struct uart_8250_port *up)
{
	struct uart_port *port = &up->port;
	unsigned int status = serial_in(up, UART_MSR);

	status |= up->msr_saved_flags;
	up->msr_saved_flags = 0;
	if (status & UART_MSR_ANY_DELTA && up->ier & UART_IER_MSI &&
	    port->state != NULL) {
		if (status & UART_MSR_TERI)
			port->icount.rng++;
		if (status & UART_MSR_DDSR)
			port->icount.dsr++;
		if (status & UART_MSR_DDCD)
			uart_handle_dcd_change(port, status & UART_MSR_DCD);
		if (status & UART_MSR_DCTS)
			uart_handle_cts_change(port, status & UART_MSR_CTS);

		wake_up_interruptible(&port->state->port.delta_msr_wait);
	}

	return status;
}
EXPORT_SYMBOL_GPL(serial8250_modem_status);

/*
 * This handles the interrupt from one port.
 */
int serial8250_handle_irq(struct uart_port *port, unsigned int iir)
{
	unsigned char status;
	unsigned long flags;
	struct uart_8250_port *up = up_to_u8250p(port);
	int dma_err = 0;

	if (iir & UART_IIR_NO_INT)
		return 0;

	spin_lock_irqsave(&port->lock, flags);

	status = serial_port_in(port, UART_LSR);

	DEBUG_INTR("status = %x...", status);

	if (status & (UART_LSR_DR | UART_LSR_BI)) {
		if (up->dma)
			dma_err = up->dma->rx_dma(up, iir);

		if (!up->dma || dma_err)
			status = serial8250_rx_chars(up, status);
	}
	serial8250_modem_status(up);
	if ((!up->dma || (up->dma && up->dma->tx_err)) &&
	    (status & UART_LSR_THRE))
		serial8250_tx_chars(up);

	spin_unlock_irqrestore(&port->lock, flags);
	return 1;
}
EXPORT_SYMBOL_GPL(serial8250_handle_irq);

static int serial8250_default_handle_irq(struct uart_port *port)
{
	struct uart_8250_port *up = up_to_u8250p(port);
	unsigned int iir;
	int ret;

	serial8250_rpm_get(up);

	iir = serial_port_in(port, UART_IIR);
	ret = serial8250_handle_irq(port, iir);

	serial8250_rpm_put(up);
	return ret;
}

/*
 * These Exar UARTs have an extra interrupt indicator that could
 * fire for a few unimplemented interrupts.  One of which is a
 * wakeup event when coming out of sleep.  Put this here just
 * to be on the safe side that these interrupts don't go unhandled.
 */
static int exar_handle_irq(struct uart_port *port)
{
	unsigned char int0, int1, int2, int3;
	unsigned int iir = serial_port_in(port, UART_IIR);
	int ret;

	ret = serial8250_handle_irq(port, iir);

	if ((port->type == PORT_XR17V35X) ||
	   (port->type == PORT_XR17D15X)) {
		int0 = serial_port_in(port, 0x80);
		int1 = serial_port_in(port, 0x81);
		int2 = serial_port_in(port, 0x82);
		int3 = serial_port_in(port, 0x83);
	}

	return ret;
}

/*
 * This is the serial driver's interrupt routine.
 *
 * Arjan thinks the old way was overly complex, so it got simplified.
 * Alan disagrees, saying that need the complexity to handle the weird
 * nature of ISA shared interrupts.  (This is a special exception.)
 *
 * In order to handle ISA shared interrupts properly, we need to check
 * that all ports have been serviced, and therefore the ISA interrupt
 * line has been de-asserted.
 *
 * This means we need to loop through all ports. checking that they
 * don't have an interrupt pending.
 */
static irqreturn_t serial8250_interrupt(int irq, void *dev_id)
{
	struct irq_info *i = dev_id;
	struct list_head *l, *end = NULL;
	int pass_counter = 0, handled = 0;

	DEBUG_INTR("serial8250_interrupt(%d)...", irq);

	spin_lock(&i->lock);

	l = i->head;
	do {
		struct uart_8250_port *up;
		struct uart_port *port;

		up = list_entry(l, struct uart_8250_port, list);
		port = &up->port;

		if (port->handle_irq(port)) {
			handled = 1;
			end = NULL;
		} else if (end == NULL)
			end = l;

		l = l->next;

		if (l == i->head && pass_counter++ > PASS_LIMIT) {
			/* If we hit this, we're dead. */
			printk_ratelimited(KERN_ERR
				"serial8250: too much work for irq%d\n", irq);
			break;
		}
	} while (l != end);

	spin_unlock(&i->lock);

	DEBUG_INTR("end.\n");

	return IRQ_RETVAL(handled);
}

/*
 * To support ISA shared interrupts, we need to have one interrupt
 * handler that ensures that the IRQ line has been deasserted
 * before returning.  Failing to do this will result in the IRQ
 * line being stuck active, and, since ISA irqs are edge triggered,
 * no more IRQs will be seen.
 */
static void serial_do_unlink(struct irq_info *i, struct uart_8250_port *up)
{
	spin_lock_irq(&i->lock);

	if (!list_empty(i->head)) {
		if (i->head == &up->list)
			i->head = i->head->next;
		list_del(&up->list);
	} else {
		BUG_ON(i->head != &up->list);
		i->head = NULL;
	}
	spin_unlock_irq(&i->lock);
	/* List empty so throw away the hash node */
	if (i->head == NULL) {
		hlist_del(&i->node);
		kfree(i);
	}
}

static int serial_link_irq_chain(struct uart_8250_port *up)
{
	struct hlist_head *h;
	struct hlist_node *n;
	struct irq_info *i;
	int ret, irq_flags = up->port.flags & UPF_SHARE_IRQ ? IRQF_SHARED : 0;

	mutex_lock(&hash_mutex);

	h = &irq_lists[up->port.irq % NR_IRQ_HASH];

	hlist_for_each(n, h) {
		i = hlist_entry(n, struct irq_info, node);
		if (i->irq == up->port.irq)
			break;
	}

	if (n == NULL) {
		i = kzalloc(sizeof(struct irq_info), GFP_KERNEL);
		if (i == NULL) {
			mutex_unlock(&hash_mutex);
			return -ENOMEM;
		}
		spin_lock_init(&i->lock);
		i->irq = up->port.irq;
		hlist_add_head(&i->node, h);
	}
	mutex_unlock(&hash_mutex);

	spin_lock_irq(&i->lock);

	if (i->head) {
		list_add(&up->list, i->head);
		spin_unlock_irq(&i->lock);

		ret = 0;
	} else {
		INIT_LIST_HEAD(&up->list);
		i->head = &up->list;
		spin_unlock_irq(&i->lock);
		irq_flags |= up->port.irqflags;
		ret = request_irq(up->port.irq, serial8250_interrupt,
				  irq_flags, "serial", i);
		if (ret < 0)
			serial_do_unlink(i, up);
	}

	return ret;
}

static void serial_unlink_irq_chain(struct uart_8250_port *up)
{
	/*
	 * yes, some broken gcc emit "warning: 'i' may be used uninitialized"
	 * but no, we are not going to take a patch that assigns NULL below.
	 */
	struct irq_info *i;
	struct hlist_node *n;
	struct hlist_head *h;
#if defined(CONFIG_BCM_KF_KERN_WARNING)
	i = NULL;
#endif

	mutex_lock(&hash_mutex);

	h = &irq_lists[up->port.irq % NR_IRQ_HASH];

	hlist_for_each(n, h) {
		i = hlist_entry(n, struct irq_info, node);
		if (i->irq == up->port.irq)
			break;
	}

	BUG_ON(n == NULL);
	BUG_ON(i->head == NULL);

	if (list_empty(i->head))
		free_irq(up->port.irq, i);

	serial_do_unlink(i, up);
	mutex_unlock(&hash_mutex);
}

/*
 * This function is used to handle ports that do not have an
 * interrupt.  This doesn't work very well for 16450's, but gives
 * barely passable results for a 16550A.  (Although at the expense
 * of much CPU overhead).
 */
static void serial8250_timeout(unsigned long data)
{
	struct uart_8250_port *up = (struct uart_8250_port *)data;

	up->port.handle_irq(&up->port);
	mod_timer(&up->timer, jiffies + uart_poll_timeout(&up->port));
}

static void serial8250_backup_timeout(unsigned long data)
{
	struct uart_8250_port *up = (struct uart_8250_port *)data;
	unsigned int iir, ier = 0, lsr;
	unsigned long flags;

	spin_lock_irqsave(&up->port.lock, flags);

	/*
	 * Must disable interrupts or else we risk racing with the interrupt
	 * based handler.
	 */
	if (up->port.irq) {
		ier = serial_in(up, UART_IER);
		serial_out(up, UART_IER, 0);
	}

	iir = serial_in(up, UART_IIR);

	/*
	 * This should be a safe test for anyone who doesn't trust the
	 * IIR bits on their UART, but it's specifically designed for
	 * the "Diva" UART used on the management processor on many HP
	 * ia64 and parisc boxes.
	 */
	lsr = serial_in(up, UART_LSR);
	up->lsr_saved_flags |= lsr & LSR_SAVE_FLAGS;
	if ((iir & UART_IIR_NO_INT) && (up->ier & UART_IER_THRI) &&
	    (!uart_circ_empty(&up->port.state->xmit) || up->port.x_char) &&
	    (lsr & UART_LSR_THRE)) {
		iir &= ~(UART_IIR_ID | UART_IIR_NO_INT);
		iir |= UART_IIR_THRI;
	}

	if (!(iir & UART_IIR_NO_INT))
		serial8250_tx_chars(up);

	if (up->port.irq)
		serial_out(up, UART_IER, ier);

	spin_unlock_irqrestore(&up->port.lock, flags);

	/* Standard timer interval plus 0.2s to keep the port running */
	mod_timer(&up->timer,
		jiffies + uart_poll_timeout(&up->port) + HZ / 5);
}

static int univ8250_setup_irq(struct uart_8250_port *up)
{
	struct uart_port *port = &up->port;
	int retval = 0;

	/*
	 * The above check will only give an accurate result the first time
	 * the port is opened so this value needs to be preserved.
	 */
	if (up->bugs & UART_BUG_THRE) {
		pr_debug("ttyS%d - using backup timer\n", serial_index(port));

		up->timer.function = serial8250_backup_timeout;
		up->timer.data = (unsigned long)up;
		mod_timer(&up->timer, jiffies +
			  uart_poll_timeout(port) + HZ / 5);
	}

	/*
	 * If the "interrupt" for this port doesn't correspond with any
	 * hardware interrupt, we use a timer-based system.  The original
	 * driver used to do this with IRQ0.
	 */
	if (!port->irq) {
		up->timer.data = (unsigned long)up;
		mod_timer(&up->timer, jiffies + uart_poll_timeout(port));
	} else
		retval = serial_link_irq_chain(up);

	return retval;
}

static void univ8250_release_irq(struct uart_8250_port *up)
{
	struct uart_port *port = &up->port;

	del_timer_sync(&up->timer);
	up->timer.function = serial8250_timeout;
	if (port->irq)
		serial_unlink_irq_chain(up);
}

static unsigned int serial8250_tx_empty(struct uart_port *port)
{
	struct uart_8250_port *up = up_to_u8250p(port);
	unsigned long flags;
	unsigned int lsr;

	serial8250_rpm_get(up);

	spin_lock_irqsave(&port->lock, flags);
	lsr = serial_port_in(port, UART_LSR);
	up->lsr_saved_flags |= lsr & LSR_SAVE_FLAGS;
	spin_unlock_irqrestore(&port->lock, flags);

	serial8250_rpm_put(up);

	return (lsr & BOTH_EMPTY) == BOTH_EMPTY ? TIOCSER_TEMT : 0;
}

static unsigned int serial8250_get_mctrl(struct uart_port *port)
{
	struct uart_8250_port *up = up_to_u8250p(port);
	unsigned int status;
	unsigned int ret;

	serial8250_rpm_get(up);
	status = serial8250_modem_status(up);
	serial8250_rpm_put(up);

	ret = 0;
	if (status & UART_MSR_DCD)
		ret |= TIOCM_CAR;
	if (status & UART_MSR_RI)
		ret |= TIOCM_RNG;
	if (status & UART_MSR_DSR)
		ret |= TIOCM_DSR;
	if (status & UART_MSR_CTS)
		ret |= TIOCM_CTS;
	return ret;
}

void serial8250_do_set_mctrl(struct uart_port *port, unsigned int mctrl)
{
	struct uart_8250_port *up = up_to_u8250p(port);
	unsigned char mcr = 0;

	if (mctrl & TIOCM_RTS)
		mcr |= UART_MCR_RTS;
	if (mctrl & TIOCM_DTR)
		mcr |= UART_MCR_DTR;
	if (mctrl & TIOCM_OUT1)
		mcr |= UART_MCR_OUT1;
	if (mctrl & TIOCM_OUT2)
		mcr |= UART_MCR_OUT2;
	if (mctrl & TIOCM_LOOP)
		mcr |= UART_MCR_LOOP;

	mcr = (mcr & up->mcr_mask) | up->mcr_force | up->mcr;

	serial_port_out(port, UART_MCR, mcr);
}
EXPORT_SYMBOL_GPL(serial8250_do_set_mctrl);

static void serial8250_set_mctrl(struct uart_port *port, unsigned int mctrl)
{
	if (port->set_mctrl)
		return port->set_mctrl(port, mctrl);
	return serial8250_do_set_mctrl(port, mctrl);
}

static void serial8250_break_ctl(struct uart_port *port, int break_state)
{
	struct uart_8250_port *up = up_to_u8250p(port);
	unsigned long flags;

	serial8250_rpm_get(up);
	spin_lock_irqsave(&port->lock, flags);
	if (break_state == -1)
		up->lcr |= UART_LCR_SBC;
	else
		up->lcr &= ~UART_LCR_SBC;
	serial_port_out(port, UART_LCR, up->lcr);
	spin_unlock_irqrestore(&port->lock, flags);
	serial8250_rpm_put(up);
}

/*
 *	Wait for transmitter & holding register to empty
 */
static void wait_for_xmitr(struct uart_8250_port *up, int bits)
{
	unsigned int status, tmout = 10000;

	/* Wait up to 10ms for the character(s) to be sent. */
	for (;;) {
		status = serial_in(up, UART_LSR);

		up->lsr_saved_flags |= status & LSR_SAVE_FLAGS;

		if ((status & bits) == bits)
			break;
		if (--tmout == 0)
			break;
		udelay(1);
	}

	/* Wait up to 1s for flow control if necessary */
	if (up->port.flags & UPF_CONS_FLOW) {
		unsigned int tmout;
		for (tmout = 1000000; tmout; tmout--) {
			unsigned int msr = serial_in(up, UART_MSR);
			up->msr_saved_flags |= msr & MSR_SAVE_FLAGS;
			if (msr & UART_MSR_CTS)
				break;
			udelay(1);
			touch_nmi_watchdog();
		}
	}
}

#ifdef CONFIG_CONSOLE_POLL
/*
 * Console polling routines for writing and reading from the uart while
 * in an interrupt or debug context.
 */

static int serial8250_get_poll_char(struct uart_port *port)
{
	struct uart_8250_port *up = up_to_u8250p(port);
	unsigned char lsr;
	int status;

	serial8250_rpm_get(up);

	lsr = serial_port_in(port, UART_LSR);

	if (!(lsr & UART_LSR_DR)) {
		status = NO_POLL_CHAR;
		goto out;
	}

	status = serial_port_in(port, UART_RX);
out:
	serial8250_rpm_put(up);
	return status;
}


static void serial8250_put_poll_char(struct uart_port *port,
			 unsigned char c)
{
	unsigned int ier;
	struct uart_8250_port *up = up_to_u8250p(port);

	serial8250_rpm_get(up);
	/*
	 *	First save the IER then disable the interrupts
	 */
	ier = serial_port_in(port, UART_IER);
	if (up->capabilities & UART_CAP_UUE)
		serial_port_out(port, UART_IER, UART_IER_UUE);
	else
		serial_port_out(port, UART_IER, 0);

	wait_for_xmitr(up, BOTH_EMPTY);
	/*
	 *	Send the character out.
	 */
	serial_port_out(port, UART_TX, c);

	/*
	 *	Finally, wait for transmitter to become empty
	 *	and restore the IER
	 */
	wait_for_xmitr(up, BOTH_EMPTY);
	serial_port_out(port, UART_IER, ier);
	serial8250_rpm_put(up);
}

#endif /* CONFIG_CONSOLE_POLL */

int serial8250_do_startup(struct uart_port *port)
{
	struct uart_8250_port *up = up_to_u8250p(port);
	unsigned long flags;
	unsigned char lsr, iir;
	int retval;

	if (port->type == PORT_8250_CIR)
		return -ENODEV;

	if (!port->fifosize)
		port->fifosize = uart_config[port->type].fifo_size;
	if (!up->tx_loadsz)
		up->tx_loadsz = uart_config[port->type].tx_loadsz;
	if (!up->capabilities)
		up->capabilities = uart_config[port->type].flags;
	up->mcr = 0;

	if (port->iotype != up->cur_iotype)
		set_io_from_upio(port);

	serial8250_rpm_get(up);
	if (port->type == PORT_16C950) {
		/* Wake up and initialize UART */
		up->acr = 0;
		serial_port_out(port, UART_LCR, UART_LCR_CONF_MODE_B);
		serial_port_out(port, UART_EFR, UART_EFR_ECB);
		serial_port_out(port, UART_IER, 0);
		serial_port_out(port, UART_LCR, 0);
		serial_icr_write(up, UART_CSR, 0); /* Reset the UART */
		serial_port_out(port, UART_LCR, UART_LCR_CONF_MODE_B);
		serial_port_out(port, UART_EFR, UART_EFR_ECB);
		serial_port_out(port, UART_LCR, 0);
	}

#ifdef CONFIG_SERIAL_8250_RSA
	/*
	 * If this is an RSA port, see if we can kick it up to the
	 * higher speed clock.
	 */
	enable_rsa(up);
#endif
	/*
	 * Clear the FIFO buffers and disable them.
	 * (they will be reenabled in set_termios())
	 */
	serial8250_clear_fifos(up);

	/*
	 * Clear the interrupt registers.
	 */
	serial_port_in(port, UART_LSR);
	serial_port_in(port, UART_RX);
	serial_port_in(port, UART_IIR);
	serial_port_in(port, UART_MSR);

	/*
	 * At this point, there's no way the LSR could still be 0xff;
	 * if it is, then bail out, because there's likely no UART
	 * here.
	 */
	if (!(port->flags & UPF_BUGGY_UART) &&
	    (serial_port_in(port, UART_LSR) == 0xff)) {
		printk_ratelimited(KERN_INFO "ttyS%d: LSR safety check engaged!\n",
				   serial_index(port));
		retval = -ENODEV;
		goto out;
	}

	/*
	 * For a XR16C850, we need to set the trigger levels
	 */
	if (port->type == PORT_16850) {
		unsigned char fctr;

		serial_out(up, UART_LCR, UART_LCR_CONF_MODE_B);

		fctr = serial_in(up, UART_FCTR) & ~(UART_FCTR_RX|UART_FCTR_TX);
		serial_port_out(port, UART_FCTR,
				fctr | UART_FCTR_TRGD | UART_FCTR_RX);
		serial_port_out(port, UART_TRG, UART_TRG_96);
		serial_port_out(port, UART_FCTR,
				fctr | UART_FCTR_TRGD | UART_FCTR_TX);
		serial_port_out(port, UART_TRG, UART_TRG_96);

		serial_port_out(port, UART_LCR, 0);
	}

	if (port->irq) {
		unsigned char iir1;
		/*
		 * Test for UARTs that do not reassert THRE when the
		 * transmitter is idle and the interrupt has already
		 * been cleared.  Real 16550s should always reassert
		 * this interrupt whenever the transmitter is idle and
		 * the interrupt is enabled.  Delays are necessary to
		 * allow register changes to become visible.
		 */
		spin_lock_irqsave(&port->lock, flags);
		if (up->port.irqflags & IRQF_SHARED)
			disable_irq_nosync(port->irq);

		wait_for_xmitr(up, UART_LSR_THRE);
		serial_port_out_sync(port, UART_IER, UART_IER_THRI);
		udelay(1); /* allow THRE to set */
		iir1 = serial_port_in(port, UART_IIR);
		serial_port_out(port, UART_IER, 0);
		serial_port_out_sync(port, UART_IER, UART_IER_THRI);
		udelay(1); /* allow a working UART time to re-assert THRE */
		iir = serial_port_in(port, UART_IIR);
		serial_port_out(port, UART_IER, 0);

		if (port->irqflags & IRQF_SHARED)
			enable_irq(port->irq);
		spin_unlock_irqrestore(&port->lock, flags);

		/*
		 * If the interrupt is not reasserted, or we otherwise
		 * don't trust the iir, setup a timer to kick the UART
		 * on a regular basis.
		 */
		if ((!(iir1 & UART_IIR_NO_INT) && (iir & UART_IIR_NO_INT)) ||
		    up->port.flags & UPF_BUG_THRE) {
			up->bugs |= UART_BUG_THRE;
		}
	}

	retval = up->ops->setup_irq(up);
	if (retval)
		goto out;

	/*
	 * Now, initialize the UART
	 */
	serial_port_out(port, UART_LCR, UART_LCR_WLEN8);

	spin_lock_irqsave(&port->lock, flags);
	if (up->port.flags & UPF_FOURPORT) {
		if (!up->port.irq)
			up->port.mctrl |= TIOCM_OUT1;
	} else
		/*
		 * Most PC uarts need OUT2 raised to enable interrupts.
		 */
		if (port->irq)
			up->port.mctrl |= TIOCM_OUT2;

	serial8250_set_mctrl(port, port->mctrl);

	/* Serial over Lan (SoL) hack:
	   Intel 8257x Gigabit ethernet chips have a
	   16550 emulation, to be used for Serial Over Lan.
	   Those chips take a longer time than a normal
	   serial device to signalize that a transmission
	   data was queued. Due to that, the above test generally
	   fails. One solution would be to delay the reading of
	   iir. However, this is not reliable, since the timeout
	   is variable. So, let's just don't test if we receive
	   TX irq. This way, we'll never enable UART_BUG_TXEN.
	 */
	if (up->port.flags & UPF_NO_TXEN_TEST)
		goto dont_test_tx_en;

	/*
	 * Do a quick test to see if we receive an
	 * interrupt when we enable the TX irq.
	 */
	serial_port_out(port, UART_IER, UART_IER_THRI);
	lsr = serial_port_in(port, UART_LSR);
	iir = serial_port_in(port, UART_IIR);
	serial_port_out(port, UART_IER, 0);

	if (lsr & UART_LSR_TEMT && iir & UART_IIR_NO_INT) {
		if (!(up->bugs & UART_BUG_TXEN)) {
			up->bugs |= UART_BUG_TXEN;
			pr_debug("ttyS%d - enabling bad tx status workarounds\n",
				 serial_index(port));
		}
	} else {
		up->bugs &= ~UART_BUG_TXEN;
	}

dont_test_tx_en:
	spin_unlock_irqrestore(&port->lock, flags);

	/*
	 * Clear the interrupt registers again for luck, and clear the
	 * saved flags to avoid getting false values from polling
	 * routines or the previous session.
	 */
	serial_port_in(port, UART_LSR);
	serial_port_in(port, UART_RX);
	serial_port_in(port, UART_IIR);
	serial_port_in(port, UART_MSR);
	up->lsr_saved_flags = 0;
	up->msr_saved_flags = 0;

	/*
	 * Request DMA channels for both RX and TX.
	 */
	if (up->dma) {
		retval = serial8250_request_dma(up);
		if (retval) {
			pr_warn_ratelimited("ttyS%d - failed to request DMA\n",
					    serial_index(port));
			up->dma = NULL;
		}
	}

	/*
	 * Finally, enable interrupts.  Note: Modem status interrupts
	 * are set via set_termios(), which will be occurring imminently
	 * anyway, so we don't enable them here.
	 */
	up->ier = UART_IER_RLSI | UART_IER_RDI;
	serial_port_out(port, UART_IER, up->ier);

	if (port->flags & UPF_FOURPORT) {
		unsigned int icp;
		/*
		 * Enable interrupts on the AST Fourport board
		 */
		icp = (port->iobase & 0xfe0) | 0x01f;
		outb_p(0x80, icp);
		inb_p(icp);
	}
	retval = 0;
out:
	serial8250_rpm_put(up);
	return retval;
}
EXPORT_SYMBOL_GPL(serial8250_do_startup);

static int serial8250_startup(struct uart_port *port)
{
	if (port->startup)
		return port->startup(port);
	return serial8250_do_startup(port);
}

void serial8250_do_shutdown(struct uart_port *port)
{
	struct uart_8250_port *up = up_to_u8250p(port);
	unsigned long flags;

	serial8250_rpm_get(up);
	/*
	 * Disable interrupts from this port
	 */
	up->ier = 0;
	serial_port_out(port, UART_IER, 0);

	if (up->dma)
		serial8250_release_dma(up);

	spin_lock_irqsave(&port->lock, flags);
	if (port->flags & UPF_FOURPORT) {
		/* reset interrupts on the AST Fourport board */
		inb((port->iobase & 0xfe0) | 0x1f);
		port->mctrl |= TIOCM_OUT1;
	} else
		port->mctrl &= ~TIOCM_OUT2;

	serial8250_set_mctrl(port, port->mctrl);
	spin_unlock_irqrestore(&port->lock, flags);

	/*
	 * Disable break condition and FIFOs
	 */
	serial_port_out(port, UART_LCR,
			serial_port_in(port, UART_LCR) & ~UART_LCR_SBC);
	serial8250_clear_fifos(up);

#ifdef CONFIG_SERIAL_8250_RSA
	/*
	 * Reset the RSA board back to 115kbps compat mode.
	 */
	disable_rsa(up);
#endif

	/*
	 * Read data port to reset things, and then unlink from
	 * the IRQ chain.
	 */
	serial_port_in(port, UART_RX);
	serial8250_rpm_put(up);

	up->ops->release_irq(up);
}
EXPORT_SYMBOL_GPL(serial8250_do_shutdown);

static void serial8250_shutdown(struct uart_port *port)
{
	if (port->shutdown)
		port->shutdown(port);
	else
		serial8250_do_shutdown(port);
}

/*
 * XR17V35x UARTs have an extra fractional divisor register (DLD)
 * Calculate divisor with extra 4-bit fractional portion
 */
static unsigned int xr17v35x_get_divisor(struct uart_8250_port *up,
					 unsigned int baud,
					 unsigned int *frac)
{
	struct uart_port *port = &up->port;
	unsigned int quot_16;

	quot_16 = DIV_ROUND_CLOSEST(port->uartclk, baud);
	*frac = quot_16 & 0x0f;

	return quot_16 >> 4;
}

static unsigned int serial8250_get_divisor(struct uart_8250_port *up,
					   unsigned int baud,
					   unsigned int *frac)
{
	struct uart_port *port = &up->port;
	unsigned int quot;

	/*
	 * Handle magic divisors for baud rates above baud_base on
	 * SMSC SuperIO chips.
	 *
	 */
	if ((port->flags & UPF_MAGIC_MULTIPLIER) &&
	    baud == (port->uartclk/4))
		quot = 0x8001;
	else if ((port->flags & UPF_MAGIC_MULTIPLIER) &&
		 baud == (port->uartclk/8))
		quot = 0x8002;
	else if (up->port.type == PORT_XR17V35X)
		quot = xr17v35x_get_divisor(up, baud, frac);
	else
		quot = uart_get_divisor(port, baud);

	/*
	 * Oxford Semi 952 rev B workaround
	 */
	if (up->bugs & UART_BUG_QUOT && (quot & 0xff) == 0)
		quot++;

	return quot;
}

static unsigned char serial8250_compute_lcr(struct uart_8250_port *up,
					    tcflag_t c_cflag)
{
	unsigned char cval;

	switch (c_cflag & CSIZE) {
	case CS5:
		cval = UART_LCR_WLEN5;
		break;
	case CS6:
		cval = UART_LCR_WLEN6;
		break;
	case CS7:
		cval = UART_LCR_WLEN7;
		break;
	default:
	case CS8:
		cval = UART_LCR_WLEN8;
		break;
	}

	if (c_cflag & CSTOPB)
		cval |= UART_LCR_STOP;
	if (c_cflag & PARENB) {
		cval |= UART_LCR_PARITY;
		if (up->bugs & UART_BUG_PARITY)
			up->fifo_bug = true;
	}
	if (!(c_cflag & PARODD))
		cval |= UART_LCR_EPAR;
#ifdef CMSPAR
	if (c_cflag & CMSPAR)
		cval |= UART_LCR_SPAR;
#endif

	return cval;
}

static void serial8250_set_divisor(struct uart_port *port, unsigned int baud,
			    unsigned int quot, unsigned int quot_frac)
{
	struct uart_8250_port *up = up_to_u8250p(port);

	/* Workaround to enable 115200 baud on OMAP1510 internal ports */
	if (is_omap1510_8250(up)) {
		if (baud == 115200) {
			quot = 1;
			serial_port_out(port, UART_OMAP_OSC_12M_SEL, 1);
		} else
			serial_port_out(port, UART_OMAP_OSC_12M_SEL, 0);
	}

	/*
	 * For NatSemi, switch to bank 2 not bank 1, to avoid resetting EXCR2,
	 * otherwise just set DLAB
	 */
	if (up->capabilities & UART_NATSEMI)
		serial_port_out(port, UART_LCR, 0xe0);
	else
		serial_port_out(port, UART_LCR, up->lcr | UART_LCR_DLAB);

	serial_dl_write(up, quot);

	/* XR17V35x UARTs have an extra fractional divisor register (DLD) */
	if (up->port.type == PORT_XR17V35X)
		serial_port_out(port, 0x2, quot_frac);
}

void
serial8250_do_set_termios(struct uart_port *port, struct ktermios *termios,
		          struct ktermios *old)
{
	struct uart_8250_port *up = up_to_u8250p(port);
	unsigned char cval;
	unsigned long flags;
	unsigned int baud, quot, frac = 0;

	cval = serial8250_compute_lcr(up, termios->c_cflag);

	/*
	 * Ask the core to calculate the divisor for us.
	 */
	baud = uart_get_baud_rate(port, termios, old,
				  port->uartclk / 16 / 0xffff,
				  port->uartclk / 16);
	quot = serial8250_get_divisor(up, baud, &frac);

	/*
	 * Ok, we're now changing the port state.  Do it with
	 * interrupts disabled.
	 */
	serial8250_rpm_get(up);
	spin_lock_irqsave(&port->lock, flags);

	up->lcr = cval;					/* Save computed LCR */

	if (up->capabilities & UART_CAP_FIFO && port->fifosize > 1) {
		/* NOTE: If fifo_bug is not set, a user can set RX_trigger. */
		if ((baud < 2400 && !up->dma) || up->fifo_bug) {
			up->fcr &= ~UART_FCR_TRIGGER_MASK;
			up->fcr |= UART_FCR_TRIGGER_1;
		}
	}

	/*
	 * MCR-based auto flow control.  When AFE is enabled, RTS will be
	 * deasserted when the receive FIFO contains more characters than
	 * the trigger, or the MCR RTS bit is cleared.  In the case where
	 * the remote UART is not using CTS auto flow control, we must
	 * have sufficient FIFO entries for the latency of the remote
	 * UART to respond.  IOW, at least 32 bytes of FIFO.
	 */
	if (up->capabilities & UART_CAP_AFE && port->fifosize >= 32) {
		up->mcr &= ~UART_MCR_AFE;
		if (termios->c_cflag & CRTSCTS)
			up->mcr |= UART_MCR_AFE;
	}

	/*
	 * Update the per-port timeout.
	 */
	uart_update_timeout(port, termios->c_cflag, baud);

	port->read_status_mask = UART_LSR_OE | UART_LSR_THRE | UART_LSR_DR;
	if (termios->c_iflag & INPCK)
		port->read_status_mask |= UART_LSR_FE | UART_LSR_PE;
	if (termios->c_iflag & (IGNBRK | BRKINT | PARMRK))
		port->read_status_mask |= UART_LSR_BI;

	/*
	 * Characteres to ignore
	 */
	port->ignore_status_mask = 0;
	if (termios->c_iflag & IGNPAR)
		port->ignore_status_mask |= UART_LSR_PE | UART_LSR_FE;
	if (termios->c_iflag & IGNBRK) {
		port->ignore_status_mask |= UART_LSR_BI;
		/*
		 * If we're ignoring parity and break indicators,
		 * ignore overruns too (for real raw support).
		 */
		if (termios->c_iflag & IGNPAR)
			port->ignore_status_mask |= UART_LSR_OE;
	}

	/*
	 * ignore all characters if CREAD is not set
	 */
	if ((termios->c_cflag & CREAD) == 0)
		port->ignore_status_mask |= UART_LSR_DR;

	/*
	 * CTS flow control flag and modem status interrupts
	 */
	up->ier &= ~UART_IER_MSI;
	if (!(up->bugs & UART_BUG_NOMSR) &&
			UART_ENABLE_MS(&up->port, termios->c_cflag))
		up->ier |= UART_IER_MSI;
	if (up->capabilities & UART_CAP_UUE)
		up->ier |= UART_IER_UUE;
	if (up->capabilities & UART_CAP_RTOIE)
		up->ier |= UART_IER_RTOIE;

	serial_port_out(port, UART_IER, up->ier);

	if (up->capabilities & UART_CAP_EFR) {
		unsigned char efr = 0;
		/*
		 * TI16C752/Startech hardware flow control.  FIXME:
		 * - TI16C752 requires control thresholds to be set.
		 * - UART_MCR_RTS is ineffective if auto-RTS mode is enabled.
		 */
		if (termios->c_cflag & CRTSCTS)
			efr |= UART_EFR_CTS;

		serial_port_out(port, UART_LCR, UART_LCR_CONF_MODE_B);
		if (port->flags & UPF_EXAR_EFR)
			serial_port_out(port, UART_XR_EFR, efr);
		else
			serial_port_out(port, UART_EFR, efr);
	}

	serial8250_set_divisor(port, baud, quot, frac);

	/*
	 * LCR DLAB must be set to enable 64-byte FIFO mode. If the FCR
	 * is written without DLAB set, this mode will be disabled.
	 */
	if (port->type == PORT_16750)
		serial_port_out(port, UART_FCR, up->fcr);

	serial_port_out(port, UART_LCR, up->lcr);	/* reset DLAB */
	if (port->type != PORT_16750) {
		/* emulated UARTs (Lucent Venus 167x) need two steps */
		if (up->fcr & UART_FCR_ENABLE_FIFO)
			serial_port_out(port, UART_FCR, UART_FCR_ENABLE_FIFO);
		serial_port_out(port, UART_FCR, up->fcr);	/* set fcr */
	}
	serial8250_set_mctrl(port, port->mctrl);
	spin_unlock_irqrestore(&port->lock, flags);
	serial8250_rpm_put(up);

	/* Don't rewrite B0 */
	if (tty_termios_baud_rate(termios))
		tty_termios_encode_baud_rate(termios, baud, baud);
}
EXPORT_SYMBOL(serial8250_do_set_termios);

static void
serial8250_set_termios(struct uart_port *port, struct ktermios *termios,
		       struct ktermios *old)
{
	if (port->set_termios)
		port->set_termios(port, termios, old);
	else
		serial8250_do_set_termios(port, termios, old);
}

static void
serial8250_set_ldisc(struct uart_port *port, struct ktermios *termios)
{
	if (termios->c_line == N_PPS) {
		port->flags |= UPF_HARDPPS_CD;
		spin_lock_irq(&port->lock);
		serial8250_enable_ms(port);
		spin_unlock_irq(&port->lock);
	} else {
		port->flags &= ~UPF_HARDPPS_CD;
		if (!UART_ENABLE_MS(port, termios->c_cflag)) {
			spin_lock_irq(&port->lock);
			serial8250_disable_ms(port);
			spin_unlock_irq(&port->lock);
		}
	}
}


void serial8250_do_pm(struct uart_port *port, unsigned int state,
		      unsigned int oldstate)
{
	struct uart_8250_port *p = up_to_u8250p(port);

	serial8250_set_sleep(p, state != 0);
}
EXPORT_SYMBOL(serial8250_do_pm);

static void
serial8250_pm(struct uart_port *port, unsigned int state,
	      unsigned int oldstate)
{
	if (port->pm)
		port->pm(port, state, oldstate);
	else
		serial8250_do_pm(port, state, oldstate);
}

static unsigned int serial8250_port_size(struct uart_8250_port *pt)
{
	if (pt->port.mapsize)
		return pt->port.mapsize;
	if (pt->port.iotype == UPIO_AU) {
		if (pt->port.type == PORT_RT2880)
			return 0x100;
		return 0x1000;
	}
	if (is_omap1_8250(pt))
		return 0x16 << pt->port.regshift;

	return 8 << pt->port.regshift;
}

/*
 * Resource handling.
 */
static int serial8250_request_std_resource(struct uart_8250_port *up)
{
	unsigned int size = serial8250_port_size(up);
	struct uart_port *port = &up->port;
	int ret = 0;

	switch (port->iotype) {
	case UPIO_AU:
	case UPIO_TSI:
	case UPIO_MEM32:
	case UPIO_MEM32BE:
	case UPIO_MEM:
		if (!port->mapbase)
			break;

		if (!request_mem_region(port->mapbase, size, "serial")) {
			ret = -EBUSY;
			break;
		}

		if (port->flags & UPF_IOREMAP) {
			port->membase = ioremap_nocache(port->mapbase, size);
			if (!port->membase) {
				release_mem_region(port->mapbase, size);
				ret = -ENOMEM;
			}
		}
		break;

	case UPIO_HUB6:
	case UPIO_PORT:
		if (!request_region(port->iobase, size, "serial"))
			ret = -EBUSY;
		break;
	}
	return ret;
}

static void serial8250_release_std_resource(struct uart_8250_port *up)
{
	unsigned int size = serial8250_port_size(up);
	struct uart_port *port = &up->port;

	switch (port->iotype) {
	case UPIO_AU:
	case UPIO_TSI:
	case UPIO_MEM32:
	case UPIO_MEM32BE:
	case UPIO_MEM:
		if (!port->mapbase)
			break;

		if (port->flags & UPF_IOREMAP) {
			iounmap(port->membase);
			port->membase = NULL;
		}

		release_mem_region(port->mapbase, size);
		break;

	case UPIO_HUB6:
	case UPIO_PORT:
		release_region(port->iobase, size);
		break;
	}
}

#ifdef CONFIG_SERIAL_8250_RSA
static int serial8250_request_rsa_resource(struct uart_8250_port *up)
{
	unsigned long start = UART_RSA_BASE << up->port.regshift;
	unsigned int size = 8 << up->port.regshift;
	struct uart_port *port = &up->port;
	int ret = -EINVAL;

	switch (port->iotype) {
	case UPIO_HUB6:
	case UPIO_PORT:
		start += port->iobase;
		if (request_region(start, size, "serial-rsa"))
			ret = 0;
		else
			ret = -EBUSY;
		break;
	}

	return ret;
}

static void serial8250_release_rsa_resource(struct uart_8250_port *up)
{
	unsigned long offset = UART_RSA_BASE << up->port.regshift;
	unsigned int size = 8 << up->port.regshift;
	struct uart_port *port = &up->port;

	switch (port->iotype) {
	case UPIO_HUB6:
	case UPIO_PORT:
		release_region(port->iobase + offset, size);
		break;
	}
}
#endif

static void serial8250_release_port(struct uart_port *port)
{
	struct uart_8250_port *up = up_to_u8250p(port);

	serial8250_release_std_resource(up);
}

static int serial8250_request_port(struct uart_port *port)
{
	struct uart_8250_port *up = up_to_u8250p(port);
	int ret;

	if (port->type == PORT_8250_CIR)
		return -ENODEV;

	ret = serial8250_request_std_resource(up);

	return ret;
}

static int fcr_get_rxtrig_bytes(struct uart_8250_port *up)
{
	const struct serial8250_config *conf_type = &uart_config[up->port.type];
	unsigned char bytes;

	bytes = conf_type->rxtrig_bytes[UART_FCR_R_TRIG_BITS(up->fcr)];

	return bytes ? bytes : -EOPNOTSUPP;
}

static int bytes_to_fcr_rxtrig(struct uart_8250_port *up, unsigned char bytes)
{
	const struct serial8250_config *conf_type = &uart_config[up->port.type];
	int i;

	if (!conf_type->rxtrig_bytes[UART_FCR_R_TRIG_BITS(UART_FCR_R_TRIG_00)])
		return -EOPNOTSUPP;

	for (i = 1; i < UART_FCR_R_TRIG_MAX_STATE; i++) {
		if (bytes < conf_type->rxtrig_bytes[i])
			/* Use the nearest lower value */
			return (--i) << UART_FCR_R_TRIG_SHIFT;
	}

	return UART_FCR_R_TRIG_11;
}

static int do_get_rxtrig(struct tty_port *port)
{
	struct uart_state *state = container_of(port, struct uart_state, port);
	struct uart_port *uport = state->uart_port;
	struct uart_8250_port *up =
		container_of(uport, struct uart_8250_port, port);

	if (!(up->capabilities & UART_CAP_FIFO) || uport->fifosize <= 1)
		return -EINVAL;

	return fcr_get_rxtrig_bytes(up);
}

static int do_serial8250_get_rxtrig(struct tty_port *port)
{
	int rxtrig_bytes;

	mutex_lock(&port->mutex);
	rxtrig_bytes = do_get_rxtrig(port);
	mutex_unlock(&port->mutex);

	return rxtrig_bytes;
}

static ssize_t serial8250_get_attr_rx_trig_bytes(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct tty_port *port = dev_get_drvdata(dev);
	int rxtrig_bytes;

	rxtrig_bytes = do_serial8250_get_rxtrig(port);
	if (rxtrig_bytes < 0)
		return rxtrig_bytes;

	return snprintf(buf, PAGE_SIZE, "%d\n", rxtrig_bytes);
}

static int do_set_rxtrig(struct tty_port *port, unsigned char bytes)
{
	struct uart_state *state = container_of(port, struct uart_state, port);
	struct uart_port *uport = state->uart_port;
	struct uart_8250_port *up =
		container_of(uport, struct uart_8250_port, port);
	int rxtrig;

	if (!(up->capabilities & UART_CAP_FIFO) || uport->fifosize <= 1 ||
	    up->fifo_bug)
		return -EINVAL;

	rxtrig = bytes_to_fcr_rxtrig(up, bytes);
	if (rxtrig < 0)
		return rxtrig;

	serial8250_clear_fifos(up);
	up->fcr &= ~UART_FCR_TRIGGER_MASK;
	up->fcr |= (unsigned char)rxtrig;
	serial_out(up, UART_FCR, up->fcr);
	return 0;
}

static int do_serial8250_set_rxtrig(struct tty_port *port, unsigned char bytes)
{
	int ret;

	mutex_lock(&port->mutex);
	ret = do_set_rxtrig(port, bytes);
	mutex_unlock(&port->mutex);

	return ret;
}

static ssize_t serial8250_set_attr_rx_trig_bytes(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct tty_port *port = dev_get_drvdata(dev);
	unsigned char bytes;
	int ret;

	if (!count)
		return -EINVAL;

	ret = kstrtou8(buf, 10, &bytes);
	if (ret < 0)
		return ret;

	ret = do_serial8250_set_rxtrig(port, bytes);
	if (ret < 0)
		return ret;

	return count;
}

static DEVICE_ATTR(rx_trig_bytes, S_IRUSR | S_IWUSR | S_IRGRP,
		   serial8250_get_attr_rx_trig_bytes,
		   serial8250_set_attr_rx_trig_bytes);

static struct attribute *serial8250_dev_attrs[] = {
	&dev_attr_rx_trig_bytes.attr,
	NULL,
	};

static struct attribute_group serial8250_dev_attr_group = {
	.attrs = serial8250_dev_attrs,
	};

static void register_dev_spec_attr_grp(struct uart_8250_port *up)
{
	const struct serial8250_config *conf_type = &uart_config[up->port.type];

	if (conf_type->rxtrig_bytes[0])
		up->port.attr_group = &serial8250_dev_attr_group;
}

static void serial8250_config_port(struct uart_port *port, int flags)
{
	struct uart_8250_port *up = up_to_u8250p(port);
	int ret;

	if (port->type == PORT_8250_CIR)
		return;

	/*
	 * Find the region that we can probe for.  This in turn
	 * tells us whether we can probe for the type of port.
	 */
	ret = serial8250_request_std_resource(up);
	if (ret < 0)
		return;

	if (port->iotype != up->cur_iotype)
		set_io_from_upio(port);

	if (flags & UART_CONFIG_TYPE)
		autoconfig(up);

	/* if access method is AU, it is a 16550 with a quirk */
	if (port->type == PORT_16550A && port->iotype == UPIO_AU)
		up->bugs |= UART_BUG_NOMSR;

	/* HW bugs may trigger IRQ while IIR == NO_INT */
	if (port->type == PORT_TEGRA)
		up->bugs |= UART_BUG_NOMSR;

	if (port->type != PORT_UNKNOWN && flags & UART_CONFIG_IRQ)
		autoconfig_irq(up);

	if (port->type == PORT_UNKNOWN)
		serial8250_release_std_resource(up);

	/* Fixme: probably not the best place for this */
	if ((port->type == PORT_XR17V35X) ||
	   (port->type == PORT_XR17D15X))
		port->handle_irq = exar_handle_irq;

	register_dev_spec_attr_grp(up);
	up->fcr = uart_config[up->port.type].fcr;
}

static int
serial8250_verify_port(struct uart_port *port, struct serial_struct *ser)
{
	if (ser->irq >= nr_irqs || ser->irq < 0 ||
	    ser->baud_base < 9600 || ser->type < PORT_UNKNOWN ||
	    ser->type >= ARRAY_SIZE(uart_config) || ser->type == PORT_CIRRUS ||
	    ser->type == PORT_STARTECH)
		return -EINVAL;
	return 0;
}

static const char *
serial8250_type(struct uart_port *port)
{
	int type = port->type;

	if (type >= ARRAY_SIZE(uart_config))
		type = 0;
	return uart_config[type].name;
}

static const struct uart_ops serial8250_pops = {
	.tx_empty	= serial8250_tx_empty,
	.set_mctrl	= serial8250_set_mctrl,
	.get_mctrl	= serial8250_get_mctrl,
	.stop_tx	= serial8250_stop_tx,
	.start_tx	= serial8250_start_tx,
	.throttle	= serial8250_throttle,
	.unthrottle	= serial8250_unthrottle,
	.stop_rx	= serial8250_stop_rx,
	.enable_ms	= serial8250_enable_ms,
	.break_ctl	= serial8250_break_ctl,
	.startup	= serial8250_startup,
	.shutdown	= serial8250_shutdown,
	.set_termios	= serial8250_set_termios,
	.set_ldisc	= serial8250_set_ldisc,
	.pm		= serial8250_pm,
	.type		= serial8250_type,
	.release_port	= serial8250_release_port,
	.request_port	= serial8250_request_port,
	.config_port	= serial8250_config_port,
	.verify_port	= serial8250_verify_port,
#ifdef CONFIG_CONSOLE_POLL
	.poll_get_char = serial8250_get_poll_char,
	.poll_put_char = serial8250_put_poll_char,
#endif
};

static const struct uart_ops *base_ops;
static struct uart_ops univ8250_port_ops;

static const struct uart_8250_ops univ8250_driver_ops = {
	.setup_irq	= univ8250_setup_irq,
	.release_irq	= univ8250_release_irq,
};

static struct uart_8250_port serial8250_ports[UART_NR];

/**
 * serial8250_get_port - retrieve struct uart_8250_port
 * @line: serial line number
 *
 * This function retrieves struct uart_8250_port for the specific line.
 * This struct *must* *not* be used to perform a 8250 or serial core operation
 * which is not accessible otherwise. Its only purpose is to make the struct
 * accessible to the runtime-pm callbacks for context suspend/restore.
 * The lock assumption made here is none because runtime-pm suspend/resume
 * callbacks should not be invoked if there is any operation performed on the
 * port.
 */
struct uart_8250_port *serial8250_get_port(int line)
{
	return &serial8250_ports[line];
}
EXPORT_SYMBOL_GPL(serial8250_get_port);

static void (*serial8250_isa_config)(int port, struct uart_port *up,
	unsigned short *capabilities);

void serial8250_set_isa_configurator(
	void (*v)(int port, struct uart_port *up, unsigned short *capabilities))
{
	serial8250_isa_config = v;
}
EXPORT_SYMBOL(serial8250_set_isa_configurator);

static void serial8250_init_port(struct uart_8250_port *up)
{
	struct uart_port *port = &up->port;

	spin_lock_init(&port->lock);
	port->ops = &serial8250_pops;

	up->cur_iotype = 0xFF;
}

static void serial8250_set_defaults(struct uart_8250_port *up)
{
	struct uart_port *port = &up->port;

	if (up->port.flags & UPF_FIXED_TYPE) {
		unsigned int type = up->port.type;

		if (!up->port.fifosize)
			up->port.fifosize = uart_config[type].fifo_size;
		if (!up->tx_loadsz)
			up->tx_loadsz = uart_config[type].tx_loadsz;
		if (!up->capabilities)
			up->capabilities = uart_config[type].flags;
	}

	set_io_from_upio(port);

	/* default dma handlers */
	if (up->dma) {
		if (!up->dma->tx_dma)
			up->dma->tx_dma = serial8250_tx_dma;
		if (!up->dma->rx_dma)
			up->dma->rx_dma = serial8250_rx_dma;
	}
}

#ifdef CONFIG_SERIAL_8250_RSA

static void univ8250_config_port(struct uart_port *port, int flags)
{
	struct uart_8250_port *up = up_to_u8250p(port);

	up->probe &= ~UART_PROBE_RSA;
	if (port->type == PORT_RSA) {
		if (serial8250_request_rsa_resource(up) == 0)
			up->probe |= UART_PROBE_RSA;
	} else if (flags & UART_CONFIG_TYPE) {
		int i;

		for (i = 0; i < probe_rsa_count; i++) {
			if (probe_rsa[i] == up->port.iobase) {
				if (serial8250_request_rsa_resource(up) == 0)
					up->probe |= UART_PROBE_RSA;
				break;
			}
		}
	}

	base_ops->config_port(port, flags);

	if (port->type != PORT_RSA && up->probe & UART_PROBE_RSA)
		serial8250_release_rsa_resource(up);
}

static int univ8250_request_port(struct uart_port *port)
{
	struct uart_8250_port *up = up_to_u8250p(port);
	int ret;

	ret = base_ops->request_port(port);
	if (ret == 0 && port->type == PORT_RSA) {
		ret = serial8250_request_rsa_resource(up);
		if (ret < 0)
			base_ops->release_port(port);
	}

	return ret;
}

static void univ8250_release_port(struct uart_port *port)
{
	struct uart_8250_port *up = up_to_u8250p(port);

	if (port->type == PORT_RSA)
		serial8250_release_rsa_resource(up);
	base_ops->release_port(port);
}

static void univ8250_rsa_support(struct uart_ops *ops)
{
	ops->config_port  = univ8250_config_port;
	ops->request_port = univ8250_request_port;
	ops->release_port = univ8250_release_port;
}

#else
#define univ8250_rsa_support(x)		do { } while (0)
#endif /* CONFIG_SERIAL_8250_RSA */

static void __init serial8250_isa_init_ports(void)
{
	struct uart_8250_port *up;
	static int first = 1;
	int i, irqflag = 0;

	if (!first)
		return;
	first = 0;

	if (nr_uarts > UART_NR)
		nr_uarts = UART_NR;

	for (i = 0; i < nr_uarts; i++) {
		struct uart_8250_port *up = &serial8250_ports[i];
		struct uart_port *port = &up->port;

		port->line = i;
		serial8250_init_port(up);
		if (!base_ops)
			base_ops = port->ops;
		port->ops = &univ8250_port_ops;

		init_timer(&up->timer);
		up->timer.function = serial8250_timeout;

		up->ops = &univ8250_driver_ops;

		/*
		 * ALPHA_KLUDGE_MCR needs to be killed.
		 */
		up->mcr_mask = ~ALPHA_KLUDGE_MCR;
		up->mcr_force = ALPHA_KLUDGE_MCR;
	}

	/* chain base port ops to support Remote Supervisor Adapter */
	univ8250_port_ops = *base_ops;
	univ8250_rsa_support(&univ8250_port_ops);

	if (share_irqs)
		irqflag = IRQF_SHARED;

	for (i = 0, up = serial8250_ports;
	     i < ARRAY_SIZE(old_serial_port) && i < nr_uarts;
	     i++, up++) {
		struct uart_port *port = &up->port;

		port->iobase   = old_serial_port[i].port;
		port->irq      = irq_canonicalize(old_serial_port[i].irq);
		port->irqflags = old_serial_port[i].irqflags;
		port->uartclk  = old_serial_port[i].baud_base * 16;
		port->flags    = old_serial_port[i].flags;
		port->hub6     = old_serial_port[i].hub6;
		port->membase  = old_serial_port[i].iomem_base;
		port->iotype   = old_serial_port[i].io_type;
		port->regshift = old_serial_port[i].iomem_reg_shift;
		serial8250_set_defaults(up);

		port->irqflags |= irqflag;
		if (serial8250_isa_config != NULL)
			serial8250_isa_config(i, &up->port, &up->capabilities);
	}
}

static void __init
serial8250_register_ports(struct uart_driver *drv, struct device *dev)
{
	int i;

	for (i = 0; i < nr_uarts; i++) {
		struct uart_8250_port *up = &serial8250_ports[i];

		if (up->port.dev)
			continue;

		up->port.dev = dev;

		if (skip_txen_test)
			up->port.flags |= UPF_NO_TXEN_TEST;

		uart_add_one_port(drv, &up->port);
	}
}

#ifdef CONFIG_SERIAL_8250_CONSOLE

static void serial8250_console_putchar(struct uart_port *port, int ch)
{
	struct uart_8250_port *up = up_to_u8250p(port);

	wait_for_xmitr(up, UART_LSR_THRE);
	serial_port_out(port, UART_TX, ch);
}

/*
 *	Print a string to the serial port trying not to disturb
 *	any possible real use of the port...
 *
 *	The console_lock must be held when we get here.
 */
static void serial8250_console_write(struct uart_8250_port *up, const char *s,
				     unsigned int count)
{
	struct uart_port *port = &up->port;
	unsigned long flags;
	unsigned int ier;
	int locked = 1;

	touch_nmi_watchdog();

	serial8250_rpm_get(up);

	if (port->sysrq)
		locked = 0;
	else if (oops_in_progress)
		locked = spin_trylock_irqsave(&port->lock, flags);
	else
		spin_lock_irqsave(&port->lock, flags);

	/*
	 *	First save the IER then disable the interrupts
	 */
	ier = serial_port_in(port, UART_IER);

	if (up->capabilities & UART_CAP_UUE)
		serial_port_out(port, UART_IER, UART_IER_UUE);
	else
		serial_port_out(port, UART_IER, 0);

	/* check scratch reg to see if port powered off during system sleep */
	if (up->canary && (up->canary != serial_port_in(port, UART_SCR))) {
		struct ktermios termios;
		unsigned int baud, quot, frac = 0;

		termios.c_cflag = port->cons->cflag;
		if (port->state->port.tty && termios.c_cflag == 0)
			termios.c_cflag = port->state->port.tty->termios.c_cflag;

		baud = uart_get_baud_rate(port, &termios, NULL,
					  port->uartclk / 16 / 0xffff,
					  port->uartclk / 16);
		quot = serial8250_get_divisor(up, baud, &frac);

		serial8250_set_divisor(port, baud, quot, frac);
		serial_port_out(port, UART_LCR, up->lcr);
		serial_port_out(port, UART_MCR, UART_MCR_DTR | UART_MCR_RTS);

		up->canary = 0;
	}

	uart_console_write(port, s, count, serial8250_console_putchar);

	/*
	 *	Finally, wait for transmitter to become empty
	 *	and restore the IER
	 */
	wait_for_xmitr(up, BOTH_EMPTY);
	serial_port_out(port, UART_IER, ier);

	/*
	 *	The receive handling will happen properly because the
	 *	receive ready bit will still be set; it is not cleared
	 *	on read.  However, modem control will not, we must
	 *	call it if we have saved something in the saved flags
	 *	while processing with interrupts off.
	 */
	if (up->msr_saved_flags)
		serial8250_modem_status(up);

	if (locked)
		spin_unlock_irqrestore(&port->lock, flags);
	serial8250_rpm_put(up);
}

static void univ8250_console_write(struct console *co, const char *s,
				   unsigned int count)
{
	struct uart_8250_port *up = &serial8250_ports[co->index];

	serial8250_console_write(up, s, count);
}

static unsigned int probe_baud(struct uart_port *port)
{
	unsigned char lcr, dll, dlm;
	unsigned int quot;

	lcr = serial_port_in(port, UART_LCR);
	serial_port_out(port, UART_LCR, lcr | UART_LCR_DLAB);
	dll = serial_port_in(port, UART_DLL);
	dlm = serial_port_in(port, UART_DLM);
	serial_port_out(port, UART_LCR, lcr);

	quot = (dlm << 8) | dll;
	return (port->uartclk / 16) / quot;
}

static int serial8250_console_setup(struct uart_port *port, char *options, bool probe)
{
	int baud = 9600;
	int bits = 8;
	int parity = 'n';
	int flow = 'n';

	if (!port->iobase && !port->membase)
		return -ENODEV;

	if (options)
		uart_parse_options(options, &baud, &parity, &bits, &flow);
	else if (probe)
		baud = probe_baud(port);

	return uart_set_options(port, port->cons, baud, parity, bits, flow);
}

static int univ8250_console_setup(struct console *co, char *options)
{
	struct uart_port *port;

	/*
	 * Check whether an invalid uart number has been specified, and
	 * if so, search for the first available port that does have
	 * console support.
	 */
	if (co->index >= nr_uarts)
		co->index = 0;
	port = &serial8250_ports[co->index].port;
	/* link port to console */
	port->cons = co;

	return serial8250_console_setup(port, options, false);
}

/**
 *	univ8250_console_match - non-standard console matching
 *	@co:	  registering console
 *	@name:	  name from console command line
 *	@idx:	  index from console command line
 *	@options: ptr to option string from console command line
 *
 *	Only attempts to match console command lines of the form:
 *	    console=uart[8250],io|mmio|mmio32,<addr>[,<options>]
 *	    console=uart[8250],0x<addr>[,<options>]
 *	This form is used to register an initial earlycon boot console and
 *	replace it with the serial8250_console at 8250 driver init.
 *
 *	Performs console setup for a match (as required by interface)
 *	If no <options> are specified, then assume the h/w is already setup.
 *
 *	Returns 0 if console matches; otherwise non-zero to use default matching
 */
static int univ8250_console_match(struct console *co, char *name, int idx,
				  char *options)
{
	char match[] = "uart";	/* 8250-specific earlycon name */
	unsigned char iotype;
	unsigned long addr;
	int i;

	if (strncmp(name, match, 4) != 0)
		return -ENODEV;

	if (uart_parse_earlycon(options, &iotype, &addr, &options))
		return -ENODEV;

	/* try to match the port specified on the command line */
	for (i = 0; i < nr_uarts; i++) {
		struct uart_port *port = &serial8250_ports[i].port;

		if (port->iotype != iotype)
			continue;
		if ((iotype == UPIO_MEM || iotype == UPIO_MEM32) &&
		    (port->mapbase != addr))
			continue;
		if (iotype == UPIO_PORT && port->iobase != addr)
			continue;

		co->index = i;
		port->cons = co;
		return serial8250_console_setup(port, options, true);
	}

	return -ENODEV;
}

static struct console univ8250_console = {
	.name		= "ttyS",
	.write		= univ8250_console_write,
	.device		= uart_console_device,
	.setup		= univ8250_console_setup,
	.match		= univ8250_console_match,
	.flags		= CON_PRINTBUFFER | CON_ANYTIME,
	.index		= -1,
	.data		= &serial8250_reg,
};

static int __init univ8250_console_init(void)
{
	serial8250_isa_init_ports();
	register_console(&univ8250_console);
	return 0;
}
console_initcall(univ8250_console_init);

#define SERIAL8250_CONSOLE	&univ8250_console
#else
#define SERIAL8250_CONSOLE	NULL
#endif

static struct uart_driver serial8250_reg = {
	.owner			= THIS_MODULE,
	.driver_name		= "serial",
	.dev_name		= "ttyS",
	.major			= TTY_MAJOR,
	.minor			= 64,
	.cons			= SERIAL8250_CONSOLE,
};

/*
 * early_serial_setup - early registration for 8250 ports
 *
 * Setup an 8250 port structure prior to console initialisation.  Use
 * after console initialisation will cause undefined behaviour.
 */
int __init early_serial_setup(struct uart_port *port)
{
	struct uart_port *p;

	if (port->line >= ARRAY_SIZE(serial8250_ports))
		return -ENODEV;

	serial8250_isa_init_ports();
	p = &serial8250_ports[port->line].port;
	p->iobase       = port->iobase;
	p->membase      = port->membase;
	p->irq          = port->irq;
	p->irqflags     = port->irqflags;
	p->uartclk      = port->uartclk;
	p->fifosize     = port->fifosize;
	p->regshift     = port->regshift;
	p->iotype       = port->iotype;
	p->flags        = port->flags;
	p->mapbase      = port->mapbase;
	p->mapsize      = port->mapsize;
	p->private_data = port->private_data;
	p->type		= port->type;
	p->line		= port->line;

	serial8250_set_defaults(up_to_u8250p(p));

	if (port->serial_in)
		p->serial_in = port->serial_in;
	if (port->serial_out)
		p->serial_out = port->serial_out;
	if (port->handle_irq)
		p->handle_irq = port->handle_irq;

	return 0;
}

/**
 *	serial8250_suspend_port - suspend one serial port
 *	@line:  serial line number
 *
 *	Suspend one serial port.
 */
void serial8250_suspend_port(int line)
{
	struct uart_8250_port *up = &serial8250_ports[line];
	struct uart_port *port = &up->port;

	if (!console_suspend_enabled && uart_console(port) &&
	    port->type != PORT_8250) {
		unsigned char canary = 0xa5;
		serial_out(up, UART_SCR, canary);
		if (serial_in(up, UART_SCR) == canary)
			up->canary = canary;
	}

	uart_suspend_port(&serial8250_reg, port);
}

/**
 *	serial8250_resume_port - resume one serial port
 *	@line:  serial line number
 *
 *	Resume one serial port.
 */
void serial8250_resume_port(int line)
{
	struct uart_8250_port *up = &serial8250_ports[line];
	struct uart_port *port = &up->port;

	up->canary = 0;

	if (up->capabilities & UART_NATSEMI) {
		/* Ensure it's still in high speed mode */
		serial_port_out(port, UART_LCR, 0xE0);

		ns16550a_goto_highspeed(up);

		serial_port_out(port, UART_LCR, 0);
		port->uartclk = 921600*16;
	}
	uart_resume_port(&serial8250_reg, port);
}

/*
 * Register a set of serial devices attached to a platform device.  The
 * list is terminated with a zero flags entry, which means we expect
 * all entries to have at least UPF_BOOT_AUTOCONF set.
 */
static int serial8250_probe(struct platform_device *dev)
{
	struct plat_serial8250_port *p = dev_get_platdata(&dev->dev);
	struct uart_8250_port uart;
	int ret, i, irqflag = 0;

	memset(&uart, 0, sizeof(uart));

	if (share_irqs)
		irqflag = IRQF_SHARED;

	for (i = 0; p && p->flags != 0; p++, i++) {
		uart.port.iobase	= p->iobase;
		uart.port.membase	= p->membase;
		uart.port.irq		= p->irq;
		uart.port.irqflags	= p->irqflags;
		uart.port.uartclk	= p->uartclk;
		uart.port.regshift	= p->regshift;
		uart.port.iotype	= p->iotype;
		uart.port.flags		= p->flags;
		uart.port.mapbase	= p->mapbase;
		uart.port.hub6		= p->hub6;
		uart.port.private_data	= p->private_data;
		uart.port.type		= p->type;
		uart.port.serial_in	= p->serial_in;
		uart.port.serial_out	= p->serial_out;
		uart.port.handle_irq	= p->handle_irq;
		uart.port.handle_break	= p->handle_break;
		uart.port.set_termios	= p->set_termios;
		uart.port.pm		= p->pm;
		uart.port.dev		= &dev->dev;
		uart.port.irqflags	|= irqflag;
		ret = serial8250_register_8250_port(&uart);
		if (ret < 0) {
			dev_err(&dev->dev, "unable to register port at index %d "
				"(IO%lx MEM%llx IRQ%d): %d\n", i,
				p->iobase, (unsigned long long)p->mapbase,
				p->irq, ret);
		}
	}
	return 0;
}

/*
 * Remove serial ports registered against a platform device.
 */
static int serial8250_remove(struct platform_device *dev)
{
	int i;

	for (i = 0; i < nr_uarts; i++) {
		struct uart_8250_port *up = &serial8250_ports[i];

		if (up->port.dev == &dev->dev)
			serial8250_unregister_port(i);
	}
	return 0;
}

static int serial8250_suspend(struct platform_device *dev, pm_message_t state)
{
	int i;

	for (i = 0; i < UART_NR; i++) {
		struct uart_8250_port *up = &serial8250_ports[i];

		if (up->port.type != PORT_UNKNOWN && up->port.dev == &dev->dev)
			uart_suspend_port(&serial8250_reg, &up->port);
	}

	return 0;
}

static int serial8250_resume(struct platform_device *dev)
{
	int i;

	for (i = 0; i < UART_NR; i++) {
		struct uart_8250_port *up = &serial8250_ports[i];

		if (up->port.type != PORT_UNKNOWN && up->port.dev == &dev->dev)
			serial8250_resume_port(i);
	}

	return 0;
}

static struct platform_driver serial8250_isa_driver = {
	.probe		= serial8250_probe,
	.remove		= serial8250_remove,
	.suspend	= serial8250_suspend,
	.resume		= serial8250_resume,
	.driver		= {
		.name	= "serial8250",
	},
};

/*
 * This "device" covers _all_ ISA 8250-compatible serial devices listed
 * in the table in include/asm/serial.h
 */
static struct platform_device *serial8250_isa_devs;

/*
 * serial8250_register_8250_port and serial8250_unregister_port allows for
 * 16x50 serial ports to be configured at run-time, to support PCMCIA
 * modems and PCI multiport cards.
 */
static DEFINE_MUTEX(serial_mutex);

static struct uart_8250_port *serial8250_find_match_or_unused(struct uart_port *port)
{
	int i;

	/*
	 * First, find a port entry which matches.
	 */
	for (i = 0; i < nr_uarts; i++)
		if (uart_match_port(&serial8250_ports[i].port, port))
			return &serial8250_ports[i];

	/* try line number first if still available */
	i = port->line;
	if (i < nr_uarts && serial8250_ports[i].port.type == PORT_UNKNOWN &&
			serial8250_ports[i].port.iobase == 0)
		return &serial8250_ports[i];
	/*
	 * We didn't find a matching entry, so look for the first
	 * free entry.  We look for one which hasn't been previously
	 * used (indicated by zero iobase).
	 */
	for (i = 0; i < nr_uarts; i++)
		if (serial8250_ports[i].port.type == PORT_UNKNOWN &&
		    serial8250_ports[i].port.iobase == 0)
			return &serial8250_ports[i];

	/*
	 * That also failed.  Last resort is to find any entry which
	 * doesn't have a real port associated with it.
	 */
	for (i = 0; i < nr_uarts; i++)
		if (serial8250_ports[i].port.type == PORT_UNKNOWN)
			return &serial8250_ports[i];

	return NULL;
}

/**
 *	serial8250_register_8250_port - register a serial port
 *	@up: serial port template
 *
 *	Configure the serial port specified by the request. If the
 *	port exists and is in use, it is hung up and unregistered
 *	first.
 *
 *	The port is then probed and if necessary the IRQ is autodetected
 *	If this fails an error is returned.
 *
 *	On success the port is ready to use and the line number is returned.
 */
int serial8250_register_8250_port(struct uart_8250_port *up)
{
	struct uart_8250_port *uart;
	int ret = -ENOSPC;

	if (up->port.uartclk == 0)
		return -EINVAL;

	mutex_lock(&serial_mutex);

	uart = serial8250_find_match_or_unused(&up->port);
	if (uart && uart->port.type != PORT_8250_CIR) {
		if (uart->port.dev)
			uart_remove_one_port(&serial8250_reg, &uart->port);

		uart->port.iobase       = up->port.iobase;
		uart->port.membase      = up->port.membase;
		uart->port.irq          = up->port.irq;
		uart->port.irqflags     = up->port.irqflags;
		uart->port.uartclk      = up->port.uartclk;
		uart->port.fifosize     = up->port.fifosize;
		uart->port.regshift     = up->port.regshift;
		uart->port.iotype       = up->port.iotype;
		uart->port.flags        = up->port.flags | UPF_BOOT_AUTOCONF;
		uart->bugs		= up->bugs;
		uart->port.mapbase      = up->port.mapbase;
		uart->port.mapsize      = up->port.mapsize;
		uart->port.private_data = up->port.private_data;
		uart->port.fifosize	= up->port.fifosize;
		uart->tx_loadsz		= up->tx_loadsz;
		uart->capabilities	= up->capabilities;
		uart->port.throttle	= up->port.throttle;
		uart->port.unthrottle	= up->port.unthrottle;
		uart->port.rs485_config	= up->port.rs485_config;
		uart->port.rs485	= up->port.rs485;
		uart->dma		= up->dma;

		/* Take tx_loadsz from fifosize if it wasn't set separately */
		if (uart->port.fifosize && !uart->tx_loadsz)
			uart->tx_loadsz = uart->port.fifosize;

		if (up->port.dev)
			uart->port.dev = up->port.dev;

		if (skip_txen_test)
			uart->port.flags |= UPF_NO_TXEN_TEST;

		if (up->port.flags & UPF_FIXED_TYPE)
			uart->port.type = up->port.type;

		serial8250_set_defaults(uart);

		/* Possibly override default I/O functions.  */
		if (up->port.serial_in)
			uart->port.serial_in = up->port.serial_in;
		if (up->port.serial_out)
			uart->port.serial_out = up->port.serial_out;
		if (up->port.handle_irq)
			uart->port.handle_irq = up->port.handle_irq;
		/*  Possibly override set_termios call */
		if (up->port.set_termios)
			uart->port.set_termios = up->port.set_termios;
		if (up->port.set_mctrl)
			uart->port.set_mctrl = up->port.set_mctrl;
		if (up->port.startup)
			uart->port.startup = up->port.startup;
		if (up->port.shutdown)
			uart->port.shutdown = up->port.shutdown;
		if (up->port.pm)
			uart->port.pm = up->port.pm;
		if (up->port.handle_break)
			uart->port.handle_break = up->port.handle_break;
		if (up->dl_read)
			uart->dl_read = up->dl_read;
		if (up->dl_write)
			uart->dl_write = up->dl_write;

		if (serial8250_isa_config != NULL)
			serial8250_isa_config(0, &uart->port,
					&uart->capabilities);

		ret = uart_add_one_port(&serial8250_reg, &uart->port);
		if (ret == 0)
			ret = uart->port.line;
	}
	mutex_unlock(&serial_mutex);

	return ret;
}
EXPORT_SYMBOL(serial8250_register_8250_port);

/**
 *	serial8250_unregister_port - remove a 16x50 serial port at runtime
 *	@line: serial line number
 *
 *	Remove one serial port.  This may not be called from interrupt
 *	context.  We hand the port back to the our control.
 */
void serial8250_unregister_port(int line)
{
	struct uart_8250_port *uart = &serial8250_ports[line];

	mutex_lock(&serial_mutex);
	uart_remove_one_port(&serial8250_reg, &uart->port);
	if (serial8250_isa_devs) {
		uart->port.flags &= ~UPF_BOOT_AUTOCONF;
		if (skip_txen_test)
			uart->port.flags |= UPF_NO_TXEN_TEST;
		uart->port.type = PORT_UNKNOWN;
		uart->port.dev = &serial8250_isa_devs->dev;
		uart->capabilities = 0;
		uart_add_one_port(&serial8250_reg, &uart->port);
	} else {
		uart->port.dev = NULL;
	}
	mutex_unlock(&serial_mutex);
}
EXPORT_SYMBOL(serial8250_unregister_port);

static int __init serial8250_init(void)
{
	int ret;

	serial8250_isa_init_ports();

	printk(KERN_INFO "Serial: 8250/16550 driver, "
		"%d ports, IRQ sharing %sabled\n", nr_uarts,
		share_irqs ? "en" : "dis");

#ifdef CONFIG_SPARC
	ret = sunserial_register_minors(&serial8250_reg, UART_NR);
#else
	serial8250_reg.nr = UART_NR;
	ret = uart_register_driver(&serial8250_reg);
#endif
	if (ret)
		goto out;

	ret = serial8250_pnp_init();
	if (ret)
		goto unreg_uart_drv;

	serial8250_isa_devs = platform_device_alloc("serial8250",
						    PLAT8250_DEV_LEGACY);
	if (!serial8250_isa_devs) {
		ret = -ENOMEM;
		goto unreg_pnp;
	}

	ret = platform_device_add(serial8250_isa_devs);
	if (ret)
		goto put_dev;

	serial8250_register_ports(&serial8250_reg, &serial8250_isa_devs->dev);

	ret = platform_driver_register(&serial8250_isa_driver);
	if (ret == 0)
		goto out;

	platform_device_del(serial8250_isa_devs);
put_dev:
	platform_device_put(serial8250_isa_devs);
unreg_pnp:
	serial8250_pnp_exit();
unreg_uart_drv:
#ifdef CONFIG_SPARC
	sunserial_unregister_minors(&serial8250_reg, UART_NR);
#else
	uart_unregister_driver(&serial8250_reg);
#endif
out:
	return ret;
}

static void __exit serial8250_exit(void)
{
	struct platform_device *isa_dev = serial8250_isa_devs;

	/*
	 * This tells serial8250_unregister_port() not to re-register
	 * the ports (thereby making serial8250_isa_driver permanently
	 * in use.)
	 */
	serial8250_isa_devs = NULL;

	platform_driver_unregister(&serial8250_isa_driver);
	platform_device_unregister(isa_dev);

	serial8250_pnp_exit();

#ifdef CONFIG_SPARC
	sunserial_unregister_minors(&serial8250_reg, UART_NR);
#else
	uart_unregister_driver(&serial8250_reg);
#endif
}

module_init(serial8250_init);
module_exit(serial8250_exit);

EXPORT_SYMBOL(serial8250_suspend_port);
EXPORT_SYMBOL(serial8250_resume_port);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Generic 8250/16x50 serial driver");

module_param(share_irqs, uint, 0644);
MODULE_PARM_DESC(share_irqs, "Share IRQs with other non-8250/16x50 devices"
	" (unsafe)");

module_param(nr_uarts, uint, 0644);
MODULE_PARM_DESC(nr_uarts, "Maximum number of UARTs supported. (1-" __MODULE_STRING(CONFIG_SERIAL_8250_NR_UARTS) ")");

module_param(skip_txen_test, uint, 0644);
MODULE_PARM_DESC(skip_txen_test, "Skip checking for the TXEN bug at init time");

#ifdef CONFIG_SERIAL_8250_RSA
module_param_array(probe_rsa, ulong, &probe_rsa_count, 0444);
MODULE_PARM_DESC(probe_rsa, "Probe I/O ports for RSA");
#endif
MODULE_ALIAS_CHARDEV_MAJOR(TTY_MAJOR);

#ifdef CONFIG_SERIAL_8250_DEPRECATED_OPTIONS
#ifndef MODULE
/* This module was renamed to 8250_core in 3.7.  Keep the old "8250" name
 * working as well for the module options so we don't break people.  We
 * need to keep the names identical and the convenient macros will happily
 * refuse to let us do that by failing the build with redefinition errors
 * of global variables.  So we stick them inside a dummy function to avoid
 * those conflicts.  The options still get parsed, and the redefined
 * MODULE_PARAM_PREFIX lets us keep the "8250." syntax alive.
 *
 * This is hacky.  I'm sorry.
 */
static void __used s8250_options(void)
{
#undef MODULE_PARAM_PREFIX
#define MODULE_PARAM_PREFIX "8250_core."

	module_param_cb(share_irqs, &param_ops_uint, &share_irqs, 0644);
	module_param_cb(nr_uarts, &param_ops_uint, &nr_uarts, 0644);
	module_param_cb(skip_txen_test, &param_ops_uint, &skip_txen_test, 0644);
#ifdef CONFIG_SERIAL_8250_RSA
	__module_param_call(MODULE_PARAM_PREFIX, probe_rsa,
		&param_array_ops, .arr = &__param_arr_probe_rsa,
		0444, -1, 0);
#endif
}
#else
MODULE_ALIAS("8250_core");
#endif
#endif
