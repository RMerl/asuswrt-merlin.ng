/*
* <:copyright-BRCM:2013:GPL/GPL:standard
* 
*    Copyright (c) 2013 Broadcom 
*    All Rights Reserved
* 
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License, version 2, as published by
* the Free Software Foundation (the "GPL").
* 
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
* 
* 
* A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
* writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
* Boston, MA 02111-1307, USA.
* 
* :>
*/

/* Description: Serial port driver for the BCM963XX. */

#include <linux/version.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 33)
#include <generated/autoconf.h>
#else
#include <linux/autoconf.h>
#endif
#include <linux/module.h>
#include <linux/tty.h>
#include <linux/ioport.h>
#include <linux/init.h>
#include <linux/serial.h>
#include <linux/delay.h>
#include <linux/console.h>
#include <linux/sysrq.h>
#include <linux/device.h>
#include <linux/version.h>
#include <linux/tty_flip.h>

#include <board.h>
#include <bcm_map_part.h>
#include <bcm_intr.h>
#include <linux/bcm_colors.h>
#ifdef __arm__
#include <mach/hardware.h>
#endif

#if defined(CONFIG_BCM_SERIAL_CONSOLE) && defined(CONFIG_MAGIC_SYSRQ)
#define SUPPORT_SYSRQ
#ifdef CONFIG_BCM_KF_SYSRQ_AUX_CHAR
static char sysrq_aux_start_char='^';
#endif
#endif

#include <linux/serial_core.h>

#if defined UART2_BASE
#define UART_NR     3
#else
#if defined UART1_BASE
#define UART_NR     2
#else
#define UART_NR     1
#endif
#endif

#define BCM63XX_PORT    63

#define BCM63XX_ISR_PASS_LIMIT  256

/* generate TXFIFOTHOLD interrupt if tx fifo level falls below this number.
 * Must define a constant becuase bcm63xx_console_write needs this and
 * (UART_REG(port))->fifocfg is not set until later.
 */
#define BCMTXFIFODRAINTHRESH  4

#define UART_REG(p) ((volatile Uart *) (p)->membase)

#ifndef IO_ADDRESS
#define IO_ADDRESS(x) (x)
#endif

static void bcm63xx_stop_tx(struct uart_port *port)
{
    (UART_REG(port))->intMask &= ~(TXFIFOEMT | TXFIFOTHOLD);
}

static void bcm63xx_stop_rx(struct uart_port *port)
{
    (UART_REG(port))->intMask &= ~RXFIFONE;
}

static void bcm63xx_enable_ms(struct uart_port *port)
{
}

static void bcm63xx_rx_chars(struct uart_port *port)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,9,1)
    struct tty_port *tty = &port->state->port; 
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,32)
    struct tty_struct *tty = port->state->port.tty;    
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,29)
    struct tty_struct *tty = port->info->port.tty;
#else
    struct tty_struct *tty = port->info->tty;
#endif
    unsigned int max_count = 256;
    unsigned short status;
    unsigned char ch, flag = TTY_NORMAL;

    status = (UART_REG(port))->intStatus;
    while ((status & RXFIFONE) && max_count--) {

        ch = (UART_REG(port))->Data;
        port->icount.rx++;

        /*
         * Note that the error handling code is
         * out of the main execution path
         */
        if (status & (RXBRK | RXFRAMERR | RXPARERR | RXOVFERR)) {
            if (status & RXBRK) {
                status &= ~(RXFRAMERR | RXPARERR);
                port->icount.brk++;
                if (uart_handle_break(port))
                    goto ignore_char;
            } 
            else if (status & RXPARERR)
                port->icount.parity++;
            else if (status & RXFRAMERR)
                port->icount.frame++;
            if (status & RXOVFERR)
                port->icount.overrun++;

            status &= port->read_status_mask;

            if (status & RXBRK)
                flag = TTY_BREAK;
            else if (status & RXPARERR)
                flag = TTY_PARITY;
            else if (status & RXFRAMERR)
                flag = TTY_FRAME;
        }

#if defined(CONFIG_BCM_SERIAL_CONSOLE) && defined(CONFIG_MAGIC_SYSRQ)
#ifdef CONFIG_BCM_KF_SYSRQ_AUX_CHAR
        /*
         * Simple hack for also detecting a regular ASCII char as the break
         * char for the start of the Magic Sysrq sequence.  This duplicates
         * some of the code in uart_handle_break() in serial_core.h
         */
        if (port->cons && port->cons->index == port->line)
        {
            if (port->sysrq == 0)
            {
                if (ch == sysrq_aux_start_char) {
                    port->sysrq = jiffies + HZ*5;
                    goto ignore_char;
                }
            }
        }
#endif
#endif

        if (uart_handle_sysrq_char(port, ch))
            goto ignore_char;

        tty_insert_flip_char(tty, ch, flag);

    ignore_char:
        status = (UART_REG(port))->intStatus;
    }
    
    tty_flip_buffer_push(tty);
}

static void bcm63xx_tx_chars(struct uart_port *port)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,32)
    struct circ_buf *xmit = &port->state->xmit;
#else
    struct circ_buf *xmit = &port->info->xmit;
#endif

    (UART_REG(port))->intMask &= ~TXFIFOTHOLD;

    if (port->x_char) {
        while (!((UART_REG(port))->intStatus & TXFIFOEMT));
        /* Send character */
        (UART_REG(port))->Data = port->x_char;
        port->icount.tx++;
        port->x_char = 0;
        return;
    }
    if (uart_circ_empty(xmit) || uart_tx_stopped(port)) {
        bcm63xx_stop_tx(port);
        return;
    }

    while ((UART_REG(port))->txf_levl < port->fifosize) {
        (UART_REG(port))->Data = xmit->buf[xmit->tail];
        xmit->tail = (xmit->tail + 1) & (UART_XMIT_SIZE - 1);
        port->icount.tx++;
        if (uart_circ_empty(xmit))
            break;
    }

    if (uart_circ_chars_pending(xmit) < WAKEUP_CHARS)
        uart_write_wakeup(port);

    if (uart_circ_empty(xmit))
        bcm63xx_stop_tx(port);
    else
        (UART_REG(port))->intMask |= TXFIFOTHOLD;
}

static void bcm63xx_modem_status(struct uart_port *port)
{
    unsigned int status;

    status = (UART_REG(port))->DeltaIP_SyncIP;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,32)
    wake_up_interruptible(&port->state->port.delta_msr_wait);    
#else
    wake_up_interruptible(&port->info->delta_msr_wait);
#endif
}

static void bcm63xx_start_tx(struct uart_port *port)
{

    /* No need for explicit spinlock taking here, since
     * calling function in kernel always takes the
     * port->lock
     */

    /* If TXFIFOEMT interrupt is enabled, this means
     * that tx is in progress, and all tx buffers
     * will be transmitted in interrupt context
     * whenever the tx fifo is empty, therefore we 
     * do nothing here
     */
    if (!((UART_REG(port))->intMask & TXFIFOEMT)) {
        /* Enable TX fifo empty interrupts and fill
         * up the TX fifo as much as possible 
         */
        (UART_REG(port))->intMask |= TXFIFOEMT;
        bcm63xx_tx_chars(port);
    }
}

static irqreturn_t bcm63xx_int(int irq, void *dev_id)
{
    struct uart_port *port = dev_id;
    unsigned int status, pass_counter = BCM63XX_ISR_PASS_LIMIT;

    spin_lock(&port->lock);

    while ((status = (UART_REG(port))->intStatus & (UART_REG(port))->intMask)) {

        if (status & RXFIFONE)
            bcm63xx_rx_chars(port);

        if (status & (TXFIFOEMT | TXFIFOTHOLD))
            bcm63xx_tx_chars(port);

        if (status & DELTAIP)
            bcm63xx_modem_status(port);

        if (pass_counter-- == 0)
            break;

    }

    spin_unlock(&port->lock);

#if !defined(CONFIG_ARM) && !defined(CONFIG_ARM64)
    // Clear the interrupt
    BcmHalInterruptEnable (irq);
#endif

    return IRQ_HANDLED;
}

static unsigned int bcm63xx_tx_empty(struct uart_port *port)
{
    return (UART_REG(port))->intStatus & TXFIFOEMT ? TIOCSER_TEMT : 0;
}

static unsigned int bcm63xx_get_mctrl(struct uart_port *port)
{
    return 0;
}

static void bcm63xx_set_mctrl(struct uart_port *port, unsigned int mctrl)
{
}

static void bcm63xx_break_ctl(struct uart_port *port, int break_state)
{
}

static int bcm63xx_startup(struct uart_port *port)
{
    unsigned int rv;
#if defined(CONFIG_ARM) || defined(CONFIG_ARM64)
    /* for ARM it will always rearm!! */
    rv = BcmHalMapInterruptEx((FN_HANDLER)bcm63xx_int,
                              (void*)port, port->irq,
                              "serial", INTR_REARM_YES,
                              INTR_AFFINITY_TP1_IF_POSSIBLE);
#else
    rv = BcmHalMapInterruptEx((FN_HANDLER)bcm63xx_int,
                              (void*)port, port->irq,
                              "serial", INTR_REARM_NO,
                              INTR_AFFINITY_TP1_IF_POSSIBLE);
#endif

    if (rv != 0)
    {
        printk(KERN_WARNING "bcm63xx_startup: failed to register "
                            "intr %d rv=%d\n", port->irq, rv);
        return rv;
    }

#if !defined(CONFIG_ARM) && !defined(CONFIG_ARM64)
    BcmHalInterruptEnable(port->irq);
#endif

    /*
     * Set TX FIFO Threshold in the upper nibble of this byte.
     */
    (UART_REG(port))->fifocfg = (BCMTXFIFODRAINTHRESH << 4);

    /*
     * Finally, enable interrupts
     */
    (UART_REG(port))->intMask = RXFIFONE;

    return 0;
}

static void bcm63xx_shutdown(struct uart_port *port)
{
    (UART_REG(port))->intMask  = 0;       
#if !defined(CONFIG_ARM) && !defined(CONFIG_ARM64)
    BcmHalInterruptDisable(port->irq);
#endif

#if defined(CONFIG_SMP)
    irq_set_affinity_hint(port->irq, NULL);
#endif
    free_irq(port->irq, port);
}

static void bcm63xx_set_termios(struct uart_port *port, 
    struct ktermios *termios, struct ktermios *old)
{
    unsigned long flags;
    unsigned int tmpVal;
    unsigned char config, control;
    unsigned int baud;

    spin_lock_irqsave(&port->lock, flags);

    /* Ask the core to calculate the divisor for us */
    baud = uart_get_baud_rate(port, termios, old, 0, port->uartclk/16); 

    /* Wait until TXFIFO is empty before changing settings */
    while (!((UART_REG(port))->intStatus & TXFIFOEMT));

    (UART_REG(port))->control = 0;
    (UART_REG(port))->config = 0;

    switch (termios->c_cflag & CSIZE) {
    case CS5:
        config = BITS5SYM;
        break;
    case CS6:
        config = BITS6SYM;
        break;
    case CS7:
        config = BITS7SYM;
        break;
    case CS8:
    default:
        config = BITS8SYM;
        break;
    }
    if (termios->c_cflag & CSTOPB)
        config |= TWOSTOP;
    else
        config |= ONESTOP;

    control = 0;
    if (termios->c_cflag & PARENB) {
        control |= (TXPARITYEN | RXPARITYEN);
        if (!(termios->c_cflag & PARODD))
            control |= (TXPARITYEVEN | RXPARITYEVEN);
    }

    /*
     * Update the per-port timeout.
     */
    uart_update_timeout(port, termios->c_cflag, baud);

    port->read_status_mask = RXOVFERR;
    if (termios->c_iflag & INPCK)
        port->read_status_mask |= RXFRAMERR | RXPARERR;
    if (termios->c_iflag & (BRKINT | PARMRK))
        port->read_status_mask |= RXBRK;

    /*
     * Characters to ignore
     */
    port->ignore_status_mask = 0;
    if (termios->c_iflag & IGNPAR)
        port->ignore_status_mask |= RXFRAMERR | RXPARERR;
    if (termios->c_iflag & IGNBRK) {
        port->ignore_status_mask |= RXBRK;
        /*
         * If we're ignoring parity and break indicators,
         * ignore overruns too (for real raw support).
         */
        if (termios->c_iflag & IGNPAR)
            port->ignore_status_mask |= RXOVFERR;
    }

    /*
     * Ignore all characters if CREAD is not set.
     */
    if ((termios->c_cflag & CREAD) == 0)
        port->ignore_status_mask |= RXFIFONE;

#if 0
    if (UART_ENABLE_MS(port, termios->c_cflag))
        (UART_REG(port))->intMask = DELTAIP;
#endif

    (UART_REG(port))->control = control;
    (UART_REG(port))->config = config;

    /* Set the FIFO interrupt depth */
    (UART_REG(port))->fifoctl = RSTTXFIFOS | RSTRXFIFOS;

    /* 
     * Write the table value to the clock select register.
     * Value = clockFreqHz / baud / 32-1
     * take into account any necessary rounding.
     */
    tmpVal = (FPERIPH / baud) / 16;
    if( tmpVal & 0x01 )
        tmpVal /= 2;  /* Rounding up, so sub is already accounted for */
    else
        tmpVal = (tmpVal / 2) - 1; /* Rounding down so we must sub 1 */

#if !defined(CONFIG_BRCM_IKOS)
    (UART_REG(port))->baudword = tmpVal;
#endif

    /* Finally, re-enable the transmitter and receiver */
    (UART_REG(port))->control |= (BRGEN|TXEN|RXEN);

    spin_unlock_irqrestore(&port->lock, flags);
}

static const char *bcm63xx_type(struct uart_port *port)
{
    return port->type == BCM63XX_PORT ? "BCM63XX" : NULL;
}

/*
 * Release the memory region(s) being used by 'port'
 */
static void bcm63xx_release_port(struct uart_port *port)
{
}

/*
 * Request the memory region(s) being used by 'port'
 */
static int bcm63xx_request_port(struct uart_port *port)
{
    return 0;
}

/*
 * Configure/autoconfigure the port.
 */
static void bcm63xx_config_port(struct uart_port *port, int flags)
{
    if (flags & UART_CONFIG_TYPE) {
        port->type = BCM63XX_PORT;
    }
}

/*
 * verify the new serial_struct (for TIOCSSERIAL).
 */
static int bcm63xx_verify_port(struct uart_port *port, struct serial_struct *ser)
{
    return 0;
}

#ifdef CONFIG_CONSOLE_POLL
static int bcm63xx_get_poll_char(struct uart_port *port)
{
    int status;
    unsigned char ch;

    status = (UART_REG(port))->intStatus;

    if (!(status & RXFIFONE))
        return NO_POLL_CHAR;

    ch = (UART_REG(port))->Data;
    /* BUG: no error handling */

    return ch;
}

static void bcm63xx_put_poll_char(struct uart_port *port,
         unsigned char c)
{
    /*
     * tx fifo must not be full and also not low enough
     * to trigger TXFIFOTHOLD or TXFIFOEMPT
     */
    while ((UART_REG(port))->txf_levl > BCMTXFIFODRAINTHRESH + 1)  {};

   (UART_REG(port))->Data = c;

   return;
}
#endif /* CONFIG_CONSOLE_POLL */

static struct uart_ops bcm63xx_pops = {
    .tx_empty    = bcm63xx_tx_empty,
    .set_mctrl    = bcm63xx_set_mctrl,
    .get_mctrl    = bcm63xx_get_mctrl,
    .stop_tx    = bcm63xx_stop_tx,
    .start_tx    = bcm63xx_start_tx,
    .stop_rx    = bcm63xx_stop_rx,
    .enable_ms    = bcm63xx_enable_ms,
    .break_ctl    = bcm63xx_break_ctl,
    .startup    = bcm63xx_startup,
    .shutdown    = bcm63xx_shutdown,
    .set_termios    = bcm63xx_set_termios,
    .type        = bcm63xx_type,
    .release_port    = bcm63xx_release_port,
    .request_port    = bcm63xx_request_port,
    .config_port    = bcm63xx_config_port,
    .verify_port    = bcm63xx_verify_port,
#ifdef CONFIG_CONSOLE_POLL
    .poll_get_char = bcm63xx_get_poll_char,
    .poll_put_char = bcm63xx_put_poll_char,
#endif
};

static struct uart_port bcm63xx_ports[] = {
    {
#ifndef UART_PHYS_BASE
#define UART_PHYS_BASE  UART_BASE
#endif
        .mapbase    = UART_PHYS_BASE,
        .iotype     = SERIAL_IO_MEM,
        .uartclk    = 14745600,
        .fifosize   = 16,
        .ops        = &bcm63xx_pops,
        .flags      = ASYNC_BOOT_AUTOCONF,
        .line       = 0,
    },
#if defined UART1_BASE
    {
#ifndef UART1_PHYS_BASE
#define UART1_PHYS_BASE  UART1_BASE
#endif
        .mapbase    = UART1_PHYS_BASE,
        .iotype     = SERIAL_IO_MEM,
        .uartclk    = 14745600,
        .fifosize   = 16,
        .ops        = &bcm63xx_pops,
        .flags      = ASYNC_BOOT_AUTOCONF,
        .line       = 1,
    },
#endif
#if defined UART2_BASE
    {
#ifndef UART2_PHYS_BASE
#define UART2_PHYS_BASE  UART2_BASE
#endif
        .mapbase    = UART2_PHYS_BASE,
        .iotype     = SERIAL_IO_MEM,
        .uartclk    = 14745600,
        .fifosize   = 16,
        .ops        = &bcm63xx_pops,
        .flags      = ASYNC_BOOT_AUTOCONF,
        .line       = 2,
    }
#endif
};

#ifdef CONFIG_BCM_SERIAL_CONSOLE

static void bcm63xx_console_putc(struct uart_port *port, int c)
{
    while ((UART_REG(port))->txf_levl > BCMTXFIFODRAINTHRESH + 1) 
    {
       udelay(20); 
    }

    /* Send character */
    (UART_REG(port))->Data = c;

}

static void
bcm63xx_console_write(struct console *co, const char *s, unsigned int count)
{
    struct uart_port *port = &bcm63xx_ports[co->index];

#if !defined(CONFIG_BCM_KF_PRINTK_INT_ENABLED) || !defined(CONFIG_BCM_PRINTK_INT_ENABLED)
    unsigned long flags;
    int locked;

    local_irq_save(flags);

    if (port->sysrq) 
    {
        /* bcm63xx_int() already took the lock */
        locked = 0;
    } 
    else if (oops_in_progress) 
    {
        locked = spin_trylock(&port->lock);
    } 
    else 
    {
        spin_lock(&port->lock);
        locked = 1;
    }
#endif    

    uart_console_write(port, s, count, bcm63xx_console_putc);

#if !defined(CONFIG_BCM_KF_PRINTK_INT_ENABLED) || !defined(CONFIG_BCM_PRINTK_INT_ENABLED)
    if (locked)
            spin_unlock(&port->lock);

    local_irq_restore(flags);
#endif    

}

static int __init bcm63xx_console_setup(struct console *co, char *options)
{
    struct uart_port *port;
    int baud = 115200;
    int bits = 8;
    int parity = 'n';
    int flow = 'n';

    /*
     * Check whether an invalid uart number has been specified, and
     * if so, set it to port 0
     */
    if (co->index >= UART_NR)
        co->index = 0;
    port = &bcm63xx_ports[co->index];

    if (options)
        uart_parse_options(options, &baud, &parity, &bits, &flow);

    return uart_set_options(port, co, baud, parity, bits, flow);
}

struct uart_driver;
static struct uart_driver bcm63xx_reg;
static struct console bcm63xx_console = {
    .name        = "ttyS",
    .write        = bcm63xx_console_write,
    .device        = uart_console_device,
    .setup        = bcm63xx_console_setup,
    .flags        = CON_PRINTBUFFER,
    .index        = -1,
    .data        = &bcm63xx_reg,
};

static int __init bcm63xx_console_init(void)
{
    bcm63xx_ports[0].membase = (unsigned char*)UART_BASE;
    bcm63xx_ports[0].irq = INTERRUPT_ID_UART;
#if defined UART1_BASE
    bcm63xx_ports[1].membase = (unsigned char*)UART1_BASE;
    bcm63xx_ports[1].irq = INTERRUPT_ID_UART1;
#endif
#if defined UART2_BASE
    bcm63xx_ports[2].membase = (unsigned char*)UART2_BASE;
    bcm63xx_ports[2].irq = INTERRUPT_ID_UART2;
#endif    
    register_console(&bcm63xx_console);
    return 0;
}
console_initcall(bcm63xx_console_init);

#define BCM63XX_CONSOLE    &bcm63xx_console
#else
#define BCM63XX_CONSOLE    NULL
#endif

static struct uart_driver bcm63xx_reg = {
    .owner            = THIS_MODULE,
    .driver_name    = "bcmserial",
    .dev_name        = "ttyS",
    .major            = TTY_MAJOR,
    .minor            = 64,
    .nr                = UART_NR,
    .cons            = BCM63XX_CONSOLE,
};

static int __init serial_bcm63xx_init(void)
{
    int ret;
    int i;

    printk(KERN_INFO "Serial: BCM63XX driver $Revision: 3.00 $\n");

#if defined(CONFIG_BCM_SERIAL_CONSOLE) && defined(CONFIG_MAGIC_SYSRQ)
#ifdef CONFIG_BCM_KF_SYSRQ_AUX_CHAR
    printk(KERN_INFO CLRy "Magic SysRq with Auxilliary trigger char enabled "
                          "(type %c h for list of supported commands)"
                          CLRnl, sysrq_aux_start_char);
#endif
#endif

    ret = uart_register_driver(&bcm63xx_reg);
    if (ret >= 0) {
        for (i = 0; i < UART_NR; i++) {
            uart_add_one_port(&bcm63xx_reg, &bcm63xx_ports[i]);
        }
    } else {
        uart_unregister_driver(&bcm63xx_reg);
    }
    return ret;
}

static void __exit serial_bcm63xx_exit(void)
{
    uart_unregister_driver(&bcm63xx_reg);
}

module_init(serial_bcm63xx_init);
module_exit(serial_bcm63xx_exit);

MODULE_DESCRIPTION("BCM63XX serial port driver $Revision: 3.00 $");
MODULE_LICENSE("GPL");
