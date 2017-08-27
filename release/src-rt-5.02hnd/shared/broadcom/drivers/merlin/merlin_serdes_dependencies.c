/*
<:copyright-BRCM:2015:proprietary:standard

   Copyright (c) 2015 Broadcom 
   All Rights Reserved

 This program is the proprietary software of Broadcom and/or its
 licensors, and may only be used, duplicated, modified or distributed pursuant
 to the terms and conditions of a separate, written license agreement executed
 between you and Broadcom (an "Authorized License").  Except as set forth in
 an Authorized License, Broadcom grants no license (express or implied), right
 to use, or waiver of any kind with respect to the Software, and Broadcom
 expressly reserves all rights in and to the Software and all intellectual
 property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
 NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
 BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.

 Except as expressly set forth in the Authorized License,

 1. This program, including its structure, sequence and organization,
    constitutes the valuable trade secrets of Broadcom, and you shall use
    all reasonable efforts to protect the confidentiality thereof, and to
    use this information only in connection with your use of Broadcom
    integrated circuit products.

 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
    RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
    ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
    FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
    COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
    TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
    PERFORMANCE OF THE SOFTWARE.

 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
    ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
    INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
    WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
    IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
    OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
    SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
    SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
    LIMITED REMEDY.
:>

*/
/***********************************************************************/
/*                                                                     */
/*   MODULE:  merlin_serdes_dependencies.c                             */
/*   DATE:    30/11/2015                                               */
/*   PURPOSE: dependencies functions used by SDK code                  */
/*                                                                     */
/***********************************************************************/ 
 
#ifdef _CFE_
extern void cfe_usleep(int usec);
#define udelay(_a) cfe_usleep(_a)
#define mdelay(_a) cfe_usleep(1000*_a)
#else
#include <asm/delay.h>
#endif

#include "merlin_mptwo_dependencies.h"
#include "merlin_serdes.h"
#include "serdes_access.h"

#define MERLIN_BLOCK(x) (x&0xF000)

#ifdef MERLIN_PC_DEBUG
int write_serdes_reg(E_MERLIN_ID merlin_id, uint32_t addr, uint16_t mask, uint16_t value)
{
    //printk("merlin_id: %d, addr: 0x%8x, mask: 0x%x, value: 0x%4x\n", merlin_id, addr, mask, value);
    return 0;
}

int read_serdes_reg(E_MERLIN_ID merlin_id, uint32_t addr, uint16_t mask,uint16_t *value)
{
    //printk("merlin_id: %d, addr: 0x%8x, mask: 0x%x, value: 0x%4x\n", merlin_id, addr, mask, *value);
    return 0;
}
#endif

int logger_write(int message_verbose_level, const char *format, ...){
    va_list args;
#ifndef _CFE_
    int r;
#endif    

    va_start(args, format);
#ifndef _CFE_
    r = vprintk(format, args);
#else
    vprintf(format, args);
#endif    
    va_end(args);
    return ERR_CODE_NONE;
}

err_code_t merlin_mptwo_pmd_mwr_reg(merlin_access_t *ma, uint16_t addr, uint16_t mask, uint8_t lsb, uint16_t val) {
    E_MERLIN_ID merlin_id = ma->index/MERLIN_LANES_PER_CORE;
    uint32_t lane_index = ma->index%MERLIN_LANES_PER_CORE;
    uint32_t addr_hi;

    if((MERLIN_BLOCK(addr) == 0xC000) || (MERLIN_BLOCK(addr) == 0x9000))
        addr_hi = MERLIN_PCS<<MMD_TYPE_OFFSET | lane_index<<LANE_ADDRESS_OFFSET;
    else
        addr_hi = MERLIN_PMD_PMA<<MMD_TYPE_OFFSET | lane_index<<LANE_ADDRESS_OFFSET;
    return write_serdes_reg(merlin_id, (addr_hi|(uint32_t)addr), mask, val <<lsb);
}

err_code_t merlin_mptwo_pmd_rdt_reg(merlin_access_t *ma, uint16_t address, uint16_t *val){
    E_MERLIN_ID merlin_id = ma->index/MERLIN_LANES_PER_CORE;
    uint32_t lane_index = ma->index%MERLIN_LANES_PER_CORE;
    uint32_t addr_hi;
    
    if((MERLIN_BLOCK(address) == 0xC000) || (MERLIN_BLOCK(address) == 0x9000))
        addr_hi = MERLIN_PCS<<MMD_TYPE_OFFSET | lane_index<<LANE_ADDRESS_OFFSET;
    else
        addr_hi = MERLIN_PMD_PMA<<MMD_TYPE_OFFSET | lane_index<<LANE_ADDRESS_OFFSET;
    return read_serdes_reg(merlin_id, (addr_hi|(uint32_t)address), 0xFFFF, val);
}

err_code_t merlin_mptwo_pmd_wr_reg(merlin_access_t *ma, uint16_t address, uint16_t val){
    E_MERLIN_ID merlin_id = ma->index/MERLIN_LANES_PER_CORE;
    uint32_t lane_index = ma->index%MERLIN_LANES_PER_CORE;
    uint32_t addr_hi;

    if((MERLIN_BLOCK(address) == 0xC000) || (MERLIN_BLOCK(address) == 0x9000))
        addr_hi = MERLIN_PCS<<MMD_TYPE_OFFSET | lane_index<<LANE_ADDRESS_OFFSET;
    else
        addr_hi = MERLIN_PMD_PMA<<MMD_TYPE_OFFSET | lane_index<<LANE_ADDRESS_OFFSET;
    return write_serdes_reg(merlin_id, (addr_hi|(uint32_t)address), 0xFFFF, val);
}

err_code_t merlin_mptwo_delay_us(uint32_t delay_us){
    if(delay_us < 1000)
        udelay(delay_us);
    else
        mdelay(delay_us/1000);
    return ERR_CODE_NONE;
}

err_code_t merlin_mptwo_delay_ns(uint16_t delay_ns) {
#ifndef _CFE_
    ndelay(delay_ns);
#else
    udelay(1);  /*1000ns for CFE*/
#endif
    return ERR_CODE_NONE;
}

uint8_t merlin_mptwo_get_lane(merlin_access_t *ma) {
    return (ma->index%MERLIN_LANES_PER_CORE);
}

uint8_t merlin_mptwo_get_core(merlin_access_t *ma) {
    return (ma->index/MERLIN_LANES_PER_CORE);
}

err_code_t merlin_mptwo_uc_lane_idx_to_system_id(char *string , uint8_t uc_lane_idx) {
    return(0);
}

err_code_t merlin_mptwo_pmd_rdt_reg_addr32(merlin_access_t *ma, uint32_t address, uint16_t *val)
{
    E_MERLIN_ID merlin_id = ma->index/MERLIN_LANES_PER_CORE;
    uint32_t lane_index = ma->index%MERLIN_LANES_PER_CORE;

    address = address|lane_index<<LANE_ADDRESS_OFFSET;
    
    return read_serdes_reg(merlin_id, address, 0xFFFF, val);
}

err_code_t merlin_mptwo_pmd_wr_reg_addr32(merlin_access_t *ma, uint32_t address, uint16_t val)
{
    E_MERLIN_ID merlin_id = ma->index/MERLIN_LANES_PER_CORE;
    uint32_t lane_index = ma->index%MERLIN_LANES_PER_CORE;

    address = address|lane_index<<LANE_ADDRESS_OFFSET;
    
    return write_serdes_reg(merlin_id, address, 0xFFFF, val);
}

err_code_t merlin_mptwo_pmd_mwr_reg_addr32(merlin_access_t *ma, uint32_t addr, uint16_t mask, uint8_t lsb, uint16_t val)
{
    E_MERLIN_ID merlin_id = ma->index/MERLIN_LANES_PER_CORE;
    uint32_t lane_index = ma->index%MERLIN_LANES_PER_CORE;

    addr = addr|lane_index<<LANE_ADDRESS_OFFSET;
    
    return write_serdes_reg(merlin_id, addr, mask, val<<lsb);
}

