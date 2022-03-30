/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2015-2018 NXP
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
 *      file.  U-boot will scan the PCI bus and for each device discovered:
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

#if defined(CONFIG_ARCH_LS2080A) || defined(CONFIG_ARCH_LX2160A)
#define FSL_SATA2_STREAM_ID		5
#endif

#if defined(CONFIG_ARCH_LS2080A) || defined(CONFIG_ARCH_LX2160A)
#define FSL_DMA_STREAM_ID		6
#elif defined(CONFIG_ARCH_LS1088A)
#define FSL_DMA_STREAM_ID		5
#endif

/* PCI - programmed in PEXn_LUT */
#define FSL_PEX_STREAM_ID_START		7

#ifdef CONFIG_ARCH_LX2160A
#define FSL_PEX_STREAM_ID_NUM		(0x100)
#endif

#if defined(CONFIG_ARCH_LS2080A) || defined(CONFIG_ARCH_LS1028A)
#define FSL_PEX_STREAM_ID_END		22
#elif defined(CONFIG_ARCH_LS1088A)
#define FSL_PEX_STREAM_ID_END		18
#endif


/* DPAA2 - set in MC DPC and alloced by MC */
#define FSL_DPAA2_STREAM_ID_START	23
#define FSL_DPAA2_STREAM_ID_END		63

#endif
