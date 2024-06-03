/* SPDX-License-Identifier: GPL-2.0+
 *
 *  Copyright 2019 Broadcom Ltd.
 */
#ifndef _SOTP_H
#define _SOTP_H

#define SOTP_BASE			0xff800c00
#define SOTP_OTP_REGION_RD_LOCK		0x3c


#define  SOTP_OTP_PROG_CTRL_OFFSET             0x00   
#define  SOTP_OTP_WDATA_0_OFFSET               0x04 
#define  SOTP_OTP_WDATA_1_OFFSET               0x08 
#define  SOTP_OTP_ADDR_OFFSET                  0x0C 
#define  SOTP_OTP_CTRL_0_OFFSET                0x10 
#define  DUMMY1_OFFSET                         0x14   
#define  SOTP_OTP_STATUS_0_OFFSET              0x18 
#define  SOTP_OTP_STATUS_1_OFFSET              0x1C 
#define  SOTP_OTP_RDATA_0_OFFSET               0x20 
#define  SOTP_OTP_RDATA_1_OFFSET               0x24 
#define  SOTP_CHIP_STATES_OFFSET               0x28 
#define  DUMMY2_OFFSET                         0x2C   
#define  SOTP_OTP_ECCCNT_OFFSET                0x30 
#define  SOTP_OTP_BAD_ADDR_OFFSET              0x34 
#define  SOTP_OTP_WR_LOCK_OFFSET               0x38 
#define  SOTP_OTP_RD_LOCK_OFFSET               0x3C 
#define  SOTP_ROM_BLOCK_START_OFFSET           0x40 
#define  SOTP_ROM_BLOCK_END_OFFSET             0x44 
#define  SOTP_SAMU_CNTRL_OFFSET                0x48 
#define  SOTP_CHIP_CNTRL_OFFSET                0x4C 
#define  SOTP_SR_STATE_0_OFFSET                0x50 
#define  SOTP_SR_STATE_1_OFFSET                0x54 
#define  SOTP_SR_STATE_2_OFFSET                0x58 
#define  SOTP_SR_STATE_3_OFFSET                0x5C 
#define  SOTP_SR_STATE_4_OFFSET                0x60 
#define  SOTP_SR_STATE_5_OFFSET                0x64 
#define  SOTP_SR_STATE_6_OFFSET                0x68 
#define  SOTP_SR_STATE_7_OFFSET                0x6C 
#define  SOTP_PERM_OFFSET                      0x70 
#define  SOTP_SOTP_OUT_0_OFFSET                0x74 
#define  SOTP_SOTP_OUT_1_OFFSET                0x78 
#define  SOTP_SOTP_OUT_2_OFFSET                0x7C 
#define  SOTP_SOTP_INOUT_OFFSET                0x80
/*
#####################################################################
# SOTP Registers
#####################################################################
*/

#define SOTP_OTP_PROG_CTRL			0x00

#define SOTP_OTP_PROG_CTRL_OTP_ECC_WREN_SHIFT   8
#define SOTP_OTP_PROG_CTRL_OTP_ECC_WREN 	(1 << SOTP_OTP_PROG_CTRL_OTP_ECC_WREN_SHIFT)

#define SOTP_OTP_PROG_CTRL_OTP_DISABLE_ECC_SHIFT 9
#define SOTP_OTP_PROG_CTRL_OTP_DISABLE_ECC 	(1 << SOTP_OTP_PROG_CTRL_OTP_DISABLE_ECC_SHIFT)

#define SOTP_OTP_PROG_CTRL_OTP_CPU_MODE_EN_SHIFT 15
#define SOTP_OTP_PROG_CTRL_OTP_CPU_MODE_EN	(1 << SOTP_OTP_PROG_CTRL_OTP_CPU_MODE_EN_SHIFT)

#define SOTP_OTP_PROG_CTRL_REGS_ECC_EN_SHIFT     17
#define SOTP_OTP_PROG_CTRL_REGS_ECC_EN_MASK      (0xf << SOTP_OTP_PROG_CTRL_REGS_ECC_EN_SHIFT) /* bits 20:17 */
#define SOTP_OTP_PROG_CTRL_REGS_ECC_EN           (0xa << SOTP_OTP_PROG_CTRL_REGS_ECC_EN_SHIFT)
#define SOTP_OTP_PROG_CTRL_REGS_ECC_DIS          (0x5 << SOTP_OTP_PROG_CTRL_REGS_ECC_EN_SHIFT)

/* --- */

#define SOTP_OTP_WDATA_0			0x04

/* --- */

#define SOTP_OTP_WDATA_1			0x08
#define SOTP_OTP_WDATA_1_FAIL_SHIFT		7
#define SOTP_OTP_WDATA_1_FAIL_MASK		(0x3 << SOTP_OTP_WDATA_1_FAIL_SHIFT)

/* --- */

#define SOTP_OTP_ADDR				0x0c
#define SOTP_OTP_ADDR_OTP_ADDR_SHIFT		6

/* --- */

#define SOTP_OTP_CTRL_0				0x10

#define SOTP_OTP_CTRL_0_OTP_CMD_SHIFT		1
#define SOTP_OTP_CTRL_0_OTP_CMD_MASK		(0x1f << SOTP_OTP_CTRL_0_OTP_CMD_SHIFT) /* bits [05:01] */
#define SOTP_OTP_CTRL_0_OTP_CMD_OTP_READ	(0x0 << SOTP_OTP_CTRL_0_OTP_CMD_SHIFT) 
#define SOTP_OTP_CTRL_0_OTP_CMD_OTP_PROG_ENABLE	(0x2 << SOTP_OTP_CTRL_0_OTP_CMD_SHIFT) 
#define SOTP_OTP_CTRL_0_OTP_CMD_PROG		(0xa << SOTP_OTP_CTRL_0_OTP_CMD_SHIFT) 

#define SOTP_OTP_CTRL_0_START_SHIFT		0
#define SOTP_OTP_CTRL_0_START            	(0x1 << SOTP_OTP_CTRL_0_START_SHIFT)   

/* --- */

#define SOTP_OTP_STATUS_1			0x1c

#define SOTP_OTP_STATUS_1_CMD_DONE_SHIFT	1
#define SOTP_OTP_STATUS_1_CMD_DONE	        (0x1 << SOTP_OTP_STATUS_1_CMD_DONE_SHIFT) 

#define SOTP_OTP_STATUS_1_ECC_COR_SHIFT		16
#define SOTP_OTP_STATUS_1_ECC_COR	        (0x1 << SOTP_OTP_STATUS_1_ECC_COR_SHIFT) 

#define SOTP_OTP_STATUS_1_ECC_DET_SHIFT		17
#define SOTP_OTP_STATUS_1_ECC_DET	        (0x1 << SOTP_OTP_STATUS_1_ECC_DET_SHIFT) 

/* --- */

#define SOTP_OTP_RDATA_0			0x20

/* --- */

#define SOTP_OTP_RDATA_1			0x24
#define SOTP_OTP_RDATA_1_FAIL_SHIFT		7
#define SOTP_OTP_RDATA_1_FAIL_MASK		(0x3 << SOTP_OTP_RDATA_1_FAIL_SHIFT)

/* --- */

#define SOTP_OTP_REGION_RD_LOCK			0x3c

/* --- */

#define SOTP_CHIP_CTRL                          0x4c

#define SOTP_CHIP_CNTRL_SW_OVERRIDE_CHIP_STATES_SHIFT   4
#define SOTP_CHIP_CNTRL_SW_OVERRIDE_CHIP_STATES (0x1 << SOTP_CHIP_CNTRL_SW_OVERRIDE_CHIP_STATES_SHIFT)

#define SOTP_CHIP_CTRL_SW_MANU_PROG_SHIFT       5
#define SOTP_CHIP_CTRL_SW_MANU_PROG             (0x1 << SOTP_CHIP_CTRL_SW_MANU_PROG_SHIFT)

#define SOTP_CHIP_CTRL_SW_NON_AB_DEVICE_SHIFT   7
#define SOTP_CHIP_CTRL_SW_NON_AB_DEVICE         (0x1 << SOTP_CHIP_CTRL_SW_NON_AB_DEVICE_SHIFT)

/* --- */

#define SOTP_SOTP_OUT_0                         0x74
#define SOTP_SOTP_OUT_0_SOTP_OTP_READY_SHIFT    26
#define SOTP_SOTP_OUT_0_SOTP_OTP_READY          (1 << SOTP_SOTP_OUT_0_SOTP_OTP_READY_SHIFT)

#define SOTP ((volatile sotp_t * const) SOTP_BASE)

#define SOTP_PERM		0x70
#define SOTP_PERM_SHIFT 	0x0
#define SOTP_PERM_BLK_SHIFT	0x4

#define SOTP_PERM_SEC_W 	0x8
#define SOTP_PERM_SEC_R 	0x4
#define SOTP_PERM_NSEC_W 	0x2
#define SOTP_PERM_NSEC_R 	0x1

#define SOTP_PERM_BLK_SEC_W 	0x8
#define SOTP_PERM_BLK_SEC_R 	0x4
#define SOTP_PERM_BLK_NSEC_W 	0x2
#define SOTP_PERM_BLK_NSEC_R 	0x1

#define SOTP_PERM_NSEC_ENABLE   (((SOTP_PERM_BLK_NSEC_R|SOTP_PERM_BLK_NSEC_W)<<SOTP_PERM_BLK_SHIFT)| (SOTP_PERM_SEC_W|SOTP_PERM_SEC_R)<<SOTP_PERM_SHIFT)
#define SOTP_PERM_SEC_ENABLE    (((SOTP_PERM_BLK_SEC_R|SOTP_PERM_BLK_SEC_W)<<SOTP_PERM_BLK_SHIFT)|  (SOTP_PERM_SEC_W|SOTP_PERM_SEC_R)<<SOTP_PERM_SHIFT)
#define SOTP_PERM_DISABLE_ALL	0x0
/* When enabling all access, keep the NONSEC access locked for permissions register */
#define SOTP_PERM_ENABLE_ALL	(((SOTP_PERM_BLK_SEC_W|SOTP_PERM_BLK_SEC_R|SOTP_PERM_BLK_NSEC_W|SOTP_PERM_BLK_NSEC_R)<<SOTP_PERM_BLK_SHIFT)| ((SOTP_PERM_SEC_W|SOTP_PERM_SEC_R)<<SOTP_PERM_SHIFT))

#define SOTP_PERM_ALLOW_SECURE_ACCESS_ONLY	0xCC
#define SOTP_PERM_ALLOW_NONSEC_ACCESS_ONLY	0x33
/* When enabling NONSEC, enable SEC as well */
#define SOTP_PERM_ALLOW_NONSEC_BLK_ACCESS	(SOTP_PERM_ENABLE_ALL)
/* When disabling SEC, disable NONSEC as well */
#define SOTP_PERM_DISABLE_SECURE_ACCESS		(SOTP_PERM_DISABLE_ALL)

#define ROWS_PER_RGN 4
#define SOTP_RGN_CNT 28 
#define SOTP_ROW_CNT  (ROWS_PER_RGN*SOTP_ROW_CNT)


typedef enum _sotp_row_status_ {
   SOTP_S_ROW_SUCCESS,	   // The function has completed successfully
   SOTP_S_ROW_ECC_COR, 	   // The function corrected 1 bad bit within the row (row is still usable)
   SOTP_E_ROW_ECC_DET,	   // The function detected 2 or more uncorrectable bits within the row (row is bad)
   SOTP_E_ROW_FAIL_SET,	   // The function detected that 1 or both of the fail bits are set (row is bad)
   SOTP_E_ROW_READ_LOCK,   // The function detected that the row is locked from reading
   SOTP_E_ROW_FUSE_LOCK,   // The function detected that the row is locked from further fusing
   SOTP_E_ROW_TIMEOUT,	   // The function experienced unexpected FSM timeout from the SOTP block 
   SOTP_E_ROW_ERROR	   // The function experienced unexpected register return value from the SOTP block 
} SotpRowStatus;


typedef enum _sotp_keys_status{
   SOTP_S_KEY_SUCCESS,	   // The function has completed successfully, and a credential exists
   SOTP_S_KEY_EMPTY,	   // The function has completed successfully, but the section is empty
   SOTP_E_KEY_BADPARAM,	   // The function received a bad input parameter 
   SOTP_E_KEY_OVERRUN,	   // The function ran out of rows within the section before the entire key and crc was fused
   SOTP_E_KEY_UNDERRUN,	   // The function ran out of valid section rows before reading the entire key and crc
   SOTP_E_KEY_CRC_MIS,	   // The function detected a CRC mismatch during the key read (key is bad or empty)
   SOTP_E_KEY_ERROR	   // The function experienced an unexpected return value  
} SotpKeyStatus;


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

/* Device CFG section */
#define    SOTP_DEVICE_CFG_SECTION      3       /* Section # containing SOTP device cfg */
#define    SOTP_REGIONS_IN_DEVCFG	1       /* Number of regions in device cfg */


/* General CFG sections */
#define    SOTP_FIRST_CFG_SECTION       4       /* First general CFG section # */
#define    SOTP_FIRST_ECC_CONFIG_ROW    16      /* First row containing ECC protected 32-bit config data */
#define    SOTP_REGIONS_IN_CFG          1       /* Number of regions in the General CFG sections */



/* An SOTP row initializer that maps actual row number with mask and shift to a feature name;
 * this allows to use features vs. rows for common functionality, 
 * such as secure keys reading and fusing 
 * prevent ifdef dependencies when used outside of arch directories for common among SoCs logic
 * */
	
#define	DEFINE_SOTP_MAP_ROW_INITLR(__VV__)					\
	static otp_hw_cmn_row_t __VV__[ ] = {					\
		{.addr = 16, .range = 4, .feat = SOTP_MAP_ANTI_ROLLBACK, .conf = {.addr_type = OTP_HW_CMN_ROW_ADDR_ROW, .op_type = OTP_HW_CMN_CTL_CONF} },	\
		{.addr = 28, .range = 8, .feat = SOTP_MAP_KEY_DEV_SPECIFIC },	\
		{.addr = 40, .range = 8, .feat = SOTP_MAP_FLD_ROE },		\
		{.addr = 52, .range = 8, .feat = SOTP_MAP_FLD_HMID },		\
		{.addr = 64, .range = 8, .feat = SOTP_MAP_SER_NUM },	\
		{.addr = 76, .range = 8, .feat = SOTP_MAP_KEY_SECT_1},		\
		{.addr = 88, .range = 8, .feat = SOTP_MAP_KEY_SECT_2},		\
		{.addr = 100, .range = 8, .feat = SOTP_MAP_KEY_SECT_3},		\
	};
#endif
