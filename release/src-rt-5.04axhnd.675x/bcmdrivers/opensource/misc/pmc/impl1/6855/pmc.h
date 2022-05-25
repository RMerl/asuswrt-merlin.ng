/*
<:copyright-BRCM:2020:DUAL/GPL:standard 

   Copyright (c) 2020 Broadcom 
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
#ifndef _PMC_REG_H
#define _PMC_REG_H

/*
 * Power Management Control
 */

typedef union
{
    struct
    {
        uint32_t propagate_to_err  :  1; // [00:00] -+
        uint32_t propagate_slv_err :  1; // [01:01]  | - these are potentially dangerous and MAY cause a system crash
        uint32_t pmbus_reset_n     :  1; // [02:02] -+
        uint32_t reserved0         :  1; // [03:03]
        uint32_t maxPmbIdx         :  3; // [06:04] 0-based (0-7)
        uint32_t reserved1         :  1; // [07:07]
        uint32_t maxClientId       : 12; // [19:08] 0-based (theoreticaly 0-4095, but code limits this to 256 devices - 0-255)
        uint32_t numRegsPerClient  : 10; // [29:20] some power of 2 - number of 32-bit registers in each client (max = 512)
        uint32_t startDiscovery    :  1; // [30:30] kicks off H/W discovery of clients and fills in the map (see PMB_REGS below)
        uint32_t discoveryBusy     :  1; // [31:31] whether or not H/W discovery is still busy creating the map
    } Bits;
    uint32_t Reg32;
} PMB_CONFIG_REG;

typedef union
{
    struct {
        uint32_t data      : 16; // [15:00]
        uint32_t reserved1 : 16; // [31:16]
    } Bits;
    uint32_t Reg32;
} SSBM_data_reg;

typedef union
{
    struct {
        uint32_t  ssb_addr    : 10; // [09:00]
        uint32_t  ssb_cmd     :  2; // [11:10]
        uint32_t  ssb_en      :  1; // [12:12]
        uint32_t  ssb_add_pre :  1; // [13:13]
        uint32_t  reserved2   :  1; // [14:14]
        uint32_t  ssb_start   :  1; // [15:15]
        uint32_t  reserved1   : 16; // [31:16]
    } Bits;
    uint32_t Reg32;
} SSBM_control_reg;

typedef union
{
    struct {
        uint32_t busy      :  1; // [00:00]
        uint32_t reserved1 : 31; // [31:01]
    } Bits;
    uint32_t Reg32;
} SSBM_status_reg;

typedef union
{
    struct {
        uint32_t swreg_th_lo : 8; // [07:00]
        uint32_t swreg_th_hi : 8; // [15:08]
        uint32_t reserved    :16; // [31:16]
    } Bits;
    uint32_t Reg32;
} SSBM_SWREG_th_hilo_reg;


typedef union
{
    struct {
        uint32_t ssb_lock_addr : 10; // [09:00]
        uint32_t lock_bit      :  1; // [10:10]
        uint32_t lock_mode     :  1; // [11:11]
        uint32_t reserved      : 20; // [31:12]
    } Bits;
    uint32_t Reg32;
} SSBM_SWREG_lock_reg;

#define kSSBWrite   0x01
#define kSSBRead    0x02
#define kSSBEn      (1 << 12)
#define kSSBStart   (1 << 15)

typedef struct SSBMaster {
    SSBM_control_reg        ssbmControl;    /* 0x0060 */
    SSBM_data_reg           ssbmWrData;     /* 0x0064 */
    SSBM_data_reg           ssbmRdData;     /* 0x0068 */
    SSBM_status_reg         ssbmStatus;     /* 0x006c */
    SSBM_SWREG_th_hilo_reg  ssbmThHiLo;     /* 0x0070 */
    SSBM_SWREG_lock_reg     ssbmSwLock;     /* 0x0074 */
} SSBMaster;

typedef struct keyholeReg {
    uint32_t control;
#define PMC_PMBM_START                  (1 << 31)
#define PMC_PMBM_TIMEOUT                (1 << 30)
#define PMC_PMBM_SLAVE_ERR              (1 << 29)
#define PMC_PMBM_BUSY                   (1 << 28)
#define PMC_PMBM_BUS_SHIFT              (20)
#define PMC_PMBM_Read                   (0 << 24)
#define PMC_PMBM_Write                  (1 << 24)
    uint32_t wr_data;
    uint32_t mutex;
    uint32_t rd_data;
} keyholeReg;

typedef struct PmbBus {
    uint32_t config;          /* 0x0100 */
#define PMB_NUM_REGS_SHIFT (20)
#define PMB_NUM_REGS_MASK  (0x3ff)
    uint32_t arbiter;         /* 0x0104 */
    uint32_t timeout;         /* 0x0108 */
    uint32_t unused1;         /* 0x010c */
    keyholeReg keyhole[4];  /* 0x0110-0x014f */
    uint32_t unused2[44];     /* 0x0150-0x01ff */
    uint32_t map[16];         /* 0x0200-0x023f */ 
}PmbBus;

typedef struct Pmc {
    SSBMaster ssbMasterCtrl;            /* 0x20060-0x20077 */
    uint32_t unused12[34];                /* 0x20078-0x200ff */
    PmbBus pmb;                         /* 0x20100 */
} Pmc;

#define PCMBUS_PHYS_BASE            0x83010A00
#define AVS_MODE                    0xff80262c
#endif
