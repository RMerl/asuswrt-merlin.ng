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
 *  Created on: Nov/2015
 *      Author: ido@broadcom.com
 */

#ifndef _PORT_PLATFORM_H_
#define _PORT_PLATFORM_H_

#define MAX_SWITCHES               2
#if defined(CONFIG_BCM96858) || defined(_BCM96858_) || \
    defined(CONFIG_BCM96856) || defined(_BCM96856_) || \
    defined(CONFIG_BCM96846) || defined(_BCM96846_) || \
    defined(CONFIG_BCM96878) || defined(_BCM96878_) || \
    defined(CONFIG_BCM96855) || defined(_BCM96855_)
#define MAX_SWITCH_PORTS             9
#elif defined(CONFIG_BCM94912) || defined(CONFIG_BCM96813)
#define MAX_SWITCH_PORTS            12
#else
#define MAX_SWITCH_PORTS             8
#endif
#define CROSSBAR_PORT_BASE          (MAX_SWITCH_PORTS+1)
#define PHY_PORT_TO_CROSSBAR_PORT(phy_port)  (phy_port - CROSSBAR_PORT_BASE)
#define CROSSBAR_PORT_TO_PHY_PORT(cross_port) (cross_port + CROSSBAR_PORT_BASE)

#endif

