/*
   <:copyright-BRCM:2015:DUAL/GPL:standard
   
      Copyright (c) 2015 Broadcom 
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

#if defined(_CFE_) || defined(__UBOOT__)

#include "rdd.h"
#include "rdd_defs.h"
#include "bdmf_data_types.h"
#include "rdp_common.h"

#define SLEEP_ADDR_TAG   0xffffffff
#define BREAK_TAG        0xfffffffe

extern void cfe_usleep(int);
#define xrdp_alloc(_a) KMALLOC(_a, 32)
#define xrdp_usleep(_a) cfe_usleep(_a)
#define BDMF_TRACE_ERR xprintf
#define bdmf_ioremap(_a, _b) _a

dpi_params_t *p_dpi_cfg;

uintptr_t rdp_runner_core_addr[NUM_OF_RUNNER_CORES];

static const access_log_tuple_t init_data[] = {
    #include "data_path_init_basic_data.h"
    { (ACCESS_LOG_OP_STOP << 24), 0 }
};

int data_path_init(dpi_params_t *dpi_params);
int data_path_init_basic(dpi_params_t *dpi_params)
{
    int rc = 0;

    p_dpi_cfg = dpi_params;

    printf("%s: Restore HW configuration\n", __FUNCTION__);
    rc = access_log_restore(init_data);
    printf("%s: Restore HW configuration done. rc=%d\n", __FUNCTION__, rc);

    return rc;
}

#endif
