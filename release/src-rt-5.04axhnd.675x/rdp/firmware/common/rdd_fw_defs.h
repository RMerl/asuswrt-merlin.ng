/*
    <:copyright-BRCM:2014-2016:DUAL/GPL:standard
    
       Copyright (c) 2014-2016 Broadcom 
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




