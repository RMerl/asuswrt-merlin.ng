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

#ifndef _RDPA_DHD_HELPER_H_
#define _RDPA_DHD_HELPER_H_

#include "rdpa_types.h"
#include "rdpa_dhd_helper_basic.h"

/** \defgroup dhd_helper DHD Helper 
 * Objects in this group control DHD offload configuration
 */

/** Send wakeup to Runner upon TX/RX complete
  */
void rdpa_dhd_helper_complete_wakeup(uint32_t radio_idx, bdmf_boolean is_tx_complete);

/** Send packet to dongle 
 *
 * \param[in]   data        Packet data
 * \param[in]   length      Packet length
 * \param[in]   is_bpm      Additional TX post info
 * \return 0=OK or int error code\n
 */
int rdpa_dhd_helper_send_packet_to_dongle(void *data, uint32_t length, const rdpa_dhd_tx_post_info_t *info);

/** Clear dooorbell interrupt, to be invoked from ISR */
void rdpa_dhd_helper_doorbell_interrupt_clear(uint32_t radio_idx);

/** Return Runner wakeup information */
void rdpa_dhd_helper_wakeup_information_get(rdpa_dhd_wakeup_info_t *wakeup_info);

/** Create DHD Complete ring from Runner to CPU */
int rdpa_dhd_helper_dhd_complete_ring_create(uint32_t radio_idx, uint32_t ring_size);

/** Destroy DHD Complete ring from Runner to CPU */
int rdpa_dhd_helper_dhd_complete_ring_destroy(uint32_t radio_idx, uint32_t ring_size);

/** Return DHD Complete ring entry */
int rdpa_dhd_helper_dhd_complete_message_get(rdpa_dhd_complete_data_t *dhd_complete_info);

/** @} end of dhd_heler Doxygen group */

#endif /* _RDPA_DHD_HELPER_H_ */
