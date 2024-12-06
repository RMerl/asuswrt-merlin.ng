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


#include "XRDP_NATC_DDR_CFG_AG.h"

/******************************************************************************
 * Register: NAME: NATC_DDR_CFG_DDR_SIZE, TYPE: Type_NATC_DDR_SIZE
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DDR_SIZE_TBL0 *****/
const ru_field_rec NATC_DDR_CFG_DDR_SIZE_DDR_SIZE_TBL0_FIELD =
{
    "DDR_SIZE_TBL0",
#if RU_INCLUDE_DESC
    "",
    "Number of entries in DDR table 0\nValue of 6 or above is invalid\nTo compute the actual size of the table, add DDR_BINS_PER_BUCKET field\nto the table size selection;\nFor instance, if DDR_BINS_PER_BUCKET is 3 (4 bins per bucket)\nand DDR_size is 3 (64k entries), the actual size of the table in DDR is\n(64*1024+3) multiply by total length (TOTAL_LEN) of key and context in bytes\nExtra 3 entries are used to store collided entries of the last entry\n",
#endif
    { NATC_DDR_CFG_DDR_SIZE_DDR_SIZE_TBL0_FIELD_MASK },
    0,
    { NATC_DDR_CFG_DDR_SIZE_DDR_SIZE_TBL0_FIELD_WIDTH },
    { NATC_DDR_CFG_DDR_SIZE_DDR_SIZE_TBL0_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DDR_SIZE_TBL1 *****/
const ru_field_rec NATC_DDR_CFG_DDR_SIZE_DDR_SIZE_TBL1_FIELD =
{
    "DDR_SIZE_TBL1",
#if RU_INCLUDE_DESC
    "",
    "Number of entries in DDR table 1\n0=8k; 1=16k; 2=32k; 3=64k; 4=128k; 5=256k; 6=invalid; 7=invalid\nSee description of DDR_SIZE_TBL0\n",
#endif
    { NATC_DDR_CFG_DDR_SIZE_DDR_SIZE_TBL1_FIELD_MASK },
    0,
    { NATC_DDR_CFG_DDR_SIZE_DDR_SIZE_TBL1_FIELD_WIDTH },
    { NATC_DDR_CFG_DDR_SIZE_DDR_SIZE_TBL1_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DDR_SIZE_TBL2 *****/
const ru_field_rec NATC_DDR_CFG_DDR_SIZE_DDR_SIZE_TBL2_FIELD =
{
    "DDR_SIZE_TBL2",
#if RU_INCLUDE_DESC
    "",
    "Number of entries in DDR table 2\nSee description of DDR_SIZE_TBL0\n0=8k; 1=16k; 2=32k; 3=64k; 4=128k; 5=256k; 6=invalid; 7=invalid\n",
#endif
    { NATC_DDR_CFG_DDR_SIZE_DDR_SIZE_TBL2_FIELD_MASK },
    0,
    { NATC_DDR_CFG_DDR_SIZE_DDR_SIZE_TBL2_FIELD_WIDTH },
    { NATC_DDR_CFG_DDR_SIZE_DDR_SIZE_TBL2_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DDR_SIZE_TBL3 *****/
const ru_field_rec NATC_DDR_CFG_DDR_SIZE_DDR_SIZE_TBL3_FIELD =
{
    "DDR_SIZE_TBL3",
#if RU_INCLUDE_DESC
    "",
    "Number of entries in DDR table 3\nSee description of DDR_SIZE_TBL0\n0=8k; 1=16k; 2=32k; 3=64k; 4=128k; 5=256k; 6=invalid; 7=invalid\n",
#endif
    { NATC_DDR_CFG_DDR_SIZE_DDR_SIZE_TBL3_FIELD_MASK },
    0,
    { NATC_DDR_CFG_DDR_SIZE_DDR_SIZE_TBL3_FIELD_WIDTH },
    { NATC_DDR_CFG_DDR_SIZE_DDR_SIZE_TBL3_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DDR_SIZE_TBL4 *****/
const ru_field_rec NATC_DDR_CFG_DDR_SIZE_DDR_SIZE_TBL4_FIELD =
{
    "DDR_SIZE_TBL4",
#if RU_INCLUDE_DESC
    "",
    "Number of entries in DDR table 4\nSee description of DDR_SIZE_TBL0\n0=8k; 1=16k; 2=32k; 3=64k; 4=128k; 5=256k; 6=invalid; 7=invalid\n",
#endif
    { NATC_DDR_CFG_DDR_SIZE_DDR_SIZE_TBL4_FIELD_MASK },
    0,
    { NATC_DDR_CFG_DDR_SIZE_DDR_SIZE_TBL4_FIELD_WIDTH },
    { NATC_DDR_CFG_DDR_SIZE_DDR_SIZE_TBL4_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DDR_SIZE_TBL5 *****/
const ru_field_rec NATC_DDR_CFG_DDR_SIZE_DDR_SIZE_TBL5_FIELD =
{
    "DDR_SIZE_TBL5",
#if RU_INCLUDE_DESC
    "",
    "Number of entries in DDR table 5\nSee description of DDR_SIZE_TBL0\n0=8k; 1=16k; 2=32k; 3=64k; 4=128k; 5=256k; 6=invalid; 7=invalid\n",
#endif
    { NATC_DDR_CFG_DDR_SIZE_DDR_SIZE_TBL5_FIELD_MASK },
    0,
    { NATC_DDR_CFG_DDR_SIZE_DDR_SIZE_TBL5_FIELD_WIDTH },
    { NATC_DDR_CFG_DDR_SIZE_DDR_SIZE_TBL5_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DDR_SIZE_TBL6 *****/
const ru_field_rec NATC_DDR_CFG_DDR_SIZE_DDR_SIZE_TBL6_FIELD =
{
    "DDR_SIZE_TBL6",
#if RU_INCLUDE_DESC
    "",
    "Number of entries in DDR table 6\nSee description of DDR_SIZE_TBL0\n0=8k; 1=16k; 2=32k; 3=64k; 4=128k; 5=256k; 6=invalid; 7=invalid\n",
#endif
    { NATC_DDR_CFG_DDR_SIZE_DDR_SIZE_TBL6_FIELD_MASK },
    0,
    { NATC_DDR_CFG_DDR_SIZE_DDR_SIZE_TBL6_FIELD_WIDTH },
    { NATC_DDR_CFG_DDR_SIZE_DDR_SIZE_TBL6_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DDR_SIZE_TBL7 *****/
const ru_field_rec NATC_DDR_CFG_DDR_SIZE_DDR_SIZE_TBL7_FIELD =
{
    "DDR_SIZE_TBL7",
#if RU_INCLUDE_DESC
    "",
    "Number of entries in DDR table 7\nSee description of DDR_SIZE_TBL0\n0=8k; 1=16k; 2=32k; 3=64k; 4=128k; 5=256k; 6=invalid; 7=invalid\n",
#endif
    { NATC_DDR_CFG_DDR_SIZE_DDR_SIZE_TBL7_FIELD_MASK },
    0,
    { NATC_DDR_CFG_DDR_SIZE_DDR_SIZE_TBL7_FIELD_WIDTH },
    { NATC_DDR_CFG_DDR_SIZE_DDR_SIZE_TBL7_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *NATC_DDR_CFG_DDR_SIZE_FIELDS[] =
{
    &NATC_DDR_CFG_DDR_SIZE_DDR_SIZE_TBL0_FIELD,
    &NATC_DDR_CFG_DDR_SIZE_DDR_SIZE_TBL1_FIELD,
    &NATC_DDR_CFG_DDR_SIZE_DDR_SIZE_TBL2_FIELD,
    &NATC_DDR_CFG_DDR_SIZE_DDR_SIZE_TBL3_FIELD,
    &NATC_DDR_CFG_DDR_SIZE_DDR_SIZE_TBL4_FIELD,
    &NATC_DDR_CFG_DDR_SIZE_DDR_SIZE_TBL5_FIELD,
    &NATC_DDR_CFG_DDR_SIZE_DDR_SIZE_TBL6_FIELD,
    &NATC_DDR_CFG_DDR_SIZE_DDR_SIZE_TBL7_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: NATC_DDR_CFG_DDR_SIZE *****/
const ru_reg_rec NATC_DDR_CFG_DDR_SIZE_REG =
{
    "DDR_SIZE",
#if RU_INCLUDE_DESC
    "NAT Cache DDR Size Register",
    "DDR Size Register\n",
#endif
    { NATC_DDR_CFG_DDR_SIZE_REG_OFFSET },
    0,
    0,
    610,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    8,
    NATC_DDR_CFG_DDR_SIZE_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: NATC_DDR_CFG_DDR_BINS_PER_BUCKET_0, TYPE: Type_NATC_DDR_BINS_PER_BUCKET_0
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DDR_BINS_PER_BUCKET_TBL0 *****/
const ru_field_rec NATC_DDR_CFG_DDR_BINS_PER_BUCKET_0_DDR_BINS_PER_BUCKET_TBL0_FIELD =
{
    "DDR_BINS_PER_BUCKET_TBL0",
#if RU_INCLUDE_DESC
    "",
    "Number of entries per bucket in DDR table 0\nThis is limited by bus max burst size.  For instance, if\nUBUS supports max burst size of 128 bytes, key length is 16\nbytes, maximum DDR_BINS_PER_BUCKET that can be programmed\nis 128 bytes / 16-bytes (bytes per bin) = 8 entries\n0h: 1 entry\n1h: 2 entries\n2h: 3 entries\n3h: 4 entries\n4h: 5 entries\n5h: 6 entries\n6h: 7 entries\n7h: 8 entries\n.............\n",
#endif
    { NATC_DDR_CFG_DDR_BINS_PER_BUCKET_0_DDR_BINS_PER_BUCKET_TBL0_FIELD_MASK },
    0,
    { NATC_DDR_CFG_DDR_BINS_PER_BUCKET_0_DDR_BINS_PER_BUCKET_TBL0_FIELD_WIDTH },
    { NATC_DDR_CFG_DDR_BINS_PER_BUCKET_0_DDR_BINS_PER_BUCKET_TBL0_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DDR_BINS_PER_BUCKET_TBL1 *****/
const ru_field_rec NATC_DDR_CFG_DDR_BINS_PER_BUCKET_0_DDR_BINS_PER_BUCKET_TBL1_FIELD =
{
    "DDR_BINS_PER_BUCKET_TBL1",
#if RU_INCLUDE_DESC
    "",
    "Number of entries per bucket in DDR table 1\nSee description of DDR_BINS_PER_BUCKET_TBL0\n",
#endif
    { NATC_DDR_CFG_DDR_BINS_PER_BUCKET_0_DDR_BINS_PER_BUCKET_TBL1_FIELD_MASK },
    0,
    { NATC_DDR_CFG_DDR_BINS_PER_BUCKET_0_DDR_BINS_PER_BUCKET_TBL1_FIELD_WIDTH },
    { NATC_DDR_CFG_DDR_BINS_PER_BUCKET_0_DDR_BINS_PER_BUCKET_TBL1_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DDR_BINS_PER_BUCKET_TBL2 *****/
const ru_field_rec NATC_DDR_CFG_DDR_BINS_PER_BUCKET_0_DDR_BINS_PER_BUCKET_TBL2_FIELD =
{
    "DDR_BINS_PER_BUCKET_TBL2",
#if RU_INCLUDE_DESC
    "",
    "Number of entries per bucket in DDR table 2\nSee description of DDR_BINS_PER_BUCKET_TBL0\n",
#endif
    { NATC_DDR_CFG_DDR_BINS_PER_BUCKET_0_DDR_BINS_PER_BUCKET_TBL2_FIELD_MASK },
    0,
    { NATC_DDR_CFG_DDR_BINS_PER_BUCKET_0_DDR_BINS_PER_BUCKET_TBL2_FIELD_WIDTH },
    { NATC_DDR_CFG_DDR_BINS_PER_BUCKET_0_DDR_BINS_PER_BUCKET_TBL2_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DDR_BINS_PER_BUCKET_TBL3 *****/
const ru_field_rec NATC_DDR_CFG_DDR_BINS_PER_BUCKET_0_DDR_BINS_PER_BUCKET_TBL3_FIELD =
{
    "DDR_BINS_PER_BUCKET_TBL3",
#if RU_INCLUDE_DESC
    "",
    "Number of entries per bucket in DDR table 3\nSee description of DDR_BINS_PER_BUCKET_TBL0\n",
#endif
    { NATC_DDR_CFG_DDR_BINS_PER_BUCKET_0_DDR_BINS_PER_BUCKET_TBL3_FIELD_MASK },
    0,
    { NATC_DDR_CFG_DDR_BINS_PER_BUCKET_0_DDR_BINS_PER_BUCKET_TBL3_FIELD_WIDTH },
    { NATC_DDR_CFG_DDR_BINS_PER_BUCKET_0_DDR_BINS_PER_BUCKET_TBL3_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *NATC_DDR_CFG_DDR_BINS_PER_BUCKET_0_FIELDS[] =
{
    &NATC_DDR_CFG_DDR_BINS_PER_BUCKET_0_DDR_BINS_PER_BUCKET_TBL0_FIELD,
    &NATC_DDR_CFG_DDR_BINS_PER_BUCKET_0_DDR_BINS_PER_BUCKET_TBL1_FIELD,
    &NATC_DDR_CFG_DDR_BINS_PER_BUCKET_0_DDR_BINS_PER_BUCKET_TBL2_FIELD,
    &NATC_DDR_CFG_DDR_BINS_PER_BUCKET_0_DDR_BINS_PER_BUCKET_TBL3_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: NATC_DDR_CFG_DDR_BINS_PER_BUCKET_0 *****/
const ru_reg_rec NATC_DDR_CFG_DDR_BINS_PER_BUCKET_0_REG =
{
    "DDR_BINS_PER_BUCKET_0",
#if RU_INCLUDE_DESC
    "NAT Cache DDR Bins Per Bucket 0 register",
    "DDR Bins Per Bucket Register 0\n",
#endif
    { NATC_DDR_CFG_DDR_BINS_PER_BUCKET_0_REG_OFFSET },
    0,
    0,
    611,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    NATC_DDR_CFG_DDR_BINS_PER_BUCKET_0_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: NATC_DDR_CFG_DDR_BINS_PER_BUCKET_1, TYPE: Type_NATC_DDR_BINS_PER_BUCKET_1
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DDR_BINS_PER_BUCKET_TBL4 *****/
const ru_field_rec NATC_DDR_CFG_DDR_BINS_PER_BUCKET_1_DDR_BINS_PER_BUCKET_TBL4_FIELD =
{
    "DDR_BINS_PER_BUCKET_TBL4",
#if RU_INCLUDE_DESC
    "",
    "Number of entries per bucket in DDR table 4\nSee description of DDR_BINS_PER_BUCKET_TBL0\n",
#endif
    { NATC_DDR_CFG_DDR_BINS_PER_BUCKET_1_DDR_BINS_PER_BUCKET_TBL4_FIELD_MASK },
    0,
    { NATC_DDR_CFG_DDR_BINS_PER_BUCKET_1_DDR_BINS_PER_BUCKET_TBL4_FIELD_WIDTH },
    { NATC_DDR_CFG_DDR_BINS_PER_BUCKET_1_DDR_BINS_PER_BUCKET_TBL4_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DDR_BINS_PER_BUCKET_TBL5 *****/
const ru_field_rec NATC_DDR_CFG_DDR_BINS_PER_BUCKET_1_DDR_BINS_PER_BUCKET_TBL5_FIELD =
{
    "DDR_BINS_PER_BUCKET_TBL5",
#if RU_INCLUDE_DESC
    "",
    "Number of entries per bucket in DDR table 5\nSee description of DDR_BINS_PER_BUCKET_TBL0\n",
#endif
    { NATC_DDR_CFG_DDR_BINS_PER_BUCKET_1_DDR_BINS_PER_BUCKET_TBL5_FIELD_MASK },
    0,
    { NATC_DDR_CFG_DDR_BINS_PER_BUCKET_1_DDR_BINS_PER_BUCKET_TBL5_FIELD_WIDTH },
    { NATC_DDR_CFG_DDR_BINS_PER_BUCKET_1_DDR_BINS_PER_BUCKET_TBL5_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DDR_BINS_PER_BUCKET_TBL6 *****/
const ru_field_rec NATC_DDR_CFG_DDR_BINS_PER_BUCKET_1_DDR_BINS_PER_BUCKET_TBL6_FIELD =
{
    "DDR_BINS_PER_BUCKET_TBL6",
#if RU_INCLUDE_DESC
    "",
    "Number of entries per bucket in DDR table 6\nSee description of DDR_BINS_PER_BUCKET_TBL0\n",
#endif
    { NATC_DDR_CFG_DDR_BINS_PER_BUCKET_1_DDR_BINS_PER_BUCKET_TBL6_FIELD_MASK },
    0,
    { NATC_DDR_CFG_DDR_BINS_PER_BUCKET_1_DDR_BINS_PER_BUCKET_TBL6_FIELD_WIDTH },
    { NATC_DDR_CFG_DDR_BINS_PER_BUCKET_1_DDR_BINS_PER_BUCKET_TBL6_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DDR_BINS_PER_BUCKET_TBL7 *****/
const ru_field_rec NATC_DDR_CFG_DDR_BINS_PER_BUCKET_1_DDR_BINS_PER_BUCKET_TBL7_FIELD =
{
    "DDR_BINS_PER_BUCKET_TBL7",
#if RU_INCLUDE_DESC
    "",
    "Number of entries per bucket in DDR table 7\nSee description of DDR_BINS_PER_BUCKET_TBL0\n",
#endif
    { NATC_DDR_CFG_DDR_BINS_PER_BUCKET_1_DDR_BINS_PER_BUCKET_TBL7_FIELD_MASK },
    0,
    { NATC_DDR_CFG_DDR_BINS_PER_BUCKET_1_DDR_BINS_PER_BUCKET_TBL7_FIELD_WIDTH },
    { NATC_DDR_CFG_DDR_BINS_PER_BUCKET_1_DDR_BINS_PER_BUCKET_TBL7_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *NATC_DDR_CFG_DDR_BINS_PER_BUCKET_1_FIELDS[] =
{
    &NATC_DDR_CFG_DDR_BINS_PER_BUCKET_1_DDR_BINS_PER_BUCKET_TBL4_FIELD,
    &NATC_DDR_CFG_DDR_BINS_PER_BUCKET_1_DDR_BINS_PER_BUCKET_TBL5_FIELD,
    &NATC_DDR_CFG_DDR_BINS_PER_BUCKET_1_DDR_BINS_PER_BUCKET_TBL6_FIELD,
    &NATC_DDR_CFG_DDR_BINS_PER_BUCKET_1_DDR_BINS_PER_BUCKET_TBL7_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: NATC_DDR_CFG_DDR_BINS_PER_BUCKET_1 *****/
const ru_reg_rec NATC_DDR_CFG_DDR_BINS_PER_BUCKET_1_REG =
{
    "DDR_BINS_PER_BUCKET_1",
#if RU_INCLUDE_DESC
    "NAT Cache DDR Bins Per Bucket 1 Register",
    "DDR Bins Per Bucket Register 1\n",
#endif
    { NATC_DDR_CFG_DDR_BINS_PER_BUCKET_1_REG_OFFSET },
    0,
    0,
    612,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    NATC_DDR_CFG_DDR_BINS_PER_BUCKET_1_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: NATC_DDR_CFG_TOTAL_LEN, TYPE: Type_NATC_TOTAL_LEN
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: TOTAL_LEN_TBL0 *****/
const ru_field_rec NATC_DDR_CFG_TOTAL_LEN_TOTAL_LEN_TBL0_FIELD =
{
    "TOTAL_LEN_TBL0",
#if RU_INCLUDE_DESC
    "",
    "Length of the lookup key plus context (including 8-byte counters) in DDR table 0\nThe context length (including 8-byte counters) is calculated by TOTAL_LEN minus KEY_LEN\nThe maximum value should not exceed hardware capability.\nFor instance, most projects have max of 80-bytes and BCM63158 has max value of 144-byte.\n0h: 48-byte\n1h: 64-byte\n2h: 80-byte\n3h: 96-byte\n4h: 112-byte\n5h: 128-byte\n6h: 144-byte\n7h: 160-byte\n",
#endif
    { NATC_DDR_CFG_TOTAL_LEN_TOTAL_LEN_TBL0_FIELD_MASK },
    0,
    { NATC_DDR_CFG_TOTAL_LEN_TOTAL_LEN_TBL0_FIELD_WIDTH },
    { NATC_DDR_CFG_TOTAL_LEN_TOTAL_LEN_TBL0_FIELD_SHIFT },
    2,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: TOTAL_LEN_TBL1 *****/
const ru_field_rec NATC_DDR_CFG_TOTAL_LEN_TOTAL_LEN_TBL1_FIELD =
{
    "TOTAL_LEN_TBL1",
#if RU_INCLUDE_DESC
    "",
    "Length of the lookup key plus context (including 8-byte counters) in DDR table 1\nSee description of TOTAL_LEN_TBL0.\n",
#endif
    { NATC_DDR_CFG_TOTAL_LEN_TOTAL_LEN_TBL1_FIELD_MASK },
    0,
    { NATC_DDR_CFG_TOTAL_LEN_TOTAL_LEN_TBL1_FIELD_WIDTH },
    { NATC_DDR_CFG_TOTAL_LEN_TOTAL_LEN_TBL1_FIELD_SHIFT },
    2,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: TOTAL_LEN_TBL2 *****/
const ru_field_rec NATC_DDR_CFG_TOTAL_LEN_TOTAL_LEN_TBL2_FIELD =
{
    "TOTAL_LEN_TBL2",
#if RU_INCLUDE_DESC
    "",
    "Length of the lookup key plus context (including 8-byte counters) in DDR table 2\nSee description of TOTAL_LEN_TBL0.\n",
#endif
    { NATC_DDR_CFG_TOTAL_LEN_TOTAL_LEN_TBL2_FIELD_MASK },
    0,
    { NATC_DDR_CFG_TOTAL_LEN_TOTAL_LEN_TBL2_FIELD_WIDTH },
    { NATC_DDR_CFG_TOTAL_LEN_TOTAL_LEN_TBL2_FIELD_SHIFT },
    2,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: TOTAL_LEN_TBL3 *****/
const ru_field_rec NATC_DDR_CFG_TOTAL_LEN_TOTAL_LEN_TBL3_FIELD =
{
    "TOTAL_LEN_TBL3",
#if RU_INCLUDE_DESC
    "",
    "Length of the lookup key plus context (including 8-byte counters) in DDR table 3\nSee description of TOTAL_LEN_TBL0.\n",
#endif
    { NATC_DDR_CFG_TOTAL_LEN_TOTAL_LEN_TBL3_FIELD_MASK },
    0,
    { NATC_DDR_CFG_TOTAL_LEN_TOTAL_LEN_TBL3_FIELD_WIDTH },
    { NATC_DDR_CFG_TOTAL_LEN_TOTAL_LEN_TBL3_FIELD_SHIFT },
    2,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: TOTAL_LEN_TBL4 *****/
const ru_field_rec NATC_DDR_CFG_TOTAL_LEN_TOTAL_LEN_TBL4_FIELD =
{
    "TOTAL_LEN_TBL4",
#if RU_INCLUDE_DESC
    "",
    "Length of the lookup key plus context (including 8-byte counters) in DDR table 4\nSee description of TOTAL_LEN_TBL0.\n",
#endif
    { NATC_DDR_CFG_TOTAL_LEN_TOTAL_LEN_TBL4_FIELD_MASK },
    0,
    { NATC_DDR_CFG_TOTAL_LEN_TOTAL_LEN_TBL4_FIELD_WIDTH },
    { NATC_DDR_CFG_TOTAL_LEN_TOTAL_LEN_TBL4_FIELD_SHIFT },
    2,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: TOTAL_LEN_TBL5 *****/
const ru_field_rec NATC_DDR_CFG_TOTAL_LEN_TOTAL_LEN_TBL5_FIELD =
{
    "TOTAL_LEN_TBL5",
#if RU_INCLUDE_DESC
    "",
    "Length of the lookup key plus context (including 8-byte counters) in DDR table 5\nSee description of TOTAL_LEN_TBL0.\n",
#endif
    { NATC_DDR_CFG_TOTAL_LEN_TOTAL_LEN_TBL5_FIELD_MASK },
    0,
    { NATC_DDR_CFG_TOTAL_LEN_TOTAL_LEN_TBL5_FIELD_WIDTH },
    { NATC_DDR_CFG_TOTAL_LEN_TOTAL_LEN_TBL5_FIELD_SHIFT },
    2,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: TOTAL_LEN_TBL6 *****/
const ru_field_rec NATC_DDR_CFG_TOTAL_LEN_TOTAL_LEN_TBL6_FIELD =
{
    "TOTAL_LEN_TBL6",
#if RU_INCLUDE_DESC
    "",
    "Length of the lookup key plus context (including 8-byte counters) in DDR table 6\nSee description of TOTAL_LEN_TBL0.\n",
#endif
    { NATC_DDR_CFG_TOTAL_LEN_TOTAL_LEN_TBL6_FIELD_MASK },
    0,
    { NATC_DDR_CFG_TOTAL_LEN_TOTAL_LEN_TBL6_FIELD_WIDTH },
    { NATC_DDR_CFG_TOTAL_LEN_TOTAL_LEN_TBL6_FIELD_SHIFT },
    2,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: TOTAL_LEN_TBL7 *****/
const ru_field_rec NATC_DDR_CFG_TOTAL_LEN_TOTAL_LEN_TBL7_FIELD =
{
    "TOTAL_LEN_TBL7",
#if RU_INCLUDE_DESC
    "",
    "Length of the lookup key plus context (including 8-byte counters) in DDR table 7\nSee description of TOTAL_LEN_TBL0.\n",
#endif
    { NATC_DDR_CFG_TOTAL_LEN_TOTAL_LEN_TBL7_FIELD_MASK },
    0,
    { NATC_DDR_CFG_TOTAL_LEN_TOTAL_LEN_TBL7_FIELD_WIDTH },
    { NATC_DDR_CFG_TOTAL_LEN_TOTAL_LEN_TBL7_FIELD_SHIFT },
    2,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *NATC_DDR_CFG_TOTAL_LEN_FIELDS[] =
{
    &NATC_DDR_CFG_TOTAL_LEN_TOTAL_LEN_TBL0_FIELD,
    &NATC_DDR_CFG_TOTAL_LEN_TOTAL_LEN_TBL1_FIELD,
    &NATC_DDR_CFG_TOTAL_LEN_TOTAL_LEN_TBL2_FIELD,
    &NATC_DDR_CFG_TOTAL_LEN_TOTAL_LEN_TBL3_FIELD,
    &NATC_DDR_CFG_TOTAL_LEN_TOTAL_LEN_TBL4_FIELD,
    &NATC_DDR_CFG_TOTAL_LEN_TOTAL_LEN_TBL5_FIELD,
    &NATC_DDR_CFG_TOTAL_LEN_TOTAL_LEN_TBL6_FIELD,
    &NATC_DDR_CFG_TOTAL_LEN_TOTAL_LEN_TBL7_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: NATC_DDR_CFG_TOTAL_LEN *****/
const ru_reg_rec NATC_DDR_CFG_TOTAL_LEN_REG =
{
    "TOTAL_LEN",
#if RU_INCLUDE_DESC
    "NAT Cache Total Length Register",
    "DDR TABLE Total Length Register\n",
#endif
    { NATC_DDR_CFG_TOTAL_LEN_REG_OFFSET },
    0,
    0,
    613,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    8,
    NATC_DDR_CFG_TOTAL_LEN_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: NATC_DDR_CFG_SM_STATUS, TYPE: Type_NATC_SM_STATUS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: NAT_STATE *****/
const ru_field_rec NATC_DDR_CFG_SM_STATUS_NAT_STATE_FIELD =
{
    "NAT_STATE",
#if RU_INCLUDE_DESC
    "",
    "Nat state machine.\n15'b000000000000000: NAT_ST_IDLE.\n15'b000000000000001: NAT_ST_IDLE_WRITE_SMEM.\n15'b000000000000010: NAT_ST_IDLE_DDR_PENDING.\n15'b000000000000100: NAT_ST_HASH.\n15'b000000000001000: NAT_ST_NAT_MEM_READ_REQ.\n15'b000000000010000: NAT_ST_NAT_MEM_WRITE_REQ.\n15'b000000000100000: NAT_ST_READ_SMEM.\n15'b000000001000000: NAT_ST_UPDATE_DDR.\n15'b000000010000000: NAT_ST_IDLE_BLOCKING_PENDING.\n15'b000000100000000: NAT_ST_EVICT_WAIT.\n15'b000001000000000: NAT_ST_CHECK_NON_CACHEABLE.\n15'b000010000000000: NAT_ST_WAIT.\n15'b000100000000000: NAT_ST_WAIT_NATC_MEM_REQ_DONE.\n15'b001000000000000: NAT_ST_CACHE_FLUSH.\n15'b010000000000000: NAT_ST_DDR_MISS_0.\n15'b100000000000000: NAT_ST_DDR_MISS_1.\n",
#endif
    { NATC_DDR_CFG_SM_STATUS_NAT_STATE_FIELD_MASK },
    0,
    { NATC_DDR_CFG_SM_STATUS_NAT_STATE_FIELD_WIDTH },
    { NATC_DDR_CFG_SM_STATUS_NAT_STATE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: WB_STATE *****/
const ru_field_rec NATC_DDR_CFG_SM_STATUS_WB_STATE_FIELD =
{
    "WB_STATE",
#if RU_INCLUDE_DESC
    "",
    "Write-back state machine.\n1'b0: WB_ST_IDLE.\n1'b1: WB_ST_WRITE_BACIF.\n",
#endif
    { NATC_DDR_CFG_SM_STATUS_WB_STATE_FIELD_MASK },
    0,
    { NATC_DDR_CFG_SM_STATUS_WB_STATE_FIELD_WIDTH },
    { NATC_DDR_CFG_SM_STATUS_WB_STATE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: RUNNER_CMD_STATE *****/
const ru_field_rec NATC_DDR_CFG_SM_STATUS_RUNNER_CMD_STATE_FIELD =
{
    "RUNNER_CMD_STATE",
#if RU_INCLUDE_DESC
    "",
    "Runner command state machine.\n1'b0: RUNNER_CMD_ST_IDLE.\n1'b1: RUNNER_CMD_ST_WRITE_RUNNER_FIFO.\n",
#endif
    { NATC_DDR_CFG_SM_STATUS_RUNNER_CMD_STATE_FIELD_MASK },
    0,
    { NATC_DDR_CFG_SM_STATUS_RUNNER_CMD_STATE_FIELD_WIDTH },
    { NATC_DDR_CFG_SM_STATUS_RUNNER_CMD_STATE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DDR_REP_STATE *****/
const ru_field_rec NATC_DDR_CFG_SM_STATUS_DDR_REP_STATE_FIELD =
{
    "DDR_REP_STATE",
#if RU_INCLUDE_DESC
    "",
    "DDR reply state machine.\n3'b000: DDR_REP_ST_IDLE.\n3'b001: DDR_REP_ST_READ_DATA.\n3'b010: DDR_REP_ST_READ_RESULT.\n3'b011: DDR_REP_ST_READ_WAIT.\n3'b100: DDR_REP_ST_EVICT_WR_NON_CACHEABLE.\n",
#endif
    { NATC_DDR_CFG_SM_STATUS_DDR_REP_STATE_FIELD_MASK },
    0,
    { NATC_DDR_CFG_SM_STATUS_DDR_REP_STATE_FIELD_WIDTH },
    { NATC_DDR_CFG_SM_STATUS_DDR_REP_STATE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DDR_REQ_STATE *****/
const ru_field_rec NATC_DDR_CFG_SM_STATUS_DDR_REQ_STATE_FIELD =
{
    "DDR_REQ_STATE",
#if RU_INCLUDE_DESC
    "",
    "DDR request state machine.\n2'b00: DDR_REQ_ST_IDLE.\n2'b01: DDR_REQ_ST_WRITE_HEADER.\n2'b10: DDR_REQ_ST_WRITE_HEADER_DELAY.\n",
#endif
    { NATC_DDR_CFG_SM_STATUS_DDR_REQ_STATE_FIELD_MASK },
    0,
    { NATC_DDR_CFG_SM_STATUS_DDR_REQ_STATE_FIELD_WIDTH },
    { NATC_DDR_CFG_SM_STATUS_DDR_REQ_STATE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: APB_STATE *****/
const ru_field_rec NATC_DDR_CFG_SM_STATUS_APB_STATE_FIELD =
{
    "APB_STATE",
#if RU_INCLUDE_DESC
    "",
    "APB to RBUS bridge state machine.\n2'b00: APB_ST_IDLE.\n2'b01: APB_ST_RW.\n2'b10: AOB_ST_END.\n",
#endif
    { NATC_DDR_CFG_SM_STATUS_APB_STATE_FIELD_MASK },
    0,
    { NATC_DDR_CFG_SM_STATUS_APB_STATE_FIELD_WIDTH },
    { NATC_DDR_CFG_SM_STATUS_APB_STATE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DEBUG_SEL *****/
const ru_field_rec NATC_DDR_CFG_SM_STATUS_DEBUG_SEL_FIELD =
{
    "DEBUG_SEL",
#if RU_INCLUDE_DESC
    "",
    "Debug bus select.\n2'b00: prb_nat_control.\n2'b01: prb_cmd_control.\n2'b10: prb_wb_control.\n2'b11: prb_ddr_control.\n",
#endif
    { NATC_DDR_CFG_SM_STATUS_DEBUG_SEL_FIELD_MASK },
    0,
    { NATC_DDR_CFG_SM_STATUS_DEBUG_SEL_FIELD_WIDTH },
    { NATC_DDR_CFG_SM_STATUS_DEBUG_SEL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *NATC_DDR_CFG_SM_STATUS_FIELDS[] =
{
    &NATC_DDR_CFG_SM_STATUS_NAT_STATE_FIELD,
    &NATC_DDR_CFG_SM_STATUS_WB_STATE_FIELD,
    &NATC_DDR_CFG_SM_STATUS_RUNNER_CMD_STATE_FIELD,
    &NATC_DDR_CFG_SM_STATUS_DDR_REP_STATE_FIELD,
    &NATC_DDR_CFG_SM_STATUS_DDR_REQ_STATE_FIELD,
    &NATC_DDR_CFG_SM_STATUS_APB_STATE_FIELD,
    &NATC_DDR_CFG_SM_STATUS_DEBUG_SEL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: NATC_DDR_CFG_SM_STATUS *****/
const ru_reg_rec NATC_DDR_CFG_SM_STATUS_REG =
{
    "SM_STATUS",
#if RU_INCLUDE_DESC
    "NAT State Machine Status Register",
    "NAT State Machine Status Register\n",
#endif
    { NATC_DDR_CFG_SM_STATUS_REG_OFFSET },
    0,
    0,
    614,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    7,
    NATC_DDR_CFG_SM_STATUS_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: NATC_DDR_CFG_DDR_HASH_MODE, TYPE: Type_NATC_DDR_HASH_MODE
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DDR_HASH_MODE_TBL0 *****/
const ru_field_rec NATC_DDR_CFG_DDR_HASH_MODE_DDR_HASH_MODE_TBL0_FIELD =
{
    "DDR_HASH_MODE_TBL0",
#if RU_INCLUDE_DESC
    "",
    "Hash algorithm used for DDR table 0 lookup.\nHash value is DDR table size dependent.\n0h: 32-bit rolling XOR hash is used as DDR hash function. It is reduced to N-bit\nDDR table size is 8K,   N = 13.\nDDR table size is 16K,  N = 14.\nDDR table size is 32K,  N = 15.\nDDR table size is 64K,  N = 16.\nDDR table size is 128K, N = 17.\nDDR table size is 256K, N = 18.\n1h: CRC32 hash is used as DDR hash function. CRC32 is reduced to N-bit using\nthe same method as in 32-bit rolling XOR hash.\nDDR table size is 8K,   N = 13.\nDDR table size is 16K,  N = 14.\nDDR table size is 32K,  N = 15.\nDDR table size is 64K,  N = 16.\nDDR table size is 128K, N = 17.\nDDR table size is 256K, N = 18.\n2h: CRC32 hash is used as DDR hash function. CRC32[N:0] is used as hash value\nDDR table size is 8K,   N = 12.\nDDR table size is 16K,  N = 13.\nDDR table size is 32K,  N = 14.\nDDR table size is 64K,  N = 15.\nDDR table size is 128K, N = 16.\nDDR table size is 256K, N = 17.\n3h: CRC32 hash is used as DDR hash function. CRC32[31:N] is used as hash value\nDDR table size is 8K,   N = 19.\nDDR table size is 16K,  N = 18.\nDDR table size is 32K,  N = 17.\nDDR table size is 64K,  N = 16.\nDDR table size is 128K, N = 15.\nDDR table size is 256K, N = 14.\n4h: RSS hash is used as DDR hash function using secret key 0. RSS[N:0] is used as hash value.\nDDR table size is 8K,   N = 13.\nDDR table size is 16K,  N = 14.\nDDR table size is 32K,  N = 15.\nDDR table size is 64K,  N = 16.\nDDR table size is 128K, N = 17.\nDDR table size is 256K, N = 18.\n5h: RSS hash is used as DDR hash function using secret key 1. RSS[N:0] is used as hash value.\nDDR table size is 8K,   N = 13.\nDDR table size is 16K,  N = 14.\nDDR table size is 32K,  N = 15.\nDDR table size is 64K,  N = 16.\nDDR table size is 128K, N = 17.\nDDR table size is 256K, N = 18.\n6h: RSS hash is used as DDR hash function using secret key 2. RSS[N:0] is used as hash value.\nDDR table size is 8K,   N = 13.\nDDR table size is 16K,  N = 14.\nDDR table size is 32K,  N = 15.\nDDR table size is 64K,  N = 16.\nDDR table size is 128K, N = 17.\nDDR table size is 256K, N = 18.\n7h: RSS hash is used as DDR hash function using secret key 3. RSS[N:0] is used as hash value.\nDDR table size is 8K,   N = 13.\nDDR table size is 16K,  N = 14.\nDDR table size is 32K,  N = 15.\nDDR table size is 64K,  N = 16.\nDDR table size is 128K, N = 17.\nDDR table size is 256K, N = 18.\n8h: Key[N:4] is used as hash value.\nDDR table size is 8K,   N = 16.\nDDR table size is 16K,  N = 17.\nDDR table size is 32K,  N = 18.\nDDR table size is 64K,  N = 19.\nDDR table size is 128K, N = 20.\nDDR table size is 256K, N = 21.\n",
#endif
    { NATC_DDR_CFG_DDR_HASH_MODE_DDR_HASH_MODE_TBL0_FIELD_MASK },
    0,
    { NATC_DDR_CFG_DDR_HASH_MODE_DDR_HASH_MODE_TBL0_FIELD_WIDTH },
    { NATC_DDR_CFG_DDR_HASH_MODE_DDR_HASH_MODE_TBL0_FIELD_SHIFT },
    2,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DDR_HASH_MODE_TBL1 *****/
const ru_field_rec NATC_DDR_CFG_DDR_HASH_MODE_DDR_HASH_MODE_TBL1_FIELD =
{
    "DDR_HASH_MODE_TBL1",
#if RU_INCLUDE_DESC
    "",
    "Hash algorithm used for DDR table 1 lookup.\nSee description of DDR_HASH_MODE_TBL0.\n",
#endif
    { NATC_DDR_CFG_DDR_HASH_MODE_DDR_HASH_MODE_TBL1_FIELD_MASK },
    0,
    { NATC_DDR_CFG_DDR_HASH_MODE_DDR_HASH_MODE_TBL1_FIELD_WIDTH },
    { NATC_DDR_CFG_DDR_HASH_MODE_DDR_HASH_MODE_TBL1_FIELD_SHIFT },
    2,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DDR_HASH_MODE_TBL2 *****/
const ru_field_rec NATC_DDR_CFG_DDR_HASH_MODE_DDR_HASH_MODE_TBL2_FIELD =
{
    "DDR_HASH_MODE_TBL2",
#if RU_INCLUDE_DESC
    "",
    "Hash algorithm used for DDR table 2 lookup.\nSee description of DDR_HASH_MODE_TBL0.\n",
#endif
    { NATC_DDR_CFG_DDR_HASH_MODE_DDR_HASH_MODE_TBL2_FIELD_MASK },
    0,
    { NATC_DDR_CFG_DDR_HASH_MODE_DDR_HASH_MODE_TBL2_FIELD_WIDTH },
    { NATC_DDR_CFG_DDR_HASH_MODE_DDR_HASH_MODE_TBL2_FIELD_SHIFT },
    2,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DDR_HASH_MODE_TBL3 *****/
const ru_field_rec NATC_DDR_CFG_DDR_HASH_MODE_DDR_HASH_MODE_TBL3_FIELD =
{
    "DDR_HASH_MODE_TBL3",
#if RU_INCLUDE_DESC
    "",
    "Hash algorithm used for DDR table 3 lookup.\nSee description of DDR_HASH_MODE_TBL0.\n",
#endif
    { NATC_DDR_CFG_DDR_HASH_MODE_DDR_HASH_MODE_TBL3_FIELD_MASK },
    0,
    { NATC_DDR_CFG_DDR_HASH_MODE_DDR_HASH_MODE_TBL3_FIELD_WIDTH },
    { NATC_DDR_CFG_DDR_HASH_MODE_DDR_HASH_MODE_TBL3_FIELD_SHIFT },
    2,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DDR_HASH_MODE_TBL4 *****/
const ru_field_rec NATC_DDR_CFG_DDR_HASH_MODE_DDR_HASH_MODE_TBL4_FIELD =
{
    "DDR_HASH_MODE_TBL4",
#if RU_INCLUDE_DESC
    "",
    "Hash algorithm used for DDR table 4 lookup.\nSee description of DDR_HASH_MODE_TBL0.\n",
#endif
    { NATC_DDR_CFG_DDR_HASH_MODE_DDR_HASH_MODE_TBL4_FIELD_MASK },
    0,
    { NATC_DDR_CFG_DDR_HASH_MODE_DDR_HASH_MODE_TBL4_FIELD_WIDTH },
    { NATC_DDR_CFG_DDR_HASH_MODE_DDR_HASH_MODE_TBL4_FIELD_SHIFT },
    2,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DDR_HASH_MODE_TBL5 *****/
const ru_field_rec NATC_DDR_CFG_DDR_HASH_MODE_DDR_HASH_MODE_TBL5_FIELD =
{
    "DDR_HASH_MODE_TBL5",
#if RU_INCLUDE_DESC
    "",
    "Hash algorithm used for DDR table 5 lookup.\nSee description of DDR_HASH_MODE_TBL0.\n",
#endif
    { NATC_DDR_CFG_DDR_HASH_MODE_DDR_HASH_MODE_TBL5_FIELD_MASK },
    0,
    { NATC_DDR_CFG_DDR_HASH_MODE_DDR_HASH_MODE_TBL5_FIELD_WIDTH },
    { NATC_DDR_CFG_DDR_HASH_MODE_DDR_HASH_MODE_TBL5_FIELD_SHIFT },
    2,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DDR_HASH_MODE_TBL6 *****/
const ru_field_rec NATC_DDR_CFG_DDR_HASH_MODE_DDR_HASH_MODE_TBL6_FIELD =
{
    "DDR_HASH_MODE_TBL6",
#if RU_INCLUDE_DESC
    "",
    "Hash algorithm used for DDR table 6 lookup.\nSee description of DDR_HASH_MODE_TBL0.\n",
#endif
    { NATC_DDR_CFG_DDR_HASH_MODE_DDR_HASH_MODE_TBL6_FIELD_MASK },
    0,
    { NATC_DDR_CFG_DDR_HASH_MODE_DDR_HASH_MODE_TBL6_FIELD_WIDTH },
    { NATC_DDR_CFG_DDR_HASH_MODE_DDR_HASH_MODE_TBL6_FIELD_SHIFT },
    2,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DDR_HASH_MODE_TBL7 *****/
const ru_field_rec NATC_DDR_CFG_DDR_HASH_MODE_DDR_HASH_MODE_TBL7_FIELD =
{
    "DDR_HASH_MODE_TBL7",
#if RU_INCLUDE_DESC
    "",
    "Hash algorithm used for DDR table 7 lookup.\nSee description of DDR_HASH_MODE_TBL0.\n",
#endif
    { NATC_DDR_CFG_DDR_HASH_MODE_DDR_HASH_MODE_TBL7_FIELD_MASK },
    0,
    { NATC_DDR_CFG_DDR_HASH_MODE_DDR_HASH_MODE_TBL7_FIELD_WIDTH },
    { NATC_DDR_CFG_DDR_HASH_MODE_DDR_HASH_MODE_TBL7_FIELD_SHIFT },
    2,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *NATC_DDR_CFG_DDR_HASH_MODE_FIELDS[] =
{
    &NATC_DDR_CFG_DDR_HASH_MODE_DDR_HASH_MODE_TBL0_FIELD,
    &NATC_DDR_CFG_DDR_HASH_MODE_DDR_HASH_MODE_TBL1_FIELD,
    &NATC_DDR_CFG_DDR_HASH_MODE_DDR_HASH_MODE_TBL2_FIELD,
    &NATC_DDR_CFG_DDR_HASH_MODE_DDR_HASH_MODE_TBL3_FIELD,
    &NATC_DDR_CFG_DDR_HASH_MODE_DDR_HASH_MODE_TBL4_FIELD,
    &NATC_DDR_CFG_DDR_HASH_MODE_DDR_HASH_MODE_TBL5_FIELD,
    &NATC_DDR_CFG_DDR_HASH_MODE_DDR_HASH_MODE_TBL6_FIELD,
    &NATC_DDR_CFG_DDR_HASH_MODE_DDR_HASH_MODE_TBL7_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: NATC_DDR_CFG_DDR_HASH_MODE *****/
const ru_reg_rec NATC_DDR_CFG_DDR_HASH_MODE_REG =
{
    "DDR_HASH_MODE",
#if RU_INCLUDE_DESC
    "NAT Cache DDR Hash Mode Register",
    "NAT Cache DDR Hash Mode Register\n",
#endif
    { NATC_DDR_CFG_DDR_HASH_MODE_REG_OFFSET },
    0,
    0,
    615,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    8,
    NATC_DDR_CFG_DDR_HASH_MODE_FIELDS,
#endif
};

unsigned long NATC_DDR_CFG_ADDRS[] =
{
    0x82950000,
};

static const ru_reg_rec *NATC_DDR_CFG_REGS[] =
{
    &NATC_DDR_CFG_DDR_SIZE_REG,
    &NATC_DDR_CFG_DDR_BINS_PER_BUCKET_0_REG,
    &NATC_DDR_CFG_DDR_BINS_PER_BUCKET_1_REG,
    &NATC_DDR_CFG_TOTAL_LEN_REG,
    &NATC_DDR_CFG_SM_STATUS_REG,
    &NATC_DDR_CFG_DDR_HASH_MODE_REG,
};

const ru_block_rec NATC_DDR_CFG_BLOCK =
{
    "NATC_DDR_CFG",
    NATC_DDR_CFG_ADDRS,
    1,
    6,
    NATC_DDR_CFG_REGS,
};
