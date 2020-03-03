/*
 * <:copyright-BRCM:2019:DUAL/GPL:standard
 * 
 *    Copyright (c) 2019 Broadcom 
 *    All Rights Reserved
 * 
 * Unless you and Broadcom execute a separate written software license
 * agreement governing use of this software, this software is licensed
 * to you under the terms of the GNU General Public License version 2
 * (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
 * with the following added to such license:
 * 
 *    As a special exception, the copyright holders of this software give
 *    you permission to link this software with independent modules, and
 *    to copy and distribute the resulting executable under terms of your
 *    choice, provided that you also meet, for each linked independent
 *    module, the terms and conditions of the license of that module.
 *    An independent module is a module which is not derived from this
 *    software.  The special exception does not apply to any modifications
 *    of the software.
 * 
 * Not withstanding the above, under no circumstances may you combine
 * this software in any way with any other Broadcom software provided
 * under a license other than the GPL, without Broadcom's express prior
 * written consent.
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

    for (i = 0; i < RDD_VPORT_TO_DSCP_TO_PBITS_TABLE_SIZE; i++)
    {
        GROUP_MWRITE_8(RDD_VPORT_TO_DSCP_TO_PBITS_TABLE_ADDRESS_ARR, i, 0);
    }

    return BDMF_ERR_OK;
}

