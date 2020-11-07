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
 * Register: BBH_TX_COMMON_CONFIGURATIONS_MACTYPE
 ******************************************************************************/
const ru_reg_rec BBH_TX_COMMON_CONFIGURATIONS_MACTYPE_REG = 
{
    "COMMON_CONFIGURATIONS_MACTYPE",
#if RU_INCLUDE_DESC
    "MAC_TYPE Register",
    "The BBH supports working with different MAC types. Each MAC requires different interface and features. This register defines the type of MAC the BBH works with.",
#endif
    BBH_TX_COMMON_CONFIGURATIONS_MACTYPE_REG_OFFSET,
    0,
    0,
    468,
};

/******************************************************************************
 * Register: BBH_TX_COMMON_CONFIGURATIONS_BBCFG_1_TX
 ******************************************************************************/
const ru_reg_rec BBH_TX_COMMON_CONFIGURATIONS_BBCFG_1_TX_REG = 
{
    "COMMON_CONFIGURATIONS_BBCFG_1_TX",
#if RU_INCLUDE_DESC
    "BB_CFG_1 Register",
    "Each BBH unit has its own position on the BB tree. This position defines the Route address when approaching the Runner, S/DMA or S/BPM. The route is determined by a dedicated generic logic which uses the source id of the destination.",
#endif
    BBH_TX_COMMON_CONFIGURATIONS_BBCFG_1_TX_REG_OFFSET,
    0,
    0,
    469,
};

/******************************************************************************
 * Register: BBH_TX_COMMON_CONFIGURATIONS_BBCFG_2_TX
 ******************************************************************************/
const ru_reg_rec BBH_TX_COMMON_CONFIGURATIONS_BBCFG_2_TX_REG = 
{
    "COMMON_CONFIGURATIONS_BBCFG_2_TX",
#if RU_INCLUDE_DESC
    "BB_CFG_2 Register",
    "Each BBH unit has its own position on the BB tree. This position defines the Route address when approaching the Runner, S/DMA or S/BPM. The route is determined by a dedicated generic logic which uses the source id of the destination.",
#endif
    BBH_TX_COMMON_CONFIGURATIONS_BBCFG_2_TX_REG_OFFSET,
    0,
    0,
    470,
};

/******************************************************************************
 * Register: BBH_TX_COMMON_CONFIGURATIONS_DDRCFG_TX
 ******************************************************************************/
const ru_reg_rec BBH_TX_COMMON_CONFIGURATIONS_DDRCFG_TX_REG = 
{
    "COMMON_CONFIGURATIONS_DDRCFG_TX",
#if RU_INCLUDE_DESC
    "RD_ADDR_CFG Register",
    "Configurations for determining the address to read from the DDR/PSRAm",
#endif
    BBH_TX_COMMON_CONFIGURATIONS_DDRCFG_TX_REG_OFFSET,
    0,
    0,
    471,
};

/******************************************************************************
 * Register: BBH_TX_COMMON_CONFIGURATIONS_RNRCFG_1
 ******************************************************************************/
const ru_reg_rec BBH_TX_COMMON_CONFIGURATIONS_RNRCFG_1_REG = 
{
    "COMMON_CONFIGURATIONS_RNRCFG_1",
#if RU_INCLUDE_DESC
    "PD_RNR_CFG_1 %i Register",
    "Queue index address:"
    "The BBH requests a Packet descriptor from the Runner. The BBH writes the queue number in a predefined address at the Runner SRAM. The message serves also as a wake-up request to the Runner."
    "This register defines the queue index address within the Runner address space."
    "SKB address:"
    "When the packet is transmitted from absolute address, then, instead of releasing the BN, the BBH writes a 6 bits read counter into the Runner SRAM. It writes it into a pre-defined address + TCONT_NUM (for Ethernet TCONT_NUM = 0)."
    "This register defines the SKB free base address within the Runner address."
    ""
    "Note: all addresses are in 8 byte resolution. As the Runner memory is limited to 12 bits address, use the 12 lsb bits."
    "",
#endif
    BBH_TX_COMMON_CONFIGURATIONS_RNRCFG_1_REG_OFFSET,
    BBH_TX_COMMON_CONFIGURATIONS_RNRCFG_1_REG_RAM_CNT,
    4,
    472,
};

/******************************************************************************
 * Register: BBH_TX_COMMON_CONFIGURATIONS_RNRCFG_2
 ******************************************************************************/
const ru_reg_rec BBH_TX_COMMON_CONFIGURATIONS_RNRCFG_2_REG = 
{
    "COMMON_CONFIGURATIONS_RNRCFG_2",
#if RU_INCLUDE_DESC
    "PD_RNR_CFG_2 %i Register",
    "PD transfer process:"
    "-The Runner wont ACK the BBH; therefore the BBH wont wake the TX task."
    "-The Runner will push the PDs into the BBH (without any wakeup from the BBH)."
    "-Each time that the BBH reads a PD from the PD FIFO, it will write the read pointer into a pre-defined address in the Runner. The pointer is 6 bits width (one bit larger than needed to distinguish between full and empty)."
    "-The Runner should manage the congestion over the PD FIFO (in the BBH) by reading the BBH read pointer prior to each PD write."
    "-PD drop should be done by the Runner only. The BBH will drop PD when the FIFO is full and will count each drop. The BBH wont release the BN in this case."
    "-There will be a full threshold, which can be smaller than the actual size of the FIFO. When the BBH will move from full to not full state, the BBH will wakeup the Runner."
    ""
    "Note: all addresses are in 8 byte resolution. As the Runner memory is limited to 12 bits address, use the 12 lsb bits."
    "",
#endif
    BBH_TX_COMMON_CONFIGURATIONS_RNRCFG_2_REG_OFFSET,
    BBH_TX_COMMON_CONFIGURATIONS_RNRCFG_2_REG_RAM_CNT,
    4,
    473,
};

/******************************************************************************
 * Register: BBH_TX_COMMON_CONFIGURATIONS_DMACFG_TX
 ******************************************************************************/
const ru_reg_rec BBH_TX_COMMON_CONFIGURATIONS_DMACFG_TX_REG = 
{
    "COMMON_CONFIGURATIONS_DMACFG_TX",
#if RU_INCLUDE_DESC
    "DMA_CFG Register",
    "The BBH reads the packet data from the DDR in chunks (with a maximal size of 128 bytes)."
    "For each chunk the BBH writes a read request (descriptor) into the DMA memory space. The read descriptors are arranged in a predefined space in the DMA memory and managed in a cyclic FIFO style."
    "A special configuration limits the maximum number of read requests.",
#endif
    BBH_TX_COMMON_CONFIGURATIONS_DMACFG_TX_REG_OFFSET,
    0,
    0,
    474,
};

/******************************************************************************
 * Register: BBH_TX_COMMON_CONFIGURATIONS_SDMACFG_TX
 ******************************************************************************/
const ru_reg_rec BBH_TX_COMMON_CONFIGURATIONS_SDMACFG_TX_REG = 
{
    "COMMON_CONFIGURATIONS_SDMACFG_TX",
#if RU_INCLUDE_DESC
    "SDMA_CFG Register",
    "The BBH reads the packet data from the PSRAM in chunks (with a maximal size of 128 bytes)."
    "For each chunk the BBH writes a read request (descriptor) into the SDMA memory space. The read descriptors are arranged in a predefined space in the SDMA memory and managed in a cyclic FIFO style."
    "A special configuration limits the maximum number of read requests.",
#endif
    BBH_TX_COMMON_CONFIGURATIONS_SDMACFG_TX_REG_OFFSET,
    0,
    0,
    475,
};

/******************************************************************************
 * Register: BBH_TX_COMMON_CONFIGURATIONS_SBPMCFG
 ******************************************************************************/
const ru_reg_rec BBH_TX_COMMON_CONFIGURATIONS_SBPMCFG_REG = 
{
    "COMMON_CONFIGURATIONS_SBPMCFG",
#if RU_INCLUDE_DESC
    "SBPM_CFG Register",
    "When packet transmission is done, the BBH releases the SBPM buffers."
    "This register defines which release command is used:"
    "1. Normal free with context"
    "2. Special free with context"
    "3. free without context",
#endif
    BBH_TX_COMMON_CONFIGURATIONS_SBPMCFG_REG_OFFSET,
    0,
    0,
    476,
};

/******************************************************************************
 * Register: BBH_TX_COMMON_CONFIGURATIONS_DDRTMBASEL
 ******************************************************************************/
const ru_reg_rec BBH_TX_COMMON_CONFIGURATIONS_DDRTMBASEL_REG = 
{
    "COMMON_CONFIGURATIONS_DDRTMBASEL",
#if RU_INCLUDE_DESC
    "DDR_TM_BASE_LOW %i Register",
    "The BBH calculate the DDR physical address according to the Buffer number and buffer size and then adds the DDR TM base."
    ""
    "The DDR TM address space is divided to two - coherent and non coherent."
    ""
    "The first register in this array defines the base address of the non coherent space and the second is for the coherent."
    ""
    "The value of this register should match the relevant registers value in the BBH RX, QM and the Runner."
    "",
#endif
    BBH_TX_COMMON_CONFIGURATIONS_DDRTMBASEL_REG_OFFSET,
    BBH_TX_COMMON_CONFIGURATIONS_DDRTMBASEL_REG_RAM_CNT,
    4,
    477,
};

/******************************************************************************
 * Register: BBH_TX_COMMON_CONFIGURATIONS_DDRTMBASEH
 ******************************************************************************/
const ru_reg_rec BBH_TX_COMMON_CONFIGURATIONS_DDRTMBASEH_REG = 
{
    "COMMON_CONFIGURATIONS_DDRTMBASEH",
#if RU_INCLUDE_DESC
    "DDR_TM_BASE_HIGH %i Register",
    "The BBH calculate the DDR physical address according to the Buffer number and buffer size and then adds the DDR TM base."
    ""
    "The DDR TM address space is divided to two - coherent and non coherent."
    ""
    "The first register in this array defines the base address of the non coherent space and the second is for the coherent."
    ""
    "The value of this register should match the relevant registers value in the BBH RX, QM and the Runner."
    "",
#endif
    BBH_TX_COMMON_CONFIGURATIONS_DDRTMBASEH_REG_OFFSET,
    BBH_TX_COMMON_CONFIGURATIONS_DDRTMBASEH_REG_RAM_CNT,
    4,
    478,
};

/******************************************************************************
 * Register: BBH_TX_COMMON_CONFIGURATIONS_DFIFOCTRL
 ******************************************************************************/
const ru_reg_rec BBH_TX_COMMON_CONFIGURATIONS_DFIFOCTRL_REG = 
{
    "COMMON_CONFIGURATIONS_DFIFOCTRL",
#if RU_INCLUDE_DESC
    "DATA_FIFO_CTRL Register",
    "The BBH orders data both from DDR and PSRAM. The returned data is stored in two FIFOs for reordering. The two FIFOs are implemented in a single RAM. This register defines the division of the RAM to two FIFOs.",
#endif
    BBH_TX_COMMON_CONFIGURATIONS_DFIFOCTRL_REG_OFFSET,
    0,
    0,
    479,
};

/******************************************************************************
 * Register: BBH_TX_COMMON_CONFIGURATIONS_ARB_CFG
 ******************************************************************************/
const ru_reg_rec BBH_TX_COMMON_CONFIGURATIONS_ARB_CFG_REG = 
{
    "COMMON_CONFIGURATIONS_ARB_CFG",
#if RU_INCLUDE_DESC
    "ARB_CFG Register",
    "configurations related to different arbitration processes (ordering PDs, ordering data)",
#endif
    BBH_TX_COMMON_CONFIGURATIONS_ARB_CFG_REG_OFFSET,
    0,
    0,
    480,
};

/******************************************************************************
 * Register: BBH_TX_COMMON_CONFIGURATIONS_BBROUTE
 ******************************************************************************/
const ru_reg_rec BBH_TX_COMMON_CONFIGURATIONS_BBROUTE_REG = 
{
    "COMMON_CONFIGURATIONS_BBROUTE",
#if RU_INCLUDE_DESC
    "BB_ROUTE_OVERRIDE Register",
    "override configuration for the route of one of the peripherals (DMA/SDMMA/FPM/SBPM?Runners)",
#endif
    BBH_TX_COMMON_CONFIGURATIONS_BBROUTE_REG_OFFSET,
    0,
    0,
    481,
};

/******************************************************************************
 * Register: BBH_TX_COMMON_CONFIGURATIONS_PERQTASK
 ******************************************************************************/
const ru_reg_rec BBH_TX_COMMON_CONFIGURATIONS_PERQTASK_REG = 
{
    "COMMON_CONFIGURATIONS_PERQTASK",
#if RU_INCLUDE_DESC
    "PER_Q_TASK Register",
    "which task in the runner to wake-up when requesting a PD for a certain q."
    ""
    "This register holds the task number of the first 8 queues."
    ""
    "For queues 8-40 (if they exist) the task that will be waking is the one appearing in the PD_RNR_CFG regs, depending on which runner this queue is associated with.",
#endif
    BBH_TX_COMMON_CONFIGURATIONS_PERQTASK_REG_OFFSET,
    0,
    0,
    482,
};

/******************************************************************************
 * Register: BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD
 ******************************************************************************/
const ru_reg_rec BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_REG = 
{
    "COMMON_CONFIGURATIONS_TXRSTCMD",
#if RU_INCLUDE_DESC
    "TX_RESET_COMMAND Register",
    "This register enables reset of internal units (for possible WA purposes).",
#endif
    BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_REG_OFFSET,
    0,
    0,
    483,
};

/******************************************************************************
 * Register: BBH_TX_COMMON_CONFIGURATIONS_DBGSEL
 ******************************************************************************/
const ru_reg_rec BBH_TX_COMMON_CONFIGURATIONS_DBGSEL_REG = 
{
    "COMMON_CONFIGURATIONS_DBGSEL",
#if RU_INCLUDE_DESC
    "DEBUG_SELECT Register",
    "This register selects 1 of 8 debug vectors."
    "The selected vector is reflected to DBGOUTREG.",
#endif
    BBH_TX_COMMON_CONFIGURATIONS_DBGSEL_REG_OFFSET,
    0,
    0,
    484,
};

/******************************************************************************
 * Register: BBH_TX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL
 ******************************************************************************/
const ru_reg_rec BBH_TX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_REG = 
{
    "COMMON_CONFIGURATIONS_CLK_GATE_CNTRL",
#if RU_INCLUDE_DESC
    "CLOCK_GATE_CONTROL Register",
    "Clock Gate control register including timer config and bypass control",
#endif
    BBH_TX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_REG_OFFSET,
    0,
    0,
    485,
};

/******************************************************************************
 * Register: BBH_TX_COMMON_CONFIGURATIONS_GPR
 ******************************************************************************/
const ru_reg_rec BBH_TX_COMMON_CONFIGURATIONS_GPR_REG = 
{
    "COMMON_CONFIGURATIONS_GPR",
#if RU_INCLUDE_DESC
    "GENERAL_PURPOSE_REGISTER Register",
    "general purpose register",
#endif
    BBH_TX_COMMON_CONFIGURATIONS_GPR_REG_OFFSET,
    0,
    0,
    486,
};

/******************************************************************************
 * Register: BBH_TX_COMMON_CONFIGURATIONS_DS_DMA_SUP
 ******************************************************************************/
const ru_reg_rec BBH_TX_COMMON_CONFIGURATIONS_DS_DMA_SUP_REG = 
{
    "COMMON_CONFIGURATIONS_DS_DMA_SUP",
#if RU_INCLUDE_DESC
    "SUPPORT_DUAL_SLAVE_DMA Register",
    "configuration whether the BBH works with single DMA for both DDR and SRAM accesses or not.",
#endif
    BBH_TX_COMMON_CONFIGURATIONS_DS_DMA_SUP_REG_OFFSET,
    0,
    0,
    487,
};

/******************************************************************************
 * Register: BBH_TX_WAN_CONFIGURATIONS_Q2RNR
 ******************************************************************************/
const ru_reg_rec BBH_TX_WAN_CONFIGURATIONS_Q2RNR_REG = 
{
    "WAN_CONFIGURATIONS_Q2RNR",
#if RU_INCLUDE_DESC
    "Q_TO_RNR %i Register",
    "configuration which queue is managed by each of the two runners."
    ""
    "Each register in this array configures 2 queues.",
#endif
    BBH_TX_WAN_CONFIGURATIONS_Q2RNR_REG_OFFSET,
    BBH_TX_WAN_CONFIGURATIONS_Q2RNR_REG_RAM_CNT,
    4,
    488,
};

/******************************************************************************
 * Register: BBH_TX_WAN_CONFIGURATIONS_QPROF
 ******************************************************************************/
const ru_reg_rec BBH_TX_WAN_CONFIGURATIONS_QPROF_REG = 
{
    "WAN_CONFIGURATIONS_QPROF",
#if RU_INCLUDE_DESC
    "PER_Q_PROFILE %i Register",
    "configuration of the profile per queue."
    "The profile determines the PD FIFO size, the wakeup threshold and bytes threshold."
    ""
    "Each register in this array configures 2 queues.",
#endif
    BBH_TX_WAN_CONFIGURATIONS_QPROF_REG_OFFSET,
    BBH_TX_WAN_CONFIGURATIONS_QPROF_REG_RAM_CNT,
    4,
    489,
};

/******************************************************************************
 * Register: BBH_TX_WAN_CONFIGURATIONS_PDSIZE
 ******************************************************************************/
const ru_reg_rec BBH_TX_WAN_CONFIGURATIONS_PDSIZE_REG = 
{
    "WAN_CONFIGURATIONS_PDSIZE",
#if RU_INCLUDE_DESC
    "PD_FIFO_SIZE Register",
    "The BBH manages 40 queues for GPON and 32 queues for EPON (FIFO per TCONT/LLID). For each queue it manages a PD FIFO."
    "A total of 256 PDs are available for all queues."
    "There are 2 profiles of queues. For each profile the SW configures the size of the PD FIFO and then specifies for each queue which profile it is associated with."
    "This register defines the PD FIFO size of the 2 profiles."
    "",
#endif
    BBH_TX_WAN_CONFIGURATIONS_PDSIZE_REG_OFFSET,
    0,
    0,
    490,
};

/******************************************************************************
 * Register: BBH_TX_WAN_CONFIGURATIONS_PDWKUPH
 ******************************************************************************/
const ru_reg_rec BBH_TX_WAN_CONFIGURATIONS_PDWKUPH_REG = 
{
    "WAN_CONFIGURATIONS_PDWKUPH",
#if RU_INCLUDE_DESC
    "PD_WKUP_THRESH Register",
    "When a FIFO occupancy is above this wakeup threshold, the BBH will not wake-up the Runner for sending a new PD. This threshold does not represent the actual size of the FIFO. If a PD will arrive from the Runner when the FIFO is above the threshold, it will not be dropped unless the FIFO is actually full."
    "This register defines the threshold of the 2 profiles."
    "",
#endif
    BBH_TX_WAN_CONFIGURATIONS_PDWKUPH_REG_OFFSET,
    0,
    0,
    491,
};

/******************************************************************************
 * Register: BBH_TX_WAN_CONFIGURATIONS_PD_BYTE_TH
 ******************************************************************************/
const ru_reg_rec BBH_TX_WAN_CONFIGURATIONS_PD_BYTE_TH_REG = 
{
    "WAN_CONFIGURATIONS_PD_BYTE_TH",
#if RU_INCLUDE_DESC
    "PD_BYTES_THRESHOLD Register",
    "The BBH requests PDs from the Runner and maintains a pre-fetch PDs FIFO."
    "The PDs pre fetch is limited either by the PD FIFO configurable size or according to the total number of bytes (deducting bytes already requested/transmitted) for preventing HOL. This register defines the threshold of the 2 queue profiles.",
#endif
    BBH_TX_WAN_CONFIGURATIONS_PD_BYTE_TH_REG_OFFSET,
    0,
    0,
    492,
};

/******************************************************************************
 * Register: BBH_TX_WAN_CONFIGURATIONS_QMQ
 ******************************************************************************/
const ru_reg_rec BBH_TX_WAN_CONFIGURATIONS_QMQ_REG = 
{
    "WAN_CONFIGURATIONS_QMQ",
#if RU_INCLUDE_DESC
    "QM_Q %i Register",
    "This configuration determines whether the Q works with QM or with TM Runner."
    ""
    "QM queues will not send wakeup and read pointer messages."
    ""
    "Each register in this array configures 2 queues.",
#endif
    BBH_TX_WAN_CONFIGURATIONS_QMQ_REG_OFFSET,
    BBH_TX_WAN_CONFIGURATIONS_QMQ_REG_RAM_CNT,
    4,
    493,
};

/******************************************************************************
 * Register: BBH_TX_WAN_CONFIGURATIONS_PD_BYTE_TH_EN
 ******************************************************************************/
const ru_reg_rec BBH_TX_WAN_CONFIGURATIONS_PD_BYTE_TH_EN_REG = 
{
    "WAN_CONFIGURATIONS_PD_BYTE_TH_EN",
#if RU_INCLUDE_DESC
    "PD_BYTES_THRESHOLD_EN Register",
    "The BBH requests PDs from the Runner and maintains a pre-fetch PDs FIFO."
    "The PDs pre fetch is limited either by the PD FIFO configurable size or according to the total number of bytes (deducting bytes already requested/transmitted) for preventing HOL. Full configuration for the first 8 TCONT and one configuration per group of 8 TCONTs for the rest.",
#endif
    BBH_TX_WAN_CONFIGURATIONS_PD_BYTE_TH_EN_REG_OFFSET,
    0,
    0,
    494,
};

/******************************************************************************
 * Register: BBH_TX_WAN_CONFIGURATIONS_PDEMPTY
 ******************************************************************************/
const ru_reg_rec BBH_TX_WAN_CONFIGURATIONS_PDEMPTY_REG = 
{
    "WAN_CONFIGURATIONS_PDEMPTY",
#if RU_INCLUDE_DESC
    "PD_EMPTY_THRESHOLD Register",
    "The BBH manages 32 queues for EPON (FIFO per LLID). For each queue it manages a PD FIFO. Usually, the BBH orders PDs from the Runner in RR between all queues. In EPON BBH, if a FIFO occupancy is below this threshold, the queue will have higher priority in PD ordering arbitration (with RR between all the empty queues)."
    "This configuration is global for all queues."
    "Relevant only for EPON BBH.",
#endif
    BBH_TX_WAN_CONFIGURATIONS_PDEMPTY_REG_OFFSET,
    0,
    0,
    495,
};

/******************************************************************************
 * Register: BBH_TX_WAN_CONFIGURATIONS_STSRNRCFG_1
 ******************************************************************************/
const ru_reg_rec BBH_TX_WAN_CONFIGURATIONS_STSRNRCFG_1_REG = 
{
    "WAN_CONFIGURATIONS_STSRNRCFG_1",
#if RU_INCLUDE_DESC
    "STS_RNR_CFG_1 %i Register",
    "Queue index address:"
    "The BBH requests a Packet descriptor from the Runner. The BBH writes the queue number in a predefined address at the Runner SRAM. The message serves also as a wake-up request to the Runner."
    "This register defines the queue index address within the Runner address space."
    "SKB address:"
    "When the packet is transmitted from absolute address, then, instead of releasing the BN, the BBH writes a 6 bits read counter into the Runner SRAM. It writes it into a pre-defined address + TCONT_NUM (for Ethernet TCONT_NUM = 0)."
    "This register defines the SKB free base address within the Runner address."
    ""
    "Note: all addresses are in 8 byte resolution. As the Runner memory is limited to 12 bits address, use the 12 lsb bits."
    "",
#endif
    BBH_TX_WAN_CONFIGURATIONS_STSRNRCFG_1_REG_OFFSET,
    BBH_TX_WAN_CONFIGURATIONS_STSRNRCFG_1_REG_RAM_CNT,
    4,
    496,
};

/******************************************************************************
 * Register: BBH_TX_WAN_CONFIGURATIONS_STSRNRCFG_2
 ******************************************************************************/
const ru_reg_rec BBH_TX_WAN_CONFIGURATIONS_STSRNRCFG_2_REG = 
{
    "WAN_CONFIGURATIONS_STSRNRCFG_2",
#if RU_INCLUDE_DESC
    "STS_RNR_CFG_2 %i Register",
    "PD transfer process:"
    "-The Runner wont ACK the BBH; therefore the BBH wont wake the TX task."
    "-The Runner will push the PDs into the BBH (without any wakeup from the BBH)."
    "-Each time that the BBH reads a PD from the PD FIFO, it will write the read pointer into a pre-defined address in the Runner. The pointer is 6 bits width (one bit larger than needed to distinguish between full and empty)."
    "-The Runner should manage the congestion over the PD FIFO (in the BBH) by reading the BBH read pointer prior to each PD write."
    "-PD drop should be done by the Runner only. The BBH will drop PD when the FIFO is full and will count each drop. The BBH wont release the BN in this case."
    "-There will be a full threshold, which can be smaller than the actual size of the FIFO. When the BBH will move from full to not full state, the BBH will wakeup the Runner."
    ""
    "Note: all addresses are in 8 byte resolution. As the Runner memory is limited to 12 bits address, use the 12 lsb bits."
    "",
#endif
    BBH_TX_WAN_CONFIGURATIONS_STSRNRCFG_2_REG_OFFSET,
    BBH_TX_WAN_CONFIGURATIONS_STSRNRCFG_2_REG_RAM_CNT,
    4,
    497,
};

/******************************************************************************
 * Register: BBH_TX_WAN_CONFIGURATIONS_MSGRNRCFG_1
 ******************************************************************************/
const ru_reg_rec BBH_TX_WAN_CONFIGURATIONS_MSGRNRCFG_1_REG = 
{
    "WAN_CONFIGURATIONS_MSGRNRCFG_1",
#if RU_INCLUDE_DESC
    "MSG_RNR_CFG_1 %i Register",
    "Queue index address:"
    "The BBH requests a Packet descriptor from the Runner. The BBH writes the queue number in a predefined address at the Runner SRAM. The message serves also as a wake-up request to the Runner."
    "This register defines the queue index address within the Runner address space."
    "SKB address:"
    "When the packet is transmitted from absolute address, then, instead of releasing the BN, the BBH writes a 6 bits read counter into the Runner SRAM. It writes it into a pre-defined address + TCONT_NUM (for Ethernet TCONT_NUM = 0)."
    "This register defines the SKB free base address within the Runner address."
    ""
    "Note: all addresses are in 8 byte resolution. As the Runner memory is limited to 12 bits address, use the 12 lsb bits."
    "",
#endif
    BBH_TX_WAN_CONFIGURATIONS_MSGRNRCFG_1_REG_OFFSET,
    BBH_TX_WAN_CONFIGURATIONS_MSGRNRCFG_1_REG_RAM_CNT,
    4,
    498,
};

/******************************************************************************
 * Register: BBH_TX_WAN_CONFIGURATIONS_MSGRNRCFG_2
 ******************************************************************************/
const ru_reg_rec BBH_TX_WAN_CONFIGURATIONS_MSGRNRCFG_2_REG = 
{
    "WAN_CONFIGURATIONS_MSGRNRCFG_2",
#if RU_INCLUDE_DESC
    "MSG_RNR_CFG_2 %i Register",
    "PD transfer process:"
    "-The Runner wont ACK the BBH; therefore the BBH wont wake the TX task."
    "-The Runner will push the PDs into the BBH (without any wakeup from the BBH)."
    "-Each time that the BBH reads a PD from the PD FIFO, it will write the read pointer into a pre-defined address in the Runner. The pointer is 6 bits width (one bit larger than needed to distinguish between full and empty)."
    "-The Runner should manage the congestion over the PD FIFO (in the BBH) by reading the BBH read pointer prior to each PD write."
    "-PD drop should be done by the Runner only. The BBH will drop PD when the FIFO is full and will count each drop. The BBH wont release the BN in this case."
    "-There will be a full threshold, which can be smaller than the actual size of the FIFO. When the BBH will move from full to not full state, the BBH will wakeup the Runner."
    ""
    "Note: all addresses are in 8 byte resolution. As the Runner memory is limited to 12 bits address, use the 12 lsb bits."
    "",
#endif
    BBH_TX_WAN_CONFIGURATIONS_MSGRNRCFG_2_REG_OFFSET,
    BBH_TX_WAN_CONFIGURATIONS_MSGRNRCFG_2_REG_RAM_CNT,
    4,
    499,
};

/******************************************************************************
 * Register: BBH_TX_WAN_CONFIGURATIONS_EPNCFG
 ******************************************************************************/
const ru_reg_rec BBH_TX_WAN_CONFIGURATIONS_EPNCFG_REG = 
{
    "WAN_CONFIGURATIONS_EPNCFG",
#if RU_INCLUDE_DESC
    "EPN_CFG Register",
    "Configurations related to EPON MAC.",
#endif
    BBH_TX_WAN_CONFIGURATIONS_EPNCFG_REG_OFFSET,
    0,
    0,
    500,
};

/******************************************************************************
 * Register: BBH_TX_WAN_CONFIGURATIONS_FLOW2PORT
 ******************************************************************************/
const ru_reg_rec BBH_TX_WAN_CONFIGURATIONS_FLOW2PORT_REG = 
{
    "WAN_CONFIGURATIONS_FLOW2PORT",
#if RU_INCLUDE_DESC
    "FLOW2PORT Register",
    "interface for SW to access the flow id to port-id table",
#endif
    BBH_TX_WAN_CONFIGURATIONS_FLOW2PORT_REG_OFFSET,
    0,
    0,
    501,
};

/******************************************************************************
 * Register: BBH_TX_WAN_CONFIGURATIONS_TS
 ******************************************************************************/
const ru_reg_rec BBH_TX_WAN_CONFIGURATIONS_TS_REG = 
{
    "WAN_CONFIGURATIONS_TS",
#if RU_INCLUDE_DESC
    "TS Register",
    "The BBH is responsible for indicating the EPON MAC that the current packet that is being transmitted is a 1588 paacket. The BBH gets the 1588 parameters in the PD and forward it to the MAC."
    ""
    "This register is used to enable this feature.",
#endif
    BBH_TX_WAN_CONFIGURATIONS_TS_REG_OFFSET,
    0,
    0,
    502,
};

/******************************************************************************
 * Register: BBH_TX_WAN_CONFIGURATIONS_MAXWLEN
 ******************************************************************************/
const ru_reg_rec BBH_TX_WAN_CONFIGURATIONS_MAXWLEN_REG = 
{
    "WAN_CONFIGURATIONS_MAXWLEN",
#if RU_INCLUDE_DESC
    "DSL_MAXWLEN Register",
    "VDSL max word len"
    ""
    "relevant only for VDSL BBH",
#endif
    BBH_TX_WAN_CONFIGURATIONS_MAXWLEN_REG_OFFSET,
    0,
    0,
    503,
};

/******************************************************************************
 * Register: BBH_TX_WAN_CONFIGURATIONS_FLUSH
 ******************************************************************************/
const ru_reg_rec BBH_TX_WAN_CONFIGURATIONS_FLUSH_REG = 
{
    "WAN_CONFIGURATIONS_FLUSH",
#if RU_INCLUDE_DESC
    "DSL_FLUSH Register",
    "VDSL Flush indication"
    ""
    "relevant only for VDSL BBH",
#endif
    BBH_TX_WAN_CONFIGURATIONS_FLUSH_REG_OFFSET,
    0,
    0,
    504,
};

/******************************************************************************
 * Register: BBH_TX_LAN_CONFIGURATIONS_Q2RNR
 ******************************************************************************/
const ru_reg_rec BBH_TX_LAN_CONFIGURATIONS_Q2RNR_REG = 
{
    "LAN_CONFIGURATIONS_Q2RNR",
#if RU_INCLUDE_DESC
    "Q_TO_RNR Register",
    "configuration which queue is managed by each of the two runners."
    ""
    "Each register in this array configures 2 queues.",
#endif
    BBH_TX_LAN_CONFIGURATIONS_Q2RNR_REG_OFFSET,
    0,
    0,
    505,
};

/******************************************************************************
 * Register: BBH_TX_LAN_CONFIGURATIONS_QPROF
 ******************************************************************************/
const ru_reg_rec BBH_TX_LAN_CONFIGURATIONS_QPROF_REG = 
{
    "LAN_CONFIGURATIONS_QPROF",
#if RU_INCLUDE_DESC
    "PER_Q_PROFILE Register",
    "configuration of the profile per queue."
    "The profile determines the PD FIFO size, the wakeup threshold and bytes threshold."
    ""
    "Each register in this array configures 2 queues.",
#endif
    BBH_TX_LAN_CONFIGURATIONS_QPROF_REG_OFFSET,
    0,
    0,
    506,
};

/******************************************************************************
 * Register: BBH_TX_LAN_CONFIGURATIONS_PDSIZE
 ******************************************************************************/
const ru_reg_rec BBH_TX_LAN_CONFIGURATIONS_PDSIZE_REG = 
{
    "LAN_CONFIGURATIONS_PDSIZE",
#if RU_INCLUDE_DESC
    "PD_FIFO_SIZE Register",
    "The BBH manages 40 queues for GPON and 32 queues for EPON (FIFO per TCONT/LLID). For each queue it manages a PD FIFO."
    "A total of 256 PDs are available for all queues."
    "There are 2 profiles of queues. For each profile the SW configures the size of the PD FIFO and then specifies for each queue which profile it is associated with."
    "This register defines the PD FIFO size of the 2 profiles."
    "",
#endif
    BBH_TX_LAN_CONFIGURATIONS_PDSIZE_REG_OFFSET,
    0,
    0,
    507,
};

/******************************************************************************
 * Register: BBH_TX_LAN_CONFIGURATIONS_PDWKUPH
 ******************************************************************************/
const ru_reg_rec BBH_TX_LAN_CONFIGURATIONS_PDWKUPH_REG = 
{
    "LAN_CONFIGURATIONS_PDWKUPH",
#if RU_INCLUDE_DESC
    "PD_WKUP_THRESH Register",
    "When a FIFO occupancy is above this wakeup threshold, the BBH will not wake-up the Runner for sending a new PD. This threshold does not represent the actual size of the FIFO. If a PD will arrive from the Runner when the FIFO is above the threshold, it will not be dropped unless the FIFO is actually full."
    "This register defines the threshold of the 2 profiles."
    "",
#endif
    BBH_TX_LAN_CONFIGURATIONS_PDWKUPH_REG_OFFSET,
    0,
    0,
    508,
};

/******************************************************************************
 * Register: BBH_TX_LAN_CONFIGURATIONS_PD_BYTE_TH
 ******************************************************************************/
const ru_reg_rec BBH_TX_LAN_CONFIGURATIONS_PD_BYTE_TH_REG = 
{
    "LAN_CONFIGURATIONS_PD_BYTE_TH",
#if RU_INCLUDE_DESC
    "PD_BYTES_THRESHOLD Register",
    "The BBH requests PDs from the Runner and maintains a pre-fetch PDs FIFO."
    "The PDs pre fetch is limited either by the PD FIFO configurable size or according to the total number of bytes (deducting bytes already requested/transmitted) for preventing HOL. This register defines the threshold of the 2 queue profiles.",
#endif
    BBH_TX_LAN_CONFIGURATIONS_PD_BYTE_TH_REG_OFFSET,
    0,
    0,
    509,
};

/******************************************************************************
 * Register: BBH_TX_LAN_CONFIGURATIONS_QMQ
 ******************************************************************************/
const ru_reg_rec BBH_TX_LAN_CONFIGURATIONS_QMQ_REG = 
{
    "LAN_CONFIGURATIONS_QMQ",
#if RU_INCLUDE_DESC
    "QM_Q Register",
    "This configuration determines whether the Q works with QM or with TM Runner."
    ""
    "QM queues will not send wakeup and read pointer messages."
    ""
    "Each register in this array configures 2 queues.",
#endif
    BBH_TX_LAN_CONFIGURATIONS_QMQ_REG_OFFSET,
    0,
    0,
    510,
};

/******************************************************************************
 * Register: BBH_TX_LAN_CONFIGURATIONS_PD_BYTE_TH_EN
 ******************************************************************************/
const ru_reg_rec BBH_TX_LAN_CONFIGURATIONS_PD_BYTE_TH_EN_REG = 
{
    "LAN_CONFIGURATIONS_PD_BYTE_TH_EN",
#if RU_INCLUDE_DESC
    "PD_BYTES_THRESHOLD_EN Register",
    "The BBH requests PDs from the Runner and maintains a pre-fetch PDs FIFO."
    "The PDs pre fetch is limited either by the PD FIFO configurable size or according to the total number of bytes (deducting bytes already requested/transmitted) for preventing HOL. Full configuration for the first 8 TCONT and one configuration per group of 8 TCONTs for the rest.",
#endif
    BBH_TX_LAN_CONFIGURATIONS_PD_BYTE_TH_EN_REG_OFFSET,
    0,
    0,
    511,
};

/******************************************************************************
 * Register: BBH_TX_LAN_CONFIGURATIONS_PDEMPTY
 ******************************************************************************/
const ru_reg_rec BBH_TX_LAN_CONFIGURATIONS_PDEMPTY_REG = 
{
    "LAN_CONFIGURATIONS_PDEMPTY",
#if RU_INCLUDE_DESC
    "PD_EMPTY_THRESHOLD Register",
    "The BBH manages 32 queues for EPON (FIFO per LLID). For each queue it manages a PD FIFO. Usually, the BBH orders PDs from the Runner in RR between all queues. In EPON BBH, if a FIFO occupancy is below this threshold, the queue will have higher priority in PD ordering arbitration (with RR between all the empty queues)."
    "This configuration is global for all queues."
    "Relevant only for EPON BBH.",
#endif
    BBH_TX_LAN_CONFIGURATIONS_PDEMPTY_REG_OFFSET,
    0,
    0,
    512,
};

/******************************************************************************
 * Register: BBH_TX_LAN_CONFIGURATIONS_TXTHRESH
 ******************************************************************************/
const ru_reg_rec BBH_TX_LAN_CONFIGURATIONS_TXTHRESH_REG = 
{
    "LAN_CONFIGURATIONS_TXTHRESH",
#if RU_INCLUDE_DESC
    "TX_THRESHOLD Register",
    "Transmit threshold in 8 bytes resolution."
    "The BBH TX will not start to transmit data towards the XLMAC until the amount of data in the TX FIFO is larger than the threshold or if there is a complete packet in the FIFO.",
#endif
    BBH_TX_LAN_CONFIGURATIONS_TXTHRESH_REG_OFFSET,
    0,
    0,
    513,
};

/******************************************************************************
 * Register: BBH_TX_LAN_CONFIGURATIONS_EEE
 ******************************************************************************/
const ru_reg_rec BBH_TX_LAN_CONFIGURATIONS_EEE_REG = 
{
    "LAN_CONFIGURATIONS_EEE",
#if RU_INCLUDE_DESC
    "EEE Register",
    "The BBH is responsible for indicating the XLMAC that no traffic is about to arrive so the XLMAC may try to enter power saving mode."
    ""
    "This register is used to enable this feature.",
#endif
    BBH_TX_LAN_CONFIGURATIONS_EEE_REG_OFFSET,
    0,
    0,
    514,
};

/******************************************************************************
 * Register: BBH_TX_LAN_CONFIGURATIONS_TS
 ******************************************************************************/
const ru_reg_rec BBH_TX_LAN_CONFIGURATIONS_TS_REG = 
{
    "LAN_CONFIGURATIONS_TS",
#if RU_INCLUDE_DESC
    "TS Register",
    "The BBH is responsible for indicating the XLMAC that it should and calculate timestamp for the current packet that is being transmitted. The BBH gets the timestamping parameters in the PD and forward it to the XLMAC."
    ""
    "This register is used to enable this feature.",
#endif
    BBH_TX_LAN_CONFIGURATIONS_TS_REG_OFFSET,
    0,
    0,
    515,
};

/******************************************************************************
 * Register: BBH_TX_UNIFIED_CONFIGURATIONS_Q2RNR
 ******************************************************************************/
const ru_reg_rec BBH_TX_UNIFIED_CONFIGURATIONS_Q2RNR_REG = 
{
    "UNIFIED_CONFIGURATIONS_Q2RNR",
#if RU_INCLUDE_DESC
    "Q_TO_RNR %i Register",
    "configuration which queue is managed by each of the two runners."
    ""
    "Each register in this array configures 2 queues.",
#endif
    BBH_TX_UNIFIED_CONFIGURATIONS_Q2RNR_REG_OFFSET,
    BBH_TX_UNIFIED_CONFIGURATIONS_Q2RNR_REG_RAM_CNT,
    4,
    516,
};

/******************************************************************************
 * Register: BBH_TX_UNIFIED_CONFIGURATIONS_QPROF
 ******************************************************************************/
const ru_reg_rec BBH_TX_UNIFIED_CONFIGURATIONS_QPROF_REG = 
{
    "UNIFIED_CONFIGURATIONS_QPROF",
#if RU_INCLUDE_DESC
    "PER_Q_PROFILE %i Register",
    "configuration of the profile per queue."
    "The profile determines the PD FIFO size, the wakeup threshold and bytes threshold."
    ""
    "Each register in this array configures 2 queues.",
#endif
    BBH_TX_UNIFIED_CONFIGURATIONS_QPROF_REG_OFFSET,
    BBH_TX_UNIFIED_CONFIGURATIONS_QPROF_REG_RAM_CNT,
    4,
    517,
};

/******************************************************************************
 * Register: BBH_TX_UNIFIED_CONFIGURATIONS_PDSIZE
 ******************************************************************************/
const ru_reg_rec BBH_TX_UNIFIED_CONFIGURATIONS_PDSIZE_REG = 
{
    "UNIFIED_CONFIGURATIONS_PDSIZE",
#if RU_INCLUDE_DESC
    "PD_FIFO_SIZE Register",
    "The BBH manages 40 queues for GPON and 32 queues for EPON (FIFO per TCONT/LLID). For each queue it manages a PD FIFO."
    "A total of 256 PDs are available for all queues."
    "There are 2 profiles of queues. For each profile the SW configures the size of the PD FIFO and then specifies for each queue which profile it is associated with."
    "This register defines the PD FIFO size of the 2 profiles."
    "",
#endif
    BBH_TX_UNIFIED_CONFIGURATIONS_PDSIZE_REG_OFFSET,
    0,
    0,
    518,
};

/******************************************************************************
 * Register: BBH_TX_UNIFIED_CONFIGURATIONS_PDWKUPH
 ******************************************************************************/
const ru_reg_rec BBH_TX_UNIFIED_CONFIGURATIONS_PDWKUPH_REG = 
{
    "UNIFIED_CONFIGURATIONS_PDWKUPH",
#if RU_INCLUDE_DESC
    "PD_WKUP_THRESH Register",
    "When a FIFO occupancy is above this wakeup threshold, the BBH will not wake-up the Runner for sending a new PD. This threshold does not represent the actual size of the FIFO. If a PD will arrive from the Runner when the FIFO is above the threshold, it will not be dropped unless the FIFO is actually full."
    "This register defines the threshold of the 2 profiles."
    "",
#endif
    BBH_TX_UNIFIED_CONFIGURATIONS_PDWKUPH_REG_OFFSET,
    0,
    0,
    519,
};

/******************************************************************************
 * Register: BBH_TX_UNIFIED_CONFIGURATIONS_PD_BYTE_TH
 ******************************************************************************/
const ru_reg_rec BBH_TX_UNIFIED_CONFIGURATIONS_PD_BYTE_TH_REG = 
{
    "UNIFIED_CONFIGURATIONS_PD_BYTE_TH",
#if RU_INCLUDE_DESC
    "PD_BYTES_THRESHOLD Register",
    "The BBH requests PDs from the Runner and maintains a pre-fetch PDs FIFO."
    "The PDs pre fetch is limited either by the PD FIFO configurable size or according to the total number of bytes (deducting bytes already requested/transmitted) for preventing HOL. This register defines the threshold of the 2 queue profiles.",
#endif
    BBH_TX_UNIFIED_CONFIGURATIONS_PD_BYTE_TH_REG_OFFSET,
    0,
    0,
    520,
};

/******************************************************************************
 * Register: BBH_TX_UNIFIED_CONFIGURATIONS_QMQ
 ******************************************************************************/
const ru_reg_rec BBH_TX_UNIFIED_CONFIGURATIONS_QMQ_REG = 
{
    "UNIFIED_CONFIGURATIONS_QMQ",
#if RU_INCLUDE_DESC
    "QM_Q %i Register",
    "This configuration determines whether the Q works with QM or with TM Runner."
    ""
    "QM queues will not send wakeup and read pointer messages."
    ""
    "Each register in this array configures 2 queues.",
#endif
    BBH_TX_UNIFIED_CONFIGURATIONS_QMQ_REG_OFFSET,
    BBH_TX_UNIFIED_CONFIGURATIONS_QMQ_REG_RAM_CNT,
    4,
    521,
};

/******************************************************************************
 * Register: BBH_TX_UNIFIED_CONFIGURATIONS_PD_BYTE_TH_EN
 ******************************************************************************/
const ru_reg_rec BBH_TX_UNIFIED_CONFIGURATIONS_PD_BYTE_TH_EN_REG = 
{
    "UNIFIED_CONFIGURATIONS_PD_BYTE_TH_EN",
#if RU_INCLUDE_DESC
    "PD_BYTES_THRESHOLD_EN Register",
    "The BBH requests PDs from the Runner and maintains a pre-fetch PDs FIFO."
    "The PDs pre fetch is limited either by the PD FIFO configurable size or according to the total number of bytes (deducting bytes already requested/transmitted) for preventing HOL. Full configuration for the first 8 TCONT and one configuration per group of 8 TCONTs for the rest.",
#endif
    BBH_TX_UNIFIED_CONFIGURATIONS_PD_BYTE_TH_EN_REG_OFFSET,
    0,
    0,
    522,
};

/******************************************************************************
 * Register: BBH_TX_UNIFIED_CONFIGURATIONS_PDEMPTY
 ******************************************************************************/
const ru_reg_rec BBH_TX_UNIFIED_CONFIGURATIONS_PDEMPTY_REG = 
{
    "UNIFIED_CONFIGURATIONS_PDEMPTY",
#if RU_INCLUDE_DESC
    "PD_EMPTY_THRESHOLD Register",
    "The BBH manages 32 queues for EPON (FIFO per LLID). For each queue it manages a PD FIFO. Usually, the BBH orders PDs from the Runner in RR between all queues. In EPON BBH, if a FIFO occupancy is below this threshold, the queue will have higher priority in PD ordering arbitration (with RR between all the empty queues)."
    "This configuration is global for all queues."
    "Relevant only for EPON BBH.",
#endif
    BBH_TX_UNIFIED_CONFIGURATIONS_PDEMPTY_REG_OFFSET,
    0,
    0,
    523,
};

/******************************************************************************
 * Register: BBH_TX_UNIFIED_CONFIGURATIONS_GTXTHRESH
 ******************************************************************************/
const ru_reg_rec BBH_TX_UNIFIED_CONFIGURATIONS_GTXTHRESH_REG = 
{
    "UNIFIED_CONFIGURATIONS_GTXTHRESH",
#if RU_INCLUDE_DESC
    "GLOBAL_TX_THRESHOLD Register",
    "Transmit threshold in 8 bytes resolution."
    "The BBH TX will not start to transmit data towards the XLMAC until the amount of data in the TX FIFO is larger than the threshold or if there is a complete packet in the FIFO."
    "This threshold is used by the non unified BBH. for unified BBH it should be set to 0.",
#endif
    BBH_TX_UNIFIED_CONFIGURATIONS_GTXTHRESH_REG_OFFSET,
    0,
    0,
    524,
};

/******************************************************************************
 * Register: BBH_TX_UNIFIED_CONFIGURATIONS_EEE
 ******************************************************************************/
const ru_reg_rec BBH_TX_UNIFIED_CONFIGURATIONS_EEE_REG = 
{
    "UNIFIED_CONFIGURATIONS_EEE",
#if RU_INCLUDE_DESC
    "EEE Register",
    "The BBH is responsible for indicating the XLMAC that no traffic is about to arrive so the XLMAC may try to enter power saving mode."
    ""
    "This register is used to enable this feature per MAC",
#endif
    BBH_TX_UNIFIED_CONFIGURATIONS_EEE_REG_OFFSET,
    0,
    0,
    525,
};

/******************************************************************************
 * Register: BBH_TX_UNIFIED_CONFIGURATIONS_TS
 ******************************************************************************/
const ru_reg_rec BBH_TX_UNIFIED_CONFIGURATIONS_TS_REG = 
{
    "UNIFIED_CONFIGURATIONS_TS",
#if RU_INCLUDE_DESC
    "TS Register",
    "The BBH is responsible for indicating the XLMAC that it should and calculate timestamp for the current packet that is being transmitted. The BBH gets the timestamping parameters in the PD and forward it to the XLMAC."
    ""
    "This register is used to enable this feature per MAC",
#endif
    BBH_TX_UNIFIED_CONFIGURATIONS_TS_REG_OFFSET,
    0,
    0,
    526,
};

/******************************************************************************
 * Register: BBH_TX_UNIFIED_CONFIGURATIONS_FEBASE
 ******************************************************************************/
const ru_reg_rec BBH_TX_UNIFIED_CONFIGURATIONS_FEBASE_REG = 
{
    "UNIFIED_CONFIGURATIONS_FEBASE",
#if RU_INCLUDE_DESC
    "FE_FIFO_BASE %i Register",
    "The BBH manages 40 queues for GPON or 32 queus for EPON (1 for each TCONT/LLID). For each queue it manages a PD FIFO."
    "A total of 256 PDs are available for all queues."
    "For each Queue the SW configures the base and the size within these 256 PDs."
    ""
    "The size of the 1st BN FIFO and get-next FIFO is the same as the size of the PD FIFO of each queue."
    ""
    "each register in this array defines the PD FIFO base of 2 queues."
    "",
#endif
    BBH_TX_UNIFIED_CONFIGURATIONS_FEBASE_REG_OFFSET,
    BBH_TX_UNIFIED_CONFIGURATIONS_FEBASE_REG_RAM_CNT,
    4,
    527,
};

/******************************************************************************
 * Register: BBH_TX_UNIFIED_CONFIGURATIONS_FESIZE
 ******************************************************************************/
const ru_reg_rec BBH_TX_UNIFIED_CONFIGURATIONS_FESIZE_REG = 
{
    "UNIFIED_CONFIGURATIONS_FESIZE",
#if RU_INCLUDE_DESC
    "FE_FIFO_SIZE %i Register",
    "The BBH manages 40 queues for GPON and 32 queues for EPON (FIFO per TCONT/LLID). For each queue it manages a PD FIFO."
    "A total of 256 PDs are available for all queues."
    "For each Queue the SW configures the base and the size within these."
    "each register in this array defines the PD FIFO size of 2 queues."
    "",
#endif
    BBH_TX_UNIFIED_CONFIGURATIONS_FESIZE_REG_OFFSET,
    BBH_TX_UNIFIED_CONFIGURATIONS_FESIZE_REG_RAM_CNT,
    4,
    528,
};

/******************************************************************************
 * Register: BBH_TX_UNIFIED_CONFIGURATIONS_FEPDBASE
 ******************************************************************************/
const ru_reg_rec BBH_TX_UNIFIED_CONFIGURATIONS_FEPDBASE_REG = 
{
    "UNIFIED_CONFIGURATIONS_FEPDBASE",
#if RU_INCLUDE_DESC
    "FE_PD_FIFO_BASE %i Register",
    "The BBH manages 40 queues for GPON or 32 queus for EPON (1 for each TCONT/LLID). For each queue it manages a PD FIFO."
    "A total of 256 PDs are available for all queues."
    "For each Queue the SW configures the base and the size within these 256 PDs."
    ""
    "The size of the 1st BN FIFO and get-next FIFO is the same as the size of the PD FIFO of each queue."
    ""
    "each register in this array defines the PD FIFO base of 2 queues."
    "",
#endif
    BBH_TX_UNIFIED_CONFIGURATIONS_FEPDBASE_REG_OFFSET,
    BBH_TX_UNIFIED_CONFIGURATIONS_FEPDBASE_REG_RAM_CNT,
    4,
    529,
};

/******************************************************************************
 * Register: BBH_TX_UNIFIED_CONFIGURATIONS_FEPDSIZE
 ******************************************************************************/
const ru_reg_rec BBH_TX_UNIFIED_CONFIGURATIONS_FEPDSIZE_REG = 
{
    "UNIFIED_CONFIGURATIONS_FEPDSIZE",
#if RU_INCLUDE_DESC
    "FE_PD_FIFO_SIZE %i Register",
    "The BBH manages 40 queues for GPON and 32 queues for EPON (FIFO per TCONT/LLID). For each queue it manages a PD FIFO."
    "A total of 256 PDs are available for all queues."
    "For each Queue the SW configures the base and the size within these."
    "each register in this array defines the PD FIFO size of 2 queues."
    "",
#endif
    BBH_TX_UNIFIED_CONFIGURATIONS_FEPDSIZE_REG_OFFSET,
    BBH_TX_UNIFIED_CONFIGURATIONS_FEPDSIZE_REG_RAM_CNT,
    4,
    530,
};

/******************************************************************************
 * Register: BBH_TX_UNIFIED_CONFIGURATIONS_TXWRR
 ******************************************************************************/
const ru_reg_rec BBH_TX_UNIFIED_CONFIGURATIONS_TXWRR_REG = 
{
    "UNIFIED_CONFIGURATIONS_TXWRR",
#if RU_INCLUDE_DESC
    "TX_RR_WEIGHT %i Register",
    "The unified BBH TX serves multiple MACs."
    ""
    "The TX arbitration between these MACs is WRR."
    ""
    "This register array determines the weight of each MAC. Each register in the array represents 2 MACs.",
#endif
    BBH_TX_UNIFIED_CONFIGURATIONS_TXWRR_REG_OFFSET,
    BBH_TX_UNIFIED_CONFIGURATIONS_TXWRR_REG_RAM_CNT,
    4,
    531,
};

/******************************************************************************
 * Register: BBH_TX_UNIFIED_CONFIGURATIONS_TXTHRESH
 ******************************************************************************/
const ru_reg_rec BBH_TX_UNIFIED_CONFIGURATIONS_TXTHRESH_REG = 
{
    "UNIFIED_CONFIGURATIONS_TXTHRESH",
#if RU_INCLUDE_DESC
    "TX_THRESHOLD %i Register",
    "Transmit threshold in 8 bytes resolution."
    "The BBH TX will not start to transmit data towards the MAC until the amount of data in the TX FIFO is larger than the threshold or if there is a complete packet in the FIFO.",
#endif
    BBH_TX_UNIFIED_CONFIGURATIONS_TXTHRESH_REG_OFFSET,
    BBH_TX_UNIFIED_CONFIGURATIONS_TXTHRESH_REG_RAM_CNT,
    4,
    532,
};

/******************************************************************************
 * Register: BBH_TX_DEBUG_COUNTERS_SRAMPD
 ******************************************************************************/
const ru_reg_rec BBH_TX_DEBUG_COUNTERS_SRAMPD_REG = 
{
    "DEBUG_COUNTERS_SRAMPD",
#if RU_INCLUDE_DESC
    "SRAM_PD_COUNTER Register",
    "This counter counts the number of received PD for packets to be transmitted from the SRAM."
    "It counts the packets for all queues together."
    "This counter is cleared when read and freezes when maximum value is reached."
    "",
#endif
    BBH_TX_DEBUG_COUNTERS_SRAMPD_REG_OFFSET,
    0,
    0,
    533,
};

/******************************************************************************
 * Register: BBH_TX_DEBUG_COUNTERS_DDRPD
 ******************************************************************************/
const ru_reg_rec BBH_TX_DEBUG_COUNTERS_DDRPD_REG = 
{
    "DEBUG_COUNTERS_DDRPD",
#if RU_INCLUDE_DESC
    "DDR_PD_COUNTER Register",
    "This counter counts the number of received PDs for packets to be transmitted from the DDR."
    "It counts the packets for all queues together."
    "This counter is cleared when read and freezes when maximum value is reached.",
#endif
    BBH_TX_DEBUG_COUNTERS_DDRPD_REG_OFFSET,
    0,
    0,
    534,
};

/******************************************************************************
 * Register: BBH_TX_DEBUG_COUNTERS_PDDROP
 ******************************************************************************/
const ru_reg_rec BBH_TX_DEBUG_COUNTERS_PDDROP_REG = 
{
    "DEBUG_COUNTERS_PDDROP",
#if RU_INCLUDE_DESC
    "PD_DROP_COUNTER Register",
    "This counter counts the number of PDs which were dropped due to PD FIFO full."
    "It counts the packets for all queues together."
    "This counter is cleared when read and freezes when maximum value is reached."
    "",
#endif
    BBH_TX_DEBUG_COUNTERS_PDDROP_REG_OFFSET,
    0,
    0,
    535,
};

/******************************************************************************
 * Register: BBH_TX_DEBUG_COUNTERS_STSCNT
 ******************************************************************************/
const ru_reg_rec BBH_TX_DEBUG_COUNTERS_STSCNT_REG = 
{
    "DEBUG_COUNTERS_STSCNT",
#if RU_INCLUDE_DESC
    "STS_COUNTER Register",
    "This counter counts the number of STS messages which were received from Runner."
    "It counts the packets for all queues together."
    "This counter is cleared when read and freezes when maximum value is reached."
    "",
#endif
    BBH_TX_DEBUG_COUNTERS_STSCNT_REG_OFFSET,
    0,
    0,
    536,
};

/******************************************************************************
 * Register: BBH_TX_DEBUG_COUNTERS_STSDROP
 ******************************************************************************/
const ru_reg_rec BBH_TX_DEBUG_COUNTERS_STSDROP_REG = 
{
    "DEBUG_COUNTERS_STSDROP",
#if RU_INCLUDE_DESC
    "STS_DROP_COUNTER Register",
    "This counter counts the number of STS which were dropped due to PD FIFO full."
    "It counts the packets for all queues together."
    "This counter is cleared when read and freezes when maximum value is reached."
    "",
#endif
    BBH_TX_DEBUG_COUNTERS_STSDROP_REG_OFFSET,
    0,
    0,
    537,
};

/******************************************************************************
 * Register: BBH_TX_DEBUG_COUNTERS_MSGCNT
 ******************************************************************************/
const ru_reg_rec BBH_TX_DEBUG_COUNTERS_MSGCNT_REG = 
{
    "DEBUG_COUNTERS_MSGCNT",
#if RU_INCLUDE_DESC
    "MSG_COUNTER Register",
    "This counter counts the number of MSG (DBR/Ghost) messages which were received from Runner."
    "It counts the packets for all queues together."
    "This counter is cleared when read and freezes when maximum value is reached."
    "",
#endif
    BBH_TX_DEBUG_COUNTERS_MSGCNT_REG_OFFSET,
    0,
    0,
    538,
};

/******************************************************************************
 * Register: BBH_TX_DEBUG_COUNTERS_MSGDROP
 ******************************************************************************/
const ru_reg_rec BBH_TX_DEBUG_COUNTERS_MSGDROP_REG = 
{
    "DEBUG_COUNTERS_MSGDROP",
#if RU_INCLUDE_DESC
    "MSG_DROP_COUNTER Register",
    "This counter counts the number of MSG which were dropped due to PD FIFO full."
    "It counts the packets for all queues together."
    "This counter is cleared when read and freezes when maximum value is reached."
    "",
#endif
    BBH_TX_DEBUG_COUNTERS_MSGDROP_REG_OFFSET,
    0,
    0,
    539,
};

/******************************************************************************
 * Register: BBH_TX_DEBUG_COUNTERS_GETNEXTNULL
 ******************************************************************************/
const ru_reg_rec BBH_TX_DEBUG_COUNTERS_GETNEXTNULL_REG = 
{
    "DEBUG_COUNTERS_GETNEXTNULL",
#if RU_INCLUDE_DESC
    "GET_NEXT_IS_NULL_COUNTER Register",
    "This counter counts the number Get next responses with a null BN."
    "It counts the packets for all TCONTs together."
    "This counter is cleared when read and freezes when maximum value is reached."
    "This counter is relevant for Ethernet only.",
#endif
    BBH_TX_DEBUG_COUNTERS_GETNEXTNULL_REG_OFFSET,
    0,
    0,
    540,
};

/******************************************************************************
 * Register: BBH_TX_DEBUG_COUNTERS_FLUSHPKTS
 ******************************************************************************/
const ru_reg_rec BBH_TX_DEBUG_COUNTERS_FLUSHPKTS_REG = 
{
    "DEBUG_COUNTERS_FLUSHPKTS",
#if RU_INCLUDE_DESC
    "FLUSHED_PACKETS_COUNTER Register",
    "This counter counts the number of packets that were flushed (bn was released without sending the data to the EPON MAC) due to flush request."
    "The counter is global for all queues."
    "The counter is read clear.",
#endif
    BBH_TX_DEBUG_COUNTERS_FLUSHPKTS_REG_OFFSET,
    0,
    0,
    541,
};

/******************************************************************************
 * Register: BBH_TX_DEBUG_COUNTERS_LENERR
 ******************************************************************************/
const ru_reg_rec BBH_TX_DEBUG_COUNTERS_LENERR_REG = 
{
    "DEBUG_COUNTERS_LENERR",
#if RU_INCLUDE_DESC
    "REQ_LENGTH_ERROR_COUNTER Register",
    "This counter counts the number of times a length error (mismatch between a request from the MAC and a PD from the Runner) occured."
    "This counter is cleared when read and freezes when maximum value is reached.",
#endif
    BBH_TX_DEBUG_COUNTERS_LENERR_REG_OFFSET,
    0,
    0,
    542,
};

/******************************************************************************
 * Register: BBH_TX_DEBUG_COUNTERS_AGGRLENERR
 ******************************************************************************/
const ru_reg_rec BBH_TX_DEBUG_COUNTERS_AGGRLENERR_REG = 
{
    "DEBUG_COUNTERS_AGGRLENERR",
#if RU_INCLUDE_DESC
    "AGGREGATION_LENGTH_ERROR_COUNTER Register",
    "This counter Counts aggregation length error events."
    "If one or more of the packets in an aggregated PD is shorter than 60 bytes, this counter will be incremented by 1."
    "This counter is cleared when read and freezes when maximum value is reached.",
#endif
    BBH_TX_DEBUG_COUNTERS_AGGRLENERR_REG_OFFSET,
    0,
    0,
    543,
};

/******************************************************************************
 * Register: BBH_TX_DEBUG_COUNTERS_SRAMPKT
 ******************************************************************************/
const ru_reg_rec BBH_TX_DEBUG_COUNTERS_SRAMPKT_REG = 
{
    "DEBUG_COUNTERS_SRAMPKT",
#if RU_INCLUDE_DESC
    "SRAM_PKT_COUNTER Register",
    "This counter counts the number of received packets to be transmitted from the SRAM."
    "It counts the packets for all queues together."
    "This counter is cleared when read and freezes when maximum value is reached."
    "",
#endif
    BBH_TX_DEBUG_COUNTERS_SRAMPKT_REG_OFFSET,
    0,
    0,
    544,
};

/******************************************************************************
 * Register: BBH_TX_DEBUG_COUNTERS_DDRPKT
 ******************************************************************************/
const ru_reg_rec BBH_TX_DEBUG_COUNTERS_DDRPKT_REG = 
{
    "DEBUG_COUNTERS_DDRPKT",
#if RU_INCLUDE_DESC
    "DDR_PKT_COUNTER Register",
    "This counter counts the number of received packets to be transmitted from the DDR."
    "It counts the packets for all queues together."
    "This counter is cleared when read and freezes when maximum value is reached."
    "",
#endif
    BBH_TX_DEBUG_COUNTERS_DDRPKT_REG_OFFSET,
    0,
    0,
    545,
};

/******************************************************************************
 * Register: BBH_TX_DEBUG_COUNTERS_SRAMBYTE
 ******************************************************************************/
const ru_reg_rec BBH_TX_DEBUG_COUNTERS_SRAMBYTE_REG = 
{
    "DEBUG_COUNTERS_SRAMBYTE",
#if RU_INCLUDE_DESC
    "SRAM_BYTE_COUNTER Register",
    "This counter counts the number of transmitted bytes from the SRAM."
    "It counts the bytes for all queues together."
    "This counter is cleared when read and freezes when maximum value is reached."
    "",
#endif
    BBH_TX_DEBUG_COUNTERS_SRAMBYTE_REG_OFFSET,
    0,
    0,
    546,
};

/******************************************************************************
 * Register: BBH_TX_DEBUG_COUNTERS_DDRBYTE
 ******************************************************************************/
const ru_reg_rec BBH_TX_DEBUG_COUNTERS_DDRBYTE_REG = 
{
    "DEBUG_COUNTERS_DDRBYTE",
#if RU_INCLUDE_DESC
    "DDR_BYTE_COUNTER Register",
    "This counter counts the number of transmitted bytes from the DDR."
    "It counts the bytes for all queues together."
    "This counter is cleared when read and freezes when maximum value is reached."
    "",
#endif
    BBH_TX_DEBUG_COUNTERS_DDRBYTE_REG_OFFSET,
    0,
    0,
    547,
};

/******************************************************************************
 * Register: BBH_TX_DEBUG_COUNTERS_SWRDEN
 ******************************************************************************/
const ru_reg_rec BBH_TX_DEBUG_COUNTERS_SWRDEN_REG = 
{
    "DEBUG_COUNTERS_SWRDEN",
#if RU_INCLUDE_DESC
    "SW_RD_EN Register",
    "writing to this register creates a rd_en pulse to the selected array the SW wants to access."
    ""
    "Each bit in the register represents one of the arrays the SW can access."
    ""
    "The address inside the array is determined in the previous register (sw_rd_address)."
    ""
    "When writing to this register the SW should assert only one bit. If more than one is asserted, The HW will return the value read from the lsb selected array.",
#endif
    BBH_TX_DEBUG_COUNTERS_SWRDEN_REG_OFFSET,
    0,
    0,
    548,
};

/******************************************************************************
 * Register: BBH_TX_DEBUG_COUNTERS_SWRDADDR
 ******************************************************************************/
const ru_reg_rec BBH_TX_DEBUG_COUNTERS_SWRDADDR_REG = 
{
    "DEBUG_COUNTERS_SWRDADDR",
#if RU_INCLUDE_DESC
    "SW_RD_ADDR Register",
    "the address inside the array the SW wants to read",
#endif
    BBH_TX_DEBUG_COUNTERS_SWRDADDR_REG_OFFSET,
    0,
    0,
    549,
};

/******************************************************************************
 * Register: BBH_TX_DEBUG_COUNTERS_SWRDDATA
 ******************************************************************************/
const ru_reg_rec BBH_TX_DEBUG_COUNTERS_SWRDDATA_REG = 
{
    "DEBUG_COUNTERS_SWRDDATA",
#if RU_INCLUDE_DESC
    "SW_RD_DATA Register",
    "indirect memories and arrays read data",
#endif
    BBH_TX_DEBUG_COUNTERS_SWRDDATA_REG_OFFSET,
    0,
    0,
    550,
};

/******************************************************************************
 * Register: BBH_TX_DEBUG_COUNTERS_UNIFIEDPKT
 ******************************************************************************/
const ru_reg_rec BBH_TX_DEBUG_COUNTERS_UNIFIEDPKT_REG = 
{
    "DEBUG_COUNTERS_UNIFIEDPKT",
#if RU_INCLUDE_DESC
    "UNIFIED_PKT_COUNTER %i Register",
    "This counter array counts the number of transmitted packets through each interface in the unified BBH."
    ""
    "This counter is cleared when read and freezes when maximum value is reached."
    "",
#endif
    BBH_TX_DEBUG_COUNTERS_UNIFIEDPKT_REG_OFFSET,
    BBH_TX_DEBUG_COUNTERS_UNIFIEDPKT_REG_RAM_CNT,
    4,
    551,
};

/******************************************************************************
 * Register: BBH_TX_DEBUG_COUNTERS_UNIFIEDBYTE
 ******************************************************************************/
const ru_reg_rec BBH_TX_DEBUG_COUNTERS_UNIFIEDBYTE_REG = 
{
    "DEBUG_COUNTERS_UNIFIEDBYTE",
#if RU_INCLUDE_DESC
    "UNIFIED_BYTE_COUNTER %i Register",
    "This counter array counts the number of transmitted bytes through each interface in the unified BBH."
    ""
    "This counter is cleared when read and freezes when maximum value is reached."
    "",
#endif
    BBH_TX_DEBUG_COUNTERS_UNIFIEDBYTE_REG_OFFSET,
    BBH_TX_DEBUG_COUNTERS_UNIFIEDBYTE_REG_RAM_CNT,
    4,
    552,
};

/******************************************************************************
 * Register: BBH_TX_DEBUG_COUNTERS_DBGOUTREG
 ******************************************************************************/
const ru_reg_rec BBH_TX_DEBUG_COUNTERS_DBGOUTREG_REG = 
{
    "DEBUG_COUNTERS_DBGOUTREG",
#if RU_INCLUDE_DESC
    "DEBUG_OUT_REG %i Register",
    "an array including all the debug vectors of the BBH TX."
    "entries 30 and 31 are DSL debug.",
#endif
    BBH_TX_DEBUG_COUNTERS_DBGOUTREG_REG_OFFSET,
    BBH_TX_DEBUG_COUNTERS_DBGOUTREG_REG_RAM_CNT,
    4,
    553,
};

/******************************************************************************
 * Register: BBH_TX_DEBUG_COUNTERS_IN_SEGMENTATION
 ******************************************************************************/
const ru_reg_rec BBH_TX_DEBUG_COUNTERS_IN_SEGMENTATION_REG = 
{
    "DEBUG_COUNTERS_IN_SEGMENTATION",
#if RU_INCLUDE_DESC
    "IN_SEGMENTATION %i Register",
    "40 bit vector in which each bit represents if the segmentation SM is currently handling a PD of a certain TCONT."
    ""
    "first address is for TCONTS [31:0]"
    "second is for TCONTS [39:32]",
#endif
    BBH_TX_DEBUG_COUNTERS_IN_SEGMENTATION_REG_OFFSET,
    BBH_TX_DEBUG_COUNTERS_IN_SEGMENTATION_REG_RAM_CNT,
    4,
    554,
};

/******************************************************************************
 * Block: BBH_TX
 ******************************************************************************/
static const ru_reg_rec *BBH_TX_REGS[] =
{
    &BBH_TX_COMMON_CONFIGURATIONS_MACTYPE_REG,
    &BBH_TX_COMMON_CONFIGURATIONS_BBCFG_1_TX_REG,
    &BBH_TX_COMMON_CONFIGURATIONS_BBCFG_2_TX_REG,
    &BBH_TX_COMMON_CONFIGURATIONS_DDRCFG_TX_REG,
    &BBH_TX_COMMON_CONFIGURATIONS_RNRCFG_1_REG,
    &BBH_TX_COMMON_CONFIGURATIONS_RNRCFG_2_REG,
    &BBH_TX_COMMON_CONFIGURATIONS_DMACFG_TX_REG,
    &BBH_TX_COMMON_CONFIGURATIONS_SDMACFG_TX_REG,
    &BBH_TX_COMMON_CONFIGURATIONS_SBPMCFG_REG,
    &BBH_TX_COMMON_CONFIGURATIONS_DDRTMBASEL_REG,
    &BBH_TX_COMMON_CONFIGURATIONS_DDRTMBASEH_REG,
    &BBH_TX_COMMON_CONFIGURATIONS_DFIFOCTRL_REG,
    &BBH_TX_COMMON_CONFIGURATIONS_ARB_CFG_REG,
    &BBH_TX_COMMON_CONFIGURATIONS_BBROUTE_REG,
    &BBH_TX_COMMON_CONFIGURATIONS_PERQTASK_REG,
    &BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_REG,
    &BBH_TX_COMMON_CONFIGURATIONS_DBGSEL_REG,
    &BBH_TX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_REG,
    &BBH_TX_COMMON_CONFIGURATIONS_GPR_REG,
    &BBH_TX_COMMON_CONFIGURATIONS_DS_DMA_SUP_REG,
    &BBH_TX_WAN_CONFIGURATIONS_Q2RNR_REG,
    &BBH_TX_WAN_CONFIGURATIONS_QPROF_REG,
    &BBH_TX_WAN_CONFIGURATIONS_PDSIZE_REG,
    &BBH_TX_WAN_CONFIGURATIONS_PDWKUPH_REG,
    &BBH_TX_WAN_CONFIGURATIONS_PD_BYTE_TH_REG,
    &BBH_TX_WAN_CONFIGURATIONS_QMQ_REG,
    &BBH_TX_WAN_CONFIGURATIONS_PD_BYTE_TH_EN_REG,
    &BBH_TX_WAN_CONFIGURATIONS_PDEMPTY_REG,
    &BBH_TX_WAN_CONFIGURATIONS_STSRNRCFG_1_REG,
    &BBH_TX_WAN_CONFIGURATIONS_STSRNRCFG_2_REG,
    &BBH_TX_WAN_CONFIGURATIONS_MSGRNRCFG_1_REG,
    &BBH_TX_WAN_CONFIGURATIONS_MSGRNRCFG_2_REG,
    &BBH_TX_WAN_CONFIGURATIONS_EPNCFG_REG,
    &BBH_TX_WAN_CONFIGURATIONS_FLOW2PORT_REG,
    &BBH_TX_WAN_CONFIGURATIONS_TS_REG,
    &BBH_TX_WAN_CONFIGURATIONS_MAXWLEN_REG,
    &BBH_TX_WAN_CONFIGURATIONS_FLUSH_REG,
    &BBH_TX_LAN_CONFIGURATIONS_Q2RNR_REG,
    &BBH_TX_LAN_CONFIGURATIONS_QPROF_REG,
    &BBH_TX_LAN_CONFIGURATIONS_PDSIZE_REG,
    &BBH_TX_LAN_CONFIGURATIONS_PDWKUPH_REG,
    &BBH_TX_LAN_CONFIGURATIONS_PD_BYTE_TH_REG,
    &BBH_TX_LAN_CONFIGURATIONS_QMQ_REG,
    &BBH_TX_LAN_CONFIGURATIONS_PD_BYTE_TH_EN_REG,
    &BBH_TX_LAN_CONFIGURATIONS_PDEMPTY_REG,
    &BBH_TX_LAN_CONFIGURATIONS_TXTHRESH_REG,
    &BBH_TX_LAN_CONFIGURATIONS_EEE_REG,
    &BBH_TX_LAN_CONFIGURATIONS_TS_REG,
    &BBH_TX_UNIFIED_CONFIGURATIONS_Q2RNR_REG,
    &BBH_TX_UNIFIED_CONFIGURATIONS_QPROF_REG,
    &BBH_TX_UNIFIED_CONFIGURATIONS_PDSIZE_REG,
    &BBH_TX_UNIFIED_CONFIGURATIONS_PDWKUPH_REG,
    &BBH_TX_UNIFIED_CONFIGURATIONS_PD_BYTE_TH_REG,
    &BBH_TX_UNIFIED_CONFIGURATIONS_QMQ_REG,
    &BBH_TX_UNIFIED_CONFIGURATIONS_PD_BYTE_TH_EN_REG,
    &BBH_TX_UNIFIED_CONFIGURATIONS_PDEMPTY_REG,
    &BBH_TX_UNIFIED_CONFIGURATIONS_GTXTHRESH_REG,
    &BBH_TX_UNIFIED_CONFIGURATIONS_EEE_REG,
    &BBH_TX_UNIFIED_CONFIGURATIONS_TS_REG,
    &BBH_TX_UNIFIED_CONFIGURATIONS_FEBASE_REG,
    &BBH_TX_UNIFIED_CONFIGURATIONS_FESIZE_REG,
    &BBH_TX_UNIFIED_CONFIGURATIONS_FEPDBASE_REG,
    &BBH_TX_UNIFIED_CONFIGURATIONS_FEPDSIZE_REG,
    &BBH_TX_UNIFIED_CONFIGURATIONS_TXWRR_REG,
    &BBH_TX_UNIFIED_CONFIGURATIONS_TXTHRESH_REG,
    &BBH_TX_DEBUG_COUNTERS_SRAMPD_REG,
    &BBH_TX_DEBUG_COUNTERS_DDRPD_REG,
    &BBH_TX_DEBUG_COUNTERS_PDDROP_REG,
    &BBH_TX_DEBUG_COUNTERS_STSCNT_REG,
    &BBH_TX_DEBUG_COUNTERS_STSDROP_REG,
    &BBH_TX_DEBUG_COUNTERS_MSGCNT_REG,
    &BBH_TX_DEBUG_COUNTERS_MSGDROP_REG,
    &BBH_TX_DEBUG_COUNTERS_GETNEXTNULL_REG,
    &BBH_TX_DEBUG_COUNTERS_FLUSHPKTS_REG,
    &BBH_TX_DEBUG_COUNTERS_LENERR_REG,
    &BBH_TX_DEBUG_COUNTERS_AGGRLENERR_REG,
    &BBH_TX_DEBUG_COUNTERS_SRAMPKT_REG,
    &BBH_TX_DEBUG_COUNTERS_DDRPKT_REG,
    &BBH_TX_DEBUG_COUNTERS_SRAMBYTE_REG,
    &BBH_TX_DEBUG_COUNTERS_DDRBYTE_REG,
    &BBH_TX_DEBUG_COUNTERS_SWRDEN_REG,
    &BBH_TX_DEBUG_COUNTERS_SWRDADDR_REG,
    &BBH_TX_DEBUG_COUNTERS_SWRDDATA_REG,
    &BBH_TX_DEBUG_COUNTERS_UNIFIEDPKT_REG,
    &BBH_TX_DEBUG_COUNTERS_UNIFIEDBYTE_REG,
    &BBH_TX_DEBUG_COUNTERS_DBGOUTREG_REG,
    &BBH_TX_DEBUG_COUNTERS_IN_SEGMENTATION_REG,
};

unsigned long BBH_TX_ADDRS[] =
{
    0x82d90000,
    0x82d92000,
};

const ru_block_rec BBH_TX_BLOCK = 
{
    "BBH_TX",
    BBH_TX_ADDRS,
    2,
    87,
    BBH_TX_REGS
};

/* End of file XRDP_BBH_TX.c */
