/*
<:copyright-BRCM:2011:DUAL/GPL:standard

   Copyright (c) 2011 Broadcom 
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
