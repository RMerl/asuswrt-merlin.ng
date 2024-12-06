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


#include "XRDP_BBH_TX_AG.h"

/******************************************************************************
 * Register: NAME: BBH_TX_COMMON_CONFIGURATIONS_MACTYPE, TYPE: Type_BBH_TX_BBHTX_COMMON_CONFIGURATIONS_MACTYPE
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: TYPE *****/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_MACTYPE_TYPE_FIELD =
{
    "TYPE",
#if RU_INCLUDE_DESC
    "",
    "MAC type\n",
#endif
    { BBH_TX_COMMON_CONFIGURATIONS_MACTYPE_TYPE_FIELD_MASK },
    0,
    { BBH_TX_COMMON_CONFIGURATIONS_MACTYPE_TYPE_FIELD_WIDTH },
    { BBH_TX_COMMON_CONFIGURATIONS_MACTYPE_TYPE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_COMMON_CONFIGURATIONS_MACTYPE_FIELDS[] =
{
    &BBH_TX_COMMON_CONFIGURATIONS_MACTYPE_TYPE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_TX_COMMON_CONFIGURATIONS_MACTYPE *****/
const ru_reg_rec BBH_TX_COMMON_CONFIGURATIONS_MACTYPE_REG =
{
    "COMMON_CONFIGURATIONS_MACTYPE",
#if RU_INCLUDE_DESC
    "MAC_TYPE Register",
    "The BBH supports working with different MAC types. Each MAC requires different interface and features. This register defines the type of MAC the BBH works with.\n",
#endif
    { BBH_TX_COMMON_CONFIGURATIONS_MACTYPE_REG_OFFSET },
    0,
    0,
    99,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_TX_COMMON_CONFIGURATIONS_MACTYPE_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_TX_COMMON_CONFIGURATIONS_BBCFG_1_TX, TYPE: Type_BBH_TX_BBHTX_COMMON_CONFIGURATIONS_BBCFG_1_TX
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DMASRC *****/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_BBCFG_1_TX_DMASRC_FIELD =
{
    "DMASRC",
#if RU_INCLUDE_DESC
    "",
    "source id. This id is used to determine the route to the module.\n",
#endif
    { BBH_TX_COMMON_CONFIGURATIONS_BBCFG_1_TX_DMASRC_FIELD_MASK },
    0,
    { BBH_TX_COMMON_CONFIGURATIONS_BBCFG_1_TX_DMASRC_FIELD_WIDTH },
    { BBH_TX_COMMON_CONFIGURATIONS_BBCFG_1_TX_DMASRC_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: SDMASRC *****/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_BBCFG_1_TX_SDMASRC_FIELD =
{
    "SDMASRC",
#if RU_INCLUDE_DESC
    "",
    "source id. This id is used to determine the route to the module.\n",
#endif
    { BBH_TX_COMMON_CONFIGURATIONS_BBCFG_1_TX_SDMASRC_FIELD_MASK },
    0,
    { BBH_TX_COMMON_CONFIGURATIONS_BBCFG_1_TX_SDMASRC_FIELD_WIDTH },
    { BBH_TX_COMMON_CONFIGURATIONS_BBCFG_1_TX_SDMASRC_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: SBPMSRC *****/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_BBCFG_1_TX_SBPMSRC_FIELD =
{
    "SBPMSRC",
#if RU_INCLUDE_DESC
    "",
    "source id. This id is used to determine the route to the module.\n",
#endif
    { BBH_TX_COMMON_CONFIGURATIONS_BBCFG_1_TX_SBPMSRC_FIELD_MASK },
    0,
    { BBH_TX_COMMON_CONFIGURATIONS_BBCFG_1_TX_SBPMSRC_FIELD_WIDTH },
    { BBH_TX_COMMON_CONFIGURATIONS_BBCFG_1_TX_SBPMSRC_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FPMSRC *****/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_BBCFG_1_TX_FPMSRC_FIELD =
{
    "FPMSRC",
#if RU_INCLUDE_DESC
    "",
    "source id. This id is used to determine the route to the module.\n",
#endif
    { BBH_TX_COMMON_CONFIGURATIONS_BBCFG_1_TX_FPMSRC_FIELD_MASK },
    0,
    { BBH_TX_COMMON_CONFIGURATIONS_BBCFG_1_TX_FPMSRC_FIELD_WIDTH },
    { BBH_TX_COMMON_CONFIGURATIONS_BBCFG_1_TX_FPMSRC_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_COMMON_CONFIGURATIONS_BBCFG_1_TX_FIELDS[] =
{
    &BBH_TX_COMMON_CONFIGURATIONS_BBCFG_1_TX_DMASRC_FIELD,
    &BBH_TX_COMMON_CONFIGURATIONS_BBCFG_1_TX_SDMASRC_FIELD,
    &BBH_TX_COMMON_CONFIGURATIONS_BBCFG_1_TX_SBPMSRC_FIELD,
    &BBH_TX_COMMON_CONFIGURATIONS_BBCFG_1_TX_FPMSRC_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_TX_COMMON_CONFIGURATIONS_BBCFG_1_TX *****/
const ru_reg_rec BBH_TX_COMMON_CONFIGURATIONS_BBCFG_1_TX_REG =
{
    "COMMON_CONFIGURATIONS_BBCFG_1_TX",
#if RU_INCLUDE_DESC
    "BB_CFG_1 Register",
    "Each BBH unit has its own position on the BB tree. This position defines the Route address when approaching the Runner, S/DMA or S/BPM. The route is determined by a dedicated generic logic which uses the source id of the destination.\n",
#endif
    { BBH_TX_COMMON_CONFIGURATIONS_BBCFG_1_TX_REG_OFFSET },
    0,
    0,
    100,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    BBH_TX_COMMON_CONFIGURATIONS_BBCFG_1_TX_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_TX_COMMON_CONFIGURATIONS_BBCFG_2_TX, TYPE: Type_BBH_TX_BBHTX_COMMON_CONFIGURATIONS_BBCFG_2_TX
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: PDRNR0SRC *****/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_BBCFG_2_TX_PDRNR0SRC_FIELD =
{
    "PDRNR0SRC",
#if RU_INCLUDE_DESC
    "",
    "source id. This id is used to determine the route to the 1st (out of possible 2 runners) which are responsible for sending PDs.\n",
#endif
    { BBH_TX_COMMON_CONFIGURATIONS_BBCFG_2_TX_PDRNR0SRC_FIELD_MASK },
    0,
    { BBH_TX_COMMON_CONFIGURATIONS_BBCFG_2_TX_PDRNR0SRC_FIELD_WIDTH },
    { BBH_TX_COMMON_CONFIGURATIONS_BBCFG_2_TX_PDRNR0SRC_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: PDRNR1SRC *****/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_BBCFG_2_TX_PDRNR1SRC_FIELD =
{
    "PDRNR1SRC",
#if RU_INCLUDE_DESC
    "",
    "source id. This id is used to determine the route to the 2nd (out of possible 2 runners) which are responsible for sending PDs.\n",
#endif
    { BBH_TX_COMMON_CONFIGURATIONS_BBCFG_2_TX_PDRNR1SRC_FIELD_MASK },
    0,
    { BBH_TX_COMMON_CONFIGURATIONS_BBCFG_2_TX_PDRNR1SRC_FIELD_WIDTH },
    { BBH_TX_COMMON_CONFIGURATIONS_BBCFG_2_TX_PDRNR1SRC_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: PDRNR2SRC *****/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_BBCFG_2_TX_PDRNR2SRC_FIELD =
{
    "PDRNR2SRC",
#if RU_INCLUDE_DESC
    "",
    "source id. This id is used to determine the route to the 3rd (out of possible 4 runners) which are responsible for sending PDs.\n",
#endif
    { BBH_TX_COMMON_CONFIGURATIONS_BBCFG_2_TX_PDRNR2SRC_FIELD_MASK },
    0,
    { BBH_TX_COMMON_CONFIGURATIONS_BBCFG_2_TX_PDRNR2SRC_FIELD_WIDTH },
    { BBH_TX_COMMON_CONFIGURATIONS_BBCFG_2_TX_PDRNR2SRC_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: PDRNR3SRC *****/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_BBCFG_2_TX_PDRNR3SRC_FIELD =
{
    "PDRNR3SRC",
#if RU_INCLUDE_DESC
    "",
    "source id. This id is used to determine the route to the 4th (out of possible 4 runners) which are responsible for sending PDs.\n",
#endif
    { BBH_TX_COMMON_CONFIGURATIONS_BBCFG_2_TX_PDRNR3SRC_FIELD_MASK },
    0,
    { BBH_TX_COMMON_CONFIGURATIONS_BBCFG_2_TX_PDRNR3SRC_FIELD_WIDTH },
    { BBH_TX_COMMON_CONFIGURATIONS_BBCFG_2_TX_PDRNR3SRC_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_COMMON_CONFIGURATIONS_BBCFG_2_TX_FIELDS[] =
{
    &BBH_TX_COMMON_CONFIGURATIONS_BBCFG_2_TX_PDRNR0SRC_FIELD,
    &BBH_TX_COMMON_CONFIGURATIONS_BBCFG_2_TX_PDRNR1SRC_FIELD,
    &BBH_TX_COMMON_CONFIGURATIONS_BBCFG_2_TX_PDRNR2SRC_FIELD,
    &BBH_TX_COMMON_CONFIGURATIONS_BBCFG_2_TX_PDRNR3SRC_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_TX_COMMON_CONFIGURATIONS_BBCFG_2_TX *****/
const ru_reg_rec BBH_TX_COMMON_CONFIGURATIONS_BBCFG_2_TX_REG =
{
    "COMMON_CONFIGURATIONS_BBCFG_2_TX",
#if RU_INCLUDE_DESC
    "BB_CFG_2 Register",
    "Each BBH unit has its own position on the BB tree. This position defines the Route address when approaching the Runner, S/DMA or S/BPM. The route is determined by a dedicated generic logic which uses the source id of the destination.\n",
#endif
    { BBH_TX_COMMON_CONFIGURATIONS_BBCFG_2_TX_REG_OFFSET },
    0,
    0,
    101,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    BBH_TX_COMMON_CONFIGURATIONS_BBCFG_2_TX_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_TX_COMMON_CONFIGURATIONS_BBCFG_3_TX, TYPE: Type_BBH_TX_BBHTX_COMMON_CONFIGURATIONS_BBCFG_3_TX
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: MSGRNRSRC *****/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_BBCFG_3_TX_MSGRNRSRC_FIELD =
{
    "MSGRNRSRC",
#if RU_INCLUDE_DESC
    "",
    "source id. This id is used to determine the route to the Runner which is responsible for sending DBR/Ghost messages (WAN only).\n",
#endif
    { BBH_TX_COMMON_CONFIGURATIONS_BBCFG_3_TX_MSGRNRSRC_FIELD_MASK },
    0,
    { BBH_TX_COMMON_CONFIGURATIONS_BBCFG_3_TX_MSGRNRSRC_FIELD_WIDTH },
    { BBH_TX_COMMON_CONFIGURATIONS_BBCFG_3_TX_MSGRNRSRC_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: STSRNRSRC *****/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_BBCFG_3_TX_STSRNRSRC_FIELD =
{
    "STSRNRSRC",
#if RU_INCLUDE_DESC
    "",
    "source id. This id is used to determine the route to the Runner that is responsible for sending status messages (WAN only).\n",
#endif
    { BBH_TX_COMMON_CONFIGURATIONS_BBCFG_3_TX_STSRNRSRC_FIELD_MASK },
    0,
    { BBH_TX_COMMON_CONFIGURATIONS_BBCFG_3_TX_STSRNRSRC_FIELD_WIDTH },
    { BBH_TX_COMMON_CONFIGURATIONS_BBCFG_3_TX_STSRNRSRC_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_COMMON_CONFIGURATIONS_BBCFG_3_TX_FIELDS[] =
{
    &BBH_TX_COMMON_CONFIGURATIONS_BBCFG_3_TX_MSGRNRSRC_FIELD,
    &BBH_TX_COMMON_CONFIGURATIONS_BBCFG_3_TX_STSRNRSRC_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_TX_COMMON_CONFIGURATIONS_BBCFG_3_TX *****/
const ru_reg_rec BBH_TX_COMMON_CONFIGURATIONS_BBCFG_3_TX_REG =
{
    "COMMON_CONFIGURATIONS_BBCFG_3_TX",
#if RU_INCLUDE_DESC
    "BB_CFG_3 Register",
    "Each BBH unit has its own position on the BB tree. This position defines the Route address when approaching the Runner, S/DMA or S/BPM. The route is determined by a dedicated generic logic which uses the source id of the destination.\n",
#endif
    { BBH_TX_COMMON_CONFIGURATIONS_BBCFG_3_TX_REG_OFFSET },
    0,
    0,
    102,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    BBH_TX_COMMON_CONFIGURATIONS_BBCFG_3_TX_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_TX_COMMON_CONFIGURATIONS_DMACFG_TX, TYPE: Type_BBH_TX_BBHTX_COMMON_CONFIGURATIONS_DMACFG_TX
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DESCBASE *****/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_DMACFG_TX_DESCBASE_FIELD =
{
    "DESCBASE",
#if RU_INCLUDE_DESC
    "",
    "Defines the base address of the read request FIFO within the DMA address space.\nThe value should be identical to the relevant configuration in the DMA.\n",
#endif
    { BBH_TX_COMMON_CONFIGURATIONS_DMACFG_TX_DESCBASE_FIELD_MASK },
    0,
    { BBH_TX_COMMON_CONFIGURATIONS_DMACFG_TX_DESCBASE_FIELD_WIDTH },
    { BBH_TX_COMMON_CONFIGURATIONS_DMACFG_TX_DESCBASE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DESCSIZE *****/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_DMACFG_TX_DESCSIZE_FIELD =
{
    "DESCSIZE",
#if RU_INCLUDE_DESC
    "",
    "The size of the BBH read requests FIFO inside the DMA\n",
#endif
    { BBH_TX_COMMON_CONFIGURATIONS_DMACFG_TX_DESCSIZE_FIELD_MASK },
    0,
    { BBH_TX_COMMON_CONFIGURATIONS_DMACFG_TX_DESCSIZE_FIELD_WIDTH },
    { BBH_TX_COMMON_CONFIGURATIONS_DMACFG_TX_DESCSIZE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: MAXREQ *****/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_DMACFG_TX_MAXREQ_FIELD =
{
    "MAXREQ",
#if RU_INCLUDE_DESC
    "",
    "Defines the maximum allowed number of on-the-fly read requests.\n",
#endif
    { BBH_TX_COMMON_CONFIGURATIONS_DMACFG_TX_MAXREQ_FIELD_MASK },
    0,
    { BBH_TX_COMMON_CONFIGURATIONS_DMACFG_TX_MAXREQ_FIELD_WIDTH },
    { BBH_TX_COMMON_CONFIGURATIONS_DMACFG_TX_MAXREQ_FIELD_SHIFT },
    8,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: EPNURGNT *****/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_DMACFG_TX_EPNURGNT_FIELD =
{
    "EPNURGNT",
#if RU_INCLUDE_DESC
    "",
    "When asserted, this bit forces urgent priority on the EPON read requests towards the DMA (relevant only for EPON BBH)\n",
#endif
    { BBH_TX_COMMON_CONFIGURATIONS_DMACFG_TX_EPNURGNT_FIELD_MASK },
    0,
    { BBH_TX_COMMON_CONFIGURATIONS_DMACFG_TX_EPNURGNT_FIELD_WIDTH },
    { BBH_TX_COMMON_CONFIGURATIONS_DMACFG_TX_EPNURGNT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: JUMBOURGNT *****/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_DMACFG_TX_JUMBOURGNT_FIELD =
{
    "JUMBOURGNT",
#if RU_INCLUDE_DESC
    "",
    "When asserted, this bit forces urgent priority on read requests of a jumbo packet (>2K)\n",
#endif
    { BBH_TX_COMMON_CONFIGURATIONS_DMACFG_TX_JUMBOURGNT_FIELD_MASK },
    0,
    { BBH_TX_COMMON_CONFIGURATIONS_DMACFG_TX_JUMBOURGNT_FIELD_WIDTH },
    { BBH_TX_COMMON_CONFIGURATIONS_DMACFG_TX_JUMBOURGNT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_COMMON_CONFIGURATIONS_DMACFG_TX_FIELDS[] =
{
    &BBH_TX_COMMON_CONFIGURATIONS_DMACFG_TX_DESCBASE_FIELD,
    &BBH_TX_COMMON_CONFIGURATIONS_DMACFG_TX_DESCSIZE_FIELD,
    &BBH_TX_COMMON_CONFIGURATIONS_DMACFG_TX_MAXREQ_FIELD,
    &BBH_TX_COMMON_CONFIGURATIONS_DMACFG_TX_EPNURGNT_FIELD,
    &BBH_TX_COMMON_CONFIGURATIONS_DMACFG_TX_JUMBOURGNT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_TX_COMMON_CONFIGURATIONS_DMACFG_TX *****/
const ru_reg_rec BBH_TX_COMMON_CONFIGURATIONS_DMACFG_TX_REG =
{
    "COMMON_CONFIGURATIONS_DMACFG_TX",
#if RU_INCLUDE_DESC
    "DMA_CFG Register",
    "The BBH reads the packet data from the DDR in chunks (with a maximal size of 128 bytes).\nFor each chunk the BBH writes a read request (descriptor) into the DMA memory space. The read descriptors are arranged in a predefined space in the DMA memory and managed in a cyclic FIFO style.\nA special configuration limits the maximum number of read requests.\n",
#endif
    { BBH_TX_COMMON_CONFIGURATIONS_DMACFG_TX_REG_OFFSET },
    0,
    0,
    103,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    5,
    BBH_TX_COMMON_CONFIGURATIONS_DMACFG_TX_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_TX_COMMON_CONFIGURATIONS_SDMACFG_TX, TYPE: Type_BBH_TX_BBHTX_COMMON_CONFIGURATIONS_SDMACFG_TX
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DESCBASE *****/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_SDMACFG_TX_DESCBASE_FIELD =
{
    "DESCBASE",
#if RU_INCLUDE_DESC
    "",
    "Defines the base address of the read request FIFO within the DMA address space.\nThe value should be identical to the relevant configuration in the DMA.\n",
#endif
    { BBH_TX_COMMON_CONFIGURATIONS_SDMACFG_TX_DESCBASE_FIELD_MASK },
    0,
    { BBH_TX_COMMON_CONFIGURATIONS_SDMACFG_TX_DESCBASE_FIELD_WIDTH },
    { BBH_TX_COMMON_CONFIGURATIONS_SDMACFG_TX_DESCBASE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DESCSIZE *****/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_SDMACFG_TX_DESCSIZE_FIELD =
{
    "DESCSIZE",
#if RU_INCLUDE_DESC
    "",
    "The size of the BBH read requests FIFO inside the DMA\n",
#endif
    { BBH_TX_COMMON_CONFIGURATIONS_SDMACFG_TX_DESCSIZE_FIELD_MASK },
    0,
    { BBH_TX_COMMON_CONFIGURATIONS_SDMACFG_TX_DESCSIZE_FIELD_WIDTH },
    { BBH_TX_COMMON_CONFIGURATIONS_SDMACFG_TX_DESCSIZE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: MAXREQ *****/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_SDMACFG_TX_MAXREQ_FIELD =
{
    "MAXREQ",
#if RU_INCLUDE_DESC
    "",
    "Defines the maximum allowed number of on-the-fly read requests.\n",
#endif
    { BBH_TX_COMMON_CONFIGURATIONS_SDMACFG_TX_MAXREQ_FIELD_MASK },
    0,
    { BBH_TX_COMMON_CONFIGURATIONS_SDMACFG_TX_MAXREQ_FIELD_WIDTH },
    { BBH_TX_COMMON_CONFIGURATIONS_SDMACFG_TX_MAXREQ_FIELD_SHIFT },
    8,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: EPNURGNT *****/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_SDMACFG_TX_EPNURGNT_FIELD =
{
    "EPNURGNT",
#if RU_INCLUDE_DESC
    "",
    "When asserted, this bit forces urgent priority on the EPON read requests towards the DMA (relevant only for EPON BBH)\n",
#endif
    { BBH_TX_COMMON_CONFIGURATIONS_SDMACFG_TX_EPNURGNT_FIELD_MASK },
    0,
    { BBH_TX_COMMON_CONFIGURATIONS_SDMACFG_TX_EPNURGNT_FIELD_WIDTH },
    { BBH_TX_COMMON_CONFIGURATIONS_SDMACFG_TX_EPNURGNT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: JUMBOURGNT *****/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_SDMACFG_TX_JUMBOURGNT_FIELD =
{
    "JUMBOURGNT",
#if RU_INCLUDE_DESC
    "",
    "When asserted, this bit forces urgent priority on Jumbo packets (>2k) read requests\n",
#endif
    { BBH_TX_COMMON_CONFIGURATIONS_SDMACFG_TX_JUMBOURGNT_FIELD_MASK },
    0,
    { BBH_TX_COMMON_CONFIGURATIONS_SDMACFG_TX_JUMBOURGNT_FIELD_WIDTH },
    { BBH_TX_COMMON_CONFIGURATIONS_SDMACFG_TX_JUMBOURGNT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_COMMON_CONFIGURATIONS_SDMACFG_TX_FIELDS[] =
{
    &BBH_TX_COMMON_CONFIGURATIONS_SDMACFG_TX_DESCBASE_FIELD,
    &BBH_TX_COMMON_CONFIGURATIONS_SDMACFG_TX_DESCSIZE_FIELD,
    &BBH_TX_COMMON_CONFIGURATIONS_SDMACFG_TX_MAXREQ_FIELD,
    &BBH_TX_COMMON_CONFIGURATIONS_SDMACFG_TX_EPNURGNT_FIELD,
    &BBH_TX_COMMON_CONFIGURATIONS_SDMACFG_TX_JUMBOURGNT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_TX_COMMON_CONFIGURATIONS_SDMACFG_TX *****/
const ru_reg_rec BBH_TX_COMMON_CONFIGURATIONS_SDMACFG_TX_REG =
{
    "COMMON_CONFIGURATIONS_SDMACFG_TX",
#if RU_INCLUDE_DESC
    "SDMA_CFG Register",
    "The BBH reads the packet data from the PSRAM in chunks (with a maximal size of 128 bytes).\nFor each chunk the BBH writes a read request (descriptor) into the SDMA memory space. The read descriptors are arranged in a predefined space in the SDMA memory and managed in a cyclic FIFO style.\nA special configuration limits the maximum number of read requests.\n",
#endif
    { BBH_TX_COMMON_CONFIGURATIONS_SDMACFG_TX_REG_OFFSET },
    0,
    0,
    104,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    5,
    BBH_TX_COMMON_CONFIGURATIONS_SDMACFG_TX_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_TX_COMMON_CONFIGURATIONS_SBPMCFG, TYPE: Type_BBH_TX_BBHTX_COMMON_CONFIGURATIONS_SBPMCFG
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: FREENOCNTXT *****/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_SBPMCFG_FREENOCNTXT_FIELD =
{
    "FREENOCNTXT",
#if RU_INCLUDE_DESC
    "",
    "When this bit is enabled, the BBH will use free without context command.\n",
#endif
    { BBH_TX_COMMON_CONFIGURATIONS_SBPMCFG_FREENOCNTXT_FIELD_MASK },
    0,
    { BBH_TX_COMMON_CONFIGURATIONS_SBPMCFG_FREENOCNTXT_FIELD_WIDTH },
    { BBH_TX_COMMON_CONFIGURATIONS_SBPMCFG_FREENOCNTXT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: SPECIALFREE *****/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_SBPMCFG_SPECIALFREE_FIELD =
{
    "SPECIALFREE",
#if RU_INCLUDE_DESC
    "",
    "When this bit is enabled, the BBH will use special free with context command.\nThis bit is relevant only if free without context_en is configured to 0.\n",
#endif
    { BBH_TX_COMMON_CONFIGURATIONS_SBPMCFG_SPECIALFREE_FIELD_MASK },
    0,
    { BBH_TX_COMMON_CONFIGURATIONS_SBPMCFG_SPECIALFREE_FIELD_WIDTH },
    { BBH_TX_COMMON_CONFIGURATIONS_SBPMCFG_SPECIALFREE_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: SECOND_BN_DIS *****/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_SBPMCFG_SECOND_BN_DIS_FIELD =
{
    "SECOND_BN_DIS",
#if RU_INCLUDE_DESC
    "",
    "The bbh tx uses the second BN from the PD if the packet is 2 buffers long. This bit disables this optimization, so the BBH tx will always use get next command to understand what is the next BN\n",
#endif
    { BBH_TX_COMMON_CONFIGURATIONS_SBPMCFG_SECOND_BN_DIS_FIELD_MASK },
    0,
    { BBH_TX_COMMON_CONFIGURATIONS_SBPMCFG_SECOND_BN_DIS_FIELD_WIDTH },
    { BBH_TX_COMMON_CONFIGURATIONS_SBPMCFG_SECOND_BN_DIS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: SECOND_BN_LEN_MIS_DIS *****/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_SBPMCFG_SECOND_BN_LEN_MIS_DIS_FIELD =
{
    "SECOND_BN_LEN_MIS_DIS",
#if RU_INCLUDE_DESC
    "",
    "The bbh tx uses the second BN from the PD if the packet is 2 buffers long. This bit disables this optimization in case there is a mismatch between the PLEN and the number of SBNs in the PD, so the BBH tx will use get next command to understand what is the next BN in this case\n",
#endif
    { BBH_TX_COMMON_CONFIGURATIONS_SBPMCFG_SECOND_BN_LEN_MIS_DIS_FIELD_MASK },
    0,
    { BBH_TX_COMMON_CONFIGURATIONS_SBPMCFG_SECOND_BN_LEN_MIS_DIS_FIELD_WIDTH },
    { BBH_TX_COMMON_CONFIGURATIONS_SBPMCFG_SECOND_BN_LEN_MIS_DIS_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: USE_SECOND_BN_FROM_PD_IN_FREE *****/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_SBPMCFG_USE_SECOND_BN_FROM_PD_IN_FREE_FIELD =
{
    "USE_SECOND_BN_FROM_PD_IN_FREE",
#if RU_INCLUDE_DESC
    "",
    "When freeing a packet that is located in SRAM, the BBH uses the last SBN he got from issuing get next commands for the free command.\nWhen this bit is asserted, the BBH will not use the last SBN from the get-next, but the one from the PD.\n",
#endif
    { BBH_TX_COMMON_CONFIGURATIONS_SBPMCFG_USE_SECOND_BN_FROM_PD_IN_FREE_FIELD_MASK },
    0,
    { BBH_TX_COMMON_CONFIGURATIONS_SBPMCFG_USE_SECOND_BN_FROM_PD_IN_FREE_FIELD_WIDTH },
    { BBH_TX_COMMON_CONFIGURATIONS_SBPMCFG_USE_SECOND_BN_FROM_PD_IN_FREE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: USE_SECOND_BN_FROM_PD_IN_FREE_LEN_MIS *****/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_SBPMCFG_USE_SECOND_BN_FROM_PD_IN_FREE_LEN_MIS_FIELD =
{
    "USE_SECOND_BN_FROM_PD_IN_FREE_LEN_MIS",
#if RU_INCLUDE_DESC
    "",
    "When freeing a packet that is located in SRAM, the BBH uses the last SBN he got from issuing get next commands for the free command.\nWhen this bit is asserted and there is a mismatch between the number of SBNs and the PLEN in the PD, and the BBH is not configured to use free without context, the BBH will not use the last SBN from the get-next, but the one from the PD.\n",
#endif
    { BBH_TX_COMMON_CONFIGURATIONS_SBPMCFG_USE_SECOND_BN_FROM_PD_IN_FREE_LEN_MIS_FIELD_MASK },
    0,
    { BBH_TX_COMMON_CONFIGURATIONS_SBPMCFG_USE_SECOND_BN_FROM_PD_IN_FREE_LEN_MIS_FIELD_WIDTH },
    { BBH_TX_COMMON_CONFIGURATIONS_SBPMCFG_USE_SECOND_BN_FROM_PD_IN_FREE_LEN_MIS_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: USE_FREE_WITHOUT_CNTXT_LEN_MIS *****/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_SBPMCFG_USE_FREE_WITHOUT_CNTXT_LEN_MIS_FIELD =
{
    "USE_FREE_WITHOUT_CNTXT_LEN_MIS",
#if RU_INCLUDE_DESC
    "",
    "When freeing a packet that is located in SRAM, the BBH uses the last SBN he got from issuing get next commands for the free command.\nWhen this bit is asserted and there is a length mismatch between the PLEN and the number of SBNs in the PD, the BBH will not use the last SBN, but will send free without context command to the SBPM\n",
#endif
    { BBH_TX_COMMON_CONFIGURATIONS_SBPMCFG_USE_FREE_WITHOUT_CNTXT_LEN_MIS_FIELD_MASK },
    0,
    { BBH_TX_COMMON_CONFIGURATIONS_SBPMCFG_USE_FREE_WITHOUT_CNTXT_LEN_MIS_FIELD_WIDTH },
    { BBH_TX_COMMON_CONFIGURATIONS_SBPMCFG_USE_FREE_WITHOUT_CNTXT_LEN_MIS_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: MAXGN *****/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_SBPMCFG_MAXGN_FIELD =
{
    "MAXGN",
#if RU_INCLUDE_DESC
    "",
    "maximum number of pending on the fly get next commands\n",
#endif
    { BBH_TX_COMMON_CONFIGURATIONS_SBPMCFG_MAXGN_FIELD_MASK },
    0,
    { BBH_TX_COMMON_CONFIGURATIONS_SBPMCFG_MAXGN_FIELD_WIDTH },
    { BBH_TX_COMMON_CONFIGURATIONS_SBPMCFG_MAXGN_FIELD_SHIFT },
    12,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_COMMON_CONFIGURATIONS_SBPMCFG_FIELDS[] =
{
    &BBH_TX_COMMON_CONFIGURATIONS_SBPMCFG_FREENOCNTXT_FIELD,
    &BBH_TX_COMMON_CONFIGURATIONS_SBPMCFG_SPECIALFREE_FIELD,
    &BBH_TX_COMMON_CONFIGURATIONS_SBPMCFG_SECOND_BN_DIS_FIELD,
    &BBH_TX_COMMON_CONFIGURATIONS_SBPMCFG_SECOND_BN_LEN_MIS_DIS_FIELD,
    &BBH_TX_COMMON_CONFIGURATIONS_SBPMCFG_USE_SECOND_BN_FROM_PD_IN_FREE_FIELD,
    &BBH_TX_COMMON_CONFIGURATIONS_SBPMCFG_USE_SECOND_BN_FROM_PD_IN_FREE_LEN_MIS_FIELD,
    &BBH_TX_COMMON_CONFIGURATIONS_SBPMCFG_USE_FREE_WITHOUT_CNTXT_LEN_MIS_FIELD,
    &BBH_TX_COMMON_CONFIGURATIONS_SBPMCFG_MAXGN_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_TX_COMMON_CONFIGURATIONS_SBPMCFG *****/
const ru_reg_rec BBH_TX_COMMON_CONFIGURATIONS_SBPMCFG_REG =
{
    "COMMON_CONFIGURATIONS_SBPMCFG",
#if RU_INCLUDE_DESC
    "SBPM_CFG Register",
    "When packet transmission is done, the BBH releases the SBPM buffers.\nThis register defines which release command is used:\n1. Normal free with context\n2. Special free with context\n3. free without context\n",
#endif
    { BBH_TX_COMMON_CONFIGURATIONS_SBPMCFG_REG_OFFSET },
    0,
    0,
    105,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    8,
    BBH_TX_COMMON_CONFIGURATIONS_SBPMCFG_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_TX_COMMON_CONFIGURATIONS_DDRTMBASEL, TYPE: Type_BBH_TX_BBHTX_COMMON_CONFIGURATIONS_DDRTMBASEL
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DDRTMBASE *****/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_DDRTMBASEL_DDRTMBASE_FIELD =
{
    "DDRTMBASE",
#if RU_INCLUDE_DESC
    "",
    "DDR TM base.\nThe address is in bytes resolution.\nThe address should be aligned to 128 bytes.\n",
#endif
    { BBH_TX_COMMON_CONFIGURATIONS_DDRTMBASEL_DDRTMBASE_FIELD_MASK },
    0,
    { BBH_TX_COMMON_CONFIGURATIONS_DDRTMBASEL_DDRTMBASE_FIELD_WIDTH },
    { BBH_TX_COMMON_CONFIGURATIONS_DDRTMBASEL_DDRTMBASE_FIELD_SHIFT },
    8388608,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_COMMON_CONFIGURATIONS_DDRTMBASEL_FIELDS[] =
{
    &BBH_TX_COMMON_CONFIGURATIONS_DDRTMBASEL_DDRTMBASE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_TX_COMMON_CONFIGURATIONS_DDRTMBASEL *****/
const ru_reg_rec BBH_TX_COMMON_CONFIGURATIONS_DDRTMBASEL_REG =
{
    "COMMON_CONFIGURATIONS_DDRTMBASEL",
#if RU_INCLUDE_DESC
    "DDR_TM_BASE_LOW 0..1 Register",
    "The BBH calculate the DDR physical address according to the Buffer number and buffer size and then adds the DDR TM base.\n\nThe DDR TM address space is divided to two - coherent and non coherent.\n\nThe first register in this array defines the base address of the non coherent space and the second is for the coherent.\n\nThe value of this register should match the relevant registers value in the BBH RX, QM and the Runner.\n\n",
#endif
    { BBH_TX_COMMON_CONFIGURATIONS_DDRTMBASEL_REG_OFFSET },
    BBH_TX_COMMON_CONFIGURATIONS_DDRTMBASEL_REG_RAM_CNT,
    4,
    106,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_TX_COMMON_CONFIGURATIONS_DDRTMBASEL_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_TX_COMMON_CONFIGURATIONS_DDRTMBASEH, TYPE: Type_BBH_TX_BBHTX_COMMON_CONFIGURATIONS_DDRTMBASEH
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DDRTMBASE *****/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_DDRTMBASEH_DDRTMBASE_FIELD =
{
    "DDRTMBASE",
#if RU_INCLUDE_DESC
    "",
    "MSB of DDR TM base.\n\n",
#endif
    { BBH_TX_COMMON_CONFIGURATIONS_DDRTMBASEH_DDRTMBASE_FIELD_MASK },
    0,
    { BBH_TX_COMMON_CONFIGURATIONS_DDRTMBASEH_DDRTMBASE_FIELD_WIDTH },
    { BBH_TX_COMMON_CONFIGURATIONS_DDRTMBASEH_DDRTMBASE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_COMMON_CONFIGURATIONS_DDRTMBASEH_FIELDS[] =
{
    &BBH_TX_COMMON_CONFIGURATIONS_DDRTMBASEH_DDRTMBASE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_TX_COMMON_CONFIGURATIONS_DDRTMBASEH *****/
const ru_reg_rec BBH_TX_COMMON_CONFIGURATIONS_DDRTMBASEH_REG =
{
    "COMMON_CONFIGURATIONS_DDRTMBASEH",
#if RU_INCLUDE_DESC
    "DDR_TM_BASE_HIGH 0..1 Register",
    "The BBH calculate the DDR physical address according to the Buffer number and buffer size and then adds the DDR TM base.\n\nThe DDR TM address space is divided to two - coherent and non coherent.\n\nThe first register in this array defines the base address of the non coherent space and the second is for the coherent.\n\nThe value of this register should match the relevant registers value in the BBH RX, QM and the Runner.\n\n",
#endif
    { BBH_TX_COMMON_CONFIGURATIONS_DDRTMBASEH_REG_OFFSET },
    BBH_TX_COMMON_CONFIGURATIONS_DDRTMBASEH_REG_RAM_CNT,
    4,
    107,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_TX_COMMON_CONFIGURATIONS_DDRTMBASEH_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_TX_COMMON_CONFIGURATIONS_DFIFOCTRL, TYPE: Type_BBH_TX_BBHTX_COMMON_CONFIGURATIONS_DFIFOCTRL
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: PSRAMSIZE *****/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_DFIFOCTRL_PSRAMSIZE_FIELD =
{
    "PSRAMSIZE",
#if RU_INCLUDE_DESC
    "",
    "The size of the PSRAM data FIFO in 8 bytes resolution. The BBH uses this information for determining the amount of data that can be ordered from the PSRAM.\n",
#endif
    { BBH_TX_COMMON_CONFIGURATIONS_DFIFOCTRL_PSRAMSIZE_FIELD_MASK },
    0,
    { BBH_TX_COMMON_CONFIGURATIONS_DFIFOCTRL_PSRAMSIZE_FIELD_WIDTH },
    { BBH_TX_COMMON_CONFIGURATIONS_DFIFOCTRL_PSRAMSIZE_FIELD_SHIFT },
    64,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DDRSIZE *****/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_DFIFOCTRL_DDRSIZE_FIELD =
{
    "DDRSIZE",
#if RU_INCLUDE_DESC
    "",
    "The size of the DDR data FIFO in 8 bytes resolution. The BBH uses this information for determining the amount of data that can be ordered from the DDR.\n",
#endif
    { BBH_TX_COMMON_CONFIGURATIONS_DFIFOCTRL_DDRSIZE_FIELD_MASK },
    0,
    { BBH_TX_COMMON_CONFIGURATIONS_DFIFOCTRL_DDRSIZE_FIELD_WIDTH },
    { BBH_TX_COMMON_CONFIGURATIONS_DFIFOCTRL_DDRSIZE_FIELD_SHIFT },
    256,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: PSRAMBASE *****/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_DFIFOCTRL_PSRAMBASE_FIELD =
{
    "PSRAMBASE",
#if RU_INCLUDE_DESC
    "",
    "the base address of the PSRAM data FIFO in 8 bytes resolution. The DDR data FIFO base address is always 0.\nIn case the whole RAM is to be dedicated to PSRAM data, the base should be 0 as well, and the DDR FIFO size should be configured to 0.\n",
#endif
    { BBH_TX_COMMON_CONFIGURATIONS_DFIFOCTRL_PSRAMBASE_FIELD_MASK },
    0,
    { BBH_TX_COMMON_CONFIGURATIONS_DFIFOCTRL_PSRAMBASE_FIELD_WIDTH },
    { BBH_TX_COMMON_CONFIGURATIONS_DFIFOCTRL_PSRAMBASE_FIELD_SHIFT },
    256,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: REORDER_PER_Q_EN *****/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_DFIFOCTRL_REORDER_PER_Q_EN_FIELD =
{
    "REORDER_PER_Q_EN",
#if RU_INCLUDE_DESC
    "",
    "When asserted, the BBH TX will do reorder per q, meaning order between SRAM and DDR  pds willl be kept only per q.\n",
#endif
    { BBH_TX_COMMON_CONFIGURATIONS_DFIFOCTRL_REORDER_PER_Q_EN_FIELD_MASK },
    0,
    { BBH_TX_COMMON_CONFIGURATIONS_DFIFOCTRL_REORDER_PER_Q_EN_FIELD_WIDTH },
    { BBH_TX_COMMON_CONFIGURATIONS_DFIFOCTRL_REORDER_PER_Q_EN_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_COMMON_CONFIGURATIONS_DFIFOCTRL_FIELDS[] =
{
    &BBH_TX_COMMON_CONFIGURATIONS_DFIFOCTRL_PSRAMSIZE_FIELD,
    &BBH_TX_COMMON_CONFIGURATIONS_DFIFOCTRL_DDRSIZE_FIELD,
    &BBH_TX_COMMON_CONFIGURATIONS_DFIFOCTRL_PSRAMBASE_FIELD,
    &BBH_TX_COMMON_CONFIGURATIONS_DFIFOCTRL_REORDER_PER_Q_EN_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_TX_COMMON_CONFIGURATIONS_DFIFOCTRL *****/
const ru_reg_rec BBH_TX_COMMON_CONFIGURATIONS_DFIFOCTRL_REG =
{
    "COMMON_CONFIGURATIONS_DFIFOCTRL",
#if RU_INCLUDE_DESC
    "DATA_FIFO_CTRL Register",
    "The BBH orders data both from DDR and PSRAM. The returned data is stored in two FIFOs for reordering. The two FIFOs are implemented in a single RAM. This register defines the division of the RAM to two FIFOs.\n",
#endif
    { BBH_TX_COMMON_CONFIGURATIONS_DFIFOCTRL_REG_OFFSET },
    0,
    0,
    108,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    BBH_TX_COMMON_CONFIGURATIONS_DFIFOCTRL_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_TX_COMMON_CONFIGURATIONS_ARB_CFG, TYPE: Type_BBH_TX_BBHTX_COMMON_CONFIGURATIONS_ARB_CFG
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: HIGHTRXQ *****/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_ARB_CFG_HIGHTRXQ_FIELD =
{
    "HIGHTRXQ",
#if RU_INCLUDE_DESC
    "",
    "this configuration determines whether to give high priority to a current transmitting queue or not.\n",
#endif
    { BBH_TX_COMMON_CONFIGURATIONS_ARB_CFG_HIGHTRXQ_FIELD_MASK },
    0,
    { BBH_TX_COMMON_CONFIGURATIONS_ARB_CFG_HIGHTRXQ_FIELD_WIDTH },
    { BBH_TX_COMMON_CONFIGURATIONS_ARB_CFG_HIGHTRXQ_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_COMMON_CONFIGURATIONS_ARB_CFG_FIELDS[] =
{
    &BBH_TX_COMMON_CONFIGURATIONS_ARB_CFG_HIGHTRXQ_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_TX_COMMON_CONFIGURATIONS_ARB_CFG *****/
const ru_reg_rec BBH_TX_COMMON_CONFIGURATIONS_ARB_CFG_REG =
{
    "COMMON_CONFIGURATIONS_ARB_CFG",
#if RU_INCLUDE_DESC
    "ARB_CFG Register",
    "configurations related to different arbitration processes (ordering PDs, ordering data)\n",
#endif
    { BBH_TX_COMMON_CONFIGURATIONS_ARB_CFG_REG_OFFSET },
    0,
    0,
    109,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_TX_COMMON_CONFIGURATIONS_ARB_CFG_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_TX_COMMON_CONFIGURATIONS_BBROUTE, TYPE: Type_BBH_TX_BBHTX_COMMON_CONFIGURATIONS_BBROUTE
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: ROUTE *****/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_BBROUTE_ROUTE_FIELD =
{
    "ROUTE",
#if RU_INCLUDE_DESC
    "",
    "route address\n",
#endif
    { BBH_TX_COMMON_CONFIGURATIONS_BBROUTE_ROUTE_FIELD_MASK },
    0,
    { BBH_TX_COMMON_CONFIGURATIONS_BBROUTE_ROUTE_FIELD_WIDTH },
    { BBH_TX_COMMON_CONFIGURATIONS_BBROUTE_ROUTE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DEST *****/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_BBROUTE_DEST_FIELD =
{
    "DEST",
#if RU_INCLUDE_DESC
    "",
    "destination source id\n",
#endif
    { BBH_TX_COMMON_CONFIGURATIONS_BBROUTE_DEST_FIELD_MASK },
    0,
    { BBH_TX_COMMON_CONFIGURATIONS_BBROUTE_DEST_FIELD_WIDTH },
    { BBH_TX_COMMON_CONFIGURATIONS_BBROUTE_DEST_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: EN *****/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_BBROUTE_EN_FIELD =
{
    "EN",
#if RU_INCLUDE_DESC
    "",
    "enable\n",
#endif
    { BBH_TX_COMMON_CONFIGURATIONS_BBROUTE_EN_FIELD_MASK },
    0,
    { BBH_TX_COMMON_CONFIGURATIONS_BBROUTE_EN_FIELD_WIDTH },
    { BBH_TX_COMMON_CONFIGURATIONS_BBROUTE_EN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_COMMON_CONFIGURATIONS_BBROUTE_FIELDS[] =
{
    &BBH_TX_COMMON_CONFIGURATIONS_BBROUTE_ROUTE_FIELD,
    &BBH_TX_COMMON_CONFIGURATIONS_BBROUTE_DEST_FIELD,
    &BBH_TX_COMMON_CONFIGURATIONS_BBROUTE_EN_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_TX_COMMON_CONFIGURATIONS_BBROUTE *****/
const ru_reg_rec BBH_TX_COMMON_CONFIGURATIONS_BBROUTE_REG =
{
    "COMMON_CONFIGURATIONS_BBROUTE",
#if RU_INCLUDE_DESC
    "BB_ROUTE_OVERRIDE Register",
    "override configuration for the route of one of the peripherals (DMA/SDMMA/FPM/SBPM?Runners)\n",
#endif
    { BBH_TX_COMMON_CONFIGURATIONS_BBROUTE_REG_OFFSET },
    0,
    0,
    110,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    BBH_TX_COMMON_CONFIGURATIONS_BBROUTE_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_TX_COMMON_CONFIGURATIONS_DDRCFG_TX, TYPE: Type_BBH_TX_BBHTX_COMMON_CONFIGURATIONS_DDRCFG_TX
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: BUFSIZE *****/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_DDRCFG_TX_BUFSIZE_FIELD =
{
    "BUFSIZE",
#if RU_INCLUDE_DESC
    "",
    "The data is arranged in the DDR in a fixed size buffers.\n",
#endif
    { BBH_TX_COMMON_CONFIGURATIONS_DDRCFG_TX_BUFSIZE_FIELD_MASK },
    0,
    { BBH_TX_COMMON_CONFIGURATIONS_DDRCFG_TX_BUFSIZE_FIELD_WIDTH },
    { BBH_TX_COMMON_CONFIGURATIONS_DDRCFG_TX_BUFSIZE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: BYTERESUL *****/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_DDRCFG_TX_BYTERESUL_FIELD =
{
    "BYTERESUL",
#if RU_INCLUDE_DESC
    "",
    "The packet offset byte resulotion.\n",
#endif
    { BBH_TX_COMMON_CONFIGURATIONS_DDRCFG_TX_BYTERESUL_FIELD_MASK },
    0,
    { BBH_TX_COMMON_CONFIGURATIONS_DDRCFG_TX_BYTERESUL_FIELD_WIDTH },
    { BBH_TX_COMMON_CONFIGURATIONS_DDRCFG_TX_BYTERESUL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DDRTXOFFSET *****/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_DDRCFG_TX_DDRTXOFFSET_FIELD =
{
    "DDRTXOFFSET",
#if RU_INCLUDE_DESC
    "",
    "Static offset in 8-bytes resolution for non aggregated packets in DDR\n",
#endif
    { BBH_TX_COMMON_CONFIGURATIONS_DDRCFG_TX_DDRTXOFFSET_FIELD_MASK },
    0,
    { BBH_TX_COMMON_CONFIGURATIONS_DDRCFG_TX_DDRTXOFFSET_FIELD_WIDTH },
    { BBH_TX_COMMON_CONFIGURATIONS_DDRCFG_TX_DDRTXOFFSET_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: HNSIZE0 *****/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_DDRCFG_TX_HNSIZE0_FIELD =
{
    "HNSIZE0",
#if RU_INCLUDE_DESC
    "",
    "The size of the HN (Header number) in bytes. The BBH decides between size 0 and size 1 according to a bit in the PD.\nIn multicast, several headers are located in the same DDR buffer. the offset of header N within a buffer is (N-1)*64bytes, regardless of the HN actual size.\n",
#endif
    { BBH_TX_COMMON_CONFIGURATIONS_DDRCFG_TX_HNSIZE0_FIELD_MASK },
    0,
    { BBH_TX_COMMON_CONFIGURATIONS_DDRCFG_TX_HNSIZE0_FIELD_WIDTH },
    { BBH_TX_COMMON_CONFIGURATIONS_DDRCFG_TX_HNSIZE0_FIELD_SHIFT },
    64,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: HNSIZE1 *****/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_DDRCFG_TX_HNSIZE1_FIELD =
{
    "HNSIZE1",
#if RU_INCLUDE_DESC
    "",
    "The size of the HN (Header number) in bytes. The BBH decides between size 0 and size 1 according to a bit in the PD.\nIn multicast, several headers are located in the same DDR buffer. the offset of header N within a buffer is (N-1)*64bytes, regardless of the HN actual size.\n",
#endif
    { BBH_TX_COMMON_CONFIGURATIONS_DDRCFG_TX_HNSIZE1_FIELD_MASK },
    0,
    { BBH_TX_COMMON_CONFIGURATIONS_DDRCFG_TX_HNSIZE1_FIELD_WIDTH },
    { BBH_TX_COMMON_CONFIGURATIONS_DDRCFG_TX_HNSIZE1_FIELD_SHIFT },
    64,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_COMMON_CONFIGURATIONS_DDRCFG_TX_FIELDS[] =
{
    &BBH_TX_COMMON_CONFIGURATIONS_DDRCFG_TX_BUFSIZE_FIELD,
    &BBH_TX_COMMON_CONFIGURATIONS_DDRCFG_TX_BYTERESUL_FIELD,
    &BBH_TX_COMMON_CONFIGURATIONS_DDRCFG_TX_DDRTXOFFSET_FIELD,
    &BBH_TX_COMMON_CONFIGURATIONS_DDRCFG_TX_HNSIZE0_FIELD,
    &BBH_TX_COMMON_CONFIGURATIONS_DDRCFG_TX_HNSIZE1_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_TX_COMMON_CONFIGURATIONS_DDRCFG_TX *****/
const ru_reg_rec BBH_TX_COMMON_CONFIGURATIONS_DDRCFG_TX_REG =
{
    "COMMON_CONFIGURATIONS_DDRCFG_TX",
#if RU_INCLUDE_DESC
    "DDR_ADDR_CFG Register",
    "Configurations for determining the address to read from the DDR/PSRAm\n",
#endif
    { BBH_TX_COMMON_CONFIGURATIONS_DDRCFG_TX_REG_OFFSET },
    0,
    0,
    111,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    5,
    BBH_TX_COMMON_CONFIGURATIONS_DDRCFG_TX_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_TX_COMMON_CONFIGURATIONS_DDRCFG_TX2, TYPE: Type_BBH_TX_BBHTX_COMMON_CONFIGURATIONS_DDRCFG_TX2
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: AGG_POOLID *****/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_DDRCFG_TX2_AGG_POOLID_FIELD =
{
    "AGG_POOLID",
#if RU_INCLUDE_DESC
    "",
    "the pool-d is needed by the FPM in free commands. The pool-id for regular packets is determined in the PD. For aggregated PD it is determined here. This register should be aligned with the same value that is configured in the QM.\n",
#endif
    { BBH_TX_COMMON_CONFIGURATIONS_DDRCFG_TX2_AGG_POOLID_FIELD_MASK },
    0,
    { BBH_TX_COMMON_CONFIGURATIONS_DDRCFG_TX2_AGG_POOLID_FIELD_WIDTH },
    { BBH_TX_COMMON_CONFIGURATIONS_DDRCFG_TX2_AGG_POOLID_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: MCST_POOLID *****/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_DDRCFG_TX2_MCST_POOLID_FIELD =
{
    "MCST_POOLID",
#if RU_INCLUDE_DESC
    "",
    "the pool-id is needed by the FPM in free commands. The pool-id for regular packets is determined in the PD. multicast headers BN, the pool-id will be taken from here\n",
#endif
    { BBH_TX_COMMON_CONFIGURATIONS_DDRCFG_TX2_MCST_POOLID_FIELD_MASK },
    0,
    { BBH_TX_COMMON_CONFIGURATIONS_DDRCFG_TX2_MCST_POOLID_FIELD_WIDTH },
    { BBH_TX_COMMON_CONFIGURATIONS_DDRCFG_TX2_MCST_POOLID_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_COMMON_CONFIGURATIONS_DDRCFG_TX2_FIELDS[] =
{
    &BBH_TX_COMMON_CONFIGURATIONS_DDRCFG_TX2_AGG_POOLID_FIELD,
    &BBH_TX_COMMON_CONFIGURATIONS_DDRCFG_TX2_MCST_POOLID_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_TX_COMMON_CONFIGURATIONS_DDRCFG_TX2 *****/
const ru_reg_rec BBH_TX_COMMON_CONFIGURATIONS_DDRCFG_TX2_REG =
{
    "COMMON_CONFIGURATIONS_DDRCFG_TX2",
#if RU_INCLUDE_DESC
    "DDR_ADDR_CFG2 Register",
    "ddr related configs\n",
#endif
    { BBH_TX_COMMON_CONFIGURATIONS_DDRCFG_TX2_REG_OFFSET },
    0,
    0,
    112,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    BBH_TX_COMMON_CONFIGURATIONS_DDRCFG_TX2_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_TX_COMMON_CONFIGURATIONS_RNRCFG_1, TYPE: Type_BBH_TX_BBHTX_COMMON_CONFIGURATIONS_RNRCFG_1
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: TCONTADDR *****/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_RNRCFG_1_TCONTADDR_FIELD =
{
    "TCONTADDR",
#if RU_INCLUDE_DESC
    "",
    "Defines the TCONT address within the Runner address space.\nThe address is in 8 bytes resolution.\n\n",
#endif
    { BBH_TX_COMMON_CONFIGURATIONS_RNRCFG_1_TCONTADDR_FIELD_MASK },
    0,
    { BBH_TX_COMMON_CONFIGURATIONS_RNRCFG_1_TCONTADDR_FIELD_WIDTH },
    { BBH_TX_COMMON_CONFIGURATIONS_RNRCFG_1_TCONTADDR_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: SKBADDR *****/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_RNRCFG_1_SKBADDR_FIELD =
{
    "SKBADDR",
#if RU_INCLUDE_DESC
    "",
    "Defines the SKB free address within the Runner address space.\nThe address is in 8-bytes resolution.\n\n",
#endif
    { BBH_TX_COMMON_CONFIGURATIONS_RNRCFG_1_SKBADDR_FIELD_MASK },
    0,
    { BBH_TX_COMMON_CONFIGURATIONS_RNRCFG_1_SKBADDR_FIELD_WIDTH },
    { BBH_TX_COMMON_CONFIGURATIONS_RNRCFG_1_SKBADDR_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_COMMON_CONFIGURATIONS_RNRCFG_1_FIELDS[] =
{
    &BBH_TX_COMMON_CONFIGURATIONS_RNRCFG_1_TCONTADDR_FIELD,
    &BBH_TX_COMMON_CONFIGURATIONS_RNRCFG_1_SKBADDR_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_TX_COMMON_CONFIGURATIONS_RNRCFG_1 *****/
const ru_reg_rec BBH_TX_COMMON_CONFIGURATIONS_RNRCFG_1_REG =
{
    "COMMON_CONFIGURATIONS_RNRCFG_1",
#if RU_INCLUDE_DESC
    "PD_RNR_CFG_1 0..3 Register",
    "Queue index address:\nThe BBH requests a Packet descriptor from the Runner. The BBH writes the queue number in a predefined address at the Runner SRAM. The message serves also as a wake-up request to the Runner.\nThis register defines the queue index address within the Runner address space.\nSKB address:\nWhen the packet is transmitted from absolute address, then, instead of releasing the BN, the BBH writes a 6 bits read counter into the Runner SRAM. It writes it into a pre-defined address + TCONT_NUM (for Ethernet TCONT_NUM = 0).\nThis register defines the SKB free base address within the Runner address.\n\nNote: all addresses are in 8 byte resolution. As the Runner memory is limited to 12 bits address, use the 12 lsb bits.\n\n",
#endif
    { BBH_TX_COMMON_CONFIGURATIONS_RNRCFG_1_REG_OFFSET },
    BBH_TX_COMMON_CONFIGURATIONS_RNRCFG_1_REG_RAM_CNT,
    4,
    113,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    BBH_TX_COMMON_CONFIGURATIONS_RNRCFG_1_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_TX_COMMON_CONFIGURATIONS_RNRCFG_2, TYPE: Type_BBH_TX_BBHTX_COMMON_CONFIGURATIONS_RNRCFG_2
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: PTRADDR *****/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_RNRCFG_2_PTRADDR_FIELD =
{
    "PTRADDR",
#if RU_INCLUDE_DESC
    "",
    "This field defins the address in the Runner memory space to which the read pointer is written.\nThe address is in 8-bytes resolution.\n",
#endif
    { BBH_TX_COMMON_CONFIGURATIONS_RNRCFG_2_PTRADDR_FIELD_MASK },
    0,
    { BBH_TX_COMMON_CONFIGURATIONS_RNRCFG_2_PTRADDR_FIELD_WIDTH },
    { BBH_TX_COMMON_CONFIGURATIONS_RNRCFG_2_PTRADDR_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: TASK *****/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_RNRCFG_2_TASK_FIELD =
{
    "TASK",
#if RU_INCLUDE_DESC
    "",
    "The number of the task that is responsible for sending PDs to the BBH\n",
#endif
    { BBH_TX_COMMON_CONFIGURATIONS_RNRCFG_2_TASK_FIELD_MASK },
    0,
    { BBH_TX_COMMON_CONFIGURATIONS_RNRCFG_2_TASK_FIELD_WIDTH },
    { BBH_TX_COMMON_CONFIGURATIONS_RNRCFG_2_TASK_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_COMMON_CONFIGURATIONS_RNRCFG_2_FIELDS[] =
{
    &BBH_TX_COMMON_CONFIGURATIONS_RNRCFG_2_PTRADDR_FIELD,
    &BBH_TX_COMMON_CONFIGURATIONS_RNRCFG_2_TASK_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_TX_COMMON_CONFIGURATIONS_RNRCFG_2 *****/
const ru_reg_rec BBH_TX_COMMON_CONFIGURATIONS_RNRCFG_2_REG =
{
    "COMMON_CONFIGURATIONS_RNRCFG_2",
#if RU_INCLUDE_DESC
    "PD_RNR_CFG_2 0..3 Register",
    "PD transfer process:\n-The Runner wont ACK the BBH; therefore the BBH wont wake the TX task.\n-The Runner will push the PDs into the BBH (without any wakeup from the BBH).\n-Each time that the BBH reads a PD from the PD FIFO, it will write the read pointer into a pre-defined address in the Runner. The pointer is 6 bits width (one bit larger than needed to distinguish between full and empty).\n-The Runner should manage the congestion over the PD FIFO (in the BBH) by reading the BBH read pointer prior to each PD write.\n-PD drop should be done by the Runner only. The BBH will drop PD when the FIFO is full and will count each drop. The BBH wont release the BN in this case.\n-There will be a full threshold, which can be smaller than the actual size of the FIFO. When the BBH will move from full to not full state, the BBH will wakeup the Runner.\n\nNote: all addresses are in 8 byte resolution. As the Runner memory is limited to 12 bits address, use the 12 lsb bits.\n\n",
#endif
    { BBH_TX_COMMON_CONFIGURATIONS_RNRCFG_2_REG_OFFSET },
    BBH_TX_COMMON_CONFIGURATIONS_RNRCFG_2_REG_RAM_CNT,
    4,
    114,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    BBH_TX_COMMON_CONFIGURATIONS_RNRCFG_2_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_TX_COMMON_CONFIGURATIONS_PERQTASK, TYPE: Type_BBH_TX_BBHTX_COMMON_CONFIGURATIONS_PERQTASK
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: TASK0 *****/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_PERQTASK_TASK0_FIELD =
{
    "TASK0",
#if RU_INCLUDE_DESC
    "",
    "task number for queue 0\n",
#endif
    { BBH_TX_COMMON_CONFIGURATIONS_PERQTASK_TASK0_FIELD_MASK },
    0,
    { BBH_TX_COMMON_CONFIGURATIONS_PERQTASK_TASK0_FIELD_WIDTH },
    { BBH_TX_COMMON_CONFIGURATIONS_PERQTASK_TASK0_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: TASK1 *****/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_PERQTASK_TASK1_FIELD =
{
    "TASK1",
#if RU_INCLUDE_DESC
    "",
    "task number for queue 1\n",
#endif
    { BBH_TX_COMMON_CONFIGURATIONS_PERQTASK_TASK1_FIELD_MASK },
    0,
    { BBH_TX_COMMON_CONFIGURATIONS_PERQTASK_TASK1_FIELD_WIDTH },
    { BBH_TX_COMMON_CONFIGURATIONS_PERQTASK_TASK1_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: TASK2 *****/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_PERQTASK_TASK2_FIELD =
{
    "TASK2",
#if RU_INCLUDE_DESC
    "",
    "task number for queue 2\n",
#endif
    { BBH_TX_COMMON_CONFIGURATIONS_PERQTASK_TASK2_FIELD_MASK },
    0,
    { BBH_TX_COMMON_CONFIGURATIONS_PERQTASK_TASK2_FIELD_WIDTH },
    { BBH_TX_COMMON_CONFIGURATIONS_PERQTASK_TASK2_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: TASK3 *****/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_PERQTASK_TASK3_FIELD =
{
    "TASK3",
#if RU_INCLUDE_DESC
    "",
    "task number for queue 3\n",
#endif
    { BBH_TX_COMMON_CONFIGURATIONS_PERQTASK_TASK3_FIELD_MASK },
    0,
    { BBH_TX_COMMON_CONFIGURATIONS_PERQTASK_TASK3_FIELD_WIDTH },
    { BBH_TX_COMMON_CONFIGURATIONS_PERQTASK_TASK3_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: TASK4 *****/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_PERQTASK_TASK4_FIELD =
{
    "TASK4",
#if RU_INCLUDE_DESC
    "",
    "task number for queue 4\n",
#endif
    { BBH_TX_COMMON_CONFIGURATIONS_PERQTASK_TASK4_FIELD_MASK },
    0,
    { BBH_TX_COMMON_CONFIGURATIONS_PERQTASK_TASK4_FIELD_WIDTH },
    { BBH_TX_COMMON_CONFIGURATIONS_PERQTASK_TASK4_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: TASK5 *****/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_PERQTASK_TASK5_FIELD =
{
    "TASK5",
#if RU_INCLUDE_DESC
    "",
    "task number for queue 5\n",
#endif
    { BBH_TX_COMMON_CONFIGURATIONS_PERQTASK_TASK5_FIELD_MASK },
    0,
    { BBH_TX_COMMON_CONFIGURATIONS_PERQTASK_TASK5_FIELD_WIDTH },
    { BBH_TX_COMMON_CONFIGURATIONS_PERQTASK_TASK5_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: TASK6 *****/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_PERQTASK_TASK6_FIELD =
{
    "TASK6",
#if RU_INCLUDE_DESC
    "",
    "task number for queue 6\n",
#endif
    { BBH_TX_COMMON_CONFIGURATIONS_PERQTASK_TASK6_FIELD_MASK },
    0,
    { BBH_TX_COMMON_CONFIGURATIONS_PERQTASK_TASK6_FIELD_WIDTH },
    { BBH_TX_COMMON_CONFIGURATIONS_PERQTASK_TASK6_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: TASK7 *****/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_PERQTASK_TASK7_FIELD =
{
    "TASK7",
#if RU_INCLUDE_DESC
    "",
    "task number for queue 7\n",
#endif
    { BBH_TX_COMMON_CONFIGURATIONS_PERQTASK_TASK7_FIELD_MASK },
    0,
    { BBH_TX_COMMON_CONFIGURATIONS_PERQTASK_TASK7_FIELD_WIDTH },
    { BBH_TX_COMMON_CONFIGURATIONS_PERQTASK_TASK7_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

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

/***** Register struct: BBH_TX_COMMON_CONFIGURATIONS_PERQTASK *****/
const ru_reg_rec BBH_TX_COMMON_CONFIGURATIONS_PERQTASK_REG =
{
    "COMMON_CONFIGURATIONS_PERQTASK",
#if RU_INCLUDE_DESC
    "PER_Q_TASK Register",
    "which task in the runner to wake-up when requesting a PD for a certain q.\n\nThis register holds the task number of the first 8 queues.\n\nFor queues 8-40 (if they exist) the task that will be waking is the one appearing in the PD_RNR_CFG regs, depending on which runner this queue is associated with.\n",
#endif
    { BBH_TX_COMMON_CONFIGURATIONS_PERQTASK_REG_OFFSET },
    0,
    0,
    115,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    8,
    BBH_TX_COMMON_CONFIGURATIONS_PERQTASK_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD, TYPE: Type_BBH_TX_BBHTX_COMMON_CONFIGURATIONS_TXRSTCMD
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: CNTXTRST *****/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_CNTXTRST_FIELD =
{
    "CNTXTRST",
#if RU_INCLUDE_DESC
    "",
    "Writing 1 to this register will reset the segmentation context table.\nThe reset is done immediately. Reading this register will always return 0.\n",
#endif
    { BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_CNTXTRST_FIELD_MASK },
    0,
    { BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_CNTXTRST_FIELD_WIDTH },
    { BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_CNTXTRST_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: PDFIFORST *****/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_PDFIFORST_FIELD =
{
    "PDFIFORST",
#if RU_INCLUDE_DESC
    "",
    "Writing 1 to this register will reset the PDs FIFOs.\nThe reset is done immediately. Reading this register will always return 0.\n",
#endif
    { BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_PDFIFORST_FIELD_MASK },
    0,
    { BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_PDFIFORST_FIELD_WIDTH },
    { BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_PDFIFORST_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DMAPTRRST *****/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_DMAPTRRST_FIELD =
{
    "DMAPTRRST",
#if RU_INCLUDE_DESC
    "",
    "Writing 1 to this register will reset the DMA write pointer.\nThe reset is done immediately. Reading this register will always return 0.\n",
#endif
    { BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_DMAPTRRST_FIELD_MASK },
    0,
    { BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_DMAPTRRST_FIELD_WIDTH },
    { BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_DMAPTRRST_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: SDMAPTRRST *****/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_SDMAPTRRST_FIELD =
{
    "SDMAPTRRST",
#if RU_INCLUDE_DESC
    "",
    "Writing 1 to this register will reset the SDMA write pointer.\nThe reset is done immediately. Reading this register will always return 0.\nThis register is relevalt only for Ethernet.\n",
#endif
    { BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_SDMAPTRRST_FIELD_MASK },
    0,
    { BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_SDMAPTRRST_FIELD_WIDTH },
    { BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_SDMAPTRRST_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: BPMFIFORST *****/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_BPMFIFORST_FIELD =
{
    "BPMFIFORST",
#if RU_INCLUDE_DESC
    "",
    "Writing 1 to this register will reset the BPM FIFO.\nThe reset is done immediately. Reading this register will always return 0.\n",
#endif
    { BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_BPMFIFORST_FIELD_MASK },
    0,
    { BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_BPMFIFORST_FIELD_WIDTH },
    { BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_BPMFIFORST_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: SBPMFIFORST *****/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_SBPMFIFORST_FIELD =
{
    "SBPMFIFORST",
#if RU_INCLUDE_DESC
    "",
    "Writing 1 to this register will reset the SBPM FIFO.\nThe reset is done immediately. Reading this register will always return 0.\nThis register is relevalt only for Ethernet.\n",
#endif
    { BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_SBPMFIFORST_FIELD_MASK },
    0,
    { BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_SBPMFIFORST_FIELD_WIDTH },
    { BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_SBPMFIFORST_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: OKFIFORST *****/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_OKFIFORST_FIELD =
{
    "OKFIFORST",
#if RU_INCLUDE_DESC
    "",
    "Writing 1 to this register will reset the order keeper FIFO.\nThe reset is done immediately. Reading this register will always return 0.\nThis register is relevalt only for Ethernet.\n",
#endif
    { BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_OKFIFORST_FIELD_MASK },
    0,
    { BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_OKFIFORST_FIELD_WIDTH },
    { BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_OKFIFORST_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DDRFIFORST *****/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_DDRFIFORST_FIELD =
{
    "DDRFIFORST",
#if RU_INCLUDE_DESC
    "",
    "Writing 1 to this register will reset the DDR data FIFO.\nThe reset is done immediately. Reading this register will always return 0.\nThis register is relevalt only for Ethernet.\n",
#endif
    { BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_DDRFIFORST_FIELD_MASK },
    0,
    { BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_DDRFIFORST_FIELD_WIDTH },
    { BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_DDRFIFORST_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: SRAMFIFORST *****/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_SRAMFIFORST_FIELD =
{
    "SRAMFIFORST",
#if RU_INCLUDE_DESC
    "",
    "Writing 1 to this register will reset the SRAM data FIFO.\nThe reset is done immediately. Reading this register will always return 0.\nThis register is relevalt only for Ethernet.\n",
#endif
    { BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_SRAMFIFORST_FIELD_MASK },
    0,
    { BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_SRAMFIFORST_FIELD_WIDTH },
    { BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_SRAMFIFORST_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: SKBPTRRST *****/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_SKBPTRRST_FIELD =
{
    "SKBPTRRST",
#if RU_INCLUDE_DESC
    "",
    "Writing 1 to this register will reset the SKB pointers.\nThe reset is done immediately. Reading this register will always return 0.\n\n",
#endif
    { BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_SKBPTRRST_FIELD_MASK },
    0,
    { BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_SKBPTRRST_FIELD_WIDTH },
    { BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_SKBPTRRST_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: STSFIFORST *****/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_STSFIFORST_FIELD =
{
    "STSFIFORST",
#if RU_INCLUDE_DESC
    "",
    "Writing 1 to this register will reset the EPON status FIFOs (per queue 32 fifos).\nThe reset is done immediately. Reading this register will always return 0.\n",
#endif
    { BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_STSFIFORST_FIELD_MASK },
    0,
    { BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_STSFIFORST_FIELD_WIDTH },
    { BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_STSFIFORST_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: REQFIFORST *****/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_REQFIFORST_FIELD =
{
    "REQFIFORST",
#if RU_INCLUDE_DESC
    "",
    "Writing 1 to this register will reset the EPON request FIFO (8 entries FIFO that holds the packet requests from the EPON MAC).\nThe reset is done immediately. Reading this register will always return 0.\n",
#endif
    { BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_REQFIFORST_FIELD_MASK },
    0,
    { BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_REQFIFORST_FIELD_WIDTH },
    { BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_REQFIFORST_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: MSGFIFORST *****/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_MSGFIFORST_FIELD =
{
    "MSGFIFORST",
#if RU_INCLUDE_DESC
    "",
    "Writing 1 to this register will reset the EPON/GPON MSG FIFO\nThe reset is done immediately. Reading this register will always return 0.\n",
#endif
    { BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_MSGFIFORST_FIELD_MASK },
    0,
    { BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_MSGFIFORST_FIELD_WIDTH },
    { BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_MSGFIFORST_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: GNXTFIFORST *****/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_GNXTFIFORST_FIELD =
{
    "GNXTFIFORST",
#if RU_INCLUDE_DESC
    "",
    "Writing 1 to this register will reset the GET NEXT FIFOs\nThe reset is done immediately. Reading this register will always return 0.\n",
#endif
    { BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_GNXTFIFORST_FIELD_MASK },
    0,
    { BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_GNXTFIFORST_FIELD_WIDTH },
    { BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_GNXTFIFORST_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FBNFIFORST *****/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_FBNFIFORST_FIELD =
{
    "FBNFIFORST",
#if RU_INCLUDE_DESC
    "",
    "Writing 1 to this register will reset the FIRST BN FIFOs\nThe reset is done immediately. Reading this register will always return 0.\n",
#endif
    { BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_FBNFIFORST_FIELD_MASK },
    0,
    { BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_FBNFIFORST_FIELD_WIDTH },
    { BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_FBNFIFORST_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

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
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD *****/
const ru_reg_rec BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_REG =
{
    "COMMON_CONFIGURATIONS_TXRSTCMD",
#if RU_INCLUDE_DESC
    "TX_RESET_COMMAND Register",
    "This register enables reset of internal units (for possible WA purposes).\n",
#endif
    { BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_REG_OFFSET },
    0,
    0,
    116,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    15,
    BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_TX_COMMON_CONFIGURATIONS_DBGSEL, TYPE: Type_BBH_TX_BBHTX_COMMON_CONFIGURATIONS_DBGSEL
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DBGSEL *****/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_DBGSEL_DBGSEL_FIELD =
{
    "DBGSEL",
#if RU_INCLUDE_DESC
    "",
    "This register selects 1 of 8 debug vectors.\nThe selected vector is reflected to DBGOUTREG.\n",
#endif
    { BBH_TX_COMMON_CONFIGURATIONS_DBGSEL_DBGSEL_FIELD_MASK },
    0,
    { BBH_TX_COMMON_CONFIGURATIONS_DBGSEL_DBGSEL_FIELD_WIDTH },
    { BBH_TX_COMMON_CONFIGURATIONS_DBGSEL_DBGSEL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_COMMON_CONFIGURATIONS_DBGSEL_FIELDS[] =
{
    &BBH_TX_COMMON_CONFIGURATIONS_DBGSEL_DBGSEL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_TX_COMMON_CONFIGURATIONS_DBGSEL *****/
const ru_reg_rec BBH_TX_COMMON_CONFIGURATIONS_DBGSEL_REG =
{
    "COMMON_CONFIGURATIONS_DBGSEL",
#if RU_INCLUDE_DESC
    "DEBUG_SELECT Register",
    "This register selects 1 of 8 debug vectors.\nThe selected vector is reflected to DBGOUTREG.\n",
#endif
    { BBH_TX_COMMON_CONFIGURATIONS_DBGSEL_REG_OFFSET },
    0,
    0,
    117,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_TX_COMMON_CONFIGURATIONS_DBGSEL_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_TX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL, TYPE: Type_BBH_TX_BBHTX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: BYPASS_CLK_GATE *****/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_BYPASS_CLK_GATE_FIELD =
{
    "BYPASS_CLK_GATE",
#if RU_INCLUDE_DESC
    "",
    "If set to 1b1 will disable the clock gate logic such to always enable the clock\n",
#endif
    { BBH_TX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_BYPASS_CLK_GATE_FIELD_MASK },
    0,
    { BBH_TX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_BYPASS_CLK_GATE_FIELD_WIDTH },
    { BBH_TX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_BYPASS_CLK_GATE_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: TIMER_VAL *****/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_TIMER_VAL_FIELD =
{
    "TIMER_VAL",
#if RU_INCLUDE_DESC
    "",
    "For how long should the clock stay active once all conditions for clock disable are met.\n\n\n",
#endif
    { BBH_TX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_TIMER_VAL_FIELD_MASK },
    0,
    { BBH_TX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_TIMER_VAL_FIELD_WIDTH },
    { BBH_TX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_TIMER_VAL_FIELD_SHIFT },
    15,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: KEEP_ALIVE_EN *****/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_KEEP_ALIVE_EN_FIELD =
{
    "KEEP_ALIVE_EN",
#if RU_INCLUDE_DESC
    "",
    "Enables the keep alive logic which will periodically enable the clock to assure that no deadlock of clock being removed completely will occur\n",
#endif
    { BBH_TX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_KEEP_ALIVE_EN_FIELD_MASK },
    0,
    { BBH_TX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_KEEP_ALIVE_EN_FIELD_WIDTH },
    { BBH_TX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_KEEP_ALIVE_EN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: KEEP_ALIVE_INTRVL *****/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_KEEP_ALIVE_INTRVL_FIELD =
{
    "KEEP_ALIVE_INTRVL",
#if RU_INCLUDE_DESC
    "",
    "If the KEEP alive option is enabled the field will determine for how many cycles should the clock be active\n",
#endif
    { BBH_TX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_KEEP_ALIVE_INTRVL_FIELD_MASK },
    0,
    { BBH_TX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_KEEP_ALIVE_INTRVL_FIELD_WIDTH },
    { BBH_TX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_KEEP_ALIVE_INTRVL_FIELD_SHIFT },
    2,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: KEEP_ALIVE_CYC *****/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_KEEP_ALIVE_CYC_FIELD =
{
    "KEEP_ALIVE_CYC",
#if RU_INCLUDE_DESC
    "",
    "If the KEEP alive option is enabled this field will determine for how many cycles should the clock be disabled (minus the KEEP_ALIVE_INTERVAL)\n\nSo KEEP_ALIVE_CYCLE must be larger than KEEP_ALIVE_INTERVAL.\n",
#endif
    { BBH_TX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_KEEP_ALIVE_CYC_FIELD_MASK },
    0,
    { BBH_TX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_KEEP_ALIVE_CYC_FIELD_WIDTH },
    { BBH_TX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_KEEP_ALIVE_CYC_FIELD_SHIFT },
    15,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_FIELDS[] =
{
    &BBH_TX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_BYPASS_CLK_GATE_FIELD,
    &BBH_TX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_TIMER_VAL_FIELD,
    &BBH_TX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_KEEP_ALIVE_EN_FIELD,
    &BBH_TX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_KEEP_ALIVE_INTRVL_FIELD,
    &BBH_TX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_KEEP_ALIVE_CYC_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_TX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL *****/
const ru_reg_rec BBH_TX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_REG =
{
    "COMMON_CONFIGURATIONS_CLK_GATE_CNTRL",
#if RU_INCLUDE_DESC
    "CLOCK_GATE_CONTROL Register",
    "Clock Gate control register including timer config and bypass control\n",
#endif
    { BBH_TX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_REG_OFFSET },
    0,
    0,
    118,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    5,
    BBH_TX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_TX_COMMON_CONFIGURATIONS_GPR, TYPE: Type_BBH_TX_BBHTX_COMMON_CONFIGURATIONS_GPR
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: GPR *****/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_GPR_GPR_FIELD =
{
    "GPR",
#if RU_INCLUDE_DESC
    "",
    "general purpose register\n",
#endif
    { BBH_TX_COMMON_CONFIGURATIONS_GPR_GPR_FIELD_MASK },
    0,
    { BBH_TX_COMMON_CONFIGURATIONS_GPR_GPR_FIELD_WIDTH },
    { BBH_TX_COMMON_CONFIGURATIONS_GPR_GPR_FIELD_SHIFT },
    3,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_COMMON_CONFIGURATIONS_GPR_FIELDS[] =
{
    &BBH_TX_COMMON_CONFIGURATIONS_GPR_GPR_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_TX_COMMON_CONFIGURATIONS_GPR *****/
const ru_reg_rec BBH_TX_COMMON_CONFIGURATIONS_GPR_REG =
{
    "COMMON_CONFIGURATIONS_GPR",
#if RU_INCLUDE_DESC
    "GENERAL_PURPOSE_REGISTER Register",
    "general purpose register\n",
#endif
    { BBH_TX_COMMON_CONFIGURATIONS_GPR_REG_OFFSET },
    0,
    0,
    119,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_TX_COMMON_CONFIGURATIONS_GPR_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_TX_COMMON_CONFIGURATIONS_GENERAL_CFG, TYPE: Type_BBH_TX_BBHTX_COMMON_CONFIGURATIONS_GENERAL_CFG
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DSDMA *****/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_GENERAL_CFG_DSDMA_FIELD =
{
    "DSDMA",
#if RU_INCLUDE_DESC
    "",
    "support dual slave DMA\n",
#endif
    { BBH_TX_COMMON_CONFIGURATIONS_GENERAL_CFG_DSDMA_FIELD_MASK },
    0,
    { BBH_TX_COMMON_CONFIGURATIONS_GENERAL_CFG_DSDMA_FIELD_WIDTH },
    { BBH_TX_COMMON_CONFIGURATIONS_GENERAL_CFG_DSDMA_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: AGG640 *****/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_GENERAL_CFG_AGG640_FIELD =
{
    "AGG640",
#if RU_INCLUDE_DESC
    "",
    "support aggregation of 640 bytes\n",
#endif
    { BBH_TX_COMMON_CONFIGURATIONS_GENERAL_CFG_AGG640_FIELD_MASK },
    0,
    { BBH_TX_COMMON_CONFIGURATIONS_GENERAL_CFG_AGG640_FIELD_WIDTH },
    { BBH_TX_COMMON_CONFIGURATIONS_GENERAL_CFG_AGG640_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: PAIR256 *****/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_GENERAL_CFG_PAIR256_FIELD =
{
    "PAIR256",
#if RU_INCLUDE_DESC
    "",
    "when asserted, the segmentation SM will try to group read commands of the same PD to pairs in order to create longer reads from the DDR\n",
#endif
    { BBH_TX_COMMON_CONFIGURATIONS_GENERAL_CFG_PAIR256_FIELD_MASK },
    0,
    { BBH_TX_COMMON_CONFIGURATIONS_GENERAL_CFG_PAIR256_FIELD_WIDTH },
    { BBH_TX_COMMON_CONFIGURATIONS_GENERAL_CFG_PAIR256_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: MACDROP *****/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_GENERAL_CFG_MACDROP_FIELD =
{
    "MACDROP",
#if RU_INCLUDE_DESC
    "",
    "when asserted, the BBH TX will drop the packet data of an invalid MAC flow\n",
#endif
    { BBH_TX_COMMON_CONFIGURATIONS_GENERAL_CFG_MACDROP_FIELD_MASK },
    0,
    { BBH_TX_COMMON_CONFIGURATIONS_GENERAL_CFG_MACDROP_FIELD_WIDTH },
    { BBH_TX_COMMON_CONFIGURATIONS_GENERAL_CFG_MACDROP_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: EMBHDR *****/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_GENERAL_CFG_EMBHDR_FIELD =
{
    "EMBHDR",
#if RU_INCLUDE_DESC
    "",
    "when asserted, the BBH TX will read extra 4 bytes for each packet. these 4 bytes will reside before the packet SOP and will contain extra PD info.\n",
#endif
    { BBH_TX_COMMON_CONFIGURATIONS_GENERAL_CFG_EMBHDR_FIELD_MASK },
    0,
    { BBH_TX_COMMON_CONFIGURATIONS_GENERAL_CFG_EMBHDR_FIELD_WIDTH },
    { BBH_TX_COMMON_CONFIGURATIONS_GENERAL_CFG_EMBHDR_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ECNRMRK *****/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_GENERAL_CFG_ECNRMRK_FIELD =
{
    "ECNRMRK",
#if RU_INCLUDE_DESC
    "",
    "when asserted, the BBH TX will change the ecn bit according to the information in the PD + embedded header\n",
#endif
    { BBH_TX_COMMON_CONFIGURATIONS_GENERAL_CFG_ECNRMRK_FIELD_MASK },
    0,
    { BBH_TX_COMMON_CONFIGURATIONS_GENERAL_CFG_ECNRMRK_FIELD_WIDTH },
    { BBH_TX_COMMON_CONFIGURATIONS_GENERAL_CFG_ECNRMRK_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: CNTRADDCRC *****/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_GENERAL_CFG_CNTRADDCRC_FIELD =
{
    "CNTRADDCRC",
#if RU_INCLUDE_DESC
    "",
    "when asserted, the BBH TX will add 4 bytes per packet to all the byte counters, compensating the crc bytes\n",
#endif
    { BBH_TX_COMMON_CONFIGURATIONS_GENERAL_CFG_CNTRADDCRC_FIELD_MASK },
    0,
    { BBH_TX_COMMON_CONFIGURATIONS_GENERAL_CFG_CNTRADDCRC_FIELD_WIDTH },
    { BBH_TX_COMMON_CONFIGURATIONS_GENERAL_CFG_CNTRADDCRC_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DBLSOP *****/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_GENERAL_CFG_DBLSOP_FIELD =
{
    "DBLSOP",
#if RU_INCLUDE_DESC
    "",
    "when asserted, the BBH TX will double the SOP offset of g9991 PDs, to allow 2K sop offset with 10 bits SOP field in the PD\n",
#endif
    { BBH_TX_COMMON_CONFIGURATIONS_GENERAL_CFG_DBLSOP_FIELD_MASK },
    0,
    { BBH_TX_COMMON_CONFIGURATIONS_GENERAL_CFG_DBLSOP_FIELD_WIDTH },
    { BBH_TX_COMMON_CONFIGURATIONS_GENERAL_CFG_DBLSOP_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FPMINI *****/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_GENERAL_CFG_FPMINI_FIELD =
{
    "FPMINI",
#if RU_INCLUDE_DESC
    "",
    "When asserted, the BBH TX will use FPMini free commands format\n",
#endif
    { BBH_TX_COMMON_CONFIGURATIONS_GENERAL_CFG_FPMINI_FIELD_MASK },
    0,
    { BBH_TX_COMMON_CONFIGURATIONS_GENERAL_CFG_FPMINI_FIELD_WIDTH },
    { BBH_TX_COMMON_CONFIGURATIONS_GENERAL_CFG_FPMINI_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: CRDTFIX *****/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_GENERAL_CFG_CRDTFIX_FIELD =
{
    "CRDTFIX",
#if RU_INCLUDE_DESC
    "",
    "allow unified credits to reach fifos full and not just almost full\n",
#endif
    { BBH_TX_COMMON_CONFIGURATIONS_GENERAL_CFG_CRDTFIX_FIELD_MASK },
    0,
    { BBH_TX_COMMON_CONFIGURATIONS_GENERAL_CFG_CRDTFIX_FIELD_WIDTH },
    { BBH_TX_COMMON_CONFIGURATIONS_GENERAL_CFG_CRDTFIX_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DISCOPYCRDT *****/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_GENERAL_CFG_DISCOPYCRDT_FIELD =
{
    "DISCOPYCRDT",
#if RU_INCLUDE_DESC
    "",
    "For the copy BBH TX, there is no need to check for credits in the FE buffer. When this bit is asserted, the segmentation SM will not check for credits, and the unified IF will not read from the data FIFO is the FE buffers are full.\n",
#endif
    { BBH_TX_COMMON_CONFIGURATIONS_GENERAL_CFG_DISCOPYCRDT_FIELD_MASK },
    0,
    { BBH_TX_COMMON_CONFIGURATIONS_GENERAL_CFG_DISCOPYCRDT_FIELD_WIDTH },
    { BBH_TX_COMMON_CONFIGURATIONS_GENERAL_CFG_DISCOPYCRDT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_COMMON_CONFIGURATIONS_GENERAL_CFG_FIELDS[] =
{
    &BBH_TX_COMMON_CONFIGURATIONS_GENERAL_CFG_DSDMA_FIELD,
    &BBH_TX_COMMON_CONFIGURATIONS_GENERAL_CFG_AGG640_FIELD,
    &BBH_TX_COMMON_CONFIGURATIONS_GENERAL_CFG_PAIR256_FIELD,
    &BBH_TX_COMMON_CONFIGURATIONS_GENERAL_CFG_MACDROP_FIELD,
    &BBH_TX_COMMON_CONFIGURATIONS_GENERAL_CFG_EMBHDR_FIELD,
    &BBH_TX_COMMON_CONFIGURATIONS_GENERAL_CFG_ECNRMRK_FIELD,
    &BBH_TX_COMMON_CONFIGURATIONS_GENERAL_CFG_CNTRADDCRC_FIELD,
    &BBH_TX_COMMON_CONFIGURATIONS_GENERAL_CFG_DBLSOP_FIELD,
    &BBH_TX_COMMON_CONFIGURATIONS_GENERAL_CFG_FPMINI_FIELD,
    &BBH_TX_COMMON_CONFIGURATIONS_GENERAL_CFG_CRDTFIX_FIELD,
    &BBH_TX_COMMON_CONFIGURATIONS_GENERAL_CFG_DISCOPYCRDT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_TX_COMMON_CONFIGURATIONS_GENERAL_CFG *****/
const ru_reg_rec BBH_TX_COMMON_CONFIGURATIONS_GENERAL_CFG_REG =
{
    "COMMON_CONFIGURATIONS_GENERAL_CFG",
#if RU_INCLUDE_DESC
    "GENERAL_CONFIG Register",
    "1. configuration whether the BBH works with single DMA for both DDR and SRAM accesses or not\n2. configuration whether to support 640 bytes aggregation\n",
#endif
    { BBH_TX_COMMON_CONFIGURATIONS_GENERAL_CFG_REG_OFFSET },
    0,
    0,
    120,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    11,
    BBH_TX_COMMON_CONFIGURATIONS_GENERAL_CFG_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_TX_COMMON_CONFIGURATIONS_ECNCFG, TYPE: Type_BBH_TX_BBHTX_COMMON_CONFIGURATIONS_ECNCFG
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: ECNBYTEIPV4 *****/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_ECNCFG_ECNBYTEIPV4_FIELD =
{
    "ECNBYTEIPV4",
#if RU_INCLUDE_DESC
    "",
    "the number of byte in which the ecn bits are located in ipv4 packets, relative to the ip header offset\n",
#endif
    { BBH_TX_COMMON_CONFIGURATIONS_ECNCFG_ECNBYTEIPV4_FIELD_MASK },
    0,
    { BBH_TX_COMMON_CONFIGURATIONS_ECNCFG_ECNBYTEIPV4_FIELD_WIDTH },
    { BBH_TX_COMMON_CONFIGURATIONS_ECNCFG_ECNBYTEIPV4_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ECNBITIPV4 *****/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_ECNCFG_ECNBITIPV4_FIELD =
{
    "ECNBITIPV4",
#if RU_INCLUDE_DESC
    "",
    "the bit offset of the ECN bits within the byte. 0 means the ecn bits are [1:0], 6 means the ecn bits are [7:6]\n",
#endif
    { BBH_TX_COMMON_CONFIGURATIONS_ECNCFG_ECNBITIPV4_FIELD_MASK },
    0,
    { BBH_TX_COMMON_CONFIGURATIONS_ECNCFG_ECNBITIPV4_FIELD_WIDTH },
    { BBH_TX_COMMON_CONFIGURATIONS_ECNCFG_ECNBITIPV4_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ECNBYTEIPV6 *****/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_ECNCFG_ECNBYTEIPV6_FIELD =
{
    "ECNBYTEIPV6",
#if RU_INCLUDE_DESC
    "",
    "the number of byte in which the ecn bits are located in ipv6 packets, relative to the ip header offset\n",
#endif
    { BBH_TX_COMMON_CONFIGURATIONS_ECNCFG_ECNBYTEIPV6_FIELD_MASK },
    0,
    { BBH_TX_COMMON_CONFIGURATIONS_ECNCFG_ECNBYTEIPV6_FIELD_WIDTH },
    { BBH_TX_COMMON_CONFIGURATIONS_ECNCFG_ECNBYTEIPV6_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ECNBITIPV6 *****/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_ECNCFG_ECNBITIPV6_FIELD =
{
    "ECNBITIPV6",
#if RU_INCLUDE_DESC
    "",
    "the bit offset of the ECN bits within the byte. 0 means the ecn bits are [1:0], 6 means the ecn bits are [7:6]\n",
#endif
    { BBH_TX_COMMON_CONFIGURATIONS_ECNCFG_ECNBITIPV6_FIELD_MASK },
    0,
    { BBH_TX_COMMON_CONFIGURATIONS_ECNCFG_ECNBITIPV6_FIELD_WIDTH },
    { BBH_TX_COMMON_CONFIGURATIONS_ECNCFG_ECNBITIPV6_FIELD_SHIFT },
    4,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: CHKSUMBYTEIPV4 *****/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_ECNCFG_CHKSUMBYTEIPV4_FIELD =
{
    "CHKSUMBYTEIPV4",
#if RU_INCLUDE_DESC
    "",
    "the offset of the checksum field related to ip header offset in ipv4 packets. The offset must be multiplication of 2\n",
#endif
    { BBH_TX_COMMON_CONFIGURATIONS_ECNCFG_CHKSUMBYTEIPV4_FIELD_MASK },
    0,
    { BBH_TX_COMMON_CONFIGURATIONS_ECNCFG_CHKSUMBYTEIPV4_FIELD_WIDTH },
    { BBH_TX_COMMON_CONFIGURATIONS_ECNCFG_CHKSUMBYTEIPV4_FIELD_SHIFT },
    10,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_COMMON_CONFIGURATIONS_ECNCFG_FIELDS[] =
{
    &BBH_TX_COMMON_CONFIGURATIONS_ECNCFG_ECNBYTEIPV4_FIELD,
    &BBH_TX_COMMON_CONFIGURATIONS_ECNCFG_ECNBITIPV4_FIELD,
    &BBH_TX_COMMON_CONFIGURATIONS_ECNCFG_ECNBYTEIPV6_FIELD,
    &BBH_TX_COMMON_CONFIGURATIONS_ECNCFG_ECNBITIPV6_FIELD,
    &BBH_TX_COMMON_CONFIGURATIONS_ECNCFG_CHKSUMBYTEIPV4_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_TX_COMMON_CONFIGURATIONS_ECNCFG *****/
const ru_reg_rec BBH_TX_COMMON_CONFIGURATIONS_ECNCFG_REG =
{
    "COMMON_CONFIGURATIONS_ECNCFG",
#if RU_INCLUDE_DESC
    "ECN_CONFIGS Register",
    "configurations related to ecn\n",
#endif
    { BBH_TX_COMMON_CONFIGURATIONS_ECNCFG_REG_OFFSET },
    0,
    0,
    121,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    5,
    BBH_TX_COMMON_CONFIGURATIONS_ECNCFG_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_TX_COMMON_CONFIGURATIONS_DDRFPMINIBASEL, TYPE: Type_BBH_TX_BBHTX_COMMON_CONFIGURATIONS_DDRFPMINIBASEL
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DDRFPMINIBASE *****/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_DDRFPMINIBASEL_DDRFPMINIBASE_FIELD =
{
    "DDRFPMINIBASE",
#if RU_INCLUDE_DESC
    "",
    "DDR FPMINI base.\nThe address is in bytes resolution.\nThe address should be aligned to 128 bytes.\n",
#endif
    { BBH_TX_COMMON_CONFIGURATIONS_DDRFPMINIBASEL_DDRFPMINIBASE_FIELD_MASK },
    0,
    { BBH_TX_COMMON_CONFIGURATIONS_DDRFPMINIBASEL_DDRFPMINIBASE_FIELD_WIDTH },
    { BBH_TX_COMMON_CONFIGURATIONS_DDRFPMINIBASEL_DDRFPMINIBASE_FIELD_SHIFT },
    8388608,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_COMMON_CONFIGURATIONS_DDRFPMINIBASEL_FIELDS[] =
{
    &BBH_TX_COMMON_CONFIGURATIONS_DDRFPMINIBASEL_DDRFPMINIBASE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_TX_COMMON_CONFIGURATIONS_DDRFPMINIBASEL *****/
const ru_reg_rec BBH_TX_COMMON_CONFIGURATIONS_DDRFPMINIBASEL_REG =
{
    "COMMON_CONFIGURATIONS_DDRFPMINIBASEL",
#if RU_INCLUDE_DESC
    "DDR_TM_BASE_LOW_FPMINI Register",
    "The BBH calculate the DDR physical address according to the Buffer number and buffer size and then adds the DDR TM base.\n\nThe value of this register should match the relevant registers value in the QM and the Runner.\n\n",
#endif
    { BBH_TX_COMMON_CONFIGURATIONS_DDRFPMINIBASEL_REG_OFFSET },
    0,
    0,
    122,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_TX_COMMON_CONFIGURATIONS_DDRFPMINIBASEL_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_TX_COMMON_CONFIGURATIONS_DDRFPMINIBASEH, TYPE: Type_BBH_TX_BBHTX_COMMON_CONFIGURATIONS_DDRFPMINIBASEH
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DDRFPMINIBASE *****/
const ru_field_rec BBH_TX_COMMON_CONFIGURATIONS_DDRFPMINIBASEH_DDRFPMINIBASE_FIELD =
{
    "DDRFPMINIBASE",
#if RU_INCLUDE_DESC
    "",
    "DDR FPMINI base.\nThe address is in bytes resolution.\nThe address should be aligned to 128 bytes.\n",
#endif
    { BBH_TX_COMMON_CONFIGURATIONS_DDRFPMINIBASEH_DDRFPMINIBASE_FIELD_MASK },
    0,
    { BBH_TX_COMMON_CONFIGURATIONS_DDRFPMINIBASEH_DDRFPMINIBASE_FIELD_WIDTH },
    { BBH_TX_COMMON_CONFIGURATIONS_DDRFPMINIBASEH_DDRFPMINIBASE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_COMMON_CONFIGURATIONS_DDRFPMINIBASEH_FIELDS[] =
{
    &BBH_TX_COMMON_CONFIGURATIONS_DDRFPMINIBASEH_DDRFPMINIBASE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_TX_COMMON_CONFIGURATIONS_DDRFPMINIBASEH *****/
const ru_reg_rec BBH_TX_COMMON_CONFIGURATIONS_DDRFPMINIBASEH_REG =
{
    "COMMON_CONFIGURATIONS_DDRFPMINIBASEH",
#if RU_INCLUDE_DESC
    "DDR_TM_BASE_HIGH_FPMINI Register",
    "The BBH calculate the DDR physical address according to the Buffer number and buffer size and then adds the DDR TM base.\n\nThe value of this register should match the relevant registers value in the QM and the Runner.\n\n",
#endif
    { BBH_TX_COMMON_CONFIGURATIONS_DDRFPMINIBASEH_REG_OFFSET },
    0,
    0,
    123,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_TX_COMMON_CONFIGURATIONS_DDRFPMINIBASEH_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_TX_WAN_CONFIGURATIONS_Q2RNR, TYPE: Type_BBH_TX_BBHTX_WAN_CONFIGURATIONS_Q2RNR
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: Q0 *****/
const ru_field_rec BBH_TX_WAN_CONFIGURATIONS_Q2RNR_Q0_FIELD =
{
    "Q0",
#if RU_INCLUDE_DESC
    "",
    "Q0 configuration\n",
#endif
    { BBH_TX_WAN_CONFIGURATIONS_Q2RNR_Q0_FIELD_MASK },
    0,
    { BBH_TX_WAN_CONFIGURATIONS_Q2RNR_Q0_FIELD_WIDTH },
    { BBH_TX_WAN_CONFIGURATIONS_Q2RNR_Q0_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: Q1 *****/
const ru_field_rec BBH_TX_WAN_CONFIGURATIONS_Q2RNR_Q1_FIELD =
{
    "Q1",
#if RU_INCLUDE_DESC
    "",
    "Q1 configuration\n",
#endif
    { BBH_TX_WAN_CONFIGURATIONS_Q2RNR_Q1_FIELD_MASK },
    0,
    { BBH_TX_WAN_CONFIGURATIONS_Q2RNR_Q1_FIELD_WIDTH },
    { BBH_TX_WAN_CONFIGURATIONS_Q2RNR_Q1_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_WAN_CONFIGURATIONS_Q2RNR_FIELDS[] =
{
    &BBH_TX_WAN_CONFIGURATIONS_Q2RNR_Q0_FIELD,
    &BBH_TX_WAN_CONFIGURATIONS_Q2RNR_Q1_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_TX_WAN_CONFIGURATIONS_Q2RNR *****/
const ru_reg_rec BBH_TX_WAN_CONFIGURATIONS_Q2RNR_REG =
{
    "WAN_CONFIGURATIONS_Q2RNR",
#if RU_INCLUDE_DESC
    "Q_TO_RNR 0..19 Register",
    "configuration which queue is managed by each of the two runners.\n\nEach register in this array configures 2 queues.\n",
#endif
    { BBH_TX_WAN_CONFIGURATIONS_Q2RNR_REG_OFFSET },
    BBH_TX_WAN_CONFIGURATIONS_Q2RNR_REG_RAM_CNT,
    4,
    124,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    BBH_TX_WAN_CONFIGURATIONS_Q2RNR_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_TX_WAN_CONFIGURATIONS_QPROF, TYPE: Type_BBH_TX_BBHTX_WAN_CONFIGURATIONS_QPROF
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: Q0 *****/
const ru_field_rec BBH_TX_WAN_CONFIGURATIONS_QPROF_Q0_FIELD =
{
    "Q0",
#if RU_INCLUDE_DESC
    "",
    "Q0 configuration\n",
#endif
    { BBH_TX_WAN_CONFIGURATIONS_QPROF_Q0_FIELD_MASK },
    0,
    { BBH_TX_WAN_CONFIGURATIONS_QPROF_Q0_FIELD_WIDTH },
    { BBH_TX_WAN_CONFIGURATIONS_QPROF_Q0_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: Q1 *****/
const ru_field_rec BBH_TX_WAN_CONFIGURATIONS_QPROF_Q1_FIELD =
{
    "Q1",
#if RU_INCLUDE_DESC
    "",
    "Q1 configuration\n",
#endif
    { BBH_TX_WAN_CONFIGURATIONS_QPROF_Q1_FIELD_MASK },
    0,
    { BBH_TX_WAN_CONFIGURATIONS_QPROF_Q1_FIELD_WIDTH },
    { BBH_TX_WAN_CONFIGURATIONS_QPROF_Q1_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DIS0 *****/
const ru_field_rec BBH_TX_WAN_CONFIGURATIONS_QPROF_DIS0_FIELD =
{
    "DIS0",
#if RU_INCLUDE_DESC
    "",
    "disable q 0\n",
#endif
    { BBH_TX_WAN_CONFIGURATIONS_QPROF_DIS0_FIELD_MASK },
    0,
    { BBH_TX_WAN_CONFIGURATIONS_QPROF_DIS0_FIELD_WIDTH },
    { BBH_TX_WAN_CONFIGURATIONS_QPROF_DIS0_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DIS1 *****/
const ru_field_rec BBH_TX_WAN_CONFIGURATIONS_QPROF_DIS1_FIELD =
{
    "DIS1",
#if RU_INCLUDE_DESC
    "",
    "disable q 1\n",
#endif
    { BBH_TX_WAN_CONFIGURATIONS_QPROF_DIS1_FIELD_MASK },
    0,
    { BBH_TX_WAN_CONFIGURATIONS_QPROF_DIS1_FIELD_WIDTH },
    { BBH_TX_WAN_CONFIGURATIONS_QPROF_DIS1_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_WAN_CONFIGURATIONS_QPROF_FIELDS[] =
{
    &BBH_TX_WAN_CONFIGURATIONS_QPROF_Q0_FIELD,
    &BBH_TX_WAN_CONFIGURATIONS_QPROF_Q1_FIELD,
    &BBH_TX_WAN_CONFIGURATIONS_QPROF_DIS0_FIELD,
    &BBH_TX_WAN_CONFIGURATIONS_QPROF_DIS1_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_TX_WAN_CONFIGURATIONS_QPROF *****/
const ru_reg_rec BBH_TX_WAN_CONFIGURATIONS_QPROF_REG =
{
    "WAN_CONFIGURATIONS_QPROF",
#if RU_INCLUDE_DESC
    "PER_Q_PROFILE 0..19 Register",
    "configuration of the profile per queue.\nThe profile determines the PD FIFO size, the wakeup threshold and bytes threshold.\n\nEach register in this array configures 2 queues.\n",
#endif
    { BBH_TX_WAN_CONFIGURATIONS_QPROF_REG_OFFSET },
    BBH_TX_WAN_CONFIGURATIONS_QPROF_REG_RAM_CNT,
    4,
    125,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    BBH_TX_WAN_CONFIGURATIONS_QPROF_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_TX_WAN_CONFIGURATIONS_PDSIZE, TYPE: Type_BBH_TX_BBHTX_WAN_CONFIGURATIONS_PDSIZE
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: FIFOSIZE0 *****/
const ru_field_rec BBH_TX_WAN_CONFIGURATIONS_PDSIZE_FIFOSIZE0_FIELD =
{
    "FIFOSIZE0",
#if RU_INCLUDE_DESC
    "",
    "The size of PD FIFO for queue 0.\nA value of n refers to n+1.\nFor GPON, the max value is 0x7\nFor EPON, the max value is 0xf\n",
#endif
    { BBH_TX_WAN_CONFIGURATIONS_PDSIZE_FIFOSIZE0_FIELD_MASK },
    0,
    { BBH_TX_WAN_CONFIGURATIONS_PDSIZE_FIFOSIZE0_FIELD_WIDTH },
    { BBH_TX_WAN_CONFIGURATIONS_PDSIZE_FIFOSIZE0_FIELD_SHIFT },
    7,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FIFOSIZE1 *****/
const ru_field_rec BBH_TX_WAN_CONFIGURATIONS_PDSIZE_FIFOSIZE1_FIELD =
{
    "FIFOSIZE1",
#if RU_INCLUDE_DESC
    "",
    "The size of PD FIFO for queue 1.\nA value of n refers to n+1.\n\n",
#endif
    { BBH_TX_WAN_CONFIGURATIONS_PDSIZE_FIFOSIZE1_FIELD_MASK },
    0,
    { BBH_TX_WAN_CONFIGURATIONS_PDSIZE_FIFOSIZE1_FIELD_WIDTH },
    { BBH_TX_WAN_CONFIGURATIONS_PDSIZE_FIFOSIZE1_FIELD_SHIFT },
    7,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_WAN_CONFIGURATIONS_PDSIZE_FIELDS[] =
{
    &BBH_TX_WAN_CONFIGURATIONS_PDSIZE_FIFOSIZE0_FIELD,
    &BBH_TX_WAN_CONFIGURATIONS_PDSIZE_FIFOSIZE1_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_TX_WAN_CONFIGURATIONS_PDSIZE *****/
const ru_reg_rec BBH_TX_WAN_CONFIGURATIONS_PDSIZE_REG =
{
    "WAN_CONFIGURATIONS_PDSIZE",
#if RU_INCLUDE_DESC
    "PD_FIFO_SIZE Register",
    "The BBH manages 40 queues for GPON and 32 queues for EPON (FIFO per TCONT/LLID). For each queue it manages a PD FIFO.\nA total of 256 PDs are available for all queues.\nThere are 2 profiles of queues. For each profile the SW configures the size of the PD FIFO and then specifies for each queue which profile it is associated with.\nThis register defines the PD FIFO size of the 2 profiles.\n\n",
#endif
    { BBH_TX_WAN_CONFIGURATIONS_PDSIZE_REG_OFFSET },
    0,
    0,
    126,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    BBH_TX_WAN_CONFIGURATIONS_PDSIZE_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_TX_WAN_CONFIGURATIONS_PDWKUPH, TYPE: Type_BBH_TX_BBHTX_WAN_CONFIGURATIONS_PDWKUPH
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: WKUPTHRESH0 *****/
const ru_field_rec BBH_TX_WAN_CONFIGURATIONS_PDWKUPH_WKUPTHRESH0_FIELD =
{
    "WKUPTHRESH0",
#if RU_INCLUDE_DESC
    "",
    "The wakeup threshold of the PD FIFO for queue 0.\nA value of n refers to n+1.\nRelevant only for EPON BBH.\n",
#endif
    { BBH_TX_WAN_CONFIGURATIONS_PDWKUPH_WKUPTHRESH0_FIELD_MASK },
    0,
    { BBH_TX_WAN_CONFIGURATIONS_PDWKUPH_WKUPTHRESH0_FIELD_WIDTH },
    { BBH_TX_WAN_CONFIGURATIONS_PDWKUPH_WKUPTHRESH0_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: WKUPTHRESH1 *****/
const ru_field_rec BBH_TX_WAN_CONFIGURATIONS_PDWKUPH_WKUPTHRESH1_FIELD =
{
    "WKUPTHRESH1",
#if RU_INCLUDE_DESC
    "",
    "The wakeup threshold of the PD FIFO for queue 1.\nA value of n refers to n+1.\nRelevant only for EPON BBH.\n",
#endif
    { BBH_TX_WAN_CONFIGURATIONS_PDWKUPH_WKUPTHRESH1_FIELD_MASK },
    0,
    { BBH_TX_WAN_CONFIGURATIONS_PDWKUPH_WKUPTHRESH1_FIELD_WIDTH },
    { BBH_TX_WAN_CONFIGURATIONS_PDWKUPH_WKUPTHRESH1_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_WAN_CONFIGURATIONS_PDWKUPH_FIELDS[] =
{
    &BBH_TX_WAN_CONFIGURATIONS_PDWKUPH_WKUPTHRESH0_FIELD,
    &BBH_TX_WAN_CONFIGURATIONS_PDWKUPH_WKUPTHRESH1_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_TX_WAN_CONFIGURATIONS_PDWKUPH *****/
const ru_reg_rec BBH_TX_WAN_CONFIGURATIONS_PDWKUPH_REG =
{
    "WAN_CONFIGURATIONS_PDWKUPH",
#if RU_INCLUDE_DESC
    "PD_WKUP_THRESH Register",
    "When a FIFO occupancy is above this wakeup threshold, the BBH will not wake-up the Runner for sending a new PD. This threshold does not represent the actual size of the FIFO. If a PD will arrive from the Runner when the FIFO is above the threshold, it will not be dropped unless the FIFO is actually full.\nThis register defines the threshold of the 2 profiles.\n\n",
#endif
    { BBH_TX_WAN_CONFIGURATIONS_PDWKUPH_REG_OFFSET },
    0,
    0,
    127,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    BBH_TX_WAN_CONFIGURATIONS_PDWKUPH_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_TX_WAN_CONFIGURATIONS_PD_BYTE_TH, TYPE: Type_BBH_TX_BBHTX_WAN_CONFIGURATIONS_PD_BYTE_TH
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: PDLIMIT0 *****/
const ru_field_rec BBH_TX_WAN_CONFIGURATIONS_PD_BYTE_TH_PDLIMIT0_FIELD =
{
    "PDLIMIT0",
#if RU_INCLUDE_DESC
    "",
    "Defines the number of bytes for PDs pre fetch limited according to the total number of bytes.\nThe value is in 8-bytes resolution.\n\n",
#endif
    { BBH_TX_WAN_CONFIGURATIONS_PD_BYTE_TH_PDLIMIT0_FIELD_MASK },
    0,
    { BBH_TX_WAN_CONFIGURATIONS_PD_BYTE_TH_PDLIMIT0_FIELD_WIDTH },
    { BBH_TX_WAN_CONFIGURATIONS_PD_BYTE_TH_PDLIMIT0_FIELD_SHIFT },
    4095,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: PDLIMIT1 *****/
const ru_field_rec BBH_TX_WAN_CONFIGURATIONS_PD_BYTE_TH_PDLIMIT1_FIELD =
{
    "PDLIMIT1",
#if RU_INCLUDE_DESC
    "",
    "Defines the number of bytes for PDs pre fetch limited according to the total number of bytes.\nThe value is in 8-bytes resolution.\n\n",
#endif
    { BBH_TX_WAN_CONFIGURATIONS_PD_BYTE_TH_PDLIMIT1_FIELD_MASK },
    0,
    { BBH_TX_WAN_CONFIGURATIONS_PD_BYTE_TH_PDLIMIT1_FIELD_WIDTH },
    { BBH_TX_WAN_CONFIGURATIONS_PD_BYTE_TH_PDLIMIT1_FIELD_SHIFT },
    4095,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_WAN_CONFIGURATIONS_PD_BYTE_TH_FIELDS[] =
{
    &BBH_TX_WAN_CONFIGURATIONS_PD_BYTE_TH_PDLIMIT0_FIELD,
    &BBH_TX_WAN_CONFIGURATIONS_PD_BYTE_TH_PDLIMIT1_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_TX_WAN_CONFIGURATIONS_PD_BYTE_TH *****/
const ru_reg_rec BBH_TX_WAN_CONFIGURATIONS_PD_BYTE_TH_REG =
{
    "WAN_CONFIGURATIONS_PD_BYTE_TH",
#if RU_INCLUDE_DESC
    "PD_BYTES_THRESHOLD Register",
    "The BBH requests PDs from the Runner and maintains a pre-fetch PDs FIFO.\nThe PDs pre fetch is limited either by the PD FIFO configurable size or according to the total number of bytes (deducting bytes already requested/transmitted) for preventing HOL. This register defines the threshold of the 2 queue profiles.\n",
#endif
    { BBH_TX_WAN_CONFIGURATIONS_PD_BYTE_TH_REG_OFFSET },
    0,
    0,
    128,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    BBH_TX_WAN_CONFIGURATIONS_PD_BYTE_TH_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_TX_WAN_CONFIGURATIONS_QMQ, TYPE: Type_BBH_TX_BBHTX_WAN_CONFIGURATIONS_QMQ
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: Q0 *****/
const ru_field_rec BBH_TX_WAN_CONFIGURATIONS_QMQ_Q0_FIELD =
{
    "Q0",
#if RU_INCLUDE_DESC
    "",
    "Q0 configuration\n",
#endif
    { BBH_TX_WAN_CONFIGURATIONS_QMQ_Q0_FIELD_MASK },
    0,
    { BBH_TX_WAN_CONFIGURATIONS_QMQ_Q0_FIELD_WIDTH },
    { BBH_TX_WAN_CONFIGURATIONS_QMQ_Q0_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: Q1 *****/
const ru_field_rec BBH_TX_WAN_CONFIGURATIONS_QMQ_Q1_FIELD =
{
    "Q1",
#if RU_INCLUDE_DESC
    "",
    "Q1 configuration\n",
#endif
    { BBH_TX_WAN_CONFIGURATIONS_QMQ_Q1_FIELD_MASK },
    0,
    { BBH_TX_WAN_CONFIGURATIONS_QMQ_Q1_FIELD_WIDTH },
    { BBH_TX_WAN_CONFIGURATIONS_QMQ_Q1_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_WAN_CONFIGURATIONS_QMQ_FIELDS[] =
{
    &BBH_TX_WAN_CONFIGURATIONS_QMQ_Q0_FIELD,
    &BBH_TX_WAN_CONFIGURATIONS_QMQ_Q1_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_TX_WAN_CONFIGURATIONS_QMQ *****/
const ru_reg_rec BBH_TX_WAN_CONFIGURATIONS_QMQ_REG =
{
    "WAN_CONFIGURATIONS_QMQ",
#if RU_INCLUDE_DESC
    "QM_Q 0..19 Register",
    "This configuration determines whether the Q works with QM or with TM Runner.\n\nQM queues will not send wakeup and read pointer messages.\n\nEach register in this array configures 2 queues.\n",
#endif
    { BBH_TX_WAN_CONFIGURATIONS_QMQ_REG_OFFSET },
    BBH_TX_WAN_CONFIGURATIONS_QMQ_REG_RAM_CNT,
    4,
    129,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    BBH_TX_WAN_CONFIGURATIONS_QMQ_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_TX_WAN_CONFIGURATIONS_STSSIZE, TYPE: Type_BBH_TX_BBHTX_WAN_CONFIGURATIONS_STSSIZE
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: FIFOSIZE0 *****/
const ru_field_rec BBH_TX_WAN_CONFIGURATIONS_STSSIZE_FIFOSIZE0_FIELD =
{
    "FIFOSIZE0",
#if RU_INCLUDE_DESC
    "",
    "The size of PD FIFO for queue 0.\nA value of n refers to n+1.\nFor GPON, the max value is 0x7\nFor EPON, the max value is 0xf\n",
#endif
    { BBH_TX_WAN_CONFIGURATIONS_STSSIZE_FIFOSIZE0_FIELD_MASK },
    0,
    { BBH_TX_WAN_CONFIGURATIONS_STSSIZE_FIFOSIZE0_FIELD_WIDTH },
    { BBH_TX_WAN_CONFIGURATIONS_STSSIZE_FIFOSIZE0_FIELD_SHIFT },
    7,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FIFOSIZE1 *****/
const ru_field_rec BBH_TX_WAN_CONFIGURATIONS_STSSIZE_FIFOSIZE1_FIELD =
{
    "FIFOSIZE1",
#if RU_INCLUDE_DESC
    "",
    "The size of PD FIFO for queue 1.\nA value of n refers to n+1.\n\n",
#endif
    { BBH_TX_WAN_CONFIGURATIONS_STSSIZE_FIFOSIZE1_FIELD_MASK },
    0,
    { BBH_TX_WAN_CONFIGURATIONS_STSSIZE_FIFOSIZE1_FIELD_WIDTH },
    { BBH_TX_WAN_CONFIGURATIONS_STSSIZE_FIFOSIZE1_FIELD_SHIFT },
    7,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_WAN_CONFIGURATIONS_STSSIZE_FIELDS[] =
{
    &BBH_TX_WAN_CONFIGURATIONS_STSSIZE_FIFOSIZE0_FIELD,
    &BBH_TX_WAN_CONFIGURATIONS_STSSIZE_FIFOSIZE1_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_TX_WAN_CONFIGURATIONS_STSSIZE *****/
const ru_reg_rec BBH_TX_WAN_CONFIGURATIONS_STSSIZE_REG =
{
    "WAN_CONFIGURATIONS_STSSIZE",
#if RU_INCLUDE_DESC
    "STS_FIFO_SIZE Register",
    "The BBH manages 32 queues for EPON (FIFO per TCONT/LLID). For each queue it manages a sts FIFO.\nA total of 128 PDs are available for all queues.\nThere are 2 profiles of queues. For each profile the SW configures the size of the STS FIFO and then specifies for each queue which profile it is associated with.\nThis register defines the STS FIFO size of the 2 profiles.\n\n",
#endif
    { BBH_TX_WAN_CONFIGURATIONS_STSSIZE_REG_OFFSET },
    0,
    0,
    130,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    BBH_TX_WAN_CONFIGURATIONS_STSSIZE_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_TX_WAN_CONFIGURATIONS_STSWKUPH, TYPE: Type_BBH_TX_BBHTX_WAN_CONFIGURATIONS_STSWKUPH
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: WKUPTHRESH0 *****/
const ru_field_rec BBH_TX_WAN_CONFIGURATIONS_STSWKUPH_WKUPTHRESH0_FIELD =
{
    "WKUPTHRESH0",
#if RU_INCLUDE_DESC
    "",
    "The wakeup threshold of the PD FIFO for queue 0.\nA value of n refers to n+1.\nRelevant only for EPON BBH.\n",
#endif
    { BBH_TX_WAN_CONFIGURATIONS_STSWKUPH_WKUPTHRESH0_FIELD_MASK },
    0,
    { BBH_TX_WAN_CONFIGURATIONS_STSWKUPH_WKUPTHRESH0_FIELD_WIDTH },
    { BBH_TX_WAN_CONFIGURATIONS_STSWKUPH_WKUPTHRESH0_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: WKUPTHRESH1 *****/
const ru_field_rec BBH_TX_WAN_CONFIGURATIONS_STSWKUPH_WKUPTHRESH1_FIELD =
{
    "WKUPTHRESH1",
#if RU_INCLUDE_DESC
    "",
    "The wakeup threshold of the PD FIFO for queue 1.\nA value of n refers to n+1.\nRelevant only for EPON BBH.\n",
#endif
    { BBH_TX_WAN_CONFIGURATIONS_STSWKUPH_WKUPTHRESH1_FIELD_MASK },
    0,
    { BBH_TX_WAN_CONFIGURATIONS_STSWKUPH_WKUPTHRESH1_FIELD_WIDTH },
    { BBH_TX_WAN_CONFIGURATIONS_STSWKUPH_WKUPTHRESH1_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_WAN_CONFIGURATIONS_STSWKUPH_FIELDS[] =
{
    &BBH_TX_WAN_CONFIGURATIONS_STSWKUPH_WKUPTHRESH0_FIELD,
    &BBH_TX_WAN_CONFIGURATIONS_STSWKUPH_WKUPTHRESH1_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_TX_WAN_CONFIGURATIONS_STSWKUPH *****/
const ru_reg_rec BBH_TX_WAN_CONFIGURATIONS_STSWKUPH_REG =
{
    "WAN_CONFIGURATIONS_STSWKUPH",
#if RU_INCLUDE_DESC
    "STS_WKUP_THRESH Register",
    "When a FIFO occupancy is above this wakeup threshold, the BBH will not wake-up the Runner for sending a new PD. This threshold does not represent the actual size of the FIFO. If a STS will arrive from the Runner when the FIFO is above the threshold, it will not be dropped unless the FIFO is actually full.\nThis register defines the threshold of the 2 profiles.\n\n",
#endif
    { BBH_TX_WAN_CONFIGURATIONS_STSWKUPH_REG_OFFSET },
    0,
    0,
    131,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    BBH_TX_WAN_CONFIGURATIONS_STSWKUPH_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_TX_WAN_CONFIGURATIONS_PD_BYTE_TH_EN, TYPE: Type_BBH_TX_BBHTX_WAN_CONFIGURATIONS_PD_BYTE_TH_EN
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: PDLIMITEN *****/
const ru_field_rec BBH_TX_WAN_CONFIGURATIONS_PD_BYTE_TH_EN_PDLIMITEN_FIELD =
{
    "PDLIMITEN",
#if RU_INCLUDE_DESC
    "",
    "This bit enables the above feature (PDs pre fetch limited according to the total number of bytes).\n",
#endif
    { BBH_TX_WAN_CONFIGURATIONS_PD_BYTE_TH_EN_PDLIMITEN_FIELD_MASK },
    0,
    { BBH_TX_WAN_CONFIGURATIONS_PD_BYTE_TH_EN_PDLIMITEN_FIELD_WIDTH },
    { BBH_TX_WAN_CONFIGURATIONS_PD_BYTE_TH_EN_PDLIMITEN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_WAN_CONFIGURATIONS_PD_BYTE_TH_EN_FIELDS[] =
{
    &BBH_TX_WAN_CONFIGURATIONS_PD_BYTE_TH_EN_PDLIMITEN_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_TX_WAN_CONFIGURATIONS_PD_BYTE_TH_EN *****/
const ru_reg_rec BBH_TX_WAN_CONFIGURATIONS_PD_BYTE_TH_EN_REG =
{
    "WAN_CONFIGURATIONS_PD_BYTE_TH_EN",
#if RU_INCLUDE_DESC
    "PD_BYTES_THRESHOLD_EN Register",
    "The BBH requests PDs from the Runner and maintains a pre-fetch PDs FIFO.\nThe PDs pre fetch is limited either by the PD FIFO configurable size or according to the total number of bytes (deducting bytes already requested/transmitted) for preventing HOL. Full configuration for the first 8 TCONT and one configuration per group of 8 TCONTs for the rest.\n",
#endif
    { BBH_TX_WAN_CONFIGURATIONS_PD_BYTE_TH_EN_REG_OFFSET },
    0,
    0,
    132,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_TX_WAN_CONFIGURATIONS_PD_BYTE_TH_EN_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_TX_WAN_CONFIGURATIONS_PDEMPTY, TYPE: Type_BBH_TX_BBHTX_WAN_CONFIGURATIONS_PDEMPTY
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: EMPTY *****/
const ru_field_rec BBH_TX_WAN_CONFIGURATIONS_PDEMPTY_EMPTY_FIELD =
{
    "EMPTY",
#if RU_INCLUDE_DESC
    "",
    "EPON PD FIFO empty threshold.\nA queue which its PD FIFO occupancy is below this threshold will have high priority in PD ordering arbitration.\n",
#endif
    { BBH_TX_WAN_CONFIGURATIONS_PDEMPTY_EMPTY_FIELD_MASK },
    0,
    { BBH_TX_WAN_CONFIGURATIONS_PDEMPTY_EMPTY_FIELD_WIDTH },
    { BBH_TX_WAN_CONFIGURATIONS_PDEMPTY_EMPTY_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_WAN_CONFIGURATIONS_PDEMPTY_FIELDS[] =
{
    &BBH_TX_WAN_CONFIGURATIONS_PDEMPTY_EMPTY_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_TX_WAN_CONFIGURATIONS_PDEMPTY *****/
const ru_reg_rec BBH_TX_WAN_CONFIGURATIONS_PDEMPTY_REG =
{
    "WAN_CONFIGURATIONS_PDEMPTY",
#if RU_INCLUDE_DESC
    "PD_EMPTY_THRESHOLD Register",
    "The BBH manages 32 queues for EPON (FIFO per LLID). For each queue it manages a PD FIFO. Usually, the BBH orders PDs from the Runner in RR between all queues. In EPON BBH, if a FIFO occupancy is below this threshold, the queue will have higher priority in PD ordering arbitration (with RR between all the empty queues).\nThis configuration is global for all queues.\nRelevant only for EPON BBH.\n",
#endif
    { BBH_TX_WAN_CONFIGURATIONS_PDEMPTY_REG_OFFSET },
    0,
    0,
    133,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_TX_WAN_CONFIGURATIONS_PDEMPTY_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_TX_WAN_CONFIGURATIONS_STSEMPTY, TYPE: Type_BBH_TX_BBHTX_WAN_CONFIGURATIONS_STSEMPTY
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: EMPTY *****/
const ru_field_rec BBH_TX_WAN_CONFIGURATIONS_STSEMPTY_EMPTY_FIELD =
{
    "EMPTY",
#if RU_INCLUDE_DESC
    "",
    "EPON PD FIFO empty threshold.\nA queue which its PD FIFO occupancy is below this threshold will have high priority in PD ordering arbitration.\n",
#endif
    { BBH_TX_WAN_CONFIGURATIONS_STSEMPTY_EMPTY_FIELD_MASK },
    0,
    { BBH_TX_WAN_CONFIGURATIONS_STSEMPTY_EMPTY_FIELD_WIDTH },
    { BBH_TX_WAN_CONFIGURATIONS_STSEMPTY_EMPTY_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_WAN_CONFIGURATIONS_STSEMPTY_FIELDS[] =
{
    &BBH_TX_WAN_CONFIGURATIONS_STSEMPTY_EMPTY_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_TX_WAN_CONFIGURATIONS_STSEMPTY *****/
const ru_reg_rec BBH_TX_WAN_CONFIGURATIONS_STSEMPTY_REG =
{
    "WAN_CONFIGURATIONS_STSEMPTY",
#if RU_INCLUDE_DESC
    "STS_EMPTY_THRESHOLD Register",
    "The BBH manages 32 queues for EPON (FIFO per LLID). For each queue it manages a STS FIFO. Usually, the BBH orders STSs from the Runner in RR between all queues. In EPON BBH, if a FIFO occupancy is below this threshold, the queue will have higher priority in PD ordering arbitration (with RR between all the empty queues).\nThis configuration is global for all queues.\nRelevant only for EPON BBH.\n",
#endif
    { BBH_TX_WAN_CONFIGURATIONS_STSEMPTY_REG_OFFSET },
    0,
    0,
    134,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_TX_WAN_CONFIGURATIONS_STSEMPTY_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_TX_WAN_CONFIGURATIONS_STSRNRCFG_1, TYPE: Type_BBH_TX_BBHTX_WAN_CONFIGURATIONS_STSRNRCFG_1
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: TCONTADDR *****/
const ru_field_rec BBH_TX_WAN_CONFIGURATIONS_STSRNRCFG_1_TCONTADDR_FIELD =
{
    "TCONTADDR",
#if RU_INCLUDE_DESC
    "",
    "Defines the TCONT address within the Runner address space.\nThe address is in 8 bytes resolution.\n\n",
#endif
    { BBH_TX_WAN_CONFIGURATIONS_STSRNRCFG_1_TCONTADDR_FIELD_MASK },
    0,
    { BBH_TX_WAN_CONFIGURATIONS_STSRNRCFG_1_TCONTADDR_FIELD_WIDTH },
    { BBH_TX_WAN_CONFIGURATIONS_STSRNRCFG_1_TCONTADDR_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_WAN_CONFIGURATIONS_STSRNRCFG_1_FIELDS[] =
{
    &BBH_TX_WAN_CONFIGURATIONS_STSRNRCFG_1_TCONTADDR_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_TX_WAN_CONFIGURATIONS_STSRNRCFG_1 *****/
const ru_reg_rec BBH_TX_WAN_CONFIGURATIONS_STSRNRCFG_1_REG =
{
    "WAN_CONFIGURATIONS_STSRNRCFG_1",
#if RU_INCLUDE_DESC
    "STS_RNR_CFG_1 0..1 Register",
    "Queue index address:\nThe BBH requests a Packet descriptor from the Runner. The BBH writes the queue number in a predefined address at the Runner SRAM. The message serves also as a wake-up request to the Runner.\nThis register defines the queue index address within the Runner address space.\nSKB address:\nWhen the packet is transmitted from absolute address, then, instead of releasing the BN, the BBH writes a 6 bits read counter into the Runner SRAM. It writes it into a pre-defined address + TCONT_NUM (for Ethernet TCONT_NUM = 0).\nThis register defines the SKB free base address within the Runner address.\n\nNote: all addresses are in 8 byte resolution. As the Runner memory is limited to 12 bits address, use the 12 lsb bits.\n\n",
#endif
    { BBH_TX_WAN_CONFIGURATIONS_STSRNRCFG_1_REG_OFFSET },
    BBH_TX_WAN_CONFIGURATIONS_STSRNRCFG_1_REG_RAM_CNT,
    4,
    135,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_TX_WAN_CONFIGURATIONS_STSRNRCFG_1_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_TX_WAN_CONFIGURATIONS_STSRNRCFG_2, TYPE: Type_BBH_TX_BBHTX_WAN_CONFIGURATIONS_STSRNRCFG_2
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: PTRADDR *****/
const ru_field_rec BBH_TX_WAN_CONFIGURATIONS_STSRNRCFG_2_PTRADDR_FIELD =
{
    "PTRADDR",
#if RU_INCLUDE_DESC
    "",
    "This field defins the address in the Runner memory space to which the read pointer is written.\nThe address is in 8-bytes resolution.\n",
#endif
    { BBH_TX_WAN_CONFIGURATIONS_STSRNRCFG_2_PTRADDR_FIELD_MASK },
    0,
    { BBH_TX_WAN_CONFIGURATIONS_STSRNRCFG_2_PTRADDR_FIELD_WIDTH },
    { BBH_TX_WAN_CONFIGURATIONS_STSRNRCFG_2_PTRADDR_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: TASK *****/
const ru_field_rec BBH_TX_WAN_CONFIGURATIONS_STSRNRCFG_2_TASK_FIELD =
{
    "TASK",
#if RU_INCLUDE_DESC
    "",
    "The number of the task that is responsible for sending PDs to the BBH\n",
#endif
    { BBH_TX_WAN_CONFIGURATIONS_STSRNRCFG_2_TASK_FIELD_MASK },
    0,
    { BBH_TX_WAN_CONFIGURATIONS_STSRNRCFG_2_TASK_FIELD_WIDTH },
    { BBH_TX_WAN_CONFIGURATIONS_STSRNRCFG_2_TASK_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_WAN_CONFIGURATIONS_STSRNRCFG_2_FIELDS[] =
{
    &BBH_TX_WAN_CONFIGURATIONS_STSRNRCFG_2_PTRADDR_FIELD,
    &BBH_TX_WAN_CONFIGURATIONS_STSRNRCFG_2_TASK_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_TX_WAN_CONFIGURATIONS_STSRNRCFG_2 *****/
const ru_reg_rec BBH_TX_WAN_CONFIGURATIONS_STSRNRCFG_2_REG =
{
    "WAN_CONFIGURATIONS_STSRNRCFG_2",
#if RU_INCLUDE_DESC
    "STS_RNR_CFG_2 0..1 Register",
    "PD transfer process:\n-The Runner wont ACK the BBH; therefore the BBH wont wake the TX task.\n-The Runner will push the PDs into the BBH (without any wakeup from the BBH).\n-Each time that the BBH reads a PD from the PD FIFO, it will write the read pointer into a pre-defined address in the Runner. The pointer is 6 bits width (one bit larger than needed to distinguish between full and empty).\n-The Runner should manage the congestion over the PD FIFO (in the BBH) by reading the BBH read pointer prior to each PD write.\n-PD drop should be done by the Runner only. The BBH will drop PD when the FIFO is full and will count each drop. The BBH wont release the BN in this case.\n-There will be a full threshold, which can be smaller than the actual size of the FIFO. When the BBH will move from full to not full state, the BBH will wakeup the Runner.\n\nNote: all addresses are in 8 byte resolution. As the Runner memory is limited to 12 bits address, use the 12 lsb bits.\n\n",
#endif
    { BBH_TX_WAN_CONFIGURATIONS_STSRNRCFG_2_REG_OFFSET },
    BBH_TX_WAN_CONFIGURATIONS_STSRNRCFG_2_REG_RAM_CNT,
    4,
    136,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    BBH_TX_WAN_CONFIGURATIONS_STSRNRCFG_2_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_TX_WAN_CONFIGURATIONS_MSGRNRCFG_1, TYPE: Type_BBH_TX_BBHTX_WAN_CONFIGURATIONS_MSGRNRCFG_1
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: TCONTADDR *****/
const ru_field_rec BBH_TX_WAN_CONFIGURATIONS_MSGRNRCFG_1_TCONTADDR_FIELD =
{
    "TCONTADDR",
#if RU_INCLUDE_DESC
    "",
    "Defines the TCONT address within the Runner address space.\nThe address is in 8 bytes resolution.\n\n",
#endif
    { BBH_TX_WAN_CONFIGURATIONS_MSGRNRCFG_1_TCONTADDR_FIELD_MASK },
    0,
    { BBH_TX_WAN_CONFIGURATIONS_MSGRNRCFG_1_TCONTADDR_FIELD_WIDTH },
    { BBH_TX_WAN_CONFIGURATIONS_MSGRNRCFG_1_TCONTADDR_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_WAN_CONFIGURATIONS_MSGRNRCFG_1_FIELDS[] =
{
    &BBH_TX_WAN_CONFIGURATIONS_MSGRNRCFG_1_TCONTADDR_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_TX_WAN_CONFIGURATIONS_MSGRNRCFG_1 *****/
const ru_reg_rec BBH_TX_WAN_CONFIGURATIONS_MSGRNRCFG_1_REG =
{
    "WAN_CONFIGURATIONS_MSGRNRCFG_1",
#if RU_INCLUDE_DESC
    "MSG_RNR_CFG_1 0..1 Register",
    "Queue index address:\nThe BBH requests a Packet descriptor from the Runner. The BBH writes the queue number in a predefined address at the Runner SRAM. The message serves also as a wake-up request to the Runner.\nThis register defines the queue index address within the Runner address space.\nSKB address:\nWhen the packet is transmitted from absolute address, then, instead of releasing the BN, the BBH writes a 6 bits read counter into the Runner SRAM. It writes it into a pre-defined address + TCONT_NUM (for Ethernet TCONT_NUM = 0).\nThis register defines the SKB free base address within the Runner address.\n\nNote: all addresses are in 8 byte resolution. As the Runner memory is limited to 12 bits address, use the 12 lsb bits.\n\n",
#endif
    { BBH_TX_WAN_CONFIGURATIONS_MSGRNRCFG_1_REG_OFFSET },
    BBH_TX_WAN_CONFIGURATIONS_MSGRNRCFG_1_REG_RAM_CNT,
    4,
    137,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_TX_WAN_CONFIGURATIONS_MSGRNRCFG_1_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_TX_WAN_CONFIGURATIONS_MSGRNRCFG_2, TYPE: Type_BBH_TX_BBHTX_WAN_CONFIGURATIONS_MSGRNRCFG_2
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: PTRADDR *****/
const ru_field_rec BBH_TX_WAN_CONFIGURATIONS_MSGRNRCFG_2_PTRADDR_FIELD =
{
    "PTRADDR",
#if RU_INCLUDE_DESC
    "",
    "This field defins the address in the Runner memory space to which the read pointer is written.\nThe address is in 8-bytes resolution.\n",
#endif
    { BBH_TX_WAN_CONFIGURATIONS_MSGRNRCFG_2_PTRADDR_FIELD_MASK },
    0,
    { BBH_TX_WAN_CONFIGURATIONS_MSGRNRCFG_2_PTRADDR_FIELD_WIDTH },
    { BBH_TX_WAN_CONFIGURATIONS_MSGRNRCFG_2_PTRADDR_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: TASK *****/
const ru_field_rec BBH_TX_WAN_CONFIGURATIONS_MSGRNRCFG_2_TASK_FIELD =
{
    "TASK",
#if RU_INCLUDE_DESC
    "",
    "The number of the task that is responsible for sending PDs to the BBH\n",
#endif
    { BBH_TX_WAN_CONFIGURATIONS_MSGRNRCFG_2_TASK_FIELD_MASK },
    0,
    { BBH_TX_WAN_CONFIGURATIONS_MSGRNRCFG_2_TASK_FIELD_WIDTH },
    { BBH_TX_WAN_CONFIGURATIONS_MSGRNRCFG_2_TASK_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_WAN_CONFIGURATIONS_MSGRNRCFG_2_FIELDS[] =
{
    &BBH_TX_WAN_CONFIGURATIONS_MSGRNRCFG_2_PTRADDR_FIELD,
    &BBH_TX_WAN_CONFIGURATIONS_MSGRNRCFG_2_TASK_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_TX_WAN_CONFIGURATIONS_MSGRNRCFG_2 *****/
const ru_reg_rec BBH_TX_WAN_CONFIGURATIONS_MSGRNRCFG_2_REG =
{
    "WAN_CONFIGURATIONS_MSGRNRCFG_2",
#if RU_INCLUDE_DESC
    "MSG_RNR_CFG_2 0..1 Register",
    "PD transfer process:\n-The Runner wont ACK the BBH; therefore the BBH wont wake the TX task.\n-The Runner will push the PDs into the BBH (without any wakeup from the BBH).\n-Each time that the BBH reads a PD from the PD FIFO, it will write the read pointer into a pre-defined address in the Runner. The pointer is 6 bits width (one bit larger than needed to distinguish between full and empty).\n-The Runner should manage the congestion over the PD FIFO (in the BBH) by reading the BBH read pointer prior to each PD write.\n-PD drop should be done by the Runner only. The BBH will drop PD when the FIFO is full and will count each drop. The BBH wont release the BN in this case.\n-There will be a full threshold, which can be smaller than the actual size of the FIFO. When the BBH will move from full to not full state, the BBH will wakeup the Runner.\n\nNote: all addresses are in 8 byte resolution. As the Runner memory is limited to 12 bits address, use the 12 lsb bits.\n\n",
#endif
    { BBH_TX_WAN_CONFIGURATIONS_MSGRNRCFG_2_REG_OFFSET },
    BBH_TX_WAN_CONFIGURATIONS_MSGRNRCFG_2_REG_RAM_CNT,
    4,
    138,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    BBH_TX_WAN_CONFIGURATIONS_MSGRNRCFG_2_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_TX_WAN_CONFIGURATIONS_EPNCFG, TYPE: Type_BBH_TX_BBHTX_WAN_CONFIGURATIONS_EPNCFG
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: STPLENERR *****/
const ru_field_rec BBH_TX_WAN_CONFIGURATIONS_EPNCFG_STPLENERR_FIELD =
{
    "STPLENERR",
#if RU_INCLUDE_DESC
    "",
    "In case of fatal length error - a mismatch between the request message from MAC and its relevant PD from Runner - the BBH can stop performing or continue regardless of the error.\nThe error is also reflected to the SW in a counter.\n",
#endif
    { BBH_TX_WAN_CONFIGURATIONS_EPNCFG_STPLENERR_FIELD_MASK },
    0,
    { BBH_TX_WAN_CONFIGURATIONS_EPNCFG_STPLENERR_FIELD_WIDTH },
    { BBH_TX_WAN_CONFIGURATIONS_EPNCFG_STPLENERR_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: CMP_WIDTH *****/
const ru_field_rec BBH_TX_WAN_CONFIGURATIONS_EPNCFG_CMP_WIDTH_FIELD =
{
    "CMP_WIDTH",
#if RU_INCLUDE_DESC
    "",
    "configures the width of the comparison of the packet ength.\nThe length field in the EPON request interface is 11 bit, while it is 14 bit in the pd.\nIf this bit is 0, then the comparison of the length will be between the 11 bit of the interface and the 11 lsb bits of the pd.\nIf this ibt is 1, the comparison will be done between the 11 bits of the interface, concatenated with 3 zeros and the 14 bits of the pd\n",
#endif
    { BBH_TX_WAN_CONFIGURATIONS_EPNCFG_CMP_WIDTH_FIELD_MASK },
    0,
    { BBH_TX_WAN_CONFIGURATIONS_EPNCFG_CMP_WIDTH_FIELD_WIDTH },
    { BBH_TX_WAN_CONFIGURATIONS_EPNCFG_CMP_WIDTH_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: CONSIDERFULL *****/
const ru_field_rec BBH_TX_WAN_CONFIGURATIONS_EPNCFG_CONSIDERFULL_FIELD =
{
    "CONSIDERFULL",
#if RU_INCLUDE_DESC
    "",
    "determines whether the BBH will consider the sts_full vector state when pushing STS messages to the MAC or not.\n\nThe status fifos inside the MAC should never go full as they are mirror of the BBH PD FIFOs, but in cases where the MAC design behaves different than expected, we want the BBH to be able to operate as in 1G EPON mode\n",
#endif
    { BBH_TX_WAN_CONFIGURATIONS_EPNCFG_CONSIDERFULL_FIELD_MASK },
    0,
    { BBH_TX_WAN_CONFIGURATIONS_EPNCFG_CONSIDERFULL_FIELD_WIDTH },
    { BBH_TX_WAN_CONFIGURATIONS_EPNCFG_CONSIDERFULL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ADDCRC *****/
const ru_field_rec BBH_TX_WAN_CONFIGURATIONS_EPNCFG_ADDCRC_FIELD =
{
    "ADDCRC",
#if RU_INCLUDE_DESC
    "",
    "configuration whether to add 4 bytes per packet to the length received in the status message from the Runner so the MAC would know the actual length to be transmitted.\n",
#endif
    { BBH_TX_WAN_CONFIGURATIONS_EPNCFG_ADDCRC_FIELD_MASK },
    0,
    { BBH_TX_WAN_CONFIGURATIONS_EPNCFG_ADDCRC_FIELD_WIDTH },
    { BBH_TX_WAN_CONFIGURATIONS_EPNCFG_ADDCRC_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: REQ_FULL *****/
const ru_field_rec BBH_TX_WAN_CONFIGURATIONS_EPNCFG_REQ_FULL_FIELD =
{
    "REQ_FULL",
#if RU_INCLUDE_DESC
    "",
    "defines the depth of the req fifo.\nPhysically there are 8 entries in the FIFO, but this configuration changes the full indication towards the EPON MAC so the FIFO depth is reduced effectively.\n",
#endif
    { BBH_TX_WAN_CONFIGURATIONS_EPNCFG_REQ_FULL_FIELD_MASK },
    0,
    { BBH_TX_WAN_CONFIGURATIONS_EPNCFG_REQ_FULL_FIELD_WIDTH },
    { BBH_TX_WAN_CONFIGURATIONS_EPNCFG_REQ_FULL_FIELD_SHIFT },
    15,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: SENDRDPTR *****/
const ru_field_rec BBH_TX_WAN_CONFIGURATIONS_EPNCFG_SENDRDPTR_FIELD =
{
    "SENDRDPTR",
#if RU_INCLUDE_DESC
    "",
    "in xepon, the BBH generates the STS message from the PD so no need to send rd pointer to the runner, but we keep this option in case the runner would like to check that the STS fifo is not full before sending a PD. In epon, this bit should be set anyway.\n",
#endif
    { BBH_TX_WAN_CONFIGURATIONS_EPNCFG_SENDRDPTR_FIELD_MASK },
    0,
    { BBH_TX_WAN_CONFIGURATIONS_EPNCFG_SENDRDPTR_FIELD_WIDTH },
    { BBH_TX_WAN_CONFIGURATIONS_EPNCFG_SENDRDPTR_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_WAN_CONFIGURATIONS_EPNCFG_FIELDS[] =
{
    &BBH_TX_WAN_CONFIGURATIONS_EPNCFG_STPLENERR_FIELD,
    &BBH_TX_WAN_CONFIGURATIONS_EPNCFG_CMP_WIDTH_FIELD,
    &BBH_TX_WAN_CONFIGURATIONS_EPNCFG_CONSIDERFULL_FIELD,
    &BBH_TX_WAN_CONFIGURATIONS_EPNCFG_ADDCRC_FIELD,
    &BBH_TX_WAN_CONFIGURATIONS_EPNCFG_REQ_FULL_FIELD,
    &BBH_TX_WAN_CONFIGURATIONS_EPNCFG_SENDRDPTR_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_TX_WAN_CONFIGURATIONS_EPNCFG *****/
const ru_reg_rec BBH_TX_WAN_CONFIGURATIONS_EPNCFG_REG =
{
    "WAN_CONFIGURATIONS_EPNCFG",
#if RU_INCLUDE_DESC
    "EPN_CFG Register",
    "Configurations related to EPON MAC.\n",
#endif
    { BBH_TX_WAN_CONFIGURATIONS_EPNCFG_REG_OFFSET },
    0,
    0,
    139,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    6,
    BBH_TX_WAN_CONFIGURATIONS_EPNCFG_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_TX_WAN_CONFIGURATIONS_FLOW2PORT, TYPE: Type_BBH_TX_BBHTX_WAN_CONFIGURATIONS_FLOW2PORT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: WDATA *****/
const ru_field_rec BBH_TX_WAN_CONFIGURATIONS_FLOW2PORT_WDATA_FIELD =
{
    "WDATA",
#if RU_INCLUDE_DESC
    "",
    "write data.\n\n15:0 - port-id - default is 0x0000\n16 - regenerate CRC - enabled by default\n17 - enc enable - disabled by default\n18 - enable - enabled by default\n\n\n",
#endif
    { BBH_TX_WAN_CONFIGURATIONS_FLOW2PORT_WDATA_FIELD_MASK },
    0,
    { BBH_TX_WAN_CONFIGURATIONS_FLOW2PORT_WDATA_FIELD_WIDTH },
    { BBH_TX_WAN_CONFIGURATIONS_FLOW2PORT_WDATA_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: A *****/
const ru_field_rec BBH_TX_WAN_CONFIGURATIONS_FLOW2PORT_A_FIELD =
{
    "A",
#if RU_INCLUDE_DESC
    "",
    "address\n",
#endif
    { BBH_TX_WAN_CONFIGURATIONS_FLOW2PORT_A_FIELD_MASK },
    0,
    { BBH_TX_WAN_CONFIGURATIONS_FLOW2PORT_A_FIELD_WIDTH },
    { BBH_TX_WAN_CONFIGURATIONS_FLOW2PORT_A_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: CMD *****/
const ru_field_rec BBH_TX_WAN_CONFIGURATIONS_FLOW2PORT_CMD_FIELD =
{
    "CMD",
#if RU_INCLUDE_DESC
    "",
    "rd/wr cmd\n",
#endif
    { BBH_TX_WAN_CONFIGURATIONS_FLOW2PORT_CMD_FIELD_MASK },
    0,
    { BBH_TX_WAN_CONFIGURATIONS_FLOW2PORT_CMD_FIELD_WIDTH },
    { BBH_TX_WAN_CONFIGURATIONS_FLOW2PORT_CMD_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_WAN_CONFIGURATIONS_FLOW2PORT_FIELDS[] =
{
    &BBH_TX_WAN_CONFIGURATIONS_FLOW2PORT_WDATA_FIELD,
    &BBH_TX_WAN_CONFIGURATIONS_FLOW2PORT_A_FIELD,
    &BBH_TX_WAN_CONFIGURATIONS_FLOW2PORT_CMD_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_TX_WAN_CONFIGURATIONS_FLOW2PORT *****/
const ru_reg_rec BBH_TX_WAN_CONFIGURATIONS_FLOW2PORT_REG =
{
    "WAN_CONFIGURATIONS_FLOW2PORT",
#if RU_INCLUDE_DESC
    "FLOW2PORT Register",
    "interface for SW to access the flow id to port-id table.\nThe table entry:\n15:0 - port-id - default is 0x0000\n16 - regenerate CRC - enabled by default\n17 - enc enable - disabled by default\n18 - enable - enabled by default\n",
#endif
    { BBH_TX_WAN_CONFIGURATIONS_FLOW2PORT_REG_OFFSET },
    0,
    0,
    140,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    BBH_TX_WAN_CONFIGURATIONS_FLOW2PORT_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_TX_WAN_CONFIGURATIONS_TS, TYPE: Type_BBH_TX_BBHTX_WAN_CONFIGURATIONS_TS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: EN *****/
const ru_field_rec BBH_TX_WAN_CONFIGURATIONS_TS_EN_FIELD =
{
    "EN",
#if RU_INCLUDE_DESC
    "",
    "1588 enable\n",
#endif
    { BBH_TX_WAN_CONFIGURATIONS_TS_EN_FIELD_MASK },
    0,
    { BBH_TX_WAN_CONFIGURATIONS_TS_EN_FIELD_WIDTH },
    { BBH_TX_WAN_CONFIGURATIONS_TS_EN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_WAN_CONFIGURATIONS_TS_FIELDS[] =
{
    &BBH_TX_WAN_CONFIGURATIONS_TS_EN_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_TX_WAN_CONFIGURATIONS_TS *****/
const ru_reg_rec BBH_TX_WAN_CONFIGURATIONS_TS_REG =
{
    "WAN_CONFIGURATIONS_TS",
#if RU_INCLUDE_DESC
    "TS Register",
    "The BBH is responsible for indicating the EPON MAC that the current packet that is being transmitted is a 1588 paacket. The BBH gets the 1588 parameters in the PD and forward it to the MAC.\n\nThis register is used to enable this feature.\n",
#endif
    { BBH_TX_WAN_CONFIGURATIONS_TS_REG_OFFSET },
    0,
    0,
    141,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_TX_WAN_CONFIGURATIONS_TS_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_TX_WAN_CONFIGURATIONS_DSL_CFG, TYPE: Type_BBH_TX_BBHTX_WAN_CONFIGURATIONS_DSL_CFG
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: MAXWLEN *****/
const ru_field_rec BBH_TX_WAN_CONFIGURATIONS_DSL_CFG_MAXWLEN_FIELD =
{
    "MAXWLEN",
#if RU_INCLUDE_DESC
    "",
    "max_word_len\n",
#endif
    { BBH_TX_WAN_CONFIGURATIONS_DSL_CFG_MAXWLEN_FIELD_MASK },
    0,
    { BBH_TX_WAN_CONFIGURATIONS_DSL_CFG_MAXWLEN_FIELD_WIDTH },
    { BBH_TX_WAN_CONFIGURATIONS_DSL_CFG_MAXWLEN_FIELD_SHIFT },
    8,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: MIN_CREDIT *****/
const ru_field_rec BBH_TX_WAN_CONFIGURATIONS_DSL_CFG_MIN_CREDIT_FIELD =
{
    "MIN_CREDIT",
#if RU_INCLUDE_DESC
    "",
    "min credits that allow the bbh to order new chunk of data. Default is 18 as the max size of a chunk is 16 (128 bytes) + 2 PDs (in case of aggregation in DDR).\n",
#endif
    { BBH_TX_WAN_CONFIGURATIONS_DSL_CFG_MIN_CREDIT_FIELD_MASK },
    0,
    { BBH_TX_WAN_CONFIGURATIONS_DSL_CFG_MIN_CREDIT_FIELD_WIDTH },
    { BBH_TX_WAN_CONFIGURATIONS_DSL_CFG_MIN_CREDIT_FIELD_SHIFT },
    20,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: SPARE *****/
const ru_field_rec BBH_TX_WAN_CONFIGURATIONS_DSL_CFG_SPARE_FIELD =
{
    "SPARE",
#if RU_INCLUDE_DESC
    "",
    "spare\n",
#endif
    { BBH_TX_WAN_CONFIGURATIONS_DSL_CFG_SPARE_FIELD_MASK },
    0,
    { BBH_TX_WAN_CONFIGURATIONS_DSL_CFG_SPARE_FIELD_WIDTH },
    { BBH_TX_WAN_CONFIGURATIONS_DSL_CFG_SPARE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: PRIO_EN *****/
const ru_field_rec BBH_TX_WAN_CONFIGURATIONS_DSL_CFG_PRIO_EN_FIELD =
{
    "PRIO_EN",
#if RU_INCLUDE_DESC
    "",
    "enables the priority mechanism (q_in_burst)\n",
#endif
    { BBH_TX_WAN_CONFIGURATIONS_DSL_CFG_PRIO_EN_FIELD_MASK },
    0,
    { BBH_TX_WAN_CONFIGURATIONS_DSL_CFG_PRIO_EN_FIELD_WIDTH },
    { BBH_TX_WAN_CONFIGURATIONS_DSL_CFG_PRIO_EN_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: SRST_N *****/
const ru_field_rec BBH_TX_WAN_CONFIGURATIONS_DSL_CFG_SRST_N_FIELD =
{
    "SRST_N",
#if RU_INCLUDE_DESC
    "",
    "soft reset\n",
#endif
    { BBH_TX_WAN_CONFIGURATIONS_DSL_CFG_SRST_N_FIELD_MASK },
    0,
    { BBH_TX_WAN_CONFIGURATIONS_DSL_CFG_SRST_N_FIELD_WIDTH },
    { BBH_TX_WAN_CONFIGURATIONS_DSL_CFG_SRST_N_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_WAN_CONFIGURATIONS_DSL_CFG_FIELDS[] =
{
    &BBH_TX_WAN_CONFIGURATIONS_DSL_CFG_MAXWLEN_FIELD,
    &BBH_TX_WAN_CONFIGURATIONS_DSL_CFG_MIN_CREDIT_FIELD,
    &BBH_TX_WAN_CONFIGURATIONS_DSL_CFG_SPARE_FIELD,
    &BBH_TX_WAN_CONFIGURATIONS_DSL_CFG_PRIO_EN_FIELD,
    &BBH_TX_WAN_CONFIGURATIONS_DSL_CFG_SRST_N_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_TX_WAN_CONFIGURATIONS_DSL_CFG *****/
const ru_reg_rec BBH_TX_WAN_CONFIGURATIONS_DSL_CFG_REG =
{
    "WAN_CONFIGURATIONS_DSL_CFG",
#if RU_INCLUDE_DESC
    "DSL_CFG Register",
    "dsl configurations\n\nrelevant only for VDSL BBH\n",
#endif
    { BBH_TX_WAN_CONFIGURATIONS_DSL_CFG_REG_OFFSET },
    0,
    0,
    142,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    5,
    BBH_TX_WAN_CONFIGURATIONS_DSL_CFG_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_TX_WAN_CONFIGURATIONS_DSL_CFG2, TYPE: Type_BBH_TX_BBHTX_WAN_CONFIGURATIONS_DSL_CFG2
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: FLUSH_WAIT_CYCLES *****/
const ru_field_rec BBH_TX_WAN_CONFIGURATIONS_DSL_CFG2_FLUSH_WAIT_CYCLES_FIELD =
{
    "FLUSH_WAIT_CYCLES",
#if RU_INCLUDE_DESC
    "",
    "option to delay the flush done indication towards the VDSL MAC.\nThe field indicates the number of cycles to wait before flush done, in 1024 cycles resolution.\n",
#endif
    { BBH_TX_WAN_CONFIGURATIONS_DSL_CFG2_FLUSH_WAIT_CYCLES_FIELD_MASK },
    0,
    { BBH_TX_WAN_CONFIGURATIONS_DSL_CFG2_FLUSH_WAIT_CYCLES_FIELD_WIDTH },
    { BBH_TX_WAN_CONFIGURATIONS_DSL_CFG2_FLUSH_WAIT_CYCLES_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: SW_FLUSH_DONE *****/
const ru_field_rec BBH_TX_WAN_CONFIGURATIONS_DSL_CFG2_SW_FLUSH_DONE_FIELD =
{
    "SW_FLUSH_DONE",
#if RU_INCLUDE_DESC
    "",
    "indication from the SW to end the flush sequence.\n\nThe HW will send flush done indication to the VDSL MAC once the HW flush done condition is met and this bit is set to 1.\nIt is SW responsibility to de-assert this bit once the flush sequence is ended and before a new flush sequence.\n",
#endif
    { BBH_TX_WAN_CONFIGURATIONS_DSL_CFG2_SW_FLUSH_DONE_FIELD_MASK },
    0,
    { BBH_TX_WAN_CONFIGURATIONS_DSL_CFG2_SW_FLUSH_DONE_FIELD_WIDTH },
    { BBH_TX_WAN_CONFIGURATIONS_DSL_CFG2_SW_FLUSH_DONE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FLUSH_WAIT_EN *****/
const ru_field_rec BBH_TX_WAN_CONFIGURATIONS_DSL_CFG2_FLUSH_WAIT_EN_FIELD =
{
    "FLUSH_WAIT_EN",
#if RU_INCLUDE_DESC
    "",
    "enables the options to wait after flush done condition is met.\none option is to wait for the delay counter to end. The second option is to wait for SW to end the flush sequence.\nThe SW has visibility of the HW flush state.\n",
#endif
    { BBH_TX_WAN_CONFIGURATIONS_DSL_CFG2_FLUSH_WAIT_EN_FIELD_MASK },
    0,
    { BBH_TX_WAN_CONFIGURATIONS_DSL_CFG2_FLUSH_WAIT_EN_FIELD_WIDTH },
    { BBH_TX_WAN_CONFIGURATIONS_DSL_CFG2_FLUSH_WAIT_EN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: SW_FLUSH_REQ *****/
const ru_field_rec BBH_TX_WAN_CONFIGURATIONS_DSL_CFG2_SW_FLUSH_REQ_FIELD =
{
    "SW_FLUSH_REQ",
#if RU_INCLUDE_DESC
    "",
    "request from the SW to enter flush state\n",
#endif
    { BBH_TX_WAN_CONFIGURATIONS_DSL_CFG2_SW_FLUSH_REQ_FIELD_MASK },
    0,
    { BBH_TX_WAN_CONFIGURATIONS_DSL_CFG2_SW_FLUSH_REQ_FIELD_WIDTH },
    { BBH_TX_WAN_CONFIGURATIONS_DSL_CFG2_SW_FLUSH_REQ_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FLUSH_IGNORE_RD *****/
const ru_field_rec BBH_TX_WAN_CONFIGURATIONS_DSL_CFG2_FLUSH_IGNORE_RD_FIELD =
{
    "FLUSH_IGNORE_RD",
#if RU_INCLUDE_DESC
    "",
    "ignore DSL MAC reads during flush, meaning the BBH TX will push data to the MAC regardless of the read state\n",
#endif
    { BBH_TX_WAN_CONFIGURATIONS_DSL_CFG2_FLUSH_IGNORE_RD_FIELD_MASK },
    0,
    { BBH_TX_WAN_CONFIGURATIONS_DSL_CFG2_FLUSH_IGNORE_RD_FIELD_WIDTH },
    { BBH_TX_WAN_CONFIGURATIONS_DSL_CFG2_FLUSH_IGNORE_RD_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: SW_CRDTS_VAL *****/
const ru_field_rec BBH_TX_WAN_CONFIGURATIONS_DSL_CFG2_SW_CRDTS_VAL_FIELD =
{
    "SW_CRDTS_VAL",
#if RU_INCLUDE_DESC
    "",
    "The number of credits per DSL channel in case the SW initializes the credits counters.\n",
#endif
    { BBH_TX_WAN_CONFIGURATIONS_DSL_CFG2_SW_CRDTS_VAL_FIELD_MASK },
    0,
    { BBH_TX_WAN_CONFIGURATIONS_DSL_CFG2_SW_CRDTS_VAL_FIELD_WIDTH },
    { BBH_TX_WAN_CONFIGURATIONS_DSL_CFG2_SW_CRDTS_VAL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: SW_CRDTS_INIT *****/
const ru_field_rec BBH_TX_WAN_CONFIGURATIONS_DSL_CFG2_SW_CRDTS_INIT_FIELD =
{
    "SW_CRDTS_INIT",
#if RU_INCLUDE_DESC
    "",
    "indication for the HW to initialize the credits counters. The HW will identify assertion of this bit, so the SW is responsible for de-assertion before the next init.\n",
#endif
    { BBH_TX_WAN_CONFIGURATIONS_DSL_CFG2_SW_CRDTS_INIT_FIELD_MASK },
    0,
    { BBH_TX_WAN_CONFIGURATIONS_DSL_CFG2_SW_CRDTS_INIT_FIELD_WIDTH },
    { BBH_TX_WAN_CONFIGURATIONS_DSL_CFG2_SW_CRDTS_INIT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_WAN_CONFIGURATIONS_DSL_CFG2_FIELDS[] =
{
    &BBH_TX_WAN_CONFIGURATIONS_DSL_CFG2_FLUSH_WAIT_CYCLES_FIELD,
    &BBH_TX_WAN_CONFIGURATIONS_DSL_CFG2_SW_FLUSH_DONE_FIELD,
    &BBH_TX_WAN_CONFIGURATIONS_DSL_CFG2_FLUSH_WAIT_EN_FIELD,
    &BBH_TX_WAN_CONFIGURATIONS_DSL_CFG2_SW_FLUSH_REQ_FIELD,
    &BBH_TX_WAN_CONFIGURATIONS_DSL_CFG2_FLUSH_IGNORE_RD_FIELD,
    &BBH_TX_WAN_CONFIGURATIONS_DSL_CFG2_SW_CRDTS_VAL_FIELD,
    &BBH_TX_WAN_CONFIGURATIONS_DSL_CFG2_SW_CRDTS_INIT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_TX_WAN_CONFIGURATIONS_DSL_CFG2 *****/
const ru_reg_rec BBH_TX_WAN_CONFIGURATIONS_DSL_CFG2_REG =
{
    "WAN_CONFIGURATIONS_DSL_CFG2",
#if RU_INCLUDE_DESC
    "DSL_CFG_2 Register",
    "dsl configurations\n\nrelevant only for VDSL BBH\n",
#endif
    { BBH_TX_WAN_CONFIGURATIONS_DSL_CFG2_REG_OFFSET },
    0,
    0,
    143,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    7,
    BBH_TX_WAN_CONFIGURATIONS_DSL_CFG2_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_TX_WAN_CONFIGURATIONS_GEMCTRINIT, TYPE: Type_BBH_TX_BBHTX_WAN_CONFIGURATIONS_GEMCTRINIT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: INIT *****/
const ru_field_rec BBH_TX_WAN_CONFIGURATIONS_GEMCTRINIT_INIT_FIELD =
{
    "INIT",
#if RU_INCLUDE_DESC
    "",
    "when asserted, the HW will start initializing the counters with value of 0. Should not be done during traffic.\n",
#endif
    { BBH_TX_WAN_CONFIGURATIONS_GEMCTRINIT_INIT_FIELD_MASK },
    0,
    { BBH_TX_WAN_CONFIGURATIONS_GEMCTRINIT_INIT_FIELD_WIDTH },
    { BBH_TX_WAN_CONFIGURATIONS_GEMCTRINIT_INIT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: INITDONE *****/
const ru_field_rec BBH_TX_WAN_CONFIGURATIONS_GEMCTRINIT_INITDONE_FIELD =
{
    "INITDONE",
#if RU_INCLUDE_DESC
    "",
    "Asserted by the HW, this bit indicates the HW finished initializing the counters with value of 0.\n",
#endif
    { BBH_TX_WAN_CONFIGURATIONS_GEMCTRINIT_INITDONE_FIELD_MASK },
    0,
    { BBH_TX_WAN_CONFIGURATIONS_GEMCTRINIT_INITDONE_FIELD_WIDTH },
    { BBH_TX_WAN_CONFIGURATIONS_GEMCTRINIT_INITDONE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_WAN_CONFIGURATIONS_GEMCTRINIT_FIELDS[] =
{
    &BBH_TX_WAN_CONFIGURATIONS_GEMCTRINIT_INIT_FIELD,
    &BBH_TX_WAN_CONFIGURATIONS_GEMCTRINIT_INITDONE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_TX_WAN_CONFIGURATIONS_GEMCTRINIT *****/
const ru_reg_rec BBH_TX_WAN_CONFIGURATIONS_GEMCTRINIT_REG =
{
    "WAN_CONFIGURATIONS_GEMCTRINIT",
#if RU_INCLUDE_DESC
    "GEM_COUNTERS_INIT Register",
    "init configuration\n",
#endif
    { BBH_TX_WAN_CONFIGURATIONS_GEMCTRINIT_REG_OFFSET },
    0,
    0,
    144,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    BBH_TX_WAN_CONFIGURATIONS_GEMCTRINIT_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_TX_WAN_CONFIGURATIONS_GEMCTRRD, TYPE: Type_BBH_TX_BBHTX_WAN_CONFIGURATIONS_GEMCTRRD
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: RDADDRESS *****/
const ru_field_rec BBH_TX_WAN_CONFIGURATIONS_GEMCTRRD_RDADDRESS_FIELD =
{
    "RDADDRESS",
#if RU_INCLUDE_DESC
    "",
    "the counter to be read\n",
#endif
    { BBH_TX_WAN_CONFIGURATIONS_GEMCTRRD_RDADDRESS_FIELD_MASK },
    0,
    { BBH_TX_WAN_CONFIGURATIONS_GEMCTRRD_RDADDRESS_FIELD_WIDTH },
    { BBH_TX_WAN_CONFIGURATIONS_GEMCTRRD_RDADDRESS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: RD *****/
const ru_field_rec BBH_TX_WAN_CONFIGURATIONS_GEMCTRRD_RD_FIELD =
{
    "RD",
#if RU_INCLUDE_DESC
    "",
    "rd\n",
#endif
    { BBH_TX_WAN_CONFIGURATIONS_GEMCTRRD_RD_FIELD_MASK },
    0,
    { BBH_TX_WAN_CONFIGURATIONS_GEMCTRRD_RD_FIELD_WIDTH },
    { BBH_TX_WAN_CONFIGURATIONS_GEMCTRRD_RD_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_WAN_CONFIGURATIONS_GEMCTRRD_FIELDS[] =
{
    &BBH_TX_WAN_CONFIGURATIONS_GEMCTRRD_RDADDRESS_FIELD,
    &BBH_TX_WAN_CONFIGURATIONS_GEMCTRRD_RD_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_TX_WAN_CONFIGURATIONS_GEMCTRRD *****/
const ru_reg_rec BBH_TX_WAN_CONFIGURATIONS_GEMCTRRD_REG =
{
    "WAN_CONFIGURATIONS_GEMCTRRD",
#if RU_INCLUDE_DESC
    "GEM_COUNTERS_RD Register",
    "the GEM counters are a set of 130 dual counters. each pair contains 28bit packet counter and 36 bits of bytes counter. The pair is always being read together.\nThe counters are read clear.\n",
#endif
    { BBH_TX_WAN_CONFIGURATIONS_GEMCTRRD_REG_OFFSET },
    0,
    0,
    145,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    BBH_TX_WAN_CONFIGURATIONS_GEMCTRRD_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_TX_WAN_CONFIGURATIONS_GEMCTRRD0, TYPE: Type_BBH_TX_BBHTX_WAN_CONFIGURATIONS_GEMCTRRD0
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: RDDATA *****/
const ru_field_rec BBH_TX_WAN_CONFIGURATIONS_GEMCTRRD0_RDDATA_FIELD =
{
    "RDDATA",
#if RU_INCLUDE_DESC
    "",
    "read data:\npkts count 27:0, byte cnt 35:32\n",
#endif
    { BBH_TX_WAN_CONFIGURATIONS_GEMCTRRD0_RDDATA_FIELD_MASK },
    0,
    { BBH_TX_WAN_CONFIGURATIONS_GEMCTRRD0_RDDATA_FIELD_WIDTH },
    { BBH_TX_WAN_CONFIGURATIONS_GEMCTRRD0_RDDATA_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_WAN_CONFIGURATIONS_GEMCTRRD0_FIELDS[] =
{
    &BBH_TX_WAN_CONFIGURATIONS_GEMCTRRD0_RDDATA_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_TX_WAN_CONFIGURATIONS_GEMCTRRD0 *****/
const ru_reg_rec BBH_TX_WAN_CONFIGURATIONS_GEMCTRRD0_REG =
{
    "WAN_CONFIGURATIONS_GEMCTRRD0",
#if RU_INCLUDE_DESC
    "GEM_COUTERS_READ_DATA_0 Register",
    "upper 32 bits of the pair:\n{pkt count [27:0], byte count [35:32]}\n",
#endif
    { BBH_TX_WAN_CONFIGURATIONS_GEMCTRRD0_REG_OFFSET },
    0,
    0,
    146,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_TX_WAN_CONFIGURATIONS_GEMCTRRD0_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_TX_WAN_CONFIGURATIONS_GEMCTRRD1, TYPE: Type_BBH_TX_BBHTX_WAN_CONFIGURATIONS_GEMCTRRD1
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: RDDATA *****/
const ru_field_rec BBH_TX_WAN_CONFIGURATIONS_GEMCTRRD1_RDDATA_FIELD =
{
    "RDDATA",
#if RU_INCLUDE_DESC
    "",
    "read data:\nbyte cnt 31:0\n",
#endif
    { BBH_TX_WAN_CONFIGURATIONS_GEMCTRRD1_RDDATA_FIELD_MASK },
    0,
    { BBH_TX_WAN_CONFIGURATIONS_GEMCTRRD1_RDDATA_FIELD_WIDTH },
    { BBH_TX_WAN_CONFIGURATIONS_GEMCTRRD1_RDDATA_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_WAN_CONFIGURATIONS_GEMCTRRD1_FIELDS[] =
{
    &BBH_TX_WAN_CONFIGURATIONS_GEMCTRRD1_RDDATA_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_TX_WAN_CONFIGURATIONS_GEMCTRRD1 *****/
const ru_reg_rec BBH_TX_WAN_CONFIGURATIONS_GEMCTRRD1_REG =
{
    "WAN_CONFIGURATIONS_GEMCTRRD1",
#if RU_INCLUDE_DESC
    "GEM_COUTERS_READ_DATA_1 Register",
    "lower 32 bits of the pair:\nbyte count [31:0]\n",
#endif
    { BBH_TX_WAN_CONFIGURATIONS_GEMCTRRD1_REG_OFFSET },
    0,
    0,
    147,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_TX_WAN_CONFIGURATIONS_GEMCTRRD1_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_TX_UNIFIED_CONFIGURATIONS_Q2RNR, TYPE: Type_BBH_TX_BBHTX_UNIFIED_CONFIGURATIONS_Q2RNR
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: Q0 *****/
const ru_field_rec BBH_TX_UNIFIED_CONFIGURATIONS_Q2RNR_Q0_FIELD =
{
    "Q0",
#if RU_INCLUDE_DESC
    "",
    "Q0 configuration\n",
#endif
    { BBH_TX_UNIFIED_CONFIGURATIONS_Q2RNR_Q0_FIELD_MASK },
    0,
    { BBH_TX_UNIFIED_CONFIGURATIONS_Q2RNR_Q0_FIELD_WIDTH },
    { BBH_TX_UNIFIED_CONFIGURATIONS_Q2RNR_Q0_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: Q1 *****/
const ru_field_rec BBH_TX_UNIFIED_CONFIGURATIONS_Q2RNR_Q1_FIELD =
{
    "Q1",
#if RU_INCLUDE_DESC
    "",
    "Q1 configuration\n",
#endif
    { BBH_TX_UNIFIED_CONFIGURATIONS_Q2RNR_Q1_FIELD_MASK },
    0,
    { BBH_TX_UNIFIED_CONFIGURATIONS_Q2RNR_Q1_FIELD_WIDTH },
    { BBH_TX_UNIFIED_CONFIGURATIONS_Q2RNR_Q1_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_UNIFIED_CONFIGURATIONS_Q2RNR_FIELDS[] =
{
    &BBH_TX_UNIFIED_CONFIGURATIONS_Q2RNR_Q0_FIELD,
    &BBH_TX_UNIFIED_CONFIGURATIONS_Q2RNR_Q1_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_TX_UNIFIED_CONFIGURATIONS_Q2RNR *****/
const ru_reg_rec BBH_TX_UNIFIED_CONFIGURATIONS_Q2RNR_REG =
{
    "UNIFIED_CONFIGURATIONS_Q2RNR",
#if RU_INCLUDE_DESC
    "Q_TO_RNR 0..3 Register",
    "configuration which queue is managed by each of the two runners.\n\nEach register in this array configures 2 queues.\n",
#endif
    { BBH_TX_UNIFIED_CONFIGURATIONS_Q2RNR_REG_OFFSET },
    BBH_TX_UNIFIED_CONFIGURATIONS_Q2RNR_REG_RAM_CNT,
    4,
    148,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    BBH_TX_UNIFIED_CONFIGURATIONS_Q2RNR_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_TX_UNIFIED_CONFIGURATIONS_CHKSUMQ, TYPE: Type_BBH_TX_BBHTX_UNIFIED_CONFIGURATIONS_CHKSUMQ
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: Q0 *****/
const ru_field_rec BBH_TX_UNIFIED_CONFIGURATIONS_CHKSUMQ_Q0_FIELD =
{
    "Q0",
#if RU_INCLUDE_DESC
    "",
    "Q0 configuration\n",
#endif
    { BBH_TX_UNIFIED_CONFIGURATIONS_CHKSUMQ_Q0_FIELD_MASK },
    0,
    { BBH_TX_UNIFIED_CONFIGURATIONS_CHKSUMQ_Q0_FIELD_WIDTH },
    { BBH_TX_UNIFIED_CONFIGURATIONS_CHKSUMQ_Q0_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: Q1 *****/
const ru_field_rec BBH_TX_UNIFIED_CONFIGURATIONS_CHKSUMQ_Q1_FIELD =
{
    "Q1",
#if RU_INCLUDE_DESC
    "",
    "Q1 configuration\n",
#endif
    { BBH_TX_UNIFIED_CONFIGURATIONS_CHKSUMQ_Q1_FIELD_MASK },
    0,
    { BBH_TX_UNIFIED_CONFIGURATIONS_CHKSUMQ_Q1_FIELD_WIDTH },
    { BBH_TX_UNIFIED_CONFIGURATIONS_CHKSUMQ_Q1_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_UNIFIED_CONFIGURATIONS_CHKSUMQ_FIELDS[] =
{
    &BBH_TX_UNIFIED_CONFIGURATIONS_CHKSUMQ_Q0_FIELD,
    &BBH_TX_UNIFIED_CONFIGURATIONS_CHKSUMQ_Q1_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_TX_UNIFIED_CONFIGURATIONS_CHKSUMQ *****/
const ru_reg_rec BBH_TX_UNIFIED_CONFIGURATIONS_CHKSUMQ_REG =
{
    "UNIFIED_CONFIGURATIONS_CHKSUMQ",
#if RU_INCLUDE_DESC
    "CHECKSUM_Q 0..3 Register",
    "configuration whether the queue is used for transmission or checksum calculation.\nIn case of checksum calculation, the read data from sram/ddr will be dropped and the buffers will not be freed.\n",
#endif
    { BBH_TX_UNIFIED_CONFIGURATIONS_CHKSUMQ_REG_OFFSET },
    BBH_TX_UNIFIED_CONFIGURATIONS_CHKSUMQ_REG_RAM_CNT,
    4,
    149,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    BBH_TX_UNIFIED_CONFIGURATIONS_CHKSUMQ_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_TX_UNIFIED_CONFIGURATIONS_QPROF, TYPE: Type_BBH_TX_BBHTX_UNIFIED_CONFIGURATIONS_QPROF
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: Q0 *****/
const ru_field_rec BBH_TX_UNIFIED_CONFIGURATIONS_QPROF_Q0_FIELD =
{
    "Q0",
#if RU_INCLUDE_DESC
    "",
    "Q0 configuration\n",
#endif
    { BBH_TX_UNIFIED_CONFIGURATIONS_QPROF_Q0_FIELD_MASK },
    0,
    { BBH_TX_UNIFIED_CONFIGURATIONS_QPROF_Q0_FIELD_WIDTH },
    { BBH_TX_UNIFIED_CONFIGURATIONS_QPROF_Q0_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: Q1 *****/
const ru_field_rec BBH_TX_UNIFIED_CONFIGURATIONS_QPROF_Q1_FIELD =
{
    "Q1",
#if RU_INCLUDE_DESC
    "",
    "Q1 configuration\n",
#endif
    { BBH_TX_UNIFIED_CONFIGURATIONS_QPROF_Q1_FIELD_MASK },
    0,
    { BBH_TX_UNIFIED_CONFIGURATIONS_QPROF_Q1_FIELD_WIDTH },
    { BBH_TX_UNIFIED_CONFIGURATIONS_QPROF_Q1_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DIS0 *****/
const ru_field_rec BBH_TX_UNIFIED_CONFIGURATIONS_QPROF_DIS0_FIELD =
{
    "DIS0",
#if RU_INCLUDE_DESC
    "",
    "disable q 0\n",
#endif
    { BBH_TX_UNIFIED_CONFIGURATIONS_QPROF_DIS0_FIELD_MASK },
    0,
    { BBH_TX_UNIFIED_CONFIGURATIONS_QPROF_DIS0_FIELD_WIDTH },
    { BBH_TX_UNIFIED_CONFIGURATIONS_QPROF_DIS0_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DIS1 *****/
const ru_field_rec BBH_TX_UNIFIED_CONFIGURATIONS_QPROF_DIS1_FIELD =
{
    "DIS1",
#if RU_INCLUDE_DESC
    "",
    "disable q 1\n",
#endif
    { BBH_TX_UNIFIED_CONFIGURATIONS_QPROF_DIS1_FIELD_MASK },
    0,
    { BBH_TX_UNIFIED_CONFIGURATIONS_QPROF_DIS1_FIELD_WIDTH },
    { BBH_TX_UNIFIED_CONFIGURATIONS_QPROF_DIS1_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_UNIFIED_CONFIGURATIONS_QPROF_FIELDS[] =
{
    &BBH_TX_UNIFIED_CONFIGURATIONS_QPROF_Q0_FIELD,
    &BBH_TX_UNIFIED_CONFIGURATIONS_QPROF_Q1_FIELD,
    &BBH_TX_UNIFIED_CONFIGURATIONS_QPROF_DIS0_FIELD,
    &BBH_TX_UNIFIED_CONFIGURATIONS_QPROF_DIS1_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_TX_UNIFIED_CONFIGURATIONS_QPROF *****/
const ru_reg_rec BBH_TX_UNIFIED_CONFIGURATIONS_QPROF_REG =
{
    "UNIFIED_CONFIGURATIONS_QPROF",
#if RU_INCLUDE_DESC
    "PER_Q_PROFILE 0..3 Register",
    "configuration of the profile per queue.\nThe profile determines the PD FIFO size, the wakeup threshold and bytes threshold.\n\nEach register in this array configures 2 queues.\n",
#endif
    { BBH_TX_UNIFIED_CONFIGURATIONS_QPROF_REG_OFFSET },
    BBH_TX_UNIFIED_CONFIGURATIONS_QPROF_REG_RAM_CNT,
    4,
    150,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    BBH_TX_UNIFIED_CONFIGURATIONS_QPROF_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_TX_UNIFIED_CONFIGURATIONS_PDSIZE, TYPE: Type_BBH_TX_BBHTX_UNIFIED_CONFIGURATIONS_PDSIZE
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: FIFOSIZE0 *****/
const ru_field_rec BBH_TX_UNIFIED_CONFIGURATIONS_PDSIZE_FIFOSIZE0_FIELD =
{
    "FIFOSIZE0",
#if RU_INCLUDE_DESC
    "",
    "The size of PD FIFO for queue 0.\nA value of n refers to n+1.\nFor GPON, the max value is 0x7\nFor EPON, the max value is 0xf\n",
#endif
    { BBH_TX_UNIFIED_CONFIGURATIONS_PDSIZE_FIFOSIZE0_FIELD_MASK },
    0,
    { BBH_TX_UNIFIED_CONFIGURATIONS_PDSIZE_FIFOSIZE0_FIELD_WIDTH },
    { BBH_TX_UNIFIED_CONFIGURATIONS_PDSIZE_FIFOSIZE0_FIELD_SHIFT },
    7,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FIFOSIZE1 *****/
const ru_field_rec BBH_TX_UNIFIED_CONFIGURATIONS_PDSIZE_FIFOSIZE1_FIELD =
{
    "FIFOSIZE1",
#if RU_INCLUDE_DESC
    "",
    "The size of PD FIFO for queue 1.\nA value of n refers to n+1.\n\n",
#endif
    { BBH_TX_UNIFIED_CONFIGURATIONS_PDSIZE_FIFOSIZE1_FIELD_MASK },
    0,
    { BBH_TX_UNIFIED_CONFIGURATIONS_PDSIZE_FIFOSIZE1_FIELD_WIDTH },
    { BBH_TX_UNIFIED_CONFIGURATIONS_PDSIZE_FIFOSIZE1_FIELD_SHIFT },
    7,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_UNIFIED_CONFIGURATIONS_PDSIZE_FIELDS[] =
{
    &BBH_TX_UNIFIED_CONFIGURATIONS_PDSIZE_FIFOSIZE0_FIELD,
    &BBH_TX_UNIFIED_CONFIGURATIONS_PDSIZE_FIFOSIZE1_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_TX_UNIFIED_CONFIGURATIONS_PDSIZE *****/
const ru_reg_rec BBH_TX_UNIFIED_CONFIGURATIONS_PDSIZE_REG =
{
    "UNIFIED_CONFIGURATIONS_PDSIZE",
#if RU_INCLUDE_DESC
    "PD_FIFO_SIZE Register",
    "The BBH manages 40 queues for GPON and 32 queues for EPON (FIFO per TCONT/LLID). For each queue it manages a PD FIFO.\nA total of 256 PDs are available for all queues.\nThere are 2 profiles of queues. For each profile the SW configures the size of the PD FIFO and then specifies for each queue which profile it is associated with.\nThis register defines the PD FIFO size of the 2 profiles.\n\n",
#endif
    { BBH_TX_UNIFIED_CONFIGURATIONS_PDSIZE_REG_OFFSET },
    0,
    0,
    151,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    BBH_TX_UNIFIED_CONFIGURATIONS_PDSIZE_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_TX_UNIFIED_CONFIGURATIONS_PDWKUPH, TYPE: Type_BBH_TX_BBHTX_UNIFIED_CONFIGURATIONS_PDWKUPH
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: WKUPTHRESH0 *****/
const ru_field_rec BBH_TX_UNIFIED_CONFIGURATIONS_PDWKUPH_WKUPTHRESH0_FIELD =
{
    "WKUPTHRESH0",
#if RU_INCLUDE_DESC
    "",
    "The wakeup threshold of the PD FIFO for queue 0.\nA value of n refers to n+1.\nRelevant only for EPON BBH.\n",
#endif
    { BBH_TX_UNIFIED_CONFIGURATIONS_PDWKUPH_WKUPTHRESH0_FIELD_MASK },
    0,
    { BBH_TX_UNIFIED_CONFIGURATIONS_PDWKUPH_WKUPTHRESH0_FIELD_WIDTH },
    { BBH_TX_UNIFIED_CONFIGURATIONS_PDWKUPH_WKUPTHRESH0_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: WKUPTHRESH1 *****/
const ru_field_rec BBH_TX_UNIFIED_CONFIGURATIONS_PDWKUPH_WKUPTHRESH1_FIELD =
{
    "WKUPTHRESH1",
#if RU_INCLUDE_DESC
    "",
    "The wakeup threshold of the PD FIFO for queue 1.\nA value of n refers to n+1.\nRelevant only for EPON BBH.\n",
#endif
    { BBH_TX_UNIFIED_CONFIGURATIONS_PDWKUPH_WKUPTHRESH1_FIELD_MASK },
    0,
    { BBH_TX_UNIFIED_CONFIGURATIONS_PDWKUPH_WKUPTHRESH1_FIELD_WIDTH },
    { BBH_TX_UNIFIED_CONFIGURATIONS_PDWKUPH_WKUPTHRESH1_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_UNIFIED_CONFIGURATIONS_PDWKUPH_FIELDS[] =
{
    &BBH_TX_UNIFIED_CONFIGURATIONS_PDWKUPH_WKUPTHRESH0_FIELD,
    &BBH_TX_UNIFIED_CONFIGURATIONS_PDWKUPH_WKUPTHRESH1_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_TX_UNIFIED_CONFIGURATIONS_PDWKUPH *****/
const ru_reg_rec BBH_TX_UNIFIED_CONFIGURATIONS_PDWKUPH_REG =
{
    "UNIFIED_CONFIGURATIONS_PDWKUPH",
#if RU_INCLUDE_DESC
    "PD_WKUP_THRESH Register",
    "When a FIFO occupancy is above this wakeup threshold, the BBH will not wake-up the Runner for sending a new PD. This threshold does not represent the actual size of the FIFO. If a PD will arrive from the Runner when the FIFO is above the threshold, it will not be dropped unless the FIFO is actually full.\nThis register defines the threshold of the 2 profiles.\n\n",
#endif
    { BBH_TX_UNIFIED_CONFIGURATIONS_PDWKUPH_REG_OFFSET },
    0,
    0,
    152,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    BBH_TX_UNIFIED_CONFIGURATIONS_PDWKUPH_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_TX_UNIFIED_CONFIGURATIONS_PD_BYTE_TH, TYPE: Type_BBH_TX_BBHTX_UNIFIED_CONFIGURATIONS_PD_BYTE_TH
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: PDLIMIT0 *****/
const ru_field_rec BBH_TX_UNIFIED_CONFIGURATIONS_PD_BYTE_TH_PDLIMIT0_FIELD =
{
    "PDLIMIT0",
#if RU_INCLUDE_DESC
    "",
    "Defines the number of bytes for PDs pre fetch limited according to the total number of bytes.\nThe value is in 8-bytes resolution.\n\n",
#endif
    { BBH_TX_UNIFIED_CONFIGURATIONS_PD_BYTE_TH_PDLIMIT0_FIELD_MASK },
    0,
    { BBH_TX_UNIFIED_CONFIGURATIONS_PD_BYTE_TH_PDLIMIT0_FIELD_WIDTH },
    { BBH_TX_UNIFIED_CONFIGURATIONS_PD_BYTE_TH_PDLIMIT0_FIELD_SHIFT },
    4095,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: PDLIMIT1 *****/
const ru_field_rec BBH_TX_UNIFIED_CONFIGURATIONS_PD_BYTE_TH_PDLIMIT1_FIELD =
{
    "PDLIMIT1",
#if RU_INCLUDE_DESC
    "",
    "Defines the number of bytes for PDs pre fetch limited according to the total number of bytes.\nThe value is in 8-bytes resolution.\n\n",
#endif
    { BBH_TX_UNIFIED_CONFIGURATIONS_PD_BYTE_TH_PDLIMIT1_FIELD_MASK },
    0,
    { BBH_TX_UNIFIED_CONFIGURATIONS_PD_BYTE_TH_PDLIMIT1_FIELD_WIDTH },
    { BBH_TX_UNIFIED_CONFIGURATIONS_PD_BYTE_TH_PDLIMIT1_FIELD_SHIFT },
    4095,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_UNIFIED_CONFIGURATIONS_PD_BYTE_TH_FIELDS[] =
{
    &BBH_TX_UNIFIED_CONFIGURATIONS_PD_BYTE_TH_PDLIMIT0_FIELD,
    &BBH_TX_UNIFIED_CONFIGURATIONS_PD_BYTE_TH_PDLIMIT1_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_TX_UNIFIED_CONFIGURATIONS_PD_BYTE_TH *****/
const ru_reg_rec BBH_TX_UNIFIED_CONFIGURATIONS_PD_BYTE_TH_REG =
{
    "UNIFIED_CONFIGURATIONS_PD_BYTE_TH",
#if RU_INCLUDE_DESC
    "PD_BYTES_THRESHOLD Register",
    "The BBH requests PDs from the Runner and maintains a pre-fetch PDs FIFO.\nThe PDs pre fetch is limited either by the PD FIFO configurable size or according to the total number of bytes (deducting bytes already requested/transmitted) for preventing HOL. This register defines the threshold of the 2 queue profiles.\n",
#endif
    { BBH_TX_UNIFIED_CONFIGURATIONS_PD_BYTE_TH_REG_OFFSET },
    0,
    0,
    153,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    BBH_TX_UNIFIED_CONFIGURATIONS_PD_BYTE_TH_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_TX_UNIFIED_CONFIGURATIONS_QMQ, TYPE: Type_BBH_TX_BBHTX_UNIFIED_CONFIGURATIONS_QMQ
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: Q0 *****/
const ru_field_rec BBH_TX_UNIFIED_CONFIGURATIONS_QMQ_Q0_FIELD =
{
    "Q0",
#if RU_INCLUDE_DESC
    "",
    "Q0 configuration\n",
#endif
    { BBH_TX_UNIFIED_CONFIGURATIONS_QMQ_Q0_FIELD_MASK },
    0,
    { BBH_TX_UNIFIED_CONFIGURATIONS_QMQ_Q0_FIELD_WIDTH },
    { BBH_TX_UNIFIED_CONFIGURATIONS_QMQ_Q0_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: Q1 *****/
const ru_field_rec BBH_TX_UNIFIED_CONFIGURATIONS_QMQ_Q1_FIELD =
{
    "Q1",
#if RU_INCLUDE_DESC
    "",
    "Q1 configuration\n",
#endif
    { BBH_TX_UNIFIED_CONFIGURATIONS_QMQ_Q1_FIELD_MASK },
    0,
    { BBH_TX_UNIFIED_CONFIGURATIONS_QMQ_Q1_FIELD_WIDTH },
    { BBH_TX_UNIFIED_CONFIGURATIONS_QMQ_Q1_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_UNIFIED_CONFIGURATIONS_QMQ_FIELDS[] =
{
    &BBH_TX_UNIFIED_CONFIGURATIONS_QMQ_Q0_FIELD,
    &BBH_TX_UNIFIED_CONFIGURATIONS_QMQ_Q1_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_TX_UNIFIED_CONFIGURATIONS_QMQ *****/
const ru_reg_rec BBH_TX_UNIFIED_CONFIGURATIONS_QMQ_REG =
{
    "UNIFIED_CONFIGURATIONS_QMQ",
#if RU_INCLUDE_DESC
    "QM_Q 0..3 Register",
    "This configuration determines whether the Q works with QM or with TM Runner.\n\nQM queues will not send wakeup and read pointer messages.\n\nEach register in this array configures 2 queues.\n",
#endif
    { BBH_TX_UNIFIED_CONFIGURATIONS_QMQ_REG_OFFSET },
    BBH_TX_UNIFIED_CONFIGURATIONS_QMQ_REG_RAM_CNT,
    4,
    154,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    BBH_TX_UNIFIED_CONFIGURATIONS_QMQ_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_TX_UNIFIED_CONFIGURATIONS_MOTF, TYPE: Type_BBH_TX_BBHTX_UNIFIED_CONFIGURATIONS_MOTF
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: Q0 *****/
const ru_field_rec BBH_TX_UNIFIED_CONFIGURATIONS_MOTF_Q0_FIELD =
{
    "Q0",
#if RU_INCLUDE_DESC
    "",
    "Q0 configuration\n",
#endif
    { BBH_TX_UNIFIED_CONFIGURATIONS_MOTF_Q0_FIELD_MASK },
    0,
    { BBH_TX_UNIFIED_CONFIGURATIONS_MOTF_Q0_FIELD_WIDTH },
    { BBH_TX_UNIFIED_CONFIGURATIONS_MOTF_Q0_FIELD_SHIFT },
    63,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: Q1 *****/
const ru_field_rec BBH_TX_UNIFIED_CONFIGURATIONS_MOTF_Q1_FIELD =
{
    "Q1",
#if RU_INCLUDE_DESC
    "",
    "Q1 configuration\n",
#endif
    { BBH_TX_UNIFIED_CONFIGURATIONS_MOTF_Q1_FIELD_MASK },
    0,
    { BBH_TX_UNIFIED_CONFIGURATIONS_MOTF_Q1_FIELD_WIDTH },
    { BBH_TX_UNIFIED_CONFIGURATIONS_MOTF_Q1_FIELD_SHIFT },
    63,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_UNIFIED_CONFIGURATIONS_MOTF_FIELDS[] =
{
    &BBH_TX_UNIFIED_CONFIGURATIONS_MOTF_Q0_FIELD,
    &BBH_TX_UNIFIED_CONFIGURATIONS_MOTF_Q1_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_TX_UNIFIED_CONFIGURATIONS_MOTF *****/
const ru_reg_rec BBH_TX_UNIFIED_CONFIGURATIONS_MOTF_REG =
{
    "UNIFIED_CONFIGURATIONS_MOTF",
#if RU_INCLUDE_DESC
    "MAX_ON_THE_FLY 0..3 Register",
    "This configuration determines the max number of on-the-fly DMA commands per queue.\n\nEach register in this array configures 2 queues.\n",
#endif
    { BBH_TX_UNIFIED_CONFIGURATIONS_MOTF_REG_OFFSET },
    BBH_TX_UNIFIED_CONFIGURATIONS_MOTF_REG_RAM_CNT,
    4,
    155,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    BBH_TX_UNIFIED_CONFIGURATIONS_MOTF_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_TX_UNIFIED_CONFIGURATIONS_PD_BYTE_TH_EN, TYPE: Type_BBH_TX_BBHTX_UNIFIED_CONFIGURATIONS_PD_BYTE_TH_EN
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: PDLIMITEN *****/
const ru_field_rec BBH_TX_UNIFIED_CONFIGURATIONS_PD_BYTE_TH_EN_PDLIMITEN_FIELD =
{
    "PDLIMITEN",
#if RU_INCLUDE_DESC
    "",
    "This bit enables the above feature (PDs pre fetch limited according to the total number of bytes).\n",
#endif
    { BBH_TX_UNIFIED_CONFIGURATIONS_PD_BYTE_TH_EN_PDLIMITEN_FIELD_MASK },
    0,
    { BBH_TX_UNIFIED_CONFIGURATIONS_PD_BYTE_TH_EN_PDLIMITEN_FIELD_WIDTH },
    { BBH_TX_UNIFIED_CONFIGURATIONS_PD_BYTE_TH_EN_PDLIMITEN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_UNIFIED_CONFIGURATIONS_PD_BYTE_TH_EN_FIELDS[] =
{
    &BBH_TX_UNIFIED_CONFIGURATIONS_PD_BYTE_TH_EN_PDLIMITEN_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_TX_UNIFIED_CONFIGURATIONS_PD_BYTE_TH_EN *****/
const ru_reg_rec BBH_TX_UNIFIED_CONFIGURATIONS_PD_BYTE_TH_EN_REG =
{
    "UNIFIED_CONFIGURATIONS_PD_BYTE_TH_EN",
#if RU_INCLUDE_DESC
    "PD_BYTES_THRESHOLD_EN Register",
    "The BBH requests PDs from the Runner and maintains a pre-fetch PDs FIFO.\nThe PDs pre fetch is limited either by the PD FIFO configurable size or according to the total number of bytes (deducting bytes already requested/transmitted) for preventing HOL. Full configuration for the first 8 TCONT and one configuration per group of 8 TCONTs for the rest.\n",
#endif
    { BBH_TX_UNIFIED_CONFIGURATIONS_PD_BYTE_TH_EN_REG_OFFSET },
    0,
    0,
    156,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_TX_UNIFIED_CONFIGURATIONS_PD_BYTE_TH_EN_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_TX_UNIFIED_CONFIGURATIONS_PDEMPTY, TYPE: Type_BBH_TX_BBHTX_UNIFIED_CONFIGURATIONS_PDEMPTY
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: EMPTY *****/
const ru_field_rec BBH_TX_UNIFIED_CONFIGURATIONS_PDEMPTY_EMPTY_FIELD =
{
    "EMPTY",
#if RU_INCLUDE_DESC
    "",
    "EPON PD FIFO empty threshold.\nA queue which its PD FIFO occupancy is below this threshold will have high priority in PD ordering arbitration.\n",
#endif
    { BBH_TX_UNIFIED_CONFIGURATIONS_PDEMPTY_EMPTY_FIELD_MASK },
    0,
    { BBH_TX_UNIFIED_CONFIGURATIONS_PDEMPTY_EMPTY_FIELD_WIDTH },
    { BBH_TX_UNIFIED_CONFIGURATIONS_PDEMPTY_EMPTY_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_UNIFIED_CONFIGURATIONS_PDEMPTY_FIELDS[] =
{
    &BBH_TX_UNIFIED_CONFIGURATIONS_PDEMPTY_EMPTY_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_TX_UNIFIED_CONFIGURATIONS_PDEMPTY *****/
const ru_reg_rec BBH_TX_UNIFIED_CONFIGURATIONS_PDEMPTY_REG =
{
    "UNIFIED_CONFIGURATIONS_PDEMPTY",
#if RU_INCLUDE_DESC
    "PD_EMPTY_THRESHOLD Register",
    "The BBH manages 32 queues for EPON (FIFO per LLID). For each queue it manages a PD FIFO. Usually, the BBH orders PDs from the Runner in RR between all queues. In EPON BBH, if a FIFO occupancy is below this threshold, the queue will have higher priority in PD ordering arbitration (with RR between all the empty queues).\nThis configuration is global for all queues.\nRelevant only for EPON BBH.\n",
#endif
    { BBH_TX_UNIFIED_CONFIGURATIONS_PDEMPTY_REG_OFFSET },
    0,
    0,
    157,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_TX_UNIFIED_CONFIGURATIONS_PDEMPTY_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_TX_UNIFIED_CONFIGURATIONS_GTXTHRESH, TYPE: Type_BBH_TX_BBHTX_UNIFIED_CONFIGURATIONS_GTXTHRESH
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DDRTHRESH *****/
const ru_field_rec BBH_TX_UNIFIED_CONFIGURATIONS_GTXTHRESH_DDRTHRESH_FIELD =
{
    "DDRTHRESH",
#if RU_INCLUDE_DESC
    "",
    "DDR Transmit threshold in 8 bytes resoltion\n",
#endif
    { BBH_TX_UNIFIED_CONFIGURATIONS_GTXTHRESH_DDRTHRESH_FIELD_MASK },
    0,
    { BBH_TX_UNIFIED_CONFIGURATIONS_GTXTHRESH_DDRTHRESH_FIELD_WIDTH },
    { BBH_TX_UNIFIED_CONFIGURATIONS_GTXTHRESH_DDRTHRESH_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: SRAMTHRESH *****/
const ru_field_rec BBH_TX_UNIFIED_CONFIGURATIONS_GTXTHRESH_SRAMTHRESH_FIELD =
{
    "SRAMTHRESH",
#if RU_INCLUDE_DESC
    "",
    "SRAM Transmit threshold in 8 bytes resoltion\n",
#endif
    { BBH_TX_UNIFIED_CONFIGURATIONS_GTXTHRESH_SRAMTHRESH_FIELD_MASK },
    0,
    { BBH_TX_UNIFIED_CONFIGURATIONS_GTXTHRESH_SRAMTHRESH_FIELD_WIDTH },
    { BBH_TX_UNIFIED_CONFIGURATIONS_GTXTHRESH_SRAMTHRESH_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_UNIFIED_CONFIGURATIONS_GTXTHRESH_FIELDS[] =
{
    &BBH_TX_UNIFIED_CONFIGURATIONS_GTXTHRESH_DDRTHRESH_FIELD,
    &BBH_TX_UNIFIED_CONFIGURATIONS_GTXTHRESH_SRAMTHRESH_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_TX_UNIFIED_CONFIGURATIONS_GTXTHRESH *****/
const ru_reg_rec BBH_TX_UNIFIED_CONFIGURATIONS_GTXTHRESH_REG =
{
    "UNIFIED_CONFIGURATIONS_GTXTHRESH",
#if RU_INCLUDE_DESC
    "GLOBAL_TX_THRESHOLD Register",
    "Transmit threshold in 8 bytes resolution.\nThe BBH TX will not start to transmit data towards the XLMAC until the amount of data in the TX FIFO is larger than the threshold or if there is a complete packet in the FIFO.\nThis threshold is used by the non unified BBH. for unified BBH it should be set to 0.\n",
#endif
    { BBH_TX_UNIFIED_CONFIGURATIONS_GTXTHRESH_REG_OFFSET },
    0,
    0,
    158,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    BBH_TX_UNIFIED_CONFIGURATIONS_GTXTHRESH_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_TX_UNIFIED_CONFIGURATIONS_EEE, TYPE: Type_BBH_TX_BBHTX_UNIFIED_CONFIGURATIONS_EEE
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: EN *****/
const ru_field_rec BBH_TX_UNIFIED_CONFIGURATIONS_EEE_EN_FIELD =
{
    "EN",
#if RU_INCLUDE_DESC
    "",
    "enable bit\n",
#endif
    { BBH_TX_UNIFIED_CONFIGURATIONS_EEE_EN_FIELD_MASK },
    0,
    { BBH_TX_UNIFIED_CONFIGURATIONS_EEE_EN_FIELD_WIDTH },
    { BBH_TX_UNIFIED_CONFIGURATIONS_EEE_EN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_UNIFIED_CONFIGURATIONS_EEE_FIELDS[] =
{
    &BBH_TX_UNIFIED_CONFIGURATIONS_EEE_EN_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_TX_UNIFIED_CONFIGURATIONS_EEE *****/
const ru_reg_rec BBH_TX_UNIFIED_CONFIGURATIONS_EEE_REG =
{
    "UNIFIED_CONFIGURATIONS_EEE",
#if RU_INCLUDE_DESC
    "EEE Register",
    "The BBH is responsible for indicating the XLMAC that no traffic is about to arrive so the XLMAC may try to enter power saving mode.\n\nThis register is used to enable this feature per MAC\n",
#endif
    { BBH_TX_UNIFIED_CONFIGURATIONS_EEE_REG_OFFSET },
    0,
    0,
    159,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_TX_UNIFIED_CONFIGURATIONS_EEE_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_TX_UNIFIED_CONFIGURATIONS_TS, TYPE: Type_BBH_TX_BBHTX_UNIFIED_CONFIGURATIONS_TS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: EN *****/
const ru_field_rec BBH_TX_UNIFIED_CONFIGURATIONS_TS_EN_FIELD =
{
    "EN",
#if RU_INCLUDE_DESC
    "",
    "enable bit\n",
#endif
    { BBH_TX_UNIFIED_CONFIGURATIONS_TS_EN_FIELD_MASK },
    0,
    { BBH_TX_UNIFIED_CONFIGURATIONS_TS_EN_FIELD_WIDTH },
    { BBH_TX_UNIFIED_CONFIGURATIONS_TS_EN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_UNIFIED_CONFIGURATIONS_TS_FIELDS[] =
{
    &BBH_TX_UNIFIED_CONFIGURATIONS_TS_EN_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_TX_UNIFIED_CONFIGURATIONS_TS *****/
const ru_reg_rec BBH_TX_UNIFIED_CONFIGURATIONS_TS_REG =
{
    "UNIFIED_CONFIGURATIONS_TS",
#if RU_INCLUDE_DESC
    "TS Register",
    "The BBH is responsible for indicating the XLMAC that it should and calculate timestamp for the current packet that is being transmitted. The BBH gets the timestamping parameters in the PD and forward it to the XLMAC.\n\nThis register is used to enable this feature per MAC\n",
#endif
    { BBH_TX_UNIFIED_CONFIGURATIONS_TS_REG_OFFSET },
    0,
    0,
    160,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_TX_UNIFIED_CONFIGURATIONS_TS_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_TX_UNIFIED_CONFIGURATIONS_FE_CREDITS, TYPE: Type_BBH_TX_BBHTX_UNIFIED_CONFIGURATIONS_FE_CREDITS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: INIT *****/
const ru_field_rec BBH_TX_UNIFIED_CONFIGURATIONS_FE_CREDITS_INIT_FIELD =
{
    "INIT",
#if RU_INCLUDE_DESC
    "",
    "initialization of the credits counter with the size of the FE FIFOS (pd and data)\n",
#endif
    { BBH_TX_UNIFIED_CONFIGURATIONS_FE_CREDITS_INIT_FIELD_MASK },
    0,
    { BBH_TX_UNIFIED_CONFIGURATIONS_FE_CREDITS_INIT_FIELD_WIDTH },
    { BBH_TX_UNIFIED_CONFIGURATIONS_FE_CREDITS_INIT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: MIN_PD *****/
const ru_field_rec BBH_TX_UNIFIED_CONFIGURATIONS_FE_CREDITS_MIN_PD_FIELD =
{
    "MIN_PD",
#if RU_INCLUDE_DESC
    "",
    "minimum number of credits for the FE PD FIFO in order to perform an ordering of chuck of data from the SRAM/DDR\n",
#endif
    { BBH_TX_UNIFIED_CONFIGURATIONS_FE_CREDITS_MIN_PD_FIELD_MASK },
    0,
    { BBH_TX_UNIFIED_CONFIGURATIONS_FE_CREDITS_MIN_PD_FIELD_WIDTH },
    { BBH_TX_UNIFIED_CONFIGURATIONS_FE_CREDITS_MIN_PD_FIELD_SHIFT },
    4,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: MIN_DATA *****/
const ru_field_rec BBH_TX_UNIFIED_CONFIGURATIONS_FE_CREDITS_MIN_DATA_FIELD =
{
    "MIN_DATA",
#if RU_INCLUDE_DESC
    "",
    "minimum number of credits for the FE TX FIFO in order to perform an ordering of chuck of data from the SRAM/DDR\n",
#endif
    { BBH_TX_UNIFIED_CONFIGURATIONS_FE_CREDITS_MIN_DATA_FIELD_MASK },
    0,
    { BBH_TX_UNIFIED_CONFIGURATIONS_FE_CREDITS_MIN_DATA_FIELD_WIDTH },
    { BBH_TX_UNIFIED_CONFIGURATIONS_FE_CREDITS_MIN_DATA_FIELD_SHIFT },
    17,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: USE_BUF_RDY *****/
const ru_field_rec BBH_TX_UNIFIED_CONFIGURATIONS_FE_CREDITS_USE_BUF_RDY_FIELD =
{
    "USE_BUF_RDY",
#if RU_INCLUDE_DESC
    "",
    "determine whether to use credits mechanism or buf_rdy mechanism\n",
#endif
    { BBH_TX_UNIFIED_CONFIGURATIONS_FE_CREDITS_USE_BUF_RDY_FIELD_MASK },
    0,
    { BBH_TX_UNIFIED_CONFIGURATIONS_FE_CREDITS_USE_BUF_RDY_FIELD_WIDTH },
    { BBH_TX_UNIFIED_CONFIGURATIONS_FE_CREDITS_USE_BUF_RDY_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_UNIFIED_CONFIGURATIONS_FE_CREDITS_FIELDS[] =
{
    &BBH_TX_UNIFIED_CONFIGURATIONS_FE_CREDITS_INIT_FIELD,
    &BBH_TX_UNIFIED_CONFIGURATIONS_FE_CREDITS_MIN_PD_FIELD,
    &BBH_TX_UNIFIED_CONFIGURATIONS_FE_CREDITS_MIN_DATA_FIELD,
    &BBH_TX_UNIFIED_CONFIGURATIONS_FE_CREDITS_USE_BUF_RDY_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_TX_UNIFIED_CONFIGURATIONS_FE_CREDITS *****/
const ru_reg_rec BBH_TX_UNIFIED_CONFIGURATIONS_FE_CREDITS_REG =
{
    "UNIFIED_CONFIGURATIONS_FE_CREDITS",
#if RU_INCLUDE_DESC
    "FE_CREDITS Register",
    "between the FE and segmentation, the indication whether there is a free space in the FE FIFOs or not can be done with either buf_rdy indication as in GPON or with credits mechanism. This register configures the credits mechanism.\n",
#endif
    { BBH_TX_UNIFIED_CONFIGURATIONS_FE_CREDITS_REG_OFFSET },
    0,
    0,
    161,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    BBH_TX_UNIFIED_CONFIGURATIONS_FE_CREDITS_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_TX_UNIFIED_CONFIGURATIONS_FEBASE, TYPE: Type_BBH_TX_BBHTX_UNIFIED_CONFIGURATIONS_FEBASE
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: FIFOBASE0 *****/
const ru_field_rec BBH_TX_UNIFIED_CONFIGURATIONS_FEBASE_FIFOBASE0_FIELD =
{
    "FIFOBASE0",
#if RU_INCLUDE_DESC
    "",
    "The base of FE FIFO for queue 0.\n",
#endif
    { BBH_TX_UNIFIED_CONFIGURATIONS_FEBASE_FIFOBASE0_FIELD_MASK },
    0,
    { BBH_TX_UNIFIED_CONFIGURATIONS_FEBASE_FIFOBASE0_FIELD_WIDTH },
    { BBH_TX_UNIFIED_CONFIGURATIONS_FEBASE_FIFOBASE0_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FIFOBASE1 *****/
const ru_field_rec BBH_TX_UNIFIED_CONFIGURATIONS_FEBASE_FIFOBASE1_FIELD =
{
    "FIFOBASE1",
#if RU_INCLUDE_DESC
    "",
    "The base of FE FIFO for queue 1.\n",
#endif
    { BBH_TX_UNIFIED_CONFIGURATIONS_FEBASE_FIFOBASE1_FIELD_MASK },
    0,
    { BBH_TX_UNIFIED_CONFIGURATIONS_FEBASE_FIFOBASE1_FIELD_WIDTH },
    { BBH_TX_UNIFIED_CONFIGURATIONS_FEBASE_FIFOBASE1_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_UNIFIED_CONFIGURATIONS_FEBASE_FIELDS[] =
{
    &BBH_TX_UNIFIED_CONFIGURATIONS_FEBASE_FIFOBASE0_FIELD,
    &BBH_TX_UNIFIED_CONFIGURATIONS_FEBASE_FIFOBASE1_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_TX_UNIFIED_CONFIGURATIONS_FEBASE *****/
const ru_reg_rec BBH_TX_UNIFIED_CONFIGURATIONS_FEBASE_REG =
{
    "UNIFIED_CONFIGURATIONS_FEBASE",
#if RU_INCLUDE_DESC
    "FE_FIFO_BASE 0..3 Register",
    "The BBH manages 40 queues for GPON or 32 queus for EPON (1 for each TCONT/LLID). For each queue it manages a PD FIFO.\nA total of 256 PDs are available for all queues.\nFor each Queue the SW configures the base and the size within these 256 PDs.\n\nThe size of the 1st BN FIFO and get-next FIFO is the same as the size of the PD FIFO of each queue.\n\neach register in this array defines the PD FIFO base of 2 queues.\n\n",
#endif
    { BBH_TX_UNIFIED_CONFIGURATIONS_FEBASE_REG_OFFSET },
    BBH_TX_UNIFIED_CONFIGURATIONS_FEBASE_REG_RAM_CNT,
    4,
    162,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    BBH_TX_UNIFIED_CONFIGURATIONS_FEBASE_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_TX_UNIFIED_CONFIGURATIONS_FESIZE, TYPE: Type_BBH_TX_BBHTX_UNIFIED_CONFIGURATIONS_FESIZE
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: FIFOSIZE0 *****/
const ru_field_rec BBH_TX_UNIFIED_CONFIGURATIONS_FESIZE_FIFOSIZE0_FIELD =
{
    "FIFOSIZE0",
#if RU_INCLUDE_DESC
    "",
    "The size of PD FIFO for queue 0.\nA value of n refers to n+1.\n\n",
#endif
    { BBH_TX_UNIFIED_CONFIGURATIONS_FESIZE_FIFOSIZE0_FIELD_MASK },
    0,
    { BBH_TX_UNIFIED_CONFIGURATIONS_FESIZE_FIFOSIZE0_FIELD_WIDTH },
    { BBH_TX_UNIFIED_CONFIGURATIONS_FESIZE_FIFOSIZE0_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FIFOSIZE1 *****/
const ru_field_rec BBH_TX_UNIFIED_CONFIGURATIONS_FESIZE_FIFOSIZE1_FIELD =
{
    "FIFOSIZE1",
#if RU_INCLUDE_DESC
    "",
    "The size of FE FIFO for queue 1.\nA value of n refers to n+1.\n\n",
#endif
    { BBH_TX_UNIFIED_CONFIGURATIONS_FESIZE_FIFOSIZE1_FIELD_MASK },
    0,
    { BBH_TX_UNIFIED_CONFIGURATIONS_FESIZE_FIFOSIZE1_FIELD_WIDTH },
    { BBH_TX_UNIFIED_CONFIGURATIONS_FESIZE_FIFOSIZE1_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_UNIFIED_CONFIGURATIONS_FESIZE_FIELDS[] =
{
    &BBH_TX_UNIFIED_CONFIGURATIONS_FESIZE_FIFOSIZE0_FIELD,
    &BBH_TX_UNIFIED_CONFIGURATIONS_FESIZE_FIFOSIZE1_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_TX_UNIFIED_CONFIGURATIONS_FESIZE *****/
const ru_reg_rec BBH_TX_UNIFIED_CONFIGURATIONS_FESIZE_REG =
{
    "UNIFIED_CONFIGURATIONS_FESIZE",
#if RU_INCLUDE_DESC
    "FE_FIFO_SIZE 0..3 Register",
    "The BBH manages 40 queues for GPON and 32 queues for EPON (FIFO per TCONT/LLID). For each queue it manages a PD FIFO.\nA total of 256 PDs are available for all queues.\nFor each Queue the SW configures the base and the size within these.\neach register in this array defines the PD FIFO size of 2 queues.\n\n",
#endif
    { BBH_TX_UNIFIED_CONFIGURATIONS_FESIZE_REG_OFFSET },
    BBH_TX_UNIFIED_CONFIGURATIONS_FESIZE_REG_RAM_CNT,
    4,
    163,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    BBH_TX_UNIFIED_CONFIGURATIONS_FESIZE_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_TX_UNIFIED_CONFIGURATIONS_FEPDBASE, TYPE: Type_BBH_TX_BBHTX_UNIFIED_CONFIGURATIONS_FEPDBASE
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: FIFOBASE0 *****/
const ru_field_rec BBH_TX_UNIFIED_CONFIGURATIONS_FEPDBASE_FIFOBASE0_FIELD =
{
    "FIFOBASE0",
#if RU_INCLUDE_DESC
    "",
    "The base of FE PD FIFO for queue 0.\n",
#endif
    { BBH_TX_UNIFIED_CONFIGURATIONS_FEPDBASE_FIFOBASE0_FIELD_MASK },
    0,
    { BBH_TX_UNIFIED_CONFIGURATIONS_FEPDBASE_FIFOBASE0_FIELD_WIDTH },
    { BBH_TX_UNIFIED_CONFIGURATIONS_FEPDBASE_FIFOBASE0_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FIFOBASE1 *****/
const ru_field_rec BBH_TX_UNIFIED_CONFIGURATIONS_FEPDBASE_FIFOBASE1_FIELD =
{
    "FIFOBASE1",
#if RU_INCLUDE_DESC
    "",
    "The base of FE PD FIFO for queue 1.\n",
#endif
    { BBH_TX_UNIFIED_CONFIGURATIONS_FEPDBASE_FIFOBASE1_FIELD_MASK },
    0,
    { BBH_TX_UNIFIED_CONFIGURATIONS_FEPDBASE_FIFOBASE1_FIELD_WIDTH },
    { BBH_TX_UNIFIED_CONFIGURATIONS_FEPDBASE_FIFOBASE1_FIELD_SHIFT },
    8,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_UNIFIED_CONFIGURATIONS_FEPDBASE_FIELDS[] =
{
    &BBH_TX_UNIFIED_CONFIGURATIONS_FEPDBASE_FIFOBASE0_FIELD,
    &BBH_TX_UNIFIED_CONFIGURATIONS_FEPDBASE_FIFOBASE1_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_TX_UNIFIED_CONFIGURATIONS_FEPDBASE *****/
const ru_reg_rec BBH_TX_UNIFIED_CONFIGURATIONS_FEPDBASE_REG =
{
    "UNIFIED_CONFIGURATIONS_FEPDBASE",
#if RU_INCLUDE_DESC
    "FE_PD_FIFO_BASE 0..3 Register",
    "The BBH manages 40 queues for GPON or 32 queus for EPON (1 for each TCONT/LLID). For each queue it manages a PD FIFO.\nA total of 256 PDs are available for all queues.\nFor each Queue the SW configures the base and the size within these 256 PDs.\n\nThe size of the 1st BN FIFO and get-next FIFO is the same as the size of the PD FIFO of each queue.\n\neach register in this array defines the PD FIFO base of 2 queues.\n\n",
#endif
    { BBH_TX_UNIFIED_CONFIGURATIONS_FEPDBASE_REG_OFFSET },
    BBH_TX_UNIFIED_CONFIGURATIONS_FEPDBASE_REG_RAM_CNT,
    4,
    164,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    BBH_TX_UNIFIED_CONFIGURATIONS_FEPDBASE_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_TX_UNIFIED_CONFIGURATIONS_FEPDSIZE, TYPE: Type_BBH_TX_BBHTX_UNIFIED_CONFIGURATIONS_FEPDSIZE
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: FIFOSIZE0 *****/
const ru_field_rec BBH_TX_UNIFIED_CONFIGURATIONS_FEPDSIZE_FIFOSIZE0_FIELD =
{
    "FIFOSIZE0",
#if RU_INCLUDE_DESC
    "",
    "The size of FE PD FIFO for queue 0.\nA value of n refers to n+1.\n\n",
#endif
    { BBH_TX_UNIFIED_CONFIGURATIONS_FEPDSIZE_FIFOSIZE0_FIELD_MASK },
    0,
    { BBH_TX_UNIFIED_CONFIGURATIONS_FEPDSIZE_FIFOSIZE0_FIELD_WIDTH },
    { BBH_TX_UNIFIED_CONFIGURATIONS_FEPDSIZE_FIFOSIZE0_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FIFOSIZE1 *****/
const ru_field_rec BBH_TX_UNIFIED_CONFIGURATIONS_FEPDSIZE_FIFOSIZE1_FIELD =
{
    "FIFOSIZE1",
#if RU_INCLUDE_DESC
    "",
    "The size of FE PD FIFO for queue 1.\nA value of n refers to n+1.\n\n",
#endif
    { BBH_TX_UNIFIED_CONFIGURATIONS_FEPDSIZE_FIFOSIZE1_FIELD_MASK },
    0,
    { BBH_TX_UNIFIED_CONFIGURATIONS_FEPDSIZE_FIFOSIZE1_FIELD_WIDTH },
    { BBH_TX_UNIFIED_CONFIGURATIONS_FEPDSIZE_FIFOSIZE1_FIELD_SHIFT },
    7,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_UNIFIED_CONFIGURATIONS_FEPDSIZE_FIELDS[] =
{
    &BBH_TX_UNIFIED_CONFIGURATIONS_FEPDSIZE_FIFOSIZE0_FIELD,
    &BBH_TX_UNIFIED_CONFIGURATIONS_FEPDSIZE_FIFOSIZE1_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_TX_UNIFIED_CONFIGURATIONS_FEPDSIZE *****/
const ru_reg_rec BBH_TX_UNIFIED_CONFIGURATIONS_FEPDSIZE_REG =
{
    "UNIFIED_CONFIGURATIONS_FEPDSIZE",
#if RU_INCLUDE_DESC
    "FE_PD_FIFO_SIZE 0..3 Register",
    "The BBH manages 40 queues for GPON and 32 queues for EPON (FIFO per TCONT/LLID). For each queue it manages a PD FIFO.\nA total of 256 PDs are available for all queues.\nFor each Queue the SW configures the base and the size within these.\neach register in this array defines the PD FIFO size of 2 queues.\n\n",
#endif
    { BBH_TX_UNIFIED_CONFIGURATIONS_FEPDSIZE_REG_OFFSET },
    BBH_TX_UNIFIED_CONFIGURATIONS_FEPDSIZE_REG_RAM_CNT,
    4,
    165,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    BBH_TX_UNIFIED_CONFIGURATIONS_FEPDSIZE_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_TX_UNIFIED_CONFIGURATIONS_TXWRR, TYPE: Type_BBH_TX_BBHTX_UNIFIED_CONFIGURATIONS_TXWRR
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: W0 *****/
const ru_field_rec BBH_TX_UNIFIED_CONFIGURATIONS_TXWRR_W0_FIELD =
{
    "W0",
#if RU_INCLUDE_DESC
    "",
    "weight of MAC0\n",
#endif
    { BBH_TX_UNIFIED_CONFIGURATIONS_TXWRR_W0_FIELD_MASK },
    0,
    { BBH_TX_UNIFIED_CONFIGURATIONS_TXWRR_W0_FIELD_WIDTH },
    { BBH_TX_UNIFIED_CONFIGURATIONS_TXWRR_W0_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: W1 *****/
const ru_field_rec BBH_TX_UNIFIED_CONFIGURATIONS_TXWRR_W1_FIELD =
{
    "W1",
#if RU_INCLUDE_DESC
    "",
    "weight of MAC1\n",
#endif
    { BBH_TX_UNIFIED_CONFIGURATIONS_TXWRR_W1_FIELD_MASK },
    0,
    { BBH_TX_UNIFIED_CONFIGURATIONS_TXWRR_W1_FIELD_WIDTH },
    { BBH_TX_UNIFIED_CONFIGURATIONS_TXWRR_W1_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_UNIFIED_CONFIGURATIONS_TXWRR_FIELDS[] =
{
    &BBH_TX_UNIFIED_CONFIGURATIONS_TXWRR_W0_FIELD,
    &BBH_TX_UNIFIED_CONFIGURATIONS_TXWRR_W1_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_TX_UNIFIED_CONFIGURATIONS_TXWRR *****/
const ru_reg_rec BBH_TX_UNIFIED_CONFIGURATIONS_TXWRR_REG =
{
    "UNIFIED_CONFIGURATIONS_TXWRR",
#if RU_INCLUDE_DESC
    "TX_RR_WEIGHT 0..3 Register",
    "The unified BBH TX serves multiple MACs.\n\nThe TX arbitration between these MACs is WRR.\n\nThis register array determines the weight of each MAC. Each register in the array represents 2 MACs.\n",
#endif
    { BBH_TX_UNIFIED_CONFIGURATIONS_TXWRR_REG_OFFSET },
    BBH_TX_UNIFIED_CONFIGURATIONS_TXWRR_REG_RAM_CNT,
    4,
    166,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    BBH_TX_UNIFIED_CONFIGURATIONS_TXWRR_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_TX_UNIFIED_CONFIGURATIONS_SGMTWRR, TYPE: Type_BBH_TX_BBHTX_UNIFIED_CONFIGURATIONS_SGMTWRR
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: W0 *****/
const ru_field_rec BBH_TX_UNIFIED_CONFIGURATIONS_SGMTWRR_W0_FIELD =
{
    "W0",
#if RU_INCLUDE_DESC
    "",
    "weight of MAC0\n",
#endif
    { BBH_TX_UNIFIED_CONFIGURATIONS_SGMTWRR_W0_FIELD_MASK },
    0,
    { BBH_TX_UNIFIED_CONFIGURATIONS_SGMTWRR_W0_FIELD_WIDTH },
    { BBH_TX_UNIFIED_CONFIGURATIONS_SGMTWRR_W0_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: W1 *****/
const ru_field_rec BBH_TX_UNIFIED_CONFIGURATIONS_SGMTWRR_W1_FIELD =
{
    "W1",
#if RU_INCLUDE_DESC
    "",
    "weight of MAC1\n",
#endif
    { BBH_TX_UNIFIED_CONFIGURATIONS_SGMTWRR_W1_FIELD_MASK },
    0,
    { BBH_TX_UNIFIED_CONFIGURATIONS_SGMTWRR_W1_FIELD_WIDTH },
    { BBH_TX_UNIFIED_CONFIGURATIONS_SGMTWRR_W1_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_UNIFIED_CONFIGURATIONS_SGMTWRR_FIELDS[] =
{
    &BBH_TX_UNIFIED_CONFIGURATIONS_SGMTWRR_W0_FIELD,
    &BBH_TX_UNIFIED_CONFIGURATIONS_SGMTWRR_W1_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_TX_UNIFIED_CONFIGURATIONS_SGMTWRR *****/
const ru_reg_rec BBH_TX_UNIFIED_CONFIGURATIONS_SGMTWRR_REG =
{
    "UNIFIED_CONFIGURATIONS_SGMTWRR",
#if RU_INCLUDE_DESC
    "SGMT_RR_WEIGHT 0..3 Register",
    "The unified BBH TX serves multiple MACs.\n\nThe TX arbitration between these MACs is WRR.\n\nThis register array determines the weight of each MAC. Each register in the array represents 2 MACs.\n",
#endif
    { BBH_TX_UNIFIED_CONFIGURATIONS_SGMTWRR_REG_OFFSET },
    BBH_TX_UNIFIED_CONFIGURATIONS_SGMTWRR_REG_RAM_CNT,
    4,
    167,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    BBH_TX_UNIFIED_CONFIGURATIONS_SGMTWRR_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_TX_UNIFIED_CONFIGURATIONS_TXTHRESH, TYPE: Type_BBH_TX_BBHTX_UNIFIED_CONFIGURATIONS_TXTHRESH
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: THRESH0 *****/
const ru_field_rec BBH_TX_UNIFIED_CONFIGURATIONS_TXTHRESH_THRESH0_FIELD =
{
    "THRESH0",
#if RU_INCLUDE_DESC
    "",
    "Transmit threshold in 8 bytes resoltion for mac 0\n",
#endif
    { BBH_TX_UNIFIED_CONFIGURATIONS_TXTHRESH_THRESH0_FIELD_MASK },
    0,
    { BBH_TX_UNIFIED_CONFIGURATIONS_TXTHRESH_THRESH0_FIELD_WIDTH },
    { BBH_TX_UNIFIED_CONFIGURATIONS_TXTHRESH_THRESH0_FIELD_SHIFT },
    255,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: THRESH1 *****/
const ru_field_rec BBH_TX_UNIFIED_CONFIGURATIONS_TXTHRESH_THRESH1_FIELD =
{
    "THRESH1",
#if RU_INCLUDE_DESC
    "",
    "Transmit threshold in 8 bytes resolution for MAC1\n",
#endif
    { BBH_TX_UNIFIED_CONFIGURATIONS_TXTHRESH_THRESH1_FIELD_MASK },
    0,
    { BBH_TX_UNIFIED_CONFIGURATIONS_TXTHRESH_THRESH1_FIELD_WIDTH },
    { BBH_TX_UNIFIED_CONFIGURATIONS_TXTHRESH_THRESH1_FIELD_SHIFT },
    255,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_UNIFIED_CONFIGURATIONS_TXTHRESH_FIELDS[] =
{
    &BBH_TX_UNIFIED_CONFIGURATIONS_TXTHRESH_THRESH0_FIELD,
    &BBH_TX_UNIFIED_CONFIGURATIONS_TXTHRESH_THRESH1_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_TX_UNIFIED_CONFIGURATIONS_TXTHRESH *****/
const ru_reg_rec BBH_TX_UNIFIED_CONFIGURATIONS_TXTHRESH_REG =
{
    "UNIFIED_CONFIGURATIONS_TXTHRESH",
#if RU_INCLUDE_DESC
    "TX_THRESHOLD 0..3 Register",
    "Transmit threshold in 8 bytes resolution.\nThe BBH TX will not start to transmit data towards the MAC until the amount of data in the TX FIFO is larger than the threshold or if there is a complete packet in the FIFO.\n",
#endif
    { BBH_TX_UNIFIED_CONFIGURATIONS_TXTHRESH_REG_OFFSET },
    BBH_TX_UNIFIED_CONFIGURATIONS_TXTHRESH_REG_RAM_CNT,
    4,
    168,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    BBH_TX_UNIFIED_CONFIGURATIONS_TXTHRESH_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_TX_DEBUG_COUNTERS_TXSRAMPD, TYPE: Type_BBH_TX_BBHTX_DEBUG_COUNTERS_TXSRAMPD
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: SRAMPD *****/
const ru_field_rec BBH_TX_DEBUG_COUNTERS_TXSRAMPD_SRAMPD_FIELD =
{
    "SRAMPD",
#if RU_INCLUDE_DESC
    "",
    "This counter counts the number of packets which were transmitted from the SRAM.\n",
#endif
    { BBH_TX_DEBUG_COUNTERS_TXSRAMPD_SRAMPD_FIELD_MASK },
    0,
    { BBH_TX_DEBUG_COUNTERS_TXSRAMPD_SRAMPD_FIELD_WIDTH },
    { BBH_TX_DEBUG_COUNTERS_TXSRAMPD_SRAMPD_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_DEBUG_COUNTERS_TXSRAMPD_FIELDS[] =
{
    &BBH_TX_DEBUG_COUNTERS_TXSRAMPD_SRAMPD_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_TX_DEBUG_COUNTERS_TXSRAMPD *****/
const ru_reg_rec BBH_TX_DEBUG_COUNTERS_TXSRAMPD_REG =
{
    "DEBUG_COUNTERS_TXSRAMPD",
#if RU_INCLUDE_DESC
    "TX_SRAM_PD_COUNTER Register",
    "This counter counts the number of received PD for packets to be transmitted from the SRAM.\nIt counts the packets for all queues together.\nThis counter is cleared when read and freezes when maximum value is reached.\n\n",
#endif
    { BBH_TX_DEBUG_COUNTERS_TXSRAMPD_REG_OFFSET },
    0,
    0,
    169,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_TX_DEBUG_COUNTERS_TXSRAMPD_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_TX_DEBUG_COUNTERS_TXDDRPD, TYPE: Type_BBH_TX_BBHTX_DEBUG_COUNTERS_TXDDRPD
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DDRPD *****/
const ru_field_rec BBH_TX_DEBUG_COUNTERS_TXDDRPD_DDRPD_FIELD =
{
    "DDRPD",
#if RU_INCLUDE_DESC
    "",
    "This counter counts the number of packets which were transmitted from the DDR.\n",
#endif
    { BBH_TX_DEBUG_COUNTERS_TXDDRPD_DDRPD_FIELD_MASK },
    0,
    { BBH_TX_DEBUG_COUNTERS_TXDDRPD_DDRPD_FIELD_WIDTH },
    { BBH_TX_DEBUG_COUNTERS_TXDDRPD_DDRPD_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_DEBUG_COUNTERS_TXDDRPD_FIELDS[] =
{
    &BBH_TX_DEBUG_COUNTERS_TXDDRPD_DDRPD_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_TX_DEBUG_COUNTERS_TXDDRPD *****/
const ru_reg_rec BBH_TX_DEBUG_COUNTERS_TXDDRPD_REG =
{
    "DEBUG_COUNTERS_TXDDRPD",
#if RU_INCLUDE_DESC
    "TX_DDR_PD_COUNTER Register",
    "This counter counts the number of received PDs for packets to be transmitted from the DDR.\nIt counts the packets for all queues together.\nThis counter is cleared when read and freezes when maximum value is reached.\n",
#endif
    { BBH_TX_DEBUG_COUNTERS_TXDDRPD_REG_OFFSET },
    0,
    0,
    170,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_TX_DEBUG_COUNTERS_TXDDRPD_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_TX_DEBUG_COUNTERS_PDDROP, TYPE: Type_BBH_TX_BBHTX_DEBUG_COUNTERS_PDDROP
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: PDDROP *****/
const ru_field_rec BBH_TX_DEBUG_COUNTERS_PDDROP_PDDROP_FIELD =
{
    "PDDROP",
#if RU_INCLUDE_DESC
    "",
    "This counter counts the number of PDs which were dropped due to PD FIFO full.\n",
#endif
    { BBH_TX_DEBUG_COUNTERS_PDDROP_PDDROP_FIELD_MASK },
    0,
    { BBH_TX_DEBUG_COUNTERS_PDDROP_PDDROP_FIELD_WIDTH },
    { BBH_TX_DEBUG_COUNTERS_PDDROP_PDDROP_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_DEBUG_COUNTERS_PDDROP_FIELDS[] =
{
    &BBH_TX_DEBUG_COUNTERS_PDDROP_PDDROP_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_TX_DEBUG_COUNTERS_PDDROP *****/
const ru_reg_rec BBH_TX_DEBUG_COUNTERS_PDDROP_REG =
{
    "DEBUG_COUNTERS_PDDROP",
#if RU_INCLUDE_DESC
    "PD_DROP_COUNTER Register",
    "This counter counts the number of PDs which were dropped due to PD FIFO full.\nIt counts the packets for all queues together.\nThis counter is cleared when read and freezes when maximum value is reached.\n\n",
#endif
    { BBH_TX_DEBUG_COUNTERS_PDDROP_REG_OFFSET },
    0,
    0,
    171,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_TX_DEBUG_COUNTERS_PDDROP_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_TX_DEBUG_COUNTERS_STSCNT, TYPE: Type_BBH_TX_BBHTX_DEBUG_COUNTERS_STSCNT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: STSCNT *****/
const ru_field_rec BBH_TX_DEBUG_COUNTERS_STSCNT_STSCNT_FIELD =
{
    "STSCNT",
#if RU_INCLUDE_DESC
    "",
    "This counter counts the number of received status messages.\n",
#endif
    { BBH_TX_DEBUG_COUNTERS_STSCNT_STSCNT_FIELD_MASK },
    0,
    { BBH_TX_DEBUG_COUNTERS_STSCNT_STSCNT_FIELD_WIDTH },
    { BBH_TX_DEBUG_COUNTERS_STSCNT_STSCNT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_DEBUG_COUNTERS_STSCNT_FIELDS[] =
{
    &BBH_TX_DEBUG_COUNTERS_STSCNT_STSCNT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_TX_DEBUG_COUNTERS_STSCNT *****/
const ru_reg_rec BBH_TX_DEBUG_COUNTERS_STSCNT_REG =
{
    "DEBUG_COUNTERS_STSCNT",
#if RU_INCLUDE_DESC
    "STS_COUNTER Register",
    "This counter counts the number of STS messages which were received from Runner.\nIt counts the packets for all queues together.\nThis counter is cleared when read and freezes when maximum value is reached.\n\n",
#endif
    { BBH_TX_DEBUG_COUNTERS_STSCNT_REG_OFFSET },
    0,
    0,
    172,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_TX_DEBUG_COUNTERS_STSCNT_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_TX_DEBUG_COUNTERS_STSDROP, TYPE: Type_BBH_TX_BBHTX_DEBUG_COUNTERS_STSDROP
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: STSDROP *****/
const ru_field_rec BBH_TX_DEBUG_COUNTERS_STSDROP_STSDROP_FIELD =
{
    "STSDROP",
#if RU_INCLUDE_DESC
    "",
    "This counter counts the number of STS which were dropped due to PD FIFO full.\n",
#endif
    { BBH_TX_DEBUG_COUNTERS_STSDROP_STSDROP_FIELD_MASK },
    0,
    { BBH_TX_DEBUG_COUNTERS_STSDROP_STSDROP_FIELD_WIDTH },
    { BBH_TX_DEBUG_COUNTERS_STSDROP_STSDROP_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_DEBUG_COUNTERS_STSDROP_FIELDS[] =
{
    &BBH_TX_DEBUG_COUNTERS_STSDROP_STSDROP_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_TX_DEBUG_COUNTERS_STSDROP *****/
const ru_reg_rec BBH_TX_DEBUG_COUNTERS_STSDROP_REG =
{
    "DEBUG_COUNTERS_STSDROP",
#if RU_INCLUDE_DESC
    "STS_DROP_COUNTER Register",
    "This counter counts the number of STS which were dropped due to PD FIFO full.\nIt counts the packets for all queues together.\nThis counter is cleared when read and freezes when maximum value is reached.\n\n",
#endif
    { BBH_TX_DEBUG_COUNTERS_STSDROP_REG_OFFSET },
    0,
    0,
    173,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_TX_DEBUG_COUNTERS_STSDROP_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_TX_DEBUG_COUNTERS_MSGCNT, TYPE: Type_BBH_TX_BBHTX_DEBUG_COUNTERS_MSGCNT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: MSGCNT *****/
const ru_field_rec BBH_TX_DEBUG_COUNTERS_MSGCNT_MSGCNT_FIELD =
{
    "MSGCNT",
#if RU_INCLUDE_DESC
    "",
    "This counter counts the number of received DBR/ghost messages.\n",
#endif
    { BBH_TX_DEBUG_COUNTERS_MSGCNT_MSGCNT_FIELD_MASK },
    0,
    { BBH_TX_DEBUG_COUNTERS_MSGCNT_MSGCNT_FIELD_WIDTH },
    { BBH_TX_DEBUG_COUNTERS_MSGCNT_MSGCNT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_DEBUG_COUNTERS_MSGCNT_FIELDS[] =
{
    &BBH_TX_DEBUG_COUNTERS_MSGCNT_MSGCNT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_TX_DEBUG_COUNTERS_MSGCNT *****/
const ru_reg_rec BBH_TX_DEBUG_COUNTERS_MSGCNT_REG =
{
    "DEBUG_COUNTERS_MSGCNT",
#if RU_INCLUDE_DESC
    "MSG_COUNTER Register",
    "This counter counts the number of MSG (DBR/Ghost) messages which were received from Runner.\nIt counts the packets for all queues together.\nThis counter is cleared when read and freezes when maximum value is reached.\n\n",
#endif
    { BBH_TX_DEBUG_COUNTERS_MSGCNT_REG_OFFSET },
    0,
    0,
    174,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_TX_DEBUG_COUNTERS_MSGCNT_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_TX_DEBUG_COUNTERS_MSGDROP, TYPE: Type_BBH_TX_BBHTX_DEBUG_COUNTERS_MSGDROP
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: MSGDROP *****/
const ru_field_rec BBH_TX_DEBUG_COUNTERS_MSGDROP_MSGDROP_FIELD =
{
    "MSGDROP",
#if RU_INCLUDE_DESC
    "",
    "This counter counts the number of MSG which were dropped due to PD FIFO full.\n",
#endif
    { BBH_TX_DEBUG_COUNTERS_MSGDROP_MSGDROP_FIELD_MASK },
    0,
    { BBH_TX_DEBUG_COUNTERS_MSGDROP_MSGDROP_FIELD_WIDTH },
    { BBH_TX_DEBUG_COUNTERS_MSGDROP_MSGDROP_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_DEBUG_COUNTERS_MSGDROP_FIELDS[] =
{
    &BBH_TX_DEBUG_COUNTERS_MSGDROP_MSGDROP_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_TX_DEBUG_COUNTERS_MSGDROP *****/
const ru_reg_rec BBH_TX_DEBUG_COUNTERS_MSGDROP_REG =
{
    "DEBUG_COUNTERS_MSGDROP",
#if RU_INCLUDE_DESC
    "MSG_DROP_COUNTER Register",
    "This counter counts the number of MSG which were dropped due to PD FIFO full.\nIt counts the packets for all queues together.\nThis counter is cleared when read and freezes when maximum value is reached.\n\n",
#endif
    { BBH_TX_DEBUG_COUNTERS_MSGDROP_REG_OFFSET },
    0,
    0,
    175,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_TX_DEBUG_COUNTERS_MSGDROP_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_TX_DEBUG_COUNTERS_GETNEXTNULL, TYPE: Type_BBH_TX_BBHTX_DEBUG_COUNTERS_GETNEXTNULL
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: GETNEXTNULL *****/
const ru_field_rec BBH_TX_DEBUG_COUNTERS_GETNEXTNULL_GETNEXTNULL_FIELD =
{
    "GETNEXTNULL",
#if RU_INCLUDE_DESC
    "",
    "This counter counts the number Get next responses with a null BN.\n",
#endif
    { BBH_TX_DEBUG_COUNTERS_GETNEXTNULL_GETNEXTNULL_FIELD_MASK },
    0,
    { BBH_TX_DEBUG_COUNTERS_GETNEXTNULL_GETNEXTNULL_FIELD_WIDTH },
    { BBH_TX_DEBUG_COUNTERS_GETNEXTNULL_GETNEXTNULL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_DEBUG_COUNTERS_GETNEXTNULL_FIELDS[] =
{
    &BBH_TX_DEBUG_COUNTERS_GETNEXTNULL_GETNEXTNULL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_TX_DEBUG_COUNTERS_GETNEXTNULL *****/
const ru_reg_rec BBH_TX_DEBUG_COUNTERS_GETNEXTNULL_REG =
{
    "DEBUG_COUNTERS_GETNEXTNULL",
#if RU_INCLUDE_DESC
    "GET_NEXT_IS_NULL_COUNTER Register",
    "This counter counts the number Get next responses with a null BN.\nIt counts the packets for all TCONTs together.\nThis counter is cleared when read and freezes when maximum value is reached.\nThis counter is relevant for Ethernet only.\n",
#endif
    { BBH_TX_DEBUG_COUNTERS_GETNEXTNULL_REG_OFFSET },
    0,
    0,
    176,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_TX_DEBUG_COUNTERS_GETNEXTNULL_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_TX_DEBUG_COUNTERS_FLUSHPKTS, TYPE: Type_BBH_TX_BBHTX_DEBUG_COUNTERS_FLUSHPKTS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: FLSHPKTS *****/
const ru_field_rec BBH_TX_DEBUG_COUNTERS_FLUSHPKTS_FLSHPKTS_FIELD =
{
    "FLSHPKTS",
#if RU_INCLUDE_DESC
    "",
    "This counter counts the number of flushed packets\n",
#endif
    { BBH_TX_DEBUG_COUNTERS_FLUSHPKTS_FLSHPKTS_FIELD_MASK },
    0,
    { BBH_TX_DEBUG_COUNTERS_FLUSHPKTS_FLSHPKTS_FIELD_WIDTH },
    { BBH_TX_DEBUG_COUNTERS_FLUSHPKTS_FLSHPKTS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_DEBUG_COUNTERS_FLUSHPKTS_FIELDS[] =
{
    &BBH_TX_DEBUG_COUNTERS_FLUSHPKTS_FLSHPKTS_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_TX_DEBUG_COUNTERS_FLUSHPKTS *****/
const ru_reg_rec BBH_TX_DEBUG_COUNTERS_FLUSHPKTS_REG =
{
    "DEBUG_COUNTERS_FLUSHPKTS",
#if RU_INCLUDE_DESC
    "FLUSHED_PACKETS_COUNTER Register",
    "This counter counts the number of packets that were flushed (bn was released without sending the data to the EPON MAC) due to flush request.\nThe counter is global for all queues.\nThe counter is read clear.\n",
#endif
    { BBH_TX_DEBUG_COUNTERS_FLUSHPKTS_REG_OFFSET },
    0,
    0,
    177,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_TX_DEBUG_COUNTERS_FLUSHPKTS_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_TX_DEBUG_COUNTERS_LENERR, TYPE: Type_BBH_TX_BBHTX_DEBUG_COUNTERS_LENERR
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: LENERR *****/
const ru_field_rec BBH_TX_DEBUG_COUNTERS_LENERR_LENERR_FIELD =
{
    "LENERR",
#if RU_INCLUDE_DESC
    "",
    "This counter counts the number of times a length error occuered\n",
#endif
    { BBH_TX_DEBUG_COUNTERS_LENERR_LENERR_FIELD_MASK },
    0,
    { BBH_TX_DEBUG_COUNTERS_LENERR_LENERR_FIELD_WIDTH },
    { BBH_TX_DEBUG_COUNTERS_LENERR_LENERR_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_DEBUG_COUNTERS_LENERR_FIELDS[] =
{
    &BBH_TX_DEBUG_COUNTERS_LENERR_LENERR_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_TX_DEBUG_COUNTERS_LENERR *****/
const ru_reg_rec BBH_TX_DEBUG_COUNTERS_LENERR_REG =
{
    "DEBUG_COUNTERS_LENERR",
#if RU_INCLUDE_DESC
    "REQ_LENGTH_ERROR_COUNTER Register",
    "This counter counts the number of times a length error (mismatch between a request from the MAC and a PD from the Runner) occured.\nThis counter is cleared when read and freezes when maximum value is reached.\n",
#endif
    { BBH_TX_DEBUG_COUNTERS_LENERR_REG_OFFSET },
    0,
    0,
    178,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_TX_DEBUG_COUNTERS_LENERR_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_TX_DEBUG_COUNTERS_AGGRLENERR, TYPE: Type_BBH_TX_BBHTX_DEBUG_COUNTERS_AGGRLENERR
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: AGGRLENERR *****/
const ru_field_rec BBH_TX_DEBUG_COUNTERS_AGGRLENERR_AGGRLENERR_FIELD =
{
    "AGGRLENERR",
#if RU_INCLUDE_DESC
    "",
    "This counter counts the number of times an aggregation length error occuered\n",
#endif
    { BBH_TX_DEBUG_COUNTERS_AGGRLENERR_AGGRLENERR_FIELD_MASK },
    0,
    { BBH_TX_DEBUG_COUNTERS_AGGRLENERR_AGGRLENERR_FIELD_WIDTH },
    { BBH_TX_DEBUG_COUNTERS_AGGRLENERR_AGGRLENERR_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_DEBUG_COUNTERS_AGGRLENERR_FIELDS[] =
{
    &BBH_TX_DEBUG_COUNTERS_AGGRLENERR_AGGRLENERR_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_TX_DEBUG_COUNTERS_AGGRLENERR *****/
const ru_reg_rec BBH_TX_DEBUG_COUNTERS_AGGRLENERR_REG =
{
    "DEBUG_COUNTERS_AGGRLENERR",
#if RU_INCLUDE_DESC
    "AGGREGATION_LENGTH_ERROR_COUNTER Register",
    "This counter Counts aggregation length error events.\nIf one or more of the packets in an aggregated PD is shorter than 60 bytes, this counter will be incremented by 1.\nThis counter is cleared when read and freezes when maximum value is reached.\n",
#endif
    { BBH_TX_DEBUG_COUNTERS_AGGRLENERR_REG_OFFSET },
    0,
    0,
    179,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_TX_DEBUG_COUNTERS_AGGRLENERR_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_TX_DEBUG_COUNTERS_SRAMPKT, TYPE: Type_BBH_TX_BBHTX_DEBUG_COUNTERS_SRAMPKT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: SRAMPKT *****/
const ru_field_rec BBH_TX_DEBUG_COUNTERS_SRAMPKT_SRAMPKT_FIELD =
{
    "SRAMPKT",
#if RU_INCLUDE_DESC
    "",
    "This counter counts the number of packets which were transmitted from the SRAM.\n",
#endif
    { BBH_TX_DEBUG_COUNTERS_SRAMPKT_SRAMPKT_FIELD_MASK },
    0,
    { BBH_TX_DEBUG_COUNTERS_SRAMPKT_SRAMPKT_FIELD_WIDTH },
    { BBH_TX_DEBUG_COUNTERS_SRAMPKT_SRAMPKT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_DEBUG_COUNTERS_SRAMPKT_FIELDS[] =
{
    &BBH_TX_DEBUG_COUNTERS_SRAMPKT_SRAMPKT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_TX_DEBUG_COUNTERS_SRAMPKT *****/
const ru_reg_rec BBH_TX_DEBUG_COUNTERS_SRAMPKT_REG =
{
    "DEBUG_COUNTERS_SRAMPKT",
#if RU_INCLUDE_DESC
    "SRAM_PKT_COUNTER Register",
    "This counter counts the number of received packets to be transmitted from the SRAM.\nIt counts the packets for all queues together.\nThis counter is cleared when read and freezes when maximum value is reached.\n\n",
#endif
    { BBH_TX_DEBUG_COUNTERS_SRAMPKT_REG_OFFSET },
    0,
    0,
    180,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_TX_DEBUG_COUNTERS_SRAMPKT_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_TX_DEBUG_COUNTERS_DDRPKT, TYPE: Type_BBH_TX_BBHTX_DEBUG_COUNTERS_DDRPKT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DDRPKT *****/
const ru_field_rec BBH_TX_DEBUG_COUNTERS_DDRPKT_DDRPKT_FIELD =
{
    "DDRPKT",
#if RU_INCLUDE_DESC
    "",
    "This counter counts the number of packets which were transmitted from the DDR.\n",
#endif
    { BBH_TX_DEBUG_COUNTERS_DDRPKT_DDRPKT_FIELD_MASK },
    0,
    { BBH_TX_DEBUG_COUNTERS_DDRPKT_DDRPKT_FIELD_WIDTH },
    { BBH_TX_DEBUG_COUNTERS_DDRPKT_DDRPKT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_DEBUG_COUNTERS_DDRPKT_FIELDS[] =
{
    &BBH_TX_DEBUG_COUNTERS_DDRPKT_DDRPKT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_TX_DEBUG_COUNTERS_DDRPKT *****/
const ru_reg_rec BBH_TX_DEBUG_COUNTERS_DDRPKT_REG =
{
    "DEBUG_COUNTERS_DDRPKT",
#if RU_INCLUDE_DESC
    "DDR_PKT_COUNTER Register",
    "This counter counts the number of received packets to be transmitted from the DDR.\nIt counts the packets for all queues together.\nThis counter is cleared when read and freezes when maximum value is reached.\n\n",
#endif
    { BBH_TX_DEBUG_COUNTERS_DDRPKT_REG_OFFSET },
    0,
    0,
    181,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_TX_DEBUG_COUNTERS_DDRPKT_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_TX_DEBUG_COUNTERS_TXSRAMBYTE, TYPE: Type_BBH_TX_BBHTX_DEBUG_COUNTERS_TXSRAMBYTE
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: SRAMBYTE *****/
const ru_field_rec BBH_TX_DEBUG_COUNTERS_TXSRAMBYTE_SRAMBYTE_FIELD =
{
    "SRAMBYTE",
#if RU_INCLUDE_DESC
    "",
    "This counter counts the number of transmitted bytes from the SRAM.\n",
#endif
    { BBH_TX_DEBUG_COUNTERS_TXSRAMBYTE_SRAMBYTE_FIELD_MASK },
    0,
    { BBH_TX_DEBUG_COUNTERS_TXSRAMBYTE_SRAMBYTE_FIELD_WIDTH },
    { BBH_TX_DEBUG_COUNTERS_TXSRAMBYTE_SRAMBYTE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_DEBUG_COUNTERS_TXSRAMBYTE_FIELDS[] =
{
    &BBH_TX_DEBUG_COUNTERS_TXSRAMBYTE_SRAMBYTE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_TX_DEBUG_COUNTERS_TXSRAMBYTE *****/
const ru_reg_rec BBH_TX_DEBUG_COUNTERS_TXSRAMBYTE_REG =
{
    "DEBUG_COUNTERS_TXSRAMBYTE",
#if RU_INCLUDE_DESC
    "TX_SRAM_BYTE_COUNTER Register",
    "This counter counts the number of transmitted bytes from the SRAM.\nIt counts the bytes for all queues together.\nThis counter is cleared when read and freezes when maximum value is reached.\n\n",
#endif
    { BBH_TX_DEBUG_COUNTERS_TXSRAMBYTE_REG_OFFSET },
    0,
    0,
    182,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_TX_DEBUG_COUNTERS_TXSRAMBYTE_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_TX_DEBUG_COUNTERS_TXDDRBYTE, TYPE: Type_BBH_TX_BBHTX_DEBUG_COUNTERS_TXDDRBYTE
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DDRBYTE *****/
const ru_field_rec BBH_TX_DEBUG_COUNTERS_TXDDRBYTE_DDRBYTE_FIELD =
{
    "DDRBYTE",
#if RU_INCLUDE_DESC
    "",
    "This counter counts the number of transmitted bytes from the DDr.\n",
#endif
    { BBH_TX_DEBUG_COUNTERS_TXDDRBYTE_DDRBYTE_FIELD_MASK },
    0,
    { BBH_TX_DEBUG_COUNTERS_TXDDRBYTE_DDRBYTE_FIELD_WIDTH },
    { BBH_TX_DEBUG_COUNTERS_TXDDRBYTE_DDRBYTE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_DEBUG_COUNTERS_TXDDRBYTE_FIELDS[] =
{
    &BBH_TX_DEBUG_COUNTERS_TXDDRBYTE_DDRBYTE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_TX_DEBUG_COUNTERS_TXDDRBYTE *****/
const ru_reg_rec BBH_TX_DEBUG_COUNTERS_TXDDRBYTE_REG =
{
    "DEBUG_COUNTERS_TXDDRBYTE",
#if RU_INCLUDE_DESC
    "TX_DDR_BYTE_COUNTER Register",
    "This counter counts the number of transmitted bytes from the DDR.\nIt counts the bytes for all queues together.\nThis counter is cleared when read and freezes when maximum value is reached.\n\n",
#endif
    { BBH_TX_DEBUG_COUNTERS_TXDDRBYTE_REG_OFFSET },
    0,
    0,
    183,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_TX_DEBUG_COUNTERS_TXDDRBYTE_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_TX_DEBUG_COUNTERS_SWRDEN, TYPE: Type_BBH_TX_BBHTX_DEBUG_COUNTERS_SWRDEN
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: PDSEL *****/
const ru_field_rec BBH_TX_DEBUG_COUNTERS_SWRDEN_PDSEL_FIELD =
{
    "PDSEL",
#if RU_INCLUDE_DESC
    "",
    "rd from the PD FIFO\n",
#endif
    { BBH_TX_DEBUG_COUNTERS_SWRDEN_PDSEL_FIELD_MASK },
    0,
    { BBH_TX_DEBUG_COUNTERS_SWRDEN_PDSEL_FIELD_WIDTH },
    { BBH_TX_DEBUG_COUNTERS_SWRDEN_PDSEL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: PDVSEL *****/
const ru_field_rec BBH_TX_DEBUG_COUNTERS_SWRDEN_PDVSEL_FIELD =
{
    "PDVSEL",
#if RU_INCLUDE_DESC
    "",
    "rd from the PD valid array\n",
#endif
    { BBH_TX_DEBUG_COUNTERS_SWRDEN_PDVSEL_FIELD_MASK },
    0,
    { BBH_TX_DEBUG_COUNTERS_SWRDEN_PDVSEL_FIELD_WIDTH },
    { BBH_TX_DEBUG_COUNTERS_SWRDEN_PDVSEL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: PDEMPTYSEL *****/
const ru_field_rec BBH_TX_DEBUG_COUNTERS_SWRDEN_PDEMPTYSEL_FIELD =
{
    "PDEMPTYSEL",
#if RU_INCLUDE_DESC
    "",
    "rd from the PD empty array\n",
#endif
    { BBH_TX_DEBUG_COUNTERS_SWRDEN_PDEMPTYSEL_FIELD_MASK },
    0,
    { BBH_TX_DEBUG_COUNTERS_SWRDEN_PDEMPTYSEL_FIELD_WIDTH },
    { BBH_TX_DEBUG_COUNTERS_SWRDEN_PDEMPTYSEL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: PDFULLSEL *****/
const ru_field_rec BBH_TX_DEBUG_COUNTERS_SWRDEN_PDFULLSEL_FIELD =
{
    "PDFULLSEL",
#if RU_INCLUDE_DESC
    "",
    "rd from the PD Full array\n",
#endif
    { BBH_TX_DEBUG_COUNTERS_SWRDEN_PDFULLSEL_FIELD_MASK },
    0,
    { BBH_TX_DEBUG_COUNTERS_SWRDEN_PDFULLSEL_FIELD_WIDTH },
    { BBH_TX_DEBUG_COUNTERS_SWRDEN_PDFULLSEL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: PDBEMPTYSEL *****/
const ru_field_rec BBH_TX_DEBUG_COUNTERS_SWRDEN_PDBEMPTYSEL_FIELD =
{
    "PDBEMPTYSEL",
#if RU_INCLUDE_DESC
    "",
    "rd from the PD beliow empty array\n",
#endif
    { BBH_TX_DEBUG_COUNTERS_SWRDEN_PDBEMPTYSEL_FIELD_MASK },
    0,
    { BBH_TX_DEBUG_COUNTERS_SWRDEN_PDBEMPTYSEL_FIELD_WIDTH },
    { BBH_TX_DEBUG_COUNTERS_SWRDEN_PDBEMPTYSEL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: PDFFWKPSEL *****/
const ru_field_rec BBH_TX_DEBUG_COUNTERS_SWRDEN_PDFFWKPSEL_FIELD =
{
    "PDFFWKPSEL",
#if RU_INCLUDE_DESC
    "",
    "rd from the PD full for wakeup empty array\n",
#endif
    { BBH_TX_DEBUG_COUNTERS_SWRDEN_PDFFWKPSEL_FIELD_MASK },
    0,
    { BBH_TX_DEBUG_COUNTERS_SWRDEN_PDFFWKPSEL_FIELD_WIDTH },
    { BBH_TX_DEBUG_COUNTERS_SWRDEN_PDFFWKPSEL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FBNSEL *****/
const ru_field_rec BBH_TX_DEBUG_COUNTERS_SWRDEN_FBNSEL_FIELD =
{
    "FBNSEL",
#if RU_INCLUDE_DESC
    "",
    "rd from the first BN array\n",
#endif
    { BBH_TX_DEBUG_COUNTERS_SWRDEN_FBNSEL_FIELD_MASK },
    0,
    { BBH_TX_DEBUG_COUNTERS_SWRDEN_FBNSEL_FIELD_WIDTH },
    { BBH_TX_DEBUG_COUNTERS_SWRDEN_FBNSEL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FBNVSEL *****/
const ru_field_rec BBH_TX_DEBUG_COUNTERS_SWRDEN_FBNVSEL_FIELD =
{
    "FBNVSEL",
#if RU_INCLUDE_DESC
    "",
    "rd from the first BN valid array\n",
#endif
    { BBH_TX_DEBUG_COUNTERS_SWRDEN_FBNVSEL_FIELD_MASK },
    0,
    { BBH_TX_DEBUG_COUNTERS_SWRDEN_FBNVSEL_FIELD_WIDTH },
    { BBH_TX_DEBUG_COUNTERS_SWRDEN_FBNVSEL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FBNEMPTYSEL *****/
const ru_field_rec BBH_TX_DEBUG_COUNTERS_SWRDEN_FBNEMPTYSEL_FIELD =
{
    "FBNEMPTYSEL",
#if RU_INCLUDE_DESC
    "",
    "rd from the first BN empty array\n",
#endif
    { BBH_TX_DEBUG_COUNTERS_SWRDEN_FBNEMPTYSEL_FIELD_MASK },
    0,
    { BBH_TX_DEBUG_COUNTERS_SWRDEN_FBNEMPTYSEL_FIELD_WIDTH },
    { BBH_TX_DEBUG_COUNTERS_SWRDEN_FBNEMPTYSEL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FBNFULLSEL *****/
const ru_field_rec BBH_TX_DEBUG_COUNTERS_SWRDEN_FBNFULLSEL_FIELD =
{
    "FBNFULLSEL",
#if RU_INCLUDE_DESC
    "",
    "rd from the first BN full array\n",
#endif
    { BBH_TX_DEBUG_COUNTERS_SWRDEN_FBNFULLSEL_FIELD_MASK },
    0,
    { BBH_TX_DEBUG_COUNTERS_SWRDEN_FBNFULLSEL_FIELD_WIDTH },
    { BBH_TX_DEBUG_COUNTERS_SWRDEN_FBNFULLSEL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: GETNEXTSEL *****/
const ru_field_rec BBH_TX_DEBUG_COUNTERS_SWRDEN_GETNEXTSEL_FIELD =
{
    "GETNEXTSEL",
#if RU_INCLUDE_DESC
    "",
    "rd from the first Get Next array\n",
#endif
    { BBH_TX_DEBUG_COUNTERS_SWRDEN_GETNEXTSEL_FIELD_MASK },
    0,
    { BBH_TX_DEBUG_COUNTERS_SWRDEN_GETNEXTSEL_FIELD_WIDTH },
    { BBH_TX_DEBUG_COUNTERS_SWRDEN_GETNEXTSEL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: GETNEXTVSEL *****/
const ru_field_rec BBH_TX_DEBUG_COUNTERS_SWRDEN_GETNEXTVSEL_FIELD =
{
    "GETNEXTVSEL",
#if RU_INCLUDE_DESC
    "",
    "rd from the get_next valid array\n",
#endif
    { BBH_TX_DEBUG_COUNTERS_SWRDEN_GETNEXTVSEL_FIELD_MASK },
    0,
    { BBH_TX_DEBUG_COUNTERS_SWRDEN_GETNEXTVSEL_FIELD_WIDTH },
    { BBH_TX_DEBUG_COUNTERS_SWRDEN_GETNEXTVSEL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: GETNEXTEMPTYSEL *****/
const ru_field_rec BBH_TX_DEBUG_COUNTERS_SWRDEN_GETNEXTEMPTYSEL_FIELD =
{
    "GETNEXTEMPTYSEL",
#if RU_INCLUDE_DESC
    "",
    "rd from the get next empty array\n",
#endif
    { BBH_TX_DEBUG_COUNTERS_SWRDEN_GETNEXTEMPTYSEL_FIELD_MASK },
    0,
    { BBH_TX_DEBUG_COUNTERS_SWRDEN_GETNEXTEMPTYSEL_FIELD_WIDTH },
    { BBH_TX_DEBUG_COUNTERS_SWRDEN_GETNEXTEMPTYSEL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: GETNEXTFULLSEL *****/
const ru_field_rec BBH_TX_DEBUG_COUNTERS_SWRDEN_GETNEXTFULLSEL_FIELD =
{
    "GETNEXTFULLSEL",
#if RU_INCLUDE_DESC
    "",
    "rd from the get next full array\n",
#endif
    { BBH_TX_DEBUG_COUNTERS_SWRDEN_GETNEXTFULLSEL_FIELD_MASK },
    0,
    { BBH_TX_DEBUG_COUNTERS_SWRDEN_GETNEXTFULLSEL_FIELD_WIDTH },
    { BBH_TX_DEBUG_COUNTERS_SWRDEN_GETNEXTFULLSEL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: GPNCNTXTSEL *****/
const ru_field_rec BBH_TX_DEBUG_COUNTERS_SWRDEN_GPNCNTXTSEL_FIELD =
{
    "GPNCNTXTSEL",
#if RU_INCLUDE_DESC
    "",
    "rd from the gpon context array\n",
#endif
    { BBH_TX_DEBUG_COUNTERS_SWRDEN_GPNCNTXTSEL_FIELD_MASK },
    0,
    { BBH_TX_DEBUG_COUNTERS_SWRDEN_GPNCNTXTSEL_FIELD_WIDTH },
    { BBH_TX_DEBUG_COUNTERS_SWRDEN_GPNCNTXTSEL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: BPMSEL *****/
const ru_field_rec BBH_TX_DEBUG_COUNTERS_SWRDEN_BPMSEL_FIELD =
{
    "BPMSEL",
#if RU_INCLUDE_DESC
    "",
    "rd from the BPM FIFO\n",
#endif
    { BBH_TX_DEBUG_COUNTERS_SWRDEN_BPMSEL_FIELD_MASK },
    0,
    { BBH_TX_DEBUG_COUNTERS_SWRDEN_BPMSEL_FIELD_WIDTH },
    { BBH_TX_DEBUG_COUNTERS_SWRDEN_BPMSEL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: BPMFSEL *****/
const ru_field_rec BBH_TX_DEBUG_COUNTERS_SWRDEN_BPMFSEL_FIELD =
{
    "BPMFSEL",
#if RU_INCLUDE_DESC
    "",
    "rd from the BPM FLUSH FIFO\n",
#endif
    { BBH_TX_DEBUG_COUNTERS_SWRDEN_BPMFSEL_FIELD_MASK },
    0,
    { BBH_TX_DEBUG_COUNTERS_SWRDEN_BPMFSEL_FIELD_WIDTH },
    { BBH_TX_DEBUG_COUNTERS_SWRDEN_BPMFSEL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: SBPMSEL *****/
const ru_field_rec BBH_TX_DEBUG_COUNTERS_SWRDEN_SBPMSEL_FIELD =
{
    "SBPMSEL",
#if RU_INCLUDE_DESC
    "",
    "rd from the SBPM FIFO\n",
#endif
    { BBH_TX_DEBUG_COUNTERS_SWRDEN_SBPMSEL_FIELD_MASK },
    0,
    { BBH_TX_DEBUG_COUNTERS_SWRDEN_SBPMSEL_FIELD_WIDTH },
    { BBH_TX_DEBUG_COUNTERS_SWRDEN_SBPMSEL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: SBPMFSEL *****/
const ru_field_rec BBH_TX_DEBUG_COUNTERS_SWRDEN_SBPMFSEL_FIELD =
{
    "SBPMFSEL",
#if RU_INCLUDE_DESC
    "",
    "rd from the SBPM FLUSH FIFO\n",
#endif
    { BBH_TX_DEBUG_COUNTERS_SWRDEN_SBPMFSEL_FIELD_MASK },
    0,
    { BBH_TX_DEBUG_COUNTERS_SWRDEN_SBPMFSEL_FIELD_WIDTH },
    { BBH_TX_DEBUG_COUNTERS_SWRDEN_SBPMFSEL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: STSSEL *****/
const ru_field_rec BBH_TX_DEBUG_COUNTERS_SWRDEN_STSSEL_FIELD =
{
    "STSSEL",
#if RU_INCLUDE_DESC
    "",
    "rd from the STS FIFO\n",
#endif
    { BBH_TX_DEBUG_COUNTERS_SWRDEN_STSSEL_FIELD_MASK },
    0,
    { BBH_TX_DEBUG_COUNTERS_SWRDEN_STSSEL_FIELD_WIDTH },
    { BBH_TX_DEBUG_COUNTERS_SWRDEN_STSSEL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: STSVSEL *****/
const ru_field_rec BBH_TX_DEBUG_COUNTERS_SWRDEN_STSVSEL_FIELD =
{
    "STSVSEL",
#if RU_INCLUDE_DESC
    "",
    "rd from the STS valid array\n",
#endif
    { BBH_TX_DEBUG_COUNTERS_SWRDEN_STSVSEL_FIELD_MASK },
    0,
    { BBH_TX_DEBUG_COUNTERS_SWRDEN_STSVSEL_FIELD_WIDTH },
    { BBH_TX_DEBUG_COUNTERS_SWRDEN_STSVSEL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: STSEMPTYSEL *****/
const ru_field_rec BBH_TX_DEBUG_COUNTERS_SWRDEN_STSEMPTYSEL_FIELD =
{
    "STSEMPTYSEL",
#if RU_INCLUDE_DESC
    "",
    "rd from the STS empty array\n",
#endif
    { BBH_TX_DEBUG_COUNTERS_SWRDEN_STSEMPTYSEL_FIELD_MASK },
    0,
    { BBH_TX_DEBUG_COUNTERS_SWRDEN_STSEMPTYSEL_FIELD_WIDTH },
    { BBH_TX_DEBUG_COUNTERS_SWRDEN_STSEMPTYSEL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: STSFULLSEL *****/
const ru_field_rec BBH_TX_DEBUG_COUNTERS_SWRDEN_STSFULLSEL_FIELD =
{
    "STSFULLSEL",
#if RU_INCLUDE_DESC
    "",
    "rd from the STS Full array\n",
#endif
    { BBH_TX_DEBUG_COUNTERS_SWRDEN_STSFULLSEL_FIELD_MASK },
    0,
    { BBH_TX_DEBUG_COUNTERS_SWRDEN_STSFULLSEL_FIELD_WIDTH },
    { BBH_TX_DEBUG_COUNTERS_SWRDEN_STSFULLSEL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: STSBEMPTYSEL *****/
const ru_field_rec BBH_TX_DEBUG_COUNTERS_SWRDEN_STSBEMPTYSEL_FIELD =
{
    "STSBEMPTYSEL",
#if RU_INCLUDE_DESC
    "",
    "rd from the STS beliow empty array\n",
#endif
    { BBH_TX_DEBUG_COUNTERS_SWRDEN_STSBEMPTYSEL_FIELD_MASK },
    0,
    { BBH_TX_DEBUG_COUNTERS_SWRDEN_STSBEMPTYSEL_FIELD_WIDTH },
    { BBH_TX_DEBUG_COUNTERS_SWRDEN_STSBEMPTYSEL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: STSFFWKPSEL *****/
const ru_field_rec BBH_TX_DEBUG_COUNTERS_SWRDEN_STSFFWKPSEL_FIELD =
{
    "STSFFWKPSEL",
#if RU_INCLUDE_DESC
    "",
    "rd from the STS full for wakeup empty array\n",
#endif
    { BBH_TX_DEBUG_COUNTERS_SWRDEN_STSFFWKPSEL_FIELD_MASK },
    0,
    { BBH_TX_DEBUG_COUNTERS_SWRDEN_STSFFWKPSEL_FIELD_WIDTH },
    { BBH_TX_DEBUG_COUNTERS_SWRDEN_STSFFWKPSEL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: MSGSEL *****/
const ru_field_rec BBH_TX_DEBUG_COUNTERS_SWRDEN_MSGSEL_FIELD =
{
    "MSGSEL",
#if RU_INCLUDE_DESC
    "",
    "rd from the MSG FIFO\n",
#endif
    { BBH_TX_DEBUG_COUNTERS_SWRDEN_MSGSEL_FIELD_MASK },
    0,
    { BBH_TX_DEBUG_COUNTERS_SWRDEN_MSGSEL_FIELD_WIDTH },
    { BBH_TX_DEBUG_COUNTERS_SWRDEN_MSGSEL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: MSGVSEL *****/
const ru_field_rec BBH_TX_DEBUG_COUNTERS_SWRDEN_MSGVSEL_FIELD =
{
    "MSGVSEL",
#if RU_INCLUDE_DESC
    "",
    "rd from the msg valid array\n",
#endif
    { BBH_TX_DEBUG_COUNTERS_SWRDEN_MSGVSEL_FIELD_MASK },
    0,
    { BBH_TX_DEBUG_COUNTERS_SWRDEN_MSGVSEL_FIELD_WIDTH },
    { BBH_TX_DEBUG_COUNTERS_SWRDEN_MSGVSEL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: EPNREQSEL *****/
const ru_field_rec BBH_TX_DEBUG_COUNTERS_SWRDEN_EPNREQSEL_FIELD =
{
    "EPNREQSEL",
#if RU_INCLUDE_DESC
    "",
    "rd from the epon request FIFO\n",
#endif
    { BBH_TX_DEBUG_COUNTERS_SWRDEN_EPNREQSEL_FIELD_MASK },
    0,
    { BBH_TX_DEBUG_COUNTERS_SWRDEN_EPNREQSEL_FIELD_WIDTH },
    { BBH_TX_DEBUG_COUNTERS_SWRDEN_EPNREQSEL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DATASEL *****/
const ru_field_rec BBH_TX_DEBUG_COUNTERS_SWRDEN_DATASEL_FIELD =
{
    "DATASEL",
#if RU_INCLUDE_DESC
    "",
    "rd from the DATA FIFO (SRAM and DDR)\n",
#endif
    { BBH_TX_DEBUG_COUNTERS_SWRDEN_DATASEL_FIELD_MASK },
    0,
    { BBH_TX_DEBUG_COUNTERS_SWRDEN_DATASEL_FIELD_WIDTH },
    { BBH_TX_DEBUG_COUNTERS_SWRDEN_DATASEL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: REORDERSEL *****/
const ru_field_rec BBH_TX_DEBUG_COUNTERS_SWRDEN_REORDERSEL_FIELD =
{
    "REORDERSEL",
#if RU_INCLUDE_DESC
    "",
    "rd from the reorder FIFO\n",
#endif
    { BBH_TX_DEBUG_COUNTERS_SWRDEN_REORDERSEL_FIELD_MASK },
    0,
    { BBH_TX_DEBUG_COUNTERS_SWRDEN_REORDERSEL_FIELD_WIDTH },
    { BBH_TX_DEBUG_COUNTERS_SWRDEN_REORDERSEL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: TSINFOSEL *****/
const ru_field_rec BBH_TX_DEBUG_COUNTERS_SWRDEN_TSINFOSEL_FIELD =
{
    "TSINFOSEL",
#if RU_INCLUDE_DESC
    "",
    "rd from the Timestamp Info FIFO\n",
#endif
    { BBH_TX_DEBUG_COUNTERS_SWRDEN_TSINFOSEL_FIELD_MASK },
    0,
    { BBH_TX_DEBUG_COUNTERS_SWRDEN_TSINFOSEL_FIELD_WIDTH },
    { BBH_TX_DEBUG_COUNTERS_SWRDEN_TSINFOSEL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: MACTXSEL *****/
const ru_field_rec BBH_TX_DEBUG_COUNTERS_SWRDEN_MACTXSEL_FIELD =
{
    "MACTXSEL",
#if RU_INCLUDE_DESC
    "",
    "rd from the MAC TX FIFO.\n\n",
#endif
    { BBH_TX_DEBUG_COUNTERS_SWRDEN_MACTXSEL_FIELD_MASK },
    0,
    { BBH_TX_DEBUG_COUNTERS_SWRDEN_MACTXSEL_FIELD_WIDTH },
    { BBH_TX_DEBUG_COUNTERS_SWRDEN_MACTXSEL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

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

/***** Register struct: BBH_TX_DEBUG_COUNTERS_SWRDEN *****/
const ru_reg_rec BBH_TX_DEBUG_COUNTERS_SWRDEN_REG =
{
    "DEBUG_COUNTERS_SWRDEN",
#if RU_INCLUDE_DESC
    "SW_RD_EN Register",
    "writing to this register creates a rd_en pulse to the selected array the SW wants to access.\n\nEach bit in the register represents one of the arrays the SW can access.\n\nThe address inside the array is determined in the previous register (sw_rd_address).\n\nWhen writing to this register the SW should assert only one bit. If more than one is asserted, The HW will return the value read from the lsb selected array.\n",
#endif
    { BBH_TX_DEBUG_COUNTERS_SWRDEN_REG_OFFSET },
    0,
    0,
    184,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    32,
    BBH_TX_DEBUG_COUNTERS_SWRDEN_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_TX_DEBUG_COUNTERS_SWRDADDR, TYPE: Type_BBH_TX_BBHTX_DEBUG_COUNTERS_SWRDADDR
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: RDADDR *****/
const ru_field_rec BBH_TX_DEBUG_COUNTERS_SWRDADDR_RDADDR_FIELD =
{
    "RDADDR",
#if RU_INCLUDE_DESC
    "",
    "The address inside the array the sw wants to read\n",
#endif
    { BBH_TX_DEBUG_COUNTERS_SWRDADDR_RDADDR_FIELD_MASK },
    0,
    { BBH_TX_DEBUG_COUNTERS_SWRDADDR_RDADDR_FIELD_WIDTH },
    { BBH_TX_DEBUG_COUNTERS_SWRDADDR_RDADDR_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_DEBUG_COUNTERS_SWRDADDR_FIELDS[] =
{
    &BBH_TX_DEBUG_COUNTERS_SWRDADDR_RDADDR_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_TX_DEBUG_COUNTERS_SWRDADDR *****/
const ru_reg_rec BBH_TX_DEBUG_COUNTERS_SWRDADDR_REG =
{
    "DEBUG_COUNTERS_SWRDADDR",
#if RU_INCLUDE_DESC
    "SW_RD_ADDR Register",
    "the address inside the array the SW wants to read\n",
#endif
    { BBH_TX_DEBUG_COUNTERS_SWRDADDR_REG_OFFSET },
    0,
    0,
    185,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_TX_DEBUG_COUNTERS_SWRDADDR_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_TX_DEBUG_COUNTERS_SWRDDATA, TYPE: Type_BBH_TX_BBHTX_DEBUG_COUNTERS_SWRDDATA
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DATA *****/
const ru_field_rec BBH_TX_DEBUG_COUNTERS_SWRDDATA_DATA_FIELD =
{
    "DATA",
#if RU_INCLUDE_DESC
    "",
    "data\n",
#endif
    { BBH_TX_DEBUG_COUNTERS_SWRDDATA_DATA_FIELD_MASK },
    0,
    { BBH_TX_DEBUG_COUNTERS_SWRDDATA_DATA_FIELD_WIDTH },
    { BBH_TX_DEBUG_COUNTERS_SWRDDATA_DATA_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_DEBUG_COUNTERS_SWRDDATA_FIELDS[] =
{
    &BBH_TX_DEBUG_COUNTERS_SWRDDATA_DATA_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_TX_DEBUG_COUNTERS_SWRDDATA *****/
const ru_reg_rec BBH_TX_DEBUG_COUNTERS_SWRDDATA_REG =
{
    "DEBUG_COUNTERS_SWRDDATA",
#if RU_INCLUDE_DESC
    "SW_RD_DATA Register",
    "indirect memories and arrays read data\n",
#endif
    { BBH_TX_DEBUG_COUNTERS_SWRDDATA_REG_OFFSET },
    0,
    0,
    186,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_TX_DEBUG_COUNTERS_SWRDDATA_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_TX_DEBUG_COUNTERS_UNIFIEDPKT, TYPE: Type_BBH_TX_BBHTX_DEBUG_COUNTERS_UNIFIEDPKT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: UNIFIED_PKT *****/
const ru_field_rec BBH_TX_DEBUG_COUNTERS_UNIFIEDPKT_UNIFIED_PKT_FIELD =
{
    "UNIFIED_PKT",
#if RU_INCLUDE_DESC
    "",
    "This counter counts the number of packets transmitted per unified port.\n",
#endif
    { BBH_TX_DEBUG_COUNTERS_UNIFIEDPKT_UNIFIED_PKT_FIELD_MASK },
    0,
    { BBH_TX_DEBUG_COUNTERS_UNIFIEDPKT_UNIFIED_PKT_FIELD_WIDTH },
    { BBH_TX_DEBUG_COUNTERS_UNIFIEDPKT_UNIFIED_PKT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_DEBUG_COUNTERS_UNIFIEDPKT_FIELDS[] =
{
    &BBH_TX_DEBUG_COUNTERS_UNIFIEDPKT_UNIFIED_PKT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_TX_DEBUG_COUNTERS_UNIFIEDPKT *****/
const ru_reg_rec BBH_TX_DEBUG_COUNTERS_UNIFIEDPKT_REG =
{
    "DEBUG_COUNTERS_UNIFIEDPKT",
#if RU_INCLUDE_DESC
    "UNIFIED_PKT_COUNTER 0..7 Register",
    "This counter array counts the number of transmitted packets through each interface in the unified BBH.\n\nThis counter is cleared when read and freezes when maximum value is reached.\n\n",
#endif
    { BBH_TX_DEBUG_COUNTERS_UNIFIEDPKT_REG_OFFSET },
    BBH_TX_DEBUG_COUNTERS_UNIFIEDPKT_REG_RAM_CNT,
    4,
    187,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_TX_DEBUG_COUNTERS_UNIFIEDPKT_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_TX_DEBUG_COUNTERS_UNIFIEDBYTE, TYPE: Type_BBH_TX_BBHTX_DEBUG_COUNTERS_UNIFIEDBYTE
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DDRBYTE *****/
const ru_field_rec BBH_TX_DEBUG_COUNTERS_UNIFIEDBYTE_DDRBYTE_FIELD =
{
    "DDRBYTE",
#if RU_INCLUDE_DESC
    "",
    "This counter counts the number of transmitted bytes from the DDr.\n",
#endif
    { BBH_TX_DEBUG_COUNTERS_UNIFIEDBYTE_DDRBYTE_FIELD_MASK },
    0,
    { BBH_TX_DEBUG_COUNTERS_UNIFIEDBYTE_DDRBYTE_FIELD_WIDTH },
    { BBH_TX_DEBUG_COUNTERS_UNIFIEDBYTE_DDRBYTE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_DEBUG_COUNTERS_UNIFIEDBYTE_FIELDS[] =
{
    &BBH_TX_DEBUG_COUNTERS_UNIFIEDBYTE_DDRBYTE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_TX_DEBUG_COUNTERS_UNIFIEDBYTE *****/
const ru_reg_rec BBH_TX_DEBUG_COUNTERS_UNIFIEDBYTE_REG =
{
    "DEBUG_COUNTERS_UNIFIEDBYTE",
#if RU_INCLUDE_DESC
    "UNIFIED_BYTE_COUNTER 0..7 Register",
    "This counter array counts the number of transmitted bytes through each interface in the unified BBH.\n\nThis counter is cleared when read and freezes when maximum value is reached.\n\n",
#endif
    { BBH_TX_DEBUG_COUNTERS_UNIFIEDBYTE_REG_OFFSET },
    BBH_TX_DEBUG_COUNTERS_UNIFIEDBYTE_REG_RAM_CNT,
    4,
    188,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_TX_DEBUG_COUNTERS_UNIFIEDBYTE_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_TX_DEBUG_COUNTERS_DSL_STS, TYPE: Type_BBH_TX_BBHTX_DEBUG_COUNTERS_DSL_STS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: OFLW_Q *****/
const ru_field_rec BBH_TX_DEBUG_COUNTERS_DSL_STS_OFLW_Q_FIELD =
{
    "OFLW_Q",
#if RU_INCLUDE_DESC
    "",
    "The queue in which an overflow occurred. valid only when credits_overflow is asserted.\n",
#endif
    { BBH_TX_DEBUG_COUNTERS_DSL_STS_OFLW_Q_FIELD_MASK },
    0,
    { BBH_TX_DEBUG_COUNTERS_DSL_STS_OFLW_Q_FIELD_WIDTH },
    { BBH_TX_DEBUG_COUNTERS_DSL_STS_OFLW_Q_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: OFLW *****/
const ru_field_rec BBH_TX_DEBUG_COUNTERS_DSL_STS_OFLW_FIELD =
{
    "OFLW",
#if RU_INCLUDE_DESC
    "",
    "Indication that one of the queues credits counter has reached a value that is larger than the Queues FIFO.\n",
#endif
    { BBH_TX_DEBUG_COUNTERS_DSL_STS_OFLW_FIELD_MASK },
    0,
    { BBH_TX_DEBUG_COUNTERS_DSL_STS_OFLW_FIELD_WIDTH },
    { BBH_TX_DEBUG_COUNTERS_DSL_STS_OFLW_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FLUSH_REQ *****/
const ru_field_rec BBH_TX_DEBUG_COUNTERS_DSL_STS_FLUSH_REQ_FIELD =
{
    "FLUSH_REQ",
#if RU_INCLUDE_DESC
    "",
    "flush request from the MAC\n",
#endif
    { BBH_TX_DEBUG_COUNTERS_DSL_STS_FLUSH_REQ_FIELD_MASK },
    0,
    { BBH_TX_DEBUG_COUNTERS_DSL_STS_FLUSH_REQ_FIELD_WIDTH },
    { BBH_TX_DEBUG_COUNTERS_DSL_STS_FLUSH_REQ_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FLUSH *****/
const ru_field_rec BBH_TX_DEBUG_COUNTERS_DSL_STS_FLUSH_FIELD =
{
    "FLUSH",
#if RU_INCLUDE_DESC
    "",
    "indication that the BBH is in flush state\n",
#endif
    { BBH_TX_DEBUG_COUNTERS_DSL_STS_FLUSH_FIELD_MASK },
    0,
    { BBH_TX_DEBUG_COUNTERS_DSL_STS_FLUSH_FIELD_WIDTH },
    { BBH_TX_DEBUG_COUNTERS_DSL_STS_FLUSH_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FLUSH_DONE *****/
const ru_field_rec BBH_TX_DEBUG_COUNTERS_DSL_STS_FLUSH_DONE_FIELD =
{
    "FLUSH_DONE",
#if RU_INCLUDE_DESC
    "",
    "Indication that the flush process is done\n",
#endif
    { BBH_TX_DEBUG_COUNTERS_DSL_STS_FLUSH_DONE_FIELD_MASK },
    0,
    { BBH_TX_DEBUG_COUNTERS_DSL_STS_FLUSH_DONE_FIELD_WIDTH },
    { BBH_TX_DEBUG_COUNTERS_DSL_STS_FLUSH_DONE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: INIT_REQ *****/
const ru_field_rec BBH_TX_DEBUG_COUNTERS_DSL_STS_INIT_REQ_FIELD =
{
    "INIT_REQ",
#if RU_INCLUDE_DESC
    "",
    "initialization request from the DSL. At init state, the BBH sets its credit counters to the amount indicated by the DSL\n",
#endif
    { BBH_TX_DEBUG_COUNTERS_DSL_STS_INIT_REQ_FIELD_MASK },
    0,
    { BBH_TX_DEBUG_COUNTERS_DSL_STS_INIT_REQ_FIELD_WIDTH },
    { BBH_TX_DEBUG_COUNTERS_DSL_STS_INIT_REQ_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: INIT_DONE *****/
const ru_field_rec BBH_TX_DEBUG_COUNTERS_DSL_STS_INIT_DONE_FIELD =
{
    "INIT_DONE",
#if RU_INCLUDE_DESC
    "",
    "Init process is done\n",
#endif
    { BBH_TX_DEBUG_COUNTERS_DSL_STS_INIT_DONE_FIELD_MASK },
    0,
    { BBH_TX_DEBUG_COUNTERS_DSL_STS_INIT_DONE_FIELD_WIDTH },
    { BBH_TX_DEBUG_COUNTERS_DSL_STS_INIT_DONE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: CREDIT_INIT_VAL *****/
const ru_field_rec BBH_TX_DEBUG_COUNTERS_DSL_STS_CREDIT_INIT_VAL_FIELD =
{
    "CREDIT_INIT_VAL",
#if RU_INCLUDE_DESC
    "",
    "the size of the FIFO of each queue in 8 bytes resolution.\n",
#endif
    { BBH_TX_DEBUG_COUNTERS_DSL_STS_CREDIT_INIT_VAL_FIELD_MASK },
    0,
    { BBH_TX_DEBUG_COUNTERS_DSL_STS_CREDIT_INIT_VAL_FIELD_WIDTH },
    { BBH_TX_DEBUG_COUNTERS_DSL_STS_CREDIT_INIT_VAL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_DEBUG_COUNTERS_DSL_STS_FIELDS[] =
{
    &BBH_TX_DEBUG_COUNTERS_DSL_STS_OFLW_Q_FIELD,
    &BBH_TX_DEBUG_COUNTERS_DSL_STS_OFLW_FIELD,
    &BBH_TX_DEBUG_COUNTERS_DSL_STS_FLUSH_REQ_FIELD,
    &BBH_TX_DEBUG_COUNTERS_DSL_STS_FLUSH_FIELD,
    &BBH_TX_DEBUG_COUNTERS_DSL_STS_FLUSH_DONE_FIELD,
    &BBH_TX_DEBUG_COUNTERS_DSL_STS_INIT_REQ_FIELD,
    &BBH_TX_DEBUG_COUNTERS_DSL_STS_INIT_DONE_FIELD,
    &BBH_TX_DEBUG_COUNTERS_DSL_STS_CREDIT_INIT_VAL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_TX_DEBUG_COUNTERS_DSL_STS *****/
const ru_reg_rec BBH_TX_DEBUG_COUNTERS_DSL_STS_REG =
{
    "DEBUG_COUNTERS_DSL_STS",
#if RU_INCLUDE_DESC
    "DSL_STS Register",
    "BBH status bits related to DSL mode of operation\n",
#endif
    { BBH_TX_DEBUG_COUNTERS_DSL_STS_REG_OFFSET },
    0,
    0,
    189,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    8,
    BBH_TX_DEBUG_COUNTERS_DSL_STS_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_TX_DEBUG_COUNTERS_DSL_CREDITS, TYPE: Type_BBH_TX_BBHTX_DEBUG_COUNTERS_DSL_CREDITS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: CREDITS *****/
const ru_field_rec BBH_TX_DEBUG_COUNTERS_DSL_CREDITS_CREDITS_FIELD =
{
    "CREDITS",
#if RU_INCLUDE_DESC
    "",
    "credits\n",
#endif
    { BBH_TX_DEBUG_COUNTERS_DSL_CREDITS_CREDITS_FIELD_MASK },
    0,
    { BBH_TX_DEBUG_COUNTERS_DSL_CREDITS_CREDITS_FIELD_WIDTH },
    { BBH_TX_DEBUG_COUNTERS_DSL_CREDITS_CREDITS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_DEBUG_COUNTERS_DSL_CREDITS_FIELDS[] =
{
    &BBH_TX_DEBUG_COUNTERS_DSL_CREDITS_CREDITS_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_TX_DEBUG_COUNTERS_DSL_CREDITS *****/
const ru_reg_rec BBH_TX_DEBUG_COUNTERS_DSL_CREDITS_REG =
{
    "DEBUG_COUNTERS_DSL_CREDITS",
#if RU_INCLUDE_DESC
    "DSL_CREDITS 0..15 Register",
    "per q status of the credit counter that indicate the number of free entries in the DSL TXIF module.\nThe BBH decrements the counter upon requesting data from the DMA. The TXIF increments the counter upon reading from the FIFO.\n",
#endif
    { BBH_TX_DEBUG_COUNTERS_DSL_CREDITS_REG_OFFSET },
    BBH_TX_DEBUG_COUNTERS_DSL_CREDITS_REG_RAM_CNT,
    4,
    190,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_TX_DEBUG_COUNTERS_DSL_CREDITS_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_TX_DEBUG_COUNTERS_DBGOUTREG, TYPE: Type_BBH_TX_BBHTX_DEBUG_COUNTERS_DBGOUTREG
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DBGVEC *****/
const ru_field_rec BBH_TX_DEBUG_COUNTERS_DBGOUTREG_DBGVEC_FIELD =
{
    "DBGVEC",
#if RU_INCLUDE_DESC
    "",
    "Selected debug vector.\n",
#endif
    { BBH_TX_DEBUG_COUNTERS_DBGOUTREG_DBGVEC_FIELD_MASK },
    0,
    { BBH_TX_DEBUG_COUNTERS_DBGOUTREG_DBGVEC_FIELD_WIDTH },
    { BBH_TX_DEBUG_COUNTERS_DBGOUTREG_DBGVEC_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_DEBUG_COUNTERS_DBGOUTREG_FIELDS[] =
{
    &BBH_TX_DEBUG_COUNTERS_DBGOUTREG_DBGVEC_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_TX_DEBUG_COUNTERS_DBGOUTREG *****/
const ru_reg_rec BBH_TX_DEBUG_COUNTERS_DBGOUTREG_REG =
{
    "DEBUG_COUNTERS_DBGOUTREG",
#if RU_INCLUDE_DESC
    "DEBUG_OUT_REG 0..31 Register",
    "an array including all the debug vectors of the BBH TX.\nentries 30 and 31 are DSL debug.\n",
#endif
    { BBH_TX_DEBUG_COUNTERS_DBGOUTREG_REG_OFFSET },
    BBH_TX_DEBUG_COUNTERS_DBGOUTREG_REG_RAM_CNT,
    4,
    191,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_TX_DEBUG_COUNTERS_DBGOUTREG_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_TX_DEBUG_COUNTERS_IN_SEGMENTATION, TYPE: Type_BBH_TX_BBHTX_DEBUG_COUNTERS_IN_SEGMENTATION
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: IN_SEGMENTATION *****/
const ru_field_rec BBH_TX_DEBUG_COUNTERS_IN_SEGMENTATION_IN_SEGMENTATION_FIELD =
{
    "IN_SEGMENTATION",
#if RU_INCLUDE_DESC
    "",
    "in_segmentation indication\n",
#endif
    { BBH_TX_DEBUG_COUNTERS_IN_SEGMENTATION_IN_SEGMENTATION_FIELD_MASK },
    0,
    { BBH_TX_DEBUG_COUNTERS_IN_SEGMENTATION_IN_SEGMENTATION_FIELD_WIDTH },
    { BBH_TX_DEBUG_COUNTERS_IN_SEGMENTATION_IN_SEGMENTATION_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_DEBUG_COUNTERS_IN_SEGMENTATION_FIELDS[] =
{
    &BBH_TX_DEBUG_COUNTERS_IN_SEGMENTATION_IN_SEGMENTATION_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_TX_DEBUG_COUNTERS_IN_SEGMENTATION *****/
const ru_reg_rec BBH_TX_DEBUG_COUNTERS_IN_SEGMENTATION_REG =
{
    "DEBUG_COUNTERS_IN_SEGMENTATION",
#if RU_INCLUDE_DESC
    "IN_SEGMENTATION 0..1 Register",
    "40 bit vector in which each bit represents if the segmentation SM is currently handling a PD of a certain TCONT.\n\nfirst address is for TCONTS [31:0]\nsecond is for TCONTS [39:32]\n",
#endif
    { BBH_TX_DEBUG_COUNTERS_IN_SEGMENTATION_REG_OFFSET },
    BBH_TX_DEBUG_COUNTERS_IN_SEGMENTATION_REG_RAM_CNT,
    4,
    192,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_TX_DEBUG_COUNTERS_IN_SEGMENTATION_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_TX_DEBUG_COUNTERS_UNIFIED_DATA_CREDITS, TYPE: Type_BBH_TX_BBHTX_DEBUG_COUNTERS_UNIFIED_DATA_CREDITS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: CREDITS *****/
const ru_field_rec BBH_TX_DEBUG_COUNTERS_UNIFIED_DATA_CREDITS_CREDITS_FIELD =
{
    "CREDITS",
#if RU_INCLUDE_DESC
    "",
    "credits\n",
#endif
    { BBH_TX_DEBUG_COUNTERS_UNIFIED_DATA_CREDITS_CREDITS_FIELD_MASK },
    0,
    { BBH_TX_DEBUG_COUNTERS_UNIFIED_DATA_CREDITS_CREDITS_FIELD_WIDTH },
    { BBH_TX_DEBUG_COUNTERS_UNIFIED_DATA_CREDITS_CREDITS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_DEBUG_COUNTERS_UNIFIED_DATA_CREDITS_FIELDS[] =
{
    &BBH_TX_DEBUG_COUNTERS_UNIFIED_DATA_CREDITS_CREDITS_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_TX_DEBUG_COUNTERS_UNIFIED_DATA_CREDITS *****/
const ru_reg_rec BBH_TX_DEBUG_COUNTERS_UNIFIED_DATA_CREDITS_REG =
{
    "DEBUG_COUNTERS_UNIFIED_DATA_CREDITS",
#if RU_INCLUDE_DESC
    "UNIFIED_DATA_CREDITS 0..7 Register",
    "per q status of the data credit counter that indicate the number of free entries in the FE TX buffer.\nThe BBH decrements the counter upon requesting data from the DMA. The counter is incremented when data is popped from the TX buffer and is sent to the MAC.\n",
#endif
    { BBH_TX_DEBUG_COUNTERS_UNIFIED_DATA_CREDITS_REG_OFFSET },
    BBH_TX_DEBUG_COUNTERS_UNIFIED_DATA_CREDITS_REG_RAM_CNT,
    4,
    193,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_TX_DEBUG_COUNTERS_UNIFIED_DATA_CREDITS_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_TX_DEBUG_COUNTERS_UNIFIED_PD_CREDITS, TYPE: Type_BBH_TX_BBHTX_DEBUG_COUNTERS_UNIFIED_PD_CREDITS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: CREDITS *****/
const ru_field_rec BBH_TX_DEBUG_COUNTERS_UNIFIED_PD_CREDITS_CREDITS_FIELD =
{
    "CREDITS",
#if RU_INCLUDE_DESC
    "",
    "credits\n",
#endif
    { BBH_TX_DEBUG_COUNTERS_UNIFIED_PD_CREDITS_CREDITS_FIELD_MASK },
    0,
    { BBH_TX_DEBUG_COUNTERS_UNIFIED_PD_CREDITS_CREDITS_FIELD_WIDTH },
    { BBH_TX_DEBUG_COUNTERS_UNIFIED_PD_CREDITS_CREDITS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_DEBUG_COUNTERS_UNIFIED_PD_CREDITS_FIELDS[] =
{
    &BBH_TX_DEBUG_COUNTERS_UNIFIED_PD_CREDITS_CREDITS_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_TX_DEBUG_COUNTERS_UNIFIED_PD_CREDITS *****/
const ru_reg_rec BBH_TX_DEBUG_COUNTERS_UNIFIED_PD_CREDITS_REG =
{
    "DEBUG_COUNTERS_UNIFIED_PD_CREDITS",
#if RU_INCLUDE_DESC
    "UNIFIED_PD_CREDITS 0..7 Register",
    "per q status of the pd credit counter that indicate the number of free entries in the FE PD buffer.\nThe BBH decrements the counter upon requesting data from the DMA. The counter is incremented when data is popped from the PD buffer and is sent to the MAC.\n",
#endif
    { BBH_TX_DEBUG_COUNTERS_UNIFIED_PD_CREDITS_REG_OFFSET },
    BBH_TX_DEBUG_COUNTERS_UNIFIED_PD_CREDITS_REG_RAM_CNT,
    4,
    194,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_TX_DEBUG_COUNTERS_UNIFIED_PD_CREDITS_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_TX_DEBUG_COUNTERS_FIFOS_OVERRUN, TYPE: Type_BBH_TX_BBHTX_DEBUG_COUNTERS_FIFOS_OVERRUN
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DDR_FIFO *****/
const ru_field_rec BBH_TX_DEBUG_COUNTERS_FIFOS_OVERRUN_DDR_FIFO_FIELD =
{
    "DDR_FIFO",
#if RU_INCLUDE_DESC
    "",
    "ddr_fifo_overrun\n",
#endif
    { BBH_TX_DEBUG_COUNTERS_FIFOS_OVERRUN_DDR_FIFO_FIELD_MASK },
    0,
    { BBH_TX_DEBUG_COUNTERS_FIFOS_OVERRUN_DDR_FIFO_FIELD_WIDTH },
    { BBH_TX_DEBUG_COUNTERS_FIFOS_OVERRUN_DDR_FIFO_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: SRAM_FIFO *****/
const ru_field_rec BBH_TX_DEBUG_COUNTERS_FIFOS_OVERRUN_SRAM_FIFO_FIELD =
{
    "SRAM_FIFO",
#if RU_INCLUDE_DESC
    "",
    "sram_fifo_overrun\n",
#endif
    { BBH_TX_DEBUG_COUNTERS_FIFOS_OVERRUN_SRAM_FIFO_FIELD_MASK },
    0,
    { BBH_TX_DEBUG_COUNTERS_FIFOS_OVERRUN_SRAM_FIFO_FIELD_WIDTH },
    { BBH_TX_DEBUG_COUNTERS_FIFOS_OVERRUN_SRAM_FIFO_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DDR_REORDER_FIFO *****/
const ru_field_rec BBH_TX_DEBUG_COUNTERS_FIFOS_OVERRUN_DDR_REORDER_FIFO_FIELD =
{
    "DDR_REORDER_FIFO",
#if RU_INCLUDE_DESC
    "",
    "ddr_reorder_fifo_overrun\n",
#endif
    { BBH_TX_DEBUG_COUNTERS_FIFOS_OVERRUN_DDR_REORDER_FIFO_FIELD_MASK },
    0,
    { BBH_TX_DEBUG_COUNTERS_FIFOS_OVERRUN_DDR_REORDER_FIFO_FIELD_WIDTH },
    { BBH_TX_DEBUG_COUNTERS_FIFOS_OVERRUN_DDR_REORDER_FIFO_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: SRAM_REORDER_FIFO *****/
const ru_field_rec BBH_TX_DEBUG_COUNTERS_FIFOS_OVERRUN_SRAM_REORDER_FIFO_FIELD =
{
    "SRAM_REORDER_FIFO",
#if RU_INCLUDE_DESC
    "",
    "sram_reorder_fifo_overrun\n",
#endif
    { BBH_TX_DEBUG_COUNTERS_FIFOS_OVERRUN_SRAM_REORDER_FIFO_FIELD_MASK },
    0,
    { BBH_TX_DEBUG_COUNTERS_FIFOS_OVERRUN_SRAM_REORDER_FIFO_FIELD_WIDTH },
    { BBH_TX_DEBUG_COUNTERS_FIFOS_OVERRUN_SRAM_REORDER_FIFO_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: BPM_FIFO *****/
const ru_field_rec BBH_TX_DEBUG_COUNTERS_FIFOS_OVERRUN_BPM_FIFO_FIELD =
{
    "BPM_FIFO",
#if RU_INCLUDE_DESC
    "",
    "bpm_fifo_overrun\n",
#endif
    { BBH_TX_DEBUG_COUNTERS_FIFOS_OVERRUN_BPM_FIFO_FIELD_MASK },
    0,
    { BBH_TX_DEBUG_COUNTERS_FIFOS_OVERRUN_BPM_FIFO_FIELD_WIDTH },
    { BBH_TX_DEBUG_COUNTERS_FIFOS_OVERRUN_BPM_FIFO_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: BPM_FLUSH_FIFO *****/
const ru_field_rec BBH_TX_DEBUG_COUNTERS_FIFOS_OVERRUN_BPM_FLUSH_FIFO_FIELD =
{
    "BPM_FLUSH_FIFO",
#if RU_INCLUDE_DESC
    "",
    "bpm_flush_fifo_overrun\n",
#endif
    { BBH_TX_DEBUG_COUNTERS_FIFOS_OVERRUN_BPM_FLUSH_FIFO_FIELD_MASK },
    0,
    { BBH_TX_DEBUG_COUNTERS_FIFOS_OVERRUN_BPM_FLUSH_FIFO_FIELD_WIDTH },
    { BBH_TX_DEBUG_COUNTERS_FIFOS_OVERRUN_BPM_FLUSH_FIFO_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: BPM_EOP_FIFO *****/
const ru_field_rec BBH_TX_DEBUG_COUNTERS_FIFOS_OVERRUN_BPM_EOP_FIFO_FIELD =
{
    "BPM_EOP_FIFO",
#if RU_INCLUDE_DESC
    "",
    "bpm_eop_fifo_overrun\n",
#endif
    { BBH_TX_DEBUG_COUNTERS_FIFOS_OVERRUN_BPM_EOP_FIFO_FIELD_MASK },
    0,
    { BBH_TX_DEBUG_COUNTERS_FIFOS_OVERRUN_BPM_EOP_FIFO_FIELD_WIDTH },
    { BBH_TX_DEBUG_COUNTERS_FIFOS_OVERRUN_BPM_EOP_FIFO_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: SBPM_FIFO *****/
const ru_field_rec BBH_TX_DEBUG_COUNTERS_FIFOS_OVERRUN_SBPM_FIFO_FIELD =
{
    "SBPM_FIFO",
#if RU_INCLUDE_DESC
    "",
    "sbpm_fifo_overrun\n",
#endif
    { BBH_TX_DEBUG_COUNTERS_FIFOS_OVERRUN_SBPM_FIFO_FIELD_MASK },
    0,
    { BBH_TX_DEBUG_COUNTERS_FIFOS_OVERRUN_SBPM_FIFO_FIELD_WIDTH },
    { BBH_TX_DEBUG_COUNTERS_FIFOS_OVERRUN_SBPM_FIFO_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: SBPM_FLUSH_FIFO *****/
const ru_field_rec BBH_TX_DEBUG_COUNTERS_FIFOS_OVERRUN_SBPM_FLUSH_FIFO_FIELD =
{
    "SBPM_FLUSH_FIFO",
#if RU_INCLUDE_DESC
    "",
    "sbpm_flush_fifo_overrun\n",
#endif
    { BBH_TX_DEBUG_COUNTERS_FIFOS_OVERRUN_SBPM_FLUSH_FIFO_FIELD_MASK },
    0,
    { BBH_TX_DEBUG_COUNTERS_FIFOS_OVERRUN_SBPM_FLUSH_FIFO_FIELD_WIDTH },
    { BBH_TX_DEBUG_COUNTERS_FIFOS_OVERRUN_SBPM_FLUSH_FIFO_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: SBPM_EOP_FIFO *****/
const ru_field_rec BBH_TX_DEBUG_COUNTERS_FIFOS_OVERRUN_SBPM_EOP_FIFO_FIELD =
{
    "SBPM_EOP_FIFO",
#if RU_INCLUDE_DESC
    "",
    "sbpm_eop_fifo_overrun\n",
#endif
    { BBH_TX_DEBUG_COUNTERS_FIFOS_OVERRUN_SBPM_EOP_FIFO_FIELD_MASK },
    0,
    { BBH_TX_DEBUG_COUNTERS_FIFOS_OVERRUN_SBPM_EOP_FIFO_FIELD_WIDTH },
    { BBH_TX_DEBUG_COUNTERS_FIFOS_OVERRUN_SBPM_EOP_FIFO_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DBR_SYNC_FIFO *****/
const ru_field_rec BBH_TX_DEBUG_COUNTERS_FIFOS_OVERRUN_DBR_SYNC_FIFO_FIELD =
{
    "DBR_SYNC_FIFO",
#if RU_INCLUDE_DESC
    "",
    "dbr_sync_fifo_overrun\n",
#endif
    { BBH_TX_DEBUG_COUNTERS_FIFOS_OVERRUN_DBR_SYNC_FIFO_FIELD_MASK },
    0,
    { BBH_TX_DEBUG_COUNTERS_FIFOS_OVERRUN_DBR_SYNC_FIFO_FIELD_WIDTH },
    { BBH_TX_DEBUG_COUNTERS_FIFOS_OVERRUN_DBR_SYNC_FIFO_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_DEBUG_COUNTERS_FIFOS_OVERRUN_FIELDS[] =
{
    &BBH_TX_DEBUG_COUNTERS_FIFOS_OVERRUN_DDR_FIFO_FIELD,
    &BBH_TX_DEBUG_COUNTERS_FIFOS_OVERRUN_SRAM_FIFO_FIELD,
    &BBH_TX_DEBUG_COUNTERS_FIFOS_OVERRUN_DDR_REORDER_FIFO_FIELD,
    &BBH_TX_DEBUG_COUNTERS_FIFOS_OVERRUN_SRAM_REORDER_FIFO_FIELD,
    &BBH_TX_DEBUG_COUNTERS_FIFOS_OVERRUN_BPM_FIFO_FIELD,
    &BBH_TX_DEBUG_COUNTERS_FIFOS_OVERRUN_BPM_FLUSH_FIFO_FIELD,
    &BBH_TX_DEBUG_COUNTERS_FIFOS_OVERRUN_BPM_EOP_FIFO_FIELD,
    &BBH_TX_DEBUG_COUNTERS_FIFOS_OVERRUN_SBPM_FIFO_FIELD,
    &BBH_TX_DEBUG_COUNTERS_FIFOS_OVERRUN_SBPM_FLUSH_FIFO_FIELD,
    &BBH_TX_DEBUG_COUNTERS_FIFOS_OVERRUN_SBPM_EOP_FIFO_FIELD,
    &BBH_TX_DEBUG_COUNTERS_FIFOS_OVERRUN_DBR_SYNC_FIFO_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_TX_DEBUG_COUNTERS_FIFOS_OVERRUN *****/
const ru_reg_rec BBH_TX_DEBUG_COUNTERS_FIFOS_OVERRUN_REG =
{
    "DEBUG_COUNTERS_FIFOS_OVERRUN",
#if RU_INCLUDE_DESC
    "FIFOS_OVERRUN Register",
    "debug information regarding overrun event in one of the internal fifos.\n",
#endif
    { BBH_TX_DEBUG_COUNTERS_FIFOS_OVERRUN_REG_OFFSET },
    0,
    0,
    195,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    11,
    BBH_TX_DEBUG_COUNTERS_FIFOS_OVERRUN_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_TX_DEBUG_COUNTERS_CHKSUMSRAMPD, TYPE: Type_BBH_TX_BBHTX_DEBUG_COUNTERS_CHKSUMSRAMPD
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: SRAMPD *****/
const ru_field_rec BBH_TX_DEBUG_COUNTERS_CHKSUMSRAMPD_SRAMPD_FIELD =
{
    "SRAMPD",
#if RU_INCLUDE_DESC
    "",
    "This counter counts the number of packets which were calculated checksum from the SRAM.\n",
#endif
    { BBH_TX_DEBUG_COUNTERS_CHKSUMSRAMPD_SRAMPD_FIELD_MASK },
    0,
    { BBH_TX_DEBUG_COUNTERS_CHKSUMSRAMPD_SRAMPD_FIELD_WIDTH },
    { BBH_TX_DEBUG_COUNTERS_CHKSUMSRAMPD_SRAMPD_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_DEBUG_COUNTERS_CHKSUMSRAMPD_FIELDS[] =
{
    &BBH_TX_DEBUG_COUNTERS_CHKSUMSRAMPD_SRAMPD_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_TX_DEBUG_COUNTERS_CHKSUMSRAMPD *****/
const ru_reg_rec BBH_TX_DEBUG_COUNTERS_CHKSUMSRAMPD_REG =
{
    "DEBUG_COUNTERS_CHKSUMSRAMPD",
#if RU_INCLUDE_DESC
    "CHKSUM_SRAM_PD_COUNTER Register",
    "This counter counts the number of received PD for packets for checksum calc from the SRAM.\nIt counts the packets for all queues together.\nThis counter is cleared when read and freezes when maximum value is reached.\n\n",
#endif
    { BBH_TX_DEBUG_COUNTERS_CHKSUMSRAMPD_REG_OFFSET },
    0,
    0,
    196,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_TX_DEBUG_COUNTERS_CHKSUMSRAMPD_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_TX_DEBUG_COUNTERS_CHKSUMDDRPD, TYPE: Type_BBH_TX_BBHTX_DEBUG_COUNTERS_CHKSUMDDRPD
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DDRPD *****/
const ru_field_rec BBH_TX_DEBUG_COUNTERS_CHKSUMDDRPD_DDRPD_FIELD =
{
    "DDRPD",
#if RU_INCLUDE_DESC
    "",
    "This counter counts the number of packets which were calculated checksum from the DDR.\n",
#endif
    { BBH_TX_DEBUG_COUNTERS_CHKSUMDDRPD_DDRPD_FIELD_MASK },
    0,
    { BBH_TX_DEBUG_COUNTERS_CHKSUMDDRPD_DDRPD_FIELD_WIDTH },
    { BBH_TX_DEBUG_COUNTERS_CHKSUMDDRPD_DDRPD_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_DEBUG_COUNTERS_CHKSUMDDRPD_FIELDS[] =
{
    &BBH_TX_DEBUG_COUNTERS_CHKSUMDDRPD_DDRPD_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_TX_DEBUG_COUNTERS_CHKSUMDDRPD *****/
const ru_reg_rec BBH_TX_DEBUG_COUNTERS_CHKSUMDDRPD_REG =
{
    "DEBUG_COUNTERS_CHKSUMDDRPD",
#if RU_INCLUDE_DESC
    "CHKSUM_DDR_PD_COUNTER Register",
    "This counter counts the number of received PDs for checksum calc packets  from the DDR.\nIt counts the packets for all queues together.\nThis counter is cleared when read and freezes when maximum value is reached.\n",
#endif
    { BBH_TX_DEBUG_COUNTERS_CHKSUMDDRPD_REG_OFFSET },
    0,
    0,
    197,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_TX_DEBUG_COUNTERS_CHKSUMDDRPD_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_TX_DEBUG_COUNTERS_CHKSUMSRAMBYTE, TYPE: Type_BBH_TX_BBHTX_DEBUG_COUNTERS_CHKSUMSRAMBYTE
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: SRAMBYTE *****/
const ru_field_rec BBH_TX_DEBUG_COUNTERS_CHKSUMSRAMBYTE_SRAMBYTE_FIELD =
{
    "SRAMBYTE",
#if RU_INCLUDE_DESC
    "",
    "This counter counts the number of calculated checksum bytes from the SRAM.\n",
#endif
    { BBH_TX_DEBUG_COUNTERS_CHKSUMSRAMBYTE_SRAMBYTE_FIELD_MASK },
    0,
    { BBH_TX_DEBUG_COUNTERS_CHKSUMSRAMBYTE_SRAMBYTE_FIELD_WIDTH },
    { BBH_TX_DEBUG_COUNTERS_CHKSUMSRAMBYTE_SRAMBYTE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_DEBUG_COUNTERS_CHKSUMSRAMBYTE_FIELDS[] =
{
    &BBH_TX_DEBUG_COUNTERS_CHKSUMSRAMBYTE_SRAMBYTE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_TX_DEBUG_COUNTERS_CHKSUMSRAMBYTE *****/
const ru_reg_rec BBH_TX_DEBUG_COUNTERS_CHKSUMSRAMBYTE_REG =
{
    "DEBUG_COUNTERS_CHKSUMSRAMBYTE",
#if RU_INCLUDE_DESC
    "CHKSUM_SRAM_BYTE_COUNTER Register",
    "This counter counts the number of checksum calc bytes from the SRAM.\nIt counts the bytes for all queues together.\nThis counter is cleared when read and freezes when maximum value is reached.\n\n",
#endif
    { BBH_TX_DEBUG_COUNTERS_CHKSUMSRAMBYTE_REG_OFFSET },
    0,
    0,
    198,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_TX_DEBUG_COUNTERS_CHKSUMSRAMBYTE_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_TX_DEBUG_COUNTERS_CHKSUMDDRBYTE, TYPE: Type_BBH_TX_BBHTX_DEBUG_COUNTERS_CHKSUMDDRBYTE
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DDRBYTE *****/
const ru_field_rec BBH_TX_DEBUG_COUNTERS_CHKSUMDDRBYTE_DDRBYTE_FIELD =
{
    "DDRBYTE",
#if RU_INCLUDE_DESC
    "",
    "This counter counts the number of calculated checksum bytes from the DDr.\n",
#endif
    { BBH_TX_DEBUG_COUNTERS_CHKSUMDDRBYTE_DDRBYTE_FIELD_MASK },
    0,
    { BBH_TX_DEBUG_COUNTERS_CHKSUMDDRBYTE_DDRBYTE_FIELD_WIDTH },
    { BBH_TX_DEBUG_COUNTERS_CHKSUMDDRBYTE_DDRBYTE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_DEBUG_COUNTERS_CHKSUMDDRBYTE_FIELDS[] =
{
    &BBH_TX_DEBUG_COUNTERS_CHKSUMDDRBYTE_DDRBYTE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_TX_DEBUG_COUNTERS_CHKSUMDDRBYTE *****/
const ru_reg_rec BBH_TX_DEBUG_COUNTERS_CHKSUMDDRBYTE_REG =
{
    "DEBUG_COUNTERS_CHKSUMDDRBYTE",
#if RU_INCLUDE_DESC
    "CHKSUM_DDR_BYTE_COUNTER Register",
    "This counter counts the number of checksum calc bytes from the DDR.\nIt counts the bytes for all queues together.\nThis counter is cleared when read and freezes when maximum value is reached.\n\n",
#endif
    { BBH_TX_DEBUG_COUNTERS_CHKSUMDDRBYTE_REG_OFFSET },
    0,
    0,
    199,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_TX_DEBUG_COUNTERS_CHKSUMDDRBYTE_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_TX_DEBUG_COUNTERS_CHKSUMBYPSRAMPD, TYPE: Type_BBH_TX_BBHTX_DEBUG_COUNTERS_CHKSUMBYPSRAMPD
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: SRAMPD *****/
const ru_field_rec BBH_TX_DEBUG_COUNTERS_CHKSUMBYPSRAMPD_SRAMPD_FIELD =
{
    "SRAMPD",
#if RU_INCLUDE_DESC
    "",
    "This counter counts the number of packets which were bypassing the checksum from the SRAM.\n",
#endif
    { BBH_TX_DEBUG_COUNTERS_CHKSUMBYPSRAMPD_SRAMPD_FIELD_MASK },
    0,
    { BBH_TX_DEBUG_COUNTERS_CHKSUMBYPSRAMPD_SRAMPD_FIELD_WIDTH },
    { BBH_TX_DEBUG_COUNTERS_CHKSUMBYPSRAMPD_SRAMPD_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_DEBUG_COUNTERS_CHKSUMBYPSRAMPD_FIELDS[] =
{
    &BBH_TX_DEBUG_COUNTERS_CHKSUMBYPSRAMPD_SRAMPD_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_TX_DEBUG_COUNTERS_CHKSUMBYPSRAMPD *****/
const ru_reg_rec BBH_TX_DEBUG_COUNTERS_CHKSUMBYPSRAMPD_REG =
{
    "DEBUG_COUNTERS_CHKSUMBYPSRAMPD",
#if RU_INCLUDE_DESC
    "CHKSUM_BYP_SRAM_PD_COUNTER Register",
    "This counter counts the number of received PD for packets for checksum bypass from the SRAM.\nIt counts the packets for all queues together.\nThis counter is cleared when read and freezes when maximum value is reached.\n\n",
#endif
    { BBH_TX_DEBUG_COUNTERS_CHKSUMBYPSRAMPD_REG_OFFSET },
    0,
    0,
    200,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_TX_DEBUG_COUNTERS_CHKSUMBYPSRAMPD_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_TX_DEBUG_COUNTERS_CHKSUMBYPDDRPD, TYPE: Type_BBH_TX_BBHTX_DEBUG_COUNTERS_CHKSUMBYPDDRPD
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DDRPD *****/
const ru_field_rec BBH_TX_DEBUG_COUNTERS_CHKSUMBYPDDRPD_DDRPD_FIELD =
{
    "DDRPD",
#if RU_INCLUDE_DESC
    "",
    "This counter counts the number of packets which were bypassing checksum from the DDR.\n",
#endif
    { BBH_TX_DEBUG_COUNTERS_CHKSUMBYPDDRPD_DDRPD_FIELD_MASK },
    0,
    { BBH_TX_DEBUG_COUNTERS_CHKSUMBYPDDRPD_DDRPD_FIELD_WIDTH },
    { BBH_TX_DEBUG_COUNTERS_CHKSUMBYPDDRPD_DDRPD_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_DEBUG_COUNTERS_CHKSUMBYPDDRPD_FIELDS[] =
{
    &BBH_TX_DEBUG_COUNTERS_CHKSUMBYPDDRPD_DDRPD_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_TX_DEBUG_COUNTERS_CHKSUMBYPDDRPD *****/
const ru_reg_rec BBH_TX_DEBUG_COUNTERS_CHKSUMBYPDDRPD_REG =
{
    "DEBUG_COUNTERS_CHKSUMBYPDDRPD",
#if RU_INCLUDE_DESC
    "CHKSUM_BYP_DDR_PD_COUNTER Register",
    "This counter counts the number of checksum bypass packets  from the DDR.\nIt counts the packets for all queues together.\nThis counter is cleared when read and freezes when maximum value is reached.\n",
#endif
    { BBH_TX_DEBUG_COUNTERS_CHKSUMBYPDDRPD_REG_OFFSET },
    0,
    0,
    201,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_TX_DEBUG_COUNTERS_CHKSUMBYPDDRPD_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_TX_DEBUG_COUNTERS_CHKSUMBYPSRAMBYTE, TYPE: Type_BBH_TX_BBHTX_DEBUG_COUNTERS_CHKSUMBYPSRAMBYTE
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: SRAMBYTE *****/
const ru_field_rec BBH_TX_DEBUG_COUNTERS_CHKSUMBYPSRAMBYTE_SRAMBYTE_FIELD =
{
    "SRAMBYTE",
#if RU_INCLUDE_DESC
    "",
    "This counter counts the number of bypass checksum bytes from the SRAM.\n",
#endif
    { BBH_TX_DEBUG_COUNTERS_CHKSUMBYPSRAMBYTE_SRAMBYTE_FIELD_MASK },
    0,
    { BBH_TX_DEBUG_COUNTERS_CHKSUMBYPSRAMBYTE_SRAMBYTE_FIELD_WIDTH },
    { BBH_TX_DEBUG_COUNTERS_CHKSUMBYPSRAMBYTE_SRAMBYTE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_DEBUG_COUNTERS_CHKSUMBYPSRAMBYTE_FIELDS[] =
{
    &BBH_TX_DEBUG_COUNTERS_CHKSUMBYPSRAMBYTE_SRAMBYTE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_TX_DEBUG_COUNTERS_CHKSUMBYPSRAMBYTE *****/
const ru_reg_rec BBH_TX_DEBUG_COUNTERS_CHKSUMBYPSRAMBYTE_REG =
{
    "DEBUG_COUNTERS_CHKSUMBYPSRAMBYTE",
#if RU_INCLUDE_DESC
    "CHKSUM_BYP_SRAM_BYTE_COUNTER Register",
    "This counter counts the number of checksum bypass bytes from the SRAM.\nIt counts the bytes for all queues together.\nThis counter is cleared when read and freezes when maximum value is reached.\n\n",
#endif
    { BBH_TX_DEBUG_COUNTERS_CHKSUMBYPSRAMBYTE_REG_OFFSET },
    0,
    0,
    202,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_TX_DEBUG_COUNTERS_CHKSUMBYPSRAMBYTE_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_TX_DEBUG_COUNTERS_CHKSUMBYPDDRBYTE, TYPE: Type_BBH_TX_BBHTX_DEBUG_COUNTERS_CHKSUMBYPDDRBYTE
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DDRBYTE *****/
const ru_field_rec BBH_TX_DEBUG_COUNTERS_CHKSUMBYPDDRBYTE_DDRBYTE_FIELD =
{
    "DDRBYTE",
#if RU_INCLUDE_DESC
    "",
    "This counter counts the number of bypass checksum bytes from the DDr.\n",
#endif
    { BBH_TX_DEBUG_COUNTERS_CHKSUMBYPDDRBYTE_DDRBYTE_FIELD_MASK },
    0,
    { BBH_TX_DEBUG_COUNTERS_CHKSUMBYPDDRBYTE_DDRBYTE_FIELD_WIDTH },
    { BBH_TX_DEBUG_COUNTERS_CHKSUMBYPDDRBYTE_DDRBYTE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_DEBUG_COUNTERS_CHKSUMBYPDDRBYTE_FIELDS[] =
{
    &BBH_TX_DEBUG_COUNTERS_CHKSUMBYPDDRBYTE_DDRBYTE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_TX_DEBUG_COUNTERS_CHKSUMBYPDDRBYTE *****/
const ru_reg_rec BBH_TX_DEBUG_COUNTERS_CHKSUMBYPDDRBYTE_REG =
{
    "DEBUG_COUNTERS_CHKSUMBYPDDRBYTE",
#if RU_INCLUDE_DESC
    "CHKSUM_BYP_DDR_BYTE_COUNTER Register",
    "This counter counts the number of checksum bypass bytes from the DDR.\nIt counts the bytes for all queues together.\nThis counter is cleared when read and freezes when maximum value is reached.\n\n",
#endif
    { BBH_TX_DEBUG_COUNTERS_CHKSUMBYPDDRBYTE_REG_OFFSET },
    0,
    0,
    203,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_TX_DEBUG_COUNTERS_CHKSUMBYPDDRBYTE_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_TX_DEBUG_COUNTERS_DISGEMDROPPKT, TYPE: Type_BBH_TX_BBHTX_DEBUG_COUNTERS_DISGEMDROPPKT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DROPPKT *****/
const ru_field_rec BBH_TX_DEBUG_COUNTERS_DISGEMDROPPKT_DROPPKT_FIELD =
{
    "DROPPKT",
#if RU_INCLUDE_DESC
    "",
    "This counter counts the number of dropped pkts due to GEM disable\n",
#endif
    { BBH_TX_DEBUG_COUNTERS_DISGEMDROPPKT_DROPPKT_FIELD_MASK },
    0,
    { BBH_TX_DEBUG_COUNTERS_DISGEMDROPPKT_DROPPKT_FIELD_WIDTH },
    { BBH_TX_DEBUG_COUNTERS_DISGEMDROPPKT_DROPPKT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_DEBUG_COUNTERS_DISGEMDROPPKT_FIELDS[] =
{
    &BBH_TX_DEBUG_COUNTERS_DISGEMDROPPKT_DROPPKT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_TX_DEBUG_COUNTERS_DISGEMDROPPKT *****/
const ru_reg_rec BBH_TX_DEBUG_COUNTERS_DISGEMDROPPKT_REG =
{
    "DEBUG_COUNTERS_DISGEMDROPPKT",
#if RU_INCLUDE_DESC
    "DISABLED_GEM_DROPPED_PACKETS_COUNTER Register",
    "This counter counts the number of packets that were dropped due to the fact that the GEM is disabled (relevant only to the WAN BBH)\n",
#endif
    { BBH_TX_DEBUG_COUNTERS_DISGEMDROPPKT_REG_OFFSET },
    0,
    0,
    204,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_TX_DEBUG_COUNTERS_DISGEMDROPPKT_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_TX_DEBUG_COUNTERS_DISGEMDROPBYTE, TYPE: Type_BBH_TX_BBHTX_DEBUG_COUNTERS_DISGEMDROPBYTE
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DROPBYTE *****/
const ru_field_rec BBH_TX_DEBUG_COUNTERS_DISGEMDROPBYTE_DROPBYTE_FIELD =
{
    "DROPBYTE",
#if RU_INCLUDE_DESC
    "",
    "This counter counts the number of dropped bytes due to GEM disable\n",
#endif
    { BBH_TX_DEBUG_COUNTERS_DISGEMDROPBYTE_DROPBYTE_FIELD_MASK },
    0,
    { BBH_TX_DEBUG_COUNTERS_DISGEMDROPBYTE_DROPBYTE_FIELD_WIDTH },
    { BBH_TX_DEBUG_COUNTERS_DISGEMDROPBYTE_DROPBYTE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_DEBUG_COUNTERS_DISGEMDROPBYTE_FIELDS[] =
{
    &BBH_TX_DEBUG_COUNTERS_DISGEMDROPBYTE_DROPBYTE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_TX_DEBUG_COUNTERS_DISGEMDROPBYTE *****/
const ru_reg_rec BBH_TX_DEBUG_COUNTERS_DISGEMDROPBYTE_REG =
{
    "DEBUG_COUNTERS_DISGEMDROPBYTE",
#if RU_INCLUDE_DESC
    "DISABLED_GEM_DROPPED_BYTES_COUNTER Register",
    "This counter counts the number of bytes that were dropped due to the fact that the GEM is disabled (relevant only to the WAN BBH)\n",
#endif
    { BBH_TX_DEBUG_COUNTERS_DISGEMDROPBYTE_REG_OFFSET },
    0,
    0,
    205,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_TX_DEBUG_COUNTERS_DISGEMDROPBYTE_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_TX_DEBUG_COUNTERS_ECNPKT, TYPE: Type_BBH_TX_BBHTX_DEBUG_COUNTERS_ECNPKT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: ECNPKT *****/
const ru_field_rec BBH_TX_DEBUG_COUNTERS_ECNPKT_ECNPKT_FIELD =
{
    "ECNPKT",
#if RU_INCLUDE_DESC
    "",
    "This counter counts the number of packets that were ecn remarked\n",
#endif
    { BBH_TX_DEBUG_COUNTERS_ECNPKT_ECNPKT_FIELD_MASK },
    0,
    { BBH_TX_DEBUG_COUNTERS_ECNPKT_ECNPKT_FIELD_WIDTH },
    { BBH_TX_DEBUG_COUNTERS_ECNPKT_ECNPKT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_DEBUG_COUNTERS_ECNPKT_FIELDS[] =
{
    &BBH_TX_DEBUG_COUNTERS_ECNPKT_ECNPKT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_TX_DEBUG_COUNTERS_ECNPKT *****/
const ru_reg_rec BBH_TX_DEBUG_COUNTERS_ECNPKT_REG =
{
    "DEBUG_COUNTERS_ECNPKT",
#if RU_INCLUDE_DESC
    "ECN_PKT_COUNTER Register",
    "This counter counts the number of packets that their ECN bits were remarked\n",
#endif
    { BBH_TX_DEBUG_COUNTERS_ECNPKT_REG_OFFSET },
    0,
    0,
    206,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_TX_DEBUG_COUNTERS_ECNPKT_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_TX_DEBUG_COUNTERS_ECNBYTES, TYPE: Type_BBH_TX_BBHTX_DEBUG_COUNTERS_ECNBYTES
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: ECNBYTES *****/
const ru_field_rec BBH_TX_DEBUG_COUNTERS_ECNBYTES_ECNBYTES_FIELD =
{
    "ECNBYTES",
#if RU_INCLUDE_DESC
    "",
    "This counter counts the number of bytes of ecn marked packets.\n",
#endif
    { BBH_TX_DEBUG_COUNTERS_ECNBYTES_ECNBYTES_FIELD_MASK },
    0,
    { BBH_TX_DEBUG_COUNTERS_ECNBYTES_ECNBYTES_FIELD_WIDTH },
    { BBH_TX_DEBUG_COUNTERS_ECNBYTES_ECNBYTES_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_DEBUG_COUNTERS_ECNBYTES_FIELDS[] =
{
    &BBH_TX_DEBUG_COUNTERS_ECNBYTES_ECNBYTES_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_TX_DEBUG_COUNTERS_ECNBYTES *****/
const ru_reg_rec BBH_TX_DEBUG_COUNTERS_ECNBYTES_REG =
{
    "DEBUG_COUNTERS_ECNBYTES",
#if RU_INCLUDE_DESC
    "ECN_BYTES_COUNTER Register",
    "This counter counts the number of bytes of packets that their ECN bits were remarked\n",
#endif
    { BBH_TX_DEBUG_COUNTERS_ECNBYTES_REG_OFFSET },
    0,
    0,
    207,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_TX_DEBUG_COUNTERS_ECNBYTES_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_TX_DEBUG_COUNTERS_ECNMARKERR, TYPE: Type_BBH_TX_BBHTX_DEBUG_COUNTERS_ECNMARKERR
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: V1 *****/
const ru_field_rec BBH_TX_DEBUG_COUNTERS_ECNMARKERR_V1_FIELD =
{
    "V1",
#if RU_INCLUDE_DESC
    "",
    "value1\n",
#endif
    { BBH_TX_DEBUG_COUNTERS_ECNMARKERR_V1_FIELD_MASK },
    0,
    { BBH_TX_DEBUG_COUNTERS_ECNMARKERR_V1_FIELD_WIDTH },
    { BBH_TX_DEBUG_COUNTERS_ECNMARKERR_V1_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: V2 *****/
const ru_field_rec BBH_TX_DEBUG_COUNTERS_ECNMARKERR_V2_FIELD =
{
    "V2",
#if RU_INCLUDE_DESC
    "",
    "value2\n",
#endif
    { BBH_TX_DEBUG_COUNTERS_ECNMARKERR_V2_FIELD_MASK },
    0,
    { BBH_TX_DEBUG_COUNTERS_ECNMARKERR_V2_FIELD_WIDTH },
    { BBH_TX_DEBUG_COUNTERS_ECNMARKERR_V2_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_DEBUG_COUNTERS_ECNMARKERR_FIELDS[] =
{
    &BBH_TX_DEBUG_COUNTERS_ECNMARKERR_V1_FIELD,
    &BBH_TX_DEBUG_COUNTERS_ECNMARKERR_V2_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_TX_DEBUG_COUNTERS_ECNMARKERR *****/
const ru_reg_rec BBH_TX_DEBUG_COUNTERS_ECNMARKERR_REG =
{
    "DEBUG_COUNTERS_ECNMARKERR",
#if RU_INCLUDE_DESC
    "ECN_MARK_ERR Register",
    "this counter counts two types of ecn mark error:\n1. the ECN capable bits were asserted in both PD and EH, but the ECN bits in the packet were zero.\n2. the ECN capable bits were asserted in both PD and EH, but the ECN bits in the packet were already asserted.\n",
#endif
    { BBH_TX_DEBUG_COUNTERS_ECNMARKERR_REG_OFFSET },
    0,
    0,
    208,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    BBH_TX_DEBUG_COUNTERS_ECNMARKERR_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_TX_DEBUG_COUNTERS_ECNLENERR, TYPE: Type_BBH_TX_BBHTX_DEBUG_COUNTERS_ECNLENERR
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: V1 *****/
const ru_field_rec BBH_TX_DEBUG_COUNTERS_ECNLENERR_V1_FIELD =
{
    "V1",
#if RU_INCLUDE_DESC
    "",
    "value1\n",
#endif
    { BBH_TX_DEBUG_COUNTERS_ECNLENERR_V1_FIELD_MASK },
    0,
    { BBH_TX_DEBUG_COUNTERS_ECNLENERR_V1_FIELD_WIDTH },
    { BBH_TX_DEBUG_COUNTERS_ECNLENERR_V1_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: V2 *****/
const ru_field_rec BBH_TX_DEBUG_COUNTERS_ECNLENERR_V2_FIELD =
{
    "V2",
#if RU_INCLUDE_DESC
    "",
    "value2\n",
#endif
    { BBH_TX_DEBUG_COUNTERS_ECNLENERR_V2_FIELD_MASK },
    0,
    { BBH_TX_DEBUG_COUNTERS_ECNLENERR_V2_FIELD_WIDTH },
    { BBH_TX_DEBUG_COUNTERS_ECNLENERR_V2_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_TX_DEBUG_COUNTERS_ECNLENERR_FIELDS[] =
{
    &BBH_TX_DEBUG_COUNTERS_ECNLENERR_V1_FIELD,
    &BBH_TX_DEBUG_COUNTERS_ECNLENERR_V2_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_TX_DEBUG_COUNTERS_ECNLENERR *****/
const ru_reg_rec BBH_TX_DEBUG_COUNTERS_ECNLENERR_REG =
{
    "DEBUG_COUNTERS_ECNLENERR",
#if RU_INCLUDE_DESC
    "ECN_PLEN_ERR Register",
    "These counters count plen error events related to ECN.\nevent 1 - packet is marked as ECN capable on both PD and EH, but the PLEN is shorter than the ECN offset\nevent 2 - packet is marked as ECN capable on both PD and EH and the packet is IPV, but the PLEN is shorter than the checksum offset.\n",
#endif
    { BBH_TX_DEBUG_COUNTERS_ECNLENERR_REG_OFFSET },
    0,
    0,
    209,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    BBH_TX_DEBUG_COUNTERS_ECNLENERR_FIELDS,
#endif
};

unsigned long BBH_TX_ADDRS[] =
{
    0x82890000,
    0x82892000,
    0x82894000,
    0x82896000,
    0x80170000,
};

static const ru_reg_rec *BBH_TX_REGS[] =
{
    &BBH_TX_COMMON_CONFIGURATIONS_MACTYPE_REG,
    &BBH_TX_COMMON_CONFIGURATIONS_BBCFG_1_TX_REG,
    &BBH_TX_COMMON_CONFIGURATIONS_BBCFG_2_TX_REG,
    &BBH_TX_COMMON_CONFIGURATIONS_BBCFG_3_TX_REG,
    &BBH_TX_COMMON_CONFIGURATIONS_DMACFG_TX_REG,
    &BBH_TX_COMMON_CONFIGURATIONS_SDMACFG_TX_REG,
    &BBH_TX_COMMON_CONFIGURATIONS_SBPMCFG_REG,
    &BBH_TX_COMMON_CONFIGURATIONS_DDRTMBASEL_REG,
    &BBH_TX_COMMON_CONFIGURATIONS_DDRTMBASEH_REG,
    &BBH_TX_COMMON_CONFIGURATIONS_DFIFOCTRL_REG,
    &BBH_TX_COMMON_CONFIGURATIONS_ARB_CFG_REG,
    &BBH_TX_COMMON_CONFIGURATIONS_BBROUTE_REG,
    &BBH_TX_COMMON_CONFIGURATIONS_DDRCFG_TX_REG,
    &BBH_TX_COMMON_CONFIGURATIONS_DDRCFG_TX2_REG,
    &BBH_TX_COMMON_CONFIGURATIONS_RNRCFG_1_REG,
    &BBH_TX_COMMON_CONFIGURATIONS_RNRCFG_2_REG,
    &BBH_TX_COMMON_CONFIGURATIONS_PERQTASK_REG,
    &BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_REG,
    &BBH_TX_COMMON_CONFIGURATIONS_DBGSEL_REG,
    &BBH_TX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_REG,
    &BBH_TX_COMMON_CONFIGURATIONS_GPR_REG,
    &BBH_TX_COMMON_CONFIGURATIONS_GENERAL_CFG_REG,
    &BBH_TX_COMMON_CONFIGURATIONS_ECNCFG_REG,
    &BBH_TX_COMMON_CONFIGURATIONS_DDRFPMINIBASEL_REG,
    &BBH_TX_COMMON_CONFIGURATIONS_DDRFPMINIBASEH_REG,
    &BBH_TX_WAN_CONFIGURATIONS_Q2RNR_REG,
    &BBH_TX_WAN_CONFIGURATIONS_QPROF_REG,
    &BBH_TX_WAN_CONFIGURATIONS_PDSIZE_REG,
    &BBH_TX_WAN_CONFIGURATIONS_PDWKUPH_REG,
    &BBH_TX_WAN_CONFIGURATIONS_PD_BYTE_TH_REG,
    &BBH_TX_WAN_CONFIGURATIONS_QMQ_REG,
    &BBH_TX_WAN_CONFIGURATIONS_STSSIZE_REG,
    &BBH_TX_WAN_CONFIGURATIONS_STSWKUPH_REG,
    &BBH_TX_WAN_CONFIGURATIONS_PD_BYTE_TH_EN_REG,
    &BBH_TX_WAN_CONFIGURATIONS_PDEMPTY_REG,
    &BBH_TX_WAN_CONFIGURATIONS_STSEMPTY_REG,
    &BBH_TX_WAN_CONFIGURATIONS_STSRNRCFG_1_REG,
    &BBH_TX_WAN_CONFIGURATIONS_STSRNRCFG_2_REG,
    &BBH_TX_WAN_CONFIGURATIONS_MSGRNRCFG_1_REG,
    &BBH_TX_WAN_CONFIGURATIONS_MSGRNRCFG_2_REG,
    &BBH_TX_WAN_CONFIGURATIONS_EPNCFG_REG,
    &BBH_TX_WAN_CONFIGURATIONS_FLOW2PORT_REG,
    &BBH_TX_WAN_CONFIGURATIONS_TS_REG,
    &BBH_TX_WAN_CONFIGURATIONS_DSL_CFG_REG,
    &BBH_TX_WAN_CONFIGURATIONS_DSL_CFG2_REG,
    &BBH_TX_WAN_CONFIGURATIONS_GEMCTRINIT_REG,
    &BBH_TX_WAN_CONFIGURATIONS_GEMCTRRD_REG,
    &BBH_TX_WAN_CONFIGURATIONS_GEMCTRRD0_REG,
    &BBH_TX_WAN_CONFIGURATIONS_GEMCTRRD1_REG,
    &BBH_TX_UNIFIED_CONFIGURATIONS_Q2RNR_REG,
    &BBH_TX_UNIFIED_CONFIGURATIONS_CHKSUMQ_REG,
    &BBH_TX_UNIFIED_CONFIGURATIONS_QPROF_REG,
    &BBH_TX_UNIFIED_CONFIGURATIONS_PDSIZE_REG,
    &BBH_TX_UNIFIED_CONFIGURATIONS_PDWKUPH_REG,
    &BBH_TX_UNIFIED_CONFIGURATIONS_PD_BYTE_TH_REG,
    &BBH_TX_UNIFIED_CONFIGURATIONS_QMQ_REG,
    &BBH_TX_UNIFIED_CONFIGURATIONS_MOTF_REG,
    &BBH_TX_UNIFIED_CONFIGURATIONS_PD_BYTE_TH_EN_REG,
    &BBH_TX_UNIFIED_CONFIGURATIONS_PDEMPTY_REG,
    &BBH_TX_UNIFIED_CONFIGURATIONS_GTXTHRESH_REG,
    &BBH_TX_UNIFIED_CONFIGURATIONS_EEE_REG,
    &BBH_TX_UNIFIED_CONFIGURATIONS_TS_REG,
    &BBH_TX_UNIFIED_CONFIGURATIONS_FE_CREDITS_REG,
    &BBH_TX_UNIFIED_CONFIGURATIONS_FEBASE_REG,
    &BBH_TX_UNIFIED_CONFIGURATIONS_FESIZE_REG,
    &BBH_TX_UNIFIED_CONFIGURATIONS_FEPDBASE_REG,
    &BBH_TX_UNIFIED_CONFIGURATIONS_FEPDSIZE_REG,
    &BBH_TX_UNIFIED_CONFIGURATIONS_TXWRR_REG,
    &BBH_TX_UNIFIED_CONFIGURATIONS_SGMTWRR_REG,
    &BBH_TX_UNIFIED_CONFIGURATIONS_TXTHRESH_REG,
    &BBH_TX_DEBUG_COUNTERS_TXSRAMPD_REG,
    &BBH_TX_DEBUG_COUNTERS_TXDDRPD_REG,
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
    &BBH_TX_DEBUG_COUNTERS_TXSRAMBYTE_REG,
    &BBH_TX_DEBUG_COUNTERS_TXDDRBYTE_REG,
    &BBH_TX_DEBUG_COUNTERS_SWRDEN_REG,
    &BBH_TX_DEBUG_COUNTERS_SWRDADDR_REG,
    &BBH_TX_DEBUG_COUNTERS_SWRDDATA_REG,
    &BBH_TX_DEBUG_COUNTERS_UNIFIEDPKT_REG,
    &BBH_TX_DEBUG_COUNTERS_UNIFIEDBYTE_REG,
    &BBH_TX_DEBUG_COUNTERS_DSL_STS_REG,
    &BBH_TX_DEBUG_COUNTERS_DSL_CREDITS_REG,
    &BBH_TX_DEBUG_COUNTERS_DBGOUTREG_REG,
    &BBH_TX_DEBUG_COUNTERS_IN_SEGMENTATION_REG,
    &BBH_TX_DEBUG_COUNTERS_UNIFIED_DATA_CREDITS_REG,
    &BBH_TX_DEBUG_COUNTERS_UNIFIED_PD_CREDITS_REG,
    &BBH_TX_DEBUG_COUNTERS_FIFOS_OVERRUN_REG,
    &BBH_TX_DEBUG_COUNTERS_CHKSUMSRAMPD_REG,
    &BBH_TX_DEBUG_COUNTERS_CHKSUMDDRPD_REG,
    &BBH_TX_DEBUG_COUNTERS_CHKSUMSRAMBYTE_REG,
    &BBH_TX_DEBUG_COUNTERS_CHKSUMDDRBYTE_REG,
    &BBH_TX_DEBUG_COUNTERS_CHKSUMBYPSRAMPD_REG,
    &BBH_TX_DEBUG_COUNTERS_CHKSUMBYPDDRPD_REG,
    &BBH_TX_DEBUG_COUNTERS_CHKSUMBYPSRAMBYTE_REG,
    &BBH_TX_DEBUG_COUNTERS_CHKSUMBYPDDRBYTE_REG,
    &BBH_TX_DEBUG_COUNTERS_DISGEMDROPPKT_REG,
    &BBH_TX_DEBUG_COUNTERS_DISGEMDROPBYTE_REG,
    &BBH_TX_DEBUG_COUNTERS_ECNPKT_REG,
    &BBH_TX_DEBUG_COUNTERS_ECNBYTES_REG,
    &BBH_TX_DEBUG_COUNTERS_ECNMARKERR_REG,
    &BBH_TX_DEBUG_COUNTERS_ECNLENERR_REG,
};

const ru_block_rec BBH_TX_BLOCK =
{
    "BBH_TX",
    BBH_TX_ADDRS,
    5,
    111,
    BBH_TX_REGS,
};
