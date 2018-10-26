/*
* <:copyright-BRCM:2014:DUAL/GPL:standard
* 
*    Copyright (c) 2014 Broadcom 
*    All Rights Reserved
* 
* Unless you and Broadcom execute a separate written software license
* agreement governing use of this software, this software is licensed
* to you under the terms of the GNU General Public License version 2
* (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
* with the following added to such license:
* 
*    As a special exception, the copyright holders of this software give
*    you permission to link this software with independent modules, and
*    to copy and distribute the resulting executable under terms of your
*    choice, provided that you also meet, for each linked independent
*    module, the terms and conditions of the license of that module.
*    An independent module is a module which is not derived from this
*    software.  The special exception does not apply to any modifications
*    of the software.
* 
* Not withstanding the above, under no circumstances may you combine
* this software in any way with any other Broadcom software provided
* under a license other than the GPL, without Broadcom's express prior
* written consent.
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
