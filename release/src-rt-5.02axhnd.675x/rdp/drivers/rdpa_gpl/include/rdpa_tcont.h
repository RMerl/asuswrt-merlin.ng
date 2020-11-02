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

#ifndef _RDPA_TCONT_H_
#define _RDPA_TCONT_H_

/**
 * \defgroup tcont T-CONT Management
 * \ingroup xgpon
 * @{
 */
#if defined(XRDP) && !defined(BCM63158)
#define RDPA_MAX_TCONT  33      /**< Max number of T-CONTs*/
#else
#define RDPA_MAX_TCONT  32      /**< Max number of T-CONTs */
#endif

uint8_t tcont_tc_table_get(bdmf_object_handle tcont);
uint8_t tcont_pbit_table_get(bdmf_object_handle tcont);

/** @} end of tcont Doxygen group */

int rdpa_tcont_sr_dba_callback(uint32_t tcont_id, uint32_t *runner_ddr_occupancy);

#endif /* _RDPA_TCONT_H_ */
