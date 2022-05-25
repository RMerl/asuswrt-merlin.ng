/*
* <:copyright-BRCM:2019:DUAL/GPL:standard
* 
*    Copyright (c) 2019 Broadcom 
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

#ifndef BCM_BCA_LED_DT_BINDINGS_H
#define BCM_BCA_LED_DT_BINDINGS_H

#define LED_SPEED_10   0x00000001
#define LED_SPEED_100  0x00000002
#define LED_SPEED_1G   0x00000004
#define LED_SPEED_2500 0x00000008
#define LED_SPEED_10G  0x00000010

#define LED_SPEED_FAE  (LED_SPEED_10 | LED_SPEED_100)
#define LED_SPEED_GBE  (LED_SPEED_FAE | LED_SPEED_1G)
#define LED_SPEED_ALL  (LED_SPEED_GBE | LED_SPEED_2500 | LED_SPEED_10G)

#define STATE_ON  1
#define STATE_OFF 1
#endif
