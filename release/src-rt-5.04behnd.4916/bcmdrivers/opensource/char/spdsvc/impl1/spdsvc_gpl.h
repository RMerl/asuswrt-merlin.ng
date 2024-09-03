/*
   <:copyright-BRCM:2022:DUAL/GPL:standard
   
      Copyright (c) 2022 Broadcom 
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
int spdsvc_tr471_rx_queue_write(spdsvc_tr471_rx_queue_if_entry_t *entry_p);
int spdsvc_tr471_burst_cmpl_event(void);

#endif  /* __SPDSVC_GPL_H_INCLUDED__ */
