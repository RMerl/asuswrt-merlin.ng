/*
<:copyright-BRCM:2015:DUAL/GPL:standard 

   Copyright (c) 2015 Broadcom 
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

#ifndef __BCM_IUDMA_H
#define __BCM_IUDMA_H

#ifdef __cplusplus
extern "C" {
#endif
#if defined(CONFIG_BCM963178)
/*
** DMA Buffer
*/
typedef union DmaDesc {
    struct {
        union {
            struct {
                uint16_t        status;                   /* buffer status */
#define          DMA_OWN                0x8000  /* cleared by DMA, set by SW */
#define          DMA_EOP                0x4000  /* last buffer in packet */
#define          DMA_SOP                0x2000  /* first buffer in packet */
#define          DMA_WRAP               0x1000  /* */
#define          DMA_PRIO               0x0C00  /* Prio for Tx */
#define          DMA_APPEND_BRCM_TAG    0x0200
#define          DMA_APPEND_CRC         0x0100
#define          USB_ZERO_PKT           (1<< 0) // Set to send zero length packet
                uint16_t        length;                   /* in bytes of data in buffer */
#define          DMA_DESC_USEFPM    0x8000
#define          DMA_DESC_MULTICAST 0x4000
#define          DMA_DESC_BUFLENGTH 0x0fff
            };
            uint32_t      word0;
        };

        uint32_t        address;                /* address of data */
    };
    uint64_t u64_0;
} IUDmaDesc;

/*
** DMA Channel Configuration (1 .. 20)
*/
typedef struct SAR_DmaChannelCfg {
  uint32_t        cfg;                    /* (00) assorted configuration */
#define         DMA_ENABLE      0x00000001  /* set to enable channel */
#define         DMA_PKT_HALT    0x00000002  /* idle after an EOP flag is detected */
#define         DMA_BURST_HALT  0x00000004  /* idle after finish current memory burst */
  uint32_t        intStat;                /* (04) interrupts control and status */
  uint32_t        intMask;                /* (08) interrupts mask */
#define         DMA_BUFF_DONE   0x00000001  /* buffer done */
#define         DMA_DONE        0x00000002  /* packet xfer complete */
#define         DMA_NO_DESC     0x00000004  /* no valid descriptors */
#define         DMA_RX_ERROR    0x00000008  /* rxdma detect client protocol error */
  uint32_t        maxBurst;               /* (0C) max burst length permitted */
#define         DMA_DESCSIZE_SEL 0x00040000  /* DMA Descriptor Size Selection */
} SAR_DmaChannelCfg;

/*
** DMA State RAM (1 .. 16)
*/
typedef struct SAR_DmaStateRam {
  uint32_t        baseDescPtr;            /* (00) descriptor ring start address */
  uint32_t        state_data;             /* (04) state/bytes done/ring offset */
  uint32_t        desc_len_status;        /* (08) buffer descriptor status and len */
  uint32_t        desc_base_bufptr;       /* (0C) buffer descrpitor current processing */
} SAR_DmaStateRam;

/*
** DMA Registers
*/

#define IUDMA_MAX_CHANNELS          20

typedef struct SAR_IUDMAControlReg {
    uint32_t controller_cfg;              /* (00) controller configuration */
#define DMA_MASTER_EN           0x00000001
#define DMA_FLOWC_CH1_EN        0x00000002
#define DMA_FLOWC_CH3_EN        0x00000004

    // Flow control Ch1
    uint32_t flowctl_ch1_thresh_lo;           /* 004 */
    uint32_t flowctl_ch1_thresh_hi;           /* 008 */
    uint32_t flowctl_ch1_alloc;               /* 00c */
#define DMA_BUF_ALLOC_FORCE     0x80000000

    // Flow control Ch3
    uint32_t flowctl_ch3_thresh_lo;           /* 010 */
    uint32_t flowctl_ch3_thresh_hi;           /* 014 */
    uint32_t flowctl_ch3_alloc;               /* 018 */

    // Flow control Ch5
    uint32_t flowctl_ch5_thresh_lo;           /* 01C */
    uint32_t flowctl_ch5_thresh_hi;           /* 020 */
    uint32_t flowctl_ch5_alloc;               /* 024 */

    // Flow control Ch7
    uint32_t flowctl_ch7_thresh_lo;           /* 028 */
    uint32_t flowctl_ch7_thresh_hi;           /* 02C */
    uint32_t flowctl_ch7_alloc;               /* 030 */

    uint32_t ctrl_channel_reset;              /* 034 */
    uint32_t ctrl_channel_debug;              /* 038 */
    uint32_t reserved1;                       /* 03C */
    uint32_t ctrl_global_interrupt_status;    /* 040 */
    uint32_t ctrl_global_interrupt_mask;      /* 044 */

    // Unused words
    uint8_t reserved2[0x200-0x48];

    // Per channel registers/state ram
    SAR_DmaChannelCfg chcfg[IUDMA_MAX_CHANNELS];/* (200-33F) Channel configuration */

    uint8_t reserved3[0x400-0x340];

    union {
        SAR_DmaStateRam     s[IUDMA_MAX_CHANNELS];
        uint32_t              u32[4 * IUDMA_MAX_CHANNELS];
    } stram;                                /* (400-53F) state ram */
} SAR_IUDMAControlReg;
#endif

#ifdef __cplusplus
}
#endif
#endif //__BCM_IUDMA_H
