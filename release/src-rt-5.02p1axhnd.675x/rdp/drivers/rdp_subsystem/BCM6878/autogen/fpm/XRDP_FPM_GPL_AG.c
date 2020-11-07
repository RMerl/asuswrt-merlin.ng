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

#include "ru.h"

/******************************************************************************
 * Register: FPM_FPM_CTL
 ******************************************************************************/
const ru_reg_rec FPM_FPM_CTL_REG = 
{
    "FPM_CTL",
#if RU_INCLUDE_DESC
    "FPM Control Register",
    "",
#endif
    FPM_FPM_CTL_REG_OFFSET,
    0,
    0,
    169,
};

/******************************************************************************
 * Register: FPM_FPM_CFG1
 ******************************************************************************/
const ru_reg_rec FPM_FPM_CFG1_REG = 
{
    "FPM_CFG1",
#if RU_INCLUDE_DESC
    "FPM Configuration Register",
    "",
#endif
    FPM_FPM_CFG1_REG_OFFSET,
    0,
    0,
    170,
};

/******************************************************************************
 * Register: FPM_FPM_WEIGHT
 ******************************************************************************/
const ru_reg_rec FPM_FPM_WEIGHT_REG = 
{
    "FPM_WEIGHT",
#if RU_INCLUDE_DESC
    "FPM Configuration Register",
    "",
#endif
    FPM_FPM_WEIGHT_REG_OFFSET,
    0,
    0,
    171,
};

/******************************************************************************
 * Register: FPM_FPM_BB_CFG
 ******************************************************************************/
const ru_reg_rec FPM_FPM_BB_CFG_REG = 
{
    "FPM_BB_CFG",
#if RU_INCLUDE_DESC
    "FPM_BB Configuration Register",
    "",
#endif
    FPM_FPM_BB_CFG_REG_OFFSET,
    0,
    0,
    172,
};

/******************************************************************************
 * Register: FPM_POOL1_INTR_MSK
 ******************************************************************************/
const ru_reg_rec FPM_POOL1_INTR_MSK_REG = 
{
    "POOL1_INTR_MSK",
#if RU_INCLUDE_DESC
    "POOL2 Interrupt Mask Register",
    "Mask bits are active high and are disabled by default. Software enables "
    "desired bits as necessary",
#endif
    FPM_POOL1_INTR_MSK_REG_OFFSET,
    0,
    0,
    173,
};

/******************************************************************************
 * Register: FPM_POOL1_INTR_STS
 ******************************************************************************/
const ru_reg_rec FPM_POOL1_INTR_STS_REG = 
{
    "POOL1_INTR_STS",
#if RU_INCLUDE_DESC
    "POOL2 Interrupt Status Register",
    "Interrupt bits are active high. When a bit in this register is set to 1 and the "
    "corresponding bit in interrupt mask register is set to 1, interrupt to CPU will "
    "occur. When set (1), interrupts bits can be cleared (0) by writing a 1 to the "
    "desired bit. ",
#endif
    FPM_POOL1_INTR_STS_REG_OFFSET,
    0,
    0,
    174,
};

/******************************************************************************
 * Register: FPM_POOL1_STALL_MSK
 ******************************************************************************/
const ru_reg_rec FPM_POOL1_STALL_MSK_REG = 
{
    "POOL1_STALL_MSK",
#if RU_INCLUDE_DESC
    "POOL2 Stall FPM mask",
    "Software sets desired stall bits that upon corresponding active interrupt status "
    "will stall FPM from new allocation, de-allocation, and mcast update process. Listed "
    "below are the supported interrupt statuses \n"
    "1. Invalid free token (bit[3] of interrupt status register 0x14)\n"
    "2. Invalid free token with index out-of-range (bit[4] of interrupt status register 0x14)\n"
    "3. Invalid mcast token (bit[5] of interrupt status register 0x14)\n"
    "4. Invalid mcast token with index out-of-range (bit[6] of interrupt status register 0x14)\n"
    "5. Memory corrupt status (bit[8] of interrupt status register 0x14)\n"
    "When state machine is stalled, registers and memory can still be accessed. Any new token "
    "allocation request will be serviced with valid tokens (if available in alloc cache) and invalid "
    "tokens (if alloc cache is empty). Any new de-allocation/mcast update requests will be either stored "
    "in de-allocation fifo (if there is space in free fifo) or dropped (if free fifo is full). "
    "Bit locations in this register matches the location of corrseponding interrupt status bits in "
    "register 0x14. To un-stall (enable) state machine interrupt status bits (in register 0x14) "
    "corresponding to these mask bits should be cleared. Stall mask bits are active high and "
    "are disabled by default. This is for debug purposes only. ",
#endif
    FPM_POOL1_STALL_MSK_REG_OFFSET,
    0,
    0,
    175,
};

/******************************************************************************
 * Register: FPM_POOL2_INTR_MSK
 ******************************************************************************/
const ru_reg_rec FPM_POOL2_INTR_MSK_REG = 
{
    "POOL2_INTR_MSK",
#if RU_INCLUDE_DESC
    "POOL2 Interrupt Mask Register",
    "Mask bits are active high and are disabled by default. Software enables "
    "desired bits as necessary",
#endif
    FPM_POOL2_INTR_MSK_REG_OFFSET,
    0,
    0,
    176,
};

/******************************************************************************
 * Register: FPM_POOL2_INTR_STS
 ******************************************************************************/
const ru_reg_rec FPM_POOL2_INTR_STS_REG = 
{
    "POOL2_INTR_STS",
#if RU_INCLUDE_DESC
    "POOL2 Interrupt Status Register",
    "Interrupt bits are active high. When a bit in this register is set to 1 and the "
    "corresponding bit in interrupt mask register is set to 1, interrupt to CPU will "
    "occur. When set (1), interrupts bits can be cleared (0) by writing a 1 to the "
    "desired bit. ",
#endif
    FPM_POOL2_INTR_STS_REG_OFFSET,
    0,
    0,
    177,
};

/******************************************************************************
 * Register: FPM_POOL2_STALL_MSK
 ******************************************************************************/
const ru_reg_rec FPM_POOL2_STALL_MSK_REG = 
{
    "POOL2_STALL_MSK",
#if RU_INCLUDE_DESC
    "POOL2 Stall FPM mask",
    "Software sets desired stall bits that upon corresponding active interrupt status "
    "will stall FPM from new allocation, de-allocation, and mcast update process. Listed "
    "below are the supported interrupt statuses \n"
    "1. Invalid free token (bit[3] of interrupt status register 0x14)\n"
    "2. Invalid free token with index out-of-range (bit[4] of interrupt status register 0x14)\n"
    "3. Invalid mcast token (bit[5] of interrupt status register 0x14)\n"
    "4. Invalid mcast token with index out-of-range (bit[6] of interrupt status register 0x14)\n"
    "5. Memory corrupt status (bit[8] of interrupt status register 0x14)\n"
    "When state machine is stalled, registers and memory can still be accessed. Any new token "
    "allocation request will be serviced with valid tokens (if available in alloc cache) and invalid "
    "tokens (if alloc cache is empty). Any new de-allocation/mcast update requests will be either stored "
    "in de-allocation fifo (if there is space in free fifo) or dropped (if free fifo is full). "
    "Bit locations in this register matches the location of corrseponding interrupt status bits in "
    "register 0x14. To un-stall (enable) state machine interrupt status bits (in register 0x14) "
    "corresponding to these mask bits should be cleared. Stall mask bits are active high and "
    "are disabled by default. This is for debug purposes only. ",
#endif
    FPM_POOL2_STALL_MSK_REG_OFFSET,
    0,
    0,
    178,
};

/******************************************************************************
 * Register: FPM_POOL1_CFG1
 ******************************************************************************/
const ru_reg_rec FPM_POOL1_CFG1_REG = 
{
    "POOL1_CFG1",
#if RU_INCLUDE_DESC
    "POOL1 Configuration Register 1",
    "",
#endif
    FPM_POOL1_CFG1_REG_OFFSET,
    0,
    0,
    179,
};

/******************************************************************************
 * Register: FPM_POOL1_CFG2
 ******************************************************************************/
const ru_reg_rec FPM_POOL1_CFG2_REG = 
{
    "POOL1_CFG2",
#if RU_INCLUDE_DESC
    "POOL1 Configuration Register 2",
    "This register sets the physical base address of this memory. "
    "The memory block should be the number of buffers times "
    "the buffer size. This is mainly used for multi-pool memory "
    "configuration. NOTE: POOL_BASE_ADDRESS[7:2] and reserved[1:0] "
    "field must be written with 0x00 in the BCM3382 because its"
    "token-to-address converter assumes the buffers start on "
    "a 2kB boundary.",
#endif
    FPM_POOL1_CFG2_REG_OFFSET,
    0,
    0,
    180,
};

/******************************************************************************
 * Register: FPM_POOL1_CFG3
 ******************************************************************************/
const ru_reg_rec FPM_POOL1_CFG3_REG = 
{
    "POOL1_CFG3",
#if RU_INCLUDE_DESC
    "POOL1 Configuration Register 3",
    "This register sets the physical base address of this memory. "
    "The memory block should be the number of buffers times "
    "the buffer size. This is mainly used for multi-pool memory "
    "configuration. ",
#endif
    FPM_POOL1_CFG3_REG_OFFSET,
    0,
    0,
    181,
};

/******************************************************************************
 * Register: FPM_POOL1_STAT1
 ******************************************************************************/
const ru_reg_rec FPM_POOL1_STAT1_REG = 
{
    "POOL1_STAT1",
#if RU_INCLUDE_DESC
    "POOL2 Status Register 1",
    "This read only register allows software to read the count of "
    "free pool overflows and underflows. A overflow condition occurs when pool is empty, "
    "ie., no tokens are allocated and free/mcast request is encountered. A underflow "
    "condition occurs when pool is full, ie., there are no free tokens and a "
    "allocation request is encountered. When the counter values reaches maximum "
    "count, it will hold the max value and not increment the count "
    "value unless it is cleared. Any write to this register will clear both "
    "both counters. ",
#endif
    FPM_POOL1_STAT1_REG_OFFSET,
    0,
    0,
    182,
};

/******************************************************************************
 * Register: FPM_POOL1_STAT2
 ******************************************************************************/
const ru_reg_rec FPM_POOL1_STAT2_REG = 
{
    "POOL1_STAT2",
#if RU_INCLUDE_DESC
    "POOL2 Status Register 2",
    "This read only register provide status of index memory, "
    "alloc & free cache/fifos. These are real time statuses "
    "and bits are not sticky. Write to any bits will have no effect. ",
#endif
    FPM_POOL1_STAT2_REG_OFFSET,
    0,
    0,
    183,
};

/******************************************************************************
 * Register: FPM_POOL1_STAT3
 ******************************************************************************/
const ru_reg_rec FPM_POOL1_STAT3_REG = 
{
    "POOL1_STAT3",
#if RU_INCLUDE_DESC
    "POOL2 Status Register 3",
    "This read only register allows software to read the count of "
    "free token requests with in-valid tokens "
    "When the counter values reaches maximum "
    "count, it will hold the max value and not increment the count "
    "value unless it is cleared. Any write to this register will clear "
    "count value. ",
#endif
    FPM_POOL1_STAT3_REG_OFFSET,
    0,
    0,
    184,
};

/******************************************************************************
 * Register: FPM_POOL1_STAT4
 ******************************************************************************/
const ru_reg_rec FPM_POOL1_STAT4_REG = 
{
    "POOL1_STAT4",
#if RU_INCLUDE_DESC
    "POOL2 Status Register 4",
    "This read only register allows software to read the count of "
    "multi-cast token update requests with in-valid tokens. "
    "When the counter values reaches maximum "
    "count, it will hold the max value and not increment the count "
    "value unless it is cleared. Any write to this register will clear "
    "count value. ",
#endif
    FPM_POOL1_STAT4_REG_OFFSET,
    0,
    0,
    185,
};

/******************************************************************************
 * Register: FPM_POOL1_STAT5
 ******************************************************************************/
const ru_reg_rec FPM_POOL1_STAT5_REG = 
{
    "POOL1_STAT5",
#if RU_INCLUDE_DESC
    "POOL2 Status Register 5",
    "This read only register allows software to read the alloc token that "
    "causes memory corrupt interrupt (intr[8]) to go active. This is for debug "
    "purposes only. Any write to this register will clear "
    "token value (makes all bits zero). ",
#endif
    FPM_POOL1_STAT5_REG_OFFSET,
    0,
    0,
    186,
};

/******************************************************************************
 * Register: FPM_POOL1_STAT6
 ******************************************************************************/
const ru_reg_rec FPM_POOL1_STAT6_REG = 
{
    "POOL1_STAT6",
#if RU_INCLUDE_DESC
    "POOL2 Status Register 6",
    "This read only register allows software to read the free token that "
    "causes invalid free request or free token with index out-of-range interrupts "
    "(intr[3] or intr[4]) to go active. This is for debug "
    "purposes only. Any write to this register will clear "
    "token value (makes all bits zero). ",
#endif
    FPM_POOL1_STAT6_REG_OFFSET,
    0,
    0,
    187,
};

/******************************************************************************
 * Register: FPM_POOL1_STAT7
 ******************************************************************************/
const ru_reg_rec FPM_POOL1_STAT7_REG = 
{
    "POOL1_STAT7",
#if RU_INCLUDE_DESC
    "POOL2 Status Register 7",
    "This read only register allows software to read the multi-cast token that "
    "causes invalid mcast request or mcast token with index out-of-range interrupts "
    "(intr[5] or intr[6]) to go active. This is for debug "
    "purposes only. Any write to this register will clear "
    "token value (makes all bits zero). ",
#endif
    FPM_POOL1_STAT7_REG_OFFSET,
    0,
    0,
    188,
};

/******************************************************************************
 * Register: FPM_POOL1_STAT8
 ******************************************************************************/
const ru_reg_rec FPM_POOL1_STAT8_REG = 
{
    "POOL1_STAT8",
#if RU_INCLUDE_DESC
    "POOL2 Status Register 8",
    "This register allows software to read the lowest value "
    "the NUM_OF_TOKENS_AVAILABLE count reached since the last "
    "time it was cleared. Any write to this register will "
    "reset the value back to the maximum number of tokens (0x10000) ",
#endif
    FPM_POOL1_STAT8_REG_OFFSET,
    0,
    0,
    189,
};

/******************************************************************************
 * Register: FPM_POOL2_STAT1
 ******************************************************************************/
const ru_reg_rec FPM_POOL2_STAT1_REG = 
{
    "POOL2_STAT1",
#if RU_INCLUDE_DESC
    "POOL2 Status Register 1",
    "This read only register allows software to read the count of "
    "free pool overflows and underflows. A overflow condition occurs when pool is empty, "
    "ie., no tokens are allocated and free/mcast request is encountered. A underflow "
    "condition occurs when pool is full, ie., there are no free tokens and a "
    "allocation request is encountered. When the counter values reaches maximum "
    "count, it will hold the max value and not increment the count "
    "value unless it is cleared. Any write to this register will clear both "
    "both counters. ",
#endif
    FPM_POOL2_STAT1_REG_OFFSET,
    0,
    0,
    190,
};

/******************************************************************************
 * Register: FPM_POOL2_STAT2
 ******************************************************************************/
const ru_reg_rec FPM_POOL2_STAT2_REG = 
{
    "POOL2_STAT2",
#if RU_INCLUDE_DESC
    "POOL2 Status Register 2",
    "This read only register provide status of index memory, "
    "alloc & free cache/fifos. These are real time statuses "
    "and bits are not sticky. Write to any bits will have no effect. ",
#endif
    FPM_POOL2_STAT2_REG_OFFSET,
    0,
    0,
    191,
};

/******************************************************************************
 * Register: FPM_POOL2_STAT3
 ******************************************************************************/
const ru_reg_rec FPM_POOL2_STAT3_REG = 
{
    "POOL2_STAT3",
#if RU_INCLUDE_DESC
    "POOL2 Status Register 3",
    "This read only register allows software to read the count of "
    "free token requests with in-valid tokens "
    "When the counter values reaches maximum "
    "count, it will hold the max value and not increment the count "
    "value unless it is cleared. Any write to this register will clear "
    "count value. ",
#endif
    FPM_POOL2_STAT3_REG_OFFSET,
    0,
    0,
    192,
};

/******************************************************************************
 * Register: FPM_POOL2_STAT4
 ******************************************************************************/
const ru_reg_rec FPM_POOL2_STAT4_REG = 
{
    "POOL2_STAT4",
#if RU_INCLUDE_DESC
    "POOL2 Status Register 4",
    "This read only register allows software to read the count of "
    "multi-cast token update requests with in-valid tokens. "
    "When the counter values reaches maximum "
    "count, it will hold the max value and not increment the count "
    "value unless it is cleared. Any write to this register will clear "
    "count value. ",
#endif
    FPM_POOL2_STAT4_REG_OFFSET,
    0,
    0,
    193,
};

/******************************************************************************
 * Register: FPM_POOL2_STAT5
 ******************************************************************************/
const ru_reg_rec FPM_POOL2_STAT5_REG = 
{
    "POOL2_STAT5",
#if RU_INCLUDE_DESC
    "POOL2 Status Register 5",
    "This read only register allows software to read the alloc token that "
    "causes memory corrupt interrupt (intr[8]) to go active. This is for debug "
    "purposes only. Any write to this register will clear "
    "token value (makes all bits zero). ",
#endif
    FPM_POOL2_STAT5_REG_OFFSET,
    0,
    0,
    194,
};

/******************************************************************************
 * Register: FPM_POOL2_STAT6
 ******************************************************************************/
const ru_reg_rec FPM_POOL2_STAT6_REG = 
{
    "POOL2_STAT6",
#if RU_INCLUDE_DESC
    "POOL2 Status Register 6",
    "This read only register allows software to read the free token that "
    "causes invalid free request or free token with index out-of-range interrupts "
    "(intr[3] or intr[4]) to go active. This is for debug "
    "purposes only. Any write to this register will clear "
    "token value (makes all bits zero). ",
#endif
    FPM_POOL2_STAT6_REG_OFFSET,
    0,
    0,
    195,
};

/******************************************************************************
 * Register: FPM_POOL2_STAT7
 ******************************************************************************/
const ru_reg_rec FPM_POOL2_STAT7_REG = 
{
    "POOL2_STAT7",
#if RU_INCLUDE_DESC
    "POOL2 Status Register 7",
    "This read only register allows software to read the multi-cast token that "
    "causes invalid mcast request or mcast token with index out-of-range interrupts "
    "(intr[5] or intr[6]) to go active. This is for debug "
    "purposes only. Any write to this register will clear "
    "token value (makes all bits zero). ",
#endif
    FPM_POOL2_STAT7_REG_OFFSET,
    0,
    0,
    196,
};

/******************************************************************************
 * Register: FPM_POOL2_STAT8
 ******************************************************************************/
const ru_reg_rec FPM_POOL2_STAT8_REG = 
{
    "POOL2_STAT8",
#if RU_INCLUDE_DESC
    "POOL2 Status Register 8",
    "This register allows software to read the lowest value "
    "the NUM_OF_TOKENS_AVAILABLE count reached since the last "
    "time it was cleared. Any write to this register will "
    "reset the value back to the maximum number of tokens (0x10000) ",
#endif
    FPM_POOL2_STAT8_REG_OFFSET,
    0,
    0,
    197,
};

/******************************************************************************
 * Register: FPM_POOL1_XON_XOFF_CFG
 ******************************************************************************/
const ru_reg_rec FPM_POOL1_XON_XOFF_CFG_REG = 
{
    "POOL1_XON_XOFF_CFG",
#if RU_INCLUDE_DESC
    "POOL1 XON/XOFF Threshold Configuration Register",
    "",
#endif
    FPM_POOL1_XON_XOFF_CFG_REG_OFFSET,
    0,
    0,
    198,
};

/******************************************************************************
 * Register: FPM_FPM_NOT_EMPTY_CFG
 ******************************************************************************/
const ru_reg_rec FPM_FPM_NOT_EMPTY_CFG_REG = 
{
    "FPM_NOT_EMPTY_CFG",
#if RU_INCLUDE_DESC
    "FPM_NOT_EMPTY Threshold Configuration Register",
    "",
#endif
    FPM_FPM_NOT_EMPTY_CFG_REG_OFFSET,
    0,
    0,
    199,
};

/******************************************************************************
 * Register: FPM_MEM_CTL
 ******************************************************************************/
const ru_reg_rec FPM_MEM_CTL_REG = 
{
    "MEM_CTL",
#if RU_INCLUDE_DESC
    "Back door Memory Access Control Register",
    "",
#endif
    FPM_MEM_CTL_REG_OFFSET,
    0,
    0,
    200,
};

/******************************************************************************
 * Register: FPM_MEM_DATA1
 ******************************************************************************/
const ru_reg_rec FPM_MEM_DATA1_REG = 
{
    "MEM_DATA1",
#if RU_INCLUDE_DESC
    "Back door Memory Data1 Register",
    "",
#endif
    FPM_MEM_DATA1_REG_OFFSET,
    0,
    0,
    201,
};

/******************************************************************************
 * Register: FPM_MEM_DATA2
 ******************************************************************************/
const ru_reg_rec FPM_MEM_DATA2_REG = 
{
    "MEM_DATA2",
#if RU_INCLUDE_DESC
    "Back door Memory Data2 Register",
    "",
#endif
    FPM_MEM_DATA2_REG_OFFSET,
    0,
    0,
    202,
};

/******************************************************************************
 * Register: FPM_TOKEN_RECOVER_CTL
 ******************************************************************************/
const ru_reg_rec FPM_TOKEN_RECOVER_CTL_REG = 
{
    "TOKEN_RECOVER_CTL",
#if RU_INCLUDE_DESC
    "Token Recovery Control Register",
    "",
#endif
    FPM_TOKEN_RECOVER_CTL_REG_OFFSET,
    0,
    0,
    203,
};

/******************************************************************************
 * Register: FPM_SHORT_AGING_TIMER
 ******************************************************************************/
const ru_reg_rec FPM_SHORT_AGING_TIMER_REG = 
{
    "SHORT_AGING_TIMER",
#if RU_INCLUDE_DESC
    "Long Aging Timer",
    "",
#endif
    FPM_SHORT_AGING_TIMER_REG_OFFSET,
    0,
    0,
    204,
};

/******************************************************************************
 * Register: FPM_LONG_AGING_TIMER
 ******************************************************************************/
const ru_reg_rec FPM_LONG_AGING_TIMER_REG = 
{
    "LONG_AGING_TIMER",
#if RU_INCLUDE_DESC
    "Long Aging Timer",
    "",
#endif
    FPM_LONG_AGING_TIMER_REG_OFFSET,
    0,
    0,
    205,
};

/******************************************************************************
 * Register: FPM_CACHE_RECYCLE_TIMER
 ******************************************************************************/
const ru_reg_rec FPM_CACHE_RECYCLE_TIMER_REG = 
{
    "CACHE_RECYCLE_TIMER",
#if RU_INCLUDE_DESC
    "Token Cache Recycle Timer",
    "",
#endif
    FPM_CACHE_RECYCLE_TIMER_REG_OFFSET,
    0,
    0,
    206,
};

/******************************************************************************
 * Register: FPM_EXPIRED_TOKEN_COUNT_POOL1
 ******************************************************************************/
const ru_reg_rec FPM_EXPIRED_TOKEN_COUNT_POOL1_REG = 
{
    "EXPIRED_TOKEN_COUNT_POOL1",
#if RU_INCLUDE_DESC
    "Expired Token Count",
    "",
#endif
    FPM_EXPIRED_TOKEN_COUNT_POOL1_REG_OFFSET,
    0,
    0,
    207,
};

/******************************************************************************
 * Register: FPM_RECOVERED_TOKEN_COUNT_POOL1
 ******************************************************************************/
const ru_reg_rec FPM_RECOVERED_TOKEN_COUNT_POOL1_REG = 
{
    "RECOVERED_TOKEN_COUNT_POOL1",
#if RU_INCLUDE_DESC
    "Recovered Token Count",
    "",
#endif
    FPM_RECOVERED_TOKEN_COUNT_POOL1_REG_OFFSET,
    0,
    0,
    208,
};

/******************************************************************************
 * Register: FPM_EXPIRED_TOKEN_COUNT_POOL2
 ******************************************************************************/
const ru_reg_rec FPM_EXPIRED_TOKEN_COUNT_POOL2_REG = 
{
    "EXPIRED_TOKEN_COUNT_POOL2",
#if RU_INCLUDE_DESC
    "Expired Token Count",
    "",
#endif
    FPM_EXPIRED_TOKEN_COUNT_POOL2_REG_OFFSET,
    0,
    0,
    209,
};

/******************************************************************************
 * Register: FPM_RECOVERED_TOKEN_COUNT_POOL2
 ******************************************************************************/
const ru_reg_rec FPM_RECOVERED_TOKEN_COUNT_POOL2_REG = 
{
    "RECOVERED_TOKEN_COUNT_POOL2",
#if RU_INCLUDE_DESC
    "Recovered Token Count",
    "",
#endif
    FPM_RECOVERED_TOKEN_COUNT_POOL2_REG_OFFSET,
    0,
    0,
    210,
};

/******************************************************************************
 * Register: FPM_TOKEN_RECOVER_START_END_POOL1
 ******************************************************************************/
const ru_reg_rec FPM_TOKEN_RECOVER_START_END_POOL1_REG = 
{
    "TOKEN_RECOVER_START_END_POOL1",
#if RU_INCLUDE_DESC
    "Token Recovery Start/End Range",
    "",
#endif
    FPM_TOKEN_RECOVER_START_END_POOL1_REG_OFFSET,
    0,
    0,
    211,
};

/******************************************************************************
 * Register: FPM_TOKEN_RECOVER_START_END_POOL2
 ******************************************************************************/
const ru_reg_rec FPM_TOKEN_RECOVER_START_END_POOL2_REG = 
{
    "TOKEN_RECOVER_START_END_POOL2",
#if RU_INCLUDE_DESC
    "Token Recovery Start/End Range",
    "",
#endif
    FPM_TOKEN_RECOVER_START_END_POOL2_REG_OFFSET,
    0,
    0,
    212,
};

/******************************************************************************
 * Register: FPM_POOL1_ALLOC_DEALLOC
 ******************************************************************************/
const ru_reg_rec FPM_POOL1_ALLOC_DEALLOC_REG = 
{
    "POOL1_ALLOC_DEALLOC",
#if RU_INCLUDE_DESC
    "POOL4 Allocation & De-allocation/Free Management Register",
    "The free pool FIFO contains pointers to the buffers in the pool. "
    "To allocate a buffer from the pool, read "
    "token from this port. To de-allocate/free a buffer to the pool , write the token "
    "of the buffer to this port. After reset, software must initialize the "
    "FIFO. The buffer size is given in the control register above. All "
    "buffers must be of the same size and contiguous. ",
#endif
    FPM_POOL1_ALLOC_DEALLOC_REG_OFFSET,
    0,
    0,
    213,
};

/******************************************************************************
 * Register: FPM_POOL2_ALLOC_DEALLOC
 ******************************************************************************/
const ru_reg_rec FPM_POOL2_ALLOC_DEALLOC_REG = 
{
    "POOL2_ALLOC_DEALLOC",
#if RU_INCLUDE_DESC
    "POOL4 Allocation & De-allocation/Free Management Register",
    "The free pool FIFO contains pointers to the buffers in the pool. "
    "To allocate a buffer from the pool, read "
    "token from this port. To de-allocate/free a buffer to the pool , write the token "
    "of the buffer to this port. After reset, software must initialize the "
    "FIFO. The buffer size is given in the control register above. All "
    "buffers must be of the same size and contiguous. ",
#endif
    FPM_POOL2_ALLOC_DEALLOC_REG_OFFSET,
    0,
    0,
    214,
};

/******************************************************************************
 * Register: FPM_POOL3_ALLOC_DEALLOC
 ******************************************************************************/
const ru_reg_rec FPM_POOL3_ALLOC_DEALLOC_REG = 
{
    "POOL3_ALLOC_DEALLOC",
#if RU_INCLUDE_DESC
    "POOL4 Allocation & De-allocation/Free Management Register",
    "The free pool FIFO contains pointers to the buffers in the pool. "
    "To allocate a buffer from the pool, read "
    "token from this port. To de-allocate/free a buffer to the pool , write the token "
    "of the buffer to this port. After reset, software must initialize the "
    "FIFO. The buffer size is given in the control register above. All "
    "buffers must be of the same size and contiguous. ",
#endif
    FPM_POOL3_ALLOC_DEALLOC_REG_OFFSET,
    0,
    0,
    215,
};

/******************************************************************************
 * Register: FPM_POOL4_ALLOC_DEALLOC
 ******************************************************************************/
const ru_reg_rec FPM_POOL4_ALLOC_DEALLOC_REG = 
{
    "POOL4_ALLOC_DEALLOC",
#if RU_INCLUDE_DESC
    "POOL4 Allocation & De-allocation/Free Management Register",
    "The free pool FIFO contains pointers to the buffers in the pool. "
    "To allocate a buffer from the pool, read "
    "token from this port. To de-allocate/free a buffer to the pool , write the token "
    "of the buffer to this port. After reset, software must initialize the "
    "FIFO. The buffer size is given in the control register above. All "
    "buffers must be of the same size and contiguous. ",
#endif
    FPM_POOL4_ALLOC_DEALLOC_REG_OFFSET,
    0,
    0,
    216,
};

/******************************************************************************
 * Register: FPM_SPARE
 ******************************************************************************/
const ru_reg_rec FPM_SPARE_REG = 
{
    "SPARE",
#if RU_INCLUDE_DESC
    "Spare Register for future use",
    "",
#endif
    FPM_SPARE_REG_OFFSET,
    0,
    0,
    217,
};

/******************************************************************************
 * Register: FPM_POOL_MULTI
 ******************************************************************************/
const ru_reg_rec FPM_POOL_MULTI_REG = 
{
    "POOL_MULTI",
#if RU_INCLUDE_DESC
    "Multi-cast Token Update Control Register",
    "Update/Modify the multi-cast value of the token",
#endif
    FPM_POOL_MULTI_REG_OFFSET,
    0,
    0,
    218,
};

/******************************************************************************
 * Register: FPM_FPM_BB_FORCE
 ******************************************************************************/
const ru_reg_rec FPM_FPM_BB_FORCE_REG = 
{
    "FPM_BB_FORCE",
#if RU_INCLUDE_DESC
    "FPM_BB_FORCE Register",
    "Write this register to force FPM_BB transaction",
#endif
    FPM_FPM_BB_FORCE_REG_OFFSET,
    0,
    0,
    219,
};

/******************************************************************************
 * Register: FPM_FPM_BB_FORCED_CTRL
 ******************************************************************************/
const ru_reg_rec FPM_FPM_BB_FORCED_CTRL_REG = 
{
    "FPM_BB_FORCED_CTRL",
#if RU_INCLUDE_DESC
    "FPM_BB_FORCED_CTRL Register",
    "Control to be sent on forced transaction",
#endif
    FPM_FPM_BB_FORCED_CTRL_REG_OFFSET,
    0,
    0,
    220,
};

/******************************************************************************
 * Register: FPM_FPM_BB_FORCED_ADDR
 ******************************************************************************/
const ru_reg_rec FPM_FPM_BB_FORCED_ADDR_REG = 
{
    "FPM_BB_FORCED_ADDR",
#if RU_INCLUDE_DESC
    "FPM_BB_FORCED_ADDR Register",
    "Address to be sent on forced transaction",
#endif
    FPM_FPM_BB_FORCED_ADDR_REG_OFFSET,
    0,
    0,
    221,
};

/******************************************************************************
 * Register: FPM_FPM_BB_FORCED_DATA
 ******************************************************************************/
const ru_reg_rec FPM_FPM_BB_FORCED_DATA_REG = 
{
    "FPM_BB_FORCED_DATA",
#if RU_INCLUDE_DESC
    "FPM_BB_FORCED_DATA Register",
    "Data to be sent on forced transaction",
#endif
    FPM_FPM_BB_FORCED_DATA_REG_OFFSET,
    0,
    0,
    222,
};

/******************************************************************************
 * Register: FPM_FPM_BB_DECODE_CFG
 ******************************************************************************/
const ru_reg_rec FPM_FPM_BB_DECODE_CFG_REG = 
{
    "FPM_BB_DECODE_CFG",
#if RU_INCLUDE_DESC
    "FPM_BB_DECODE_CFG Register",
    "set configuration for BB decoder",
#endif
    FPM_FPM_BB_DECODE_CFG_REG_OFFSET,
    0,
    0,
    223,
};

/******************************************************************************
 * Register: FPM_FPM_BB_DBG_CFG
 ******************************************************************************/
const ru_reg_rec FPM_FPM_BB_DBG_CFG_REG = 
{
    "FPM_BB_DBG_CFG",
#if RU_INCLUDE_DESC
    "FPM_BB_DBG_CFG Register",
    "Set SW addr to read FPM_BB FIFOs",
#endif
    FPM_FPM_BB_DBG_CFG_REG_OFFSET,
    0,
    0,
    224,
};

/******************************************************************************
 * Register: FPM_FPM_BB_DBG_RXFIFO_STS
 ******************************************************************************/
const ru_reg_rec FPM_FPM_BB_DBG_RXFIFO_STS_REG = 
{
    "FPM_BB_DBG_RXFIFO_STS",
#if RU_INCLUDE_DESC
    "FPM_BB_DBG_RXFIFO_STS Register",
    "Status of FPM BB RXFIFO",
#endif
    FPM_FPM_BB_DBG_RXFIFO_STS_REG_OFFSET,
    0,
    0,
    225,
};

/******************************************************************************
 * Register: FPM_FPM_BB_DBG_TXFIFO_STS
 ******************************************************************************/
const ru_reg_rec FPM_FPM_BB_DBG_TXFIFO_STS_REG = 
{
    "FPM_BB_DBG_TXFIFO_STS",
#if RU_INCLUDE_DESC
    "FPM_BB_DBG_TXFIFO_STS Register",
    "Status of FPM BB TXFIFO",
#endif
    FPM_FPM_BB_DBG_TXFIFO_STS_REG_OFFSET,
    0,
    0,
    226,
};

/******************************************************************************
 * Register: FPM_FPM_BB_DBG_RXFIFO_DATA1
 ******************************************************************************/
const ru_reg_rec FPM_FPM_BB_DBG_RXFIFO_DATA1_REG = 
{
    "FPM_BB_DBG_RXFIFO_DATA1",
#if RU_INCLUDE_DESC
    "FPM_BB_DBG_RXFIFO_DATA1 Register",
    "Data from FPM BB RXFIFO bits [31:0]",
#endif
    FPM_FPM_BB_DBG_RXFIFO_DATA1_REG_OFFSET,
    0,
    0,
    227,
};

/******************************************************************************
 * Register: FPM_FPM_BB_DBG_RXFIFO_DATA2
 ******************************************************************************/
const ru_reg_rec FPM_FPM_BB_DBG_RXFIFO_DATA2_REG = 
{
    "FPM_BB_DBG_RXFIFO_DATA2",
#if RU_INCLUDE_DESC
    "FPM_BB_DBG_RXFIFO_DATA2 Register",
    "Data from FPM BB RXFIFO bits [39:32]",
#endif
    FPM_FPM_BB_DBG_RXFIFO_DATA2_REG_OFFSET,
    0,
    0,
    228,
};

/******************************************************************************
 * Register: FPM_FPM_BB_DBG_TXFIFO_DATA1
 ******************************************************************************/
const ru_reg_rec FPM_FPM_BB_DBG_TXFIFO_DATA1_REG = 
{
    "FPM_BB_DBG_TXFIFO_DATA1",
#if RU_INCLUDE_DESC
    "FPM_BB_DBG_TXFIFO_DATA1 Register",
    "Data from FPM BB TXFIFO bits [31:0]",
#endif
    FPM_FPM_BB_DBG_TXFIFO_DATA1_REG_OFFSET,
    0,
    0,
    229,
};

/******************************************************************************
 * Register: FPM_FPM_BB_DBG_TXFIFO_DATA2
 ******************************************************************************/
const ru_reg_rec FPM_FPM_BB_DBG_TXFIFO_DATA2_REG = 
{
    "FPM_BB_DBG_TXFIFO_DATA2",
#if RU_INCLUDE_DESC
    "FPM_BB_DBG_TXFIFO_DATA2 Register",
    "Data from FPM BB TXFIFO bits [63:32]",
#endif
    FPM_FPM_BB_DBG_TXFIFO_DATA2_REG_OFFSET,
    0,
    0,
    230,
};

/******************************************************************************
 * Register: FPM_FPM_BB_DBG_TXFIFO_DATA3
 ******************************************************************************/
const ru_reg_rec FPM_FPM_BB_DBG_TXFIFO_DATA3_REG = 
{
    "FPM_BB_DBG_TXFIFO_DATA3",
#if RU_INCLUDE_DESC
    "FPM_BB_DBG_TXFIFO_DATA3 Register",
    "Data from FPM BB TXFIFO bits [79:64]",
#endif
    FPM_FPM_BB_DBG_TXFIFO_DATA3_REG_OFFSET,
    0,
    0,
    231,
};

/******************************************************************************
 * Block: FPM
 ******************************************************************************/
static const ru_reg_rec *FPM_REGS[] =
{
    &FPM_FPM_CTL_REG,
    &FPM_FPM_CFG1_REG,
    &FPM_FPM_WEIGHT_REG,
    &FPM_FPM_BB_CFG_REG,
    &FPM_POOL1_INTR_MSK_REG,
    &FPM_POOL1_INTR_STS_REG,
    &FPM_POOL1_STALL_MSK_REG,
    &FPM_POOL2_INTR_MSK_REG,
    &FPM_POOL2_INTR_STS_REG,
    &FPM_POOL2_STALL_MSK_REG,
    &FPM_POOL1_CFG1_REG,
    &FPM_POOL1_CFG2_REG,
    &FPM_POOL1_CFG3_REG,
    &FPM_POOL1_STAT1_REG,
    &FPM_POOL1_STAT2_REG,
    &FPM_POOL1_STAT3_REG,
    &FPM_POOL1_STAT4_REG,
    &FPM_POOL1_STAT5_REG,
    &FPM_POOL1_STAT6_REG,
    &FPM_POOL1_STAT7_REG,
    &FPM_POOL1_STAT8_REG,
    &FPM_POOL2_STAT1_REG,
    &FPM_POOL2_STAT2_REG,
    &FPM_POOL2_STAT3_REG,
    &FPM_POOL2_STAT4_REG,
    &FPM_POOL2_STAT5_REG,
    &FPM_POOL2_STAT6_REG,
    &FPM_POOL2_STAT7_REG,
    &FPM_POOL2_STAT8_REG,
    &FPM_POOL1_XON_XOFF_CFG_REG,
    &FPM_FPM_NOT_EMPTY_CFG_REG,
    &FPM_MEM_CTL_REG,
    &FPM_MEM_DATA1_REG,
    &FPM_MEM_DATA2_REG,
    &FPM_TOKEN_RECOVER_CTL_REG,
    &FPM_SHORT_AGING_TIMER_REG,
    &FPM_LONG_AGING_TIMER_REG,
    &FPM_CACHE_RECYCLE_TIMER_REG,
    &FPM_EXPIRED_TOKEN_COUNT_POOL1_REG,
    &FPM_RECOVERED_TOKEN_COUNT_POOL1_REG,
    &FPM_EXPIRED_TOKEN_COUNT_POOL2_REG,
    &FPM_RECOVERED_TOKEN_COUNT_POOL2_REG,
    &FPM_TOKEN_RECOVER_START_END_POOL1_REG,
    &FPM_TOKEN_RECOVER_START_END_POOL2_REG,
    &FPM_POOL1_ALLOC_DEALLOC_REG,
    &FPM_POOL2_ALLOC_DEALLOC_REG,
    &FPM_POOL3_ALLOC_DEALLOC_REG,
    &FPM_POOL4_ALLOC_DEALLOC_REG,
    &FPM_SPARE_REG,
    &FPM_POOL_MULTI_REG,
    &FPM_FPM_BB_FORCE_REG,
    &FPM_FPM_BB_FORCED_CTRL_REG,
    &FPM_FPM_BB_FORCED_ADDR_REG,
    &FPM_FPM_BB_FORCED_DATA_REG,
    &FPM_FPM_BB_DECODE_CFG_REG,
    &FPM_FPM_BB_DBG_CFG_REG,
    &FPM_FPM_BB_DBG_RXFIFO_STS_REG,
    &FPM_FPM_BB_DBG_TXFIFO_STS_REG,
    &FPM_FPM_BB_DBG_RXFIFO_DATA1_REG,
    &FPM_FPM_BB_DBG_RXFIFO_DATA2_REG,
    &FPM_FPM_BB_DBG_TXFIFO_DATA1_REG,
    &FPM_FPM_BB_DBG_TXFIFO_DATA2_REG,
    &FPM_FPM_BB_DBG_TXFIFO_DATA3_REG,
};

unsigned long FPM_ADDRS[] =
{
    0x82a00000,
};

const ru_block_rec FPM_BLOCK = 
{
    "FPM",
    FPM_ADDRS,
    1,
    63,
    FPM_REGS
};

/* End of file XRDP_FPM.c */
