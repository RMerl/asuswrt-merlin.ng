// SPDX-License-Identifier: GPL-2.0+
/*
   Copyright (c) 2015 Broadcom
   All Rights Reserved

*/

#ifndef _BCM4912_XPORT_TOP_AG_H_
#define _BCM4912_XPORT_TOP_AG_H_

#include "access_macros.h"
#include "bcmtypes.h"
int ag_drv_xport_top_ctrl_set(uint8_t xlmac_id, uint8_t p3_mode, uint8_t p2_mode, uint8_t p1_mode, uint8_t p0_mode);
int ag_drv_xport_top_ctrl_get(uint8_t xlmac_id, uint8_t *p3_mode, uint8_t *p2_mode, uint8_t *p1_mode, uint8_t *p0_mode);
int ag_drv_xport_top_status_get(uint8_t xlmac_id, uint8_t *link_status);
int ag_drv_xport_top_revision_get(uint8_t xlmac_id, uint32_t *xport_rev);
int ag_drv_xport_top_spare_cntrl_set(uint8_t xlmac_id, uint32_t spare_reg);
int ag_drv_xport_top_spare_cntrl_get(uint8_t xlmac_id, uint32_t *spare_reg);

#ifdef USE_BDMF_SHELL
bdmfmon_handle_t ag_drv_xport_top_cli_init(bdmfmon_handle_t driver_dir);
#endif


#endif

