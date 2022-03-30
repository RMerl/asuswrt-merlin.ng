/*
   <:copyright-BRCM:2019:DUAL/GPL:standard
   
      Copyright (c) 2019 Broadcom 
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
 *  Created on: Jul/2019
 *      Author: nikolai.iosifov@broadcom.com
 */

#if defined(CONFIG_NET_SWITCHDEV)

#ifndef _BR_SDEV_H_
#define _BR_SDEV_H_

#include <linux/netdevice.h>

int runner_switchdev_init(void);
int runner_switchdev_cleanup(void);

int runner_switchdev_learning_enabled(struct net_device *dev);

#endif /* _BR_SDEV_H_ */

#endif /* CONFIG_NET_SWITCHDEV */
