/*
<:copyright-BRCM:2021:DUAL/GPL:standard

   Copyright (c) 2021 Broadcom 
   All Rights Reserved

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License, version 2, as published by
the Free Software Foundation (the "GPL").

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.


A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.

:>
*/
/***************************************************************************/
#ifndef _PCMHAL_CHIP_H
#define _PCMHAL_CHIP_H

#include <linux/bcm_colors.h>

/****************************************************************************
* Typedefs and Constants
****************************************************************************/
#define PCM_MS_PER_TICK            5
#define PCM_FRAMESYNC_RATE         8000
#define NUM_BUF_PER_CHAN           2 /* double-buffer scheme */

#define PCM_BYTES_PER_SAMPLE       sizeof(uint16_t)
#define PCM_BYTES_PER_SAMPLE_WB    sizeof(uint32_t)
#define PCM_TS_SAMPLES_PER_MS      (PCM_FRAMESYNC_RATE/1000)
#define PCM_FRAMES_PER_TICK        (PCM_TS_SAMPLES_PER_MS * PCM_MS_PER_TICK)
#define PCM_FRAME_BUF_SIZE(size)   ((size + 7) & (~7)) /* must be multiple of 8 bytes */

#define NUM_PCM_DMA_CHAN           2


struct debug_stats
{
   uint64_t isr_counts;
   uint64_t dma_realigns;
   uint64_t dma_restarts;
   unsigned int tx_underflow_counter;
   unsigned int rx_overflow_counter;
   unsigned int secondDescUsed;
#define TX_RX_CHANS_PCM 2
   unsigned int repin_error[TX_RX_CHANS_PCM];
   unsigned int rxdma_error[TX_RX_CHANS_PCM];
   unsigned int notvld[TX_RX_CHANS_PCM];
   unsigned int pdone[TX_RX_CHANS_PCM];
   unsigned int bdone[TX_RX_CHANS_PCM];

   unsigned int txUfNoTxNotvld; /* tx underflow but not tx NOTVLD */
   unsigned int txNotvldNoTxUf;
   unsigned int rxOfNoRxNotvld;
   unsigned int rxNotvldNoRxOf;
   unsigned int rxOfNoTxUf;
   unsigned int txUfNoRxOf;
};

#define CLRbr          BCMCOLOR("\e[0;31;1m")
#define PHLOG_PRINT(level,x,...)    do { \
     if(level <= loglevel)               \
        printk(x, ##__VA_ARGS__);        \
   } while(0)
#define PHLOG_ALERT(x,...)   PHLOG_PRINT(1, KERN_ALERT    CLRyr "[PCMHAL] " CLRnorm x, ##__VA_ARGS__)
#define PHLOG_CRIT(x,...)    PHLOG_PRINT(2, KERN_CRIT     CLRbr "[PCMHAL] " CLRnorm x, ##__VA_ARGS__)
#define PHLOG_ERR(x,...)     PHLOG_PRINT(3, KERN_ERR      CLRr  "[PCMHAL] " CLRnorm x, ##__VA_ARGS__)
#define PHLOG_WARNING(x,...) PHLOG_PRINT(4, KERN_WARNING  CLRy  "[PCMHAL] " CLRnorm x, ##__VA_ARGS__)
#define PHLOG_NOTICE(x,...)  PHLOG_PRINT(5, KERN_NOTICE   CLRc  "[PCMHAL] " CLRnorm x, ##__VA_ARGS__)
#define PHLOG_INFO(x,...)    PHLOG_PRINT(6, KERN_INFO     CLRm  "[PCMHAL] " CLRnorm x, ##__VA_ARGS__)
#define PHLOG_DEBUG(x,...)   PHLOG_PRINT(7, KERN_DEBUG    CLRg  "[PCMHAL] " CLRnorm x, ##__VA_ARGS__)



#endif /* _PCMHAL_CHIP_H */
