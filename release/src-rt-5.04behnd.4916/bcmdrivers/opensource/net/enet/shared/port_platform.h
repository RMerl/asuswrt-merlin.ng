/*
   <:copyright-BRCM:2015:DUAL/GPL:standard
   
      Copyright (c) 2015 Broadcom 
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
 *  Created on: Nov/2015
 *      Author: ido@broadcom.com
 */

#ifndef _PORT_PLATFORM_H_
#define _PORT_PLATFORM_H_

#ifndef CONFIG_BCM_FTTDP_G9991
#define MAX_SWITCHES               2
#else
#define MAX_SWITCHES               10
#endif

#if defined(CONFIG_BCM96858) || defined(_BCM96858_) || \
    defined(CONFIG_BCM96856) || defined(_BCM96856_) || \
    defined(CONFIG_BCM96846) || defined(_BCM96846_) || \
    defined(CONFIG_BCM96878) || defined(_BCM96878_) || \
    defined(CONFIG_BCM96855) || defined(_BCM96855_)
#define MAX_SWITCH_PORTS             9
#elif defined(CONFIG_BCM94912) || defined(CONFIG_BCM96813)
#define MAX_SWITCH_PORTS            12
#elif defined(CONFIG_BCM96888)
#define MAX_SWITCH_PORTS            16
#else
#define MAX_SWITCH_PORTS             8
#endif
#define CROSSBAR_PORT_BASE          (MAX_SWITCH_PORTS+1)
#define PHY_PORT_TO_CROSSBAR_PORT(phy_port)  (phy_port - CROSSBAR_PORT_BASE)
#define CROSSBAR_PORT_TO_PHY_PORT(cross_port) (cross_port + CROSSBAR_PORT_BASE)

#endif

