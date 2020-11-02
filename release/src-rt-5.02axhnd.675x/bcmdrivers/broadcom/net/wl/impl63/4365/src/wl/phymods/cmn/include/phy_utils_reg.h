/*
 * PHY utils - register access functions.
 *
 * Copyright 2020 Broadcom
 *
 * This program is the proprietary software of Broadcom and/or
 * its licensors, and may only be used, duplicated, modified or distributed
 * pursuant to the terms and conditions of a separate, written license
 * agreement executed between you and Broadcom (an "Authorized License").
 * Except as set forth in an Authorized License, Broadcom grants no license
 * (express or implied), right to use, or waiver of any kind with respect to
 * the Software, and Broadcom expressly reserves all rights in and to the
 * Software and all intellectual property rights therein.  IF YOU HAVE NO
 * AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY
 * WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF
 * THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use
 * all reasonable efforts to protect the confidentiality thereof, and to
 * use this information only in connection with your use of Broadcom
 * integrated circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES,
 * REPRESENTATIONS OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR
 * OTHERWISE, WITH RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY
 * DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY,
 * NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES,
 * ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR
 * CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING
 * OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL
 * BROADCOM OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL,
 * SPECIAL, INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR
 * IN ANY WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
 * IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii)
 * ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF
 * OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY
 * NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
 * $Id$
 */

#ifndef _phy_utils_reg_h_
#define _phy_utils_reg_h_

#include <typedefs.h>
#include <bcmdevs.h>
#include <bcmdefs.h>

#include <wlc_phy_types.h>

#define RADIO_REG_READ_FAIL 0xffff

/* %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */
/*  table-driven register operations                            */
/* %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */

#define PHY_REG_MOD(pi, phy_type, reg_name, field, value) \
	phy_utils_mod_phyreg(pi, phy_type##_##reg_name, \
	phy_type##_##reg_name##_##field##_MASK, \
	(value) << phy_type##_##reg_name##_##field##_SHIFT)

#define PHY_REG_MOD2(pi, phy_type, reg_name, field, field2, value, value2) \
	phy_utils_mod_phyreg(pi, phy_type##_##reg_name, \
	phy_type##_##reg_name##_##field##_MASK | phy_type##_##reg_name##_##field2##_MASK, \
	((value) << phy_type##_##reg_name##_##field##_SHIFT) | \
	((value2) << phy_type##_##reg_name##_##field2##_SHIFT))

#define PHY_REG_MOD3(pi, phy_type, reg_name, field, field2, field3, value, value2, value3) \
	phy_utils_mod_phyreg(pi, phy_type##_##reg_name, \
	phy_type##_##reg_name##_##field##_MASK | phy_type##_##reg_name##_##field2##_MASK | \
	phy_type##_##reg_name##_##field3##_MASK, \
	((value) << phy_type##_##reg_name##_##field##_SHIFT) | \
	((value2) << phy_type##_##reg_name##_##field2##_SHIFT) | \
	((value3) << phy_type##_##reg_name##_##field3##_SHIFT))

#define PHY_REG_MOD_RAW(pi, addr, mask, value) \
	phy_utils_mod_phyreg(pi, addr, mask, value)

#define PHY_REG_READ(pi, phy_type, reg_name, field) \
	((phy_utils_read_phyreg(pi, phy_type##_##reg_name) & \
	phy_type##_##reg_name##_##field##_##MASK) >> phy_type##_##reg_name##_##field##_##SHIFT)

#define PHY_REG_READ_CORE(pi, phy_type, core, reg_name) \
	phy_utils_read_phyreg(pi, \
	core == 0 ? phy_type##_##reg_name##0 : phy_type##_##reg_name##1)

#define PHY_REG_MOD_CORE(pi, phy_type, core, reg_name, field, value) \
	phy_utils_mod_phyreg(pi, \
	core == 0 ? phy_type##_##reg_name##0 : phy_type##_##reg_name##1, \
	phy_type##_##reg_name##_##field##_MASK, \
	(value) << phy_type##_##reg_name##_##field##_SHIFT)

#define PHY_REG_WRITE(pi, phy_type, reg_name, value) \
	phy_utils_write_phyreg(pi, phy_type##_##reg_name, value)

#define PHY_REG_WRITE_CORE(pi, phy_type, core, reg_name, value) \
	phy_utils_write_phyreg(pi, \
	core == 0 ? phy_type##_##reg_name##0 : phy_type##_##reg_name##1, \
	value)

#define PHY_REG_AND(pi, phy_type, reg_name, value) \
	phy_utils_and_phyreg(pi, phy_type##_##reg_name, value)

#define PHY_REG_OR(pi, phy_type, reg_name, value) \
	phy_utils_or_phyreg(pi, phy_type##_##reg_name, value)

/*
 * For table driven PHY/RADIO register access functionality, we'll
 * multiplex the register access type (AND/OR/MOD/WRITE) and Radio/PHY
 * type with the register address space in order to save a uint16 per
 * entry for dongles.
 * Bits 0-11 are in use for the register address spaces of the various PHYs
 * and radios, bit 12 is used for ACPHY_REG_BROADCAST.
 * Bit 15 can be used to indicate PHY or RADIO type, bit 14 and 13 can be
 * used to indicate access type (MOD/WRITE/AND/OR).
 * Unfortunately, the 2062 RADIO also uses bit 14 for its register space,
 * so we need to make an exception for dongles with 2062 radio
 */
#define PHY_RADIO_REG_MASK_TYPE		0xE000
#define RADIO_REG_TYPE			0x8000
#define PHY_REG_MOD_TYPE		0x0000
#define PHY_REG_WRITE_TYPE		0x4000
#define PHY_REG_AND_TYPE		0x2000
#define PHY_REG_OR_TYPE			0x6000

#if defined(DONGLEBUILD) && (BCMCHIPID != BCM4328_CHIP_ID)
#define PHY_RADIO_ACCESS_ADDR(type, addr) ((type) | (addr))
#else
#define PHY_RADIO_ACCESS_ADDR(type, addr) (type), (addr)
#endif // endif

#define PHY_REG_LIST_START \
	{ static const uint16 write_phy_reg_table[] = {

#define PHY_REG_LIST_START_WLBANDINITDATA \
	{ static const uint16 WLBANDINITDATA(write_phy_reg_table)[] = {

#define PHY_REG_LIST_START_BCMATTACHDATA \
	{ static const uint16 BCMATTACHDATA(write_phy_reg_table)[] = {

#define PHY_REG_LIST_EXECUTE(pi) \
	};						     \
	phy_utils_write_phyreg_array(pi, write_phy_reg_table, \
	sizeof(write_phy_reg_table)/sizeof(write_phy_reg_table[0])); }

#define PHY_REG_MOD_ENTRY(phy_type, reg_name, field, value) \
	PHY_RADIO_ACCESS_ADDR(PHY_REG_MOD_TYPE, (phy_type##_##reg_name)), \
	phy_type##_##reg_name##_##field##_MASK, \
	(value) << phy_type##_##reg_name##_##field##_SHIFT, \

#define PHY_REG_MOD2_ENTRY(phy_type, reg_name, field, field2, value, value2) \
	PHY_RADIO_ACCESS_ADDR(PHY_REG_MOD_TYPE, (phy_type##_##reg_name)), \
	(phy_type##_##reg_name##_##field##_MASK | phy_type##_##reg_name##_##field2##_MASK), \
	((value) << phy_type##_##reg_name##_##field##_SHIFT) | \
	((value2) << phy_type##_##reg_name##_##field2##_SHIFT), \

#define PHY_REG_MOD3_ENTRY(phy_type, reg_name, field, field2, field3, value, value2, value3) \
	PHY_RADIO_ACCESS_ADDR(PHY_REG_MOD_TYPE, (phy_type##_##reg_name)), \
	(phy_type##_##reg_name##_##field##_MASK | phy_type##_##reg_name##_##field2##_MASK | \
	phy_type##_##reg_name##_##field3##_MASK), \
	((value) << phy_type##_##reg_name##_##field##_SHIFT) | \
	((value2) << phy_type##_##reg_name##_##field2##_SHIFT) | \
	((value3) << phy_type##_##reg_name##_##field3##_SHIFT), \

#define PHY_REG_MOD_CORE_ENTRY(phy_type, core, reg_name, field, value) \
	PHY_RADIO_ACCESS_ADDR(PHY_REG_MOD_TYPE, \
		(core == 0 ? phy_type##_##reg_name##0 : phy_type##_##reg_name##1)), \
	phy_type##_##reg_name##_##field##_MASK, \
	(value) << phy_type##_##reg_name##_##field##_SHIFT, \

#define PHY_REG_MOD_RAW_ENTRY(addr, mask, value) \
	PHY_RADIO_ACCESS_ADDR(PHY_REG_MOD_TYPE, (addr)), (mask), (value), \

#define PHY_REG_WRITE_ENTRY(phy_type, reg_name, value) \
	PHY_RADIO_ACCESS_ADDR(PHY_REG_WRITE_TYPE, (phy_type##_##reg_name)), (value), \

#define PHY_REG_WRITE_RAW_ENTRY(addr, value) \
	PHY_RADIO_ACCESS_ADDR(PHY_REG_WRITE_TYPE, (addr)), (value), \

#define PHY_REG_AND_ENTRY(phy_type, reg_name, value) \
	PHY_RADIO_ACCESS_ADDR(PHY_REG_AND_TYPE, (phy_type##_##reg_name)), (value), \

#define PHY_REG_AND_RAW_ENTRY(addr, value) \
	PHY_RADIO_ACCESS_ADDR(PHY_REG_AND_TYPE, (addr)), (value), \

#define PHY_REG_OR_ENTRY(phy_type, reg_name, value) \
	PHY_RADIO_ACCESS_ADDR(PHY_REG_OR_TYPE, (phy_type##_##reg_name)), (value), \

#define PHY_REG_OR_RAW_ENTRY(addr, value) \
	PHY_RADIO_ACCESS_ADDR(PHY_REG_OR_TYPE, (addr)), (value), \

#define RADIO_REG_MOD_ENTRY(reg_name, field, value) \
	PHY_RADIO_ACCESS_ADDR((RADIO_REG_TYPE | PHY_REG_MOD_TYPE), (reg_name)), (field), (value), \

#define RADIO_REG_WRITE_ENTRY(reg_name, value) \
	PHY_RADIO_ACCESS_ADDR((RADIO_REG_TYPE | PHY_REG_WRITE_TYPE), (reg_name)), (value), \

#define RADIO_REG_AND_ENTRY(reg_name, value) \
	PHY_RADIO_ACCESS_ADDR((RADIO_REG_TYPE | PHY_REG_AND_TYPE), (reg_name)), (value), \

#define RADIO_REG_OR_ENTRY(reg_name, value) \
	PHY_RADIO_ACCESS_ADDR((RADIO_REG_TYPE | PHY_REG_OR_TYPE), (reg_name)), (value), \

void phy_utils_phyreg_enter(phy_info_t *pi);
void phy_utils_phyreg_exit(phy_info_t *pi);
void phy_utils_radioreg_enter(phy_info_t *pi);
void phy_utils_radioreg_exit(phy_info_t *pi);
uint16 phy_utils_read_radioreg(phy_info_t *pi, uint16 addr);
void phy_utils_write_radioreg(phy_info_t *pi, uint16 addr, uint16 val);
void phy_utils_and_radioreg(phy_info_t *pi, uint16 addr, uint16 val);
void phy_utils_or_radioreg(phy_info_t *pi, uint16 addr, uint16 val);
void phy_utils_xor_radioreg(phy_info_t *pi, uint16 addr, uint16 mask);
void phy_utils_mod_radioreg(phy_info_t *pi, uint16 addr, uint16 mask, uint16 val);
void phy_utils_gen_radioreg(phy_info_t *pi, uint16 addr, uint16 mask, uint16 val,
	uint16* orig_reg_addr, uint16* orig_reg_data,
	uint16* updated_reg_addr, uint16* updated_reg_data);
void phy_utils_write_phychannelreg(phy_info_t *pi, uint val);
uint16 phy_utils_read_phyreg(phy_info_t *pi, uint16 addr);
uint16 phy_utils_read_phyreg_wide(phy_info_t *pi);
void phy_utils_write_phyreg_array(phy_info_t *pi, const uint16* regp, int length);
void phy_utils_write_phyreg(phy_info_t *pi, uint16 addr, uint16 val);
void phy_utils_write_phyreg_wide(phy_info_t *pi, uint16 val);
void phy_utils_and_phyreg(phy_info_t *pi, uint16 addr, uint16 val);
void phy_utils_or_phyreg(phy_info_t *pi, uint16 addr, uint16 val);
void phy_utils_mod_phyreg(phy_info_t *pi, uint16 addr, uint16 mask, uint16 val);
void phy_utils_gen_phyreg(phy_info_t *pi, uint16 addr, uint16 mask, uint16 val,
	uint16* orig_reg_addr, uint16* orig_reg_data,
	uint16* updated_reg_addr, uint16* updated_reg_data);

#if defined(BCMDBG_PHYREGS_TRACE)
extern uint16 phy_utils_read_phyreg_debug(phy_info_t *pi, uint16 addr, const char *reg_name);
extern void phy_utils_mod_phyreg_debug(phy_info_t *pi, uint16 addr, uint16 mask, uint16 val,
                              const char *reg_name);
extern void phy_utils_write_phyreg_debug(phy_info_t *pi, uint16 addr, uint16 val,
	const char *reg_name);
extern uint16 phy_utils_read_radioreg_debug(phy_info_t *pi, uint16 addr, const char *reg_name);
extern void phy_utils_mod_radioreg_debug(phy_info_t *pi, uint16 addr, uint16 mask, uint16 val,
                                const char *reg_name);
#endif /* BCMDBG_PHYREGS_TRACE */

void phy_utils_write_phytable_addr(phy_info_t *pi, uint tbl_id, uint tbl_offset,
                   uint16 tblAddr, uint16 tblDataHi, uint16 tblDataLo);
void phy_utils_write_phytable_data(phy_info_t *pi, uint width, uint32 val);
void phy_utils_write_phytable(phy_info_t *pi, const phytbl_info_t *ptbl_info, uint16 tblAddr,
	uint16 tblDataHi, uint16 tblDataLo);
void phy_utils_read_phytable(phy_info_t *pi, const phytbl_info_t *ptbl_info, uint16 tblAddr,
	uint16 tblDataHi, uint16 tblDataLo);
void phy_utils_write_phytable_ext(phy_info_t *pi, const phytbl_info_t *ptbl_info, uint16 tblId,
    uint16 tblOffset, uint16 tblDataWide, uint16 tblDataHi, uint16 tblDataLo);
void phy_utils_read_phytable_ext(phy_info_t *pi, const phytbl_info_t *ptbl_info, uint16 tblId,
    uint16 tblOffset, uint16 tblDataWide, uint16 tblDataHi, uint16 tblDataLo);
void phy_utils_read_common_phytable(phy_info_t *pi, uint32 tbl_id,
	const void *tbl_ptr, uint32 tbl_len, uint32 tbl_width, uint32 tbl_offset,
	void (*tbl_rfunc)(phy_info_t *, phytbl_info_t *));
void phy_utils_write_common_phytable(phy_info_t *pi, uint32 tbl_id, const void *tbl_ptr,
	uint32 tbl_len, uint32 tbl_width, uint32 tbl_offset,
	void (*tbl_wfunc)(phy_info_t *, const phytbl_info_t *));

/*
 * These are utility routines for reading or writing a sequence of registers
 * in a space-efficient manner, driven by a table of addresses.
 * The 'addrvals' parameter is table of uint16s: addr1, val1, addr2, val2, ..., addrN, valN.
 * 'nregs' is the number of pairs, which is twice the number of array uint16 elements.
 *
 * In 'write' operations, the source data is in the array and the dest is in hardware.
 * In read operations, the source is in hardware and the destination is in the array.
 *
 */

void phy_utils_bulkread_phyregs(phy_info_t *pi, uint16 *addrvals, uint32 nregs);
void phy_utils_bulkwrite_phyregs(phy_info_t *pi, const uint16 *addrvals, uint32 nregs);
void phy_utils_bulkread_radioregs(phy_info_t *pi, uint16 *addrvals, uint32 nregs);
void phy_utils_bulkwrite_radioregs(phy_info_t *pi, const uint16 *addrvals, uint32 nregs);
void phy_utils_bulkmod_phyregs(phy_info_t *pi, uint16 *regs, uint16 *mask, uint16 *val,
	uint32 nregs);

#endif	/* _phy_utils_reg_h_ */
