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

#ifndef _RDPA_TCONT_H_
#define _RDPA_TCONT_H_

/**
 * \defgroup tcont T-CONT Management
 * \ingroup xgpon
 * @{
 */
#define RDPA_MAX_TCONT  32      /**< Max number of T-CONTs */

uint8_t tcont_tc_table_get(bdmf_object_handle tcont);
uint8_t tcont_pbit_table_get(bdmf_object_handle tcont);

/** @} end of tcont Doxygen group */

int rdpa_tcont_sr_dba_callback(uint32_t tcont_id, uint32_t *runner_ddr_occupancy);

#endif /* _RDPA_TCONT_H_ */
