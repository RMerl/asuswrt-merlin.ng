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
#include <linux/serial_core.h>
#include <linux/kthread.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <asm/uaccess.h>
#include <board.h>
#include <bcm_map_part.h>
#include <bcm_intr.h>

#include "hs_uart.h"

#define HS_UART_BT_API_DBG    0

#define RX_STATE_DISABLED         0
#define RX_STATE_PROCESS_HDR_TYPE 1
#define RX_STATE_PROCESS_HDR      2
#define RX_STATE_PROCESS_DATA     3

#define MAX_HDR_DECODE_RX_LEN     1536

#define HCI_TYPE_COMMAND     1
#define HCI_TYPE_ACL_DATA    2
#define HCI_TYPE_SCO_DATA    3
#define HCI_TYPE_EVENT       4
#define HCI_TYPE_LM_DIAG     7
#define HCI_TYPE_LM_DIAG_LEN 63

#define SLIP_FRAME_END       0xC0
#define SLIP_FRAME_ESC       0xDB
#define SLIP_FRAME_ESC_END   0xDC
#define SLIP_FRAME_ESC_ESC   0xDD
#define SLIP_FRAME_ESC_XON   0xDE
#define SLIP_FRAME_ESC_XOFF  0xDF
#define SW_FLOW_CONTROL_XON  0x11
#define SW_FLOW_CONTROL_XOFF 0x13

typedef enum
{
   HS_UART_BT_RX_DECODE_DISABLED,
   HS_UART_BT_RX_DECODE_ENABLED,
   HS_UART_BT_RX_DECODE_ENABLED_SWSLIP,
   HS_UART_BT_RX_DECODE_MAX,
} HS_UART_BT_RX_DECODE;

typedef struct
{
   HS_UART_BT_RX_DECODE mode;
   char                 mode_desc[40];
} HS_UART_BT_RX_DECODE_MAP;

struct hs_uart_bt_info
{
   HS_UART_BT_RX_DECODE      rx_decode_mode;
   int                       rx_decode_state;
   unsigned char             rx_data[MAX_HDR_DECODE_RX_LEN];
   int                       rx_data_index;
   int                       rx_hdr_rem;
   int                       rx_len_rem;
   struct task_struct       *thread;
   struct proc_dir_entry    *rx_decode_proc_entry;
};

#if HS_UART_BT_API_DBG
#  define HS_UART_BT_DEBUG(fmt, args...) printk( KERN_DEBUG "hs_uart_bt: " fmt, ## args)
#else
#  define HS_UART_BT_DEBUG(fmt, args...)
#endif

#define HS_UART_REG(p) ((volatile HsUartCtrlRegs *) (p)->membase)

#define HS_UART_PROC_ENTRY_ROOT "driver/hs_uart"
#define HS_UART_BT_PROC_ENTRY "bt_rx_decode"

static struct hs_uart_bt_info hs_uart_bt = {HS_UART_BT_RX_DECODE_DISABLED};

static char *rx_decode_map[] =
{
   "Bluetooth RX decode disabled",
   "Bluetooth RX decode enabled",
   "Bluetooth RX decode enabled with SW SLIP",
   "null",
};

static int hs_uart_bt_decode_slip_header( unsigned char *pEncode, int encLen, unsigned char *pDecode, int maxDecLen)
{
   int decIndex = 0;
   int encIndex = 0;
   while( (encIndex < encLen) && (decIndex < maxDecLen ))
   {
      HS_UART_BT_DEBUG("encIndex %d, encLen %d, echar %02x, decIndex %d, ma %d\n", encIndex, encLen, pEncode[encIndex], decIndex, maxDecLen);
      switch( pEncode[encIndex] )
      {
         case SLIP_FRAME_END:
            HS_UART_BT_DEBUG("unexpected\n");
            /* unexpected data */
            return -1;

         case SLIP_FRAME_ESC:
            HS_UART_BT_DEBUG("found escape char\n");
            /* start of escape sequence */
            encIndex++;
            if (encIndex == encLen)
            {
               HS_UART_BT_DEBUG("not enough header data\n");
               /* not enough data to decode */
               return -1;
            }
            
            switch( pEncode[encIndex] )
            {
               case SLIP_FRAME_ESC_END:
                  pDecode[decIndex] = SLIP_FRAME_END;
                  break;
               case SLIP_FRAME_ESC_ESC:
                  pDecode[decIndex] = SLIP_FRAME_ESC;
                  break;
               /* ESC_XON and ESC_XOFF are only possible if SW flow control is enabled
                  rely on upper layer do decide if htese are recevied in error */
               case SLIP_FRAME_ESC_XON:
                  pDecode[decIndex] = SW_FLOW_CONTROL_XON;
                  break;
               case SLIP_FRAME_ESC_XOFF:
                  pDecode[decIndex] = SW_FLOW_CONTROL_XOFF;
                  break;
               default:
                  HS_UART_BT_DEBUG("invalid escape char\n");
                  return -1;
            }
            break;

         default:
            pDecode[decIndex] = pEncode[encIndex];
            break;
      }
      HS_UART_BT_DEBUG("decode byte is %02x\n", pDecode[decIndex]);
      encIndex++;
      decIndex++;
   }

   return 0;
}

static int hs_uart_bt_get_payload_len( unsigned char *pkt_header, int pkt_header_len, int *payload_len )
{
   int           len = 0;
   unsigned char integrity;
   int           dec_success;
   unsigned char dec_header[4];
   int           ret_val = 0;

   len = 0;  
   switch (pkt_header[0])
   {
      case SLIP_FRAME_END:
         dec_success = hs_uart_bt_decode_slip_header(&pkt_header[1], pkt_header_len - 1, &dec_header[0], 4);
         if ( 0 == dec_success )
         {
            HS_UART_BT_DEBUG("dec bytes %02x %02x %02x %02x\n", dec_header[0], dec_header[1], dec_header[2], dec_header[3]);
            integrity = (dec_header[0] >> 6) & 0x01;
            if ( integrity )
            {
               len += 2;
            }
            len += ((dec_header[1] >> 4) & 0x0F);
            len += ((dec_header[2] << 4) + 1); /* account for trailing SLIP_FRAME_END */
            *payload_len = len;
         }
         else
         {
            HS_UART_BT_DEBUG("decode failed\n");
            ret_val = -1;
         }
         break;

      case HCI_TYPE_COMMAND:
      case HCI_TYPE_SCO_DATA:
         *payload_len = pkt_header[3];
         break;

      case HCI_TYPE_ACL_DATA:
         *payload_len = pkt_header[3] | (pkt_header[4] << 8);
         break;

      case HCI_TYPE_EVENT:
         *payload_len = pkt_header[2];
         break;

      case HCI_TYPE_LM_DIAG:
         *payload_len = 0; /* all data received as header */
         break;

      default:
         ret_val = -1;
         break;
   }
   
   return ret_val;
}

static void hs_uart_bt_disable_rx_decode( void )
{
   hs_uart_bt.rx_decode_state = RX_STATE_DISABLED;
   hs_uart_bt.rx_data_index   = 0;
   hs_uart_bt.rx_hdr_rem      = 0;
   hs_uart_bt.rx_len_rem      = 0;
}

static void hs_uart_bt_reset_rx_decode( void )
{
   hs_uart_bt.rx_decode_state = RX_STATE_PROCESS_HDR_TYPE;
   hs_uart_bt.rx_data_index   = 0;
   hs_uart_bt.rx_hdr_rem      = 0;
   hs_uart_bt.rx_len_rem      = 0;
}

static int hs_uart_bt_rx_decode_header_type( struct uart_port *port, unsigned char ch )
{
   int charsInserted = 0;
   
   hs_uart_bt.rx_decode_state = RX_STATE_PROCESS_HDR;
   hs_uart_bt.rx_data[hs_uart_bt.rx_data_index] = ch;
   hs_uart_bt.rx_data_index++;
   
   HS_UART_BT_DEBUG("decode header type %02x\n", ch);

   if ( hs_uart_bt.rx_decode_mode == HS_UART_BT_RX_DECODE_ENABLED_SWSLIP )
   {
      if ( ch == SLIP_FRAME_END )
      {
         hs_uart_bt.rx_hdr_rem = 4;
      }
      else
      {
         /* unknown data */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,1,0)
         charsInserted += tty_insert_flip_char(&port->state->port, ch, TTY_NORMAL);
#else
         charsInserted += tty_insert_flip_char(port->state->port.tty, ch, TTY_NORMAL);
#endif
         hs_uart_bt_disable_rx_decode();
      }
   }
   else
   {
      switch (ch)
      {
         case HCI_TYPE_ACL_DATA:
             /* SWSLIP mode or HCI_ACL header - need four more bytes */
             hs_uart_bt.rx_hdr_rem = 4;
             break;
      
         case HCI_TYPE_COMMAND:
         case HCI_TYPE_SCO_DATA:
             /* HCI_SCO or HCI_CMD - need three more bytes */
             hs_uart_bt.rx_hdr_rem = 0x3;
             break;
             
         case HCI_TYPE_EVENT:
             /* HCI_EVT - need 2 more bytes */
             hs_uart_bt.rx_hdr_rem = 2;
             break;

         case HCI_TYPE_LM_DIAG:
             hs_uart_bt.rx_hdr_rem = HCI_TYPE_LM_DIAG_LEN;
             break;
   
         default:
             HS_UART_BT_DEBUG("unknown header - %02x\n", ch);
             /* unknown packet */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,1,0)
             charsInserted += tty_insert_flip_char(&port->state->port, ch, TTY_NORMAL);
#else
             charsInserted += tty_insert_flip_char(port->state->port.tty, ch, TTY_NORMAL);
#endif
             hs_uart_bt_disable_rx_decode();
             break;
      }
   }
   return charsInserted;
}

static int hs_uart_bt_rx_decode_header( struct uart_port *port, unsigned char ch )
{
   int charsInserted = 0;

   HS_UART_BT_DEBUG("decode header %02x\n", ch);

   /* decrement count if slip is not enabled or slip is 
      enabled and char is not the escape char */
   if ( (hs_uart_bt.rx_decode_mode < HS_UART_BT_RX_DECODE_ENABLED_SWSLIP) || (SLIP_FRAME_ESC != ch) )
   {
       hs_uart_bt.rx_hdr_rem--;
   }
   hs_uart_bt.rx_data[hs_uart_bt.rx_data_index] = ch;
   hs_uart_bt.rx_data_index++;

   if ( 0 == hs_uart_bt.rx_hdr_rem )
   {
      /* have complete header */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,1,0)
      charsInserted += tty_insert_flip_string(&port->state->port, &hs_uart_bt.rx_data[0], hs_uart_bt.rx_data_index);
#else
      charsInserted += tty_insert_flip_string(port->state->port.tty, &hs_uart_bt.rx_data[0], hs_uart_bt.rx_data_index);
#endif
      if ( hs_uart_bt_get_payload_len(&hs_uart_bt.rx_data[0], hs_uart_bt.rx_data_index, &hs_uart_bt.rx_len_rem) != 0 )
      {
         HS_UART_BT_DEBUG("failed to get payload len\n");
         hs_uart_bt_disable_rx_decode();
         return charsInserted;
      }

      HS_UART_BT_DEBUG("get payload returned len %d, hdr len %d\n", hs_uart_bt.rx_len_rem, hs_uart_bt.rx_data_index);

      if ( hs_uart_bt.rx_len_rem )
      {
         hs_uart_bt.rx_data_index = 0;
         hs_uart_bt.rx_decode_state = RX_STATE_PROCESS_DATA;
      }
      else
      {
         hs_uart_bt_reset_rx_decode();
      }
   }
   return charsInserted;
}

static int hs_uart_bt_rx_decode_data( struct uart_port *port, unsigned char ch )
{
   int charsInserted = 0;
   
   HS_UART_BT_DEBUG("decode data %02x\n", ch);

   /* decrement count if slip is not enabled or slip is 
      enabled and char is not the escape char */
   if ( (hs_uart_bt.rx_decode_mode < HS_UART_BT_RX_DECODE_ENABLED_SWSLIP) || (SLIP_FRAME_ESC != ch) )
   {
       hs_uart_bt.rx_len_rem--;
   }
   hs_uart_bt.rx_data[hs_uart_bt.rx_data_index] = ch;
   hs_uart_bt.rx_data_index++;

   if ( 0 == hs_uart_bt.rx_len_rem )
   {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,1,0)
      charsInserted += tty_insert_flip_string(&port->state->port, &hs_uart_bt.rx_data[0], hs_uart_bt.rx_data_index);
#else
      charsInserted += tty_insert_flip_string(port->state->port.tty, &hs_uart_bt.rx_data[0], hs_uart_bt.rx_data_index);
#endif
      hs_uart_bt_reset_rx_decode();
   }
   else
   {
      if (hs_uart_bt.rx_data_index >= MAX_HDR_DECODE_RX_LEN)
      {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,1,0)
         charsInserted += tty_insert_flip_string(&port->state->port, &hs_uart_bt.rx_data[0], hs_uart_bt.rx_data_index);
#else
         charsInserted += tty_insert_flip_string(port->state->port.tty, &hs_uart_bt.rx_data[0], hs_uart_bt.rx_data_index);
#endif
         hs_uart_bt.rx_data_index = 0;
      }
   }

   return charsInserted;
}

static int hs_uart_bt_thread(void *arg)
{
   struct uart_port *port = (struct uart_port *)arg;
   unsigned long     flags;
   int               count = 0;
   int               quietCount = 0;

   /* Main task loop */
   while (1)
   {
      /* Abort if we were woken up to terminate */
      if (kthread_should_stop())
      {
         HS_UART_BT_DEBUG("%s: kthread_should_stop detected\n", __FUNCTION__);
         break;
      }

      spin_lock_irqsave(&port->lock, flags);
      if ( RX_STATE_DISABLED == hs_uart_bt.rx_decode_state )
      {
         /* check if we are in a quiet period */
         if ( port->icount.rx == count )
         {
            quietCount++;
            if ( quietCount > 3 )
            {
               HS_UART_BT_DEBUG("%s: enabling decode\n", __FUNCTION__);
               quietCount = 0;
               hs_uart_bt_reset_rx_decode();
            }
         }
         else
         {
            quietCount = 0;
         }
      }
      else if ( RX_STATE_PROCESS_HDR_TYPE != hs_uart_bt.rx_decode_state )
      {
         /* expecting data but none has been received */
         if ( port->icount.rx == count )
         {
            quietCount++;
            if ( quietCount > 2 )
            {
               HS_UART_BT_DEBUG("%s: disabling decode\n", __FUNCTION__);
               hs_uart_bt_disable_rx_decode();
               HS_UART_REG(port)->RFL = 1;
            }
         }
         else
         {
            quietCount = 0;
         }
      }
      count = port->icount.rx;
      spin_unlock_irqrestore(&port->lock, flags);

      set_current_state(TASK_INTERRUPTIBLE);
      schedule_timeout(msecs_to_jiffies(50));
   }

   return 0;
}

int hs_uart_decode_rx_error(struct uart_port *port)
{
   int charsInserted = 0;

   if ( hs_uart_bt.rx_decode_mode > HS_UART_BT_RX_DECODE_DISABLED )
   {
      if ( hs_uart_bt.rx_data_index )
      {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,1,0)
         charsInserted += tty_insert_flip_string(&port->state->port, &hs_uart_bt.rx_data[0], hs_uart_bt.rx_data_index);
#else
         charsInserted += tty_insert_flip_string(port->state->port.tty, &hs_uart_bt.rx_data[0], hs_uart_bt.rx_data_index);
#endif
         if ( charsInserted != hs_uart_bt.rx_data_index )
         {
            HS_UART_BT_DEBUG("hs_uart_decode_rx_error: unable to insert all chars\n");
         }
      }
      hs_uart_bt_disable_rx_decode();
   }

   return charsInserted;
}

int hs_uart_decode_rx_comp(struct uart_port *port)
{
   if ( hs_uart_bt.rx_decode_mode > HS_UART_BT_RX_DECODE_DISABLED )
   {
      switch ( hs_uart_bt.rx_decode_state )
      {
          case RX_STATE_PROCESS_HDR:
              HS_UART_REG(port)->RFL = hs_uart_bt.rx_hdr_rem;
              break;
          case RX_STATE_PROCESS_DATA:
              HS_UART_REG(port)->RFL = hs_uart_bt.rx_len_rem > MAX_HDR_DECODE_RFL ? MAX_HDR_DECODE_RFL : hs_uart_bt.rx_len_rem;
              break;
          default:
              HS_UART_REG(port)->RFL = 1;
              break;
      }
   }
   return 0;
}

int hs_uart_decode_rx_char(struct uart_port *port, unsigned char ch, int *stopRx)
{
   int charsInserted = 0;

   if ( hs_uart_bt.rx_decode_mode > HS_UART_BT_RX_DECODE_DISABLED )
   {
      switch ( hs_uart_bt.rx_decode_state )
      {
         case RX_STATE_PROCESS_HDR_TYPE:
            charsInserted += hs_uart_bt_rx_decode_header_type(port, ch);
            break;
   
         case RX_STATE_PROCESS_HDR:
            charsInserted += hs_uart_bt_rx_decode_header(port, ch);
            break;
   
         case RX_STATE_PROCESS_DATA:
            charsInserted += hs_uart_bt_rx_decode_data(port, ch);
            break;
   
         case RX_STATE_DISABLED:
         default:
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,1,0)
            charsInserted += tty_insert_flip_char(&port->state->port, ch, TTY_NORMAL);
#else
            charsInserted += tty_insert_flip_char(port->state->port.tty, ch, TTY_NORMAL);
#endif
            break;
      }
   }
   else
   {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,1,0)
      charsInserted += tty_insert_flip_char(&port->state->port, ch, TTY_NORMAL);
#else
      charsInserted += tty_insert_flip_char(port->state->port.tty, ch, TTY_NORMAL);
#endif
   }

   return charsInserted;
}

int hs_uart_decode_rx_shutdown( struct uart_port *port )
{
   unsigned long flags;

   HS_UART_BT_DEBUG("hs_uart_decode_rx_shutdown called\n");

   if ( hs_uart_bt.rx_decode_mode > HS_UART_BT_RX_DECODE_DISABLED )
   {
      if ( hs_uart_bt.thread != NULL )
      {
         kthread_stop(hs_uart_bt.thread);
         hs_uart_bt.thread = NULL;
      }

      spin_lock_irqsave(&port->lock, flags);
      hs_uart_bt_disable_rx_decode();
      spin_unlock_irqrestore(&port->lock, flags);
   }
   return 0;
}

int hs_uart_decode_rx_startup( struct uart_port *port )
{
   unsigned long flags;

   HS_UART_BT_DEBUG("hs_uart_decode_rx_startup called\n");

   if ( hs_uart_bt.rx_decode_mode > HS_UART_BT_RX_DECODE_DISABLED )
   {
      spin_lock_irqsave(&port->lock, flags);
      hs_uart_bt_reset_rx_decode();
      spin_unlock_irqrestore(&port->lock, flags);
   
      hs_uart_bt.thread = kthread_create(hs_uart_bt_thread, port, "hs_uart_bt");
      wake_up_process(hs_uart_bt.thread);
   }
   return 0;
}

/* 
 * set rx decode mode 
 */
static void hs_uart_bt_set_rx_decode( HS_UART_BT_RX_DECODE mode )
{
   unsigned int  uart_active = 0;
   int i;

   for ( i = 0; i < UART_NR; i++ )
   {
      uart_active |= HS_UART_REG(&hs_uart_ports[i])->LCR & (HS_UART_LCR_RXEN | HS_UART_LCR_TXOEN);
      uart_active |= HS_UART_REG(&hs_uart_ports[i])->MCR & HS_UART_MCR_TX_ENABLE;
   }

   /* Only allow mode change if uart is not active */
   if( uart_active )
   {
      printk(KERN_ERR "%s: xfer_mode change not allowed when UART is active\n", __FUNCTION__);
   }
   else
   {
      hs_uart_bt.rx_decode_mode = mode;
   }

}

static ssize_t hs_uart_write_mode(struct file *file, const char __user *buffer,
                                  size_t count, loff_t *ppos)
{
   char locald[16] = {0};
   unsigned long mode;
   char *endp;

   if ( count > (sizeof(locald) - 1) )
   {
      return -EINVAL;
   }

   if (copy_from_user(locald, buffer, count))
   {
      return -EFAULT;
   }

   mode = simple_strtoul(locald, &endp, 10);
   if (endp == locald)
   {
      printk(KERN_INFO "%s: Invalid parameter\n", __FUNCTION__);
      return count;
   }

   if( mode < HS_UART_BT_RX_DECODE_MAX )
   {
      hs_uart_bt_set_rx_decode( (HS_UART_BT_RX_DECODE)mode);
   }

   *ppos += count;
   return count;
}

static int hs_uart_show_mode(struct seq_file *seq, void *offset)
{
   int i;

   seq_printf(seq, "Current RX Decode mode: %d - %s\n", hs_uart_bt.rx_decode_mode, 
              rx_decode_map[hs_uart_bt.rx_decode_mode]);

   seq_printf(seq, "Available Modes:\n");
   for( i=HS_UART_BT_RX_DECODE_DISABLED; i< HS_UART_BT_RX_DECODE_MAX; i++ )
   {
      seq_printf(seq, "%d - %s\n", i, rx_decode_map[i] );
   }

   return 0;
}

static int hs_uart_bt_seq_open(struct inode *inode, struct file *file)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,1,0)
    return single_open(file, hs_uart_show_mode, PDE_DATA(inode));
#else
    return single_open(file, hs_uart_show_mode, PDE(inode)->data);
#endif
}

static struct file_operations hs_uart_bt_fops = {
   .owner = THIS_MODULE,
   .open = hs_uart_bt_seq_open,
   .read = seq_read,
   .write = hs_uart_write_mode,
   .llseek = seq_lseek,
   .release = single_release,
};

static int hs_uart_bt_create_proc_entries( void )
{
   hs_uart_bt.rx_decode_proc_entry = proc_mkdir(HS_UART_PROC_ENTRY_ROOT, NULL);
   if(hs_uart_bt.rx_decode_proc_entry == NULL)
   {
      return -ENOMEM;
   }

   proc_create(HS_UART_BT_PROC_ENTRY, S_IWUSR | S_IRUGO, hs_uart_bt.rx_decode_proc_entry, &hs_uart_bt_fops);

   return 0;
}

/*
 * Remove proc entry
 */
static int hs_uart_bt_remove_proc_entries( void )
{
   remove_proc_entry(HS_UART_BT_PROC_ENTRY, hs_uart_bt.rx_decode_proc_entry);
   return 0;
}

/*
 * hs_uart_bt module init 
 */
int __init hs_uart_decode_rx_init(void)
{
   /* Create proc entries */
   hs_uart_bt_disable_rx_decode();
   
   hs_uart_bt_create_proc_entries();
   return 1;
}

/*
 * hs_uart_bt module de-init 
 */
void __exit hs_uart_decode_rx_exit(void)
{
   /* Remove proc entries */
   hs_uart_bt_remove_proc_entries();
}
