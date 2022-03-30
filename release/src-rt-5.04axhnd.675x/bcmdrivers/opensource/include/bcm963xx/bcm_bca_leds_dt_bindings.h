/*
* <:copyright-BRCM:2019:DUAL/GPL:standard
* 
*    Copyright (c) 2019 Broadcom 
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
