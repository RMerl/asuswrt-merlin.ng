/*
   <:copyright-BRCM:2014-2016:DUAL/GPL:standard
   
      Copyright (c) 2014-2016 Broadcom 
      All Rights Reserved
   
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

#include "rdd.h"
#include "rdd_tunnels_parsing.h"

void rdd_tunnels_parsing_enable(const rdd_module_t *module, bdmf_boolean enable)
{
    GROUP_MWRITE_8(module->group, module->cfg_ptr, enable);
}

int rdd_tunnels_parsing_init(const rdd_module_t *module)
{
    RDD_TUNNELS_PARSING_CFG_DTS cfg_entry = {};

    cfg_entry.ds_lite_enable = ((tunnels_parsing_params_t *)module->params)->ds_lite_enable;
    cfg_entry.res_offset = module->res_offset;

    MWRITE_GROUP_BLOCK_32(module->group, module->cfg_ptr, (void *)&cfg_entry, sizeof(RDD_TUNNELS_PARSING_CFG_DTS));
    return BDMF_ERR_OK;
}
