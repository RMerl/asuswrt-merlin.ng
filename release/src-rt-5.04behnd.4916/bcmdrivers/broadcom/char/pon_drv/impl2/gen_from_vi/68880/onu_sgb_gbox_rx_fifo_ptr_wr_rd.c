/*
    Copyright 2024 Broadcom Corporation

    <:label-BRCM:2024:DUAL/GPL:standard

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


#include "pon_drv_hw_acc.h"

  uint32_t onu_sgb_gbox_rx_fifo_ptr_wr_rd(int port, int __ARG0, int __ARG1, int __ARG2, int __ARG3) {
    uint32_t itut_or_ieee_sel = __ARG0;
    uint32_t pon_type_select = __ARG1;
    uint32_t load_pointers = __ARG2;
    uint32_t rx_fifo_offset_val = __ARG3;
    uint32_t enum_itut = 0;
    uint32_t enum_ieee = 1;
    uint32_t enum_xgs = 0;
    uint32_t enum_xgpon = 1;
    uint32_t enum_gpon = 2;
    uint32_t enum_epon_10g10g = 0;
    uint32_t enum_epon_10g1g = 1;
    uint32_t enum_epon_2g1g = 2;
    uint32_t enum_epon_1g1g = 3;
    uint32_t enum_ae_10g10g = 4;
    uint32_t enum_ae_5g5g = 5;
    uint32_t enum_ae_2g2g = 6;
    uint32_t enum_ae_1g1g = 7;
    uint32_t enum_mpcs_10g10g = 8;
    uint32_t enum_mpcs_5g5g = 9;
    uint32_t enum_mpcs_2g2g = 10;
    uint32_t enum_mpcs_1g1g = 11;
    uint32_t enum_ah_gb = 0;
    uint32_t enum_av_gb = 1;
    uint32_t enum_ae_gb = 2;
    uint32_t enum_gpon_gb = 3;
    uint32_t enum_ngpon_gb = 4;

    uint32_t rx_gearbox_select = 0;
    uint32_t reg_data = 0;

    uint32_t    __RETC = 0;
 
    print_log("pa_comment: sgb_msg_ral_sequence: BEGIN onu_sgb_gbox_rx_fifo_ptr_wr_rd port:%d __ARG0:%d\n", port, __ARG0);
    //#######################################
    // call_msg: ..\..\ONU\Common\D6S__SerDes\MSG\General\onu_sgb_gbox_rx_fifo_ptr_wr_rd
    //#######################################

    if (itut_or_ieee_sel==enum_itut) {
        if (pon_type_select==enum_xgs) {
            rx_gearbox_select = enum_ngpon_gb;
        } else
        if (pon_type_select==enum_xgpon) {
            rx_gearbox_select = enum_ngpon_gb;
        } else
        if (pon_type_select==enum_gpon) {
            rx_gearbox_select = enum_gpon_gb;
        } else {
            //return(1);
        }
    } else
    if (itut_or_ieee_sel==enum_ieee) {
        if (pon_type_select==enum_epon_10g10g) {
            rx_gearbox_select = enum_av_gb;
        } else
        if (pon_type_select==enum_epon_10g1g) {
            rx_gearbox_select = enum_av_gb;
        } else
        if (pon_type_select==enum_epon_2g1g) {
            rx_gearbox_select = enum_ah_gb;
        } else
        if (pon_type_select==enum_epon_1g1g) {
            rx_gearbox_select = enum_ah_gb;
        } else
        if (pon_type_select==enum_ae_10g10g) {
            rx_gearbox_select = enum_av_gb;
        } else
        if (pon_type_select==enum_ae_5g5g) {
            rx_gearbox_select = enum_av_gb;
        } else
        if (pon_type_select==enum_ae_2g2g) {
            rx_gearbox_select = enum_ah_gb;
        } else
        if (pon_type_select==enum_ae_1g1g) {
            rx_gearbox_select = enum_ah_gb;
        } else
        if (pon_type_select==enum_mpcs_10g10g) {
            rx_gearbox_select = enum_ae_gb;
        } else
        if (pon_type_select==enum_mpcs_5g5g) {
            rx_gearbox_select = enum_ae_gb;
        } else
        if (pon_type_select==enum_mpcs_2g2g) {
            rx_gearbox_select = enum_ae_gb;
        } else
        if (pon_type_select==enum_mpcs_1g1g) {
            rx_gearbox_select = enum_ae_gb;
        } else {
            //return(2);
        }
    } else {
        //return(3);
    }
    // load atm file: ..\..\ONU\Common\D6S__SerDes\ATM\General\onu_general.csv, val_select:val
    if (rx_gearbox_select==enum_ah_gb) {
        //#######################################
        // cfg_class: ah_rx
        //#######################################
        // pre_order
        reg_data = epon_read32("0x80110058 ", 0x80110058 );
        reg_data = set_reg_field(reg_data,
                                 0x0000000f ,
                                 4 ,
                                 0 ,
                                 rx_fifo_offset_val);
        epon_write32("0x80110058 ", 0x80110058 , reg_data);
        reg_data = epon_read32("0x80110058 ", 0x80110058 );
        reg_data = set_reg_field(reg_data,
                                 0x00000100 ,
                                 1 ,
                                 8 ,
                                 0x0);
        epon_write32("0x80110058 ", 0x80110058 , reg_data);
        reg_data = epon_read32("0x80110058 ", 0x80110058 );
        reg_data = set_reg_field(reg_data,
                                 0x00000100 ,
                                 1 ,
                                 8 ,
                                 load_pointers);
        epon_write32("0x80110058 ", 0x80110058 , reg_data);
        reg_data = epon_read32("0x80110058 ", 0x80110058 );
        reg_data = set_reg_field(reg_data,
                                 0x00000100 ,
                                 1 ,
                                 8 ,
                                 0x0);
        epon_write32("0x80110058 ", 0x80110058 , reg_data);
        reg_data = epon_read32("0x8011005c ", 0x8011005c );
        // unorder
        // post_order
        //#######################################
        // end of cfg_class: ah_rx
        //#######################################
    } else
    if (rx_gearbox_select==enum_av_gb) {
        //#######################################
        // cfg_class: av_rx
        //#######################################
        // pre_order
        reg_data = epon_read32("0x80110078 ", 0x80110078 );
        // unorder
        // post_order
        //#######################################
        // end of cfg_class: av_rx
        //#######################################
    } else
    if (rx_gearbox_select==enum_ae_gb) {
        //#######################################
        // cfg_class: ae_rx
        //#######################################
        // pre_order
        reg_data = epon_read32("0x801100a8 ", 0x801100a8 );
        reg_data = set_reg_field(reg_data,
                                 0x00f00000 ,
                                 4 ,
                                 20 ,
                                 rx_fifo_offset_val);
        epon_write32("0x801100a8 ", 0x801100a8 , reg_data);
        reg_data = epon_read32("0x801100a8 ", 0x801100a8 );
        reg_data = set_reg_field(reg_data,
                                 0x00080000 ,
                                 1 ,
                                 19 ,
                                 0x0);
        epon_write32("0x801100a8 ", 0x801100a8 , reg_data);
        reg_data = epon_read32("0x801100a8 ", 0x801100a8 );
        reg_data = set_reg_field(reg_data,
                                 0x00080000 ,
                                 1 ,
                                 19 ,
                                 load_pointers);
        epon_write32("0x801100a8 ", 0x801100a8 , reg_data);
        reg_data = epon_read32("0x801100a8 ", 0x801100a8 );
        reg_data = set_reg_field(reg_data,
                                 0x00080000 ,
                                 1 ,
                                 19 ,
                                 0x0);
        epon_write32("0x801100a8 ", 0x801100a8 , reg_data);
        reg_data = epon_read32("0x801100ac ", 0x801100ac );
        // unorder
        // post_order
        //#######################################
        // end of cfg_class: ae_rx
        //#######################################
    } else
    if (rx_gearbox_select==enum_gpon_gb) {
        //#######################################
        // cfg_class: gpon_rx
        //#######################################
        // pre_order
        reg_data = epon_read32("0x801100c4 ", 0x801100c4 );
        reg_data = set_reg_field(reg_data,
                                 0x0000001f ,
                                 5 ,
                                 0 ,
                                 rx_fifo_offset_val);
        epon_write32("0x801100c4 ", 0x801100c4 , reg_data);
        reg_data = epon_read32("0x801100c4 ", 0x801100c4 );
        reg_data = set_reg_field(reg_data,
                                 0x00000020 ,
                                 1 ,
                                 5 ,
                                 0x0);
        epon_write32("0x801100c4 ", 0x801100c4 , reg_data);
        reg_data = epon_read32("0x801100c4 ", 0x801100c4 );
        reg_data = set_reg_field(reg_data,
                                 0x00000020 ,
                                 1 ,
                                 5 ,
                                 load_pointers);
        epon_write32("0x801100c4 ", 0x801100c4 , reg_data);
        reg_data = epon_read32("0x801100c4 ", 0x801100c4 );
        reg_data = set_reg_field(reg_data,
                                 0x00000020 ,
                                 1 ,
                                 5 ,
                                 0x0);
        epon_write32("0x801100c4 ", 0x801100c4 , reg_data);
        reg_data = epon_read32("0x801100c8 ", 0x801100c8 );
        // unorder
        // post_order
        //#######################################
        // end of cfg_class: gpon_rx
        //#######################################
    } else
    if (rx_gearbox_select==enum_ngpon_gb) {
        //#######################################
        // cfg_class: ngpon_rx
        //#######################################
        // pre_order
        reg_data = epon_read32("0x801100e0 ", 0x801100e0 );
        reg_data = set_reg_field(reg_data,
                                 0x0000001f ,
                                 5 ,
                                 0 ,
                                 rx_fifo_offset_val);
        epon_write32("0x801100e0 ", 0x801100e0 , reg_data);
        reg_data = epon_read32("0x801100e0 ", 0x801100e0 );
        reg_data = set_reg_field(reg_data,
                                 0x00000020 ,
                                 1 ,
                                 5 ,
                                 0x0);
        epon_write32("0x801100e0 ", 0x801100e0 , reg_data);
        reg_data = epon_read32("0x801100e0 ", 0x801100e0 );
        reg_data = set_reg_field(reg_data,
                                 0x00000020 ,
                                 1 ,
                                 5 ,
                                 load_pointers);
        epon_write32("0x801100e0 ", 0x801100e0 , reg_data);
        reg_data = epon_read32("0x801100e0 ", 0x801100e0 );
        reg_data = set_reg_field(reg_data,
                                 0x00000020 ,
                                 1 ,
                                 5 ,
                                 0x0);
        epon_write32("0x801100e0 ", 0x801100e0 , reg_data);
        reg_data = epon_read32("0x801100e4 ", 0x801100e4 );
        // unorder
        // post_order
        //#######################################
        // end of cfg_class: ngpon_rx
        //#######################################
    } else {
        //return(4);
    }
    __RETC = 1;
    //return(0);
    //#######################################
    // end of call_msg: onu_sgb_gbox_rx_fifo_ptr_wr_rd
    //#######################################
    print_log("pa_comment: sgb_msg_ral_sequence: END onu_sgb_gbox_rx_fifo_ptr_wr_rd port:%d __ARG0:%d\n", port, __ARG0);

    return __RETC;
  }

