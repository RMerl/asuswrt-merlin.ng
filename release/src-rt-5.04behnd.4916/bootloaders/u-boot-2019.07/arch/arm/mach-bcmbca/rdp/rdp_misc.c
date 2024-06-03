/* SPDX-License-Identifier: GPL-2.0+
 *
 *  Copyright 2019 Broadcom Ltd.
 */

#include "hwapi_mac.h"
#include "rdp_drv_bbh.h"
#include "rdp_ubus.h"

int rdp_post_init(void)
{
    /* Ethernet WAN */
    mac_hwapi_init_emac(DRV_BBH_EMAC_0);
    mac_hwapi_set_unimac_cfg(DRV_BBH_EMAC_0,1);
    mac_hwapi_set_rxtx_enable(DRV_BBH_EMAC_0,0,0);/* Ethernet WAN EMAC will be enabled when the WAN service gets created */
    mac_hwapi_set_tx_max_frame_len(DRV_BBH_EMAC_0, 2048); /* Why do we set the max frame len here 'hard-coded' ??? FIXME */

    /* SF2 */
    mac_hwapi_init_emac(DRV_BBH_EMAC_1);
    mac_hwapi_set_unimac_cfg(DRV_BBH_EMAC_1,1);
    mac_hwapi_set_rxtx_enable(DRV_BBH_EMAC_1,1,1);
    mac_hwapi_set_tx_max_frame_len(DRV_BBH_EMAC_1, 2048); /* Why do we set the max frame len here 'hard-coded' ??? FIXME */
    mac_hwapi_set_backpressure_ext(DRV_BBH_EMAC_1, 1); /* Enable backpressure towards SF2 */

    return 0;
}

void rdp_enable_ubus_masters(void)
{
    UBUS_UBUS_MASTER_BRDG_REG_EN ubus_en;
    UBUS_UBUS_MASTER_BRDG_REG_REQ_CNTRL ubus_req_ctl;
    UBUS_UBUS_MASTER_BRDG_REG_HP ubus_hp;

    /* Configuration taken from simulation registers. */

    /*first Ubus Master*/
    READ_32(UBUS_MASTER_1_RDP_UBUS_MASTER_BRDG_REG_REQ_CNTRL,ubus_req_ctl);
    ubus_req_ctl.max_pkt_len = 0x90;
    ubus_req_ctl.endian_mode = UBUS_UBUS_MASTER_BRDG_REG_REQ_CNTRL_ENDIAN_MODE_LB_VALUE;
    WRITE_32(UBUS_MASTER_1_RDP_UBUS_MASTER_BRDG_REG_REQ_CNTRL,ubus_req_ctl);

    READ_32(UBUS_MASTER_1_RDP_UBUS_MASTER_BRDG_REG_HP,ubus_hp);
    ubus_hp.hp_en = 1;
    ubus_hp.hp_cnt_high = 1;
    ubus_hp.hp_cnt_total = 2;
    WRITE_32(UBUS_MASTER_1_RDP_UBUS_MASTER_BRDG_REG_HP,ubus_hp);

    READ_32(UBUS_MASTER_1_RDP_UBUS_MASTER_BRDG_REG_EN,ubus_en);
    ubus_en.en = 1;
    WRITE_32(UBUS_MASTER_1_RDP_UBUS_MASTER_BRDG_REG_EN,ubus_en);

    /*second Ubus Master*/
    READ_32(UBUS_MASTER_2_RDP_UBUS_MASTER_BRDG_REG_REQ_CNTRL,ubus_req_ctl);
    ubus_req_ctl.max_pkt_len = 0x90;
    ubus_req_ctl.endian_mode = UBUS_UBUS_MASTER_BRDG_REG_REQ_CNTRL_ENDIAN_MODE_LB_VALUE;
    WRITE_32(UBUS_MASTER_2_RDP_UBUS_MASTER_BRDG_REG_REQ_CNTRL,ubus_req_ctl);

    READ_32(UBUS_MASTER_2_RDP_UBUS_MASTER_BRDG_REG_HP,ubus_hp);
    ubus_hp.hp_en = 1;
    ubus_hp.hp_cnt_high = 11;
    ubus_hp.hp_cnt_total = 13;
    WRITE_32(UBUS_MASTER_2_RDP_UBUS_MASTER_BRDG_REG_HP,ubus_hp);

    READ_32(UBUS_MASTER_2_RDP_UBUS_MASTER_BRDG_REG_EN,ubus_en);
    ubus_en.en = 1;
    WRITE_32(UBUS_MASTER_2_RDP_UBUS_MASTER_BRDG_REG_EN,ubus_en);

    /*third Ubus Master*/
    READ_32(UBUS_MASTER_3_RDP_UBUS_MASTER_BRDG_REG_REQ_CNTRL,ubus_req_ctl);
    ubus_req_ctl.max_pkt_len = 0x90;
    ubus_req_ctl.endian_mode = UBUS_UBUS_MASTER_BRDG_REG_REQ_CNTRL_ENDIAN_MODE_LB_VALUE;
    WRITE_32(UBUS_MASTER_3_RDP_UBUS_MASTER_BRDG_REG_REQ_CNTRL,ubus_req_ctl);

    READ_32(UBUS_MASTER_3_RDP_UBUS_MASTER_BRDG_REG_HP,ubus_hp);
    ubus_hp.hp_en = 1;
    ubus_hp.hp_comb = 1;
    ubus_hp.hp_cnt_high = 1;
    ubus_hp.hp_cnt_total = 6;
    WRITE_32(UBUS_MASTER_3_RDP_UBUS_MASTER_BRDG_REG_HP,ubus_hp);

    READ_32(UBUS_MASTER_3_RDP_UBUS_MASTER_BRDG_REG_EN,ubus_en);
    ubus_en.en = 1;
    WRITE_32(UBUS_MASTER_3_RDP_UBUS_MASTER_BRDG_REG_EN,ubus_en);

    // UBUS arbitration configuration, done through clocks and reset (via PMC)
}
