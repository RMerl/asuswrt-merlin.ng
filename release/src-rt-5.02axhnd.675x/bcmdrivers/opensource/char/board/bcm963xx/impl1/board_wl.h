/*
* <:copyright-BRCM:2016:DUAL/GPL:standard
* 
*    Copyright (c) 2016 Broadcom 
*    All Rights Reserved
* 
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License, version 2, as published by
* the Free Software Foundation (the "GPL").
* 
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
* 
* 
* A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
* writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
* Boston, MA 02111-1307, USA.
* 
* :> 
*/


#ifndef _BOARD_WL_H_
#define _BOARD_WL_H_

#include <linux/poll.h>

/* SES Button press types */
#define SES_BTN_LEGACY             1
#define SES_BTN_AP                 2
#define SES_BTN_STA                3

#if defined (WIRELESS)
#define SES_LED_OFF                0
#define SES_LED_ON                 1
#define SES_LED_BLINK              2

#if defined(CONFIG_BCM963268)
#define WLAN_ONBOARD_SLOT       WLAN_ONCHIP_DEV_SLOT
#else
#define WLAN_ONBOARD_SLOT       1 /* Corresponds to IDSEL -- EBI_A11/PCI_AD12 */
#endif

#define BRCM_VENDOR_ID       0x14e4
#define BRCM_WLAN_DEVICE_IDS 0x4300
#define BRCM_WLAN_DEVICE_IDS_DEC 43

#define WLAN_ON   1
#define WLAN_OFF  0
#endif

void __init  ses_board_init(void);
void __exit  ses_board_deinit(void);

unsigned short sesBtn_getIrq(void);
void sesLed_ctrl(int action);
int _wlsrom_write_file(char *name,char *content,int size);
int _get_wl_nandmanufacture(void );

void kerSetWirelessPD(int state);
#if defined(WIRELESS)
unsigned int sesBtn_poll(struct file *file, struct poll_table_struct *wait);
ssize_t sesBtn_read(struct file *file,  char __user *buffer, size_t count, loff_t *ppos);
void board_util_wl_godefault(void);
#endif
void btnHook_Ses(unsigned long timeInMs, void* param);

void __init  board_wl_init(void);
void __exit  board_wl_deinit(void);

#endif
