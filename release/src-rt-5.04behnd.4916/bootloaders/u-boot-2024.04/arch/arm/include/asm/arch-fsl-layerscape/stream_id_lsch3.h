/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2015-2021 NXP
 * Copyright 2014 Freescale Semiconductor, Inc.
 *
 */
#ifndef __FSL_STREAM_ID_H
#define __FSL_STREAM_ID_H

/*
 * Stream IDs on NXP Chassis-3 (for example ls2080a, ls1088a, ls2088a)
 * devices are not hardwired and are programmed by sw. There are a limited
 * number of stream IDs available, and the partitioning of them is scenario
 * dependent. This header defines the partitioning between legacy,
 * PCI, and DPAA2 devices.
 *
 * This partitioning can be customized in this file depending
 * on the specific hardware config:
 *
 *  -non-PCI legacy, platform devices (USB, SD/MMC, SATA, DMA)
 *     -all legacy devices get a unique stream ID assigned and programmed in
 *      their AMQR registers by u-boot
 *
 *  -PCIe
 *     -there is a range of stream IDs set aside for PCI in this
 *      file.  U-Boot will scan the PCI bus and for each device discovered:
 *         -allocate a streamID
 *         -set a PEXn LUT table entry mapping 'requester ID' to 'stream ID'
 *         -set a msi-map entry in the PEXn controller node in the
 *          device tree (see Documentation/devicetree/bindings/pci/pci-msi.txt
 *          for more info on the msi-map definition)
 *         -set a iommu-map entry in the PEXn controller node in the
 *          device tree (see Documentation/devicetree/bindings/pci/pci-iommu.txt
 *          for more info on the iommu-map definition)
 *
 *  -DPAA2
 *     -u-boot will allocate a range of stream IDs to be used by the Management
 *      Complex for containers and will set these values in the MC DPC image.
 *     -u-boot will fixup the iommu-map property in the fsl-mc node in the
 *      device tree (see Documentation/devicetree/bindings/misc/fsl,qoriq-mc.txt
 *      for more info on the msi-map definition)
 *     -the MC is responsible for allocating and setting up 'isolation context
 *      IDs (ICIDs) based on the allocated stream IDs for all DPAA2 devices.
 *
 *  - ECAM (integrated PCI)
 *     - U-Boot applies the value here to HW and does DT fix-up for both
 *       'iommu-map' and 'msi-map'
 *
 * On Chasis-3 SoCs stream IDs are programmed in AMQ registers (32-bits) for
 * each of the different bus masters.  The relationship between
 * the AMQ registers and stream IDs is defined in the table below:
 *          AMQ bit    streamID bit
 *      ---------------------------
 *           PL[18]         9        // privilege bit
 *          BMT[17]         8        // bypass translation
 *           VA[16]         7        // reserved
 *             [15]         -        // unused
 *         ICID[14:7]       -        // unused
 *         ICID[6:0]        6-0      // isolation context id
 *     ----------------------------
 *
 */

#define AMQ_PL_MASK			(0x1 << 18)   /* priviledge bit */
#define AMQ_BMT_MASK			(0x1 << 17)   /* bypass bit */

#define FSL_INVALID_STREAM_ID		0

#define FSL_BYPASS_AMQ			(AMQ_PL_MASK | AMQ_BMT_MASK)

/* legacy devices */
#define FSL_USB1_STREAM_ID		1
#define FSL_USB2_STREAM_ID		2
#define FSL_SDMMC_STREAM_ID		3
#define FSL_SATA1_STREAM_ID		4

#if defined(CONFIG_ARCH_LS2080A) || defined(CONFIG_ARCH_LX2160A) || \
	defined(CONFIG_ARCH_LX2162A)
#define FSL_SATA2_STREAM_ID		5
#endif

#if defined(CONFIG_ARCH_LS2080A) || defined(CONFIG_ARCH_LX2160A) || \
	defined(CONFIG_ARCH_LX2162A)
#define FSL_DMA_STREAM_ID		6
#elif defined(CONFIG_ARCH_LS1088A) || defined(CONFIG_ARCH_LS1028A)
#define FSL_DMA_STREAM_ID		5
#endif

/* PCI - programmed in PEXn_LUT */
#define FSL_PEX_STREAM_ID_START		7

#if defined(CONFIG_ARCH_LS2080A) || defined(CONFIG_ARCH_LS1028A)
#define FSL_PEX_STREAM_ID_END		22
#elif defined(CONFIG_ARCH_LS1088A)
#define FSL_PEX_STREAM_ID_END		18
#elif defined(CONFIG_ARCH_LX2160A) || defined(CONFIG_ARCH_LX2162A)
#define FSL_PEX_STREAM_ID_END          (0x100)
#endif


/* DPAA2 - set in MC DPC and alloced by MC */
#define FSL_DPAA2_STREAM_ID_START	23
#define FSL_DPAA2_STREAM_ID_END		63

/* PCI IEPs, this overlaps DPAA2 but these two are exclusive at least for now */
#define FSL_ECAM_STREAM_ID_START	41
#define FSL_ECAM_STREAM_ID_END		63

#define FSL_SEC_STREAM_ID		64
#define FSL_SEC_JR1_STREAM_ID		65
#define FSL_SEC_JR2_STREAM_ID		66
#define FSL_SEC_JR3_STREAM_ID		67
#define FSL_SEC_JR4_STREAM_ID		68

#define FSL_SDMMC2_STREAM_ID		69

/*
 * Erratum A-050382 workaround
 *
 * Description:
 *   The eDMA ICID programmed in the eDMA_AMQR register in DCFG is not
 *   correctly forwarded to the SMMU.
 * Workaround:
 *   Program eDMA ICID in the eDMA_AMQR register in DCFG to 40.
 */
#ifdef CONFIG_SYS_FSL_ERRATUM_A050382
#define FSL_EDMA_STREAM_ID		40
#else
#define FSL_EDMA_STREAM_ID		70
#endif

#define FSL_GPU_STREAM_ID		71
#define FSL_DISPLAY_STREAM_ID		72
#define FSL_SATA3_STREAM_ID		73
#define FSL_SATA4_STREAM_ID		74

#endif
