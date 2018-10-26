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
 * Field: NATC_DDR_CFG_SIZE_RESERVED0
 ******************************************************************************/
const ru_field_rec NATC_DDR_CFG_SIZE_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    NATC_DDR_CFG_SIZE_RESERVED0_FIELD_MASK,
    0,
    NATC_DDR_CFG_SIZE_RESERVED0_FIELD_WIDTH,
    NATC_DDR_CFG_SIZE_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NATC_DDR_CFG_SIZE_DDR_SIZE_TBL7
 ******************************************************************************/
const ru_field_rec NATC_DDR_CFG_SIZE_DDR_SIZE_TBL7_FIELD =
{
    "DDR_SIZE_TBL7",
#if RU_INCLUDE_DESC
    "",
    "Number of entries in DDR table 7"
    "See description of DDR_SIZE_TBL0"
    "0=8k; 1=16k; 2=32k; 3=64k; 4=128k; 5=256k; 6=invalid; 7=invalid",
#endif
    NATC_DDR_CFG_SIZE_DDR_SIZE_TBL7_FIELD_MASK,
    0,
    NATC_DDR_CFG_SIZE_DDR_SIZE_TBL7_FIELD_WIDTH,
    NATC_DDR_CFG_SIZE_DDR_SIZE_TBL7_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NATC_DDR_CFG_SIZE_DDR_SIZE_TBL6
 ******************************************************************************/
const ru_field_rec NATC_DDR_CFG_SIZE_DDR_SIZE_TBL6_FIELD =
{
    "DDR_SIZE_TBL6",
#if RU_INCLUDE_DESC
    "",
    "Number of entries in DDR table 6"
    "See description of DDR_SIZE_TBL0"
    "0=8k; 1=16k; 2=32k; 3=64k; 4=128k; 5=256k; 6=invalid; 7=invalid",
#endif
    NATC_DDR_CFG_SIZE_DDR_SIZE_TBL6_FIELD_MASK,
    0,
    NATC_DDR_CFG_SIZE_DDR_SIZE_TBL6_FIELD_WIDTH,
    NATC_DDR_CFG_SIZE_DDR_SIZE_TBL6_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NATC_DDR_CFG_SIZE_DDR_SIZE_TBL5
 ******************************************************************************/
const ru_field_rec NATC_DDR_CFG_SIZE_DDR_SIZE_TBL5_FIELD =
{
    "DDR_SIZE_TBL5",
#if RU_INCLUDE_DESC
    "",
    "Number of entries in DDR table 5"
    "See description of DDR_SIZE_TBL0"
    "0=8k; 1=16k; 2=32k; 3=64k; 4=128k; 5=256k; 6=invalid; 7=invalid",
#endif
    NATC_DDR_CFG_SIZE_DDR_SIZE_TBL5_FIELD_MASK,
    0,
    NATC_DDR_CFG_SIZE_DDR_SIZE_TBL5_FIELD_WIDTH,
    NATC_DDR_CFG_SIZE_DDR_SIZE_TBL5_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NATC_DDR_CFG_SIZE_DDR_SIZE_TBL4
 ******************************************************************************/
const ru_field_rec NATC_DDR_CFG_SIZE_DDR_SIZE_TBL4_FIELD =
{
    "DDR_SIZE_TBL4",
#if RU_INCLUDE_DESC
    "",
    "Number of entries in DDR table 4"
    "See description of DDR_SIZE_TBL0"
    "0=8k; 1=16k; 2=32k; 3=64k; 4=128k; 5=256k; 6=invalid; 7=invalid",
#endif
    NATC_DDR_CFG_SIZE_DDR_SIZE_TBL4_FIELD_MASK,
    0,
    NATC_DDR_CFG_SIZE_DDR_SIZE_TBL4_FIELD_WIDTH,
    NATC_DDR_CFG_SIZE_DDR_SIZE_TBL4_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NATC_DDR_CFG_SIZE_DDR_SIZE_TBL3
 ******************************************************************************/
const ru_field_rec NATC_DDR_CFG_SIZE_DDR_SIZE_TBL3_FIELD =
{
    "DDR_SIZE_TBL3",
#if RU_INCLUDE_DESC
    "",
    "Number of entries in DDR table 3"
    "See description of DDR_SIZE_TBL0"
    "0=8k; 1=16k; 2=32k; 3=64k; 4=128k; 5=256k; 6=invalid; 7=invalid",
#endif
    NATC_DDR_CFG_SIZE_DDR_SIZE_TBL3_FIELD_MASK,
    0,
    NATC_DDR_CFG_SIZE_DDR_SIZE_TBL3_FIELD_WIDTH,
    NATC_DDR_CFG_SIZE_DDR_SIZE_TBL3_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NATC_DDR_CFG_SIZE_DDR_SIZE_TBL2
 ******************************************************************************/
const ru_field_rec NATC_DDR_CFG_SIZE_DDR_SIZE_TBL2_FIELD =
{
    "DDR_SIZE_TBL2",
#if RU_INCLUDE_DESC
    "",
    "Number of entries in DDR table 2"
    "See description of DDR_SIZE_TBL0"
    "0=8k; 1=16k; 2=32k; 3=64k; 4=128k; 5=256k; 6=invalid; 7=invalid",
#endif
    NATC_DDR_CFG_SIZE_DDR_SIZE_TBL2_FIELD_MASK,
    0,
    NATC_DDR_CFG_SIZE_DDR_SIZE_TBL2_FIELD_WIDTH,
    NATC_DDR_CFG_SIZE_DDR_SIZE_TBL2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NATC_DDR_CFG_SIZE_DDR_SIZE_TBL1
 ******************************************************************************/
const ru_field_rec NATC_DDR_CFG_SIZE_DDR_SIZE_TBL1_FIELD =
{
    "DDR_SIZE_TBL1",
#if RU_INCLUDE_DESC
    "",
    "Number of entries in DDR table 1"
    "0=8k; 1=16k; 2=32k; 3=64k; 4=128k; 5=256k; 6=invalid; 7=invalid"
    "See description of DDR_SIZE_TBL0",
#endif
    NATC_DDR_CFG_SIZE_DDR_SIZE_TBL1_FIELD_MASK,
    0,
    NATC_DDR_CFG_SIZE_DDR_SIZE_TBL1_FIELD_WIDTH,
    NATC_DDR_CFG_SIZE_DDR_SIZE_TBL1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NATC_DDR_CFG_SIZE_DDR_SIZE_TBL0
 ******************************************************************************/
const ru_field_rec NATC_DDR_CFG_SIZE_DDR_SIZE_TBL0_FIELD =
{
    "DDR_SIZE_TBL0",
#if RU_INCLUDE_DESC
    "",
    "Number of entries in DDR table 0"
    "Value of 6 or above is invalid"
    "To compute the actual size of the table, add DDR_BINS_PER_BUCKET field"
    "to the table size selection;"
    "For instance, if DDR_BINS_PER_BUCKET is 3 (4 bins per bucket)"
    "and DDR_size is 3 (64k entries), the actual size of the table in DDR is"
    "(64*1024+3) multiply by total length (TOTAL_LEN) of key and context in bytes"
    "Extra 3 entries are used to store collided entries of the last entry"
    "value 256k 5"
    "256k entries"
    "value 128k 4"
    "128k entries"
    "value 64k 3"
    "64k entries"
    "value 32k 2"
    "32k entries"
    "value 16k 1"
    "16k entries"
    "value 8k 0"
    "8k entries"
    "default 0h",
#endif
    NATC_DDR_CFG_SIZE_DDR_SIZE_TBL0_FIELD_MASK,
    0,
    NATC_DDR_CFG_SIZE_DDR_SIZE_TBL0_FIELD_WIDTH,
    NATC_DDR_CFG_SIZE_DDR_SIZE_TBL0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NATC_DDR_CFG_BINS_PER_BUCKET_0_DDR_BINS_PER_BUCKET_TBL3
 ******************************************************************************/
const ru_field_rec NATC_DDR_CFG_BINS_PER_BUCKET_0_DDR_BINS_PER_BUCKET_TBL3_FIELD =
{
    "DDR_BINS_PER_BUCKET_TBL3",
#if RU_INCLUDE_DESC
    "",
    "Number of entries per bucket in DDR table 3"
    "See description of DDR_BINS_PER_BUCKET_TBL0",
#endif
    NATC_DDR_CFG_BINS_PER_BUCKET_0_DDR_BINS_PER_BUCKET_TBL3_FIELD_MASK,
    0,
    NATC_DDR_CFG_BINS_PER_BUCKET_0_DDR_BINS_PER_BUCKET_TBL3_FIELD_WIDTH,
    NATC_DDR_CFG_BINS_PER_BUCKET_0_DDR_BINS_PER_BUCKET_TBL3_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NATC_DDR_CFG_BINS_PER_BUCKET_0_DDR_BINS_PER_BUCKET_TBL2
 ******************************************************************************/
const ru_field_rec NATC_DDR_CFG_BINS_PER_BUCKET_0_DDR_BINS_PER_BUCKET_TBL2_FIELD =
{
    "DDR_BINS_PER_BUCKET_TBL2",
#if RU_INCLUDE_DESC
    "",
    "Number of entries per bucket in DDR table 2"
    "See description of DDR_BINS_PER_BUCKET_TBL0",
#endif
    NATC_DDR_CFG_BINS_PER_BUCKET_0_DDR_BINS_PER_BUCKET_TBL2_FIELD_MASK,
    0,
    NATC_DDR_CFG_BINS_PER_BUCKET_0_DDR_BINS_PER_BUCKET_TBL2_FIELD_WIDTH,
    NATC_DDR_CFG_BINS_PER_BUCKET_0_DDR_BINS_PER_BUCKET_TBL2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NATC_DDR_CFG_BINS_PER_BUCKET_0_DDR_BINS_PER_BUCKET_TBL1
 ******************************************************************************/
const ru_field_rec NATC_DDR_CFG_BINS_PER_BUCKET_0_DDR_BINS_PER_BUCKET_TBL1_FIELD =
{
    "DDR_BINS_PER_BUCKET_TBL1",
#if RU_INCLUDE_DESC
    "",
    "Number of entries per bucket in DDR table 1"
    "See description of DDR_BINS_PER_BUCKET_TBL0",
#endif
    NATC_DDR_CFG_BINS_PER_BUCKET_0_DDR_BINS_PER_BUCKET_TBL1_FIELD_MASK,
    0,
    NATC_DDR_CFG_BINS_PER_BUCKET_0_DDR_BINS_PER_BUCKET_TBL1_FIELD_WIDTH,
    NATC_DDR_CFG_BINS_PER_BUCKET_0_DDR_BINS_PER_BUCKET_TBL1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NATC_DDR_CFG_BINS_PER_BUCKET_0_DDR_BINS_PER_BUCKET_TBL0
 ******************************************************************************/
const ru_field_rec NATC_DDR_CFG_BINS_PER_BUCKET_0_DDR_BINS_PER_BUCKET_TBL0_FIELD =
{
    "DDR_BINS_PER_BUCKET_TBL0",
#if RU_INCLUDE_DESC
    "",
    "Number of entries per bucket in DDR table 0"
    "This is limited by bus max burst size.  For instance, if"
    "UBUS supports max burst size of 128 bytes, key length is 16"
    "bytes, maximum DDR_BINS_PER_BUCKET that can be programmed"
    "is 128 bytes / 16-bytes (bytes per bin) = 8 entries"
    "0h: 1 entry"
    "1h: 2 entries"
    "2h: 3 entries"
    "3h: 4 entries"
    "4h: 5 entries"
    "5h: 6 entries"
    "6h: 7 entries"
    "7h: 8 entries"
    ".............",
#endif
    NATC_DDR_CFG_BINS_PER_BUCKET_0_DDR_BINS_PER_BUCKET_TBL0_FIELD_MASK,
    0,
    NATC_DDR_CFG_BINS_PER_BUCKET_0_DDR_BINS_PER_BUCKET_TBL0_FIELD_WIDTH,
    NATC_DDR_CFG_BINS_PER_BUCKET_0_DDR_BINS_PER_BUCKET_TBL0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NATC_DDR_CFG_BINS_PER_BUCKET_1_DDR_BINS_PER_BUCKET_TBL7
 ******************************************************************************/
const ru_field_rec NATC_DDR_CFG_BINS_PER_BUCKET_1_DDR_BINS_PER_BUCKET_TBL7_FIELD =
{
    "DDR_BINS_PER_BUCKET_TBL7",
#if RU_INCLUDE_DESC
    "",
    "Number of entries per bucket in DDR table 7"
    "See description of DDR_BINS_PER_BUCKET_TBL0",
#endif
    NATC_DDR_CFG_BINS_PER_BUCKET_1_DDR_BINS_PER_BUCKET_TBL7_FIELD_MASK,
    0,
    NATC_DDR_CFG_BINS_PER_BUCKET_1_DDR_BINS_PER_BUCKET_TBL7_FIELD_WIDTH,
    NATC_DDR_CFG_BINS_PER_BUCKET_1_DDR_BINS_PER_BUCKET_TBL7_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NATC_DDR_CFG_BINS_PER_BUCKET_1_DDR_BINS_PER_BUCKET_TBL6
 ******************************************************************************/
const ru_field_rec NATC_DDR_CFG_BINS_PER_BUCKET_1_DDR_BINS_PER_BUCKET_TBL6_FIELD =
{
    "DDR_BINS_PER_BUCKET_TBL6",
#if RU_INCLUDE_DESC
    "",
    "Number of entries per bucket in DDR table 6"
    "See description of DDR_BINS_PER_BUCKET_TBL0",
#endif
    NATC_DDR_CFG_BINS_PER_BUCKET_1_DDR_BINS_PER_BUCKET_TBL6_FIELD_MASK,
    0,
    NATC_DDR_CFG_BINS_PER_BUCKET_1_DDR_BINS_PER_BUCKET_TBL6_FIELD_WIDTH,
    NATC_DDR_CFG_BINS_PER_BUCKET_1_DDR_BINS_PER_BUCKET_TBL6_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NATC_DDR_CFG_BINS_PER_BUCKET_1_DDR_BINS_PER_BUCKET_TBL5
 ******************************************************************************/
const ru_field_rec NATC_DDR_CFG_BINS_PER_BUCKET_1_DDR_BINS_PER_BUCKET_TBL5_FIELD =
{
    "DDR_BINS_PER_BUCKET_TBL5",
#if RU_INCLUDE_DESC
    "",
    "Number of entries per bucket in DDR table 5"
    "See description of DDR_BINS_PER_BUCKET_TBL0",
#endif
    NATC_DDR_CFG_BINS_PER_BUCKET_1_DDR_BINS_PER_BUCKET_TBL5_FIELD_MASK,
    0,
    NATC_DDR_CFG_BINS_PER_BUCKET_1_DDR_BINS_PER_BUCKET_TBL5_FIELD_WIDTH,
    NATC_DDR_CFG_BINS_PER_BUCKET_1_DDR_BINS_PER_BUCKET_TBL5_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NATC_DDR_CFG_BINS_PER_BUCKET_1_DDR_BINS_PER_BUCKET_TBL4
 ******************************************************************************/
const ru_field_rec NATC_DDR_CFG_BINS_PER_BUCKET_1_DDR_BINS_PER_BUCKET_TBL4_FIELD =
{
    "DDR_BINS_PER_BUCKET_TBL4",
#if RU_INCLUDE_DESC
    "",
    "Number of entries per bucket in DDR table 4"
    "See description of DDR_BINS_PER_BUCKET_TBL0",
#endif
    NATC_DDR_CFG_BINS_PER_BUCKET_1_DDR_BINS_PER_BUCKET_TBL4_FIELD_MASK,
    0,
    NATC_DDR_CFG_BINS_PER_BUCKET_1_DDR_BINS_PER_BUCKET_TBL4_FIELD_WIDTH,
    NATC_DDR_CFG_BINS_PER_BUCKET_1_DDR_BINS_PER_BUCKET_TBL4_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NATC_DDR_CFG_TOTAL_LEN_RESERVED0
 ******************************************************************************/
const ru_field_rec NATC_DDR_CFG_TOTAL_LEN_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    NATC_DDR_CFG_TOTAL_LEN_RESERVED0_FIELD_MASK,
    0,
    NATC_DDR_CFG_TOTAL_LEN_RESERVED0_FIELD_WIDTH,
    NATC_DDR_CFG_TOTAL_LEN_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NATC_DDR_CFG_TOTAL_LEN_TOTAL_LEN_TBL7
 ******************************************************************************/
const ru_field_rec NATC_DDR_CFG_TOTAL_LEN_TOTAL_LEN_TBL7_FIELD =
{
    "TOTAL_LEN_TBL7",
#if RU_INCLUDE_DESC
    "",
    "Length of the lookup key plus context (including 8-byte counters) in DDR table 7"
    "See description of TOTAL_LEN_TBL0.",
#endif
    NATC_DDR_CFG_TOTAL_LEN_TOTAL_LEN_TBL7_FIELD_MASK,
    0,
    NATC_DDR_CFG_TOTAL_LEN_TOTAL_LEN_TBL7_FIELD_WIDTH,
    NATC_DDR_CFG_TOTAL_LEN_TOTAL_LEN_TBL7_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NATC_DDR_CFG_TOTAL_LEN_TOTAL_LEN_TBL6
 ******************************************************************************/
const ru_field_rec NATC_DDR_CFG_TOTAL_LEN_TOTAL_LEN_TBL6_FIELD =
{
    "TOTAL_LEN_TBL6",
#if RU_INCLUDE_DESC
    "",
    "Length of the lookup key plus context (including 8-byte counters) in DDR table 6"
    "See description of TOTAL_LEN_TBL0.",
#endif
    NATC_DDR_CFG_TOTAL_LEN_TOTAL_LEN_TBL6_FIELD_MASK,
    0,
    NATC_DDR_CFG_TOTAL_LEN_TOTAL_LEN_TBL6_FIELD_WIDTH,
    NATC_DDR_CFG_TOTAL_LEN_TOTAL_LEN_TBL6_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NATC_DDR_CFG_TOTAL_LEN_TOTAL_LEN_TBL5
 ******************************************************************************/
const ru_field_rec NATC_DDR_CFG_TOTAL_LEN_TOTAL_LEN_TBL5_FIELD =
{
    "TOTAL_LEN_TBL5",
#if RU_INCLUDE_DESC
    "",
    "Length of the lookup key plus context (including 8-byte counters) in DDR table 5"
    "See description of TOTAL_LEN_TBL0.",
#endif
    NATC_DDR_CFG_TOTAL_LEN_TOTAL_LEN_TBL5_FIELD_MASK,
    0,
    NATC_DDR_CFG_TOTAL_LEN_TOTAL_LEN_TBL5_FIELD_WIDTH,
    NATC_DDR_CFG_TOTAL_LEN_TOTAL_LEN_TBL5_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NATC_DDR_CFG_TOTAL_LEN_TOTAL_LEN_TBL4
 ******************************************************************************/
const ru_field_rec NATC_DDR_CFG_TOTAL_LEN_TOTAL_LEN_TBL4_FIELD =
{
    "TOTAL_LEN_TBL4",
#if RU_INCLUDE_DESC
    "",
    "Length of the lookup key plus context (including 8-byte counters) in DDR table 4"
    "See description of TOTAL_LEN_TBL0.",
#endif
    NATC_DDR_CFG_TOTAL_LEN_TOTAL_LEN_TBL4_FIELD_MASK,
    0,
    NATC_DDR_CFG_TOTAL_LEN_TOTAL_LEN_TBL4_FIELD_WIDTH,
    NATC_DDR_CFG_TOTAL_LEN_TOTAL_LEN_TBL4_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NATC_DDR_CFG_TOTAL_LEN_TOTAL_LEN_TBL3
 ******************************************************************************/
const ru_field_rec NATC_DDR_CFG_TOTAL_LEN_TOTAL_LEN_TBL3_FIELD =
{
    "TOTAL_LEN_TBL3",
#if RU_INCLUDE_DESC
    "",
    "Length of the lookup key plus context (including 8-byte counters) in DDR table 3"
    "See description of TOTAL_LEN_TBL0.",
#endif
    NATC_DDR_CFG_TOTAL_LEN_TOTAL_LEN_TBL3_FIELD_MASK,
    0,
    NATC_DDR_CFG_TOTAL_LEN_TOTAL_LEN_TBL3_FIELD_WIDTH,
    NATC_DDR_CFG_TOTAL_LEN_TOTAL_LEN_TBL3_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NATC_DDR_CFG_TOTAL_LEN_TOTAL_LEN_TBL2
 ******************************************************************************/
const ru_field_rec NATC_DDR_CFG_TOTAL_LEN_TOTAL_LEN_TBL2_FIELD =
{
    "TOTAL_LEN_TBL2",
#if RU_INCLUDE_DESC
    "",
    "Length of the lookup key plus context (including 8-byte counters) in DDR table 2"
    "See description of TOTAL_LEN_TBL0.",
#endif
    NATC_DDR_CFG_TOTAL_LEN_TOTAL_LEN_TBL2_FIELD_MASK,
    0,
    NATC_DDR_CFG_TOTAL_LEN_TOTAL_LEN_TBL2_FIELD_WIDTH,
    NATC_DDR_CFG_TOTAL_LEN_TOTAL_LEN_TBL2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NATC_DDR_CFG_TOTAL_LEN_TOTAL_LEN_TBL1
 ******************************************************************************/
const ru_field_rec NATC_DDR_CFG_TOTAL_LEN_TOTAL_LEN_TBL1_FIELD =
{
    "TOTAL_LEN_TBL1",
#if RU_INCLUDE_DESC
    "",
    "Length of the lookup key plus context (including 8-byte counters) in DDR table 1"
    "See description of TOTAL_LEN_TBL0.",
#endif
    NATC_DDR_CFG_TOTAL_LEN_TOTAL_LEN_TBL1_FIELD_MASK,
    0,
    NATC_DDR_CFG_TOTAL_LEN_TOTAL_LEN_TBL1_FIELD_WIDTH,
    NATC_DDR_CFG_TOTAL_LEN_TOTAL_LEN_TBL1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NATC_DDR_CFG_TOTAL_LEN_TOTAL_LEN_TBL0
 ******************************************************************************/
const ru_field_rec NATC_DDR_CFG_TOTAL_LEN_TOTAL_LEN_TBL0_FIELD =
{
    "TOTAL_LEN_TBL0",
#if RU_INCLUDE_DESC
    "",
    "Length of the lookup key plus context (including 8-byte counters) in DDR table 0"
    "The context length (including 8-byte counters) is calculated by TOTAL_LEN minus KEY_LEN"
    "The maximum value should not exceed hardware capability."
    "For instance, most projects have max of 80-bytes and BCM63158 has max value of 144-byte."
    "0h: 48-byte"
    "1h: 64-byte"
    "2h: 80-byte"
    "3h: 96-byte"
    "4h: 112-byte"
    "5h: 128-byte"
    "6h: 144-byte"
    "7h: 160-byte",
#endif
    NATC_DDR_CFG_TOTAL_LEN_TOTAL_LEN_TBL0_FIELD_MASK,
    0,
    NATC_DDR_CFG_TOTAL_LEN_TOTAL_LEN_TBL0_FIELD_WIDTH,
    NATC_DDR_CFG_TOTAL_LEN_TOTAL_LEN_TBL0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NATC_DDR_CFG_SM_STATUS_RESERVED0
 ******************************************************************************/
const ru_field_rec NATC_DDR_CFG_SM_STATUS_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    NATC_DDR_CFG_SM_STATUS_RESERVED0_FIELD_MASK,
    0,
    NATC_DDR_CFG_SM_STATUS_RESERVED0_FIELD_WIDTH,
    NATC_DDR_CFG_SM_STATUS_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NATC_DDR_CFG_SM_STATUS_DEBUG_SEL
 ******************************************************************************/
const ru_field_rec NATC_DDR_CFG_SM_STATUS_DEBUG_SEL_FIELD =
{
    "DEBUG_SEL",
#if RU_INCLUDE_DESC
    "",
    "Debug bus select."
    "2'b00: prb_nat_control."
    "2'b01: prb_cmd_control."
    "2'b10: prb_wb_control."
    "2'b11: prb_ddr_control.",
#endif
    NATC_DDR_CFG_SM_STATUS_DEBUG_SEL_FIELD_MASK,
    0,
    NATC_DDR_CFG_SM_STATUS_DEBUG_SEL_FIELD_WIDTH,
    NATC_DDR_CFG_SM_STATUS_DEBUG_SEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NATC_DDR_CFG_SM_STATUS_APB_STATE
 ******************************************************************************/
const ru_field_rec NATC_DDR_CFG_SM_STATUS_APB_STATE_FIELD =
{
    "APB_STATE",
#if RU_INCLUDE_DESC
    "",
    "APB to RBUS bridge state machine."
    "2'b00: APB_ST_IDLE."
    "2'b01: APB_ST_RW."
    "2'b10: AOB_ST_END.",
#endif
    NATC_DDR_CFG_SM_STATUS_APB_STATE_FIELD_MASK,
    0,
    NATC_DDR_CFG_SM_STATUS_APB_STATE_FIELD_WIDTH,
    NATC_DDR_CFG_SM_STATUS_APB_STATE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NATC_DDR_CFG_SM_STATUS_DDR_REQ_STATE
 ******************************************************************************/
const ru_field_rec NATC_DDR_CFG_SM_STATUS_DDR_REQ_STATE_FIELD =
{
    "DDR_REQ_STATE",
#if RU_INCLUDE_DESC
    "",
    "DDR request state machine."
    "2'b00: DDR_REQ_ST_IDLE."
    "2'b01: DDR_REQ_ST_WRITE_HEADER."
    "2'b10: DDR_REQ_ST_WRITE_HEADER_DELAY.",
#endif
    NATC_DDR_CFG_SM_STATUS_DDR_REQ_STATE_FIELD_MASK,
    0,
    NATC_DDR_CFG_SM_STATUS_DDR_REQ_STATE_FIELD_WIDTH,
    NATC_DDR_CFG_SM_STATUS_DDR_REQ_STATE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NATC_DDR_CFG_SM_STATUS_DDR_REP_STATE
 ******************************************************************************/
const ru_field_rec NATC_DDR_CFG_SM_STATUS_DDR_REP_STATE_FIELD =
{
    "DDR_REP_STATE",
#if RU_INCLUDE_DESC
    "",
    "DDR reply state machine."
    "3'b000: DDR_REP_ST_IDLE."
    "3'b001: DDR_REP_ST_READ_DATA."
    "3'b010: DDR_REP_ST_READ_RESULT."
    "3'b011: DDR_REP_ST_READ_WAIT."
    "3'b100: DDR_REP_ST_EVICT_WR_NON_CACHEABLE.",
#endif
    NATC_DDR_CFG_SM_STATUS_DDR_REP_STATE_FIELD_MASK,
    0,
    NATC_DDR_CFG_SM_STATUS_DDR_REP_STATE_FIELD_WIDTH,
    NATC_DDR_CFG_SM_STATUS_DDR_REP_STATE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NATC_DDR_CFG_SM_STATUS_RUNNER_CMD_STATE
 ******************************************************************************/
const ru_field_rec NATC_DDR_CFG_SM_STATUS_RUNNER_CMD_STATE_FIELD =
{
    "RUNNER_CMD_STATE",
#if RU_INCLUDE_DESC
    "",
    "Runner command state machine."
    "1'b0: RUNNER_CMD_ST_IDLE."
    "1'b1: RUNNER_CMD_ST_WRITE_RUNNER_FIFO.",
#endif
    NATC_DDR_CFG_SM_STATUS_RUNNER_CMD_STATE_FIELD_MASK,
    0,
    NATC_DDR_CFG_SM_STATUS_RUNNER_CMD_STATE_FIELD_WIDTH,
    NATC_DDR_CFG_SM_STATUS_RUNNER_CMD_STATE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NATC_DDR_CFG_SM_STATUS_WB_STATE
 ******************************************************************************/
const ru_field_rec NATC_DDR_CFG_SM_STATUS_WB_STATE_FIELD =
{
    "WB_STATE",
#if RU_INCLUDE_DESC
    "",
    "Write-back state machine."
    "1'b0: WB_ST_IDLE."
    "1'b1: WB_ST_WRITE_BACIF.",
#endif
    NATC_DDR_CFG_SM_STATUS_WB_STATE_FIELD_MASK,
    0,
    NATC_DDR_CFG_SM_STATUS_WB_STATE_FIELD_WIDTH,
    NATC_DDR_CFG_SM_STATUS_WB_STATE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NATC_DDR_CFG_SM_STATUS_NAT_STATE
 ******************************************************************************/
const ru_field_rec NATC_DDR_CFG_SM_STATUS_NAT_STATE_FIELD =
{
    "NAT_STATE",
#if RU_INCLUDE_DESC
    "",
    "Nat state machine."
    "15'b000000000000000: NAT_ST_IDLE."
    "15'b000000000000001: NAT_ST_IDLE_WRITE_SMEM."
    "15'b000000000000010: NAT_ST_IDLE_DDR_PENDING."
    "15'b000000000000100: NAT_ST_HASH."
    "15'b000000000001000: NAT_ST_NAT_MEM_READ_REQ."
    "15'b000000000010000: NAT_ST_NAT_MEM_WRITE_REQ."
    "15'b000000000100000: NAT_ST_READ_SMEM."
    "15'b000000001000000: NAT_ST_UPDATE_DDR."
    "15'b000000010000000: NAT_ST_IDLE_BLOCKING_PENDING."
    "15'b000000100000000: NAT_ST_EVICT_WAIT."
    "15'b000001000000000: NAT_ST_CHECK_NON_CACHEABLE."
    "15'b000010000000000: NAT_ST_WAIT."
    "15'b000100000000000: NAT_ST_WAIT_NATC_MEM_REQ_DONE."
    "15'b001000000000000: NAT_ST_CACHE_FLUSH."
    "15'b010000000000000: NAT_ST_DDR_MISS_0."
    "15'b100000000000000: NAT_ST_DDR_MISS_1.",
#endif
    NATC_DDR_CFG_SM_STATUS_NAT_STATE_FIELD_MASK,
    0,
    NATC_DDR_CFG_SM_STATUS_NAT_STATE_FIELD_WIDTH,
    NATC_DDR_CFG_SM_STATUS_NAT_STATE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

#endif /* RU_INCLUDE_FIELD_DB */

/******************************************************************************
 * Register: NATC_DDR_CFG_SIZE
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *NATC_DDR_CFG_SIZE_FIELDS[] =
{
    &NATC_DDR_CFG_SIZE_RESERVED0_FIELD,
    &NATC_DDR_CFG_SIZE_DDR_SIZE_TBL7_FIELD,
    &NATC_DDR_CFG_SIZE_DDR_SIZE_TBL6_FIELD,
    &NATC_DDR_CFG_SIZE_DDR_SIZE_TBL5_FIELD,
    &NATC_DDR_CFG_SIZE_DDR_SIZE_TBL4_FIELD,
    &NATC_DDR_CFG_SIZE_DDR_SIZE_TBL3_FIELD,
    &NATC_DDR_CFG_SIZE_DDR_SIZE_TBL2_FIELD,
    &NATC_DDR_CFG_SIZE_DDR_SIZE_TBL1_FIELD,
    &NATC_DDR_CFG_SIZE_DDR_SIZE_TBL0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec NATC_DDR_CFG_SIZE_REG = 
{
    "SIZE",
#if RU_INCLUDE_DESC
    "NAT Cache DDR Size Register",
    "DDR Size Register",
#endif
    NATC_DDR_CFG_SIZE_REG_OFFSET,
    0,
    0,
    1110,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    9,
    NATC_DDR_CFG_SIZE_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: NATC_DDR_CFG_BINS_PER_BUCKET_0
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *NATC_DDR_CFG_BINS_PER_BUCKET_0_FIELDS[] =
{
    &NATC_DDR_CFG_BINS_PER_BUCKET_0_DDR_BINS_PER_BUCKET_TBL3_FIELD,
    &NATC_DDR_CFG_BINS_PER_BUCKET_0_DDR_BINS_PER_BUCKET_TBL2_FIELD,
    &NATC_DDR_CFG_BINS_PER_BUCKET_0_DDR_BINS_PER_BUCKET_TBL1_FIELD,
    &NATC_DDR_CFG_BINS_PER_BUCKET_0_DDR_BINS_PER_BUCKET_TBL0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec NATC_DDR_CFG_BINS_PER_BUCKET_0_REG = 
{
    "BINS_PER_BUCKET_0",
#if RU_INCLUDE_DESC
    "NAT Cache DDR Bins Per Bucket 0 register",
    "DDR Bins Per Bucket Register 0",
#endif
    NATC_DDR_CFG_BINS_PER_BUCKET_0_REG_OFFSET,
    0,
    0,
    1111,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    NATC_DDR_CFG_BINS_PER_BUCKET_0_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: NATC_DDR_CFG_BINS_PER_BUCKET_1
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *NATC_DDR_CFG_BINS_PER_BUCKET_1_FIELDS[] =
{
    &NATC_DDR_CFG_BINS_PER_BUCKET_1_DDR_BINS_PER_BUCKET_TBL7_FIELD,
    &NATC_DDR_CFG_BINS_PER_BUCKET_1_DDR_BINS_PER_BUCKET_TBL6_FIELD,
    &NATC_DDR_CFG_BINS_PER_BUCKET_1_DDR_BINS_PER_BUCKET_TBL5_FIELD,
    &NATC_DDR_CFG_BINS_PER_BUCKET_1_DDR_BINS_PER_BUCKET_TBL4_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec NATC_DDR_CFG_BINS_PER_BUCKET_1_REG = 
{
    "BINS_PER_BUCKET_1",
#if RU_INCLUDE_DESC
    "NAT Cache DDR Bins Per Bucket 1 Register",
    "DDR Bins Per Bucket Register 1",
#endif
    NATC_DDR_CFG_BINS_PER_BUCKET_1_REG_OFFSET,
    0,
    0,
    1112,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    NATC_DDR_CFG_BINS_PER_BUCKET_1_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: NATC_DDR_CFG_TOTAL_LEN
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *NATC_DDR_CFG_TOTAL_LEN_FIELDS[] =
{
    &NATC_DDR_CFG_TOTAL_LEN_RESERVED0_FIELD,
    &NATC_DDR_CFG_TOTAL_LEN_TOTAL_LEN_TBL7_FIELD,
    &NATC_DDR_CFG_TOTAL_LEN_TOTAL_LEN_TBL6_FIELD,
    &NATC_DDR_CFG_TOTAL_LEN_TOTAL_LEN_TBL5_FIELD,
    &NATC_DDR_CFG_TOTAL_LEN_TOTAL_LEN_TBL4_FIELD,
    &NATC_DDR_CFG_TOTAL_LEN_TOTAL_LEN_TBL3_FIELD,
    &NATC_DDR_CFG_TOTAL_LEN_TOTAL_LEN_TBL2_FIELD,
    &NATC_DDR_CFG_TOTAL_LEN_TOTAL_LEN_TBL1_FIELD,
    &NATC_DDR_CFG_TOTAL_LEN_TOTAL_LEN_TBL0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec NATC_DDR_CFG_TOTAL_LEN_REG = 
{
    "TOTAL_LEN",
#if RU_INCLUDE_DESC
    "NAT Cache Total Length Register",
    "DDR TABLE Total Length Register",
#endif
    NATC_DDR_CFG_TOTAL_LEN_REG_OFFSET,
    0,
    0,
    1113,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    9,
    NATC_DDR_CFG_TOTAL_LEN_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: NATC_DDR_CFG_SM_STATUS
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *NATC_DDR_CFG_SM_STATUS_FIELDS[] =
{
    &NATC_DDR_CFG_SM_STATUS_RESERVED0_FIELD,
    &NATC_DDR_CFG_SM_STATUS_DEBUG_SEL_FIELD,
    &NATC_DDR_CFG_SM_STATUS_APB_STATE_FIELD,
    &NATC_DDR_CFG_SM_STATUS_DDR_REQ_STATE_FIELD,
    &NATC_DDR_CFG_SM_STATUS_DDR_REP_STATE_FIELD,
    &NATC_DDR_CFG_SM_STATUS_RUNNER_CMD_STATE_FIELD,
    &NATC_DDR_CFG_SM_STATUS_WB_STATE_FIELD,
    &NATC_DDR_CFG_SM_STATUS_NAT_STATE_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec NATC_DDR_CFG_SM_STATUS_REG = 
{
    "SM_STATUS",
#if RU_INCLUDE_DESC
    "NAT State Machine Status Register",
    "NAT State Machine Status Register",
#endif
    NATC_DDR_CFG_SM_STATUS_REG_OFFSET,
    0,
    0,
    1114,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    8,
    NATC_DDR_CFG_SM_STATUS_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Block: NATC_DDR_CFG
 ******************************************************************************/
static const ru_reg_rec *NATC_DDR_CFG_REGS[] =
{
    &NATC_DDR_CFG_SIZE_REG,
    &NATC_DDR_CFG_BINS_PER_BUCKET_0_REG,
    &NATC_DDR_CFG_BINS_PER_BUCKET_1_REG,
    &NATC_DDR_CFG_TOTAL_LEN_REG,
    &NATC_DDR_CFG_SM_STATUS_REG,
};

unsigned long NATC_DDR_CFG_ADDRS[] =
{
    0x82e503d0,
};

const ru_block_rec NATC_DDR_CFG_BLOCK = 
{
    "NATC_DDR_CFG",
    NATC_DDR_CFG_ADDRS,
    1,
    5,
    NATC_DDR_CFG_REGS
};

/* End of file XRDP_NATC_DDR_CFG.c */
