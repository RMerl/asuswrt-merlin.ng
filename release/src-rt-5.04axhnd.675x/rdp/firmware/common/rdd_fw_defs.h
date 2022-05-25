/*
    <:copyright-BRCM:2014-2016:DUAL/GPL:standard
    
       Copyright (c) 2014-2016 Broadcom 
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

#ifndef _RDD_FW_DEFS_H
#define _RDD_FW_DEFS_H

/* chip version - should contain all chips increment by the chip order*/

#define    RDP_GEN_30      30 /* ALL RDP projects*/
#define    RDP_GEN_40      40 /* BCM6858 */
#define    RDP_GEN_41      41 /* BCM6846 */
#define    RDP_GEN_42      42 /* BCM63158 */
#define    RDP_GEN_43      43 /* BCM6856*/
#define    RDP_GEN_50      50 /* BCM6878 */
#define    RDP_GEN_60      60 /* BCM63146_A0, BCM4912 */
#define    RDP_GEN_61      61 /* BCM6855, BCM63146_B0 */

#if defined(OPERATION_MODE_PRV)
#define    RDD_LAN29_VPORT  RDD_VPORT_ID_30
#define    RDD_SYSTEM_VPORT RDD_LAN29_VPORT
#define    RDD_WAN1_VPORT   (RDD_LAN29_VPORT + 1)
#endif
#endif




