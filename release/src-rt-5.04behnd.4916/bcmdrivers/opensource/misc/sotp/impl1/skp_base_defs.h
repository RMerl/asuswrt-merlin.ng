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

/***********************************************************************/
/*                                                                     */
/*   MODULE:  skp_base_defs.h                                          */
/*   PURPOSE: Base sotp related definition.                            */
/*                                                                     */
/***********************************************************************/
#ifndef _SKP_BASE_DEFS_H
#define _SKP_BASE_DEFS_H

#include <bcmtypes.h>

#ifdef __cplusplus
extern "C" {
#endif

/* KSR defs */
#define KSR_STATUS_KEY_LOADED_SHIFT	24
#define KSR_STATUS_KEY_LOADED_MASK	0x1
#define KSR_STATUS_READY_SHIFT		21
#define KSR_STATUS_READY_MASK		0x1
#define KSR_STATUS_DESC_VALID_SHIFT	20
#define KSR_STATUS_DESC_VALID_MASK	0x1
#define KSR_RD_DESC_DGOOD_MASK          0x1
#define KSR_RD_DESC_DGOOD_SHIFT         29
#define KSR_RD_DGOOD(_V_)      ((_V_>>KSR_RD_DESC_DGOOD_SHIFT)&KSR_RD_DESC_DGOOD_MASK) 

#define	KSR_PORTAL_VALID(_V_)	(	((_V_>>KSR_STATUS_KEY_LOADED_SHIFT)&KSR_STATUS_KEY_LOADED_MASK) &\
					((_V_>>KSR_STATUS_READY_SHIFT)&KSR_STATUS_READY_MASK)		&\
 					((_V_>>KSR_STATUS_DESC_VALID_SHIFT)&KSR_STATUS_DESC_VALID_MASK) )

#define	KSR_PORTAL_READY(_V_)		((_V_>>KSR_STATUS_READY_SHIFT)&KSR_STATUS_READY_MASK)

#define	KSR_PORTAL_KEY_LOADED(_V_)		((_V_>>KSR_STATUS_KEY_LOADED_SHIFT)&KSR_STATUS_KEY_LOADED_MASK)

#define	KSR_PORTAL_DESC_VALID(_V_)		((_V_>>KSR_STATUS_DESC_VALID_SHIFT)&KSR_STATUS_DESC_VALID_MASK)

#define KSR_STATUS_ECC_DATA_SHIFT	23
#define KSR_STATUS_ECC_DATA_MASK	0x1
#define KSR_STATUS_ECC_DESC_SHIFT	22
#define KSR_STATUS_ECC_DESC_MASK	0x1
#define KSR_PORTAL_ECC_DATA_ERR(_V_)	((_V_>>KSR_STATUS_ECC_DATA_SHIFT)&KSR_STATUS_ECC_DATA_MASK)
#define KSR_PORTAL_ECC_DESC_ERR(_V_)	((_V_>>KSR_STATUS_ECC_DESC_SHIFT)&KSR_STATUS_ECC_DESC_MASK)
#define	KSR_PORTAL_KEY_LOADED(_V_) 	((_V_>>KSR_STATUS_KEY_LOADED_SHIFT)&KSR_STATUS_KEY_LOADED_MASK)


#define KSR_PORTAL_OTP_PGM_DESC_VALID_SHIFT	31
#define KSR_PORTAL_OTP_PGM_DESC_VALID_MASK	0x1

#define KSR_PORTAL_PGM_DESC_PERM_SHIFT		0
#define KSR_PORTAL_PGM_DESC_PERM_MASK		0xf

#define KSR_PORTAL_PGM_DESC_LOCK_MASK		0x1	

#define KSR_PORTAL_PGM_DESC_BLK_SW_LOCK_SHIFT	7
#define KSR_PORTAL_PGM_DESC_BLK_SR_LOCK_SHIFT	6
#define KSR_PORTAL_PGM_DESC_BLK_NSW_LOCK_SHIFT	5
#define KSR_PORTAL_PGM_DESC_BLK_NSR_LOCK_SHIFT	4
#define KSR_PORTAL_PGM_DESC_PERM_SW_LOCK_SHIFT	3	
#define KSR_PORTAL_PGM_DESC_PERM_SR_LOCK_SHIFT	2
#define KSR_PORTAL_PGM_DESC_PERM_NSW_LOCK_SHIFT	1
#define KSR_PORTAL_PGM_DESC_PERM_NSR_LOCK_SHIFT	0

#define KSR_PERM_BLK_SR_LOCK	(0x1<<KSR_PORTAL_PGM_DESC_BLK_SR_LOCK_SHIFT)
#define KSR_PERM_BLK_NSR_LOCK	(0x1<<KSR_PORTAL_PGM_DESC_BLK_NSR_LOCK_SHIFT)

#define KSR_PERM_BLK_SW_LOCK	(0x1<<KSR_PORTAL_PGM_DESC_BLK_SW_LOCK_SHIFT)
#define KSR_PERM_BLK_NSW_LOCK	(0x1<<KSR_PORTAL_PGM_DESC_BLK_NSW_LOCK_SHIFT)

#define KSR_PERM_PAC_SR_LOCK	(0x1<<KSR_PORTAL_PGM_DESC_PERM_SR_LOCK_SHIFT)
#define KSR_PERM_PAC_NSR_LOCK	(0x1<<KSR_PORTAL_PGM_DESC_PERM_NSR_LOCK_SHIFT)

#define KSR_PERM_PAC_SW_LOCK	(0x1<<KSR_PORTAL_PGM_DESC_PERM_SW_LOCK_SHIFT)
#define KSR_PERM_PAC_NSW_LOCK	(0x1<<KSR_PORTAL_PGM_DESC_PERM_NSW_LOCK_SHIFT)

#define KSR_PERM_LOCK_ALL	0xff
/* Allways keep access to PERM fields locked for NSEC */
#define KSR_PERM_UNLOCK_ALL	(KSR_PERM_PAC_NSR_LOCK | KSR_PERM_PAC_NSW_LOCK)
#define KSR_PERM_LOCK_NS					\
				(KSR_PERM_BLK_NSR_LOCK 	| 	\
				KSR_PERM_BLK_NSW_LOCK 	| 	\
				KSR_PERM_PAC_NSR_LOCK 	| 	\
				KSR_PERM_PAC_NSW_LOCK)
/* Unlocking NONSEC BLK access should unlock SEC BLK access as well */
#define KSR_PERM_UNLOCK_NS	(KSR_PERM_UNLOCK_ALL)
/* Locking SEC BLK access should also lock NONSEC BLK acces as well */
#define KSR_PERM_LOCK_S		(KSR_PERM_LOCK_ALL)
/* Unlocking SEC BLK access should lock NONSEC BLK access as well */
#define KSR_PERM_UNLOCK_S	(KSR_PERM_LOCK_NS)


#define KSR_PORTAL_PGM_DESC_NRW_PERM					\
		(0x1<<KSR_PORTAL_OTP_PGM_DESC_VALID_SHIFT)

#define KSR_PORTAL_PGM_DESC_SRW_PERM					\
		((0x1<<KSR_PORTAL_OTP_PGM_DESC_VALID_SHIFT)    |	\
		(0x1<<KSR_PORTAL_PGM_DESC_BLK_NSW_LOCK_SHIFT)  |	\
		(0x1<<KSR_PORTAL_PGM_DESC_BLK_NSR_LOCK_SHIFT)  |	\
		(0x1<<KSR_PORTAL_PGM_DESC_PERM_NSW_LOCK_SHIFT) |	\
		(0x1<<KSR_PORTAL_PGM_DESC_PERM_NSR_LOCK_SHIFT))

#define KSR_PORTAL_OTP_CMD_MASK			0xf
#define KSR_PORTAL_OTP_CMD_SHIFT		0x0

#define KSR_PORTAL_OTP_CMD_PGM_DESC_WORD	0x8
#define KSR_PORTAL_OTP_CMD_PGM_DESC_ECC		0x9
#define KSR_PORTAL_OTP_CMD_PGM_KEY		0x2

#define KSR_OTP_CMD_PGM_DESC_WORD		((KSR_PORTAL_OTP_CMD_MASK&KSR_PORTAL_OTP_CMD_PGM_DESC_WORD)<<KSR_PORTAL_OTP_CMD_SHIFT)
#define KSR_OTP_CMD_PGM_DESC_ECC		((KSR_PORTAL_OTP_CMD_MASK&KSR_PORTAL_OTP_CMD_PGM_DESC_ECC)<<KSR_PORTAL_OTP_CMD_SHIFT)
#define KSR_OTP_CMD_PGM_KEY			((KSR_PORTAL_OTP_CMD_MASK&KSR_PORTAL_OTP_CMD_PGM_KEY)<<KSR_PORTAL_OTP_CMD_SHIFT)


#define KSR_OTP_STATUS_CMD_ACTIVE_SHIFT	31
#define KSR_OTP_STATUS_CMD_ACTIVE_MASK	0x1
#define KSR_OTP_STATUS_ACTIVE(_V_)		\
		((_V_>>KSR_OTP_STATUS_CMD_ACTIVE_SHIFT)&KSR_OTP_STATUS_CMD_ACTIVE_MASK)
#define KSR_OTP_STATUS_OPCODE_SHIFT	16
#define KSR_OTP_STATUS_OPCODE_MASK	0xf
#define KSR_OTP_STATUS_CMD_ERR_SHIFT	0x0
#define KSR_OTP_STATUS_CMD_ERR_MASK	0xff

#define KSR_OTP_STATUS_CMD_ERROR(_V_)		\
		((_V_>>KSR_OTP_STATUS_CMD_ERR_SHIFT)&KSR_OTP_STATUS_CMD_ERR_MASK)


#define KSR_PAC_OFFSET                 	 	0x00       
#define KSR_OTP_CMD_OFFSET                  	0x04       
#define KSR_OTP_STATUS_OFFSET               	0x08       
#define KSR_RD_DESC_OFFSET                  	0x0c       
#define KSR_PGM_DESC_OFFSET                 	0x10       
#define KSR_PORTAL_STATUS_OFFSET            	0x14       
#define KSR_DATA_OFFSET		    		0x20

#define KSR_SIZE  0x80 
#define KSR_BASE_OFFSET 0x0 
#define KSR_START_ROW_ADDR			0
#define KSR_MAX_ROW_ADDR			(KSR_START_ROW_ADDR+6)


/* FSR defs */
#define FSR_PORTAL_OTP_CMD_READ_DATA		0x2		
#define FSR_PORTAL_OTP_CMD_READ_DESC		0x3
#define FSR_PORTAL_OTP_CMD_WRITE_DATA		0x4
#define FSR_PORTAL_OTP_CMD_WRITE_DATA_ECC	0x5
#define FSR_PORTAL_OTP_CMD_WRITE_DESC		0x8
#define FSR_PORTAL_OTP_CMD_WRITE_DESC_ECC	0x9
#define FSR_PORTAL_OTP_CMD_RELOAD_DATA		0xc
#define FSR_PORTAL_OTP_CMD_RELOAD_DESC		0xd	

#define FSR_PORTAL_OTP_CMD_WORD_SEL_MASK	0xfff
#define FSR_PORTAL_OTP_CMD_WORD_SEL_SHIFT	0x4

#define FSR_PORTAL_OTP_CMD_MASK			0xf
#define FSR_PORTAL_OTP_CMD_SHIFT		0x0


#define FSR_OTP_CMD_READ_DATA		((FSR_PORTAL_OTP_CMD_MASK&FSR_PORTAL_OTP_CMD_READ_DATA)<<FSR_PORTAL_OTP_CMD_SHIFT)
#define FSR_OTP_CMD_READ_DESC		((FSR_PORTAL_OTP_CMD_MASK&FSR_PORTAL_OTP_CMD_READ_DESC)<<FSR_PORTAL_OTP_CMD_SHIFT)
#define FSR_OTP_CMD_PGM_DATA		((FSR_PORTAL_OTP_CMD_MASK&FSR_PORTAL_OTP_CMD_WRITE_DATA)<<FSR_PORTAL_OTP_CMD_SHIFT)
#define FSR_OTP_CMD_PGM_DATA_ECC	((FSR_PORTAL_OTP_CMD_MASK&FSR_PORTAL_OTP_CMD_WRITE_DATA_ECC)<<FSR_PORTAL_OTP_CMD_SHIFT)
#define FSR_OTP_CMD_PGM_DESC		((FSR_PORTAL_OTP_CMD_MASK&FSR_PORTAL_OTP_CMD_WRITE_DESC)<<FSR_PORTAL_OTP_CMD_SHIFT)
#define FSR_OTP_CMD_PGM_DESC_ECC	((FSR_PORTAL_OTP_CMD_MASK&FSR_PORTAL_OTP_CMD_WRITE_DESC_ECC)<<FSR_PORTAL_OTP_CMD_SHIFT)
#define FSR_OTP_CMD_RELOAD_DATA		((FSR_PORTAL_OTP_CMD_MASK&FSR_PORTAL_OTP_CMD_RELOAD_DATA)<<FSR_PORTAL_OTP_CMD_SHIFT)
#define FSR_OTP_CMD_RELOAD_DESC		((FSR_PORTAL_OTP_CMD_MASK&FSR_PORTAL_OTP_CMD_RELOAD_DESC)<<FSR_PORTAL_OTP_CMD_SHIFT)

#define FSR_PORTAL_STATUS_ECC_PGM_DIS_SHIFT	23
#define FSR_PORTAL_STATUS_ECC_PGM_DIS_MASK	0x1
#define FSR_PORTAL_STATUS_RLOCK_SHIFT	22
#define FSR_PORTAL_STATUS_RLOCK_MASK	0x1
#define FSR_PORTAL_STATUS_READY_SHIFT		21
#define FSR_PORTAL_STATUS_READY_MASK		0x1
#define FSR_PORTAL_STATUS_DESC_VALID_SHIFT	20
#define FSR_PORTAL_STATUS_DESC_VALID_MASK	0x1

#define	FSR_PORTAL_RLOCK(_V_)	(	((_V_>>	FSR_PORTAL_STATUS_RLOCK_SHIFT)&\
						FSR_PORTAL_STATUS_RLOCK_MASK)	 	)

#define	FSR_PORTAL_ECC_DIS(_V_)	(	((_V_>>	FSR_PORTAL_STATUS_ECC_PGM_DIS_SHIFT)&\
						FSR_PORTAL_STATUS_ECC_PGM_DIS_MASK)	)

#define	FSR_PORTAL_VALID(_V_)	(	((_V_>>	FSR_PORTAL_STATUS_READY_SHIFT)&\
						FSR_PORTAL_STATUS_READY_MASK)		&\
 					((_V_>>	FSR_PORTAL_STATUS_DESC_VALID_SHIFT)&\
						FSR_PORTAL_STATUS_DESC_VALID_MASK)	)

#define	FSR_PORTAL_READY(_V_)	(	((_V_>>	FSR_PORTAL_STATUS_READY_SHIFT)&\
						FSR_PORTAL_STATUS_READY_MASK)		)

#define FSR_RD_DESC_VALID_SHIFT         31
#define FSR_RD_DESC_VALID_MASK          0x1
#define FSR_RD_DESC_ECC_PGM_DIS_SHIFT	29
#define FSR_RD_DESC_ECC_PGM_DIS_MASK	0x1
#define FSR_RD_DESC_RLOCK_SHIFT		28
#define FSR_RD_DESC_RLOCK_MASK		0x1
#define FSR_RD_DESC_SIZE_SHIFT		12
#define FSR_RD_DESC_SIZE_MASK		0xfff

#define FSR_RD_DDATA_SIZE(_V_)	((_V_>>FSR_RD_DESC_SIZE_SHIFT)&FSR_RD_DESC_SIZE_MASK)
#define FSR_RD_DGOOD(_V_)      	((_V_>>FSR_RD_DESC_VALID_SHIFT)&FSR_RD_DESC_VALID_MASK) 

#define FSR_OTP_STATUS_CMD_ACTIVE_SHIFT	31
#define FSR_OTP_STATUS_CMD_ACTIVE_MASK	0x1
#define FSR_OTP_STATUS_OPCODE_SHIFT	16
#define FSR_OTP_STATUS_OPCODE_MASK	0xf
#define FSR_OTP_STATUS_CMD_ERR_SHIFT	0x0
#define FSR_OTP_STATUS_CMD_ERR_MASK	0xff

#define FSR_OTP_STATUS_ACTIVE(_V_)		\
		((_V_>>FSR_OTP_STATUS_CMD_ACTIVE_SHIFT)&FSR_OTP_STATUS_CMD_ACTIVE_MASK)
#define FSR_OTP_STATUS_CMD_ERROR(_V_)		\
		((_V_>>FSR_OTP_STATUS_CMD_ERR_SHIFT)&FSR_OTP_STATUS_CMD_ERR_MASK)

#define FSR_PORTAL_RD_ECC_MASK			0x3ff

#define FSR_PORTAL_PGM_BLK_SW_LOCK_SHIFT	7
#define FSR_PORTAL_PGM_BLK_SR_LOCK_SHIFT	6
#define FSR_PORTAL_PGM_BLK_NSW_LOCK_SHIFT	5
#define FSR_PORTAL_PGM_BLK_NSR_LOCK_SHIFT	4
#define FSR_PORTAL_PGM_PERM_SW_LOCK_SHIFT	3	
#define FSR_PORTAL_PGM_PERM_SR_LOCK_SHIFT	2
#define FSR_PORTAL_PGM_PERM_NSW_LOCK_SHIFT	1
#define FSR_PORTAL_PGM_PERM_NSR_LOCK_SHIFT	0

#define FSR_PERM_BLK_SR_LOCK	(0x1<<FSR_PORTAL_PGM_BLK_SR_LOCK_SHIFT)
#define FSR_PERM_BLK_NSR_LOCK	(0x1<<FSR_PORTAL_PGM_BLK_NSR_LOCK_SHIFT)

#define FSR_PERM_BLK_SW_LOCK	(0x1<<FSR_PORTAL_PGM_BLK_SW_LOCK_SHIFT)
#define FSR_PERM_BLK_NSW_LOCK	(0x1<<FSR_PORTAL_PGM_BLK_NSW_LOCK_SHIFT)

#define FSR_PERM_PAC_SR_LOCK	(0x1<<FSR_PORTAL_PGM_PERM_SR_LOCK_SHIFT)
#define FSR_PERM_PAC_NSR_LOCK	(0x1<<FSR_PORTAL_PGM_PERM_NSR_LOCK_SHIFT)

#define FSR_PERM_PAC_SW_LOCK	(0x1<<FSR_PORTAL_PGM_PERM_SW_LOCK_SHIFT)
#define FSR_PERM_PAC_NSW_LOCK	(0x1<<FSR_PORTAL_PGM_PERM_NSW_LOCK_SHIFT)

#define FSR_PERM_LOCK_ALL	0xff
/* Allways keep access to PERM fields locked for NSEC */
#define FSR_PERM_UNLOCK_ALL	(FSR_PERM_PAC_NSR_LOCK | FSR_PERM_PAC_NSW_LOCK)
#define FSR_PERM_LOCK_NS					\
				(FSR_PERM_BLK_NSR_LOCK 	| 	\
				FSR_PERM_BLK_NSW_LOCK 	| 	\
				FSR_PERM_PAC_NSR_LOCK 	| 	\
				FSR_PERM_PAC_NSW_LOCK)
/* Unlocking NONSEC BLK access should UNLOCK SEC BLK access as well */
#define FSR_PERM_UNLOCK_NS	(FSR_PERM_UNLOCK_ALL)
/* Locking SEC BLK access should also LOCK NONSEC BLK acces as well */
#define FSR_PERM_LOCK_S		(FSR_PERM_LOCK_ALL)
/* Unlocking SEC BLK access should LOCK NONSEC BLK access as well */
#define FSR_PERM_UNLOCK_S	(FSR_PERM_LOCK_NS)


#define FSR_OTP_CMD_OFFSET                  	0x00       
#define FSR_OTP_STATUS_OFFSET               	0x04       
#define FSR_DATA_OFFSET		    		0x08
#define FSR_RD_ECC_OFFSET                  	0x0c       
#define FSR_PORTAL_STATUS_OFFSET            	0x10       
#define FSR_PAC_OFFSET                 	 	0x18       

#define FSR_SIZE  0x20 
#define FSR_BASE_OFFSET 0x300 
#define FSR_START_ROW_ADDR			6
#define FSR_MAX_ROW_ADDR			(FSR_START_ROW_ADDR+8)
	

#define SKP_ANTI_ROLLBACK_LVL_NUMBITS		2
#define SKP_ANTI_ROLLBACK_LVL_MASK		((1 << SKP_ANTI_ROLLBACK_LVL_NUMBITS)-1)
#define SKP_ANTI_ROLLBACK_LVL_SHIFT		SKP_ANTI_ROLLBACK_LVL_NUMBITS

typedef enum skp_hw_cmn_perm_e{
	SKP_HW_CMN_CTL_LOCK_NONE 		= 0x0,
	SKP_HW_CMN_CTL_LOCK_PAC_SW 		= 0x1,
	SKP_HW_CMN_CTL_LOCK_PAC_NSW 		= 0x2,
	SKP_HW_CMN_CTL_LOCK_PAC_NSRD 		= 0x4,
	SKP_HW_CMN_CTL_LOCK_PAC_SRD 		= 0x8,
	SKP_HW_CMN_CTL_LOCK_SRD 		= 0x10,
	SKP_HW_CMN_CTL_LOCK_NSRD 		= 0x20,
	SKP_HW_CMN_CTL_LOCK_SW 			= 0x40,
	SKP_HW_CMN_CTL_LOCK_NSW 		= 0x80,
	SKP_HW_CMN_CTL_LOCK_ALL 		= 0x100,
	SKP_HW_CMN_CTL_LOCK_NS 			= 0x200,
	SKP_HW_CMN_CTL_LOCK_NS_PROV 		= 0x400,
	SKP_HW_CMN_CTL_LOCK_S 			= 0x800,
	SKP_HW_CMN_CTL_LOCK_ROW_RD 		= 0x1000,
	SKP_HW_CMN_CTL_LOCK_ROW_W 		= 0x2000,
} skp_hw_cmn_perm_t;

typedef enum otp_hw_cmn_status_e{
	SKP_HW_CMN_STATUS_UNLOCKED		= 0x0,
	SKP_HW_CMN_STATUS_SRD_LOCKED		= 0x1,
	SKP_HW_CMN_STATUS_SW_LOCKED		= 0x2,
	SKP_HW_CMN_STATUS_NSRD_LOCKED		= 0x4,
	SKP_HW_CMN_STATUS_NSW_LOCKED		= 0x8,
	SKP_HW_CMN_STATUS_SW_PAC_LOCKED		= 0x10,
	SKP_HW_CMN_STATUS_NSW_PAC_LOCKED	= 0x20,
	SKP_HW_CMN_STATUS_SRD_PAC_LOCKED	= 0x40,
	SKP_HW_CMN_STATUS_NSRD_PAC_LOCKED	= 0x80,
	SKP_HW_CMN_STATUS_NS_LOCKED 		= 0x100,
	SKP_HW_CMN_STATUS_S_LOCKED 		= 0x200,
	SKP_HW_CMN_STATUS_ROW_W_LOCKED 		= 0x400,
	SKP_HW_CMN_STATUS_ROW_RD_LOCKED 	= 0x800,
	SKP_HW_CMN_STATUS_ROW_DATA_VALID	= 0x1000
} skp_hw_cmn_status_t;

/* container for the control call*/
typedef struct _skp_hw_ctl_data_s {
	uint32_t addr;
	union {
		skp_hw_cmn_perm_t perm;
		skp_hw_cmn_status_t status;
	};
} skp_hw_ctl_data_t;


typedef enum skp_hw_cmn_err_e{
	SKP_HW_CMN_OK = 0,
	SKP_HW_CMN_ERR_FAIL = -1,
	SKP_HW_CMN_ERR_UNSP = -2,
	SKP_HW_CMN_ERR_KEY_EMPTY = -3,
	SKP_HW_CMN_ERR_DESC_ECC = -4,
	SKP_HW_CMN_ERR_DATA_ECC = -5,
	SKP_HW_CMN_ERR_TMO = -6,
	SKP_HW_CMN_ERR_INVAL = -7,
	SKP_HW_CMN_ERR_BAD_PARAM = -8,
	SKP_HW_CMN_ERR_WRITE_FAIL= -9,
} skp_hw_cmn_err_t;

/* generic row id*/
typedef enum otp_hw_cmn_row_addr_type_e{
	SKP_HW_CMN_ROW_ADDR_NONE = 0x0,
	SKP_HW_CMN_ROW_ADDR_ROW,
} skp_hw_cmn_row_addr_type_t;

/* generic OTP command options*/
typedef enum skp_hw_cmn_ctl_e {
	SKP_HW_CMN_CTL_NONE			= 0x0,
	SKP_HW_CMN_CTL_STATUS 		= 0x1,
	SKP_HW_CMN_CTL_LOCK 			= 0x2,
	SKP_HW_CMN_CTL_UNLOCK 		= 0x4,
	SKP_HW_CMN_CTL_OTPCMD_AUTH_PROG	= 0x8,
	SKP_HW_CMN_CTL_OTPCMD_AUTH_PROG_DONE	= 0x10,
	SKP_HW_CMN_CTL_OTPCMD_ECC 		= 0x20,
	SKP_HW_CMN_CTL_OTPCMD_ECC_WREAD 	= 0x40,
	SKP_HW_CMN_CTL_OTPCMD_PROG_LOCK 	= 0x80,
	SKP_HW_CMN_CTL_CONF			= 0x100
} skp_hw_cmn_ctl_t;

typedef enum skp_hw_content_id {
	SKP_CONT_ID_INVALID,
	SKP_CONT_ID_FLD_ROE,
	SKP_CONT_ID_FLD_ROE1,
	SKP_CONT_ID_FLD_HMID,
	SKP_CONT_ID_KEY_DEV_SPECIFIC,
	SKP_CONT_ID_SER_NUM,
	SKP_CONT_ID_LIC_UNIQUE_ID,
	SKP_CONT_ID_KEY_SECT_1,
	SKP_CONT_ID_KEY_SECT_2,
	SKP_CONT_ID_KEY_SECT_3,
	SKP_CONT_ID_KEY_SECT_4,
	SKP_CONT_ID_KEY_SECT_5,
	SKP_CONT_ID_ANTI_ROLLBACK,
	SKP_CONT_ID_UNUSED_1,
	SKP_CONT_ID_UNUSED_2,
	SKP_CONT_ID_UNUSED_3,
	SKP_CONT_ID_UNUSED_4,
	SKP_CONT_ID_UNUSED_5,
	SKP_CONT_ID_UNUSED_6,
	SKP_CONT_ID_UNUSED_7,
	SKP_CONT_ID_MAX
} skp_hw_content_id_t;

/* a control functions argument */
typedef struct _skp_hw_ctl_data_ {
	skp_hw_cmn_ctl_t ctl;
	uintptr_t data;
	uint32_t size;
} skp_hw_cmn_ctl_cmd_t;

/* a row configuration container */
typedef struct skp_hw_row_conf_s {
	skp_hw_cmn_ctl_t	op_type;
	skp_hw_cmn_row_addr_type_t addr_type;
	ulong perm;
	uintptr_t arg;
} skp_hw_cmn_row_conf_t;

/* a row data container with map to feature name*/
typedef struct skp_hw_cmn_row_s {
	skp_hw_content_id_t feat;
	char * reserved_name;
	uint32_t addr;
	uint32_t mask;
	uint32_t shift;
	uint32_t range;
	skp_hw_cmn_row_conf_t conf;
	union {
		u8* pdata;
		uint32_t data;
	};
	uint32_t valid;
} skp_hw_cmn_row_t;

/* Basic Functions */
int skp_get_keyslot_data( int section_num, char* skp_data, int data_len, int * result );
int skp_set_keyslot_data( int section_num, char* skp_data, int data_len, int * result );
int skp_set_keyslot_readlock(int section_num, int * result);
int skp_get_keyslot_readlock_status(int section_num, int * result );
int skp_get_rollback_lvl( uint32_t * lvl, int * result);
int skp_set_rollback_lvl( uint32_t * lvl, int * result);
int skp_get_info( SOTP_INFO_BLK * info_blk, int data_len, int * result );
int skp_init ( void * base_ptr );
int skp_dump_map( int * result );
#ifdef __cplusplus
}
#endif

#endif /* _BCM_HWDEFS_H */

