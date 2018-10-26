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
 * Register: DMA_CONFIG_BBROUTEOVRD
 ******************************************************************************/
const ru_reg_rec DMA_CONFIG_BBROUTEOVRD_REG = 
{
    "CONFIG_BBROUTEOVRD",
#if RU_INCLUDE_DESC
    "BB_ROUTE_OVERRIDE Register",
    "Broadbus route override",
#endif
    DMA_CONFIG_BBROUTEOVRD_REG_OFFSET,
    0,
    0,
    633,
};

/******************************************************************************
 * Register: DMA_CONFIG_NUM_OF_WRITES
 ******************************************************************************/
const ru_reg_rec DMA_CONFIG_NUM_OF_WRITES_REG = 
{
    "CONFIG_NUM_OF_WRITES",
#if RU_INCLUDE_DESC
    "NUM_OF_WRITE_REQ %i Register",
    "This array of registers defines the memory allocation for the peripherals, for upstream."
    "The allocation is of number of 128byte buffers out of the total 48 buffers for both sdma and dma."
    ""
    "The allocation is done by defining a only the number of allocated buffers. base address is calculated by HW, when base of peripheral 0 is 0."
    "Note that the memory allocation should not contain wrap around."
    "The number of allocated CDs is the same of data buffers."
    "",
#endif
    DMA_CONFIG_NUM_OF_WRITES_REG_OFFSET,
    DMA_CONFIG_NUM_OF_WRITES_REG_RAM_CNT,
    4,
    634,
};

/******************************************************************************
 * Register: DMA_CONFIG_NUM_OF_READS
 ******************************************************************************/
const ru_reg_rec DMA_CONFIG_NUM_OF_READS_REG = 
{
    "CONFIG_NUM_OF_READS",
#if RU_INCLUDE_DESC
    "NUM_OF_READ_REQ %i Register",
    "This array of registers controls the number of read requests of each peripheral within the read requests RAM."
    "total of 64 requests are divided between peripherals."
    "Base address of peripheral 0 is 0, base of peripheral 1 is 0 + periph0_num_of_read_requests and so on.",
#endif
    DMA_CONFIG_NUM_OF_READS_REG_OFFSET,
    DMA_CONFIG_NUM_OF_READS_REG_RAM_CNT,
    4,
    635,
};

/******************************************************************************
 * Register: DMA_CONFIG_U_THRESH
 ******************************************************************************/
const ru_reg_rec DMA_CONFIG_U_THRESH_REG = 
{
    "CONFIG_U_THRESH",
#if RU_INCLUDE_DESC
    "URGENT_THRESHOLDS %i Register",
    "the in/out of urgent thresholds mark the number of write requests in the queue in which the peripherals priority is changed. The two thresholds should create hysteresis."
    "The moving into urgent threshold must always be greater than the moving out of urgent threshold.",
#endif
    DMA_CONFIG_U_THRESH_REG_OFFSET,
    DMA_CONFIG_U_THRESH_REG_RAM_CNT,
    4,
    636,
};

/******************************************************************************
 * Register: DMA_CONFIG_PRI
 ******************************************************************************/
const ru_reg_rec DMA_CONFIG_PRI_REG = 
{
    "CONFIG_PRI",
#if RU_INCLUDE_DESC
    "STRICT_PRIORITY %i Register",
    "The arbitration between the requests of the different peripherals is done in two stages:"
    "1. Strict priority - chooses the peripherals with the highest priority among all perpherals who have a request pending."
    "2. Weighted Round-Robin between all peripherals with the same priority."
    ""
    "This array of registers allow configuration of the priority of each peripheral (both rx and tx) in the following manner:"
    "There are 4 levels of priorities, when each bit in the register represents a different level of priority. One should assert the relevant bit according to the desired priority -"
    "For the lowest  - 0001"
    "For the highest - 1000",
#endif
    DMA_CONFIG_PRI_REG_OFFSET,
    DMA_CONFIG_PRI_REG_RAM_CNT,
    4,
    637,
};

/******************************************************************************
 * Register: DMA_CONFIG_PERIPH_SOURCE
 ******************************************************************************/
const ru_reg_rec DMA_CONFIG_PERIPH_SOURCE_REG = 
{
    "CONFIG_PERIPH_SOURCE",
#if RU_INCLUDE_DESC
    "BB_SOURCE_DMA_PERIPH %i Register",
    "Broadbus source address of the DMA peripherals. Register per peripheral (rx and tx). The source is used to determine the route address to the different peripherals.",
#endif
    DMA_CONFIG_PERIPH_SOURCE_REG_OFFSET,
    DMA_CONFIG_PERIPH_SOURCE_REG_RAM_CNT,
    4,
    638,
};

/******************************************************************************
 * Register: DMA_CONFIG_WEIGHT
 ******************************************************************************/
const ru_reg_rec DMA_CONFIG_WEIGHT_REG = 
{
    "CONFIG_WEIGHT",
#if RU_INCLUDE_DESC
    "WEIGHT_OF_ROUND_ROBIN %i Register",
    "The second phase of the arbitration between requests is weighted round robin between requests of peripherals with the same priority."
    "This array of registers allow configurtion of the weight of each peripheral (rx and tx). The actual weight will be weight + 1, meaning configuration of 0 is actual weight of 1.",
#endif
    DMA_CONFIG_WEIGHT_REG_OFFSET,
    DMA_CONFIG_WEIGHT_REG_RAM_CNT,
    4,
    639,
};

/******************************************************************************
 * Register: DMA_CONFIG_PTRRST
 ******************************************************************************/
const ru_reg_rec DMA_CONFIG_PTRRST_REG = 
{
    "CONFIG_PTRRST",
#if RU_INCLUDE_DESC
    "POINTERS_RESET Register",
    "Resets the pointers of the peripherals FIFOs within the DMA. Bit per peripheral side (rx and tx)."
    "For rx side resets the data and CD FIFOs."
    "For tx side resets the read requests FIFO.",
#endif
    DMA_CONFIG_PTRRST_REG_OFFSET,
    0,
    0,
    640,
};

/******************************************************************************
 * Register: DMA_CONFIG_MAX_OTF
 ******************************************************************************/
const ru_reg_rec DMA_CONFIG_MAX_OTF_REG = 
{
    "CONFIG_MAX_OTF",
#if RU_INCLUDE_DESC
    "MAX_ON_THE_FLY Register",
    "max number of on the fly read commands the DMA may issue to DDR before receiving any data.",
#endif
    DMA_CONFIG_MAX_OTF_REG_OFFSET,
    0,
    0,
    641,
};

/******************************************************************************
 * Register: DMA_CONFIG_CLK_GATE_CNTRL
 ******************************************************************************/
const ru_reg_rec DMA_CONFIG_CLK_GATE_CNTRL_REG = 
{
    "CONFIG_CLK_GATE_CNTRL",
#if RU_INCLUDE_DESC
    "CLOCK_GATE_CONTROL Register",
    "Clock Gate control register including timer config and bypass control",
#endif
    DMA_CONFIG_CLK_GATE_CNTRL_REG_OFFSET,
    0,
    0,
    642,
};

/******************************************************************************
 * Register: DMA_CONFIG_DBG_SEL
 ******************************************************************************/
const ru_reg_rec DMA_CONFIG_DBG_SEL_REG = 
{
    "CONFIG_DBG_SEL",
#if RU_INCLUDE_DESC
    "DBG_SEL Register",
    "debug bus select",
#endif
    DMA_CONFIG_DBG_SEL_REG_OFFSET,
    0,
    0,
    643,
};

/******************************************************************************
 * Register: DMA_DEBUG_NEMPTY
 ******************************************************************************/
const ru_reg_rec DMA_DEBUG_NEMPTY_REG = 
{
    "DEBUG_NEMPTY",
#if RU_INCLUDE_DESC
    "NOT_EMPTY_VECTOR Register",
    "Each peripheral is represented in a bit on the not empty vector."
    "LSB is for rx peripherals, MSB for tx peripherals."
    "If the bit is asserted, the requests queue of the relevant peripheral is not empty."
    "The not empty vector is used by the DMA scheduler to determine which peripheral is the next to be served.",
#endif
    DMA_DEBUG_NEMPTY_REG_OFFSET,
    0,
    0,
    644,
};

/******************************************************************************
 * Register: DMA_DEBUG_URGNT
 ******************************************************************************/
const ru_reg_rec DMA_DEBUG_URGNT_REG = 
{
    "DEBUG_URGNT",
#if RU_INCLUDE_DESC
    "URGENT_VECTOR Register",
    "Each peripheral, a is represented in a bit on the urgent vector. 8 LSB are rx peripherlas, 8 MSB are tx peripherals."
    "If the bit is asserted, the requests queue of the relevant peripheral is in urgent state."
    "The urgent vector is used by the DMA scheduler to determine which peripheral is the next to be served.",
#endif
    DMA_DEBUG_URGNT_REG_OFFSET,
    0,
    0,
    645,
};

/******************************************************************************
 * Register: DMA_DEBUG_SELSRC
 ******************************************************************************/
const ru_reg_rec DMA_DEBUG_SELSRC_REG = 
{
    "DEBUG_SELSRC",
#if RU_INCLUDE_DESC
    "SELECTED_SOURCE_NUM Register",
    "The decision of the dma schedule rand the next peripheral to be served, represented by its source address",
#endif
    DMA_DEBUG_SELSRC_REG_OFFSET,
    0,
    0,
    646,
};

/******************************************************************************
 * Register: DMA_DEBUG_REQ_CNT_RX
 ******************************************************************************/
const ru_reg_rec DMA_DEBUG_REQ_CNT_RX_REG = 
{
    "DEBUG_REQ_CNT_RX",
#if RU_INCLUDE_DESC
    "REQUEST_COUNTERS_RX %i Register",
    "the number of write requests currently pending for each rx peripheral.",
#endif
    DMA_DEBUG_REQ_CNT_RX_REG_OFFSET,
    DMA_DEBUG_REQ_CNT_RX_REG_RAM_CNT,
    4,
    647,
};

/******************************************************************************
 * Register: DMA_DEBUG_REQ_CNT_TX
 ******************************************************************************/
const ru_reg_rec DMA_DEBUG_REQ_CNT_TX_REG = 
{
    "DEBUG_REQ_CNT_TX",
#if RU_INCLUDE_DESC
    "REQUEST_COUNTERS_TX %i Register",
    "the number of read requestscurrently pending for each TX peripheral.",
#endif
    DMA_DEBUG_REQ_CNT_TX_REG_OFFSET,
    DMA_DEBUG_REQ_CNT_TX_REG_RAM_CNT,
    4,
    648,
};

/******************************************************************************
 * Register: DMA_DEBUG_REQ_CNT_RX_ACC
 ******************************************************************************/
const ru_reg_rec DMA_DEBUG_REQ_CNT_RX_ACC_REG = 
{
    "DEBUG_REQ_CNT_RX_ACC",
#if RU_INCLUDE_DESC
    "ACC_REQUEST_COUNTERS_RX %i Register",
    "the accumulated number of write requests served so far for each peripheral. Wrap around on max value, not read clear.",
#endif
    DMA_DEBUG_REQ_CNT_RX_ACC_REG_OFFSET,
    DMA_DEBUG_REQ_CNT_RX_ACC_REG_RAM_CNT,
    4,
    649,
};

/******************************************************************************
 * Register: DMA_DEBUG_REQ_CNT_TX_ACC
 ******************************************************************************/
const ru_reg_rec DMA_DEBUG_REQ_CNT_TX_ACC_REG = 
{
    "DEBUG_REQ_CNT_TX_ACC",
#if RU_INCLUDE_DESC
    "ACC_REQUEST_COUNTERS_TX %i Register",
    "the accumulated number of read requests served so far for each peripheral. Wrap around on max value, not read clear.",
#endif
    DMA_DEBUG_REQ_CNT_TX_ACC_REG_OFFSET,
    DMA_DEBUG_REQ_CNT_TX_ACC_REG_RAM_CNT,
    4,
    650,
};

/******************************************************************************
 * Register: DMA_DEBUG_RDADD
 ******************************************************************************/
const ru_reg_rec DMA_DEBUG_RDADD_REG = 
{
    "DEBUG_RDADD",
#if RU_INCLUDE_DESC
    "RAM_ADDRES Register",
    "the address and cs of the ram the user wishes to read using the indirect access read mechanism.",
#endif
    DMA_DEBUG_RDADD_REG_OFFSET,
    0,
    0,
    651,
};

/******************************************************************************
 * Register: DMA_DEBUG_RDVALID
 ******************************************************************************/
const ru_reg_rec DMA_DEBUG_RDVALID_REG = 
{
    "DEBUG_RDVALID",
#if RU_INCLUDE_DESC
    "INDIRECT_READ_REQUEST_VALID Register",
    "After determining the address and cs, the user should assert this bit for indicating that the address and cs are valid.",
#endif
    DMA_DEBUG_RDVALID_REG_OFFSET,
    0,
    0,
    652,
};

/******************************************************************************
 * Register: DMA_DEBUG_RDDATA
 ******************************************************************************/
const ru_reg_rec DMA_DEBUG_RDDATA_REG = 
{
    "DEBUG_RDDATA",
#if RU_INCLUDE_DESC
    "INDIRECT_READ_DATA %i Register",
    "The returned read data from the selected RAM. Array of 4 registers (128 bits total)."
    "The width of the different memories is as follows:"
    "write data - 128 bits"
    "chunk descriptors - 36 bits"
    "read requests - 42 bits"
    "read data - 64 bits"
    ""
    "The the memories with width smaller than 128, the data will appear in the first registers of the array, for example:"
    "data from the cd RAM will appear in - {reg1[5:0], reg0[31:0]}.",
#endif
    DMA_DEBUG_RDDATA_REG_OFFSET,
    DMA_DEBUG_RDDATA_REG_RAM_CNT,
    4,
    653,
};

/******************************************************************************
 * Register: DMA_DEBUG_RDDATARDY
 ******************************************************************************/
const ru_reg_rec DMA_DEBUG_RDDATARDY_REG = 
{
    "DEBUG_RDDATARDY",
#if RU_INCLUDE_DESC
    "READ_DATA_READY Register",
    "When assertd indicats that the data in the previous array is valid.Willremain asserted until the user deasserts the valid bit in regiser RDVALID.",
#endif
    DMA_DEBUG_RDDATARDY_REG_OFFSET,
    0,
    0,
    654,
};

/******************************************************************************
 * Block: DMA
 ******************************************************************************/
static const ru_reg_rec *DMA_REGS[] =
{
    &DMA_CONFIG_BBROUTEOVRD_REG,
    &DMA_CONFIG_NUM_OF_WRITES_REG,
    &DMA_CONFIG_NUM_OF_READS_REG,
    &DMA_CONFIG_U_THRESH_REG,
    &DMA_CONFIG_PRI_REG,
    &DMA_CONFIG_PERIPH_SOURCE_REG,
    &DMA_CONFIG_WEIGHT_REG,
    &DMA_CONFIG_PTRRST_REG,
    &DMA_CONFIG_MAX_OTF_REG,
    &DMA_CONFIG_CLK_GATE_CNTRL_REG,
    &DMA_CONFIG_DBG_SEL_REG,
    &DMA_DEBUG_NEMPTY_REG,
    &DMA_DEBUG_URGNT_REG,
    &DMA_DEBUG_SELSRC_REG,
    &DMA_DEBUG_REQ_CNT_RX_REG,
    &DMA_DEBUG_REQ_CNT_TX_REG,
    &DMA_DEBUG_REQ_CNT_RX_ACC_REG,
    &DMA_DEBUG_REQ_CNT_TX_ACC_REG,
    &DMA_DEBUG_RDADD_REG,
    &DMA_DEBUG_RDVALID_REG,
    &DMA_DEBUG_RDDATA_REG,
    &DMA_DEBUG_RDDATARDY_REG,
};

unsigned long DMA_ADDRS[] =
{
    0x82d98800,
    0x82d98c00,
    0x82d99000,
};

const ru_block_rec DMA_BLOCK = 
{
    "DMA",
    DMA_ADDRS,
    3,
    22,
    DMA_REGS
};

/* End of file XRDP_DMA.c */
