/*
<:copyright-BRCM:2021:DUAL/GPL:standard

   Copyright (c) 2021 Broadcom 
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
/***************************************************************************/
#ifndef _IUDMA_H
#define _IUDMA_H


#include <linux/dma-direction.h>
#include <linux/dma-mapping.h>
#include<pcm_regs.h>
#include <dsphal_chip.h>

/****************************************************************************
* Typedefs and Constants
****************************************************************************/
#define IUDMA_STATE_MASK           0xc0000000
#define IUDMA_STATE_SHIFT          30
#define IUDMA_BYTESDONE_MASK       0x0fff0000
#define IUDMA_BYTESDONE_SHIFT      16
#define IUDMA_RINGOFFSET_MASK      0x00003fff
#define IUDMA_RINGOFFSET_SHIFT     0

struct dma_addr
{
   char*       cpu; /* virtual address */
   dma_addr_t  dma; /* physical address */
};
struct iudma_ctrl_regs
{
   uint32_t ctrl_config;
#define IUDMA_REGS_CTRLCONFIG_MASTER_EN        0x0001
#define IUDMA_REGS_CTRLCONFIG_FLOWC_CH1_EN     0x0002
#define IUDMA_REGS_CTRLCONFIG_FLOWC_CH3_EN     0x0004
#define IUDMA_REGS_CTRLCONFIG_FLOWC_CH5_EN     0x0008
#define IUDMA_REGS_CTRLCONFIG_FLOWC_CH7_EN     0x0010

   // Flow control Ch1
   uint32_t ch1_FC_Low_Thr;
   uint32_t ch1_FC_High_Thr;
   uint32_t ch1_Buff_Alloc;

   // Flow control Ch3
   uint32_t ch3_FC_Low_Thr;
   uint32_t ch3_FC_High_Thr;
   uint32_t ch3_Buff_Alloc;

   // Flow control Ch5
   uint32_t ch5_FC_Low_Thr;
   uint32_t ch5_FC_High_Thr;
   uint32_t ch5_Buff_Alloc;

   // Flow control Ch7
   uint32_t ch7_FC_Low_Thr;
   uint32_t ch7_FC_High_Thr;
   uint32_t ch7_Buff_Alloc;

   uint32_t channel_reset;
   uint32_t channel_debug;

   uint32_t reserved;
   uint32_t gbl_int_stat;
   uint32_t gbl_int_mask;
#if IUDMA_NUM_CHANNELS == 6
  #define IUDMA_IRQ_TX_PCM      0x00000020
  #define IUDMA_IRQ_RX_PCM      0x00000010
  #define IUDMA_IRQ_TX_B        0x00000008
  #define IUDMA_IRQ_RX_B        0x00000004
  #define IUDMA_IRQ_TX_A        0x00000002
  #define IUDMA_IRQ_RX_A        0x00000001
#else
  #define IUDMA_IRQ_TX_PCM      0x00000002
  #define IUDMA_IRQ_RX_PCM      0x00000001
#endif
};

struct iudma_chan_ctrl
{
    uint32_t config;
#define IUDMA_CONFIG_ENDMA         0x00000001
#define IUDMA_CONFIG_PKTHALT       0x00000002
#define IUDMA_CONFIG_BURSTHALT     0x00000004

    uint32_t int_status;
#define IUDMA_INTSTAT_BDONE        0x00000001
#define IUDMA_INTSTAT_PDONE        0x00000002
#define IUDMA_INTSTAT_NOTVLD       0x00000004
#define IUDMA_INTSTAT_RXDMAERROR   0x00000008
#define IUDMA_INTSTAT_REPIN_ERROR  0x00000010
#define IUDMA_INTSTAT_MASK         0x00000007

    uint32_t int_mask;
#define IUDMA_INTMASK_BDONE        0x00000001
#define IUDMA_INTMASK_PDONE        0x00000002
#define IUDMA_INTMASK_NOTVLD       0x00000004

    uint32_t max_burst;
#define IUDMA_MAXBURST_SIZE        1 /* 32-bit words */
};

struct iudma_state_ram
{
   uint32_t base_desc_pointer;
   uint32_t state_bytes_done_ring_offset;
#define IUDMA_STRAM_DESC_RING_OFFSET  0x3fff
   uint32_t flags_length_status;
   uint32_t current_buffer_pointer;
};

struct iudma_descr
{
   uint32_t flags;
#define IUDMA_FLAGS_LENGTH_MASK    0xffff0000
#define IUDMA_FLAGS_LENGTH_SHIFT   16
#define IUDMA_FLAGS_STATUS_MASK    0x0000ffff
#define IUDMA_FLAGS_STATUS_SHIFT   0

#define IUDMA_STATUS_OWN           0x8000
#define IUDMA_STATUS_EOP           0x4000
#define IUDMA_STATUS_SOP           0x2000
#define IUDMA_STATUS_WRAP          0x1000
#define IUDMA_STATUS_PERIPH        0x0fff

   uint32_t addr;
};

struct iudma
{
   struct iudma_ctrl_regs  regs;
   uint32_t                reserved1[110];
   struct iudma_chan_ctrl  ctrl[IUDMA_NUM_CHANNELS];
   uint32_t                reserved2[104];
#if IUDMA_NUM_CHANNELS == 2
   uint32_t                reserved3[16];
#endif
   struct iudma_state_ram  stram[IUDMA_NUM_CHANNELS];

};
extern void __iomem *apm_reg; 
#define IUDMA    ((volatile struct iudma * const) (((char*)apm_reg)+APM_DMA_CTRL))


#endif /* _IUDMA_H */
