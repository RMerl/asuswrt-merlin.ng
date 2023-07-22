
/*
<:copyright-BRCM:2019:DUAL/GPL:standard

   Copyright (c) 2019 Broadcom 
   All Rights Reserved

Unless you and Broadcom execute a separate written software license
agreement governing use of this software, this software is licensed
to you under the terms of the GNU General Public License version 2
(the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
with the following added to such license:

   As a special exception, the copyright holders of this software give
   you permission to link this software with independent modules, and
   to copy and distribute the resulting executable under terms of your
   choice, provided that you also meet, for each linked independent
   module, the terms and conditions of the license of that module.
   An independent module is a module which is not derived from this
   software.  The special exception does not apply to any modifications
   of the software.

Not withstanding the above, under no circumstances may you combine
this software in any way with any other Broadcom software provided
under a license other than the GPL, without Broadcom's express prior
written consent.

:>
*/
#include <bcm_uart.h>
#include <keep.h>
typedef char byte;
/*
 * UART Peripheral
 */
typedef struct UartChannel {
   byte fifoctl;  /* 0x00 */
#define RSTTXFIFOS   0x80
#define RSTRXFIFOS   0x40
   /* 5-bit TimeoutCnt is in low bits of this register.
    *  This count represents the number of characters
    *  idle times before setting receive Irq when below threshold
    */
   byte config;
#define XMITBREAK 0x40
#define BITS5SYM  0x00
#define BITS6SYM  0x10
#define BITS7SYM  0x20
#define BITS8SYM  0x30
#define ONESTOP      0x07
#define TWOSTOP      0x0f
   /* 4-LSBS represent STOP bits/char
    * in 1/8 bit-time intervals.  Zero
    * represents 1/8 stop bit interval.
    * Fifteen represents 2 stop bits.
    */
   byte control;
#define BRGEN     0x80  /* Control register bit defs */
#define TXEN      0x40
#define RXEN      0x20
#define LOOPBK    0x10
#define TXPARITYEN   0x08
#define TXPARITYEVEN 0x04
#define RXPARITYEN   0x02
#define RXPARITYEVEN 0x01
   byte unused0;
          
   uint32_t baudword;  /* 0x04 */
   /* When divide SysClk/2/(1+baudword) we should get 32*bit-rate
    */

   /* 0x08 */
   byte  prog_out;  /* Set value of DTR (Bit0), RTS (Bit1)
                *  if these bits are also enabled to
                *  GPIO_o
                */
#define ARMUARTEN 0x04
#define RTSEN     0x02
#define DTREN     0x01
   byte fifocfg;  /* Upper 4-bits are TxThresh, Lower are
                * RxThreshold.  Irq can be asserted
                * when rx fifo> thresh, txfifo<thresh
                */

   byte rxf_levl;  /* Read-only fifo depth */
   byte txf_levl;  /* Read-only fifo depth */

   /* 0x0c */
   byte DeltaIP_SyncIP;  /* Upper 4 bits show which bits
                   *  have changed (may set IRQ).
                   *  read automatically clears
                   *  bit.
                   *  Lower 4 bits are actual
                   *  status
                   */
   byte DeltaIPConfig_Mask; /* Upper 4 bits: 1 for posedge
                   * sense 0 for negedge sense if
                   * not configured for edge
                   * insensitive (see above)
                   * Lower 4 bits: Mask to enable
                   * change detection IRQ for
                   * corresponding GPIO_i
                   */
   byte DeltaIPEdgeNoSense; /* Low 4-bits, set corr bit to
                   * 1 to detect irq on rising
                   * AND falling edges for
                   * corresponding GPIO_i
                   * if enabled (edge insensitive)
                   */
   byte unused1;

   uint16_t intStatus; /* 0x10 */
#define DELTAIP         0x0001
#define TXUNDERR        0x0002
#define TXOVFERR        0x0004
#define TXFIFOTHOLD     0x0008
#define TXREADLATCH     0x0010
#define TXFIFOEMT       0x0020
#define RXUNDERR        0x0040
#define RXOVFERR        0x0080
#define RXTIMEOUT       0x0100
#define RXFIFOFULL      0x0200
#define RXFIFOTHOLD     0x0400
#define RXFIFONE        0x0800
#define RXFRAMERR       0x1000
#define RXPARERR        0x2000
#define RXBRK           0x4000
   uint16_t intMask;

   uint16_t Data;    /* 0x14  Write to TX, Read from RX */
                     /* bits 10:8 are BRK,PAR,FRM errors */
   uint16_t unused2;
}bcm_uart;



static vaddr_t chip_to_base(struct serial_chip *chip)
{
	struct bcm_uart_data *pd =
		container_of(chip, struct bcm_uart_data, chip);

	return io_pa_or_va(&pd->base);
}

static bool bcm_uart_have_rx_data(struct serial_chip *chip)
{
	vaddr_t base = chip_to_base(chip);

	return ((bcm_uart*)base)->intStatus & RXFIFONE;
}

static int bcm_uart_getchar(struct serial_chip *chip)
{
	vaddr_t base = chip_to_base(chip);
	uint32_t status;

	status = ((bcm_uart*)base)->intStatus;
	if(status & (RXOVFERR | RXPARERR | RXFRAMERR | RXBRK)) {
		/* RX over flow */
		if(status & RXOVFERR) {
			/* reset RX FIFO to clr interrupt */
			((bcm_uart*)base)->fifoctl |= RSTRXFIFOS;
		}
		return 0;
	}
	return ((bcm_uart*)base)->Data;
}


static void bcm_uart_putc(struct serial_chip *chip, int ch)
{
	vaddr_t base = chip_to_base(chip);
	volatile uint32_t status = 0;

	/* Wait until FIFO is empty */
	do {
		status = ((volatile bcm_uart*)base)->intStatus & TXFIFOEMT;
	} while( !status );
	((bcm_uart*)base)->Data = ch;

	status  = ((volatile bcm_uart*)base)->intStatus & (TXOVFERR|TXUNDERR);
	if( status ) {
		/* Reset TX FIFO */
		((bcm_uart*)base)->fifoctl |= RSTTXFIFOS;
	}
}

static void bcm_uart_flush(struct serial_chip *chip)
{
	chip = chip;
	/* TODO: Do something */
}


static const struct serial_ops bcm_uart_ops = {
	.flush = bcm_uart_flush,
	.getchar = bcm_uart_getchar,
	.have_rx_data = bcm_uart_have_rx_data,
	.putc = bcm_uart_putc,
};
KEEP_PAGER(bcm_uart_ops);


void bcm_uart_init(struct bcm_uart_data *pd, paddr_t pbase)
{
	pd->base.pa = pbase;
	pd->chip.ops = &bcm_uart_ops;
}
