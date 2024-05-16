/*
<:copyright-BRCM:2018:DUAL/GPL:standard

   Copyright (c) 2018 Broadcom 
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

/*
*******************************************************************************
* File Name  : genl_msgs.h
*
* Description: This file contains the Broadcom Tcp Speed Test Generic Netlink global definitions.
*
*  Created on: Dec 20, 2017
*      Author: ilanb, yonatani
*******************************************************************************
*/

#ifndef __GENL_MSGS_H_INCLUDED__
#define __GENL_MSGS_H_INCLUDED__

#include "tcpspdtest_defs.h"

/******************************************* Functions *******************************************/
int tcpspd_genl_init(void);
int tcpspd_genl_shutdown(void);
int tcpspd_genl_send_speed_report_msg(uint8_t stream_idx, tcp_spdt_rep_t *spd_report, tcpspdtest_genl_cmd_status_t status);
int tcpspd_genl_clear_event(uint8_t stream_idx);
int tcpspd_genl_send_event(uint8_t stream_idx, tcp_spdt_rep_t *spd_report, tcpspdtest_genl_cmd_status_t status);
int tcpspd_genl_send_pending_event(uint8_t stream_idx);

#endif /* __GENL_MSGS_H_INCLUDED__ */
