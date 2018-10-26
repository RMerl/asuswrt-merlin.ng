/*
<:copyright-BRCM:2012:proprietary:standard

   Copyright (c) 2012 Broadcom 
   All Rights Reserved

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
/*   MODULE:  6848_map.h                                               */
/*   DATE:    23/11/14                                                 */
/*   PURPOSE: Define the proprietary hardware blocks/subblocks for     */
/*            BCM6848                                                  */
/*                                                                     */
/***********************************************************************/
#ifndef __BCM6848_MAP_H
#define __BCM6848_MAP_H

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
#include "6848_common.h"
#include "6848_intr.h"
#include "6848_map_part.h"

/* define proprietary block here . non-proprietary blocks in xxxx_map_part.h */
#define BROM_BASE                   (REG_BASE + 0x00800600)  /* bootrom registers */
#define BROM_SEC_BASE               (REG_BASE + 0x00800620)  /* bootrom secure registers */

#ifndef __ASSEMBLER__

#if defined(__KERNEL__) && !defined(MODULE)
#error "PRIVATE FILE INCLUDED IN KERNEL"
#endif

/* macro to convert logical data addresses to physical */
/* DMA hardware must see physical address */
#define LtoP( x )       ( (uint32)x & 0x1fffffff )
#define PtoL( x )       ( LtoP(x) | 0xa0000000 )

/*
** OTP control registers 
*/
typedef struct Otp {
    uint32        GeneralCtl[5];
    uint32        GeneralSts[10];
} Otp;
#define OTP ((volatile Otp * const) JTAG_OTP_BASE)

typedef struct JtagOtp {
        uint32 ctrl0;           /* 0x00 */
#define JTAG_OTP_CTRL_BYPASS_OTP_CLK            (1 << 31)
#define JTAG_OTP_CTRL_READ_FOUT                 (1 << 30)
#define JTAG_OTP_CTRL_TESTCOL                   (1 << 29)
#define JTAG_OTP_CTRL_CPU_DEBUG_SEL             (0xf << 25)
#define JTAG_OTP_CTRL_BURST_UART_SEL            (1 << 24)
#define JTAG_OTP_CTRL_ACCESS_MODE               (0x3 << 22)
#define JTAG_OTP_CTRL_PROG_EN                   (1 << 21)
#define JTAG_OTP_CTRL_DEBUG_MDE                 (1 << 20)
#define JTAG_OTP_CTRL_WRP_CONTINUE_ON_FAIL      (1 << 19)
#define JTAG_OTP_CTRL_WRP_PROGRAM_VERIFY_FLAG   (1 << 18)
#define JTAG_OTP_CTRL_WRP_DOUBLE_WORD           (1 << 17)
#define JTAG_OTP_CTRL_WRP_REGC_SEL              (0x7 << 14)
#define JTAG_OTP_CTRL_WRP_QUADFUSE              (1 << 13)
#define JTAG_OTP_CTRL_WRP_DOUBLEFUSE            (1 << 12)
#define JTAG_OTP_CTRL_WRP_READ4X                (1 << 11)
#define JTAG_OTP_CTRL_WRP_READ2X                (1 << 10)
#define JTAG_OTP_CTRL_WRP_IN_DEBUG              (1 << 9)
#define JTAG_OTP_CTRL_COMMAND                   (0x1f << 1)
#define JTAG_OTP_CTRL_CMD_READ          (0x00 << 1)
#define JTAG_OTP_CTRL_CMD_READBURST     (0x01 << 1)
#define JTAG_OTP_CTRL_CMD_OTP_PROG_EN   (0x02 << 1)
#define JTAG_OTP_CTRL_CMD_OTP_PROG_DIS  (0x03 << 1)
#define JTAG_OTP_CTRL_CMD_PRESCREEN     (0x04 << 1)
#define JTAG_OTP_CTRL_CMD_PRESCREEN_RP  (0x05 << 1)
#define JTAG_OTP_CTRL_CMD_FLUSH         (0x06 << 1)
#define JTAG_OTP_CTRL_CMD_NOP           (0x07 << 1)
#define JTAG_OTP_CTRL_CMD_PROG          (0x0a << 1)
#define JTAG_OTP_CTRL_CMD_PROG_RP       (0x0b << 1)
#define JTAG_OTP_CTRL_CMD_PROG_OVST     (0x0c << 1)
#define JTAG_OTP_CTRL_CMD_RELOAD        (0x0d << 1)
#define JTAG_OTP_CTRL_CMD_ERASE         (0x0e << 1)
#define JTAG_OTP_CTRL_CMD_LOAD_RF       (0x0f << 1)
#define JTAG_OTP_CTRL_CMD_CTRL_WR       (0x10 << 1)
#define JTAG_OTP_CTRL_CMD_CTRL_RD       (0x11 << 1)
#define JTAG_OTP_CTRL_CMD_READ_HP       (0x12 << 1)
#define JTAG_OTP_CTRL_CMD_READ_OVST     (0x13 << 1)
#define JTAG_OTP_CTRL_CMD_READ_VERIFY0  (0x14 << 1)
#define JTAG_OTP_CTRL_CMD_READ_VERIFY1  (0x15 << 1)
#define JTAG_OTP_CTRL_CMD_READ_FORCE0   (0x16 << 1)
#define JTAG_OTP_CTRL_CMD_READ_FORCE1   (0x17 << 1)
#define JTAG_OTP_CTRL_CMD_BURNING       (0x18 << 1)
#define JTAG_OTP_CTRL_CMD_PROG_LOCK     (0x19 << 1)
#define JTAG_OTP_CTRL_CMD_PROG_TESTCOL  (0x1a << 1)
#define JTAG_OTP_CTRL_CMD_READ_TESTCOL  (0x1b << 1)
#define JTAG_OTP_CTRL_CMD_READ_FOUT     (0x1e << 1)
#define JTAG_OTP_CTRL_CMD_SFT_RESET     (0x1f << 1)

#define JTAG_OTP_CTRL_START                     (1 << 0)
        uint32 ctrl1;           /* 0x04 */
#define JTAG_OTP_CTRL_CPU_MODE                  (1 << 0)
        uint32 ctrl2;           /* 0x08 */
        uint32 ctrl3;           /* 0x0c */
        uint32 ctrl4;           /* 0x10 */
        uint32 status0;         /* 0x14 */
        uint32 status1;         /* 0x18 */
#define JTAG_OTP_STATUS_1_CMD_DONE	(1 << 1)
        uint32 status2;         /* 0x1c */
        uint32 status3;         /* 0x20 */
#define OTP_SAR_DISABLE       0
#define OTP_VDSL_DSLDISABLE   1
#define OTP_DIS2LINE          2
#define OTP_VDSL_DISVDSL      3
#define OTP_VDSL_DIS6BND      4
#define OTP_VDSL_DISRNC       5
        uint32 status4;         /* 0x24 */
#define OTP_SF2_DISABLE       32
#define OTP_PCIE_RCAL_VALID     33
        uint32 status5;         /* 0x28 */
#define OTP_SPHY_DISABLE      64
#define OTP_RNR_DISIPSEC      75
#define OTP_RNR_DISSHA1       76
#define OTP_RNR_DISSHA2       77
#define OTP_PRNR_DISAES256    78
#define OTP_PRNR_DISAES192    79
#define OTP_PRNR_DISAES128    80
#define OTP_USBH_DISEHCI      81
#define OTP_USBH_XHCIDIS      82
#define OTP_USBD_DISABLE      83
#define OTP_PCIE_DISABLE0     84
#define OTP_PCIE_DISABLE1     85
#define OTP_A9_DISC1          86
#define OTP_A9_DISNEON        87
#define OTP_SATA_DISABLE      88
#define OTP_DECT_DISABLE      89        
        uint32 status6;         /* 0x2c */
#define OTP_TBUS_DISABLE      120
#define OTP_PMC_BOOT          121       
        uint32 status7;         /* 0x30 */
#define OTP_PMC_BOOTROM       132
#define OTP_PMC_SECUREREG     133
#define OTP_PBMU_DISABLE      134
        uint32 status8;         /* 0x34 */
        uint32 status9;         /* 0x38 */
#define OTP_DDR_SECUREACC     211
#define OTP_DDR_SECIRELCK     212
#define OTP_ECC_MREPAIR_EN    217       
} JtagOtp;

#define JTAGOTP ((volatile JtagOtp * const) JTAG_OTP_BASE)

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
#define NINT_STS_MASK           0x00000fff
#define NINT_ECC_ERROR_CORR_SEC 0x00000800
#define NINT_ECC_ERROR_UNC_SEC  0x00000400
#define NINT_CTRL_READY_SEC     0x00000200
#define NINT_INV_ACC_SEC        0x00000100
#define NINT_ECC_ERROR_CORR     0x00000080
#define NINT_ECC_ERROR_UNC      0x00000040
#define NINT_DEV_RBPIN          0x00000020
#define NINT_CTRL_READY         0x00000010
#define NINT_PAGE_PGM           0x00000008
#define NINT_COPY_BACK          0x00000004
#define NINT_BLOCK_ERASE        0x00000002
#define NINT_NP_READ            0x00000001
    uint32 NandInterruptEn;
#define NINT_ENABLE_MASK        0x0000ffff
    uint32 NandBaseAddr0;   /* Default address when booting from NAND flash */
    uint32 NandBaseAddr1;   /* Secondary base address for NAND flash */
} NandIntrCtrlRegs;

#define NAND_INTR ((volatile NandIntrCtrlRegs * const) NAND_INTR_BASE)
#define NAND_CACHE ((volatile uint8 * const) NAND_CACHE_BASE)


typedef struct BootBase {
    uint32 general_secbootcfg;
#define BOOTROM_CRC_DONE    (1 << 31)
#define BOOTROM_CRC_FAIL    (1 << 30)
    uint32 general_boot_crc_low;
    uint32 general_boot_crc_high;
} BootBase;

#define BOOTBASE ((volatile BootBase * const) BROM_BASE)

typedef struct BootSec {
    uint32 AccessCtrl;
    uint32 AccessRangeChk[4];
} BootSec;

#define BOOTSECURE ((volatile BootSec * const) BROM_SEC_BASE)

#endif

#ifdef __cplusplus
}
#endif

#endif

