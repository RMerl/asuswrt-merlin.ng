/*
<:copyright-BRCM:2013:DUAL/GPL:standard

   Copyright (c) 2013 Broadcom 
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

#ifndef __63138_INTR_H
#define __63138_INTR_H

#ifdef __cplusplus
extern "C" {
#endif

/*=====================================================================*/
/* Peripheral SPI Table Offset                                         */
/*=====================================================================*/
#define SPI_TABLE_OFFSET		32	// FIXME?
#define SPI_TABLE2_OFFSET		SPI_TABLE_OFFSET + 32
#define SPI_TABLE3_OFFSET		SPI_TABLE2_OFFSET + 32
#define SPI_TABLE4_OFFSET		SPI_TABLE3_OFFSET + 32

/*=====================================================================*/
/* Physical Interrupt IDs                                              */
/*=====================================================================*/
#define INTERRUPT_L2CC			(SPI_TABLE_OFFSET + 0)
#define INTERRUPT_PWRWDOG		(SPI_TABLE_OFFSET + 1)
#define INTERRUPT_TRAPAXI0		(SPI_TABLE_OFFSET + 2)
#define INTERRUPT_TRAPAXI1		(SPI_TABLE_OFFSET + 3)
#define INTERRUPT_COMMTX		(SPI_TABLE_OFFSET + 4)
#define INTERRUPT_COMMRX		(SPI_TABLE_OFFSET + 5)
#define INTERRUPT_PMU			(SPI_TABLE_OFFSET + 6)
#define INTERRUPT_CTI			(SPI_TABLE_OFFSET + 7)
#define INTERRUPT_DEFFLG0		(SPI_TABLE_OFFSET + 8)
#define INTERRUPT_DEFFLG1		(SPI_TABLE_OFFSET + 9)
#define INTERRUPT_PARITYFAIL_CPU0	(SPI_TABLE_OFFSET + 10)
#define INTERRUPT_PARITYFAIL_CPU1	(SPI_TABLE_OFFSET + 11)
#define INTERRUPT_PARITYFAIL_SCU0	(SPI_TABLE_OFFSET + 12)
#define INTERRUPT_PARITYFAIL_SCU1	(SPI_TABLE_OFFSET + 13)
#define INTERRUPT_ARM_TIMER		(SPI_TABLE_OFFSET + 15)
#define INTERRUPT_WDTIMER		(SPI_TABLE_OFFSET + 16)
#define INTERRUPT_AES			(SPI_TABLE_OFFSET + 17)
#define INTERRUPT_DDRSEC		(SPI_TABLE_OFFSET + 18)
#define INTERRUPT_AIPSEC		(SPI_TABLE_OFFSET + 19)
#define INTERRUPT_PERIPHSEC		(SPI_TABLE_OFFSET + 20)
#define INTERRUPT_PMCSEC		(SPI_TABLE_OFFSET + 21)
#define INTERRUPT_UBUSERR		(SPI_TABLE_OFFSET + 22)
#define INTERRUPT_MBOX2			(SPI_TABLE_OFFSET + 23)
#define INTERRUPT_MBOX3			(SPI_TABLE_OFFSET + 24)
#define INTERRUPT_DG     		(SPI_TABLE_OFFSET + 29)
#define INTERRUPT_PMC0			(SPI_TABLE_OFFSET + 30)
#define INTERRUPT_PMC1			(SPI_TABLE_OFFSET + 31)

#define INTERRUPT_UART0			(SPI_TABLE2_OFFSET + 0)
#define INTERRUPT_UART1			(SPI_TABLE2_OFFSET + 1)
#define INTERRUPT_HS_UART		(SPI_TABLE2_OFFSET + 2)
#define INTERRUPT_AIPETB		(SPI_TABLE2_OFFSET + 3)
#define INTERRUPT_UBUS2ER		(SPI_TABLE2_OFFSET + 4)
#define INTERRUPT_HS_SPIM		(SPI_TABLE2_OFFSET + 5)
#define INTERRUPT_NAND_FLASH		(SPI_TABLE2_OFFSET + 6)
#define INTERRUPT_DDRC			(SPI_TABLE2_OFFSET + 7)
#define INTERRUPT_VDSL			(SPI_TABLE2_OFFSET + 8)
#define INTERRUPT_SARC			(SPI_TABLE2_OFFSET + 9)
#define INTERRUPT_USBDC			(SPI_TABLE2_OFFSET + 10)
#define INTERRUPT_PCMC			(SPI_TABLE2_OFFSET + 11)
#define INTERRUPT_SATAERR		(SPI_TABLE2_OFFSET + 12)
#define INTERRUPT_SATAC			(SPI_TABLE2_OFFSET + 13)
#define INTERRUPT_RUNNER_0		(SPI_TABLE2_OFFSET + 14)
#define INTERRUPT_RUNNER_1		(SPI_TABLE2_OFFSET + 15)
#define INTERRUPT_RUNNER_2		(SPI_TABLE2_OFFSET + 16)
#define INTERRUPT_RUNNER_3		(SPI_TABLE2_OFFSET + 17)
#define INTERRUPT_RUNNER_4		(SPI_TABLE2_OFFSET + 18)
#define INTERRUPT_RUNNER_5		(SPI_TABLE2_OFFSET + 19)
#define INTERRUPT_RUNNER_6		(SPI_TABLE2_OFFSET + 20)
#define INTERRUPT_RUNNER_7		(SPI_TABLE2_OFFSET + 21)
#define INTERRUPT_RUNNER_8		(SPI_TABLE2_OFFSET + 22)
#define INTERRUPT_RUNNER_9		(SPI_TABLE2_OFFSET + 23)
#define INTERRUPT_RDP_SBPM		(SPI_TABLE2_OFFSET + 24)
#define INTERRUPT_RDP_BPM		(SPI_TABLE2_OFFSET + 25)
#define INTERRUPT_SF2_0			(SPI_TABLE2_OFFSET + 26)
#define INTERRUPT_SF2_1			(SPI_TABLE2_OFFSET + 27)
#define INTERRUPT_PCIE0			(SPI_TABLE2_OFFSET + 28)
#define INTERRUPT_PCIE1			(SPI_TABLE2_OFFSET + 29)
#define INTERRUPT_DECT_0		(SPI_TABLE2_OFFSET + 30)
#define INTERRUPT_DECT_1		(SPI_TABLE2_OFFSET + 31)

#define INTERRUPT_PER_MBOX0		(SPI_TABLE3_OFFSET + 4)
#define INTERRUPT_PER_MBOX1		(SPI_TABLE3_OFFSET + 5)
#define INTERRUPT_PER_MBOX2		(SPI_TABLE3_OFFSET + 6)
#define INTERRUPT_PER_MBOX3		(SPI_TABLE3_OFFSET + 7)
#define INTERRUPT_USB_OHCI		(SPI_TABLE3_OFFSET + 8)
#define INTERRUPT_USB_EHCI		(SPI_TABLE3_OFFSET + 9)
#define INTERRUPT_USB_XHCI		(SPI_TABLE3_OFFSET + 10)
#define INTERRUPT_USB_HBR		(SPI_TABLE3_OFFSET + 11)
#define INTERRUPT_USB_HEV		(SPI_TABLE3_OFFSET + 12)
#define INTERRUPT_EXTERNAL_0		(SPI_TABLE3_OFFSET + 13)
#define INTERRUPT_EXTERNAL_1		(SPI_TABLE3_OFFSET + 14)
#define INTERRUPT_EXTERNAL_2		(SPI_TABLE3_OFFSET + 15)
#define INTERRUPT_EXTERNAL_3		(SPI_TABLE3_OFFSET + 16)
#define INTERRUPT_EXTERNAL_4		(SPI_TABLE3_OFFSET + 17)
#define INTERRUPT_EXTERNAL_5		(SPI_TABLE3_OFFSET + 18)
#define MAP_EXT_IRQ_TO_GPIO(n)		((n)+32)
#define INTERRUPT_I2C			(SPI_TABLE3_OFFSET + 19)
#define INTERRUPT_I2S			(SPI_TABLE3_OFFSET + 20)
#define INTERRUPT_RNG			(SPI_TABLE3_OFFSET + 21)
#define INTERRUPT_EMMC			(SPI_TABLE3_OFFSET + 22)
#define INTERRUPT_PL081			(SPI_TABLE3_OFFSET + 23)

#define INTERRUPT_SAR_0			(SPI_TABLE4_OFFSET + 0)
#define INTERRUPT_SAR_1			(SPI_TABLE4_OFFSET + 1)
#define INTERRUPT_SAR_2			(SPI_TABLE4_OFFSET + 2)
#define INTERRUPT_SAR_3			(SPI_TABLE4_OFFSET + 3)
#define INTERRUPT_SAR_4			(SPI_TABLE4_OFFSET + 4)
#define INTERRUPT_SAR_5			(SPI_TABLE4_OFFSET + 5)
#define INTERRUPT_SAR_6			(SPI_TABLE4_OFFSET + 6)
#define INTERRUPT_SAR_7			(SPI_TABLE4_OFFSET + 7)
#define INTERRUPT_SAR_8			(SPI_TABLE4_OFFSET + 8)
#define INTERRUPT_SAR_9			(SPI_TABLE4_OFFSET + 9)
#define INTERRUPT_SAR_10		(SPI_TABLE4_OFFSET + 10)
#define INTERRUPT_SAR_11		(SPI_TABLE4_OFFSET + 11)
#define INTERRUPT_SAR_12		(SPI_TABLE4_OFFSET + 12)
#define INTERRUPT_SAR_13		(SPI_TABLE4_OFFSET + 13)
#define INTERRUPT_SAR_14		(SPI_TABLE4_OFFSET + 14)
#define INTERRUPT_SAR_15		(SPI_TABLE4_OFFSET + 15)
#define INTERRUPT_SAR_16		(SPI_TABLE4_OFFSET + 16)
#define INTERRUPT_SAR_17		(SPI_TABLE4_OFFSET + 17)
#define INTERRUPT_SAR_18		(SPI_TABLE4_OFFSET + 18)
#define INTERRUPT_SAR_19		(SPI_TABLE4_OFFSET + 19)

#define INTERRUPT_PCM_0			(SPI_TABLE4_OFFSET + 20)
#define INTERRUPT_PCM_1			(SPI_TABLE4_OFFSET + 21)
#define INTERRUPT_USBD_0		(SPI_TABLE4_OFFSET + 22)
#define INTERRUPT_USBD_1		(SPI_TABLE4_OFFSET + 23)
#define INTERRUPT_USBD_2		(SPI_TABLE4_OFFSET + 24)
#define INTERRUPT_USBD_3		(SPI_TABLE4_OFFSET + 25)
#define INTERRUPT_USBD_4		(SPI_TABLE4_OFFSET + 26)
#define INTERRUPT_USBD_5		(SPI_TABLE4_OFFSET + 27)


/*=====================================================================*/
/* Logical Peripheral Interrupt IDs                                    */
/*=====================================================================*/
#ifndef __ASSEMBLER__
#define _2MAP(V) (bcm_legacy_irq_map[(V - SPI_TABLE_OFFSET)])

#define INTERRUPT_ID_ARM_TIMER		_2MAP(INTERRUPT_ARM_TIMER)
#define INTERRUPT_ID_WDTIMER		_2MAP(INTERRUPT_WDTIMER)
#define INTERRUPT_ID_DDRSEC		_2MAP(INTERRUPT_DDRSEC)
#define INTERRUPT_ID_RANGE_CHECK	INTERRUPT_ID_DDRSEC
#define INTERRUPT_ID_UBUSERR		_2MAP(INTERRUPT_UBUSERR)
#define INTERRUPT_ID_DG     		_2MAP(INTERRUPT_DG)
#define INTERRUPT_ID_PMC0		_2MAP(INTERRUPT_PMC0)
#define INTERRUPT_ID_PMC1		_2MAP(INTERRUPT_PMC1)
#define INTERRUPT_ID_UART0		_2MAP(INTERRUPT_UART0)
#define INTERRUPT_ID_UART		INTERRUPT_ID_UART0
#define INTERRUPT_ID_UART1		_2MAP(INTERRUPT_UART1)
#define INTERRUPT_ID_HS_UART		_2MAP(INTERRUPT_HS_UART)
#define INTERRUPT_ID_AIPETB		_2MAP(INTERRUPT_AIPETB)
#define INTERRUPT_ID_UBUS2ER		_2MAP(INTERRUPT_UBUS2ER)
#define INTERRUPT_ID_HS_SPIM		_2MAP(INTERRUPT_HS_SPIM)
#define INTERRUPT_ID_NAND_FLASH		_2MAP(INTERRUPT_NAND_FLASH)
#define INTERRUPT_ID_DDRC		_2MAP(INTERRUPT_DDRC)
#define INTERRUPT_ID_SARC		_2MAP(INTERRUPT_SARC)
#define INTERRUPT_ID_USBDC		_2MAP(INTERRUPT_USBDC)
#define INTERRUPT_ID_PCMC		_2MAP(INTERRUPT_PCMC)
#define INTERRUPT_ID_RUNNER_0		_2MAP(INTERRUPT_RUNNER_0)
#define INTERRUPT_ID_RUNNER_1		_2MAP(INTERRUPT_RUNNER_1)
#define INTERRUPT_ID_RUNNER_2		_2MAP(INTERRUPT_RUNNER_2)
#define INTERRUPT_ID_RUNNER_3		_2MAP(INTERRUPT_RUNNER_3)
#define INTERRUPT_ID_RUNNER_4		_2MAP(INTERRUPT_RUNNER_4)
#define INTERRUPT_ID_RUNNER_5		_2MAP(INTERRUPT_RUNNER_5)
#define INTERRUPT_ID_RUNNER_6		_2MAP(INTERRUPT_RUNNER_6)
#define INTERRUPT_ID_RUNNER_7		_2MAP(INTERRUPT_RUNNER_7)
#define INTERRUPT_ID_RUNNER_8		_2MAP(INTERRUPT_RUNNER_8)
#define INTERRUPT_ID_RUNNER_9		_2MAP(INTERRUPT_RUNNER_9)
#define INTERRUPT_ID_RDP_SBPM		_2MAP(INTERRUPT_RDP_SBPM)
#define INTERRUPT_ID_RDP_BPM		_2MAP(INTERRUPT_RDP_BPM)
#define INTERRUPT_ID_SF2_0		_2MAP(INTERRUPT_SF2_0)
#define INTERRUPT_ID_SF2_1		_2MAP(INTERRUPT_SF2_1)
#define INTERRUPT_ID_PCIE0		_2MAP(INTERRUPT_PCIE0)
#define INTERRUPT_ID_PCIE1		_2MAP(INTERRUPT_PCIE1)
#define INTERRUPT_ID_DECT_0		_2MAP(INTERRUPT_DECT_0)
#define INTERRUPT_ID_DECT_1		_2MAP(INTERRUPT_DECT_1)
#define INTERRUPT_ID_SAR		INTERRUPT_ID_SARC
#define INTERRUPT_ID_USB_OHCI		_2MAP(INTERRUPT_USB_OHCI)
#define INTERRUPT_ID_USB_EHCI		_2MAP(INTERRUPT_USB_EHCI)
#define INTERRUPT_ID_USB_XHCI		_2MAP(INTERRUPT_USB_XHCI)
#define INTERRUPT_ID_USB_HBR		_2MAP(INTERRUPT_USB_HBR)
#define INTERRUPT_ID_USB_HEV		_2MAP(INTERRUPT_USB_HEV)
#define INTERRUPT_ID_EXTERNAL_0		_2MAP(INTERRUPT_EXTERNAL_0)
#define INTERRUPT_ID_EXTERNAL_1		_2MAP(INTERRUPT_EXTERNAL_1)
#define INTERRUPT_ID_EXTERNAL_2		_2MAP(INTERRUPT_EXTERNAL_2)
#define INTERRUPT_ID_EXTERNAL_3		_2MAP(INTERRUPT_EXTERNAL_3)
#define INTERRUPT_ID_EXTERNAL_4		_2MAP(INTERRUPT_EXTERNAL_4)
#define INTERRUPT_ID_EXTERNAL_5		_2MAP(INTERRUPT_EXTERNAL_5)
#define INTERRUPT_ID_EXTERNAL_MAX	INTERRUPT_ID_EXTERNAL_5
#define INTERRUPT_ID_I2C		_2MAP(INTERRUPT_I2C)
#define INTERRUPT_ID_I2S		_2MAP(INTERRUPT_I2S)
#define INTERRUPT_ID_RNG		_2MAP(INTERRUPT_RNG)
#define INTERRUPT_ID_EMMC		_2MAP(INTERRUPT_EMMC)
#define INTERRUPT_ID_PL081		_2MAP(INTERRUPT_PL081)
#define INTERRUPT_ID_SAR_0		_2MAP(INTERRUPT_SAR_0)
#define INTERRUPT_ID_ATM_DMA_0          INTERRUPT_ID_SAR_0
#define INTERRUPT_ID_SAR_1		_2MAP(INTERRUPT_SAR_1)
#define INTERRUPT_ID_ATM_DMA_1          INTERRUPT_ID_SAR_1
#define INTERRUPT_ID_SAR_2		_2MAP(INTERRUPT_SAR_2)
#define INTERRUPT_ID_ATM_DMA_2          INTERRUPT_ID_SAR_2
#define INTERRUPT_ID_SAR_3		_2MAP(INTERRUPT_SAR_3)
#define INTERRUPT_ID_ATM_DMA_3          INTERRUPT_ID_SAR_3
#define INTERRUPT_ID_SAR_4		_2MAP(INTERRUPT_SAR_4)
#define INTERRUPT_ID_ATM_DMA_4          INTERRUPT_ID_SAR_4
#define INTERRUPT_ID_SAR_5		_2MAP(INTERRUPT_SAR_5)
#define INTERRUPT_ID_ATM_DMA_5          INTERRUPT_ID_SAR_5
#define INTERRUPT_ID_SAR_6		_2MAP(INTERRUPT_SAR_6)
#define INTERRUPT_ID_ATM_DMA_6          INTERRUPT_ID_SAR_6
#define INTERRUPT_ID_SAR_7		_2MAP(INTERRUPT_SAR_7)
#define INTERRUPT_ID_ATM_DMA_7          INTERRUPT_ID_SAR_7
#define INTERRUPT_ID_SAR_8		_2MAP(INTERRUPT_SAR_8)
#define INTERRUPT_ID_ATM_DMA_8          INTERRUPT_ID_SAR_8
#define INTERRUPT_ID_SAR_9		_2MAP(INTERRUPT_SAR_9)
#define INTERRUPT_ID_ATM_DMA_9          INTERRUPT_ID_SAR_9
#define INTERRUPT_ID_SAR_10		_2MAP(INTERRUPT_SAR_10)
#define INTERRUPT_ID_ATM_DMA_10          INTERRUPT_ID_SAR_10
#define INTERRUPT_ID_SAR_11		_2MAP(INTERRUPT_SAR_11)
#define INTERRUPT_ID_ATM_DMA_11          INTERRUPT_ID_SAR_11
#define INTERRUPT_ID_SAR_12		_2MAP(INTERRUPT_SAR_12)
#define INTERRUPT_ID_ATM_DMA_12          INTERRUPT_ID_SAR_12
#define INTERRUPT_ID_SAR_13		_2MAP(INTERRUPT_SAR_13)
#define INTERRUPT_ID_ATM_DMA_13          INTERRUPT_ID_SAR_13
#define INTERRUPT_ID_SAR_14		_2MAP(INTERRUPT_SAR_14)
#define INTERRUPT_ID_ATM_DMA_14          INTERRUPT_ID_SAR_14
#define INTERRUPT_ID_SAR_15		_2MAP(INTERRUPT_SAR_15)
#define INTERRUPT_ID_ATM_DMA_15          INTERRUPT_ID_SAR_15
#define INTERRUPT_ID_SAR_16		_2MAP(INTERRUPT_SAR_16)
#define INTERRUPT_ID_ATM_DMA_16          INTERRUPT_ID_SAR_16
#define INTERRUPT_ID_SAR_17		_2MAP(INTERRUPT_SAR_17)
#define INTERRUPT_ID_ATM_DMA_17          INTERRUPT_ID_SAR_17
#define INTERRUPT_ID_SAR_18		_2MAP(INTERRUPT_SAR_18)
#define INTERRUPT_ID_ATM_DMA_18          INTERRUPT_ID_SAR_18
#define INTERRUPT_ID_SAR_19		_2MAP(INTERRUPT_SAR_19)
#define INTERRUPT_ID_ATM_DMA_19          INTERRUPT_ID_SAR_19
#define INTERRUPT_ID_PCM_0		_2MAP(INTERRUPT_PCM_0)
#define INTERRUPT_ID_PCM_1		_2MAP(INTERRUPT_PCM_1)
#define INTERRUPT_ID_USBD_0		_2MAP(INTERRUPT_USBD_0)
#define INTERRUPT_ID_USBD_1		_2MAP(INTERRUPT_USBD_1)
#define INTERRUPT_ID_USBD_2		_2MAP(INTERRUPT_USBD_2)
#define INTERRUPT_ID_USBD_3		_2MAP(INTERRUPT_USBD_3)
#define INTERRUPT_ID_USBD_4		_2MAP(INTERRUPT_USBD_4)
#define INTERRUPT_ID_USBD_5		_2MAP(INTERRUPT_USBD_5)	

/* Last Physical Interrupt ID */
#define INTERRUPT_ID_LAST_PHYS		INTERRUPT_ID_USBD_5

/* Virtual interrupts */
#define	VIRTUAL_INTR_TABLE_OFFSET	(INTERRUPT_ID_LAST_PHYS + 1)

/* PCIE MSI virtual interrupts */
#define	PCIE_MSI_IDS_PER_DOMAIN		8
#define	INTERRUPT_ID_PCIE_MSI_FIRST	(VIRTUAL_INTR_TABLE_OFFSET + 0)
#define	INTERRUPT_ID_PCIE0_MSI_FIRST	INTERRUPT_ID_PCIE_MSI_FIRST
#define	INTERRUPT_ID_PCIE0_MSI_LAST	(INTERRUPT_ID_PCIE0_MSI_FIRST + PCIE_MSI_IDS_PER_DOMAIN - 1)
#define	INTERRUPT_ID_PCIE1_MSI_FIRST	(INTERRUPT_ID_PCIE0_MSI_LAST + 1)
#define	INTERRUPT_ID_PCIE1_MSI_LAST	(INTERRUPT_ID_PCIE1_MSI_FIRST + PCIE_MSI_IDS_PER_DOMAIN - 1)
#define	INTERRUPT_ID_PCIE_MSI_LAST	INTERRUPT_ID_PCIE1_MSI_LAST

/* Last Virtual Interrupt ID */
#define INTERRUPT_ID_LAST_VIRT		INTERRUPT_ID_PCIE_MSI_LAST

#define INTERRUPT_ID_LAST		INTERRUPT_ID_LAST_VIRT

#ifdef __BOARD_DRV_ARMV7__
unsigned int bcm_phys_irqs_to_map[] =
{
	INTERRUPT_ARM_TIMER,
	INTERRUPT_WDTIMER,
	INTERRUPT_DDRSEC,
	INTERRUPT_UBUSERR,
	INTERRUPT_DG,
	INTERRUPT_PMC0,
	INTERRUPT_PMC1,
	INTERRUPT_UART0,
	INTERRUPT_UART1,
	INTERRUPT_HS_UART,
	INTERRUPT_AIPETB,
	INTERRUPT_UBUS2ER,
	INTERRUPT_HS_SPIM,
	INTERRUPT_NAND_FLASH,
	INTERRUPT_DDRC,
	INTERRUPT_SARC,
	INTERRUPT_USBDC,
	INTERRUPT_PCMC,
	INTERRUPT_RUNNER_0,
	INTERRUPT_RUNNER_1,
	INTERRUPT_RUNNER_2,
	INTERRUPT_RUNNER_3,
	INTERRUPT_RUNNER_4,
	INTERRUPT_RUNNER_5,
	INTERRUPT_RUNNER_6,
	INTERRUPT_RUNNER_7,
	INTERRUPT_RUNNER_8,
	INTERRUPT_RUNNER_9,
	INTERRUPT_RDP_SBPM,
	INTERRUPT_RDP_BPM,
	INTERRUPT_SF2_0,
	INTERRUPT_SF2_1,
	INTERRUPT_PCIE0,
	INTERRUPT_PCIE1,
	INTERRUPT_DECT_0,
	INTERRUPT_DECT_1,
	INTERRUPT_USB_OHCI,
	INTERRUPT_USB_EHCI,
	INTERRUPT_USB_XHCI,
	INTERRUPT_USB_HBR,
	INTERRUPT_USB_HEV,
	INTERRUPT_I2C,
	INTERRUPT_I2S,
	INTERRUPT_RNG,
	INTERRUPT_EMMC,
	INTERRUPT_PL081,
	INTERRUPT_SAR_0,
	INTERRUPT_SAR_1,
	INTERRUPT_SAR_2,
	INTERRUPT_SAR_3,
	INTERRUPT_SAR_4,
	INTERRUPT_SAR_5,
	INTERRUPT_SAR_6,
	INTERRUPT_SAR_7,
	INTERRUPT_SAR_8,
	INTERRUPT_SAR_9,
	INTERRUPT_SAR_10,
	INTERRUPT_SAR_11,
	INTERRUPT_SAR_12,
	INTERRUPT_SAR_13,
	INTERRUPT_SAR_14,
	INTERRUPT_SAR_15,
	INTERRUPT_SAR_16,
	INTERRUPT_SAR_17,
	INTERRUPT_SAR_18,
	INTERRUPT_SAR_19,
	INTERRUPT_PCM_0,
	INTERRUPT_PCM_1,
	INTERRUPT_USBD_0,
	INTERRUPT_USBD_1,
	INTERRUPT_USBD_2,
	INTERRUPT_USBD_3,
	INTERRUPT_USBD_4,
	INTERRUPT_USBD_5
};
unsigned int bcm_legacy_irq_map[256];
#else
extern unsigned int bcm_phys_irqs_to_map[];
extern unsigned int bcm_legacy_irq_map[];
#endif
#endif

#define NUM_EXT_INT			(INTERRUPT_EXTERNAL_5-INTERRUPT_EXTERNAL_0+1)

#ifdef __cplusplus
}
#endif

#endif /* __BCM63138_H */

