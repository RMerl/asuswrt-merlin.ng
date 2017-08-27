/*
   <:copyright-BRCM:2015:DUAL/GPL:standard
   
      Copyright (c) 2015 Broadcom 
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
 *  Created on: Jan/2016
 *      Author: ido@broadcom.com
 */

#ifndef _PTP_1588_H_
#define _PTP_1588_H_

#ifdef CONFIG_BCM_PTP_1588
int ptp_1588_init(void);
void ptp_1588_uninit(void);
/* store the timestamp of a ptp 1588 rx pkt */
int ptp_1588_rx_pkt_store_timestamp(unsigned char *pBuf, int len, uint16_t ptp_index);
/* parse the packet and check if it is ptp 1588 */
int is_pkt_ptp_1588(pNBuff_t pNBuff, char **ptp_offset);
/* add the packet to a list which will sent the packet to  rdpa_cpu_send_sysb */
int ptp_1588_cpu_send_sysb(bdmf_sysb sysb, rdpa_cpu_tx_info_t *info, char *ptp_header);
#endif

#endif

