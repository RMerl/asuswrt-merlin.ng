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

#ifndef __HS_UART_H
#define __HS_UART_H

#include <linux/serial_core.h>

/*
 * High Speed Uart Control
 */
typedef struct HsUartCtrlRegs {
    unsigned int      reserved[5];
    unsigned int      ptu_hc;         /* 14 */
#define HS_UART_PTU_HC_DATA (1 << 1)
    unsigned int      reserved1;
    unsigned int      uart_data;
    unsigned int      reserved2[25];  /* 20 */
    unsigned int      uart_int_stat;  /* 84 */
    unsigned int      reserved3[8];   /* 88 */
    unsigned int      uart_int_en;    /* a8 */
#define HS_UART_TXFIFOFULL    (1 << 0) /* tx full          */
#define HS_UART_TXFIFOAEMPTY  (1 << 1) /* tx almost empty  */
#define HS_UART_RXFIFOAFULL   (1 << 2) /* rx almost full   */
#define HS_UART_RXFIFOEMPTY   (1 << 3) /* rx empty         */
#define HS_UART_RXFIFORES     (1 << 4) /* rx residue       */
#define HS_UART_RXPARITYERR   (1 << 5) /* rx parity error  */
#define HS_UART_RXBRKDET      (1 << 6) /* rx uart break    */
#define HS_UART_RXCTS         (1 << 7) /* uart cts         */
#define HS_UART_INT_MASK (0xff)
#define HS_UART_INT_MASK_DISABLE (0x00000000)
    unsigned int      reserved4[53];  /* ac */
    unsigned int      dhbr;           /* 180 */
    unsigned int      dlbr;
#define HS_UART_DHBR_3000K    0x00000001 /* Target is 16 clocks/baud(bit), Based on 50Mhz Uart clk */
#define HS_UART_DLBR_3000K    0x000000ff /* Target is 16 clocks/baud(bit), Based on 50Mhz Uart clk */
#define HS_UART_DHBR_115200   0x00000011 /* Target is 16 clocks/baud(bit), Based on 50Mhz Uart clk */
#define HS_UART_DLBR_115200   0x000000e5 /* Target is 16 clocks/baud(bit), Based on 50Mhz Uart clk */
    unsigned int      ab0;
    unsigned int      reserved5;
    unsigned int      FCR;            /* 190 */
#define HS_UART_FCR_AUTOLOAD_MODE (1 << 8)
#define HS_UART_FCR_AB_MODE       (1 << 7)
#define HS_UART_FCR_AUTO_RTS_OE   (1 << 6)
#define HS_UART_FCR_AUTO_TX_OE    (1 << 5)
#define HS_UART_FCR_AUTOBAUD      (1 << 4)
#define HS_UART_FCR_TX_PACKET     (1 << 3)
#define HS_UART_FCR_SLIP_RESYNC   (1 << 2)
    unsigned int      ab1;
    unsigned int      reserved6;
    unsigned int      LCR;            /* 19c */
#define HS_UART_LCR_SLIP_TX_CRC       (1 << 11)
#define HS_UART_LCR_SLIP_CRC_LSBFIRST (1 << 10)
#define HS_UART_LCR_SLIP_CRC_INV      (1 << 9)
#define HS_UART_LCR_SLIP_RX_CRC       (1 << 8)
#define HS_UART_LCR_SLIP              (1 << 7)
#define HS_UART_LCR_RTSOEN            (1 << 6)
#define HS_UART_LCR_TXOEN             (1 << 5)
#define HS_UART_LCR_LBC               (1 << 4)
#define HS_UART_LCR_RXEN              (1 << 3)
#define HS_UART_LCR_EPS               (1 << 2)
#define HS_UART_LCR_PEN               (1 << 1)
#define HS_UART_LCR_STB               (1 << 0)
    unsigned int      MCR;
#define HS_UART_MCR_REPEAT_XON_XOFF (1 << 9)
#define HS_UART_MCR_PACKET_FLOW     (1 << 8)
#define HS_UART_MCR_BAUD_ADJ_EN     (1 << 7)
#define HS_UART_MCR_AUTO_TX_DISABLE (1 << 6)
#define HS_UART_MCR_AUTO_RTS        (1 << 5)
#define HS_UART_MCR_LOOPBACK        (1 << 4)
#define HS_UART_MCR_HIGH_RATE       (1 << 3)
#define HS_UART_MCR_XON_XOff_EN     (1 << 2)
#define HS_UART_MCR_PROG_RTS        (1 << 1)
#define HS_UART_MCR_TX_ENABLE       (1 << 0)
    unsigned int      LSR;
#define HS_UART_LSR_TX_HALT        (1 << 5)
#define HS_UART_LSR_TX_PACKET_RDY  (1 << 4)
#define HS_UART_LSR_TX_IDLE        (1 << 3)
#define HS_UART_LSR_TX_DATA_AVAIL  (1 << 2) /* 1 - Data in TX FIFO, 0 - TX FIFO empty */
#define HS_UART_LSR_RX_FULL        (1 << 1) /* 1 - RX FIFO full */
#define HS_UART_LSR_RX_OVERFLOW    (1 << 0) /* 1 - RX FIFO Overflow */
    unsigned int      MSR;
#define HS_UART_MSR_RX_IN     (1 << 2)
#define HS_UART_MSR_RTS_STAT  (1 << 1)
#define HS_UART_MSR_CTS_STAT  (1 << 0)
    unsigned int      RFL;            /* 1ac */
    unsigned int      TFL;
    unsigned int      RFC;
#define HS_UART_RFC_1039BYTES_FC_DATA (1039)
#define HS_UART_RFC_NO_FC_DATA (HS_UART_RFC_1039BYTES_FC_DATA) // or should be LLI_BUFF_SIZE
    unsigned int      ESC;            /* 1b8 */
#define HS_UART_ESC_SLIP_DATA (0xDB)
#define HS_UART_ESC_NO_SLIP_DATA (0xDA)
    unsigned int      reserved7[3];
    unsigned int      HOPKT_LEN;      /* 1c8 */
    unsigned int      HIPKT_LEN;
    unsigned int      HO_DMA_CTL;
    unsigned int      HI_DMA_CTL;
    unsigned int      HO_BSIZE;
    unsigned int      HI_BSIZE;
#define HS_UART_DMA_CTL_BURSTMODE_EN   (1 << 3)
#define HS_UART_DMA_CTL_AFMODE_EN      (1 << 2)
#define HS_UART_DMA_CTL_FASTMODE_EN    (1 << 1)
#define HS_UART_DMA_CTL_DMA_EN         (1 << 0)
#define HS_UART_HO_BSIZE_DATA (0x3)
#define HS_UART_HI_BSIZE_DATA (0x3)
} HsUartCtrlRegs;


#define UART_NR            1
#define MAX_HDR_DECODE_RFL 768

extern struct uart_port hs_uart_ports[];

#endif /* __HS_UART_H */
