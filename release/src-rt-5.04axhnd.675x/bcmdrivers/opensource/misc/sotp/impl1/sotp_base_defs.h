/*
<:copyright-BRCM:2018:DUAL/GPL:standard

   Copyright (c) 2018 Broadcom 
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

/***********************************************************************/
/*                                                                     */
/*   MODULE:  sotp_base_defs.h                                         */
/*   PURPOSE: Base sotp related definition.                            */
/*                                                                     */
/***********************************************************************/
#ifndef _SOTP_BASE_DEFS_H
#define _SOTP_BASE_DEFS_H

#include <bcmtypes.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct SotpRegs {
   uint32 sotp_otp_prog_ctrl;            /* 0x00 */  
   uint32 sotp_otp_wdata_0;              /* 0x04 */
   uint32 sotp_otp_wdata_1;              /* 0x08 */
   uint32 sotp_otp_addr;                 /* 0x0c */
   uint32 sotp_otp_ctrl_0;               /* 0x10 */
   uint32 dummy1;                        /* 0x14 */  
   uint32 sotp_otp_status_0;             /* 0x18 */
   uint32 sotp_otp_status_1;             /* 0x1c */
   uint32 sotp_otp_rdata_0;              /* 0x20 */
   uint32 sotp_otp_rdata_1;              /* 0x24 */
   uint32 sotp_chip_states;              /* 0x28 */
   uint32 dummy2;                        /* 0x2c */  
   uint32 sotp_otp_ecccnt;               /* 0x30 */
   uint32 sotp_otp_bad_addr;             /* 0x34 */
   uint32 sotp_otp_wr_lock;              /* 0x38 */
   uint32 sotp_otp_rd_lock;              /* 0x3c */
   uint32 sotp_rom_block_start;          /* 0x40 */
   uint32 sotp_rom_block_end;            /* 0x44 */
   uint32 sotp_samu_cntrl;               /* 0x48 */
   uint32 sotp_chip_cntrl;               /* 0x4c */
   uint32 sotp_sr_state_0;               /* 0x50 */
   uint32 sotp_sr_state_1;               /* 0x54 */
   uint32 sotp_sr_state_2;               /* 0x58 */
   uint32 sotp_sr_state_3;               /* 0x5c */
   uint32 sotp_sr_state_4;               /* 0x60 */
   uint32 sotp_sr_state_5;               /* 0x64 */
   uint32 sotp_sr_state_6;               /* 0x68 */
   uint32 sotp_sr_state_7;               /* 0x6c */
   uint32 sotp_perm;                     /* 0x70 */
   uint32 sotp_sotp_out_0;               /* 0x74 */
   uint32 sotp_sotp_out_1;               /* 0x78 */
   uint32 sotp_sotp_out_2;               /* 0x7c */
   uint32 sotp_sotp_inout;               /* 0x80 */
} SotpRegs;

/* SOTP base defines */
#define    SOTP_MAX_CNTR     		1000
#define    SOTP_NUM_ROWS		112     /* Maximum rows accessable in SOTP */
#define    SOTP_BITS_PER_ROW		41	/* Bits in each SOTP row, only some rows allow direct programming of all 41 bits */
#define    SOTP_BYTES_PER_ROW  		4       /* User Data bytes in one keyslot row */
#define    SOTP_ROWS_IN_REGION          4

/* LOCK sections */
#define    SOTP_WRAPPER_LOCK_SECTION    1       /* Section # containing write lock bits */
#define    SOTP_PROG_LOCK_SECTION       2       /* Section # containing read lock bits */
#define    SOTP_REGIONS_IN_LOCK		1       /* Number of regions in combined lock sections */
#define    SOTP_FIRST_FUSELOCK_ROW 	8	/* First row containing fuse-lock ctrl bits */	
#define    SOTP_NUM_REG_IN_FUSELOCK_ROW	21      /* Num regions represented in each fuse-lock row */

/* Device CFG section */
#define    SOTP_DEVICE_CFG_SECTION      3       /* Section # containing SOTP device cfg */
#define    SOTP_REGIONS_IN_DEVCFG	1       /* Number of regions in device cfg */

/* General CFG sections */
#define    SOTP_FIRST_CFG_SECTION       4       /* First general CFG section # */
#define    SOTP_FIRST_ECC_CONFIG_ROW    16      /* First row containing ECC protected 32-bit config data */
#define    SOTP_REGIONS_IN_CFG          1       /* Number of regions in the General CFG sections */

/* Keyslot sections */
#define    SOTP_MIN_KEYSLOT		7	/* First keyslot section # */
#define    SOTP_MAX_KEYSLOT		13	/* Last  keyslot section # */
#define    SOTP_REGIONS_IN_KEYSLOT      3       /* Number of regions in each keyslot section */
#define    SOTP_ROWS_IN_KEYSLOT 	12      /* Number of rows in each keyslot section */
#define    SOTP_FIRST_KEYSLOT_ROW 	28      /* First row containing 32-bit credential data */
#define    SOTP_FIRST_KEYSLOT_REGION    7       /* First region containing credential data */
#define    SOTP_FIRST_KEYSLOT_SECTION   7       /* First keyslot section # */
#define    SOTP_REGIONS_MASK_IN_KEYSLOT 7
#define    SOTP_MAX_KEYLEN 		8       /* Maximum key length in 32-bit words */ 

#define SOTP_OTP_PROG_CTRL_OTP_ECC_WREN_SHIFT   8
#define SOTP_OTP_PROG_CTRL_OTP_ECC_WREN         (1 << SOTP_OTP_PROG_CTRL_OTP_ECC_WREN_SHIFT)
#define SOTP_OTP_PROG_CTRL_OTP_DISABLE_ECC_SHIFT 9
#define SOTP_OTP_PROG_CTRL_OTP_DISABLE_ECC      (1 << SOTP_OTP_PROG_CTRL_OTP_DISABLE_ECC_SHIFT)
#define SOTP_OTP_PROG_CTRL_OTP_CPU_MODE_EN_SHIFT 15
#define SOTP_OTP_PROG_CTRL_OTP_CPU_MODE_EN      (1 << SOTP_OTP_PROG_CTRL_OTP_CPU_MODE_EN_SHIFT)

#define SOTP_OTP_PROG_CTRL_REGS_ECC_EN_SHIFT     17
#define SOTP_OTP_PROG_CTRL_REGS_ECC_EN_MASK      (0xf << SOTP_OTP_PROG_CTRL_REGS_ECC_EN_SHIFT) /* bits 20:17 */
#define SOTP_OTP_PROG_CTRL_REGS_ECC_EN           (0xa << SOTP_OTP_PROG_CTRL_REGS_ECC_EN_SHIFT)
#define SOTP_OTP_PROG_CTRL_REGS_ECC_DIS          (0x5 << SOTP_OTP_PROG_CTRL_REGS_ECC_EN_SHIFT)

#define SOTP_OTP_WDATA_1_FAIL_SHIFT             7
#define SOTP_OTP_WDATA_1_FAIL_MASK              (0x3 << SOTP_OTP_WDATA_1_FAIL_SHIFT)

#define SOTP_OTP_ADDR_OTP_ADDR_SHIFT            6

#define SOTP_OTP_CTRL_0_OTP_CMD_SHIFT           1
#define SOTP_OTP_CTRL_0_OTP_CMD_MASK            (0x1f << SOTP_OTP_CTRL_0_OTP_CMD_SHIFT) /* bits [05:01] */
#define SOTP_OTP_CTRL_0_OTP_CMD_OTP_READ        (0x0 << SOTP_OTP_CTRL_0_OTP_CMD_SHIFT) 
#define SOTP_OTP_CTRL_0_OTP_CMD_OTP_PROG_ENABLE (0x2 << SOTP_OTP_CTRL_0_OTP_CMD_SHIFT) 
#define SOTP_OTP_CTRL_0_OTP_CMD_PROG            (0xa << SOTP_OTP_CTRL_0_OTP_CMD_SHIFT) 
#define SOTP_OTP_CTRL_0_START_SHIFT             0
#define SOTP_OTP_CTRL_0_START                   (0x1 << SOTP_OTP_CTRL_0_START_SHIFT)   

#define SOTP_OTP_STATUS_1_CMD_DONE_SHIFT        1
#define SOTP_OTP_STATUS_1_CMD_DONE              (0x1 << SOTP_OTP_STATUS_1_CMD_DONE_SHIFT) 
#define SOTP_OTP_STATUS_1_ECC_COR_SHIFT         16
#define SOTP_OTP_STATUS_1_ECC_COR               (0x1 << SOTP_OTP_STATUS_1_ECC_COR_SHIFT) 
#define SOTP_OTP_STATUS_1_ECC_DET_SHIFT         17
#define SOTP_OTP_STATUS_1_ECC_DET               (0x1 << SOTP_OTP_STATUS_1_ECC_DET_SHIFT) 

#define SOTP_OTP_RDATA_1_FAIL_SHIFT             7
#define SOTP_OTP_RDATA_1_FAIL_MASK              (0x3 << SOTP_OTP_RDATA_1_FAIL_SHIFT)

#define SOTP_CHIP_CNTRL_SW_OVERRIDE_CHIP_STATES_SHIFT   4
#define SOTP_CHIP_CNTRL_SW_OVERRIDE_CHIP_STATES (0x1 << SOTP_CHIP_CNTRL_SW_OVERRIDE_CHIP_STATES_SHIFT)
#define SOTP_CHIP_CNTRL_SW_MANU_PROG_SHIFT       5
#define SOTP_CHIP_CNTRL_SW_MANU_PROG             (0x1 << SOTP_CHIP_CNTRL_SW_MANU_PROG_SHIFT)
#define SOTP_CHIP_CNTRL_SW_NON_AB_DEVICE_SHIFT   7
#define SOTP_CHIP_CNTRL_SW_NON_AB_DEVICE         (0x1 << SOTP_CHIP_CNTRL_SW_NON_AB_DEVICE_SHIFT)

#define SOTP_PERM_ALLOW_SECURE_ACCESS           0xCC
#define SOTP_PERM_ALLOW_NONSEC_ACCESS           0x33

#define SOTP_SOTP_OUT_0_SOTP_OTP_READY_SHIFT    26
#define SOTP_SOTP_OUT_0_SOTP_OTP_READY          (1 << SOTP_SOTP_OUT_0_SOTP_OTP_READY_SHIFT)

typedef enum
{
   SOTP_S_MODE_MFG,        // The SOTP block has not been personalized yet and is in manufacuring mode
   SOTP_S_MODE_FIELD,      // The SOTP block has been personalized and is ready for deployment
   SOTP_E_MODE_TIMEOUT,	   // The function experienced unexpected FSM timeout from the SOTP block 
   SOTP_E_MODE_ERROR       // The function experienced unexpected register return value from the SOTP block 
} SotpMode;

typedef enum
{
   SOTP_S_ROW_SUCCESS,	   // The function has completed successfully
   SOTP_S_ROW_ECC_COR, 	   // The function corrected 1 bad bit within the row (row is still usable)
   SOTP_E_ROW_ECC_DET,	   // The function detected 2 or more uncorrectable bits within the row (row is bad)
   SOTP_E_ROW_FAIL_SET,	   // The function detected that 1 or both of the fail bits are set (row is bad)
   SOTP_E_ROW_READ_LOCK,   // The function detected that the row is locked from reading
   SOTP_E_ROW_FUSE_LOCK,   // The function detected that the row is locked from further fusing
   SOTP_E_ROW_TIMEOUT,	   // The function experienced unexpected FSM timeout from the SOTP block 
   SOTP_E_ROW_ERROR	   // The function experienced unexpected register return value from the SOTP block 
} SotpRowStatus;


typedef enum
{
   SOTP_S_KEY_SUCCESS,	   // The function has completed successfully, and a credential exists
   SOTP_S_KEY_EMPTY,	   // The function has completed successfully, but the section is empty
   SOTP_E_KEY_BADPARAM,	   // The function received a bad input parameter 
   SOTP_E_KEY_OVERRUN,	   // The function ran out of rows within the section before the entire key and crc was fused
   SOTP_E_KEY_UNDERRUN,	   // The function ran out of valid section rows before reading the entire key and crc
   SOTP_E_KEY_CRC_MIS,	   // The function detected a CRC mismatch during the key read (key is bad or empty)
   SOTP_E_KEY_ERROR	   // The function experienced an unexpected return value  
} SotpKeyStatus;

/* Basic Functions */
int sotp_get_row_data( int row_addr, char* sotp_data, int data_len, int * result, int raw );
int sotp_get_keyslot_data( int section_num, char* sotp_data, int data_len, int * result );
int sotp_set_row_data( int row_addr, char* sotp_data, int data_len, int * result, int raw );
int sotp_set_keyslot_data( int section_num, char* sotp_data, int data_len, int * result );
int sotp_set_keyslot_readlock(int section_num, int * result);
int sotp_set_region_readlock(int region_num, int * result);
int sotp_set_region_fuselock(int region_num, int * result);
int sotp_get_keyslot_readlock_status(int section_num, int * result );
int sotp_get_region_readlock_status( int region_num, int * result );
int sotp_get_region_fuselock_status( int region_num, int * result );
int sotp_get_rollback_lvl( char * lvl, int * result);
int sotp_set_rollback_lvl( char * lvl, int * result);
int sotp_init ( void * base_ptr );
int sotp_dump_map( int * result );
#ifdef __cplusplus
}
#endif

#endif /* _BCM_HWDEFS_H */

