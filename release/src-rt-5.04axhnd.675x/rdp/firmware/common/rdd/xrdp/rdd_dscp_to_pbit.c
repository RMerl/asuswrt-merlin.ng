/*
 * <:copyright-BRCM:2019:DUAL/GPL:standard
 * 
 *    Copyright (c) 2019 Broadcom 
 *    All Rights Reserved
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 2, as published by
 * the Free Software Foundation (the "GPL").
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * 
 * A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
 * writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 * 
 * :>
 */


#include "rdp_subsystem_common.h"
#include "rdd_dscp_to_pbit.h"
#include "rdp_common.h"
#include "XRDP_AG.h"
#include "fw_binary_auto.h"
#include "rdd_runner_proj_defs.h"
#include "rdd_map_auto.h"
#include "rdd_data_structures_auto.h"
#include "rdd_platform.h"

int rdp_dscp_to_pbit_init(void)
{
    int i, j;

    for (i = 0; i < RDD_DSCP_TO_PBITS_MAP_TABLE_SIZE; i++)
    {
        for (j = 0; j < 64; j++)
            GROUP_MWRITE_8(RDD_DSCP_TO_PBITS_MAP_TABLE_ADDRESS_ARR, i*64 + j, 0);
    }

#ifdef OPERATION_MODE_PRV
    for (i = 0; i < RDD_VPORT_TO_DSCP_TO_PBITS_TABLE_SIZE; i++)
    {
        GROUP_MWRITE_8(RDD_VPORT_TO_DSCP_TO_PBITS_TABLE_ADDRESS_ARR, i, 0);
    }
#endif

    return BDMF_ERR_OK;
}

