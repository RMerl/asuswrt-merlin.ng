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



/* This is an automated file. Do not edit its contents. */


#include "rdd_ag_reas.h"

int rdd_ag_reas_g9991_reassembly_error_cntrs_table_packets_set(uint32_t _entry, uint32_t packets)
{
    if(_entry >= RDD_G9991_REASSEMBLY_ERROR_CNTRS_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_PACKETS_AND_BYTES_PACKETS_WRITE_G(packets, RDD_G9991_REASSEMBLY_ERROR_CNTRS_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_reas_g9991_reassembly_error_cntrs_table_packets_get(uint32_t _entry, uint32_t *packets)
{
    if(_entry >= RDD_G9991_REASSEMBLY_ERROR_CNTRS_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_PACKETS_AND_BYTES_PACKETS_READ_G(*packets, RDD_G9991_REASSEMBLY_ERROR_CNTRS_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_reas_g9991_reassembly_error_cntrs_table_bytes_set(uint32_t _entry, uint32_t bytes)
{
    if(_entry >= RDD_G9991_REASSEMBLY_ERROR_CNTRS_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_PACKETS_AND_BYTES_BYTES_WRITE_G(bytes, RDD_G9991_REASSEMBLY_ERROR_CNTRS_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_reas_g9991_reassembly_error_cntrs_table_bytes_get(uint32_t _entry, uint32_t *bytes)
{
    if(_entry >= RDD_G9991_REASSEMBLY_ERROR_CNTRS_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_PACKETS_AND_BYTES_BYTES_READ_G(*bytes, RDD_G9991_REASSEMBLY_ERROR_CNTRS_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

