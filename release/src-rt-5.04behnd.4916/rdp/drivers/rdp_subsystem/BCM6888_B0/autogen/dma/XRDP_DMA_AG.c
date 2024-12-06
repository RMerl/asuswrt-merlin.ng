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


#include "XRDP_DMA_AG.h"

/******************************************************************************
 * Register: NAME: DMA_CONFIG_NUM_OF_WRITES, TYPE: Type_DMA_DMA_CONFIG_NUM_OF_WRITES
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: NUMOFBUFF *****/
const ru_field_rec DMA_CONFIG_NUM_OF_WRITES_NUMOFBUFF_FIELD =
{
    "NUMOFBUFF",
#if RU_INCLUDE_DESC
    "",
    "the number of 128bytes buffers allocated to the peripheral.\n\n",
#endif
    { DMA_CONFIG_NUM_OF_WRITES_NUMOFBUFF_FIELD_MASK },
    0,
    { DMA_CONFIG_NUM_OF_WRITES_NUMOFBUFF_FIELD_WIDTH },
    { DMA_CONFIG_NUM_OF_WRITES_NUMOFBUFF_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DMA_CONFIG_NUM_OF_WRITES_FIELDS[] =
{
    &DMA_CONFIG_NUM_OF_WRITES_NUMOFBUFF_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DMA_CONFIG_NUM_OF_WRITES *****/
const ru_reg_rec DMA_CONFIG_NUM_OF_WRITES_REG =
{
    "CONFIG_NUM_OF_WRITES",
#if RU_INCLUDE_DESC
    "NUM_OF_WRITE_REQ 0..7 Register",
    "This array of registers defines the memory allocation for the peripherals, for upstream.\nThe allocation is of number of 128byte buffers out of the total 32 buffers for both sdma and dma.\n\nThe allocation is done by defining a only the number of allocated buffers. base address is calculated by HW, when base of peripheral 0 is 0.\nNote that the memory allocation should not contain wrap around.\nThe number of allocated CDs is the same of data buffers.\n\n",
#endif
    { DMA_CONFIG_NUM_OF_WRITES_REG_OFFSET },
    DMA_CONFIG_NUM_OF_WRITES_REG_RAM_CNT,
    4,
    250,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DMA_CONFIG_NUM_OF_WRITES_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DMA_CONFIG_NUM_OF_READS, TYPE: Type_DMA_DMA_CONFIG_NUM_OF_READS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: RR_NUM *****/
const ru_field_rec DMA_CONFIG_NUM_OF_READS_RR_NUM_FIELD =
{
    "RR_NUM",
#if RU_INCLUDE_DESC
    "",
    "number of read requests\n",
#endif
    { DMA_CONFIG_NUM_OF_READS_RR_NUM_FIELD_MASK },
    0,
    { DMA_CONFIG_NUM_OF_READS_RR_NUM_FIELD_WIDTH },
    { DMA_CONFIG_NUM_OF_READS_RR_NUM_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DMA_CONFIG_NUM_OF_READS_FIELDS[] =
{
    &DMA_CONFIG_NUM_OF_READS_RR_NUM_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DMA_CONFIG_NUM_OF_READS *****/
const ru_reg_rec DMA_CONFIG_NUM_OF_READS_REG =
{
    "CONFIG_NUM_OF_READS",
#if RU_INCLUDE_DESC
    "NUM_OF_READ_REQ 0..7 Register",
    "This array of registers controls the number of read requests of each peripheral within the read requests RAM.\ntotal of 64 requests are divided between peripherals.\nBase address of peripheral 0 is 0, base of peripheral 1 is 0 + periph0_num_of_read_requests and so on.\n",
#endif
    { DMA_CONFIG_NUM_OF_READS_REG_OFFSET },
    DMA_CONFIG_NUM_OF_READS_REG_RAM_CNT,
    4,
    251,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DMA_CONFIG_NUM_OF_READS_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DMA_CONFIG_U_THRESH, TYPE: Type_DMA_DMA_CONFIG_U_THRESH
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: INTO_U *****/
const ru_field_rec DMA_CONFIG_U_THRESH_INTO_U_FIELD =
{
    "INTO_U",
#if RU_INCLUDE_DESC
    "",
    "moving into urgent threshold\n",
#endif
    { DMA_CONFIG_U_THRESH_INTO_U_FIELD_MASK },
    0,
    { DMA_CONFIG_U_THRESH_INTO_U_FIELD_WIDTH },
    { DMA_CONFIG_U_THRESH_INTO_U_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: OUT_OF_U *****/
const ru_field_rec DMA_CONFIG_U_THRESH_OUT_OF_U_FIELD =
{
    "OUT_OF_U",
#if RU_INCLUDE_DESC
    "",
    "moving out ot urgent threshold\n",
#endif
    { DMA_CONFIG_U_THRESH_OUT_OF_U_FIELD_MASK },
    0,
    { DMA_CONFIG_U_THRESH_OUT_OF_U_FIELD_WIDTH },
    { DMA_CONFIG_U_THRESH_OUT_OF_U_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DMA_CONFIG_U_THRESH_FIELDS[] =
{
    &DMA_CONFIG_U_THRESH_INTO_U_FIELD,
    &DMA_CONFIG_U_THRESH_OUT_OF_U_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DMA_CONFIG_U_THRESH *****/
const ru_reg_rec DMA_CONFIG_U_THRESH_REG =
{
    "CONFIG_U_THRESH",
#if RU_INCLUDE_DESC
    "URGENT_THRESHOLDS 0..7 Register",
    "the in/out of urgent thresholds mark the number of write requests in the queue in which the peripherals priority is changed. The two thresholds should create hysteresis.\nThe moving into urgent threshold must always be greater than the moving out of urgent threshold.\n",
#endif
    { DMA_CONFIG_U_THRESH_REG_OFFSET },
    DMA_CONFIG_U_THRESH_REG_RAM_CNT,
    4,
    252,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    DMA_CONFIG_U_THRESH_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DMA_CONFIG_PRI, TYPE: Type_DMA_DMA_CONFIG_PRI
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: RXPRI *****/
const ru_field_rec DMA_CONFIG_PRI_RXPRI_FIELD =
{
    "RXPRI",
#if RU_INCLUDE_DESC
    "",
    "priority of rx side (upload) of the peripheral\n",
#endif
    { DMA_CONFIG_PRI_RXPRI_FIELD_MASK },
    0,
    { DMA_CONFIG_PRI_RXPRI_FIELD_WIDTH },
    { DMA_CONFIG_PRI_RXPRI_FIELD_SHIFT },
    8,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: TXPRI *****/
const ru_field_rec DMA_CONFIG_PRI_TXPRI_FIELD =
{
    "TXPRI",
#if RU_INCLUDE_DESC
    "",
    "priority of tx side (download) of the peripheral\n",
#endif
    { DMA_CONFIG_PRI_TXPRI_FIELD_MASK },
    0,
    { DMA_CONFIG_PRI_TXPRI_FIELD_WIDTH },
    { DMA_CONFIG_PRI_TXPRI_FIELD_SHIFT },
    8,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DMA_CONFIG_PRI_FIELDS[] =
{
    &DMA_CONFIG_PRI_RXPRI_FIELD,
    &DMA_CONFIG_PRI_TXPRI_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DMA_CONFIG_PRI *****/
const ru_reg_rec DMA_CONFIG_PRI_REG =
{
    "CONFIG_PRI",
#if RU_INCLUDE_DESC
    "STRICT_PRIORITY 0..7 Register",
    "The arbitration between the requests of the different peripherals is done in two stages:\n1. Strict priority - chooses the peripherals with the highest priority among all perpherals who have a request pending.\n2. Weighted Round-Robin between all peripherals with the same priority.\n\nThis array of registers allow configuration of the priority of each peripheral (both rx and tx) in the following manner:\nThere are 4 levels of priorities, when each bit in the register represents a different level of priority. One should assert the relevant bit according to the desired priority -\nFor the lowest  - 0001\nFor the highest - 1000\n",
#endif
    { DMA_CONFIG_PRI_REG_OFFSET },
    DMA_CONFIG_PRI_REG_RAM_CNT,
    4,
    253,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    DMA_CONFIG_PRI_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DMA_CONFIG_WEIGHT, TYPE: Type_DMA_DMA_CONFIG_WEIGHT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: RXWEIGHT *****/
const ru_field_rec DMA_CONFIG_WEIGHT_RXWEIGHT_FIELD =
{
    "RXWEIGHT",
#if RU_INCLUDE_DESC
    "",
    "weight of rx side (upload) of the peripheral\n",
#endif
    { DMA_CONFIG_WEIGHT_RXWEIGHT_FIELD_MASK },
    0,
    { DMA_CONFIG_WEIGHT_RXWEIGHT_FIELD_WIDTH },
    { DMA_CONFIG_WEIGHT_RXWEIGHT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: TXWEIGHT *****/
const ru_field_rec DMA_CONFIG_WEIGHT_TXWEIGHT_FIELD =
{
    "TXWEIGHT",
#if RU_INCLUDE_DESC
    "",
    "weight of tx side (download) of the peripheral\n",
#endif
    { DMA_CONFIG_WEIGHT_TXWEIGHT_FIELD_MASK },
    0,
    { DMA_CONFIG_WEIGHT_TXWEIGHT_FIELD_WIDTH },
    { DMA_CONFIG_WEIGHT_TXWEIGHT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DMA_CONFIG_WEIGHT_FIELDS[] =
{
    &DMA_CONFIG_WEIGHT_RXWEIGHT_FIELD,
    &DMA_CONFIG_WEIGHT_TXWEIGHT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DMA_CONFIG_WEIGHT *****/
const ru_reg_rec DMA_CONFIG_WEIGHT_REG =
{
    "CONFIG_WEIGHT",
#if RU_INCLUDE_DESC
    "WEIGHT_OF_ROUND_ROBIN 0..7 Register",
    "The second phase of the arbitration between requests is weighted round robin between requests of peripherals with the same priority.\nThis array of registers allow configurtion of the weight of each peripheral (rx and tx). The actual weight will be weight + 1, meaning configuration of 0 is actual weight of 1.\n",
#endif
    { DMA_CONFIG_WEIGHT_REG_OFFSET },
    DMA_CONFIG_WEIGHT_REG_RAM_CNT,
    4,
    254,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    DMA_CONFIG_WEIGHT_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DMA_CONFIG_PERIPH_SOURCE, TYPE: Type_DMA_DMA_CONFIG_PERIPH_SOURCE
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: RXSOURCE *****/
const ru_field_rec DMA_CONFIG_PERIPH_SOURCE_RXSOURCE_FIELD =
{
    "RXSOURCE",
#if RU_INCLUDE_DESC
    "",
    "bb source of rx side (upload) of the peripheral\n",
#endif
    { DMA_CONFIG_PERIPH_SOURCE_RXSOURCE_FIELD_MASK },
    0,
    { DMA_CONFIG_PERIPH_SOURCE_RXSOURCE_FIELD_WIDTH },
    { DMA_CONFIG_PERIPH_SOURCE_RXSOURCE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: TXSOURCE *****/
const ru_field_rec DMA_CONFIG_PERIPH_SOURCE_TXSOURCE_FIELD =
{
    "TXSOURCE",
#if RU_INCLUDE_DESC
    "",
    "bb source of tx side (download) of the peripheral\n",
#endif
    { DMA_CONFIG_PERIPH_SOURCE_TXSOURCE_FIELD_MASK },
    0,
    { DMA_CONFIG_PERIPH_SOURCE_TXSOURCE_FIELD_WIDTH },
    { DMA_CONFIG_PERIPH_SOURCE_TXSOURCE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DMA_CONFIG_PERIPH_SOURCE_FIELDS[] =
{
    &DMA_CONFIG_PERIPH_SOURCE_RXSOURCE_FIELD,
    &DMA_CONFIG_PERIPH_SOURCE_TXSOURCE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DMA_CONFIG_PERIPH_SOURCE *****/
const ru_reg_rec DMA_CONFIG_PERIPH_SOURCE_REG =
{
    "CONFIG_PERIPH_SOURCE",
#if RU_INCLUDE_DESC
    "BB_SOURCE_DMA_PERIPH 0..7 Register",
    "Broadbus source address of the DMA peripherals. Register per peripheral (rx and tx). The source is used to determine the route address to the different peripherals.\n",
#endif
    { DMA_CONFIG_PERIPH_SOURCE_REG_OFFSET },
    DMA_CONFIG_PERIPH_SOURCE_REG_RAM_CNT,
    4,
    255,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    DMA_CONFIG_PERIPH_SOURCE_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DMA_CONFIG_TARGET_MEM, TYPE: Type_DMA_DMA_CONFIG_TARGET_MEM
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: RXTMEM *****/
const ru_field_rec DMA_CONFIG_TARGET_MEM_RXTMEM_FIELD =
{
    "RXTMEM",
#if RU_INCLUDE_DESC
    "",
    "target memory of rx side (upload) of the peripheral\n",
#endif
    { DMA_CONFIG_TARGET_MEM_RXTMEM_FIELD_MASK },
    0,
    { DMA_CONFIG_TARGET_MEM_RXTMEM_FIELD_WIDTH },
    { DMA_CONFIG_TARGET_MEM_RXTMEM_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: TXTMEM *****/
const ru_field_rec DMA_CONFIG_TARGET_MEM_TXTMEM_FIELD =
{
    "TXTMEM",
#if RU_INCLUDE_DESC
    "",
    "target memory of tx side (download) of the peripheral\n",
#endif
    { DMA_CONFIG_TARGET_MEM_TXTMEM_FIELD_MASK },
    0,
    { DMA_CONFIG_TARGET_MEM_TXTMEM_FIELD_WIDTH },
    { DMA_CONFIG_TARGET_MEM_TXTMEM_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DMA_CONFIG_TARGET_MEM_FIELDS[] =
{
    &DMA_CONFIG_TARGET_MEM_RXTMEM_FIELD,
    &DMA_CONFIG_TARGET_MEM_TXTMEM_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DMA_CONFIG_TARGET_MEM *****/
const ru_reg_rec DMA_CONFIG_TARGET_MEM_REG =
{
    "CONFIG_TARGET_MEM",
#if RU_INCLUDE_DESC
    "TARGET_MEMORY 0..7 Register",
    "Configuration that determines whether the peripheral accesses the DDR or PSRAM.\n",
#endif
    { DMA_CONFIG_TARGET_MEM_REG_OFFSET },
    DMA_CONFIG_TARGET_MEM_REG_RAM_CNT,
    4,
    256,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    DMA_CONFIG_TARGET_MEM_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DMA_CONFIG_PTRRST, TYPE: Type_DMA_DMA_CONFIG_PTRRST
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: RSTVEC *****/
const ru_field_rec DMA_CONFIG_PTRRST_RSTVEC_FIELD =
{
    "RSTVEC",
#if RU_INCLUDE_DESC
    "",
    "vector in which each bit represents a peripheral.\nLSB represent RX peripherals and MSB represent TX peripherals.\nWhen asserted, the relevant FIFOS of the selected peripheral will be reset to zero\n",
#endif
    { DMA_CONFIG_PTRRST_RSTVEC_FIELD_MASK },
    0,
    { DMA_CONFIG_PTRRST_RSTVEC_FIELD_WIDTH },
    { DMA_CONFIG_PTRRST_RSTVEC_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DMA_CONFIG_PTRRST_FIELDS[] =
{
    &DMA_CONFIG_PTRRST_RSTVEC_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DMA_CONFIG_PTRRST *****/
const ru_reg_rec DMA_CONFIG_PTRRST_REG =
{
    "CONFIG_PTRRST",
#if RU_INCLUDE_DESC
    "POINTERS_RESET Register",
    "Resets the pointers of the peripherals FIFOs within the DMA. Bit per peripheral side (rx and tx).\nFor rx side resets the data and CD FIFOs.\nFor tx side resets the read requests FIFO.\n",
#endif
    { DMA_CONFIG_PTRRST_REG_OFFSET },
    0,
    0,
    257,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DMA_CONFIG_PTRRST_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DMA_CONFIG_BBROUTEOVRD, TYPE: Type_DMA_DMA_CONFIG_BBROUTEOVRD
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DEST *****/
const ru_field_rec DMA_CONFIG_BBROUTEOVRD_DEST_FIELD =
{
    "DEST",
#if RU_INCLUDE_DESC
    "",
    "destination ID\n",
#endif
    { DMA_CONFIG_BBROUTEOVRD_DEST_FIELD_MASK },
    0,
    { DMA_CONFIG_BBROUTEOVRD_DEST_FIELD_WIDTH },
    { DMA_CONFIG_BBROUTEOVRD_DEST_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ROUTE *****/
const ru_field_rec DMA_CONFIG_BBROUTEOVRD_ROUTE_FIELD =
{
    "ROUTE",
#if RU_INCLUDE_DESC
    "",
    "the route to be used (override the default route)\n",
#endif
    { DMA_CONFIG_BBROUTEOVRD_ROUTE_FIELD_MASK },
    0,
    { DMA_CONFIG_BBROUTEOVRD_ROUTE_FIELD_WIDTH },
    { DMA_CONFIG_BBROUTEOVRD_ROUTE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: OVRD *****/
const ru_field_rec DMA_CONFIG_BBROUTEOVRD_OVRD_FIELD =
{
    "OVRD",
#if RU_INCLUDE_DESC
    "",
    "override enable\n",
#endif
    { DMA_CONFIG_BBROUTEOVRD_OVRD_FIELD_MASK },
    0,
    { DMA_CONFIG_BBROUTEOVRD_OVRD_FIELD_WIDTH },
    { DMA_CONFIG_BBROUTEOVRD_OVRD_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DMA_CONFIG_BBROUTEOVRD_FIELDS[] =
{
    &DMA_CONFIG_BBROUTEOVRD_DEST_FIELD,
    &DMA_CONFIG_BBROUTEOVRD_ROUTE_FIELD,
    &DMA_CONFIG_BBROUTEOVRD_OVRD_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DMA_CONFIG_BBROUTEOVRD *****/
const ru_reg_rec DMA_CONFIG_BBROUTEOVRD_REG =
{
    "CONFIG_BBROUTEOVRD",
#if RU_INCLUDE_DESC
    "BB_ROUTE_OVERRIDE Register",
    "Broadbus route override\n",
#endif
    { DMA_CONFIG_BBROUTEOVRD_REG_OFFSET },
    0,
    0,
    258,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    DMA_CONFIG_BBROUTEOVRD_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DMA_CONFIG_CLK_GATE_CNTRL, TYPE: Type_DMA_DMA_CONFIG_CLK_GATE_CNTRL
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: BYPASS_CLK_GATE *****/
const ru_field_rec DMA_CONFIG_CLK_GATE_CNTRL_BYPASS_CLK_GATE_FIELD =
{
    "BYPASS_CLK_GATE",
#if RU_INCLUDE_DESC
    "",
    "If set to 1b1 will disable the clock gate logic such to always enable the clock\n",
#endif
    { DMA_CONFIG_CLK_GATE_CNTRL_BYPASS_CLK_GATE_FIELD_MASK },
    0,
    { DMA_CONFIG_CLK_GATE_CNTRL_BYPASS_CLK_GATE_FIELD_WIDTH },
    { DMA_CONFIG_CLK_GATE_CNTRL_BYPASS_CLK_GATE_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: TIMER_VAL *****/
const ru_field_rec DMA_CONFIG_CLK_GATE_CNTRL_TIMER_VAL_FIELD =
{
    "TIMER_VAL",
#if RU_INCLUDE_DESC
    "",
    "For how long should the clock stay active once all conditions for clock disable are met.\n\n\n",
#endif
    { DMA_CONFIG_CLK_GATE_CNTRL_TIMER_VAL_FIELD_MASK },
    0,
    { DMA_CONFIG_CLK_GATE_CNTRL_TIMER_VAL_FIELD_WIDTH },
    { DMA_CONFIG_CLK_GATE_CNTRL_TIMER_VAL_FIELD_SHIFT },
    15,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: KEEP_ALIVE_EN *****/
const ru_field_rec DMA_CONFIG_CLK_GATE_CNTRL_KEEP_ALIVE_EN_FIELD =
{
    "KEEP_ALIVE_EN",
#if RU_INCLUDE_DESC
    "",
    "Enables the keep alive logic which will periodically enable the clock to assure that no deadlock of clock being removed completely will occur\n",
#endif
    { DMA_CONFIG_CLK_GATE_CNTRL_KEEP_ALIVE_EN_FIELD_MASK },
    0,
    { DMA_CONFIG_CLK_GATE_CNTRL_KEEP_ALIVE_EN_FIELD_WIDTH },
    { DMA_CONFIG_CLK_GATE_CNTRL_KEEP_ALIVE_EN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: KEEP_ALIVE_INTRVL *****/
const ru_field_rec DMA_CONFIG_CLK_GATE_CNTRL_KEEP_ALIVE_INTRVL_FIELD =
{
    "KEEP_ALIVE_INTRVL",
#if RU_INCLUDE_DESC
    "",
    "If the KEEP alive option is enabled the field will determine for how many cycles should the clock be active\n",
#endif
    { DMA_CONFIG_CLK_GATE_CNTRL_KEEP_ALIVE_INTRVL_FIELD_MASK },
    0,
    { DMA_CONFIG_CLK_GATE_CNTRL_KEEP_ALIVE_INTRVL_FIELD_WIDTH },
    { DMA_CONFIG_CLK_GATE_CNTRL_KEEP_ALIVE_INTRVL_FIELD_SHIFT },
    2,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: KEEP_ALIVE_CYC *****/
const ru_field_rec DMA_CONFIG_CLK_GATE_CNTRL_KEEP_ALIVE_CYC_FIELD =
{
    "KEEP_ALIVE_CYC",
#if RU_INCLUDE_DESC
    "",
    "If the KEEP alive option is enabled this field will determine for how many cycles should the clock be disabled (minus the KEEP_ALIVE_INTERVAL)\n\nSo KEEP_ALIVE_CYCLE must be larger than KEEP_ALIVE_INTERVAL.\n",
#endif
    { DMA_CONFIG_CLK_GATE_CNTRL_KEEP_ALIVE_CYC_FIELD_MASK },
    0,
    { DMA_CONFIG_CLK_GATE_CNTRL_KEEP_ALIVE_CYC_FIELD_WIDTH },
    { DMA_CONFIG_CLK_GATE_CNTRL_KEEP_ALIVE_CYC_FIELD_SHIFT },
    15,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DMA_CONFIG_CLK_GATE_CNTRL_FIELDS[] =
{
    &DMA_CONFIG_CLK_GATE_CNTRL_BYPASS_CLK_GATE_FIELD,
    &DMA_CONFIG_CLK_GATE_CNTRL_TIMER_VAL_FIELD,
    &DMA_CONFIG_CLK_GATE_CNTRL_KEEP_ALIVE_EN_FIELD,
    &DMA_CONFIG_CLK_GATE_CNTRL_KEEP_ALIVE_INTRVL_FIELD,
    &DMA_CONFIG_CLK_GATE_CNTRL_KEEP_ALIVE_CYC_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DMA_CONFIG_CLK_GATE_CNTRL *****/
const ru_reg_rec DMA_CONFIG_CLK_GATE_CNTRL_REG =
{
    "CONFIG_CLK_GATE_CNTRL",
#if RU_INCLUDE_DESC
    "CLOCK_GATE_CONTROL Register",
    "Clock Gate control register including timer config and bypass control\n",
#endif
    { DMA_CONFIG_CLK_GATE_CNTRL_REG_OFFSET },
    0,
    0,
    259,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    5,
    DMA_CONFIG_CLK_GATE_CNTRL_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DMA_CONFIG_UBUS_DPIDS, TYPE: Type_DMA_DMA_CONFIG_UBUS_DPIDS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DDR *****/
const ru_field_rec DMA_CONFIG_UBUS_DPIDS_DDR_FIELD =
{
    "DDR",
#if RU_INCLUDE_DESC
    "",
    "DDR destination port-id\n",
#endif
    { DMA_CONFIG_UBUS_DPIDS_DDR_FIELD_MASK },
    0,
    { DMA_CONFIG_UBUS_DPIDS_DDR_FIELD_WIDTH },
    { DMA_CONFIG_UBUS_DPIDS_DDR_FIELD_SHIFT },
    2,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: SRAM *****/
const ru_field_rec DMA_CONFIG_UBUS_DPIDS_SRAM_FIELD =
{
    "SRAM",
#if RU_INCLUDE_DESC
    "",
    "SRAM destination port id\n",
#endif
    { DMA_CONFIG_UBUS_DPIDS_SRAM_FIELD_MASK },
    0,
    { DMA_CONFIG_UBUS_DPIDS_SRAM_FIELD_WIDTH },
    { DMA_CONFIG_UBUS_DPIDS_SRAM_FIELD_SHIFT },
    11,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DMA_CONFIG_UBUS_DPIDS_FIELDS[] =
{
    &DMA_CONFIG_UBUS_DPIDS_DDR_FIELD,
    &DMA_CONFIG_UBUS_DPIDS_SRAM_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DMA_CONFIG_UBUS_DPIDS *****/
const ru_reg_rec DMA_CONFIG_UBUS_DPIDS_REG =
{
    "CONFIG_UBUS_DPIDS",
#if RU_INCLUDE_DESC
    "UBUS_DEST_PORT_IDS Register",
    "UBUS Destination port-ids of the DDR and PSRAM\n",
#endif
    { DMA_CONFIG_UBUS_DPIDS_REG_OFFSET },
    0,
    0,
    260,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    DMA_CONFIG_UBUS_DPIDS_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DMA_CONFIG_MAX_OTF_BYTES, TYPE: Type_DMA_DMA_CONFIG_MAX_OTF_BYTES
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: MAX_DDR *****/
const ru_field_rec DMA_CONFIG_MAX_OTF_BYTES_MAX_DDR_FIELD =
{
    "MAX_DDR",
#if RU_INCLUDE_DESC
    "",
    "max on the fly bytes for DDR\n",
#endif
    { DMA_CONFIG_MAX_OTF_BYTES_MAX_DDR_FIELD_MASK },
    0,
    { DMA_CONFIG_MAX_OTF_BYTES_MAX_DDR_FIELD_WIDTH },
    { DMA_CONFIG_MAX_OTF_BYTES_MAX_DDR_FIELD_SHIFT },
    65535,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: MAX_SRAM *****/
const ru_field_rec DMA_CONFIG_MAX_OTF_BYTES_MAX_SRAM_FIELD =
{
    "MAX_SRAM",
#if RU_INCLUDE_DESC
    "",
    "max on the fly bytes for SRAM\n",
#endif
    { DMA_CONFIG_MAX_OTF_BYTES_MAX_SRAM_FIELD_MASK },
    0,
    { DMA_CONFIG_MAX_OTF_BYTES_MAX_SRAM_FIELD_WIDTH },
    { DMA_CONFIG_MAX_OTF_BYTES_MAX_SRAM_FIELD_SHIFT },
    65535,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DMA_CONFIG_MAX_OTF_BYTES_FIELDS[] =
{
    &DMA_CONFIG_MAX_OTF_BYTES_MAX_DDR_FIELD,
    &DMA_CONFIG_MAX_OTF_BYTES_MAX_SRAM_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DMA_CONFIG_MAX_OTF_BYTES *****/
const ru_reg_rec DMA_CONFIG_MAX_OTF_BYTES_REG =
{
    "CONFIG_MAX_OTF_BYTES",
#if RU_INCLUDE_DESC
    "MAX_BYTES_ON_THE_FLY Register",
    "The UBUS port does not issue read transaction unless there is space for the read data in the repin FIFO.\nIn order not to cause Head-of-line blocking when DDR transactions fill the repin FIFO and PSRAM transaction are blocked, this configuration limit the number of read data bytes that can issued on the fly.\n",
#endif
    { DMA_CONFIG_MAX_OTF_BYTES_REG_OFFSET },
    0,
    0,
    261,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    DMA_CONFIG_MAX_OTF_BYTES_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DMA_CONFIG_UBUS_CREDITS, TYPE: Type_DMA_DMA_CONFIG_UBUS_CREDITS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DDR *****/
const ru_field_rec DMA_CONFIG_UBUS_CREDITS_DDR_FIELD =
{
    "DDR",
#if RU_INCLUDE_DESC
    "",
    "DDR_credits\n",
#endif
    { DMA_CONFIG_UBUS_CREDITS_DDR_FIELD_MASK },
    0,
    { DMA_CONFIG_UBUS_CREDITS_DDR_FIELD_WIDTH },
    { DMA_CONFIG_UBUS_CREDITS_DDR_FIELD_SHIFT },
    4,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: SRAM *****/
const ru_field_rec DMA_CONFIG_UBUS_CREDITS_SRAM_FIELD =
{
    "SRAM",
#if RU_INCLUDE_DESC
    "",
    "SRAM_credits\n",
#endif
    { DMA_CONFIG_UBUS_CREDITS_SRAM_FIELD_MASK },
    0,
    { DMA_CONFIG_UBUS_CREDITS_SRAM_FIELD_WIDTH },
    { DMA_CONFIG_UBUS_CREDITS_SRAM_FIELD_SHIFT },
    4,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DDR_SET *****/
const ru_field_rec DMA_CONFIG_UBUS_CREDITS_DDR_SET_FIELD =
{
    "DDR_SET",
#if RU_INCLUDE_DESC
    "",
    "set the DMA DDR credits counter to the number in DDR_credits field\n",
#endif
    { DMA_CONFIG_UBUS_CREDITS_DDR_SET_FIELD_MASK },
    0,
    { DMA_CONFIG_UBUS_CREDITS_DDR_SET_FIELD_WIDTH },
    { DMA_CONFIG_UBUS_CREDITS_DDR_SET_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: SRAM_SET *****/
const ru_field_rec DMA_CONFIG_UBUS_CREDITS_SRAM_SET_FIELD =
{
    "SRAM_SET",
#if RU_INCLUDE_DESC
    "",
    "set the DMA SRAM credits counter to the number in SRAM_credits field\n",
#endif
    { DMA_CONFIG_UBUS_CREDITS_SRAM_SET_FIELD_MASK },
    0,
    { DMA_CONFIG_UBUS_CREDITS_SRAM_SET_FIELD_WIDTH },
    { DMA_CONFIG_UBUS_CREDITS_SRAM_SET_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DSPACE_FULL_THRSH *****/
const ru_field_rec DMA_CONFIG_UBUS_CREDITS_DSPACE_FULL_THRSH_FIELD =
{
    "DSPACE_FULL_THRSH",
#if RU_INCLUDE_DESC
    "",
    "in ccb mode, when not working with credits, this register determines the minimal number of dspaces to be considered not full full\n",
#endif
    { DMA_CONFIG_UBUS_CREDITS_DSPACE_FULL_THRSH_FIELD_MASK },
    0,
    { DMA_CONFIG_UBUS_CREDITS_DSPACE_FULL_THRSH_FIELD_WIDTH },
    { DMA_CONFIG_UBUS_CREDITS_DSPACE_FULL_THRSH_FIELD_SHIFT },
    16,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: HSPACE_FULL_THRSH *****/
const ru_field_rec DMA_CONFIG_UBUS_CREDITS_HSPACE_FULL_THRSH_FIELD =
{
    "HSPACE_FULL_THRSH",
#if RU_INCLUDE_DESC
    "",
    "in ccb mode, when not working with credits, this register determines the minimal number of hspaces to be considered not full full\n",
#endif
    { DMA_CONFIG_UBUS_CREDITS_HSPACE_FULL_THRSH_FIELD_MASK },
    0,
    { DMA_CONFIG_UBUS_CREDITS_HSPACE_FULL_THRSH_FIELD_WIDTH },
    { DMA_CONFIG_UBUS_CREDITS_HSPACE_FULL_THRSH_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DDRCH *****/
const ru_field_rec DMA_CONFIG_UBUS_CREDITS_DDRCH_FIELD =
{
    "DDRCH",
#if RU_INCLUDE_DESC
    "",
    "determines the UBUS channel in which the DDR traffic is using in ccb mode\n",
#endif
    { DMA_CONFIG_UBUS_CREDITS_DDRCH_FIELD_MASK },
    0,
    { DMA_CONFIG_UBUS_CREDITS_DDRCH_FIELD_WIDTH },
    { DMA_CONFIG_UBUS_CREDITS_DDRCH_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DMA_CONFIG_UBUS_CREDITS_FIELDS[] =
{
    &DMA_CONFIG_UBUS_CREDITS_DDR_FIELD,
    &DMA_CONFIG_UBUS_CREDITS_SRAM_FIELD,
    &DMA_CONFIG_UBUS_CREDITS_DDR_SET_FIELD,
    &DMA_CONFIG_UBUS_CREDITS_SRAM_SET_FIELD,
    &DMA_CONFIG_UBUS_CREDITS_DSPACE_FULL_THRSH_FIELD,
    &DMA_CONFIG_UBUS_CREDITS_HSPACE_FULL_THRSH_FIELD,
    &DMA_CONFIG_UBUS_CREDITS_DDRCH_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DMA_CONFIG_UBUS_CREDITS *****/
const ru_reg_rec DMA_CONFIG_UBUS_CREDITS_REG =
{
    "CONFIG_UBUS_CREDITS",
#if RU_INCLUDE_DESC
    "UBUS_CREDITS Register",
    "Number of initial UBUS credits for DDR and SRAM transactions.\n",
#endif
    { DMA_CONFIG_UBUS_CREDITS_REG_OFFSET },
    0,
    0,
    262,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    7,
    DMA_CONFIG_UBUS_CREDITS_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DMA_CONFIG_PSRAM_BASE, TYPE: Type_DMA_DMA_CONFIG_PSRAM_BASE
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: BASE *****/
const ru_field_rec DMA_CONFIG_PSRAM_BASE_BASE_FIELD =
{
    "BASE",
#if RU_INCLUDE_DESC
    "",
    "base address\n",
#endif
    { DMA_CONFIG_PSRAM_BASE_BASE_FIELD_MASK },
    0,
    { DMA_CONFIG_PSRAM_BASE_BASE_FIELD_WIDTH },
    { DMA_CONFIG_PSRAM_BASE_BASE_FIELD_SHIFT },
    8519680,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DMA_CONFIG_PSRAM_BASE_FIELDS[] =
{
    &DMA_CONFIG_PSRAM_BASE_BASE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DMA_CONFIG_PSRAM_BASE *****/
const ru_reg_rec DMA_CONFIG_PSRAM_BASE_REG =
{
    "CONFIG_PSRAM_BASE",
#if RU_INCLUDE_DESC
    "PSRAM_BASE Register",
    "the base address of the PSRAM n the chip address space in 256byte resolution.\n\n",
#endif
    { DMA_CONFIG_PSRAM_BASE_REG_OFFSET },
    0,
    0,
    263,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DMA_CONFIG_PSRAM_BASE_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DMA_CONFIG_DDR_BASE, TYPE: Type_DMA_DMA_CONFIG_DDR_BASE
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: BASE *****/
const ru_field_rec DMA_CONFIG_DDR_BASE_BASE_FIELD =
{
    "BASE",
#if RU_INCLUDE_DESC
    "",
    "base address\n",
#endif
    { DMA_CONFIG_DDR_BASE_BASE_FIELD_MASK },
    0,
    { DMA_CONFIG_DDR_BASE_BASE_FIELD_WIDTH },
    { DMA_CONFIG_DDR_BASE_BASE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DMA_CONFIG_DDR_BASE_FIELDS[] =
{
    &DMA_CONFIG_DDR_BASE_BASE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DMA_CONFIG_DDR_BASE *****/
const ru_reg_rec DMA_CONFIG_DDR_BASE_REG =
{
    "CONFIG_DDR_BASE",
#if RU_INCLUDE_DESC
    "DDR_BASE Register",
    "the base address of the DDR n the chip address space in 256byte resolution.\n\n",
#endif
    { DMA_CONFIG_DDR_BASE_REG_OFFSET },
    0,
    0,
    264,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DMA_CONFIG_DDR_BASE_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DMA_CONFIG_GEN_CFG, TYPE: Type_DMA_DMA_CONFIG_GEN_CFG
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: PAIR256 *****/
const ru_field_rec DMA_CONFIG_GEN_CFG_PAIR256_FIELD =
{
    "PAIR256",
#if RU_INCLUDE_DESC
    "",
    "when asserted the DMA will try to pair 2 read commands of the same client to the DDR. The commands will be issued back to back if they are to the DDR and if the first is not eop.\n",
#endif
    { DMA_CONFIG_GEN_CFG_PAIR256_FIELD_MASK },
    0,
    { DMA_CONFIG_GEN_CFG_PAIR256_FIELD_WIDTH },
    { DMA_CONFIG_GEN_CFG_PAIR256_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: R *****/
const ru_field_rec DMA_CONFIG_GEN_CFG_R_FIELD =
{
    "R",
#if RU_INCLUDE_DESC
    "",
    "default\n",
#endif
    { DMA_CONFIG_GEN_CFG_R_FIELD_MASK },
    0,
    { DMA_CONFIG_GEN_CFG_R_FIELD_WIDTH },
    { DMA_CONFIG_GEN_CFG_R_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DMA_CONFIG_GEN_CFG_FIELDS[] =
{
    &DMA_CONFIG_GEN_CFG_PAIR256_FIELD,
    &DMA_CONFIG_GEN_CFG_R_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DMA_CONFIG_GEN_CFG *****/
const ru_reg_rec DMA_CONFIG_GEN_CFG_REG =
{
    "CONFIG_GEN_CFG",
#if RU_INCLUDE_DESC
    "GENERAL_CONFIG Register",
    "general configurations\n",
#endif
    { DMA_CONFIG_GEN_CFG_REG_OFFSET },
    0,
    0,
    265,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    DMA_CONFIG_GEN_CFG_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DMA_CONFIG_MAX_OTF_REQ, TYPE: Type_DMA_DMA_CONFIG_MAX_OTF_REQ
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: MAX_DDR *****/
const ru_field_rec DMA_CONFIG_MAX_OTF_REQ_MAX_DDR_FIELD =
{
    "MAX_DDR",
#if RU_INCLUDE_DESC
    "",
    "max on the fly req for DDR\n",
#endif
    { DMA_CONFIG_MAX_OTF_REQ_MAX_DDR_FIELD_MASK },
    0,
    { DMA_CONFIG_MAX_OTF_REQ_MAX_DDR_FIELD_WIDTH },
    { DMA_CONFIG_MAX_OTF_REQ_MAX_DDR_FIELD_SHIFT },
    26,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: MAX_SRAM *****/
const ru_field_rec DMA_CONFIG_MAX_OTF_REQ_MAX_SRAM_FIELD =
{
    "MAX_SRAM",
#if RU_INCLUDE_DESC
    "",
    "max on the fly req for SRAM\n",
#endif
    { DMA_CONFIG_MAX_OTF_REQ_MAX_SRAM_FIELD_MASK },
    0,
    { DMA_CONFIG_MAX_OTF_REQ_MAX_SRAM_FIELD_WIDTH },
    { DMA_CONFIG_MAX_OTF_REQ_MAX_SRAM_FIELD_SHIFT },
    63,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DMA_CONFIG_MAX_OTF_REQ_FIELDS[] =
{
    &DMA_CONFIG_MAX_OTF_REQ_MAX_DDR_FIELD,
    &DMA_CONFIG_MAX_OTF_REQ_MAX_SRAM_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DMA_CONFIG_MAX_OTF_REQ *****/
const ru_reg_rec DMA_CONFIG_MAX_OTF_REQ_REG =
{
    "CONFIG_MAX_OTF_REQ",
#if RU_INCLUDE_DESC
    "MAX_REQ_ON_THE_FLY Register",
    "The UBUS port does not issue read or write reply transaction unless there is space for the read data or write reply in the repin header FIFO.\nIn order not to cause Head-of-line blocking when DDR transactions fill the repin header FIFO and PSRAM transaction are blocked, this configuration limit the number of on the fly commands that can be issued.\n",
#endif
    { DMA_CONFIG_MAX_OTF_REQ_REG_OFFSET },
    0,
    0,
    266,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    DMA_CONFIG_MAX_OTF_REQ_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DMA_DEBUG_NEMPTY, TYPE: Type_DMA_DMA_DEBUG_NEMPTY
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: NEMPTY *****/
const ru_field_rec DMA_DEBUG_NEMPTY_NEMPTY_FIELD =
{
    "NEMPTY",
#if RU_INCLUDE_DESC
    "",
    "indication of the queue state\n",
#endif
    { DMA_DEBUG_NEMPTY_NEMPTY_FIELD_MASK },
    0,
    { DMA_DEBUG_NEMPTY_NEMPTY_FIELD_WIDTH },
    { DMA_DEBUG_NEMPTY_NEMPTY_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DMA_DEBUG_NEMPTY_FIELDS[] =
{
    &DMA_DEBUG_NEMPTY_NEMPTY_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DMA_DEBUG_NEMPTY *****/
const ru_reg_rec DMA_DEBUG_NEMPTY_REG =
{
    "DEBUG_NEMPTY",
#if RU_INCLUDE_DESC
    "NOT_EMPTY_VECTOR Register",
    "Each peripheral is represented in a bit on the not empty vector.\nLSB is for rx peripherals, MSB for tx peripherals.\nIf the bit is asserted, the requests queue of the relevant peripheral is not empty.\nThe not empty vector is used by the DMA scheduler to determine which peripheral is the next to be served.\n",
#endif
    { DMA_DEBUG_NEMPTY_REG_OFFSET },
    0,
    0,
    267,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DMA_DEBUG_NEMPTY_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DMA_DEBUG_URGNT, TYPE: Type_DMA_DMA_DEBUG_URGNT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: URGNT *****/
const ru_field_rec DMA_DEBUG_URGNT_URGNT_FIELD =
{
    "URGNT",
#if RU_INCLUDE_DESC
    "",
    "indication whether the queue is in urgent state or not\n",
#endif
    { DMA_DEBUG_URGNT_URGNT_FIELD_MASK },
    0,
    { DMA_DEBUG_URGNT_URGNT_FIELD_WIDTH },
    { DMA_DEBUG_URGNT_URGNT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DMA_DEBUG_URGNT_FIELDS[] =
{
    &DMA_DEBUG_URGNT_URGNT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DMA_DEBUG_URGNT *****/
const ru_reg_rec DMA_DEBUG_URGNT_REG =
{
    "DEBUG_URGNT",
#if RU_INCLUDE_DESC
    "URGENT_VECTOR Register",
    "Each peripheral, a is represented in a bit on the urgent vector. 8 LSB are rx peripherlas, 8 MSB are tx peripherals.\nIf the bit is asserted, the requests queue of the relevant peripheral is in urgent state.\nThe urgent vector is used by the DMA scheduler to determine which peripheral is the next to be served.\n",
#endif
    { DMA_DEBUG_URGNT_REG_OFFSET },
    0,
    0,
    268,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DMA_DEBUG_URGNT_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DMA_DEBUG_SELSRC, TYPE: Type_DMA_DMA_DEBUG_SELSRC
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: SEL_SRC *****/
const ru_field_rec DMA_DEBUG_SELSRC_SEL_SRC_FIELD =
{
    "SEL_SRC",
#if RU_INCLUDE_DESC
    "",
    "the next peripheral to be served by the dma\n",
#endif
    { DMA_DEBUG_SELSRC_SEL_SRC_FIELD_MASK },
    0,
    { DMA_DEBUG_SELSRC_SEL_SRC_FIELD_WIDTH },
    { DMA_DEBUG_SELSRC_SEL_SRC_FIELD_SHIFT },
    63,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DMA_DEBUG_SELSRC_FIELDS[] =
{
    &DMA_DEBUG_SELSRC_SEL_SRC_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DMA_DEBUG_SELSRC *****/
const ru_reg_rec DMA_DEBUG_SELSRC_REG =
{
    "DEBUG_SELSRC",
#if RU_INCLUDE_DESC
    "SELECTED_SOURCE_NUM Register",
    "The decision of the dma schedule rand the next peripheral to be served, represented by its source address\n",
#endif
    { DMA_DEBUG_SELSRC_REG_OFFSET },
    0,
    0,
    269,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DMA_DEBUG_SELSRC_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DMA_DEBUG_REQ_CNT_RX, TYPE: Type_DMA_DMA_DEBUG_REQ_CNT_RX
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: REQ_CNT *****/
const ru_field_rec DMA_DEBUG_REQ_CNT_RX_REQ_CNT_FIELD =
{
    "REQ_CNT",
#if RU_INCLUDE_DESC
    "",
    "the number of pending write requests\n",
#endif
    { DMA_DEBUG_REQ_CNT_RX_REQ_CNT_FIELD_MASK },
    0,
    { DMA_DEBUG_REQ_CNT_RX_REQ_CNT_FIELD_WIDTH },
    { DMA_DEBUG_REQ_CNT_RX_REQ_CNT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DMA_DEBUG_REQ_CNT_RX_FIELDS[] =
{
    &DMA_DEBUG_REQ_CNT_RX_REQ_CNT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DMA_DEBUG_REQ_CNT_RX *****/
const ru_reg_rec DMA_DEBUG_REQ_CNT_RX_REG =
{
    "DEBUG_REQ_CNT_RX",
#if RU_INCLUDE_DESC
    "REQUEST_COUNTERS_RX 0..7 Register",
    "the number of write requests currently pending for each rx peripheral.\n",
#endif
    { DMA_DEBUG_REQ_CNT_RX_REG_OFFSET },
    DMA_DEBUG_REQ_CNT_RX_REG_RAM_CNT,
    4,
    270,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DMA_DEBUG_REQ_CNT_RX_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DMA_DEBUG_REQ_CNT_TX, TYPE: Type_DMA_DMA_DEBUG_REQ_CNT_TX
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: REQ_CNT *****/
const ru_field_rec DMA_DEBUG_REQ_CNT_TX_REQ_CNT_FIELD =
{
    "REQ_CNT",
#if RU_INCLUDE_DESC
    "",
    "the number of pending read requests\n",
#endif
    { DMA_DEBUG_REQ_CNT_TX_REQ_CNT_FIELD_MASK },
    0,
    { DMA_DEBUG_REQ_CNT_TX_REQ_CNT_FIELD_WIDTH },
    { DMA_DEBUG_REQ_CNT_TX_REQ_CNT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DMA_DEBUG_REQ_CNT_TX_FIELDS[] =
{
    &DMA_DEBUG_REQ_CNT_TX_REQ_CNT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DMA_DEBUG_REQ_CNT_TX *****/
const ru_reg_rec DMA_DEBUG_REQ_CNT_TX_REG =
{
    "DEBUG_REQ_CNT_TX",
#if RU_INCLUDE_DESC
    "REQUEST_COUNTERS_TX 0..7 Register",
    "the number of read requestscurrently pending for each TX peripheral.\n",
#endif
    { DMA_DEBUG_REQ_CNT_TX_REG_OFFSET },
    DMA_DEBUG_REQ_CNT_TX_REG_RAM_CNT,
    4,
    271,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DMA_DEBUG_REQ_CNT_TX_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DMA_DEBUG_REQ_CNT_RX_ACC, TYPE: Type_DMA_DMA_DEBUG_REQ_CNT_RX_ACC
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: REQ_CNT *****/
const ru_field_rec DMA_DEBUG_REQ_CNT_RX_ACC_REQ_CNT_FIELD =
{
    "REQ_CNT",
#if RU_INCLUDE_DESC
    "",
    "the number of pending write requests\n",
#endif
    { DMA_DEBUG_REQ_CNT_RX_ACC_REQ_CNT_FIELD_MASK },
    0,
    { DMA_DEBUG_REQ_CNT_RX_ACC_REQ_CNT_FIELD_WIDTH },
    { DMA_DEBUG_REQ_CNT_RX_ACC_REQ_CNT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DMA_DEBUG_REQ_CNT_RX_ACC_FIELDS[] =
{
    &DMA_DEBUG_REQ_CNT_RX_ACC_REQ_CNT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DMA_DEBUG_REQ_CNT_RX_ACC *****/
const ru_reg_rec DMA_DEBUG_REQ_CNT_RX_ACC_REG =
{
    "DEBUG_REQ_CNT_RX_ACC",
#if RU_INCLUDE_DESC
    "ACC_REQUEST_COUNTERS_RX 0..7 Register",
    "the accumulated number of write requests served so far for each peripheral. Wrap around on max value, not read clear.\n",
#endif
    { DMA_DEBUG_REQ_CNT_RX_ACC_REG_OFFSET },
    DMA_DEBUG_REQ_CNT_RX_ACC_REG_RAM_CNT,
    4,
    272,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DMA_DEBUG_REQ_CNT_RX_ACC_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DMA_DEBUG_REQ_CNT_TX_ACC, TYPE: Type_DMA_DMA_DEBUG_REQ_CNT_TX_ACC
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: REQ_CNT *****/
const ru_field_rec DMA_DEBUG_REQ_CNT_TX_ACC_REQ_CNT_FIELD =
{
    "REQ_CNT",
#if RU_INCLUDE_DESC
    "",
    "the number of pending write requests\n",
#endif
    { DMA_DEBUG_REQ_CNT_TX_ACC_REQ_CNT_FIELD_MASK },
    0,
    { DMA_DEBUG_REQ_CNT_TX_ACC_REQ_CNT_FIELD_WIDTH },
    { DMA_DEBUG_REQ_CNT_TX_ACC_REQ_CNT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DMA_DEBUG_REQ_CNT_TX_ACC_FIELDS[] =
{
    &DMA_DEBUG_REQ_CNT_TX_ACC_REQ_CNT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DMA_DEBUG_REQ_CNT_TX_ACC *****/
const ru_reg_rec DMA_DEBUG_REQ_CNT_TX_ACC_REG =
{
    "DEBUG_REQ_CNT_TX_ACC",
#if RU_INCLUDE_DESC
    "ACC_REQUEST_COUNTERS_TX 0..7 Register",
    "the accumulated number of read requests served so far for each peripheral. Wrap around on max value, not read clear.\n",
#endif
    { DMA_DEBUG_REQ_CNT_TX_ACC_REG_OFFSET },
    DMA_DEBUG_REQ_CNT_TX_ACC_REG_RAM_CNT,
    4,
    273,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DMA_DEBUG_REQ_CNT_TX_ACC_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DMA_DEBUG_UBUSCRDT, TYPE: Type_DMA_DMA_DEBUG_UBUSCRDT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DDR *****/
const ru_field_rec DMA_DEBUG_UBUSCRDT_DDR_FIELD =
{
    "DDR",
#if RU_INCLUDE_DESC
    "",
    "credits\n",
#endif
    { DMA_DEBUG_UBUSCRDT_DDR_FIELD_MASK },
    0,
    { DMA_DEBUG_UBUSCRDT_DDR_FIELD_WIDTH },
    { DMA_DEBUG_UBUSCRDT_DDR_FIELD_SHIFT },
    4,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: SRAM *****/
const ru_field_rec DMA_DEBUG_UBUSCRDT_SRAM_FIELD =
{
    "SRAM",
#if RU_INCLUDE_DESC
    "",
    "credits\n",
#endif
    { DMA_DEBUG_UBUSCRDT_SRAM_FIELD_MASK },
    0,
    { DMA_DEBUG_UBUSCRDT_SRAM_FIELD_WIDTH },
    { DMA_DEBUG_UBUSCRDT_SRAM_FIELD_SHIFT },
    4,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DMA_DEBUG_UBUSCRDT_FIELDS[] =
{
    &DMA_DEBUG_UBUSCRDT_DDR_FIELD,
    &DMA_DEBUG_UBUSCRDT_SRAM_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DMA_DEBUG_UBUSCRDT *****/
const ru_reg_rec DMA_DEBUG_UBUSCRDT_REG =
{
    "DEBUG_UBUSCRDT",
#if RU_INCLUDE_DESC
    "UBUS_CREDIT_COUNTERS Register",
    "number of DDR and SRAM UBUS credits that the DMA currently have.\n",
#endif
    { DMA_DEBUG_UBUSCRDT_REG_OFFSET },
    0,
    0,
    274,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    DMA_DEBUG_UBUSCRDT_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DMA_DEBUG_UBUSBYTES, TYPE: Type_DMA_DMA_DEBUG_UBUSBYTES
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DDR *****/
const ru_field_rec DMA_DEBUG_UBUSBYTES_DDR_FIELD =
{
    "DDR",
#if RU_INCLUDE_DESC
    "",
    "on the fly bytes\n",
#endif
    { DMA_DEBUG_UBUSBYTES_DDR_FIELD_MASK },
    0,
    { DMA_DEBUG_UBUSBYTES_DDR_FIELD_WIDTH },
    { DMA_DEBUG_UBUSBYTES_DDR_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: SRAM *****/
const ru_field_rec DMA_DEBUG_UBUSBYTES_SRAM_FIELD =
{
    "SRAM",
#if RU_INCLUDE_DESC
    "",
    "on the fly bytes\n",
#endif
    { DMA_DEBUG_UBUSBYTES_SRAM_FIELD_MASK },
    0,
    { DMA_DEBUG_UBUSBYTES_SRAM_FIELD_WIDTH },
    { DMA_DEBUG_UBUSBYTES_SRAM_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DMA_DEBUG_UBUSBYTES_FIELDS[] =
{
    &DMA_DEBUG_UBUSBYTES_DDR_FIELD,
    &DMA_DEBUG_UBUSBYTES_SRAM_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DMA_DEBUG_UBUSBYTES *****/
const ru_reg_rec DMA_DEBUG_UBUSBYTES_REG =
{
    "DEBUG_UBUSBYTES",
#if RU_INCLUDE_DESC
    "UBUS_BYTES_COUNTERS Register",
    "number of DDR and SRAM read data bytes that are waiting to be returned from the UBUS.\n",
#endif
    { DMA_DEBUG_UBUSBYTES_REG_OFFSET },
    0,
    0,
    275,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    DMA_DEBUG_UBUSBYTES_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DMA_DEBUG_ON_THE_FLY, TYPE: Type_DMA_DMA_DEBUG_ON_THE_FLY
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: OTF *****/
const ru_field_rec DMA_DEBUG_ON_THE_FLY_OTF_FIELD =
{
    "OTF",
#if RU_INCLUDE_DESC
    "",
    "reads on the fly\n",
#endif
    { DMA_DEBUG_ON_THE_FLY_OTF_FIELD_MASK },
    0,
    { DMA_DEBUG_ON_THE_FLY_OTF_FIELD_WIDTH },
    { DMA_DEBUG_ON_THE_FLY_OTF_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DMA_DEBUG_ON_THE_FLY_FIELDS[] =
{
    &DMA_DEBUG_ON_THE_FLY_OTF_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DMA_DEBUG_ON_THE_FLY *****/
const ru_reg_rec DMA_DEBUG_ON_THE_FLY_REG =
{
    "DEBUG_ON_THE_FLY",
#if RU_INCLUDE_DESC
    "READS_ON_THE_FLY Register",
    "number of read requests that were issued but not done yet.\n",
#endif
    { DMA_DEBUG_ON_THE_FLY_REG_OFFSET },
    0,
    0,
    276,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DMA_DEBUG_ON_THE_FLY_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DMA_DEBUG_DBG_SEL, TYPE: Type_DMA_DMA_DEBUG_DBG_SEL
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DBGSEL *****/
const ru_field_rec DMA_DEBUG_DBG_SEL_DBGSEL_FIELD =
{
    "DBGSEL",
#if RU_INCLUDE_DESC
    "",
    "select\n",
#endif
    { DMA_DEBUG_DBG_SEL_DBGSEL_FIELD_MASK },
    0,
    { DMA_DEBUG_DBG_SEL_DBGSEL_FIELD_WIDTH },
    { DMA_DEBUG_DBG_SEL_DBGSEL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DMA_DEBUG_DBG_SEL_FIELDS[] =
{
    &DMA_DEBUG_DBG_SEL_DBGSEL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DMA_DEBUG_DBG_SEL *****/
const ru_reg_rec DMA_DEBUG_DBG_SEL_REG =
{
    "DEBUG_DBG_SEL",
#if RU_INCLUDE_DESC
    "DBG_SEL Register",
    "select of the debug bus\n",
#endif
    { DMA_DEBUG_DBG_SEL_REG_OFFSET },
    0,
    0,
    277,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DMA_DEBUG_DBG_SEL_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DMA_DEBUG_DEBUGOUT, TYPE: Type_DMA_DMA_DEBUG_DEBUGOUT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DBG *****/
const ru_field_rec DMA_DEBUG_DEBUGOUT_DBG_FIELD =
{
    "DBG",
#if RU_INCLUDE_DESC
    "",
    "debug\n",
#endif
    { DMA_DEBUG_DEBUGOUT_DBG_FIELD_MASK },
    0,
    { DMA_DEBUG_DEBUGOUT_DBG_FIELD_WIDTH },
    { DMA_DEBUG_DEBUGOUT_DBG_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DMA_DEBUG_DEBUGOUT_FIELDS[] =
{
    &DMA_DEBUG_DEBUGOUT_DBG_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DMA_DEBUG_DEBUGOUT *****/
const ru_reg_rec DMA_DEBUG_DEBUGOUT_REG =
{
    "DEBUG_DEBUGOUT",
#if RU_INCLUDE_DESC
    "DEBUG_OUT Register",
    "output of the debug bus\n",
#endif
    { DMA_DEBUG_DEBUGOUT_REG_OFFSET },
    0,
    0,
    278,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DMA_DEBUG_DEBUGOUT_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DMA_DEBUG_UBUSREQ, TYPE: Type_DMA_DMA_DEBUG_UBUSREQ
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DDR *****/
const ru_field_rec DMA_DEBUG_UBUSREQ_DDR_FIELD =
{
    "DDR",
#if RU_INCLUDE_DESC
    "",
    "ddr requests\n",
#endif
    { DMA_DEBUG_UBUSREQ_DDR_FIELD_MASK },
    0,
    { DMA_DEBUG_UBUSREQ_DDR_FIELD_WIDTH },
    { DMA_DEBUG_UBUSREQ_DDR_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: SRAM *****/
const ru_field_rec DMA_DEBUG_UBUSREQ_SRAM_FIELD =
{
    "SRAM",
#if RU_INCLUDE_DESC
    "",
    "SRAM requests\n",
#endif
    { DMA_DEBUG_UBUSREQ_SRAM_FIELD_MASK },
    0,
    { DMA_DEBUG_UBUSREQ_SRAM_FIELD_WIDTH },
    { DMA_DEBUG_UBUSREQ_SRAM_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DMA_DEBUG_UBUSREQ_FIELDS[] =
{
    &DMA_DEBUG_UBUSREQ_DDR_FIELD,
    &DMA_DEBUG_UBUSREQ_SRAM_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DMA_DEBUG_UBUSREQ *****/
const ru_reg_rec DMA_DEBUG_UBUSREQ_REG =
{
    "DEBUG_UBUSREQ",
#if RU_INCLUDE_DESC
    "UBUS_REQ_COUNTERS Register",
    "number of DDR and SRAM UBUS requests on the fly.\n",
#endif
    { DMA_DEBUG_UBUSREQ_REG_OFFSET },
    0,
    0,
    279,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    DMA_DEBUG_UBUSREQ_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DMA_DEBUG_RDADD, TYPE: Type_DMA_DMA_DEBUG_RDADD
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: ADDRESS *****/
const ru_field_rec DMA_DEBUG_RDADD_ADDRESS_FIELD =
{
    "ADDRESS",
#if RU_INCLUDE_DESC
    "",
    "address within the ram\n",
#endif
    { DMA_DEBUG_RDADD_ADDRESS_FIELD_MASK },
    0,
    { DMA_DEBUG_RDADD_ADDRESS_FIELD_WIDTH },
    { DMA_DEBUG_RDADD_ADDRESS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DATACS *****/
const ru_field_rec DMA_DEBUG_RDADD_DATACS_FIELD =
{
    "DATACS",
#if RU_INCLUDE_DESC
    "",
    "chip select for write data ram\n",
#endif
    { DMA_DEBUG_RDADD_DATACS_FIELD_MASK },
    0,
    { DMA_DEBUG_RDADD_DATACS_FIELD_WIDTH },
    { DMA_DEBUG_RDADD_DATACS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: CDCS *****/
const ru_field_rec DMA_DEBUG_RDADD_CDCS_FIELD =
{
    "CDCS",
#if RU_INCLUDE_DESC
    "",
    "chip select for chunk descriptors ram\n",
#endif
    { DMA_DEBUG_RDADD_CDCS_FIELD_MASK },
    0,
    { DMA_DEBUG_RDADD_CDCS_FIELD_WIDTH },
    { DMA_DEBUG_RDADD_CDCS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: RRCS *****/
const ru_field_rec DMA_DEBUG_RDADD_RRCS_FIELD =
{
    "RRCS",
#if RU_INCLUDE_DESC
    "",
    "chip select for read requests ram\n",
#endif
    { DMA_DEBUG_RDADD_RRCS_FIELD_MASK },
    0,
    { DMA_DEBUG_RDADD_RRCS_FIELD_WIDTH },
    { DMA_DEBUG_RDADD_RRCS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DMA_DEBUG_RDADD_FIELDS[] =
{
    &DMA_DEBUG_RDADD_ADDRESS_FIELD,
    &DMA_DEBUG_RDADD_DATACS_FIELD,
    &DMA_DEBUG_RDADD_CDCS_FIELD,
    &DMA_DEBUG_RDADD_RRCS_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DMA_DEBUG_RDADD *****/
const ru_reg_rec DMA_DEBUG_RDADD_REG =
{
    "DEBUG_RDADD",
#if RU_INCLUDE_DESC
    "RAM_ADDRES Register",
    "the address and cs of the ram the user wishes to read using the indirect access read mechanism.\n",
#endif
    { DMA_DEBUG_RDADD_REG_OFFSET },
    0,
    0,
    280,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    DMA_DEBUG_RDADD_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DMA_DEBUG_RDVALID, TYPE: Type_DMA_DMA_DEBUG_RDVALID
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: VALID *****/
const ru_field_rec DMA_DEBUG_RDVALID_VALID_FIELD =
{
    "VALID",
#if RU_INCLUDE_DESC
    "",
    "indirect read request is valid\n",
#endif
    { DMA_DEBUG_RDVALID_VALID_FIELD_MASK },
    0,
    { DMA_DEBUG_RDVALID_VALID_FIELD_WIDTH },
    { DMA_DEBUG_RDVALID_VALID_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DMA_DEBUG_RDVALID_FIELDS[] =
{
    &DMA_DEBUG_RDVALID_VALID_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DMA_DEBUG_RDVALID *****/
const ru_reg_rec DMA_DEBUG_RDVALID_REG =
{
    "DEBUG_RDVALID",
#if RU_INCLUDE_DESC
    "INDIRECT_READ_REQUEST_VALID Register",
    "After determining the address and cs, the user should assert this bit for indicating that the address and cs are valid.\n",
#endif
    { DMA_DEBUG_RDVALID_REG_OFFSET },
    0,
    0,
    281,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DMA_DEBUG_RDVALID_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DMA_DEBUG_RDDATARDY, TYPE: Type_DMA_DMA_DEBUG_RDDATARDY
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: READY *****/
const ru_field_rec DMA_DEBUG_RDDATARDY_READY_FIELD =
{
    "READY",
#if RU_INCLUDE_DESC
    "",
    "read data ready\n",
#endif
    { DMA_DEBUG_RDDATARDY_READY_FIELD_MASK },
    0,
    { DMA_DEBUG_RDDATARDY_READY_FIELD_WIDTH },
    { DMA_DEBUG_RDDATARDY_READY_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DMA_DEBUG_RDDATARDY_FIELDS[] =
{
    &DMA_DEBUG_RDDATARDY_READY_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DMA_DEBUG_RDDATARDY *****/
const ru_reg_rec DMA_DEBUG_RDDATARDY_REG =
{
    "DEBUG_RDDATARDY",
#if RU_INCLUDE_DESC
    "READ_DATA_READY Register",
    "When assertd indicats that the data in the previous array is valid.Willremain asserted until the user deasserts the valid bit in regiser RDVALID.\n",
#endif
    { DMA_DEBUG_RDDATARDY_REG_OFFSET },
    0,
    0,
    282,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DMA_DEBUG_RDDATARDY_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DMA_DEBUG_RDDATA, TYPE: Type_DMA_DMA_DEBUG_RDDATA
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DATA *****/
const ru_field_rec DMA_DEBUG_RDDATA_DATA_FIELD =
{
    "DATA",
#if RU_INCLUDE_DESC
    "",
    "read data from ram\n",
#endif
    { DMA_DEBUG_RDDATA_DATA_FIELD_MASK },
    0,
    { DMA_DEBUG_RDDATA_DATA_FIELD_WIDTH },
    { DMA_DEBUG_RDDATA_DATA_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DMA_DEBUG_RDDATA_FIELDS[] =
{
    &DMA_DEBUG_RDDATA_DATA_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DMA_DEBUG_RDDATA *****/
const ru_reg_rec DMA_DEBUG_RDDATA_REG =
{
    "DEBUG_RDDATA",
#if RU_INCLUDE_DESC
    "INDIRECT_READ_DATA 0..3 Register",
    "The returned read data from the selected RAM. Array of 4 registers (128 bits total).\nThe width of the different memories is as follows:\nwrite data - 128 bits\nchunk descriptors - 36 bits\nread requests - 42 bits\nread data - 64 bits\n\nThe the memories with width smaller than 128, the data will appear in the first registers of the array, for example:\ndata from the cd RAM will appear in - {reg1[5:0], reg0[31:0]}.\n",
#endif
    { DMA_DEBUG_RDDATA_REG_OFFSET },
    DMA_DEBUG_RDDATA_REG_RAM_CNT,
    4,
    283,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DMA_DEBUG_RDDATA_FIELDS,
#endif
};

unsigned long DMA_ADDRS[] =
{
    0x828A1800,
    0x828A1C00,
    0x828A2000,
    0x828A2400,
};

static const ru_reg_rec *DMA_REGS[] =
{
    &DMA_CONFIG_NUM_OF_WRITES_REG,
    &DMA_CONFIG_NUM_OF_READS_REG,
    &DMA_CONFIG_U_THRESH_REG,
    &DMA_CONFIG_PRI_REG,
    &DMA_CONFIG_WEIGHT_REG,
    &DMA_CONFIG_PERIPH_SOURCE_REG,
    &DMA_CONFIG_TARGET_MEM_REG,
    &DMA_CONFIG_PTRRST_REG,
    &DMA_CONFIG_BBROUTEOVRD_REG,
    &DMA_CONFIG_CLK_GATE_CNTRL_REG,
    &DMA_CONFIG_UBUS_DPIDS_REG,
    &DMA_CONFIG_MAX_OTF_BYTES_REG,
    &DMA_CONFIG_UBUS_CREDITS_REG,
    &DMA_CONFIG_PSRAM_BASE_REG,
    &DMA_CONFIG_DDR_BASE_REG,
    &DMA_CONFIG_GEN_CFG_REG,
    &DMA_CONFIG_MAX_OTF_REQ_REG,
    &DMA_DEBUG_NEMPTY_REG,
    &DMA_DEBUG_URGNT_REG,
    &DMA_DEBUG_SELSRC_REG,
    &DMA_DEBUG_REQ_CNT_RX_REG,
    &DMA_DEBUG_REQ_CNT_TX_REG,
    &DMA_DEBUG_REQ_CNT_RX_ACC_REG,
    &DMA_DEBUG_REQ_CNT_TX_ACC_REG,
    &DMA_DEBUG_UBUSCRDT_REG,
    &DMA_DEBUG_UBUSBYTES_REG,
    &DMA_DEBUG_ON_THE_FLY_REG,
    &DMA_DEBUG_DBG_SEL_REG,
    &DMA_DEBUG_DEBUGOUT_REG,
    &DMA_DEBUG_UBUSREQ_REG,
    &DMA_DEBUG_RDADD_REG,
    &DMA_DEBUG_RDVALID_REG,
    &DMA_DEBUG_RDDATARDY_REG,
    &DMA_DEBUG_RDDATA_REG,
};

const ru_block_rec DMA_BLOCK =
{
    "DMA",
    DMA_ADDRS,
    4,
    34,
    DMA_REGS,
};
