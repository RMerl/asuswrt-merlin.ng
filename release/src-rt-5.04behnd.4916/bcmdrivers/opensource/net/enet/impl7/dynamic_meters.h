/*
  <:copyright-BRCM:2018:DUAL/GPL:standard
  
     Copyright (c) 2018 Broadcom 
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

  Author: kosta.sopov@broadcom.com
*/
#ifndef _DYNAMIC_METERS_H_
#define _DYNAMIC_METERS_H_

void dynamic_meters_init(bdmf_object_handle cpu_obj, int watch_qid);
void dynamic_meters_uninit(bdmf_object_handle cpu_obj);

#endif
