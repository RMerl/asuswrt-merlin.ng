/*
<:copyright-broadcom 
 
 Copyright (c) 2007 Broadcom Corporation 
 All Rights Reserved 
 No portions of this material may be reproduced in any form without the 
 written permission of: 
          Broadcom Corporation 
          5300 California Avenue
          Irvine, California 92617 
 All information contained in this document is Broadcom Corporation 
 company private, proprietary, and trade secret. 
 
:>
*/
/***************************************************************************
 * File Name  : MoCABlock.h
 *
 * Description: This file contains definitions for the MoCA Block for the
 *              BCM6816 and the BCM3450 chipset(s).
 ***************************************************************************/

#if !defined(_MoCABLOCK_H_)
#define _MoCABLOCK_H_

/* BCM 96816 related definitions . */
#define MoCA_INTERRUPT_DISABLE               0x0
#define MoCA_INTERRUPT_ENABLE                0x1

/* Definitions for coreInterrupts from host */
#define MoCA_HOST_RESP_TO_CORE               0x1
#define MoCA_HOST_REQ_TO_CORE                0x2

/* Definitions for hostInterrupts from core */
#define MoCA_CORE_RESP_TO_HOST               0x1
#define MoCA_CORE_REQ_TO_HOST                0x2
#define MoCA_CORE_ASSERT_TO_HOST             0x4
#define MoCA_CORE_UNUSED                     0xFC
#define MoCA_CORE_DDR_START                  0x100
#define MoCA_CORE_DDR_END                    0x200

#define MoCA_LED_LINK_ON_ACTIVE_OFF          0x0
#define MoCA_LED_LINK_OFF_ACTIVE_OFF         0x1
#define MoCA_LED_LINK_ON_ACTIVE_ON           0x2
#define MoCA_LED_LINK_OFF_ACTIVE_ON          0x3 /* NW Search and New Node Admission */
typedef struct _MoCAExtras {
   UINT32 host2MoCAIntEn ;
   UINT32 host2MoCAIntTrig ;
   UINT32 host2MoCAIntStatus ;
   UINT32 MoCA2HostIntEn ;
   UINT32 MoCA2HostIntTrig ;
   UINT32 MoCA2HostIntStatus ;
   UINT32 genPurpose0 ;
   UINT32 genPurpose1 ;
   UINT32 sideBandGmiiFC ;
   UINT32 leds ;
   UINT32 MoCAStatus ;
   UINT32 testMuxSel ;
   UINT32 mdCtrl ;
   UINT32 mdcDivider ;
   UINT32 outRefIntR01 ;
   UINT32 outRefIntR02 ;
   UINT32 outRefIntR03 ;
   UINT32 outRefIntR04 ;
   UINT32 outRefIntR05 ;
   UINT32 outRefIntSel ;
} MoCAExtras ;


typedef struct _MoCAMACRegs {
   UINT32 macCtrl ;           // 0x00
#define MoCA_MAC_REGS_MAC_CTRL_MAC_ENABLE    0x00000001
   UINT32 resv1 [7] ;
   UINT32 frmHdr  ;           // 0x20
   UINT32 msduHdr ;           // 0x24
   UINT32 resv2 [2] ;
   UINT32 macStatus ;         // 0x30
   UINT32 macStatusEn ;       // 0x34
   UINT32 resv3 [2] ;
   UINT32 netTimer ;          // 0x40
   UINT32 maxNetTimerCorr ;   // 0x44
   UINT32 timerCorrCtrl ;     // 0x48
   UINT32 bitParams ;         // 0x4c
   UINT32 fineCorr ;          // 0x50
   UINT32 coarseCorr ;        // 0x54
   UINT32 loadTimer1 ;        // 0x58
   UINT32 loadTimer2 ;        // 0x5c
   UINT32 mpiConfigCtrl ;     // 0x60
#define MoCA_MAC_REGS_MPI_CONFIG_CTRL_READ 			0x00000001
#define MoCA_MAC_REGS_MPI_CONFIG_CTRL_WRITE			0x00000002
   UINT32 mpiConfigAddr ;     // 0x64
   UINT32 mpiConfigDataW ;    // 0x68
   UINT32 mpiConfigDataR ;    // 0x6c
} MoCA_MAC_REGST, *PMoCA_MAC_REGST ;

#define MoCA_BLOCK_BASE                                          MOCA_MEM_BASE
typedef struct MoCABlockT {

   UINT8       dataMem [0x3FFFC] ;
   UINT8       resvd [0x61403] ;
   MoCAExtras  extras ;                                           // 0xb0da1400.
   //MoCAHostM2M follows.
   //MoCAMoCAM2M follows.
} MoCA_BLOCKT, *PMoCA_BLOCKT ;

#define MoCA_PHYS_IO_BASE								NONCACHE_TO_PHYS(MOCA_IO_BASE)
#define MoCA_BLOCK_MAC_REGS_START					(MOCA_IO_BASE+0x400)
#define MoCA_BLOCK_PHY_START                    MoCA_PHYS_IO_BASE+0x8000
#define MoCA_BLOCK_PHY_END                      MoCA_PHYS_IO_BASE+0xA3FF
#define MoCA_BLOCK ((volatile PMoCA_BLOCKT const) MoCA_BLOCK_BASE)
#define MoCA_MAC_REGS ((volatile PMoCA_MAC_REGST const) MoCA_BLOCK_MAC_REGS_START)
#define MoCA_CORE_MEM_BASE                      MoCA_BLOCK->dataMem
#define MoCA_MAIL_BOX_ADDR_REG                  MoCA_BLOCK->extras.genPurpose0
#define MoCA_IQ_SNR_ADDR_REG                    MoCA_BLOCK->extras.genPurpose1

/* BCM 93450 related definitions . */

#define BCM3450_I2C_CHIP_ADDRESS    0x70
typedef struct _Bcm3450Reg {
   UINT32   ChipId;           /* 0x0 */
   UINT32   ChipRev;          /* 0x4 */
   UINT32   Test;             /* 0x8 */
   UINT32   SerialCtl;        /* 0xc */
   UINT32   StatusRead;       /* 0x10 */
   UINT32   LnaCntl;          /* 0x14 */
   UINT32   PaCntl;           /* 0x18 */
#define BCM3450_PACNTL_PA_RDEG_SHIFT           11
#define BCM3450_PACNTL_PA_RDEG_MASK            0x00007800
#define BCM3450_PACNTL_PA_CURR_CONT_SHIFT      5
#define BCM3450_PACNTL_PA_CURR_CONT_MASK       0x000007E0
#define BCM3450_PACNTL_PA_CURR_FOLLOWER_SHIFT  2
#define BCM3450_PACNTL_PA_CURR_FOLLOWER_MASK   0x0000001C
#define BCM3450_PACNTL_PA_PWRDWN_SHIFT         0
#define BCM3450_PACNTL_PA_PWRDWN_MASK          0x00000001
#define BCM3450_PACNTL_OFFSET          		  0x18
   UINT32   Misc;             /* 0x1c */
#define BCM3452_MISC_BG_PWRDWN_SHIFT 15
#define BCM3452_MISC_BG_PWRDWN_MASK  0x00008000
#define BCM3450_MISC_IIC_RESET       0x1
#define BCM3450_MISC_SERIAL_RESET    0x2
#define BCM3450_MISC_OFFSET          0x1c
} Bcm3450Reg ;

#endif /* _MoCABLOCK_H_ */
