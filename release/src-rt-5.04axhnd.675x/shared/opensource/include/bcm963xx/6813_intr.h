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

#ifndef __6813_INTR_H
#define __6813_INTR_H

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
/*  ------------- CHIP_IRQS[31-0] ----------------------------*/
#define INTERRUPT_MEMC_SEC_IRQ            (SPI_TABLE_OFFSET + 19)
#define INTERRUPT_DYING_GASP_IRQ          (SPI_TABLE_OFFSET + 28)
#define INTERRUPT_PMC_TEMP_WARN           (SPI_TABLE_OFFSET + 29)

/*  -------------- CHIP_IRQS[63-32] ---------------------------*/
/* FIXME */
#define INTERRUPT_NAND_FLASH_IRQ          (SPI_TABLE_OFFSET + 37)

#define INTERRUPT_PCM                     (SPI_TABLE_OFFSET + 40)
#define INTERRUPT_PCM_DMA0                (SPI_TABLE_OFFSET + 41)
#define INTERRUPT_PCM_DMA1                (SPI_TABLE_OFFSET + 42)

#define INTERRUPT_PER_EXT_0               (SPI_TABLE_OFFSET + 47)
#define INTERRUPT_PER_EXT_1               (SPI_TABLE_OFFSET + 48)
#define INTERRUPT_PER_EXT_2               (SPI_TABLE_OFFSET + 49)
#define INTERRUPT_PER_EXT_3               (SPI_TABLE_OFFSET + 50)
#define INTERRUPT_PER_EXT_4               (SPI_TABLE_OFFSET + 51)
#define INTERRUPT_PER_EXT_5               (SPI_TABLE_OFFSET + 52)
#define INTERRUPT_PER_EXT_6               (SPI_TABLE_OFFSET + 53)
#define INTERRUPT_PER_EXT_7               (SPI_TABLE_OFFSET + 54)  

/*  ------------- CHIP_IRQS[95-64] ----------------------------*/

#define INTERRUPT_MEMC                    (SPI_TABLE_OFFSET + 66)
#define INTERRUPT_MPM                     (SPI_TABLE_OFFSET + 67)
#define INTERRUPT_PCIE_0_CPU_INTR         (SPI_TABLE_OFFSET + 68)
#define INTERRUPT_PCIE_1_CPU_INTR         (SPI_TABLE_OFFSET + 69)
#define INTERRUPT_PCIE_2_CPU_INTR         (SPI_TABLE_OFFSET + 70)
#define INTERRUPT_PCIE_3_CPU_INTR         (SPI_TABLE_OFFSET + 71)

/* TODO --- FIXME */
/*  ------------- CHIP_IRQS[159-128] ----------------------------*/
#define INTERRUPT_XRDP_QUEUE_0            (SPI_TABLE_OFFSET + 75)
#define INTERRUPT_XRDP_QUEUE_1            (SPI_TABLE_OFFSET + 76)
#define INTERRUPT_XRDP_QUEUE_2            (SPI_TABLE_OFFSET + 77)
#define INTERRUPT_XRDP_QUEUE_3            (SPI_TABLE_OFFSET + 78)
#define INTERRUPT_XRDP_QUEUE_4            (SPI_TABLE_OFFSET + 79)
#define INTERRUPT_XRDP_QUEUE_5            (SPI_TABLE_OFFSET + 80)
#define INTERRUPT_XRDP_QUEUE_6            (SPI_TABLE_OFFSET + 81)
#define INTERRUPT_XRDP_QUEUE_7            (SPI_TABLE_OFFSET + 82)
#define INTERRUPT_XRDP_QUEUE_8            (SPI_TABLE_OFFSET + 83)
#define INTERRUPT_XRDP_QUEUE_9            (SPI_TABLE_OFFSET + 84)
#define INTERRUPT_XRDP_QUEUE_10           (SPI_TABLE_OFFSET + 85)
#define INTERRUPT_XRDP_QUEUE_11           (SPI_TABLE_OFFSET + 86)
#define INTERRUPT_XRDP_QUEUE_12           (SPI_TABLE_OFFSET + 87)
#define INTERRUPT_XRDP_QUEUE_13           (SPI_TABLE_OFFSET + 88)
#define INTERRUPT_XRDP_QUEUE_14           (SPI_TABLE_OFFSET + 89)
#define INTERRUPT_XRDP_QUEUE_15           (SPI_TABLE_OFFSET + 90)
#define INTERRUPT_XRDP_QUEUE_16           (SPI_TABLE_OFFSET + 91)
#define INTERRUPT_XRDP_QUEUE_17           (SPI_TABLE_OFFSET + 92)
#define INTERRUPT_XRDP_QUEUE_18           (SPI_TABLE_OFFSET + 93)
#define INTERRUPT_XRDP_QUEUE_19           (SPI_TABLE_OFFSET + 94)
#define INTERRUPT_XRDP_QUEUE_20           (SPI_TABLE_OFFSET + 95)
#define INTERRUPT_XRDP_QUEUE_21           (SPI_TABLE_OFFSET + 96)
#define INTERRUPT_XRDP_QUEUE_22           (SPI_TABLE_OFFSET + 97)
#define INTERRUPT_XRDP_QUEUE_23           (SPI_TABLE_OFFSET + 98)
#define INTERRUPT_XRDP_QUEUE_24           (SPI_TABLE_OFFSET + 99)
#define INTERRUPT_XRDP_QUEUE_25           (SPI_TABLE_OFFSET + 100)
#define INTERRUPT_XRDP_QUEUE_26           (SPI_TABLE_OFFSET + 101)
#define INTERRUPT_XRDP_QUEUE_27           (SPI_TABLE_OFFSET + 102)
#define INTERRUPT_XRDP_QUEUE_28           (SPI_TABLE_OFFSET + 103)
#define INTERRUPT_XRDP_QUEUE_29           (SPI_TABLE_OFFSET + 104)
#define INTERRUPT_XRDP_QUEUE_30           (SPI_TABLE_OFFSET + 105)
#define INTERRUPT_XRDP_QUEUE_31           (SPI_TABLE_OFFSET + 106)
#define INTERRUPT_XRDP_FPM                (SPI_TABLE_OFFSET + 107)

/*  ------------- CHIP_IRQS[191-160] ----------------------------*/

#ifndef __ASSEMBLER__

/*========================================================================*/
/* Linux(Virtual) Interrupt IDs for Legacy drivers                        */ 
/* -Legacy Drivers with no DT support should retrieve VIRQ ids from these */
/*  defines                                                               */ 
/* -If a legacy driver requires a VIRQ, its corresponding physical irq id */
/*  must be placed in bcm_phys_irqs_to_map array below                    */
/*========================================================================*/
#define INTERRUPT_ID_RANGE_CHECK          (bcm_legacy_irq_map[INTERRUPT_MEMC_SEC_IRQ - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_DG                   (bcm_legacy_irq_map[INTERRUPT_DYING_GASP_IRQ - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_PMC_TEMP_WARN        (bcm_legacy_irq_map[INTERRUPT_PMC_TEMP_WARN - SPI_TABLE_OFFSET])

/* FIXME */
#define INTERRUPT_ID_NAND_FLASH           (bcm_legacy_irq_map[INTERRUPT_NAND_FLASH_IRQ - SPI_TABLE_OFFSET])

#define INTERRUPT_ID_FPM                  (bcm_legacy_irq_map[INTERRUPT_XRDP_FPM - SPI_TABLE_OFFSET]) 

#define INTERRUPT_ID_EXTERNAL_0           (bcm_legacy_irq_map[INTERRUPT_PER_EXT_0 - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_EXTERNAL_1           (bcm_legacy_irq_map[INTERRUPT_PER_EXT_1 - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_EXTERNAL_2           (bcm_legacy_irq_map[INTERRUPT_PER_EXT_2 - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_EXTERNAL_3           (bcm_legacy_irq_map[INTERRUPT_PER_EXT_3 - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_EXTERNAL_4           (bcm_legacy_irq_map[INTERRUPT_PER_EXT_4 - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_EXTERNAL_5           (bcm_legacy_irq_map[INTERRUPT_PER_EXT_5 - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_EXTERNAL_6           (bcm_legacy_irq_map[INTERRUPT_PER_EXT_6 - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_EXTERNAL_7           (bcm_legacy_irq_map[INTERRUPT_PER_EXT_7 - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_EXTERNAL_MAX         INTERRUPT_ID_EXTERNAL_7

#define INTERRUPT_ID_XRDP_QUEUE_0            (bcm_legacy_irq_map[INTERRUPT_XRDP_QUEUE_0 - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_XRDP_QUEUE_1            (bcm_legacy_irq_map[INTERRUPT_XRDP_QUEUE_1 - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_XRDP_QUEUE_2            (bcm_legacy_irq_map[INTERRUPT_XRDP_QUEUE_2 - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_XRDP_QUEUE_3            (bcm_legacy_irq_map[INTERRUPT_XRDP_QUEUE_3 - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_XRDP_QUEUE_4            (bcm_legacy_irq_map[INTERRUPT_XRDP_QUEUE_4 - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_XRDP_QUEUE_5            (bcm_legacy_irq_map[INTERRUPT_XRDP_QUEUE_5 - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_XRDP_QUEUE_6            (bcm_legacy_irq_map[INTERRUPT_XRDP_QUEUE_6 - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_XRDP_QUEUE_7            (bcm_legacy_irq_map[INTERRUPT_XRDP_QUEUE_7 - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_XRDP_QUEUE_8            (bcm_legacy_irq_map[INTERRUPT_XRDP_QUEUE_8 - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_XRDP_QUEUE_9            (bcm_legacy_irq_map[INTERRUPT_XRDP_QUEUE_9- SPI_TABLE_OFFSET])
#define INTERRUPT_ID_XRDP_QUEUE_10           (bcm_legacy_irq_map[INTERRUPT_XRDP_QUEUE_10 - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_XRDP_QUEUE_11           (bcm_legacy_irq_map[INTERRUPT_XRDP_QUEUE_11 - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_XRDP_QUEUE_12           (bcm_legacy_irq_map[INTERRUPT_XRDP_QUEUE_12 - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_XRDP_QUEUE_13           (bcm_legacy_irq_map[INTERRUPT_XRDP_QUEUE_13 - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_XRDP_QUEUE_14           (bcm_legacy_irq_map[INTERRUPT_XRDP_QUEUE_14 - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_XRDP_QUEUE_15           (bcm_legacy_irq_map[INTERRUPT_XRDP_QUEUE_15 - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_XRDP_QUEUE_16           (bcm_legacy_irq_map[INTERRUPT_XRDP_QUEUE_16 - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_XRDP_QUEUE_17           (bcm_legacy_irq_map[INTERRUPT_XRDP_QUEUE_17 - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_XRDP_QUEUE_18           (bcm_legacy_irq_map[INTERRUPT_XRDP_QUEUE_18 - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_XRDP_QUEUE_19           (bcm_legacy_irq_map[INTERRUPT_XRDP_QUEUE_19 - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_XRDP_QUEUE_20           (bcm_legacy_irq_map[INTERRUPT_XRDP_QUEUE_20 - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_XRDP_QUEUE_21           (bcm_legacy_irq_map[INTERRUPT_XRDP_QUEUE_21 - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_XRDP_QUEUE_22           (bcm_legacy_irq_map[INTERRUPT_XRDP_QUEUE_22 - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_XRDP_QUEUE_23           (bcm_legacy_irq_map[INTERRUPT_XRDP_QUEUE_23 - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_XRDP_QUEUE_24           (bcm_legacy_irq_map[INTERRUPT_XRDP_QUEUE_24 - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_XRDP_QUEUE_25           (bcm_legacy_irq_map[INTERRUPT_XRDP_QUEUE_25 - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_XRDP_QUEUE_26           (bcm_legacy_irq_map[INTERRUPT_XRDP_QUEUE_26 - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_XRDP_QUEUE_27           (bcm_legacy_irq_map[INTERRUPT_XRDP_QUEUE_27 - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_XRDP_QUEUE_28           (bcm_legacy_irq_map[INTERRUPT_XRDP_QUEUE_28 - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_XRDP_QUEUE_39           (bcm_legacy_irq_map[INTERRUPT_XRDP_QUEUE_29 - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_XRDP_QUEUE_30           (bcm_legacy_irq_map[INTERRUPT_XRDP_QUEUE_30 - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_XRDP_QUEUE_31           (bcm_legacy_irq_map[INTERRUPT_XRDP_QUEUE_31 - SPI_TABLE_OFFSET])
#define INTERRUPT_PCM_DMA_IRQ                (bcm_legacy_irq_map[INTERRUPT_PCM_DMA0 - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_MPM                     (bcm_legacy_irq_map[INTERRUPT_MPM - SPI_TABLE_OFFSET])

#ifdef __BOARD_DRV_AARCH64__
// add here any legacy driver's (driver that have no device tree node) interrupt to be mapped
unsigned int bcm_phys_irqs_to_map[] =
{
    INTERRUPT_MEMC_SEC_IRQ,
    INTERRUPT_DYING_GASP_IRQ, 
    INTERRUPT_PMC_TEMP_WARN,
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
    INTERRUPT_PCM_DMA0,
    INTERRUPT_PCM_DMA1,
    INTERRUPT_MPM,
    INTERRUPT_NAND_FLASH_IRQ,	/* FIXME */
};
unsigned int bcm_legacy_irq_map[256];
#else
extern unsigned int bcm_phys_irqs_to_map[];
extern unsigned int bcm_legacy_irq_map[];
#endif

#endif

#define NUM_EXT_INT    (INTERRUPT_PER_EXT_7-INTERRUPT_PER_EXT_0+1)

#ifdef __cplusplus
}
#endif

#endif /* __6813_INTR_H */
