/*
* <:copyright-BRCM:2016:DUAL/GPL:standard
* 
*    Copyright (c) 2016 Broadcom 
*    All Rights Reserved
* 
* Unless you and Broadcom execute a separate written software license
* agreement governing use of this software, this software is licensed
* to you under the terms of the GNU General Public License version 2
* (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
* with the following added to such license:
* 
*    As a special exception, the copyright holders of this software give
*    you permission to link this software with independent modules, and
*    to copy and distribute the resulting executable under terms of your
*    choice, provided that you also meet, for each linked independent
*    module, the terms and conditions of the license of that module.
*    An independent module is a module which is not derived from this
*    software.  The special exception does not apply to any modifications
*    of the software.
* 
* Not withstanding the above, under no circumstances may you combine
* this software in any way with any other Broadcom software provided
* under a license other than the GPL, without Broadcom's express prior
* written consent.
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
