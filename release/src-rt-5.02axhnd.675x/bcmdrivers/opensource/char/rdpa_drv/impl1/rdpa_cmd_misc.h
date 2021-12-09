#ifndef __RDPA_CMD_MISC_H_INCLUDED__
#define __RDPA_CMD_MISC_H_INCLUDED__

/*
 <:copyright-BRCM:2015:DUAL/GPL:standard
 
    Copyright (c) 2015 Broadcom 
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
 * File Name  : rdpa_cmd_misc.h
 *
 * Description: This file contains the miscellaneous API.
 *
 *******************************************************************************
 */
int rdpa_cmd_misc_ioctl(unsigned long arg);
void rdpa_cmd_misc_init(void);
int get_rdpa_wan_type(rdpa_if if_, rdpa_wan_type *wan_type);
int is_ae_enable(rdpa_if if_);


#endif /* __RDPA_CMD_MISC_H_INCLUDED__ */

