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
 * File Name  : xtmrt_iq.h
 *
 * Description: This file contains constant definitions and structure
 *              definitions for the BCM6368 ATM/PTM network device driver.
 ***************************************************************************/

#if (defined(CONFIG_BCM_INGQOS) || defined(CONFIG_BCM_INGQOS_MODULE))

#if !defined(_BCMXTMRTIQ_H)
#define _BCMXTMRTIQ_H

#include <linux/iqos.h>
#include "ingqos.h"


/**** Prototypes ****/
void xtm_iq_status(void);
void xtm_iq_update_cong_status(int chnl);
void xtm_rx_set_iq_thresh(int chnl);
void xtm_rx_init_iq_thresh(int chnl);

#if defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE)
void xtm_iq_dqm_update_cong_status(int chnl);
#endif

#endif /* _BCMXTMRTIQ_H */
#endif
