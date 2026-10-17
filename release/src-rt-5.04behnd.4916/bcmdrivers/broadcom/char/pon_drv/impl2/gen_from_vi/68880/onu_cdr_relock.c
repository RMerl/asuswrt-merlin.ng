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


  void onu_cdr_relock(int port, int __ARG0) {
    uint32_t reg_data;
    print_log("pa_comment: sgb_msg_ral_sequence: BEGIN onu_cdr_relock port:%d __ARG0:%d\n", port, __ARG0);
    //#######################################
    // call_msg: ..\..\ONU\Common\D6S__SerDes\MSG\General\onu_cdr_relock
    //#######################################
    // load atm file: ..\..\ONU\Common\D6S__SerDes\ATM\General\onu_cdr_relock.csv, val_select:val
    //#######################################
    // cfg_class: cdr_relock
    //#######################################
    // pre_order
    reg_data = epon_read32("0x80118b14 ", 0x80118b14 );
    reg_data = set_reg_field(reg_data,
                             0x00000002 ,
                             1 ,
                             1 ,
                             0x1);
    epon_write32("0x80118b14 ", 0x80118b14 , reg_data);
    reg_data = epon_read32("0x80118a14 ", 0x80118a14 );
    reg_data = set_reg_field(reg_data,
                             0x00000020 ,
                             1 ,
                             5 ,
                             0x0);
    epon_write32("0x80118a14 ", 0x80118a14 , reg_data);
    reg_data = epon_read32("0x80118a14 ", 0x80118a14 );
    reg_data = set_reg_field(reg_data,
                             0x00000020 ,
                             1 ,
                             5 ,
                             0x1);
    epon_write32("0x80118a14 ", 0x80118a14 , reg_data);
    reg_data = epon_read32("0x80118a14 ", 0x80118a14 );
    reg_data = set_reg_field(reg_data,
                             0x00000020 ,
                             1 ,
                             5 ,
                             0x0);
    epon_write32("0x80118a14 ", 0x80118a14 , reg_data);
    reg_data = epon_read32("0x80118b14 ", 0x80118b14 );
    reg_data = set_reg_field(reg_data,
                             0x00000002 ,
                             1 ,
                             1 ,
                             0x0);
    epon_write32("0x80118b14 ", 0x80118b14 , reg_data);
    // unorder
    // post_order
    //#######################################
    // end of cfg_class: cdr_relock
    //#######################################
    //return(0);
    //#######################################
    // end of call_msg: onu_cdr_relock
    //#######################################
    print_log("pa_comment: sgb_msg_ral_sequence: END onu_cdr_relock port:%d __ARG0:%d\n", port, __ARG0);
  }

