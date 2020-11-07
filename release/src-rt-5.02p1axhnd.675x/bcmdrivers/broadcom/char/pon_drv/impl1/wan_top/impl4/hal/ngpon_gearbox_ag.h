/*
   Copyright (c) 2015 Broadcom Corporation
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

#ifndef _NGPON_GEARBOX_AG_H_
#define _NGPON_GEARBOX_AG_H_

#include "access_macros.h"
#include "bdmf_interface.h"
#ifdef USE_BDMF_SHELL
#include "bdmf_shell.h"
#endif
#include "rdp_common.h"

typedef struct
{
    uint8_t cfngpongboxrxfifordptr;
    uint8_t cfngpongboxrxptrautolddis;
    uint8_t cfngpongboxrxmaxbadk;
    uint8_t cfngpongboxrxfrmk28only;
    uint8_t cfngpongboxrxmaxgoodk;
    uint8_t cfngpongboxrxfrchunt;
    uint8_t cfngpongboxrxoutdataflip;
    uint8_t cfngpongboxrxfrcmuxsel;
    uint8_t cfngpongboxrxfrcmuxval;
    uint8_t cfngpongboxrx20bdataflip;
    uint8_t cfngpongboxrxserdataflip;
    uint8_t cfngpongboxrxserdatainv;
    uint8_t cfngpongboxrxfifoptrld;
    uint8_t cfngpongboxrxswsynchold;
    uint8_t cfngpongboxrxmode;
    uint8_t cfngpongboxrxen;
    uint8_t cfngpongboxrstn;
} ngpon_gearbox_rx_ctl_0;

typedef struct
{
    uint8_t cfngpongboxtxfifodatardptr;
    uint8_t cfngpongboxtxfifovldoff;
    uint8_t cfngpongboxtxservldflip;
    uint8_t cfngpongboxtxserdataflip;
    uint8_t cfngpongboxtxservldinv;
    uint8_t cfngpongboxtxserdatainv;
    uint8_t cfngpongboxtxfifovldptrld;
    uint8_t cfngpongboxtxfifoptrld;
    uint8_t cfngpongboxtxen;
} ngpon_gearbox_tx_ctl;

typedef struct
{
    uint8_t ngpontxgboxfifovldptrcol;
    uint8_t ngponrxgboxstate;
    uint8_t ngpontxgboxfifodataptrcol;
    uint8_t ngponrxgboxkcnt;
    uint8_t ngponrxgboxfifoptrdelta;
    uint8_t ngponrxgboxsyncacq;
    uint8_t ngponrxgboxfifoptrcol;
    uint16_t ngponrxgboxcodeerrcntstat;
} ngpon_gearbox_status;

int ag_drv_ngpon_gearbox_rx_ctl_0_set(const ngpon_gearbox_rx_ctl_0 *rx_ctl_0);
int ag_drv_ngpon_gearbox_rx_ctl_0_get(ngpon_gearbox_rx_ctl_0 *rx_ctl_0);
int ag_drv_ngpon_gearbox_rx_ctl_1_set(uint32_t cfngpongboxrxmaxtimercnt);
int ag_drv_ngpon_gearbox_rx_ctl_1_get(uint32_t *cfngpongboxrxmaxtimercnt);
int ag_drv_ngpon_gearbox_rx_ctl_2_set(uint16_t cfngpongboxrxk28d5rdp, uint16_t cfngpongboxrxk28d5rdn);
int ag_drv_ngpon_gearbox_rx_ctl_2_get(uint16_t *cfngpongboxrxk28d5rdp, uint16_t *cfngpongboxrxk28d5rdn);
int ag_drv_ngpon_gearbox_rx_ctl_3_set(uint16_t cfngpongboxrxd5d7rdp, uint16_t cfngpongboxrxd5d7rdn);
int ag_drv_ngpon_gearbox_rx_ctl_3_get(uint16_t *cfngpongboxrxd5d7rdp, uint16_t *cfngpongboxrxd5d7rdn);
int ag_drv_ngpon_gearbox_tx_ctl_set(const ngpon_gearbox_tx_ctl *tx_ctl);
int ag_drv_ngpon_gearbox_tx_ctl_get(ngpon_gearbox_tx_ctl *tx_ctl);
int ag_drv_ngpon_gearbox_status_set(const ngpon_gearbox_status *status);
int ag_drv_ngpon_gearbox_status_get(ngpon_gearbox_status *status);

#ifdef USE_BDMF_SHELL
bdmfmon_handle_t ag_drv_ngpon_gearbox_cli_init(bdmfmon_handle_t driver_dir);
#endif


#endif

