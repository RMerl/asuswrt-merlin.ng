/*
<:copyright-BRCM:2007:DUAL/GPL:standard

   Copyright (c) 2007 Broadcom 
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

#ifndef _BCMSW_API_H_
#define _BCMSW_API_H_

/*LinkChange arguments to pass in to bcm_log registered function
 *BCM_LOG_FUN_ID_ENET_LINK_CHG.*/
typedef struct {
  int port;
  int linkstatus;
  int speed;
  int duplex;
  bool activeELink;
} LinkChangeArgs;

#endif /* _BCMSW_API_H_ */

