/*
<:copyright-BRCM:2011:DUAL/GPL:standard

   Copyright (c) 2011 Broadcom 
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
/**************************************************************************
 * File Name  : xtmrt_dbg.h
 *
 * Description: This file contains constant definitions and debug structure
 * definitions and declarations of XTMRT common and feature specific debug API
 ***************************************************************************/
#ifndef __XTMRT_DBG_H_INCLUDED__
#define __XTMRT_DBG_H_INCLUDED__

#if defined(XTM_DEBUG_PHY_VECTOR_PKTS)
void dump_phy_error_vector_frame_info(struct sk_buff *skb);
#endif //XTM_DEBUG_PHY_VECTOR_PKTS

#endif //__XTMRT_DBG_H_INCLUDED___
