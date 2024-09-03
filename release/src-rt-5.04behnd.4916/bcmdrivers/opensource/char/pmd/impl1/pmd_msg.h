/*
   Copyright (c) 2013 Broadcom Corporation
   All Rights Reserved

    <:label-BRCM:2013:DUAL/GPL:standard
    
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
