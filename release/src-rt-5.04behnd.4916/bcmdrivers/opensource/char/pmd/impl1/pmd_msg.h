/*
   Copyright (c) 2013 Broadcom Corporation
   All Rights Reserved

    <:label-BRCM:2013:DUAL/GPL:standard

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
#ifndef _PMD_MSG_H_
#define _PMD_MSG_H_

#include "reg/HostCommon.h"

typedef enum {
    pmd_get_msg,
    pmd_set_msg,
} host_msg_type;

int pmd_msg_handler(hm_msg_id id, int16_t *buf, uint16_t len);
host_msg_type get_msg_type(hm_msg_id id);
uint16_t get_msg_seq_num(hm_msg_id id);
void msg_system_init(void);

#endif /* _PMD_MSG_H_ */
