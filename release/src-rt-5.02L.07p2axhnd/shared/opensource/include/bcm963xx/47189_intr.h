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

#ifndef __47189_INTR_H
#define __47189_INTR_H

#ifdef __cplusplus
extern "C" {
#endif

#define SPI_TABLE_OFFSET             32
#define INTERRUPT_ID_EXT_0_INTR		    (SPI_TABLE_OFFSET)
#define INTERRUPT_D11_0_CPU_INTR	    (SPI_TABLE_OFFSET + 1)
#define INTERRUPT_PCIE_GEN2_CPU_INTR	    (SPI_TABLE_OFFSET + 2)
#define INTERRUPT_USB_EHCI              (SPI_TABLE_OFFSET + 4)
#define INTERRUPT_USB_OHCI              (SPI_TABLE_OFFSET + 4)
#define INTERRUPT_ENETCORE0_INTR        (SPI_TABLE_OFFSET + 5)
#define INTERRUPT_ENETCORE1_INTR        (SPI_TABLE_OFFSET + 6)
#define INTERRUPT_D11_1_CPU_INTR	    (SPI_TABLE_OFFSET + 7)

#ifndef __ASSEMBLER__
/*========================================================================*/
/* Linux(Virtual) Interrupt IDs for Legacy drivers                        */ 
/* -Legacy Drivers with no DT support should retrieve VIRQ ids from these */
/*  defines                                                               */ 
/* -If a legacy driver requires a VIRQ, its corresponding physical irq id */
/*  must be placed in bcm_phys_irqs_to_map array below                    */
/*========================================================================*/
#define INTERRUPT_ID_EXTERNAL_0	    (bcm_legacy_irq_map[INTERRUPT_ID_EXT_0_INTR - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_D11_0	    (bcm_legacy_irq_map[INTERRUPT_D11_0_CPU_INTR - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_PCIE_GEN2	    (bcm_legacy_irq_map[INTERRUPT_PCIE_GEN2_CPU_INTR - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_USB_EHCI       (bcm_legacy_irq_map[INTERRUPT_USB_EHCI - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_USB_OHCI       (bcm_legacy_irq_map[INTERRUPT_USB_OHCI - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_ENETCORE0	    (bcm_legacy_irq_map[INTERRUPT_ENETCORE0_INTR - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_ENETCORE1	    (bcm_legacy_irq_map[INTERRUPT_ENETCORE1_INTR - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_D11_1	    (bcm_legacy_irq_map[INTERRUPT_D11_1_CPU_INTR - SPI_TABLE_OFFSET])

/* Dummy definitions */
#define INTERRUPT_ID_ENETSW_RX_DMA_0	INTERRUPT_ID_ENETCORE0
#define INTERRUPT_ID_ENETSW_RX_DMA_1	INTERRUPT_ID_ENETCORE1
#define INTERRUPT_ID_ENETSW_RX_DMA_2	0
#define INTERRUPT_ID_ENETSW_RX_DMA_3	0
#define INTERRUPT_ID_EXTERNAL_MAX       INTERRUPT_ID_EXTERNAL_0
#ifdef __BOARD_DRV_ARMV7__
// add here any legacy driver's (driver that have no device tree node) interrupt to be mapped
unsigned int bcm_phys_irqs_to_map[] =
{
    INTERRUPT_ID_EXT_0_INTR,
    INTERRUPT_D11_0_CPU_INTR,
    INTERRUPT_PCIE_GEN2_CPU_INTR,
    INTERRUPT_USB_EHCI,
    INTERRUPT_USB_OHCI,
	INTERRUPT_ENETCORE0_INTR,
	INTERRUPT_ENETCORE1_INTR,
    INTERRUPT_D11_1_CPU_INTR
};
unsigned int bcm_legacy_irq_map[256];
#else
extern unsigned int bcm_phys_irqs_to_map[];
extern unsigned int bcm_legacy_irq_map[];
#endif

#endif

#define NUM_EXT_INT    1

#ifdef __cplusplus
}
#endif

#endif /* __BCM47189_H */
