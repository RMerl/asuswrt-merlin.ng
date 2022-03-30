/*
   <:copyright-BRCM:2014-2016:DUAL/GPL:standard
   
      Copyright (c) 2014-2016 Broadcom 
      All Rights Reserved
   
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License, version 2, as published by
   the Free Software Foundation (the "GPL").
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   
   A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
   writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
   
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
