/*
   Copyright (c) 2015 Broadcom
   All Rights Reserved

    <:label-BRCM:2015:DUAL/GPL:standard

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



/* This is an automated file. Do not edit its contents. */


#include "rdd.h"
#include "ru_types.h"

#include "rdd_data_structures_auto.h"
/* >>>RDD_CPU_RECYCLE_INTERRUPT_COALESCING_TABLE_ADDRESS_ARR */
uint32_t RDD_CPU_RECYCLE_INTERRUPT_COALESCING_TABLE_ADDRESS_ARR[GROUPED_EN_SEGMENTS_NUM] = {
	INVALID_TABLE_ADDRESS,
	0x2b0,
	0x9b0,
};
/* <<<RDD_CPU_RECYCLE_INTERRUPT_COALESCING_TABLE_ADDRESS_ARR */
/* >>>RDD_CPU_RECYCLE_RING_DESCRIPTOR_TABLE_ADDRESS_ARR */
uint32_t RDD_CPU_RECYCLE_RING_DESCRIPTOR_TABLE_ADDRESS_ARR[GROUPED_EN_SEGMENTS_NUM] = {
	INVALID_TABLE_ADDRESS,
	0x730,
	0xa20,
};
/* <<<RDD_CPU_RECYCLE_RING_DESCRIPTOR_TABLE_ADDRESS_ARR */
/* >>>RDD_CPU_RECYCLE_RING_INDEX_DDR_ADDR_TABLE_ADDRESS_ARR */
uint32_t RDD_CPU_RECYCLE_RING_INDEX_DDR_ADDR_TABLE_ADDRESS_ARR[GROUPED_EN_SEGMENTS_NUM] = {
	INVALID_TABLE_ADDRESS,
	0x760,
	0xa30,
};
/* <<<RDD_CPU_RECYCLE_RING_INDEX_DDR_ADDR_TABLE_ADDRESS_ARR */
/* >>>RDD_CPU_FEED_RING_DESCRIPTOR_TABLE_ADDRESS_ARR */
uint32_t RDD_CPU_FEED_RING_DESCRIPTOR_TABLE_ADDRESS_ARR[GROUPED_EN_SEGMENTS_NUM] = {
	INVALID_TABLE_ADDRESS,
	0x7e0,
	INVALID_TABLE_ADDRESS,
};
/* <<<RDD_CPU_FEED_RING_DESCRIPTOR_TABLE_ADDRESS_ARR */
/* >>>RDD_CPU_RECYCLE_RING_INTERRUPT_COUNTER_TABLE_ADDRESS_ARR */
uint32_t RDD_CPU_RECYCLE_RING_INTERRUPT_COUNTER_TABLE_ADDRESS_ARR[GROUPED_EN_SEGMENTS_NUM] = {
	INVALID_TABLE_ADDRESS,
	0xb28,
	0xa68,
};
/* <<<RDD_CPU_RECYCLE_RING_INTERRUPT_COUNTER_TABLE_ADDRESS_ARR */
/* >>>RDD_CPU_FEED_RING_INTERRUPT_THRESHOLD_ADDRESS_ARR */
uint32_t RDD_CPU_FEED_RING_INTERRUPT_THRESHOLD_ADDRESS_ARR[GROUPED_EN_SEGMENTS_NUM] = {
	INVALID_TABLE_ADDRESS,
	0xb4c,
	INVALID_TABLE_ADDRESS,
};
/* <<<RDD_CPU_FEED_RING_INTERRUPT_THRESHOLD_ADDRESS_ARR */
/* >>>RDD_CPU_RING_DESCRIPTORS_TABLE_ADDRESS_ARR */
uint32_t RDD_CPU_RING_DESCRIPTORS_TABLE_ADDRESS_ARR[GROUPED_EN_SEGMENTS_NUM] = {
	INVALID_TABLE_ADDRESS,
	0x3000,
	INVALID_TABLE_ADDRESS,
};
/* <<<RDD_CPU_RING_DESCRIPTORS_TABLE_ADDRESS_ARR */
/* >>>RDD_CPU_TX_RING_TABLE_ADDRESS_ARR */
uint32_t RDD_CPU_TX_RING_TABLE_ADDRESS_ARR[GROUPED_EN_SEGMENTS_NUM] = {
	INVALID_TABLE_ADDRESS,
	INVALID_TABLE_ADDRESS,
	0x780,
};
/* <<<RDD_CPU_TX_RING_TABLE_ADDRESS_ARR */
