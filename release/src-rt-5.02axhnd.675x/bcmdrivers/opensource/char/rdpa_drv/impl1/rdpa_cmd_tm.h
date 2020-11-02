#ifndef __RDPA_CMD_TM_H_INCLUDED__
#define __RDPA_CMD_TM_H_INCLUDED__

/*
 <:copyright-BRCM:2007:DUAL/GPL:standard
 
    Copyright (c) 2007 Broadcom 
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

/*
 *******************************************************************************
 * File Name  : rdpa_cmd_tm.h
 *
 * Description: This file contains the PacketRunner Traffic Manager API.
 *
 *******************************************************************************
 */
int rdpa_cmd_tm_ioctl(unsigned long arg);
void rdpa_cmd_tm_init(void);
void rdpa_cmd_tm_exit(void);

/* rdpa_cmd_drv_get_q_size()
 * DESCRIPTION:
 *      This API returns the size of an egress queue.
 *
 * INPUT:
 *      dev_id: RDPA interface id (rdpa_if_wan0, rdpa_if_wan1, rdpa_if_lan0..rdpa_if_lan6
 *      q_id: The egress queue for which queue size is requested.
 *      dir: queue outgoing traffic direction (rdpa_dir_us, rdpa_dir_ds)
 *
 * OUTPUT:
 *      pq_size: queue size
 *
 * RETURN:
 *      0 if success, non-zero otherwise. queue size is valid only when the returned code is 0.
 */
extern int rdpa_cmd_drv_get_q_size(uint32_t dev_id, uint32_t q_id, uint32_t dir, uint32_t *pq_size);

/* rdpa_cmd_drv_get_q_occupancy()
 * DESCRIPTION:
 *      This API returns the current occupancy (fill level) of an egress queue.
 *
 * INPUT:
 *      dev_id: RDPA interface id (rdpa_if_wan0, rdpa_if_wan1, rdpa_if_lan0..rdpa_if_lan6
 *      q_id: The egress queue for which queue occupancy is requested.
 *      dir: queue outgoing traffic direction (rdpa_dir_us, rdpa_dir_ds)
 *
 * OUTPUT:
 *      pq_occupancy: queue occupancy
 *
 * RETURN:
 *      0 if success, non-zero otherwise. queue occupancy is valid only when the returned code is 0.
 */ 
extern int rdpa_cmd_drv_get_q_occupancy(uint32_t dev_id, uint32_t q_id, uint32_t dir, uint32_t *pq_occupancy);


#endif /* __RDPA_CMD_TM_H_INCLUDED__ */
