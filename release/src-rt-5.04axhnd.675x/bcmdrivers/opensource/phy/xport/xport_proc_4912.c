/*
   Copyright (c) 2017 Broadcom Corporation
   All Rights Reserved

   <:label-BRCM:2017:DUAL/GPL:standard

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
/*
 * xport_proc_4912.c
 *
 */
void xport_reset_p0_ctrl_dump(uint8_t xlmac_id)
{
    uint8_t port_sw_reset;
    ag_drv_xport_portreset_p0_ctrl_get(xlmac_id, &port_sw_reset);
    pr_info("\n");
    pr_info("XPORT_PORTRESET_%d_P0_CTRL                          = \n", xlmac_id);
    pr_info("port_sw_reset                                      = 0x%02x\n",port_sw_reset);
}

void xport_reset_p1_ctrl_dump(uint8_t xlmac_id)
{
    uint8_t port_sw_reset;
    ag_drv_xport_portreset_p1_ctrl_get(xlmac_id, &port_sw_reset);
    pr_info("\n");
    pr_info("XPORT_PORTRESET_%d_P1_CTRL                          = \n", xlmac_id);
    pr_info("port_sw_reset                                      = 0x%02x\n",port_sw_reset);
}

void xport_reset_p2_ctrl_dump(uint8_t xlmac_id)
{
    uint8_t port_sw_reset;
    ag_drv_xport_portreset_p2_ctrl_get(xlmac_id, &port_sw_reset);
    pr_info("\n");
    pr_info("XPORT_PORTRESET_%d_P2_CTRL                          = \n", xlmac_id);
    pr_info("port_sw_reset                                      = 0x%02x\n",port_sw_reset);
}

void xport_reset_p3_ctrl_dump(uint8_t xlmac_id)
{
    uint8_t port_sw_reset;
    ag_drv_xport_portreset_p3_ctrl_get(xlmac_id, &port_sw_reset);
    pr_info("\n");
    pr_info("XPORT_PORTRESET_%d_P3_CTRL                          = \n", xlmac_id);
    pr_info("port_sw_reset                                      = 0x%02x\n",port_sw_reset);
}

void xport_reset_config_dump(uint8_t xlmac_id)
{
    uint8_t link_down_rst_en, enable_sm_run;
    uint16_t tick_timer_ndiv;
    ag_drv_xport_portreset_config_get(xlmac_id, &link_down_rst_en, &enable_sm_run, &tick_timer_ndiv);
    pr_info("\n");
    pr_info("XPORT_PORTRESET_%d_CONFIG                           = \n", xlmac_id);
    pr_info("link_down_rst_en                                   = 0x%02x\n",link_down_rst_en);
    pr_info("enable_sm_run                                      = 0x%02x\n",enable_sm_run);
    pr_info("tick_timer_ndiv                                    = 0x%02x\n",tick_timer_ndiv);
}

void xport_reset_p0_link_debounce_dump(uint8_t xlmac_id)
{
    uint8_t disable;
    uint16_t debounce_time;
    ag_drv_xport_portreset_p0_link_stat_debounce_cfg_get(xlmac_id, &disable, &debounce_time);
    pr_info("\n");
    pr_info("XPORT_PORTRESET_%d_P0_LINK_STAT_DEBOUNCE_CFG        = \n", xlmac_id);
    pr_info("disable                                            = 0x%02x\n",disable);
    pr_info("debounce_time                                      = 0x%02x\n",debounce_time);
}

void xport_reset_p1_link_debounce_dump(uint8_t xlmac_id)
{
    uint8_t disable;
    uint16_t debounce_time;
    ag_drv_xport_portreset_p1_link_stat_debounce_cfg_get(xlmac_id, &disable, &debounce_time);
    pr_info("\n");
    pr_info("XPORT_PORTRESET_%d_P1_LINK_STAT_DEBOUNCE_CFG        = \n", xlmac_id);
    pr_info("disable                                            = 0x%02x\n",disable);
    pr_info("debounce_time                                      = 0x%02x\n",debounce_time);
}
void xport_reset_p2_link_debounce_dump(uint8_t xlmac_id)
{
    uint8_t disable;
    uint16_t debounce_time;
    ag_drv_xport_portreset_p2_link_stat_debounce_cfg_get(xlmac_id, &disable, &debounce_time);
    pr_info("\n");
    pr_info("XPORT_PORTRESET_%d_P2_LINK_STAT_DEBOUNCE_CFG        = \n", xlmac_id);
    pr_info("disable                                            = 0x%02x\n",disable);
    pr_info("debounce_time                                      = 0x%02x\n",debounce_time);
}
void xport_reset_p3_link_debounce_dump(uint8_t xlmac_id)
{
    uint8_t disable;
    uint16_t debounce_time;
    ag_drv_xport_portreset_p3_link_stat_debounce_cfg_get(xlmac_id, &disable, &debounce_time);
    pr_info("\n");
    pr_info("XPORT_PORTRESET_%d_P3_LINK_STAT_DEBOUNCE_CFG        = \n", xlmac_id);
    pr_info("disable                                            = 0x%02x\n",disable);
    pr_info("debounce_time                                      = 0x%02x\n",debounce_time);
}

void xport_reset_p0_sig_en_dump(uint8_t xlmac_id)
{
    xport_portreset_sig_en sig_en;
    ag_drv_xport_portreset_p0_sig_en_get(xlmac_id, &sig_en);
    pr_info("\n");
    pr_info("XPORT_PORTRESET_%d_P0_SIG_EN                        = \n", xlmac_id);
    pr_info("enable_xlmac_rx_disab                              = 0x%02x\n",sig_en.enable_xlmac_rx_disab);
    pr_info("enable_xlmac_tx_disab                              = 0x%02x\n",sig_en.enable_xlmac_tx_disab);
    pr_info("enable_xlmac_tx_discard                            = 0x%02x\n",sig_en.enable_xlmac_tx_discard);
    pr_info("enable_xlmac_soft_reset                            = 0x%02x\n",sig_en.enable_xlmac_soft_reset);
    pr_info("enable_mab_rx_port_init                            = 0x%02x\n",sig_en.enable_mab_rx_port_init);
    pr_info("enable_mab_tx_port_init                            = 0x%02x\n",sig_en.enable_mab_tx_port_init);
    pr_info("enable_mab_tx_credit_disab                         = 0x%02x\n",sig_en.enable_mab_tx_credit_disab);
    pr_info("enable_mab_tx_fifo_init                            = 0x%02x\n",sig_en.enable_mab_tx_fifo_init);
    pr_info("enable_port_is_under_reset                         = 0x%02x\n",sig_en.enable_port_is_under_reset);
    pr_info("enable_xlmac_ep_discard                            = 0x%02x\n",sig_en.enable_xlmac_ep_discard);
}

void xport_reset_p1_sig_en_dump(uint8_t xlmac_id)
{
    xport_portreset_sig_en sig_en;
    ag_drv_xport_portreset_p1_sig_en_get(xlmac_id, &sig_en);
    pr_info("\n");
    pr_info("XPORT_PORTRESET_%d_P1_SIG_EN                        = \n", xlmac_id);
    pr_info("enable_xlmac_rx_disab                              = 0x%02x\n",sig_en.enable_xlmac_rx_disab);
    pr_info("enable_xlmac_tx_disab                              = 0x%02x\n",sig_en.enable_xlmac_tx_disab);
    pr_info("enable_xlmac_tx_discard                            = 0x%02x\n",sig_en.enable_xlmac_tx_discard);
    pr_info("enable_xlmac_soft_reset                            = 0x%02x\n",sig_en.enable_xlmac_soft_reset);
    pr_info("enable_mab_rx_port_init                            = 0x%02x\n",sig_en.enable_mab_rx_port_init);
    pr_info("enable_mab_tx_port_init                            = 0x%02x\n",sig_en.enable_mab_tx_port_init);
    pr_info("enable_mab_tx_credit_disab                         = 0x%02x\n",sig_en.enable_mab_tx_credit_disab);
    pr_info("enable_mab_tx_fifo_init                            = 0x%02x\n",sig_en.enable_mab_tx_fifo_init);
    pr_info("enable_port_is_under_reset                         = 0x%02x\n",sig_en.enable_port_is_under_reset);
    pr_info("enable_xlmac_ep_discard                            = 0x%02x\n",sig_en.enable_xlmac_ep_discard);
}

void xport_reset_p2_sig_en_dump(uint8_t xlmac_id)
{
    xport_portreset_sig_en sig_en;
    ag_drv_xport_portreset_p2_sig_en_get(xlmac_id, &sig_en);
    pr_info("\n");
    pr_info("XPORT_PORTRESET_%d_P2_SIG_EN                        = \n", xlmac_id);
    pr_info("enable_xlmac_rx_disab                              = 0x%02x\n",sig_en.enable_xlmac_rx_disab);
    pr_info("enable_xlmac_tx_disab                              = 0x%02x\n",sig_en.enable_xlmac_tx_disab);
    pr_info("enable_xlmac_tx_discard                            = 0x%02x\n",sig_en.enable_xlmac_tx_discard);
    pr_info("enable_xlmac_soft_reset                            = 0x%02x\n",sig_en.enable_xlmac_soft_reset);
    pr_info("enable_mab_rx_port_init                            = 0x%02x\n",sig_en.enable_mab_rx_port_init);
    pr_info("enable_mab_tx_port_init                            = 0x%02x\n",sig_en.enable_mab_tx_port_init);
    pr_info("enable_mab_tx_credit_disab                         = 0x%02x\n",sig_en.enable_mab_tx_credit_disab);
    pr_info("enable_mab_tx_fifo_init                            = 0x%02x\n",sig_en.enable_mab_tx_fifo_init);
    pr_info("enable_port_is_under_reset                         = 0x%02x\n",sig_en.enable_port_is_under_reset);
    pr_info("enable_xlmac_ep_discard                            = 0x%02x\n",sig_en.enable_xlmac_ep_discard);
}

void xport_reset_p3_sig_en_dump(uint8_t xlmac_id)
{
    xport_portreset_sig_en sig_en;
    ag_drv_xport_portreset_p3_sig_en_get(xlmac_id, &sig_en);
    pr_info("\n");
    pr_info("XPORT_PORTRESET_%d_P3_SIG_EN                        = \n", xlmac_id);
    pr_info("enable_xlmac_rx_disab                              = 0x%02x\n",sig_en.enable_xlmac_rx_disab);
    pr_info("enable_xlmac_tx_disab                              = 0x%02x\n",sig_en.enable_xlmac_tx_disab);
    pr_info("enable_xlmac_tx_discard                            = 0x%02x\n",sig_en.enable_xlmac_tx_discard);
    pr_info("enable_xlmac_soft_reset                            = 0x%02x\n",sig_en.enable_xlmac_soft_reset);
    pr_info("enable_mab_rx_port_init                            = 0x%02x\n",sig_en.enable_mab_rx_port_init);
    pr_info("enable_mab_tx_port_init                            = 0x%02x\n",sig_en.enable_mab_tx_port_init);
    pr_info("enable_mab_tx_credit_disab                         = 0x%02x\n",sig_en.enable_mab_tx_credit_disab);
    pr_info("enable_mab_tx_fifo_init                            = 0x%02x\n",sig_en.enable_mab_tx_fifo_init);
    pr_info("enable_port_is_under_reset                         = 0x%02x\n",sig_en.enable_port_is_under_reset);
    pr_info("enable_xlmac_ep_discard                            = 0x%02x\n",sig_en.enable_xlmac_ep_discard);
}

reg_name_func1 port_reset_name_func_array[] = {
    {"P0_CTRL", xport_reset_p0_ctrl_dump },
    {"P1_CTRL", xport_reset_p1_ctrl_dump },
    {"P2_CTRL", xport_reset_p2_ctrl_dump },
    {"P3_CTRL", xport_reset_p3_ctrl_dump },
    {"CONFIG", xport_reset_config_dump },
    {"P0_LNK_DEBOUNCE", xport_reset_p0_link_debounce_dump },
    {"P1_LNK_DEBOUNCE", xport_reset_p1_link_debounce_dump },
    {"P2_LNK_DEBOUNCE", xport_reset_p2_link_debounce_dump },
    {"P3_LNK_DEBOUNCE", xport_reset_p3_link_debounce_dump },
    {"P0_SIG_EN", xport_reset_p0_sig_en_dump },
    {"P1_SIG_EN", xport_reset_p1_sig_en_dump },
    {"P2_SIG_EN", xport_reset_p2_sig_en_dump },
    {"P3_SIG_EN", xport_reset_p3_sig_en_dump },
    { "", NULL},
};

static void xport_proc_cmd_port_reset_dump(uint8_t xlmac_id, char *reg_str)
{
    reg_name_func1 *p_array = &port_reset_name_func_array[0];
    pr_info("\n**** XPORT_PORTRESET_%d block\n", xlmac_id);
    while(p_array->name[0] != '\0')
    {
        if (!reg_str || strstr(p_array->name, reg_str)) p_array->func1(xlmac_id);
        p_array++;
    }
}

void xport_mab_ctrl_dump(uint8_t xlmac_id)
{
    uint8_t tx_credit_disab, tx_fifo_rst, tx_port_rst, rx_port_rst;
    ag_drv_xport_mab_ctrl_get(xlmac_id, &tx_credit_disab, &tx_fifo_rst, &tx_port_rst, &rx_port_rst);
    pr_info("\n");
    pr_info("XPORT_MAB_%d_CNTRL                                  = \n", xlmac_id);
    pr_info("tx_credit_disab                                    = 0x%02x\n",tx_credit_disab);
    pr_info("tx_fifo_rst                                        = 0x%02x\n",tx_fifo_rst);
    pr_info("tx_port_rst                                        = 0x%02x\n",tx_port_rst);
    pr_info("rx_port_rst                                        = 0x%02x\n",rx_port_rst);
}
void xport_mab_tx_wrr_ctrl_dump(uint8_t xlmac_id)
{
    xport_mab_tx_wrr_ctrl tx_wrr_ctrl;
    ag_drv_xport_mab_tx_wrr_ctrl_get(xlmac_id, &tx_wrr_ctrl);
    pr_info("\n");
    pr_info("XPORT_MAB_%d_TX_WRR_CTRL                            = \n", xlmac_id);
    pr_info("arb_mode                                           = 0x%02x\n",tx_wrr_ctrl.arb_mode);
    pr_info("p3_weight                                          = 0x%02x\n",tx_wrr_ctrl.p3_weight);
    pr_info("p2_weight                                          = 0x%02x\n",tx_wrr_ctrl.p2_weight);
    pr_info("p1_weight                                          = 0x%02x\n",tx_wrr_ctrl.p1_weight);
    pr_info("p0_weight                                          = 0x%02x\n",tx_wrr_ctrl.p0_weight);

}
void xport_mab_tx_threshold_dump(uint8_t xlmac_id)
{
    xport_mab_tx_threshold tx_threshold;
    ag_drv_xport_mab_tx_threshold_get(xlmac_id, &tx_threshold);
    pr_info("\n");
    pr_info("XPORT_MAB_%d_TX_THRESHOLD                           = \n", xlmac_id);
    pr_info("xgmii0_tx_threshold                                = 0x%02x\n",tx_threshold.xgmii0_tx_threshold);
    pr_info("gmii3_tx_threshold                                 = 0x%02x\n",tx_threshold.gmii3_tx_threshold);
    pr_info("gmii2_tx_threshold                                 = 0x%02x\n",tx_threshold.gmii2_tx_threshold);
    pr_info("gmii1_tx_threshold                                 = 0x%02x\n",tx_threshold.gmii1_tx_threshold);
    pr_info("gmii0_tx_threshold                                 = 0x%02x\n",tx_threshold.gmii0_tx_threshold);

}
void xport_mab_status_dump(uint8_t xlmac_id)
{
    uint8_t tx_frm_underrun_vect, tx_outstanding_credits_cnt_underrun_vect, tx_fifo_overrun_vect, rx_fifo_overrun_vect;
    ag_drv_xport_mab_status_get(xlmac_id, &tx_frm_underrun_vect, &tx_outstanding_credits_cnt_underrun_vect, &tx_fifo_overrun_vect, &rx_fifo_overrun_vect);
    pr_info("\n");
    pr_info("XPORT_MAB_%d_STATUS                                 = \n", xlmac_id);
    pr_info("tx_frm_underrun_vect                               = 0x%02x\n",tx_frm_underrun_vect);
    pr_info("tx_outstanding_credits_cnt_underrun_vect           = 0x%02x\n",tx_outstanding_credits_cnt_underrun_vect);
    pr_info("tx_fifo_overrun_vect                               = 0x%02x\n",tx_fifo_overrun_vect);
    pr_info("rx_fifo_overrun_vect                               = 0x%02x\n",rx_fifo_overrun_vect);
}

reg_name_func1 mab_reg_name_func_array[] = {
    {"CNTRL", xport_mab_ctrl_dump },        
    {"TX_WRR_CTRL", xport_mab_tx_wrr_ctrl_dump},
    {"TX_THRESHOLD", xport_mab_tx_threshold_dump}, 
    {"STATUS", xport_mab_status_dump},
    { "", NULL},
};

static void xport_proc_cmd_mab_reg_dump(uint8_t xlmac_id, char *reg_str)
{
    reg_name_func1 *p_array = &mab_reg_name_func_array[0];
    pr_info("\n**** XPORT_MAB_%d block\n", xlmac_id);
    while(p_array->name[0] != '\0')
    {
        if (!reg_str || strstr(p_array->name, reg_str)) p_array->func1(xlmac_id);
        p_array++;
    }
}

void xlmac_reg_dir_acc_data_write_dump(uint8_t xlmac_id)
{
}
void xlmac_reg_dir_acc_data_read_dump(uint8_t xlmac_id)
{
}
void xlmac_reg_indir_acc_addr_0_dump(uint8_t xlmac_id)
{
}
void xlmac_reg_indir_acc_data_low_0_dump(uint8_t xlmac_id)
{
}
void xlmac_reg_indir_acc_data_high_0_dump(uint8_t xlmac_id)
{
}
void xlmac_reg_indir_acc_addr_1_dump(uint8_t xlmac_id)
{
}
void xlmac_reg_indir_acc_data_low_1_dump(uint8_t xlmac_id)
{
}
void xlmac_reg_indir_acc_data_high_1_dump(uint8_t xlmac_id)
{
}
void xlmac_reg_config_dump(uint8_t xlmac_id)
{
    xport_xlmac_reg_config config;
    ag_drv_xport_xlmac_reg_config_get(xlmac_id, &config);
    pr_info("\n");
    pr_info("XLMAC_REG_%d_CONFIG                                   = \n", xlmac_id);
    pr_info("xlmac_reset                                        = 0x%02x\n",config.xlmac_reset);
    pr_info("rx_dual_cycle_tdm_en                               = 0x%02x\n",config.rx_dual_cycle_tdm_en);
    pr_info("rx_non_linear_quad_tdm_en                          = 0x%02x\n",config.rx_non_linear_quad_tdm_en);
    pr_info("rx_flex_tdm_enable                                 = 0x%02x\n",config.rx_flex_tdm_enable);
    pr_info("mac_mode                                           = 0x%02x\n",config.mac_mode);
    pr_info("osts_timer_disable                                 = 0x%02x\n",config.osts_timer_disable);
    pr_info("bypass_osts                                        = 0x%02x\n",config.bypass_osts);
    pr_info("egr_1588_timestamping_mode                         = 0x%02x\n",config.egr_1588_timestamping_mode);
}
void xlmac_reg_interrupt_check_dump(uint8_t xlmac_id)
{
    uint8_t xlmac_intr_check;
    ag_drv_xport_xlmac_reg_interrupt_check_get(xlmac_id, &xlmac_intr_check);
    pr_info("\n");
    pr_info("XLMAC_REG_%d_INTERRUPT_CHECK                        = \n", xlmac_id);
    pr_info("xlmac_intr_check                                   = 0x%02x\n",xlmac_intr_check);
}
void xlmac_reg_port_0_rxerr_mask_dump(uint8_t xlmac_id)
{
    uint32_t rsv_err_mask;
    ag_drv_xport_xlmac_reg_port_0_rxerr_mask_get(xlmac_id, &rsv_err_mask);
    pr_info("\n");
    pr_info("XLMAC_REG_%d_PORT_0_RXERR_MASK                      = \n", xlmac_id);
    pr_info("rsv_err_mask                                       = 0x%02x\n",rsv_err_mask);
}
void xlmac_reg_port_1_rxerr_mask_dump(uint8_t xlmac_id)
{
    uint32_t rsv_err_mask;
    ag_drv_xport_xlmac_reg_port_1_rxerr_mask_get(xlmac_id, &rsv_err_mask);
    pr_info("\n");
    pr_info("XLMAC_REG_%d_PORT_1_RXERR_MASK                      = \n", xlmac_id);
    pr_info("rsv_err_mask                                       = 0x%02x\n",rsv_err_mask);
}
void xlmac_reg_port_2_rxerr_mask_dump(uint8_t xlmac_id)
{
    uint32_t rsv_err_mask;
    ag_drv_xport_xlmac_reg_port_2_rxerr_mask_get(xlmac_id, &rsv_err_mask);
    pr_info("\n");
    pr_info("XLMAC_REG_%d_PORT_2_RXERR_MASK                      = \n", xlmac_id);
    pr_info("rsv_err_mask                                       = 0x%02x\n",rsv_err_mask);
}
void xlmac_reg_port_3_rxerr_mask_dump(uint8_t xlmac_id)
{
    uint32_t rsv_err_mask;
    ag_drv_xport_xlmac_reg_port_3_rxerr_mask_get(xlmac_id, &rsv_err_mask);
    pr_info("\n");
    pr_info("XLMAC_REG_%d_PORT_3_RXERR_MASK                      = \n", xlmac_id);
    pr_info("rsv_err_mask                                       = 0x%02x\n",rsv_err_mask);
}
void xlmac_reg_rmt_lpbk_cntrl_dump(uint8_t xlmac_id)
{
    xport_xlmac_reg_rmt_lpbk_cntrl rmt_lpbk_cntrl;
    ag_drv_xport_xlmac_reg_rmt_lpbk_cntrl_get(xlmac_id, &rmt_lpbk_cntrl);
    pr_info("\n");
    pr_info("XLMAC_REG_%d_RMT_LPBK_CNTRL                         = \n", xlmac_id);
    pr_info("read_threshold                                     = 0x%02x\n",rmt_lpbk_cntrl.read_threshold);
    pr_info("tx_port_id                                         = 0x%02x\n",rmt_lpbk_cntrl.tx_port_id);
    pr_info("tx_port_sel                                        = 0x%02x\n",rmt_lpbk_cntrl.tx_port_sel);
    pr_info("rxerr_en                                           = 0x%02x\n",rmt_lpbk_cntrl.rxerr_en);
    pr_info("tx_crc_err                                         = 0x%02x\n",rmt_lpbk_cntrl.tx_crc_err);
    pr_info("tx_crc_mode                                        = 0x%02x\n",rmt_lpbk_cntrl.tx_crc_mode);
    pr_info("rmt_loopback_en                                    = 0x%02x\n",rmt_lpbk_cntrl.rmt_loopback_en);

}

reg_name_func1 xlmac_reg_name_func_array[] = {
    { "DIR_ACC_DATA_WRITE", xlmac_reg_dir_acc_data_write_dump },
    { "DIR_ACC_DATA_READ", xlmac_reg_dir_acc_data_read_dump },
    { "INDIR_ACC_ADDR_0", xlmac_reg_indir_acc_addr_0_dump },
    { "INDIR_ACC_DATA_LOW_0", xlmac_reg_indir_acc_data_low_0_dump },
    { "INDIR_ACC_DATA_HIGH_0", xlmac_reg_indir_acc_data_high_0_dump },
    { "INDIR_ACC_ADDR_1", xlmac_reg_indir_acc_addr_1_dump },
    { "INDIR_ACC_DATA_LOW_1", xlmac_reg_indir_acc_data_low_1_dump },
    { "INDIR_ACC_DATA_HIGH_1", xlmac_reg_indir_acc_data_high_1_dump },
    { "CONFIG", xlmac_reg_config_dump },
    { "INTERRUPT_CHECK", xlmac_reg_interrupt_check_dump },
    { "PORT_0_RXERR_MASK", xlmac_reg_port_0_rxerr_mask_dump },
    { "PORT_1_RXERR_MASK", xlmac_reg_port_1_rxerr_mask_dump },
    { "PORT_2_RXERR_MASK", xlmac_reg_port_2_rxerr_mask_dump },
    { "PORT_3_RXERR_MASK", xlmac_reg_port_3_rxerr_mask_dump },
    { "RMT_LPBK_CNTRL", xlmac_reg_rmt_lpbk_cntrl_dump },
    { "", NULL},
};

static void xport_proc_cmd_xlmac_reg_dump(uint8_t xlmac_id, char *reg_str)
{
    reg_name_func1 *p_array = &xlmac_reg_name_func_array[0];
    pr_info("\n**** XLMAC_REG_%d block\n", xlmac_id);
    while(p_array->name[0] != '\0')
    {
        if (!reg_str || strstr(p_array->name, reg_str)) p_array->func1(xlmac_id);
        p_array++;
    }
}

void mib_reg_cntrl_dump(uint8_t xlmac_id)
{
    uint8_t eee_cnt_mode; uint8_t saturate_en; uint8_t cor_en; uint8_t cnt_rst;
    ag_drv_xport_mib_reg_control_get(xlmac_id, &eee_cnt_mode,&saturate_en,&cor_en,&cnt_rst);
    pr_info("\n");
    pr_info("MIB_REG_%d_CNTRL                                    = \n", xlmac_id);
    pr_info("eee_cnt_mode                                       = 0x%02x\n",eee_cnt_mode);
    pr_info("saturate_en                                        = 0x%02x\n",saturate_en);
    pr_info("cor_en                                             = 0x%02x\n",cor_en);
    pr_info("cnt_rst                                            = 0x%02x\n",cnt_rst);
}
void mib_reg_eee_pulse_duration_cntrl_dump(uint8_t xlmac_id)
{
    uint8_t cnt;
    ag_drv_xport_mib_reg_eee_pulse_duration_cntrl_get(xlmac_id, &cnt);
    pr_info("\n");
    pr_info("MIB_REG_%d_EEE_PULSE_DURATION_CNTRL                 = \n", xlmac_id);
    pr_info("cnt                                                = 0x%02x\n",cnt);
}
void mib_reg_gport0_max_pkt_size_dump(uint8_t xlmac_id)
{
    uint16_t max_pkt_size;
    ag_drv_xport_mib_reg_gport0_max_pkt_size_get(xlmac_id, &max_pkt_size);
    pr_info("\n");
    pr_info("MIB_REG_%d_GPORT0_MAX_PKT_SIZE                      = \n", xlmac_id);
    pr_info("max_pkt_size                                       = 0x%04x\n",max_pkt_size);
}
void mib_reg_gport1_max_pkt_size_dump(uint8_t xlmac_id)
{
    uint16_t max_pkt_size;
    ag_drv_xport_mib_reg_gport1_max_pkt_size_get(xlmac_id, &max_pkt_size);
    pr_info("\n");
    pr_info("MIB_REG_%d_GPORT1_MAX_PKT_SIZE                      = \n", xlmac_id);
    pr_info("max_pkt_size                                       = 0x%04x\n",max_pkt_size);
}
void mib_reg_gport2_max_pkt_size_dump(uint8_t xlmac_id)
{
    uint16_t max_pkt_size;
    ag_drv_xport_mib_reg_gport2_max_pkt_size_get(xlmac_id, &max_pkt_size);
    pr_info("\n");
    pr_info("MIB_REG_%d_GPORT2_MAX_PKT_SIZE                      = \n", xlmac_id);
    pr_info("max_pkt_size                                       = 0x%04x\n",max_pkt_size);
}
void mib_reg_gport3_max_pkt_size_dump(uint8_t xlmac_id)
{
    uint16_t max_pkt_size;
    ag_drv_xport_mib_reg_gport3_max_pkt_size_get(xlmac_id, &max_pkt_size);
    pr_info("\n");
    pr_info("MIB_REG_%d_GPORT3_MAX_PKT_SIZE                      = \n", xlmac_id);
    pr_info("max_pkt_size                                       = 0x%04x\n",max_pkt_size);
}
void mib_reg_ecc_cntrl_dump(uint8_t xlmac_id)
{
    uint8_t tx_mib_ecc_en; uint8_t rx_mib_ecc_en;
    ag_drv_xport_mib_reg_ecc_cntrl_get(xlmac_id, &tx_mib_ecc_en,&rx_mib_ecc_en);
    pr_info("\n");
    pr_info("MIB_REG_%d_ECC_CNTRL                                = \n", xlmac_id);
    pr_info("tx_mib_ecc_en                                      = 0x%04x\n",tx_mib_ecc_en);
    pr_info("rx_mib_ecc_en                                      = 0x%04x\n",rx_mib_ecc_en);

}
void mib_reg_force_sb_ecc_err_dump(uint8_t xlmac_id)
{
    xport_mib_reg_force_sb_ecc_err force_sb_ecc_err;
    ag_drv_xport_mib_reg_force_sb_ecc_err_get(xlmac_id, &force_sb_ecc_err);
    pr_info("\n");
    pr_info("MIB_REG_%d_FORCE_SB_ECC_ERR                         = \n", xlmac_id);
    pr_info("force_tx_mem3_serr                                 = 0x%02x\n",force_sb_ecc_err.force_tx_mem3_serr);
    pr_info("force_tx_mem2_serr                                 = 0x%02x\n",force_sb_ecc_err.force_tx_mem2_serr);
    pr_info("force_tx_mem1_serr                                 = 0x%02x\n",force_sb_ecc_err.force_tx_mem1_serr);
    pr_info("force_tx_mem0_serr                                 = 0x%02x\n",force_sb_ecc_err.force_tx_mem0_serr);
    pr_info("force_rx_mem4_serr                                 = 0x%02x\n",force_sb_ecc_err.force_rx_mem4_serr);
    pr_info("force_rx_mem3_serr                                 = 0x%02x\n",force_sb_ecc_err.force_rx_mem3_serr);
    pr_info("force_rx_mem2_serr                                 = 0x%02x\n",force_sb_ecc_err.force_rx_mem2_serr);
    pr_info("force_rx_mem1_serr                                 = 0x%02x\n",force_sb_ecc_err.force_rx_mem1_serr);
    pr_info("force_rx_mem0_serr                                 = 0x%02x\n",force_sb_ecc_err.force_rx_mem0_serr);

}
void mib_reg_force_db_ecc_err_dump(uint8_t xlmac_id)
{
    xport_mib_reg_force_db_ecc_err force_db_ecc_err;
    ag_drv_xport_mib_reg_force_db_ecc_err_get(xlmac_id, &force_db_ecc_err);
    pr_info("\n");
    pr_info("MIB_REG_%d_FORCE_DB_ECC_ERR                         = \n", xlmac_id);
    pr_info("force_tx_mem3_derr                                 = 0x%02x\n",force_db_ecc_err.force_tx_mem3_derr);
    pr_info("force_tx_mem2_derr                                 = 0x%02x\n",force_db_ecc_err.force_tx_mem2_derr);
    pr_info("force_tx_mem1_derr                                 = 0x%02x\n",force_db_ecc_err.force_tx_mem1_derr);
    pr_info("force_tx_mem0_derr                                 = 0x%02x\n",force_db_ecc_err.force_tx_mem0_derr);
    pr_info("force_rx_mem4_derr                                 = 0x%02x\n",force_db_ecc_err.force_rx_mem4_derr);
    pr_info("force_rx_mem3_derr                                 = 0x%02x\n",force_db_ecc_err.force_rx_mem3_derr);
    pr_info("force_rx_mem2_derr                                 = 0x%02x\n",force_db_ecc_err.force_rx_mem2_derr);
    pr_info("force_rx_mem1_derr                                 = 0x%02x\n",force_db_ecc_err.force_rx_mem1_derr);
    pr_info("force_rx_mem0_derr                                 = 0x%02x\n",force_db_ecc_err.force_rx_mem0_derr);

}
void mib_reg_rx_mem0_ecc_status_dump(uint8_t xlmac_id)
{
    uint8_t mem_addr; uint8_t double_bit_ecc_err; uint8_t multi_ecc_err; uint8_t ecc_err;
    ag_drv_xport_mib_reg_rx_mem0_ecc_status_get(xlmac_id, &mem_addr,&double_bit_ecc_err,&multi_ecc_err,&ecc_err);
    pr_info("\n");
    pr_info("MIB_REG_%d_RX_MEM0_ECC_STATUS                       = \n", xlmac_id);
    pr_info("mem_addr                                           = 0x%02x\n",mem_addr);
    pr_info("double_bit_ecc_err                                 = 0x%02x\n",double_bit_ecc_err);
    pr_info("multi_ecc_err                                      = 0x%02x\n",multi_ecc_err);
    pr_info("ecc_err                                            = 0x%02x\n",ecc_err);
}
void mib_reg_rx_mem1_ecc_status_dump(uint8_t xlmac_id)
{
    uint8_t mem_addr; uint8_t double_bit_ecc_err; uint8_t multi_ecc_err; uint8_t ecc_err;
    ag_drv_xport_mib_reg_rx_mem1_ecc_status_get(xlmac_id, &mem_addr,&double_bit_ecc_err,&multi_ecc_err,&ecc_err);
    pr_info("\n");
    pr_info("MIB_REG_%d_RX_MEM1_ECC_STATUS                       = \n", xlmac_id);
    pr_info("mem_addr                                           = 0x%02x\n",mem_addr);
    pr_info("double_bit_ecc_err                                 = 0x%02x\n",double_bit_ecc_err);
    pr_info("multi_ecc_err                                      = 0x%02x\n",multi_ecc_err);
    pr_info("ecc_err                                            = 0x%02x\n",ecc_err);
}
void mib_reg_rx_mem2_ecc_status_dump(uint8_t xlmac_id)
{
    uint8_t mem_addr; uint8_t double_bit_ecc_err; uint8_t multi_ecc_err; uint8_t ecc_err;
    ag_drv_xport_mib_reg_rx_mem2_ecc_status_get(xlmac_id, &mem_addr,&double_bit_ecc_err,&multi_ecc_err,&ecc_err);
    pr_info("\n");
    pr_info("MIB_REG_%d_RX_MEM2_ECC_STATUS                       = \n", xlmac_id);
    pr_info("mem_addr                                           = 0x%02x\n",mem_addr);
    pr_info("double_bit_ecc_err                                 = 0x%02x\n",double_bit_ecc_err);
    pr_info("multi_ecc_err                                      = 0x%02x\n",multi_ecc_err);
    pr_info("ecc_err                                            = 0x%02x\n",ecc_err);
}
void mib_reg_rx_mem3_ecc_status_dump(uint8_t xlmac_id)
{
    uint8_t mem_addr; uint8_t double_bit_ecc_err; uint8_t multi_ecc_err; uint8_t ecc_err;
    ag_drv_xport_mib_reg_rx_mem3_ecc_status_get(xlmac_id, &mem_addr,&double_bit_ecc_err,&multi_ecc_err,&ecc_err);
    pr_info("\n");
    pr_info("MIB_REG_%d_RX_MEM3_ECC_STATUS                       = \n", xlmac_id);
    pr_info("mem_addr                                           = 0x%02x\n",mem_addr);
    pr_info("double_bit_ecc_err                                 = 0x%02x\n",double_bit_ecc_err);
    pr_info("multi_ecc_err                                      = 0x%02x\n",multi_ecc_err);
    pr_info("ecc_err                                            = 0x%02x\n",ecc_err);
}
void mib_reg_rx_mem4_ecc_status_dump(uint8_t xlmac_id)
{
    uint8_t mem_addr; uint8_t double_bit_ecc_err; uint8_t multi_ecc_err; uint8_t ecc_err;
    ag_drv_xport_mib_reg_rx_mem4_ecc_status_get(xlmac_id, &mem_addr,&double_bit_ecc_err,&multi_ecc_err,&ecc_err);
    pr_info("\n");
    pr_info("MIB_REG_%d_RX_MEM4_ECC_STATUS                       = \n", xlmac_id);
    pr_info("mem_addr                                           = 0x%02x\n",mem_addr);
    pr_info("double_bit_ecc_err                                 = 0x%02x\n",double_bit_ecc_err);
    pr_info("multi_ecc_err                                      = 0x%02x\n",multi_ecc_err);
    pr_info("ecc_err                                            = 0x%02x\n",ecc_err);
}
void mib_reg_tx_mem0_ecc_status_dump(uint8_t xlmac_id)
{
    uint8_t mem_addr; uint8_t double_bit_ecc_err; uint8_t multi_ecc_err; uint8_t ecc_err;
    ag_drv_xport_mib_reg_tx_mem0_ecc_status_get(xlmac_id, &mem_addr,&double_bit_ecc_err,&multi_ecc_err,&ecc_err);
    pr_info("\n");
    pr_info("MIB_REG_%d_TX_MEM0_ECC_STATUS                       = \n", xlmac_id);
    pr_info("mem_addr                                           = 0x%02x\n",mem_addr);
    pr_info("double_bit_ecc_err                                 = 0x%02x\n",double_bit_ecc_err);
    pr_info("multi_ecc_err                                      = 0x%02x\n",multi_ecc_err);
    pr_info("ecc_err                                            = 0x%02x\n",ecc_err);
}
void mib_reg_tx_mem1_ecc_status_dump(uint8_t xlmac_id)
{
    uint8_t mem_addr; uint8_t double_bit_ecc_err; uint8_t multi_ecc_err; uint8_t ecc_err;
    ag_drv_xport_mib_reg_tx_mem1_ecc_status_get(xlmac_id, &mem_addr,&double_bit_ecc_err,&multi_ecc_err,&ecc_err);
    pr_info("\n");
    pr_info("MIB_REG_%d_TX_MEM1_ECC_STATUS                       = \n", xlmac_id);
    pr_info("mem_addr                                           = 0x%02x\n",mem_addr);
    pr_info("double_bit_ecc_err                                 = 0x%02x\n",double_bit_ecc_err);
    pr_info("multi_ecc_err                                      = 0x%02x\n",multi_ecc_err);
    pr_info("ecc_err                                            = 0x%02x\n",ecc_err);
}
void mib_reg_tx_mem2_ecc_status_dump(uint8_t xlmac_id)
{
    uint8_t mem_addr; uint8_t double_bit_ecc_err; uint8_t multi_ecc_err; uint8_t ecc_err;
    ag_drv_xport_mib_reg_tx_mem2_ecc_status_get(xlmac_id, &mem_addr,&double_bit_ecc_err,&multi_ecc_err,&ecc_err);
    pr_info("\n");
    pr_info("MIB_REG_%d_TX_MEM2_ECC_STATUS                       = \n", xlmac_id);
    pr_info("mem_addr                                           = 0x%02x\n",mem_addr);
    pr_info("double_bit_ecc_err                                 = 0x%02x\n",double_bit_ecc_err);
    pr_info("multi_ecc_err                                      = 0x%02x\n",multi_ecc_err);
    pr_info("ecc_err                                            = 0x%02x\n",ecc_err);
}
void mib_reg_tx_mem3_ecc_status_dump(uint8_t xlmac_id)
{
    uint8_t mem_addr; uint8_t double_bit_ecc_err; uint8_t multi_ecc_err; uint8_t ecc_err;
    ag_drv_xport_mib_reg_tx_mem3_ecc_status_get(xlmac_id, &mem_addr,&double_bit_ecc_err,&multi_ecc_err,&ecc_err);
    pr_info("\n");
    pr_info("MIB_REG_%d_TX_MEM3_ECC_STATUS                       = \n", xlmac_id);
    pr_info("mem_addr                                           = 0x%02x\n",mem_addr);
    pr_info("double_bit_ecc_err                                 = 0x%02x\n",double_bit_ecc_err);
    pr_info("multi_ecc_err                                      = 0x%02x\n",multi_ecc_err);
    pr_info("ecc_err                                            = 0x%02x\n",ecc_err);
}

reg_name_func1 mib_reg_name_func_array[] = {
    { "MIB_CNTRL", mib_reg_cntrl_dump },	 		
    { "MIB_EEE_PULSE_DURATION_CNTRL", mib_reg_eee_pulse_duration_cntrl_dump },
    { "MIB_GPORT0_MAX_PKT_SIZE", mib_reg_gport0_max_pkt_size_dump },	
    { "MIB_GPORT1_MAX_PKT_SIZE", mib_reg_gport1_max_pkt_size_dump },	
    { "MIB_GPORT2_MAX_PKT_SIZE", mib_reg_gport2_max_pkt_size_dump },	
    { "MIB_GPORT3_MAX_PKT_SIZE", mib_reg_gport3_max_pkt_size_dump },	
    { "MIB_ECC_CNTRL", mib_reg_ecc_cntrl_dump },	 		
    { "MIB_FORCE_SB_ECC_ERR", mib_reg_force_sb_ecc_err_dump },	 	
    { "MIB_FORCE_DB_ECC_ERR", mib_reg_force_db_ecc_err_dump },	 	
    { "MIB_RX_MEM0_ECC_STATUS", mib_reg_rx_mem0_ecc_status_dump },	 	
    { "MIB_RX_MEM1_ECC_STATUS", mib_reg_rx_mem1_ecc_status_dump },	 	
    { "MIB_RX_MEM2_ECC_STATUS", mib_reg_rx_mem2_ecc_status_dump },	 	
    { "MIB_RX_MEM3_ECC_STATUS", mib_reg_rx_mem3_ecc_status_dump },	 	
    { "MIB_RX_MEM4_ECC_STATUS", mib_reg_rx_mem4_ecc_status_dump },	 	
    { "MIB_TX_MEM0_ECC_STATUS", mib_reg_tx_mem0_ecc_status_dump },	 	
    { "MIB_TX_MEM1_ECC_STATUS", mib_reg_tx_mem1_ecc_status_dump },	 	
    { "MIB_TX_MEM2_ECC_STATUS", mib_reg_tx_mem2_ecc_status_dump },	 	
    { "MIB_TX_MEM3_ECC_STATUS", mib_reg_tx_mem3_ecc_status_dump },
    { "", NULL},
};

static void xport_proc_cmd_mib_reg_dump(uint8_t xlmac_id, char *reg_str)
{
    reg_name_func1 *p_array = &mib_reg_name_func_array[0];
    pr_info("\n**** XPORT_MIB_REG_%d block\n", xlmac_id);
    while(p_array->name[0] != '\0')
    {
        if (!reg_str || strstr(p_array->name, reg_str)) p_array->func1(xlmac_id);
        p_array++;
    }
}

void xport_top_ctrl_dump(uint8_t xlmac_id)
{
    uint8_t p3_mode, p2_mode, p1_mode, p0_mode;
    ag_drv_xport_top_ctrl_get(xlmac_id, &p3_mode, &p2_mode, &p1_mode, &p0_mode);
    pr_info("\n");
    pr_info("XPORT_TOP_%d_CNTRL                                  = \n", xlmac_id);
    pr_info("p3_mode                                            = 0x%02x\n",p3_mode);
    pr_info("p2_mode                                            = 0x%02x\n",p2_mode);
    pr_info("p1_mode                                            = 0x%02x\n",p1_mode);
    pr_info("p0_mode                                            = 0x%02x\n",p0_mode);
}
void xport_top_status_dump(uint8_t xlmac_id)
{
    uint8_t link_status;
    ag_drv_xport_top_status_get(xlmac_id, &link_status);
    pr_info("\n");
    pr_info("XPORT_TOP_%d_STATUS                                 = \n", xlmac_id);
    pr_info("link_status                                        = 0x%02x\n",link_status);
}
void xport_top_revision_dump(uint8_t xlmac_id)
{
    uint32_t xport_rev;
    ag_drv_xport_top_revision_get(xlmac_id, &xport_rev);
    pr_info("\n");
    pr_info("XPORT_TOP_%d_REVISION                               = \n", xlmac_id);
    pr_info("xport_rev                                          = 0x%x\n",xport_rev);
}
void xport_top_spare_dump(uint8_t xlmac_id)
{
}

reg_name_func1 xport_top_reg_name_func_array[] = {
    {"CNTRL", xport_top_ctrl_dump },        
    {"STATUS", xport_top_status_dump},
    {"REVISION", xport_top_revision_dump}, 
    {"SPARE_CNTRL", xport_top_spare_dump},
    { "", NULL},
};

static void xport_proc_cmd_xport_top_reg_dump(uint8_t xlmac_id, char *reg_str)
{
    reg_name_func1 *p_array = &xport_top_reg_name_func_array[0];
    pr_info("\n**** XPORT_TOP_%d block\n", xlmac_id);
    while(p_array->name[0] != '\0')
    {
        if (!reg_str || strstr(p_array->name, reg_str)) p_array->func1(xlmac_id);
        p_array++;
    }
}

typedef enum {
    eFIRST_BLK,
    eXLMAC_CORE = eFIRST_BLK,
    eXLMAC_REG,
    eXPORT_REG, //XPORT_TOP
    eMIB_REG,
    ePRESET, //XPORT_PORTRESET
    eMAB,
    eLAST_BLK = eMAB,
    eADDR,
    eINVALID
}enum_blk_type;

typedef struct {
    enum_blk_type blk_type;
    uint32_t start_addr;
    uint32_t size;
}blk_addr_range;

blk_addr_range blk_addr[] = {
    {eXLMAC_CORE,   (XPORT_PHYS_BASE+XPORT_XLMAC_CORE_OFFSET), XPORT_XLMAC_CORE_SIZE },
    {eXLMAC_REG,    (XPORT_PHYS_BASE+XPORT_XLMAC_REG_OFFSET), XPORT_XLMAC_REG_SIZE },
    {eXPORT_REG,    (XPORT_PHYS_BASE+XPORT_REG_OFFSET), XPORT_REG_SIZE },
    {eMIB_REG,      (XPORT_PHYS_BASE+XPORT_MIB_REG_OFFSET), XPORT_MIB_REG_SIZE },
    {eMAB,          (XPORT_PHYS_BASE+XPORT_MAB_REG_OFFSET), XPORT_MAB_REG_SIZE },
    {ePRESET,       (XPORT_PHYS_BASE+XPORT_PORTRESET_OFFSET), XPORT_PORTRESET_SIZE },
    {eXLMAC_CORE,   (XPORT1_PHYS_BASE+XPORT_XLMAC_CORE_OFFSET), XPORT_XLMAC_CORE_SIZE },
    {eXLMAC_REG,    (XPORT1_PHYS_BASE+XPORT_XLMAC_REG_OFFSET), XPORT_XLMAC_REG_SIZE },
    {eXPORT_REG,    (XPORT1_PHYS_BASE+XPORT_REG_OFFSET), XPORT_REG_SIZE },
    {eMIB_REG,      (XPORT1_PHYS_BASE+XPORT_MIB_REG_OFFSET), XPORT_MIB_REG_SIZE },
    {eMAB,          (XPORT1_PHYS_BASE+XPORT_MAB_REG_OFFSET), XPORT_MAB_REG_SIZE },
    {ePRESET,       (XPORT1_PHYS_BASE+XPORT_PORTRESET_OFFSET), XPORT_PORTRESET_SIZE },
    {eINVALID,0,0}
};


static int xport_proc_cmd_reg_dump(int argc, char *argv[])
{
    int instance = 0;
    char *xport_reg_dump_usage = 
        "Usage: reg_dump       #dump all xport registers\n"
        "       reg_dump XLMAC_CORE <port_id 0..7>\n"
        "       reg_dump XLMAC_WRAP | MIB_WRAP | XPORT_TOP | MAB | PORTRESET  <xlmac_id 0..1>\n"
        "       reg_dump addr <Qualifying string>\n";
    int instance_max = 1;
    int blk_idx = 1;
    char *blk_p = argv[1];
    char *q_str_p = NULL;
    enum_blk_type blk_type = eINVALID;
    uint32_t reg_phy_addr = 0;
    uintptr_t reg_addr;

    if (argc == 1)
    {
        instance = 0;
        blk_type = eFIRST_BLK;
        goto DUMP_REGS;
    }
    if (argc < 2)
    {
        pr_info("\n");
        pr_info("%s\n", xport_reg_dump_usage);
        return 0;
    }

    if (!strcmp(blk_p, "XLMAC_CORE")) { blk_type = eXLMAC_CORE; instance_max = 7; } 
    else if (!strcmp(blk_p, "XLMAC_WRAP")) blk_type = eXLMAC_REG;
    else if (!strcmp(blk_p, "XPORT_TOP")) blk_type = eXPORT_REG;
    else if (!strcmp(blk_p, "MIB_WRAP")) blk_type = eMIB_REG;
    else if (!strcmp(blk_p, "MAB")) blk_type = eMAB;
    else if (!strcmp(blk_p, "PORTRESET")) blk_type = ePRESET;
    else if (!strcmp(blk_p, "addr")) blk_type = eADDR;

    if (blk_type == eADDR && argc == 3) {
        int i;
        uint32_t v32;
        uint64_t v64;
        kstrtou32(argv[2], 16, &reg_phy_addr);
        for (i=0; blk_addr[i].blk_type != eINVALID;i++) {
            if (reg_phy_addr >= (blk_addr[i].start_addr) && reg_phy_addr <= (blk_addr[i].start_addr+blk_addr[i].size)) {
                blk_type = blk_addr[i].blk_type;
                break;
            }
        }
        if (blk_addr[i].blk_type == eINVALID) {
            pr_info("\n");
            pr_info("Reg Address <0x%08x> Outside of XPORT block range\n", reg_phy_addr);
            return 0;
        }
        reg_addr = (uintptr_t)XPORT_BASE + ( reg_phy_addr - XPORT_PHYS_BASE+XPORT_OFFSET );

        switch ( blk_type )
        {
        case eXLMAC_CORE:
            xport_xlmac_indirect_read(0, (reg_phy_addr>=XPORT1_PHYS_BASE)?1:0, reg_addr, &v64);
            printk(" 0x%08x = 0x%016llx \n",reg_phy_addr,v64);
            break;
        case eXLMAC_REG:
        case eXPORT_REG:
        case eMIB_REG:
        case ePRESET:
        case eMAB:
            READ_32(reg_addr, v32);
            printk(" 0x%08x = 0x%08x \n",reg_phy_addr,v32);
            break;
        default:
            break;
        }
        return 0;
    }

    if (blk_type == eINVALID || argc <= blk_idx+1)
    {
        pr_info("\n");
        pr_info("%s\n", xport_reg_dump_usage);
        return 0;
    }
    if ((instance = xport_port_validate(argc, argv, blk_idx+1, 0, instance_max)) == -1)
    {
        pr_info("Command validation failed : instance <%d> argc <%d> ", instance, argc);
        pr_info("\n");
        pr_info("%s\n", xport_reg_dump_usage);
        return 0;
    }


    if (argc > blk_idx+2)
    {
        q_str_p = argv[blk_idx+2];
    }

DUMP_REGS:
    do
    {
        switch ( blk_type )
        {
        case eXLMAC_CORE:
            xport_proc_cmd_xlmac_core_reg_dump(instance, q_str_p);
            break;
        case eXLMAC_REG:
            xport_proc_cmd_xlmac_reg_dump(instance, q_str_p);
            break;
        case eXPORT_REG:
            xport_proc_cmd_xport_top_reg_dump(instance, q_str_p);
            break;
        case eMIB_REG:
            xport_proc_cmd_mib_reg_dump(instance, q_str_p);
            break;
        case eMAB:
            xport_proc_cmd_mab_reg_dump(instance, q_str_p);
            break;
        case ePRESET:
            xport_proc_cmd_port_reset_dump(instance, q_str_p);
        default:
            break;
        }
        if ((blk_type == eXLMAC_CORE && instance < 7) || instance < 1)
        {
            instance++;
        }
        else
        {   blk_type++; instance=0; }

    } while (argc == 1 && blk_type != eLAST_BLK+1);
    return 0;
}
