/*
 * FIS (Force Immediate Save) ASM arrays for DHD
 *
 * Copyright (C) 2025, Broadcom. All Rights Reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 *
 * <<Broadcom-WL-IPTag/Open:>>
 *
 * $Id$
 */

#ifndef _DHD_FIS_DUMP_ASM_H_INCLUDED_
#define _DHD_FIS_DUMP_ASM_H_INCLUDED_

/* 6726(6716) ASM load address in BM */
#define ASM_START_ADDR		0x004A0000
#define ASM_ALL_SIZE		(18696) //bytes - end : 0x4a4908
#define ASM_CORE_SIZE		(256 * 1024) //bytes

/* The start address of FIS data for each core */
#define CORE_DATA_START_ADDR	0x004A4000

/*
 * The start address of FIS data for all the cores
 * The size of the ASM for all the cores (0x4a0000 ~ 0x4a4908) is 18696 Bytes.
 * So if the ASM is loaded,
 * the start address of FIS data for all the cores is 0x4a8000.
 */
#define ALL_DATA_START_ADDR		0x004A8000
#define ALL_DATA_END_ADDR		0x0059BEB0
#define ALL_DATA_WO_DIG_END_ADDR	0x00579E90

/* The FIS data size for all the cores */
#define ALL_DATA_SIZE_WO_DIG	(ALL_DATA_WO_DIG_END_ADDR - ALL_DATA_START_ADDR) // 859792 bytes
#define ALL_DATA_SIZE		(ALL_DATA_END_ADDR - ALL_DATA_START_ADDR) // 999088 bytes

/*
 * The FIS data size for each core
 * mac1 : 0x4a4000 ~ 0x4d712c = 209196 Bytes
 * mac2 : 0x4a4000 ~ 0x4b0ffc = 53244 Bytes
 * phy1phy2 : 0x4a4000 ~ 0x4dc4ec = 230636 Bytes
 * rx1 : 0x4a4000 ~ 0x4cb46c = 160876 Bytes
 * rx2 : 0x4a4000 ~ 0x4c37ac = 128940 Bytes
 * rx4 : 0x4a4000 ~ 0x4b172c = 55084 Bytes
 * cpu : 0x4a4000 ~ 0x4a969c = 22172 Bytes
 *
 * For all the cores (w/o DDIG)
 * w/o DIG : 0x4a8000 ~ 0x579e90 = 859792 Bytes
 * w/ DIG  : 0x4a8000 ~ 0x59beb0 = 999088 Bytes
 */

#define CORE_DATA_SIZE		(256 * 1024)

/* asm binaries */
uint32 asm_array_d11mac1[] =
#include "sr_asm/6726/wl_chip_pcie_82777_d11mac1_inst.h"
;

uint32 asm_array_d11mac2[] =
#include "sr_asm/6726/wl_chip_pcie_82778_d11mac2_inst.h"
;

uint32 asm_array_all[] =
#include "sr_asm/6726/wl_chip_pcie_82779_chip_with_dig_inst.h"
;

uint32 asm_array_all_wo_dig[] =
#include "sr_asm/6726/wl_chip_pcie_82779_chip_wo_dig_inst.h"
;

uint32 asm_array_phy12[] =
#include "sr_asm/6726/wl_chip_pcie_82773_phy1phy2_inst.h"
;

uint32 asm_array_rx1[] =
#include "sr_asm/6726/wl_chip_pcie_82774_phyrx1_inst.h"
;

uint32 asm_array_rx2[] =
#include "sr_asm/6726/wl_chip_pcie_82775_phyrx2_inst.h"
;

uint32 asm_array_rx4[] =
#include "sr_asm/6726/wl_chip_pcie_82776_phyrx4_inst.h"
;

uint32 asm_array_cpusub[] =
#include "sr_asm/6726/wl_chip_pcie_82770_cpusub_inst.h"
;

uint32 asm_array_dig[] =
#include "sr_asm/6726/wl_chip_pcie_82771_dig_inst.h"
;

uint32 asm_array_all_mac_phy_only[] =
#include "sr_asm/6726/wl_chip_pcie_82780_chip_only_mac_phy_inst.h"
;

#endif /* _DHD_FIS_DUMP_ASM_H_INCLUDED_ */
