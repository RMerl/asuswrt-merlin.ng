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
 * Register: SBPM_REGS_INIT_FREE_LIST
 ******************************************************************************/
const ru_reg_rec SBPM_REGS_INIT_FREE_LIST_REG = 
{
    "REGS_INIT_FREE_LIST",
#if RU_INCLUDE_DESC
    "INIT_FREE_LIST Register",
    "request for building the free list using HW accelerator",
#endif
    SBPM_REGS_INIT_FREE_LIST_REG_OFFSET,
    0,
    0,
    615,
};

/******************************************************************************
 * Register: SBPM_REGS_BN_ALLOC
 ******************************************************************************/
const ru_reg_rec SBPM_REGS_BN_ALLOC_REG = 
{
    "REGS_BN_ALLOC",
#if RU_INCLUDE_DESC
    "BN_ALLOC Register",
    "request for a new buffer",
#endif
    SBPM_REGS_BN_ALLOC_REG_OFFSET,
    0,
    0,
    616,
};

/******************************************************************************
 * Register: SBPM_REGS_BN_ALLOC_RPLY
 ******************************************************************************/
const ru_reg_rec SBPM_REGS_BN_ALLOC_RPLY_REG = 
{
    "REGS_BN_ALLOC_RPLY",
#if RU_INCLUDE_DESC
    "BN_ALLOC_RPLY Register",
    "reply for a new buffer alloc",
#endif
    SBPM_REGS_BN_ALLOC_RPLY_REG_OFFSET,
    0,
    0,
    617,
};

/******************************************************************************
 * Register: SBPM_REGS_BN_FREE_WITH_CONTXT_LOW
 ******************************************************************************/
const ru_reg_rec SBPM_REGS_BN_FREE_WITH_CONTXT_LOW_REG = 
{
    "REGS_BN_FREE_WITH_CONTXT_LOW",
#if RU_INCLUDE_DESC
    "BN_FREE_WITH_CONTXT_LOW Register",
    "Request for freeing buffers of a packet offline with context (lower 32-bit)",
#endif
    SBPM_REGS_BN_FREE_WITH_CONTXT_LOW_REG_OFFSET,
    0,
    0,
    618,
};

/******************************************************************************
 * Register: SBPM_REGS_BN_FREE_WITH_CONTXT_HIGH
 ******************************************************************************/
const ru_reg_rec SBPM_REGS_BN_FREE_WITH_CONTXT_HIGH_REG = 
{
    "REGS_BN_FREE_WITH_CONTXT_HIGH",
#if RU_INCLUDE_DESC
    "BN_FREE_WITH_CONTXT_HIGH Register",
    "Request for freeing buffers of a packet offline with context (higher 32-bit)",
#endif
    SBPM_REGS_BN_FREE_WITH_CONTXT_HIGH_REG_OFFSET,
    0,
    0,
    619,
};

/******************************************************************************
 * Register: SBPM_REGS_MCST_INC
 ******************************************************************************/
const ru_reg_rec SBPM_REGS_MCST_INC_REG = 
{
    "REGS_MCST_INC",
#if RU_INCLUDE_DESC
    "MCST_INC Register",
    "Multicast counter increment. Contains the BN, which is head of the packet to be multicast and its counter value",
#endif
    SBPM_REGS_MCST_INC_REG_OFFSET,
    0,
    0,
    620,
};

/******************************************************************************
 * Register: SBPM_REGS_MCST_INC_RPLY
 ******************************************************************************/
const ru_reg_rec SBPM_REGS_MCST_INC_RPLY_REG = 
{
    "REGS_MCST_INC_RPLY",
#if RU_INCLUDE_DESC
    "MCST_INC_RPLY Register",
    "mcst_inc_rply",
#endif
    SBPM_REGS_MCST_INC_RPLY_REG_OFFSET,
    0,
    0,
    621,
};

/******************************************************************************
 * Register: SBPM_REGS_BN_CONNECT
 ******************************************************************************/
const ru_reg_rec SBPM_REGS_BN_CONNECT_REG = 
{
    "REGS_BN_CONNECT",
#if RU_INCLUDE_DESC
    "BN_CONNECT Register",
    "request for connection between two buffers in a linked list. The connection request may be replied with ACK message if the ACK request bit is asserted."
    "This command is used as write command.",
#endif
    SBPM_REGS_BN_CONNECT_REG_OFFSET,
    0,
    0,
    622,
};

/******************************************************************************
 * Register: SBPM_REGS_BN_CONNECT_RPLY
 ******************************************************************************/
const ru_reg_rec SBPM_REGS_BN_CONNECT_RPLY_REG = 
{
    "REGS_BN_CONNECT_RPLY",
#if RU_INCLUDE_DESC
    "BN_CONNECT_RPLY Register",
    "bn_connect_rply",
#endif
    SBPM_REGS_BN_CONNECT_RPLY_REG_OFFSET,
    0,
    0,
    623,
};

/******************************************************************************
 * Register: SBPM_REGS_GET_NEXT
 ******************************************************************************/
const ru_reg_rec SBPM_REGS_GET_NEXT_REG = 
{
    "REGS_GET_NEXT",
#if RU_INCLUDE_DESC
    "GET_NEXT Register",
    "a pointer to a buffer in a packet linked list and request for the next buffer in the list"
    "this command is used as read command.",
#endif
    SBPM_REGS_GET_NEXT_REG_OFFSET,
    0,
    0,
    624,
};

/******************************************************************************
 * Register: SBPM_REGS_GET_NEXT_RPLY
 ******************************************************************************/
const ru_reg_rec SBPM_REGS_GET_NEXT_RPLY_REG = 
{
    "REGS_GET_NEXT_RPLY",
#if RU_INCLUDE_DESC
    "GET_NEXT_RPLY Register",
    "get_next_rply",
#endif
    SBPM_REGS_GET_NEXT_RPLY_REG_OFFSET,
    0,
    0,
    625,
};

/******************************************************************************
 * Register: SBPM_REGS_SBPM_CLK_GATE_CNTRL
 ******************************************************************************/
const ru_reg_rec SBPM_REGS_SBPM_CLK_GATE_CNTRL_REG = 
{
    "REGS_SBPM_CLK_GATE_CNTRL",
#if RU_INCLUDE_DESC
    "SBPM_CLK_GATE_CNTRL Register",
    "control for the bl_clk_control module",
#endif
    SBPM_REGS_SBPM_CLK_GATE_CNTRL_REG_OFFSET,
    0,
    0,
    626,
};

/******************************************************************************
 * Register: SBPM_REGS_BN_FREE_WITHOUT_CONTXT
 ******************************************************************************/
const ru_reg_rec SBPM_REGS_BN_FREE_WITHOUT_CONTXT_REG = 
{
    "REGS_BN_FREE_WITHOUT_CONTXT",
#if RU_INCLUDE_DESC
    "BN_FREE_WITHOUT_CONTXT Register",
    "bn_free_without_contxt",
#endif
    SBPM_REGS_BN_FREE_WITHOUT_CONTXT_REG_OFFSET,
    0,
    0,
    627,
};

/******************************************************************************
 * Register: SBPM_REGS_BN_FREE_WITHOUT_CONTXT_RPLY
 ******************************************************************************/
const ru_reg_rec SBPM_REGS_BN_FREE_WITHOUT_CONTXT_RPLY_REG = 
{
    "REGS_BN_FREE_WITHOUT_CONTXT_RPLY",
#if RU_INCLUDE_DESC
    "BN_FREE_WITHOUT_CONTXT_RPLY Register",
    "bn_free_without_contxt_rply",
#endif
    SBPM_REGS_BN_FREE_WITHOUT_CONTXT_RPLY_REG_OFFSET,
    0,
    0,
    628,
};

/******************************************************************************
 * Register: SBPM_REGS_BN_FREE_WITH_CONTXT_RPLY
 ******************************************************************************/
const ru_reg_rec SBPM_REGS_BN_FREE_WITH_CONTXT_RPLY_REG = 
{
    "REGS_BN_FREE_WITH_CONTXT_RPLY",
#if RU_INCLUDE_DESC
    "BN_FREE_WITH_CONTXT_RPLY Register",
    "bn_free_with_contxt_rply",
#endif
    SBPM_REGS_BN_FREE_WITH_CONTXT_RPLY_REG_OFFSET,
    0,
    0,
    629,
};

/******************************************************************************
 * Register: SBPM_REGS_SBPM_GL_TRSH
 ******************************************************************************/
const ru_reg_rec SBPM_REGS_SBPM_GL_TRSH_REG = 
{
    "REGS_SBPM_GL_TRSH",
#if RU_INCLUDE_DESC
    "GLOBAL_THRESHOLD Register",
    "Global Threshold for Allocated Buffers."
    "SBPM will issue BN in the accepted range upon to Global threshold setup."
    "Ths register also holds global hysteresis value for ACK/NACK transition setting. We cross to Nack state if BAC equals the threshold. We cross down to Ack if BAC equals the thrshold minus the histeresis value.",
#endif
    SBPM_REGS_SBPM_GL_TRSH_REG_OFFSET,
    0,
    0,
    630,
};

/******************************************************************************
 * Register: SBPM_REGS_SBPM_UG0_TRSH
 ******************************************************************************/
const ru_reg_rec SBPM_REGS_SBPM_UG0_TRSH_REG = 
{
    "REGS_SBPM_UG0_TRSH",
#if RU_INCLUDE_DESC
    "UG0_THRESHOLD Register",
    "Threshold for Allocated Buffers of UG0"
    "Ths register also holds UG0 hysteresis value for ACK/NACK transition setting."
    "We cross to Nack state if BAC equals the threshold. We cross down to Ack if BAC equals the thrshold minus the histeresis value.",
#endif
    SBPM_REGS_SBPM_UG0_TRSH_REG_OFFSET,
    0,
    0,
    631,
};

/******************************************************************************
 * Register: SBPM_REGS_SBPM_UG1_TRSH
 ******************************************************************************/
const ru_reg_rec SBPM_REGS_SBPM_UG1_TRSH_REG = 
{
    "REGS_SBPM_UG1_TRSH",
#if RU_INCLUDE_DESC
    "UG1_THRESHOLD Register",
    "Threshold for Allocated Buffers of UG1"
    "Ths register also holds UG1 hysteresis value for ACK/NACK transition setting."
    "We cross to Nack state if BAC equals the threshold. We cross down to Ack if BAC equals the thrshold minus the histeresis value.",
#endif
    SBPM_REGS_SBPM_UG1_TRSH_REG_OFFSET,
    0,
    0,
    632,
};

/******************************************************************************
 * Register: SBPM_REGS_SBPM_DBG
 ******************************************************************************/
const ru_reg_rec SBPM_REGS_SBPM_DBG_REG = 
{
    "REGS_SBPM_DBG",
#if RU_INCLUDE_DESC
    "SBPM_DBG Register",
    "SBPM select the debug bus",
#endif
    SBPM_REGS_SBPM_DBG_REG_OFFSET,
    0,
    0,
    633,
};

/******************************************************************************
 * Register: SBPM_REGS_SBPM_UG0_BAC
 ******************************************************************************/
const ru_reg_rec SBPM_REGS_SBPM_UG0_BAC_REG = 
{
    "REGS_SBPM_UG0_BAC",
#if RU_INCLUDE_DESC
    "SBPM_UG0_BAC Register",
    "SBPM UG0 allocated BN counter",
#endif
    SBPM_REGS_SBPM_UG0_BAC_REG_OFFSET,
    0,
    0,
    634,
};

/******************************************************************************
 * Register: SBPM_REGS_SBPM_UG1_BAC
 ******************************************************************************/
const ru_reg_rec SBPM_REGS_SBPM_UG1_BAC_REG = 
{
    "REGS_SBPM_UG1_BAC",
#if RU_INCLUDE_DESC
    "SBPM_UG1_BAC Register",
    "SBPM UG1 allocated BN Counter",
#endif
    SBPM_REGS_SBPM_UG1_BAC_REG_OFFSET,
    0,
    0,
    635,
};

/******************************************************************************
 * Register: SBPM_REGS_SBPM_GL_BAC
 ******************************************************************************/
const ru_reg_rec SBPM_REGS_SBPM_GL_BAC_REG = 
{
    "REGS_SBPM_GL_BAC",
#if RU_INCLUDE_DESC
    "SBPM_GL_BAC Register",
    "SBPM global BN Counter",
#endif
    SBPM_REGS_SBPM_GL_BAC_REG_OFFSET,
    0,
    0,
    636,
};

/******************************************************************************
 * Register: SBPM_REGS_SBPM_UG0_EXCL_HIGH_TRSH
 ******************************************************************************/
const ru_reg_rec SBPM_REGS_SBPM_UG0_EXCL_HIGH_TRSH_REG = 
{
    "REGS_SBPM_UG0_EXCL_HIGH_TRSH",
#if RU_INCLUDE_DESC
    "SBPM_UG0_EXCLUSIVE_HIGH_THRESHOLD Register",
    "SBPM UG0 Exclusive high and hysteresis threshold."
    "We cross to Excl state if BAC equals the threshold. We cross down to not Excl if BAC equals the thrshold minus the histeresis value.",
#endif
    SBPM_REGS_SBPM_UG0_EXCL_HIGH_TRSH_REG_OFFSET,
    0,
    0,
    637,
};

/******************************************************************************
 * Register: SBPM_REGS_SBPM_UG1_EXCL_HIGH_TRSH
 ******************************************************************************/
const ru_reg_rec SBPM_REGS_SBPM_UG1_EXCL_HIGH_TRSH_REG = 
{
    "REGS_SBPM_UG1_EXCL_HIGH_TRSH",
#if RU_INCLUDE_DESC
    "SBPM_UG1_EXCLUSIVE_HIGH_THRESHOLD Register",
    "SBPM UG1 Exclusive high and hysteresis threshold."
    "We cross to Excl state if BAC equals the threshold. We cross down to not Excl if BAC equals the thrshold minus the histeresis value.",
#endif
    SBPM_REGS_SBPM_UG1_EXCL_HIGH_TRSH_REG_OFFSET,
    0,
    0,
    638,
};

/******************************************************************************
 * Register: SBPM_REGS_SBPM_UG0_EXCL_LOW_TRSH
 ******************************************************************************/
const ru_reg_rec SBPM_REGS_SBPM_UG0_EXCL_LOW_TRSH_REG = 
{
    "REGS_SBPM_UG0_EXCL_LOW_TRSH",
#if RU_INCLUDE_DESC
    "SBPM_UG0_EXCLUSIVE_LOW_THRESHOLD Register",
    "SBPM UG0 Exclusive low and hysteresis threshold."
    "We cross to Excl state if BAC equals the threshold. We cross down to not Excl if BAC equals the thrshold minus the histeresis value.",
#endif
    SBPM_REGS_SBPM_UG0_EXCL_LOW_TRSH_REG_OFFSET,
    0,
    0,
    639,
};

/******************************************************************************
 * Register: SBPM_REGS_SBPM_UG1_EXCL_LOW_TRSH
 ******************************************************************************/
const ru_reg_rec SBPM_REGS_SBPM_UG1_EXCL_LOW_TRSH_REG = 
{
    "REGS_SBPM_UG1_EXCL_LOW_TRSH",
#if RU_INCLUDE_DESC
    "SBPM_UG1_EXCLUSIVE_LOW_THRESHOLD Register",
    "SBPM UG1 Exclusive low and hysteresis threshold."
    "We cross to Excl state if BAC equals the threshold. We cross down to not Excl if BAC equals the thrshold minus the histeresis value.",
#endif
    SBPM_REGS_SBPM_UG1_EXCL_LOW_TRSH_REG_OFFSET,
    0,
    0,
    640,
};

/******************************************************************************
 * Register: SBPM_REGS_SBPM_UG_STATUS
 ******************************************************************************/
const ru_reg_rec SBPM_REGS_SBPM_UG_STATUS_REG = 
{
    "REGS_SBPM_UG_STATUS",
#if RU_INCLUDE_DESC
    "USER_GROUP_STATUS_REGISTER Register",
    "This register is status set of all 8 Ugs: Ack/NACK state and in addition Exclusive state pereach of 8 UGs",
#endif
    SBPM_REGS_SBPM_UG_STATUS_REG_OFFSET,
    0,
    0,
    641,
};

/******************************************************************************
 * Register: SBPM_REGS_ERROR_HANDLING_PARAMS
 ******************************************************************************/
const ru_reg_rec SBPM_REGS_ERROR_HANDLING_PARAMS_REG = 
{
    "REGS_ERROR_HANDLING_PARAMS",
#if RU_INCLUDE_DESC
    "ERROR_HANDLING_PARAMS Register",
    "Parameters and thresholds used for Error handling: error detection, max search enable and threshold, etc.",
#endif
    SBPM_REGS_ERROR_HANDLING_PARAMS_REG_OFFSET,
    0,
    0,
    642,
};

/******************************************************************************
 * Register: SBPM_REGS_SBPM_IIR_LOW
 ******************************************************************************/
const ru_reg_rec SBPM_REGS_SBPM_IIR_LOW_REG = 
{
    "REGS_SBPM_IIR_LOW",
#if RU_INCLUDE_DESC
    "SBPM_IIR_LOW_REGISTER Register",
    "SBPM IIR low (Interrupt information register)",
#endif
    SBPM_REGS_SBPM_IIR_LOW_REG_OFFSET,
    0,
    0,
    643,
};

/******************************************************************************
 * Register: SBPM_REGS_SBPM_IIR_HIGH
 ******************************************************************************/
const ru_reg_rec SBPM_REGS_SBPM_IIR_HIGH_REG = 
{
    "REGS_SBPM_IIR_HIGH",
#if RU_INCLUDE_DESC
    "SBPM_IIR_HIGH_REGISTER Register",
    "SBPM IIR high (Interrupt information register)",
#endif
    SBPM_REGS_SBPM_IIR_HIGH_REG_OFFSET,
    0,
    0,
    644,
};

/******************************************************************************
 * Register: SBPM_REGS_SBPM_DBG_VEC0
 ******************************************************************************/
const ru_reg_rec SBPM_REGS_SBPM_DBG_VEC0_REG = 
{
    "REGS_SBPM_DBG_VEC0",
#if RU_INCLUDE_DESC
    "SBPM_DBG_VEC0 Register",
    "SBPM debug vector0 includes 21 bit of control/state machine of CMD pipe"
    ""
    "",
#endif
    SBPM_REGS_SBPM_DBG_VEC0_REG_OFFSET,
    0,
    0,
    645,
};

/******************************************************************************
 * Register: SBPM_REGS_SBPM_DBG_VEC1
 ******************************************************************************/
const ru_reg_rec SBPM_REGS_SBPM_DBG_VEC1_REG = 
{
    "REGS_SBPM_DBG_VEC1",
#if RU_INCLUDE_DESC
    "SBPM_DBG_VEC1 Register",
    "SBPM debug vector1 includes 21 bit of control/state machine of CMD pipe"
    ""
    ""
    "",
#endif
    SBPM_REGS_SBPM_DBG_VEC1_REG_OFFSET,
    0,
    0,
    646,
};

/******************************************************************************
 * Register: SBPM_REGS_SBPM_DBG_VEC2
 ******************************************************************************/
const ru_reg_rec SBPM_REGS_SBPM_DBG_VEC2_REG = 
{
    "REGS_SBPM_DBG_VEC2",
#if RU_INCLUDE_DESC
    "SBPM_DBG_VEC2 Register",
    "This is one of the TX_handler debug vectors",
#endif
    SBPM_REGS_SBPM_DBG_VEC2_REG_OFFSET,
    0,
    0,
    647,
};

/******************************************************************************
 * Register: SBPM_REGS_SBPM_DBG_VEC3
 ******************************************************************************/
const ru_reg_rec SBPM_REGS_SBPM_DBG_VEC3_REG = 
{
    "REGS_SBPM_DBG_VEC3",
#if RU_INCLUDE_DESC
    "SBPM_DBG_VEC3 Register",
    "This is one of TX_handler debug vectors",
#endif
    SBPM_REGS_SBPM_DBG_VEC3_REG_OFFSET,
    0,
    0,
    648,
};

/******************************************************************************
 * Register: SBPM_REGS_SBPM_SP_BBH_LOW
 ******************************************************************************/
const ru_reg_rec SBPM_REGS_SBPM_SP_BBH_LOW_REG = 
{
    "REGS_SBPM_SP_BBH_LOW",
#if RU_INCLUDE_DESC
    "SBPM_SP_BBH_LOW Register",
    "This register mark all the SPs which are BBHs."
    "Each bit in this register, refers to a SP with the same index",
#endif
    SBPM_REGS_SBPM_SP_BBH_LOW_REG_OFFSET,
    0,
    0,
    649,
};

/******************************************************************************
 * Register: SBPM_REGS_SBPM_SP_BBH_HIGH
 ******************************************************************************/
const ru_reg_rec SBPM_REGS_SBPM_SP_BBH_HIGH_REG = 
{
    "REGS_SBPM_SP_BBH_HIGH",
#if RU_INCLUDE_DESC
    "SBPM_SP_BBH_HIGH Register",
    "This register mark all the SPs which are BBHs."
    "Each bit in this register, refers to a SP with the same index",
#endif
    SBPM_REGS_SBPM_SP_BBH_HIGH_REG_OFFSET,
    0,
    0,
    650,
};

/******************************************************************************
 * Register: SBPM_REGS_SBPM_SP_RNR_LOW
 ******************************************************************************/
const ru_reg_rec SBPM_REGS_SBPM_SP_RNR_LOW_REG = 
{
    "REGS_SBPM_SP_RNR_LOW",
#if RU_INCLUDE_DESC
    "SBPM_SP_RNR_LOW Register",
    "This register mark all the SPs which are runners."
    "Each bit in this register, refers to a SP with the same index",
#endif
    SBPM_REGS_SBPM_SP_RNR_LOW_REG_OFFSET,
    0,
    0,
    651,
};

/******************************************************************************
 * Register: SBPM_REGS_SBPM_SP_RNR_HIGH
 ******************************************************************************/
const ru_reg_rec SBPM_REGS_SBPM_SP_RNR_HIGH_REG = 
{
    "REGS_SBPM_SP_RNR_HIGH",
#if RU_INCLUDE_DESC
    "SBPM_SP_RNR_HIGH Register",
    "This register mark all the SPs which are runners."
    "Each bit in this register, refers to a SP with the same index",
#endif
    SBPM_REGS_SBPM_SP_RNR_HIGH_REG_OFFSET,
    0,
    0,
    652,
};

/******************************************************************************
 * Register: SBPM_REGS_SBPM_UG_MAP_LOW
 ******************************************************************************/
const ru_reg_rec SBPM_REGS_SBPM_UG_MAP_LOW_REG = 
{
    "REGS_SBPM_UG_MAP_LOW",
#if RU_INCLUDE_DESC
    "SBPM_UG_MAP_LOW Register",
    "bit i value determine if SP number i belongs to UG0 (ingress) or UG1 (egress)",
#endif
    SBPM_REGS_SBPM_UG_MAP_LOW_REG_OFFSET,
    0,
    0,
    653,
};

/******************************************************************************
 * Register: SBPM_REGS_SBPM_UG_MAP_HIGH
 ******************************************************************************/
const ru_reg_rec SBPM_REGS_SBPM_UG_MAP_HIGH_REG = 
{
    "REGS_SBPM_UG_MAP_HIGH",
#if RU_INCLUDE_DESC
    "SBPM_UG_MAP_HIGH Register",
    "bit i value determine if SP number i belongs to UG0 (ingress) or UG1 (egress)",
#endif
    SBPM_REGS_SBPM_UG_MAP_HIGH_REG_OFFSET,
    0,
    0,
    654,
};

/******************************************************************************
 * Register: SBPM_REGS_SBPM_NACK_MASK_LOW
 ******************************************************************************/
const ru_reg_rec SBPM_REGS_SBPM_NACK_MASK_LOW_REG = 
{
    "REGS_SBPM_NACK_MASK_LOW",
#if RU_INCLUDE_DESC
    "SBPM_NACK_MASK_LOW Register",
    "bit i value determine if SP number i got nack or not",
#endif
    SBPM_REGS_SBPM_NACK_MASK_LOW_REG_OFFSET,
    0,
    0,
    655,
};

/******************************************************************************
 * Register: SBPM_REGS_SBPM_NACK_MASK_HIGH
 ******************************************************************************/
const ru_reg_rec SBPM_REGS_SBPM_NACK_MASK_HIGH_REG = 
{
    "REGS_SBPM_NACK_MASK_HIGH",
#if RU_INCLUDE_DESC
    "SBPM_NACK_MASK_HIGH Register",
    "bit i value determine if SP number i got nack or not",
#endif
    SBPM_REGS_SBPM_NACK_MASK_HIGH_REG_OFFSET,
    0,
    0,
    656,
};

/******************************************************************************
 * Register: SBPM_REGS_SBPM_EXCL_MASK_LOW
 ******************************************************************************/
const ru_reg_rec SBPM_REGS_SBPM_EXCL_MASK_LOW_REG = 
{
    "REGS_SBPM_EXCL_MASK_LOW",
#if RU_INCLUDE_DESC
    "SBPM_EXCL_MASK_LOW Register",
    "This register mark all the SPs that should get exclusive messages",
#endif
    SBPM_REGS_SBPM_EXCL_MASK_LOW_REG_OFFSET,
    0,
    0,
    657,
};

/******************************************************************************
 * Register: SBPM_REGS_SBPM_EXCL_MASK_HIGH
 ******************************************************************************/
const ru_reg_rec SBPM_REGS_SBPM_EXCL_MASK_HIGH_REG = 
{
    "REGS_SBPM_EXCL_MASK_HIGH",
#if RU_INCLUDE_DESC
    "SBPM_EXCL_MASK_HIGH Register",
    "This register mark all the SPs that should get exclusive messages",
#endif
    SBPM_REGS_SBPM_EXCL_MASK_HIGH_REG_OFFSET,
    0,
    0,
    658,
};

/******************************************************************************
 * Register: SBPM_REGS_SBPM_RADDR_DECODER
 ******************************************************************************/
const ru_reg_rec SBPM_REGS_SBPM_RADDR_DECODER_REG = 
{
    "REGS_SBPM_RADDR_DECODER",
#if RU_INCLUDE_DESC
    "SBPM_RADDR_DECODER Register",
    "This register let you choose one user that you would like to change its default RA.",
#endif
    SBPM_REGS_SBPM_RADDR_DECODER_REG_OFFSET,
    0,
    0,
    659,
};

/******************************************************************************
 * Register: SBPM_REGS_SBPM_WR_DATA
 ******************************************************************************/
const ru_reg_rec SBPM_REGS_SBPM_WR_DATA_REG = 
{
    "REGS_SBPM_WR_DATA",
#if RU_INCLUDE_DESC
    "SBPM_WR_DATA Register",
    "If SW want to write a whole word into the SBPMs RAM, it needs first to write the data to this register and then, send connect request with the wr_req bit asserted, with the address (BN field).",
#endif
    SBPM_REGS_SBPM_WR_DATA_REG_OFFSET,
    0,
    0,
    660,
};

/******************************************************************************
 * Register: SBPM_REGS_SBPM_UG_BAC_MAX
 ******************************************************************************/
const ru_reg_rec SBPM_REGS_SBPM_UG_BAC_MAX_REG = 
{
    "REGS_SBPM_UG_BAC_MAX",
#if RU_INCLUDE_DESC
    "SBPM_UG_BAC_MAX Register",
    "This register tracks the max values of the UG counters. it can be reset/modified by SW.",
#endif
    SBPM_REGS_SBPM_UG_BAC_MAX_REG_OFFSET,
    0,
    0,
    661,
};

/******************************************************************************
 * Register: SBPM_REGS_SBPM_SPARE
 ******************************************************************************/
const ru_reg_rec SBPM_REGS_SBPM_SPARE_REG = 
{
    "REGS_SBPM_SPARE",
#if RU_INCLUDE_DESC
    "SBPM_SPARE Register",
    "sbpm spare register",
#endif
    SBPM_REGS_SBPM_SPARE_REG_OFFSET,
    0,
    0,
    662,
};

/******************************************************************************
 * Register: SBPM_INTR_CTRL_ISR
 ******************************************************************************/
const ru_reg_rec SBPM_INTR_CTRL_ISR_REG = 
{
    "INTR_CTRL_ISR",
#if RU_INCLUDE_DESC
    "INTERRUPT_STATUS_REGISTER Register",
    "This register contains the current active TM interrupts. Each asserted bit represents an active interrupt source. The interrupt remains active until the software clears it by writing 1 to the corresponding bit.",
#endif
    SBPM_INTR_CTRL_ISR_REG_OFFSET,
    0,
    0,
    663,
};

/******************************************************************************
 * Register: SBPM_INTR_CTRL_ISM
 ******************************************************************************/
const ru_reg_rec SBPM_INTR_CTRL_ISM_REG = 
{
    "INTR_CTRL_ISM",
#if RU_INCLUDE_DESC
    "INTERRUPT_STATUS_MASKED_REGISTER Register",
    "This register provides only the  enabled interrupts for each of the interrupt sources depicted in the ISR register.",
#endif
    SBPM_INTR_CTRL_ISM_REG_OFFSET,
    0,
    0,
    664,
};

/******************************************************************************
 * Register: SBPM_INTR_CTRL_IER
 ******************************************************************************/
const ru_reg_rec SBPM_INTR_CTRL_IER_REG = 
{
    "INTR_CTRL_IER",
#if RU_INCLUDE_DESC
    "INTERRUPT_ENABLE_REGISTER Register",
    "This register provides an enable mask for each of the interrupt sources depicted in the ISR register.",
#endif
    SBPM_INTR_CTRL_IER_REG_OFFSET,
    0,
    0,
    665,
};

/******************************************************************************
 * Register: SBPM_INTR_CTRL_ITR
 ******************************************************************************/
const ru_reg_rec SBPM_INTR_CTRL_ITR_REG = 
{
    "INTR_CTRL_ITR",
#if RU_INCLUDE_DESC
    "INTERRUPT_TEST_REGISTER Register",
    "This register enables testing by simulating interrupt sources. When the software sets a bit in the ITR, the corresponding bit in the ISR shows an active interrupt. The interrupt remains active until software clears the bit in the ITR",
#endif
    SBPM_INTR_CTRL_ITR_REG_OFFSET,
    0,
    0,
    666,
};

/******************************************************************************
 * Block: SBPM
 ******************************************************************************/
static const ru_reg_rec *SBPM_REGS[] =
{
    &SBPM_REGS_INIT_FREE_LIST_REG,
    &SBPM_REGS_BN_ALLOC_REG,
    &SBPM_REGS_BN_ALLOC_RPLY_REG,
    &SBPM_REGS_BN_FREE_WITH_CONTXT_LOW_REG,
    &SBPM_REGS_BN_FREE_WITH_CONTXT_HIGH_REG,
    &SBPM_REGS_MCST_INC_REG,
    &SBPM_REGS_MCST_INC_RPLY_REG,
    &SBPM_REGS_BN_CONNECT_REG,
    &SBPM_REGS_BN_CONNECT_RPLY_REG,
    &SBPM_REGS_GET_NEXT_REG,
    &SBPM_REGS_GET_NEXT_RPLY_REG,
    &SBPM_REGS_SBPM_CLK_GATE_CNTRL_REG,
    &SBPM_REGS_BN_FREE_WITHOUT_CONTXT_REG,
    &SBPM_REGS_BN_FREE_WITHOUT_CONTXT_RPLY_REG,
    &SBPM_REGS_BN_FREE_WITH_CONTXT_RPLY_REG,
    &SBPM_REGS_SBPM_GL_TRSH_REG,
    &SBPM_REGS_SBPM_UG0_TRSH_REG,
    &SBPM_REGS_SBPM_UG1_TRSH_REG,
    &SBPM_REGS_SBPM_DBG_REG,
    &SBPM_REGS_SBPM_UG0_BAC_REG,
    &SBPM_REGS_SBPM_UG1_BAC_REG,
    &SBPM_REGS_SBPM_GL_BAC_REG,
    &SBPM_REGS_SBPM_UG0_EXCL_HIGH_TRSH_REG,
    &SBPM_REGS_SBPM_UG1_EXCL_HIGH_TRSH_REG,
    &SBPM_REGS_SBPM_UG0_EXCL_LOW_TRSH_REG,
    &SBPM_REGS_SBPM_UG1_EXCL_LOW_TRSH_REG,
    &SBPM_REGS_SBPM_UG_STATUS_REG,
    &SBPM_REGS_ERROR_HANDLING_PARAMS_REG,
    &SBPM_REGS_SBPM_IIR_LOW_REG,
    &SBPM_REGS_SBPM_IIR_HIGH_REG,
    &SBPM_REGS_SBPM_DBG_VEC0_REG,
    &SBPM_REGS_SBPM_DBG_VEC1_REG,
    &SBPM_REGS_SBPM_DBG_VEC2_REG,
    &SBPM_REGS_SBPM_DBG_VEC3_REG,
    &SBPM_REGS_SBPM_SP_BBH_LOW_REG,
    &SBPM_REGS_SBPM_SP_BBH_HIGH_REG,
    &SBPM_REGS_SBPM_SP_RNR_LOW_REG,
    &SBPM_REGS_SBPM_SP_RNR_HIGH_REG,
    &SBPM_REGS_SBPM_UG_MAP_LOW_REG,
    &SBPM_REGS_SBPM_UG_MAP_HIGH_REG,
    &SBPM_REGS_SBPM_NACK_MASK_LOW_REG,
    &SBPM_REGS_SBPM_NACK_MASK_HIGH_REG,
    &SBPM_REGS_SBPM_EXCL_MASK_LOW_REG,
    &SBPM_REGS_SBPM_EXCL_MASK_HIGH_REG,
    &SBPM_REGS_SBPM_RADDR_DECODER_REG,
    &SBPM_REGS_SBPM_WR_DATA_REG,
    &SBPM_REGS_SBPM_UG_BAC_MAX_REG,
    &SBPM_REGS_SBPM_SPARE_REG,
    &SBPM_INTR_CTRL_ISR_REG,
    &SBPM_INTR_CTRL_ISM_REG,
    &SBPM_INTR_CTRL_IER_REG,
    &SBPM_INTR_CTRL_ITR_REG,
};

unsigned long SBPM_ADDRS[] =
{
    0x82d2c000,
};

const ru_block_rec SBPM_BLOCK = 
{
    "SBPM",
    SBPM_ADDRS,
    1,
    52,
    SBPM_REGS
};

/* End of file XRDP_SBPM.c */
