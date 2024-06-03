/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2017 NXP Semiconductor, Inc.
 *
 */
#ifndef __FSL_STREAM_ID_H
#define __FSL_STREAM_ID_H

/*
 * Stream IDs on Chassis-2 (for example ls1043a, ls1046a, ls1012) devices
 * are not hardwired and are programmed by sw.  There are a limited number
 * of stream IDs available, and the partitioning of them is scenario
 * dependent. This header defines the partitioning between legacy, PCI,
 * and DPAA1 devices.
 *
 * This partitioning can be customized in this file depending
 * on the specific hardware config:
 *
 *  -non-PCI legacy, platform devices (USB, SDHC, SATA, DMA, QE etc)
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
 *  -DPAA1
 *     - Stream ids for DPAA1 use are reserved for future usecase.
 *
 */


#define FSL_INVALID_STREAM_ID		0

/* legacy devices */
#define FSL_USB1_STREAM_ID		1
#define FSL_USB2_STREAM_ID		2
#define FSL_USB3_STREAM_ID		3
#define FSL_SDHC_STREAM_ID		4
#define FSL_SATA_STREAM_ID		5
#define FSL_QE_STREAM_ID		6
#define FSL_QDMA_STREAM_ID		7
#define FSL_EDMA_STREAM_ID		8
#define FSL_ETR_STREAM_ID		9
#define FSL_DEBUG_STREAM_ID		10

/* PCI - programmed in PEXn_LUT */
#define FSL_PEX_STREAM_ID_START		11
#define FSL_PEX_STREAM_ID_END		26

/* DPAA1 - Stream-ID that can be programmed in DPAA1 h/w */
#define FSL_DPAA1_STREAM_ID_START	27
#define FSL_DPAA1_STREAM_ID_END		63

#endif
