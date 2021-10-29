/*
* <:copyright-BRCM:2014:DUAL/GPL:standard
* 
*    Copyright (c) 2014 Broadcom 
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


#ifndef _RDPA_EGRESS_TM_BASIC_H_
#define _RDPA_EGRESS_TM_BASIC_H_

/** Priority for overall rate limiting */
typedef enum {
    rdpa_tm_orl_prty_low,       /**< Packets are counted. Excess packets are discarded */
    rdpa_tm_orl_prty_high,      /**< Packets are counted. Excess packets are transmitted */
} rdpa_tm_orl_prty;

#endif /* _RDPA_EGRESS_TM_H_ */
