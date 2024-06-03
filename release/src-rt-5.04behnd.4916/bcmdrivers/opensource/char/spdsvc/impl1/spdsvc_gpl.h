/*
   <:copyright-BRCM:2022:DUAL/GPL:standard
   
      Copyright (c) 2022 Broadcom 
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
*
* File Name  : spdsvc_gpl.h
*
* Description: Speed Service GPL Driver
*
*******************************************************************************
*/

#ifndef __SPDSVC_GPL_H_INCLUDED__
#define __SPDSVC_GPL_H_INCLUDED__

typedef int (* spdsvc_ioctl_command_t)(spdsvc_ioctl_t ioctlCmd, spdsvc_ioctl_arg_t *userSpdsvc_p);

int spdsvc_gpl_bind(spdsvc_ioctl_command_t spdsvc_ioctl_command);

typedef enum {
    SPDSVC_TR471_EVT_RX_QUEUE,
    SPDSVC_TR471_EVT_BURST_CMPL,
    SPDSVC_TR471_EVT_MAX
}eSpdSvc_tr471_evt_type;

int spdsvc_event_register(eSpdSvc_tr471_evt_type etype, int event_fd);
int spdsvc_event_unregister(eSpdSvc_tr471_evt_type etype);
int spdsvc_event_unregister_all(void);

void spdsvc_tr471_rx_queue_init(void);
int spdsvc_tr471_rx_queue_write(spdsvc_tr471_rx_queue_entry_t *entry_p);
int spdsvc_tr471_burst_cmpl_event(void);

#endif  /* __SPDSVC_GPL_H_INCLUDED__ */
