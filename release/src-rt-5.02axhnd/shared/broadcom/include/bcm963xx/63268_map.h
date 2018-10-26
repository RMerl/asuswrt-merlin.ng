/*
   Copyright (c) 2011-2012 Broadcom Corporation
   All Rights Reserved

<:label-BRCM:2011:proprietary:standard

 This program is the proprietary software of Broadcom and/or its
 licensors, and may only be used, duplicated, modified or distributed pursuant
 to the terms and conditions of a separate, written license agreement executed
 between you and Broadcom (an "Authorized License").  Except as set forth in
 an Authorized License, Broadcom grants no license (express or implied), right
 to use, or waiver of any kind with respect to the Software, and Broadcom
 expressly reserves all rights in and to the Software and all intellectual
 property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
 NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
 BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.

 Except as expressly set forth in the Authorized License,

 1. This program, including its structure, sequence and organization,
    constitutes the valuable trade secrets of Broadcom, and you shall use
    all reasonable efforts to protect the confidentiality thereof, and to
    use this information only in connection with your use of Broadcom
    integrated circuit products.

 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
    RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
    ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
    FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
    COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
    TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
    PERFORMANCE OF THE SOFTWARE.

 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
    ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
    INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
    WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
    IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
    OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
    SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
    SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
    LIMITED REMEDY.
:>
*/
/***********************************************************************/
/*                                                                     */
/*   MODULE:  63268_map.h                                              */
/*   DATE:    11/09/10                                                 */
/*   PURPOSE: Define the proprietary hardware blocks/subblocks for     */
/*            BCM63268                                                 */
/*                                                                     */
/***********************************************************************/
#ifndef __BCM63268_MAP_H
#define __BCM63268_MAP_H

#ifdef __cplusplus
extern "C" {
#endif


#if defined(__KERNEL__) || defined (_CFE_) || defined(FAP_4KE)
/* Access to registers from kernelspace */
#define REG_BASE                 0xb0000000
#else
/* Access to registers from userspace, see bcm_mmap.h for api */
#define REG_BASE                 (bcm_mmap_info.mmap_addr)
#define BCM_MMAP_INFO_BASE       0x10000000
#define BCM_MMAP_INFO_SIZE       0x01000000
#endif


#include "bcmtypes.h"
#include "63268_common.h"
#include "63268_intr.h"
#include "63268_map_part.h"

/* define proprietary block here . non-proprietary blocks in xxxx_map_part.h */
#define NAND_INTR_BASE              (REG_BASE + 0x000000b0)  /* nand interrupt control */
#define VDSL_CTRL_BASE              (REG_BASE + 0x00001800)
#define UBUS_STAT_BASE              (REG_BASE + 0x00001a00)
#define USB_CTL_BASE                (REG_BASE + 0x00002400)  /* USB 2.0 device control */
#define IPSEC_RXBUF_CMD             (REG_BASE + 0x0000287c)
#define DECT_SHIM_CTRL_BASE         (REG_BASE + 0x0000a000)
#define DECT_SHIM_DMA_CTRL_BASE     (REG_BASE + 0x0000a050)
#define DECT_SHIM_TEST_BASE         (REG_BASE + 0x0000a0f0)
#define DECT_APB_REG_BASE           (REG_BASE + 0x0000a800)
#define PCM_BASE                    (REG_BASE + 0x0000b000)  /* PCM control registers */
#define PCM_DMA_BASE                (REG_BASE + 0x0000b800)  /* PCM UIDMA register base */
#define USB_DMA_BASE                (REG_BASE + 0x0000c800)  /* USB 2.0 device DMA */
#define DECT_AHB_SHARED_RAM_BASE    (REG_BASE + 0x006f0000)
#define DECT_AHB_REG_BASE           (REG_BASE + 0x006f7f80)
#define PCIE_MEM1_BASE              (REG_BASE + 0x00f00000)

#define FAP0_CNTRL_BASE             (REG_BASE + 0x00801000)
#define FAP0_IN_FIFO_BASE           (REG_BASE + 0x00801100)
#define FAP0_OUT_FIFO_BASE          (REG_BASE + 0x00801200)
#define FAP0_DMA0_BASE              (REG_BASE + 0x00801300)
#define FAP0_DMA1_BASE              (REG_BASE + 0x00801320)
#define FAP0_TOKEN_BASE             (REG_BASE + 0x00801400)
#define FAP0_MSG_BASE               (REG_BASE + 0x00801700)
#define FAP0_DQM_BASE               (REG_BASE + 0x00801800)
#define FAP0_DQM_CNTRL_BASE         (REG_BASE + 0x00801a00)
#define FAP0_DQM_DATA_BASE          (REG_BASE + 0x00801c00)
#define FAP0_DQM_MIB_BASE           (REG_BASE + 0x00802000)
#define FAP0_SMISB                  (REG_BASE + 0x00c00000)

#define FAP1_CNTRL_BASE             (REG_BASE + 0x00a01000)
#define FAP1_IN_FIFO_BASE           (REG_BASE + 0x00a01100)
#define FAP1_OUT_FIFO_BASE          (REG_BASE + 0x00a01200)
#define FAP1_DMA0_BASE              (REG_BASE + 0x00a01300)
#define FAP1_DMA1_BASE              (REG_BASE + 0x00a01320)
#define FAP1_TOKEN_BASE             (REG_BASE + 0x00a01400)
#define FAP1_MSG_BASE               (REG_BASE + 0x00a01700)
#define FAP1_DQM_BASE               (REG_BASE + 0x00a01800)
#define FAP1_DQM_CNTRL_BASE         (REG_BASE + 0x00a01a00)
#define FAP1_DQM_DATA_BASE          (REG_BASE + 0x00a01c00)
#define FAP1_DQM_MIB_BASE           (REG_BASE + 0x00a02000)
#define FAP1_SMISB                  (REG_BASE + 0x00e00000)

#ifndef __ASSEMBLER__

#if defined(__KERNEL__) && !defined(MODULE)
#error "PRIVATE FILE INCLUDED IN KERNEL"
#endif

/* macro to convert logical data addresses to physical */
/* DMA hardware must see physical address */
#define LtoP( x )       ( (uint32)x & 0x1fffffff )
#define PtoL( x )       ( LtoP(x) | 0xa0000000 )

/*
** OTP
*/
typedef struct Otp {
    uint32      Config;                 /* 0x0 */
    uint32      Control;                /* 0x4 */
    uint32      Status;                 /* 0x8 */
    uint32      Addr;                   /* 0xc */
    uint32      WriteData;              /* 0x10 */
    uint32      DOut;                   /* 0x14 */
    uint32      unused0[2];             /* 0x18 - 0x1c */
    uint32      SecureBootConfig;       /* 0x20 */
    uint32      unused1[7];             /* 0x24 - 0x3c */
    uint32      BrcmCtrl[4];            /* 0x40 - 0x4c */
/* bits are relative to LSB of BrcmCtrl[0] */
#define OTP_WLAN_DISABLE                25 
#define OTP_SUPPORT_STBC                26 
#define OTP_DECT_DISABLE                29
#define OTP_IPSEC_DISABLE               30
    uint32      BrcmSec[2];            /* 0x50 - 0x54 */
    uint32      unused2[2];            /* 0x58 - 0x5c */
    uint32      CustCtrl[2];           /* 0x60 - 0x64 */
    uint32      CustSec[4];            /* 0x68 - 0x74 */
    uint32      unused3[22];           /* 0x78 - 0xcc */
    uint32      RAMRepair[24];         /* 0xd0 - 0x12c */
} Otp;

#define OTP ((volatile Otp * const) OTP_BASE)

/* This is now a misnomer, but maintains compatibility with 6362 */
/* OTP_W4 values actually show up in OTP_BRCM_CTRL_0 */
#define OTP_GET_USER_BIT(x)             ((OTP->BrcmCtrl[((x)/32)] >> ((x) % 32)) & 1)


/*
** USB 2.0 Device Registers
*/
typedef struct UsbRegisters {
#define USBD_CONTROL_APP_DONECSR                 0x0001
#define USBD_CONTROL_APP_RESUME                  0x0002
#define USBD_CONTROL_APP_RXFIFIO_INIT            0x0040
#define USBD_CONTROL_APP_TXFIFIO_INIT            0x0080
#define USBD_CONTROL_APP_FIFO_SEL_SHIFT          0x8
#define USBD_CONTROL_APP_FIFO_INIT_SEL(x)        (((x)&0x0f)<<USBD_CONTROL_APP_FIFO_SEL_SHIFT)
#define USBD_CONTROL_APP_AUTO_CSRS               0x2000
#define USBD_CONTROL_APP_AUTO_INS_ZERO_LEN_PKT   0x4000
#define EN_TXZLENINS  (1<<14)
#define EN_RXZSCFG   (1<<12)
#define APPSETUPERRLOCK  (1<<5)
    uint32 usbd_control ;
#define USBD_STRAPS_APP_SELF_PWR    0x0400
#define USBD_STRAPS_APP_DEV_DISCON  0x0200
#define USBD_STRAPS_APP_CSRPRG_SUP  0x0100
#define USBD_STRAPS_APP_RAM_IF      0x0080
#define USBD_STRAPS_APP_DEV_RMTWKUP 0x0040
#define USBD_STRAPS_APP_PHYIF_8BIT  0x0004
#define USBD_STRAPS_FULL_SPEED      0x0003
#define USBD_STRAPS_LOW_SPEED       0x0002
#define USBD_STRAPS_HIGH_SPEED      0x0000
#define APPUTMIDIR(x)  ((x&1)<<3)
#define UNIDIR   0
    uint32 usbd_straps;
#define USB_ENDPOINT_0  0x01
    uint32 usbd_stall;
#define USBD_ENUM_SPEED_SHIFT       12
#define USBD_ENUM_SPEED             0x3000
#define UDC20_ALTINTF(x) ((x>>8)&0xf)
#define UDC20_INTF(x)  ((x>>4)&0xf)
#define UDC20_CFG(x)  ((x>>0)&0xf)
    uint32 usbd_status;
#define USBD_LINK                   (0x1<<10)
#define USBD_SET_CSRS               0x40
#define USBD_SUSPEND                0x20
#define USBD_EARLY_SUSPEND          0x10
#define USBD_SOF                    0x08
#define USBD_ENUMON                 0x04
#define USBD_SETUP                  0x02
#define USBD_USBRESET               0x01
    uint32 usbd_events;
    uint32 usbd_events_irq;
#define UPPER(x)   (16+x)
#define ENABLE(x)   (1<<x)
#define SWP_TXBSY   (15)
#define SWP_RXBSY   (14)
#define SETUP_ERR   (13)
#define APPUDCSTALLCHG  (12)
#define BUS_ERR    (11)
#define USB_LINK                                (10)
#define HST_SETCFG   (9)
#define HST_SETINTF   (8)
#define ERRATIC_ERR   (7)
#define SET_CSRS   (6)
#define SUSPEND    (5)
#define ERLY_SUSPEND  (4)
#define SOF     (3)
#define ENUM_ON    (2)
#define SETUP    (1)
#define USB_RESET   (0)
#define RISING(x)   (0x0<<2*x)
#define FALLING(x)   (0x1<<2*x)
#define USBD_IRQCFG_ENUM_ON_FALLING_EDGE 0x00000010
    uint32 usbd_irqcfg_hi ;
    uint32 usbd_irqcfg_lo ;
#define USBD_USB_RESET_IRQ          0x00000001
#define USBD_USB_SETUP_IRQ          0x00000002 // non-standard setup cmd rcvd
#define USBD_USB_ENUM_ON_IRQ        0x00000004
#define USBD_USB_SOF_IRQ            0x00000008
#define USBD_USB_EARLY_SUSPEND_IRQ  0x00000010
#define USBD_USB_SUSPEND_IRQ        0x00000020 // non-standard setup cmd rcvd
#define USBD_USB_SET_CSRS_IRQ       0x00000040
#define USBD_USB_ERRATIC_ERR_IRQ    0x00000080
#define USBD_USB_SETCFG_IRQ         0x00000200
#define USBD_USB_LINK_IRQ                       0x00000400
    uint32 usbd_events_irq_mask;
    uint32 usbd_swcfg;
    uint32 usbd_swtxctl;
    uint32 usbd_swrxctl;
    uint32 usbd_txfifo_rwptr;
    uint32 usbd_rxfifo_rwptr;
    uint32 usbd_txfifo_st_rwptr;
    uint32 usbd_rxfifo_st_rwptr;
    uint32 usbd_txfifo_config ;
    uint32 usbd_rxfifo_config ;
    uint32 usbd_txfifo_epsize ;
    uint32 usbd_rxfifo_epsize ;
#define USBD_EPNUM_CTRL             0x0
#define USBD_EPNUM_ISO              0x1
#define USBD_EPNUM_BULK             0x2
#define USBD_EPNUM_IRQ              0x3
#define USBD_EPNUM_EPTYPE(x)        (((x)&0x3)<<8)
#define USBD_EPNUM_EPDMACHMAP(x)    (((x)&0xf)<<0)
    uint32 usbd_epnum_typemap ;
    uint32 usbd_reserved [0xB] ;
    uint32 usbd_csr_setupaddr ;
#define USBD_EPNUM_MASK             0xf
#define USBD_EPNUM(x)               ((x&USBD_EPNUM_MASK)<<0)
#define USBD_EPDIR_IN               (1<<4)
#define USBD_EPDIR_OUT              (0<<4)
#define USBD_EPTYP_CTRL             (USBD_EPNUM_CTRL<<5)
#define USBD_EPTYP_ISO              (USBD_EPNUM_ISO<<5)
#define USBD_EPTYP_BULK             (USBD_EPNUM_BULK<<5)
#define USBD_EPTYP_IRQ              (USBD_EPNUM_IRQ<<5)
#define USBD_EPCFG_MASK             0xf
#define USBD_EPCFG(x)               ((x&USBD_EPCFG_MASK)<<7)
#define USBD_EPINTF_MASK            0xf
#define USBD_EPINTF(x)              ((x&USBD_EPINTF_MASK)<<11)
#define USBD_EPAINTF_MASK           0xf
#define USBD_EPAINTF(x)             ((x&USBD_EPAINTF_MASK)<<15)
#define USBD_EPMAXPKT_MSK           0x7ff
#define USBD_EPMAXPKT(x)            ((x&USBD_EPMAXPKT_MSK)<<19)
#define USBD_EPISOPID_MASK          0x3
#define USBD_EPISOPID(x)            ((x&USBD_ISOPID_MASK)<<30)
    uint32 usbd_csr_ep [5] ;
} UsbRegisters;

#define USB ((volatile UsbRegisters * const) USB_CTL_BASE)



/*
** NAND Interrupt Controller Registers
*/
typedef struct NandIntrCtrlRegs {
    uint32 NandInterrupt;
#define NINT_ENABLE_MASK    0xffff0000
#define NINT_STS_MASK       0x00000fff
#define NINT_ECC_ERROR_CORR 0x00000080
#define NINT_ECC_ERROR_UNC  0x00000040
#define NINT_DEV_RBPIN      0x00000020
#define NINT_CTRL_READY     0x00000010
#define NINT_PAGE_PGM       0x00000008
#define NINT_COPY_BACK      0x00000004
#define NINT_BLOCK_ERASE    0x00000002
#define NINT_NP_READ        0x00000001

    uint32 NandBaseAddr0;   /* Default address when booting from NAND flash */
    uint32 reserved;
    uint32 NandBaseAddr1;   /* Secondary base address for NAND flash */
} NandIntrCtrlRegs;

#define NAND_INTR ((volatile NandIntrCtrlRegs * const) NAND_INTR_BASE)


#define NAND_CACHE ((volatile uint8 * const) NAND_CACHE_BASE)


typedef struct ahbRegisters
{
   uint16 dsp_main_sync0;     /* 0xb0e57f80 DSP main counter outputs sel reg 0 */
   uint16 dsp_main_sync1;     /* 0xb0e57f82 DSP main counter outputs sel reg 1 */
   uint16 dsp_main_cnt;       /* 0xb0e57f84 DSP main counter reg */
   uint16 reserved1;          /* 0xb0e57f86 Reserved */
   uint16 reserved2;          /* 0xb0e57f88 Reserved */
   uint16 reserved3;          /* 0xb0e57f8a Reserved */
   uint16 reserved4;          /* 0xb0e57f8c Reserved */
   uint16 reserved5;          /* 0xb0e57f8e Reserved */
   uint16 reserved6;          /* 0xb0e57f90 Reserved */
   uint16 dsp_ram_out0;       /* 0xb0e57f92 DSP RAM output register 0 */
   uint16 dsp_ram_out1;       /* 0xb0e57f94 DSP RAM output register 1 */
   uint16 dsp_ram_out2;       /* 0xb0e57f96 DSP RAM output register 2 */
   uint16 dsp_ram_out3;       /* 0xb0e57f98 DSP RAM output register 3 */
   uint16 dsp_ram_in0;        /* 0xb0e57f9a DSP RAM input register 0 */
   uint16 dsp_ram_in1;        /* 0xb0e57f9c DSP RAM input register 1 */
   uint16 dsp_ram_in2;        /* 0xb0e57f9e DSP RAM input register 2 */
   uint16 dsp_ram_in3;        /* 0xb0e57fa0 DSP RAM input register 3 */
   uint16 dsp_zcross1_out;    /* 0xb0e57fa2 DSP RAM zero crossing 1 output reg */
   uint16 dsp_zcross2_out;    /* 0xb0e57fa4 DSP RAM zero crossing 2 output reg */
   uint16 reserved7;          /* 0xb0e57fa6 Reserved */
   uint16 reserved8;          /* 0xb0e57fa8 Reserved */
   uint16 reserved9;          /* 0xb0e57faa Reserved */
   uint16 reserved10;         /* 0xb0e57fac Reserved */
   uint16 reserved11;         /* 0xb0e57fae Reserved */
   uint16 reserved12;         /* 0xb0e57fb0 Reserved */
   uint16 reserved13;         /* 0xb0e57fb2 Reserved */
   uint16 reserved14;         /* 0xb0e57fb4 Reserved */
   uint16 reserved15;         /* 0xb0e57fb6 Reserved */
   uint16 reserved16;         /* 0xb0e57fb8 Reserved */
   uint16 reserved17;         /* 0xb0e57fba Reserved */
   uint16 dsp_main_ctrl;      /* 0xb0e57fbc DSP main counter control and preset reg */
   uint16 reserved18;         /* 0xb0e57fbe Reserved */
   uint16 reserved19;         /* 0xb0e57fc0 Reserved */
   uint16 reserved20;         /* 0xb0e57fc2 Reserved */
   uint16 reserved21;         /* 0xb0e57fc4 Reserved */
   uint16 reserved22;         /* 0xb0e57fc6 Reserved */
   uint16 reserved23;         /* 0xb0e57fc8 Reserved */
   uint16 reserved24;         /* 0xb0e57fca Reserved */
   uint16 reserved25;         /* 0xb0e57fce Reserved */
   uint16 dsp_ctrl;           /* 0xb0e57fd0 DSP control reg */
   uint16 dsp_pc;             /* 0xb0e57fd2 DSP program counter */
   uint16 dsp_pc_start;       /* 0xb0e57fd4 DSP program counter start */
   uint16 dsp_irq_start;      /* 0xb0e57fd6 DSP interrupt vector start */
   uint16 dsp_int;            /* 0xb0e57fd8 DSP to system bus interrupt vector */
   uint16 dsp_int_mask;       /* 0xb0e57fda DSP to system bus interrupt vector mask */
   uint16 dsp_int_prio1;      /* 0xb0e57fdc DSP interrupt mux 1 */
   uint16 dsp_int_prio2;      /* 0xb0e57fde DSP interrupt mux 2 */
   uint16 dsp_overflow;       /* 0xb0e57fe0 DSP to system bus interrupt overflow reg */
   uint16 dsp_jtbl_start;     /* 0xb0e57fe2 DSP jump table start address */
   uint16 reserved26;         /* 0xb0e57fe4 Reserved */
   uint16 reserved27;         /* 0xb0e57fe6 Reserved */
   uint16 reserved28;         /* 0xb0e57fe8 Reserved */
   uint16 reserved29;         /* 0xb0e57fea Reserved */
   uint16 reserved30;         /* 0xb0e57fec Reserved */
   uint16 reserved31;         /* 0xb0e57fee Reserved */
   uint16 dsp_debug_inst;     /* 0xb0e57ff0 DSP debug instruction register */
   uint16 reserved32;         /* 0xb0e57ff2 Reserved */
   uint16 dsp_debug_inout_l;  /* 0xb0e57ff4 DSP debug data (LSW) */
   uint16 dsp_debug_inout_h;  /* 0xb0e57ff6 DSP debug data (MSW) */
   uint16 reserved33;         /* 0xb0e57ff8 Reserved */
   uint16 reserved34;         /* 0xb0e57ffa Reserved */
   uint16 reserved35;         /* 0xb0e57ffc Reserved */
   uint16 reserved36;         /* 0xb0e57ffe Reserved */
} ahbRegisters;

#define AHB_REGISTERS ((volatile ahbRegisters * const) DECT_AHB_REG_BASE)

#endif

#ifdef __cplusplus
}
#endif

#endif

