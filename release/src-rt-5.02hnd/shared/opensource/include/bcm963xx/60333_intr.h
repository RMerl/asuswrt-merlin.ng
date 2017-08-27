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

#ifndef __60333_INTR_H
#define __60333_INTR_H

#ifdef __cplusplus
    extern "C" {
#endif

#define INTERRUPT_ID_SOFTWARE_0           0
#define INTERRUPT_ID_SOFTWARE_1           1

/*=====================================================================*/
/* BCM60333 Timer Interrupt Level Assignments                          */
/*=====================================================================*/
#define MIPS_TIMER_INT                  7

/*=====================================================================*/
/* Peripheral ISR Table Offset                                              */
/*=====================================================================*/
#define INTERNAL_ISR_TABLE_OFFSET           8

/*=====================================================================*/
/* Logical Peripheral Interrupt IDs                                    */
/*=====================================================================*/

/* NOTE
 * Three of these are aliased:
 * INTERRUPT_ID_ENET_CORE0_DMA_RX  -- INTERRUPT_ID_ENETSW_RX_DMA_0
 * INTERRUPT_ID_ENET_CORE1_DMA_RX  -- INTERRUPT_ID_ENETSW_RX_DMA_1
 * INTERRUPT_ID_BRIDGE_DMA_RX      -- INTERRUPT_ID_ENETSW_RX_DMA_2
 *
 * PKTDMA Linux driver uses the naming scheme in the right for all chips.
 * Aliasing the names here prevents Duna-specific changes in the driver code
 */
#define INTERRUPT_ID_TIMER               (INTERNAL_ISR_TABLE_OFFSET + 0)
#define INTERRUPT_ID_PERIPHS_MBOX        (INTERNAL_ISR_TABLE_OFFSET + 1)
#define INTERRUPT_ID_USTAT               (INTERNAL_ISR_TABLE_OFFSET + 2)
#define INTERRUPT_ID_UART                (INTERNAL_ISR_TABLE_OFFSET + 3)
#define INTERRUPT_ID_HS_SPIM             (INTERNAL_ISR_TABLE_OFFSET + 4)
#define INTERRUPT_ID_AVS_ROSC_TH1        (INTERNAL_ISR_TABLE_OFFSET + 5)
#define INTERRUPT_ID_AVS_ROSC_TH2        (INTERNAL_ISR_TABLE_OFFSET + 6)
#define INTERRUPT_ID_AVS_MON_PWD         (INTERNAL_ISR_TABLE_OFFSET + 7)
#define INTERRUPT_ID_AVS_SWMSR_DONE      (INTERNAL_ISR_TABLE_OFFSET + 8)
#define INTERRUPT_ID_EXTERNAL_0          (INTERNAL_ISR_TABLE_OFFSET + 9)
#define INTERRUPT_ID_EXTERNAL_1          (INTERNAL_ISR_TABLE_OFFSET + 10)
#define INTERRUPT_ID_EXTERNAL_2          (INTERNAL_ISR_TABLE_OFFSET + 11)
#define INTERRUPT_ID_EXTERNAL_3          (INTERNAL_ISR_TABLE_OFFSET + 12)
#define INTERRUPT_ID_SYS                 (INTERNAL_ISR_TABLE_OFFSET + 13)
#define INTERRUPT_ID_ENET_CORE0_DMA_TX   (INTERNAL_ISR_TABLE_OFFSET + 14)
#define INTERRUPT_ID_ENET_CORE0_DMA_RX   (INTERNAL_ISR_TABLE_OFFSET + 15)
        /* Alias -- Linux drivers use these names for all RX DMA interrupts */
#define INTERRUPT_ID_ENETSW_RX_DMA_0     (INTERNAL_ISR_TABLE_OFFSET + 15)
#define INTERRUPT_ID_ENET_CORE0_UMAC     (INTERNAL_ISR_TABLE_OFFSET + 16)
#define INTERRUPT_ID_ENET_CORE1_DMA_TX   (INTERNAL_ISR_TABLE_OFFSET + 17)
#define INTERRUPT_ID_ENET_CORE1_DMA_RX   (INTERNAL_ISR_TABLE_OFFSET + 18)
        /* Alias -- Linux drivers use these names for all RX DMA interrupts */
#define INTERRUPT_ID_ENETSW_RX_DMA_1     (INTERNAL_ISR_TABLE_OFFSET + 18)
#define INTERRUPT_ID_ENET_CORE1_UMAC     (INTERNAL_ISR_TABLE_OFFSET + 19)
#define INTERRUPT_ID_EPHY                (INTERNAL_ISR_TABLE_OFFSET + 20)
#define INTERRUPT_ID_BRIDGE_DMA_TX       (INTERNAL_ISR_TABLE_OFFSET + 21)
#define INTERRUPT_ID_BRIDGE_DMA_RX       (INTERNAL_ISR_TABLE_OFFSET + 22)
        /* Alias -- Linux drivers use these names for all RX DMA interrupts */
#define INTERRUPT_ID_ENETSW_RX_DMA_2     (INTERNAL_ISR_TABLE_OFFSET + 22)
#define INTERRUPT_ID_PCIE                (INTERNAL_ISR_TABLE_OFFSET + 23)
#define INTERRUPT_ID_PCIE_RC             INTERRUPT_ID_PCIE
#define INTERRUPT_ID_FP0                 (INTERNAL_ISR_TABLE_OFFSET + 24)
#define INTERRUPT_ID_FP1                 (INTERNAL_ISR_TABLE_OFFSET + 25)
#define INTERRUPT_ID_DIGREG_PMU_STABLE   (INTERNAL_ISR_TABLE_OFFSET + 26)
#define INTERRUPT_ID_DDR                 (INTERNAL_ISR_TABLE_OFFSET + 27)
#define INTERRUPT_ID_PLCPHY              (INTERNAL_ISR_TABLE_OFFSET + 28)

#define INTERRUPT_ID_LAST                INTERRUPT_ID_PLCPHY

/* Dummy constant to make the code compile */
#define INTERRUPT_ID_ENETSW_RX_DMA_3     -1

#ifdef __cplusplus
    }
#endif

#endif  /* __BCM60333_H */

