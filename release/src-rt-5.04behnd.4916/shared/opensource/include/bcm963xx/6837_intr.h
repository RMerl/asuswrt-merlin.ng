/*
<:copyright-BRCM:2012:DUAL/GPL:standard 

   Copyright (c) 2012 Broadcom 
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

#ifndef __6837_INTR_H
#define __6837_INTR_H

#ifdef __cplusplus
    extern "C" {
#endif

/*=====================================================================*/
/* SPI Table Offset                                                    */
/*=====================================================================*/
#define SPI_TABLE_OFFSET             32

/*=====================================================================*/
/* Physical Interrupt IDs                                              */
/*=====================================================================*/
#define INTERRUPT_PER_EXT_0         (SPI_TABLE_OFFSET + 47)
#define INTERRUPT_WAN_XGRX          (SPI_TABLE_OFFSET + 161)
#define INTERRUPT_WAN_XGTX_INTR2    (SPI_TABLE_OFFSET + 163)
#define INTERRUPT_WAN_XGTX_INTR1    (SPI_TABLE_OFFSET + 164)
#define INTERRUPT_WAN_XGTX_INTR0    (SPI_TABLE_OFFSET + 165)
#define INTERRUPT_WAN_GPON_TX       (SPI_TABLE_OFFSET + 172)
#define INTERRUPT_WAN_GPON_RX       (SPI_TABLE_OFFSET + 173)
      
#define INTERRUPT_XRDP_QUEUE_0      (SPI_TABLE_OFFSET + 96)
#define INTERRUPT_XRDP_QUEUE_1      (SPI_TABLE_OFFSET + 97)
#define INTERRUPT_XRDP_QUEUE_2      (SPI_TABLE_OFFSET + 98)
#define INTERRUPT_XRDP_QUEUE_3      (SPI_TABLE_OFFSET + 99)
#define INTERRUPT_XRDP_QUEUE_4      (SPI_TABLE_OFFSET + 100)
#define INTERRUPT_XRDP_QUEUE_5      (SPI_TABLE_OFFSET + 101)
#define INTERRUPT_XRDP_QUEUE_6      (SPI_TABLE_OFFSET + 102)
#define INTERRUPT_XRDP_QUEUE_7      (SPI_TABLE_OFFSET + 103)
#define INTERRUPT_XRDP_QUEUE_8      (SPI_TABLE_OFFSET + 104)
#define INTERRUPT_XRDP_QUEUE_9      (SPI_TABLE_OFFSET + 105)
#define INTERRUPT_XRDP_QUEUE_10     (SPI_TABLE_OFFSET + 106)
#define INTERRUPT_XRDP_QUEUE_11     (SPI_TABLE_OFFSET + 107)
#define INTERRUPT_XRDP_QUEUE_12     (SPI_TABLE_OFFSET + 108)
#define INTERRUPT_XRDP_QUEUE_13     (SPI_TABLE_OFFSET + 109)
#define INTERRUPT_XRDP_QUEUE_14     (SPI_TABLE_OFFSET + 110)
#define INTERRUPT_XRDP_QUEUE_15     (SPI_TABLE_OFFSET + 111)
#define INTERRUPT_XRDP_QUEUE_16     (SPI_TABLE_OFFSET + 112)
#define INTERRUPT_XRDP_QUEUE_17     (SPI_TABLE_OFFSET + 113)
#define INTERRUPT_XRDP_QUEUE_18     (SPI_TABLE_OFFSET + 114)
#define INTERRUPT_XRDP_QUEUE_19     (SPI_TABLE_OFFSET + 115)
#define INTERRUPT_XRDP_QUEUE_20     (SPI_TABLE_OFFSET + 116)
#define INTERRUPT_XRDP_QUEUE_21     (SPI_TABLE_OFFSET + 117)
#define INTERRUPT_XRDP_QUEUE_22     (SPI_TABLE_OFFSET + 118)
#define INTERRUPT_XRDP_QUEUE_23     (SPI_TABLE_OFFSET + 119)
#define INTERRUPT_XRDP_QUEUE_24     (SPI_TABLE_OFFSET + 120)
#define INTERRUPT_XRDP_QUEUE_25     (SPI_TABLE_OFFSET + 121)
#define INTERRUPT_XRDP_QUEUE_26     (SPI_TABLE_OFFSET + 122)
#define INTERRUPT_XRDP_QUEUE_27     (SPI_TABLE_OFFSET + 123)
#define INTERRUPT_XRDP_QUEUE_28     (SPI_TABLE_OFFSET + 124)
#define INTERRUPT_XRDP_QUEUE_29     (SPI_TABLE_OFFSET + 125)
#define INTERRUPT_XRDP_QUEUE_30     (SPI_TABLE_OFFSET + 126)
#define INTERRUPT_XRDP_QUEUE_31     (SPI_TABLE_OFFSET + 127)
#define INTERRUPT_XRDP_QUEUE_32     (SPI_TABLE_OFFSET + 128)      
#define INTERRUPT_XRDP_FPM          (INTERRUPT_XRDP_QUEUE_32) 

#ifndef __ASSEMBLER__
/*=====================================================================*/
/* Linux(Virtual) Interrupt IDs                                        */
/* Each physical irq id to be mapped should be added to                */
/* bcm_phys_irqs_to_map array in board_aarch64.c file                  */
/*=====================================================================*/
#define _2MAP(V) (bcm_legacy_irq_map[(V - SPI_TABLE_OFFSET)])
#define INTERRUPT_ID_EXTERNAL_0           (bcm_legacy_irq_map[INTERRUPT_PER_EXT_0 - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_WAN_XGRX             _2MAP(INTERRUPT_WAN_XGRX)
#define INTERRUPT_ID_WAN_XGTX_INTR2       _2MAP(INTERRUPT_WAN_XGTX_INTR2)        
#define INTERRUPT_ID_WAN_XGTX_INTR1       _2MAP(INTERRUPT_WAN_XGTX_INTR1)
#define INTERRUPT_ID_WAN_XGTX_INTR0       _2MAP(INTERRUPT_WAN_XGTX_INTR0)
#define INTERRUPT_ID_WAN_GPON_TX          _2MAP(INTERRUPT_WAN_GPON_TX) 
#define INTERRUPT_ID_WAN_GPON_RX          _2MAP(INTERRUPT_WAN_GPON_RX) 
#define INTERRUPT_ID_XRDP_QUEUE_0         _2MAP(INTERRUPT_XRDP_QUEUE_0)
#define INTERRUPT_ID_XRDP_QUEUE_1         _2MAP(INTERRUPT_XRDP_QUEUE_1)
#define INTERRUPT_ID_XRDP_QUEUE_2         _2MAP(INTERRUPT_XRDP_QUEUE_2)
#define INTERRUPT_ID_XRDP_QUEUE_3         _2MAP(INTERRUPT_XRDP_QUEUE_3)
#define INTERRUPT_ID_XRDP_QUEUE_4         _2MAP(INTERRUPT_XRDP_QUEUE_4)
#define INTERRUPT_ID_XRDP_QUEUE_5         _2MAP(INTERRUPT_XRDP_QUEUE_5)
#define INTERRUPT_ID_XRDP_QUEUE_6         _2MAP(INTERRUPT_XRDP_QUEUE_6)
#define INTERRUPT_ID_XRDP_QUEUE_7         _2MAP(INTERRUPT_XRDP_QUEUE_7)
#define INTERRUPT_ID_XRDP_QUEUE_8         _2MAP(INTERRUPT_XRDP_QUEUE_8)
#define INTERRUPT_ID_XRDP_QUEUE_9         _2MAP(INTERRUPT_XRDP_QUEUE_9)
#define INTERRUPT_ID_XRDP_QUEUE_10        _2MAP(INTERRUPT_XRDP_QUEUE_10)
#define INTERRUPT_ID_XRDP_QUEUE_11        _2MAP(INTERRUPT_XRDP_QUEUE_11)
#define INTERRUPT_ID_XRDP_QUEUE_12        _2MAP(INTERRUPT_XRDP_QUEUE_12)
#define INTERRUPT_ID_XRDP_QUEUE_13        _2MAP(INTERRUPT_XRDP_QUEUE_13)
#define INTERRUPT_ID_XRDP_QUEUE_14        _2MAP(INTERRUPT_XRDP_QUEUE_14)
#define INTERRUPT_ID_XRDP_QUEUE_15        _2MAP(INTERRUPT_XRDP_QUEUE_15)
#define INTERRUPT_ID_XRDP_QUEUE_16        _2MAP(INTERRUPT_XRDP_QUEUE_16)
#define INTERRUPT_ID_XRDP_QUEUE_17        _2MAP(INTERRUPT_XRDP_QUEUE_17)
#define INTERRUPT_ID_XRDP_QUEUE_18        _2MAP(INTERRUPT_XRDP_QUEUE_18)
#define INTERRUPT_ID_XRDP_QUEUE_19        _2MAP(INTERRUPT_XRDP_QUEUE_19)
#define INTERRUPT_ID_XRDP_QUEUE_20        _2MAP(INTERRUPT_XRDP_QUEUE_20)
#define INTERRUPT_ID_XRDP_QUEUE_21        _2MAP(INTERRUPT_XRDP_QUEUE_21)
#define INTERRUPT_ID_XRDP_QUEUE_22        _2MAP(INTERRUPT_XRDP_QUEUE_22)
#define INTERRUPT_ID_XRDP_QUEUE_23        _2MAP(INTERRUPT_XRDP_QUEUE_23)
#define INTERRUPT_ID_XRDP_QUEUE_24        _2MAP(INTERRUPT_XRDP_QUEUE_24)
#define INTERRUPT_ID_XRDP_QUEUE_25        _2MAP(INTERRUPT_XRDP_QUEUE_25)
#define INTERRUPT_ID_XRDP_QUEUE_26        _2MAP(INTERRUPT_XRDP_QUEUE_26)
#define INTERRUPT_ID_XRDP_QUEUE_27        _2MAP(INTERRUPT_XRDP_QUEUE_27)
#define INTERRUPT_ID_XRDP_QUEUE_28        _2MAP(INTERRUPT_XRDP_QUEUE_28)
#define INTERRUPT_ID_XRDP_QUEUE_29        _2MAP(INTERRUPT_XRDP_QUEUE_29)
#define INTERRUPT_ID_XRDP_QUEUE_30        _2MAP(INTERRUPT_XRDP_QUEUE_30)
#define INTERRUPT_ID_XRDP_QUEUE_31        _2MAP(INTERRUPT_XRDP_QUEUE_31)
#define INTERRUPT_ID_XRDP_FPM             _2MAP(INTERRUPT_XRDP_FPM)
#endif

#ifdef __BOARD_DRV_AARCH64__
// add here any legacy driver's (driver that have no device tree node) interrupt to be mapped
unsigned int bcm_phys_irqs_to_map[] =
{
    INTERRUPT_PER_EXT_0,
    INTERRUPT_WAN_XGRX,
    INTERRUPT_WAN_XGTX_INTR2,
    INTERRUPT_WAN_XGTX_INTR1,
    INTERRUPT_WAN_XGTX_INTR0,
    INTERRUPT_WAN_GPON_TX,
    INTERRUPT_WAN_GPON_RX,
    INTERRUPT_XRDP_QUEUE_0,
    INTERRUPT_XRDP_QUEUE_1,
    INTERRUPT_XRDP_QUEUE_2,
    INTERRUPT_XRDP_QUEUE_3,
    INTERRUPT_XRDP_QUEUE_4,
    INTERRUPT_XRDP_QUEUE_5,
    INTERRUPT_XRDP_QUEUE_6,
    INTERRUPT_XRDP_QUEUE_7,
    INTERRUPT_XRDP_QUEUE_8,
    INTERRUPT_XRDP_QUEUE_9,
    INTERRUPT_XRDP_QUEUE_10,
    INTERRUPT_XRDP_QUEUE_11,
    INTERRUPT_XRDP_QUEUE_12,
    INTERRUPT_XRDP_QUEUE_13,
    INTERRUPT_XRDP_QUEUE_14,
    INTERRUPT_XRDP_QUEUE_15,
    INTERRUPT_XRDP_QUEUE_16,
    INTERRUPT_XRDP_QUEUE_17,
    INTERRUPT_XRDP_QUEUE_18,
    INTERRUPT_XRDP_QUEUE_19,
    INTERRUPT_XRDP_QUEUE_20,
    INTERRUPT_XRDP_QUEUE_21,
    INTERRUPT_XRDP_QUEUE_22,
    INTERRUPT_XRDP_QUEUE_23,
    INTERRUPT_XRDP_QUEUE_24,
    INTERRUPT_XRDP_QUEUE_25,
    INTERRUPT_XRDP_QUEUE_26,
    INTERRUPT_XRDP_QUEUE_27,
    INTERRUPT_XRDP_QUEUE_28,
    INTERRUPT_XRDP_QUEUE_29,
    INTERRUPT_XRDP_QUEUE_30,
    INTERRUPT_XRDP_QUEUE_31,
    INTERRUPT_XRDP_FPM,
};
unsigned int bcm_legacy_irq_map[256];
#else
extern unsigned int bcm_phys_irqs_to_map[];
extern unsigned int bcm_legacy_irq_map[];
#endif

#ifdef __cplusplus
    }
#endif

#endif  /* __BCM6837_H */
