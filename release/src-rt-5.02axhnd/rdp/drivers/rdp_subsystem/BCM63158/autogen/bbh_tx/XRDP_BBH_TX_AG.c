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

#if RU_INCLUDE_FIELD_DB
/******************************************************************************
 * Field: BBH_TX_COMMON_CONFIGURATIONS_MACTYPE_TYPE
 ******************************************************************************/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_MACTYPE_TYPE_FIELD =
{
    "TYPE",
#if RU_INCLUDE_DESC
    "MAC_type",
    "MAC type",
#endif
    BBH_TX_COMMON_CONFIGURATIONS_MACTYPE_TYPE_FIELD_MASK,
    0,
    BBH_TX_COMMON_CONFIGURATIONS_MACTYPE_TYPE_FIELD_WIDTH,
    BBH_TX_COMMON_CONFIGURATIONS_MACTYPE_TYPE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_COMMON_CONFIGURATIONS_MACTYPE_RESERVED0
 ******************************************************************************/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_MACTYPE_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    BBH_TX_COMMON_CONFIGURATIONS_MACTYPE_RESERVED0_FIELD_MASK,
    0,
    BBH_TX_COMMON_CONFIGURATIONS_MACTYPE_RESERVED0_FIELD_WIDTH,
    BBH_TX_COMMON_CONFIGURATIONS_MACTYPE_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_COMMON_CONFIGURATIONS_BBCFG_1_TX_DMASRC
 ******************************************************************************/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_BBCFG_1_TX_DMASRC_FIELD =
{
    "DMASRC",
#if RU_INCLUDE_DESC
    "DMA_source_id",
    "source id. This id is used to determine the route to the module.",
#endif
    BBH_TX_COMMON_CONFIGURATIONS_BBCFG_1_TX_DMASRC_FIELD_MASK,
    0,
    BBH_TX_COMMON_CONFIGURATIONS_BBCFG_1_TX_DMASRC_FIELD_WIDTH,
    BBH_TX_COMMON_CONFIGURATIONS_BBCFG_1_TX_DMASRC_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_COMMON_CONFIGURATIONS_BBCFG_1_TX_RESERVED0
 ******************************************************************************/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_BBCFG_1_TX_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    BBH_TX_COMMON_CONFIGURATIONS_BBCFG_1_TX_RESERVED0_FIELD_MASK,
    0,
    BBH_TX_COMMON_CONFIGURATIONS_BBCFG_1_TX_RESERVED0_FIELD_WIDTH,
    BBH_TX_COMMON_CONFIGURATIONS_BBCFG_1_TX_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_COMMON_CONFIGURATIONS_BBCFG_1_TX_SDMASRC
 ******************************************************************************/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_BBCFG_1_TX_SDMASRC_FIELD =
{
    "SDMASRC",
#if RU_INCLUDE_DESC
    "SDMA_source_id",
    "source id. This id is used to determine the route to the module.",
#endif
    BBH_TX_COMMON_CONFIGURATIONS_BBCFG_1_TX_SDMASRC_FIELD_MASK,
    0,
    BBH_TX_COMMON_CONFIGURATIONS_BBCFG_1_TX_SDMASRC_FIELD_WIDTH,
    BBH_TX_COMMON_CONFIGURATIONS_BBCFG_1_TX_SDMASRC_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_COMMON_CONFIGURATIONS_BBCFG_1_TX_RESERVED1
 ******************************************************************************/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_BBCFG_1_TX_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    BBH_TX_COMMON_CONFIGURATIONS_BBCFG_1_TX_RESERVED1_FIELD_MASK,
    0,
    BBH_TX_COMMON_CONFIGURATIONS_BBCFG_1_TX_RESERVED1_FIELD_WIDTH,
    BBH_TX_COMMON_CONFIGURATIONS_BBCFG_1_TX_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_COMMON_CONFIGURATIONS_BBCFG_1_TX_SBPMSRC
 ******************************************************************************/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_BBCFG_1_TX_SBPMSRC_FIELD =
{
    "SBPMSRC",
#if RU_INCLUDE_DESC
    "SBPM_source_id",
    "source id. This id is used to determine the route to the module.",
#endif
    BBH_TX_COMMON_CONFIGURATIONS_BBCFG_1_TX_SBPMSRC_FIELD_MASK,
    0,
    BBH_TX_COMMON_CONFIGURATIONS_BBCFG_1_TX_SBPMSRC_FIELD_WIDTH,
    BBH_TX_COMMON_CONFIGURATIONS_BBCFG_1_TX_SBPMSRC_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_COMMON_CONFIGURATIONS_BBCFG_1_TX_RESERVED2
 ******************************************************************************/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_BBCFG_1_TX_RESERVED2_FIELD =
{
    "RESERVED2",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    BBH_TX_COMMON_CONFIGURATIONS_BBCFG_1_TX_RESERVED2_FIELD_MASK,
    0,
    BBH_TX_COMMON_CONFIGURATIONS_BBCFG_1_TX_RESERVED2_FIELD_WIDTH,
    BBH_TX_COMMON_CONFIGURATIONS_BBCFG_1_TX_RESERVED2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_COMMON_CONFIGURATIONS_BBCFG_1_TX_FPMSRC
 ******************************************************************************/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_BBCFG_1_TX_FPMSRC_FIELD =
{
    "FPMSRC",
#if RU_INCLUDE_DESC
    "FPM_source_id",
    "source id. This id is used to determine the route to the module.",
#endif
    BBH_TX_COMMON_CONFIGURATIONS_BBCFG_1_TX_FPMSRC_FIELD_MASK,
    0,
    BBH_TX_COMMON_CONFIGURATIONS_BBCFG_1_TX_FPMSRC_FIELD_WIDTH,
    BBH_TX_COMMON_CONFIGURATIONS_BBCFG_1_TX_FPMSRC_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_COMMON_CONFIGURATIONS_BBCFG_1_TX_RESERVED3
 ******************************************************************************/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_BBCFG_1_TX_RESERVED3_FIELD =
{
    "RESERVED3",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    BBH_TX_COMMON_CONFIGURATIONS_BBCFG_1_TX_RESERVED3_FIELD_MASK,
    0,
    BBH_TX_COMMON_CONFIGURATIONS_BBCFG_1_TX_RESERVED3_FIELD_WIDTH,
    BBH_TX_COMMON_CONFIGURATIONS_BBCFG_1_TX_RESERVED3_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_COMMON_CONFIGURATIONS_BBCFG_2_TX_PDRNR0SRC
 ******************************************************************************/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_BBCFG_2_TX_PDRNR0SRC_FIELD =
{
    "PDRNR0SRC",
#if RU_INCLUDE_DESC
    "pd_runner0_source_id",
    "source id. This id is used to determine the route to the 1st (out of possible 2 runners) which are responsible for sending PDs.",
#endif
    BBH_TX_COMMON_CONFIGURATIONS_BBCFG_2_TX_PDRNR0SRC_FIELD_MASK,
    0,
    BBH_TX_COMMON_CONFIGURATIONS_BBCFG_2_TX_PDRNR0SRC_FIELD_WIDTH,
    BBH_TX_COMMON_CONFIGURATIONS_BBCFG_2_TX_PDRNR0SRC_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_COMMON_CONFIGURATIONS_BBCFG_2_TX_RESERVED0
 ******************************************************************************/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_BBCFG_2_TX_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    BBH_TX_COMMON_CONFIGURATIONS_BBCFG_2_TX_RESERVED0_FIELD_MASK,
    0,
    BBH_TX_COMMON_CONFIGURATIONS_BBCFG_2_TX_RESERVED0_FIELD_WIDTH,
    BBH_TX_COMMON_CONFIGURATIONS_BBCFG_2_TX_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_COMMON_CONFIGURATIONS_BBCFG_2_TX_PDRNR1SRC
 ******************************************************************************/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_BBCFG_2_TX_PDRNR1SRC_FIELD =
{
    "PDRNR1SRC",
#if RU_INCLUDE_DESC
    "pd_runner1_source_id",
    "source id. This id is used to determine the route to the 2nd (out of possible 2 runners) which are responsible for sending PDs.",
#endif
    BBH_TX_COMMON_CONFIGURATIONS_BBCFG_2_TX_PDRNR1SRC_FIELD_MASK,
    0,
    BBH_TX_COMMON_CONFIGURATIONS_BBCFG_2_TX_PDRNR1SRC_FIELD_WIDTH,
    BBH_TX_COMMON_CONFIGURATIONS_BBCFG_2_TX_PDRNR1SRC_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_COMMON_CONFIGURATIONS_BBCFG_2_TX_RESERVED1
 ******************************************************************************/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_BBCFG_2_TX_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    BBH_TX_COMMON_CONFIGURATIONS_BBCFG_2_TX_RESERVED1_FIELD_MASK,
    0,
    BBH_TX_COMMON_CONFIGURATIONS_BBCFG_2_TX_RESERVED1_FIELD_WIDTH,
    BBH_TX_COMMON_CONFIGURATIONS_BBCFG_2_TX_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_COMMON_CONFIGURATIONS_BBCFG_2_TX_STSRNRSRC
 ******************************************************************************/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_BBCFG_2_TX_STSRNRSRC_FIELD =
{
    "STSRNRSRC",
#if RU_INCLUDE_DESC
    "Status_Runner_source_id",
    "source id. This id is used to determine the route to the Runner that is responsible for sending status messages (WAN only).",
#endif
    BBH_TX_COMMON_CONFIGURATIONS_BBCFG_2_TX_STSRNRSRC_FIELD_MASK,
    0,
    BBH_TX_COMMON_CONFIGURATIONS_BBCFG_2_TX_STSRNRSRC_FIELD_WIDTH,
    BBH_TX_COMMON_CONFIGURATIONS_BBCFG_2_TX_STSRNRSRC_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_COMMON_CONFIGURATIONS_BBCFG_2_TX_RESERVED2
 ******************************************************************************/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_BBCFG_2_TX_RESERVED2_FIELD =
{
    "RESERVED2",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    BBH_TX_COMMON_CONFIGURATIONS_BBCFG_2_TX_RESERVED2_FIELD_MASK,
    0,
    BBH_TX_COMMON_CONFIGURATIONS_BBCFG_2_TX_RESERVED2_FIELD_WIDTH,
    BBH_TX_COMMON_CONFIGURATIONS_BBCFG_2_TX_RESERVED2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_COMMON_CONFIGURATIONS_BBCFG_2_TX_MSGRNRSRC
 ******************************************************************************/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_BBCFG_2_TX_MSGRNRSRC_FIELD =
{
    "MSGRNRSRC",
#if RU_INCLUDE_DESC
    "Message_Runner_source_id",
    "source id. This id is used to determine the route to the Runner which is responsible for sending DBR/Ghost messages (WAN only).",
#endif
    BBH_TX_COMMON_CONFIGURATIONS_BBCFG_2_TX_MSGRNRSRC_FIELD_MASK,
    0,
    BBH_TX_COMMON_CONFIGURATIONS_BBCFG_2_TX_MSGRNRSRC_FIELD_WIDTH,
    BBH_TX_COMMON_CONFIGURATIONS_BBCFG_2_TX_MSGRNRSRC_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_COMMON_CONFIGURATIONS_BBCFG_2_TX_RESERVED3
 ******************************************************************************/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_BBCFG_2_TX_RESERVED3_FIELD =
{
    "RESERVED3",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    BBH_TX_COMMON_CONFIGURATIONS_BBCFG_2_TX_RESERVED3_FIELD_MASK,
    0,
    BBH_TX_COMMON_CONFIGURATIONS_BBCFG_2_TX_RESERVED3_FIELD_WIDTH,
    BBH_TX_COMMON_CONFIGURATIONS_BBCFG_2_TX_RESERVED3_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_COMMON_CONFIGURATIONS_DDRCFG_TX_BUFSIZE
 ******************************************************************************/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_DDRCFG_TX_BUFSIZE_FIELD =
{
    "BUFSIZE",
#if RU_INCLUDE_DESC
    "DDR_buffer_size",
    "The data is arranged in the DDR in a fixed size buffers.",
#endif
    BBH_TX_COMMON_CONFIGURATIONS_DDRCFG_TX_BUFSIZE_FIELD_MASK,
    0,
    BBH_TX_COMMON_CONFIGURATIONS_DDRCFG_TX_BUFSIZE_FIELD_WIDTH,
    BBH_TX_COMMON_CONFIGURATIONS_DDRCFG_TX_BUFSIZE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_COMMON_CONFIGURATIONS_DDRCFG_TX_BYTERESUL
 ******************************************************************************/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_DDRCFG_TX_BYTERESUL_FIELD =
{
    "BYTERESUL",
#if RU_INCLUDE_DESC
    "PO_bytes_resulotion",
    "The packet offset byte resulotion.",
#endif
    BBH_TX_COMMON_CONFIGURATIONS_DDRCFG_TX_BYTERESUL_FIELD_MASK,
    0,
    BBH_TX_COMMON_CONFIGURATIONS_DDRCFG_TX_BYTERESUL_FIELD_WIDTH,
    BBH_TX_COMMON_CONFIGURATIONS_DDRCFG_TX_BYTERESUL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_COMMON_CONFIGURATIONS_DDRCFG_TX_DDRTXOFFSET
 ******************************************************************************/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_DDRCFG_TX_DDRTXOFFSET_FIELD =
{
    "DDRTXOFFSET",
#if RU_INCLUDE_DESC
    "DDR_tx_offset",
    "Static offset in 8-bytes resolution for non aggregated packets in DDR",
#endif
    BBH_TX_COMMON_CONFIGURATIONS_DDRCFG_TX_DDRTXOFFSET_FIELD_MASK,
    0,
    BBH_TX_COMMON_CONFIGURATIONS_DDRCFG_TX_DDRTXOFFSET_FIELD_WIDTH,
    BBH_TX_COMMON_CONFIGURATIONS_DDRCFG_TX_DDRTXOFFSET_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_COMMON_CONFIGURATIONS_DDRCFG_TX_RESERVED0
 ******************************************************************************/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_DDRCFG_TX_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    BBH_TX_COMMON_CONFIGURATIONS_DDRCFG_TX_RESERVED0_FIELD_MASK,
    0,
    BBH_TX_COMMON_CONFIGURATIONS_DDRCFG_TX_RESERVED0_FIELD_WIDTH,
    BBH_TX_COMMON_CONFIGURATIONS_DDRCFG_TX_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_COMMON_CONFIGURATIONS_DDRCFG_TX_HNSIZE0
 ******************************************************************************/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_DDRCFG_TX_HNSIZE0_FIELD =
{
    "HNSIZE0",
#if RU_INCLUDE_DESC
    "HN_size_0",
    "The size of the HN (Header number) in bytes. The BBH decides between size 0 and size 1 according to a bit in the PD",
#endif
    BBH_TX_COMMON_CONFIGURATIONS_DDRCFG_TX_HNSIZE0_FIELD_MASK,
    0,
    BBH_TX_COMMON_CONFIGURATIONS_DDRCFG_TX_HNSIZE0_FIELD_WIDTH,
    BBH_TX_COMMON_CONFIGURATIONS_DDRCFG_TX_HNSIZE0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_COMMON_CONFIGURATIONS_DDRCFG_TX_RESERVED1
 ******************************************************************************/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_DDRCFG_TX_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    BBH_TX_COMMON_CONFIGURATIONS_DDRCFG_TX_RESERVED1_FIELD_MASK,
    0,
    BBH_TX_COMMON_CONFIGURATIONS_DDRCFG_TX_RESERVED1_FIELD_WIDTH,
    BBH_TX_COMMON_CONFIGURATIONS_DDRCFG_TX_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_COMMON_CONFIGURATIONS_DDRCFG_TX_HNSIZE1
 ******************************************************************************/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_DDRCFG_TX_HNSIZE1_FIELD =
{
    "HNSIZE1",
#if RU_INCLUDE_DESC
    "HN_size_1",
    "The size of the HN (Header number) in bytes. The BBH decides between size 0 and size 1 according to a bit in the PD",
#endif
    BBH_TX_COMMON_CONFIGURATIONS_DDRCFG_TX_HNSIZE1_FIELD_MASK,
    0,
    BBH_TX_COMMON_CONFIGURATIONS_DDRCFG_TX_HNSIZE1_FIELD_WIDTH,
    BBH_TX_COMMON_CONFIGURATIONS_DDRCFG_TX_HNSIZE1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_COMMON_CONFIGURATIONS_DDRCFG_TX_RESERVED2
 ******************************************************************************/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_DDRCFG_TX_RESERVED2_FIELD =
{
    "RESERVED2",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    BBH_TX_COMMON_CONFIGURATIONS_DDRCFG_TX_RESERVED2_FIELD_MASK,
    0,
    BBH_TX_COMMON_CONFIGURATIONS_DDRCFG_TX_RESERVED2_FIELD_WIDTH,
    BBH_TX_COMMON_CONFIGURATIONS_DDRCFG_TX_RESERVED2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_COMMON_CONFIGURATIONS_RNRCFG_1_TCONTADDR
 ******************************************************************************/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_RNRCFG_1_TCONTADDR_FIELD =
{
    "TCONTADDR",
#if RU_INCLUDE_DESC
    "TCONT_address",
    "Defines the TCONT address within the Runner address space."
    "The address is in 8 bytes resolution."
    "",
#endif
    BBH_TX_COMMON_CONFIGURATIONS_RNRCFG_1_TCONTADDR_FIELD_MASK,
    0,
    BBH_TX_COMMON_CONFIGURATIONS_RNRCFG_1_TCONTADDR_FIELD_WIDTH,
    BBH_TX_COMMON_CONFIGURATIONS_RNRCFG_1_TCONTADDR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_COMMON_CONFIGURATIONS_RNRCFG_1_SKBADDR
 ******************************************************************************/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_RNRCFG_1_SKBADDR_FIELD =
{
    "SKBADDR",
#if RU_INCLUDE_DESC
    "SKB_address",
    "Defines the SKB free address within the Runner address space."
    "The address is in 8-bytes resolution."
    "",
#endif
    BBH_TX_COMMON_CONFIGURATIONS_RNRCFG_1_SKBADDR_FIELD_MASK,
    0,
    BBH_TX_COMMON_CONFIGURATIONS_RNRCFG_1_SKBADDR_FIELD_WIDTH,
    BBH_TX_COMMON_CONFIGURATIONS_RNRCFG_1_SKBADDR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_COMMON_CONFIGURATIONS_RNRCFG_2_PTRADDR
 ******************************************************************************/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_RNRCFG_2_PTRADDR_FIELD =
{
    "PTRADDR",
#if RU_INCLUDE_DESC
    "PTRADDR",
    "This field defins the address in the Runner memory space to which the read pointer is written."
    "The address is in 8-bytes resolution.",
#endif
    BBH_TX_COMMON_CONFIGURATIONS_RNRCFG_2_PTRADDR_FIELD_MASK,
    0,
    BBH_TX_COMMON_CONFIGURATIONS_RNRCFG_2_PTRADDR_FIELD_WIDTH,
    BBH_TX_COMMON_CONFIGURATIONS_RNRCFG_2_PTRADDR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_COMMON_CONFIGURATIONS_RNRCFG_2_TASK
 ******************************************************************************/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_RNRCFG_2_TASK_FIELD =
{
    "TASK",
#if RU_INCLUDE_DESC
    "Task_number",
    "The number of the task that is responsible for sending PDs to the BBH",
#endif
    BBH_TX_COMMON_CONFIGURATIONS_RNRCFG_2_TASK_FIELD_MASK,
    0,
    BBH_TX_COMMON_CONFIGURATIONS_RNRCFG_2_TASK_FIELD_WIDTH,
    BBH_TX_COMMON_CONFIGURATIONS_RNRCFG_2_TASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_COMMON_CONFIGURATIONS_RNRCFG_2_RESERVED0
 ******************************************************************************/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_RNRCFG_2_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    BBH_TX_COMMON_CONFIGURATIONS_RNRCFG_2_RESERVED0_FIELD_MASK,
    0,
    BBH_TX_COMMON_CONFIGURATIONS_RNRCFG_2_RESERVED0_FIELD_WIDTH,
    BBH_TX_COMMON_CONFIGURATIONS_RNRCFG_2_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_COMMON_CONFIGURATIONS_DMACFG_TX_DESCBASE
 ******************************************************************************/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_DMACFG_TX_DESCBASE_FIELD =
{
    "DESCBASE",
#if RU_INCLUDE_DESC
    "Descriptor_FIFO_base",
    "Defines the base address of the read request FIFO within the DMA address space."
    "The value should be identical to the relevant configuration in the DMA.",
#endif
    BBH_TX_COMMON_CONFIGURATIONS_DMACFG_TX_DESCBASE_FIELD_MASK,
    0,
    BBH_TX_COMMON_CONFIGURATIONS_DMACFG_TX_DESCBASE_FIELD_WIDTH,
    BBH_TX_COMMON_CONFIGURATIONS_DMACFG_TX_DESCBASE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_COMMON_CONFIGURATIONS_DMACFG_TX_DESCSIZE
 ******************************************************************************/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_DMACFG_TX_DESCSIZE_FIELD =
{
    "DESCSIZE",
#if RU_INCLUDE_DESC
    "Descriptor_FIFO_size",
    "The size of the BBH read requests FIFO inside the DMA",
#endif
    BBH_TX_COMMON_CONFIGURATIONS_DMACFG_TX_DESCSIZE_FIELD_MASK,
    0,
    BBH_TX_COMMON_CONFIGURATIONS_DMACFG_TX_DESCSIZE_FIELD_WIDTH,
    BBH_TX_COMMON_CONFIGURATIONS_DMACFG_TX_DESCSIZE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_COMMON_CONFIGURATIONS_DMACFG_TX_RESERVED0
 ******************************************************************************/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_DMACFG_TX_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    BBH_TX_COMMON_CONFIGURATIONS_DMACFG_TX_RESERVED0_FIELD_MASK,
    0,
    BBH_TX_COMMON_CONFIGURATIONS_DMACFG_TX_RESERVED0_FIELD_WIDTH,
    BBH_TX_COMMON_CONFIGURATIONS_DMACFG_TX_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_COMMON_CONFIGURATIONS_DMACFG_TX_MAXREQ
 ******************************************************************************/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_DMACFG_TX_MAXREQ_FIELD =
{
    "MAXREQ",
#if RU_INCLUDE_DESC
    "Maximum_number_of_requests",
    "Defines the maximum allowed number of on-the-fly read requests.",
#endif
    BBH_TX_COMMON_CONFIGURATIONS_DMACFG_TX_MAXREQ_FIELD_MASK,
    0,
    BBH_TX_COMMON_CONFIGURATIONS_DMACFG_TX_MAXREQ_FIELD_WIDTH,
    BBH_TX_COMMON_CONFIGURATIONS_DMACFG_TX_MAXREQ_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_COMMON_CONFIGURATIONS_DMACFG_TX_RESERVED1
 ******************************************************************************/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_DMACFG_TX_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    BBH_TX_COMMON_CONFIGURATIONS_DMACFG_TX_RESERVED1_FIELD_MASK,
    0,
    BBH_TX_COMMON_CONFIGURATIONS_DMACFG_TX_RESERVED1_FIELD_WIDTH,
    BBH_TX_COMMON_CONFIGURATIONS_DMACFG_TX_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_COMMON_CONFIGURATIONS_DMACFG_TX_EPNURGNT
 ******************************************************************************/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_DMACFG_TX_EPNURGNT_FIELD =
{
    "EPNURGNT",
#if RU_INCLUDE_DESC
    "Epon_read_urgent",
    "When asserted, this bit forces urgent priority on the EPON read requests towards the DMA (relevant only for EPON BBH)",
#endif
    BBH_TX_COMMON_CONFIGURATIONS_DMACFG_TX_EPNURGNT_FIELD_MASK,
    0,
    BBH_TX_COMMON_CONFIGURATIONS_DMACFG_TX_EPNURGNT_FIELD_WIDTH,
    BBH_TX_COMMON_CONFIGURATIONS_DMACFG_TX_EPNURGNT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_COMMON_CONFIGURATIONS_DMACFG_TX_RESERVED2
 ******************************************************************************/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_DMACFG_TX_RESERVED2_FIELD =
{
    "RESERVED2",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    BBH_TX_COMMON_CONFIGURATIONS_DMACFG_TX_RESERVED2_FIELD_MASK,
    0,
    BBH_TX_COMMON_CONFIGURATIONS_DMACFG_TX_RESERVED2_FIELD_WIDTH,
    BBH_TX_COMMON_CONFIGURATIONS_DMACFG_TX_RESERVED2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_COMMON_CONFIGURATIONS_SDMACFG_TX_DESCBASE
 ******************************************************************************/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_SDMACFG_TX_DESCBASE_FIELD =
{
    "DESCBASE",
#if RU_INCLUDE_DESC
    "Descriptor_FIFO_base",
    "Defines the base address of the read request FIFO within the DMA address space."
    "The value should be identical to the relevant configuration in the DMA.",
#endif
    BBH_TX_COMMON_CONFIGURATIONS_SDMACFG_TX_DESCBASE_FIELD_MASK,
    0,
    BBH_TX_COMMON_CONFIGURATIONS_SDMACFG_TX_DESCBASE_FIELD_WIDTH,
    BBH_TX_COMMON_CONFIGURATIONS_SDMACFG_TX_DESCBASE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_COMMON_CONFIGURATIONS_SDMACFG_TX_DESCSIZE
 ******************************************************************************/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_SDMACFG_TX_DESCSIZE_FIELD =
{
    "DESCSIZE",
#if RU_INCLUDE_DESC
    "Descriptor_FIFO_size",
    "The size of the BBH read requests FIFO inside the DMA",
#endif
    BBH_TX_COMMON_CONFIGURATIONS_SDMACFG_TX_DESCSIZE_FIELD_MASK,
    0,
    BBH_TX_COMMON_CONFIGURATIONS_SDMACFG_TX_DESCSIZE_FIELD_WIDTH,
    BBH_TX_COMMON_CONFIGURATIONS_SDMACFG_TX_DESCSIZE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_COMMON_CONFIGURATIONS_SDMACFG_TX_RESERVED0
 ******************************************************************************/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_SDMACFG_TX_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    BBH_TX_COMMON_CONFIGURATIONS_SDMACFG_TX_RESERVED0_FIELD_MASK,
    0,
    BBH_TX_COMMON_CONFIGURATIONS_SDMACFG_TX_RESERVED0_FIELD_WIDTH,
    BBH_TX_COMMON_CONFIGURATIONS_SDMACFG_TX_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_COMMON_CONFIGURATIONS_SDMACFG_TX_MAXREQ
 ******************************************************************************/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_SDMACFG_TX_MAXREQ_FIELD =
{
    "MAXREQ",
#if RU_INCLUDE_DESC
    "Maximum_number_of_requests",
    "Defines the maximum allowed number of on-the-fly read requests.",
#endif
    BBH_TX_COMMON_CONFIGURATIONS_SDMACFG_TX_MAXREQ_FIELD_MASK,
    0,
    BBH_TX_COMMON_CONFIGURATIONS_SDMACFG_TX_MAXREQ_FIELD_WIDTH,
    BBH_TX_COMMON_CONFIGURATIONS_SDMACFG_TX_MAXREQ_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_COMMON_CONFIGURATIONS_SDMACFG_TX_RESERVED1
 ******************************************************************************/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_SDMACFG_TX_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    BBH_TX_COMMON_CONFIGURATIONS_SDMACFG_TX_RESERVED1_FIELD_MASK,
    0,
    BBH_TX_COMMON_CONFIGURATIONS_SDMACFG_TX_RESERVED1_FIELD_WIDTH,
    BBH_TX_COMMON_CONFIGURATIONS_SDMACFG_TX_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_COMMON_CONFIGURATIONS_SDMACFG_TX_EPNURGNT
 ******************************************************************************/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_SDMACFG_TX_EPNURGNT_FIELD =
{
    "EPNURGNT",
#if RU_INCLUDE_DESC
    "Epon_read_urgent",
    "When asserted, this bit forces urgent priority on the EPON read requests towards the DMA (relevant only for EPON BBH)",
#endif
    BBH_TX_COMMON_CONFIGURATIONS_SDMACFG_TX_EPNURGNT_FIELD_MASK,
    0,
    BBH_TX_COMMON_CONFIGURATIONS_SDMACFG_TX_EPNURGNT_FIELD_WIDTH,
    BBH_TX_COMMON_CONFIGURATIONS_SDMACFG_TX_EPNURGNT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_COMMON_CONFIGURATIONS_SDMACFG_TX_RESERVED2
 ******************************************************************************/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_SDMACFG_TX_RESERVED2_FIELD =
{
    "RESERVED2",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    BBH_TX_COMMON_CONFIGURATIONS_SDMACFG_TX_RESERVED2_FIELD_MASK,
    0,
    BBH_TX_COMMON_CONFIGURATIONS_SDMACFG_TX_RESERVED2_FIELD_WIDTH,
    BBH_TX_COMMON_CONFIGURATIONS_SDMACFG_TX_RESERVED2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_COMMON_CONFIGURATIONS_SBPMCFG_FREENOCNTXT
 ******************************************************************************/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_SBPMCFG_FREENOCNTXT_FIELD =
{
    "FREENOCNTXT",
#if RU_INCLUDE_DESC
    "Free_without_context_en",
    "When this bit is enabled, the BBH will use free without context command.",
#endif
    BBH_TX_COMMON_CONFIGURATIONS_SBPMCFG_FREENOCNTXT_FIELD_MASK,
    0,
    BBH_TX_COMMON_CONFIGURATIONS_SBPMCFG_FREENOCNTXT_FIELD_WIDTH,
    BBH_TX_COMMON_CONFIGURATIONS_SBPMCFG_FREENOCNTXT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_COMMON_CONFIGURATIONS_SBPMCFG_SPECIALFREE
 ******************************************************************************/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_SBPMCFG_SPECIALFREE_FIELD =
{
    "SPECIALFREE",
#if RU_INCLUDE_DESC
    "Special_free_with_context_en",
    "When this bit is enabled, the BBH will use special free with context command."
    "This bit is relevant only if free without context_en is configured to 0.",
#endif
    BBH_TX_COMMON_CONFIGURATIONS_SBPMCFG_SPECIALFREE_FIELD_MASK,
    0,
    BBH_TX_COMMON_CONFIGURATIONS_SBPMCFG_SPECIALFREE_FIELD_WIDTH,
    BBH_TX_COMMON_CONFIGURATIONS_SBPMCFG_SPECIALFREE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_COMMON_CONFIGURATIONS_SBPMCFG_RESERVED0
 ******************************************************************************/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_SBPMCFG_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    BBH_TX_COMMON_CONFIGURATIONS_SBPMCFG_RESERVED0_FIELD_MASK,
    0,
    BBH_TX_COMMON_CONFIGURATIONS_SBPMCFG_RESERVED0_FIELD_WIDTH,
    BBH_TX_COMMON_CONFIGURATIONS_SBPMCFG_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_COMMON_CONFIGURATIONS_SBPMCFG_MAXGN
 ******************************************************************************/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_SBPMCFG_MAXGN_FIELD =
{
    "MAXGN",
#if RU_INCLUDE_DESC
    "max_get_next_on_the_fly",
    "maximum number of pending on the fly get next commands",
#endif
    BBH_TX_COMMON_CONFIGURATIONS_SBPMCFG_MAXGN_FIELD_MASK,
    0,
    BBH_TX_COMMON_CONFIGURATIONS_SBPMCFG_MAXGN_FIELD_WIDTH,
    BBH_TX_COMMON_CONFIGURATIONS_SBPMCFG_MAXGN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_COMMON_CONFIGURATIONS_SBPMCFG_RESERVED1
 ******************************************************************************/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_SBPMCFG_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    BBH_TX_COMMON_CONFIGURATIONS_SBPMCFG_RESERVED1_FIELD_MASK,
    0,
    BBH_TX_COMMON_CONFIGURATIONS_SBPMCFG_RESERVED1_FIELD_WIDTH,
    BBH_TX_COMMON_CONFIGURATIONS_SBPMCFG_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_COMMON_CONFIGURATIONS_DDRTMBASEL_DDRTMBASE
 ******************************************************************************/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_DDRTMBASEL_DDRTMBASE_FIELD =
{
    "DDRTMBASE",
#if RU_INCLUDE_DESC
    "DDR_TM_BASE",
    "DDR TM base."
    "The address is in bytes resolution."
    "The address should be aligned to 128 bytes.",
#endif
    BBH_TX_COMMON_CONFIGURATIONS_DDRTMBASEL_DDRTMBASE_FIELD_MASK,
    0,
    BBH_TX_COMMON_CONFIGURATIONS_DDRTMBASEL_DDRTMBASE_FIELD_WIDTH,
    BBH_TX_COMMON_CONFIGURATIONS_DDRTMBASEL_DDRTMBASE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_COMMON_CONFIGURATIONS_DDRTMBASEH_DDRTMBASE
 ******************************************************************************/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_DDRTMBASEH_DDRTMBASE_FIELD =
{
    "DDRTMBASE",
#if RU_INCLUDE_DESC
    "DDR_TM_BASE",
    "MSB of DDR TM base."
    "",
#endif
    BBH_TX_COMMON_CONFIGURATIONS_DDRTMBASEH_DDRTMBASE_FIELD_MASK,
    0,
    BBH_TX_COMMON_CONFIGURATIONS_DDRTMBASEH_DDRTMBASE_FIELD_WIDTH,
    BBH_TX_COMMON_CONFIGURATIONS_DDRTMBASEH_DDRTMBASE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_COMMON_CONFIGURATIONS_DDRTMBASEH_RESERVED0
 ******************************************************************************/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_DDRTMBASEH_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    BBH_TX_COMMON_CONFIGURATIONS_DDRTMBASEH_RESERVED0_FIELD_MASK,
    0,
    BBH_TX_COMMON_CONFIGURATIONS_DDRTMBASEH_RESERVED0_FIELD_WIDTH,
    BBH_TX_COMMON_CONFIGURATIONS_DDRTMBASEH_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_COMMON_CONFIGURATIONS_DFIFOCTRL_PSRAMSIZE
 ******************************************************************************/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_DFIFOCTRL_PSRAMSIZE_FIELD =
{
    "PSRAMSIZE",
#if RU_INCLUDE_DESC
    "PSRAM_FIFO_SIZE",
    "The size of the PSRAM data FIFO in 8 bytes resolution. The BBH uses this information for determining the amount of data that can be ordered from the PSRAM.",
#endif
    BBH_TX_COMMON_CONFIGURATIONS_DFIFOCTRL_PSRAMSIZE_FIELD_MASK,
    0,
    BBH_TX_COMMON_CONFIGURATIONS_DFIFOCTRL_PSRAMSIZE_FIELD_WIDTH,
    BBH_TX_COMMON_CONFIGURATIONS_DFIFOCTRL_PSRAMSIZE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_COMMON_CONFIGURATIONS_DFIFOCTRL_DDRSIZE
 ******************************************************************************/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_DFIFOCTRL_DDRSIZE_FIELD =
{
    "DDRSIZE",
#if RU_INCLUDE_DESC
    "DDR_FIFO_SIZE",
    "The size of the DDR data FIFO in 8 bytes resolution. The BBH uses this information for determining the amount of data that can be ordered from the DDR.",
#endif
    BBH_TX_COMMON_CONFIGURATIONS_DFIFOCTRL_DDRSIZE_FIELD_MASK,
    0,
    BBH_TX_COMMON_CONFIGURATIONS_DFIFOCTRL_DDRSIZE_FIELD_WIDTH,
    BBH_TX_COMMON_CONFIGURATIONS_DFIFOCTRL_DDRSIZE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_COMMON_CONFIGURATIONS_DFIFOCTRL_PSRAMBASE
 ******************************************************************************/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_DFIFOCTRL_PSRAMBASE_FIELD =
{
    "PSRAMBASE",
#if RU_INCLUDE_DESC
    "PSRAM_FIFO_BASE",
    "the base address of the PSRAM data FIFO in 8 bytes resolution. The DDR data FIFO base address is always 0."
    "In case the whole RAM is to be dedicated to PSRAM data, the base should be 0 as well, and the DDR FIFO size should be configured to 0.",
#endif
    BBH_TX_COMMON_CONFIGURATIONS_DFIFOCTRL_PSRAMBASE_FIELD_MASK,
    0,
    BBH_TX_COMMON_CONFIGURATIONS_DFIFOCTRL_PSRAMBASE_FIELD_WIDTH,
    BBH_TX_COMMON_CONFIGURATIONS_DFIFOCTRL_PSRAMBASE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_COMMON_CONFIGURATIONS_DFIFOCTRL_RESERVED0
 ******************************************************************************/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_DFIFOCTRL_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    BBH_TX_COMMON_CONFIGURATIONS_DFIFOCTRL_RESERVED0_FIELD_MASK,
    0,
    BBH_TX_COMMON_CONFIGURATIONS_DFIFOCTRL_RESERVED0_FIELD_WIDTH,
    BBH_TX_COMMON_CONFIGURATIONS_DFIFOCTRL_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_COMMON_CONFIGURATIONS_ARB_CFG_HIGHTRXQ
 ******************************************************************************/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_ARB_CFG_HIGHTRXQ_FIELD =
{
    "HIGHTRXQ",
#if RU_INCLUDE_DESC
    "consider_transmitting_q",
    "this configuration determines whether to give high priority to a current transmitting queue or not.",
#endif
    BBH_TX_COMMON_CONFIGURATIONS_ARB_CFG_HIGHTRXQ_FIELD_MASK,
    0,
    BBH_TX_COMMON_CONFIGURATIONS_ARB_CFG_HIGHTRXQ_FIELD_WIDTH,
    BBH_TX_COMMON_CONFIGURATIONS_ARB_CFG_HIGHTRXQ_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_COMMON_CONFIGURATIONS_ARB_CFG_RESERVED0
 ******************************************************************************/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_ARB_CFG_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    BBH_TX_COMMON_CONFIGURATIONS_ARB_CFG_RESERVED0_FIELD_MASK,
    0,
    BBH_TX_COMMON_CONFIGURATIONS_ARB_CFG_RESERVED0_FIELD_WIDTH,
    BBH_TX_COMMON_CONFIGURATIONS_ARB_CFG_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_COMMON_CONFIGURATIONS_BBROUTE_ROUTE
 ******************************************************************************/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_BBROUTE_ROUTE_FIELD =
{
    "ROUTE",
#if RU_INCLUDE_DESC
    "route",
    "route address",
#endif
    BBH_TX_COMMON_CONFIGURATIONS_BBROUTE_ROUTE_FIELD_MASK,
    0,
    BBH_TX_COMMON_CONFIGURATIONS_BBROUTE_ROUTE_FIELD_WIDTH,
    BBH_TX_COMMON_CONFIGURATIONS_BBROUTE_ROUTE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_COMMON_CONFIGURATIONS_BBROUTE_DEST
 ******************************************************************************/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_BBROUTE_DEST_FIELD =
{
    "DEST",
#if RU_INCLUDE_DESC
    "dest_id",
    "destination source id",
#endif
    BBH_TX_COMMON_CONFIGURATIONS_BBROUTE_DEST_FIELD_MASK,
    0,
    BBH_TX_COMMON_CONFIGURATIONS_BBROUTE_DEST_FIELD_WIDTH,
    BBH_TX_COMMON_CONFIGURATIONS_BBROUTE_DEST_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_COMMON_CONFIGURATIONS_BBROUTE_EN
 ******************************************************************************/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_BBROUTE_EN_FIELD =
{
    "EN",
#if RU_INCLUDE_DESC
    "enable",
    "enable",
#endif
    BBH_TX_COMMON_CONFIGURATIONS_BBROUTE_EN_FIELD_MASK,
    0,
    BBH_TX_COMMON_CONFIGURATIONS_BBROUTE_EN_FIELD_WIDTH,
    BBH_TX_COMMON_CONFIGURATIONS_BBROUTE_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_COMMON_CONFIGURATIONS_BBROUTE_RESERVED0
 ******************************************************************************/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_BBROUTE_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    BBH_TX_COMMON_CONFIGURATIONS_BBROUTE_RESERVED0_FIELD_MASK,
    0,
    BBH_TX_COMMON_CONFIGURATIONS_BBROUTE_RESERVED0_FIELD_WIDTH,
    BBH_TX_COMMON_CONFIGURATIONS_BBROUTE_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_COMMON_CONFIGURATIONS_Q2RNR_Q0
 ******************************************************************************/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_Q2RNR_Q0_FIELD =
{
    "Q0",
#if RU_INCLUDE_DESC
    "Q_0",
    "Q0 configuration",
#endif
    BBH_TX_COMMON_CONFIGURATIONS_Q2RNR_Q0_FIELD_MASK,
    0,
    BBH_TX_COMMON_CONFIGURATIONS_Q2RNR_Q0_FIELD_WIDTH,
    BBH_TX_COMMON_CONFIGURATIONS_Q2RNR_Q0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_COMMON_CONFIGURATIONS_Q2RNR_Q1
 ******************************************************************************/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_Q2RNR_Q1_FIELD =
{
    "Q1",
#if RU_INCLUDE_DESC
    "Q_1",
    "Q1 configuration",
#endif
    BBH_TX_COMMON_CONFIGURATIONS_Q2RNR_Q1_FIELD_MASK,
    0,
    BBH_TX_COMMON_CONFIGURATIONS_Q2RNR_Q1_FIELD_WIDTH,
    BBH_TX_COMMON_CONFIGURATIONS_Q2RNR_Q1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_COMMON_CONFIGURATIONS_Q2RNR_RESERVED0
 ******************************************************************************/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_Q2RNR_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    BBH_TX_COMMON_CONFIGURATIONS_Q2RNR_RESERVED0_FIELD_MASK,
    0,
    BBH_TX_COMMON_CONFIGURATIONS_Q2RNR_RESERVED0_FIELD_WIDTH,
    BBH_TX_COMMON_CONFIGURATIONS_Q2RNR_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_COMMON_CONFIGURATIONS_PERQTASK_TASK0
 ******************************************************************************/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_PERQTASK_TASK0_FIELD =
{
    "TASK0",
#if RU_INCLUDE_DESC
    "task_0",
    "task number for queue 0",
#endif
    BBH_TX_COMMON_CONFIGURATIONS_PERQTASK_TASK0_FIELD_MASK,
    0,
    BBH_TX_COMMON_CONFIGURATIONS_PERQTASK_TASK0_FIELD_WIDTH,
    BBH_TX_COMMON_CONFIGURATIONS_PERQTASK_TASK0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_COMMON_CONFIGURATIONS_PERQTASK_TASK1
 ******************************************************************************/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_PERQTASK_TASK1_FIELD =
{
    "TASK1",
#if RU_INCLUDE_DESC
    "task_1",
    "task number for queue 1",
#endif
    BBH_TX_COMMON_CONFIGURATIONS_PERQTASK_TASK1_FIELD_MASK,
    0,
    BBH_TX_COMMON_CONFIGURATIONS_PERQTASK_TASK1_FIELD_WIDTH,
    BBH_TX_COMMON_CONFIGURATIONS_PERQTASK_TASK1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_COMMON_CONFIGURATIONS_PERQTASK_TASK2
 ******************************************************************************/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_PERQTASK_TASK2_FIELD =
{
    "TASK2",
#if RU_INCLUDE_DESC
    "task_2",
    "task number for queue 2",
#endif
    BBH_TX_COMMON_CONFIGURATIONS_PERQTASK_TASK2_FIELD_MASK,
    0,
    BBH_TX_COMMON_CONFIGURATIONS_PERQTASK_TASK2_FIELD_WIDTH,
    BBH_TX_COMMON_CONFIGURATIONS_PERQTASK_TASK2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_COMMON_CONFIGURATIONS_PERQTASK_TASK3
 ******************************************************************************/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_PERQTASK_TASK3_FIELD =
{
    "TASK3",
#if RU_INCLUDE_DESC
    "task_3",
    "task number for queue 3",
#endif
    BBH_TX_COMMON_CONFIGURATIONS_PERQTASK_TASK3_FIELD_MASK,
    0,
    BBH_TX_COMMON_CONFIGURATIONS_PERQTASK_TASK3_FIELD_WIDTH,
    BBH_TX_COMMON_CONFIGURATIONS_PERQTASK_TASK3_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_COMMON_CONFIGURATIONS_PERQTASK_TASK4
 ******************************************************************************/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_PERQTASK_TASK4_FIELD =
{
    "TASK4",
#if RU_INCLUDE_DESC
    "task_4",
    "task number for queue 4",
#endif
    BBH_TX_COMMON_CONFIGURATIONS_PERQTASK_TASK4_FIELD_MASK,
    0,
    BBH_TX_COMMON_CONFIGURATIONS_PERQTASK_TASK4_FIELD_WIDTH,
    BBH_TX_COMMON_CONFIGURATIONS_PERQTASK_TASK4_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_COMMON_CONFIGURATIONS_PERQTASK_TASK5
 ******************************************************************************/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_PERQTASK_TASK5_FIELD =
{
    "TASK5",
#if RU_INCLUDE_DESC
    "task_5",
    "task number for queue 5",
#endif
    BBH_TX_COMMON_CONFIGURATIONS_PERQTASK_TASK5_FIELD_MASK,
    0,
    BBH_TX_COMMON_CONFIGURATIONS_PERQTASK_TASK5_FIELD_WIDTH,
    BBH_TX_COMMON_CONFIGURATIONS_PERQTASK_TASK5_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_COMMON_CONFIGURATIONS_PERQTASK_TASK6
 ******************************************************************************/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_PERQTASK_TASK6_FIELD =
{
    "TASK6",
#if RU_INCLUDE_DESC
    "task_6",
    "task number for queue 6",
#endif
    BBH_TX_COMMON_CONFIGURATIONS_PERQTASK_TASK6_FIELD_MASK,
    0,
    BBH_TX_COMMON_CONFIGURATIONS_PERQTASK_TASK6_FIELD_WIDTH,
    BBH_TX_COMMON_CONFIGURATIONS_PERQTASK_TASK6_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_COMMON_CONFIGURATIONS_PERQTASK_TASK7
 ******************************************************************************/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_PERQTASK_TASK7_FIELD =
{
    "TASK7",
#if RU_INCLUDE_DESC
    "task_7",
    "task number for queue 7",
#endif
    BBH_TX_COMMON_CONFIGURATIONS_PERQTASK_TASK7_FIELD_MASK,
    0,
    BBH_TX_COMMON_CONFIGURATIONS_PERQTASK_TASK7_FIELD_WIDTH,
    BBH_TX_COMMON_CONFIGURATIONS_PERQTASK_TASK7_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_CNTXTRST
 ******************************************************************************/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_CNTXTRST_FIELD =
{
    "CNTXTRST",
#if RU_INCLUDE_DESC
    "Context_reset",
    "Writing 1 to this register will reset the segmentation context table."
    "The reset is done immediately. Reading this register will always return 0.",
#endif
    BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_CNTXTRST_FIELD_MASK,
    0,
    BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_CNTXTRST_FIELD_WIDTH,
    BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_CNTXTRST_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_PDFIFORST
 ******************************************************************************/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_PDFIFORST_FIELD =
{
    "PDFIFORST",
#if RU_INCLUDE_DESC
    "PDs_FIFOs_reset",
    "Writing 1 to this register will reset the PDs FIFOs."
    "The reset is done immediately. Reading this register will always return 0.",
#endif
    BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_PDFIFORST_FIELD_MASK,
    0,
    BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_PDFIFORST_FIELD_WIDTH,
    BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_PDFIFORST_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_DMAPTRRST
 ******************************************************************************/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_DMAPTRRST_FIELD =
{
    "DMAPTRRST",
#if RU_INCLUDE_DESC
    "DMA_write_pointer_reset",
    "Writing 1 to this register will reset the DMA write pointer."
    "The reset is done immediately. Reading this register will always return 0.",
#endif
    BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_DMAPTRRST_FIELD_MASK,
    0,
    BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_DMAPTRRST_FIELD_WIDTH,
    BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_DMAPTRRST_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_SDMAPTRRST
 ******************************************************************************/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_SDMAPTRRST_FIELD =
{
    "SDMAPTRRST",
#if RU_INCLUDE_DESC
    "SDMA_write_pointer_reset",
    "Writing 1 to this register will reset the SDMA write pointer."
    "The reset is done immediately. Reading this register will always return 0."
    "This register is relevalt only for Ethernet.",
#endif
    BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_SDMAPTRRST_FIELD_MASK,
    0,
    BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_SDMAPTRRST_FIELD_WIDTH,
    BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_SDMAPTRRST_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_BPMFIFORST
 ******************************************************************************/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_BPMFIFORST_FIELD =
{
    "BPMFIFORST",
#if RU_INCLUDE_DESC
    "BPM_FIFO_reset",
    "Writing 1 to this register will reset the BPM FIFO."
    "The reset is done immediately. Reading this register will always return 0.",
#endif
    BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_BPMFIFORST_FIELD_MASK,
    0,
    BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_BPMFIFORST_FIELD_WIDTH,
    BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_BPMFIFORST_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_SBPMFIFORST
 ******************************************************************************/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_SBPMFIFORST_FIELD =
{
    "SBPMFIFORST",
#if RU_INCLUDE_DESC
    "SBPM_FIFO_reset",
    "Writing 1 to this register will reset the SBPM FIFO."
    "The reset is done immediately. Reading this register will always return 0."
    "This register is relevalt only for Ethernet.",
#endif
    BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_SBPMFIFORST_FIELD_MASK,
    0,
    BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_SBPMFIFORST_FIELD_WIDTH,
    BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_SBPMFIFORST_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_OKFIFORST
 ******************************************************************************/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_OKFIFORST_FIELD =
{
    "OKFIFORST",
#if RU_INCLUDE_DESC
    "Order_Keeper_FIFO_reset",
    "Writing 1 to this register will reset the order keeper FIFO."
    "The reset is done immediately. Reading this register will always return 0."
    "This register is relevalt only for Ethernet.",
#endif
    BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_OKFIFORST_FIELD_MASK,
    0,
    BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_OKFIFORST_FIELD_WIDTH,
    BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_OKFIFORST_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_DDRFIFORST
 ******************************************************************************/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_DDRFIFORST_FIELD =
{
    "DDRFIFORST",
#if RU_INCLUDE_DESC
    "DDR_FIFO_reset",
    "Writing 1 to this register will reset the DDR data FIFO."
    "The reset is done immediately. Reading this register will always return 0."
    "This register is relevalt only for Ethernet.",
#endif
    BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_DDRFIFORST_FIELD_MASK,
    0,
    BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_DDRFIFORST_FIELD_WIDTH,
    BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_DDRFIFORST_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_SRAMFIFORST
 ******************************************************************************/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_SRAMFIFORST_FIELD =
{
    "SRAMFIFORST",
#if RU_INCLUDE_DESC
    "SRAM_FIFO_reset",
    "Writing 1 to this register will reset the SRAM data FIFO."
    "The reset is done immediately. Reading this register will always return 0."
    "This register is relevalt only for Ethernet.",
#endif
    BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_SRAMFIFORST_FIELD_MASK,
    0,
    BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_SRAMFIFORST_FIELD_WIDTH,
    BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_SRAMFIFORST_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_SKBPTRRST
 ******************************************************************************/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_SKBPTRRST_FIELD =
{
    "SKBPTRRST",
#if RU_INCLUDE_DESC
    "SKB_PTR_reset",
    "Writing 1 to this register will reset the SKB pointers."
    "The reset is done immediately. Reading this register will always return 0."
    "",
#endif
    BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_SKBPTRRST_FIELD_MASK,
    0,
    BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_SKBPTRRST_FIELD_WIDTH,
    BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_SKBPTRRST_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_STSFIFORST
 ******************************************************************************/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_STSFIFORST_FIELD =
{
    "STSFIFORST",
#if RU_INCLUDE_DESC
    "STS_FIFOs_reset",
    "Writing 1 to this register will reset the EPON status FIFOs (per queue 32 fifos)."
    "The reset is done immediately. Reading this register will always return 0.",
#endif
    BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_STSFIFORST_FIELD_MASK,
    0,
    BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_STSFIFORST_FIELD_WIDTH,
    BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_STSFIFORST_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_REQFIFORST
 ******************************************************************************/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_REQFIFORST_FIELD =
{
    "REQFIFORST",
#if RU_INCLUDE_DESC
    "REQ_FIFO_reset",
    "Writing 1 to this register will reset the EPON request FIFO (8 entries FIFO that holds the packet requests from the EPON MAC)."
    "The reset is done immediately. Reading this register will always return 0.",
#endif
    BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_REQFIFORST_FIELD_MASK,
    0,
    BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_REQFIFORST_FIELD_WIDTH,
    BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_REQFIFORST_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_MSGFIFORST
 ******************************************************************************/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_MSGFIFORST_FIELD =
{
    "MSGFIFORST",
#if RU_INCLUDE_DESC
    "MSG_FIFO_reset",
    "Writing 1 to this register will reset the EPON/GPON MSG FIFO"
    "The reset is done immediately. Reading this register will always return 0.",
#endif
    BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_MSGFIFORST_FIELD_MASK,
    0,
    BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_MSGFIFORST_FIELD_WIDTH,
    BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_MSGFIFORST_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_GNXTFIFORST
 ******************************************************************************/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_GNXTFIFORST_FIELD =
{
    "GNXTFIFORST",
#if RU_INCLUDE_DESC
    "GET_NXT_FIFO_reset",
    "Writing 1 to this register will reset the GET NEXT FIFOs"
    "The reset is done immediately. Reading this register will always return 0.",
#endif
    BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_GNXTFIFORST_FIELD_MASK,
    0,
    BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_GNXTFIFORST_FIELD_WIDTH,
    BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_GNXTFIFORST_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_FBNFIFORST
 ******************************************************************************/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_FBNFIFORST_FIELD =
{
    "FBNFIFORST",
#if RU_INCLUDE_DESC
    "FIRST_BN_FIFO_reset",
    "Writing 1 to this register will reset the FIRST BN FIFOs"
    "The reset is done immediately. Reading this register will always return 0.",
#endif
    BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_FBNFIFORST_FIELD_MASK,
    0,
    BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_FBNFIFORST_FIELD_WIDTH,
    BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_FBNFIFORST_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_RESERVED0
 ******************************************************************************/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_RESERVED0_FIELD_MASK,
    0,
    BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_RESERVED0_FIELD_WIDTH,
    BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_COMMON_CONFIGURATIONS_DBGSEL_DBGSEL
 ******************************************************************************/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_DBGSEL_DBGSEL_FIELD =
{
    "DBGSEL",
#if RU_INCLUDE_DESC
    "debug_select",
    "This register selects 1 of 8 debug vectors."
    "The selected vector is reflected to DBGOUTREG.",
#endif
    BBH_TX_COMMON_CONFIGURATIONS_DBGSEL_DBGSEL_FIELD_MASK,
    0,
    BBH_TX_COMMON_CONFIGURATIONS_DBGSEL_DBGSEL_FIELD_WIDTH,
    BBH_TX_COMMON_CONFIGURATIONS_DBGSEL_DBGSEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_COMMON_CONFIGURATIONS_DBGSEL_RESERVED0
 ******************************************************************************/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_DBGSEL_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    BBH_TX_COMMON_CONFIGURATIONS_DBGSEL_RESERVED0_FIELD_MASK,
    0,
    BBH_TX_COMMON_CONFIGURATIONS_DBGSEL_RESERVED0_FIELD_WIDTH,
    BBH_TX_COMMON_CONFIGURATIONS_DBGSEL_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_BYPASS_CLK_GATE
 ******************************************************************************/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_BYPASS_CLK_GATE_FIELD =
{
    "BYPASS_CLK_GATE",
#if RU_INCLUDE_DESC
    "BYPASS_CLOCK_GATE",
    "If set to 1b1 will disable the clock gate logic such to always enable the clock",
#endif
    BBH_TX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_BYPASS_CLK_GATE_FIELD_MASK,
    0,
    BBH_TX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_BYPASS_CLK_GATE_FIELD_WIDTH,
    BBH_TX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_BYPASS_CLK_GATE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_RESERVED0
 ******************************************************************************/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    BBH_TX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_RESERVED0_FIELD_MASK,
    0,
    BBH_TX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_RESERVED0_FIELD_WIDTH,
    BBH_TX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_TIMER_VAL
 ******************************************************************************/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_TIMER_VAL_FIELD =
{
    "TIMER_VAL",
#if RU_INCLUDE_DESC
    "TIMER_VALUE",
    "For how long should the clock stay active once all conditions for clock disable are met."
    ""
    "",
#endif
    BBH_TX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_TIMER_VAL_FIELD_MASK,
    0,
    BBH_TX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_TIMER_VAL_FIELD_WIDTH,
    BBH_TX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_TIMER_VAL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_KEEP_ALIVE_EN
 ******************************************************************************/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_KEEP_ALIVE_EN_FIELD =
{
    "KEEP_ALIVE_EN",
#if RU_INCLUDE_DESC
    "KEEP_ALIVE_ENABLE",
    "Enables the keep alive logic which will periodically enable the clock to assure that no deadlock of clock being removed completely will occur",
#endif
    BBH_TX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_KEEP_ALIVE_EN_FIELD_MASK,
    0,
    BBH_TX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_KEEP_ALIVE_EN_FIELD_WIDTH,
    BBH_TX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_KEEP_ALIVE_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_RESERVED1
 ******************************************************************************/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    BBH_TX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_RESERVED1_FIELD_MASK,
    0,
    BBH_TX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_RESERVED1_FIELD_WIDTH,
    BBH_TX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_KEEP_ALIVE_INTRVL
 ******************************************************************************/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_KEEP_ALIVE_INTRVL_FIELD =
{
    "KEEP_ALIVE_INTRVL",
#if RU_INCLUDE_DESC
    "KEEP_ALIVE_INTERVAL",
    "If the KEEP alive option is enabled the field will determine for how many cycles should the clock be active",
#endif
    BBH_TX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_KEEP_ALIVE_INTRVL_FIELD_MASK,
    0,
    BBH_TX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_KEEP_ALIVE_INTRVL_FIELD_WIDTH,
    BBH_TX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_KEEP_ALIVE_INTRVL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_RESERVED2
 ******************************************************************************/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_RESERVED2_FIELD =
{
    "RESERVED2",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    BBH_TX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_RESERVED2_FIELD_MASK,
    0,
    BBH_TX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_RESERVED2_FIELD_WIDTH,
    BBH_TX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_RESERVED2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_KEEP_ALIVE_CYC
 ******************************************************************************/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_KEEP_ALIVE_CYC_FIELD =
{
    "KEEP_ALIVE_CYC",
#if RU_INCLUDE_DESC
    "KEEP_ALIVE_CYCLE",
    "If the KEEP alive option is enabled this field will determine for how many cycles should the clock be disabled (minus the KEEP_ALIVE_INTERVAL)"
    ""
    "So KEEP_ALIVE_CYCLE must be larger than KEEP_ALIVE_INTERVAL.",
#endif
    BBH_TX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_KEEP_ALIVE_CYC_FIELD_MASK,
    0,
    BBH_TX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_KEEP_ALIVE_CYC_FIELD_WIDTH,
    BBH_TX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_KEEP_ALIVE_CYC_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_WAN_CONFIGURATIONS_PDBASE_FIFOBASE0
 ******************************************************************************/
const ru_field_rec BBH_TX_WAN_CONFIGURATIONS_PDBASE_FIFOBASE0_FIELD =
{
    "FIFOBASE0",
#if RU_INCLUDE_DESC
    "FIFO_base_0",
    "The base of PD FIFO for queue 0.",
#endif
    BBH_TX_WAN_CONFIGURATIONS_PDBASE_FIFOBASE0_FIELD_MASK,
    0,
    BBH_TX_WAN_CONFIGURATIONS_PDBASE_FIFOBASE0_FIELD_WIDTH,
    BBH_TX_WAN_CONFIGURATIONS_PDBASE_FIFOBASE0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_WAN_CONFIGURATIONS_PDBASE_RESERVED0
 ******************************************************************************/
const ru_field_rec BBH_TX_WAN_CONFIGURATIONS_PDBASE_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    BBH_TX_WAN_CONFIGURATIONS_PDBASE_RESERVED0_FIELD_MASK,
    0,
    BBH_TX_WAN_CONFIGURATIONS_PDBASE_RESERVED0_FIELD_WIDTH,
    BBH_TX_WAN_CONFIGURATIONS_PDBASE_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_WAN_CONFIGURATIONS_PDBASE_FIFOBASE1
 ******************************************************************************/
const ru_field_rec BBH_TX_WAN_CONFIGURATIONS_PDBASE_FIFOBASE1_FIELD =
{
    "FIFOBASE1",
#if RU_INCLUDE_DESC
    "FIFO_base_1",
    "The base of PD FIFO for queue 1.",
#endif
    BBH_TX_WAN_CONFIGURATIONS_PDBASE_FIFOBASE1_FIELD_MASK,
    0,
    BBH_TX_WAN_CONFIGURATIONS_PDBASE_FIFOBASE1_FIELD_WIDTH,
    BBH_TX_WAN_CONFIGURATIONS_PDBASE_FIFOBASE1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_WAN_CONFIGURATIONS_PDBASE_RESERVED1
 ******************************************************************************/
const ru_field_rec BBH_TX_WAN_CONFIGURATIONS_PDBASE_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    BBH_TX_WAN_CONFIGURATIONS_PDBASE_RESERVED1_FIELD_MASK,
    0,
    BBH_TX_WAN_CONFIGURATIONS_PDBASE_RESERVED1_FIELD_WIDTH,
    BBH_TX_WAN_CONFIGURATIONS_PDBASE_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_WAN_CONFIGURATIONS_PDSIZE_FIFOSIZE0
 ******************************************************************************/
const ru_field_rec BBH_TX_WAN_CONFIGURATIONS_PDSIZE_FIFOSIZE0_FIELD =
{
    "FIFOSIZE0",
#if RU_INCLUDE_DESC
    "FIFO_size_0",
    "The size of PD FIFO for queue 0."
    "A value of n refers to n+1."
    "For GPON, the max value is 0x7"
    "For EPON, the max value is 0xf",
#endif
    BBH_TX_WAN_CONFIGURATIONS_PDSIZE_FIFOSIZE0_FIELD_MASK,
    0,
    BBH_TX_WAN_CONFIGURATIONS_PDSIZE_FIFOSIZE0_FIELD_WIDTH,
    BBH_TX_WAN_CONFIGURATIONS_PDSIZE_FIFOSIZE0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_WAN_CONFIGURATIONS_PDSIZE_RESERVED0
 ******************************************************************************/
const ru_field_rec BBH_TX_WAN_CONFIGURATIONS_PDSIZE_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    BBH_TX_WAN_CONFIGURATIONS_PDSIZE_RESERVED0_FIELD_MASK,
    0,
    BBH_TX_WAN_CONFIGURATIONS_PDSIZE_RESERVED0_FIELD_WIDTH,
    BBH_TX_WAN_CONFIGURATIONS_PDSIZE_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_WAN_CONFIGURATIONS_PDSIZE_FIFOSIZE1
 ******************************************************************************/
const ru_field_rec BBH_TX_WAN_CONFIGURATIONS_PDSIZE_FIFOSIZE1_FIELD =
{
    "FIFOSIZE1",
#if RU_INCLUDE_DESC
    "FIFO_size_1",
    "The size of PD FIFO for queue 1."
    "A value of n refers to n+1."
    "",
#endif
    BBH_TX_WAN_CONFIGURATIONS_PDSIZE_FIFOSIZE1_FIELD_MASK,
    0,
    BBH_TX_WAN_CONFIGURATIONS_PDSIZE_FIFOSIZE1_FIELD_WIDTH,
    BBH_TX_WAN_CONFIGURATIONS_PDSIZE_FIFOSIZE1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_WAN_CONFIGURATIONS_PDSIZE_RESERVED1
 ******************************************************************************/
const ru_field_rec BBH_TX_WAN_CONFIGURATIONS_PDSIZE_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    BBH_TX_WAN_CONFIGURATIONS_PDSIZE_RESERVED1_FIELD_MASK,
    0,
    BBH_TX_WAN_CONFIGURATIONS_PDSIZE_RESERVED1_FIELD_WIDTH,
    BBH_TX_WAN_CONFIGURATIONS_PDSIZE_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_WAN_CONFIGURATIONS_PDWKUPH_WKUPTHRESH0
 ******************************************************************************/
const ru_field_rec BBH_TX_WAN_CONFIGURATIONS_PDWKUPH_WKUPTHRESH0_FIELD =
{
    "WKUPTHRESH0",
#if RU_INCLUDE_DESC
    "FIFO_wake_up_threshold_0",
    "The wakeup threshold of the PD FIFO for queue 0."
    "A value of n refers to n+1."
    "Relevant only for EPON BBH.",
#endif
    BBH_TX_WAN_CONFIGURATIONS_PDWKUPH_WKUPTHRESH0_FIELD_MASK,
    0,
    BBH_TX_WAN_CONFIGURATIONS_PDWKUPH_WKUPTHRESH0_FIELD_WIDTH,
    BBH_TX_WAN_CONFIGURATIONS_PDWKUPH_WKUPTHRESH0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_WAN_CONFIGURATIONS_PDWKUPH_RESERVED0
 ******************************************************************************/
const ru_field_rec BBH_TX_WAN_CONFIGURATIONS_PDWKUPH_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    BBH_TX_WAN_CONFIGURATIONS_PDWKUPH_RESERVED0_FIELD_MASK,
    0,
    BBH_TX_WAN_CONFIGURATIONS_PDWKUPH_RESERVED0_FIELD_WIDTH,
    BBH_TX_WAN_CONFIGURATIONS_PDWKUPH_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_WAN_CONFIGURATIONS_PDWKUPH_WKUPTHRESH1
 ******************************************************************************/
const ru_field_rec BBH_TX_WAN_CONFIGURATIONS_PDWKUPH_WKUPTHRESH1_FIELD =
{
    "WKUPTHRESH1",
#if RU_INCLUDE_DESC
    "FIFO_wake_up_threshold_1",
    "The wakeup threshold of the PD FIFO for queue 1."
    "A value of n refers to n+1."
    "Relevant only for EPON BBH.",
#endif
    BBH_TX_WAN_CONFIGURATIONS_PDWKUPH_WKUPTHRESH1_FIELD_MASK,
    0,
    BBH_TX_WAN_CONFIGURATIONS_PDWKUPH_WKUPTHRESH1_FIELD_WIDTH,
    BBH_TX_WAN_CONFIGURATIONS_PDWKUPH_WKUPTHRESH1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_WAN_CONFIGURATIONS_PDWKUPH_RESERVED1
 ******************************************************************************/
const ru_field_rec BBH_TX_WAN_CONFIGURATIONS_PDWKUPH_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    BBH_TX_WAN_CONFIGURATIONS_PDWKUPH_RESERVED1_FIELD_MASK,
    0,
    BBH_TX_WAN_CONFIGURATIONS_PDWKUPH_RESERVED1_FIELD_WIDTH,
    BBH_TX_WAN_CONFIGURATIONS_PDWKUPH_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_WAN_CONFIGURATIONS_PD_BYTE_TH_PDLIMIT0
 ******************************************************************************/
const ru_field_rec BBH_TX_WAN_CONFIGURATIONS_PD_BYTE_TH_PDLIMIT0_FIELD =
{
    "PDLIMIT0",
#if RU_INCLUDE_DESC
    "PD_limit_0",
    "Defines the number of bytes for PDs pre fetch limited according to the total number of bytes."
    "The value is in 8-bytes resolution."
    "",
#endif
    BBH_TX_WAN_CONFIGURATIONS_PD_BYTE_TH_PDLIMIT0_FIELD_MASK,
    0,
    BBH_TX_WAN_CONFIGURATIONS_PD_BYTE_TH_PDLIMIT0_FIELD_WIDTH,
    BBH_TX_WAN_CONFIGURATIONS_PD_BYTE_TH_PDLIMIT0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_WAN_CONFIGURATIONS_PD_BYTE_TH_PDLIMIT1
 ******************************************************************************/
const ru_field_rec BBH_TX_WAN_CONFIGURATIONS_PD_BYTE_TH_PDLIMIT1_FIELD =
{
    "PDLIMIT1",
#if RU_INCLUDE_DESC
    "PD_limit_1",
    "Defines the number of bytes for PDs pre fetch limited according to the total number of bytes."
    "The value is in 8-bytes resolution."
    "",
#endif
    BBH_TX_WAN_CONFIGURATIONS_PD_BYTE_TH_PDLIMIT1_FIELD_MASK,
    0,
    BBH_TX_WAN_CONFIGURATIONS_PD_BYTE_TH_PDLIMIT1_FIELD_WIDTH,
    BBH_TX_WAN_CONFIGURATIONS_PD_BYTE_TH_PDLIMIT1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_WAN_CONFIGURATIONS_PD_BYTE_TH_EN_PDLIMITEN
 ******************************************************************************/
const ru_field_rec BBH_TX_WAN_CONFIGURATIONS_PD_BYTE_TH_EN_PDLIMITEN_FIELD =
{
    "PDLIMITEN",
#if RU_INCLUDE_DESC
    "PD_limit_enable",
    "This bit enables the above feature (PDs pre fetch limited according to the total number of bytes).",
#endif
    BBH_TX_WAN_CONFIGURATIONS_PD_BYTE_TH_EN_PDLIMITEN_FIELD_MASK,
    0,
    BBH_TX_WAN_CONFIGURATIONS_PD_BYTE_TH_EN_PDLIMITEN_FIELD_WIDTH,
    BBH_TX_WAN_CONFIGURATIONS_PD_BYTE_TH_EN_PDLIMITEN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_WAN_CONFIGURATIONS_PD_BYTE_TH_EN_RESERVED0
 ******************************************************************************/
const ru_field_rec BBH_TX_WAN_CONFIGURATIONS_PD_BYTE_TH_EN_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    BBH_TX_WAN_CONFIGURATIONS_PD_BYTE_TH_EN_RESERVED0_FIELD_MASK,
    0,
    BBH_TX_WAN_CONFIGURATIONS_PD_BYTE_TH_EN_RESERVED0_FIELD_WIDTH,
    BBH_TX_WAN_CONFIGURATIONS_PD_BYTE_TH_EN_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_WAN_CONFIGURATIONS_PDEMPTY_EMPTY
 ******************************************************************************/
const ru_field_rec BBH_TX_WAN_CONFIGURATIONS_PDEMPTY_EMPTY_FIELD =
{
    "EMPTY",
#if RU_INCLUDE_DESC
    "Empty_thershold",
    "EPON PD FIFO empty threshold."
    "A queue which its PD FIFO occupancy is below this threshold will have high priority in PD ordering arbitration.",
#endif
    BBH_TX_WAN_CONFIGURATIONS_PDEMPTY_EMPTY_FIELD_MASK,
    0,
    BBH_TX_WAN_CONFIGURATIONS_PDEMPTY_EMPTY_FIELD_WIDTH,
    BBH_TX_WAN_CONFIGURATIONS_PDEMPTY_EMPTY_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_WAN_CONFIGURATIONS_PDEMPTY_RESERVED0
 ******************************************************************************/
const ru_field_rec BBH_TX_WAN_CONFIGURATIONS_PDEMPTY_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    BBH_TX_WAN_CONFIGURATIONS_PDEMPTY_RESERVED0_FIELD_MASK,
    0,
    BBH_TX_WAN_CONFIGURATIONS_PDEMPTY_RESERVED0_FIELD_WIDTH,
    BBH_TX_WAN_CONFIGURATIONS_PDEMPTY_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_WAN_CONFIGURATIONS_STSRNRCFG_1_TCONTADDR
 ******************************************************************************/
const ru_field_rec BBH_TX_WAN_CONFIGURATIONS_STSRNRCFG_1_TCONTADDR_FIELD =
{
    "TCONTADDR",
#if RU_INCLUDE_DESC
    "TCONT_address",
    "Defines the TCONT address within the Runner address space."
    "The address is in 8 bytes resolution."
    "",
#endif
    BBH_TX_WAN_CONFIGURATIONS_STSRNRCFG_1_TCONTADDR_FIELD_MASK,
    0,
    BBH_TX_WAN_CONFIGURATIONS_STSRNRCFG_1_TCONTADDR_FIELD_WIDTH,
    BBH_TX_WAN_CONFIGURATIONS_STSRNRCFG_1_TCONTADDR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_WAN_CONFIGURATIONS_STSRNRCFG_1_RESERVED0
 ******************************************************************************/
const ru_field_rec BBH_TX_WAN_CONFIGURATIONS_STSRNRCFG_1_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    BBH_TX_WAN_CONFIGURATIONS_STSRNRCFG_1_RESERVED0_FIELD_MASK,
    0,
    BBH_TX_WAN_CONFIGURATIONS_STSRNRCFG_1_RESERVED0_FIELD_WIDTH,
    BBH_TX_WAN_CONFIGURATIONS_STSRNRCFG_1_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_WAN_CONFIGURATIONS_STSRNRCFG_2_PTRADDR
 ******************************************************************************/
const ru_field_rec BBH_TX_WAN_CONFIGURATIONS_STSRNRCFG_2_PTRADDR_FIELD =
{
    "PTRADDR",
#if RU_INCLUDE_DESC
    "PTRADDR",
    "This field defins the address in the Runner memory space to which the read pointer is written."
    "The address is in 8-bytes resolution.",
#endif
    BBH_TX_WAN_CONFIGURATIONS_STSRNRCFG_2_PTRADDR_FIELD_MASK,
    0,
    BBH_TX_WAN_CONFIGURATIONS_STSRNRCFG_2_PTRADDR_FIELD_WIDTH,
    BBH_TX_WAN_CONFIGURATIONS_STSRNRCFG_2_PTRADDR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_WAN_CONFIGURATIONS_STSRNRCFG_2_TASK
 ******************************************************************************/
const ru_field_rec BBH_TX_WAN_CONFIGURATIONS_STSRNRCFG_2_TASK_FIELD =
{
    "TASK",
#if RU_INCLUDE_DESC
    "Task_number",
    "The number of the task that is responsible for sending PDs to the BBH",
#endif
    BBH_TX_WAN_CONFIGURATIONS_STSRNRCFG_2_TASK_FIELD_MASK,
    0,
    BBH_TX_WAN_CONFIGURATIONS_STSRNRCFG_2_TASK_FIELD_WIDTH,
    BBH_TX_WAN_CONFIGURATIONS_STSRNRCFG_2_TASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_WAN_CONFIGURATIONS_STSRNRCFG_2_RESERVED0
 ******************************************************************************/
const ru_field_rec BBH_TX_WAN_CONFIGURATIONS_STSRNRCFG_2_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    BBH_TX_WAN_CONFIGURATIONS_STSRNRCFG_2_RESERVED0_FIELD_MASK,
    0,
    BBH_TX_WAN_CONFIGURATIONS_STSRNRCFG_2_RESERVED0_FIELD_WIDTH,
    BBH_TX_WAN_CONFIGURATIONS_STSRNRCFG_2_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_WAN_CONFIGURATIONS_MSGRNRCFG_1_TCONTADDR
 ******************************************************************************/
const ru_field_rec BBH_TX_WAN_CONFIGURATIONS_MSGRNRCFG_1_TCONTADDR_FIELD =
{
    "TCONTADDR",
#if RU_INCLUDE_DESC
    "TCONT_address",
    "Defines the TCONT address within the Runner address space."
    "The address is in 8 bytes resolution."
    "",
#endif
    BBH_TX_WAN_CONFIGURATIONS_MSGRNRCFG_1_TCONTADDR_FIELD_MASK,
    0,
    BBH_TX_WAN_CONFIGURATIONS_MSGRNRCFG_1_TCONTADDR_FIELD_WIDTH,
    BBH_TX_WAN_CONFIGURATIONS_MSGRNRCFG_1_TCONTADDR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_WAN_CONFIGURATIONS_MSGRNRCFG_1_RESERVED0
 ******************************************************************************/
const ru_field_rec BBH_TX_WAN_CONFIGURATIONS_MSGRNRCFG_1_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    BBH_TX_WAN_CONFIGURATIONS_MSGRNRCFG_1_RESERVED0_FIELD_MASK,
    0,
    BBH_TX_WAN_CONFIGURATIONS_MSGRNRCFG_1_RESERVED0_FIELD_WIDTH,
    BBH_TX_WAN_CONFIGURATIONS_MSGRNRCFG_1_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_WAN_CONFIGURATIONS_MSGRNRCFG_2_PTRADDR
 ******************************************************************************/
const ru_field_rec BBH_TX_WAN_CONFIGURATIONS_MSGRNRCFG_2_PTRADDR_FIELD =
{
    "PTRADDR",
#if RU_INCLUDE_DESC
    "PTRADDR",
    "This field defins the address in the Runner memory space to which the read pointer is written."
    "The address is in 8-bytes resolution.",
#endif
    BBH_TX_WAN_CONFIGURATIONS_MSGRNRCFG_2_PTRADDR_FIELD_MASK,
    0,
    BBH_TX_WAN_CONFIGURATIONS_MSGRNRCFG_2_PTRADDR_FIELD_WIDTH,
    BBH_TX_WAN_CONFIGURATIONS_MSGRNRCFG_2_PTRADDR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_WAN_CONFIGURATIONS_MSGRNRCFG_2_TASK
 ******************************************************************************/
const ru_field_rec BBH_TX_WAN_CONFIGURATIONS_MSGRNRCFG_2_TASK_FIELD =
{
    "TASK",
#if RU_INCLUDE_DESC
    "Task_number",
    "The number of the task that is responsible for sending PDs to the BBH",
#endif
    BBH_TX_WAN_CONFIGURATIONS_MSGRNRCFG_2_TASK_FIELD_MASK,
    0,
    BBH_TX_WAN_CONFIGURATIONS_MSGRNRCFG_2_TASK_FIELD_WIDTH,
    BBH_TX_WAN_CONFIGURATIONS_MSGRNRCFG_2_TASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_WAN_CONFIGURATIONS_MSGRNRCFG_2_RESERVED0
 ******************************************************************************/
const ru_field_rec BBH_TX_WAN_CONFIGURATIONS_MSGRNRCFG_2_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    BBH_TX_WAN_CONFIGURATIONS_MSGRNRCFG_2_RESERVED0_FIELD_MASK,
    0,
    BBH_TX_WAN_CONFIGURATIONS_MSGRNRCFG_2_RESERVED0_FIELD_WIDTH,
    BBH_TX_WAN_CONFIGURATIONS_MSGRNRCFG_2_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_WAN_CONFIGURATIONS_EPNCFG_STPLENERR
 ******************************************************************************/
const ru_field_rec BBH_TX_WAN_CONFIGURATIONS_EPNCFG_STPLENERR_FIELD =
{
    "STPLENERR",
#if RU_INCLUDE_DESC
    "Stop_on_len_error",
    "In case of fatal length error - a mismatch between the request message from MAC and its relevant PD from Runner - the BBH can stop performing or continue regardless of the error."
    "The error is also reflected to the SW in a counter.",
#endif
    BBH_TX_WAN_CONFIGURATIONS_EPNCFG_STPLENERR_FIELD_MASK,
    0,
    BBH_TX_WAN_CONFIGURATIONS_EPNCFG_STPLENERR_FIELD_WIDTH,
    BBH_TX_WAN_CONFIGURATIONS_EPNCFG_STPLENERR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_WAN_CONFIGURATIONS_EPNCFG_CMP_WIDTH
 ******************************************************************************/
const ru_field_rec BBH_TX_WAN_CONFIGURATIONS_EPNCFG_CMP_WIDTH_FIELD =
{
    "CMP_WIDTH",
#if RU_INCLUDE_DESC
    "comp_width",
    "configures the width of the comparison of the packet ength."
    "The length field in the EPON request interface is 11 bit, while it is 14 bit in the pd."
    "If this bit is 0, then the comparison of the length will be between the 11 bit of the interface and the 11 lsb bits of the pd."
    "If this ibt is 1, the comparison will be done between the 11 bits of the interface, concatenated with 3 zeros and the 14 bits of the pd",
#endif
    BBH_TX_WAN_CONFIGURATIONS_EPNCFG_CMP_WIDTH_FIELD_MASK,
    0,
    BBH_TX_WAN_CONFIGURATIONS_EPNCFG_CMP_WIDTH_FIELD_WIDTH,
    BBH_TX_WAN_CONFIGURATIONS_EPNCFG_CMP_WIDTH_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_WAN_CONFIGURATIONS_EPNCFG_CONSIDERFULL
 ******************************************************************************/
const ru_field_rec BBH_TX_WAN_CONFIGURATIONS_EPNCFG_CONSIDERFULL_FIELD =
{
    "CONSIDERFULL",
#if RU_INCLUDE_DESC
    "xepon_consider_sts_full",
    "determines whether the BBH will consider the sts_full vector state when pushing STS messages to the MAC or not."
    ""
    "The status fifos inside the MAC should never go full as they are mirror of the BBH PD FIFOs, but in cases where the MAC design behaves different than expected, we want the BBH to be able to operate as in 1G EPON mode",
#endif
    BBH_TX_WAN_CONFIGURATIONS_EPNCFG_CONSIDERFULL_FIELD_MASK,
    0,
    BBH_TX_WAN_CONFIGURATIONS_EPNCFG_CONSIDERFULL_FIELD_WIDTH,
    BBH_TX_WAN_CONFIGURATIONS_EPNCFG_CONSIDERFULL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_WAN_CONFIGURATIONS_EPNCFG_ADDCRC
 ******************************************************************************/
const ru_field_rec BBH_TX_WAN_CONFIGURATIONS_EPNCFG_ADDCRC_FIELD =
{
    "ADDCRC",
#if RU_INCLUDE_DESC
    "add_crc_bytes_to_len",
    "configuration whether to add 4 bytes per packet to the length received in the status message from the Runner so the MAC would know the actual length to be transmitted.",
#endif
    BBH_TX_WAN_CONFIGURATIONS_EPNCFG_ADDCRC_FIELD_MASK,
    0,
    BBH_TX_WAN_CONFIGURATIONS_EPNCFG_ADDCRC_FIELD_WIDTH,
    BBH_TX_WAN_CONFIGURATIONS_EPNCFG_ADDCRC_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_WAN_CONFIGURATIONS_EPNCFG_RESERVED0
 ******************************************************************************/
const ru_field_rec BBH_TX_WAN_CONFIGURATIONS_EPNCFG_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    BBH_TX_WAN_CONFIGURATIONS_EPNCFG_RESERVED0_FIELD_MASK,
    0,
    BBH_TX_WAN_CONFIGURATIONS_EPNCFG_RESERVED0_FIELD_WIDTH,
    BBH_TX_WAN_CONFIGURATIONS_EPNCFG_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_WAN_CONFIGURATIONS_FLOW2PORT_WDATA
 ******************************************************************************/
const ru_field_rec BBH_TX_WAN_CONFIGURATIONS_FLOW2PORT_WDATA_FIELD =
{
    "WDATA",
#if RU_INCLUDE_DESC
    "wr_data",
    "write data."
    ""
    "15:0 - port-id - default is 0x0000"
    "16 - regenerate CRC - enabled by default"
    "17 - enc enable - disabled by default"
    ""
    "",
#endif
    BBH_TX_WAN_CONFIGURATIONS_FLOW2PORT_WDATA_FIELD_MASK,
    0,
    BBH_TX_WAN_CONFIGURATIONS_FLOW2PORT_WDATA_FIELD_WIDTH,
    BBH_TX_WAN_CONFIGURATIONS_FLOW2PORT_WDATA_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_WAN_CONFIGURATIONS_FLOW2PORT_A
 ******************************************************************************/
const ru_field_rec BBH_TX_WAN_CONFIGURATIONS_FLOW2PORT_A_FIELD =
{
    "A",
#if RU_INCLUDE_DESC
    "address",
    "address",
#endif
    BBH_TX_WAN_CONFIGURATIONS_FLOW2PORT_A_FIELD_MASK,
    0,
    BBH_TX_WAN_CONFIGURATIONS_FLOW2PORT_A_FIELD_WIDTH,
    BBH_TX_WAN_CONFIGURATIONS_FLOW2PORT_A_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_WAN_CONFIGURATIONS_FLOW2PORT_CMD
 ******************************************************************************/
const ru_field_rec BBH_TX_WAN_CONFIGURATIONS_FLOW2PORT_CMD_FIELD =
{
    "CMD",
#if RU_INCLUDE_DESC
    "cmd",
    "rd/wr cmd",
#endif
    BBH_TX_WAN_CONFIGURATIONS_FLOW2PORT_CMD_FIELD_MASK,
    0,
    BBH_TX_WAN_CONFIGURATIONS_FLOW2PORT_CMD_FIELD_WIDTH,
    BBH_TX_WAN_CONFIGURATIONS_FLOW2PORT_CMD_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_WAN_CONFIGURATIONS_FLOW2PORT_RESERVED0
 ******************************************************************************/
const ru_field_rec BBH_TX_WAN_CONFIGURATIONS_FLOW2PORT_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    BBH_TX_WAN_CONFIGURATIONS_FLOW2PORT_RESERVED0_FIELD_MASK,
    0,
    BBH_TX_WAN_CONFIGURATIONS_FLOW2PORT_RESERVED0_FIELD_WIDTH,
    BBH_TX_WAN_CONFIGURATIONS_FLOW2PORT_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_WAN_CONFIGURATIONS_TS_EN
 ******************************************************************************/
const ru_field_rec BBH_TX_WAN_CONFIGURATIONS_TS_EN_FIELD =
{
    "EN",
#if RU_INCLUDE_DESC
    "enable",
    "1588 enable",
#endif
    BBH_TX_WAN_CONFIGURATIONS_TS_EN_FIELD_MASK,
    0,
    BBH_TX_WAN_CONFIGURATIONS_TS_EN_FIELD_WIDTH,
    BBH_TX_WAN_CONFIGURATIONS_TS_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_WAN_CONFIGURATIONS_TS_RESERVED0
 ******************************************************************************/
const ru_field_rec BBH_TX_WAN_CONFIGURATIONS_TS_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    BBH_TX_WAN_CONFIGURATIONS_TS_RESERVED0_FIELD_MASK,
    0,
    BBH_TX_WAN_CONFIGURATIONS_TS_RESERVED0_FIELD_WIDTH,
    BBH_TX_WAN_CONFIGURATIONS_TS_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_WAN_CONFIGURATIONS_MAXWLEN_MAXWLEN
 ******************************************************************************/
const ru_field_rec BBH_TX_WAN_CONFIGURATIONS_MAXWLEN_MAXWLEN_FIELD =
{
    "MAXWLEN",
#if RU_INCLUDE_DESC
    "max_word_len",
    "VDSL max word len",
#endif
    BBH_TX_WAN_CONFIGURATIONS_MAXWLEN_MAXWLEN_FIELD_MASK,
    0,
    BBH_TX_WAN_CONFIGURATIONS_MAXWLEN_MAXWLEN_FIELD_WIDTH,
    BBH_TX_WAN_CONFIGURATIONS_MAXWLEN_MAXWLEN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_WAN_CONFIGURATIONS_MAXWLEN_RESERVED0
 ******************************************************************************/
const ru_field_rec BBH_TX_WAN_CONFIGURATIONS_MAXWLEN_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    BBH_TX_WAN_CONFIGURATIONS_MAXWLEN_RESERVED0_FIELD_MASK,
    0,
    BBH_TX_WAN_CONFIGURATIONS_MAXWLEN_RESERVED0_FIELD_WIDTH,
    BBH_TX_WAN_CONFIGURATIONS_MAXWLEN_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_WAN_CONFIGURATIONS_FLUSH_FLUSH
 ******************************************************************************/
const ru_field_rec BBH_TX_WAN_CONFIGURATIONS_FLUSH_FLUSH_FIELD =
{
    "FLUSH",
#if RU_INCLUDE_DESC
    "flush",
    "VDSL flush",
#endif
    BBH_TX_WAN_CONFIGURATIONS_FLUSH_FLUSH_FIELD_MASK,
    0,
    BBH_TX_WAN_CONFIGURATIONS_FLUSH_FLUSH_FIELD_WIDTH,
    BBH_TX_WAN_CONFIGURATIONS_FLUSH_FLUSH_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_WAN_CONFIGURATIONS_FLUSH_RESERVED0
 ******************************************************************************/
const ru_field_rec BBH_TX_WAN_CONFIGURATIONS_FLUSH_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    BBH_TX_WAN_CONFIGURATIONS_FLUSH_RESERVED0_FIELD_MASK,
    0,
    BBH_TX_WAN_CONFIGURATIONS_FLUSH_RESERVED0_FIELD_WIDTH,
    BBH_TX_WAN_CONFIGURATIONS_FLUSH_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_WAN_CONFIGURATIONS_FLUSH_SRST_N
 ******************************************************************************/
const ru_field_rec BBH_TX_WAN_CONFIGURATIONS_FLUSH_SRST_N_FIELD =
{
    "SRST_N",
#if RU_INCLUDE_DESC
    "soft_rst_n",
    "soft reset",
#endif
    BBH_TX_WAN_CONFIGURATIONS_FLUSH_SRST_N_FIELD_MASK,
    0,
    BBH_TX_WAN_CONFIGURATIONS_FLUSH_SRST_N_FIELD_WIDTH,
    BBH_TX_WAN_CONFIGURATIONS_FLUSH_SRST_N_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_LAN_CONFIGURATIONS_PDBASE_FIFOBASE0
 ******************************************************************************/
const ru_field_rec BBH_TX_LAN_CONFIGURATIONS_PDBASE_FIFOBASE0_FIELD =
{
    "FIFOBASE0",
#if RU_INCLUDE_DESC
    "FIFO_base_0",
    "The base of PD FIFO for queue 0.",
#endif
    BBH_TX_LAN_CONFIGURATIONS_PDBASE_FIFOBASE0_FIELD_MASK,
    0,
    BBH_TX_LAN_CONFIGURATIONS_PDBASE_FIFOBASE0_FIELD_WIDTH,
    BBH_TX_LAN_CONFIGURATIONS_PDBASE_FIFOBASE0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_LAN_CONFIGURATIONS_PDBASE_RESERVED0
 ******************************************************************************/
const ru_field_rec BBH_TX_LAN_CONFIGURATIONS_PDBASE_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    BBH_TX_LAN_CONFIGURATIONS_PDBASE_RESERVED0_FIELD_MASK,
    0,
    BBH_TX_LAN_CONFIGURATIONS_PDBASE_RESERVED0_FIELD_WIDTH,
    BBH_TX_LAN_CONFIGURATIONS_PDBASE_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_LAN_CONFIGURATIONS_PDBASE_FIFOBASE1
 ******************************************************************************/
const ru_field_rec BBH_TX_LAN_CONFIGURATIONS_PDBASE_FIFOBASE1_FIELD =
{
    "FIFOBASE1",
#if RU_INCLUDE_DESC
    "FIFO_base_1",
    "The base of PD FIFO for queue 1.",
#endif
    BBH_TX_LAN_CONFIGURATIONS_PDBASE_FIFOBASE1_FIELD_MASK,
    0,
    BBH_TX_LAN_CONFIGURATIONS_PDBASE_FIFOBASE1_FIELD_WIDTH,
    BBH_TX_LAN_CONFIGURATIONS_PDBASE_FIFOBASE1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_LAN_CONFIGURATIONS_PDBASE_RESERVED1
 ******************************************************************************/
const ru_field_rec BBH_TX_LAN_CONFIGURATIONS_PDBASE_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    BBH_TX_LAN_CONFIGURATIONS_PDBASE_RESERVED1_FIELD_MASK,
    0,
    BBH_TX_LAN_CONFIGURATIONS_PDBASE_RESERVED1_FIELD_WIDTH,
    BBH_TX_LAN_CONFIGURATIONS_PDBASE_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_LAN_CONFIGURATIONS_PDSIZE_FIFOSIZE0
 ******************************************************************************/
const ru_field_rec BBH_TX_LAN_CONFIGURATIONS_PDSIZE_FIFOSIZE0_FIELD =
{
    "FIFOSIZE0",
#if RU_INCLUDE_DESC
    "FIFO_size_0",
    "The size of PD FIFO for queue 0."
    "A value of n refers to n+1."
    "For GPON, the max value is 0x7"
    "For EPON, the max value is 0xf",
#endif
    BBH_TX_LAN_CONFIGURATIONS_PDSIZE_FIFOSIZE0_FIELD_MASK,
    0,
    BBH_TX_LAN_CONFIGURATIONS_PDSIZE_FIFOSIZE0_FIELD_WIDTH,
    BBH_TX_LAN_CONFIGURATIONS_PDSIZE_FIFOSIZE0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_LAN_CONFIGURATIONS_PDSIZE_RESERVED0
 ******************************************************************************/
const ru_field_rec BBH_TX_LAN_CONFIGURATIONS_PDSIZE_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    BBH_TX_LAN_CONFIGURATIONS_PDSIZE_RESERVED0_FIELD_MASK,
    0,
    BBH_TX_LAN_CONFIGURATIONS_PDSIZE_RESERVED0_FIELD_WIDTH,
    BBH_TX_LAN_CONFIGURATIONS_PDSIZE_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_LAN_CONFIGURATIONS_PDSIZE_FIFOSIZE1
 ******************************************************************************/
const ru_field_rec BBH_TX_LAN_CONFIGURATIONS_PDSIZE_FIFOSIZE1_FIELD =
{
    "FIFOSIZE1",
#if RU_INCLUDE_DESC
    "FIFO_size_1",
    "The size of PD FIFO for queue 1."
    "A value of n refers to n+1."
    "",
#endif
    BBH_TX_LAN_CONFIGURATIONS_PDSIZE_FIFOSIZE1_FIELD_MASK,
    0,
    BBH_TX_LAN_CONFIGURATIONS_PDSIZE_FIFOSIZE1_FIELD_WIDTH,
    BBH_TX_LAN_CONFIGURATIONS_PDSIZE_FIFOSIZE1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_LAN_CONFIGURATIONS_PDSIZE_RESERVED1
 ******************************************************************************/
const ru_field_rec BBH_TX_LAN_CONFIGURATIONS_PDSIZE_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    BBH_TX_LAN_CONFIGURATIONS_PDSIZE_RESERVED1_FIELD_MASK,
    0,
    BBH_TX_LAN_CONFIGURATIONS_PDSIZE_RESERVED1_FIELD_WIDTH,
    BBH_TX_LAN_CONFIGURATIONS_PDSIZE_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_LAN_CONFIGURATIONS_PDWKUPH_WKUPTHRESH0
 ******************************************************************************/
const ru_field_rec BBH_TX_LAN_CONFIGURATIONS_PDWKUPH_WKUPTHRESH0_FIELD =
{
    "WKUPTHRESH0",
#if RU_INCLUDE_DESC
    "FIFO_wake_up_threshold_0",
    "The wakeup threshold of the PD FIFO for queue 0."
    "A value of n refers to n+1."
    "Relevant only for EPON BBH.",
#endif
    BBH_TX_LAN_CONFIGURATIONS_PDWKUPH_WKUPTHRESH0_FIELD_MASK,
    0,
    BBH_TX_LAN_CONFIGURATIONS_PDWKUPH_WKUPTHRESH0_FIELD_WIDTH,
    BBH_TX_LAN_CONFIGURATIONS_PDWKUPH_WKUPTHRESH0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_LAN_CONFIGURATIONS_PDWKUPH_WKUPTHRESH1
 ******************************************************************************/
const ru_field_rec BBH_TX_LAN_CONFIGURATIONS_PDWKUPH_WKUPTHRESH1_FIELD =
{
    "WKUPTHRESH1",
#if RU_INCLUDE_DESC
    "FIFO_wake_up_threshold_1",
    "The wakeup threshold of the PD FIFO for queue 1."
    "A value of n refers to n+1."
    "Relevant only for EPON BBH.",
#endif
    BBH_TX_LAN_CONFIGURATIONS_PDWKUPH_WKUPTHRESH1_FIELD_MASK,
    0,
    BBH_TX_LAN_CONFIGURATIONS_PDWKUPH_WKUPTHRESH1_FIELD_WIDTH,
    BBH_TX_LAN_CONFIGURATIONS_PDWKUPH_WKUPTHRESH1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_LAN_CONFIGURATIONS_PDWKUPH_RESERVED0
 ******************************************************************************/
const ru_field_rec BBH_TX_LAN_CONFIGURATIONS_PDWKUPH_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    BBH_TX_LAN_CONFIGURATIONS_PDWKUPH_RESERVED0_FIELD_MASK,
    0,
    BBH_TX_LAN_CONFIGURATIONS_PDWKUPH_RESERVED0_FIELD_WIDTH,
    BBH_TX_LAN_CONFIGURATIONS_PDWKUPH_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_LAN_CONFIGURATIONS_PD_BYTE_TH_PDLIMIT0
 ******************************************************************************/
const ru_field_rec BBH_TX_LAN_CONFIGURATIONS_PD_BYTE_TH_PDLIMIT0_FIELD =
{
    "PDLIMIT0",
#if RU_INCLUDE_DESC
    "PD_limit_0",
    "Defines the number of bytes for PDs pre fetch limited according to the total number of bytes."
    "The value is in 8-bytes resolution."
    "",
#endif
    BBH_TX_LAN_CONFIGURATIONS_PD_BYTE_TH_PDLIMIT0_FIELD_MASK,
    0,
    BBH_TX_LAN_CONFIGURATIONS_PD_BYTE_TH_PDLIMIT0_FIELD_WIDTH,
    BBH_TX_LAN_CONFIGURATIONS_PD_BYTE_TH_PDLIMIT0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_LAN_CONFIGURATIONS_PD_BYTE_TH_PDLIMIT1
 ******************************************************************************/
const ru_field_rec BBH_TX_LAN_CONFIGURATIONS_PD_BYTE_TH_PDLIMIT1_FIELD =
{
    "PDLIMIT1",
#if RU_INCLUDE_DESC
    "PD_limit_1",
    "Defines the number of bytes for PDs pre fetch limited according to the total number of bytes."
    "The value is in 8-bytes resolution."
    "",
#endif
    BBH_TX_LAN_CONFIGURATIONS_PD_BYTE_TH_PDLIMIT1_FIELD_MASK,
    0,
    BBH_TX_LAN_CONFIGURATIONS_PD_BYTE_TH_PDLIMIT1_FIELD_WIDTH,
    BBH_TX_LAN_CONFIGURATIONS_PD_BYTE_TH_PDLIMIT1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_LAN_CONFIGURATIONS_PD_BYTE_TH_EN_PDLIMITEN
 ******************************************************************************/
const ru_field_rec BBH_TX_LAN_CONFIGURATIONS_PD_BYTE_TH_EN_PDLIMITEN_FIELD =
{
    "PDLIMITEN",
#if RU_INCLUDE_DESC
    "PD_limit_enable",
    "This bit enables the above feature (PDs pre fetch limited according to the total number of bytes).",
#endif
    BBH_TX_LAN_CONFIGURATIONS_PD_BYTE_TH_EN_PDLIMITEN_FIELD_MASK,
    0,
    BBH_TX_LAN_CONFIGURATIONS_PD_BYTE_TH_EN_PDLIMITEN_FIELD_WIDTH,
    BBH_TX_LAN_CONFIGURATIONS_PD_BYTE_TH_EN_PDLIMITEN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_LAN_CONFIGURATIONS_PD_BYTE_TH_EN_RESERVED0
 ******************************************************************************/
const ru_field_rec BBH_TX_LAN_CONFIGURATIONS_PD_BYTE_TH_EN_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    BBH_TX_LAN_CONFIGURATIONS_PD_BYTE_TH_EN_RESERVED0_FIELD_MASK,
    0,
    BBH_TX_LAN_CONFIGURATIONS_PD_BYTE_TH_EN_RESERVED0_FIELD_WIDTH,
    BBH_TX_LAN_CONFIGURATIONS_PD_BYTE_TH_EN_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_LAN_CONFIGURATIONS_PDEMPTY_EMPTY
 ******************************************************************************/
const ru_field_rec BBH_TX_LAN_CONFIGURATIONS_PDEMPTY_EMPTY_FIELD =
{
    "EMPTY",
#if RU_INCLUDE_DESC
    "Empty_thershold",
    "EPON PD FIFO empty threshold."
    "A queue which its PD FIFO occupancy is below this threshold will have high priority in PD ordering arbitration.",
#endif
    BBH_TX_LAN_CONFIGURATIONS_PDEMPTY_EMPTY_FIELD_MASK,
    0,
    BBH_TX_LAN_CONFIGURATIONS_PDEMPTY_EMPTY_FIELD_WIDTH,
    BBH_TX_LAN_CONFIGURATIONS_PDEMPTY_EMPTY_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_LAN_CONFIGURATIONS_PDEMPTY_RESERVED0
 ******************************************************************************/
const ru_field_rec BBH_TX_LAN_CONFIGURATIONS_PDEMPTY_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    BBH_TX_LAN_CONFIGURATIONS_PDEMPTY_RESERVED0_FIELD_MASK,
    0,
    BBH_TX_LAN_CONFIGURATIONS_PDEMPTY_RESERVED0_FIELD_WIDTH,
    BBH_TX_LAN_CONFIGURATIONS_PDEMPTY_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_LAN_CONFIGURATIONS_TXTHRESH_DDRTHRESH
 ******************************************************************************/
const ru_field_rec BBH_TX_LAN_CONFIGURATIONS_TXTHRESH_DDRTHRESH_FIELD =
{
    "DDRTHRESH",
#if RU_INCLUDE_DESC
    "ddr_tx_threshold",
    "DDR Transmit threshold in 8 bytes resoltion",
#endif
    BBH_TX_LAN_CONFIGURATIONS_TXTHRESH_DDRTHRESH_FIELD_MASK,
    0,
    BBH_TX_LAN_CONFIGURATIONS_TXTHRESH_DDRTHRESH_FIELD_WIDTH,
    BBH_TX_LAN_CONFIGURATIONS_TXTHRESH_DDRTHRESH_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_LAN_CONFIGURATIONS_TXTHRESH_RESERVED0
 ******************************************************************************/
const ru_field_rec BBH_TX_LAN_CONFIGURATIONS_TXTHRESH_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    BBH_TX_LAN_CONFIGURATIONS_TXTHRESH_RESERVED0_FIELD_MASK,
    0,
    BBH_TX_LAN_CONFIGURATIONS_TXTHRESH_RESERVED0_FIELD_WIDTH,
    BBH_TX_LAN_CONFIGURATIONS_TXTHRESH_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_LAN_CONFIGURATIONS_TXTHRESH_SRAMTHRESH
 ******************************************************************************/
const ru_field_rec BBH_TX_LAN_CONFIGURATIONS_TXTHRESH_SRAMTHRESH_FIELD =
{
    "SRAMTHRESH",
#if RU_INCLUDE_DESC
    "sram_tx_threshold",
    "SRAM Transmit threshold in 8 bytes resoltion",
#endif
    BBH_TX_LAN_CONFIGURATIONS_TXTHRESH_SRAMTHRESH_FIELD_MASK,
    0,
    BBH_TX_LAN_CONFIGURATIONS_TXTHRESH_SRAMTHRESH_FIELD_WIDTH,
    BBH_TX_LAN_CONFIGURATIONS_TXTHRESH_SRAMTHRESH_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_LAN_CONFIGURATIONS_TXTHRESH_RESERVED1
 ******************************************************************************/
const ru_field_rec BBH_TX_LAN_CONFIGURATIONS_TXTHRESH_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    BBH_TX_LAN_CONFIGURATIONS_TXTHRESH_RESERVED1_FIELD_MASK,
    0,
    BBH_TX_LAN_CONFIGURATIONS_TXTHRESH_RESERVED1_FIELD_WIDTH,
    BBH_TX_LAN_CONFIGURATIONS_TXTHRESH_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_LAN_CONFIGURATIONS_EEE_EN
 ******************************************************************************/
const ru_field_rec BBH_TX_LAN_CONFIGURATIONS_EEE_EN_FIELD =
{
    "EN",
#if RU_INCLUDE_DESC
    "enable",
    "enable bit",
#endif
    BBH_TX_LAN_CONFIGURATIONS_EEE_EN_FIELD_MASK,
    0,
    BBH_TX_LAN_CONFIGURATIONS_EEE_EN_FIELD_WIDTH,
    BBH_TX_LAN_CONFIGURATIONS_EEE_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_LAN_CONFIGURATIONS_EEE_RESERVED0
 ******************************************************************************/
const ru_field_rec BBH_TX_LAN_CONFIGURATIONS_EEE_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    BBH_TX_LAN_CONFIGURATIONS_EEE_RESERVED0_FIELD_MASK,
    0,
    BBH_TX_LAN_CONFIGURATIONS_EEE_RESERVED0_FIELD_WIDTH,
    BBH_TX_LAN_CONFIGURATIONS_EEE_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_LAN_CONFIGURATIONS_TS_EN
 ******************************************************************************/
const ru_field_rec BBH_TX_LAN_CONFIGURATIONS_TS_EN_FIELD =
{
    "EN",
#if RU_INCLUDE_DESC
    "enable",
    "enable bit",
#endif
    BBH_TX_LAN_CONFIGURATIONS_TS_EN_FIELD_MASK,
    0,
    BBH_TX_LAN_CONFIGURATIONS_TS_EN_FIELD_WIDTH,
    BBH_TX_LAN_CONFIGURATIONS_TS_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_LAN_CONFIGURATIONS_TS_RESERVED0
 ******************************************************************************/
const ru_field_rec BBH_TX_LAN_CONFIGURATIONS_TS_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    BBH_TX_LAN_CONFIGURATIONS_TS_RESERVED0_FIELD_MASK,
    0,
    BBH_TX_LAN_CONFIGURATIONS_TS_RESERVED0_FIELD_WIDTH,
    BBH_TX_LAN_CONFIGURATIONS_TS_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_UNIFIED_CONFIGURATIONS_PDBASE_FIFOBASE0
 ******************************************************************************/
const ru_field_rec BBH_TX_UNIFIED_CONFIGURATIONS_PDBASE_FIFOBASE0_FIELD =
{
    "FIFOBASE0",
#if RU_INCLUDE_DESC
    "FIFO_base_0",
    "The base of PD FIFO for queue 0.",
#endif
    BBH_TX_UNIFIED_CONFIGURATIONS_PDBASE_FIFOBASE0_FIELD_MASK,
    0,
    BBH_TX_UNIFIED_CONFIGURATIONS_PDBASE_FIFOBASE0_FIELD_WIDTH,
    BBH_TX_UNIFIED_CONFIGURATIONS_PDBASE_FIFOBASE0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_UNIFIED_CONFIGURATIONS_PDBASE_RESERVED0
 ******************************************************************************/
const ru_field_rec BBH_TX_UNIFIED_CONFIGURATIONS_PDBASE_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    BBH_TX_UNIFIED_CONFIGURATIONS_PDBASE_RESERVED0_FIELD_MASK,
    0,
    BBH_TX_UNIFIED_CONFIGURATIONS_PDBASE_RESERVED0_FIELD_WIDTH,
    BBH_TX_UNIFIED_CONFIGURATIONS_PDBASE_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_UNIFIED_CONFIGURATIONS_PDBASE_FIFOBASE1
 ******************************************************************************/
const ru_field_rec BBH_TX_UNIFIED_CONFIGURATIONS_PDBASE_FIFOBASE1_FIELD =
{
    "FIFOBASE1",
#if RU_INCLUDE_DESC
    "FIFO_base_1",
    "The base of PD FIFO for queue 1.",
#endif
    BBH_TX_UNIFIED_CONFIGURATIONS_PDBASE_FIFOBASE1_FIELD_MASK,
    0,
    BBH_TX_UNIFIED_CONFIGURATIONS_PDBASE_FIFOBASE1_FIELD_WIDTH,
    BBH_TX_UNIFIED_CONFIGURATIONS_PDBASE_FIFOBASE1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_UNIFIED_CONFIGURATIONS_PDBASE_RESERVED1
 ******************************************************************************/
const ru_field_rec BBH_TX_UNIFIED_CONFIGURATIONS_PDBASE_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    BBH_TX_UNIFIED_CONFIGURATIONS_PDBASE_RESERVED1_FIELD_MASK,
    0,
    BBH_TX_UNIFIED_CONFIGURATIONS_PDBASE_RESERVED1_FIELD_WIDTH,
    BBH_TX_UNIFIED_CONFIGURATIONS_PDBASE_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_UNIFIED_CONFIGURATIONS_PDSIZE_FIFOSIZE0
 ******************************************************************************/
const ru_field_rec BBH_TX_UNIFIED_CONFIGURATIONS_PDSIZE_FIFOSIZE0_FIELD =
{
    "FIFOSIZE0",
#if RU_INCLUDE_DESC
    "FIFO_size_0",
    "The size of PD FIFO for queue 0."
    "A value of n refers to n+1."
    "For GPON, the max value is 0x7"
    "For EPON, the max value is 0xf",
#endif
    BBH_TX_UNIFIED_CONFIGURATIONS_PDSIZE_FIFOSIZE0_FIELD_MASK,
    0,
    BBH_TX_UNIFIED_CONFIGURATIONS_PDSIZE_FIFOSIZE0_FIELD_WIDTH,
    BBH_TX_UNIFIED_CONFIGURATIONS_PDSIZE_FIFOSIZE0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_UNIFIED_CONFIGURATIONS_PDSIZE_RESERVED0
 ******************************************************************************/
const ru_field_rec BBH_TX_UNIFIED_CONFIGURATIONS_PDSIZE_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    BBH_TX_UNIFIED_CONFIGURATIONS_PDSIZE_RESERVED0_FIELD_MASK,
    0,
    BBH_TX_UNIFIED_CONFIGURATIONS_PDSIZE_RESERVED0_FIELD_WIDTH,
    BBH_TX_UNIFIED_CONFIGURATIONS_PDSIZE_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_UNIFIED_CONFIGURATIONS_PDSIZE_FIFOSIZE1
 ******************************************************************************/
const ru_field_rec BBH_TX_UNIFIED_CONFIGURATIONS_PDSIZE_FIFOSIZE1_FIELD =
{
    "FIFOSIZE1",
#if RU_INCLUDE_DESC
    "FIFO_size_1",
    "The size of PD FIFO for queue 1."
    "A value of n refers to n+1."
    "",
#endif
    BBH_TX_UNIFIED_CONFIGURATIONS_PDSIZE_FIFOSIZE1_FIELD_MASK,
    0,
    BBH_TX_UNIFIED_CONFIGURATIONS_PDSIZE_FIFOSIZE1_FIELD_WIDTH,
    BBH_TX_UNIFIED_CONFIGURATIONS_PDSIZE_FIFOSIZE1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_UNIFIED_CONFIGURATIONS_PDSIZE_RESERVED1
 ******************************************************************************/
const ru_field_rec BBH_TX_UNIFIED_CONFIGURATIONS_PDSIZE_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    BBH_TX_UNIFIED_CONFIGURATIONS_PDSIZE_RESERVED1_FIELD_MASK,
    0,
    BBH_TX_UNIFIED_CONFIGURATIONS_PDSIZE_RESERVED1_FIELD_WIDTH,
    BBH_TX_UNIFIED_CONFIGURATIONS_PDSIZE_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_UNIFIED_CONFIGURATIONS_PDWKUPH_WKUPTHRESH0
 ******************************************************************************/
const ru_field_rec BBH_TX_UNIFIED_CONFIGURATIONS_PDWKUPH_WKUPTHRESH0_FIELD =
{
    "WKUPTHRESH0",
#if RU_INCLUDE_DESC
    "FIFO_wake_up_threshold_0",
    "The wakeup threshold of the PD FIFO for queue 0."
    "A value of n refers to n+1."
    "Relevant only for EPON BBH.",
#endif
    BBH_TX_UNIFIED_CONFIGURATIONS_PDWKUPH_WKUPTHRESH0_FIELD_MASK,
    0,
    BBH_TX_UNIFIED_CONFIGURATIONS_PDWKUPH_WKUPTHRESH0_FIELD_WIDTH,
    BBH_TX_UNIFIED_CONFIGURATIONS_PDWKUPH_WKUPTHRESH0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_UNIFIED_CONFIGURATIONS_PDWKUPH_WKUPTHRESH1
 ******************************************************************************/
const ru_field_rec BBH_TX_UNIFIED_CONFIGURATIONS_PDWKUPH_WKUPTHRESH1_FIELD =
{
    "WKUPTHRESH1",
#if RU_INCLUDE_DESC
    "FIFO_wake_up_threshold_1",
    "The wakeup threshold of the PD FIFO for queue 1."
    "A value of n refers to n+1."
    "Relevant only for EPON BBH.",
#endif
    BBH_TX_UNIFIED_CONFIGURATIONS_PDWKUPH_WKUPTHRESH1_FIELD_MASK,
    0,
    BBH_TX_UNIFIED_CONFIGURATIONS_PDWKUPH_WKUPTHRESH1_FIELD_WIDTH,
    BBH_TX_UNIFIED_CONFIGURATIONS_PDWKUPH_WKUPTHRESH1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_UNIFIED_CONFIGURATIONS_PDWKUPH_RESERVED0
 ******************************************************************************/
const ru_field_rec BBH_TX_UNIFIED_CONFIGURATIONS_PDWKUPH_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    BBH_TX_UNIFIED_CONFIGURATIONS_PDWKUPH_RESERVED0_FIELD_MASK,
    0,
    BBH_TX_UNIFIED_CONFIGURATIONS_PDWKUPH_RESERVED0_FIELD_WIDTH,
    BBH_TX_UNIFIED_CONFIGURATIONS_PDWKUPH_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_UNIFIED_CONFIGURATIONS_PD_BYTE_TH_PDLIMIT0
 ******************************************************************************/
const ru_field_rec BBH_TX_UNIFIED_CONFIGURATIONS_PD_BYTE_TH_PDLIMIT0_FIELD =
{
    "PDLIMIT0",
#if RU_INCLUDE_DESC
    "PD_limit_0",
    "Defines the number of bytes for PDs pre fetch limited according to the total number of bytes."
    "The value is in 8-bytes resolution."
    "",
#endif
    BBH_TX_UNIFIED_CONFIGURATIONS_PD_BYTE_TH_PDLIMIT0_FIELD_MASK,
    0,
    BBH_TX_UNIFIED_CONFIGURATIONS_PD_BYTE_TH_PDLIMIT0_FIELD_WIDTH,
    BBH_TX_UNIFIED_CONFIGURATIONS_PD_BYTE_TH_PDLIMIT0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_UNIFIED_CONFIGURATIONS_PD_BYTE_TH_PDLIMIT1
 ******************************************************************************/
const ru_field_rec BBH_TX_UNIFIED_CONFIGURATIONS_PD_BYTE_TH_PDLIMIT1_FIELD =
{
    "PDLIMIT1",
#if RU_INCLUDE_DESC
    "PD_limit_1",
    "Defines the number of bytes for PDs pre fetch limited according to the total number of bytes."
    "The value is in 8-bytes resolution."
    "",
#endif
    BBH_TX_UNIFIED_CONFIGURATIONS_PD_BYTE_TH_PDLIMIT1_FIELD_MASK,
    0,
    BBH_TX_UNIFIED_CONFIGURATIONS_PD_BYTE_TH_PDLIMIT1_FIELD_WIDTH,
    BBH_TX_UNIFIED_CONFIGURATIONS_PD_BYTE_TH_PDLIMIT1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_UNIFIED_CONFIGURATIONS_PD_BYTE_TH_EN_PDLIMITEN
 ******************************************************************************/
const ru_field_rec BBH_TX_UNIFIED_CONFIGURATIONS_PD_BYTE_TH_EN_PDLIMITEN_FIELD =
{
    "PDLIMITEN",
#if RU_INCLUDE_DESC
    "PD_limit_enable",
    "This bit enables the above feature (PDs pre fetch limited according to the total number of bytes).",
#endif
    BBH_TX_UNIFIED_CONFIGURATIONS_PD_BYTE_TH_EN_PDLIMITEN_FIELD_MASK,
    0,
    BBH_TX_UNIFIED_CONFIGURATIONS_PD_BYTE_TH_EN_PDLIMITEN_FIELD_WIDTH,
    BBH_TX_UNIFIED_CONFIGURATIONS_PD_BYTE_TH_EN_PDLIMITEN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_UNIFIED_CONFIGURATIONS_PD_BYTE_TH_EN_RESERVED0
 ******************************************************************************/
const ru_field_rec BBH_TX_UNIFIED_CONFIGURATIONS_PD_BYTE_TH_EN_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    BBH_TX_UNIFIED_CONFIGURATIONS_PD_BYTE_TH_EN_RESERVED0_FIELD_MASK,
    0,
    BBH_TX_UNIFIED_CONFIGURATIONS_PD_BYTE_TH_EN_RESERVED0_FIELD_WIDTH,
    BBH_TX_UNIFIED_CONFIGURATIONS_PD_BYTE_TH_EN_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_UNIFIED_CONFIGURATIONS_PDEMPTY_EMPTY
 ******************************************************************************/
const ru_field_rec BBH_TX_UNIFIED_CONFIGURATIONS_PDEMPTY_EMPTY_FIELD =
{
    "EMPTY",
#if RU_INCLUDE_DESC
    "Empty_thershold",
    "EPON PD FIFO empty threshold."
    "A queue which its PD FIFO occupancy is below this threshold will have high priority in PD ordering arbitration.",
#endif
    BBH_TX_UNIFIED_CONFIGURATIONS_PDEMPTY_EMPTY_FIELD_MASK,
    0,
    BBH_TX_UNIFIED_CONFIGURATIONS_PDEMPTY_EMPTY_FIELD_WIDTH,
    BBH_TX_UNIFIED_CONFIGURATIONS_PDEMPTY_EMPTY_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_UNIFIED_CONFIGURATIONS_PDEMPTY_RESERVED0
 ******************************************************************************/
const ru_field_rec BBH_TX_UNIFIED_CONFIGURATIONS_PDEMPTY_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    BBH_TX_UNIFIED_CONFIGURATIONS_PDEMPTY_RESERVED0_FIELD_MASK,
    0,
    BBH_TX_UNIFIED_CONFIGURATIONS_PDEMPTY_RESERVED0_FIELD_WIDTH,
    BBH_TX_UNIFIED_CONFIGURATIONS_PDEMPTY_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_UNIFIED_CONFIGURATIONS_GTXTHRESH_DDRTHRESH
 ******************************************************************************/
const ru_field_rec BBH_TX_UNIFIED_CONFIGURATIONS_GTXTHRESH_DDRTHRESH_FIELD =
{
    "DDRTHRESH",
#if RU_INCLUDE_DESC
    "ddr_tx_threshold",
    "DDR Transmit threshold in 8 bytes resoltion",
#endif
    BBH_TX_UNIFIED_CONFIGURATIONS_GTXTHRESH_DDRTHRESH_FIELD_MASK,
    0,
    BBH_TX_UNIFIED_CONFIGURATIONS_GTXTHRESH_DDRTHRESH_FIELD_WIDTH,
    BBH_TX_UNIFIED_CONFIGURATIONS_GTXTHRESH_DDRTHRESH_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_UNIFIED_CONFIGURATIONS_GTXTHRESH_RESERVED0
 ******************************************************************************/
const ru_field_rec BBH_TX_UNIFIED_CONFIGURATIONS_GTXTHRESH_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    BBH_TX_UNIFIED_CONFIGURATIONS_GTXTHRESH_RESERVED0_FIELD_MASK,
    0,
    BBH_TX_UNIFIED_CONFIGURATIONS_GTXTHRESH_RESERVED0_FIELD_WIDTH,
    BBH_TX_UNIFIED_CONFIGURATIONS_GTXTHRESH_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_UNIFIED_CONFIGURATIONS_GTXTHRESH_SRAMTHRESH
 ******************************************************************************/
const ru_field_rec BBH_TX_UNIFIED_CONFIGURATIONS_GTXTHRESH_SRAMTHRESH_FIELD =
{
    "SRAMTHRESH",
#if RU_INCLUDE_DESC
    "sram_tx_threshold",
    "SRAM Transmit threshold in 8 bytes resoltion",
#endif
    BBH_TX_UNIFIED_CONFIGURATIONS_GTXTHRESH_SRAMTHRESH_FIELD_MASK,
    0,
    BBH_TX_UNIFIED_CONFIGURATIONS_GTXTHRESH_SRAMTHRESH_FIELD_WIDTH,
    BBH_TX_UNIFIED_CONFIGURATIONS_GTXTHRESH_SRAMTHRESH_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_UNIFIED_CONFIGURATIONS_GTXTHRESH_RESERVED1
 ******************************************************************************/
const ru_field_rec BBH_TX_UNIFIED_CONFIGURATIONS_GTXTHRESH_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    BBH_TX_UNIFIED_CONFIGURATIONS_GTXTHRESH_RESERVED1_FIELD_MASK,
    0,
    BBH_TX_UNIFIED_CONFIGURATIONS_GTXTHRESH_RESERVED1_FIELD_WIDTH,
    BBH_TX_UNIFIED_CONFIGURATIONS_GTXTHRESH_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_UNIFIED_CONFIGURATIONS_EEE_EN
 ******************************************************************************/
const ru_field_rec BBH_TX_UNIFIED_CONFIGURATIONS_EEE_EN_FIELD =
{
    "EN",
#if RU_INCLUDE_DESC
    "enable",
    "enable bit",
#endif
    BBH_TX_UNIFIED_CONFIGURATIONS_EEE_EN_FIELD_MASK,
    0,
    BBH_TX_UNIFIED_CONFIGURATIONS_EEE_EN_FIELD_WIDTH,
    BBH_TX_UNIFIED_CONFIGURATIONS_EEE_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_UNIFIED_CONFIGURATIONS_EEE_RESERVED0
 ******************************************************************************/
const ru_field_rec BBH_TX_UNIFIED_CONFIGURATIONS_EEE_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    BBH_TX_UNIFIED_CONFIGURATIONS_EEE_RESERVED0_FIELD_MASK,
    0,
    BBH_TX_UNIFIED_CONFIGURATIONS_EEE_RESERVED0_FIELD_WIDTH,
    BBH_TX_UNIFIED_CONFIGURATIONS_EEE_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_UNIFIED_CONFIGURATIONS_TS_EN
 ******************************************************************************/
const ru_field_rec BBH_TX_UNIFIED_CONFIGURATIONS_TS_EN_FIELD =
{
    "EN",
#if RU_INCLUDE_DESC
    "enable",
    "enable bit",
#endif
    BBH_TX_UNIFIED_CONFIGURATIONS_TS_EN_FIELD_MASK,
    0,
    BBH_TX_UNIFIED_CONFIGURATIONS_TS_EN_FIELD_WIDTH,
    BBH_TX_UNIFIED_CONFIGURATIONS_TS_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_UNIFIED_CONFIGURATIONS_TS_RESERVED0
 ******************************************************************************/
const ru_field_rec BBH_TX_UNIFIED_CONFIGURATIONS_TS_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    BBH_TX_UNIFIED_CONFIGURATIONS_TS_RESERVED0_FIELD_MASK,
    0,
    BBH_TX_UNIFIED_CONFIGURATIONS_TS_RESERVED0_FIELD_WIDTH,
    BBH_TX_UNIFIED_CONFIGURATIONS_TS_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_UNIFIED_CONFIGURATIONS_FEBASE_FIFOBASE0
 ******************************************************************************/
const ru_field_rec BBH_TX_UNIFIED_CONFIGURATIONS_FEBASE_FIFOBASE0_FIELD =
{
    "FIFOBASE0",
#if RU_INCLUDE_DESC
    "FIFO_base_0",
    "The base of FE FIFO for queue 0.",
#endif
    BBH_TX_UNIFIED_CONFIGURATIONS_FEBASE_FIFOBASE0_FIELD_MASK,
    0,
    BBH_TX_UNIFIED_CONFIGURATIONS_FEBASE_FIFOBASE0_FIELD_WIDTH,
    BBH_TX_UNIFIED_CONFIGURATIONS_FEBASE_FIFOBASE0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_UNIFIED_CONFIGURATIONS_FEBASE_RESERVED0
 ******************************************************************************/
const ru_field_rec BBH_TX_UNIFIED_CONFIGURATIONS_FEBASE_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    BBH_TX_UNIFIED_CONFIGURATIONS_FEBASE_RESERVED0_FIELD_MASK,
    0,
    BBH_TX_UNIFIED_CONFIGURATIONS_FEBASE_RESERVED0_FIELD_WIDTH,
    BBH_TX_UNIFIED_CONFIGURATIONS_FEBASE_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_UNIFIED_CONFIGURATIONS_FEBASE_FIFOBASE1
 ******************************************************************************/
const ru_field_rec BBH_TX_UNIFIED_CONFIGURATIONS_FEBASE_FIFOBASE1_FIELD =
{
    "FIFOBASE1",
#if RU_INCLUDE_DESC
    "FIFO_base_1",
    "The base of FE FIFO for queue 1.",
#endif
    BBH_TX_UNIFIED_CONFIGURATIONS_FEBASE_FIFOBASE1_FIELD_MASK,
    0,
    BBH_TX_UNIFIED_CONFIGURATIONS_FEBASE_FIFOBASE1_FIELD_WIDTH,
    BBH_TX_UNIFIED_CONFIGURATIONS_FEBASE_FIFOBASE1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_UNIFIED_CONFIGURATIONS_FEBASE_RESERVED1
 ******************************************************************************/
const ru_field_rec BBH_TX_UNIFIED_CONFIGURATIONS_FEBASE_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    BBH_TX_UNIFIED_CONFIGURATIONS_FEBASE_RESERVED1_FIELD_MASK,
    0,
    BBH_TX_UNIFIED_CONFIGURATIONS_FEBASE_RESERVED1_FIELD_WIDTH,
    BBH_TX_UNIFIED_CONFIGURATIONS_FEBASE_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_UNIFIED_CONFIGURATIONS_FESIZE_FIFOSIZE0
 ******************************************************************************/
const ru_field_rec BBH_TX_UNIFIED_CONFIGURATIONS_FESIZE_FIFOSIZE0_FIELD =
{
    "FIFOSIZE0",
#if RU_INCLUDE_DESC
    "FIFO_size_0",
    "The size of PD FIFO for queue 0."
    "A value of n refers to n+1."
    "",
#endif
    BBH_TX_UNIFIED_CONFIGURATIONS_FESIZE_FIFOSIZE0_FIELD_MASK,
    0,
    BBH_TX_UNIFIED_CONFIGURATIONS_FESIZE_FIFOSIZE0_FIELD_WIDTH,
    BBH_TX_UNIFIED_CONFIGURATIONS_FESIZE_FIFOSIZE0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_UNIFIED_CONFIGURATIONS_FESIZE_RESERVED0
 ******************************************************************************/
const ru_field_rec BBH_TX_UNIFIED_CONFIGURATIONS_FESIZE_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    BBH_TX_UNIFIED_CONFIGURATIONS_FESIZE_RESERVED0_FIELD_MASK,
    0,
    BBH_TX_UNIFIED_CONFIGURATIONS_FESIZE_RESERVED0_FIELD_WIDTH,
    BBH_TX_UNIFIED_CONFIGURATIONS_FESIZE_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_UNIFIED_CONFIGURATIONS_FESIZE_FIFOSIZE1
 ******************************************************************************/
const ru_field_rec BBH_TX_UNIFIED_CONFIGURATIONS_FESIZE_FIFOSIZE1_FIELD =
{
    "FIFOSIZE1",
#if RU_INCLUDE_DESC
    "FIFO_size_1",
    "The size of FE FIFO for queue 1."
    "A value of n refers to n+1."
    "",
#endif
    BBH_TX_UNIFIED_CONFIGURATIONS_FESIZE_FIFOSIZE1_FIELD_MASK,
    0,
    BBH_TX_UNIFIED_CONFIGURATIONS_FESIZE_FIFOSIZE1_FIELD_WIDTH,
    BBH_TX_UNIFIED_CONFIGURATIONS_FESIZE_FIFOSIZE1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_UNIFIED_CONFIGURATIONS_FESIZE_RESERVED1
 ******************************************************************************/
const ru_field_rec BBH_TX_UNIFIED_CONFIGURATIONS_FESIZE_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    BBH_TX_UNIFIED_CONFIGURATIONS_FESIZE_RESERVED1_FIELD_MASK,
    0,
    BBH_TX_UNIFIED_CONFIGURATIONS_FESIZE_RESERVED1_FIELD_WIDTH,
    BBH_TX_UNIFIED_CONFIGURATIONS_FESIZE_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_UNIFIED_CONFIGURATIONS_FEPDBASE_FIFOBASE0
 ******************************************************************************/
const ru_field_rec BBH_TX_UNIFIED_CONFIGURATIONS_FEPDBASE_FIFOBASE0_FIELD =
{
    "FIFOBASE0",
#if RU_INCLUDE_DESC
    "FIFO_base_0",
    "The base of FE PD FIFO for queue 0.",
#endif
    BBH_TX_UNIFIED_CONFIGURATIONS_FEPDBASE_FIFOBASE0_FIELD_MASK,
    0,
    BBH_TX_UNIFIED_CONFIGURATIONS_FEPDBASE_FIFOBASE0_FIELD_WIDTH,
    BBH_TX_UNIFIED_CONFIGURATIONS_FEPDBASE_FIFOBASE0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_UNIFIED_CONFIGURATIONS_FEPDBASE_RESERVED0
 ******************************************************************************/
const ru_field_rec BBH_TX_UNIFIED_CONFIGURATIONS_FEPDBASE_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    BBH_TX_UNIFIED_CONFIGURATIONS_FEPDBASE_RESERVED0_FIELD_MASK,
    0,
    BBH_TX_UNIFIED_CONFIGURATIONS_FEPDBASE_RESERVED0_FIELD_WIDTH,
    BBH_TX_UNIFIED_CONFIGURATIONS_FEPDBASE_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_UNIFIED_CONFIGURATIONS_FEPDBASE_FIFOBASE1
 ******************************************************************************/
const ru_field_rec BBH_TX_UNIFIED_CONFIGURATIONS_FEPDBASE_FIFOBASE1_FIELD =
{
    "FIFOBASE1",
#if RU_INCLUDE_DESC
    "FIFO_base_1",
    "The base of FE PD FIFO for queue 1.",
#endif
    BBH_TX_UNIFIED_CONFIGURATIONS_FEPDBASE_FIFOBASE1_FIELD_MASK,
    0,
    BBH_TX_UNIFIED_CONFIGURATIONS_FEPDBASE_FIFOBASE1_FIELD_WIDTH,
    BBH_TX_UNIFIED_CONFIGURATIONS_FEPDBASE_FIFOBASE1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_UNIFIED_CONFIGURATIONS_FEPDBASE_RESERVED1
 ******************************************************************************/
const ru_field_rec BBH_TX_UNIFIED_CONFIGURATIONS_FEPDBASE_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    BBH_TX_UNIFIED_CONFIGURATIONS_FEPDBASE_RESERVED1_FIELD_MASK,
    0,
    BBH_TX_UNIFIED_CONFIGURATIONS_FEPDBASE_RESERVED1_FIELD_WIDTH,
    BBH_TX_UNIFIED_CONFIGURATIONS_FEPDBASE_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_UNIFIED_CONFIGURATIONS_FEPDSIZE_FIFOSIZE0
 ******************************************************************************/
const ru_field_rec BBH_TX_UNIFIED_CONFIGURATIONS_FEPDSIZE_FIFOSIZE0_FIELD =
{
    "FIFOSIZE0",
#if RU_INCLUDE_DESC
    "FIFO_size_0",
    "The size of FE PD FIFO for queue 0."
    "A value of n refers to n+1."
    "",
#endif
    BBH_TX_UNIFIED_CONFIGURATIONS_FEPDSIZE_FIFOSIZE0_FIELD_MASK,
    0,
    BBH_TX_UNIFIED_CONFIGURATIONS_FEPDSIZE_FIFOSIZE0_FIELD_WIDTH,
    BBH_TX_UNIFIED_CONFIGURATIONS_FEPDSIZE_FIFOSIZE0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_UNIFIED_CONFIGURATIONS_FEPDSIZE_RESERVED0
 ******************************************************************************/
const ru_field_rec BBH_TX_UNIFIED_CONFIGURATIONS_FEPDSIZE_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    BBH_TX_UNIFIED_CONFIGURATIONS_FEPDSIZE_RESERVED0_FIELD_MASK,
    0,
    BBH_TX_UNIFIED_CONFIGURATIONS_FEPDSIZE_RESERVED0_FIELD_WIDTH,
    BBH_TX_UNIFIED_CONFIGURATIONS_FEPDSIZE_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_UNIFIED_CONFIGURATIONS_FEPDSIZE_FIFOSIZE1
 ******************************************************************************/
const ru_field_rec BBH_TX_UNIFIED_CONFIGURATIONS_FEPDSIZE_FIFOSIZE1_FIELD =
{
    "FIFOSIZE1",
#if RU_INCLUDE_DESC
    "FIFO_size_1",
    "The size of FE PD FIFO for queue 1."
    "A value of n refers to n+1."
    "",
#endif
    BBH_TX_UNIFIED_CONFIGURATIONS_FEPDSIZE_FIFOSIZE1_FIELD_MASK,
    0,
    BBH_TX_UNIFIED_CONFIGURATIONS_FEPDSIZE_FIFOSIZE1_FIELD_WIDTH,
    BBH_TX_UNIFIED_CONFIGURATIONS_FEPDSIZE_FIFOSIZE1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_UNIFIED_CONFIGURATIONS_FEPDSIZE_RESERVED1
 ******************************************************************************/
const ru_field_rec BBH_TX_UNIFIED_CONFIGURATIONS_FEPDSIZE_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    BBH_TX_UNIFIED_CONFIGURATIONS_FEPDSIZE_RESERVED1_FIELD_MASK,
    0,
    BBH_TX_UNIFIED_CONFIGURATIONS_FEPDSIZE_RESERVED1_FIELD_WIDTH,
    BBH_TX_UNIFIED_CONFIGURATIONS_FEPDSIZE_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_UNIFIED_CONFIGURATIONS_TXWRR_W0
 ******************************************************************************/
const ru_field_rec BBH_TX_UNIFIED_CONFIGURATIONS_TXWRR_W0_FIELD =
{
    "W0",
#if RU_INCLUDE_DESC
    "weight_0",
    "weight of MAC0",
#endif
    BBH_TX_UNIFIED_CONFIGURATIONS_TXWRR_W0_FIELD_MASK,
    0,
    BBH_TX_UNIFIED_CONFIGURATIONS_TXWRR_W0_FIELD_WIDTH,
    BBH_TX_UNIFIED_CONFIGURATIONS_TXWRR_W0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_UNIFIED_CONFIGURATIONS_TXWRR_RESERVED0
 ******************************************************************************/
const ru_field_rec BBH_TX_UNIFIED_CONFIGURATIONS_TXWRR_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    BBH_TX_UNIFIED_CONFIGURATIONS_TXWRR_RESERVED0_FIELD_MASK,
    0,
    BBH_TX_UNIFIED_CONFIGURATIONS_TXWRR_RESERVED0_FIELD_WIDTH,
    BBH_TX_UNIFIED_CONFIGURATIONS_TXWRR_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_UNIFIED_CONFIGURATIONS_TXWRR_W1
 ******************************************************************************/
const ru_field_rec BBH_TX_UNIFIED_CONFIGURATIONS_TXWRR_W1_FIELD =
{
    "W1",
#if RU_INCLUDE_DESC
    "weight_1",
    "weight of MAC1",
#endif
    BBH_TX_UNIFIED_CONFIGURATIONS_TXWRR_W1_FIELD_MASK,
    0,
    BBH_TX_UNIFIED_CONFIGURATIONS_TXWRR_W1_FIELD_WIDTH,
    BBH_TX_UNIFIED_CONFIGURATIONS_TXWRR_W1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_UNIFIED_CONFIGURATIONS_TXWRR_RESERVED1
 ******************************************************************************/
const ru_field_rec BBH_TX_UNIFIED_CONFIGURATIONS_TXWRR_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    BBH_TX_UNIFIED_CONFIGURATIONS_TXWRR_RESERVED1_FIELD_MASK,
    0,
    BBH_TX_UNIFIED_CONFIGURATIONS_TXWRR_RESERVED1_FIELD_WIDTH,
    BBH_TX_UNIFIED_CONFIGURATIONS_TXWRR_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_UNIFIED_CONFIGURATIONS_TXTHRESH_THRESH0
 ******************************************************************************/
const ru_field_rec BBH_TX_UNIFIED_CONFIGURATIONS_TXTHRESH_THRESH0_FIELD =
{
    "THRESH0",
#if RU_INCLUDE_DESC
    "tx_threshold_0",
    "Transmit threshold in 8 bytes resoltion for mac 0",
#endif
    BBH_TX_UNIFIED_CONFIGURATIONS_TXTHRESH_THRESH0_FIELD_MASK,
    0,
    BBH_TX_UNIFIED_CONFIGURATIONS_TXTHRESH_THRESH0_FIELD_WIDTH,
    BBH_TX_UNIFIED_CONFIGURATIONS_TXTHRESH_THRESH0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_UNIFIED_CONFIGURATIONS_TXTHRESH_RESERVED0
 ******************************************************************************/
const ru_field_rec BBH_TX_UNIFIED_CONFIGURATIONS_TXTHRESH_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    BBH_TX_UNIFIED_CONFIGURATIONS_TXTHRESH_RESERVED0_FIELD_MASK,
    0,
    BBH_TX_UNIFIED_CONFIGURATIONS_TXTHRESH_RESERVED0_FIELD_WIDTH,
    BBH_TX_UNIFIED_CONFIGURATIONS_TXTHRESH_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_UNIFIED_CONFIGURATIONS_TXTHRESH_THRESH1
 ******************************************************************************/
const ru_field_rec BBH_TX_UNIFIED_CONFIGURATIONS_TXTHRESH_THRESH1_FIELD =
{
    "THRESH1",
#if RU_INCLUDE_DESC
    "tx_threshold_1",
    "Transmit threshold in 8 bytes resolution for MAC1",
#endif
    BBH_TX_UNIFIED_CONFIGURATIONS_TXTHRESH_THRESH1_FIELD_MASK,
    0,
    BBH_TX_UNIFIED_CONFIGURATIONS_TXTHRESH_THRESH1_FIELD_WIDTH,
    BBH_TX_UNIFIED_CONFIGURATIONS_TXTHRESH_THRESH1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_UNIFIED_CONFIGURATIONS_TXTHRESH_RESERVED1
 ******************************************************************************/
const ru_field_rec BBH_TX_UNIFIED_CONFIGURATIONS_TXTHRESH_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    BBH_TX_UNIFIED_CONFIGURATIONS_TXTHRESH_RESERVED1_FIELD_MASK,
    0,
    BBH_TX_UNIFIED_CONFIGURATIONS_TXTHRESH_RESERVED1_FIELD_WIDTH,
    BBH_TX_UNIFIED_CONFIGURATIONS_TXTHRESH_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_DEBUG_COUNTERS_SRAMPD_SRAMPD
 ******************************************************************************/
const ru_field_rec BBH_TX_DEBUG_COUNTERS_SRAMPD_SRAMPD_FIELD =
{
    "SRAMPD",
#if RU_INCLUDE_DESC
    "SRAM_PD",
    "This counter counts the number of packets which were transmitted from the SRAM.",
#endif
    BBH_TX_DEBUG_COUNTERS_SRAMPD_SRAMPD_FIELD_MASK,
    0,
    BBH_TX_DEBUG_COUNTERS_SRAMPD_SRAMPD_FIELD_WIDTH,
    BBH_TX_DEBUG_COUNTERS_SRAMPD_SRAMPD_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: BBH_TX_DEBUG_COUNTERS_DDRPD_DDRPD
 ******************************************************************************/
const ru_field_rec BBH_TX_DEBUG_COUNTERS_DDRPD_DDRPD_FIELD =
{
    "DDRPD",
#if RU_INCLUDE_DESC
    "DDR_PD",
    "This counter counts the number of packets which were transmitted from the DDR.",
#endif
    BBH_TX_DEBUG_COUNTERS_DDRPD_DDRPD_FIELD_MASK,
    0,
    BBH_TX_DEBUG_COUNTERS_DDRPD_DDRPD_FIELD_WIDTH,
    BBH_TX_DEBUG_COUNTERS_DDRPD_DDRPD_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: BBH_TX_DEBUG_COUNTERS_PDDROP_PDDROP
 ******************************************************************************/
const ru_field_rec BBH_TX_DEBUG_COUNTERS_PDDROP_PDDROP_FIELD =
{
    "PDDROP",
#if RU_INCLUDE_DESC
    "PD_DROP",
    "This counter counts the number of PDs which were dropped due to PD FIFO full.",
#endif
    BBH_TX_DEBUG_COUNTERS_PDDROP_PDDROP_FIELD_MASK,
    0,
    BBH_TX_DEBUG_COUNTERS_PDDROP_PDDROP_FIELD_WIDTH,
    BBH_TX_DEBUG_COUNTERS_PDDROP_PDDROP_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: BBH_TX_DEBUG_COUNTERS_PDDROP_RESERVED0
 ******************************************************************************/
const ru_field_rec BBH_TX_DEBUG_COUNTERS_PDDROP_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    BBH_TX_DEBUG_COUNTERS_PDDROP_RESERVED0_FIELD_MASK,
    0,
    BBH_TX_DEBUG_COUNTERS_PDDROP_RESERVED0_FIELD_WIDTH,
    BBH_TX_DEBUG_COUNTERS_PDDROP_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: BBH_TX_DEBUG_COUNTERS_STSCNT_STSCNT
 ******************************************************************************/
const ru_field_rec BBH_TX_DEBUG_COUNTERS_STSCNT_STSCNT_FIELD =
{
    "STSCNT",
#if RU_INCLUDE_DESC
    "STS_CNT",
    "This counter counts the number of received status messages.",
#endif
    BBH_TX_DEBUG_COUNTERS_STSCNT_STSCNT_FIELD_MASK,
    0,
    BBH_TX_DEBUG_COUNTERS_STSCNT_STSCNT_FIELD_WIDTH,
    BBH_TX_DEBUG_COUNTERS_STSCNT_STSCNT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: BBH_TX_DEBUG_COUNTERS_STSDROP_STSDROP
 ******************************************************************************/
const ru_field_rec BBH_TX_DEBUG_COUNTERS_STSDROP_STSDROP_FIELD =
{
    "STSDROP",
#if RU_INCLUDE_DESC
    "STS_DROP",
    "This counter counts the number of STS which were dropped due to PD FIFO full.",
#endif
    BBH_TX_DEBUG_COUNTERS_STSDROP_STSDROP_FIELD_MASK,
    0,
    BBH_TX_DEBUG_COUNTERS_STSDROP_STSDROP_FIELD_WIDTH,
    BBH_TX_DEBUG_COUNTERS_STSDROP_STSDROP_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: BBH_TX_DEBUG_COUNTERS_STSDROP_RESERVED0
 ******************************************************************************/
const ru_field_rec BBH_TX_DEBUG_COUNTERS_STSDROP_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    BBH_TX_DEBUG_COUNTERS_STSDROP_RESERVED0_FIELD_MASK,
    0,
    BBH_TX_DEBUG_COUNTERS_STSDROP_RESERVED0_FIELD_WIDTH,
    BBH_TX_DEBUG_COUNTERS_STSDROP_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: BBH_TX_DEBUG_COUNTERS_MSGCNT_MSGCNT
 ******************************************************************************/
const ru_field_rec BBH_TX_DEBUG_COUNTERS_MSGCNT_MSGCNT_FIELD =
{
    "MSGCNT",
#if RU_INCLUDE_DESC
    "MSG_CNT",
    "This counter counts the number of received DBR/ghost messages.",
#endif
    BBH_TX_DEBUG_COUNTERS_MSGCNT_MSGCNT_FIELD_MASK,
    0,
    BBH_TX_DEBUG_COUNTERS_MSGCNT_MSGCNT_FIELD_WIDTH,
    BBH_TX_DEBUG_COUNTERS_MSGCNT_MSGCNT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: BBH_TX_DEBUG_COUNTERS_MSGDROP_MSGDROP
 ******************************************************************************/
const ru_field_rec BBH_TX_DEBUG_COUNTERS_MSGDROP_MSGDROP_FIELD =
{
    "MSGDROP",
#if RU_INCLUDE_DESC
    "MSG_DROP",
    "This counter counts the number of MSG which were dropped due to PD FIFO full.",
#endif
    BBH_TX_DEBUG_COUNTERS_MSGDROP_MSGDROP_FIELD_MASK,
    0,
    BBH_TX_DEBUG_COUNTERS_MSGDROP_MSGDROP_FIELD_WIDTH,
    BBH_TX_DEBUG_COUNTERS_MSGDROP_MSGDROP_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: BBH_TX_DEBUG_COUNTERS_MSGDROP_RESERVED0
 ******************************************************************************/
const ru_field_rec BBH_TX_DEBUG_COUNTERS_MSGDROP_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    BBH_TX_DEBUG_COUNTERS_MSGDROP_RESERVED0_FIELD_MASK,
    0,
    BBH_TX_DEBUG_COUNTERS_MSGDROP_RESERVED0_FIELD_WIDTH,
    BBH_TX_DEBUG_COUNTERS_MSGDROP_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: BBH_TX_DEBUG_COUNTERS_GETNEXTNULL_GETNEXTNULL
 ******************************************************************************/
const ru_field_rec BBH_TX_DEBUG_COUNTERS_GETNEXTNULL_GETNEXTNULL_FIELD =
{
    "GETNEXTNULL",
#if RU_INCLUDE_DESC
    "Get_next_is_null",
    "This counter counts the number Get next responses with a null BN.",
#endif
    BBH_TX_DEBUG_COUNTERS_GETNEXTNULL_GETNEXTNULL_FIELD_MASK,
    0,
    BBH_TX_DEBUG_COUNTERS_GETNEXTNULL_GETNEXTNULL_FIELD_WIDTH,
    BBH_TX_DEBUG_COUNTERS_GETNEXTNULL_GETNEXTNULL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: BBH_TX_DEBUG_COUNTERS_GETNEXTNULL_RESERVED0
 ******************************************************************************/
const ru_field_rec BBH_TX_DEBUG_COUNTERS_GETNEXTNULL_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    BBH_TX_DEBUG_COUNTERS_GETNEXTNULL_RESERVED0_FIELD_MASK,
    0,
    BBH_TX_DEBUG_COUNTERS_GETNEXTNULL_RESERVED0_FIELD_WIDTH,
    BBH_TX_DEBUG_COUNTERS_GETNEXTNULL_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: BBH_TX_DEBUG_COUNTERS_FLUSHPKTS_FLSHPKTS
 ******************************************************************************/
const ru_field_rec BBH_TX_DEBUG_COUNTERS_FLUSHPKTS_FLSHPKTS_FIELD =
{
    "FLSHPKTS",
#if RU_INCLUDE_DESC
    "FLSH_PKTS",
    "This counter counts the number of flushed packets",
#endif
    BBH_TX_DEBUG_COUNTERS_FLUSHPKTS_FLSHPKTS_FIELD_MASK,
    0,
    BBH_TX_DEBUG_COUNTERS_FLUSHPKTS_FLSHPKTS_FIELD_WIDTH,
    BBH_TX_DEBUG_COUNTERS_FLUSHPKTS_FLSHPKTS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: BBH_TX_DEBUG_COUNTERS_FLUSHPKTS_RESERVED0
 ******************************************************************************/
const ru_field_rec BBH_TX_DEBUG_COUNTERS_FLUSHPKTS_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    BBH_TX_DEBUG_COUNTERS_FLUSHPKTS_RESERVED0_FIELD_MASK,
    0,
    BBH_TX_DEBUG_COUNTERS_FLUSHPKTS_RESERVED0_FIELD_WIDTH,
    BBH_TX_DEBUG_COUNTERS_FLUSHPKTS_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: BBH_TX_DEBUG_COUNTERS_LENERR_LENERR
 ******************************************************************************/
const ru_field_rec BBH_TX_DEBUG_COUNTERS_LENERR_LENERR_FIELD =
{
    "LENERR",
#if RU_INCLUDE_DESC
    "LEN_ERR",
    "This counter counts the number of times a length error occuered",
#endif
    BBH_TX_DEBUG_COUNTERS_LENERR_LENERR_FIELD_MASK,
    0,
    BBH_TX_DEBUG_COUNTERS_LENERR_LENERR_FIELD_WIDTH,
    BBH_TX_DEBUG_COUNTERS_LENERR_LENERR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: BBH_TX_DEBUG_COUNTERS_LENERR_RESERVED0
 ******************************************************************************/
const ru_field_rec BBH_TX_DEBUG_COUNTERS_LENERR_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    BBH_TX_DEBUG_COUNTERS_LENERR_RESERVED0_FIELD_MASK,
    0,
    BBH_TX_DEBUG_COUNTERS_LENERR_RESERVED0_FIELD_WIDTH,
    BBH_TX_DEBUG_COUNTERS_LENERR_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: BBH_TX_DEBUG_COUNTERS_AGGRLENERR_AGGRLENERR
 ******************************************************************************/
const ru_field_rec BBH_TX_DEBUG_COUNTERS_AGGRLENERR_AGGRLENERR_FIELD =
{
    "AGGRLENERR",
#if RU_INCLUDE_DESC
    "AGGR_LEN_ERR",
    "This counter counts the number of times an aggregation length error occuered",
#endif
    BBH_TX_DEBUG_COUNTERS_AGGRLENERR_AGGRLENERR_FIELD_MASK,
    0,
    BBH_TX_DEBUG_COUNTERS_AGGRLENERR_AGGRLENERR_FIELD_WIDTH,
    BBH_TX_DEBUG_COUNTERS_AGGRLENERR_AGGRLENERR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: BBH_TX_DEBUG_COUNTERS_AGGRLENERR_RESERVED0
 ******************************************************************************/
const ru_field_rec BBH_TX_DEBUG_COUNTERS_AGGRLENERR_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    BBH_TX_DEBUG_COUNTERS_AGGRLENERR_RESERVED0_FIELD_MASK,
    0,
    BBH_TX_DEBUG_COUNTERS_AGGRLENERR_RESERVED0_FIELD_WIDTH,
    BBH_TX_DEBUG_COUNTERS_AGGRLENERR_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: BBH_TX_DEBUG_COUNTERS_SRAMPKT_SRAMPKT
 ******************************************************************************/
const ru_field_rec BBH_TX_DEBUG_COUNTERS_SRAMPKT_SRAMPKT_FIELD =
{
    "SRAMPKT",
#if RU_INCLUDE_DESC
    "SRAM_PKT",
    "This counter counts the number of packets which were transmitted from the SRAM.",
#endif
    BBH_TX_DEBUG_COUNTERS_SRAMPKT_SRAMPKT_FIELD_MASK,
    0,
    BBH_TX_DEBUG_COUNTERS_SRAMPKT_SRAMPKT_FIELD_WIDTH,
    BBH_TX_DEBUG_COUNTERS_SRAMPKT_SRAMPKT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: BBH_TX_DEBUG_COUNTERS_DDRPKT_DDRPKT
 ******************************************************************************/
const ru_field_rec BBH_TX_DEBUG_COUNTERS_DDRPKT_DDRPKT_FIELD =
{
    "DDRPKT",
#if RU_INCLUDE_DESC
    "DDR_PKT",
    "This counter counts the number of packets which were transmitted from the DDR.",
#endif
    BBH_TX_DEBUG_COUNTERS_DDRPKT_DDRPKT_FIELD_MASK,
    0,
    BBH_TX_DEBUG_COUNTERS_DDRPKT_DDRPKT_FIELD_WIDTH,
    BBH_TX_DEBUG_COUNTERS_DDRPKT_DDRPKT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: BBH_TX_DEBUG_COUNTERS_SRAMBYTE_SRAMBYTE
 ******************************************************************************/
const ru_field_rec BBH_TX_DEBUG_COUNTERS_SRAMBYTE_SRAMBYTE_FIELD =
{
    "SRAMBYTE",
#if RU_INCLUDE_DESC
    "SRAM_BYTE",
    "This counter counts the number of transmitted bytes from the SRAM.",
#endif
    BBH_TX_DEBUG_COUNTERS_SRAMBYTE_SRAMBYTE_FIELD_MASK,
    0,
    BBH_TX_DEBUG_COUNTERS_SRAMBYTE_SRAMBYTE_FIELD_WIDTH,
    BBH_TX_DEBUG_COUNTERS_SRAMBYTE_SRAMBYTE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: BBH_TX_DEBUG_COUNTERS_DDRBYTE_DDRBYTE
 ******************************************************************************/
const ru_field_rec BBH_TX_DEBUG_COUNTERS_DDRBYTE_DDRBYTE_FIELD =
{
    "DDRBYTE",
#if RU_INCLUDE_DESC
    "DDR_BYTE",
    "This counter counts the number of transmitted bytes from the DDr.",
#endif
    BBH_TX_DEBUG_COUNTERS_DDRBYTE_DDRBYTE_FIELD_MASK,
    0,
    BBH_TX_DEBUG_COUNTERS_DDRBYTE_DDRBYTE_FIELD_WIDTH,
    BBH_TX_DEBUG_COUNTERS_DDRBYTE_DDRBYTE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: BBH_TX_DEBUG_COUNTERS_SWRDEN_PDSEL
 ******************************************************************************/
const ru_field_rec BBH_TX_DEBUG_COUNTERS_SWRDEN_PDSEL_FIELD =
{
    "PDSEL",
#if RU_INCLUDE_DESC
    "pd_array_sel",
    "rd from the PD FIFO",
#endif
    BBH_TX_DEBUG_COUNTERS_SWRDEN_PDSEL_FIELD_MASK,
    0,
    BBH_TX_DEBUG_COUNTERS_SWRDEN_PDSEL_FIELD_WIDTH,
    BBH_TX_DEBUG_COUNTERS_SWRDEN_PDSEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_DEBUG_COUNTERS_SWRDEN_PDVSEL
 ******************************************************************************/
const ru_field_rec BBH_TX_DEBUG_COUNTERS_SWRDEN_PDVSEL_FIELD =
{
    "PDVSEL",
#if RU_INCLUDE_DESC
    "pd_valid_array_sel",
    "rd from the PD valid array",
#endif
    BBH_TX_DEBUG_COUNTERS_SWRDEN_PDVSEL_FIELD_MASK,
    0,
    BBH_TX_DEBUG_COUNTERS_SWRDEN_PDVSEL_FIELD_WIDTH,
    BBH_TX_DEBUG_COUNTERS_SWRDEN_PDVSEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_DEBUG_COUNTERS_SWRDEN_PDEMPTYSEL
 ******************************************************************************/
const ru_field_rec BBH_TX_DEBUG_COUNTERS_SWRDEN_PDEMPTYSEL_FIELD =
{
    "PDEMPTYSEL",
#if RU_INCLUDE_DESC
    "pd_empty_array_sel",
    "rd from the PD empty array",
#endif
    BBH_TX_DEBUG_COUNTERS_SWRDEN_PDEMPTYSEL_FIELD_MASK,
    0,
    BBH_TX_DEBUG_COUNTERS_SWRDEN_PDEMPTYSEL_FIELD_WIDTH,
    BBH_TX_DEBUG_COUNTERS_SWRDEN_PDEMPTYSEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_DEBUG_COUNTERS_SWRDEN_PDFULLSEL
 ******************************************************************************/
const ru_field_rec BBH_TX_DEBUG_COUNTERS_SWRDEN_PDFULLSEL_FIELD =
{
    "PDFULLSEL",
#if RU_INCLUDE_DESC
    "pd_full_array_sel",
    "rd from the PD Full array",
#endif
    BBH_TX_DEBUG_COUNTERS_SWRDEN_PDFULLSEL_FIELD_MASK,
    0,
    BBH_TX_DEBUG_COUNTERS_SWRDEN_PDFULLSEL_FIELD_WIDTH,
    BBH_TX_DEBUG_COUNTERS_SWRDEN_PDFULLSEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_DEBUG_COUNTERS_SWRDEN_PDBEMPTYSEL
 ******************************************************************************/
const ru_field_rec BBH_TX_DEBUG_COUNTERS_SWRDEN_PDBEMPTYSEL_FIELD =
{
    "PDBEMPTYSEL",
#if RU_INCLUDE_DESC
    "pd_below_empty_array_sel",
    "rd from the PD beliow empty array",
#endif
    BBH_TX_DEBUG_COUNTERS_SWRDEN_PDBEMPTYSEL_FIELD_MASK,
    0,
    BBH_TX_DEBUG_COUNTERS_SWRDEN_PDBEMPTYSEL_FIELD_WIDTH,
    BBH_TX_DEBUG_COUNTERS_SWRDEN_PDBEMPTYSEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_DEBUG_COUNTERS_SWRDEN_PDFFWKPSEL
 ******************************************************************************/
const ru_field_rec BBH_TX_DEBUG_COUNTERS_SWRDEN_PDFFWKPSEL_FIELD =
{
    "PDFFWKPSEL",
#if RU_INCLUDE_DESC
    "pd_full_for_wakeup_array_sel",
    "rd from the PD full for wakeup empty array",
#endif
    BBH_TX_DEBUG_COUNTERS_SWRDEN_PDFFWKPSEL_FIELD_MASK,
    0,
    BBH_TX_DEBUG_COUNTERS_SWRDEN_PDFFWKPSEL_FIELD_WIDTH,
    BBH_TX_DEBUG_COUNTERS_SWRDEN_PDFFWKPSEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_DEBUG_COUNTERS_SWRDEN_FBNSEL
 ******************************************************************************/
const ru_field_rec BBH_TX_DEBUG_COUNTERS_SWRDEN_FBNSEL_FIELD =
{
    "FBNSEL",
#if RU_INCLUDE_DESC
    "first_BN_array_sel",
    "rd from the first BN array",
#endif
    BBH_TX_DEBUG_COUNTERS_SWRDEN_FBNSEL_FIELD_MASK,
    0,
    BBH_TX_DEBUG_COUNTERS_SWRDEN_FBNSEL_FIELD_WIDTH,
    BBH_TX_DEBUG_COUNTERS_SWRDEN_FBNSEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_DEBUG_COUNTERS_SWRDEN_FBNVSEL
 ******************************************************************************/
const ru_field_rec BBH_TX_DEBUG_COUNTERS_SWRDEN_FBNVSEL_FIELD =
{
    "FBNVSEL",
#if RU_INCLUDE_DESC
    "first_BN_valid_array_sel",
    "rd from the first BN valid array",
#endif
    BBH_TX_DEBUG_COUNTERS_SWRDEN_FBNVSEL_FIELD_MASK,
    0,
    BBH_TX_DEBUG_COUNTERS_SWRDEN_FBNVSEL_FIELD_WIDTH,
    BBH_TX_DEBUG_COUNTERS_SWRDEN_FBNVSEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_DEBUG_COUNTERS_SWRDEN_FBNEMPTYSEL
 ******************************************************************************/
const ru_field_rec BBH_TX_DEBUG_COUNTERS_SWRDEN_FBNEMPTYSEL_FIELD =
{
    "FBNEMPTYSEL",
#if RU_INCLUDE_DESC
    "first_BN_empty_array_sel",
    "rd from the first BN empty array",
#endif
    BBH_TX_DEBUG_COUNTERS_SWRDEN_FBNEMPTYSEL_FIELD_MASK,
    0,
    BBH_TX_DEBUG_COUNTERS_SWRDEN_FBNEMPTYSEL_FIELD_WIDTH,
    BBH_TX_DEBUG_COUNTERS_SWRDEN_FBNEMPTYSEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_DEBUG_COUNTERS_SWRDEN_FBNFULLSEL
 ******************************************************************************/
const ru_field_rec BBH_TX_DEBUG_COUNTERS_SWRDEN_FBNFULLSEL_FIELD =
{
    "FBNFULLSEL",
#if RU_INCLUDE_DESC
    "first_BN_full_array_sel",
    "rd from the first BN full array",
#endif
    BBH_TX_DEBUG_COUNTERS_SWRDEN_FBNFULLSEL_FIELD_MASK,
    0,
    BBH_TX_DEBUG_COUNTERS_SWRDEN_FBNFULLSEL_FIELD_WIDTH,
    BBH_TX_DEBUG_COUNTERS_SWRDEN_FBNFULLSEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_DEBUG_COUNTERS_SWRDEN_GETNEXTSEL
 ******************************************************************************/
const ru_field_rec BBH_TX_DEBUG_COUNTERS_SWRDEN_GETNEXTSEL_FIELD =
{
    "GETNEXTSEL",
#if RU_INCLUDE_DESC
    "get_next_array_sel",
    "rd from the first Get Next array",
#endif
    BBH_TX_DEBUG_COUNTERS_SWRDEN_GETNEXTSEL_FIELD_MASK,
    0,
    BBH_TX_DEBUG_COUNTERS_SWRDEN_GETNEXTSEL_FIELD_WIDTH,
    BBH_TX_DEBUG_COUNTERS_SWRDEN_GETNEXTSEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_DEBUG_COUNTERS_SWRDEN_GETNEXTVSEL
 ******************************************************************************/
const ru_field_rec BBH_TX_DEBUG_COUNTERS_SWRDEN_GETNEXTVSEL_FIELD =
{
    "GETNEXTVSEL",
#if RU_INCLUDE_DESC
    "get_next_valid_array_sel",
    "rd from the get_next valid array",
#endif
    BBH_TX_DEBUG_COUNTERS_SWRDEN_GETNEXTVSEL_FIELD_MASK,
    0,
    BBH_TX_DEBUG_COUNTERS_SWRDEN_GETNEXTVSEL_FIELD_WIDTH,
    BBH_TX_DEBUG_COUNTERS_SWRDEN_GETNEXTVSEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_DEBUG_COUNTERS_SWRDEN_GETNEXTEMPTYSEL
 ******************************************************************************/
const ru_field_rec BBH_TX_DEBUG_COUNTERS_SWRDEN_GETNEXTEMPTYSEL_FIELD =
{
    "GETNEXTEMPTYSEL",
#if RU_INCLUDE_DESC
    "get_next_empty_array_sel",
    "rd from the get next empty array",
#endif
    BBH_TX_DEBUG_COUNTERS_SWRDEN_GETNEXTEMPTYSEL_FIELD_MASK,
    0,
    BBH_TX_DEBUG_COUNTERS_SWRDEN_GETNEXTEMPTYSEL_FIELD_WIDTH,
    BBH_TX_DEBUG_COUNTERS_SWRDEN_GETNEXTEMPTYSEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_DEBUG_COUNTERS_SWRDEN_GETNEXTFULLSEL
 ******************************************************************************/
const ru_field_rec BBH_TX_DEBUG_COUNTERS_SWRDEN_GETNEXTFULLSEL_FIELD =
{
    "GETNEXTFULLSEL",
#if RU_INCLUDE_DESC
    "get_next_full_array_sel",
    "rd from the get next full array",
#endif
    BBH_TX_DEBUG_COUNTERS_SWRDEN_GETNEXTFULLSEL_FIELD_MASK,
    0,
    BBH_TX_DEBUG_COUNTERS_SWRDEN_GETNEXTFULLSEL_FIELD_WIDTH,
    BBH_TX_DEBUG_COUNTERS_SWRDEN_GETNEXTFULLSEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_DEBUG_COUNTERS_SWRDEN_GPNCNTXTSEL
 ******************************************************************************/
const ru_field_rec BBH_TX_DEBUG_COUNTERS_SWRDEN_GPNCNTXTSEL_FIELD =
{
    "GPNCNTXTSEL",
#if RU_INCLUDE_DESC
    "gpon_context_array_sel",
    "rd from the gpon context array",
#endif
    BBH_TX_DEBUG_COUNTERS_SWRDEN_GPNCNTXTSEL_FIELD_MASK,
    0,
    BBH_TX_DEBUG_COUNTERS_SWRDEN_GPNCNTXTSEL_FIELD_WIDTH,
    BBH_TX_DEBUG_COUNTERS_SWRDEN_GPNCNTXTSEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_DEBUG_COUNTERS_SWRDEN_BPMSEL
 ******************************************************************************/
const ru_field_rec BBH_TX_DEBUG_COUNTERS_SWRDEN_BPMSEL_FIELD =
{
    "BPMSEL",
#if RU_INCLUDE_DESC
    "BPM_FIFO_sel",
    "rd from the BPM FIFO",
#endif
    BBH_TX_DEBUG_COUNTERS_SWRDEN_BPMSEL_FIELD_MASK,
    0,
    BBH_TX_DEBUG_COUNTERS_SWRDEN_BPMSEL_FIELD_WIDTH,
    BBH_TX_DEBUG_COUNTERS_SWRDEN_BPMSEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_DEBUG_COUNTERS_SWRDEN_BPMFSEL
 ******************************************************************************/
const ru_field_rec BBH_TX_DEBUG_COUNTERS_SWRDEN_BPMFSEL_FIELD =
{
    "BPMFSEL",
#if RU_INCLUDE_DESC
    "BPM_FLUSH_FIFO_sel",
    "rd from the BPM FLUSH FIFO",
#endif
    BBH_TX_DEBUG_COUNTERS_SWRDEN_BPMFSEL_FIELD_MASK,
    0,
    BBH_TX_DEBUG_COUNTERS_SWRDEN_BPMFSEL_FIELD_WIDTH,
    BBH_TX_DEBUG_COUNTERS_SWRDEN_BPMFSEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_DEBUG_COUNTERS_SWRDEN_SBPMSEL
 ******************************************************************************/
const ru_field_rec BBH_TX_DEBUG_COUNTERS_SWRDEN_SBPMSEL_FIELD =
{
    "SBPMSEL",
#if RU_INCLUDE_DESC
    "SBPM_FIFO_sel",
    "rd from the SBPM FIFO",
#endif
    BBH_TX_DEBUG_COUNTERS_SWRDEN_SBPMSEL_FIELD_MASK,
    0,
    BBH_TX_DEBUG_COUNTERS_SWRDEN_SBPMSEL_FIELD_WIDTH,
    BBH_TX_DEBUG_COUNTERS_SWRDEN_SBPMSEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_DEBUG_COUNTERS_SWRDEN_SBPMFSEL
 ******************************************************************************/
const ru_field_rec BBH_TX_DEBUG_COUNTERS_SWRDEN_SBPMFSEL_FIELD =
{
    "SBPMFSEL",
#if RU_INCLUDE_DESC
    "SBPM_FLUSH_FIFO_sel",
    "rd from the SBPM FLUSH FIFO",
#endif
    BBH_TX_DEBUG_COUNTERS_SWRDEN_SBPMFSEL_FIELD_MASK,
    0,
    BBH_TX_DEBUG_COUNTERS_SWRDEN_SBPMFSEL_FIELD_WIDTH,
    BBH_TX_DEBUG_COUNTERS_SWRDEN_SBPMFSEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_DEBUG_COUNTERS_SWRDEN_STSSEL
 ******************************************************************************/
const ru_field_rec BBH_TX_DEBUG_COUNTERS_SWRDEN_STSSEL_FIELD =
{
    "STSSEL",
#if RU_INCLUDE_DESC
    "sts_array_sel",
    "rd from the STS FIFO",
#endif
    BBH_TX_DEBUG_COUNTERS_SWRDEN_STSSEL_FIELD_MASK,
    0,
    BBH_TX_DEBUG_COUNTERS_SWRDEN_STSSEL_FIELD_WIDTH,
    BBH_TX_DEBUG_COUNTERS_SWRDEN_STSSEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_DEBUG_COUNTERS_SWRDEN_STSVSEL
 ******************************************************************************/
const ru_field_rec BBH_TX_DEBUG_COUNTERS_SWRDEN_STSVSEL_FIELD =
{
    "STSVSEL",
#if RU_INCLUDE_DESC
    "sts_valid_array_sel",
    "rd from the STS valid array",
#endif
    BBH_TX_DEBUG_COUNTERS_SWRDEN_STSVSEL_FIELD_MASK,
    0,
    BBH_TX_DEBUG_COUNTERS_SWRDEN_STSVSEL_FIELD_WIDTH,
    BBH_TX_DEBUG_COUNTERS_SWRDEN_STSVSEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_DEBUG_COUNTERS_SWRDEN_STSEMPTYSEL
 ******************************************************************************/
const ru_field_rec BBH_TX_DEBUG_COUNTERS_SWRDEN_STSEMPTYSEL_FIELD =
{
    "STSEMPTYSEL",
#if RU_INCLUDE_DESC
    "sts_empty_array_sel",
    "rd from the STS empty array",
#endif
    BBH_TX_DEBUG_COUNTERS_SWRDEN_STSEMPTYSEL_FIELD_MASK,
    0,
    BBH_TX_DEBUG_COUNTERS_SWRDEN_STSEMPTYSEL_FIELD_WIDTH,
    BBH_TX_DEBUG_COUNTERS_SWRDEN_STSEMPTYSEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_DEBUG_COUNTERS_SWRDEN_STSFULLSEL
 ******************************************************************************/
const ru_field_rec BBH_TX_DEBUG_COUNTERS_SWRDEN_STSFULLSEL_FIELD =
{
    "STSFULLSEL",
#if RU_INCLUDE_DESC
    "sts_full_array_sel",
    "rd from the STS Full array",
#endif
    BBH_TX_DEBUG_COUNTERS_SWRDEN_STSFULLSEL_FIELD_MASK,
    0,
    BBH_TX_DEBUG_COUNTERS_SWRDEN_STSFULLSEL_FIELD_WIDTH,
    BBH_TX_DEBUG_COUNTERS_SWRDEN_STSFULLSEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_DEBUG_COUNTERS_SWRDEN_STSBEMPTYSEL
 ******************************************************************************/
const ru_field_rec BBH_TX_DEBUG_COUNTERS_SWRDEN_STSBEMPTYSEL_FIELD =
{
    "STSBEMPTYSEL",
#if RU_INCLUDE_DESC
    "sts_below_empty_array_sel",
    "rd from the STS beliow empty array",
#endif
    BBH_TX_DEBUG_COUNTERS_SWRDEN_STSBEMPTYSEL_FIELD_MASK,
    0,
    BBH_TX_DEBUG_COUNTERS_SWRDEN_STSBEMPTYSEL_FIELD_WIDTH,
    BBH_TX_DEBUG_COUNTERS_SWRDEN_STSBEMPTYSEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_DEBUG_COUNTERS_SWRDEN_STSFFWKPSEL
 ******************************************************************************/
const ru_field_rec BBH_TX_DEBUG_COUNTERS_SWRDEN_STSFFWKPSEL_FIELD =
{
    "STSFFWKPSEL",
#if RU_INCLUDE_DESC
    "sts_full_for_wakeup_array_sel",
    "rd from the STS full for wakeup empty array",
#endif
    BBH_TX_DEBUG_COUNTERS_SWRDEN_STSFFWKPSEL_FIELD_MASK,
    0,
    BBH_TX_DEBUG_COUNTERS_SWRDEN_STSFFWKPSEL_FIELD_WIDTH,
    BBH_TX_DEBUG_COUNTERS_SWRDEN_STSFFWKPSEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_DEBUG_COUNTERS_SWRDEN_MSGSEL
 ******************************************************************************/
const ru_field_rec BBH_TX_DEBUG_COUNTERS_SWRDEN_MSGSEL_FIELD =
{
    "MSGSEL",
#if RU_INCLUDE_DESC
    "msg_array_sel",
    "rd from the MSG FIFO",
#endif
    BBH_TX_DEBUG_COUNTERS_SWRDEN_MSGSEL_FIELD_MASK,
    0,
    BBH_TX_DEBUG_COUNTERS_SWRDEN_MSGSEL_FIELD_WIDTH,
    BBH_TX_DEBUG_COUNTERS_SWRDEN_MSGSEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_DEBUG_COUNTERS_SWRDEN_MSGVSEL
 ******************************************************************************/
const ru_field_rec BBH_TX_DEBUG_COUNTERS_SWRDEN_MSGVSEL_FIELD =
{
    "MSGVSEL",
#if RU_INCLUDE_DESC
    "msg_valid_array_sel",
    "rd from the msg valid array",
#endif
    BBH_TX_DEBUG_COUNTERS_SWRDEN_MSGVSEL_FIELD_MASK,
    0,
    BBH_TX_DEBUG_COUNTERS_SWRDEN_MSGVSEL_FIELD_WIDTH,
    BBH_TX_DEBUG_COUNTERS_SWRDEN_MSGVSEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_DEBUG_COUNTERS_SWRDEN_EPNREQSEL
 ******************************************************************************/
const ru_field_rec BBH_TX_DEBUG_COUNTERS_SWRDEN_EPNREQSEL_FIELD =
{
    "EPNREQSEL",
#if RU_INCLUDE_DESC
    "epon_request_FIFO_sel",
    "rd from the epon request FIFO",
#endif
    BBH_TX_DEBUG_COUNTERS_SWRDEN_EPNREQSEL_FIELD_MASK,
    0,
    BBH_TX_DEBUG_COUNTERS_SWRDEN_EPNREQSEL_FIELD_WIDTH,
    BBH_TX_DEBUG_COUNTERS_SWRDEN_EPNREQSEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_DEBUG_COUNTERS_SWRDEN_DATASEL
 ******************************************************************************/
const ru_field_rec BBH_TX_DEBUG_COUNTERS_SWRDEN_DATASEL_FIELD =
{
    "DATASEL",
#if RU_INCLUDE_DESC
    "DATA_FIFO_sel",
    "rd from the DATA FIFO (SRAM and DDR)",
#endif
    BBH_TX_DEBUG_COUNTERS_SWRDEN_DATASEL_FIELD_MASK,
    0,
    BBH_TX_DEBUG_COUNTERS_SWRDEN_DATASEL_FIELD_WIDTH,
    BBH_TX_DEBUG_COUNTERS_SWRDEN_DATASEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_DEBUG_COUNTERS_SWRDEN_REORDERSEL
 ******************************************************************************/
const ru_field_rec BBH_TX_DEBUG_COUNTERS_SWRDEN_REORDERSEL_FIELD =
{
    "REORDERSEL",
#if RU_INCLUDE_DESC
    "reorder_FIFO_sel",
    "rd from the reorder FIFO",
#endif
    BBH_TX_DEBUG_COUNTERS_SWRDEN_REORDERSEL_FIELD_MASK,
    0,
    BBH_TX_DEBUG_COUNTERS_SWRDEN_REORDERSEL_FIELD_WIDTH,
    BBH_TX_DEBUG_COUNTERS_SWRDEN_REORDERSEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_DEBUG_COUNTERS_SWRDEN_TSINFOSEL
 ******************************************************************************/
const ru_field_rec BBH_TX_DEBUG_COUNTERS_SWRDEN_TSINFOSEL_FIELD =
{
    "TSINFOSEL",
#if RU_INCLUDE_DESC
    "Timestamp_info_FIFO_sel",
    "rd from the Timestamp Info FIFO",
#endif
    BBH_TX_DEBUG_COUNTERS_SWRDEN_TSINFOSEL_FIELD_MASK,
    0,
    BBH_TX_DEBUG_COUNTERS_SWRDEN_TSINFOSEL_FIELD_WIDTH,
    BBH_TX_DEBUG_COUNTERS_SWRDEN_TSINFOSEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_DEBUG_COUNTERS_SWRDEN_MACTXSEL
 ******************************************************************************/
const ru_field_rec BBH_TX_DEBUG_COUNTERS_SWRDEN_MACTXSEL_FIELD =
{
    "MACTXSEL",
#if RU_INCLUDE_DESC
    "MAC_TX_FIFO_sel",
    "rd from the MAC TX FIFO."
    "",
#endif
    BBH_TX_DEBUG_COUNTERS_SWRDEN_MACTXSEL_FIELD_MASK,
    0,
    BBH_TX_DEBUG_COUNTERS_SWRDEN_MACTXSEL_FIELD_WIDTH,
    BBH_TX_DEBUG_COUNTERS_SWRDEN_MACTXSEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_DEBUG_COUNTERS_SWRDADDR_RDADDR
 ******************************************************************************/
const ru_field_rec BBH_TX_DEBUG_COUNTERS_SWRDADDR_RDADDR_FIELD =
{
    "RDADDR",
#if RU_INCLUDE_DESC
    "sw_rd_address",
    "The address inside the array the sw wants to read",
#endif
    BBH_TX_DEBUG_COUNTERS_SWRDADDR_RDADDR_FIELD_MASK,
    0,
    BBH_TX_DEBUG_COUNTERS_SWRDADDR_RDADDR_FIELD_WIDTH,
    BBH_TX_DEBUG_COUNTERS_SWRDADDR_RDADDR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_DEBUG_COUNTERS_SWRDADDR_RESERVED0
 ******************************************************************************/
const ru_field_rec BBH_TX_DEBUG_COUNTERS_SWRDADDR_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    BBH_TX_DEBUG_COUNTERS_SWRDADDR_RESERVED0_FIELD_MASK,
    0,
    BBH_TX_DEBUG_COUNTERS_SWRDADDR_RESERVED0_FIELD_WIDTH,
    BBH_TX_DEBUG_COUNTERS_SWRDADDR_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_TX_DEBUG_COUNTERS_SWRDDATA_DATA
 ******************************************************************************/
const ru_field_rec BBH_TX_DEBUG_COUNTERS_SWRDDATA_DATA_FIELD =
{
    "DATA",
#if RU_INCLUDE_DESC
    "data",
    "data",
#endif
    BBH_TX_DEBUG_COUNTERS_SWRDDATA_DATA_FIELD_MASK,
    0,
    BBH_TX_DEBUG_COUNTERS_SWRDDATA_DATA_FIELD_WIDTH,
    BBH_TX_DEBUG_COUNTERS_SWRDDATA_DATA_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: BBH_TX_DEBUG_COUNTERS_UNIFIEDPKT_DDRBYTE
 ******************************************************************************/
const ru_field_rec BBH_TX_DEBUG_COUNTERS_UNIFIEDPKT_DDRBYTE_FIELD =
{
    "DDRBYTE",
#if RU_INCLUDE_DESC
    "DDR_BYTE",
    "This counter counts the number of transmitted bytes from the DDr.",
#endif
    BBH_TX_DEBUG_COUNTERS_UNIFIEDPKT_DDRBYTE_FIELD_MASK,
    0,
    BBH_TX_DEBUG_COUNTERS_UNIFIEDPKT_DDRBYTE_FIELD_WIDTH,
    BBH_TX_DEBUG_COUNTERS_UNIFIEDPKT_DDRBYTE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: BBH_TX_DEBUG_COUNTERS_UNIFIEDBYTE_DDRBYTE
 ******************************************************************************/
const ru_field_rec BBH_TX_DEBUG_COUNTERS_UNIFIEDBYTE_DDRBYTE_FIELD =
{
    "DDRBYTE",
#if RU_INCLUDE_DESC
    "DDR_BYTE",
    "This counter counts the number of transmitted bytes from the DDr.",
#endif
    BBH_TX_DEBUG_COUNTERS_UNIFIEDBYTE_DDRBYTE_FIELD_MASK,
    0,
    BBH_TX_DEBUG_COUNTERS_UNIFIEDBYTE_DDRBYTE_FIELD_WIDTH,
    BBH_TX_DEBUG_COUNTERS_UNIFIEDBYTE_DDRBYTE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: BBH_TX_DEBUG_COUNTERS_DBGOUTREG_DBGVEC
 ******************************************************************************/
const ru_field_rec BBH_TX_DEBUG_COUNTERS_DBGOUTREG_DBGVEC_FIELD =
{
    "DBGVEC",
#if RU_INCLUDE_DESC
    "Debug_vector",
    "Selected debug vector.",
#endif
    BBH_TX_DEBUG_COUNTERS_DBGOUTREG_DBGVEC_FIELD_MASK,
    0,
    BBH_TX_DEBUG_COUNTERS_DBGOUTREG_DBGVEC_FIELD_WIDTH,
    BBH_TX_DEBUG_COUNTERS_DBGOUTREG_DBGVEC_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

#endif /* RU_INCLUDE_FIELD_DB */

/******************************************************************************
 * Register: BBH_TX_COMMON_CONFIGURATIONS_MACTYPE
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_COMMON_CONFIGURATIONS_MACTYPE_FIELDS[] =
{
    &BBH_TX_COMMON_CONFIGURATIONS_MACTYPE_TYPE_FIELD,
    &BBH_TX_COMMON_CONFIGURATIONS_MACTYPE_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    975,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    BBH_TX_COMMON_CONFIGURATIONS_MACTYPE_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: BBH_TX_COMMON_CONFIGURATIONS_BBCFG_1_TX
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_COMMON_CONFIGURATIONS_BBCFG_1_TX_FIELDS[] =
{
    &BBH_TX_COMMON_CONFIGURATIONS_BBCFG_1_TX_DMASRC_FIELD,
    &BBH_TX_COMMON_CONFIGURATIONS_BBCFG_1_TX_RESERVED0_FIELD,
    &BBH_TX_COMMON_CONFIGURATIONS_BBCFG_1_TX_SDMASRC_FIELD,
    &BBH_TX_COMMON_CONFIGURATIONS_BBCFG_1_TX_RESERVED1_FIELD,
    &BBH_TX_COMMON_CONFIGURATIONS_BBCFG_1_TX_SBPMSRC_FIELD,
    &BBH_TX_COMMON_CONFIGURATIONS_BBCFG_1_TX_RESERVED2_FIELD,
    &BBH_TX_COMMON_CONFIGURATIONS_BBCFG_1_TX_FPMSRC_FIELD,
    &BBH_TX_COMMON_CONFIGURATIONS_BBCFG_1_TX_RESERVED3_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    976,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    8,
    BBH_TX_COMMON_CONFIGURATIONS_BBCFG_1_TX_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: BBH_TX_COMMON_CONFIGURATIONS_BBCFG_2_TX
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_COMMON_CONFIGURATIONS_BBCFG_2_TX_FIELDS[] =
{
    &BBH_TX_COMMON_CONFIGURATIONS_BBCFG_2_TX_PDRNR0SRC_FIELD,
    &BBH_TX_COMMON_CONFIGURATIONS_BBCFG_2_TX_RESERVED0_FIELD,
    &BBH_TX_COMMON_CONFIGURATIONS_BBCFG_2_TX_PDRNR1SRC_FIELD,
    &BBH_TX_COMMON_CONFIGURATIONS_BBCFG_2_TX_RESERVED1_FIELD,
    &BBH_TX_COMMON_CONFIGURATIONS_BBCFG_2_TX_STSRNRSRC_FIELD,
    &BBH_TX_COMMON_CONFIGURATIONS_BBCFG_2_TX_RESERVED2_FIELD,
    &BBH_TX_COMMON_CONFIGURATIONS_BBCFG_2_TX_MSGRNRSRC_FIELD,
    &BBH_TX_COMMON_CONFIGURATIONS_BBCFG_2_TX_RESERVED3_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    977,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    8,
    BBH_TX_COMMON_CONFIGURATIONS_BBCFG_2_TX_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: BBH_TX_COMMON_CONFIGURATIONS_DDRCFG_TX
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_COMMON_CONFIGURATIONS_DDRCFG_TX_FIELDS[] =
{
    &BBH_TX_COMMON_CONFIGURATIONS_DDRCFG_TX_BUFSIZE_FIELD,
    &BBH_TX_COMMON_CONFIGURATIONS_DDRCFG_TX_BYTERESUL_FIELD,
    &BBH_TX_COMMON_CONFIGURATIONS_DDRCFG_TX_DDRTXOFFSET_FIELD,
    &BBH_TX_COMMON_CONFIGURATIONS_DDRCFG_TX_RESERVED0_FIELD,
    &BBH_TX_COMMON_CONFIGURATIONS_DDRCFG_TX_HNSIZE0_FIELD,
    &BBH_TX_COMMON_CONFIGURATIONS_DDRCFG_TX_RESERVED1_FIELD,
    &BBH_TX_COMMON_CONFIGURATIONS_DDRCFG_TX_HNSIZE1_FIELD,
    &BBH_TX_COMMON_CONFIGURATIONS_DDRCFG_TX_RESERVED2_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    978,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    8,
    BBH_TX_COMMON_CONFIGURATIONS_DDRCFG_TX_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: BBH_TX_COMMON_CONFIGURATIONS_RNRCFG_1
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_COMMON_CONFIGURATIONS_RNRCFG_1_FIELDS[] =
{
    &BBH_TX_COMMON_CONFIGURATIONS_RNRCFG_1_TCONTADDR_FIELD,
    &BBH_TX_COMMON_CONFIGURATIONS_RNRCFG_1_SKBADDR_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    979,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    BBH_TX_COMMON_CONFIGURATIONS_RNRCFG_1_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: BBH_TX_COMMON_CONFIGURATIONS_RNRCFG_2
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_COMMON_CONFIGURATIONS_RNRCFG_2_FIELDS[] =
{
    &BBH_TX_COMMON_CONFIGURATIONS_RNRCFG_2_PTRADDR_FIELD,
    &BBH_TX_COMMON_CONFIGURATIONS_RNRCFG_2_TASK_FIELD,
    &BBH_TX_COMMON_CONFIGURATIONS_RNRCFG_2_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    980,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    BBH_TX_COMMON_CONFIGURATIONS_RNRCFG_2_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: BBH_TX_COMMON_CONFIGURATIONS_DMACFG_TX
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_COMMON_CONFIGURATIONS_DMACFG_TX_FIELDS[] =
{
    &BBH_TX_COMMON_CONFIGURATIONS_DMACFG_TX_DESCBASE_FIELD,
    &BBH_TX_COMMON_CONFIGURATIONS_DMACFG_TX_DESCSIZE_FIELD,
    &BBH_TX_COMMON_CONFIGURATIONS_DMACFG_TX_RESERVED0_FIELD,
    &BBH_TX_COMMON_CONFIGURATIONS_DMACFG_TX_MAXREQ_FIELD,
    &BBH_TX_COMMON_CONFIGURATIONS_DMACFG_TX_RESERVED1_FIELD,
    &BBH_TX_COMMON_CONFIGURATIONS_DMACFG_TX_EPNURGNT_FIELD,
    &BBH_TX_COMMON_CONFIGURATIONS_DMACFG_TX_RESERVED2_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    981,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    7,
    BBH_TX_COMMON_CONFIGURATIONS_DMACFG_TX_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: BBH_TX_COMMON_CONFIGURATIONS_SDMACFG_TX
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_COMMON_CONFIGURATIONS_SDMACFG_TX_FIELDS[] =
{
    &BBH_TX_COMMON_CONFIGURATIONS_SDMACFG_TX_DESCBASE_FIELD,
    &BBH_TX_COMMON_CONFIGURATIONS_SDMACFG_TX_DESCSIZE_FIELD,
    &BBH_TX_COMMON_CONFIGURATIONS_SDMACFG_TX_RESERVED0_FIELD,
    &BBH_TX_COMMON_CONFIGURATIONS_SDMACFG_TX_MAXREQ_FIELD,
    &BBH_TX_COMMON_CONFIGURATIONS_SDMACFG_TX_RESERVED1_FIELD,
    &BBH_TX_COMMON_CONFIGURATIONS_SDMACFG_TX_EPNURGNT_FIELD,
    &BBH_TX_COMMON_CONFIGURATIONS_SDMACFG_TX_RESERVED2_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    982,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    7,
    BBH_TX_COMMON_CONFIGURATIONS_SDMACFG_TX_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: BBH_TX_COMMON_CONFIGURATIONS_SBPMCFG
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_COMMON_CONFIGURATIONS_SBPMCFG_FIELDS[] =
{
    &BBH_TX_COMMON_CONFIGURATIONS_SBPMCFG_FREENOCNTXT_FIELD,
    &BBH_TX_COMMON_CONFIGURATIONS_SBPMCFG_SPECIALFREE_FIELD,
    &BBH_TX_COMMON_CONFIGURATIONS_SBPMCFG_RESERVED0_FIELD,
    &BBH_TX_COMMON_CONFIGURATIONS_SBPMCFG_MAXGN_FIELD,
    &BBH_TX_COMMON_CONFIGURATIONS_SBPMCFG_RESERVED1_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    983,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    5,
    BBH_TX_COMMON_CONFIGURATIONS_SBPMCFG_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: BBH_TX_COMMON_CONFIGURATIONS_DDRTMBASEL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_COMMON_CONFIGURATIONS_DDRTMBASEL_FIELDS[] =
{
    &BBH_TX_COMMON_CONFIGURATIONS_DDRTMBASEL_DDRTMBASE_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    984,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_TX_COMMON_CONFIGURATIONS_DDRTMBASEL_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: BBH_TX_COMMON_CONFIGURATIONS_DDRTMBASEH
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_COMMON_CONFIGURATIONS_DDRTMBASEH_FIELDS[] =
{
    &BBH_TX_COMMON_CONFIGURATIONS_DDRTMBASEH_DDRTMBASE_FIELD,
    &BBH_TX_COMMON_CONFIGURATIONS_DDRTMBASEH_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    985,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    BBH_TX_COMMON_CONFIGURATIONS_DDRTMBASEH_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: BBH_TX_COMMON_CONFIGURATIONS_DFIFOCTRL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_COMMON_CONFIGURATIONS_DFIFOCTRL_FIELDS[] =
{
    &BBH_TX_COMMON_CONFIGURATIONS_DFIFOCTRL_PSRAMSIZE_FIELD,
    &BBH_TX_COMMON_CONFIGURATIONS_DFIFOCTRL_DDRSIZE_FIELD,
    &BBH_TX_COMMON_CONFIGURATIONS_DFIFOCTRL_PSRAMBASE_FIELD,
    &BBH_TX_COMMON_CONFIGURATIONS_DFIFOCTRL_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    986,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    BBH_TX_COMMON_CONFIGURATIONS_DFIFOCTRL_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: BBH_TX_COMMON_CONFIGURATIONS_ARB_CFG
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_COMMON_CONFIGURATIONS_ARB_CFG_FIELDS[] =
{
    &BBH_TX_COMMON_CONFIGURATIONS_ARB_CFG_HIGHTRXQ_FIELD,
    &BBH_TX_COMMON_CONFIGURATIONS_ARB_CFG_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    987,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    BBH_TX_COMMON_CONFIGURATIONS_ARB_CFG_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: BBH_TX_COMMON_CONFIGURATIONS_BBROUTE
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_COMMON_CONFIGURATIONS_BBROUTE_FIELDS[] =
{
    &BBH_TX_COMMON_CONFIGURATIONS_BBROUTE_ROUTE_FIELD,
    &BBH_TX_COMMON_CONFIGURATIONS_BBROUTE_DEST_FIELD,
    &BBH_TX_COMMON_CONFIGURATIONS_BBROUTE_EN_FIELD,
    &BBH_TX_COMMON_CONFIGURATIONS_BBROUTE_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    988,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    BBH_TX_COMMON_CONFIGURATIONS_BBROUTE_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: BBH_TX_COMMON_CONFIGURATIONS_Q2RNR
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_COMMON_CONFIGURATIONS_Q2RNR_FIELDS[] =
{
    &BBH_TX_COMMON_CONFIGURATIONS_Q2RNR_Q0_FIELD,
    &BBH_TX_COMMON_CONFIGURATIONS_Q2RNR_Q1_FIELD,
    &BBH_TX_COMMON_CONFIGURATIONS_Q2RNR_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec BBH_TX_COMMON_CONFIGURATIONS_Q2RNR_REG = 
{
    "COMMON_CONFIGURATIONS_Q2RNR",
#if RU_INCLUDE_DESC
    "Q_TO_RNR %i Register",
    "configuration which queue is managed by each of the two runners."
    ""
    "Each register in this array configures 2 queues.",
#endif
    BBH_TX_COMMON_CONFIGURATIONS_Q2RNR_REG_OFFSET,
    BBH_TX_COMMON_CONFIGURATIONS_Q2RNR_REG_RAM_CNT,
    4,
    989,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    BBH_TX_COMMON_CONFIGURATIONS_Q2RNR_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: BBH_TX_COMMON_CONFIGURATIONS_PERQTASK
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_COMMON_CONFIGURATIONS_PERQTASK_FIELDS[] =
{
    &BBH_TX_COMMON_CONFIGURATIONS_PERQTASK_TASK0_FIELD,
    &BBH_TX_COMMON_CONFIGURATIONS_PERQTASK_TASK1_FIELD,
    &BBH_TX_COMMON_CONFIGURATIONS_PERQTASK_TASK2_FIELD,
    &BBH_TX_COMMON_CONFIGURATIONS_PERQTASK_TASK3_FIELD,
    &BBH_TX_COMMON_CONFIGURATIONS_PERQTASK_TASK4_FIELD,
    &BBH_TX_COMMON_CONFIGURATIONS_PERQTASK_TASK5_FIELD,
    &BBH_TX_COMMON_CONFIGURATIONS_PERQTASK_TASK6_FIELD,
    &BBH_TX_COMMON_CONFIGURATIONS_PERQTASK_TASK7_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    990,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    8,
    BBH_TX_COMMON_CONFIGURATIONS_PERQTASK_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_FIELDS[] =
{
    &BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_CNTXTRST_FIELD,
    &BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_PDFIFORST_FIELD,
    &BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_DMAPTRRST_FIELD,
    &BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_SDMAPTRRST_FIELD,
    &BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_BPMFIFORST_FIELD,
    &BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_SBPMFIFORST_FIELD,
    &BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_OKFIFORST_FIELD,
    &BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_DDRFIFORST_FIELD,
    &BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_SRAMFIFORST_FIELD,
    &BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_SKBPTRRST_FIELD,
    &BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_STSFIFORST_FIELD,
    &BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_REQFIFORST_FIELD,
    &BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_MSGFIFORST_FIELD,
    &BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_GNXTFIFORST_FIELD,
    &BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_FBNFIFORST_FIELD,
    &BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    991,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    16,
    BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: BBH_TX_COMMON_CONFIGURATIONS_DBGSEL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_COMMON_CONFIGURATIONS_DBGSEL_FIELDS[] =
{
    &BBH_TX_COMMON_CONFIGURATIONS_DBGSEL_DBGSEL_FIELD,
    &BBH_TX_COMMON_CONFIGURATIONS_DBGSEL_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    992,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    BBH_TX_COMMON_CONFIGURATIONS_DBGSEL_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: BBH_TX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_FIELDS[] =
{
    &BBH_TX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_BYPASS_CLK_GATE_FIELD,
    &BBH_TX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_RESERVED0_FIELD,
    &BBH_TX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_TIMER_VAL_FIELD,
    &BBH_TX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_KEEP_ALIVE_EN_FIELD,
    &BBH_TX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_RESERVED1_FIELD,
    &BBH_TX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_KEEP_ALIVE_INTRVL_FIELD,
    &BBH_TX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_RESERVED2_FIELD,
    &BBH_TX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_KEEP_ALIVE_CYC_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    993,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    8,
    BBH_TX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: BBH_TX_WAN_CONFIGURATIONS_PDBASE
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_WAN_CONFIGURATIONS_PDBASE_FIELDS[] =
{
    &BBH_TX_WAN_CONFIGURATIONS_PDBASE_FIFOBASE0_FIELD,
    &BBH_TX_WAN_CONFIGURATIONS_PDBASE_RESERVED0_FIELD,
    &BBH_TX_WAN_CONFIGURATIONS_PDBASE_FIFOBASE1_FIELD,
    &BBH_TX_WAN_CONFIGURATIONS_PDBASE_RESERVED1_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec BBH_TX_WAN_CONFIGURATIONS_PDBASE_REG = 
{
    "WAN_CONFIGURATIONS_PDBASE",
#if RU_INCLUDE_DESC
    "PD_FIFO_BASE %i Register",
    "The BBH manages 40 queues for GPON or 32 queus for EPON (1 for each TCONT/LLID). For each queue it manages a PD FIFO."
    "A total of 256 PDs are available for all queues."
    "For each Queue the SW configures the base and the size within these 256 PDs."
    ""
    "The size of the Status FIFO, 1st BN FIFO and get-next FIFO is the same as the size of the PD FIFO of each queue."
    ""
    "each register in this array defines the PD FIFO base of 2 queues."
    "",
#endif
    BBH_TX_WAN_CONFIGURATIONS_PDBASE_REG_OFFSET,
    BBH_TX_WAN_CONFIGURATIONS_PDBASE_REG_RAM_CNT,
    4,
    994,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    BBH_TX_WAN_CONFIGURATIONS_PDBASE_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: BBH_TX_WAN_CONFIGURATIONS_PDSIZE
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_WAN_CONFIGURATIONS_PDSIZE_FIELDS[] =
{
    &BBH_TX_WAN_CONFIGURATIONS_PDSIZE_FIFOSIZE0_FIELD,
    &BBH_TX_WAN_CONFIGURATIONS_PDSIZE_RESERVED0_FIELD,
    &BBH_TX_WAN_CONFIGURATIONS_PDSIZE_FIFOSIZE1_FIELD,
    &BBH_TX_WAN_CONFIGURATIONS_PDSIZE_RESERVED1_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec BBH_TX_WAN_CONFIGURATIONS_PDSIZE_REG = 
{
    "WAN_CONFIGURATIONS_PDSIZE",
#if RU_INCLUDE_DESC
    "PD_FIFO_SIZE %i Register",
    "The BBH manages 40 queues for GPON and 32 queues for EPON (FIFO per TCONT/LLID). For each queue it manages a PD FIFO."
    "A total of 256 PDs are available for all queues."
    "For each Queue the SW configures the base and the size within these."
    "each register in this array defines the PD FIFO size of 2 queues."
    "",
#endif
    BBH_TX_WAN_CONFIGURATIONS_PDSIZE_REG_OFFSET,
    BBH_TX_WAN_CONFIGURATIONS_PDSIZE_REG_RAM_CNT,
    4,
    995,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    BBH_TX_WAN_CONFIGURATIONS_PDSIZE_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: BBH_TX_WAN_CONFIGURATIONS_PDWKUPH
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_WAN_CONFIGURATIONS_PDWKUPH_FIELDS[] =
{
    &BBH_TX_WAN_CONFIGURATIONS_PDWKUPH_WKUPTHRESH0_FIELD,
    &BBH_TX_WAN_CONFIGURATIONS_PDWKUPH_RESERVED0_FIELD,
    &BBH_TX_WAN_CONFIGURATIONS_PDWKUPH_WKUPTHRESH1_FIELD,
    &BBH_TX_WAN_CONFIGURATIONS_PDWKUPH_RESERVED1_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec BBH_TX_WAN_CONFIGURATIONS_PDWKUPH_REG = 
{
    "WAN_CONFIGURATIONS_PDWKUPH",
#if RU_INCLUDE_DESC
    "PD_WKUP_THRESH %i Register",
    "When a FIFO occupancy is above this wakeup threshold, the BBH will not wake-up the Runner for sending a new PD. This threshold does not represent the actual size of the FIFO. If a PD will arrive from the Runner when the FIFO is above the threshold, it will not be dropped unless the FIFO is actually full."
    "Each register defines the threshold of 2 queues."
    "",
#endif
    BBH_TX_WAN_CONFIGURATIONS_PDWKUPH_REG_OFFSET,
    BBH_TX_WAN_CONFIGURATIONS_PDWKUPH_REG_RAM_CNT,
    4,
    996,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    BBH_TX_WAN_CONFIGURATIONS_PDWKUPH_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: BBH_TX_WAN_CONFIGURATIONS_PD_BYTE_TH
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_WAN_CONFIGURATIONS_PD_BYTE_TH_FIELDS[] =
{
    &BBH_TX_WAN_CONFIGURATIONS_PD_BYTE_TH_PDLIMIT0_FIELD,
    &BBH_TX_WAN_CONFIGURATIONS_PD_BYTE_TH_PDLIMIT1_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec BBH_TX_WAN_CONFIGURATIONS_PD_BYTE_TH_REG = 
{
    "WAN_CONFIGURATIONS_PD_BYTE_TH",
#if RU_INCLUDE_DESC
    "PD_BYTES_THRESHOLD %i Register",
    "The BBH requests PDs from the Runner and maintains a pre-fetch PDs FIFO."
    "The PDs pre fetch is limited either by the PD FIFO configurable size or according to the total number of bytes (deducting bytes already requested/transmitted) for preventing HOL. Full configuration for the first 8 TCONT and one configuration for the rest (TCONTs 8-39)."
    "Each register in this array defines the threshold of 2 queues.",
#endif
    BBH_TX_WAN_CONFIGURATIONS_PD_BYTE_TH_REG_OFFSET,
    BBH_TX_WAN_CONFIGURATIONS_PD_BYTE_TH_REG_RAM_CNT,
    4,
    997,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    BBH_TX_WAN_CONFIGURATIONS_PD_BYTE_TH_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: BBH_TX_WAN_CONFIGURATIONS_PD_BYTE_TH_EN
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_WAN_CONFIGURATIONS_PD_BYTE_TH_EN_FIELDS[] =
{
    &BBH_TX_WAN_CONFIGURATIONS_PD_BYTE_TH_EN_PDLIMITEN_FIELD,
    &BBH_TX_WAN_CONFIGURATIONS_PD_BYTE_TH_EN_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    998,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    BBH_TX_WAN_CONFIGURATIONS_PD_BYTE_TH_EN_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: BBH_TX_WAN_CONFIGURATIONS_PDEMPTY
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_WAN_CONFIGURATIONS_PDEMPTY_FIELDS[] =
{
    &BBH_TX_WAN_CONFIGURATIONS_PDEMPTY_EMPTY_FIELD,
    &BBH_TX_WAN_CONFIGURATIONS_PDEMPTY_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    999,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    BBH_TX_WAN_CONFIGURATIONS_PDEMPTY_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: BBH_TX_WAN_CONFIGURATIONS_STSRNRCFG_1
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_WAN_CONFIGURATIONS_STSRNRCFG_1_FIELDS[] =
{
    &BBH_TX_WAN_CONFIGURATIONS_STSRNRCFG_1_TCONTADDR_FIELD,
    &BBH_TX_WAN_CONFIGURATIONS_STSRNRCFG_1_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    1000,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    BBH_TX_WAN_CONFIGURATIONS_STSRNRCFG_1_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: BBH_TX_WAN_CONFIGURATIONS_STSRNRCFG_2
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_WAN_CONFIGURATIONS_STSRNRCFG_2_FIELDS[] =
{
    &BBH_TX_WAN_CONFIGURATIONS_STSRNRCFG_2_PTRADDR_FIELD,
    &BBH_TX_WAN_CONFIGURATIONS_STSRNRCFG_2_TASK_FIELD,
    &BBH_TX_WAN_CONFIGURATIONS_STSRNRCFG_2_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    1001,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    BBH_TX_WAN_CONFIGURATIONS_STSRNRCFG_2_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: BBH_TX_WAN_CONFIGURATIONS_MSGRNRCFG_1
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_WAN_CONFIGURATIONS_MSGRNRCFG_1_FIELDS[] =
{
    &BBH_TX_WAN_CONFIGURATIONS_MSGRNRCFG_1_TCONTADDR_FIELD,
    &BBH_TX_WAN_CONFIGURATIONS_MSGRNRCFG_1_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    1002,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    BBH_TX_WAN_CONFIGURATIONS_MSGRNRCFG_1_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: BBH_TX_WAN_CONFIGURATIONS_MSGRNRCFG_2
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_WAN_CONFIGURATIONS_MSGRNRCFG_2_FIELDS[] =
{
    &BBH_TX_WAN_CONFIGURATIONS_MSGRNRCFG_2_PTRADDR_FIELD,
    &BBH_TX_WAN_CONFIGURATIONS_MSGRNRCFG_2_TASK_FIELD,
    &BBH_TX_WAN_CONFIGURATIONS_MSGRNRCFG_2_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    1003,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    BBH_TX_WAN_CONFIGURATIONS_MSGRNRCFG_2_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: BBH_TX_WAN_CONFIGURATIONS_EPNCFG
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_WAN_CONFIGURATIONS_EPNCFG_FIELDS[] =
{
    &BBH_TX_WAN_CONFIGURATIONS_EPNCFG_STPLENERR_FIELD,
    &BBH_TX_WAN_CONFIGURATIONS_EPNCFG_CMP_WIDTH_FIELD,
    &BBH_TX_WAN_CONFIGURATIONS_EPNCFG_CONSIDERFULL_FIELD,
    &BBH_TX_WAN_CONFIGURATIONS_EPNCFG_ADDCRC_FIELD,
    &BBH_TX_WAN_CONFIGURATIONS_EPNCFG_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    1004,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    5,
    BBH_TX_WAN_CONFIGURATIONS_EPNCFG_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: BBH_TX_WAN_CONFIGURATIONS_FLOW2PORT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_WAN_CONFIGURATIONS_FLOW2PORT_FIELDS[] =
{
    &BBH_TX_WAN_CONFIGURATIONS_FLOW2PORT_WDATA_FIELD,
    &BBH_TX_WAN_CONFIGURATIONS_FLOW2PORT_A_FIELD,
    &BBH_TX_WAN_CONFIGURATIONS_FLOW2PORT_CMD_FIELD,
    &BBH_TX_WAN_CONFIGURATIONS_FLOW2PORT_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    1005,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    BBH_TX_WAN_CONFIGURATIONS_FLOW2PORT_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: BBH_TX_WAN_CONFIGURATIONS_TS
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_WAN_CONFIGURATIONS_TS_FIELDS[] =
{
    &BBH_TX_WAN_CONFIGURATIONS_TS_EN_FIELD,
    &BBH_TX_WAN_CONFIGURATIONS_TS_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    1006,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    BBH_TX_WAN_CONFIGURATIONS_TS_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: BBH_TX_WAN_CONFIGURATIONS_MAXWLEN
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_WAN_CONFIGURATIONS_MAXWLEN_FIELDS[] =
{
    &BBH_TX_WAN_CONFIGURATIONS_MAXWLEN_MAXWLEN_FIELD,
    &BBH_TX_WAN_CONFIGURATIONS_MAXWLEN_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    1007,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    BBH_TX_WAN_CONFIGURATIONS_MAXWLEN_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: BBH_TX_WAN_CONFIGURATIONS_FLUSH
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_WAN_CONFIGURATIONS_FLUSH_FIELDS[] =
{
    &BBH_TX_WAN_CONFIGURATIONS_FLUSH_FLUSH_FIELD,
    &BBH_TX_WAN_CONFIGURATIONS_FLUSH_RESERVED0_FIELD,
    &BBH_TX_WAN_CONFIGURATIONS_FLUSH_SRST_N_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    1008,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    BBH_TX_WAN_CONFIGURATIONS_FLUSH_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: BBH_TX_LAN_CONFIGURATIONS_PDBASE
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_LAN_CONFIGURATIONS_PDBASE_FIELDS[] =
{
    &BBH_TX_LAN_CONFIGURATIONS_PDBASE_FIFOBASE0_FIELD,
    &BBH_TX_LAN_CONFIGURATIONS_PDBASE_RESERVED0_FIELD,
    &BBH_TX_LAN_CONFIGURATIONS_PDBASE_FIFOBASE1_FIELD,
    &BBH_TX_LAN_CONFIGURATIONS_PDBASE_RESERVED1_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec BBH_TX_LAN_CONFIGURATIONS_PDBASE_REG = 
{
    "LAN_CONFIGURATIONS_PDBASE",
#if RU_INCLUDE_DESC
    "PD_FIFO_BASE Register",
    "The BBH manages 40 queues for GPON or 32 queus for EPON (1 for each TCONT/LLID). For each queue it manages a PD FIFO."
    "A total of 256 PDs are available for all queues."
    "For each Queue the SW configures the base and the size within these 256 PDs."
    ""
    "The size of the 1st BN FIFO and get-next FIFO is the same as the size of the PD FIFO of each queue."
    ""
    "each register in this array defines the PD FIFO base of 2 queues."
    "",
#endif
    BBH_TX_LAN_CONFIGURATIONS_PDBASE_REG_OFFSET,
    0,
    0,
    1009,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    BBH_TX_LAN_CONFIGURATIONS_PDBASE_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: BBH_TX_LAN_CONFIGURATIONS_PDSIZE
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_LAN_CONFIGURATIONS_PDSIZE_FIELDS[] =
{
    &BBH_TX_LAN_CONFIGURATIONS_PDSIZE_FIFOSIZE0_FIELD,
    &BBH_TX_LAN_CONFIGURATIONS_PDSIZE_RESERVED0_FIELD,
    &BBH_TX_LAN_CONFIGURATIONS_PDSIZE_FIFOSIZE1_FIELD,
    &BBH_TX_LAN_CONFIGURATIONS_PDSIZE_RESERVED1_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec BBH_TX_LAN_CONFIGURATIONS_PDSIZE_REG = 
{
    "LAN_CONFIGURATIONS_PDSIZE",
#if RU_INCLUDE_DESC
    "PD_FIFO_SIZE Register",
    "The BBH manages 40 queues for GPON and 32 queues for EPON (FIFO per TCONT/LLID). For each queue it manages a PD FIFO."
    "A total of 256 PDs are available for all queues."
    "For each Queue the SW configures the base and the size within these."
    "each register in this array defines the PD FIFO size of 2 queues."
    "",
#endif
    BBH_TX_LAN_CONFIGURATIONS_PDSIZE_REG_OFFSET,
    0,
    0,
    1010,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    BBH_TX_LAN_CONFIGURATIONS_PDSIZE_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: BBH_TX_LAN_CONFIGURATIONS_PDWKUPH
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_LAN_CONFIGURATIONS_PDWKUPH_FIELDS[] =
{
    &BBH_TX_LAN_CONFIGURATIONS_PDWKUPH_WKUPTHRESH0_FIELD,
    &BBH_TX_LAN_CONFIGURATIONS_PDWKUPH_WKUPTHRESH1_FIELD,
    &BBH_TX_LAN_CONFIGURATIONS_PDWKUPH_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec BBH_TX_LAN_CONFIGURATIONS_PDWKUPH_REG = 
{
    "LAN_CONFIGURATIONS_PDWKUPH",
#if RU_INCLUDE_DESC
    "PD_WKUP_THRESH Register",
    "When a FIFO occupancy is above this wakeup threshold, the BBH will not wake-up the Runner for sending a new PD. This threshold does not represent the actual size of the FIFO. If a PD will arrive from the Runner when the FIFO is above the threshold, it will not be dropped unless the FIFO is actually full."
    "Each register defines the threshold of 2 queues."
    "",
#endif
    BBH_TX_LAN_CONFIGURATIONS_PDWKUPH_REG_OFFSET,
    0,
    0,
    1011,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    BBH_TX_LAN_CONFIGURATIONS_PDWKUPH_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: BBH_TX_LAN_CONFIGURATIONS_PD_BYTE_TH
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_LAN_CONFIGURATIONS_PD_BYTE_TH_FIELDS[] =
{
    &BBH_TX_LAN_CONFIGURATIONS_PD_BYTE_TH_PDLIMIT0_FIELD,
    &BBH_TX_LAN_CONFIGURATIONS_PD_BYTE_TH_PDLIMIT1_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec BBH_TX_LAN_CONFIGURATIONS_PD_BYTE_TH_REG = 
{
    "LAN_CONFIGURATIONS_PD_BYTE_TH",
#if RU_INCLUDE_DESC
    "PD_BYTES_THRESHOLD Register",
    "The BBH requests PDs from the Runner and maintains a pre-fetch PDs FIFO."
    "The PDs pre fetch is limited either by the PD FIFO configurable size or according to the total number of bytes (deducting bytes already requested/transmitted) for preventing HOL. Full configuration for the first 8 TCONT and one configuration for the rest (TCONTs 8-39)."
    "Each register in this array defines the threshold of 2 queues.",
#endif
    BBH_TX_LAN_CONFIGURATIONS_PD_BYTE_TH_REG_OFFSET,
    0,
    0,
    1012,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    BBH_TX_LAN_CONFIGURATIONS_PD_BYTE_TH_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: BBH_TX_LAN_CONFIGURATIONS_PD_BYTE_TH_EN
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_LAN_CONFIGURATIONS_PD_BYTE_TH_EN_FIELDS[] =
{
    &BBH_TX_LAN_CONFIGURATIONS_PD_BYTE_TH_EN_PDLIMITEN_FIELD,
    &BBH_TX_LAN_CONFIGURATIONS_PD_BYTE_TH_EN_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    1013,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    BBH_TX_LAN_CONFIGURATIONS_PD_BYTE_TH_EN_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: BBH_TX_LAN_CONFIGURATIONS_PDEMPTY
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_LAN_CONFIGURATIONS_PDEMPTY_FIELDS[] =
{
    &BBH_TX_LAN_CONFIGURATIONS_PDEMPTY_EMPTY_FIELD,
    &BBH_TX_LAN_CONFIGURATIONS_PDEMPTY_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    1014,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    BBH_TX_LAN_CONFIGURATIONS_PDEMPTY_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: BBH_TX_LAN_CONFIGURATIONS_TXTHRESH
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_LAN_CONFIGURATIONS_TXTHRESH_FIELDS[] =
{
    &BBH_TX_LAN_CONFIGURATIONS_TXTHRESH_DDRTHRESH_FIELD,
    &BBH_TX_LAN_CONFIGURATIONS_TXTHRESH_RESERVED0_FIELD,
    &BBH_TX_LAN_CONFIGURATIONS_TXTHRESH_SRAMTHRESH_FIELD,
    &BBH_TX_LAN_CONFIGURATIONS_TXTHRESH_RESERVED1_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    1015,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    BBH_TX_LAN_CONFIGURATIONS_TXTHRESH_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: BBH_TX_LAN_CONFIGURATIONS_EEE
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_LAN_CONFIGURATIONS_EEE_FIELDS[] =
{
    &BBH_TX_LAN_CONFIGURATIONS_EEE_EN_FIELD,
    &BBH_TX_LAN_CONFIGURATIONS_EEE_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    1016,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    BBH_TX_LAN_CONFIGURATIONS_EEE_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: BBH_TX_LAN_CONFIGURATIONS_TS
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_LAN_CONFIGURATIONS_TS_FIELDS[] =
{
    &BBH_TX_LAN_CONFIGURATIONS_TS_EN_FIELD,
    &BBH_TX_LAN_CONFIGURATIONS_TS_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    1017,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    BBH_TX_LAN_CONFIGURATIONS_TS_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: BBH_TX_UNIFIED_CONFIGURATIONS_PDBASE
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_UNIFIED_CONFIGURATIONS_PDBASE_FIELDS[] =
{
    &BBH_TX_UNIFIED_CONFIGURATIONS_PDBASE_FIFOBASE0_FIELD,
    &BBH_TX_UNIFIED_CONFIGURATIONS_PDBASE_RESERVED0_FIELD,
    &BBH_TX_UNIFIED_CONFIGURATIONS_PDBASE_FIFOBASE1_FIELD,
    &BBH_TX_UNIFIED_CONFIGURATIONS_PDBASE_RESERVED1_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec BBH_TX_UNIFIED_CONFIGURATIONS_PDBASE_REG = 
{
    "UNIFIED_CONFIGURATIONS_PDBASE",
#if RU_INCLUDE_DESC
    "PD_FIFO_BASE %i Register",
    "The BBH manages 6 queues. Each queue is dedicated to one MAC interface."
    "A total of 48 PDs are available for all queues."
    "For each Queue the SW configures the base and the size within these 48 PDs."
    ""
    "The size of the 1st BN FIFO and get-next FIFO is the same as the size of the PD FIFO of each queue."
    ""
    "each register in this array defines the PD FIFO base of 2 queues."
    "",
#endif
    BBH_TX_UNIFIED_CONFIGURATIONS_PDBASE_REG_OFFSET,
    BBH_TX_UNIFIED_CONFIGURATIONS_PDBASE_REG_RAM_CNT,
    4,
    1018,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    BBH_TX_UNIFIED_CONFIGURATIONS_PDBASE_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: BBH_TX_UNIFIED_CONFIGURATIONS_PDSIZE
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_UNIFIED_CONFIGURATIONS_PDSIZE_FIELDS[] =
{
    &BBH_TX_UNIFIED_CONFIGURATIONS_PDSIZE_FIFOSIZE0_FIELD,
    &BBH_TX_UNIFIED_CONFIGURATIONS_PDSIZE_RESERVED0_FIELD,
    &BBH_TX_UNIFIED_CONFIGURATIONS_PDSIZE_FIFOSIZE1_FIELD,
    &BBH_TX_UNIFIED_CONFIGURATIONS_PDSIZE_RESERVED1_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec BBH_TX_UNIFIED_CONFIGURATIONS_PDSIZE_REG = 
{
    "UNIFIED_CONFIGURATIONS_PDSIZE",
#if RU_INCLUDE_DESC
    "PD_FIFO_SIZE %i Register",
    "The BBH manages 6 queues. Each queue is dedicated to one MAC interface."
    "A total of 48 PDs are available for all queues."
    "For each Queue the SW configures the base and the size within these 48 PDs."
    "each register in this array defines the PD FIFO size of 2 queues."
    "",
#endif
    BBH_TX_UNIFIED_CONFIGURATIONS_PDSIZE_REG_OFFSET,
    BBH_TX_UNIFIED_CONFIGURATIONS_PDSIZE_REG_RAM_CNT,
    4,
    1019,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    BBH_TX_UNIFIED_CONFIGURATIONS_PDSIZE_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: BBH_TX_UNIFIED_CONFIGURATIONS_PDWKUPH
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_UNIFIED_CONFIGURATIONS_PDWKUPH_FIELDS[] =
{
    &BBH_TX_UNIFIED_CONFIGURATIONS_PDWKUPH_WKUPTHRESH0_FIELD,
    &BBH_TX_UNIFIED_CONFIGURATIONS_PDWKUPH_WKUPTHRESH1_FIELD,
    &BBH_TX_UNIFIED_CONFIGURATIONS_PDWKUPH_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec BBH_TX_UNIFIED_CONFIGURATIONS_PDWKUPH_REG = 
{
    "UNIFIED_CONFIGURATIONS_PDWKUPH",
#if RU_INCLUDE_DESC
    "PD_WKUP_THRESH %i Register",
    "When a FIFO occupancy is above this wakeup threshold, the BBH will not wake-up the Runner for sending a new PD. This threshold does not represent the actual size of the FIFO. If a PD will arrive from the Runner when the FIFO is above the threshold, it will not be dropped unless the FIFO is actually full."
    "Each register defines the threshold of 2 queues."
    "",
#endif
    BBH_TX_UNIFIED_CONFIGURATIONS_PDWKUPH_REG_OFFSET,
    BBH_TX_UNIFIED_CONFIGURATIONS_PDWKUPH_REG_RAM_CNT,
    4,
    1020,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    BBH_TX_UNIFIED_CONFIGURATIONS_PDWKUPH_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: BBH_TX_UNIFIED_CONFIGURATIONS_PD_BYTE_TH
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_UNIFIED_CONFIGURATIONS_PD_BYTE_TH_FIELDS[] =
{
    &BBH_TX_UNIFIED_CONFIGURATIONS_PD_BYTE_TH_PDLIMIT0_FIELD,
    &BBH_TX_UNIFIED_CONFIGURATIONS_PD_BYTE_TH_PDLIMIT1_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec BBH_TX_UNIFIED_CONFIGURATIONS_PD_BYTE_TH_REG = 
{
    "UNIFIED_CONFIGURATIONS_PD_BYTE_TH",
#if RU_INCLUDE_DESC
    "PD_BYTES_THRESHOLD %i Register",
    "The BBH requests PDs from the Runner and maintains a pre-fetch PDs FIFO."
    "The PDs pre fetch is limited either by the PD FIFO configurable size or according to the total number of bytes (deducting bytes already requested/transmitted) for preventing HOL. Full configuration for the first 8 TCONT and one configuration for the rest (TCONTs 8-39)."
    "Each register in this array defines the threshold of 2 queues.",
#endif
    BBH_TX_UNIFIED_CONFIGURATIONS_PD_BYTE_TH_REG_OFFSET,
    BBH_TX_UNIFIED_CONFIGURATIONS_PD_BYTE_TH_REG_RAM_CNT,
    4,
    1021,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    BBH_TX_UNIFIED_CONFIGURATIONS_PD_BYTE_TH_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: BBH_TX_UNIFIED_CONFIGURATIONS_PD_BYTE_TH_EN
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_UNIFIED_CONFIGURATIONS_PD_BYTE_TH_EN_FIELDS[] =
{
    &BBH_TX_UNIFIED_CONFIGURATIONS_PD_BYTE_TH_EN_PDLIMITEN_FIELD,
    &BBH_TX_UNIFIED_CONFIGURATIONS_PD_BYTE_TH_EN_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    1022,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    BBH_TX_UNIFIED_CONFIGURATIONS_PD_BYTE_TH_EN_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: BBH_TX_UNIFIED_CONFIGURATIONS_PDEMPTY
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_UNIFIED_CONFIGURATIONS_PDEMPTY_FIELDS[] =
{
    &BBH_TX_UNIFIED_CONFIGURATIONS_PDEMPTY_EMPTY_FIELD,
    &BBH_TX_UNIFIED_CONFIGURATIONS_PDEMPTY_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    1023,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    BBH_TX_UNIFIED_CONFIGURATIONS_PDEMPTY_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: BBH_TX_UNIFIED_CONFIGURATIONS_GTXTHRESH
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_UNIFIED_CONFIGURATIONS_GTXTHRESH_FIELDS[] =
{
    &BBH_TX_UNIFIED_CONFIGURATIONS_GTXTHRESH_DDRTHRESH_FIELD,
    &BBH_TX_UNIFIED_CONFIGURATIONS_GTXTHRESH_RESERVED0_FIELD,
    &BBH_TX_UNIFIED_CONFIGURATIONS_GTXTHRESH_SRAMTHRESH_FIELD,
    &BBH_TX_UNIFIED_CONFIGURATIONS_GTXTHRESH_RESERVED1_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    1024,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    BBH_TX_UNIFIED_CONFIGURATIONS_GTXTHRESH_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: BBH_TX_UNIFIED_CONFIGURATIONS_EEE
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_UNIFIED_CONFIGURATIONS_EEE_FIELDS[] =
{
    &BBH_TX_UNIFIED_CONFIGURATIONS_EEE_EN_FIELD,
    &BBH_TX_UNIFIED_CONFIGURATIONS_EEE_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    1025,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    BBH_TX_UNIFIED_CONFIGURATIONS_EEE_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: BBH_TX_UNIFIED_CONFIGURATIONS_TS
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_UNIFIED_CONFIGURATIONS_TS_FIELDS[] =
{
    &BBH_TX_UNIFIED_CONFIGURATIONS_TS_EN_FIELD,
    &BBH_TX_UNIFIED_CONFIGURATIONS_TS_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    1026,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    BBH_TX_UNIFIED_CONFIGURATIONS_TS_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: BBH_TX_UNIFIED_CONFIGURATIONS_FEBASE
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_UNIFIED_CONFIGURATIONS_FEBASE_FIELDS[] =
{
    &BBH_TX_UNIFIED_CONFIGURATIONS_FEBASE_FIFOBASE0_FIELD,
    &BBH_TX_UNIFIED_CONFIGURATIONS_FEBASE_RESERVED0_FIELD,
    &BBH_TX_UNIFIED_CONFIGURATIONS_FEBASE_FIFOBASE1_FIELD,
    &BBH_TX_UNIFIED_CONFIGURATIONS_FEBASE_RESERVED1_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    1027,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    BBH_TX_UNIFIED_CONFIGURATIONS_FEBASE_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: BBH_TX_UNIFIED_CONFIGURATIONS_FESIZE
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_UNIFIED_CONFIGURATIONS_FESIZE_FIELDS[] =
{
    &BBH_TX_UNIFIED_CONFIGURATIONS_FESIZE_FIFOSIZE0_FIELD,
    &BBH_TX_UNIFIED_CONFIGURATIONS_FESIZE_RESERVED0_FIELD,
    &BBH_TX_UNIFIED_CONFIGURATIONS_FESIZE_FIFOSIZE1_FIELD,
    &BBH_TX_UNIFIED_CONFIGURATIONS_FESIZE_RESERVED1_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    1028,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    BBH_TX_UNIFIED_CONFIGURATIONS_FESIZE_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: BBH_TX_UNIFIED_CONFIGURATIONS_FEPDBASE
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_UNIFIED_CONFIGURATIONS_FEPDBASE_FIELDS[] =
{
    &BBH_TX_UNIFIED_CONFIGURATIONS_FEPDBASE_FIFOBASE0_FIELD,
    &BBH_TX_UNIFIED_CONFIGURATIONS_FEPDBASE_RESERVED0_FIELD,
    &BBH_TX_UNIFIED_CONFIGURATIONS_FEPDBASE_FIFOBASE1_FIELD,
    &BBH_TX_UNIFIED_CONFIGURATIONS_FEPDBASE_RESERVED1_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    1029,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    BBH_TX_UNIFIED_CONFIGURATIONS_FEPDBASE_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: BBH_TX_UNIFIED_CONFIGURATIONS_FEPDSIZE
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_UNIFIED_CONFIGURATIONS_FEPDSIZE_FIELDS[] =
{
    &BBH_TX_UNIFIED_CONFIGURATIONS_FEPDSIZE_FIFOSIZE0_FIELD,
    &BBH_TX_UNIFIED_CONFIGURATIONS_FEPDSIZE_RESERVED0_FIELD,
    &BBH_TX_UNIFIED_CONFIGURATIONS_FEPDSIZE_FIFOSIZE1_FIELD,
    &BBH_TX_UNIFIED_CONFIGURATIONS_FEPDSIZE_RESERVED1_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    1030,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    BBH_TX_UNIFIED_CONFIGURATIONS_FEPDSIZE_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: BBH_TX_UNIFIED_CONFIGURATIONS_TXWRR
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_UNIFIED_CONFIGURATIONS_TXWRR_FIELDS[] =
{
    &BBH_TX_UNIFIED_CONFIGURATIONS_TXWRR_W0_FIELD,
    &BBH_TX_UNIFIED_CONFIGURATIONS_TXWRR_RESERVED0_FIELD,
    &BBH_TX_UNIFIED_CONFIGURATIONS_TXWRR_W1_FIELD,
    &BBH_TX_UNIFIED_CONFIGURATIONS_TXWRR_RESERVED1_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    1031,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    BBH_TX_UNIFIED_CONFIGURATIONS_TXWRR_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: BBH_TX_UNIFIED_CONFIGURATIONS_TXTHRESH
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_UNIFIED_CONFIGURATIONS_TXTHRESH_FIELDS[] =
{
    &BBH_TX_UNIFIED_CONFIGURATIONS_TXTHRESH_THRESH0_FIELD,
    &BBH_TX_UNIFIED_CONFIGURATIONS_TXTHRESH_RESERVED0_FIELD,
    &BBH_TX_UNIFIED_CONFIGURATIONS_TXTHRESH_THRESH1_FIELD,
    &BBH_TX_UNIFIED_CONFIGURATIONS_TXTHRESH_RESERVED1_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    1032,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    BBH_TX_UNIFIED_CONFIGURATIONS_TXTHRESH_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: BBH_TX_DEBUG_COUNTERS_SRAMPD
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_DEBUG_COUNTERS_SRAMPD_FIELDS[] =
{
    &BBH_TX_DEBUG_COUNTERS_SRAMPD_SRAMPD_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    1033,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_TX_DEBUG_COUNTERS_SRAMPD_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: BBH_TX_DEBUG_COUNTERS_DDRPD
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_DEBUG_COUNTERS_DDRPD_FIELDS[] =
{
    &BBH_TX_DEBUG_COUNTERS_DDRPD_DDRPD_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    1034,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_TX_DEBUG_COUNTERS_DDRPD_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: BBH_TX_DEBUG_COUNTERS_PDDROP
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_DEBUG_COUNTERS_PDDROP_FIELDS[] =
{
    &BBH_TX_DEBUG_COUNTERS_PDDROP_PDDROP_FIELD,
    &BBH_TX_DEBUG_COUNTERS_PDDROP_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    1035,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    BBH_TX_DEBUG_COUNTERS_PDDROP_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: BBH_TX_DEBUG_COUNTERS_STSCNT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_DEBUG_COUNTERS_STSCNT_FIELDS[] =
{
    &BBH_TX_DEBUG_COUNTERS_STSCNT_STSCNT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    1036,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_TX_DEBUG_COUNTERS_STSCNT_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: BBH_TX_DEBUG_COUNTERS_STSDROP
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_DEBUG_COUNTERS_STSDROP_FIELDS[] =
{
    &BBH_TX_DEBUG_COUNTERS_STSDROP_STSDROP_FIELD,
    &BBH_TX_DEBUG_COUNTERS_STSDROP_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    1037,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    BBH_TX_DEBUG_COUNTERS_STSDROP_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: BBH_TX_DEBUG_COUNTERS_MSGCNT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_DEBUG_COUNTERS_MSGCNT_FIELDS[] =
{
    &BBH_TX_DEBUG_COUNTERS_MSGCNT_MSGCNT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    1038,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_TX_DEBUG_COUNTERS_MSGCNT_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: BBH_TX_DEBUG_COUNTERS_MSGDROP
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_DEBUG_COUNTERS_MSGDROP_FIELDS[] =
{
    &BBH_TX_DEBUG_COUNTERS_MSGDROP_MSGDROP_FIELD,
    &BBH_TX_DEBUG_COUNTERS_MSGDROP_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    1039,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    BBH_TX_DEBUG_COUNTERS_MSGDROP_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: BBH_TX_DEBUG_COUNTERS_GETNEXTNULL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_DEBUG_COUNTERS_GETNEXTNULL_FIELDS[] =
{
    &BBH_TX_DEBUG_COUNTERS_GETNEXTNULL_GETNEXTNULL_FIELD,
    &BBH_TX_DEBUG_COUNTERS_GETNEXTNULL_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    1040,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    BBH_TX_DEBUG_COUNTERS_GETNEXTNULL_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: BBH_TX_DEBUG_COUNTERS_FLUSHPKTS
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_DEBUG_COUNTERS_FLUSHPKTS_FIELDS[] =
{
    &BBH_TX_DEBUG_COUNTERS_FLUSHPKTS_FLSHPKTS_FIELD,
    &BBH_TX_DEBUG_COUNTERS_FLUSHPKTS_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    1041,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    BBH_TX_DEBUG_COUNTERS_FLUSHPKTS_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: BBH_TX_DEBUG_COUNTERS_LENERR
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_DEBUG_COUNTERS_LENERR_FIELDS[] =
{
    &BBH_TX_DEBUG_COUNTERS_LENERR_LENERR_FIELD,
    &BBH_TX_DEBUG_COUNTERS_LENERR_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    1042,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    BBH_TX_DEBUG_COUNTERS_LENERR_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: BBH_TX_DEBUG_COUNTERS_AGGRLENERR
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_DEBUG_COUNTERS_AGGRLENERR_FIELDS[] =
{
    &BBH_TX_DEBUG_COUNTERS_AGGRLENERR_AGGRLENERR_FIELD,
    &BBH_TX_DEBUG_COUNTERS_AGGRLENERR_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    1043,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    BBH_TX_DEBUG_COUNTERS_AGGRLENERR_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: BBH_TX_DEBUG_COUNTERS_SRAMPKT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_DEBUG_COUNTERS_SRAMPKT_FIELDS[] =
{
    &BBH_TX_DEBUG_COUNTERS_SRAMPKT_SRAMPKT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    1044,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_TX_DEBUG_COUNTERS_SRAMPKT_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: BBH_TX_DEBUG_COUNTERS_DDRPKT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_DEBUG_COUNTERS_DDRPKT_FIELDS[] =
{
    &BBH_TX_DEBUG_COUNTERS_DDRPKT_DDRPKT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    1045,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_TX_DEBUG_COUNTERS_DDRPKT_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: BBH_TX_DEBUG_COUNTERS_SRAMBYTE
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_DEBUG_COUNTERS_SRAMBYTE_FIELDS[] =
{
    &BBH_TX_DEBUG_COUNTERS_SRAMBYTE_SRAMBYTE_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    1046,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_TX_DEBUG_COUNTERS_SRAMBYTE_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: BBH_TX_DEBUG_COUNTERS_DDRBYTE
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_DEBUG_COUNTERS_DDRBYTE_FIELDS[] =
{
    &BBH_TX_DEBUG_COUNTERS_DDRBYTE_DDRBYTE_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    1047,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_TX_DEBUG_COUNTERS_DDRBYTE_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: BBH_TX_DEBUG_COUNTERS_SWRDEN
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_DEBUG_COUNTERS_SWRDEN_FIELDS[] =
{
    &BBH_TX_DEBUG_COUNTERS_SWRDEN_PDSEL_FIELD,
    &BBH_TX_DEBUG_COUNTERS_SWRDEN_PDVSEL_FIELD,
    &BBH_TX_DEBUG_COUNTERS_SWRDEN_PDEMPTYSEL_FIELD,
    &BBH_TX_DEBUG_COUNTERS_SWRDEN_PDFULLSEL_FIELD,
    &BBH_TX_DEBUG_COUNTERS_SWRDEN_PDBEMPTYSEL_FIELD,
    &BBH_TX_DEBUG_COUNTERS_SWRDEN_PDFFWKPSEL_FIELD,
    &BBH_TX_DEBUG_COUNTERS_SWRDEN_FBNSEL_FIELD,
    &BBH_TX_DEBUG_COUNTERS_SWRDEN_FBNVSEL_FIELD,
    &BBH_TX_DEBUG_COUNTERS_SWRDEN_FBNEMPTYSEL_FIELD,
    &BBH_TX_DEBUG_COUNTERS_SWRDEN_FBNFULLSEL_FIELD,
    &BBH_TX_DEBUG_COUNTERS_SWRDEN_GETNEXTSEL_FIELD,
    &BBH_TX_DEBUG_COUNTERS_SWRDEN_GETNEXTVSEL_FIELD,
    &BBH_TX_DEBUG_COUNTERS_SWRDEN_GETNEXTEMPTYSEL_FIELD,
    &BBH_TX_DEBUG_COUNTERS_SWRDEN_GETNEXTFULLSEL_FIELD,
    &BBH_TX_DEBUG_COUNTERS_SWRDEN_GPNCNTXTSEL_FIELD,
    &BBH_TX_DEBUG_COUNTERS_SWRDEN_BPMSEL_FIELD,
    &BBH_TX_DEBUG_COUNTERS_SWRDEN_BPMFSEL_FIELD,
    &BBH_TX_DEBUG_COUNTERS_SWRDEN_SBPMSEL_FIELD,
    &BBH_TX_DEBUG_COUNTERS_SWRDEN_SBPMFSEL_FIELD,
    &BBH_TX_DEBUG_COUNTERS_SWRDEN_STSSEL_FIELD,
    &BBH_TX_DEBUG_COUNTERS_SWRDEN_STSVSEL_FIELD,
    &BBH_TX_DEBUG_COUNTERS_SWRDEN_STSEMPTYSEL_FIELD,
    &BBH_TX_DEBUG_COUNTERS_SWRDEN_STSFULLSEL_FIELD,
    &BBH_TX_DEBUG_COUNTERS_SWRDEN_STSBEMPTYSEL_FIELD,
    &BBH_TX_DEBUG_COUNTERS_SWRDEN_STSFFWKPSEL_FIELD,
    &BBH_TX_DEBUG_COUNTERS_SWRDEN_MSGSEL_FIELD,
    &BBH_TX_DEBUG_COUNTERS_SWRDEN_MSGVSEL_FIELD,
    &BBH_TX_DEBUG_COUNTERS_SWRDEN_EPNREQSEL_FIELD,
    &BBH_TX_DEBUG_COUNTERS_SWRDEN_DATASEL_FIELD,
    &BBH_TX_DEBUG_COUNTERS_SWRDEN_REORDERSEL_FIELD,
    &BBH_TX_DEBUG_COUNTERS_SWRDEN_TSINFOSEL_FIELD,
    &BBH_TX_DEBUG_COUNTERS_SWRDEN_MACTXSEL_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    1048,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    32,
    BBH_TX_DEBUG_COUNTERS_SWRDEN_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: BBH_TX_DEBUG_COUNTERS_SWRDADDR
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_DEBUG_COUNTERS_SWRDADDR_FIELDS[] =
{
    &BBH_TX_DEBUG_COUNTERS_SWRDADDR_RDADDR_FIELD,
    &BBH_TX_DEBUG_COUNTERS_SWRDADDR_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    1049,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    BBH_TX_DEBUG_COUNTERS_SWRDADDR_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: BBH_TX_DEBUG_COUNTERS_SWRDDATA
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_DEBUG_COUNTERS_SWRDDATA_FIELDS[] =
{
    &BBH_TX_DEBUG_COUNTERS_SWRDDATA_DATA_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    1050,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_TX_DEBUG_COUNTERS_SWRDDATA_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: BBH_TX_DEBUG_COUNTERS_UNIFIEDPKT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_DEBUG_COUNTERS_UNIFIEDPKT_FIELDS[] =
{
    &BBH_TX_DEBUG_COUNTERS_UNIFIEDPKT_DDRBYTE_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    1051,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_TX_DEBUG_COUNTERS_UNIFIEDPKT_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: BBH_TX_DEBUG_COUNTERS_UNIFIEDBYTE
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_DEBUG_COUNTERS_UNIFIEDBYTE_FIELDS[] =
{
    &BBH_TX_DEBUG_COUNTERS_UNIFIEDBYTE_DDRBYTE_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    1052,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_TX_DEBUG_COUNTERS_UNIFIEDBYTE_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: BBH_TX_DEBUG_COUNTERS_DBGOUTREG
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_DEBUG_COUNTERS_DBGOUTREG_FIELDS[] =
{
    &BBH_TX_DEBUG_COUNTERS_DBGOUTREG_DBGVEC_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    1053,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_TX_DEBUG_COUNTERS_DBGOUTREG_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
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
    &BBH_TX_COMMON_CONFIGURATIONS_Q2RNR_REG,
    &BBH_TX_COMMON_CONFIGURATIONS_PERQTASK_REG,
    &BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_REG,
    &BBH_TX_COMMON_CONFIGURATIONS_DBGSEL_REG,
    &BBH_TX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_REG,
    &BBH_TX_WAN_CONFIGURATIONS_PDBASE_REG,
    &BBH_TX_WAN_CONFIGURATIONS_PDSIZE_REG,
    &BBH_TX_WAN_CONFIGURATIONS_PDWKUPH_REG,
    &BBH_TX_WAN_CONFIGURATIONS_PD_BYTE_TH_REG,
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
    &BBH_TX_LAN_CONFIGURATIONS_PDBASE_REG,
    &BBH_TX_LAN_CONFIGURATIONS_PDSIZE_REG,
    &BBH_TX_LAN_CONFIGURATIONS_PDWKUPH_REG,
    &BBH_TX_LAN_CONFIGURATIONS_PD_BYTE_TH_REG,
    &BBH_TX_LAN_CONFIGURATIONS_PD_BYTE_TH_EN_REG,
    &BBH_TX_LAN_CONFIGURATIONS_PDEMPTY_REG,
    &BBH_TX_LAN_CONFIGURATIONS_TXTHRESH_REG,
    &BBH_TX_LAN_CONFIGURATIONS_EEE_REG,
    &BBH_TX_LAN_CONFIGURATIONS_TS_REG,
    &BBH_TX_UNIFIED_CONFIGURATIONS_PDBASE_REG,
    &BBH_TX_UNIFIED_CONFIGURATIONS_PDSIZE_REG,
    &BBH_TX_UNIFIED_CONFIGURATIONS_PDWKUPH_REG,
    &BBH_TX_UNIFIED_CONFIGURATIONS_PD_BYTE_TH_REG,
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
};

unsigned long BBH_TX_ADDRS[] =
{
    0x82d90000,
    0x80170000,
    0x80172000,
    0x80174000,
    0x80176000,
};

const ru_block_rec BBH_TX_BLOCK = 
{
    "BBH_TX",
    BBH_TX_ADDRS,
    5,
    79,
    BBH_TX_REGS
};

/* End of file XRDP_BBH_TX.c */
