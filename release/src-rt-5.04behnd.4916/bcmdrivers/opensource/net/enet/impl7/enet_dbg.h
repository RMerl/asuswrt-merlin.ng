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

#ifndef _ENET_DBG_H_
#define _ENET_DBG_H_

#define enet_err(fmt, ...) \
    printk(KERN_ERR "%s:L%d "fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)

//#define enet_dbg_enabled 
//#define enet_dbg_rx_enabled
//#define enet_dbg_tx_enabled
//#define enet_dbgv_enabled   /* verbose debug */


#if defined(enet_dbg_enabled)
#define enet_dbg(fmt, ...) \
    printk(KERN_ERR "%s:L%d "fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#else
#define enet_dbg(fmt, ...) \
    {}
#endif

#if defined(enet_dbg_enabled) && defined(enet_dbg_rx_enabled)
#define enet_dbg_rx(fmt, ...) \
    printk(KERN_ERR "%s:L%d "fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#else
#define enet_dbg_rx(fmt, ...) \
    {}
#endif

#if defined(enet_dbg_enabled) && defined(enet_dbg_tx_enabled)
#define enet_dbg_tx(fmt, ...) \
    printk(KERN_ERR "%s:L%d "fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#else
#define enet_dbg_tx(fmt, ...) \
    {}
#endif

#if defined(enet_dbg_enabled) && defined(enet_dbgv_enabled)
#define enet_dbgv(fmt, ...) \
    printk(KERN_ERR " DBGV>%s:L%d "fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#else
#define enet_dbgv(fmt, ...) \
    {}
#endif



#endif // _ENET_DBG_H_

