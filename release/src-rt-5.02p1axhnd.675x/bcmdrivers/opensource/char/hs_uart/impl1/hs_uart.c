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
// BCMFORMAT: notabs reindent:uncrustify:bcm_minimal_i3.cfg

/* Description: Bluetooth Serial port driver for the BCM963XX. */

#include <linux/version.h>
#include <generated/autoconf.h>
#include <linux/module.h>
#include <linux/tty.h>
#include <linux/ioport.h>
#include <linux/init.h>
#include <linux/serial.h>
#include <linux/console.h>
#include <linux/sysrq.h>
#include <linux/device.h>
#include <linux/version.h>
#include <linux/tty_flip.h>
#include <linux/slab.h>
#include <linux/completion.h>
#include <board.h>
#include <bcm_map_part.h>
#include <bcm_intr.h>
#include <linux/bcm_colors.h>
#include <asm/delay.h>

#include <linux/serial_core.h>
#include "hs_uart.h"
#include "shared_utils.h"

/* Note on locking policy: All uart ops take the
   port->lock, except startup shutdown and termios */

/******* Defines & Types *******/
#define HS_UART_PRINT_CHARS       0
#define HS_UART_DUMP_REGS         0
#define HS_UART_API_DEBUG         0
#define HS_UART_MIN_BAUD          9600     /* Arbitrary value, this can be as low as 9600 */
#define HS_UART_CALCULATE_DLHBR   1        /* 1 - Calculate dl/hbr values for any baudrates, 0 - use predefined values */

#define HS_UART_LOOPBACK_ENABLE   0

#undef HS_UART_DEBUG             /* undef it, just in case */
#if HS_UART_API_DEBUG
#  define HS_UART_DEBUG(fmt, args...) pr_info("hs_uart: " fmt, ## args)
#else
#  define HS_UART_DEBUG(fmt, args...) /* not debugging: nothing */
#endif

#define HS_UART_PORT    66

/* generate HS_UART_TXFIFOAEMPTYTHRESH interrupt if tx fifo level falls below this number.
 * Must define a constant becuase hs_uart_console_write needs this and
 * HS_UART_REG(port)->fifocfg is not set until later. */
#define HS_UART_TXFIFOAEMPTYTHRESH      100 /* Threhsold indicates when TX FIFO has more room */
#define HS_UART_RXFIFOAFULLTHRESH       1   /* Threshold indicates when RX FIFO is not empty */

#if defined(CONFIG_BCM94908)
#define HS_UART_REG(p) ((volatile HsUartCtrlRegs *) IO_ADDRESS(HS_UART_BASE))
#else
#define HS_UART_REG(p) ((volatile HsUartCtrlRegs *) (p)->membase)
#endif

#ifndef IO_ADDRESS
#define IO_ADDRESS(x) (x)
#endif

/******* Private prototypes *******/
static void deinit_hs_uart_port(struct uart_port * port );
static int init_hs_uart_port( struct uart_port * port );
static unsigned int hs_uart_tx_empty( struct uart_port * port );
static void hs_uart_set_mctrl( struct uart_port * port, unsigned int mctrl );
static unsigned int hs_uart_get_mctrl( struct uart_port * port );
static void hs_uart_stop_tx( struct uart_port * port );
static void hs_uart_start_tx( struct uart_port * port );
static void hs_uart_stop_rx( struct uart_port * port );
static void hs_uart_enable_ms( struct uart_port * port );
static void hs_uart_break_ctl( struct uart_port * port, int break_state );
static int hs_uart_startup( struct uart_port * port );
static void hs_uart_shutdown( struct uart_port * port );
static int hs_uart_set_baud_rate( struct uart_port *port, int baud );
static void hs_uart_set_termios( struct uart_port * port, struct ktermios *termios, struct ktermios *old );
static const char * hs_uart_type( struct uart_port * port );
static void hs_uart_release_port( struct uart_port * port );
static int hs_uart_request_port( struct uart_port * port );
static void hs_uart_config_port( struct uart_port * port, int flags );
static int hs_uart_verify_port( struct uart_port * port, struct serial_struct *ser );
#if HS_UART_DUMP_REGS
static void hs_uart_dump_regs(struct uart_port * port);
#endif

/******* Local Variables *******/
static struct uart_ops hs_uart_pops =
{
   .tx_empty     = hs_uart_tx_empty,
   .set_mctrl    = hs_uart_set_mctrl,
   .get_mctrl    = hs_uart_get_mctrl,
   .stop_tx      = hs_uart_stop_tx,
   .start_tx     = hs_uart_start_tx,
   .stop_rx      = hs_uart_stop_rx,
   .enable_ms    = hs_uart_enable_ms,
   .break_ctl    = hs_uart_break_ctl,
   .startup      = hs_uart_startup,
   .shutdown     = hs_uart_shutdown,
   .set_termios  = hs_uart_set_termios,
   .type         = hs_uart_type,
   .release_port = hs_uart_release_port,
   .request_port = hs_uart_request_port,
   .config_port  = hs_uart_config_port,
   .verify_port  = hs_uart_verify_port,
};

struct uart_port hs_uart_ports[] =
{
   {
#if defined(CONFIG_BCM963138) || defined(CONFIG_BCM963148)
      .membase    = (void *)IO_ADDRESS(HS_UART_PHYS_BASE),
      .mapbase    = HS_UART_PHYS_BASE,
      .irq        = INTERRUPT_ID_HS_UART,
#endif
      .iotype     = SERIAL_IO_MEM,
      .uartclk    = 50000000,
      .fifosize   = 1040,
      .ops        = &hs_uart_pops,
      .flags      = ASYNC_BOOT_AUTOCONF,
      .line       = 0,
   }
};

static struct uart_driver hs_uart_reg =
{
   .owner            = THIS_MODULE,
   .driver_name      = "hsuart",
   .dev_name         = "ttyH",
   .major            = TTY_MAJOR,
   .minor            = 70,
   .nr               = UART_NR,
};

/******* Functions ********/
int __init __attribute__ ((weak)) hs_uart_decode_rx_init(void) { return 0;};
int __exit __attribute__ ((weak)) hs_uart_decode_rx_exit(void) { return 0;};
int __attribute__ ((weak)) hs_uart_decode_rx_startup(struct uart_port *port) { return 0;};
int __attribute__ ((weak)) hs_uart_decode_rx_shutdown(struct uart_port *port) { return 0;};
int __attribute__ ((weak)) hs_uart_decode_rx_error(struct uart_port *port) { return 0;};
int __attribute__ ((weak)) hs_uart_decode_rx_comp(struct uart_port *port) { return 0;};
int __attribute__ ((weak)) hs_uart_decode_rx_char(struct uart_port *port, unsigned char ch, int *stopRx)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,1,0)
   return tty_insert_flip_char(&port->state->port, ch, TTY_NORMAL);
#else
   return tty_insert_flip_char(port->state->port.tty, ch, TTY_NORMAL);
#endif
};

/*
 * Enable ms
 */
static void hs_uart_enable_ms(struct uart_port *port)
{
   HS_UART_DEBUG("%s\n", __FUNCTION__);
}

/*
 * Get MCR register
 */
static unsigned int hs_uart_get_mctrl(struct uart_port *port)
{
   unsigned int mctrl = 0;

   HS_UART_DEBUG("%s\n", __FUNCTION__);

   if( !(HS_UART_REG(port)->MSR & HS_UART_MSR_CTS_STAT) )
   {
      mctrl |= TIOCM_CTS;
   }

   if( !(HS_UART_REG(port)->MSR & HS_UART_MSR_RTS_STAT) )
   {
      mctrl |= TIOCM_RTS;
   }

   return mctrl;
}

/*
 * Set MCR register
 */
static void hs_uart_set_mctrl(struct uart_port *port, unsigned int mctrl)
{
   HS_UART_DEBUG("%s\n", __FUNCTION__);

   if (HS_UART_REG(port)->MCR & HS_UART_MCR_AUTO_RTS)
      return;

   if (mctrl & TIOCM_RTS)
   {
      HS_UART_REG(port)->MCR |= HS_UART_MCR_PROG_RTS;
   }
   else
   {
      HS_UART_REG(port)->MCR &= ~HS_UART_MCR_PROG_RTS;
   }
}

/*
 * Set break state
 */
static void hs_uart_break_ctl(struct uart_port *port, int break_state)
{
   HS_UART_DEBUG("%s\n", __FUNCTION__);
   if (break_state)
      HS_UART_REG(port)->LCR |= HS_UART_LCR_LBC;
   else
      HS_UART_REG(port)->LCR &= ~HS_UART_LCR_LBC;
}

/*
 * Initialize serial port
 */
static int hs_uart_startup(struct uart_port *port)
{
   unsigned long flags;

   hs_uart_decode_rx_startup(port);

   spin_lock_irqsave(&port->lock, flags);
   init_hs_uart_port( port );
   spin_unlock_irqrestore(&port->lock, flags);

   HS_UART_DEBUG("%s\n", __FUNCTION__);
   return 0;
}

/*
 * Shutdown serial port
 */
static void hs_uart_shutdown(struct uart_port *port)
{
   unsigned long flags;

#if HS_UART_DUMP_REGS
   hs_uart_dump_regs(port);
#endif

   hs_uart_decode_rx_shutdown(port);

   spin_lock_irqsave(&port->lock, flags);
   deinit_hs_uart_port(port);
   spin_unlock_irqrestore(&port->lock, flags);

   HS_UART_DEBUG("%s\n", __FUNCTION__);
}

/*
 * Release the memory region(s) being used by 'port'
 */
static void hs_uart_release_port(struct uart_port *port)
{
   HS_UART_DEBUG("%s\n", __FUNCTION__);
}

/*
 * Request the memory region(s) being used by 'port'
 */
static int hs_uart_request_port(struct uart_port *port)
{
   HS_UART_DEBUG("%s\n", __FUNCTION__);
   return 0;
}

/*
 * verify the new serial_struct (for TIOCSSERIAL).
 */
static int hs_uart_verify_port(struct uart_port *port, struct serial_struct *ser)
{
   HS_UART_DEBUG("%s\n", __FUNCTION__);
   return 0;
}

/*
 * Disable tx transmission
 */
static void hs_uart_stop_tx(struct uart_port *port)
{
   HS_UART_DEBUG("%s\n", __FUNCTION__);
   HS_UART_REG(port)->uart_int_en &= ~HS_UART_TXFIFOAEMPTY;
}

/*
 * Disable rx reception
 */
static void hs_uart_stop_rx(struct uart_port *port)
{
   HS_UART_DEBUG("%s\n", __FUNCTION__);
   HS_UART_REG(port)->uart_int_en &= ~HS_UART_RXFIFOAFULL;
}

/*
 * Receive rx chars - Called from ISR
 */
static void hs_uart_rx_chars(struct uart_port *port)
{
   unsigned int       status;
   unsigned char      ch;
   int                charsInserted = 0;
   int                stopRx;

   /* data received, clear empty status */
   HS_UART_REG(port)->uart_int_stat = HS_UART_RXFIFOEMPTY;
   do
   {
      /* Keep reading from RX FIFO as long as it is not empty */
      status = HS_UART_REG(port)->uart_int_stat;
      if ( status & HS_UART_RXFIFOEMPTY )
      {
         break;
      }

      if (unlikely(HS_UART_REG(port)->LSR & HS_UART_LSR_RX_OVERFLOW))
      {
         HS_UART_DEBUG("HS UART overrun detected\n");

         charsInserted += hs_uart_decode_rx_error(port);
         port->icount.overrun++;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,1,0)
         charsInserted += tty_insert_flip_char(&port->state->port, 0, TTY_OVERRUN);
#else
         charsInserted += tty_insert_flip_char(port->state->port.tty, 0, TTY_OVERRUN);
#endif
         /* overrun does not affect current character */
      }

      ch = HS_UART_REG(port)->uart_data;

      if (unlikely(status & (HS_UART_RXBRKDET | HS_UART_RXPARITYERR)))
      {
         unsigned short tmpStatus;
         unsigned char  flag;
         if (status & HS_UART_RXBRKDET)
         {
            port->icount.brk++;
            if (uart_handle_break(port))
            {
               continue;
            }
         }
         else
         {
            port->icount.parity++;
         }

         /* update flag according to read_status_mask */
         tmpStatus = status & port->read_status_mask;
         if (tmpStatus & HS_UART_RXBRKDET)
         {
            flag = TTY_BREAK;
         }
         else
         {
            flag = TTY_PARITY;
         }

         charsInserted += hs_uart_decode_rx_error(port);
         HS_UART_REG(port)->uart_int_stat = (HS_UART_RXBRKDET | HS_UART_RXPARITYERR);
         HS_UART_DEBUG("Receive error\n");

         if ((status & port->ignore_status_mask) == 0)
         {
#if HS_UART_PRINT_CHARS
            printk(KERN_INFO "%s: Received char: %02x\n", __FUNCTION__, ch);
#endif
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,1,0)
             tty_insert_flip_char(&port->state->port, ch, flag);
#else
             tty_insert_flip_char(port->state->port.tty, ch, flag);
#endif
         }
         continue;
      }

#if HS_UART_PRINT_CHARS
      printk(KERN_INFO "%s: Received char: %02x\n", __FUNCTION__, ch);
#endif
      stopRx = 0;
      charsInserted += hs_uart_decode_rx_char(port, ch, &stopRx);
      if ( stopRx )
      {
         break;
      }
   } while ( 1 );

   hs_uart_decode_rx_comp(port);
   if (charsInserted > 0)
   {
      port->icount.rx += charsInserted;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,1,0)
      tty_flip_buffer_push(&port->state->port);
#else
      tty_flip_buffer_push(port->state->port.tty);
#endif
   }
}

/*
 * Transmit tx chars
 */
static void hs_uart_tx_chars(struct uart_port *port)
{
   struct circ_buf *xmit = &port->state->xmit;

   HS_UART_REG(port)->uart_int_en   &= ~HS_UART_TXFIFOAEMPTY;
   HS_UART_REG(port)->uart_int_stat  = HS_UART_TXFIFOFULL;
   if (port->x_char)
   {
      if (0 == (HS_UART_REG(port)->uart_int_stat & HS_UART_TXFIFOFULL))
      {
         /* TX FIFO has room - send x_char */
         HS_UART_REG(port)->uart_data = port->x_char;
         port->icount.tx++;
         port->x_char = 0;
      }
      else
      {
         HS_UART_REG(port)->uart_int_stat  = HS_UART_TXFIFOAEMPTY;
         HS_UART_REG(port)->uart_int_en   |= HS_UART_TXFIFOAEMPTY;
      }
      return;
   }

   if (uart_tx_stopped(port))
   {
      return;
   }

   /* Write data until TX-FIFO is full OR circbuff is empty */
   while ( 1 )
   {
      if ( uart_circ_empty(xmit) )
      {
         break;
      }

      if ( (HS_UART_REG(port)->uart_int_stat & HS_UART_TXFIFOFULL) != 0)
      {
         /* still have characters to send */
         HS_UART_REG(port)->uart_int_stat  = HS_UART_TXFIFOAEMPTY;
         HS_UART_REG(port)->uart_int_en   |= HS_UART_TXFIFOAEMPTY;
         break;
      }

#if HS_UART_PRINT_CHARS
      printk(KERN_INFO "%s: Sent char: %02x\n", __FUNCTION__, (unsigned char)xmit->buf[xmit->tail]);
#endif
      HS_UART_REG(port)->uart_data = xmit->buf[xmit->tail];
      /* UART_XMIT_SIZE is power of 2 */
      xmit->tail = (xmit->tail + 1) & (UART_XMIT_SIZE - 1);
      port->icount.tx++;
   }

   if (uart_circ_chars_pending(xmit) < WAKEUP_CHARS)
      uart_write_wakeup(port);
}

/*
 * Enable TX transmission
 */
static void hs_uart_start_tx(struct uart_port *port)
{
   hs_uart_tx_chars(port);
}

/*
 * hs_uart ISR
 */
static irqreturn_t hs_uart_int(int irq, void *dev_id)
{
   struct uart_port *port = dev_id;
   unsigned int status;

   spin_lock(&port->lock);
   status = HS_UART_REG(port)->uart_int_stat & HS_UART_REG(port)->uart_int_en;

   /* We have RX data to process */
   if (status & HS_UART_RXFIFOAFULL)
   {
      hs_uart_rx_chars(port);
   }

   /* TX FIFO now has room */
   if (status & HS_UART_TXFIFOAEMPTY)
   {
      hs_uart_tx_chars(port);
   }

   if (status & HS_UART_RXCTS)
   {
      uart_handle_cts_change(port, status & HS_UART_RXCTS);
   }

   /* clearing status before processing events has no effect
      so interrutps are cleared here */
   HS_UART_REG(port)->uart_int_stat = status;

   spin_unlock(&port->lock);

#if !defined(CONFIG_ARM) && !defined(CONFIG_ARM64)
   /* re-enable the interrupt */
   BcmHalInterruptEnable (irq);
#endif

   return IRQ_HANDLED;
}

/*
 * Check if TX FIFO is empty
 */
static unsigned int hs_uart_tx_empty(struct uart_port *port)
{
   HS_UART_DEBUG("%s\n", __FUNCTION__);
   //When no data is available -> TX FIFO is empty
   return HS_UART_REG(port)->LSR & HS_UART_LSR_TX_DATA_AVAIL ? 0 : TIOCSER_TEMT;
}

/*
 * Set hs uart baudrate registers
 */
static int hs_uart_set_baud_rate( struct uart_port *port, int baud )
{
#if HS_UART_CALCULATE_DLHBR
   unsigned int extraCyc;
   unsigned int intDiv;

   HS_UART_DEBUG("%s: requested baud: %d\n", __FUNCTION__, baud);
   intDiv = port->uartclk / (16 * baud);
   if (0 == intDiv)
   {
      /* Calculate the integer divider */
      printk(KERN_WARNING "%s: Unable to set baudrate to: %d\n", __FUNCTION__, baud);
   }
   else
   {
      /* Calculate the integer divider */
      HS_UART_REG(port)->dlbr = 256 - intDiv;
      HS_UART_REG(port)->MCR &= ~HS_UART_MCR_HIGH_RATE;

      /* Calculate the extra cycles of uartclk required to full-  *
       * -fill the bit timing requirements for required baudrate  */
      extraCyc =  ( ( (port->uartclk * 10) / baud ) - (intDiv*16*10) + 5 ) / 10;
      if( extraCyc )
      {
         /* Equally distribute the extraCycles at the start and end of bit interval */
         HS_UART_REG(port)->dhbr = ( extraCyc/2 | (extraCyc/2) << 4 ) + extraCyc % 2;
      }
      else
      {
         HS_UART_REG(port)->dhbr = 0;
      }

      if( extraCyc > 16 )
      {
         printk(KERN_WARNING "hs_uart_set_baud_rate: Cannot set required extra cycles %d ", extraCyc);
      }

      HS_UART_DEBUG("dlbr: 0x%08x, dhbr: 0x%08x\n", (unsigned int)HS_UART_REG(port)->dlbr,
                                                    (unsigned int)HS_UART_REG(port)->dhbr);
      HS_UART_DEBUG("%s: Setting baudrate to: %d\n", __FUNCTION__, baud);
   }

#else

   /* Use predefined values */
   switch( baud )
   {
   case 3000000:
      HS_UART_REG(port)->dhbr = HS_UART_DHBR_3000K;
      HS_UART_REG(port)->dlbr = HS_UART_DLBR_3000K;
      break;

   case 115200:
   default:
      HS_UART_REG(port)->dhbr = HS_UART_DHBR_115200;
      HS_UART_REG(port)->dlbr = HS_UART_DLBR_115200;
      baud = 115200;
      break;
   }
#endif /* HS_UART_CALCULATE_DLHBR */

   return 0;
}

/*
 * Set terminal options
 */
static void hs_uart_set_termios(struct uart_port *port,
   struct ktermios *termios, struct ktermios *old)
{
   unsigned long flags;
   unsigned int baud;

   HS_UART_DEBUG("%s\n", __FUNCTION__);

   spin_lock_irqsave(&port->lock, flags);

   /* Wait until TXFIFO is empty before changing settings */
   while( HS_UART_REG(port)->LSR & HS_UART_LSR_TX_DATA_AVAIL );

   /* Ask the core to return selected baudrate value ( bps ) */
   baud = uart_get_baud_rate(port, termios, old, HS_UART_MIN_BAUD, port->uartclk/8);

   /* Set baud rate registers */
   hs_uart_set_baud_rate(port, baud);

   /* Set stop bits */
   if (termios->c_cflag & CSTOPB)
      HS_UART_REG(port)->LCR |= HS_UART_LCR_STB;
   else
      HS_UART_REG(port)->LCR &= ~(HS_UART_LCR_STB);

   /* Set Parity */
   if (termios->c_cflag & PARENB)
   {
      HS_UART_REG(port)->LCR |= HS_UART_LCR_PEN;
      if (!(termios->c_cflag & PARODD))
         HS_UART_REG(port)->LCR |= HS_UART_LCR_EPS;
      else
         HS_UART_REG(port)->LCR &= ~HS_UART_LCR_EPS;
   }
   else
   {
      HS_UART_REG(port)->LCR &= ~HS_UART_LCR_PEN;
   }

   if (termios->c_cflag & CRTSCTS)
   {
      HS_UART_REG(port)->MCR &= ~HS_UART_MCR_PROG_RTS;
      HS_UART_REG(port)->MCR |= HS_UART_MCR_AUTO_RTS;
      HS_UART_REG(port)->uart_int_en |= HS_UART_RXCTS;
   }
   else
   {
      HS_UART_REG(port)->uart_int_en &= ~HS_UART_RXCTS;
//    HS_UART_REG(port)->MCR |= HS_UART_MCR_PROG_RTS;
      HS_UART_REG(port)->MCR &= ~HS_UART_MCR_AUTO_RTS;
   }

   /* Update the per-port timeout */
   uart_update_timeout(port, termios->c_cflag, baud);

   /* Unused in this driver */
   port->read_status_mask = 0;
   port->ignore_status_mask = 0;

   spin_unlock_irqrestore(&port->lock, flags);
}

/*
 * Check serial type
 */
static const char *hs_uart_type(struct uart_port *port)
{
   HS_UART_DEBUG("%s\n", __FUNCTION__);
   return port->type == HS_UART_PORT ? "HS_UART" : NULL;
}

/*
 * Configure/autoconfigure the port.
 */
static void hs_uart_config_port(struct uart_port *port, int flags)
{
   HS_UART_DEBUG("%s\n", __FUNCTION__);
   if (flags & UART_CONFIG_TYPE)
   {
      port->type = HS_UART_PORT;
   }
}

/*
 * Initialize serial port registers.
 */
static int init_hs_uart_port( struct uart_port * port )
{
   unsigned int temp;

   /* Disable TX/RX */
   HS_UART_REG(port)->LCR = 0;
   HS_UART_REG(port)->MCR = 0;

   /* Assign HC data */
   HS_UART_REG(port)->ptu_hc = HS_UART_PTU_HC_DATA;

#if defined(CONFIG_BCM963138) || defined(CONFIG_BCM963148)
   /* Route hsuart signals out on uart2 pins */
   UART1->prog_out |= ARMUARTEN;
#endif

   /* Disable/Clear interrupts */
   HS_UART_REG(port)->uart_int_stat = HS_UART_INT_MASK;
   HS_UART_REG(port)->uart_int_en = HS_UART_INT_MASK_DISABLE;

   /* Config FCR */
   HS_UART_REG(port)->FCR = 0;

   /* Config LCR - 1 Stop bit (HS_UART_LCR_STB is 2 stop-bits) */
   HS_UART_REG(port)->LCR = 0;
   HS_UART_REG(port)->LCR |= HS_UART_LCR_RTSOEN;

   /* Config MCR - Enable baud rate adjustment */
   HS_UART_REG(port)->MCR = HS_UART_MCR_BAUD_ADJ_EN;

#if HS_UART_LOOPBACK_ENABLE
   /* Config MCR - Set loopback */
   HS_UART_REG(port)->MCR |= HS_UART_MCR_LOOPBACK;
#endif

   /* Set TX-almost-empty and RX-almost-full thresholds*/
   HS_UART_REG(port)->TFL = HS_UART_TXFIFOAEMPTYTHRESH;

   /* Set Recieve FIFO flow control register */
   HS_UART_REG(port)->RFC = HS_UART_RFC_NO_FC_DATA;

   /* Set escape character register */
   HS_UART_REG(port)->ESC = HS_UART_ESC_NO_SLIP_DATA;


   /* Clear DMA packet lengths and set burst length */
   HS_UART_REG(port)->HOPKT_LEN  = 0;
   HS_UART_REG(port)->HIPKT_LEN  = 0;
   HS_UART_REG(port)->HO_BSIZE   = HS_UART_HO_BSIZE_DATA;
   HS_UART_REG(port)->HI_BSIZE   = HS_UART_HI_BSIZE_DATA;

   HS_UART_REG(port)->RFL = HS_UART_RXFIFOAFULLTHRESH;

   /* Clear RX FIFO - interrupt stat was cleared above */
   while( !(HS_UART_REG(port)->uart_int_stat & HS_UART_RXFIFOEMPTY) )
   {
      temp = HS_UART_REG(port)->uart_data;
   }

   /* Config LCR - RX/TX enables */
   HS_UART_REG(port)->LCR |= HS_UART_LCR_RXEN | HS_UART_LCR_TXOEN;

   /* Config MCR - TX state machine enable */
   HS_UART_REG(port)->MCR |= HS_UART_MCR_TX_ENABLE;

   /* Flush TX FIFO */
   while( (HS_UART_REG(port)->LSR & HS_UART_LSR_TX_DATA_AVAIL) &&
         !(HS_UART_REG(port)->LSR & HS_UART_LSR_TX_HALT)  );

   /* Enable RX FIFO almost full interrupt */
   HS_UART_REG(port)->uart_int_en = HS_UART_RXFIFOAFULL;

#if HS_UART_DUMP_REGS
   hs_uart_dump_regs(port);
#endif

   return 0;
}

/*
 * De-initialize serial port registers
 */
static void deinit_hs_uart_port(struct uart_port * port )
{
   /* Disable and clear interrupts */
   HS_UART_REG(port)->uart_int_en = HS_UART_INT_MASK_DISABLE;
   HS_UART_REG(port)->uart_int_stat = HS_UART_INT_MASK;

   /* Stop all DMAs */
   HS_UART_REG(port)->HI_DMA_CTL = 0;
   HS_UART_REG(port)->HO_DMA_CTL = 0;

   /* Config LCR - RX/TX disables */
   HS_UART_REG(port)->LCR = 0;

   /* Config MCR - TX state machine disable */
   HS_UART_REG(port)->MCR = 0;
}

#if HS_UART_DUMP_REGS
static void hs_uart_dump_regs(struct uart_port * port)
{
#  define REG_NVA(m) #m,(unsigned)HS_UART_REG(port)->m,&HS_UART_REG(port)->m
   pr_info("%-13s : 0x%08x @ 0x%p\n", REG_NVA(ptu_hc));
   pr_info("%-13s : 0x%08x @ 0x%p\n", REG_NVA(uart_int_stat));
   pr_info("%-13s : 0x%08x @ 0x%p\n", REG_NVA(uart_int_en));
   pr_info("%-13s : 0x%08x @ 0x%p\n", REG_NVA(dhbr));
   pr_info("%-13s : 0x%08x @ 0x%p\n", REG_NVA(dlbr));
   pr_info("%-13s : 0x%08x @ 0x%p\n", REG_NVA(ab0));
   pr_info("%-13s : 0x%08x @ 0x%p\n", REG_NVA(FCR));
   pr_info("%-13s : 0x%08x @ 0x%p\n", REG_NVA(ab1));
   pr_info("%-13s : 0x%08x @ 0x%p\n", REG_NVA(LCR));
   pr_info("%-13s : 0x%08x @ 0x%p\n", REG_NVA(MCR));
   pr_info("%-13s : 0x%08x @ 0x%p\n", REG_NVA(LSR));
   pr_info("%-13s : 0x%08x @ 0x%p\n", REG_NVA(MSR));
   pr_info("%-13s : 0x%08x @ 0x%p\n", REG_NVA(RFL));
   pr_info("%-13s : 0x%08x @ 0x%p\n", REG_NVA(TFL));
   pr_info("%-13s : 0x%08x @ 0x%p\n", REG_NVA(RFC));
   pr_info("%-13s : 0x%08x @ 0x%p\n", REG_NVA(ESC));
   pr_info("%-13s : 0x%08x @ 0x%p\n", REG_NVA(HOPKT_LEN));
   pr_info("%-13s : 0x%08x @ 0x%p\n", REG_NVA(HIPKT_LEN));
   pr_info("%-13s : 0x%08x @ 0x%p\n", REG_NVA(HO_DMA_CTL));
   pr_info("%-13s : 0x%08x @ 0x%p\n", REG_NVA(HI_DMA_CTL));
   pr_info("%-13s : 0x%08x @ 0x%p\n", REG_NVA(HO_BSIZE));
   pr_info("%-13s : 0x%08x @ 0x%p\n", REG_NVA(HI_BSIZE));
#  undef REG_NVA
}
#endif

/*
 * hs_uart module init
 */
static int __init hs_uart_init(void)
{
   int ret;
   int i;

   printk(KERN_INFO "HS UART: Driver $Revision: 1.00 $\n");

   /* Register driver with serial core */
   ret = uart_register_driver(&hs_uart_reg);
   if( ret < 0 )
      goto out;

#if (UART_NR != 1)
#error "hs_uart.c requires recoding to handle more than 1 High Speed UART"
#endif

   for (i = 0; i < UART_NR; i++)
   {
#if !defined(CONFIG_BCM963138) && !defined(CONFIG_BCM963148)
#if defined(CONFIG_BCM963158)
      if( UtilGetChipId() == 0x63158 && UtilGetChipRev() == 0xA0 )
      {
          hs_uart_ports[i].membase = (void *)HS_UART_BASE_A0;
          hs_uart_ports[i].mapbase = (uintptr_t)HS_UART_BASE_A0;
      }
      else
#endif
      {
          hs_uart_ports[i].membase = (void *)HS_UART_BASE;
          hs_uart_ports[i].mapbase = (uintptr_t)HS_UART_BASE;
      }
      hs_uart_ports[i].irq     = INTERRUPT_ID_HS_UART;
#endif
      /* Register port with serial core */
      ret = uart_add_one_port(&hs_uart_reg, &hs_uart_ports[i]);

      if( ret < 0 )
         goto out;

#if defined(CONFIG_ARM) || defined(CONFIG_ARM64)
      /* for ARM it will always rearm!! */
      ret = BcmHalMapInterruptEx((FN_HANDLER)hs_uart_int,
                             (void*)&hs_uart_ports[i],
                             hs_uart_ports[i].irq,
                             "hs_uart", INTR_REARM_YES,
                             INTR_AFFINITY_TP1_IF_POSSIBLE);
#else
      ret = BcmHalMapInterruptEx((FN_HANDLER)hs_uart_int,
                             (void*)&hs_uart_ports[i],
                             hs_uart_ports[i].irq,
                             "hs_uart", INTR_REARM_NO,
                             INTR_AFFINITY_TP1_IF_POSSIBLE);
#endif
      if (ret != 0)
      {
         pr_warning("%s: failed to register intr %d rv=%d\n",
            __FUNCTION__, hs_uart_ports[i].irq, ret);
         goto out;
      }
      else
      {
         HS_UART_DEBUG("%s: Successfully registered intr %d\n",
            __FUNCTION__, hs_uart_ports[i].irq);
      }
   }

   /* init decode */
   hs_uart_decode_rx_init();

   goto init_ok;

out:
   uart_unregister_driver(&hs_uart_reg);
init_ok:
   return ret;
}


/*
 * hs_uart module de-init
 */
static void __exit hs_uart_exit(void)
{
   int i;

   for (i = 0; i < UART_NR; i++)
   {
#if !defined(CONFIG_ARM) && !defined(CONFIG_ARM64)
      BcmHalInterruptDisable(hs_uart_ports[i].irq);
#endif
      free_irq(hs_uart_ports[i].irq, &hs_uart_ports[i]);
   }

   /* Unregister with serial core */
   uart_unregister_driver(&hs_uart_reg);

   /* de-init decode */
   hs_uart_decode_rx_exit();

}

module_init(hs_uart_init);
module_exit(hs_uart_exit);

MODULE_DESCRIPTION("BCM63XX serial port driver $Revision: 3.00 $");
MODULE_LICENSE("GPL");
