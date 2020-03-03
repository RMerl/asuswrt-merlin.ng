/*
 * Intel MIC Platform Software Stack (MPSS)
 *
 * Copyright(c) 2013 Intel Corporation.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 2, as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * The full GNU General Public License is included in this distribution in
 * the file called "COPYING".
 *
 * Intel MIC Host driver.
 *
 */
#ifndef _MIC_X100_HW_H_
#define _MIC_X100_HW_H_

#define MIC_X100_PCI_DEVICE_2250 0x2250
#define MIC_X100_PCI_DEVICE_2251 0x2251
#define MIC_X100_PCI_DEVICE_2252 0x2252
#define MIC_X100_PCI_DEVICE_2253 0x2253
#define MIC_X100_PCI_DEVICE_2254 0x2254
#define MIC_X100_PCI_DEVICE_2255 0x2255
#define MIC_X100_PCI_DEVICE_2256 0x2256
#define MIC_X100_PCI_DEVICE_2257 0x2257
#define MIC_X100_PCI_DEVICE_2258 0x2258
#define MIC_X100_PCI_DEVICE_2259 0x2259
#define MIC_X100_PCI_DEVICE_225a 0x225a
#define MIC_X100_PCI_DEVICE_225b 0x225b
#define MIC_X100_PCI_DEVICE_225c 0x225c
#define MIC_X100_PCI_DEVICE_225d 0x225d
#define MIC_X100_PCI_DEVICE_225e 0x225e

#define MIC_X100_APER_BAR 0
#define MIC_X100_MMIO_BAR 4

#define MIC_X100_SBOX_BASE_ADDRESS 0x00010000
#define MIC_X100_SBOX_SPAD0 0x0000AB20
#define MIC_X100_SBOX_SICR0_DBR(x) ((x) & 0xf)
#define MIC_X100_SBOX_SICR0_DMA(x) (((x) >> 8) & 0xff)
#define MIC_X100_SBOX_SICE0_DBR(x) ((x) & 0xf)
#define MIC_X100_SBOX_DBR_BITS(x) ((x) & 0xf)
#define MIC_X100_SBOX_SICE0_DMA(x) (((x) >> 8) & 0xff)
#define MIC_X100_SBOX_DMA_BITS(x) (((x) & 0xff) << 8)

#define MIC_X100_SBOX_APICICR0 0x0000A9D0
#define MIC_X100_SBOX_SICR0 0x00009004
#define MIC_X100_SBOX_SICE0 0x0000900C
#define MIC_X100_SBOX_SICC0 0x00009010
#define MIC_X100_SBOX_SIAC0 0x00009014
#define MIC_X100_SBOX_MSIXPBACR 0x00009084
#define MIC_X100_SBOX_MXAR0 0x00009044
#define MIC_X100_SBOX_SMPT00 0x00003100
#define MIC_X100_SBOX_RDMASR0 0x0000B180

#define MIC_X100_DOORBELL_IDX_START 0
#define MIC_X100_NUM_DOORBELL 4
#define MIC_X100_DMA_IDX_START 8
#define MIC_X100_NUM_DMA 8
#define MIC_X100_ERR_IDX_START 30
#define MIC_X100_NUM_ERR 1

#define MIC_X100_NUM_SBOX_IRQ 8
#define MIC_X100_NUM_RDMASR_IRQ 8
#define MIC_X100_RDMASR_IRQ_BASE 17
#define MIC_X100_SPAD2_DOWNLOAD_STATUS(x) ((x) & 0x1)
#define MIC_X100_SPAD2_APIC_ID(x)	(((x) >> 1) & 0x1ff)
#define MIC_X100_SPAD2_DOWNLOAD_ADDR(x) ((x) & 0xfffff000)
#define MIC_X100_SBOX_APICICR7 0x0000AA08
#define MIC_X100_SBOX_RGCR 0x00004010
#define MIC_X100_SBOX_SDBIC0 0x0000CC90
#define MIC_X100_DOWNLOAD_INFO 2
#define MIC_X100_FW_SIZE 5
#define MIC_X100_POSTCODE 0x242c

static const u16 mic_x100_intr_init[] = {
		MIC_X100_DOORBELL_IDX_START,
		MIC_X100_DMA_IDX_START,
		MIC_X100_ERR_IDX_START,
		MIC_X100_NUM_DOORBELL,
		MIC_X100_NUM_DMA,
		MIC_X100_NUM_ERR,
};

/* Host->Card(bootstrap) Interrupt Vector */
#define MIC_X100_BSP_INTERRUPT_VECTOR 229

extern struct mic_hw_ops mic_x100_ops;
extern struct mic_smpt_ops mic_x100_smpt_ops;
extern struct mic_hw_intr_ops mic_x100_intr_ops;

#endif
