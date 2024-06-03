/*
<:copyright-BRCM:2013:DUAL/GPL:standard

   Copyright (c) 2013 Broadcom 
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

#ifndef __BCM63138_CPU_H
#define __BCM63138_CPU_H
// FIXME!! this file is dummy file to get the BSP compilation passes.
// Need a lot of modifications

#ifdef __cplusplus
extern "C" {
#endif

/*
#************************************************************************
#* Coprocessor 0 Register Names
#************************************************************************
*/
#define C0_BCM_CONFIG          $22

/*
# Select 1
#  Bit  31:   unused
#  Bits 30:25 MMU Size (Num TLB entries-1)
#  Bits 24:22 ICache sets/way (2^n * 64)
#  Bits 21:19 ICache Line size (2^(n+1) bytes) 0=No Icache
#  Bits 18:16 ICache Associativity (n+1) way                    
#  Bits 15:13 DCache sets/way (2^n * 64)
#  Bits 12:10 DCache Line size (2^(n+1) bytes) 0=No Dcache
#  Bits 9:7   DCache Associativity (n+1) way                    
#  Bits 6:4   unused
#  Bit  3:    1=At least 1 watch register
#  Bit  2:    1=MIPS16 code compression implemented
#  Bit  1:    1=EJTAG implemented                   
#  Bit  0:    1=FPU implemented                   
*/
#define CP0_CFG_ISMSK      (0x7 << 22)
#define CP0_CFG_ISSHF      22
#define CP0_CFG_ILMSK      (0x7 << 19)
#define CP0_CFG_ILSHF      19
#define CP0_CFG_IAMSK      (0x7 << 16)
#define CP0_CFG_IASHF      16
#define CP0_CFG_DSMSK      (0x7 << 13)
#define CP0_CFG_DSSHF      13
#define CP0_CFG_DLMSK      (0x7 << 10)
#define CP0_CFG_DLSHF      10
#define CP0_CFG_DAMSK      (0x7 << 7)
#define CP0_CFG_DASHF      7

/*
#************************************************************************
#* Coprocessor 0 Broadcom Config Register Bits
#************************************************************************
*/
#define CP0_BCM_CFG_ICSHEN          (0x1 << 31)
#define CP0_BCM_CFG_DCSHEN          (0x1 << 30)
#define CP0_BCM_CFG_TLBPD           (0x1 << 28)
#define CP0_BCM_CFG_BTHD            (0x1 << 21)
#define CP0_BCM_CFG_CLF             (0x1 << 20)
#define CP0_BCM_CFG_NBK             (0x1 << 17)

/*
#************************************************************************
#* Coprocessor 0 CMT Interrupt Register
#************************************************************************
*/
#define CP0_CMT_XIR_4               (0x1 << 31)
#define CP0_CMT_XIR_3               (0x1 << 30)
#define CP0_CMT_XIR_2               (0x1 << 29)
#define CP0_CMT_XIR_1               (0x1 << 28)
#define CP0_CMT_XIR_0               (0x1 << 27)
#define CP0_CMT_SIR_1               (0x1 << 16)
#define CP0_CMT_SIR_0               (0x1 << 15)
#define CP0_CMT_NMIR_TP1            (0x1 << 1)
#define CP0_CMT_NMIR_TP0            (0x1 << 0)

/*
#************************************************************************
#* Coprocessor 0 CMT Control Register
#************************************************************************
*/
#define CP0_CMT_DSU_TP1             (0x1 << 30)
#define CP0_CMT_TPS_SHFT            16
#define CP0_CMT_TPS_MASK            (0xF << CP0_CMT_TPS_SHFT)
#define CP0_CMT_PRIO_TP1            (0x1 << 5)
#define CP0_CMT_PRIO_TP0            (0x1 << 4)
#define CP0_CMT_RSTSE               (0x1 << 0)

/*
#************************************************************************
#* Coprocessor 0 CMT Local Register
#************************************************************************
*/
#define CP0_CMT_TPID                (0x1 << 31)

/*
#************************************************************************
#* MIPS Registers
#************************************************************************
*/

#define MIPS_BASE_BOOT  0xbfa00000
#define MIPS_BASE       0xff400000

#define MIPS_RAC_CR0    0x00        // RAC Configuration Register
#define MIPS_RAC_CR1    0x08        // RAC Configuration Register 1
#define RAC_FLH         (1 << 8)
#define RAC_DPF         (1 << 6)
#define RAC_NCH         (1 << 5)
#define RAC_C_INV       (1 << 4)
#define RAC_PF_D        (1 << 3)
#define RAC_PF_I        (1 << 2)
#define RAC_D           (1 << 1)
#define RAC_I           (1 << 0)

#define MIPS_RAC_ARR    0x04        // RAC Address Range Register
#define RAC_UPB_SHFT    16
#define RAC_LWB_SHFT    0

#define MIPS_LMB_CR     0x1C        // LMB Control Register
#define LMB_EN          (1 << 0)

#define MIPS_SBR        0x20        // System Base Register

#define MIPS_TP0_ALT_BV 0x30000
#define MIPS_TP1_ALT_BV 0x38000
#define ENABLE_ALT_BV   (1 << 19)

#ifdef __cplusplus
}
#endif

#endif
