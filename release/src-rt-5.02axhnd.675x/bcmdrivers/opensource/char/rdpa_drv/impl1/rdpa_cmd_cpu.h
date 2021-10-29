#ifndef __RDPA_CMD_CPU_H_INCLUDED__
#define __RDPA_CMD_CPU_H_INCLUDED__

/*
 <:copyright-BRCM:2007:DUAL/GPL:standard
 
    Copyright (c) 2007 Broadcom 
    All Rights Reserved
 
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

/*
 *******************************************************************************
 * File Name  : rdpa_cmd_cpu.h
 *
 * Description: This file contains the CPU API.
 *
 *******************************************************************************
 */
int rdpa_iq_cpu_add_l4port_to_reason(void *iq_param);
int rdpa_iq_cpu_rem_l4port_to_reason(void *iq_param);
int rdpa_iq_cpu_get_l4port_to_reason(void *iq_param);
int rdpa_iq_cpu_get_status(void *iq_param);

#endif /* __RDPA_CMD_BR_H_INCLUDED__ */
