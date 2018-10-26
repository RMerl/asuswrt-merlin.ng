/*
   <:copyright-BRCM:2015:DUAL/GPL:standard

      Copyright (c) 2015 Broadcom
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

#ifdef _CFE_

#include "rdd.h"
#include "rdd_defs.h"

#include "data_path_init_basic.h"

#define SLEEP_ADDR_TAG   0xffffffff
#define BREAK_TAG        0xfffffffe

extern void cfe_usleep(int);
#define xrdp_alloc(_a) KMALLOC(_a, 32)
#define xrdp_usleep(_a) cfe_usleep(_a)
#define BDMF_TRACE_ERR xprintf
#define bdmf_ioremap(_a, _b) _a

dpi_params_t *p_dpi_cfg;

uintptr_t rdp_runner_core_addr[GROUPED_EN_SEGMENTS_NUM];

static const access_log_tuple_t init_data[] = {
    #include "data_path_init_basic_data.h"
    { ACCESS_LOG_OP_STOP, 0, 0, 0 }
};

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
