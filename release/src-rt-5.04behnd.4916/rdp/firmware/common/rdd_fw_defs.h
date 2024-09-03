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
#define    RDP_GEN_60      60 /* BCM63146_A0, BCM4912_A0 */
#define    RDP_GEN_61      61 /* BCM6855, BCM63146_B0, BCM4912_B0, BCM6813 */
#define    RDP_GEN_62      62 /* BCM6888, BCM6837 */

#define CHIP_VER_USE 1

#define DYNAMIC_SECTION_SIZE_PER_TASK   32
#define RESIDUE_NUMBER_OF_CYCLES        16
#define RESIDUE_NUMBER_OF_CYCLES_BIT    4

/*
 * cpu_tx ring_pd has 14bit packet length field, but max length possible is 10KByte
 * use an invalid value CPU_TX_SG_PACKET_TYPE_LEN to indicate SG frag packet
 */
#define CPU_TX_SG_PACKET_TYPE_LEN  0x3FFF

#endif
